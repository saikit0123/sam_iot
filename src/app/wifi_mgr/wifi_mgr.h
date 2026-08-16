/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    wifi_mgr.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application. Implement the header file of the wifi mgr include
    the mqtt mechanism
*******************************************************************************/

#ifndef WIFI_MGR_H
#define WIFI_MGR_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

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
#define FUNC_ELEMENT(element)  (sizeof(element)/sizeof(element[0]))

typedef enum
{
    WIFI_MGR_WINC_INIT,
    WIFI_MGR_CLOUD_WIFI_INIT,
    WIFI_MGR_CLOUD_WIFI_CONFIGURE,
    WIFI_MGR_CLOUD_WIFI_CONNECTING,
    WIFI_MGR_CLOUD_WIFI_CONNECTED,
} tWifiMgr_fsmState;

// ******************************************************************************
// ******************************************************************************
// Section: Function Prototype
// ******************************************************************************
// ******************************************************************************
void wifiMgr_moduleCallbackFromSphere( tMcuMgrSphereMsg* sphereMsg );

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* WIFI_MGR_H */

/*******************************************************************************
 End of File
 */

