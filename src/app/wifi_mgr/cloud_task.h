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
#include "socket.h"

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
typedef enum
{
    WIFI_STATUS_UNKNOWN          = 0,
    WIFI_STATUS_MESSAGE_RECEIVED = 1,
    WIFI_STATUS_MESSAGE_SENT     = 2,
    WIFI_STATUS_TIMEOUT          = 3,
    WIFI_STATUS_ERROR            = 4
} cloudTask_status;

typedef struct
{
    SOCKET   socket;
    uint32_t address;
    uint16_t port;
} cloudTaskConnection;

typedef struct socket_connection
{
    SOCKET   socket;
    uint32_t address;
    uint16_t port;
} socket_connection;

typedef struct
{
    int         code;
    const char* name;
} ErrorInfo;

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

