#include "Sensors/apps.h"
#include "Sensors/filters.h"
#include "main.h"

static MedianFilter_t        s_med_apps[2];
static MovingAverageFilter_t s_ma_apps[2];

static uint32_t s_mismatch_start_tick = 0;
static uint32_t s_oor1_start_tick = 0;
static uint32_t s_oor2_start_tick = 0;
static bool     s_implausibility_latched = false;

void APPS_Init(void) {
    Filter_Median_Init(&s_med_apps[0], APPS_MEDIAN_WINDOW_SIZE);
    Filter_Median_Init(&s_med_apps[1], APPS_MEDIAN_WINDOW_SIZE);
    Filter_MovingAverage_Init(&s_ma_apps[0], APPS_MA_WINDOW_SIZE);
    Filter_MovingAverage_Init(&s_ma_apps[1], APPS_MA_WINDOW_SIZE);

    s_mismatch_start_tick = 0;
    s_oor1_start_tick = 0;
    s_oor2_start_tick = 0;
    s_implausibility_latched = false;
}

/* Debounce di una condizione di fuori-range: vera solo se persiste oltre
 * APPS_OUT_OF_RANGE_TIMEOUT_MS. Un timer per canale, cosi' APPS1 e APPS2
 * restano distinguibili (bit 4/5 di 0x006). */
static bool oor_debounce(bool oor, uint32_t *start_tick, uint32_t now) {
    if (!oor) {
        *start_tick = 0;
        return false;
    }
    if (*start_tick == 0) {
        *start_tick = now;
        return false;
    }
    return (now - *start_tick) > APPS_OUT_OF_RANGE_TIMEOUT_MS;
}

static uint8_t normalize_apps_pct(uint16_t adc_val, uint16_t adc_min, uint16_t adc_max) {
    if (adc_val <= adc_min) return 0;
    if (adc_val >= adc_max) return 100;

    uint32_t range = adc_max - adc_min;
    uint32_t val   = adc_val - adc_min;
    uint32_t pct   = (val * 100U) / range;

#if (APPS_DEADZONE_PERCENT > 0)
    if (pct <= APPS_DEADZONE_PERCENT) return 0;
    uint32_t rescaled = ((pct - APPS_DEADZONE_PERCENT) * 100U) / (100U - APPS_DEADZONE_PERCENT);
    return (rescaled > 100U) ? 100U : (uint8_t)rescaled;
#else
    return (uint8_t)pct;
#endif
}

void APPS_Process(uint16_t raw1, uint16_t raw2, APPS_Output_t *out) {
    if (!out) return;
    uint32_t now = HAL_GetTick();

    out->raw1 = raw1;
    out->raw2 = raw2;

    /* 1. Filtering chain */
    uint16_t f1 = Filter_MovingAverage_Apply(&s_ma_apps[0], Filter_Median_Apply(&s_med_apps[0], raw1));
    uint16_t f2 = Filter_MovingAverage_Apply(&s_ma_apps[1], Filter_Median_Apply(&s_med_apps[1], raw2));

    out->percent1 = normalize_apps_pct(f1, APPS1_ADC_MIN, APPS1_ADC_MAX);
    out->percent2 = normalize_apps_pct(f2, APPS2_ADC_MIN, APPS2_ADC_MAX);

    /* 2. Verify Out-of-Range (Short/Open) */
    uint16_t low1 = (APPS1_ADC_MIN > APPS_OUT_OF_RANGE_MARGIN_ADC) ? (APPS1_ADC_MIN - APPS_OUT_OF_RANGE_MARGIN_ADC) : 0;
    uint16_t low2 = (APPS2_ADC_MIN > APPS_OUT_OF_RANGE_MARGIN_ADC) ? (APPS2_ADC_MIN - APPS_OUT_OF_RANGE_MARGIN_ADC) : 0;

    bool oor1 = (raw1 < low1) || (raw1 > (APPS1_ADC_MAX + APPS_OUT_OF_RANGE_MARGIN_ADC));
    bool oor2 = (raw2 < low2) || (raw2 > (APPS2_ADC_MAX + APPS_OUT_OF_RANGE_MARGIN_ADC));

    out->fault_oor1 = oor_debounce(oor1, &s_oor1_start_tick, now);
    out->fault_oor2 = oor_debounce(oor2, &s_oor2_start_tick, now);
    out->fault_out_of_range = out->fault_oor1 || out->fault_oor2;

    /* 3. Implausibility > 10% */
    uint8_t diff = (out->percent1 > out->percent2) ? (out->percent1 - out->percent2) : (out->percent2 - out->percent1);
    if (diff > APPS_MAX_DISCREPANCY_PERCENT) {
        if (s_mismatch_start_tick == 0) {
            s_mismatch_start_tick = now;
        } else if ((now - s_mismatch_start_tick) > APPS_IMPLAUSIBILITY_TIMEOUT_MS) {
            out->fault_mismatch = true;
        }
    } else {
        s_mismatch_start_tick = 0;
        out->fault_mismatch = false;
    }

    /* 4. Implausibility and Auto-Recovery on release */
    if (out->fault_mismatch || out->fault_out_of_range) {
        s_implausibility_latched = true;
    }

    if (s_implausibility_latched) {
        if (!out->fault_mismatch && !out->fault_out_of_range &&
            (out->percent1 <= APPS_RESET_THRESHOLD_PERCENT) && 
            (out->percent2 <= APPS_RESET_THRESHOLD_PERCENT)) {
            s_implausibility_latched = false;
        }
    }

    out->implausibility = s_implausibility_latched;

    if (out->implausibility) {
        out->final_percent = 0;
    } else {
        out->final_percent = (uint8_t)(((uint16_t)out->percent1 + (uint16_t)out->percent2) / 2U);
    }
}