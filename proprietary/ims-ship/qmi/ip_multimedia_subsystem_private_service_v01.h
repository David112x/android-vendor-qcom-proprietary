#ifndef IMSPRIVATE_SERVICE_01_H
#define IMSPRIVATE_SERVICE_01_H
/**
  @file ip_multimedia_subsystem_private_service_v01.h

  @brief This is the public header file which defines the imsprivate service Data structures.

  This header file defines the types and structures that were defined in
  imsprivate. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2012-2018 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.



  $Header: //depot/SBA/users/lteswint/ebf/mpl/BF-ENG-pixel_headers-MPSS.AT.4.0.c2-00417-SDM845_GEN_TEST-1/modem_proc/qmimsgs/imsprivate/api/ip_multimedia_subsystem_private_service_v01.h#3 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.14.7
   It was generated on: Tue Mar 20 2018 (Spin 0)
   From IDL File: ip_multimedia_subsystem_private_service_v01.idl */

/** @defgroup imsprivate_qmi_consts Constant values defined in the IDL */
/** @defgroup imsprivate_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup imsprivate_qmi_enums Enumerated types used in QMI messages */
/** @defgroup imsprivate_qmi_messages Structures sent as QMI messages */
/** @defgroup imsprivate_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup imsprivate_qmi_accessor Accessor for QMI service object */
/** @defgroup imsprivate_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"
#include "ip_multimedia_subsystem_ims_common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup imsprivate_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define IMSPRIVATE_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define IMSPRIVATE_V01_IDL_MINOR_VERS 0x01
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define IMSPRIVATE_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define IMSPRIVATE_V01_MAX_MESSAGE_ID 0x003F
/**
    @}
  */


/** @addtogroup imsprivate_qmi_consts
    @{
  */

/**

 Enumeration for all the common IMS error codes */
#define IMS_PRIVATE_SERVICE_MAX_MT_INVITE_HEADERS_V01 15
#define IMS_PRIVATE_SERVICE_HEADER_STR_LEN_V01 1024
#define IMS_PRIVATE_SERVICE_MAX_ICCID_LEN_V01 21
/**
    @}
  */

/** @addtogroup imsprivate_qmi_aggregates
    @{
  */
/**  Structures required by IMS service specific messages.

 */
typedef struct {

  char header[IMS_PRIVATE_SERVICE_HEADER_STR_LEN_V01 + 1];
  /**<   HEADER. */

  char value[IMS_PRIVATE_SERVICE_HEADER_STR_LEN_V01 + 1];
  /**<   VALUE. */
}ims_private_service_header_value_v01;  /* Type */
/**
    @}
  */

/** @addtogroup imsprivate_qmi_messages
    @{
  */
/** Request Message; Sets the subscription state for various service
             indications for the requesting control points. */
typedef struct {

  /* Optional */
  /*  MT INVITE INDICATION (sent via QMI_IMS_PRIVATE_SERVICE_MT_INVITE_IND) */
  uint8_t mt_invite_valid;  /**< Must be set to true if mt_invite is being passed */
  uint8_t mt_invite;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */
}ims_private_service_subscribe_for_indications_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsprivate_qmi_messages
    @{
  */
/** Response Message; Sets the subscription state for various service
             indications for the requesting control points. */
typedef struct {

  /* Mandatory */
  /*  Result Code     */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members: \n
      - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
      - qmi_error_type  -- Error code. Possible error code values are described
                          in the error codes section of each message definition.
    */
}ims_private_service_subscribe_for_indications_rsp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup imsprivate_qmi_messages
    @{
  */
/** Indication Message; Provides headers for incoming SIP INVITE    */
typedef struct {

  /* Mandatory */
  /*  Subscription id */
  ims_subscription_type_enum_v01 subscription_type;
  /**<   Subscription id of IMS stack on which INVITE is received.
	*/

  /* Optional */
  /*  ICCID  */
  uint8_t iccid_valid;  /**< Must be set to true if iccid is being passed */
  char iccid[IMS_PRIVATE_SERVICE_MAX_ICCID_LEN_V01 + 1];

  /* Optional */
  /*  List of Headers and Values in incoming SIP INVITE (maybe a subset)  */
  uint8_t header_value_list_valid;  /**< Must be set to true if header_value_list is being passed */
  uint32_t header_value_list_len;  /**< Must be set to # of elements in header_value_list */
  ims_private_service_header_value_v01 header_value_list[IMS_PRIVATE_SERVICE_MAX_MT_INVITE_HEADERS_V01];
}ims_private_service_mt_invite_ind_msg_v01;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */
//#define REMOVE_QMI_IMS_PRIVATE_SERVICE_MT_INVITE_IND_V01
//#define REMOVE_QMI_IMS_PRIVATE_SERVICE_SUBSCRIBE_FOR_INDICATIONS_REQ_V01

/*Service Message Definition*/
/** @addtogroup imsprivate_qmi_msg_ids
    @{
  */
#define QMI_IMS_PRIVATE_SERVICE_SUBSCRIBE_FOR_INDICATIONS_REQ_V01 0x003E
#define QMI_IMS_PRIVATE_SERVICE_SUBSCRIBE_FOR_INDICATIONS_RSP_V01 0x003E
#define QMI_IMS_PRIVATE_SERVICE_MT_INVITE_IND_V01 0x003F
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro imsprivate_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type imsprivate_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );

/** This macro should be used to get the service object */
#define imsprivate_get_service_object_v01( ) \
          imsprivate_get_service_object_internal_v01( \
            IMSPRIVATE_V01_IDL_MAJOR_VERS, IMSPRIVATE_V01_IDL_MINOR_VERS, \
            IMSPRIVATE_V01_IDL_TOOL_VERS )
/**
    @}
  */


#ifdef __cplusplus
}
#endif
#endif
