/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    security_mgr.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the security application, cryptographic
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

// *****************************************************************************
static void securityMgr_initFunc( tMcuMgrSphereMsg* sphereMsg );

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************
static const tMcuMgrDispatchMsg dispatchMsg[] =
{
    { PT_SPHERE_START_REQ_ID, securityMgr_initFunc },
};

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************
static void securityMgr_initFunc( tMcuMgrSphereMsg* sphereMsg )
{
    SYS_CONSOLE_PRINT("\r\n Security mgr init function\r\n");
};

/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Public Functions
// *****************************************************************************
// *****************************************************************************
void securityMgr_moduleCallbackFromSphere( tMcuMgrSphereMsg* sphereMsg )
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
