#include "cmsis_os2.h"
#include "shared_data.h"
#include "main.h"
#include "mutexes.h"

#include "Connectivity/Can.h"
#include "Control/inverter.h"
#include "Sensors/adc_manager.h"
#include "Sensors/apps.h"
#include "Sensors/sas.h"
#include "Sensors/bs.h"
#include "Sensors/wss.h"

extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim2;
extern IWDG_HandleTypeDef hiwdg;

#define DATA_MANAGER_PERIOD_MS  5U   /**< Task period: 5 ms -> 200 Hz        */
#define MS_TO_TICKS(ms) ((uint32_t)((ms) * osKernelGetTickFreq() / 1000U))

/* Chiamata da StartDataManager() in main.c: non ridefinire quella qui (duplicate symbol) */
void DataManagerTask(void) {
    ADC_Manager_Init(&hadc1);
    APPS_Init();
    SAS_Init();
    Brake_Init();
    WSS_Init(&htim2);

    uint32_t next_wake = osKernelGetTickCount();
    const uint32_t period_ticks = MS_TO_TICKS(DATA_MANAGER_PERIOD_MS);
    
    ADC_Buffer_t        adc_buf   = {0};
    APPS_Output_t       apps_out  = {0};
    SAS_Output_t        sas_out   = {0};
    Brake_Output_t      brake_out = {0};
    WSS_Output_t        wss_out   = {0};
    VehicleData_t       v_data;

    for (;;) {

        /* Svuota le code RX riempite dalle ISR CAN1/CAN2: qui gira in task context, i mutex sono ammessi */
        CAN_MOTOR_ProcessQueueDrain();
        CAN_MAIN_ProcessQueueDrain();

        if (ADC_Manager_GetBuffer(&adc_buf)) {
            APPS_Process(adc_buf.raw[ADC_CH_APPS1], adc_buf.raw[ADC_CH_APPS2], &apps_out);
            SAS_Process(adc_buf.raw[ADC_CH_STEERING], &sas_out);
            Brake_Process(adc_buf.raw[ADC_CH_BRAKE], &brake_out);
        }

        WSS_Process(&wss_out);

        VehicleData_GetSnapshot(&v_data);

        /* --- CAN2/bus MAIN: IMU, BMS, Dashboard --- */
        Mutex_FDCAN2_MAIN_Lock();
        CarData_t car_snapshot = *CAN_MAIN_GetData();
        Mutex_FDCAN2_MAIN_Unlock();

        uint32_t now_ms = HAL_GetTick();

        v_data.imu_accel_x_raw  = car_snapshot.imu_data.accel_x;
        v_data.imu_accel_y_raw  = car_snapshot.imu_data.accel_y;
        v_data.imu_accel_z_raw  = car_snapshot.imu_data.accel_z;
        v_data.imu_gyro_x_raw   = car_snapshot.imu_data.gyro_x;
        v_data.imu_gyro_y_raw   = car_snapshot.imu_data.gyro_y;
        v_data.imu_gyro_z_raw   = car_snapshot.imu_data.gyro_z;
        v_data.imu_mag_x_raw    = car_snapshot.imu_data.mag_x;
        v_data.imu_mag_y_raw    = car_snapshot.imu_data.mag_y;
        v_data.imu_mag_z_raw    = car_snapshot.imu_data.mag_z;
        v_data.last_imu_rx_tick = car_snapshot.imu_data.last_update_timestamp;
        v_data.imu_timeout_flag = (now_ms - car_snapshot.imu_data.last_update_timestamp) > CAN_TIMEOUT_MS;

        v_data.engine_map        = (uint8_t)car_snapshot.dashboard.engine_map;
        v_data.parameters_config = car_snapshot.dashboard.parameters_config;
        v_data.r2d_request       = (car_snapshot.dashboard.r2d != 0U);
        v_data.last_dash_rx_tick = car_snapshot.dashboard.last_update_timestamp;
        v_data.dash_timeout_flag = (now_ms - car_snapshot.dashboard.last_update_timestamp) > CAN_TIMEOUT_MS;

        v_data.bms_soc_percent     = (float)car_snapshot.bms_data.soc;
        v_data.bms_soh_percent     = (float)car_snapshot.bms_data.soh;
        v_data.bms_max_discharge_a = car_snapshot.bms_data.max_discharge;
        v_data.bms_max_regen_a     = car_snapshot.bms_data.max_regen;
        v_data.bms_sdc_status      = car_snapshot.bms_data.SDC_status;
        v_data.bms_error           = car_snapshot.bms_data.bms_error;
        v_data.last_bms_rx_tick    = car_snapshot.bms_data.last_update_timestamp;
        v_data.bms_timeout_flag    = (now_ms - car_snapshot.bms_data.last_update_timestamp) > CAN_TIMEOUT_MS;

        /* Corrente reale di pacco - sensore LEM CAB 500 (0x3C2), separato dal BMS */
        v_data.bms_pack_current    = car_snapshot.bms_data.pack_current_a;
        v_data.cab500_timeout_flag = (now_ms - car_snapshot.bms_data.cab500_last_update) > CAN_TIMEOUT_MS;

        v_data.PDM1_error       = car_snapshot.pdm_data.PDM1_error;
        v_data.PDM2_error       = car_snapshot.pdm_data.PDM2_error;
        v_data.last_pdm_rx_tick = car_snapshot.pdm_data.last_update_timestamp;
        v_data.pdm_timeout_flag = (now_ms - car_snapshot.pdm_data.last_update_timestamp) > CAN_TIMEOUT_MS;

        Mutex_FDCAN1_MOTOR_Lock();
        Inverter_t inv_l_snapshot = g_inverter_left;
        Inverter_t inv_r_snapshot = g_inverter_right;
        Mutex_FDCAN1_MOTOR_Unlock();

        /* Inverter SX */
        v_data.inv_l_status_word           = inv_l_snapshot.status_word;
        v_data.inv_l_speed_rpm             = inv_l_snapshot.speed_rpm;
        v_data.inv_l_torque                = inv_l_snapshot.actual_torque_nm;
        v_data.inv_l_motor_temp_degc       = inv_l_snapshot.motor_temp_degc;
        v_data.inv_l_temp_degc             = inv_l_snapshot.inverter_temp_degc;
        v_data.inv_l_igbt_temp_degc        = inv_l_snapshot.igbt_temp_degc;
        v_data.inv_l_dc_bus_voltage        = inv_l_snapshot.dc_bus_voltage;
        v_data.inv_l_error_code            = inv_l_snapshot.error_code;
        v_data.inv_l_error_info_1          = inv_l_snapshot.error_info_1;
        v_data.inv_l_state                 = (uint8_t)inv_l_snapshot.state;
        v_data.inv_l_raw_torque_current_iq = inv_l_snapshot.raw_torque_current_iq;
        v_data.inv_l_raw_magn_current_id   = inv_l_snapshot.raw_magn_current_id;
        v_data.inv_l_actual_power_w        = inv_l_snapshot.actual_power_w;

        /* Inverter DX */
        v_data.inv_r_status_word           = inv_r_snapshot.status_word;
        v_data.inv_r_speed_rpm             = inv_r_snapshot.speed_rpm;
        v_data.inv_r_torque                = inv_r_snapshot.actual_torque_nm;
        v_data.inv_r_motor_temp_degc       = inv_r_snapshot.motor_temp_degc;
        v_data.inv_r_temp_degc             = inv_r_snapshot.inverter_temp_degc;
        v_data.inv_r_igbt_temp_degc        = inv_r_snapshot.igbt_temp_degc;
        v_data.inv_r_dc_bus_voltage        = inv_r_snapshot.dc_bus_voltage;
        v_data.inv_r_error_code            = inv_r_snapshot.error_code;
        v_data.inv_r_error_info_1          = inv_r_snapshot.error_info_1;
        v_data.inv_r_state                 = (uint8_t)inv_r_snapshot.state;
        v_data.inv_r_raw_torque_current_iq = inv_r_snapshot.raw_torque_current_iq;
        v_data.inv_r_raw_magn_current_id   = inv_r_snapshot.raw_magn_current_id;
        v_data.inv_r_actual_power_w        = inv_r_snapshot.actual_power_w;


        v_data.last_inv_rx_tick = (inv_l_snapshot.last_rx_timestamp_ms > inv_r_snapshot.last_rx_timestamp_ms)
                                  ? inv_l_snapshot.last_rx_timestamp_ms : inv_r_snapshot.last_rx_timestamp_ms;
        v_data.inv_timeout_flag = ((now_ms - inv_l_snapshot.last_rx_timestamp_ms) > CAN_TIMEOUT_MS) ||
                                  ((now_ms - inv_r_snapshot.last_rx_timestamp_ms) > CAN_TIMEOUT_MS);

        v_data.can1_rx_cnt = g_can_rx_count;
        v_data.can2_rx_cnt = g_can2_rx_count;

        v_data.apps_percent        = (float)apps_out.final_percent;
        v_data.apps1_percent       = (float)apps_out.percent1;
        v_data.apps2_percent       = (float)apps_out.percent2;
        v_data.apps1_raw_adc       = apps_out.raw1;
        v_data.apps2_raw_adc       = apps_out.raw2;

        v_data.brake_percent       = brake_out.percent;
        v_data.brake_pressure_bar  = brake_out.pressure_bar;
        v_data.brake_raw_adc       = brake_out.raw_adc;
        v_data.brake_is_pressed    = brake_out.is_pressed;
        v_data.brake_is_hard       = brake_out.is_hard_braking;

        v_data.steering_angle_deg  = (float)sas_out.angle_deg;
        v_data.steering_raw_adc    = sas_out.raw_adc;
        v_data.wheel_speed_fl_kmh  = wss_out.speed_fl_kmh;
        v_data.wheel_speed_fr_kmh  = wss_out.speed_fr_kmh;
        v_data.vehicle_speed_kmh   = wss_out.vehicle_speed_kmh;
        
        v_data.apps_implausibility = apps_out.implausibility;
        v_data.apps1_oor           = apps_out.fault_oor1;
        v_data.apps2_oor           = apps_out.fault_oor2;
        v_data.sas_oor             = sas_out.fault_out_of_range;
        v_data.brake_oor           = brake_out.fault_out_of_range;

        v_data.sensor_fault_flag   = (apps_out.implausibility ||
                                      sas_out.fault_out_of_range ||
                                      brake_out.fault_out_of_range);

        /* Pubblica solo i campi generati da DataManagerTask (sensori, IMU,
         * telemetria inverter, BMS/Dash/PDM, statistiche CAN). I campi di
         * DriveLogicTask (FSM + coppie) NON vengono toccati. */
        VehicleData_PublishSensors(&v_data);
        HAL_IWDG_Refresh(&hiwdg);

        next_wake += period_ticks;
        osDelayUntil(next_wake);
    }

}