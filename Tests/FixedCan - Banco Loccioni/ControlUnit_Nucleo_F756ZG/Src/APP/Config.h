/**
 * @file Config.h
 * @brief Configurazione globale del sistema
 * 
 * Contiene tutte le costanti condivise tra più moduli
 */

#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

// CONFIGURAZIONE PEDALE APP

/** Valori min/max attesi dall'ADC per il potenziometro */
#define APP_ADC_MIN             0U
#define APP_ADC_MAX          4000U     /**< Dovrebbe essere 4095, ridotto per rumore */

/** Zona morta pedale (percentuale) */
#define APP_DEADZONE_PERCENT   35U

/** Finestra filtro media mobile */
#define APP_FILTER_WINDOW      32U


// CONFIGURAZIONE COPPIA MOTORE
/** Limiti coppia [0.1% Mn] */
#define TORQUE_SETPOINT_MAX      14000   /**< Setpoint massimo */
#define TORQUE_LIMIT_POS         1000   /**< Limite positivo */
#define TORQUE_LIMIT_NEG        -1000   /**< Limite negativo */

/** Rate limiter coppia - max variazione per ms [0.1% Mn] */
#define TORQUE_RATE_LIMIT_PER_MS   50


/* PARAMETRI MOTORE */
#define MOTOR_NOMINAL_TORQUE_NM   9.8f  /**< Coppia nominale [Nm] */
#define MOTOR_MAX_TORQUE_NM      21.0f  /**< Coppia massima [Nm] */
#define MOTOR_KE_V_PER_KRPM      18.8f  /**< Costante di tensione [V/kRPM] */
#define MOTOR_MAX_CURRENT_A     105.0f  /**< Corrente massima [A] */
#define MOTOR_EFFICIENCY          0.95f /**< Efficienza motore */


//CONVERSIONI CAN

#define CAN_SCALE_FACTOR       1000.0f  /**< 1000 = 100% Mn */


#ifdef __cplusplus
}
#endif

#endif /* CONFIG_H */
