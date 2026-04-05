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
#include "Config.h"
#include "cmsis_os.h"

extern CAN_HandleTypeDef hcan1;

#define N_INVERTERS  2      // if more than two inverters, change CAN ID logic
static Inverter_t *s_inverter[N_INVERTERS] = {NULL, NULL};

static CarData_t car_data = {0};

/* =============================================================================
 *  CAN RX MESSAGE QUEUE (ISR-safe architecture)
 * =============================================================================
 *  Messages are enqueued by ISR (ISR-safe), dequeued by readings_manage task.
 *  This avoids calling FreeRTOS Mutexes inside the ISR.
 * ============================================================================= */

/**
 * @brief Structure to hold a single CAN RX message in the queue.
 */
typedef struct {
    uint32_t can_id;        /**< Standard CAN ID (11-bit) */
    uint8_t  data[8];       /**< CAN payload (8 bytes) */
    uint8_t  dlc;           /**< Data Length Code */
} CAN_RxMsg_t;

#define CAN_RX_QUEUE_SIZE  32   /**< Number of messages that can be queued */

static osMessageQueueId_t can_rx_queue = NULL;  /**< Queue handle */

/* Forward declarations - must be before CAN_ProcessQueueDrain which calls them */
static void configure_can_filters(CAN_HandleTypeDef *hcan);
static bool transmit_can_message(CAN_HandleTypeDef *hcan, uint32_t can_id, const uint8_t *data, uint8_t dlc);
static Inverter_t* find_inverter_by_rx_id(uint32_t can_id, uint8_t *msg_id_out);
static void process_rx_message(uint32_t can_id, const uint8_t *data, uint8_t dlc);
static inline void pack_u16(uint8_t *dest, uint16_t value);
static inline void pack_i16(uint8_t *dest, int16_t value);

/**
 * @brief Initialize the CAN RX message queue.
 *        Call this once from main.c after FreeRTOS kernel starts.
 */
void CAN_ProcessQueue_Init(void) {
    if (can_rx_queue == NULL) {
        can_rx_queue = osMessageQueueNew(CAN_RX_QUEUE_SIZE, sizeof(CAN_RxMsg_t), NULL);
    }
}

/**
 * @brief Drain all pending CAN messages from the queue and process them.
 *        Call this from readings_manage task (or any task context where Mutexes are allowed).
 *        This is ISR-SAFE to call because it runs outside ISR context.
 */
void CAN_ProcessQueueDrain(void) {
    if (can_rx_queue == NULL) return;
    
    CAN_RxMsg_t msg;
    osStatus_t status;
    
    /* Non-blocking dequeue: process all available messages */
    while ((status = osMessageQueueGet(can_rx_queue, &msg, NULL, 0)) == osOK) {
        /* Process the message safely (inside task context, mutexes are OK) */
        process_rx_message(msg.can_id, msg.data, msg.dlc);
    }
}

/**
 * @brief Enqueue a CAN message from ISR (ISR-safe).
 *        Called by the CAN RX interrupt handler.
 */
static void CAN_EnqueueRxMessage(uint32_t can_id, const uint8_t *data, uint8_t dlc) {
    if (can_rx_queue == NULL) return;
    
    CAN_RxMsg_t msg;
    msg.can_id = can_id;
    msg.dlc = (dlc <= 8U) ? dlc : 8U;
    memset(msg.data, 0, sizeof(msg.data));
    if (data != NULL && msg.dlc > 0U) {
        memcpy(msg.data, data, msg.dlc);
    }
    
    /* osMessageQueuePut is ISR-safe (uses internal ISR flag) */
    osMessageQueuePut(can_rx_queue, &msg, 0, 0);
}

/* =============================================================================
 *  CAN2 RX MESSAGE QUEUE (ISR-safe architecture)
 * =============================================================================
 *  Messages are enqueued by ISR (ISR-safe), dequeued by readings_manage task.
 *  This avoids calling FreeRTOS Mutexes inside the ISR.
 * ============================================================================= */

#define CAN2_RX_QUEUE_SIZE  16   /**< CAN2 queue size (car data, lower frequency) */

static osMessageQueueId_t can2_rx_queue = NULL;  /**< CAN2 Queue handle */

/* Forward declaration */
static void process_car_rx_message(uint32_t can_id, const uint8_t *data, uint8_t dlc);

/**
 * @brief Initialize the CAN2 RX message queue.
 *        Call this once from main.c after FreeRTOS kernel starts.
 */
void CAN_Car_ProcessQueue_Init(void) {
    if (can2_rx_queue == NULL) {
        can2_rx_queue = osMessageQueueNew(CAN2_RX_QUEUE_SIZE, sizeof(CAN_RxMsg_t), NULL);
    }
}

/**
 * @brief Enqueue a CAN2 message from ISR (ISR-safe).
 *        Called by the CAN RX interrupt handler.
 */
static void CAN_Car_EnqueueRxMessage(uint32_t can_id, const uint8_t *data, uint8_t dlc) {
    if (can2_rx_queue == NULL) return;
    
    CAN_RxMsg_t msg;
    msg.can_id = can_id;
    msg.dlc = (dlc <= 8U) ? dlc : 8U;
    memset(msg.data, 0, sizeof(msg.data));
    if (data != NULL && msg.dlc > 0U) {
        memcpy(msg.data, data, msg.dlc);
    }
    
    /* osMessageQueuePut is ISR-safe (uses internal ISR flag) */
    osMessageQueuePut(can2_rx_queue, &msg, 0, 0);
}

/**
 * @brief Drain all pending CAN2 messages from the queue and process them.
 *        Call this from readings_manage task (or any task context where Mutexes are allowed).
 *        This is ISR-SAFE to call because it runs outside ISR context.
 */
void CAN_Car_ProcessQueueDrain(void) {
    if (can2_rx_queue == NULL) return;
    
    CAN_RxMsg_t msg;
    osStatus_t status;
    
    /* Non-blocking dequeue: process all available messages */
    while ((status = osMessageQueueGet(can2_rx_queue, &msg, NULL, 0)) == osOK) {
        /* Process the message safely (inside task context, mutexes are OK) */
        process_car_rx_message(msg.can_id, msg.data, msg.dlc);
    }
}


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
    HAL_CAN_ActivateNotification(hcan,CAN_IT_ERROR_WARNING | CAN_IT_ERROR_PASSIVE );

}

void CAN_Car_Init(CAN_HandleTypeDef *hcan) {
    if (hcan == NULL) return;

    configure_can_filters(hcan);
    HAL_CAN_Start(hcan);
    HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO1_MSG_PENDING);
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan) {
    uint32_t esr = hcan->Instance->ESR;
    uint8_t tec = (esr >> 24) & 0xFF;
    uint8_t rec = (esr >> 16) & 0xFF;

    /* This callback is IRQ context: avoid Serial_Log here (it uses mutexes/osDelay). */
    (void)esr;
    (void)tec;
    (void)rec;
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

    // Drain all available messages in FIFO1 and enqueue them
    // (avoids overrun under burst traffic)
    while (HAL_CAN_GetRxFifoFillLevel(hcan, CAN_RX_FIFO1) > 0) {
        if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &rx_header, rx_data) == HAL_OK) {
            /* Enqueue message for processing in task context */
            CAN_Car_EnqueueRxMessage(rx_header.StdId, rx_data, rx_header.DLC);
        } else {
            break;
        }
    }
}

/**
 * @brief Process a single received CAN2 message (TASK-SAFE - called from CAN_Car_ProcessQueueDrain)
 *        Mutexes are safe here (not in ISR context).
 */
static void process_car_rx_message(uint32_t can_id, const uint8_t *data, uint8_t dlc) {
    if (data == NULL) return;

    /* Lock CAN2 data mutex BEFORE updating shared car_data */
    /* SAFE HERE: We are in task context, NOT inside ISR */
    Mutex_CAN2_Lock();

    switch (can_id) {
        case CAN_ID_IMU_ACC:
            if (dlc < 8U) break;
            // Big Endian: byte[n]=MSB, byte[n+1]=LSB
            car_data.imu_data.accel_x    = (int16_t)(((uint16_t)data[0] << 8 | data[1])* IMU_ACCEL_SCALE);
            car_data.imu_data.accel_y    = (int16_t)(((uint16_t)data[2] << 8 | data[3] )* IMU_ACCEL_SCALE);
            car_data.imu_data.accel_z    = (int16_t)(((uint16_t)data[4] << 8 | data[5])* IMU_ACCEL_SCALE);
            car_data.imu_data.serial_num = (uint16_t)((uint16_t)data[6] << 8 | data[7]);
            break;
        case CAN_ID_IMU_GYRO:
            if (dlc < 8U) break;
            car_data.imu_data.gyro_x        = (int16_t)(((uint16_t)data[0] << 8 | data[1]) * IMU_GYRO_SCALE);
            car_data.imu_data.gyro_y        = (int16_t)(((uint16_t)data[2] << 8 | data[3]) * IMU_GYRO_SCALE);
            car_data.imu_data.gyro_z        = (int16_t)(((uint16_t)data[4] << 8 | data[5]) * IMU_GYRO_SCALE);
            car_data.imu_data.internal_temp = (int16_t)((uint16_t)data[6] << 8 | data[7]);
            break;
        case CAN_ID_IMU_MAG:
            if (dlc < 6U) break;
            car_data.imu_data.mag_x = (int16_t)(((uint16_t)data[0] << 8 | data[1]) * IMU_MAG_SCALE);
            car_data.imu_data.mag_y = (int16_t)(((uint16_t)data[2] << 8 | data[3]) * IMU_MAG_SCALE);
            car_data.imu_data.mag_z = (int16_t)(((uint16_t)data[4] << 8 | data[5]) * IMU_MAG_SCALE);
            break;
        case CAN_ID_BMS:
            if (dlc < 6U) break;
            car_data.bms_data.soc           = (int8_t)data[0];
            car_data.bms_data.soh           = (int8_t)data[1];
            car_data.bms_data.max_discharge = (uint16_t)((uint16_t)data[2] << 8 | data[3]);
            car_data.bms_data.max_regen     = (uint16_t)((uint16_t)data[4] << 8 | data[5]);
            break;
        case CAN_ID_ECU_RPM:
            if (dlc < 4U) break;
            car_data.fwheels_rpm_data.rpm_fr = (uint16_t)((uint16_t)data[0] << 8 | data[1]);
            car_data.fwheels_rpm_data.rpm_fl = (uint16_t)((uint16_t)data[2] << 8 | data[3]);
            break;
        case CAN_ID_ECU_BRAKE:
            if (dlc < 1U) break;
            car_data.brake_pressure = data[0];
            break;
        case CAN_ID_DASH_R2D:
            if (dlc < 2U) break;
            car_data.engine_map = (int8_t)data[1];
            break;
        default:
            break;
    }

    /* Unlock CAN2 data mutex */
    Mutex_CAN2_Unlock();
}


const CarData_t* CAN_Car_GetData(void) {
    /* NOTE: Caller must ensure proper synchronization when reading this structure.
     * The CAN2 data may be updated by ReadingsManageTask, so concurrent reads/writes
     * are possible. Use Mutex_CAN2_Lock/Unlock around critical sections.
     *
     * For now, we return the pointer directly. Callers accessing this should:
     *   - Acquire Mutex_CAN2_Lock()
     *   - Access the data
     *   - Call Mutex_CAN2_Unlock()
     * OR use volatile reads if only reading primitives (no compound structures).
     */
    return &car_data;
}



// Executed by CAN interrupt for each message received on CAN Inverter-Motors
// ISR-SAFE: Extracts CAN messages and enqueues them. Processing happens in task context.
void CAN_Inverter_ProcessRxMessages(CAN_HandleTypeDef *hcan) {
    if (hcan == NULL) return;
    
    CAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];
    
    // Drain all available messages in FIFO0 and enqueue them
    // (avoids overrun under burst traffic)
    while (HAL_CAN_GetRxFifoFillLevel(hcan, CAN_RX_FIFO0) > 0) {
        if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK) {
            /* Enqueue message for processing in task context */
            CAN_EnqueueRxMessage(rx_header.StdId, rx_data, rx_header.DLC);
        } else {
            break;
        }
    }
}


/**
 * @brief Process a single received CAN message (TASK-SAFE - called from CAN_ProcessTask)
 *        Mutexes are safe here (not in ISR context).
 */
static void process_rx_message(uint32_t can_id, const uint8_t *data, uint8_t dlc) {
    if (data == NULL || dlc < 8U) return;

    uint8_t msg_id;     // which Msg arrived (1-5)
    Inverter_t *inv = find_inverter_by_rx_id(can_id, &msg_id);

    if (inv == NULL) return;

    /* Increment RX counter for diagnostics */
    extern uint32_t g_can_rx_count;
    g_can_rx_count++;

    /* Lock the appropriate inverter mutex BEFORE updating shared state */
    /* SAFE HERE: We are in task context, NOT inside ISR */
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

#if !INVERTER_RIGHT_PRESENT
    if (inv->node_id == INVERTER_RIGHT_NODE_ID) return false;
#endif
#if !INVERTER_LEFT_PRESENT
    if (inv->node_id == INVERTER_LEFT_NODE_ID) return false;
#endif

    uint8_t tx_data[8];
    pack_u16(&tx_data[0], cmd->control_word);
    pack_i16(&tx_data[2], cmd->torque_setpoint);
    pack_i16(&tx_data[4], cmd->torque_limit_pos);
    pack_i16(&tx_data[6], cmd->torque_limit_neg);

    bool ok = transmit_can_message(hcan, inv->tx_can_id, tx_data, 8);

    /* NOTE: Serial_Log is intentionally NOT called here.
     * This function is called under Mutex_CAN1 from InvertersManageTask.
     * Calling Serial_Log (which acquires Mutex_UART3 + osDelay) inside
     * a CAN mutex region would cause deadlock risk and add variable latency
     * to the 10 ms control loop. TX diagnostics are available via
     * g_can_tx_ok_count / g_can_tx_fail_count in the data logger.
     */
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
