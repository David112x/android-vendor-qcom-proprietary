/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_qmi_api.c
  @brief   qmi API's for test case dev

  DESCRIPTION

  API's for modifying QMI messages used in RIL UTF tests.
  ---------------------------------------------------------------------------
******************************************************************************/
#include "ril_utf_qmi_api.h"

// Expect
void update_default_qmi_voice_ind_reg_req_msg(elnptr node)
{
  node->payload = (void *) malloc(sizeof(voice_indication_register_req_msg_v02));
  node->payload_len = sizeof(voice_indication_register_req_msg_v02);
  memset(node->payload, 0x00, node->payload_len);
  node->type = ril_utf_qmi_request;
  node->e.is_valid = TRUE;
  node->e.msg_id = QMI_VOICE_INDICATION_REGISTER_REQ_V02;
  node->e.status = not_met;

  //use default timeout
}

void set_qmi_voice_ind_req_reg_voice_privacy_events(elnptr node, int val)
{
  voice_indication_register_req_msg_v02 *ptr = node->payload;
  ptr->reg_voice_privacy_events_valid = TRUE;
  ptr->reg_voice_privacy_events       = val;
}

void set_qmi_voice_ind_req_ext_brst_intl_events(elnptr node, int val)
{
  voice_indication_register_req_msg_v02 *ptr = node->payload;
  ptr->ext_brst_intl_events_valid = TRUE;
  ptr->ext_brst_intl_events       = val;
}

void set_qmi_voice_ind_req_speech_events(elnptr node, int val)
{
  voice_indication_register_req_msg_v02 *ptr = node->payload;
  ptr->speech_events_valid = TRUE;
  ptr->speech_events       = val;
}

//inject
void update_default_qmi_voice_ind_reg_resp_msg(elnptr node)
{
  node->payload = (void *) malloc(sizeof(voice_indication_register_resp_msg_v02));
  node->payload_len = sizeof(voice_indication_register_resp_msg_v02);
  memset(node->payload, 0x00, node->payload_len);
  node->type = ril_utf_qmi_response;
  node->i.is_valid = TRUE;
  node->i.msg_id = QMI_VOICE_INDICATION_REGISTER_RESP_V02;
  node->i.qmi_srv_id = QMI_VOICE;
  node->i.is_sync_msg = TRUE;

  //use default client_id
}

/* QMI_DMS_SET_EVENT_REPORT_REQ_V01
*/
// expect
void update_default_qmi_dms_set_event_report_req_msg(elnptr node)
{
  node->payload = (void *) malloc(sizeof(dms_set_event_report_req_msg_v01));
  node->payload_len = sizeof(dms_set_event_report_req_msg_v01);
  memset(node->payload, 0x00, node->payload_len);
  node->type = ril_utf_qmi_request;
  node->e.is_valid = TRUE;
  node->e.msg_id = QMI_DMS_SET_EVENT_REPORT_REQ_V01;
  node->e.status = not_met;

  //use default timeout
}
void set_qmi_dsm_set_event_report_req_report_oprt_mode_state(elnptr node, int val)
{
  dms_set_event_report_req_msg_v01 *ptr = node->payload;
  ptr->report_oprt_mode_state_valid = TRUE;
  ptr->report_oprt_mode_state       = val;
}

void set_qmi_dsm_set_event_report_req_report_prl_init(elnptr node, int val)
{
  dms_set_event_report_req_msg_v01 *ptr = node->payload;
  ptr->report_prl_init_valid = TRUE;
  ptr->report_prl_init       = val;
}

// inject
void update_default_qmi_dms_set_event_report_resp_msg(elnptr node)
{
  node->payload = (void *) malloc(sizeof(dms_set_event_report_resp_msg_v01));
  node->payload_len = sizeof(dms_set_event_report_resp_msg_v01);
  memset(node->payload, 0x00, node->payload_len);
  node->type = ril_utf_qmi_response;
  node->i.is_valid = TRUE;
  node->i.msg_id = QMI_DMS_SET_EVENT_REPORT_RESP_V01;
  node->i.qmi_srv_id = QMI_DMS;
  node->i.is_sync_msg = TRUE;


  dms_set_event_report_resp_msg_v01 *ptr = node->payload;
  ptr->resp.error = QMI_ERR_INVALID_INDEX_V01;
  ptr->resp.result= QMI_RESULT_SUCCESS_V01;

  //use default client_id
}

void update_default_qmi_dms_get_operating_mode_req_msg(elnptr node)
{
  node->payload = NULL ;//req msg is empty
  node->payload_len = 0;
  node->type = ril_utf_qmi_request;
  node->e.is_valid = TRUE;
  node->e.msg_id = QMI_DMS_GET_OPERATING_MODE_REQ_V01;
  node->e.status = not_met;

  //use default timeout
}

void update_default_qmi_dms_get_operating_mode_resp_msg(elnptr node)
{
  node->payload = (void *) malloc(sizeof(dms_get_operating_mode_resp_msg_v01));
  node->payload_len = sizeof(dms_get_operating_mode_resp_msg_v01);
  memset(node->payload, 0x00, node->payload_len);
  node->type = ril_utf_qmi_response;
  node->i.is_valid = TRUE;
  node->i.msg_id = QMI_DMS_GET_OPERATING_MODE_RESP_V01;
  node->i.qmi_srv_id = QMI_DMS;
  node->i.is_sync_msg = TRUE;


  dms_get_operating_mode_resp_msg_v01 *ptr = node->payload;
  ptr->resp.error = QMI_ERR_NONE_V01;
  ptr->resp.result= QMI_RESULT_SUCCESS_V01;

  //use default client_id
}

void set_qmi_dms_operating_mode_resp_operating_mode(elnptr node,dms_operating_mode_enum_v01 mode)
{
  dms_get_operating_mode_resp_msg_v01 *ptr = node->payload;
  ptr->operating_mode = mode;
}

void update_default_qmi_pbm_indication_register_req_msg(elnptr node)
{
  node->payload = (void *)malloc(sizeof(pbm_indication_register_req_msg_v01)) ;//req msg is empty
  node->payload_len = sizeof(pbm_indication_register_req_msg_v01);
  node->type = ril_utf_qmi_request;
  node->e.is_valid = TRUE;
  node->e.msg_id = QMI_PBM_INDICATION_REGISTER_REQ_V01;
  node->e.status = not_met;

  //use default timeout
}

void set_qmi_pbm_indication_register_mask(elnptr node, long int mask)
{
  pbm_indication_register_req_msg_v01 *ptr = node->payload;
  ptr->reg_mask = mask;
}
