#ifndef PLAUSIBILITY_H
#define PLAUSIBILITY_H

#include <stdint.h>
#include <stdbool.h>
#include "shared_data.h"

/* ============================================================================
 * Monitor di plausibilità - una serie di controlli di sicurezza indipendenti,
 * ognuno debounced (deve fallire per N cicli consecutivi prima di scattare) e
 * aggregati in una bitmask. Un qualsiasi bit attivo -> il veicolo va in
 * VEHICLE_STATE_IMPLAUSIBILITY (coppia tagliata).
 *
 * Per aggiungere un check: scrivi una static bool check_xxx(...) in
 * plausibility.c e aggiungi una riga alla tabella k_checks[].
 * ============================================================================ */
typedef enum {
    PLAUS_APPS_BRAKE        = (1U << 0),  /* APPS>25% & freno>20bar (EV.4.7), auto-latch fino a APPS<5% */
    PLAUS_APPS_DISAGREE     = (1U << 1),  /* |APPS1-APPS2| troppo grande (cross-check di APPS.c) */
    PLAUS_SENSOR_RANGE      = (1U << 2),  /* apps/brake/sterzo fuori dal range fisico */
    PLAUS_OVERPOWER         = (1U << 3),  /* potenza totale stimata > soglia (trip veloce) */
    PLAUS_OVERCURRENT       = (1U << 4),  /* corrente stimata > max scarica BMS (trip veloce) */
    PLAUS_TORQUE_GT_PEDAL   = (1U << 5),  /* coppia richiesta > proporzione pedale + margine */
    PLAUS_REGEN_WHILE_ACCEL = (1U << 6),  /* coppia rigenerativa mentre si accelera */
    PLAUS_DRIVE_WHILE_BRAKE = (1U << 7),  /* coppia di trazione con freno duro e pedale rilasciato */
    PLAUS_MOTOR_OVERSPEED   = (1U << 8),  /* giri motore oltre il massimo */
    PLAUS_BUS_VOLTAGE       = (1U << 9),  /* V_dc fuori finestra plausibile (a SDC chiuso) */
    PLAUS_FUSE_I2T          = (1U << 10), /* budget I^2t del fusibile di pacco esaurito */
} PlausibilityFault_t;

#define PLAUS_CHECK_COUNT   11U

typedef struct {
    uint16_t fail_cnt[PLAUS_CHECK_COUNT];  /* contatore debounce per check */
    uint32_t active;                       /* bitmask guasti scattati (debounced) */
    bool     apps_brake_latch;             /* stato latch EV.4.7 */
    float    fuse_i2t;                     /* accumulatore I^2t fusibile [A^2 s] */
} PlausibilityMonitor_t;

void Plausibility_Init(PlausibilityMonitor_t *m);

/**
 * @brief Esegue tutti i check e aggiorna il debounce.
 * @param v            snapshot dati veicolo
 * @param torque_active true se il veicolo sta erogando coppia (alcuni check
 *                      hanno senso solo in quel caso)
 * @return bitmask (PlausibilityFault_t) dei guasti attualmente attivi. 0 = tutto ok.
 */
uint32_t Plausibility_Update(PlausibilityMonitor_t *m, const VehicleData_t *v, bool torque_active);

/* Reset completo (contatori + latch): da chiamare quando la FSM rientra in
 * READY da uno stato di errore con condizioni pulite. */
void Plausibility_Reset(PlausibilityMonitor_t *m);

/* Nome del primo bit settato (per log). "OK" se f == 0. */
const char *Plausibility_FaultName(uint32_t faults);

#endif /* PLAUSIBILITY_H */
