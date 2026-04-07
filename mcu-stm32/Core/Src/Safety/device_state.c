#include "Safety/device_state.h"

static volatile DeviceState_t s_device_state = DEVICE_STATE_INIT;

void DeviceState_Set(DeviceState_t new_state)
{
    s_device_state = new_state;
}

DeviceState_t DeviceState_Get(void)
{
    return (DeviceState_t)s_device_state;
}
