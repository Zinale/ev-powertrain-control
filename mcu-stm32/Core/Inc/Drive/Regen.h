/**
 * @file Regen.h
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief Regenerative braking logic for AMK inverters
 *
 * Implements single-pedal regenerative braking with:
 *   - Stage 1: Pedal-dependent linear torque
 *   - Stage 2: Speed-dependent fade-out
 *   - Stage 3: Battery power limit
 *   - Stage 4: DC bus voltage derating
 *   - Latch-based hysteresis to prevent mode chattering
 */

#ifndef REGEN_H
#define REGEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "Drive/Inverter.h"
#include "Config.h"

#ifdef REGEN_ENABLED
#if REGEN_ENABLED

/**
 * @brief Determine if the inverter should use regenerative braking
 *
 * Uses pedal position, speed, and a latching mechanism with hysteresis
 * to avoid chattering between traction and regen modes.
 *
 * @param inv           Pointer to inverter struct
 * @param pedal_percent Current pedal position [0-100%]
 * @return true if regen should be applied, false for traction
 */
bool Regen_ShouldUseRegen(const Inverter_t *inv, uint8_t pedal_percent);

/**
 * @brief Calculate regenerative braking torque
 *
 * Four-stage formula:
 *   1. Pedal-dependent: T = -T_max * (1 - pedal / threshold)
 *   2. Speed fade-out:  k_vel = min(1, speed / speed_min)
 *   3. Power limit:     |T| <= P_batt_max / omega
 *   4. DC bus derating: soft-clip at high voltage
 *
 * @param speed_rpm     Current motor speed [RPM]
 * @param pedal_percent Current pedal position [0-100%]
 * @param dc_voltage    Current DC bus voltage [0.1V]
 * @return Regenerative torque [0.1% Mn] (negative value)
 */
int16_t Regen_CalculateTorque(int16_t speed_rpm,
                              uint8_t pedal_percent,
                              uint16_t dc_voltage);

#endif /* REGEN_ENABLED */
#endif

#ifdef __cplusplus
}
#endif

#endif /* REGEN_H */
