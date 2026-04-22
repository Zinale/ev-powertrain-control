/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: SLC1.h
 *
 * Code generated for Simulink model 'SLC1'.
 *
 * Model version                  : 1.14
 * Simulink Coder version         : 25.2 (R2025b) 28-Jul-2025
 * C/C++ source code generated on : Wed Apr 22 14:46:45 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex-M
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#ifndef SLC1_h_
#define SLC1_h_
#ifndef SLC1_COMMON_INCLUDES_
#define SLC1_COMMON_INCLUDES_
#include <stdbool.h>
#include <stdint.h>
#include "complex_types.h"
#include "rt_nonfinite.h"
#include "math.h"
#endif                                 /* SLC1_COMMON_INCLUDES_ */

#include "SLC1_types.h"
#include <string.h>
#include "rt_defines.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

/* Block states (default storage) for system '<Root>' */
typedef struct {
  double Integrator_DSTATE;            /* '<S41>/Integrator' */
  double UD_DSTATE;                    /* '<S34>/UD' */
} DW_SLC1_T;

/* Real-time Model Data Structure */
struct tag_RTM_SLC1_T {
  const char * volatile errorStatus;
  DW_SLC1_T *dwork;
};

/* Model entry point functions */
extern void SLC1_initialize(RT_MODEL_SLC1_T *const SLC1_M, double
  *SLC1_U_T_driver, double *SLC1_U_T_RL, double *SLC1_U_T_RR, bool
  *SLC1_U_Enable, double *SLC1_Y_T_reqRL, double *SLC1_Y_T_reqRR);
extern void SLC1_step(RT_MODEL_SLC1_T *const SLC1_M, double SLC1_U_T_driver,
                      double SLC1_U_T_RL, double SLC1_U_T_RR, bool SLC1_U_Enable,
                      double *SLC1_Y_T_reqRL, double *SLC1_Y_T_reqRR);
extern void SLC1_terminate(RT_MODEL_SLC1_T *const SLC1_M);

/*-
 * These blocks were eliminated from the model due to optimizations:
 *
 * Block '<S34>/DTDup' : Unused code path elimination
 * Block '<S1>/Scope' : Unused code path elimination
 */

/*-
 * The generated code includes comments that allow you to trace directly
 * back to the appropriate location in the model.  The basic format
 * is <system>/block_name, where system is the system number (uniquely
 * assigned by Simulink) and block_name is the name of the block.
 *
 * Note that this particular code originates from a subsystem build,
 * and has its own system numbers different from the parent model.
 * Refer to the system hierarchy for this subsystem below, and use the
 * MATLAB hilite_system command to trace the generated code back
 * to the parent model.  For example,
 *
 * hilite_system('SLC/SLC1')    - opens subsystem SLC/SLC1
 * hilite_system('SLC/SLC1/Kp') - opens and selects block Kp
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'SLC'
 * '<S1>'   : 'SLC/SLC1'
 * '<S2>'   : 'SLC/SLC1/PID Controller1'
 * '<S3>'   : 'SLC/SLC1/PID Controller1/Anti-windup'
 * '<S4>'   : 'SLC/SLC1/PID Controller1/D Gain'
 * '<S5>'   : 'SLC/SLC1/PID Controller1/External Derivative'
 * '<S6>'   : 'SLC/SLC1/PID Controller1/Filter'
 * '<S7>'   : 'SLC/SLC1/PID Controller1/Filter ICs'
 * '<S8>'   : 'SLC/SLC1/PID Controller1/I Gain'
 * '<S9>'   : 'SLC/SLC1/PID Controller1/Ideal P Gain'
 * '<S10>'  : 'SLC/SLC1/PID Controller1/Ideal P Gain Fdbk'
 * '<S11>'  : 'SLC/SLC1/PID Controller1/Integrator'
 * '<S12>'  : 'SLC/SLC1/PID Controller1/Integrator ICs'
 * '<S13>'  : 'SLC/SLC1/PID Controller1/N Copy'
 * '<S14>'  : 'SLC/SLC1/PID Controller1/N Gain'
 * '<S15>'  : 'SLC/SLC1/PID Controller1/P Copy'
 * '<S16>'  : 'SLC/SLC1/PID Controller1/Parallel P Gain'
 * '<S17>'  : 'SLC/SLC1/PID Controller1/Reset Signal'
 * '<S18>'  : 'SLC/SLC1/PID Controller1/Saturation'
 * '<S19>'  : 'SLC/SLC1/PID Controller1/Saturation Fdbk'
 * '<S20>'  : 'SLC/SLC1/PID Controller1/Sum'
 * '<S21>'  : 'SLC/SLC1/PID Controller1/Sum Fdbk'
 * '<S22>'  : 'SLC/SLC1/PID Controller1/Tracking Mode'
 * '<S23>'  : 'SLC/SLC1/PID Controller1/Tracking Mode Sum'
 * '<S24>'  : 'SLC/SLC1/PID Controller1/Tsamp - Integral'
 * '<S25>'  : 'SLC/SLC1/PID Controller1/Tsamp - Ngain'
 * '<S26>'  : 'SLC/SLC1/PID Controller1/postSat Signal'
 * '<S27>'  : 'SLC/SLC1/PID Controller1/preInt Signal'
 * '<S28>'  : 'SLC/SLC1/PID Controller1/preSat Signal'
 * '<S29>'  : 'SLC/SLC1/PID Controller1/Anti-windup/Disc. Clamping Parallel'
 * '<S30>'  : 'SLC/SLC1/PID Controller1/Anti-windup/Disc. Clamping Parallel/Dead Zone'
 * '<S31>'  : 'SLC/SLC1/PID Controller1/Anti-windup/Disc. Clamping Parallel/Dead Zone/Enabled'
 * '<S32>'  : 'SLC/SLC1/PID Controller1/D Gain/Internal Parameters'
 * '<S33>'  : 'SLC/SLC1/PID Controller1/External Derivative/Error'
 * '<S34>'  : 'SLC/SLC1/PID Controller1/Filter/Differentiator'
 * '<S35>'  : 'SLC/SLC1/PID Controller1/Filter/Differentiator/Tsamp'
 * '<S36>'  : 'SLC/SLC1/PID Controller1/Filter/Differentiator/Tsamp/Internal Ts'
 * '<S37>'  : 'SLC/SLC1/PID Controller1/Filter ICs/Internal IC - Differentiator'
 * '<S38>'  : 'SLC/SLC1/PID Controller1/I Gain/Internal Parameters'
 * '<S39>'  : 'SLC/SLC1/PID Controller1/Ideal P Gain/Passthrough'
 * '<S40>'  : 'SLC/SLC1/PID Controller1/Ideal P Gain Fdbk/Disabled'
 * '<S41>'  : 'SLC/SLC1/PID Controller1/Integrator/Discrete'
 * '<S42>'  : 'SLC/SLC1/PID Controller1/Integrator ICs/Internal IC'
 * '<S43>'  : 'SLC/SLC1/PID Controller1/N Copy/Disabled wSignal Specification'
 * '<S44>'  : 'SLC/SLC1/PID Controller1/N Gain/Passthrough'
 * '<S45>'  : 'SLC/SLC1/PID Controller1/P Copy/Disabled'
 * '<S46>'  : 'SLC/SLC1/PID Controller1/Parallel P Gain/Internal Parameters'
 * '<S47>'  : 'SLC/SLC1/PID Controller1/Reset Signal/Disabled'
 * '<S48>'  : 'SLC/SLC1/PID Controller1/Saturation/Enabled'
 * '<S49>'  : 'SLC/SLC1/PID Controller1/Saturation Fdbk/Disabled'
 * '<S50>'  : 'SLC/SLC1/PID Controller1/Sum/Sum_PID'
 * '<S51>'  : 'SLC/SLC1/PID Controller1/Sum Fdbk/Disabled'
 * '<S52>'  : 'SLC/SLC1/PID Controller1/Tracking Mode/Disabled'
 * '<S53>'  : 'SLC/SLC1/PID Controller1/Tracking Mode Sum/Passthrough'
 * '<S54>'  : 'SLC/SLC1/PID Controller1/Tsamp - Integral/TsSignalSpecification'
 * '<S55>'  : 'SLC/SLC1/PID Controller1/Tsamp - Ngain/Passthrough'
 * '<S56>'  : 'SLC/SLC1/PID Controller1/postSat Signal/Forward_Path'
 * '<S57>'  : 'SLC/SLC1/PID Controller1/preInt Signal/Internal PreInt'
 * '<S58>'  : 'SLC/SLC1/PID Controller1/preSat Signal/Forward_Path'
 */
#endif                                 /* SLC1_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
