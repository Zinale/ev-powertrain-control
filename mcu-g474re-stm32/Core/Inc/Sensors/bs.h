#ifndef BS_H
#define BS_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

#define BRAKE_MEDIAN_WINDOW_SIZE         11U
#define BRAKE_MA_WINDOW_SIZE             16U

/* Calibrazione Aviorace SP170 + Partitore (10k / 5.17k @ 3.3V) */
#define BRAKE_ADC_0_BAR                  409U    /* ~0.33 V */
#define BRAKE_ADC_170_BAR                3681U   /* ~2.97 V */
#define BRAKE_MAX_SCALE_BAR              170.0f
#define BRAKE_MAX_EFFECTIVE_BAR          70.0f   /* Fondo scala utile impianto frenante */

#define BRAKE_ACTIVE_THRESHOLD_BAR       5.0f    /* Soglia accensione Brake Light / R2D */
#define BRAKE_PLAUSIBILITY_THRESH_BAR    20.0f   /* Soglia APPS/Brake Plausibility check */
#define BRAKE_DEADZONE_BAR               1.0f

#define BRAKE_OUT_OF_RANGE_MARGIN_ADC    120U
#define BRAKE_OUT_OF_RANGE_TIMEOUT_MS    100U

typedef struct {
    uint16_t raw_adc;
    uint16_t filtered_adc;
    float    pressure_bar;
    float    percent;                /* Percentuale [0.0 - 100.0 %] calcolata sul range utile */
    bool     is_pressed;
    bool     is_hard_braking;
    bool     fault_out_of_range;
} Brake_Output_t;

void Brake_Init(void);
void Brake_Process(uint16_t raw_adc, Brake_Output_t *out);

#endif /* BS_H */