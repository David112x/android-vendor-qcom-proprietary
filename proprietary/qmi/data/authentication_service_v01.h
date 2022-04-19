#ifndef AUTH_SERVICE_01_H
#define AUTH_SERVICE_01_H
/**
  @file authentication_service_v01.h

  @brief This is the public header file which defines the auth service Data structures.

  This header file defines the types and structures that were defined in
  auth. It contains the constant values defined, enums, structures,
  messages, and service message IDs (in that order) Structures that were
  defined in the IDL as messages contain mandatory elements, optional
  elements, a combination of mandatory and optional elements (mandatory
  always come before optionals in the structure), or nothing (null message)

  An optional element in a message is preceded by a uint8_t value that must be
  set to true if the element is going to be included. When decoding a received
  message, the uint8_t values will be set to true or false by the decode
  routine, and should be checked before accessing the values that they
  correspond to.

  Variable sized arrays are defined as static sized arrays with an unsigned
  integer (32 bit) preceding it that must be set to the number of elements
  in the array that are valid. For Example:

  uint32_t test_opaque_len;
  uint8_t test_opaque[16];

  If only 4 elements are added to test_opaque[] then test_opaque_len must be
  set to 4 before sending the message.  When decoding, the _len value is set
  by the decode routine and should be checked so that the correct number of
  elements in the array will be accessed.

*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  (c) 2011-2018 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.14.7
   It was generated on: Sat Dec  7 2019 (Spin 0)
   From IDL File: authentication_service_v01.idl */

/** @defgroup auth_qmi_consts Constant values defined in the IDL */
/** @defgroup auth_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup auth_qmi_enums Enumerated types used in QMI messages */
/** @defgroup auth_qmi_messages Structures sent as QMI messages */
/** @defgroup auth_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup auth_qmi_accessor Accessor for QMI service object */
/** @defgroup auth_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup auth_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define AUTH_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define AUTH_V01_IDL_MINOR_VERS 0x0E
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define AUTH_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define AUTH_V01_MAX_MESSAGE_ID 0x0032
/**
    @}
  */


/** @addtogroup auth_qmi_consts
    @{
  */
#define QMI_AUTH_EAP_REQ_PACKET_MAX_V01 256
#define QMI_AUTH_EAP_RESP_PACKET_MAX_V01 256
#define QMI_AUTH_SESSION_KEY_MAX_V01 256
#define QMI_AUTH_AKA_RAND_MAX_V01 255
#define QMI_AUTH_AKA_AUTN_MAX_V01 255
#define QMI_AUTH_AKA_DIGEST_MAX_V01 255
#define QMI_AUTH_AKA_DATA_MAX_V01 255
#define QMI_AUTH_EAP_IDENTITY_MAX_CHAR_V01 255
#define QMI_AUTH_EAP_REQ_PACKET_EXT_MAX_V01 4096
#define QMI_AUTH_EAP_RESP_PACKET_EXT_MAX_V01 4096
#define QMI_AUTH_EAP_INFO_MAX_V01 256
#define QMI_AUTH_MCC_MNC_SIZE_V01 3
#define QMI_AUTH_CA_CERT_MAX_V01 2048
#define QMI_AUTH_CERT_DN_MAX_V01 256
#define QMI_AUTH_IMSI_PUBLIC_KEY_MAX_V01 512
#define QMI_AUTH_IMSI_KEY_ID_AVP_MAX_V01 256
#define QMI_AUTH_IMSI_KEY_NUM_MAX_V01 2
/**
    @}
  */

typedef uint32_t auth_eap_cred_mask_v01;
#define QMI_AUTH_MASK_EAP_CRED_MSK_V01 ((auth_eap_cred_mask_v01)0x00000001)
#define QMI_AUTH_MASK_EAP_CRED_EXT_MSK_V01 ((auth_eap_cred_mask_v01)0x00000002)
#define QMI_AUTH_MASK_EAP_CRED_SESSION_ID_V01 ((auth_eap_cred_mask_v01)0x00000004)
/** @addtogroup auth_qmi_aggregates
    @{
  */
/**  EAP Master Session Key
 EAP Extended Master Session Key
 Session ID
 */
typedef struct {

  char mcc[QMI_AUTH_MCC_MNC_SIZE_V01 + 1];
  /**<   Mobile Country Code (MCC). Range: 0 to 999. */

  char mnc[QMI_AUTH_MCC_MNC_SIZE_V01 + 1];
  /**<   Mobile Network Code (MNC). Range: 0 to 999. */
}auth_mcc_mnc_info_v01;  /* Type */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Request Message; Resets the client. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}auth_reset_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Response Message; Resets the client. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.       */
}auth_reset_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef uint32_t auth_eap_method_mask_v01;
#define QMI_AUTH_EAP_METHOD_MASK_SIM_V01 ((auth_eap_method_mask_v01)0x01)
#define QMI_AUTH_EAP_METHOD_MASK_AKA_V01 ((auth_eap_method_mask_v01)0x02)
#define QMI_AUTH_EAP_METHOD_MASK_AKA_PRIME_V01 ((auth_eap_method_mask_v01)0x04)
/** @addtogroup auth_qmi_enums
    @{
  */
typedef enum {
  AUTH_EAP_SIM_AKA_ALGO_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  AUTH_EAP_AKA_ALGO_NONE_V01 = 0x0000,
  AUTH_EAP_AKA_ALGO_SHA1_V01 = 0x0001,
  AUTH_EAP_AKA_ALGO_MILENAGE_V01 = 0x0002,
  AUTH_EAP_AKA_ALGO_CAVE_V01 = 0x0003,
  AUTH_EAP_AKA_ALGO_GSM_V01 = 0x0004,
  AUTH_EAP_AKA_ALGO_USIM_GSM_V01 = 0x0005,
  AUTH_EAP_SIM_AKA_ALGO_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}auth_eap_sim_aka_algo_enum_v01;
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Request Message; Starts the EAP session. */
typedef struct {

  /* Optional */
  /*  EAP Method Mask */
  uint8_t eap_method_mask_valid;  /**< Must be set to true if eap_method_mask is being passed */
  auth_eap_method_mask_v01 eap_method_mask;
  /**<   Bitmask. The bits corresponding to the methods to be supported
       must be set to 1. Bit values:\n
       - 0 -- EAP-SIM\n
       - 1 -- EAP-AKA\n
       - 2 -- EAP-AKA'
  */

  /* Optional */
  /*  User ID */
  uint8_t user_id_valid;  /**< Must be set to true if user_id is being passed */
  uint32_t user_id_len;  /**< Must be set to # of elements in user_id */
  uint8_t user_id[QMI_AUTH_EAP_IDENTITY_MAX_CHAR_V01];
  /**<   Buffer containing the EAP identity.*/

  /* Optional */
  /*  EAP Meta Identity */
  uint8_t eap_meta_identity_valid;  /**< Must be set to true if eap_meta_identity is being passed */
  uint32_t eap_meta_identity_len;  /**< Must be set to # of elements in eap_meta_identity */
  uint8_t eap_meta_identity[QMI_AUTH_EAP_IDENTITY_MAX_CHAR_V01];
  /**<   Buffer containing the EAP meta identity.*/

  /* Optional */
  /*  EAP SIM AKA Algorithm */
  uint8_t eap_sim_aka_algo_valid;  /**< Must be set to true if eap_sim_aka_algo is being passed */
  auth_eap_sim_aka_algo_enum_v01 eap_sim_aka_algo;
  /**<   EAP AKA algorithm. Values: \n
       - 0x0000 -- EAP AKA none \n
       - 0x0001 -- EAP AKA SHA-1 \n
       - 0x0002 -- EAP AKA MILENAGE \n
       - 0x0003 -- EAP AKA CAVE \n
       - 0x0004 -- EAP SIM GSM \n
       - 0x0005 -- EAP SIM USIM GSM \n

       If only the eap_sim_aka_algo parameter is specified, the EAP method type
       associated with the current EAP session is set as
       follows:\n
       - 0x0001, 0x0003 -- EAP method is QMI_AUTH_EAP_METHOD_ MASK_AKA \n
       - 0x0004, 0x0005 -- EAP method is QMI_AUTH_EAP_METHOD_ MASK_SIM \n
       - 0x0002 -- EAP method is set to AKA|AKA'. Depending on which
                       packets are sent, the EAP module uses either AKA or AKA'
                       at a later time \n
       - 0x0000 -- All EAP methods are supported
  */

  /* Optional */
  /*  Use Encrypted IMSI Keys */
  uint8_t use_encrypted_imsi_valid;  /**< Must be set to true if use_encrypted_imsi is being passed */
  uint8_t use_encrypted_imsi;
}auth_start_eap_session_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Response Message; Starts the EAP session. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.     */
}auth_start_eap_session_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Request Message; Sends and receives EAP packets. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  EAP Request Packet */
  uint32_t eap_request_pkt_len;  /**< Must be set to # of elements in eap_request_pkt */
  uint8_t eap_request_pkt[QMI_AUTH_EAP_REQ_PACKET_MAX_V01];
  /**<   Buffer containing the EAP request packet.
  */
}auth_send_eap_packet_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Response Message; Sends and receives EAP packets. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.       */

  /* Mandatory */
  /*  EAP Response Packet */
  uint32_t eap_response_pkt_len;  /**< Must be set to # of elements in eap_response_pkt */
  uint8_t eap_response_pkt[QMI_AUTH_EAP_RESP_PACKET_MAX_V01];
  /**<   Buffer containing the EAP response packet.
  */
}auth_send_eap_packet_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Indication Message; Communicates the result of the EAP session. */
typedef struct {

  /* Mandatory */
  /*  EAP Result */
  uint8_t eap_result;
  /**<   Values:\n
       - 0 -- SUCCESS\n
       - 1 -- FAILURE
  */
}auth_eap_session_result_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Request Message; Queries the EAP session keys. (Deprecated) */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}auth_get_eap_session_keys_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Response Message; Queries the EAP session keys. (Deprecated) */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.      */

  /* Mandatory */
  /*  Key */
  uint32_t session_key_len;  /**< Must be set to # of elements in session_key */
  uint8_t session_key[QMI_AUTH_SESSION_KEY_MAX_V01];
  /**<   Session key.
  */
}auth_get_eap_session_keys_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Request Message; Ends the EAP session. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}auth_end_eap_session_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Response Message; Ends the EAP session. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.     */
}auth_end_eap_session_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_enums
    @{
  */
typedef enum {
  AUTH_AKA_VER_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  AUTH_AKA_VER_V1_V01 = 0,
  AUTH_AKA_VER_V2_V01 = 1,
  AUTH_AKA_VER_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}auth_aka_ver_enum_v01;
/**
    @}
  */

/** @addtogroup auth_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t rand_len;  /**< Must be set to # of elements in rand */
  uint8_t rand[QMI_AUTH_AKA_RAND_MAX_V01];
  /**<   Buffer containing the random challenge value.*/

  uint32_t autn_len;  /**< Must be set to # of elements in autn */
  uint8_t autn[QMI_AUTH_AKA_AUTN_MAX_V01];
  /**<   Buffer containing the authentication token.*/
}auth_aka_params_v01;  /* Type */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Request Message; Runs the AKA algorithm. */
typedef struct {

  /* Mandatory */
  /*  AKA Version */
  auth_aka_ver_enum_v01 aka_ver;
  /**<   AKA version the algorithm must use:\n
       - 0 -- AKA_V1\n
       - 1 -- AKA_V2\n
       All other values are reserved for future use.
  */

  /* Optional */
  /*  AKA_V1/V2 Authentication Parameters */
  uint8_t aka_params_valid;  /**< Must be set to true if aka_params is being passed */
  auth_aka_params_v01 aka_params;
}auth_run_aka_algo_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Response Message; Runs the AKA algorithm. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.       */

  /* Mandatory */
  /*  AKA Handle */
  uint32_t aka_handle;
  /**<   AKA handle to identify the AKA request.
  */
}auth_run_aka_algo_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_enums
    @{
  */
typedef enum {
  AUTH_AKA_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  AUTH_AKA_STATUS_SUCCESS_V01 = 0,
  AUTH_AKA_STATUS_SYNC_FAILURE_V01 = 1,
  AUTH_AKA_STATUS_FAILURE_V01 = 2,
  AUTH_AKA_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}auth_aka_status_enum_v01;
/**
    @}
  */

/** @addtogroup auth_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t aka_handle;
  /**<   AKA handle to identify the AKA request.*/

  auth_aka_status_enum_v01 aka_status;
  /**<   Result of the AKA Request algorithm:\n
       - 0 -- AKA_SUCCESS\n
       - 1 -- AKA_SYNC_FAILURE\n
       - 2 -- AKA_FAILURE\n
       All other values are reserved for future use.
  */
}auth_aka_algo_result_v01;  /* Type */
/**
    @}
  */

/** @addtogroup auth_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t digest_len;  /**< Must be set to # of elements in digest */
  uint8_t digest[QMI_AUTH_AKA_DIGEST_MAX_V01];
  /**<   Buffer containing the digest response.*/

  uint32_t aka_data_len;  /**< Must be set to # of elements in aka_data */
  uint8_t aka_data[QMI_AUTH_AKA_DATA_MAX_V01];
  /**<   Buffer containing the AKA response data.*/
}auth_aka_response_data_v01;  /* Type */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Indication Message; Communicates the result of the AKA algorithm. */
typedef struct {

  /* Mandatory */
  /*  AKA Algorithm Result Indication */
  auth_aka_algo_result_v01 aka_algo_result;

  /* Optional */
  /*  AKA_V1/V2 Response Data */
  uint8_t aka_response_data_valid;  /**< Must be set to true if aka_response_data is being passed */
  auth_aka_response_data_v01 aka_response_data;
}auth_aka_algo_result_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_enums
    @{
  */
typedef enum {
  AUTH_BIND_SUBSCRIPTION_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  AUTH_PRIMARY_SUBS_V01 = 0x0001, /**<  Primary subscription \n  */
  AUTH_SECONDARY_SUBS_V01 = 0x0002, /**<  Secondary subscription  \n */
  AUTH_TERTIARY_SUBS_V01 = 0x0003, /**<  Tertiary subscription   */
  AUTH_BIND_SUBSCRIPTION_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}auth_bind_subscription_enum_v01;
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Request Message; Associates the requesting control point with the requested subscription. */
typedef struct {

  /* Mandatory */
  /*  Bind Subscription */
  auth_bind_subscription_enum_v01 bind_subs;
  /**<   Subscription to which to bind. Values: \n
      - AUTH_PRIMARY_SUBS (0x0001) --  Primary subscription \n
      - AUTH_SECONDARY_SUBS (0x0002) --  Secondary subscription  \n
      - AUTH_TERTIARY_SUBS (0x0003) --  Tertiary subscription
 */
}auth_set_subscription_binding_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Response Message; Associates the requesting control point with the requested subscription. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}auth_set_subscription_binding_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Request Message; Queries the subscription associated with the control point. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}auth_get_bind_subscription_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Response Message; Queries the subscription associated with the control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Bound Subscription */
  uint8_t bind_subscription_valid;  /**< Must be set to true if bind_subscription is being passed */
  auth_bind_subscription_enum_v01 bind_subscription;
  /**<   Values: \n
      - AUTH_PRIMARY_SUBS (0x0001) --  Primary subscription \n
      - AUTH_SECONDARY_SUBS (0x0002) --  Secondary subscription  \n
      - AUTH_TERTIARY_SUBS (0x0003) --  Tertiary subscription
 */
}auth_get_bind_subscription_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Request Message; Sets the registration state of the QMI_AUTH indication
            for the requesting control point. */
typedef struct {

  /* Optional */
  /*  EAP Session Error Notification */
  uint8_t report_eap_notification_code_valid;  /**< Must be set to true if report_eap_notification_code is being passed */
  uint8_t report_eap_notification_code;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report the error
  */

  /* Optional */
  /*  EAP Session Error code */
  uint8_t report_eap_error_code_valid;  /**< Must be set to true if report_eap_error_code is being passed */
  uint8_t report_eap_error_code;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report the error
  */

  /* Optional */
  /*  EAP auth reject notification */
  uint8_t report_eap_auth_reject_valid;  /**< Must be set to true if report_eap_auth_reject is being passed */
  uint8_t report_eap_auth_reject;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report the error
  */

  /* Optional */
  /*  IMSI Public Key Retrieval Notification */
  uint8_t report_imsi_public_key_request_valid;  /**< Must be set to true if report_imsi_public_key_request is being passed */
  uint8_t report_imsi_public_key_request;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report the request for IMSI public key information
  */

  /* Optional */
  /*  CA Certificate Retrival Request */
  uint8_t report_ca_cert_retr_request_valid;  /**< Must be set to true if report_ca_cert_retr_request is being passed */
  uint8_t report_ca_cert_retr_request;
  /**<   Values: \n
       - 0 -- Do not report \n
       - 1 -- Report the request to retrieve ca certificate
  */
}auth_indication_register_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Response Message; Sets the registration state of the QMI_AUTH indication
            for the requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}auth_indication_register_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Indication Message; Provides a notification code from the EAP
             server to the requested control point. */
typedef struct {

  /* Mandatory */
  /*  Notification Code */
  uint16_t eap_notification_code;
  /**<   Notification code. */
}auth_eap_notification_code_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Indication Message; EAP error code indication */
typedef struct {

  /* Mandatory */
  /*  EAP ERROR Code */
  uint16_t eap_error_code;
  /**<   Error code. */
}auth_eap_error_code_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Indication Message; EAP Authentication rejected indication */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}auth_eap_auth_rej_ind_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Request Message; Sends and receives EAP packets. */
typedef struct {

  /* Mandatory */
  /*  EAP Request Packet Ext */
  uint32_t eap_request_ext_pkt_len;  /**< Must be set to # of elements in eap_request_ext_pkt */
  uint8_t eap_request_ext_pkt[QMI_AUTH_EAP_REQ_PACKET_EXT_MAX_V01];
  /**<   Buffer containing the EAP request packet.
  */
}auth_send_eap_packet_ext_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Response Message; Sends and receives EAP packets. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.       */

  /* Mandatory */
  /*  EAP Response Packet Ext */
  uint32_t eap_response_ext_pkt_len;  /**< Must be set to # of elements in eap_response_ext_pkt */
  uint8_t eap_response_ext_pkt[QMI_AUTH_EAP_RESP_PACKET_EXT_MAX_V01];
  /**<   Buffer containing the EAP response packet.
  */
}auth_send_eap_packet_ext_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Request Message; Queries the EAP authentication credentials. */
typedef struct {

  /* Mandatory */
  /*  Credential Request Mask */
  auth_eap_cred_mask_v01 cred_request_mask;
  /**<   Values: \n
       @Mask
    */
}auth_get_eap_auth_credentials_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Response Message; Queries the EAP authentication credentials. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  Master Session Key */
  uint8_t msk_valid;  /**< Must be set to true if msk is being passed */
  uint32_t msk_len;  /**< Must be set to # of elements in msk */
  uint8_t msk[QMI_AUTH_EAP_INFO_MAX_V01];
  /**<   Master Session key.
  */

  /* Optional */
  /*  Extended Master Session Key */
  uint8_t ext_msk_valid;  /**< Must be set to true if ext_msk is being passed */
  uint32_t ext_msk_len;  /**< Must be set to # of elements in ext_msk */
  uint8_t ext_msk[QMI_AUTH_EAP_INFO_MAX_V01];
  /**<   Extended Master Session key.
  */

  /* Optional */
  /*  Session ID */
  uint8_t session_id_valid;  /**< Must be set to true if session_id is being passed */
  uint32_t session_id_len;  /**< Must be set to # of elements in session_id */
  uint8_t session_id[QMI_AUTH_EAP_INFO_MAX_V01];
  /**<   Session ID. Values defined as per IETF RFC 5247.
  */
}auth_get_eap_auth_credentials_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Request Message; Update the IMSI public key (deprecated) */
typedef struct {

  /* Mandatory */
  /*  IMSI Public Key */
  uint32_t public_key_len;  /**< Must be set to # of elements in public_key */
  uint8_t public_key[QMI_AUTH_IMSI_PUBLIC_KEY_MAX_V01];

  /* Optional */
  /*  Key Identifier AVP */
  uint8_t key_identifier_valid;  /**< Must be set to true if key_identifier is being passed */
  uint32_t key_identifier_len;  /**< Must be set to # of elements in key_identifier */
  char key_identifier[QMI_AUTH_IMSI_KEY_ID_AVP_MAX_V01];
  /**<   Key identifier attribute value pair. An optional null terminated string
       that helps server locate the private key. */

  /* Optional */
  /*   MCC_MNC Information */
  uint8_t mcc_mnc_info_valid;  /**< Must be set to true if mcc_mnc_info is being passed */
  auth_mcc_mnc_info_v01 mcc_mnc_info;

  /* Optional */
  /*  Expiry Time */
  uint8_t expiry_time_valid;  /**< Must be set to true if expiry_time is being passed */
  uint64_t expiry_time;
  /**<   Format in UTC. Time value in milliseconds with an offset of
       "Jan 1, 1970 00:00:00". */
}auth_send_imsi_public_key_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Response Message; Update the IMSI public key (deprecated) */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.       */
}auth_send_imsi_public_key_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_enums
    @{
  */
typedef enum {
  AUTH_IMSI_PUBLIC_KEY_ACTION_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  AUTH_IMSI_PUBLIC_KEY_RETRIEVE_V01 = 1,
  AUTH_IMSI_PUBLIC_KEY_REFRESH_V01 = 2,
  AUTH_IMSI_PUBLIC_KEY_ACTION_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}auth_imsi_public_key_action_enum_v01;
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Indication Message; Indication to TE to send the IMSI Public Key to Modem */
typedef struct {

  /* Mandatory */
  /*  Action */
  auth_imsi_public_key_action_enum_v01 action;
  /**<   Values:\n
       @ENUM
  */
}auth_retrieve_imsi_public_key_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Indication Message; Indication to TE to send the Certificate Authority (CA) certificate to Modem */
typedef struct {

  /* Mandatory */
  /*  Transaction ID */
  uint32_t tx_id;
  /**<   Transaction ID
  */

  /* Mandatory */
  /*  Certificate Distinguished Name (Cert DN) */
  uint32_t cert_dn_len;  /**< Must be set to # of elements in cert_dn */
  uint8_t cert_dn[QMI_AUTH_CERT_DN_MAX_V01];
  /**<   Byte array for Certificate distinguished name.
  */
}auth_retrieve_ca_certificate_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_enums
    @{
  */
typedef enum {
  AUTH_CA_CERT_SEND_FAILURE_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  AUTH_CA_CERT_FAILURE_GENERAL_V01 = 1,
  AUTH_CA_CERT_FAILURE_NOT_FOUND_V01 = 2,
  AUTH_CA_CERT_SEND_FAILURE_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}auth_ca_cert_send_failure_enum_type_v01;
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Request Message; Sends the Certificate Authority (CA) certificate to Modem */
typedef struct {

  /* Mandatory */
  /*  Transaction ID */
  uint32_t tx_id;
  /**<   Transaction ID
  */

  /* Optional */
  /*  Total Number of Certificates */
  uint8_t num_certificates_valid;  /**< Must be set to true if num_certificates is being passed */
  uint16_t num_certificates;
  /**<   Total number of certificates.
       Must be present if CA Certificate is present.
  */

  /* Optional */
  /*  Certificate Index */
  uint8_t cert_index_valid;  /**< Must be set to true if cert_index is being passed */
  uint16_t cert_index;
  /**<   Certificate index. Starts from 1, increment by 1 only.
       Must be less than or equal to Total Number of Certificates.
       Must be present if CA Certificate is present.
  */

  /* Optional */
  /*  CA Certificate */
  uint8_t ca_cert_valid;  /**< Must be set to true if ca_cert is being passed */
  uint32_t ca_cert_len;  /**< Must be set to # of elements in ca_cert */
  uint8_t ca_cert[QMI_AUTH_CA_CERT_MAX_V01];
  /**<   Byte array for CA certificate. */

  /* Optional */
  /*  Total Number of Segments per CA certificate */
  uint8_t num_segments_valid;  /**< Must be set to true if num_segments is being passed */
  uint8_t num_segments;
  /**<   If a CA certificate is split into segments across multiple
       messages, this TLV identifies the total number of segments
       that the certificate is split into.
  */

  /* Optional */
  /*  CA Certificate Sequence Number */
  uint8_t sequence_number_valid;  /**< Must be set to true if sequence_number is being passed */
  uint8_t sequence_number;
  /**<   Sequence number identifying the segment number of
       the certificate being sent.
       Starts from 1, increment by 1 only.
       Must be less than or equal to Total Number of Segments.
       Must be present if CA Certificate is present.
  */

  /* Optional */
  /*  Retrieval Failure Reason */
  uint8_t failure_reason_valid;  /**< Must be set to true if failure_reason is being passed */
  auth_ca_cert_send_failure_enum_type_v01 failure_reason;
  /**<   Retrieval Failure reason. Values: \n
       @Enum */
}auth_send_ca_certificate_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Response Message; Sends the Certificate Authority (CA) certificate to Modem */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.       */
}auth_send_ca_certificate_resp_msg_v01;  /* Message */
/**
    @}
  */

typedef uint64_t auth_imsi_public_key_type_mask_v01;
#define AUTH_IMSI_PUBLIC_KEY_EPDG_V01 ((auth_imsi_public_key_type_mask_v01)0x01ull)
#define AUTH_IMSI_PUBLIC_KEY_WLAN_V01 ((auth_imsi_public_key_type_mask_v01)0x02ull)
/** @addtogroup auth_qmi_aggregates
    @{
  */
typedef struct {

  /*  IMSI Public Key */
  uint32_t public_key_len;  /**< Must be set to # of elements in public_key */
  uint8_t public_key[QMI_AUTH_IMSI_PUBLIC_KEY_MAX_V01];

  /*  Key Identifier AVP Valid Flag */
  uint8_t key_identifier_valid;

  /*  Key Identifier AVP */
  uint32_t key_identifier_len;  /**< Must be set to # of elements in key_identifier */
  char key_identifier[QMI_AUTH_IMSI_KEY_ID_AVP_MAX_V01];
  /**<   Key identifier attribute value pair. An optional null terminated string
       that helps server locate the private key. */

  /*   IMSI Public Key Type */
  auth_imsi_public_key_type_mask_v01 key_type;

  /*   MCC_MNC Information Valid Flag */
  uint8_t mcc_mnc_info_valid;

  /*   MCC_MNC Information */
  auth_mcc_mnc_info_v01 mcc_mnc_info;

  /*  Expiry Time Valid Flag */
  uint8_t expiry_time_valid;

  /*  Expiry Time */
  uint64_t expiry_time;
  /**<   Format in UTC. Time value in milliseconds with an offset of
       "Jan 1, 1970 00:00:00". */
}auth_imsi_public_key_info_v01;  /* Type */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Request Message; Update the IMSI public keys */
typedef struct {

  /* Optional */
  /*  IMSI Public Keys */
  uint8_t public_keys_valid;  /**< Must be set to true if public_keys is being passed */
  uint32_t public_keys_len;  /**< Must be set to # of elements in public_keys */
  auth_imsi_public_key_info_v01 public_keys[QMI_AUTH_IMSI_KEY_NUM_MAX_V01];
}auth_send_imsi_public_key_ex_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup auth_qmi_messages
    @{
  */
/** Response Message; Update the IMSI public keys */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.       */
}auth_send_imsi_public_key_ex_resp_msg_v01;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */
//#define REMOVE_QMI_AUTH_AKA_ALGO_RESULT_IND_V01
//#define REMOVE_QMI_AUTH_EAP_AUTH_REJ_IND_V01
//#define REMOVE_QMI_AUTH_EAP_ERROR_CODE_IND_V01
//#define REMOVE_QMI_AUTH_EAP_NOTIFICATION_CODE_IND_V01
//#define REMOVE_QMI_AUTH_EAP_SESSION_RESULT_IND_V01
//#define REMOVE_QMI_AUTH_END_EAP_SESSION_V01
//#define REMOVE_QMI_AUTH_GET_BIND_SUBSCRIPTION_V01
//#define REMOVE_QMI_AUTH_GET_EAP_AUTH_CREDENTIALS_V01
//#define REMOVE_QMI_AUTH_GET_EAP_SESSION_KEYS_V01
//#define REMOVE_QMI_AUTH_GET_SUPPORTED_FIELDS_V01
//#define REMOVE_QMI_AUTH_GET_SUPPORTED_MSGS_V01
//#define REMOVE_QMI_AUTH_INDICATION_REGISTER_V01
//#define REMOVE_QMI_AUTH_RESET_V01
//#define REMOVE_QMI_AUTH_RETRIEVE_CA_CERTIFICATE_IND_V01
//#define REMOVE_QMI_AUTH_RETRIEVE_IMSI_PUBLIC_KEY_IND_V01
//#define REMOVE_QMI_AUTH_RUN_AKA_ALGO_V01
//#define REMOVE_QMI_AUTH_SEND_CA_CERTIFICATE_V01
//#define REMOVE_QMI_AUTH_SEND_EAP_PACKET_V01
//#define REMOVE_QMI_AUTH_SEND_EAP_PACKET_EXT_V01
//#define REMOVE_QMI_AUTH_SEND_IMSI_PUBLIC_KEY_V01
//#define REMOVE_QMI_AUTH_SEND_IMSI_PUBLIC_KEY_EX_V01
//#define REMOVE_QMI_AUTH_SET_SUBSCRIPTION_BINDING_V01
//#define REMOVE_QMI_AUTH_START_EAP_SESSION_V01

/*Service Message Definition*/
/** @addtogroup auth_qmi_msg_ids
    @{
  */
#define QMI_AUTH_RESET_REQ_V01 0x0000
#define QMI_AUTH_RESET_RESP_V01 0x0000
#define QMI_AUTH_INDICATION_REGISTER_REQ_V01 0x0003
#define QMI_AUTH_INDICATION_REGISTER_RESP_V01 0x0003
#define QMI_AUTH_GET_SUPPORTED_MSGS_REQ_V01 0x001E
#define QMI_AUTH_GET_SUPPORTED_MSGS_RESP_V01 0x001E
#define QMI_AUTH_GET_SUPPORTED_FIELDS_REQ_V01 0x001F
#define QMI_AUTH_GET_SUPPORTED_FIELDS_RESP_V01 0x001F
#define QMI_AUTH_START_EAP_SESSION_REQ_V01 0x0020
#define QMI_AUTH_START_EAP_SESSION_RESP_V01 0x0020
#define QMI_AUTH_SEND_EAP_PACKET_REQ_V01 0x0021
#define QMI_AUTH_SEND_EAP_PACKET_RESP_V01 0x0021
#define QMI_AUTH_EAP_SESSION_RESULT_IND_V01 0x0022
#define QMI_AUTH_GET_EAP_SESSION_KEYS_REQ_V01 0x0023
#define QMI_AUTH_GET_EAP_SESSION_KEYS_RESP_V01 0x0023
#define QMI_AUTH_END_EAP_SESSION_REQ_V01 0x0024
#define QMI_AUTH_END_EAP_SESSION_RESP_V01 0x0024
#define QMI_AUTH_RUN_AKA_ALGO_REQ_V01 0x0025
#define QMI_AUTH_RUN_AKA_ALGO_RESP_V01 0x0025
#define QMI_AUTH_AKA_ALGO_RESULT_IND_V01 0x0026
#define QMI_AUTH_SET_SUBSCRIPTION_BINDING_REQ_V01 0x0027
#define QMI_AUTH_SET_SUBSCRIPTION_BINDING_RESP_V01 0x0027
#define QMI_AUTH_GET_BIND_SUBSCRIPTION_REQ_V01 0x0028
#define QMI_AUTH_GET_BIND_SUBSCRIPTION_RESP_V01 0x0028
#define QMI_AUTH_EAP_NOTIFICATION_CODE_IND_V01 0x0029
#define QMI_AUTH_EAP_ERROR_CODE_IND_V01 0x002A
#define QMI_AUTH_EAP_AUTH_REJ_IND_V01 0x002B
#define QMI_AUTH_SEND_EAP_PACKET_EXT_REQ_V01 0x002C
#define QMI_AUTH_SEND_EAP_PACKET_EXT_RESP_V01 0x002C
#define QMI_AUTH_GET_EAP_AUTH_CREDENTIALS_REQ_V01 0x002D
#define QMI_AUTH_GET_EAP_AUTH_CREDENTIALS_RESP_V01 0x002D
#define QMI_AUTH_SEND_IMSI_PUBLIC_KEY_REQ_V01 0x002E
#define QMI_AUTH_SEND_IMSI_PUBLIC_KEY_RESP_V01 0x002E
#define QMI_AUTH_RETRIEVE_IMSI_PUBLIC_KEY_IND_V01 0x002F
#define QMI_AUTH_RETRIEVE_CA_CERTIFICATE_IND_V01 0x0030
#define QMI_AUTH_SEND_CA_CERTIFICATE_REQ_V01 0x0031
#define QMI_AUTH_SEND_CA_CERTIFICATE_RESP_V01 0x0031
#define QMI_AUTH_SEND_IMSI_PUBLIC_KEY_EX_REQ_V01 0x0032
#define QMI_AUTH_SEND_IMSI_PUBLIC_KEY_EX_RESP_V01 0x0032
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro auth_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type auth_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );

/** This macro should be used to get the service object */
#define auth_get_service_object_v01( ) \
          auth_get_service_object_internal_v01( \
            AUTH_V01_IDL_MAJOR_VERS, AUTH_V01_IDL_MINOR_VERS, \
            AUTH_V01_IDL_TOOL_VERS )
/**
    @}
  */


#ifdef __cplusplus
}
#endif
#endif
