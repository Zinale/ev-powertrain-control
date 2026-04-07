/**
 * @file BaseControlMotor.c
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief Implementation of basic engine & Inverter control functions
 */

#include "Controls/BaseControlMotor.h"
#include "Communication/Can.h"
#include "Communication/Serial.h"
#include "APP/APPS.h"
#include "Config.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#if defined(DATA_COLLECT_MODE) && (DATA_COLLECT_BACKEND == DATA_COLLECT_BACKEND_FEATHER_LOCAL)
    #define MOTOR_ERR_LOG_CHANNEL  LOG_CH_UART3
#else
    #define MOTOR_ERR_LOG_CHANNEL  LOG_CH_BOTH
#endif

/* CAN transmission diagnostics (global variables) */
uint32_t g_can_tx_ok_count = 0;
uint32_t g_can_tx_fail_count = 0;

/* CAN reception diagnostics */
uint32_t g_can_rx_count = 0;  // Messages received from inverter

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

volatile int16_t torque_request;    /**< Written by InvertersManageTask, read by DataLoggerTask — must be volatile */
volatile int16_t torque_limit_dyn;  /**< Written by InvertersManageTask, read by DataLoggerTask — must be volatile */

#if defined(REGEN_ENABLED) && REGEN_ENABLED
/* Latch regen selection per node_id to avoid mode chattering near pedal threshold. */
static bool s_regen_latch[3] = {false, false, false};

static bool Motor_ShouldUseRegen(const Inverter_t *inv, uint8_t pedal_percent)
{
    if (inv == NULL || inv->node_id == 0U || inv->node_id >= 3U) {
        return false;
    }

    if (inv->state != INV_STATE_RUNNING) {
        s_regen_latch[inv->node_id] = false;
        return false;
    }

    /* Speed hysteresis on latch RESET:
     *   - Latch CAN DEACTIVATE only when speed drops below REGEN_SPEED_CRITICAL_RPM
     *     (already the intended exit threshold).
     *   - We do NOT reset the latch here if speed flickers
     *     around CRITICAL. The latch reset due to speed happens only when
     *     speed is strictly below CRITICAL — the lower-guard prevents
     *     re-triggering from noise.
     * The check below uses a 10% lower guard band so noise around 2000 RPM
     * does NOT cause repeated latch-reset -> regen-enter oscillations. */
    const int16_t speed_latch_reset_thr = (int16_t)(REGEN_SPEED_CRITICAL_RPM * 9U / 10U); /* 90% of CRITICAL = 1800 RPM */
    if (inv->speed_rpm <= speed_latch_reset_thr) {
        s_regen_latch[inv->node_id] = false;
        return false;
    }

    /* If latch is ON but speed fell below CRITICAL (but above the hard reset above),
     * keep the latch alive but let k_vel fade-out smooth the torque to ~0. */
    if (!s_regen_latch[inv->node_id] && inv->speed_rpm <= (int16_t)REGEN_SPEED_CRITICAL_RPM) {
        /* Speed is in [speed_latch_reset_thr, CRITICAL]: cannot enter regen */
        return false;
    }

    const uint8_t threshold = REGEN_PEDAL_THRESHOLD_PCT;
    const uint8_t hysteresis = REGEN_PEDAL_HYST_PCT;
    const uint8_t enter_thr = (threshold > hysteresis) ? (threshold - hysteresis) : 0U;
    const uint8_t exit_thr = ((uint16_t)threshold + (uint16_t)hysteresis <= 100U)
        ? (uint8_t)(threshold + hysteresis)
        : 100U;

    if (s_regen_latch[inv->node_id]) {
        if (pedal_percent >= exit_thr) {
            s_regen_latch[inv->node_id] = false;
        }
    } else {
        /* Enter regen only if pedal is low AND speed is high enough.
         * This creates speed hysteresis: enter at SPEED_MIN,
         * exit at SPEED_CRITICAL — avoids entering regen at
         * marginal speeds where k_vel fade-out makes it near-zero. */
        if (pedal_percent <= enter_thr && inv->speed_rpm > (int16_t)REGEN_SPEED_MIN_RPM) {
            s_regen_latch[inv->node_id] = true;
        }
    }

    return s_regen_latch[inv->node_id];
}
#endif

extern uint32_t s_error_l_timestamp;
extern uint16_t s_error_l_dc_voltage;
extern int16_t  s_error_l_torque;
extern int16_t  s_error_l_speed;
extern uint32_t s_error_r_timestamp;
extern uint16_t s_error_r_dc_voltage;
extern int16_t  s_error_r_torque;
extern int16_t  s_error_r_speed;

int16_t Motor_CalculateDynamicTorqueLimit(int16_t speed_rpm, uint32_t actual_power) {
    float speed = (float)abs(speed_rpm);
    
    if (speed < 10.0f) speed = 10.0f;
    
    /* Calculate available power: P = V × I × η */
    float power_w = actual_power * MOTOR_EFFICIENCY;  /* Account for estimated efficiency losses */
    
    /* AMK formula: M_soll_max = P / (2 × π × N_ist / 60) */
    float omega_rad_s = 2.0f * M_PI * speed / 60.0f;
    float torque_lim_nm = power_w / omega_rad_s;
    
    /* Saturation to motor mechanical limit */
    if (torque_lim_nm > MOTOR_MAX_TORQUE_NM) {
        torque_lim_nm = MOTOR_MAX_TORQUE_NM;
    }
    
    /* Conversion to CAN units */
    return (int16_t)((torque_lim_nm / MOTOR_NOMINAL_TORQUE_NM) * CAN1_SCALE_FACTOR);
}


int16_t Motor_ApplyTorqueRateLimit(int16_t target, int16_t previous, int16_t max_delta) {
    int16_t delta = target - previous;
    
    if (delta > max_delta) {
        return previous + max_delta;
    } else if (delta < -max_delta) {
        return previous - max_delta;
    } else {
        return target;
    }
}


#ifdef REGEN_ENABLED
#if REGEN_ENABLED

/**
 * @brief Calculate regenerative braking torque using three-stage formula
 * 
 * Stage 1 (Linear pedal-dependent): 
 *   T_req = T_max_regen * (1 - pedal_pct / P_threshold)
 *   At pedal=0%: full regen (-T_max_regen)
 *   At pedal=20%: zero regen (0 Nm)
 *   Above 20%: zero regen or switch to acceleration
 * 
 * Stage 2 (Speed-dependent fade-out):
 *   k_vel = min(1, speed_current_rpm / speed_min_rpm)
 *   Prevents regen at very low speeds (< 50 RPM):
 *   - At 50 RPM: factor = 1.0 (full regen)
 *   - At 25 RPM: factor = 0.5 (half regen)
 *   - Below 10 RPM: factor ≈ 0 (no regen)
 * 
 * Stage 3 (Power limit - checked after):
 *   |T_req| <= P_max [W] * 60 / (2π * speed_rpm)
 *   Ensures battery protection from regenerative power spikes
 */
int16_t Motor_CalculateRegenerativeBraking(int16_t speed_rpm, 
                                            uint8_t pedal_percent, 
                                            uint16_t dc_voltage) {
    if (pedal_percent >= REGEN_PEDAL_THRESHOLD_PCT) {
        return 0;
    }

    /* Safety: if motor effectively stopped, no regeneration.
     * Use the LOWER guard band (90% of CRITICAL), NOT
     * REGEN_SPEED_CRITICAL_RPM, to stay CONSISTENT with the
     * latch reset threshold in Motor_ShouldUseRegen().*/
    const int16_t speed_hard_stop = (int16_t)(REGEN_SPEED_CRITICAL_RPM * 9U / 10U); 
    if (abs(speed_rpm) <= speed_hard_stop) {
        return 0;
    }
    
    float speed = (float)(speed_rpm);
    if (speed< 10.0f){
        speed = 10.0f;
        return 0;
    }
    
    /*  STAGE 1: Pedal-dependent linear ramping */
    /* T_req = -T_max_regen * (1 - pedal_pct / threshold)  [NEGATIVE = braking] */
    float pedal_norm = (float)pedal_percent / (float)REGEN_PEDAL_THRESHOLD_PCT;
    float torque_stage1_nm = -REGEN_TORQUE_MAX_NM * (1.0f - pedal_norm);
    
    /*  STAGE 2: Speed-dependent fade-out (low speeds)  */
    /* k_vel = min(1, speed_current / speed_min) */
    float speed_min = (float)REGEN_SPEED_MIN_RPM;
    float k_vel = (speed < speed_min) ? (speed / speed_min) : 1.0f;
    
    float torque_stage2_nm = torque_stage1_nm * k_vel;
    
    /*  STAGE 3: Battery power limit  */
    /* |T_max| <= P_batt_max / omega   [Nm] */
    float omega_rad_s = 2.0f * M_PI * speed / 60.0f;
    float pbatt_max = (float)REGEN_PBATT_MAX_W;
    float torque_power_limit_nm = pbatt_max / omega_rad_s;
    
    /* Clamp: torque_stage2_nm is negative, limit is positive → check lower bound */
    if (torque_stage2_nm < -torque_power_limit_nm) {
        torque_stage2_nm = -torque_power_limit_nm;
    }

    /* STAGE 4: DC Bus Voltage Derating (Soft-clipping) */
    /* Reduce regen from VCU before inverter hardware intervention */
    float voltage_factor = 1.0f;
    
    if (dc_voltage >= REGEN_DC_DERATE_END_V) {
        voltage_factor = 0.0f; /* Voltage too high, turn off regen */
    } else if (dc_voltage > REGEN_DC_DERATE_START_V) {
        /* Calculate the percentage of linear reduction */
        voltage_factor = 1.0f - (float)(dc_voltage - REGEN_DC_DERATE_START_V) / 
                                (float)(REGEN_DC_DERATE_END_V - REGEN_DC_DERATE_START_V);
    }
    
    /* Apply voltage reduction to the torque calculated so far */
    torque_stage2_nm *= voltage_factor;
    



    /* CAN_unit = (Nm / MOTOR_NOMINAL_TORQUE_NM) * 1000 */
    int16_t torque_can_units = (int16_t)((torque_stage2_nm / MOTOR_NOMINAL_TORQUE_NM) * 1000.0f);
    
    /* Respect torque limits */
    if (torque_can_units < TORQUE_LIMIT_NEG) {
        torque_can_units = TORQUE_LIMIT_NEG;
    }
    if (torque_can_units > 0) {
        torque_can_units = 0;  /* Regeneration must be negative */
    }
    
    return torque_can_units;
}

#endif /* REGEN_ENABLED */
#endif


void Motor_UpdateStatusLeds(bool apps_implausibility,
                            InverterState_t right_state,
                            InverterState_t left_state) {
    /* LD1: APPS implausibility */
    if (apps_implausibility) {
       // HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
    } else {
       // HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
    }

    /* LD2: LEFT inverter (node 2 / inverter204) in error */
    if (left_state == INV_STATE_ERROR) {
        //HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
    } else {
        //HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
    }

    /* LD3: RIGHT inverter (node 1 / inverter104) in error */
    if (right_state == INV_STATE_ERROR) {
       // HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
    } else {
        //HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
    }
}


/* Error recovery state machine (independent per inverter node_id). */
typedef struct {
    uint32_t start_time_ms;
    uint8_t  recovery_step;
    uint32_t log_last_ms;
} InverterErrorRecovery_t;

/* Index by node_id (1 and 2 used, 0 unused). */
static InverterErrorRecovery_t s_error_recovery_ctx[3] = {0};

static InverterErrorRecovery_t *Motor_GetErrorRecoveryCtx(const Inverter_t *inv)
{
    if (inv == NULL || inv->node_id == 0U || inv->node_id >= 3U) {
        return NULL;
    }
    return &s_error_recovery_ctx[inv->node_id];
}

void Motor_ProcessInverterControl(CAN_HandleTypeDef *hcan, 
                                   Inverter_t *inv, 
                                   uint8_t pedal_percent, bool torque_allowed,
                                   int16_t *torque_setpoint_prev) {
    if (inv == NULL || torque_setpoint_prev == NULL || hcan == NULL) return;

    InverterErrorRecovery_t *err_ctx = Motor_GetErrorRecoveryCtx(inv);
    if (err_ctx == NULL) return;
    
    const InverterState_t state = inv->state;
    
    if (state == INV_STATE_OFF) return;
    
    if (state == INV_STATE_ERROR) {
        uint32_t now_ms = HAL_GetTick();
        if (now_ms - err_ctx->log_last_ms >= 100U) {
            err_ctx->log_last_ms = now_ms;

            uint32_t error_ts = s_error_l_timestamp;
            uint16_t error_dc = s_error_l_dc_voltage;
            int16_t error_tq = s_error_l_torque;
            int16_t error_sp = s_error_l_speed;
            if (inv == &g_inverter_right) {
                error_ts = s_error_r_timestamp;
                error_dc = s_error_r_dc_voltage;
                error_tq = s_error_r_torque;
                error_sp = s_error_r_speed;
            }

            Serial_Log(MOTOR_ERR_LOG_CHANNEL, "[ERROR] Code: %d | Entered at t=%lu (+%lu ms ago)\r\n",
                inv->error_code,
                error_ts,
                now_ms - error_ts);
            Serial_Log(MOTOR_ERR_LOG_CHANNEL, "[CONDs]  Conditions: DC=%u.%uV | Torque=%d | Speed=%d RPM%s\r\n",
                error_dc / 10, error_dc % 10,
                error_tq,
                error_sp,
                (error_dc > 400) ? " HIGH VOLTAGE!" : "");
        }

        if (err_ctx->start_time_ms == 0U) {
            err_ctx->start_time_ms = HAL_GetTick();
            err_ctx->recovery_step = 0U;
        }
        
        uint32_t elapsed_ms = HAL_GetTick() - err_ctx->start_time_ms;
        InverterCommandMsg1_t cmd = {0};
        
        /* Progressive reset with delays to give inverter time to recover */
        if (err_ctx->recovery_step == 0U) {
            cmd.control_word = INV_CTRL_ERROR_RESET;
            CAN_Inverter_TransmitCommand(hcan, inv, &cmd);
            err_ctx->recovery_step = 1U;
        } 
        else if (err_ctx->recovery_step == 1U && elapsed_ms >= 5U) {
            memset(&cmd, 0, sizeof(cmd));
            CAN_Inverter_TransmitCommand(hcan, inv, &cmd);
            err_ctx->recovery_step = 2U;
        }
        
        *torque_setpoint_prev = 0;
        return;   /* do NOT fall through to normal torque command */
    } else {
        /* Reset error recovery when not in error */
        err_ctx->start_time_ms = 0U;
        err_ctx->recovery_step = 0U;
    }

    

    if (!torque_allowed) {
        inv->torque_request = 0;
        *torque_setpoint_prev = 0;
        torque_request   = 0;
        torque_limit_dyn = 0;
    }
    else
    {
        /* Calculate dynamic torque limit */
        torque_limit_dyn = Motor_CalculateDynamicTorqueLimit(
            inv->speed_rpm,
            inv->actual_power
        );

        /* Calculate torque requested from pedal (or regen if enabled and in regen zone).
         * torque_raw is intentionally a local variable: it is only used within this
         * function and must not be shared across tasks. */
        int16_t torque_raw;

        #if defined(REGEN_ENABLED) && REGEN_ENABLED
            if (Motor_ShouldUseRegen(inv, pedal_percent)) {
                /* Pedal released: regenerative braking torque (negative) */
                torque_raw = Motor_CalculateRegenerativeBraking(
                    inv->speed_rpm,
                    pedal_percent,
                    inv->dc_bus_voltage
                );
            } else {
                /* Pedal pressed: normal traction torque (positive) */
                torque_raw = (int16_t)((pedal_percent * TORQUE_SETPOINT_MAX) / 100);
            }
        #else
            /* Regen disabled: traction only */
            torque_raw = (int16_t)((pedal_percent * TORQUE_SETPOINT_MAX) / 100);
        #endif

        torque_request = Motor_ApplyTorqueRateLimit(
            torque_raw,
            *torque_setpoint_prev,
            TORQUE_RATE_LIMIT_PER_CYCLE
        );

        /* Saturation to limits */
        if (torque_request > TORQUE_LIMIT_POS) torque_request = TORQUE_LIMIT_POS;
        if (torque_request < TORQUE_LIMIT_NEG) torque_request = TORQUE_LIMIT_NEG;
        if (inv->speed_rpm > MOTOR_MAX_SPEED_RPM) {
            torque_request = 0;
        }
        if (abs(inv->speed_rpm) < 10 && torque_request < 0) {
            torque_request = 0;
        }

        inv->torque_request = torque_request;
        if (state != INV_STATE_RUNNING) {
            *torque_setpoint_prev = 0;
            inv->torque_request   = 0;
        } else {
            *torque_setpoint_prev = torque_request;
        }
    }





    /* Build and send command */
    InverterCommandMsg1_t cmd = Inverter_BuildCommand(
        inv,
        torque_request,
        TORQUE_LIMIT_POS,
        TORQUE_LIMIT_NEG
    );
    

    /* Send command */
    bool tx_ok = CAN_Inverter_TransmitCommand(hcan, inv, &cmd);
    
    if (tx_ok) {
        g_can_tx_ok_count++;
    } else {
        /* Mailbox full: abort old messages and retry with fresh command */
        HAL_CAN_AbortTxRequest(hcan, CAN_TX_MAILBOX0 | CAN_TX_MAILBOX1 | CAN_TX_MAILBOX2);
        tx_ok = CAN_Inverter_TransmitCommand(hcan, inv, &cmd);
        
        if (tx_ok) {
            g_can_tx_ok_count++;
        } else {
            g_can_tx_fail_count++;
        }
    }
}
