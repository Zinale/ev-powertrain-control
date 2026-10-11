#include "cmsis_os2.h"
#include "shared_data.h"
#include "main.h"

#include "Control/control_config.h"
#include "Control/drive_profiles.h"
#include "Control/inverter.h"
#include "Control/motor_control.h"
#include "Control/error_recovery.h"
#include "Control/plausibility.h"

#if (ACTIVE_CONTROL_MODE == CONTROL_MODE_SIMULINK)
#include "Control/Simulink/driveController.h"
#endif

extern FDCAN_HandleTypeDef hfdcan1;

#define DRIVE_LOGIC_PERIOD_MS   10U /* 100 Hz */
#define DEG_TO_RAD              0.017453292519943295f
#define MS_TO_TICKS(ms)         ((uint32_t)((ms) * osKernelGetTickFreq() / 1000U))

/* Il modello 'driveController' (v1.58) e' esportato con fixed-step 0.01 s:
 * base rate = periodo del task -> una sola driveController_step() per ciclo.
 * Il blocco TorqueAllocator gira a 0.05 s come subrate (gestito internamente
 * da rate_scheduler()), quindi T_RL/T_RR si aggiornano ogni 5 cicli. */

static PlausibilityMonitor_t s_plaus;

/* --- Stato R2D / arming (posseduto dalla MCU) --------------------------------
 * Il volante manda solo una RICHIESTA (v_data.r2d_request, toggle). La MCU
 * possiede il latch "armato" e ne fa l'edge-detection. Il latch si azzera su
 * ogni uscita da RUNNING verso un fault -> il riarmo richiede una pressione
 * fresca (EV.10.4). */
static bool     s_armed          = false;
static bool     s_r2d_req_prev   = false;
static uint8_t  s_sdc_open_cnt   = 0;      /* debounce SDC: N frame consecutivi "aperto" */
#define SDC_OPEN_DEBOUNCE_FRAMES  3U

void DriveLogicTask(void) {
    Inverters_Init();

#if (ACTIVE_CONTROL_MODE == CONTROL_MODE_SIMULINK)
    driveController_initialize();
#endif

    /* Dopo driveController_initialize(): DriveProfiles_Init() programma i
     * parametri di mappa (T_peak_p, lambda_ref, ...) nel modello. */
    DriveProfiles_Init();
    Plausibility_Init(&s_plaus);

    VehicleData_t v_data;
    uint32_t next_wake = osKernelGetTickCount();
    const uint32_t period_ticks = MS_TO_TICKS(DRIVE_LOGIC_PERIOD_MS);

    for (;;) {
        VehicleData_GetSnapshot(&v_data);

        /* 1. Monitor di plausibilità (EV.4.7 + overcurrent/overpower + coerenza
         *    coppia/pedale/freno + range sensori + ...). Ogni check e' debounced;
         *    un qualsiasi bit attivo -> IMPLAUSIBILITY. */
        bool was_running = (v_data.fsm_state == VEHICLE_STATE_RUNNING);
        uint32_t plaus_faults = Plausibility_Update(&s_plaus, &v_data, was_running);
        v_data.plausibility_faults = plaus_faults;

        bool safety_ok = (plaus_faults == 0U) && !v_data.sensor_fault_flag;

        /* Timeout di comunicazione CAN su una qualsiasi scheda */
        bool can_error = v_data.inv_timeout_flag  || v_data.bms_timeout_flag ||
                         v_data.dash_timeout_flag || v_data.pdm_timeout_flag ||
                         v_data.imu_timeout_flag;

        /* --- SDC con debounce: N frame consecutivi "aperto" prima di crederci --- */
        if (!v_data.bms_sdc_status) {
            if (s_sdc_open_cnt < SDC_OPEN_DEBOUNCE_FRAMES) s_sdc_open_cnt++;
        } else {
            s_sdc_open_cnt = 0U;
        }
        bool sdc_closed = (s_sdc_open_cnt < SDC_OPEN_DEBOUNCE_FRAMES);

        /* --- R2D: la dash manda una RICHIESTA (toggle). La MCU possiede il latch.
         * Arma solo su fronte di salita, stando in READY (un pulsante premuto
         * durante un fault non arma da solo) E col freno premuto nell'istante
         * della pressione (EV.10.4 / RTDS: il freno deve essere gia' attivo
         * quando si preme il pulsante, non premuto dopo). Al banco
         * (R2D_BYPASS_ARM_CONDITIONS) il vincolo freno e' bypassato.
         * Disarma su fronte di discesa da qualsiasi stato. */
#if (R2D_BYPASS_ARM_CONDITIONS == 1U)
        const bool arm_edge_brake_ok = true;
#else
        const bool arm_edge_brake_ok = v_data.brake_is_pressed;
#endif
        bool r2d_req = v_data.r2d_request && !v_data.dash_timeout_flag;
        if (r2d_req && !s_r2d_req_prev &&
            (v_data.fsm_state == VEHICLE_STATE_READY) && arm_edge_brake_ok) {
            s_armed = true;
        } else if (!r2d_req && s_r2d_req_prev) {
            s_armed = false;
        }
        s_r2d_req_prev = r2d_req;

        /* --- Condizioni FSM (con override BANCO PROVA) --- */
#if (R2D_IGNORE_TS_SHUTDOWN == 1U)
        const bool can_err_fsm = false;
        const bool sdc_ok_fsm  = true;
        (void)can_error; (void)sdc_closed;   /* al banco non entrano nella FSM */
#else
        const bool can_err_fsm = can_error;
        const bool sdc_ok_fsm  = sdc_closed;
#endif
        const bool ts_fault = can_err_fsm || !sdc_ok_fsm;

        bool inv_ready = (g_inverter_left.state == INV_STATE_RUNNING || g_inverter_left.state == INV_STATE_READY) &&
                         (g_inverter_right.state == INV_STATE_RUNNING || g_inverter_right.state == INV_STATE_READY);

        /* Il vincolo "freno premuto" e' gia' applicato sul fronte di arming
         * (EV.10.4): qui non va ripetuto, altrimenti il pilota dovrebbe tenere
         * il freno premuto anche nel ciclo successivo perche' scatti RUNNING. */
#if (R2D_BYPASS_ARM_CONDITIONS == 1U)
        const bool arm_ok = s_armed && inv_ready;
#else
        const bool arm_ok = s_armed && inv_ready && safety_ok && !can_err_fsm &&
                            sdc_ok_fsm;
#endif

        /* 2. Macchina a Stati Veicolo (FSM) */
        switch (v_data.fsm_state) {
            case VEHICLE_STATE_INIT:
#if (R2D_BYPASS_ARM_CONDITIONS == 1U)
                v_data.fsm_state = VEHICLE_STATE_READY;   /* banco: passa comunque, i fault li becca READY */
#else
                if (safety_ok && !can_err_fsm && sdc_ok_fsm) {
                    v_data.fsm_state = VEHICLE_STATE_READY;
                }
#endif
                break;

            case VEHICLE_STATE_READY:
                if (!safety_ok) {
                    v_data.fsm_state = VEHICLE_STATE_IMPLAUSIBILITY;
                } else if (ts_fault) {
                    v_data.fsm_state = VEHICLE_STATE_CAR_ERROR;
                } else if (arm_ok) {
                    v_data.fsm_state = VEHICLE_STATE_RUNNING;
                }
                break;

            case VEHICLE_STATE_RUNNING:
                if (!safety_ok) {
                    v_data.fsm_state = VEHICLE_STATE_IMPLAUSIBILITY;
                } else if (ts_fault) {
                    v_data.fsm_state = VEHICLE_STATE_CAR_ERROR;
                } else if (!s_armed) {                    /* disarmo volontario (2a pressione) */
                    v_data.fsm_state = VEHICLE_STATE_READY;
                }
                break;

            case VEHICLE_STATE_CAR_ERROR:
                if (ErrorRecovery_GetCtx(&g_inverter_left)->retry_count  >= ERROR_RESET_MAX_RETRIES ||
                    ErrorRecovery_GetCtx(&g_inverter_right)->retry_count >= ERROR_RESET_MAX_RETRIES) {
                    v_data.fsm_state = VEHICLE_STATE_ERROR;
                } else if (!can_err_fsm && safety_ok && sdc_ok_fsm) {
                    v_data.fsm_state = VEHICLE_STATE_READY;
                }
                break;

            case VEHICLE_STATE_IMPLAUSIBILITY:
                if (ts_fault) {
                    v_data.fsm_state = VEHICLE_STATE_CAR_ERROR;
                } else if (safety_ok && !can_err_fsm && sdc_ok_fsm &&
                           (v_data.apps_percent < SAFETY_APPS_BRAKE_RESET_APPS_PCT)) {
                    v_data.fsm_state = VEHICLE_STATE_READY;
                }
                break;

            case VEHICLE_STATE_ERROR:
            default:
                /* Uscita solo con reset HW: VehicleData_Init() al riavvio -> INIT. */
                break;
        }

        /* Qualsiasi stato di fault -> disarma: il riarmo richiede sempre una
         * pressione fresca del pulsante stando in READY (EV.10.4). */
        if (v_data.fsm_state == VEHICLE_STATE_IMPLAUSIBILITY ||
            v_data.fsm_state == VEHICLE_STATE_CAR_ERROR ||
            v_data.fsm_state == VEHICLE_STATE_ERROR) {
            s_armed = false;
        }

        v_data.r2d_armed = s_armed;

        /* 3. Mappa attiva dal volante (dati CAN gia' mirrorati in v_data da DataManagerTask).
         *    DriveProfiles_Update() risolve il tipo di mappa, riprogramma il
         *    controllore solo se mappa/parametri sono cambiati, e ad ogni ciclo
         *    aggiorna V_dc_p / I_discharge_max_p / I_charge_max_p col dato reale. */
        const PowertrainLimits_t pwr_limits = {
            .bus_voltage      = (v_data.inv_l_dc_bus_voltage + v_data.inv_r_dc_bus_voltage) * 0.5f,
            .i_discharge_max_a = (float)v_data.bms_max_discharge_a,   /* limite TOTALE pacco */
            .i_charge_max_a    = (float)v_data.bms_max_regen_a,       /* limite TOTALE pacco */
        };
        DriveProfiles_Update((int8_t)v_data.engine_map, v_data.parameters_config, &pwr_limits);
        const ProfileSettings_t *profile = DriveProfiles_GetActive();

        /* 4. Calcolo Coppia (Simulink o Simple TVC) */
        float target_t_left_nm  = 0.0f;
        float target_t_right_nm = 0.0f;
        bool torque_allowed = (v_data.fsm_state == VEHICLE_STATE_RUNNING);

        if (torque_allowed) {
#if (ACTIVE_CONTROL_MODE == CONTROL_MODE_SIMULINK)
            /* Ingressi del modello: aggiornati una volta per ciclo task (100 Hz). */
            driveController_U.Throttle_    = (double)v_data.apps_percent;
            driveController_U.brake_       = (double)v_data.brake_percent;
            driveController_U.Vx           = (double)(v_data.vehicle_speed_kmh / 3.6f);
            /* Il modello vuole l'angolo VOLANTE in gradi: applica internamente
             * rapporto di sterzo (0.238) e deg->rad. Nessuna conversione qui. */
            driveController_U.SteeringAngle = (double)v_data.steering_angle_deg;
            driveController_U.ax           = (double)(v_data.imu_accel_x_raw * 0.001f);
            driveController_U.ay           = (double)(v_data.imu_accel_y_raw * 0.001f);
            driveController_U.yaw_meas     = (float)(v_data.imu_gyro_z_raw * 0.1f * DEG_TO_RAD);
            driveController_U.n_motorRL    = (double)v_data.inv_l_speed_rpm;
            driveController_U.n_motorRR    = (double)v_data.inv_r_speed_rpm;
            driveController_U.Derating_RL  = (v_data.inv_l_status_word & INV_STAT_DERATING) != 0U;
            driveController_U.Derating_RR  = (v_data.inv_r_status_word & INV_STAT_DERATING) != 0U;
            /* lambda_refRL/RR sono impostati da DriveProfiles_Update() al cambio mappa. */

            driveController_step();

            target_t_left_nm  = (float)driveController_Y.T_RL;
            target_t_right_nm = (float)driveController_Y.T_RR;

            /* --- Debug controlli -> shared data ---
             * Le grandezze con scala DBC 0.01 vengono gia' moltiplicate x100
             * qui, cosi' il DataLogger invia direttamente il valore grezzo.
             * NOTA lambda[]: il modello v1.66 espone [refRL, refRR, slipRL,
             * slipRR]; FL/FR qui riportano quindi i riferimenti, RL/RR lo slip
             * reale. Rivedere il modello se servono i 4 slip veri. */
            v_data.k_slip_left   = (int16_t)(driveController_Y.K_slips[0] * 100.0);
            v_data.k_slip_right  = (int16_t)(driveController_Y.K_slips[1] * 100.0);
            v_data.T_max_RL      = (int16_t)(driveController_Y.T_max_RL * 100.0f);
            v_data.T_max_RR      = (int16_t)(driveController_Y.T_max_RR * 100.0f);
            v_data.lambda_ref_RL     = (uint8_t)(driveController_Y.lambda[0] * 100.0);
            v_data.lambda_ref_RR     = (uint8_t)(driveController_Y.lambda[1] * 100.0);
            v_data.lambda_RL     = (uint8_t)(driveController_Y.lambda[2] * 100.0);
            v_data.lambda_RR     = (uint8_t)(driveController_Y.lambda[3] * 100.0);
            v_data.tvc_weight    = (uint8_t)(driveController_Y.TVC_Target_Weight * 100.0);
            v_data.yaw_th        = (int16_t)(driveController_Y.yaw_th * 100.0f);
            v_data.regen_active  = (driveController_Y.regen != 0.0);
            v_data.throttle_real = (int8_t)driveController_Y.Throttle_real;
#elif (ACTIVE_CONTROL_MODE == CONTROL_MODE_SIMPLE_TVC)
            calculate_simple_tvc(v_data.apps_percent, v_data.steering_angle_deg, &target_t_left_nm, &target_t_right_nm);
#else
            target_t_left_nm  = (v_data.apps_percent / 100.0f) * MOTOR_MAX_TORQUE_NM;
            target_t_right_nm = target_t_left_nm;
#endif
        }

        /* Clamp finale secondo la mappa attiva (vale per tutte le modalita' di
         * controllo, anche se il Simulink ha gia' i suoi limiti interni). */
        float t_pos_max = profile->t_peak_nm;
        float t_neg_max = profile->regen_enabled ? -profile->regen_t_max_nm : 0.0f;
        if (target_t_left_nm  > t_pos_max) target_t_left_nm  = t_pos_max;
        if (target_t_right_nm > t_pos_max) target_t_right_nm = t_pos_max;
        if (target_t_left_nm  < t_neg_max) target_t_left_nm  = t_neg_max;
        if (target_t_right_nm < t_neg_max) target_t_right_nm = t_neg_max;

        v_data.torque_left_nm   = target_t_left_nm;
        v_data.torque_right_nm  = target_t_right_nm;

        /* 5. Invio comandi Inverter via FDCAN1 */
        /* Conversione Nm -> Unità CAN AMK (0.1% Mn) */
        int16_t can_tq_cmd_l = (int16_t)((target_t_left_nm  / MOTOR_NOMINAL_TORQUE_NM) * 1000.0f);
        int16_t can_tq_cmd_r = (int16_t)((target_t_right_nm / MOTOR_NOMINAL_TORQUE_NM) * 1000.0f);

        /* Sequenza di accensione HV: parte quando la MCU e' "armata" (s_armed) e
         * l'SDC e' chiuso. Prima di allora Motor_ProcessInverterControl invia
         * control word = 0 (l'inverter resta in IDLE/LV) ma trasmette il frame.
         * Disarmo (2a pressione R2D) o fault -> s_armed=false -> stessa procedura
         * di spegnimento inverter. */
#if (R2D_IGNORE_TS_SHUTDOWN == 1U)
        bool hv_enable = s_armed && (v_data.fsm_state != VEHICLE_STATE_ERROR);
#else
        bool hv_enable = s_armed && sdc_closed && (v_data.fsm_state != VEHICLE_STATE_ERROR);
#endif

        /* Setpoint realmente inviato all'inverter, per debug DataLogger */
        v_data.inv_l_torque_cmd_raw = Motor_ProcessInverterControl(&hfdcan1, &g_inverter_left,
                                          can_tq_cmd_l, torque_allowed, hv_enable);
        v_data.inv_r_torque_cmd_raw = Motor_ProcessInverterControl(&hfdcan1, &g_inverter_right,
                                          can_tq_cmd_r, torque_allowed, hv_enable);

        /* Pubblica solo i campi generati da DriveLogicTask (FSM + coppie):
         * i sensori/telemetria restano di competenza di DataManagerTask. */
        VehicleData_PublishControl(&v_data);

        next_wake += period_ticks;
        osDelayUntil(next_wake);
    }
}