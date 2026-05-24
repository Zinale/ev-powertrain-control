/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: driveController_data.c
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

/* Invariant block signals (default storage) */
const ConstB_driveController_T driveController_ConstB = {
  0.0F                                 /* '<S9>/Constant' */
};

/* Constant parameters (default storage) */
const ConstP_driveController_T driveController_ConstP = {
  /* Computed Parameter: uDLookupTable_tableData
   * Referenced by: '<S1>/1-D Lookup Table'
   */
  { 0.0F, 0.1F, 0.2F, 0.3F, 0.4F, 0.5F, 0.6F, 0.7F, 0.8F, 0.9F, 1.0F },

  /* Computed Parameter: uDLookupTable_bp01Data
   * Referenced by: '<S1>/1-D Lookup Table'
   */
  { 0.0F, 10.0F, 20.0F, 30.0F, 40.0F, 50.0F, 60.0F, 70.0F, 80.0F, 90.0F, 100.0F
  }
};

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
