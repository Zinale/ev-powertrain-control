/**
 * @file runtime_stats.c
 * @brief Base tempo per le statistiche di runtime FreeRTOS: cycle counter
 *        DWT del Cortex-M4 (CYCCNT), a 170 MHz -> risoluzione ~5.9 ns,
 *        overflow (wrap a 32 bit) ogni ~25 s. Sufficiente per confrontare
 *        il tempo CPU consumato dai singoli task tra due letture ravvicinate.
 */

#include "runtime_stats.h"
#include "stm32g4xx.h"

void RunTimeStats_ConfigureTimer(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;  /* abilita il blocco trace (DWT) */
    DWT->CYCCNT = 0U;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;            /* avvia il cycle counter */
}

uint32_t RunTimeStats_GetCounter(void)
{
    return DWT->CYCCNT;
}
