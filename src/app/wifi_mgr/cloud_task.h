/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    cloud_task.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application. Implement the header file of the timer
*******************************************************************************/

#ifndef CLOUD_TASK_H
#define CLOUD_TASK_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "wdrv_winc.h"

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
int cloud_wifi_init(DRV_HANDLE handle);

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* CLOUD_TASK_H */

/*******************************************************************************
 End of File
 */

