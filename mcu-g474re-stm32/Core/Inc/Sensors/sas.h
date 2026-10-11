#ifndef SAS_H
#define SAS_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

#define SAS_MEDIAN_WINDOW_SIZE          11U
#define SAS_MA_WINDOW_SIZE              32U
#define SAS_ADC_MIN                     400U
#define SAS_ADC_CENTER                  2048U
#define SAS_ADC_MAX                     3700U
#define SAS_ANGLE_MAX_LEFT_DEG          -110
#define SAS_ANGLE_MAX_RIGHT_DEG         110
#define SAS_DEADZONE_DEG                1U
#define SAS_OUT_OF_RANGE_MARGIN_ADC     100U
#define SAS_OUT_OF_RANGE_TIMEOUT_MS     100U

typedef struct {
    uint16_t raw_adc;
    uint16_t filtered_adc;
    int16_t  angle_deg;
    bool     fault_out_of_range;
} SAS_Output_t;

void SAS_Init(void);
void SAS_Process(uint16_t raw_adc, SAS_Output_t *out);

#endif /* SAS_H */