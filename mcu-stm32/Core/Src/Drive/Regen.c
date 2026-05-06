/**
 * @file Regen.c
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief Regenerative braking implementation
 */

#include "Drive/Regen.h"
#include "Drive/BaseControlMotor.h"
#include <math.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#ifdef REGEN_ENABLED
#if REGEN_ENABLED

/* Latch regen selection per node_id to avoid mode chattering near pedal threshold. */
static bool s_regen_latch[3] = {false, false, false};

bool Regen_ShouldUseRegen(const Inverter_t *inv, uint8_t pedal_percent)
{

    /*
    if (pedal_percent <= REGEN_PEDAL_THRESHOLD_PCT && inv->speed_rpm > (int16_t)REGEN_SPEED_MIN_RPM) {
        // Below threshold: always allow regen (if speed is high enough, checked in the latch logic below)
        return true;
    }
    */

    if (inv == NULL || inv->node_id == 0U || inv->node_id >= 3U) {
        return false;
    }

    if (inv->state != INV_STATE_RUNNING) {
        s_regen_latch[inv->node_id] = false;
        return false;
    }



    /* Speed hysteresis on latch RESET:
     *   - Latch CAN DEACTIVATE only when speed drops below REGEN_SPEED_CRITICAL_RPM
     *     (already the intended exit threshold).
     *   - We do NOT reset the latch here if speed flickers
     *     around CRITICAL. The latch reset due to speed happens only when
     *     speed is strictly below CRITICAL — the lower-guard prevents
     *     re-triggering from noise.
     * The check below uses a 10% lower guard band so noise around 2000 RPM
     * does NOT cause repeated latch-reset -> regen-enter oscillations. */
    const int16_t speed_latch_reset_thr = (int16_t)(REGEN_SPEED_CRITICAL_RPM * 9U / 10U); /* 90% of CRITICAL = 1800 RPM */
    if (inv->speed_rpm <= speed_latch_reset_thr) {
        s_regen_latch[inv->node_id] = false;
        return false;
    }

    /* If latch is ON but speed fell below CRITICAL (but above the hard reset above),
     * keep the latch alive but let k_vel fade-out smooth the torque to ~0. */
    if (!s_regen_latch[inv->node_id] && inv->speed_rpm <= (int16_t)REGEN_SPEED_CRITICAL_RPM) {
        /* Speed is in [speed_latch_reset_thr, CRITICAL]: cannot enter regen */
        return false;
    }

    const uint8_t threshold = REGEN_PEDAL_THRESHOLD_PCT;
    const uint8_t hysteresis = REGEN_PEDAL_HYST_PCT;
    const uint8_t enter_thr = (threshold > hysteresis) ? (threshold - hysteresis) : 0U;
    const uint8_t exit_thr = ((uint16_t)threshold + (uint16_t)hysteresis <= 100U)
        ? (uint8_t)(threshold + hysteresis)
        : 100U;

    if (s_regen_latch[inv->node_id]) {
        if (pedal_percent >= exit_thr) {
            s_regen_latch[inv->node_id] = false;
        }
    } else {
        /* Enter regen only if pedal is low AND speed is high enough.
         * This creates speed hysteresis: enter at SPEED_MIN,
         * exit at SPEED_CRITICAL — avoids entering regen at
         * marginal speeds where k_vel fade-out makes it near-zero. */
        if (pedal_percent <= enter_thr && inv->speed_rpm > (int16_t)REGEN_SPEED_MIN_RPM) {
            s_regen_latch[inv->node_id] = true;
        }
    }

    return s_regen_latch[inv->node_id];
}

int16_t Regen_CalculateTorque(int16_t speed_rpm, uint8_t pedal_percent, uint16_t dc_voltage){
    
    #ifdef REGEN_FORCE_ENABLE
        /* Force regen for testing regardless of speed or pedal */
        return (int16_t)(-REGEN_TORQUE_MAX_NM / MOTOR_NOMINAL_TORQUE_NM * 1000.0f);
    #endif

    if (pedal_percent >= REGEN_PEDAL_THRESHOLD_PCT) {
        return 0;
    }

    if(speed_rpm < REGEN_SPEED_CRITICAL_RPM) {
        return 0;
    }
    
    
    /*  STAGE 1: Pedal-dependent linear ramping */
    /* T_req = -T_max_regen * (1 - pedal_pct / threshold)  [NEGATIVE = braking] */
    float pedal_norm = (float)pedal_percent / (float)REGEN_PEDAL_THRESHOLD_PCT;
    float torque_stage1_nm = -REGEN_TORQUE_MAX_NM * (1.0f - pedal_norm);
    
    /*  STAGE 2: Speed-dependent fade-out (low speeds)  */
    /* k_vel = min(1, speed_current / speed_min) */
    float speed_min = (float)REGEN_SPEED_MIN_RPM;
    float k_vel = (speed_rpm < speed_min) ? ((float)speed_rpm / speed_min) : 1.0f;
    
    float torque_stage2_nm = torque_stage1_nm * k_vel;
    
    /*  STAGE 3: Battery power limit  */
    /* |T_max| <= P_batt_max / omega   [Nm] */
    float omega_rad_s = 2.0f * M_PI * (float)speed_rpm / 60.0f;
    float pbatt_max = (float)REGEN_PBATT_MAX_W;
    float torque_power_limit_nm = pbatt_max / omega_rad_s;
    
    /* Clamp: torque_stage2_nm is negative, limit is positive → check lower bound */
    if (torque_stage2_nm < -torque_power_limit_nm) {
        torque_stage2_nm = -torque_power_limit_nm;
    }

    /* STAGE 4: DC Bus Voltage Derating (Soft-clipping) */
    /* Reduce regen from VCU before inverter hardware intervention */
    float voltage_factor = 1.0f;
    
    if (dc_voltage >= REGEN_DC_DERATE_END_V) {
        voltage_factor = 0.0f; /* Voltage too high, turn off regen */
    } else if (dc_voltage > REGEN_DC_DERATE_START_V) {
        /* Calculate the percentage of linear reduction */
        voltage_factor = 1.0f - (float)(dc_voltage - REGEN_DC_DERATE_START_V) / 
                                (float)(REGEN_DC_DERATE_END_V - REGEN_DC_DERATE_START_V);
    }
    
    /* Apply voltage reduction to the torque calculated so far */
    torque_stage2_nm *= voltage_factor;
    

    /* CAN_unit = (Nm / MOTOR_NOMINAL_TORQUE_NM) * 1000 */
    int16_t torque_can_units = (int16_t)((torque_stage2_nm / MOTOR_NOMINAL_TORQUE_NM) * 1000.0f);
    
    /* Respect torque limits */
    if (torque_can_units < TORQUE_LIMIT_NEG) {
        torque_can_units = TORQUE_LIMIT_NEG;
    }
    if (torque_can_units > 0) {
        torque_can_units = 0;  /* Regeneration must be negative */
    }
    
    return torque_can_units;
}

#endif /* REGEN_ENABLED */
#endif
