/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: driveController.h
 *
 * Code generated for Simulink model 'driveController'.
 *
 * Model version                  : 1.16
 * Simulink Coder version         : 26.1 (R2026a) 20-Nov-2025
 * C/C++ source code generated on : Fri Jul 31 11:03:13 2026
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
  double Switch1;                      /* '<Root>/Switch1' */
  double dT_real;                      /* '<S8>/Product' */
  double T_RL_cmd;                     /* '<S8>/TorqueAllocator' */
  double T_RR_cmd;                     /* '<S8>/TorqueAllocator' */
  double T_max_RL;                     /* '<Root>/MATLAB Function1' */
  double T_max_RR;                     /* '<Root>/MATLAB Function1' */
} B_driveController_T;

/* Block states (default storage) for system '<Root>' */
typedef struct {
  double Integrator_DSTATE;            /* '<S103>/Integrator' */
  double Filter_DSTATE;                /* '<S98>/Filter' */
  double Integrator_DSTATE_o;          /* '<S51>/Integrator' */
  double Filter_DSTATE_e;              /* '<S46>/Filter' */
  double Integrator_DSTATE_a;          /* '<S160>/Integrator' */
  double Filter_DSTATE_g;              /* '<S155>/Filter' */
  double PrevY;                        /* '<S2>/Rate Limiter1' */
  double PrevY_j;                      /* '<S2>/Rate Limiter2' */
  double PrevY_p;                      /* '<Root>/Rate Limiter3' */
  double PrevY_a;                      /* '<S5>/Rate Limiter' */
  double PrevY_f;                      /* '<S5>/Rate Limiter1' */
  double PrevY_e;                      /* '<S9>/Rate Lmiter' */
  double Memory_PreviousInput;         /* '<S8>/Memory' */
  double PrevY_c;                      /* '<Root>/Rate Limiter1' */
  double PrevY_i;                      /* '<Root>/Rate Limiter2' */
  double PrevY_ck;                     /* '<Root>/W_RateLimiter' */
  double t_elapsed_s;                  /* '<Root>/MATLAB Function' */
  int8_t Integrator_PrevResetState;    /* '<S103>/Integrator' */
  int8_t Filter_PrevResetState;        /* '<S98>/Filter' */
  int8_t Integrator_PrevResetState_a;  /* '<S51>/Integrator' */
  int8_t Filter_PrevResetState_p;      /* '<S46>/Filter' */
  int8_t Integrator_PrevResetState_g;  /* '<S160>/Integrator' */
  int8_t Filter_PrevResetState_h;      /* '<S155>/Filter' */
  uint8_t Integrator_IC_LOADING;       /* '<S160>/Integrator' */
  uint8_t is_active_c30_driveController;/* '<Root>/Mode Manager' */
  uint8_t is_c30_driveController;      /* '<Root>/Mode Manager' */
} DW_driveController_T;

/* Invariant block signals (default storage) */
typedef struct {
  const double Constant;               /* '<S8>/Constant' */
} ConstB_driveController_T;

/* Constant parameters (default storage) */
typedef struct {
  /* Expression: pedal_map
   * Referenced by: '<S2>/1-D Lookup Table'
   */
  double uDLookupTable_tableData[11];

  /* Expression: pedal_bp
   * Referenced by: '<S2>/1-D Lookup Table'
   */
  double uDLookupTable_bp01Data[11];

  /* Expression: [1,1,0]
   * Referenced by: '<Root>/1-D Lookup Table'
   */
  double uDLookupTable_tableData_j[3];

  /* Expression: [0,3,12]
   * Referenced by: '<Root>/1-D Lookup Table'
   */
  double uDLookupTable_bp01Data_b[3];

  /* Expression: [0,0,1]
   * Referenced by: '<S8>/1-D Lookup Table'
   */
  double uDLookupTable_tableData_b[3];

  /* Expression: [0,steering_deadband,5]
   * Referenced by: '<S8>/1-D Lookup Table'
   */
  double uDLookupTable_bp01Data_m[3];
} ConstP_driveController_T;

/* External inputs (root inport signals with default storage) */
typedef struct {
  double Throttle_;                    /* '<Root>/Throttle_%' */
  double Vx;                           /* '<Root>/Vx' */
  double ax;                           /* '<Root>/ax' */
  double ay;                           /* '<Root>/ay' */
  double yaw_meas;                     /* '<Root>/Yaw_rate_act' */
  double Drad;                         /* '<Root>/D_rad' */
  double brake_;                       /* '<Root>/brake_%' */
  double T_RL_act;                     /* '<Root>/T_RL_act' */
  double T_RR_act;                     /* '<Root>/T_RR_act' */
  bool Derating_RL;                    /* '<Root>/Derating_RL' */
  bool Derating_RR;                    /* '<Root>/Derating_RR' */
  double n_motorRL;                    /* '<Root>/n_motorRL' */
  double n_motorRR;                    /* '<Root>/n_motorRR' */
  double lambda_refRL;                 /* '<Root>/lambda_refRL' */
  double lambda_refRR;                 /* '<Root>/lambda_refRR' */
} ExtU_driveController_T;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  double T_RL;                         /* '<Root>/T_RL' */
  double T_RR;                         /* '<Root>/T_RR' */
  double yaw_th;                       /* '<Root>/yaw_th' */
  double TVC_Target_Weight;            /* '<Root>/TVC_Target_Weight' */
  double K_slips[2];                   /* '<Root>/K_slips' */
  double regen;                        /* '<Root>/regen' */
  double lambda[4];                    /* '<Root>/lambda' */
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
      uint8_t TID[3];
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
extern double V_dc_p;                  /* Variable: V_dc_p
                                        * Referenced by: '<S8>/TorqueAllocator'
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
 * Block '<Root>/Scope' : Unused code path elimination
 * Block '<S8>/Scope' : Unused code path elimination
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
 * '<S1>'   : 'driveController/Compare To Constant'
 * '<S2>'   : 'driveController/LaunchThrottleController'
 * '<S3>'   : 'driveController/MATLAB Function'
 * '<S4>'   : 'driveController/MATLAB Function1'
 * '<S5>'   : 'driveController/MTFC'
 * '<S6>'   : 'driveController/Mode Manager'
 * '<S7>'   : 'driveController/SRC'
 * '<S8>'   : 'driveController/TVC'
 * '<S9>'   : 'driveController/Yaw_th'
 * '<S10>'  : 'driveController/SRC/Compare To Constant1'
 * '<S11>'  : 'driveController/SRC/Compare To Constant2'
 * '<S12>'  : 'driveController/SRC/Compare To Constant4'
 * '<S13>'  : 'driveController/SRC/MATLAB Function'
 * '<S14>'  : 'driveController/SRC/MATLAB Function1'
 * '<S15>'  : 'driveController/SRC/PID Controller'
 * '<S16>'  : 'driveController/SRC/PID Controller1'
 * '<S17>'  : 'driveController/SRC/PID Controller/Anti-windup'
 * '<S18>'  : 'driveController/SRC/PID Controller/D Gain'
 * '<S19>'  : 'driveController/SRC/PID Controller/External Derivative'
 * '<S20>'  : 'driveController/SRC/PID Controller/Filter'
 * '<S21>'  : 'driveController/SRC/PID Controller/Filter ICs'
 * '<S22>'  : 'driveController/SRC/PID Controller/I Gain'
 * '<S23>'  : 'driveController/SRC/PID Controller/Ideal P Gain'
 * '<S24>'  : 'driveController/SRC/PID Controller/Ideal P Gain Fdbk'
 * '<S25>'  : 'driveController/SRC/PID Controller/Integrator'
 * '<S26>'  : 'driveController/SRC/PID Controller/Integrator ICs'
 * '<S27>'  : 'driveController/SRC/PID Controller/N Copy'
 * '<S28>'  : 'driveController/SRC/PID Controller/N Gain'
 * '<S29>'  : 'driveController/SRC/PID Controller/P Copy'
 * '<S30>'  : 'driveController/SRC/PID Controller/Parallel P Gain'
 * '<S31>'  : 'driveController/SRC/PID Controller/Reset Signal'
 * '<S32>'  : 'driveController/SRC/PID Controller/Saturation'
 * '<S33>'  : 'driveController/SRC/PID Controller/Saturation Fdbk'
 * '<S34>'  : 'driveController/SRC/PID Controller/Sum'
 * '<S35>'  : 'driveController/SRC/PID Controller/Sum Fdbk'
 * '<S36>'  : 'driveController/SRC/PID Controller/Tracking Mode'
 * '<S37>'  : 'driveController/SRC/PID Controller/Tracking Mode Sum'
 * '<S38>'  : 'driveController/SRC/PID Controller/Tsamp - Integral'
 * '<S39>'  : 'driveController/SRC/PID Controller/Tsamp - Ngain'
 * '<S40>'  : 'driveController/SRC/PID Controller/postSat Signal'
 * '<S41>'  : 'driveController/SRC/PID Controller/preInt Signal'
 * '<S42>'  : 'driveController/SRC/PID Controller/preSat Signal'
 * '<S43>'  : 'driveController/SRC/PID Controller/Anti-windup/Back Calculation'
 * '<S44>'  : 'driveController/SRC/PID Controller/D Gain/Internal Parameters'
 * '<S45>'  : 'driveController/SRC/PID Controller/External Derivative/Error'
 * '<S46>'  : 'driveController/SRC/PID Controller/Filter/Disc. Forward Euler Filter'
 * '<S47>'  : 'driveController/SRC/PID Controller/Filter ICs/Internal IC - Filter'
 * '<S48>'  : 'driveController/SRC/PID Controller/I Gain/Internal Parameters'
 * '<S49>'  : 'driveController/SRC/PID Controller/Ideal P Gain/Passthrough'
 * '<S50>'  : 'driveController/SRC/PID Controller/Ideal P Gain Fdbk/Disabled'
 * '<S51>'  : 'driveController/SRC/PID Controller/Integrator/Discrete'
 * '<S52>'  : 'driveController/SRC/PID Controller/Integrator ICs/Internal IC'
 * '<S53>'  : 'driveController/SRC/PID Controller/N Copy/Disabled'
 * '<S54>'  : 'driveController/SRC/PID Controller/N Gain/Internal Parameters'
 * '<S55>'  : 'driveController/SRC/PID Controller/P Copy/Disabled'
 * '<S56>'  : 'driveController/SRC/PID Controller/Parallel P Gain/Internal Parameters'
 * '<S57>'  : 'driveController/SRC/PID Controller/Reset Signal/External Reset'
 * '<S58>'  : 'driveController/SRC/PID Controller/Saturation/Enabled'
 * '<S59>'  : 'driveController/SRC/PID Controller/Saturation Fdbk/Disabled'
 * '<S60>'  : 'driveController/SRC/PID Controller/Sum/Sum_PID'
 * '<S61>'  : 'driveController/SRC/PID Controller/Sum Fdbk/Disabled'
 * '<S62>'  : 'driveController/SRC/PID Controller/Tracking Mode/Disabled'
 * '<S63>'  : 'driveController/SRC/PID Controller/Tracking Mode Sum/Passthrough'
 * '<S64>'  : 'driveController/SRC/PID Controller/Tsamp - Integral/TsSignalSpecification'
 * '<S65>'  : 'driveController/SRC/PID Controller/Tsamp - Ngain/Passthrough'
 * '<S66>'  : 'driveController/SRC/PID Controller/postSat Signal/Forward_Path'
 * '<S67>'  : 'driveController/SRC/PID Controller/preInt Signal/Internal PreInt'
 * '<S68>'  : 'driveController/SRC/PID Controller/preSat Signal/Forward_Path'
 * '<S69>'  : 'driveController/SRC/PID Controller1/Anti-windup'
 * '<S70>'  : 'driveController/SRC/PID Controller1/D Gain'
 * '<S71>'  : 'driveController/SRC/PID Controller1/External Derivative'
 * '<S72>'  : 'driveController/SRC/PID Controller1/Filter'
 * '<S73>'  : 'driveController/SRC/PID Controller1/Filter ICs'
 * '<S74>'  : 'driveController/SRC/PID Controller1/I Gain'
 * '<S75>'  : 'driveController/SRC/PID Controller1/Ideal P Gain'
 * '<S76>'  : 'driveController/SRC/PID Controller1/Ideal P Gain Fdbk'
 * '<S77>'  : 'driveController/SRC/PID Controller1/Integrator'
 * '<S78>'  : 'driveController/SRC/PID Controller1/Integrator ICs'
 * '<S79>'  : 'driveController/SRC/PID Controller1/N Copy'
 * '<S80>'  : 'driveController/SRC/PID Controller1/N Gain'
 * '<S81>'  : 'driveController/SRC/PID Controller1/P Copy'
 * '<S82>'  : 'driveController/SRC/PID Controller1/Parallel P Gain'
 * '<S83>'  : 'driveController/SRC/PID Controller1/Reset Signal'
 * '<S84>'  : 'driveController/SRC/PID Controller1/Saturation'
 * '<S85>'  : 'driveController/SRC/PID Controller1/Saturation Fdbk'
 * '<S86>'  : 'driveController/SRC/PID Controller1/Sum'
 * '<S87>'  : 'driveController/SRC/PID Controller1/Sum Fdbk'
 * '<S88>'  : 'driveController/SRC/PID Controller1/Tracking Mode'
 * '<S89>'  : 'driveController/SRC/PID Controller1/Tracking Mode Sum'
 * '<S90>'  : 'driveController/SRC/PID Controller1/Tsamp - Integral'
 * '<S91>'  : 'driveController/SRC/PID Controller1/Tsamp - Ngain'
 * '<S92>'  : 'driveController/SRC/PID Controller1/postSat Signal'
 * '<S93>'  : 'driveController/SRC/PID Controller1/preInt Signal'
 * '<S94>'  : 'driveController/SRC/PID Controller1/preSat Signal'
 * '<S95>'  : 'driveController/SRC/PID Controller1/Anti-windup/Back Calculation'
 * '<S96>'  : 'driveController/SRC/PID Controller1/D Gain/Internal Parameters'
 * '<S97>'  : 'driveController/SRC/PID Controller1/External Derivative/Error'
 * '<S98>'  : 'driveController/SRC/PID Controller1/Filter/Disc. Forward Euler Filter'
 * '<S99>'  : 'driveController/SRC/PID Controller1/Filter ICs/Internal IC - Filter'
 * '<S100>' : 'driveController/SRC/PID Controller1/I Gain/Internal Parameters'
 * '<S101>' : 'driveController/SRC/PID Controller1/Ideal P Gain/Passthrough'
 * '<S102>' : 'driveController/SRC/PID Controller1/Ideal P Gain Fdbk/Disabled'
 * '<S103>' : 'driveController/SRC/PID Controller1/Integrator/Discrete'
 * '<S104>' : 'driveController/SRC/PID Controller1/Integrator ICs/Internal IC'
 * '<S105>' : 'driveController/SRC/PID Controller1/N Copy/Disabled'
 * '<S106>' : 'driveController/SRC/PID Controller1/N Gain/Internal Parameters'
 * '<S107>' : 'driveController/SRC/PID Controller1/P Copy/Disabled'
 * '<S108>' : 'driveController/SRC/PID Controller1/Parallel P Gain/Internal Parameters'
 * '<S109>' : 'driveController/SRC/PID Controller1/Reset Signal/External Reset'
 * '<S110>' : 'driveController/SRC/PID Controller1/Saturation/Enabled'
 * '<S111>' : 'driveController/SRC/PID Controller1/Saturation Fdbk/Disabled'
 * '<S112>' : 'driveController/SRC/PID Controller1/Sum/Sum_PID'
 * '<S113>' : 'driveController/SRC/PID Controller1/Sum Fdbk/Disabled'
 * '<S114>' : 'driveController/SRC/PID Controller1/Tracking Mode/Disabled'
 * '<S115>' : 'driveController/SRC/PID Controller1/Tracking Mode Sum/Passthrough'
 * '<S116>' : 'driveController/SRC/PID Controller1/Tsamp - Integral/TsSignalSpecification'
 * '<S117>' : 'driveController/SRC/PID Controller1/Tsamp - Ngain/Passthrough'
 * '<S118>' : 'driveController/SRC/PID Controller1/postSat Signal/Forward_Path'
 * '<S119>' : 'driveController/SRC/PID Controller1/preInt Signal/Internal PreInt'
 * '<S120>' : 'driveController/SRC/PID Controller1/preSat Signal/Forward_Path'
 * '<S121>' : 'driveController/TVC/Compare To Constant3'
 * '<S122>' : 'driveController/TVC/MATLAB Function'
 * '<S123>' : 'driveController/TVC/MATLAB Function1'
 * '<S124>' : 'driveController/TVC/PID Controller'
 * '<S125>' : 'driveController/TVC/TorqueAllocator'
 * '<S126>' : 'driveController/TVC/PID Controller/Anti-windup'
 * '<S127>' : 'driveController/TVC/PID Controller/D Gain'
 * '<S128>' : 'driveController/TVC/PID Controller/External Derivative'
 * '<S129>' : 'driveController/TVC/PID Controller/Filter'
 * '<S130>' : 'driveController/TVC/PID Controller/Filter ICs'
 * '<S131>' : 'driveController/TVC/PID Controller/I Gain'
 * '<S132>' : 'driveController/TVC/PID Controller/Ideal P Gain'
 * '<S133>' : 'driveController/TVC/PID Controller/Ideal P Gain Fdbk'
 * '<S134>' : 'driveController/TVC/PID Controller/Integrator'
 * '<S135>' : 'driveController/TVC/PID Controller/Integrator ICs'
 * '<S136>' : 'driveController/TVC/PID Controller/N Copy'
 * '<S137>' : 'driveController/TVC/PID Controller/N Gain'
 * '<S138>' : 'driveController/TVC/PID Controller/P Copy'
 * '<S139>' : 'driveController/TVC/PID Controller/Parallel P Gain'
 * '<S140>' : 'driveController/TVC/PID Controller/Reset Signal'
 * '<S141>' : 'driveController/TVC/PID Controller/Saturation'
 * '<S142>' : 'driveController/TVC/PID Controller/Saturation Fdbk'
 * '<S143>' : 'driveController/TVC/PID Controller/Sum'
 * '<S144>' : 'driveController/TVC/PID Controller/Sum Fdbk'
 * '<S145>' : 'driveController/TVC/PID Controller/Tracking Mode'
 * '<S146>' : 'driveController/TVC/PID Controller/Tracking Mode Sum'
 * '<S147>' : 'driveController/TVC/PID Controller/Tsamp - Integral'
 * '<S148>' : 'driveController/TVC/PID Controller/Tsamp - Ngain'
 * '<S149>' : 'driveController/TVC/PID Controller/postSat Signal'
 * '<S150>' : 'driveController/TVC/PID Controller/preInt Signal'
 * '<S151>' : 'driveController/TVC/PID Controller/preSat Signal'
 * '<S152>' : 'driveController/TVC/PID Controller/Anti-windup/Back Calculation'
 * '<S153>' : 'driveController/TVC/PID Controller/D Gain/Internal Parameters'
 * '<S154>' : 'driveController/TVC/PID Controller/External Derivative/Error'
 * '<S155>' : 'driveController/TVC/PID Controller/Filter/Disc. Forward Euler Filter'
 * '<S156>' : 'driveController/TVC/PID Controller/Filter ICs/External IC'
 * '<S157>' : 'driveController/TVC/PID Controller/I Gain/Internal Parameters'
 * '<S158>' : 'driveController/TVC/PID Controller/Ideal P Gain/Passthrough'
 * '<S159>' : 'driveController/TVC/PID Controller/Ideal P Gain Fdbk/Disabled'
 * '<S160>' : 'driveController/TVC/PID Controller/Integrator/Discrete'
 * '<S161>' : 'driveController/TVC/PID Controller/Integrator ICs/External IC'
 * '<S162>' : 'driveController/TVC/PID Controller/N Copy/Disabled'
 * '<S163>' : 'driveController/TVC/PID Controller/N Gain/Internal Parameters'
 * '<S164>' : 'driveController/TVC/PID Controller/P Copy/Disabled'
 * '<S165>' : 'driveController/TVC/PID Controller/Parallel P Gain/Internal Parameters'
 * '<S166>' : 'driveController/TVC/PID Controller/Reset Signal/External Reset'
 * '<S167>' : 'driveController/TVC/PID Controller/Saturation/Enabled'
 * '<S168>' : 'driveController/TVC/PID Controller/Saturation Fdbk/Disabled'
 * '<S169>' : 'driveController/TVC/PID Controller/Sum/Sum_PID'
 * '<S170>' : 'driveController/TVC/PID Controller/Sum Fdbk/Disabled'
 * '<S171>' : 'driveController/TVC/PID Controller/Tracking Mode/Enabled'
 * '<S172>' : 'driveController/TVC/PID Controller/Tracking Mode Sum/Tracking Mode'
 * '<S173>' : 'driveController/TVC/PID Controller/Tsamp - Integral/TsSignalSpecification'
 * '<S174>' : 'driveController/TVC/PID Controller/Tsamp - Ngain/Passthrough'
 * '<S175>' : 'driveController/TVC/PID Controller/postSat Signal/Forward_Path'
 * '<S176>' : 'driveController/TVC/PID Controller/preInt Signal/Internal PreInt'
 * '<S177>' : 'driveController/TVC/PID Controller/preSat Signal/Forward_Path'
 * '<S178>' : 'driveController/Yaw_th/Bicycle Model Dynamic'
 * '<S179>' : 'driveController/Yaw_th/tand safe'
 * '<S180>' : 'driveController/Yaw_th/yaw_saturation'
 */
#endif                                 /* driveController_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
