/* ***********************************************************************
 * Includes
 * ***********************************************************************/

#include "Tasks.h"


/* ***********************************************************************
 * Global Variables
 * ***********************************************************************/

uint16_t adc_buf[ADC_BUFFER_LENGTH]; // Buffer to store the raw ADC readings for each conversion

CalibCircularBuffer zeroCalibBuffer;
CalibCircularBuffer fullCalibBuffer;

// Remember to set the ADC channels associated with the APPs in pairs:
// Channel 0 and 1 for the two signals of the first sensor & Channel 2 and 3 for the two signals of the second sensor
volatile uint16_t APPs_val[4] = {0}; // Acceleration Pedal Position Sensors (APPs): mapped & filtered readings for each of 4 channels
volatile uint16_t APPs_flt = 0;

volatile int16_t STEERING_val = 0; // Steering wheel potentiometer

volatile int16_t leftWheelMotorRPM = 0;
volatile int16_t rightWheelMotorRPM = 0;

Inverter inverter_104;
Inverter inverter_204;

// Variables for using software filters (for ADC) - for each of 4 APPs channels
static const float kFIRCoefficients[FIR_ORDER] = {0.2f, 0.2f, 0.2f, 0.2f, 0.2f};

// Runtime structures holding channel states, filter states, and calibration data
static PedalChannelState pedalChannelStates[4] = {0};   // State per ADC channel
static FilterState pedalChannelFilters[4];        // FIR filter state per channel - used to store previous states useful for filter control
PedalCalibrationData pedalCalibrationData = {0};   // Calibration loaded from flash
volatile bool zeroCollectionActive = false;
volatile bool fullCollectionActive = false;
static uint8_t implausibilityCounter = 0;

volatile float pedalPosition = 0.0f;   // normalized pedal position [0.0, 1.0]

/* ***********************************************************************
 * General Tasks
 * ***********************************************************************/

void TaskInit(void)
{
	/* Executed once at startup. */
	/* TODO Functions for HW/SW initialization. */

	SchedulerInitFct();

	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buf, ADC_BUFFER_LENGTH);

	CanInit();

	Inverter_Init(&inverter_104, INVERTER_1_NODE_ADDRESS);
	Inverter_Init(&inverter_204, INVERTER_2_NODE_ADDRESS);

	InitializePedalModule();

	/* Enable EXTI for blue USER_BUTTON on PC13 */
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void TaskFast(void)
{
	/* Executed at medium scheduling intervals (e.g. every 1 ms). */

	AcquireSensorValues();    // updates STEERING_val values

	// ---- ZERO calibration ----
	if (zeroCollectionActive) {
		CalibBuffer_Push(&zeroCalibBuffer, adc_buf);
		if (zeroCalibBuffer.full) {
			// Capture raw ADC readings for each channel as the 'zero' reference
			CalibBuffer_ComputeMax(&zeroCalibBuffer, pedalCalibrationData.zero_adc);
			zeroCollectionActive = false;

			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); // GREEN LED OFF

			Display_Message(&huart3, "[CALIB] ZERO complete: [%u,%u,%u,%u]",
				pedalCalibrationData.zero_adc[0], pedalCalibrationData.zero_adc[1],
				pedalCalibrationData.zero_adc[2], pedalCalibrationData.zero_adc[3]);
		}
	}else if (fullCollectionActive) { // ---- FULL calibration ----
		CalibBuffer_Push(&fullCalibBuffer, adc_buf);
		if (fullCalibBuffer.full) {
			// Capture raw ADC readings for each channel as the 'full' reference
			CalibBuffer_ComputeMin(&fullCalibBuffer, pedalCalibrationData.full_adc);
			fullCollectionActive = false;

			Display_Message(&huart3, "[CALIB] FULL complete: [%u,%u,%u,%u]",
				pedalCalibrationData.full_adc[0], pedalCalibrationData.full_adc[1],
				pedalCalibrationData.full_adc[2], pedalCalibrationData.full_adc[3]);

			// To preserve flash memory longevity, avoid unnecessary write/erase cycles.
			// The STM32F756ZG's internal flash memory supports a minimum of 10,000 write/erase cycles per sector.
			// Therefore, compare the new calibration data with the existing data in flash memory.
			// If the data is unchanged, skip the write operation to prevent premature wear of the flash sector.
			/*if (memcmp(stored, &pedalCalibrationData, sizeof(PedalCalibrationData)) != 0) {
				// Data is different, save to Flash
				if (SavePedalCalibration()) {
					Display_Message(&huart3, "[CALIB] Calibration saved to Flash.");
				} else {
					Display_Message(&huart3, "[CALIB] Error saving calibration!");
				}
			} else {
				// Same as existing — skip flash write
				Display_Message(&huart3, "[CALIB] Calibration unchanged — Flash write skipped.");
			}*/

			if (SavePedalCalibration()) {
				Display_Message(&huart3, "[CALIB] Calibration saved to Flash.");
			} else {
				// Toggle the built-in LED to indicate an error
				HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14); // RED Led
				Display_Message(&huart3, "[CALIB] Error saving calibration!");
			}

			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); // GREEN LED OFF

			pedalCalibrationData.crc32 = 0; // Reset for future calibration
		}
	}

	// ---- IMPLAUSIBILITY check ----
	ProcessAcceleratorPedalChannels(
		adc_buf,
		&APPs_flt,
		&pedalPosition
	);

	//Determine wheel assignment based on the steering wheel angle:
	// For a right turn (steeringWheelAngle > 0): the right wheel is the inner wheel.
	// For a left turn (steeringWheelAngle < 0): the left wheel is the inner wheel.
	MotorRPM motorRPM = CalculateMotorRPM(APPs_flt, STEERING_val);
	if (STEERING_val > 0) {
		// Right turn: inner wheel (lower speed) is the right wheel, outer wheel is the left wheel.
		rightWheelMotorRPM = motorRPM.rpm_motor_inner;
		leftWheelMotorRPM  = motorRPM.rpm_motor_outer;
	} else if (STEERING_val < 0) {
		// Left turn: inner wheel is the left wheel, outer wheel is the right wheel.
		leftWheelMotorRPM  = motorRPM.rpm_motor_inner;
		rightWheelMotorRPM = motorRPM.rpm_motor_outer;
	} else {
		// Straight-line driving: both wheels receive the same RPM.
		leftWheelMotorRPM  = motorRPM.rpm_motor_inner;
		rightWheelMotorRPM = motorRPM.rpm_motor_inner;
	}

	Display_CAN_Messages();

	HandleInverterCommunication();
}

void TaskMed(void)
{
	/* Executed at medium scheduling intervals (e.g. every 10 ms). */

}

void TaskSlow(void)
{
	/* TODO functions executed periodically (medium schedulation, e.g. 100 ms). */

	/* ***********************************************************************
	 * WARNING: RECEPTION MAY BE LESS ACCURATE AS INTERRUPTS ARE NOT USED
	 * FOR HANDLING CAN MESSAGES WITH THE MCP2515 CAN BUS TRANSCEIVER.
	 *************************************************************************/
	Receive_CAN_Message(NULL);
}

/* ***********************************************************************
 * STEERING WEEL Function
 * ***********************************************************************/
void AcquireSensorValues()
{
	// Process the steering sensor reading and map it to the defined range
	STEERING_val = map_signed(adc_buf[4], STEERING_ZERO_POSITION, STEERING_FULL_POSITION, STEERING_RANGE_MIN, STEERING_RANGE_MAX);
}


/* ***********************************************************************
 * Motor Functions
 * ***********************************************************************/

void HandleInverterCommunication()
{
	uint8_t activationValue = 1;

	// Communication with inverter 104
	if (inverter_104.state == CONTROLLER_ACTIVE){
		inverter_104.setpoints1 = (Setpoints1){cbDcOn | cbEnable | cbInverterOn, leftWheelMotorRPM, (int16_t)TORQUE_POS, (int16_t)TORQUE_NEG};
	}

	// Communication with inverter 204
	if (inverter_204.state == CONTROLLER_ACTIVE){
		inverter_204.setpoints1 = (Setpoints1){cbDcOn | cbEnable | cbInverterOn, rightWheelMotorRPM, (int16_t)TORQUE_POS, (int16_t)TORQUE_NEG};
	}

	// if switch is on, activate inverter, else deactivate
	(activationValue) ? Inverter_Activate(&inverter_104) : Inverter_Deactivate(&inverter_104);
	(activationValue) ? Inverter_Activate(&inverter_204) : Inverter_Deactivate(&inverter_204);

	// ------------------------------------------------------------------------------------------------
	// Send temperatures of motors and inverters over CAN ID 0x023 (8 bytes) - (for the steering wheel)
	// Format: T_Motor_1, T_Motor_2, T_Inverter_1, T_Inverter_2 (int16_t each)
	// ------------------------------------------------------------------------------------------------
	{
		uint8_t tempPayload[8];

		int16_t t_motor_1 = inverter_104.actualValues2.tempMotor;
		int16_t t_motor_2 = inverter_204.actualValues2.tempMotor;
		int16_t t_inv_1   = inverter_104.actualValues2.tempInverter;
		int16_t t_inv_2   = inverter_204.actualValues2.tempInverter;

		tempPayload[0] = (uint8_t)(t_motor_1 >> 8);
		tempPayload[1] = (uint8_t)(t_motor_1 & 0xFF);
		tempPayload[2] = (uint8_t)(t_motor_2 >> 8);
		tempPayload[3] = (uint8_t)(t_motor_2 & 0xFF);
		tempPayload[4] = (uint8_t)(t_inv_1 >> 8);
		tempPayload[5] = (uint8_t)(t_inv_1 & 0xFF);
		tempPayload[6] = (uint8_t)(t_inv_2 >> 8);
		tempPayload[7] = (uint8_t)(t_inv_2 & 0xFF);

		Transmit_CAN_Message(&hcan1, 0x023, 8, tempPayload);

		uint8_t rpmData[2];  // Array to hold the RPM data (2 bytes for uint16_t)

		// Populate rpmData with the value of APPs_flt
		rpmData[0] = (uint8_t)(APPs_flt & 0xFF);           // LSB (Least Significant Byte)
		rpmData[1] = (uint8_t)((APPs_flt >> 8) & 0xFF);    // MSB (Most Significant Byte)

		// Send the CAN message with the RPM data
		Transmit_CAN_Message(&hcan1, 0x031, 2, rpmData);     // 0x040 is the CAN address, 2 is the DLC (Data Length Code), rpmData contains the data
	}
}

/**
 * @brief Calculates corrected motor RPM values for the inner and outer wheels,
 *        simulating an electronic differential to aid in turning.
 *
 * This function accepts the base motor RPM (the RPM that would be applied during
 * straight-line driving, e.g., 2000 RPM at full throttle) as a uint16_t, and the steering
 * wheel angle (in degrees) as an int8_t. It converts these values into a linear speed and
 * computes the turning radius using Ackermann geometry. The inner and outer wheel speeds are
 * then calculated and converted back into RPM.
 *
 * @param baseMotorRPM Base motor RPM (uint16_t), e.g., 2000 RPM for full throttle.
 * @param steeringWheelAngleDeg Steering wheel angle in degrees (int8_t).
 * @return MotorRPM structure containing the corrected RPM for the inner and outer wheels.
 */
MotorRPM CalculateMotorRPM(uint16_t baseMotorRPM, int8_t steeringWheelAngleDeg)
{
    MotorRPM rpmValues = {0, 0};

    // Convert baseMotorRPM to wheel linear speed in m/s.
    // Circumference = 2 * PI * WHEEL_RADIUS, and conversion factor from RPM to RPS is 1/60.
    double speed = (baseMotorRPM * 2 * PI * WHEEL_RADIUS) / 60.0;

    // Convert steering wheel angle (int8_t) to double and calculate the actual wheel steering angle.
    // The wheel steering angle is determined by dividing the steering wheel angle by STEERING_RATIO.
    double wheelSteeringAngleDeg = ((double)steeringWheelAngleDeg) / STEERING_RATIO;

    // Convert wheel steering angle from degrees to radians.
    double wheelSteeringAngleRad = wheelSteeringAngleDeg * PI / 180.0;
    double absWheelSteeringAngleRad = fabs(wheelSteeringAngleRad);

    // If the steering angle is nearly zero (straight-line driving), no differential correction is needed.
    if (absWheelSteeringAngleRad < 1e-6) {
        rpmValues.rpm_motor_inner = baseMotorRPM;
        rpmValues.rpm_motor_outer = baseMotorRPM;
        return rpmValues;
    }

    // Calculate the turning radius (R) using Ackermann geometry.
    // R = WHEEL_BASE / tan(|wheelSteeringAngleRad|)
    double R = WHEEL_BASE / tan(absWheelSteeringAngleRad);

    // Calculate the vehicle's angular velocity (omega) in rad/s.
    double omega = speed / R;

    // Calculate the linear speeds for the inner and outer wheels.
    // Inner wheel: travels on a circle with radius (R - TRACK_WIDTH/2)
    // Outer wheel: travels on a circle with radius (R + TRACK_WIDTH/2)
    double v_inner = speed - (omega * TRACK_WIDTH / 2.0) * DIFFERENTIAL_GAIN;
    double v_outer = speed + (omega * TRACK_WIDTH / 2.0) * DIFFERENTIAL_GAIN;

    // Convert linear speeds back into RPM.
    // RPM = (60 * speed) / (2 * PI * WHEEL_RADIUS)
    double rpm_inner = (60.0 * v_inner) / (2 * PI * WHEEL_RADIUS);
    double rpm_outer = (60.0 * v_outer) / (2 * PI * WHEEL_RADIUS);

    if (rpm_inner < 0) rpm_inner = 0;
    if (rpm_outer > baseMotorRPM) rpm_outer = baseMotorRPM;

    // Store the corrected RPM values as uint16_t.
    rpmValues.rpm_motor_inner = (uint16_t)rpm_inner;
    rpmValues.rpm_motor_outer = (uint16_t)rpm_outer;

    return rpmValues;
}

/* ***********************************************************************
 * APPs Functions
 * ***********************************************************************/

void InitializePedalModule(void)
{
	// Initialize calibration buffers so head=0, full=false
	CalibBuffer_Init(&zeroCalibBuffer);
	CalibBuffer_Init(&fullCalibBuffer);

    // Initialize FIR filters for each pedal channel
    for (int i = 0; i < 4; i++) {
        Filter_Init(&pedalChannelFilters[i]);
    }
    // Load stored calibration values
    if (LoadPedalCalibration()) {
        Display_Message(&huart3, "[INIT] Calibration loaded from Flash.");
    } else {
        Display_Message(&huart3, "[INIT] No valid calibration found.");
    }
    // Force the calibration state machine back to step 0
    pedalCalibrationData.crc32 = 0;
}

/**
 * @brief Validate a single pedal ADC channel by checking for excessive jumps.
 *
 * On first invocation, seeds the previous_adc history. Thereafter, marks the channel
 * invalid if the absolute difference between current and previous ADC exceeds threshold.
 *
 * @param state           Pointer to channel state structure.
 * @param current_adc     Latest raw ADC reading for this channel.
 */
static void ValidateAndUpdateChannel(PedalChannelState *state, uint16_t current_adc)
{
    if (!state->initialized) {
        state->previous_adc = current_adc;
        state->is_valid     = true;
        state->initialized  = true;
        return;
    }
    // Detect jump beyond allowed step
    if ((uint16_t)abs((int32_t)current_adc - (int32_t)state->previous_adc) > APP_CHANNEL_MAX_STEP) {
        state->is_valid = false;
    }
    state->previous_adc = current_adc;
}

void ProcessAcceleratorPedalChannels(const uint16_t raw_adc_readings[ADC_BUFFER_LENGTH],
                                     volatile uint16_t *out_filtered,
                                     volatile float    *out_percentage)
{
    uint32_t accumulator = 0;
    uint8_t  valid_count = 0;

    // Process each of the four ADC channels
    for (int channel = 0; channel < 4; channel++) {
        uint16_t raw_value = raw_adc_readings[channel];
        ValidateAndUpdateChannel(&pedalChannelStates[channel], raw_value);

        if (pedalChannelStates[channel].is_valid) {
            // Map raw ADC to application output range
            uint16_t mapped = map_unsigned(
                raw_value,
                pedalCalibrationData.zero_adc[channel],
                pedalCalibrationData.full_adc[channel],
				APPS_RANGE_MIN,
				APPS_RANGE_MAX
            );
            // Apply FIR filtering to mapped value
            uint16_t filtered = Combined_Filter(&pedalChannelFilters[channel], kFIRCoefficients, mapped);

            accumulator += filtered;
            valid_count++;
        }
    }

    // Compute final averaged output or report error
    if (valid_count == 0) {
    	// IMPLAUSIBILITY ERROR - The error persists for more than 5 cycles
    	if (++implausibilityCounter >= 5) HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14); // RED Led
    } else {
    	implausibilityCounter = 0;
        *out_filtered   = (uint16_t)(accumulator / valid_count);
        *out_percentage = (float)(*out_filtered) / (float)APPS_RANGE_MAX;
    }
}

/**
 * @brief Compute a simple CRC32 checksum over a data block.
 *
 * Performs a bitwise XOR over each byte, starting from 0xFFFFFFFF.
 * Used to verify integrity of stored calibration data.
 *
 * @param data Pointer to the data buffer to checksum.
 * @param len  Number of bytes to include in the CRC calculation.
 * @return CRC32 value for the specified data range.
 */
uint32_t CalculateCRC32(const uint8_t *data, size_t len)
{
    uint32_t crc = 0xFFFFFFFFU;
    while (len--) crc ^= *data++;
    return crc;
}

/**
 * @brief Load pedal calibration data from flash memory and verify it.
 *
 * This function retrieves previously saved calibration data — the ADC values
 * recorded at pedal rest (zero_adc) and full press (full_adc) — from a
 * predefined flash memory address. To ensure the integrity of the data,
 * a CRC32 checksum is computed over the read content (excluding the CRC itself)
 * and compared against the stored checksum.
 *
 * If the CRC matches, the data is copied into the RAM structure used at runtime.
 *
 * @return true if the calibration data is valid and successfully loaded,
 *         false if the CRC check fails.
 */

bool LoadPedalCalibration(void)
{
	PedalCalibrationData *stored = (PedalCalibrationData*)PEDAL_CALIB_FLASH_ADDR;
    uint32_t crc = CalculateCRC32((uint8_t*)stored, sizeof(*stored) - sizeof(stored->crc32));
    if (crc != stored->crc32) return false;

    memcpy(pedalCalibrationData.zero_adc, stored->zero_adc, sizeof(stored->zero_adc));
    memcpy(pedalCalibrationData.full_adc, stored->full_adc, sizeof(stored->full_adc));
    pedalCalibrationData.crc32 = stored->crc32;
    return true;
}

/**
 * @brief Save the current pedal calibration data to flash memory.
 *
 * This function creates a local copy of the current calibration structure,
 * computes a CRC32 over its contents (excluding the CRC field itself),
 * and writes the full structure to a predefined flash memory sector.
 * Flash memory is first unlocked and erased before being programmed word-by-word.
 *
 * This process ensures that valid calibration data can be reliably restored
 * after a power cycle or reset.
 *
 * @return true if the flash write completed successfully, false otherwise.
 */

bool SavePedalCalibration(void)
{
    PedalCalibrationData cfg = pedalCalibrationData;
    cfg.crc32 = CalculateCRC32((uint8_t*)&cfg, sizeof(cfg) - sizeof(cfg.crc32));

    HAL_FLASH_Unlock();
    FLASH_Erase_Sector(PEDAL_CALIB_FLASH_SECTOR, VOLTAGE_RANGE_3);
    uint32_t *ptr = (uint32_t*)&cfg;
    for (size_t i = 0; i < sizeof(cfg)/4; i++) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
                              PEDAL_CALIB_FLASH_ADDR + 4*i,
                              ptr[i]) != HAL_OK) {
            HAL_FLASH_Lock();
            return false;
        }
    }
    HAL_FLASH_Lock();
    return true;
}
