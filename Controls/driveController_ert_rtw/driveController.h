/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: driveController.h
 *
 * Code generated for Simulink model 'driveController'.
 *
 * Model version                  : 1.1
 * Simulink Coder version         : 26.1 (R2026a) 20-Nov-2025
 * C/C++ source code generated on : Fri May  1 20:55:48 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex-M
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#ifndef driveController_h_
#define driveController_h_
#ifndef driveController_COMMON_INCLUDES_
#define driveController_COMMON_INCLUDES_
#include <stdbool.h>
#include <stdint.h>
#include "complex_types.h"
#include "rt_nonfinite.h"
#include "math.h"
#endif                                 /* driveController_COMMON_INCLUDES_ */

#include "driveController_types.h"
#include "rtGetNaN.h"
#include "zero_crossing_types.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

/* Block signals (default storage) */
typedef struct {
  double dT;                           /* '<S3>/MATLAB Function' */
} B_driveController_T;

/* Block states (default storage) for system '<Root>' */
typedef struct {
  double Integrator_DSTATE;            /* '<S102>/Integrator' */
  double Filter_DSTATE;                /* '<S97>/Filter' */
  double Integrator_DSTATE_o;          /* '<S45>/Integrator' */
  double UD_DSTATE;                    /* '<S38>/UD' */
  double PrevY;                        /* '<S4>/Rate Lmiter' */
  double PrevY_d;                      /* '<Root>/W_RateLimiter' */
  double Memory_PreviousInput;         /* '<S3>/Memory' */
  int8_t Integrator_PrevResetState;    /* '<S102>/Integrator' */
  int8_t Filter_PrevResetState;        /* '<S97>/Filter' */
  int8_t Integrator_PrevResetState_a;  /* '<S45>/Integrator' */
  uint8_t Integrator_IC_LOADING;       /* '<S102>/Integrator' */
  uint8_t is_active_c5_driveController;/* '<Root>/Mode Manager' */
  uint8_t is_c5_driveController;       /* '<Root>/Mode Manager' */
} DW_driveController_T;

/* Zero-crossing (trigger) state */
typedef struct {
  ZCSigState UD_Reset_ZCE;             /* '<S38>/UD' */
} PrevZCX_driveController_T;

/* Invariant block signals (default storage) */
typedef struct {
  const double Constant;               /* '<S3>/Constant' */
} ConstB_driveController_T;

/* External inputs (root inport signals with default storage) */
typedef struct {
  double V;                            /* '<Root>/V' */
  double Yaw_meas;                     /* '<Root>/Yaw_meas' */
  double Steering;                     /* '<Root>/Steering' */
  double Throttle_;                    /* '<Root>/Throttle_%' */
  double brake;                        /* '<Root>/brake' */
  double T_RL_act;                     /* '<Root>/T_RL_act' */
  double T_RR_act;                     /* '<Root>/T_RR_act' */
  double T_RR_max;                     /* '<Root>/T_RR_max' */
  double T_RL_max;                     /* '<Root>/T_RL_max' */
} ExtU_driveController_T;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  double T_RL;                         /* '<Root>/T_RL' */
  double T_RR;                         /* '<Root>/T_RR' */
  double yaw_th;                       /* '<Root>/yaw_th' */
  double TVC_Target_Weight;            /* '<Root>/TVC_Target_Weight' */
} ExtY_driveController_T;

/* Real-time Model Data Structure */
struct tag_RTM_driveController_T {
  const char * volatile errorStatus;

  /*
   * Timing:
   * The following substructure contains information regarding
   * the timing information for the model.
   */
  struct {
    struct {
      uint8_t TID[2];
    } TaskCounters;
  } Timing;
};

/* Block signals (default storage) */
extern B_driveController_T driveController_B;

/* Block states (default storage) */
extern DW_driveController_T driveController_DW;

/* Zero-crossing (trigger) state */
extern PrevZCX_driveController_T driveController_PrevZCX;

/* External inputs (root inport signals with default storage) */
extern ExtU_driveController_T driveController_U;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY_driveController_T driveController_Y;
extern const ConstB_driveController_T driveController_ConstB;/* constant block i/o */

/* Model entry point functions */
extern void driveController_initialize(void);
extern void driveController_step(void);
extern void driveController_terminate(void);

/* Real-time Model object */
extern RT_MODEL_driveController_T *const driveController_M;

/*-
 * These blocks were eliminated from the model due to optimizations:
 *
 * Block '<S38>/DTDup' : Unused code path elimination
 * Block '<S2>/Scope' : Unused code path elimination
 * Block '<Root>/Scope' : Unused code path elimination
 * Block '<S3>/Scope' : Unused code path elimination
 * Block '<S94>/Kb' : Eliminated nontunable gain of 1
 */

/*-
 * The generated code includes comments that allow you to trace directly
 * back to the appropriate location in the model.  The basic format
 * is <system>/block_name, where system is the system number (uniquely
 * assigned by Simulink) and block_name is the name of the block.
 *
 * Use the MATLAB hilite_system command to trace the generated code back
 * to the model.  For example,
 *
 * hilite_system('<S3>')    - opens system 3
 * hilite_system('<S3>/Kp') - opens and selects block Kp which resides in S3
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'driveController'
 * '<S1>'   : 'driveController/Mode Manager'
 * '<S2>'   : 'driveController/SLC'
 * '<S3>'   : 'driveController/TVC'
 * '<S4>'   : 'driveController/Yaw_th'
 * '<S5>'   : 'driveController/SLC/Compare To Constant'
 * '<S6>'   : 'driveController/SLC/PID Controller1'
 * '<S7>'   : 'driveController/SLC/PID Controller1/Anti-windup'
 * '<S8>'   : 'driveController/SLC/PID Controller1/D Gain'
 * '<S9>'   : 'driveController/SLC/PID Controller1/External Derivative'
 * '<S10>'  : 'driveController/SLC/PID Controller1/Filter'
 * '<S11>'  : 'driveController/SLC/PID Controller1/Filter ICs'
 * '<S12>'  : 'driveController/SLC/PID Controller1/I Gain'
 * '<S13>'  : 'driveController/SLC/PID Controller1/Ideal P Gain'
 * '<S14>'  : 'driveController/SLC/PID Controller1/Ideal P Gain Fdbk'
 * '<S15>'  : 'driveController/SLC/PID Controller1/Integrator'
 * '<S16>'  : 'driveController/SLC/PID Controller1/Integrator ICs'
 * '<S17>'  : 'driveController/SLC/PID Controller1/N Copy'
 * '<S18>'  : 'driveController/SLC/PID Controller1/N Gain'
 * '<S19>'  : 'driveController/SLC/PID Controller1/P Copy'
 * '<S20>'  : 'driveController/SLC/PID Controller1/Parallel P Gain'
 * '<S21>'  : 'driveController/SLC/PID Controller1/Reset Signal'
 * '<S22>'  : 'driveController/SLC/PID Controller1/Saturation'
 * '<S23>'  : 'driveController/SLC/PID Controller1/Saturation Fdbk'
 * '<S24>'  : 'driveController/SLC/PID Controller1/Sum'
 * '<S25>'  : 'driveController/SLC/PID Controller1/Sum Fdbk'
 * '<S26>'  : 'driveController/SLC/PID Controller1/Tracking Mode'
 * '<S27>'  : 'driveController/SLC/PID Controller1/Tracking Mode Sum'
 * '<S28>'  : 'driveController/SLC/PID Controller1/Tsamp - Integral'
 * '<S29>'  : 'driveController/SLC/PID Controller1/Tsamp - Ngain'
 * '<S30>'  : 'driveController/SLC/PID Controller1/postSat Signal'
 * '<S31>'  : 'driveController/SLC/PID Controller1/preInt Signal'
 * '<S32>'  : 'driveController/SLC/PID Controller1/preSat Signal'
 * '<S33>'  : 'driveController/SLC/PID Controller1/Anti-windup/Disc. Clamping Parallel'
 * '<S34>'  : 'driveController/SLC/PID Controller1/Anti-windup/Disc. Clamping Parallel/Dead Zone'
 * '<S35>'  : 'driveController/SLC/PID Controller1/Anti-windup/Disc. Clamping Parallel/Dead Zone/Enabled'
 * '<S36>'  : 'driveController/SLC/PID Controller1/D Gain/Internal Parameters'
 * '<S37>'  : 'driveController/SLC/PID Controller1/External Derivative/Error'
 * '<S38>'  : 'driveController/SLC/PID Controller1/Filter/Differentiator'
 * '<S39>'  : 'driveController/SLC/PID Controller1/Filter/Differentiator/Tsamp'
 * '<S40>'  : 'driveController/SLC/PID Controller1/Filter/Differentiator/Tsamp/Internal Ts'
 * '<S41>'  : 'driveController/SLC/PID Controller1/Filter ICs/Internal IC - Differentiator'
 * '<S42>'  : 'driveController/SLC/PID Controller1/I Gain/Internal Parameters'
 * '<S43>'  : 'driveController/SLC/PID Controller1/Ideal P Gain/Passthrough'
 * '<S44>'  : 'driveController/SLC/PID Controller1/Ideal P Gain Fdbk/Disabled'
 * '<S45>'  : 'driveController/SLC/PID Controller1/Integrator/Discrete'
 * '<S46>'  : 'driveController/SLC/PID Controller1/Integrator ICs/Internal IC'
 * '<S47>'  : 'driveController/SLC/PID Controller1/N Copy/Disabled wSignal Specification'
 * '<S48>'  : 'driveController/SLC/PID Controller1/N Gain/Passthrough'
 * '<S49>'  : 'driveController/SLC/PID Controller1/P Copy/Disabled'
 * '<S50>'  : 'driveController/SLC/PID Controller1/Parallel P Gain/Internal Parameters'
 * '<S51>'  : 'driveController/SLC/PID Controller1/Reset Signal/External Reset'
 * '<S52>'  : 'driveController/SLC/PID Controller1/Saturation/Enabled'
 * '<S53>'  : 'driveController/SLC/PID Controller1/Saturation Fdbk/Disabled'
 * '<S54>'  : 'driveController/SLC/PID Controller1/Sum/Sum_PID'
 * '<S55>'  : 'driveController/SLC/PID Controller1/Sum Fdbk/Disabled'
 * '<S56>'  : 'driveController/SLC/PID Controller1/Tracking Mode/Disabled'
 * '<S57>'  : 'driveController/SLC/PID Controller1/Tracking Mode Sum/Passthrough'
 * '<S58>'  : 'driveController/SLC/PID Controller1/Tsamp - Integral/TsSignalSpecification'
 * '<S59>'  : 'driveController/SLC/PID Controller1/Tsamp - Ngain/Passthrough'
 * '<S60>'  : 'driveController/SLC/PID Controller1/postSat Signal/Forward_Path'
 * '<S61>'  : 'driveController/SLC/PID Controller1/preInt Signal/Internal PreInt'
 * '<S62>'  : 'driveController/SLC/PID Controller1/preSat Signal/Forward_Path'
 * '<S63>'  : 'driveController/TVC/Compare To Constant'
 * '<S64>'  : 'driveController/TVC/MATLAB Function'
 * '<S65>'  : 'driveController/TVC/MATLAB Function1'
 * '<S66>'  : 'driveController/TVC/PID Controller'
 * '<S67>'  : 'driveController/TVC/TorqueAllocator'
 * '<S68>'  : 'driveController/TVC/PID Controller/Anti-windup'
 * '<S69>'  : 'driveController/TVC/PID Controller/D Gain'
 * '<S70>'  : 'driveController/TVC/PID Controller/External Derivative'
 * '<S71>'  : 'driveController/TVC/PID Controller/Filter'
 * '<S72>'  : 'driveController/TVC/PID Controller/Filter ICs'
 * '<S73>'  : 'driveController/TVC/PID Controller/I Gain'
 * '<S74>'  : 'driveController/TVC/PID Controller/Ideal P Gain'
 * '<S75>'  : 'driveController/TVC/PID Controller/Ideal P Gain Fdbk'
 * '<S76>'  : 'driveController/TVC/PID Controller/Integrator'
 * '<S77>'  : 'driveController/TVC/PID Controller/Integrator ICs'
 * '<S78>'  : 'driveController/TVC/PID Controller/N Copy'
 * '<S79>'  : 'driveController/TVC/PID Controller/N Gain'
 * '<S80>'  : 'driveController/TVC/PID Controller/P Copy'
 * '<S81>'  : 'driveController/TVC/PID Controller/Parallel P Gain'
 * '<S82>'  : 'driveController/TVC/PID Controller/Reset Signal'
 * '<S83>'  : 'driveController/TVC/PID Controller/Saturation'
 * '<S84>'  : 'driveController/TVC/PID Controller/Saturation Fdbk'
 * '<S85>'  : 'driveController/TVC/PID Controller/Sum'
 * '<S86>'  : 'driveController/TVC/PID Controller/Sum Fdbk'
 * '<S87>'  : 'driveController/TVC/PID Controller/Tracking Mode'
 * '<S88>'  : 'driveController/TVC/PID Controller/Tracking Mode Sum'
 * '<S89>'  : 'driveController/TVC/PID Controller/Tsamp - Integral'
 * '<S90>'  : 'driveController/TVC/PID Controller/Tsamp - Ngain'
 * '<S91>'  : 'driveController/TVC/PID Controller/postSat Signal'
 * '<S92>'  : 'driveController/TVC/PID Controller/preInt Signal'
 * '<S93>'  : 'driveController/TVC/PID Controller/preSat Signal'
 * '<S94>'  : 'driveController/TVC/PID Controller/Anti-windup/Back Calculation'
 * '<S95>'  : 'driveController/TVC/PID Controller/D Gain/Internal Parameters'
 * '<S96>'  : 'driveController/TVC/PID Controller/External Derivative/Error'
 * '<S97>'  : 'driveController/TVC/PID Controller/Filter/Disc. Forward Euler Filter'
 * '<S98>'  : 'driveController/TVC/PID Controller/Filter ICs/External IC'
 * '<S99>'  : 'driveController/TVC/PID Controller/I Gain/Internal Parameters'
 * '<S100>' : 'driveController/TVC/PID Controller/Ideal P Gain/Passthrough'
 * '<S101>' : 'driveController/TVC/PID Controller/Ideal P Gain Fdbk/Disabled'
 * '<S102>' : 'driveController/TVC/PID Controller/Integrator/Discrete'
 * '<S103>' : 'driveController/TVC/PID Controller/Integrator ICs/External IC'
 * '<S104>' : 'driveController/TVC/PID Controller/N Copy/Disabled'
 * '<S105>' : 'driveController/TVC/PID Controller/N Gain/Internal Parameters'
 * '<S106>' : 'driveController/TVC/PID Controller/P Copy/Disabled'
 * '<S107>' : 'driveController/TVC/PID Controller/Parallel P Gain/Internal Parameters'
 * '<S108>' : 'driveController/TVC/PID Controller/Reset Signal/External Reset'
 * '<S109>' : 'driveController/TVC/PID Controller/Saturation/Enabled'
 * '<S110>' : 'driveController/TVC/PID Controller/Saturation Fdbk/Disabled'
 * '<S111>' : 'driveController/TVC/PID Controller/Sum/Sum_PID'
 * '<S112>' : 'driveController/TVC/PID Controller/Sum Fdbk/Disabled'
 * '<S113>' : 'driveController/TVC/PID Controller/Tracking Mode/Enabled'
 * '<S114>' : 'driveController/TVC/PID Controller/Tracking Mode Sum/Tracking Mode'
 * '<S115>' : 'driveController/TVC/PID Controller/Tsamp - Integral/TsSignalSpecification'
 * '<S116>' : 'driveController/TVC/PID Controller/Tsamp - Ngain/Passthrough'
 * '<S117>' : 'driveController/TVC/PID Controller/postSat Signal/Forward_Path'
 * '<S118>' : 'driveController/TVC/PID Controller/preInt Signal/Internal PreInt'
 * '<S119>' : 'driveController/TVC/PID Controller/preSat Signal/Forward_Path'
 * '<S120>' : 'driveController/Yaw_th/Bicycle Model Dynamic'
 * '<S121>' : 'driveController/Yaw_th/tand safe'
 * '<S122>' : 'driveController/Yaw_th/yaw_saturation'
 */
#endif                                 /* driveController_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
