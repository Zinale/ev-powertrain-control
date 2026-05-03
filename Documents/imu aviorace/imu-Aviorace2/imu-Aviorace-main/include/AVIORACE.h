// AVIORACE.h
#ifndef AVIORACE_H
#define AVIORACE_H

#include <Arduino.h>

#define BASE_ID 0x00

// #define ACC_ID (BASE_ID + 0x100)
 #define ACC_ID 0x00
#define GYRO_ID  0x01
#define MAG_ID 0x02

#define REPROGAM_ID 0x0E5

typedef struct
{
    int16_t value_x;
    int16_t value_y;
    int16_t value_z;
    int16_t tail;
} imu_actual_values;

void init_device();
bool send_message(uint8_t *message, size_t address);
void set_imu();
void receive_message(int packetSize);
void process_actual_values(const imu_actual_values *actual, long packet_ID);

#endif
