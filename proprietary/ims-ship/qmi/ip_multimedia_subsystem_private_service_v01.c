/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        I P _ M U L T I M E D I A _ S U B S Y S T E M _ P R I V A T E _ S E R V I C E _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the imsprivate service Data structures.

  Copyright (c) 2012-2018 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.



  $Header: //depot/SBA/users/lteswint/ebf/mpl/BF-ENG-pixel_headers-MPSS.AT.4.0.c2-00417-SDM845_GEN_TEST-1/modem_proc/qmimsgs/imsprivate/src/ip_multimedia_subsystem_private_service_v01.c#3 $
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.14.7
   It was generated on: Tue Mar 20 2018 (Spin 0)
   From IDL File: ip_multimedia_subsystem_private_service_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "ip_multimedia_subsystem_private_service_v01.h"
#include "common_v01.h"
#include "ip_multimedia_subsystem_ims_common_v01.h"


/*Type Definitions*/
static const uint8_t ims_private_service_header_value_data_v01[] = {
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_private_service_header_value_v01, header),
  ((IMS_PRIVATE_SERVICE_HEADER_STR_LEN_V01) & 0xFF), ((IMS_PRIVATE_SERVICE_HEADER_STR_LEN_V01) >> 8),

  QMI_IDL_FLAGS_OFFSET_IS_16 | QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN | QMI_IDL_FLAGS_SZ_IS_16 | QMI_IDL_STRING,
  QMI_IDL_OFFSET16ARRAY(ims_private_service_header_value_v01, value),
  ((IMS_PRIVATE_SERVICE_HEADER_STR_LEN_V01) & 0xFF), ((IMS_PRIVATE_SERVICE_HEADER_STR_LEN_V01) >> 8),

  QMI_IDL_FLAG_END_VALUE
};

/*Message Definitions*/
static const uint8_t ims_private_service_subscribe_for_indications_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_private_service_subscribe_for_indications_req_msg_v01, mt_invite) - QMI_IDL_OFFSET8(ims_private_service_subscribe_for_indications_req_msg_v01, mt_invite_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ims_private_service_subscribe_for_indications_req_msg_v01, mt_invite)
};

static const uint8_t ims_private_service_subscribe_for_indications_rsp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_private_service_subscribe_for_indications_rsp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t ims_private_service_mt_invite_ind_msg_data_v01[] = {
  0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ims_private_service_mt_invite_ind_msg_v01, subscription_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_private_service_mt_invite_ind_msg_v01, iccid) - QMI_IDL_OFFSET8(ims_private_service_mt_invite_ind_msg_v01, iccid_valid)),
  0x10,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_STRING,
  QMI_IDL_OFFSET8(ims_private_service_mt_invite_ind_msg_v01, iccid),
  IMS_PRIVATE_SERVICE_MAX_ICCID_LEN_V01,

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ims_private_service_mt_invite_ind_msg_v01, header_value_list) - QMI_IDL_OFFSET8(ims_private_service_mt_invite_ind_msg_v01, header_value_list_valid)),
  0x11,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ims_private_service_mt_invite_ind_msg_v01, header_value_list),
  IMS_PRIVATE_SERVICE_MAX_MT_INVITE_HEADERS_V01,
  QMI_IDL_OFFSET8(ims_private_service_mt_invite_ind_msg_v01, header_value_list) - QMI_IDL_OFFSET8(ims_private_service_mt_invite_ind_msg_v01, header_value_list_len),
  QMI_IDL_TYPE88(0, 0)
};

/* Type Table */
static const qmi_idl_type_table_entry  imsprivate_type_table_v01[] = {
  {sizeof(ims_private_service_header_value_v01), ims_private_service_header_value_data_v01}
};

/* Message Table */
static const qmi_idl_message_table_entry imsprivate_message_table_v01[] = {
  {sizeof(ims_private_service_subscribe_for_indications_req_msg_v01), ims_private_service_subscribe_for_indications_req_msg_data_v01},
  {sizeof(ims_private_service_subscribe_for_indications_rsp_msg_v01), ims_private_service_subscribe_for_indications_rsp_msg_data_v01},
  {sizeof(ims_private_service_mt_invite_ind_msg_v01), ims_private_service_mt_invite_ind_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object imsprivate_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *imsprivate_qmi_idl_type_table_object_referenced_tables_v01[] =
{&imsprivate_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01, &ip_multimedia_subsystem_ims_common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object imsprivate_qmi_idl_type_table_object_v01 = {
  sizeof(imsprivate_type_table_v01)/sizeof(qmi_idl_type_table_entry ),
  sizeof(imsprivate_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  imsprivate_type_table_v01,
  imsprivate_message_table_v01,
  imsprivate_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry imsprivate_service_command_messages_v01[] = {
  {QMI_IMS_PRIVATE_SERVICE_SUBSCRIBE_FOR_INDICATIONS_REQ_V01, QMI_IDL_TYPE16(0, 0), 4}
};

static const qmi_idl_service_message_table_entry imsprivate_service_response_messages_v01[] = {
  {QMI_IMS_PRIVATE_SERVICE_SUBSCRIBE_FOR_INDICATIONS_RSP_V01, QMI_IDL_TYPE16(0, 1), 7}
};

static const qmi_idl_service_message_table_entry imsprivate_service_indication_messages_v01[] = {
  {QMI_IMS_PRIVATE_SERVICE_MT_INVITE_IND_V01, QMI_IDL_TYPE16(0, 2), 30815}
};

/*Service Object*/
struct qmi_idl_service_object imsprivate_qmi_idl_service_object_v01 = {
  0x06,
  0x01,
  0x4D,
  30815,
  { sizeof(imsprivate_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(imsprivate_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(imsprivate_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { imsprivate_service_command_messages_v01, imsprivate_service_response_messages_v01, imsprivate_service_indication_messages_v01},
  &imsprivate_qmi_idl_type_table_object_v01,
  0x01,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type imsprivate_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( IMSPRIVATE_V01_IDL_MAJOR_VERS != idl_maj_version || IMSPRIVATE_V01_IDL_MINOR_VERS != idl_min_version
       || IMSPRIVATE_V01_IDL_TOOL_VERS != library_version)
  {
    return NULL;
  }
  return (qmi_idl_service_object_type)&imsprivate_qmi_idl_service_object_v01;
}
