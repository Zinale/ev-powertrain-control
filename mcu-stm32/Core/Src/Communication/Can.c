/**
 * @file Can.c
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief CAN implementation for motor inverter (CAN1)
 * 
 * Handles CAN1 communication dedicated to AMK motor inverters.
 * Scalable: passing hcan as parameter allows future CAN2 integration.
 */

#include <Communication/Can.h>
#include "mutexes.h"
#include <string.h>
#include "Communication/Serial.h"

extern CAN_HandleTypeDef hcan1;

#define N_INVERTERS  2      // if more than two inverters, change CAN ID logic
static Inverter_t *s_inverter[N_INVERTERS] = {NULL, NULL};

static CarData_t car_data = {0};



static void configure_can_filters(CAN_HandleTypeDef *hcan);
static bool transmit_can_message(CAN_HandleTypeDef *hcan, uint32_t can_id, const uint8_t *data, uint8_t dlc);
static Inverter_t* find_inverter_by_rx_id(uint32_t can_id, uint8_t *msg_id_out);
static void process_rx_message(uint32_t can_id, const uint8_t *data, uint8_t dlc);
static inline void pack_u16(uint8_t *dest, uint16_t value);
static inline void pack_i16(uint8_t *dest, int16_t value);



/**
 * @brief Pack unsigned 16-bit value in 2 byte (Little Endian)
 */
static inline void pack_u16(uint8_t *dest, uint16_t value) {
    dest[0] = (uint8_t)(value & 0xFF);
    dest[1] = (uint8_t)(value >> 8);
}

/**
 * @brief Pack signed 16-bit value in 2 byte (Little Endian)
 */
static inline void pack_i16(uint8_t *dest, int16_t value) {
    pack_u16(dest, (uint16_t)value);
}




void CAN_Inverter_Init(CAN_HandleTypeDef *hcan) {
    if (hcan == NULL) return;

    configure_can_filters(hcan);
    HAL_CAN_Start(hcan);
    HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
}

void CAN_Car_Init(CAN_HandleTypeDef *hcan) {
    if (hcan == NULL) return;

    configure_can_filters(hcan);
    HAL_CAN_Start(hcan);
    HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO1_MSG_PENDING);
}

/**
 * @brief Register an inverter in the CAN module for message handling.
 *
 * Saves the pointer to the inverter in a static array indexed by node_id:
 *   - node_id = 1 -> s_inverter[0] (right inverter)
 *   - node_id = 2 -> s_inverter[1] (left inverter)
 *
 * After registration, the CAN module can directly update the inverter state
 * when it receives messages on the CAN bus.
 *
 * @param inv Pointer to Inverter_t structure to register (NULL ignored)
 */
void CAN_Inverter_Register(Inverter_t *inv) {
    if (inv == NULL) return;

    uint8_t idx = inv->node_id - 1;
    if (idx < 2) {
        s_inverter[idx] = inv;
    }
}


/**
 * @brief Configure CAN filters
 *
 * CAN1: pass-all mask on FIFO0 (inverter IDs filtered by software).
 * CAN2: 16-bit ID list on FIFO1, only accepts 0x100-0x106 (IMU/BMS/ECU/DASH).
 *       FilterBank 14: 0x100, 0x101, 0x102, 0x103
 *       FilterBank 15: 0x104, 0x105, 0x106  (slot 4 repeats 0x106)
 */
static void configure_can_filters(CAN_HandleTypeDef *hcan) {
    CAN_FilterTypeDef filter_config = {0};

    if (hcan == NULL) return;

    if (hcan->Instance == CAN1) {
        filter_config.FilterBank           = 0;
        filter_config.FilterMode           = CAN_FILTERMODE_IDMASK;
        filter_config.FilterScale          = CAN_FILTERSCALE_32BIT;
        filter_config.FilterIdHigh         = 0x0000;
        filter_config.FilterIdLow          = 0x0000;
        filter_config.FilterMaskIdHigh     = 0x0000;  // accept all
        filter_config.FilterMaskIdLow      = 0x0000;
        filter_config.FilterFIFOAssignment = CAN_RX_FIFO0;
        filter_config.FilterActivation     = ENABLE;
        filter_config.SlaveStartFilterBank = 14;      // bank 0-13 -> CAN1, 14-27 -> CAN2
        HAL_CAN_ConfigFilter(hcan, &filter_config);
    }
    else if (hcan->Instance == CAN2) {
        /*
         * 16-bit ID list mode: each bank holds 4 standard IDs.
         * Standard ID must be shifted left by 5 in the filter register.
         *   FilterIdHigh   = ID slot 1
         *   FilterIdLow    = ID slot 2
         *   FilterMaskIdHigh = ID slot 3
         *   FilterMaskIdLow  = ID slot 4
         */

        /* Bank 14: IMU accel (0x100), IMU gyro (0x101), IMU mag (0x102), BMS (0x103) */
        filter_config.FilterBank           = 14;
        filter_config.FilterMode           = CAN_FILTERMODE_IDLIST;
        filter_config.FilterScale          = CAN_FILTERSCALE_16BIT;
        filter_config.FilterIdHigh         = (CAN_ID_IMU_ACC  << 5);
        filter_config.FilterIdLow          = (CAN_ID_IMU_GYRO << 5);
        filter_config.FilterMaskIdHigh     = (CAN_ID_IMU_MAG  << 5);
        filter_config.FilterMaskIdLow      = (CAN_ID_BMS      << 5);
        filter_config.FilterFIFOAssignment = CAN_RX_FIFO1;
        filter_config.FilterActivation     = ENABLE;
        filter_config.SlaveStartFilterBank = 14;
        HAL_CAN_ConfigFilter(hcan, &filter_config);

        /* Bank 15: ECU RPM (0x104), ECU Brake (0x105), DASH (0x106), DASH (repeated) */
        filter_config.FilterBank           = 15;
        filter_config.FilterIdHigh         = (CAN_ID_ECU_RPM   << 5);
        filter_config.FilterIdLow          = (CAN_ID_ECU_BRAKE << 5);
        filter_config.FilterMaskIdHigh     = (CAN_ID_DASH_R2D      << 5);
        filter_config.FilterMaskIdLow      = (CAN_ID_DASH_R2D      << 5);
        HAL_CAN_ConfigFilter(hcan, &filter_config);
    }
}


void CAN_Car_ProcessRxMessages(CAN_HandleTypeDef *hcan) {
    if (hcan == NULL) return;

    CAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];

    while (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &rx_header, rx_data) == HAL_OK) {
        switch (rx_header.StdId) {
            case CAN_ID_IMU_ACC:
                // Big Endian: byte[n]=MSB, byte[n+1]=LSB
                car_data.imu_data.accel_x    = (int16_t)(((uint16_t)rx_data[0] << 8 | rx_data[1])* IMU_ACCEL_SCALE);
                car_data.imu_data.accel_y    = (int16_t)(((uint16_t)rx_data[2] << 8 | rx_data[3] )* IMU_ACCEL_SCALE);
                car_data.imu_data.accel_z    = (int16_t)(((uint16_t)rx_data[4] << 8 | rx_data[5])* IMU_ACCEL_SCALE);
                car_data.imu_data.serial_num = (uint16_t)((uint16_t)rx_data[6] << 8 | rx_data[7]);
                break;
            case CAN_ID_IMU_GYRO:
                car_data.imu_data.gyro_x        = (int16_t)(((uint16_t)rx_data[0] << 8 | rx_data[1]) * IMU_GYRO_SCALE);
                car_data.imu_data.gyro_y        = (int16_t)(((uint16_t)rx_data[2] << 8 | rx_data[3]) * IMU_GYRO_SCALE);
                car_data.imu_data.gyro_z        = (int16_t)(((uint16_t)rx_data[4] << 8 | rx_data[5]) * IMU_GYRO_SCALE);
                car_data.imu_data.internal_temp = (int16_t)((uint16_t)rx_data[6] << 8 | rx_data[7]);
                break;
            case CAN_ID_IMU_MAG:
                car_data.imu_data.mag_x = (int16_t)(((uint16_t)rx_data[0] << 8 | rx_data[1]) * IMU_MAG_SCALE);
                car_data.imu_data.mag_y = (int16_t)(((uint16_t)rx_data[2] << 8 | rx_data[3]) * IMU_MAG_SCALE);
                car_data.imu_data.mag_z = (int16_t)(((uint16_t)rx_data[4] << 8 | rx_data[5]) * IMU_MAG_SCALE);
                break;
            case CAN_ID_BMS:
                car_data.bms_data.soc           = (int8_t)rx_data[0];
                car_data.bms_data.soh           = (int8_t)rx_data[1];
                car_data.bms_data.max_discharge = (uint16_t)((uint16_t)rx_data[2] << 8 | rx_data[3]);
                car_data.bms_data.max_regen     = (uint16_t)((uint16_t)rx_data[4] << 8 | rx_data[5]);
                break;
            case CAN_ID_ECU_RPM:
                car_data.fwheels_rpm_data.rpm_fr = (uint16_t)((uint16_t)rx_data[0] << 8 | rx_data[1]);
                car_data.fwheels_rpm_data.rpm_fl = (uint16_t)((uint16_t)rx_data[2] << 8 | rx_data[3]);
                break;
            case CAN_ID_ECU_BRAKE:
                car_data.brake_pressure = rx_data[0];
                break;
            case CAN_ID_DASH_R2D:
                car_data.engine_map = (int8_t)rx_data[1];
                break;
            default:
                break;
        }
    }
}

const CarData_t* CAN_Car_GetData(void) {
    return &car_data;
}


// Executed by CAN interrupt for each message received on CAN Inverter-Motors
void CAN_Inverter_ProcessRxMessages(CAN_HandleTypeDef *hcan) {
    if (hcan == NULL) return;
    
    CAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];
    
    // Read all available messages in FIFO0
    /*
    while (HAL_CAN_GetRxFifoFillLevel(hcan, CAN_RX_FIFO0) > 0) {
        if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK) {
            process_rx_message(rx_header.StdId, rx_data, rx_header.DLC);
        } else {
            break;
        }
    }
    */
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK) {
    	process_rx_message(rx_header.StdId, rx_data, rx_header.DLC);
    }
}


/**
 * @brief Process a single received CAN message
 */
static void process_rx_message(uint32_t can_id, const uint8_t *data, uint8_t dlc) {
    uint8_t msg_id;     // which Msg arrived (1-5)
    Inverter_t *inv = find_inverter_by_rx_id(can_id, &msg_id);

    if (inv == NULL) return;

    /* Increment RX counter for diagnostics */
    extern uint32_t g_can_rx_count;
    g_can_rx_count++;

    /* Lock the appropriate inverter mutex before updating shared state */
    if (inv->node_id == INVERTER_RIGHT_NODE_ID)
        Mutex_INVERTER_R_Lock();
    else if (inv->node_id == INVERTER_LEFT_NODE_ID)
        Mutex_INVERTER_L_Lock();

    Inverter_UpdateStatusMessage(inv, msg_id, data);

    if (msg_id == 1) {
        Inverter_UpdateState(inv);
    }

    /* Unlock inverter mutex */
    if (inv->node_id == INVERTER_RIGHT_NODE_ID)
        Mutex_INVERTER_R_Unlock();
    else if (inv->node_id == INVERTER_LEFT_NODE_ID)
        Mutex_INVERTER_L_Unlock();
}



/**
 * @brief Find inverter by CAN ID and determine msg_id using stored IDs
 */
static Inverter_t* find_inverter_by_rx_id(uint32_t can_id, uint8_t *msg_id_out) {
    for (uint8_t i = 0; i < N_INVERTERS; i++) {
        if (s_inverter[i] == NULL) continue;
        
        if (can_id == s_inverter[i]->rx1_can_id) {
            *msg_id_out = 1;
            return s_inverter[i];
        }
        if (can_id == s_inverter[i]->rx2_can_id) {
            *msg_id_out = 2;
            return s_inverter[i];
        }
        if (can_id == s_inverter[i]->rx3_can_id) {
            *msg_id_out = 3;
            return s_inverter[i];
        }
        if (can_id == s_inverter[i]->rx4_can_id) {
            *msg_id_out = 4;
            return s_inverter[i];
        }
        if (can_id == s_inverter[i]->rx5_can_id) {
            *msg_id_out = 5;
            return s_inverter[i];
        }
    }
    
    return NULL;
}


/**
 * @brief Transmit a generic CAN message
 */
static bool transmit_can_message(CAN_HandleTypeDef *hcan, uint32_t can_id, const uint8_t *data, uint8_t dlc) {
    CAN_TxHeaderTypeDef tx_header = {0};
    uint32_t mailbox;
    
    tx_header.StdId = can_id;
    tx_header.IDE = CAN_ID_STD;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.DLC = dlc;
    
    HAL_StatusTypeDef status = HAL_CAN_AddTxMessage(hcan, &tx_header, (uint8_t*)data, &mailbox);
    return (status == HAL_OK);
}


bool CAN_Inverter_TransmitCommand(CAN_HandleTypeDef *hcan, const Inverter_t *inv, const InverterCommandMsg1_t *cmd) {
    if (hcan == NULL || inv == NULL || cmd == NULL) return false;

    static uint32_t s_last_tx_tick[N_INVERTERS] = {0};
    uint8_t idx = (inv->node_id >= 1U && inv->node_id <= N_INVERTERS) ? (inv->node_id - 1U) : 0U;

    uint8_t tx_data[8];
    pack_u16(&tx_data[0], cmd->control_word);
    pack_i16(&tx_data[2], cmd->torque_setpoint);
    pack_i16(&tx_data[4], cmd->torque_limit_pos);
    pack_i16(&tx_data[6], cmd->torque_limit_neg);

    bool ok = transmit_can_message(hcan, inv->tx_can_id, tx_data, 8);

    uint32_t now   = HAL_GetTick();
    uint32_t delta = (s_last_tx_tick[idx] > 0U) ? (now - s_last_tx_tick[idx]) : 0U;
    s_last_tx_tick[idx] = now;

    Serial_Log(LOG_CH_UART3,
        "[CAN1 TX] id=0x%03lX cw=0x%04X trq=%d lim+=%d lim-=%d t=%lu dt=%lu ms ok=%d\r\n",
        (unsigned long)inv->tx_can_id,
        cmd->control_word,
        cmd->torque_setpoint,
        cmd->torque_limit_pos,
        cmd->torque_limit_neg,
        (unsigned long)now,
        (unsigned long)delta,
        (int)ok);

    return ok;
}

/**
 * @brief 0x002 / 0x003 — inverter error packet.
 *
 * Byte 0-1 : INV_ERROR_NUM   = error_code   [U16 LE]
 * Byte 2-5 : INV_ERROR_INFO1 = error_info_1 [S32 LE]
 */
void CAN_Car_TransmitInverterError(CAN_HandleTypeDef *hcan, uint8_t node_id, const Inverter_t *inv)
{
    if (hcan == NULL || inv == NULL) return;

    uint32_t can_id = (node_id == 1U) ? CAN_ID_TX_INV1_ERROR : CAN_ID_TX_INV2_ERROR;

    uint8_t tx_data[6];
    /* Byte 0-1: error_code (U16 LE) */
    pack_u16(&tx_data[0], inv->error_code);
    /* Byte 2-5: error_info_1 (S32 LE) */
    tx_data[2] = (uint8_t)( inv->error_info_1        & 0xFFU);
    tx_data[3] = (uint8_t)((inv->error_info_1 >>  8) & 0xFFU);
    tx_data[4] = (uint8_t)((inv->error_info_1 >> 16) & 0xFFU);
    tx_data[5] = (uint8_t)((inv->error_info_1 >> 24) & 0xFFU);

    transmit_can_message(hcan, can_id, tx_data, 6U);
}

/**
 * @brief 0x006 — MCU general errors byte, DLC=1.
 *
 * Byte 0 (4 bits): bit0=INV1 Derating, bit1=INV2 Derating,
 *                  bit2=APPs Implausibility, bit3=Regen Active.
 */
void CAN_Car_TransmitErrors(CAN_HandleTypeDef *hcan, uint8_t errors_byte)
{
    if (hcan == NULL) return;
    transmit_can_message(hcan, CAN_ID_TX_MCU_ERRORS, &errors_byte, 1U);
}

/**
 * @brief 0x108 — APPs and SAS packet, DLC=2.
 *
 * Byte 0 : APPS_VALUE  [U8, 0-100 %]
 * Byte 1 : SAS_VALUE   [S8, -100..+100 %]
 */
void CAN_Car_TransmitAppsSas(CAN_HandleTypeDef *hcan, uint8_t apps_pct, int8_t sas_pct)
{
    if (hcan == NULL) return;

    uint8_t tx_data[2];
    tx_data[0] = apps_pct;
    tx_data[1] = (uint8_t)sas_pct;

    transmit_can_message(hcan, CAN_ID_TX_APPs_SAS, tx_data, 2U);
}

/**
 * @brief 0x250 / 0x251 — Inverter real-time data, DLC=8, Little-Endian.
 *
 * Byte 0-1 : INVx_STAT_WORD   = status_word        [U16 LE]
 * Byte 2-3 : INVx_ACT_SPEED   = speed_rpm          [S16 LE, rpm]
 * Byte 4-5 : INVx_MOTOR_TEMP  = motor_temp_degC    [S16 LE, 0.1°C]
 * Byte 6-7 : INVx_TEMP        = inverter_temp_degC [S16 LE, 0.1°C]
 */
void CAN_Car_TransmitInverterData(CAN_HandleTypeDef *hcan, uint8_t node_id, const Inverter_t *inv)
{
    if (hcan == NULL || inv == NULL) return;

    uint32_t can_id = (node_id == 1U) ? CAN_ID_TX_INV1_DATA : CAN_ID_TX_INV2_DATA;

    uint8_t tx_data[8];
    pack_u16(&tx_data[0], inv->status_word);
    pack_i16(&tx_data[2], inv->speed_rpm);
    pack_i16(&tx_data[4], inv->motor_temp_degC);
    pack_i16(&tx_data[6], inv->inverter_temp_degC);

    transmit_can_message(hcan, can_id, tx_data, 8U);
}
