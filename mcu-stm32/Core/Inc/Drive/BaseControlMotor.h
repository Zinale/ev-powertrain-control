/**
 * @file BaseControlMotor.h
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief Basic motor control and inverter management functions
 * 
 * Contains:
 *   - Dynamic torque limit calculation (AMK formula)
 *   - Torque rate limiting
 *   - Status LED management
 *   - Single inverter control
 */

#ifndef BASE_CONTROL_MOTOR_H
#define BASE_CONTROL_MOTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

extern uint32_t g_can_tx_ok_count;     /**< Successful CAN transmissions */
extern uint32_t g_can_tx_fail_count;   /**< Failed CAN transmissions (mailbox full) */
extern uint32_t g_can_rx_count;        /**< Received CAN messages from inverter */

#include "Drive/Inverter.h"
#include <stdint.h>

/* Motor Configuration
 * AMK motor parameters */

/* CAN scale factor (unit conversion) */
#define CAN1_SCALE_FACTOR            10U     /**< 1 unit = 0.1% Mn */

/* Torque limits [unit: 0.1% Mn] */
#define TORQUE_SETPOINT_MAX         2050U   /**< Maximum setpoint = 150% Mn */
#define TORQUE_LIMIT_POS            2050   /**< Positive limit accelerating */
#define TORQUE_LIMIT_NEG           (-2050)  /**< Negative limit (regen braking) */

/* Torque rate limiter */
#define TORQUE_RATE_LIMIT_PER_MS    10U     /**< Max variation per ms [0.1% Mn] */
#define TORQUE_RATE_LIMIT_PER_CYCLE (TORQUE_RATE_LIMIT_PER_MS * CONTROL_LOOP_PERIOD_MS) /**< Scaled for actual task period */

/* AMK motor physical parameters */
#define MOTOR_NOMINAL_TORQUE_NM     9.8f    /**< Nominal torque Mn [Nm] */
#define MOTOR_MAX_TORQUE_NM         21.0f   /**< Maximum torque [Nm] */
#define MOTOR_KE_V_PER_KRPM         18.8f   /**< Back-EMF constant [V/kRPM] */
#define MOTOR_MAX_CURRENT_A         105.0f  /**< Maximum current [A] */
#define MOTOR_EFFICIENCY            0.98f   /**< Estimated efficiency ~95% */

#define MOTOR_MAX_SPEED_RPM         16000U  /**< Maximum speed [RPM] */

/* 
 * REGENERATIVE BRAKING PARAMETERS 
 *
 * Formula:
 *
 * Stage 1 :
 *     T_req_linear = T_max_regen * (1 - pedal_pct / P_threshold)
 *
 * Stage 2 (Speed-dependent fade-out at low speeds):
 *     k_vel = min(1, speed_current / speed_min)
 *     T_req_faded = T_req_linear * k_vel
 *
 * Stage 3 (Power limit from battery - checked later in main logic):
 *     |T_req_limited| <= P_batt_max * 60 / (2π * speed_rpm)
 */

#include "Config.h"

#ifdef REGEN_ENABLED
    #if REGEN_ENABLED
    
    /* Regeneration torque limits (all values in Nm)*/
    #if REGEN_CURRENT_MODE == REGEN_MODE_CONSERVATIVE
        #define REGEN_TORQUE_MAX_NM     (MOTOR_NOMINAL_TORQUE_NM * 0.5f)   /**< 50% = ~5 Nm */
    #elif REGEN_CURRENT_MODE == REGEN_MODE_BALANCED
        #define REGEN_TORQUE_MAX_NM     (MOTOR_NOMINAL_TORQUE_NM * 0.75f)  /**< 75% = ~7.35 Nm */
    #elif REGEN_CURRENT_MODE == REGEN_MODE_AGGRESSIVE
        #define REGEN_TORQUE_MAX_NM     (MOTOR_NOMINAL_TORQUE_NM * 1.0f)   /**< 100% = 9.8 Nm */
    #else
        #define REGEN_TORQUE_MAX_NM     (MOTOR_NOMINAL_TORQUE_NM * 0.75f)  /**< Default: balanced */
    #endif
    
    #endif /* REGEN_ENABLED */
#endif

/**
 * @brief Calculate dynamic torque limit using AMK formula
 * 
 * Formula: M_soll_max = P[W] / (2 × π × N_ist[RPM] / 60)
 * 
 * @param speed_rpm Current speed [RPM]
 * @param dc_voltage DC bus voltage [0.1V]
 * @return Torque limit [0.1% Mn]
 */
int16_t Motor_CalculateDynamicTorqueLimit(int16_t speed_rpm, uint32_t actual_power);

/**
 * @brief Apply rate limiting to requested torque
 * 
 * @param target Target torque [0.1% Mn]
 * @param previous Previous torque [0.1% Mn]
 * @param max_delta Maximum delta per cycle
 * @return Limited torque [0.1% Mn]
 */
int16_t Motor_ApplyTorqueRateLimit(int16_t target, int16_t previous, int16_t max_delta);

/**
 * @brief Update status LEDs with fixed signal mapping.
 * 
 * LD1: APPS implausibility
 * LD2: LEFT inverter (node 2 / inverter204) in error
 * LD3: RIGHT inverter (node 1 / inverter104) in error
 * 
 * @param apps_implausibility APPS implausibility flag
 * @param right_state         RIGHT inverter state
 * @param left_state          LEFT inverter state
 */
void Motor_UpdateStatusLeds(bool apps_implausibility,
                            InverterState_t right_state,
                            InverterState_t left_state);

/**
 * @brief Process single inverter control
 * 
 * Handles: error reset, torque calculation, rate limiting, CAN command send
 * 
 * @param hcan CAN handle to use
 * @param inv Pointer to inverter to control
 * @param pedal_percent Pedal percentage 0-100%
 * @param torque_setpoint_prev Pointer to previous setpoint (for rate limiting)
 */
void Motor_ProcessInverterControl(CAN_HandleTypeDef *hcan, 
                                   Inverter_t *inv, 
                                   uint8_t pedal_percent, bool torque_allowed,
                                   int16_t *torque_setpoint_prev);

#ifdef __cplusplus
}
#endif

#endif /* BASE_CONTROL_MOTOR_H */
