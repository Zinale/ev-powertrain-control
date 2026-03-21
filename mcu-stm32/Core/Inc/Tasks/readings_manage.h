/**
 * @file readings_manage.h
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief Readings Manager Task — ADC sampling, APPS and SAS update
 *
 * This task owns the raw ADC buffers and is solely responsible for:
 *  - Triggering ADC conversions via ADC_Manager
 *  - Calling APPS_Update() and SAS_Update() under their respective mutexes
 *
 * Period: 1 ms (1 kHz) — same rate as the old TaskFast ADC/sensor block.
 *
 * Thread-safety notes
 * -------------------
 *  - g_apps  is protected by Mutex_APPS
 *  - g_sas   is protected by Mutex_SAS
 *  - Readings_GetAppsRaw() MUST be called while holding Mutex_APPS,
 *    because it reads the raw buffer that this task also writes.
 */

#ifndef READINGS_MANAGE_H
#define READINGS_MANAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "APP/APPS.h"   /* APPS_NUM_SENSORS, APPS_Data_t */
#include "SAS/SAS.h"    /* SAS_Data_t */

/* ─── Initialisation ────────────────────────────────────────────────────── */

/**
 * @brief Initialise ADC, APPS and SAS drivers.
 *
 * Must be called once before osKernelStart().
 * Already invoked from main.c USER CODE section.
 */
void Readings_Init(void);

/* ─── FreeRTOS task entry point ─────────────────────────────────────────── */

/**
 * @brief FreeRTOS task: samples ADC and updates APPS / SAS at 1 kHz.
 *
 * Never returns — contains the infinite for(;;) loop.
 * Registered in main.c as:
 *   osThreadNew(StartReadingsManager, NULL, &ReadingsManage_attributes)
 */
void ReadingsManageTask(void);

/* ─── Shared-data accessors ─────────────────────────────────────────────── */

/**
 * @brief Copy the last raw ADC values captured for APPS sensors.
 *
 * @param[out] out  Array of exactly APPS_NUM_SENSORS uint16_t values.
 *                  out[0] = APPS sensor 1, out[1] = APPS sensor 2.
 *
 * @note MUST be called while holding Mutex_APPS.
 */
void Readings_GetAppsRaw(uint16_t out[APPS_NUM_SENSORS]);

/**
 * @brief Copy the latest processed APPS data snapshot.
 *        Must be called while Mutex_APPS is held.
 */
void Readings_GetApps(APPS_Data_t *out);

/**
 * @brief Copy the latest processed SAS data snapshot.
 *        Must be called while Mutex_SAS is held.
 */
void Readings_GetSAS(SAS_Data_t *out);

#ifdef __cplusplus
}
#endif

#endif /* READINGS_MANAGE_H */
