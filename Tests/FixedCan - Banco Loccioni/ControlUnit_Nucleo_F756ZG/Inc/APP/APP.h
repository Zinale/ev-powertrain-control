#ifndef APP_H
#define APP_H

#include "main.h"
#include <stdint.h>

/**
 * @brief Inizializza il modulo APP (filtri, variabili interne).
 */
void APP_Init(void);

/**
 * @brief Aggiorna il valore del pedale a partire da una lettura ADC grezza.
 *
 * @param raw_adc_val Valore ADC grezzo (es. adc_buf[0]).
 */
void APP_Update(uint16_t raw_adc_val);

/**
 * @brief Ritorna il valore filtrato del pedale in unità ADC.
 *
 * @return Valore filtrato (stesso range dell'ADC, es. 0..4095).
 */
uint16_t APP_GetFiltered(void);

/**
 * @brief Ritorna la posizione del pedale in percentuale 0–100.
 *
 * @return Percentuale intera 0..100.
 */
uint8_t APP_GetPercent(void);

#endif /* APP_H */
