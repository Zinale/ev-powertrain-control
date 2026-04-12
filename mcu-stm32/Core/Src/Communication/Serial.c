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

char uart5_dma_buffer[UART5_DMA_BUFFER_SIZE];
volatile uint8_t uart5_tx_busy = 0;
extern UART_HandleTypeDef huart5;

char uart3_dma_buffer[UART3_DMA_BUFFER_SIZE];
volatile uint8_t uart3_tx_busy = 0;
extern UART_HandleTypeDef huart3;

char uart6_dma_buffer[UART6_DMA_BUFFER_SIZE];
volatile uint8_t uart6_tx_busy = 0;
extern UART_HandleTypeDef huart6;

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
 * Channels whose compile-time guard is absent (SERIAL5/3_LOG_ENABLED in
 * Config.h) are silently skipped — the preprocessor removes that branch
 * entirely, leaving zero CPU overhead.
 *
 * Usage examples:
 *   Serial_Log(LOG_CH_UART5, "speed=%d\r\n", rpm);   // UART5 only
 *   Serial_Log(LOG_CH_UART3, "speed=%d\r\n", rpm);   // UART3 only
 *   Serial_Log(LOG_CH_BOTH,  "speed=%d\r\n", rpm);   // both channels
 *
 * @param ch      Bitmask of target channels (LOG_CH_UART5, LOG_CH_UART3, LOG_CH_BOTH).
 * @param format  printf-style format string.
 * @param ...     Variable arguments.
 */
void Serial_Log(SerialChannel_t ch, const char *format, ...)
{
    /* Format once into a stack buffer shared by all channels */
    char tmp[UART5_DMA_BUFFER_SIZE];

    va_list args;
    va_start(args, format);
    int length = vsnprintf(tmp, sizeof(tmp), format, args);
    va_end(args);

    if (length <= 0 || length >= (int)sizeof(tmp)) return;

	#ifdef SERIAL5_LOG_ENABLED
		if (ch & LOG_CH_UART5)
		{
			Mutex_UART5_Lock();
			memcpy(uart5_dma_buffer, tmp, (size_t)length);
			uart5_tx_busy = 1;

			if (HAL_UART_Transmit_DMA(&huart5, (uint8_t *)uart5_dma_buffer, (uint16_t)length) == HAL_OK)
			{
				while (uart5_tx_busy) { osDelay(1); }
			}
			else
			{
				uart5_tx_busy = 0;
			}
			Mutex_UART5_Unlock();
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
				while (uart3_tx_busy) { osDelay(1); }
			}
			else
			{
				uart3_tx_busy = 0;
			}
			Mutex_UART3_Unlock();
		}
	#endif

	#ifdef SERIAL6_LOG_ENABLED
		/* UART6 mirrors UART5: inviare se LOG_CH_UART5 o LOG_CH_UART6 sono attivi */
		if ((ch & LOG_CH_UART5) || (ch & LOG_CH_UART6))
		{
			Mutex_UART6_Lock();
			memcpy(uart6_dma_buffer, tmp, (size_t)length);
			uart6_tx_busy = 1;

			if (HAL_UART_Transmit_DMA(&huart6, (uint8_t *)uart6_dma_buffer, (uint16_t)length) == HAL_OK)
			{
				while (uart6_tx_busy) { osDelay(1); }
			}
			else
			{
				uart6_tx_busy = 0;
			}
			Mutex_UART6_Unlock();
		}
	#endif
}

