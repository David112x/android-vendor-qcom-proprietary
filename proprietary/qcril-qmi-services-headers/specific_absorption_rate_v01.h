#ifndef SAR_SERVICE_01_H
#define SAR_SERVICE_01_H
/**
  @file specific_absorption_rate_v01.h

  @brief This is the public header file which defines the sar service Data structures.

  This header file defines the types and structures that were defined in
  sar. It contains the constant values defined, enums, structures,
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
  Copyright (c) 2019  Qualcomm Technologies Incorporated.
  All rights reserved.
  Confidential and Proprietary -  Qualcomm Technologies Incorporated.


  $Header: //commercial/MPSS.HI.1.0.r30/Main/modem_proc/qmimsgs/sar/api/specific_absorption_rate_v01.h#1 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.14.7 
   It was generated on: Thu Apr  4 2019 (Spin 0)
   From IDL File: specific_absorption_rate_v01.idl */

/** @defgroup sar_qmi_consts Constant values defined in the IDL */
/** @defgroup sar_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup sar_qmi_enums Enumerated types used in QMI messages */
/** @defgroup sar_qmi_messages Structures sent as QMI messages */
/** @defgroup sar_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup sar_qmi_accessor Accessor for QMI service object */
/** @defgroup sar_qmi_version Constant values for versioning information */

#include <stdint.h>
#include "qmi_idl_lib.h"
#include "common_v01.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup sar_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define SAR_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define SAR_V01_IDL_MINOR_VERS 0x09
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define SAR_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define SAR_V01_MAX_MESSAGE_ID 0x0026
/**
    @}
  */


/** @addtogroup sar_qmi_consts
    @{
  */
#define SAR_MPE_MAX_MODULES_V01 64
/**
    @}
  */

/** @addtogroup sar_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t module_id;
  /**<   ID of the module for which the power limit is being reported.
       The valid range for the module ID is 0 to 7 (inclusive) */

  uint8_t enable_detection;
  /**<   Flag denoting whether detection is enabled for this module. */

  int32_t pmax_dbm_q7;
  /**<   Maximum power applied during single port transmissions in Q7 format. 
       The default PMAX is -128dBm for single port transmissions if PMAX is 
       not configured using QMI command */

  int32_t pmax_dbm_q7_mimo;
  /**<   Maximum power applied during multiport transmissions in Q7 format.
       The default PMAX is -128dBm for multiport transmissions if PMAX is 
       not configured using QMI command */

  int32_t band;
  /**<   mmW band for which this power limit is being reported.
       The client shall use sys_band_class_e_type in sys.h to populate
       this field. */
}sar_mpe_beam_mtpl_module_info_v01;  /* Type */
/**
    @}
  */

/** @addtogroup sar_qmi_enums
    @{
  */
typedef enum {
  QMI_SAR_RF_STATE_ENUM_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QMI_SAR_RF_STATE_DEFAULT_V01 = 0, /**<  Default RF state.  */
  QMI_SAR_RF_STATE_1_V01 = 1, /**<  RF state 1.  \n  */
  QMI_SAR_RF_STATE_2_V01 = 2, /**<  RF state 2.  \n  */
  QMI_SAR_RF_STATE_3_V01 = 3, /**<  RF state 3.  \n  */
  QMI_SAR_RF_STATE_4_V01 = 4, /**<  RF state 4.  \n  */
  QMI_SAR_RF_STATE_5_V01 = 5, /**<  RF state 5.  \n  */
  QMI_SAR_RF_STATE_6_V01 = 6, /**<  RF state 6.  \n  */
  QMI_SAR_RF_STATE_7_V01 = 7, /**<  RF state 7.  \n  */
  QMI_SAR_RF_STATE_8_V01 = 8, /**<  RF state 8.  \n  */
  QMI_SAR_RF_STATE_9_V01 = 9, /**<  RF state 9.  \n  */
  QMI_SAR_RF_STATE_10_V01 = 10, /**<  RF state 10. \n  */
  QMI_SAR_RF_STATE_11_V01 = 11, /**<  RF state 11. \n  */
  QMI_SAR_RF_STATE_12_V01 = 12, /**<  RF state 12. \n  */
  QMI_SAR_RF_STATE_13_V01 = 13, /**<  RF state 13. \n  */
  QMI_SAR_RF_STATE_14_V01 = 14, /**<  RF state 14. \n  */
  QMI_SAR_RF_STATE_15_V01 = 15, /**<  RF state 15. \n  */
  QMI_SAR_RF_STATE_16_V01 = 16, /**<  RF state 16. \n  */
  QMI_SAR_RF_STATE_17_V01 = 17, /**<  RF state 17. \n  */
  QMI_SAR_RF_STATE_18_V01 = 18, /**<  RF state 18. \n  */
  QMI_SAR_RF_STATE_19_V01 = 19, /**<  RF state 19. \n  */
  QMI_SAR_RF_STATE_20_V01 = 20, /**<  RF state 20.     */
  QMI_SAR_RF_STATE_ENUM_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}qmi_sar_rf_state_enum_type_v01;
/**
    @}
  */

/** @addtogroup sar_qmi_messages
    @{
  */
/** Request Message; Sets the specified SAR RF state. */
typedef struct {

  /* Mandatory */
  /*  SAR RF State */
  qmi_sar_rf_state_enum_type_v01 sar_rf_state;
  /**<  
 SAR RF state must be specified: \n
      - QMI_SAR_RF_STATE_DEFAULT (0) --  Default RF state. 
      - QMI_SAR_RF_STATE_1 (1) --  RF state 1.  \n 
      - QMI_SAR_RF_STATE_2 (2) --  RF state 2.  \n 
      - QMI_SAR_RF_STATE_3 (3) --  RF state 3.  \n 
      - QMI_SAR_RF_STATE_4 (4) --  RF state 4.  \n 
      - QMI_SAR_RF_STATE_5 (5) --  RF state 5.  \n 
      - QMI_SAR_RF_STATE_6 (6) --  RF state 6.  \n 
      - QMI_SAR_RF_STATE_7 (7) --  RF state 7.  \n 
      - QMI_SAR_RF_STATE_8 (8) --  RF state 8.  \n 
      - QMI_SAR_RF_STATE_9 (9) --  RF state 9.  \n 
      - QMI_SAR_RF_STATE_10 (10) --  RF state 10. \n 
      - QMI_SAR_RF_STATE_11 (11) --  RF state 11. \n 
      - QMI_SAR_RF_STATE_12 (12) --  RF state 12. \n 
      - QMI_SAR_RF_STATE_13 (13) --  RF state 13. \n 
      - QMI_SAR_RF_STATE_14 (14) --  RF state 14. \n 
      - QMI_SAR_RF_STATE_15 (15) --  RF state 15. \n 
      - QMI_SAR_RF_STATE_16 (16) --  RF state 16. \n 
      - QMI_SAR_RF_STATE_17 (17) --  RF state 17. \n 
      - QMI_SAR_RF_STATE_18 (18) --  RF state 18. \n 
      - QMI_SAR_RF_STATE_19 (19) --  RF state 19. \n 
      - QMI_SAR_RF_STATE_20 (20) --  RF state 20.    
 */

  /* Optional */
  /*  Compatibility Key */
  uint8_t compatibility_key_valid;  /**< Must be set to true if compatibility_key is being passed */
  uint32_t compatibility_key;
  /**<  
     Compatibility key to be verified. 
  */
}sar_rf_set_state_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sar_qmi_messages
    @{
  */
/** Response Message; Sets the specified SAR RF state. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
     - qmi_error_type  -- Error code. Possible error code values are
                          described in the error codes section of each 
                          message definition.
    */
}sar_rf_set_state_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sar_qmi_messages
    @{
  */
/** Request Message; Gets the specified SAR RF state. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}sar_rf_get_state_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup sar_qmi_messages
    @{
  */
/** Response Message; Gets the specified SAR RF state. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  Vendor Specific Configuration */
  uint8_t sar_rf_state_valid;  /**< Must be set to true if sar_rf_state is being passed */
  qmi_sar_rf_state_enum_type_v01 sar_rf_state;
  /**<  
 SAR RF state must be specified:
      - QMI_SAR_RF_STATE_DEFAULT (0) --  Default RF state. 
      - QMI_SAR_RF_STATE_1 (1) --  RF state 1.  \n 
      - QMI_SAR_RF_STATE_2 (2) --  RF state 2.  \n 
      - QMI_SAR_RF_STATE_3 (3) --  RF state 3.  \n 
      - QMI_SAR_RF_STATE_4 (4) --  RF state 4.  \n 
      - QMI_SAR_RF_STATE_5 (5) --  RF state 5.  \n 
      - QMI_SAR_RF_STATE_6 (6) --  RF state 6.  \n 
      - QMI_SAR_RF_STATE_7 (7) --  RF state 7.  \n 
      - QMI_SAR_RF_STATE_8 (8) --  RF state 8.  \n 
      - QMI_SAR_RF_STATE_9 (9) --  RF state 9.  \n 
      - QMI_SAR_RF_STATE_10 (10) --  RF state 10. \n 
      - QMI_SAR_RF_STATE_11 (11) --  RF state 11. \n 
      - QMI_SAR_RF_STATE_12 (12) --  RF state 12. \n 
      - QMI_SAR_RF_STATE_13 (13) --  RF state 13. \n 
      - QMI_SAR_RF_STATE_14 (14) --  RF state 14. \n 
      - QMI_SAR_RF_STATE_15 (15) --  RF state 15. \n 
      - QMI_SAR_RF_STATE_16 (16) --  RF state 16. \n 
      - QMI_SAR_RF_STATE_17 (17) --  RF state 17. \n 
      - QMI_SAR_RF_STATE_18 (18) --  RF state 18. \n 
      - QMI_SAR_RF_STATE_19 (19) --  RF state 19. \n 
      - QMI_SAR_RF_STATE_20 (20) --  RF state 20.    
 */
}sar_rf_get_state_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sar_qmi_messages
    @{
  */
/** Request Message; Gets the compatibility key information. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}sar_rf_get_compatibility_key_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup sar_qmi_messages
    @{
  */
/** Response Message; Gets the compatibility key information. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. */

  /* Optional */
  /*  Vendor Specific Configuration */
  uint8_t compatibility_key_valid;  /**< Must be set to true if compatibility_key is being passed */
  uint32_t compatibility_key;
  /**<  
     Compatibility key value.
  */
}sar_rf_get_compatibility_key_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sar_qmi_messages
    @{
  */
/** Request Message; Enables/disables transmission from mmWave modules. */
typedef struct {

  /* Mandatory */
  /*  SAR mmWave Module Disable */
  uint32_t module_disable_bitmask;
  /**<  
     Bitmask of mmWave modules to disable. Bits turned on denote
     modules whose emission is disabled.
     
     module_disable_bitmask = 0x3 means modules 0 and 1 are disabled;
     all others are enabled.
  */
}sar_mmw_module_disable_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sar_qmi_messages
    @{
  */
/** Response Message; Enables/disables transmission from mmWave modules. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are 
                          described in the error codes section of each 
                          message definition.
    */
}sar_mmw_module_disable_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sar_qmi_messages
    @{
  */
/** Indication Message; QMI service-initiated transaction that signals power limit
               and backoff to the QMI client, which routes it to the WLAN
               for exposure compliance. */
typedef struct {

  /* Mandatory */
  /*  Sequence Number of the Transaction */
  uint32_t seq_num;
  /**<  
    Sequence number used for acknowledgement in the transaction.
    The QMI client mirrors the value in the 
    QMI_SAR_WLAN_PWR_LIMIT_REQ message in response to this
    indication.
  */

  /* Mandatory */
  /*  WLAN Power Backoff in 10*dB */
  int32_t pwr_backoff_db_10;
  /**<  
     Power "back-off" applied to the maximum transmit power for each
     WLAN transmission mode to ensure SAR compliance.
     
     If the maximum transmit power limit is 14 dBm, a backoff value of 30
     results in a limit reduction to 11 dBm.
  */

  /* Optional */
  /*  Power Limit for WLAN Tx in 10*dBm */
  uint8_t pwr_limit_dbm_10_valid;  /**< Must be set to true if pwr_limit_dbm_10 is being passed */
  int32_t pwr_limit_dbm_10;
  /**<  
     Absolute power limit applied to the maximum transmit power for each of
     WLAN transmission mode to ensure SAR compliance.
     
     A value of 100 indicates a limit of \n
     10 dBm.
  */
}sar_wlan_pwr_limit_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sar_qmi_messages
    @{
  */
/** Request Message; QMI client-initiated transaction to query power limit and 
               backoff for WLAN for exposure compliance. */
typedef struct {

  /* Mandatory */
  /*  Sequence Number of the Transaction */
  uint32_t seq_num;
  /**<  
    Sequence number used for acknowledgement in this transaction.
    If this message was sent by the client in response to 
    QMI_SAR_WLAN_PWR_LIMIT_IND, the QMI client mirrors the
    value received in the indication.

    Otherwise the QMI client generates a fresh value, seq_num, for each new 
    asynchronous request message.
  */
}sar_wlan_pwr_limit_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sar_qmi_messages
    @{
  */
/** Response Message; QMI client-initiated transaction to query power limit and 
               backoff for WLAN for exposure compliance. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are
                          described in the error codes section of each 
                          message definition.
  */

  /* Mandatory */
  /*  Sequence Number of the Transaction */
  uint32_t seq_num;
  /**<  
    Sequence number used for acknowledgement in the transaction.
    The QMI service mirrors the value received in the 
    QMI_SAR_WLAN_PWR_LIMIT_REQ message.
  */

  /* Mandatory */
  /*  WLAN Power Backoff in 10*dB */
  int32_t pwr_backoff_db_10;
  /**<  
     Power "back-off" applied to the maximum transmit power for each
     WLAN transmission mode to ensure SAR compliance.
     
     If the maxiumum transmit power limit is 14 dBm, a backoff value of 30
     results in a limit reduction to 11 dBm.
  */

  /* Optional */
  /*  Power Limit for WLAN Tx in 10*dBm */
  uint8_t pwr_limit_dbm_10_valid;  /**< Must be set to true if pwr_limit_dbm_10 is being passed */
  int32_t pwr_limit_dbm_10;
  /**<  
     Absolute power limit applied to the maximum transmit power for each
     WLAN transmission mode to ensure SAR compliance.
     
     A value of 100 indicates a limit of 10 dBm.
  */
}sar_wlan_pwr_limit_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sar_qmi_messages
    @{
  */
/** Request Message; Sets the registration state for different
              SAR indications for the requesting control point.
              \label{idl:indicationRegister} */
typedef struct {

  /* Optional */
  /*  Control for WLAN Power Limit Indication */
  uint8_t report_wlan_pwr_limit_valid;  /**< Must be set to true if report_wlan_pwr_limit is being passed */
  uint8_t report_wlan_pwr_limit;
  /**<   Values: \n
       - 0x00 -- Disable \n
       - 0x01 -- Enable
  */
}sar_indication_register_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sar_qmi_messages
    @{
  */
/** Response Message; Sets the registration state for different
              SAR indications for the requesting control point.
              \label{idl:indicationRegister} */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are described in
                          the error codes section of each message definition.
  */
}sar_indication_register_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sar_qmi_messages
    @{
  */
/** Request Message; QMI client-sent acknowledgement that it received WLAN power 
               limit information from the service. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_result_type_v01 result;
  /**<   Standard result type. Contains the following data members: \n
     -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE
  */

  /* Mandatory */
  /*  Sequence Number of the Transaction */
  uint32_t seq_num;
  /**<  
    Sequence number used for acknowledgement in this transaction.
    If this message was sent by the client in response to 
    QMI_SAR_WLAN_PWR_LIMIT_IND, the QMI client mirrors the
    value received in the indication.

    Otherwise the QMI client generates a fresh value, seq_num, for each new 
    asynchronous request message.
  */
}sar_wlan_pwr_limit_ack_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sar_qmi_messages
    @{
  */
/** Response Message; QMI client-sent acknowledgement that it received WLAN power 
               limit information from the service. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are
                          described in the error codes section of each 
                          message definition.
  */
}sar_wlan_pwr_limit_ack_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sar_qmi_messages
    @{
  */
/** Request Message; QMI client-initiated transaction that signals MPE power limits
               to the QMI service, which routes to NR5G ML1 for exposure
               compliance.

               5G stack must be active before this command is sent. For a 
               given module, the configuration provided is absolute and 
               replaces any previous configuration in its entirety.
               The MTPL sent in this command is applied within 250 ms
               and another request should not be sent within 250 ms. */
typedef struct {

  /* Optional */
  /*  Module Specific Information */
  uint8_t module_info_valid;  /**< Must be set to true if module_info is being passed */
  uint32_t module_info_len;  /**< Must be set to # of elements in module_info */
  sar_mpe_beam_mtpl_module_info_v01 module_info[SAR_MPE_MAX_MODULES_V01];
  /**<  
    \n  MTPL information for each module that is reporting MPE limits.
        A maximum of 64 modules can be configured using this interface.
  */
}sar_mpe_beam_mtpl_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup sar_qmi_messages
    @{
  */
/** Response Message; QMI client-initiated transaction that signals MPE power limits
               to the QMI service, which routes to NR5G ML1 for exposure
               compliance.

               5G stack must be active before this command is sent. For a 
               given module, the configuration provided is absolute and 
               replaces any previous configuration in its entirety.
               The MTPL sent in this command is applied within 250 ms
               and another request should not be sent within 250 ms. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  qmi_response_type_v01 resp;
  /**<   Standard response type. Contains the following data members:
     - qmi_result_type -- QMI_RESULT_SUCCESS or QMI_RESULT_FAILURE \n
     - qmi_error_type  -- Error code. Possible error code values are
                          described in the error codes section of each 
                          message definition.
  */
}sar_mpe_beam_mtpl_resp_msg_v01;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */ 
//#define REMOVE_QMI_SAR_GET_COMPATIBILITY_KEY_V01 
//#define REMOVE_QMI_SAR_GET_SUPPORTED_FIELDS_V01 
//#define REMOVE_QMI_SAR_GET_SUPPORTED_MSGS_V01 
//#define REMOVE_QMI_SAR_INDICATION_REGISTER_V01 
//#define REMOVE_QMI_SAR_MMW_MODULE_DISABLE_V01 
//#define REMOVE_QMI_SAR_MPE_BEAM_MTPL_V01 
//#define REMOVE_QMI_SAR_RF_GET_STATE_V01 
//#define REMOVE_QMI_SAR_RF_SET_STATE_V01 
//#define REMOVE_QMI_SAR_WLAN_PWR_LIMIT_V01 
//#define REMOVE_QMI_SAR_WLAN_PWR_LIMIT_ACK_V01 
//#define REMOVE_QMI_SAR_WLAN_PWR_LIMIT_IND_V01 

/*Service Message Definition*/
/** @addtogroup sar_qmi_msg_ids
    @{
  */
#define QMI_SAR_RF_SET_STATE_REQ_MSG_V01 0x0001
#define QMI_SAR_RF_SET_STATE_RESP_MSG_V01 0x0001
#define QMI_SAR_RF_GET_STATE_REQ_MSG_V01 0x0002
#define QMI_SAR_RF_GET_STATE_RESP_MSG_V01 0x0002
#define QMI_SAR_GET_SUPPORTED_MSGS_REQ_V01 0x001E
#define QMI_SAR_GET_SUPPORTED_MSGS_RESP_V01 0x001E
#define QMI_SAR_GET_SUPPORTED_FIELDS_REQ_V01 0x001F
#define QMI_SAR_GET_SUPPORTED_FIELDS_RESP_V01 0x001F
#define QMI_SAR_GET_COMPATIBILITY_KEY_REQ_MSG_V01 0x0020
#define QMI_SAR_GET_COMPATIBILITY_KEY_RESP_MSG_V01 0x0020
#define QMI_SAR_MMW_MODULE_DISABLE_REQ_V01 0x0021
#define QMI_SAR_MMW_MODULE_DISABLE_RESP_V01 0x0021
#define QMI_SAR_WLAN_PWR_LIMIT_IND_V01 0x0022
#define QMI_SAR_WLAN_PWR_LIMIT_REQ_V01 0x0023
#define QMI_SAR_WLAN_PWR_LIMIT_RESP_V01 0x0023
#define QMI_SAR_INDICATION_REGISTER_REQ_V01 0x0024
#define QMI_SAR_INDICATION_REGISTER_RESP_V01 0x0024
#define QMI_SAR_WLAN_PWR_LIMIT_ACK_REQ_V01 0x0025
#define QMI_SAR_WLAN_PWR_LIMIT_ACK_RESP_V01 0x0025
#define QMI_SAR_MPE_BEAM_MTPL_REQ_V01 0x0026
#define QMI_SAR_MPE_BEAM_MTPL_RESP_V01 0x0026
/**
    @}
  */

/* Service Object Accessor */
/** @addtogroup wms_qmi_accessor
    @{
  */
/** This function is used internally by the autogenerated code.  Clients should use the
   macro sar_get_service_object_v01( ) that takes in no arguments. */
qmi_idl_service_object_type sar_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version );

/** This macro should be used to get the service object */
#define sar_get_service_object_v01( ) \
          sar_get_service_object_internal_v01( \
            SAR_V01_IDL_MAJOR_VERS, SAR_V01_IDL_MINOR_VERS, \
            SAR_V01_IDL_TOOL_VERS )
/**
    @}
  */


#ifdef __cplusplus
}
#endif
#endif

