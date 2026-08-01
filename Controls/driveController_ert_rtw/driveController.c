/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: driveController.c
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

#include "driveController.h"
#include <stdint.h>
#include <math.h>
#include "rt_nonfinite.h"
#include <stdbool.h>
#include "driveController_private.h"

/* Named constants for Chart: '<Root>/Mode Manager' */
#define driveController_IN_IDLE_       ((uint8_t)1U)
#define driveController_IN_TVC_        ((uint8_t)2U)
#define driveController_period         (0.01)

/* Exported block parameters */
double V_dc_p = 540.0;                 /* Variable: V_dc_p
                                        * Referenced by: '<S8>/TorqueAllocator'
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
  if ((driveController_M->Timing.TaskCounters.TID[1]) > 9) {/* Sample time: [0.01s, 0.0s] */
    driveController_M->Timing.TaskCounters.TID[1] = 0;
  }

  (driveController_M->Timing.TaskCounters.TID[2])++;
  if ((driveController_M->Timing.TaskCounters.TID[2]) > 49) {/* Sample time: [0.05s, 0.0s] */
    driveController_M->Timing.TaskCounters.TID[2] = 0;
  }
}

/* Model step function */
void driveController_step(void)
{
  double Fz_RL;
  double P_tot_req;
  double T_L;
  double T_req_eff;
  double delta_Fz_lat;
  double omega_safe;
  double rtb_Filter;
  double rtb_FilterCoefficient_l;
  double rtb_FilterCoefficient_na;
  double rtb_Integrator;
  double rtb_Integrator_c;
  double rtb_Max;
  double rtb_Saturation;
  double rtb_Saturation_g;
  double rtb_Sum;
  double rtb_Sum_h;
  double rtb_T_RL;
  double rtb_T_RR;
  double rtb_T_cut_RL;
  double rtb_Throttle_l;
  double rtb_e_RL_i;
  double speed_avg_rpm;
  double w_RL;
  double w_RR;
  double w_RR_tmp;
  int32_t rtb_TVC_Target_Weight;
  int32_t rtb_regen_active;
  bool rtb_Compare_i;
  bool rtb_LogicalOperator1;
  bool rtb_LogicalOperator2;
  bool rtb_enable;
  bool tmp;
  tmp = (driveController_M->Timing.TaskCounters.TID[1] == 0);
  if (tmp) {
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
    rtb_Filter = fmax(rtb_Max, 0.1);

    /* Gain: '<S7>/w_car//2' incorporates:
     *  Inport: '<Root>/Yaw_rate_act'
     */
    rtb_Integrator = 0.6 * driveController_U.yaw_meas;

    /* Product: '<S7>/Divide' incorporates:
     *  Constant: '<S7>/Constant1'
     *  MinMax: '<S7>/Max1'
     *  Sum: '<S7>/Add'
     *  Sum: '<S7>/Add1'
     */
    rtb_e_RL_i = (rtb_e_RL_i - (rtb_Filter - rtb_Integrator)) / fmax(rtb_e_RL_i,
      0.1);

    /* Gain: '<S7>/Gain5' incorporates:
     *  Gain: '<S7>/Gain3'
     *  Gain: '<S7>/Gain4'
     *  Inport: '<Root>/n_motorRR'
     */
    rtb_Sum_h = 0.06666666666666667 * driveController_U.n_motorRR *
      0.10471975511965977 * 0.2032;

    /* Saturate: '<S7>/Saturation5' */
    if (rtb_Sum_h <= 0.0) {
      rtb_Sum_h = 0.0;
    }

    /* End of Saturate: '<S7>/Saturation5' */

    /* Product: '<S7>/Divide1' incorporates:
     *  Constant: '<S7>/Constant2'
     *  MinMax: '<S7>/Max2'
     *  Sum: '<S7>/Add2'
     *  Sum: '<S7>/Add3'
     */
    rtb_Sum_h = (rtb_Sum_h - (rtb_Filter + rtb_Integrator)) / fmax(rtb_Sum_h,
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

    if (rtb_Sum_h > 1.0) {
      /* Outport: '<Root>/lambda' */
      driveController_Y.lambda[3] = 1.0;
    } else if (rtb_Sum_h < 0.0) {
      /* Outport: '<Root>/lambda' */
      driveController_Y.lambda[3] = 0.0;
    } else {
      /* Outport: '<Root>/lambda' */
      driveController_Y.lambda[3] = rtb_Sum_h;
    }

    /* End of Saturate: '<S7>/Saturation2' */

    /* Sum: '<S7>/Sum1' incorporates:
     *  Inport: '<Root>/lambda_refRR'
     */
    rtb_Sum_h -= driveController_U.lambda_refRR;

    /* RelationalOperator: '<S7>/Relational Operator' incorporates:
     *  Constant: '<S7>/Constant'
     */
    rtb_enable = (rtb_Filter >= 3.0);

    /* Logic: '<S7>/Logical Operator2' incorporates:
     *  Constant: '<S10>/Constant'
     *  Constant: '<S12>/Constant'
     *  Inport: '<Root>/brake_%'
     *  Logic: '<S7>/Logical Operator'
     *  Logic: '<S7>/Logical Operator1'
     *  RelationalOperator: '<S10>/Compare'
     *  RelationalOperator: '<S12>/Compare'
     */
    rtb_LogicalOperator1 = (!rtb_enable || (driveController_U.brake_ >= 5.0));
    rtb_LogicalOperator2 = (rtb_LogicalOperator1 || (rtb_Sum_h <= 0.0));

    /* DiscreteIntegrator: '<S103>/Integrator' */
    if (rtb_LogicalOperator2 || (driveController_DW.Integrator_PrevResetState !=
         0)) {
      driveController_DW.Integrator_DSTATE = 0.0;
    }

    /* DiscreteIntegrator: '<S98>/Filter' */
    if (rtb_LogicalOperator2 || (driveController_DW.Filter_PrevResetState != 0))
    {
      driveController_DW.Filter_DSTATE = 0.0;
    }

    /* Gain: '<S106>/Filter Coefficient' incorporates:
     *  DiscreteIntegrator: '<S98>/Filter'
     *  Gain: '<S96>/Derivative Gain'
     *  Sum: '<S98>/SumD'
     */
    rtb_Filter = (0.8 * rtb_Sum_h - driveController_DW.Filter_DSTATE) * 80.0;

    /* Sum: '<S112>/Sum' incorporates:
     *  DiscreteIntegrator: '<S103>/Integrator'
     *  Gain: '<S108>/Proportional Gain'
     */
    rtb_Integrator = (50.0 * rtb_Sum_h + driveController_DW.Integrator_DSTATE) +
      rtb_Filter;

    /* Saturate: '<S110>/Saturation' */
    if (rtb_Integrator > 25.0) {
      rtb_Integrator_c = 25.0;
    } else if (rtb_Integrator < 0.0) {
      rtb_Integrator_c = 0.0;
    } else {
      rtb_Integrator_c = rtb_Integrator;
    }

    /* End of Saturate: '<S110>/Saturation' */

    /* Sum: '<S95>/SumI4' incorporates:
     *  Gain: '<S100>/Integral Gain'
     *  Gain: '<S95>/Kb'
     *  Sum: '<S95>/SumI2'
     */
    rtb_Integrator = (rtb_Integrator_c - rtb_Integrator) * 10.0 + 13.0 *
      rtb_Sum_h;

    /* MATLAB Function: '<S7>/MATLAB Function1' incorporates:
     *  Saturate: '<S7>/Saturation3'
     */
    rtb_Integrator_c *= (double)rtb_enable;

    /* Sum: '<S7>/Sum' incorporates:
     *  Inport: '<Root>/lambda_refRL'
     */
    rtb_e_RL_i -= driveController_U.lambda_refRL;

    /* Logic: '<S7>/Logical Operator1' incorporates:
     *  Constant: '<S11>/Constant'
     *  RelationalOperator: '<S11>/Compare'
     */
    rtb_LogicalOperator1 = (rtb_LogicalOperator1 || (rtb_e_RL_i <= 0.0));

    /* DiscreteIntegrator: '<S51>/Integrator' */
    if (rtb_LogicalOperator1 || (driveController_DW.Integrator_PrevResetState_a
         != 0)) {
      driveController_DW.Integrator_DSTATE_o = 0.0;
    }

    /* DiscreteIntegrator: '<S46>/Filter' */
    if (rtb_LogicalOperator1 || (driveController_DW.Filter_PrevResetState_p != 0))
    {
      driveController_DW.Filter_DSTATE_e = 0.0;
    }

    /* Gain: '<S54>/Filter Coefficient' incorporates:
     *  DiscreteIntegrator: '<S46>/Filter'
     *  Gain: '<S44>/Derivative Gain'
     *  Sum: '<S46>/SumD'
     */
    rtb_FilterCoefficient_l = (0.8 * rtb_e_RL_i -
      driveController_DW.Filter_DSTATE_e) * 80.0;

    /* Sum: '<S60>/Sum' incorporates:
     *  DiscreteIntegrator: '<S51>/Integrator'
     *  Gain: '<S56>/Proportional Gain'
     */
    rtb_Sum_h = (50.0 * rtb_e_RL_i + driveController_DW.Integrator_DSTATE_o) +
      rtb_FilterCoefficient_l;

    /* Saturate: '<S58>/Saturation' */
    if (rtb_Sum_h > 25.0) {
      rtb_Saturation_g = 25.0;
    } else if (rtb_Sum_h < 0.0) {
      rtb_Saturation_g = 0.0;
    } else {
      rtb_Saturation_g = rtb_Sum_h;
    }

    /* End of Saturate: '<S58>/Saturation' */

    /* MATLAB Function: '<S7>/MATLAB Function' incorporates:
     *  Saturate: '<S7>/Saturation1'
     */
    rtb_T_cut_RL = (double)rtb_enable * rtb_Saturation_g;

    /* MinMax: '<Root>/Max1' */
    rtb_T_RR = fmax(rtb_T_cut_RL, rtb_Integrator_c);

    /* Abs: '<Root>/Abs' incorporates:
     *  Abs: '<S8>/Abs1'
     *  Gain: '<Root>/Gain'
     *  Inport: '<Root>/D_rad'
     */
    delta_Fz_lat = fabs(240.64227395494575 * driveController_U.Drad);

    /* RelationalOperator: '<S1>/Compare' incorporates:
     *  Abs: '<Root>/Abs'
     *  Constant: '<S1>/Constant'
     */
    rtb_enable = (delta_Fz_lat <= 2.0);

    /* Switch: '<Root>/Switch3' */
    if (rtb_enable) {
      rtb_T_RL = rtb_T_RR;
    } else {
      rtb_T_RL = rtb_T_cut_RL;
    }

    /* End of Switch: '<Root>/Switch3' */

    /* Saturate: '<S2>/Saturation1' incorporates:
     *  Inport: '<Root>/Throttle_%'
     */
    if (driveController_U.Throttle_ > 100.0) {
      /* DiscreteIntegrator: '<S155>/Filter' */
      driveController_B.Switch1 = 100.0;
    } else if (driveController_U.Throttle_ < 0.0) {
      /* DiscreteIntegrator: '<S155>/Filter' */
      driveController_B.Switch1 = 0.0;
    } else {
      /* DiscreteIntegrator: '<S155>/Filter' */
      driveController_B.Switch1 = driveController_U.Throttle_;
    }

    /* End of Saturate: '<S2>/Saturation1' */

    /* Gain: '<S2>/Gain5' incorporates:
     *  DiscreteIntegrator: '<S155>/Filter'
     *  Lookup_n-D: '<S2>/1-D Lookup Table'
     */
    omega_safe = 100.0 * look1_binlcapw(driveController_B.Switch1,
      driveController_ConstP.uDLookupTable_bp01Data,
      driveController_ConstP.uDLookupTable_tableData, 10U);

    /* RateLimiter: '<S2>/Rate Limiter1' */
    T_L = omega_safe - driveController_DW.PrevY;
    if (T_L > 10.0) {
      omega_safe = driveController_DW.PrevY + 10.0;
    } else if (T_L < -2.5) {
      omega_safe = driveController_DW.PrevY - 2.5;
    }

    driveController_DW.PrevY = omega_safe;

    /* End of RateLimiter: '<S2>/Rate Limiter1' */

    /* RateLimiter: '<S2>/Rate Limiter2' */
    T_L = driveController_B.Switch1 - driveController_DW.PrevY_j;
    if (T_L > 10.0) {
      /* DiscreteIntegrator: '<S155>/Filter' */
      driveController_B.Switch1 = driveController_DW.PrevY_j + 10.0;
    } else if (T_L < -2.5) {
      /* DiscreteIntegrator: '<S155>/Filter' */
      driveController_B.Switch1 = driveController_DW.PrevY_j - 2.5;
    }

    driveController_DW.PrevY_j = driveController_B.Switch1;

    /* End of RateLimiter: '<S2>/Rate Limiter2' */

    /* Switch: '<S2>/Switch1' incorporates:
     *  Constant: '<S2>/Constant'
     *  Inport: '<Root>/Throttle_%'
     */
    if (driveController_U.Throttle_ > 5.0) {
      /* Switch: '<S2>/Switch' */
      if (rtb_Max > 5.0) {
        /* DiscreteIntegrator: '<S155>/Filter' */
        driveController_B.Switch1 = omega_safe;
      }

      /* Saturate: '<S2>/Saturation' incorporates:
       *  Switch: '<S2>/Switch'
       */
      if (driveController_B.Switch1 > 100.0) {
        rtb_Throttle_l = 100.0;
      } else if (driveController_B.Switch1 < 0.0) {
        rtb_Throttle_l = 0.0;
      } else {
        rtb_Throttle_l = driveController_B.Switch1;
      }

      /* End of Saturate: '<S2>/Saturation' */
    } else {
      rtb_Throttle_l = 0.0;
    }

    /* End of Switch: '<S2>/Switch1' */

    /* MATLAB Function: '<Root>/MATLAB Function' incorporates:
     *  Inport: '<Root>/ay'
     *  Inport: '<Root>/n_motorRL'
     *  Inport: '<Root>/n_motorRR'
     */
    speed_avg_rpm = (fabs(driveController_U.n_motorRL) + fabs
                     (driveController_U.n_motorRR)) * 0.5;
    if (!(rtb_Throttle_l <= 5.0) || !(speed_avg_rpm > 1000.0) || !(rtb_Max > 0.5))
    {
      driveController_DW.t_elapsed_s = 0.0;
      omega_safe = 0.0;
      rtb_regen_active = 0;
    } else {
      driveController_DW.t_elapsed_s += 0.01;
      if ((driveController_U.n_motorRL < 4000.0) || (driveController_U.n_motorRR
           < 4000.0)) {
        driveController_DW.t_elapsed_s = 0.0;
        omega_safe = 0.0;
        rtb_regen_active = 0;
      } else {
        omega_safe = 6.283185307179586 * speed_avg_rpm / 60.0;
        if (speed_avg_rpm >= 4000.0) {
          speed_avg_rpm = 1.0;
        } else {
          speed_avg_rpm /= 4000.0;
        }

        if (driveController_DW.t_elapsed_s < 4.0) {
          rtb_regen_active = 21;
        } else {
          rtb_regen_active = 14;
        }

        omega_safe = fmax(fmax((1.0 - rtb_Throttle_l / 5.0) * -9.0 *
          speed_avg_rpm, -(17500.0 / omega_safe)), -((double)rtb_regen_active *
          0.5 * 540.0 / omega_safe)) * fmax(0.0, 1.0 - fabs(driveController_U.ay)
          / 15.0);
        rtb_regen_active = 1;
      }
    }

    /* End of MATLAB Function: '<Root>/MATLAB Function' */

    /* RateLimiter: '<Root>/Rate Limiter3' */
    T_L = omega_safe - driveController_DW.PrevY_p;
    if (T_L > 0.6) {
      omega_safe = driveController_DW.PrevY_p + 0.6;
    } else if (T_L < -0.2) {
      omega_safe = driveController_DW.PrevY_p - 0.2;
    }

    driveController_DW.PrevY_p = omega_safe;

    /* End of RateLimiter: '<Root>/Rate Limiter3' */

    /* Switch: '<S5>/Switch' incorporates:
     *  Constant: '<S5>/Constant'
     *  Constant: '<S5>/Constant1'
     *  Inport: '<Root>/Derating_RL'
     *  Inport: '<Root>/Derating_RR'
     *  Logic: '<S5>/Logical Operator'
     */
    if (driveController_U.Derating_RL || driveController_U.Derating_RR) {
      speed_avg_rpm = 9.8;
    } else {
      speed_avg_rpm = 21.0;
    }

    /* End of Switch: '<S5>/Switch' */

    /* RateLimiter: '<S5>/Rate Limiter' */
    T_L = speed_avg_rpm - driveController_DW.PrevY_a;
    if (T_L > 2.5) {
      speed_avg_rpm = driveController_DW.PrevY_a + 2.5;
    } else if (T_L < -2.5) {
      speed_avg_rpm = driveController_DW.PrevY_a - 2.5;
    }

    driveController_DW.PrevY_a = speed_avg_rpm;

    /* End of RateLimiter: '<S5>/Rate Limiter' */

    /* DiscreteIntegrator: '<S155>/Filter' incorporates:
     *  Gain: '<S5>/Gain'
     *  MinMax: '<S5>/Min'
     */
    driveController_B.Switch1 = fmin(0.21 * rtb_Throttle_l, speed_avg_rpm);

    /* RateLimiter: '<S5>/Rate Limiter1' */
    T_L = driveController_B.Switch1 - driveController_DW.PrevY_f;
    if (T_L > 2.5) {
      /* DiscreteIntegrator: '<S155>/Filter' */
      driveController_B.Switch1 = driveController_DW.PrevY_f + 2.5;
    } else if (T_L < -0.1) {
      /* DiscreteIntegrator: '<S155>/Filter' */
      driveController_B.Switch1 = driveController_DW.PrevY_f - 0.1;
    }

    driveController_DW.PrevY_f = driveController_B.Switch1;

    /* End of RateLimiter: '<S5>/Rate Limiter1' */

    /* Switch: '<Root>/Switch1' */
    if (rtb_regen_active > 0) {
      /* DiscreteIntegrator: '<S155>/Filter' incorporates:
       *  Switch: '<Root>/Switch1'
       */
      driveController_B.Switch1 = omega_safe;
    }

    /* End of Switch: '<Root>/Switch1' */

    /* Trigonometry: '<S9>/Trigonometric Function' incorporates:
     *  Inport: '<Root>/D_rad'
     */
    speed_avg_rpm = tan(driveController_U.Drad);

    /* MATLAB Function: '<S9>/tand safe' */
    if (fabs(speed_avg_rpm) < 0.017455064928217585) {
      speed_avg_rpm = 0.0;
    }

    /* MATLAB Function: '<S9>/Bicycle Model Dynamic' incorporates:
     *  MATLAB Function: '<Root>/MATLAB Function1'
     *  MATLAB Function: '<S9>/tand safe'
     */
    Fz_RL = rtb_Max * rtb_Max;
    omega_safe = rtb_Max / 1.55 * speed_avg_rpm / (Fz_RL * 0.03 + 1.0);

    /* RateLimiter: '<S9>/Rate Lmiter' */
    T_L = omega_safe - driveController_DW.PrevY_e;
    if (T_L > driveController_period) {
      omega_safe = driveController_DW.PrevY_e + driveController_period;
    } else if (T_L < -0.01) {
      omega_safe = driveController_DW.PrevY_e - 0.01;
    }

    driveController_DW.PrevY_e = omega_safe;

    /* End of RateLimiter: '<S9>/Rate Lmiter' */

    /* MATLAB Function: '<S9>/yaw_saturation' */
    if (rtIsNaN(omega_safe)) {
      speed_avg_rpm = (rtNaN);
    } else if (omega_safe < 0.0) {
      speed_avg_rpm = -1.0;
    } else {
      speed_avg_rpm = (omega_safe > 0.0);
    }

    omega_safe = fmin(fabs(omega_safe), 14.715 / fmax(rtb_Max, 0.5)) *
      speed_avg_rpm;

    /* End of MATLAB Function: '<S9>/yaw_saturation' */

    /* Sum: '<Root>/Sum' incorporates:
     *  Inport: '<Root>/Yaw_rate_act'
     */
    speed_avg_rpm = omega_safe - driveController_U.yaw_meas;

    /* DeadZone: '<S8>/Dead Zone' */
    if (speed_avg_rpm > 0.03) {
      speed_avg_rpm -= 0.03;
    } else if (speed_avg_rpm >= -0.03) {
      speed_avg_rpm = 0.0;
    } else {
      speed_avg_rpm -= -0.03;
    }

    /* End of DeadZone: '<S8>/Dead Zone' */

    /* RelationalOperator: '<S121>/Compare' incorporates:
     *  Constant: '<S121>/Constant'
     */
    rtb_Compare_i = (speed_avg_rpm == 0.0);

    /* DiscreteIntegrator: '<S160>/Integrator' incorporates:
     *  Memory: '<S8>/Memory'
     */
    if (driveController_DW.Integrator_IC_LOADING != 0) {
      driveController_DW.Integrator_DSTATE_a =
        driveController_DW.Memory_PreviousInput;
    }

    if (rtb_Compare_i || (driveController_DW.Integrator_PrevResetState_g != 0))
    {
      driveController_DW.Integrator_DSTATE_a =
        driveController_DW.Memory_PreviousInput;
    }

    /* DiscreteIntegrator: '<S155>/Filter' */
    if (rtb_Compare_i || (driveController_DW.Filter_PrevResetState_h != 0)) {
      driveController_DW.Filter_DSTATE_g = driveController_ConstB.Constant;
    }

    /* Gain: '<S163>/Filter Coefficient' incorporates:
     *  DiscreteIntegrator: '<S155>/Filter'
     *  Gain: '<S153>/Derivative Gain'
     *  Sum: '<S155>/SumD'
     */
    rtb_FilterCoefficient_na = (0.8 * speed_avg_rpm -
      driveController_DW.Filter_DSTATE_g) * 30.0;

    /* Sum: '<S169>/Sum' incorporates:
     *  DiscreteIntegrator: '<S160>/Integrator'
     *  Gain: '<S165>/Proportional Gain'
     */
    rtb_Sum = (700.0 * speed_avg_rpm + driveController_DW.Integrator_DSTATE_a) +
      rtb_FilterCoefficient_na;

    /* Saturate: '<S167>/Saturation' */
    if (rtb_Sum > 600.0) {
      rtb_Saturation = 600.0;
    } else if (rtb_Sum < -600.0) {
      rtb_Saturation = -600.0;
    } else {
      rtb_Saturation = rtb_Sum;
    }

    /* End of Saturate: '<S167>/Saturation' */

    /* Product: '<S8>/Product' incorporates:
     *  Lookup_n-D: '<Root>/1-D Lookup Table'
     *  Lookup_n-D: '<S8>/1-D Lookup Table'
     *  MATLAB Function: '<S8>/MATLAB Function'
     *  Saturate: '<Root>/Saturation'
     */
    driveController_B.dT_real = rtb_Saturation * 0.2032 / 14.4 * look1_binlcapw
      (rtb_T_RR, driveController_ConstP.uDLookupTable_bp01Data_b,
       driveController_ConstP.uDLookupTable_tableData_j, 2U) * look1_binlcapw
      (delta_Fz_lat, driveController_ConstP.uDLookupTable_bp01Data_m,
       driveController_ConstP.uDLookupTable_tableData_b, 2U);

    /* MATLAB Function: '<Root>/MATLAB Function1' incorporates:
     *  Inport: '<Root>/ax'
     *  Inport: '<Root>/ay'
     */
    delta_Fz_lat = 320.0 * driveController_U.ay * 0.35 / 1.2 * 0.45;
    T_L = (Fz_RL * 0.6737500000000001 * 0.5 + 1569.6000000000001) / 2.0 + 320.0 *
      driveController_U.ax * 0.35 / 1.55 * 0.5;
    Fz_RL = T_L + delta_Fz_lat;
    delta_Fz_lat = T_L - delta_Fz_lat;
    if (Fz_RL < 0.0) {
      Fz_RL = 0.0;
    }

    if (delta_Fz_lat < 0.0) {
      delta_Fz_lat = 0.0;
    }

    driveController_B.T_max_RL = Fz_RL * 1.5 * 0.2032 / 12.0 * 1.01;
    driveController_B.T_max_RR = delta_Fz_lat * 1.5 * 0.2032 / 12.0 * 1.01;
  }

  if (driveController_M->Timing.TaskCounters.TID[2] == 0) {
    /* MATLAB Function: '<S8>/TorqueAllocator' incorporates:
     *  Inport: '<Root>/D_rad'
     *  Inport: '<Root>/n_motorRL'
     *  Inport: '<Root>/n_motorRR'
     */
    Fz_RL = V_dc_p * 58.33;
    P_tot_req = fabs(driveController_U.n_motorRL);
    w_RL = fmax(P_tot_req * 0.10471975511965977, 1.0);
    w_RR_tmp = fabs(driveController_U.n_motorRR);
    w_RR = fmax(w_RR_tmp * 0.10471975511965977, 1.0);
    delta_Fz_lat = fmin(fmin(21.0, Fz_RL / w_RL), driveController_B.T_max_RL);
    Fz_RL = fmin(fmin(21.0, Fz_RL / w_RR), driveController_B.T_max_RR);
    if (driveController_B.Switch1 >= 0.0) {
      T_req_eff = fmax(0.0, fmin(driveController_B.Switch1, fmin(delta_Fz_lat,
        Fz_RL) - fmin(4.5, 8.0 * fabs(driveController_U.Drad))));
    } else {
      T_req_eff = fmax(fmin(4.5, 8.0 * fabs(driveController_U.Drad)) + -fmin
                       (driveController_B.T_max_RL, driveController_B.T_max_RR),
                       fmin(0.0, driveController_B.Switch1));
    }

    T_L = T_req_eff - driveController_B.dT_real;
    T_req_eff += driveController_B.dT_real;
    if ((P_tot_req < 100.0) && (T_L < 0.0)) {
      T_L = 0.0;
    }

    if ((w_RR_tmp < 100.0) && (T_req_eff < 0.0)) {
      T_req_eff = 0.0;
    }

    P_tot_req = fmax(0.0, T_L * w_RL) + fmax(0.0, T_req_eff * w_RR);
    w_RL = V_dc_p * 80.0;
    if (P_tot_req > w_RL) {
      P_tot_req = w_RL / P_tot_req;
      T_L *= P_tot_req;
      T_req_eff *= P_tot_req;
    }

    driveController_B.T_RL_cmd = fmax(-driveController_B.T_max_RL, fmin
      (delta_Fz_lat, T_L));
    driveController_B.T_RR_cmd = fmax(-driveController_B.T_max_RR, fmin(Fz_RL,
      T_req_eff));

    /* End of MATLAB Function: '<S8>/TorqueAllocator' */
  }

  /* MATLAB Function: '<S8>/MATLAB Function1' incorporates:
   *  Inport: '<Root>/T_RL_act'
   *  Inport: '<Root>/T_RR_act'
   *  Sum: '<S8>/Sum3'
   */
  driveController_DW.Memory_PreviousInput = (driveController_U.T_RR_act -
    driveController_U.T_RL_act) * 1.2 * 15.0 * 0.8 / 0.4064;
  if (tmp) {
    /* Sum: '<Root>/Add' */
    rtb_T_RL = driveController_B.T_RL_cmd - rtb_T_RL;

    /* RateLimiter: '<Root>/Rate Limiter1' */
    T_L = rtb_T_RL - driveController_DW.PrevY_c;
    if (T_L > 1.0) {
      rtb_T_RL = driveController_DW.PrevY_c + 1.0;
    } else if (T_L < -5.0) {
      rtb_T_RL = driveController_DW.PrevY_c - 5.0;
    }

    driveController_DW.PrevY_c = rtb_T_RL;

    /* End of RateLimiter: '<Root>/Rate Limiter1' */

    /* Saturate: '<Root>/Saturation1' */
    if (rtb_T_RL > 21.0) {
      /* Outport: '<Root>/T_RL' */
      driveController_Y.T_RL = 21.0;
    } else if (rtb_T_RL < -9.0) {
      /* Outport: '<Root>/T_RL' */
      driveController_Y.T_RL = -9.0;
    } else {
      /* Outport: '<Root>/T_RL' */
      driveController_Y.T_RL = rtb_T_RL;
    }

    /* End of Saturate: '<Root>/Saturation1' */

    /* Switch: '<Root>/Switch' */
    if (!rtb_enable) {
      rtb_T_RR = rtb_Integrator_c;
    }

    /* Sum: '<Root>/Add1' incorporates:
     *  Switch: '<Root>/Switch'
     */
    rtb_T_RR = driveController_B.T_RR_cmd - rtb_T_RR;

    /* RateLimiter: '<Root>/Rate Limiter2' */
    T_L = rtb_T_RR - driveController_DW.PrevY_i;
    if (T_L > 1.0) {
      rtb_T_RR = driveController_DW.PrevY_i + 1.0;
    } else if (T_L < -5.0) {
      rtb_T_RR = driveController_DW.PrevY_i - 5.0;
    }

    driveController_DW.PrevY_i = rtb_T_RR;

    /* End of RateLimiter: '<Root>/Rate Limiter2' */

    /* Saturate: '<Root>/Saturation' */
    if (rtb_T_RR > 21.0) {
      /* Outport: '<Root>/T_RR' */
      driveController_Y.T_RR = 21.0;
    } else if (rtb_T_RR < -9.0) {
      /* Outport: '<Root>/T_RR' */
      driveController_Y.T_RR = -9.0;
    } else {
      /* Outport: '<Root>/T_RR' */
      driveController_Y.T_RR = rtb_T_RR;
    }

    /* End of Saturate: '<Root>/Saturation' */

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
    T_L = (double)rtb_TVC_Target_Weight - driveController_DW.PrevY_ck;
    if (T_L > 0.02) {
      /* RateLimiter: '<Root>/W_RateLimiter' */
      driveController_DW.PrevY_ck += 0.02;
    } else if (T_L < -0.02) {
      /* RateLimiter: '<Root>/W_RateLimiter' */
      driveController_DW.PrevY_ck -= 0.02;
    } else {
      /* RateLimiter: '<Root>/W_RateLimiter' */
      driveController_DW.PrevY_ck = rtb_TVC_Target_Weight;
    }

    /* End of RateLimiter: '<Root>/W_RateLimiter' */

    /* Outport: '<Root>/TVC_Target_Weight' */
    driveController_Y.TVC_Target_Weight = driveController_DW.PrevY_ck;

    /* Outport: '<Root>/yaw_th' */
    driveController_Y.yaw_th = omega_safe;

    /* Outport: '<Root>/regen' */
    driveController_Y.regen = rtb_regen_active;

    /* Outport: '<Root>/Throttle_real' */
    driveController_Y.Throttle_real = rtb_Throttle_l;

    /* Outport: '<Root>/K_slips' */
    driveController_Y.K_slips[0] = rtb_T_cut_RL;
    driveController_Y.K_slips[1] = rtb_Integrator_c;

    /* Update for DiscreteIntegrator: '<S103>/Integrator' */
    driveController_DW.Integrator_DSTATE += 0.01 * rtb_Integrator;
    if (driveController_DW.Integrator_DSTATE > 25.0) {
      driveController_DW.Integrator_DSTATE = 25.0;
    } else if (driveController_DW.Integrator_DSTATE < 0.0) {
      driveController_DW.Integrator_DSTATE = 0.0;
    }

    driveController_DW.Integrator_PrevResetState = (int8_t)rtb_LogicalOperator2;

    /* Update for DiscreteIntegrator: '<S98>/Filter' incorporates:
     *  DiscreteIntegrator: '<S103>/Integrator'
     */
    driveController_DW.Filter_DSTATE += 0.01 * rtb_Filter;
    driveController_DW.Filter_PrevResetState = (int8_t)rtb_LogicalOperator2;

    /* Update for DiscreteIntegrator: '<S51>/Integrator' incorporates:
     *  Gain: '<S43>/Kb'
     *  Gain: '<S48>/Integral Gain'
     *  Sum: '<S43>/SumI2'
     *  Sum: '<S43>/SumI4'
     */
    driveController_DW.Integrator_DSTATE_o += ((rtb_Saturation_g - rtb_Sum_h) *
      10.0 + 13.0 * rtb_e_RL_i) * 0.01;
    if (driveController_DW.Integrator_DSTATE_o > 25.0) {
      driveController_DW.Integrator_DSTATE_o = 25.0;
    } else if (driveController_DW.Integrator_DSTATE_o < 0.0) {
      driveController_DW.Integrator_DSTATE_o = 0.0;
    }

    driveController_DW.Integrator_PrevResetState_a = (int8_t)
      rtb_LogicalOperator1;

    /* Update for DiscreteIntegrator: '<S46>/Filter' incorporates:
     *  DiscreteIntegrator: '<S51>/Integrator'
     */
    driveController_DW.Filter_DSTATE_e += 0.01 * rtb_FilterCoefficient_l;
    driveController_DW.Filter_PrevResetState_p = (int8_t)rtb_LogicalOperator1;

    /* Update for DiscreteIntegrator: '<S160>/Integrator' incorporates:
     *  Gain: '<S152>/Kb'
     *  Gain: '<S157>/Integral Gain'
     *  Gain: '<S171>/Kt'
     *  Sum: '<S152>/SumI2'
     *  Sum: '<S152>/SumI4'
     *  Sum: '<S171>/SumI3'
     *  Sum: '<S172>/SumI1'
     */
    driveController_DW.Integrator_IC_LOADING = 0U;
    driveController_DW.Integrator_DSTATE_a +=
      (((driveController_DW.Memory_PreviousInput - rtb_Saturation) * 7.0 + 30.0 *
        speed_avg_rpm) + (rtb_Saturation - rtb_Sum) * 4.0) * 0.01;
    driveController_DW.Integrator_PrevResetState_g = (int8_t)rtb_Compare_i;

    /* Update for DiscreteIntegrator: '<S155>/Filter' incorporates:
     *  DiscreteIntegrator: '<S160>/Integrator'
     */
    driveController_DW.Filter_DSTATE_g += 0.01 * rtb_FilterCoefficient_na;
    driveController_DW.Filter_PrevResetState_h = (int8_t)rtb_Compare_i;
  }

  rate_scheduler();
}

/* Model initialize function */
void driveController_initialize(void)
{
  /* InitializeConditions for DiscreteIntegrator: '<S160>/Integrator' */
  driveController_DW.Integrator_IC_LOADING = 1U;

  /* InitializeConditions for DiscreteIntegrator: '<S155>/Filter' */
  driveController_DW.Filter_DSTATE_g = driveController_ConstB.Constant;
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
