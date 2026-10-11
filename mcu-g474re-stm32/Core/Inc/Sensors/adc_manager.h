/*
 * Core/Inc/Sensors/ADC_Manager.h
 * Header file for the ADC Manager module.
 */

#ifndef ADC_MANAGER_H
#define ADC_MANAGER_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* Mappatura Canali DMA */
#define ADC_CH_APPS1                    0U
#define ADC_CH_APPS2                    1U
#define ADC_CH_STEERING                 2U
#define ADC_CH_BRAKE                    3U
#define ADC_TOTAL_CHANNELS              4U

/** ADC resolution */
#define ADC_RESOLUTION_BITS         12U
#define ADC_MAX_VALUE               ((1U << ADC_RESOLUTION_BITS) - 1U)  

typedef struct {
    uint16_t raw[ADC_TOTAL_CHANNELS];
    uint32_t timestamp;
    bool     valid;
} ADC_Buffer_t;




/**
 * @brief Initialize the ADC Manager module and start DMA
 * @param hadc Pointer to ADC handle (ex. &hadc1)
 */
void ADC_Manager_Init(ADC_HandleTypeDef *hadc);

/**
 * @brief Callback to call in HAL_ADC_ConvCpltCallback
 * @param hadc Pointer to ADC handle
 */
void ADC_Manager_ConversionComplete(ADC_HandleTypeDef *hadc);

/**
 * @brief Get copy of current ADC buffer
 * @param[out] buffer Pointer to structure to fill
 * @return true if data is valid, false otherwise
 */
bool ADC_Manager_GetBuffer(ADC_Buffer_t *buffer);


#endif /* ADC_MANAGER_H */