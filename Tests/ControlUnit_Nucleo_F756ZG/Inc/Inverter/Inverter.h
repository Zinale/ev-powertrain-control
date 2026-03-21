#ifndef INVERTER_H
#define INVERTER_H

#include "main.h"

/* ***********************************************************************
 * INVERTER.H
 *
 * Header file defining constants, structures, enumerations, and function
 * prototypes for managing inverters in the system. This includes node
 * addresses, message mappings, control/status values, and inverter
 * operations.
 * ***********************************************************************/

/* ===========================================================================
 * NODE ADDRESSES
 * Unique node addresses for each inverter in the system.
 * -------------------------------------------------------------------------*/
#define INVERTER_1_NODE_ADDRESS 1 // Node address for Inverter 1.
#define INVERTER_2_NODE_ADDRESS 2 // Node address for Inverter 2.
#define INVERTER_3_NODE_ADDRESS 5 // Node address for Inverter 3.
#define INVERTER_4_NODE_ADDRESS 6 // Node address for Inverter 4.

/* ===========================================================================
 * MESSAGE BASE ADDRESSES
 * Base addresses for message types exchanged between inverters and the CAN
 * controller.
 * -------------------------------------------------------------------------*/
#define ACTUAL_VALUES_1_BASE_ADDRESS 0x282 // Actual Values 1 (Inverter to CAN controller).
#define ACTUAL_VALUES_2_BASE_ADDRESS 0x284 // Actual Values 2 (Inverter to CAN controller).
#define SETPOINTS_1_BASE_ADDRESS 0x183     // Setpoints 1 (CAN controller to Inverter).

/* ===========================================================================
 * MESSAGE ADDRESSES PER INVERTER
 * Specific message addresses derived from base addresses and node addresses.
 * -------------------------------------------------------------------------*/
// Inverter 1
#define INVERTER_1_ACTUAL_VALUES_1 (ACTUAL_VALUES_1_BASE_ADDRESS + INVERTER_1_NODE_ADDRESS)
#define INVERTER_1_ACTUAL_VALUES_2 (ACTUAL_VALUES_2_BASE_ADDRESS + INVERTER_1_NODE_ADDRESS)
#define INVERTER_1_SETPOINTS_1 (SETPOINTS_1_BASE_ADDRESS + INVERTER_1_NODE_ADDRESS)

// Inverter 2
#define INVERTER_2_ACTUAL_VALUES_1 (ACTUAL_VALUES_1_BASE_ADDRESS + INVERTER_2_NODE_ADDRESS)
#define INVERTER_2_ACTUAL_VALUES_2 (ACTUAL_VALUES_2_BASE_ADDRESS + INVERTER_2_NODE_ADDRESS)
#define INVERTER_2_SETPOINTS_1 (SETPOINTS_1_BASE_ADDRESS + INVERTER_2_NODE_ADDRESS)

// Inverter 3
#define INVERTER_3_ACTUAL_VALUES_1 (ACTUAL_VALUES_1_BASE_ADDRESS + INVERTER_3_NODE_ADDRESS)
#define INVERTER_3_ACTUAL_VALUES_2 (ACTUAL_VALUES_2_BASE_ADDRESS + INVERTER_3_NODE_ADDRESS)
#define INVERTER_3_SETPOINTS_1 (SETPOINTS_1_BASE_ADDRESS + INVERTER_3_NODE_ADDRESS)

// Inverter 4
#define INVERTER_4_ACTUAL_VALUES_1 (ACTUAL_VALUES_1_BASE_ADDRESS + INVERTER_4_NODE_ADDRESS)
#define INVERTER_4_ACTUAL_VALUES_2 (ACTUAL_VALUES_2_BASE_ADDRESS + INVERTER_4_NODE_ADDRESS)
#define INVERTER_4_SETPOINTS_1 (SETPOINTS_1_BASE_ADDRESS + INVERTER_4_NODE_ADDRESS)

/* ===========================================================================
 * CONTROL WORD VALUES
 * Values used to control inverter states.
 * -------------------------------------------------------------------------*/
#define cbInverterOn 0x100    // Enable controller.
#define cbDcOn 0x200          // Activate high voltage.
#define cbEnable 0x400        // Enable driver.
#define cbErrorReset 0x800    // Reset errors.

/* ===========================================================================
 * STATUS WORD VALUES
 * Values indicating inverter states and conditions.
 * -------------------------------------------------------------------------*/
#define bSystemReady 0x100     // Indicates system is ready.
#define bError 0x200           // Indicates an error condition.
#define bWarn 0x400            // Indicates a warning condition.
#define bQuitDcOn 0x800        // High voltage activation acknowledgment.
#define bDcOn 0x1000           // High voltage activation level.
#define bQuitInverterOn 0x2000 // Controller enable acknowledgment.
#define bInverterOn 0x4000     // Controller enable level.
#define bDerating 0x8000       // Indicates derating condition.

/* ===========================================================================
 * STATIC PARAMETERS
 * Fixed parameters for the inverter.
 * -------------------------------------------------------------------------*/
#define ID110 107 // Peak current for the converter.

/* ===========================================================================
 * STRUCTURE DEFINITIONS
 * Represent inverter data, control, and operational states.
 * -------------------------------------------------------------------------*/
typedef struct ActualValues1 {
    uint16_t status;         // Status word.
    int16_t actualVelocity;  // Actual speed value.
    int16_t torqueCurrent;   // Torque current (Iq).
    int16_t magnetizingCurrent; // Magnetizing current (Id).
} ActualValues1;

typedef struct ActualValues2 {
    int16_t tempMotor;    // Motor temperature.
    int16_t tempInverter; // Inverter temperature.
    uint16_t errorInfo;   // Diagnostic error code.
    int16_t tempIGBT;     // IGBT temperature.
} ActualValues2;

typedef struct Setpoints1 {
    uint16_t control;           // Control word.
    int16_t targetVelocity;     // Speed setpoint.
    int16_t torqueLimitPositiv; // Positive torque limit.
    int16_t torqueLimitNegativ; // Negative torque limit.
} Setpoints1;

/* ===========================================================================
 * ENUMERATIONS
 * Define possible states of the inverter.
 * -------------------------------------------------------------------------*/
typedef enum {
    IDLE,                 // System is inactive.
    LV_ON,                // Low Voltage circuit enabled.
    HV_ON,                // High Voltage circuit enabled.
    READY,                // System ready for operation.
    CONTROLLER_ACTIVE,    // Controller is active.
    FAIL,                 // System error detected.
} InverterState;

/* ===========================================================================
 * INVERTER STRUCTURE
 * Represents the configuration and state of an inverter.
 * -------------------------------------------------------------------------*/
typedef struct {
    uint16_t nodeAddress;        // Node address of the inverter.
    ActualValues1 actualValues1; // Actual values set 1.
    ActualValues2 actualValues2; // Actual values set 2.
    Setpoints1 setpoints1;       // Setpoints set 1.
    InverterState state;         // Current state of the inverter.
} Inverter;

/* ===========================================================================
 * EXTERNAL DECLARATIONS
 * Declare global inverter variables and function prototypes.
 * -------------------------------------------------------------------------*/
extern Inverter inverter_104;
extern Inverter inverter_204;

// Function prototypes
void Inverter_Init(Inverter* inverter, uint16_t nodeAddress);
void Inverter_CheckStatus(Inverter* inverter);
void Inverter_Activate(Inverter* inverter);
void Inverter_Deactivate(Inverter* inverter);

void parseActualValues1(Inverter* inverter, uint8_t data[8]);
void parseActualValues2(Inverter* inverter, uint8_t data[8]);

#endif /* INVERTER_H */
