#ifndef __UTILS_H
#define __UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

// Constants for filter configurations
// THRESHOLD: Defines the maximum allowed difference between consecutive readings
// for the threshold filter. Increase this value to ignore smaller variations (reducing sensitivity),
// or decrease it to respond to even minor changes.
#define THRESHOLD 50

// WINDOW_SIZE: Sets the number of samples used in the median filter.
// It is recommended to use an odd number. A larger window provides stronger noise reduction
// but introduces more delay, while a smaller window yields faster response.
#define WINDOW_SIZE 5

// ALPHA: The smoothing factor for the EMA (Exponential Moving Average) filter.
// Lower values (e.g., 0.05) provide more smoothing (but slower response),
// while higher values (e.g., 0.5) yield a faster response with less smoothing.
#define ALPHA 0.5f

// FIR_ORDER: Specifies the number of taps (coefficients) in the FIR filter.
// A higher order improves the frequency response and filtering effectiveness,
// but increases computational complexity and latency.
#define FIR_ORDER 5

/**
 * @brief Number of independent ADC channels to use for pedal calibration.
 *
 * Each channel corresponds to one of the four potentiometer outputs
 * on the accelerator pedal. This must match the number of raw ADC
 * inputs you sample.
 */
#define CALIB_CHANNELS      4

/**
 * @brief Depth of the circular buffer per channel for calibration.
 *
 * During calibration we collect this many consecutive samples per channel
 * and then compute the per-channel minimum (for full travel) or maximum
 * (for zero travel). A size of 10 balances noise rejection with memory use.
 */
#define CALIB_BUFFER_SIZE  10

typedef struct {
    uint16_t previous_output;  // Threshold filter state
    uint16_t median_buffer[WINDOW_SIZE];  // Median filter state
    uint8_t median_index;
    float ema_output;  // EMA filter state
    uint16_t fir_buffer[FIR_ORDER];  // FIR filter state
    uint8_t fir_index;
} FilterState;

typedef struct {
    uint16_t data[CALIB_CHANNELS][CALIB_BUFFER_SIZE];  // storage samples per channel
    uint8_t  head;                                     // next write position
    bool     full;                                     // has the buffer wrapped?
} CalibCircularBuffer;

extern CalibCircularBuffer zeroCalibBuffer; // Circular buffer to collect raw ADC samples for zero (rest) pedal calibration
extern CalibCircularBuffer fullCalibBuffer; // Circular buffer to collect raw ADC samples for full (max travel) pedal calibration

// Generic filtering functions
void Filter_Init(FilterState *state);
uint16_t Threshold_Filter(FilterState *state, uint16_t input);
uint16_t Median_Filter(FilterState *state, uint16_t input);
uint16_t EMA_Filter(FilterState *state, uint16_t input);
uint16_t FIR_Filter(FilterState *state, const float *coeffs, uint16_t input);
uint16_t Combined_Filter(FilterState *state, const float *coeffs, uint16_t input);

// Circular buffer functions to get the minimum and maximum in a buffer of sensor readings (APPs)
void CalibBuffer_Init(CalibCircularBuffer *cb);
void CalibBuffer_Push(CalibCircularBuffer *cb, const uint16_t sample[CALIB_CHANNELS]);
void CalibBuffer_ComputeMax(const CalibCircularBuffer *cb, uint16_t out[CALIB_CHANNELS]);
void CalibBuffer_ComputeMin(const CalibCircularBuffer *cb, uint16_t out[CALIB_CHANNELS]);

// For linear mappings
uint16_t map_unsigned(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);
int16_t map_signed(int16_t x, int16_t in_min, int16_t in_max, int16_t out_min, int16_t out_max);

// Serial communication function
void Display_Message(UART_HandleTypeDef *huart, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* __UTILS_H */
