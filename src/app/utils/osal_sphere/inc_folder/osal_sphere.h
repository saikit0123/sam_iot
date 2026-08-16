/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    osal_sphere.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application. Implement the header file of the timer
*******************************************************************************/

#ifndef OSAL_SPHERE_H
#define OSAL_SPHERE_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "sys_tasks.h"


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


// ******************************************************************************
// ******************************************************************************
// Section: Function Prototype
// ******************************************************************************
// ******************************************************************************
bool osalSphereSendMsg(tMcuMgrSphereMsg* payload);

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* TIMER_CONTROL_H */

/*******************************************************************************
 End of File
 */

