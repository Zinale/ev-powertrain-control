/*
 * Core/Inc/Sensors/apps.h
 * Header file for the Accelerator Pedal Position Sensor (APPS) module.
 */
#ifndef APPS_H
#define APPS_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

#define APPS_MEDIAN_WINDOW_SIZE         19U
#define APPS_MA_WINDOW_SIZE             32U
#define APPS1_ADC_MIN                   400U
#define APPS1_ADC_MAX                   3200U
#define APPS2_ADC_MIN                   300U
#define APPS2_ADC_MAX                   2800U
#define APPS_DEADZONE_PERCENT           0U
#define APPS_RESET_THRESHOLD_PERCENT    5U
#define APPS_MAX_DISCREPANCY_PERCENT    10U
#define APPS_IMPLAUSIBILITY_TIMEOUT_MS  100U
#define APPS_OUT_OF_RANGE_MARGIN_ADC    100U
#define APPS_OUT_OF_RANGE_TIMEOUT_MS    100U

typedef struct {
    uint16_t raw1;
    uint16_t raw2;
    uint8_t  percent1;
    uint8_t  percent2;
    uint8_t  final_percent;
    bool     fault_mismatch;
    bool     fault_oor1;            /* APPS1 fuori range (debounced) */
    bool     fault_oor2;            /* APPS2 fuori range (debounced) */
    bool     fault_out_of_range;    /* fault_oor1 || fault_oor2 */
    bool     implausibility;
} APPS_Output_t;

void APPS_Init(void);
void APPS_Process(uint16_t raw1, uint16_t raw2, APPS_Output_t *out);

#endif /* APPS_H */