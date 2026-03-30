/**
 * @file Inverter.c
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief AMK inverter management module implementation
 */

#include "Inverter.h"
#include <string.h>
#include "stm32f7xx_hal.h"
#include "Can.h"
#include "Config.h"
#include "BaseControlMotor.h"

Inverter_t g_inverter_left = {0};
Inverter_t g_inverter_right = {0};

void Inverters_Init(){
    /* Initialize/register only physically present inverter nodes. */
#if INVERTER_RIGHT_PRESENT
    Inverter_Init(&g_inverter_right, INVERTER_RIGHT_NODE_ID);
    CAN_Inverter_Register(&g_inverter_right);
#else
    memset(&g_inverter_right, 0, sizeof(g_inverter_right));
    g_inverter_right.node_id = INVERTER_RIGHT_NODE_ID;
    g_inverter_right.state = INV_STATE_OFF;
    g_inverter_right.previous_state = INV_STATE_OFF;
#endif

#if INVERTER_LEFT_PRESENT
    Inverter_Init(&g_inverter_left, INVERTER_LEFT_NODE_ID);
    CAN_Inverter_Register(&g_inverter_left);
#else
    memset(&g_inverter_left, 0, sizeof(g_inverter_left));
    g_inverter_left.node_id = INVERTER_LEFT_NODE_ID;
    g_inverter_left.state = INV_STATE_OFF;
    g_inverter_left.previous_state = INV_STATE_OFF;
#endif
}



/**
 * @brief Unpack 16-bit value from 2 bytes (Little Endian)
 */
static inline uint16_t unpack_u16(uint8_t lsb, uint8_t msb) {
    return (uint16_t)lsb | ((uint16_t)msb << 8);
}

/**
 * @brief Unpack 32-bit value from 4 bytes (Little Endian)
 */
static inline uint32_t unpack_u32(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3) {
    return (uint32_t)b0 | ((uint32_t)b1 << 8) | ((uint32_t)b2 << 16) | ((uint32_t)b3 << 24);
}

/**
 * @brief Check bit in status word
 */
static inline bool check_status_bit(uint16_t status_word, uint16_t bit_mask) {
    return (status_word & bit_mask) != 0;
}

/**
 * @brief Decode Status Message 1 directly into inverter
 */
static void decode_sm1(Inverter_t *inv, const uint8_t data[8]) {
    inv->status_word = unpack_u16(data[0], data[1]);
    inv->speed_rpm = (int16_t)unpack_u16(data[2], data[3]);
    inv->torque_value = (int16_t)(unpack_u16(data[4], data[5]));
    inv->raw_torque_current = (int16_t)unpack_u16(data[6], data[7]);
}

/**
 * @brief Decode Status Message 2 directly into inverter
 */
static void decode_sm2(Inverter_t *inv, const uint8_t data[8]) {
    inv->motor_temp_degC = (int16_t)unpack_u16(data[0], data[1]);
    inv->inverter_temp_degC = (int16_t)unpack_u16(data[2], data[3]);
    inv->error_code = unpack_u16(data[4], data[5]);
    inv->igbt_temp_degC = (int16_t)unpack_u16(data[6], data[7]);
}

/**
 * @brief Decode Status Message 3 directly into inverter
 */
static void decode_sm3(Inverter_t *inv, const uint8_t data[8]) {
    inv->dc_bus_voltage = unpack_u16(data[0], data[1]);
    inv->raw_magnetizing_current = unpack_u16(data[2], data[3]);
    inv->phase_u_current = (int32_t)unpack_u32(data[4], data[5], data[6], data[7]);
}

/**
 * @brief Decode Status Message 4 directly into inverter
 */
static void decode_sm4(Inverter_t *inv, const uint8_t data[8]) {
    inv->phase_v_current = (int32_t)unpack_u32(data[0], data[1], data[2], data[3]);
    inv->phase_w_current = (int32_t)unpack_u32(data[4], data[5], data[6], data[7]);
}

/**
 * @brief Decode Status Message 5 directly into inverter
 * SM5: CAN ID 0x290 + node_id
 * Bytes 0-3: Error Info 1 (U32)
 * Bytes 4-7: Actual Power (U32) [W]
 */
static void decode_sm5(Inverter_t *inv, const uint8_t data[8]) {
    inv->error_info_1 = unpack_u32(data[0], data[1], data[2], data[3]);
    inv->actual_power = unpack_u32(data[4], data[5], data[6], data[7]);
}

/**
 * @brief Determine state based on status word
 * 
 * State transition logic according to AMK datasheet:
 * - IDLE: No active bit (high byte = 0)
 * - LV_ACTIVE: SystemReady active
 * - HV_ACTIVE: DcOn active
 * - READY: QuitDcOn active (DC bus charged confirmation)
 * - RUNNING: QuitInverterOn active (controller active)
 * - ERROR: Error active (highest priority)
 */
static InverterState_t determine_state(uint16_t status_word) {
    // Priority 1: Error
    if (check_status_bit(status_word, INV_STAT_ERROR)) {
        return INV_STATE_ERROR;
    }
    
    // Priority 2: Running (controller active)
    if (check_status_bit(status_word, INV_STAT_QUIT_INV_ON)) {
        return INV_STATE_RUNNING;
    }
    
    // Priority 3: Ready (DC bus confirmed)
    if (check_status_bit(status_word, INV_STAT_QUIT_DC_ON)) {
        return INV_STATE_READY;
    }
    
    // Priority 4: HV active (DC bus charging)
    if (check_status_bit(status_word, INV_STAT_DC_ON)) {
        return INV_STATE_HV_ACTIVE;
    }
    
    // Priority 5: LV active (system ready but HV off)
    if (check_status_bit(status_word, INV_STAT_SYSTEM_READY)) {
        return INV_STATE_LV_ACTIVE;
    }
    
    // Default: IDLE
    return INV_STATE_IDLE;
}

void Inverter_Init(Inverter_t *inv, uint8_t node_id) {
    if (inv == NULL) return;
    
    memset(inv, 0, sizeof(Inverter_t));
    inv->node_id = node_id;
    inv->state = INV_STATE_OFF;
    inv->previous_state = INV_STATE_OFF;
    
    // formula: BASE + node_id 
    inv->tx_can_id = 0x0183U + node_id;  // CAN_ID_BASE_ADDRESS_TX1 + node_id
    inv->rx1_can_id = 0x0282U + node_id; // CAN_ID_BASE_ADDRESS_RX1 + node_id
    inv->rx2_can_id = 0x0284U + node_id; // CAN_ID_BASE_ADDRESS_RX2 + node_id
    inv->rx3_can_id = 0x0286U + node_id; // CAN_ID_BASE_ADDRESS_RX3 + node_id
    inv->rx4_can_id = 0x0288U + node_id; // CAN_ID_BASE_ADDRESS_RX4 + node_id
    inv->rx5_can_id = 0x0290U + node_id; // CAN_ID_BASE_ADDRESS_RX5 + node_id
}

void Inverter_UpdateState(Inverter_t *inv) {
    if (inv == NULL) return;
    
    inv->previous_state = inv->state;
    inv->last_rx_timestamp_ms = HAL_GetTick();
    
    inv->state = determine_state(inv->status_word);
}

void Inverter_UpdateStatusMessage(Inverter_t *inv, uint8_t msg_id, const uint8_t *data) {
    if (inv == NULL || data == NULL) return;
    
    switch (msg_id) {
        case 1:  
            decode_sm1(inv, data);
            break;
            
        case 2:  
            decode_sm2(inv, data);
            break;
            
        case 3: 
            decode_sm3(inv, data);
            break;
            
        case 4: 
            decode_sm4(inv, data);
            break;
            
        case 5:
            decode_sm5(inv, data);
            break;
            
        default:
            break;
    }
}

InverterCommandMsg1_t Inverter_BuildCommand(const Inverter_t *inv, 
                                             int16_t torque_setpoint,
                                             int16_t torque_limit_pos,
                                             int16_t torque_limit_neg) {
    InverterCommandMsg1_t cmd = {0};
    
    if (inv == NULL) return cmd;
    
    switch (inv->state) {
        case INV_STATE_IDLE:
            cmd.control_word = 0;
            cmd.torque_setpoint = 0;
            cmd.torque_limit_pos = 0;
            cmd.torque_limit_neg = 0;
            break;
            
        case INV_STATE_LV_ACTIVE:
            cmd.control_word = INV_CTRL_DC_ON;
            cmd.torque_setpoint = 0;
            cmd.torque_limit_pos = 0;
            cmd.torque_limit_neg = 0;
            break;
            
        case INV_STATE_HV_ACTIVE:
            cmd.control_word = INV_CTRL_DC_ON;
            cmd.torque_setpoint = 0;
            cmd.torque_limit_pos = 0;
            cmd.torque_limit_neg = 0;
            break;
            
        case INV_STATE_READY:
            cmd.control_word = INV_CTRL_DC_ON | INV_CTRL_ENABLE | INV_CTRL_INVERTER_ON;
            cmd.torque_setpoint = 0;
            cmd.torque_limit_pos = 0;
            cmd.torque_limit_neg = 0;
            break;
            
        case INV_STATE_RUNNING:
            cmd.control_word = INV_CTRL_DC_ON | INV_CTRL_ENABLE | INV_CTRL_INVERTER_ON;
            cmd.torque_setpoint = torque_setpoint;
            cmd.torque_limit_pos = torque_limit_pos;
            cmd.torque_limit_neg = torque_limit_neg;
            break;
            
        case INV_STATE_OFF:
            /* No communication — send nothing (zero frame) */
            cmd.control_word = 0;
            cmd.torque_setpoint = 0;
            cmd.torque_limit_pos = 0;
            cmd.torque_limit_neg = 0;
            break;

        case INV_STATE_ERROR:
            cmd.control_word = INV_CTRL_ERROR_RESET;
            cmd.torque_setpoint = 0;
            cmd.torque_limit_pos = 0;
            cmd.torque_limit_neg = 0;
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

bool Inverter_HasWarning(const Inverter_t *inv) {
    if (inv == NULL) return false;
    return check_status_bit(inv->status_word, INV_STAT_WARN);
}

bool Inverter_CheckTimeout(Inverter_t *inv, uint32_t current_time_ms, uint32_t timeout_ms) {
    if (inv == NULL) return true;
    
    uint32_t elapsed = current_time_ms - inv->last_rx_timestamp_ms;
    return (elapsed > timeout_ms);
}

const char* Inverter_GetStateName(InverterState_t state) {
    switch (state) {
        case INV_STATE_OFF:          return "OFF";
        case INV_STATE_IDLE:         return "IDLE";
        case INV_STATE_LV_ACTIVE:    return "LV_ACTIVE";
        case INV_STATE_HV_ACTIVE:    return "HV_ACTIVE";
        case INV_STATE_READY:        return "READY";
        case INV_STATE_RUNNING:      return "RUNNING";
        case INV_STATE_ERROR:        return "ERROR";
        default:                     return "UNKNOWN";
    }
}

void Inverter_UpdateOffTimeout(Inverter_t *inv)
{
    if (inv == NULL) return;
    if (inv->state == INV_STATE_OFF) return; /* already off, nothing to do */

    if ((HAL_GetTick() - inv->last_rx_timestamp_ms) >= INV_TIMEOUT_OFF_MS)
    {
        inv->previous_state = inv->state;
        inv->state          = INV_STATE_OFF;
    }
}





