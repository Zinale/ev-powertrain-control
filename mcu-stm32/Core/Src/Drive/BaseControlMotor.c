/**
 * @file BaseControlMotor.c
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief Implementation of basic engine & Inverter control functions
 */

#include "Drive/BaseControlMotor.h"
#include "Drive/ErrorRecovery.h"
#include "Drive/Regen.h"
#include "Communication/Can.h"
#include "Communication/Serial.h"
#include "Sensors/APPS.h"
#include "Config.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

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


void Motor_UpdateStatusLeds(bool apps_implausibility,
                            InverterState_t right_state,
                            InverterState_t left_state) {
    /* LD1: APPS implausibility */
    if (apps_implausibility) {
       //HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
    } else {
        //HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
    }

    /* LD2: LEFT inverter (node 2 / inverter204) in error */
    if (left_state == INV_STATE_ERROR) {
       HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
    } else {
       HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
    }

    /* LD3: RIGHT inverter (node 1 / inverter104) in error */
    if (right_state == INV_STATE_ERROR) {
        HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
    }
}




void Motor_ProcessInverterControl(CAN_HandleTypeDef *hcan, 
                                   Inverter_t *inv, 
                                   uint8_t pedal_percent, bool torque_allowed,
                                   int16_t *torque_setpoint_prev) {

    if (inv == NULL || torque_setpoint_prev == NULL || hcan == NULL) return;

    InverterErrorRecovery_t *err_ctx = ErrorRecovery_GetCtx(inv);
    if (err_ctx == NULL) return;
    
    const InverterState_t state = inv->state;
    
    if (state == INV_STATE_OFF) return;
    
    if (state == INV_STATE_ERROR) {
        ErrorRecovery_Process(hcan, inv, err_ctx);
        
        *torque_setpoint_prev = 0;
        return;   /* do NOT fall through to normal torque command */
    } else {
        ErrorRecovery_Reset(err_ctx);
    }

    

    bool use_regen = false;

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


        int16_t torque_raw;

        #if defined(REGEN_ENABLED) && REGEN_ENABLED
            use_regen = Regen_ShouldUseRegen(inv, pedal_percent);
            if (use_regen) {
                /* Pedal released: regenerative braking torque (negative).
                 * If we are entering regen from a positive traction state, reset
                 * the rate-limiter prev to 0 immediately.  Without this, the rate
                 * limiter takes up to 280 ms to cross from +2100 to 0 before it can
                 * produce any negative torque, and the AMK receives lim_neg=0 the
                 * entire time — blocking regen altogether. */
                if (*torque_setpoint_prev > 0) {
                    *torque_setpoint_prev = 0;
                }
                torque_raw = Regen_CalculateTorque(
                    inv->speed_rpm,
                    pedal_percent,
                    inv->dc_bus_voltage
                );
            } else {
                /* Pedal pressed: normal traction torque (positive).
                 * Symmetric reset to the regen-entry reset above: if we are
                 * exiting regen the rate-limiter prev is still negative.
                 * Without this reset, it must ramp all the way from the last
                 * negative regen value to 0 before any positive torque is sent,
                 * creating a dead zone (both effective limits = 0) that can last
                 * up to |prev| / TORQUE_RATE_LIMIT_PER_CYCLE cycles (~200 ms at
                 * full regen torque) — felt by the driver as a torque hole. */
                if (*torque_setpoint_prev < 0) {
                    *torque_setpoint_prev = 0;
                }
                torque_raw = (int16_t)((pedal_percent * TORQUE_SETPOINT_MAX) / 100);
            }
        #else
            /* Regen disabled: traction only */
            pedal_percent = (pedal_percent < 10) ? 0U : pedal_percent; /* sanity cap */
            torque_raw = (int16_t)((pedal_percent * TORQUE_SETPOINT_MAX) / 100);
        #endif

        
        torque_request = Motor_ApplyTorqueRateLimit(
            torque_raw,
            *torque_setpoint_prev,
            TORQUE_RATE_LIMIT_PER_CYCLE
        );
        
        #if REVERSE_TORQUE_ENABLED == 1
            torque_request *= -1;
        #endif

        /* Saturation to limits */
        if (torque_request > TORQUE_LIMIT_POS) torque_request = TORQUE_LIMIT_POS;
        if (torque_request < TORQUE_LIMIT_NEG) torque_request = TORQUE_LIMIT_NEG;
        if ((inv->speed_rpm > (int16_t)MOTOR_MAX_SPEED_RPM) ||
            (inv->speed_rpm < -(int16_t)MOTOR_MAX_SPEED_RPM)) {
            torque_request = 0;
        }

        #ifdef ANTI_NEG_WHILESTOPPED
        if (abs(inv->speed_rpm) < 10 && torque_request < 0) {
            torque_request = 0;
        }
        #endif

        inv->torque_request = torque_request;
        if (state != INV_STATE_RUNNING) {
            *torque_setpoint_prev = 0;
            inv->torque_request   = 0;
        } else {
            *torque_setpoint_prev = torque_request;
        }
    }

    int16_t effective_limit_pos;
    int16_t effective_limit_neg;
    if (use_regen) {
        /* Regen is active: always open the negative limit regardless of the sign
         * of torque_request.  Without this, while the rate-limiter is still
         * stepping down through positive values the AMK receives lim_neg=0 and
         * cannot execute any negative torque — it would keep the motor in traction
         * for the full 280 ms ramp.  With lim_pos=0 here the AMK immediately
         * clamps to 0 (coasts) until the setpoint crosses zero, then smoothly
         * ramps through negative values. */
        effective_limit_pos = 0;
        effective_limit_neg = TORQUE_LIMIT_NEG;
    } else if (torque_request > 0) {
        /* Traction mode: only allow positive torque */
        effective_limit_pos = TORQUE_LIMIT_POS;
        effective_limit_neg = 0;
    } else {
        /* Zero torque request (pedal released, no regen).
         *
         * Keep lim_pos open as long as the motor is spinning in either direction,
         * so the AMK can actively brake back to 0 RPM and — critically — prevent
         * a runaway in the negative direction caused by inertia overshooting 0.
         * Without lim_pos > 0, once the motor drifts negative the AMK has no
         * authority to recover: both limits are 0, torque_setpoint is 0, so the
         * motor accelerates freely in reverse driven by back-EMF coupling.
         *
         * lim_neg is kept at 0: we never want the inverter to pull negative torque
         * autonomously when the pedal is released (no regen enabled here). */
        if (inv->speed_rpm != 0) {
            effective_limit_pos = TORQUE_LIMIT_POS;
            effective_limit_neg = 0;
        } else {
            effective_limit_pos = 0;
            effective_limit_neg = 0;
        }
    }

    InverterCommandMsg1_t cmd = Inverter_BuildCommand(
        inv,
        torque_request,
        //TORQUE_LIMIT_POS,
        //TORQUE_LIMIT_NEG
		effective_limit_pos,
        effective_limit_neg
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
