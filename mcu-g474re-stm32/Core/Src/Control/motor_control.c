/**
 * @file motor_control.c
 * @brief Controllo periodico degli inverter AMK: sequenza di accensione HV,
 *        gestione errori e invio comando su FDCAN1.
 *
 * La coppia SX/DX arriva gia' calcolata e rate-limitata da DriveLogicTask
 * (modello Simulink). Qui si applica solo la saturazione ai limiti assoluti,
 * il "cancello di direzione" verso l'AMK e il gate di accensione HV.
 */

#include "Control/motor_control.h"
#include "Control/error_recovery.h"
#include "Connectivity/Can.h"

#include <math.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/* Diagnostica TX (lette dal DataLogger) */
volatile uint32_t g_can_tx_ok_count   = 0U;
volatile uint32_t g_can_tx_fail_count = 0U;

/* ============================================================================
 * Backstop opzionale - limite coppia da potenza disponibile.
 * Portato dal BaseControlMotor di banco. In modalita' SIMULINK il limite di
 * potenza vero e' gia' nel modello (I_discharge_max_p nel TorqueAllocator):
 * questa funzione resta come utility / per le modalita' di fallback, ma NON e'
 * agganciata al path comando qui sotto. Semantica di 'power_w' da confermare
 * sulla DBC AMK prima di usarla.
 * ============================================================================ */
int16_t Motor_CalculateDynamicTorqueLimit(int16_t speed_rpm, uint32_t power_w) {
    float speed = fabsf((float)speed_rpm);
    if (speed < 10.0f) {
        speed = 10.0f;
    }

    float omega_rad_s   = 2.0f * (float)M_PI * speed / 60.0f;
    float torque_lim_nm = ((float)power_w * MOTOR_EFFICIENCY) / omega_rad_s;

    if (torque_lim_nm > MOTOR_MAX_TORQUE_NM) {
        torque_lim_nm = MOTOR_MAX_TORQUE_NM;
    }
    return (int16_t)((torque_lim_nm / MOTOR_NOMINAL_TORQUE_NM) * CAN1_SCALE_FACTOR);
}

/* ============================================================================
 * Gestione periodica di un inverter
 * ============================================================================ */
int16_t Motor_ProcessInverterControl(FDCAN_HandleTypeDef *hfdcan,
                                     Inverter_t *inv,
                                     int16_t torque_cmd_raw,
                                     bool torque_allowed,
                                     bool hv_enable) {

    if (hfdcan == NULL || inv == NULL) {
        return 0;
    }

    InverterErrorRecovery_t *err_ctx = ErrorRecovery_GetCtx(inv);
    const InverterState_t state = inv->state;

    /* Inverter non ancora visto sul bus. */
    if (state == INV_STATE_OFF) {
        return 0;
    }

    /* Guasto: sequenza di reset AMK, nessun comando di coppia. */
    if (state == INV_STATE_ERROR) {
        if (err_ctx != NULL) {
            ErrorRecovery_Process(hfdcan, inv, err_ctx);
        }
        return 0;
    }
    if (err_ctx != NULL) {
        ErrorRecovery_Reset(err_ctx);
    }

    /* Setpoint: passa solo se veicolo RUNNING + HV abilitato + inverter RUNNING. */
    int16_t setpoint = 0;
    if (torque_allowed && hv_enable && (state == INV_STATE_RUNNING)) {
        setpoint = torque_cmd_raw;
        if (setpoint > TORQUE_LIMIT_POS) setpoint = TORQUE_LIMIT_POS;
        if (setpoint < TORQUE_LIMIT_NEG) setpoint = TORQUE_LIMIT_NEG;

        /* Sovravelocita': taglio di sicurezza indipendente dal modello. */
        if (abs((int)inv->speed_rpm) > (int)MOTOR_MAX_SPEED_RPM) {
            setpoint = 0;
        }
    }

    /* Cancello di direzione verso l'AMK: apre solo il verso richiesto.
     * 0/0 = coast puro (il regolatore di velocita' interno resta clampato). */
    int16_t lim_pos = (setpoint > 0) ? TORQUE_LIMIT_POS : 0;
    int16_t lim_neg = (setpoint < 0) ? TORQUE_LIMIT_NEG : 0;

    InverterCommandMsg1_t cmd = Inverter_BuildCommand(inv, setpoint, lim_pos, lim_neg);

    /* Gate accensione HV: senza hv_enable (R2D) la control word resta 0 e
     * l'inverter non supera LV_ACTIVE. Il frame viene comunque trasmesso. */
    if (!hv_enable) {
        cmd.control_word     = 0U;
        cmd.torque_setpoint  = 0;
        cmd.torque_limit_pos = 0;
        cmd.torque_limit_neg = 0;
        setpoint = 0;
    }

    bool tx_ok = CAN_MOTOR_TransmitCommand(hfdcan, inv, &cmd);
    if (!tx_ok) {
        tx_ok = CAN_MOTOR_TransmitCommand(hfdcan, inv, &cmd);   /* un retry */
    }
    if (tx_ok) {
        g_can_tx_ok_count++;
    } else {
        g_can_tx_fail_count++;
    }

    return setpoint;
}
