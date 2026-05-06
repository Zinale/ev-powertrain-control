/**
 * @file data_logger.c
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief Data Logger Task periodic UART4 diagnostic output for the MCU-ECU
 *

 *  Data Logger Task
 *  ---------------------------------------------------------------------------
 *  Provides a structured multi-line diagnostic output for the whole vehicle
 *  control system at 10 Hz (100 ms period):
 *   - APPS sensor raw ADC, filtered, percent values + error/plausibility state
 *   - SAS steering angle + validity
 *   - Control: torque request and dynamic limit
 *   - Inverter LEFT/RIGHT: state, speed, torque, temperatures
 *   - InvertersManage task timing statistics
 *   - CAN TX / RX statistics and hardware state
 *
 *  ---------------------------------------------------------------------------
 */

#include "data_manager.h"
#include <motors_manager.h>
#include "Tasks/data_logger.h"
#include "mutexes.h"
#include "Drive/Inverter.h"
#include "cmsis_os.h"
#include "stm32f7xx_hal.h"
#include "Communication/Serial.h"
#include "Communication/Can.h"
#include "Safety/MCU_State.h"

#include <stdint.h>

extern CAN_HandleTypeDef hcan2;

/**
 * @brief Target UART channel(s) for all data-logger output.
 *
 * Change to LOG_CH_UART3 or LOG_CH_BOTH to redirect / duplicate output.
 */
#ifdef DATA_COLLECT_MODE
    #if (DATA_COLLECT_BACKEND == DATA_COLLECT_BACKEND_FEATHER_LOCAL)
        /* Feather logger is connected on UART4: keep UART4 strictly data-only. */
        #define LOGGER_CHANNEL      LOG_CH_UART4
    #else
        /* ESP32_REMOTE: UART4 goes directly to the ESP32 bridge. */
        #define LOGGER_CHANNEL      LOG_CH_UART4
    #endif
    #define LOGGER_TASK_PERIOD_MS   DATA_COLLECT_PERIOD_MS  
#else
    #define LOGGER_CHANNEL          LOG_CH_BOTH
    #define LOGGER_TASK_PERIOD_MS   100U                   /**< 0.1 s = 10 Hz   */
#endif

#define LOGGER_PRINT_PERIOD     1U

/** Convert ms to RTOS ticks (portable, requires kernel already started) */
#define MS_TO_TICKS(ms) ((uint32_t)((ms) * osKernelGetTickFreq() / 1000U))

static uint32_t       cycle_counter = 0U;               /**< Incremented every task activation */
static MotorDriveState_t s_motor_state = MOTOR_STATE_IDLE; /**< Last computed drive state         */

#if defined(DATA_COLLECT_MODE) && (DATA_COLLECT_BACKEND == DATA_COLLECT_BACKEND_FEATHER_LOCAL)
static uint8_t  s_feather_logging_active = 0U;
static uint32_t s_feather_start_tick_ms = 0U;
static InverterState_t s_last_inv_state_right = INV_STATE_OFF;  /**< Track state transitions for debug logging */
static InverterState_t s_last_inv_state_left = INV_STATE_OFF;   /**< Track state transitions for debug logging */
#endif

#if defined(DATA_COLLECT_MODE) && (DATA_COLLECT_BACKEND == DATA_COLLECT_BACKEND_ESP32_REMOTE)
static uint8_t  s_esp32_logging_active = 0U;
static uint32_t s_esp32_start_tick_ms = 0U;
#endif

MotorDriveState_t DataLogger_GetMotorState(void)
{
    return s_motor_state;
}


void DataLoggerTask(void)
{
    /* CMSIS-RTOS: osDelayUntil expects an ABSOLUTE tick */
    uint32_t next_wake          = osKernelGetTickCount();
    const uint32_t period_ticks = MS_TO_TICKS(LOGGER_TASK_PERIOD_MS);

    /* Startup beacon: confirms DataLoggerTask is alive and UART4 DMA works.
     * If this line is received the hardware path is functional.             */
    Serial_Log(LOG_CH_UART4, "[BOOT] DataLogger started t=%lu\r\n",
               (unsigned long)HAL_GetTick());

    for (;;)
    {
        /* Increment counter at the START of the loop */
        cycle_counter++;

        if ((cycle_counter % LOGGER_PRINT_PERIOD) == 0U)
        {
            APPS_Data_t apps_copy;
            uint16_t    apps_raw_copy[APPS_NUM_SENSORS];

            Mutex_APPS_Lock();
            Readings_GetApps(&apps_copy);
            Readings_GetAppsRaw(apps_raw_copy);
            Mutex_APPS_Unlock();

            SAS_Data_t sas_copy;

            Mutex_SAS_Lock();
            Readings_GetSAS(&sas_copy);
            Mutex_SAS_Unlock();

            Inverter_t         inv_l_copy;
            InvErrorSnapshot_t err_snap_l;

            Mutex_INVERTER_L_Lock();
            InvertersManage_GetLeft(&inv_l_copy);
            InvertersManage_GetErrorSnapshot(&err_snap_l);
            Mutex_INVERTER_L_Unlock();

            Inverter_t         inv_r_copy;
            InvErrorSnapshot_t err_snap_r;

            Mutex_INVERTER_R_Lock();
            InvertersManage_GetRight(&inv_r_copy);
            InvertersManage_GetErrorSnapshotRight(&err_snap_r);
            Mutex_INVERTER_R_Unlock();

            int16_t torque_req_copy;
            int16_t torque_lim_copy;
            InvertersManage_GetControlSignals(&torque_req_copy, &torque_lim_copy);

            InvTimingStats_t timing;
            InvertersManage_GetTimingStats(&timing);

            if (g_can1_busoff) {
                Serial_Log(LOGGER_CHANNEL, "[CAN1 BUS-OFF] TEC=%u REC=%u ESR=0x%08lX events=%lu\n",
                    g_can1_sce_tec, g_can1_sce_rec, g_can1_sce_esr, g_can1_sce_count);
                g_can1_busoff = 0U;  // reset dopo averlo loggato
            }

            HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin); 

#ifdef DATA_COLLECT_MODE
            const Inverter_t *dc_inv = &inv_r_copy;
            const InvErrorSnapshot_t *dc_err_snap = &err_snap_r;
            const char *dc_inv_label = "INV_R";

            #if (DATA_COLLECT_INV_SIDE == DATA_COLLECT_INV_SIDE_RIGHT)
                dc_inv = &inv_r_copy;
                dc_err_snap = &err_snap_r;
                dc_inv_label = "INV_R";
            #elif (DATA_COLLECT_INV_SIDE == DATA_COLLECT_INV_SIDE_LEFT)
                dc_inv = &inv_l_copy;
                dc_err_snap = &err_snap_l;
                dc_inv_label = "INV_L";
            #else
                #if INVERTER_LEFT_TORQUE_CONTROL_ENABLED && !INVERTER_RIGHT_TORQUE_CONTROL_ENABLED
                    dc_inv = &inv_l_copy;
                    dc_err_snap = &err_snap_l;
                    dc_inv_label = "INV_L";
                #endif
            #endif

            #if (DATA_COLLECT_BACKEND == DATA_COLLECT_BACKEND_FEATHER_LOCAL)
                /**
                 * Track inverter state transitions and log them for debugging.
                 * Logging starts when inverter reaches LV_ACTIVE or higher.
                 */
                InverterState_t *p_last_state = (dc_inv == &inv_r_copy) ? &s_last_inv_state_right : &s_last_inv_state_left;
                
                /* Detect state transition */
                if (dc_inv->state != *p_last_state) {
                    Serial_Log(LOGGER_CHANNEL, "[STATE_TRANS] %s: %s -> %s (t=%lu ms)\n",
                        dc_inv_label,
                        Inverter_GetStateName(*p_last_state),
                        Inverter_GetStateName(dc_inv->state),
                        (unsigned long)HAL_GetTick());
                    *p_last_state = dc_inv->state;
                }
                
                /* Check if inverter is in active state (LV_ACTIVE or higher) */
                const uint8_t inverter_active = (dc_inv->state >= INV_STATE_LV_ACTIVE && dc_inv->state != INV_STATE_ERROR) ? 1U : 0U;
                const uint8_t inverter_error = (dc_inv->state == INV_STATE_ERROR) ? 1U : 0U;

                if (inverter_active && (s_feather_logging_active == 0U)) {
                    Serial_Log(LOGGER_CHANNEL, "[START]\n");
                    s_feather_logging_active = 1U;
                    s_feather_start_tick_ms = HAL_GetTick();
                }

                if (s_feather_logging_active != 0U) {
                    if (inverter_error || !inverter_active) {
                        if (inverter_error) {
                            const uint32_t now_ms = HAL_GetTick();
                            Serial_Log(LOGGER_CHANNEL,
                                "[ERROR] src=%s code=%u info1=%lu snap_t=%lu age=%lu dc=%u.%uV speed=%d torque=%d\n",
                                dc_inv_label,
                                dc_inv->error_code,
                                (unsigned long)dc_inv->error_info_1,
                                (unsigned long)dc_err_snap->timestamp_ms,
                                (unsigned long)(now_ms - dc_err_snap->timestamp_ms),
                                dc_err_snap->dc_voltage / 10U,
                                dc_err_snap->dc_voltage % 10U,
                                dc_err_snap->speed,
                                dc_err_snap->torque);
                        }
                        Serial_Log(LOGGER_CHANNEL, "[STOP]\n");
                        s_feather_logging_active = 0U;
                    } else {
                        const uint32_t elapsed_ms = HAL_GetTick() - s_feather_start_tick_ms;
                        /*
                         * CSV line — colonne (Feather appende NTC1,NTC2,NTC3 dopo):
                         *   Time_s, TempMotor, TempInverter, TempIGBT, Voltage,
                         *   Speed, Iq, Id, TorqueMotor, PedalPerc,
                         *   StatusWord, ErrCode, ErrInfo1,
                         *   PhaseU_mA, PhaseV_mA, PhaseW_mA, Power_W
                         */
                        Serial_Log(LOGGER_CHANNEL,
                            "%.1f,%.1f,%.1f,%.1f,%d,%d,%.1f,%.1f,%.1f,%u,%u,%u,%lu,%ld,%ld,%ld,%lu\n",
                            elapsed_ms / 1000.0f,
                            (float)dc_inv->motor_temp_degC    / 10.0f,
                            (float)dc_inv->inverter_temp_degC / 10.0f,
                            (float)dc_inv->igbt_temp_degC     / 10.0f,
                            dc_inv->dc_bus_voltage,
                            dc_inv->speed_rpm,
                            (float)dc_inv->raw_torque_current * Iq_SCALE_FACTOR,
                            (float)dc_inv->raw_magnetizing_current * Id_SCALE_FACTOR,
                            (float)dc_inv->torque_value * TORQUE_SCALE_FACTOR,
                            apps_copy.final_percent,
                            (unsigned)dc_inv->status_word,
                            (unsigned)dc_inv->error_code,
                            (unsigned long)dc_inv->error_info_1,
                            (long)dc_inv->phase_u_current,
                            (long)dc_inv->phase_v_current,
                            (long)dc_inv->phase_w_current,
                            (unsigned long)dc_inv->actual_power);
                    }
                }
            #else
                const uint8_t inverter_running = (dc_inv->state == INV_STATE_RUNNING) ? 1U : 0U;

                if (inverter_running && (s_esp32_logging_active == 0U)) {
                    s_esp32_logging_active = 1U;
                    s_esp32_start_tick_ms = HAL_GetTick();
                } else if (!inverter_running) {
                    s_esp32_logging_active = 0U;
                }

                const uint32_t elapsed_ms = (s_esp32_logging_active != 0U)
                                            ? (HAL_GetTick() - s_esp32_start_tick_ms)
                                            : 0U;

                /* Verbose [APPS] line: only every 10 cycles to reduce BT traffic.
                 * The CSV line below carries all data every cycle.
                 */
                if ((cycle_counter % 10U) == 0U) {
                    Serial_Log(LOGGER_CHANNEL,
                        "[%s]: %s | p=%u%% spd=%d T=%.1f DC=%dV E:%d\r\n",
                        dc_inv_label,
                        Inverter_GetStateName(dc_inv->state),
                        apps_copy.final_percent,
                        dc_inv->speed_rpm,
                        (float)dc_inv->torque_value * (float)TORQUE_SCALE_FACTOR,
                        (int)dc_inv->dc_bus_voltage,
                        dc_inv->error_code);
                }

                /*
                 * CSV line — column order must match the Python script header:
                 *   ElapsedMs, TempMotor, TempInverter, TempIGBT, Voltage,
                 *   Speed, Iq, Id, TorqueMotor, PedalPerc, InvState, ErrCode,
                 *   StatusWord, ErrInfo1, PhaseU_mA, PhaseV_mA, PhaseW_mA, Power_W,
                 *   TorqueSetpoint, TorqueLimitDyn
                 */
                Serial_Log(LOGGER_CHANNEL,
                    "%lu,%.1f,%.1f,%.1f,%d,%d,%.1f,%.1f,%.1f,%u,%d,%u,%u,%lu,%ld,%ld,%ld,%lu,%d,%d\n",
                    (unsigned long)elapsed_ms,
                    (float)dc_inv->motor_temp_degC    / 10.0f,
                    (float)dc_inv->inverter_temp_degC / 10.0f,
                    (float)dc_inv->igbt_temp_degC     / 10.0f,
                    (int)dc_inv->dc_bus_voltage,
                    dc_inv->speed_rpm,
                    (float)dc_inv->raw_torque_current * Iq_SCALE_FACTOR,
                    (float)dc_inv->raw_magnetizing_current * Id_SCALE_FACTOR,
                    (float)dc_inv->torque_value * TORQUE_SCALE_FACTOR,
                    apps_copy.final_percent,
                    (int)dc_inv->state,
                    (unsigned)dc_inv->error_code,
                    (unsigned)dc_inv->status_word,
                    (unsigned long)dc_inv->error_info_1,
                    (long)dc_inv->phase_u_current,
                    (long)dc_inv->phase_v_current,
                    (long)dc_inv->phase_w_current,
                    (unsigned long)dc_inv->actual_power,
                    (int)dc_inv->torque_request,
                    (int)torque_lim_copy);
            #endif
#else
            /* ---- State-aware serial output ------------------------------------------ */
            const MCU_State_t mcu_state = MCU_State_Get();

            switch (mcu_state)
            {
                /* ----------------------------------------------------------------------- */
                /* READY / RUNNING: full diagnostic output + all CAN2 TX                  */
                /* ----------------------------------------------------------------------- */
                case MCU_STATE_READY:
                case MCU_STATE_RUNNING:
                {
                    Serial_Log(LOGGER_CHANNEL, "--------------------------------------------------------------\r\n");
                    Serial_Log(LOGGER_CHANNEL,
                        "--- MCU: %s (t=%lu ms) ---\r\n",
                        MCU_State_ToString(mcu_state), HAL_GetTick());

                    // -- APPS ------------------------------------------------------------
                    Serial_Log(LOGGER_CHANNEL,
                        "[APPS] ADC0=%u ADC1=%u | "
                        "raw0=%u filt0=%u pct0=%u%% | "
                        "raw1=%u filt1=%u pct1=%u%% | "
                        "final=%u%% err=0x%02X implaus=%d trq_ok=%d\r\n",
                        apps_raw_copy[0], apps_raw_copy[1],
                        apps_copy.sensors[0].raw,      apps_copy.sensors[0].filtered,
                        apps_copy.sensors[0].percent,
                        apps_copy.sensors[1].raw,      apps_copy.sensors[1].filtered,
                        apps_copy.sensors[1].percent,
                        apps_copy.final_percent,
                        apps_copy.error_code,
                        (int)apps_copy.implausibility_active,
                        (int)apps_copy.torque_allowed);

                    // -- SAS -------------------------------------------------------------
                    Serial_Log(LOGGER_CHANNEL,
                        "[SAS ] raw=%u filt=%u angle=%d deg valid=%d err=0x%02X\r\n",
                        sas_copy.raw, sas_copy.filtered, sas_copy.angle_deg,
                        (int)sas_copy.sensor_valid, sas_copy.error_code);

                    // -- Control ---------------------------------------------------------
                    Serial_Log(LOGGER_CHANNEL,
                        "[CTRL] torque_req=%d torque_lim_dyn=%d\r\n",
                        torque_req_copy, torque_lim_copy);

                    // -- Inverter LEFT ---------------------------------------------------
                    Serial_Log(LOGGER_CHANNEL,
                        "[INV_L] t=%lu | state=%s sw=0x%04X speed=%d RPM torque=%d Iq=%.1f Id=%.1f T_mot=%.1fC T_inv=%.1fC T_igbt=%.1fC DC=%dV pwr=%luW lastRx=%lu ms\r\n",
                        HAL_GetTick(),
                        Inverter_GetStateName(inv_l_copy.state),
                        inv_l_copy.status_word,
                        inv_l_copy.speed_rpm,
                        inv_l_copy.torque_value * TORQUE_SCALE_FACTOR,
                        inv_l_copy.raw_torque_current * Iq_SCALE_FACTOR,
                        inv_l_copy.raw_magnetizing_current * Id_SCALE_FACTOR,
                        (float)inv_l_copy.motor_temp_degC    / 10.0f,
                        (float)inv_l_copy.inverter_temp_degC / 10.0f,
                        (float)inv_l_copy.igbt_temp_degC     / 10.0f,
                        inv_l_copy.dc_bus_voltage,
                        inv_l_copy.actual_power,
                        HAL_GetTick() - inv_l_copy.last_rx_timestamp_ms);

                    Serial_Log(LOGGER_CHANNEL,
                        "[SW_L ] 0x%04X | SysRdy=%d Err=%d Warn=%d QDCon=%d DCon=%d QInvOn=%d InvOn=%d Der=%d\r\n",
                        inv_l_copy.status_word,
                        !!(inv_l_copy.status_word & INV_STAT_SYSTEM_READY),
                        !!(inv_l_copy.status_word & INV_STAT_ERROR),
                        !!(inv_l_copy.status_word & INV_STAT_WARN),
                        !!(inv_l_copy.status_word & INV_STAT_QUIT_DC_ON),
                        !!(inv_l_copy.status_word & INV_STAT_DC_ON),
                        !!(inv_l_copy.status_word & INV_STAT_QUIT_INV_ON),
                        !!(inv_l_copy.status_word & INV_STAT_INVERTER_ON),
                        !!(inv_l_copy.status_word & INV_STAT_DERATING));

                    if (inv_l_copy.state == INV_STATE_ERROR)
                    {
                        Serial_Log(LOGGER_CHANNEL,
                            "[INV_L ERROR] code=%u at t=%lu (+%lu ms ago)\r\n",
                            inv_l_copy.error_code,
                            err_snap_l.timestamp_ms,
                            HAL_GetTick() - err_snap_l.timestamp_ms);
                        Serial_Log(LOGGER_CHANNEL,
                            "  Conditions: DC=%u.%uV torque=%d speed=%d RPM%s\r\n",
                            err_snap_l.dc_voltage / 10U, err_snap_l.dc_voltage % 10U,
                            err_snap_l.torque, err_snap_l.speed,
                            (err_snap_l.dc_voltage > 4000U) ? " HIGH VOLTAGE!" : "");
                    }

                    // -- Inverter RIGHT --------------------------------------------------
                    Serial_Log(LOGGER_CHANNEL,
                        "[INV_R] t=%lu | state=%s sw=0x%04X speed=%d RPM torque=%d Iq=%.1f Id=%.1f T_mot=%.1fC T_inv=%.1fC T_igbt=%.1fC DC=%dV pwr=%luW lastRx=%lu ms\r\n",
                        HAL_GetTick(),
                        Inverter_GetStateName(inv_r_copy.state),
                        inv_r_copy.status_word,
                        inv_r_copy.speed_rpm,
                        inv_r_copy.torque_value * TORQUE_SCALE_FACTOR,
                        inv_r_copy.raw_torque_current * Iq_SCALE_FACTOR,
                        inv_r_copy.raw_magnetizing_current * Id_SCALE_FACTOR,
                        (float)inv_r_copy.motor_temp_degC    / 10.0f,
                        (float)inv_r_copy.inverter_temp_degC / 10.0f,
                        (float)inv_r_copy.igbt_temp_degC     / 10.0f,
                        inv_r_copy.dc_bus_voltage,
                        inv_r_copy.actual_power,
                        HAL_GetTick() - inv_r_copy.last_rx_timestamp_ms);

                    Serial_Log(LOGGER_CHANNEL,
                        "[SW_R ] 0x%04X | SysRdy=%d Err=%d Warn=%d QDCon=%d DCon=%d QInvOn=%d InvOn=%d Der=%d\r\n",
                        inv_r_copy.status_word,
                        !!(inv_r_copy.status_word & INV_STAT_SYSTEM_READY),
                        !!(inv_r_copy.status_word & INV_STAT_ERROR),
                        !!(inv_r_copy.status_word & INV_STAT_WARN),
                        !!(inv_r_copy.status_word & INV_STAT_QUIT_DC_ON),
                        !!(inv_r_copy.status_word & INV_STAT_DC_ON),
                        !!(inv_r_copy.status_word & INV_STAT_QUIT_INV_ON),
                        !!(inv_r_copy.status_word & INV_STAT_INVERTER_ON),
                        !!(inv_r_copy.status_word & INV_STAT_DERATING));

                    if (inv_r_copy.state == INV_STATE_ERROR)
                    {
                        Serial_Log(LOGGER_CHANNEL,
                            "[INV_R ERROR] code=%u at t=%lu (+%lu ms ago)\r\n",
                            inv_r_copy.error_code,
                            err_snap_r.timestamp_ms,
                            HAL_GetTick() - err_snap_r.timestamp_ms);
                        Serial_Log(LOGGER_CHANNEL,
                            "  Conditions: DC=%u.%uV torque=%d speed=%d RPM%s\r\n",
                            err_snap_r.dc_voltage / 10U, err_snap_r.dc_voltage % 10U,
                            err_snap_r.torque, err_snap_r.speed,
                            (err_snap_r.dc_voltage > 4000U) ? " HIGH VOLTAGE!" : "");
                    }

                    // -- Task timing -----------------------------------------------------
                    Serial_Log(LOGGER_CHANNEL,
                        "[TIMING] InvManage MAX=%lu us Warnings(>900us)=%lu\r\n",
                        timing.max_time_us, timing.warning_count);

                    // -- Motor state (per inverter, based on actual torque feedback) ------
                    MotorDriveState_t state_l, state_r;

                    if (inv_l_copy.state == INV_STATE_RUNNING && inv_l_copy.torque_value < 0)
                        state_l = MOTOR_STATE_REGEN;
                    else if (inv_l_copy.state == INV_STATE_RUNNING && inv_l_copy.torque_value > 0)
                        state_l = MOTOR_STATE_DRIVE;
                    else
                        state_l = MOTOR_STATE_IDLE;

                    if (inv_r_copy.state == INV_STATE_RUNNING && inv_r_copy.torque_value < 0)
                        state_r = MOTOR_STATE_REGEN;
                    else if (inv_r_copy.state == INV_STATE_RUNNING && inv_r_copy.torque_value > 0)
                        state_r = MOTOR_STATE_DRIVE;
                    else
                        state_r = MOTOR_STATE_IDLE;

                    if (state_l == MOTOR_STATE_REGEN || state_r == MOTOR_STATE_REGEN)
                        s_motor_state = MOTOR_STATE_REGEN;
                    else if (state_l == MOTOR_STATE_DRIVE || state_r == MOTOR_STATE_DRIVE)
                        s_motor_state = MOTOR_STATE_DRIVE;
                    else
                        s_motor_state = MOTOR_STATE_IDLE;

                    {
                        const char *name_l = (state_l == MOTOR_STATE_REGEN) ? "REGEN" :
                                             (state_l == MOTOR_STATE_DRIVE)  ? "DRIVE" : "IDLE";
                        const char *name_r = (state_r == MOTOR_STATE_REGEN) ? "REGEN" :
                                             (state_r == MOTOR_STATE_DRIVE)  ? "DRIVE" : "IDLE";
                        float torque_l_nm = ((float)inv_l_copy.torque_value / 1000.0f) * 9.8f;
                        float torque_r_nm = ((float)inv_r_copy.torque_value / 1000.0f) * 9.8f;
                        Serial_Log(LOGGER_CHANNEL,
                            "[MSTATE] | L:%s torqueRequest=%d torque=%d (%.2f Nm) speed=%d RPM | "
                            "R:%s torqueRequest=%d torque=%d (%.2f Nm) speed=%d RPM\r\n",
                            name_l, inv_l_copy.torque_request, inv_l_copy.torque_value, torque_l_nm, inv_l_copy.speed_rpm,
                            name_r, inv_r_copy.torque_request, inv_r_copy.torque_value, torque_r_nm, inv_r_copy.speed_rpm);
                    }

                    // -- CAN2 TX full @ 10 Hz --------------------------------------------
                    if (inv_l_copy.state == INV_STATE_ERROR)
                        CAN_Car_TransmitInverterError(&hcan2, 1U, &inv_l_copy);

                    if (inv_r_copy.state == INV_STATE_ERROR)
                        CAN_Car_TransmitInverterError(&hcan2, 2U, &inv_r_copy);

                    {
                        uint8_t err_byte = 0U;
                        if (inv_l_copy.status_word & INV_STAT_DERATING)
                            err_byte |= CAN_ERR_INV1_DERATING;
                        if (inv_r_copy.status_word & INV_STAT_DERATING)
                            err_byte |= CAN_ERR_INV2_DERATING;
                        if (apps_copy.implausibility_active)
                            err_byte |= CAN_ERR_APPS_IMPLAUS;
                        if (s_motor_state == MOTOR_STATE_REGEN)
                            err_byte |= CAN_ERR_REGEN_ACTIVE;
                        CAN_Car_TransmitErrors(&hcan2, err_byte);
                    }

                    {
                        int32_t divisor = (sas_copy.angle_deg >= 0)
                                          ? (int32_t)SAS_ANGLE_MAX_RIGHT
                                          : (int32_t)(-SAS_ANGLE_MAX_LEFT);
                        int16_t sas_pct = (int16_t)((int32_t)sas_copy.angle_deg * 100 / divisor);
                        if (sas_pct >  100) sas_pct =  100;
                        if (sas_pct < -100) sas_pct = -100;
                        CAN_Car_TransmitAppsSas(&hcan2, apps_copy.final_percent, (int8_t)sas_pct);
                    }

                    CAN_Car_TransmitInverterData(&hcan2, 1U, &inv_l_copy);
                    CAN_Car_TransmitInverterData(&hcan2, 2U, &inv_r_copy);
                    break;
                }

                /* ----------------------------------------------------------------------- */
                /* IMPLAUSIBILITY: sensor details + 0x006 only                             */
                /* ----------------------------------------------------------------------- */
                case MCU_STATE_IMPLAUSIBILITY:
                {
                    Serial_Log(LOGGER_CHANNEL,
                        "--- MCU: IMPLAUSIBILITY (t=%lu ms, in state %lu ms) ---\r\n",
                        HAL_GetTick(), HAL_GetTick() - MCU_State_GetEntryTick());

                    Serial_Log(LOGGER_CHANNEL,
                        "[APPS] final=%u%% err=0x%02X implaus=%d trq_ok=%d\r\n",
                        apps_copy.final_percent, apps_copy.error_code,
                        (int)apps_copy.implausibility_active, (int)apps_copy.torque_allowed);

                    Serial_Log(LOGGER_CHANNEL,
                        "[SAS ] angle=%d deg valid=%d err=0x%02X\r\n",
                        sas_copy.angle_deg, (int)sas_copy.sensor_valid, sas_copy.error_code);

                    Serial_Log(LOGGER_CHANNEL,
                        "[CAN ] CAN1=%s CAN2=%s\r\n",
                        (CAN_GetCan1State() == CAN_RUNTIME_READY) ? "OK" : "FAIL",
                        (CAN_GetCan2State() == CAN_RUNTIME_READY) ? "OK" : "FAIL");

                    /* CAN2 TX: only 0x006 to signal implausibility to telemetry */
                    CAN_Car_TransmitErrors(&hcan2, CAN_ERR_APPS_IMPLAUS);
                    break;
                }

                /* ----------------------------------------------------------------------- */
                /* ERROR: CAN diagnostics only + 0x006                                     */
                /* ----------------------------------------------------------------------- */
                case MCU_STATE_ERROR:
                {
                    Serial_Log(LOGGER_CHANNEL,
                        "--- MCU: ERROR (t=%lu ms, in state %lu ms) ---\r\n",
                        HAL_GetTick(), HAL_GetTick() - MCU_State_GetEntryTick());

                    Serial_Log(LOGGER_CHANNEL,
                        "[CAN ] CAN1=%s (err=0x%lX) CAN2=%s | sce_events=%lu TEC=%u REC=%u ESR=0x%08lX\r\n",
                        (CAN_GetCan1State() == CAN_RUNTIME_READY) ? "OK" : "FAIL",
                        g_can1_last_error,
                        (CAN_GetCan2State() == CAN_RUNTIME_READY) ? "OK" : "FAIL",
                        g_can1_sce_count, g_can1_sce_tec, g_can1_sce_rec, g_can1_sce_esr);

                    /* CAN2 TX: only 0x006 — no sensor / inverter data */
                    CAN_Car_TransmitErrors(&hcan2, 0U);
                    break;
                }

                /* ----------------------------------------------------------------------- */
                /* INIT: heartbeat only                                                     */
                /* ----------------------------------------------------------------------- */
                case MCU_STATE_INIT:
                default:
                {
                    Serial_Log(LOGGER_CHANNEL,
                        "[MCU] INIT t=%lu ms\r\n", HAL_GetTick());
                    break;
                }
            }
#endif /* DATA_COLLECT_MODE */


        } 

        next_wake += period_ticks;
        osDelayUntil(next_wake);
    }
}

