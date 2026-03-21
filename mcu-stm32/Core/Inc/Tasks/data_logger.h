/**
 * @file data_logger.h
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief Data logger task - periodic debug output over UART5
 */

#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


/**
 * @brief FreeRTOS task: prints vehicle state on UART5 at 10 Hz and
 *        transmits CAN2 telemetry packets (0x002, 0x003, 0x006, 0x108, 0x250, 0x251).
 *
 * Displays:
 *  - APPS sensor values and error code
 *  - SAS steering angle
 *  - Inverter left/right state, speed, torque, temperatures
 *  - Control: torque request and dynamic limit
 *  - Motor drive state (IDLE / DRIVE / REGEN)
 *  - Task timing diagnostics
 *  - CAN TX/RX statistics
 *
 * CAN2 TX packets (10 Hz):
 *  - 0x002 / 0x003 — INV1/INV2 error snapshot   (only on INV_STATE_ERROR)
 *  - 0x006         — MCU error bitmask            (cyclically)
 *  - 0x108         — APPs % + SAS %              (cyclically)
 *  - 0x250         — INV1 status/speed/temps      (cyclically)
 *  - 0x251         — INV2 status/speed/temps      (cyclically)
 */
void DataLoggerTask(void);

#ifdef __cplusplus
}
#endif

#endif /* DATA_LOGGER_H */
