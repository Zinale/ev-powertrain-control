#include "Utils.h"

/**
 * @brief Initialize the filter state structure
 */
void Filter_Init(FilterState *state) {
    state->previous_output = 0;
    state->ema_output = 0.0f;
    state->median_index = 0;
    state->fir_index = 0;

    for (uint8_t i = 0; i < WINDOW_SIZE; i++) {
        state->median_buffer[i] = 0;
    }
    for (uint8_t i = 0; i < FIR_ORDER; i++) {
        state->fir_buffer[i] = 0;
    }
}

/**
 * @brief Threshold filter: Updates the output only if the change exceeds the threshold
 */
uint16_t Threshold_Filter(FilterState *state, uint16_t input) {
    if (abs((int)input - (int)state->previous_output) > THRESHOLD) {
        state->previous_output = input;
    }
    return state->previous_output;
}

/**
 * @brief Median filter with circular insertion and optimized sorting
 */
uint16_t Median_Filter(FilterState *state, uint16_t input) {
    state->median_buffer[state->median_index] = input;
    state->median_index = (state->median_index + 1) % WINDOW_SIZE;

    // Copy and sort the temporary array
    uint16_t temp[WINDOW_SIZE];
    for (uint8_t i = 0; i < WINDOW_SIZE; i++) {
        temp[i] = state->median_buffer[i];
    }

    // Selection Sort to find median value in O(n^2) (better than Bubble Sort)
    for (uint8_t i = 0; i <= WINDOW_SIZE / 2; i++) {
        uint8_t min_idx = i;
        for (uint8_t j = i + 1; j < WINDOW_SIZE; j++) {
            if (temp[j] < temp[min_idx]) {
                min_idx = j;
            }
        }
        uint16_t swap = temp[i];
        temp[i] = temp[min_idx];
        temp[min_idx] = swap;
    }

    return temp[WINDOW_SIZE / 2];
}

/**
 * @brief EMA (Exponential Moving Average) filter
 */
uint16_t EMA_Filter(FilterState *state, uint16_t input) {
    state->ema_output = ALPHA * input + (1.0f - ALPHA) * state->ema_output;
    return (uint16_t)state->ema_output;
}

/**
 * @brief FIR (Finite Impulse Response) filter with circular buffer
 */
uint16_t FIR_Filter(FilterState *state, const float *coeffs, uint16_t input) {
    state->fir_buffer[state->fir_index] = input;
    float output = 0.0f;

    for (uint8_t i = 0; i < FIR_ORDER; i++) {
        output += coeffs[i] * state->fir_buffer[(state->fir_index - i + FIR_ORDER) % FIR_ORDER];
    }

    state->fir_index = (state->fir_index + 1) % FIR_ORDER;
    return (uint16_t)output;
}

/**
 * @brief Apply filters sequentially: Threshold -> Median -> EMA -> FIR
 */
uint16_t Combined_Filter(FilterState *state, const float *coeffs, uint16_t input) {
    uint16_t output = Threshold_Filter(state, input);
    output = Median_Filter(state, output);
    output = EMA_Filter(state, output);
    output = FIR_Filter(state, coeffs, output);
    return output;
}

/**
 * @brief  Initialize the circular buffer (reset head & full flag).
 */
void CalibBuffer_Init(CalibCircularBuffer *cb)
{
    cb->head = 0;
    cb->full = false;
}

/**
 * @brief  Push one multi-channel sample into the circular buffer.
 * @param  cb      Pointer to the buffer struct.
 * @param  sample  Array of CALIB_CHANNELS new samples.
 */
void CalibBuffer_Push(CalibCircularBuffer *cb, const uint16_t sample[CALIB_CHANNELS])
{
    for (int ch = 0; ch < CALIB_CHANNELS; ch++) {
        cb->data[ch][cb->head] = sample[ch];
    }
    cb->head = (cb->head + 1) % CALIB_BUFFER_SIZE;
    if (cb->head == 0) {
        cb->full = true;
    }
}

/**
 * @brief  Compute the maximum value seen on each channel in the buffer.
 *         (Use for “zero” calibration: you want the highest noise-peak.)
 * @param  cb   Pointer to the filled buffer.
 * @param  out  Preallocated array of size CALIB_CHANNELS to receive maxima.
 */
void CalibBuffer_ComputeMax(const CalibCircularBuffer *cb, uint16_t out[CALIB_CHANNELS])
{
    uint8_t count = cb->full ? CALIB_BUFFER_SIZE : cb->head;
    for (int ch = 0; ch < CALIB_CHANNELS; ch++) {
        uint16_t m = cb->data[ch][0];
        for (int i = 1; i < count; i++) {
            if (cb->data[ch][i] > m) {
                m = cb->data[ch][i];
            }
        }
        out[ch] = m;
    }
}

/**
 * @brief  Compute the minimum value seen on each channel in the buffer.
 *         (Use for “full” calibration: you want the lowest noise-dip.)
 * @param  cb   Pointer to the filled buffer.
 * @param  out  Preallocated array of size CALIB_CHANNELS to receive minima.
 */
void CalibBuffer_ComputeMin(const CalibCircularBuffer *cb, uint16_t out[CALIB_CHANNELS])
{
    uint8_t count = cb->full ? CALIB_BUFFER_SIZE : cb->head;
    for (int ch = 0; ch < CALIB_CHANNELS; ch++) {
        uint16_t m = cb->data[ch][0];
        for (int i = 1; i < count; i++) {
            if (cb->data[ch][i] < m) {
                m = cb->data[ch][i];
            }
        }
        out[ch] = m;
    }
}

/**
 * @brief Maps an unsigned integer from one range to another.
 *
 * This function takes an input value `x` that lies within the range [`in_min`, `in_max`]
 * and maps it proportionally to a value within the range [`out_min`, `out_max`].
 * If `x` is less than `in_min`, the function returns `out_min`.
 * If `x` is greater than `in_max`, the function returns `out_max`.
 *
 * @param x The input value to be mapped.
 * @param in_min The lower bound of the input range.
 * @param in_max The upper bound of the input range.
 * @param out_min The lower bound of the output range.
 * @param out_max The upper bound of the output range.
 * @return The mapped value within the output range.
 */
uint16_t map_unsigned(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max) {
    if (x < in_min) return out_min;
    if (x > in_max) return out_max;
    return (uint16_t)((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}

/**
 * @brief Maps a signed integer from one range to another.
 *
 * Similar to `map_unsigned`, this function maps an input value `x` from the range [`in_min`, `in_max`]
 * to a value within the range [`out_min`, `out_max`]. It is designed for signed integers and
 * correctly handles negative values. If `x` is less than `in_min`, the function returns `out_min`.
 * If `x` is greater than `in_max`, the function returns `out_max`.
 *
 * @param x The input value to be mapped.
 * @param in_min The lower bound of the input range.
 * @param in_max The upper bound of the input range.
 * @param out_min The lower bound of the output range.
 * @param out_max The upper bound of the output range.
 * @return The mapped value within the output range.
 */
int16_t map_signed(int16_t x, int16_t in_min, int16_t in_max, int16_t out_min, int16_t out_max) {
    if (x < in_min) return out_min;
    if (x > in_max) return out_max;
    return (int16_t)((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}

/**
 * @brief Transmits a formatted message over the specified UART interface.
 *
 * This function formats a string using a variable argument list and transmits it
 * via the provided UART interface. It dynamically allocates memory for the formatted
 * message and ensures that the allocated memory is freed after transmission.
 * If memory allocation fails, the function exits without attempting transmission.
 *
 * @param huart Pointer to the `UART_HandleTypeDef` structure representing the UART interface.
 * @param format Format string, similar to that used in `printf`, specifying how to format the arguments.
 * @param ... Variable arguments to be formatted into the string.
 */
void Display_Message(UART_HandleTypeDef *huart, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    // Calculate message length and allocate buffer dynamically
    size_t length = vsnprintf(NULL, 0, format, args) + 1;
    char *msg = (char *)malloc(length);
    if (!msg) { va_end(args); return; }

    // Format the message
    vsnprintf(msg, length, format, args);
    va_end(args);

    // Transmit the message and free the allocated memory
    HAL_UART_Transmit(huart, (uint8_t *)msg, length - 1, HAL_MAX_DELAY);
    free(msg);
}

