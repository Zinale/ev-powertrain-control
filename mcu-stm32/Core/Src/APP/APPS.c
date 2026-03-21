/**
 * @file APPS.c
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief APPS signal processing implementation
 * 
 * Manages 2 sensors with different transfer functions
 * due to physical positioning. All data is accessible
 * through the global variable g_apps.
 */

#include "APP/APPS.h"
#include <string.h>

APPS_Data_t g_apps = {0};


/** Median filter buffer for each sensor */
static uint16_t s_median_buffer[APPS_NUM_SENSORS][APPS_MEDIAN_WINDOW];
static uint8_t  s_median_index[APPS_NUM_SENSORS];
static uint8_t  s_median_count[APPS_NUM_SENSORS];

/** Moving average filter buffer for each sensor */
static uint16_t s_filter_buffer[APPS_NUM_SENSORS][APPS_FILTER_WINDOW];
static uint8_t  s_filter_index[APPS_NUM_SENSORS];
static uint8_t  s_filter_count[APPS_NUM_SENSORS];


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
 * @brief Apply median filter to a sensor (removes spikes/glitches)
 */
static uint16_t apply_median_filter(uint8_t sensor, uint16_t value)
{
    if (sensor >= APPS_NUM_SENSORS) return value;
    
    /* Insert into circular buffer */
    s_median_buffer[sensor][s_median_index[sensor]] = value;
    s_median_index[sensor] = (s_median_index[sensor] + 1) % APPS_MEDIAN_WINDOW;
    
    if (s_median_count[sensor] < APPS_MEDIAN_WINDOW) {
        s_median_count[sensor]++;
    }
    
    /* Copy buffer for sorting */
    uint16_t sorted[APPS_MEDIAN_WINDOW];
    for (uint8_t i = 0; i < s_median_count[sensor]; i++) {
        sorted[i] = s_median_buffer[sensor][i];
    }
    
    /* Sort and return median */
    sort_array(sorted, s_median_count[sensor]);
    return sorted[s_median_count[sensor] / 2];
}

/**
 * @brief Apply moving average filter to a sensor
 */
static uint16_t apply_moving_average(uint8_t sensor, uint16_t value)
{
    if (sensor >= APPS_NUM_SENSORS) return value;
    
    /* Insert into circular buffer */
    s_filter_buffer[sensor][s_filter_index[sensor]] = value;
    s_filter_index[sensor] = (s_filter_index[sensor] + 1) % APPS_FILTER_WINDOW;
    
    if (s_filter_count[sensor] < APPS_FILTER_WINDOW) {
        s_filter_count[sensor]++;
    }
    
    /* Calculate average */
    uint32_t sum = 0;
    for (uint8_t i = 0; i < s_filter_count[sensor]; i++) {
        sum += s_filter_buffer[sensor][i];
    }
    
    return (uint16_t)(sum / s_filter_count[sensor]);
}

/**
 * @brief Apply filter chain: Median -> Moving Average
 */
static uint16_t apply_filter(uint8_t sensor, uint16_t value)
{
    uint16_t median_out = apply_median_filter(sensor, value);
    return apply_moving_average(sensor, median_out);
}

/**
 * @brief Normalize ADC value to percentage 0-100
 */
static uint8_t normalize_to_percent(uint16_t filtered, const APPS_Calibration_t *cal)
{
    if (filtered <= cal->adc_min) return 0;
    if (filtered >= cal->adc_max) return 100;
    
    uint32_t range = cal->adc_max - cal->adc_min;
    uint32_t value = filtered - cal->adc_min;
    uint32_t percent = (value * 100U) / range;
    
#ifndef APPS_DISABLE_DEADZONE
    /* Apply deadzone */
    if (percent <= APPS_DEADZONE_PERCENT) {
        return 0;
    }
    
    /* Rescale after deadzone */
    uint32_t rescaled = ((percent - APPS_DEADZONE_PERCENT) * 100U) / 
                        (100U - APPS_DEADZONE_PERCENT);
    
    if (rescaled > 100U) rescaled = 100U;
    
    return (uint8_t)rescaled;
#else
    /* if APPS_DISABLE_DEADZONE is defined = TEST MODE: No deadzone, return raw percentage */
    if (percent > 100U) percent = 100U;
    return (uint8_t)percent;
#endif
}


/**
 * @brief Check difference between the two sensors (T11.8.9)
 * 
 * @return true  = Sensors OK (diff <= 10% or diff > 10% but for less than 100ms)
 * @return false = IMPLAUSIBILITY! (diff > 10% for over 100ms consecutive)
 */
#ifdef APPS_TEST_SINGLE_SENSOR
/* In single sensor test mode, skip mismatch check */
__attribute__((unused))
#endif
static bool check_sensor_mismatch(uint8_t sensor1_pct, uint8_t sensor2_pct)
{
    uint8_t diff = (sensor1_pct > sensor2_pct) ? 
                   (sensor1_pct - sensor2_pct) : 
                   (sensor2_pct - sensor1_pct);
    
    if (diff > APPS_IMPLAUSIBILITY_THRESHOLD_PERCENT) {
        /* Excessive difference detected */
        if (g_apps.implausibility_start_tick == 0) {
            /* First detection: start timer */
            g_apps.implausibility_start_tick = HAL_GetTick();
        } else if ((HAL_GetTick() - g_apps.implausibility_start_tick) > APPS_IMPLAUSIBILITY_TIMEOUT_MS) {
            /* Timer expired: ERROR CONFIRMED */
            return false;
        }
        /* Timer running: still waiting for confirm */
    } else {
        /* Reset timer */
        g_apps.implausibility_start_tick = 0;
    }
    
    return true;
}

/**
 * @brief Check if a sensor is out of range (broken wire/short circuit)
 * 
 * Verifies that the ADC value is within calibration limits ± margin.
 * Ex: if calibration = [400, 3200] and margin = 100, accepts [300, 3300]
 * 
 * @param sensor_index Sensor index (0 or 1)
 * @param raw_value    Raw ADC value to verify
 * 
 * @return true  = Sensor OK (in range or out of range but for less than 100ms)
 * @return false = IMPLAUSIBILITY! (out of range for over 100ms consecutive)
 */
#ifdef APPS_TEST_SINGLE_SENSOR
__attribute__((unused))
#endif
static bool check_out_of_range(uint8_t sensor_index, uint16_t raw_value)
{
    if (sensor_index >= APPS_NUM_SENSORS) return true;
    
    uint16_t adc_min = g_apps.calibration[sensor_index].adc_min;
    uint16_t adc_max = g_apps.calibration[sensor_index].adc_max;
    
    uint16_t low_limit = (adc_min > APPS_OUT_OF_RANGE_MARGIN_ADC) ? 
                         (adc_min - APPS_OUT_OF_RANGE_MARGIN_ADC) : 0;
    uint16_t high_limit = adc_max + APPS_OUT_OF_RANGE_MARGIN_ADC;
    
    if (raw_value < low_limit || raw_value > high_limit) {
        if (g_apps.out_of_range_start_tick[sensor_index] == 0) {
            g_apps.out_of_range_start_tick[sensor_index] = HAL_GetTick();
        } else if ((HAL_GetTick() - g_apps.out_of_range_start_tick[sensor_index]) > APPS_OUT_OF_RANGE_TIMEOUT_MS) {
            return false;
        }
    } else {
        g_apps.out_of_range_start_tick[sensor_index] = 0;
    }
    
    return true;
}

/**
 * @brief Check error reset conditions (pedal released)
 */
static bool check_reset_condition(void)
{
    bool app1 = g_apps.sensors[0].percent <= APPS_RESET_THRESHOLD_PERCENT;
    bool app2 = false;
    #ifndef TEST_MODE_SINGLE_ADC
    app2 = g_apps.sensors[1].percent <= APPS_RESET_THRESHOLD_PERCENT;
    #endif
    return app1 && app2;
}

/**
 * @brief Perform all plausibility checks
 */
static void run_plausibility_checks(void)
{
    uint8_t new_error_code = APPS_ERROR_NONE;
    #ifdef APPS_TEST_SINGLE_SENSOR
    __attribute__((unused))
    #endif
    uint8_t active_sensors = APPS_NUM_SENSORS;
    
    #ifdef APPS_TEST_SINGLE_SENSOR
        active_sensors = 1;
    #endif

    
    #ifndef APPS_DISABLE_MISMATCH_CHECK
        /* CHECK 1: Sensor mismatch */
        if (!check_sensor_mismatch(g_apps.sensors[0].percent, g_apps.sensors[1].percent)) {
            new_error_code |= APPS_ERROR_SENSOR_MISMATCH;
        }
    #endif
    
    #ifndef APPS_DISABLE_RANGE_CHECK
        /* CHECK 2: Out-of-range for each sensor */
        for (uint8_t s = 0; s < active_sensors; s++) {
            if (!check_out_of_range(s, g_apps.sensors[s].raw)) {
                new_error_code |= APPS_ERROR_OUT_OF_RANGE;
            }
        }
    #endif
    
    /* Update error state */
    if (new_error_code != APPS_ERROR_NONE) {
        g_apps.implausibility_active = true;
        g_apps.error_code = new_error_code;
        g_apps.torque_allowed = false;
    }
    
    /* CHECK RESET: */
    if (g_apps.implausibility_active) {
        g_apps.torque_allowed = false;
        
        if (check_reset_condition() && new_error_code == APPS_ERROR_NONE) {
        //if (new_error_code == APPS_ERROR_NONE) {
            g_apps.implausibility_active = false;
            g_apps.error_code = APPS_ERROR_NONE;
            g_apps.torque_allowed = true;
        }
    }
}


void APPS_Init(void)
{
    memset(&g_apps, 0, sizeof(g_apps));
    
    memset(s_median_buffer, 0, sizeof(s_median_buffer));
    memset(s_median_index, 0, sizeof(s_median_index));
    memset(s_median_count, 0, sizeof(s_median_count));
    memset(s_filter_buffer, 0, sizeof(s_filter_buffer));
    memset(s_filter_index, 0, sizeof(s_filter_index));
    memset(s_filter_count, 0, sizeof(s_filter_count));
    
    g_apps.calibration[0].adc_min = APPS1_ADC_MIN;
    g_apps.calibration[0].adc_max = APPS1_ADC_MAX;
#ifndef TEST_MODE_SINGLE_ADC
    g_apps.calibration[1].adc_min = APPS2_ADC_MIN;
    g_apps.calibration[1].adc_max = APPS2_ADC_MAX;
#endif
    g_apps.torque_allowed = true;
}


void APPS_Update(const uint16_t raw_values[APPS_NUM_SENSORS])
{
    if (raw_values == NULL) return;
    
    g_apps.timestamp = HAL_GetTick();
    
    uint32_t percent_sum = 0;
__attribute__((unused)) uint8_t active_sensors = APPS_NUM_SENSORS;
    
    #ifdef APPS_TEST_SINGLE_SENSOR
        /* TEST MODE: Use only sensor 0 */
        active_sensors = 1;
    #endif
    
    for (uint8_t s = 0; s < active_sensors; s++) {
        APPS_SensorState_t *sensor = &g_apps.sensors[s];
        
        sensor->raw = raw_values[s];
        sensor->filtered = apply_filter(s, sensor->raw);
        sensor->percent = normalize_to_percent(sensor->filtered, &g_apps.calibration[s]);
        
        percent_sum += sensor->percent;
    }
    
    g_apps.final_percent = (uint8_t)(percent_sum / active_sensors);
    
    /* Run plausibility checks */
    run_plausibility_checks();
}


void APPS_SetCalibration(uint8_t sensor_index, uint16_t adc_min, uint16_t adc_max)
{
    if (sensor_index >= APPS_NUM_SENSORS) return;
    if (adc_min >= adc_max) return;
    
    g_apps.calibration[sensor_index].adc_min = adc_min;
    g_apps.calibration[sensor_index].adc_max = adc_max;
}


void APPS_ForceReset(void)
{
    /* WARNING: Debug only. Resets plausibility state unconditionally */
    g_apps.implausibility_active = false;
    g_apps.error_code = APPS_ERROR_NONE;
    g_apps.torque_allowed = true;
    g_apps.implausibility_start_tick = 0;
    g_apps.out_of_range_start_tick[0] = 0;
#ifndef TEST_MODE_SINGLE_ADC
    g_apps.out_of_range_start_tick[1] = 0;
#endif
}


uint8_t APPS_GetPercent(void)
{
    return g_apps.final_percent;
}


uint8_t APPS_GetSensorPercent(uint8_t sensor_index)
{
    if (sensor_index >= APPS_NUM_SENSORS) return 0;
    return g_apps.sensors[sensor_index].percent;
}
