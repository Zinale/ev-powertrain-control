#include "Sensors/sas.h"
#include "Sensors/filters.h"
#include "main.h"

static MedianFilter_t        s_med_sas;
static MovingAverageFilter_t s_ma_sas;
static uint32_t              s_oor_start_tick = 0;

void SAS_Init(void) {
    Filter_Median_Init(&s_med_sas, SAS_MEDIAN_WINDOW_SIZE);
    Filter_MovingAverage_Init(&s_ma_sas, SAS_MA_WINDOW_SIZE);
    s_oor_start_tick = 0;
}

static int16_t convert_adc_to_angle(uint16_t filtered) {
    int32_t angle = 0;

    if (filtered < SAS_ADC_CENTER) {
        /* Sterzata a Sinistra (Gradi Negativi) */
        if (filtered <= SAS_ADC_MIN) return SAS_ANGLE_MAX_LEFT_DEG;
        
        int32_t adc_range   = (int32_t)SAS_ADC_CENTER - (int32_t)SAS_ADC_MIN;
        int32_t angle_range = 0 - (int32_t)SAS_ANGLE_MAX_LEFT_DEG;
        int32_t adc_offset  = (int32_t)filtered - (int32_t)SAS_ADC_MIN;

        angle = SAS_ANGLE_MAX_LEFT_DEG + ((adc_offset * angle_range) / adc_range);
    } else {
        /* Sterzata a Destra (Gradi Positivi) */
        if (filtered >= SAS_ADC_MAX) return SAS_ANGLE_MAX_RIGHT_DEG;

        int32_t adc_range   = (int32_t)SAS_ADC_MAX - (int32_t)SAS_ADC_CENTER;
        int32_t angle_range = (int32_t)SAS_ANGLE_MAX_RIGHT_DEG;
        int32_t adc_offset  = (int32_t)filtered - (int32_t)SAS_ADC_CENTER;

        angle = 0 + ((adc_offset * angle_range) / adc_range);
    }

    /* Applicazione Deadzone attorno allo zero */
    if (angle > -(int16_t)SAS_DEADZONE_DEG && angle < (int16_t)SAS_DEADZONE_DEG) {
        angle = 0;
    }

    return (int16_t)angle;
}

void SAS_Process(uint16_t raw_adc, SAS_Output_t *out) {
    if (!out) return;
    uint32_t now = HAL_GetTick();

    out->raw_adc = raw_adc;

    /* 1. Filtraggio */
    uint16_t med_val = Filter_Median_Apply(&s_med_sas, raw_adc);
    out->filtered_adc = Filter_MovingAverage_Apply(&s_ma_sas, med_val);

    /* 2. Out-of-Range */
    uint16_t low_limit = (SAS_ADC_MIN > SAS_OUT_OF_RANGE_MARGIN_ADC) ? (SAS_ADC_MIN - SAS_OUT_OF_RANGE_MARGIN_ADC) : 0;
    bool out_of_range = (raw_adc < low_limit) || (raw_adc > (SAS_ADC_MAX + SAS_OUT_OF_RANGE_MARGIN_ADC));

    if (out_of_range) {
        if (s_oor_start_tick == 0) {
            s_oor_start_tick = now;
        } else if ((now - s_oor_start_tick) > SAS_OUT_OF_RANGE_TIMEOUT_MS) {
            out->fault_out_of_range = true;
        }
    } else {
        s_oor_start_tick = 0;
        out->fault_out_of_range = false;
    }

    /* 3. Conversione in Gradi */
    out->angle_deg = convert_adc_to_angle(out->filtered_adc);
}