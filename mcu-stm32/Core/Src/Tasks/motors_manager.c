/**
 * @file inverters_manage.c
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief Inverters Manager Task — inverter control loop and error diagnostics
 *
 * =============================================================================
 *  Inverters Manager Task
 *  ---------------------------------------------------------------------------
 *  Runs the inverter control loop at 1 kHz (1 ms period), previously done
 *  by the bare-metal TaskFast:
 *
 *   1. Read APPS output (pedal percent) under Mutex_APPS
 *   2. Update status LEDs based on inverter state
 *   3. Execute Motor_ProcessInverterControl() for LEFT  inverter
 *      (CAN command transmission) under Mutex_INVERTER_L + Mutex_CAN1
 *   4. Detect INV_STATE_ERROR transitions and freeze diagnostics snapshot (LEFT)
 *   5. Execute Motor_ProcessInverterControl() for RIGHT inverter
 *      (CAN command transmission) under Mutex_INVERTER_R + Mutex_CAN1
 *   6. Detect INV_STATE_ERROR transitions and freeze diagnostics snapshot (RIGHT)
 *   7. Track worst-case execution time for monitoring
 *
 *  Ownership of error snapshot variables
 *  ---------------------------------------------------------------------------
 *  s_error_[l|r]_* variables are defined here.  BaseControlMotor.c externs
 *  the left-inverter symbols (same names, same linkage) — no change needed there.
 * =============================================================================
 */

#include <motors_manager.h>
#include "mutexes.h"
#include "cmsis_os.h"
#include "stm32f7xx_hal.h"
#include "watchdog.h"
#include "Sensors/APPS.h"
#include "Drive/Inverter.h"
#include "Drive/BaseControlMotor.h"
#include "Communication/Can.h"
#include "Safety/MCU_State.h"

#include <stdint.h>
#include <string.h>

/** Task period — must match CONTROL_LOOP_PERIOD_MS in Config.h */
#define INVERTERS_TASK_PERIOD_MS   CONTROL_LOOP_PERIOD_MS

/** Execution-time threshold for a warning  [µs] */
#define INVERTERS_TIMING_WARNING_US  900U

/** Convert ms to RTOS ticks (portable, requires kernel already started) */
#define MS_TO_TICKS(ms) ((uint32_t)((ms) * osKernelGetTickFreq() / 1000U))


#define WATCHDOG_REFRESH_PERIOD 100U
static uint32_t cycle_counter = 0U;

extern CAN_HandleTypeDef  hcan1;
extern Inverter_t         g_inverter_left;
extern Inverter_t         g_inverter_right;
extern APPS_Data_t        g_apps;
extern volatile int16_t   torque_request;    /**< volatile: written here, read by DataLoggerTask */
extern volatile int16_t   torque_limit_dyn;  /**< volatile: written here, read by DataLoggerTask */
extern uint32_t           g_can_tx_ok_count;
extern uint32_t           g_can_tx_fail_count;
extern uint32_t           g_can_rx_count;



/**
 * @brief Previous torque setpoints for rate-limiters — one per inverter.
 */
static int16_t s_torque_setpoint_prev_left  = 0;
static int16_t s_torque_setpoint_prev_right = 0;

/**
 * @brief Previous inverter states — used to detect ERROR transitions.
 */
static InverterState_t s_prev_inv_state_left  = INV_STATE_OFF;
static InverterState_t s_prev_inv_state_right = INV_STATE_OFF;

/**
 * @brief Execution time tracking (worst-case per log window).
 *        Written only by this task; read atomically on Cortex-M7.
 */
static volatile uint32_t s_max_time_us    = 0U;
static volatile uint32_t s_warning_count  = 0U;
/* 
 *  ERROR SNAPSHOT
 *  Defined here so that the BaseControlMotor.c `extern` declarations resolve
 *  to these symbols (same names, same linkage).
 */
/* LEFT inverter error snapshot (externs resolved by BaseControlMotor.c) */
uint32_t s_error_l_timestamp  = 0U;
uint16_t s_error_l_dc_voltage = 0U;
int16_t  s_error_l_torque     = 0;
int16_t  s_error_l_speed      = 0;

/* RIGHT inverter error snapshot */
uint32_t s_error_r_timestamp  = 0U;
uint16_t s_error_r_dc_voltage = 0U;
int16_t  s_error_r_torque     = 0;
int16_t  s_error_r_speed      = 0;

void MotorsManagerTask(void)
{
    uint32_t next_wake          = osKernelGetTickCount();
    const uint32_t period_ticks = MS_TO_TICKS(INVERTERS_TASK_PERIOD_MS);

    for (;;)
    {
        uint32_t t_start = HAL_GetTick();

        /* 
         *  1 — Read APPS output under mutex 
         */
        Mutex_APPS_Lock();
        uint8_t pedal_percent = g_apps.torque_allowed ? g_apps.final_percent : 0U;
        /* Double safety gate: APPS plausibility AND MCU state (R2D + CAN health) */
        bool torque_allowed = g_apps.torque_allowed && MCU_IsTorqueAllowed();
        bool apps_implausibility = g_apps.implausibility_active;
        Mutex_APPS_Unlock();

        /* 
         *  2 - Snapshot inverter states under their respective mutexes,
         *      then update LEDs outside any mutex (GPIO writes are fast).
         *      Reading g_inverter_x.state without a mutex is a race condition
         *      with the ReadingsManage task which updates state via CAN drain.
         */
        InverterState_t snap_left_state, snap_right_state;

        Mutex_INVERTER_L_Lock();
        snap_left_state  = g_inverter_left.state;
        Mutex_INVERTER_L_Unlock();

        Mutex_INVERTER_R_Lock();
        snap_right_state = g_inverter_right.state;
        Mutex_INVERTER_R_Unlock();

        Motor_UpdateStatusLeds(
            apps_implausibility,
            snap_right_state,
            snap_left_state
        );


        /* 
         *  3 - Inverter LEFT control loop
         *      Mutex_INVERTER_L protects g_inverter_left struct.
         *      Mutex_CAN1       protects the CAN1 peripheral during TX.
         */
        #if INVERTER_LEFT_PRESENT
            Mutex_INVERTER_L_Lock();
            Mutex_CAN1_Lock();

            #if INVERTER_LEFT_TORQUE_CONTROL_ENABLED
                Motor_ProcessInverterControl(
                    &hcan1,
                    &g_inverter_left,
                    pedal_percent,
                    torque_allowed,
                    &s_torque_setpoint_prev_left
                );
            #else
                s_torque_setpoint_prev_left = 0;
                g_inverter_left.torque_request = 0;

                /* Keep CAN command stream alive with a safe zero-torque request. */
                InverterCommandMsg1_t cmd_l = Inverter_BuildCommand(
                    &g_inverter_left,
                    0,
                    TORQUE_LIMIT_POS,
                    TORQUE_LIMIT_NEG
                );
                (void)CAN_Inverter_TransmitCommand(&hcan1, &g_inverter_left, &cmd_l);
            #endif

            Mutex_CAN1_Unlock();

            /* 
             *  4 - Detect INV_STATE_ERROR transition and capture diagnostic snapshot
             *      (still inside Mutex_INVERTER_L)
             */
            if (g_inverter_left.state == INV_STATE_ERROR &&
                s_prev_inv_state_left != INV_STATE_ERROR)
            {
                /* First cycle in ERROR — freeze conditions for data logger */
                s_error_l_timestamp  = HAL_GetTick();
                s_error_l_dc_voltage = g_inverter_left.dc_bus_voltage;
                s_error_l_torque     = g_inverter_left.torque_value;
                s_error_l_speed      = g_inverter_left.speed_rpm;
            }
            s_prev_inv_state_left = g_inverter_left.state;

            /* Check 10 s CAN silence -> transition to INV_STATE_OFF */
            Inverter_UpdateOffTimeout(&g_inverter_left);

            /* Check HV_ACTIVE timeout -> force transition to READY if stuck */
            Inverter_UpdateHVTransitionTimeout(&g_inverter_left, HAL_GetTick());

            Mutex_INVERTER_L_Unlock();
        #else
            s_prev_inv_state_left = INV_STATE_OFF;
        #endif

        /*
         *  5 - Inverter RIGHT control loop (symmetric to LEFT)
         */
        #if INVERTER_RIGHT_PRESENT
            Mutex_INVERTER_R_Lock();
            Mutex_CAN1_Lock();

            #if INVERTER_RIGHT_TORQUE_CONTROL_ENABLED
                Motor_ProcessInverterControl(
                    &hcan1,
                    &g_inverter_right,
                    pedal_percent, torque_allowed,
                    &s_torque_setpoint_prev_right
                );
            #else
                s_torque_setpoint_prev_right = 0;
                g_inverter_right.torque_request = 0;

                /* Keep CAN command stream alive with a safe zero-torque request. */
                InverterCommandMsg1_t cmd_r = Inverter_BuildCommand(
                    &g_inverter_right,
                    0,
                    TORQUE_LIMIT_POS,
                    TORQUE_LIMIT_NEG
                );
                (void)CAN_Inverter_TransmitCommand(&hcan1, &g_inverter_right, &cmd_r);
            #endif

            Mutex_CAN1_Unlock();

            /*
             *  6 - Detect INV_STATE_ERROR transition for RIGHT inverter
             */
            if (g_inverter_right.state == INV_STATE_ERROR &&
                s_prev_inv_state_right != INV_STATE_ERROR)
            {
                s_error_r_timestamp  = HAL_GetTick();
                s_error_r_dc_voltage = g_inverter_right.dc_bus_voltage;
                s_error_r_torque     = g_inverter_right.torque_value;
                s_error_r_speed      = g_inverter_right.speed_rpm;
            }
            s_prev_inv_state_right = g_inverter_right.state;

            /* Check 10 s CAN silence -> transition to INV_STATE_OFF */
            Inverter_UpdateOffTimeout(&g_inverter_right);

            /* Check HV_ACTIVE timeout -> force transition to READY if stuck */
            Inverter_UpdateHVTransitionTimeout(&g_inverter_right, HAL_GetTick());

            Mutex_INVERTER_R_Unlock();
        #else
            s_prev_inv_state_right = INV_STATE_OFF;
        #endif

        /* 
         *  7 - Update execution-time statistics
         *      HAL_GetTick() granularity is 1 ms; multiply to get µs approximation.
         */
        uint32_t elapsed_us = (HAL_GetTick() - t_start) * 1000U;

        if (elapsed_us > s_max_time_us)
        {
            s_max_time_us = elapsed_us;
        }
        if (elapsed_us > INVERTERS_TIMING_WARNING_US)
        {
            s_warning_count++;
        }

        if ((cycle_counter % WATCHDOG_REFRESH_PERIOD) == 0U)
        {
            Watchdog_Refresh();
        }
        cycle_counter++;


        /* Deterministic timing — absolute tick prevents drift */
        next_wake += period_ticks;
        osDelayUntil(next_wake);
    }
}

void InvertersManage_GetErrorSnapshot(InvErrorSnapshot_t *out)
{
    if (out == NULL) return;
    out->timestamp_ms = s_error_l_timestamp;
    out->dc_voltage   = s_error_l_dc_voltage;
    out->torque       = s_error_l_torque;
    out->speed        = s_error_l_speed;
}

void InvertersManage_GetErrorSnapshotRight(InvErrorSnapshot_t *out)
{
    if (out == NULL) return;
    out->timestamp_ms = s_error_r_timestamp;
    out->dc_voltage   = s_error_r_dc_voltage;
    out->torque       = s_error_r_torque;
    out->speed        = s_error_r_speed;
}

void InvertersManage_GetTimingStats(InvTimingStats_t *out)
{
    /* 32-bit reads/writes are atomic on Cortex-M7 - no mutex required. */
    out->max_time_us   = s_max_time_us;
    out->warning_count = s_warning_count;

    /* Reset for the next log window */
    s_max_time_us   = 0U;
    s_warning_count = 0U;
}

void InvertersManage_GetLeft(Inverter_t *out)
{
    if (out != NULL) { *out = g_inverter_left; }
}

void InvertersManage_GetRight(Inverter_t *out)
{
    if (out != NULL) { *out = g_inverter_right; }
}

void InvertersManage_GetControlSignals(int16_t *torque_req, int16_t *torque_lim)
{
    /* Atomic 16-bit reads on Cortex-M7. */
    if (torque_req != NULL) { *torque_req = torque_request;   }
    if (torque_lim != NULL) { *torque_lim = torque_limit_dyn; }
}
