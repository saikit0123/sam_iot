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
#include "atcacert.h"
#include "atcacert_def.h"
#include "tng_atcacert_client.h"
#include "atcacert_pem.h"
#include "tngtls_cert_def_1_signer.h"
#include "tng_atca.h"
#include "network_interface.h"
#include "MQTTClient.h"

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
#define CERT_SN_MAX_LEN             32
#define MAX_TLS_CERT_LENGTH         1024
#define SIGNER_CERT_MAX_LEN         600
#define DEVICE_CERT_MAX_LEN         600
#define TLS_SRV_ECDSA_CHAIN_FILE    "ECDSA.lst"
#define MAX_TLS_CERT_LENGTH         1024
#define TLS_FILE_NAME_MAX           48
#define INIT_CERT_BUFFER_LEN        (MAX_TLS_CERT_LENGTH*sizeof(uint32_t) - TLS_FILE_NAME_MAX*2 - SIGNER_CERT_MAX_LEN - DEVICE_CERT_MAX_LEN)
#define SSL_CIPHER_SUITE_SELECTION           SSL_NON_ECC_CIPHERS_AES_128
#define MQTT_COMMAND_TIMEOUT_MS     (4000)
#define MQTT_BUFFER_SIZE            (1024)

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
static cloudTaskConnection m_cloudTask_connection;
static Network g_mqtt_network;
MQTTClient g_mqtt_client;
static WDRV_WINC_BSS_CONTEXT bssCtx;
static WDRV_WINC_AUTH_CONTEXT authCtx;
static uint8_t g_host_ip_address[4];
//extern char config_thing_id[130];
char config_thing_id[130] =  "BAAAAADD1DBAAADD1D";
volatile enum cloud_iot_state g_cloud_wifi_state = CLOUD_STATE_WINC1500_INIT;
static socket_connection g_socket_connection;
static int32_t g_rx_buffer_length = 0;
static uint32_t g_tx_size = 0;
static cloudTask_status g_wifi_status = WIFI_STATUS_UNKNOWN;
static char ssid[MAX_WIFI_CREDENTIALS_LENGTH];
static char pass[MAX_WIFI_CREDENTIALS_LENGTH];
static uint32_t g_ecdh_key_slot_index = 0;
static uint16_t g_ecdh_key_slot[] = { 2 };
static uint8_t g_mqtt_rx_buffer[MQTT_BUFFER_SIZE];
static uint8_t g_mqtt_tx_buffer[MQTT_BUFFER_SIZE];
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
static int8_t transfer_ecc_certs_to_winc(void);
static int8_t ecc_transfer_certificates();
static const char* bin2hex(const void* data, size_t data_size);
static int8_t winc_certs_append_file_buf(uint32_t* buffer32, uint32_t buffer_size,
                                         const char* file_name, uint32_t file_size,
                                         const uint8_t* file_data);
static size_t winc_certs_get_total_files_size(const tstrTlsSrvSecHdr* header);
static void winc_wifi_ssl_callback(uint8_t u8MsgType, void *pvMsg);
static void ecc_process_request(tstrEccReqInfo *ecc_request);
static int8_t ecdh_derive_client_shared_secret(tstrECPoint *server_public_key,
                                               uint8_t *    ecdh_shared_secret,
                                               tstrECPoint *client_public_key);
static int8_t ecdh_derive_key_pair(tstrECPoint *server_public_key);
static int8_t ecdsa_process_sign_verify_request(uint32_t number_of_signatures);
static int8_t ecdsa_process_sign_gen_request(tstrEcdsaSignReqInfo *sign_request,
                                             uint8_t *             signature,
                                             uint16_t *            signature_size);
static int8_t ecdh_derive_server_shared_secret(uint16_t     private_key_id,
            tstrECPoint *client_public_key,
            uint8_t *    ecdh_shared_secret);

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************
static const char* bin2hex(const void* data, size_t data_size)
{
    static char buf[256];
    static char hex[] = "0123456789abcdef";
    const uint8_t* data8 = data;

    if (data_size * 2 > sizeof(buf) - 1)
    {
        return "[buf too small]";
    }

    for (size_t i = 0; i < data_size; i++)
    {
        buf[i * 2 + 0] = hex[(*data8) >> 4];
        buf[i * 2 + 1] = hex[(*data8) & 0xF];
        data8++;
    }
    buf[data_size * 2] = 0;

    return buf;
}

static int8_t ecdh_derive_client_shared_secret(tstrECPoint *server_public_key,
                                               uint8_t *    ecdh_shared_secret,
                                               tstrECPoint *client_public_key)
{
    int8_t status = M2M_ERR_FAIL;
    uint8_t ecdh_mode;
    uint16_t key_id;

    if ((g_ecdh_key_slot_index < 0) ||
        (g_ecdh_key_slot_index >= (sizeof(g_ecdh_key_slot) / sizeof(g_ecdh_key_slot[0]))))
    {
        g_ecdh_key_slot_index = 0;
    }

    if (_gDevice->mIface.mIfaceCFG->devtype == ATECC608A)
    {
        //do special ecdh functions for the 608, keep ephemeral keys in SRAM
        ecdh_mode = ECDH_MODE_SOURCE_TEMPKEY | ECDH_MODE_COPY_OUTPUT_BUFFER;
        key_id = GENKEY_PRIVATE_TO_TEMPKEY;
    }
    else
    {
        //specializations for the 508, use an EEPROM key slot
        ecdh_mode = ECDH_PREFIX_MODE;
        key_id = g_ecdh_key_slot[g_ecdh_key_slot_index];
        g_ecdh_key_slot_index++;
    }

    //generate an ephemeral key
    //TODO - add loop to make sure we get an acceptable private key
    if (atcab_genkey(key_id, client_public_key->X) == ATCA_SUCCESS)
    {
        client_public_key->u16Size = 32;
        //do the ecdh from the private key in tempkey, results put in ecdh_shared_secret
        if (atcab_ecdh_base(ecdh_mode, key_id, server_public_key->X, ecdh_shared_secret, NULL) == ATCA_SUCCESS)
        {
            status = M2M_SUCCESS;
        }
    }

    return status;
}

static int8_t ecdh_derive_key_pair(tstrECPoint *server_public_key)
{
    int8_t status = M2M_ERR_FAIL;

    if ((g_ecdh_key_slot_index < 0) ||
        (g_ecdh_key_slot_index >= (sizeof(g_ecdh_key_slot) / sizeof(g_ecdh_key_slot[0]))))
    {
        g_ecdh_key_slot_index = 0;
    }

    if ( (status = atcab_genkey(g_ecdh_key_slot[g_ecdh_key_slot_index], server_public_key->X) ) == ATCA_SUCCESS)
    {
        server_public_key->u16Size      = 32;
        server_public_key->u16PrivKeyID = g_ecdh_key_slot[g_ecdh_key_slot_index];

        g_ecdh_key_slot_index++;

        status = M2M_SUCCESS;
    }

    return status;
}

static int8_t ecdsa_process_sign_verify_request(uint32_t number_of_signatures)
{
    int8_t status = M2M_ERR_FAIL;
    tstrECPoint Key;
    uint32_t index = 0;
    uint8_t signature[80];
    uint8_t hash[80] = { 0 };
    uint16_t curve_type = 0;

    for (index = 0; index < number_of_signatures; index++)
    {
        status = m2m_ssl_retrieve_cert(&curve_type, hash, signature, &Key);

        if (status != M2M_SUCCESS)
        {
            M2M_ERR("m2m_ssl_retrieve_cert() failed with ret=%d", status);
            return status;
        }

        if (curve_type == EC_SECP256R1)
        {
            bool is_verified = false;

            status = atcab_verify_extern(hash, signature, Key.X, &is_verified);
            if (status == ATCA_SUCCESS)
            {
                status = (is_verified == true) ? M2M_SUCCESS : M2M_ERR_FAIL;
                if (is_verified == false)
                {
                    M2M_INFO("ECDSA SigVerif FAILED\n");
                }
            }
            else
            {
                status = M2M_ERR_FAIL;
            }

            if (status != M2M_SUCCESS)
            {
                m2m_ssl_stop_processing_certs();
                break;
            }
        }
    }

    return status;
}

static int8_t ecdsa_process_sign_gen_request(tstrEcdsaSignReqInfo *sign_request,
                                             uint8_t *             signature,
                                             uint16_t *            signature_size)
{
    int8_t status = M2M_ERR_FAIL;
    uint8_t hash[32];
    uint16_t device_key_slot = 0;

    status = m2m_ssl_retrieve_hash(hash, sign_request->u16HashSz);
    if (status != M2M_SUCCESS)
    {
        M2M_ERR("m2m_ssl_retrieve_hash() failed with ret=%d", status);
        return status;
    }

    if (sign_request->u16CurveType == EC_SECP256R1)
    {
        *signature_size = 64;
        status = atcab_sign(device_key_slot, hash, signature);
    }

    return status;
}

static int8_t ecdh_derive_server_shared_secret(uint16_t     private_key_id,
                                               tstrECPoint *client_public_key,
                                               uint8_t *    ecdh_shared_secret)
{
    uint16_t key_slot   = private_key_id;
    int8_t status = M2M_ERR_FAIL;
    uint8_t atca_status = ATCA_STATUS_UNKNOWN;

    atca_status = atcab_ecdh(key_slot, client_public_key->X, ecdh_shared_secret);
    if (atca_status == ATCA_SUCCESS)
    {
        status = M2M_SUCCESS;
    }
    else
    {
        M2M_INFO("__SLOT = %u, Err = %X\n", key_slot, atca_status);
    }

    return status;
}

static void ecc_process_request(tstrEccReqInfo *ecc_request)
{
    tstrEccReqInfo ecc_response;
    uint8_t signature[80];
    uint16_t response_data_size = 0;
    uint8_t *response_data_buffer = NULL;

    ecc_response.u16Status = 1;

    switch (ecc_request->u16REQ)
    {
    case ECC_REQ_CLIENT_ECDH:
        ecc_response.u16Status = ecdh_derive_client_shared_secret(&(ecc_request->strEcdhREQ.strPubKey),
                                                                  ecc_response.strEcdhREQ.au8Key,
                                                                  &ecc_response.strEcdhREQ.strPubKey);
        break;

    case ECC_REQ_GEN_KEY:
        ecc_response.u16Status = ecdh_derive_key_pair(&ecc_response.strEcdhREQ.strPubKey);
        break;

    case ECC_REQ_SERVER_ECDH:
        ecc_response.u16Status = ecdh_derive_server_shared_secret(ecc_request->strEcdhREQ.strPubKey.u16PrivKeyID,
                                                                  &(ecc_request->strEcdhREQ.strPubKey),
                                                                  ecc_response.strEcdhREQ.au8Key);
        break;

    case ECC_REQ_SIGN_VERIFY:
        ecc_response.u16Status = ecdsa_process_sign_verify_request(ecc_request->strEcdsaVerifyREQ.u32nSig);
        break;

    case ECC_REQ_SIGN_GEN:
        ecc_response.u16Status = ecdsa_process_sign_gen_request(&(ecc_request->strEcdsaSignREQ), signature,
                                                                &response_data_size);
        response_data_buffer = signature;
        break;

    default:
        // Do nothing
        break;
    }

    ecc_response.u16REQ      = ecc_request->u16REQ;
    ecc_response.u32UserData = ecc_request->u32UserData;
    ecc_response.u32SeqNo    = ecc_request->u32SeqNo;

    m2m_ssl_ecc_process_done();
    m2m_ssl_handshake_rsp(&ecc_response, response_data_buffer, response_data_size);
}

static void winc_wifi_ssl_callback(uint8_t u8MsgType, void *pvMsg)
{
    tstrEccReqInfo *ecc_request = NULL;

    switch (u8MsgType)
    {
    case M2M_SSL_REQ_ECC:
        ecc_request = (tstrEccReqInfo*)pvMsg;
        ecc_process_request(ecc_request);
        break;

    case M2M_SSL_RESP_SET_CS_LIST:
    default:
        // Do nothing
        break;
    }
}

static size_t winc_certs_get_total_files_size(const tstrTlsSrvSecHdr* header)
{
    uint8_t *pBuffer = (uint8_t*)header;
    uint16_t count = 0;

    while ((*pBuffer) == 0xFF)
    {

        if (count == INIT_CERT_BUFFER_LEN)
        {
            break;
        }
        count++;
        pBuffer++;
    }

    if (count == INIT_CERT_BUFFER_LEN)
    {
        return sizeof(*header); // Buffer is empty, no files

    }
    return header->u32NextWriteAddr;
}

static int8_t winc_certs_append_file_buf(uint32_t* buffer32, uint32_t buffer_size,
                                         const char* file_name, uint32_t file_size,
                                         const uint8_t* file_data)
{
    tstrTlsSrvSecHdr* header = (tstrTlsSrvSecHdr*)buffer32;
    tstrTlsSrvSecFileEntry* file_entry = NULL;
    uint16_t str_size = (uint8_t)strlen((char*)file_name) + 1;
    uint16_t count = 0;
    uint8_t *pBuffer = (uint8_t*)buffer32;

    while ((*pBuffer) == 0xFF)
    {

        if (count == INIT_CERT_BUFFER_LEN)
        {
            break;
        }
        count++;
        pBuffer++;
    }

    if (count == INIT_CERT_BUFFER_LEN)
    {
        // The WINC will need to add the reference start pattern to the header
        header->u32nEntries = 0;                    // No certs
        // The WINC will need to add the offset of the flash were the certificates are stored to this address
        header->u32NextWriteAddr = sizeof(*header); // Next cert will be written after the header
    }

    if (header->u32nEntries >= sizeof(header->astrEntries) / sizeof(header->astrEntries[0]))
    {
        return M2M_ERR_FAIL; // Already at max number of files

    }
    if ((header->u32NextWriteAddr + file_size) > buffer_size)
    {
        return M2M_ERR_FAIL; // Not enough space in buffer for new file

    }
    file_entry = &header->astrEntries[header->u32nEntries];
    header->u32nEntries++;

    if (str_size > sizeof(file_entry->acFileName))
    {
        return M2M_ERR_FAIL; // File name too long
    }
    memcpy((uint8_t*)file_entry->acFileName, (uint8_t*)file_name, str_size);

    file_entry->u32FileSize = file_size;
    file_entry->u32FileAddr = header->u32NextWriteAddr;
    header->u32NextWriteAddr += file_size;

    // Use memmove to accommodate optimizations where the file data is temporarily stored
    // in buffer32
    memmove(((uint8_t*)buffer32) + (file_entry->u32FileAddr), (uint8_t*)file_data, file_size);

    return M2M_SUCCESS;
}

static int8_t ecc_transfer_certificates()
{
    int8_t status = M2M_SUCCESS;
    int atca_status = ATCACERT_E_SUCCESS;
    uint8_t *signer_cert = NULL;
    size_t signer_cert_size;
#ifdef CLOUD_CONNECT_WITH_CUSTOM_CERTS
    uint8_t signer_public_key[SIGNER_PUBLIC_KEY_MAX_LEN];
#endif
    uint8_t *device_cert = NULL;
    size_t device_cert_size;
    uint8_t cert_sn[CERT_SN_MAX_LEN];
    size_t cert_sn_size;
    uint8_t *file_list = NULL;
    char *device_cert_filename = NULL;
    char *signer_cert_filename = NULL;
    uint32_t sector_buffer[MAX_TLS_CERT_LENGTH];
    char pem_cert[1024];
    size_t pem_cert_size;
    const atcacert_def_t *device_cert_def, *signer_cert_def;

    // Clear cert chain buffer
    memset(sector_buffer, 0xFF, sizeof(sector_buffer));

    // Use the end of the sector buffer to temporarily hold the data to save RAM
    file_list   = ((uint8_t*)sector_buffer) + (sizeof(sector_buffer) - TLS_FILE_NAME_MAX * 2);
    signer_cert = file_list - SIGNER_CERT_MAX_LEN;
    device_cert = signer_cert - DEVICE_CERT_MAX_LEN;

    // Init the file list
    memset(file_list, 0, TLS_FILE_NAME_MAX * 2);
    device_cert_filename = (char*)&file_list[0];
    signer_cert_filename = (char*)&file_list[TLS_FILE_NAME_MAX];

#ifdef CLOUD_CONNECT_WITH_CUSTOM_CERTS 
    // Uncompress the signer certificate from the ATECCx08A device
    signer_cert_size = SIGNER_CERT_MAX_LEN;
    if(ATCACERT_E_SUCCESS != (atca_status = atcacert_read_cert(&g_cert_def_1_signer, g_cert_ca_public_key_1_signer,
                                    signer_cert, &signer_cert_size)))
    {
        return 1;
    }
    pem_cert_size = sizeof(pem_cert);
    atcacert_encode_pem_cert(signer_cert, signer_cert_size, pem_cert, &pem_cert_size);
    SYS_CONSOLE_PRINT("Signer Cert : \r\n%s\r\n", pem_cert);

    // Get the signer's public key from its certificate
    if(ATCACERT_E_SUCCESS != (atca_status = atcacert_get_subj_public_key(&g_cert_def_1_signer, signer_cert,
                                    signer_cert_size, signer_public_key)))
    {
        return 1;
    }

    // Uncompress the device certificate from the ATECCx08A device.
    device_cert_size = DEVICE_CERT_MAX_LEN;
    if(ATCACERT_E_SUCCESS != (atca_status = atcacert_read_cert(&g_cert_def_2_device, signer_public_key,
                                    device_cert, &device_cert_size)))
    {
        return 1;
    }
    pem_cert_size = sizeof(pem_cert);
    atcacert_encode_pem_cert(device_cert, device_cert_size, pem_cert, &pem_cert_size);
    SYS_CONSOLE_PRINT("Device Cert : \r\n%s\r\n", pem_cert);

    if(ATCACERT_E_SUCCESS != (atca_status = atcacert_get_subj_key_id(&g_cert_def_2_device, device_cert,
                                    device_cert_size, subject_key_id)))
    {
        return 1;
    }

    signer_cert_def = &g_cert_def_1_signer;
    device_cert_def = &g_cert_def_2_device;
#else
    // Uncompress the signer certificate from the ATECCx08A device
    if(ATCACERT_E_SUCCESS != (atca_status = tng_atcacert_max_signer_cert_size(&signer_cert_size)))
    {
        return 1;
    }

    if(ATCACERT_E_SUCCESS != (atca_status = tng_atcacert_read_signer_cert(signer_cert, &signer_cert_size)))
    {
        return 1;
    }
    pem_cert_size = sizeof(pem_cert);
    atcacert_encode_pem_cert(signer_cert, signer_cert_size, pem_cert, &pem_cert_size);
    // SYS_CONSOLE_PRINT("Signer Cert : \r\n%s\r\n", pem_cert);

    // Uncompress the device certificate from the ATECCx08A device.
    if(ATCACERT_E_SUCCESS != (atca_status = tng_atcacert_max_device_cert_size(&device_cert_size)))
    {
        return 1;
    }
    if(ATCACERT_E_SUCCESS != (atca_status = tng_atcacert_read_device_cert(device_cert, &device_cert_size, NULL)))
    {
        return 1;
    }
    pem_cert_size = sizeof(pem_cert);
    atcacert_encode_pem_cert(device_cert, device_cert_size, pem_cert, &pem_cert_size);
    //SYS_CONSOLE_PRINT("Device Cert : \r\n%s\r\n", pem_cert);

    signer_cert_def = &g_tngtls_cert_def_1_signer;
    if(ATCA_SUCCESS != (atca_status = tng_get_device_cert_def(&device_cert_def)))
    {
        return 1;
    }
#endif

    // Get the device certificate SN for the filename
    cert_sn_size = sizeof(cert_sn);
    if(ATCACERT_E_SUCCESS != (atca_status = atcacert_get_cert_sn(device_cert_def, device_cert,
                                        device_cert_size, cert_sn, &cert_sn_size)))
    {
        return 1;
    }

    // Build the device certificate filename
    strcpy(device_cert_filename, "CERT_");
    strcat(device_cert_filename, bin2hex(cert_sn, cert_sn_size));

    // Add the DER device certificate the TLS certs buffer
    status = winc_certs_append_file_buf(sector_buffer, sizeof(sector_buffer),
                                        device_cert_filename, device_cert_size, device_cert);
    if (status != M2M_SUCCESS)
    {
        return status;
    }

    device_cert = NULL; // Make sure we don't use this now that it has moved

    // Get the signer certificate SN for the filename
    cert_sn_size = sizeof(cert_sn);
    if(ATCACERT_E_SUCCESS != (atca_status = atcacert_get_cert_sn(signer_cert_def, signer_cert,
                                        signer_cert_size, cert_sn, &cert_sn_size)))
    {
        return 1;
    }


    // Build the signer certificate filename
    strcpy(signer_cert_filename, "CERT_");
    strcat(signer_cert_filename, bin2hex(cert_sn, cert_sn_size));

    // Add the DER signer certificate the TLS certs buffer
    status = winc_certs_append_file_buf(sector_buffer, sizeof(sector_buffer),
                                        signer_cert_filename, signer_cert_size, signer_cert);
    if (status != M2M_SUCCESS)
    {
        return status;
    }

    // Add the cert chain list file to the TLS certs buffer
    status = winc_certs_append_file_buf(sector_buffer, sizeof(sector_buffer),
                                        TLS_SRV_ECDSA_CHAIN_FILE, TLS_FILE_NAME_MAX * 2, file_list);
    if (status != M2M_SUCCESS)
    {
        return status;
    }

    file_list = NULL;
    signer_cert_filename = NULL;
    device_cert_filename = NULL;

    // Update the TLS cert chain on the WINC.
    status = m2m_ssl_send_certs_to_winc((uint8_t*)sector_buffer,
                                        (uint32_t)winc_certs_get_total_files_size((tstrTlsSrvSecHdr*)sector_buffer));
    if (status != M2M_SUCCESS)
    {
        return 1;
    }

    if (atca_status)
    {
        M2M_ERR("eccSendCertsToWINC() failed with ret=%d", atca_status);
        status =  M2M_ERR_FAIL;
    }

    return status;
}

static int8_t transfer_ecc_certs_to_winc(void)
{
    int8_t ret_value;

    if ((ret_value = ecc_transfer_certificates()) != M2M_SUCCESS)
    {
        return 1;
    }

    if ((ret_value = m2m_ssl_init(winc_wifi_ssl_callback)) != M2M_SUCCESS)
    {
        return 1;
    }

    return ret_value;
}

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

    tstrM2mRev strtmp;
    int8_t ret = nm_get_firmware_full_info(&strtmp);
    if(M2M_ERR_FW_VER_MISMATCH == ret)
    {
        status = !0;
        return status;
    }

    SYS_CONSOLE_PRINT("\nWINC1500 Firmware Data:\r\n");
    SYS_CONSOLE_PRINT("Firmware Ver: %u.%u.%u SVN Rev %u\r\n", strtmp.u8FirmwareMajor, strtmp.u8FirmwareMinor, strtmp.u8FirmwarePatch, strtmp.u16FirmwareSvnNum);
    SYS_CONSOLE_PRINT("Firmware Built at %s Time %s\r\n", strtmp.BuildDate, strtmp.BuildTime);
    SYS_CONSOLE_PRINT("Firmware Min Driver Ver: %u.%u.%u\r\n", strtmp.u8DriverMajor, strtmp.u8DriverMinor, strtmp.u8DriverPatch);
    SYS_CONSOLE_PRINT("Driver Ver: %u.%u.%u\r\n", M2M_RELEASE_VERSION_MAJOR_NO, M2M_RELEASE_VERSION_MINOR_NO, M2M_RELEASE_VERSION_PATCH_NO);
    SYS_CONSOLE_PRINT("Driver Built at %s Time %s\r\n\r\n", __DATE__, __TIME__);

    SYS_CONSOLE_PRINT("Secure Element Address: 0x%02X \r\n", SECURE_ELEMENT_ADDRESS);
    SYS_CONSOLE_PRINT("Cloud Endpoint: %s \r\n", CLOUD_ENDPOINT);
#ifdef CLOUD_CONNECT_WITH_CUSTOM_CERTS
    SYS_CONSOLE_PRINT("Connecting with CUSTOM certs\r\n\rn\n");
#else
    SYS_CONSOLE_PRINT("Connecting with MCHP certs\r\n\r\n");
#endif

#ifndef CLOUD_CONFIG_GCP
    if( !transfer_ecc_certs_to_winc() )
    {
        return 1;
    }
#endif

    if ((status = m2m_ssl_set_active_ciphersuites(SSL_CIPHER_SUITE_SELECTION)) != M2M_SUCCESS)
    {
        return 1;
    }

    // Initialize the MQTT library
    g_mqtt_network.mqttread  = &mqtt_packet_read;
    g_mqtt_network.mqttwrite = &mqtt_packet_write;

    MQTTClientInit(&g_mqtt_client, &g_mqtt_network, MQTT_COMMAND_TIMEOUT_MS,
                    g_mqtt_tx_buffer, sizeof(g_mqtt_tx_buffer),
                    g_mqtt_rx_buffer, sizeof(g_mqtt_rx_buffer));


    return status;
}
