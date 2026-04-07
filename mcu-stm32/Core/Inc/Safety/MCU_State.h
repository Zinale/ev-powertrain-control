/**
 * @file MCU_State.h
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief MCU top-level state machine — system-wide safety state
 *
 * Defines four operating states for the MCU and the logic to transition
 * between them based on CAN health and sensor plausibility.
 *
 * State summary
 * -------------
 *  INIT           — Power-on before all subsystems are ready.
 *                   Torque NOT allowed. Serial enabled (heartbeat only).
 *
 *  READY          — CAN1 and CAN2 functional, APPS and SAS plausible.
 *                   Waiting for R2D signal from dashboard.
 *                   Torque NOT allowed. Serial enabled (full output).
 *
 *  RUNNING        — READY conditions met AND R2D active.
 *                   Torque ALLOWED. Serial enabled (full output).
 *
 *  IMPLAUSIBILITY — CAN buses ok, but APPS or SAS reports an implausibility
 *                   or out-of-range fault.
 *                   Torque NOT allowed. Serial enabled (sensor detail).
 *
 *  ERROR          — CAN1 or CAN2 has failed (init failed or bus-off).
 *                   Torque NOT allowed. Serial enabled (CAN diagnostics).
 *
 * Transition diagram
 * ------------------
 *
 *   INIT ──(CAN fail)──────────────────────────────► ERROR
 *   INIT ──(CAN ok, sensors ok)────────────────────► READY
 *   INIT ──(CAN ok, sensors KO)────────────────────► IMPLAUSIBILITY
 *
 *   READY ──(CAN fail)─────────────────────────────► ERROR
 *   READY ──(sensor KO)────────────────────────────► IMPLAUSIBILITY
 *   READY ──(R2D)──────────────────────────────────► RUNNING
 *
 *   RUNNING ──(CAN fail)───────────────────────────► ERROR
 *   RUNNING ──(sensor KO)──────────────────────────► IMPLAUSIBILITY
 *   RUNNING ──(!R2D)───────────────────────────────► READY
 *
 *   IMPLAUSIBILITY ──(CAN fail)────────────────────► ERROR
 *   IMPLAUSIBILITY ──(sensors ok + !R2D)───────────► READY
 *   IMPLAUSIBILITY ──(sensors ok + R2D)────────────► RUNNING
 *
 *   ERROR ──(CAN recover + sensors ok + !R2D)──────► READY
 *   ERROR ──(CAN recover + sensors ok + R2D)───────► RUNNING
 *   ERROR ──(CAN recover + sensors KO)─────────────► IMPLAUSIBILITY
 *
 * Usage
 * -----
 *  1. Once per task cycle, fill a MCU_StateInputs_t and call MCU_State_Update().
 *  2. Read MCU_State_Get() / MCU_IsTorqueAllowed() wherever needed.
 *
 * Thread-safety: MCU_State_Update() is NOT reentrant and MUST be called from a
 * single task (DataManagerTask). MCU_State_Get() and the query helpers perform
 * a single volatile read and are safe to call from any task.
 */

#ifndef MCU_STATE_H
#define MCU_STATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

/* ============================================================================
 * State enumeration
 * ============================================================================ */

typedef enum
{
    MCU_STATE_INIT           = 0, /**< Power-on / not yet ready                  */
    MCU_STATE_READY          = 1, /**< Systems nominal, waiting R2D — no torque  */
    MCU_STATE_RUNNING        = 2, /**< R2D active — torque allowed               */
    MCU_STATE_IMPLAUSIBILITY = 3, /**< Sensor fault — no torque                  */
    MCU_STATE_ERROR          = 4, /**< CAN failure  — no torque                  */
} MCU_State_t;

/* ============================================================================
 * Input struct
 * ============================================================================
 *
 * The caller (DataManagerTask) is responsible for assembling this struct
 * under the appropriate mutexes / in the same task context in which CAN
 * and sensor states are updated.
 */

typedef struct
{
    bool can1_ok;     /**< CAN1 is READY and not in bus-off                                 */
    bool can2_ok;     /**< CAN2 is READY                                                    */
    bool apps_ok;     /**< No APPS implausibility and no out-of-range fault                 */
    bool sas_ok;      /**< SAS sensor valid and no out-of-range fault                       */
    bool r2d_active;  /**< Ready-to-Drive flag received on CAN2 (0x106)                     */
    bool inv_ready;   /**< All present+control-enabled inverters are in INV_STATE_RUNNING    */
} MCU_StateInputs_t;

/* ============================================================================
 * API
 * ============================================================================ */

/**
 * @brief Run one state-machine evaluation cycle.
 *
 * Reads the input conditions, computes the next state, and updates the
 * internal state variable. Records the entry tick on every state change.
 *
 * @param in  Pointer to a filled MCU_StateInputs_t snapshot.
 *            Must not be NULL.
 *
 * @note Call once per DataManagerTask cycle (4 ms period).
 * @note NOT reentrant — call from DataManagerTask only.
 */
void MCU_State_Update(const MCU_StateInputs_t *in);

/**
 * @brief Return the current MCU state.
 *
 * Safe to call from any task (single volatile read on Cortex-M7).
 */
MCU_State_t MCU_State_Get(void);

/**
 * @brief Return the HAL tick at which the current state was entered.
 *
 * Useful for computing how long the system has been in ERROR or
 * IMPLAUSIBILITY. Resets to HAL_GetTick() on every state transition.
 */
uint32_t MCU_State_GetEntryTick(void);

/**
 * @brief Return true if the current state permits torque output.
 *
 * Torque is permitted only in MCU_STATE_RUNNING.
 */
bool MCU_IsTorqueAllowed(void);

/**
 * @brief Return true if serial diagnostic output is enabled.
 *
 * Serial is enabled in RUNNING, IMPLAUSIBILITY, and ERROR (not in INIT).
 */
bool MCU_IsSerialEnabled(void);

/**
 * @brief Return a short human-readable string for a given state.
 *
 * Returns a pointer to a string literal — do not free.
 */
const char *MCU_State_ToString(MCU_State_t state);

#ifdef __cplusplus
}
#endif

#endif /* MCU_STATE_H */
