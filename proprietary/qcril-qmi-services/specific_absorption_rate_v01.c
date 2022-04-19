/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        S P E C I F I C _ A B S O R P T I O N _ R A T E _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the sar service Data structures.

  Copyright (c) 2019  Qualcomm Technologies Incorporated.
  All rights reserved.
  Confidential and Proprietary -  Qualcomm Technologies Incorporated.


  $Header: //commercial/MPSS.HI.1.0.r30/Main/modem_proc/qmimsgs/sar/src/specific_absorption_rate_v01.c#1 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.14.7 
   It was generated on: Thu Apr  4 2019 (Spin 0)
   From IDL File: specific_absorption_rate_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "specific_absorption_rate_v01.h"
#include "common_v01.h"


/*Type Definitions*/
static const uint8_t sar_mpe_beam_mtpl_module_info_data_v01[] = {
  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sar_mpe_beam_mtpl_module_info_v01, module_id),

  QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sar_mpe_beam_mtpl_module_info_v01, enable_detection),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sar_mpe_beam_mtpl_module_info_v01, pmax_dbm_q7),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sar_mpe_beam_mtpl_module_info_v01, pmax_dbm_q7_mimo),

  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sar_mpe_beam_mtpl_module_info_v01, band),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
static const uint8_t sar_rf_set_state_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sar_rf_set_state_req_msg_v01, sar_rf_state),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sar_rf_set_state_req_msg_v01, compatibility_key) - QMI_IDL_OFFSET8(sar_rf_set_state_req_msg_v01, compatibility_key_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sar_rf_set_state_req_msg_v01, compatibility_key)
};

static const uint8_t sar_rf_set_state_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sar_rf_set_state_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/*
 * sar_rf_get_state_req_msg is empty
 * static const uint8_t sar_rf_get_state_req_msg_data_v01[] = {
 * };
 */

static const uint8_t sar_rf_get_state_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sar_rf_get_state_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sar_rf_get_state_resp_msg_v01, sar_rf_state) - QMI_IDL_OFFSET8(sar_rf_get_state_resp_msg_v01, sar_rf_state_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sar_rf_get_state_resp_msg_v01, sar_rf_state)
};

/*
 * sar_rf_get_compatibility_key_req_msg is empty
 * static const uint8_t sar_rf_get_compatibility_key_req_msg_data_v01[] = {
 * };
 */

static const uint8_t sar_rf_get_compatibility_key_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sar_rf_get_compatibility_key_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sar_rf_get_compatibility_key_resp_msg_v01, compatibility_key) - QMI_IDL_OFFSET8(sar_rf_get_compatibility_key_resp_msg_v01, compatibility_key_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sar_rf_get_compatibility_key_resp_msg_v01, compatibility_key)
};

static const uint8_t sar_mmw_module_disable_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sar_mmw_module_disable_req_msg_v01, module_disable_bitmask)
};

static const uint8_t sar_mmw_module_disable_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sar_mmw_module_disable_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t sar_wlan_pwr_limit_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sar_wlan_pwr_limit_ind_msg_v01, seq_num),

  0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sar_wlan_pwr_limit_ind_msg_v01, pwr_backoff_db_10),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sar_wlan_pwr_limit_ind_msg_v01, pwr_limit_dbm_10) - QMI_IDL_OFFSET8(sar_wlan_pwr_limit_ind_msg_v01, pwr_limit_dbm_10_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sar_wlan_pwr_limit_ind_msg_v01, pwr_limit_dbm_10)
};

static const uint8_t sar_wlan_pwr_limit_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sar_wlan_pwr_limit_req_msg_v01, seq_num)
};

static const uint8_t sar_wlan_pwr_limit_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sar_wlan_pwr_limit_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  0x03,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sar_wlan_pwr_limit_resp_msg_v01, seq_num),

  0x04,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sar_wlan_pwr_limit_resp_msg_v01, pwr_backoff_db_10),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sar_wlan_pwr_limit_resp_msg_v01, pwr_limit_dbm_10) - QMI_IDL_OFFSET8(sar_wlan_pwr_limit_resp_msg_v01, pwr_limit_dbm_10_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sar_wlan_pwr_limit_resp_msg_v01, pwr_limit_dbm_10)
};

static const uint8_t sar_indication_register_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sar_indication_register_req_msg_v01, report_wlan_pwr_limit) - QMI_IDL_OFFSET8(sar_indication_register_req_msg_v01, report_wlan_pwr_limit_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(sar_indication_register_req_msg_v01, report_wlan_pwr_limit)
};

static const uint8_t sar_indication_register_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sar_indication_register_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t sar_wlan_pwr_limit_ack_req_msg_data_v01[] = {
  0x01,
   QMI_IDL_2_BYTE_ENUM,
  QMI_IDL_OFFSET8(sar_wlan_pwr_limit_ack_req_msg_v01, result),

  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(sar_wlan_pwr_limit_ack_req_msg_v01, seq_num)
};

static const uint8_t sar_wlan_pwr_limit_ack_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sar_wlan_pwr_limit_ack_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t sar_mpe_beam_mtpl_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(sar_mpe_beam_mtpl_req_msg_v01, module_info) - QMI_IDL_OFFSET8(sar_mpe_beam_mtpl_req_msg_v01, module_info_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sar_mpe_beam_mtpl_req_msg_v01, module_info),
  SAR_MPE_MAX_MODULES_V01,
  QMI_IDL_OFFSET8(sar_mpe_beam_mtpl_req_msg_v01, module_info) - QMI_IDL_OFFSET8(sar_mpe_beam_mtpl_req_msg_v01, module_info_len),
  QMI_IDL_TYPE88(0, 0)
};

static const uint8_t sar_mpe_beam_mtpl_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(sar_mpe_beam_mtpl_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/* Type Table */
static const qmi_idl_type_table_entry  sar_type_table_v01[] = {
  {sizeof(sar_mpe_beam_mtpl_module_info_v01), sar_mpe_beam_mtpl_module_info_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry sar_message_table_v01[] = {
  {sizeof(sar_rf_set_state_req_msg_v01), sar_rf_set_state_req_msg_data_v01},
  {sizeof(sar_rf_set_state_resp_msg_v01), sar_rf_set_state_resp_msg_data_v01},
  {sizeof(sar_rf_get_state_req_msg_v01), 0},
  {sizeof(sar_rf_get_state_resp_msg_v01), sar_rf_get_state_resp_msg_data_v01},
  {sizeof(sar_rf_get_compatibility_key_req_msg_v01), 0},
  {sizeof(sar_rf_get_compatibility_key_resp_msg_v01), sar_rf_get_compatibility_key_resp_msg_data_v01},
  {sizeof(sar_mmw_module_disable_req_msg_v01), sar_mmw_module_disable_req_msg_data_v01},
  {sizeof(sar_mmw_module_disable_resp_msg_v01), sar_mmw_module_disable_resp_msg_data_v01},
  {sizeof(sar_wlan_pwr_limit_ind_msg_v01), sar_wlan_pwr_limit_ind_msg_data_v01},
  {sizeof(sar_wlan_pwr_limit_req_msg_v01), sar_wlan_pwr_limit_req_msg_data_v01},
  {sizeof(sar_wlan_pwr_limit_resp_msg_v01), sar_wlan_pwr_limit_resp_msg_data_v01},
  {sizeof(sar_indication_register_req_msg_v01), sar_indication_register_req_msg_data_v01},
  {sizeof(sar_indication_register_resp_msg_v01), sar_indication_register_resp_msg_data_v01},
  {sizeof(sar_wlan_pwr_limit_ack_req_msg_v01), sar_wlan_pwr_limit_ack_req_msg_data_v01},
  {sizeof(sar_wlan_pwr_limit_ack_resp_msg_v01), sar_wlan_pwr_limit_ack_resp_msg_data_v01},
  {sizeof(sar_mpe_beam_mtpl_req_msg_v01), sar_mpe_beam_mtpl_req_msg_data_v01},
  {sizeof(sar_mpe_beam_mtpl_resp_msg_v01), sar_mpe_beam_mtpl_resp_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object sar_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *sar_qmi_idl_type_table_object_referenced_tables_v01[] =
{&sar_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object sar_qmi_idl_type_table_object_v01 = {
  sizeof(sar_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(sar_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  sar_type_table_v01,
  sar_message_table_v01,
  sar_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry sar_service_command_messages_v01[] = {
  {QMI_SAR_RF_SET_STATE_REQ_MSG_V01, QMI_IDL_TYPE16(0, 0), 14},
  {QMI_SAR_RF_GET_STATE_REQ_MSG_V01, QMI_IDL_TYPE16(0, 2), 0},
  {QMI_SAR_GET_SUPPORTED_MSGS_REQ_V01, QMI_IDL_TYPE16(1, 0), 0},
  {QMI_SAR_GET_SUPPORTED_FIELDS_REQ_V01, QMI_IDL_TYPE16(1, 2), 5},
  {QMI_SAR_GET_COMPATIBILITY_KEY_REQ_MSG_V01, QMI_IDL_TYPE16(0, 4), 0},
  {QMI_SAR_MMW_MODULE_DISABLE_REQ_V01, QMI_IDL_TYPE16(0, 6), 7},
  {QMI_SAR_WLAN_PWR_LIMIT_REQ_V01, QMI_IDL_TYPE16(0, 9), 7},
  {QMI_SAR_INDICATION_REGISTER_REQ_V01, QMI_IDL_TYPE16(0, 11), 4},
  {QMI_SAR_WLAN_PWR_LIMIT_ACK_REQ_V01, QMI_IDL_TYPE16(0, 13), 12},
  {QMI_SAR_MPE_BEAM_MTPL_REQ_V01, QMI_IDL_TYPE16(0, 15), 1092}
};

static const qmi_idl_service_message_table_entry sar_service_response_messages_v01[] = {
  {QMI_SAR_RF_SET_STATE_RESP_MSG_V01, QMI_IDL_TYPE16(0, 1), 7},
  {QMI_SAR_RF_GET_STATE_RESP_MSG_V01, QMI_IDL_TYPE16(0, 3), 14},
  {QMI_SAR_GET_SUPPORTED_MSGS_RESP_V01, QMI_IDL_TYPE16(1, 1), 8204},
  {QMI_SAR_GET_SUPPORTED_FIELDS_RESP_V01, QMI_IDL_TYPE16(1, 3), 115},
  {QMI_SAR_GET_COMPATIBILITY_KEY_RESP_MSG_V01, QMI_IDL_TYPE16(0, 5), 14},
  {QMI_SAR_MMW_MODULE_DISABLE_RESP_V01, QMI_IDL_TYPE16(0, 7), 7},
  {QMI_SAR_WLAN_PWR_LIMIT_RESP_V01, QMI_IDL_TYPE16(0, 10), 28},
  {QMI_SAR_INDICATION_REGISTER_RESP_V01, QMI_IDL_TYPE16(0, 12), 7},
  {QMI_SAR_WLAN_PWR_LIMIT_ACK_RESP_V01, QMI_IDL_TYPE16(0, 14), 7},
  {QMI_SAR_MPE_BEAM_MTPL_RESP_V01, QMI_IDL_TYPE16(0, 16), 7}
};

static const qmi_idl_service_message_table_entry sar_service_indication_messages_v01[] = {
  {QMI_SAR_WLAN_PWR_LIMIT_IND_V01, QMI_IDL_TYPE16(0, 8), 21}
};

/*Service Object*/
struct qmi_idl_service_object sar_qmi_idl_service_object_v01 = {
  0x06,
  0x01,
  0x11,
  8204,
  { sizeof(sar_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(sar_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(sar_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { sar_service_command_messages_v01, sar_service_response_messages_v01, sar_service_indication_messages_v01},
  &sar_qmi_idl_type_table_object_v01,
  0x09,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type sar_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( SAR_V01_IDL_MAJOR_VERS != idl_maj_version || SAR_V01_IDL_MINOR_VERS != idl_min_version
       || SAR_V01_IDL_TOOL_VERS != library_version)
  {
    return NULL;
  }
  return (qmi_idl_service_object_type)&sar_qmi_idl_service_object_v01;
}

