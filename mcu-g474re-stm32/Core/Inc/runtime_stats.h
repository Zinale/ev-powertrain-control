/**
 * @file runtime_stats.h
 * @brief Base tempo per le statistiche di runtime FreeRTOS (config
 *        GENERATE_RUN_TIME_STATS), basata sul cycle counter DWT del
 *        Cortex-M4 (nessuna periferica timer aggiuntiva richiesta).
 */

#ifndef INC_RUNTIME_STATS_H_
#define INC_RUNTIME_STATS_H_

#include <stdint.h>

/* Referenziate da FreeRTOSConfig.h (portCONFIGURE_TIMER_FOR_RUN_TIME_STATS /
 * portGET_RUN_TIME_COUNTER_VALUE): abilita e legge DWT->CYCCNT. */
void     RunTimeStats_ConfigureTimer(void);
uint32_t RunTimeStats_GetCounter(void);

#endif /* INC_RUNTIME_STATS_H_ */
