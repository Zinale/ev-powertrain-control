/**
 * @file SAS.h
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief Steering Angle Sensor - Signal processing
 * 
 * Manages the steering wheel angle sensor for FSAE electric vehicle.
 * Reads from ADC channel 3 (ADC1 PC3) with advanced filtering for noise reduction.
 * 
 * Features:
 *   - Median filter for spike removal
 *   - Moving average for smooth output
 *   - Calibrated angle output in degrees
 *   - Out-of-range detection
 * 
 * USAGE: Access all data through the global variable g_sas
 */

#ifndef SAS_H
#define SAS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "Config.h"
#include <stdbool.h>
#include <stdint.h>


#define SAS_MEDIAN_WINDOW          11U      /**< Samples for median filter (odd number!) */
#define SAS_FILTER_WINDOW          40U      /**< Samples for moving average filter */
#define SAS_DEADZONE_DEGREES       1U       /**< Deadzone around center [degrees] */

/* ADC Calibration Limits */
#define SAS_ADC_MIN                400U     /**< ADC at max left turn (~0.32V) */
#define SAS_ADC_CENTER             2048U    /**< ADC at center position (~1.65V) */
#define SAS_ADC_MAX                3700U    /**< ADC at max right turn (~2.98V) */

/* Angle Limits */
#define SAS_ANGLE_MAX_LEFT         -110    /**< Maximum left angle [degrees] */
#define SAS_ANGLE_MAX_RIGHT        110      /**< Maximum right angle [degrees] */

/* Plausibility Configuration */
#define SAS_OUT_OF_RANGE_MARGIN_ADC    100U    /**< Margin for out-of-range detection [ADC counts] */
#define SAS_OUT_OF_RANGE_TIMEOUT_MS    100U    /**< Time before out-of-range error [ms] */


/**
 * @brief Plausibility error codes (usable as bitmap)
 */
typedef enum {
    SAS_ERROR_NONE          = 0x00,     /**< No error */
    SAS_ERROR_OUT_OF_RANGE  = 0x01      /**< Sensor out of range (open/short circuit) */
} SAS_ErrorCode_t;

/**
 * @brief Calibration data for steering angle sensor
 */
typedef struct {
    uint16_t adc_min;       /**< ADC value at maximum left turn */
    uint16_t adc_center;    /**< ADC value at center position */
    uint16_t adc_max;       /**< ADC value at maximum right turn */
    int16_t  angle_min;     /**< Angle at maximum left turn [degrees] */
    int16_t  angle_max;     /**< Angle at maximum right turn [degrees] */
} SAS_Calibration_t;

/**
 * @brief Global Steering Angle Sensor data structure
 * 
 * Contains all sensor readings, filtered values, and plausibility state.
 * Accessible globally through g_sas variable.
 */
typedef struct {
    uint16_t raw;               /**< Last raw ADC value */
    uint16_t filtered;          /**< Value after median + moving average filter */
    int16_t  angle_deg;         /**< Steering angle in degrees */
    
    SAS_Calibration_t calibration;  /**< ADC and angle limits */
    
    bool     sensor_valid;      /**< true if sensor is within valid range */
    uint8_t  error_code;        /**< Error bitmap (SAS_ErrorCode_t) */
    
    uint32_t timestamp;         /**< Tick at last reading */
    uint32_t out_of_range_start_tick;  /**< Tick when out-of-range started */
    
} SAS_Data_t;


/**
 * @brief Global Steering Angle Sensor data
 * 
 * This variable is accessible from anywhere in the project.
 * Contains all sensor readings, filtered values, and status.
 */
extern SAS_Data_t g_sas;


/**
 * @brief Initialize SAS module (filters, calibration, plausibility checks)
 * 
 * Call this once during system initialization before using SAS_Update().
 */
void SAS_Init(void);

/**
 * @brief Update SAS: read ADC, filter, calculate angle, check plausibility
 * 
 *   1. Update sensor with new ADC reading
 *   2. Apply filters (median + moving average)
 *   3. Calculate steering angle in degrees
 *   4. Run plausibility checks (out-of-range detection)
 *   5. Update g_sas with all results
 * 
 * @param raw_value Raw ADC value from channel 3
 */
void SAS_Update(uint16_t raw_value);

/**
 * @brief Set calibration for steering angle sensor
 * 
 * @param adc_min ADC value at maximum left turn
 * @param adc_center ADC value at center position
 * @param adc_max ADC value at maximum right turn
 * @param angle_min Angle at maximum left turn [degrees] (typically negative)
 * @param angle_max Angle at maximum right turn [degrees] (typically positive)
 */
void SAS_SetCalibration(uint16_t adc_min, uint16_t adc_center, uint16_t adc_max,
                        int16_t angle_min, int16_t angle_max);

/**
 * @brief Force sensor state reset (debug/test only)
 * @warning Do not use in production
 */
void SAS_ForceReset(void);


#define SAS_ANGLE()             (g_sas.angle_deg)           /**< Get steering angle [degrees] */
#define SAS_IS_VALID()          (g_sas.sensor_valid)        /**< Check if sensor is valid */
#define SAS_HAS_ERROR()         (g_sas.error_code != SAS_ERROR_NONE)  /**< Check if error present */
#define SAS_ERROR()             (g_sas.error_code)          /**< Get error code */
#define SAS_RAW()               (g_sas.raw)                 /**< Get raw ADC value */
#define SAS_FILTERED()          (g_sas.filtered)            /**< Get filtered ADC value */


#ifdef __cplusplus
}
#endif

#endif /* SAS_H */
