/* Core/Inc/Sensors/wss.h */
#ifndef WSS_H
#define WSS_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* Configurazione Ruota Fonica e Geometria */
#define WSS_NUM_TEETH               22.0f
#define WSS_WHEEL_CIRCUMFERENCE_M   1.8f
#define WSS_IC_BUF_SIZE             16U
#define WSS_TIMEOUT_CYCLES          60U    /* ~300ms a 200Hz -> ruota considerata ferma */
#define WSS_MA_WINDOW_SIZE          8U     /* Finestra media mobile per la velocità */
#define WSS_MEDIAN_WINDOW_SIZE       5U   /* Finestra dispari, più stretta della MA: rigetta un singolo fronte spurio senza appiattire la dinamica della velocità */

typedef struct {
    float rpm_fl;
    float rpm_fr;
    float speed_fl_kmh;
    float speed_fr_kmh;
    float vehicle_speed_kmh;
} WSS_Output_t;

void WSS_Init(TIM_HandleTypeDef *htim);
void WSS_Process(WSS_Output_t *out);

#endif /* WSS_H */