/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: driveController.h
 *
 * Code generated for Simulink model 'driveController'.
 *
 * Model version                  : 1.66
 * Simulink Coder version         : 26.1 (R2026a) 20-Nov-2025
 * C/C++ source code generated on : Thu Sep  3 10:00:25 2026
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

/* Block states (default storage) for system '<Root>' */
typedef struct {
  double Integrator_DSTATE;            /* '<S104>/Integrator' */
  double Filter_DSTATE;                /* '<S99>/Filter' */
  double Integrator_DSTATE_g;          /* '<S52>/Integrator' */
  double Filter_DSTATE_i;              /* '<S47>/Filter' */
  double Integrator_DSTATE_k;          /* '<S160>/Integrator' */
  double Filter_DSTATE_e;              /* '<S155>/Filter' */
  double PrevY;                        /* '<S3>/Rate Limiter1' */
  double PrevY_p;                      /* '<S3>/Rate Limiter2' */
  double PrevY_h;                      /* '<S4>/Rate Limiter' */
  double PrevY_k;                      /* '<S4>/Rate Limiter1' */
  double PrevY_k0;                     /* '<Root>/W_RateLimiter' */
  double t_elapsed_s;                  /* '<Root>/Regen MATLAB Function' */
  float PrevY_m;                       /* '<Root>/Rate Limiter3' */
  float PrevY_a;                       /* '<S9>/Rate Lmiter' */
  float PrevY_f;                       /* '<Root>/Rate Limiter1' */
  float PrevY_j;                       /* '<Root>/Rate Limiter2' */
  int8_t Integrator_PrevResetState;    /* '<S104>/Integrator' */
  int8_t Filter_PrevResetState;        /* '<S99>/Filter' */
  int8_t Integrator_PrevResetState_l;  /* '<S52>/Integrator' */
  int8_t Filter_PrevResetState_i;      /* '<S47>/Filter' */
  int8_t Integrator_PrevResetState_a;  /* '<S160>/Integrator' */
  int8_t Filter_PrevResetState_a;      /* '<S155>/Filter' */
  uint8_t is_active_c30_driveController;/* '<Root>/Mode Manager' */
  uint8_t is_c30_driveController;      /* '<Root>/Mode Manager' */
  bool regen_latch;                    /* '<Root>/Regen MATLAB Function' */
} DW_driveController_T;

/* Constant parameters (default storage) */
typedef struct {
  /* Expression: pedal_map_eco
   * Referenced by: '<S3>/1-D Lookup Table'
   */
  double uDLookupTable_tableData[21];

  /* Pooled Parameter (Expression: pedal_bp)
   * Referenced by:
   *   '<S3>/1-D Lookup Table'
   *   '<S3>/1-D Lookup Table1'
   *   '<S3>/1-D Lookup Table2'
   */
  double pooled7[21];

  /* Expression: pedal_map_normal
   * Referenced by: '<S3>/1-D Lookup Table1'
   */
  double uDLookupTable1_tableData[21];

  /* Expression: pedal_map_gas
   * Referenced by: '<S3>/1-D Lookup Table2'
   */
  double uDLookupTable2_tableData[21];

  /* Expression: [1,1,0]
   * Referenced by: '<Root>/1-D Lookup Table'
   */
  double uDLookupTable_tableData_o[3];

  /* Expression: [0,3,12]
   * Referenced by: '<Root>/1-D Lookup Table'
   */
  double uDLookupTable_bp01Data[3];

  /* Expression: [0,0,1]
   * Referenced by: '<S8>/1-D Lookup Table'
   */
  double uDLookupTable_tableData_j[3];

  /* Expression: [0,steering_deadband,4]
   * Referenced by: '<S8>/1-D Lookup Table'
   */
  double uDLookupTable_bp01Data_i[3];
} ConstP_driveController_T;

/* External inputs (root inport signals with default storage) */
typedef struct {
  double Throttle_;                    /* '<Root>/Throttle_%' */
  double Vx;                           /* '<Root>/Vx' */
  double ax;                           /* '<Root>/ax' */
  double ay;                           /* '<Root>/ay' */
  float yaw_meas;                      /* '<Root>/Yaw_rate_act' */
  double SteeringAngle;                /* '<Root>/Steering Angle °' */
  double brake_;                       /* '<Root>/brake_%' */
  bool Derating_RL;                    /* '<Root>/Derating_RL' */
  bool Derating_RR;                    /* '<Root>/Derating_RR' */
  double n_motorRL;                    /* '<Root>/n_motorRL' */
  double n_motorRR;                    /* '<Root>/n_motorRR' */
  double lambda_refRL;                 /* '<Root>/lambda_refRL' */
  double lambda_refRR;                 /* '<Root>/lambda_refRR' */
} ExtU_driveController_T;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  float T_RL;                          /* '<Root>/T_RL' */
  float T_RR;                          /* '<Root>/T_RR' */
  float yaw_th;                        /* '<Root>/yaw_th' */
  double TVC_Target_Weight;            /* '<Root>/TVC_Target_Weight' */
  double K_slips[2];                   /* '<Root>/K_slips' */
  double regen;                        /* '<Root>/regen' */
  double lambda[4];                    /* '<Root>/lambda' */
  double Throttle_real;                /* '<Root>/Throttle_real' */
  float T_max_RL;                      /* '<Root>/T_max_RL' */
  float T_max_RR;                      /* '<Root>/T_max_RR' */
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

/* Block states (default storage) */
extern DW_driveController_T driveController_DW;

/* External inputs (root inport signals with default storage) */
extern ExtU_driveController_T driveController_U;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY_driveController_T driveController_Y;

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
extern float I_charge_max_p;           /* Variable: I_charge_max_p
                                        * Referenced by:
                                        *   '<Root>/Regen MATLAB Function'
                                        *   '<S8>/TorqueAllocator'
                                        */
extern float I_discharge_max_p;        /* Variable: I_discharge_max_p
                                        * Referenced by: '<S8>/TorqueAllocator'
                                        */
extern float Kus_p;                    /* Variable: Kus_p
                                        * Referenced by: '<S9>/Bicycle Model Dynamic'
                                        */
extern float T_peak_p;                 /* Variable: T_peak_p
                                        * Referenced by:
                                        *   '<Root>/Saturation'
                                        *   '<Root>/Saturation1'
                                        *   '<S8>/TorqueAllocator'
                                        */
extern float V_dc_p;                   /* Variable: V_dc_p
                                        * Referenced by:
                                        *   '<Root>/Regen MATLAB Function'
                                        *   '<S8>/TorqueAllocator'
                                        */
extern float beta_dot_threshold_p;     /* Variable: beta_dot_threshold_p
                                        * Referenced by: '<Root>/Grip MATLAB Estimator'
                                        */
extern float mu_p;                     /* Variable: mu_p
                                        * Referenced by: '<Root>/Grip MATLAB Estimator'
                                        */
extern float overslip_factor_p;        /* Variable: overslip_factor_p
                                        * Referenced by: '<Root>/Grip MATLAB Estimator'
                                        */
extern float regen_T_max_p;            /* Variable: regen_T_max_p
                                        * Referenced by:
                                        *   '<Root>/Regen MATLAB Function'
                                        *   '<Root>/Saturation'
                                        *   '<Root>/Saturation1'
                                        *   '<S8>/TorqueAllocator'
                                        */
extern float regen_pedal_hyst_p;       /* Variable: regen_pedal_hyst_p
                                        * Referenced by: '<Root>/Regen MATLAB Function'
                                        */
extern float regen_pedal_thr_p;        /* Variable: regen_pedal_thr_p
                                        * Referenced by: '<Root>/Regen MATLAB Function'
                                        */
extern float slip_Kd_p;                /* Variable: slip_Kd_p
                                        * Referenced by:
                                        *   '<S45>/Derivative Gain'
                                        *   '<S97>/Derivative Gain'
                                        */
extern float slip_Ki_p;                /* Variable: slip_Ki_p
                                        * Referenced by:
                                        *   '<S49>/Integral Gain'
                                        *   '<S101>/Integral Gain'
                                        */
extern float slip_Kp_p;                /* Variable: slip_Kp_p
                                        * Referenced by:
                                        *   '<S57>/Proportional Gain'
                                        *   '<S109>/Proportional Gain'
                                        */
extern float tvc_Kd_p;                 /* Variable: tvc_Kd_p
                                        * Referenced by: '<S153>/Derivative Gain'
                                        */
extern float tvc_Ki_p;                 /* Variable: tvc_Ki_p
                                        * Referenced by: '<S157>/Integral Gain'
                                        */
extern float tvc_Kp_p;                 /* Variable: tvc_Kp_p
                                        * Referenced by: '<S165>/Proportional Gain'
                                        */
extern uint8_t pedal_map_select_p;     /* Variable: pedal_map_select_p
                                        * Referenced by: '<S3>/Constant1'
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
 * Block '<S9>/Data Type Conversion' : Eliminate redundant data type conversion
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
 * '<S2>'   : 'driveController/Grip MATLAB Estimator'
 * '<S3>'   : 'driveController/LaunchThrottleController'
 * '<S4>'   : 'driveController/MTFC'
 * '<S5>'   : 'driveController/Mode Manager'
 * '<S6>'   : 'driveController/Regen MATLAB Function'
 * '<S7>'   : 'driveController/SRC'
 * '<S8>'   : 'driveController/TVC'
 * '<S9>'   : 'driveController/Yaw_th'
 * '<S10>'  : 'driveController/beta_dot_estimator'
 * '<S11>'  : 'driveController/SRC/Compare To Constant1'
 * '<S12>'  : 'driveController/SRC/Compare To Constant2'
 * '<S13>'  : 'driveController/SRC/Compare To Constant4'
 * '<S14>'  : 'driveController/SRC/MATLAB Function'
 * '<S15>'  : 'driveController/SRC/MATLAB Function1'
 * '<S16>'  : 'driveController/SRC/PID Controller'
 * '<S17>'  : 'driveController/SRC/PID Controller1'
 * '<S18>'  : 'driveController/SRC/PID Controller/Anti-windup'
 * '<S19>'  : 'driveController/SRC/PID Controller/D Gain'
 * '<S20>'  : 'driveController/SRC/PID Controller/External Derivative'
 * '<S21>'  : 'driveController/SRC/PID Controller/Filter'
 * '<S22>'  : 'driveController/SRC/PID Controller/Filter ICs'
 * '<S23>'  : 'driveController/SRC/PID Controller/I Gain'
 * '<S24>'  : 'driveController/SRC/PID Controller/Ideal P Gain'
 * '<S25>'  : 'driveController/SRC/PID Controller/Ideal P Gain Fdbk'
 * '<S26>'  : 'driveController/SRC/PID Controller/Integrator'
 * '<S27>'  : 'driveController/SRC/PID Controller/Integrator ICs'
 * '<S28>'  : 'driveController/SRC/PID Controller/N Copy'
 * '<S29>'  : 'driveController/SRC/PID Controller/N Gain'
 * '<S30>'  : 'driveController/SRC/PID Controller/P Copy'
 * '<S31>'  : 'driveController/SRC/PID Controller/Parallel P Gain'
 * '<S32>'  : 'driveController/SRC/PID Controller/Reset Signal'
 * '<S33>'  : 'driveController/SRC/PID Controller/Saturation'
 * '<S34>'  : 'driveController/SRC/PID Controller/Saturation Fdbk'
 * '<S35>'  : 'driveController/SRC/PID Controller/Sum'
 * '<S36>'  : 'driveController/SRC/PID Controller/Sum Fdbk'
 * '<S37>'  : 'driveController/SRC/PID Controller/Tracking Mode'
 * '<S38>'  : 'driveController/SRC/PID Controller/Tracking Mode Sum'
 * '<S39>'  : 'driveController/SRC/PID Controller/Tsamp - Integral'
 * '<S40>'  : 'driveController/SRC/PID Controller/Tsamp - Ngain'
 * '<S41>'  : 'driveController/SRC/PID Controller/postSat Signal'
 * '<S42>'  : 'driveController/SRC/PID Controller/preInt Signal'
 * '<S43>'  : 'driveController/SRC/PID Controller/preSat Signal'
 * '<S44>'  : 'driveController/SRC/PID Controller/Anti-windup/Back Calculation'
 * '<S45>'  : 'driveController/SRC/PID Controller/D Gain/Internal Parameters'
 * '<S46>'  : 'driveController/SRC/PID Controller/External Derivative/Error'
 * '<S47>'  : 'driveController/SRC/PID Controller/Filter/Disc. Forward Euler Filter'
 * '<S48>'  : 'driveController/SRC/PID Controller/Filter ICs/Internal IC - Filter'
 * '<S49>'  : 'driveController/SRC/PID Controller/I Gain/Internal Parameters'
 * '<S50>'  : 'driveController/SRC/PID Controller/Ideal P Gain/Passthrough'
 * '<S51>'  : 'driveController/SRC/PID Controller/Ideal P Gain Fdbk/Disabled'
 * '<S52>'  : 'driveController/SRC/PID Controller/Integrator/Discrete'
 * '<S53>'  : 'driveController/SRC/PID Controller/Integrator ICs/Internal IC'
 * '<S54>'  : 'driveController/SRC/PID Controller/N Copy/Disabled'
 * '<S55>'  : 'driveController/SRC/PID Controller/N Gain/Internal Parameters'
 * '<S56>'  : 'driveController/SRC/PID Controller/P Copy/Disabled'
 * '<S57>'  : 'driveController/SRC/PID Controller/Parallel P Gain/Internal Parameters'
 * '<S58>'  : 'driveController/SRC/PID Controller/Reset Signal/External Reset'
 * '<S59>'  : 'driveController/SRC/PID Controller/Saturation/Enabled'
 * '<S60>'  : 'driveController/SRC/PID Controller/Saturation Fdbk/Disabled'
 * '<S61>'  : 'driveController/SRC/PID Controller/Sum/Sum_PID'
 * '<S62>'  : 'driveController/SRC/PID Controller/Sum Fdbk/Disabled'
 * '<S63>'  : 'driveController/SRC/PID Controller/Tracking Mode/Disabled'
 * '<S64>'  : 'driveController/SRC/PID Controller/Tracking Mode Sum/Passthrough'
 * '<S65>'  : 'driveController/SRC/PID Controller/Tsamp - Integral/TsSignalSpecification'
 * '<S66>'  : 'driveController/SRC/PID Controller/Tsamp - Ngain/Passthrough'
 * '<S67>'  : 'driveController/SRC/PID Controller/postSat Signal/Forward_Path'
 * '<S68>'  : 'driveController/SRC/PID Controller/preInt Signal/Internal PreInt'
 * '<S69>'  : 'driveController/SRC/PID Controller/preSat Signal/Forward_Path'
 * '<S70>'  : 'driveController/SRC/PID Controller1/Anti-windup'
 * '<S71>'  : 'driveController/SRC/PID Controller1/D Gain'
 * '<S72>'  : 'driveController/SRC/PID Controller1/External Derivative'
 * '<S73>'  : 'driveController/SRC/PID Controller1/Filter'
 * '<S74>'  : 'driveController/SRC/PID Controller1/Filter ICs'
 * '<S75>'  : 'driveController/SRC/PID Controller1/I Gain'
 * '<S76>'  : 'driveController/SRC/PID Controller1/Ideal P Gain'
 * '<S77>'  : 'driveController/SRC/PID Controller1/Ideal P Gain Fdbk'
 * '<S78>'  : 'driveController/SRC/PID Controller1/Integrator'
 * '<S79>'  : 'driveController/SRC/PID Controller1/Integrator ICs'
 * '<S80>'  : 'driveController/SRC/PID Controller1/N Copy'
 * '<S81>'  : 'driveController/SRC/PID Controller1/N Gain'
 * '<S82>'  : 'driveController/SRC/PID Controller1/P Copy'
 * '<S83>'  : 'driveController/SRC/PID Controller1/Parallel P Gain'
 * '<S84>'  : 'driveController/SRC/PID Controller1/Reset Signal'
 * '<S85>'  : 'driveController/SRC/PID Controller1/Saturation'
 * '<S86>'  : 'driveController/SRC/PID Controller1/Saturation Fdbk'
 * '<S87>'  : 'driveController/SRC/PID Controller1/Sum'
 * '<S88>'  : 'driveController/SRC/PID Controller1/Sum Fdbk'
 * '<S89>'  : 'driveController/SRC/PID Controller1/Tracking Mode'
 * '<S90>'  : 'driveController/SRC/PID Controller1/Tracking Mode Sum'
 * '<S91>'  : 'driveController/SRC/PID Controller1/Tsamp - Integral'
 * '<S92>'  : 'driveController/SRC/PID Controller1/Tsamp - Ngain'
 * '<S93>'  : 'driveController/SRC/PID Controller1/postSat Signal'
 * '<S94>'  : 'driveController/SRC/PID Controller1/preInt Signal'
 * '<S95>'  : 'driveController/SRC/PID Controller1/preSat Signal'
 * '<S96>'  : 'driveController/SRC/PID Controller1/Anti-windup/Back Calculation'
 * '<S97>'  : 'driveController/SRC/PID Controller1/D Gain/Internal Parameters'
 * '<S98>'  : 'driveController/SRC/PID Controller1/External Derivative/Error'
 * '<S99>'  : 'driveController/SRC/PID Controller1/Filter/Disc. Forward Euler Filter'
 * '<S100>' : 'driveController/SRC/PID Controller1/Filter ICs/Internal IC - Filter'
 * '<S101>' : 'driveController/SRC/PID Controller1/I Gain/Internal Parameters'
 * '<S102>' : 'driveController/SRC/PID Controller1/Ideal P Gain/Passthrough'
 * '<S103>' : 'driveController/SRC/PID Controller1/Ideal P Gain Fdbk/Disabled'
 * '<S104>' : 'driveController/SRC/PID Controller1/Integrator/Discrete'
 * '<S105>' : 'driveController/SRC/PID Controller1/Integrator ICs/Internal IC'
 * '<S106>' : 'driveController/SRC/PID Controller1/N Copy/Disabled'
 * '<S107>' : 'driveController/SRC/PID Controller1/N Gain/Internal Parameters'
 * '<S108>' : 'driveController/SRC/PID Controller1/P Copy/Disabled'
 * '<S109>' : 'driveController/SRC/PID Controller1/Parallel P Gain/Internal Parameters'
 * '<S110>' : 'driveController/SRC/PID Controller1/Reset Signal/External Reset'
 * '<S111>' : 'driveController/SRC/PID Controller1/Saturation/Enabled'
 * '<S112>' : 'driveController/SRC/PID Controller1/Saturation Fdbk/Disabled'
 * '<S113>' : 'driveController/SRC/PID Controller1/Sum/Sum_PID'
 * '<S114>' : 'driveController/SRC/PID Controller1/Sum Fdbk/Disabled'
 * '<S115>' : 'driveController/SRC/PID Controller1/Tracking Mode/Disabled'
 * '<S116>' : 'driveController/SRC/PID Controller1/Tracking Mode Sum/Passthrough'
 * '<S117>' : 'driveController/SRC/PID Controller1/Tsamp - Integral/TsSignalSpecification'
 * '<S118>' : 'driveController/SRC/PID Controller1/Tsamp - Ngain/Passthrough'
 * '<S119>' : 'driveController/SRC/PID Controller1/postSat Signal/Forward_Path'
 * '<S120>' : 'driveController/SRC/PID Controller1/preInt Signal/Internal PreInt'
 * '<S121>' : 'driveController/SRC/PID Controller1/preSat Signal/Forward_Path'
 * '<S122>' : 'driveController/TVC/Compare To Constant'
 * '<S123>' : 'driveController/TVC/MATLAB Function'
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
 * '<S156>' : 'driveController/TVC/PID Controller/Filter ICs/Internal IC - Filter'
 * '<S157>' : 'driveController/TVC/PID Controller/I Gain/Internal Parameters'
 * '<S158>' : 'driveController/TVC/PID Controller/Ideal P Gain/Passthrough'
 * '<S159>' : 'driveController/TVC/PID Controller/Ideal P Gain Fdbk/Disabled'
 * '<S160>' : 'driveController/TVC/PID Controller/Integrator/Discrete'
 * '<S161>' : 'driveController/TVC/PID Controller/Integrator ICs/Internal IC'
 * '<S162>' : 'driveController/TVC/PID Controller/N Copy/Disabled'
 * '<S163>' : 'driveController/TVC/PID Controller/N Gain/Internal Parameters'
 * '<S164>' : 'driveController/TVC/PID Controller/P Copy/Disabled'
 * '<S165>' : 'driveController/TVC/PID Controller/Parallel P Gain/Internal Parameters'
 * '<S166>' : 'driveController/TVC/PID Controller/Reset Signal/External Reset'
 * '<S167>' : 'driveController/TVC/PID Controller/Saturation/Enabled'
 * '<S168>' : 'driveController/TVC/PID Controller/Saturation Fdbk/Disabled'
 * '<S169>' : 'driveController/TVC/PID Controller/Sum/Sum_PID'
 * '<S170>' : 'driveController/TVC/PID Controller/Sum Fdbk/Disabled'
 * '<S171>' : 'driveController/TVC/PID Controller/Tracking Mode/Disabled'
 * '<S172>' : 'driveController/TVC/PID Controller/Tracking Mode Sum/Passthrough'
 * '<S173>' : 'driveController/TVC/PID Controller/Tsamp - Integral/TsSignalSpecification'
 * '<S174>' : 'driveController/TVC/PID Controller/Tsamp - Ngain/Passthrough'
 * '<S175>' : 'driveController/TVC/PID Controller/postSat Signal/Forward_Path'
 * '<S176>' : 'driveController/TVC/PID Controller/preInt Signal/Internal PreInt'
 * '<S177>' : 'driveController/TVC/PID Controller/preSat Signal/Forward_Path'
 * '<S178>' : 'driveController/Yaw_th/Bicycle Model Dynamic'
 * '<S179>' : 'driveController/Yaw_th/tand safe'
 * '<S180>' : 'driveController/Yaw_th/yaw_saturation'
 * '<S181>' : 'driveController/beta_dot_estimator/beta_dot estimator'
 */
#endif                                 /* driveController_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
