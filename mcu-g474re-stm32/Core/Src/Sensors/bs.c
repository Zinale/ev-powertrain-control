#include "Sensors/bs.h"
#include "Sensors/filters.h"

static MedianFilter_t        s_med_brake;
static MovingAverageFilter_t s_ma_brake;
static uint32_t              s_oor_start_tick = 0;

void Brake_Init(void) {
    Filter_Median_Init(&s_med_brake, BRAKE_MEDIAN_WINDOW_SIZE);
    Filter_MovingAverage_Init(&s_ma_brake, BRAKE_MA_WINDOW_SIZE);
    s_oor_start_tick = 0;
}

void Brake_Process(uint16_t raw_adc, Brake_Output_t *out) {
    if (!out) return;
    
    uint32_t now = HAL_GetTick();
    out->raw_adc = raw_adc;

    uint16_t med_val = Filter_Median_Apply(&s_med_brake, raw_adc);
    out->filtered_adc = Filter_MovingAverage_Apply(&s_ma_brake, med_val);

    /* 1. Out-of-Range */
    bool out_of_range = (raw_adc < (BRAKE_ADC_0_BAR - BRAKE_OUT_OF_RANGE_MARGIN_ADC)) ||
                        (raw_adc > (BRAKE_ADC_170_BAR + BRAKE_OUT_OF_RANGE_MARGIN_ADC));

    if (out_of_range) {
        if (s_oor_start_tick == 0) {
            s_oor_start_tick = now;
        } else if ((now - s_oor_start_tick) > BRAKE_OUT_OF_RANGE_TIMEOUT_MS) {
            out->fault_out_of_range = true;
        }
    } else {
        s_oor_start_tick = 0;
        out->fault_out_of_range = false;
    }

    /* 2. Calcolo Pressione [bar] */
    if (out->filtered_adc <= BRAKE_ADC_0_BAR) {
        out->pressure_bar = 0.0f;
    } else if (out->filtered_adc >= BRAKE_ADC_170_BAR) {
        out->pressure_bar = BRAKE_MAX_SCALE_BAR;
    } else {
        out->pressure_bar = ((float)(out->filtered_adc - BRAKE_ADC_0_BAR) / 
                            (float)(BRAKE_ADC_170_BAR - BRAKE_ADC_0_BAR)) * BRAKE_MAX_SCALE_BAR;
    }

    if (out->pressure_bar < BRAKE_DEADZONE_BAR) {
        out->pressure_bar = 0.0f;
    }

    /* 3. Calcolo Percentuale Normalizzata [0 - 100%] su fondo scala utile (70 bar) */
    out->percent = (out->pressure_bar / BRAKE_MAX_EFFECTIVE_BAR) * 100.0f;
    if (out->percent > 100.0f) {
        out->percent = 100.0f;
    }

    /* 4. Flag */
    out->is_pressed      = (out->pressure_bar >= BRAKE_ACTIVE_THRESHOLD_BAR);
    out->is_hard_braking = (out->pressure_bar >= BRAKE_PLAUSIBILITY_THRESH_BAR);
}