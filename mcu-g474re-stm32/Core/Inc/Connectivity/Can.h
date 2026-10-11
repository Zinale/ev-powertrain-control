/**
 * @file Can.h
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief CAN communication module for AMK motor inverter (CAN1)
 * 
 * This module handles CAN1 communication dedicated to motor inverters.
 * Prepared for future CAN2 integration (telemetry/other function).
 */

#ifndef CAN_H
#define CAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "Control/inverter.h"
#include <stdint.h>
#include <stdbool.h>

/* AMK CAN ID DEFINITIONS CAN-1-MOTOR (INVs)*/
#define CAN_ID_BASE_ADDRESS_TX1 0x0183U 
#define CAN_ID_BASE_ADDRESS_RX1 0x0282U 
#define CAN_ID_BASE_ADDRESS_RX2 0x0284U 
#define CAN_ID_BASE_ADDRESS_RX3 0x0286U 
#define CAN_ID_BASE_ADDRESS_RX4 0x0288U 
#define CAN_ID_BASE_ADDRESS_RX5 0x0290U 


/* Helper to calculate CAN ID: CAN_ID = BASE_ADDRESS + node_id */
#define CAN_GET_TX_ID(node_id)      (CAN_ID_BASE_ADDRESS_TX1 + (node_id))
#define CAN_GET_RX1_ID(node_id)     (CAN_ID_BASE_ADDRESS_RX1 + (node_id))
#define CAN_GET_RX2_ID(node_id)     (CAN_ID_BASE_ADDRESS_RX2 + (node_id))
#define CAN_GET_RX3_ID(node_id)     (CAN_ID_BASE_ADDRESS_RX3 + (node_id))
#define CAN_GET_RX4_ID(node_id)     (CAN_ID_BASE_ADDRESS_RX4 + (node_id))
#define CAN_GET_RX5_ID(node_id)     (CAN_ID_BASE_ADDRESS_RX5 + (node_id))


/*CAN IDs ERRORS FRAME CAN2-MAIN (CAR)*/
#define CAN_ID_BMS_ERROR    0x001U
#define CAN_ID_PDM1_ERROR   0x005U
#define CAN_ID_PDM2_ERROR   0x004U

/*CAN IDs DEFINITIONS CAN-2-MAIN (CAR)*/
#define CAN_ID_IMU_ACC                      0x100   // IMU - Accelerometer
#define CAN_ID_IMU_GYRO                     0x101   // IMU - Gyroscope
#define CAN_ID_IMU_MAG                      0x102   // IMU - Magnetometer
#define CAN_ID_BMS                          0x103   // BMS
#define CAN_ID_DASH_R2D_ENGINE_MAP          0x106   // DASH - Engine Map & Parameters
#define CAN_ID_SDC_STATUS                  0x107   // SDC - Status
#define CAN_ID_CAB500_CURRENT               0x3C2  // LEM CAB 500 - corrente reale di pacco (S32 mA, offset 0x80000000, BE) @100Hz

/* --- Aviorace IMU (fw 2.09) - configurazione via CAN --------------------- */
#define IMU_CAN_ID_REPROGRAM    0x0E5U   /* TX: messaggi di (ri)configurazione   */
#define IMU_CAN_ID_INFO_REPLY   0x0E4U   /* RX: risposta a "Get IMU information" */

/* Comandi (Byte1 del frame 0x0E5, con Byte0 = 0x01) */
#define IMU_CMD_SET_RANGES      0xA0U    /* CAN id/baud + acc/gyro range         */
#define IMU_CMD_ZERO_OFFSET     0xA1U
#define IMU_CMD_GET_INFO        0xA2U
#define IMU_CMD_SET_POS_FREQ    0xA3U    /* positioning + cutoff acc/gyro        */

/* Valori attesi al boot */
#define IMU_EXPECT_ACC_RANGE    0x02U    /* +-12 G   */
#define IMU_EXPECT_GYRO_RANGE   0x01U    /* +-250 dps */
#define IMU_EXPECT_ACC_FREQ     0x01U    /* 19 Hz cutoff */
#define IMU_EXPECT_GYRO_FREQ    0x01U    /* 32 Hz cutoff */
#define IMU_EXPECT_POSITIONING  0x00U    /* TODO: confermare orientamento di montaggio (0x00..0x07) */
#define IMU_BASE_CAN_ID         CAN_ID_IMU_ACC   /* base-id attuale (0x100) */
/* Baud: FDCAN2 e' configurato a 500 kbps (vedi MX_FDCAN2_Init: presc.17, 20tq
 * @170MHz). L'IMU di default e' a 1 Mbps -> su questa vettura e' gia' stata
 * riprogrammata a 500 kbps. Usato solo dal path AUTOCONFIG. */
#define IMU_EXPECT_BAUD         0x01U    /* 0x00=250k 0x01=500k 0x02=1M */

/* Procedura di boot */
#define IMU_BOOT_REPLY_TIMEOUT_MS  300U  /* attesa risposta per singolo tentativo */
#define IMU_BOOT_RETRIES             4U  /* -> ~1.2 s max se l'IMU e' assente/lenta al power-up */
#define IMU_BOOT_RESET_WAIT_MS    3000U  /* attesa dopo ogni scrittura config (auto-reset IMU) */
#define IMU_BOOT_AUTOCONFIG         0U   /* 1 = riscrive la config se non combacia (banco); 0 = solo verifica */

typedef enum {
    IMU_BOOT_UNKNOWN = 0,
    IMU_BOOT_OK,             /* risposta ricevuta, configurazione coerente          */
    IMU_BOOT_MISMATCH,       /* risposta ricevuta, parametri diversi da quelli attesi */
    IMU_BOOT_RECONFIGURED,   /* config riscritta e IMU ripartita                    */
    IMU_BOOT_NO_RESPONSE     /* nessuna risposta dopo i retry                       */
} ImuBootStatus_t;

extern volatile ImuBootStatus_t g_imu_boot_status;
extern volatile uint16_t        g_imu_fw_version;   /* [0.01], es. 209 = v2.09 */

/* CAN2 TX IDs  */
#define CAN_ID_TX_INV1_ERROR      0x002U  /**< Inverter 1 (nodo 1 = DESTRO) error status - sent every cycle */
#define CAN_ID_TX_INV2_ERROR      0x003U  /**< Inverter 2 (nodo 2 = SINISTRO) error status - sent every cycle */
#define CAN_ID_TX_MCU_ERRORS      0x006U  /**< MCU general errors byte  - sent every cycle   */
#define CAN_ID_TX_MCU_PLAUSIBILITY 0x007U /**< Dettaglio bitmask implausibilità (U16 BE) - vedi PlausibilityFault_t */
#define CAN_ID_TX_MCU_WHEELS    0x104   /**< DLC8: [0-1]WheelFR [2-3]WheelFL [4-5]CarSpeed (S16 km/h*10 BE) [6]FSM state [7] bit0=R2D armato, bit1=R2D ATTIVO (RUNNING) -> DASH/PDM */
#define CAN_ID_TX_MCU_BRAKE     0x105   /**< Brake pressure front [bar] + brake-light state -> DASH/PDM1 */
#define WHEEL_SPEED_CAN_SCALE   10.0f   /* km/h -> unita' CAN 0x104 (DBC factor 0.1) */
#define CAN_ID_TX_APPs_SAS      0x108U  /**< APPs and SAS % / +-% - sent every cycle  */
#define CAN_ID_TX_INV1_DATA     0x250U  /**< Inverter 1 real-time data - sent every cycle  */
#define CAN_ID_TX_INV2_DATA     0x251U  /**< Inverter 2 real-time data - sent every cycle  */
#define CAN_ID_TX_CTRL_DEBUG_1  0x109U  /**< Debug controlli: Tq cmd RL/RR, Yaw Th, Regen/TVC, Throttle - 10 Hz */
#define CAN_ID_TX_CTRL_DEBUG_2  0x110U  /**< Debug controlli: Lambda FL/FR/RL/RR, K-slip RL/RR - 10 Hz */
#define CAN_ID_TX_CTRL_DEBUG_3  0x111U  /**< Debug controlli: Grip Estimator T_RL/RR, T_act RL/RR - 10 Hz */

/* Bit masks for 0x006 MCU errors byte (tabella interfaccia CAN: bit6 = SAS, bit7 = BS) */
#define CAN_ERR_INV1_DERATING   (1U << 0)   /**< bit0: INV1 (destro) in Derating mode  */
#define CAN_ERR_INV2_DERATING   (1U << 1)   /**< bit1: INV2 (sinistro) in Derating mode */
#define CAN_ERR_APPS_IMPLAUS    (1U << 2)   /**< bit2: APPs Implausibility    */
#define CAN_ERR_REGEN_ACTIVE    (1U << 3)   /**< bit3: Regen Active           */
#define CAN_ERR_APPS1_OOR       (1U << 4)   /**< bit4: APPs1 Out of range     */
#define CAN_ERR_APPS2_OOR       (1U << 5)   /**< bit5: APPs2 Out of range     */
#define CAN_ERR_SAS_OOR         (1U << 6)   /**< bit6: SAS Out of range       */
#define CAN_ERR_BRAKE_OOR       (1U << 7)   /**< bit7: BS (Brake) Out of range */

/*Scaling macros CAN-2*/
#define CAN_TIMEOUT_MS      1000U     

/* =============================================================================
 *  CAN1 SCE diagnostics — leggibili dalle task (scritti solo dall'ISR)
 * ============================================================================= */
extern volatile uint32_t g_can1_sce_esr;    /**< ESR snapshot al momento dell'errore */
extern volatile uint8_t  g_can1_sce_tec;    /**< Transmit Error Counter all'errore   */
extern volatile uint8_t  g_can1_sce_rec;    /**< Receive Error Counter all'errore    */
extern volatile uint8_t  g_can1_busoff;     /**< 1 = bus-off attivo (reset da task)  */
extern volatile uint32_t g_can1_sce_count;  /**< Totale eventi SCE dall'avvio        */

typedef enum {
    CAN_RUNTIME_UNINITIALIZED = 0U,
    CAN_RUNTIME_READY         = 1U,
    CAN_RUNTIME_INIT_FAILED   = 2U
} CAN_RuntimeState_t;

extern volatile CAN_RuntimeState_t g_can1_runtime_state;
extern volatile CAN_RuntimeState_t g_can2_runtime_state;
extern volatile uint32_t g_can1_last_error;
extern volatile uint32_t g_can2_last_error;
extern volatile uint32_t g_can_rx_count;    /**< Contatore totale messaggi CAN1 processati */
extern volatile uint32_t g_can2_rx_count;   /**< Contatore totale messaggi CAN2 (main) processati */

#define IMU_ACCEL_SCALE     0.001f
#define IMU_GYRO_SCALE      0.1f
#define IMU_MAG_SCALE       0.01f

/*0x100 0x101 0x102*/
typedef struct {
    int16_t accel_x;        // g
    int16_t accel_y;        // g
    int16_t accel_z;        // g

    int16_t gyro_x;         // °/s
    int16_t gyro_y;         // °/s
    int16_t gyro_z;         // °/s  
    
    int16_t mag_x;          // uT
    int16_t mag_y;          // uT
    int16_t mag_z;          // uT

    uint32_t last_update_timestamp;  // Timestamp of the last update
} IMU_t;

/*0x103*/
typedef struct {
    int8_t  soc;            // %
    int8_t  soh;            // %
    uint16_t max_discharge; // A
    uint16_t max_regen;     // A

    bool SDC_status;       // SDC status flag, true if closed, false if open
    bool bms_error;         // BMS error flag

    /* Corrente reale di pacco - sensore LEM CAB 500 su 0x3C2 (non dal BMS) */
    float    pack_current_a;         // + = scarica (dopo normalizzazione segno)
    uint32_t cab500_last_update;     // timestamp ultimo 0x3C2

    uint32_t last_update_timestamp;  // Timestamp of the last update
} BMS_t;

/*0x106*/
typedef struct {
    uint8_t r2d;            // Ready-to-Drive flag
    int8_t  engine_map;     // Engine map selection
    uint8_t parameters_config;  // Parameters configuration byte (Each 2 bits represent a different parameter, so 4 parameters can be configured in 4 different ways: 00 = Option 0, 01 = Option 1, 10 = Option 2, 11 = Option 3)
    uint32_t last_update_timestamp;  // Timestamp of the last update
} Dashboard_t;

typedef struct {
    bool PDM1_error;       // PDM1 error flag
    bool PDM2_error;       // PDM2 error flag

    uint32_t last_update_timestamp;  // Timestamp of the last update
} PDM_t;

typedef struct {
    IMU_t         	imu_data;
    BMS_t         	bms_data;
    Dashboard_t     dashboard;      /**< Dashboard data (bytes 0-2 of 0x106) */

    PDM_t pdm_data;        /**< PDM data (PDM1 and PDM2 error flags and timestamp) */
} CarData_t;


/**
 * @brief Initialize CAN1/FDCAN1 (bus motori) for motor inverter communication
 * 
 * Configures filters, starts FDCAN1 peripheral and enables receive interrupt.
 * @param hfdcan FDCAN handle (hfdcan1 for inverter)
 */
void CAN_MOTOR_Init(FDCAN_HandleTypeDef *hfdcan);

/**
 * @brief Initialize CAN1/FDCAN1 (bus motori) RX message queue.
 *
 * Creates a FreeRTOS Queue for CAN messages enqueued by ISR.
 * Call this once after FreeRTOS kernel initializes but before CAN interrupts start.
 *
 * CRITICAL: This MUST be called before any CAN receipts.
 */
void CAN_MOTOR_ProcessQueue_Init(void);

bool CAN_QueuesReady(void);

/**
 * @brief Drain and process all pending CAN1/FDCAN1 (bus motori) RX messages from the queue.
 *
 * Dequeues all available CAN messages and processes them (calling Inverter_UpdateStatusMessage, etc).
 * This must be called from a task context (NOT ISR) where FreeRTOS Mutexes are safe.
 * Typically call this from the readings_manage task in its main loop.
 *
 * Non-blocking: returns immediately if queue is empty.
 */
void CAN_MOTOR_ProcessQueueDrain(void);

/**
 * @brief Register an inverter (max 2 inverters supported)
 * @param inv Pointer to inverter structure
 */
void CAN_MOTOR_Register(Inverter_t *inv);

/**
 * @brief Process inverter messages from CAN1/FDCAN1 (bus motori)
 * 
 * Called from CAN interrupt when messages arrive.
 * Automatically updates inverter structures.
 * @param hfdcan FDCAN handle that received messages (hfdcan1)
 */
void CAN_MOTOR_ProcessRxMessages(FDCAN_HandleTypeDef *hfdcan);

/**
 * @brief Transmit command to motor inverter
 * 
 * @param hfdcan FDCAN handle to use (hfdcan1)
 * @param inv Pointer to inverter structure
 * @param cmd Complete command structure
 * @return true if transmission OK
 */
bool CAN_MOTOR_TransmitCommand(FDCAN_HandleTypeDef *hfdcan, const Inverter_t *inv, const InverterCommandMsg1_t *cmd);

/**
 * @brief Initialize CAN2/FDCAN2 (bus main/car data) for car data reception.
 *        = CAN_MAIN_Start() + attivazione notifiche RX. Da chiamare dopo la
 *        creazione della coda RX (kernel avviato).
 * @param hfdcan FDCAN handle (hfdcan2)
 */
void CAN_MAIN_Init(FDCAN_HandleTypeDef *hfdcan);

/**
 * @brief Avvio "hardware" di FDCAN2: filtri + HAL_FDCAN_Start, SENZA notifiche.
 *        Idempotente (se gia' avviato non fa nulla). Utilizzabile a bare-metal
 *        (prima del kernel) per la procedura di boot IMU in polling.
 * @return true se FDCAN2 e' avviato e pronto per TX/RX in polling.
 */
bool CAN_MAIN_Start(FDCAN_HandleTypeDef *hfdcan);

/**
 * @brief Procedura di boot IMU (Aviorace, via CAN2, polling).
 *
 * Interroga l'IMU ("Get info", 0x0E5/0x0E4), verifica range/cutoff/posizione
 * rispetto ai valori attesi e, se IMU_BOOT_AUTOCONFIG=1, riscrive la config
 * (l'IMU si auto-resetta ad ogni scrittura). Aggiorna g_imu_boot_status /
 * g_imu_fw_version. NON blocca l'avvio in caso di IMU assente: ritorna lo stato.
 *
 * Richiede FDCAN2 gia' avviato (CAN_MAIN_Start) e coda RX NON ancora attiva
 * (nessuna notifica): svuota la FIFO0 in polling.
 *
 * @param hiwdg  watchdog da rinfrescare durante l'attesa (NULL per saltare).
 */
ImuBootStatus_t CAN_MAIN_ImuBoot(FDCAN_HandleTypeDef *hfdcan, IWDG_HandleTypeDef *hiwdg);

/**
 * @brief Initialize CAN2/FDCAN2 (bus main/car data) RX message queue.
 *
 * Creates a FreeRTOS Queue for CAN2 messages enqueued by ISR.
 * Call this once after FreeRTOS kernel initializes but before CAN2 interrupts start.
 *
 * CRITICAL: This MUST be called before any CAN2 receipts.
 */
void CAN_MAIN_ProcessQueue_Init(void);

/**
 * @brief Drain and process all pending CAN2/FDCAN2 (bus main/car data) RX messages from the queue.
 *
 * Dequeues all available CAN2 messages and processes them (updating car_data safely with mutex).
 * This must be called from a task context (NOT ISR) where FreeRTOS Mutexes are safe.
 * Typically call this from the readings_manage task in its main loop.
 *
 * Non-blocking: returns immediately if queue is empty.
 */
void CAN_MAIN_ProcessQueueDrain(void);

/**
 * @brief Process car data messages from CAN2/FDCAN2 (bus main/car data)
 * 
 * Called from CAN2 interrupt. Drains FIFO and enqueues messages for task-based processing.
 * ISR-safe: does not directly update car_data.
 * @param hfdcan FDCAN handle (hfdcan2)
 */
void CAN_MAIN_ProcessRxMessages(FDCAN_HandleTypeDef *hfdcan);

/**
 * @brief Get pointer to latest received car data
 * @return const pointer to CarData_t
 */
const CarData_t* CAN_MAIN_GetData(void);

/**
 * @brief Transmit inverter error packet on CAN2 (0x002 or 0x003), DLC=6.
 *
 * Layout (DBC, Little-Endian):
 *   Byte 0-1 : INV_ERROR_NUM   — error_code   [U16 LE]
 *   Byte 2-5 : INV_ERROR_INFO1 — error_info_1 [S32 LE]
 *
 * Trasmesso a ogni ciclo (error_code = 0 quando l'inverter non e' in fault).
 * I valori vengono dallo snapshot di VehicleData_t, non dalle strutture globali.
 *
 * @param hfdcan      FDCAN handle (hfdcan2)
 * @param node_id     1 = INV1 = inverter DESTRO (0x002), 2 = INV2 = SINISTRO (0x003)
 * @param error_code  Codice errore inverter
 * @param error_info_1 Dettaglio errore (error_info_1)
 */
void CAN_MAIN_TransmitInverterError(FDCAN_HandleTypeDef *hfdcan, uint8_t node_id,
                                    uint16_t error_code, uint32_t error_info_1);

/**
 * @brief Transmit MCU general errors byte on CAN2 (0x006), DLC=1.
 *
 * Layout (tabella interfaccia CAN), vedi maschere CAN_ERR_*:
 *   bit0=INV1 Derating, bit1=INV2 Derating, bit2=APPs Implausibility,
 *   bit3=Regen Active (conferma dagli inverter), bit4=APPs1 OOR,
 *   bit5=APPs2 OOR, bit6=SAS OOR, bit7=BS OOR
 *
 * Trasmesso a ogni ciclo, anche con tutti i bit a zero.
 *
 * @param hfdcan        FDCAN handle (hfdcan2)
 * @param errors_byte Bitmask built with CAN_ERR_* macros
 */
void CAN_MAIN_TransmitErrors(FDCAN_HandleTypeDef *hfdcan, uint8_t errors_byte);

/**
 * @brief Transmit APPs and SAS packet on CAN2 (0x108), DLC=4 -> DASH.
 *
 * Layout (tabella interfaccia CAN):
 *   Byte 0 : APPS_VALUE — final accelerator pedal position [U8, 0-100 %]
 *   Byte 1 : APP1_VALUE — APPS1 channel                    [U8, 0-100 %]
 *   Byte 2 : APP2_VALUE — APPS2 channel                    [U8, 0-100 %]
 *   Byte 3 : SAS_VALUE  — steering wheel angle             [S8, gradi]
 *
 * @param hfdcan    FDCAN handle (hfdcan2)
 * @param apps_pct  Final accelerator pedal 0-100 %
 * @param app1_pct  APPS1 0-100 %
 * @param app2_pct  APPS2 0-100 %
 * @param sas_deg   Steering wheel angle in degrees (saturated to -128..+127)
 */
void CAN_MAIN_TransmitAppsSas(FDCAN_HandleTypeDef *hfdcan, uint8_t apps_pct,
                              uint8_t app1_pct, uint8_t app2_pct, int8_t sas_deg);

/**
 * @brief Transmit inverter real-time data on CAN2 (0x250 / 0x251), DLC=8.
 *
 * Layout (DBC, Little-Endian):
 *   Byte 0-1 : INVx_STAT_WORD   — status_word         [U16 LE]
 *   Byte 2-3 : INVx_ACT_SPEED   — speed_rpm           [S16 LE, rpm]
 *   Byte 4-5 : INVx_MOTOR_TEMP  — motor_temp_degC     [S16 LE, 0.1°C]
 *   Byte 6-7 : INVx_TEMP        — inverter_temp_degC  [S16 LE, 0.1°C]
 *
 * I valori vengono dallo snapshot di VehicleData_t, non dalle strutture globali.
 *
 * @param hfdcan              FDCAN handle (hfdcan2)
 * @param node_id             1 = INV1 = DESTRO (0x250), 2 = INV2 = SINISTRO (0x251)
 * @param status_word         Parola di stato inverter
 * @param speed_rpm           Velocita' motore [rpm]
 * @param motor_temp_degc     Temperatura motore
 * @param inverter_temp_degc  Temperatura inverter
 */
void CAN_MAIN_TransmitInverterData(FDCAN_HandleTypeDef *hfdcan, uint8_t node_id,
                                   uint16_t status_word, int16_t speed_rpm,
                                   int16_t motor_temp_degc, int16_t inverter_temp_degc);

/**
 * @brief Invio generico di un frame dati sul bus MAIN (FDCAN2).
 * @param can_id  identificatore standard
 * @param data    payload (max 8 byte), gia' impacchettato dal chiamante
 * @param dlc     lunghezza payload [0..8]
 * @return true se accodato in TX FIFO
 */
bool CAN_MAIN_TransmitRaw(FDCAN_HandleTypeDef *hfdcan, uint32_t can_id, const uint8_t *data, uint8_t dlc);

CAN_RuntimeState_t CAN_MOTOR_GetState(void);
CAN_RuntimeState_t CAN_MAIN_GetState(void);
void CAN_ServiceRecovery(FDCAN_HandleTypeDef *hfdcan1, FDCAN_HandleTypeDef *hfdcan2, uint32_t now_ms);
void CAN_SetRecoveryPeriodMs(uint32_t period_ms);

#ifdef __cplusplus
}
#endif

#endif /* CAN_H */
