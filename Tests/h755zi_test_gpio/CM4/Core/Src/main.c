/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* DUAL_CORE_BOOT_SYNC_SEQUENCE: Define for dual core boot synchronization    */
/*                             demonstration code based on hardware semaphore */
/* This define is present in both CM7/CM4 projects                            */
/* To comment when developping/debugging on a single core                     */
#define DUAL_CORE_BOOT_SYNC_SEQUENCE

#if defined(DUAL_CORE_BOOT_SYNC_SEQUENCE)
#ifndef HSEM_ID_0
#define HSEM_ID_0 (0U) /* HW semaphore 0*/
#endif
#endif /* DUAL_CORE_BOOT_SYNC_SEQUENCE */

/* -----------------------------------------------------------------------
 *  Generatore segnale ruota fonica – parametri configurabili
 *
 *  FONICA_FREQ_HZ : frequenza del segnale quadro su FONICA_OUT (PD7).
 *                   Rappresenta la frequenza di un singolo dente della
 *                   ruota fonica, es. 100 Hz = 100 denti/s.
 *                   Range consigliato: 10 – 2000 Hz.
 *
 *  BURST_DURATION_MS : durata del burst dopo pressione del button [ms].
 * --------------------------------------------------------------------- */
#define FONICA_FREQ_HZ       100u          /* frequenza segnale [Hz]        */
#define BURST_DURATION_MS    20000u        /* durata burst [ms]  = 20 s     */

/* Semiperiodo in ms: si usa HAL_Delay, quindi il minimo è 1 ms            */
/* Per frequenze > 500 Hz usare un TIM hardware al posto di HAL_Delay.     */
#define FONICA_HALF_PERIOD_MS  (1000u / (FONICA_FREQ_HZ * 2u))

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* Flag settato dall'EXTI del button (PC13) – volatile perché scritto in IRQ */
volatile uint8_t g_btn_pressed = 0u;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* -----------------------------------------------------------------------
 *  EXTI callback – button user (PC13, rising edge)
 *  Schedula il burst nel main loop tramite flag.
 * --------------------------------------------------------------------- */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_13)
    {
        g_btn_pressed = 1u;
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

/* USER CODE BEGIN Boot_Mode_Sequence_1 */
#if defined(DUAL_CORE_BOOT_SYNC_SEQUENCE)
  /*HW semaphore Clock enable*/
  __HAL_RCC_HSEM_CLK_ENABLE();
  /* Activate HSEM notification for Cortex-M4*/
  HAL_HSEM_ActivateNotification(__HAL_HSEM_SEMID_TO_MASK(HSEM_ID_0));
  /*
  Domain D2 goes to STOP mode (Cortex-M4 in deep-sleep) waiting for Cortex-M7 to
  perform system initialization (system clock config, external memory configuration.. )
  */
  HAL_PWREx_ClearPendingEvent();
  HAL_PWREx_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFE, PWR_D2_DOMAIN);
  /* Clear HSEM flag */
  __HAL_HSEM_CLEAR_FLAG(__HAL_HSEM_SEMID_TO_MASK(HSEM_ID_0));

#endif /* DUAL_CORE_BOOT_SYNC_SEQUENCE */
/* USER CODE END Boot_Mode_Sequence_1 */
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* Aggiorna SystemCoreClock con la configurazione reale impostata dal CM7.
   * Senza questa chiamata SystemCoreClock rimane al valore di default HSI
   * (64 MHz) e HAL_Delay produce ritardi scorretti. */
  SystemCoreClockUpdate();
  /* Riconfigura SysTick con il clock corretto */
  HAL_InitTick(TICK_INT_PRIORITY);
  /* USER CODE END Init */

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */

  /* Abilita NVIC per EXTI15_10 (PC13 = user button) */
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* Tutti i LED spenti all'avvio */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_14, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_1, GPIO_PIN_RESET);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    /* ------------------------------------------------------------------
     *  Attesa pressione button
     * ---------------------------------------------------------------- */
    if (!g_btn_pressed)
    {
        /* Idle: LED LD1 (PB0) lampeggia lento (1 Hz) per segnalare stand-by */
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_1,  GPIO_PIN_RESET);
        HAL_Delay(100);
        continue;
    }

    /* ------------------------------------------------------------------
     *  Button premuto → avvio burst di BURST_DURATION_MS ms
     * ---------------------------------------------------------------- */
    g_btn_pressed = 0u;

    /* LD1 spento, LD2 e LD3 accesi durante il burst */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,  GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_1,  GPIO_PIN_SET);

    uint32_t burst_start = HAL_GetTick();

    while ((HAL_GetTick() - burst_start) < BURST_DURATION_MS)
    {
        /* Genera semiperiodo HIGH */
        HAL_GPIO_WritePin(FONICA_OUT_GPIO_Port, FONICA_OUT_Pin, GPIO_PIN_SET);
        HAL_Delay(FONICA_HALF_PERIOD_MS);

        /* Genera semiperiodo LOW */
        HAL_GPIO_WritePin(FONICA_OUT_GPIO_Port, FONICA_OUT_Pin, GPIO_PIN_RESET);
        HAL_Delay(FONICA_HALF_PERIOD_MS);

        /* Sequenza LED rotante durante il burst (effetto chase a ~FONICA_FREQ_HZ/2) */
        /* Usa i ms trascorsi per alternare: ogni 100 ms cambia configurazione LED  */
        uint32_t elapsed = HAL_GetTick() - burst_start;
        uint8_t  phase   = (uint8_t)((elapsed / 100u) % 3u);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,  (phase == 0u) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_1,  (phase == 1u) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, (phase == 2u) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }

    /* Fine burst: assicurarsi che FONICA_OUT torni LOW */
    HAL_GPIO_WritePin(FONICA_OUT_GPIO_Port, FONICA_OUT_Pin, GPIO_PIN_RESET);

    /* Tutti i LED accesi per 500 ms → conferma fine burst */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,  GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_1,  GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
    HAL_Delay(500);

    /* Spegni tutti i LED → torna in stand-by */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,  GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_1,  GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);

  }
  /* USER CODE END 3 */
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_14, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(FONICA_OUT_GPIO_Port, FONICA_OUT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_1, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB14 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : FONICA_OUT_Pin */
  GPIO_InitStruct.Pin = FONICA_OUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(FONICA_OUT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PE1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
