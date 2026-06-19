/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    wifi_mgr.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the wifi application which include the mqtt
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "sys_tasks.h"
#include "mcu_mgr.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
extern SYSTEM_OBJECTS sysObj;

// *****************************************************************************
static void wifiMgr_initFunc( tMcuMgrSphereMsg* sphereMsg );


// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************
static const tMcuMgrDispatchMsg dispatchMsg[] =
{
    { PT_SPHERE_START_REQ_ID, wifiMgr_initFunc },
};

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************
static void wifiMgr_initFunc( tMcuMgrSphereMsg* sphereMsg )
{
    SYS_CONSOLE_PRINT("\r\n Wifi mgr init function\n");
    
    // init wifi driver
    WDRV_WINC_Tasks(sysObj.drvWifiWinc);

    // WDRV_WINC_Status(sysObj.drvWifiWinc)

    // WDRV_WINC_Open(0, (int)NULL)

    // transit to init cloud

};

/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Public Functions
// *****************************************************************************
// *****************************************************************************
void wifiMgr_moduleCallbackFromSphere( tMcuMgrSphereMsg* sphereMsg )
{
    for(uint8_t i = 0; i < sizeof(dispatchMsg)/sizeof(dispatchMsg[0]); i++ )
    {
        if( dispatchMsg[i].msgId == sphereMsg->msgId )
        {
            dispatchMsg[i].dispatchFunc( sphereMsg );
            break;
        }
    }
}
