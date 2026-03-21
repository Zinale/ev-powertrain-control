#ifndef __SCHEDULER_H
#define __SCHEDULER_H

/* ***********************************************************************
 * SCHEDULER MODULE HEADER
 *
 * Includes all necessary headers and declares variables and functions
 * that are visible to other modules including "Scheduler.h".
 * ***********************************************************************/

#include "main.h"

// Standard library inclusion (for serial communication) - to be removed if unused
#include <stdio.h>
#include <stdbool.h>
#include <string.h> // memcpy
#include <stdlib.h>  // Required for NULL, free() and abs()
#include <stdarg.h>  // Required for handling variable arguments in printf-style functions

#include <math.h> // Required for tan() and fabs()

// Local library inclusion
#include "Utils.h"
#include "Tasks.h"
#include "Can.h"

#include "AMKDiagnostic.h"
#include "Inverter.h"

/* ***********************************************************************
 * Exported functions for the Scheduler module
 * ***********************************************************************/

extern void SchTimerInterruptCallback(void);

extern void SchedulerInitFct(void);

extern void SchedulerMgmFct(void);

#endif /* __SCHEDULER_H */


