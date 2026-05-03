// AVIORACE.cpp
#include <CAN.h>
#include <CANSAME5x.h>
#include "AVIORACE.h"

extern CANSAME5x CAN;
extern imu_actual_values actual_values;

void init_device()
{
    Serial.begin(115200);
    
    pinMode(PIN_CAN_STANDBY, OUTPUT);
    digitalWrite(PIN_CAN_STANDBY, false); // turn off STANDBY
    pinMode(PIN_CAN_BOOSTEN, OUTPUT);
    digitalWrite(PIN_CAN_BOOSTEN, true); // turn on booster

    // avvia il can bus a 500kbps
    if (!CAN.begin(500E3))
    {
        Serial.printf("Starting CAN failed!\n");
        // se la comunicazione non avviene correttamente il led lampeggia
        while (1)
        {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(200);
            digitalWrite(LED_BUILTIN, LOW);
            delay(200);
        }
    }

    set_imu();
}

bool send_message(uint8_t* message, size_t address)
{
    Serial.printf("Sending packet to ...0x%X\n", address);
    CAN.beginPacket(address);
    size_t bytesSent = CAN.write(message, 8);
    CAN.endPacket();
    return (bytesSent == 8);
}

void set_imu()
{
    //reprogram_data
    uint8_t reprogram_data[8] = {0x01, 0xA0, 0xAA, 0x02, 0x01, 0x01, 0x00, 0x00};
    //positioning_data
    uint8_t positioning_data[8] = {0x01, 0xA3, 0xAA, 0xAA, 0xAA, 0x00, 0x01, 0x01};
    //zero offset
    uint8_t zero_offset[8] = {0x01, 0xA1, 0xAA, 0xAA, 0xAA, 0xAA,0xAA, 0xAA};
    //messaggio relativo al reprogram data
    if(send_message(reprogram_data, REPROGAM_ID) && 
        send_message(positioning_data, REPROGAM_ID) &&
        send_message(zero_offset, REPROGAM_ID)){
        Serial.printf("Imu set!\n");
    }
    else{
        while(1){
            Serial.printf("Error setting imu!\n");
        }
    }

}


void receive_message(int packetSize)
{
    long packet_ID = CAN.packetId();
    Serial.printf("Received CAN packet from...0x%X\n", packet_ID);

    if (CAN.available() >= packetSize)
    {
        Serial.println("Packet received:");
        for (int i = 0; i < packetSize; i++)
        {
            Serial.print(CAN.read(), HEX);
            Serial.print(" ");
        }
        Serial.println();

      // Legge valori 16-bit da CAN bus in formato big-endian
            uint16_t value_x = (uint16_t)CAN.read() << 8 | (uint16_t)CAN.read();
            uint16_t value_y = (uint16_t)CAN.read() << 8 | (uint16_t)CAN.read();
            uint16_t value_z = (uint16_t)CAN.read() << 8 | (uint16_t)CAN.read();
            uint16_t tail    = (uint16_t)CAN.read() << 8 | (uint16_t)CAN.read();
        Serial.print("Value X: ");
        Serial.println((int) value_x);
        Serial.print("Value Y: ");
        Serial.println((int) value_y);
        Serial.print("Value Z: ");
        Serial.println((int) value_z);
        Serial.print("Tail: ");
        Serial.println(tail);

        // Check for overflow or underflow
        if (value_x > 32767 || value_x < -32768 ||
            value_y > 32767 || value_y < -32768 ||
            value_z > 32767 || value_z < -32768 ||
            tail > 32767 || tail < -32768)
        {
            Serial.println("Overflow or underflow detected!");
        }

      

        process_actual_values(&actual_values, packet_ID);
    }
    else
    {
        Serial.println("Invalid packet size");
        return;
    }
}
void process_actual_values(const imu_actual_values *actual, long packet_ID)
{
    if (packet_ID == ACC_ID)
    {
        Serial.printf(
            "Acceleration: %.3f, %.3f, %.3f, Serial: %d \n",
            actual->value_x,
            actual-> value_y,
            actual->value_z,
            actual->tail
        );
    }
    else if (packet_ID == GYRO_ID)
    {
        Serial.printf(
            "Gyroscope: %.1f, %.1f, %.1f, Internal Temp: %.3f \n",
            actual->value_x,
            actual->value_y,
            actual->value_z,
            actual->tail 
        );
    }
    else if (packet_ID == MAG_ID)
    {
        Serial.printf(
            "Magnetometer: %.2f, %.2f, %.2f \n",
            actual->value_x,
            actual->value_y,
            actual->value_z
        );
    }
    
}
