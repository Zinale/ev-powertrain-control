/**
 * @file Can.c
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief CAN implementation for motor inverter (CAN1)
 * 
 * Handles CAN1 communication dedicated to AMK motor inverters.
 * Scalable: passing hcan as parameter allows future CAN2 integration.
 */

#include "Connectivity/Can.h"
#include "mutexes.h"
#include <string.h>
#include "Connectivity/Serial.h"
#include "Control/control_config.h"
#include "cmsis_os.h"

#define N_INVERTERS  2      // if more than two inverters, change CAN ID logic
static Inverter_t *s_inverter[N_INVERTERS] = {NULL, NULL};

static CarData_t car_data = {0};

volatile CAN_RuntimeState_t g_can1_runtime_state = CAN_RUNTIME_UNINITIALIZED;
volatile CAN_RuntimeState_t g_can2_runtime_state = CAN_RUNTIME_UNINITIALIZED;
volatile uint32_t g_can1_last_error = 0U;
volatile uint32_t g_can2_last_error = 0U;

static uint32_t s_can_recovery_period_ms = 2000U;
static uint32_t s_can1_last_retry_ms = 0U;
static uint32_t s_can2_last_retry_ms = 0U;

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

static osMessageQueueId_t can_motor_rx_queue = NULL;  /**< Queue handle */

/* Forward declarations - must be before CAN_MOTOR_ProcessQueueDrain which calls them */
static bool configure_fdcan_filters(FDCAN_HandleTypeDef *hfdcan);
static bool configure_fdcan_filters_main(FDCAN_HandleTypeDef *hfdcan);
static bool transmit_can_message(FDCAN_HandleTypeDef *hfdcan, uint32_t can_id, const uint8_t *data, uint8_t dlc);
static Inverter_t* find_inverter_by_rx_id(uint32_t can_id, uint8_t *msg_id_out);
static void process_motor_rx_message(uint32_t can_id, const uint8_t *data, uint8_t dlc);
static inline void pack_u16(uint8_t *dest, uint16_t value);
static inline void pack_i16(uint8_t *dest, int16_t value);
static inline uint32_t dlc_to_fdcan(uint8_t dlc);

/**
 * @brief Initialize the CAN RX message queue.
 *        Call this once from main.c after FreeRTOS kernel starts.
 */
void CAN_MOTOR_ProcessQueue_Init(void) {
    if (can_motor_rx_queue == NULL) {
        can_motor_rx_queue = osMessageQueueNew(CAN_RX_QUEUE_SIZE, sizeof(CAN_RxMsg_t), NULL);
    }
}

/**
 * @brief Drain all pending CAN messages from the queue and process them.
 *        Call this from readings_manage task (or any task context where Mutexes are allowed).
 *        This is ISR-SAFE to call because it runs outside ISR context.
 */
void CAN_MOTOR_ProcessQueueDrain(void) {
    if (can_motor_rx_queue == NULL) return;
    
    CAN_RxMsg_t msg;
    osStatus_t status;
    
    /* Non-blocking dequeue: process all available messages */
    while ((status = osMessageQueueGet(can_motor_rx_queue, &msg, NULL, 0)) == osOK) {
        /* Process the message safely (inside task context, mutexes are OK) */
        process_motor_rx_message(msg.can_id, msg.data, msg.dlc);
    }
}

/**
 * @brief Enqueue a CAN message from ISR (ISR-safe).
 *        Called by the CAN RX interrupt handler.
 */
static void CAN_MOTOR_EnqueueRxMessage(uint32_t can_id, const uint8_t *data, uint8_t dlc) {
    if (can_motor_rx_queue == NULL) return;
    if (osKernelGetState() != osKernelRunning) return;
    
    CAN_RxMsg_t msg;
    msg.can_id = can_id;
    msg.dlc = (dlc <= 8U) ? dlc : 8U;
    memset(msg.data, 0, sizeof(msg.data));
    if (data != NULL && msg.dlc > 0U) {
        memcpy(msg.data, data, msg.dlc);
    }
    
    /* osMessageQueuePut is ISR-safe (uses internal ISR flag) */
    osMessageQueuePut(can_motor_rx_queue, &msg, 0, 0);
}

/* =============================================================================
 *  CAN2 RX MESSAGE QUEUE (ISR-safe architecture)
 * =============================================================================
 *  Messages are enqueued by ISR (ISR-safe), dequeued by readings_manage task.
 *  This avoids calling FreeRTOS Mutexes inside the ISR.
 * ============================================================================= */

#define CAN2_RX_QUEUE_SIZE  32   /**< CAN2 queue size (car data, lower frequency) */

static osMessageQueueId_t can_main_rx_queue = NULL;  /**< CAN2 Queue handle */

/* Forward declaration */
static void process_main_rx_message(uint32_t can_id, const uint8_t *data, uint8_t dlc);

/**
 * @brief Initialize the CAN2 RX message queue.
 *        Call this once from main.c after FreeRTOS kernel starts.
 */
void CAN_MAIN_ProcessQueue_Init(void) {
    if (can_main_rx_queue == NULL) {
        can_main_rx_queue = osMessageQueueNew(CAN2_RX_QUEUE_SIZE, sizeof(CAN_RxMsg_t), NULL);
    }
}

bool CAN_QueuesReady(void) {
    return (can_motor_rx_queue != NULL) && (can_main_rx_queue != NULL);
}

/**
 * @brief Enqueue a CAN2 message from ISR (ISR-safe).
 *        Called by the CAN RX interrupt handler.
 */
static void CAN_MAIN_EnqueueRxMessage(uint32_t can_id, const uint8_t *data, uint8_t dlc) {
    if (can_main_rx_queue == NULL) return;
    if (osKernelGetState() != osKernelRunning) return;
    
    CAN_RxMsg_t msg;
    msg.can_id = can_id;
    msg.dlc = (dlc <= 8U) ? dlc : 8U;
    memset(msg.data, 0, sizeof(msg.data));
    if (data != NULL && msg.dlc > 0U) {
        memcpy(msg.data, data, msg.dlc);
    }
    
    /* osMessageQueuePut is ISR-safe (uses internal ISR flag) */
    osMessageQueuePut(can_main_rx_queue, &msg, 0, 0);
}

/**
 * @brief Drain all pending CAN2 messages from the queue and process them.
 *        Call this from readings_manage task (or any task context where Mutexes are allowed).
 *        This is ISR-SAFE to call because it runs outside ISR context.
 */
void CAN_MAIN_ProcessQueueDrain(void) {
    if (can_main_rx_queue == NULL) return;
    
    CAN_RxMsg_t msg;
    osStatus_t status;
    
    /* Non-blocking dequeue: process all available messages */
    while ((status = osMessageQueueGet(can_main_rx_queue, &msg, NULL, 0)) == osOK) {
        /* Process the message safely (inside task context, mutexes are OK) */
        process_main_rx_message(msg.can_id, msg.data, msg.dlc);
        g_can2_rx_count++;
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


/* =============================================================================
 *  CAN1 SCE (Status Change Error) — ISR-safe diagnostics
 * =============================================================================
 Snapshot del registro ESR al momento dell'ultimo errore CAN1 */
volatile uint32_t g_can1_sce_esr    = 0U;

/** Transmit/Receive Error Counters al momento dell'errore */
volatile uint8_t  g_can1_sce_tec    = 0U;   /**< Transmit Error Counter */
volatile uint8_t  g_can1_sce_rec    = 0U;   /**< Receive  Error Counter */

/** Flag bus-off: scritto a 1 dall'ISR, resettato dalla task dopo recovery */
volatile uint8_t  g_can1_busoff     = 0U;

/** Contatore totale eventi SCE (per diagnostica in DataLogger) */
volatile uint32_t g_can1_sce_count  = 0U;

/** Contatore totale messaggi CAN1 processati (per diagnostica in DataLogger) */
volatile uint32_t g_can_rx_count    = 0U;

/** Contatore totale messaggi CAN2 (main) processati: scritto solo da CAN_MAIN_ProcessQueueDrain */
volatile uint32_t g_can2_rx_count   = 0U;


void CAN_MOTOR_Init(FDCAN_HandleTypeDef *hfdcan) {
    if (hfdcan == NULL) return;

    g_can1_runtime_state = CAN_RUNTIME_UNINITIALIZED;
    g_can1_last_error = 0U;

    if (!configure_fdcan_filters(hfdcan)) {
        g_can1_runtime_state = CAN_RUNTIME_INIT_FAILED;
        g_can1_last_error = hfdcan->ErrorCode;
        return;
    }

    {
        const uint32_t max_attempts = 5U;
        uint32_t attempt;
        for (attempt = 1U; attempt <= max_attempts; attempt++) {
            if (HAL_FDCAN_Start(hfdcan) == HAL_OK) {
                g_can1_runtime_state = CAN_RUNTIME_READY;
                g_can1_last_error = 0U;
                break;
            }

            g_can1_last_error = hfdcan->ErrorCode;
            (void)HAL_FDCAN_Stop(hfdcan);
            osDelay(20);
        }

        if (attempt > max_attempts) {
            g_can1_runtime_state = CAN_RUNTIME_INIT_FAILED;
            return;
        }
    }

    if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0U) != HAL_OK) {
        g_can1_runtime_state = CAN_RUNTIME_INIT_FAILED;
        g_can1_last_error = hfdcan->ErrorCode;
        return;
    }

    /* Abilita le notifiche di errore CAN1 (unico bus con diagnostica SCE dettagliata) */
    if (HAL_FDCAN_ActivateNotification(hfdcan,
        FDCAN_IT_ERROR_WARNING |   /**< TEC o REC > 96  — bus degradato */
        FDCAN_IT_ERROR_PASSIVE |   /**< TEC o REC > 127 — passive mode  */
        FDCAN_IT_BUS_OFF,          /**< TEC > 255       — bus off       */
        0U
    ) != HAL_OK) {
        g_can1_runtime_state = CAN_RUNTIME_INIT_FAILED;
        g_can1_last_error = hfdcan->ErrorCode;
        return;
    }
}

bool CAN_MAIN_Start(FDCAN_HandleTypeDef *hfdcan) {
    if (hfdcan == NULL) return false;

    /* Gia' avviato (es. dalla procedura di boot IMU in main): niente da fare. */
    if (HAL_FDCAN_GetState(hfdcan) == HAL_FDCAN_STATE_BUSY) {
        return true;
    }

    g_can2_runtime_state = CAN_RUNTIME_UNINITIALIZED;
    g_can2_last_error = 0U;

    if (!configure_fdcan_filters_main(hfdcan)) {
        g_can2_runtime_state = CAN_RUNTIME_INIT_FAILED;
        g_can2_last_error = hfdcan->ErrorCode;
        return false;
    }

    if (HAL_FDCAN_Start(hfdcan) != HAL_OK) {
        g_can2_runtime_state = CAN_RUNTIME_INIT_FAILED;
        g_can2_last_error = hfdcan->ErrorCode;
        return false;
    }

    return true;
}

void CAN_MAIN_Init(FDCAN_HandleTypeDef *hfdcan) {
    if (hfdcan == NULL) return;

    if (!CAN_MAIN_Start(hfdcan)) {
        return;   /* g_can2_runtime_state gia' = CAN_RUNTIME_INIT_FAILED */
    }

    if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0U) != HAL_OK) {
        g_can2_runtime_state = CAN_RUNTIME_INIT_FAILED;
        g_can2_last_error = hfdcan->ErrorCode;
        return;
    }

    /* FDCAN2 puo' essere stato avviato molto prima (boot IMU in main): svuota
     * l'eventuale backlog in FIFO0 accumulato senza ISR, altrimenti con FIFO
     * piena non arriverebbe piu' alcuna notifica "new message". */
    CAN_MAIN_ProcessRxMessages(hfdcan);

    g_can2_runtime_state = CAN_RUNTIME_READY;
    g_can2_last_error = 0U;
}

/* ============================================================================
 * Procedura di boot IMU (Aviorace fw 2.09) - via CAN2, in polling
 * ============================================================================ */
volatile ImuBootStatus_t g_imu_boot_status = IMU_BOOT_UNKNOWN;
volatile uint16_t         g_imu_fw_version = 0U;

static inline void imu_kick_wdg(IWDG_HandleTypeDef *hiwdg) {
    if (hiwdg != NULL) {
        (void)HAL_IWDG_Refresh(hiwdg);
    }
}

/* Attende un frame 0x0E4 in polling sulla FIFO0, scartando gli altri.
 * Ritorna true e riempie reply[8] se ricevuto entro timeout_ms. */
static bool imu_wait_info_reply(FDCAN_HandleTypeDef *hfdcan, IWDG_HandleTypeDef *hiwdg,
                                uint8_t reply[8], uint32_t timeout_ms) {
    const uint32_t t0 = HAL_GetTick();
    FDCAN_RxHeaderTypeDef hdr;
    uint8_t data[8];

    while ((HAL_GetTick() - t0) < timeout_ms) {
        imu_kick_wdg(hiwdg);

        if (HAL_FDCAN_GetRxFifoFillLevel(hfdcan, FDCAN_RX_FIFO0) == 0U) {
            continue;
        }
        if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &hdr, data) != HAL_OK) {
            continue;
        }
        if (hdr.Identifier == IMU_CAN_ID_INFO_REPLY) {
            memcpy(reply, data, 8U);
            return true;
        }
        /* frame dati IMU (0x100..) o altro traffico: scartati durante il boot */
    }
    return false;
}

ImuBootStatus_t CAN_MAIN_ImuBoot(FDCAN_HandleTypeDef *hfdcan, IWDG_HandleTypeDef *hiwdg) {
    if (hfdcan == NULL) {
        g_imu_boot_status = IMU_BOOT_NO_RESPONSE;
        return g_imu_boot_status;
    }

    static const uint8_t get_info[8] = {
        0x01U, IMU_CMD_GET_INFO, 0xAAU, 0xAAU, 0xAAU, 0xAAU, 0xAAU, 0xAAU
    };
    uint8_t reply[8] = {0};
    bool got = false;

    for (uint8_t i = 0U; (i < IMU_BOOT_RETRIES) && !got; ++i) {
        (void)CAN_MAIN_TransmitRaw(hfdcan, IMU_CAN_ID_REPROGRAM, get_info, 8U);
        got = imu_wait_info_reply(hfdcan, hiwdg, reply, IMU_BOOT_REPLY_TIMEOUT_MS);
    }

    if (!got) {
        g_imu_boot_status = IMU_BOOT_NO_RESPONSE;
        return g_imu_boot_status;
    }

    /* 0x0E4 (BIG ENDIAN):
     *  [0-1] Fw version (x0.01)  [2] Acc range   [3] Gyro range
     *  [4] Acc cutoff            [5] Gyro cutoff
     *  [6] Positioning           [7] Bootloader ver (x0.1)               */
    g_imu_fw_version = (uint16_t)(((uint16_t)reply[0] << 8) | reply[1]);

    const bool cfg_ok = (reply[2] == IMU_EXPECT_ACC_RANGE)  &&
                        (reply[3] == IMU_EXPECT_GYRO_RANGE) &&
                        (reply[4] == IMU_EXPECT_ACC_FREQ)   &&
                        (reply[5] == IMU_EXPECT_GYRO_FREQ)  &&
                        (reply[6] == IMU_EXPECT_POSITIONING);

    if (cfg_ok) {
        g_imu_boot_status = IMU_BOOT_OK;
        return g_imu_boot_status;
    }

#if (IMU_BOOT_AUTOCONFIG == 1)
    {
        /* 0xA0: acc/gyro range + can base-id + baud. L'IMU si auto-resetta.
         * ATTENZIONE: baud errato = IMU fuori dal bus. IMU_EXPECT_BAUD deve
         * combaciare con il bit-timing di MX_FDCAN2_Init (500 kbps). */
        const uint8_t set_ranges[8] = {
            0x01U, IMU_CMD_SET_RANGES, 0xAAU,
            IMU_EXPECT_ACC_RANGE, IMU_EXPECT_GYRO_RANGE, IMU_EXPECT_BAUD,
            (uint8_t)((IMU_BASE_CAN_ID >> 8) & 0xFFU),
            (uint8_t)(IMU_BASE_CAN_ID & 0xFFU)
        };
        /* 0xA3: positioning + cutoff acc/gyro. L'IMU si auto-resetta. */
        const uint8_t set_posfreq[8] = {
            0x01U, IMU_CMD_SET_POS_FREQ, 0xAAU, 0xAAU, 0xAAU,
            IMU_EXPECT_POSITIONING, IMU_EXPECT_ACC_FREQ, IMU_EXPECT_GYRO_FREQ
        };

        (void)CAN_MAIN_TransmitRaw(hfdcan, IMU_CAN_ID_REPROGRAM, set_ranges, 8U);
        for (uint32_t w = 0U; w < IMU_BOOT_RESET_WAIT_MS; w += 100U) {
            imu_kick_wdg(hiwdg);
            HAL_Delay(100U);
        }
        (void)CAN_MAIN_TransmitRaw(hfdcan, IMU_CAN_ID_REPROGRAM, set_posfreq, 8U);
        for (uint32_t w = 0U; w < IMU_BOOT_RESET_WAIT_MS; w += 100U) {
            imu_kick_wdg(hiwdg);
            HAL_Delay(100U);
        }

        got = false;
        for (uint8_t i = 0U; (i < IMU_BOOT_RETRIES) && !got; ++i) {
            (void)CAN_MAIN_TransmitRaw(hfdcan, IMU_CAN_ID_REPROGRAM, get_info, 8U);
            got = imu_wait_info_reply(hfdcan, hiwdg, reply, IMU_BOOT_REPLY_TIMEOUT_MS);
        }
        g_imu_boot_status = got ? IMU_BOOT_RECONFIGURED : IMU_BOOT_NO_RESPONSE;
    }
#else
    g_imu_boot_status = IMU_BOOT_MISMATCH;
#endif

    return g_imu_boot_status;
}

void HAL_FDCAN_ErrorStatusCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t ErrorStatusITs) {
    /* Filtro: gestisci solo CAN1 (bus inverter - safety critical). */
    if (hfdcan->Instance != FDCAN1) return;
    (void)ErrorStatusITs;

    FDCAN_ProtocolStatusTypeDef proto = {0};
    FDCAN_ErrorCountersTypeDef  cnt   = {0};
    HAL_FDCAN_GetProtocolStatus(hfdcan, &proto);
    HAL_FDCAN_GetErrorCounters(hfdcan, &cnt);

    g_can1_sce_esr = proto.LastErrorCode;
    g_can1_sce_tec = (uint8_t)cnt.TxErrorCnt;
    g_can1_sce_rec = (uint8_t)cnt.RxErrorCnt;
    g_can1_sce_count++;

    if (proto.BusOff) {
        g_can1_busoff = 1U;
    }
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
void CAN_MOTOR_Register(Inverter_t *inv) {
    if (inv == NULL) return;

    uint8_t idx = inv->node_id - 1;
    if (idx < 2) {
        s_inverter[idx] = inv;
    }
}


/**
 * @brief Configure FDCAN filters
 *
 * Ogni bus (FDCAN1 = inverter, FDCAN2 = car data) ha un solo filtro hardware
 * disponibile (StdFiltersNbr=1 in CubeMX): un filtro a maschera 0x000/0x000
 * accetta tutti gli ID standard e li instrada sulla FIFO0 del bus; il dispatch
 * per singolo ID resta software (find_inverter_by_rx_id / switch in
 * process_main_rx_message), come gia' avveniva col CAN classico.
 */
static bool configure_fdcan_filters(FDCAN_HandleTypeDef *hfdcan) {
    FDCAN_FilterTypeDef filter_config = {0};

    if (hfdcan == NULL) return false;

    filter_config.IdType       = FDCAN_STANDARD_ID;
    filter_config.FilterIndex  = 0U;
    filter_config.FilterType   = FDCAN_FILTER_MASK;
    filter_config.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    filter_config.FilterID1    = 0x000U;
    filter_config.FilterID2    = 0x000U;  /* mask 0 -> accetta tutti gli ID standard */
    if (HAL_FDCAN_ConfigFilter(hfdcan, &filter_config) != HAL_OK) {
        return false;
    }

    /* Scarta cio' che non passa dal filtro sopra (ID estesi, remote frame) */
    if (HAL_FDCAN_ConfigGlobalFilter(hfdcan, FDCAN_REJECT, FDCAN_REJECT,
                                      FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE) != HAL_OK) {
        return false;
    }

    return true;
}

/**
 * @brief Configura il filtro FDCAN2 (bus CAN Main) per range.
 *
 * Un solo filtro hardware (StdFiltersNbr=1 in CubeMX), ID standard 0x000-0x3FF:
 * copre IMU/BMS/SDC/PDM/dashboard (< 0x200) e il sensore corrente LEM CAB 500
 * (0x3C2). Tutto cio' che sta sopra 0x3FF viene scartato dalla periferica.
 */
static bool configure_fdcan_filters_main(FDCAN_HandleTypeDef *hfdcan) {
    FDCAN_FilterTypeDef filter_config = {0};

    if (hfdcan == NULL) return false;

    filter_config.IdType       = FDCAN_STANDARD_ID;
    filter_config.FilterIndex  = 0U;
    filter_config.FilterType   = FDCAN_FILTER_RANGE;
    filter_config.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    filter_config.FilterID1    = 0x000U;
    filter_config.FilterID2    = 0x3FFU;
    if (HAL_FDCAN_ConfigFilter(hfdcan, &filter_config) != HAL_OK) {
        return false;
    }

    return (HAL_FDCAN_ConfigGlobalFilter(hfdcan, FDCAN_REJECT, FDCAN_REJECT,
                                          FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE) == HAL_OK);
}


void CAN_MAIN_ProcessRxMessages(FDCAN_HandleTypeDef *hfdcan) {
    if (hfdcan == NULL) return;

    FDCAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];

    // Drain all available messages in FIFO0 and enqueue them
    // (avoids overrun under burst traffic)
    while (HAL_FDCAN_GetRxFifoFillLevel(hfdcan, FDCAN_RX_FIFO0) > 0) {
        if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK) {
            /* Enqueue message for processing in task context */
            CAN_MAIN_EnqueueRxMessage(rx_header.Identifier, rx_data, (uint8_t)(rx_header.DataLength >> 16));
        } else {
            break;
        }
    }
}

/**
 * @brief Process a single received CAN2 message (TASK-SAFE - called from CAN_MAIN_ProcessQueueDrain)
 *        Mutexes are safe here (not in ISR context).
 */
static void process_main_rx_message(uint32_t can_id, const uint8_t *data, uint8_t dlc) {
    if (data == NULL) return;

    /* Lock CAN2 data mutex BEFORE updating shared car_data */
    /* SAFE HERE: We are in task context, NOT inside ISR */
    Mutex_FDCAN2_MAIN_Lock();

    switch (can_id) {
        case CAN_ID_IMU_ACC:
            if (dlc < 6U) break;
            // Big Endian: byte[n]=MSB, byte[n+1]=LSB
            car_data.imu_data.accel_x = (int16_t)(((uint16_t)data[0] << 8 | data[1]) * IMU_ACCEL_SCALE);
            car_data.imu_data.accel_y = (int16_t)(((uint16_t)data[2] << 8 | data[3]) * IMU_ACCEL_SCALE);
            car_data.imu_data.accel_z = (int16_t)(((uint16_t)data[4] << 8 | data[5]) * IMU_ACCEL_SCALE);
            car_data.imu_data.last_update_timestamp = HAL_GetTick();
            break;
        case CAN_ID_IMU_GYRO:
            if (dlc < 6U) break;
            car_data.imu_data.gyro_x = (int16_t)(((uint16_t)data[0] << 8 | data[1]) * IMU_GYRO_SCALE);
            car_data.imu_data.gyro_y = (int16_t)(((uint16_t)data[2] << 8 | data[3]) * IMU_GYRO_SCALE);
            car_data.imu_data.gyro_z = (int16_t)(((uint16_t)data[4] << 8 | data[5]) * IMU_GYRO_SCALE);
            car_data.imu_data.last_update_timestamp = HAL_GetTick();
            break;
        case CAN_ID_IMU_MAG:
            if (dlc < 6U) break;
            car_data.imu_data.mag_x = (int16_t)(((uint16_t)data[0] << 8 | data[1]) * IMU_MAG_SCALE);
            car_data.imu_data.mag_y = (int16_t)(((uint16_t)data[2] << 8 | data[3]) * IMU_MAG_SCALE);
            car_data.imu_data.mag_z = (int16_t)(((uint16_t)data[4] << 8 | data[5]) * IMU_MAG_SCALE);
            car_data.imu_data.last_update_timestamp = HAL_GetTick();
            break;
        case CAN_ID_BMS:
            if (dlc < 6U) break;
            car_data.bms_data.soc           = (int8_t)data[0];
            car_data.bms_data.soh           = (int8_t)data[1];
            car_data.bms_data.max_discharge = (uint16_t)(((uint16_t)data[2] << 8 | data[3]) * 0.01);
            car_data.bms_data.max_regen     = (uint16_t)(((uint16_t)data[4] << 8 | data[5]) * 0.01);
            car_data.bms_data.last_update_timestamp = HAL_GetTick();
            break;
        case CAN_ID_DASH_R2D_ENGINE_MAP:
            if (dlc < 3U) break;
            car_data.dashboard.r2d                   = data[0];
            car_data.dashboard.engine_map            = (int8_t)data[1];
            car_data.dashboard.parameters_config     = data[2];
            car_data.dashboard.last_update_timestamp = HAL_GetTick();
            break;
        case CAN_ID_SDC_STATUS:
            if (dlc > 1U) break;
            car_data.bms_data.SDC_status = (bool)data[0];
            car_data.bms_data.last_update_timestamp = HAL_GetTick();
            break;
        case CAN_ID_CAB500_CURRENT:
            if (dlc < 4U) break;
            /* CSM_BAT_CURRENT: 32 bit BIG-ENDIAN nei byte 0-3, offset binario
             * (0x80000000 = 0 mA). Layout allineato al driver del progetto BMS
             * (LEM_CAB500_CURR_BYTE_INDEX = 0). */
            {
                uint32_t raw = ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) |
                               ((uint32_t)data[2] << 8)  |  (uint32_t)data[3];
                int32_t  i_ma = (int32_t)(raw - 0x80000000U);
                car_data.bms_data.pack_current_a    = CAB500_DISCHARGE_SIGN * ((float)i_ma * 0.001f);
                car_data.bms_data.cab500_last_update = HAL_GetTick();
            }
            break;
        case CAN_ID_BMS_ERROR:
            if (dlc < 4 || dlc > 4U) break;
            car_data.bms_data.bms_error = (bool)data[0] || (bool)data[1] || (bool)data[2] || (bool)data[3];
            car_data.bms_data.last_update_timestamp = HAL_GetTick();
            break;
        case CAN_ID_PDM1_ERROR:
            if (dlc < 2 || dlc > 2U) break;
            /* byte 0 = Errors Power, byte 1 = Errors Output: bit0 di ciascuno = "Error present" */
            car_data.pdm_data.PDM1_error = (((data[0] | data[1]) & 0x01U) != 0U);
            car_data.pdm_data.last_update_timestamp = HAL_GetTick();
            break;
        case CAN_ID_PDM2_ERROR:
            if (dlc < 1 || dlc > 1U) break;
            car_data.pdm_data.PDM2_error = (bool)(data[0] & 0x01);
            car_data.pdm_data.last_update_timestamp = HAL_GetTick();
            break;
        default:
            break;
    }

    /* Unlock CAN2 data mutex */
    Mutex_FDCAN2_MAIN_Unlock();
}


const CarData_t* CAN_MAIN_GetData(void) {
    /* NOTE: Caller must ensure proper synchronization when reading this structure.
     * The CAN2 data may be updated by ReadingsManageTask, so concurrent reads/writes
     * are possible. Use Mutex_FDCAN2_MAIN_Lock/Unlock around critical sections.
     *
     * For now, we return the pointer directly. Callers accessing this should:
     *   - Acquire Mutex_FDCAN2_MAIN_Lock()
     *   - Access the data
     *   - Call Mutex_FDCAN2_MAIN_Unlock()
     * OR use volatile reads if only reading primitives (no compound structures).
     */
    return &car_data;
}

CAN_RuntimeState_t CAN_MOTOR_GetState(void) {
    return g_can1_runtime_state;
}

CAN_RuntimeState_t CAN_MAIN_GetState(void) {
    return g_can2_runtime_state;
}

void CAN_SetRecoveryPeriodMs(uint32_t period_ms) {
    if (period_ms == 0U) {
        return;
    }
    s_can_recovery_period_ms = period_ms;
}

void CAN_ServiceRecovery(FDCAN_HandleTypeDef *hfdcan1, FDCAN_HandleTypeDef *hfdcan2, uint32_t now_ms) {
    if (hfdcan1 != NULL && g_can1_runtime_state == CAN_RUNTIME_INIT_FAILED) {
        if ((now_ms - s_can1_last_retry_ms) >= s_can_recovery_period_ms) {
            s_can1_last_retry_ms = now_ms;
            CAN_MOTOR_Init(hfdcan1);
        }
    }

    if (hfdcan2 != NULL && g_can2_runtime_state == CAN_RUNTIME_INIT_FAILED) {
        if ((now_ms - s_can2_last_retry_ms) >= s_can_recovery_period_ms) {
            s_can2_last_retry_ms = now_ms;
            CAN_MAIN_Init(hfdcan2);
        }
    }
}



// Executed by CAN interrupt for each message received on CAN Inverter-Motors
// ISR-SAFE: Extracts CAN messages and enqueues them. Processing happens in task context.
void CAN_MOTOR_ProcessRxMessages(FDCAN_HandleTypeDef *hfdcan) {
    if (hfdcan == NULL) return;

    FDCAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];

    // Drain all available messages in FIFO0 and enqueue them
    // (avoids overrun under burst traffic)
    while (HAL_FDCAN_GetRxFifoFillLevel(hfdcan, FDCAN_RX_FIFO0) > 0) {
        if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK) {
            /* Enqueue message for processing in task context */
            CAN_MOTOR_EnqueueRxMessage(rx_header.Identifier, rx_data, (uint8_t)(rx_header.DataLength >> 16));
        } else {
            break;
        }
    }
}


/**
 * @brief Process a single received CAN message (TASK-SAFE - called from CAN_MOTOR_ProcessQueueDrain)
 *        Mutexes are safe here (not in ISR context).
 */
static void process_motor_rx_message(uint32_t can_id, const uint8_t *data, uint8_t dlc) {
    if (data == NULL || dlc < 8U) return;

    uint8_t msg_id;     // which Msg arrived (1-5)
    Inverter_t *inv = find_inverter_by_rx_id(can_id, &msg_id);

    if (inv == NULL) return;

    /* Increment RX counter for diagnostics */
    g_can_rx_count++;

    /* Lock the appropriate inverter mutex BEFORE updating shared state */
    /* SAFE HERE: We are in task context, NOT inside ISR */
    Mutex_FDCAN1_MOTOR_Lock();

    Inverter_UpdateStatusMessage(inv, msg_id, data);

    if (msg_id == 1) {
        Inverter_UpdateState(inv);
    }

    /* Unlock inverter mutex */
    Mutex_FDCAN1_MOTOR_Unlock();
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
static inline uint32_t dlc_to_fdcan(uint8_t dlc) {
    return ((uint32_t)(dlc > 8U ? 8U : dlc)) << 16;   /* FDCAN_DLC_BYTES_x per frame classici (0-8 byte) */
}

/* TX FIFO/Queue del G4 = 3 elementi FISSI (non ingrandibile come su H7). Inviando
 * molti frame di fila (DataLogger) la coda si riempie e i frame in eccesso
 * verrebbero persi. Qui si aspetta uno slot libero, con timeout corto per non
 * bloccare mai il task (bus-off / bus muto -> si rinuncia al frame). */
#define TX_SLOT_WAIT_MS   2U   /* ~1-2 ms max: uno slot si libera in ~230 us @500k */

static bool transmit_can_message(FDCAN_HandleTypeDef *hfdcan, uint32_t can_id, const uint8_t *data, uint8_t dlc) {
    FDCAN_TxHeaderTypeDef tx_header = {0};

    tx_header.Identifier         = can_id;
    tx_header.IdType             = FDCAN_STANDARD_ID;
    tx_header.TxFrameType        = FDCAN_DATA_FRAME;
    tx_header.DataLength         = dlc_to_fdcan(dlc);
    tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    tx_header.BitRateSwitch      = FDCAN_BRS_OFF;
    tx_header.FDFormat           = FDCAN_CLASSIC_CAN;
    tx_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    tx_header.MessageMarker      = 0U;

    uint32_t t0 = HAL_GetTick();
    while ((HAL_FDCAN_GetTxFifoFreeLevel(hfdcan) == 0U) &&
           ((HAL_GetTick() - t0) < TX_SLOT_WAIT_MS)) {
        /* attesa breve che il bus svuoti uno slot */
    }

    HAL_StatusTypeDef status = HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &tx_header, (uint8_t*)data);
    return (status == HAL_OK);
}


bool CAN_MOTOR_TransmitCommand(FDCAN_HandleTypeDef *hfdcan, const Inverter_t *inv, const InverterCommandMsg1_t *cmd) {
    if (hfdcan == NULL || inv == NULL || cmd == NULL) return false;

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

    bool ok = transmit_can_message(hfdcan, inv->tx_can_id, tx_data, 8);
    return ok;
}

/**
 * @brief 0x002 / 0x003 — inverter error packet.
 *
 * Byte 0-1 : INV_ERROR_NUM   = error_code   [U16 LE]
 * Byte 2-5 : INV_ERROR_INFO1 = error_info_1 [S32 LE]
 */
void CAN_MAIN_TransmitInverterError(FDCAN_HandleTypeDef *hfdcan, uint8_t node_id,
                                    uint16_t error_code, uint32_t error_info_1)
{
    if (hfdcan == NULL) return;

    uint32_t can_id = (node_id == 1U) ? CAN_ID_TX_INV1_ERROR : CAN_ID_TX_INV2_ERROR;

    uint8_t tx_data[6];
    /* Byte 0-1: error_code (U16 LE) */
    pack_u16(&tx_data[0], error_code);
    /* Byte 2-5: error_info_1 (S32 LE) */
    tx_data[2] = (uint8_t)( error_info_1        & 0xFFU);
    tx_data[3] = (uint8_t)((error_info_1 >>  8) & 0xFFU);
    tx_data[4] = (uint8_t)((error_info_1 >> 16) & 0xFFU);
    tx_data[5] = (uint8_t)((error_info_1 >> 24) & 0xFFU);

    transmit_can_message(hfdcan, can_id, tx_data, 6U);
}

/**
 * @brief 0x006 — MCU general errors byte, DLC=1.
 *
 * bit0=INV1 Derating, bit1=INV2 Derating, bit2=APPs Implausibility,
 * bit3=Regen Active, bit4=APPs1 OOR, bit5=APPs2 OOR, bit6=SAS OOR, bit7=BS OOR
 */
void CAN_MAIN_TransmitErrors(FDCAN_HandleTypeDef *hfdcan, uint8_t errors_byte)
{
    if (hfdcan == NULL) return;
    transmit_can_message(hfdcan, CAN_ID_TX_MCU_ERRORS, &errors_byte, 1U);
}

/**
 * @brief 0x108 — APPs and SAS packet, DLC=4 (MCU -> DASH).
 *
 * Byte 0 : APPS_VALUE  [U8, 0-100 %]  pedale finale
 * Byte 1 : APP1_VALUE  [U8, 0-100 %]  canale APPS1
 * Byte 2 : APP2_VALUE  [U8, 0-100 %]  canale APPS2
 * Byte 3 : SAS_VALUE   [S8, gradi]    angolo volante
 *
 * La dash scarta il frame se DLC < 4 e interpreta il byte 3 come gradi.
 */
void CAN_MAIN_TransmitAppsSas(FDCAN_HandleTypeDef *hfdcan, uint8_t apps_pct,
                              uint8_t app1_pct, uint8_t app2_pct, int8_t sas_deg)
{
    if (hfdcan == NULL) return;

    uint8_t tx_data[4];
    tx_data[0] = apps_pct;
    tx_data[1] = app1_pct;
    tx_data[2] = app2_pct;
    tx_data[3] = (uint8_t)sas_deg;

    transmit_can_message(hfdcan, CAN_ID_TX_APPs_SAS, tx_data, 4U);
}

/**
 * @brief 0x250 / 0x251 — Inverter real-time data, DLC=8, Little-Endian.
 *
 * Byte 0-1 : INVx_STAT_WORD   = status_word        [U16 LE]
 * Byte 2-3 : INVx_ACT_SPEED   = speed_rpm          [S16 LE, rpm]
 * Byte 4-5 : INVx_MOTOR_TEMP  = motor_temp_degC    [S16 LE, 0.1°C]
 * Byte 6-7 : INVx_TEMP        = inverter_temp_degC [S16 LE, 0.1°C]
 */
void CAN_MAIN_TransmitInverterData(FDCAN_HandleTypeDef *hfdcan, uint8_t node_id,
                                   uint16_t status_word, int16_t speed_rpm,
                                   int16_t motor_temp_degc, int16_t inverter_temp_degc)
{
    if (hfdcan == NULL) return;

    uint32_t can_id = (node_id == 1U) ? CAN_ID_TX_INV1_DATA : CAN_ID_TX_INV2_DATA;

    uint8_t tx_data[8];
    pack_u16(&tx_data[0], status_word);
    pack_i16(&tx_data[2], speed_rpm);
    pack_i16(&tx_data[4], motor_temp_degc);
    pack_i16(&tx_data[6], inverter_temp_degc);

    transmit_can_message(hfdcan, can_id, tx_data, 8U);
}

bool CAN_MAIN_TransmitRaw(FDCAN_HandleTypeDef *hfdcan, uint32_t can_id, const uint8_t *data, uint8_t dlc)
{
    if (hfdcan == NULL || data == NULL || dlc > 8U) return false;
    return transmit_can_message(hfdcan, can_id, data, dlc);
}
