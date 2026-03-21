#include "APP.h"
#include "Config.h"
#include <string.h> // per memset


static uint16_t s_samples[APP_FILTER_WINDOW]; // buffer ultimi N campioni
static uint8_t  s_index = 0;                  // indice circolare nel buffer
static uint8_t  s_count = 0;                  // quanti campioni validi abbiamo
static uint16_t s_filtered = 0;               // ultimo valore filtrato

/* ***********************************************************************
 * Funzioni interne
 * ***********************************************************************/

//Guardia per max e min
static uint16_t APP_guardrail(uint16_t val)
{
    if (val < APP_ADC_MIN)  return APP_ADC_MIN;
    if (val > APP_ADC_MAX)  return APP_ADC_MAX;
    return val;
}


void APP_Init(void)
{
    memset(s_samples, 0, sizeof(s_samples));
    s_index    = 0;
    s_count    = 0;
    s_filtered = 0;
}

void APP_Update(uint16_t raw_adc_val)
{
    uint16_t clamped = APP_guardrail(raw_adc_val);

    s_samples[s_index] = clamped;
    s_index = (uint8_t)((s_index + 1U) % APP_FILTER_WINDOW);

    if (s_count < APP_FILTER_WINDOW) {
        s_count++;
    }

    uint32_t sum = 0;
    for (uint8_t i = 0; i < s_count; i++) {
        sum += s_samples[i];
    }

    if (s_count > 0) {
        s_filtered = (uint16_t)(sum / s_count);
    } else {
        s_filtered = 0;
    }
}

uint16_t APP_GetFiltered(void)
{
    return s_filtered;
}

uint8_t APP_GetPercent(void)
{
    uint32_t deadzone_value = (APP_ADC_MAX * APP_DEADZONE_PERCENT) / 100U;
    
    if (s_filtered <= deadzone_value) {
        return 0;
    }
    
    uint32_t range = APP_ADC_MAX - deadzone_value;
    uint32_t value = s_filtered - deadzone_value;
    uint32_t pct = (value * 100U) / range;
    
    if (pct > 100U) pct = 100U;
    
    return (uint8_t)pct;
}
