#include "Connectivity/Serial.h"
#include "mutexes.h"
#include "cmsis_os2.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

char lpuart1_dma_buffer[LPUART1_DMA_BUFFER_SIZE];
volatile uint8_t lpuart1_tx_busy = 0;
extern UART_HandleTypeDef hlpuart1;

void Serial_Log(SerialChannel_t ch, const char *format, ...)
{
#ifdef LPUART1_LOG_ENABLED
    if (!(ch & LOG_CH_LPUART1)) return;

    Mutex_LPUART1_Lock();

    uint32_t timeout_ms = 20U;
    while (lpuart1_tx_busy && (timeout_ms > 0U))
    {
        if (osKernelGetState() == osKernelRunning) {
            osDelay(1);
        } else {
            HAL_Delay(1);
        }
        timeout_ms--;
    }

    if (lpuart1_tx_busy)
    {
        HAL_UART_AbortTransmit(&hlpuart1);
        lpuart1_tx_busy = 0;
    }

    va_list args;
    va_start(args, format);
    int length = vsnprintf(lpuart1_dma_buffer, sizeof(lpuart1_dma_buffer), format, args);
    va_end(args);

    if (length > 0 && length < (int)sizeof(lpuart1_dma_buffer))
    {
        lpuart1_tx_busy = 1;
        if (HAL_UART_Transmit_DMA(&hlpuart1, (uint8_t *)lpuart1_dma_buffer, (uint16_t)length) != HAL_OK)
        {
            lpuart1_tx_busy = 0;
        }
    }

    Mutex_LPUART1_Unlock();
#endif
}