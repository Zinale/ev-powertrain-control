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

FDCAN_HandleTypeDef hfdcan1;
FDCAN_HandleTypeDef hfdcan2;

IWDG_HandleTypeDef hiwdg;

UART_HandleTypeDef hlpuart1;
DMA_HandleTypeDef hdma_lpuart1_tx;

TIM_HandleTypeDef htim2;
DMA_HandleTypeDef hdma_tim2_ch2;
DMA_HandleTypeDef hdma_tim2_ch3;

/* USER CODE BEGIN PV */

/* -----------------------------------------------------------------------
 *  ADC – 4 canali DMA continuo (circolare)
 *  Buffer: [0]=PB14/CH5  [1]=PB12/CH11  [2]=PB1/CH12  [3]=PB11/CH14
 * --------------------------------------------------------------------- */
#define ADC_NUM_CH   4u
volatile uint16_t g_adc[ADC_NUM_CH];          /* raw 12-bit ADC values  */

/* -----------------------------------------------------------------------
 *  LPUART1 – debug su ST-Link VCP via DMA
 *  Pattern identico a Serial.c (buffer DMA + flag busy + TxCplt callback)
 * --------------------------------------------------------------------- */
#define LPUART_BUF_SIZE   128u
static char      g_lpuart_buf[LPUART_BUF_SIZE];
volatile uint8_t g_lpuart_busy = 0;

/* -----------------------------------------------------------------------
 *  FDCAN  (CAN1 = PA11/PA12 @ 500 kbps,  CAN2 = PB5/PB6 @ 500 kbps)
 *
 *  NOTA: la G474RE NON bypassa automaticamente un bus CAN assente.
 *  HAL_FDCAN_Init / Start riescono sempre; la trasmissione fallirà
 *  silenziosamente se non c'è nessun altro nodo ad ACK-are.
 *  Con AutoRetransmission=DISABLE (già impostato dal CubeMX) il TX
 *  viene tentato una sola volta e non si innesca il bus-off loop.
 *  Il bus-off viene comunque rilevato e recuperato nel main loop.
 *  Per test senza hardware CAN → cambiare Mode in
 *  FDCAN_MODE_INTERNAL_LOOPBACK nel .ioc e rigenerare.
 * --------------------------------------------------------------------- */
volatile uint8_t       g_btn_tx_req  = 0;     /* richiesta TX da pulsante */
FDCAN_RxHeaderTypeDef  g_can_rx_hdr;
uint8_t                g_can_rx_data[8];
volatile uint32_t      g_can1_rx_cnt = 0;
volatile uint32_t      g_can2_rx_cnt = 0;

/* -----------------------------------------------------------------------
 *  Speed sensors – TIM2 Input Capture DMA circolare
 *  PB3  → TIM2_CH2 → ruota 1
 *  PB10 → TIM2_CH3 → ruota 2
 *
 *  Clock TIM2: 170 MHz / (Prescaler+1) = 170 MHz / 170 = 1 MHz
 *  → 1 tick = 1 µs  → diff [µs] tra due denti consecutivi
 *
 *  rpm  = 60 × 10⁶ / (diff_µs × NUM_DENTI)
 *  km/h = rpm_medio × CIRC_RUOTA_M × 60 / 1000
 * --------------------------------------------------------------------- */
#define NUM_DENTI      22.0f
#define CIRC_RUOTA_M    1.8f              /* circonferenza ruota [m]       */
#define IC_BUF_SIZE    16u

uint32_t          g_ic_buf_ch2[IC_BUF_SIZE];  /* DMA scrive TIM2 CH2 qui */
uint32_t          g_ic_buf_ch3[IC_BUF_SIZE];  /* DMA scrive TIM2 CH3 qui */
uint16_t          g_last_idx_ch2 = 0;
uint16_t          g_last_idx_ch3 = 0;
uint16_t          g_timeout_ch2  = 0;         /* cnt loop senza nuovi dati */
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
static void MX_IWDG_Init(void);
static void MX_LPUART1_UART_Init(void);
static void MX_FDCAN1_Init(void);
static void MX_FDCAN2_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* -----------------------------------------------------------------------
 *  Serial_Log – invia stringa formattata su LPUART1 via DMA
 *  Stesso pattern di Serial.c: buffer statico + flag busy + callback
 *  Il chiamante attende al più 20 ms che il TX precedente finisca;
 *  se scade il timeout il messaggio viene scartato (no blocco).
 * --------------------------------------------------------------------- */
void Serial_Log(const char *fmt, ...)
{
    uint32_t t0 = HAL_GetTick();
    while (g_lpuart_busy && (HAL_GetTick() - t0 < 20u)) {}
    if (g_lpuart_busy) return;

    va_list ap;
    va_start(ap, fmt);
    int len = vsnprintf(g_lpuart_buf, sizeof(g_lpuart_buf), fmt, ap);
    va_end(ap);

    if (len <= 0 || len >= (int)sizeof(g_lpuart_buf)) return;

    g_lpuart_busy = 1u;
    if (HAL_UART_Transmit_DMA(&hlpuart1, (uint8_t *)g_lpuart_buf, (uint16_t)len) != HAL_OK)
        g_lpuart_busy = 0u;
}

/* -----------------------------------------------------------------------
 *  HAL Callbacks
 * --------------------------------------------------------------------- */

/* TX LPUART completato → libera il flag (chiamato da DMA1_Ch1 IRQ) */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == LPUART1)
        g_lpuart_busy = 0u;
}

/* Pulsante B1 (PC13, rising edge EXTI) → schedula TX su CAN1 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == USER_BUTTON_Pin)
        g_btn_tx_req = 1u;
}

/*
 * FDCAN RX FIFO0 callback – svuota il FIFO in loop (gestisce burst)
 *  • CAN1: toggle LED verde (PA5) ad ogni frame ricevuto
 *  • CAN2: incrementa solo il contatore
 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    if (!(RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE)) return;

    while (HAL_FDCAN_GetRxFifoFillLevel(hfdcan, FDCAN_RX_FIFO0) > 0u)
    {
        HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &g_can_rx_hdr, g_can_rx_data);

        if (hfdcan->Instance == FDCAN1)
        {
            g_can1_rx_cnt++;
            HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);
        }
        else if (hfdcan->Instance == FDCAN2)
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
  MX_IWDG_Init();
  MX_LPUART1_UART_Init();
  MX_FDCAN1_Init();
  MX_FDCAN2_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  /* -----------------------------------------------------------------------
   *  ADC – calibrazione offset + avvio DMA continuo circolare
   * --------------------------------------------------------------------- */
  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)g_adc, ADC_NUM_CH);

  /* -----------------------------------------------------------------------
   *  FDCAN1 – filtro "accetta tutto" standard, start, notifica RX
   * --------------------------------------------------------------------- */
  {
    FDCAN_FilterTypeDef f = {0};
    f.IdType       = FDCAN_STANDARD_ID;
    f.FilterIndex  = 0;
    f.FilterType   = FDCAN_FILTER_MASK;
    f.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    f.FilterID1    = 0x000u;
    f.FilterID2    = 0x000u;   /* maschera 0 → accetta qualunque ID standard */

    HAL_FDCAN_ConfigFilter(&hfdcan1, &f);
    HAL_FDCAN_ConfigGlobalFilter(&hfdcan1,
        FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_ACCEPT_IN_RX_FIFO0,
        FDCAN_REJECT, FDCAN_REJECT);
    if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK) { Error_Handler(); }
    HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
  }

  /* -----------------------------------------------------------------------
   *  FDCAN2 – stesso schema
   * --------------------------------------------------------------------- */
  {
    FDCAN_FilterTypeDef f = {0};
    f.IdType       = FDCAN_STANDARD_ID;
    f.FilterIndex  = 0;
    f.FilterType   = FDCAN_FILTER_MASK;
    f.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    f.FilterID1    = 0x000u;
    f.FilterID2    = 0x000u;

    HAL_FDCAN_ConfigFilter(&hfdcan2, &f);
    HAL_FDCAN_ConfigGlobalFilter(&hfdcan2,
        FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_ACCEPT_IN_RX_FIFO0,
        FDCAN_REJECT, FDCAN_REJECT);
    if (HAL_FDCAN_Start(&hfdcan2) != HAL_OK) { Error_Handler(); }
    HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
  }

  /* -----------------------------------------------------------------------
   *  TIM2 IC DMA – avvio circolare su CH2 (PB3) e CH3 (PB10)
   *  DMA già configurato in DMA_CIRCULAR nel MSP generato da CubeMX.
   * --------------------------------------------------------------------- */
  HAL_TIM_IC_Start_DMA(&htim2, TIM_CHANNEL_2, g_ic_buf_ch2, IC_BUF_SIZE);
  HAL_TIM_IC_Start_DMA(&htim2, TIM_CHANNEL_3, g_ic_buf_ch3, IC_BUF_SIZE);

  Serial_Log("=== Nucleo G474RE test start ===\r\n");

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

      FDCAN_TxHeaderTypeDef txh = {0};
      txh.Identifier          = 0x123u;
      txh.IdType              = FDCAN_STANDARD_ID;
      txh.TxFrameType         = FDCAN_DATA_FRAME;
      txh.DataLength          = FDCAN_DLC_BYTES_8;
      txh.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
      txh.BitRateSwitch       = FDCAN_BRS_OFF;
      txh.FDFormat            = FDCAN_CLASSIC_CAN;
      txh.TxEventFifoControl  = FDCAN_NO_TX_EVENTS;
      txh.MessageMarker       = 0;

      uint8_t txd[8] = {0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02, 0x03, 0x04};
      if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &txh, txd) == HAL_OK)
        Serial_Log("CAN1 TX ok\r\n");
      else
        Serial_Log("CAN1 TX ERR (bus assente?)\r\n");
    }

    /* ---------------------------------------------------------------
     *  Bus-off recovery – se il CAN è isolato, PSR.BO viene settato
     *  dopo che il TEC supera 256 (es. dopo 32 TX senza ACK).
     *  Stop + Start resetta i contatori di errore.
     * ------------------------------------------------------------- */
    if (hfdcan1.Instance->PSR & FDCAN_PSR_BO)
    {
      HAL_FDCAN_Stop(&hfdcan1);
      HAL_FDCAN_Start(&hfdcan1);
      Serial_Log("FDCAN1 bus-off: recovery\r\n");
    }
    if (hfdcan2.Instance->PSR & FDCAN_PSR_BO)
    {
      HAL_FDCAN_Stop(&hfdcan2);
      HAL_FDCAN_Start(&hfdcan2);
      Serial_Log("FDCAN2 bus-off: recovery\r\n");
    }

    /* ---------------------------------------------------------------
     *  RUOTA 1 – TIM2 CH2 (PB3)
     *
     *  NDTR del DMA conta quante scritture mancano a riempire il buffer.
     *  curr_idx = posizione corrente di scrittura nel buffer circolare.
     *  Il % IC_BUF_SIZE gestisce il caso NDTR==0 (wrap-around DMA).
     * ------------------------------------------------------------- */
    {
      uint16_t ndtr     = (uint16_t)__HAL_DMA_GET_COUNTER(htim2.hdma[TIM_DMA_ID_CC2]);
      uint16_t curr_idx = (uint16_t)((IC_BUF_SIZE - ndtr) % IC_BUF_SIZE);

      if (curr_idx != g_last_idx_ch2)
      {
        /* ultimi due timestamp catturati in ordine circolare */
        uint16_t i_new  = (uint16_t)((curr_idx - 1u + IC_BUF_SIZE) % IC_BUF_SIZE);
        uint16_t i_prev = (uint16_t)((curr_idx - 2u + IC_BUF_SIZE) % IC_BUF_SIZE);

        /* sottrazione uint32 gestisce overflow del counter a 32 bit */
        uint32_t diff = g_ic_buf_ch2[i_new] - g_ic_buf_ch2[i_prev];
        if (diff > 0u)
          g_rpm1 = 60000000.0f / ((float)diff * NUM_DENTI);

        g_last_idx_ch2 = curr_idx;
        g_timeout_ch2  = 0u;
      }
      else
      {
        if (g_timeout_ch2 < 0xFFFFu) g_timeout_ch2++;
        if (g_timeout_ch2 > 10u) g_rpm1 = 0.0f;   /* ~100 ms → ruota ferma */
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
          g_rpm2 = 60000000.0f / ((float)diff * NUM_DENTI);

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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
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

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.GainCompensation = 0;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.NbrOfConversion = 4;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_11;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_12;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_14;
  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief FDCAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_FDCAN1_Init(void)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = DISABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = 17;
  hfdcan1.Init.NominalSyncJumpWidth = 1;
  hfdcan1.Init.NominalTimeSeg1 = 15;
  hfdcan1.Init.NominalTimeSeg2 = 4;
  hfdcan1.Init.DataPrescaler = 1;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 1;
  hfdcan1.Init.DataTimeSeg2 = 1;
  hfdcan1.Init.StdFiltersNbr = 0;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* USER CODE END FDCAN1_Init 2 */

}

/**
  * @brief FDCAN2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_FDCAN2_Init(void)
{

  /* USER CODE BEGIN FDCAN2_Init 0 */

  /* USER CODE END FDCAN2_Init 0 */

  /* USER CODE BEGIN FDCAN2_Init 1 */

  /* USER CODE END FDCAN2_Init 1 */
  hfdcan2.Instance = FDCAN2;
  hfdcan2.Init.ClockDivider = FDCAN_CLOCK_DIV1;
  hfdcan2.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan2.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan2.Init.AutoRetransmission = DISABLE;
  hfdcan2.Init.TransmitPause = DISABLE;
  hfdcan2.Init.ProtocolException = DISABLE;
  hfdcan2.Init.NominalPrescaler = 17;
  hfdcan2.Init.NominalSyncJumpWidth = 1;
  hfdcan2.Init.NominalTimeSeg1 = 15;
  hfdcan2.Init.NominalTimeSeg2 = 4;
  hfdcan2.Init.DataPrescaler = 1;
  hfdcan2.Init.DataSyncJumpWidth = 1;
  hfdcan2.Init.DataTimeSeg1 = 1;
  hfdcan2.Init.DataTimeSeg2 = 1;
  hfdcan2.Init.StdFiltersNbr = 0;
  hfdcan2.Init.ExtFiltersNbr = 0;
  hfdcan2.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN2_Init 2 */

  /* USER CODE END FDCAN2_Init 2 */

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
  hiwdg.Init.Prescaler = IWDG_PRESCALER_8;
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
  * @brief LPUART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_LPUART1_UART_Init(void)
{

  /* USER CODE BEGIN LPUART1_Init 0 */

  /* USER CODE END LPUART1_Init 0 */

  /* USER CODE BEGIN LPUART1_Init 1 */

  /* USER CODE END LPUART1_Init 1 */
  hlpuart1.Instance = LPUART1;
  hlpuart1.Init.BaudRate = 115200;
  hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
  hlpuart1.Init.StopBits = UART_STOPBITS_1;
  hlpuart1.Init.Parity = UART_PARITY_NONE;
  hlpuart1.Init.Mode = UART_MODE_TX_RX;
  hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hlpuart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&hlpuart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&hlpuart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPUART1_Init 2 */

  /* USER CODE END LPUART1_Init 2 */

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
  htim2.Init.Prescaler = 169;
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
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
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
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA1_Channel2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
  /* DMA1_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);

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

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USER_LED_GPIO_Port, USER_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_BUTTON_Pin */
  GPIO_InitStruct.Pin = USER_BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_BUTTON_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USER_LED_Pin */
  GPIO_InitStruct.Pin = USER_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USER_LED_GPIO_Port, &GPIO_InitStruct);

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
