#ifndef ERROR_RECOVERY_H
#define ERROR_RECOVERY_H

#include "main.h"
#include "Control/inverter.h"
#include <stdint.h>
#include <stdbool.h>

#define ERROR_RESET_HOLD_MS      50U   /* Tempo di mantenimento bErrorReset = 1 */
#define ERROR_RESET_COOLDOWN_MS  100U  /* Attesa verifica post-reset */
#define ERROR_RESET_MAX_RETRIES  5U    /* Tentativi massimi prima di blocco definitivo */

typedef struct {
    uint32_t start_time_ms;
    uint8_t  recovery_step;
    uint8_t  retry_count;
    uint32_t log_last_ms;
} InverterErrorRecovery_t;

InverterErrorRecovery_t* ErrorRecovery_GetCtx(const Inverter_t *inv);
void ErrorRecovery_Process(FDCAN_HandleTypeDef *hfdcan, Inverter_t *inv, InverterErrorRecovery_t *err_ctx);
void ErrorRecovery_Reset(InverterErrorRecovery_t *err_ctx);

#endif /* ERROR_RECOVERY_H */