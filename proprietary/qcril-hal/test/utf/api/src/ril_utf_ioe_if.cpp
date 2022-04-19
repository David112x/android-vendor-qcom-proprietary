/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include "ril_utf_ioe_if.h"

using namespace std;

//========================================================
//  IOE interface implementations
//========================================================

/*========================================================
  MCM_SMS_SEND_MO_MSG_REQ_V01
========================================================*/
void ril_utf_ioe_if::update_mcm_sms_send_mo_msg_req_msg(uint32_t format, char * message_content, char *destination)
{
  payload = (void *) malloc(sizeof(mcm_sms_send_mo_msg_req_msg_v01));
  payload_len = sizeof(mcm_sms_send_mo_msg_req_msg_v01);
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: payload_len: %d", payload_len);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ioe_request;
    mcm_sms_send_mo_msg_req_msg_v01 *ptr =
    (mcm_sms_send_mo_msg_req_msg_v01*) payload;
  msg_id = MCM_SMS_SEND_MO_MSG_REQ_V01;
  ptr->message_format = 0x0001;
  strcpy(ptr->message_content, message_content);
  strcpy(ptr->destination, destination);
  ptr->size_validation_valid = FALSE;
//  qmi_srv_id = 0;
   status = not_met;
}

void ril_utf_ioe_if::set_mcm_sms_send_mo_msg_req_msg_size_validation(uint8_t size_validation)
{
       mcm_sms_send_mo_msg_req_msg_v01 *ptr = (mcm_sms_send_mo_msg_req_msg_v01 *) payload;
   ptr->size_validation_valid= TRUE;
   ptr->size_validation = size_validation;
}
/*========================================================
  MCM_SMS_SEND_MO_MSG_RESP_V01
========================================================*/

void ril_utf_ioe_if::update_mcm_sms_send_mo_msg_resp_msg(mcm_result_t_v01 result, mcm_error_t_v01 error)
{
  payload = (void *) malloc(sizeof(mcm_sms_send_mo_msg_resp_msg_v01));
  payload_len = sizeof(mcm_sms_send_mo_msg_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ioe_response;
//qmi_srv_id = 0;
  msg_id = MCM_SMS_SEND_MO_MSG_RESP_V01;

  mcm_sms_send_mo_msg_resp_msg_v01 *ptr = (mcm_sms_send_mo_msg_resp_msg_v01 *) payload;
  ptr->response.error = error;
  ptr->response.result= result;
}

/*========================================================
  MCM_DM_GET_RADIO_MODE_REQ_V01
========================================================*/

void ril_utf_ioe_if::update_mcm_dm_get_radio_mode_req_msg()
{
  payload = (void *) malloc(sizeof(mcm_dm_get_radio_mode_req_msg_v01));
  payload_len = sizeof(mcm_dm_get_radio_mode_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ioe_request;
//qmi_srv_id = 0;
  msg_id = MCM_DM_GET_RADIO_MODE_REQ_V01;
  status = not_met;
}

/*========================================================
  MCM_DM_GET_RADIO_MODE_RESP_V01
========================================================*/
void ril_utf_ioe_if::update_mcm_dm_get_radio_mode_resp_msg(mcm_result_t_v01 result, mcm_error_t_v01 error)
{
  payload = (void *) malloc(sizeof(mcm_dm_get_radio_mode_resp_msg_v01));
  payload_len = sizeof(mcm_dm_get_radio_mode_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ioe_response;
  msg_id = MCM_DM_GET_RADIO_MODE_RESP_V01;
//qmi_srv_id = 0;
}

void ril_utf_ioe_if::set_mcm_dm_get_radio_mode_resp_msg_radio_mode(uint8_t radio_mode)
{
  mcm_dm_get_radio_mode_resp_msg_v01 *ptr =
    (mcm_dm_get_radio_mode_resp_msg_v01 *)payload;
  ptr->radio_mode_valid = TRUE;
  ptr->radio_mode = radio_mode;
}
