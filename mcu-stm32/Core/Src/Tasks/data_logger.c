/**
 * @file data_logger.c
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief Data Logger Task periodic UART5 diagnostic output for the MCU-ECU
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

#include "Tasks/data_logger.h"
#include "Tasks/readings_manage.h"
#include "Tasks/inverters_manage.h"
#include "mutexes.h"
#include "Inverter/Inverter.h"
#include "cmsis_os.h"
#include "stm32f7xx_hal.h"
#include "Communication/Serial.h"
#include "Communication/Can.h"

#include <stdint.h>

extern CAN_HandleTypeDef hcan2;

/**
 * @brief Target UART channel(s) for all data-logger output.
 *
 * Change to LOG_CH_UART3 or LOG_CH_BOTH to redirect / duplicate output.
 */
#ifdef DATA_COLLECT_MODE
    /* CSV mode: UART5 only, no verbose text */
    #define LOGGER_CHANNEL          LOG_CH_BOTH
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

MotorDriveState_t DataLogger_GetMotorState(void)
{
    return s_motor_state;
}


void DataLoggerTask(void)
{
    /* CMSIS-RTOS: osDelayUntil expects an ABSOLUTE tick */
    uint32_t next_wake          = osKernelGetTickCount();
    const uint32_t period_ticks = MS_TO_TICKS(LOGGER_TASK_PERIOD_MS);

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

#ifdef DATA_COLLECT_MODE
            const Inverter_t *dc_inv = &inv_r_copy;
            const char *dc_inv_label = "INV_R";

            #if (DATA_COLLECT_INV_SIDE == DATA_COLLECT_INV_SIDE_RIGHT)
                dc_inv = &inv_r_copy;
                dc_inv_label = "INV_R";
            #elif (DATA_COLLECT_INV_SIDE == DATA_COLLECT_INV_SIDE_LEFT)
                dc_inv = &inv_l_copy;
                dc_inv_label = "INV_L";
            #else
                #if INVERTER_LEFT_TORQUE_CONTROL_ENABLED && !INVERTER_RIGHT_TORQUE_CONTROL_ENABLED
                    dc_inv = &inv_l_copy;
                    dc_inv_label = "INV_L";
                #endif
            #endif

            /* [APPS]: pedal position, raw ADC, filtered ADC, error state
             * [selected inverter]: state, speed, torque, temperatures, DC voltage
             */
            Serial_Log(LOGGER_CHANNEL,
                "[APPS]: pedal=%u%% | raw=%u filt=%u | err=0x%02X implaus=%d trq_ok=%d | "
                "[%s]: %s | speed=%d RPM | torque=%.1f Nm | "
                "Tmot=%.1fC Tinv=%.1fC Tigbt=%.1fC | DC=%.1fV | Err: %d Info: %d\r\n",
                apps_copy.final_percent,
                apps_copy.sensors[0].raw,
                apps_copy.sensors[0].filtered,
                apps_copy.error_code,
                (int)apps_copy.implausibility_active,
                (int)apps_copy.torque_allowed,
                dc_inv_label,
                Inverter_GetStateName(dc_inv->state),
                dc_inv->speed_rpm,
                (float)dc_inv->torque_value * (float)TORQUE_SCALE_FACTOR,
                (float)dc_inv->motor_temp_degC    / 10.0f,
                (float)dc_inv->inverter_temp_degC / 10.0f,
                (float)dc_inv->igbt_temp_degC     / 10.0f,
                (float)dc_inv->dc_bus_voltage     / 10.0f,
                dc_inv->error_code, dc_inv->error_info_1);

            /*
             * CSV line — column order must match the Python script header:
             *   TempMotor, TempInverter, TempIGBT, Voltage,
             *   Speed, Iq, Id, TorqueMotor, PedalPerc
             *
             * Units:
             *   Temperatures : °C  (raw field is 0.1°C → /10)
             *   Voltage      : V   (raw field is 0.1V  → /10)
             *   Speed        : RPM (direct)
             *   Iq           : A   (raw_torque_current  × Iq_SCALE_FACTOR)
             *   Id           : A   (raw_magnetizing_current × Id_SCALE_FACTOR)
             *   TorqueMotor  : Nm  (torque_value × TORQUE_SCALE_FACTOR)
             *   PedalPerc    : %   (0–100, direct)
             */
            Serial_Log(LOGGER_CHANNEL,
                "%.1f,%.1f,%.1f,%.1f,%d,%.1f,%.1f,%.1f,%u\n",
                (float)dc_inv->motor_temp_degC    / 10.0f,
                (float)dc_inv->inverter_temp_degC / 10.0f,
                (float)dc_inv->igbt_temp_degC     / 10.0f,
				(float)dc_inv->dc_bus_voltage / 10.0f,
                dc_inv->speed_rpm,
                (float)dc_inv->raw_torque_current * Iq_SCALE_FACTOR,
                (float)dc_inv->raw_magnetizing_current * Id_SCALE_FACTOR,
                (float)dc_inv->torque_value * TORQUE_SCALE_FACTOR,
                apps_copy.final_percent);
#else
            Serial_Log(LOGGER_CHANNEL, "--------------------------------------------------------------\r\n");

            // -- APPS --------------------------------------------------------
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

            // -- SAS ---------------------------------------------------------
            Serial_Log(LOGGER_CHANNEL,
                "[SAS ] raw=%u filt=%u angle=%d deg valid=%d err=0x%02X\r\n",
                sas_copy.raw, sas_copy.filtered, sas_copy.angle_deg,
                (int)sas_copy.sensor_valid, sas_copy.error_code);

            // -- Control -----------------------------------------------------
            Serial_Log(LOGGER_CHANNEL,
                "[CTRL] torque_req=%d torque_lim_dyn=%d\r\n",
                torque_req_copy, torque_lim_copy);

            // -- Inverter LEFT -----------------------------------------------
            Serial_Log(LOGGER_CHANNEL,
                "[INV_L] t=%lu | state=%s sw=0x%04X speed=%d RPM torque=%d Iq=%.1f Id=%.1f T_mot=%.1fC T_inv=%.1fC T_igbt=%.1fC DC=%u.%uV pwr=%luW lastRx=%lu ms\r\n",
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
                inv_l_copy.dc_bus_voltage / 10U, inv_l_copy.dc_bus_voltage % 10U,
                inv_l_copy.actual_power,
                HAL_GetTick() - inv_l_copy.last_rx_timestamp_ms);

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

            // -- Inverter RIGHT ----------------------------------------------
            Serial_Log(LOGGER_CHANNEL,
                "[INV_R] t=%lu | state=%s sw=0x%04X speed=%d RPM torque=%d Iq=%.1f Id=%.1f T_mot=%.1fC T_inv=%.1fC T_igbt=%.1fC DC=%u.%uV pwr=%luW lastRx=%lu ms\r\n",
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
                inv_r_copy.dc_bus_voltage / 10U, inv_r_copy.dc_bus_voltage % 10U,
                inv_r_copy.actual_power,
                HAL_GetTick() - inv_r_copy.last_rx_timestamp_ms);

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

            // -- Task timing -------------------------------------------------
            Serial_Log(LOGGER_CHANNEL,
                "[TIMING] InvManage MAX=%lu us Warnings(>900us)=%lu\r\n",
                timing.max_time_us, timing.warning_count);

            // -- Motor state (per inverter, based on actual torque feedback) ---
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


            // ----------------------------------------------------------------
             //  CAN2 TX @ 10 Hz
             // --------------------------------------------------------------

            // 0x002 — INV1 error packet: sent ONLY on active error
            if (inv_l_copy.state == INV_STATE_ERROR)
            {
                CAN_Car_TransmitInverterError(&hcan2, 1U, &inv_l_copy);
            }

            // 0x003 — INV2 error packet: sent ONLY on active error
            if (inv_r_copy.state == INV_STATE_ERROR)
            {
                CAN_Car_TransmitInverterError(&hcan2, 2U, &inv_r_copy);
            }

            // 0x006 — MCU errors byte: sent cyclically
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

            // 0x108 — APPs % and SAS normalised %: sent cyclically
            {
                // Scale SAS angle [SAS_ANGLE_MAX_LEFT..SAS_ANGLE_MAX_RIGHT] -> [-100..+100 %]
                int32_t divisor = (sas_copy.angle_deg >= 0)
                                  ? (int32_t)SAS_ANGLE_MAX_RIGHT
                                  : (int32_t)(-SAS_ANGLE_MAX_LEFT);
                int16_t sas_pct = (int16_t)((int32_t)sas_copy.angle_deg * 100 / divisor);
                if (sas_pct >  100) sas_pct =  100;
                if (sas_pct < -100) sas_pct = -100;
                CAN_Car_TransmitAppsSas(&hcan2, apps_copy.final_percent, (int8_t)sas_pct);
            }

            // 0x250 — INV1 real-time data: sent cyclically
            CAN_Car_TransmitInverterData(&hcan2, 1U, &inv_l_copy);

            // 0x251 — INV2 real-time data: sent cyclically
            CAN_Car_TransmitInverterData(&hcan2, 2U, &inv_r_copy);
#endif /* DATA_COLLECT_MODE */


        } 

        next_wake += period_ticks;
        osDelayUntil(next_wake);
    }
}

