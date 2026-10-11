#include "Sensors/filters.h"

static void sort_u16(uint16_t *arr, uint8_t len) {
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

void Filter_Median_Init(MedianFilter_t *f, uint8_t window_size) {
    if (!f) return;
    memset(f->buffer, 0, sizeof(f->buffer));
    f->window_size = (window_size > MAX_MEDIAN_WINDOW) ? MAX_MEDIAN_WINDOW : window_size;
    if ((f->window_size % 2) == 0) f->window_size--; /* Forza numero dispari per il mediano */
    f->index = 0;
    f->count = 0;
}

uint16_t Filter_Median_Apply(MedianFilter_t *f, uint16_t new_val) {
    if (!f || f->window_size == 0) return new_val;

    f->buffer[f->index] = new_val;
    f->index = (f->index + 1) % f->window_size;
    if (f->count < f->window_size) f->count++;

    uint16_t sorted[MAX_MEDIAN_WINDOW];
    memcpy(sorted, f->buffer, f->count * sizeof(uint16_t));
    sort_u16(sorted, f->count);

    return sorted[f->count / 2];
}

void Filter_MovingAverage_Init(MovingAverageFilter_t *f, uint8_t window_size) {
    if (!f) return;
    memset(f->buffer, 0, sizeof(f->buffer));
    f->window_size = (window_size > MAX_MA_WINDOW) ? MAX_MA_WINDOW : window_size;
    f->index = 0;
    f->count = 0;
    f->sum = 0;
}

uint16_t Filter_MovingAverage_Apply(MovingAverageFilter_t *f, uint16_t new_val) {
    if (!f || f->window_size == 0) return new_val;

    if (f->count < f->window_size) {
        f->sum += new_val;
        f->buffer[f->index] = new_val;
        f->count++;
    } else {
        f->sum = f->sum - f->buffer[f->index] + new_val;
        f->buffer[f->index] = new_val;
    }
    f->index = (f->index + 1) % f->window_size;

    return (uint16_t)(f->sum / f->count);
}