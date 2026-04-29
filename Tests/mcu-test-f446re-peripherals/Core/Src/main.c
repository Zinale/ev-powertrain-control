/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body – STM32F446RE peripheral test
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
  * Adattato da mcu-g474re-stm32 – differenze principali G474RE → F446RE:
  *   · FDCAN   → bxCAN  (HAL_CAN_*)
  *   · LPUART1 → USART2 (PA2=TX / PA3=RX)
  *   · ADC ch  → IN6,IN7,IN9,IN14  (PA6,PA7,PB1,PC4)
  *   · TIM2    → CH1(PA0) e CH3(PB10); clock TIM2=90 MHz, psc=179 → 2 µs/tick
  *   · IWDG    → no finestra (F4 IWDG non ha windowed mode)
 *   · CH1 DMA → DMA1_Stream5/Ch3 generato da CubeMX in DMA_CIRCULAR
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
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

TIM_HandleTypeDef htim2;
DMA_HandleTypeDef hdma_tim2_up_ch3;
DMA_HandleTypeDef hdma_tim2_ch1;

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_tx;

/* USER CODE BEGIN PV */

/* -----------------------------------------------------------------------
 *  IWDG – non generato da CubeMX per questa board; aggiunto manualmente
 * --------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
 *  ADC – 4 canali DMA continuo (circolare)
 *  Buffer: [0]=PA6/IN6  [1]=PA7/IN7  [2]=PB1/IN9  [3]=PC4/IN14
 * --------------------------------------------------------------------- */
#define ADC_NUM_CH   4u
volatile uint16_t g_adc[ADC_NUM_CH];          /* raw 12-bit ADC values  */

/* -----------------------------------------------------------------------
 *  USART2 – debug su ST-Link VCP via DMA (PA2=TX, PA3=RX @ 115200)
 *  Pattern identico a Serial.c: buffer DMA + flag busy + TxCplt callback
 * --------------------------------------------------------------------- */
#define UART_BUF_SIZE   128u
static char      g_uart_buf[UART_BUF_SIZE];
volatile uint8_t g_uart_busy = 0;

/* -----------------------------------------------------------------------
 *  bxCAN  (CAN1 = PA11/PA12 @ 500 kbps,  CAN2 = PB5/PB6 @ 500 kbps)
 *
 *  NOTA: la F446RE ha AutoBusOff=ENABLE → il recovery bus-off è automatico
 *  (hardware attende 128×11 bit recessivi). Con AutoRetransmission=DISABLE
 *  il TX viene tentato una sola volta e non si innesca il bus-off loop.
 *  Per test senza hardware CAN → cambiare Mode in CAN_MODE_LOOPBACK nel
 *  .ioc e rigenerare.
 * --------------------------------------------------------------------- */
volatile uint8_t      g_btn_tx_req  = 0;      /* richiesta TX da pulsante */
CAN_RxHeaderTypeDef   g_can_rx_hdr;
uint8_t               g_can_rx_data[8];
volatile uint32_t     g_can1_rx_cnt = 0;
volatile uint32_t     g_can2_rx_cnt = 0;

/* -----------------------------------------------------------------------
 *  Speed sensors – TIM2 Input Capture DMA circolare
 *  PA0  → TIM2_CH1 → ruota 1
 *  PB10 → TIM2_CH3 → ruota 2
 *
 *  Clock TIM2: APB1=45 MHz → timer clock = 2×45 = 90 MHz
 *  Prescaler 179 → tick = 90 MHz / 180 = 500 kHz → 2 µs/tick
 *
 *  rpm  = 60 × 500 000 / (diff_tick × NUM_DENTI)
 *       = 30 000 000   / (diff_tick × NUM_DENTI)
 *  km/h = rpm_medio × CIRC_RUOTA_M × 60 / 1000
 *
 *  TIM2_CH3 → DMA1_Stream1, Ch3 (generato da CubeMX, handle hdma_tim2_up_ch3)
 *  TIM2_CH1 → DMA1_Stream5, Ch3 (generato da CubeMX, handle hdma_tim2_ch1;
 *                                  mode CIRCULAR fissato in USER CODE TIM2_MspInit 1)
 * --------------------------------------------------------------------- */
#define NUM_DENTI      22.0f
#define CIRC_RUOTA_M    1.8f              /* circonferenza ruota [m]       */
#define IC_BUF_SIZE    16u

/* hdma_tim2_ch1 → dichiarato da CubeMX nella sezione non-USER CODE       */

uint32_t          g_ic_buf_ch1[IC_BUF_SIZE];  /* DMA scrive TIM2 CH1 qui */
uint32_t          g_ic_buf_ch3[IC_BUF_SIZE];  /* DMA scrive TIM2 CH3 qui */
uint16_t          g_last_idx_ch1 = 0;
uint16_t          g_last_idx_ch3 = 0;
uint16_t          g_timeout_ch1  = 0;         /* cnt loop senza nuovi dati */
uint16_t          g_timeout_ch3  = 0;

volatile float    g_rpm1      = 0.0f;
volatile float    g_rpm2      = 0.0f;
volatile float    g_speed_kmh = 0.0f;

/* logging tick: globale per visibilità in debug */
uint32_t          g_log_tick  = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_CAN1_Init(void);
static void MX_CAN2_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_IWDG_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* -----------------------------------------------------------------------
 *  Serial_Log – invia stringa formattata su USART2 via DMA
 *  Stesso pattern di Serial.c: buffer statico + flag busy + callback.
 *  Il chiamante attende al più 20 ms che il TX precedente finisca;
 *  se scade il timeout il messaggio viene scartato (no blocco).
 * --------------------------------------------------------------------- */
void Serial_Log(const char *fmt, ...)
{
    uint32_t t0 = HAL_GetTick();
    while (g_uart_busy && (HAL_GetTick() - t0 < 20u)) {}
    if (g_uart_busy) return;

    va_list ap;
    va_start(ap, fmt);
    int len = vsnprintf(g_uart_buf, sizeof(g_uart_buf), fmt, ap);
    va_end(ap);

    if (len <= 0 || len >= (int)sizeof(g_uart_buf)) return;

    g_uart_busy = 1u;
    if (HAL_UART_Transmit_DMA(&huart2, (uint8_t *)g_uart_buf, (uint16_t)len) != HAL_OK)
        g_uart_busy = 0u;
}

/* -----------------------------------------------------------------------
 *  HAL Callbacks
 * --------------------------------------------------------------------- */

/* TX USART2 completato → libera il flag (chiamato da DMA1_Stream6 IRQ) */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
        g_uart_busy = 0u;
}

/* Pulsante B1 (PC13, falling edge EXTI) → schedula TX su CAN1 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == B1_Pin)
        g_btn_tx_req = 1u;
}

/*
 * bxCAN RX FIFO0 callback – svuota il FIFO in loop (gestisce burst)
 *  • CAN1: toggle LED verde (PA5 / LD2) ad ogni frame ricevuto
 *  • CAN2: incrementa solo il contatore
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    while (HAL_CAN_GetRxFifoFillLevel(hcan, CAN_RX_FIFO0) > 0u)
    {
        HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &g_can_rx_hdr, g_can_rx_data);

        if (hcan->Instance == CAN1)
        {
            g_can1_rx_cnt++;
            HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
        }
        else if (hcan->Instance == CAN2)
        {
            g_can2_rx_cnt++;
        }
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
  MX_ADC1_Init();
  MX_CAN1_Init();
  MX_CAN2_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();
  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */

  /* -----------------------------------------------------------------------
   *  IWDG – prescaler 8, reload 4095 → timeout ≈ 1 s con LSI@32 kHz
   *  (F4 IWDG non ha windowed mode: solo Prescaler + Reload)
   * --------------------------------------------------------------------- */
  hiwdg.Instance       = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_8;
  hiwdg.Init.Reload    = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK) { Error_Handler(); }

  /* -----------------------------------------------------------------------
   *  ADC – avvio DMA continuo circolare (4 canali)
   * --------------------------------------------------------------------- */
  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)g_adc, ADC_NUM_CH);

  /* -----------------------------------------------------------------------
   *  CAN1 – filtro "accetta tutto", start, abilita notifica RX FIFO0
   *  bank 0-13 → CAN1,  bank 14-27 → CAN2  (SlaveStartFilterBank=14)
   * --------------------------------------------------------------------- */
  {
    CAN_FilterTypeDef f = {0};
    f.FilterBank           = 0;
    f.FilterMode           = CAN_FILTERMODE_IDMASK;
    f.FilterScale          = CAN_FILTERSCALE_32BIT;
    f.FilterIdHigh         = 0x0000u;
    f.FilterIdLow          = 0x0000u;
    f.FilterMaskIdHigh     = 0x0000u;  /* maschera 0 → accetta qualunque ID */
    f.FilterMaskIdLow      = 0x0000u;
    f.FilterFIFOAssignment = CAN_RX_FIFO0;
    f.FilterActivation     = ENABLE;
    f.SlaveStartFilterBank = 14;
    HAL_CAN_ConfigFilter(&hcan1, &f);
    if (HAL_CAN_Start(&hcan1) != HAL_OK) { Error_Handler(); }
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
  }

  /* -----------------------------------------------------------------------
   *  CAN2 – stesso schema (bank 14)
   * --------------------------------------------------------------------- */
  {
    CAN_FilterTypeDef f = {0};
    f.FilterBank           = 14;
    f.FilterMode           = CAN_FILTERMODE_IDMASK;
    f.FilterScale          = CAN_FILTERSCALE_32BIT;
    f.FilterIdHigh         = 0x0000u;
    f.FilterIdLow          = 0x0000u;
    f.FilterMaskIdHigh     = 0x0000u;
    f.FilterMaskIdLow      = 0x0000u;
    f.FilterFIFOAssignment = CAN_RX_FIFO0;
    f.FilterActivation     = ENABLE;
    f.SlaveStartFilterBank = 14;
    HAL_CAN_ConfigFilter(&hcan2, &f);
    if (HAL_CAN_Start(&hcan2) != HAL_OK) { Error_Handler(); }
    HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING);
  }

  /* -----------------------------------------------------------------------
   *  TIM2 IC DMA – avvio circolare su CH1 (PA0) e CH3 (PB10)
   * --------------------------------------------------------------------- */
  HAL_TIM_IC_Start_DMA(&htim2, TIM_CHANNEL_1, g_ic_buf_ch1, IC_BUF_SIZE);
  HAL_TIM_IC_Start_DMA(&htim2, TIM_CHANNEL_3, g_ic_buf_ch3, IC_BUF_SIZE);

  Serial_Log("=== Nucleo F446RE test start ===\r\n");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    uint32_t now = HAL_GetTick();

    /* ---------------------------------------------------------------
     *  Pulsante B1 → TX su CAN1 (ID 0x123, 8 byte)
     * ------------------------------------------------------------- */
    if (g_btn_tx_req)
    {
      g_btn_tx_req = 0u;

      CAN_TxHeaderTypeDef txh = {0};
      txh.StdId              = 0x123u;
      txh.IDE                = CAN_ID_STD;
      txh.RTR                = CAN_RTR_DATA;
      txh.DLC                = 8u;
      txh.TransmitGlobalTime = DISABLE;

      uint8_t  txd[8]    = {0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02, 0x03, 0x04};
      uint32_t txMailbox = 0;
      if (HAL_CAN_AddTxMessage(&hcan1, &txh, txd, &txMailbox) == HAL_OK)
        Serial_Log("CAN1 TX ok (mb=%lu)\r\n", (unsigned long)txMailbox);
      else
        Serial_Log("CAN1 TX ERR (bus assente?)\r\n");
    }

    /* ---------------------------------------------------------------
     *  Bus-off status – con AutoBusOff=ENABLE il recovery è automatico;
     *  log solo per visibilità in debug.
     * ------------------------------------------------------------- */
    if (hcan1.Instance->ESR & CAN_ESR_BOFF)
      Serial_Log("CAN1: bus-off rilevato\r\n");
    if (hcan2.Instance->ESR & CAN_ESR_BOFF)
      Serial_Log("CAN2: bus-off rilevato\r\n");

    /* ---------------------------------------------------------------
     *  RUOTA 1 – TIM2 CH1 (PA0)
     *
     *  NDTR del DMA conta quante scritture mancano a riempire il buffer.
     *  curr_idx = posizione corrente di scrittura nel buffer circolare.
     *  Il % IC_BUF_SIZE gestisce il caso NDTR==0 (wrap-around DMA).
     * ------------------------------------------------------------- */
    {
      uint16_t ndtr     = (uint16_t)__HAL_DMA_GET_COUNTER(htim2.hdma[TIM_DMA_ID_CC1]);
      uint16_t curr_idx = (uint16_t)((IC_BUF_SIZE - ndtr) % IC_BUF_SIZE);

      if (curr_idx != g_last_idx_ch1)
      {
        /* ultimi due timestamp catturati in ordine circolare */
        uint16_t i_new  = (uint16_t)((curr_idx - 1u + IC_BUF_SIZE) % IC_BUF_SIZE);
        uint16_t i_prev = (uint16_t)((curr_idx - 2u + IC_BUF_SIZE) % IC_BUF_SIZE);

        /* sottrazione uint32 gestisce overflow del counter a 32 bit */
        uint32_t diff = g_ic_buf_ch1[i_new] - g_ic_buf_ch1[i_prev];
        if (diff > 0u)
          /* 30 000 000 = 60 × 500 000 tick/s  (tick = 2 µs) */
          g_rpm1 = 30000000.0f / ((float)diff * NUM_DENTI);

        g_last_idx_ch1 = curr_idx;
        g_timeout_ch1  = 0u;
      }
      else
      {
        if (g_timeout_ch1 < 0xFFFFu) g_timeout_ch1++;
        if (g_timeout_ch1 > 10u) g_rpm1 = 0.0f;   /* ~100 ms → ruota ferma */
      }
    }

    /* ---------------------------------------------------------------
     *  RUOTA 2 – TIM2 CH3 (PB10)
     * ------------------------------------------------------------- */
    {
      uint16_t ndtr     = (uint16_t)__HAL_DMA_GET_COUNTER(htim2.hdma[TIM_DMA_ID_CC3]);
      uint16_t curr_idx = (uint16_t)((IC_BUF_SIZE - ndtr) % IC_BUF_SIZE);

      if (curr_idx != g_last_idx_ch3)
      {
        uint16_t i_new  = (uint16_t)((curr_idx - 1u + IC_BUF_SIZE) % IC_BUF_SIZE);
        uint16_t i_prev = (uint16_t)((curr_idx - 2u + IC_BUF_SIZE) % IC_BUF_SIZE);

        uint32_t diff = g_ic_buf_ch3[i_new] - g_ic_buf_ch3[i_prev];
        if (diff > 0u)
          g_rpm2 = 30000000.0f / ((float)diff * NUM_DENTI);

        g_last_idx_ch3 = curr_idx;
        g_timeout_ch3  = 0u;
      }
      else
      {
        if (g_timeout_ch3 < 0xFFFFu) g_timeout_ch3++;
        if (g_timeout_ch3 > 10u) g_rpm2 = 0.0f;
      }
    }

    /* ---------------------------------------------------------------
     *  Velocità veicolo
     *  km/h = rpm_medio × circ[m] × 60 / 1000
     * ------------------------------------------------------------- */
    g_speed_kmh = ((g_rpm1 + g_rpm2) * 0.5f * CIRC_RUOTA_M * 60.0f) / 1000.0f;

    /* ---------------------------------------------------------------
     *  Log UART ogni 200 ms
     *  ADC raw 12-bit, RPM ruote, velocità, contatori CAN RX
     * ------------------------------------------------------------- */
    if (now - g_log_tick >= 200u)
    {
      g_log_tick = now;
      Serial_Log("ADC[%4d,%4d,%4d,%4d] RPM1=%6.1f RPM2=%6.1f V=%5.1f km/h RX1=%u RX2=%u\r\n",
                 (int)g_adc[0], (int)g_adc[1], (int)g_adc[2], (int)g_adc[3],
                 (double)g_rpm1, (double)g_rpm2, (double)g_speed_kmh,
                 (unsigned int)g_can1_rx_cnt, (unsigned int)g_can2_rx_cnt);
    }

    /* kick watchdog (timeout IWDG ≈ 1 s) */
    HAL_IWDG_Refresh(&hiwdg);

    HAL_Delay(10);
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
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
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

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 4;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_7;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = 3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_14;
  sConfig.Rank = 4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */
  /*
   * CubeMX genera ContinuousConvMode=DISABLE e EOCSelection=SINGLE_CONV;
   * forziamo qui le due impostazioni necessarie per DMA circolare.
   * Canali, ScanConvMode, NbrOfConversion e DMAContinuousRequests sono
   * già corretti nel codice generato sopra.
   */
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.EOCSelection       = ADC_EOC_SEQ_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK) { Error_Handler(); }
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

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 15;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_1TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_4TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = ENABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
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

  /* USER CODE END CAN2_Init 1 */
  hcan2.Instance = CAN2;
  hcan2.Init.Prescaler = 15;
  hcan2.Init.Mode = CAN_MODE_NORMAL;
  hcan2.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan2.Init.TimeSeg1 = CAN_BS1_1TQ;
  hcan2.Init.TimeSeg2 = CAN_BS2_4TQ;
  hcan2.Init.TimeTriggeredMode = DISABLE;
  hcan2.Init.AutoBusOff = ENABLE;
  hcan2.Init.AutoWakeUp = DISABLE;
  hcan2.Init.AutoRetransmission = DISABLE;
  hcan2.Init.ReceiveFifoLocked = DISABLE;
  hcan2.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN2_Init 2 */

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
  hiwdg.Init.Prescaler = IWDG_PRESCALER_4;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 179;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 8;
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

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
  /* DMA1_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
  /* DMA1_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

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
