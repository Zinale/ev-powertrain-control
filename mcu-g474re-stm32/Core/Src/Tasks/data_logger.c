/**
 * @file data_logger.c
 * @brief Telemetry & Diagnostics Logger Task @ 10 Hz
 * 
 * - Trasmissione periodica frame telemetria veicolo su FDCAN2
 * - Stampa diagnostica seriale condizionale via LPUART1 (DMA)
 */

#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include "shared_data.h"
#include "main.h"
#include "Connectivity/Can.h"
#include "Connectivity/Serial.h"
#include "Control/inverter.h"
#include "Control/plausibility.h"

/* ============================================================================
 * SWITCH PRECOMPILATORE DEBUG SERIALE
 * Imposta a 1 per abilitare le stampe UART, a 0 per disabilitarle del tutto
 * ============================================================================ */
#define DATALOGGER_ENABLE_UART_DEBUG    1U

#define DATA_LOGGER_PERIOD_MS           100U   /* 10 Hz */
#define MS_TO_TICKS(ms)                 ((uint32_t)((ms) * osKernelGetTickFreq() / 1000U))

extern FDCAN_HandleTypeDef hfdcan2;

/* I frame debug controlli 0x109/0x110/0x111 e 0x104/0x105/0x007 sono Big-Endian
 * (Motorola) come da DBC. NB: gli altri frame TX MCU (0x108, 0x250...) sono LE. */
static inline void be_u16(uint8_t *d, uint16_t v) { d[0] = (uint8_t)(v >> 8); d[1] = (uint8_t)v; }
static inline void be_i16(uint8_t *d, int16_t  v) { be_u16(d, (uint16_t)v); }

static inline int16_t kmh_to_s16(float kmh) {
    float x = kmh * WHEEL_SPEED_CAN_SCALE;
    if (x >  32767.0f) x =  32767.0f;
    if (x < -32768.0f) x = -32768.0f;
    return (int16_t)x;
}

/* Rigenerazione confermata dagli inverter, sulla coppia MISURATA. Isteresi
 * (scatta oltre 0.2 Nm, rilascia sotto 0.1 Nm) per non far sfarfallare i bit
 * attorno allo zero.
 *  - "any": almeno un inverter rigenera (bit 3 di 0x006, diagnostica);
 *  - "net": coppia netta (somma) negativa, cioe' la vettura sta davvero
 *    decelerando (brake light). Con il torque vectoring la ruota interna puo'
 *    avere coppia negativa mentre il pilota accelera: li' la luce non deve
 *    accendersi. */
#define REGEN_CONFIRM_MIN_NM       0.2f
#define REGEN_CONFIRM_RELEASE_NM   0.1f

static bool below_with_hysteresis(bool *active, float torque_nm) {
    if (torque_nm < -REGEN_CONFIRM_MIN_NM) {
        *active = true;
    } else if (torque_nm > -REGEN_CONFIRM_RELEASE_NM) {
        *active = false;
    }
    return *active;
}

/* Nm -> S16 con fattore DBC 0.01 (come Grip Estimator in 0x111). */
static inline int16_t nm_to_s16_x100(float nm) {
    float x = nm * 100.0f;
    if (x >  32767.0f) x =  32767.0f;
    if (x < -32768.0f) x = -32768.0f;
    return (int16_t)x;
}

static inline uint8_t bar_to_u8(float bar) {
    if (bar <   0.0f) bar =   0.0f;
    if (bar > 255.0f) bar = 255.0f;
    return (uint8_t)bar;
}

#if (DATALOGGER_ENABLE_UART_DEBUG == 1)
#define RUNTIME_STATS_MAX_TASKS     8U
#define RUNTIME_STATS_PERIOD_CYCLES (5000U / DATA_LOGGER_PERIOD_MS)  /* ~ogni 5 s */

/* Carico CPU per task sull'ultima finestra tra due stampe (~5 s): permette di
 * verificare che nessun task sia in starvation (IDLE a 0% = CPU satura) senza
 * oscilloscopio/analizzatore logico.
 * I contatori FreeRTOS sono cumulativi dal boot e a 32 bit: il cycle counter
 * DWT (170 MHz) fa overflow dopo ~25 s, quindi il rapporto cumulativo diventa
 * errato. Si lavora perche' sui DELTA tra due letture, che restano corretti
 * attraverso l'overflow (sottrazione unsigned) finche' la finestra e' < ~25 s.
 * uxTaskGetSystemState non alloca memoria (array fornito dal chiamante), a
 * differenza di vTaskGetRunTimeStats(). */
static void print_runtime_stats(void)
{
    static TaskHandle_t prev_handle[RUNTIME_STATS_MAX_TASKS];
    static uint32_t     prev_cnt[RUNTIME_STATS_MAX_TASKS];
    static UBaseType_t  prev_n     = 0U;
    static uint32_t     prev_total = 0U;

    TaskStatus_t task_status[RUNTIME_STATS_MAX_TASKS];
    uint32_t total_run_time = 0U;

    UBaseType_t n = uxTaskGetSystemState(task_status, RUNTIME_STATS_MAX_TASKS, &total_run_time);

    const uint32_t total_delta = total_run_time - prev_total;   /* mod 2^32 */
    if (n == 0U || total_delta == 0U) return;

    LPUART1_Log("---- CPU load, ultima finestra (%lu task) ----\r\n", (unsigned long)n);
    for (UBaseType_t i = 0; i < n; i++) {
        uint32_t prev = 0U;   /* task nuovo rispetto alla stampa precedente */
        for (UBaseType_t j = 0; j < prev_n; j++) {
            if (prev_handle[j] == task_status[i].xHandle) { prev = prev_cnt[j]; break; }
        }
        const uint32_t task_delta = task_status[i].ulRunTimeCounter - prev;   /* mod 2^32 */
        uint32_t pct_x10 = (uint32_t)(((uint64_t)task_delta * 1000ULL) / total_delta);
        LPUART1_Log("  %-16s prio:%2lu stack_free:%4uw CPU:%3lu.%1lu%%\r\n",
                    task_status[i].pcTaskName,
                    (unsigned long)task_status[i].uxCurrentPriority,
                    (unsigned)task_status[i].usStackHighWaterMark,
                    (unsigned long)(pct_x10 / 10U), (unsigned long)(pct_x10 % 10U));
    }

    for (UBaseType_t i = 0; i < n; i++) {
        prev_handle[i] = task_status[i].xHandle;
        prev_cnt[i]    = task_status[i].ulRunTimeCounter;
    }
    prev_n     = n;
    prev_total = total_run_time;
}

static const char* imu_boot_to_str(ImuBootStatus_t s) {
    switch (s) {
        case IMU_BOOT_OK:           return "OK";
        case IMU_BOOT_MISMATCH:     return "MISMATCH";
        case IMU_BOOT_RECONFIGURED: return "RECONFIG";
        case IMU_BOOT_NO_RESPONSE:  return "NO_RESP";
        default:                    return "UNKNOWN";
    }
}

static const char* fsm_state_to_str(VehicleState_t state) {
    switch (state) {
        case VEHICLE_STATE_INIT:           return "INIT";
        case VEHICLE_STATE_READY:          return "READY";
        case VEHICLE_STATE_RUNNING:        return "RUNNING";
        case VEHICLE_STATE_IMPLAUSIBILITY: return "IMPLAUS";
        case VEHICLE_STATE_CAR_ERROR:      return "CAR_ERR";
        case VEHICLE_STATE_ERROR:          return "ERROR";
        default:                           return "UNK";
    }
}
#endif

/* ============================================================================
 * Funzione Trasmissione Frame Telemetria su CAN2 (FDCAN2)
 * ============================================================================ */
static void DataLogger_TransmitCanTelemetry(const VehicleData_t *v) {
    /* 1. Frame 0x108 (DLC 4): APPS finale, APP1, APP2 [0-100 %] e angolo volante
     *    in gradi S8 (la dash lo legge come gradi, fondo scala 120). */
    float sas_f = v->steering_angle_deg;
    sas_f += (sas_f >= 0.0f) ? 0.5f : -0.5f;                 /* arrotonda al grado */
    if (sas_f >  127.0f) sas_f =  127.0f;
    if (sas_f < -128.0f) sas_f = -128.0f;
    CAN_MAIN_TransmitAppsSas(&hfdcan2, (uint8_t)v->apps_percent,
                             (uint8_t)v->apps1_percent, (uint8_t)v->apps2_percent,
                             (int8_t)sas_f);

    /* 2. Frame 0x006: Byte cumulativo errori e stati MCU */
    uint8_t errors_byte = 0U;
    /* INV1 = nodo 1 = DESTRO, INV2 = nodo 2 = SINISTRO: stessa convenzione di 0x002/0x250 */
    if (v->inv_r_status_word & INV_STAT_DERATING) errors_byte |= CAN_ERR_INV1_DERATING;
    if (v->inv_l_status_word & INV_STAT_DERATING) errors_byte |= CAN_ERR_INV2_DERATING;
    /* Implausibilita' APPS: latch di apps.c (mismatch / fuori range) oppure
     * APPS_BRAKE / APPS_DISAGREE del monitor di plausibilita'. */
    if (v->apps_implausibility ||
        (v->plausibility_faults & (PLAUS_APPS_BRAKE | PLAUS_APPS_DISAGREE))) {
        errors_byte |= CAN_ERR_APPS_IMPLAUS;
    }
    /* Regen attiva = conferma dagli inverter: coppia MISURATA negativa su almeno
     * uno dei due. (0x109 bit 7 e' invece il flag regen del modello Simulink, per debug.) */
    static bool s_regen_any = false;
    static bool s_regen_net = false;
    const float t_min = (v->inv_l_torque < v->inv_r_torque) ? v->inv_l_torque : v->inv_r_torque;
    const bool regen_any = below_with_hysteresis(&s_regen_any, t_min);
    const bool regen_net = below_with_hysteresis(&s_regen_net, v->inv_l_torque + v->inv_r_torque);
    if (regen_any) errors_byte |= CAN_ERR_REGEN_ACTIVE;
    if (v->apps1_oor)  errors_byte |= CAN_ERR_APPS1_OOR;
    if (v->apps2_oor)  errors_byte |= CAN_ERR_APPS2_OOR;
    if (v->sas_oor)    errors_byte |= CAN_ERR_SAS_OOR;
    if (v->brake_oor)  errors_byte |= CAN_ERR_BRAKE_OOR;
    CAN_MAIN_TransmitErrors(&hfdcan2, errors_byte);

    /* 3. Frame 0x250 (INV1 = destro) e 0x251 (INV2 = sinistro): telemetria
     *    inverter, dai campi dello snapshot (gia' copiati da DataManager sotto mutex). */
    CAN_MAIN_TransmitInverterData(&hfdcan2, INVERTER_RIGHT_NODE_ID,
                                  v->inv_r_status_word, v->inv_r_speed_rpm,
                                  v->inv_r_motor_temp_degc, v->inv_r_temp_degc);
    CAN_MAIN_TransmitInverterData(&hfdcan2, INVERTER_LEFT_NODE_ID,
                                  v->inv_l_status_word, v->inv_l_speed_rpm,
                                  v->inv_l_motor_temp_degc, v->inv_l_temp_degc);

    /* 4. Frame 0x002 e 0x003: error_code / error_info degli inverter (sempre;
     *    error_code = 0 quando non in fault). */
    CAN_MAIN_TransmitInverterError(&hfdcan2, INVERTER_RIGHT_NODE_ID,
                                   v->inv_r_error_code, v->inv_r_error_info_1);
    CAN_MAIN_TransmitInverterError(&hfdcan2, INVERTER_LEFT_NODE_ID,
                                   v->inv_l_error_code, v->inv_l_error_info_1);

    /* 5. Frame 0x109: debug controlli #1 (Torque cmd, Yaw Th, Regen/TVC, Throttle) */
    {
        uint8_t d[8];
        be_i16(&d[0], v->inv_l_torque_cmd_raw);          /* Torque Command RL  [0.1% Mn] */
        be_i16(&d[2], v->inv_r_torque_cmd_raw);          /* Torque Command RR  [0.1% Mn] */
        be_i16(&d[4], v->yaw_th);                        /* Yaw Th             [rad/s * 0.01] */
        uint8_t tw = (v->tvc_weight > 100U) ? 100U : v->tvc_weight;
        d[6] = (uint8_t)((v->regen_active ? 0x80U : 0x00U) | (tw & 0x7FU));
        d[7] = (uint8_t)((v->throttle_real < 0) ? 0 : v->throttle_real);   /* Throttle Real [0-100 %] */
        CAN_MAIN_TransmitRaw(&hfdcan2, CAN_ID_TX_CTRL_DEBUG_1, d, 8U);
    }

    /* 6. Frame 0x110: debug controlli #2 (Lambda 4 ruote, K-slip RL/RR) */
    {
        uint8_t d[8];
        d[0] = v->lambda_ref_RL;                             /* [0-100] = slip * 100 */
        d[1] = v->lambda_ref_RR;
        d[2] = v->lambda_RL;
        d[3] = v->lambda_RR;
        be_i16(&d[4], v->k_slip_left);                   /* K-slip RL [Nm * 0.01] */
        be_i16(&d[6], v->k_slip_right);                  /* K-slip RR [Nm * 0.01] */
        CAN_MAIN_TransmitRaw(&hfdcan2, CAN_ID_TX_CTRL_DEBUG_2, d, 8U);
    }

    /* 7. Frame 0x111: debug controlli #3 (Grip Estimator T_RL/RR, coppia attuale inverter) */
    {
        uint8_t d[8];
        be_i16(&d[0], v->T_max_RL);                      /* Grip Estimator T_RL [Nm * 0.01] */
        be_i16(&d[2], v->T_max_RR);                      /* Grip Estimator T_RR [Nm * 0.01] */
        be_i16(&d[4], nm_to_s16_x100(v->inv_l_torque));  /* T_act RL [Nm * 0.01] (feedback inverter) */
        be_i16(&d[6], nm_to_s16_x100(v->inv_r_torque));  /* T_act RR [Nm * 0.01] */
        CAN_MAIN_TransmitRaw(&hfdcan2, CAN_ID_TX_CTRL_DEBUG_3, d, 8U);
    }

    /* 8. Frame 0x007: dettaglio bitmask implausibilità (U16 BE, vedi PlausibilityFault_t) */
    {
        uint8_t d[2];
        be_u16(&d[0], (uint16_t)v->plausibility_faults);
        CAN_MAIN_TransmitRaw(&hfdcan2, CAN_ID_TX_MCU_PLAUSIBILITY, d, 2U);
    }

    /* 9. Frame 0x104 -> DASH/PDM: velocita' ruote/veicolo [km/h * 10, S16 BE] +
     *    stato FSM (= stato R2D autoritativo) + flag "armato". */
    {
        uint8_t d[8];
        be_i16(&d[0], kmh_to_s16(v->wheel_speed_fr_kmh));   /* Wheel FR  */
        be_i16(&d[2], kmh_to_s16(v->wheel_speed_fl_kmh));   /* Wheel FL  */
        be_i16(&d[4], kmh_to_s16(v->vehicle_speed_kmh));    /* Car speed */
        d[6] = (uint8_t)v->fsm_state;                       /* FSM MCU state (0=INIT..5=ERROR) */
        /* Byte 7 - stato R2D auto-contenuto (non serve leggere l'enum FSM):
         *   bit0 = R2D armato (pilota ha premuto; HV magari ancora in salita)
         *   bit1 = R2D ATTIVO / EFFETTIVO (coppia abilitata = FSM RUNNING)
         *          -> questo fa suonare il buzzer PDM e si vede sul display  */
        d[7] = (uint8_t)((v->r2d_armed ? 0x01U : 0x00U) |
                         ((v->fsm_state == VEHICLE_STATE_RUNNING) ? 0x02U : 0x00U));
        CAN_MAIN_TransmitRaw(&hfdcan2, CAN_ID_TX_MCU_WHEELS, d, 8U);
    }

    /* 10. Frame 0x105 -> DASH/PDM1: pressione freno anteriore + stato luce stop */
    {
        uint8_t d[2];
        d[0] = bar_to_u8(v->brake_pressure_bar);            /* [bar] */
        /* Brake light: freno premuto OPPURE rigenerazione netta confermata dagli
         * inverter (la vettura decelera anche senza pressione idraulica).
         * 0 = OFF, 1 = ON. */
        d[1] = (v->brake_is_pressed || regen_net) ? 1U : 0U;
        CAN_MAIN_TransmitRaw(&hfdcan2, CAN_ID_TX_MCU_BRAKE, d, 2U);
    }
}

/* ============================================================================
 * Task FreeRTOS DataLogger
 * ============================================================================ */
void DataLoggerTask(void) {
    uint32_t next_wake = osKernelGetTickCount();
    const uint32_t period_ticks = MS_TO_TICKS(DATA_LOGGER_PERIOD_MS);

    VehicleData_t v;
    uint32_t log_seq = 0;

    for (;;) {
        /* Prelievo atomico sincronizzato */
        VehicleData_GetSnapshot(&v);
        log_seq++;

        /* --------------------------------------------------------------------
         * 1. Streaming Telemetria su Bus CAN2 (Car Data)
         * -------------------------------------------------------------------- */
        DataLogger_TransmitCanTelemetry(&v);

        /* --------------------------------------------------------------------
         * 2. Stampa Seriale di Debug via LPUART1 (Opzionale da Precompilatore)
         * -------------------------------------------------------------------- */
#if (DATALOGGER_ENABLE_UART_DEBUG == 1)
        /* Blocco 1: FSM, Sicurezza, Pedali e Sterzo */
        LPUART1_Log("=== [#%lu] t=%lu ms | FSM: %s | R2D req:%d armed:%d | Map: %u | Plaus: %s (0x%03lX) ===\r\n"
                    "APPS: %5.1f%% [A1: %5.1f%% (%u), A2: %5.1f%% (%u)] | Fault: %d\r\n"
                    "BRAKE: %5.1f bar (%5.1f%%, ADC: %u) [Pressed: %d, Hard: %d]\r\n"
                    "STEER: %5.1f deg (ADC: %u) | SPEED: %5.1f km/h [FL: %5.1f, FR: %5.1f]\r\n",
                    log_seq, HAL_GetTick(), fsm_state_to_str(v.fsm_state), v.r2d_request, v.r2d_armed, v.engine_map,
                    Plausibility_FaultName(v.plausibility_faults), (unsigned long)v.plausibility_faults,
                    v.apps_percent, v.apps1_percent, v.apps1_raw_adc, v.apps2_percent, v.apps2_raw_adc, v.sensor_fault_flag,
                    v.brake_pressure_bar, v.brake_percent, v.brake_raw_adc, v.brake_is_pressed, v.brake_is_hard,
                    v.steering_angle_deg, v.steering_raw_adc, v.vehicle_speed_kmh, v.wheel_speed_fl_kmh, v.wheel_speed_fr_kmh);

        /* Blocco 2: Target Powertrain & Telemetria Inverter */
        LPUART1_Log("TQ_TGT: %5.1f Nm [L_tgt: %5.1f, R_tgt: %5.1f] | CmdRaw: [L: %d, R: %d]\r\n"
                    "INV_L: St: %s (0x%04X) | %5d RPM | %5.1f Nm | %u V | Pwr: %lu W | T_mot: %.1f C, T_igbt: %.1f C | Err: 0x%04X (Info: 0x%08lX)\r\n"
                    "INV_R: St: %s (0x%04X) | %5d RPM | %5.1f Nm | %u V | Pwr: %lu W | T_mot: %.1f C, T_igbt: %.1f C | Err: 0x%04X (Info: 0x%08lX)\r\n",
                    (v.torque_left_nm + v.torque_right_nm), v.torque_left_nm, v.torque_right_nm, v.inv_l_torque_cmd_raw, v.inv_r_torque_cmd_raw,
                    Inverter_GetStateName((InverterState_t)v.inv_l_state), v.inv_l_status_word, v.inv_l_speed_rpm,
                    v.inv_l_torque, v.inv_l_dc_bus_voltage, v.inv_l_actual_power_w,
                    (float)v.inv_l_motor_temp_degc * 0.1f, (float)v.inv_l_igbt_temp_degc * 0.1f, v.inv_l_error_code, v.inv_l_error_info_1,
                    Inverter_GetStateName((InverterState_t)v.inv_r_state), v.inv_r_status_word, v.inv_r_speed_rpm,
                    v.inv_r_torque, v.inv_r_dc_bus_voltage, v.inv_r_actual_power_w,
                    (float)v.inv_r_motor_temp_degc * 0.1f, (float)v.inv_r_igbt_temp_degc * 0.1f, v.inv_r_error_code, v.inv_r_error_info_1);

        /* Blocco 3: BMS, IMU e Stato Bus CAN */
        LPUART1_Log("BMS: SOC: %4.1f%%, SOH: %4.1f%% | I_pack: %6.1f A (CAB to:%d) | MaxDch: %u A, MaxReg: %u A | SDC: %d | Err: %d\r\n"
                    "IMU: Acc[X: %d, Y: %d, Z: %d] | Gyro[Z: %d] | Boot: %s (fw %u.%02u) | PDM_Err: [P1: %d, P2: %d]\r\n"
                    "CAN_STAT: RX1_cnt: %lu, RX2_cnt: %lu | Timeouts: [INV: %d, BMS: %d, IMU: %d, DASH: %d, PDM: %d]\r\n\r\n",
                    v.bms_soc_percent, v.bms_soh_percent, v.bms_pack_current, v.cab500_timeout_flag,
                    v.bms_max_discharge_a, v.bms_max_regen_a, v.bms_sdc_status, v.bms_error,
                    v.imu_accel_x_raw, v.imu_accel_y_raw, v.imu_accel_z_raw, v.imu_gyro_z_raw,
                    imu_boot_to_str(g_imu_boot_status), (g_imu_fw_version / 100U), (g_imu_fw_version % 100U),
                    v.PDM1_error, v.PDM2_error,
                    v.can1_rx_cnt, v.can2_rx_cnt, v.inv_timeout_flag, v.bms_timeout_flag, v.imu_timeout_flag, v.dash_timeout_flag, v.pdm_timeout_flag);

        /* Blocco 4: carico CPU per task, a bassa cadenza (non ogni ciclo) */
        if ((log_seq % RUNTIME_STATS_PERIOD_CYCLES) == 0U) {
            print_runtime_stats();
        }
#endif

        next_wake += period_ticks;
        osDelayUntil(next_wake);
    }
}