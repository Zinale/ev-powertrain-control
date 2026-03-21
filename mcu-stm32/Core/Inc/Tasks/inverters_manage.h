/**
 * @file inverters_manage.h
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief Inverters Manager Task — inverter control loop and error diagnostics
 *
 * This task owns all inverter-related control logic that previously lived in
 * TaskFast, including:
 *  - Motor_ProcessInverterControl() for left (and future right) inverter
 *  - Motor_UpdateStatusLeds()
 *  - Error-state snapshot capture (conditions at the moment INV_STATE_ERROR occurs)
 *  - Worst-case execution-time tracking
 *
 * Period: 1 ms (1 kHz) — matches the AMK CAN heartbeat requirement.
 *
 * Thread-safety notes
 * -------------------
 *  InvertersManage_GetErrorSnapshot()       MUST be called while holding Mutex_INVERTER_L.
 *  InvertersManage_GetErrorSnapshotRight()   MUST be called while holding Mutex_INVERTER_R.
 *  InvertersManage_GetTimingStats()          is safe to call without a mutex (atomic 32-bit
 *                                            reads on Cortex-M7) and resets the counters
 *                                            after copying them.
 */

#ifndef INVERTERS_MANAGE_H
#define INVERTERS_MANAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "Inverter/Inverter.h"  /* Inverter_t, INV_STATE_ERROR, InverterState_t */

/* =============================================================================
 *  PUBLIC TYPES
 * ============================================================================= */

/**
 * @brief Conditions captured at the moment the left inverter entered ERROR state.
 *
 * Values are frozen on the first ERROR transition and kept until the next one.
 */
typedef struct
{
    uint32_t timestamp_ms;  /**< HAL_GetTick() at error entry            */
    uint16_t dc_voltage;    /**< DC bus voltage at error entry  [0.1 V]  */
    int16_t  torque;        /**< Torque current at error entry  [0.1% Mn]*/
    int16_t  speed;         /**< Motor speed at error entry     [RPM]    */
} InvErrorSnapshot_t;

/**
 * @brief Worst-case timing statistics for InvertersManageTask.
 *
 * @note InvertersManage_GetTimingStats() resets both counters after copying
 *       so that each DataLogger window reflects only the most recent period.
 */
typedef struct
{
    uint32_t max_time_us;    /**< Worst-case task execution time [µs]          */
    uint32_t warning_count;  /**< Executions that exceeded 900 µs (1 ms limit) */
} InvTimingStats_t;

/**
 * @brief CAN software counters snapshot.
 */
typedef struct
{
    uint32_t tx_ok;   /**< Successful TX frames since last reset  */
    uint32_t tx_fail; /**< Failed TX frames (mailbox full)        */
    uint32_t rx;      /**< Received frames since last reset       */
} CanStats_t;

/**
 * @brief CAN hardware diagnostic snapshot (no HAL types exposed).
 */
typedef struct
{
    const char *state_str; /**< Human-readable HAL_CAN state (READY, ERROR …)  */
    uint32_t    err_code;  /**< HAL error bitmask (0 = no error)               */
    uint8_t     tec;       /**< Transmit Error Counter from ESR                */
    uint8_t     rec;       /**< Receive Error Counter from ESR                 */
} CanHwState_t;

/* =============================================================================
 *  FUNCTION DECLARATIONS
 * ============================================================================= */

/**
 * @brief FreeRTOS task: runs inverter control loop at 1 kHz.
 *
 * Never returns — contains the infinite for(;;) loop.
 * Registered in main.c as:
 *   osThreadNew(StartInvertersManager, NULL, &InvertersManage_attributes)
 */
void InvertersManageTask(void);

/**
 * @brief Copy the error snapshot captured when the LEFT inverter last entered ERROR.
 *
 * @param[out] out  Pointer to caller-allocated InvErrorSnapshot_t.
 *
 * @note MUST be called while holding Mutex_INVERTER_L.
 */
void InvertersManage_GetErrorSnapshot(InvErrorSnapshot_t *out);

/**
 * @brief Copy the error snapshot captured when the RIGHT inverter last entered ERROR.
 *
 * @param[out] out  Pointer to caller-allocated InvErrorSnapshot_t.
 *
 * @note MUST be called while holding Mutex_INVERTER_R.
 */
void InvertersManage_GetErrorSnapshotRight(InvErrorSnapshot_t *out);

/**
 * @brief Copy and reset the task timing statistics.
 *
 * After this call both internal counters are cleared so the next log window
 * starts fresh.
 *
 * @param[out] out  Pointer to caller-allocated InvTimingStats_t.
 */
void InvertersManage_GetTimingStats(InvTimingStats_t *out);

/**
 * @brief Copy left inverter snapshot.
 *        Must be called while Mutex_INVERTER_L is held.
 */
void InvertersManage_GetLeft(Inverter_t *out);

/**
 * @brief Copy right inverter snapshot.
 *        Must be called while Mutex_INVERTER_R is held.
 */
void InvertersManage_GetRight(Inverter_t *out);

/**
 * @brief Copy CAN software counters and optionally reset them.
 * @param out    Destination (non-NULL).
 * @param reset  Non-zero → clear internal counters after copy.
 */
void InvertersManage_GetCanStats(CanStats_t *out, uint8_t reset);

/**
 * @brief Read last atomic copy of torque_request / torque_limit_dyn.
 */
void InvertersManage_GetControlSignals(int16_t *torque_req, int16_t *torque_lim);

/**
 * @brief Snapshot CAN hardware state (state string, error code, TEC, REC).
 */
void InvertersManage_GetCanHwState(CanHwState_t *out);

#ifdef __cplusplus
}
#endif

#endif /* INVERTERS_MANAGE_H */
