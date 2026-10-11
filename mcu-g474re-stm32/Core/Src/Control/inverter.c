#include "Control/inverter.h"
#include "Connectivity/Can.h"
#include <string.h>

Inverter_t g_inverter_left  = {0};
Inverter_t g_inverter_right = {0};

void Inverters_Init(void) {
    Inverter_Init(&g_inverter_right, INVERTER_RIGHT_NODE_ID);
    Inverter_Init(&g_inverter_left,  INVERTER_LEFT_NODE_ID);

    /* Senza questa registrazione, process_motor_rx_message() non trova mai l'inverter e scarta tutto */
    CAN_MOTOR_Register(&g_inverter_right);
    CAN_MOTOR_Register(&g_inverter_left);
}

void Inverter_Init(Inverter_t *inv, uint8_t node_id) {
    if (inv == NULL) return;
    
    memset(inv, 0, sizeof(Inverter_t));
    inv->node_id             = node_id;
    inv->state               = INV_STATE_OFF;
    inv->previous_state      = INV_STATE_OFF;
    inv->state_entry_time_ms = HAL_GetTick();
    
    /* Mapping ID CAN AMK: Base + node_id */
    inv->tx_can_id  = 0x0183U + node_id;  /* TX Command */
    inv->rx1_can_id = 0x0282U + node_id;  /* SM1: Status & Speed */
    inv->rx2_can_id = 0x0284U + node_id;  /* SM2: Temp & Errors */
    inv->rx3_can_id = 0x0286U + node_id;  /* SM3: DC Voltage & Curr */
    inv->rx4_can_id = 0x0288U + node_id;  /* SM4: Phase Currents */
    inv->rx5_can_id = 0x0290U + node_id;  /* SM5: Power & ErrorInfo */
}

static inline uint16_t unpack_u16(uint8_t lsb, uint8_t msb) {
    return (uint16_t)lsb | ((uint16_t)msb << 8);
}

static inline uint32_t unpack_u32(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3) {
    return (uint32_t)b0 | ((uint32_t)b1 << 8) | ((uint32_t)b2 << 16) | ((uint32_t)b3 << 24);
}

static inline bool check_status_bit(uint16_t status_word, uint16_t bit_mask) {
    return (status_word & bit_mask) != 0;
}



static void decode_sm1(Inverter_t *inv, const uint8_t data[8]) {
    inv->status_word       = unpack_u16(data[0], data[1]);
    inv->speed_rpm         = (int16_t)unpack_u16(data[2], data[3]);
    /* Il drive manda la coppia in 0.1 % di Mn: Nm = raw * 0.001 * Mn */
    inv->actual_torque_nm  = (float)((int16_t)unpack_u16(data[4], data[5])) * 0.001f * MOTOR_NOMINAL_TORQUE_NM;
    inv->dc_bus_voltage    = unpack_u16(data[6], data[7]);
}

static void decode_sm2(Inverter_t *inv, const uint8_t data[8]) {
    inv->raw_torque_current_iq = (int16_t)unpack_u16(data[0], data[1]);
    inv->raw_magn_current_id   = (int16_t)unpack_u16(data[2], data[3]);
    inv->actual_power_w        = unpack_u32(data[4], data[5], data[6], data[7]);
}

static void decode_sm3(Inverter_t *inv, const uint8_t data[8]) {
    inv->motor_temp_degc    = (int16_t)unpack_u16(data[0], data[1]);
    inv->inverter_temp_degc = (int16_t)unpack_u16(data[2], data[3]);
    inv->igbt_temp_degc     = (int16_t)unpack_u16(data[4], data[5]);
    inv->error_code         = unpack_u16(data[6], data[7]);
}

static void decode_sm4(Inverter_t *inv, const uint8_t data[8]) {
    inv->phase_current_iu = (int32_t)unpack_u32(data[0], data[1], data[2], data[3]);
    inv->phase_current_iv = (int32_t)unpack_u32(data[4], data[5], data[6], data[7]);
}

static void decode_sm5(Inverter_t *inv, const uint8_t data[8]) {
    inv->phase_current_iw = (int32_t)unpack_u32(data[0], data[1], data[2], data[3]);
    inv->error_info_1     = unpack_u32(data[4], data[5], data[6], data[7]);
}

static InverterState_t determine_state(uint16_t status_word) {
    if (check_status_bit(status_word, INV_STAT_ERROR))       return INV_STATE_ERROR;
    if (check_status_bit(status_word, INV_STAT_QUIT_INV_ON)) return INV_STATE_RUNNING;
    if (check_status_bit(status_word, INV_STAT_QUIT_DC_ON))  return INV_STATE_READY;
    if (check_status_bit(status_word, INV_STAT_DC_ON))       return INV_STATE_HV_ACTIVE;
    if (check_status_bit(status_word, INV_STAT_SYSTEM_READY))return INV_STATE_LV_ACTIVE;
    return INV_STATE_IDLE;
}

void Inverter_UpdateState(Inverter_t *inv) {
    if (inv == NULL) return;
    
    inv->previous_state        = inv->state;
    inv->last_rx_timestamp_ms = HAL_GetTick();
    
    InverterState_t new_state = determine_state(inv->status_word);
    if (new_state != inv->state) {
        inv->state_entry_time_ms = HAL_GetTick();
    }
    inv->state = new_state;
}

void Inverter_UpdateStatusMessage(Inverter_t *inv, uint8_t msg_id, const uint8_t *data) {
    if (inv == NULL || data == NULL) return;
    
    switch (msg_id) {
        case 1: decode_sm1(inv, data); break;
        case 2: decode_sm2(inv, data); break;
        case 3: decode_sm3(inv, data); break;
        case 4: decode_sm4(inv, data); break;
        case 5: decode_sm5(inv, data); break;
        default: break;
    }
}

InverterCommandMsg1_t Inverter_BuildCommand(const Inverter_t *inv, int16_t t_setpoint, int16_t lim_pos, int16_t lim_neg) {
    InverterCommandMsg1_t cmd = {0};
    if (inv == NULL) return cmd;
    
    switch (inv->state) {
        case INV_STATE_LV_ACTIVE:
        case INV_STATE_HV_ACTIVE:
            cmd.control_word = INV_CTRL_DC_ON;
            break;
            
        case INV_STATE_READY:
            cmd.control_word = INV_CTRL_DC_ON | INV_CTRL_ENABLE | INV_CTRL_INVERTER_ON;
            break;
            
        case INV_STATE_RUNNING:
            cmd.control_word     = INV_CTRL_DC_ON | INV_CTRL_ENABLE | INV_CTRL_INVERTER_ON;
            cmd.torque_setpoint  = t_setpoint;
            cmd.torque_limit_pos = lim_pos;
            cmd.torque_limit_neg = lim_neg;
            break;
            
        default:
            cmd.control_word = 0;
            break;
    }
    return cmd;
}

bool Inverter_HasError(const Inverter_t *inv) {
    if (inv == NULL) return false;
    return check_status_bit(inv->status_word, INV_STAT_ERROR);
}

const char* Inverter_GetStateName(InverterState_t state) {
    switch (state) {
        case INV_STATE_OFF:       return "OFF";
        case INV_STATE_IDLE:      return "IDLE";
        case INV_STATE_LV_ACTIVE: return "LV_ACTIVE";
        case INV_STATE_HV_ACTIVE: return "HV_ACTIVE";
        case INV_STATE_READY:     return "READY";
        case INV_STATE_RUNNING:   return "RUNNING";
        case INV_STATE_ERROR:     return "ERROR";
        default:                  return "UNKNOWN";
    }
}