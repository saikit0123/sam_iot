/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    cloud_task.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the osal common function
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "cloud_task.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Application Public Functions
// *****************************************************************************
// *****************************************************************************
int cloud_wifi_init(DRV_HANDLE handle)
{
    return 0;
    // int8_t status = !0;
    // tstrM2mRev strtmp;
    // int8_t ret;

    // debug_printInfo("\r\n");
    // debug_printInfo("===========================\r\n");
    // debug_printInfo("Trust Platform Cloud Example\r\n");
    // debug_printInfo("===========================\r\n");

    // /* Register callback function for TC5 period interrupt */
    // TC5_TimerCallbackRegister(TC5_Callback_InterruptHandler, (uintptr_t)NULL);
    // memset(&g_socket_connection, 0, sizeof(g_socket_connection));
    // extern ATCAIfaceCfg atecc608_0_init_data;
    // /* Start the timer*/
    // TC5_TimerStart();

    // do
    // {
    //     // Initialize the cryptoauthlib stack
    //     atecc608_0_init_data.atcai2c.address = SECURE_ELEMENT_ADDRESS;
    //     if ((status = atcab_init(&atecc608_0_init_data)) != ATCA_SUCCESS)
    //     {
    //         break;
    //     }
    //     config_set_thing_id();
    //     #ifdef CFG_ENABLE_CLI     
    //         set_deviceId(config_thing_id);
    //     #endif   
    //         debug_init(config_thing_id);   
    //     /* Enable use of DHCP for network configuration, DHCP is the default
    //        but this also registers the callback for notifications. */
    //     if ((status = WDRV_WINC_IPUseDHCPSet(handle, &APP_ExampleDHCPAddressEventCallback))  != WDRV_WINC_STATUS_OK)
    //     {
    //         break;
    //     }

    //     /* Initialize the BSS context to use default values. */
    //     if ((status = WDRV_WINC_BSSCtxSetDefaults(&bssCtx)) != WDRV_WINC_STATUS_OK)
    //     {
    //         break;
    //     }
            
    //     /* Update BSS context with target SSID for connection. */
    //     if ((status = WDRV_WINC_BSSCtxSetSSID(&bssCtx, (uint8_t*)ssid, strlen(ssid))) != WDRV_WINC_STATUS_OK)
    //     {
    //         break;
    //     }

    //     /*if (WDRV_WINC_STATUS_OK != WDRV_WINC_AuthCtxSetOpen(&authCtx))
    //        {
    //         break;
    //        }*/

    //     /*Initialize the authentication context for WPA. */
    //     if ((status = WDRV_WINC_AuthCtxSetWPA(&authCtx, (uint8_t*)pass, strlen(pass))) != WDRV_WINC_STATUS_OK)
    //     {
    //         break;
    //     }

    //     /* Initialize the system time callback handler. */
    //     if ((status = WDRV_WINC_SystemTimeGetCurrent(handle, &APP_ExampleGetSystemTimeEventCallback)) != WDRV_WINC_STATUS_OK)
    //     {
    //         break;
    //     }

    //     /* Register callback handler for DNS resolver . */
    //     if ((status = WDRV_WINC_SocketRegisterResolverCallback(handle, &cloud_dns_resolve_handler)) != WDRV_WINC_STATUS_OK)
    //     {
    //         break;
    //     }

    //     /* Register callback handler for socket events. */
    //     if ((status = WDRV_WINC_SocketRegisterEventCallback(handle, &socket_callback_handler)) != WDRV_WINC_STATUS_OK)
    //     {
    //         break;
    //     }

    //     ret = nm_get_firmware_full_info(&strtmp);
    //     if(M2M_ERR_FW_VER_MISMATCH == ret)
    //     {
    //         status = !0;
    //         break;
    //     }

    //     debug_printInfo("\nWINC1500 Firmware Data:\r\n");
    //     debug_printInfo("Firmware Ver: %u.%u.%u SVN Rev %u\r\n", strtmp.u8FirmwareMajor, strtmp.u8FirmwareMinor, strtmp.u8FirmwarePatch, strtmp.u16FirmwareSvnNum);
    //     debug_printInfo("Firmware Built at %s Time %s\r\n", strtmp.BuildDate, strtmp.BuildTime);
    //     debug_printInfo("Firmware Min Driver Ver: %u.%u.%u\r\n", strtmp.u8DriverMajor, strtmp.u8DriverMinor, strtmp.u8DriverPatch);
    //     debug_printInfo("Driver Ver: %u.%u.%u\r\n", M2M_RELEASE_VERSION_MAJOR_NO, M2M_RELEASE_VERSION_MINOR_NO, M2M_RELEASE_VERSION_PATCH_NO);
    //     debug_printInfo("Driver Built at %s Time %s\r\n\r\n", __DATE__, __TIME__);

    //     debug_printInfo("Secure Element Address: 0x%02X \r\n", SECURE_ELEMENT_ADDRESS);
    //     debug_printInfo("Cloud Endpoint: %s \r\n", CLOUD_ENDPOINT);
    // #ifdef CLOUD_CONNECT_WITH_CUSTOM_CERTS
    //     debug_printInfo("Connecting with CUSTOM certs\r\n\rn\n");
    // #else
    //     debug_printInfo("Connecting with MCHP certs\r\n\r\n");
    // #endif

    // #ifndef CLOUD_CONFIG_GCP
    //     transfer_ecc_certs_to_winc();
    // #endif

    //     if ((status = m2m_ssl_set_active_ciphersuites(SSL_CIPHER_SUITE_SELECTION)) != M2M_SUCCESS)
    //     {
    //         break;
    //     }

    //     // Initialize the MQTT library
    //     g_mqtt_network.mqttread  = &mqtt_packet_read;
    //     g_mqtt_network.mqttwrite = &mqtt_packet_write;

    //     MQTTClientInit(&g_mqtt_client, &g_mqtt_network, MQTT_COMMAND_TIMEOUT_MS,
    //                    g_mqtt_tx_buffer, sizeof(g_mqtt_tx_buffer),
    //                    g_mqtt_rx_buffer, sizeof(g_mqtt_rx_buffer));

    // }
    // while (0);


    // return status;
}
