#ifndef DRIVE_PROFILES_H
#define DRIVE_PROFILES_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * Configurazione powertrain selezionata dal volante (frame CAN 0x106)
 * ----------------------------------------------------------------------------
 *  1) engine_map (byte 1) -> MAPPA POWERTRAIN = colore pagina dash.
 *     Il volante (task_can.c: tx_data[1] = SR_map) invia:
 *       1 = ECO    (verde)  coppia 50% nominale, regen tetto 4 Nm, pedale morbido
 *       2 = NORMAL (blu)    coppia nominale 9.8 Nm, regen tetto 6 Nm
 *       3 = GAS    (rosso)  coppia picco 21 Nm, regen tetto 9 Nm, pedale pronto
 *     DriveProfiles_ResolveMap() traduce 1/2/3 -> PowertrainMap_t (0/1/2 interno).
 *
 *  2) parameters_config (byte 2, uint8) -> 4 campi da 2 bit, valore 0..3
 *       (il dash li mostra come 1..4)
 *       slot 0 = P1 REGEN   0=OFF 1=debole(40%) 2=media(70%) 3=piena(100%)
 *       slot 1 = P2 SRC     0=off .. 3=aggressivo (PID+mu su, overslip giu')
 *       slot 2 = P3 TVC     0=off  1=-60%  2=target  3=+60%
 *       slot 3 = P4 EVENT   agisce su Kus: 0->0  1->0.005  2->0.01  3->-0.005
 * ============================================================================ */

/* Valori grezzi inviati dal volante su 0x106 byte 1 */
#define DASH_ENGINE_MAP_ECO     1
#define DASH_ENGINE_MAP_NORMAL  2
#define DASH_ENGINE_MAP_GAS     3

typedef enum {
    PWR_MAP_ECO    = 0,   /* indice interno (usato per k_map_def[], k_pedal_map_select[]) */
    PWR_MAP_NORMAL = 1,
    PWR_MAP_GAS    = 2
} PowertrainMap_t;

/* Parametri di controllo derivati dai selettori. Vivono in un modulo locale
 * (static in drive_profiles.c), non nella struct condivisa fra task. */
typedef struct {
    PowertrainMap_t map;
    uint8_t  parameters_config;       /* byte config grezzo applicato           */

    /* --- Mappa powertrain --- */
    float    t_peak_nm;               /* cap coppia trazione / motore -> T_peak_p */
    float    regen_ceiling_nm;        /* tetto regen della mappa (base per P1)   */
    float    pedal_alpha;             /* esponente curva pedale (informativo)    */
    uint8_t  pedal_map_select;        /* -> pedal_map_select_p (1=ECO 2=NORMAL 0=GAS) */

    /* --- P1: REGEN --- */
    uint8_t  regen_level;
    bool     regen_enabled;
    float    regen_t_max_nm;          /* -> regen_T_max_p                        */
    float    regen_pedal_hyst;        /* -> regen_pedal_hyst_p                   */
    float    regen_pedal_thr;         /* -> regen_pedal_thr_p                    */

    /* --- P2: SRC (Slip/Traction Control) --- */
    uint8_t  src_level;
    float    slip_kp, slip_ki, slip_kd;  /* -> slip_K*_p                         */
    float    mu;                          /* -> mu_p                             */
    float    overslip_factor;             /* -> overslip_factor_p                */

    /* --- P3: TVC (Torque Vectoring) --- */
    uint8_t  tvc_level;
    float    tvc_kp, tvc_ki, tvc_kd;     /* -> tvc_K*_p                          */

    /* --- P4: EVENT --- */
    uint8_t  event_level;
    float    kus;                        /* -> Kus_p                             */
} ProfileSettings_t;

/* Grandezze elettriche runtime spinte nel modello ad ogni ciclo (come V_dc).
 * Correnti = limiti TOTALI di pacco (il modello divide /2 fra i due motori). */
typedef struct {
    float bus_voltage;         /* -> V_dc_p            [V]  */
    float i_discharge_max_a;   /* -> I_discharge_max_p [A]  (BMS max scarica) */
    float i_charge_max_a;      /* -> I_charge_max_p    [A]  (BMS max regen)   */
} PowertrainLimits_t;

/* Default sicuro (ECO, tutti i parametri a livello 1). Cattura anche i
 * guadagni base del controllore: chiamare DOPO driveController_initialize(). */
void DriveProfiles_Init(void);

/* Traduce il byte grezzo del volante nella mappa logica.
 * Valori fuori range -> PWR_MAP_ECO. */
PowertrainMap_t DriveProfiles_ResolveMap(int8_t raw_engine_map);

/* Entry point periodico (DriveLogicTask, 100 Hz): risolve mappa + config,
 * riprogramma il controllore solo se qualcosa e' cambiato, e aggiorna ad ogni
 * ciclo le grandezze elettriche (V_dc_p, I_discharge_max_p, I_charge_max_p).
 * Valori <= 0 in 'limits' vengono ignorati (si tiene il precedente).
 * Ritorna true se la config attiva e' cambiata. */
bool DriveProfiles_Update(int8_t raw_engine_map, uint8_t parameters_config,
                          const PowertrainLimits_t *limits);

/* Config attiva (sola lettura, mai NULL). */
const ProfileSettings_t *DriveProfiles_GetActive(void);

#endif /* DRIVE_PROFILES_H */
