/**
 * @file ADC_Manager.c
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief Centralized ADC management implementation with DMA
 */

#include "ADC/ADC_Manager.h"
#include "Config.h"
#include <string.h>


/** DMA Buffer */
static volatile uint16_t s_dma_buffer[ADC_NUM_CHANNELS];

static ADC_Buffer_t s_adc_data = {0};

/** Saved ADC handle for reference-static */
static ADC_HandleTypeDef *s_hadc = NULL;

static volatile bool s_new_data_flag = false;


void ADC_Manager_Init(ADC_HandleTypeDef *hadc)
{
    if (hadc == NULL) return;
    
    s_hadc = hadc;
    
    memset((void*)s_dma_buffer, 0, sizeof(s_dma_buffer));
    memset(&s_adc_data, 0, sizeof(s_adc_data));
    
    HAL_ADC_Start_DMA(hadc, (uint32_t*)s_dma_buffer, ADC_NUM_CHANNELS);
    
    s_adc_data.valid = false;
    s_new_data_flag = false;
}


void ADC_Manager_ConversionComplete(ADC_HandleTypeDef *hadc)
{
    if (hadc != s_hadc) return;
    
    /* Copy from DMA buffer to ADC1 buffer */
    for (uint8_t i = 0; i < ADC_NUM_CHANNELS; i++) {
        s_adc_data.raw[i] = s_dma_buffer[i];
    }
    
    s_adc_data.timestamp = HAL_GetTick();
    s_adc_data.valid = true;
    s_new_data_flag = true;
}


bool ADC_Manager_GetBuffer(ADC_Buffer_t *buffer)
{
    if (buffer == NULL) return false;
    
    *buffer = s_adc_data;
    
    return s_adc_data.valid;
}


void ADC_Manager_GetAPPS(uint16_t apps_values[APPS_NUM_SENSORS])
{
    if (apps_values == NULL) return;
        
    #ifdef TEST_MODE_SINGLE_ADC
        /* TEST MODE: Only sensor 1 available */
        apps_values[0] = s_adc_data.raw[ADC_CH_APPS1];
        /* NOTE: Do NOT write apps_values[1] — array size is APPS_NUM_SENSORS (=1) */
    #else
        /* Normal mode: First 2 channels are APPS1 and APPS2 */
        apps_values[0] = s_adc_data.raw[ADC_CH_APPS1];
        apps_values[1] = s_adc_data.raw[ADC_CH_APPS2];
    #endif
}


void ADC_Manager_GetSteering(uint16_t *steering_value)
{
    if (steering_value == NULL) return;
    
    #ifdef TEST_MODE_SINGLE_ADC
        *steering_value = 2048;  /* Center position */
    #else
        *steering_value = s_adc_data.raw[ADC_CH_STEERING];
    #endif
}


