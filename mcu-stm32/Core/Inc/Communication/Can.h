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
#include "Drive/Inverter.h"
#include <stdint.h>
#include <stdbool.h>

/* AMK CAN ID DEFINITIONS CAN-1 (INVs)*/
#define CAN_ID_BASE_ADDRESS_TX1 0x0183U /**< Command Message 1 - Setpoint and control */
#define CAN_ID_BASE_ADDRESS_RX1 0x0282U /**< Status Message 1 - Main inverter status */
#define CAN_ID_BASE_ADDRESS_RX2 0x0284U /**< Status Message 2 - Temperatures and errors */
#define CAN_ID_BASE_ADDRESS_RX3 0x0286U /**< Status Message 3 - Phase U voltages and currents */
#define CAN_ID_BASE_ADDRESS_RX4 0x0288U /**< Status Message 4 - Phase V and W currents */

/* Helper to calculate CAN ID: CAN_ID = BASE_ADDRESS + node_id */
#define CAN_GET_TX_ID(node_id)      (CAN_ID_BASE_ADDRESS_TX1 + (node_id))
#define CAN_GET_RX1_ID(node_id)     (CAN_ID_BASE_ADDRESS_RX1 + (node_id))
#define CAN_GET_RX2_ID(node_id)     (CAN_ID_BASE_ADDRESS_RX2 + (node_id))
#define CAN_GET_RX3_ID(node_id)     (CAN_ID_BASE_ADDRESS_RX3 + (node_id))
#define CAN_GET_RX4_ID(node_id)     (CAN_ID_BASE_ADDRESS_RX4 + (node_id))



/*CAN IDs DEFINITIONS CAN-2 (CAR)*/
#define CAN_ID_IMU_ACC       0x100   // IMU - Accelerometer
#define CAN_ID_IMU_GYRO      0x101   // IMU - Gyroscope
#define CAN_ID_IMU_MAG       0x102   // IMU - Magnetometer
#define CAN_ID_BMS           0x103   // BMS
#define CAN_ID_ECU_RPM       0x104   // ECU - Wheel RPM
#define CAN_ID_ECU_BRAKE     0x105   // ECU - Brake Pressure
#define CAN_ID_DASH_R2D          0x106   // DASH - Engine Map

/* CAN2 TX IDs  */
#define CAN_ID_TX_INV1_ERROR    0x002U  /**< Inverter 1 error status  - sent only on error */
#define CAN_ID_TX_INV2_ERROR    0x003U  /**< Inverter 2 error status  - sent only on error */
#define CAN_ID_TX_MCU_ERRORS    0x006U  /**< MCU general errors byte  - sent every cycle   */
#define CAN_ID_TX_APPs_SAS      0x108U  /**< APPs and SAS % / +-% - sent every cycle  */
#define CAN_ID_TX_INV1_DATA     0x250U  /**< Inverter 1 real-time data - sent every cycle  */
#define CAN_ID_TX_INV2_DATA     0x251U  /**< Inverter 2 real-time data - sent every cycle  */

/* Bit masks for 0x006 MCU errors byte */
#define CAN_ERR_INV1_DERATING   (1U << 0)   /**< bit0: INV1 in Derating mode  */
#define CAN_ERR_INV2_DERATING   (1U << 1)   /**< bit1: INV2 in Derating mode  */
#define CAN_ERR_APPS_IMPLAUS    (1U << 2)   /**< bit2: APPs Implausibility    */
#define CAN_ERR_REGEN_ACTIVE    (1U << 3)   /**< bit3: Regen Active           */

/*Scaling macros CAN-2*/
#define CAN2_SCALE_FACTOR    1000.0f 
#define CAN_TIMEOUT_MS      10U     

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

#define IMU_ACCEL_SCALE     0.001f
#define IMU_GYRO_SCALE      0.1f
#define IMU_MAG_SCALE       0.01f
#define BMS_CURRENT_SCALE   0.01f

/*0x100 0x101 0x102*/
typedef struct {
    int16_t accel_x;        // m/s²
    int16_t accel_y;        // m/s²
    int16_t accel_z;        // m/s²
    uint16_t serial_num;    // 
    int16_t gyro_x;         // °/s²
    int16_t gyro_y;         // °/s²
    int16_t gyro_z;         // °/s²
    
    int16_t internal_temp;  // °C
    int16_t mag_x;          // uT
    int16_t mag_y;          // uT
    int16_t mag_z;          // uT
} IMU_t;

/*0x103*/
typedef struct {
    int8_t  soc;            // %
    int8_t  soh;            // %
    uint16_t max_discharge; // A
    uint16_t max_regen;     // A
} BMS_t;

/* 0x104 */
typedef struct {
    uint16_t rpm_fr;        // rpm
    uint16_t rpm_fl;        // rpm
} FWheels_RPM_t;

/*0x105*/
/*0x106*/

typedef struct {
    IMU_t         	imu_data;
    BMS_t         	bms_data;
    FWheels_RPM_t 	fwheels_rpm_data;
    uint8_t       	brake_pressure;
    uint8_t         r2d;            /**< Ready-to-Drive flag (byte 0 of 0x106) */
    int8_t			engine_map;     /**< Engine map selection (byte 1 of 0x106) */
} CarData_t;


/**
 * @brief Initialize CAN1 for motor inverter communication
 * 
 * Configures filters, starts CAN1 peripheral and enables receive interrupt.
 * @param hcan CAN handle (hcan1 for inverter, hcan2 for future use)
 */
void CAN_Inverter_Init(CAN_HandleTypeDef *hcan);

/**
 * @brief Initialize CAN RX message queue.
 *
 * Creates a FreeRTOS Queue for CAN messages enqueued by ISR.
 * Call this once after FreeRTOS kernel initializes but before CAN interrupts start.
 *
 * CRITICAL: This MUST be called before any CAN receipts.
 */
void CAN_ProcessQueue_Init(void);

bool CAN_QueuesReady(void);

/**
 * @brief Drain and process all pending CAN RX messages from the queue.
 *
 * Dequeues all available CAN messages and processes them (calling Inverter_UpdateStatusMessage, etc).
 * This must be called from a task context (NOT ISR) where FreeRTOS Mutexes are safe.
 * Typically call this from the readings_manage task in its main loop.
 *
 * Non-blocking: returns immediately if queue is empty.
 */
void CAN_ProcessQueueDrain(void);

/**
 * @brief Register an inverter (max 2 inverters supported)
 * @param inv Pointer to inverter structure
 */
void CAN_Inverter_Register(Inverter_t *inv);

/**
 * @brief Process inverter messages from CAN1
 * 
 * Called from CAN interrupt when messages arrive.
 * Automatically updates inverter structures.
 * @param hcan CAN handle that received messages (hcan1)
 */
void CAN_Inverter_ProcessRxMessages(CAN_HandleTypeDef *hcan);

/**
 * @brief Transmit command to motor inverter
 * 
 * @param hcan CAN handle to use (hcan1)
 * @param inv Pointer to inverter structure
 * @param cmd Complete command structure
 * @return true if transmission OK
 */
bool CAN_Inverter_TransmitCommand(CAN_HandleTypeDef *hcan, const Inverter_t *inv, const InverterCommandMsg1_t *cmd);

/**
 * @brief Initialize CAN2 for car data reception
 * @param hcan CAN handle (hcan2)
 */
void CAN_Car_Init(CAN_HandleTypeDef *hcan);

/**
 * @brief Initialize CAN2 RX message queue.
 *
 * Creates a FreeRTOS Queue for CAN2 messages enqueued by ISR.
 * Call this once after FreeRTOS kernel initializes but before CAN2 interrupts start.
 *
 * CRITICAL: This MUST be called before any CAN2 receipts.
 */
void CAN_Car_ProcessQueue_Init(void);

/**
 * @brief Drain and process all pending CAN2 RX messages from the queue.
 *
 * Dequeues all available CAN2 messages and processes them (updating car_data safely with mutex).
 * This must be called from a task context (NOT ISR) where FreeRTOS Mutexes are safe.
 * Typically call this from the readings_manage task in its main loop.
 *
 * Non-blocking: returns immediately if queue is empty.
 */
void CAN_Car_ProcessQueueDrain(void);

/**
 * @brief Process car data messages from CAN2 FIFO1
 * 
 * Called from CAN2 interrupt. Drains FIFO and enqueues messages for task-based processing.
 * ISR-safe: does not directly update car_data.
 * @param hcan CAN handle (hcan2)
 */
void CAN_Car_ProcessRxMessages(CAN_HandleTypeDef *hcan);

/**
 * @brief Get pointer to latest received car data
 * @return const pointer to CarData_t
 */
const CarData_t* CAN_Car_GetData(void);

/**
 * @brief Transmit inverter error packet on CAN2 (0x002 or 0x003), DLC=6.
 *
 * Layout (DBC, Little-Endian):
 *   Byte 0-1 : INV_ERROR_NUM   — error_code   [U16 LE]
 *   Byte 2-5 : INV_ERROR_INFO1 — error_info_1 [S32 LE]
 *
 * Call ONLY when the inverter is in INV_STATE_ERROR.
 *
 * @param hcan    CAN handle (hcan2)
 * @param node_id 1 = left INV1 (0x002), 2 = right INV2 (0x003)
 * @param inv     Pointer to inverter snapshot (must not be NULL)
 */
void CAN_Car_TransmitInverterError(CAN_HandleTypeDef *hcan, uint8_t node_id, const Inverter_t *inv);

/**
 * @brief Transmit MCU general errors byte on CAN2 (0x006), DLC=1.
 *
 * Layout (DBC):
 *   Byte 0 (4 bits): bit0=INV1 Derating, bit1=INV2 Derating,
 *                    bit2=APPs Implausibility, bit3=Regen Active
 *
 * Only transmit when at least one error bit is set.
 *
 * @param hcan        CAN handle (hcan2)
 * @param errors_byte Bitmask built with CAN_ERR_* macros
 */
void CAN_Car_TransmitErrors(CAN_HandleTypeDef *hcan, uint8_t errors_byte);

/**
 * @brief Transmit APPs and SAS packet on CAN2 (0x108), DLC=2.
 *
 * Layout (DBC):
 *   Byte 0 : APPS_VALUE  — accelerator pedal position [U8, 0-100 %]
 *   Byte 1 : SAS_VALUE   — steering angle normalised   [S8, -100..+100 %]
 *
 * @param hcan     CAN handle (hcan2)
 * @param apps_pct Accelerator pedal 0-100 %
 * @param sas_pct  Steering angle -100..+100 % (full-lock = ±100)
 */
void CAN_Car_TransmitAppsSas(CAN_HandleTypeDef *hcan, uint8_t apps_pct, int8_t sas_pct);

/**
 * @brief Transmit inverter real-time data on CAN2 (0x250 / 0x251), DLC=8.
 *
 * Layout (DBC, Little-Endian):
 *   Byte 0-1 : INVx_STAT_WORD   — status_word         [U16 LE]
 *   Byte 2-3 : INVx_ACT_SPEED   — speed_rpm           [S16 LE, rpm]
 *   Byte 4-5 : INVx_MOTOR_TEMP  — motor_temp_degC     [S16 LE, 0.1°C]
 *   Byte 6-7 : INVx_TEMP        — inverter_temp_degC  [S16 LE, 0.1°C]
 *
 * @param hcan    CAN handle (hcan2)
 * @param node_id 1 = INV1 (0x250), 2 = INV2 (0x251)
 * @param inv     Pointer to inverter snapshot (must not be NULL)
 */
void CAN_Car_TransmitInverterData(CAN_HandleTypeDef *hcan, uint8_t node_id, const Inverter_t *inv);

CAN_RuntimeState_t CAN_GetCan1State(void);
CAN_RuntimeState_t CAN_GetCan2State(void);
void CAN_ServiceRecovery(CAN_HandleTypeDef *hcan1, CAN_HandleTypeDef *hcan2, uint32_t now_ms);
void CAN_SetRecoveryPeriodMs(uint32_t period_ms);

#ifdef __cplusplus
}
#endif

#endif /* CAN_H */
