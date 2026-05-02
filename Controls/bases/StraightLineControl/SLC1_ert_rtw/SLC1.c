/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: SLC1.c
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

#include "SLC1.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* Model step function */
void SLC1_step(RT_MODEL_SLC1_T *const SLC1_M, double SLC1_U_T_driver, double
               SLC1_U_T_RL, double SLC1_U_T_RR, bool SLC1_U_Enable, double
               *SLC1_Y_T_reqRL, double *SLC1_Y_T_reqRR)
{
  DW_SLC1_T *SLC1_DW = SLC1_M->dwork;
  double rtb_IntegralGain;
  double rtb_Saturation;
  double rtb_Saturation1;
  double rtb_Tsamp;
  double tmp;
  int8_t tmp_0;
  int8_t tmp_1;

  /* Outputs for Enabled SubSystem: '<Root>/SLC1' incorporates:
   *  EnablePort: '<S1>/Enable'
   */
  /* Inport: '<Root>/Enable' */
  if (SLC1_U_Enable) {
    /* Sum: '<S1>/Sum7' incorporates:
     *  Inport: '<Root>/T_RL_fb'
     *  Inport: '<Root>/T_RR_fb'
     */
    rtb_IntegralGain = SLC1_U_T_RL - SLC1_U_T_RR;

    /* SampleTimeMath: '<S36>/Tsamp' incorporates:
     *  Gain: '<S32>/Derivative Gain'
     *
     * About '<S36>/Tsamp':
     *  y = u * K where K = 1 / ( w * Ts )
     *   */
    rtb_Tsamp = 0.0 * rtb_IntegralGain * 100.0;

    /* Sum: '<S50>/Sum' incorporates:
     *  Delay: '<S34>/UD'
     *  DiscreteIntegrator: '<S41>/Integrator'
     *  Gain: '<S46>/Proportional Gain'
     *  Sum: '<S34>/Diff'
     */
    rtb_Saturation1 = (0.735101548489 * rtb_IntegralGain +
                       SLC1_DW->Integrator_DSTATE) + (rtb_Tsamp -
      SLC1_DW->UD_DSTATE);

    /* DeadZone: '<S31>/DeadZone' incorporates:
     *  Saturate: '<S48>/Saturation'
     */
    if (rtb_Saturation1 > 17.0) {
      rtb_Saturation = rtb_Saturation1 - 17.0;
      rtb_Saturation1 = 17.0;
    } else {
      if (rtb_Saturation1 >= -17.0) {
        rtb_Saturation = 0.0;
      } else {
        rtb_Saturation = rtb_Saturation1 - -17.0;
      }

      if (rtb_Saturation1 < -17.0) {
        rtb_Saturation1 = -17.0;
      }
    }

    /* End of DeadZone: '<S31>/DeadZone' */

    /* Gain: '<S38>/Integral Gain' */
    rtb_IntegralGain *= 1.7089788571201481;

    /* Saturate: '<S1>/Saturation' incorporates:
     *  Gain: '<S1>/Gain1'
     */
    if (-rtb_Saturation1 <= 0.0) {
      tmp = 0.0;
    } else {
      tmp = -rtb_Saturation1;
    }

    /* Outport: '<Root>/T_reqRR' incorporates:
     *  Inport: '<Root>/T_driver'
     *  Saturate: '<S1>/Saturation'
     *  Sum: '<S1>/Sum9'
     */
    *SLC1_Y_T_reqRR = SLC1_U_T_driver - tmp;

    /* Saturate: '<S1>/Saturation1' */
    if (rtb_Saturation1 <= 0.0) {
      rtb_Saturation1 = 0.0;
    }

    /* Outport: '<Root>/T_reqRL' incorporates:
     *  Inport: '<Root>/T_driver'
     *  Saturate: '<S1>/Saturation1'
     *  Sum: '<S1>/Sum8'
     */
    *SLC1_Y_T_reqRL = SLC1_U_T_driver - rtb_Saturation1;

    /* Switch: '<S29>/Switch1' incorporates:
     *  Constant: '<S29>/Clamping_zero'
     *  Constant: '<S29>/Constant'
     *  Constant: '<S29>/Constant2'
     *  RelationalOperator: '<S29>/fix for DT propagation issue'
     */
    if (rtb_Saturation > 0.0) {
      tmp_0 = 1;
    } else {
      tmp_0 = -1;
    }

    /* Switch: '<S29>/Switch2' incorporates:
     *  Constant: '<S29>/Clamping_zero'
     *  Constant: '<S29>/Constant3'
     *  Constant: '<S29>/Constant4'
     *  RelationalOperator: '<S29>/fix for DT propagation issue1'
     */
    if (rtb_IntegralGain > 0.0) {
      tmp_1 = 1;
    } else {
      tmp_1 = -1;
    }

    /* Switch: '<S29>/Switch' incorporates:
     *  Constant: '<S29>/Clamping_zero'
     *  Constant: '<S29>/Constant1'
     *  Logic: '<S29>/AND3'
     *  RelationalOperator: '<S29>/Equal1'
     *  RelationalOperator: '<S29>/Relational Operator'
     *  Switch: '<S29>/Switch1'
     *  Switch: '<S29>/Switch2'
     */
    if ((rtb_Saturation != 0.0) && (tmp_0 == tmp_1)) {
      rtb_IntegralGain = 0.0;
    }

    /* Update for DiscreteIntegrator: '<S41>/Integrator' incorporates:
     *  Switch: '<S29>/Switch'
     */
    SLC1_DW->Integrator_DSTATE += 0.01 * rtb_IntegralGain;

    /* Update for Delay: '<S34>/UD' */
    SLC1_DW->UD_DSTATE = rtb_Tsamp;
  }

  /* End of Inport: '<Root>/Enable' */
  /* End of Outputs for SubSystem: '<Root>/SLC1' */
}

/* Model initialize function */
void SLC1_initialize(RT_MODEL_SLC1_T *const SLC1_M, double *SLC1_U_T_driver,
                     double *SLC1_U_T_RL, double *SLC1_U_T_RR, bool
                     *SLC1_U_Enable, double *SLC1_Y_T_reqRL, double
                     *SLC1_Y_T_reqRR)
{
  DW_SLC1_T *SLC1_DW = SLC1_M->dwork;

  /* Registration code */

  /* states (dwork) */
  (void) memset((void *)SLC1_DW, 0,
                sizeof(DW_SLC1_T));

  /* external inputs */
  *SLC1_U_T_driver = 0.0;
  *SLC1_U_T_RL = 0.0;
  *SLC1_U_T_RR = 0.0;
  *SLC1_U_Enable = false;

  /* external outputs */
  *SLC1_Y_T_reqRL = 0.0;
  *SLC1_Y_T_reqRR = 0.0;
}

/* Model terminate function */
void SLC1_terminate(RT_MODEL_SLC1_T *const SLC1_M)
{
  /* (no terminate code required) */
  UNUSED_PARAMETER(SLC1_M);
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
