#include "Control/error_recovery.h"
#include "Connectivity/Can.h"
#include "Connectivity/Serial.h"
#include <string.h>

static InverterErrorRecovery_t s_error_recovery_ctx[3] = {0};

InverterErrorRecovery_t *ErrorRecovery_GetCtx(const Inverter_t *inv) {
    if (inv == NULL || inv->node_id == 0U || inv->node_id >= 3U) {
        return NULL;
    }
    return &s_error_recovery_ctx[inv->node_id];
}

void ErrorRecovery_Process(FDCAN_HandleTypeDef *hfdcan, Inverter_t *inv, InverterErrorRecovery_t *err_ctx) {
    if (hfdcan == NULL || inv == NULL || err_ctx == NULL) return;

    uint32_t now_ms = HAL_GetTick();

    /* Log periodico guasto su LPUART1 */
    if (now_ms - err_ctx->log_last_ms >= 500U) {
        err_ctx->log_last_ms = now_ms;
        LPUART1_Log("[AMK_ERR] Node %d | Code: 0x%04X | DC: %u V | Step: %d\r\n",
                    inv->node_id, inv->error_code, inv->dc_bus_voltage, err_ctx->recovery_step);
    }

    if (err_ctx->start_time_ms == 0U) {
        err_ctx->start_time_ms = now_ms;
        err_ctx->recovery_step = 0U;
    }

    uint32_t elapsed_ms = now_ms - err_ctx->start_time_ms;
    InverterCommandMsg1_t cmd = {0};

    /* Sequenza AMK 8.2.6: fronte di salita -> mantenimento -> fronte di discesa */
    if (err_ctx->recovery_step == 0U) {
        cmd.control_word = INV_CTRL_ERROR_RESET;
        CAN_MOTOR_TransmitCommand(hfdcan, inv, &cmd);
        err_ctx->recovery_step = 1U;
    }
    else if (err_ctx->recovery_step == 1U && elapsed_ms >= ERROR_RESET_HOLD_MS) {
        /* Rilascio bErrorReset a 0 (fronte di discesa) */
        memset(&cmd, 0, sizeof(cmd));
        CAN_MOTOR_TransmitCommand(hfdcan, inv, &cmd);
        err_ctx->recovery_step = 2U;
    }
    else if (err_ctx->recovery_step == 2U && elapsed_ms >= (ERROR_RESET_HOLD_MS + ERROR_RESET_COOLDOWN_MS)) {
        err_ctx->retry_count++;
        if (err_ctx->retry_count < ERROR_RESET_MAX_RETRIES) {
            err_ctx->start_time_ms = HAL_GetTick();
            err_ctx->recovery_step = 0U;
        }
    }
}

void ErrorRecovery_Reset(InverterErrorRecovery_t *err_ctx) {
    if (err_ctx == NULL) return;
    err_ctx->start_time_ms = 0U;
    err_ctx->recovery_step = 0U;
    err_ctx->retry_count   = 0U;
}