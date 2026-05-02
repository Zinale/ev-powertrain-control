/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: rt_nonfinite.c
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

#include <stdbool.h>
#include "rt_nonfinite.h"
#include "math.h"

double rtNaN = -(double)NAN;
double rtInf = (double)INFINITY;
double rtMinusInf = -(double)INFINITY;
float rtNaNF = -(float)NAN;
float rtInfF = (float)INFINITY;
float rtMinusInfF = -(float)INFINITY;

/* Test if value is infinite */
bool rtIsInf(double value)
{
  return (bool)isinf(value);
}

/* Test if single-precision value is infinite */
bool rtIsInfF(float value)
{
  return (bool)isinf(value);
}

/* Test if value is not a number */
bool rtIsNaN(double value)
{
  return (bool)(isnan(value) != 0);
}

/* Test if single-precision value is not a number */
bool rtIsNaNF(float value)
{
  return (bool)(isnan(value) != 0);
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
