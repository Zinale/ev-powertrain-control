/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: driveController.h
 *
 * Code generated for Simulink model 'driveController'.
 *
 * Model version                  : 1.13
 * Simulink Coder version         : 26.1 (R2026a) 20-Nov-2025
 * C/C++ source code generated on : Sat May 23 16:55:46 2026
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

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

/* Block signals (default storage) */
typedef struct {
  float T_RL_cmd;                      /* '<S9>/TorqueAllocator' */
  float T_RR_cmd;                      /* '<S9>/TorqueAllocator' */
} B_driveController_T;

/* Block states (default storage) for system '<Root>' */
typedef struct {
  double Integrator_DSTATE;            /* '<S101>/Integrator' */
  double Filter_DSTATE;                /* '<S96>/Filter' */
  double Integrator_DSTATE_m;          /* '<S49>/Integrator' */
  double Filter_DSTATE_b;              /* '<S44>/Filter' */
  double t_elapsed_s;                  /* '<Root>/MATLAB Function' */
  float Integrator_DSTATE_n;           /* '<S158>/Integrator' */
  float Filter_DSTATE_n;               /* '<S153>/Filter' */
  float DiscreteTransferFcn1_states;   /* '<Root>/Discrete Transfer Fcn1' */
  float DiscreteTransferFcn_states;    /* '<Root>/Discrete Transfer Fcn' */
  float PrevY;                         /* '<S1>/Rate Limiter1' */
  float PrevY_j;                       /* '<S1>/Rate Limiter2' */
  float PrevY_g;                       /* '<S4>/Rate Limiter' */
  float PrevY_a;                       /* '<S4>/Rate Limiter1' */
  float PrevY_o;                       /* '<S10>/Rate Lmiter' */
  float PrevY_i;                       /* '<Root>/W_RateLimiter' */
  float Memory_PreviousInput;          /* '<S9>/Memory' */
  float PrevY_f;                       /* '<Root>/Rate Limiter1' */
  float PrevY_e;                       /* '<Root>/Rate Limiter3' */
  float PrevY_oa;                      /* '<Root>/Rate Limiter4' */
  float PrevY_ag;                      /* '<Root>/Rate Limiter2' */
  int8_t Integrator_PrevResetState;    /* '<S101>/Integrator' */
  int8_t Filter_PrevResetState;        /* '<S96>/Filter' */
  int8_t Integrator_PrevResetState_e;  /* '<S49>/Integrator' */
  int8_t Filter_PrevResetState_f;      /* '<S44>/Filter' */
  int8_t Integrator_PrevResetState_f;  /* '<S158>/Integrator' */
  int8_t Filter_PrevResetState_a;      /* '<S153>/Filter' */
  uint8_t Integrator_IC_LOADING;       /* '<S158>/Integrator' */
  uint8_t is_active_c5_driveController;/* '<Root>/Mode Manager' */
  uint8_t is_c5_driveController;       /* '<Root>/Mode Manager' */
} DW_driveController_T;

/* Invariant block signals (default storage) */
typedef struct {
  const float Constant;                /* '<S9>/Constant' */
} ConstB_driveController_T;

/* Constant parameters (default storage) */
typedef struct {
  /* Computed Parameter: uDLookupTable_tableData
   * Referenced by: '<S1>/1-D Lookup Table'
   */
  float uDLookupTable_tableData[11];

  /* Computed Parameter: uDLookupTable_bp01Data
   * Referenced by: '<S1>/1-D Lookup Table'
   */
  float uDLookupTable_bp01Data[11];
} ConstP_driveController_T;

/* External inputs (root inport signals with default storage) */
typedef struct {
  float Vx;                            /* '<Root>/Vx' */
  float ax;                            /* '<Root>/ax' */
  float ay;                            /* '<Root>/ay' */
  float Yaw_meas;                      /* '<Root>/Yaw_meas' */
  float Drad;                          /* '<Root>/D' */
  float Throttle_;                     /* '<Root>/Throttle_%' */
  float brake;                         /* '<Root>/brake' */
  float T_RL_act;                      /* '<Root>/T_RL_act' */
  float T_RR_act;                      /* '<Root>/T_RR_act' */
  float Derating_RL;                   /* '<Root>/Derating_RL' */
  float Derating_RR;                   /* '<Root>/Derating_RR' */
  float n_motorRL;                     /* '<Root>/n_motorRL' */
  float n_motorRR;                     /* '<Root>/n_motorRR' */
  float lambda_refRL;                  /* '<Root>/lambda_refRL' */
  float lambda_refRR;                  /* '<Root>/lambda_refRR' */
} ExtU_driveController_T;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  float T_RL;                          /* '<Root>/T_RL' */
  float T_RR;                          /* '<Root>/T_RR' */
  float yaw_th;                        /* '<Root>/yaw_th' */
  float TVC_Target_Weight;             /* '<Root>/TVC_Target_Weight' */
  float K_slips[2];                    /* '<Root>/K_slips' */
  float regen;                         /* '<Root>/regen' */
  float lambda[4];                     /* '<Root>/lambda' */
  double Throttle_real;                /* '<Root>/Throttle_real' */
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

/* External inputs (root inport signals with default storage) */
extern ExtU_driveController_T driveController_U;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY_driveController_T driveController_Y;
extern const ConstB_driveController_T driveController_ConstB;/* constant block i/o */

/* Constant parameters (default storage) */
extern const ConstP_driveController_T driveController_ConstP;

/*
 * Exported Global Parameters
 *
 * Note: Exported global parameters are tunable parameters with an exported
 * global storage class designation.  Code generation will declare the memory for
 * these parameters and exports their symbols.
 *
 */
extern float T_headroom_k;             /* Variable: T_headroom_k
                                        * Referenced by: '<S9>/TorqueAllocator'
                                        */
extern float T_headroom_max;           /* Variable: T_headroom_max
                                        * Referenced by: '<S9>/TorqueAllocator'
                                        */
extern float T_rated;                  /* Variable: T_rated
                                        * Referenced by:
                                        *   '<S4>/Constant1'
                                        *   '<S9>/TorqueAllocator'
                                        */
extern float regen_T_max;              /* Variable: regen_T_max
                                        * Referenced by: '<Root>/MATLAB Function'
                                        */
extern float slip_Kd;                  /* Variable: slip_Kd
                                        * Referenced by:
                                        *   '<S42>/Derivative Gain'
                                        *   '<S94>/Derivative Gain'
                                        */
extern float slip_Ki;                  /* Variable: slip_Ki
                                        * Referenced by:
                                        *   '<S46>/Integral Gain'
                                        *   '<S98>/Integral Gain'
                                        */
extern float slip_Kp;                  /* Variable: slip_Kp
                                        * Referenced by:
                                        *   '<S54>/Proportional Gain'
                                        *   '<S106>/Proportional Gain'
                                        */
extern float slip_bc_coeff;            /* Variable: slip_bc_coeff
                                        * Referenced by:
                                        *   '<S41>/Kb'
                                        *   '<S93>/Kb'
                                        */
extern float tvc_Kd;                   /* Variable: tvc_Kd
                                        * Referenced by: '<S151>/Derivative Gain'
                                        */
extern float tvc_Ki;                   /* Variable: tvc_Ki
                                        * Referenced by: '<S155>/Integral Gain'
                                        */
extern float tvc_Kp;                   /* Variable: tvc_Kp
                                        * Referenced by: '<S163>/Proportional Gain'
                                        */
extern float tvc_N_filter;             /* Variable: tvc_N_filter
                                        * Referenced by: '<S161>/Filter Coefficient'
                                        */
extern float tvc_V_off;                /* Variable: tvc_V_off
                                        * Referenced by: '<Root>/Mode Manager'
                                        */
extern float tvc_V_on;                 /* Variable: tvc_V_on
                                        * Referenced by: '<Root>/Mode Manager'
                                        */
extern float tvc_low_sat;              /* Variable: tvc_low_sat
                                        * Referenced by: '<S165>/Saturation'
                                        */
extern float tvc_throttle_on;          /* Variable: tvc_throttle_on
                                        * Referenced by: '<Root>/Mode Manager'
                                        */
extern float tvc_tr;                   /* Variable: tvc_tr
                                        * Referenced by: '<S169>/Kt'
                                        */
extern float tvc_up_sat;               /* Variable: tvc_up_sat
                                        * Referenced by: '<S165>/Saturation'
                                        */

/* Model entry point functions */
extern void driveController_initialize(void);
extern void driveController_step(void);
extern void driveController_terminate(void);

/* Real-time Model object */
extern RT_MODEL_driveController_T *const driveController_M;

/*-
 * These blocks were eliminated from the model due to optimizations:
 *
 * Block '<S6>/Add4' : Unused code path elimination
 * Block '<S6>/Gain6' : Unused code path elimination
 * Block '<S7>/Data Type Duplicate' : Unused code path elimination
 * Block '<S7>/Data Type Propagation' : Unused code path elimination
 * Block '<S8>/Data Type Duplicate' : Unused code path elimination
 * Block '<S8>/Data Type Propagation' : Unused code path elimination
 * Block '<Root>/Scope1' : Unused code path elimination
 * Block '<S150>/Kb' : Eliminated nontunable gain of 1
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
 * '<S1>'   : 'driveController/LaunchThrottleController'
 * '<S2>'   : 'driveController/MATLAB Function'
 * '<S3>'   : 'driveController/MATLAB Function1'
 * '<S4>'   : 'driveController/MTFC'
 * '<S5>'   : 'driveController/Mode Manager'
 * '<S6>'   : 'driveController/SRC'
 * '<S7>'   : 'driveController/Saturation Dynamic'
 * '<S8>'   : 'driveController/Saturation Dynamic1'
 * '<S9>'   : 'driveController/TVC'
 * '<S10>'  : 'driveController/Yaw_th'
 * '<S11>'  : 'driveController/SRC/MATLAB Function'
 * '<S12>'  : 'driveController/SRC/MATLAB Function1'
 * '<S13>'  : 'driveController/SRC/PID Controller'
 * '<S14>'  : 'driveController/SRC/PID Controller1'
 * '<S15>'  : 'driveController/SRC/PID Controller/Anti-windup'
 * '<S16>'  : 'driveController/SRC/PID Controller/D Gain'
 * '<S17>'  : 'driveController/SRC/PID Controller/External Derivative'
 * '<S18>'  : 'driveController/SRC/PID Controller/Filter'
 * '<S19>'  : 'driveController/SRC/PID Controller/Filter ICs'
 * '<S20>'  : 'driveController/SRC/PID Controller/I Gain'
 * '<S21>'  : 'driveController/SRC/PID Controller/Ideal P Gain'
 * '<S22>'  : 'driveController/SRC/PID Controller/Ideal P Gain Fdbk'
 * '<S23>'  : 'driveController/SRC/PID Controller/Integrator'
 * '<S24>'  : 'driveController/SRC/PID Controller/Integrator ICs'
 * '<S25>'  : 'driveController/SRC/PID Controller/N Copy'
 * '<S26>'  : 'driveController/SRC/PID Controller/N Gain'
 * '<S27>'  : 'driveController/SRC/PID Controller/P Copy'
 * '<S28>'  : 'driveController/SRC/PID Controller/Parallel P Gain'
 * '<S29>'  : 'driveController/SRC/PID Controller/Reset Signal'
 * '<S30>'  : 'driveController/SRC/PID Controller/Saturation'
 * '<S31>'  : 'driveController/SRC/PID Controller/Saturation Fdbk'
 * '<S32>'  : 'driveController/SRC/PID Controller/Sum'
 * '<S33>'  : 'driveController/SRC/PID Controller/Sum Fdbk'
 * '<S34>'  : 'driveController/SRC/PID Controller/Tracking Mode'
 * '<S35>'  : 'driveController/SRC/PID Controller/Tracking Mode Sum'
 * '<S36>'  : 'driveController/SRC/PID Controller/Tsamp - Integral'
 * '<S37>'  : 'driveController/SRC/PID Controller/Tsamp - Ngain'
 * '<S38>'  : 'driveController/SRC/PID Controller/postSat Signal'
 * '<S39>'  : 'driveController/SRC/PID Controller/preInt Signal'
 * '<S40>'  : 'driveController/SRC/PID Controller/preSat Signal'
 * '<S41>'  : 'driveController/SRC/PID Controller/Anti-windup/Back Calculation'
 * '<S42>'  : 'driveController/SRC/PID Controller/D Gain/Internal Parameters'
 * '<S43>'  : 'driveController/SRC/PID Controller/External Derivative/Error'
 * '<S44>'  : 'driveController/SRC/PID Controller/Filter/Disc. Forward Euler Filter'
 * '<S45>'  : 'driveController/SRC/PID Controller/Filter ICs/Internal IC - Filter'
 * '<S46>'  : 'driveController/SRC/PID Controller/I Gain/Internal Parameters'
 * '<S47>'  : 'driveController/SRC/PID Controller/Ideal P Gain/Passthrough'
 * '<S48>'  : 'driveController/SRC/PID Controller/Ideal P Gain Fdbk/Disabled'
 * '<S49>'  : 'driveController/SRC/PID Controller/Integrator/Discrete'
 * '<S50>'  : 'driveController/SRC/PID Controller/Integrator ICs/Internal IC'
 * '<S51>'  : 'driveController/SRC/PID Controller/N Copy/Disabled'
 * '<S52>'  : 'driveController/SRC/PID Controller/N Gain/Internal Parameters'
 * '<S53>'  : 'driveController/SRC/PID Controller/P Copy/Disabled'
 * '<S54>'  : 'driveController/SRC/PID Controller/Parallel P Gain/Internal Parameters'
 * '<S55>'  : 'driveController/SRC/PID Controller/Reset Signal/External Reset'
 * '<S56>'  : 'driveController/SRC/PID Controller/Saturation/Enabled'
 * '<S57>'  : 'driveController/SRC/PID Controller/Saturation Fdbk/Disabled'
 * '<S58>'  : 'driveController/SRC/PID Controller/Sum/Sum_PID'
 * '<S59>'  : 'driveController/SRC/PID Controller/Sum Fdbk/Disabled'
 * '<S60>'  : 'driveController/SRC/PID Controller/Tracking Mode/Disabled'
 * '<S61>'  : 'driveController/SRC/PID Controller/Tracking Mode Sum/Passthrough'
 * '<S62>'  : 'driveController/SRC/PID Controller/Tsamp - Integral/TsSignalSpecification'
 * '<S63>'  : 'driveController/SRC/PID Controller/Tsamp - Ngain/Passthrough'
 * '<S64>'  : 'driveController/SRC/PID Controller/postSat Signal/Forward_Path'
 * '<S65>'  : 'driveController/SRC/PID Controller/preInt Signal/Internal PreInt'
 * '<S66>'  : 'driveController/SRC/PID Controller/preSat Signal/Forward_Path'
 * '<S67>'  : 'driveController/SRC/PID Controller1/Anti-windup'
 * '<S68>'  : 'driveController/SRC/PID Controller1/D Gain'
 * '<S69>'  : 'driveController/SRC/PID Controller1/External Derivative'
 * '<S70>'  : 'driveController/SRC/PID Controller1/Filter'
 * '<S71>'  : 'driveController/SRC/PID Controller1/Filter ICs'
 * '<S72>'  : 'driveController/SRC/PID Controller1/I Gain'
 * '<S73>'  : 'driveController/SRC/PID Controller1/Ideal P Gain'
 * '<S74>'  : 'driveController/SRC/PID Controller1/Ideal P Gain Fdbk'
 * '<S75>'  : 'driveController/SRC/PID Controller1/Integrator'
 * '<S76>'  : 'driveController/SRC/PID Controller1/Integrator ICs'
 * '<S77>'  : 'driveController/SRC/PID Controller1/N Copy'
 * '<S78>'  : 'driveController/SRC/PID Controller1/N Gain'
 * '<S79>'  : 'driveController/SRC/PID Controller1/P Copy'
 * '<S80>'  : 'driveController/SRC/PID Controller1/Parallel P Gain'
 * '<S81>'  : 'driveController/SRC/PID Controller1/Reset Signal'
 * '<S82>'  : 'driveController/SRC/PID Controller1/Saturation'
 * '<S83>'  : 'driveController/SRC/PID Controller1/Saturation Fdbk'
 * '<S84>'  : 'driveController/SRC/PID Controller1/Sum'
 * '<S85>'  : 'driveController/SRC/PID Controller1/Sum Fdbk'
 * '<S86>'  : 'driveController/SRC/PID Controller1/Tracking Mode'
 * '<S87>'  : 'driveController/SRC/PID Controller1/Tracking Mode Sum'
 * '<S88>'  : 'driveController/SRC/PID Controller1/Tsamp - Integral'
 * '<S89>'  : 'driveController/SRC/PID Controller1/Tsamp - Ngain'
 * '<S90>'  : 'driveController/SRC/PID Controller1/postSat Signal'
 * '<S91>'  : 'driveController/SRC/PID Controller1/preInt Signal'
 * '<S92>'  : 'driveController/SRC/PID Controller1/preSat Signal'
 * '<S93>'  : 'driveController/SRC/PID Controller1/Anti-windup/Back Calculation'
 * '<S94>'  : 'driveController/SRC/PID Controller1/D Gain/Internal Parameters'
 * '<S95>'  : 'driveController/SRC/PID Controller1/External Derivative/Error'
 * '<S96>'  : 'driveController/SRC/PID Controller1/Filter/Disc. Forward Euler Filter'
 * '<S97>'  : 'driveController/SRC/PID Controller1/Filter ICs/Internal IC - Filter'
 * '<S98>'  : 'driveController/SRC/PID Controller1/I Gain/Internal Parameters'
 * '<S99>'  : 'driveController/SRC/PID Controller1/Ideal P Gain/Passthrough'
 * '<S100>' : 'driveController/SRC/PID Controller1/Ideal P Gain Fdbk/Disabled'
 * '<S101>' : 'driveController/SRC/PID Controller1/Integrator/Discrete'
 * '<S102>' : 'driveController/SRC/PID Controller1/Integrator ICs/Internal IC'
 * '<S103>' : 'driveController/SRC/PID Controller1/N Copy/Disabled'
 * '<S104>' : 'driveController/SRC/PID Controller1/N Gain/Internal Parameters'
 * '<S105>' : 'driveController/SRC/PID Controller1/P Copy/Disabled'
 * '<S106>' : 'driveController/SRC/PID Controller1/Parallel P Gain/Internal Parameters'
 * '<S107>' : 'driveController/SRC/PID Controller1/Reset Signal/External Reset'
 * '<S108>' : 'driveController/SRC/PID Controller1/Saturation/Enabled'
 * '<S109>' : 'driveController/SRC/PID Controller1/Saturation Fdbk/Disabled'
 * '<S110>' : 'driveController/SRC/PID Controller1/Sum/Sum_PID'
 * '<S111>' : 'driveController/SRC/PID Controller1/Sum Fdbk/Disabled'
 * '<S112>' : 'driveController/SRC/PID Controller1/Tracking Mode/Disabled'
 * '<S113>' : 'driveController/SRC/PID Controller1/Tracking Mode Sum/Passthrough'
 * '<S114>' : 'driveController/SRC/PID Controller1/Tsamp - Integral/TsSignalSpecification'
 * '<S115>' : 'driveController/SRC/PID Controller1/Tsamp - Ngain/Passthrough'
 * '<S116>' : 'driveController/SRC/PID Controller1/postSat Signal/Forward_Path'
 * '<S117>' : 'driveController/SRC/PID Controller1/preInt Signal/Internal PreInt'
 * '<S118>' : 'driveController/SRC/PID Controller1/preSat Signal/Forward_Path'
 * '<S119>' : 'driveController/TVC/Compare To Constant'
 * '<S120>' : 'driveController/TVC/MATLAB Function'
 * '<S121>' : 'driveController/TVC/MATLAB Function1'
 * '<S122>' : 'driveController/TVC/PID Controller'
 * '<S123>' : 'driveController/TVC/TorqueAllocator'
 * '<S124>' : 'driveController/TVC/PID Controller/Anti-windup'
 * '<S125>' : 'driveController/TVC/PID Controller/D Gain'
 * '<S126>' : 'driveController/TVC/PID Controller/External Derivative'
 * '<S127>' : 'driveController/TVC/PID Controller/Filter'
 * '<S128>' : 'driveController/TVC/PID Controller/Filter ICs'
 * '<S129>' : 'driveController/TVC/PID Controller/I Gain'
 * '<S130>' : 'driveController/TVC/PID Controller/Ideal P Gain'
 * '<S131>' : 'driveController/TVC/PID Controller/Ideal P Gain Fdbk'
 * '<S132>' : 'driveController/TVC/PID Controller/Integrator'
 * '<S133>' : 'driveController/TVC/PID Controller/Integrator ICs'
 * '<S134>' : 'driveController/TVC/PID Controller/N Copy'
 * '<S135>' : 'driveController/TVC/PID Controller/N Gain'
 * '<S136>' : 'driveController/TVC/PID Controller/P Copy'
 * '<S137>' : 'driveController/TVC/PID Controller/Parallel P Gain'
 * '<S138>' : 'driveController/TVC/PID Controller/Reset Signal'
 * '<S139>' : 'driveController/TVC/PID Controller/Saturation'
 * '<S140>' : 'driveController/TVC/PID Controller/Saturation Fdbk'
 * '<S141>' : 'driveController/TVC/PID Controller/Sum'
 * '<S142>' : 'driveController/TVC/PID Controller/Sum Fdbk'
 * '<S143>' : 'driveController/TVC/PID Controller/Tracking Mode'
 * '<S144>' : 'driveController/TVC/PID Controller/Tracking Mode Sum'
 * '<S145>' : 'driveController/TVC/PID Controller/Tsamp - Integral'
 * '<S146>' : 'driveController/TVC/PID Controller/Tsamp - Ngain'
 * '<S147>' : 'driveController/TVC/PID Controller/postSat Signal'
 * '<S148>' : 'driveController/TVC/PID Controller/preInt Signal'
 * '<S149>' : 'driveController/TVC/PID Controller/preSat Signal'
 * '<S150>' : 'driveController/TVC/PID Controller/Anti-windup/Back Calculation'
 * '<S151>' : 'driveController/TVC/PID Controller/D Gain/Internal Parameters'
 * '<S152>' : 'driveController/TVC/PID Controller/External Derivative/Error'
 * '<S153>' : 'driveController/TVC/PID Controller/Filter/Disc. Forward Euler Filter'
 * '<S154>' : 'driveController/TVC/PID Controller/Filter ICs/External IC'
 * '<S155>' : 'driveController/TVC/PID Controller/I Gain/Internal Parameters'
 * '<S156>' : 'driveController/TVC/PID Controller/Ideal P Gain/Passthrough'
 * '<S157>' : 'driveController/TVC/PID Controller/Ideal P Gain Fdbk/Disabled'
 * '<S158>' : 'driveController/TVC/PID Controller/Integrator/Discrete'
 * '<S159>' : 'driveController/TVC/PID Controller/Integrator ICs/External IC'
 * '<S160>' : 'driveController/TVC/PID Controller/N Copy/Disabled'
 * '<S161>' : 'driveController/TVC/PID Controller/N Gain/Internal Parameters'
 * '<S162>' : 'driveController/TVC/PID Controller/P Copy/Disabled'
 * '<S163>' : 'driveController/TVC/PID Controller/Parallel P Gain/Internal Parameters'
 * '<S164>' : 'driveController/TVC/PID Controller/Reset Signal/External Reset'
 * '<S165>' : 'driveController/TVC/PID Controller/Saturation/Enabled'
 * '<S166>' : 'driveController/TVC/PID Controller/Saturation Fdbk/Disabled'
 * '<S167>' : 'driveController/TVC/PID Controller/Sum/Sum_PID'
 * '<S168>' : 'driveController/TVC/PID Controller/Sum Fdbk/Disabled'
 * '<S169>' : 'driveController/TVC/PID Controller/Tracking Mode/Enabled'
 * '<S170>' : 'driveController/TVC/PID Controller/Tracking Mode Sum/Tracking Mode'
 * '<S171>' : 'driveController/TVC/PID Controller/Tsamp - Integral/TsSignalSpecification'
 * '<S172>' : 'driveController/TVC/PID Controller/Tsamp - Ngain/Passthrough'
 * '<S173>' : 'driveController/TVC/PID Controller/postSat Signal/Forward_Path'
 * '<S174>' : 'driveController/TVC/PID Controller/preInt Signal/Internal PreInt'
 * '<S175>' : 'driveController/TVC/PID Controller/preSat Signal/Forward_Path'
 * '<S176>' : 'driveController/Yaw_th/Bicycle Model Dynamic'
 * '<S177>' : 'driveController/Yaw_th/tand safe'
 * '<S178>' : 'driveController/Yaw_th/yaw_saturation'
 */
#endif                                 /* driveController_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
