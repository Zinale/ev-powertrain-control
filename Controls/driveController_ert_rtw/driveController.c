/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: driveController.c
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

#include "driveController.h"
#include <stdint.h>
#include <math.h>
#include "rt_nonfinite.h"
#include <stdbool.h>
#include "driveController_private.h"

/* Named constants for Chart: '<Root>/Mode Manager' */
#define driveController_IN_IDLE_       ((uint8_t)1U)
#define driveController_IN_TVC_        ((uint8_t)2U)
#define driveController_period         (0.01F)

/* Exported block parameters */
float T_headroom_k = 5.0F;             /* Variable: T_headroom_k
                                        * Referenced by: '<S9>/TorqueAllocator'
                                        */
float T_headroom_max = 5.0F;           /* Variable: T_headroom_max
                                        * Referenced by: '<S9>/TorqueAllocator'
                                        */
float T_rated = 9.8F;                  /* Variable: T_rated
                                        * Referenced by:
                                        *   '<S4>/Constant1'
                                        *   '<S9>/TorqueAllocator'
                                        */
float regen_T_max = 4.9F;              /* Variable: regen_T_max
                                        * Referenced by: '<Root>/MATLAB Function'
                                        */
float slip_Kd = 5.0F;                  /* Variable: slip_Kd
                                        * Referenced by:
                                        *   '<S42>/Derivative Gain'
                                        *   '<S94>/Derivative Gain'
                                        */
float slip_Ki = 110.0F;                /* Variable: slip_Ki
                                        * Referenced by:
                                        *   '<S46>/Integral Gain'
                                        *   '<S98>/Integral Gain'
                                        */
float slip_Kp = 180.0F;                /* Variable: slip_Kp
                                        * Referenced by:
                                        *   '<S54>/Proportional Gain'
                                        *   '<S106>/Proportional Gain'
                                        */
float slip_bc_coeff = 1.5F;            /* Variable: slip_bc_coeff
                                        * Referenced by:
                                        *   '<S41>/Kb'
                                        *   '<S93>/Kb'
                                        */
float tvc_Kd = 5.0F;                   /* Variable: tvc_Kd
                                        * Referenced by: '<S151>/Derivative Gain'
                                        */
float tvc_Ki = 10.0F;                  /* Variable: tvc_Ki
                                        * Referenced by: '<S155>/Integral Gain'
                                        */
float tvc_Kp = 70.0F;                  /* Variable: tvc_Kp
                                        * Referenced by: '<S163>/Proportional Gain'
                                        */
float tvc_N_filter = 20.0F;            /* Variable: tvc_N_filter
                                        * Referenced by: '<S161>/Filter Coefficient'
                                        */
float tvc_V_off = 2.0F;                /* Variable: tvc_V_off
                                        * Referenced by: '<Root>/Mode Manager'
                                        */
float tvc_V_on = 4.0F;                 /* Variable: tvc_V_on
                                        * Referenced by: '<Root>/Mode Manager'
                                        */
float tvc_low_sat = 0.0F;              /* Variable: tvc_low_sat
                                        * Referenced by: '<S165>/Saturation'
                                        */
float tvc_throttle_on = 3.0F;          /* Variable: tvc_throttle_on
                                        * Referenced by: '<Root>/Mode Manager'
                                        */
float tvc_tr = 1.0F;                   /* Variable: tvc_tr
                                        * Referenced by: '<S169>/Kt'
                                        */
float tvc_up_sat = 21.0F;              /* Variable: tvc_up_sat
                                        * Referenced by: '<S165>/Saturation'
                                        */

/* Block signals (default storage) */
B_driveController_T driveController_B;

/* Block states (default storage) */
DW_driveController_T driveController_DW;

/* External inputs (root inport signals with default storage) */
ExtU_driveController_T driveController_U;

/* External outputs (root outports fed by signals with default storage) */
ExtY_driveController_T driveController_Y;

/* Real-time model */
static RT_MODEL_driveController_T driveController_M_;
RT_MODEL_driveController_T *const driveController_M = &driveController_M_;
static void rate_scheduler(void);
float look1_iflf_binlcapw(float u0, const float bp0[], const float table[],
  uint32_t maxIndex)
{
  float frac;
  float y;
  uint32_t iLeft;

  /* Column-major Lookup 1-D
     Search method: 'binary'
     Use previous index: 'off'
     Interpolation method: 'Linear point-slope'
     Extrapolation method: 'Clip'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
   */
  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Clip'
     Use previous index: 'off'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
   */
  if (u0 <= bp0[0U]) {
    iLeft = 0U;
    frac = 0.0F;
  } else if (u0 < bp0[maxIndex]) {
    uint32_t bpIdx;
    uint32_t iRght;

    /* Binary Search */
    bpIdx = maxIndex >> 1U;
    iLeft = 0U;
    iRght = maxIndex;
    while (iRght - iLeft > 1U) {
      if (u0 < bp0[bpIdx]) {
        iRght = bpIdx;
      } else {
        iLeft = bpIdx;
      }

      bpIdx = (iRght + iLeft) >> 1U;
    }

    frac = (u0 - bp0[iLeft]) / (bp0[iLeft + 1U] - bp0[iLeft]);
  } else {
    iLeft = maxIndex;
    frac = 0.0F;
  }

  /* Column-major Interpolation 1-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'on'
     Overflow mode: 'portable wrapping'
   */
  if (iLeft == maxIndex) {
    y = table[iLeft];
  } else {
    float yL_0d0;
    yL_0d0 = table[iLeft];
    y = (table[iLeft + 1U] - yL_0d0) * frac + yL_0d0;
  }

  return y;
}

/*
 *         This function updates active task flag for each subrate.
 *         The function is called at model base rate, hence the
 *         generated code self-manages all its subrates.
 */
static void rate_scheduler(void)
{
  /* Compute which subrates run during the next base time step.  Subrates
   * are an integer multiple of the base rate counter.  Therefore, the subtask
   * counter is reset when it reaches its limit (zero means run).
   */
  (driveController_M->Timing.TaskCounters.TID[1])++;
  if ((driveController_M->Timing.TaskCounters.TID[1]) > 4) {/* Sample time: [0.05s, 0.0s] */
    driveController_M->Timing.TaskCounters.TID[1] = 0;
  }
}

/* Model step function */
void driveController_step(void)
{
  double rtb_Filter;
  double rtb_FilterCoefficient;
  double rtb_FilterCoefficient_h;
  double rtb_Integrator;
  double rtb_Saturation;
  double rtb_Sum;
  double tmp;
  float Fz_RL;
  float T_max_L;
  float T_max_R;
  float T_req_eff;
  float rtb_FilterCoefficient_f;
  float rtb_IntegralGain;
  float rtb_Max;
  float rtb_ProportionalGain_e;
  float rtb_Saturation_b;
  float rtb_Sum_l;
  float rtb_T_cut_RL;
  float rtb_T_cut_RR;
  float rtb_TrigonometricFunction;
  float rtb_e_RL_a2;
  int32_t rtb_TVC_Target_Weight;
  bool Compare;
  bool rtb_LogicalOperator;
  bool rtb_enable;

  /* Gain: '<S6>/Gain2' incorporates:
   *  Gain: '<S6>/Gain'
   *  Gain: '<S6>/Gain1'
   *  Inport: '<Root>/n_motorRL'
   */
  rtb_e_RL_a2 = 0.06666667F * driveController_U.n_motorRL * 0.10471976F *
    0.2032F;

  /* MinMax: '<Root>/Max' incorporates:
   *  Constant: '<Root>/Constant2'
   *  Inport: '<Root>/Vx'
   */
  rtb_Max = fmaxf(0.01F, driveController_U.Vx);

  /* MinMax: '<S6>/Max' incorporates:
   *  Constant: '<S6>/v_min [m//s]'
   */
  rtb_Filter = fmax(rtb_Max, 0.1);

  /* Gain: '<S6>/w_car//2' incorporates:
   *  Inport: '<Root>/Yaw_meas'
   */
  T_req_eff = 0.84852815F * driveController_U.Yaw_meas;

  /* Product: '<S6>/Divide' incorporates:
   *  Constant: '<S6>/Constant1'
   *  MinMax: '<S6>/Max1'
   *  Sum: '<S6>/Add'
   *  Sum: '<S6>/Add1'
   */
  rtb_e_RL_a2 = (float)((rtb_e_RL_a2 - (rtb_Filter - T_req_eff)) / fmax
                        (rtb_e_RL_a2, 0.01));

  /* RateLimiter: '<Root>/W_RateLimiter' incorporates:
   *  Constant: '<S6>/Constant2'
   *  Gain: '<S6>/Gain3'
   *  Gain: '<S6>/Gain4'
   *  Gain: '<S6>/Gain5'
   *  Inport: '<Root>/n_motorRR'
   *  MinMax: '<S6>/Max2'
   *  Product: '<S6>/Divide1'
   *  Sum: '<S6>/Add2'
   *  Sum: '<S6>/Add3'
   */
  driveController_Y.TVC_Target_Weight = 0.06666667F *
    driveController_U.n_motorRR * 0.10471976F * 0.2032F;
  driveController_Y.TVC_Target_Weight = (float)
    ((driveController_Y.TVC_Target_Weight - (rtb_Filter + T_req_eff)) / fmax
     (driveController_Y.TVC_Target_Weight, 0.01));

  /* Saturate: '<S6>/Saturation2' incorporates:
   *  Inport: '<Root>/lambda_refRL'
   *  Inport: '<Root>/lambda_refRR'
   */
  if (driveController_U.lambda_refRL > 1.0F) {
    /* Outport: '<Root>/lambda' */
    driveController_Y.lambda[0] = 1.0F;
  } else if (driveController_U.lambda_refRL < 0.0F) {
    /* Outport: '<Root>/lambda' */
    driveController_Y.lambda[0] = 0.0F;
  } else {
    /* Outport: '<Root>/lambda' */
    driveController_Y.lambda[0] = driveController_U.lambda_refRL;
  }

  if (driveController_U.lambda_refRR > 1.0F) {
    /* Outport: '<Root>/lambda' */
    driveController_Y.lambda[1] = 1.0F;
  } else if (driveController_U.lambda_refRR < 0.0F) {
    /* Outport: '<Root>/lambda' */
    driveController_Y.lambda[1] = 0.0F;
  } else {
    /* Outport: '<Root>/lambda' */
    driveController_Y.lambda[1] = driveController_U.lambda_refRR;
  }

  if (rtb_e_RL_a2 > 1.0F) {
    /* Outport: '<Root>/lambda' */
    driveController_Y.lambda[2] = 1.0F;
  } else if (rtb_e_RL_a2 < 0.0F) {
    /* Outport: '<Root>/lambda' */
    driveController_Y.lambda[2] = 0.0F;
  } else {
    /* Outport: '<Root>/lambda' */
    driveController_Y.lambda[2] = rtb_e_RL_a2;
  }

  if (driveController_Y.TVC_Target_Weight > 1.0F) {
    /* Outport: '<Root>/lambda' */
    driveController_Y.lambda[3] = 1.0F;
  } else if (driveController_Y.TVC_Target_Weight < 0.0F) {
    /* Outport: '<Root>/lambda' */
    driveController_Y.lambda[3] = 0.0F;
  } else {
    /* Outport: '<Root>/lambda' */
    driveController_Y.lambda[3] = driveController_Y.TVC_Target_Weight;
  }

  /* End of Saturate: '<S6>/Saturation2' */

  /* RateLimiter: '<Root>/W_RateLimiter' incorporates:
   *  Inport: '<Root>/lambda_refRR'
   *  Sum: '<S6>/Sum1'
   */
  driveController_Y.TVC_Target_Weight -= driveController_U.lambda_refRR;

  /* RelationalOperator: '<S6>/Relational Operator' incorporates:
   *  Constant: '<S6>/Constant'
   */
  rtb_enable = (rtb_Filter >= 1.0);

  /* Logic: '<S6>/Logical Operator' */
  rtb_LogicalOperator = !rtb_enable;

  /* DiscreteIntegrator: '<S101>/Integrator' */
  if (rtb_LogicalOperator || (driveController_DW.Integrator_PrevResetState != 0))
  {
    driveController_DW.Integrator_DSTATE = 0.0;
  }

  /* DiscreteIntegrator: '<S96>/Filter' */
  if (rtb_LogicalOperator || (driveController_DW.Filter_PrevResetState != 0)) {
    driveController_DW.Filter_DSTATE = 0.0;
  }

  /* Gain: '<S104>/Filter Coefficient' incorporates:
   *  DiscreteIntegrator: '<S96>/Filter'
   *  Gain: '<S94>/Derivative Gain'
   *  Sum: '<S96>/SumD'
   */
  rtb_FilterCoefficient = (slip_Kd * driveController_Y.TVC_Target_Weight -
    driveController_DW.Filter_DSTATE) * 15.0;

  /* Sum: '<S110>/Sum' incorporates:
   *  DiscreteIntegrator: '<S101>/Integrator'
   *  Gain: '<S106>/Proportional Gain'
   */
  rtb_Filter = (slip_Kp * driveController_Y.TVC_Target_Weight +
                driveController_DW.Integrator_DSTATE) + rtb_FilterCoefficient;

  /* Saturate: '<S108>/Saturation' */
  if (rtb_Filter > 21.0) {
    rtb_Integrator = 21.0;
  } else if (rtb_Filter < 0.0) {
    rtb_Integrator = 0.0;
  } else {
    rtb_Integrator = rtb_Filter;
  }

  /* End of Saturate: '<S108>/Saturation' */

  /* Gain: '<S98>/Integral Gain' */
  rtb_IntegralGain = slip_Ki * driveController_Y.TVC_Target_Weight;

  /* Saturate: '<S6>/Saturation3' */
  if (rtb_Integrator <= 0.0) {
    tmp = 0.0;
  } else {
    tmp = rtb_Integrator;
  }

  /* MATLAB Function: '<S6>/MATLAB Function1' incorporates:
   *  Saturate: '<S6>/Saturation3'
   */
  rtb_T_cut_RR = (float)((double)rtb_enable * tmp);

  /* Sum: '<S6>/Sum' incorporates:
   *  Inport: '<Root>/lambda_refRL'
   */
  rtb_e_RL_a2 -= driveController_U.lambda_refRL;

  /* DiscreteIntegrator: '<S49>/Integrator' */
  if (rtb_LogicalOperator || (driveController_DW.Integrator_PrevResetState_e !=
       0)) {
    driveController_DW.Integrator_DSTATE_m = 0.0;
  }

  /* DiscreteIntegrator: '<S44>/Filter' */
  if (rtb_LogicalOperator || (driveController_DW.Filter_PrevResetState_f != 0))
  {
    driveController_DW.Filter_DSTATE_b = 0.0;
  }

  /* Gain: '<S52>/Filter Coefficient' incorporates:
   *  DiscreteIntegrator: '<S44>/Filter'
   *  Gain: '<S42>/Derivative Gain'
   *  Sum: '<S44>/SumD'
   */
  rtb_FilterCoefficient_h = (slip_Kd * rtb_e_RL_a2 -
    driveController_DW.Filter_DSTATE_b) * 15.0;

  /* Sum: '<S58>/Sum' incorporates:
   *  DiscreteIntegrator: '<S49>/Integrator'
   *  Gain: '<S54>/Proportional Gain'
   */
  rtb_Sum = (slip_Kp * rtb_e_RL_a2 + driveController_DW.Integrator_DSTATE_m) +
    rtb_FilterCoefficient_h;

  /* Saturate: '<S56>/Saturation' */
  if (rtb_Sum > 21.0) {
    rtb_Saturation = 21.0;
  } else if (rtb_Sum < 0.0) {
    rtb_Saturation = 0.0;
  } else {
    rtb_Saturation = rtb_Sum;
  }

  /* End of Saturate: '<S56>/Saturation' */

  /* Saturate: '<S6>/Saturation1' */
  if (rtb_Saturation <= 0.0) {
    tmp = 0.0;
  } else {
    tmp = rtb_Saturation;
  }

  /* MATLAB Function: '<S6>/MATLAB Function' incorporates:
   *  Saturate: '<S6>/Saturation1'
   */
  rtb_T_cut_RL = (float)((double)rtb_enable * tmp);

  /* Outport: '<Root>/K_slips' */
  driveController_Y.K_slips[0] = rtb_T_cut_RL;
  driveController_Y.K_slips[1] = rtb_T_cut_RR;

  /* Saturate: '<S1>/Saturation1' incorporates:
   *  Inport: '<Root>/Throttle_%'
   */
  if (driveController_U.Throttle_ > 100.0F) {
    /* RateLimiter: '<Root>/W_RateLimiter' */
    driveController_Y.TVC_Target_Weight = 100.0F;
  } else if (driveController_U.Throttle_ < 0.0F) {
    /* RateLimiter: '<Root>/W_RateLimiter' */
    driveController_Y.TVC_Target_Weight = 0.0F;
  } else {
    /* RateLimiter: '<Root>/W_RateLimiter' */
    driveController_Y.TVC_Target_Weight = driveController_U.Throttle_;
  }

  /* End of Saturate: '<S1>/Saturation1' */

  /* Gain: '<S1>/Gain5' incorporates:
   *  Lookup_n-D: '<S1>/1-D Lookup Table'
   *  RateLimiter: '<Root>/W_RateLimiter'
   */
  T_req_eff = 100.0F * look1_iflf_binlcapw(driveController_Y.TVC_Target_Weight,
    driveController_ConstP.uDLookupTable_bp01Data,
    driveController_ConstP.uDLookupTable_tableData, 10U);

  /* RateLimiter: '<S1>/Rate Limiter1' */
  T_max_R = T_req_eff - driveController_DW.PrevY;
  if (T_max_R > 10.0F) {
    T_req_eff = driveController_DW.PrevY + 10.0F;
  } else if (T_max_R < -2.5F) {
    T_req_eff = driveController_DW.PrevY - 2.5F;
  }

  driveController_DW.PrevY = T_req_eff;

  /* End of RateLimiter: '<S1>/Rate Limiter1' */

  /* RateLimiter: '<S1>/Rate Limiter2' */
  T_max_R = driveController_Y.TVC_Target_Weight - driveController_DW.PrevY_j;
  if (T_max_R > 10.0F) {
    /* RateLimiter: '<Root>/W_RateLimiter' */
    driveController_Y.TVC_Target_Weight = driveController_DW.PrevY_j + 10.0F;
  } else if (T_max_R < -2.5F) {
    /* RateLimiter: '<Root>/W_RateLimiter' */
    driveController_Y.TVC_Target_Weight = driveController_DW.PrevY_j - 2.5F;
  }

  driveController_DW.PrevY_j = driveController_Y.TVC_Target_Weight;

  /* End of RateLimiter: '<S1>/Rate Limiter2' */

  /* Switch: '<S1>/Switch1' incorporates:
   *  Inport: '<Root>/Throttle_%'
   */
  if (driveController_U.Throttle_ > 5.0F) {
    /* Switch: '<S1>/Switch' */
    if (rtb_Max > 5.0F) {
      /* RateLimiter: '<Root>/W_RateLimiter' */
      driveController_Y.TVC_Target_Weight = T_req_eff;
    }

    /* Saturate: '<S1>/Saturation' incorporates:
     *  Switch: '<S1>/Switch'
     */
    if (driveController_Y.TVC_Target_Weight > 100.0F) {
      /* Switch: '<S1>/Switch1' */
      driveController_Y.Throttle_real = 100.0;
    } else if (driveController_Y.TVC_Target_Weight < 0.0F) {
      /* Switch: '<S1>/Switch1' */
      driveController_Y.Throttle_real = 0.0;
    } else {
      /* Switch: '<S1>/Switch1' */
      driveController_Y.Throttle_real = driveController_Y.TVC_Target_Weight;
    }

    /* End of Saturate: '<S1>/Saturation' */
  } else {
    /* Switch: '<S1>/Switch1' incorporates:
     *  Constant: '<S1>/Constant'
     */
    driveController_Y.Throttle_real = 0.0;
  }

  /* End of Switch: '<S1>/Switch1' */

  /* Switch: '<S4>/Switch' incorporates:
   *  Inport: '<Root>/Derating_RL'
   *  Inport: '<Root>/Derating_RR'
   *  Logic: '<S4>/Logical Operator'
   */
  if ((driveController_U.Derating_RL != 0.0F) || (driveController_U.Derating_RR
       != 0.0F)) {
    /* RateLimiter: '<Root>/W_RateLimiter' incorporates:
     *  Constant: '<S4>/Constant1'
     */
    driveController_Y.TVC_Target_Weight = T_rated;
  } else {
    /* RateLimiter: '<Root>/W_RateLimiter' incorporates:
     *  Constant: '<S4>/Constant'
     */
    driveController_Y.TVC_Target_Weight = 21.0F;
  }

  /* End of Switch: '<S4>/Switch' */

  /* RateLimiter: '<S4>/Rate Limiter' */
  T_max_R = driveController_Y.TVC_Target_Weight - driveController_DW.PrevY_g;
  if (T_max_R > 2.5F) {
    /* RateLimiter: '<Root>/W_RateLimiter' */
    driveController_Y.TVC_Target_Weight = driveController_DW.PrevY_g + 2.5F;
  } else if (T_max_R < -2.5F) {
    /* RateLimiter: '<Root>/W_RateLimiter' */
    driveController_Y.TVC_Target_Weight = driveController_DW.PrevY_g - 2.5F;
  }

  driveController_DW.PrevY_g = driveController_Y.TVC_Target_Weight;

  /* End of RateLimiter: '<S4>/Rate Limiter' */

  /* MinMax: '<S4>/Min' incorporates:
   *  Gain: '<S4>/Gain'
   */
  T_req_eff = (float)fmin(0.21 * driveController_Y.Throttle_real,
    driveController_Y.TVC_Target_Weight);

  /* RateLimiter: '<S4>/Rate Limiter1' */
  T_max_R = T_req_eff - driveController_DW.PrevY_a;
  if (T_max_R > 2.5F) {
    /* RateLimiter: '<S4>/Rate Limiter1' */
    T_req_eff = driveController_DW.PrevY_a + 2.5F;
  } else if (T_max_R < -0.099999994F) {
    /* RateLimiter: '<S4>/Rate Limiter1' */
    T_req_eff = driveController_DW.PrevY_a - 0.099999994F;
  }

  driveController_DW.PrevY_a = T_req_eff;

  /* End of RateLimiter: '<S4>/Rate Limiter1' */

  /* Trigonometry: '<S10>/Trigonometric Function' incorporates:
   *  Inport: '<Root>/D'
   */
  rtb_TrigonometricFunction = tanf(driveController_U.Drad);

  /* MATLAB Function: '<S10>/tand safe' */
  if (fabsf(rtb_TrigonometricFunction) < 0.017455064928217585) {
    rtb_TrigonometricFunction = 0.0F;
  }

  /* MATLAB Function: '<S10>/Bicycle Model Dynamic' incorporates:
   *  MATLAB Function: '<Root>/MATLAB Function1'
   *  MATLAB Function: '<S10>/tand safe'
   */
  Fz_RL = rtb_Max * rtb_Max;
  rtb_TrigonometricFunction = rtb_Max / 1.55F * rtb_TrigonometricFunction /
    (Fz_RL * 0.0F + 1.0F);

  /* RateLimiter: '<S10>/Rate Lmiter' */
  T_max_R = rtb_TrigonometricFunction - driveController_DW.PrevY_o;
  if (T_max_R > driveController_period) {
    rtb_TrigonometricFunction = driveController_DW.PrevY_o +
      driveController_period;
  } else if (T_max_R < -0.01F) {
    rtb_TrigonometricFunction = driveController_DW.PrevY_o - 0.01F;
  }

  driveController_DW.PrevY_o = rtb_TrigonometricFunction;

  /* End of RateLimiter: '<S10>/Rate Lmiter' */

  /* MATLAB Function: '<S10>/yaw_saturation' */
  if (rtIsNaNF(rtb_TrigonometricFunction)) {
    T_max_R = (rtNaNF);
  } else if (rtb_TrigonometricFunction < 0.0F) {
    T_max_R = -1.0F;
  } else {
    T_max_R = (float)(rtb_TrigonometricFunction > 0.0F);
  }

  driveController_Y.yaw_th = fminf(fabsf(rtb_TrigonometricFunction), 14.715F /
    fmaxf(rtb_Max, 0.5F)) * T_max_R;

  /* End of MATLAB Function: '<S10>/yaw_saturation' */

  /* Chart: '<Root>/Mode Manager' incorporates:
   *  Inport: '<Root>/brake'
   */
  if (driveController_DW.is_active_c5_driveController == 0) {
    driveController_DW.is_active_c5_driveController = 1U;
    driveController_DW.is_c5_driveController = driveController_IN_IDLE_;
    rtb_TVC_Target_Weight = 0;
  } else if (driveController_DW.is_c5_driveController ==
             driveController_IN_IDLE_) {
    rtb_TVC_Target_Weight = 0;
    if ((rtb_Max > tvc_V_on) && (driveController_Y.Throttle_real >
         tvc_throttle_on) && (driveController_U.brake == 0.0F)) {
      driveController_DW.is_c5_driveController = driveController_IN_TVC_;
      rtb_TVC_Target_Weight = 1;
    }
  } else {
    /* case IN_TVC_: */
    rtb_TVC_Target_Weight = 1;
    if ((driveController_U.brake > 0.0F) || ((driveController_Y.Throttle_real <
          tvc_throttle_on) && (rtb_Max <= tvc_V_off))) {
      driveController_DW.is_c5_driveController = driveController_IN_IDLE_;
      rtb_TVC_Target_Weight = 0;
    }
  }

  /* End of Chart: '<Root>/Mode Manager' */

  /* RateLimiter: '<Root>/W_RateLimiter' */
  T_max_R = (float)rtb_TVC_Target_Weight - driveController_DW.PrevY_i;
  if (T_max_R > 0.02F) {
    /* RateLimiter: '<Root>/W_RateLimiter' */
    driveController_Y.TVC_Target_Weight = driveController_DW.PrevY_i + 0.02F;
  } else if (T_max_R < -0.02F) {
    /* RateLimiter: '<Root>/W_RateLimiter' */
    driveController_Y.TVC_Target_Weight = driveController_DW.PrevY_i - 0.02F;
  } else {
    /* RateLimiter: '<Root>/W_RateLimiter' */
    driveController_Y.TVC_Target_Weight = (float)rtb_TVC_Target_Weight;
  }

  driveController_DW.PrevY_i = driveController_Y.TVC_Target_Weight;

  /* End of RateLimiter: '<Root>/W_RateLimiter' */

  /* Product: '<S9>/e_yaw * TVC_weight' incorporates:
   *  Inport: '<Root>/Yaw_meas'
   *  Sum: '<Root>/Sum'
   */
  rtb_TrigonometricFunction = (driveController_Y.yaw_th -
    driveController_U.Yaw_meas) * driveController_Y.TVC_Target_Weight;
  rtb_enable = (driveController_M->Timing.TaskCounters.TID[1] == 0);

  /* RelationalOperator: '<S119>/Compare' incorporates:
   *  Constant: '<S119>/Constant'
   */
  Compare = (driveController_Y.TVC_Target_Weight <= 0.05F);
  if (rtb_enable) {
    /* DiscreteIntegrator: '<S158>/Integrator' incorporates:
     *  Memory: '<S9>/Memory'
     */
    if (driveController_DW.Integrator_IC_LOADING != 0) {
      driveController_DW.Integrator_DSTATE_n =
        driveController_DW.Memory_PreviousInput;
    }

    if (Compare || (driveController_DW.Integrator_PrevResetState_f != 0)) {
      driveController_DW.Integrator_DSTATE_n =
        driveController_DW.Memory_PreviousInput;
    }

    /* DiscreteIntegrator: '<S153>/Filter' */
    if (Compare || (driveController_DW.Filter_PrevResetState_a != 0)) {
      driveController_DW.Filter_DSTATE_n = driveController_ConstB.Constant;
    }

    /* Gain: '<S161>/Filter Coefficient' incorporates:
     *  DiscreteIntegrator: '<S153>/Filter'
     *  Gain: '<S151>/Derivative Gain'
     *  Sum: '<S153>/SumD'
     */
    rtb_FilterCoefficient_f = (tvc_Kd * rtb_TrigonometricFunction -
      driveController_DW.Filter_DSTATE_n) * tvc_N_filter;

    /* Sum: '<S167>/Sum' incorporates:
     *  DiscreteIntegrator: '<S158>/Integrator'
     *  Gain: '<S163>/Proportional Gain'
     */
    rtb_Sum_l = (tvc_Kp * rtb_TrigonometricFunction +
                 driveController_DW.Integrator_DSTATE_n) +
      rtb_FilterCoefficient_f;

    /* Saturate: '<S165>/Saturation' */
    if (rtb_Sum_l > tvc_up_sat) {
      rtb_Saturation_b = tvc_up_sat;
    } else if (rtb_Sum_l < tvc_low_sat) {
      rtb_Saturation_b = tvc_low_sat;
    } else {
      rtb_Saturation_b = rtb_Sum_l;
    }

    /* End of Saturate: '<S165>/Saturation' */

    /* MATLAB Function: '<S9>/MATLAB Function' */
    rtb_ProportionalGain_e = rtb_Saturation_b * 0.2032F / 20.364676F;

    /* MATLAB Function: '<S9>/TorqueAllocator' incorporates:
     *  Inport: '<Root>/D'
     *  Inport: '<Root>/Derating_RL'
     *  Inport: '<Root>/Derating_RR'
     */
    T_max_L = (1.0F - driveController_U.Derating_RL) * 21.0F +
      driveController_U.Derating_RL * T_rated;
    T_max_R = (1.0F - driveController_U.Derating_RR) * 21.0F +
      driveController_U.Derating_RR * T_rated;
    T_req_eff = fmaxf(0.0F, fminf(T_req_eff, fminf(T_max_L, T_max_R) - fminf
      (T_headroom_max, T_headroom_k * fabsf(driveController_U.Drad))));
    driveController_B.T_RL_cmd = fmaxf(0.0F, fminf(T_max_L, T_req_eff -
      rtb_ProportionalGain_e));
    driveController_B.T_RR_cmd = fmaxf(0.0F, fminf(T_max_R, T_req_eff +
      rtb_ProportionalGain_e));
  }

  /* Sum: '<Root>/Add' */
  T_req_eff = driveController_B.T_RL_cmd - rtb_T_cut_RL;

  /* RateLimiter: '<Root>/Rate Limiter1' */
  T_max_R = T_req_eff - driveController_DW.PrevY_f;
  if (T_max_R > 1.0F) {
    /* RateLimiter: '<Root>/Rate Limiter1' */
    driveController_DW.PrevY_f++;
  } else if (T_max_R < -5.0F) {
    /* RateLimiter: '<Root>/Rate Limiter1' */
    driveController_DW.PrevY_f -= 5.0F;
  } else {
    /* RateLimiter: '<Root>/Rate Limiter1' */
    driveController_DW.PrevY_f = T_req_eff;
  }

  /* End of RateLimiter: '<Root>/Rate Limiter1' */

  /* DiscreteTransferFcn: '<Root>/Discrete Transfer Fcn1' incorporates:
   *  Inport: '<Root>/ay'
   */
  rtb_T_cut_RL = driveController_U.ay - -0.9047619F *
    driveController_DW.DiscreteTransferFcn1_states;
  T_req_eff = 0.04761905F * rtb_T_cut_RL + 0.04761905F *
    driveController_DW.DiscreteTransferFcn1_states;

  /* DiscreteTransferFcn: '<Root>/Discrete Transfer Fcn' incorporates:
   *  Inport: '<Root>/ax'
   */
  rtb_ProportionalGain_e = driveController_U.ax - -0.9047619F *
    driveController_DW.DiscreteTransferFcn_states;

  /* MATLAB Function: '<Root>/MATLAB Function1' incorporates:
   *  DiscreteTransferFcn: '<Root>/Discrete Transfer Fcn'
   *  DiscreteTransferFcn: '<Root>/Discrete Transfer Fcn1'
   */
  T_max_R = 300.0F * T_req_eff * 0.35F / 1.2F * 0.5F;
  T_max_L = (0.04761905F * rtb_ProportionalGain_e + 0.04761905F *
             driveController_DW.DiscreteTransferFcn_states) * 300.0F * 0.35F /
    1.55F * 0.5F + (Fz_RL * 0.67375F * 0.5F + 1471.5F) / 2.0F;
  Fz_RL = T_max_L + T_max_R;
  T_max_R = T_max_L - T_max_R;
  if (Fz_RL < 0.0F) {
    Fz_RL = 0.0F;
  }

  if (T_max_R < 0.0F) {
    T_max_R = 0.0F;
  }

  driveController_Y.T_RL = Fz_RL * 1.5F * 0.2032F / 12.0F * 1.13F;
  Fz_RL = T_max_R * 1.5F * 0.2032F / 12.0F * 1.13F;

  /* MATLAB Function: '<Root>/MATLAB Function' incorporates:
   *  DiscreteTransferFcn: '<Root>/Discrete Transfer Fcn1'
   *  Inport: '<Root>/n_motorRL'
   *  Inport: '<Root>/n_motorRR'
   */
  T_max_R = (fabsf(driveController_U.n_motorRL) + fabsf
             (driveController_U.n_motorRR)) * 0.5F;
  if (!(driveController_Y.Throttle_real <= 10.0) || !(T_max_R > 500.0F) ||
      !(rtb_Max > 0.5F)) {
    driveController_DW.t_elapsed_s = 0.0;
    T_req_eff = 0.0F;
    rtb_Max = 0.0F;
    rtb_TVC_Target_Weight = 0;
  } else {
    driveController_DW.t_elapsed_s += 0.01;
    rtb_Max = 6.2831855F * T_max_R / 60.0F;
    if (T_max_R >= 3000.0F) {
      T_max_R = 1.0F;
    } else {
      T_max_R /= 3000.0F;
    }

    if (driveController_DW.t_elapsed_s < 4.0) {
      tmp = 10.5;
    } else {
      tmp = 7.0;
    }

    T_req_eff = fmaxf(fmaxf((float)(1.0 - driveController_Y.Throttle_real / 10.0)
      * -regen_T_max * T_max_R, -(17500.0F / rtb_Max)), -((float)(tmp * 540.0) /
      rtb_Max)) * fmaxf(0.0F, 1.0F - fabsf(T_req_eff) / 15.0F);
    rtb_Max = T_req_eff;
    rtb_TVC_Target_Weight = 1;
  }

  /* End of MATLAB Function: '<Root>/MATLAB Function' */

  /* RateLimiter: '<Root>/Rate Limiter3' */
  T_max_R = T_req_eff - driveController_DW.PrevY_e;
  if (T_max_R > 0.59999996F) {
    T_req_eff = driveController_DW.PrevY_e + 0.59999996F;
  } else if (T_max_R < -0.099999994F) {
    T_req_eff = driveController_DW.PrevY_e - 0.099999994F;
  }

  driveController_DW.PrevY_e = T_req_eff;

  /* End of RateLimiter: '<Root>/Rate Limiter3' */

  /* Switch: '<Root>/Switch1' incorporates:
   *  RelationalOperator: '<S7>/LowerRelop1'
   *  Switch: '<S7>/Switch2'
   */
  if (rtb_TVC_Target_Weight > 0.5F) {
    /* Outport: '<Root>/T_RL' */
    driveController_Y.T_RL = T_req_eff;
  } else if (!(driveController_DW.PrevY_f > driveController_Y.T_RL)) {
    /* Switch: '<S7>/Switch' incorporates:
     *  Constant: '<Root>/Constant'
     *  RelationalOperator: '<S7>/UpperRelop'
     *  Switch: '<S7>/Switch2'
     */
    if (driveController_DW.PrevY_f < 0.0F) {
      /* Outport: '<Root>/T_RL' */
      driveController_Y.T_RL = 0.0F;
    } else {
      /* Outport: '<Root>/T_RL' */
      driveController_Y.T_RL = driveController_DW.PrevY_f;
    }

    /* End of Switch: '<S7>/Switch' */
  }

  /* End of Switch: '<Root>/Switch1' */

  /* RateLimiter: '<Root>/Rate Limiter4' */
  T_max_R = rtb_Max - driveController_DW.PrevY_oa;
  if (T_max_R > 0.59999996F) {
    T_req_eff = driveController_DW.PrevY_oa + 0.59999996F;
  } else if (T_max_R < -0.099999994F) {
    T_req_eff = driveController_DW.PrevY_oa - 0.099999994F;
  } else {
    T_req_eff = rtb_Max;
  }

  driveController_DW.PrevY_oa = T_req_eff;

  /* End of RateLimiter: '<Root>/Rate Limiter4' */

  /* Sum: '<Root>/Add1' */
  driveController_Y.T_RR = driveController_B.T_RR_cmd - rtb_T_cut_RR;

  /* RateLimiter: '<Root>/Rate Limiter2' */
  T_max_R = driveController_Y.T_RR - driveController_DW.PrevY_ag;
  if (T_max_R > 1.0F) {
    /* Sum: '<Root>/Add1' */
    driveController_Y.T_RR = driveController_DW.PrevY_ag + 1.0F;
  } else if (T_max_R < -5.0F) {
    /* Sum: '<Root>/Add1' */
    driveController_Y.T_RR = driveController_DW.PrevY_ag - 5.0F;
  }

  driveController_DW.PrevY_ag = driveController_Y.T_RR;

  /* End of RateLimiter: '<Root>/Rate Limiter2' */

  /* Switch: '<Root>/Switch2' incorporates:
   *  Constant: '<Root>/Constant1'
   *  RelationalOperator: '<S8>/LowerRelop1'
   *  RelationalOperator: '<S8>/UpperRelop'
   *  Switch: '<S8>/Switch'
   *  Switch: '<S8>/Switch2'
   */
  if (rtb_TVC_Target_Weight > 0.5F) {
    /* Sum: '<Root>/Add1' incorporates:
     *  Outport: '<Root>/T_RR'
     */
    driveController_Y.T_RR = T_req_eff;
  } else if (driveController_Y.T_RR > Fz_RL) {
    /* Sum: '<Root>/Add1' incorporates:
     *  Outport: '<Root>/T_RR'
     *  Switch: '<S8>/Switch2'
     */
    driveController_Y.T_RR = Fz_RL;
  } else if (driveController_Y.T_RR < 0.0F) {
    /* Sum: '<Root>/Add1' incorporates:
     *  Constant: '<Root>/Constant1'
     *  Outport: '<Root>/T_RR'
     *  Switch: '<S8>/Switch'
     *  Switch: '<S8>/Switch2'
     */
    driveController_Y.T_RR = 0.0F;
  }

  /* End of Switch: '<Root>/Switch2' */

  /* Outport: '<Root>/regen' */
  driveController_Y.regen = (float)rtb_TVC_Target_Weight;

  /* MATLAB Function: '<S9>/MATLAB Function1' incorporates:
   *  Inport: '<Root>/T_RL_act'
   *  Inport: '<Root>/T_RR_act'
   *  Sum: '<S9>/Sum3'
   */
  driveController_DW.Memory_PreviousInput = (driveController_U.T_RL_act -
    driveController_U.T_RR_act) * 1.6970563F * 15.0F * 0.8F / 0.4064F;
  if (rtb_enable) {
    /* Update for DiscreteIntegrator: '<S158>/Integrator' incorporates:
     *  Gain: '<S155>/Integral Gain'
     *  Gain: '<S169>/Kt'
     *  Sum: '<S150>/SumI2'
     *  Sum: '<S150>/SumI4'
     *  Sum: '<S169>/SumI3'
     *  Sum: '<S170>/SumI1'
     */
    driveController_DW.Integrator_IC_LOADING = 0U;
    driveController_DW.Integrator_DSTATE_n +=
      (((driveController_DW.Memory_PreviousInput - rtb_Saturation_b) * tvc_tr +
        tvc_Ki * rtb_TrigonometricFunction) + (rtb_Saturation_b - rtb_Sum_l)) *
      0.05F;
    driveController_DW.Integrator_PrevResetState_f = (int8_t)Compare;

    /* Update for DiscreteIntegrator: '<S153>/Filter' incorporates:
     *  DiscreteIntegrator: '<S158>/Integrator'
     */
    driveController_DW.Filter_DSTATE_n += 0.05F * rtb_FilterCoefficient_f;
    driveController_DW.Filter_PrevResetState_a = (int8_t)Compare;
  }

  /* Update for DiscreteIntegrator: '<S101>/Integrator' incorporates:
   *  Gain: '<S93>/Kb'
   *  Sum: '<S93>/SumI2'
   *  Sum: '<S93>/SumI4'
   */
  driveController_DW.Integrator_DSTATE += ((rtb_Integrator - rtb_Filter) *
    slip_bc_coeff + rtb_IntegralGain) * 0.01;
  driveController_DW.Integrator_PrevResetState = (int8_t)rtb_LogicalOperator;

  /* Update for DiscreteIntegrator: '<S96>/Filter' incorporates:
   *  DiscreteIntegrator: '<S101>/Integrator'
   */
  driveController_DW.Filter_DSTATE += 0.01 * rtb_FilterCoefficient;
  driveController_DW.Filter_PrevResetState = (int8_t)rtb_LogicalOperator;

  /* Update for DiscreteIntegrator: '<S49>/Integrator' incorporates:
   *  DiscreteIntegrator: '<S101>/Integrator'
   *  Gain: '<S41>/Kb'
   *  Gain: '<S46>/Integral Gain'
   *  Sum: '<S41>/SumI2'
   *  Sum: '<S41>/SumI4'
   */
  driveController_DW.Integrator_DSTATE_m += ((rtb_Saturation - rtb_Sum) *
    slip_bc_coeff + slip_Ki * rtb_e_RL_a2) * 0.01;
  driveController_DW.Integrator_PrevResetState_e = (int8_t)rtb_LogicalOperator;

  /* Update for DiscreteIntegrator: '<S44>/Filter' incorporates:
   *  DiscreteIntegrator: '<S101>/Integrator'
   */
  driveController_DW.Filter_DSTATE_b += 0.01 * rtb_FilterCoefficient_h;
  driveController_DW.Filter_PrevResetState_f = (int8_t)rtb_LogicalOperator;

  /* Update for DiscreteTransferFcn: '<Root>/Discrete Transfer Fcn1' */
  driveController_DW.DiscreteTransferFcn1_states = rtb_T_cut_RL;

  /* Update for DiscreteTransferFcn: '<Root>/Discrete Transfer Fcn' */
  driveController_DW.DiscreteTransferFcn_states = rtb_ProportionalGain_e;
  rate_scheduler();
}

/* Model initialize function */
void driveController_initialize(void)
{
  /* InitializeConditions for DiscreteIntegrator: '<S158>/Integrator' */
  driveController_DW.Integrator_IC_LOADING = 1U;

  /* InitializeConditions for DiscreteIntegrator: '<S153>/Filter' */
  driveController_DW.Filter_DSTATE_n = driveController_ConstB.Constant;
}

/* Model terminate function */
void driveController_terminate(void)
{
  /* (no terminate code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
