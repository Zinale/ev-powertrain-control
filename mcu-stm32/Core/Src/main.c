/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Config.h"
#include "Safety/watchdog.h"
#include "FreeRTOS.h"
#include "Can.h"
#include "Serial.h"
#include "Inverter.h"
#include "ADC_Manager.h"
#include "Tasks/data_logger.h"
#include "Tasks/motors_manager.h"
#include "Tasks/data_manager.h"
#include "Safety/device_state.h"
#include "task.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

CAN_HandleTypeDef hcan1;
CAN_HandleTypeDef hcan2;

IWDG_HandleTypeDef hiwdg;

UART_HandleTypeDef huart5;
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_uart5_tx;
DMA_HandleTypeDef hdma_usart3_tx;

/* Definitions for DataLogger */
osThreadId_t DataLoggerHandle;
const osThreadAttr_t DataLogger_attributes = {
  .name = "DataLogger",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for MotorsManager */
osThreadId_t MotorsManagerHandle;
const osThreadAttr_t MotorsManager_attributes = {
  .name = "MotorsManager",
  .stack_size = 2048 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for DataManager */
osThreadId_t DataManagerHandle;
const osThreadAttr_t DataManager_attributes = {
  .name = "DataManager",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for mutex_APPS */
osMutexId_t mutex_APPSHandle;
const osMutexAttr_t mutex_APPS_attributes = {
  .name = "mutex_APPS"
};
/* Definitions for mutex_SAS */
osMutexId_t mutex_SASHandle;
const osMutexAttr_t mutex_SAS_attributes = {
  .name = "mutex_SAS"
};
/* Definitions for mutex_INVERTER_L */
osMutexId_t mutex_INVERTER_LHandle;
const osMutexAttr_t mutex_INVERTER_L_attributes = {
  .name = "mutex_INVERTER_L"
};
/* Definitions for mutex_INVERTER_R */
osMutexId_t mutex_INVERTER_RHandle;
const osMutexAttr_t mutex_INVERTER_R_attributes = {
  .name = "mutex_INVERTER_R"
};
/* Definitions for mutex_UART5 */
osMutexId_t mutex_UART5Handle;
const osMutexAttr_t mutex_UART5_attributes = {
  .name = "mutex_UART5"
};
/* Definitions for mutex_CAN1 */
osMutexId_t mutex_CAN1Handle;
const osMutexAttr_t mutex_CAN1_attributes = {
  .name = "mutex_CAN1"
};
/* Definitions for mutex_UART3 */
osMutexId_t mutex_UART3Handle;
const osMutexAttr_t mutex_UART3_attributes = {
  .name = "mutex_UART3"
};
/* Definitions for mutex_CAN2 */
osMutexId_t mutex_CAN2Handle;
const osMutexAttr_t mutex_CAN2_attributes = {
  .name = "mutex_CAN2"
};
/* USER CODE BEGIN PV */


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_CAN1_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_UART5_Init(void);
static void MX_CAN2_Init(void);
static void MX_IWDG_Init(void);
void StartDataLogger(void *argument);
void StartMotorsManager(void *argument);
void StartDataManager(void *argument);

/* USER CODE BEGIN PFP */
void MX_FREERTOS_Init(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_CAN1_Init();
  MX_ADC1_Init();
  MX_USART3_UART_Init();
  MX_UART5_Init();
  MX_CAN2_Init();
  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */

  DeviceState_Set(DEVICE_STATE_INIT);
  
  /* Freeze Independent Watchdog (IWDG) while debugging:
   * When the debugger halts the CPU, the IWDG would normally continue
   * counting and eventually reset the MCU. Calling this macro freezes
   * the IWDG counter while the core is halted by the debugger. It has
   * no effect when no debugger is connected.
   */
  __HAL_DBGMCU_FREEZE_IWDG();

  /* Ensure the IWDG counter is refreshed immediately after initialization
   * to give the rest of the system time to complete startup without
   * triggering an unwanted reset. This is a one-time refresh; periodic
   * refreshes are handled by the safety monitor task.
   */
  Watchdog_Refresh();

  Inverters_Init();
  DataReadings_Init();
  Watchdog_Refresh();
  //SchedulerInitFct();
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();
  /* Create the mutex(es) */
  /* creation of mutex_APPS */
  mutex_APPSHandle = osMutexNew(&mutex_APPS_attributes);

  /* creation of mutex_SAS */
  mutex_SASHandle = osMutexNew(&mutex_SAS_attributes);

  /* creation of mutex_INVERTER_L */
  mutex_INVERTER_LHandle = osMutexNew(&mutex_INVERTER_L_attributes);

  /* creation of mutex_INVERTER_R */
  mutex_INVERTER_RHandle = osMutexNew(&mutex_INVERTER_R_attributes);

  /* creation of mutex_UART5 */
  mutex_UART5Handle = osMutexNew(&mutex_UART5_attributes);

  /* creation of mutex_CAN1 */
  mutex_CAN1Handle = osMutexNew(&mutex_CAN1_attributes);

  /* creation of mutex_UART3 */
  mutex_UART3Handle = osMutexNew(&mutex_UART3_attributes);

  /* creation of mutex_CAN2 */
  mutex_CAN2Handle = osMutexNew(&mutex_CAN2_attributes);

  if ((mutex_APPSHandle == NULL) ||
      (mutex_SASHandle == NULL) ||
      (mutex_INVERTER_LHandle == NULL) ||
      (mutex_INVERTER_RHandle == NULL) ||
      (mutex_UART5Handle == NULL) ||
      (mutex_CAN1Handle == NULL) ||
      (mutex_UART3Handle == NULL) ||
      (mutex_CAN2Handle == NULL))
  {
    Error_Handler();
  }

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* CAN RX queues must be created after osKernelInitialize() and before CAN IRQ notifications. */
  CAN_ProcessQueue_Init();      /* CAN1 RX queue */
  CAN_Car_ProcessQueue_Init();  /* CAN2 RX queue */

  if (!CAN_QueuesReady())
  {
    Error_Handler();
  }

  /* Defer CAN peripheral start to task context (after osKernelStart). */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of DataLogger */
  DataLoggerHandle = osThreadNew(StartDataLogger, NULL, &DataLogger_attributes);

  /* creation of MotorsManager */
  MotorsManagerHandle = osThreadNew(StartMotorsManager, NULL, &MotorsManager_attributes);

  /* creation of DataManager */
  DataManagerHandle = osThreadNew(StartDataManager, NULL, &DataManager_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  //SchedulerMgmFct();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 208;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */
  /* Override ADC configuration based on Config.h build mode */
  /* This section is protected from CubeMX regeneration */
  
  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV8;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 3;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_13;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */
  /* Override ADC configuration for different build modes */
  /* This part of code is protected from CubeMX regeneration */
  
  #if defined(TEST_MODE_SINGLE_ADC)
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.NbrOfConversion = 1;
    if (HAL_ADC_Init(&hadc1) != HAL_OK) {
      Error_Handler();
    }
    
    /* Reconfigure only CH0 */
    ADC_ChannelConfTypeDef sConfig_override = {0};
    sConfig_override.Channel = ADC_CHANNEL_0;
    sConfig_override.Rank = ADC_REGULAR_RANK_1;
    sConfig_override.SamplingTime = ADC_SAMPLETIME_480CYCLES;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig_override) != HAL_OK) {
      Error_Handler();
    }
  #endif
  /* In TEST_MODE_FULL and PRODUCTION_MODE, use CubeMX configuration (3 channels) */
  
  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */
  /* Override CAN1 configuration based on Config.h */
  /* This section is protected from CubeMX regeneration */
  
  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 26;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_4TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_2TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_1TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = ENABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = ENABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = ENABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  /* USER CODE END CAN1_Init 2 */

}

/**
  * @brief CAN2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN2_Init(void)
{

  /* USER CODE BEGIN CAN2_Init 0 */

  /* USER CODE END CAN2_Init 0 */

  /* USER CODE BEGIN CAN2_Init 1 */
  /* Override CAN2 configuration based on Config.h */
  /* This section is protected from CubeMX regeneration */
  
  /* USER CODE END CAN2_Init 1 */
  hcan2.Instance = CAN2;
  hcan2.Init.Prescaler = 26;
  hcan2.Init.Mode = CAN_MODE_NORMAL;
  hcan2.Init.SyncJumpWidth = CAN_SJW_2TQ;
  hcan2.Init.TimeSeg1 = CAN_BS1_2TQ;
  hcan2.Init.TimeSeg2 = CAN_BS2_1TQ;
  hcan2.Init.TimeTriggeredMode = DISABLE;
  hcan2.Init.AutoBusOff = ENABLE;
  hcan2.Init.AutoWakeUp = DISABLE;
  hcan2.Init.AutoRetransmission = DISABLE;
  hcan2.Init.ReceiveFifoLocked = DISABLE;
  hcan2.Init.TransmitFifoPriority = ENABLE;
  if (HAL_CAN_Init(&hcan2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN2_Init 2 */
  /* CAN2 configuration from CubeMX is correct (AutoRetransmission = DISABLE) */
  /* This matches CAN2_AUTO_RETRANSMISSION in Config.h */
  /* No override needed - CubeMX config already correct */
  
  /* USER CODE END CAN2_Init 2 */

}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
  hiwdg.Init.Window = 4095;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief UART5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART5_Init(void)
{

  /* USER CODE BEGIN UART5_Init 0 */

  /* USER CODE END UART5_Init 0 */

  /* USER CODE BEGIN UART5_Init 1 */

  /* USER CODE END UART5_Init 1 */
  huart5.Instance = UART5;
  huart5.Init.BaudRate = 115200;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  huart5.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart5.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART5_Init 2 */

  /* USER CODE END UART5_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);
  /* DMA1_Stream7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream7_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream7_IRQn);
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_btn_Pin */
  GPIO_InitStruct.Pin = USER_btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_btn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/**
 * @brief CAN Interrupt - Message reception
 * @param hcan CAN handle that generated interrupt
 * @return void
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan){
    // CAN1 only - Motor inverters
    if (hcan->Instance == CAN1) {
        CAN_Inverter_ProcessRxMessages(hcan);
    }
}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan){
    // CAN2 only - Car data (IMU, BMS, ECU, DASH)
    if (hcan->Instance == CAN2) {
        CAN_Car_ProcessRxMessages(hcan);
    }
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc){

}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc){
    ADC_Manager_ConversionComplete(hadc);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart5)
    {
        extern volatile uint8_t uart5_tx_busy;
        uart5_tx_busy = 0;
    }
    if (huart == &huart3)
    {
        extern volatile uint8_t uart3_tx_busy;
        uart3_tx_busy = 0;
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart5)
    {   
        extern volatile uint8_t uart5_tx_busy;
        uart5_tx_busy = 0;
    }
    if (huart == &huart3)
    {
        extern volatile uint8_t uart3_tx_busy;
        uart3_tx_busy = 0;
    }
}


/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDataLogger */
/**
* @brief Function implementing the DataLogger thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDataLogger */
void StartDataLogger(void *argument)
{
  /* USER CODE BEGIN 5 */
  DataLoggerTask();
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartMotorsManager */
/**
* @brief Function implementing the MotorsManager thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartMotorsManager */
void StartMotorsManager(void *argument)
{
  /* USER CODE BEGIN StartMotorsManager */
	MotorsManagerTask();
  /* USER CODE END StartMotorsManager */
}

/* USER CODE BEGIN Header_StartDataManager */
/**
* @brief Function implementing the DataManager thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDataManager */
void StartDataManager(void *argument)
{
  /* USER CODE BEGIN StartDataManager */
  static uint8_t s_can_started = 0U;

  if (s_can_started == 0U)
  {
    CAN_Inverter_Init(&hcan1);
    CAN_Car_Init(&hcan2);
    s_can_started = 1U;
  }

  DataManagerTask();
  /* USER CODE END StartDataManager */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM7 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM7)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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
