#ifndef IMSA_SERVICE_01_H
#define IMSA_SERVICE_01_H
/**
  @file ip_multimedia_subsystem_application_v01.h

  @brief This is the public header file which defines the imsa service Data structures.

  This header file defines the types and structures that were defined in
  imsa. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2012-2018, 2020 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.14.7 
   It was generated on: Fri Jan 31 2020 (Spin 0)
   From IDL File: ip_multimedia_subsystem_application_v01.idl */

/** @defgroup imsa_qmi_consts Constant values defined in the IDL */
/** @defgroup imsa_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup imsa_qmi_enums Enumerated types used in QMI messages */
/** @defgroup imsa_qmi_messages Structures sent as QMI messages */
/** @defgroup imsa_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup imsa_qmi_accessor Accessor for QMI service object */
/** @defgroup imsa_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup imsa_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define IMSA_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define IMSA_V01_IDL_MINOR_VERS 0x27
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define IMSA_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define IMSA_V01_MAX_MESSAGE_ID 0x003F
/**
    @}
  */


/** @addtogroup imsa_qmi_consts
    @{
  */
#define IMSA_HO_FAILURE_CAUSE_CODE_STR_LEN_V01 125
#define IMSA_SUBSCRIPTION_FAILURE_ERROR_STR_LEN_V01 125
#define IMSA_REGISTRATION_FAILURE_ERROR_STR_LEN_V01 255
#define IMSA_P_ASSOCIATED_URI_MAX_STR_LEN_V01 255
#define IMSA_MAX_NUM_URIS_V01 30
#define IMSA_GEO_LOC_CITY_STR_LEN_V01 75
#define IMSA_GEO_LOC_COUNTRY_CODE_STR_LEN_V01 10
#define IMSA_GEO_LOC_COUNTRY_NAME_STR_LEN_V01 75
#define IMSA_GEO_LOC_STREET_NAME_STR_LEN_V01 127
#define IMSA_GEO_LOC_HOUSE_NUMBER_STR_LEN_V01 127
#define IMSA_ZIP_CODE_STR_LEN_V01 10
#define IMSA_STATE_STR_LEN_V01 75
#define IMSA_MSISDN_LEN_V01 20
#define IMSA_DIGITS_TOKEN_LEN_V01 300
#define IMSA_DIGITS_MAX_URIS_V01 8
#define IMSA_MAX_NO_VIRTUAL_LINES_V01 10
/**
    @}
  */

/** @addtogroup imsa_qmi_enums
    @{
  */
typedef enum {
  IMSA_SERVICE_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSA_NO_SERVICE_V01 = 0, 
  IMSA_LIMITED_SERVICE_V01 = 1, /**<  IMS is in limited service  */
  IMSA_FULL_SERVICE_V01 = 2, /**<  IMS is in full service  */
  IMSA_SERVICE_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsa_service_status_enum_v01;
/**
    @}
  */

/** @addtogroup imsa_qmi_enums
    @{
  */
typedef enum {
  IMSA_SERVICE_RAT_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSA_WLAN_V01 = 0, /**<  IMS service is registered on WLAN \n  */
  IMSA_WWAN_V01 = 1, /**<  IMS service is registered on WWAN \n  */
  IMSA_IWLAN_V01 = 2, /**<  IMS service is registered on interworking WLAN  */
  IMSA_SERVICE_RAT_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsa_service_rat_enum_v01;
/**
    @}
  */

/** @addtogroup imsa_qmi_enums
    @{
  */
typedef enum {
  IMSA_IMS_REGISTRATION_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSA_STATUS_NOT_REGISTERED_V01 = 0, /**<  Not registered for IMS \n  */
  IMSA_STATUS_REGISTERING_V01 = 1, /**<  Registering for IMS \n  */
  IMSA_STATUS_REGISTERED_V01 = 2, /**<  Registered for IMS  */
  IMSA_STATUS_LIMITED_REGISTERED_V01 = 3, /**<  Limited registration for IMS  */
  IMSA_IMS_REGISTRATION_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsa_ims_registration_status_enum_v01;
/**
    @}
  */

/** @addtogroup imsa_qmi_enums
    @{
  */
typedef enum {
  IMSA_RAT_HANDOVER_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSA_STATUS_RAT_HO_SUCCESS_V01 = 0, /**<  RAT handover was successful \n  */
  IMSA_STATUS_RAT_HO_FAILURE_V01 = 1, /**<  RAT handover failed  */
  IMSA_STATUS_RAT_HO_NOT_TRIGGERED_V01 = 2, /**<  RAT handover could not be triggered  */
  IMSA_STATUS_RAT_HO_NOT_TRIGGERED_MOBILE_DATA_OFF_V01 = 3, /**<  RAT handover could not be triggered because mobile data is off  */
  IMSA_RAT_HANDOVER_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsa_rat_handover_status_enum_v01;
/**
    @}
  */

/** @addtogroup imsa_qmi_enums
    @{
  */
typedef enum {
  IMSA_PDP_FAILURE_ERROR_CODE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSA_PDP_STATUS_OTHER_FAILURE_V01 = 0, /**<  Generic failure reason for other than specified \n  */
  IMSA_PDP_STATUS_OPTION_UNSUBSCRIBED_V01 = 1, /**<  Option is unsubscribed \n  */
  IMSA_PDP_STATUS_UNKNOWN_PDP_V01 = 2, /**<  PDP was unknown  */
  IMSA_PDP_STATUS_REASON_NOT_SPECIFIED_V01 = 3, /**<  Reason not specified \n */
  IMSA_PDP_STATUS_CONNECTION_BRINGUP_FAILURE_V01 = 4, /**<  Connection bring-up failure  */
  IMSA_PDP_STATUS_CONNECTION_IKE_AUTH_FAILURE_V01 = 5, /**<  IKE authentication failure  */
  IMSA_PDP_STATUS_USER_AUTH_FAILED_V01 = 6, /**<  User authentication failed    */
  IMSA_PDP_FAILURE_ERROR_CODE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsa_pdp_failure_error_code_enum_v01;
/**
    @}
  */

/** @addtogroup imsa_qmi_enums
    @{
  */
typedef enum {
  IMSA_ACS_FAILURE_STATUS_CODE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSA_ACS_FAILURE_NO_EXISTING_ACS_DATA_V01 = 0, /**<  ACS failure; there is no earlier APCS success data, and
       the ISIM parameters are not available \n  */
  IMSA_ACS_FAILURE_ERROR_CODE_RECEIVED_V01 = 1, /**<  Error code received from the network for an
       ACS configuration retrieval request  */
  IMSA_ACS_FAILURE_STATUS_CODE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsa_acs_failure_status_code_enum_v01;
/**
    @}
  */

/** @addtogroup imsa_qmi_enums
    @{
  */
typedef enum {
  IMSA_SUBSCRIPTION_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSA_SUBSCRIPTION_TYPE_REG_V01 = 0, /**<  Register subscription\n  */
  IMSA_SUBSCRIPTION_TYPE_MWI_V01 = 1, /**<  MWI subscription  */
  IMSA_SUBSCRIPTION_TYPE_CONFERENCE_V01 = 2, /**<  Conference subscription  */
  IMSA_SUBSCRIPTION_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsa_subscription_type_enum_v01;
/**
    @}
  */

/** @addtogroup imsa_qmi_enums
    @{
  */
typedef enum {
  IMSA_NETWORK_PROVISIONING_STATUS_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSA_NETWORK_PROV_OUT_OF_SYNC_V01 = 0, /**<  Network is out of sync. This is an 
       indication sent when the IMS UE detects the network provisioning status through 
       SIP messages. When the network is out of sync with the UE, DM resync 
       may be triggered by the OEM based on this indication.
   */
  IMSA_NETWORK_PROVISIONING_STATUS_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsa_network_provisioning_status_type_enum_v01;
/**
    @}
  */

/** @addtogroup imsa_qmi_enums
    @{
  */
typedef enum {
  IMSA_VOWIFIPROFILE_STATUS_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSA_SERVICE_VOWIFI_QUALITIY_NO_ERR_V01 = 0, /**<   No error   */
  IMSA_SERVICE_VOWIFI_QUALITIY_EXCELLENT_V01 = 1, /**<   Excellent indication   */
  IMSA_SERVICE_VOWIFI_QUALITIY_FAIR_V01 = 2, /**<   Fair indication  */
  IMSA_SERVICE_VOWIFI_QUALITIY_BAD_V01 = 3, /**<   Bad indication  */
  IMSA_VOWIFIPROFILE_STATUS_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsa_vowifiprofile_status_enum_v01;
/**
    @}
  */

/** @addtogroup imsa_qmi_enums
    @{
  */
typedef enum {
  IMSA_BIND_SUBSCRIPTION_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSA_BIND_SUBSCRIPTION_TYPE_PRIMARY_V01 = 0, /**<  Primary subscription  */
  IMSA_BIND_SUBSCRIPTION_TYPE_SECONDARY_V01 = 1, /**<  Secondary subscription  */
  IMSA_BIND_SUBSCRIPTION_TYPE_TERTIARY_V01 = 2, /**<  Tertiary subscription  */
  IMSA_BIND_SUBSCRIPTION_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsa_bind_subscription_type_enum_v01;
/**
    @}
  */

/** @addtogroup imsa_qmi_enums
    @{
  */
typedef enum {
  IMSA_VERBOSE_CALL_END_REASON_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSA_VCER_TYPE_UNSPECIFIED_V01 = 0x00, /**<  Unspecified \n  */
  IMSA_VCER_TYPE_MOBILE_IP_V01 = 0x01, /**<  Mobile IP \n  */
  IMSA_VCER_TYPE_INTERNAL_V01 = 0x02, /**<  Internal \n  */
  IMSA_VCER_TYPE_CALL_MANAGER_DEFINED_V01 = 0x03, /**<  Call manager is defined \n  */
  IMSA_VCER_TYPE_3GPP_SPEC_DEFINED_V01 = 0x06, /**<  3GPP specification is defined \n  */
  IMSA_VCER_TYPE_PPP_V01 = 0x07, /**<  PPP \n  */
  IMSA_VCER_TYPE_EHRPD_V01 = 0x08, /**<  EHRPD \n  */
  IMSA_VCER_TYPE_IPV6_V01 = 0x09, /**<  IPv6 \n  */
  IMSA_VCER_TYPE_IWLAN_V01 = 0x0B, /**<  IWLAN \n  */
  IMSA_VCER_TYPE_HANDOFF_V01 = 0x0C, /**<  Handoff  */
  IMSA_VERBOSE_CALL_END_REASON_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsa_verbose_call_end_reason_type_enum_v01;
/**
    @}
  */

/** @addtogroup imsa_qmi_enums
    @{
  */
typedef enum {
  IMSA_URI_LINE_STATUS_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSA_ACTIVE_V01 = 1, /**<  Active  */
  IMSA_INACTIVE_V01 = 2, /**<  Inactive  */
  IMSA_PENDING_V01 = 3, /**<  Pending  */
  IMSA_URI_LINE_STATUS_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsa_uri_line_status_v01;
/**
    @}
  */

/** @addtogroup imsa_qmi_enums
    @{
  */
typedef enum {
  IMSA_VOICE_INFO_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  IMSA_VOICE_INFO_SILENT_V01 = 0, /**<  IMS voice status silent \n  */
  IMSA_VOICE_INFO_SPEECH_V01 = 1, /**<  IMS voice status speech  */
  IMSA_VOICE_INFO_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}imsa_voice_info_enum_v01;
/**
    @}
  */

/**  IMS service is not available 
Enumeration for a verbose call end reason (VCER) type. 
 Bitmask values for the IMS blocking reason.
  */
typedef uint64_t imsa_block_reason_type_mask_v01;
#define IMSA_BLOCK_REASON_PDP_FAILURE_BIT_MASK_V01 ((imsa_block_reason_type_mask_v01)0x1ull) /**<  PDP failure  */
#define IMSA_BLOCK_REASON_REGISTRATION_FAILURE_BIT_MASK_V01 ((imsa_block_reason_type_mask_v01)0x2ull) /**<  Registration failure  */
#define IMSA_BLOCK_REASON_HANDOVER_FAILURE_BIT_MASK_V01 ((imsa_block_reason_type_mask_v01)0x4ull) /**<  Handover failure  */
#define IMSA_BLOCK_REASON_OTHER_FAILURE_BIT_MASK_V01 ((imsa_block_reason_type_mask_v01)0x8ull) /**<  Other failure  */
/**   Bitmask values to indicate the feature flags enabled in modem.
  */
typedef uint64_t imsa_modem_feature_mask_v01;
#define IMSA_FEATURE_FLAG_TYPE_NONE_V01 ((imsa_modem_feature_mask_v01)0x0000000000000000ull) /**<  Bitmask to indicate the No Feature Flag.  */
#define IMSA_FEATURE_FLAG_TYPE_DISABLE_USER_PREF_RTT_V01 ((imsa_modem_feature_mask_v01)0x0000000000000001ull) /**<  Bitmask to explicitly disable the user pref RTT feature.  */
/** @addtogroup imsa_qmi_aggregates
    @{
  */
typedef struct {

  imsa_verbose_call_end_reason_type_enum_v01 call_end_reason_type;
  /**<   Values: \n
      - IMSA_VCER_TYPE_UNSPECIFIED (0x00) --  Unspecified \n 
      - IMSA_VCER_TYPE_MOBILE_IP (0x01) --  Mobile IP \n 
      - IMSA_VCER_TYPE_INTERNAL (0x02) --  Internal \n 
      - IMSA_VCER_TYPE_CALL_MANAGER_DEFINED (0x03) --  Call manager is defined \n 
      - IMSA_VCER_TYPE_3GPP_SPEC_DEFINED (0x06) --  3GPP specification is defined \n 
      - IMSA_VCER_TYPE_PPP (0x07) --  PPP \n 
      - IMSA_VCER_TYPE_EHRPD (0x08) --  EHRPD \n 
      - IMSA_VCER_TYPE_IPV6 (0x09) --  IPv6 \n 
      - IMSA_VCER_TYPE_IWLAN (0x0B) --  IWLAN \n 
      - IMSA_VCER_TYPE_HANDOFF (0x0C) --  Handoff 
 */

  uint16_t call_end_reason;
  /**<   Failure reason (verbose); see Spec 24.008/24.007/C.S0017
       for the definition of these values.
  */
}imsa_call_end_failure_reason_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsa_qmi_aggregates
    @{
  */
typedef struct {

  imsa_rat_handover_status_enum_v01 rat_ho_status;
  /**<   RAT handover status. */

  imsa_service_rat_enum_v01 source_rat;
  /**<   Source RAT information; IWLAN/WWAN. */

  imsa_service_rat_enum_v01 target_rat;
  /**<   Target RAT information; IWLAN/WWAN. */

  char cause_code[IMSA_HO_FAILURE_CAUSE_CODE_STR_LEN_V01 + 1];
  /**<   Handover failure cause code string
       when the status is IMSA_STATUS_RAT_HO_FAILURE. */
}imsa_rat_handover_status_info_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsa_qmi_aggregates
    @{
  */
typedef struct {

  char uri[IMSA_P_ASSOCIATED_URI_MAX_STR_LEN_V01 + 1];
  /**<   URI string. */
}p_associated_uri_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsa_qmi_aggregates
    @{
  */
typedef struct {

  imsa_block_reason_type_mask_v01 block_reason_mask;
  /**<   Service mask extended. */

  imsa_call_end_failure_reason_type_v01 pdp_failure_reason;
  /**<   Reason PDP failed.
  */

  uint16_t registration_failure_reason;
  /**<   Reason registration failed. This is the same as for SIP error codes.
       Values: \n
       - 3xx -- Redirection responses \n
       - 4xx -- Client failure responses \n
       - 5xx -- Server failure responses \n
       - 6xx -- Global failure responses
  */

  imsa_call_end_failure_reason_type_v01 handover_failure_reason;
  /**<   Reason handover failed.
  */
}imsa_ims_failure_reason_type_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsa_qmi_aggregates
    @{
  */
typedef struct {

  char msisdn[IMSA_MSISDN_LEN_V01 + 1];
  /**<   MSISDN of the line to include IMS registration after activating with TMO servers. */

  imsa_uri_line_status_v01 uri_line_status;
  /**<   IMS updates the line status after and before IMS registration with the required MSISDNs. */
}imsa_digits_line_info_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsa_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t uri_list_len;  /**< Must be set to # of elements in uri_list */
  p_associated_uri_v01 uri_list[IMSA_DIGITS_MAX_URIS_V01];
  /**<   Specifies the p_associated_uri. */
}imsa_virtual_line_info_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Request Message; Gets the registration status for various IMS services for the
             requesting control point. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}imsa_get_registration_status_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Response Message; Gets the registration status for various IMS services for the
             requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  IMS Registration Status (Deprecated) */
  uint8_t ims_registered_valid;  /**< Must be set to true if ims_registered is being passed */
  uint8_t ims_registered;
  /**<   Values: \n
       - TRUE  -- UE is registered on the IMS network \n
       - FALSE -- UE is not registered on the IMS network

       Note: This TLV is deprecated; use the New IMS Registration Status TLV 
       instead.
   */

  /* Optional */
  /*  IMS Registration Error Code */
  uint8_t ims_registration_failure_error_code_valid;  /**< Must be set to true if ims_registration_failure_error_code is being passed */
  uint16_t ims_registration_failure_error_code;
  /**<   An error code is returned when the IMS
       registration status is IMSA_STATUS_NOT_REGISTERED. \n
       Values: \n
       - 3xx -- Redirection responses \n
       - 4xx -- Client failure responses \n
       - 5xx -- Server failure responses \n
       - 6xx -- Global failure responses
  */

  /* Optional */
  /*  New IMS Registration Status */
  uint8_t ims_reg_status_valid;  /**< Must be set to true if ims_reg_status is being passed */
  imsa_ims_registration_status_enum_v01 ims_reg_status;
  /**<   Values: \n
      - IMSA_STATUS_NOT_REGISTERED (0) --  Not registered for IMS \n 
      - IMSA_STATUS_REGISTERING (1) --  Registering for IMS \n 
      - IMSA_STATUS_REGISTERED (2) --  Registered for IMS 
      - IMSA_STATUS_LIMITED_REGISTERED (3) --  Limited registration for IMS 
 */

  /* Optional */
  /*  Registration Error String */
  uint8_t registration_error_string_valid;  /**< Must be set to true if registration_error_string is being passed */
  char registration_error_string[IMSA_REGISTRATION_FAILURE_ERROR_STR_LEN_V01 + 1];
  /**<   Registration failure error string
       when the ims_registered value is FALSE. */

  /* Optional */
  /*  Registration Network */
  uint8_t registration_network_valid;  /**< Must be set to true if registration_network is being passed */
  imsa_service_rat_enum_v01 registration_network;
  /**<   Network is returned when IMS 
       registration is being attempted or is successful. \n
       Values: \n
       - IMSA_WLAN = 0 \n
       - IMSA_WWAN = 1 \n
       - IMSA_IWLAN = 2
   */

  /* Optional */
  /*  P-Associated URI List */
  uint8_t uri_list_valid;  /**< Must be set to true if uri_list is being passed */
  uint32_t uri_list_len;  /**< Must be set to # of elements in uri_list */
  p_associated_uri_v01 uri_list[IMSA_MAX_NUM_URIS_V01];
  /**<   \n P-associated URI list of the current user. This URI list is
       returned in 200 OK SIP messages of the REGISTER packet.
   */
}imsa_get_registration_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Request Message; Gets the service status for various IMS services for the
             requesting control point. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}imsa_get_service_status_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Response Message; Gets the service status for various IMS services for the
             requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  SMS Service Status */
  uint8_t sms_service_status_valid;  /**< Must be set to true if sms_service_status is being passed */
  imsa_service_status_enum_v01 sms_service_status;
  /**<   Values: \n
       - 0 -- IMS SMS service is not available \n
       - 1 -- IMS SMS is in limited service \n
       - 2 -- IMS SMS is in full service
  */

  /* Optional */
  /*  VoIP Service Status */
  uint8_t voip_service_status_valid;  /**< Must be set to true if voip_service_status is being passed */
  imsa_service_status_enum_v01 voip_service_status;
  /**<   Values: \n
       - 0 -- IMS VoIP service is not available \n
       - 2 -- IMS VoIP is in full service
  */

  /* Optional */
  /*  VT Service Status */
  uint8_t vt_service_status_valid;  /**< Must be set to true if vt_service_status is being passed */
  imsa_service_status_enum_v01 vt_service_status;
  /**<   Values: \n
       - 0 -- IMS VT service is not available \n
       - 2 -- IMS VT is in full service
  */

  /* Optional */
  /*  SMS RAT */
  uint8_t sms_service_rat_valid;  /**< Must be set to true if sms_service_rat is being passed */
  imsa_service_rat_enum_v01 sms_service_rat;
  /**<   Values: \n
      - IMSA_WLAN (0) --  IMS service is registered on WLAN \n 
      - IMSA_WWAN (1) --  IMS service is registered on WWAN \n 
      - IMSA_IWLAN (2) --  IMS service is registered on interworking WLAN 
 */

  /* Optional */
  /*  VoIP RAT */
  uint8_t voip_service_rat_valid;  /**< Must be set to true if voip_service_rat is being passed */
  imsa_service_rat_enum_v01 voip_service_rat;
  /**<   Values: \n
      - IMSA_WLAN (0) --  IMS service is registered on WLAN \n 
      - IMSA_WWAN (1) --  IMS service is registered on WWAN \n 
      - IMSA_IWLAN (2) --  IMS service is registered on interworking WLAN 
 */

  /* Optional */
  /*  VT RAT */
  uint8_t vt_service_rat_valid;  /**< Must be set to true if vt_service_rat is being passed */
  imsa_service_rat_enum_v01 vt_service_rat;
  /**<   Values: \n
      - IMSA_WLAN (0) --  IMS service is registered on WLAN \n 
      - IMSA_WWAN (1) --  IMS service is registered on WWAN \n 
      - IMSA_IWLAN (2) --  IMS service is registered on interworking WLAN 
 */

  /* Optional */
  /*  UT Service Status */
  uint8_t ut_service_status_valid;  /**< Must be set to true if ut_service_status is being passed */
  imsa_service_status_enum_v01 ut_service_status;
  /**<   Values: \n
       - 0 -- IMS UT service is not available \n
       - 2 -- IMS UT is in full service
  */

  /* Optional */
  /*  UT RAT */
  uint8_t ut_service_rat_valid;  /**< Must be set to true if ut_service_rat is being passed */
  imsa_service_rat_enum_v01 ut_service_rat;
  /**<   Values: \n
      - IMSA_WLAN (0) --  IMS service is registered on WLAN \n 
      - IMSA_WWAN (1) --  IMS service is registered on WWAN \n 
      - IMSA_IWLAN (2) --  IMS service is registered on interworking WLAN 
 */

  /* Optional */
  /*  VS Service Status */
  uint8_t vs_service_status_valid;  /**< Must be set to true if vs_service_status is being passed */
  imsa_service_status_enum_v01 vs_service_status;
  /**<   Values: \n
       - 0 -- IMS UT service is not available \n
       - 2 -- IMS UT is in full service
  */

  /* Optional */
  /*  VS RAT */
  uint8_t vs_service_rat_valid;  /**< Must be set to true if vs_service_rat is being passed */
  imsa_service_rat_enum_v01 vs_service_rat;
  /**<   Values: \n
      - IMSA_WLAN (0) --  IMS service is registered on WLAN \n 
      - IMSA_WWAN (1) --  IMS service is registered on WWAN \n 
      - IMSA_IWLAN (2) --  IMS service is registered on interworking WLAN 
 */
}imsa_get_service_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Request Message; Gets the PDP status.  */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}imsa_get_pdp_status_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Response Message; Gets the PDP status.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  IMS PDP Connection State */
  uint8_t is_ims_pdp_connected_valid;  /**< Must be set to true if is_ims_pdp_connected is being passed */
  uint8_t is_ims_pdp_connected;
  /**<   IMS PDP connection state information.\n
     Values:\n
       - TRUE  -- IMS PDP is connected \n
       - FALSE -- IMS PDP is not connected
    */

  /* Optional */
  /*  IMS PDP Failure Error Code */
  uint8_t ims_pdp_failure_error_code_valid;  /**< Must be set to true if ims_pdp_failure_error_code is being passed */
  imsa_pdp_failure_error_code_enum_v01 ims_pdp_failure_error_code;
  /**<   IMS PDP connection failure reason when the
 IMS PDP Connection State TLV is FALSE (that is, IMS PDP is not in
 the Connected state). Values:\n
      - IMSA_PDP_STATUS_OTHER_FAILURE (0) --  Generic failure reason for other than specified \n 
      - IMSA_PDP_STATUS_OPTION_UNSUBSCRIBED (1) --  Option is unsubscribed \n 
      - IMSA_PDP_STATUS_UNKNOWN_PDP (2) --  PDP was unknown 
      - IMSA_PDP_STATUS_REASON_NOT_SPECIFIED (3) --  Reason not specified \n
      - IMSA_PDP_STATUS_CONNECTION_BRINGUP_FAILURE (4) --  Connection bring-up failure 
      - IMSA_PDP_STATUS_CONNECTION_IKE_AUTH_FAILURE (5) --  IKE authentication failure 
      - IMSA_PDP_STATUS_USER_AUTH_FAILED (6) --  User authentication failed   
 */

  /* Optional */
  /*  IMS PDP Failure Cause Code */
  uint8_t ims_pdp_failure_cause_code_valid;  /**< Must be set to true if ims_pdp_failure_cause_code is being passed */
  imsa_call_end_failure_reason_type_v01 ims_pdp_failure_cause_code;
  /**<   \n IMS PDP connection failure error cause code when the
     IMS PDP Connection State TLV is FALSE (that is, IMS PDP is not in
     the Connected state).
  */

  /* Optional */
  /*  PDP Failure on Network */
  uint8_t pdp_failure_network_valid;  /**< Must be set to true if pdp_failure_network is being passed */
  imsa_service_rat_enum_v01 pdp_failure_network;
  /**<   Network is returned when IMS 
 PDP is being attempted or is successful. \n
 Values: \n
      - IMSA_WLAN (0) --  IMS service is registered on WLAN \n 
      - IMSA_WWAN (1) --  IMS service is registered on WWAN \n 
      - IMSA_IWLAN (2) --  IMS service is registered on interworking WLAN 
 */
}imsa_get_pdp_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Request Message; Gets the LTE attach parameters.  */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}imsa_get_lte_attach_parameters_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Response Message; Gets the LTE attach parameters.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  LTE Attach Parameters  */
  uint8_t operator_reserved_pco_valid;  /**< Must be set to true if operator_reserved_pco is being passed */
  uint8_t operator_reserved_pco;
  /**<   Operator-reserved PCO value for subscription status. Preconditions: \n
       - Operator must always have an operator-reserved PCO container in the Attach response. \n
       - Applications should query to get the API only when the PDP is connected.
      */
}imsa_get_lte_attach_parameters_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Request Message; Gets the RTP statistics. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}imsa_get_rtp_statistics_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Response Message; Gets the RTP statistics. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
}imsa_get_rtp_statistics_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Request Message; Sets the registration state for different IMS service indications
             for the requesting control point. */
typedef struct {

  /* Optional */
  /*  Registration Status Configuration */
  uint8_t reg_status_config_valid;  /**< Must be set to true if reg_status_config is being passed */
  uint8_t reg_status_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  Service Status Configuration */
  uint8_t service_status_config_valid;  /**< Must be set to true if service_status_config is being passed */
  uint8_t service_status_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  RAT Handover Status Configuration */
  uint8_t rat_handover_config_valid;  /**< Must be set to true if rat_handover_config is being passed */
  uint8_t rat_handover_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  PDP Status Configuration */
  uint8_t pdp_status_config_valid;  /**< Must be set to true if pdp_status_config is being passed */
  uint8_t pdp_status_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  ACS Retrieval Status Configuration */
  uint8_t acs_status_config_valid;  /**< Must be set to true if acs_status_config is being passed */
  uint8_t acs_status_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  LTE Attach Parameters Configuration */
  uint8_t lte_attach_params_config_valid;  /**< Must be set to true if lte_attach_params_config is being passed */
  uint8_t lte_attach_params_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  Subscription Status Configuration */
  uint8_t subscription_status_config_valid;  /**< Must be set to true if subscription_status_config is being passed */
  uint8_t subscription_status_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
    */

  /* Optional */
  /*  Network Provisioning Status Configuration */
  uint8_t network_provisioning_status_config_valid;  /**< Must be set to true if network_provisioning_status_config is being passed */
  uint8_t network_provisioning_status_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  Geolocation Status Configuration */
  uint8_t geo_location_status_config_valid;  /**< Must be set to true if geo_location_status_config is being passed */
  uint8_t geo_location_status_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  VoWiFi Quality Status Configuration */
  uint8_t vowifiprofile_status_config_valid;  /**< Must be set to true if vowifiprofile_status_config is being passed */
  uint8_t vowifiprofile_status_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  Bind Subscription Configuration */
  uint8_t bind_subscription_config_valid;  /**< Must be set to true if bind_subscription_config is being passed */
  uint8_t bind_subscription_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  IMS Block Status Configuration */
  uint8_t ims_block_status_config_valid;  /**< Must be set to true if ims_block_status_config is being passed */
  uint8_t ims_block_status_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  Digits Information Update Indication */
  uint8_t ims_digits_update_info_ind_valid;  /**< Must be set to true if ims_digits_update_info_ind is being passed */
  uint8_t ims_digits_update_info_ind;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  Digits Information Pending Indication */
  uint8_t ims_digits_pending_info_ind_valid;  /**< Must be set to true if ims_digits_pending_info_ind is being passed */
  uint8_t ims_digits_pending_info_ind;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  Voice Configuration */
  uint8_t voice_config_valid;  /**< Must be set to true if voice_config is being passed */
  uint8_t voice_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */

  /* Optional */
  /*  Modem feature */
  uint8_t modem_feature_config_valid;  /**< Must be set to true if modem_feature_config is being passed */
  uint8_t modem_feature_config;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */
}imsa_ind_reg_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Response Message; Sets the registration state for different IMS service indications
             for the requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}imsa_ind_reg_rsp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Request Message; Gets the VoWiFi status.  */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}imsa_get_vowifi_status_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Response Message; Gets the VoWiFi status.  */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  VoWiFi Status Parameters */
  uint8_t vowifiprofile_status_valid;  /**< Must be set to true if vowifiprofile_status is being passed */
  imsa_vowifiprofile_status_enum_v01 vowifiprofile_status;
  /**<   Operator-reserved PCO value for subscription status. Preconditions: \n
       - The operator will always have an operator-reserved PCO container in the Attach response. \n
       - Applications should query to get the API only when the PDP is connected.
      */
}imsa_get_vowifi_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Request Message; Sets the geolocation, including latitude and longitude. */
typedef struct {

  /* Optional */
  /*  City Name */
  uint8_t city_valid;  /**< Must be set to true if city is being passed */
  char city[IMSA_GEO_LOC_CITY_STR_LEN_V01 + 1];
  /**<   City name.  */

  /* Optional */
  /*  Country Code */
  uint8_t countryCode_valid;  /**< Must be set to true if countryCode is being passed */
  char countryCode[IMSA_GEO_LOC_COUNTRY_CODE_STR_LEN_V01 + 1];
  /**<   Country code.  */

  /* Optional */
  /*  Country Name */
  uint8_t countryName_valid;  /**< Must be set to true if countryName is being passed */
  char countryName[IMSA_GEO_LOC_COUNTRY_NAME_STR_LEN_V01 + 1];
  /**<   Country name. */

  /* Optional */
  /*  Zip Code */
  uint8_t zipCode_valid;  /**< Must be set to true if zipCode is being passed */
  char zipCode[IMSA_ZIP_CODE_STR_LEN_V01 + 1];
  /**<   Zip code. */

  /* Optional */
  /*  State */
  uint8_t state_valid;  /**< Must be set to true if state is being passed */
  char state[IMSA_STATE_STR_LEN_V01 + 1];
  /**<   State name. */

  /* Optional */
  /*  Latitude */
  uint8_t latitude_valid;  /**< Must be set to true if latitude is being passed */
  double latitude;
  /**<   Latitude (specified in WGS84 datum).
       \begin{itemize1}
       \item    Type: Floating point
       \item    Units: Degrees
       \item    Range: -90.0 to 90.0   \begin{itemize1}
         \item    Positive values indicate northern latitude
         \item    Negative values indicate southern latitude
       \vspace{-0.18in} \end{itemize1} \end{itemize1} */

  /* Optional */
  /*   Longitude */
  uint8_t longitude_valid;  /**< Must be set to true if longitude is being passed */
  double longitude;
  /**<   Longitude (specified in WGS84 datum). \vspace{-13pt}
       \begin{itemize1}
       \item    Type: Floating point
       \item    Units: Degrees
       \item    Range: -180.0 to 180.0   \begin{itemize1}
         \item    Positive values indicate eastern longitude
         \item    Negative values indicate western longitude
       \vspace{-0.18in} \end{itemize1} \end{itemize1} */

  /* Optional */
  /*  Street */
  uint8_t street_valid;  /**< Must be set to true if street is being passed */
  char street[IMSA_GEO_LOC_STREET_NAME_STR_LEN_V01 + 1];
  /**<   Street name. */

  /* Optional */
  /*  House Number */
  uint8_t housenumber_valid;  /**< Must be set to true if housenumber is being passed */
  char housenumber[IMSA_GEO_LOC_HOUSE_NUMBER_STR_LEN_V01 + 1];
  /**<   House number. */
}imsa_set_geo_loc_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Response Message; Sets the geolocation, including latitude and longitude. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}imsa_set_geo_loc_rsp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Request Message; Binds the subscription for the requesting control point. */
typedef struct {

  /* Optional */
  /*  Bind Subscription */
  uint8_t subscription_type_valid;  /**< Must be set to true if subscription_type is being passed */
  imsa_bind_subscription_type_enum_v01 subscription_type;
  /**<   Subscription type with which to bind. Values: \n

      - IMSA_BIND_SUBSCRIPTION_TYPE_PRIMARY (0) --  Primary subscription 
      - IMSA_BIND_SUBSCRIPTION_TYPE_SECONDARY (1) --  Secondary subscription 
      - IMSA_BIND_SUBSCRIPTION_TYPE_TERTIARY (2) --  Tertiary subscription 

 If not set, the default is IMSA_SUBSCRIPTION_TYPE_PRIMARY.
 */
}imsa_set_bind_subscription_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Response Message; Binds the subscription for the requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
       - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
       - qmi_error_type  -- Error code. Possible error code values are described 
                            in the error codes section of each message 
                            definition.
    */
}imsa_set_bind_subscription_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Request Message; Retrieves the IMS subscription configuration parameters. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}imsa_get_bind_subscription_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Response Message; Retrieves the IMS subscription configuration parameters. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Bind Subscription */
  uint8_t subscription_type_valid;  /**< Must be set to true if subscription_type is being passed */
  imsa_bind_subscription_type_enum_v01 subscription_type;
  /**<   Values: \n

      - IMSA_BIND_SUBSCRIPTION_TYPE_PRIMARY (0) --  Primary subscription 
      - IMSA_BIND_SUBSCRIPTION_TYPE_SECONDARY (1) --  Secondary subscription 
      - IMSA_BIND_SUBSCRIPTION_TYPE_TERTIARY (2) --  Tertiary subscription 
 */
}imsa_get_bind_subscription_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Request Message; Gets the active subscription status for IMS from the
             requesting control point. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}imsa_get_active_subscription_status_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Response Message; Gets the active subscription status for IMS from the
             requesting control point. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Primary Subscription Status */
  uint8_t active_on_primary_subscription_valid;  /**< Must be set to true if active_on_primary_subscription is being passed */
  uint8_t active_on_primary_subscription;
  /**<   Values: \n
       - TRUE  -- IMS is active on the primary subscription \n
       - FALSE -- IMS is not active on the primary subscription (default)
   */

  /* Optional */
  /*  Secondary Subscription Status */
  uint8_t active_on_secondary_subscription_valid;  /**< Must be set to true if active_on_secondary_subscription is being passed */
  uint8_t active_on_secondary_subscription;
  /**<   Values: \n
       - TRUE  -- IMS is active on the secondary subscription \n
       - FALSE -- IMS is not active on the secondary subscription (default)
   */

  /* Optional */
  /*  Tertiary Subscription Status */
  uint8_t active_on_tertiary_subscription_valid;  /**< Must be set to true if active_on_tertiary_subscription is being passed */
  uint8_t active_on_tertiary_subscription;
  /**<   Values: \n
       - TRUE  -- IMS is active on the tertiary subscription \n
       - FALSE -- IMS is not active on the tertiary subscription (default)
   */
}imsa_get_active_subscription_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Request Message; Retrieves the IMS block status. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}imsa_get_ims_block_status_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Response Message; Retrieves the IMS block status. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;

  /* Optional */
  /*  Failure Reason of IMS over WWAN */
  uint8_t ims_failure_on_wwan_valid;  /**< Must be set to true if ims_failure_on_wwan is being passed */
  imsa_ims_failure_reason_type_v01 ims_failure_on_wwan;

  /* Optional */
  /*  Failure Reason of IMS over WLAN */
  uint8_t ims_failure_on_wlan_valid;  /**< Must be set to true if ims_failure_on_wlan is being passed */
  imsa_ims_failure_reason_type_v01 ims_failure_on_wlan;
}imsa_get_ims_block_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Request Message; Resets the IMS blocking status. */
typedef struct {

  /* Mandatory */
  /*  PDN Status */
  uint8_t pdn_unblock;
  /**<   Values: \n
       - TRUE  -- Not blocked \n
       - FALSE -- Blocked (default)
   */

  /* Mandatory */
  /*  Registration Status */
  uint8_t registration_unblock;
  /**<   Values: \n
       - TRUE  -- Not blocked \n
       - FALSE -- Blocked (default)
   */
}imsa_reset_ims_blocking_status_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Response Message; Resets the IMS blocking status. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/
}imsa_reset_ims_blocking_status_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Request Message; Gets the virtual line information. */
typedef struct {

  /* Mandatory */
  /*  Get Virtual Line Request */
  char msisdn[IMSA_MSISDN_LEN_V01 + 1];
  /**<   MSISDN of the digits line for which the p_associated_uri list is needed. */
}imsa_get_virtual_line_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Response Message; Gets the virtual line information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type.*/

  /* Optional */
  /*  Virtual Line Information */
  uint8_t virtual_line_info_valid;  /**< Must be set to true if virtual_line_info is being passed */
  imsa_virtual_line_info_v01 virtual_line_info;
}imsa_get_virtual_line_rsp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Indication Message; Indication sent when the IMS service registration status changes. */
typedef struct {

  /* Mandatory */
  /*  IMS Registration Status (Deprecated) */
  uint8_t ims_registered;
  /**<   Values: \n
       - TRUE  -- UE is registered on the IMS network \n
       - FALSE -- UE is not registered on the IMS network \n
       Note: This TLV is deprecated; use the New IMS Registration Status 
       TLV instead.
   */

  /* Optional */
  /*  IMS Registration Error Code */
  uint8_t ims_registration_failure_error_code_valid;  /**< Must be set to true if ims_registration_failure_error_code is being passed */
  uint16_t ims_registration_failure_error_code;
  /**<   An error code is returned when the IMS
       registration status is IMSA_STATUS_NOT_REGISTERED.\n
       Values: \n
       - 3xx -- Redirection responses \n
       - 4xx -- Client failure responses \n
       - 5xx -- Server failure responses \n
       - 6xx -- Global failure responses
  */

  /* Optional */
  /*  New IMS Registration Status */
  uint8_t ims_reg_status_valid;  /**< Must be set to true if ims_reg_status is being passed */
  imsa_ims_registration_status_enum_v01 ims_reg_status;
  /**<   Values: \n
      - IMSA_STATUS_NOT_REGISTERED (0) --  Not registered for IMS \n 
      - IMSA_STATUS_REGISTERING (1) --  Registering for IMS \n 
      - IMSA_STATUS_REGISTERED (2) --  Registered for IMS 
      - IMSA_STATUS_LIMITED_REGISTERED (3) --  Limited registration for IMS 
 */

  /* Optional */
  /*  Registration Error String */
  uint8_t registration_error_string_valid;  /**< Must be set to true if registration_error_string is being passed */
  char registration_error_string[IMSA_REGISTRATION_FAILURE_ERROR_STR_LEN_V01 + 1];
  /**<   Registration failure error string
       when the ims_registered is FALSE. */

  /* Optional */
  /*  Registration Network */
  uint8_t registration_network_valid;  /**< Must be set to true if registration_network is being passed */
  imsa_service_rat_enum_v01 registration_network;
  /**<   Network is returned when IMS 
       registration is being attempted or is successful. \n
       Values: \n
       - IMSA_WLAN = 0 \n
       - IMSA_WWAN = 1 \n
       - IMSA_IWLAN = 2
   */

  /* Optional */
  /*  P-Associated URI List */
  uint8_t uri_list_valid;  /**< Must be set to true if uri_list is being passed */
  uint32_t uri_list_len;  /**< Must be set to # of elements in uri_list */
  p_associated_uri_v01 uri_list[IMSA_MAX_NUM_URIS_V01];
  /**<   \n P-associated URI list of the current user. This URI list is
       returned in the 200 OK SIP messages of the REGISTER packet.
   */
}imsa_registration_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Indication Message; Indication sent with the latitude and longitude of the geolocation.  */
typedef struct {

  /* Optional */
  /*  Latitude */
  uint8_t latitude_valid;  /**< Must be set to true if latitude is being passed */
  double latitude;
  /**<   Latitude (specified in WGS84 datum).
       \begin{itemize1}
       \item    Type: Floating point
       \item    Units: Degrees
       \item    Range: -90.0 to 90.0   \begin{itemize1}
         \item    Positive values indicate northern latitude
         \item    Negative values indicate southern latitude
       \vspace{-0.18in} \end{itemize1} \end{itemize1} */

  /* Optional */
  /*   Longitude */
  uint8_t longitude_valid;  /**< Must be set to true if longitude is being passed */
  double longitude;
  /**<   Longitude (specified in WGS84 datum). \vspace{-13pt}
       \begin{itemize1}
       \item    Type: Floating point
       \item    Units: Degrees
       \item    Range: -180.0 to 180.0   \begin{itemize1}
         \item    Positive values indicate eastern longitude
         \item    Negative values indicate western longitude
       \vspace{-0.18in} \end{itemize1} \end{itemize1} */
}imsa_geo_loc_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Indication Message; Indication sent when the service status of an IMS service changes. */
typedef struct {

  /* Optional */
  /*  SMS Service Status */
  uint8_t sms_service_status_valid;  /**< Must be set to true if sms_service_status is being passed */
  imsa_service_status_enum_v01 sms_service_status;
  /**<   Values: \n
       - 0 -- IMS SMS service is not available \n
       - 1 -- IMS SMS is in limited service \n
       - 2 -- IMS SMS is in full service
   */

  /* Optional */
  /*  VoIP Service Status */
  uint8_t voip_service_status_valid;  /**< Must be set to true if voip_service_status is being passed */
  imsa_service_status_enum_v01 voip_service_status;
  /**<   Values: \n
       - 0 -- IMS VoIP service is not available \n
       - 2 -- IMS VoIP is in full service
    */

  /* Optional */
  /*  VT Service Status */
  uint8_t vt_service_status_valid;  /**< Must be set to true if vt_service_status is being passed */
  imsa_service_status_enum_v01 vt_service_status;
  /**<   Values: \n
       - 0 -- IMS VT service is not available \n
       - 2 -- IMS VT is in full service
  */

  /* Optional */
  /*  SMS RAT */
  uint8_t sms_service_rat_valid;  /**< Must be set to true if sms_service_rat is being passed */
  imsa_service_rat_enum_v01 sms_service_rat;
  /**<   Values: \n
      - IMSA_WLAN (0) --  IMS service is registered on WLAN \n 
      - IMSA_WWAN (1) --  IMS service is registered on WWAN \n 
      - IMSA_IWLAN (2) --  IMS service is registered on interworking WLAN 
 */

  /* Optional */
  /*  VoIP RAT */
  uint8_t voip_service_rat_valid;  /**< Must be set to true if voip_service_rat is being passed */
  imsa_service_rat_enum_v01 voip_service_rat;
  /**<   Values: \n
      - IMSA_WLAN (0) --  IMS service is registered on WLAN \n 
      - IMSA_WWAN (1) --  IMS service is registered on WWAN \n 
      - IMSA_IWLAN (2) --  IMS service is registered on interworking WLAN 
 */

  /* Optional */
  /*  VT RAT */
  uint8_t vt_service_rat_valid;  /**< Must be set to true if vt_service_rat is being passed */
  imsa_service_rat_enum_v01 vt_service_rat;
  /**<   Values: \n
      - IMSA_WLAN (0) --  IMS service is registered on WLAN \n 
      - IMSA_WWAN (1) --  IMS service is registered on WWAN \n 
      - IMSA_IWLAN (2) --  IMS service is registered on interworking WLAN 
 */

  /* Optional */
  /*  UT Service Status */
  uint8_t ut_service_status_valid;  /**< Must be set to true if ut_service_status is being passed */
  imsa_service_status_enum_v01 ut_service_status;
  /**<   Values: \n
       - 0 -- IMS UT service is not available \n
       - 2 -- IMS UT is in full service
  */

  /* Optional */
  /*  UT RAT */
  uint8_t ut_service_rat_valid;  /**< Must be set to true if ut_service_rat is being passed */
  imsa_service_rat_enum_v01 ut_service_rat;
  /**<   Values: \n
      - IMSA_WLAN (0) --  IMS service is registered on WLAN \n 
      - IMSA_WWAN (1) --  IMS service is registered on WWAN \n 
      - IMSA_IWLAN (2) --  IMS service is registered on interworking WLAN 
 */

  /* Optional */
  /*  VS Service Status */
  uint8_t vs_service_status_valid;  /**< Must be set to true if vs_service_status is being passed */
  imsa_service_status_enum_v01 vs_service_status;
  /**<   Values: \n
       - 0 -- IMS UT service is not available \n
       - 2 -- IMS UT is in full service
  */

  /* Optional */
  /*  VS RAT */
  uint8_t vs_service_rat_valid;  /**< Must be set to true if vs_service_rat is being passed */
  imsa_service_rat_enum_v01 vs_service_rat;
  /**<   Values: \n
      - IMSA_WLAN (0) --  IMS service is registered on WLAN \n 
      - IMSA_WWAN (1) --  IMS service is registered on WWAN \n 
      - IMSA_IWLAN (2) --  IMS service is registered on interworking WLAN 
 */
}imsa_service_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Indication Message; Indication sent when the status of a RAT handover changes. */
typedef struct {

  /* Optional */
  /*  RAT Handover Status Information */
  uint8_t rat_ho_status_info_valid;  /**< Must be set to true if rat_ho_status_info is being passed */
  imsa_rat_handover_status_info_v01 rat_ho_status_info;

  /* Optional */
  /*  RAT Handover Failure Reason */
  uint8_t handover_failure_reason_valid;  /**< Must be set to true if handover_failure_reason is being passed */
  imsa_call_end_failure_reason_type_v01 handover_failure_reason;
}imsa_rat_handover_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Indication Message; Indication sent when the status of a PDP connection changes. */
typedef struct {

  /* Mandatory */
  /*  IMS PDP Connection State */
  uint8_t is_ims_pdp_connected;
  /**<   Values:\n
       - TRUE  -- IMS PDP is connected \n
       - FALSE -- IMS PDP is not connected
    */

  /* Optional */
  /*  IMS PDP Failure Error Code */
  uint8_t ims_pdp_failure_error_code_valid;  /**< Must be set to true if ims_pdp_failure_error_code is being passed */
  imsa_pdp_failure_error_code_enum_v01 ims_pdp_failure_error_code;
  /**<   IMS PDP connection failure reason when the
 IMS PDP Connection State TLV is FALSE (that is, IMS PDP is not in
 the Connected state). Values:\n
      - IMSA_PDP_STATUS_OTHER_FAILURE (0) --  Generic failure reason for other than specified \n 
      - IMSA_PDP_STATUS_OPTION_UNSUBSCRIBED (1) --  Option is unsubscribed \n 
      - IMSA_PDP_STATUS_UNKNOWN_PDP (2) --  PDP was unknown 
      - IMSA_PDP_STATUS_REASON_NOT_SPECIFIED (3) --  Reason not specified \n
      - IMSA_PDP_STATUS_CONNECTION_BRINGUP_FAILURE (4) --  Connection bring-up failure 
      - IMSA_PDP_STATUS_CONNECTION_IKE_AUTH_FAILURE (5) --  IKE authentication failure 
      - IMSA_PDP_STATUS_USER_AUTH_FAILED (6) --  User authentication failed   
 */

  /* Optional */
  /*  IMS PDP Failure Cause Code */
  uint8_t ims_pdp_failure_cause_code_valid;  /**< Must be set to true if ims_pdp_failure_cause_code is being passed */
  imsa_call_end_failure_reason_type_v01 ims_pdp_failure_cause_code;
  /**<   \n IMS PDP connection failure error cause code when the
     IMS PDP Connection State TLV is FALSE (that is, IMS PDP is not in
     the Connected state).
  */

  /* Optional */
  /*  PDP Failure on Network */
  uint8_t pdp_failure_network_valid;  /**< Must be set to true if pdp_failure_network is being passed */
  imsa_service_rat_enum_v01 pdp_failure_network;
  /**<   The network is returned when IMS 
 registration is being attempted or is successful. \n
 Values: \n
      - IMSA_WLAN (0) --  IMS service is registered on WLAN \n 
      - IMSA_WWAN (1) --  IMS service is registered on WWAN \n 
      - IMSA_IWLAN (2) --  IMS service is registered on interworking WLAN 
 */
}imsa_pdp_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Indication Message; Indication sent with the ACS retrieval status.	This indication
             currently sends the status only when the ACS retrieval fails. */
typedef struct {

  /* Mandatory */
  /*  ACS Failure Status Code */
  imsa_acs_failure_status_code_enum_v01 acs_failure_status_code;
  /**<   ACS failure status code when the ACS retrieval procedure fails.
 Values:\n
      - IMSA_ACS_FAILURE_NO_EXISTING_ACS_DATA (0) --  ACS failure; there is no earlier APCS success data, and
       the ISIM parameters are not available \n 
      - IMSA_ACS_FAILURE_ERROR_CODE_RECEIVED (1) --  Error code received from the network for an
       ACS configuration retrieval request 
 */

  /* Optional */
  /*  ACS Failure Error Code Received */
  uint8_t acs_failure_error_code_valid;  /**< Must be set to true if acs_failure_error_code is being passed */
  uint16_t acs_failure_error_code;
  /**<   ACS failure error code received from the network for	a
       configuration retrieval request when the
       ACS retrieval procedure fails.
  */
}imsa_acs_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Indication Message; Indication sent when LTE attach parameters change. */
typedef struct {

  /* Mandatory */
  /*  LTE Attach Parameters */
  uint8_t operator_reserved_pco;
  /**<   Operator-reserved PCO value for subscription status.
    */
}imsa_lte_attach_parameters_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Indication Message; Indication sent with the subscription status. */
typedef struct {

  /* Mandatory */
  /*  Subscription Type */
  imsa_subscription_type_enum_v01 subscription_type;
  /**<   Subscription type. \n
 Values: \n
      - IMSA_SUBSCRIPTION_TYPE_REG (0) --  Register subscription\n 
      - IMSA_SUBSCRIPTION_TYPE_MWI (1) --  MWI subscription 
      - IMSA_SUBSCRIPTION_TYPE_CONFERENCE (2) --  Conference subscription 
 */

  /* Mandatory */
  /*  Subscription Status */
  uint8_t subscription_status;
  /**<   Values:\n
       - TRUE  -- Subscription Success \n
       - FALSE -- Subscription Failure
    */

  /* Optional */
  /*  Subscription Error String */
  uint8_t subscription_error_string_valid;  /**< Must be set to true if subscription_error_string is being passed */
  char subscription_error_string[IMSA_SUBSCRIPTION_FAILURE_ERROR_STR_LEN_V01 + 1];
  /**<   Subscription failure error string
       when the subscription_status is FALSE. */

  /* Optional */
  /*  Subscription Error Code Received */
  uint8_t subscription_error_code_valid;  /**< Must be set to true if subscription_error_code is being passed */
  uint16_t subscription_error_code;
  /**<   Subscription error code received from the network
       when the subscription_status is FALSE.*/
}imsa_subscription_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Indication Message; Indication sent with the network provisioning status. */
typedef struct {

  /* Mandatory */
  /*  IMS Network Provisioning Status */
  imsa_network_provisioning_status_type_enum_v01 network_provisioning_status;
  /**<   Values:\n
      - IMSA_NETWORK_PROV_OUT_OF_SYNC (0) --  Network is out of sync. This is an 
       indication sent when the IMS UE detects the network provisioning status through 
       SIP messages. When the network is out of sync with the UE, DM resync 
       may be triggered by the OEM based on this indication.
  
 */
}imsa_network_provisioning_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Indication Message; Indication sent when the RTP statistics request is queried */
typedef struct {

  /* Optional */
  /*  RTP Statistics Parameter Packet Count */
  uint8_t total_rx_expected_rtp_pkt_count_valid;  /**< Must be set to true if total_rx_expected_rtp_pkt_count is being passed */
  uint64_t total_rx_expected_rtp_pkt_count;
  /**<   Total number of packets expected on Rx
       from the beginning of the call until the time of the request,
       measured across all opened sessions.
    */

  /* Optional */
  /*  RTP Statistics Parameter Packets Lost */
  uint8_t total_rx_rtp_pkt_loss_count_valid;  /**< Must be set to true if total_rx_rtp_pkt_loss_count is being passed */
  uint64_t total_rx_rtp_pkt_loss_count;
  /**<   Total number of packets lost on Rx
       from the beginning of the call until the time of the request,
       measured across all the opened sessions.
    */
}imsa_rtp_statistics_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Indication Message; Indication sent with VoWiFi status changes. */
typedef struct {

  /* Mandatory */
  /*  IMS VoWiFi Quality Status */
  imsa_vowifiprofile_status_enum_v01 vowifiprofile_status;
  /**<   Values:\n
      - IMSA_SERVICE_VOWIFI_QUALITIY_NO_ERR (0) --   No error  
      - IMSA_SERVICE_VOWIFI_QUALITIY_EXCELLENT (1) --   Excellent indication  
      - IMSA_SERVICE_VOWIFI_QUALITIY_FAIR (2) --   Fair indication 
      - IMSA_SERVICE_VOWIFI_QUALITIY_BAD (3) --   Bad indication 
 */
}imsa_vowifiprofile_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Indication Message; Indication sent with subscription status changes. */
typedef struct {

  /* Optional */
  /*  Bind Subscription */
  uint8_t subscription_type_valid;  /**< Must be set to true if subscription_type is being passed */
  imsa_bind_subscription_type_enum_v01 subscription_type;
  /**<   Active subscription information. Values: \n

      - IMSA_BIND_SUBSCRIPTION_TYPE_PRIMARY (0) --  Primary subscription 
      - IMSA_BIND_SUBSCRIPTION_TYPE_SECONDARY (1) --  Secondary subscription 
      - IMSA_BIND_SUBSCRIPTION_TYPE_TERTIARY (2) --  Tertiary subscription 
 */
}imsa_bind_subscription_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Indication Message; Indication sent with subscription status changes. */
typedef struct {

  /* Optional */
  /*  Blocking Reason of IMS over WWAN */
  uint8_t ims_failure_on_wwan_valid;  /**< Must be set to true if ims_failure_on_wwan is being passed */
  imsa_ims_failure_reason_type_v01 ims_failure_on_wwan;

  /* Optional */
  /*  Blocking Reason of IMS over WLAN */
  uint8_t ims_failure_on_wlan_valid;  /**< Must be set to true if ims_failure_on_wlan is being passed */
  imsa_ims_failure_reason_type_v01 ims_failure_on_wlan;
}imsa_ims_block_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Indication Message; Informs clients whether IMS is active or inactive in the subscription to which the client is bound. */
typedef struct {

  /* Mandatory */
  /*  IMS Active Subscription Status */
  uint8_t isActive;
  /**<   Specifies whether IMS is active.
  Values: \n
  - TRUE -- IMS subscription is active.  \n
  - FALSE -- IMS subscription is not active.
  */
}imsa_active_subscription_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Indication Message; Informs clients whether the number of virtual lines received 200 OK registration response. */
typedef struct {

  /* Optional */
  /*  Digits Information Update Indication */
  uint8_t no_of_lines_valid;  /**< Must be set to true if no_of_lines is being passed */
  uint8_t no_of_lines;
  /**<   Total number of virtual lines the modem sends information to the HLOS */

  /* Optional */
  /*  Digits Information Pending Indication */
  uint8_t line_info_list_valid;  /**< Must be set to true if line_info_list is being passed */
  uint32_t line_info_list_len;  /**< Must be set to # of elements in line_info_list */
  imsa_digits_line_info_v01 line_info_list[IMSA_MAX_NO_VIRTUAL_LINES_V01];
  /**<   This item has details about the information of each virtual line with 
  status that is sent to the IMS registration */
}imsa_digits_update_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Indication Message; Sends request to the HLOS to request digits information (APM, SIM refresh, and so forth). */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}imsa_digits_info_pending_ind_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Indication Message; Indication sent with voice status changes. */
typedef struct {

  /* Mandatory */
  /*  IMS Voice Status */
  imsa_voice_info_enum_v01 voice_info;
  /**<   Values:\n
      - IMSA_VOICE_INFO_SILENT (0) --  IMS voice status silent \n 
      - IMSA_VOICE_INFO_SPEECH (1) --  IMS voice status speech 
 */
}imsa_voice_info_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsa_qmi_messages
    @{
  */
/** Indication Message; Indicates when the modem specific feature flag gets enabled. */
typedef struct {

  /* Mandatory */
  /*  IMSA Feature Flag Status */
  imsa_modem_feature_mask_v01 modem_feature;
  /**<   Bitmask indicating which services are enabled on the device. */
}imsa_modem_feature_info_ind_v01;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */ 
//#define REMOVE_QMI_IMSA_ACS_STATUS_IND_V01 
//#define REMOVE_QMI_IMSA_ACTIVE_SUBSCRIPTION_IND_V01 
//#define REMOVE_QMI_IMSA_BIND_SUBSCRIPTION_STATUS_IND_V01 
//#define REMOVE_QMI_IMSA_DIGITS_INFO_PENDING_IND_V01 
//#define REMOVE_QMI_IMSA_DIGITS_UPDATE_IND_V01 
//#define REMOVE_QMI_IMSA_GEO_LOC_STATUS_IND_V01 
//#define REMOVE_QMI_IMSA_GET_ACTIVE_SUBSCRIPTION_STATUS_V01 
//#define REMOVE_QMI_IMSA_GET_BIND_SUBSCRIPTION_V01 
//#define REMOVE_QMI_IMSA_GET_IMS_BLOCK_STATUS_V01 
//#define REMOVE_QMI_IMSA_GET_LTE_ATTACH_PARAMETERS_V01 
//#define REMOVE_QMI_IMSA_GET_PDP_STATUS_V01 
//#define REMOVE_QMI_IMSA_GET_REGISTRATION_STATUS_V01 
//#define REMOVE_QMI_IMSA_GET_RTP_STATISTICS_V01 
//#define REMOVE_QMI_IMSA_GET_SERVICE_STATUS_V01 
//#define REMOVE_QMI_IMSA_GET_SUPPORTED_FIELDS_V01 
//#define REMOVE_QMI_IMSA_GET_SUPPORTED_MSGS_V01 
//#define REMOVE_QMI_IMSA_GET_VIRTUAL_LINE_V01 
//#define REMOVE_QMI_IMSA_GET_VoWIFI_STATUS_V01 
//#define REMOVE_QMI_IMSA_IMS_BLOCK_STATUS_IND_V01 
//#define REMOVE_QMI_IMSA_IND_REG_V01 
//#define REMOVE_QMI_IMSA_LTE_ATTACH_PARAMETERS_IND_V01 
//#define REMOVE_QMI_IMSA_MODEM_FEATURE_INFO_IND_V01 
//#define REMOVE_QMI_IMSA_NETWORK_PROVISIONING_STATUS_IND_V01 
//#define REMOVE_QMI_IMSA_PDP_STATUS_IND_V01 
//#define REMOVE_QMI_IMSA_RAT_HANDOVER_STATUS_IND_V01 
//#define REMOVE_QMI_IMSA_REGISTRATION_STATUS_IND_V01 
//#define REMOVE_QMI_IMSA_RESET_IMS_BLOCKING_STATUS_V01 
//#define REMOVE_QMI_IMSA_RTP_STATISTICS_IND_V01 
//#define REMOVE_QMI_IMSA_SERVICE_STATUS_IND_V01 
//#define REMOVE_QMI_IMSA_SET_BIND_SUBSCRIPTION_V01 
//#define REMOVE_QMI_IMSA_SUBSCRIPTION_STATUS_IND_V01 
//#define REMOVE_QMI_IMSA_VOICE_INFO_IND_V01 
//#define REMOVE_QMI_IMSA_VoWIFI_STATUS_IND_V01 
//#define REMOVE_QMI_SET_IND_GEO_LOC_V01 

/*Service Message Definition*/
/** @addtogroup imsa_qmi_msg_ids
    @{
  */
#define QMI_IMSA_GET_SUPPORTED_MSGS_REQ_V01 0x001E
#define QMI_IMSA_GET_SUPPORTED_MSGS_RESP_V01 0x001E
#define QMI_IMSA_GET_SUPPORTED_FIELDS_REQ_V01 0x001F
#define QMI_IMSA_GET_SUPPORTED_FIELDS_RESP_V01 0x001F
#define QMI_IMSA_GET_REGISTRATION_STATUS_REQ_V01 0x0020
#define QMI_IMSA_GET_REGISTRATION_STATUS_RSP_V01 0x0020
#define QMI_IMSA_GET_SERVICE_STATUS_REQ_V01 0x0021
#define QMI_IMSA_GET_SERVICE_STATUS_RSP_V01 0x0021
#define QMI_IMSA_IND_REG_REQ_V01 0x0022
#define QMI_IMSA_IND_REG_RSP_V01 0x0022
#define QMI_IMSA_REGISTRATION_STATUS_IND_V01 0x0023
#define QMI_IMSA_SERVICE_STATUS_IND_V01 0x0024
#define QMI_IMSA_RAT_HANDOVER_STATUS_IND_V01 0x0025
#define QMI_IMSA_PDP_STATUS_IND_V01 0x0026
#define QMI_IMSA_ACS_STATUS_IND_V01 0x0027
#define QMI_IMSA_GET_PDP_STATUS_REQ_V01 0x0028
#define QMI_IMSA_GET_PDP_STATUS_RSP_V01 0x0028
#define QMI_IMSA_LTE_ATTACH_PARAMETERS_IND_V01 0x0029
#define QMI_IMSA_GET_LTE_ATTACH_PARAMETERS_REQ_V01 0x002A
#define QMI_IMSA_GET_LTE_ATTACH_PARAMETERS_RSP_V01 0x002A
#define QMI_IMSA_SUBSCRIPTION_STATUS_IND_V01 0x002B
#define QMI_IMSA_NETWORK_PROVISIONING_STATUS_IND_V01 0x002C
#define QMI_IMSA_GET_RTP_STATISTICS_REQ_V01 0x002D
#define QMI_IMSA_GET_RTP_STATISTICS_RSP_V01 0x002D
#define QMI_IMSA_RTP_STATISTICS_IND_V01 0x002E
#define QMI_IMSA_SET_GEO_LOC_REQ_V01 0x002F
#define QMI_IMSA_SET_GEO_LOC_RSP_V01 0x002F
#define QMI_IMSA_GEO_LOC_STATUS_IND_V01 0x0030
#define QMI_IMSA_GET_VoWIFI_STATUS_REQ_V01 0x0031
#define QMI_IMSA_GET_VoWIFI_STATUSRSP_V01 0x0031
#define QMI_IMSA_VoWIFI_STATUS_IND_V01 0x0032
#define QMI_IMSA_SET_BIND_SUBSCRIPTION_REQ_V01 0x0033
#define QMI_IMSA_SET_BIND_SUBSCRIPTION_RESP_V01 0x0033
#define QMI_IMSA_GET_BIND_SUBSCRIPTION_REQ_V01 0x0034
#define QMI_IMSA_GET_BIND_SUBSCRIPTION_RESP_V01 0x0034
#define QMI_IMSA_BIND_SUBSCRIPTION_STATUS_IND_V01 0x0035
#define QMI_IMSA_GET_ACTIVE_SUBSCRIPTION_STATUS_REQ_V01 0x0036
#define QMI_IMSA_GET_ACTIVE_SUBSCRIPTION_STATUS_RESP_V01 0x0036
#define QMI_IMSA_GET_IMS_BLOCK_STATUS_REQ_V01 0x0037
#define QMI_IMSA_GET_IMS_BLOCK_STATUS_RESP_V01 0x0037
#define QMI_IMSA_IMS_BLOCK_STATUS_IND_V01 0x0038
#define QMI_IMSA_ACTIVE_SUBSCRIPTION_IND_V01 0x0039
#define QMI_IMSA_RESET_IMS_BLOCKING_STATUS_REQ_V01 0x003A
#define QMI_IMSA_RESET_IMS_BLOCKING_STATUS_RESP_V01 0x003A
#define QMI_IMSA_DIGITS_UPDATE_IND_V01 0x003B
#define QMI_IMSA_DIGITS_INFO_PENDING_IND_V01 0x003C
#define QMI_IMSA_GET_VIRTUAL_LINE_REQ_V01 0x003D
#define QMI_IMSA_GET_VIRTUAL_LINE_RSP_V01 0x003D
#define QMI_IMSA_VOICE_INFO_IND_V01 0x003E
#define QMI_IMSA_MODEM_FEATURE_INFO_IND_V01 0x003F
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro imsa_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type imsa_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );

/** This macro should be used to get the service object */
#define imsa_get_service_object_v01( ) \
          imsa_get_service_object_internal_v01( \
            IMSA_V01_IDL_MAJOR_VERS, IMSA_V01_IDL_MINOR_VERS, \
            IMSA_V01_IDL_TOOL_VERS )
/**
    @}
  */


#ifdef __cplusplus
}
#endif
#endif

