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
static tTimer_details osalTimerDetails2;
static TimerHandle_t wifiMgrHandle;
static TimerHandle_t wifiMgrHandle2;

// *****************************************************************************
static void wifiMgr_initFunc( tMcuMgrSphereMsg* sphereMsg );
static void osalTimerReload( tMcuMgrSphereMsg* sphereMsg );
static void wifiMgr_wincInit();
static void wifiMgr_cloudWifiInit();
static void wifiMgr_MqttInit();
static void wifiMgr_changeState(uint16_t changeState);
static void wifi_mgr_cloud_periodic_func();

// look up function for winc init
static tMcuMgrDispatchMsg dispatchCommonDoHandler[] =
{
    { PT_GENERAL_OSAL_TIMER, osalTimerReload },
};

static tMcuMgrDispatchMsg dispatchConnectingDoHandler[] =
{
    { PT_GENERAL_OSAL_TIMER, osalTimerReload },
    { PT_CLOUD_OSAL_TIMER, wifi_mgr_cloud_periodic_func }
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
    [WIFI_MGR_CLOUD_WIFI_CONNECTING] = 
    {
        .entryFunc = NULL,
        .doFunc = dispatchConnectingDoHandler,
        .doFuncElement = FUNC_ELEMENT(dispatchConnectingDoHandler),
        .exitFunc = NULL
    },
    [WIFI_MGR_CLOUD_WIFI_CONNECTED] = 
    {
        .entryFunc = wifiMgr_MqttInit,
        .doFunc = dispatchConnectingDoHandler,
        .doFuncElement = FUNC_ELEMENT(dispatchConnectingDoHandler),
        .exitFunc = NULL
    },
    [WIFI_MGR_CLOUD_WIFI_DISCONNECTED] = 
    {
        .entryFunc = NULL,
        .doFunc = dispatchCommonDoHandler,
        .doFuncElement = FUNC_ELEMENT(dispatchCommonDoHandler),
        .exitFunc = NULL
    },
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
    SYS_DEBUG_ErrorLevelSet(SYS_ERROR_FATAL);

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
        osalTimerDetails.timer = 100;
        osalTimerDetails.oneShot = false;
        osalTimerDetails.callbackFunc = commonTimerCallbackFunc;
        osalTimerDetails.source = COMPONENT_ID_WIFI_MGR;
        osalTimerDetails.msgId = PT_GENERAL_OSAL_TIMER;
        wifiMgrHandle = createOsalTimer(&osalTimerDetails);

        osalTimerDetails2.name = "Cloud Periodic Function";
        osalTimerDetails2.timer = 100;
        osalTimerDetails2.oneShot = false;
        osalTimerDetails2.callbackFunc = commonTimerCallbackFunc;
        osalTimerDetails2.source = COMPONENT_ID_WIFI_MGR;
        osalTimerDetails2.msgId = PT_CLOUD_OSAL_TIMER;
        wifiMgrHandle2 = createOsalTimer(&osalTimerDetails2);
    }
    
    fsm_init(wifiMgrFsm);
    // transit to init cloud

};

static void osalTimerReload( tMcuMgrSphereMsg* sphereMsg )
{
    SYS_CONSOLE_PRINT("\r\n Wifi mgr osal timer reload function in %d state\r\n", fsm_getCurrentState());
    static uint8_t counter = 0;
    counter++;
    counter = counter % 5;

    WDRV_WINC_Tasks(sysObj.drvWifiWinc);

    if( fsm_getCurrentState() == WIFI_MGR_CLOUD_WIFI_CONNECTED )
    {
        static uint16_t number=0;
        cloud_publish_msg(number++);
    }
}

static void wifi_mgr_cloud_periodic_func()
{
    cloudPeriodicFunc();
}

static void wifiMgr_wincInit()
{
    // run example task
    SYS_CONSOLE_PRINT("\r\n Wifi mgr winc init function\r\n");
    startTimer(wifiMgrHandle);
    startTimer(wifiMgrHandle2);
    fsm_changeState( wifiMgrFsm, WIFI_MGR_CLOUD_WIFI_INIT );
}

static void wifiMgr_cloudWifiInit()
{
    SYS_CONSOLE_PRINT("\r\n Wifi mgr cloud wifi init function\r\n");
    int wifi_status = cloud_wifi_init(wdrvHandle, wifiMgr_changeState);
    if(wifi_status == M2M_SUCCESS)
    {
        SYS_CONSOLE_PRINT("\r\n Cloud init successfully\r\n");
        //fsm_changeState( wifiMgrFsm, WIFI_MGR_CLOUD_WIFI_CONNECTING );
    }
}

static void wifiMgr_MqttInit()
{
    SYS_CONSOLE_PRINT("\r\n Mqtt init function\r\n");
    cloudMqttInit();
}

static void wifiMgr_changeState(uint16_t changeState)
{
    fsm_changeState( wifiMgrFsm, changeState );
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
