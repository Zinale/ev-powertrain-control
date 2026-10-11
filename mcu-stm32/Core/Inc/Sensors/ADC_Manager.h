/**
 * @file ADC_Manager.h
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief Centralized ADC management with DMA
 * 
 * This module handles DMA reading of ADC1 and distributes
 * raw data to other modules (APPS, Steering, etc.)
 * 
 * Flow:
 *   1. ADC1 in DMA Circular mode reads N channels
 *   2. DMA Callback copies data to internal buffer
 *   3. Other modules access via thread-safe getter
 */

#ifndef ADC_MANAGER_H
#define ADC_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "Sensors/APPS.h"
#include <stdbool.h>
#include <stdint.h>

/* ===========================================================================
 * ADC Configuration
 * =========================================================================== */

/** ADC channel indices */
#define ADC_CH_APPS1                0U      /**< APPS sensor 1 */
#define ADC_CH_APPS2                1U      /**< APPS sensor 2 */
#define ADC_CH_STEERING             2U      /**< Steering potentiometer */

/** ADC resolution */
#define ADC_RESOLUTION_BITS         12U
#define ADC_MAX_VALUE               ((1U << ADC_RESOLUTION_BITS) - 1U)  



/**
 * @brief ADC buffer structure
 */
typedef struct {
    uint16_t raw[ADC_NUM_CHANNELS];     /**< Raw values per channel */
    uint32_t timestamp;                  /**< Tick at the moment of reading */
    bool     valid;                      /**< Valid data flag */
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



#ifdef __cplusplus
}
#endif

#endif /* ADC_MANAGER_H */
