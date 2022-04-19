#ifndef WIRELESS_DATA_SERVICE_COMMON_SERVICE_01_H
#define WIRELESS_DATA_SERVICE_COMMON_SERVICE_01_H
/**
  @file wireless_data_service_common_v01.h

  @brief This is the public header file which defines the wireless_data_service_common service Data structures.

  This header file defines the types and structures that were defined in
  wireless_data_service_common. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2015, 2019 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.14.7
   It was generated on: Wed Sep 11 2019 (Spin 0)
   From IDL File: wireless_data_service_common_v01.idl */

/** @defgroup wireless_data_service_common_qmi_consts Constant values defined in the IDL */
/** @defgroup wireless_data_service_common_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup wireless_data_service_common_qmi_enums Enumerated types used in QMI messages */
/** @defgroup wireless_data_service_common_qmi_messages Structures sent as QMI messages */
/** @defgroup wireless_data_service_common_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup wireless_data_service_common_qmi_accessor Accessor for QMI service object */
/** @defgroup wireless_data_service_common_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup wireless_data_service_common_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define WIRELESS_DATA_SERVICE_COMMON_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define WIRELESS_DATA_SERVICE_COMMON_V01_IDL_MINOR_VERS 0x06
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define WIRELESS_DATA_SERVICE_COMMON_V01_IDL_TOOL_VERS 0x06

/**
    @}
  */


/** @addtogroup wireless_data_service_common_qmi_consts
    @{
  */

/**  */
#define QMI_WDS_APN_NAME_MAX_V01 150
#define QMI_WDS_MAX_BEARER_CONTEXT_V01 8
/**
    @}
  */

/** @addtogroup wireless_data_service_common_qmi_enums
    @{
  */
typedef enum {
  WDS_IP_FAMILY_PREFERENCE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_IP_FAMILY_PREF_NON_IP_V01 = 0x02,
  WDS_IP_FAMILY_PREF_IPV4_V01 = 0x04,
  WDS_IP_FAMILY_PREF_IPV6_V01 = 0x06,
  WDS_IP_FAMILY_PREF_UNSPEC_V01 = 0x08,
  WDS_IP_FAMILY_PREFERENCE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_ip_family_preference_enum_v01;
/**
    @}
  */

/** @addtogroup wireless_data_service_common_qmi_enums
    @{
  */
typedef enum {
  WDS_EXT_TECH_PREF_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_EXT_TECH_PREF_CDMA_V01 = -32767,
  WDS_EXT_TECH_PREF_UMTS_V01 = -32764,
  WDS_EXT_TECH_PREF_EMBMS_V01 = -30590,
  WDS_EXT_TECH_PREF_MODEM_LINK_LOCAL_V01 = -30584,
  WDS_EXT_TECH_PREF_NON_IP_V01 = -30588,
  WDS_EXT_TECH_PREF_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_ext_tech_pref_enum_v01;
/**
    @}
  */

/** @addtogroup wireless_data_service_common_qmi_enums
    @{
  */
typedef enum {
  WDS_APN_TYPE_ENUM_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  WDS_APN_TYPE_UNSPECIFIED_V01 = 0, /**<  APN type unspecified \n  */
  WDS_APN_TYPE_INTERNET_V01 = 1, /**<  APN type for internet traffic \n  */
  WDS_APN_TYPE_IMS_V01 = 2, /**<  APN type for IMS \n  */
  WDS_APN_TYPE_VSIM_V01 = 3, /**<  APN type for virtual or remote SIM    */
  WDS_APN_TYPE_MMS_V01 = 4, /**<  APN type for MMS    */
  WDS_APN_TYPE_DUN_V01 = 5, /**<  APN type for DUN    */
  WDS_APN_TYPE_SUPL_V01 = 6, /**<  APN type for SUPL    */
  WDS_APN_TYPE_HIPRI_V01 = 7, /**<  APN type for HIPRI    */
  WDS_APN_TYPE_FOTA_V01 = 8, /**<  APN type for FOTA    */
  WDS_APN_TYPE_CBS_V01 = 9, /**<  APN type for CBS    */
  WDS_APN_TYPE_IA_V01 = 10, /**<  APN type for IA   */
  WDS_APN_TYPE_EMERGENCY_V01 = 11, /**<  APN type for Emergency
 APN TYPE 12 Reserved  */
  WDS_APN_TYPE_MCX_V01 = 13, /**<  APN type for mission critical service   */
  WDS_APN_TYPE_ENUM_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}wds_apn_type_enum_v01;
/**
    @}
  */

typedef uint64_t wds_apn_type_mask_v01;
#define WDS_APN_TYPE_MASK_UNSPECIFIED_V01 ((wds_apn_type_mask_v01)0x00ull)
#define WDS_APN_TYPE_MASK_DEFAULT_V01 ((wds_apn_type_mask_v01)0x01ull) /**<  APN type for default/internet traffic  */
#define WDS_APN_TYPE_MASK_IMS_V01 ((wds_apn_type_mask_v01)0x02ull) /**<  APN type for the IP multimedia subsystem  */
#define WDS_APN_TYPE_MASK_MMS_V01 ((wds_apn_type_mask_v01)0x04ull) /**<  APN type for the multimedia messaging service  */
#define WDS_APN_TYPE_MASK_DUN_V01 ((wds_apn_type_mask_v01)0x08ull) /**<  APN type for the dial up network  */
#define WDS_APN_TYPE_MASK_SUPL_V01 ((wds_apn_type_mask_v01)0x10ull) /**<  APN type for secure user plane location  */
#define WDS_APN_TYPE_MASK_HIPRI_V01 ((wds_apn_type_mask_v01)0x20ull) /**<  APN type for high priority mobile data  */
#define WDS_APN_TYPE_MASK_FOTA_V01 ((wds_apn_type_mask_v01)0x40ull) /**<  APN type for over the air administration  */
#define WDS_APN_TYPE_MASK_CBS_V01 ((wds_apn_type_mask_v01)0x80ull) /**<  APN type for carrier branded services  */
#define WDS_APN_TYPE_MASK_IA_V01 ((wds_apn_type_mask_v01)0x100ull) /**<  APN type for initial attach  */
#define WDS_APN_TYPE_MASK_EMERGENCY_V01 ((wds_apn_type_mask_v01)0x200ull) /**<  APN type for emergency
 APN type 0x400 Reserved  */
#define WDS_APN_TYPE_MASK_MCX_V01 ((wds_apn_type_mask_v01)0x800ull) /**<  APN type for mission critical service  */
/* Conditional compilation tags for message removal */

/*Extern Definition of Type Table Object*/
/*THIS IS AN INTERNAL OBJECT AND SHOULD ONLY*/
/*BE ACCESSED BY AUTOGENERATED FILES*/
extern const qmi_idl_type_table_object wireless_data_service_common_qmi_idl_type_table_object_v01;


#ifdef __cplusplus
}
#endif
#endif

