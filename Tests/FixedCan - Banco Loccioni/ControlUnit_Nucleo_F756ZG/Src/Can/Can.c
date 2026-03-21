/* ***********************************************************************
 * Includes
 * ***********************************************************************/

#include "Can.h"

/* ***********************************************************************
 * Private Defines
 * ***********************************************************************/

/* ***********************************************************************
 * Global Variables
 * ***********************************************************************/

Vector accelData = {0};              // Vector to store accelerometer data
Vector gyroData = {0};               // Vector to store gyroscope data

CAN_StatusFlags canStatusFlags;      // Status flags for CAN messages

uint16_t nodeAddress;
uint16_t baseAddress;

uint8_t canMsg[8]; 					 // Buffer for the messages read by the inverter

/* ***********************************************************************
 * Functions
 * ***********************************************************************/

/* ***********************************************************************
 * CAN INITIALIZATION
 * Initializes the CAN communication settings, configures filters, and
 * activates notifications for received messages.
 *
 * This function must be called once at startup to ensure proper
 * operation of the CAN interface.
 *************************************************************************/
void CanInit(void) {
    /* Initialize CAN communication settings */

	// Configuring CAN filter parameters for CAN1 and CAN2
	CAN_FilterTypeDef sFilterConfig1;
	CAN_FilterTypeDef sFilterConfig2;

	// Configure filter for CAN1
	// Use filter bank 0 for this filter configuration
	// STM32F466RE supports up to 28 filter banks, with Bank 0 being the first.
	sFilterConfig1.FilterBank = 0;

	// Set the filter mode to identifier mask mode
	// This mode filters messages based on specific bits in the CAN ID.
	sFilterConfig1.FilterMode = CAN_FILTERMODE_IDMASK;

	// Use a 32-bit filter scale
	// This allows for a single 32-bit filter to match either a 29-bit extended CAN ID or two 11-bit standard CAN IDs.
	sFilterConfig1.FilterScale = CAN_FILTERSCALE_32BIT;

	// Set the high part of the filter ID
	// The upper 16 bits of the 32-bit filter ID.
	sFilterConfig1.FilterIdHigh = 0x0000;

	// Set the low part of the filter ID
	// The lower 16 bits of the 32-bit filter ID.
	sFilterConfig1.FilterIdLow = 0x0000;

	// Set the high part of the filter mask
	// The upper 16 bits of the mask to determine significant bits in the CAN ID.
	sFilterConfig1.FilterMaskIdHigh = 0x0000;

	// Set the low part of the filter mask
	// The lower 16 bits of the mask.
	sFilterConfig1.FilterMaskIdLow = 0x0000;

	// Assign this filter to CAN receive FIFO 0
	// Messages that pass this filter will be stored in FIFO 0.
	sFilterConfig1.FilterFIFOAssignment = CAN_RX_FIFO0;

	// Enable this filter configuration
	// Activates the filter for processing incoming messages.
	sFilterConfig1.FilterActivation = ENABLE;

	// Configure the starting filter bank for CAN2 to filter bank 14
	// The STM32F7 mcu has a total of 28 filter banks, with filter banks 0-13 for CAN1 and 14-27 for CAN2.
	sFilterConfig1.SlaveStartFilterBank = 14;

	// Apply filter configuration to CAN1
	if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig1) != HAL_OK)
	{
		//Error_Handler(); // Handle configuration error
	}

	// Configure filter for CAN2
	// Use filter bank 14 for this filter configuration
	sFilterConfig2.FilterBank = 14;

	// Set the filter mode to identifier mask mode
	sFilterConfig2.FilterMode = CAN_FILTERMODE_IDMASK;

	// Use a 32-bit filter scale
	sFilterConfig2.FilterScale = CAN_FILTERSCALE_32BIT;

	// Set the high part of the filter ID
	sFilterConfig2.FilterIdHigh = 0x0000;

	// Set the low part of the filter ID
	sFilterConfig2.FilterIdLow = 0x0000;

	// Set the high part of the filter mask
	sFilterConfig2.FilterMaskIdHigh = 0x0000;

	// Set the low part of the filter mask
	sFilterConfig2.FilterMaskIdLow = 0x0000;

	// Assign this filter to CAN receive FIFO 0
	sFilterConfig2.FilterFIFOAssignment = CAN_RX_FIFO0;

	// Enable this filter configuration
	sFilterConfig2.FilterActivation = ENABLE;

	// Apply filter configuration to CAN2
	if (HAL_CAN_ConfigFilter(&hcan2, &sFilterConfig2) != HAL_OK)
	{
		//Error_Handler(); // Handle configuration error
	}

	// Initialize the CAN bus transceiver
	HAL_CAN_Start(&hcan1);
	HAL_CAN_Start(&hcan2);

	// This interrupt is triggered when there is at least one new message pending in FIFO 0
	HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
	HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING);
}

/* ***********************************************************************
 * CAN TRANSMISSION
 * Transmits a CAN message based on the specified parameters and selected
 * transceiver type. This function abstracts the transmission process,
 * supporting different transceiver implementations.
 *
 * Use the appropriate transmission function depending on the defined
 * transceiver macro (USE_SN65HVD230 or MCP2515).
 *************************************************************************/
void Transmit_CAN_Message(CAN_HandleTypeDef *hcan, uint32_t StdId, uint32_t DLC, uint8_t *TxData) {
    /* Transmit CAN message based on the selected transceiver */
    #ifdef USE_SN65HVD230
        Transmit_CAN_Message_SN65HVD230(hcan, StdId, DLC, TxData);                // Use SN65HVD230 transceiver
    #endif
}

void Transmit_CAN_Message_SN65HVD230(CAN_HandleTypeDef *hcan, uint32_t StdId, uint32_t DLC, uint8_t *TxData)
{
	// Initialize CAN header
	CAN_TxHeaderTypeDef TxHeader;

	// Standard 11-bit ID
	TxHeader.StdId = StdId; // Use TxHeader.ExtId = StdId; for 29-bit extended IDs

	// Configuring a CAN message with standard ID
	TxHeader.IDE = CAN_ID_STD;

	// Request type: Data frame
	TxHeader.RTR = CAN_RTR_DATA;

	// Number of bytes sent (maximum 8 bytes)
	uint8_t maxLength = (DLC < 8) ? DLC : 8;
	TxHeader.DLC = maxLength;

	// Select Tx Mailbox
	uint32_t TxMailbox = CAN_TX_MAILBOX0;

	// Try to add the message to the CAN bus
	if (HAL_CAN_AddTxMessage(hcan, &TxHeader, TxData, &TxMailbox) == HAL_OK) {
		// Toggle the the built-in LED to indicate successful transmission
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7); // BLUE Led
	} else {
		// Toggle the built-in LED to indicate an error during transmission
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14); // RED Led

		uint32_t error = HAL_CAN_GetError(hcan);

		if (error & HAL_CAN_ERROR_ACK) {
			// Receiving node unavailable: you may want to enable automatic message retransmission (Automatic Retransmission)
			Display_Message(&huart3, "ACK error: Node unreachable\n");
		}

		if (error & HAL_CAN_ERROR_TX_TERR0) {
			// Transmission error
			Display_Message(&huart3, "Transmission failed\n");
		}

		if (error & HAL_CAN_ERROR_BOF) {
			// If enabled, Automatic Bus-Off Management resets the Bus automatically
			Display_Message(&huart3, "Bus Off detected, wait for reset\n");
		}
	}
}

/**
 * ***********************************************************************
 * RECEIVE CAN MESSAGE
 * Receive a CAN message based on the specified parameters and selected
 * transceiver type. This function abstracts the reception process,
 * supporting different transceiver implementations.
 *
 * Use the appropriate reception function depending on the defined
 * transceiver macro (USE_SN65HVD230 or MCP2515).
 * ***********************************************************************
 */
void Receive_CAN_Message(CAN_HandleTypeDef *hcan)
{
	#ifdef USE_SN65HVD230
		if (hcan != NULL) Receive_CAN_Message_SN65HVD230(hcan);     // Use SN65HVD230 transceiver
    #endif
}

/* ***********************************************************************
 * SN65HVD230 MESSAGE RECEIVING
 * Handles the reception of CAN messages from the SN65HVD230 transceiver,
 * processes the messages based on their IDs, and updates corresponding
 * status flags.
 *
 * This function must be called from the pending CAN RX FIFO 0 message
 * interrupt service routine.
 *************************************************************************/
void Receive_CAN_Message_SN65HVD230(CAN_HandleTypeDef *hcan)
{
	CAN_RxHeaderTypeDef RxHeader;
	uint8_t RxData[8]; // Buffer to store received CAN data

	// Check if there is a CAN message available on CAN2
	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK) {

		// Process CAN messages based on their IDs
		switch (RxHeader.StdId) {
			case INVERTER_1_ACTUAL_VALUES_1:
			case INVERTER_1_ACTUAL_VALUES_2:
			case INVERTER_1_SETPOINTS_1:
				// Extract data from CAN message
				canMsg[0] = (uint8_t) RxData[0];
				canMsg[1] = (uint8_t) RxData[1];
				canMsg[2] = (uint8_t) RxData[2];
				canMsg[3] = (uint8_t) RxData[3];
				canMsg[4] = (uint8_t) RxData[4];
				canMsg[5] = (uint8_t) RxData[5];
				canMsg[6] = (uint8_t) RxData[6];
				canMsg[7] = (uint8_t) RxData[7];

				nodeAddress = INVERTER_1_NODE_ADDRESS;
				baseAddress = RxHeader.StdId - nodeAddress;

				// PARSE DATA IMMEDIATELY upon reception
				if (inverter_104.nodeAddress == nodeAddress) {
					switch (baseAddress)
					{
						case ACTUAL_VALUES_1_BASE_ADDRESS:
							parseActualValues1(&inverter_104, canMsg);
							break;

						case ACTUAL_VALUES_2_BASE_ADDRESS:
							parseActualValues2(&inverter_104, canMsg);
							break;

						default:
							break;
					}
				}

				canStatusFlags.msgID_Inverter = 1;
				break;

			case INVERTER_2_ACTUAL_VALUES_1:
			case INVERTER_2_ACTUAL_VALUES_2:
			case INVERTER_2_SETPOINTS_1:
				// Extract data from CAN message
				canMsg[0] = (uint8_t) RxData[0];
				canMsg[1] = (uint8_t) RxData[1];
				canMsg[2] = (uint8_t) RxData[2];
				canMsg[3] = (uint8_t) RxData[3];
				canMsg[4] = (uint8_t) RxData[4];
				canMsg[5] = (uint8_t) RxData[5];
				canMsg[6] = (uint8_t) RxData[6];
				canMsg[7] = (uint8_t) RxData[7];

				nodeAddress = INVERTER_2_NODE_ADDRESS;
				baseAddress = RxHeader.StdId - nodeAddress;

				canStatusFlags.msgID_Inverter = 1;
				break;

		case 0x33:
			// Extract data from CAN message
			accelData.x = (uint8_t) RxData[0];
			accelData.y = (uint8_t) RxData[1];
			accelData.z = (uint8_t) RxData[2];

			canStatusFlags.msgID_0x33 = 1;
			break;

		case 0x34:
			// Extract data from CAN message
			gyroData.x = (uint8_t) RxData[0];
			gyroData.y = (uint8_t) RxData[1];
			gyroData.z = (uint8_t) RxData[2];

			canStatusFlags.msgID_0x34 = 1;
			break;

		default:
			// Handle other CAN message IDs if needed
			break;
		}
	}else{
		// Toggle the built-in LED to indicate an error during reception
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14); // RED Led

		if (HAL_CAN_GetError(hcan) & HAL_CAN_ERROR_BOF) {
			// Reset errors
			HAL_CAN_ResetError(hcan);

			// Stop and restart the device
			HAL_CAN_Stop(hcan);
			HAL_CAN_Start(hcan);

			// (Optional) Re-enable interrupts if you use interrupt mode
			HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_ERROR);
		}
	}
}

/* ***********************************************************************
 * CAN MESSAGE DISPLAY
 * Displays received CAN messages via UART, formatting them for
 * transmission. The function checks the status flags for received
 * messages and sends the appropriate data.
 *
 * Messages are sent only once per reception to avoid duplication.
 *************************************************************************/
void Display_CAN_Messages()
{
    /* Display received CAN messages via UART (data already parsed at reception) */
	if (canStatusFlags.msgID_Inverter) {
		Display_Message(&huart3, "Received packet: %X %X %X %X %X %X %X %X\n",
			canMsg[0],
			canMsg[1],
			canMsg[2],
			canMsg[3],
			canMsg[4],
			canMsg[5],
			canMsg[6],
			canMsg[7]
		);

		if (inverter_104.nodeAddress == nodeAddress) {
			switch (baseAddress)
			{
				case ACTUAL_VALUES_1_BASE_ADDRESS:
					Display_Message(&huart3,
							"ID: 0x%X\nStatus: %X\nActual Velocity: %d\nTorque current: %f\nMagnetizing current: %f\n",
							baseAddress + nodeAddress,
							inverter_104.actualValues1.status,
							inverter_104.actualValues1.actualVelocity,
							(float)(inverter_104.actualValues1.torqueCurrent * ID110) / 16384,
							(float)(inverter_104.actualValues1.magnetizingCurrent * ID110) / 16384);
					break;

				case ACTUAL_VALUES_2_BASE_ADDRESS:
					Display_Message(&huart3,
							"ID: 0x%X\nMotor temperature: %f\nInverter temperature: %f\nError: %d\nIGBT temperature: %f\n",
							baseAddress + nodeAddress,
							(float)inverter_104.actualValues2.tempMotor * 0.1,
							(float)inverter_104.actualValues2.tempInverter * 0.1,
							inverter_104.actualValues2.errorInfo,
							(float)inverter_104.actualValues2.tempIGBT * 0.1);
					break;

				default:
					break;

			}
		}
//
//		if (inverter_204.nodeAddress == nodeAddress) {
//			switch (baseAddress)
//			{
//				case ACTUAL_VALUES_1_BASE_ADDRESS:
//					parseActualValues1(&inverter_204, canMsg);
//
//					Display_Message(&huart3,
//							"ID: 0x%X\nStatus: %X\nActual Velocity: %d\nTorque current: %f\nMagnetizing current: %f\n",
//							baseAddress + nodeAddress,
//							inverter_204.actualValues1.status,
//							inverter_204.actualValues1.actualVelocity,
//							(float)(inverter_204.actualValues1.torqueCurrent * ID110) / 16384,
//							(float)(inverter_204.actualValues1.magnetizingCurrent * ID110) / 16384);
//					break;
//
//				case ACTUAL_VALUES_2_BASE_ADDRESS:
//					parseActualValues2(&inverter_204, canMsg);
//
//					Display_Message(&huart3,
//							"ID: 0x%X\nMotor temperature: %f\nInverter temperature: %f\nError: %d\nIGBT temperature: %f\n",
//							baseAddress + nodeAddress,
//							(float)inverter_204.actualValues2.tempMotor * 0.1,
//							(float)inverter_204.actualValues2.tempInverter * 0.1,
//							inverter_204.actualValues2.errorInfo,
//							(float)inverter_204.actualValues2.tempIGBT * 0.1);
//					break;
//
//				default:
//					break;
//
//			}
//		}

		canStatusFlags.msgID_Inverter = 0;                          // Reset status flag for message ID Inverter
	}

    if (canStatusFlags.msgID_0x33) {
        char accelMsg[50];                                          // Buffer for accelerometer message
        uint16_t accelMsgLength = sprintf(accelMsg, "Accel: x = %d; y = %d; z = %d\n", accelData.x, accelData.y, accelData.z);
        HAL_UART_Transmit(&huart3, (uint8_t*)accelMsg, accelMsgLength, HAL_MAX_DELAY); // Transmit accelerometer message
        canStatusFlags.msgID_0x33 = 0;                              // Reset status flag for message ID 0x33
    }

    if (canStatusFlags.msgID_0x34) {
        char gyroMsg[50];                                           // Buffer for gyroscope message
        uint16_t gyroMsgLength = sprintf(gyroMsg, "Gyro: x = %d; y = %d; z = %d\n", gyroData.x, gyroData.y, gyroData.z);
        HAL_UART_Transmit(&huart3, (uint8_t*)gyroMsg, gyroMsgLength, HAL_MAX_DELAY); // Transmit gyroscope message
        canStatusFlags.msgID_0x34 = 0;                              // Reset status flag for message ID 0x34
    }
}
