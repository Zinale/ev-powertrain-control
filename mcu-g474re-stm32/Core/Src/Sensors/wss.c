/* Core/Src/Sensors/wss.c */
#include "Sensors/wss.h"
#include "Sensors/filters.h"
#include <string.h>

static TIM_HandleTypeDef *s_htim = NULL;

static uint32_t s_ic_buf_ch2[WSS_IC_BUF_SIZE];
static uint32_t s_ic_buf_ch3[WSS_IC_BUF_SIZE];
static uint16_t s_last_idx_ch2 = 0;
static uint16_t s_last_idx_ch3 = 0;
static uint16_t s_timeout_ch2  = 0;
static uint16_t s_timeout_ch3  = 0;
static float    s_last_rpm_ch2 = 0.0f;   /* ultimo RPM valido, mantenuto fino al timeout */
static float    s_last_rpm_ch3 = 0.0f;

static MovingAverageFilter_t s_ma_speed_fl;
static MovingAverageFilter_t s_ma_speed_fr;
static MedianFilter_t        s_med_speed_fl;
static MedianFilter_t        s_med_speed_fr;

void WSS_Init(TIM_HandleTypeDef *htim) {
    s_htim = htim;
    memset(s_ic_buf_ch2, 0, sizeof(s_ic_buf_ch2));
    memset(s_ic_buf_ch3, 0, sizeof(s_ic_buf_ch3));
    s_last_idx_ch2 = 0;
    s_last_idx_ch3 = 0;
    s_timeout_ch2 = 0;
    s_timeout_ch3 = 0;
    s_last_rpm_ch2 = 0.0f;
    s_last_rpm_ch3 = 0.0f;
    Filter_Median_Init(&s_med_speed_fl, WSS_MEDIAN_WINDOW_SIZE);
    Filter_Median_Init(&s_med_speed_fr, WSS_MEDIAN_WINDOW_SIZE);
    Filter_MovingAverage_Init(&s_ma_speed_fl, WSS_MA_WINDOW_SIZE);
    Filter_MovingAverage_Init(&s_ma_speed_fr, WSS_MA_WINDOW_SIZE);

    /* Avvio canali DMA Input Capture su TIM2 */
    HAL_TIM_IC_Start_DMA(s_htim, TIM_CHANNEL_2, s_ic_buf_ch2, WSS_IC_BUF_SIZE);
    HAL_TIM_IC_Start_DMA(s_htim, TIM_CHANNEL_3, s_ic_buf_ch3, WSS_IC_BUF_SIZE);
}

static float compute_wheel_rpm(uint32_t *buf, uint16_t *last_idx, uint16_t *timeout_cnt,
                               float *last_rpm, uint32_t dma_channel_id) {
    uint16_t ndtr = (uint16_t)__HAL_DMA_GET_COUNTER(s_htim->hdma[dma_channel_id]);
    uint16_t curr_idx = (uint16_t)((WSS_IC_BUF_SIZE - ndtr) % WSS_IC_BUF_SIZE);

    if (curr_idx != *last_idx) {
        uint16_t i_new  = (uint16_t)((curr_idx - 1U + WSS_IC_BUF_SIZE) % WSS_IC_BUF_SIZE);
        uint16_t i_prev = (uint16_t)((curr_idx - 2U + WSS_IC_BUF_SIZE) % WSS_IC_BUF_SIZE);
        uint32_t diff = buf[i_new] - buf[i_prev];
        if (diff > 0U) *last_rpm = 60000000.0f / ((float)diff * WSS_NUM_TEETH);
        *last_idx = curr_idx;
        *timeout_cnt = 0U;
    } else {
        if (*timeout_cnt < 0xFFFFU) (*timeout_cnt)++;
        if (*timeout_cnt > WSS_TIMEOUT_CYCLES) *last_rpm = 0.0f;
    }
    return *last_rpm;
}

void WSS_Process(WSS_Output_t *out) {
    if (!out || !s_htim) return;

    /* 1. Calcolo RPM grezzo da delta timer */
    float raw_rpm_fl = compute_wheel_rpm(s_ic_buf_ch2, &s_last_idx_ch2, &s_timeout_ch2, &s_last_rpm_ch2, TIM_DMA_ID_CC2);
    float raw_rpm_fr = compute_wheel_rpm(s_ic_buf_ch3, &s_last_idx_ch3, &s_timeout_ch3, &s_last_rpm_ch3, TIM_DMA_ID_CC3);

    /* 2. Conversione in km/h grezza (km/h = rpm * circ * 60 / 1000) */
    float raw_kmh_fl = (raw_rpm_fl * WSS_WHEEL_CIRCUMFERENCE_M * 60.0f) / 1000.0f;
    float raw_kmh_fr = (raw_rpm_fr * WSS_WHEEL_CIRCUMFERENCE_M * 60.0f) / 1000.0f;

    /* 3. Filtraggio a due stadi (valori in scala x10 per precisione uint16) */
    uint16_t med_fl = Filter_Median_Apply(&s_med_speed_fl, (uint16_t)(raw_kmh_fl * 10.0f));
    uint16_t med_fr = Filter_Median_Apply(&s_med_speed_fr, (uint16_t)(raw_kmh_fr * 10.0f));

    out->speed_fl_kmh = (float)Filter_MovingAverage_Apply(&s_ma_speed_fl, med_fl) / 10.0f;
    out->speed_fr_kmh = (float)Filter_MovingAverage_Apply(&s_ma_speed_fr, med_fr) / 10.0f;

    /* 4. Ricalcolo RPM filtrati e stima velocità veicolo */
    out->rpm_fl = (out->speed_fl_kmh * 1000.0f) / (WSS_WHEEL_CIRCUMFERENCE_M * 60.0f);
    out->rpm_fr = (out->speed_fr_kmh * 1000.0f) / (WSS_WHEEL_CIRCUMFERENCE_M * 60.0f);
    out->vehicle_speed_kmh = (out->speed_fl_kmh + out->speed_fr_kmh) * 0.5f;
}