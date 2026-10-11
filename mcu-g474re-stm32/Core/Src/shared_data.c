#include "shared_data.h"
#include "mutexes.h"
#include <string.h>

VehicleData_t g_vehicle_data;

void VehicleData_Init(void) {
    memset(&g_vehicle_data, 0, sizeof(VehicleData_t));
    g_vehicle_data.fsm_state = VEHICLE_STATE_INIT;
}

void VehicleData_Update(const VehicleData_t *src) {
    if (src == NULL) return;

    Mutex_VehicleData_Lock();
    memcpy(&g_vehicle_data, src, sizeof(VehicleData_t));
    Mutex_VehicleData_Unlock();
}

void VehicleData_GetSnapshot(VehicleData_t *dst) {
    if (dst == NULL) return;

    Mutex_VehicleData_Lock();
    memcpy(dst, &g_vehicle_data, sizeof(VehicleData_t));
    Mutex_VehicleData_Unlock();
}

/* ----------------------------------------------------------------------------
 * Campi generati da DriveLogicTask - elenco unico.
 * X(campo) e' usato sia per pubblicarli (PublishControl) sia per preservarli
 * dal memcpy di PublishSensors. Aggiungere qui ogni nuovo campo "control".
 * -------------------------------------------------------------------------- */
#define DRIVELOGIC_OWNED_FIELDS(X) \
    X(fsm_state)                   \
    X(r2d_armed)                  \
    X(torque_left_nm)             \
    X(torque_right_nm)            \
    X(inv_l_torque_cmd_raw)       \
    X(inv_r_torque_cmd_raw)       \
    X(k_slip_left)                \
    X(k_slip_right)              \
    X(T_max_RL)                  \
    X(T_max_RR)                  \
    X(lambda_ref_RL)              \
    X(lambda_ref_RR)              \
    X(lambda_RL)                 \
    X(lambda_RR)                 \
    X(tvc_weight)                \
    X(yaw_th)                    \
    X(regen_active)              \
    X(throttle_real)             \
    X(plausibility_faults)

/* ----------------------------------------------------------------------------
 * Pubblicazione parziale - DriveLogicTask
 * Scrive solo i campi "control". Nessun "lost update": DriveLogic e' l'unico
 * scrittore di questi campi.
 * -------------------------------------------------------------------------- */
void VehicleData_PublishControl(const VehicleData_t *src) {
    if (src == NULL) return;

    Mutex_VehicleData_Lock();
    #define X(f) g_vehicle_data.f = src->f;
    DRIVELOGIC_OWNED_FIELDS(X)
    #undef X
    Mutex_VehicleData_Unlock();
}

/* ----------------------------------------------------------------------------
 * Pubblicazione parziale - DataManagerTask
 * DataManager e' proprietario di quasi tutta la struct: copiamo l'intero src
 * ma PRESERVIAMO i campi DRIVELOGIC_OWNED_FIELDS, che nello snapshot di inizio
 * ciclo potrebbero essere gia' obsoleti.
 * -------------------------------------------------------------------------- */
void VehicleData_PublishSensors(const VehicleData_t *src) {
    if (src == NULL) return;

    Mutex_VehicleData_Lock();

    /* Salva i campi di competenza di DriveLogicTask (valore corrente). */
    #define X(f) __typeof__(g_vehicle_data.f) keep_##f = g_vehicle_data.f;
    DRIVELOGIC_OWNED_FIELDS(X)
    #undef X

    memcpy(&g_vehicle_data, src, sizeof(VehicleData_t));

    /* Ripristina i campi di DriveLogicTask: non li tocchiamo. */
    #define X(f) g_vehicle_data.f = keep_##f;
    DRIVELOGIC_OWNED_FIELDS(X)
    #undef X

    Mutex_VehicleData_Unlock();
}