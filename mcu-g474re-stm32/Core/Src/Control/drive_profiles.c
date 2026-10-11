/**
 * @file drive_profiles.c
 * @brief Mappa powertrain (ECO/NORMAL/GAS) + configurazione parametri
 *        P1 REGEN / P2 SRC / P3 TVC / P4 EVENT selezionati dal volante
 *        (frame CAN 0x106), e programmazione dei parametri globali esportati
 *        dal modello Simulink 'driveController' (v1.64).
 *
 * MODELLO A RUNTIME: il codice generato NON contiene Simulink. I parametri
 * esportati (tvc_Kp_p, slip_Kp_p, mu_p, Kus_p, overslip_factor_p,
 * pedal_map_select_p, ...) sono normali variabili globali C inizializzate al
 * valore preso da MATLAB al momento della code-gen. Scrivendoci da qui il
 * valore resta finche' non lo riscriviamo o si resetta la MCU. I "fattori per
 * livello" sono logica applicativa -> stanno qui.
 * Per NON accumulare le scalature (es. x0.4 su un valore gia' x0.4), a init
 * catturiamo UNA volta i guadagni base (s_base) e ricalcoliamo sempre da li'.
 *
 * Modello di riferimento: driveController v1.66.
 */

#include "Control/drive_profiles.h"
#include "Control/control_config.h"
#include "Control/inverter.h"

#include <math.h>

#if (ACTIVE_CONTROL_MODE == CONTROL_MODE_SIMULINK)
#include "Control/Simulink/driveController.h"
#endif

/* ============================================================================
 * Valori base per mappa powertrain
 * ============================================================================ */
typedef struct {
    float t_peak_nm;
    float regen_ceiling_nm;
    float pedal_alpha;
} PowertrainMapDef_t;

static const PowertrainMapDef_t k_map_def[3] = {
    /* PWR_MAP_ECO    */ { 0.50f * MOTOR_NOMINAL_TORQUE_NM, 4.0f, 1.6f },
    /* PWR_MAP_NORMAL */ {         MOTOR_NOMINAL_TORQUE_NM,  6.0f, 1.0f },
    /* PWR_MAP_GAS    */ {         MOTOR_MAX_TORQUE_NM,      9.0f, 0.6f },
};

/* Valore di pedal_map_select_p atteso dal Multiport Switch '<S3>' del modello:
 *   1 -> curva pedal_map_eco, 2 -> pedal_map_normal, 3/altro -> pedal_map_gas. */
static const uint8_t k_pedal_map_select[3] = {
    /* PWR_MAP_ECO    */ 1U,
    /* PWR_MAP_NORMAL */ 2U,
    /* PWR_MAP_GAS    */ 3U,
};

/* ============================================================================
 * P1 REGEN - livello 0..3 (dash 1..4)
 * I limiti fisici del pacco (I_charge_max, regen_speed_*_rpm) restano nel modello.
 * ============================================================================ */
typedef struct {
    float ceiling_frac;   /* 0 = regen spenta */
    float pedal_hyst;
    float pedal_thr;
} RegenLevelDef_t;

static const RegenLevelDef_t k_regen_lvl[4] = {
    /* L1 */ { 0.00f, 3.0f,  0.0f },   /* OFF                 */
    /* L2 */ { 0.40f, 3.0f,  5.0f },   /* debole (40% mappa)  */
    /* L3 */ { 0.70f, 4.0f,  8.0f },   /* media  (70% mappa)  */
    /* L4 */ { 1.00f, 5.0f, 15.0f },   /* piena  (100% mappa) */
};

/* ============================================================================
 * P3 TVC - fattore sui guadagni base: OFF / -60% / target / +60%
 * P2 SRC - stesso fattore sul PID slip; mu e overslip in valore assoluto
 *          (SRC basso = permissivo: PID/mu bassi, overslip alto).
 * ============================================================================ */
static const float k_tvc_factor[4] = { 0.0f, 0.4f, 1.0f, 1.6f };
static const float k_src_pid_factor[4] = { 0.0f, 0.4f, 1.0f, 1.6f };
static const float k_src_mu[4]         = { 1.08f, 1.11f, 1.14f, 1.17f };
static const float k_src_overslip[4]   = { 1.05f, 1.03f, 1.01f, 1.00f };

/* ============================================================================
 * P4 EVENT - per ora agisce solo su Kus (gradiente di sottosterzo yaw_th)
 * ============================================================================ */
static const float k_event_kus[4] = { 0.0f, 0.005f, 0.010f, -0.005f };

/* ============================================================================
 * Stato modulo
 * ============================================================================ */
static ProfileSettings_t s_active;
static bool              s_initialized = false;

/* Guadagni base del controllore, catturati una sola volta a init. */
typedef struct {
    float tvc_kp, tvc_ki, tvc_kd;
    float slip_kp, slip_ki, slip_kd;
} ControllerBaseGains_t;

static ControllerBaseGains_t s_base;
static bool                  s_base_captured = false;

#define PCFG_SLOT(cfg, n)   (uint8_t)(((cfg) >> (2U * (n))) & 0x03U)

static void capture_base_gains(void) {
    if (s_base_captured) return;
#if (ACTIVE_CONTROL_MODE == CONTROL_MODE_SIMULINK)
    s_base.tvc_kp  = tvc_Kp_p;   s_base.tvc_ki  = tvc_Ki_p;   s_base.tvc_kd  = tvc_Kd_p;
    s_base.slip_kp = slip_Kp_p;  s_base.slip_ki = slip_Ki_p;  s_base.slip_kd = slip_Kd_p;
#else
    /* Fallback ai valori dello script MATLAB. */
    s_base.tvc_kp  = 400.0f; s_base.tvc_ki  = 110.0f; s_base.tvc_kd  = 50.0f;
    s_base.slip_kp = 15.0f;  s_base.slip_ki = 22.0f;  s_base.slip_kd = 1.5f;
#endif
    s_base_captured = true;
}

/* ============================================================================
 * Ricalcolo di s_active dai selettori
 * ============================================================================ */
static void profile_recompute(PowertrainMap_t map, uint8_t cfg, ProfileSettings_t *out) {
    const PowertrainMapDef_t *m = &k_map_def[map];

    out->map               = map;
    out->parameters_config = cfg;
    out->t_peak_nm         = m->t_peak_nm;
    out->regen_ceiling_nm  = m->regen_ceiling_nm;
    out->pedal_alpha       = m->pedal_alpha;
    out->pedal_map_select  = k_pedal_map_select[map];

    /* --- P1 REGEN --- */
    out->regen_level      = PCFG_SLOT(cfg, 0);
    {
        const RegenLevelDef_t *r = &k_regen_lvl[out->regen_level];
        out->regen_enabled    = (r->ceiling_frac > 0.0f);
        out->regen_t_max_nm   = r->ceiling_frac * m->regen_ceiling_nm;
        out->regen_pedal_hyst = r->pedal_hyst;
        out->regen_pedal_thr  = r->pedal_thr;
    }

    /* --- P2 SRC --- */
    out->src_level = PCFG_SLOT(cfg, 1);
    {
        float f = k_src_pid_factor[out->src_level];
        out->slip_kp = s_base.slip_kp * f;
        out->slip_ki = s_base.slip_ki * f;
        out->slip_kd = s_base.slip_kd * f;
        out->mu              = k_src_mu[out->src_level];
        out->overslip_factor = k_src_overslip[out->src_level];
    }

    /* --- P3 TVC --- */
    out->tvc_level = PCFG_SLOT(cfg, 2);
    {
        float f = k_tvc_factor[out->tvc_level];
        out->tvc_kp = s_base.tvc_kp * f;
        out->tvc_ki = s_base.tvc_ki * f;
        out->tvc_kd = s_base.tvc_kd * f;
    }

    /* --- P4 EVENT --- */
    out->event_level = PCFG_SLOT(cfg, 3);
    out->kus         = k_event_kus[out->event_level];
}

/* ============================================================================
 * Push dei parametri nel controllore attivo
 * ============================================================================ */
static void profile_program_controller(const ProfileSettings_t *s) {
#if (ACTIVE_CONTROL_MODE == CONTROL_MODE_SIMULINK)
    /* Mappa powertrain: cap coppia + curva pedale */
    T_peak_p           = (float)s->t_peak_nm;
    pedal_map_select_p = s->pedal_map_select;

    /* P1 REGEN */
    regen_T_max_p      = s->regen_enabled ? (float)s->regen_t_max_nm : 0.0f;
    regen_pedal_hyst_p = (float)s->regen_pedal_hyst;
    regen_pedal_thr_p  = (float)s->regen_pedal_thr;

    /* P2 SRC */
    slip_Kp_p         = (float)s->slip_kp;
    slip_Ki_p         = (float)s->slip_ki;
    slip_Kd_p         = (float)s->slip_kd;
    mu_p              = (float)s->mu;
    overslip_factor_p = (float)s->overslip_factor;

    /* P3 TVC */
    tvc_Kp_p = (float)s->tvc_kp;
    tvc_Ki_p = (float)s->tvc_ki;
    tvc_Kd_p = (float)s->tvc_kd;

    /* P4 EVENT */
    Kus_p = (float)s->kus;

    /* Target slip per l'SRC: default modello (slip_ref = 0.15). Necessario
     * perche' lambda_ref* sono input root a 0. */
    driveController_U.lambda_refRL = 0.15;
    driveController_U.lambda_refRR = 0.15;
#else
    (void)s;
#endif
}

/* ============================================================================
 * API pubblica
 * ============================================================================ */
PowertrainMap_t DriveProfiles_ResolveMap(int8_t raw_engine_map) {
    /* Codifica del volante (0x106 byte 1): 1=ECO, 2=NORMAL, 3=GAS. */
    switch (raw_engine_map) {
        case DASH_ENGINE_MAP_ECO:    return PWR_MAP_ECO;
        case DASH_ENGINE_MAP_NORMAL: return PWR_MAP_NORMAL;
        case DASH_ENGINE_MAP_GAS:    return PWR_MAP_GAS;
        default:                     return PWR_MAP_ECO;   /* fallback conservativo */
    }
}

const ProfileSettings_t *DriveProfiles_GetActive(void) {
    return &s_active;
}

bool DriveProfiles_Update(int8_t raw_engine_map, uint8_t parameters_config,
                          const PowertrainLimits_t *limits) {
    PowertrainMap_t map = DriveProfiles_ResolveMap(raw_engine_map);

    bool changed = (!s_initialized) ||
                   (map != s_active.map) ||
                   (parameters_config != s_active.parameters_config);

    if (changed) {
        profile_recompute(map, parameters_config, &s_active);
        s_initialized = true;
        profile_program_controller(&s_active);
    }

    /* Grandezze elettriche runtime, ogni ciclo (come V_dc). */
#if (ACTIVE_CONTROL_MODE == CONTROL_MODE_SIMULINK)
    if (limits != NULL) {
        if (limits->bus_voltage      >= 1.0f) V_dc_p            = limits->bus_voltage;
        if (limits->i_discharge_max_a >= 0.0f) I_discharge_max_p = limits->i_discharge_max_a;
        if (limits->i_charge_max_a    >= 0.0f) I_charge_max_p    = limits->i_charge_max_a;
    }
#else
    (void)limits;
#endif

    return changed;
}

void DriveProfiles_Init(void) {
    capture_base_gains();
    s_initialized = false;
    /* Valori nominali di partenza; verranno sovrascritti dal primo ciclo con i
     * dati reali BMS/inverter. */
    const PowertrainLimits_t nominal = { .bus_voltage = 540.0f,
                                         .i_discharge_max_a = 100.0f,
                                         .i_charge_max_a = 21.0f };
    (void)DriveProfiles_Update((int8_t)DASH_ENGINE_MAP_ECO, 0U, &nominal);
}
