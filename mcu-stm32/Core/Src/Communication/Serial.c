/**
 * @file Serial.c
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief UART communication functions implementation
 */

#include "Serial.h"
#include "mutexes.h"
#include "cmsis_os.h"
#include "Sensors/APPS.h"
#include "Config.h"

char uart4_dma_buffer[UART4_DMA_BUFFER_SIZE];
volatile uint8_t uart4_tx_busy = 0;
extern UART_HandleTypeDef huart4;

char uart3_dma_buffer[UART3_DMA_BUFFER_SIZE];
volatile uint8_t uart3_tx_busy = 0;
extern UART_HandleTypeDef huart3;

/* External variables from other modules */
extern volatile int16_t torque_request;    /**< From BaseControlMotor.c — volatile: written by InvertersManageTask */
extern volatile int16_t torque_limit_dyn;  /**< From BaseControlMotor.c — volatile: written by InvertersManageTask */
extern uint32_t g_can_tx_ok_count;         /**< From BaseControlMotor.c */
extern uint32_t g_can_tx_fail_count;       /**< From BaseControlMotor.c */
extern uint32_t g_can_rx_count;            /**< From BaseControlMotor.c */




/**
 * @brief Log a printf-style message to one or more UART channels via DMA.
 *
 * The format string is resolved exactly once into a temporary stack buffer.
 * Each channel selected in @p ch is then transmitted under its own mutex.
 * Channels whose compile-time guard is absent (SERIAL4/3_LOG_ENABLED in
 * Config.h) are silently skipped — the preprocessor removes that branch
 * entirely, leaving zero CPU overhead.
 *
 * Usage examples:
 *   Serial_Log(LOG_CH_UART4, "speed=%d\r\n", rpm);   // UART4 only  (→ ESP32)
 *   Serial_Log(LOG_CH_UART3, "speed=%d\r\n", rpm);   // UART3 only  (→ PC)
 *   Serial_Log(LOG_CH_BOTH,  "speed=%d\r\n", rpm);   // both channels
 *
 * @param ch      Bitmask of target channels (LOG_CH_UART4, LOG_CH_UART3, LOG_CH_BOTH).
 * @param format  printf-style format string.
 * @param ...     Variable arguments.
 */
void Serial_Log(SerialChannel_t ch, const char *format, ...)
{
    /* Format once into a stack buffer shared by all channels */
    char tmp[UART4_DMA_BUFFER_SIZE];

    va_list args;
    va_start(args, format);
    int length = vsnprintf(tmp, sizeof(tmp), format, args);
    va_end(args);

    if (length <= 0 || length >= (int)sizeof(tmp)) return;

	#ifdef SERIAL4_LOG_ENABLED
		if (ch & LOG_CH_UART4)
		{
			Mutex_UART4_Lock();
			memcpy(uart4_dma_buffer, tmp, (size_t)length);
			uart4_tx_busy = 1;

			if (HAL_UART_Transmit_DMA(&huart4, (uint8_t *)uart4_dma_buffer, (uint16_t)length) == HAL_OK)
			{
				uint32_t to4 = 500U;
				while (uart4_tx_busy && to4 > 0U) { osDelay(1); to4--; }
				if (uart4_tx_busy) { HAL_UART_AbortTransmit(&huart4); uart4_tx_busy = 0; }
			}
			else
			{
				uart4_tx_busy = 0;
			}
			Mutex_UART4_Unlock();
		}
	#endif

	#ifdef SERIAL3_LOG_ENABLED
		if (ch & LOG_CH_UART3)
		{
			Mutex_UART3_Lock();
			memcpy(uart3_dma_buffer, tmp, (size_t)length);
			uart3_tx_busy = 1;

			if (HAL_UART_Transmit_DMA(&huart3, (uint8_t *)uart3_dma_buffer, (uint16_t)length) == HAL_OK)
			{
				uint32_t to3 = 100U;
				while (uart3_tx_busy && to3 > 0U) { osDelay(1); to3--; }
				if (uart3_tx_busy) { HAL_UART_AbortTransmit(&huart3); uart3_tx_busy = 0; }
			}
			else
			{
				uart3_tx_busy = 0;
			}
			Mutex_UART3_Unlock();
		}
	#endif

	#ifdef SERIAL6_LOG_ENABLED
		#error "SERIAL6_LOG_ENABLED is obsolete: UART6 has been replaced by UART4. Use SERIAL4_LOG_ENABLED in Config.h."
	#endif
}

