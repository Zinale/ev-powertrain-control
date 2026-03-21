/*
 * watchdog.h
 * Simple wrapper for Independent Watchdog (IWDG) refresh
 */
#ifndef WATCHDOG_H
#define WATCHDOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f7xx_hal.h"

void Watchdog_Init(void);
void Watchdog_Refresh(void);

#ifdef __cplusplus
}
#endif

#endif /* WATCHDOG_H */
