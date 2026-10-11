#ifndef FILTERS_H
#define FILTERS_H

#include <stdint.h>
#include <string.h>

#define MAX_MEDIAN_WINDOW   21U
#define MAX_MA_WINDOW       64U

typedef struct {
    uint16_t buffer[MAX_MEDIAN_WINDOW];
    uint8_t  window_size;
    uint8_t  index;
    uint8_t  count;
} MedianFilter_t;

typedef struct {
    uint16_t buffer[MAX_MA_WINDOW];
    uint8_t  window_size;
    uint8_t  index;
    uint8_t  count;
    uint32_t sum;
} MovingAverageFilter_t;

void Filter_Median_Init(MedianFilter_t *f, uint8_t window_size);
uint16_t Filter_Median_Apply(MedianFilter_t *f, uint16_t new_val);

void Filter_MovingAverage_Init(MovingAverageFilter_t *f, uint8_t window_size);
uint16_t Filter_MovingAverage_Apply(MovingAverageFilter_t *f, uint16_t new_val);

#endif