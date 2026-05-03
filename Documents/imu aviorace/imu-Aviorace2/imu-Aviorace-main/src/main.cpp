#include <CANSAME5x.h>
#include <Arduino.h>
#include "AVIORACE.h"

CANSAME5x CAN;
imu_actual_values actual_values;

void setup() {
  init_device();

  CAN.onReceive(receive_message);
  
}

void loop() {
}
