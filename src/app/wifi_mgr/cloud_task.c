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
#include "atca_iface.h"
#include "atca_basic.h"
#include "cloud_status.h"

// *****************************************************************************
// *****************************************************************************
// Section: Macro Definitions
// *****************************************************************************
// *****************************************************************************
#define SECURE_ELEMENT_ADDRESS      0x6C
#define CLOUD_ENDPOINT              "mqtt.googleapis.com"
#define CLOUD_PORT                  (8883)
#define NEW_SOCKET_ERROR(err) { err, #err }
#define MAX_WIFI_CREDENTIALS_LENGTH 31

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
static cloudTaskConnection m_cloudTask_connection;
static WDRV_WINC_BSS_CONTEXT bssCtx;
static WDRV_WINC_AUTH_CONTEXT authCtx;
static uint8_t g_host_ip_address[4];
extern char config_thing_id[130];
volatile enum cloud_iot_state g_cloud_wifi_state = CLOUD_STATE_WINC1500_INIT;
static socket_connection g_socket_connection;
static int32_t g_rx_buffer_length = 0;
static uint32_t g_tx_size = 0;
static cloudTask_status g_wifi_status = WIFI_STATUS_UNKNOWN;
static char ssid[MAX_WIFI_CREDENTIALS_LENGTH];
static char pass[MAX_WIFI_CREDENTIALS_LENGTH];
static const ErrorInfo g_socket_error_info[] =
{
    NEW_SOCKET_ERROR(SOCK_ERR_NO_ERROR),
    NEW_SOCKET_ERROR(SOCK_ERR_INVALID_ADDRESS),
    NEW_SOCKET_ERROR(SOCK_ERR_ADDR_ALREADY_IN_USE),
    NEW_SOCKET_ERROR(SOCK_ERR_MAX_TCP_SOCK),
    NEW_SOCKET_ERROR(SOCK_ERR_MAX_UDP_SOCK),
    NEW_SOCKET_ERROR(SOCK_ERR_INVALID_ARG),
    NEW_SOCKET_ERROR(SOCK_ERR_MAX_LISTEN_SOCK),
    NEW_SOCKET_ERROR(SOCK_ERR_INVALID),
    NEW_SOCKET_ERROR(SOCK_ERR_ADDR_IS_REQUIRED),
    NEW_SOCKET_ERROR(SOCK_ERR_CONN_ABORTED),
    NEW_SOCKET_ERROR(SOCK_ERR_TIMEOUT),
    NEW_SOCKET_ERROR(SOCK_ERR_BUFFER_FULL),
};

// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************
static int config_set_thing_id();
static void APP_ExampleDHCPAddressEventCallback(DRV_HANDLE handle, uint32_t ipAddress);
static const char* get_socket_error_name(int error_code);

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************
static const char* get_socket_error_name(int error_code)
{
    for (size_t i = 0; i < sizeof(g_socket_error_info) / sizeof(g_socket_error_info[0]); i++)
    {
        if (error_code == g_socket_error_info[i].code)
        {
            return g_socket_error_info[i].name;
        }
    }
    return "UNKNOWN";
}

static int config_set_thing_id()
{
    uint8_t serial_num[9];
    size_t hex_size;

    hex_size = sizeof(config_thing_id) - 1;
    ATCA_STATUS rv;

    rv = atcab_read_serial_number(serial_num);
    if (ATCA_SUCCESS != rv)
    {
        return rv;
    }
    rv = atcab_bin2hex_(serial_num, sizeof(serial_num), config_thing_id, &hex_size, false, false, true);

    if (ATCA_SUCCESS != rv)
    {
        return rv;
    }

    return ATCA_SUCCESS;
}

static void APP_ExampleDHCPAddressEventCallback(DRV_HANDLE handle, uint32_t ipAddress)
{
    char s[20];

    SYS_CONSOLE_PRINT("IP address is %s\r\n", inet_ntop(AF_INET, &ipAddress, s, sizeof(s)));
    gethostbyname((char*)CLOUD_ENDPOINT);

}

static void cloud_dns_resolve_handler(uint8_t *pu8DomainName, uint32_t u32ServerIP)
{

    int8_t status = SOCK_ERR_INVALID_ARG;
    SOCKET new_socket = SOCK_ERR_INVALID;
    struct sockaddr_in socket_address;
    char message[128];

    if (u32ServerIP != 0)
    {
        // Save the Host IP Address
        g_host_ip_address[0] = u32ServerIP & 0xFF;
        g_host_ip_address[1] = (u32ServerIP >> 8) & 0xFF;
        g_host_ip_address[2] = (u32ServerIP >> 16) & 0xFF;
        g_host_ip_address[3] = (u32ServerIP >> 24) & 0xFF;

        sprintf(&message[0], "WINC1500 WIFI: DNS lookup:\r\n  Host:       %s\r\n  IP Address: %u.%u.%u.%u",
                (char*)pu8DomainName, g_host_ip_address[0], g_host_ip_address[1],
                g_host_ip_address[2], g_host_ip_address[3]);
        SYS_CONSOLE_PRINT("%s",message);

        do
        {
            // Create the socket
            new_socket = socket(AF_INET, SOCK_STREAM, 1);
            if (new_socket < 0)
            {
                SYS_CONSOLE_PRINT("Failed to create the socket.");

                // Set the state to disconnect from the cloud IoT
                g_cloud_wifi_state = CLOUD_STATE_CLOUD_DISCONNECT;

                // Break the do/while loop
                break;
            }

            // Set the socket address information
            socket_address.sin_family      = AF_INET;
            socket_address.sin_addr.s_addr = _htonl((uint32_t)((g_host_ip_address[0] << 24) |
                                                               (g_host_ip_address[1] << 16) |
                                                               (g_host_ip_address[2] << 8)  |
                                                               g_host_ip_address[3]));
            socket_address.sin_port        = _htons(CLOUD_PORT);

        #ifdef CLOUD_CONFIG_GCP
            int ssl_caching_enabled = 1;
            setsockopt(new_socket, SOL_SSL_SOCKET, SO_SSL_ENABLE_SESSION_CACHING, &ssl_caching_enabled, sizeof(ssl_caching_enabled));
        #else
            setsockopt(new_socket, SOL_SSL_SOCKET, SO_SSL_SNI,
                       CLOUD_ENDPOINT, sizeof(CLOUD_PORT) + 1);
        #endif

            // Connect to the cloud IoT server
            status = connect(new_socket, (struct sockaddr*)&socket_address,
                             sizeof(socket_address));
            if (status != SOCK_ERR_NO_ERROR)
            {
                memset(&message[0], 0, sizeof(message));
                sprintf(&message[0], "WINC1500 WIFI: Failed to connect to cloud Iot.");
                SYS_CONSOLE_PRINT("%s",message); 

                // Close the socket
                shutdown(new_socket);

                // Set the state to disconnect from the cloud IoT
                g_cloud_wifi_state = CLOUD_STATE_CLOUD_DISCONNECT;

                // Break the do/while loop
                break;
            }

            // Save the new socket connection information
            SYS_CONSOLE_PRINT("socket %d \r\n", new_socket);
            g_socket_connection.socket    = new_socket;
            g_socket_connection.address   = socket_address.sin_addr.s_addr;
            g_socket_connection.port      = CLOUD_PORT;
        }
        while (false);
    }
    else
    {
        // An error has occurred
        SYS_CONSOLE_PRINT("WINC1500 DNS lookup failed.");

        // Set the state to disconnect from the cloud IoT
        g_cloud_wifi_state = CLOUD_STATE_CLOUD_DISCONNECT;
    }

}

static void socket_callback_handler(SOCKET socket, uint8_t messageType, void *pMessage)
{
    tstrSocketConnectMsg *socket_connect_message = NULL;
    tstrSocketRecvMsg *socket_receive_message = NULL;
    int16_t *bytes_sent = NULL;

    switch (messageType)
    {
    case SOCKET_MSG_CONNECT:
    {
        socket_connect_message = (tstrSocketConnectMsg*)pMessage;

        if (NULL != socket_connect_message)
        {
            if (socket_connect_message->s8Error == SOCK_ERR_NO_ERROR)
            {
                // Set the state to connected to the cloud IoT
                g_cloud_wifi_state = CLOUD_STATE_CLOUD_CONNECTED;
            }
            else
            {
                // An error has occurred
                SYS_CONSOLE_PRINT("SOCKET_MSG_CONNECT error %s(%d)\r\n", get_socket_error_name(socket_connect_message->s8Error), socket_connect_message->s8Error);

                // Set the state to disconnect from the cloud_wifi_socket_handler IoT
                g_cloud_wifi_state = CLOUD_STATE_CLOUD_DISCONNECT;
            }
        }
        break;
    }

    case SOCKET_MSG_RECV:
    case SOCKET_MSG_RECVFROM:
    {
        socket_receive_message = (tstrSocketRecvMsg*)pMessage;
        if (socket_receive_message != NULL)
        {
            if (socket_receive_message->s16BufferSize >= 0)
            {
                g_rx_buffer_length += socket_receive_message->s16BufferSize;

                // The message was received
                if (socket_receive_message->u16RemainingSize == 0)
                {
                    g_wifi_status = WIFI_STATUS_MESSAGE_RECEIVED;
                }
                //printf("%s: SOCKET_MSG_RECV %d\r\n", __FUNCTION__, (int)socket_receive_message->s16BufferSize);
            }
            else
            {
                if (socket_receive_message->s16BufferSize == SOCK_ERR_TIMEOUT)
                {
                    // A timeout has occurred
                    g_wifi_status = WIFI_STATUS_TIMEOUT;
                }
                else
                {
                    // An error has occurred
                    g_wifi_status = WIFI_STATUS_ERROR;

                    // Set the state to disconnect from the cloud IoT
                    g_cloud_wifi_state = CLOUD_STATE_WIFI_DISCONNECT;
                }
            }
        }
        break;
    }

    case SOCKET_MSG_SEND:

        bytes_sent = (int16_t*)pMessage;

        if (*bytes_sent <= 0 || *bytes_sent > (int32_t)g_tx_size)
        {
            // Seen an odd instance where bytes_sent is way more than the requested bytes sent.
            // This happens when we're expecting an error, so were assuming this is an error
            // condition.
            g_wifi_status = WIFI_STATUS_ERROR;

            // Set the state to disconnect from the cloud IoT
            g_cloud_wifi_state = CLOUD_STATE_WIFI_DISCONNECT;
        }
        else if (*bytes_sent > 0)
        {
            // The message was sent
            g_wifi_status = WIFI_STATUS_MESSAGE_SENT;
        }
        break;
    default:
        SYS_CONSOLE_PRINT("%s: unhandled message %d\r\n", __FUNCTION__, (int)messageType);
        // Do nothing
        break;

    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Public Functions
// *****************************************************************************
// *****************************************************************************
int cloud_wifi_init(DRV_HANDLE handle)
{
    int8_t status = 0;
    memset(&m_cloudTask_connection, 0, sizeof(m_cloudTask_connection));
    extern ATCAIfaceCfg atecc608_0_init_data;
    // Initialize the cryptoauthlib stack
    atecc608_0_init_data.atcai2c.address = SECURE_ELEMENT_ADDRESS;
    if ((status = atcab_init(&atecc608_0_init_data)) != ATCA_SUCCESS)
    {
        return status;
    }
    config_set_thing_id();

   
    if ((status = WDRV_WINC_IPUseDHCPSet(handle, &APP_ExampleDHCPAddressEventCallback))  != WDRV_WINC_STATUS_OK)
    {
        return status;
    }

    /* Initialize the BSS context to use default values. */
    if ((status = WDRV_WINC_BSSCtxSetDefaults(&bssCtx)) != WDRV_WINC_STATUS_OK)
    {
        return status;
    }
        
    /* Update BSS context with target SSID for connection. */
    if ((status = WDRV_WINC_BSSCtxSetSSID(&bssCtx, (uint8_t*)ssid, strlen(ssid))) != WDRV_WINC_STATUS_OK)
    {
        return status;
    }

    /*if (WDRV_WINC_STATUS_OK != WDRV_WINC_AuthCtxSetOpen(&authCtx))
        {
        break;
        }*/

    /*Initialize the authentication context for WPA. */
    if ((status = WDRV_WINC_AuthCtxSetWPA(&authCtx, (uint8_t*)pass, strlen(pass))) != WDRV_WINC_STATUS_OK)
    {
        return status;
    }

    /* Register callback handler for DNS resolver . */
    if ((status = WDRV_WINC_SocketRegisterResolverCallback(handle, &cloud_dns_resolve_handler)) != WDRV_WINC_STATUS_OK)
    {
        return status;
    }

    /* Register callback handler for socket events. */
    if ((status = WDRV_WINC_SocketRegisterEventCallback(handle, &socket_callback_handler)) != WDRV_WINC_STATUS_OK)
    {
        return status;
    }

    //     ret = nm_get_firmware_full_info(&strtmp);
    //     if(M2M_ERR_FW_VER_MISMATCH == ret)
    //     {
    //         status = !0;
    //         break;
    //     }

    //     SYS_CONSOLE_PRINT("\nWINC1500 Firmware Data:\r\n");
    //     SYS_CONSOLE_PRINT("Firmware Ver: %u.%u.%u SVN Rev %u\r\n", strtmp.u8FirmwareMajor, strtmp.u8FirmwareMinor, strtmp.u8FirmwarePatch, strtmp.u16FirmwareSvnNum);
    //     SYS_CONSOLE_PRINT("Firmware Built at %s Time %s\r\n", strtmp.BuildDate, strtmp.BuildTime);
    //     SYS_CONSOLE_PRINT("Firmware Min Driver Ver: %u.%u.%u\r\n", strtmp.u8DriverMajor, strtmp.u8DriverMinor, strtmp.u8DriverPatch);
    //     SYS_CONSOLE_PRINT("Driver Ver: %u.%u.%u\r\n", M2M_RELEASE_VERSION_MAJOR_NO, M2M_RELEASE_VERSION_MINOR_NO, M2M_RELEASE_VERSION_PATCH_NO);
    //     SYS_CONSOLE_PRINT("Driver Built at %s Time %s\r\n\r\n", __DATE__, __TIME__);

    //     SYS_CONSOLE_PRINT("Secure Element Address: 0x%02X \r\n", SECURE_ELEMENT_ADDRESS);
    //     SYS_CONSOLE_PRINT("Cloud Endpoint: %s \r\n", CLOUD_ENDPOINT);
    // #ifdef CLOUD_CONNECT_WITH_CUSTOM_CERTS
    //     SYS_CONSOLE_PRINT("Connecting with CUSTOM certs\r\n\rn\n");
    // #else
    //     SYS_CONSOLE_PRINT("Connecting with MCHP certs\r\n\r\n");
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


    return status;
}
