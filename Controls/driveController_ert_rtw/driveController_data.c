/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: driveController_data.c
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

/* Invariant block signals (default storage) */
const ConstB_driveController_T driveController_ConstB = {
  0.0                                  /* '<S8>/Constant' */
};

/* Constant parameters (default storage) */
const ConstP_driveController_T driveController_ConstP = {
  /* Expression: pedal_map
   * Referenced by: '<S2>/1-D Lookup Table'
   */
  { 0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0 },

  /* Expression: pedal_bp
   * Referenced by: '<S2>/1-D Lookup Table'
   */
  { 0.0, 10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0, 90.0, 100.0 },

  /* Expression: [1,1,0]
   * Referenced by: '<Root>/1-D Lookup Table'
   */
  { 1.0, 1.0, 0.0 },

  /* Expression: [0,3,12]
   * Referenced by: '<Root>/1-D Lookup Table'
   */
  { 0.0, 3.0, 12.0 },

  /* Expression: [0,0,1]
   * Referenced by: '<S8>/1-D Lookup Table'
   */
  { 0.0, 0.0, 1.0 },

  /* Expression: [0,steering_deadband,5]
   * Referenced by: '<S8>/1-D Lookup Table'
   */
  { 0.0, 2.0, 5.0 }
};

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
