/**
 * @file SAS.c
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief Steering Angle Sensor - Signal processing implementation
 * 
 * Manages steering wheel angle sensor with advanced filtering.
 * All data is accessible through the global variable g_sas.
 */

#include "SAS/SAS.h"
#include <string.h>

SAS_Data_t g_sas = {0};

/** Median filter buffer */
static uint16_t s_median_buffer[SAS_MEDIAN_WINDOW];
static uint8_t  s_median_index = 0;
static uint8_t  s_median_count = 0;

/** Moving average filter buffer */
static uint16_t s_filter_buffer[SAS_FILTER_WINDOW];
static uint8_t  s_filter_index = 0;
static uint8_t  s_filter_count = 0;

/**
 * @brief Sort array for median filter (insertion sort, optimal for small N)
 */
static void sort_array(uint16_t *arr, uint8_t len)
{
    for (uint8_t i = 1; i < len; i++) {
        uint16_t key = arr[i];
        int8_t j = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

/**
 * @brief Apply median filter (removes spikes/glitches)
 */
static uint16_t apply_median_filter(uint16_t value)
{
    /* Insert into circular buffer */
    s_median_buffer[s_median_index] = value;
    s_median_index = (s_median_index + 1) % SAS_MEDIAN_WINDOW;
    
    if (s_median_count < SAS_MEDIAN_WINDOW) {
        s_median_count++;
    }
    
    uint16_t sorted[SAS_MEDIAN_WINDOW];
    for (uint8_t i = 0; i < s_median_count; i++) {
        sorted[i] = s_median_buffer[i];
    }
    
    /* Sort and return median */
    sort_array(sorted, s_median_count);
    return sorted[s_median_count / 2];
}

/**
 * @brief Apply moving average filter
 */
static uint16_t apply_moving_average(uint16_t value)
{
    /* Insert into circular buffer */
    s_filter_buffer[s_filter_index] = value;
    s_filter_index = (s_filter_index + 1) % SAS_FILTER_WINDOW;
    
    if (s_filter_count < SAS_FILTER_WINDOW) {
        s_filter_count++;
    }
    
    /* Calculate average */
    uint32_t sum = 0;
    for (uint8_t i = 0; i < s_filter_count; i++) {
        sum += s_filter_buffer[i];
    }
    
    return (uint16_t)(sum / s_filter_count);
}

/**
 * @brief Apply filter chain: Median -> Moving Average
 */
static uint16_t apply_filter(uint16_t value)
{
    uint16_t median_out = apply_median_filter(value);
    return apply_moving_average(median_out);
}

/**
 * @brief Convert ADC value to steering angle in degrees
 */
static int16_t adc_to_angle(uint16_t filtered, const SAS_Calibration_t *cal)
{
    int32_t angle;
    
    if (filtered < cal->adc_center) {
        /* Left turn (negative angle) */
        if (filtered <= cal->adc_min) {
            return cal->angle_min;
        }
        
        /* Linear interpolation: adc_min -> adc_center maps to angle_min -> 0 */
        int32_t adc_range = cal->adc_center - cal->adc_min;
        int32_t angle_range = 0 - cal->angle_min;  /* positive value */
        int32_t adc_offset = filtered - cal->adc_min;
        
        angle = cal->angle_min + ((adc_offset * angle_range) / adc_range);
        
    } else {
        /* Right turn (positive angle) */
        if (filtered >= cal->adc_max) {
            return cal->angle_max;
        }
        
        /* Linear interpolation: adc_center -> adc_max maps to 0 -> angle_max */
        int32_t adc_range = cal->adc_max - cal->adc_center;
        int32_t angle_range = cal->angle_max - 0;
        int32_t adc_offset = filtered - cal->adc_center;
        
        angle = 0 + ((adc_offset * angle_range) / adc_range);
    }
    
    /* Apply deadzone around center */
    if (angle > -SAS_DEADZONE_DEGREES && angle < SAS_DEADZONE_DEGREES) {
        angle = 0;
    }
    
    return (int16_t)angle;
}

/**
 * @brief Check if sensor value is within valid range
 * 
 * @return true  = Sensor OK (within range or out-of-range for < 100ms)
 * @return false = ERROR! (out of range for > 100ms)
 */
static bool check_out_of_range(uint16_t raw_value)
{
    const uint16_t lower_limit = SAS_ADC_MIN - SAS_OUT_OF_RANGE_MARGIN_ADC;
    const uint16_t upper_limit = SAS_ADC_MAX + SAS_OUT_OF_RANGE_MARGIN_ADC;
    
    if (raw_value < lower_limit || raw_value > upper_limit) {
        if (g_sas.out_of_range_start_tick == 0) {
            g_sas.out_of_range_start_tick = HAL_GetTick();
        } else if ((HAL_GetTick() - g_sas.out_of_range_start_tick) > SAS_OUT_OF_RANGE_TIMEOUT_MS) {
            return false;
        }
    } else {
        g_sas.out_of_range_start_tick = 0;
    }
    
    return true;
}


/**
 * @brief Initialize SAS module (filters, calibration, plausibility)
 */
void SAS_Init(void)
{
    memset(&g_sas, 0, sizeof(g_sas));
    
    g_sas.calibration.adc_min = SAS_ADC_MIN;
    g_sas.calibration.adc_center = SAS_ADC_CENTER;
    g_sas.calibration.adc_max = SAS_ADC_MAX;
    g_sas.calibration.angle_min = SAS_ANGLE_MAX_LEFT;
    g_sas.calibration.angle_max = SAS_ANGLE_MAX_RIGHT;
    
    g_sas.sensor_valid = true;
    g_sas.error_code = SAS_ERROR_NONE;
    
    memset(s_median_buffer, 0, sizeof(s_median_buffer));
    memset(s_filter_buffer, 0, sizeof(s_filter_buffer));
    s_median_index = 0;
    s_median_count = 0;
    s_filter_index = 0;
    s_filter_count = 0;
}

/**
 * @brief Update SAS: read ADC, filter, calculate angle, check plausibility
 */
void SAS_Update(uint16_t raw_value)
{
    g_sas.timestamp = HAL_GetTick();
    
    g_sas.raw = raw_value;
    
    g_sas.filtered = apply_filter(raw_value);
    
    g_sas.angle_deg = adc_to_angle(g_sas.filtered, &g_sas.calibration);
    
    bool range_ok = check_out_of_range(raw_value);
    
    if (!range_ok) {
        g_sas.sensor_valid = false;
        g_sas.error_code |= SAS_ERROR_OUT_OF_RANGE;
    } else {
        g_sas.sensor_valid = true;
        g_sas.error_code &= ~SAS_ERROR_OUT_OF_RANGE;
    }
}

/**
 * @brief Set calibration for steering angle sensor
 */
void SAS_SetCalibration(uint16_t adc_min, uint16_t adc_center, uint16_t adc_max,
                        int16_t angle_min, int16_t angle_max)
{
    g_sas.calibration.adc_min = adc_min;
    g_sas.calibration.adc_center = adc_center;
    g_sas.calibration.adc_max = adc_max;
    g_sas.calibration.angle_min = angle_min;
    g_sas.calibration.angle_max = angle_max;
}

/**
 * @brief Force sensor state reset (debug/test only)
 */
void SAS_ForceReset(void)
{
    g_sas.error_code = SAS_ERROR_NONE;
    g_sas.sensor_valid = true;
    g_sas.out_of_range_start_tick = 0;
}
