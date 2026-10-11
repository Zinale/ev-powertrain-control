/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: driveController_data.c
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

/* Constant parameters (default storage) */
const ConstP_driveController_T driveController_ConstP = {
  /* Expression: pedal_map_eco
   * Referenced by: '<S3>/1-D Lookup Table'
   */
  { 0.0, 0.008286135043349966, 0.025118864315095798, 0.048055822463052081,
    0.076146157548635129, 0.1088188204120155, 0.14567801244906112,
    0.18642702577112424, 0.23083198494515417, 0.27870195942000231,
    0.32987697769322355, 0.38422029683805425, 0.44161315369069992,
    0.5019510541446327, 0.56514106281310794, 0.63109976931348721,
    0.699751727323698, 0.77102823316636648, 0.84486635402361254,
    0.9212081434582966, 1.0 },

  /* Pooled Parameter (Expression: pedal_bp)
   * Referenced by:
   *   '<S3>/1-D Lookup Table'
   *   '<S3>/1-D Lookup Table1'
   *   '<S3>/1-D Lookup Table2'
   */
  { 0.0, 5.0, 10.0, 15.0, 20.0, 25.0, 30.0, 35.0, 40.0, 45.0, 50.0, 55.0, 60.0,
    65.0, 70.0, 75.0, 80.0, 85.0, 90.0, 95.0, 100.0 },

  /* Expression: pedal_map_normal
   * Referenced by: '<S3>/1-D Lookup Table1'
   */
  { 0.0, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65,
    0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 1.0 },

  /* Expression: pedal_map_gas
   * Referenced by: '<S3>/1-D Lookup Table2'
   */
  { 0.0, 0.16572270086699936, 0.251188643150958, 0.32037214975368061,
    0.3807307877431757, 0.43527528164806206, 0.48559337483020382,
    0.53264864506035514, 0.5770799623628855, 0.61933768760000518,
    0.6597539553864471, 0.69858235788737133, 0.7360219228178333,
    0.77223239099174268, 0.8073443754472972, 0.84146635908464962,
    0.87468965915462249, 0.90709203901925473, 0.93874039335956938,
    0.96969278258768077, 1.0 },

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

  /* Expression: [0,steering_deadband,4]
   * Referenced by: '<S8>/1-D Lookup Table'
   */
  { 0.0, 1.0, 4.0 }
};

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
