
/**
 * @file readings_manage.c
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief Readings Manager Task — ADC sampling, APPS and SAS update
 *
 * =============================================================================
 *  Readings Manager Task
 *  ---------------------------------------------------------------------------
 *  Handles all sensor acquisition at 1 kHz (1 ms period):
 *   1. Read raw ADC values via DMA-backed ADC_Manager
 *   2. Update APPS processing (filtering, plausibility, percent output)
 *      under Mutex_APPS — g_apps is the shared result
 *   3. Update SAS processing (filtering, angle conversion)
 *      under Mutex_SAS  — g_sas is the shared result
 *
 *  Ownership of raw buffers
 *  ---------------------------------------------------------------------------
 *  s_apps_raw[] and s_sas_raw live here and are never exposed directly.
 *  External code requesting raw values must call Readings_GetAppsRaw()
 *  while holding Mutex_APPS.
 * =============================================================================
 */

#include "Tasks/readings_manage.h"
#include "mutexes.h"
#include "cmsis_os.h"
#include "stm32f7xx_hal.h"

#include "ADC/ADC_Manager.h"
#include "APP/APPS.h"
#include "SAS/SAS.h"
#include "Communication/Can.h"

extern APPS_Data_t g_apps;
extern SAS_Data_t  g_sas;

#include <stdint.h>
#include <string.h>

#define READINGS_TASK_PERIOD_MS  4U   /**< Task period: 4 ms → 0.25 kHz        */

/** Convert ms to RTOS ticks (portable, requires kernel already started) */
#define MS_TO_TICKS(ms) ((uint32_t)((ms) * osKernelGetTickFreq() / 1000U))

extern ADC_HandleTypeDef hadc1;
static uint16_t s_apps_raw[APPS_NUM_SENSORS] = {0U};

static uint16_t s_sas_raw = 0U;


void Readings_Init(void)
{
    ADC_Manager_Init(&hadc1);
    APPS_Init();
    SAS_Init();
}


void ReadingsManageTask(void)
{
    uint32_t next_wake          = osKernelGetTickCount();
    const uint32_t period_ticks = MS_TO_TICKS(READINGS_TASK_PERIOD_MS);

    for (;;)
    {
        /* 
        *  1 - Sample raw ADC values from the DMA-backed manager
        *      These reads are always safe (DMA circular, no mutex needed here).
        */
        ADC_Manager_GetAPPS(s_apps_raw);
        ADC_Manager_GetSteering(&s_sas_raw);

        /* 
         *  2 - Update APPS processing under mutex
         *      APPS_Update() reads s_apps_raw and writes g_apps.
         */
        Mutex_APPS_Lock();
        APPS_Update(s_apps_raw);
        Mutex_APPS_Unlock();

        /* 
         *  3 - Update SAS processing under mutex
         *      SAS_Update() reads s_sas_raw and writes g_sas.
         */
        Mutex_SAS_Lock();
        SAS_Update(s_sas_raw);
        Mutex_SAS_Unlock();

        /* 
         *  4 - Drain CAN RX message queue and process all pending inverter messages
         *      CAN_ProcessQueueDrain() is ISR-SAFE (runs in task context, not ISR)
         *      and uses Mutexes internally to safely update inverter structures.
         */
        CAN_ProcessQueueDrain();

        /* 
         *  5 - Drain CAN2 RX message queue and process all pending car telemetry messages
         *      CAN_Car_ProcessQueueDrain() is ISR-SAFE (runs in task context, not ISR)
         *      and uses Mutexes internally to safely update car_data structure.
         */
        CAN_Car_ProcessQueueDrain();

        /* Deterministic timing - absolute tick prevents drift */
        next_wake += period_ticks;
        osDelayUntil(next_wake);
    }
}

void Readings_GetAppsRaw(uint16_t out[APPS_NUM_SENSORS])
{
    /* Caller MUST hold Mutex_APPS — see header documentation. */
    #ifdef TEST_MODE_SINGLE_ADC
        /* TEST MODE: Only sensor 1 available */
        out[0] = s_apps_raw[0];
        /* NOTE: Do NOT write out[1] — array size is APPS_NUM_SENSORS (=1) */
    #else
        /* Normal mode: First 2 channels are APPS1 and APPS2 */
        out[0] = s_apps_raw[0];
        out[1] = s_apps_raw[1];
    #endif
}

void Readings_GetApps(APPS_Data_t *out)
{
    /* Caller MUST hold Mutex_APPS — see header documentation. */
    if (out != NULL) { *out = g_apps; }
}

void Readings_GetSAS(SAS_Data_t *out)
{
    /* Caller MUST hold Mutex_SAS — see header documentation. */
    if (out != NULL) { *out = g_sas; }
}
