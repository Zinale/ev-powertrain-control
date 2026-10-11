#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include "main.h"
#include "Control/inverter.h"
#include <stdint.h>
#include <stdbool.h>

/* Unità coppia CAN AMK: 1 = 0.1% Mn, 1000 = 100% Mn */
#define CAN1_SCALE_FACTOR   1000.0f
#define TORQUE_LIMIT_POS    2100     /* tetto assoluto trazione [0.1% Mn] */
#define TORQUE_LIMIT_NEG   (-2100)   /* tetto assoluto regen    [0.1% Mn] */

/* Diagnostica TX (definite in motor_control.c) */
extern volatile uint32_t g_can_tx_ok_count;
extern volatile uint32_t g_can_tx_fail_count;

/* Backstop opzionale: limite coppia dalla potenza disponibile (M = P*eta/omega),
 * saturato a MOTOR_MAX_TORQUE_NM. Ritorna unità CAN [0.1% Mn].
 * NON usato di default nel path comando: mappa/regen/TVC/slip sono gia'
 * applicati a monte da Simulink. Vedi nota in motor_control.c. */
int16_t Motor_CalculateDynamicTorqueLimit(int16_t speed_rpm, uint32_t power_w);

/**
 * @brief Gestione periodica di un inverter: sequenza accensione HV, gestione
 *        errori, invio comando su FDCAN1. Da chiamare a ogni ciclo di
 *        DriveLogicTask per entrambi gli inverter (watchdog control-word AMK).
 *
 * Il rate-limit della coppia e' gia' fatto dal modello Simulink: qui si invia
 * il setpoint cosi' com'e', con la sola saturazione ai limiti assoluti.
 *
 * @param torque_cmd_raw  setpoint coppia [0.1% Mn], segno incluso (+ trazione,
 *                        - rigenerativa), gia' shaped da DriveLogic
 * @param torque_allowed  true solo in VEHICLE_STATE_RUNNING
 * @param hv_enable       true = avvia/mantieni accensione HV (tipicamente R2D).
 *                        Se false la control word e' 0 ma il frame parte comunque.
 * @return setpoint coppia effettivamente inviato [0.1% Mn] (0 se non abilitato)
 */
int16_t Motor_ProcessInverterControl(FDCAN_HandleTypeDef *hfdcan,
                                     Inverter_t *inv,
                                     int16_t torque_cmd_raw,
                                     bool torque_allowed,
                                     bool hv_enable);

#endif /* MOTOR_CONTROL_H */
