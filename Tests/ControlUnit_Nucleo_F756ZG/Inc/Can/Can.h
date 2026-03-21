#ifndef __CAN_H
#define __CAN_H

/**
 * ***********************************************************************
 * CAN MODULE HEADER
 *
 * Includes necessary headers, defines macros, and declares variables and
 * functions that are visible to other modules including "CAN.h".
 *
 * Note: Define USE_SN65HVD230 to use the SN65HVD230 CAN bus transceiver;
 * if undefined, the MCP2515 transceiver code will be used.
 * ***********************************************************************
 */

// Required headers for the CAN module
#include "main.h"

// Transceiver selection for CAN communication
#define USE_SN65HVD230

// Structure for CAN message status flags, with volatile fields to ensure up-to-date reads/writes.
typedef struct {
    volatile uint8_t msgID_0x33;
    volatile uint8_t msgID_0x34;
    volatile uint8_t msgID_Inverter;
} CAN_StatusFlags;

// Structure definition for 3-axis vector (e.g., accelerometer and gyroscope data)
typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t z;
} Vector;

// Global variables for sensor data
extern Vector a; // Accelerometer vector
extern Vector g; // Gyroscope vector

// Initialization function for CAN interface
extern void CanInit(void);

// Functions for CAN message transmission
extern void Transmit_CAN_Message(CAN_HandleTypeDef *, uint32_t, uint32_t, uint8_t *);
void Transmit_CAN_Message_SN65HVD230(CAN_HandleTypeDef *, uint32_t, uint32_t, uint8_t *);

// Functions for CAN message reception
extern void Receive_CAN_Message(CAN_HandleTypeDef *);
void Receive_CAN_Message_SN65HVD230(CAN_HandleTypeDef *);

// Utility function to display CAN messages
extern void Display_CAN_Messages(void);

#endif /* __CAN_H */
