/**
 * @file mutexes.h
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief Centralized mutex declarations for the MCU-ECU FreeRTOS system
 *
 * Provides thread-safe wrapper function prototypes for all FreeRTOS
 * mutexes used throughout the MCU-ECU. Implementations are in mutexes.c.
 */

#ifndef INC_MUTEXES_H_
#define INC_MUTEXES_H_

#include <stdbool.h>

/* ---- APPS ---------------------------------------------------------------- */
void Mutex_APPS_Lock(void);
void Mutex_APPS_Unlock(void);

/* ---- SAS ----------------------------------------------------------------- */
void Mutex_SAS_Lock(void);
void Mutex_SAS_Unlock(void);

/* ---- Inverter left ------------------------------------------------------- */
void Mutex_INVERTER_L_Lock(void);
void Mutex_INVERTER_L_Unlock(void);

/* ---- Inverter right ------------------------------------------------------ */
void Mutex_INVERTER_R_Lock(void);
void Mutex_INVERTER_R_Unlock(void);

/* ---- UART5 --------------------------------------------------------------- */
void Mutex_UART5_Lock(void);
void Mutex_UART5_Unlock(void);

/* ---- UART3 --------------------------------------------------------------- */
void Mutex_UART3_Lock(void);
void Mutex_UART3_Unlock(void);

/* ---- CAN1 ---------------------------------------------------------------- */
void Mutex_CAN1_Lock(void);
void Mutex_CAN1_Unlock(void);

/* ---- Diagnostics --------------------------------------------------------- */
bool Mutex_IsInitialized(void);

#endif /* INC_MUTEXES_H_ */
