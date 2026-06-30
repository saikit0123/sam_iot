/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    mcu_mgr.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file to control the task mgr
*******************************************************************************/

#ifndef MCU_MGR_H
#define MCU_MGR_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "msg_id.h"
#include "stdint.h"

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
#define TASK_QUEUE_LOW  ( 10U )
#define TASK_QUEUE_MED  ( 20U )
#define TASK_QUEUE_HIGH ( 40U )

typedef struct
{
    uint16_t dst;
    uint16_t src;
    uint16_t msgId;
    uint16_t length;
    uint8_t* payload;
} tMcuMgrSphereMsg;

typedef void (*callDispatchFunc)( tMcuMgrSphereMsg* sphereMsg );
typedef struct
{
    uint16_t msgId;
    callDispatchFunc dispatchFunc;
} tMcuMgrDispatchMsg;

typedef enum
{
    COMPONENT_ID_LED_MGR,
    COMPONENT_ID_SECURITY_MGR,
    COMPONENT_ID_WIFI_MGR,
    COMPONENT_ID_MAX
} tMcuMgrComponentId;

#define TASK_TBL_CFG \
    { COMPONENT_ID_LED_MGR,      "TaskLedMgr",      SYS_CMD_RTOS_STACK_SIZE, SYS_CMD_RTOS_TASK_PRIORITY, ledMgr_moduleCallbackFromSphere }, \
    { COMPONENT_ID_SECURITY_MGR, "TaskSecurityMgr", SYS_CMD_RTOS_STACK_SIZE, SYS_CMD_RTOS_TASK_PRIORITY, securityMgr_moduleCallbackFromSphere }, \
    { COMPONENT_ID_WIFI_MGR, "TaskWifiMgr", SYS_CMD_RTOS_STACK_SIZE, SYS_CMD_RTOS_TASK_PRIORITY, wifiMgr_moduleCallbackFromSphere }

#define QUEUE_TBL_CFG \
    { COMPONENT_ID_LED_MGR, "ledQ", TASK_QUEUE_LOW }, \
    { COMPONENT_ID_SECURITY_MGR, "secQ", TASK_QUEUE_LOW }, \
    { COMPONENT_ID_WIFI_MGR, "wifiQ", TASK_QUEUE_LOW }

// ******************************************************************************
// ******************************************************************************
// Section: Function Prototype
// ******************************************************************************
// ******************************************************************************


//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* MCU_MGR_H */

/*******************************************************************************
 End of File
 */

