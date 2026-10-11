#ifndef INVERTER_H
#define INVERTER_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* Costanti Motore AMK */
#define MOTOR_NOMINAL_TORQUE_NM     9.8f    /* Coppia nominale Mn [Nm] */
#define MOTOR_MAX_TORQUE_NM         21.0f   /* Coppia massima [Nm] */
#define MOTOR_MAX_SPEED_RPM         20000U  /* Regime massimo [RPM] (mot_max_rpm nello script Simulink) */
#define MOTOR_EFFICIENCY           0.98f    /* Rendimento stimato powertrain (per limite coppia dinamico) */

/* Control Word Bits (TX verso Inverter) */
#define INV_CTRL_INVERTER_ON        (1U << 8)   /* Abilitazione controller */
#define INV_CTRL_DC_ON              (1U << 9)   /* Chiusura contattori HV */
#define INV_CTRL_ENABLE             (1U << 10)  /* Abilitazione stadio di potenza (Driver) */
#define INV_CTRL_ERROR_RESET        (1U << 11)  /* Reset guasti (richiede fronte di discesa) */

/* Status Word Bits (RX da Inverter) */
#define INV_STAT_SYSTEM_READY       (1U << 8)
#define INV_STAT_ERROR              (1U << 9)
#define INV_STAT_WARN               (1U << 10)
#define INV_STAT_QUIT_DC_ON         (1U << 11)  /* Conferma DC bus in tensione */
#define INV_STAT_DC_ON              (1U << 12)
#define INV_STAT_QUIT_INV_ON        (1U << 13)  /* Conferma inverter operativo */
#define INV_STAT_INVERTER_ON        (1U << 14)
#define INV_STAT_DERATING           (1U << 15)

/* Canoni Node ID AMK */
#define INVERTER_RIGHT_NODE_ID      1U
#define INVERTER_LEFT_NODE_ID       2U

/* Timeout Comunicazione */
#define INV_TIMEOUT_OFF_MS          5000U
#define INV_TIMEOUT_HV_ACTIVE_MS    3000U

/* Stati Macchina a Stati Inverter */
typedef enum {
    INV_STATE_OFF = -1,
    INV_STATE_IDLE = 0,
    INV_STATE_LV_ACTIVE,
    INV_STATE_HV_ACTIVE,
    INV_STATE_READY,
    INV_STATE_RUNNING,
    INV_STATE_ERROR
} InverterState_t;

/* Comando TX verso Inverter (0.1% Mn) */
typedef struct {
    uint16_t control_word;
    int16_t  torque_setpoint;
    int16_t  torque_limit_pos;
    int16_t  torque_limit_neg;
} InverterCommandMsg1_t;

/* Struttura Dati Principale Inverter */
typedef struct {
    uint8_t         node_id;
    InverterState_t state;
    InverterState_t previous_state;
    uint32_t        state_entry_time_ms;
    uint32_t        last_rx_timestamp_ms;

    /* Telemetria decodificata dai messaggi CAN (SM1..SM5) */
    uint16_t status_word;
    int16_t  speed_rpm;
    float    actual_torque_nm;      /* Coppia misurata [Nm], segno: < 0 = rigenerazione */
    int16_t  motor_temp_degc;       /* [0.1 °C] valore grezzo del drive */
    int16_t  inverter_temp_degc;    /* [0.1 °C] */
    int16_t  igbt_temp_degc;        /* [0.1 °C] */
    uint16_t error_code;
    uint16_t dc_bus_voltage;
    uint32_t actual_power_w;
    uint32_t error_info_1;
    int16_t raw_torque_current_iq;
    int16_t raw_magn_current_id;

    int32_t phase_current_iu;
    int32_t phase_current_iv;
    int32_t phase_current_iw;

    /* CAN IDs associati al nodo */
    uint32_t tx_can_id;
    uint32_t rx1_can_id;
    uint32_t rx2_can_id;
    uint32_t rx3_can_id;
    uint32_t rx4_can_id;
    uint32_t rx5_can_id;
} Inverter_t;

extern Inverter_t g_inverter_left;
extern Inverter_t g_inverter_right;

void Inverters_Init(void);
void Inverter_Init(Inverter_t *inv, uint8_t node_id);
void Inverter_UpdateState(Inverter_t *inv);
void Inverter_UpdateStatusMessage(Inverter_t *inv, uint8_t msg_id, const uint8_t *data);
InverterCommandMsg1_t Inverter_BuildCommand(const Inverter_t *inv, int16_t t_setpoint, int16_t lim_pos, int16_t lim_neg);
bool Inverter_HasError(const Inverter_t *inv);
const char* Inverter_GetStateName(InverterState_t state);

#endif /* INVERTER_H */