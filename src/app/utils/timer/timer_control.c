/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    timer.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the timer function
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "timer_control.h"
#include "sys_tasks.h"
#include "osal_sphere.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************
void commonTimerCallbackFunc( TimerHandle_t xTimer )
{
    // send sphere msg over here
    SYS_CONSOLE_PRINT("\r\n Common callback function\r\n");
    tTimer_details *timerDetails = (tTimer_details *)pvTimerGetTimerID(xTimer);
    tMcuMgrSphereMsg payload =
    {
        .dst =  timerDetails->source,
        .src = timerDetails->source,
        .msgId = timerDetails->msgId,
        .length = 0,
        .payload = NULL
    };
    osalSphereSendMsg(&payload);
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Application Public Functions
// *****************************************************************************
// *****************************************************************************
TimerHandle_t createOsalTimer(tTimer_details* xOsalTimer)
{
    return xTimerCreate(
                xOsalTimer->name,
                pdMS_TO_TICKS(xOsalTimer->timer),
                xOsalTimer->oneShot? pdFALSE : pdTRUE,      // One-shot
                xOsalTimer,
                xOsalTimer->callbackFunc
            );
}

void startTimer(TimerHandle_t xOsalTimer)
{
    xTimerStart(xOsalTimer, 0);
}

void changeTimerPeriod(TimerHandle_t xOsalTimer, uint16_t duration)
{
    xTimerChangePeriod(
        xOsalTimer,
        pdMS_TO_TICKS(duration),
        portMAX_DELAY
    );
}

void stopTimer(TimerHandle_t xOsalTimer)
{
    xTimerStop(xOsalTimer, portMAX_DELAY);
}