/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ADC1_BS_Pin GPIO_PIN_1
#define ADC1_BS_GPIO_Port GPIOB
#define TIM2_WSS_FR_Pin GPIO_PIN_10
#define TIM2_WSS_FR_GPIO_Port GPIOB
#define ADC1_APPS1_Pin GPIO_PIN_11
#define ADC1_APPS1_GPIO_Port GPIOB
#define ADC1_APPS2_Pin GPIO_PIN_12
#define ADC1_APPS2_GPIO_Port GPIOB
#define ADC1_SAS_Pin GPIO_PIN_14
#define ADC1_SAS_GPIO_Port GPIOB
#define FDCAN_MOTOR_RX_Pin GPIO_PIN_11
#define FDCAN_MOTOR_RX_GPIO_Port GPIOA
#define FDCAN_MOTOR_TX_Pin GPIO_PIN_12
#define FDCAN_MOTOR_TX_GPIO_Port GPIOA
#define T_SWDIO_Pin GPIO_PIN_13
#define T_SWDIO_GPIO_Port GPIOA
#define T_SWCLK_Pin GPIO_PIN_14
#define T_SWCLK_GPIO_Port GPIOA
#define TIM2_WSS_FL_Pin GPIO_PIN_3
#define TIM2_WSS_FL_GPIO_Port GPIOB
#define FDCAN_MAIN_RX_Pin GPIO_PIN_5
#define FDCAN_MAIN_RX_GPIO_Port GPIOB
#define FDCAN_MAIN_TX_Pin GPIO_PIN_6
#define FDCAN_MAIN_TX_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
