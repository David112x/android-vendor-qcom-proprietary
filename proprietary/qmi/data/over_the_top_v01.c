/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        O V E R _ T H E _ T O P _ V 0 1  . C

GENERAL DESCRIPTION
  This is the file which defines the ott service Data structures.

  Copyright (c) 2015-2018 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.14.7
   It was generated on: Fri Mar  2 2018 (Spin 0)
   From IDL File: over_the_top_v01.idl */

#include "stdint.h"
#include "qmi_idl_lib_internal.h"
#include "over_the_top_v01.h"
#include "common_v01.h"


/*Type Definitions*/
/*Message Definitions*/
static const uint8_t ott_bind_subscription_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_bind_subscription_req_msg_v01, subscription)
};

static const uint8_t ott_bind_subscription_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ott_bind_subscription_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

/*
 * ott_get_bind_subscription_req_msg is empty
 * static const uint8_t ott_get_bind_subscription_req_msg_data_v01[] = {
 * };
 */

static const uint8_t ott_get_bind_subscription_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ott_get_bind_subscription_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_get_bind_subscription_resp_msg_v01, subscription) - QMI_IDL_OFFSET8(ott_get_bind_subscription_resp_msg_v01, subscription_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_get_bind_subscription_resp_msg_v01, subscription)
};

static const uint8_t ott_indication_register_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_indication_register_req_msg_v01, report_ul_thrpt_reporting_status_change) - QMI_IDL_OFFSET8(ott_indication_register_req_msg_v01, report_ul_thrpt_reporting_status_change_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ott_indication_register_req_msg_v01, report_ul_thrpt_reporting_status_change),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_indication_register_req_msg_v01, report_ul_thrpt_info) - QMI_IDL_OFFSET8(ott_indication_register_req_msg_v01, report_ul_thrpt_info_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ott_indication_register_req_msg_v01, report_ul_thrpt_info),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_indication_register_req_msg_v01, report_dl_thrpt_reporting_status_change) - QMI_IDL_OFFSET8(ott_indication_register_req_msg_v01, report_dl_thrpt_reporting_status_change_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ott_indication_register_req_msg_v01, report_dl_thrpt_reporting_status_change),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_indication_register_req_msg_v01, report_dl_thrpt_info) - QMI_IDL_OFFSET8(ott_indication_register_req_msg_v01, report_dl_thrpt_info_valid)),
  0x13,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ott_indication_register_req_msg_v01, report_dl_thrpt_info)
};

static const uint8_t ott_indication_register_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ott_indication_register_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t ott_configure_uplink_throughput_settings_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_configure_uplink_throughput_settings_req_msg_v01, ul_throughput_report_period) - QMI_IDL_OFFSET8(ott_configure_uplink_throughput_settings_req_msg_v01, ul_throughput_report_period_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_configure_uplink_throughput_settings_req_msg_v01, ul_throughput_report_period),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_configure_uplink_throughput_settings_req_msg_v01, report_ul_queue_size) - QMI_IDL_OFFSET8(ott_configure_uplink_throughput_settings_req_msg_v01, report_ul_queue_size_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ott_configure_uplink_throughput_settings_req_msg_v01, report_ul_queue_size),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_configure_uplink_throughput_settings_req_msg_v01, rat_type) - QMI_IDL_OFFSET8(ott_configure_uplink_throughput_settings_req_msg_v01, rat_type_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_configure_uplink_throughput_settings_req_msg_v01, rat_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_configure_uplink_throughput_settings_req_msg_v01, ul_throughput_hyst_time) - QMI_IDL_OFFSET8(ott_configure_uplink_throughput_settings_req_msg_v01, ul_throughput_hyst_time_valid)),
  0x13,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_configure_uplink_throughput_settings_req_msg_v01, ul_throughput_hyst_time),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_configure_uplink_throughput_settings_req_msg_v01, ul_throughput_hyst_mag_chng_size) - QMI_IDL_OFFSET8(ott_configure_uplink_throughput_settings_req_msg_v01, ul_throughput_hyst_mag_chng_size_valid)),
  0x14,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_configure_uplink_throughput_settings_req_msg_v01, ul_throughput_hyst_mag_chng_size),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_configure_uplink_throughput_settings_req_msg_v01, ul_throughput_threshold_list) - QMI_IDL_OFFSET8(ott_configure_uplink_throughput_settings_req_msg_v01, ul_throughput_threshold_list_valid)),
  0x15,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_configure_uplink_throughput_settings_req_msg_v01, ul_throughput_threshold_list),
  QMI_OTT_UL_THRESHOLD_LIST_LEN_V01,
  QMI_IDL_OFFSET8(ott_configure_uplink_throughput_settings_req_msg_v01, ul_throughput_threshold_list) - QMI_IDL_OFFSET8(ott_configure_uplink_throughput_settings_req_msg_v01, ul_throughput_threshold_list_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_configure_uplink_throughput_settings_req_msg_v01, ul_threshold_action) - QMI_IDL_OFFSET8(ott_configure_uplink_throughput_settings_req_msg_v01, ul_threshold_action_valid)),
  0x16,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_configure_uplink_throughput_settings_req_msg_v01, ul_threshold_action)
};

static const uint8_t ott_configure_uplink_throughput_settings_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ott_configure_uplink_throughput_settings_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t ott_uplink_throughput_reporting_status_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_uplink_throughput_reporting_status_ind_msg_v01, reporting_status) - QMI_IDL_OFFSET8(ott_uplink_throughput_reporting_status_ind_msg_v01, reporting_status_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_uplink_throughput_reporting_status_ind_msg_v01, reporting_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_uplink_throughput_reporting_status_ind_msg_v01, actual_interval) - QMI_IDL_OFFSET8(ott_uplink_throughput_reporting_status_ind_msg_v01, actual_interval_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_uplink_throughput_reporting_status_ind_msg_v01, actual_interval),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_uplink_throughput_reporting_status_ind_msg_v01, rat_type) - QMI_IDL_OFFSET8(ott_uplink_throughput_reporting_status_ind_msg_v01, rat_type_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_uplink_throughput_reporting_status_ind_msg_v01, rat_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_uplink_throughput_reporting_status_ind_msg_v01, actual_ul_throughput_hyst_time) - QMI_IDL_OFFSET8(ott_uplink_throughput_reporting_status_ind_msg_v01, actual_ul_throughput_hyst_time_valid)),
  0x13,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_uplink_throughput_reporting_status_ind_msg_v01, actual_ul_throughput_hyst_time),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_uplink_throughput_reporting_status_ind_msg_v01, actual_ul_throughput_hyst_mag_chng_size) - QMI_IDL_OFFSET8(ott_uplink_throughput_reporting_status_ind_msg_v01, actual_ul_throughput_hyst_mag_chng_size_valid)),
  0x14,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_uplink_throughput_reporting_status_ind_msg_v01, actual_ul_throughput_hyst_mag_chng_size),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_uplink_throughput_reporting_status_ind_msg_v01, actual_ul_throughput_threshold_list) - QMI_IDL_OFFSET8(ott_uplink_throughput_reporting_status_ind_msg_v01, actual_ul_throughput_threshold_list_valid)),
  0x15,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_uplink_throughput_reporting_status_ind_msg_v01, actual_ul_throughput_threshold_list),
  QMI_OTT_UL_THRESHOLD_LIST_LEN_V01,
  QMI_IDL_OFFSET8(ott_uplink_throughput_reporting_status_ind_msg_v01, actual_ul_throughput_threshold_list) - QMI_IDL_OFFSET8(ott_uplink_throughput_reporting_status_ind_msg_v01, actual_ul_throughput_threshold_list_len)
};

/*
 * ott_query_uplink_throughput_reporting_status_req_msg is empty
 * static const uint8_t ott_query_uplink_throughput_reporting_status_req_msg_data_v01[] = {
 * };
 */

static const uint8_t ott_query_uplink_throughput_reporting_status_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ott_query_uplink_throughput_reporting_status_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_query_uplink_throughput_reporting_status_resp_msg_v01, reporting_status) - QMI_IDL_OFFSET8(ott_query_uplink_throughput_reporting_status_resp_msg_v01, reporting_status_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_query_uplink_throughput_reporting_status_resp_msg_v01, reporting_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_query_uplink_throughput_reporting_status_resp_msg_v01, actual_interval) - QMI_IDL_OFFSET8(ott_query_uplink_throughput_reporting_status_resp_msg_v01, actual_interval_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_query_uplink_throughput_reporting_status_resp_msg_v01, actual_interval),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_query_uplink_throughput_reporting_status_resp_msg_v01, rat_type) - QMI_IDL_OFFSET8(ott_query_uplink_throughput_reporting_status_resp_msg_v01, rat_type_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_query_uplink_throughput_reporting_status_resp_msg_v01, rat_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_query_uplink_throughput_reporting_status_resp_msg_v01, actual_ul_throughput_hyst_time) - QMI_IDL_OFFSET8(ott_query_uplink_throughput_reporting_status_resp_msg_v01, actual_ul_throughput_hyst_time_valid)),
  0x13,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_query_uplink_throughput_reporting_status_resp_msg_v01, actual_ul_throughput_hyst_time),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_query_uplink_throughput_reporting_status_resp_msg_v01, actual_ul_throughput_hyst_mag_chng_size) - QMI_IDL_OFFSET8(ott_query_uplink_throughput_reporting_status_resp_msg_v01, actual_ul_throughput_hyst_mag_chng_size_valid)),
  0x14,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_query_uplink_throughput_reporting_status_resp_msg_v01, actual_ul_throughput_hyst_mag_chng_size),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_query_uplink_throughput_reporting_status_resp_msg_v01, actual_ul_throughput_threshold_list) - QMI_IDL_OFFSET8(ott_query_uplink_throughput_reporting_status_resp_msg_v01, actual_ul_throughput_threshold_list_valid)),
  0x15,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_query_uplink_throughput_reporting_status_resp_msg_v01, actual_ul_throughput_threshold_list),
  QMI_OTT_UL_THRESHOLD_LIST_LEN_V01,
  QMI_IDL_OFFSET8(ott_query_uplink_throughput_reporting_status_resp_msg_v01, actual_ul_throughput_threshold_list) - QMI_IDL_OFFSET8(ott_query_uplink_throughput_reporting_status_resp_msg_v01, actual_ul_throughput_threshold_list_len)
};

static const uint8_t ott_uplink_throughput_info_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_uplink_throughput_info_ind_msg_v01, uplink_rate) - QMI_IDL_OFFSET8(ott_uplink_throughput_info_ind_msg_v01, uplink_rate_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_uplink_throughput_info_ind_msg_v01, uplink_rate),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_uplink_throughput_info_ind_msg_v01, confidence_level) - QMI_IDL_OFFSET8(ott_uplink_throughput_info_ind_msg_v01, confidence_level_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ott_uplink_throughput_info_ind_msg_v01, confidence_level),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_uplink_throughput_info_ind_msg_v01, uplink_queue_size) - QMI_IDL_OFFSET8(ott_uplink_throughput_info_ind_msg_v01, uplink_queue_size_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_uplink_throughput_info_ind_msg_v01, uplink_queue_size)
};

static const uint8_t ott_query_uplink_throughput_info_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_query_uplink_throughput_info_req_msg_v01, report_ul_queue_size) - QMI_IDL_OFFSET8(ott_query_uplink_throughput_info_req_msg_v01, report_ul_queue_size_valid)),
  0x10,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ott_query_uplink_throughput_info_req_msg_v01, report_ul_queue_size)
};

static const uint8_t ott_query_uplink_throughput_info_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ott_query_uplink_throughput_info_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_query_uplink_throughput_info_resp_msg_v01, uplink_rate) - QMI_IDL_OFFSET8(ott_query_uplink_throughput_info_resp_msg_v01, uplink_rate_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_query_uplink_throughput_info_resp_msg_v01, uplink_rate),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_query_uplink_throughput_info_resp_msg_v01, confidence_level) - QMI_IDL_OFFSET8(ott_query_uplink_throughput_info_resp_msg_v01, confidence_level_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ott_query_uplink_throughput_info_resp_msg_v01, confidence_level),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_query_uplink_throughput_info_resp_msg_v01, uplink_queue_size) - QMI_IDL_OFFSET8(ott_query_uplink_throughput_info_resp_msg_v01, uplink_queue_size_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_query_uplink_throughput_info_resp_msg_v01, uplink_queue_size)
};

static const uint8_t ott_configure_downlink_throughput_settings_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_configure_downlink_throughput_settings_req_msg_v01, dl_throughput_report_period) - QMI_IDL_OFFSET8(ott_configure_downlink_throughput_settings_req_msg_v01, dl_throughput_report_period_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_configure_downlink_throughput_settings_req_msg_v01, dl_throughput_report_period),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_configure_downlink_throughput_settings_req_msg_v01, rat_type) - QMI_IDL_OFFSET8(ott_configure_downlink_throughput_settings_req_msg_v01, rat_type_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_configure_downlink_throughput_settings_req_msg_v01, rat_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_configure_downlink_throughput_settings_req_msg_v01, dl_throughput_hyst_time) - QMI_IDL_OFFSET8(ott_configure_downlink_throughput_settings_req_msg_v01, dl_throughput_hyst_time_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_configure_downlink_throughput_settings_req_msg_v01, dl_throughput_hyst_time),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_configure_downlink_throughput_settings_req_msg_v01, dl_throughput_hyst_mag_chng_size) - QMI_IDL_OFFSET8(ott_configure_downlink_throughput_settings_req_msg_v01, dl_throughput_hyst_mag_chng_size_valid)),
  0x13,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_configure_downlink_throughput_settings_req_msg_v01, dl_throughput_hyst_mag_chng_size),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_configure_downlink_throughput_settings_req_msg_v01, dl_throughput_threshold_list) - QMI_IDL_OFFSET8(ott_configure_downlink_throughput_settings_req_msg_v01, dl_throughput_threshold_list_valid)),
  0x14,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_configure_downlink_throughput_settings_req_msg_v01, dl_throughput_threshold_list),
  QMI_OTT_DL_THRESHOLD_LIST_LEN_V01,
  QMI_IDL_OFFSET8(ott_configure_downlink_throughput_settings_req_msg_v01, dl_throughput_threshold_list) - QMI_IDL_OFFSET8(ott_configure_downlink_throughput_settings_req_msg_v01, dl_throughput_threshold_list_len),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_configure_downlink_throughput_settings_req_msg_v01, dl_threshold_action) - QMI_IDL_OFFSET8(ott_configure_downlink_throughput_settings_req_msg_v01, dl_threshold_action_valid)),
  0x15,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_configure_downlink_throughput_settings_req_msg_v01, dl_threshold_action)
};

static const uint8_t ott_configure_downlink_throughput_settings_resp_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ott_configure_downlink_throughput_settings_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0)
};

static const uint8_t ott_downlink_throughput_reporting_status_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_downlink_throughput_reporting_status_ind_msg_v01, reporting_status) - QMI_IDL_OFFSET8(ott_downlink_throughput_reporting_status_ind_msg_v01, reporting_status_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_downlink_throughput_reporting_status_ind_msg_v01, reporting_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_downlink_throughput_reporting_status_ind_msg_v01, actual_interval) - QMI_IDL_OFFSET8(ott_downlink_throughput_reporting_status_ind_msg_v01, actual_interval_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_downlink_throughput_reporting_status_ind_msg_v01, actual_interval),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_downlink_throughput_reporting_status_ind_msg_v01, rat_type) - QMI_IDL_OFFSET8(ott_downlink_throughput_reporting_status_ind_msg_v01, rat_type_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_downlink_throughput_reporting_status_ind_msg_v01, rat_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_downlink_throughput_reporting_status_ind_msg_v01, actual_dl_throughput_hyst_time) - QMI_IDL_OFFSET8(ott_downlink_throughput_reporting_status_ind_msg_v01, actual_dl_throughput_hyst_time_valid)),
  0x13,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_downlink_throughput_reporting_status_ind_msg_v01, actual_dl_throughput_hyst_time),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_downlink_throughput_reporting_status_ind_msg_v01, actual_dl_throughput_hyst_mag_chng_size) - QMI_IDL_OFFSET8(ott_downlink_throughput_reporting_status_ind_msg_v01, actual_dl_throughput_hyst_mag_chng_size_valid)),
  0x14,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_downlink_throughput_reporting_status_ind_msg_v01, actual_dl_throughput_hyst_mag_chng_size),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_downlink_throughput_reporting_status_ind_msg_v01, actual_dl_throughput_threshold_list) - QMI_IDL_OFFSET8(ott_downlink_throughput_reporting_status_ind_msg_v01, actual_dl_throughput_threshold_list_valid)),
  0x15,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_downlink_throughput_reporting_status_ind_msg_v01, actual_dl_throughput_threshold_list),
  QMI_OTT_DL_THRESHOLD_LIST_LEN_V01,
  QMI_IDL_OFFSET8(ott_downlink_throughput_reporting_status_ind_msg_v01, actual_dl_throughput_threshold_list) - QMI_IDL_OFFSET8(ott_downlink_throughput_reporting_status_ind_msg_v01, actual_dl_throughput_threshold_list_len)
};

/*
 * ott_query_downlink_throughput_reporting_status_req_msg is empty
 * static const uint8_t ott_query_downlink_throughput_reporting_status_req_msg_data_v01[] = {
 * };
 */

static const uint8_t ott_query_downlink_throughput_reporting_status_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ott_query_downlink_throughput_reporting_status_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_query_downlink_throughput_reporting_status_resp_msg_v01, reporting_status) - QMI_IDL_OFFSET8(ott_query_downlink_throughput_reporting_status_resp_msg_v01, reporting_status_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_query_downlink_throughput_reporting_status_resp_msg_v01, reporting_status),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_query_downlink_throughput_reporting_status_resp_msg_v01, actual_interval) - QMI_IDL_OFFSET8(ott_query_downlink_throughput_reporting_status_resp_msg_v01, actual_interval_valid)),
  0x11,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_query_downlink_throughput_reporting_status_resp_msg_v01, actual_interval),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_query_downlink_throughput_reporting_status_resp_msg_v01, rat_type) - QMI_IDL_OFFSET8(ott_query_downlink_throughput_reporting_status_resp_msg_v01, rat_type_valid)),
  0x12,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_query_downlink_throughput_reporting_status_resp_msg_v01, rat_type),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_query_downlink_throughput_reporting_status_resp_msg_v01, actual_dl_throughput_hyst_time) - QMI_IDL_OFFSET8(ott_query_downlink_throughput_reporting_status_resp_msg_v01, actual_dl_throughput_hyst_time_valid)),
  0x13,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_query_downlink_throughput_reporting_status_resp_msg_v01, actual_dl_throughput_hyst_time),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_query_downlink_throughput_reporting_status_resp_msg_v01, actual_dl_throughput_hyst_mag_chng_size) - QMI_IDL_OFFSET8(ott_query_downlink_throughput_reporting_status_resp_msg_v01, actual_dl_throughput_hyst_mag_chng_size_valid)),
  0x14,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_query_downlink_throughput_reporting_status_resp_msg_v01, actual_dl_throughput_hyst_mag_chng_size),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_query_downlink_throughput_reporting_status_resp_msg_v01, actual_dl_throughput_threshold_list) - QMI_IDL_OFFSET8(ott_query_downlink_throughput_reporting_status_resp_msg_v01, actual_dl_throughput_threshold_list_valid)),
  0x15,
  QMI_IDL_FLAGS_IS_ARRAY | QMI_IDL_FLAGS_IS_VARIABLE_LEN |  QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_query_downlink_throughput_reporting_status_resp_msg_v01, actual_dl_throughput_threshold_list),
  QMI_OTT_UL_THRESHOLD_LIST_LEN_V01,
  QMI_IDL_OFFSET8(ott_query_downlink_throughput_reporting_status_resp_msg_v01, actual_dl_throughput_threshold_list) - QMI_IDL_OFFSET8(ott_query_downlink_throughput_reporting_status_resp_msg_v01, actual_dl_throughput_threshold_list_len)
};

static const uint8_t ott_downlink_throughput_info_ind_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_downlink_throughput_info_ind_msg_v01, downlink_rate) - QMI_IDL_OFFSET8(ott_downlink_throughput_info_ind_msg_v01, downlink_rate_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_downlink_throughput_info_ind_msg_v01, downlink_rate),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_downlink_throughput_info_ind_msg_v01, confidence_level) - QMI_IDL_OFFSET8(ott_downlink_throughput_info_ind_msg_v01, confidence_level_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ott_downlink_throughput_info_ind_msg_v01, confidence_level),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_downlink_throughput_info_ind_msg_v01, is_suspended) - QMI_IDL_OFFSET8(ott_downlink_throughput_info_ind_msg_v01, is_suspended_valid)),
  0x12,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ott_downlink_throughput_info_ind_msg_v01, is_suspended)
};

/*
 * ott_get_downlink_throughput_info_req_msg is empty
 * static const uint8_t ott_get_downlink_throughput_info_req_msg_data_v01[] = {
 * };
 */

static const uint8_t ott_get_downlink_throughput_info_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ott_get_downlink_throughput_info_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_get_downlink_throughput_info_resp_msg_v01, downlink_rate) - QMI_IDL_OFFSET8(ott_get_downlink_throughput_info_resp_msg_v01, downlink_rate_valid)),
  0x10,
   QMI_IDL_GENERIC_4_BYTE,
  QMI_IDL_OFFSET8(ott_get_downlink_throughput_info_resp_msg_v01, downlink_rate),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_get_downlink_throughput_info_resp_msg_v01, confidence_level) - QMI_IDL_OFFSET8(ott_get_downlink_throughput_info_resp_msg_v01, confidence_level_valid)),
  0x11,
   QMI_IDL_GENERIC_1_BYTE,
  QMI_IDL_OFFSET8(ott_get_downlink_throughput_info_resp_msg_v01, confidence_level)
};

static const uint8_t ott_get_capabilities_req_msg_data_v01[] = {
  QMI_IDL_TLV_FLAGS_LAST_TLV | 0x01,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(ott_get_capabilities_req_msg_v01, req_capability_mask)
};

static const uint8_t ott_get_capabilities_resp_msg_data_v01[] = {
  0x02,
   QMI_IDL_AGGREGATE,
  QMI_IDL_OFFSET8(ott_get_capabilities_resp_msg_v01, resp),
  QMI_IDL_TYPE88(1, 0),

  QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_get_capabilities_resp_msg_v01, ul_capability) - QMI_IDL_OFFSET8(ott_get_capabilities_resp_msg_v01, ul_capability_valid)),
  0x10,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(ott_get_capabilities_resp_msg_v01, ul_capability),

  QMI_IDL_TLV_FLAGS_LAST_TLV | QMI_IDL_TLV_FLAGS_OPTIONAL | (QMI_IDL_OFFSET8(ott_get_capabilities_resp_msg_v01, dl_capability) - QMI_IDL_OFFSET8(ott_get_capabilities_resp_msg_v01, dl_capability_valid)),
  0x11,
   QMI_IDL_GENERIC_8_BYTE,
  QMI_IDL_OFFSET8(ott_get_capabilities_resp_msg_v01, dl_capability)
};

/* Type Table */
/* No Types Defined in IDL */

/* Message Table */
static const qmi_idl_message_table_entry ott_message_table_v01[] = {
  {sizeof(ott_bind_subscription_req_msg_v01), ott_bind_subscription_req_msg_data_v01},
  {sizeof(ott_bind_subscription_resp_msg_v01), ott_bind_subscription_resp_msg_data_v01},
  {sizeof(ott_get_bind_subscription_req_msg_v01), 0},
  {sizeof(ott_get_bind_subscription_resp_msg_v01), ott_get_bind_subscription_resp_msg_data_v01},
  {sizeof(ott_indication_register_req_msg_v01), ott_indication_register_req_msg_data_v01},
  {sizeof(ott_indication_register_resp_msg_v01), ott_indication_register_resp_msg_data_v01},
  {sizeof(ott_configure_uplink_throughput_settings_req_msg_v01), ott_configure_uplink_throughput_settings_req_msg_data_v01},
  {sizeof(ott_configure_uplink_throughput_settings_resp_msg_v01), ott_configure_uplink_throughput_settings_resp_msg_data_v01},
  {sizeof(ott_uplink_throughput_reporting_status_ind_msg_v01), ott_uplink_throughput_reporting_status_ind_msg_data_v01},
  {sizeof(ott_query_uplink_throughput_reporting_status_req_msg_v01), 0},
  {sizeof(ott_query_uplink_throughput_reporting_status_resp_msg_v01), ott_query_uplink_throughput_reporting_status_resp_msg_data_v01},
  {sizeof(ott_uplink_throughput_info_ind_msg_v01), ott_uplink_throughput_info_ind_msg_data_v01},
  {sizeof(ott_query_uplink_throughput_info_req_msg_v01), ott_query_uplink_throughput_info_req_msg_data_v01},
  {sizeof(ott_query_uplink_throughput_info_resp_msg_v01), ott_query_uplink_throughput_info_resp_msg_data_v01},
  {sizeof(ott_configure_downlink_throughput_settings_req_msg_v01), ott_configure_downlink_throughput_settings_req_msg_data_v01},
  {sizeof(ott_configure_downlink_throughput_settings_resp_msg_v01), ott_configure_downlink_throughput_settings_resp_msg_data_v01},
  {sizeof(ott_downlink_throughput_reporting_status_ind_msg_v01), ott_downlink_throughput_reporting_status_ind_msg_data_v01},
  {sizeof(ott_query_downlink_throughput_reporting_status_req_msg_v01), 0},
  {sizeof(ott_query_downlink_throughput_reporting_status_resp_msg_v01), ott_query_downlink_throughput_reporting_status_resp_msg_data_v01},
  {sizeof(ott_downlink_throughput_info_ind_msg_v01), ott_downlink_throughput_info_ind_msg_data_v01},
  {sizeof(ott_get_downlink_throughput_info_req_msg_v01), 0},
  {sizeof(ott_get_downlink_throughput_info_resp_msg_v01), ott_get_downlink_throughput_info_resp_msg_data_v01},
  {sizeof(ott_get_capabilities_req_msg_v01), ott_get_capabilities_req_msg_data_v01},
  {sizeof(ott_get_capabilities_resp_msg_v01), ott_get_capabilities_resp_msg_data_v01}
};

/* Range Table */
/* No Ranges Defined in IDL */

/* Predefine the Type Table Object */
static const qmi_idl_type_table_object ott_qmi_idl_type_table_object_v01;

/*Referenced Tables Array*/
static const qmi_idl_type_table_object *ott_qmi_idl_type_table_object_referenced_tables_v01[] =
{&ott_qmi_idl_type_table_object_v01, &common_qmi_idl_type_table_object_v01};

/*Type Table Object*/
static const qmi_idl_type_table_object ott_qmi_idl_type_table_object_v01 = {
  0,
  sizeof(ott_message_table_v01)/sizeof(qmi_idl_message_table_entry),
  1,
  NULL,
  ott_message_table_v01,
  ott_qmi_idl_type_table_object_referenced_tables_v01,
  NULL
};

/*Arrays of service_message_table_entries for commands, responses and indications*/
static const qmi_idl_service_message_table_entry ott_service_command_messages_v01[] = {
  {QMI_OTT_BIND_SUBSCRIPTION_REQ_V01, QMI_IDL_TYPE16(0, 0), 7},
  {QMI_OTT_GET_BIND_SUBSCRIPTION_REQ_V01, QMI_IDL_TYPE16(0, 2), 0},
  {QMI_OTT_INDICATION_REGISTER_REQ_V01, QMI_IDL_TYPE16(0, 4), 16},
  {QMI_OTT_CONFIGURE_UPLINK_THROUGHPUT_SETTINGS_REQ_V01, QMI_IDL_TYPE16(0, 6), 123},
  {QMI_OTT_QUERY_UPLINK_THROUGHPUT_REPORTING_STATUS_REQ_V01, QMI_IDL_TYPE16(0, 9), 0},
  {QMI_OTT_QUERY_UPLINK_THROUGHPUT_INFO_REQ_V01, QMI_IDL_TYPE16(0, 12), 4},
  {QMI_OTT_CONFIGURE_DOWNLINK_THROUGHPUT_SETTINGS_REQ_V01, QMI_IDL_TYPE16(0, 14), 119},
  {QMI_OTT_QUERY_DOWNLINK_THROUGHPUT_REPORTING_STATUS_REQ_V01, QMI_IDL_TYPE16(0, 17), 0},
  {QMI_OTT_GET_DOWNLINK_THROUGHPUT_INFO_REQ_V01, QMI_IDL_TYPE16(0, 20), 0},
  {QMI_OTT_GET_CAPABILITIES_REQ_V01, QMI_IDL_TYPE16(0, 22), 11}
};

static const qmi_idl_service_message_table_entry ott_service_response_messages_v01[] = {
  {QMI_OTT_BIND_SUBSCRIPTION_RESP_V01, QMI_IDL_TYPE16(0, 1), 7},
  {QMI_OTT_GET_BIND_SUBSCRIPTION_RESP_V01, QMI_IDL_TYPE16(0, 3), 14},
  {QMI_OTT_INDICATION_REGISTER_RESP_V01, QMI_IDL_TYPE16(0, 5), 7},
  {QMI_OTT_CONFIGURE_UPLINK_THROUGHPUT_SETTINGS_RESP_V01, QMI_IDL_TYPE16(0, 7), 7},
  {QMI_OTT_QUERY_UPLINK_THROUGHPUT_REPORTING_STATUS_RESP_V01, QMI_IDL_TYPE16(0, 10), 126},
  {QMI_OTT_QUERY_UPLINK_THROUGHPUT_INFO_RESP_V01, QMI_IDL_TYPE16(0, 13), 25},
  {QMI_OTT_CONFIGURE_DOWNLINK_THROUGHPUT_SETTINGS_RESP_V01, QMI_IDL_TYPE16(0, 15), 7},
  {QMI_OTT_QUERY_DOWNLINK_THROUGHPUT_REPORTING_STATUS_RESP_V01, QMI_IDL_TYPE16(0, 18), 126},
  {QMI_OTT_GET_DOWNLINK_THROUGHPUT_INFO_RESP_V01, QMI_IDL_TYPE16(0, 21), 18},
  {QMI_OTT_GET_CAPABILITIES_RESP_V01, QMI_IDL_TYPE16(0, 23), 29}
};

static const qmi_idl_service_message_table_entry ott_service_indication_messages_v01[] = {
  {QMI_OTT_UPLINK_THROUGHPUT_REPORTING_STATUS_IND_V01, QMI_IDL_TYPE16(0, 8), 119},
  {QMI_OTT_UPLINK_THROUGHPUT_INFO_IND_V01, QMI_IDL_TYPE16(0, 11), 18},
  {QMI_OTT_DOWNLINK_THROUGHPUT_REPORTING_STATUS_IND_V01, QMI_IDL_TYPE16(0, 16), 119},
  {QMI_OTT_DOWNLINK_THROUGHPUT_INFO_IND_V01, QMI_IDL_TYPE16(0, 19), 15}
};

/*Service Object*/
struct qmi_idl_service_object ott_qmi_idl_service_object_v01 = {
  0x06,
  0x01,
  0x44,
  126,
  { sizeof(ott_service_command_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(ott_service_response_messages_v01)/sizeof(qmi_idl_service_message_table_entry),
    sizeof(ott_service_indication_messages_v01)/sizeof(qmi_idl_service_message_table_entry) },
  { ott_service_command_messages_v01, ott_service_response_messages_v01, ott_service_indication_messages_v01},
  &ott_qmi_idl_type_table_object_v01,
  0x02,
  NULL
};

/* Service Object Accessor */
qmi_idl_service_object_type ott_get_service_object_internal_v01
 ( int32_t idl_maj_version, int32_t idl_min_version, int32_t library_version ){
  if ( OTT_V01_IDL_MAJOR_VERS != idl_maj_version || OTT_V01_IDL_MINOR_VERS != idl_min_version
       || OTT_V01_IDL_TOOL_VERS != library_version)
  {
    return NULL;
  }
  return (qmi_idl_service_object_type)&ott_qmi_idl_service_object_v01;
}
