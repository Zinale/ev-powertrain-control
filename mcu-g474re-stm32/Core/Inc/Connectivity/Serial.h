/**
 * @file Serial.h
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief UART communication function declarations
 */

#ifndef INC_COMMUNICATION_SERIAL_H_
#define INC_COMMUNICATION_SERIAL_H_

#include "stm32g4xx_hal.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LPUART1_LOG_ENABLED

#define LPUART1_DMA_BUFFER_SIZE  512U
extern volatile uint8_t lpuart1_tx_busy;


typedef uint8_t SerialChannel_t;
#define LOG_CH_LPUART1  ((SerialChannel_t)0x01U)


/**
 * @brief Log a printf-style message to one or more UART channels.
 *
 * The string is formatted exactly once regardless of how many channels
 * are selected. Each active channel is then sent via DMA under its own mutex.
 * Channels disabled in Config.h are compiled out entirely.
 *
 * @param ch      Bitmask of target channels (LOG_CH_LPUART1).
 * @param format  printf-style format string.
 * @param ...     Variable arguments.
 */
void Serial_Log(SerialChannel_t ch, const char *format, ...);

/* Convenience macros */
#define LPUART1_Log(fmt, ...)  Serial_Log(LOG_CH_LPUART1, fmt, ##__VA_ARGS__)

#endif /* INC_COMMUNICATION_SERIAL_H_ */
