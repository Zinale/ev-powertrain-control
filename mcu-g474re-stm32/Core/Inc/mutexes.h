/**
 * @file mutexes.h
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief Centralized mutex declarations for the MCU-ECU FreeRTOS system
 *
 * Provides thread-safe wrapper function prototypes for all FreeRTOS
 * mutexes used throughout the MCU. Implementations are in mutexes.c.
 */

 /*
  * Mutexes defined (from .ioc):
 *  - mutex_VehicleData       : Vehicle data structure protection
 *  - mutex_LPUART1           : LPUART1 serial communication protection
 *  - mutex_FDCAN1_MOTOR      : FDCAN1 motor controller communication protection
 *  - mutex_FDCAN2_MAIN       : FDCAN2 main controller communication protection
 */

#ifndef INC_MUTEXES_H_
#define INC_MUTEXES_H_

#include <stdbool.h>

/* ---- VEHICLE DATA -------------------------------------------------------- */
void Mutex_VehicleData_Lock(void);
void Mutex_VehicleData_Unlock(void);

/* ---- LPUART1 ------------------------------------------------------------- */
void Mutex_LPUART1_Lock(void);
void Mutex_LPUART1_Unlock(void);

/* ---- FDCAN1 MOTOR -------------------------------------------------------- */
void Mutex_FDCAN1_MOTOR_Lock(void);
void Mutex_FDCAN1_MOTOR_Unlock(void);

/* ---- FDCAN2 MAIN --------------------------------------------------------- */
void Mutex_FDCAN2_MAIN_Lock(void);
void Mutex_FDCAN2_MAIN_Unlock(void);


/* ---- Diagnostics --------------------------------------------------------- */
bool Mutex_IsInitialized(void);

#endif /* INC_MUTEXES_H_ */
