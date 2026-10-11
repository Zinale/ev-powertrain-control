#include "Sensors/adc_manager.h"
#include <string.h>

static volatile uint16_t s_dma_buffer[ADC_TOTAL_CHANNELS];
static ADC_HandleTypeDef *s_hadc = NULL;
static volatile ADC_Buffer_t s_adc_data = {0};
static volatile bool s_new_data_flag = false;



void ADC_Manager_Init(ADC_HandleTypeDef *hadc)
{
    if (hadc == NULL) return;
    
    s_hadc = hadc;
    
    memset((void*)s_dma_buffer, 0, sizeof(s_dma_buffer));
    memset((void *)&s_adc_data, 0, sizeof(s_adc_data));
    HAL_ADC_Start_DMA(hadc, (uint32_t*)s_dma_buffer, ADC_TOTAL_CHANNELS);
    
    s_adc_data.valid = false;
    s_new_data_flag = false;
}


void ADC_Manager_ConversionComplete(ADC_HandleTypeDef *hadc) {
    if (hadc == NULL || s_hadc == NULL) return;

    /* Update ADC buffer with new DMA data */
    for (uint8_t i = 0; i < ADC_TOTAL_CHANNELS; i++) {
        s_adc_data.raw[i] = s_dma_buffer[i];
    }
    s_adc_data.timestamp = HAL_GetTick();
    s_adc_data.valid = true;
    s_new_data_flag = true;
}


bool ADC_Manager_GetBuffer(ADC_Buffer_t *buffer) {
    if (buffer == NULL || s_hadc == NULL) return false;

    for (uint8_t i = 0; i < ADC_TOTAL_CHANNELS; i++) {
        buffer->raw[i] = s_dma_buffer[i];
    }
    buffer->timestamp = HAL_GetTick();
    buffer->valid = true;
    return true;
}
