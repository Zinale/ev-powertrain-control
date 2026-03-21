#ifndef __TASKS_H
#define __TASKS_H

/* ***********************************************************************
 * Required Headers for Module Integration
 * ***********************************************************************
 *
 * This module includes "main.h", which provides essential system-wide
 * configurations and function prototypes. Importantly, including
 * "main.h" also brings in "Scheduler.h", allowing access to scheduling
 * functions and timing mechanisms necessary for effective task management
 * within the system.
 *
 * Ensure that any module utilizing core functionalities or scheduling
 * relies on this header for consistent operation and resource access.
 *********************************************************************** */
#include "main.h"

/* ***********************************************************************
 * Module Export Definitions and Constants
 * ***********************************************************************
 *
 * This section declares definitions, constants, and macros that are
 * exported for use across different modules. Proper organization and
 * clear documentation of these elements facilitate maintainability
 * and enhance code clarity.
 *
 *********************************************************************** */

// *************************************************************************
// Buffer Lengths
// *************************************************************************

#define ADC_BUFFER_LENGTH 5  // Length of the ADC buffer

// *************************************************************************
// Sensor Calibration Constants
// *************************************************************************

// Accelerator Pedal Position Sensor (APPs) 1 Calibration
//#define APPS_1_ZERO_POSITION 2220+15  // ADC value corresponding to 0% pedal position
//#define APPS_1_FULL_POSITION 2460-30  // ADC value corresponding to 100% pedal position

// Accelerator Pedal Position Sensor (APPs) 2 Calibration
//#define APPS_2_ZERO_POSITION 3205+35  // ADC value corresponding to 0% pedal position
//#define APPS_2_FULL_POSITION 3538-30  // ADC value corresponding to 100% pedal position

// APPs Output Range
#define APPS_RANGE_MIN 0      // Minimum output value representing 0% pedal position
#define APPS_RANGE_MAX 2000   // Maximum output value representing 100% pedal position

// Steering Sensor Calibration
#define STEERING_ZERO_POSITION 840   // ADC value corresponding to center (0°) steering position
#define STEERING_FULL_POSITION 3780  // ADC value corresponding to full lock steering position
#define STEERING_RANGE_MIN 110      // Minimum output value representing full left lock (degrees)
#define STEERING_RANGE_MAX -110       // Maximum output value representing full right lock (degrees)

// *************************************************************************
// Threshold and Limit Constants
// *************************************************************************

// Calculated threshold for APPs implausibility detection
//#define APPS_IMPLAUSIBILITY_THRESHOLD (APPS_2_ZERO_POSITION - APPS_1_ZERO_POSITION + 150)

//#define MAX_RPM 2000       // Maximum allowable RPM for the motor
#define TORQUE_POS 1000    // Positive torque limit in 0.1% of nominal torque
#define TORQUE_NEG 0       // Negative torque limit in 0.1% of nominal torque

// *************************************************************************
// Mathematical Constants
// *************************************************************************

#define PI 3.141592653589793  // Value of π

// *************************************************************************
// Vehicle Parameters
// *************************************************************************

#define STEERING_RATIO 4.2f     // Steering ratio: degrees of steering wheel turn per degree of wheel turn
#define WHEEL_RADIUS 0.3f       // Radius of the wheel in meters
#define TRACK_WIDTH 1.5f        // Distance between the left and right wheels in meters
#define WHEEL_BASE 2.5f         // Distance between the front and rear axles in meters

// Increase it (> 1) for a more aggressive differential i.e. a greater correction with a greater difference between the speeds of the two wheels
#define DIFFERENTIAL_GAIN 3

/* ***********************************************************************
 * Constants & Types
 * ***********************************************************************/
// Threshold for detecting unreasonable jumps in a single channel
#define APP_CHANNEL_MAX_STEP   800U   // Maximum permitted ADC delta between samples

// Flash storage definitions (sector/address) - adapt to MCU
#define PEDAL_CALIB_FLASH_ADDR   ((uint32_t)0x080C0000U)
#define PEDAL_CALIB_FLASH_SECTOR FLASH_SECTOR_7

// *************************************************************************
// Data Structures
// *************************************************************************

/**
 * @brief Structure to hold RPM values for the motors controlling the inner and outer wheels.
 */
typedef struct {
    uint16_t rpm_motor_inner;  // RPM for the motor driving the inner wheel during a turn
    uint16_t rpm_motor_outer;  // RPM for the motor driving the outer wheel during a turn
} MotorRPM;

/**
 * @brief Maintains state for one raw ADC channel of the accelerator pedal.
 *
 * previous_adc:  Last raw ADC reading from this channel.
 * is_valid:     Indicates if this channel reading has passed plausibility checks.
 * initialized:  Flag indicating initial history has been set.
 */
typedef struct {
    uint16_t previous_adc;
    bool     is_valid;
    bool     initialized;
} PedalChannelState;

/**
 * @brief Stores calibration endpoints for each ADC channel.
 *
 * zero_adc: Raw ADC value at pedal minimum (rest).
 * full_adc: Raw ADC value at pedal maximum (full depression).
 * crc32:    CRC for calibration integrity check.
 */
typedef struct {
    uint16_t zero_adc[4];
    uint16_t full_adc[4];
    uint32_t crc32;
} PedalCalibrationData;

/* ***********************************************************************
 * Exported variables for the Tasks module
 * ***********************************************************************/

extern uint16_t adc_buf[ADC_BUFFER_LENGTH]; // Raw readings from each sensor

extern PedalCalibrationData pedalCalibrationData;
extern volatile bool zeroCollectionActive;
extern volatile bool fullCollectionActive;

extern volatile int16_t RPMs;

/* ***********************************************************************
 * Exported functions for the Tasks module
 * ***********************************************************************/

extern void TaskInit(void);

extern void TaskFast(void);
extern void TaskSetpointsCyclic(void); // Cyclic transmission of Setpoints1 (every 5ms)
extern void TaskMed(void);
extern void TaskSlow(void);

extern void Display_Message(UART_HandleTypeDef *huart, const char *format, ...);

/* ***********************************************************************
 * Private functions for the Tasks module
 * ***********************************************************************/

void AcquireSensorValues();

void HandleInverterCommunication();
MotorRPM CalculateMotorRPM(uint16_t baseMotorRPM, int8_t steeringWheelAngleDeg);

/**
 * @brief Initialize pedal processing: filters and load calibration.
 */
void InitializePedalModule(void);

/**
 * @brief Perform reading validation, mapping, filtering, and aggregation for all four
 *        accelerator pedal channels. Produces a single filtered pedal output and
 *        normalized percentage. Sets error_flag if no channel remains valid.
 *
 * @param raw_adc_readings  Array of 4 raw ADC samples [channel0..channel3].
 * @param out_filtered      Pointer to store final filtered pedal value (0..APP_OUTPUT_MAX).
 * @param out_percentage    Pointer to store normalized pedal position (0.0f..1.0f).
 */
void ProcessAcceleratorPedalChannels(const uint16_t raw_adc_readings[ADC_BUFFER_LENGTH],
									 volatile uint16_t *out_filtered,
									 volatile float    *out_percentage);

/**
 * @brief Calculate the CRC32 checksum for a given data buffer.
 *
 * @param data Pointer to the data buffer to calculate the CRC32 for.
 * @param len Length of the data buffer in bytes.
 *
 * @return The calculated CRC32 checksum.
 */
uint32_t CalculateCRC32(const uint8_t *data, size_t len);

/**
 * @brief Load calibration data from flash into ram struct.
 *
 * @return true if calibration data was successfully retrieved and CRC checked.
 */
bool LoadPedalCalibration(void);

/**
 * @brief Save calibration data from ram struct to flash memory.
 *
 * @return true if data successfully programmed to flash.
 */
extern bool SavePedalCalibration(void);

#endif /* __TASKS_H */
