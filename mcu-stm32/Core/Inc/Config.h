/**
 * @file Config.h
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief Global configuration and build options
 * 
 * This file contains compile-time configuration switches for
 * different build modes (test, debug, release).
 * 
 */

#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif


/* =========================================================
 * Serial logging enable/disable
 *
 * Comment out a define to completely silence that UART channel.
 * The corresponding Serial5_Log / Serial3_Log calls compile to
 * a single (void)format statement — no code generated, no CPU cost.
 * ========================================================= */
#define SERIAL5_LOG_ENABLED   /**< UART5 — PC debug / USB-UART adapter */
#define SERIAL3_LOG_ENABLED   /**< UART3 — Dashboard / external device  */



/* =========================================================
 * DATA COLLECTION MODE
 *
 * When defined, data_logger outputs a SINGLE CSV line on
 * UART5 only (no verbose text). Designed for bench logging
 * through ESP32 → WiFi → Python script.
 *
 * STM32 CSV column order (left inverter):
 *   TempMotor[°C], TempInverter[°C], TempIGBT[°C],
 *   Speed[RPM], CurrentMotor[A], TorqueMotor[Nm], PedalPerc[%]
 *
 * ESP32 appends: NTC1, NTC2, NTC3
 * Python script appends: Timestamp
 * ========================================================= */
#define DATA_COLLECT_MODE

/** Sampling period [ms] when DATA_COLLECT_MODE is active */
#define DATA_COLLECT_PERIOD_MS      1000U




#define APPS_NUM_SENSORS            1U

/* ============================================================================
 * BUILD MODE CONFIGURATION
 * ============================================================================
 * Uncomment ONE of the following to select the build mode:
 * 
 * - RELEASE_MODE:         Release/race mode - FULL SAFETY ACTIVE
 *                         - 3 complete ADC channels (APPS1, APPS2, STEERING)
 *                         - Plausibility check APPS1 vs APPS2 active
 *                         - Range check sensors active
 *                         - Deadzone 30% active    
 *                         - Timeout implausibility active
 *                         - Requires complete hardware and accurate calibration!
 * 
 * - TEST_MODE_SINGLE_ADC: Only ADC1 CH0 (PA0) is available for testing.
 *                         APPS uses only sensor 1, plausibility checks disabled.
 * 
 * - TEST_MODE_FULL:       All ADC channels available, but plausibility
 *                         checks are more lenient for bench testing.
 * 
 * ============================================================================ */




/* =============================================================================
 * BUILD MODE SELECTION
   SELECT BUILD MODE (UNCOMMENT ONE)

 * RELEASE_MODE:
*/
#define TEST_MODE_SINGLE_ADC
//define TEST_MODE_FULL        
//#define RELEASE_MODE


#ifdef TEST_MODE_SINGLE_ADC
    #define ADC_TEST_NUM_CHANNELS       1U
    
    #define APPS_TEST_SINGLE_SENSOR     1
    
    #define APPS_DISABLE_MISMATCH_CHECK 1
    
    #define APPS_DISABLE_RANGE_CHECK    1  /* Disable out-of-range check in test */
    
    //#define APPS_DISABLE_DEADZONE 
    
    /* NOTE: In this mode:
     * - Only PA0 (ADC1_CH0) connected
     * - APPS2 duplicated from APPS1 internally
     * - Calibration in APPS.h set to 0-4095 to see full range
     * - Deadzone 30% ACTIVE for realistic pedal behavior (ffor currently linear potentiometer)
     * - TODO: After testing, update calibration with real measured values!
     */
    
#elif defined(TEST_MODE_FULL)
    #define ADC_TEST_NUM_CHANNELS       3U
    
    #define APPS_TEST_IMPLAUSIBILITY_THRESHOLD  10U
    #define APPS_TEST_IMPLAUSIBILITY_TIMEOUT    100U
    
    
#elif defined(RELEASE_MODE)
    /* Release mode: full safety, all channels, production-ready */
    #define ADC_RELEASE_NUM_CHANNELS    3U
    
    /* No test warnings in release mode */
    
    /* All APPS safety checks enabled by default:
     * - APPS_DISABLE_MISMATCH_CHECK: NOT defined -> plausibility check ACTIVE
     * - APPS_DISABLE_RANGE_CHECK: NOT defined -> range check ACTIVE  
     * - APPS_DISABLE_DEADZONE: NOT defined -> deadzone 30% ACTIVE
     * - Timeout implausibility: ACTIVE
     */
    
#else
    #error "No mode selected! Choose TEST_MODE_SINGLE_ADC, TEST_MODE_FULL or RELEASE_MODE in Config.h"
#endif


/* ADC CHANNEL CONFIGURATION  */
#if defined(TEST_MODE_SINGLE_ADC)
    #define ADC_NUM_CHANNELS            ADC_TEST_NUM_CHANNELS
    #define ADC_SCAN_MODE               ADC_SCAN_DISABLE
#elif defined(TEST_MODE_FULL) || defined(RELEASE_MODE)
    /* All 3 ADC channels enabled (APPS1, APPS2, Steering) */
    #define ADC_NUM_CHANNELS            3U
    #define ADC_SCAN_MODE               ADC_SCAN_ENABLE
#endif


/* 
 * REGENERATIVE BRAKING CONFIGURATION
 */

/** Control-loop period used by the inverter task [ms].  Keep in sync
 *  with INVERTERS_TASK_PERIOD_MS in inverters_manage.c.              */
#define CONTROL_LOOP_PERIOD_MS      5U

/* =========================================================
 * Inverter bench configuration
 *
 * Use these flags to switch test-bench topology quickly:
 * - *_PRESENT: inverter physically connected on CAN
 * - *_CONTROL_ENABLED: torque control loop enabled for that side
 *
 * Typical setups:
 * - Single motor on RIGHT inverter:
 *     RIGHT_PRESENT=1, LEFT_PRESENT=1, RIGHT_CONTROL=1, LEFT_CONTROL=0
 * - Single motor on LEFT inverter:
 *     RIGHT_PRESENT=1, LEFT_PRESENT=1, RIGHT_CONTROL=0, LEFT_CONTROL=1
 * - Dual motor:
 *     RIGHT_PRESENT=1, LEFT_PRESENT=1, RIGHT_CONTROL=1, LEFT_CONTROL=1
 * ========================================================= */
#define INVERTER_RIGHT_PRESENT                 1U
#define INVERTER_LEFT_PRESENT                  1U
#define INVERTER_RIGHT_TORQUE_CONTROL_ENABLED  1U
#define INVERTER_LEFT_TORQUE_CONTROL_ENABLED   0U

/* DATA_COLLECT inverter selection:
 * 0 = AUTO (if only one side is control-enabled, pick that side; otherwise RIGHT)
 * 1 = force RIGHT
 * 2 = force LEFT
 */
#define DATA_COLLECT_INV_SIDE_AUTO   0U
#define DATA_COLLECT_INV_SIDE_RIGHT  1U
#define DATA_COLLECT_INV_SIDE_LEFT   2U
#define DATA_COLLECT_INV_SIDE        DATA_COLLECT_INV_SIDE_RIGHT

#if (INVERTER_RIGHT_TORQUE_CONTROL_ENABLED && !INVERTER_RIGHT_PRESENT)
    #error "Right torque control enabled but right inverter is not present"
#endif

#if (INVERTER_LEFT_TORQUE_CONTROL_ENABLED && !INVERTER_LEFT_PRESENT)
    #error "Left torque control enabled but left inverter is not present"
#endif

#if ((DATA_COLLECT_INV_SIDE != DATA_COLLECT_INV_SIDE_AUTO) && \
     (DATA_COLLECT_INV_SIDE != DATA_COLLECT_INV_SIDE_RIGHT) && \
     (DATA_COLLECT_INV_SIDE != DATA_COLLECT_INV_SIDE_LEFT))
    #error "Invalid DATA_COLLECT_INV_SIDE value"
#endif

#define REGEN_ENABLED               0U     /**< 1 = enabled, 0 = disabled */

/* Regenerative braking strategy */
#ifdef REGEN_ENABLED
    #if REGEN_ENABLED
        /* Regeneration modes: adjust aggressiveness based on needs */
        #define REGEN_MODE_CONSERVATIVE    0U   /**< 50% of max regen torque */
        #define REGEN_MODE_BALANCED        1U   /**< 75% of max regen torque (default) */
        #define REGEN_MODE_AGGRESSIVE      2U   /**< 100% of max regen torque */
        
        #define REGEN_CURRENT_MODE         REGEN_MODE_CONSERVATIVE
        
        /* Battery safety limits (80 kW  600V) */
        #define REGEN_PBATT_MAX_W           80000U   /**< Battery max regen power [W] */
        #define REGEN_DC_BUS_NOMINAL_V      600U     /**< DC bus nominal voltage [0.1V] = 600V */
        
        /* Pedal threshold: below this %, regeneration is active */
        #define REGEN_PEDAL_THRESHOLD_PCT   20U      /**< Regen active when pedal < 20% */
        
        /* Motor speed thresholds */
        #define REGEN_SPEED_MIN_RPM         1000U      /**< Below this: fade-out regen */
        #define REGEN_SPEED_CRITICAL_RPM    400U      /**< Below this: NO regen */
        
        /* Logging */
        #define REGEN_LOG_ENABLED           1U       /**< Log regen events to serial */
    #endif
#endif

#ifdef __cplusplus
}
#endif

#endif 
