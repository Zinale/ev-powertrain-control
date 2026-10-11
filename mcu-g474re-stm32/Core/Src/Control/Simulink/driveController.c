/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: driveController.c
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

#include "driveController.h"
#include <stdint.h>
#include <math.h>
#include "rt_nonfinite.h"
#include <stdbool.h>
#include "driveController_private.h"

/* Named constants for Chart: '<Root>/Mode Manager' */
#define driveController_IN_IDLE_       ((uint8_t)1U)
#define driveController_IN_TVC_        ((uint8_t)2U)

/* Exported block parameters */
float I_charge_max_p = 21.0F;          /* Variable: I_charge_max_p
                                        * Referenced by:
                                        *   '<Root>/Regen MATLAB Function'
                                        *   '<S8>/TorqueAllocator'
                                        */
float I_discharge_max_p = 100.0F;      /* Variable: I_discharge_max_p
                                        * Referenced by: '<S8>/TorqueAllocator'
                                        */
float Kus_p = 0.0F;                    /* Variable: Kus_p
                                        * Referenced by: '<S9>/Bicycle Model Dynamic'
                                        */
float T_peak_p = 21.0F;                /* Variable: T_peak_p
                                        * Referenced by:
                                        *   '<Root>/Saturation'
                                        *   '<Root>/Saturation1'
                                        *   '<S8>/TorqueAllocator'
                                        */
float V_dc_p = 540.0F;                 /* Variable: V_dc_p
                                        * Referenced by:
                                        *   '<Root>/Regen MATLAB Function'
                                        *   '<S8>/TorqueAllocator'
                                        */
float beta_dot_threshold_p = 0.3F;     /* Variable: beta_dot_threshold_p
                                        * Referenced by: '<Root>/Grip MATLAB Estimator'
                                        */
float mu_p = 1.14F;                    /* Variable: mu_p
                                        * Referenced by: '<Root>/Grip MATLAB Estimator'
                                        */
float overslip_factor_p = 1.01F;       /* Variable: overslip_factor_p
                                        * Referenced by: '<Root>/Grip MATLAB Estimator'
                                        */
float regen_T_max_p = 9.0F;            /* Variable: regen_T_max_p
                                        * Referenced by:
                                        *   '<Root>/Regen MATLAB Function'
                                        *   '<Root>/Saturation'
                                        *   '<Root>/Saturation1'
                                        *   '<S8>/TorqueAllocator'
                                        */
float regen_pedal_hyst_p = 3.0F;       /* Variable: regen_pedal_hyst_p
                                        * Referenced by: '<Root>/Regen MATLAB Function'
                                        */
float regen_pedal_thr_p = 4.0F;        /* Variable: regen_pedal_thr_p
                                        * Referenced by: '<Root>/Regen MATLAB Function'
                                        */
float slip_Kd_p = 1.5F;                /* Variable: slip_Kd_p
                                        * Referenced by:
                                        *   '<S45>/Derivative Gain'
                                        *   '<S97>/Derivative Gain'
                                        */
float slip_Ki_p = 22.0F;               /* Variable: slip_Ki_p
                                        * Referenced by:
                                        *   '<S49>/Integral Gain'
                                        *   '<S101>/Integral Gain'
                                        */
float slip_Kp_p = 15.0F;               /* Variable: slip_Kp_p
                                        * Referenced by:
                                        *   '<S57>/Proportional Gain'
                                        *   '<S109>/Proportional Gain'
                                        */
float tvc_Kd_p = 50.0F;                /* Variable: tvc_Kd_p
                                        * Referenced by: '<S153>/Derivative Gain'
                                        */
float tvc_Ki_p = 110.0F;               /* Variable: tvc_Ki_p
                                        * Referenced by: '<S157>/Integral Gain'
                                        */
float tvc_Kp_p = 400.0F;               /* Variable: tvc_Kp_p
                                        * Referenced by: '<S165>/Proportional Gain'
                                        */
uint8_t pedal_map_select_p = 1U;       /* Variable: pedal_map_select_p
                                        * Referenced by: '<S3>/Constant1'
                                        */

/* Block states (default storage) */
DW_driveController_T driveController_DW;

/* External inputs (root inport signals with default storage) */
ExtU_driveController_T driveController_U;

/* External outputs (root outports fed by signals with default storage) */
ExtY_driveController_T driveController_Y;

/* Real-time model */
static RT_MODEL_driveController_T driveController_M_;
RT_MODEL_driveController_T *const driveController_M = &driveController_M_;

/* Forward declaration for local functions */
static float driveController_minimum(const float x[3]);
static void rate_scheduler(void);
double look1_binlcapw(double u0, const double bp0[], const double table[],
                      uint32_t maxIndex)
{
  double frac;
  double y;
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
    frac = 0.0;
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
    frac = 0.0;
  }

  /* Column-major Interpolation 1-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'on'
     Overflow mode: 'portable wrapping'
   */
  if (iLeft == maxIndex) {
    y = table[iLeft];
  } else {
    double yL_0d0;
    yL_0d0 = table[iLeft];
    y = (table[iLeft + 1U] - yL_0d0) * frac + yL_0d0;
  }

  return y;
}

double look1_binlxpw(double u0, const double bp0[], const double table[],
                     uint32_t maxIndex)
{
  double frac;
  double yL_0d0;
  uint32_t iLeft;

  /* Column-major Lookup 1-D
     Search method: 'binary'
     Use previous index: 'off'
     Interpolation method: 'Linear point-slope'
     Extrapolation method: 'Linear'
     Use last breakpoint for index at or above upper limit: 'off'
     Remove protection against out-of-range input in generated code: 'off'
   */
  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Linear'
     Use previous index: 'off'
     Use last breakpoint for index at or above upper limit: 'off'
     Remove protection against out-of-range input in generated code: 'off'
   */
  if (u0 <= bp0[0U]) {
    iLeft = 0U;
    frac = (u0 - bp0[0U]) / (bp0[1U] - bp0[0U]);
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
    iLeft = maxIndex - 1U;
    frac = (u0 - bp0[maxIndex - 1U]) / (bp0[maxIndex] - bp0[maxIndex - 1U]);
  }

  /* Column-major Interpolation 1-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'off'
     Overflow mode: 'portable wrapping'
   */
  yL_0d0 = table[iLeft];
  return (table[iLeft + 1U] - yL_0d0) * frac + yL_0d0;
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

/* Function for MATLAB Function: '<S8>/TorqueAllocator' */
static float driveController_minimum(const float x[3])
{
  float ex;
  int32_t idx;
  int32_t k;
  if (!rtIsNaNF(x[0])) {
    idx = 1;
  } else {
    bool exitg1;
    idx = 0;
    k = 2;
    exitg1 = false;
    while (!exitg1 && (k < 4)) {
      if (!rtIsNaNF(x[k - 1])) {
        idx = k;
        exitg1 = true;
      } else {
        k++;
      }
    }
  }

  if (idx == 0) {
    ex = x[0];
  } else {
    ex = x[idx - 1];
    for (k = idx + 1; k < 4; k++) {
      float x_0;
      x_0 = x[k - 1];
      if (ex > x_0) {
        ex = x_0;
      }
    }
  }

  return ex;
}

/* Model step function */
void driveController_step(void)
{
  double Sum1;
  double dT_real;
  double rtb_D_rad;
  double rtb_FilterCoefficient_k;
  double rtb_Filter_d;
  double rtb_Filter_l;
  double rtb_IntegralGain;
  double rtb_Integrator;
  double rtb_Integrator_lq;
  double rtb_Max;
  double rtb_Saturation;
  double rtb_Saturation_o;
  double rtb_Sum;
  double rtb_Sum_a;
  double rtb_T_cut_RL;
  double rtb_T_cut_RR;
  double rtb_e_RL_i;
  double rtb_k_slip;
  double w_RL;
  double w_RR;
  float tmp[3];
  float Fx_total_potential_RL;
  float Fy_est_RL;
  float Fy_est_RR;
  float Fz_RL;
  float Fz_RR;
  float delta_Fz_lat;
  float dynamic_reserve;
  int32_t rtb_TVC_Target_Weight;
  int32_t rtb_regen_active;
  bool rtb_Compare_a;
  bool rtb_LogicalOperator1;
  bool rtb_LogicalOperator2;
  bool rtb_enable;

  /* Gain: '<S7>/Gain2' incorporates:
   *  Gain: '<S7>/Gain'
   *  Gain: '<S7>/Gain1'
   *  Inport: '<Root>/n_motorRL'
   */
  rtb_e_RL_i = 0.06666666666666667 * driveController_U.n_motorRL *
    0.10471975511965977 * 0.2032;

  /* Saturate: '<S7>/Saturation4' */
  if (rtb_e_RL_i <= 0.0) {
    rtb_e_RL_i = 0.0;
  }

  /* End of Saturate: '<S7>/Saturation4' */

  /* MinMax: '<Root>/Max' incorporates:
   *  Constant: '<Root>/Constant2'
   *  Inport: '<Root>/Vx'
   */
  rtb_Max = fmax(0.01, driveController_U.Vx);

  /* MinMax: '<S7>/Max' incorporates:
   *  Constant: '<S7>/v_min_safe [m//s]'
   */
  rtb_Integrator = fmax(rtb_Max, 0.1);

  /* Gain: '<S7>/w_car//2' incorporates:
   *  Inport: '<Root>/Yaw_rate_act'
   */
  dynamic_reserve = 1.5F * driveController_U.yaw_meas;

  /* Product: '<S7>/Divide' incorporates:
   *  Constant: '<S7>/Constant1'
   *  MinMax: '<S7>/Max1'
   *  Sum: '<S7>/Add'
   *  Sum: '<S7>/Add1'
   */
  rtb_e_RL_i = (rtb_e_RL_i - (rtb_Integrator - dynamic_reserve)) / fmax
    (rtb_e_RL_i, 0.1);

  /* Gain: '<S7>/Gain5' incorporates:
   *  Gain: '<S7>/Gain3'
   *  Gain: '<S7>/Gain4'
   *  Inport: '<Root>/n_motorRR'
   */
  rtb_Sum_a = 0.06666666666666667 * driveController_U.n_motorRR *
    0.10471975511965977 * 0.2032;

  /* Saturate: '<S7>/Saturation5' */
  if (rtb_Sum_a <= 0.0) {
    rtb_Sum_a = 0.0;
  }

  /* End of Saturate: '<S7>/Saturation5' */

  /* Product: '<S7>/Divide1' incorporates:
   *  Constant: '<S7>/Constant2'
   *  MinMax: '<S7>/Max2'
   *  Sum: '<S7>/Add2'
   *  Sum: '<S7>/Add3'
   */
  rtb_Sum_a = (rtb_Sum_a - (rtb_Integrator + dynamic_reserve)) / fmax(rtb_Sum_a,
    0.1);

  /* Saturate: '<S7>/Saturation2' incorporates:
   *  Inport: '<Root>/lambda_refRL'
   *  Inport: '<Root>/lambda_refRR'
   */
  if (driveController_U.lambda_refRL > 1.0) {
    /* Outport: '<Root>/lambda' */
    driveController_Y.lambda[0] = 1.0;
  } else if (driveController_U.lambda_refRL < 0.0) {
    /* Outport: '<Root>/lambda' */
    driveController_Y.lambda[0] = 0.0;
  } else {
    /* Outport: '<Root>/lambda' */
    driveController_Y.lambda[0] = driveController_U.lambda_refRL;
  }

  if (driveController_U.lambda_refRR > 1.0) {
    /* Outport: '<Root>/lambda' */
    driveController_Y.lambda[1] = 1.0;
  } else if (driveController_U.lambda_refRR < 0.0) {
    /* Outport: '<Root>/lambda' */
    driveController_Y.lambda[1] = 0.0;
  } else {
    /* Outport: '<Root>/lambda' */
    driveController_Y.lambda[1] = driveController_U.lambda_refRR;
  }

  if (rtb_e_RL_i > 1.0) {
    /* Outport: '<Root>/lambda' */
    driveController_Y.lambda[2] = 1.0;
  } else if (rtb_e_RL_i < 0.0) {
    /* Outport: '<Root>/lambda' */
    driveController_Y.lambda[2] = 0.0;
  } else {
    /* Outport: '<Root>/lambda' */
    driveController_Y.lambda[2] = rtb_e_RL_i;
  }

  if (rtb_Sum_a > 1.0) {
    /* Outport: '<Root>/lambda' */
    driveController_Y.lambda[3] = 1.0;
  } else if (rtb_Sum_a < 0.0) {
    /* Outport: '<Root>/lambda' */
    driveController_Y.lambda[3] = 0.0;
  } else {
    /* Outport: '<Root>/lambda' */
    driveController_Y.lambda[3] = rtb_Sum_a;
  }

  /* End of Saturate: '<S7>/Saturation2' */

  /* Sum: '<S7>/Sum1' incorporates:
   *  Inport: '<Root>/lambda_refRR'
   */
  rtb_Sum_a -= driveController_U.lambda_refRR;

  /* RelationalOperator: '<S7>/Relational Operator' incorporates:
   *  Constant: '<S7>/Constant'
   */
  rtb_enable = (rtb_Integrator >= 3.0);

  /* Logic: '<S7>/Logical Operator2' incorporates:
   *  Constant: '<S11>/Constant'
   *  Constant: '<S13>/Constant'
   *  Inport: '<Root>/brake_%'
   *  Logic: '<S7>/Logical Operator'
   *  Logic: '<S7>/Logical Operator1'
   *  RelationalOperator: '<S11>/Compare'
   *  RelationalOperator: '<S13>/Compare'
   */
  rtb_LogicalOperator1 = (!rtb_enable || (driveController_U.brake_ >= 5.0));
  rtb_LogicalOperator2 = (rtb_LogicalOperator1 || (rtb_Sum_a <= 0.0));

  /* DiscreteIntegrator: '<S104>/Integrator' */
  if (rtb_LogicalOperator2 || (driveController_DW.Integrator_PrevResetState != 0))
  {
    driveController_DW.Integrator_DSTATE = 0.0;
  }

  /* DiscreteIntegrator: '<S99>/Filter' */
  if (rtb_LogicalOperator2 || (driveController_DW.Filter_PrevResetState != 0)) {
    driveController_DW.Filter_DSTATE = 0.0;
  }

  /* Gain: '<S107>/Filter Coefficient' incorporates:
   *  DiscreteIntegrator: '<S99>/Filter'
   *  Gain: '<S97>/Derivative Gain'
   *  Sum: '<S99>/SumD'
   */
  rtb_Integrator = (slip_Kd_p * rtb_Sum_a - driveController_DW.Filter_DSTATE) *
    50.0;

  /* Sum: '<S113>/Sum' incorporates:
   *  DiscreteIntegrator: '<S104>/Integrator'
   *  Gain: '<S109>/Proportional Gain'
   */
  rtb_Filter_d = (slip_Kp_p * rtb_Sum_a + driveController_DW.Integrator_DSTATE)
    + rtb_Integrator;

  /* Saturate: '<S111>/Saturation' */
  if (rtb_Filter_d > 21.0) {
    rtb_Integrator_lq = 21.0;
  } else if (rtb_Filter_d < 0.0) {
    rtb_Integrator_lq = 0.0;
  } else {
    rtb_Integrator_lq = rtb_Filter_d;
  }

  /* End of Saturate: '<S111>/Saturation' */

  /* Gain: '<S101>/Integral Gain' */
  rtb_IntegralGain = slip_Ki_p * rtb_Sum_a;

  /* MATLAB Function: '<S7>/MATLAB Function1' incorporates:
   *  Saturate: '<S7>/Saturation3'
   */
  rtb_T_cut_RR = (double)rtb_enable * rtb_Integrator_lq;

  /* Sum: '<S7>/Sum' incorporates:
   *  Inport: '<Root>/lambda_refRL'
   */
  rtb_e_RL_i -= driveController_U.lambda_refRL;

  /* Logic: '<S7>/Logical Operator1' incorporates:
   *  Constant: '<S12>/Constant'
   *  RelationalOperator: '<S12>/Compare'
   */
  rtb_LogicalOperator1 = (rtb_LogicalOperator1 || (rtb_e_RL_i <= 0.0));

  /* DiscreteIntegrator: '<S52>/Integrator' */
  if (rtb_LogicalOperator1 || (driveController_DW.Integrator_PrevResetState_l !=
       0)) {
    driveController_DW.Integrator_DSTATE_g = 0.0;
  }

  /* DiscreteIntegrator: '<S47>/Filter' */
  if (rtb_LogicalOperator1 || (driveController_DW.Filter_PrevResetState_i != 0))
  {
    driveController_DW.Filter_DSTATE_i = 0.0;
  }

  /* Gain: '<S55>/Filter Coefficient' incorporates:
   *  DiscreteIntegrator: '<S47>/Filter'
   *  Gain: '<S45>/Derivative Gain'
   *  Sum: '<S47>/SumD'
   */
  rtb_FilterCoefficient_k = (slip_Kd_p * rtb_e_RL_i -
    driveController_DW.Filter_DSTATE_i) * 50.0;

  /* Sum: '<S61>/Sum' incorporates:
   *  DiscreteIntegrator: '<S52>/Integrator'
   *  Gain: '<S57>/Proportional Gain'
   */
  rtb_Sum_a = (slip_Kp_p * rtb_e_RL_i + driveController_DW.Integrator_DSTATE_g)
    + rtb_FilterCoefficient_k;

  /* Saturate: '<S59>/Saturation' */
  if (rtb_Sum_a > 21.0) {
    rtb_Saturation_o = 21.0;
  } else if (rtb_Sum_a < 0.0) {
    rtb_Saturation_o = 0.0;
  } else {
    rtb_Saturation_o = rtb_Sum_a;
  }

  /* End of Saturate: '<S59>/Saturation' */

  /* MATLAB Function: '<S7>/MATLAB Function' incorporates:
   *  Saturate: '<S7>/Saturation1'
   */
  rtb_T_cut_RL = (double)rtb_enable * rtb_Saturation_o;

  /* MinMax: '<Root>/Max1' */
  rtb_k_slip = fmax(rtb_T_cut_RL, rtb_T_cut_RR);

  /* Gain: '<Root>/Gain9' incorporates:
   *  Gain: '<Root>/Steering Ratio'
   *  Inport: '<Root>/Steering Angle �'
   */
  rtb_D_rad = 0.23809523809523808 * driveController_U.SteeringAngle *
    0.017453292519943295;

  /* Abs: '<Root>/Abs' incorporates:
   *  Abs: '<S8>/Abs1'
   *  Gain: '<Root>/Gain'
   */
  dT_real = fabs(240.64227395494575 * rtb_D_rad);

  /* RelationalOperator: '<S1>/Compare' incorporates:
   *  Abs: '<Root>/Abs'
   *  Constant: '<S1>/Constant'
   */
  rtb_enable = (dT_real <= 1.0);

  /* MATLAB Function: '<Root>/Grip MATLAB Estimator' incorporates:
   *  Inport: '<Root>/Yaw_rate_act'
   *  Inport: '<Root>/ax'
   *  Inport: '<Root>/ay'
   *  MATLAB Function: '<Root>/Regen MATLAB Function'
   *  MATLAB Function: '<S10>/beta_dot estimator'
   */
  delta_Fz_lat = (float)driveController_U.ay * 9.81F * 270.0F * 0.194F / 1.2F *
    0.4F;
  Fz_RR = ((float)rtb_Max * (float)rtb_Max * 0.67375F * 0.7F + 1208.1108F) /
    2.0F + (float)driveController_U.ax * 9.81F * 270.0F * 0.194F / 1.57F * 0.5F;
  Fz_RL = Fz_RR + delta_Fz_lat;
  Fz_RR -= delta_Fz_lat;
  if (Fz_RL < 0.0F) {
    Fz_RL = 0.0F;
  }

  if (Fz_RR < 0.0F) {
    Fz_RR = 0.0F;
  }

  dynamic_reserve = fminf(1.0F, fmaxf(0.0F, fabsf((float)(driveController_U.ay /
    fmin(1.0, rtb_Max)) - driveController_U.yaw_meas) / beta_dot_threshold_p)) *
    0.55F + 0.05F;
  delta_Fz_lat = ((Fz_RL - 1112.0F) / 1112.0F * -0.99198F + 1.00347F) * mu_p;
  if (delta_Fz_lat < 0.5F) {
    delta_Fz_lat = 0.5F;
  }

  Fx_total_potential_RL = Fz_RL * delta_Fz_lat;
  delta_Fz_lat = fabsf((float)driveController_U.ay);
  Fy_est_RL = Fz_RL * delta_Fz_lat;
  Fz_RL = ((Fz_RR - 1112.0F) / 1112.0F * -0.99198F + 1.00347F) * mu_p;
  if (Fz_RL < 0.5F) {
    Fz_RL = 0.5F;
  }

  Fz_RL *= Fz_RR;
  Fy_est_RR = Fz_RR * delta_Fz_lat;
  if (Fx_total_potential_RL > Fy_est_RL) {
    Fz_RR = fmaxf(sqrtf(Fx_total_potential_RL * Fx_total_potential_RL -
                        Fy_est_RL * Fy_est_RL), Fx_total_potential_RL *
                  dynamic_reserve);
  } else {
    Fz_RR = Fx_total_potential_RL * dynamic_reserve;
  }

  driveController_Y.T_max_RL = Fz_RR * 0.2032F / 12.0F * overslip_factor_p;
  if (Fz_RL > Fy_est_RR) {
    Fz_RR = fmaxf(sqrtf(Fz_RL * Fz_RL - Fy_est_RR * Fy_est_RR), Fz_RL *
                  dynamic_reserve);
  } else {
    Fz_RR = Fz_RL * dynamic_reserve;
  }

  driveController_Y.T_max_RR = Fz_RR * 0.2032F / 12.0F * overslip_factor_p;

  /* End of MATLAB Function: '<Root>/Grip MATLAB Estimator' */

  /* Switch: '<Root>/Switch' */
  if (rtb_enable) {
    Sum1 = rtb_k_slip;
  } else {
    Sum1 = rtb_T_cut_RL;
  }

  /* Sum: '<Root>/Sum1' incorporates:
   *  Switch: '<Root>/Switch'
   */
  Sum1 = driveController_Y.T_max_RL - Sum1;

  /* Saturate: '<S3>/Saturation1' incorporates:
   *  Inport: '<Root>/Throttle_%'
   */
  if (driveController_U.Throttle_ > 100.0) {
    /* Saturate: '<S3>/Saturation1' */
    driveController_Y.Throttle_real = 100.0;
  } else if (driveController_U.Throttle_ < 0.0) {
    /* Saturate: '<S3>/Saturation1' */
    driveController_Y.Throttle_real = 0.0;
  } else {
    /* Saturate: '<S3>/Saturation1' */
    driveController_Y.Throttle_real = driveController_U.Throttle_;
  }

  /* End of Saturate: '<S3>/Saturation1' */

  /* MultiPortSwitch: '<S3>/Multiport Switch' incorporates:
   *  Constant: '<S3>/Constant1'
   *  Lookup_n-D: '<S3>/1-D Lookup Table'
   *  Lookup_n-D: '<S3>/1-D Lookup Table1'
   *  Lookup_n-D: '<S3>/1-D Lookup Table2'
   *  Saturate: '<S3>/Saturation1'
   */
  switch (pedal_map_select_p) {
   case 1:
    rtb_Filter_l = look1_binlxpw(driveController_Y.Throttle_real,
      driveController_ConstP.pooled7,
      driveController_ConstP.uDLookupTable_tableData, 20U);
    break;

   case 2:
    rtb_Filter_l = look1_binlxpw(driveController_Y.Throttle_real,
      driveController_ConstP.pooled7,
      driveController_ConstP.uDLookupTable1_tableData, 20U);
    break;

   default:
    rtb_Filter_l = look1_binlxpw(driveController_Y.Throttle_real,
      driveController_ConstP.pooled7,
      driveController_ConstP.uDLookupTable2_tableData, 20U);
    break;
  }

  /* End of MultiPortSwitch: '<S3>/Multiport Switch' */

  /* Gain: '<S3>/Gain5' */
  rtb_Filter_l *= 100.0;

  /* RateLimiter: '<S3>/Rate Limiter1' */
  w_RL = rtb_Filter_l - driveController_DW.PrevY;
  if (w_RL > 10.0) {
    rtb_Filter_l = driveController_DW.PrevY + 10.0;
  } else if (w_RL < -2.5) {
    rtb_Filter_l = driveController_DW.PrevY - 2.5;
  }

  driveController_DW.PrevY = rtb_Filter_l;

  /* End of RateLimiter: '<S3>/Rate Limiter1' */

  /* RateLimiter: '<S3>/Rate Limiter2' */
  w_RL = driveController_Y.Throttle_real - driveController_DW.PrevY_p;
  if (w_RL > 10.0) {
    /* Saturate: '<S3>/Saturation1' */
    driveController_Y.Throttle_real = driveController_DW.PrevY_p + 10.0;
  } else if (w_RL < -2.5) {
    /* Saturate: '<S3>/Saturation1' */
    driveController_Y.Throttle_real = driveController_DW.PrevY_p - 2.5;
  }

  driveController_DW.PrevY_p = driveController_Y.Throttle_real;

  /* End of RateLimiter: '<S3>/Rate Limiter2' */

  /* Switch: '<S3>/Switch1' incorporates:
   *  Inport: '<Root>/Throttle_%'
   */
  if (driveController_U.Throttle_ > 1.0) {
    /* Switch: '<S3>/Switch' */
    if (rtb_Max > 5.0) {
      /* Saturate: '<S3>/Saturation1' */
      driveController_Y.Throttle_real = rtb_Filter_l;
    }

    /* Saturate: '<S3>/Saturation' incorporates:
     *  Switch: '<S3>/Switch'
     */
    if (driveController_Y.Throttle_real > 100.0) {
      /* Saturate: '<S3>/Saturation1' */
      driveController_Y.Throttle_real = 100.0;
    } else if (driveController_Y.Throttle_real < 0.0) {
      /* Saturate: '<S3>/Saturation1' */
      driveController_Y.Throttle_real = 0.0;
    }

    /* End of Saturate: '<S3>/Saturation' */
  } else {
    /* Saturate: '<S3>/Saturation1' incorporates:
     *  Constant: '<S3>/Constant'
     */
    driveController_Y.Throttle_real = 0.0;
  }

  /* End of Switch: '<S3>/Switch1' */

  /* MATLAB Function: '<Root>/Regen MATLAB Function' incorporates:
   *  Inport: '<Root>/n_motorRL'
   *  Inport: '<Root>/n_motorRR'
   */
  dynamic_reserve = (fabsf((float)driveController_U.n_motorRL) + fabsf((float)
    driveController_U.n_motorRR)) * 0.5F;
  if (driveController_DW.regen_latch) {
    Fz_RR = regen_pedal_thr_p + regen_pedal_hyst_p;
  } else {
    Fz_RR = regen_pedal_thr_p;
  }

  driveController_DW.regen_latch = (((float)driveController_Y.Throttle_real <=
    Fz_RR) && (dynamic_reserve > 1000.0F) && ((float)rtb_Max > 0.5F));
  if (!driveController_DW.regen_latch) {
    driveController_DW.t_elapsed_s = 0.0;
    dynamic_reserve = 0.0F;
    rtb_regen_active = 0;
  } else {
    driveController_DW.t_elapsed_s += 0.01;
    Fx_total_potential_RL = fmaxf(6.2831855F * dynamic_reserve / 60.0F, 1.0F);
    if (dynamic_reserve >= 4000.0F) {
      Fz_RR = 1.0F;
    } else {
      Fz_RR = fmaxf(0.0F, fminf(1.0F, (dynamic_reserve - 1000.0F) / 3000.0F));
    }

    if (driveController_DW.t_elapsed_s < 4.0) {
      rtb_regen_active = 21;
    } else {
      rtb_regen_active = 14;
    }

    dynamic_reserve = fmaxf(fmaxf((1.0F - (float)driveController_Y.Throttle_real
      / regen_pedal_thr_p) * -regen_T_max_p * Fz_RR, -(17500.0F /
      Fx_total_potential_RL)), -(fminf((float)rtb_regen_active, I_charge_max_p) *
      0.5F * V_dc_p / Fx_total_potential_RL)) * fmaxf(0.0F, 1.0F - delta_Fz_lat /
      1.5F);
    rtb_regen_active = 1;
  }

  /* RateLimiter: '<Root>/Rate Limiter3' */
  Fx_total_potential_RL = dynamic_reserve - driveController_DW.PrevY_m;
  if (Fx_total_potential_RL > 0.39999998F) {
    dynamic_reserve = driveController_DW.PrevY_m + 0.39999998F;
  } else if (Fx_total_potential_RL < -0.59999996F) {
    dynamic_reserve = driveController_DW.PrevY_m - 0.59999996F;
  }

  driveController_DW.PrevY_m = dynamic_reserve;

  /* End of RateLimiter: '<Root>/Rate Limiter3' */

  /* Switch: '<S4>/Switch' incorporates:
   *  Constant: '<S4>/Constant'
   *  Constant: '<S4>/Constant1'
   *  Inport: '<Root>/Derating_RL'
   *  Inport: '<Root>/Derating_RR'
   *  Logic: '<S4>/Logical Operator'
   */
  if (driveController_U.Derating_RL || driveController_U.Derating_RR) {
    rtb_Filter_l = 9.8;
  } else {
    rtb_Filter_l = 21.0;
  }

  /* End of Switch: '<S4>/Switch' */

  /* RateLimiter: '<S4>/Rate Limiter' */
  w_RL = rtb_Filter_l - driveController_DW.PrevY_h;
  if (w_RL > 2.5) {
    rtb_Filter_l = driveController_DW.PrevY_h + 2.5;
  } else if (w_RL < -2.5) {
    rtb_Filter_l = driveController_DW.PrevY_h - 2.5;
  }

  driveController_DW.PrevY_h = rtb_Filter_l;

  /* End of RateLimiter: '<S4>/Rate Limiter' */

  /* MinMax: '<S4>/Min' incorporates:
   *  Gain: '<S4>/Gain'
   */
  rtb_Filter_l = fmin(0.21 * driveController_Y.Throttle_real, rtb_Filter_l);

  /* RateLimiter: '<S4>/Rate Limiter1' */
  w_RL = rtb_Filter_l - driveController_DW.PrevY_k;
  if (w_RL > 2.5) {
    rtb_Filter_l = driveController_DW.PrevY_k + 2.5;
  } else if (w_RL < -0.1) {
    rtb_Filter_l = driveController_DW.PrevY_k - 0.1;
  }

  driveController_DW.PrevY_k = rtb_Filter_l;

  /* End of RateLimiter: '<S4>/Rate Limiter1' */

  /* Trigonometry: '<S9>/Trigonometric Function' */
  rtb_D_rad = tan(rtb_D_rad);

  /* MATLAB Function: '<S9>/tand safe' */
  if (fabs(rtb_D_rad) < 0.017455064928217585) {
    rtb_D_rad = 0.0;
  }

  /* MATLAB Function: '<S9>/Bicycle Model Dynamic' incorporates:
   *  MATLAB Function: '<S9>/tand safe'
   */
  delta_Fz_lat = (float)(rtb_Max / 1.5699999999999998 * rtb_D_rad) / ((float)
    (rtb_Max * rtb_Max) * Kus_p + 1.0F);

  /* RateLimiter: '<S9>/Rate Lmiter' */
  Fx_total_potential_RL = delta_Fz_lat - driveController_DW.PrevY_a;
  if (Fx_total_potential_RL > 0.02F) {
    delta_Fz_lat = driveController_DW.PrevY_a + 0.02F;
  } else if (Fx_total_potential_RL < -0.02F) {
    delta_Fz_lat = driveController_DW.PrevY_a - 0.02F;
  }

  driveController_DW.PrevY_a = delta_Fz_lat;

  /* End of RateLimiter: '<S9>/Rate Lmiter' */

  /* MATLAB Function: '<S9>/yaw_saturation' */
  if (rtIsNaNF(delta_Fz_lat)) {
    Fz_RR = (rtNaNF);
  } else if (delta_Fz_lat < 0.0F) {
    Fz_RR = -1.0F;
  } else {
    Fz_RR = (float)(delta_Fz_lat > 0.0F);
  }

  driveController_Y.yaw_th = fminf(fabsf(delta_Fz_lat), (float)
    (11.183399999999999 / fmax(rtb_Max, 0.5))) * Fz_RR;

  /* End of MATLAB Function: '<S9>/yaw_saturation' */

  /* Sum: '<Root>/Sum' incorporates:
   *  Inport: '<Root>/Yaw_rate_act'
   */
  delta_Fz_lat = driveController_Y.yaw_th - driveController_U.yaw_meas;

  /* RelationalOperator: '<S122>/Compare' incorporates:
   *  Abs: '<S8>/Abs'
   *  Constant: '<S122>/Constant'
   */
  rtb_Compare_a = (fabsf(delta_Fz_lat) <= 0.01F);

  /* DiscreteIntegrator: '<S160>/Integrator' */
  if (rtb_Compare_a || (driveController_DW.Integrator_PrevResetState_a != 0)) {
    driveController_DW.Integrator_DSTATE_k = 0.0;
  }

  /* DiscreteIntegrator: '<S155>/Filter' */
  if (rtb_Compare_a || (driveController_DW.Filter_PrevResetState_a != 0)) {
    driveController_DW.Filter_DSTATE_e = 0.0;
  }

  /* Gain: '<S163>/Filter Coefficient' incorporates:
   *  DiscreteIntegrator: '<S155>/Filter'
   *  Gain: '<S153>/Derivative Gain'
   *  Sum: '<S155>/SumD'
   */
  rtb_D_rad = (tvc_Kd_p * delta_Fz_lat - driveController_DW.Filter_DSTATE_e) *
    35.0;

  /* Sum: '<S169>/Sum' incorporates:
   *  DiscreteIntegrator: '<S160>/Integrator'
   *  Gain: '<S165>/Proportional Gain'
   */
  rtb_Sum = (tvc_Kp_p * delta_Fz_lat + driveController_DW.Integrator_DSTATE_k) +
    rtb_D_rad;

  /* Saturate: '<S167>/Saturation' */
  if (rtb_Sum > 700.0) {
    rtb_Saturation = 700.0;
  } else if (rtb_Sum < -700.0) {
    rtb_Saturation = -700.0;
  } else {
    rtb_Saturation = rtb_Sum;
  }

  /* End of Saturate: '<S167>/Saturation' */

  /* Product: '<S8>/Product' incorporates:
   *  Lookup_n-D: '<Root>/1-D Lookup Table'
   *  Lookup_n-D: '<S8>/1-D Lookup Table'
   *  MATLAB Function: '<S8>/MATLAB Function'
   *  Switch: '<Root>/Switch2'
   */
  dT_real = rtb_Saturation * 0.2032 / 36.0 * look1_binlcapw(rtb_k_slip,
    driveController_ConstP.uDLookupTable_bp01Data,
    driveController_ConstP.uDLookupTable_tableData_o, 2U) * look1_binlcapw
    (dT_real, driveController_ConstP.uDLookupTable_bp01Data_i,
     driveController_ConstP.uDLookupTable_tableData_j, 2U);

  /* Switch: '<Root>/Switch2' */
  if (!rtb_enable) {
    rtb_k_slip = rtb_T_cut_RR;
  }

  /* Sum: '<Root>/Sum2' incorporates:
   *  Switch: '<Root>/Switch2'
   */
  rtb_k_slip = driveController_Y.T_max_RR - rtb_k_slip;
  if (driveController_M->Timing.TaskCounters.TID[1] == 0) {
    /* MATLAB Function: '<S8>/TorqueAllocator' incorporates:
     *  Inport: '<Root>/n_motorRL'
     *  Inport: '<Root>/n_motorRR'
     */
    Fz_RR = V_dc_p * 58.33F;
    w_RL = fmax(fabs(driveController_U.n_motorRL) * 0.10471975511965977, 1.0);
    w_RR = fmax(fabs(driveController_U.n_motorRR) * 0.10471975511965977, 1.0);
    tmp[0] = T_peak_p;
    tmp[1] = Fz_RR / (float)w_RL;
    tmp[2] = (float)Sum1;
    Fx_total_potential_RL = driveController_minimum(tmp);
    tmp[0] = T_peak_p;
    tmp[1] = Fz_RR / (float)w_RR;
    tmp[2] = (float)rtb_k_slip;
    Fz_RL = driveController_minimum(tmp);
    tmp[0] = regen_T_max_p;
    tmp[1] = (float)Sum1;
    Fz_RR = I_charge_max_p * 0.5F * V_dc_p;
    tmp[2] = Fz_RR / (float)w_RL;
    Fy_est_RL = driveController_minimum(tmp);
    tmp[0] = regen_T_max_p;
    tmp[1] = (float)rtb_k_slip;
    tmp[2] = Fz_RR / (float)w_RR;
    Fy_est_RR = driveController_minimum(tmp);

    /* Switch: '<Root>/Switch1' */
    if (rtb_regen_active > 0.5) {
      rtb_Filter_l = dynamic_reserve;
    }

    /* MATLAB Function: '<S8>/TorqueAllocator' incorporates:
     *  Inport: '<Root>/ay'
     *  Switch: '<Root>/Switch1'
     */
    Fz_RR = (1.0F - (float)fmin(0.3, 0.15 * fabs(driveController_U.ay))) *
      (float)rtb_Filter_l;

    /* Saturate: '<Root>/Saturation' incorporates:
     *  MATLAB Function: '<S8>/TorqueAllocator'
     */
    driveController_Y.T_RR = Fz_RR - (float)dT_real;

    /* MATLAB Function: '<S8>/TorqueAllocator' incorporates:
     *  Inport: '<Root>/n_motorRL'
     *  Inport: '<Root>/n_motorRR'
     */
    Fz_RR += (float)dT_real;
    if (driveController_Y.T_RR > Fx_total_potential_RL) {
      /* Saturate: '<Root>/Saturation' */
      driveController_Y.T_RR = Fx_total_potential_RL;
    } else if (driveController_Y.T_RR < -Fy_est_RL) {
      /* Saturate: '<Root>/Saturation' */
      driveController_Y.T_RR = -Fy_est_RL;
    }

    if (Fz_RR > Fz_RL) {
      Fz_RR = Fz_RL;
    } else if (Fz_RR < -Fy_est_RR) {
      Fz_RR = -Fy_est_RR;
    }

    if ((driveController_U.n_motorRL < 100.0) && (driveController_Y.T_RR < 0.0F))
    {
      /* Saturate: '<Root>/Saturation' */
      driveController_Y.T_RR = 0.0F;
    }

    if ((driveController_U.n_motorRR < 100.0) && (Fz_RR < 0.0F)) {
      Fz_RR = 0.0F;
    }

    dynamic_reserve = fmaxf(0.0F, driveController_Y.T_RR * (float)w_RL) / 0.85F
      + fmaxf(0.0F, Fz_RR * (float)w_RR) / 0.85F;
    Fx_total_potential_RL = V_dc_p * I_discharge_max_p;
    if ((dynamic_reserve > Fx_total_potential_RL) && (dynamic_reserve > 0.0F)) {
      dynamic_reserve = Fx_total_potential_RL / dynamic_reserve;

      /* Saturate: '<Root>/Saturation' */
      driveController_Y.T_RR = fmaxf(0.0F, driveController_Y.T_RR) *
        dynamic_reserve + fminf(0.0F, driveController_Y.T_RR);
      Fz_RR = fmaxf(0.0F, Fz_RR) * dynamic_reserve + fminf(0.0F, Fz_RR);
    }

    /* RateLimiter: '<Root>/Rate Limiter1' incorporates:
     *  MATLAB Function: '<S8>/TorqueAllocator'
     */
    Fx_total_potential_RL = driveController_Y.T_RR - driveController_DW.PrevY_f;
    if (Fx_total_potential_RL > 5.0F) {
      /* Saturate: '<Root>/Saturation' */
      driveController_Y.T_RR = driveController_DW.PrevY_f + 5.0F;
    } else if (Fx_total_potential_RL < -25.0F) {
      /* Saturate: '<Root>/Saturation' */
      driveController_Y.T_RR = driveController_DW.PrevY_f - 25.0F;
    }

    driveController_DW.PrevY_f = driveController_Y.T_RR;

    /* End of RateLimiter: '<Root>/Rate Limiter1' */

    /* Saturate: '<Root>/Saturation1' */
    if (driveController_Y.T_RR > T_peak_p) {
      /* Outport: '<Root>/T_RL' */
      driveController_Y.T_RL = T_peak_p;
    } else if (driveController_Y.T_RR < -regen_T_max_p) {
      /* Outport: '<Root>/T_RL' */
      driveController_Y.T_RL = -regen_T_max_p;
    } else {
      /* Outport: '<Root>/T_RL' */
      driveController_Y.T_RL = driveController_Y.T_RR;
    }

    /* End of Saturate: '<Root>/Saturation1' */

    /* RateLimiter: '<Root>/Rate Limiter2' incorporates:
     *  MATLAB Function: '<S8>/TorqueAllocator'
     */
    Fx_total_potential_RL = Fz_RR - driveController_DW.PrevY_j;
    if (Fx_total_potential_RL > 5.0F) {
      /* Saturate: '<Root>/Saturation' */
      driveController_Y.T_RR = driveController_DW.PrevY_j + 5.0F;
    } else if (Fx_total_potential_RL < -25.0F) {
      /* Saturate: '<Root>/Saturation' */
      driveController_Y.T_RR = driveController_DW.PrevY_j - 25.0F;
    } else {
      /* Saturate: '<Root>/Saturation' */
      driveController_Y.T_RR = Fz_RR;
    }

    driveController_DW.PrevY_j = driveController_Y.T_RR;

    /* End of RateLimiter: '<Root>/Rate Limiter2' */

    /* Saturate: '<Root>/Saturation' */
    if (driveController_Y.T_RR > T_peak_p) {
      /* Saturate: '<Root>/Saturation' incorporates:
       *  Outport: '<Root>/T_RR'
       */
      driveController_Y.T_RR = T_peak_p;
    } else if (driveController_Y.T_RR < -regen_T_max_p) {
      /* Saturate: '<Root>/Saturation' incorporates:
       *  Outport: '<Root>/T_RR'
       */
      driveController_Y.T_RR = -regen_T_max_p;
    }

    /* End of Saturate: '<Root>/Saturation' */
  }

  /* Chart: '<Root>/Mode Manager' */
  if (driveController_DW.is_active_c30_driveController == 0) {
    driveController_DW.is_active_c30_driveController = 1U;
    driveController_DW.is_c30_driveController = driveController_IN_IDLE_;
    rtb_TVC_Target_Weight = 0;
  } else if (driveController_DW.is_c30_driveController ==
             driveController_IN_IDLE_) {
    rtb_TVC_Target_Weight = 0;
    if (rtb_Max > 4.0) {
      driveController_DW.is_c30_driveController = driveController_IN_TVC_;
      rtb_TVC_Target_Weight = 1;
    }
  } else {
    /* case IN_TVC_: */
    rtb_TVC_Target_Weight = 1;
    if (rtb_Max <= 2.0) {
      driveController_DW.is_c30_driveController = driveController_IN_IDLE_;
      rtb_TVC_Target_Weight = 0;
    }
  }

  /* End of Chart: '<Root>/Mode Manager' */

  /* RateLimiter: '<Root>/W_RateLimiter' */
  w_RL = (double)rtb_TVC_Target_Weight - driveController_DW.PrevY_k0;
  if (w_RL > 0.02) {
    /* RateLimiter: '<Root>/W_RateLimiter' */
    driveController_DW.PrevY_k0 += 0.02;
  } else if (w_RL < -0.02) {
    /* RateLimiter: '<Root>/W_RateLimiter' */
    driveController_DW.PrevY_k0 -= 0.02;
  } else {
    /* RateLimiter: '<Root>/W_RateLimiter' */
    driveController_DW.PrevY_k0 = rtb_TVC_Target_Weight;
  }

  /* End of RateLimiter: '<Root>/W_RateLimiter' */

  /* Outport: '<Root>/TVC_Target_Weight' */
  driveController_Y.TVC_Target_Weight = driveController_DW.PrevY_k0;

  /* Outport: '<Root>/regen' */
  driveController_Y.regen = rtb_regen_active;

  /* Outport: '<Root>/K_slips' */
  driveController_Y.K_slips[0] = rtb_T_cut_RL;
  driveController_Y.K_slips[1] = rtb_T_cut_RR;

  /* Update for DiscreteIntegrator: '<S104>/Integrator' incorporates:
   *  Gain: '<S96>/Kb'
   *  Sum: '<S96>/SumI2'
   *  Sum: '<S96>/SumI4'
   */
  driveController_DW.Integrator_DSTATE += ((rtb_Integrator_lq - rtb_Filter_d) *
    10.0 + rtb_IntegralGain) * 0.01;
  if (driveController_DW.Integrator_DSTATE > 21.0) {
    driveController_DW.Integrator_DSTATE = 21.0;
  } else if (driveController_DW.Integrator_DSTATE < 0.0) {
    driveController_DW.Integrator_DSTATE = 0.0;
  }

  driveController_DW.Integrator_PrevResetState = (int8_t)rtb_LogicalOperator2;

  /* Update for DiscreteIntegrator: '<S99>/Filter' incorporates:
   *  DiscreteIntegrator: '<S104>/Integrator'
   */
  driveController_DW.Filter_DSTATE += 0.01 * rtb_Integrator;
  driveController_DW.Filter_PrevResetState = (int8_t)rtb_LogicalOperator2;

  /* Update for DiscreteIntegrator: '<S52>/Integrator' incorporates:
   *  Gain: '<S44>/Kb'
   *  Gain: '<S49>/Integral Gain'
   *  Sum: '<S44>/SumI2'
   *  Sum: '<S44>/SumI4'
   */
  driveController_DW.Integrator_DSTATE_g += ((rtb_Saturation_o - rtb_Sum_a) *
    10.0 + slip_Ki_p * rtb_e_RL_i) * 0.01;
  if (driveController_DW.Integrator_DSTATE_g > 21.0) {
    driveController_DW.Integrator_DSTATE_g = 21.0;
  } else if (driveController_DW.Integrator_DSTATE_g < 0.0) {
    driveController_DW.Integrator_DSTATE_g = 0.0;
  }

  driveController_DW.Integrator_PrevResetState_l = (int8_t)rtb_LogicalOperator1;

  /* Update for DiscreteIntegrator: '<S47>/Filter' incorporates:
   *  DiscreteIntegrator: '<S52>/Integrator'
   */
  driveController_DW.Filter_DSTATE_i += 0.01 * rtb_FilterCoefficient_k;
  driveController_DW.Filter_PrevResetState_i = (int8_t)rtb_LogicalOperator1;

  /* Update for DiscreteIntegrator: '<S160>/Integrator' incorporates:
   *  Gain: '<S152>/Kb'
   *  Gain: '<S157>/Integral Gain'
   *  Sum: '<S152>/SumI2'
   *  Sum: '<S152>/SumI4'
   */
  driveController_DW.Integrator_DSTATE_k += ((rtb_Saturation - rtb_Sum) * 17.0 +
    tvc_Ki_p * delta_Fz_lat) * 0.01;
  driveController_DW.Integrator_PrevResetState_a = (int8_t)rtb_Compare_a;

  /* Update for DiscreteIntegrator: '<S155>/Filter' incorporates:
   *  DiscreteIntegrator: '<S160>/Integrator'
   */
  driveController_DW.Filter_DSTATE_e += 0.01 * rtb_D_rad;
  driveController_DW.Filter_PrevResetState_a = (int8_t)rtb_Compare_a;
  rate_scheduler();
}

/* Model initialize function */
void driveController_initialize(void)
{
  /* (no initialization code required) */
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
