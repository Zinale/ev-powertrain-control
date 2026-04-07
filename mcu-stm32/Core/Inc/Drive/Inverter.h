/**
 * @file Inverter.h
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief AMK Motor Inverter Control Module
 * 
 * Handles communication and control of the AMK inverter.
 * Implements the state machine for inverter startup and management.
 */

#ifndef INVERTER_H
#define INVERTER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Control Word Bits (TX) */
#define INV_CTRL_INVERTER_ON    (1U << 8)   /**< bit 8: Enable controller */
#define INV_CTRL_DC_ON          (1U << 9)   /**< bit 9: Activate high voltage */
#define INV_CTRL_ENABLE         (1U << 10)  /**< bit 10: Enable driver */
#define INV_CTRL_ERROR_RESET    (1U << 11)  /**< bit 11: Error reset */

/* Status Word Bits (RX) */
#define INV_STAT_SYSTEM_READY   (1U << 8)   /**< bit 8: System ready */
#define INV_STAT_ERROR          (1U << 9)   /**< bit 9: Error active */
#define INV_STAT_WARN           (1U << 10)  /**< bit 10: Warning active */
#define INV_STAT_QUIT_DC_ON     (1U << 11)  /**< bit 11: DC_ON confirmation */
#define INV_STAT_DC_ON          (1U << 12)  /**< bit 12: DC bus active */
#define INV_STAT_QUIT_INV_ON    (1U << 13)  /**< bit 13: Inverter ON confirmation */
#define INV_STAT_INVERTER_ON    (1U << 14)  /**< bit 14: Inverter active */
#define INV_STAT_DERATING       (1U << 15)  /**< bit 15: Derating active */

#define TORQUE_SCALE_FACTOR (0.001 * MOTOR_NOMINAL_TORQUE_NM)       //Rx MSG1: Torque value is in 0.1% of nominal torque, convert to Nm for logging and control calculations
#define Iq_SCALE_FACTOR (107.2f / 16384.0f)  //Rx MSG1: Torque current 
#define Id_SCALE_FACTOR (107.2f / 16384.0f)  //Rx MSG3: Magnetizing current
/**
 * @brief Command Message 1 (TX1) - Commands and setpoints
 * CAN ID: 0x184 (Inverter 1), DLC: 8 bytes
 */
typedef struct {
    uint16_t control_word;         /**< Control Word */
    int16_t  torque_setpoint;      /**< Torque setpoint [0.1% Mn] */
    int16_t  torque_limit_pos;     /**< Positive torque limit [0.1% Mn] */
    int16_t  torque_limit_neg;     /**< Negative torque limit [0.1% Mn] */
} InverterCommandMsg1_t;

/** Timeout with no CAN packet before the inverter is considered OFF [ms] */
#define INV_TIMEOUT_OFF_MS   10000U

/** Timeout in HV_ACTIVE state before forcing transition to READY [ms] — AMK DC pre-charge timeout */
#define INV_TIMEOUT_HV_ACTIVE_MS  3000U

/**
 * @brief Inverter state machine states
 */
typedef enum {
    INV_STATE_OFF      = -1,       /**< No CAN communication detected           */
    INV_STATE_IDLE     =  0,       /**< Idle - No active command                */
    INV_STATE_LV_ACTIVE,           /**< Low Voltage active - System ready       */
    INV_STATE_HV_ACTIVE,           /**< High Voltage active - DC bus charged    */
    INV_STATE_READY,               /**< Ready - Waiting controller enable       */
    INV_STATE_RUNNING,             /**< Running - Controller active             */
    INV_STATE_ERROR                /**< Error - Reset required                  */
} InverterState_t;

/**
 * @brief Main inverter structure
 */
typedef struct {
    uint8_t node_id;                        
    InverterState_t state;                  
    InverterState_t previous_state;        
    int16_t torque_request;
    uint32_t state_entry_time_ms;          /**< Timestamp when current state was entered [ms] */

    /* Real-time data extracted from CAN messages */
    uint16_t status_word;              /**< Status word (from SM1) */
    int16_t  speed_rpm;                /**< Motor speed [RPM] (from SM1) */
    int16_t  torque_value;           /**< Torque value [Nm] (from SM1) */
    int16_t  raw_torque_current;      /**< Id: Raw torque current (from SM1) */
    int16_t  raw_magnetizing_current;  /**< Iq: Raw magnetizing current (from SM3)*/
    
    int16_t  motor_temp_degC;          /**< Motor temperature [0.1°C] (from SM2) */
    int16_t  inverter_temp_degC;       /**< ColdPlate temperature [0.1°C] (from SM2) */
    uint16_t error_code;               /**< AMK error code (from SM2) */
    int16_t  igbt_temp_degC;           /**< IGBT temperature [0.1°C] (from SM2) */
    
    uint16_t dc_bus_voltage;           /**< DC bus voltage [V] (from SM3) */
    int32_t  phase_u_current;          /**< Phase U current [mA] (from SM3) */
    
    int32_t  phase_v_current;          /**< Phase V current [mA] (from SM4) */
    int32_t  phase_w_current;          /**< Phase W current [mA] (from SM4) */
    
    uint32_t error_info_1;             /**< Error Info 1 (from SM5) */
    uint32_t actual_power;             /**< Actual power [W] (from SM5) */
    
    uint32_t last_rx_timestamp_ms;
    
    uint32_t tx_can_id;      
    uint32_t rx1_can_id;     
    uint32_t rx2_can_id;     
    uint32_t rx3_can_id;     
    uint32_t rx4_can_id;
    uint32_t rx5_can_id;     /**< Status Message 5: 0x290 + node_id */
} Inverter_t;

/* Canonical node IDs used across CAN RX/TX and task mutex mapping. */
#define INVERTER_RIGHT_NODE_ID 1U
#define INVERTER_LEFT_NODE_ID  2U


extern Inverter_t g_inverter_left;   /**< Left inverter  - node_id=2 */
extern Inverter_t g_inverter_right;  /**< Right inverter - node_id=1 */

/**
 * @brief Motor drive state — updated every DataLogger cycle (10 Hz).
 *
 * Encoded in CAN_ERR_REGEN_ACTIVE (bit 3 of 0x006).
 */
typedef enum
{
    MOTOR_STATE_IDLE  = 0, /**< No torque request or stopped    */
    MOTOR_STATE_DRIVE = 1, /**< Positive torque (traction)      */
    MOTOR_STATE_REGEN = 2, /**< Negative torque (regen braking) */
} MotorDriveState_t;

/**
 * @brief Return the motor drive state computed during the last DataLogger cycle.
 */
MotorDriveState_t DataLogger_GetMotorState(void);


/**
 * @brief Initialize the inverter
 * @param inv Pointer to inverter structure
 * @param node_id Node ID (1-4)
 */
void Inverter_Init(Inverter_t *inv, uint8_t node_id);

/**
 * @brief Initialize the inverters
 */
void Inverters_Init();

/**
 * @brief Update inverter state based on status word
 * @param inv Pointer to inverter structure
 * @param timestamp_ms Current timestamp in milliseconds
 */
void Inverter_UpdateState(Inverter_t *inv);

/**
 * @brief Update telemetry data (SM2, SM3, SM4, SM5)
 * @param inv Pointer to inverter structure
 * @param msg_id Message ID (2, 3, 4, or 5)
 * @param data Pointer to data (8 bytes)
 */
void Inverter_UpdateStatusMessage(Inverter_t *inv, uint8_t msg_id, const uint8_t *data);

/**
 * @brief Build command to send based on state
 * @param inv Pointer to inverter structure
 * @param torque_setpoint Torque setpoint [0.1% Mn]
 * @param torque_limit_pos Positive torque limit [0.1% Mn]
 * @param torque_limit_neg Negative torque limit [0.1% Mn]
 * @return Command structure to transmit
 */
InverterCommandMsg1_t Inverter_BuildCommand(const Inverter_t *inv, 
                                             int16_t torque_setpoint,
                                             int16_t torque_limit_pos,
                                             int16_t torque_limit_neg);

/**
 * @brief Check for errors
 * @param inv Pointer to inverter structure
 * @return true if error present
 */
bool Inverter_HasError(const Inverter_t *inv);

/**
 * @brief Check for warnings
 * @param inv Pointer to inverter structure
 * @return true if warning present
 */
bool Inverter_HasWarning(const Inverter_t *inv);

/**
 * @brief Check communication timeout
 * @param inv Pointer to inverter structure
 * @param current_time_ms Current timestamp [ms]
 * @param timeout_ms Timeout in milliseconds
 * @return true if communication timed out
 */
bool Inverter_CheckTimeout(Inverter_t *inv, uint32_t current_time_ms, uint32_t timeout_ms);

/**
 * @brief Transition inverter to INV_STATE_OFF if no CAN packet has been
 *        received for INV_TIMEOUT_OFF_MS milliseconds.
 *
 * Must be called every control cycle (inside the inverter mutex).
 * Has no effect if the inverter is already in INV_STATE_OFF.
 *
 * @param inv Pointer to inverter structure
 */
void Inverter_UpdateOffTimeout(Inverter_t *inv);

/**
 * @brief Force transition from HV_ACTIVE to READY after timeout.
 *
 * AMK inverters sometimes get stuck in HV_ACTIVE while waiting for
 * internal DC bus pre-charge to complete. After INV_TIMEOUT_HV_ACTIVE_MS,
 * if no error is present, force the transition to READY.
 *
 * Must be called every control cycle (inside the inverter mutex).
 *
 * @param inv Pointer to inverter structure
 * @param current_time_ms Current system time [ms]
 */
void Inverter_UpdateHVTransitionTimeout(Inverter_t *inv, uint32_t current_time_ms);

/**
 * @brief Get current state name (for debug)
 * @param state State to convert
 * @return String with state name
 */
const char* Inverter_GetStateName(InverterState_t state);

#ifdef __cplusplus
}
#endif

/* BaseControlMotor uses InverterState_t / Inverter_t — include AFTER they are defined */
#include "Drive/BaseControlMotor.h"

#endif /* INVERTER_H */












