#ifndef CONTROL_CONFIG_H
#define CONTROL_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * Modalità di Controllo Coppia (Selezione Precompilatore)
 * ============================================================================ */
#define CONTROL_MODE_PASS_THROUGH   0U  /* Coppia identica SX/DX senza Torque Vectoring */
#define CONTROL_MODE_SIMPLE_TVC     1U  /* TVC semplificato proporzionale all'angolo sterzo */
#define CONTROL_MODE_SIMULINK       2U  /* Modello completo Simulink (Yaw controller + Slip Control) */

/* Seleziona la modalità desiderata per la compilazione */
#define ACTIVE_CONTROL_MODE         CONTROL_MODE_SIMULINK

/* ============================================================================
 * Parametri Simple TVC (per prove al banco / prima accensione)
 * ============================================================================ */
#define TVC_SIMPLE_STEER_GAIN       0.30f   /* Percentuale massima di bias (es. ±30%) */
#define TVC_SIMPLE_MAX_DIFF_NM      6.0f    /* Differenza massima di coppia [Nm] */

/* ============================================================================
 * Inverter montati (per banco prova con una sola unità collegata)
 * ============================================================================ */
#define INVERTER_LEFT_PRESENT       1
#define INVERTER_RIGHT_PRESENT      1

/* ============================================================================
 * Configurazione BANCO PROVA  -  METTERE BENCH_TEST_MODE A 0 IN VETTURA!
 * ----------------------------------------------------------------------------
 * Con BENCH_TEST_MODE = 1:
 *  - R2D si arma con la SOLA pressione del pulsante volante (nessun controllo
 *    di freno / SDC / BMS / timeout CAN)
 *  - la FSM NON esce da RUNNING se vede SDC basso o timeout CAN
 * Il monitor di plausibilità (overcurrent, FUSE_I2T, overspeed, ...) resta
 * ATTIVO e taglia comunque la coppia -> rete di sicurezza al banco.
 * ============================================================================ */
#define BENCH_TEST_MODE   1U

#if (BENCH_TEST_MODE == 1U)
  #define R2D_BYPASS_ARM_CONDITIONS  1U
  #define R2D_IGNORE_TS_SHUTDOWN     1U
#else
  #define R2D_BYPASS_ARM_CONDITIONS  0U
  #define R2D_IGNORE_TS_SHUTDOWN     0U
#endif

/* ============================================================================
 * Regole di Sicurezza & Plausibilità FSAE (EV4.7 / T11.8)
 * ============================================================================ */
#define SAFETY_APPS_BRAKE_CUT_APPS_PCT   30.0f   /* Taglio coppia se APPS > 25%... */
#define SAFETY_APPS_BRAKE_CUT_BRAKE_BAR  20.0f   /* ...e Freno > 20 bar contemporaneamente */
#define SAFETY_APPS_BRAKE_RESET_APPS_PCT 5.0f    /* Ripristino solo se pedale scende sotto il 5% */

/* ============================================================================
 * Monitor di Plausibilità (plausibility.c) - soglie
 * Ogni check e' debounced: scatta dopo N cicli @100 Hz consecutivi in violazione.
 * ============================================================================ */
#define PLAUS_APPS_DISAGREE_PCT       10.0f  /* |APPS1-APPS2| oltre = implausibile (cross-check grossolano) */

/* Range fisici sensori (fuori = fault decodifica / hardware) */
#define PLAUS_APPS_MAX_PCT           102.0f
#define PLAUS_APPS_MIN_PCT            -1.0f
#define PLAUS_BRAKE_MAX_BAR          100.0f
#define PLAUS_BRAKE_MIN_BAR           -1.0f
#define PLAUS_STEER_MAX_DEG          125.0f

/* Corrente di pacco: misura reale dal sensore LEM CAB 500 (0x3C2), con fallback
 * a stima (coppia*giri inverter / eta / V_dc) se il CAB va in timeout.
 * Segno: normalizziamo a "+ = scarica"; metti -1 se il montaggio e' invertito. */
#define CAB500_DISCHARGE_SIGN       (+1.0f)

/* Il vero limite di scarica e' il FUSIBILE di pacco: PACK_FUSE_* ne emula la
 * curva tempo-corrente (I^2t). OVERPOWER/OVERCURRENT restano come trip veloci. */
#define PLAUS_DT_S                    0.01f    /* periodo DriveLogic - per gli integratori */
#define PACK_FUSE_RATED_A          100.0f     /* corrente nominale fusibile di pacco */
#define PACK_FUSE_I2T_TRIP_A2S   30000.0f     /* budget I^2t: ~1s@2xIn, ~0.2s@4xIn, ~3.6s@1.35xIn */

#define PLAUS_MAX_POWER_W          60000.0f  /* ~P al fusibile (100A * V_pack) - trip veloce */
#define PLAUS_POWER_MARGIN            1.10f
#define PLAUS_OVERCURRENT_MARGIN      1.0f  /* I_pack > I_max_discharge_BMS * questo */

/* Coppia richiesta vs pedale */
#define PLAUS_TORQUE_PEDAL_MARGIN     1.20f
#define PLAUS_TORQUE_PEDAL_OFFSET_NM   3.0f  /* tolleranza fissa: launch, ritardo rate-limit */

/* Coerenza coppia / pedale / freno */
#define PLAUS_REGEN_ACCEL_APPS_PCT   20.0f   /* coppia < 0 mentre APPS oltre = fault */
#define PLAUS_TORQUE_EPS_NM           1.0f
#define PLAUS_DRIVE_WHILE_BRAKE_NM    2.0f   /* coppia > 0 con freno duro e pedale rilasciato */

/* Tensione bus HV plausibile (a SDC chiuso) */
#define PLAUS_VDC_MIN_V             400.0f
#define PLAUS_VDC_MAX_V             600.0f

#endif /* CONTROL_CONFIG_H */