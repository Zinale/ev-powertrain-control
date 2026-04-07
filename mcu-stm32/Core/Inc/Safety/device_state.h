#ifndef DEVICE_STATE_H
#define DEVICE_STATE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DEVICE_STATE_INIT = 0,
    DEVICE_STATE_RUNNING,
    DEVICE_STATE_ERROR
} DeviceState_t;

void DeviceState_Set(DeviceState_t new_state);
DeviceState_t DeviceState_Get(void);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_STATE_H */
