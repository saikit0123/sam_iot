/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    timer_control.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application. Implement the header file of the timer
*******************************************************************************/

#ifndef TIMER_CONTROL_H
#define TIMER_CONTROL_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "stdint.h"
#include "stdbool.h"
#include "FreeRTOS.h"
#include "timers.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************

typedef struct
{
    char* name;
    uint16_t timer;
    bool oneShot;
    TimerCallbackFunction_t callbackFunc;
    uint16_t source;
    uint16_t msgId;
} tTimer_details;

// ******************************************************************************
// ******************************************************************************
// Section: Function Prototype
// ******************************************************************************
// ******************************************************************************

TimerHandle_t createOsalTimer(tTimer_details* xOsalTimer);

void startTimer(TimerHandle_t xOsalTimer);

void changeTimerPeriod(TimerHandle_t xOsalTimer, uint16_t duration);

void stopTimer(TimerHandle_t xOsalTimer);

void commonTimerCallbackFunc( TimerHandle_t xTimer );

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* TIMER_CONTROL_H */

/*******************************************************************************
 End of File
 */

