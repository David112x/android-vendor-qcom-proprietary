/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#ifndef RIL_UTF_ANDROIDTEL_PAYLOAD_H_
#define RIL_UTF_ANDROIDTEL_PAYLOAD_H_

#include <stdio.h>
#include <stdlib.h>
#include "ril.h"
#include "ril_utf_core.h"
#include "ril_utf_ril_api.h"

/*
  RIL_REQUEST_GET_CURRENT_CALLS (resp)
*/
RIL_Call** copy_ril_request_get_current_calls_resp(RIL_Call** call_list, int len);
void free_ril_request_get_current_calls_resp(RIL_Call** call_list, int len);
enum ril_utf_expectation_t verify_ril_get_current_calls(void *exp_node, int exp_len, void *data, int data_len);

RIL_LastCallFailCauseInfo* copy_ril_request_last_call_fail_cause_resp(RIL_LastCallFailCauseInfo* causeInfo);
enum ril_utf_expectation_t verify_ril_last_call_fail_cause(void *exp_node, int exp_len, void *data, int data_len);

/*
  RIL_REQUEST_GET_MUTE
*/
enum ril_utf_expectation_t verify_ril_get_mute(void *exp_node, int exp_len, void *data, int data_len);

/*
  RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE
*/
enum ril_utf_expectation_t verify_ril_cdma_query_roaming_preference(void *exp_node, int exp_len, void *data, int data_len);

/*
  RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE
*/
enum ril_utf_expectation_t verify_ril_cdma_get_subscription_source(void *exp_node, int exp_len, void *data, int data_len);

/*
  RIL_REQUEST_OPERATOR
*/
ril_request_operator_resp_from_qcril_t* copy_ril_request_operator_resp(char** opr);
enum ril_utf_expectation_t verify_ril_operator_name(void *exp_node, void *data);
void free_ril_request_operator_resp(ril_request_operator_resp_from_qcril_t* ptr);

/*
  RIL_REQUEST_QUERY_AVAILABLE_NETWORKS
*/
ril_request_query_available_networks_resp_t* copy_ril_request_query_available_networks_resp(char** networks, int len);
enum ril_utf_expectation_t verify_ril_query_available_network(void *exp_node, void *data);
void free_ril_request_query_available_networks_resp(ril_request_query_available_networks_resp_t* ptr);

char** copy_ril_unsol_on_ussd(char** data, int datalen);
enum ril_utf_expectation_t verify_ril_unsol_on_ussd(void *exp_node, void *data);

enum ril_utf_expectation_t verify_ril_unsol_on_ss(void *exp_node, void *data);

enum ril_utf_expectation_t verify_ril_unsol_supp_svc_notification(void *exp_node, void *data);
/*
 * RIL_UNSOL_CDMA_CALL_WAITING
 */
enum ril_utf_expectation_t verify_ril_unsol_cdma_call_waiting(void *exp_node, void *data);
/*
 * RIL_UNSOL_CDMA_OTA_PROVISION_STATUS
 */
enum ril_utf_expectation_t verify_ril_unsol_cdma_ota_provision_status(void *exp_node, void *data);
/*
 * RIL_UNSOL_CDMA_INFO_REC
 */
enum ril_utf_expectation_t verify_ril_unsol_cdma_info_rec(void *exp_node, void *data);
/*
 * RIL_UNSOL_RINGBACK_TONE
 */
enum ril_utf_expectation_t verify_ril_unsol_ringback_tone(void *exp_node, void *data);
/*
 * RIL_UNSOL_NETWORK_SCAN_RESULT
 */
enum ril_utf_expectation_t verify_ril_unsol_network_scan_result(void *exp_node, void *data);

enum ril_utf_expectation_t verify_ril_send_sms(void *exp_node, void *data);

/*
  RIL_REQUEST_QUERY_CALL_FORWARD_STATUS
*/
RIL_CallForwardInfo **copy_ril_request_query_call_forward_status_resp(RIL_CallForwardInfo **fwd_status_list, int len);
void free_ril_request_query_call_forward_status_resp(RIL_CallForwardInfo **call_fwd_info_list, int len);
enum ril_utf_expectation_t verify_ril_query_call_forward_status(void *exp_node, int exp_len, void *data, int data_len);

enum ril_utf_expectation_t verify_ril_get_sim_status(void *exp_node, int exp_len, void *data, int data_len);
RIL_CardStatus_v6 *copy_ril_request_get_sim_status_resp
(
   RIL_CardStatus_v6 *cardstatus, int len
);

/*
  RIL_REQUEST_SIM_OPEN_CHANNEL
*/
enum ril_utf_expectation_t verify_ril_open_channel_response(void *exp_node, int exp_len,
                             void *data, int data_len);

/*
  RIL_REQUEST_QUERY_TTY_MODE
*/
enum ril_utf_expectation_t verify_ril_query_tty_mode(void *exp_node, int exp_len, void *data, int data_len);
/*
  RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE
*/
enum ril_utf_expectation_t verify_ril_query_preferred_voice_privacy_mode(void *exp_node, int exp_len, void *data, int data_len);

/*
  RIL_REQUEST_GET_NEIGHBORING_CELL_IDS
*/
enum ril_utf_expectation_t verify_ril_get_neighboring_cell_ids(void *exp_node, int exp_len, void *data, int data_len);
#endif
