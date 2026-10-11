/**
 * @file plausibility.c
 * @brief Monitor di plausibilità: controlli di sicurezza indipendenti,
 *        ognuno debounced, aggregati in una bitmask di guasti.
 *
 * Nessuno di questi check taglia la coppia da solo: restituiscono una bitmask
 * che DriveLogicTask usa per portare la FSM in VEHICLE_STATE_IMPLAUSIBILITY.
 * Note sui segnali: potenza/corrente sono STIMATE da coppia*giri agli inverter
 * (il BMS su 0x103 non manda tensione/corrente di pacco). `bms_pack_current` in
 * VehicleData_t resta quindi inutilizzato qui.
 */

#include "Control/plausibility.h"
#include "Control/control_config.h"
#include "Control/inverter.h"

#include <math.h>
#include <stdlib.h>

#define RPM_TO_RAD_S   0.10471975511965977f

static inline float bus_voltage_v(const VehicleData_t *v) {
    return ((float)v->inv_l_dc_bus_voltage + (float)v->inv_r_dc_bus_voltage) * 0.5f;
}

/* Potenza meccanica totale [W], segno incluso (positiva = trazione). */
static inline float mech_power_total_w(const VehicleData_t *v) {
    float p_l = (float)v->inv_l_torque * ((float)v->inv_l_speed_rpm * RPM_TO_RAD_S);
    float p_r = (float)v->inv_r_torque * ((float)v->inv_r_speed_rpm * RPM_TO_RAD_S);
    return p_l + p_r;
}

static inline float torque_request_total_nm(const VehicleData_t *v) {
    return v->torque_left_nm + v->torque_right_nm;
}

/* Corrente di pacco STIMATA [A], + = scarica (fallback se il CAB 500 tace). */
static float pack_current_est_a(const VehicleData_t *v) {
    float v_dc  = bus_voltage_v(v);
    float p_tot = mech_power_total_w(v);            /* segno incluso (regen < 0) */
    if (v_dc < 50.0f) {
        return 0.0f;
    }
    /* motoring: I ~ P_mecc / (eta * V); regen: I ~ P_mecc * eta / V (rendimento inverso) */
    return (p_tot >= 0.0f) ? (p_tot / (MOTOR_EFFICIENCY * v_dc))
                           : (p_tot * MOTOR_EFFICIENCY / v_dc);
}

/* Corrente di pacco [A], + = scarica. Usa la MISURA reale del sensore LEM
 * CAB 500 se fresca, altrimenti ripiega sulla stima da inverter. */
static float pack_current_a(const VehicleData_t *v) {
    if (!v->cab500_timeout_flag) {
        return v->bms_pack_current;
    }
    return pack_current_est_a(v);
}

/* ============================================================================
 * Check individuali. Firma comune: (dati, monitor per stato, coppia attiva).
 * Ritornano true se la condizione E' VIOLATA.
 * ============================================================================ */
static bool check_apps_brake(const VehicleData_t *v, PlausibilityMonitor_t *m, bool ta) {
    (void)ta;
    if ((v->apps_percent > SAFETY_APPS_BRAKE_CUT_APPS_PCT) &&
        (v->brake_pressure_bar > SAFETY_APPS_BRAKE_CUT_BRAKE_BAR)) {
        m->apps_brake_latch = true;
    } else if (v->apps_percent < SAFETY_APPS_BRAKE_RESET_APPS_PCT) {
        m->apps_brake_latch = false;
    }
    return m->apps_brake_latch;
}

static bool check_apps_disagree(const VehicleData_t *v, PlausibilityMonitor_t *m, bool ta) {
    (void)m; (void)ta;
    return (fabsf(v->apps1_percent - v->apps2_percent) > PLAUS_APPS_DISAGREE_PCT);
}

static bool check_sensor_range(const VehicleData_t *v, PlausibilityMonitor_t *m, bool ta) {
    (void)m; (void)ta;
    return (v->apps_percent       > PLAUS_APPS_MAX_PCT)  ||
           (v->apps_percent       < PLAUS_APPS_MIN_PCT)  ||
           (v->brake_pressure_bar > PLAUS_BRAKE_MAX_BAR) ||
           (v->brake_pressure_bar < PLAUS_BRAKE_MIN_BAR) ||
           (v->steering_angle_deg >  PLAUS_STEER_MAX_DEG) ||
           (v->steering_angle_deg < -PLAUS_STEER_MAX_DEG);
}

static bool check_overpower(const VehicleData_t *v, PlausibilityMonitor_t *m, bool ta) {
    (void)m; (void)ta;
    /* Potenza elettrica di pacco = I_pack * V_dc (I dal CAB 500 se disponibile). */
    float p_elec = pack_current_a(v) * bus_voltage_v(v);
    return (p_elec > (PLAUS_MAX_POWER_W * PLAUS_POWER_MARGIN));
}

static bool check_overcurrent(const VehicleData_t *v, PlausibilityMonitor_t *m, bool ta) {
    (void)m; (void)ta;
    if (v->bms_max_discharge_a == 0U) {
        return false;   /* limite BMS non ancora ricevuto */
    }
    return (pack_current_a(v) > ((float)v->bms_max_discharge_a * PLAUS_OVERCURRENT_MARGIN));
}

/* Emulazione fusibile di pacco (I^2t). Accumula (I^2 - In^2)*dt: sopra In
 * l'accumulatore cresce, sotto In "raffredda". Scatta quando supera il budget.
 * Riproduce la curva tempo-corrente reale del fusibile senza tagliare per
 * spike brevi (es. launch). */
static bool check_fuse_i2t(const VehicleData_t *v, PlausibilityMonitor_t *m, bool ta) {
    (void)ta;
    const float i_pack  = pack_current_a(v);   /* i^2 -> il segno non conta (regen incluso) */
    const float i_rated2 = PACK_FUSE_RATED_A * PACK_FUSE_RATED_A;

    m->fuse_i2t += (i_pack * i_pack - i_rated2) * PLAUS_DT_S;

    if (m->fuse_i2t < 0.0f) {
        m->fuse_i2t = 0.0f;
    }
    /* anti-windup: non oltre il doppio della soglia */
    if (m->fuse_i2t > (2.0f * PACK_FUSE_I2T_TRIP_A2S)) {
        m->fuse_i2t = 2.0f * PACK_FUSE_I2T_TRIP_A2S;
    }

    return (m->fuse_i2t >= PACK_FUSE_I2T_TRIP_A2S);
}

static bool check_torque_gt_pedal(const VehicleData_t *v, PlausibilityMonitor_t *m, bool ta) {
    (void)m;
    if (!ta) return false;
    /* throttle_real = pedale dopo la mappa (0-100); fallback su apps_percent */
    float pedal = (v->throttle_real > 0) ? (float)v->throttle_real : v->apps_percent;
    float pedal_torque = (pedal / 100.0f) * MOTOR_MAX_TORQUE_NM * 2.0f;   /* 2 motori */
    return (torque_request_total_nm(v) >
            (pedal_torque * PLAUS_TORQUE_PEDAL_MARGIN + PLAUS_TORQUE_PEDAL_OFFSET_NM));
}

static bool check_regen_while_accel(const VehicleData_t *v, PlausibilityMonitor_t *m, bool ta) {
    (void)m;
    if (!ta) return false;
    return (torque_request_total_nm(v) < -PLAUS_TORQUE_EPS_NM) &&
           (v->apps_percent > PLAUS_REGEN_ACCEL_APPS_PCT);
}

static bool check_drive_while_brake(const VehicleData_t *v, PlausibilityMonitor_t *m, bool ta) {
    (void)m; (void)ta;
    return (torque_request_total_nm(v) > PLAUS_DRIVE_WHILE_BRAKE_NM) &&
           v->brake_is_hard &&
           (v->apps_percent < SAFETY_APPS_BRAKE_RESET_APPS_PCT);
}

static bool check_motor_overspeed(const VehicleData_t *v, PlausibilityMonitor_t *m, bool ta) {
    (void)m; (void)ta;
    return (abs((int)v->inv_l_speed_rpm) > (int)MOTOR_MAX_SPEED_RPM) ||
           (abs((int)v->inv_r_speed_rpm) > (int)MOTOR_MAX_SPEED_RPM);
}

static bool check_bus_voltage(const VehicleData_t *v, PlausibilityMonitor_t *m, bool ta) {
    (void)m; (void)ta;
    if (!v->bms_sdc_status) return false;
    float v_dc = bus_voltage_v(v);
    if (v_dc < 10.0f) return false;   /* inverter non ancora in lettura: non e' un fault di tensione */
    return (v_dc < PLAUS_VDC_MIN_V) || (v_dc > PLAUS_VDC_MAX_V);
}

/* ============================================================================
 * Tabella dei check. trip_cycles = cicli @100 Hz consecutivi prima di scattare.
 * ============================================================================ */
typedef bool (*plaus_fn)(const VehicleData_t *v, PlausibilityMonitor_t *m, bool ta);

typedef struct {
    uint32_t    bit;
    const char *name;
    uint8_t     trip_cycles;
    plaus_fn    fn;
} PlausCheckDef_t;

static const PlausCheckDef_t k_checks[] = {
    { PLAUS_APPS_BRAKE,        "APPS_BRAKE",         1U, check_apps_brake        },
    { PLAUS_APPS_DISAGREE,     "APPS_DISAGREE",     15U, check_apps_disagree     },
    { PLAUS_SENSOR_RANGE,      "SENSOR_RANGE",       3U, check_sensor_range      },
    { PLAUS_OVERPOWER,         "OVERPOWER",         20U, check_overpower         },
    { PLAUS_OVERCURRENT,       "OVERCURRENT",       20U, check_overcurrent       },
    { PLAUS_TORQUE_GT_PEDAL,   "TORQUE_GT_PEDAL",   10U, check_torque_gt_pedal   },
    { PLAUS_REGEN_WHILE_ACCEL, "REGEN_WHILE_ACCEL", 10U, check_regen_while_accel },
    { PLAUS_DRIVE_WHILE_BRAKE, "DRIVE_WHILE_BRAKE",  5U, check_drive_while_brake },
    { PLAUS_MOTOR_OVERSPEED,   "MOTOR_OVERSPEED",    5U, check_motor_overspeed   },
    { PLAUS_BUS_VOLTAGE,       "BUS_VOLTAGE",       10U, check_bus_voltage       },
    { PLAUS_FUSE_I2T,          "FUSE_I2T",           1U, check_fuse_i2t          },
};

_Static_assert((sizeof(k_checks) / sizeof(k_checks[0])) == PLAUS_CHECK_COUNT,
               "PLAUS_CHECK_COUNT non allineato a k_checks[] (aggiornalo in plausibility.h)");

/* ============================================================================
 * API
 * ============================================================================ */
void Plausibility_Init(PlausibilityMonitor_t *m) {
    if (m == NULL) return;
    for (uint32_t i = 0U; i < PLAUS_CHECK_COUNT; ++i) {
        m->fail_cnt[i] = 0U;
    }
    m->active = 0U;
    m->apps_brake_latch = false;
    m->fuse_i2t = 0.0f;
}

void Plausibility_Reset(PlausibilityMonitor_t *m) {
    Plausibility_Init(m);
}

uint32_t Plausibility_Update(PlausibilityMonitor_t *m, const VehicleData_t *v, bool torque_active) {
    if (m == NULL || v == NULL) return 0U;

    for (uint32_t i = 0U; i < PLAUS_CHECK_COUNT; ++i) {
        const bool violated = k_checks[i].fn(v, m, torque_active);

        if (violated) {
            if (m->fail_cnt[i] < k_checks[i].trip_cycles) {
                m->fail_cnt[i]++;
            }
        } else if (m->fail_cnt[i] > 0U) {
            m->fail_cnt[i]--;
        }

        if (m->fail_cnt[i] >= k_checks[i].trip_cycles) {
            m->active |= k_checks[i].bit;
        } else if (m->fail_cnt[i] == 0U) {
            m->active &= ~k_checks[i].bit;
        }
    }

    return m->active;
}

const char *Plausibility_FaultName(uint32_t faults) {
    if (faults == 0U) return "OK";
    for (uint32_t i = 0U; i < PLAUS_CHECK_COUNT; ++i) {
        if ((faults & k_checks[i].bit) != 0U) {
            return k_checks[i].name;
        }
    }
    return "?";
}
