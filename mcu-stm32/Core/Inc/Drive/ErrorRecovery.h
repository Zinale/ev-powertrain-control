/**
 * @file ErrorRecovery.h
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief AMK inverter error recovery state machine
 *
 * Implements the AMK 8.2.6 "Remove error" sequence:
 *   1. Send bErrorReset=1 (hold for ERROR_RESET_HOLD_MS)
 *   2. Send bErrorReset=0 (falling edge triggers reset in inverter)
 *   3. Wait cooldown, retry up to ERROR_RESET_MAX_RETRIES
 */

#ifndef ERROR_RECOVERY_H
#define ERROR_RECOVERY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "Drive/Inverter.h"
#include "stm32f7xx_hal.h"

/* Timing parameters for error recovery sequence */
#define ERROR_RESET_HOLD_MS      50U   /**< Time to hold bErrorReset=1 before releasing */
#define ERROR_RESET_COOLDOWN_MS  100U  /**< Cooldown between retry attempts */
#define ERROR_RESET_MAX_RETRIES  5U    /**< Max consecutive reset attempts */

/**
 * @brief Error recovery state machine context (one per inverter)
 */
typedef struct {
    uint32_t start_time_ms;    /**< Timestamp when current attempt started */
    uint8_t  recovery_step;    /**< Current step in the recovery sequence */
    uint8_t  retry_count;      /**< Number of retries so far */
    uint32_t log_last_ms;      /**< Last error log timestamp (rate-limiting) */
} InverterErrorRecovery_t;

/**
 * @brief Get the error recovery context for a given inverter
 *
 * @param inv Pointer to inverter struct
 * @return Pointer to the recovery context, or NULL if invalid
 */
InverterErrorRecovery_t *ErrorRecovery_GetCtx(const Inverter_t *inv);

/**
 * @brief Run the AMK 8.2.6 error recovery sequence for one inverter
 *
 * Call this every control cycle when the inverter is in INV_STATE_ERROR.
 * Handles logging, reset pulse generation, and retry logic.
 *
 * @param hcan   CAN handle for transmission
 * @param inv    Pointer to the inverter in error
 * @param err_ctx Pointer to the recovery context (from ErrorRecovery_GetCtx)
 */
void ErrorRecovery_Process(CAN_HandleTypeDef *hcan,
                           Inverter_t *inv,
                           InverterErrorRecovery_t *err_ctx);

/**
 * @brief Reset recovery context when the inverter exits error state
 *
 * @param err_ctx Pointer to the recovery context
 */
void ErrorRecovery_Reset(InverterErrorRecovery_t *err_ctx);

#ifdef __cplusplus
}
#endif

#endif /* ERROR_RECOVERY_H */
