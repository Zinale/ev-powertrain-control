#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * Macchina a Stati Principale del Veicolo (FSM)
 * ============================================================================ */
typedef enum {
    VEHICLE_STATE_INIT = 0,
    VEHICLE_STATE_READY,
    VEHICLE_STATE_RUNNING,
    VEHICLE_STATE_IMPLAUSIBILITY,
    VEHICLE_STATE_CAR_ERROR,        /* Timeout comunicazione CAN (Inverter/BMS/Dash/PDM/IMU) */
    VEHICLE_STATE_ERROR
} VehicleState_t;

/* ============================================================================
 * Struttura Dati Globale Condivisa
 *============================================================================ */
typedef struct {
    /* --- 1. Pedale Acceleratore (APPS) --- */
    float    apps_percent;          /* Acceleratore aggregato finale [0.0 - 100.0 %] */
    float    apps1_percent;         /* Percentuale APPS1 [0.0 - 100.0 %] */
    float    apps2_percent;         /* Percentuale APPS2 [0.0 - 100.0 %] */
    uint16_t apps1_raw_adc;         /* Valore grezzo ADC APPS1 */
    uint16_t apps2_raw_adc;         /* Valore grezzo ADC APPS2 */

    /* --- 2. Circuito Idraulico Freno (BSE) --- */
    float    brake_percent;         /* Pressione normalizzata pedale [0.0 - 100.0 %] */
    float    brake_pressure_bar;    /* Pressione idraulica [0.0 - 170.0 bar] */
    uint16_t brake_raw_adc;         /* Valore grezzo ADC Freno */
    bool     brake_is_pressed;      /* True se freno > 5 bar (accensione stop / interlock) */
    bool     brake_is_hard;         /* True se freno > 20 bar (implausibilità APPS > 25%) */

    /* --- 3. Angolo di Sterzo (SAS) & Ruote Foniche (WSS) --- */
    float    steering_angle_deg;    /* Angolo volante [-110.0° SX, +110.0° DX] */
    uint16_t steering_raw_adc;      /* Valore grezzo ADC Sterzo */
    float    wheel_speed_fl_kmh;    /* Velocità ruota fonica SX [km/h] */
    float    wheel_speed_fr_kmh;    /* Velocità ruota fonica DX [km/h] */
    float    vehicle_speed_kmh;     /* Velocità media stimata [km/h] */
    bool     sensor_fault_flag;     /* True se almeno un sensore è fuori range / implausibile (usato dalla FSM) */

    /* Dettaglio per sensore (scritti da DataManager), per i bit 2 e 4-7 di 0x006 */
    bool     apps_implausibility;   /* Latch APPS: mismatch APPS1/APPS2 o fuori range */
    bool     apps1_oor;             /* APPS1 fuori range (debounced) */
    bool     apps2_oor;             /* APPS2 fuori range (debounced) */
    bool     sas_oor;               /* SAS fuori range (debounced) */
    bool     brake_oor;             /* BS fuori range (debounced) */

    /* --- IMU (Ricevuta via FDCAN2, gia' pre-scalata e troncata in Can.c) --- */
    int16_t  imu_accel_x_raw;       /* Accelerazione longitudinale */
    int16_t  imu_accel_y_raw;       /* Accelerazione laterale */
    int16_t  imu_accel_z_raw;       /* Accelerazione verticale */
    int16_t  imu_gyro_x_raw;        /* Velocità angolare di rollio */
    int16_t  imu_gyro_y_raw;        /* Velocità angolare di beccheggio */
    int16_t  imu_gyro_z_raw;        /* Velocità di imbardata */
    int16_t  imu_mag_x_raw;
    int16_t  imu_mag_y_raw;
    int16_t  imu_mag_z_raw;

    /* --- 4. Stato Controllo & Powertrain (Scritti da DriveLogic @ 100 Hz) --- */
    VehicleState_t fsm_state;       /* Stato FSM corrente (= stato R2D autoritativo verso dash/PDM) */
    bool     r2d_armed;            /* Latch interno "il pilota vuole la macchina armata" */
    float    torque_left_nm;        /* Coppia uscita dal blocco Drive Controller NO Setpoint motore SX [Nm] */
    float    torque_right_nm;       /* Coppia uscita dal blocco Drive Controller NO Setpoint motore DX [Nm] */

    /* --- 5. Telemetria Inverter / Motori (Ricevuti via FDCAN1) --- */
    uint16_t inv_l_status_word;
    uint16_t inv_r_status_word;
    int16_t  inv_l_speed_rpm;
    int16_t  inv_r_speed_rpm;
    float    inv_l_torque; /* Coppia motore SX misurata dall'inverter [Nm], < 0 = regen */
    float    inv_r_torque; /* Coppia motore DX misurata dall'inverter [Nm], < 0 = regen */
    /* Temperature inverter/motore/IGBT: valore grezzo del drive in 0.1 °C (nome storico "_degc") */
    int16_t  inv_l_temp_degc;
    int16_t  inv_r_temp_degc;
    int16_t  inv_l_igbt_temp_degc;
    int16_t  inv_r_igbt_temp_degc;
    int16_t  inv_l_motor_temp_degc;
    int16_t  inv_r_motor_temp_degc;
    uint16_t inv_l_dc_bus_voltage;
    uint16_t inv_r_dc_bus_voltage;
    uint16_t inv_l_error_code;
    uint16_t inv_r_error_code;
    uint32_t inv_l_error_info_1;
    uint32_t inv_r_error_info_1;
    uint8_t  inv_l_state;
    uint8_t  inv_r_state;
    int16_t  inv_l_torque_cmd_raw;  /* Ultimo setpoint coppia SX trasmesso [0.1% Mn], per debug DataLogger */
    int16_t  inv_r_torque_cmd_raw;  /* Ultimo setpoint coppia DX trasmesso [0.1% Mn] */
    int16_t  inv_l_raw_torque_current_iq;
    int16_t  inv_l_raw_magn_current_id;
    int16_t  inv_r_raw_torque_current_iq;
    int16_t  inv_r_raw_magn_current_id;
    uint32_t inv_l_actual_power_w;
    uint32_t inv_r_actual_power_w;

    /* --- 6. Dati Batteria/BMS & Dash (Ricevuti via FDCAN2) --- */
    float    bms_pack_voltage;
    float    bms_pack_current;      /* Corrente reale di pacco [A], + = scarica (sensore LEM CAB 500, 0x3C2) */
    bool     cab500_timeout_flag;   /* True = nessun frame 0x3C2 da > CAN_TIMEOUT_MS */
    float    bms_soc_percent;
    float    bms_soh_percent;
    uint16_t bms_max_discharge_a;
    uint16_t bms_max_regen_a;
    bool     bms_sdc_status;        /* True = SDC chiuso */
    int16_t  bms_temp_max_degc;
    uint8_t  engine_map;            /* Mappa motore selezionata dal volante (0x106) */
    uint8_t  parameters_config;    /* Configurazione dei parametri selezionata dal volante (0x106) */
    bool     r2d_request;           /* RICHIESTA R2D dal volante (toggle su 0x106 b0) - non e' lo stato */

    /* --- 4b. Debug Controlli (Scritti da DriveLogic, unità pronte per la CAN) --- */
    int16_t  k_slip_left;       /* K_slips[0] * 100  [Nm*0.01] - coppia sottratta da SRC RL */
    int16_t  k_slip_right;      /* K_slips[1] * 100  [Nm*0.01] */
    bool     regen_active;      /* driveController_Y.regen != 0 */
    int8_t   throttle_real;     /* Throttle_real dopo mappa pedale [0-100 %] */
    int16_t  T_max_RL;          /* Grip Estimator T_RL * 100 [Nm*0.01] */
    int16_t  T_max_RR;          /* Grip Estimator T_RR * 100 [Nm*0.01] */
    uint8_t  lambda_ref_RL;         /* lambda[0] * 100 [0-100]  (vedi nota drive_logic.c) */
    uint8_t  lambda_ref_RR;         /* lambda[1] * 100 */
    uint8_t  lambda_RL;         /* lambda[2] * 100 */
    uint8_t  lambda_RR;         /* lambda[3] * 100 */
    uint8_t  tvc_weight;        /* TVC_Target_Weight * 100 [0-100] */
    int16_t  yaw_th;            /* yaw_th * 100 [rad/s * 0.01] */
    uint32_t plausibility_faults; /* bitmask PlausibilityFault_t (0 = ok) */



    bool     bms_error;
    bool PDM1_error;       // PDM1 error flag
    bool PDM2_error;       // PDM2 error flag


    /* --- 7. Statistiche Bus CAN e Watchdog --- */
    uint32_t can1_rx_cnt;
    uint32_t can2_rx_cnt;
    uint32_t last_inv_rx_tick;
    uint32_t last_bms_rx_tick;
    uint32_t last_imu_rx_tick;
    uint32_t last_dash_rx_tick;
    uint32_t last_pdm_rx_tick;
    bool     inv_timeout_flag;
    bool     bms_timeout_flag;
    bool     imu_timeout_flag;
    bool     dash_timeout_flag;
    bool     pdm_timeout_flag;

} VehicleData_t;

extern VehicleData_t g_vehicle_data;

/* ============================================================================
 * Proprieta' dei campi (chi SCRIVE cosa)
 * ----------------------------------------------------------------------------
 * Regola: ogni task pubblica SOLO i campi che genera, cosi' un task "lento"
 * non riscrive con dati vecchi (presi dallo snapshot a inizio ciclo) i campi
 * che un altro task ha aggiornato nel frattempo.
 *
 *  DataManagerTask  (200 Hz) -> tutti i sensori, IMU, telemetria inverter,
 *                               BMS/Dash/PDM, statistiche CAN, timeout flag,
 *                               engine_map / parameters_config / r2d_request.
 *                               Pubblica con VehicleData_PublishSensors().
 *
 *  DriveLogicTask   (100 Hz) -> fsm_state, torque_left_nm, torque_right_nm,
 *                               inv_l/r_torque_cmd_raw + tutta la sezione 4b
 *                               "Debug Controlli".
 *                               Pubblica con VehicleData_PublishControl().
 *
 *  DataLoggerTask   (10 Hz)  -> nessun campo (sola lettura).
 * ============================================================================ */

void VehicleData_Init(void);

/* Snapshot atomico completo (lettura) - usato da tutti i task come punto di
 * partenza del ciclo e dal DataLogger. */
void VehicleData_GetSnapshot(VehicleData_t *dst);

/* Scrittura atomica completa - da usare SOLO in fase di init / test, mai dai
 * task periodici (sovrascriverebbe i campi di un altro task). */
void VehicleData_Update(const VehicleData_t *src);

/* Pubblicazioni parziali: copiano nel globale solo i campi di competenza del
 * task chiamante, sotto mutex. */
void VehicleData_PublishSensors(const VehicleData_t *src);   /* DataManagerTask */
void VehicleData_PublishControl(const VehicleData_t *src);   /* DriveLogicTask  */

#endif /* SHARED_DATA_H */