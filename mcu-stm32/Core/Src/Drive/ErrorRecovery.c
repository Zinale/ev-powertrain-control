/**
 * @file ErrorRecovery.c
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief AMK inverter error recovery state machine implementation
 *
 * Implements the AMK 8.2.6 "Remove error" diagram:
 *   Step 0: AMK_bInverterOn = 0 (implicit — control_word has no other bits)
 *           AMK_bErrorReset = 1
 *   Step 1: Hold for ERROR_RESET_HOLD_MS, then AMK_bErrorReset = 0
 *   Step 2: Wait cooldown, check if error cleared. Retry if needed.
 */

#include "Drive/ErrorRecovery.h"
#include "Communication/Can.h"
#include "Communication/Serial.h"
#include "Config.h"
#include <string.h>

#if defined(DATA_COLLECT_MODE) && (DATA_COLLECT_BACKEND == DATA_COLLECT_BACKEND_FEATHER_LOCAL)
    #define ERR_LOG_CHANNEL  LOG_CH_UART3
#else
    #define ERR_LOG_CHANNEL  LOG_CH_BOTH
#endif

/* One context per node_id (1 and 2 used, index 0 unused). */
static InverterErrorRecovery_t s_error_recovery_ctx[3] = {0};

/* Extern error snapshot variables (captured by motors_manager on error entry) */
extern uint32_t s_error_l_timestamp;
extern uint16_t s_error_l_dc_voltage;
extern int16_t  s_error_l_torque;
extern int16_t  s_error_l_speed;
extern uint32_t s_error_r_timestamp;
extern uint16_t s_error_r_dc_voltage;
extern int16_t  s_error_r_torque;
extern int16_t  s_error_r_speed;

extern Inverter_t g_inverter_right;

InverterErrorRecovery_t *ErrorRecovery_GetCtx(const Inverter_t *inv)
{
    if (inv == NULL || inv->node_id == 0U || inv->node_id >= 3U) {
        return NULL;
    }
    return &s_error_recovery_ctx[inv->node_id];
}

void ErrorRecovery_Process(CAN_HandleTypeDef *hcan,
                           Inverter_t *inv,
                           InverterErrorRecovery_t *err_ctx)
{
    if (hcan == NULL || inv == NULL || err_ctx == NULL) return;

    /* --- Rate-limited error logging --- */
    uint32_t now_ms = HAL_GetTick();
    if (now_ms - err_ctx->log_last_ms >= 100U) {
        err_ctx->log_last_ms = now_ms;

        uint32_t error_ts = s_error_l_timestamp;
        uint16_t error_dc = s_error_l_dc_voltage;
        int16_t  error_tq = s_error_l_torque;
        int16_t  error_sp = s_error_l_speed;
        if (inv == &g_inverter_right) {
            error_ts = s_error_r_timestamp;
            error_dc = s_error_r_dc_voltage;
            error_tq = s_error_r_torque;
            error_sp = s_error_r_speed;
        }

        Serial_Log(ERR_LOG_CHANNEL, "[ERROR] Code: %d | Entered at t=%lu (+%lu ms ago)\r\n",
            inv->error_code,
            error_ts,
            now_ms - error_ts);
        Serial_Log(ERR_LOG_CHANNEL, "[CONDs]  Conditions: DC=%u.%uV | Torque=%d | Speed=%d RPM%s\r\n",
            error_dc / 10, error_dc % 10,
            error_tq,
            error_sp,
            (error_dc > 400) ? " HIGH VOLTAGE!" : "");
    }

    /* --- Initialise recovery on first entry --- */
    if (err_ctx->start_time_ms == 0U) {
        err_ctx->start_time_ms = HAL_GetTick();
        err_ctx->recovery_step = 0U;
    }

    uint32_t elapsed_ms = HAL_GetTick() - err_ctx->start_time_ms;
    InverterCommandMsg1_t cmd = {0};

    /* AMK 8.2.6 Remove-error sequence:
     *  Step 0: Send bErrorReset=1 (with InverterOn/DC_ON/Enable all 0)
     *  Step 1: After hold time, send bErrorReset=0 (falling edge triggers reset)
     *  Step 2: Wait cooldown, then check if error cleared.
     *          If still in error, retry up to MAX_RETRIES. */
    if (err_ctx->recovery_step == 0U) {
        cmd.control_word = INV_CTRL_ERROR_RESET;
        CAN_Inverter_TransmitCommand(hcan, inv, &cmd);
        err_ctx->recovery_step = 1U;
    }
    else if (err_ctx->recovery_step == 1U && elapsed_ms >= ERROR_RESET_HOLD_MS) {
        /* Release bErrorReset — falling edge */
        memset(&cmd, 0, sizeof(cmd));
        CAN_Inverter_TransmitCommand(hcan, inv, &cmd);
        err_ctx->recovery_step = 2U;
    }
    else if (err_ctx->recovery_step == 2U && elapsed_ms >= (ERROR_RESET_HOLD_MS + ERROR_RESET_COOLDOWN_MS)) {
        /* Error still active after cooldown — retry if allowed */
        err_ctx->retry_count++;
        if (err_ctx->retry_count < ERROR_RESET_MAX_RETRIES) {
            err_ctx->start_time_ms = HAL_GetTick();
            err_ctx->recovery_step = 0U;
        }
        /* else: stay in step 2, stop retrying — requires manual intervention */
    }
}

void ErrorRecovery_Reset(InverterErrorRecovery_t *err_ctx)
{
    if (err_ctx == NULL) return;
    err_ctx->start_time_ms = 0U;
    err_ctx->recovery_step = 0U;
    err_ctx->retry_count   = 0U;
}
