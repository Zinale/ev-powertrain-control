#ifndef AMK_DIAGNOSTIC_H
#define AMK_DIAGNOSTIC_H

#include "main.h"

/**
 * ***********************************************************************
 * AMK_DIAGNOSTIC_H
 *
 * This header defines the error mapping. Each error is represented by a
 * 16-bit code and associated with a string that describes the error.
 * ***********************************************************************
 */

// Structure to hold error information
typedef struct {
    const char* category;
    const char* class;
} ErrorInfo;

typedef struct {
    uint16_t id;
    ErrorInfo info;
} ErrorEntry;

// Define the error map as an array of ErrorEntry
extern const ErrorEntry ERROR_MAP[];

extern const ErrorInfo* findErrorById(uint16_t errorId);

#endif // AMK_DIAGNOSTIC_H
