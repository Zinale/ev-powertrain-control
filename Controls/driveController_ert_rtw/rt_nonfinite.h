/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: rt_nonfinite.h
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

#ifndef rt_nonfinite_h_
#define rt_nonfinite_h_
#include <stdbool.h>

extern double rtInf;
extern double rtMinusInf;
extern double rtNaN;
extern float rtInfF;
extern float rtMinusInfF;
extern float rtNaNF;
extern bool rtIsInf(double value);
extern bool rtIsInfF(float value);
extern bool rtIsNaN(double value);
extern bool rtIsNaNF(float value);

#endif                                 /* rt_nonfinite_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
