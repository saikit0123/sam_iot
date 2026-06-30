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
#include "wdrv_winc.h"
#include "fsm.h"
#include "wifi_mgr.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
extern SYSTEM_OBJECTS sysObj;
static DRV_HANDLE wdrvHandle;


// *****************************************************************************
static void wifiMgr_initFunc( tMcuMgrSphereMsg* sphereMsg );
static void wifiMgr_wincInit();

static const tFsmLookUp wifiMgrFsm[] =
{
    [WIFI_MGR_WINC_INIT] = 
    {
        .entryFunc = wifiMgr_wincInit,
        .doFunc = NULL,
        .doFuncElement = 0,
        .exitFunc = NULL
    },
    [WIFI_MGR_CLOUD_WIFI_INIT] = 
    {
        .entryFunc = NULL,
        .doFunc = NULL,
        .doFuncElement = 0,
        .exitFunc = NULL
    },
    [WIFI_MGR_CLOUD_WIFI_CONFIGURE] = 
    {
        .entryFunc = NULL,
        .doFunc = NULL,
        .doFuncElement = 0,
        .exitFunc = NULL
    },
    [WIFI_MGR_CLOUD_WIFI_CONNECTING] = 
    {
        .entryFunc = NULL,
        .doFunc = NULL,
        .doFuncElement = 0,
        .exitFunc = NULL
    },
    [WIFI_MGR_CLOUD_WIFI_CONNECTED] = 
    {
        .entryFunc = NULL,
        .doFunc = NULL,
        .doFuncElement = 0,
        .exitFunc = NULL
    }
};

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
    SYS_CONSOLE_PRINT("\r\n Wifi mgr init function\r\n");
    
    // init wifi driver
    WDRV_WINC_Tasks(sysObj.drvWifiWinc);

    if (  SYS_STATUS_READY == WDRV_WINC_Status(sysObj.drvWifiWinc) )
    {
        SYS_CONSOLE_PRINT("\r\n Wifi mgr initialisation success\n");
        wdrvHandle = WDRV_WINC_Open(0, (int)NULL);
    }

    if (DRV_HANDLE_INVALID != wdrvHandle)
    {
        // initialisation example task, osal timer
    }
    
    fsm_init(wifiMgrFsm);
    // transit to init cloud

};

static void wifiMgr_wincInit()
{
    SYS_CONSOLE_PRINT("\r\n Wifi mgr winc init function\r\n");
    
}


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
