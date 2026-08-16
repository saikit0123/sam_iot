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
#include "fsm.h"
#include "wdrv_winc.h"
#include "wifi_mgr.h"
#include "cloud_task.h"
#include "timer_control.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
extern SYSTEM_OBJECTS sysObj;
static DRV_HANDLE wdrvHandle;
static tTimer_details osalTimerDetails;
static TimerHandle_t wifiMgrHandle;

// *****************************************************************************
static void wifiMgr_initFunc( tMcuMgrSphereMsg* sphereMsg );
static void osalTimerReload( tMcuMgrSphereMsg* sphereMsg );
static void wifiMgr_wincInit();
static void wifiMgr_cloudWifiInit();

// look up function for winc init
static tMcuMgrDispatchMsg dispatchCommonDoHandler[] =
{
    { PT_GENERAL_OSAL_TIMER, osalTimerReload },
};

static const tFsmLookUp wifiMgrFsm[] =
{
    [WIFI_MGR_WINC_INIT] = 
    {
        .entryFunc = wifiMgr_wincInit,
        .doFunc = dispatchCommonDoHandler,
        .doFuncElement = FUNC_ELEMENT(dispatchCommonDoHandler),
        .exitFunc = NULL
    },
    [WIFI_MGR_CLOUD_WIFI_INIT] = 
    {
        .entryFunc = wifiMgr_cloudWifiInit,
        .doFunc = dispatchCommonDoHandler,
        .doFuncElement = FUNC_ELEMENT(dispatchCommonDoHandler),
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
        osalTimerDetails.name = "Cloud Initialization";
        osalTimerDetails.timer = 500;
        osalTimerDetails.oneShot = false;
        osalTimerDetails.callbackFunc = commonTimerCallbackFunc;
        osalTimerDetails.source = COMPONENT_ID_WIFI_MGR;
        osalTimerDetails.msgId = PT_GENERAL_OSAL_TIMER;
        wifiMgrHandle = createOsalTimer(&osalTimerDetails);
    }
    
    fsm_init(wifiMgrFsm);
    // transit to init cloud

};

static void osalTimerReload( tMcuMgrSphereMsg* sphereMsg )
{
    SYS_CONSOLE_PRINT("\r\n Wifi mgr osal timer reload function\r\n");
}

static void wifiMgr_wincInit()
{
    // run example task
    SYS_CONSOLE_PRINT("\r\n Wifi mgr winc init function\r\n");
    startTimer(wifiMgrHandle);
    fsm_changeState( wifiMgrFsm, WIFI_MGR_CLOUD_WIFI_INIT );
}

static void wifiMgr_cloudWifiInit()
{
    SYS_CONSOLE_PRINT("\r\n Wifi mgr cloud wifi init function\r\n");
    //int wifi_status = cloud_wifi_init(wdrvHandle);
    cloud_wifi_init(wdrvHandle);
    // if (wifi_status == M2M_SUCCESS)
    // {
    //     // Set the current state
    //     cloud_iot_set_status(CLOUD_STATE_WIFI_CONFIGURE, CLOUD_STATUS_SUCCESS,
    //                             "The cloud IoT Demo WINC1500 WIFI Init was successful.");

    //     // Set the next cloud WIFI state
    //     g_cloud_wifi_state = CLOUD_STATE_WIFI_CONFIGURE;
    // }
    // else
    // {
    //     // Set the current state
    //     cloud_iot_set_status(CLOUD_STATE_ATECCx08A_INIT, CLOUD_STATUS_ATECCx08A_INIT_FAILURE,
    //                             "The cloud IoT Demo WINC1500 WIFI init was not successful.");

    //     debug_printError("An WINC1500 WIFI initialization error has occurred.");
    //     debug_printError("Stopping the cloud IoT demo.");

    //     // An error has occurred during initialization.  Stop the demo.
    //     g_cloud_wifi_state = CLOUD_STATE_UNKNOWN;
    // }
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Public Functions
// *****************************************************************************
// *****************************************************************************
void wifiMgr_moduleCallbackFromSphere( tMcuMgrSphereMsg* sphereMsg )
{
    if( sphereMsg->msgId == PT_SPHERE_START_REQ_ID )
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
    else
    {
        fsm_dispatch(wifiMgrFsm, sphereMsg);
    }
}
