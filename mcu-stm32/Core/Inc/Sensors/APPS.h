/**
 * @file APPS.h
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief Accelerator Pedal Position Sensors - Signal processing
 * 
 * Manages 2 sensors with different transfer functions
 * due to physical positioning:
 *   - Sensor 1: APPS1 
 *   - Sensor 2: APPS2 
 * 
 * Formula Students (FSAE) rules T11.8 compliant
 * 
 * ADC ranges differ due to physical placement.
 * Shutdown automatically detects open/short on sensors.
 * Final output is the average of normalized 0-100% percentages.
 */

#ifndef APPS_H
#define APPS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "Config.h"
#include <stdbool.h>
#include <stdint.h>

/* ============================================================================
 * APPS Configuration Constants
 * ============================================================================ */

/* APPS_NUM_SENSORS is defined in Config.h to avoid circular include dependencies */

#define APPS_MEDIAN_WINDOW          19U       /**< Samples for median filter (keep odd) */
#define APPS_FILTER_WINDOW          45U       /**< Samples for moving average filter */
#define APPS_DEADZONE_PERCENT       30U      /**< Initial deadzone [%] */

/* ---- Plausibility thresholds  ---- */
#define APPS_IMPLAUSIBILITY_THRESHOLD_PERCENT   10U     /**< Max allowed diff between sensors [%] */
#define APPS_IMPLAUSIBILITY_TIMEOUT_MS          100U    /**< Confirm implausibility after [ms] */
#define APPS_OUT_OF_RANGE_MARGIN_ADC            100U    /**< Margin outside calibration [ADC counts] */
#define APPS_OUT_OF_RANGE_TIMEOUT_MS            100U    /**< Confirm out-of-range after [ms] */
#define APPS_RESET_THRESHOLD_PERCENT            10U      /**< Pedal must be below this to reset error [%] */

/* ---- Calibration defaults (TODO: replace with real measured values---- */
#ifdef TEST_MODE_SINGLE_ADC
    #define APPS1_ADC_MIN           0U
    #define APPS1_ADC_MAX           4095U
    #define APPS2_ADC_MIN           0U
    #define APPS2_ADC_MAX           4095U
#else
    #define APPS1_ADC_MIN           400U
    #define APPS1_ADC_MAX           3200U
    #define APPS2_ADC_MIN           300U
    #define APPS2_ADC_MAX           2800U
#endif

/* ------------ Error codes ---------------- */
#define APPS_ERROR_NONE             0x00U
#define APPS_ERROR_SENSOR_MISMATCH  0x01U   /**< Sensor 1 vs Sensor 2 diff > threshold */
#define APPS_ERROR_OUT_OF_RANGE     0x02U   /**< Sensor ADC value outside calibration */


/* ============================================================================
 * APPS Data Types
 * ============================================================================ */

/**
 * @brief Calibration data for a single sensor
 * 
 * Note: The two sensors have different ADC ranges due to
 * physical positioning (different transfer functions)
 */
typedef struct {
    uint16_t adc_min;       /**< ADC value at pedal released */
    uint16_t adc_max;       /**< ADC value at pedal pressed */
} APPS_Calibration_t;

/**
 * @brief State of a single APPS sensor
 */
typedef struct {
    uint16_t raw;               /**< Last raw value */
    uint16_t filtered;          /**< Value after filter */
    uint8_t  percent;           /**< Percentage 0-100 */
} APPS_SensorState_t;

/**
 * @brief Complete APPS system data (accessible globally via g_apps)
 * 
 * Contains all runtime state including timestamps for plausibility checks.
 * Access directly via g_apps for all APPS information.
 */
typedef struct {
    APPS_SensorState_t  sensors[APPS_NUM_SENSORS];  /**< Per-sensor state */
    APPS_Calibration_t  calibration[APPS_NUM_SENSORS]; /**< Per-sensor calibration */
    uint8_t  final_percent;             /**< Final aggregated percentage */
    bool     implausibility_active;     /**< Implausibility flag */
    uint8_t  error_code;                /**< Error code bitmask */
    bool     torque_allowed;            /**< Torque permission */
    uint32_t implausibility_start_tick; /**< Timestamp of first mismatch detection */
    uint32_t out_of_range_start_tick[APPS_NUM_SENSORS]; /**< Timestamp per sensor */
    uint32_t timestamp;                 /**< Last update tick */
} APPS_Data_t;

/** Global APPS data */
extern APPS_Data_t g_apps;


/* ============================================================================
 * APPS Function Prototypes
 * ============================================================================ */

/**
 * @brief Initialize APPS module (filters, default calibration)
 */
void APPS_Init(void);

/**
 * @brief Update both APPS sensors with new ADC readings
 * @param raw_values Array of 2 raw ADC values [APPS1, APPS2]
 */
void APPS_Update(const uint16_t raw_values[APPS_NUM_SENSORS]);

/**
 * @brief Get final pedal percentage (0-100)
 * @return Aggregated percentage (average of 2 sensors)
 */
uint8_t APPS_GetPercent(void);

/**
 * @brief Get percentage of a single sensor
 * @param sensor_index 0 for Sensor1, 1 for Sensor2
 * @return Percentage 0-100
 */
uint8_t APPS_GetSensorPercent(uint8_t sensor_index);

/**
 * @brief Set calibration for a sensor
 * @param sensor_index Sensor index 0-1
 * @param adc_min ADC value at pedal released
 * @param adc_max ADC value at pedal pressed
 */
void APPS_SetCalibration(uint8_t sensor_index, uint16_t adc_min, uint16_t adc_max);

/**
 * @brief Force reset plausibility state (DEBUG ONLY)
 */
void APPS_ForceReset(void);


#ifdef __cplusplus
}
#endif

#endif /* APPS_H */
