/**
 * @file Serial.h
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief UART communication function declarations
 */

#ifndef INC_COMMUNICATION_SERIAL_H_
#define INC_COMMUNICATION_SERIAL_H_

#include "stm32f7xx_hal.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* DMA buffer sizes */
#define UART4_DMA_BUFFER_SIZE  512U
#define UART3_DMA_BUFFER_SIZE  512U

/* Shared TX-busy flags (set by caller, cleared by DMA TC IRQ) */
extern volatile uint8_t uart4_tx_busy;
extern volatile uint8_t uart3_tx_busy;

/* Channel bitmask definitions for Serial_Log():
 *   Serial_Log(LOG_CH_UART4, "hello");   // UART4 only  (→ ESP32)
 *   Serial_Log(LOG_CH_UART3, "hello");   // UART3 only  (→ PC via STLink VCOM)
 *   Serial_Log(LOG_CH_BOTH,  "hello");   // UART4 + UART3
 *
 * Channels disabled via SERIAL4/3_LOG_ENABLED in Config.h are compiled
 * out entirely — zero CPU cost.
 */
typedef uint8_t SerialChannel_t;
#define LOG_CH_UART4  ((SerialChannel_t)0x01U)
#define LOG_CH_UART3  ((SerialChannel_t)0x02U)
#define LOG_CH_BOTH   ((SerialChannel_t)(LOG_CH_UART4 | LOG_CH_UART3))

/**
 * @brief Log a printf-style message to one or more UART channels.
 *
 * The string is formatted exactly once regardless of how many channels
 * are selected. Each active channel is then sent via DMA under its own mutex.
 * Channels disabled in Config.h are compiled out entirely.
 *
 * @param ch      Bitmask of target channels (LOG_CH_UART4, LOG_CH_UART3, LOG_CH_BOTH).
 * @param format  printf-style format string.
 * @param ...     Variable arguments.
 */
void Serial_Log(SerialChannel_t ch, const char *format, ...);

/* Convenience macros */
#define Serial4_Log(fmt, ...)  Serial_Log(LOG_CH_UART4, fmt, ##__VA_ARGS__)
#define Serial3_Log(fmt, ...)  Serial_Log(LOG_CH_UART3, fmt, ##__VA_ARGS__)

#endif /* INC_COMMUNICATION_SERIAL_H_ */
