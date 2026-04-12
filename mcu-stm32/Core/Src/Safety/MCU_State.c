/**
 * @file MCU_State.c
 * @author Alessandro Zingaretti | Polimarche Racing Team - UNIVPM
 * @brief MCU top-level state machine — implementation
 */

#include "Safety/MCU_State.h"
#include "stm32f7xx_hal.h"   /* HAL_GetTick() */

/* ============================================================================
 * Internal state
 * ============================================================================ */

volatile MCU_State_t s_state      = MCU_STATE_INIT;
static volatile uint32_t    s_entry_tick = 0U;

/* ============================================================================
 * Private helper
 * ============================================================================ */

static void set_state(MCU_State_t next)
{
    if (next != s_state)
    {
        s_state      = next;
        s_entry_tick = HAL_GetTick();
    }
}

/* ============================================================================
 * Public API
 * ============================================================================ */

void MCU_State_Update(const MCU_StateInputs_t *in)
{
    const bool can_ok  = in->can1_ok && in->can2_ok;
    const bool sens_ok = in->apps_ok && in->sas_ok;

    switch (s_state)
    {
        case MCU_STATE_INIT:
            if (!can_ok)
            {
                set_state(MCU_STATE_ERROR);
            }
            else if (sens_ok)
            {
                set_state(MCU_STATE_READY);
            }
            else
            {
                set_state(MCU_STATE_IMPLAUSIBILITY);
            }
            break;

        case MCU_STATE_READY:
            if (!can_ok)
            {
                set_state(MCU_STATE_ERROR);
            }
            else if (!sens_ok)
            {
                set_state(MCU_STATE_IMPLAUSIBILITY);
            }
            else if (in->r2d_active && in->inv_ready)
            {
                set_state(MCU_STATE_RUNNING);
            }
            break;

        case MCU_STATE_RUNNING:
            if (!can_ok)
            {
                set_state(MCU_STATE_ERROR);
            }
            else if (!sens_ok)
            {
                set_state(MCU_STATE_IMPLAUSIBILITY);
            }
            else if (!in->r2d_active || !in->inv_ready)
            {
                /* R2D released or an inverter left RUNNING — hold in READY without torque */
                set_state(MCU_STATE_READY);
            }
            break;

        case MCU_STATE_IMPLAUSIBILITY:
            if (!can_ok)
            {
                set_state(MCU_STATE_ERROR);
            }
            else if (sens_ok)
            {
                /* Auto-recovery: APPS module requires pedal below threshold first */
                const bool can_run = in->r2d_active && in->inv_ready;
                set_state(can_run ? MCU_STATE_RUNNING : MCU_STATE_READY);
            }
            break;

        case MCU_STATE_ERROR:
            /* Recover once CAN_ServiceRecovery has re-initialised both buses */
            if (can_ok)
            {
                if (sens_ok)
                {
                    const bool can_run = in->r2d_active && in->inv_ready;
                    set_state(can_run ? MCU_STATE_RUNNING : MCU_STATE_READY);
                }
                else
                {
                    set_state(MCU_STATE_IMPLAUSIBILITY);
                }
            }
            break;

        default:
            /* Defensive: unexpected value → force INIT */
            set_state(MCU_STATE_INIT);
            break;
    }
}

MCU_State_t MCU_State_Get(void)
{
    return (MCU_State_t)s_state;
}

uint32_t MCU_State_GetEntryTick(void)
{
    return s_entry_tick;
}

bool MCU_IsTorqueAllowed(void)
{
    return (s_state == MCU_STATE_RUNNING);
}

bool MCU_IsSerialEnabled(void)
{
    /* Serial is active in all states — content varies by state */
    return true;
}

const char *MCU_State_ToString(MCU_State_t state)
{
    switch (state)
    {
        case MCU_STATE_INIT:            return "INIT";
        case MCU_STATE_READY:           return "READY";
        case MCU_STATE_RUNNING:         return "RUNNING";
        case MCU_STATE_IMPLAUSIBILITY:  return "IMPLAUSIBILITY";
        case MCU_STATE_ERROR:           return "ERROR";
        default:                        return "UNKNOWN";
    }
}
