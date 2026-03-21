#include "Inverter.h"
#include "APP.h"
/* ***********************************************************************
 * GLOBAL STATIC VARIABLES
 * ***********************************************************************/
volatile uint16_t lvOnPhaseCounter = 0;  // Counts 0-200 during LV_ON initial phase (1 second at 5ms intervals)

extern volatile int16_t RPMs;

/* ***********************************************************************
 * FUNCTION: Inverter_Init
 * Initializes the inverter structure with the given node address.
 * Resets all fields of ActualValues and Setpoints to zero.
 * ***********************************************************************/
void Inverter_Init(Inverter* inverter, uint16_t nodeAddress) {
    inverter->nodeAddress   = nodeAddress;
    inverter->actualValues1 = (ActualValues1){0};
    inverter->actualValues2 = (ActualValues2){0};
    inverter->setpoints1    = (Setpoints1){0};
    inverter->state         = IDLE;   // Start in IDLE, wait for first CAN message from inverter
}


int percent = 0;

/* ***********************************************************************
 * FUNCTION: Inverter_SendSetpointsCyclic
 * Sends Setpoints1 message in a cyclic manner (every ~5ms).
 * This ensures the inverter's watchdog (50ms timeout) never expires.
 * 
 * This function also calls Inverter_Activate() to update the state machine,
 * and implements special logic for the LV_ON initial phase using global counters.
 * 
 * Uses global counters to track elapsed time between transmissions (robust against
 * currentTimeMs wraparound at 1000ms). Every call to this function represents
 * one 5ms interval, and the counter is incremented appropriately.
 * 
 * INPUT:
 *  - inverter: Pointer to the inverter structure
 *  - currentTimeMs: Current system time in milliseconds (from scheduler) - unused for timing
 * ***********************************************************************/
void Inverter_SendSetpointsCyclic(Inverter* inverter, uint32_t currentTimeMs) {

    // Call Inverter_Activate to update state machine and set appropriate setpoints
    Inverter_Activate(inverter);
    percent = APP_GetPercent();
    // CRITICAL: Do NOT send anything if inverter is not connected (still in IDLE)
    // Wait for the first CAN message from inverter to transition out of IDLE
    if (inverter->state == FAIL) {
        return;  // Inverter not yet responding, don't send anything
    }

    // Send current setpoints1 (already set by Inverter_Activate based on state and phase)
    uint8_t canMsg[8];
    
    Transmit_CAN_Message(&hcan1,
                         SETPOINTS_1_BASE_ADDRESS + inverter->nodeAddress,
                         8,
                         parseSetpoints1(inverter->setpoints1, canMsg));
}

/* ***********************************************************************
 * FUNCTION: Inverter_CheckStatus
 * Reads the status bits from ActualValues1 and updates the inverter state.
 * Uses overlapping if/else-if statements to allow bits to accumulate and reach
 * the highest states (READY, CONTROLLER_ACTIVE) when all required bits are set.
 *
 * NOTE:
 *  - All bits are checked with simple AND operations.
 *  - State transitions occur when each bit becomes set.
 *  - Final state reached depends on all bits present in the status word.
 * ***********************************************************************/
void Inverter_CheckStatus(Inverter* inverter) {
    uint16_t status = inverter->actualValues1.status;

    if (status & bError) {
        inverter->state = FAIL;
    }
    else if (status & bQuitInverterOn) {
        inverter->state = CONTROLLER_ACTIVE;
    }
    else if (status & bQuitDcOn) {
        inverter->state = READY;
    }
    else if (status & bDcOn) {
        inverter->state = HV_ON;
    }
    else if (status & bSystemReady) {
        inverter->state = LV_ON;
    }
    else {
        inverter->state = IDLE;
    }
}

/* ***********************************************************************
 * FUNCTION: parseSetpoints1
 * Converts Setpoints1 structure into a CAN message.
 * INPUT:
 *   - setpoints1: Setpoints1 structure containing control and torque limits.
 *   - canMsg: Pointer to an array where the CAN message will be stored.
 * OUTPUT:
 *   - Pointer to the filled CAN message array.
 * ***********************************************************************/
uint8_t* parseSetpoints1(Setpoints1 setpoints1, uint8_t* canMsg)
{
    canMsg[0] = (uint8_t)( setpoints1.control & 0xFF );
    canMsg[1] = (uint8_t)((setpoints1.control >> 8) & 0xFF);

    canMsg[2] = (uint8_t)( setpoints1.targetVelocity & 0xFF );
    canMsg[3] = (uint8_t)((setpoints1.targetVelocity >> 8) & 0xFF);

    canMsg[4] = (uint8_t)( setpoints1.torqueLimitPositiv & 0xFF );
    canMsg[5] = (uint8_t)((setpoints1.torqueLimitPositiv >> 8) & 0xFF);

    canMsg[6] = (uint8_t)( setpoints1.torqueLimitNegativ & 0xFF );
    canMsg[7] = (uint8_t)((setpoints1.torqueLimitNegativ >> 8) & 0xFF);

    return canMsg;
}

void parseActualValues1(Inverter* inverter, uint8_t data[8])
{
    //data[0] = 0; //TODO: added to try to fix a logical error (LASCIARE COMMENTATO)

    inverter->actualValues1.status             = (uint16_t)(data[0] | (data[1] << 8)); // status word
    inverter->actualValues1.actualVelocity     = (int16_t)(data[2] | (data[3] << 8));  // velocity
    inverter->actualValues1.torqueCurrent      = (int16_t)(data[4] | (data[5] << 8));  // torque current
    inverter->actualValues1.magnetizingCurrent = (int16_t)(data[6] | (data[7] << 8));  // magnetizing current
}

void parseActualValues2(Inverter* inverter, uint8_t data[8])
{
    inverter->actualValues2.tempMotor    = (int16_t)(data[0] | (data[1] << 8)); // Motor temperature
    inverter->actualValues2.tempInverter = (int16_t)(data[2] | (data[3] << 8)); // Inverter temperature
    inverter->actualValues2.errorInfo    = (uint16_t)(data[4] | (data[5] << 8)); // Error diagnostic info
    inverter->actualValues2.tempIGBT     = (int16_t)(data[6] | (data[7] << 8)); // IGBT temperature
}

// Helper function to send CAN messages with setpoints and a delay
static void Send_Setpoints(Inverter* inverter, Setpoints1 setpoints, uint8_t* canMsg) {
    inverter->setpoints1 = setpoints;

    Transmit_CAN_Message(&hcan1,
                         SETPOINTS_1_BASE_ADDRESS + inverter->nodeAddress,
                         8,
                         parseSetpoints1(setpoints, canMsg));
}

/* ***********************************************************************
 * FUNCTION: Inverter_Activate
 * Activates the inverter step by step based on its current state.
 * IMPORTANT: This function ONLY updates setpoints1 and state in memory.
 * It does NOT send CAN messages. CAN transmission is handled by
 * Inverter_SendSetpointsCyclic() which calls this function and then
 * sends the appropriate messages.
 * ***********************************************************************/
void Inverter_Activate(Inverter* inverter) {
    Inverter_CheckStatus(inverter);

    uint8_t canMsg[8];

    RPMs = (APP_GetPercent() * 14000)/100;

    switch (inverter->state) {
        case IDLE:
            // System not ready yet: clear all flags and reset setpoints
            inverter->setpoints1 = (Setpoints1){0, 0, 0, 0};  // Clear setpoints for clean restart
            lvOnPhaseCounter = 0;  // Reset counter when entering IDLE
            //RPMs = 0;
            break;

        case LV_ON:
        	 //RPMs = 0;
            // LV ok, HV still off
            // Special handling: first 500 cycles (5 seconds) send ZEROS only
            if (lvOnPhaseCounter < 000) {
                // Initial phase (0-199): send all zeros
                inverter->setpoints1 = (Setpoints1){0, 0, 0, 0};
                lvOnPhaseCounter++;
            } else {
                // Normal phase (counter > 200): continue with cbDcOn
                inverter->setpoints1 = (Setpoints1){cbDcOn, 0, 0, 0};
            }
            break;

        case HV_ON:
            // HV enabled, waiting for QuitDcOn: keep DcOn
            lvOnPhaseCounter = 0;  // Reset counter when leaving LV_ON
            //RPMs = 0;
            inverter->setpoints1 = (Setpoints1){cbDcOn, 0, 0, 0};
            break;

        case READY:
            // Prepare controller activation: transition towards cbEnable + cbInverterOn
            lvOnPhaseCounter = 0;  // Reset counter when leaving LV_ON
            //RPMs = 0;
            inverter->setpoints1 = (Setpoints1){cbDcOn | cbEnable | cbInverterOn, 0, 0, 0};
            break;

        case CONTROLLER_ACTIVE:
            // Controller active: send setpoints with full control flags
            // These are typically updated by the Tasks layer (e.g., HandleInverterCommunication)
            // with the latest velocity, torque, etc.
            lvOnPhaseCounter = 0;  // Reset counter when leaving LV_ON

            //if (RPMs <= 18000) RPMs += 10;

            inverter->setpoints1 = (Setpoints1){cbDcOn | cbEnable | cbInverterOn, RPMs, (int16_t)1000, (int16_t)0};
            break;

        case FAIL:
            // Error handling:
            // 1) all zeros
            Send_Setpoints(inverter, (Setpoints1){0, 0, 0, 0}, canMsg);

            // 2) send ErrorReset
            Send_Setpoints(inverter, (Setpoints1){cbErrorReset, 0, 0, 0}, canMsg);

            // 3) again all zeros
            Send_Setpoints(inverter, (Setpoints1){0, 0, 0, 0}, canMsg);
            break;

        default:
            break;
    }
}

/* ***********************************************************************
 * FUNCTION: Inverter_Deactivate
 * Deactivates the inverter step by step based on its current state.
 * IMPORTANT: This function ONLY updates setpoints1 and state in memory.
 * It does NOT send CAN messages. CAN transmission is handled by
 * Inverter_SendSetpointsCyclic().
 * ***********************************************************************/
void Inverter_Deactivate(Inverter* inverter) {
    Inverter_CheckStatus(inverter);

    switch (inverter->state) {
        case CONTROLLER_ACTIVE:
            // Switch off sequence (reverse order):
            // Step 1: Remove cbInverterOn but keep cbDcOn + cbEnable
            inverter->setpoints1 = (Setpoints1){cbDcOn | cbEnable, 0, 0, 0};
            break;

        case READY:
            // Disable HV activation level
            inverter->setpoints1 = (Setpoints1){0, 0, 0, 0};
            break;

        case HV_ON:
            // HV circuit off
            inverter->setpoints1 = (Setpoints1){0, 0, 0, 0};
            break;

        case LV_ON:
            // LV off
            inverter->setpoints1 = (Setpoints1){0, 0, 0, 0};
            break;

        case IDLE:
            // Already idle
            inverter->setpoints1 = (Setpoints1){0, 0, 0, 0};
            break;

        default:
            break;
    }
}
