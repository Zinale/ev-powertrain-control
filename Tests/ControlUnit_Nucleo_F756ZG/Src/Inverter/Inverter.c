#include "Inverter.h"

#define DELAY 0

/* ***********************************************************************
 * FUNCTION: Inverter_Init
 * Initializes the inverter structure with the given node address.
 * Resets all fields of ActualValues and Setpoints to zero.
 * ***********************************************************************/
void Inverter_Init(Inverter* inverter, uint16_t nodeAddress) {
    inverter->nodeAddress = nodeAddress;
    inverter->actualValues1 = (ActualValues1){0};
    inverter->actualValues2 = (ActualValues2){0};
    inverter->setpoints1 = (Setpoints1){0};
    inverter->state = IDLE;
}

/* ***********************************************************************
 * FUNCTION: Inverter_CheckStatus
 * Reads the status bits from ActualValues1 and updates the inverter state.
 * Outputs status messages for diagnostic purposes.
* ***********************************************************************/
void Inverter_CheckStatus(Inverter* inverter) {
    uint16_t status = inverter->actualValues1.status;

    if (status == (bSystemReady | bDerating)) {
        inverter->state = LV_ON;
        Display_Message(&huart3, "LV circuit enabled\n");
    } else if ((status == bSystemReady) || (status == (bSystemReady | bDcOn))) {
        inverter->state = HV_ON;
        Display_Message(&huart3, "HV circuit enabled\n");
    } else if ((status == (bSystemReady | bDcOn | bQuitDcOn)) ||
               (status == (bSystemReady | bDcOn | bQuitDcOn | bInverterOn))) {
        inverter->state = READY;
        Display_Message(&huart3, "DcOn enabled\n");
    } else if (status == (bSystemReady | bDcOn | bQuitDcOn | bInverterOn | bQuitInverterOn)) {
        inverter->state = CONTROLLER_ACTIVE;
        Display_Message(&huart3, "Controller enabled\n");
    } else if (status & bError) {
        inverter->state = FAIL;
        Display_Message(&huart3, "Error detected\n");
    } else {
        inverter->state = IDLE;
        Display_Message(&huart3, "System inactive (LV switched off)\n");
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
	canMsg[0] = setpoints1.control & 0xFF;
	canMsg[1] = (setpoints1.control >> 8) & 0xFF;
	canMsg[2] = setpoints1.targetVelocity & 0xFF;
	canMsg[3] = (setpoints1.targetVelocity >> 8) & 0xFF;
	canMsg[4] = setpoints1.torqueLimitPositiv & 0xFF;
	canMsg[5] = (setpoints1.torqueLimitPositiv >> 8) & 0xFF;
	canMsg[6] = setpoints1.torqueLimitNegativ & 0xFF;
	canMsg[7] = (setpoints1.torqueLimitNegativ >> 8) & 0xFF;

    return canMsg;
}

void parseActualValues1(Inverter* inverter, uint8_t data[8])
{
    inverter->actualValues1.status = data[0] | (data[1] << 8);              // Combine low and high byte for status word.
    inverter->actualValues1.actualVelocity = data[2] | (data[3] << 8);      // Combine low and high byte for velocity.
    inverter->actualValues1.torqueCurrent = data[4] | (data[5] << 8);       // Combine low and high byte for torque current.
    inverter->actualValues1.magnetizingCurrent = data[6] | (data[7] << 8);  // Combine low and high byte for magnetizing current.
}

void parseActualValues2(Inverter* inverter, uint8_t data[8])
{
	inverter->actualValues2.tempMotor = data[0] | (data[1] << 8);           // Motor temperature.
	inverter->actualValues2.tempInverter = data[2] | (data[3] << 8);        // Inverter temperature.
	inverter->actualValues2.errorInfo = data[4] | (data[5] << 8);           // Error diagnostic info.
	inverter->actualValues2.tempIGBT = data[6] | (data[7] << 8);            // IGBT temperature.
}

// Helper function to send CAN messages with setpoints and a delay
static void Send_Setpoints(Inverter* inverter, Setpoints1 setpoints, uint8_t* canMsg) {
    inverter->setpoints1 = setpoints;
    Transmit_CAN_Message(&hcan1, SETPOINTS_1_BASE_ADDRESS + inverter->nodeAddress, 8, parseSetpoints1(setpoints, canMsg));
    HAL_Delay(DELAY);
}

/* ***********************************************************************
 * FUNCTION: Inverter_Activate
 * Activates the inverter step by step based on its current state.
 * Sends appropriate commands through CAN communication.
 * ***********************************************************************/
void Inverter_Activate(Inverter* inverter) {
    Inverter_CheckStatus(inverter);

    uint8_t canMsg[8];

    switch (inverter->state) {
        case IDLE:
            //Display_Message(&huart3, "Switch on LV circuit...\n");
            Send_Setpoints(inverter, (Setpoints1){0, 0, 0, 0}, canMsg);
            break;

        case LV_ON:
            //Display_Message(&huart3, "Switch on HV circuit...\n");
            Send_Setpoints(inverter, (Setpoints1){0, 0, 0, 0}, canMsg);
            break;

        case HV_ON:
            //Display_Message(&huart3, "Enabling HV activation level...\n");
            Send_Setpoints(inverter, (Setpoints1){cbDcOn, 0, 0, 0}, canMsg);
            break;

        case READY:
            //Display_Message(&huart3, "Performing security check (torque limit pos and neg = 0)...\n");
            Send_Setpoints(inverter, (Setpoints1){cbDcOn, 0, 0, 0}, canMsg);

            //Display_Message(&huart3, "Enabling driver...\n");
            Send_Setpoints(inverter, (Setpoints1){cbDcOn | cbEnable, 0, 0, 0}, canMsg);

            //Display_Message(&huart3, "Enabling controller...\n");
            Send_Setpoints(inverter, (Setpoints1){cbDcOn | cbEnable | cbInverterOn, 0, 0, 0}, canMsg);
            break;

        case CONTROLLER_ACTIVE:
            //Display_Message(&huart3, "Sending setpoints...\n");
        	Send_Setpoints(inverter, inverter->setpoints1, canMsg);
            break;

        case FAIL:
            //Display_Message(&huart3, "Disabling controller...\n");
            Send_Setpoints(inverter, (Setpoints1){0, 0, 0, 0}, canMsg);
            //HAL_Delay(DELAY);
            //static const ErrorInfo unknownError = { "Unknown error category", "Unknown class of error" };
            //const ErrorInfo* error = findErrorById(inverter->actualValues2.errorInfo) ?: &unknownError;
            //Display_Message(&huart3, "Error code: %d (%s), Error class: %s\n", inverter->actualValues2.errorInfo, error->category, error->class);

            //Display_Message(&huart3, "Resetting error...\n");
            Send_Setpoints(inverter, (Setpoints1){cbErrorReset, 0, 0, 0}, canMsg);
            //HAL_Delay(DELAY);

            Send_Setpoints(inverter, (Setpoints1){0, 0, 0, 0}, canMsg);
            break;

        default:
            break;
    }
}

/* ***********************************************************************
 * FUNCTION: Inverter_Deactivate
 * Deactivates the inverter step by step based on its current state.
 * Sends appropriate commands through CAN communication.
 * ***********************************************************************/
void Inverter_Deactivate(Inverter* inverter) {
    Inverter_CheckStatus(inverter);

    uint8_t canMsg[8];

    switch (inverter->state) {
        case CONTROLLER_ACTIVE:
            //Display_Message(&huart3, "Switching off...\n");
            Send_Setpoints(inverter, (Setpoints1){cbDcOn | cbEnable | cbInverterOn, 0, 0, 0}, canMsg);

            //Display_Message(&huart3, "Disabling controller...\n");
            Send_Setpoints(inverter, (Setpoints1){cbDcOn | cbEnable, 0, 0, 0}, canMsg);

            //Display_Message(&huart3, "Disabling driver...\n");
            Send_Setpoints(inverter, (Setpoints1){cbDcOn, 0, 0, 0}, canMsg);
            break;

        case READY:
            //Display_Message(&huart3, "Disabling HV activation level...\n");
            Send_Setpoints(inverter, (Setpoints1){0, 0, 0, 0}, canMsg);
            break;

        case HV_ON:
            //Display_Message(&huart3, "Switch off HV circuit...\n");
            // TODO: send signal to battery pack to switch off HV
            Send_Setpoints(inverter, (Setpoints1){0, 0, 0, 0}, canMsg);
            break;

        case LV_ON:
            //Display_Message(&huart3, "Switch off LV circuit...\n");
            // TODO: send signal to battery pack to switch off HV
            Send_Setpoints(inverter, (Setpoints1){0, 0, 0, 0}, canMsg);
            break;

        case IDLE:
            Send_Setpoints(inverter, (Setpoints1){0, 0, 0, 0}, canMsg);
            break;

        default:
            break;
    }
}
