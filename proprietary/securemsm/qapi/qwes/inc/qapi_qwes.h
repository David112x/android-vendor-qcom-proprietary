// ============================================================================
// Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// ============================================================================

/** @file qapi_qwes.h

 @brief QWES Agent QAPI

 QWES stands for Qualcomm Wireless Edge Service

 Sample feature query code (Error handling omitted for clarity)
 @code
    QCBOREncode_Init(&ECtx, buffer, buffer_len);

    // All the feature requests are grouped in a CBOR Map
    QCBOREncode_OpenMap(&ECtx);

    // Add Blob version to Map
    QCBOREncode_AddInt64ToMap(&ECtx, "blobVersion", 0);

    // Add Feature ID Array to Map
    // UsefulBufC DummyIsv = SZToUsefulBufC("ISV Hash");
    QCBOREncode_OpenArrayInMap(&ECtx, "FeatureIDs");
    QCBOREncode_OpenArray(&ECtx);
    QCBOREncode_AddInt64(&ECtx, MY_FEATURE_ID);
    QCBOREncode_AddBytes(&ECtx, NULLUsefulBufC);    // empty client ID
    QCBOREncode_CloseArray(&ECtx);
    QCBOREncode_CloseArray(&ECtx);

    QCBOREncode_CloseMap(&ECtx);

    size_t request_len = 0;
    QCBOREncode_Finish(&ECtx, request_len);

    uint8_t response[1024] = {0};
    size_t resp_len = 0;
    qapi_QWES_Agent_Check_Licenses(buffer, request_len,
                                   response, sizeof(response), &resp_len);
 @endcode

 Example of parsing the data returned from qapi_QWES_Agent_Check_Licenses.
 Most error handling is omitted for clarity.
 @code
    UsefulBufC uf = {response, resp_len};
    QCBORDecodeContext DCtx;
    QCBORDecode_Init(&DCtx, uf, QCBOR_DECODE_MODE_NORMAL);

    QCBORItem Item;
    QCBORDecode_GetNext(&DCtx, &Item));
    if (Item.val.uCount < 1) {
        // No licenses enable the feature
        return -1;
    }

    // Example code: Just look at the first entry.
    // Inner array / structure
    QCBORDecode_GetNext(&DCtx, &Item);

    QCBORItem id;
    QCBORDecode_GetNext(&DCtx, &id);

    QCBORItem hash;
    QCBORDecode_GetNext(&DCtx, &hash);

    QCBORItem serial;
    QCBORDecode_GetNext(&DCtx, &serial);

    QCBORItem status;
    QCBORDecode_GetNext(&DCtx, &status);
    if (QAPI_QWES_LICENSE_OK_E == status.val.int64) {
        return 0;  // feature is licensed.
    }
    return -1;  // feature is not licensed.
 @endcode

*/

#ifndef QAPI_QWES_H_
#define QAPI_QWES_H_

#include "qapi_status.h"

#define QAPI_MOD_QWES (22)
#define QWES_ERR(x) ((qapi_Status_t)(__QAPI_ERROR(QAPI_MOD_QWES, x)))

#define QAPI_ERR_QWES_ATTESTATION_VERIFY_FAIL       QWES_ERR(1)
#define QAPI_ERR_QWES_DEVICE_NOT_CLAIMED            QWES_ERR(2)
#define QAPI_ERR_QWES_SERVICE_ID_MISMATCH           QWES_ERR(3)
#define QAPI_ERR_QWES_CERT_DENIED_DEVICE_REVOKED    QWES_ERR(4)
#define QAPI_ERR_QWES_CERT_DENIED_DEVICE_SUSPENDED  QWES_ERR(5)
#define QAPI_ERR_QWES_CERT_DENIED_DEVICE_POLICY     QWES_ERR(6)
#define QAPI_ERR_CERT_EXPIRED_OR_NOTYETVALID        QWES_ERR(7)
#define QAPI_ERR_CERT_OEM                           QWES_ERR(8)
#define QAPI_ERR_CERT_HWVERSION                     QWES_ERR(9)
#define QAPI_ERR_CERT_DEVICEID                      QWES_ERR(10)
#define QAPI_ERR_INVALID_PFM_EXTENSION              QWES_ERR(11)
#define QAPI_ERR_PFMFILER_FAILED                    QWES_ERR(12)
#define QAPI_ERR_QWES_BAD_TOKEN_CERT                QWES_ERR(13)
#define QAPI_ERR_CERT_PRODUCT_ID                    QWES_ERR(14)

#ifndef IPFM_LICENSE_HAS_OEMID
#include <stdint.h>
#define IPFM_LICENSE_HAS_OEMID UINT64_C(0x00000001)
#define IPFM_LICENSE_HAS_HWVERSION UINT64_C(0x00000002)
#define IPFM_LICENSE_HAS_DEVICEID UINT64_C(0x00000004)
#define IPFM_LICENSE_HAS_PKHASH UINT64_C(0x00000008)
#define IPFM_LICENSE_HAS_LICENSEEHASH UINT64_C(0x00000010)
#define IPFM_LICENSE_HAS_VALIDTIME UINT64_C(0x00000020)
#define IPFM_LICENSE_HAS_PRODUCTID UINT64_C(0x00000040)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** @brief maximum certificate ID length */
#ifndef QAPI_QWES_CERT_ID_MAX_LEN
#define QAPI_QWES_CERT_ID_MAX_LEN 64
#endif

/**
 * @brief Hashing Algorithms
 *
 * When signing or verifying a signature,
 * only one of these values should be passed.
 */
typedef enum {
    QAPI_QWES_HASH_NONE_E,
    QAPI_QWES_HASH_MD5_E,
    QAPI_QWES_HASH_SHA1_E,
    QAPI_QWES_HASH_SHA2_224_E,
    QAPI_QWES_HASH_SHA2_256_E,
    QAPI_QWES_HASH_SHA2_384_E,
    QAPI_QWES_HASH_SHA2_512_E,
} qapi_QWES_Hash_Type_e;

/** @name Certificate management
 *  Methods and types used to generate signed TLS certs.
 */
//!@{
/** @brief Certificate handle */
typedef int32_t qapi_QWES_Cert_Handle_t;

/** @brief Callback to receive generated certificate */
typedef void (*qapi_QWES_Cert_Callback_CB)(void* context,
                                          qapi_Status_t status,
                                          qapi_QWES_Cert_Handle_t cert_handle);

/** @brief Callback to receive Sync notification
 *
 * This function is called after the qwesd service communicates with the cloud.
 *
 * @param[in] context User-defined data
 * @param[in] status QAPI_OK for success,
 * QAPI_ERR_NO_RESOURCE if no network connection could be established,
 * QAPI_ERROR for any other failure.
 *
 * @param[in] sync_interval The new sync interval (in seconds) from the cloud.
 * When using Force_Sync, this value will normally be zero.
 * */
typedef void (*qapi_QWES_Force_Sync_CB)(void* context,
                                        qapi_Status_t status);

/** @brief Callback to receive generated attestation report
 *
 * This function is called after an asynchronous attestation succeeds or fails.
 * Note that report is only valid during this call.  It is the responsibility of
 * the callee to copy the report before returning.
 *
 * @param[in] context The context originally passed to the attest API
 * @param[in] status
 * @li QAPI_OK for success,
 * @li QAPI_ERR_NO_RESOURCE if no network connection could be established,
 * @li QAPI_ERR_NO_MEMORY if token_len is too small for generated haven token.
 *                    No data will be written to the token buffer, but
 *                    token_len_out will be set to the required size.
 * @li QAPI_ERR_INVALID_STATE if the device security has been compromised.
 * @li QAPI_ERR_QWES_BAD_TOKEN_CERT if the downloaded token cert could not be used.
 * @li QAPI_ERROR for any other failure.
 *
 * @param[in] report Pointer to report data, only valid for the length of this call.
 * @param[in] report_len Length of data (in bytes) pointed to by report.
 * */
typedef void (*qapi_QWES_Report_CB)(void* context,
                                    qapi_Status_t status,
                                    uint8_t* report,
                                    size_t report_len);

/** @brief Cert format when retrieving or registering a cert */
typedef enum {
    QAPI_QWES_CERT_FORMAT_PEM_E,
    QAPI_QWES_CERT_FORMAT_DER_E,
} qapi_QWES_Cert_Format_Type_e;

/**
* @brief Initializes QWES Agent
*/
qapi_Status_t qapi_QWES_Agent_Init(void);

/**
* @brief Cleans up resources acquired in qapi_QWES_Agent_Init
*/
void qapi_QWES_Agent_Exit(void);

/**
 * @brief Generate a new certificate
 *
 * @retval QAPI_ERR_NOT_SUPPORTED   Not currently supported
 */
qapi_Status_t qapi_QWES_Agent_Generate_Certificate(const char* service_id,
                                                   const uint8_t* csr_params,
                                                   size_t csr_params_len,
                                                   const uint8_t* key_params,
                                                   size_t key_params_len,
                                                   qapi_QWES_Cert_Callback_CB callback,
                                                   void* context);
/**
 * @brief Generate a TLS certificate request blob
 *
 * @retval QAPI_ERR_NOT_SUPPORTED   Not currently supported
 */
qapi_Status_t qapi_QWES_Agent_Get_CSR(const char* service_id,
                                      const uint8_t* csr_params,
                                      size_t csr_params_len,
                                      const uint8_t* key_params,
                                      size_t key_params_len,
                                      uint8_t* data,
                                      size_t data_len,
                                      size_t* data_len_out);

/**
 * @brief Register a cert returned from the server
 *
 * @retval QAPI_ERR_NOT_SUPPORTED   Not currently supported
 */
qapi_Status_t qapi_QWES_Agent_Save_Cert(uint8_t* cert,
                                        size_t cert_len,
                                        qapi_QWES_Cert_Handle_t* cert_handle_out);

/**
 * @brief Retrieve a certificate by ID
 *
 * @retval QAPI_ERR_NOT_SUPPORTED   Not currently supported
 */
qapi_Status_t qapi_QWES_Agent_Get_Cert_Handle(uint8_t* cert_id,
                                              size_t cert_id_len,
                                              qapi_QWES_Cert_Handle_t* cert_handle_out);
//!@}

/** @name Certificate Usage
 *  Methods and types used with certificate handles
 */
//!@{

/**
 * @brief Retrieve the certificate identifier
 *
 * @retval QAPI_ERR_NOT_SUPPORTED   Not currently supported
 */
qapi_Status_t qapi_QWES_Cert_Get_ID(qapi_QWES_Cert_Handle_t cert_handle,
                                    uint8_t* cert_id_out,
                                    size_t cert_id_len,
                                    size_t* cert_id_len_out);

/**
 * @brief Retrieve the full certificate data
 *
 * @retval QAPI_ERR_NOT_SUPPORTED   Not currently supported
 */
qapi_Status_t qapi_QWES_Cert_Format(qapi_QWES_Cert_Handle_t cert_handle,
                                    qapi_QWES_Cert_Format_Type_e format,
                                    uint8_t* data,
                                    size_t data_len,
                                    size_t* data_len_out);

/**
 * @brief Sign the given data with the certificate's private key
 *
 * @retval QAPI_ERR_NOT_SUPPORTED   Not currently supported
 */
qapi_Status_t qapi_QWES_Cert_Sign(qapi_QWES_Cert_Handle_t cert_handle,
                                  const uint8_t* data,
                                  size_t data_len,
                                  qapi_QWES_Hash_Type_e algorithm,
                                  uint8_t* sig,
                                  size_t sig_len,
                                  size_t* sig_len_out);

/**
 * @brief Decrypt the given data with the certificate's private key
 *
 * @retval QAPI_ERR_NOT_SUPPORTED   Not currently supported
 */
qapi_Status_t qapi_QWES_Cert_Decrypt(qapi_QWES_Cert_Handle_t cert_handle,
                                     const uint8_t* cipher,
                                     size_t cipher_len,
                                     uint8_t* data,
                                     size_t data_len,
                                     size_t* data_len_out);

/**
 * @brief Release the certificate
 *
 * This call frees up any underlying resources.  After this call, the
 * cert_handle is invalid and may no longer be used.
 */
qapi_Status_t qapi_QWES_Cert_Release(qapi_QWES_Cert_Handle_t cert_handle);

//!@}

/** @name Provisioning
 *  Methods and types used to retrieve provisioned data
 */
//!@{

/** @brief Opaque data buffer handle */
typedef int32_t qapi_QWES_Data_Handle_t;

/** @brief Callback to receive decrypted provisioned data
 *
 * The data handle will only be valid for the duration of the callback.
 *
 * @param[in] context           Application context
 * @param[in] status            QAPI_OK if the transaction succeeded.
 * @param[in] decrypted_data    Data handle
 * @param[in] data_len          Size of data, in bytes.
 **/
typedef void (*qapi_QWES_Provision_Callback_t)(void* context,
                                               qapi_Status_t status,
                                               qapi_QWES_Data_Handle_t decrypted_data,
                                               size_t data_len);

/**
 * @brief Retrive Provisioned data from the cloud
 *
 * Provisioning retrieves data from a cloud-based service.
 * This method will return as soon as the request is sent.
 * If the request is successful, the callback will be called asynchronously.
 * The callback will receive an opaque handle to the encrypted provisioned data,
 * and a handle to a key to decrypt the provisioned data.
 *
 * Note that the service_id string must be a value that is registered with the QWES
 * server, and is used by the QWES server to forward the request to the correct
 * service.
 *
 * Example of use:

TODO Update

 *
 * @code
 * const char* service_id = "12345678-1234-1234-1234-123456789012"
 * const char* opts = "[passwords, server_urls]";
 *
 * qapi_Status_t status = qapi_QWES_Agent_Get_Provisioned_Data(service_id, opts, callback, NULL);
 * if (QAPI_OK != status) {
 *    // Schedule a retry
 * }
 * @endcode
 *
 * @param[in] service_id    String that identifies the IoT app / service
 *                          Maximum length is 64 bytes, including terminator.
 * @param[in] request_opt   Request options.  App-defined, may pass NULL.
 * @param[in] opt_len       Options length, may be zero if request_opt is NULL.
 * @param[in] callback      Callback to call with provisioned data
 * @param[in] context       Data to pass to callback.
 *
 * @retval QAPI_OK on success
 * @retval QAPI_ERR_INVALID_PARAM if callback is null
 * @retval QAPI_ERROR if the server rejected the request
 * @retval QAPI_ERR_BUSY if the connection to the server failed.
 * @retval QAPI_ERR_INVALID_STATE if the device security has been compromised.
 */
qapi_Status_t qapi_QWES_Agent_Get_Provisioned_Data(
    const char* service_id,
    void* request_opt,
    size_t opt_len,
    qapi_QWES_Provision_Callback_t callback,
    void* context);

/**
 * @brief Generate a provisioning request blob
 *
 * @param[in] service_id            String that identifies the IoT app / service
 *                                  Maximum length is 64 bytes, including terminator.
 * @param[in] request_opt           Request options.  App-defined, may pass NULL.
 * @param[in] opt_len               Options length, may be zero if request_opt is NULL.
 * @param[out] data                 Buffer to hold request blob.
 * @param[in] data_len              Length of data buffer
 * @param[out] data_len_out         Data length
 *
 * @retval QAPI_ERR_NO_MEMORY if data_len is too small.
 *                    No data will be written to the data buffer, but
 *                    data_len_out will be set to the required size.
 */
qapi_Status_t qapi_QWES_Agent_Get_Provisioning_Blob(const char* service_id,
                                                    void* request_opt,
                                                    size_t opt_len,
                                                    uint8_t* data,
                                                    size_t data_len,
                                                    size_t* data_len_out);

/**
 * @brief Decrypt a COSE message generated by the server.
 *
 * During provisioning, the server is expected to generate a COSE_Encrypt
 * structure to securely transmit the provisioned data.  The payload
 * will be encrypted with a temporary Content Encryption Key (CEK).  The
 * CEK itself is also encrypted.
 *
 * This method will decrypt the CEK and use that CEK to decrypt the ciphertext.
 *
 * @code
 *  COSE_Encrypt = [
 *    Headers,
 *    ciphertext : bstr,
 *    recipients : [+COSE_recipient]
 *  ]
 * @endcode
 *
 * See @ref qapi_QWES_Agent_Get_CSR for details.
 *
 * @param[in] encrypted     Server-generated COSE_Encrypt structure
 * @param[in] encrypted_len Length of encrypted, in bytes.
 * @param[in] callback      Callback to call with decrypted data
 * @param[in] context       Data to pass to callback.
 */
qapi_Status_t qapi_QWES_Agent_Decrypt_Provisioned_Data(
    uint8_t* encrypted,
    size_t encrypted_len,
    qapi_QWES_Provision_Callback_t callback,
    void* context);

/**
 * @brief Retrieve data from a data handle
 *
 * Note that the offset parameter refers to the data _source_.  Bytes are
 * always written starting at the address passed as the 'data' argument.
 * If offset is >= the data source length, bytes_read will be set to zero.
 * Otherwise, bytes read will be the lesser of bytes_to_read and
 * data source length - offset.
 *
 * @param[in] decrypted_data_handle Handle to the data
 * @param[out] decrypted_data       Buffer to hold data
 * @param[in] offset                Offset into the managed data
 * @param[in] bytes_to_read         Number of bytes to read
 * @param[out] bytes_read           Number of bytes read
 */
qapi_Status_t qapi_QWES_Data_Get(qapi_QWES_Data_Handle_t decrypted_data_handle,
                                 uint8_t* decrypted_data,
                                 size_t offset,
                                 size_t bytes_to_read,
                                 size_t* bytes_read);

//!@}   End of Provisioning group

/** @name Attestation
 *  Methods and types used to generate signed TLS certs.
 */

///@{
/**
 * @brief Create attestation report with user data and nonce.
 *
 * @deprecated Pass a NULL token to @ref qapi_QWES_Agent_Create_Attestation_Report instead.
 *
 * Put user data and the nonce into a haven token and return the
 * token to the client.  The report will be base-64 encoded.
 *
 * The client will send the report to OEM server, which
 * will forward it to the QWES cloud.  The QWES cloud will unpack the report and
 * return the app data to the OEM server.
 *
 * @param[in] nonce             Transaction ID provided by the server
 * @param[in] nonce_len         Number of bytes in nonce
 * @param[in] data              Application-defined data, may be NULL
 * @param[in] data_len          Length of the data buffer, may be 0 if data is NULL
 * @param[out] report            Buffer to hold report
 * @param[in] report_len         Length of the report buffer
 * @param[out] report_len_out    Length of generated report.
 *
 * @retval QAPI_OK if the report was created successfully.
 * @retval QAPI_ERR_NO_MEMORY if report_len is too small for generated haven report.
 *                    No data will be written to the report buffer, but
 *                    report_len_out will be set to the required size.
 * @retval QAPI_ERR_INVALID_STATE if the device security has been compromised.
 * @retval QAPI_ERROR for other errors
 */
qapi_Status_t qapi_QWES_Agent_Attest(uint8_t* nonce,
                                     size_t nonce_len,
                                     const uint8_t* data,
                                     size_t data_len,
                                     uint8_t* report,
                                     size_t report_len,
                                     size_t* report_len_out);

/**
 * @brief Create attestation report with given token cert
 *
 * Put user data and the nonce into a haven token and return the
 * token to the client.  The token will be base-64 encoded.
 *
 * Client code will normally send the report to an OEM server, which
 * will forward it to the QWES cloud.  The QWES cloud will unpack the report and
 * return the app data to the OEM server.
 *
 * @param[in] token_cert        Token certificate from the QWES cloud, or NULL
 * @param[in] token_cert_len    Number of bytes in report_cert
 * @param[in] nonce             Nonce from the QWES cloud
 * @param[in] nonce_len         Number of bytes in nonce
 * @param[in] data              Application-defined data, may be NULL
 * @param[in] data_len          Length of the data buffer, may be 0 if data is NULL
 * @param[out] report            Buffer to hold report
 * @param[in] report_len         Length of the report buffer
 * @param[out] report_len_out    Length of generated report.
 *
 * @retval QAPI_OK if the report was created successfully.
 * @retval QAPI_ERR_NO_RESOURCE if the QWES service is not accessible.
 * @retval QAPI_ERR_NO_MEMORY if report_len is too small for generated haven report.
 *                    No data will be written to the report buffer, but
 *                    report_len_out will be set to the required size.
 * @retval QAPI_ERR_INVALID_STATE if the device security has been compromised.
 * @retval QAPI_ERR_QWES_BAD_TOKEN_CERT if the provided token cert could not be used.
 * @retval QAPI_ERROR for other errors
 */
qapi_Status_t qapi_QWES_Agent_Create_Attestation_Report(const uint8_t* token_cert,
                                                        size_t token_cert_len,
                                                        const uint8_t* nonce,
                                                        size_t nonce_len,
                                                        const uint8_t* data,
                                                        size_t data_len,
                                                        uint8_t* report,
                                                        size_t report_len,
                                                        size_t* report_len_out);

/**
 * @brief Set client credentials to be used in attestation report.
 *
 * If this function is called before creating an attestation report,
 * the subsequent attestation reports will contain the given buffer
 * as client credentials.  This function will normally only be called
 * by services that provide QWES functionality to other clients.
 *
 * @param[in] client_cred       Client credentials buffer
 * @param[in] client_cred_len   Number of bytes in client_cred
 *
 * @retval QAPI_OK if the report was created successfully.
 * @retval QAPI_ERR_NO_RESOURCE if the QWES service is not accessible.
 * @retval QAPI_ERR_NO_MEMORY if the client credentials could not be stored.
 * @retval QAPI_ERROR for other errors
 */
qapi_Status_t qapi_QWES_Agent_Set_Client_Credentials(const uint8_t* client_cred,
                                                     size_t client_cred_len);

/**
 * @brief Create attestation report with automatically downloaded items
 *
 * Network communication is time consuming, so this function works asynchronously
 * to avoid blocking the client code.  The callback function will be called
 * with the attestation report data on success, or an error code on failure.
 *
 * Client code will normally send the report to an OEM server, which
 * will forward it to the QWES cloud.  The QWES cloud will unpack the report and
 * return the app data to the OEM server.
 *
 * * If bring_up_network is non-zero, this function will attempt to
 * bring up a network connection.
 * See @ref qapi_QWES_Agent_Set_Network_Parameters for details.
 * Otherwise, it will use whatever network is available, according to the system
 * routing configuration.
 *
 * @param[in] data              Application-defined data, may be NULL
 * @param[in] data_len          Length of the data buffer, may be 0 if data is NULL
 * @param[in] bring_up_network  If non-zero, QWES may bring up a network connection
 * @param[in] context           Context to be passed to callback function
 * @param[in] callback          Pointer to a function that will be called
 *                              when the operation succeeds or fails.
 *
 * @retval QAPI_OK if the request task was started.
 *              In this case, callback will be called at some point in the future,
 *              and the client must make sure both the callback and context are valid
 *              until that occurs.
 * @retval QAPI_ERR_NO_MEMORY if a request task could not be started
 * @retval QAPI_ERROR for other errors
 */
qapi_Status_t qapi_QWES_Agent_Create_Attestation_Report_Auto(const uint8_t* data,
                                                             size_t data_len,
                                                             // TODO:  Option flags?
                                                             int bring_up_network,
                                                             void* context,
                                                             qapi_QWES_Report_CB callback);

///@}

/** @name Licensing
 *  Methods and types used to check licenses
 */

/**
 * @brief license status codes
 */
typedef enum {
    QAPI_QWES_LICENSE_NONE_E,     //!< No license found
    QAPI_QWES_LICENSE_OK_E,       //!< Valid license found
    QAPI_QWES_LICENSE_EXPIRED_E,  //!< Expired license found
} qapi_QWES_License_Status_e;

///@{
/**
 * @brief Check the license status of one or more feature,client pairs.
 *
 * The query is a CBOR array of feature id, ISV Hash pairs
 * @code
 * query = [ 1* query_entry ]
 * query_entry = (
 *     feature_id: uint,
 *     client_id: bstr,
 *     )
 * @endcode
 *
 * The response is an array of license serial number and status pairs
 * @code
 * response = [ 1* response_entry ]
 * response_entry = (
 *     feature_id: uint,
 *     license_hash: bstr,
 *     license_serial_num: bstr,
 *     status: uint,
 *     )
 * @endcode
 *
 * The status values match @ref qapi_QWES_License_Status_e.  If the status
 * is QAPI_QWES_LICENSE_NONE_E, the license_serial_num should be ignored.
 *
 * @param[in] query             Buffer containing CBOR query structure
 * @param[in] query_len         Length of the query buffer
 * @param[out] results          Buffer to hold CBOR results structure
 * @param[in] results_len       Length of the results buffer
 * @param[out] results_len_out  Length of generated results
 *
 * @retval QAPI_OK if the query was executed successfully.
 * @retval QAPI_ERR_INVALID_PARAM if any pointer is null.
 * @retval QAPI_ERR_NO_MEMORY if results_len is too small for generated haven results.
 *                    No data will be written to the results buffer, but
 *                    results_len_out will be set to the required size.
 * @retval QAPI_ERR_INVALID_STATE if the device security has been compromised.
 * @retval QAPI_ERROR for other errors
 */
qapi_Status_t qapi_QWES_Agent_Check_Licenses(const uint8_t* query,
                                             size_t query_len,
                                             uint8_t* results,
                                             size_t results_len,
                                             size_t* results_len_out);

///@{
/**
 * @brief Check the license status of one or more feature,client pairs against all installed licenses
 *
 * The query is a CBOR array of feature id, ISV Hash pairs
 * @code
 * query = [ 1* query_entry ]
 * query_entry = (
 *     feature_id: uint,
 *     client_id: bstr,
 *     )
 * @endcode
 *
 * The response is an array of license serial number and status pairs
 * @code
 * response = [ 1* response_entry ]
 * response_entry = (
 *     feature_id: uint,
 *     license_hash: bstr,
 *     [ 1* result ]
 *     )
 * result = (
 *     license_serial_num: bstr,
 *     status: uint
 *     )
 * @endcode
 *
 * The status values match @ref qapi_QWES_License_Status_e.  If the status
 * is QAPI_QWES_LICENSE_NONE_E, the license_serial_num should be ignored.
 *
 * @param[in] query             The feature ID to check enablement of.
 * @param[in] query_len         Length of the query buffer
 * @param[out] results          Buffer to hold CBOR results structure
 * @param[in] results_len       Length of the results buffer
 * @param[out] results_len_out  Length of generated results
 *
 * @retval QAPI_OK if the query was executed successfully.
 * @retval QAPI_ERR_INVALID_PARAM if any pointer is null.
 * @retval QAPI_ERR_NO_MEMORY if results_len is too small for generated haven results.
 *                    No data will be written to the results buffer, but
 *                    results_len_out will be set to the required size.
 * @retval QAPI_ERR_INVALID_STATE if the device security has been compromised.
 */
qapi_Status_t qapi_QWES_Agent_Check_All_Licenses(const uint8_t* query,
                                             size_t query_len,
                                             uint8_t* results,
                                             size_t results_len,
                                             size_t* results_len_out);

/**
 * @brief Register a license with the manager
 *
 * @param [in] license_buffer   Feature License Cert, in PEM or DER format.
 * @param [in] license_len      Number of bytes in license_buffer.
 * @param [out] serial          Buffer to hold license serial number.
 * @param [in] serial_len       Length of serial buffer
 * @param [out] serial_len_out  Data length
 * @param [out] info            Buffer to hold license feature ID information
 * @param [in] info_len         Length of info buffer
 * @param [out] info_len_out    Data length
 * @param [out] restrictions    Bitmask of restrictions satisfied by the license
 *
 * Data written to the info buffer is a CBOR structure.  The structure is
 * @code
 * feature_ids = {
 *   "FIDs" : [ 1* uint ],                  Array of feature IDs
 *   "FIDRanges" : [ 1 * (uint, uint) ]     Array of (first,last) ID ranges
 * }
 * @endcode
 *
 * @retval QAPI_OK if the license was installed successfully.
 * @retval QAPI_ERR_INVALID_STATE if the license manager is unavailable.
 * @retval QAPI_ERR_NO_MEMORY if serial_len or info_len is too small.
 *                    No data will be written to the buffer, but
 *                    serial_len_out and info_len_out will be set to the required size.
 * @retval QAPI_ERR_CERT_EXPIRED_OR_NOTYETVALID
 * @retval QAPI_ERR_CERT_OEM
 * @retval QAPI_ERR_CERT_HWVERSION
 * @retval QAPI_ERR_CERT_DEVICEID
 * @retval QAPI_ERR_INVALID_PFM_EXTENSION
 * @retval QAPI_ERR_PFMFILER_FAILED
 * @retval QAPI_ERR_INVALID_PARAM if the license manager rejects the license
 *                                for some reason not listed above.
 */
qapi_Status_t qapi_QWES_Agent_Install_License(const uint8_t* license_buffer,
                                              size_t license_len,
                                              uint8_t* serial,
                                              size_t serial_len,
                                              size_t* serial_len_out,
                                              uint8_t* info,
                                              size_t info_len,
                                              size_t* info_len_out,
                                              uint64_t* restrictions);

/**
 * @brief Retrieve license meta-data and extra data.
 *
 * Data written to the info buffer is a CBOR structure.  The structure is
 * @code
 * info = {
 *   "FileName" : tstr
 *   "SerialNumber" : bstr
 *   "Issuer" : tstr
 *   "SubjectKeyID" : bstr
 *   "SubjectDN" : tstr
 *   "Expiration" : bstr
 *   "LicenseeHashes" : bstr
 *   "FIDs" : [1 * uint ]
 *   "FIDRanges" : [1 * (uint, uint)]
 *   "LicenseRestrictions" : uint
 *   "ExtraData" : bstr
 *   "Flag" : uint
 * }
 * @endcode
 *
 * @retval QAPI_OK if the license was installed successfully.
 * @retval QAPI_ERROR if info retrieval failed for any reason.
 */
qapi_Status_t qapi_QWES_Agent_Get_License_Info(const uint8_t* serial,
                                               size_t serial_len,
                                               uint8_t* info,
                                               size_t info_len,
                                               size_t* info_len_out);

/**
 * @brief Define network connection parameters to override defaults.
 *
 * @param [in] param_cbor   CBOR map containing parameter identifiers and the
 *                          corresponding value to be used.
 * @param [in] param_cbor_len   Length of the param_cbor buffer, in bytes.
 *
 * Possible map key values (typically map contains one of the below):
 * Value | Meaning
 * ------|-------
 *   1   | UMTS profile ID, 0=default
 *   2   | APN name
 *   6   | CDMA profile ID
 *
 * The full list of options is shown below.
 * See QTI DSI NetCtrl documentation for details.
 *
 * CDDL
 * @code
 * Params: map() {
 *     ?1        => int;        // UMTS profile ID
 *     ?2        => tstr;       // APN name
 *     ?3        => tstr;       // APN user name
 *     ?4        => tstr;       // APN password
 *     ?5        => int;        // Authentication preference
 *     ?6        => int;        // CDMA Profile ID
 *     ?7        => int;        // IP_ADDR
 *     ?8        => tstr;       // DEVICE_NAME
 *     ?9        => int;        // Technology preference
 *     ?10       => int;        // CALL_TYPE
 *     ?11       => int;        // IP family (IPV4=4, IPV6=6, IPV4_or_V6=10)
 *     ?12       => int;        // EXT_TECH_PREF
 *     ?13       => int;        // PARTIAL_RETRY
 *     ?14       => int;        // APP_TYPE
 * }
 * @endcode
 *
 * Typical usage is to either set the profile idx (0 means default) or set a
 * specific APN.  Example code snippets below:
 *
 * (set profile idx to 0, for either CDMA or UMTS connection)
 * @code
 *    CBOREncodeContext ECtx;
 *    uint8_t pEncoded[500];
 *    size_t nEncodedLen = sizeof(pEncoded);
 *    EncodedCBOR Enc;
 *
 *    CBOREncode_Init(&ECtx, pEncoded, nEncodedLen);
 *    CBOREncode_OpenMap(&ECtx);
 *    CBOREncode_AddInt64ToMapN(&ECtx, DSI_CALL_INFO_CDMA_PROFILE_IDX, 0);
 *    CBOREncode_AddInt64ToMapN(&ECtx, DSI_CALL_INFO_UMTS_PROFILE_IDX, 0);
 *    CBOREncode_CloseMap(&ECtx);
 *    if(CBOREncode_Finish2(&ECtx, &Enc)) {
 *        return QAPI_ERROR;
 *    }
 *    return qapi_QWES_Agent_Set_Network_Parameters(Enc.Bytes.ptr, Enc.Bytes.len);
 * @endcode
 *
 * (set APN name to < param_APN >)
 * @code
 *    CBOREncodeContext ECtx;
 *    uint8_t pEncoded[500];
 *    size_t nEncodedLen = sizeof(pEncoded);
 *    EncodedCBOR Enc;
 *
 *    CBOREncode_Init(&ECtx, pEncoded, nEncodedLen);
 *    CBOREncode_OpenMap(&ECtx);
 *    CBOREncode_AddSZStringToMapN(&ECtx, DSI_CALL_INFO_APN_NAME, param_APN);
 *    CBOREncode_CloseMap(&ECtx);
 *    if(CBOREncode_Finish2(&ECtx, &Enc)) {
 *        return QAPI_ERROR;
 *    }
 *    return qapi_QWES_Agent_Set_Network_Parameters(Enc.Bytes.ptr, Enc.Bytes.len);
 * @endcode
 *
 * @retval QAPI_OK if the request was sent to the QWES service successfully
 * @retval QAPI_ERROR otherwise
 */
qapi_Status_t qapi_QWES_Agent_Set_Network_Parameters(uint8_t* param_cbor,
                                                     size_t param_cbor_len);
/**
 * @brief Register a callback method for feature license update events.
 *
 * A feature license update event occurs when a license is installed, removed,
 * updated, or expires.  When the event occurs, it is the responsibility of the
 * client to check the feature ID or IDs that the client needs.
 *
 * The "Sync" in the name is left over from when the only notifications were
 * triggered by the QWES Sync service.
 *
 * @param[in] cb        Function to call when a feature license event occurs
 * @param[in] context   User-defined data, passed to cb when the event occurs
 * @param[out] handle   Opaque handle used to unregister callback
 */
qapi_Status_t qapi_QWES_Agent_Register_Sync_Callback(qapi_QWES_Force_Sync_CB cb, void* context, void** handle);

/**
 * @brief Un-Register a callback method for sync events.
 *
 * After this method is called, the handle is not valid and must not be used.
 *
 * @param[in] handle    Handle produced by @ref qapi_QWES_Agent_Register_Sync_Callback
 */
qapi_Status_t qapi_QWES_Agent_Unregister_Sync_Callback(void* handle);

/**
 * @brief   Command the QWES service to synchronize licenses with the cloud
 */
qapi_Status_t qapi_QWES_Agent_Force_Sync(void);

///@}
#ifdef __cplusplus
}
#endif

#endif
