/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: driveController.c
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

#include "driveController.h"
#include <stdint.h>
#include <math.h>
#include "rt_nonfinite.h"
#include <stdbool.h>
#include "zero_crossing_types.h"

/* Named constants for Chart: '<Root>/Mode Manager' */
#define driveController_IN_IDLE_       ((uint8_t)1U)
#define driveController_IN_SLC_        ((uint8_t)2U)
#define driveController_IN_TVC_        ((uint8_t)3U)

/* Block signals (default storage) */
B_driveController_T driveController_B;

/* Block states (default storage) */
DW_driveController_T driveController_DW;

/* Previous zero-crossings (trigger) states */
PrevZCX_driveController_T driveController_PrevZCX;

/* External inputs (root inport signals with default storage) */
ExtU_driveController_T driveController_U;

/* External outputs (root outports fed by signals with default storage) */
ExtY_driveController_T driveController_Y;

/* Real-time model */
static RT_MODEL_driveController_T driveController_M_;
RT_MODEL_driveController_T *const driveController_M = &driveController_M_;
static void rate_scheduler(void);

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
  double rtb_FilterCoefficient;
  double rtb_Saturation;
  double rtb_Saturation1;
  double rtb_Saturation_l;
  double rtb_Sum;
  double rtb_TrigonometricFunction;
  double rtb_Tsamp;
  double rtb_e_yaw;
  double rtb_yaw_th_i;
  int32_t rtb_TVC_Target_Weight;
  int8_t tmp;
  int8_t tmp_0;
  bool Compare;
  bool rtb_Compare;

  /* Trigonometry: '<S4>/Trigonometric Function' incorporates:
   *  Inport: '<Root>/Steering'
   */
  rtb_TrigonometricFunction = tan(driveController_U.Steering);

  /* MATLAB Function: '<S4>/tand safe' */
  if (fabs(rtb_TrigonometricFunction) < 0.03492076949174773) {
    rtb_TrigonometricFunction = 0.0;
  }

  /* MATLAB Function: '<S4>/Bicycle Model Dynamic' incorporates:
   *  Inport: '<Root>/V'
   *  MATLAB Function: '<S4>/tand safe'
   */
  rtb_yaw_th_i = driveController_U.V / 1.55 * rtb_TrigonometricFunction /
    (driveController_U.V * driveController_U.V * 0.0 + 1.0);

  /* RateLimiter: '<S4>/Rate Lmiter' */
  rtb_TrigonometricFunction = rtb_yaw_th_i - driveController_DW.PrevY;
  if (rtb_TrigonometricFunction > 0.003) {
    rtb_yaw_th_i = driveController_DW.PrevY + 0.003;
  } else if (rtb_TrigonometricFunction < -0.003) {
    rtb_yaw_th_i = driveController_DW.PrevY - 0.003;
  }

  driveController_DW.PrevY = rtb_yaw_th_i;

  /* End of RateLimiter: '<S4>/Rate Lmiter' */

  /* MATLAB Function: '<S4>/yaw_saturation' incorporates:
   *  Inport: '<Root>/V'
   */
  if (rtIsNaN(rtb_yaw_th_i)) {
    rtb_e_yaw = (rtNaN);
  } else if (rtb_yaw_th_i < 0.0) {
    rtb_e_yaw = -1.0;
  } else {
    rtb_e_yaw = (rtb_yaw_th_i > 0.0);
  }

  driveController_Y.yaw_th = fmin(fabs(rtb_yaw_th_i), 15.696000000000002 / fmax
    (driveController_U.V, 0.5)) * rtb_e_yaw;

  /* End of MATLAB Function: '<S4>/yaw_saturation' */

  /* Sum: '<Root>/Sum' incorporates:
   *  Inport: '<Root>/Yaw_meas'
   */
  rtb_e_yaw = driveController_Y.yaw_th - driveController_U.Yaw_meas;

  /* Chart: '<Root>/Mode Manager' incorporates:
   *  Inport: '<Root>/Steering'
   *  Inport: '<Root>/Throttle_%'
   *  Inport: '<Root>/V'
   *  Inport: '<Root>/brake'
   */
  if (driveController_DW.is_active_c5_driveController == 0) {
    driveController_DW.is_active_c5_driveController = 1U;
    driveController_DW.is_c5_driveController = driveController_IN_IDLE_;
    rtb_TVC_Target_Weight = 0;
  } else {
    switch (driveController_DW.is_c5_driveController) {
     case driveController_IN_IDLE_:
      rtb_TVC_Target_Weight = 0;
      if ((driveController_U.Throttle_ > 3.0) && (driveController_U.brake == 0.0))
      {
        driveController_DW.is_c5_driveController = driveController_IN_SLC_;
      }
      break;

     case driveController_IN_SLC_:
      rtb_TVC_Target_Weight = 0;
      if ((fabs(driveController_U.Steering) > 0.052) || (fabs(rtb_e_yaw) > 0.1))
      {
        rtb_Compare = ((driveController_U.V > 4.0) && (driveController_U.brake ==
          0.0));
      } else {
        rtb_Compare = false;
      }

      if (rtb_Compare) {
        driveController_DW.is_c5_driveController = driveController_IN_TVC_;
        rtb_TVC_Target_Weight = 1;
      } else if ((driveController_U.Throttle_ < 2.0) && (driveController_U.V <=
                  0.5)) {
        driveController_DW.is_c5_driveController = driveController_IN_IDLE_;
      }
      break;

     default:
      /* case IN_TVC_: */
      rtb_TVC_Target_Weight = 1;
      if ((fabs(driveController_U.Steering) < 0.03) && (fabs(rtb_e_yaw) < 0.05))
      {
        rtb_Compare = true;
      } else {
        rtb_Compare = (driveController_U.V < 2.0);
      }

      if (rtb_Compare) {
        driveController_DW.is_c5_driveController = driveController_IN_SLC_;
        rtb_TVC_Target_Weight = 0;
      } else if ((driveController_U.brake > 0.0) || (driveController_U.V <= 0.5))
      {
        driveController_DW.is_c5_driveController = driveController_IN_IDLE_;
        rtb_TVC_Target_Weight = 0;
      }
      break;
    }
  }

  /* End of Chart: '<Root>/Mode Manager' */

  /* RateLimiter: '<Root>/W_RateLimiter' */
  rtb_TrigonometricFunction = (double)rtb_TVC_Target_Weight -
    driveController_DW.PrevY_d;
  if (rtb_TrigonometricFunction > 0.02) {
    /* RateLimiter: '<Root>/W_RateLimiter' */
    driveController_DW.PrevY_d += 0.02;
  } else if (rtb_TrigonometricFunction < -0.02) {
    /* RateLimiter: '<Root>/W_RateLimiter' */
    driveController_DW.PrevY_d -= 0.02;
  } else {
    /* RateLimiter: '<Root>/W_RateLimiter' */
    driveController_DW.PrevY_d = rtb_TVC_Target_Weight;
  }

  /* End of RateLimiter: '<Root>/W_RateLimiter' */

  /* Outputs for Atomic SubSystem: '<Root>/SLC' */
  /* Sum: '<S2>/Sum7' incorporates:
   *  Inport: '<Root>/T_RL_act'
   *  Inport: '<Root>/T_RR_act'
   *  Sum: '<S3>/Sum3'
   */
  rtb_TrigonometricFunction = driveController_U.T_RL_act -
    driveController_U.T_RR_act;

  /* Product: '<S2>/AW_err_x_OMW' incorporates:
   *  Constant: '<Root>/1'
   *  Sum: '<Root>/OneMinusW'
   *  Sum: '<S2>/Sum7'
   */
  rtb_yaw_th_i = (1.0 - driveController_DW.PrevY_d) * rtb_TrigonometricFunction;

  /* RelationalOperator: '<S5>/Compare' incorporates:
   *  Constant: '<Root>/1'
   *  Constant: '<S5>/Constant'
   *  Sum: '<Root>/OneMinusW'
   */
  rtb_Compare = (1.0 - driveController_DW.PrevY_d <= 0.05);

  /* DiscreteIntegrator: '<S45>/Integrator' */
  if (rtb_Compare || (driveController_DW.Integrator_PrevResetState_a != 0)) {
    driveController_DW.Integrator_DSTATE_o = 0.0;
  }

  /* SampleTimeMath: '<S40>/Tsamp' incorporates:
   *  Gain: '<S36>/Derivative Gain'
   *
   * About '<S40>/Tsamp':
   *  y = u * K where K = 1 / ( w * Ts )
   *   */
  rtb_Tsamp = 0.0 * rtb_yaw_th_i * 100.0;

  /* Delay: '<S38>/UD' */
  if ((((driveController_PrevZCX.UD_Reset_ZCE == POS_ZCSIG) != (int32_t)
        rtb_Compare) && (driveController_PrevZCX.UD_Reset_ZCE !=
                         UNINITIALIZED_ZCSIG)) || rtb_Compare) {
    driveController_DW.UD_DSTATE = 0.0;
  }

  driveController_PrevZCX.UD_Reset_ZCE = rtb_Compare;

  /* Sum: '<S54>/Sum' incorporates:
   *  Delay: '<S38>/UD'
   *  DiscreteIntegrator: '<S45>/Integrator'
   *  Gain: '<S50>/Proportional Gain'
   *  Sum: '<S38>/Diff'
   */
  rtb_Saturation1 = (0.457 * rtb_yaw_th_i +
                     driveController_DW.Integrator_DSTATE_o) + (rtb_Tsamp -
    driveController_DW.UD_DSTATE);

  /* DeadZone: '<S35>/DeadZone' */
  if (rtb_Saturation1 > 17.0) {
    rtb_Saturation_l = rtb_Saturation1 - 17.0;
  } else if (rtb_Saturation1 >= -17.0) {
    rtb_Saturation_l = 0.0;
  } else {
    rtb_Saturation_l = rtb_Saturation1 - -17.0;
  }

  /* End of DeadZone: '<S35>/DeadZone' */

  /* Gain: '<S42>/Integral Gain' */
  rtb_yaw_th_i *= 9.01;

  /* Switch: '<S33>/Switch1' incorporates:
   *  Constant: '<S33>/Clamping_zero'
   *  Constant: '<S33>/Constant'
   *  Constant: '<S33>/Constant2'
   *  RelationalOperator: '<S33>/fix for DT propagation issue'
   */
  if (rtb_Saturation_l > 0.0) {
    tmp = 1;
  } else {
    tmp = -1;
  }

  /* Switch: '<S33>/Switch2' incorporates:
   *  Constant: '<S33>/Clamping_zero'
   *  Constant: '<S33>/Constant3'
   *  Constant: '<S33>/Constant4'
   *  RelationalOperator: '<S33>/fix for DT propagation issue1'
   */
  if (rtb_yaw_th_i > 0.0) {
    tmp_0 = 1;
  } else {
    tmp_0 = -1;
  }

  /* Switch: '<S33>/Switch' incorporates:
   *  Constant: '<S33>/Clamping_zero'
   *  Constant: '<S33>/Constant1'
   *  Logic: '<S33>/AND3'
   *  RelationalOperator: '<S33>/Equal1'
   *  RelationalOperator: '<S33>/Relational Operator'
   *  Switch: '<S33>/Switch1'
   *  Switch: '<S33>/Switch2'
   */
  if ((rtb_Saturation_l != 0.0) && (tmp == tmp_0)) {
    rtb_Saturation_l = 0.0;
  } else {
    rtb_Saturation_l = rtb_yaw_th_i;
  }

  /* End of Switch: '<S33>/Switch' */

  /* Saturate: '<S52>/Saturation' */
  if (rtb_Saturation1 > 17.0) {
    rtb_Saturation1 = 17.0;
  } else if (rtb_Saturation1 < -17.0) {
    rtb_Saturation1 = -17.0;
  }

  /* End of Saturate: '<S52>/Saturation' */

  /* Gain: '<S2>/Gain' incorporates:
   *  Inport: '<Root>/Throttle_%'
   */
  rtb_yaw_th_i = 0.21 * driveController_U.Throttle_;

  /* Update for DiscreteIntegrator: '<S45>/Integrator' */
  driveController_DW.Integrator_DSTATE_o += 0.01 * rtb_Saturation_l;
  driveController_DW.Integrator_PrevResetState_a = (int8_t)rtb_Compare;

  /* Update for Delay: '<S38>/UD' */
  driveController_DW.UD_DSTATE = rtb_Tsamp;

  /* End of Outputs for SubSystem: '<Root>/SLC' */

  /* Product: '<S3>/AW_eyaw_x_W' */
  rtb_Tsamp = rtb_e_yaw * driveController_DW.PrevY_d;
  rtb_Compare = (driveController_M->Timing.TaskCounters.TID[1] == 0);

  /* RelationalOperator: '<S63>/Compare' incorporates:
   *  Constant: '<S63>/Constant'
   */
  Compare = (driveController_DW.PrevY_d <= 0.05);
  if (rtb_Compare) {
    /* DiscreteIntegrator: '<S102>/Integrator' incorporates:
     *  Memory: '<S3>/Memory'
     */
    if (driveController_DW.Integrator_IC_LOADING != 0) {
      driveController_DW.Integrator_DSTATE =
        driveController_DW.Memory_PreviousInput;
    }

    if (Compare || (driveController_DW.Integrator_PrevResetState != 0)) {
      driveController_DW.Integrator_DSTATE =
        driveController_DW.Memory_PreviousInput;
    }

    /* DiscreteIntegrator: '<S97>/Filter' */
    if (Compare || (driveController_DW.Filter_PrevResetState != 0)) {
      driveController_DW.Filter_DSTATE = driveController_ConstB.Constant;
    }

    /* Gain: '<S105>/Filter Coefficient' incorporates:
     *  DiscreteIntegrator: '<S97>/Filter'
     *  Gain: '<S95>/Derivative Gain'
     *  Sum: '<S97>/SumD'
     */
    rtb_FilterCoefficient = (0.0 * rtb_Tsamp - driveController_DW.Filter_DSTATE)
      * 10.0;

    /* Sum: '<S111>/Sum' incorporates:
     *  DiscreteIntegrator: '<S102>/Integrator'
     *  Gain: '<S107>/Proportional Gain'
     */
    rtb_Sum = (25.0 * rtb_Tsamp + driveController_DW.Integrator_DSTATE) +
      rtb_FilterCoefficient;

    /* Saturate: '<S109>/Saturation' */
    if (rtb_Sum > 35.0) {
      rtb_Saturation = 35.0;
    } else if (rtb_Sum < -35.0) {
      rtb_Saturation = -35.0;
    } else {
      rtb_Saturation = rtb_Sum;
    }

    /* End of Saturate: '<S109>/Saturation' */

    /* MATLAB Function: '<S3>/MATLAB Function' */
    driveController_B.dT = rtb_Saturation * 0.225 / 1.2;
  }

  /* MATLAB Function: '<S3>/TorqueAllocator' incorporates:
   *  Gain: '<S3>/Gain'
   *  Inport: '<Root>/Steering'
   *  Inport: '<Root>/T_RL_max'
   *  Inport: '<Root>/T_RR_max'
   */
  rtb_Saturation_l = fmax(0.0, fmin(rtb_yaw_th_i, fmin
    (driveController_U.T_RL_max, driveController_U.T_RR_max) - fmin(5.0, 8.0 *
    fabs(driveController_U.Steering))));

  /* Outputs for Atomic SubSystem: '<Root>/SLC' */
  /* Saturate: '<S2>/Saturation1' */
  if (rtb_Saturation1 <= 0.0) {
    rtb_e_yaw = 0.0;
  } else {
    rtb_e_yaw = rtb_Saturation1;
  }

  /* Outport: '<Root>/T_RL' incorporates:
   *  Constant: '<Root>/1'
   *  Inport: '<Root>/T_RL_max'
   *  MATLAB Function: '<S3>/TorqueAllocator'
   *  Product: '<Root>/Prod_SLC_RL'
   *  Product: '<Root>/Prod_TVC_RL'
   *  Saturate: '<S2>/Saturation1'
   *  Sum: '<Root>/OneMinusW'
   *  Sum: '<Root>/Sum_T_RL'
   *  Sum: '<S2>/Sum8'
   */
  driveController_Y.T_RL = (rtb_yaw_th_i - rtb_e_yaw) * (1.0 -
    driveController_DW.PrevY_d) + fmax(0.0, fmin(driveController_U.T_RL_max,
    rtb_Saturation_l + driveController_B.dT)) * driveController_DW.PrevY_d;

  /* Saturate: '<S2>/Saturation' incorporates:
   *  Gain: '<S2>/Gain1'
   */
  if (-rtb_Saturation1 <= 0.0) {
    rtb_e_yaw = 0.0;
  } else {
    rtb_e_yaw = -rtb_Saturation1;
  }

  /* Outport: '<Root>/T_RR' incorporates:
   *  Constant: '<Root>/1'
   *  Inport: '<Root>/T_RR_max'
   *  MATLAB Function: '<S3>/TorqueAllocator'
   *  Product: '<Root>/Prod_SLC_RR'
   *  Product: '<Root>/Prod_TVC_RR'
   *  Saturate: '<S2>/Saturation'
   *  Sum: '<Root>/OneMinusW'
   *  Sum: '<Root>/Sum_T_RR'
   *  Sum: '<S2>/Sum9'
   */
  driveController_Y.T_RR = (rtb_yaw_th_i - rtb_e_yaw) * (1.0 -
    driveController_DW.PrevY_d) + fmax(0.0, fmin(driveController_U.T_RR_max,
    rtb_Saturation_l - driveController_B.dT)) * driveController_DW.PrevY_d;

  /* End of Outputs for SubSystem: '<Root>/SLC' */

  /* MATLAB Function: '<S3>/MATLAB Function1' */
  driveController_DW.Memory_PreviousInput = rtb_TrigonometricFunction * 1.2 /
    0.45;
  if (rtb_Compare) {
    /* Update for DiscreteIntegrator: '<S102>/Integrator' incorporates:
     *  Gain: '<S113>/Kt'
     *  Gain: '<S99>/Integral Gain'
     *  Sum: '<S113>/SumI3'
     *  Sum: '<S114>/SumI1'
     *  Sum: '<S94>/SumI2'
     *  Sum: '<S94>/SumI4'
     */
    driveController_DW.Integrator_IC_LOADING = 0U;
    driveController_DW.Integrator_DSTATE +=
      (((driveController_DW.Memory_PreviousInput - rtb_Saturation) * 1.4 + 50.0 *
        rtb_Tsamp) + (rtb_Saturation - rtb_Sum)) * 0.05;
    driveController_DW.Integrator_PrevResetState = (int8_t)Compare;

    /* Update for DiscreteIntegrator: '<S97>/Filter' incorporates:
     *  DiscreteIntegrator: '<S102>/Integrator'
     */
    driveController_DW.Filter_DSTATE += 0.05 * rtb_FilterCoefficient;
    driveController_DW.Filter_PrevResetState = (int8_t)Compare;
  }

  /* Outport: '<Root>/TVC_Target_Weight' */
  driveController_Y.TVC_Target_Weight = driveController_DW.PrevY_d;
  rate_scheduler();
}

/* Model initialize function */
void driveController_initialize(void)
{
  driveController_PrevZCX.UD_Reset_ZCE = UNINITIALIZED_ZCSIG;

  /* InitializeConditions for DiscreteIntegrator: '<S102>/Integrator' */
  driveController_DW.Integrator_IC_LOADING = 1U;

  /* InitializeConditions for DiscreteIntegrator: '<S97>/Filter' */
  driveController_DW.Filter_DSTATE = driveController_ConstB.Constant;
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
