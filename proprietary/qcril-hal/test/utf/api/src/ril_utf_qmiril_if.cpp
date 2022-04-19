/******************************************************************************
#  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include "ril_utf_qmiril_if.h"
using namespace std;

//========================================================
//  QMI interface implementations
//========================================================

/*========================================================
  QMI_VOICE_INDICATION_REGISTER_REQ
========================================================*/
/*
void ril_utf_qmiril_if::update_default_qmi_voice_ind_reg_req_msg(void)
{
  payload = (void *) malloc(sizeof(voice_indication_register_req_msg_v02));
  payload_len = sizeof(voice_indication_register_req_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_INDICATION_REGISTER_REQ_V02;
  status = not_met;
}

void ril_utf_qmiril_if::set_qmi_voice_ind_req_reg_voice_privacy_events(int val)
{
  voice_indication_register_req_msg_v02 *ptr = (voice_indication_register_req_msg_v02 *) payload;
  ptr->reg_voice_privacy_events_valid = TRUE;
  ptr->reg_voice_privacy_events       = val;
}

void ril_utf_qmiril_if::set_qmi_voice_ind_req_ext_brst_intl_events(int val)
{
  voice_indication_register_req_msg_v02 *ptr = (voice_indication_register_req_msg_v02 *) payload;
  ptr->ext_brst_intl_events_valid = TRUE;
  ptr->ext_brst_intl_events       = val;
}

void ril_utf_qmiril_if::set_qmi_voice_ind_req_speech_events(int val)
{
  voice_indication_register_req_msg_v02 *ptr = (voice_indication_register_req_msg_v02 *) payload;
  ptr->speech_events_valid = TRUE;
  ptr->speech_events       = val;
}
*/
/*========================================================
  QMI_VOICE_INDICATION_REGISTER_RESP
========================================================
void ril_utf_qmiril_if::update_default_qmi_voice_ind_reg_resp_msg(void)
{
  payload = (void *) malloc(sizeof(voice_indication_register_resp_msg_v02));
  payload_len = sizeof(voice_indication_register_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_INDICATION_REGISTER_RESP_V02;
}
*/

void ril_utf_qmiril_if::set_qmi_client_id(int id)
{
    if (id < QCRIL_UNITTEST_MAX_QMI_CLIENT_ID)
        client_id = id;
}

void ril_utf_qmiril_if::set_qmi_voice_indication_register_resp_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  voice_indication_register_resp_msg_v02 *ptr =
    (voice_indication_register_resp_msg_v02 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
  QMI_CTL_GET_CLIENT_ID_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_get_client_id_req_msg(uint8_t qmi_svc_type)
{
  payload_len = 4;
  payload = (void *) malloc(payload_len);
  memset(payload, 0x00, payload_len);

  char *wire_msg = (char*) payload;
  wire_msg[0] = 0x01; //T
  wire_msg[1] = 0x01; //L
  wire_msg[2] = 0x00; //L
  wire_msg[3] = qmi_svc_type; //V

  payload_type = ril_utf_qmi_request;
  service_id = QMI_CTL;
  msg_id = 0x22; //QMI_CTL_GET_CLIENT_ID_REQ, interface file has incorrect id
  status = not_met;
}

void ril_utf_qmiril_if::update_default_qmi_get_client_id_resp_msg(uint8_t qmi_svc_type, uint8_t clnt_id)
{
  payload_len = 4;
  payload = (void *) malloc(payload_len);
  memset(payload, 0x00, payload_len);

  char *wire_msg = (char*) payload;
  // bytes 0 - 6 will be for result
  wire_msg[0] = 0x02;
  wire_msg[1] = 0x04;
  wire_msg[2] = 0x00;
  wire_msg[3] = 0x00;
  wire_msg[4] = 0x00;
  wire_msg[5] = 0x00;
  wire_msg[6] = 0x00;
  wire_msg[7] = 0x01; //T
  wire_msg[8] = 0x02; //L
  wire_msg[9] = 0x00; //L
  wire_msg[10] = qmi_svc_type; //V: qmi_svc_type
  wire_msg[11] = clnt_id; //V: client_id

  payload_type = ril_utf_qmi_response;
  service_id = QMI_CTL;
  msg_id = 0x22; //QMI_CTL_GET_CLIENT_ID_RESP, interface file has incorrect id
  status = not_met;
  verify_payload = false;
}
/*========================================================
  QMI_DMS_SET_EVENT_REPORT_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_set_event_report_req_msg(void)
{
  payload = (void *) malloc(sizeof(dms_set_event_report_req_msg_v01));
  payload_len = sizeof(dms_set_event_report_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_SET_EVENT_REPORT_REQ_V01;
  status = not_met;
}

void ril_utf_qmiril_if::set_qmi_dsm_set_event_report_req_report_oprt_mode_state
  (int val)
{
  dms_set_event_report_req_msg_v01 *ptr =
    (dms_set_event_report_req_msg_v01*) payload;
  ptr->report_oprt_mode_state_valid = TRUE;
  ptr->report_oprt_mode_state       = val;
}

void ril_utf_qmiril_if::set_qmi_dsm_set_event_report_req_report_prl_init
  (int val)
{
  dms_set_event_report_req_msg_v01 *ptr =
    (dms_set_event_report_req_msg_v01*) payload;
  ptr->report_prl_init_valid = TRUE;
  ptr->report_prl_init       = val;
}
void ril_utf_qmiril_if::set_qmi_dms_set_event_report_req_report_device_multisim_info(int val)
{
  dms_set_event_report_req_msg_v01 *ptr =
    (dms_set_event_report_req_msg_v01*) payload;
  ptr->report_device_multisim_info_valid = TRUE;
  ptr->report_device_multisim_info = val;
}

/*========================================================
  QMI_DMS_SET_EVENT_REPORT_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_set_event_report_resp_msg(void)
{
  payload = (void *) malloc(sizeof(dms_set_event_report_resp_msg_v01));
  payload_len = sizeof(dms_set_event_report_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_SET_EVENT_REPORT_RESP_V01;
}
void ril_utf_qmiril_if::set_qmi_dms_set_event_report_resp_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  dms_set_event_report_resp_msg_v01 *ptr =
    (dms_set_event_report_resp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
  QMI_DMS_EVENT_REPORT_IND_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_event_report_ind_v01(void)
{
  payload = (void *) malloc(sizeof(dms_event_report_ind_msg_v01));
  payload_len = sizeof(dms_event_report_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_EVENT_REPORT_IND_V01;
}
void ril_utf_qmiril_if::set_qmi_dms_event_report_ind_v01_operating_mode
  (dms_operating_mode_enum_v01 operating_mode)
{
  dms_event_report_ind_msg_v01 *ptr =
    (dms_event_report_ind_msg_v01 *) payload;
  ptr->operating_mode_valid = TRUE;
  ptr->operating_mode = operating_mode;
}
void ril_utf_qmiril_if::set_qmi_dms_event_report_ind_v01_prl_init(dms_prl_init_enum_v01 prl_init)
{
  dms_event_report_ind_msg_v01 *ptr = (dms_event_report_ind_msg_v01 *) payload;
  ptr->prl_init_valid  = TRUE;
  ptr->prl_init = prl_init;
}
void ril_utf_qmiril_if::set_qmi_dms_event_report_ind_v01_multisim_capability(uint8_t max_subscriptions, uint32_t subs_config_list_len, uint8_t max_active)
{
  dms_event_report_ind_msg_v01 *ptr = (dms_event_report_ind_msg_v01 *) payload;
  ptr->multisim_capability_valid  = TRUE;
  ptr->multisim_capability.max_subscriptions = max_subscriptions;
  ptr->multisim_capability.subscription_config_list_len = subs_config_list_len;
  ptr->multisim_capability.subscription_config_list[0].max_active = max_active;
}
void ril_utf_qmiril_if::set_qmi_dms_event_report_ind_v01_max_active_data_subscriptions(uint8_t max_active_data_subscriptions_valid, uint8_t max_active_data_subscriptions)
{
  dms_event_report_ind_msg_v01 *ptr = (dms_event_report_ind_msg_v01 *) payload;
  ptr->max_active_data_subscriptions_valid  = max_active_data_subscriptions_valid;
  ptr->max_active_data_subscriptions = max_active_data_subscriptions;
}

/*========================================================
  QMI_DMS_IMS_CAPABILITY_IND_V01
========================================================*/
dms_ims_capability_type_v01 *ril_utf_qmiril_if::update_default_qmi_dms_ims_capability_ind_v01(uint32_t ims_capability_len)
{
  payload = (void *) malloc(sizeof(dms_ims_capability_ind_msg_v01));
  payload_len = sizeof(dms_ims_capability_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_IMS_CAPABILITY_IND_V01;
  dms_ims_capability_ind_msg_v01 *ptr = (dms_ims_capability_ind_msg_v01 *) payload;
  ptr->ims_capability_len = ims_capability_len;
  return (ptr->ims_capability);
}

/*========================================================
  QMI_DMS_GET_DEVICE_CAP_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_get_device_cap_req_v01(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_GET_DEVICE_CAP_REQ_V01;
  status = not_met;
}
/*========================================================
  QMI_DMS_GET_DEVICE_CAP_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_get_device_cap_resp_v01(void)
{
  payload = (void *) malloc(sizeof(dms_get_device_cap_resp_msg_v01));
  payload_len = sizeof(dms_get_device_cap_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  msg_id = QMI_DMS_GET_DEVICE_CAP_RESP_V01;
  service_id = QMI_DMS;
}
void ril_utf_qmiril_if::update_default_qmi_dms_get_device_cap_resp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  update_default_qmi_dms_get_device_cap_resp_v01();

  dms_get_device_cap_resp_msg_v01 *ptr = (dms_get_device_cap_resp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}
void ril_utf_qmiril_if::set_default_qmi_dms_get_device_cap_resp_v01_multisim_capability
  (dms_multisim_capability_type_v01 multisim_capability)
{
  dms_get_device_cap_resp_msg_v01 *ptr = (dms_get_device_cap_resp_msg_v01 *) payload;
  ptr->multisim_capability_valid = TRUE;
  ptr->multisim_capability = multisim_capability;
}
void ril_utf_qmiril_if::set_default_qmi_dms_get_device_cap_resp_v01_max_active_data_subscriptions
  (uint8_t max_active_data_subscriptions_valid, uint8_t max_active_data_subscriptions)
{
  dms_get_device_cap_resp_msg_v01 *ptr = (dms_get_device_cap_resp_msg_v01 *) payload;
  ptr->max_active_data_subscriptions_valid = max_active_data_subscriptions_valid;
  ptr->max_active_data_subscriptions = max_active_data_subscriptions;
}
void ril_utf_qmiril_if::set_default_qmi_dms_get_device_cap_resp_v01_device_capabilities(void)
{
  dms_device_capabilities_type_v01 device_capabilities;

  device_capabilities.max_tx_channel_rate = 0x02faf080;
  device_capabilities.max_rx_channel_rate = 0x05f5e100;
  device_capabilities.data_service_capability = DMS_DATA_CAP_SIMUL_CS_AND_PS_V01;
  device_capabilities.sim_capability = DMS_SIM_SUPPORTED_V01;
  device_capabilities.radio_if_list_len = 5;
  device_capabilities.radio_if_list[0] = DMS_RADIO_IF_1X_V01;
  device_capabilities.radio_if_list[1] = DMS_RADIO_IF_1X_EVDO_V01;
  device_capabilities.radio_if_list[2] = DMS_RADIO_IF_GSM_V01;
  device_capabilities.radio_if_list[3] = DMS_RADIO_IF_UMTS_V01;
  device_capabilities.radio_if_list[4] = DMS_RADIO_IF_LTE_V01;

  dms_get_device_cap_resp_msg_v01 *ptr = (dms_get_device_cap_resp_msg_v01 *) payload;
  ptr->device_capabilities = device_capabilities;
  ptr->device_service_capability_valid = TRUE;
  ptr->device_service_capability = DMS_DEVICE_CAP_SIMUL_VOICE_AND_DATA_V01;
  ptr->voice_support_capability_valid = TRUE;
  ptr->voice_support_capability = QMI_DMS_MASK_VOICE_SUPPORT_GW_CSFB_CAPABLE_V01;
  ptr->voice_support_capability |= QMI_DMS_MASK_VOICE_SUPPORT_1x_CSFB_CAPABLE_V01;
  ptr->simul_voice_and_data_capability_valid = TRUE;
  ptr->simul_voice_and_data_capability = QMI_DMS_MASK_SVLTE_CAPABLE_V01;
  ptr->simul_voice_and_data_capability |= QMI_DMS_MASK_SVDO_CAPABLE_V01;

  ptr->subs_device_feature_mode_valid = 1;
  ptr->subs_device_feature_mode_len = 1;
  ptr->subs_device_feature_mode[0] = DMS_DEVICE_SUBS_FEATURE_MODE_SGLTE_V01;
}
void ril_utf_qmiril_if::set_qmi_dms_get_device_cap_resp_v01_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  dms_get_device_cap_resp_msg_v01 *ptr =
    (dms_get_device_cap_resp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}
void ril_utf_qmiril_if::set_default_qmi_dms_get_device_cap_resp_v01_max_ims_instances(uint8_t val)
{
  dms_get_device_cap_resp_msg_v01 *ptr = (dms_get_device_cap_resp_msg_v01 *) payload;
  ptr->max_ims_instances_valid = TRUE;
  ptr->max_ims_instances = val;
}
dms_ims_capability_type_v01 *ril_utf_qmiril_if::set_default_qmi_dms_get_device_cap_resp_v01_ims_capability(uint32_t len)
{
  dms_get_device_cap_resp_msg_v01 *ptr = (dms_get_device_cap_resp_msg_v01 *) payload;
  ptr->ims_capability_valid = TRUE;
  ptr->ims_capability_len = len;
  return (ptr->ims_capability);
}

/*========================================================
  QMI_DMS_GET_OPERATING_MODE_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_get_operating_mode_req_msg(void)
{
  payload = NULL ;//req msg is empty
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_GET_OPERATING_MODE_REQ_V01;
  status = not_met;
}

/*========================================================
  QMI_DMS_GET_OPERATING_MODE_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_get_operating_mode_resp_msg(void)
{
  payload = (void *) malloc(sizeof(dms_get_operating_mode_resp_msg_v01));
  payload_len = sizeof(dms_get_operating_mode_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  msg_id = QMI_DMS_GET_OPERATING_MODE_RESP_V01;
  service_id = QMI_DMS;
}

void ril_utf_qmiril_if::set_qmi_dms_get_operating_mode_resp_operating_mode
  (dms_operating_mode_enum_v01 mode)
{
  dms_get_operating_mode_resp_msg_v01 *ptr =
    (dms_get_operating_mode_resp_msg_v01 *)payload;
  ptr->operating_mode = mode;
}
void ril_utf_qmiril_if::set_qmi_dms_get_operating_mode_resp_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  dms_get_operating_mode_resp_msg_v01 *ptr =
    (dms_get_operating_mode_resp_msg_v01 *)payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
  QMI_DMS_SET_OPERATING_MODE_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_set_operating_mod_req_v01(dms_operating_mode_enum_v01 o_mode)
{
  payload = (void *) malloc(sizeof(dms_set_operating_mode_req_msg_v01));
  payload_len = sizeof(dms_set_operating_mode_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_SET_OPERATING_MODE_REQ_V01;
  status = not_met;

  dms_set_operating_mode_req_msg_v01 *ptr = (dms_set_operating_mode_req_msg_v01*) payload;
  ptr->operating_mode = o_mode;
}

/*========================================================
  QMI_DMS_SET_OPERATING_MODE_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_set_operating_mode_resp_v01(void)
{
  payload = (void *) malloc(sizeof(dms_set_operating_mode_resp_msg_v01));
  payload_len = sizeof(dms_set_operating_mode_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  msg_id = QMI_DMS_SET_OPERATING_MODE_RESP_V01;
  service_id = QMI_DMS;
}
void ril_utf_qmiril_if::set_qmi_dms_set_operating_mode_resp_v01_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  dms_set_operating_mode_resp_msg_v01 *ptr =
    (dms_set_operating_mode_resp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
  QMI_DMS_GET_DEVICE_MFR_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_get_device_mfr_req_v01(void)
{
  payload = NULL ;//req msg is empty
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_GET_DEVICE_MFR_REQ_V01;
  status = not_met;
}

/*========================================================
  QMI_DMS_GET_DEVICE_MFR_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_get_device_mfr_resp_v01(void)
{
  payload = (void *) malloc(sizeof(dms_get_device_mfr_resp_msg_v01));
  payload_len = sizeof(dms_get_device_mfr_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  msg_id = QMI_DMS_GET_DEVICE_MFR_RESP_V01;
  service_id = QMI_DMS;
}
void ril_utf_qmiril_if::set_default_qmi_dms_get_device_mfr_resp_v01_device_manufacturer(void)
{
  dms_get_device_mfr_resp_msg_v01 *ptr =
    (dms_get_device_mfr_resp_msg_v01 *) payload;
  strlcpy(ptr->device_manufacturer, "Umbrella Corp.", sizeof(ptr->device_manufacturer));
}
void ril_utf_qmiril_if::set_qmi_dms_get_device_mfr_resp_v01_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  dms_get_device_mfr_resp_msg_v01 *ptr =
    (dms_get_device_mfr_resp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}
/*========================================================
    QMI_DMS_GET_CURRENT_PRL_INFO_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_get_current_prl_info_req_v01(void)
{
  payload = NULL ;//req msg is empty
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_GET_CURRENT_PRL_INFO_REQ_V01;
  status = not_met;
}

/*=========================================================
   QMI_UIM_SEND_STATUS_REQ_V01
=========================================================*/
uim_send_status_cmd_req_msg_v01* ril_utf_qmiril_if::update_default_qmi_uim_send_status_req_v01(void)
{
  payload = malloc(sizeof(uim_send_status_cmd_req_msg_v01));
  payload_len = sizeof(uim_send_status_cmd_req_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_SEND_STATUS_REQ_V01;
  status = not_met;

  return (uim_send_status_cmd_req_msg_v01 *) payload;
}

/*=========================================================
   QMI_UIM_SEND_STATUS_RESP_V01
=========================================================*/

uim_send_status_cmd_resp_msg_v01* ril_utf_qmiril_if::update_default_qmi_uim_send_status_resp_v01(void)
{
  payload = (uim_send_status_cmd_resp_msg_v01* )malloc(sizeof(uim_send_status_cmd_resp_msg_v01));
  payload_len = sizeof(uim_send_status_cmd_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  msg_id = QMI_UIM_SEND_STATUS_RESP_V01;
  service_id = QMI_UIM;

  return (uim_send_status_cmd_resp_msg_v01*) payload;
}

/*========================================================
  QMI_DMS_GET_CURRENT_PRL_INFO_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_get_current_prl_info_resp_v01(void)
{
  payload = (void *) malloc(sizeof(dms_get_current_prl_info_resp_msg_v01));
  payload_len = sizeof(dms_get_current_prl_info_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  msg_id = QMI_DMS_GET_CURRENT_PRL_INFO_RESP_V01;
  service_id = QMI_DMS;
}
void ril_utf_qmiril_if::set_qmi_dms_get_current_prl_info_resp_v01_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  dms_get_current_prl_info_resp_msg_v01 *ptr =
    (dms_get_current_prl_info_resp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
    QMI_IMS_SETTINGS_GET_SUPPORTED_MSGS_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_supported_msgs_req(void)
{
  payload = NULL ;//req msg is empty
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_SUPPORTED_MSGS_REQ_V01;
  status = not_met;
}
/*========================================================
    QMI_IMS_SETTINGS_GET_SUPPORTED_MSGS_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_supported_msgs_resp(void)
{
  payload = (void *) malloc(sizeof(qmi_get_supported_msgs_resp_v01));
  payload_len = sizeof(qmi_get_supported_msgs_resp_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  msg_id = QMI_IMS_SETTINGS_GET_SUPPORTED_MSGS_RESP_V01;
  service_id = QMI_IMSS;

  qmi_get_supported_msgs_resp_v01 *qmi_resp = (qmi_get_supported_msgs_resp_v01 *) payload;
  qmi_resp->supported_msgs_valid = TRUE;
  int message_id = QMI_IMS_SETTINGS_SET_IMS_SERVICE_ENABLE_CONFIG_REQ_V01;
  uint32_t index = message_id/8;
  uint8_t bit_position = message_id - index * 8;
  uint8_t bit_position_mask = 0x01 << bit_position;
  qmi_resp->supported_msgs[index] = bit_position_mask;
  qmi_resp->supported_msgs_len = index+1;
}

/*========================================================
    QMI_IMS_SETTINGS_GET_SUPPORTED_FIELDS_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_supported_fields_req(void)
{
  payload = (void *) malloc(sizeof(qmi_get_supported_fields_req_v01));
  payload_len = sizeof(qmi_get_supported_fields_req_v01);
  memset(payload, 0x00, payload_len);
  qmi_get_supported_fields_req_v01 *qmi_req = (qmi_get_supported_fields_req_v01 *) payload;
  qmi_req->msg_id = QMI_IMS_SETTINGS_SET_IMS_SERVICE_ENABLE_CONFIG_REQ_V01;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_SUPPORTED_FIELDS_REQ_V01;
  status = not_met;
}

uint8_t imss_0x008F_supported_fields[QMI_GET_SUPPORTED_FIELDS_ARRAY_MAX_V01] =
        {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/*========================================================
    QMI_IMS_SETTINGS_GET_SUPPORTED_FIELDS_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_supported_fields_resp(void)
{
  payload = (void *) malloc(sizeof(qmi_get_supported_fields_resp_v01));
  payload_len = sizeof(qmi_get_supported_fields_resp_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  msg_id = QMI_IMS_SETTINGS_GET_SUPPORTED_FIELDS_RESP_V01;
  service_id = QMI_IMSS;

  qmi_get_supported_fields_resp_v01 *qmi_resp = (qmi_get_supported_fields_resp_v01 *) payload;
  qmi_resp->request_fields_valid = TRUE;

#if 0
  //Bit position of WFC roaming mode preference
  int field_id = 20;
  uint32_t index = field_id/8;
  uint8_t bit_position = field_id - index * 8;
  uint8_t bit_position_mask = 0x01 << bit_position;
  qmi_resp->request_fields[index] |= bit_position_mask;
  qmi_resp->request_fields_len = index+1;

  // call_mode_preference_ext
  field_id = 22;
  index = field_id/8;
  bit_position = field_id - index * 8;
  bit_position_mask = 0x01 << bit_position;
  qmi_resp->request_fields[index] |= bit_position_mask;
  qmi_resp->request_fields_len = index+1;
#endif
  memcpy(qmi_resp->request_fields, imss_0x008F_supported_fields,
         sizeof(qmi_resp->request_fields));
  qmi_resp->request_fields_len = QMI_GET_SUPPORTED_FIELDS_ARRAY_MAX_V01;

  // reset
  memset(imss_0x008F_supported_fields, 0xFF, sizeof(imss_0x008F_supported_fields));
}

/*========================================================
  QMI_DMS_TRIGGER_MODEM_ACTIVITY_INFO_CALCULATION_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_trigger_modem_activity_info_calculation_req(int enable_statistics_valid , int enable_statistics)
{
  payload = (void *) malloc(sizeof(dms_config_modem_activity_info_req_msg_v01));
  payload_len = sizeof(dms_config_modem_activity_info_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_TRIGGER_MODEM_ACTIVITY_INFO_CALCULATION_REQ_V01;
  if (enable_statistics)
  {
    dms_config_modem_activity_info_req_msg_v01* ptr = (dms_config_modem_activity_info_req_msg_v01*)payload;
    ptr->enable_statistics_valid = enable_statistics_valid;
    ptr->enable_statistics = enable_statistics;
  }
}

/*========================================================
  QMI_DMS_TRIGGER_MODEM_ACTIVITY_INFO_CALCULATION_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_trigger_modem_activity_info_calculation_resp(void)
{
  payload = (void *) malloc(sizeof(dms_config_modem_activity_info_resp_msg_v01));
  payload_len = sizeof(dms_config_modem_activity_info_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  msg_id = QMI_DMS_TRIGGER_MODEM_ACTIVITY_INFO_CALCULATION_RESP_V01;
  service_id = QMI_DMS;
}

/*========================================================
  QMI_DMS_GET_MODEM_ACTIVITY_INFO_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_get_modem_activity_info_req(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_GET_MODEM_ACTIVITY_INFO_REQ_V01;
}

/*========================================================
  QMI_DMS_GET_MODEM_ACTIVITY_INFO_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_get_modem_activity_info_resp(void)
{
  payload = (void *) malloc(sizeof(dms_get_modem_activity_info_resp_msg_v01 ));
  payload_len = sizeof(dms_get_modem_activity_info_resp_msg_v01 );
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  msg_id = QMI_DMS_GET_MODEM_ACTIVITY_INFO_RESP_V01;
  service_id = QMI_DMS;
}

/*========================================================
  QMI_DMS_MODEM_ACTIVITY_INFO_IND_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_get_modem_activity_info_ind()
{
  payload = (void *) malloc(sizeof(dms_activity_info_ind_msg_v01));
  payload_len = sizeof(dms_activity_info_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_MODEM_ACTIVITY_INFO_IND_V01;
}

void ril_utf_qmiril_if::set_qmi_dms_get_modem_activity_info_ind_is_newly_calculated_values
  (uint8_t is_newly_calculated_values)
{
  dms_activity_info_ind_msg_v01 *ptr =
    (dms_activity_info_ind_msg_v01 *) payload;
  ptr->is_newly_calculated_values_valid = TRUE;
  ptr->is_newly_calculated_values = is_newly_calculated_values;
}

/*========================================================
  QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_system_selection_preference_req_msg(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01;
  status = not_met;
}

/*========================================================
  QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_system_selection_preference_resp_msg(void)
{
  payload = (void *) malloc(sizeof(nas_get_system_selection_preference_resp_msg_v01));
  payload_len = sizeof(nas_get_system_selection_preference_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01;
}
/*
void ril_utf_qmiril_if::set_qmi_nas_get_system_selection_preference_resp_emergency_mode
  (uint8_t emergency_mode)
{
  nas_get_system_selection_preference_resp_msg_v01 *ptr =
    (nas_get_system_selection_preference_resp_msg_v01 *) payload;
  ptr->emergency_mode_valid = TRUE;
  ptr->emergency_mode = emergency_mode;
}

void ril_utf_qmiril_if::set_qmi_nas_get_system_selection_preference_resp_mode_pref
  (mode_pref_mask_type_v01 mode_pref)
{
  nas_get_system_selection_preference_resp_msg_v01 *ptr =
    (nas_get_system_selection_preference_resp_msg_v01 *) payload;
  ptr->mode_pref_valid = TRUE;
  ptr->mode_pref = mode_pref;
}

void ril_utf_qmiril_if::set_qmi_nas_get_system_selection_preference_resp_band_pref
  (nas_band_pref_mask_type_v01 band_pref)
{
  nas_get_system_selection_preference_resp_msg_v01 *ptr =
    (nas_get_system_selection_preference_resp_msg_v01 *) payload;
  ptr->band_pref_valid = TRUE;
  ptr->band_pref = band_pref;
}
*/

/*
void ril_utf_qmiril_if::set_qmi_nas_get_system_selection_preference_resp_prl_pref
  (nas_prl_pref_enum_v01 prl_pref)
{
  nas_get_system_selection_preference_resp_msg_v01 *ptr =
    (nas_get_system_selection_preference_resp_msg_v01 *) payload;
  ptr->prl_pref_valid = TRUE;
  ptr->prl_pref = prl_pref;
}
void ril_utf_qmiril_if::set_qmi_nas_get_system_selection_preference_resp_roam_pref
  (nas_roam_pref_enum_v01 roam_pref)
{
  nas_get_system_selection_preference_resp_msg_v01 *ptr =
    (nas_get_system_selection_preference_resp_msg_v01 *) payload;
  ptr->roam_pref_valid = TRUE;
  ptr->roam_pref = roam_pref;
}
*/
void ril_utf_qmiril_if::set_qmi_nas_get_system_selection_preference_resp_band_pref_ext
  (uint64_t band_pref_ext)
{
  nas_get_system_selection_preference_resp_msg_v01 *ptr =
    (nas_get_system_selection_preference_resp_msg_v01 *) payload;
  ptr->band_pref_ext_valid = TRUE;
  ptr->band_pref_ext = band_pref_ext;
}
/*
void ril_utf_qmiril_if::set_qmi_nas_get_system_selection_preference_resp_net_sel_pref
  (nas_net_sel_pref_enum_v01 net_sel_pref)
{
  nas_get_system_selection_preference_resp_msg_v01 *ptr =
    (nas_get_system_selection_preference_resp_msg_v01 *) payload;
  ptr->net_sel_pref_valid = TRUE;
  ptr->net_sel_pref = net_sel_pref;
}
*/
void ril_utf_qmiril_if::set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref
  (nas_srv_domain_pref_enum_type_v01 srv_domain_pref)
{
  nas_get_system_selection_preference_resp_msg_v01 *ptr =
    (nas_get_system_selection_preference_resp_msg_v01 *) payload;
  ptr->srv_domain_pref_valid = TRUE;
  ptr->srv_domain_pref = srv_domain_pref;
}
void ril_utf_qmiril_if::set_qmi_nas_get_system_selection_preference_resp_gw_acq_order_pref
  (nas_gw_acq_order_pref_enum_type_v01 gw_acq_order_pref)
{
  nas_get_system_selection_preference_resp_msg_v01 *ptr =
    (nas_get_system_selection_preference_resp_msg_v01 *) payload;
  ptr->gw_acq_order_pref_valid = TRUE;
  ptr->gw_acq_order_pref = gw_acq_order_pref;
}
void ril_utf_qmiril_if::set_qmi_nas_get_system_selection_preference_resp_tdscdma_band_pref
  (nas_tdscdma_band_pref_mask_type_v01 tdscdma_band_pref)
{
  nas_get_system_selection_preference_resp_msg_v01 *ptr =
    (nas_get_system_selection_preference_resp_msg_v01 *) payload;
  ptr->tdscdma_band_pref_valid = TRUE;
  ptr->tdscdma_band_pref = tdscdma_band_pref;
}
void ril_utf_qmiril_if::set_default_qmi_nas_get_system_selection_preference_resp_acq_order(void)
{
  nas_get_system_selection_preference_resp_msg_v01 *ptr =
    (nas_get_system_selection_preference_resp_msg_v01 *) payload;
  ptr->acq_order_valid = TRUE;
  ptr->acq_order_len = 6;
  ptr->acq_order[0] = NAS_RADIO_IF_CDMA_1X_V01;
  ptr->acq_order[0] = NAS_RADIO_IF_GSM_V01;
  ptr->acq_order[0] = NAS_RADIO_IF_UMTS_V01;
  ptr->acq_order[0] = NAS_RADIO_IF_CDMA_1XEVDO_V01;
  ptr->acq_order[0] = NAS_RADIO_IF_LTE_V01;
  ptr->acq_order[0] = NAS_RADIO_IF_TDSCDMA_V01;
}
/*
void ril_utf_qmiril_if::set_qmi_nas_get_system_selection_preference_resp_srv_reg_restriction
  (nas_srv_reg_restriction_enum_v01 srv_reg_restriction)
{
  nas_get_system_selection_preference_resp_msg_v01 *ptr =
    (nas_get_system_selection_preference_resp_msg_v01 *) payload;
  ptr->srv_reg_restriction_valid = TRUE;
  ptr->srv_reg_restriction_valid = srv_reg_restriction;
}
*/
void ril_utf_qmiril_if::set_qmi_nas_get_system_selection_preference_resp_usage_setting
  (nas_usage_pref_enum_v01 usage_setting)
{
  nas_get_system_selection_preference_resp_msg_v01 *ptr =
    (nas_get_system_selection_preference_resp_msg_v01 *) payload;
  ptr->usage_setting_valid = TRUE;
  ptr->usage_setting = usage_setting;
}
void ril_utf_qmiril_if::set_qmi_nas_get_system_selection_preference_resp_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  nas_get_system_selection_preference_resp_msg_v01 *ptr =
    (nas_get_system_selection_preference_resp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}
#ifdef RIL_UTF_L_MASTER
void ril_utf_qmiril_if::set_qmi_nas_get_system_selection_preference_resp_rat_disabled_mask(mode_pref_mask_type_v01 mask)
{
  nas_get_system_selection_preference_resp_msg_v01 *ptr =
    (nas_get_system_selection_preference_resp_msg_v01 *) payload;
  ptr->rat_disabled_mask_valid = TRUE;
  ptr->rat_disabled_mask = mask;
}
#endif

/*========================================================
  QMI_NAS_SET_EVENT_REPORT_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_set_event_report_req_msg_v01(void)
{
  payload = (void *) malloc(sizeof(nas_set_event_report_req_msg_v01));
  payload_len = sizeof(nas_set_event_report_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_SET_EVENT_REPORT_REQ_MSG_V01;
  status = not_met;
}
void ril_utf_qmiril_if::set_default_qmi_nas_set_event_report_req_signal_strength(void)
{
  nas_set_event_report_req_msg_v01 *ptr =
    (nas_set_event_report_req_msg_v01 *) payload;
  ptr->signal_strength.report_signal_strength = 1;
  ptr->signal_strength.report_signal_strength_threshold_list_len = 2;
  ptr->signal_strength.report_signal_strength_threshold_list[0]  = 1;
  ptr->signal_strength.report_signal_strength_threshold_list[0]  = -1;
  ptr->signal_strength_valid = TRUE;
}

/*
void ril_utf_qmiril_if::set_qmi_nas_set_event_report_req_report_rf_band_info
  (uint8_t report_rf_band_info)
{
  nas_set_event_report_req_msg_v01 *ptr =
    (nas_set_event_report_req_msg_v01 *) payload;
  ptr->report_rf_band_info_valid = TRUE;
  ptr->report_rf_band_info = report_rf_band_info;
}
void ril_utf_qmiril_if::set_qmi_nas_set_event_report_req_report_reg_reject
  (uint8_t report_reg_reject)
{
  nas_set_event_report_req_msg_v01 *ptr =
    (nas_set_event_report_req_msg_v01 *) payload;
  ptr->report_reg_reject_valid = TRUE;
  ptr->report_reg_reject = report_reg_reject;
}
*/
void ril_utf_qmiril_if::set_qmi_nas_set_event_report_req_rssi_indicator
  (uint8_t report_rssi, uint8_t rssi_delta)
{
  nas_set_event_report_req_msg_v01 *ptr =
    (nas_set_event_report_req_msg_v01 *) payload;
  ptr->rssi_indicator_valid = TRUE;
  ptr->rssi_indicator.report_rssi = report_rssi;
  ptr->rssi_indicator.rssi_delta = rssi_delta;
}
void ril_utf_qmiril_if::set_qmi_nas_set_event_report_req_ecio_indicator
  (uint8_t report_ecio, uint8_t ecio_delta)
{
  nas_set_event_report_req_msg_v01 *ptr =
    (nas_set_event_report_req_msg_v01 *) payload;
  ptr->ecio_indicator_valid = TRUE;
  ptr->ecio_indicator.report_ecio = report_ecio;
  ptr->ecio_indicator.ecio_delta = ecio_delta;
}
void ril_utf_qmiril_if::set_qmi_nas_set_event_report_req_io_indicator
  (uint8_t report_io, uint8_t io_delta)
{
  nas_set_event_report_req_msg_v01 *ptr =
    (nas_set_event_report_req_msg_v01 *) payload;
  ptr->io_indicator_valid = TRUE;
  ptr->io_indicator.report_io = report_io;
  ptr->io_indicator.io_delta = io_delta;
}
void ril_utf_qmiril_if::set_qmi_nas_set_event_report_req_sinr_indicator
  (uint8_t report_sinr, uint8_t sinr_delta)
{
  nas_set_event_report_req_msg_v01 *ptr =
    (nas_set_event_report_req_msg_v01 *) payload;
  ptr->sinr_indicator_valid = TRUE;
  ptr->sinr_indicator.report_sinr = report_sinr;
  ptr->sinr_indicator.sinr_delta = sinr_delta;
}
/*
void ril_utf_qmiril_if::set_qmi_nas_set_event_report_req_report_error_rate
  (uint8_t report_error_rate)
{
  nas_set_event_report_req_msg_v01 *ptr =
    (nas_set_event_report_req_msg_v01 *) payload;
  ptr->report_error_rate_valid = TRUE;
  ptr->report_error_rate = report_error_rate;
}
*/
void ril_utf_qmiril_if::set_qmi_nas_set_event_report_req_rsrq_indicator
  (uint8_t report_rsrq, uint8_t rsrq_delta)
{
  nas_set_event_report_req_msg_v01 *ptr =
    (nas_set_event_report_req_msg_v01 *) payload;
  ptr->rsrq_indicator_valid = TRUE;
  ptr->rsrq_indicator.report_rsrq = report_rsrq;
  ptr->rsrq_indicator.rsrq_delta = rsrq_delta;
}

/*========================================================
  QMI_NAS_SET_EVENT_REPORT_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_set_event_report_resp_msg_v01(void)
{
  payload = (void *) malloc(sizeof(nas_set_event_report_resp_msg_v01));
  payload_len = sizeof(nas_set_event_report_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_SET_EVENT_REPORT_RESP_MSG_V01;
}
void ril_utf_qmiril_if::set_qmi_nas_set_event_report_resp_msg_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  nas_set_event_report_resp_msg_v01 *ptr = (nas_set_event_report_resp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
  QMI_NAS_CONFIG_SIG_INFO_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_config_sig_info_req_msg_v01(void)
{
  payload = (void *) malloc(sizeof(nas_config_sig_info_req_msg_v01));
  payload_len = sizeof(nas_config_sig_info_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_CONFIG_SIG_INFO_REQ_MSG_V01;
  status = not_met;
//TODO: remove and add sane values
  verify_payload = false;
}

/*========================================================
  QMI_NAS_CONFIG_SIG_INFO_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_config_sig_info_resp_msg_v01(void)
{
  payload = (void *) malloc(sizeof(nas_config_sig_info_resp_msg_v01));
  payload_len = sizeof(nas_config_sig_info_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_CONFIG_SIG_INFO_RESP_MSG_V01;
}
void ril_utf_qmiril_if::set_qmi_nas_config_sig_info_resp_msg_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  nas_config_sig_info_resp_msg_v01 *ptr = (nas_config_sig_info_resp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
  QMI_NAS_CONFIG_SIG_INFO2_REQ_MSG_V01
========================================================*/

void ril_utf_qmiril_if::update_default_qmi_nas_config_sig_info2_req_msg_v01(void)
{
  payload = (void *) malloc(sizeof(nas_config_sig_info2_req_msg_v01));
  payload_len = sizeof(nas_config_sig_info2_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_CONFIG_SIG_INFO2_REQ_MSG_V01;
  status = not_met;
}

void ril_utf_qmiril_if::set_default_qmi_nas_config_sig_info2_req_msg_v01_cdma_ecio_delta(uint16_t cdma_ecio_delta)
{
  nas_config_sig_info2_req_msg_v01 *ptr = (nas_config_sig_info2_req_msg_v01 *) payload;
  ptr->cdma_ecio_delta_valid = TRUE;
  ptr->cdma_ecio_delta = cdma_ecio_delta;
}
void ril_utf_qmiril_if::set_default_qmi_nas_config_sig_info2_req_msg_v01_cdma_rssi_delta(uint16_t cdma_rssi_delta)
{
  nas_config_sig_info2_req_msg_v01 *ptr = (nas_config_sig_info2_req_msg_v01 *) payload;
  ptr->cdma_rssi_delta_valid = TRUE;
  ptr->cdma_rssi_delta = cdma_rssi_delta;
}

void ril_utf_qmiril_if::set_default_qmi_nas_config_sig_info2_req_msg_v01_gsm_rssi_delta(uint16_t gsm_rssi_delta)
{
  nas_config_sig_info2_req_msg_v01 *ptr = (nas_config_sig_info2_req_msg_v01 *) payload;
  ptr->gsm_rssi_delta_valid = TRUE;
  ptr->gsm_rssi_delta = gsm_rssi_delta;
}

void ril_utf_qmiril_if::set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_ecio_delta(uint16_t hdr_ecio_delta)
{
  nas_config_sig_info2_req_msg_v01 *ptr = (nas_config_sig_info2_req_msg_v01 *) payload;
  ptr->hdr_ecio_delta_valid = TRUE;
  ptr->hdr_ecio_delta = hdr_ecio_delta;
}
void ril_utf_qmiril_if::set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_io_delta(uint16_t hdr_io_delta)
{
  nas_config_sig_info2_req_msg_v01 *ptr = (nas_config_sig_info2_req_msg_v01 *) payload;
  ptr->hdr_io_delta_valid = TRUE;
  ptr->hdr_io_delta = hdr_io_delta;
}

void ril_utf_qmiril_if::set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_rssi_delta(uint16_t hdr_rssi_delta)
{
  nas_config_sig_info2_req_msg_v01 *ptr = (nas_config_sig_info2_req_msg_v01 *) payload;
  ptr->hdr_rssi_delta_valid = TRUE;
  ptr->hdr_rssi_delta = hdr_rssi_delta;
}

void ril_utf_qmiril_if::set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_sinr_delta(uint16_t hdr_sinr_delta)
{
  nas_config_sig_info2_req_msg_v01 *ptr = (nas_config_sig_info2_req_msg_v01 *) payload;
  ptr->hdr_sinr_delta_valid = TRUE;
  ptr->hdr_sinr_delta = hdr_sinr_delta;
}
void ril_utf_qmiril_if::set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rsrp_delta(uint16_t lte_rsrp_delta)
{
  nas_config_sig_info2_req_msg_v01 *ptr = (nas_config_sig_info2_req_msg_v01 *) payload;
  ptr->lte_rsrp_delta_valid = TRUE;
  ptr->lte_rsrp_delta = lte_rsrp_delta;
}
void ril_utf_qmiril_if::set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rsrq_delta(uint16_t lte_rsrq_delta)
{
  nas_config_sig_info2_req_msg_v01 *ptr = (nas_config_sig_info2_req_msg_v01 *) payload;
  ptr->lte_rsrq_delta_valid = TRUE;
  ptr->lte_rsrq_delta = lte_rsrq_delta;
}
void ril_utf_qmiril_if::set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rssi_delta(uint16_t lte_rssi_delta)
{
  nas_config_sig_info2_req_msg_v01 *ptr = (nas_config_sig_info2_req_msg_v01 *) payload;
  ptr->lte_rssi_delta_valid = TRUE;
  ptr->lte_rssi_delta = lte_rssi_delta;
}
void ril_utf_qmiril_if::set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_snr_delta(uint16_t lte_snr_delta)
{
  nas_config_sig_info2_req_msg_v01 *ptr = (nas_config_sig_info2_req_msg_v01 *) payload;
  ptr->lte_snr_delta_valid = TRUE;
  ptr->lte_snr_delta = lte_snr_delta;
}
void ril_utf_qmiril_if::set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rpt_rate_avg_period(uint16_t lte_rpt_rate, uint16_t lte_avg_period)
{
  nas_config_sig_info2_req_msg_v01 *ptr = (nas_config_sig_info2_req_msg_v01 *) payload;
  ptr->lte_sig_rpt_config_valid = TRUE;
  ptr->lte_sig_rpt_config.rpt_rate = (nas_lte_sig_rpt_rate_enum_type_v01)lte_rpt_rate;
  ptr->lte_sig_rpt_config.avg_period = (nas_lte_sig_avg_prd_enum_type_v01)lte_avg_period;
}
void ril_utf_qmiril_if::set_default_qmi_nas_config_sig_info2_req_msg_v01_wcdma_ecio_delta(uint16_t wcdma_ecio_delta)
{
  nas_config_sig_info2_req_msg_v01 *ptr = (nas_config_sig_info2_req_msg_v01 *) payload;
  ptr->wcdma_ecio_delta_valid = TRUE;
  ptr->wcdma_ecio_delta = wcdma_ecio_delta;
}
void ril_utf_qmiril_if::set_default_qmi_nas_config_sig_info2_req_msg_v01_wcdma_rssi_delta(uint16_t wcdma_rssi_delta)
{
  nas_config_sig_info2_req_msg_v01 *ptr = (nas_config_sig_info2_req_msg_v01 *) payload;
  ptr->wcdma_rssi_delta_valid = TRUE;
  ptr->wcdma_rssi_delta = wcdma_rssi_delta;
}
void ril_utf_qmiril_if::set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_ecio_delta(uint16_t tdscdma_ecio_delta)
{
  nas_config_sig_info2_req_msg_v01 *ptr = (nas_config_sig_info2_req_msg_v01 *) payload;
  ptr->tdscdma_ecio_delta_valid = TRUE;
  ptr->tdscdma_ecio_delta = tdscdma_ecio_delta;
}
void ril_utf_qmiril_if::set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_rscp_delta(uint16_t tdscdma_rscp_delta)
{
  nas_config_sig_info2_req_msg_v01 *ptr = (nas_config_sig_info2_req_msg_v01 *) payload;
  ptr->tdscdma_rscp_delta_valid = TRUE;
  ptr->tdscdma_rscp_delta = tdscdma_rscp_delta;
}
void ril_utf_qmiril_if::set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_rssi_delta(uint16_t tdscdma_rssi_delta)
{
  nas_config_sig_info2_req_msg_v01 *ptr = (nas_config_sig_info2_req_msg_v01 *) payload;
  ptr->tdscdma_rssi_delta_valid = TRUE;
  ptr->tdscdma_rssi_delta = tdscdma_rssi_delta;
}
void ril_utf_qmiril_if::set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_sinr_delta(uint16_t tdscdma_sinr_delta)
{
  nas_config_sig_info2_req_msg_v01 *ptr = (nas_config_sig_info2_req_msg_v01 *) payload;
  ptr->tdscdma_sinr_delta_valid = TRUE;
  ptr->tdscdma_sinr_delta = tdscdma_sinr_delta;
}
int16_t* ril_utf_qmiril_if::set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_snr_threshold_list(uint32_t lte_snr_threshold_list_len)
{
  nas_config_sig_info2_req_msg_v01 *ptr = (nas_config_sig_info2_req_msg_v01 *) payload;
  ptr->lte_snr_threshold_list_valid = TRUE;
  ptr->lte_snr_threshold_list_len = lte_snr_threshold_list_len;
  return (ptr->lte_snr_threshold_list) ;
}

void ril_utf_qmiril_if::set_default_qmi_nas_config_sig_info2_req_msg_v01_nr5g_rsrp_delta(uint16_t nr5g_rsrp_delta)
{
  nas_config_sig_info2_req_msg_v01 *ptr = (nas_config_sig_info2_req_msg_v01 *) payload;
  ptr->nr5g_rsrp_delta_valid = TRUE;
  ptr->nr5g_rsrp_delta = nr5g_rsrp_delta;
}

void ril_utf_qmiril_if::set_default_qmi_nas_config_sig_info2_req_msg_v01_nr5g_snr_delta(uint16_t nr5g_snr_delta)
{
  nas_config_sig_info2_req_msg_v01 *ptr = (nas_config_sig_info2_req_msg_v01 *) payload;
  ptr->nr5g_snr_delta_valid = TRUE;
  ptr->nr5g_snr_delta = nr5g_snr_delta;
}

void ril_utf_qmiril_if::set_default_qmi_nas_config_sig_info2_req_msg_v01_nr5g_rpt_rate_avg_period(
  nas_nr5g_common_rssi_reporting_rate_enum_type_v01 nr5g_rpt_rate,
  nas_nr5g_common_rssi_averaging_period_enum_type_v01 nr5g_avg_period)
{
  nas_config_sig_info2_req_msg_v01 *ptr = (nas_config_sig_info2_req_msg_v01 *) payload;
  ptr->nr5g_sig_rpt_config_valid = TRUE;
  ptr->nr5g_sig_rpt_config.rpt_rate = nr5g_rpt_rate;
  ptr->nr5g_sig_rpt_config.avg_period = nr5g_avg_period;
}

/*========================================================
  QMI_NAS_CONFIG_SIG_INFO2_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_config_sig_info2_resp_msg_v01(void)
{
  payload = (void *) malloc(sizeof(nas_config_sig_info2_resp_msg_v01));
  payload_len = sizeof(nas_config_sig_info2_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_CONFIG_SIG_INFO2_RESP_MSG_V01;
}
void ril_utf_qmiril_if::set_qmi_nas_config_sig_info2_resp_msg_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  nas_config_sig_info2_resp_msg_v01 *ptr = (nas_config_sig_info2_resp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}


/*========================================================
  QMI_NAS_GET_ERR_RATE_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_err_rate_req_msg_v01(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_ERR_RATE_REQ_MSG_V01;
  status = not_met;
}

/*========================================================
  QMI_NAS_GET_ERR_RATE_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_err_rate_resp_msg_v01(void)
{
  payload = (void *) malloc(sizeof(nas_get_err_rate_resp_msg_v01));
  payload_len = sizeof(nas_get_err_rate_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_ERR_RATE_RESP_MSG_V01;

  nas_get_err_rate_resp_msg_v01 *ptr =
    (nas_get_err_rate_resp_msg_v01 *) payload;
  ptr->wcdma_block_err_rate_valid = TRUE;
  ptr->wcdma_block_err_rate = 10;
}
void ril_utf_qmiril_if::set_qmi_nas_get_err_rate_resp_msg_v01_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  nas_get_err_rate_resp_msg_v01 *ptr =
    (nas_get_err_rate_resp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

#ifndef RIL_UTF_IOE
/*========================================================
  QMI_NAS_GET_EMBMS_SIG_EXT_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_embms_sig_ext_req(void)
{
  payload = (void*) malloc(sizeof(nas_get_embms_sig_req_msg_v01));
  payload_len = sizeof(nas_get_embms_sig_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_EMBMS_SIG_EXT_REQ_MSG_V01;
}
void ril_utf_qmiril_if::set_qmi_nas_get_embms_sig_ext_req_trace_id(int16_t trace_id)
{
  nas_get_embms_sig_req_msg_v01 *ptr = (nas_get_embms_sig_req_msg_v01 *) payload;
  ptr->trace_id_valid = TRUE;
  ptr->trace_id = trace_id;
}
/*========================================================
  QMI_NAS_GET_EMBMS_SIG_EXT_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_embms_sig_ext_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_get_embms_sig_ext_resp_msg_v01));
  payload_len = sizeof(nas_get_embms_sig_ext_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_EMBMS_SIG_EXT_RESP_MSG_V01;

  nas_get_embms_sig_ext_resp_msg_v01 *ptr = (nas_get_embms_sig_ext_resp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}
void ril_utf_qmiril_if::set_qmi_nas_get_embms_sig_ext_resp_trace_id(int16_t trace_id)
{
  nas_get_embms_sig_ext_resp_msg_v01 *ptr = (nas_get_embms_sig_ext_resp_msg_v01 *) payload;
  ptr->trace_id_valid = TRUE;
  ptr->trace_id = trace_id;
}
nas_lte_embms_signal_type_v01 *ril_utf_qmiril_if::set_qmi_nas_get_embms_sig_ext_resp_snr_and_tmgi_list(int len)
{
  nas_get_embms_sig_ext_resp_msg_v01 *ptr = (nas_get_embms_sig_ext_resp_msg_v01 *) payload;
  ptr->snr_and_tmgi_list_valid = TRUE;
  ptr->snr_and_tmgi_list_len = len;
  return ptr->snr_and_tmgi_list;
}

/*========================================================
  QMI_NAS_GET_EMBMS_STATUS_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_embms_status_req_msg_v01(void)
{
  payload = (void *) malloc(sizeof(nas_get_embms_status_req_msg_v01));
  payload_len = sizeof(nas_get_embms_status_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_EMBMS_STATUS_REQ_MSG_V01;
}

/*========================================================
  QMI_NAS_GET_EMBMS_STATUS_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_embms_status_resp_msg_v01(void)
{
  payload = (void *) malloc(sizeof(nas_get_embms_status_resp_msg_v01));
  payload_len = sizeof(nas_get_embms_status_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_EMBMS_STATUS_RESP_MSG_V01;

}
void ril_utf_qmiril_if::set_qmi_nas_get_embms_status_resp_result
(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  nas_get_embms_status_resp_msg_v01 *ptr =
    (nas_get_embms_status_resp_msg_v01*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}
#endif
/*========================================================
  QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_indication_register_req_msg_v01(void)
{
  payload = (void *) malloc(sizeof(nas_indication_register_req_msg_v01));
  payload_len = sizeof(nas_indication_register_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  verify_payload = false;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01;
  status = not_met;
}
void ril_utf_qmiril_if::set_qmi_nas_indication_register_req_msg_reg_sys_sel_pref
  (uint8_t reg_sys_sel_pref)
{
  nas_indication_register_req_msg_v01 *ptr =
    (nas_indication_register_req_msg_v01 *) payload;
  ptr->reg_sys_sel_pref_valid = TRUE;
  ptr->reg_sys_sel_pref = reg_sys_sel_pref;
}

void ril_utf_qmiril_if::set_qmi_nas_indication_register_req_msg_reg_sys_info_ind (uint8_t reg_sys_info_ind)
{
  nas_indication_register_req_msg_v01 *ptr = (nas_indication_register_req_msg_v01 *) payload;
  ptr->sys_info_valid = TRUE;
  ptr->sys_info = reg_sys_info_ind;
}

void ril_utf_qmiril_if::set_qmi_nas_indication_register_req_msg_dual_standby_pref
  (uint8_t dual_standby_pref)
{
  nas_indication_register_req_msg_v01 *ptr =
    (nas_indication_register_req_msg_v01 *) payload;
  ptr->dual_standby_pref_valid = TRUE;
  ptr->dual_standby_pref = dual_standby_pref;
}
void ril_utf_qmiril_if::set_qmi_nas_indication_register_req_msg_subscription_info
  (uint8_t subscription_info)
{
  nas_indication_register_req_msg_v01 *ptr =
    (nas_indication_register_req_msg_v01 *) payload;
  ptr->subscription_info_valid = TRUE;
  ptr->subscription_info = subscription_info;
}
void ril_utf_qmiril_if::set_qmi_nas_indication_register_req_msg_reg_network_time
  (uint8_t reg_network_time)
{
  nas_indication_register_req_msg_v01 *ptr =
    (nas_indication_register_req_msg_v01 *) payload;
  ptr->reg_network_time_valid = TRUE;
  ptr->reg_network_time = reg_network_time;
}
void ril_utf_qmiril_if::set_qmi_nas_indication_register_reg_msg_reg_rtre_cfg
  (uint8_t reg_rtre_cfg)
{
  nas_indication_register_req_msg_v01 *ptr =
    (nas_indication_register_req_msg_v01 *) payload;
  ptr->reg_rtre_cfg_valid = TRUE;
  ptr->reg_rtre_cfg = reg_rtre_cfg;
}
void ril_utf_qmiril_if::set_qmi_nas_indication_register_reg_msg_reg_embms_status
  (uint8_t reg_embms_status)
{
  nas_indication_register_req_msg_v01 *ptr =
    (nas_indication_register_req_msg_v01 *) payload;
  ptr->reg_embms_status_valid = TRUE;
  ptr->reg_embms_status = reg_embms_status;
}
void ril_utf_qmiril_if::set_qmi_nas_indication_register_req_msg_req_serving_system
  (uint8_t req_serving_system)
{
  nas_indication_register_req_msg_v01 *ptr =
    (nas_indication_register_req_msg_v01 *) payload;
  ptr->req_serving_system_valid = TRUE;
  ptr->req_serving_system = req_serving_system;
}
void ril_utf_qmiril_if::set_qmi_nas_indication_register_req_msg_reg_managed_roaming
  (uint8_t reg_managed_roaming)
{
  nas_indication_register_req_msg_v01 *ptr =
    (nas_indication_register_req_msg_v01 *) payload;
  ptr->reg_managed_roaming_valid = TRUE;
  ptr->reg_managed_roaming = reg_managed_roaming;
}
void ril_utf_qmiril_if::set_qmi_nas_indication_register_req_msg_sig_info
  (uint8_t sig_info)
{
  nas_indication_register_req_msg_v01 *ptr =
    (nas_indication_register_req_msg_v01 *) payload;
  ptr->sig_info_valid = TRUE;
  ptr->sig_info = sig_info;
}
void ril_utf_qmiril_if::set_qmi_nas_indication_register_reg_msg_err_rate
  (uint8_t err_rate)
{
  nas_indication_register_req_msg_v01 *ptr =
    (nas_indication_register_req_msg_v01 *) payload;
  ptr->err_rate_valid = TRUE;
  ptr->err_rate = err_rate;
}
void ril_utf_qmiril_if::set_qmi_nas_indication_register_req_msg_reg_current_plmn_name
  (uint8_t reg_current_plmn_name)
{
  nas_indication_register_req_msg_v01 *ptr =
    (nas_indication_register_req_msg_v01 *) payload;
  ptr->reg_current_plmn_name_valid = TRUE;
  ptr->reg_current_plmn_name = reg_current_plmn_name;
}
void ril_utf_qmiril_if::set_qmi_nas_indication_register_req_msg_reg_rf_band_info
  (uint8_t reg_rf_band_info)
{
  nas_indication_register_req_msg_v01 *ptr =
    (nas_indication_register_req_msg_v01 *) payload;
  ptr->reg_rf_band_info_valid = TRUE;
  ptr->reg_rf_band_info = reg_rf_band_info;
}

/*========================================================
  QMI_NAS_INDICATION_REGISTER_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_indication_register_resp_msg_v01(void)
{
  payload = (void *) malloc(sizeof(nas_indication_register_resp_msg_v01));
  payload_len = sizeof(nas_indication_register_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_INDICATION_REGISTER_RESP_MSG_V01;
}
void ril_utf_qmiril_if::set_qmi_nas_nas_indication_register_resp_msg_v01_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  nas_indication_register_resp_msg_v01 *ptr =
    (nas_indication_register_resp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
  QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_config_plmn_name_ind_reporting_req_msg_v01(void)
{
  payload = (void *) malloc(sizeof(nas_config_plmn_name_ind_reporting_req_msg_v01));
  payload_len = sizeof(nas_config_plmn_name_ind_reporting_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  verify_payload = false;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_REQ_MSG_V01;
  status = not_met;
}

void ril_utf_qmiril_if::set_qmi_nas_config_plmn_name_ind_reporting_req_msg_send_all_information(uint8_t send_all_information)
{
  nas_config_plmn_name_ind_reporting_req_msg_v01 *ptr =
    (nas_config_plmn_name_ind_reporting_req_msg_v01 *) payload;
  ptr->send_all_information = send_all_information;
}

/*========================================================
  QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_config_plmn_name_ind_reporting_resp_msg_v01(void)
{
  payload = (void *) malloc(sizeof(nas_config_plmn_name_ind_reporting_resp_msg_v01));
  payload_len = sizeof(nas_config_plmn_name_ind_reporting_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_RESP_MSG_V01;
}
void ril_utf_qmiril_if::set_qmi_nas_config_plmn_name_ind_reporting_resp_msg_v01_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  nas_config_plmn_name_ind_reporting_resp_msg_v01 *ptr =
    (nas_config_plmn_name_ind_reporting_resp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
  QMI_NAS_GET_CENTRALIZED_EONS_SUPPORT_STATUS_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_centralized_eons_support_status_req_msg_v01(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_CENTRALIZED_EONS_SUPPORT_STATUS_REQ_MSG_V01;
  status = not_met;
}

/*========================================================
  QMI_NAS_GET_CENTRALIZED_EONS_SUPPORT_STATUS_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_centralized_eons_support_status_resp_msg_v01(void)
{
  payload = (void *) malloc(sizeof(nas_get_centralized_eons_support_status_resp_msg_v01));
  payload_len = sizeof(nas_get_centralized_eons_support_status_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_CENTRALIZED_EONS_SUPPORT_STATUS_RESP_MSG_V01;
}
void ril_utf_qmiril_if::set_qmi_nas_get_centralized_eons_support_status_resp_msg_centralized_eons_supported
  (uint8_t centralized_eons_supported)
{
  nas_get_centralized_eons_support_status_resp_msg_v01 *ptr =
    (nas_get_centralized_eons_support_status_resp_msg_v01 *) payload;
  ptr->centralized_eons_supported_valid = TRUE;
  ptr->centralized_eons_supported = centralized_eons_supported;
}
void ril_utf_qmiril_if::set_qmi_nas_get_centralized_eons_support_status_resp_msg_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  nas_get_centralized_eons_support_status_resp_msg_v01 *ptr =
    (nas_get_centralized_eons_support_status_resp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
  QMI_NAS_GET_SERVING_SYSTEM_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_serving_system_req_msg_v01(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_SERVING_SYSTEM_REQ_MSG_V01;
  status = not_met;
}

/*========================================================
  QMI_NAS_GET_SERVING_SYSTEM_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_serving_system_resp_msg_v01(void)
{
  payload = (void *) malloc(sizeof(nas_get_serving_system_resp_msg_v01));
  payload_len = sizeof(nas_get_serving_system_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_SERVING_SYSTEM_RESP_MSG_V01;
}
void ril_utf_qmiril_if::set_qmi_nas_get_serving_system_resp_msg_serving_system
  (nas_serving_system_type_v01 serving_system)
{
  nas_get_serving_system_resp_msg_v01 *ptr =
    (nas_get_serving_system_resp_msg_v01 *) payload;
  ptr->serving_system = serving_system;
}
void ril_utf_qmiril_if::set_qmi_nas_get_serving_system_resp_msg_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  nas_get_serving_system_resp_msg_v01 *ptr =
    (nas_get_serving_system_resp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
  QMI_NAS_GET_SUB_BLOCKED_STATUS_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_sub_blocked_status_req_msg_v01(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_SUB_BLOCKED_STATUS_REQ_MSG_V01;
  status = not_met;
}

/*========================================================
  QMI_NAS_GET_SUB_BLOCKED_STATUS_RESP_MSG_V01
========================================================*/
nas_get_sub_blocked_status_resp_msg_v01* ril_utf_qmiril_if::update_default_qmi_nas_get_sub_blocked_status_resp_msg_v01(void)
{
  payload = (void *) malloc(sizeof(nas_get_sub_blocked_status_resp_msg_v01));
  payload_len = sizeof(nas_get_sub_blocked_status_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_SUB_BLOCKED_STATUS_RESP_MSG_V01;

  return (nas_get_sub_blocked_status_resp_msg_v01 *) payload;
}
void ril_utf_qmiril_if::set_qmi_nas_get_sub_blocked_status_resp_msg_v01_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  nas_get_sub_blocked_status_resp_msg_v01 *ptr =
    (nas_get_sub_blocked_status_resp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

void ril_utf_qmiril_if::set_qmi_nas_get_sub_blocked_status_resp_msg_is_blocked(uint8_t is_blocked_valid, uint8_t is_blocked)
{
  nas_get_sub_blocked_status_resp_msg_v01 *ptr =
    (nas_get_sub_blocked_status_resp_msg_v01 *) payload;
  ptr->is_blocked_valid = is_blocked_valid;
  ptr->is_blocked = is_blocked;
}

/*========================================================
  QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_sys_info_req_msg_v01(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_SYS_INFO_REQ_MSG_V01;
  status = not_met;
}

/*========================================================
  QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
========================================================*/
nas_get_sys_info_resp_msg_v01* ril_utf_qmiril_if::update_default_qmi_nas_get_sys_info_resp_msg_v01(void)
{
  payload = (void *) malloc(sizeof(nas_get_sys_info_resp_msg_v01));
  payload_len = sizeof(nas_get_sys_info_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_SYS_INFO_RESP_MSG_V01;

  return (nas_get_sys_info_resp_msg_v01 *) payload;
}
void ril_utf_qmiril_if::set_qmi_nas_get_sys_info_resp_msg_v01_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  nas_get_sys_info_resp_msg_v01 *ptr =
    (nas_get_sys_info_resp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

void ril_utf_qmiril_if::set_qmi_nas_get_sys_info_resp_msg_lte_attached(nas_service_domain_enum_type_v01 srv_domain)
{
  nas_get_sys_info_resp_msg_v01 *ptr =
    (nas_get_sys_info_resp_msg_v01 *) payload;
  ptr->lte_sys_info_valid = TRUE;
  ptr->lte_sys_info.common_sys_info.srv_domain_valid = TRUE;
  ptr->lte_sys_info.common_sys_info.srv_domain = srv_domain;
}
/*========================================================
  QMI_NAS_GET_SIG_INFO_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_sig_info_req_msg_v01(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_SIG_INFO_REQ_MSG_V01;
  status = not_met;
}

/*========================================================
  QMI_NAS_GET_SIG_INFO_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_sig_info_resp_msg_v01(void)
{
  payload = (void *) malloc(sizeof(nas_get_sig_info_resp_msg_v01));
  payload_len = sizeof(nas_get_sig_info_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_SIG_INFO_RESP_MSG_V01;

  nas_get_sig_info_resp_msg_v01 *ptr =
    (nas_get_sig_info_resp_msg_v01 *) payload;
  ptr->wcdma_sig_info_valid = TRUE;
  ptr->wcdma_sig_info.ecio = 7;
  ptr->wcdma_sig_info.rssi = 202;
}
void ril_utf_qmiril_if::set_qmi_nas_get_sig_info_resp_msg_v01_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  nas_get_sig_info_resp_msg_v01 *ptr =
    (nas_get_sig_info_resp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
  QMI_PBM_EMERGENCY_LIST_IND_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_pbm_list_ind_msg()
{
  payload = (void *) malloc(sizeof(pbm_emergency_list_ind_msg_v01));
  payload_len = sizeof(pbm_emergency_list_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_PBM;
  msg_id = QMI_PBM_EMERGENCY_LIST_IND_V01;
}

/*
 * Function: update_elenl_from_qmi_pbm_list_ind_msg
 * This function always needs to be called after calling update_default_qmi_pbm_list_ind_msg
 */

void ril_utf_qmiril_if::update_elenl_from_qmi_pbm_list_ind_msg(const char* number,
        const char *urn)
{
  pbm_emergency_list_ind_msg_v01 *ptr =
    (pbm_emergency_list_ind_msg_v01*) payload;
  ptr->elenl_emer_nums_valid = TRUE;
  ptr->elenl_emer_nums_len = 1;
  ptr->elenl_emer_nums[0].emer_nums_len = 1;
  ptr->elenl_emer_nums[0].emer_nums[0].sub_service_len = strlen(urn);
  strlcpy((char*)ptr->elenl_emer_nums[0].emer_nums[0].sub_service, "urn:service:sos.police.municipal",
          QMI_PBM_MAX_EMERGENCY_SUB_SERVICE_LENGTH_V01);
  ptr->elenl_emer_nums[0].emer_nums[0].emer_num_len = strlen(number);
  strlcpy((char*)ptr->elenl_emer_nums[0].emer_nums[0].emer_num, number,
          QMI_PBM_EMER_NUM_MAX_LENGTH_V01);
}

/*
 * Function: update_nw_from_qmi_pbm_list_ind_msg
 * This function always needs to be called after calling update_default_qmi_pbm_list_ind_msg
 */
void ril_utf_qmiril_if::update_nw_from_qmi_pbm_list_ind_msg(const char *number)
{
  pbm_emergency_list_ind_msg_v01 *ptr =
    (pbm_emergency_list_ind_msg_v01*) payload;
  ptr->network_emer_nums_valid = TRUE;
  ptr->network_emer_nums_len = 1;
  ptr->network_emer_nums[0].emer_nums_len = 1;
  ptr->network_emer_nums[0].emer_nums[0].emer_num_len = 3;
  strlcpy(ptr->network_emer_nums[0].emer_nums[0].emer_num, number,
    QMI_PBM_EMER_NUM_MAX_LENGTH_V01);
  ptr->network_emer_nums[0].emer_nums[0].cat = 1 << QMI_PBM_EMER_CAT_POLICE_BIT_V01 |
    1 << QMI_PBM_EMER_CAT_AMBULANCE_BIT_V01 |
    1 << QMI_PBM_EMER_CAT_FIRE_BRIGADE_BIT_V01 |
    1 << QMI_PBM_EMER_CAT_MARINE_GUARD_BIT_V01 |
    1 << QMI_PBM_EMER_CAT_MOUNTAIN_RESCUE_BIT_V01 |
    1 << QMI_PBM_EMER_CAT_MANUAL_ECALL_BIT_V01  |
    1 << QMI_PBM_EMER_CAT_AUTO_ECALL_BIT_V01;
}

/*
 * Function: update_nv_ecc_from_qmi_pbm_list_ind_msg
 * This function always needs to be called after calling update_default_qmi_pbm_list_ind_msg
 */
void ril_utf_qmiril_if::update_nv_ecc_from_qmi_pbm_list_ind_msg(const char *number)
{
  pbm_emergency_list_ind_msg_v01 *ptr =
    (pbm_emergency_list_ind_msg_v01*) payload;
  ptr->nv_emer_nums_extended_valid = TRUE;
  ptr->nv_emer_nums_extended_len = 1;
  ptr->nv_emer_nums_extended[0].emer_num_len = strlen(number);
  strlcpy(ptr->nv_emer_nums_extended[0].emer_num, number,
    QMI_PBM_EMER_NUM_MAX_LENGTH_V01);
  ptr->nv_emer_nums_extended_cat[0] = 1 << QMI_PBM_EMER_CAT_POLICE_BIT_V01 |
    1 << QMI_PBM_EMER_CAT_AMBULANCE_BIT_V01 |
    1 << QMI_PBM_EMER_CAT_FIRE_BRIGADE_BIT_V01 |
    1 << QMI_PBM_EMER_CAT_MARINE_GUARD_BIT_V01 |
    1 << QMI_PBM_EMER_CAT_MOUNTAIN_RESCUE_BIT_V01 |
    1 << QMI_PBM_EMER_CAT_MANUAL_ECALL_BIT_V01  |
    1 << QMI_PBM_EMER_CAT_AUTO_ECALL_BIT_V01;
}

/*
 * Function: update_card_ecc_from_qmi_pbm_list_ind_msg
 * This function always needs to be called after calling update_default_qmi_pbm_list_ind_msg
 */
void ril_utf_qmiril_if::update_card_ecc_from_qmi_pbm_list_ind_msg(const char *number)
{
  pbm_emergency_list_ind_msg_v01 *ptr =
    (pbm_emergency_list_ind_msg_v01*) payload;
  ptr->card_emer_nums_valid = TRUE;
  ptr->card_emer_nums_len = 1;
  ptr->card_emer_nums[0].emer_nums_len = 1;
  ptr->card_emer_nums[0].emer_nums[0].emer_num_len = strlen(number);
  strlcpy(ptr->card_emer_nums[0].emer_nums[0].emer_num, number,
    QMI_PBM_EMER_NUM_MAX_LENGTH_V01);
  ptr->card_emer_nums[0].emer_nums[0].cat = 1 << QMI_PBM_EMER_CAT_POLICE_BIT_V01 |
    1 << QMI_PBM_EMER_CAT_AMBULANCE_BIT_V01 |
    1 << QMI_PBM_EMER_CAT_FIRE_BRIGADE_BIT_V01 |
    1 << QMI_PBM_EMER_CAT_MARINE_GUARD_BIT_V01 |
    1 << QMI_PBM_EMER_CAT_MOUNTAIN_RESCUE_BIT_V01 |
    1 << QMI_PBM_EMER_CAT_MANUAL_ECALL_BIT_V01  |
    1 << QMI_PBM_EMER_CAT_AUTO_ECALL_BIT_V01;
}

/*
 * Function: update_hardcoded_ecc_from_qmi_pbm_list_ind_msg
 * This function always needs to be called after calling update_default_qmi_pbm_list_ind_msg
 */
void ril_utf_qmiril_if::update_hardcoded_ecc_from_qmi_pbm_list_ind_msg(const char *number)
{
  pbm_emergency_list_ind_msg_v01 *ptr =
    (pbm_emergency_list_ind_msg_v01*) payload;
  ptr->emer_nums_extended_valid = TRUE;
  ptr->emer_nums_extended_len = 1;
  ptr->emer_nums_extended[0].emer_num_len = strlen(number);
  strlcpy(ptr->emer_nums_extended[0].emer_num, number,
    QMI_PBM_EMER_NUM_MAX_LENGTH_V01);
  ptr->emer_nums_extended_cat[0] = 1 << QMI_PBM_EMER_CAT_POLICE_BIT_V01 |
    1 << QMI_PBM_EMER_CAT_AMBULANCE_BIT_V01 |
    1 << QMI_PBM_EMER_CAT_FIRE_BRIGADE_BIT_V01 |
    1 << QMI_PBM_EMER_CAT_MARINE_GUARD_BIT_V01 |
    1 << QMI_PBM_EMER_CAT_MOUNTAIN_RESCUE_BIT_V01 |
    1 << QMI_PBM_EMER_CAT_MANUAL_ECALL_BIT_V01  |
    1 << QMI_PBM_EMER_CAT_AUTO_ECALL_BIT_V01;
}

/*
 * Function: update_mcc_ecc_from_qmi_pbm_list_ind_msg
 * This function always needs to be called after calling update_default_qmi_pbm_list_ind_msg
 */
void ril_utf_qmiril_if::update_mcc_ecc_from_qmi_pbm_list_ind_msg(const char *number)
{
  pbm_emergency_list_ind_msg_v01 *ptr =
    (pbm_emergency_list_ind_msg_v01*) payload;
  ptr->mcc_emer_nums_valid = TRUE;
  ptr->mcc_emer_nums_len = 1;
  ptr->mcc_emer_nums[0].emer_nums_len = 1;
  ptr->mcc_emer_nums[0].emer_nums[0].emer_num_len = strlen(number);
  strlcpy(ptr->mcc_emer_nums[0].emer_nums[0].emer_num, number,
    QMI_PBM_EMER_NUM_MAX_LENGTH_V01);
}

/*========================================================
  QMI_PBM_INDICATION_REGISTER_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_pbm_indication_register_req_msg(void)
{
  payload = (void *)malloc(sizeof(pbm_indication_register_req_msg_v01)) ;//req msg is empty
  payload_len = sizeof(pbm_indication_register_req_msg_v01);
  memset(payload, 0x00, payload_len);
  service_id = QMI_PBM;
  payload_type = ril_utf_qmi_request;
  msg_id = QMI_PBM_INDICATION_REGISTER_REQ_V01;
  status = not_met;
}

void ril_utf_qmiril_if::set_qmi_pbm_indication_register_mask(pbm_reg_mask_type_v01 reg_mask)
{
  pbm_indication_register_req_msg_v01 *ptr = (pbm_indication_register_req_msg_v01*)payload;
  ptr->reg_mask = reg_mask;
}

/*========================================================
  QMI_PBM_INDICATION_REGISTER_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_pbm_indication_register_resp_msg(void)
{
  payload = (void *) malloc(sizeof(pbm_indication_register_resp_msg_v01));
  payload_len = sizeof(pbm_indication_register_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_PBM;
  msg_id = QMI_PBM_INDICATION_REGISTER_RESP_V01;
}
void ril_utf_qmiril_if::set_qmi_pbm_indication_register_resp_v01_reg_mask
  (pbm_reg_mask_type_v01 reg_mask)
{
  pbm_indication_register_resp_msg_v01 *ptr =
    (pbm_indication_register_resp_msg_v01 *) payload;
  ptr->reg_mask_valid = TRUE;
  ptr->reg_mask = reg_mask;
}
void ril_utf_qmiril_if::set_qmi_pbm_indication_register_resp_v01_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  pbm_indication_register_resp_msg_v01 *ptr =
    (pbm_indication_register_resp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
  QMI_PBM_GET_CONFIGURATION_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_pbm_get_configuration_req_msg(void)
{
  payload = (void *)malloc(sizeof(pbm_get_configuration_req_msg_v01)) ;//req msg is empty
  payload_len = sizeof(pbm_get_configuration_req_msg_v01);
  memset(payload, 0x00, payload_len);
  service_id = QMI_PBM;
  payload_type = ril_utf_qmi_request;
  msg_id = QMI_PBM_GET_CONFIGURATION_REQ_V01;
  status = not_met;
}

void ril_utf_qmiril_if::set_qmi_pbm_get_configuration_pbm_feature_status_mask(phonebook_feature_status_mask_type_v01 pbm_feature_status_mask)
{
  pbm_get_configuration_req_msg_v01 *ptr = (pbm_get_configuration_req_msg_v01*)payload;
  ptr->pbm_feature_status_mask_valid = TRUE;
  ptr->pbm_feature_status_mask = pbm_feature_status_mask;
}

/*========================================================
  QMI_PBM_GET_CONFIGURATION_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_pbm_get_configuration_resp_msg(void)
{
  payload = (void *) malloc(sizeof(pbm_get_configuration_resp_msg_v01));
  payload_len = sizeof(pbm_get_configuration_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_PBM;
  msg_id = QMI_PBM_GET_CONFIGURATION_RESP_V01;
}
void ril_utf_qmiril_if::set_qmi_pbm_get_configuration_resp_centralized_ecc
  (uint8_t centralized_ecc_valid, uint8_t centralized_ecc)
{
  pbm_get_configuration_resp_msg_v01 *ptr =
    (pbm_get_configuration_resp_msg_v01 *) payload;
  ptr->centralized_ecc_valid = centralized_ecc_valid;
  ptr->centralized_ecc = centralized_ecc;
}
void ril_utf_qmiril_if::set_qmi_pbm_get_configuration_resp_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  pbm_get_configuration_resp_msg_v01 *ptr =
    (pbm_get_configuration_resp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
  QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_get_transport_layer_info_req_v01(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ_V01;
  status = not_met;
}

/*========================================================
  QMI_WMS_GET_TRANSPORT_LAYER_INFO_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_get_transport_layer_info_resp_v01(void)
{
  payload = (void *) malloc(sizeof(wms_get_transport_layer_resp_msg_v01));
  payload_len = sizeof(wms_get_transport_layer_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_GET_TRANSPORT_LAYER_INFO_RESP_V01;
}
void ril_utf_qmiril_if::set_qmi_wms_get_transport_layer_info_resp_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  wms_get_transport_layer_resp_msg_v01 *ptr =
    (wms_get_transport_layer_resp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
QMI_NAS_SET_LTE_BAND_PRIORITY_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_set_lte_band_priority_req_msg_v01(void)
{
  payload = (void *) malloc(sizeof(nas_set_lte_band_priority_req_msg_v01));
  payload_len = sizeof(nas_set_lte_band_priority_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_SET_LTE_BAND_PRIORITY_REQ_MSG_V01;
  status = not_met;
}

//---------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------

/*========================================================
  QMI_VOICE_SET_CONFIG_REQ_V02
========================================================*/

void ril_utf_qmiril_if::update_default_qmi_voice_set_config_req_msg(void)
{
  payload = (void *)malloc(sizeof(voice_set_config_req_msg_v02));
  payload_len = sizeof(voice_set_config_req_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_SET_CONFIG_REQ_V02;
}

void ril_utf_qmiril_if::set_qmi_voice_set_config_req_tty_mode(tty_mode_enum_v02 val)
{
  voice_set_config_req_msg_v02 *ptr = (voice_set_config_req_msg_v02 *) payload;
  ptr->tty_mode_valid = TRUE;
  ptr->tty_mode = val;
}

void ril_utf_qmiril_if::set_qmi_voice_set_config_req_ui_tty_setting(tty_mode_enum_v02 tty_setting)
{
  voice_set_config_req_msg_v02 *ptr = (voice_set_config_req_msg_v02 *) payload;
  ptr->ui_tty_setting_valid = TRUE;
  ptr->ui_tty_setting = tty_setting;
}

/*========================================================
  QMI_VOICE_SET_CONFIG_RESP_V02
========================================================*/

void ril_utf_qmiril_if::update_default_qmi_voice_set_config_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(voice_set_config_resp_msg_v02));
  payload_len = sizeof(voice_set_config_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_SET_CONFIG_RESP_V02;

  voice_set_config_resp_msg_v02 *ptr = (voice_set_config_resp_msg_v02 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}



/*========================================================
  QMI_VOICE_GET_CONFIG_REQ_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_get_config_req_msg(void)
{
  payload = (void *)malloc(sizeof(voice_get_config_req_msg_v02));
  payload_len = sizeof(voice_get_config_req_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_GET_CONFIG_REQ_V02;
}
void ril_utf_qmiril_if::set_qmi_voice_get_config_req_voice_privacy(uint8_t val)
{
  voice_get_config_req_msg_v02 *ptr = (voice_get_config_req_msg_v02 *) payload;
  ptr->voice_privacy_valid = TRUE;
  ptr->voice_privacy = val;
}

void ril_utf_qmiril_if::set_qmi_voice_get_config_req_auto_answer(uint8_t val)
{
  voice_get_config_req_msg_v02 *ptr = (voice_get_config_req_msg_v02 *) payload;
  ptr->auto_answer_valid = TRUE;
  ptr->auto_answer = val;
}
void ril_utf_qmiril_if::set_qmi_voice_get_config_req_roam_timer(uint8_t val)
{
  voice_get_config_req_msg_v02 *ptr = (voice_get_config_req_msg_v02 *) payload;
  ptr->roam_timer_valid = TRUE;
  ptr->roam_timer = val;
}
void ril_utf_qmiril_if::set_qmi_voice_get_config_req_pref_voice_so(uint8_t val)
{
  voice_get_config_req_msg_v02 *ptr = (voice_get_config_req_msg_v02 *) payload;
  ptr->pref_voice_so_valid = TRUE;
  ptr->pref_voice_so = val;
}
void ril_utf_qmiril_if::set_qmi_voice_get_config_req_tty_mode(uint8_t val)
{
  voice_get_config_req_msg_v02 *ptr = (voice_get_config_req_msg_v02 *) payload;
  ptr->tty_mode_valid = TRUE;
  ptr->tty_mode = val;
}


/*========================================================
  QMI_VOICE_GET_CONFIG_RESP_V02
========================================================*/

void ril_utf_qmiril_if::update_default_qmi_voice_get_config_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(voice_get_config_resp_msg_v02));
  payload_len = sizeof(voice_get_config_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_GET_CONFIG_RESP_V02;

  voice_get_config_resp_msg_v02 *ptr = (voice_get_config_resp_msg_v02 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_voice_get_config_resp_current_tty_mode(tty_mode_enum_v02 val)
{
  voice_get_config_resp_msg_v02 *ptr = (voice_get_config_resp_msg_v02 *)payload;
  ptr->current_tty_mode_valid = TRUE;
  ptr->current_tty_mode = val;
}
void ril_utf_qmiril_if::set_qmi_voice_get_config_resp_current_voice_privacy_pref(voice_privacy_enum_v02 val)
{
  voice_get_config_resp_msg_v02 *ptr = (voice_get_config_resp_msg_v02 *)payload;
  ptr->current_voice_privacy_pref_valid = TRUE;
  ptr->current_voice_privacy_pref = val;

}


/*========================================================
  QMI_VOICE_DIAL_CALL_REQ_V02
========================================================*/

voice_dial_call_req_msg_v02 *ril_utf_qmiril_if::update_default_qmi_voice_dial_call_req_msg
(char *address)
{
  payload = (void *)malloc(sizeof(voice_dial_call_req_msg_v02));
  payload_len = sizeof(voice_dial_call_req_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_DIAL_CALL_REQ_V02;

  voice_dial_call_req_msg_v02 *ptr = (voice_dial_call_req_msg_v02 *)payload;
  strlcpy(ptr->calling_number, address, sizeof(ptr->calling_number));
  return ptr;
}

void ril_utf_qmiril_if::set_qmi_voice_dial_call_req_call_type(call_type_enum_v02 val)
{
  voice_dial_call_req_msg_v02 *ptr = (voice_dial_call_req_msg_v02 *) payload;
  ptr->call_type_valid = TRUE;
  ptr->call_type = val;
}

void ril_utf_qmiril_if::set_qmi_voice_dial_call_req_clir_type(clir_type_enum_v02 val)
{
  voice_dial_call_req_msg_v02 *ptr = (voice_dial_call_req_msg_v02 *) payload;
  ptr->clir_type_valid = TRUE;
  ptr->clir_type = (clir_type_enum_v02)val;
}

voice_uus_type_v02* ril_utf_qmiril_if::set_qmi_voice_dial_call_req_uus()
{
  voice_dial_call_req_msg_v02 *ptr = (voice_dial_call_req_msg_v02 *) payload;
  ptr->uus_valid = TRUE;
  return &(ptr->uus);
}

char* ril_utf_qmiril_if::set_qmi_voice_dial_call_req_sip_uri_overflow(char *val)
{
  voice_dial_call_req_msg_v02 *ptr = (voice_dial_call_req_msg_v02 *) payload;
  ptr->sip_uri_overflow_valid = TRUE;
  if (val) {
    strlcpy(ptr->sip_uri_overflow, val, sizeof(ptr->sip_uri_overflow));
  }
  return (ptr->sip_uri_overflow);
}


void ril_utf_qmiril_if::set_qmi_voice_dial_call_req_audio_attrib(uint64_t val)
{
  voice_dial_call_req_msg_v02 *ptr = (voice_dial_call_req_msg_v02 *) payload;
  ptr->audio_attrib_valid = TRUE;
  ptr->audio_attrib = val;
}


void ril_utf_qmiril_if::set_qmi_voice_dial_call_req_video_attrib(uint64_t val)
{
  voice_dial_call_req_msg_v02 *ptr = (voice_dial_call_req_msg_v02 *) payload;
  ptr->video_attrib_valid = TRUE;
  ptr->video_attrib = val;
}

void ril_utf_qmiril_if::set_qmi_voice_dial_call_req_pi(ip_pi_enum_v02 val)
{
  voice_dial_call_req_msg_v02 *ptr = (voice_dial_call_req_msg_v02 *) payload;
  ptr->pi_valid = TRUE;
  ptr->pi = val;
}

void ril_utf_qmiril_if::set_qmi_voice_dial_call_req_service_type(voice_dial_call_service_type_enum_v02 val)
{
  voice_dial_call_req_msg_v02 *ptr = (voice_dial_call_req_msg_v02 *) payload;
  ptr->service_type_valid = TRUE;
  ptr->service_type = val;
}

void ril_utf_qmiril_if::set_qmi_voice_dial_call_req_conf_uri_list(char *val)
{
  voice_dial_call_req_msg_v02 *ptr = (voice_dial_call_req_msg_v02 *) payload;
  ptr->conf_uri_list_valid = TRUE;
  memcpy(ptr->conf_uri_list, val, strlen(val));
}
void ril_utf_qmiril_if::set_qmi_voice_dial_call_req_called_party_subaddress_extension_bit(uint8_t val)
{
  voice_dial_call_req_msg_v02 *ptr = (voice_dial_call_req_msg_v02 *) payload;
  ptr->called_party_subaddress_valid = TRUE;
  ptr->called_party_subaddress.extension_bit = val;
}

void ril_utf_qmiril_if::set_qmi_voice_dial_call_req_called_party_subaddress_subaddress_type(subaddress_type_enum_v02 val)
{
  voice_dial_call_req_msg_v02 *ptr = (voice_dial_call_req_msg_v02 *) payload;
  ptr->called_party_subaddress_valid = TRUE;
  ptr->called_party_subaddress.subaddress_type = val;
}

void ril_utf_qmiril_if::set_qmi_voice_dial_call_req_called_party_subaddress_odd_even_ind(uint8_t val)
{
  voice_dial_call_req_msg_v02 *ptr = (voice_dial_call_req_msg_v02 *) payload;
  ptr->called_party_subaddress_valid = TRUE;
  ptr->called_party_subaddress.odd_even_ind = val;
}

void ril_utf_qmiril_if::set_qmi_voice_dial_call_req_called_party_subaddress_subaddress(char *val)
{
  voice_dial_call_req_msg_v02 *ptr = (voice_dial_call_req_msg_v02 *) payload;
  ptr->called_party_subaddress_valid = TRUE;
  memcpy(ptr->called_party_subaddress.subaddress, val, strlen(val));
  ptr->called_party_subaddress.subaddress_len = strlen(val);
}
void ril_utf_qmiril_if::set_qmi_voice_dial_call_req_is_secure_call(uint8_t is_secure_call)
{
  voice_dial_call_req_msg_v02 *ptr = (voice_dial_call_req_msg_v02 *) payload;
  ptr->is_secure_call_valid = TRUE;
  ptr->is_secure_call = is_secure_call;
}
uint16_t * ril_utf_qmiril_if::set_qmi_voice_dial_call_req_display_text(uint32_t display_text_len)
{
  voice_dial_call_req_msg_v02 *ptr = (voice_dial_call_req_msg_v02 *) payload;
  ptr->display_text_valid = TRUE;
  ptr->display_text_len = display_text_len;
  return (ptr->display_text);
}
void ril_utf_qmiril_if::set_qmi_voice_dial_call_req_emer_cat(uint8_t emer_cat)
{
  voice_dial_call_req_msg_v02 *ptr = (voice_dial_call_req_msg_v02 *) payload;
  ptr->emer_cat_valid = TRUE;
  ptr->emer_cat = emer_cat;
}
void ril_utf_qmiril_if::set_qmi_voice_dial_call_req_origination_number(char *val)
{
  voice_dial_call_req_msg_v02 *ptr = (voice_dial_call_req_msg_v02 *) payload;
  ptr->origination_number_valid = TRUE;
  strlcpy(ptr->origination_number, val, sizeof(ptr->origination_number));
}
void ril_utf_qmiril_if::set_qmi_voice_dial_call_req_is_secondary(uint8_t val)
{
  voice_dial_call_req_msg_v02 *ptr = (voice_dial_call_req_msg_v02 *) payload;
  ptr->is_secondary_valid = TRUE;
  ptr->is_secondary = val;
}
void ril_utf_qmiril_if::set_qmi_voice_dial_call_req_call_pull(uint8_t val)
{
  voice_dial_call_req_msg_v02 *ptr = (voice_dial_call_req_msg_v02 *) payload;
  ptr->call_pull_valid = TRUE;
  ptr->call_pull = val;
}
void ril_utf_qmiril_if::set_qmi_voice_dial_call_req_call_failure_reason(call_end_reason_enum_v02 val)
{
  voice_dial_call_req_msg_v02 *ptr = (voice_dial_call_req_msg_v02 *) payload;
  ptr->call_failure_reason_valid = TRUE;
  ptr->call_failure_reason = val;
}
void ril_utf_qmiril_if::set_qmi_voice_dial_call_req_call_failure_mode(call_mode_enum_v02 val)
{
  voice_dial_call_req_msg_v02 *ptr = (voice_dial_call_req_msg_v02 *) payload;
  ptr->call_failure_mode_valid = TRUE;
  ptr->call_failure_mode = val;
}

/*========================================================
  QMI_VOICE_DIAL_CALL_RESP_V02
========================================================*/

void ril_utf_qmiril_if::update_default_qmi_voice_dial_call_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(voice_dial_call_resp_msg_v02));
  payload_len = sizeof(voice_dial_call_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_DIAL_CALL_RESP_V02;

  voice_dial_call_resp_msg_v02 *ptr = (voice_dial_call_resp_msg_v02 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_voice_dial_call_resp_call_id (uint8_t call_id)
{
  voice_dial_call_resp_msg_v02 *ptr = (voice_dial_call_resp_msg_v02 *) payload;
  ptr->call_id_valid = TRUE;
  ptr->call_id = call_id;
}
void ril_utf_qmiril_if::set_qmi_voice_dial_call_resp_call_type(int val)
{
  voice_dial_call_resp_msg_v02 *ptr = (voice_dial_call_resp_msg_v02 *) payload;
  ptr->call_id_valid = TRUE;
  ptr->call_id = val;
}

voice_alpha_ident_type_v02* ril_utf_qmiril_if::set_qmi_voice_dial_call_resp_alpha_ident()
{
  voice_dial_call_resp_msg_v02 *ptr = (voice_dial_call_resp_msg_v02 *) payload;
  ptr->alpha_ident_valid = TRUE;
  return &(ptr->alpha_ident);
}

void ril_utf_qmiril_if::set_qmi_voice_dial_call_cc_result_type(voice_cc_result_type_enum_v02 val)
{
  voice_dial_call_resp_msg_v02 *ptr = (voice_dial_call_resp_msg_v02 *) payload;
  ptr->cc_result_type_valid = TRUE;
  ptr->cc_result_type = (voice_cc_result_type_enum_v02)val;
}

voice_cc_sups_result_type_v02* ril_utf_qmiril_if::set_qmi_voice_dial_call_resp_cc_sups_result()
{
  voice_dial_call_resp_msg_v02 *ptr = (voice_dial_call_resp_msg_v02 *) payload;
  ptr->cc_sups_result_valid = TRUE;
  return &(ptr->cc_sups_result);
}
void ril_utf_qmiril_if::set_qmi_voice_dial_call_resp_media_id (uint8_t media_id)
{
  voice_dial_call_resp_msg_v02 *ptr = (voice_dial_call_resp_msg_v02 *) payload;
  ptr->media_id_valid = TRUE;
  ptr->media_id = media_id;
}
void ril_utf_qmiril_if::set_qmi_voice_dial_call_resp_end_reason(call_end_reason_enum_v02  val)
{
  voice_dial_call_resp_msg_v02 *ptr = (voice_dial_call_resp_msg_v02 *) payload;
  ptr->end_reason_valid = TRUE;
  ptr->end_reason = val;
}

/*========================================================
  QMI_VOICE_ANSWER_CALL_REQ_V02
========================================================*/

void ril_utf_qmiril_if::update_default_qmi_voice_answer_call_req_msg(uint8_t call_id)
{
  payload = (void *)malloc(sizeof(voice_answer_call_req_msg_v02));
  payload_len = sizeof(voice_answer_call_req_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_ANSWER_CALL_REQ_V02;

  // Mandatory TLV call_id -- to be implemented
  voice_answer_call_req_msg_v02 *ptr = (voice_answer_call_req_msg_v02 *)payload;
  ptr->call_id = call_id;
}

void ril_utf_qmiril_if::set_qmi_voice_answer_call_req_call_type(call_type_enum_v02 val)
{
  voice_answer_call_req_msg_v02 *ptr = (voice_answer_call_req_msg_v02 *) payload;
  ptr->call_type_valid = TRUE;
  ptr->call_type = (call_type_enum_v02)val;
}

void ril_utf_qmiril_if::set_qmi_voice_answer_call_req_audio_attrib(uint64_t val)
{
  voice_answer_call_req_msg_v02 *ptr = (voice_answer_call_req_msg_v02 *) payload;
  ptr->audio_attrib_valid = TRUE;
  ptr->audio_attrib = val;
}


void ril_utf_qmiril_if::set_qmi_voice_answer_call_req_video_attrib(uint64_t val)
{
  voice_answer_call_req_msg_v02 *ptr = (voice_answer_call_req_msg_v02 *) payload;
  ptr->video_attrib_valid = TRUE;
  ptr->video_attrib = val;
}

void ril_utf_qmiril_if::set_qmi_voice_answer_call_req_pi(ip_pi_enum_v02 val)
{
  voice_answer_call_req_msg_v02 *ptr = (voice_answer_call_req_msg_v02 *) payload;
  ptr->pi_valid = TRUE;
  ptr->pi = val;
}

void ril_utf_qmiril_if::set_qmi_voice_answer_call_req_reject_call(uint8_t val)
{
  voice_answer_call_req_msg_v02 *ptr = (voice_answer_call_req_msg_v02 *) payload;
  ptr->reject_call_valid = TRUE;
  ptr->reject_call = val;
}

void ril_utf_qmiril_if::set_qmi_voice_answer_call_req_reject_cause(voice_reject_cause_enum_v02 val)
{
  voice_answer_call_req_msg_v02 *ptr = (voice_answer_call_req_msg_v02 *) payload;
  ptr->reject_cause_valid = TRUE;
  ptr->reject_cause = val;
}
/*========================================================
  QMI_VOICE_ANSWER_CALL_RESP_V02
========================================================*/

void ril_utf_qmiril_if::update_default_qmi_voice_answer_call_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(voice_answer_call_resp_msg_v02));
  payload_len = sizeof(voice_answer_call_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_ANSWER_CALL_RESP_V02;

  voice_answer_call_resp_msg_v02 *ptr = (voice_answer_call_resp_msg_v02 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

/*========================================================
  QMI_VOICE_END_CALL_REQ_V02
========================================================*/

void ril_utf_qmiril_if::update_default_qmi_voice_end_call_req_msg(uint8_t call_id_val)
{
  payload = (void *)malloc(sizeof(voice_end_call_req_msg_v02));
  payload_len = sizeof(voice_end_call_req_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_END_CALL_REQ_V02;

 // Mandatory TLV.. to be implemented
  voice_end_call_req_msg_v02 *ptr = (voice_end_call_req_msg_v02 *)payload;
  ptr->call_id = call_id_val;
}

void ril_utf_qmiril_if::set_qmi_voice_end_call_req_end_cause(voice_reject_cause_enum_v02 end_cause)
{
  voice_end_call_req_msg_v02 *ptr = (voice_end_call_req_msg_v02 *)payload;
  ptr->end_cause_valid = TRUE;
  ptr->end_cause = end_cause;
}

/*========================================================
  QMI_VOICE_END_CALL_RESP_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_end_call_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(voice_end_call_resp_msg_v02));
  payload_len = sizeof(voice_end_call_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_END_CALL_RESP_V02;

  voice_end_call_resp_msg_v02 *ptr = (voice_end_call_resp_msg_v02 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;

}
void ril_utf_qmiril_if::set_qmi_voice_end_call_resp_call_id(int call_id)
{
  voice_end_call_resp_msg_v02 *ptr = (voice_end_call_resp_msg_v02 *)payload;
  ptr->call_id_valid = TRUE;
  ptr->call_id = call_id;
}

/*========================================================
  QMI_VOICE_MANAGE_CALLS_REQ_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_manage_calls_req_v02(sups_type_enum_v02 sups_type)
{
  payload = (void *)malloc(sizeof(voice_manage_calls_req_msg_v02));
  payload_len = sizeof(voice_manage_calls_req_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_MANAGE_CALLS_REQ_V02;

  voice_manage_calls_req_msg_v02 *ptr = (voice_manage_calls_req_msg_v02*) payload;
  ptr->sups_type = sups_type;
}
void ril_utf_qmiril_if::set_qmi_voice_manage_calls_req_v02_call_id(uint8_t call_id)
{
  voice_manage_calls_req_msg_v02 *ptr = (voice_manage_calls_req_msg_v02*) payload;
  ptr->call_id_valid = TRUE;
  ptr->call_id = call_id;
}

/*========================================================
  QMI_VOICE_MANAGE_CALLS_RESP_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_manage_calls_resp_v02(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(voice_manage_calls_resp_msg_v02));
  payload_len = sizeof(voice_manage_calls_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_MANAGE_CALLS_RESP_V02;

  voice_manage_calls_resp_msg_v02 *ptr = (voice_manage_calls_resp_msg_v02*)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

/*========================================================
  QMI_VOICE_MANAGE_IP_CALLS_REQ_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_manage_ip_calls_req_msg(voip_sups_type_enum_v02 sups_type_val)
{
  payload = (void *)malloc(sizeof(voice_manage_ip_calls_req_msg_v02));
  payload_len = sizeof(voice_manage_ip_calls_req_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_MANAGE_IP_CALLS_REQ_V02;

// Mandatory TLV.. what would be the default value that has to be given to it ?
  voice_manage_ip_calls_req_msg_v02 *ptr = (voice_manage_ip_calls_req_msg_v02 *)payload;
  ptr->sups_type =  sups_type_val;
}
void ril_utf_qmiril_if::set_qmi_voice_manage_ip_calls_req_call_id(uint8_t val)
{
  voice_manage_ip_calls_req_msg_v02 *ptr = (voice_manage_ip_calls_req_msg_v02 *) payload;
  ptr->call_id_valid = TRUE;
  ptr->call_id = val;
}

void ril_utf_qmiril_if::set_qmi_voice_manage_ip_calls_req_call_type(call_type_enum_v02 val)
{
  voice_manage_ip_calls_req_msg_v02 *ptr = (voice_manage_ip_calls_req_msg_v02 *) payload;
  ptr->call_type_valid = TRUE;
  ptr->call_type = val;
}

void ril_utf_qmiril_if::set_qmi_voice_manage_ip_calls_req_audio_attrib(uint64_t val)
{
  voice_manage_ip_calls_req_msg_v02 *ptr = (voice_manage_ip_calls_req_msg_v02 *) payload;
  ptr->audio_attrib_valid = TRUE;
  ptr->audio_attrib = (voice_call_attribute_type_mask_v02)val;
}

void ril_utf_qmiril_if::set_qmi_voice_manage_ip_calls_req_video_attrib(uint64_t val)
{
  voice_manage_ip_calls_req_msg_v02 *ptr = (voice_manage_ip_calls_req_msg_v02 *) payload;
  ptr->video_attrib_valid = TRUE;
  ptr->video_attrib = (voice_call_attribute_type_mask_v02)val;
}
void ril_utf_qmiril_if::set_qmi_voice_manage_ip_calls_req_rtt_mode(rtt_mode_type_v02 val)
{
  voice_manage_ip_calls_req_msg_v02 *ptr = (voice_manage_ip_calls_req_msg_v02 *) payload;
  ptr->rtt_mode_valid = TRUE;
  ptr->rtt_mode = val;
}
void ril_utf_qmiril_if::set_qmi_voice_manage_ip_calls_req_ect_type(voip_ect_type_enum_v02 val)
{
  voice_manage_ip_calls_req_msg_v02 *ptr = (voice_manage_ip_calls_req_msg_v02 *) payload;
  ptr->ect_type_valid = TRUE;
  ptr->ect_type = val;
}
void ril_utf_qmiril_if::set_qmi_voice_manage_ip_calls_req_transfer_target_call_id(uint8_t val)
{
  voice_manage_ip_calls_req_msg_v02 *ptr = (voice_manage_ip_calls_req_msg_v02 *) payload;
  ptr->transfer_target_call_id_valid = TRUE;
  ptr->transfer_target_call_id = val;
}
char *ril_utf_qmiril_if::set_qmi_voice_manage_ip_calls_sip_uri()
{
  voice_manage_ip_calls_req_msg_v02 *ptr = (voice_manage_ip_calls_req_msg_v02 *) payload;
  ptr->sip_uri_valid = TRUE;
  return ptr->sip_uri;
}

/*========================================================
  QMI_VOICE_MANAGE_IP_CALLS_RESP_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_manage_ip_calls_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(voice_manage_ip_calls_resp_msg_v02));
  payload_len = sizeof(voice_manage_ip_calls_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_MANAGE_IP_CALLS_RESP_V02;

  voice_manage_ip_calls_resp_msg_v02 *ptr = (voice_manage_ip_calls_resp_msg_v02 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_voice_manage_ip_calls_sip_error_code(uint32_t sip_error_code) {
  voice_manage_ip_calls_resp_msg_v02 *ptr = (voice_manage_ip_calls_resp_msg_v02 *)payload;
  ptr->sip_error_code_valid = TRUE;
  ptr->sip_error_code = sip_error_code;
}
void ril_utf_qmiril_if::set_qmi_voice_manage_ip_calls_failure_cause(qmi_sups_errors_enum_v02 val) {
  voice_manage_ip_calls_resp_msg_v02 *ptr = (voice_manage_ip_calls_resp_msg_v02 *)payload;
  ptr->failure_cause_valid = TRUE;
  ptr->failure_cause = val;
}
void ril_utf_qmiril_if::set_qmi_voice_manage_ip_calls_call_modified_cause(voice_call_modified_cause_enum_v02 val) {
  voice_manage_ip_calls_resp_msg_v02 *ptr = (voice_manage_ip_calls_resp_msg_v02 *)payload;
  ptr->call_modified_cause_valid = TRUE;
  ptr->call_modified_cause = val;
}
void ril_utf_qmiril_if::set_qmi_voice_manage_ip_calls_call_id(uint8_t val) {
  voice_manage_ip_calls_resp_msg_v02 *ptr = (voice_manage_ip_calls_resp_msg_v02 *)payload;
  ptr->call_id_valid = TRUE;
  ptr->call_id = val;
}
voice_ip_end_reason_text_type_v02* ril_utf_qmiril_if::set_qmi_voice_manage_ip_calls_end_reason_text(uint32_t end_reason_text_len) {
  voice_manage_ip_calls_resp_msg_v02 *ptr = (voice_manage_ip_calls_resp_msg_v02 *)payload;
  ptr->end_reason_text_valid = TRUE;
  ptr->end_reason_text_len = end_reason_text_len;
  return (ptr->end_reason_text);
}

/*========================================================
  QMI_VOICE_IMS_CALL_CANCEL_REQ_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_ims_call_cancel_req_msg(uint8_t call_id, voip_cancel_type_enum_v02 cancel_type)
{
  payload = (void *)malloc(sizeof(voice_ims_call_cancel_req_msg_v02));
  payload_len = sizeof(voice_ims_call_cancel_req_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_IMS_CALL_CANCEL_REQ_V02;

  voice_ims_call_cancel_req_msg_v02 *ptr = (voice_ims_call_cancel_req_msg_v02 *)payload;
  ptr->call_id = call_id;
  ptr->cancel_type = cancel_type;
}

/*========================================================
  QMI_VOICE_IMS_CALL_CANCEL_RESP_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_ims_call_cancel_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(voice_ims_call_cancel_resp_msg_v02));
  payload_len = sizeof(voice_ims_call_cancel_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_IMS_CALL_CANCEL_RESP_V02;

  voice_ims_call_cancel_resp_msg_v02 *ptr = (voice_ims_call_cancel_resp_msg_v02 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_voice_ims_call_cancel_resp(uint8_t call_id, voip_cancel_type_enum_v02 cancel_type)
{
  voice_ims_call_cancel_resp_msg_v02 *ptr = (voice_ims_call_cancel_resp_msg_v02 *)payload;
  ptr->call_id = call_id;
  ptr->cancel_type = cancel_type;
}

/*========================================================
  QMI_VOICE_START_CONT_DTMF_REQ_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_start_cont_dtmf_req_msg(uint8_t call_id_val, uint8_t digit_val)
{
  payload = (void *)malloc(sizeof(voice_start_cont_dtmf_req_msg_v02));
  payload_len = sizeof(voice_start_cont_dtmf_req_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_START_CONT_DTMF_REQ_V02;

  //Mandatory TLV ..
  voice_start_cont_dtmf_req_msg_v02 *ptr = (voice_start_cont_dtmf_req_msg_v02 *) payload;
  (ptr->cont_dtmf_info).call_id = call_id_val;
  (ptr->cont_dtmf_info).digit = digit_val;
}

/*========================================================
  QMI_VOICE_START_CONT_DTMF_RESP_V02
========================================================*/

void ril_utf_qmiril_if::update_default_qmi_voice_start_cont_dtmf_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(voice_start_cont_dtmf_resp_msg_v02));
  payload_len = sizeof(voice_start_cont_dtmf_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_START_CONT_DTMF_RESP_V02;

  voice_start_cont_dtmf_resp_msg_v02 *ptr = (voice_start_cont_dtmf_resp_msg_v02 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

/*========================================================
  QMI_VOICE_STOP_CONT_DTMF_REQ_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_stop_cont_dtmf_req_msg(uint8_t call_id_val)
{
  payload = (void *)malloc(sizeof(voice_stop_cont_dtmf_req_msg_v02));
  payload_len = sizeof(voice_stop_cont_dtmf_req_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_STOP_CONT_DTMF_REQ_V02;

  //Mandatory TLV ..
  voice_stop_cont_dtmf_req_msg_v02 *ptr = (voice_stop_cont_dtmf_req_msg_v02 *) payload;
  ptr->call_id = call_id_val;
}

/*========================================================
  QMI_VOICE_STOP_CONT_DTMF_RESP_V02
========================================================*/

void ril_utf_qmiril_if::update_default_qmi_voice_stop_cont_dtmf_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(voice_stop_cont_dtmf_resp_msg_v02));
  payload_len = sizeof(voice_stop_cont_dtmf_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_STOP_CONT_DTMF_RESP_V02;

  voice_stop_cont_dtmf_resp_msg_v02 *ptr = (voice_stop_cont_dtmf_resp_msg_v02 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
/*========================================================
  QMI_VOICE_SEND_FLASH_REQ_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_send_flash_req_msg(uint8_t call_id_val)
{
  payload = (void *)malloc(sizeof(voice_send_flash_req_msg_v02));
  payload_len = sizeof(voice_send_flash_req_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_SEND_FLASH_REQ_V02;

  voice_send_flash_req_msg_v02 *ptr = (voice_send_flash_req_msg_v02 *) payload;
  ptr->call_id = call_id_val;
}
void ril_utf_qmiril_if::set_qmi_voice_send_flash_req_flash_payload(char *fp)
{
  voice_send_flash_req_msg_v02 *ptr = (voice_send_flash_req_msg_v02 *) payload;
  ptr->flash_payload_valid = TRUE;
  strlcpy(ptr->flash_payload, fp, strlen(fp)+1);
}
void ril_utf_qmiril_if::set_qmi_voice_send_flash_req_flash_type(voice_send_flash_type_enum_v02 val)
{
  voice_send_flash_req_msg_v02 *ptr = (voice_send_flash_req_msg_v02 *) payload;
  ptr->flash_type_valid = TRUE;
  ptr->flash_type = val;
}

/*========================================================
  QMI_VOICE_SEND_FLASH_RESP_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_send_flash_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(voice_send_flash_resp_msg_v02));
  payload_len = sizeof(voice_send_flash_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_SEND_FLASH_RESP_V02;

  voice_send_flash_resp_msg_v02 *ptr = (voice_send_flash_resp_msg_v02 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

/*========================================================
  QMI_VOICE_MANAGE_CALLS_REQ_V02
========================================================*/
void ril_utf_qmiril_if::update_default_voice_manage_calls_req_msg(sups_type_enum_v02 sups_type_val)
{
  payload = (void *)malloc(sizeof(voice_manage_calls_req_msg_v02));
  payload_len = sizeof(voice_manage_calls_req_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_MANAGE_CALLS_REQ_V02;

  voice_manage_calls_req_msg_v02 *ptr = (voice_manage_calls_req_msg_v02 *) payload;
  ptr->sups_type = sups_type_val;

}


/*========================================================
  QMI_VOICE_MANAGE_CALLS_RESP_V02
========================================================*/
void ril_utf_qmiril_if::update_default_voice_manage_calls_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(voice_manage_calls_resp_msg_v02));
  payload_len = sizeof(voice_manage_calls_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_MANAGE_CALLS_RESP_V02;

  voice_manage_calls_resp_msg_v02 *ptr = (voice_manage_calls_resp_msg_v02 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

/*
   QMI_VOICE_GET_SUPPORTED_MSGS_REQ_V02
*/
void ril_utf_qmiril_if::update_default_qmi_voice_get_supported_msgs_req_msg(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_GET_SUPPORTED_MSGS_REQ_V02;
}
/*
   QMI_VOICE_GET_SUPPORTED_MSGS_RESP_V02
*/
void ril_utf_qmiril_if::update_default_qmi_voice_get_supported_msgs_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(qmi_get_supported_msgs_resp_v01));
  payload_len = sizeof(qmi_get_supported_msgs_resp_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  msg_id = QMI_VOICE_GET_SUPPORTED_MSGS_RESP_V02;
  service_id = QMI_VOICE;

  qmi_get_supported_msgs_resp_v01 *qmi_resp = (qmi_get_supported_msgs_resp_v01 *) payload;
  qmi_resp->resp.error = error;
  qmi_resp->resp.result= result;

  qmi_resp->supported_msgs_valid = TRUE;
  int message_id = QMI_VOICE_SET_ALL_CALL_FWD_SUPS_REQ_V02;
  uint32_t index = message_id/8;
  uint8_t bit_position = message_id - index * 8;
  uint8_t bit_position_mask = 0x01 << bit_position;
  qmi_resp->supported_msgs[index] = bit_position_mask;
  qmi_resp->supported_msgs_len = index+1;
}


/*========================================================
  QMI_VOICE_INDICATION_REGISTER_REQ
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_ind_reg_req_msg(void)
{
  payload = (void *) malloc(sizeof(voice_indication_register_req_msg_v02));
  payload_len = sizeof(voice_indication_register_req_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_INDICATION_REGISTER_REQ_V02;
}

void ril_utf_qmiril_if::set_qmi_voice_ind_req_reg_voice_privacy_events(uint8_t val)
{
  voice_indication_register_req_msg_v02 *ptr = (voice_indication_register_req_msg_v02 *) payload;
  ptr->reg_voice_privacy_events_valid = TRUE;
  ptr->reg_voice_privacy_events       = val;
}

void ril_utf_qmiril_if::set_qmi_voice_ind_req_ext_brst_intl_events(uint8_t val)
{
  voice_indication_register_req_msg_v02 *ptr = (voice_indication_register_req_msg_v02 *) payload;
  ptr->ext_brst_intl_events_valid = TRUE;
  ptr->ext_brst_intl_events       = val;
}

void ril_utf_qmiril_if::set_qmi_voice_ind_req_speech_events(uint8_t val)
{
  voice_indication_register_req_msg_v02 *ptr = (voice_indication_register_req_msg_v02 *) payload;
  ptr->speech_events_valid = TRUE;
  ptr->speech_events       = val;
}

void ril_utf_qmiril_if::set_qmi_voice_ind_req_handover_events(uint8_t val)
{
  voice_indication_register_req_msg_v02 *ptr = (voice_indication_register_req_msg_v02 *) payload;
  ptr->handover_events_valid = TRUE;
  ptr->handover_events = val;
}

void ril_utf_qmiril_if::set_qmi_voice_ind_req_conference_events(uint8_t val)
{
  voice_indication_register_req_msg_v02 *ptr = (voice_indication_register_req_msg_v02 *) payload;
  ptr->conference_events_valid = TRUE;
  ptr->conference_events = val;
}
void ril_utf_qmiril_if::set_qmi_voice_ind_req_supps_notification_events(uint8_t val)
{
  voice_indication_register_req_msg_v02 *ptr = (voice_indication_register_req_msg_v02 *) payload;
  ptr->supps_notification_events_valid = TRUE;
  ptr->supps_notification_events = val;
}
void ril_utf_qmiril_if::set_qmi_voice_ind_req_tty_info_events (uint8_t tty_info_events)
{
  voice_indication_register_req_msg_v02 *ptr = (voice_indication_register_req_msg_v02 *) payload;
  ptr->tty_info_events_valid = TRUE;
  ptr->tty_info_events = tty_info_events;
}
void ril_utf_qmiril_if::set_qmi_voice_ind_req_reg_dtmf_events (uint8_t reg_dtmf_events)
{
  voice_indication_register_req_msg_v02 *ptr = (voice_indication_register_req_msg_v02 *) payload;
  ptr->reg_dtmf_events_valid = TRUE;
  ptr->reg_dtmf_events = reg_dtmf_events;
}
void ril_utf_qmiril_if::set_qmi_voice_ind_req_cc_result_events(uint8_t cc_result_events)
{
  voice_indication_register_req_msg_v02 *ptr = (voice_indication_register_req_msg_v02 *) payload;
  ptr->cc_result_events_valid = TRUE;
  ptr->cc_result_events = cc_result_events;
}
#ifndef UTF_TARGET_BUILD
void ril_utf_qmiril_if::set_qmi_voice_ind_req_additional_call_info_events (uint8_t additional_call_info_events)
{
  voice_indication_register_req_msg_v02 *ptr = (voice_indication_register_req_msg_v02 *) payload;
  ptr->additional_call_info_events_valid = TRUE;
  ptr->additional_call_info_events = additional_call_info_events;
}
void ril_utf_qmiril_if::set_qmi_voice_ind_req_audio_rat_change_events(uint8_t audio_rat_change_events)
{
  voice_indication_register_req_msg_v02 *ptr = (voice_indication_register_req_msg_v02 *) payload;
  ptr->audio_rat_change_events_valid = TRUE;
  ptr->audio_rat_change_events = audio_rat_change_events;
}
void ril_utf_qmiril_if::set_qmi_voice_ind_req_vice_dialog_event(uint8_t vice_dialog_event)
{
  voice_indication_register_req_msg_v02 *ptr = (voice_indication_register_req_msg_v02 *) payload;
  ptr->vice_dialog_event_valid = TRUE;
  ptr->vice_dialog_event = vice_dialog_event;
}
void ril_utf_qmiril_if::set_qmi_voice_ind_req_conf_participants_events(uint8_t conf_participants_events)
{
  voice_indication_register_req_msg_v02 *ptr = (voice_indication_register_req_msg_v02 *) payload;
  ptr->conf_participants_events_valid = TRUE;
  ptr->conf_participants_events = conf_participants_events;
}
#endif

/*========================================================
  QMI_VOICE_INDICATION_REGISTER_RESP
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_ind_reg_resp_msg(void)
{
  payload = (void *) malloc(sizeof(voice_indication_register_resp_msg_v02));
  payload_len = sizeof(voice_indication_register_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  msg_id = QMI_VOICE_INDICATION_REGISTER_RESP_V02;
  service_id = QMI_VOICE;
 /*
  voice_indication_register_resp_msg_v02 *ptr = (voice_indication_register_resp_msg_v02 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
 */
}
void ril_utf_qmiril_if::set_qmi_voice_ind_register_resp_msg
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  voice_indication_register_resp_msg_v02 *ptr =
    (voice_indication_register_resp_msg_v02 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}


#ifndef UTF_TARGET_BUILD
/*========================================================
  QMI_VOICE_SET_SUPS_SERVICE_REQ_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_set_sups_service_req_msg(voice_service_enum_v02 voice_service, voice_reason_enum_v02 reason)
{
  payload = (void *) malloc(sizeof(voice_set_sups_service_req_msg_v02));
  payload_len = sizeof(voice_set_sups_service_req_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_SET_SUPS_SERVICE_REQ_V02;

  voice_set_sups_service_req_msg_v02 *ptr = (voice_set_sups_service_req_msg_v02 *) payload;
  (ptr->supplementary_service_info).voice_service = voice_service;
  (ptr->supplementary_service_info).reason = reason;
}

void ril_utf_qmiril_if::set_qmi_voice_set_sups_service_req_service_class(uint8_t val)
{
  voice_set_sups_service_req_msg_v02 *ptr = (voice_set_sups_service_req_msg_v02 *) payload;
  ptr->service_class_valid = TRUE;
  ptr->service_class = val;
}
void ril_utf_qmiril_if::set_qmi_voice_set_sups_service_req_password(const char *p)
{
  voice_set_sups_service_req_msg_v02 *ptr = (voice_set_sups_service_req_msg_v02 *) payload;
  ptr->password_valid = TRUE;
  memcpy(ptr->password, p, std::min(sizeof(ptr->password), strlen(p)));
}
char* ril_utf_qmiril_if::set_qmi_voice_set_sups_service_req_number()
{
  voice_set_sups_service_req_msg_v02 *ptr = (voice_set_sups_service_req_msg_v02 *) payload;
  ptr->number_valid = TRUE;
  return (ptr->number);
}
void ril_utf_qmiril_if::set_qmi_voice_set_sups_service_req_timer_value(uint8_t val)
{
  voice_set_sups_service_req_msg_v02 *ptr = (voice_set_sups_service_req_msg_v02 *) payload;
  ptr->timer_value_valid = TRUE;
  ptr->timer_value = val;
}
void ril_utf_qmiril_if::set_qmi_voice_set_sups_service_req_colr_pi(pi_colr_enum_v02 val)
{
  voice_set_sups_service_req_msg_v02 *ptr = (voice_set_sups_service_req_msg_v02 *) payload;
  ptr->colr_pi_valid = TRUE;
  ptr->colr_pi = val;
}
voice_barred_number_type_v02 *ril_utf_qmiril_if::set_qmi_voice_set_sups_service_req_call_barring_numbers_list(uint32_t len)
{
  voice_set_sups_service_req_msg_v02 *ptr = (voice_set_sups_service_req_msg_v02 *) payload;
  ptr->call_barring_numbers_list_valid = TRUE;
  ptr->call_barring_numbers_list_len = len;
  return (ptr->call_barring_numbers_list);
}
#endif

void ril_utf_qmiril_if::set_qmi_voice_set_sups_service_req_call_fwd_start_time(
          uint16_t year, uint8_t month, uint8_t day, uint8_t hour,
          uint8_t minute, uint8_t second, int8_t time_zone)
{
  voice_set_sups_service_req_msg_v02 *ptr = (voice_set_sups_service_req_msg_v02 *) payload;
  ptr->call_fwd_start_time_valid = TRUE;
  ptr->call_fwd_start_time.year = year;
  ptr->call_fwd_start_time.month = month;
  ptr->call_fwd_start_time.day = day;
  ptr->call_fwd_start_time.hour = hour;
  ptr->call_fwd_start_time.minute = minute;
  ptr->call_fwd_start_time.second = second;
  ptr->call_fwd_start_time.time_zone = time_zone;
}
void ril_utf_qmiril_if::set_qmi_voice_set_sups_service_req_call_fwd_end_time(
          uint16_t year, uint8_t month, uint8_t day, uint8_t hour,
          uint8_t minute, uint8_t second, int8_t time_zone)
{
  voice_set_sups_service_req_msg_v02 *ptr = (voice_set_sups_service_req_msg_v02 *) payload;
  ptr->call_fwd_end_time_valid = TRUE;
  ptr->call_fwd_end_time.year = year;
  ptr->call_fwd_end_time.month = month;
  ptr->call_fwd_end_time.day = day;
  ptr->call_fwd_end_time.hour = hour;
  ptr->call_fwd_end_time.minute = minute;
  ptr->call_fwd_end_time.second = second;
  ptr->call_fwd_end_time.time_zone = time_zone;
}

/*========================================================
  QMI_VOICE_SET_SUPS_SERVICE_RESP_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_set_sups_service_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(voice_set_sups_service_resp_msg_v02));
  payload_len = sizeof(voice_set_sups_service_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_SET_SUPS_SERVICE_RSEP_V02;

  voice_set_sups_service_resp_msg_v02 *ptr = (voice_set_sups_service_resp_msg_v02 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_voice_set_sups_service_cc_result_type(voice_cc_result_type_enum_v02 val)
{
  voice_set_sups_service_resp_msg_v02* ptr = (voice_set_sups_service_resp_msg_v02 *)payload;
  ptr->cc_result_type_valid = TRUE;
  ptr->cc_result_type = val;
}
void ril_utf_qmiril_if::set_qmi_voice_set_sups_service_cc_sups_result(voice_cc_sups_result_service_type_enum_v02 service_type, voice_cc_sups_result_reason_enum_v02 reason)
{
  voice_set_sups_service_resp_msg_v02* ptr = (voice_set_sups_service_resp_msg_v02 *)payload;
  ptr->cc_sups_result_valid = TRUE;
  ptr->cc_sups_result.service_type = service_type;
  ptr->cc_sups_result.reason = reason;
}
void ril_utf_qmiril_if::set_qmi_voice_set_sups_service_call_id(uint8_t  val)
{
  voice_set_sups_service_resp_msg_v02* ptr = (voice_set_sups_service_resp_msg_v02 *)payload;
  ptr->call_id_valid = TRUE;
  ptr->call_id = val;
}
voice_alpha_ident_type_v02 *ril_utf_qmiril_if::set_qmi_voice_set_sups_service_alpha_ident()
{
  voice_set_sups_service_resp_msg_v02* ptr = (voice_set_sups_service_resp_msg_v02 *)payload;
  ptr->alpha_ident_valid = TRUE;
  return &(ptr->alpha_ident);
}
void ril_utf_qmiril_if::set_qmi_voice_set_sups_service_sip_error_code(uint32_t code)
{
  voice_set_sups_service_resp_msg_v02* ptr = (voice_set_sups_service_resp_msg_v02 *)payload;
  ptr->sip_error_code_valid = TRUE;
  ptr->sip_error_code = code;
}
void ril_utf_qmiril_if::set_qmi_voice_set_sups_service_resp_failure_cause(qmi_sups_errors_enum_v02 val)
{
  voice_set_sups_service_resp_msg_v02* ptr = (voice_set_sups_service_resp_msg_v02 *)payload;
  ptr->failure_cause_valid = TRUE;
  ptr->failure_cause = val;
}
void ril_utf_qmiril_if::set_qmi_voice_set_sups_service_failure_cause_description(uint32_t len, uint16_t val[])
{
  voice_set_sups_service_resp_msg_v02* ptr = (voice_set_sups_service_resp_msg_v02 *)payload;
  ptr->failure_cause_description_valid = TRUE;
  ptr->failure_cause_description_len = len;
  for (int i = 0; i < len && i < QMI_VOICE_FAILURE_CAUSE_DESC_MAX_LEN_V02; i++)
  {
    ptr->failure_cause_description[i] = val[i];
  }
}

/*========================================================
  QMI_VOICE_SET_ALL_CALL_FWD_SUPS_REQ_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_set_all_call_fwd_sups_req_msg(voice_service_enum_v02 voice_service, voice_reason_enum_v02 reason)
{
  payload = (void *) malloc(sizeof(voice_set_all_call_fwd_sups_req_msg_v02));
  payload_len = sizeof(voice_set_all_call_fwd_sups_req_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_SET_ALL_CALL_FWD_SUPS_REQ_V02;

  voice_set_all_call_fwd_sups_req_msg_v02 *ptr = (voice_set_all_call_fwd_sups_req_msg_v02 *) payload;
  ptr->voice_service = voice_service;
  ptr->reason_valid = TRUE;
  ptr->reason = reason;
}
void ril_utf_qmiril_if::set_qmi_voice_set_all_call_fwd_sups_req_service_class(voice_service_class_enum_v02 val)
{
  voice_set_all_call_fwd_sups_req_msg_v02 *ptr = (voice_set_all_call_fwd_sups_req_msg_v02 *) payload;
  ptr->service_class_valid = TRUE;
  ptr->service_class = val;
}
void ril_utf_qmiril_if::set_qmi_voice_set_all_call_fwd_sups_req_number(char *number)
{
  voice_set_all_call_fwd_sups_req_msg_v02 *ptr = (voice_set_all_call_fwd_sups_req_msg_v02 *) payload;
  if (number)
  {
    ptr->number_valid = TRUE;
    strlcpy(ptr->number, number, strlen(number)+1);
  }
}
void ril_utf_qmiril_if::set_qmi_voice_set_all_call_fwd_sups_req_timer_value(uint8_t val)
{
  voice_set_all_call_fwd_sups_req_msg_v02 *ptr = (voice_set_all_call_fwd_sups_req_msg_v02 *) payload;
  ptr->timer_value_valid = TRUE;
  ptr->timer_value = val;
}

void ril_utf_qmiril_if::set_qmi_voice_set_all_call_fwd_sups_req_call_fwd_start_time(
          uint16_t year, uint8_t month, uint8_t day, uint8_t hour,
          uint8_t minute, uint8_t second, int8_t time_zone)
{
  voice_set_all_call_fwd_sups_req_msg_v02 *ptr = (voice_set_all_call_fwd_sups_req_msg_v02 *) payload;
  ptr->call_fwd_start_time_valid = TRUE;
  ptr->call_fwd_start_time.year = year;
  ptr->call_fwd_start_time.month = month;
  ptr->call_fwd_start_time.day = day;
  ptr->call_fwd_start_time.hour = hour;
  ptr->call_fwd_start_time.minute = minute;
  ptr->call_fwd_start_time.second = second;
  ptr->call_fwd_start_time.time_zone = time_zone;
}
void ril_utf_qmiril_if::set_qmi_voice_set_all_call_fwd_sups_req_call_fwd_end_time(
          uint16_t year, uint8_t month, uint8_t day, uint8_t hour,
          uint8_t minute, uint8_t second, int8_t time_zone)
{
  voice_set_all_call_fwd_sups_req_msg_v02 *ptr = (voice_set_all_call_fwd_sups_req_msg_v02 *) payload;
  ptr->call_fwd_end_time_valid = TRUE;
  ptr->call_fwd_end_time.year = year;
  ptr->call_fwd_end_time.month = month;
  ptr->call_fwd_end_time.day = day;
  ptr->call_fwd_end_time.hour = hour;
  ptr->call_fwd_end_time.minute = minute;
  ptr->call_fwd_end_time.second = second;
  ptr->call_fwd_end_time.time_zone = time_zone;
}

/*========================================================
  QMI_VOICE_SET_ALL_CALL_FWD_SUPS_RESP_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_set_all_call_fwd_sups_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(voice_set_all_call_fwd_sups_resp_msg_v02));
  payload_len = sizeof(voice_set_all_call_fwd_sups_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_SET_ALL_CALL_FWD_SUPS_RESP_V02;

  voice_set_all_call_fwd_sups_resp_msg_v02 *ptr = (voice_set_all_call_fwd_sups_resp_msg_v02 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_voice_set_all_call_fwd_sups_resp_cfu_result(
      uint8_t cfu_result_valid, qmi_result_type_v01 cfu_result,
      uint8_t cfu_failure_cause_valid, qmi_sups_errors_enum_v02 cfu_failure_cause,
      uint8_t cfu_failure_cause_description_valid, uint16_t cfu_failure_cause_description[QMI_VOICE_FAILURE_CAUSE_DESC_MAX_LEN_V02+1],
      uint8_t cfu_retry_duration_valid, uint16_t cfu_retry_duration,
      uint8_t cfu_sip_error_code_valid, uint16_t cfu_sip_error_code)
{
  voice_set_all_call_fwd_sups_resp_msg_v02* ptr = (voice_set_all_call_fwd_sups_resp_msg_v02 *)payload;
  ptr->cfu_result_valid = cfu_result_valid;
  ptr->cfu_result = cfu_result;
  ptr->cfu_failure_cause_valid = cfu_failure_cause_valid;
  ptr->cfu_failure_cause = cfu_failure_cause;
  ptr->cfu_failure_cause_description_valid = TRUE;
  memcpy(ptr->cfu_failure_cause_description, cfu_failure_cause_description, sizeof(ptr->cfu_failure_cause_description));
#if 0
  for (int i = 0; i < cfu_failure_cause_description_len && i < QMI_VOICE_FAILURE_CAUSE_DESC_MAX_LEN_V02; i++)
  {
    ptr->cfu_failure_cause_description[i] = cfu_failure_cause_description[i];
  }
#endif
  ptr->cfu_retry_duration_valid = cfu_retry_duration_valid;
  ptr->cfu_retry_duration = cfu_retry_duration;
  ptr->cfu_sip_error_code_valid = cfu_sip_error_code_valid;
  ptr->cfu_sip_error_code = cfu_sip_error_code;
}
void ril_utf_qmiril_if::set_qmi_voice_set_all_call_fwd_sups_resp_cfb_result(
    uint8_t cfb_result_valid, qmi_result_type_v01 cfb_result,
    uint8_t cfb_failure_cause_valid, qmi_sups_errors_enum_v02 cfb_failure_cause,
    uint8_t cfb_failure_cause_description_valid, uint16_t cfb_failure_cause_description[QMI_VOICE_FAILURE_CAUSE_DESC_MAX_LEN_V02+1],
    uint8_t cfb_retry_duration_valid, uint16_t cfb_retry_duration,
    uint8_t cfb_sip_error_code_valid, uint16_t cfb_sip_error_code)
{
  voice_set_all_call_fwd_sups_resp_msg_v02* ptr = (voice_set_all_call_fwd_sups_resp_msg_v02 *)payload;
  ptr->cfb_result_valid = cfb_result_valid;
  ptr->cfb_result = cfb_result;
  ptr->cfb_failure_cause_valid = cfb_failure_cause_valid;
  ptr->cfb_failure_cause = cfb_failure_cause;
  ptr->cfb_failure_cause_description_valid = TRUE;
  memcpy(ptr->cfb_failure_cause_description, cfb_failure_cause_description, sizeof(ptr->cfb_failure_cause_description));
  ptr->cfb_retry_duration_valid = cfb_retry_duration_valid;
  ptr->cfb_retry_duration = cfb_retry_duration;
  ptr->cfb_sip_error_code_valid = cfb_sip_error_code_valid;
  ptr->cfb_sip_error_code = cfb_sip_error_code;
}
void ril_utf_qmiril_if::set_qmi_voice_set_all_call_fwd_sups_resp_cfnry_result(
    uint8_t cfnry_result_valid, qmi_result_type_v01 cfnry_result,
    uint8_t cfnry_failure_cause_valid, qmi_sups_errors_enum_v02 cfnry_failure_cause,
    uint8_t cfnry_failure_cause_description_valid, uint16_t cfnry_failure_cause_description[QMI_VOICE_FAILURE_CAUSE_DESC_MAX_LEN_V02+1],
    uint8_t cfnry_retry_duration_valid, uint16_t cfnry_retry_duration,
    uint8_t cfnry_sip_error_code_valid, uint16_t cfnry_sip_error_code)
{
  voice_set_all_call_fwd_sups_resp_msg_v02* ptr = (voice_set_all_call_fwd_sups_resp_msg_v02 *)payload;
  ptr->cfnry_result_valid = cfnry_result_valid;
  ptr->cfnry_result = cfnry_result;
  ptr->cfnry_failure_cause_valid = cfnry_failure_cause_valid;
  ptr->cfnry_failure_cause = cfnry_failure_cause;
  ptr->cfnry_failure_cause_description_valid = TRUE;
  memcpy(ptr->cfnry_failure_cause_description, cfnry_failure_cause_description, sizeof(ptr->cfnry_failure_cause_description));
  ptr->cfnry_retry_duration_valid = cfnry_retry_duration_valid;
  ptr->cfnry_retry_duration = cfnry_retry_duration;
  ptr->cfnry_sip_error_code_valid = cfnry_sip_error_code_valid;
  ptr->cfnry_sip_error_code = cfnry_sip_error_code;
}
void ril_utf_qmiril_if::set_qmi_voice_set_all_call_fwd_sups_resp_cfnrc_result(
    uint8_t cfnrc_result_valid, qmi_result_type_v01 cfnrc_result,
    uint8_t cfnrc_failure_cause_valid, qmi_sups_errors_enum_v02 cfnrc_failure_cause,
    uint8_t cfnrc_failure_cause_description_valid, uint16_t cfnrc_failure_cause_description[QMI_VOICE_FAILURE_CAUSE_DESC_MAX_LEN_V02+1],
    uint8_t cfnrc_retry_duration_valid, uint16_t cfnrc_retry_duration,
    uint8_t cfnrc_sip_error_code_valid, uint16_t cfnrc_sip_error_code)
{
  voice_set_all_call_fwd_sups_resp_msg_v02* ptr = (voice_set_all_call_fwd_sups_resp_msg_v02 *)payload;
  ptr->cfnrc_result_valid = cfnrc_result_valid;
  ptr->cfnrc_result = cfnrc_result;
  ptr->cfnrc_failure_cause_valid = cfnrc_failure_cause_valid;
  ptr->cfnrc_failure_cause = cfnrc_failure_cause;
  ptr->cfnrc_failure_cause_description_valid = TRUE;
  memcpy(ptr->cfnrc_failure_cause_description, cfnrc_failure_cause_description, sizeof(ptr->cfnrc_failure_cause_description));
  ptr->cfnrc_retry_duration_valid = cfnrc_retry_duration_valid;
  ptr->cfnrc_retry_duration = cfnrc_retry_duration;
  ptr->cfnrc_sip_error_code_valid = cfnrc_sip_error_code_valid;
  ptr->cfnrc_sip_error_code = cfnrc_sip_error_code;
}

void ril_utf_qmiril_if::set_qmi_voice_set_all_call_fwd_sups_resp_cc_result_type(voice_cc_result_type_enum_v02 val)
{
  voice_set_all_call_fwd_sups_resp_msg_v02* ptr = (voice_set_all_call_fwd_sups_resp_msg_v02 *)payload;
  ptr->cc_result_type_valid = TRUE;
  ptr->cc_result_type = val;
}
void ril_utf_qmiril_if::set_qmi_voice_set_all_call_fwd_sups_resp_cc_sups_result(voice_cc_sups_result_service_type_enum_v02 service_type, voice_cc_sups_result_reason_enum_v02 reason)
{
  voice_set_all_call_fwd_sups_resp_msg_v02* ptr = (voice_set_all_call_fwd_sups_resp_msg_v02 *)payload;
  ptr->cc_sups_result_valid = TRUE;
  ptr->cc_sups_result.service_type = service_type;
  ptr->cc_sups_result.reason = reason;
}
void ril_utf_qmiril_if::set_qmi_voice_set_all_call_fwd_sups_resp_call_id(uint8_t  val)
{
  voice_set_all_call_fwd_sups_resp_msg_v02* ptr = (voice_set_all_call_fwd_sups_resp_msg_v02 *)payload;
  ptr->call_id_valid = TRUE;
  ptr->call_id = val;
}
voice_alpha_ident_type_v02 *ril_utf_qmiril_if::set_qmi_voice_set_all_call_fwd_sups_resp_alpha_ident()
{
  voice_set_all_call_fwd_sups_resp_msg_v02* ptr = (voice_set_all_call_fwd_sups_resp_msg_v02 *)payload;
  ptr->alpha_ident_valid = TRUE;
  return &(ptr->alpha_ident);
}
void ril_utf_qmiril_if::set_qmi_voice_set_all_call_fwd_sups_resp_sip_error_code(uint32_t code)
{
  voice_set_all_call_fwd_sups_resp_msg_v02* ptr = (voice_set_all_call_fwd_sups_resp_msg_v02 *)payload;
  ptr->sip_error_code_valid = TRUE;
  ptr->sip_error_code = code;
}
void ril_utf_qmiril_if::set_qmi_voice_set_all_call_fwd_sups_resp_failure_cause(qmi_sups_errors_enum_v02 val)
{
  voice_set_all_call_fwd_sups_resp_msg_v02* ptr = (voice_set_all_call_fwd_sups_resp_msg_v02 *)payload;
  ptr->failure_cause_valid = TRUE;
  ptr->failure_cause = val;
}
void ril_utf_qmiril_if::set_qmi_voice_set_all_call_fwd_sups_resp_failure_cause_description(uint16_t failure_cause_description[QMI_VOICE_FAILURE_CAUSE_DESC_MAX_LEN_V02+1])
{
  voice_set_all_call_fwd_sups_resp_msg_v02* ptr = (voice_set_all_call_fwd_sups_resp_msg_v02 *)payload;
  ptr->failure_cause_description_valid = TRUE;
  memcpy(ptr->failure_cause_description, failure_cause_description, sizeof(ptr->failure_cause_description));
}

/*========================================================
  QMI_VOICE_GET_CLIP_REQ_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_get_clip_req_v02()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_GET_CLIP_REQ_V02;
}

/*========================================================
  QMI_VOICE_GET_CLIP_RESP_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_get_clip_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(voice_get_clip_resp_msg_v02));
  payload_len = sizeof(voice_get_clip_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_GET_CLIP_RESP_V02;

  voice_get_clip_resp_msg_v02 *ptr = (voice_get_clip_resp_msg_v02 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;

}
void ril_utf_qmiril_if::set_qmi_voice_get_clip_resp_msg_clip_response
  (active_status_enum_v02 active_status, provision_status_enum_v02 provision_status)
{
  voice_get_clip_resp_msg_v02 *ptr = (voice_get_clip_resp_msg_v02 *) payload;
  ptr->clip_response_valid = TRUE;
  ptr->clip_response.active_status = active_status;
  ptr->clip_response.provision_status = provision_status;
}
void ril_utf_qmiril_if::set_qmi_voice_get_clip_resp_msg_sip_error_code(uint16_t val)
{
  voice_get_clip_resp_msg_v02 *ptr = (voice_get_clip_resp_msg_v02 *) payload;
  ptr->sip_error_code_valid = TRUE;
  ptr->sip_error_code = val;
}
void ril_utf_qmiril_if::set_qmi_voice_get_clip_resp_msg_failure_cause_description(uint32_t len, uint16_t val[])
{
  voice_get_clip_resp_msg_v02 *ptr = (voice_get_clip_resp_msg_v02 *) payload;
  ptr->failure_cause_description_valid = TRUE;
  ptr->failure_cause_description_len = len;
  for (int i = 0; i < len && i < QMI_VOICE_FAILURE_CAUSE_DESC_MAX_LEN_V02; i++)
  {
    ptr->failure_cause_description[i] = val[i];
  }
}
void ril_utf_qmiril_if::set_qmi_voice_get_clip_resp_msg_cc_result_type(voice_cc_result_type_enum_v02  val)
{
  voice_get_clip_resp_msg_v02* ptr = (voice_get_clip_resp_msg_v02 *)payload;
  ptr->cc_result_type_valid = TRUE;
  ptr->cc_result_type = val;
}
void ril_utf_qmiril_if::set_qmi_voice_get_clip_resp_msg_cc_sups_result(voice_cc_sups_result_service_type_enum_v02 service_type, voice_cc_sups_result_reason_enum_v02 reason)
{
  voice_get_clip_resp_msg_v02* ptr = (voice_get_clip_resp_msg_v02 *)payload;
  ptr->cc_sups_result_valid = TRUE;
  ptr->cc_sups_result.service_type = service_type;
  ptr->cc_sups_result.reason = reason;
}

/*========================================================
  QMI_VOICE_GET_COLP_REQ_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_get_colp_req_v02(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_GET_COLP_REQ_V02;
}

/*========================================================
  QMI_VOICE_GET_COLP_RESP_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_get_colp_resp_msg(qmi_result_type_v01 result,
                                                                   qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(voice_get_colp_resp_msg_v02));
  payload_len = sizeof(voice_get_colp_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_GET_COLP_RESP_V02;

  voice_get_colp_resp_msg_v02 *ptr = (voice_get_colp_resp_msg_v02 *)payload;
  ptr->resp.error = error;
  ptr->resp.result = result;
}

void ril_utf_qmiril_if::set_qmi_voice_get_colp_resp_msg_colp_response(
    active_status_enum_v02 active_status, provision_status_enum_v02 provision_status)
{
  voice_get_colp_resp_msg_v02 *ptr = (voice_get_colp_resp_msg_v02 *)payload;
  ptr->colp_response_valid = TRUE;
  ptr->colp_response.active_status = active_status;
  ptr->colp_response.provision_status = provision_status;
}
void ril_utf_qmiril_if::set_qmi_voice_get_colp_resp_sip_error_code(uint16_t val)
{
  voice_get_colp_resp_msg_v02 *ptr = (voice_get_colp_resp_msg_v02 *) payload;
  ptr->sip_error_code_valid = TRUE;
  ptr->sip_error_code = val;
}
void ril_utf_qmiril_if::set_qmi_voice_get_colp_resp_failure_cause_description(
    uint32_t len, uint16_t val[])
{
  voice_get_colp_resp_msg_v02 *ptr = (voice_get_colp_resp_msg_v02 *)payload;
  ptr->failure_cause_description_valid = TRUE;
  ptr->failure_cause_description_len = len;
  for (int i = 0; i < len && i < QMI_VOICE_FAILURE_CAUSE_DESC_MAX_LEN_V02; i++) {
    ptr->failure_cause_description[i] = val[i];
  }
}

/*========================================================
  QMI_VOICE_GET_CLIR_REQ_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_get_clir_req_msg()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_GET_CLIR_REQ_V02;
}

/*========================================================
  QMI_VOICE_GET_CLIR_RESP_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_get_clir_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(voice_get_clir_resp_msg_v02));
  payload_len = sizeof(voice_get_clir_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_GET_CLIR_RESP_V02;

  voice_get_clir_resp_msg_v02 *ptr = (voice_get_clir_resp_msg_v02 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

void ril_utf_qmiril_if::set_qmi_void_get_clir_resp_clir_response
  (active_status_enum_v02 active_status, provision_status_enum_v02 provision_status)
{
  voice_get_clir_resp_msg_v02 *ptr =
    (voice_get_clir_resp_msg_v02 *) payload;
  ptr->clir_response_valid = TRUE;
  ptr->clir_response.active_status = active_status;
  ptr->clir_response.provision_status = provision_status;
}

void ril_utf_qmiril_if::set_qmi_void_get_clir_resp_failure_cause (qmi_sups_errors_enum_v02 failure_cause)
{
  voice_get_clir_resp_msg_v02 *ptr = (voice_get_clir_resp_msg_v02 *) payload;
  ptr->failure_cause_valid = TRUE;
  ptr->failure_cause       = failure_cause;
}


/*========================================================
  QMI_VOICE_GET_CALL_WAITING_REQ_V02
========================================================*/

void ril_utf_qmiril_if::update_default_qmi_voice_get_call_waiting_req_msg(void)
{
  payload = (void *) malloc(sizeof(voice_get_call_waiting_req_msg_v02));
  payload_len = sizeof(voice_get_call_waiting_req_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_GET_CALL_WAITING_REQ_V02;
}
void ril_utf_qmiril_if::set_qmi_voice_get_call_waiting_req_service_class(uint8_t val)
{
  voice_get_call_waiting_req_msg_v02 *ptr = (voice_get_call_waiting_req_msg_v02 *) payload;
  ptr->service_class_valid = TRUE;
  ptr->service_class = val;
}

/*========================================================
  QMI_VOICE_GET_CALL_WAITING_RESP_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_get_call_waiting_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(voice_get_call_waiting_resp_msg_v02));
  payload_len = sizeof(voice_get_call_waiting_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_GET_CALL_WAITING_RESP_V02;

  voice_get_call_waiting_resp_msg_v02 *ptr = (voice_get_call_waiting_resp_msg_v02 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

void ril_utf_qmiril_if::set_qmi_voice_get_call_waiting_resp_service_class(uint8_t val)
{
  voice_get_call_waiting_resp_msg_v02 *ptr = (voice_get_call_waiting_resp_msg_v02 *) payload;
  ptr->service_class_valid = TRUE;
  ptr->service_class = val;
}
void ril_utf_qmiril_if::set_qmi_voice_get_call_waiting_resp_sip_error_code(uint16_t val)
{
  voice_get_call_waiting_resp_msg_v02 *ptr = (voice_get_call_waiting_resp_msg_v02 *) payload;
  ptr->sip_error_code_valid = TRUE;
  ptr->sip_error_code = val;
}
void ril_utf_qmiril_if::set_qmi_voice_get_call_waiting_resp_failure_cause_description(uint32_t len, uint16_t val[])
{
  voice_get_call_waiting_resp_msg_v02 *ptr = (voice_get_call_waiting_resp_msg_v02 *) payload;
  ptr->failure_cause_description_valid = TRUE;
  ptr->failure_cause_description_len = len;
  for (int i = 0; i < len && i < QMI_VOICE_FAILURE_CAUSE_DESC_MAX_LEN_V02; i++)
  {
    ptr->failure_cause_description[i] = val[i];
  }
}

/*========================================================
  QMI_VOICE_BURST_DTMF_REQ_V02
========================================================*/
voice_burst_dtmf_info_type_v02 *ril_utf_qmiril_if::update_default_qmi_voice_burst_dtmf_req_msg_msg()
{
  payload = (void *) malloc(sizeof(voice_burst_dtmf_req_msg_v02));
  payload_len = sizeof(voice_burst_dtmf_req_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_BURST_DTMF_REQ_V02;

  voice_burst_dtmf_req_msg_v02 *ptr = (voice_burst_dtmf_req_msg_v02 *) payload;
  return &(ptr->burst_dtmf_info);
}
void ril_utf_qmiril_if::set_qmi_voice_burst_dtmf_req_dtmf_lengths(dtmf_onlength_enum_v02 on, dtmf_offlength_enum_v02 off)
{
  voice_burst_dtmf_req_msg_v02 *ptr = (voice_burst_dtmf_req_msg_v02 *) payload;
  ptr->dtmf_lengths_valid = TRUE;
  ptr->dtmf_lengths.dtmf_onlength = on;
  ptr->dtmf_lengths.dtmf_offlength = off;
}

/*========================================================
  QMI_VOICE_BURST_DTMF_RESP_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_burst_dtmf_resp_msg_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(voice_burst_dtmf_resp_msg_v02));
  payload_len = sizeof(voice_burst_dtmf_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_BURST_DTMF_RESP_V02;

  voice_burst_dtmf_resp_msg_v02 *ptr = (voice_burst_dtmf_resp_msg_v02 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

/*========================================================
  QMI_VOICE_SET_PREFERRED_PRIVACY_REQ_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_set_preferred_privacy_req_msg()
{
  payload = (void *) malloc(sizeof(voice_set_preferred_privacy_req_msg_v02));
  payload_len = sizeof(voice_set_preferred_privacy_req_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_SET_PREFERRED_PRIVACY_REQ_V02;
}
void ril_utf_qmiril_if::set_qmi_voice_set_preferred_privacy_req_privacy_pref(voice_privacy_enum_v02 val)
{
  voice_set_preferred_privacy_req_msg_v02 *ptr = (voice_set_preferred_privacy_req_msg_v02 *) payload;
  ptr->privacy_pref = val;
}

/*========================================================
  QMI_VOICE_SET_PREFERRED_PRIVACY_RESP_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_set_preferred_privacy_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(voice_set_preferred_privacy_resp_msg_v02));
  payload_len = sizeof(voice_set_preferred_privacy_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_SET_PREFERRED_PRIVACY_RESP_V02;

  voice_set_preferred_privacy_resp_msg_v02 *ptr = (voice_set_preferred_privacy_resp_msg_v02 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

/*========================================================
  QMI_VOICE_SET_CALL_BARRING_PASSWORD_REQ_V02
========================================================*/
voice_call_barring_password_info_type_v02* ril_utf_qmiril_if::update_default_qmi_voice_set_call_barring_password_req_msg()
{
  payload = (void *) malloc(sizeof(voice_set_call_barring_password_req_msg_v02));
  payload_len = sizeof(voice_set_call_barring_password_req_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_SET_CALL_BARRING_PASSWORD_REQ_V02;

  voice_set_call_barring_password_req_msg_v02 *ptr = (voice_set_call_barring_password_req_msg_v02 *) payload;
  return &(ptr->call_barring_password_info);
}

/*========================================================
  QMI_VOICE_SET_CALL_BARRING_PASSWORD_RESP_V02
========================================================*/

void ril_utf_qmiril_if::update_default_qmi_voice_set_call_barring_password_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(voice_set_call_barring_password_resp_msg_v02));
  payload_len = sizeof(voice_set_call_barring_password_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_SET_CALL_BARRING_PASSWORD_RESP_V02;

  voice_set_call_barring_password_resp_msg_v02 *ptr = (voice_set_call_barring_password_resp_msg_v02 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_voice_set_call_barring_password_resp_failure_cause(qmi_sups_errors_enum_v02 val)
{
  voice_set_call_barring_password_resp_msg_v02 *ptr = (voice_set_call_barring_password_resp_msg_v02 *) payload;
  ptr->failure_cause_valid = TRUE;
  ptr->failure_cause = val;
}


/*========================================================
 QMI_VOICE_GET_CALL_BARRING_REQ_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_get_call_barring_req_msg(voice_reason_enum_v02 reason)
{
  payload = (void *) malloc(sizeof(voice_get_call_barring_req_msg_v02));
  payload_len = sizeof(voice_get_call_barring_req_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_GET_CALL_BARRING_REQ_V02;

  voice_get_call_barring_req_msg_v02 *ptr = (voice_get_call_barring_req_msg_v02 *) payload;
  ptr->reason = reason;

}
void ril_utf_qmiril_if::set_voice_get_call_barring_req_service_class(uint8_t val)
{
  voice_get_call_barring_req_msg_v02 *ptr = (voice_get_call_barring_req_msg_v02 *) payload;
  ptr->service_class_valid = TRUE;
  ptr->service_class = val;
}

/*========================================================
 QMI_VOICE_GET_CALL_BARRING_RESP_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_get_call_barring_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(voice_get_call_barring_resp_msg_v02));
  payload_len = sizeof(voice_get_call_barring_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_GET_CALL_BARRING_RESP_V02;

  voice_get_call_barring_resp_msg_v02 *ptr = (voice_get_call_barring_resp_msg_v02 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_voice_get_call_barring_resp_msg_service_class(uint8_t service_class)
{
  voice_get_call_barring_resp_msg_v02 *ptr = (voice_get_call_barring_resp_msg_v02 *) payload;
  ptr->service_class_valid = TRUE;
  ptr->service_class = service_class;
}
void ril_utf_qmiril_if::set_qmi_voice_get_call_barring_resp_msg_SS_service_type(
    voice_cc_sups_result_reason_enum_v02 reason) {
  voice_get_call_barring_resp_msg_v02 *ptr = (voice_get_call_barring_resp_msg_v02 *)payload;
  ptr->cc_sups_result_valid = TRUE;
  ptr->cc_sups_result.reason = reason;
}
void ril_utf_qmiril_if::set_qmi_voice_get_call_barring_resp_msg_sip_error_code(uint16_t val)
{
  voice_get_call_barring_resp_msg_v02 *ptr = (voice_get_call_barring_resp_msg_v02 *) payload;
  ptr->sip_error_code_valid = TRUE;
  ptr->sip_error_code = val;
}
void ril_utf_qmiril_if::set_qmi_voice_get_call_barring_resp_msg_failure_cause_description(
    uint32_t len, uint16_t val[])
{
  voice_get_call_barring_resp_msg_v02 *ptr = (voice_get_call_barring_resp_msg_v02 *)payload;
  ptr->failure_cause_description_valid = TRUE;
  ptr->failure_cause_description_len = len;
  for (int i = 0; i < len && i < QMI_VOICE_FAILURE_CAUSE_DESC_MAX_LEN_V02; i++) {
    ptr->failure_cause_description[i] = val[i];
  }
}
voice_serviceclass_barred_number_list_type_v02 *ril_utf_qmiril_if::set_qmi_voice_get_call_barring_resp_msg_sc_barred_numbers_status_list(uint32_t len)
{
  voice_get_call_barring_resp_msg_v02 *ptr = (voice_get_call_barring_resp_msg_v02 *) payload;
  ptr->sc_barred_numbers_status_list_valid = TRUE;
  ptr->sc_barred_numbers_status_list_len = len;
  return (ptr->sc_barred_numbers_status_list);
}

/*========================================================
  QMI_VOICE_GET_CALL_FORWARDING_REQ_V02
========================================================*/

void ril_utf_qmiril_if::update_default_qmi_voice_get_call_forwarding_req_msg(voice_reason_enum_v02 reason)
{
  payload = (void *) malloc(sizeof(voice_get_call_forwarding_req_msg_v02));
  payload_len = sizeof(voice_get_call_forwarding_req_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_GET_CALL_FORWARDING_REQ_V02;

  voice_get_call_forwarding_req_msg_v02 *ptr = (voice_get_call_forwarding_req_msg_v02 *) payload;
  ptr->reason = reason;

}
void ril_utf_qmiril_if::set_voice_get_call_forwarding_req_service_class(uint8_t val)
{
  voice_get_call_forwarding_req_msg_v02 *ptr = (voice_get_call_forwarding_req_msg_v02 *) payload;
  ptr->service_class_valid = TRUE;
  ptr->service_class = val;
}

/*========================================================
  QMI_VOICE_GET_CALL_FORWARDING_RESP_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_get_call_forwarding_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(voice_get_call_forwarding_resp_msg_v02));
  payload_len = sizeof(voice_get_call_forwarding_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_GET_CALL_FORWARDING_RESP_V02;

  voice_get_call_forwarding_resp_msg_v02 *ptr = (voice_get_call_forwarding_resp_msg_v02 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}


voice_get_call_forwarding_info_type_v02 *ril_utf_qmiril_if::set_qmi_voice_get_call_forwarding_resp_get_call_forwarding_info (uint32_t get_call_forwarding_info_len)
{
  voice_get_call_forwarding_resp_msg_v02 *ptr = (voice_get_call_forwarding_resp_msg_v02 *) payload;
  ptr->get_call_forwarding_info_valid = TRUE;
  ptr->get_call_forwarding_info_len = get_call_forwarding_info_len;

  return ptr->get_call_forwarding_info;
}

void ril_utf_qmiril_if::set_qmi_voice_get_call_forwarding_resp_call_fwd_start_time(
          uint16_t year, uint8_t month, uint8_t day, uint8_t hour,
          uint8_t minute, uint8_t second, int8_t time_zone)
{
  voice_get_call_forwarding_resp_msg_v02 *ptr = (voice_get_call_forwarding_resp_msg_v02 *) payload;
  ptr->call_fwd_start_time_valid = TRUE;
  ptr->call_fwd_start_time.year = year;
  ptr->call_fwd_start_time.month = month;
  ptr->call_fwd_start_time.day = day;
  ptr->call_fwd_start_time.hour = hour;
  ptr->call_fwd_start_time.minute = minute;
  ptr->call_fwd_start_time.second = second;
  ptr->call_fwd_start_time.time_zone = time_zone;
}
void ril_utf_qmiril_if::set_qmi_voice_get_call_forwarding_resp_call_fwd_end_time(
          uint16_t year, uint8_t month, uint8_t day, uint8_t hour,
          uint8_t minute, uint8_t second, int8_t time_zone)
{
  voice_get_call_forwarding_resp_msg_v02 *ptr = (voice_get_call_forwarding_resp_msg_v02 *) payload;
  ptr->call_fwd_end_time_valid = TRUE;
  ptr->call_fwd_end_time.year = year;
  ptr->call_fwd_end_time.month = month;
  ptr->call_fwd_end_time.day = day;
  ptr->call_fwd_end_time.hour = hour;
  ptr->call_fwd_end_time.minute = minute;
  ptr->call_fwd_end_time.second = second;
  ptr->call_fwd_end_time.time_zone = time_zone;
}
void ril_utf_qmiril_if::set_qmi_voice_get_call_forwarding_resp_sip_error_code(uint16_t sip_error_code)
{
  voice_get_call_forwarding_resp_msg_v02 *ptr = (voice_get_call_forwarding_resp_msg_v02 *) payload;
  ptr->sip_error_code_valid = TRUE;
  ptr->sip_error_code = sip_error_code;
}
void ril_utf_qmiril_if::set_qmi_voice_get_call_forwarding_resp_failure_cause_description(uint32_t len, uint16_t val[])
{
  voice_get_call_forwarding_resp_msg_v02 *ptr = (voice_get_call_forwarding_resp_msg_v02 *) payload;
  ptr->failure_cause_description_valid = TRUE;
  ptr->failure_cause_description_len = len;
  for (int i = 0; i < len && i < QMI_VOICE_FAILURE_CAUSE_DESC_MAX_LEN_V02; i++)
  {
    ptr->failure_cause_description[i] = val[i];
  }
}
/*========================================================
  QMI_VOICE_ORIG_USSD_REQ_V02
========================================================*/
voice_uss_info_type_v02* ril_utf_qmiril_if::update_default_qmi_voice_orig_ussd_req_msg()
{
  payload = (void *) malloc(sizeof(voice_orig_ussd_req_msg_v02));
  payload_len = sizeof(voice_orig_ussd_req_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_ORIG_USSD_REQ_V02;

  voice_orig_ussd_req_msg_v02 *ptr = (voice_orig_ussd_req_msg_v02 *) payload;
  return &(ptr->uss_info);
}



/*========================================================
  QMI_VOICE_ORIG_USSD_RESP_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_orig_ussd_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(voice_orig_ussd_resp_msg_v02));
  payload_len = sizeof(voice_orig_ussd_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_ORIG_USSD_RESP_V02;

  voice_orig_ussd_resp_msg_v02 *ptr = (voice_orig_ussd_resp_msg_v02 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

void ril_utf_qmiril_if::set_qmi_voice_orig_ussd_resp_alpha_id(
        alpha_dcs_enum_v02 alpha_dcs, uint32_t alpha_text_len, uint8_t *alpha_text)
{
  voice_orig_ussd_resp_msg_v02 *ptr = (voice_orig_ussd_resp_msg_v02 *) payload;
  ptr->alpha_id_valid = TRUE;
  ptr->alpha_id.alpha_dcs = alpha_dcs;
  ptr->alpha_id.alpha_text_len = alpha_text_len;
  memcpy((void *)ptr->alpha_id.alpha_text, (void *)alpha_text, alpha_text_len);
}
void ril_utf_qmiril_if::set_qmi_voice_orig_ussd_resp_cc_result_type(voice_cc_result_type_enum_v02  val)
{
  voice_orig_ussd_resp_msg_v02* ptr = (voice_orig_ussd_resp_msg_v02 *)payload;
  ptr->cc_result_type_valid = TRUE;
  ptr->cc_result_type = val;
}
void ril_utf_qmiril_if::set_qmi_voice_orig_ussd_resp_cc_sups_result(voice_cc_sups_result_service_type_enum_v02 service_type, voice_cc_sups_result_reason_enum_v02 reason)
{
  voice_orig_ussd_resp_msg_v02* ptr = (voice_orig_ussd_resp_msg_v02 *)payload;
  ptr->cc_sups_result_valid = TRUE;
  ptr->cc_sups_result.service_type = service_type;
  ptr->cc_sups_result.reason = reason;
}
void ril_utf_qmiril_if::set_qmi_voice_orig_ussd_resp_sip_error_code(uint16_t code)
{
  voice_orig_ussd_resp_msg_v02* ptr = (voice_orig_ussd_resp_msg_v02 *)payload;
  ptr->sip_error_code_valid = TRUE;
  ptr->sip_error_code = code;
}
void ril_utf_qmiril_if::set_qmi_voice_orig_ussd_resp_ussd_error_description(uint32_t len, uint16_t val[])
{
  voice_orig_ussd_resp_msg_v02* ptr = (voice_orig_ussd_resp_msg_v02 *)payload;
  ptr->ussd_error_description_valid = TRUE;
  memcpy(ptr->ussd_error_description, val, 2*MIN(len, QMI_VOICE_USS_DATA_MAX_V02 + 1));
}

/*========================================================
  QMI_VOICE_ANSWER_USSD_REQ_V02
========================================================*/
voice_uss_info_type_v02* ril_utf_qmiril_if::update_default_qmi_voice_answer_ussd_req_msg()
{
  payload = (void *) malloc(sizeof(voice_answer_ussd_req_msg_v02));
  payload_len = sizeof(voice_answer_ussd_req_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_ANSWER_USSD_REQ_V02;

  voice_answer_ussd_req_msg_v02 *ptr = (voice_answer_ussd_req_msg_v02 *) payload;
  return &(ptr->uss_info);
}



/*========================================================
  QMI_VOICE_ANSWER_USSD_RESP_V02
========================================================*/

void ril_utf_qmiril_if::update_default_qmi_voice_answer_ussd_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(voice_answer_ussd_resp_msg_v02));
  payload_len = sizeof(voice_answer_ussd_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_ANSWER_USSD_RESP_V02;

  voice_answer_ussd_resp_msg_v02 *ptr = (voice_answer_ussd_resp_msg_v02 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;


}


/*========================================================
  QMI_VOICE_CANCEL_USSD_REQ_V02
========================================================*/
voice_cancel_ussd_req_msg_v02* ril_utf_qmiril_if::update_default_qmi_voice_cancel_ussd_req_msg()
{
  payload = (void *) malloc(sizeof(voice_cancel_ussd_req_msg_v02));
  payload_len = sizeof(voice_cancel_ussd_req_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_CANCEL_USSD_REQ_V02;

  voice_cancel_ussd_req_msg_v02 *ptr = (voice_cancel_ussd_req_msg_v02 *) payload;
  return ptr;
}


/*========================================================
  QMI_VOICE_CANCEL_USSD_RESP_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_cancel_ussd_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(voice_cancel_ussd_resp_msg_v02));
  payload_len = sizeof(voice_cancel_ussd_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_CANCEL_USSD_RESP_V02;

  voice_cancel_ussd_resp_msg_v02 *ptr = (voice_cancel_ussd_resp_msg_v02 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}


/*========================================================
  QMI_VOICE_BIND_SUBSCRIPTION_REQ_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_bind_subscription_req_msg(voice_subs_type_enum_v02 subs_type)
{
  payload = (void *) malloc(sizeof(voice_bind_subscription_req_msg_v02));
  payload_len = sizeof(voice_bind_subscription_req_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_BIND_SUBSCRIPTION_REQ_V02;

  voice_bind_subscription_req_msg_v02 *ptr = (voice_bind_subscription_req_msg_v02 *) payload;
  ptr->subs_type = subs_type;

}
/*========================================================
  QMI_VOICE_BIND_SUBSCRIPTION_RESP_V02
========================================================*/

void ril_utf_qmiril_if::update_default_qmi_voice_bind_subscription_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(voice_cancel_ussd_resp_msg_v02));
  payload_len = sizeof(voice_cancel_ussd_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_BIND_SUBSCRIPTION_RESP_V02;

  voice_bind_subscription_resp_msg_v02 *ptr = (voice_bind_subscription_resp_msg_v02 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

/*========================================================
  QMI_VOICE_GET_ALL_CALL_INFO_REQ_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_get_all_call_info_req(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_GET_ALL_CALL_INFO_REQ_V02;
}

/*========================================================
  QMI_VOICE_GET_ALL_CALL_INFO_RESP_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_get_all_call_info_resp(qmi_result_type_v01 result,
                                                                        qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(voice_get_all_call_info_resp_msg_v02));
  payload_len = sizeof(voice_get_all_call_info_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_GET_ALL_CALL_INFO_RESP_V02;

  voice_get_all_call_info_resp_msg_v02 *ptr = (voice_get_all_call_info_resp_msg_v02 *)payload;
  ptr->resp.error = error;
  ptr->resp.result = result;
}
voice_call_info2_type_v02 *ril_utf_qmiril_if::set_qmi_voice_get_all_call_info_resp_call_info(
    uint32_t call_info_len)
{
  voice_get_all_call_info_resp_msg_v02 *ptr = (voice_get_all_call_info_resp_msg_v02 *)payload;
  ptr->call_info_valid = TRUE;
  ptr->call_info_len = call_info_len;
  return (ptr->call_info);
}

/*========================================================
  QMI_VOICE_ALL_CALL_STATUS_IND_V02
========================================================*/
voice_call_info2_type_v02* ril_utf_qmiril_if::update_default_qmi_voice_all_call_status_ind_msg(uint32_t call_info_len)
{
  payload = (void *) malloc(sizeof(voice_all_call_status_ind_msg_v02));
  payload_len = sizeof(voice_all_call_status_ind_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_ALL_CALL_STATUS_IND_V02;

  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->call_info_len = call_info_len;
  return (ptr->call_info);
}
voice_remote_party_number2_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_remote_party_number(uint32_t remote_party_number_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->remote_party_number_valid = TRUE;
  ptr->remote_party_number_len = remote_party_number_len;
  return (ptr->remote_party_number);

}

voice_ip_num_id_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_ip_num_info(uint32_t ip_num_info_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->ip_num_info_valid = TRUE;
  ptr->ip_num_info_len = ip_num_info_len;
  return (ptr->ip_num_info);
}

voice_remote_party_name2_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_remote_party_name( uint32_t remote_party_name_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->remote_party_name_valid = TRUE;
  ptr->remote_party_name_len = remote_party_name_len;
  return (ptr->remote_party_name);
}

voice_conn_num_with_id_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_conn_party_number(uint32_t conn_party_num_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->conn_party_num_valid = TRUE;
  ptr->conn_party_num_len = conn_party_num_len;
  return (ptr->conn_party_num);

}

voice_conn_ip_num_with_id_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_conn_party_extension(uint32_t conn_party_extension_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->conn_ip_num_info_valid = TRUE;
  ptr->conn_ip_num_info_len = conn_party_extension_len;
  return (ptr->conn_ip_num_info);
}

voice_alerting_type_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_alerting_type(uint32_t alerting_type_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->alerting_type_valid = TRUE;
  ptr->alerting_type_len = alerting_type_len;
  return (ptr->alerting_type);
}
voice_srv_opt_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_srv_opt(uint32_t srv_opt_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->srv_opt_valid = TRUE;
  ptr->srv_opt_len = srv_opt_len;
  return (ptr->srv_opt);
}
voice_call_end_reason_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_call_end_reason(uint32_t call_end_reason_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->call_end_reason_valid = TRUE;
  ptr->call_end_reason_len = call_end_reason_len;
  return (ptr->call_end_reason);
}
voice_ip_end_sip_code_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_sip_error_code(
   uint32_t sip_error_code_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->sip_error_code_valid = TRUE;
  ptr->sip_error_code_len = sip_error_code_len;
  return (ptr->sip_error_code);
}
voice_orig_fail_reason_with_id_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_orig_fail_reason(uint32_t orig_fail_reason_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->orig_fail_reason_valid = TRUE;
  ptr->orig_fail_reason_len = orig_fail_reason_len;
  return (ptr->orig_fail_reason);
}
voice_alpha_ident_with_id_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_alpha_id(uint32_t alpha_id_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->alpha_id_valid = TRUE;
  ptr->alpha_id_len = alpha_id_len;
  return (ptr->alpha_id);
}
voice_conn_num_with_id_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_conn_party_num(uint32_t conn_party_num_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->conn_party_num_valid = TRUE;
  ptr->conn_party_num_len = conn_party_num_len;
  return (ptr->conn_party_num);
}
voice_diagnostic_info_with_id_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_diagnostic_info(uint32_t diagnostic_info_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->diagnostic_info_valid = TRUE;
  ptr->diagnostic_info_len = diagnostic_info_len;
  return (ptr->diagnostic_info);
}
voice_num_with_id_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_called_party_num(uint32_t called_party_num_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->called_party_num_valid = TRUE;
  ptr->called_party_num_len = called_party_num_len;
  return (ptr->called_party_num);
}
voice_num_with_id_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_redirecting_party_num(uint32_t redirecting_party_num_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->redirecting_party_num_valid = TRUE;
  ptr->redirecting_party_num_len = redirecting_party_num_len;
  return (ptr->redirecting_party_num);
}
voice_call_attributes_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_audio_attrib(uint32_t audio_attrib_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->audio_attrib_valid = TRUE;
  ptr->audio_attrib_len = audio_attrib_len;
  return (ptr->audio_attrib);
}
voice_call_attributes_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_video_attrib(uint32_t video_attrib_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->video_attrib_valid = TRUE;
  ptr->video_attrib_len = video_attrib_len;
  return (ptr->video_attrib);
}
voice_call_attrib_status_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_call_attrib_status(uint32_t call_attrib_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->call_attrib_status_valid = TRUE;
  ptr->call_attrib_status_len = call_attrib_len;
  return (ptr->call_attrib_status);
}
#ifndef UTF_TARGET_BUILD
voice_is_add_info_present_with_id_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_is_add_info_present(uint32_t is_add_info_present_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->is_add_info_present_valid = TRUE;
  ptr->is_add_info_present_len = is_add_info_present_len;
  return (ptr->is_add_info_present);
}
voice_ip_call_capabilities_info_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_local_call_capabilities_info(
    uint32_t local_call_capabilities_info_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->local_call_capabilities_info_valid = TRUE;
  ptr->local_call_capabilities_info_len = local_call_capabilities_info_len;
  return (ptr->local_call_capabilities_info);
}
voice_ip_call_capabilities_info_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_peer_call_capabilities_info(
    uint32_t peer_call_capabilities_info_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->peer_call_capabilities_info_valid = TRUE;
  ptr->peer_call_capabilities_info_len = peer_call_capabilities_info_len;
  return (ptr->peer_call_capabilities_info);
}

voice_display_text_info_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_display_text(uint32_t display_text_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->display_text_valid = TRUE;
  ptr->display_text_len = display_text_len;
  return (ptr->display_text);
}

voice_ip_caller_name_info_type_v02 *ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_ip_caller_name(uint32_t ip_caller_name_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->ip_caller_name_valid = TRUE;
  ptr->ip_caller_name_len = ip_caller_name_len;
  return (ptr->ip_caller_name);
}

voice_ip_end_reason_text_type_v02 *ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_end_reason_text(uint32_t end_reason_text_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->end_reason_text_valid = TRUE;
  ptr->end_reason_text_len = end_reason_text_len;
  return (ptr->end_reason_text);
}
#endif

voice_media_id_with_id_type_v02 *ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_media_id(uint32_t media_id_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->media_id_valid = TRUE;
  ptr->media_id_len = media_id_len;
  return (ptr->media_id);
}

voice_volte_is_called_party_ringing_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_is_called_party_ringing(uint32_t is_called_party_ringing_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->is_called_party_ringing_valid = TRUE;
  ptr->is_called_party_ringing_len = is_called_party_ringing_len;
  return (ptr->is_called_party_ringing);
}

voice_volte_is_secure_call_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_is_secure_call(uint32_t is_secure_call_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->is_secure_call_valid = TRUE;
  ptr->is_secure_call_len = is_secure_call_len;
  return (ptr->is_secure_call);
}
voice_remote_party_conf_cap_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_remote_party_conf_capability(uint32_t remote_party_conf_capability_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->remote_party_conf_capability_valid = TRUE;
  ptr->remote_party_conf_capability_len = remote_party_conf_capability_len;
  return (ptr->remote_party_conf_capability);
}
voice_call_rtt_mode_status_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_rtt_mode(uint32_t rtt_mode_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->rtt_mode_valid = TRUE;
  ptr->rtt_mode_len = rtt_mode_len;
  return (ptr->rtt_mode);
}
voice_emerg_srv_categ_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_emerg_srv_categ(uint32_t emerg_srv_categ_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->emerg_srv_categ_valid = TRUE;
  ptr->emerg_srv_categ_len = emerg_srv_categ_len;
  return (ptr->emerg_srv_categ);
}
voice_MT_call_verstat_with_id_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_mt_call_verstat(uint32_t mt_call_verstat_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->mt_call_verstat_valid = TRUE;
  ptr->mt_call_verstat_len = mt_call_verstat_len;
  return (ptr->mt_call_verstat);
}
voice_ip_hist_info_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_ip_hist_info(uint32_t len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->ip_hist_info_valid = TRUE;
  ptr->ip_hist_info_len = len;
  return (ptr->ip_hist_info);
}
voice_sip_uri_with_id_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_alternate_sip_uris(uint32_t len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->alternate_sip_uris_valid = TRUE;
  ptr->alternate_sip_uris_len = len;
  return (ptr->alternate_sip_uris);
}
voice_num_with_id_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_terminating_num(uint32_t len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->terminating_num_valid = TRUE;
  ptr->terminating_num_len = len;
  return (ptr->terminating_num);
}
void ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_is_secondary(uint8_t val)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->is_secondary_valid = TRUE;
  ptr->is_secondary = val;
}
voice_tir_mode_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_tir_mode(uint32_t tir_mode_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->tir_mode_valid = TRUE;
  ptr->tir_mode_len = tir_mode_len;
  return (ptr->tir_mode);
}
voice_is_srvcc_call_with_id_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_srvcc_attrib(uint32_t is_srvcc_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->is_srvcc_valid = TRUE;
  ptr->is_srvcc_len = is_srvcc_len;
  return(ptr->is_srvcc);
}
voice_srvcc_parent_call_id_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_srvcc_parent_call_info(uint32_t srvcc_parent_call_info_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->srvcc_parent_call_info_valid = TRUE;
  ptr->srvcc_parent_call_info_len = srvcc_parent_call_info_len;
  return(ptr->srvcc_parent_call_info);
}
voice_child_number_info_type_v02* ril_utf_qmiril_if::set_qmi_voice_all_call_status_ind_child_number(uint32_t child_number_info_len)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *) payload;
  ptr->child_number_valid = TRUE;
  ptr->child_number_len = child_number_info_len;
  return(ptr->child_number);
}

/*========================================================
  QMI_VOICE_HANDOVER_IND_V02
========================================================*/
void ril_utf_qmiril_if::update_qmi_voice_handover_ind_msg(voice_handover_state_enum_v02 ho_state,
                                                          voice_handover_type_enum_v02 ho_type)
{
  payload = (void *) malloc(sizeof(voice_handover_ind_msg_v02));
  payload_len = sizeof(voice_handover_ind_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_HANDOVER_IND_V02;

  voice_handover_ind_msg_v02 *ptr = (voice_handover_ind_msg_v02 *) payload;
  ptr->ho_state = ho_state;
  ptr->ho_type_valid = TRUE;
  ptr->ho_type = ho_type;
}

/*========================================================
  QMI_VOICE_TTY_IND_V02
========================================================*/
void ril_utf_qmiril_if::update_qmi_voice_tty_ind_msg(tty_mode_enum_v02 tty_mode)
{
  payload = (void *) malloc(sizeof(voice_tty_ind_msg_v02));
  payload_len = sizeof(voice_tty_ind_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_TTY_IND_V02;

  voice_tty_ind_msg_v02 *ptr = (voice_tty_ind_msg_v02 *) payload;
  ptr->tty_mode = tty_mode;
}

/*========================================================
  QMI_VOICE_OTASP_STATUS_IND_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_otasp_status_ind_msg(uint8_t call_id, otasp_status_enum_v02 otasp_status)
{
  payload = (void *) malloc(sizeof(voice_otasp_status_ind_msg_v02));
  payload_len = sizeof(voice_otasp_status_ind_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_OTASP_STATUS_IND_V02;

  voice_otasp_status_ind_msg_v02 *ptr = (voice_otasp_status_ind_msg_v02 *) payload;
  (ptr->otasp_status_info).call_id = call_id;
  (ptr->otasp_status_info).otasp_status = otasp_status;
}


/*========================================================
  QMI_VOICE_PRIVACY_IND_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_privacy_ind_msg(uint8_t call_id, voice_privacy_enum_v02 voice_privacy)
{
  payload = (void *) malloc(sizeof(voice_privacy_ind_msg_v02));
  payload_len = sizeof(voice_privacy_ind_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_PRIVACY_IND_V02;

  voice_privacy_ind_msg_v02 *ptr = (voice_privacy_ind_msg_v02 *) payload;
  (ptr->voice_privacy_info).call_id = call_id;
  (ptr->voice_privacy_info).voice_privacy = voice_privacy;

}

/*========================================================
  QMI_VOICE_SUPS_NOTIFICATION_IND_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_sups_notification_ind_msg(uint8_t call_id, notification_type_enum_v02 notification_type)
{
  payload = (void *) malloc(sizeof(voice_sups_notification_ind_msg_v02));
  payload_len = sizeof(voice_sups_notification_ind_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_SUPS_NOTIFICATION_IND_V02;

  voice_sups_notification_ind_msg_v02 *ptr = (voice_sups_notification_ind_msg_v02 *) payload;
  (ptr->notification_info).call_id = call_id;
  (ptr->notification_info).notification_type = notification_type;
}

void ril_utf_qmiril_if::set_qmi_voice_sups_notification_ind_index(uint16_t val)
{
  voice_sups_notification_ind_msg_v02 *ptr = (voice_sups_notification_ind_msg_v02 *) payload;
  ptr->index_valid = TRUE;
  ptr->index = val;
}

voice_ect_number_type_v02* ril_utf_qmiril_if::set_qmi_voice_sups_notification_ind_ect_number()
{
  voice_sups_notification_ind_msg_v02 *ptr = (voice_sups_notification_ind_msg_v02 *) payload;
  ptr->ect_number_valid = TRUE;
  return &(ptr->ect_number) ;
}

#ifdef RIL_UTF_L_MASTER
uint16_t *ril_utf_qmiril_if::set_qmi_voice_sups_notification_ind_history_info(uint32_t hist_info_len)
{
  voice_sups_notification_ind_msg_v02 *ptr = (voice_sups_notification_ind_msg_v02 *) payload;
  ptr->ip_forward_hist_info_valid = TRUE;
  ptr->ip_forward_hist_info_len = hist_info_len;
  return (ptr->ip_forward_hist_info);
}
#endif

void ril_utf_qmiril_if::set_qmi_voice_sups_notification_ind_media_direction_hold(voice_call_attribute_type_mask_v02 val)
{
  voice_sups_notification_ind_msg_v02 *ptr = (voice_sups_notification_ind_msg_v02 *) payload;
  ptr->media_direction_hold_valid = TRUE;
  ptr->media_direction_hold = val;
}


/*========================================================
  QMI_VOICE_INFO_REC_IND_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_info_rec_ind_msg(uint8_t call_id)
{
  payload = (void *) malloc(sizeof(voice_info_rec_ind_msg_v02));
  payload_len = sizeof(voice_info_rec_ind_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_INFO_REC_IND_V02;

  voice_info_rec_ind_msg_v02 *ptr = (voice_info_rec_ind_msg_v02 *) payload;
  ptr->call_id = call_id;
}
void ril_utf_qmiril_if::set_qmi_voice_info_rec_ind_call_waiting(call_waiting_enum_v02 call_waiting)
{
  voice_info_rec_ind_msg_v02 *ptr = (voice_info_rec_ind_msg_v02 *) payload;
  ptr->call_waiting_valid = TRUE;
  ptr->call_waiting = call_waiting;
}
voice_caller_id_info_type_v02* ril_utf_qmiril_if::set_qmi_voice_info_rec_ind_caller_id_info()
{
  voice_info_rec_ind_msg_v02 *ptr = (voice_info_rec_ind_msg_v02 *) payload;
  ptr->caller_id_info_valid = TRUE;
  return &(ptr->caller_id_info);
}
voice_num_info_type_v02* ril_utf_qmiril_if::set_qmi_voice_info_rec_ind_calling_party_info()
{
  voice_info_rec_ind_msg_v02 *ptr = (voice_info_rec_ind_msg_v02 *) payload;
  ptr->calling_party_info_valid = TRUE;
  return &(ptr->calling_party_info);
}
char* ril_utf_qmiril_if::set_qmi_voice_info_rec_ind_display_buffer()
{
  voice_info_rec_ind_msg_v02 *ptr = (voice_info_rec_ind_msg_v02 *) payload;
  ptr->display_buffer_valid = TRUE;
  return (ptr->display_buffer);
}
voice_ext_display_info_type_v02* ril_utf_qmiril_if::set_qmi_voice_info_rec_ind_ext_display_record()
{
  voice_info_rec_ind_msg_v02 *ptr = (voice_info_rec_ind_msg_v02 *) payload;
  ptr->ext_display_record_valid = TRUE;
  return &(ptr->ext_display_record);
}
char* ril_utf_qmiril_if::set_qmi_voice_info_rec_ind_ext_display_buffer()
{
  voice_info_rec_ind_msg_v02 *ptr = (voice_info_rec_ind_msg_v02 *) payload;
  ptr->ext_display_buffer_valid = TRUE;
  return (ptr->ext_display_buffer);
}
char* ril_utf_qmiril_if::set_qmi_voice_info_rec_ind_caller_name()
{
  voice_info_rec_ind_msg_v02 *ptr = (voice_info_rec_ind_msg_v02 *) payload;
  ptr->caller_name_valid = TRUE;
  return (ptr->caller_name);
}
voice_nss_audio_control_info_type_v02* ril_utf_qmiril_if::set_qmi_voice_info_rec_ind_audio_control()
{
  voice_info_rec_ind_msg_v02 *ptr = (voice_info_rec_ind_msg_v02 *) payload;
  ptr->audio_control_valid = TRUE;
  return &(ptr->audio_control);
}
void ril_utf_qmiril_if::set_qmi_voice_info_rec_ind_clir_cause(voice_nss_clir_cause_enum_v02 clir_cause)
{
  voice_info_rec_ind_msg_v02 *ptr = (voice_info_rec_ind_msg_v02 *) payload;
  ptr->clir_cause_valid = TRUE;
  ptr->clir_cause = clir_cause;
}
void ril_utf_qmiril_if::set_qmi_voice_info_rec_ind_nss_release(voice_nss_release_enum_v02 nss_release)
{
  voice_info_rec_ind_msg_v02 *ptr = (voice_info_rec_ind_msg_v02 *) payload;
  ptr->nss_release_valid = TRUE;
  ptr->nss_release = nss_release;
}
voice_redirecting_num_info_type_v02* ril_utf_qmiril_if::set_qmi_voice_info_rec_ind_redirecting_num_info()
{
  voice_info_rec_ind_msg_v02 *ptr = (voice_info_rec_ind_msg_v02 *) payload;
  ptr->redirecting_num_info_valid = TRUE;
  return &(ptr->redirecting_num_info);
}
voice_line_control_info_type_v02* ril_utf_qmiril_if::set_qmi_voice_info_rec_ind_line_control()
{
  voice_info_rec_ind_msg_v02 *ptr = (voice_info_rec_ind_msg_v02 *) payload;
  ptr->line_control_valid = TRUE;
  return &(ptr->line_control);
}
voice_num_info_type_v02* ril_utf_qmiril_if::set_qmi_voice_info_rec_ind_conn_num_info()
{
  voice_info_rec_ind_msg_v02 *ptr = (voice_info_rec_ind_msg_v02 *) payload;
  ptr->conn_num_info_valid = TRUE;
  return &(ptr->conn_num_info);
}
voice_signal_info_type_v02* ril_utf_qmiril_if::set_qmi_voice_info_rec_ind_signal_info()
{
  voice_info_rec_ind_msg_v02 *ptr = (voice_info_rec_ind_msg_v02 *) payload;
  ptr->signal_info_valid = TRUE;
  return &(ptr->signal_info);
}

/*========================================================
  QMI_VOICE_USSD_IND_V02
========================================================*/

void ril_utf_qmiril_if::update_default_qmi_voice_ussd_ind_msg(further_user_action_enum_v02 notification_type)
{
  payload = (void *) malloc(sizeof(voice_ussd_ind_msg_v02));
  payload_len = sizeof(voice_ussd_ind_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_USSD_IND_V02;

  voice_ussd_ind_msg_v02 *ptr = (voice_ussd_ind_msg_v02 *) payload;
  ptr->notification_type = notification_type;


}
voice_uss_info_type_v02* ril_utf_qmiril_if::set_qmi_voice_voice_uss_info()
{
  voice_ussd_ind_msg_v02 *ptr = (voice_ussd_ind_msg_v02 *) payload;
  ptr->uss_info_valid = TRUE;
  return &(ptr->uss_info);
}
void ril_utf_qmiril_if::set_qmi_voice_orig_ussd_ind_sip_error_code(uint16_t code)
{
  voice_ussd_ind_msg_v02* ptr = (voice_ussd_ind_msg_v02 *)payload;
  ptr->sip_error_code_valid = TRUE;
  ptr->sip_error_code = code;
}
void ril_utf_qmiril_if::set_qmi_voice_orig_ussd_ind_ussd_error_description(uint32_t len, uint16_t val[])
{
  voice_ussd_ind_msg_v02* ptr = (voice_ussd_ind_msg_v02 *)payload;
  ptr->ussd_error_description_valid = TRUE;
  memcpy(ptr->ussd_error_description, val, 2*MIN(len, QMI_VOICE_USS_DATA_MAX_V02 + 1));
}
uint16_t* ril_utf_qmiril_if::set_qmi_voice_uss_info_utf16(uint32_t uss_info_utf16_len)
{
  voice_ussd_ind_msg_v02 *ptr = (voice_ussd_ind_msg_v02 *) payload;
  ptr->uss_info_utf16_valid = TRUE;
  ptr->uss_info_utf16_len = uss_info_utf16_len;
  return (ptr->uss_info_utf16);
}

/*========================================================
  QMI_VOICE_USSD_RELEASE_IND_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_ussd_release_ind_msg()
{
  payload = (void *) malloc(sizeof(voice_ussd_release_ind_msg_v02));
  payload_len = sizeof(voice_ussd_release_ind_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_USSD_RELEASE_IND_V02;
}


/*========================================================
  QMI_VOICE_SUPS_IND_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_sups_ind_msg(service_type_enum_v02 service_type, uint8_t is_modified_by_call_control)
{
  payload = (void *) malloc(sizeof(voice_sups_ind_msg_v02));
  payload_len = sizeof(voice_sups_ind_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_SUPS_IND_V02;

  voice_sups_ind_msg_v02 *ptr = (voice_sups_ind_msg_v02 *) payload;
  (ptr->supplementary_service_info).service_type = service_type;
  (ptr->supplementary_service_info).is_modified_by_call_control = is_modified_by_call_control;

}
void ril_utf_qmiril_if::set_qmi_voice_sups_ind_call_id(uint8_t val)
{
  voice_sups_ind_msg_v02 *ptr = (voice_sups_ind_msg_v02 *) payload;
  ptr->call_id_valid = TRUE;
  ptr->call_id = val;
}
voice_alpha_ident_type_v02* ril_utf_qmiril_if::set_qmi_voice_sups_ind_alpha_ident()
{
  voice_sups_ind_msg_v02 *ptr = (voice_sups_ind_msg_v02 *) payload;
  ptr->alpha_ident_valid = TRUE;
  return &(ptr->alpha_ident);
}

void ril_utf_qmiril_if::set_qmi_voice_sups_ind_data_source(voice_sups_data_source_enum_v02 data_source)
{
  voice_sups_ind_msg_v02 *ptr = (voice_sups_ind_msg_v02 *) payload;
  ptr->data_source_valid = TRUE;
  ptr->data_source = data_source;
}
voice_uss_info_type_v02* ril_utf_qmiril_if::set_qmi_voice_sups_ind_uss_info()
{
  voice_sups_ind_msg_v02 *ptr = (voice_sups_ind_msg_v02 *) payload;
  ptr->uss_info_valid = TRUE;
  return &(ptr->uss_info);
}
uint16_t* ril_utf_qmiril_if::set_qmi_voice_sups_ind_uss_info(uint32_t uss_info_utf16_len)
{
  voice_sups_ind_msg_v02 *ptr = (voice_sups_ind_msg_v02 *) payload;
  ptr->uss_info_utf16_valid = TRUE;
  ptr->uss_info_utf16_len = uss_info_utf16_len ;
  return (ptr->uss_info_utf16);
}
void ril_utf_qmiril_if::set_qmi_voice_sups_ind_failure_cause(qmi_sups_errors_enum_v02 failure_cause)
{
  voice_sups_ind_msg_v02 *ptr = (voice_sups_ind_msg_v02 *) payload;
  ptr->failure_cause_valid = TRUE;
  ptr->failure_cause = failure_cause ;

}
void ril_utf_qmiril_if::set_qmi_voice_sups_ind_reason(voice_sups_ind_reason_enum_v02 reason)
{
  voice_sups_ind_msg_v02 *ptr = (voice_sups_ind_msg_v02 *) payload;
  ptr->reason_valid = TRUE;
  ptr->reason = reason ;
}
void ril_utf_qmiril_if::set_qmi_voice_sups_ind_service_class(uint8_t val)
{
  voice_sups_ind_msg_v02 *ptr = (voice_sups_ind_msg_v02 *) payload;
  ptr->service_class_valid = TRUE;
  ptr->service_class = val ;
}
voice_clip_response_type_v02* ril_utf_qmiril_if::set_qmi_voice_sups_ind_clip_status()
{
  voice_sups_ind_msg_v02 *ptr = (voice_sups_ind_msg_v02 *) payload;
  ptr->clip_status_valid = TRUE;
  return &(ptr->clip_status);
}
voice_clir_response_type_v02* ril_utf_qmiril_if::set_qmi_voice_sups_ind_clir_status()
{
  voice_sups_ind_msg_v02 *ptr = (voice_sups_ind_msg_v02 *) payload;
  ptr->clir_status_valid = TRUE;
  return &(ptr->clir_status);
}

voice_get_call_forwarding_info_type_v02 *ril_utf_qmiril_if::set_qmi_voice_sups_ind_call_forwarding_info(uint32_t  len)
{
  voice_sups_ind_msg_v02 *ptr = (voice_sups_ind_msg_v02 *) payload;
  ptr->call_forwarding_info_valid = TRUE;
  ptr->call_forwarding_info_len = len;
  return ptr->call_forwarding_info;
}



/*========================================================
  QMI_VOICE_DTMF_IND_V02
========================================================*/

voice_dtmf_info_type_v02* ril_utf_qmiril_if::update_default_qmi_voice_dtmf_ind_msg()
{
  payload = (void *) malloc(sizeof(voice_dtmf_ind_msg_v02));
  payload_len = sizeof(voice_dtmf_ind_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_DTMF_IND_V02;

  voice_dtmf_ind_msg_v02 *ptr = (voice_dtmf_ind_msg_v02 *) payload;
  return &(ptr->dtmf_info);
}
void ril_utf_qmiril_if::set_qmi_voice_dtmf_ind_on_length(dtmf_onlength_enum_v02 val)
{
  voice_dtmf_ind_msg_v02 *ptr = (voice_dtmf_ind_msg_v02 *) payload;
  ptr->on_length_valid = TRUE;
  ptr->on_length = val;

}
void ril_utf_qmiril_if::set_qmi_voice_dtmf_ind_off_length(dtmf_offlength_enum_v02 val)
{
  voice_dtmf_ind_msg_v02 *ptr = (voice_dtmf_ind_msg_v02 *) payload;
  ptr->off_length_valid = TRUE;
  ptr->off_length = val;

}

/*========================================================
  QMI_VOICE_EXT_BRST_INTL_IND_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_ext_brst_intl_ind_msg(uint16_t mcc, uint8_t db_subtype, uint8_t chg_ind, uint8_t sub_unit, uint8_t unit)
{
  payload = (void *) malloc(sizeof(voice_ext_brst_intl_ind_msg_v02));
  payload_len = sizeof(voice_ext_brst_intl_ind_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_EXT_BRST_INTL_IND_V02;

  voice_ext_brst_intl_ind_msg_v02 *ptr = (voice_ext_brst_intl_ind_msg_v02 *) payload;
  (ptr->ext_burst_data).mcc = mcc;
  (ptr->ext_burst_data).db_subtype = db_subtype;
  (ptr->ext_burst_data).chg_ind = chg_ind;
  (ptr->ext_burst_data).sub_unit = sub_unit;
  (ptr->ext_burst_data).unit = unit;
}

/*========================================================
  QMI_VOICE_UUS_IND_V02
========================================================*/
voice_uus_info_type_v02* ril_utf_qmiril_if::update_default_qmi_voice_uus_ind_msg()
{
  payload = (void *) malloc(sizeof(voice_uus_ind_msg_v02));
  payload_len = sizeof(voice_uus_ind_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_UUS_IND_V02;

  voice_uus_ind_msg_v02 *ptr = (voice_uus_ind_msg_v02 *) payload;
  return &(ptr->uus_information);
}

/*========================================================
  QMI_VOICE_MODIFIED_IND_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_modified_ind_msg(uint8_t call_id)
{
  payload = (void *) malloc(sizeof(voice_modified_ind_msg_v02));
  payload_len = sizeof(voice_modified_ind_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_MODIFIED_IND_V02;

  voice_modified_ind_msg_v02 *ptr = (voice_modified_ind_msg_v02 *) payload;
  ptr->call_id = call_id;
}
void ril_utf_qmiril_if::set_qmi_voice_modified_ind_call_type(call_type_enum_v02 call_type)
{
  voice_modified_ind_msg_v02 *ptr = (voice_modified_ind_msg_v02 *) payload;
  ptr->call_type_valid = TRUE;
  ptr->call_type = call_type;

}
void ril_utf_qmiril_if::set_qmi_voice_modified_ind_audio_attrib(voice_call_attribute_type_mask_v02 audio_attrib)
{
  voice_modified_ind_msg_v02 *ptr = (voice_modified_ind_msg_v02 *) payload;
  ptr->audio_attrib_valid = TRUE;
  ptr->audio_attrib = audio_attrib;

}
void ril_utf_qmiril_if::set_qmi_voice_modified_ind_video_attrib(voice_call_attribute_type_mask_v02 video_attrib)
{
  voice_modified_ind_msg_v02 *ptr = (voice_modified_ind_msg_v02 *) payload;
  ptr->video_attrib_valid = TRUE;
  ptr->video_attrib = video_attrib;

}
void ril_utf_qmiril_if::set_qmi_voice_modified_ind_local_attrib(uint8 call_id, voice_call_attribute_type_mask_v02 audio_attrib,
                                                           voice_capability_restrict_cause_enum_v02 audio_cause,
                                                           voice_call_attribute_type_mask_v02 video_attrib,
                                                           voice_capability_restrict_cause_enum_v02 video_cause)
{
  voice_modified_ind_msg_v02 *ptr = (voice_modified_ind_msg_v02 *) payload;
  ptr->call_id = call_id;
  ptr->local_call_capabilities_valid = TRUE;
  ptr->local_call_capabilities.audio_attrib = audio_attrib;
  ptr->local_call_capabilities.audio_cause  = audio_cause;
  ptr->local_call_capabilities.video_attrib = video_attrib;
  ptr->local_call_capabilities.video_cause  = video_cause;
}
void ril_utf_qmiril_if::set_qmi_voice_modified_ind_peer_attrib(uint8 call_id, voice_call_attribute_type_mask_v02 audio_attrib,
                                                           voice_capability_restrict_cause_enum_v02 audio_cause,
                                                           voice_call_attribute_type_mask_v02 video_attrib,
                                                           voice_capability_restrict_cause_enum_v02 video_cause)
{
  voice_modified_ind_msg_v02 *ptr = (voice_modified_ind_msg_v02 *) payload;
  ptr->call_id = call_id;
  ptr->local_call_capabilities_valid = TRUE;
  ptr->peer_call_capabilities.audio_attrib = audio_attrib;
  ptr->peer_call_capabilities.audio_cause = audio_cause;
  ptr->peer_call_capabilities.video_attrib = video_attrib;
  ptr->peer_call_capabilities.video_cause = video_cause;
}
void ril_utf_qmiril_if::set_qmi_voice_modified_ind_call_attrib_status(voice_call_attrib_status_enum_v02 call_attrib_status)
{
  voice_modified_ind_msg_v02 *ptr = (voice_modified_ind_msg_v02 *) payload;
  ptr->call_attrib_status_valid = TRUE;
  ptr->call_attrib_status = call_attrib_status;
}
void ril_utf_qmiril_if::set_qmi_voice_modified_ind_failure_cause(qmi_sups_errors_enum_v02 failure_cause)
{
  voice_modified_ind_msg_v02 *ptr = (voice_modified_ind_msg_v02 *) payload;
  ptr->failure_cause_valid = TRUE;
  ptr->failure_cause = failure_cause;
}
void ril_utf_qmiril_if::set_qmi_voice_modified_ind_rtt_mode(rtt_mode_type_v02 rtt_mode)
{
  voice_modified_ind_msg_v02 *ptr = (voice_modified_ind_msg_v02 *) payload;
  ptr->rtt_mode_valid = TRUE;
  ptr->rtt_mode = rtt_mode;
}
void ril_utf_qmiril_if::set_qmi_voice_modified_ind_call_modified_cause(voice_call_modified_cause_enum_v02 val)
{
  voice_modified_ind_msg_v02 *ptr = (voice_modified_ind_msg_v02 *) payload;
  ptr->call_modified_cause_valid = TRUE;
  ptr->call_modified_cause = val;
}

/*========================================================
  QMI_VOICE_MODIFY_ACCEPT_IND_V02
========================================================*/
void ril_utf_qmiril_if::udpate_default_qmi_voice_modify_accept_ind_msg(uint8_t call_id)
{
  payload = (void *) malloc(sizeof(voice_modify_accept_ind_msg_v02));
  payload_len = sizeof(voice_modify_accept_ind_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_MODIFY_ACCEPT_IND_V02;

  voice_modify_accept_ind_msg_v02 *ptr = (voice_modify_accept_ind_msg_v02 *) payload;
  ptr->call_id = call_id;
}
void ril_utf_qmiril_if::set_qmi_voice_modified_accept_ind_call_type(call_type_enum_v02 call_type)
{
  voice_modify_accept_ind_msg_v02 *ptr = (voice_modify_accept_ind_msg_v02 *) payload;
  ptr->call_type_valid = TRUE;
  ptr->call_type = call_type;

}
void ril_utf_qmiril_if::set_qmi_voice_modified_accept_ind_audio_attrib(voice_call_attribute_type_mask_v02 audio_attrib)
{
  voice_modify_accept_ind_msg_v02 *ptr = (voice_modify_accept_ind_msg_v02 *) payload;
  ptr->audio_attrib_valid = TRUE;
  ptr->audio_attrib = audio_attrib;
}
void ril_utf_qmiril_if::set_qmi_voice_modified_accept_ind_video_attrib(voice_call_attribute_type_mask_v02 video_attrib)
{
  voice_modify_accept_ind_msg_v02 *ptr = (voice_modify_accept_ind_msg_v02 *) payload;
  ptr->video_attrib_valid = TRUE;
  ptr->video_attrib = video_attrib;

}

/*========================================================
  QMI_VOICE_SPEECH_CODEC_INFO_IND_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_speech_codec_info_ind_msg()
{
  payload = (void *) malloc(sizeof(voice_speech_codec_info_ind_msg_v02));
  payload_len = sizeof(voice_speech_codec_info_ind_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_SPEECH_CODEC_INFO_IND_V02;
}
void ril_utf_qmiril_if::set_qmi_voice_speech_codec_info_ind_network_mode(voice_network_mode_enum_v02 network_mode)
{
  voice_speech_codec_info_ind_msg_v02 *ptr = (voice_speech_codec_info_ind_msg_v02 *) payload;
  ptr->network_mode_valid = TRUE;
  ptr->network_mode = network_mode;
}
void ril_utf_qmiril_if::set_qmi_voice_speech_codec_info_ind_speech_codec(voice_speech_codec_enum_v02 speech_codec)
{
  voice_speech_codec_info_ind_msg_v02 *ptr = (voice_speech_codec_info_ind_msg_v02 *) payload;
  ptr->speech_codec_valid = TRUE;
  ptr->speech_codec = speech_codec;
}
void ril_utf_qmiril_if::set_qmi_voice_speech_codec_info_ind_call_id(uint8_t call_id)
{
  voice_speech_codec_info_ind_msg_v02 *ptr = (voice_speech_codec_info_ind_msg_v02 *) payload;
  ptr->call_id_valid = TRUE;
  ptr->call_id = call_id;
}

/*========================================================
  QMI_VOICE_HANDOVER_IND_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_handover_ind_msg(voice_handover_state_enum_v02 ho_state)
{
  payload = (void *) malloc(sizeof(voice_handover_ind_msg_v02));
  payload_len = sizeof(voice_handover_ind_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_HANDOVER_IND_V02;

  voice_handover_ind_msg_v02 *ptr = (voice_handover_ind_msg_v02 *) payload;
  ptr->ho_state = ho_state;

}

/*========================================================
  QMI_VOICE_CALL_CONTROL_RESULT_INFO_IND_V02
========================================================*/
voice_call_control_result_info_ind_msg_v02* ril_utf_qmiril_if::update_default_qmi_voice_call_control_result_ind_msg()
{
  payload = (void *) malloc(sizeof(voice_call_control_result_info_ind_msg_v02));
  payload_len = sizeof(voice_call_control_result_info_ind_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_CALL_CONTROL_RESULT_INFO_IND_V02;

  voice_call_control_result_info_ind_msg_v02 *ptr = (voice_call_control_result_info_ind_msg_v02 *) payload;
  return ptr;

}

/*========================================================
  QMI_VOICE_VICE_DIALOG_INFO_IND_V02
========================================================*/
uint8_t* ril_utf_qmiril_if::update_default_qmi_voice_vice_dialog_info_ind_msg(uint32_t len, uint32_t sequence)
{
  payload = (void *) malloc(sizeof(vice_dialog_info_ind_msg_v02));
  payload_len = sizeof(vice_dialog_info_ind_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_VICE_DIALOG_INFO_IND_V02;

  vice_dialog_info_ind_msg_v02 *ptr = (vice_dialog_info_ind_msg_v02 *) payload;
  ptr->sequence = sequence;

  ptr->vice_dialog_xml_len = len;
  return (ptr->vice_dialog_xml);
}
void ril_utf_qmiril_if::set_qmi_voice_vice_dialog_info_ind_total_size(uint32_t total_size)
{
  vice_dialog_info_ind_msg_v02 *ptr = (vice_dialog_info_ind_msg_v02 *) payload;
  ptr->total_size_valid = TRUE;
  ptr->total_size = total_size;
}

/*========================================================
  QMI_VOICE_CONFERENCE_INFO_IND_V02
========================================================*/
uint8_t* ril_utf_qmiril_if::update_default_qmi_voice_conference_info_ind_msg(uint32_t conference_xml_len, uint32_t sequence)
{
  payload = (void *) malloc(sizeof(voice_conference_info_ind_msg_v02));
  payload_len = sizeof(voice_conference_info_ind_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_CONFERENCE_INFO_IND_V02;

  voice_conference_info_ind_msg_v02 *ptr = (voice_conference_info_ind_msg_v02 *) payload;
  ptr->sequence = sequence;

  ptr->conference_xml_len = conference_xml_len;
  return (ptr->conference_xml);
}

void ril_utf_qmiril_if::set_qmi_voice_conference_info_ind_total_size(uint32_t total_size)
{
  voice_conference_info_ind_msg_v02 *ptr = (voice_conference_info_ind_msg_v02 *) payload;
  ptr->total_size_valid = TRUE;
  ptr->total_size = total_size;
}

void ril_utf_qmiril_if::set_qmi_voice_conference_info_ind_call_id(uint32_t call_id)
{
  voice_conference_info_ind_msg_v02 *ptr = (voice_conference_info_ind_msg_v02 *) payload;
  ptr->call_id_valid = TRUE;
  ptr->call_id = call_id;
}

/*========================================================
  QMI_AUTO_REJECTED_INCOMING_CALL_END_IND_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_auto_rejected_incoming_call_end_ind_msg(
    call_type_enum_v02 call_type, call_end_reason_enum_v02 call_end_reason, uint16_t sip_error_code)
{
  payload = (void *) malloc(sizeof(auto_rejected_incoming_call_end_ind_msg_v02));
  payload_len = sizeof(auto_rejected_incoming_call_end_ind_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_VOICE;
  msg_id = QMI_AUTO_REJECTED_INCOMING_CALL_END_IND_V02;

  auto_rejected_incoming_call_end_ind_msg_v02 *ptr = (auto_rejected_incoming_call_end_ind_msg_v02 *) payload;
  ptr->call_type = call_type;
  ptr->call_end_reason = call_end_reason;
  ptr->sip_error_code = sip_error_code;
}
void ril_utf_qmiril_if::set_qmi_voice_auto_rejected_incoming_call_end_ind_num(char *number)
{
  auto_rejected_incoming_call_end_ind_msg_v02 *ptr = (auto_rejected_incoming_call_end_ind_msg_v02 *) payload;
  if (number)
  {
    ptr->num_valid = TRUE;
    ptr->num_len = strlen(number);
    strlcpy(ptr->num, number, strlen(number)+1);
  }
}

/*========================================================
  QMI_VOICE_GET_COLR_REQ_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_get_colr_req_v02()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_GET_COLR_REQ_V02;
}

#ifndef UTF_TARGET_BUILD
/*========================================================
  QMI_VOICE_GET_COLR_RESP_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_get_colr_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(voice_get_colr_resp_msg_v02));
  payload_len = sizeof(voice_get_colr_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_GET_COLR_RESP_V02;

  voice_get_colr_resp_msg_v02 *ptr = (voice_get_colr_resp_msg_v02 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_voice_get_colr_resp_msg_colr_response
  (active_status_enum_v02 active_status, provision_status_enum_v02 provision_status)
{
  voice_get_colr_resp_msg_v02 *ptr = (voice_get_colr_resp_msg_v02 *) payload;
  ptr->colr_response_valid = TRUE;
  ptr->colr_response.active_status = active_status;
  ptr->colr_response.provision_status = provision_status;
}
void ril_utf_qmiril_if::set_qmi_voice_get_colr_resp_msg_colr_pi (pi_colr_enum_v02 colr_pi)
{
  voice_get_colr_resp_msg_v02 *ptr = (voice_get_colr_resp_msg_v02 *) payload;
  ptr->colr_pi_valid = TRUE;
  ptr->colr_pi = colr_pi;
}
#endif

/*========================================================
    QMI_WMS_SET_PRIMARY_CLIENT_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_set_primary_client_req_v01(void)
{
  payload =(void *) malloc(sizeof(wms_set_primary_client_req_msg_v01));
  payload_len = sizeof(wms_set_primary_client_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_SET_PRIMARY_CLIENT_REQ_V01;
  status = not_met;
}
void ril_utf_qmiril_if::set_qmi_wms_set_primary_client_req_v01_primary_client
  (uint8_t primary_client)
{
  wms_set_primary_client_req_msg_v01 *ptr =
    (wms_set_primary_client_req_msg_v01 *) payload;
  ptr->primary_client = primary_client;
}

/*========================================================
    QMI_WMS_SET_PRIMARY_CLIENT_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_set_primary_client_resp_v01(void)
{
  payload = (void *) malloc(sizeof(wms_set_primary_client_resp_msg_v01));
  payload_len = sizeof(wms_set_primary_client_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_SET_PRIMARY_CLIENT_RESP_V01;
}
void ril_utf_qmiril_if::set_qmi_wms_set_primary_client_resp_v01_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  wms_set_primary_client_resp_msg_v01 *ptr =
    (wms_set_primary_client_resp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
  QMI_WMS_SET_ROUTES_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_set_routes_req_v01(void)
{
  payload =(void *) malloc(sizeof(wms_set_routes_req_msg_v01));
  payload_len = sizeof(wms_set_routes_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  verify_payload = false;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_SET_ROUTES_REQ_V01;
  status = not_met;
}

/*========================================================
  QMI_WMS_SET_ROUTES_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_set_routes_resp_v01(void)
{
  payload = (void *) malloc(sizeof(wms_set_routes_resp_msg_v01));
  payload_len = sizeof(wms_set_routes_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_SET_ROUTES_RESP_V01;
}
void ril_utf_qmiril_if::set_qmi_wms_set_routes_resp_v01_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  wms_set_routes_resp_msg_v01 *ptr =
    (wms_set_routes_resp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
  QMI_WMS_SET_EVENT_REPORT_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_set_event_report_req_v01(void)
{
  payload =(void *) malloc(sizeof(wms_set_event_report_req_msg_v01));
  payload_len = sizeof(wms_set_event_report_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  verify_payload = false;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_SET_EVENT_REPORT_REQ_V01;
  status = not_met;
}

/*========================================================
  QMI_WMS_SET_EVENT_REPORT_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_set_event_report_resp_v01(void)
{
  payload = (void *) malloc(sizeof(wms_set_event_report_resp_msg_v01));
  payload_len = sizeof(wms_set_event_report_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_SET_EVENT_REPORT_RESP_V01;
}
void ril_utf_qmiril_if::set_qmi_wms_set_event_report_resp_v01_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  wms_set_event_report_resp_msg_v01 *ptr =
    (wms_set_event_report_resp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
  QMI_WMS_INDICATION_REGISTER_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_indication_register_req_v01(void)
{
  payload =(void *) malloc(sizeof(wms_indication_register_req_msg_v01));
  payload_len = sizeof(wms_indication_register_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_INDICATION_REGISTER_REQ_V01;
  status = not_met;

  wms_indication_register_req_msg_v01 *ptr =
    (wms_indication_register_req_msg_v01 *) payload;
  ptr->reg_service_ready_events_valid = TRUE;
  ptr->reg_service_ready_events = 1;
  ptr->reg_transport_nw_reg_info_events_valid = TRUE;
  ptr->reg_transport_nw_reg_info_events = 1;
}
void ril_utf_qmiril_if::set_qmi_wms_indication_register_req_v01_reg_transport_layer_info_events
  (uint8_t reg_transport_layer_info_events)
{
  wms_indication_register_req_msg_v01 *ptr =
    (wms_indication_register_req_msg_v01 *) payload;
  ptr->reg_transport_layer_info_events_valid = TRUE;
  ptr->reg_transport_layer_info_events = reg_transport_layer_info_events;
}
#ifndef UTF_TARGET_BUILD
void ril_utf_qmiril_if::set_qmi_wms_indication_register_req_v01_reg_transport_layer_mwi_info_events
  (uint8_t reg_transport_layer_mwi_info_events)
{
  wms_indication_register_req_msg_v01 *ptr =
    (wms_indication_register_req_msg_v01 *) payload;
  ptr->reg_transport_layer_mwi_info_events_valid = TRUE;
  ptr->reg_transport_layer_mwi_info_events = reg_transport_layer_mwi_info_events;
}
#endif

/*========================================================
  QMI_WMS_INDICATION_REGISTER_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_indication_register_resp_v01(void)
{
  payload = (void *) malloc(sizeof(wms_indication_register_resp_msg_v01));
  payload_len = sizeof(wms_indication_register_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_INDICATION_REGISTER_RESP_V01;
}
void ril_utf_qmiril_if::set_qmi_wms_indication_register_resp_v01_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  wms_indication_register_resp_msg_v01 *ptr =
    (wms_indication_register_resp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
  QMI_WMS_GET_INDICATION_REGISTER_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_get_indication_register_req_v01(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_GET_INDICATION_REGISTER_REQ_V01;
  status = not_met;
}

/*========================================================
  QMI_WMS_GET_INDICATION_REGISTER_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_get_indication_register_resp_v01(void)
{
  payload = (void *) malloc(sizeof(wms_get_indication_register_resp_msg_v01));
  payload_len = sizeof(wms_get_indication_register_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_GET_INDICATION_REGISTER_RESP_V01;
}
void ril_utf_qmiril_if::set_qmi_wms_get_indication_register_resp_v01_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  wms_get_indication_register_resp_msg_v01 *ptr =
    (wms_get_indication_register_resp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

//---------------------NAS Implmentation----------------------------------------------------------
/*========================================================
  QMI_NAS_PERFORM_NETWORK_SCAN_REQ_MSG_V01
========================================================*/

void ril_utf_qmiril_if::update_default_qmi_nas_perfrom_network_scan_req_msg()
{
  payload = (void *) malloc(sizeof(nas_perform_network_scan_req_msg_v01));
  payload_len = sizeof(nas_perform_network_scan_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_PERFORM_NETWORK_SCAN_REQ_MSG_V01;
}
void ril_utf_qmiril_if::set_qmi_nas_perform_network_scan_req_msg_network_type(nas_network_type_mask_type_v01 val)
{
  nas_perform_network_scan_req_msg_v01 *ptr = (nas_perform_network_scan_req_msg_v01 *) payload;
  ptr->network_type_valid = TRUE;
  ptr->network_type = val;

}

/*========================================================
  QMI_NAS_PERFORM_NETWORK_SCAN_RESP_MSG_V01
========================================================*/

void ril_utf_qmiril_if::update_default_qmi_nas_perform_network_scan_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_perform_network_scan_resp_msg_v01));
  payload_len = sizeof(nas_perform_network_scan_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_PERFORM_NETWORK_SCAN_RESP_MSG_V01;

  nas_perform_network_scan_resp_msg_v01 *ptr = (nas_perform_network_scan_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

void ril_utf_qmiril_if::set_qmi_nas_3gpp_network_info_tlv_network_scan_resp_msg(int nw_cnt, uint16_t mcc, uint16_t mnc, uint8_t network_status, char *network_description)
{
  nas_perform_network_scan_resp_msg_v01 *ptr = (nas_perform_network_scan_resp_msg_v01 *)payload;
  int i = ptr->nas_3gpp_network_info_len;

  if(!ptr->nas_3gpp_network_info_valid)
  {
    ptr->nas_3gpp_network_info_valid = TRUE;
  }

  if(nw_cnt>0)
  {
    ptr->nas_3gpp_network_info_len+=1;
    ptr->nas_3gpp_network_info[i].mobile_country_code = mcc;
    ptr->nas_3gpp_network_info[i].mobile_network_code = mnc;
    ptr->nas_3gpp_network_info[i].network_status = network_status;

    if(network_description!=NULL)
    {
       strlcpy(ptr->nas_3gpp_network_info[i].network_description,network_description,NAS_NETWORK_DESCRIPTION_MAX_V01+1);
    }
  }
}

void ril_utf_qmiril_if::set_qmi_nas_radio_access_technology_tlv_network_scan_resp_msg(uint16_t mcc, uint16_t mnc, uint8_t rat)
{
  nas_perform_network_scan_resp_msg_v01 *ptr = (nas_perform_network_scan_resp_msg_v01 *)payload;
  int i = ptr->nas_network_radio_access_technology_len;

  if(!ptr->nas_network_radio_access_technology_valid)
  {
    ptr->nas_network_radio_access_technology_valid = TRUE;
  }

  ptr->nas_network_radio_access_technology_len+=1;
  ptr->nas_network_radio_access_technology[i].mcc = mcc;
  ptr->nas_network_radio_access_technology[i].mnc = mnc;
  ptr->nas_network_radio_access_technology[i].rat = rat;
}

void ril_utf_qmiril_if::set_qmi_nas_mnc_pcs_digit_tlv_network_scan_resp_msg(uint16_t mcc, uint16_t mnc, uint8_t mnc_includes_pcs_digit)
{
  nas_perform_network_scan_resp_msg_v01 *ptr = (nas_perform_network_scan_resp_msg_v01 *)payload;
  int i = ptr->mnc_includes_pcs_digit_len;

  if(!ptr->mnc_includes_pcs_digit_valid)
  {
    ptr->mnc_includes_pcs_digit_valid = TRUE;
  }
  ptr->mnc_includes_pcs_digit_len+=1;
  ptr->mnc_includes_pcs_digit[i].mcc = mcc;
  ptr->mnc_includes_pcs_digit[i].mnc = mnc;
  ptr->mnc_includes_pcs_digit[i].mnc_includes_pcs_digit = mnc_includes_pcs_digit;
}

void ril_utf_qmiril_if::set_qmi_nas_scan_result_tlv_network_scan_resp_msg(nas_scan_result_enum_v01 scan_result)
{
  nas_perform_network_scan_resp_msg_v01 *ptr = (nas_perform_network_scan_resp_msg_v01 *)payload;
  ptr->scan_result_valid = TRUE;
  ptr->scan_result = scan_result;
}

void ril_utf_qmiril_if::set_qmi_nas_csg_nw_info_tlv_network_scan_resp_msg(uint16_t mcc, uint16_t mnc, nas_csg_list_cat_enum_v01 csg_list_cat, uint32_t id, uint32_t name_len, uint16_t *name)
{
  nas_perform_network_scan_resp_msg_v01 *ptr = (nas_perform_network_scan_resp_msg_v01 *)payload;
  int i = ptr->csg_info_len;

  if(!ptr->csg_info_valid)
  {
    ptr->csg_info_valid = TRUE;
  }

  ptr->csg_info_len+=1;
  ptr->csg_info[i].mcc = mcc;
  ptr->csg_info[i].mnc = mnc;
  ptr->csg_info[i].csg_list_cat = csg_list_cat;
  ptr->csg_info[i].csg_info.id = id;
  ptr->csg_info[i].csg_info.name_len = name_len;
  memcpy(ptr->csg_info[i].csg_info.name, name, name_len);
}

void ril_utf_qmiril_if::set_qmi_nas_csg_sig_info_tlv_network_scan_resp_msg(uint16_t mcc, uint16_t mnc, uint32_t csg_id, uint32_t signal_strength)
{
  nas_perform_network_scan_resp_msg_v01 *ptr = (nas_perform_network_scan_resp_msg_v01 *)payload;
  int i = ptr->csg_sig_info_len;

  if(!ptr->csg_sig_info_valid)
  {
    ptr->csg_sig_info_valid = TRUE;
  }
  ptr->csg_sig_info_len+=1;
  ptr->csg_sig_info[i].mcc = mcc;
  ptr->csg_sig_info[i].mnc = mnc;
  ptr->csg_sig_info[i].csg_id = csg_id;
  ptr->csg_sig_info[i].signal_strength = signal_strength;
}

/*========================================================
  QMI_NAS_GET_DUAL_STANDBY_PREF_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_dual_standby_pref_req_msg()
{
  payload = nullptr;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_DUAL_STANDBY_PREF_REQ_MSG_V01;
}

/*========================================================
  QMI_NAS_GET_DUAL_STANDBY_PREF_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_dual_standby_pref_resp_msg(
        qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_get_dual_standby_pref_resp_msg_v01));
  payload_len = sizeof(nas_get_dual_standby_pref_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_DUAL_STANDBY_PREF_RESP_MSG_V01;

  nas_get_dual_standby_pref_resp_msg_v01 *ptr = (nas_get_dual_standby_pref_resp_msg_v01*) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

void ril_utf_qmiril_if::set_qmi_nas_get_dual_standby_pref_resp_active_subs_mask(
        nas_active_subs_mask_type_v01 val)
{
  nas_get_dual_standby_pref_resp_msg_v01 *ptr = (nas_get_dual_standby_pref_resp_msg_v01*) payload;
  ptr->active_subs_mask_valid = TRUE;
  ptr->active_subs_mask = val;
}

/*========================================================
  QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01
========================================================*/

void ril_utf_qmiril_if::update_default_qmi_nas_set_system_selection_preference_req_msg()
{
  payload = (void *) malloc(sizeof(nas_set_system_selection_preference_req_msg_v01));
  payload_len = sizeof(nas_set_system_selection_preference_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01;
}
nas_band_pref_mask_type_v01* ril_utf_qmiril_if::set_qmi_nas_system_selection_preference_req_band_pref()
{
  nas_set_system_selection_preference_req_msg_v01 *ptr = (nas_set_system_selection_preference_req_msg_v01 *) payload;
  ptr->band_pref_valid = TRUE;
  return &(ptr->band_pref);
}
void ril_utf_qmiril_if::set_qmi_nas_system_selection_preference_req_mnc_includes_pcs_digit(uint8_t val)
{
  nas_set_system_selection_preference_req_msg_v01 *ptr = (nas_set_system_selection_preference_req_msg_v01 *) payload;
  ptr->mnc_includes_pcs_digit_valid = TRUE;
  ptr->mnc_includes_pcs_digit = val;
}
void ril_utf_qmiril_if::set_qmi_nas_system_selection_preference_req_roam_pref(nas_roam_pref_enum_v01 val)
{
  nas_set_system_selection_preference_req_msg_v01 *ptr = (nas_set_system_selection_preference_req_msg_v01 *) payload;
  ptr->roam_pref_valid = TRUE;
  ptr->roam_pref= val;
}
nas_net_sel_pref_type_v01* ril_utf_qmiril_if::set_qmi_nas_system_selection_preference_req_net_sel_pref()
{
  nas_set_system_selection_preference_req_msg_v01 *ptr = (nas_set_system_selection_preference_req_msg_v01 *) payload;
  ptr->net_sel_pref_valid = TRUE;
  return &(ptr->net_sel_pref);
}
void ril_utf_qmiril_if::set_qmi_nas_system_selection_preference_req_mode_pref(uint16_t mode_pref_val, uint8 gw_acq_order_pref_valid, uint16_t gw_acq_order_pref)
{
  nas_set_system_selection_preference_req_msg_v01 *ptr = (nas_set_system_selection_preference_req_msg_v01 *) payload;
  ptr->mode_pref_valid = TRUE;
  ptr->mode_pref = mode_pref_val;
  ptr->gw_acq_order_pref_valid = gw_acq_order_pref_valid;
  ptr->gw_acq_order_pref = (nas_gw_acq_order_pref_enum_type_v01)gw_acq_order_pref;
}
void ril_utf_qmiril_if::set_qmi_nas_system_selection_preference_req_emergency_mode(uint8_t val)
{
  nas_set_system_selection_preference_req_msg_v01 *ptr = (nas_set_system_selection_preference_req_msg_v01 *) payload;
  ptr->emergency_mode_valid = TRUE;
  ptr->emergency_mode = val;

}
void ril_utf_qmiril_if::set_qmi_nas_system_selection_preference_req_srv_reg_restriction(nas_srv_reg_restriction_enum_v01 val)
{
  nas_set_system_selection_preference_req_msg_v01 *ptr = (nas_set_system_selection_preference_req_msg_v01 *) payload;
  ptr->srv_reg_restriction_valid = TRUE;
  ptr->srv_reg_restriction = val;

}
void ril_utf_qmiril_if::set_qmi_nas_system_selection_preference_req_srv_domain_pref(nas_srv_domain_pref_enum_type_v01 val)
{
  nas_set_system_selection_preference_req_msg_v01 *ptr = (nas_set_system_selection_preference_req_msg_v01 *) payload;
  ptr->srv_domain_pref_valid = TRUE;
  ptr->srv_domain_pref = val;
  ptr->change_duration_valid = TRUE;
  ptr->change_duration = NAS_POWER_CYCLE_V01;

}
void ril_utf_qmiril_if::set_qmi_nas_system_selection_preference_req_voice_domain_pref(nas_voice_domain_pref_enum_type_v01 val)
{
  nas_set_system_selection_preference_req_msg_v01 *ptr = (nas_set_system_selection_preference_req_msg_v01 *) payload;
  ptr->voice_domain_pref_valid = TRUE;
  ptr->voice_domain_pref = val;
}

/*========================================================
  QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_set_system_selection_preference_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_set_system_selection_preference_resp_msg_v01));
  payload_len = sizeof(nas_set_system_selection_preference_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01;

  nas_set_system_selection_preference_resp_msg_v01 *ptr = (nas_set_system_selection_preference_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;

}

/*========================================================
  QMI_NAS_GET_CELL_LOCATION_INFO_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_cell_location_info_req_msg()
{
  //payload = (void *) malloc(sizeof(nas_get_cell_location_info_req_msg_v01));
  //payload_len = sizeof(nas_get_cell_location_info_req_msg_v01);
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_CELL_LOCATION_INFO_REQ_MSG_V01;
}

/*========================================================
  QMI_NAS_GET_CELL_LOCATION_INFO_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_cell_location_info_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_get_cell_location_info_resp_msg_v01));
  payload_len = sizeof(nas_get_cell_location_info_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_CELL_LOCATION_INFO_RESP_MSG_V01;

  nas_get_cell_location_info_resp_msg_v01 *ptr = (nas_get_cell_location_info_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
nas_geran_cell_info_type_v01 *ril_utf_qmiril_if::set_qmi_nas_get_cell_location_info_resp_geran_info()
{
  nas_get_cell_location_info_resp_msg_v01 *ptr = (nas_get_cell_location_info_resp_msg_v01 *) payload;
  ptr->geran_info_valid = TRUE;
  return &(ptr->geran_info);
}
nas_umts_cell_info_type_v01 *ril_utf_qmiril_if::set_qmi_nas_get_cell_location_info_resp_umts_info()
{
  nas_get_cell_location_info_resp_msg_v01 *ptr = (nas_get_cell_location_info_resp_msg_v01 *) payload;
  ptr->umts_info_valid = TRUE;
  return &(ptr->umts_info);
}
/*========================================================
  QMI_NAS_DUAL_STANDBY_PREF_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_standby_pref_req_msg()
{
  payload = (void *) malloc(sizeof(nas_set_dual_standby_pref_req_msg_v01));
  payload_len = sizeof(nas_set_dual_standby_pref_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_DUAL_STANDBY_PREF_REQ_MSG_V01;
}
void ril_utf_qmiril_if::set_qmi_nas_dual_standby_pref_req_standby_pref(nas_standby_pref_enum_v01 val)
{
  nas_set_dual_standby_pref_req_msg_v01 *ptr = (nas_set_dual_standby_pref_req_msg_v01 *) payload;
  ptr->standby_pref_valid = TRUE;
  ptr->standby_pref= val;

}
void ril_utf_qmiril_if::set_qmi_nas_dual_standby_pref_req_default_data_subs(nas_subs_type_enum_v01 val)
{
  nas_set_dual_standby_pref_req_msg_v01 *ptr = (nas_set_dual_standby_pref_req_msg_v01 *) payload;
  ptr->default_data_subs_valid = TRUE;
  ptr->default_data_subs= val;

}
void ril_utf_qmiril_if::set_qmi_nas_dual_standby_pref_req_priority_subs(nas_subs_type_enum_v01 val)
{
  nas_set_dual_standby_pref_req_msg_v01 *ptr = (nas_set_dual_standby_pref_req_msg_v01 *) payload;
  ptr->priority_subs_valid = TRUE;
  ptr->priority_subs= val;
}


/*========================================================
  QMI_NAS_DUAL_STANDBY_PREF_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_standby_pref_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_set_dual_standby_pref_resp_msg_v01));
  payload_len = sizeof(nas_set_dual_standby_pref_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_DUAL_STANDBY_PREF_RESP_MSG_V01;

  nas_set_dual_standby_pref_resp_msg_v01 *ptr = (nas_set_dual_standby_pref_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

/*========================================================
  QMI_NAS_CONFIG_EMBMS_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_config_embms_req_msg(uint8_t enable)
{
  payload = (void *) malloc(sizeof(nas_config_embms_req_msg_v01));
  payload_len = sizeof(nas_config_embms_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_CONFIG_EMBMS_REQ_MSG_V01;

  nas_config_embms_req_msg_v01 *ptr = (nas_config_embms_req_msg_v01 *) payload;
  ptr->enable = enable;
  ptr->trace_id_valid = TRUE;

}

void ril_utf_qmiril_if::set_qmi_nas_config_ebmbms_req_trace_id(int16_t val)
{
  nas_config_embms_req_msg_v01 *ptr = (nas_config_embms_req_msg_v01 *) payload;
  ptr->trace_id_valid = TRUE;
  ptr->trace_id = val;
}

/*========================================================
  QMI_NAS_CONFIG_EMBMS_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_config_embms_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_config_embms_resp_msg_v01));
  payload_len = sizeof(nas_config_embms_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_CONFIG_EMBMS_RESP_MSG_V01;

  nas_config_embms_resp_msg_v01 *ptr = (nas_config_embms_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
  ptr->trace_id_valid = TRUE;
}
/*========================================================
  QMI_NAS_GET_EMBMS_SIG_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_embms_sig_req_msg()
{
  payload = (void *) malloc(sizeof(nas_get_embms_sig_req_msg_v01));
  payload_len = sizeof(nas_get_embms_sig_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_EMBMS_SIG_REQ_MSG_V01;

}

void ril_utf_qmiril_if::set_qmi_nas_get_embms_sig_req_trace_id(int16_t val)
{
  nas_get_embms_sig_req_msg_v01 *ptr = (nas_get_embms_sig_req_msg_v01 *) payload;
  ptr->trace_id_valid = TRUE;
  ptr->trace_id= val;

}

/*========================================================
  QMI_NAS_GET_EMBMS_SIG_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_embms_sig_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_get_embms_sig_resp_msg_v01));
  payload_len = sizeof(nas_get_embms_sig_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_EMBMS_SIG_RESP_MSG_V01;

  nas_get_embms_sig_resp_msg_v01 *ptr = (nas_get_embms_sig_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;

}
void ril_utf_qmiril_if::set_qmi_nas_get_embms_sig_resp_trace_id(int16_t val)
{
  nas_get_embms_sig_resp_msg_v01 *ptr = (nas_get_embms_sig_resp_msg_v01 *) payload;
  ptr->trace_id_valid = TRUE;
  ptr->trace_id= val;
}
nas_lte_cphy_mbsfn_area_signal_strength_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_embms_sig_resp_sig_list(uint32_t sig_list_len)
{
  nas_get_embms_sig_resp_msg_v01 *ptr = (nas_get_embms_sig_resp_msg_v01 *) payload;
  ptr->sig_list_valid = TRUE;
  ptr->sig_list_len = sig_list_len;
  return (ptr->sig_list);
}
/*========================================================
  QMI_NAS_GET_CENTRALIZED_EONS_SUPPORT_STATUS_REQ_MSG_V01
========================================================*/


/*========================================================
  QMI_NAS_GET_CENTRALIZED_EONS_SUPPORT_STATUS_RESP_MSG_V01
========================================================*/

void ril_utf_qmiril_if::update_default_qmi_nas_get_centralized_eons_support_status_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_get_centralized_eons_support_status_resp_msg_v01));
  payload_len = sizeof(nas_get_centralized_eons_support_status_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_CENTRALIZED_EONS_SUPPORT_STATUS_RESP_MSG_V01;

  nas_get_centralized_eons_support_status_resp_msg_v01 *ptr = (nas_get_centralized_eons_support_status_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;

}

void ril_utf_qmiril_if::set_qmi_nas_get_centralized_eons_support_status_resp_centralized_eons_supported(uint8_t val)
{
  nas_get_centralized_eons_support_status_resp_msg_v01 *ptr = (nas_get_centralized_eons_support_status_resp_msg_v01 *) payload;
  ptr->centralized_eons_supported_valid = TRUE;
  ptr->centralized_eons_supported= val;

}

/*========================================================
  QMI_NAS_GET_RF_BAND_INFO_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_rf_band_info_req_msg() {
  payload = (void *) malloc(sizeof(nas_get_rf_band_info_req_msg_v01));
  payload_len = sizeof(nas_get_rf_band_info_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_RF_BAND_INFO_REQ_MSG_V01;
}

/*========================================================
  QMI_NAS_GET_RF_BAND_INFO_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_rf_band_info_resp_msg() {
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_RF_BAND_INFO_RESP_MSG_V01;

  payload = (void *) malloc(sizeof(nas_get_rf_band_info_resp_msg_v01));
  if (payload != nullptr) {
    payload_len = sizeof(nas_get_rf_band_info_resp_msg_v01);
    memset(payload, 0x00, payload_len);
  } else {
    payload_len = 0;
  }
}

nas_rf_band_info_type_v01* ril_utf_qmiril_if::update_default_qmi_nas_get_rf_band_info_resp_msg(uint32_t rf_band_info_list_len,nas_radio_if_enum_v01 radio_if,nas_active_band_enum_v01 active_band,uint16_t active_channel,qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  update_default_qmi_nas_get_rf_band_info_resp_msg();

  if (payload) {
    nas_get_rf_band_info_resp_msg_v01 *ptr = (nas_get_rf_band_info_resp_msg_v01 *) payload;
    ptr->rf_band_info_list_len = rf_band_info_list_len;
    return ptr->rf_band_info_list;
  }

  return nullptr;
}

/*========================================================
  QMI_NAS_LTE_CPHY_CA_IND_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_lte_cphy_ca_ind(nas_lte_cphy_ca_indicator_type_v01 cphy_ca)
{
  payload = (void *) malloc(sizeof(nas_lte_cphy_ca_ind_msg_v01));
  payload_len = sizeof(nas_lte_cphy_ca_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_LTE_CPHY_CA_IND_V01;

  nas_lte_cphy_ca_ind_msg_v01* ptr = (nas_lte_cphy_ca_ind_msg_v01*) payload;
  ptr->cphy_ca = cphy_ca;
}

/*========================================================
  QMI_NAS_GET_LTE_CPHY_CA_INFO_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_lte_cphy_ca_info_req_msg() {
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_LTE_CPHY_CA_INFO_REQ_MSG_V01;

  payload = (void *) malloc(sizeof(nas_get_lte_cphy_ca_info_req_msg_v01));
  if (payload != nullptr) {
    payload_len = sizeof(nas_get_lte_cphy_ca_info_req_msg_v01);
    memset(payload, 0x00, payload_len);
  } else {
    payload_len = 0;
  }
}

/*========================================================
  QMI_NAS_GET_LTE_CPHY_CA_INFO_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_nas_get_lte_cphy_ca_info_resp_msg() {
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_LTE_CPHY_CA_INFO_RESP_MSG_V01;

  payload = (void *) malloc(sizeof(nas_get_lte_cphy_ca_info_resp_msg_v01));
  if (payload != nullptr) {
    payload_len = sizeof(nas_get_lte_cphy_ca_info_resp_msg_v01);
    memset(payload, 0x00, payload_len);
  } else {
    payload_len = 0;
  }
}

/*========================================================
  QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01
========================================================*/

/*========================================================
  QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_system_selection_preference_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_get_system_selection_preference_resp_msg_v01));
  payload_len = sizeof(nas_get_system_selection_preference_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01;

  nas_get_system_selection_preference_resp_msg_v01 *ptr = (nas_get_system_selection_preference_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;

}

void ril_utf_qmiril_if::set_qmi_nas_get_system_selection_preference_resp_emergency_mode(uint8_t val)
{
  nas_get_system_selection_preference_resp_msg_v01 *ptr = (nas_get_system_selection_preference_resp_msg_v01 *) payload;
  ptr->emergency_mode_valid = TRUE;
  ptr->emergency_mode= val;
}
void ril_utf_qmiril_if::set_qmi_nas_get_system_selection_preference_resp_mode_pref(uint16_t val)
{
  nas_get_system_selection_preference_resp_msg_v01 *ptr = (nas_get_system_selection_preference_resp_msg_v01 *) payload;
  ptr->mode_pref_valid = TRUE;
  ptr->mode_pref= val;
}
void ril_utf_qmiril_if::set_qmi_nas_get_system_selection_preference_resp_band_pref(uint64_t val)
{
  nas_get_system_selection_preference_resp_msg_v01 *ptr = (nas_get_system_selection_preference_resp_msg_v01 *) payload;
  ptr->band_pref_valid = TRUE;
  ptr->band_pref= val;
}
void ril_utf_qmiril_if::set_qmi_nas_get_system_selection_preference_resp_prl_pref(nas_prl_pref_enum_v01 val)
{
  nas_get_system_selection_preference_resp_msg_v01 *ptr = (nas_get_system_selection_preference_resp_msg_v01 *) payload;
  ptr->mode_pref_valid = TRUE;
  ptr->mode_pref= val;
}
void ril_utf_qmiril_if::set_qmi_nas_get_system_selection_preference_resp_roam_pref(nas_roam_pref_enum_v01 val)
{
  nas_get_system_selection_preference_resp_msg_v01 *ptr = (nas_get_system_selection_preference_resp_msg_v01 *) payload;
  ptr->roam_pref_valid = TRUE;
  ptr->roam_pref= val;
}
void ril_utf_qmiril_if::set_qmi_nas_get_system_selection_preference_resp_net_sel_pref(nas_net_sel_pref_enum_v01 val)
{
  nas_get_system_selection_preference_resp_msg_v01 *ptr = (nas_get_system_selection_preference_resp_msg_v01 *) payload;
  ptr->net_sel_pref_valid = TRUE;
  ptr->net_sel_pref= val;
}
void ril_utf_qmiril_if::set_qmi_nas_get_system_selection_preference_resp_srv_reg_restriction(nas_srv_reg_restriction_enum_v01 val)
{
  nas_get_system_selection_preference_resp_msg_v01 *ptr = (nas_get_system_selection_preference_resp_msg_v01 *) payload;
  ptr->srv_reg_restriction_valid = TRUE;
  ptr->srv_reg_restriction= val;
}

/*========================================================
  QMI_NAS_GET_SIGNAL_STRENGTH_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_signal_strength_req_msg()
{
  payload = (void *) malloc(sizeof(nas_get_signal_strength_req_msg_v01));
  payload_len = sizeof(nas_get_signal_strength_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_SIGNAL_STRENGTH_REQ_MSG_V01;
}

void ril_utf_qmiril_if::set_qmi_nas_get_signal_strength_req_request_mask(uint16_t val)
{
  nas_get_signal_strength_req_msg_v01 *ptr = (nas_get_signal_strength_req_msg_v01 *) payload;
  ptr->request_mask_valid = TRUE;
  ptr->request_mask= val;
}

/*========================================================
  QMI_NAS_GET_SIGNAL_STRENGTH_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_signal_strength_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error,int8_t sig_strength, nas_radio_if_enum_v01 radio_if)
{
  payload = (void *) malloc(sizeof(nas_get_signal_strength_resp_msg_v01));
  payload_len = sizeof(nas_get_signal_strength_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_SIGNAL_STRENGTH_RESP_MSG_V01;

  nas_get_signal_strength_resp_msg_v01 *ptr = (nas_get_signal_strength_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
  (ptr->signal_strength).sig_strength = sig_strength;
  (ptr->signal_strength).radio_if = radio_if;
}

nas_rssi_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_signal_strength_resp_rssi(uint32_t rssi_len)
{
  nas_get_signal_strength_resp_msg_v01 *ptr = (nas_get_signal_strength_resp_msg_v01 *) payload;
  ptr->rssi_valid = TRUE;
  ptr->rssi_len = rssi_len;
  return (ptr->rssi);
}

nas_signal_strength_list_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_signal_strength_resp_signal_strength_list(uint32_t signal_strength_list_len)
{
  nas_get_signal_strength_resp_msg_v01 *ptr = (nas_get_signal_strength_resp_msg_v01 *) payload;
  ptr->signal_strength_list_valid = TRUE;
  ptr->signal_strength_list_len = signal_strength_list_len;
  return (ptr->signal_strength_list);
}
void ril_utf_qmiril_if::set_qmi_nas_get_signal_strength_resp_io(uint32_t val)
{
  nas_get_signal_strength_resp_msg_v01 *ptr = (nas_get_signal_strength_resp_msg_v01 *) payload;
  ptr->io_valid = TRUE;
  ptr->io = val;
}
void ril_utf_qmiril_if::set_qmi_nas_get_signal_strength_resp_snr(int16_t val)
{
  nas_get_signal_strength_resp_msg_v01 *ptr = (nas_get_signal_strength_resp_msg_v01 *) payload;
  ptr->snr_valid = TRUE;
  ptr->snr = val;
}
nas_error_rate_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_signal_strength_resp_error_rate(uint32_t error_rate_len)
{
  nas_get_signal_strength_resp_msg_v01 *ptr = (nas_get_signal_strength_resp_msg_v01 *) payload;
  ptr->error_rate_valid = TRUE;
  ptr->error_rate_len = error_rate_len;
  return (ptr->error_rate);
}
nas_ecio_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_signal_strength_resp_ecio(uint32_t ecio_len)
{
  nas_get_signal_strength_resp_msg_v01 *ptr = (nas_get_signal_strength_resp_msg_v01 *) payload;
  ptr->ecio_valid = TRUE;
  ptr->ecio_len = ecio_len;
  return (ptr->ecio);
}
void ril_utf_qmiril_if::set_qmi_nas_get_signal_strength_resp_rsrq(int8_t rsrq,uint8_t radio_if)
{
  nas_get_signal_strength_resp_msg_v01 *ptr = (nas_get_signal_strength_resp_msg_v01 *) payload;
  ptr->rsrq_valid = TRUE;
  (ptr->rsrq).rsrq = rsrq;
  (ptr->rsrq).radio_if = radio_if;
}


/*========================================================
  QMI_NAS_GET_SIG_INFO_REQ_MSG_V01
========================================================*/

/*========================================================
  QMI_NAS_GET_SIG_INFO_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_sig_info_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_get_sig_info_resp_msg_v01));
  payload_len = sizeof(nas_get_sig_info_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_SIG_INFO_RESP_MSG_V01;

  nas_get_sig_info_resp_msg_v01 *ptr = (nas_get_sig_info_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;

}

nas_common_sig_info_param_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_sig_info_resp_cdma_sig_info()
{
  nas_get_sig_info_resp_msg_v01 *ptr = (nas_get_sig_info_resp_msg_v01 *) payload;
  ptr->cdma_sig_info_valid = TRUE;
  return &(ptr->cdma_sig_info);
}
nas_hdr_sig_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_sig_info_resp_hdr_sig_info()
{
  nas_get_sig_info_resp_msg_v01 *ptr = (nas_get_sig_info_resp_msg_v01 *) payload;
  ptr->hdr_sig_info_valid = TRUE;
  return &(ptr->hdr_sig_info);
}
void ril_utf_qmiril_if::set_qmi_nas_get_sig_info_resp_gsm_sig_info(int8_t val)
{
  nas_get_sig_info_resp_msg_v01 *ptr = (nas_get_sig_info_resp_msg_v01 *) payload;
  ptr->gsm_sig_info_valid = TRUE;
  ptr->gsm_sig_info = val;
}
nas_common_sig_info_param_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_sig_info_resp_wcdma_sig_info()
{
  nas_get_sig_info_resp_msg_v01 *ptr = (nas_get_sig_info_resp_msg_v01 *) payload;
  ptr->wcdma_sig_info_valid = TRUE;
  return &(ptr->wcdma_sig_info);
}
void ril_utf_qmiril_if::set_qmi_nas_get_sig_info_resp_rscp(int8_t val)
{
  nas_get_sig_info_resp_msg_v01 *ptr = (nas_get_sig_info_resp_msg_v01 *) payload;
  ptr->rscp_valid = TRUE;
  ptr->rscp = val;
}
nas_lte_sig_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_sig_info_resp_lte_sig_info()
{
  nas_get_sig_info_resp_msg_v01 *ptr = (nas_get_sig_info_resp_msg_v01 *) payload;
  ptr->lte_sig_info_valid = TRUE;
  return &(ptr->lte_sig_info);
}

/*========================================================
  QMI_NAS_GET_ERR_RATE_REQ_MSG_V01
========================================================*/

/*========================================================
  QMI_NAS_GET_ERR_RATE_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_qmi_nas_get_err_rate_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{

  payload = (void *) malloc(sizeof(nas_get_err_rate_resp_msg_v01));
  payload_len = sizeof(nas_get_err_rate_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_ERR_RATE_RESP_MSG_V01;

  nas_get_err_rate_resp_msg_v01 *ptr = (nas_get_err_rate_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;

}
void ril_utf_qmiril_if::set_qmi_nas_get_err_rate_resp_gsm_bit_err_rate(uint8_t val)
{
  nas_get_err_rate_resp_msg_v01 *ptr = (nas_get_err_rate_resp_msg_v01 *) payload;
  ptr->gsm_bit_err_rate_valid = TRUE;
  ptr->gsm_bit_err_rate = val;
}

void ril_utf_qmiril_if::set_qmi_nas_get_err_rate_resp_wcdma_block_err_rate(uint8_t val)
{
  nas_get_err_rate_resp_msg_v01 *ptr = (nas_get_err_rate_resp_msg_v01 *) payload;
  ptr->wcdma_block_err_rate_valid = TRUE;
  ptr->wcdma_block_err_rate = val;
}
void ril_utf_qmiril_if::set_qmi_nas_get_err_rate_resp_tdscdma_block_err_rate(uint8_t val)
{
  nas_get_err_rate_resp_msg_v01 *ptr = (nas_get_err_rate_resp_msg_v01 *) payload;
  ptr->tdscdma_block_err_rate_valid = TRUE;
  ptr->tdscdma_block_err_rate = val;
}

/*========================================================
  QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
========================================================*/

/*========================================================
  QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_qmi_nas_get_sys_info_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_get_sys_info_resp_msg_v01));
  payload_len = sizeof(nas_get_sys_info_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_SYS_INFO_RESP_MSG_V01;

  nas_get_sys_info_resp_msg_v01 *ptr = (nas_get_sys_info_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
nas_3gpp2_srv_status_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_sys_info_resp_cdma_srv_status_info()
{
  nas_get_sys_info_resp_msg_v01 *ptr = (nas_get_sys_info_resp_msg_v01 *) payload;
  ptr->cdma_srv_status_info_valid = TRUE;
  return &(ptr->cdma_srv_status_info);
}
nas_3gpp2_srv_status_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_sys_info_resp_hdr_srv_status_info()
{
  nas_get_sys_info_resp_msg_v01 *ptr = (nas_get_sys_info_resp_msg_v01 *) payload;
  ptr->hdr_srv_status_info_valid = TRUE;
  return &(ptr->hdr_srv_status_info);
}
nas_3gpp_srv_status_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_sys_info_resp_gsm_srv_status_info()
{
  nas_get_sys_info_resp_msg_v01 *ptr = (nas_get_sys_info_resp_msg_v01 *) payload;
  ptr->gsm_srv_status_info_valid = TRUE;
  return &(ptr->gsm_srv_status_info);
}
nas_3gpp_srv_status_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_sys_info_resp_wcdma_srv_status_info()
{
  nas_get_sys_info_resp_msg_v01 *ptr = (nas_get_sys_info_resp_msg_v01 *) payload;
  ptr->wcdma_srv_status_info_valid = TRUE;
  return &(ptr->wcdma_srv_status_info);
}
nas_3gpp_srv_status_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_sys_info_resp_tdscdma_srv_status_info()
{
  nas_get_sys_info_resp_msg_v01 *ptr = (nas_get_sys_info_resp_msg_v01 *) payload;
  ptr->tdscdma_srv_status_info_valid = TRUE;
  return &(ptr->tdscdma_srv_status_info);
}
nas_3gpp_srv_status_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_sys_info_resp_lte_srv_status_info()
{
  nas_get_sys_info_resp_msg_v01 *ptr = (nas_get_sys_info_resp_msg_v01 *) payload;
  ptr->lte_srv_status_info_valid = TRUE;
  return &(ptr->lte_srv_status_info);
}
nas_cdma_sys_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_sys_info_resp_cdma_sys_info()
{
  nas_get_sys_info_resp_msg_v01 *ptr = (nas_get_sys_info_resp_msg_v01 *) payload;
  ptr->cdma_sys_info_valid = TRUE;
  return &(ptr->cdma_sys_info);
}
nas_hdr_sys_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_sys_info_resp_hdr_sys_info()
{
  nas_get_sys_info_resp_msg_v01 *ptr = (nas_get_sys_info_resp_msg_v01 *) payload;
  ptr->hdr_sys_info_valid = TRUE;
  return &(ptr->hdr_sys_info);
}
nas_gsm_sys_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_sys_info_resp_gsm_sys_info()
{
  nas_get_sys_info_resp_msg_v01 *ptr = (nas_get_sys_info_resp_msg_v01 *) payload;
  ptr->gsm_sys_info_valid = TRUE;
  return &(ptr->gsm_sys_info);
}
nas_wcdma_sys_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_sys_info_resp_wcdma_sys_info()
{
  nas_get_sys_info_resp_msg_v01 *ptr = (nas_get_sys_info_resp_msg_v01 *) payload;
  ptr->wcdma_sys_info_valid = TRUE;
  return &(ptr->wcdma_sys_info);
}
nas_tdscdma_sys_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_sys_info_resp_tdscdma_sys_info()
{
  nas_get_sys_info_resp_msg_v01 *ptr = (nas_get_sys_info_resp_msg_v01 *) payload;
  ptr->tdscdma_sys_info_valid = TRUE;
  return &(ptr->tdscdma_sys_info);
}
nas_lte_sys_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_sys_info_resp_lte_sys_info()
{
  nas_get_sys_info_resp_msg_v01 *ptr = (nas_get_sys_info_resp_msg_v01 *) payload;
  ptr->lte_sys_info_valid = TRUE;
  return &(ptr->lte_sys_info);
}
nas_cdma_sys_info2_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_sys_info_resp_cdma_sys_info2()
{
  nas_get_sys_info_resp_msg_v01 *ptr = (nas_get_sys_info_resp_msg_v01 *) payload;
  ptr->cdma_sys_info2_valid = TRUE;
  return &(ptr->cdma_sys_info2);
}
nas_hdr_sys_info2_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_sys_info_resp_hdr_sys_info2()
{
  nas_get_sys_info_resp_msg_v01 *ptr = (nas_get_sys_info_resp_msg_v01 *) payload;
  ptr->hdr_sys_info2_valid = TRUE;
  return &(ptr->hdr_sys_info2);
}
nas_gsm_sys_info2_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_sys_info_resp_gsm_sys_info2()
{
  nas_get_sys_info_resp_msg_v01 *ptr = (nas_get_sys_info_resp_msg_v01 *) payload;
  ptr->gsm_sys_info2_valid = TRUE;
  return &(ptr->gsm_sys_info2);
}
nas_wcdma_sys_info2_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_sys_info_resp_wcdma_sys_info2()
{
  nas_get_sys_info_resp_msg_v01 *ptr = (nas_get_sys_info_resp_msg_v01 *) payload;
  ptr->wcdma_sys_info2_valid = TRUE;
  return &(ptr->wcdma_sys_info2);
}
nas_lte_sys_info2_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_sys_info_resp_lte_sys_info2()
{
  nas_get_sys_info_resp_msg_v01 *ptr = (nas_get_sys_info_resp_msg_v01 *) payload;
  ptr->lte_sys_info2_valid = TRUE;
  return &(ptr->lte_sys_info2);
}
nas_gw_sys_info3_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_sys_info_resp_gsm_sys_info3()
{
  nas_get_sys_info_resp_msg_v01 *ptr = (nas_get_sys_info_resp_msg_v01 *) payload;
  ptr->gsm_sys_info3_valid = TRUE;
  return &(ptr->gsm_sys_info3);
}
nas_gw_sys_info3_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_sys_info_resp_wcdma_sys_info3()
{
  nas_get_sys_info_resp_msg_v01 *ptr = (nas_get_sys_info_resp_msg_v01 *) payload;
  ptr->wcdma_sys_info3_valid = TRUE;
  return &(ptr->wcdma_sys_info3);
}
void ril_utf_qmiril_if::set_qmi_nas_get_sys_info_resp_voice_support_on_lte(uint8_t val)
{
  nas_get_sys_info_resp_msg_v01 *ptr = (nas_get_sys_info_resp_msg_v01 *) payload;
  ptr->voice_support_on_lte_valid = TRUE;
  ptr->voice_support_on_lte = val;
}
void ril_utf_qmiril_if::set_qmi_nas_get_sys_info_resp_lte_embms_coverage(uint8_t val)
{
  nas_get_sys_info_resp_msg_v01 *ptr = (nas_get_sys_info_resp_msg_v01 *) payload;
  ptr->lte_embms_coverage_valid = TRUE;
  ptr->lte_embms_coverage = val;
}
void ril_utf_qmiril_if::set_qmi_nas_get_sys_info_resp_sim_rej_info(nas_sim_rej_info_enum_type_v01 val)
{
  nas_get_sys_info_resp_msg_v01 *ptr = (nas_get_sys_info_resp_msg_v01 *) payload;
  ptr->sim_rej_info_valid = TRUE;
  ptr->sim_rej_info = val;
}
void ril_utf_qmiril_if::set_qmi_nas_get_sys_info_resp_lte_voice_status(nas_lte_voice_status_enum_type_v01 lte_voice_status)
{
  nas_get_sys_info_resp_msg_v01 *ptr = (nas_get_sys_info_resp_msg_v01 *) payload;
  ptr->lte_voice_status_valid = TRUE;
  ptr->lte_voice_status = lte_voice_status;
}
void ril_utf_qmiril_if::set_qmi_nas_get_sys_info_resp_lte_sms_status(nas_sms_status_enum_type_v01 lte_sms_status)
{
  nas_get_sys_info_resp_msg_v01 *ptr = (nas_get_sys_info_resp_msg_v01 *) payload;
  ptr->lte_sms_status_valid = TRUE;
  ptr->lte_sms_status = lte_sms_status;
}
void ril_utf_qmiril_if::set_qmi_nas_get_sys_info_resp_wcdma_csg_info(uint8_t csg_info_valid, uint32_t id, uint32_t name_len, uint16_t *name)
{
  nas_get_sys_info_resp_msg_v01 *ptr = (nas_get_sys_info_resp_msg_v01 *) payload;
  ptr->wcdma_csg_info_valid = csg_info_valid;
  ptr->wcdma_csg_info.id = id;
  ptr->wcdma_csg_info.name_len = name_len;
  if(NULL != name)
  {
    memcpy(ptr->wcdma_csg_info.name, name, name_len);
  }
}
void ril_utf_qmiril_if::set_qmi_nas_get_sys_info_resp_lte_csg_info(uint8_t csg_info_valid, uint32_t id, uint32_t name_len, uint16_t *name)
{
  nas_get_sys_info_resp_msg_v01 *ptr = (nas_get_sys_info_resp_msg_v01 *) payload;
  ptr->lte_csg_info_valid = csg_info_valid;
  ptr->lte_csg_info.id = id;
  ptr->lte_csg_info.name_len = name_len;
  if(NULL != name)
  {
    memcpy(ptr->lte_csg_info.name, name, name_len);
  }
}
/*========================================================
  QMI_NAS_GET_SERVING_SYSTEM_REQ_MSG_V01
========================================================*/
/*========================================================
  QMI_NAS_GET_SERVING_SYSTEM_RESP_MSG_V01
========================================================*/
nas_serving_system_type_v01* ril_utf_qmiril_if::update_qmi_nas_get_serving_system_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_get_serving_system_resp_msg_v01));
  payload_len = sizeof(nas_get_serving_system_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_SERVING_SYSTEM_RESP_MSG_V01;

  nas_get_serving_system_resp_msg_v01 *ptr = (nas_get_serving_system_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
  return &(ptr->serving_system);
}
nas_cdma_base_station_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_serving_system_resp_cdma_base_station_info()
{
  nas_get_serving_system_resp_msg_v01 *ptr = (nas_get_serving_system_resp_msg_v01 *) payload;
  ptr->cdma_base_station_info_valid = TRUE;
  return &(ptr->cdma_base_station_info);
}
nas_cdma_system_id_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_serving_system_resp_cdma_system_id()
{
  nas_get_serving_system_resp_msg_v01 *ptr = (nas_get_serving_system_resp_msg_v01 *) payload;
  ptr->cdma_system_id_valid = TRUE;
  return &(ptr->cdma_system_id);
}
void ril_utf_qmiril_if::set_qmi_nas_get_serving_system_resp_roaming_indicator(nas_roaming_indicator_enum_v01 val)
{
  nas_get_serving_system_resp_msg_v01 *ptr = (nas_get_serving_system_resp_msg_v01 *) payload;
  ptr->roaming_indicator_valid = TRUE;
  ptr->roaming_indicator = val;
}
void ril_utf_qmiril_if::set_qmi_nas_get_serving_system_resp_lac(uint16_t val)
{
  nas_get_serving_system_resp_msg_v01 *ptr = (nas_get_serving_system_resp_msg_v01 *) payload;
  ptr->lac_valid = TRUE;
  ptr->lac = val;
}
void ril_utf_qmiril_if::set_qmi_nas_get_serving_system_resp_cell_id(uint32_t val)
{
  nas_get_serving_system_resp_msg_v01 *ptr = (nas_get_serving_system_resp_msg_v01 *) payload;
  ptr->cell_id_valid = TRUE;
  ptr->cell_id = val;
}
void ril_utf_qmiril_if::set_qmi_nas_get_serving_system_resp_ccs(uint8_t val)
{
  nas_get_serving_system_resp_msg_v01 *ptr = (nas_get_serving_system_resp_msg_v01 *) payload;
  ptr->ccs_valid = TRUE;
  ptr->ccs = val;
}
void ril_utf_qmiril_if::set_qmi_nas_get_serving_system_resp_prl_ind(uint8_t val)
{
  nas_get_serving_system_resp_msg_v01 *ptr = (nas_get_serving_system_resp_msg_v01 *) payload;
  ptr->prl_ind_valid = TRUE;
  ptr->prl_ind = val;
}
nas_ss_detailed_service_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_serving_system_resp_detailed_service_info()
{
  nas_get_serving_system_resp_msg_v01 *ptr = (nas_get_serving_system_resp_msg_v01 *) payload;
  ptr->detailed_service_info_valid = TRUE;
  return &(ptr->detailed_service_info);
}
nas_cdma_system_id_ext_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_serving_system_resp_cdma_system_id_ext()
{
  nas_get_serving_system_resp_msg_v01 *ptr = (nas_get_serving_system_resp_msg_v01 *) payload;
  ptr->cdma_system_id_ext_valid = TRUE;
  return &(ptr->cdma_system_id_ext);
}
void ril_utf_qmiril_if::set_qmi_nas_get_serving_system_resp_hdr_personality(nas_hdr_personality_enum_v01 val)
{
  nas_get_serving_system_resp_msg_v01 *ptr = (nas_get_serving_system_resp_msg_v01 *) payload;
  ptr->hdr_personality_valid = TRUE;
  ptr->hdr_personality = val;
}
nas_data_capabilites_enum_v01* ril_utf_qmiril_if::set_qmi_nas_get_serving_system_resp_data_capabilities(uint32_t data_capabilities_len)
{
  nas_get_serving_system_resp_msg_v01 *ptr = (nas_get_serving_system_resp_msg_v01 *) payload;
  ptr->data_capabilities_valid = TRUE;
  ptr->data_capabilities_len = data_capabilities_len;
  return (ptr->data_capabilities);
}
nas_plmn_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_serving_system_resp_current_plmn()
{
  nas_get_serving_system_resp_msg_v01 *ptr = (nas_get_serving_system_resp_msg_v01 *) payload;
  ptr->current_plmn_valid = TRUE;
  return &(ptr->current_plmn);
}
void ril_utf_qmiril_if::set_qmi_nas_get_serving_system_resp_p_rev_in_use(uint8_t val)
{
  nas_get_serving_system_resp_msg_v01 *ptr = (nas_get_serving_system_resp_msg_v01 *) payload;
  ptr->p_rev_in_use_valid = TRUE;
  ptr->p_rev_in_use = val;
}
void ril_utf_qmiril_if::set_qmi_nas_get_serving_system_resp_tac(uint16_t val)
{
  nas_get_serving_system_resp_msg_v01 *ptr = (nas_get_serving_system_resp_msg_v01 *) payload;
  ptr->tac_valid = TRUE;
  ptr->tac = val;
}
nas_gw_sys_info3_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_serving_system_resp_call_barring_status()
{
  nas_get_serving_system_resp_msg_v01 *ptr = (nas_get_serving_system_resp_msg_v01 *) payload;
  ptr->call_barring_status_valid = TRUE;
  return &(ptr->call_barring_status);
}
void ril_utf_qmiril_if::set_qmi_nas_get_serving_system_resp_umts_psc(uint16_t val)
{
  nas_get_serving_system_resp_msg_v01 *ptr = (nas_get_serving_system_resp_msg_v01 *) payload;
  ptr->umts_psc_valid = TRUE;
  ptr->umts_psc = val;
}
nas_mnc_pcs_digit_include_status_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_serving_system_resp_mnc_includes_pcs_digit()
{
  nas_get_serving_system_resp_msg_v01 *ptr = (nas_get_serving_system_resp_msg_v01 *) payload;
  ptr->mnc_includes_pcs_digit_valid = TRUE;
  return &(ptr->mnc_includes_pcs_digit);
}
void ril_utf_qmiril_if::set_qmi_nas_get_serving_system_resp_hs_call_status(nas_hs_support_enum_type_v01 val)
{
  nas_get_serving_system_resp_msg_v01 *ptr = (nas_get_serving_system_resp_msg_v01 *) payload;
  ptr->hs_call_status_valid = TRUE;
  ptr->hs_call_status = val;
}
void ril_utf_qmiril_if::set_qmi_nas_get_serving_system_resp_def_roam_ind(nas_roaming_indicator_enum_v01 val)
{
  nas_get_serving_system_resp_msg_v01 *ptr = (nas_get_serving_system_resp_msg_v01 *) payload;
  ptr->def_roam_ind_valid = TRUE;
  ptr->def_roam_ind = val;
}

/*========================================================
  QMI_NAS_GET_OPERATOR_NAME_DATA_REQ_MSG_V01
========================================================*/
/*========================================================
  QMI_NAS_GET_OPERATOR_NAME_DATA_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_operator_name_data_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_get_operator_name_data_resp_msg_v01));
  payload_len = sizeof(nas_get_operator_name_data_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_OPERATOR_NAME_DATA_RESP_MSG_V01;

  nas_get_operator_name_data_resp_msg_v01 *ptr = (nas_get_operator_name_data_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

/*========================================================
  QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_3gpp2_subscription_info_req_msg(uint8_t nam_id)
{
  payload = (void *) malloc(sizeof(nas_get_3gpp2_subscription_info_req_msg_v01));
  payload_len = sizeof(nas_get_3gpp2_subscription_info_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO_REQ_MSG_V01;

  nas_get_3gpp2_subscription_info_req_msg_v01 *ptr = (nas_get_3gpp2_subscription_info_req_msg_v01 *) payload;
  ptr->nam_id = nam_id;
}
void ril_utf_qmiril_if::set_qmi_nas_get_3gpp2_subscription_info_req_get_3gpp2_info_mask(get_3gpp2_info_mask_enum_type_v01 val)
{
  nas_get_3gpp2_subscription_info_req_msg_v01 *ptr = (nas_get_3gpp2_subscription_info_req_msg_v01 *) payload;
  ptr->get_3gpp2_info_mask_valid = TRUE;
  ptr->get_3gpp2_info_mask = val;
}
/*========================================================
  QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::upate_default_qmi_nas_get_3gpp2_subscription_info_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_get_3gpp2_subscription_info_resp_msg_v01));
  payload_len = sizeof(nas_get_3gpp2_subscription_info_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO_RESP_MSG_V01;

  nas_get_3gpp2_subscription_info_resp_msg_v01 *ptr = (nas_get_3gpp2_subscription_info_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
char* ril_utf_qmiril_if::set_qmi_nas_get_3gpp2_subscription_info_resp_nam_name(uint32_t nam_name_len)
{
  nas_get_3gpp2_subscription_info_resp_msg_v01 *ptr = (nas_get_3gpp2_subscription_info_resp_msg_v01 *) payload;
  ptr->nam_name_valid = TRUE;
  ptr->nam_name_len = nam_name_len;
  return (ptr->nam_name) ;
}
char* ril_utf_qmiril_if::set_qmi_nas_get_3gpp2_subscription_info_resp_mdn(uint32_t mdn_len)
{
  nas_get_3gpp2_subscription_info_resp_msg_v01 *ptr = (nas_get_3gpp2_subscription_info_resp_msg_v01 *) payload;
  ptr->mdn_valid = TRUE;
  ptr->mdn_len = mdn_len;
  return (ptr->mdn) ;
}
nas_3gpp2_home_sid_nid_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_3gpp2_subscription_info_resp_cdma_sys_id(uint32_t cdma_sys_id_len)
{
  nas_get_3gpp2_subscription_info_resp_msg_v01 *ptr = (nas_get_3gpp2_subscription_info_resp_msg_v01 *) payload;
  ptr->cdma_sys_id_valid = TRUE;
  ptr->cdma_sys_id_len = cdma_sys_id_len;
  return (ptr->cdma_sys_id) ;
}
nas_3gpp2_min_based_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_3gpp2_subscription_info_resp_min_based_info()
{
  nas_get_3gpp2_subscription_info_resp_msg_v01 *ptr = (nas_get_3gpp2_subscription_info_resp_msg_v01 *) payload;
  ptr->min_based_info_valid = TRUE;
  return &(ptr->min_based_info) ;
}

/*========================================================
  QMI_NAS_GET_PLMN_NAME_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_plmn_name_req_msg(uint16_t mcc, uint16_t mnc)
{
  payload = (void *) malloc(sizeof(nas_get_plmn_name_req_msg_v01));
  payload_len = sizeof(nas_get_plmn_name_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_PLMN_NAME_REQ_MSG_V01;

  nas_get_plmn_name_req_msg_v01 *ptr = (nas_get_plmn_name_req_msg_v01 *) payload;
  (ptr->plmn).mcc = mcc;
  (ptr->plmn).mnc = mnc;
}
void ril_utf_qmiril_if::set_qmi_nas_get_plmn_name_req_mnc_includes_pcs_digit(uint8_t val)
{
  nas_get_plmn_name_req_msg_v01 *ptr = (nas_get_plmn_name_req_msg_v01 *) payload;
  ptr->mnc_includes_pcs_digit_valid = TRUE;
  ptr->mnc_includes_pcs_digit = val;
}
void ril_utf_qmiril_if::set_qmi_nas_get_plmn_name_req_suppress_sim_error(uint8_t val)
{
  nas_get_plmn_name_req_msg_v01 *ptr = (nas_get_plmn_name_req_msg_v01 *) payload;
  ptr->suppress_sim_error_valid = TRUE;
  ptr->suppress_sim_error = val;
}
void ril_utf_qmiril_if::set_qmi_nas_get_plmn_name_req_use_static_table_only(uint8_t val)
{
  nas_get_plmn_name_req_msg_v01 *ptr = (nas_get_plmn_name_req_msg_v01 *) payload;
  ptr->use_static_table_only_valid = TRUE;
  ptr->use_static_table_only = val;
}
void ril_utf_qmiril_if::set_qmi_nas_get_plmn_name_req_rat(nas_radio_if_enum_v01 val)
{
  nas_get_plmn_name_req_msg_v01 *ptr = (nas_get_plmn_name_req_msg_v01 *) payload;
  ptr->rat_valid = TRUE;
  ptr->rat = val;
}
void ril_utf_qmiril_if::set_qmi_nas_get_plmn_name_req_always_send_plmn_name(uint8_t val)
{
  nas_get_plmn_name_req_msg_v01 *ptr = (nas_get_plmn_name_req_msg_v01 *) payload;
  ptr->always_send_plmn_name_valid = TRUE;
  ptr->always_send_plmn_name = val;
}
void ril_utf_qmiril_if::set_qmi_nas_get_plmn_name_req_csg_id(uint32_t csg_id)
{
  nas_get_plmn_name_req_msg_v01 *ptr = (nas_get_plmn_name_req_msg_v01 *) payload;
  ptr->csg_id_valid = TRUE;
  ptr->csg_id = csg_id;
}

/*========================================================
  QMI_NAS_GET_PLMN_NAME_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_plmn_name_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_get_plmn_name_resp_msg_v01));
  payload_len = sizeof(nas_get_plmn_name_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_PLMN_NAME_RESP_MSG_V01;

  nas_get_plmn_name_resp_msg_v01 *ptr = (nas_get_plmn_name_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

nas_3gpp_eons_plmn_name_type_v01* ril_utf_qmiril_if::set_qmi_nas_get_plmn_name_resp_eons_plmn_name_3gpp()
{
  nas_get_plmn_name_resp_msg_v01 *ptr = (nas_get_plmn_name_resp_msg_v01 *) payload;
  ptr->eons_plmn_name_3gpp_valid = TRUE;
  return &(ptr->eons_plmn_name_3gpp);
}
/*========================================================
  QMI_NAS_BIND_SUBSCRIPTION_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_bind_subscription_req_msg(nas_subs_type_enum_v01 subs_type)
{
  payload = (void *) malloc(sizeof(nas_bind_subscription_req_msg_v01));
  payload_len = sizeof(nas_bind_subscription_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_BIND_SUBSCRIPTION_REQ_MSG_V01;

  nas_bind_subscription_req_msg_v01 *ptr = (nas_bind_subscription_req_msg_v01 *) payload;
  ptr->subs_type = subs_type;
}

/*========================================================
  QMI_NAS_BIND_SUBSCRIPTION_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_bind_subscription_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_bind_subscription_resp_msg_v01));
  payload_len = sizeof(nas_bind_subscription_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_BIND_SUBSCRIPTION_RESP_MSG_V01;

  nas_bind_subscription_resp_msg_v01 *ptr = (nas_bind_subscription_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
/*========================================================
  QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01
========================================================*/

void ril_utf_qmiril_if::update_default_qmi_nas_indication_register_req_msg()
{
  payload = (void *) malloc(sizeof(nas_indication_register_req_msg_v01));
  payload_len = sizeof(nas_indication_register_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01;
}
void ril_utf_qmiril_if::set_qmi_nas_indication_register_req_reg_sys_sel_pref(uint8_t val)
{
  nas_indication_register_req_msg_v01 *ptr = (nas_indication_register_req_msg_v01 *) payload;
  ptr->reg_sys_sel_pref_valid = TRUE;
  ptr->reg_sys_sel_pref = val;
}
void ril_utf_qmiril_if::set_qmi_nas_indication_register_req_reg_ddtm_events(uint8_t val)
{
  nas_indication_register_req_msg_v01 *ptr = (nas_indication_register_req_msg_v01 *) payload;
  ptr->reg_ddtm_events_valid = TRUE;
  ptr->reg_ddtm_events = val;
}
void ril_utf_qmiril_if::set_qmi_nas_indication_register_req_dual_standby_pref(uint8_t val)
{
  nas_indication_register_req_msg_v01 *ptr = (nas_indication_register_req_msg_v01 *) payload;
  ptr->dual_standby_pref_valid = TRUE;
  ptr->dual_standby_pref = val;
}
void ril_utf_qmiril_if::set_qmi_nas_indication_register_req_subscription_info(uint8_t val)
{
  nas_indication_register_req_msg_v01 *ptr = (nas_indication_register_req_msg_v01 *) payload;
  ptr->subscription_info_valid = TRUE;
  ptr->subscription_info = val;
}
void ril_utf_qmiril_if::set_qmi_nas_indication_register_req_reg_network_time(uint8_t val)
{
  nas_indication_register_req_msg_v01 *ptr = (nas_indication_register_req_msg_v01 *) payload;
  ptr->reg_network_time_valid = TRUE;
  ptr->reg_network_time = val;
}
void ril_utf_qmiril_if::set_qmi_nas_indication_register_req_reg_rtre_cfg(uint8_t val)
{
  nas_indication_register_req_msg_v01 *ptr = (nas_indication_register_req_msg_v01 *) payload;
  ptr->reg_rtre_cfg_valid = TRUE;
  ptr->reg_rtre_cfg = val;
}
void ril_utf_qmiril_if::set_qmi_nas_indication_register_req_reg_embms_status(uint8_t val)
{
  nas_indication_register_req_msg_v01 *ptr = (nas_indication_register_req_msg_v01 *) payload;
  ptr->reg_embms_status_valid = TRUE;
  ptr->reg_embms_status = val;
}
void ril_utf_qmiril_if::set_qmi_nas_indication_register_req_sig_info(uint8_t val)
{
  nas_indication_register_req_msg_v01 *ptr = (nas_indication_register_req_msg_v01 *) payload;
  ptr->sig_info_valid = TRUE;
  ptr->sig_info = val;
}
void ril_utf_qmiril_if::set_qmi_nas_indication_register_req_err_rate(uint8_t val)
{
  nas_indication_register_req_msg_v01 *ptr = (nas_indication_register_req_msg_v01 *) payload;
  ptr->err_rate_valid = TRUE;
  ptr->err_rate = val;
}
void ril_utf_qmiril_if::set_qmi_nas_indication_register_req_reg_managed_roaming(uint8_t val)
{
  nas_indication_register_req_msg_v01 *ptr = (nas_indication_register_req_msg_v01 *) payload;
  ptr->reg_hdr_session_close_valid = TRUE;
  ptr->reg_hdr_session_close = val;
}
void ril_utf_qmiril_if::set_qmi_nas_indication_register_req_reg_hdr_session_close(uint8_t val)
{
  nas_indication_register_req_msg_v01 *ptr = (nas_indication_register_req_msg_v01 *) payload;
  ptr->reg_hdr_session_close_valid = TRUE;
  ptr->reg_hdr_session_close = val;
}
void ril_utf_qmiril_if::set_qmi_nas_indication_register_req_reg_hdr_uati(uint8_t val)
{
  nas_indication_register_req_msg_v01 *ptr = (nas_indication_register_req_msg_v01 *) payload;
  ptr->reg_hdr_uati_valid = TRUE;
  ptr->reg_hdr_uati = val;
}
void ril_utf_qmiril_if::set_qmi_nas_indication_register_req_reg_current_plmn_name(uint8_t val)
{
  nas_indication_register_req_msg_v01 *ptr = (nas_indication_register_req_msg_v01 *) payload;
  ptr->reg_current_plmn_name_valid = TRUE;
  ptr->reg_current_plmn_name = val;
}
void ril_utf_qmiril_if::set_qmi_nas_indication_register_req_sys_info(uint8_t val)
{
  nas_indication_register_req_msg_v01 *ptr = (nas_indication_register_req_msg_v01 *) payload;
  ptr->sys_info_valid = TRUE;
  ptr->sys_info = val;
}
void ril_utf_qmiril_if::set_qmi_nas_indication_register_req_req_serving_system(uint8_t val)
{
  nas_indication_register_req_msg_v01 *ptr = (nas_indication_register_req_msg_v01 *) payload;
  ptr->req_serving_system_valid = TRUE;
  ptr->req_serving_system = val;
}

void ril_utf_qmiril_if::set_qmi_nas_indication_register_req_reg_rf_band_info(uint8_t val)
{
  nas_indication_register_req_msg_v01 *ptr = (nas_indication_register_req_msg_v01 *) payload;
  ptr->reg_rf_band_info_valid = TRUE;
  ptr->reg_rf_band_info = val;
}

void ril_utf_qmiril_if::set_qmi_nas_indication_register_req_msg_network_reject(uint8_t reg_network_reject, uint8_t suppress_sys_info)
{
  nas_indication_register_req_msg_v01 *ptr = (nas_indication_register_req_msg_v01 *) payload;
  ptr->network_reject_valid = TRUE;
  ptr->network_reject.reg_network_reject = reg_network_reject;
  ptr->network_reject.suppress_sys_info = suppress_sys_info;
}

/*========================================================
  QMI_NAS_INDICATION_REGISTER_RESP_MSG_V01
========================================================*/

void ril_utf_qmiril_if::update_default_qmi_nas_indication_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_indication_register_resp_msg_v01));
  payload_len = sizeof(nas_indication_register_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_INDICATION_REGISTER_RESP_MSG_V01;

  nas_indication_register_resp_msg_v01 *ptr = (nas_indication_register_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

/*========================================================
  QMI_NAS_FORCE_NETWORK_SEARCH_REQ_MSG_V01
========================================================*/
/*========================================================
  QMI_NAS_FORCE_NETWORK_SEARCH_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_force_network_search_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_force_network_search_resp_msg_v01));
  payload_len = sizeof(nas_force_network_search_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_FORCE_NETWORK_SEARCH_RESP_MSG_V01;

  nas_force_network_search_resp_msg_v01 *ptr = (nas_force_network_search_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

/*========================================================
  QMI_NAS_SET_EVENT_REPORT_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_set_event_report_req_msg()
{
  payload = (void *) malloc(sizeof(nas_set_event_report_req_msg_v01));
  payload_len = sizeof(nas_set_event_report_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_SET_EVENT_REPORT_REQ_MSG_V01;
}
nas_signal_stregth_indicator_type_v01* ril_utf_qmiril_if::set_qmi_nas_set_event_report_req_signal_strength()
{
  nas_set_event_report_req_msg_v01 *ptr = (nas_set_event_report_req_msg_v01 *) payload;
  ptr->signal_strength_valid = TRUE;
  return &(ptr->signal_strength);
}
void ril_utf_qmiril_if::set_qmi_nas_set_event_report_req_report_rf_band_info(uint8_t val)
{
  nas_set_event_report_req_msg_v01 *ptr = (nas_set_event_report_req_msg_v01 *) payload;
  ptr->report_rf_band_info_valid = TRUE;
  ptr->report_rf_band_info = val;
}
void ril_utf_qmiril_if::set_qmi_nas_set_event_report_req_report_reg_reject(uint8_t val)
{
  nas_set_event_report_req_msg_v01 *ptr = (nas_set_event_report_req_msg_v01 *) payload;
  ptr->report_reg_reject_valid = TRUE;
  ptr->report_reg_reject = val;
}
nas_rssi_indicator_type_v01* ril_utf_qmiril_if::set_qmi_nas_set_event_report_req_rssi_indicator()
{
  nas_set_event_report_req_msg_v01 *ptr = (nas_set_event_report_req_msg_v01 *) payload;
  ptr->rssi_indicator_valid = TRUE;
  return &(ptr->rssi_indicator);
}
nas_ecio_indicator_type_v01* ril_utf_qmiril_if::set_qmi_nas_set_event_report_req_ecio_indicator()
{
  nas_set_event_report_req_msg_v01 *ptr = (nas_set_event_report_req_msg_v01 *) payload;
  ptr->ecio_indicator_valid = TRUE;
  return &(ptr->ecio_indicator);
}
nas_io_indicator_type_v01* ril_utf_qmiril_if::set_qmi_nas_set_event_report_req_io_indicator()
{
  nas_set_event_report_req_msg_v01 *ptr = (nas_set_event_report_req_msg_v01 *) payload;
  ptr->io_indicator_valid = TRUE;
  return &(ptr->io_indicator);
}
nas_sinr_indicator_type_v01* ril_utf_qmiril_if::set_qmi_nas_set_event_report_req_sinr_indicator()
{
  nas_set_event_report_req_msg_v01 *ptr = (nas_set_event_report_req_msg_v01 *) payload;
  ptr->sinr_indicator_valid = TRUE;
  return &(ptr->sinr_indicator);
}
void ril_utf_qmiril_if::set_qmi_nas_set_event_report_req_report_error_rate(uint8_t val)
{
  nas_set_event_report_req_msg_v01 *ptr = (nas_set_event_report_req_msg_v01 *) payload;
  ptr->report_error_rate_valid = TRUE;
  ptr->report_error_rate = val;
}
nas_rsrq_indicator_type_v01* ril_utf_qmiril_if::set_qmi_nas_set_event_report_req_rsrq_indicator()
{
  nas_set_event_report_req_msg_v01 *ptr = (nas_set_event_report_req_msg_v01 *) payload;
  ptr->rsrq_indicator_valid = TRUE;
  return &(ptr->rsrq_indicator);
}

/*========================================================
  QMI_NAS_SET_EVENT_REPORT_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_set_event_report_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_set_event_report_resp_msg_v01));
  payload_len = sizeof(nas_set_event_report_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_SET_EVENT_REPORT_REQ_MSG_V01;

  nas_set_event_report_resp_msg_v01 *ptr = (nas_set_event_report_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
/*========================================================
  QMI_NAS_CONFIG_SIG_INFO_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_config_sig_info_req_msg()
{
  payload = (void *) malloc(sizeof(nas_config_sig_info_req_msg_v01));
  payload_len = sizeof(nas_config_sig_info_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_CONFIG_SIG_INFO_REQ_MSG_V01;
}
int8_t* ril_utf_qmiril_if::set_qmi_nas_config_sig_info_req_rssi_threshold_list(uint32_t rssi_threshold_list_len)
{
  nas_config_sig_info_req_msg_v01 *ptr = (nas_config_sig_info_req_msg_v01 *) payload;
  ptr->rssi_threshold_list_valid = TRUE;
  ptr->rssi_threshold_list_len = rssi_threshold_list_len;
  return (ptr->rssi_threshold_list) ;
}
int16_t* ril_utf_qmiril_if::set_qmi_nas_config_sig_info_req_ecio_threshold_list(uint32_t ecio_threshold_list_len)
{
  nas_config_sig_info_req_msg_v01 *ptr = (nas_config_sig_info_req_msg_v01 *) payload;
  ptr->ecio_threshold_list_valid = TRUE;
  ptr->ecio_threshold_list_len = ecio_threshold_list_len;
  return (ptr->ecio_threshold_list) ;
}
uint8_t* ril_utf_qmiril_if::set_qmi_nas_config_sig_info_req_hdr_sinr_threshold_list(uint32_t hdr_sinr_threshold_list_len)
{
  nas_config_sig_info_req_msg_v01 *ptr = (nas_config_sig_info_req_msg_v01 *) payload;
  ptr->hdr_sinr_threshold_list_valid = TRUE;
  ptr->hdr_sinr_threshold_list_len = hdr_sinr_threshold_list_len;
  return (ptr->hdr_sinr_threshold_list) ;
}
int16_t* ril_utf_qmiril_if::set_qmi_nas_config_sig_info_req_lte_snr_threshold_list(uint32_t lte_snr_threshold_list_len)
{
  nas_config_sig_info_req_msg_v01 *ptr = (nas_config_sig_info_req_msg_v01 *) payload;
  ptr->lte_snr_threshold_list_valid = TRUE;
  ptr->lte_snr_threshold_list_len = lte_snr_threshold_list_len;
  return (ptr->lte_snr_threshold_list) ;
}
int8_t* ril_utf_qmiril_if::set_qmi_nas_config_sig_info_req_lte_rsrq_threshold_list(uint32_t lte_rsrq_threshold_list_len)
{
  nas_config_sig_info_req_msg_v01 *ptr = (nas_config_sig_info_req_msg_v01 *) payload;
  ptr->lte_rsrq_threshold_list_valid = TRUE;
  ptr->lte_rsrq_threshold_list_len = lte_rsrq_threshold_list_len;
  return (ptr->lte_rsrq_threshold_list) ;
}
int16_t* ril_utf_qmiril_if::set_qmi_nas_config_sig_info_req_lte_rsrp_threshold_list(uint32_t lte_rsrp_threshold_list_len)
{
  nas_config_sig_info_req_msg_v01 *ptr = (nas_config_sig_info_req_msg_v01 *) payload;
  ptr->lte_rsrp_threshold_list_valid = TRUE;
  ptr->lte_rsrp_threshold_list_len = lte_rsrp_threshold_list_len;
  return (ptr->lte_rsrp_threshold_list) ;
}
int8_t* ril_utf_qmiril_if::set_qmi_nas_config_sig_info_req_rscp_threshold_list(uint32_t rscp_threshold_list_len)
{
  nas_config_sig_info_req_msg_v01 *ptr = (nas_config_sig_info_req_msg_v01 *) payload;
  ptr->rscp_threshold_list_valid = TRUE;
  ptr->rscp_threshold_list_len = rscp_threshold_list_len;
  return (ptr->rscp_threshold_list) ;
}

/*========================================================
  QMI_NAS_CONFIG_SIG_INFO_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_config_sig_info_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_config_sig_info_resp_msg_v01));
  payload_len = sizeof(nas_config_sig_info_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_CONFIG_SIG_INFO_RESP_MSG_V01;

  nas_config_sig_info_resp_msg_v01 *ptr = (nas_config_sig_info_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

/*========================================================
  QMI_NAS_LIMIT_SYS_INFO_IND_REPORTING_REQ_MSG_V01
========================================================*/

void ril_utf_qmiril_if::update_default_qmi_nas_limit_sys_info_ind_reporting_req_msg(nas_limit_sys_info_ind_mask_type_v01 limit_sys_info_chg_rpt)
{
  payload = (void *) malloc(sizeof(nas_limit_sys_info_ind_reporting_req_msg_v01));
  payload_len = sizeof(nas_limit_sys_info_ind_reporting_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_LIMIT_SYS_INFO_IND_REPORTING_REQ_MSG_V01;

  nas_limit_sys_info_ind_reporting_req_msg_v01 *ptr = (nas_limit_sys_info_ind_reporting_req_msg_v01 *) payload;
  ptr->limit_sys_info_chg_rpt = limit_sys_info_chg_rpt;
}
/*========================================================
  QMI_NAS_LIMIT_SYS_INFO_IND_REPORTING_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_limit_sys_info_ind_reporting_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_limit_sys_info_ind_reporting_resp_msg_v01));
  payload_len = sizeof(nas_limit_sys_info_ind_reporting_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_LIMIT_SYS_INFO_IND_REPORTING_RESP_MSG_V01;

  nas_limit_sys_info_ind_reporting_resp_msg_v01 *ptr = (nas_limit_sys_info_ind_reporting_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
/*========================================================
  QMI_NAS_GET_MODE_PREF_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_mode_pref_req()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_MODE_PREF_REQ_MSG_V01;
}

/*========================================================
  QMI_NAS_GET_MODE_PREF_RESP_MSG_V01
========================================================*/

void ril_utf_qmiril_if::update_default_qmi_nas_get_mode_pref_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_get_mode_pref_resp_msg_v01));
  payload_len = sizeof(nas_get_mode_pref_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_MODE_PREF_RESP_MSG_V01;

  nas_get_mode_pref_resp_msg_v01 *ptr = (nas_get_mode_pref_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_nas_get_mode_pref_resp_idx0_mode_pref(mode_pref_mask_type_v01 val)
{
  nas_get_mode_pref_resp_msg_v01 *ptr = (nas_get_mode_pref_resp_msg_v01 *) payload;
  ptr->idx0_mode_pref_valid = TRUE;
  ptr->idx0_mode_pref = val;
}
void ril_utf_qmiril_if::set_qmi_nas_get_mode_pref_resp_idx1_mode_pref(mode_pref_mask_type_v01 val)
{
  nas_get_mode_pref_resp_msg_v01 *ptr = (nas_get_mode_pref_resp_msg_v01 *) payload;
  ptr->idx1_mode_pref_valid = TRUE;
  ptr->idx1_mode_pref = val;
}
void ril_utf_qmiril_if::set_qmi_nas_get_mode_pref_resp_idx2_mode_pref(mode_pref_mask_type_v01 val)
{
  nas_get_mode_pref_resp_msg_v01 *ptr = (nas_get_mode_pref_resp_msg_v01 *) payload;
  ptr->idx2_mode_pref_valid = TRUE;
  ptr->idx2_mode_pref = val;
}
/*========================================================
  QMI_NAS_SET_RTRE_CONFIG_REQ_MSG_V01
========================================================*/

void ril_utf_qmiril_if::update_default_qmi_nas_set_rtre_config_req_msg(nas_rtre_cfg_enum_v01 rtre_cfg_pref, int spc_valid, char *spc, int spc_len)
{
  payload = (void *) malloc(sizeof(nas_set_rtre_config_req_v01));
  payload_len = sizeof(nas_set_rtre_config_req_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_SET_RTRE_CONFIG_REQ_MSG_V01;

  nas_set_rtre_config_req_v01 *ptr = (nas_set_rtre_config_req_v01 *) payload;
  ptr->rtre_cfg_pref = rtre_cfg_pref;
  ptr->spc_valid = spc_valid;
  if( TRUE == spc_valid )
  {
    strlcpy(ptr->spc, spc, spc_len+1);
  }
}
/*========================================================
  QMI_NAS_SET_RTRE_CONFIG_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_set_rtre_config_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_set_rtre_config_resp_v01));
  payload_len = sizeof(nas_set_rtre_config_resp_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_SET_RTRE_CONFIG_RESP_MSG_V01;

  nas_set_rtre_config_resp_v01 *ptr = (nas_set_rtre_config_resp_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
/*========================================================
  QMI_NAS_GET_RTRE_CONFIG_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_rtre_config_req_msg(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_RTRE_CONFIG_REQ_MSG_V01;
  status = not_met;
}
/*========================================================
  QMI_NAS_GET_RTRE_CONFIG_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_rtre_config_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_get_rtre_config_resp_v01));
  payload_len = sizeof(nas_get_rtre_config_resp_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_RTRE_CONFIG_RESP_MSG_V01;

  nas_get_rtre_config_resp_v01 *ptr = (nas_get_rtre_config_resp_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_nas_get_rtre_config_resp_rtre_cfg(nas_rtre_cfg_enum_v01 val)
{
  nas_get_rtre_config_resp_v01 *ptr = (nas_get_rtre_config_resp_v01 *) payload;
  ptr->rtre_cfg_valid = TRUE;
  ptr->rtre_cfg = val;
}

/*========================================================
  QMI_NAS_UPDATE_AKEY_REQ_MSG_V01
========================================================*/
uint8_t* ril_utf_qmiril_if::update_default_qmi_nas_akey_req_msg()
{
  payload = (void *) malloc(sizeof(nas_update_akey_req_msg_v01));
  payload_len = sizeof(nas_update_akey_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_UPDATE_AKEY_REQ_MSG_V01;

  nas_update_akey_req_msg_v01* ptr = (nas_update_akey_req_msg_v01 *) payload;
  return (ptr->akey);
}
/*========================================================
  QMI_NAS_UPDATE_AKEY_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_akey_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_update_akey_resp_msg_v01));
  payload_len = sizeof(nas_update_akey_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_UPDATE_AKEY_RESP_MSG_V01;

  nas_update_akey_resp_msg_v01 *ptr = (nas_update_akey_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

/*========================================================
  QMI_NAS_GET_HOME_NETWORK_REQ_MSG_V01
========================================================*/
/*========================================================
  QMI_NAS_GET_HOME_NETWORK_RESP_MSG_V01
========================================================*/
nas_plmn_type_v01* ril_utf_qmiril_if::update_default_qmi_nas_get_home_network_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_get_home_network_resp_msg_v01));
  payload_len = sizeof(nas_get_home_network_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_HOME_NETWORK_RESP_MSG_V01;

  nas_get_home_network_resp_msg_v01 *ptr = (nas_get_home_network_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
  return &(ptr->home_network);
}
/*========================================================
  QMI_NAS_GET_MOB_CAI_REV_REQ_MSG_V01
========================================================*/

/*========================================================
  QMI_NAS_GET_MOB_CAI_REV_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_mob_cai_rev_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
    payload = (void *) malloc(sizeof(nas_get_mob_cai_rev_resp_v01));
  payload_len = sizeof(nas_get_mob_cai_rev_resp_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_MOB_CAI_REV_RESP_MSG_V01;

  nas_get_mob_cai_rev_resp_v01 *ptr = (nas_get_mob_cai_rev_resp_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

/*========================================================
  QMI_NAS_EVENT_REPORT_IND_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_event_report_ind_msg()
{
  payload = (void *) malloc(sizeof(nas_event_report_ind_msg_v01));
  payload_len = sizeof(nas_event_report_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_EVENT_REPORT_IND_MSG_V01;
}
void ril_utf_qmiril_if::set_qmi_nas_even_report_ind_signal_strength(int8_t sig_strength, nas_radio_if_enum_v01 radio_if)
{
  nas_event_report_ind_msg_v01 *ptr = (nas_event_report_ind_msg_v01 *) payload;
  ptr->signal_strength_valid = TRUE;
  (ptr->signal_strength).sig_strength = sig_strength;
  (ptr->signal_strength).radio_if = radio_if;
}
void ril_utf_qmiril_if::set_qmi_nas_even_report_ind_rssi(uint8_t rssi, nas_radio_if_enum_v01 radio_if)
{
  nas_event_report_ind_msg_v01 *ptr = (nas_event_report_ind_msg_v01 *) payload;
  ptr->rssi_valid = TRUE;
  (ptr->rssi).rssi = rssi;
  (ptr->rssi).radio_if = radio_if;
}
void ril_utf_qmiril_if::set_qmi_nas_even_report_ind_error_rate(uint16_t error_rate, nas_radio_if_enum_v01 radio_if)
{
  nas_event_report_ind_msg_v01 *ptr = (nas_event_report_ind_msg_v01 *) payload;
  ptr->error_rate_valid = TRUE;
  (ptr->error_rate).error_rate = error_rate;
  (ptr->error_rate).radio_if = radio_if;
}
void ril_utf_qmiril_if::set_qmi_nas_even_report_ind_ecio(uint8_t ecio, nas_radio_if_enum_v01 radio_if)
{
  nas_event_report_ind_msg_v01 *ptr = (nas_event_report_ind_msg_v01 *) payload;
  ptr->ecio_valid = TRUE;
  (ptr->ecio).ecio = ecio;
  (ptr->ecio).radio_if = radio_if;
}
void ril_utf_qmiril_if::set_qmi_nas_even_report_ind_io(int32_t val)
{
  nas_event_report_ind_msg_v01 *ptr = (nas_event_report_ind_msg_v01 *) payload;
  ptr->io_valid = TRUE;
  ptr->io = val;
}
void ril_utf_qmiril_if::set_qmi_nas_even_report_ind_sinr(nas_sinr_enum_v01 sinr)
{
  nas_event_report_ind_msg_v01 *ptr = (nas_event_report_ind_msg_v01 *) payload;
  ptr->sinr_valid = TRUE;
  ptr->sinr = sinr;
}
void ril_utf_qmiril_if::set_qmi_nas_even_report_ind_rsrq(int8_t rsrq, uint8_t radio_if)
{
  nas_event_report_ind_msg_v01 *ptr = (nas_event_report_ind_msg_v01 *) payload;
  ptr->rsrq_valid = TRUE;
  (ptr->rsrq).rsrq = rsrq;
  (ptr->rsrq).radio_if = radio_if;
}
void ril_utf_qmiril_if::set_qmi_nas_even_report_ind_registration_reject_reason(nas_network_service_domain_enum_v01 service_domain, uint16_t reject_cause)
{
  nas_event_report_ind_msg_v01 *ptr = (nas_event_report_ind_msg_v01 *) payload;
  ptr->registration_reject_reason_valid = TRUE;
  (ptr->registration_reject_reason).service_domain = service_domain;
  (ptr->registration_reject_reason).reject_cause = reject_cause;
}
nas_rf_band_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_even_report_ind_rf_band_info_list(uint32_t rf_band_info_list_len)
{
  nas_event_report_ind_msg_v01 *ptr = (nas_event_report_ind_msg_v01 *) payload;
  ptr->rf_band_info_list_valid = TRUE;
  return (ptr->rf_band_info_list);
}
/*========================================================
  QMI_NAS_SIG_INFO_IND_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_sig_info_ind_msg()
{
  payload = (void *) malloc(sizeof(nas_sig_info_ind_msg_v01));
  payload_len = sizeof(nas_sig_info_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_SIG_INFO_IND_MSG_V01;
}
void ril_utf_qmiril_if::set_qmi_nas_sig_info_ind_cdma_sig_info (int8_t rssi, int16_t ecio)
{
  nas_sig_info_ind_msg_v01 *ptr = (nas_sig_info_ind_msg_v01 *) payload;
  ptr->cdma_sig_info_valid = TRUE;
  (ptr->cdma_sig_info).rssi = rssi;
  (ptr->cdma_sig_info).ecio = ecio;
}
nas_hdr_sig_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_sig_info_ind_hdr_sig_info()
{
  nas_sig_info_ind_msg_v01 *ptr = (nas_sig_info_ind_msg_v01 *) payload;
  ptr->hdr_sig_info_valid = TRUE;
  return &(ptr->hdr_sig_info);
}
void ril_utf_qmiril_if::set_qmi_nas_sig_info_ind_gsm_sig_info(int8_t val)
{
  nas_sig_info_ind_msg_v01 *ptr = (nas_sig_info_ind_msg_v01 *) payload;
  ptr->gsm_sig_info_valid = TRUE;
  ptr->gsm_sig_info = val;
}
void ril_utf_qmiril_if::set_qmi_nas_sig_info_ind_wcdma_sig_info(int8_t rssi, int16_t ecio)
{
  nas_sig_info_ind_msg_v01 *ptr = (nas_sig_info_ind_msg_v01 *) payload;
  ptr->wcdma_sig_info_valid = TRUE;
  (ptr->wcdma_sig_info).rssi = rssi;
  (ptr->wcdma_sig_info).ecio = ecio;
}
void ril_utf_qmiril_if::set_qmi_nas_sig_info_ind_rscp(int8_t val)
{
  nas_sig_info_ind_msg_v01 *ptr = (nas_sig_info_ind_msg_v01 *) payload;
  ptr->rscp_valid = TRUE;
  ptr->rscp = val;
}
void ril_utf_qmiril_if::set_qmi_nas_sig_info_ind_lte_sig_info(int8_t rssi, int8_t rsrq, int16_t rsrp, int16_t snr)
{
  nas_sig_info_ind_msg_v01 *ptr = (nas_sig_info_ind_msg_v01 *) payload;
  ptr->lte_sig_info_valid = TRUE;
  (ptr->lte_sig_info).rssi = rssi;
  (ptr->lte_sig_info).rsrq = rsrq;
  (ptr->lte_sig_info).rsrp = rsrp;
  (ptr->lte_sig_info).snr = snr;
}
/*========================================================
  QMI_NAS_ERR_RATE_IND_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_err_rate_ind_msg()
{
  payload = (void *) malloc(sizeof(nas_err_rate_ind_msg_v01));
  payload_len = sizeof(nas_err_rate_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_ERR_RATE_IND_MSG_V01;
}
void ril_utf_qmiril_if::set_qmi_nas_err_rate_ind_gsm_bit_err_rate(uint8_t val)
{
  nas_err_rate_ind_msg_v01 *ptr = (nas_err_rate_ind_msg_v01 *) payload;
  ptr->gsm_bit_err_rate_valid = TRUE;
  ptr->gsm_bit_err_rate = val;
}
void ril_utf_qmiril_if::set_qmi_nas_err_rate_ind_wcdma_block_err_rate(uint8_t val)
{
  nas_err_rate_ind_msg_v01 *ptr = (nas_err_rate_ind_msg_v01 *) payload;
  ptr->wcdma_block_err_rate_valid = TRUE;
  ptr->wcdma_block_err_rate = val;
}
void ril_utf_qmiril_if::set_qmi_nas_err_rate_ind_tdscdma_block_err_rate(uint8_t val)
{
  nas_err_rate_ind_msg_v01 *ptr = (nas_err_rate_ind_msg_v01 *) payload;
  ptr->tdscdma_block_err_rate_valid = TRUE;
  ptr->tdscdma_block_err_rate = val;
}

/*========================================================
  QMI_NAS_SERVING_SYSTEM_IND_MSG_V01
========================================================*/
nas_serving_system_type_v01* ril_utf_qmiril_if::update_default_qmi_nas_serving_system_ind_msg()
{
  payload = (void *) malloc(sizeof(nas_serving_system_ind_msg_v01));
  payload_len = sizeof(nas_serving_system_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_SERVING_SYSTEM_IND_MSG_V01;

  nas_serving_system_ind_msg_v01 *ptr = (nas_serving_system_ind_msg_v01 *) payload;
  return &(ptr->serving_system);
}
nas_data_capabilites_enum_v01* ril_utf_qmiril_if::set_qmi_nas_serving_system_ind_data_capabilities(uint32_t data_capabilities_len)
{
  nas_serving_system_ind_msg_v01 *ptr = (nas_serving_system_ind_msg_v01 *) payload;
  ptr->data_capabilities_valid = TRUE;
  ptr->data_capabilities_len = data_capabilities_len;
  return (ptr->data_capabilities);
}
nas_cdma_system_id_type_v01* ril_utf_qmiril_if::set_qmi_nas_serving_system_ind_cdma_system_id()
{
  nas_serving_system_ind_msg_v01 *ptr = (nas_serving_system_ind_msg_v01 *) payload;
  ptr->cdma_system_id_valid = TRUE;
  return &(ptr->cdma_system_id);
}
nas_cdma_base_station_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_serving_system_ind_cdma_base_station_info()
{
  nas_serving_system_ind_msg_v01 *ptr = (nas_serving_system_ind_msg_v01 *) payload;
  ptr->cdma_base_station_info_valid = TRUE;
  return &(ptr->cdma_base_station_info);
}
nas_ss_detailed_service_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_serving_system_ind_detailed_service_info()
{
  nas_serving_system_ind_msg_v01 *ptr = (nas_serving_system_ind_msg_v01 *) payload;
  ptr->detailed_service_info_valid = TRUE;
  return &(ptr->detailed_service_info);
}
void ril_utf_qmiril_if::set_qmi_nas_serving_system_ind_hdr_personality(nas_hdr_personality_enum_v01 val)
{
  nas_serving_system_ind_msg_v01 *ptr = (nas_serving_system_ind_msg_v01 *) payload;
  ptr->hdr_personality_valid = TRUE;
  ptr->hdr_personality = val;
}
void ril_utf_qmiril_if::set_qmi_nas_serving_system_ind_cell_id(uint32_t val)
{
  nas_serving_system_ind_msg_v01 *ptr = (nas_serving_system_ind_msg_v01 *) payload;
  ptr->cell_id_valid = TRUE;
  ptr->cell_id = val;
}
void ril_utf_qmiril_if::set_qmi_nas_serving_system_ind_lac(uint16_t val)
{
  nas_serving_system_ind_msg_v01 *ptr = (nas_serving_system_ind_msg_v01 *) payload;
  ptr->lac_valid = TRUE;
  ptr->lac = val;
}
void ril_utf_qmiril_if::set_qmi_nas_serving_system_ind_tac(uint16_t val)
{
  nas_serving_system_ind_msg_v01 *ptr = (nas_serving_system_ind_msg_v01 *) payload;
  ptr->tac_valid = TRUE;
  ptr->tac = val;
}
void ril_utf_qmiril_if::set_qmi_nas_serving_system_ind_srv_sys_no_change(uint8_t val)
{
  nas_serving_system_ind_msg_v01 *ptr = (nas_serving_system_ind_msg_v01 *) payload;
  ptr->srv_sys_no_change_valid = TRUE;
  ptr->srv_sys_no_change = val;
}

/*========================================================

  QMI_NAS_SYSTEM_SELECTION_PREFERENCE_IND_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_system_selection_preference_ind_msg()
{
  payload = (void *) malloc(sizeof(nas_system_selection_preference_ind_msg_v01));
  payload_len = sizeof(nas_system_selection_preference_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_SYSTEM_SELECTION_PREFERENCE_IND_MSG_V01;
}
void ril_utf_qmiril_if::set_qmi_nas_system_selection_preference_ind_srv_domain_pref
  (nas_srv_domain_pref_enum_type_v01 srv_domain_pref)
{
  nas_system_selection_preference_ind_msg_v01 *ptr = (nas_system_selection_preference_ind_msg_v01 *) payload;
  ptr->srv_domain_pref_valid = TRUE;
  ptr->srv_domain_pref = srv_domain_pref;
}
void ril_utf_qmiril_if::set_qmi_nas_system_selection_preference_ind_emergency_mode(uint8_t val)
{
  nas_system_selection_preference_ind_msg_v01 *ptr = (nas_system_selection_preference_ind_msg_v01 *) payload;
  ptr->emergency_mode_valid = TRUE;
  ptr->emergency_mode = val;
}
void ril_utf_qmiril_if::set_qmi_nas_system_selection_preference_ind_mode_pref(mode_pref_mask_type_v01 val)
{
  nas_system_selection_preference_ind_msg_v01 *ptr = (nas_system_selection_preference_ind_msg_v01 *) payload;
  ptr->mode_pref_valid = TRUE;
  ptr->mode_pref = val;
}
void ril_utf_qmiril_if::set_qmi_nas_system_selection_preference_ind_band_pref(nas_band_pref_mask_type_v01 val)
{
  nas_system_selection_preference_ind_msg_v01 *ptr = (nas_system_selection_preference_ind_msg_v01 *) payload;
  ptr->band_pref_valid = TRUE;
  ptr->band_pref = val;
}
void ril_utf_qmiril_if::set_qmi_nas_system_selection_preference_ind_prl_pref(nas_prl_pref_enum_v01 val)
{
  nas_system_selection_preference_ind_msg_v01 *ptr = (nas_system_selection_preference_ind_msg_v01 *) payload;
  ptr->prl_pref_valid = TRUE;
  ptr->prl_pref = val;
}
void ril_utf_qmiril_if::set_qmi_nas_system_selection_preference_ind_roam_pref(nas_roam_pref_enum_v01 val)
{
  nas_system_selection_preference_ind_msg_v01 *ptr = (nas_system_selection_preference_ind_msg_v01 *) payload;
  ptr->roam_pref_valid = TRUE;
  ptr->roam_pref = val;
}
void ril_utf_qmiril_if::set_qmi_nas_system_selection_preference_ind_lte_band_pref(lte_band_pref_mask_type_v01 val)
{
  nas_system_selection_preference_ind_msg_v01 *ptr = (nas_system_selection_preference_ind_msg_v01 *) payload;
  ptr->lte_band_pref_valid = TRUE;
  ptr->lte_band_pref = val;
}
void ril_utf_qmiril_if::set_qmi_nas_system_selection_preference_ind_net_sel_pref( nas_net_sel_pref_enum_v01 val)
{
  nas_system_selection_preference_ind_msg_v01 *ptr = (nas_system_selection_preference_ind_msg_v01 *) payload;
  ptr->net_sel_pref_valid = TRUE;
  ptr->net_sel_pref = val;
}
#ifdef RIL_UTF_L_MASTER
void ril_utf_qmiril_if::set_qmi_nas_system_selection_preference_ind_rat_disabled_mask(mode_pref_mask_type_v01 mask)
{
  nas_system_selection_preference_ind_msg_v01 *ptr = (nas_system_selection_preference_ind_msg_v01 *) payload;
  ptr->rat_disabled_mask_valid = TRUE;
  ptr->rat_disabled_mask = mask;
}
#endif
/*========================================================
  QMI_NAS_SYS_INFO_IND_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_sys_info_ind_msg()
{
  payload = (void *) malloc(sizeof(nas_sys_info_ind_msg_v01));
  payload_len = sizeof(nas_sys_info_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_SYS_INFO_IND_MSG_V01;
}
void ril_utf_qmiril_if::set_qmi_nas_sys_info_ind_sys_info_no_change()
{
  nas_sys_info_ind_msg_v01 *ptr = (nas_sys_info_ind_msg_v01 *) payload;
  ptr->sys_info_no_change_valid = TRUE;
  ptr->sys_info_no_change = 1;
}
void ril_utf_qmiril_if::set_qmi_nas_sys_info_ind_wcdma_eutra_status(nas_eutra_cell_status_enum_type_v01 val)
{
  nas_sys_info_ind_msg_v01 *ptr = (nas_sys_info_ind_msg_v01 *) payload;
  ptr->wcdma_eutra_status_valid = TRUE;
  ptr->wcdma_eutra_status = val;
}
nas_gsm_sys_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_sys_info_ind_gsm_sys_info()
{
  nas_sys_info_ind_msg_v01 *ptr = (nas_sys_info_ind_msg_v01 *) payload;
  ptr->gsm_sys_info_valid = TRUE;
  return &(ptr->gsm_sys_info);
}
void ril_utf_qmiril_if::set_qmi_nas_sys_info_ind_lte_voice_status(nas_lte_voice_status_enum_type_v01 lte_voice_status)
{
  nas_sys_info_ind_msg_v01 *ptr = (nas_sys_info_ind_msg_v01 *) payload;
  ptr->lte_voice_status_valid = TRUE;
  ptr->lte_voice_status = lte_voice_status;
}
void ril_utf_qmiril_if::set_qmi_nas_sys_info_ind_lte_sms_status(nas_sms_status_enum_type_v01 lte_sms_status)
{
  nas_sys_info_ind_msg_v01 *ptr = (nas_sys_info_ind_msg_v01 *) payload;
  ptr->lte_sms_status_valid = TRUE;
  ptr->lte_sms_status = lte_sms_status;
}
void ril_utf_qmiril_if::set_qmi_nas_sys_info_ind_lte_cell_status(nas_cell_access_status_e_type_v01 lte_cell_status)
{
  nas_sys_info_ind_msg_v01 *ptr = (nas_sys_info_ind_msg_v01 *) payload;
  ptr->lte_cell_status_valid = TRUE;
  ptr->lte_cell_status = lte_cell_status;
}
nas_3gpp_srv_status_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_sys_info_ind_wcdma_srv_status_info()
{
  nas_sys_info_ind_msg_v01 *ptr = (nas_sys_info_ind_msg_v01 *) payload;
  ptr->wcdma_srv_status_info_valid = TRUE;
  return &(ptr->wcdma_srv_status_info);
}
nas_wcdma_sys_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_sys_info_ind_wcdma_sys_info()
{
  nas_sys_info_ind_msg_v01 *ptr = (nas_sys_info_ind_msg_v01 *) payload;
  ptr->wcdma_sys_info_valid = TRUE;
  return &(ptr->wcdma_sys_info);
}
nas_tdscdma_sys_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_sys_info_ind_tdscdma_sys_info()
{
  nas_sys_info_ind_msg_v01 *ptr = (nas_sys_info_ind_msg_v01 *) payload;
  ptr->tdscdma_sys_info_valid = TRUE;
  return &(ptr->tdscdma_sys_info);
}
nas_lte_sys_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_sys_info_ind_lte_sys_info()
{
  nas_sys_info_ind_msg_v01 *ptr = (nas_sys_info_ind_msg_v01 *) payload;
  ptr->lte_sys_info_valid = TRUE;
  return &(ptr->lte_sys_info);
}
nas_cdma_sys_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_sys_info_ind_cdma_sys_info()
{
  nas_sys_info_ind_msg_v01 *ptr = (nas_sys_info_ind_msg_v01 *) payload;
  ptr->cdma_sys_info_valid = TRUE;
  return &(ptr->cdma_sys_info);
}
nas_hdr_sys_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_sys_info_ind_hdr_sys_info()
{
  nas_sys_info_ind_msg_v01 *ptr = (nas_sys_info_ind_msg_v01 *) payload;
  ptr->hdr_sys_info_valid = TRUE;
  return &(ptr->hdr_sys_info);
}
nas_3gpp2_srv_status_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_sys_info_ind_cdma_srv_status_info()
{
  nas_sys_info_ind_msg_v01 *ptr = (nas_sys_info_ind_msg_v01 *) payload;
  ptr->cdma_srv_status_info_valid = TRUE;
  return &(ptr->cdma_srv_status_info);
}
nas_3gpp2_srv_status_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_sys_info_ind_hdr_srv_status_info()
{
  nas_sys_info_ind_msg_v01 *ptr = (nas_sys_info_ind_msg_v01 *) payload;
  ptr->hdr_srv_status_info_valid = TRUE;
  return &(ptr->hdr_srv_status_info);
}
nas_3gpp_srv_status_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_sys_info_ind_gsm_srv_status_info()
{
  nas_sys_info_ind_msg_v01 *ptr = (nas_sys_info_ind_msg_v01 *) payload;
  ptr->gsm_srv_status_info_valid = TRUE;
  return &(ptr->gsm_srv_status_info);
}
nas_3gpp_srv_status_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_sys_info_ind_lte_srv_status_info()
{
  nas_sys_info_ind_msg_v01 *ptr = (nas_sys_info_ind_msg_v01 *) payload;
  ptr->lte_srv_status_info_valid = TRUE;
  return &(ptr->lte_srv_status_info);
}
nas_3gpp_srv_status_info_type_v01* ril_utf_qmiril_if::set_qmi_nas_sys_info_ind_tdscdma_srv_status_info()
{
  nas_sys_info_ind_msg_v01 *ptr = (nas_sys_info_ind_msg_v01 *) payload;
  ptr->tdscdma_srv_status_info_valid = TRUE;
  return &(ptr->tdscdma_srv_status_info);
}
nas_cdma_sys_info2_type_v01* ril_utf_qmiril_if::set_qmi_nas_sys_info_ind_cdma_sys_info2()
{
  nas_sys_info_ind_msg_v01 *ptr = (nas_sys_info_ind_msg_v01 *) payload;
  ptr->cdma_sys_info2_valid = TRUE;
  return &(ptr->cdma_sys_info2);
}
nas_hdr_sys_info2_type_v01* ril_utf_qmiril_if::set_qmi_nas_sys_info_ind_hdr_sys_info2()
{
  nas_sys_info_ind_msg_v01 *ptr = (nas_sys_info_ind_msg_v01 *) payload;
  ptr->hdr_sys_info2_valid = TRUE;
  return &(ptr->hdr_sys_info2);
}
nas_gsm_sys_info2_type_v01* ril_utf_qmiril_if::set_qmi_nas_sys_info_ind_gsm_sys_info2()
{
  nas_sys_info_ind_msg_v01 *ptr = (nas_sys_info_ind_msg_v01 *) payload;
  ptr->gsm_sys_info2_valid = TRUE;
  return &(ptr->gsm_sys_info2);
}
nas_wcdma_sys_info2_type_v01* ril_utf_qmiril_if::set_qmi_nas_sys_info_ind_wcdma_sys_info2()
{
  nas_sys_info_ind_msg_v01 *ptr = (nas_sys_info_ind_msg_v01 *) payload;
  ptr->wcdma_sys_info2_valid = TRUE;
  return &(ptr->wcdma_sys_info2);
}
nas_lte_sys_info2_type_v01* ril_utf_qmiril_if::set_qmi_nas_sys_info_ind_lte_sys_info2()
{
  nas_sys_info_ind_msg_v01 *ptr = (nas_sys_info_ind_msg_v01 *) payload;
  ptr->lte_sys_info2_valid = TRUE;
  return &(ptr->lte_sys_info2);
}
nas_gw_sys_info3_type_v01* ril_utf_qmiril_if::set_qmi_nas_sys_info_ind_gsm_sys_info3()
{
  nas_sys_info_ind_msg_v01 *ptr = (nas_sys_info_ind_msg_v01 *) payload;
  ptr->gsm_sys_info3_valid = TRUE;
  return &(ptr->gsm_sys_info3);
}
nas_gw_sys_info3_type_v01* ril_utf_qmiril_if::set_qmi_nas_sys_info_ind_wcdma_sys_info3()
{
  nas_sys_info_ind_msg_v01 *ptr = (nas_sys_info_ind_msg_v01 *) payload;
  ptr->wcdma_sys_info3_valid = TRUE;
  return &(ptr->wcdma_sys_info3);
}
void ril_utf_qmiril_if::set_qmi_nas_sys_info_ind_voice_support_on_lte(uint8_t val)
{
  nas_sys_info_ind_msg_v01 *ptr = (nas_sys_info_ind_msg_v01 *) payload;
  ptr->voice_support_on_lte_valid = TRUE;
  ptr->voice_support_on_lte = val;
}
void ril_utf_qmiril_if::set_qmi_nas_sys_info_ind_lte_embms_coverage(uint8_t val)
{
  nas_sys_info_ind_msg_v01 *ptr = (nas_sys_info_ind_msg_v01 *) payload;
  ptr->lte_embms_coverage_valid = TRUE;
  ptr->lte_embms_coverage = val;
}
void ril_utf_qmiril_if::set_qmi_nas_sys_info_ind_sim_rej_info(nas_sim_rej_info_enum_type_v01 val)
{
  nas_sys_info_ind_msg_v01 *ptr = (nas_sys_info_ind_msg_v01 *) payload;
  ptr->sim_rej_info_valid = TRUE;
  ptr->sim_rej_info = val;
}
void ril_utf_qmiril_if::set_qmi_nas_sys_info_ind_srv_reg_restriction(nas_srv_reg_restriction_enum_v01 val)
{
  nas_sys_info_ind_msg_v01 *ptr = (nas_sys_info_ind_msg_v01 *) payload;
  ptr->srv_reg_restriction_valid = TRUE;
  ptr->srv_reg_restriction = val;
}

/*========================================================
  QMI_NAS_OPERATOR_NAME_DATA_IND_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_operator_name_data_ind_msg()
{
  payload = (void *) malloc(sizeof(nas_operator_name_data_ind_msg_v01));
  payload_len = sizeof(nas_operator_name_data_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_OPERATOR_NAME_DATA_IND_MSG_V01;
}
nas_service_provider_name_type_v01* ril_utf_qmiril_if::set_qmi_nas_operator_name_data_ind_service_provider_name()
{
  nas_operator_name_data_ind_msg_v01 *ptr = (nas_operator_name_data_ind_msg_v01 *) payload;
  ptr->service_provider_name_valid = TRUE;
  return &(ptr->service_provider_name);
}
nas_operator_plmn_list_type_v01* ril_utf_qmiril_if::set_qmi_nas_operator_name_data_ind_operator_plmn_list(uint32_t operator_plmn_list_len)
{
  nas_operator_name_data_ind_msg_v01 *ptr = (nas_operator_name_data_ind_msg_v01 *) payload;
  ptr->operator_plmn_list_valid = TRUE;
  ptr->operator_plmn_list_len  = operator_plmn_list_len;
  return (ptr->operator_plmn_list);
}
nas_plmn_network_name_type_v01* ril_utf_qmiril_if::set_qmi_nas_operator_name_data_ind_plmn_network_name(uint32_t plmn_network_name_len)
{
  nas_operator_name_data_ind_msg_v01 *ptr = (nas_operator_name_data_ind_msg_v01 *) payload;
  ptr->plmn_network_name_valid = TRUE;
  ptr->plmn_network_name_len = plmn_network_name_len;
  return (ptr->plmn_network_name);
}
char* ril_utf_qmiril_if::set_qmi_nas_operator_name_data_ind_plmn_name()
{
  nas_operator_name_data_ind_msg_v01 *ptr = (nas_operator_name_data_ind_msg_v01 *) payload;
  ptr->plmn_name_valid = TRUE;
  return (ptr->plmn_name);
}
nas_plmn_network_name_type_v01* ril_utf_qmiril_if::set_qmi_nas_operator_name_data_ind_nitz_information()
{
  nas_operator_name_data_ind_msg_v01 *ptr = (nas_operator_name_data_ind_msg_v01 *) payload;
  ptr->nitz_information_valid = TRUE;
  return &(ptr->nitz_information);
}
/*========================================================
  QMI_NAS_CURRENT_PLMN_NAME_IND_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_current_plmn_name_ind_msg()
{
  payload = (void *) malloc(sizeof(nas_current_plmn_name_ind_msg_v01));
  payload_len = sizeof(nas_current_plmn_name_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_CURRENT_PLMN_NAME_IND_V01;
}
void ril_utf_qmiril_if::set_qmi_nas_current_plmn_name_ind_plmn_id(uint8_t plmn_valid, uint16_t mcc, uint16_t mnc, uint8_t mnc_includes_pcs_digit)
{
  nas_current_plmn_name_ind_msg_v01 *ptr = (nas_current_plmn_name_ind_msg_v01 *) payload;
  ptr->plmn_id_valid = plmn_valid;
  ptr->plmn_id.mcc = mcc;
  ptr->plmn_id.mnc = mnc;
  ptr->plmn_id.mnc_includes_pcs_digit = mnc_includes_pcs_digit;
}
void ril_utf_qmiril_if::set_qmi_nas_current_plmn_name_ind_spn(uint8_t spn_valid, nas_coding_scheme_enum_v01 spn_enc, uint32_t spn_len, const char *spn)
{
  nas_current_plmn_name_ind_msg_v01 *ptr = (nas_current_plmn_name_ind_msg_v01 *) payload;
  ptr->spn_valid = spn_valid;
  ptr->spn.spn_enc = spn_enc;
  ptr->spn.spn_len = spn_len;
  memcpy(ptr->spn.spn, spn, spn_len);
}
nas_plmn_name_type_v01* ril_utf_qmiril_if::set_qmi_nas_current_plmn_name_ind_short_name()
{
  nas_current_plmn_name_ind_msg_v01 *ptr = (nas_current_plmn_name_ind_msg_v01 *) payload;
  ptr->short_name_valid = TRUE;
  return &(ptr->short_name);
}
nas_plmn_name_type_v01* ril_utf_qmiril_if::set_qmi_nas_current_plmn_name_ind_long_name()
{
  nas_current_plmn_name_ind_msg_v01 *ptr = (nas_current_plmn_name_ind_msg_v01 *) payload;
  ptr->long_name_valid = TRUE;
  return &(ptr->long_name);
}
/*========================================================
  QMI_NAS_CSP_PLMN_MODE_BIT_IND_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_csp_plmn_mode_bit_ind_msg()
{
  payload = (void *) malloc(sizeof(nas_csp_plmn_mode_bit_ind_msg_v01));
  payload_len = sizeof(nas_csp_plmn_mode_bit_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_CSP_PLMN_MODE_BIT_IND_MSG_V01;
}
/*========================================================
  QMI_NAS_DUAL_STANDBY_PREF_IND_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_dual_standby_pref_ind_msg()
{
  payload = (void *) malloc(sizeof(nas_dual_standby_pref_ind_msg_v01));
  payload_len = sizeof(nas_dual_standby_pref_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_DUAL_STANDBY_PREF_IND_MSG_V01;
}
nas_standby_pref_type_v01* ril_utf_qmiril_if::set_qmi_nas_standby_pref_ind_standby_pref()
{
  nas_dual_standby_pref_ind_msg_v01 *ptr = (nas_dual_standby_pref_ind_msg_v01 *) payload;
  ptr->standby_pref_valid = TRUE;
  return &(ptr->standby_pref);
}

void ril_utf_qmiril_if::set_qmi_nas_standby_pref_ind_active_subs_mask(nas_active_subs_mask_type_v01 val)
{
  nas_dual_standby_pref_ind_msg_v01 *ptr = (nas_dual_standby_pref_ind_msg_v01 *) payload;
  ptr->active_subs_mask_valid = TRUE;
  ptr->active_subs_mask = val;
}

/*========================================================
  QMI_NAS_SUBSCRIPTION_INFO_IND_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_subscription_info_ind_msg()
{
  payload = (void *) malloc(sizeof(nas_subscription_info_ind_msg_v01));
  payload_len = sizeof(nas_subscription_info_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_SUBSCRIPTION_INFO_IND_MSG_V01;
}
void ril_utf_qmiril_if::set_qmi_nas_subscription_info_ind_is_priority_subs(nas_is_priority_subs_enum_v01 val)
{
  nas_subscription_info_ind_msg_v01 *ptr = (nas_subscription_info_ind_msg_v01 *) payload;
  ptr->is_priority_subs_valid = TRUE;
  ptr->is_priority_subs = val;
}
void ril_utf_qmiril_if::set_qmi_nas_subscription_info_ind_is_active(nas_active_subs_info_enum_v01 val)
{
  nas_subscription_info_ind_msg_v01 *ptr = (nas_subscription_info_ind_msg_v01 *) payload;
  ptr->is_active_valid = TRUE;
  ptr->is_active = val;
}
void ril_utf_qmiril_if::set_qmi_nas_subscription_info_ind_voice_system_id(uint32_t val)
{
  nas_subscription_info_ind_msg_v01 *ptr = (nas_subscription_info_ind_msg_v01 *) payload;
  ptr->voice_system_id_valid = TRUE;
  ptr->voice_system_id = val;
}
void ril_utf_qmiril_if::set_qmi_nas_subscription_info_ind_lte_voice_system_id(uint32_t val)
{
  nas_subscription_info_ind_msg_v01 *ptr = (nas_subscription_info_ind_msg_v01 *) payload;
  ptr->lte_voice_system_id_valid = TRUE;
  ptr->lte_voice_system_id = val;
}
void ril_utf_qmiril_if::set_qmi_nas_subscription_info_ind_wlan_voice_system_id(uint32_t val)
{
  nas_subscription_info_ind_msg_v01 *ptr = (nas_subscription_info_ind_msg_v01 *) payload;
  ptr->wlan_voice_system_id_valid = TRUE;
  ptr->wlan_voice_system_id = val;
}

/*========================================================
  QMI_NAS_NETWORK_TIME_IND_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_network_time_ind_msg(uint16_t year,uint8_t month,uint8_t day,uint8_t hour,uint8_t minute,uint8_t second,uint8_t day_of_week)
{
  payload = (void *) malloc(sizeof(nas_network_time_ind_msg_v01));
  payload_len = sizeof(nas_network_time_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_NETWORK_TIME_IND_MSG_V01;

  nas_network_time_ind_msg_v01 *ptr = (nas_network_time_ind_msg_v01 *) payload;
  (ptr->universal_time).year = year;
  (ptr->universal_time).month = month;
  (ptr->universal_time).day = day;
  (ptr->universal_time).hour = hour;
  (ptr->universal_time).minute = minute;
  (ptr->universal_time).second = second;
  (ptr->universal_time).day_of_week = day_of_week;
}
void ril_utf_qmiril_if::set_qmi_nas_network_time_ind_time_zone(int8_t val)
{
  nas_network_time_ind_msg_v01 *ptr = (nas_network_time_ind_msg_v01 *) payload;
  ptr->time_zone_valid = TRUE;
  ptr->time_zone = val;
}
void ril_utf_qmiril_if::set_qmi_nas_network_time_ind_daylt_sav_adj(uint8_t val)
{
  nas_network_time_ind_msg_v01 *ptr = (nas_network_time_ind_msg_v01 *) payload;
  ptr->daylt_sav_adj_valid = TRUE;
  ptr->daylt_sav_adj = val;
}

/*========================================================
  QMI_NAS_RTRE_CONFIG_IND_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_rtre_config_ind_msg()
{
  payload = (void *) malloc(sizeof(nas_rtre_cfg_ind_msg_v01));
  payload_len = sizeof(nas_rtre_cfg_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_RTRE_CONFIG_IND_V01;
}
void ril_utf_qmiril_if::set_qmi_nas_rtre_config_ind_rtre_cfg(nas_rtre_cfg_enum_v01 val)
{
  nas_rtre_cfg_ind_msg_v01 *ptr = (nas_rtre_cfg_ind_msg_v01 *) payload;
  ptr->rtre_cfg_valid = TRUE;
  ptr->rtre_cfg = val;
}
/*========================================================
  QMI_NAS_MANAGED_ROAMING_IND_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_managed_roaming_ind_msg()
{
  payload = (void *) malloc(sizeof(nas_managed_roaming_ind_msg_v01));
  payload_len = sizeof(nas_managed_roaming_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_MANAGED_ROAMING_IND_MSG_V01;
}
/*========================================================
  QMI_NAS_EMBMS_STATUS_IND_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_embms_status_ind_msg(uint8_t enabled)
{
  payload = (void *) malloc(sizeof(nas_embms_status_ind_msg_v01));
  payload_len = sizeof(nas_embms_status_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_EMBMS_STATUS_IND_V01;
  nas_embms_status_ind_msg_v01 *ptr = (nas_embms_status_ind_msg_v01 *) payload;
  ptr->enabled = enabled;
}
/*========================================================
  QMI_NAS_RF_BAND_INFO_IND_V01
========================================================*/
nas_rf_band_info_type_v01* ril_utf_qmiril_if::update_default_qmi_nas_rf_band_info_ind_msg()
{
  payload = (void *) malloc(sizeof(nas_rf_band_info_ind_msg_v01));
  payload_len = sizeof(nas_rf_band_info_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_RF_BAND_INFO_IND_V01;

  nas_rf_band_info_ind_msg_v01 *ptr = (nas_rf_band_info_ind_msg_v01 *) payload;
  return &(ptr->rf_band_info);
}

/*========================================================
  QMI_NAS_LTE_SIB16_NETWORK_TIME_IND_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_lte_sib16_network_time_ind()
{
  payload = (void *) malloc(sizeof(nas_lte_sib16_network_time_ind_msg_v01));
  payload_len = sizeof(nas_lte_sib16_network_time_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_LTE_SIB16_NETWORK_TIME_IND_V01;

}
void ril_utf_qmiril_if::set_qmi_nas_lte_sib16_network_time_ind_lte_sib16_acquired
  (nas_tri_state_boolean_type_v01 lte_sib16_acquired)
{
  nas_lte_sib16_network_time_ind_msg_v01 *ptr =
    (nas_lte_sib16_network_time_ind_msg_v01*) payload;

  ptr->lte_sib16_acquired_valid = TRUE;
  ptr->lte_sib16_acquired = lte_sib16_acquired;
}

void ril_utf_qmiril_if::set_qmi_nas_lte_sib16_network_time_ind_abs_time
  (uint64_t abs_time)
{
  nas_lte_sib16_network_time_ind_msg_v01 *ptr =
    (nas_lte_sib16_network_time_ind_msg_v01*) payload;

  ptr->abs_time_valid = TRUE;
  ptr->abs_time = abs_time;
}

/*========================================================
  QMI_NAS_GET_LTE_SIB16_NETWORK_TIME_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_lte_sib16_network_time_req_msg_v01()
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_LTE_SIB16_NETWORK_TIME_REQ_MSG_V01;
}
/*========================================================
  QMI_NAS_GET_LTE_SIB16_NETWORK_TIME_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_lte_sib16_network_time_resp_msg_v01
  (qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_get_lte_sib16_network_time_resp_msg_v01));
  payload_len = sizeof(nas_get_lte_sib16_network_time_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_LTE_SIB16_NETWORK_TIME_RESP_MSG_V01;

  nas_get_lte_sib16_network_time_resp_msg_v01 *ptr =
    (nas_get_lte_sib16_network_time_resp_msg_v01*) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}


/*========================================================
  QMI_DMS_GET_DEVICE_MFR_REQ_V01
========================================================*/
/*========================================================
  QMI_DMS_GET_DEVICE_MFR_RESP_V01
========================================================*/
char* ril_utf_qmiril_if::update_default_qmi_dms_get_device_mfr_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(dms_get_device_mfr_resp_msg_v01));
  payload_len = sizeof(dms_get_device_mfr_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_GET_DEVICE_MFR_RESP_V01;

  dms_get_device_mfr_resp_msg_v01 *ptr = (dms_get_device_mfr_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
  return ptr->device_manufacturer;
}

/*========================================================
  QMI_DMS_BIND_SUBSCRIPTION_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_bind_subscription_req_msg(dms_bind_subscription_enum_v01 bind_subs)
{
  payload = (void *) malloc(sizeof(dms_bind_subscription_req_msg_v01));
  payload_len = sizeof(dms_bind_subscription_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_BIND_SUBSCRIPTION_REQ_V01;

  dms_bind_subscription_req_msg_v01 *ptr = (dms_bind_subscription_req_msg_v01 *) payload;
  ptr->bind_subs = bind_subs;
}
/*========================================================
  QMI_DMS_BIND_SUBSCRIPTION_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_bind_subscription_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(dms_bind_subscription_resp_msg_v01));
  payload_len = sizeof(dms_bind_subscription_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_BIND_SUBSCRIPTION_RESP_V01;

  dms_bind_subscription_resp_msg_v01 *ptr = (dms_bind_subscription_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
/*========================================================
  QMI_DMS_GET_DEVICE_CAP_REQ_V01
========================================================*/
/*========================================================
  QMI_DMS_GET_DEVICE_CAP_RESP_V01
========================================================*/
dms_device_capabilities_type_v01* ril_utf_qmiril_if::update_default_qmi_dms_get_device_cap_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(dms_get_device_cap_resp_msg_v01));
  payload_len = sizeof(dms_get_device_cap_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_GET_DEVICE_CAP_RESP_V01;

  dms_get_device_cap_resp_msg_v01 *ptr = (dms_get_device_cap_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
  return &(ptr->device_capabilities);
}
/*========================================================
  QMI_DMS_SET_EVENT_REPORT_REQ_V01
========================================================*/
  /*
void ril_utf_qmiril_if::update_default_qmi_dms_set_event_report_req_msg()
{
  payload = (void *) malloc(sizeof(dms_set_event_report_req_msg_v01));
  payload_len = sizeof(dms_set_event_report_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_SET_EVENT_REPORT_REQ_V01;
}
void ril_utf_qmiril_if::set_qmi_dms_set_event_report_req_report_oprt_mode_state(int val)
{
  dms_set_event_report_req_msg_v01 *ptr = (dms_set_event_report_req_msg_v01 *) payload;
  ptr->report_oprt_mode_state_valid = TRUE;
  ptr->report_oprt_mode_state = val;
}
void ril_utf_qmiril_if::set_qmi_dms_set_event_report_req_report_prl_init(int val)
{
  dms_set_event_report_req_msg_v01 *ptr = (dms_set_event_report_req_msg_v01 *) payload;
  ptr->report_prl_init_valid = TRUE;
  ptr->report_prl_init = val;
}
*/
/*========================================================
  QMI_DMS_SET_EVENT_REPORT_RESP_V01
========================================================*/
// implemented above already..

/*========================================================
  QMI_DMS_GET_CURRENT_PRL_INFO_REQ_V01
========================================================*/
/*========================================================
  QMI_DMS_GET_CURRENT_PRL_INFO_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_get_current_prl_info_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(dms_get_current_prl_info_resp_msg_v01));
  payload_len = sizeof(dms_get_current_prl_info_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_GET_CURRENT_PRL_INFO_RESP_V01;
}
void ril_utf_qmiril_if::set_qmi_dms_get_current_prl_info_resp_prl_version(uint16_t val)
{
  dms_get_current_prl_info_resp_msg_v01 *ptr = (dms_get_current_prl_info_resp_msg_v01 *) payload;
  ptr->prl_version_valid = TRUE;
  ptr->prl_version = val;
}
/*========================================================
  QMI_DMS_SET_OPERATING_MODE_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_set_operating_mode_req_msg(dms_operating_mode_enum_v01 operating_mode)
{
  payload = (void *) malloc(sizeof(dms_set_operating_mode_req_msg_v01));
  payload_len = sizeof(dms_set_operating_mode_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_SET_OPERATING_MODE_REQ_V01;

  dms_set_operating_mode_req_msg_v01 *ptr = (dms_set_operating_mode_req_msg_v01 *) payload;
  ptr->operating_mode = operating_mode;
}
/*========================================================
  QMI_DMS_SET_OPERATING_MODE_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_set_operating_mode_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(dms_set_operating_mode_resp_msg_v01));
  payload_len = sizeof(dms_set_operating_mode_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_SET_OPERATING_MODE_RESP_V01;

  dms_set_operating_mode_resp_msg_v01 *ptr = (dms_set_operating_mode_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
/*========================================================
  QMI_DMS_GET_OPERATING_MODE_REQ_V01
========================================================*/
/*========================================================
  QMI_DMS_GET_OPERATING_MODE_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_get_operating_mode_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error, dms_operating_mode_enum_v01 operating_mode)
{
  payload = (void *) malloc(sizeof(dms_get_operating_mode_resp_msg_v01));
  payload_len = sizeof(dms_get_operating_mode_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_GET_OPERATING_MODE_RESP_V01;

  dms_get_operating_mode_resp_msg_v01 *ptr = (dms_get_operating_mode_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
  ptr->operating_mode = operating_mode;
}
/*========================================================
  QMI_DMS_GET_BAND_CAPABILITY_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_get_band_capability_req_msg()
{
  payload = nullptr;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_GET_BAND_CAPABILITY_REQ_V01;
}

/*========================================================
  QMI_DMS_GET_BAND_CAPABILITY_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_get_band_capability_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error, uint64_t band_capability)
{
  payload = (void *) malloc(sizeof(dms_get_band_capability_resp_msg_v01));
  payload_len = sizeof(dms_get_band_capability_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_GET_BAND_CAPABILITY_RESP_V01;

  dms_get_band_capability_resp_msg_v01 *ptr = (dms_get_band_capability_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
  ptr->band_capability = band_capability;
}
/*========================================================
  QMI_DMS_GET_MSISDN_REQ_V01
========================================================*/

/*========================================================
  QMI_DMS_GET_MSISDN_RESP_V01
========================================================*/
char* ril_utf_qmiril_if::update_default_qmi_dms_get_msisdn_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(dms_get_msisdn_resp_msg_v01));
  payload_len = sizeof(dms_get_msisdn_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_GET_BAND_CAPABILITY_RESP_V01;

  dms_get_msisdn_resp_msg_v01 *ptr = (dms_get_msisdn_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
  return (ptr->voice_number);
}
char* ril_utf_qmiril_if::set_qmi_dms_get_msisdn_resp_imsi()
{
  dms_get_msisdn_resp_msg_v01 *ptr = (dms_get_msisdn_resp_msg_v01 *) payload;
  ptr->imsi_valid = TRUE;
  return ptr->imsi;
}
/*========================================================
  QMI_DMS_UIM_GET_ICCID_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_get_iccid_req()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_UIM_GET_ICCID_REQ_V01;
}


/*========================================================
  QMI_DMS_UIM_GET_ICCID_RESP_V01
========================================================*/
char* ril_utf_qmiril_if::update_default_qmi_dms_get_iccid_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(dms_uim_get_iccid_resp_msg_v01));
  payload_len = sizeof(dms_uim_get_iccid_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_UIM_GET_ICCID_RESP_V01;

  dms_uim_get_iccid_resp_msg_v01 *ptr = (dms_uim_get_iccid_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
  return (ptr->uim_id);
}

/*========================================================
  QMI_DMS_GET_DEVICE_SERIAL_NUMBERS_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_get_device_serial_numbers_req()
{
  payload = nullptr;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_GET_DEVICE_SERIAL_NUMBERS_REQ_V01;
}

/*========================================================
  QMI_DMS_GET_DEVICE_SERIAL_NUMBERS_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_deafult_qmi_dms_get_device_serial_numbers_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(dms_get_device_serial_numbers_resp_msg_v01));
  payload_len = sizeof(dms_get_device_serial_numbers_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_GET_DEVICE_SERIAL_NUMBERS_RESP_V01;

  dms_get_device_serial_numbers_resp_msg_v01 *ptr = (dms_get_device_serial_numbers_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
char* ril_utf_qmiril_if::ril_utf_qmiril_if::set_qmi_dms_get_device_serial_numbers_resp_esn()
{
   dms_get_device_serial_numbers_resp_msg_v01 *ptr = (dms_get_device_serial_numbers_resp_msg_v01 *) payload;
   ptr->esn_valid = TRUE;
   return (ptr->esn);
}
char* ril_utf_qmiril_if::set_qmi_dms_get_device_serial_numbers_resp_imei()
{
   dms_get_device_serial_numbers_resp_msg_v01 *ptr = (dms_get_device_serial_numbers_resp_msg_v01 *) payload;
   ptr->imei_valid = TRUE;
   return (ptr->imei);
}
char* ril_utf_qmiril_if::set_qmi_dms_get_device_serial_numbers_resp_meid()
{
   dms_get_device_serial_numbers_resp_msg_v01 *ptr = (dms_get_device_serial_numbers_resp_msg_v01 *) payload;
   ptr->meid_valid = TRUE;
   return (ptr->meid);
}
char* ril_utf_qmiril_if::set_qmi_dms_get_device_serial_numbers_resp_imeisv_svn()
{
   dms_get_device_serial_numbers_resp_msg_v01 *ptr = (dms_get_device_serial_numbers_resp_msg_v01 *) payload;
   ptr->imeisv_svn_valid = TRUE;
   return (ptr->imeisv_svn);
}
/*========================================================
  QMI_DMS_SET_AP_SW_VERSION_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_set_ap_sw_version_req_v01(void)
{
  payload = (void *) malloc(sizeof(dms_set_ap_sw_version_req_msg_v01));
  payload_len = sizeof(dms_set_ap_sw_version_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_SET_AP_SW_VERSION_REQ_V01;
}

/*========================================================
  QMI_DMS_SET_AP_SW_VERSION_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_set_ap_sw_version_resp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(dms_set_ap_sw_version_resp_msg_v01));
  payload_len = sizeof(dms_set_ap_sw_version_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_SET_AP_SW_VERSION_RESP_V01;

  dms_set_ap_sw_version_resp_msg_v01 *ptr = (dms_set_ap_sw_version_resp_msg_v01*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;

}

/*========================================================
  QMI_DMS_GET_SW_VERSION_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_get_sw_version_req_v01(void)
{
  payload = nullptr;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_GET_SW_VERSION_REQ_V01;

}
/*========================================================
  QMI_DMS_GET_SW_VERSION_RESP_V01
========================================================*/
char* ril_utf_qmiril_if::update_default_qmi_dms_get_sw_version_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(dms_get_sw_version_resp_msg_v01));
  payload_len = sizeof(dms_get_sw_version_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_GET_SW_VERSION_RESP_V01;

  dms_get_sw_version_resp_msg_v01 *ptr = (dms_get_sw_version_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
  return (ptr->sw_version);
}

/*========================================================
  QMI_DMS_SET_AP_VERSION_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_set_ap_version_req_v01(void)
{
  payload = (void *) malloc(sizeof(dms_set_ap_sw_version_req_msg_v01));
  payload_len = sizeof(dms_set_ap_sw_version_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_SET_AP_VERSION_REQ_V01;
}

/*========================================================
  QMI_DMS_SET_AP_VERSION_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_set_ap_version_resp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(dms_set_ap_version_resp_msg_v01));
  payload_len = sizeof(dms_set_ap_version_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_SET_AP_VERSION_RESP_V01;

  dms_set_ap_version_resp_msg_v01 *ptr = (dms_set_ap_version_resp_msg_v01*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;

}

/*========================================================
  QMI_DMS_INDICATION_REGISTER_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_indication_register_req_v01(void)
{
  payload = (void *) malloc(sizeof(dms_indication_register_req_msg_v01));
  payload_len = sizeof(dms_indication_register_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_INDICATION_REGISTER_REQ_V01;
  status = not_met;
}
void ril_utf_qmiril_if::set_qmi_dms_ind_req_report_ims_capability(uint8_t ims_capability)
{
  dms_indication_register_req_msg_v01 *ptr = (dms_indication_register_req_msg_v01 *) payload;
  ptr->report_ims_capability_valid = TRUE;
  ptr->report_ims_capability = ims_capability;
}

/*========================================================
  QMI_DMS_INDICATION_REGISTER_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_dms_indication_register_resp_v01(void)
{
  payload = (void *) malloc(sizeof(dms_indication_register_resp_msg_v01));
  payload_len = sizeof(dms_indication_register_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_DMS;
  msg_id = QMI_DMS_INDICATION_REGISTER_RESP_V01;
}
void ril_utf_qmiril_if::set_qmi_dms_indication_register_resp_msg_v01_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  dms_indication_register_resp_msg_v01 *ptr =
    (dms_indication_register_resp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}
/*========================================================
  QMI_WMS_RAW_SEND_REQ_V01
========================================================*/
#ifndef RIL_UTF_IOE
void ril_utf_qmiril_if::update_default_qmi_wms_raw_send_req_msg
(char* smsc_address, char *message, wms_message_format_enum_v01 format)
{
  int byte_array_pos = 0;

  payload = (void *) malloc(sizeof(wms_raw_send_req_msg_v01));
  payload_len = sizeof(wms_raw_send_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_RAW_SEND_REQ_V01;

  wms_raw_send_req_msg_v01 *ptr = (wms_raw_send_req_msg_v01 *) payload;

#ifndef UTF_TARGET_BUILD
  qcril_sms_hex_to_byte( smsc_address,
#ifdef QMI_RIL_HAL_UTF
                     (byte *)ptr->raw_message_data.raw_message,
#else
                     ptr->raw_message_data.raw_message,
#endif
                     strlen(smsc_address) );

  byte_array_pos = strlen(smsc_address) / 2;

  qcril_sms_hex_to_byte( message,
#ifdef QMI_RIL_HAL_UTF
                   (byte *)ptr->raw_message_data.raw_message + byte_array_pos,
#else
                   ptr->raw_message_data.raw_message + byte_array_pos,
#endif
                   strlen(message) );
#endif

  ptr->raw_message_data.format = format;
  ptr->raw_message_data.raw_message_len = strlen(message) + 1;
}

void ril_utf_qmiril_if::update_default_qmi_wms_raw_send_req_msg_3gpp2(RIL_CDMA_SMS_Message *cdma_sms_msg, wms_message_format_enum_v01 format, uint8_t sms_on_ims)
{
  int byte_array_pos = 0;
  qcril_sms_OTA_message_type   ota_data;
  qcril_sms_tl_message_type    tl_msg;
  boolean status = FALSE;

  if(cdma_sms_msg != NULL)
  {
    (void) memset((uint8 *)&ota_data.data, 0, sizeof(ota_data.data));
    (void) memset(&tl_msg, 0, sizeof(tl_msg));
    ota_data.data_len = 0;

    payload = (void *) malloc(1*sizeof(wms_raw_send_req_msg_v01));
    payload_len = sizeof(wms_raw_send_req_msg_v01);
    memset(payload, 0x00, payload_len);
    payload_type = ril_utf_qmi_request;
    service_id = QMI_WMS;
    msg_id = QMI_WMS_RAW_SEND_REQ_V01;

    wms_raw_send_req_msg_v01 *ptr = (wms_raw_send_req_msg_v01 *) payload;

#ifndef UTF_TARGET_BUILD
    qcril_sms_convert_ril_to_tl(cdma_sms_msg, &tl_msg, sms_on_ims, TRUE);
    status = qcril_sms_convert_tl_to_qmi(&tl_msg,
                                       cdma_sms_msg->uBearerDataLen,
                                       cdma_sms_msg->aBearerData,
                                       &ota_data);
    if (status)
    {
        ptr->raw_message_data.raw_message_len = ota_data.data_len;
        (void) memcpy((uint8 *)&ptr->raw_message_data.raw_message, (uint8 *)&ota_data.data, MIN(WMS_MESSAGE_LENGTH_MAX_V01, ota_data.data_len));
    }
    ptr->raw_message_data.format = format;
#endif
  }
}

#else
wms_send_raw_message_data_type_v01* ril_utf_qmiril_if::update_default_qmi_wms_raw_send_req_msg()
{
  payload = (void *) malloc(sizeof(wms_raw_send_req_msg_v01));
  payload_len = sizeof(wms_raw_send_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_RAW_SEND_REQ_V01;

  wms_raw_send_req_msg_v01 *ptr = (wms_raw_send_req_msg_v01 *) payload;
  return &(ptr->raw_message_data);
}
#endif
void ril_utf_qmiril_if::set_qmi_wms_raw_send_req_link_timer(uint8_t link_timer)
{
  wms_raw_send_req_msg_v01 *ptr = (wms_raw_send_req_msg_v01 *) payload;
  ptr->link_timer_valid = TRUE;
  ptr->link_timer = link_timer;
}
void ril_utf_qmiril_if::set_qmi_wms_raw_send_req_sms_on_ims(uint8_t sms_on_ims)
{
  wms_raw_send_req_msg_v01 *ptr = (wms_raw_send_req_msg_v01 *) payload;
  ptr->sms_on_ims_valid = TRUE;
  ptr->sms_on_ims = sms_on_ims;
}
void ril_utf_qmiril_if::set_qmi_wms_raw_send_req_retry_message(wms_retry_message_enum_v01 retry_message)
{
  wms_raw_send_req_msg_v01 *ptr = (wms_raw_send_req_msg_v01 *) payload;
  ptr->retry_message_valid = TRUE;
  ptr->retry_message = retry_message;
}
void ril_utf_qmiril_if::set_qmi_wms_raw_send_req_retry_message_id(uint32_t retry_message_id)
{
  wms_raw_send_req_msg_v01 *ptr = (wms_raw_send_req_msg_v01 *) payload;
  ptr->retry_message_id_valid = TRUE;
  ptr->retry_message_id = retry_message_id;
}

/*========================================================
  QMI_WMS_RAW_SEND_RESP_V01
========================================================*/
wms_raw_send_resp_msg_v01* ril_utf_qmiril_if::update_default_qmi_wms_raw_send_resp_msg(
        qmi_result_type_v01 result, qmi_error_type_v01 error, uint16_t message_id)
{
  payload = (void *) malloc(sizeof(wms_raw_send_resp_msg_v01));
  payload_len = sizeof(wms_raw_send_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_RAW_SEND_RESP_V01;

  wms_raw_send_resp_msg_v01 *ptr = (wms_raw_send_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
  ptr->message_id = message_id;
  return ptr;
}

void ril_utf_qmiril_if::set_call_control_flag_in_raw_send_resp_mag(uint8_t flag)
{
  wms_raw_send_resp_msg_v01 *ptr = (wms_raw_send_resp_msg_v01 *) payload;
  ptr->call_control_modified_info_valid = flag;
}

void ril_utf_qmiril_if::set_call_control_alpha_text_in_raw_send_resp_mag(uint8_t text)
{
  wms_raw_send_resp_msg_v01 *ptr = (wms_raw_send_resp_msg_v01 *) payload;
  uint32_t i = 0;

  for(i=0;i<ptr->call_control_modified_info.alpha_id_len;i++){}

  if( i < WMS_ALPHA_ID_LENGTH_MAX_V01 )
  {
    ptr->call_control_modified_info.alpha_id_len += 1;
    ptr->call_control_modified_info.alpha_id[i] = text;
  }
}

void ril_utf_qmiril_if::set_qmi_wms_raw_send_resp_cause_code(wms_tl_cause_code_enum_v01 cause_code)
{
  wms_raw_send_resp_msg_v01 *ptr = (wms_raw_send_resp_msg_v01 *) payload;
  ptr->cause_code_valid = TRUE;
  ptr->cause_code = cause_code;
}
wms_gw_cause_info_type_v01* ril_utf_qmiril_if::set_qmi_wms_raw_send_resp_gw_cause_info()
{
  wms_raw_send_resp_msg_v01 *ptr = (wms_raw_send_resp_msg_v01 *) payload;
  ptr->gw_cause_info_valid = TRUE;
  return &(ptr->gw_cause_info);
}
void ril_utf_qmiril_if::set_qmi_wms_raw_send_resp_error_class(wms_error_class_send_enum_v01 error_class)
{
  wms_raw_send_resp_msg_v01 *ptr = (wms_raw_send_resp_msg_v01 *) payload;
  ptr->error_class_valid = TRUE;
  ptr->error_class = error_class;
}
void ril_utf_qmiril_if::set_qmi_wms_raw_send_resp_message_delivery_failure_type(wms_message_delivery_failure_type_enum_v01 message_delivery_failure_type)
{
  wms_raw_send_resp_msg_v01 *ptr = (wms_raw_send_resp_msg_v01 *) payload;
  ptr->message_delivery_failure_type_valid = TRUE;
  ptr->message_delivery_failure_type = message_delivery_failure_type;
}
/*========================================================
  QMI_WMS_GET_SMSC_ADDRESS_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_get_smsc_address_req()
{
    payload = NULL;
    payload_len = 0;
    payload_type = ril_utf_qmi_request;
    service_id = QMI_WMS;
    msg_id = QMI_WMS_GET_SMSC_ADDRESS_REQ_V01;
}

/*========================================================
  QMI_WMS_GET_SMSC_ADDRESS_RESP_V01
========================================================*/
wms_smsc_address_type_v01* ril_utf_qmiril_if::update_default_qmi_wms_get_smsc_address_resp(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(wms_get_smsc_address_resp_msg_v01));
  payload_len = sizeof(wms_get_smsc_address_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_GET_SMSC_ADDRESS_RESP_V01;

  wms_get_smsc_address_resp_msg_v01 *ptr = (wms_get_smsc_address_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
  return &(ptr->smsc_address);
}

/*========================================================
  QMI_WMS_SET_SMSC_ADDRESS_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_deafult_qmi_wms_set_smsc_address_req_msg(char *val)
{
  payload = (void *) malloc(sizeof(wms_set_smsc_address_req_msg_v01));
  payload_len = sizeof(wms_set_smsc_address_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_SET_SMSC_ADDRESS_REQ_V01;

  wms_set_smsc_address_req_msg_v01 *ptr = (wms_set_smsc_address_req_msg_v01 *) payload;
  strlcpy (ptr->smsc_address_digits, val, strlen (val) + 1);
}
void ril_utf_qmiril_if::set_qmi_wms_set_smsc_address_req_msg_smsc_address_type(char *val)
{
  wms_set_smsc_address_req_msg_v01 *ptr = (wms_set_smsc_address_req_msg_v01 *) payload;
  ptr->smsc_address_type_valid = TRUE;
  strlcpy (ptr->smsc_address_type, val, strlen (val) + 1);
}

/*========================================================
  QMI_WMS_SET_SMSC_ADDRESS_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_set_smsc_address_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(wms_set_smsc_address_resp_msg_v01));
  payload_len = sizeof(wms_set_smsc_address_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_SET_SMSC_ADDRESS_RESP_V01;

  wms_set_smsc_address_resp_msg_v01 *ptr = (wms_set_smsc_address_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
/*========================================================
  QMI_WMS_SET_MEMORY_STATUS_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_set_memory_status_req_msg(uint8_t memory_available)
{
  payload = (void *) malloc(sizeof(wms_set_memory_status_req_msg_v01));
  payload_len = sizeof(wms_set_memory_status_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_SET_MEMORY_STATUS_REQ_V01;
  wms_set_memory_status_req_msg_v01 *ptr = (wms_set_memory_status_req_msg_v01 *) payload;
  ptr->memory_available = memory_available;
}
/*========================================================
  QMI_WMS_SET_MEMORY_STATUS_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_set_memory_status_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(wms_set_memory_status_resp_msg_v01));
  payload_len = sizeof(wms_set_memory_status_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_SET_MEMORY_STATUS_RESP_V01;

  wms_set_memory_status_resp_msg_v01 *ptr = (wms_set_memory_status_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
/*========================================================
  QMI_WMS_SEND_ACK_REQ_V01
========================================================*/
wms_send_ack_req_msg_v01* ril_utf_qmiril_if::update_default_qmi_wms_send_ack_req_msg()
{
  payload = (void *) malloc(sizeof(wms_send_ack_req_msg_v01));
  payload_len = sizeof(wms_send_ack_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_SEND_ACK_REQ_V01;

  return (wms_send_ack_req_msg_v01 *) payload;
}
wms_3gpp_failure_information_type_v01* ril_utf_qmiril_if::set_qmi_wms_send_ack_req_wms_3gpp_failure_information()
{
  wms_send_ack_req_msg_v01 *ptr = (wms_send_ack_req_msg_v01 *) payload;
  ptr->wms_3gpp_failure_information_valid = TRUE;
  return &(ptr->wms_3gpp_failure_information);
}
void ril_utf_qmiril_if::set_qmi_wms_send_ack_req_sms_on_ims(uint8_t sms_on_ims)
{
  wms_send_ack_req_msg_v01 *ptr = (wms_send_ack_req_msg_v01 *) payload;
  ptr->sms_on_ims_valid = TRUE;
  ptr->sms_on_ims = sms_on_ims;
}
wms_3gpp2_failure_information_type_v01* ril_utf_qmiril_if::set_qmi_wms_send_ack_req_wms_3gpp2_failure_information()
{
  wms_send_ack_req_msg_v01 *ptr = (wms_send_ack_req_msg_v01 *) payload;
  ptr->wms_3gpp2_failure_information_valid = TRUE;
  return &(ptr->wms_3gpp2_failure_information);
}

/*========================================================
  QMI_WMS_SERVICE_READY_IND_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_service_ready_ind(wms_service_ready_status_enum_v01 ready_status)
{
  payload = malloc(sizeof(wms_service_ready_ind_msg_v01));
  payload_len = sizeof(wms_service_ready_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_SERVICE_READY_IND_V01;

  wms_service_ready_ind_msg_v01 *ptr = (wms_service_ready_ind_msg_v01*) payload;
  ptr->ready_status = ready_status;
}

/*========================================================
  QMI_WMS_SEND_ACK_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_send_ack_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(wms_send_ack_resp_msg_v01));
  payload_len = sizeof(wms_send_ack_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_SEND_ACK_RESP_V01;

  wms_send_ack_resp_msg_v01 *ptr = (wms_send_ack_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
/*========================================================
QMI_WMS_BIND_SUBSCRIPTION_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_bind_subscription_req(wms_subscription_type_enum_v01 subs_type)
{
  payload = (void *) malloc(sizeof(wms_bind_subscription_req_msg_v01));
  payload_len = sizeof(wms_bind_subscription_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_BIND_SUBSCRIPTION_REQ_V01;

  wms_bind_subscription_req_msg_v01 *ptr = (wms_bind_subscription_req_msg_v01*) payload;
  ptr->subs_type = subs_type;
}
/*========================================================
QMI_WMS_BIND_SUBSCRIPTION_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_bind_subscription_resp(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(wms_bind_subscription_resp_msg_v01));
  payload_len = sizeof(wms_bind_subscription_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_BIND_SUBSCRIPTION_RESP_V01;

  wms_bind_subscription_resp_msg_v01 *ptr = (wms_bind_subscription_resp_msg_v01*) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
/*========================================================
  QMI_WMS_DELETE_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_delete_req_msg(wms_storage_type_enum_v01 storage_type)
{
  payload = (void *) malloc(sizeof(wms_delete_req_msg_v01));
  payload_len = sizeof(wms_delete_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_DELETE_REQ_V01;

  wms_delete_req_msg_v01 *ptr = (wms_delete_req_msg_v01 *) payload;
  ptr->storage_type = storage_type;
}
void ril_utf_qmiril_if::set_qmi_wms_delete_req_index(uint32_t index)
{
  wms_delete_req_msg_v01 *ptr = (wms_delete_req_msg_v01 *) payload;
  ptr->index_valid = TRUE;
  ptr->index = index;
}
void ril_utf_qmiril_if::set_qmi_wms_delete_req_message_mode(wms_message_mode_enum_v01 message_mode)
{
  wms_delete_req_msg_v01 *ptr = (wms_delete_req_msg_v01 *) payload;
  ptr->message_mode_valid = TRUE;
  ptr->message_mode = message_mode;
}
/*========================================================
  QMI_WMS_DELETE_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_delete_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(wms_delete_resp_msg_v01));
  payload_len = sizeof(wms_delete_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_DELETE_RESP_V01;

  wms_delete_resp_msg_v01 *ptr = (wms_delete_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
/*========================================================
  QMI_WMS_GET_BROADCAST_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_get_broadcast_config_req_msg(wms_message_mode_enum_v01 message_mode)
{
  payload = (void *) malloc(sizeof(wms_get_broadcast_config_req_msg_v01));
  payload_len = sizeof(wms_get_broadcast_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_GET_BROADCAST_CONFIG_REQ_V01;

  wms_get_broadcast_config_req_msg_v01 *ptr = (wms_get_broadcast_config_req_msg_v01 *) payload;
  ptr->message_mode = message_mode;
}
/*========================================================
  QMI_WMS_GET_BROADCAST_CONFIG_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_get_broadcast_config_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(wms_get_broadcast_config_resp_msg_v01));
  payload_len = sizeof(wms_get_broadcast_config_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_GET_BROADCAST_CONFIG_RESP_V01;

  wms_get_broadcast_config_resp_msg_v01 *ptr = (wms_get_broadcast_config_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

void ril_utf_qmiril_if::set_wms_get_broadcast_config_resp(wms_get_broadcast_config_resp_msg_v01 &in)
{
  wms_get_broadcast_config_resp_msg_v01 *ptr = (wms_get_broadcast_config_resp_msg_v01 *) payload;
  if(in.wms_3gpp_broadcast_info_valid)
  {
    ptr->wms_3gpp_broadcast_info_valid = 1;
    ptr->wms_3gpp_broadcast_info.wms_3gpp_broadcast_config_info_len =
                            in.wms_3gpp_broadcast_info.wms_3gpp_broadcast_config_info_len;
    for(int i=0; i<in.wms_3gpp_broadcast_info.wms_3gpp_broadcast_config_info_len; i++)
    {
        ptr->wms_3gpp_broadcast_info.wms_3gpp_broadcast_config_info[i].from_service_id =
            in.wms_3gpp_broadcast_info.wms_3gpp_broadcast_config_info[i].from_service_id;
        ptr->wms_3gpp_broadcast_info.wms_3gpp_broadcast_config_info[i].to_service_id =
            in.wms_3gpp_broadcast_info.wms_3gpp_broadcast_config_info[i].to_service_id;
        ptr->wms_3gpp_broadcast_info.wms_3gpp_broadcast_config_info[i].selected =
            in.wms_3gpp_broadcast_info.wms_3gpp_broadcast_config_info[i].selected;
    }
  }
  if(in.wms_3gpp2_broadcast_info_valid)
  {
    ptr->wms_3gpp2_broadcast_info_valid = 1;
    ptr->wms_3gpp2_broadcast_info.wms_3gpp2_broadcast_config_info_len =
                            in.wms_3gpp2_broadcast_info.wms_3gpp2_broadcast_config_info_len;
    for(int i=0; i<in.wms_3gpp2_broadcast_info.wms_3gpp2_broadcast_config_info_len; i++)
    {
        ptr->wms_3gpp2_broadcast_info.wms_3gpp2_broadcast_config_info[i].language =
            in.wms_3gpp2_broadcast_info.wms_3gpp2_broadcast_config_info[i].language;
        ptr->wms_3gpp2_broadcast_info.wms_3gpp2_broadcast_config_info[i].selected =
            in.wms_3gpp2_broadcast_info.wms_3gpp2_broadcast_config_info[i].selected;
        ptr->wms_3gpp2_broadcast_info.wms_3gpp2_broadcast_config_info[i].service_category =
            in.wms_3gpp2_broadcast_info.wms_3gpp2_broadcast_config_info[i].service_category;

    }
  }
}

/*========================================================
  QMI_WMS_SET_BROADCAST_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_deafult_qmi_wms_set_broadcast_config_req_msg(wms_message_mode_enum_v01 message_mode)
{
  payload = (void *) malloc(sizeof(wms_set_broadcast_config_req_msg_v01));
  payload_len = sizeof(wms_set_broadcast_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_SET_BROADCAST_CONFIG_REQ_V01;
  wms_set_broadcast_config_req_msg_v01 *ptr = (wms_set_broadcast_config_req_msg_v01 *) payload;
  ptr->message_mode = message_mode;
}

wms_3gpp_broadcast_config_info_type_v01* ril_utf_qmiril_if::set_qmi_wms_set_broadcast_config_req_wms_3gpp_broadcast_config_info(uint32_t wms_3gpp_broadcast_config_info_len)
{
  wms_set_broadcast_config_req_msg_v01 *ptr = (wms_set_broadcast_config_req_msg_v01 *) payload;
  ptr->wms_3gpp_broadcast_config_info_valid = TRUE;
  ptr->wms_3gpp_broadcast_config_info_len = wms_3gpp_broadcast_config_info_len;
  return (ptr->wms_3gpp_broadcast_config_info);
}
wms_3gpp2_broadcast_config_info_type_v01* ril_utf_qmiril_if::set_qmi_wms_set_broadcast_config_req_wms_3gpp2_broadcast_config_info(uint32_t wms_3gpp2_broadcast_config_info_len)
{
  wms_set_broadcast_config_req_msg_v01 *ptr = (wms_set_broadcast_config_req_msg_v01 *) payload;
  ptr->wms_3gpp2_broadcast_config_info_valid = TRUE;
  ptr->wms_3gpp2_broadcast_config_info_len = wms_3gpp2_broadcast_config_info_len;
  return (ptr->wms_3gpp2_broadcast_config_info);
}

void ril_utf_qmiril_if::set_wms_set_broadcast_config_req(wms_set_broadcast_config_req_msg_v01 &in)
{
  wms_set_broadcast_config_req_msg_v01 *ptr = (wms_set_broadcast_config_req_msg_v01 *) payload;
  if(in.wms_3gpp_broadcast_config_info_valid)
  {
    ptr->wms_3gpp_broadcast_config_info_valid = 1;
    ptr->wms_3gpp_broadcast_config_info_len =
                            in.wms_3gpp_broadcast_config_info_len;
    for(int i=0; i<in.wms_3gpp_broadcast_config_info_len; i++)
    {
        ptr->wms_3gpp_broadcast_config_info[i].from_service_id =
            in.wms_3gpp_broadcast_config_info[i].from_service_id;
        ptr->wms_3gpp_broadcast_config_info[i].to_service_id =
            in.wms_3gpp_broadcast_config_info[i].to_service_id;
        ptr->wms_3gpp_broadcast_config_info[i].selected =
            in.wms_3gpp_broadcast_config_info[i].selected;
    }
  }
  if(in.wms_3gpp2_broadcast_config_info_valid)
  {
    ptr->wms_3gpp2_broadcast_config_info_valid = 1;
    ptr->wms_3gpp2_broadcast_config_info_len =
                            in.wms_3gpp2_broadcast_config_info_len;
    for(int i=0; i<in.wms_3gpp2_broadcast_config_info_len; i++)
    {
        ptr->wms_3gpp2_broadcast_config_info[i].language =
            in.wms_3gpp2_broadcast_config_info[i].language;
        ptr->wms_3gpp2_broadcast_config_info[i].selected =
            in.wms_3gpp2_broadcast_config_info[i].selected;
        ptr->wms_3gpp2_broadcast_config_info[i].service_category =
            in.wms_3gpp2_broadcast_config_info[i].service_category;
    }
  }
}

/*========================================================
  QMI_WMS_SET_BROADCAST_CONFIG_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_set_broadcast_config_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(wms_set_broadcast_config_resp_msg_v01));
  payload_len = sizeof(wms_set_broadcast_config_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_SET_BROADCAST_CONFIG_RESP_V01;

  wms_set_broadcast_config_resp_msg_v01 *ptr = (wms_set_broadcast_config_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
/*========================================================
  QMI_WMS_SET_BROADCAST_ACTIVATION_REQ_V01
========================================================*/
wms_broadcast_activation_info_type_v01* ril_utf_qmiril_if::update_default_qmi_wms_set_broadcast_activation_req_msg()
{
  payload = (void *) malloc(sizeof(wms_set_broadcast_activation_req_msg_v01));
  payload_len = sizeof(wms_set_broadcast_activation_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_SET_BROADCAST_ACTIVATION_REQ_V01;

  wms_set_broadcast_activation_req_msg_v01 *ptr = (wms_set_broadcast_activation_req_msg_v01 *) payload;
  return &(ptr->broadcast_activation_info);
}

void ril_utf_qmiril_if::set_qmi_wms_set_activation_req_msg_activate_all(uint8_t val)
{
     wms_set_broadcast_activation_req_msg_v01 *ptr = (wms_set_broadcast_activation_req_msg_v01 *) payload;
   ptr->activate_all_valid = TRUE;
   ptr->activate_all = val;
}
/*========================================================
  QMI_WMS_SET_BROADCAST_ACTIVATION_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_qmi_wms_set_broadcast_activation_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(wms_set_broadcast_activation_resp_msg_v01));
  payload_len = sizeof(wms_set_broadcast_activation_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_SET_BROADCAST_ACTIVATION_RESP_V01;

  wms_set_broadcast_activation_resp_msg_v01 *ptr = (wms_set_broadcast_activation_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
/*========================================================
  QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ_V01
========================================================*/

/*========================================================
  QMI_WMS_GET_TRANSPORT_LAYER_INFO_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_get_transport_layer_info_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(wms_get_transport_layer_resp_msg_v01));
  payload_len = sizeof(wms_get_transport_layer_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_GET_TRANSPORT_LAYER_INFO_RESP_V01;

  wms_get_transport_layer_resp_msg_v01 *ptr = (wms_get_transport_layer_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_wms_get_transport_layer_info_resp_registered_ind(uint8_t registered_ind)
{
   wms_get_transport_layer_resp_msg_v01 *ptr = (wms_get_transport_layer_resp_msg_v01 *) payload;
   ptr->registered_ind_valid = TRUE;
   ptr->registered_ind = registered_ind;
}
/*========================================================
  QMI_WMS_SET_PRIMARY_CLIENT_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_set_primary_client_req_msg(uint8_t primary_client)
{
  payload = (void *) malloc(sizeof(wms_set_primary_client_req_msg_v01));
  payload_len = sizeof(wms_set_primary_client_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_GET_TRANSPORT_LAYER_INFO_RESP_V01;

  wms_set_primary_client_req_msg_v01 *ptr = (wms_set_primary_client_req_msg_v01 *) payload;
  ptr->primary_client = primary_client;
}
/*========================================================
  QMI_WMS_SET_PRIMARY_CLIENT_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_set_primary_client_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(wms_set_primary_client_resp_msg_v01));
  payload_len = sizeof(wms_set_primary_client_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_SET_PRIMARY_CLIENT_RESP_V01;

  wms_set_primary_client_resp_msg_v01 *ptr = (wms_set_primary_client_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
/*========================================================
  QMI_WMS_SET_ROUTES_REQ_V01
========================================================*/
wms_set_route_list_tuple_type_v01* ril_utf_qmiril_if::update_default_qmi_wms_set_routes_req_msg(uint32_t route_list_tuple_len)
{
  payload = (void *) malloc(sizeof(wms_set_routes_req_msg_v01));
  payload_len = sizeof(wms_set_routes_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_SET_ROUTES_REQ_V01;

  wms_set_routes_req_msg_v01 *ptr = (wms_set_routes_req_msg_v01 *) payload;
  ptr->route_list_tuple_len = route_list_tuple_len;
  return (ptr->route_list_tuple);
}
void ril_utf_qmiril_if::set_qmi_wms_set_routes_req_transfer_ind(wms_transfer_ind_enum_v01 transfer_ind)
{
  wms_set_routes_req_msg_v01 *ptr = (wms_set_routes_req_msg_v01 *) payload;
  ptr->transfer_ind_valid = TRUE;
  ptr->transfer_ind = transfer_ind;
}
/*========================================================
  QMI_WMS_SET_ROUTES_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_set_routes_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(wms_set_routes_resp_msg_v01));
  payload_len = sizeof(wms_set_routes_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_SET_ROUTES_RESP_V01;

  wms_set_routes_resp_msg_v01 *ptr = (wms_set_routes_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

/*========================================================
  QMI_WMS_SET_EVENT_REPORT_REQ_V01
========================================================*/

void ril_utf_qmiril_if::update_default_qmi_wms_set_event_report_req_msg()
{
  payload = (void *) malloc(sizeof(wms_set_event_report_req_msg_v01));
  payload_len = sizeof(wms_set_event_report_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_SET_EVENT_REPORT_REQ_V01;
}
void ril_utf_qmiril_if::set_qmi_wms_set_event_report_report_mt_message(uint8_t report_mt_message)
{
  wms_set_event_report_req_msg_v01 *ptr = (wms_set_event_report_req_msg_v01 *) payload;
  ptr->report_mt_message_valid = TRUE;
  ptr->report_mt_message = report_mt_message;
}
void ril_utf_qmiril_if::set_qmi_wms_set_event_report_report_mwi_message(uint8_t report_mwi_message)
{
  wms_set_event_report_req_msg_v01 *ptr = (wms_set_event_report_req_msg_v01 *) payload;
  ptr->report_mwi_message_valid = TRUE;
  ptr->report_mwi_message = report_mwi_message;
}
/*========================================================
  QMI_WMS_SET_EVENT_REPORT_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_set_event_report_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(wms_set_event_report_resp_msg_v01));
  payload_len = sizeof(wms_set_event_report_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_SET_EVENT_REPORT_RESP_V01;

  wms_set_event_report_resp_msg_v01 *ptr = (wms_set_event_report_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

/*========================================================
  QMI_WMS_INDICATION_REGISTER_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_indication_register_req_msg()
{
  payload = (void *) malloc(sizeof(wms_indication_register_req_msg_v01));
  payload_len = sizeof(wms_indication_register_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_INDICATION_REGISTER_REQ_V01;
}
void ril_utf_qmiril_if::set_qmi_wms_indication_register_req_reg_transport_layer_info_events(uint8_t reg_transport_layer_info_events)
{
  wms_indication_register_req_msg_v01 *ptr = (wms_indication_register_req_msg_v01 *) payload;
  ptr->reg_transport_layer_info_events_valid = TRUE;
  ptr->reg_transport_layer_info_events = reg_transport_layer_info_events;
}
/*========================================================
  QMI_WMS_INDICATION_REGISTER_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_indication_register_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(wms_indication_register_resp_msg_v01));
  payload_len = sizeof(wms_indication_register_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_INDICATION_REGISTER_RESP_V01;

  wms_indication_register_resp_msg_v01 *ptr = (wms_indication_register_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
/*========================================================
  QMI_WMS_RAW_WRITE_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_raw_write_req_msg
(char* smsc_address, char *message, wms_message_format_enum_v01 format, wms_message_tag_type_enum_v01 tag_type)
{
  int byte_array_pos = 0;

  payload = (void *) malloc(sizeof(wms_raw_write_req_msg_v01));
  payload_len = sizeof(wms_raw_write_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_RAW_WRITE_REQ_V01;

  wms_raw_write_req_msg_v01 *ptr = (wms_raw_write_req_msg_v01 *) payload;
  ptr->tag_type_valid = 1;
  ptr->tag_type = tag_type;
  ptr->raw_message_write_data.format = format;
  ptr->raw_message_write_data.storage_type = WMS_STORAGE_TYPE_UIM_V01;

#ifndef UTF_TARGET_BUILD
  qcril_sms_hex_to_byte( smsc_address,
#ifdef QMI_RIL_HAL_UTF
                     (byte *)ptr->raw_message_write_data.raw_message,
#else
                     ptr->raw_message_write_data.raw_message,
#endif
                     strlen(smsc_address) );

  byte_array_pos = strlen(smsc_address) / 2;

  qcril_sms_hex_to_byte( message,
#ifdef QMI_RIL_HAL_UTF
                   (byte *)ptr->raw_message_write_data.raw_message + byte_array_pos,
#else
                   ptr->raw_message_write_data.raw_message + byte_array_pos,
#endif
                   strlen(message) );
#endif

  ptr->raw_message_write_data.format = format;
  ptr->raw_message_write_data.raw_message_len = strlen(message) + 1;
}

/*========================================================
  QMI_WMS_RAW_WRITE_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_raw_write_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error, uint32_t storage_index)
{
  payload = (void *) malloc(sizeof(wms_raw_write_resp_msg_v01));
  payload_len = sizeof(wms_raw_write_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_RAW_WRITE_RESP_V01;

  wms_raw_write_resp_msg_v01 *ptr = (wms_raw_write_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
  ptr->storage_index = storage_index;
}
/*========================================================
  QMI_WMS_MODIFY_TAG_REQ_V01
========================================================*/
wms_message_tag_type_v01* ril_utf_qmiril_if::update_default_qmi_wms_modify_tag_req_msg()
{
  payload = (void *) malloc(sizeof(wms_modify_tag_req_msg_v01));
  payload_len = sizeof(wms_modify_tag_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_MODIFY_TAG_REQ_V01;

  wms_modify_tag_req_msg_v01 *ptr = (wms_modify_tag_req_msg_v01 *) payload;
  return &(ptr->wms_message_tag);
}
void ril_utf_qmiril_if::set_qmi_wms_modify_tag_req_message_mode(wms_message_mode_enum_v01 message_mode)
{
  wms_modify_tag_req_msg_v01 *ptr = (wms_modify_tag_req_msg_v01 *) payload;
  ptr->message_mode_valid = TRUE;
  ptr->message_mode = message_mode;
}
/*========================================================
  QMI_WMS_MODIFY_TAG_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_modify_tag_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(wms_modify_tag_resp_msg_v01));
  payload_len = sizeof(wms_modify_tag_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_MODIFY_TAG_RESP_V01;

  wms_modify_tag_resp_msg_v01 *ptr = (wms_modify_tag_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
/*========================================================
  QMI_WMS_RAW_READ_REQ_V01
========================================================*/
wms_message_memory_storage_identification_type_v01* ril_utf_qmiril_if::update_default_qmi_wms_raw_read_req_msg()
{
  payload = (void *) malloc(sizeof(wms_raw_read_req_msg_v01));
  payload_len = sizeof(wms_raw_read_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_RAW_READ_REQ_V01;

  wms_raw_read_req_msg_v01 *ptr = (wms_raw_read_req_msg_v01 *) payload;
  return &(ptr->message_memory_storage_identification);
}
void ril_utf_qmiril_if::set_qmi_wms_raw_read_req_message_mode(wms_message_mode_enum_v01 message_mode)
{
  wms_raw_read_req_msg_v01 *ptr = (wms_raw_read_req_msg_v01 *) payload;
  ptr->message_mode_valid = TRUE;
  ptr->message_mode = message_mode;
}
/*========================================================
  QMI_WMS_RAW_READ_RESP_V01
========================================================*/
wms_read_raw_message_data_type_v01* ril_utf_qmiril_if::update_default_qmi_wms_raw_read_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(wms_raw_read_resp_msg_v01));
  payload_len = sizeof(wms_raw_read_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_RAW_RESP_MSG_V01;
  wms_raw_read_resp_msg_v01 *ptr = (wms_raw_read_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
  return &(ptr->raw_message_data);
}

/*========================================================
  QMI_WMS_EVENT_REPORT_IND_V01
========================================================*/

void ril_utf_qmiril_if::update_default_qmi_wms_event_report_ind_msg()
{
  payload = (void *) malloc(sizeof(wms_event_report_ind_msg_v01));
  payload_len = sizeof(wms_event_report_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_EVENT_REPORT_IND_V01;
}


void ril_utf_qmiril_if::set_sms_rat(wms_bc_rat_info_enum_v01 rat)
{
  wms_event_report_ind_msg_v01 *ptr = (wms_event_report_ind_msg_v01 *) payload;
  ptr->bc_sms_rat_info_valid = TRUE;
  ptr->bc_sms_rat_info = rat;
}

void ril_utf_qmiril_if::set_warning_area_coordinate(unsigned char* wac, size_t len)
{
  wms_event_report_ind_msg_v01 *ptr = (wms_event_report_ind_msg_v01 *) payload;
  ptr->warning_area_valid = TRUE;
  ptr->warning_area_len = len;
  memset(ptr->warning_area, 0, len);
  memcpy(ptr->warning_area, wac, len);
}
void ril_utf_qmiril_if::set_call_control_flag_in_event_report_ind_mag(uint8_t flag)
{
  wms_event_report_ind_msg_v01 *ptr = (wms_event_report_ind_msg_v01 *) payload;
  ptr->call_control_info_valid = flag;
}

void ril_utf_qmiril_if::set_call_control_alpha_text_in_event_report_ind_mag(uint8_t text)
{
  wms_event_report_ind_msg_v01 *ptr = (wms_event_report_ind_msg_v01 *) payload;
  uint32_t i = 0;

  for(i=0;i<ptr->call_control_info.alpha_id_len;i++){}

  if( i < WMS_ALPHA_ID_LENGTH_MAX_V01 )
  {
    ptr->call_control_info.alpha_id_len += 1;
    ptr->call_control_info.alpha_id[i] = text;
  }
}
#ifndef RIL_UTF_IOE
void ril_utf_qmiril_if::set_qmi_wms_event_report_ind_transfer_route_mt_message
( wms_ack_indicator_enum_v01 ack_indicator, uint32_t transaction_id, wms_message_format_enum_v01 format, char* message)
{
  wms_event_report_ind_msg_v01 *ptr = (wms_event_report_ind_msg_v01 *) payload;
  ptr->transfer_route_mt_message_valid = TRUE;
  ptr->transfer_route_mt_message.ack_indicator = ack_indicator;
  ptr->transfer_route_mt_message.transaction_id = transaction_id;
  ptr->transfer_route_mt_message.format = format;

#ifndef UTF_TARGET_BUILD
  qcril_sms_hex_to_byte( message,
#ifdef QMI_RIL_HAL_UTF
                     (byte*)ptr->transfer_route_mt_message.data,
#else
                     ptr->transfer_route_mt_message.data,
#endif
                     strlen(message) );
#endif

  ptr->transfer_route_mt_message.data_len = strlen(message)/ 2;



}
#else
wms_transfer_route_mt_message_type_v01* ril_utf_qmiril_if::set_qmi_wms_event_report_ind_transfer_route_mt_message()
{
  wms_event_report_ind_msg_v01 *ptr = (wms_event_report_ind_msg_v01 *) payload;
  ptr->transfer_route_mt_message_valid = TRUE;
  return &(ptr->transfer_route_mt_message);
}
#endif

void ril_utf_qmiril_if::set_qmi_wms_event_report_ind_transfer_route_bc_message(
  wms_message_format_enum_v01 format, char* message)
{
  wms_event_report_ind_msg_v01 *ptr = (wms_event_report_ind_msg_v01 *) payload;
  ptr->transfer_route_mt_message_valid = TRUE;
  ptr->transfer_route_mt_message.format = format;
#ifndef UTF_TARGET_BUILD
  qcril_sms_hex_to_byte( message,
#ifdef QMI_RIL_HAL_UTF
                     (byte*)ptr->transfer_route_mt_message.data,
#else
                     ptr->transfer_route_mt_message.data,
#endif
                     strlen(message) );
#endif

  ptr->transfer_route_mt_message.data_len = strlen(message)/ 2;
}

/* WAC TLV is expected only after CMAS*/
void ril_utf_qmiril_if::set_qmi_wms_event_report_ind_wac_message(char* message)
{
  wms_event_report_ind_msg_v01 *ptr = (wms_event_report_ind_msg_v01 *) payload;
  ptr->warning_area_valid = TRUE;
#ifndef UTF_TARGET_BUILD
  qcril_sms_hex_to_byte( message,
#ifdef QMI_RIL_HAL_UTF
                     (byte*)ptr->warning_area,
#else
                     ptr->warning_area,
#endif
                     strlen(message) );
#endif
  ptr->warning_area_len = strlen(message)/ 2;
}

void ril_utf_qmiril_if::set_qmi_wms_event_report_ind_message_mode(wms_message_mode_enum_v01 message_mode)
{
  wms_event_report_ind_msg_v01 *ptr = (wms_event_report_ind_msg_v01 *) payload;
  ptr->message_mode_valid = TRUE;
  ptr->message_mode = message_mode;
}
wms_mt_message_type_v01* ril_utf_qmiril_if::set_qmi_wms_event_report_ind_mt_message()
{
  wms_event_report_ind_msg_v01 *ptr = (wms_event_report_ind_msg_v01 *) payload;
  ptr->mt_message_valid = TRUE;
  return &(ptr->mt_message);
}
void ril_utf_qmiril_if::set_qmi_wms_event_report_ind_sms_on_ims(uint8_t sms_on_ims)
{
  wms_event_report_ind_msg_v01 *ptr = (wms_event_report_ind_msg_v01 *) payload;
  ptr->sms_on_ims_valid = TRUE;
  ptr->sms_on_ims = sms_on_ims;
}
wms_etws_message_type_v01* ril_utf_qmiril_if::set_qmi_wms_event_report_ind_etws_message()
{
  wms_event_report_ind_msg_v01 *ptr = (wms_event_report_ind_msg_v01 *) payload;
  ptr->etws_message_valid = TRUE;
  return &(ptr->etws_message);
}

/*========================================================
  QMI_WMS_MEMORY_FULL_IND_V01
========================================================*/
wms_memory_full_info_type_v01* ril_utf_qmiril_if::update_default_qmi_wms_memory_full_ind_msg()
{
  payload = (void *) malloc(sizeof(wms_memory_full_ind_msg_v01));
  payload_len = sizeof(wms_memory_full_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_MEMORY_FULL_IND_V01;

  wms_memory_full_ind_msg_v01 *ptr = (wms_memory_full_ind_msg_v01 *) payload;
  return &(ptr->memory_full_info);
}

/*========================================================
  QMI_WMS_TRANSPORT_LAYER_INFO_IND_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_transport_layer_info_ind_msg(uint8_t registered_ind)
{
  payload = (void *) malloc(sizeof(wms_transport_layer_info_ind_msg_v01));
  payload_len = sizeof(wms_transport_layer_info_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_TRANSPORT_LAYER_INFO_IND_V01;

  wms_transport_layer_info_ind_msg_v01 *ptr = (wms_transport_layer_info_ind_msg_v01 *) payload;
  ptr->registered_ind = registered_ind;
}

#ifndef UTF_TARGET_BUILD
/*========================================================
  QMI_WMS_TRANSPORT_LAYER_MWI_IND_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_transport_layer_mwi_ind_msg()
{
  payload = (void *) malloc(sizeof(wms_transport_layer_mwi_ind_msg_v01));
  payload_len = sizeof(wms_transport_layer_mwi_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_TRANSPORT_LAYER_MWI_IND_V01;
  wms_transport_layer_mwi_ind_msg_v01 *ptr = (wms_transport_layer_mwi_ind_msg_v01 *) payload;
}
wms_mwi_summary_type_v01 *ril_utf_qmiril_if::set_qmi_wms_transport_layer_mwi_ind_message_waiting_summary_info(uint32_t len)
{
  wms_transport_layer_mwi_ind_msg_v01 *ptr = (wms_transport_layer_mwi_ind_msg_v01 *) payload;
  ptr->message_waiting_summary_info_len = len;
  return (ptr->message_waiting_summary_info);
}
void ril_utf_qmiril_if::set_qmi_wms_transport_layer_mwi_ind_UE_address(char *val)
{
  wms_transport_layer_mwi_ind_msg_v01 *ptr = (wms_transport_layer_mwi_ind_msg_v01 *) payload;
  strlcpy (ptr->UE_address, val, strlen (val) + 1);
}
wms_mwi_detail_type_v01 *ril_utf_qmiril_if::set_qmi_wms_transport_layer_mwi_ind_message_waiting_detail_info(uint32_t len)
{
  wms_transport_layer_mwi_ind_msg_v01 *ptr = (wms_transport_layer_mwi_ind_msg_v01 *) payload;
  ptr->message_waiting_detail_info_valid = TRUE;
  ptr->message_waiting_detail_info_len   = len;
  return (ptr->message_waiting_detail_info);
}
#endif
/*========================================================
  QMI_PBM_BIND_SUBSCRIPTION_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_pbm_bind_subscription_req_msg(pbm_subs_type_enum_v01 subs_type)
{
  payload = (void *) malloc(sizeof(pbm_bind_subscription_req_msg_v01));
  payload_len = sizeof(pbm_bind_subscription_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_PBM;
  msg_id = QMI_PBM_BIND_SUBSCRIPTION_REQ_V01;

  pbm_bind_subscription_req_msg_v01 *ptr = (pbm_bind_subscription_req_msg_v01 *) payload;
  ptr->subs_type = subs_type;
}

/*========================================================
  QMI_PBM_BIND_SUBSCRIPTION_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_pbm_bind_subscription_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(pbm_bind_subscription_resp_msg_v01));
  payload_len = sizeof(pbm_bind_subscription_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_PBM;
  msg_id = QMI_PBM_BIND_SUBSCRIPTION_RESP_V01;

  pbm_bind_subscription_resp_msg_v01 *ptr = (pbm_bind_subscription_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

/*========================================================
  QMI_IMSP_SET_EVENT_REPORT_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_imsp_set_event_report_req_v01(void)
{
  payload = (void *) malloc(sizeof(imsp_set_event_report_req_v01));
  payload_len = sizeof(imsp_set_event_report_req_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSP;
  msg_id = QMI_IMSP_SET_EVENT_REPORT_REQ_V01;

  imsp_set_event_report_req_v01 *ptr = (imsp_set_event_report_req_v01*) payload;
  ptr->event_report_bit_masks = 7;
}
/*========================================================
  QMI_IMSP_SET_EVENT_REPORT_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_imsp_set_event_report_resp_v01(void)
{
  payload = (void *) malloc(sizeof(imsp_set_event_report_resp_v01));
  payload_len = sizeof(imsp_set_event_report_resp_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSP;
  msg_id = QMI_IMSP_SET_EVENT_REPORT_RESP_V01;
}
void ril_utf_qmiril_if::set_qmi_imsp_set_event_report_resp_v01_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  imsp_set_event_report_resp_v01 *ptr =
    (imsp_set_event_report_resp_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}
/*========================================================
  QMI_IMS_SETTINGS_GET_REG_MGR_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_reg_mgr_config_req_v01(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_REG_MGR_CONFIG_REQ_V01;
}
/*========================================================
  QMI_IMS_SETTINGS_GET_REG_MGR_CONFIG_RSP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_reg_mgr_config_rsp_v01(void)
{
  payload = (void *) malloc(sizeof(ims_settings_get_reg_mgr_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_get_reg_mgr_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_REG_MGR_CONFIG_RSP_V01;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_reg_mgr_config_rsp_v01_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  ims_settings_get_reg_mgr_config_rsp_msg_v01 *ptr =
    (ims_settings_get_reg_mgr_config_rsp_msg_v01*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}
#ifndef RIL_UTF_IOE
/*
  QMI_IMS_SETTINGS_GET_QIPCALL_CONFIG_REQ_V01
*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_qipcall_config_req_v01(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_QIPCALL_CONFIG_REQ_V01;
}

#ifndef UTF_TARGET_BUILD
/*
QMI_IMS_SETTINGS_GET_QIPCALL_CONFIG_RSP_V01
*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_qipcall_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(ims_settings_get_qipcall_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_get_qipcall_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_QIPCALL_CONFIG_RSP_V01;

  ims_settings_get_qipcall_config_rsp_msg_v01 *ptr = (ims_settings_get_qipcall_config_rsp_msg_v01 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_qipcall_config_rsp_v01_settings_resp(uint8_t val)
{
  ims_settings_get_qipcall_config_rsp_msg_v01 *ptr = (ims_settings_get_qipcall_config_rsp_msg_v01 *) payload;
  ptr->settings_resp_valid = TRUE;
  ptr->settings_resp = (ims_settings_rsp_enum_v01)val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_qipcall_config_rsp_v01_vt_quality_selector(ims_settings_qipcall_vt_quality_enum_v01 val)
{
  ims_settings_get_qipcall_config_rsp_msg_v01 *ptr = (ims_settings_get_qipcall_config_rsp_msg_v01 *) payload;
  ptr->vt_quality_selector_valid = TRUE;
  ptr->vt_quality_selector = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_qipcall_config_rsp_v01_mobile_data_enabled(uint8_t val)
{
  ims_settings_get_qipcall_config_rsp_msg_v01 *ptr = (ims_settings_get_qipcall_config_rsp_msg_v01 *) payload;
  ptr->mobile_data_enabled_valid = TRUE;
  ptr->mobile_data_enabled = val;
}
/*========================================================
  QMI_IMS_SETTINGS_SET_QIPCALL_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_set_qipcall_config_req_v01(void)
{
  payload = (void *) malloc(sizeof(ims_settings_set_qipcall_config_req_msg_v01));
  payload_len = sizeof(ims_settings_set_qipcall_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_QIPCALL_CONFIG_REQ_V01;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_qipcall_config_req_v01_vt_calling_enabled(uint8_t val)
{
  ims_settings_set_qipcall_config_req_msg_v01 *ptr = (ims_settings_set_qipcall_config_req_msg_v01 *) payload;
  ptr->vt_calling_enabled_valid = TRUE;
  ptr->vt_calling_enabled = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_qipcall_config_req_v01_mobile_data_enabled(uint8_t val)
{
  ims_settings_set_qipcall_config_req_msg_v01 *ptr = (ims_settings_set_qipcall_config_req_msg_v01 *) payload;
  ptr->mobile_data_enabled_valid = TRUE;
  ptr->mobile_data_enabled = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_qipcall_config_req_v01_volte_enabled(uint8_t val)
{
  ims_settings_set_qipcall_config_req_msg_v01 *ptr = (ims_settings_set_qipcall_config_req_msg_v01 *) payload;
  ptr->volte_enabled_valid = TRUE;
  ptr->volte_enabled = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_qipcall_config_req_v01_vt_quality_selector(ims_settings_qipcall_vt_quality_enum_v01 val)
{
  ims_settings_set_qipcall_config_req_msg_v01 *ptr = (ims_settings_set_qipcall_config_req_msg_v01 *) payload;
  ptr->vt_quality_selector_valid = TRUE;
  ptr->vt_quality_selector = val;
}
#endif

/*========================================================
  QMI_IMS_SETTINGS_SET_QIPCALL_CONFIG_RSP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_set_qipcall_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(ims_settings_set_qipcall_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_set_qipcall_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_QIPCALL_CONFIG_RSP_V01;

  ims_settings_set_qipcall_config_rsp_msg_v01 *ptr = (ims_settings_set_qipcall_config_rsp_msg_v01 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_qipcall_config_rsp_v01_settings_resp(uint8_t val)
{
  ims_settings_set_qipcall_config_rsp_msg_v01 *ptr = (ims_settings_set_qipcall_config_rsp_msg_v01 *) payload;
  ptr->settings_resp_valid = TRUE;
  ptr->settings_resp = (ims_settings_rsp_enum_v01)val;
}
#endif

/*========================================================
  QMI_IMS_SETTINGS_SET_REG_MGR_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_set_reg_mgr_config_req(void)
{
  payload = (void *) malloc(sizeof(ims_settings_set_reg_mgr_config_req_msg_v01));
  payload_len = sizeof(ims_settings_set_reg_mgr_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_REG_MGR_CONFIG_REQ_V01;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_reg_mgr_config_req_ims_test_mode_enabled(uint8_t val)
{
  ims_settings_set_reg_mgr_config_req_msg_v01 *ptr = (ims_settings_set_reg_mgr_config_req_msg_v01 *) payload;
  ptr->ims_test_mode_enabled_valid = TRUE;
  ptr->ims_test_mode_enabled = val;
}

/*========================================================
  QMI_IMS_SETTINGS_SET_REG_MGR_CONFIG_RSP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_set_reg_mgr_config_rsp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(ims_settings_set_reg_mgr_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_set_reg_mgr_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_REG_MGR_CONFIG_RSP_V01;

  ims_settings_set_reg_mgr_config_rsp_msg_v01 *ptr = (ims_settings_set_reg_mgr_config_rsp_msg_v01 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

/*========================================================
  QMI_IMS_SETTINGS_GET_VOIP_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_voip_config_req_v01()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_VOIP_CONFIG_REQ_V01;
}

/*========================================================
  QMI_IMS_SETTINGS_GET_VOIP_CONFIG_RSP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_voip_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(ims_settings_get_voip_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_get_voip_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_VOIP_CONFIG_RSP_V01;

  ims_settings_get_voip_config_rsp_msg_v01 *ptr = (ims_settings_get_voip_config_rsp_msg_v01 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_voip_config_rsp_v01_settings_resp(uint8_t val)
{
  ims_settings_get_voip_config_rsp_msg_v01 *ptr = (ims_settings_get_voip_config_rsp_msg_v01 *) payload;
  ptr->settings_resp_valid = TRUE;
  ptr->settings_resp = (ims_settings_rsp_enum_v01)val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_voip_config_rsp_v01_amr_mode(uint8_t val)
{
  ims_settings_get_voip_config_rsp_msg_v01 *ptr = (ims_settings_get_voip_config_rsp_msg_v01 *) payload;
  ptr->amr_mode_valid = TRUE;
  ptr->amr_mode = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_voip_config_rsp_v01_amr_wb_mode(uint16_t val)
{
  ims_settings_get_voip_config_rsp_msg_v01 *ptr = (ims_settings_get_voip_config_rsp_msg_v01 *) payload;
  ptr->amr_wb_mode_valid = TRUE;
  ptr->amr_wb_mode = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_voip_config_rsp_v01_session_expiry_timer(uint16_t val)
{
  ims_settings_get_voip_config_rsp_msg_v01 *ptr = (ims_settings_get_voip_config_rsp_msg_v01 *) payload;
  ptr->session_expiry_timer_valid = TRUE;
  ptr->session_expiry_timer = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_voip_config_rsp_v01_min_session_expiry(uint16_t val)
{
  ims_settings_get_voip_config_rsp_msg_v01 *ptr = (ims_settings_get_voip_config_rsp_msg_v01 *) payload;
  ptr->min_session_expiry_valid = TRUE;
  ptr->min_session_expiry = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_voip_config_rsp_v01_voip_silent_redial_enabled(uint8_t val)
{
  ims_settings_get_voip_config_rsp_msg_v01 *ptr = (ims_settings_get_voip_config_rsp_msg_v01 *) payload;
  ptr->voip_silent_redial_enabled_valid = TRUE;
  ptr->voip_silent_redial_enabled = val;
}

/*========================================================
  QMI_IMS_SETTINGS_SET_VOIP_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_set_voip_config_req_v01()
{
  payload = (void *)malloc(sizeof(ims_settings_set_voip_config_req_msg_v01));
  payload_len = sizeof(ims_settings_set_voip_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_VOIP_CONFIG_REQ_V01;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_voip_config_req_v01_amr_mode(uint8_t val)
{
  ims_settings_set_voip_config_req_msg_v01 *ptr = (ims_settings_set_voip_config_req_msg_v01 *) payload;
  ptr->amr_mode_valid = TRUE;
  ptr->amr_mode = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_voip_config_req_v01_amr_wb_mode(uint16_t val)
{
  ims_settings_set_voip_config_req_msg_v01 *ptr = (ims_settings_set_voip_config_req_msg_v01 *) payload;
  ptr->amr_wb_mode_valid = TRUE;
  ptr->amr_wb_mode = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_voip_config_req_v01_session_expiry_timer(uint16_t val)
{
  ims_settings_set_voip_config_req_msg_v01 *ptr = (ims_settings_set_voip_config_req_msg_v01 *) payload;
  ptr->session_expiry_timer_valid = TRUE;
  ptr->session_expiry_timer = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_voip_config_req_v01_min_session_expiry(uint16_t val)
{
  ims_settings_set_voip_config_req_msg_v01 *ptr = (ims_settings_set_voip_config_req_msg_v01 *) payload;
  ptr->min_session_expiry_valid = TRUE;
  ptr->min_session_expiry = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_voip_config_req_v01_voip_silent_redial_enabled(uint8_t val)
{
  ims_settings_set_voip_config_req_msg_v01 *ptr = (ims_settings_set_voip_config_req_msg_v01 *) payload;
  ptr->voip_silent_redial_enabled_valid = TRUE;
  ptr->voip_silent_redial_enabled = val;
}

/*========================================================
  QMI_IMS_SETTINGS_SET_VOIP_CONFIG_RSP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_set_voip_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(ims_settings_set_voip_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_set_voip_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_VOIP_CONFIG_RSP_V01;

  ims_settings_set_voip_config_rsp_msg_v01 *ptr = (ims_settings_set_voip_config_rsp_msg_v01 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_voip_config_rsp_v01_settings_resp(uint8_t val)
{
  ims_settings_set_voip_config_rsp_msg_v01 *ptr = (ims_settings_set_voip_config_rsp_msg_v01 *) payload;
  ptr->settings_resp_valid = TRUE;
  ptr->settings_resp = (ims_settings_rsp_enum_v01)val;
}

/*========================================================
  QMI_IMS_SETTINGS_GET_SIP_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_sip_config_req_v01(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_SIP_CONFIG_REQ_V01;
}

/*========================================================
  QMI_IMS_SETTINGS_GET_SIP_CONFIG_RSP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_sip_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(ims_settings_get_sip_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_get_sip_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_SIP_CONFIG_RSP_V01;

  ims_settings_get_sip_config_rsp_msg_v01 *ptr = (ims_settings_get_sip_config_rsp_msg_v01 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_sip_config_rsp_v01_settings_resp(uint8_t val)
{
  ims_settings_get_sip_config_rsp_msg_v01 *ptr = (ims_settings_get_sip_config_rsp_msg_v01 *) payload;
  ptr->settings_resp_valid = TRUE;
  ptr->settings_resp = (ims_settings_rsp_enum_v01)val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_sip_config_rsp_v01_sip_timer_operator_mode_a(uint32_t val)
{
  ims_settings_get_sip_config_rsp_msg_v01 *ptr = (ims_settings_get_sip_config_rsp_msg_v01 *) payload;
  ptr->sip_timer_operator_mode_a_valid = TRUE;
  ptr->sip_timer_operator_mode_a = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_sip_config_rsp_v01_timer_t1(uint32_t val)
{
  ims_settings_get_sip_config_rsp_msg_v01 *ptr = (ims_settings_get_sip_config_rsp_msg_v01 *) payload;
  ptr->timer_t1_valid = TRUE;
  ptr->timer_t1 = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_sip_config_rsp_v01_timer_t2(uint32_t val)
{
  ims_settings_get_sip_config_rsp_msg_v01 *ptr = (ims_settings_get_sip_config_rsp_msg_v01 *) payload;
  ptr->timer_t2_valid = TRUE;
  ptr->timer_t2 = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_sip_config_rsp_v01_timer_tf(uint32_t val)
{
  ims_settings_get_sip_config_rsp_msg_v01 *ptr = (ims_settings_get_sip_config_rsp_msg_v01 *) payload;
  ptr->timer_tf_valid = TRUE;
  ptr->timer_tf = val;
}

/*========================================================
  QMI_IMS_SETTINGS_SET_SIP_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_set_sip_config_req_v01()
{
  payload = (void *)malloc(sizeof(ims_settings_set_sip_config_req_msg_v01));
  payload_len = sizeof(ims_settings_set_sip_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_SIP_CONFIG_REQ_V01;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_sip_config_req_v01_sip_timer_operator_mode_a(uint32_t val)
{
  ims_settings_set_sip_config_req_msg_v01 *ptr = (ims_settings_set_sip_config_req_msg_v01 *)payload;
  ptr->sip_timer_operator_mode_a_valid = TRUE;
  ptr->sip_timer_operator_mode_a = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_sip_config_req_v01_timer_t1(uint32_t val)
{
  ims_settings_set_sip_config_req_msg_v01 *ptr = (ims_settings_set_sip_config_req_msg_v01 *)payload;
  ptr->timer_t1_valid = TRUE;
  ptr->timer_t1 = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_sip_config_req_v01_timer_t2(uint32_t val)
{
  ims_settings_set_sip_config_req_msg_v01 *ptr = (ims_settings_set_sip_config_req_msg_v01 *)payload;
  ptr->timer_t2_valid = TRUE;
  ptr->timer_t2 = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_sip_config_req_v01_timer_tf(uint32_t val)
{
  ims_settings_set_sip_config_req_msg_v01 *ptr = (ims_settings_set_sip_config_req_msg_v01 *)payload;
  ptr->timer_tf_valid = TRUE;
  ptr->timer_tf = val;
}

/*========================================================
  QMI_IMS_SETTINGS_SET_SIP_CONFIG_RSP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_set_sip_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(ims_settings_set_sip_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_set_sip_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_SIP_CONFIG_RSP_V01;

  ims_settings_set_sip_config_rsp_msg_v01 *ptr = (ims_settings_set_sip_config_rsp_msg_v01 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_sip_config_rsp_v01_settings_resp(uint8_t val)
{
  ims_settings_set_sip_config_rsp_msg_v01 *ptr = (ims_settings_set_sip_config_rsp_msg_v01 *) payload;
  ptr->settings_resp_valid = TRUE;
  ptr->settings_resp = (ims_settings_rsp_enum_v01)val;
}

/*========================================================
  QMI_IMS_SETTINGS_GET_REG_MGR_EXTENDED_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_reg_mgr_extended_config_req_v01()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_REG_MGR_EXTENDED_CONFIG_REQ_V01;
}

/*========================================================
  QMI_IMS_SETTINGS_GET_REG_MGR_EXTENDED_CONFIG_RSP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_reg_mgr_extended_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(ims_settings_get_reg_mgr_extended_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_get_reg_mgr_extended_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_REG_MGR_EXTENDED_CONFIG_RSP_V01;

  ims_settings_get_reg_mgr_extended_config_rsp_msg_v01 *ptr = (ims_settings_get_reg_mgr_extended_config_rsp_msg_v01 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_reg_mgr_extended_config_rsp_v01_settings_resp(uint8_t val)
{
  ims_settings_get_reg_mgr_extended_config_rsp_msg_v01 *ptr = (ims_settings_get_reg_mgr_extended_config_rsp_msg_v01 *) payload;
  ptr->settings_resp_valid = TRUE;
  ptr->settings_resp = (ims_settings_rsp_enum_v01)val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_reg_mgr_extended_config_rsp_v01_t_delay(uint32_t val)
{
  ims_settings_get_reg_mgr_extended_config_rsp_msg_v01 *ptr = (ims_settings_get_reg_mgr_extended_config_rsp_msg_v01 *) payload;
  ptr->t_delay_valid = TRUE;
  ptr->t_delay = val;
}

/*========================================================
  QMI_IMS_SETTINGS_SET_REG_MGR_EXTENDED_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_set_reg_mgr_extended_config_req_v01()
{
  payload = (void *)malloc(sizeof(ims_settings_set_reg_mgr_extended_config_req_msg_v01));
  payload_len = sizeof(ims_settings_set_reg_mgr_extended_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_REG_MGR_EXTENDED_CONFIG_REQ_V01;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_reg_mgr_extended_config_req_v01_t_delay(uint32_t val)
{
  ims_settings_set_reg_mgr_extended_config_req_msg_v01 *ptr = (ims_settings_set_reg_mgr_extended_config_req_msg_v01 *) payload;
  ptr->t_delay_valid = TRUE;
  ptr->t_delay = val;
}

/*========================================================
  QMI_IMS_SETTINGS_SET_REG_MGR_EXTENDED_CONFIG_RSP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_set_reg_mgr_extended_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(ims_settings_set_reg_mgr_extended_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_set_reg_mgr_extended_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_REG_MGR_EXTENDED_CONFIG_RSP_V01;

  ims_settings_set_reg_mgr_extended_config_rsp_msg_v01 *ptr = (ims_settings_set_reg_mgr_extended_config_rsp_msg_v01 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_reg_mgr_extended_config_rsp_v01_settings_resp(uint8_t val)
{
  ims_settings_set_reg_mgr_extended_config_rsp_msg_v01 *ptr = (ims_settings_set_reg_mgr_extended_config_rsp_msg_v01 *) payload;
  ptr->settings_resp_valid = TRUE;
  ptr->settings_resp = (ims_settings_rsp_enum_v01)val;
}

/*========================================================
  QMI_IMS_SETTINGS_GET_USER_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_user_config_req_v01()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_USER_CONFIG_REQ_V01;
}

/*========================================================
  QMI_IMS_SETTINGS_GET_USER_CONFIG_RSP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_user_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(ims_settings_get_user_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_get_user_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_USER_CONFIG_RSP_V01;

  ims_settings_get_user_config_rsp_msg_v01 *ptr = (ims_settings_get_user_config_rsp_msg_v01 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_user_config_rsp_v01_settings_resp(uint8_t val)
{
  ims_settings_get_user_config_rsp_msg_v01 *ptr = (ims_settings_get_user_config_rsp_msg_v01 *) payload;
  ptr->settings_resp_valid = TRUE;
  ptr->settings_resp = (ims_settings_rsp_enum_v01)val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_user_config_rsp_v01_domain_name(char* val)
{
  ims_settings_get_user_config_rsp_msg_v01 *ptr = (ims_settings_get_user_config_rsp_msg_v01 *) payload;
  ptr->ims_domain_valid = TRUE;
  memcpy(ptr->ims_domain, val, strlen(val));
}

/*========================================================
  QMI_IMS_SETTINGS_SET_USER_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_set_user_config_req_v01()
{
  payload = (void *)malloc(sizeof(ims_settings_set_user_config_req_msg_v01));
  payload_len = sizeof(ims_settings_set_user_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_USER_CONFIG_REQ_V01;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_user_config_req_v01_domain_name(char* val)
{
  ims_settings_set_user_config_req_msg_v01 *ptr = (ims_settings_set_user_config_req_msg_v01 *) payload;
  ptr->ims_domain_valid = TRUE;
  memcpy(ptr->ims_domain, val, strlen(val));
}

/*========================================================
  QMI_IMS_SETTINGS_SET_USER_CONFIG_RSP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_set_user_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(ims_settings_set_user_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_set_user_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_USER_CONFIG_RSP_V01;

  ims_settings_set_user_config_rsp_msg_v01 *ptr = (ims_settings_set_user_config_rsp_msg_v01 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_user_config_rsp_v01_settings_resp(uint8_t val)
{
  ims_settings_set_user_config_rsp_msg_v01 *ptr = (ims_settings_set_user_config_rsp_msg_v01 *) payload;
  ptr->settings_resp_valid = TRUE;
  ptr->settings_resp = (ims_settings_rsp_enum_v01)val;
}

/*========================================================
  QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_REQ_V01
========================================================*/
//void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_client_provisioning_config_req_v01()
//{
//  payload = NULL;
//  payload_len = 0;
//  payload_type = ril_utf_qmi_request;
//  service_id = QMI_IMSS;
//  msg_id = QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_REQ_V01;
//}

/*========================================================
  QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_RSP_V01
========================================================*/
//void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_client_provisioning_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error)
//{
//  payload = (void *)malloc(sizeof(ims_settings_get_client_provisioning_config_rsp_msg_v01));
//  payload_len = sizeof(ims_settings_get_client_provisioning_config_rsp_msg_v01);
//  memset(payload, 0x00, payload_len);
//  payload_type = ril_utf_qmi_response;
//  service_id = QMI_IMSS;
//  msg_id = QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_RSP_V01;
//
//  ims_settings_get_client_provisioning_config_rsp_msg_v01 *ptr = (ims_settings_get_client_provisioning_config_rsp_msg_v01 *)payload;
//  ptr->resp.error = error;
//  ptr->resp.result= result;
//}


/*========================================================
  QMI_IMS_SETTINGS_SET_CLIENT_PROVISIONING_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_set_client_provisioning_config_req_v01()
{
  payload = (void *)malloc(sizeof(ims_settings_set_client_provisioning_config_req_msg_v01));
  payload_len = sizeof(ims_settings_set_client_provisioning_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_CLIENT_PROVISIONING_CONFIG_REQ_V01;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_client_provisioning_config_req_v01_enable_volte(uint8_t val)
{
  ims_settings_set_client_provisioning_config_req_msg_v01 *ptr = (ims_settings_set_client_provisioning_config_req_msg_v01 *) payload;
  ptr->enable_volte_valid = TRUE;
  ptr->enable_volte = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_client_provisioning_config_req_v01_enable_vt(uint8_t val)
{
  ims_settings_set_client_provisioning_config_req_msg_v01 *ptr = (ims_settings_set_client_provisioning_config_req_msg_v01 *) payload;
  ptr->enable_vt_valid = TRUE;
  ptr->enable_vt = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_client_provisioning_config_req_v01_enable_presence(uint8_t val)
{
  ims_settings_set_client_provisioning_config_req_msg_v01 *ptr = (ims_settings_set_client_provisioning_config_req_msg_v01 *) payload;
  ptr->enable_presence_valid = TRUE;
  ptr->enable_presence = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_client_provisioning_config_req_v01_wifi_call(ims_settings_wfc_status_enum_v01 val)
{
  ims_settings_set_client_provisioning_config_req_msg_v01 *ptr = (ims_settings_set_client_provisioning_config_req_msg_v01 *) payload;
  ptr->wifi_call_valid = TRUE;
  ptr->wifi_call = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_client_provisioning_config_req_v01_wifi_call_roaming(ims_settings_wfc_roaming_enum_v01 val)
{
  ims_settings_set_client_provisioning_config_req_msg_v01 *ptr = (ims_settings_set_client_provisioning_config_req_msg_v01 *) payload;
  ptr->wifi_call_roaming_valid = TRUE;
  ptr->wifi_call_roaming = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_client_provisioning_config_req_v01_wifi_call_preference(ims_settings_wfc_preference_v01 val)
{
  ims_settings_set_client_provisioning_config_req_msg_v01 *ptr = (ims_settings_set_client_provisioning_config_req_msg_v01 *) payload;
  ptr->wifi_call_preference_valid = TRUE;
  ptr->wifi_call_preference = val;
}

/*========================================================
  QMI_IMS_SETTINGS_SET_CLIENT_PROVISIONING_CONFIG_RSP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_set_client_provisioning_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(ims_settings_set_client_provisioning_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_set_client_provisioning_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_CLIENT_PROVISIONING_CONFIG_RSP_V01;

  ims_settings_set_client_provisioning_config_rsp_msg_v01 *ptr = (ims_settings_set_client_provisioning_config_rsp_msg_v01 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_client_provisioning_config_rsp_v01_settings_resp(uint8_t val)
{
  ims_settings_set_client_provisioning_config_rsp_msg_v01 *ptr = (ims_settings_set_client_provisioning_config_rsp_msg_v01 *) payload;
  ptr->settings_resp_valid = TRUE;
  ptr->settings_resp = (ims_settings_rsp_enum_v01)val;
}

/*========================================================
  QMI_IMS_SETTINGS_GET_SMS_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_sms_config_req_v01()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_SMS_CONFIG_REQ_V01;
}

/*========================================================
  QMI_IMS_SETTINGS_GET_SMS_CONFIG_RSP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_sms_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(ims_settings_get_sms_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_get_sms_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_SMS_CONFIG_RSP_V01;

  ims_settings_get_sms_config_rsp_msg_v01 *ptr = (ims_settings_get_sms_config_rsp_msg_v01 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_sms_config_rsp_v01_settings_resp(uint8_t val)
{
  ims_settings_get_sms_config_rsp_msg_v01 *ptr = (ims_settings_get_sms_config_rsp_msg_v01 *) payload;
  ptr->settings_resp_valid = TRUE;
  ptr->settings_resp = (ims_settings_rsp_enum_v01)val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_sms_config_rsp_v01_sms_format(ims_settings_sms_format_enum_v01 val)
{
  ims_settings_get_sms_config_rsp_msg_v01 *ptr = (ims_settings_get_sms_config_rsp_msg_v01 *) payload;
  ptr->sms_format_valid = TRUE;
  ptr->sms_format = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_sms_config_rsp_v01_sms_over_ip_network_indication(uint8_t val)
{
  ims_settings_get_sms_config_rsp_msg_v01 *ptr = (ims_settings_get_sms_config_rsp_msg_v01 *) payload;
  ptr->sms_over_ip_network_indication_valid = TRUE;
  ptr->sms_over_ip_network_indication = val;
}

/*========================================================
  QMI_IMS_SETTINGS_SET_SMS_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_set_sms_config_req_v01()
{
  payload = (void *)malloc(sizeof(ims_settings_set_sms_config_req_msg_v01));
  payload_len = sizeof(ims_settings_set_sms_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_SMS_CONFIG_REQ_V01;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_sms_config_req_v01_sms_format(ims_settings_sms_format_enum_v01 val)
{
  ims_settings_set_sms_config_req_msg_v01 *ptr = (ims_settings_set_sms_config_req_msg_v01 *) payload;
  ptr->sms_format_valid = TRUE;
  ptr->sms_format = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_sms_config_req_v01_sms_over_ip_network_indication(uint8_t val)
{
  ims_settings_set_sms_config_req_msg_v01 *ptr = (ims_settings_set_sms_config_req_msg_v01 *) payload;
  ptr->sms_over_ip_network_indication_valid = TRUE;
  ptr->sms_over_ip_network_indication = val;
}

/*========================================================
  QMI_IMS_SETTINGS_SET_SMS_CONFIG_RSP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_set_sms_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(ims_settings_set_sms_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_set_sms_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_SMS_CONFIG_RSP_V01;

  ims_settings_set_sms_config_rsp_msg_v01 *ptr = (ims_settings_set_sms_config_rsp_msg_v01 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_sms_config_rsp_v01_settings_resp(uint8_t val)
{
  ims_settings_set_sms_config_rsp_msg_v01 *ptr = (ims_settings_set_sms_config_rsp_msg_v01 *) payload;
  ptr->settings_resp_valid = TRUE;
  ptr->settings_resp = (ims_settings_rsp_enum_v01)val;
}

/*========================================================
  QMI_IMS_SETTINGS_GET_PRESENCE_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_presence_config_req_v01()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_PRESENCE_CONFIG_REQ_V01;
}

/*========================================================
  QMI_IMS_SETTINGS_GET_PRESENCE_CONFIG_RSP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_presence_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(ims_settings_get_presence_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_get_presence_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_PRESENCE_CONFIG_RSP_V01;

  ims_settings_get_presence_config_rsp_msg_v01 *ptr = (ims_settings_get_presence_config_rsp_msg_v01 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_presence_config_rsp_v01_settings_resp(uint8_t val)
{
  ims_settings_get_presence_config_rsp_msg_v01 *ptr = (ims_settings_get_presence_config_rsp_msg_v01 *) payload;
  ptr->settings_resp_valid = TRUE;
  ptr->settings_resp = (ims_settings_rsp_enum_v01)val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_presence_config_rsp_v01_publish_expiry_timer(uint32_t val)
{
  ims_settings_get_presence_config_rsp_msg_v01 *ptr = (ims_settings_get_presence_config_rsp_msg_v01 *) payload;
  ptr->publish_expiry_timer_valid = TRUE;
  ptr->publish_expiry_timer = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_presence_config_rsp_v01_publish_extended_expiry_timer(uint32_t val)
{
  ims_settings_get_presence_config_rsp_msg_v01 *ptr = (ims_settings_get_presence_config_rsp_msg_v01 *) payload;
  ptr->publish_extended_expiry_timer_valid = TRUE;
  ptr->publish_extended_expiry_timer = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_presence_config_rsp_v01_capabilites_cache_expiration(uint32_t val)
{
  ims_settings_get_presence_config_rsp_msg_v01 *ptr = (ims_settings_get_presence_config_rsp_msg_v01 *) payload;
  ptr->capabilites_cache_expiration_valid = TRUE;
  ptr->capabilites_cache_expiration = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_presence_config_rsp_v01_availability_cache_expiration(uint32_t val)
{
  ims_settings_get_presence_config_rsp_msg_v01 *ptr = (ims_settings_get_presence_config_rsp_msg_v01 *) payload;
  ptr->availability_cache_expiration_valid = TRUE;
  ptr->availability_cache_expiration = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_presence_config_rsp_v01_capability_poll_interval(uint32_t val)
{
  ims_settings_get_presence_config_rsp_msg_v01 *ptr = (ims_settings_get_presence_config_rsp_msg_v01 *) payload;
  ptr->capability_poll_interval_valid = TRUE;
  ptr->capability_poll_interval = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_presence_config_rsp_v01_minimum_publish_interval(uint32_t val)
{
  ims_settings_get_presence_config_rsp_msg_v01 *ptr = (ims_settings_get_presence_config_rsp_msg_v01 *) payload;
  ptr->minimum_publish_interval_valid = TRUE;
  ptr->minimum_publish_interval = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_presence_config_rsp_v01_max_subcription_list_entries(uint32_t val)
{
  ims_settings_get_presence_config_rsp_msg_v01 *ptr = (ims_settings_get_presence_config_rsp_msg_v01 *) payload;
  ptr->max_subcription_list_entries_valid = TRUE;
  ptr->max_subcription_list_entries = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_presence_config_rsp_v01_capability_poll_list_subscription_expiry_timer(uint32_t val)
{
  ims_settings_get_presence_config_rsp_msg_v01 *ptr = (ims_settings_get_presence_config_rsp_msg_v01 *) payload;
  ptr->capability_poll_list_subscription_expiry_timer_valid = TRUE;
  ptr->capability_poll_list_subscription_expiry_timer = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_presence_config_rsp_v01_gzip_enabled(uint8_t val)
{
  ims_settings_get_presence_config_rsp_msg_v01 *ptr = (ims_settings_get_presence_config_rsp_msg_v01 *) payload;
  ptr->gzip_enabled_valid = TRUE;
  ptr->gzip_enabled = val;
}

/*========================================================
  QMI_IMS_SETTINGS_SET_PRESENCE_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_set_presence_config_req_v01()
{
  payload = (void *)malloc(sizeof(ims_settings_set_presence_config_req_msg_v01));
  payload_len = sizeof(ims_settings_set_presence_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_PRESENCE_CONFIG_REQ_V01;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_presence_config_req_v01_publish_expiry_timer(uint32_t val)
{
  ims_settings_set_presence_config_req_msg_v01 *ptr = (ims_settings_set_presence_config_req_msg_v01 *) payload;
  ptr->publish_expiry_timer_valid = TRUE;
  ptr->publish_expiry_timer = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_presence_config_req_v01_publish_extended_expiry_timer(uint32_t val)
{
  ims_settings_set_presence_config_req_msg_v01 *ptr = (ims_settings_set_presence_config_req_msg_v01 *) payload;
  ptr->publish_extended_expiry_timer_valid = TRUE;
  ptr->publish_extended_expiry_timer = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_presence_config_req_v01_capabilites_cache_expiration(uint32_t val)
{
  ims_settings_set_presence_config_req_msg_v01 *ptr = (ims_settings_set_presence_config_req_msg_v01 *) payload;
  ptr->capabilites_cache_expiration_valid = TRUE;
  ptr->capabilites_cache_expiration = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_presence_config_req_v01_availability_cache_expiration(uint32_t val)
{
  ims_settings_set_presence_config_req_msg_v01 *ptr = (ims_settings_set_presence_config_req_msg_v01 *) payload;
  ptr->availability_cache_expiration_valid = TRUE;
  ptr->availability_cache_expiration = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_presence_config_req_v01_capability_poll_interval(uint32_t val)
{
  ims_settings_set_presence_config_req_msg_v01 *ptr = (ims_settings_set_presence_config_req_msg_v01 *) payload;
  ptr->capability_poll_interval_valid = TRUE;
  ptr->capability_poll_interval = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_presence_config_req_v01_minimum_publish_interval(uint32_t val)
{
  ims_settings_set_presence_config_req_msg_v01 *ptr = (ims_settings_set_presence_config_req_msg_v01 *) payload;
  ptr->minimum_publish_interval_valid = TRUE;
  ptr->minimum_publish_interval = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_presence_config_req_v01_max_subcription_list_entries(uint32_t val)
{
  ims_settings_set_presence_config_req_msg_v01 *ptr = (ims_settings_set_presence_config_req_msg_v01 *) payload;
  ptr->max_subcription_list_entries_valid = TRUE;
  ptr->max_subcription_list_entries = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_presence_config_req_v01_capability_poll_list_subscription_expiry_timer(uint32_t val)
{
  ims_settings_set_presence_config_req_msg_v01 *ptr = (ims_settings_set_presence_config_req_msg_v01 *) payload;
  ptr->capability_poll_list_subscription_expiry_timer_valid = TRUE;
  ptr->capability_poll_list_subscription_expiry_timer = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_presence_config_req_v01_gzip_enabled(uint8_t val)
{
  ims_settings_set_presence_config_req_msg_v01 *ptr = (ims_settings_set_presence_config_req_msg_v01 *) payload;
  ptr->gzip_enabled_valid = TRUE;
  ptr->gzip_enabled = val;
}

/*========================================================
  QMI_IMS_SETTINGS_SET_PRESENCE_CONFIG_RSP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_set_presence_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(ims_settings_set_presence_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_set_presence_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_PRESENCE_CONFIG_RSP_V01;

  ims_settings_set_presence_config_rsp_msg_v01 *ptr = (ims_settings_set_presence_config_rsp_msg_v01 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_presence_config_rsp_v01_settings_resp(uint8_t val)
{
  ims_settings_set_presence_config_rsp_msg_v01 *ptr = (ims_settings_set_presence_config_rsp_msg_v01 *) payload;
  ptr->settings_resp_valid = TRUE;
  ptr->settings_resp = (ims_settings_rsp_enum_v01)val;
}

/*========================================================
  QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_client_provisioning_config_req(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_REQ_V01;
}

/*========================================================
  QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_RSP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_client_provisioning_config_rsp(
        qmi_result_type_v01 result,
        qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(ims_settings_get_client_provisioning_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_get_client_provisioning_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_RSP_V01;

  ims_settings_get_client_provisioning_config_rsp_msg_v01 *ptr =
      (ims_settings_get_client_provisioning_config_rsp_msg_v01 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_client_provisioning_config_rsp_wifi_call(
            ims_settings_wfc_status_enum_v01 wifi_call)
{
  ims_settings_get_client_provisioning_config_rsp_msg_v01 *ptr =
      (ims_settings_get_client_provisioning_config_rsp_msg_v01 *) payload;
  ptr->wifi_call_valid = TRUE;
  ptr->wifi_call = wifi_call;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_client_provisioning_config_rsp_wifi_call_preference(
            ims_settings_wfc_preference_v01 wifi_call_preference)
{
  ims_settings_get_client_provisioning_config_rsp_msg_v01 *ptr =
      (ims_settings_get_client_provisioning_config_rsp_msg_v01 *) payload;
  ptr->wifi_call_preference_valid = TRUE;
  ptr->wifi_call_preference = wifi_call_preference;
}

/*========================================================
  QMI_IMS_SETTINGS_SET_CLIENT_PROVISIONING_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_set_client_provisioning_config_req(void)
{
  payload = (void *)malloc(sizeof(ims_settings_set_client_provisioning_config_req_msg_v01));
  payload_len = sizeof(ims_settings_set_client_provisioning_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_CLIENT_PROVISIONING_CONFIG_REQ_V01;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_client_provisioning_config_req_wifi_call(
            ims_settings_wfc_status_enum_v01 wifi_call)
{
  ims_settings_set_client_provisioning_config_req_msg_v01 *ptr =
      (ims_settings_set_client_provisioning_config_req_msg_v01 *) payload;
  ptr->wifi_call_valid = TRUE;
  ptr->wifi_call = wifi_call;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_client_provisioning_config_req_wifi_call_preference(
            ims_settings_wfc_preference_v01 wifi_call_preference)
{
  ims_settings_set_client_provisioning_config_req_msg_v01 *ptr =
      (ims_settings_set_client_provisioning_config_req_msg_v01 *) payload;
  ptr->wifi_call_preference_valid = TRUE;
  ptr->wifi_call_preference = wifi_call_preference;
}

/*========================================================
  QMI_IMS_SETTINGS_SET_CLIENT_PROVISIONING_CONFIG_RSP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_set_client_provisioning_config_rsp(
        qmi_result_type_v01 result,
        qmi_error_type_v01 error)
{
  payload = (void *)malloc(sizeof(ims_settings_set_client_provisioning_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_set_client_provisioning_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_CLIENT_PROVISIONING_CONFIG_RSP_V01;

  ims_settings_set_client_provisioning_config_rsp_msg_v01 *ptr =
      (ims_settings_set_client_provisioning_config_rsp_msg_v01 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

/*========================================================
  QMI_IMSA_IND_REG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_imsa_ind_reg_req_v01(void)
{
  payload = (void *) malloc(sizeof(imsa_ind_reg_req_msg_v01));
  payload_len = sizeof(imsa_ind_reg_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSA;
  msg_id = QMI_IMSA_IND_REG_REQ_V01;

  imsa_ind_reg_req_msg_v01 *ptr = (imsa_ind_reg_req_msg_v01*) payload;
  ptr->reg_status_config_valid = TRUE;
  ptr->reg_status_config = 1;
  ptr->service_status_config_valid = TRUE;
  ptr->service_status_config = 1;
  ptr->geo_location_status_config_valid = TRUE;
  ptr->geo_location_status_config = 1;
  ptr->vowifiprofile_status_config_valid = TRUE;
  ptr->vowifiprofile_status_config = 1;
  ptr->ims_block_status_config_valid = TRUE;
  ptr->ims_block_status_config = 1;
  #ifndef RIL_UTF_IOE
  ptr->rat_handover_config_valid = TRUE;
  ptr->rat_handover_config = 1;
  ptr->voice_config_valid = TRUE;
  ptr->voice_config = 1;
  #endif
  ptr->pdp_status_config_valid = TRUE;
  ptr->pdp_status_config = 1;
}

/*========================================================
  QMI_IMSA_IND_REG_RSP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_imsa_ind_reg_rsp_v01(void)
{
  payload = (void *) malloc(sizeof(imsa_ind_reg_rsp_msg_v01));
  payload_len = sizeof(imsa_ind_reg_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSA;
  msg_id = QMI_IMSA_IND_REG_RSP_V01;
}
void ril_utf_qmiril_if::set_qmi_imsa_ind_req_rsp_v01_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  imsa_ind_reg_rsp_msg_v01 *ptr =
    (imsa_ind_reg_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
  QMI_IMSA_GET_SERVICE_STATUS_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_imsa_get_service_status_req_v01(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSA;
  msg_id = QMI_IMSA_GET_SERVICE_STATUS_REQ_V01;
}

/*========================================================
  QMI_IMSA_GET_SERVICE_STATUS_RSP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_imsa_get_service_status_rsp_v01(void)
{
  payload = (void *) malloc(sizeof(imsa_get_service_status_resp_msg_v01));
  payload_len = sizeof(imsa_get_service_status_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSA;
  msg_id = QMI_IMSA_GET_SERVICE_STATUS_RSP_V01;

  /*
  imsa_get_service_status_resp_msg_v01 *ptr = (imsa_get_service_status_resp_msg_v01*) payload;
  ptr->voip_service_status_valid = TRUE;
  ptr->voip_service_status = IMSA_FULL_SERVICE_V01;
  */
}
void ril_utf_qmiril_if::set_qmi_imsa_get_service_status_rsp_v01
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  imsa_get_service_status_resp_msg_v01 *ptr = (imsa_get_service_status_resp_msg_v01*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}
#ifndef RIL_UTF_IOE
void ril_utf_qmiril_if::set_qmi_imsa_get_service_status_rsp_v01_sms_service_status(imsa_service_status_enum_v01 val)
{
  imsa_get_service_status_resp_msg_v01 *ptr = (imsa_get_service_status_resp_msg_v01*) payload;
  ptr->sms_service_status_valid = TRUE;
  ptr->sms_service_status = val;
}
void ril_utf_qmiril_if::set_qmi_imsa_get_service_status_rsp_v01_voip_service_status(imsa_service_status_enum_v01 val)
{
  imsa_get_service_status_resp_msg_v01 *ptr = (imsa_get_service_status_resp_msg_v01*) payload;
  ptr->voip_service_status_valid = TRUE;
  ptr->voip_service_status = val;
}
void ril_utf_qmiril_if::set_qmi_imsa_get_service_status_rsp_v01_vt_service_status(imsa_service_status_enum_v01 val)
{
  imsa_get_service_status_resp_msg_v01 *ptr = (imsa_get_service_status_resp_msg_v01*) payload;
  ptr->vt_service_status_valid = TRUE;
  ptr->vt_service_status = val;
}
void ril_utf_qmiril_if::set_qmi_imsa_get_service_status_rsp_v01_sms_service_rat(imsa_service_rat_enum_v01 val)
{
  imsa_get_service_status_resp_msg_v01 *ptr = (imsa_get_service_status_resp_msg_v01*) payload;
  ptr->sms_service_rat_valid = TRUE;
  ptr->sms_service_rat = val;
}
void ril_utf_qmiril_if::set_qmi_imsa_get_service_status_rsp_v01_voip_service_rat(imsa_service_rat_enum_v01 val)
{
  imsa_get_service_status_resp_msg_v01 *ptr = (imsa_get_service_status_resp_msg_v01*) payload;
  ptr->voip_service_rat_valid = TRUE;
  ptr->voip_service_rat = val;
}
void ril_utf_qmiril_if::set_qmi_imsa_get_service_status_rsp_v01_vt_service_rat(imsa_service_rat_enum_v01 val)
{
  imsa_get_service_status_resp_msg_v01 *ptr = (imsa_get_service_status_resp_msg_v01*) payload;
  ptr->vt_service_rat_valid = TRUE;
  ptr->vt_service_rat = val;
}
void ril_utf_qmiril_if::set_qmi_imsa_get_service_status_rsp_v01_ut_service_status(imsa_service_status_enum_v01 val)
{
  imsa_get_service_status_resp_msg_v01 *ptr = (imsa_get_service_status_resp_msg_v01*) payload;
  ptr->ut_service_status_valid = TRUE;
  ptr->ut_service_status = val;
}
void ril_utf_qmiril_if::set_qmi_imsa_get_service_status_rsp_v01_ut_service_rat(imsa_service_rat_enum_v01 val)
{
  imsa_get_service_status_resp_msg_v01 *ptr = (imsa_get_service_status_resp_msg_v01*) payload;
  ptr->ut_service_rat_valid = TRUE;
  ptr->ut_service_rat = val;
}

/*========================================================
  QMI_IMSA_REGISTRATION_STATUS_IND_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_imsa_registration_status_ind(void)
{
  payload = (void *) malloc(sizeof(imsa_registration_status_ind_msg_v01));
  payload_len = sizeof(imsa_registration_status_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_IMSA;
  msg_id = QMI_IMSA_REGISTRATION_STATUS_IND_V01;
}

void ril_utf_qmiril_if::set_qmi_imsa_registration_status_ind_ims_registered(uint8_t ims_registered)
{
  imsa_registration_status_ind_msg_v01 *ptr = (imsa_registration_status_ind_msg_v01 *) payload;
  ptr->ims_registered = ims_registered;
}

void ril_utf_qmiril_if::set_qmi_imsa_registration_status_ind_ims_registration_failure_error_code(uint8_t ims_registration_failure_error_code_valid,
uint16_t ims_registration_failure_error_code)
{
  imsa_registration_status_ind_msg_v01 *ptr = (imsa_registration_status_ind_msg_v01 *) payload;
  ptr->ims_registration_failure_error_code_valid = ims_registration_failure_error_code_valid;
  ptr->ims_registration_failure_error_code = ims_registration_failure_error_code;
}

void ril_utf_qmiril_if::set_qmi_imsa_registration_status_ind_ims_reg_status(uint8_t ims_reg_status_valid, imsa_ims_registration_status_enum_v01 ims_reg_status)
{
  imsa_registration_status_ind_msg_v01 *ptr = (imsa_registration_status_ind_msg_v01 *) payload;
  ptr->ims_reg_status_valid = ims_reg_status_valid;
  ptr->ims_reg_status = ims_reg_status;
}

void ril_utf_qmiril_if::set_qmi_imsa_registration_status_ind_registration_network(imsa_service_rat_enum_v01 val)
{
  imsa_registration_status_ind_msg_v01 *ptr = (imsa_registration_status_ind_msg_v01 *) payload;
  ptr->registration_network_valid = TRUE;
  ptr->registration_network = val;
}

p_associated_uri_v01 *ril_utf_qmiril_if::set_qmi_imsa_registration_status_ind_uri_list(uint32_t uri_list_len)
{
  imsa_registration_status_ind_msg_v01 *ptr = (imsa_registration_status_ind_msg_v01 *) payload;
  ptr->uri_list_valid = TRUE;
  ptr->uri_list_len = uri_list_len;
  return (ptr->uri_list);
}

void ril_utf_qmiril_if::set_qmi_imsa_registration_status_ind_registration_error_string(uint8_t registration_error_string_valid, char* registration_error_string)
{
  imsa_registration_status_ind_msg_v01 *ptr = (imsa_registration_status_ind_msg_v01 *) payload;
  ptr->registration_error_string_valid = registration_error_string_valid;

  if(registration_error_string)
  {
    strlcpy(ptr->registration_error_string, registration_error_string, (IMSA_REGISTRATION_FAILURE_ERROR_STR_LEN_V01 + 1));
  }
}

/*========================================================
  QMI_IMSA_SERVICE_STATUS_IND_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_imsa_service_status_ind_v01(void)
{
  payload = (void *) malloc(sizeof(imsa_service_status_ind_msg_v01));
  payload_len = sizeof(imsa_service_status_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_IMSA;
  msg_id = QMI_IMSA_SERVICE_STATUS_IND_V01;
}
void ril_utf_qmiril_if::set_qmi_imsa_service_status_ind_v01_sms_service_status(imsa_service_status_enum_v01 val)
{
  imsa_service_status_ind_msg_v01 *ptr = (imsa_service_status_ind_msg_v01 *) payload;
  ptr->sms_service_status_valid = TRUE;
  ptr->sms_service_status = val;
}

void ril_utf_qmiril_if::set_qmi_imsa_service_status_ind_v01_voip_service_status(imsa_service_status_enum_v01 val)
{
  imsa_service_status_ind_msg_v01 *ptr = (imsa_service_status_ind_msg_v01 *) payload;
  ptr->voip_service_status_valid = TRUE;
  ptr->voip_service_status = val;
}
void ril_utf_qmiril_if::set_qmi_imsa_service_status_ind_v01_vt_service_status(imsa_service_status_enum_v01 val)
{
  imsa_service_status_ind_msg_v01 *ptr = (imsa_service_status_ind_msg_v01 *) payload;
  ptr->vt_service_status_valid = TRUE;
  ptr->vt_service_status = val;
}
void ril_utf_qmiril_if::set_qmi_imsa_service_status_ind_v01_sms_service_rat(imsa_service_rat_enum_v01 val)
{
  imsa_service_status_ind_msg_v01 *ptr = (imsa_service_status_ind_msg_v01 *) payload;
  ptr->sms_service_rat_valid = TRUE;
  ptr->sms_service_rat = val;
}
void ril_utf_qmiril_if::set_qmi_imsa_service_status_ind_v01_voip_service_rat(imsa_service_rat_enum_v01 val)
{
  imsa_service_status_ind_msg_v01 *ptr = (imsa_service_status_ind_msg_v01 *) payload;
  ptr->voip_service_rat_valid = TRUE;
  ptr->voip_service_rat = val;
}
void ril_utf_qmiril_if::set_qmi_imsa_service_status_ind_v01_vt_service_rat(imsa_service_rat_enum_v01 val)
{
  imsa_service_status_ind_msg_v01 *ptr = (imsa_service_status_ind_msg_v01 *) payload;
  ptr->vt_service_rat_valid = TRUE;
  ptr->vt_service_rat = val;
}
void ril_utf_qmiril_if::set_qmi_imsa_service_status_ind_v01_ut_service_status(imsa_service_status_enum_v01 val)
{
  imsa_service_status_ind_msg_v01 *ptr = (imsa_service_status_ind_msg_v01 *) payload;
  ptr->ut_service_status_valid = TRUE;
  ptr->ut_service_status = val;
}
void ril_utf_qmiril_if::set_qmi_imsa_service_status_ind_v01_ut_service_rat(imsa_service_rat_enum_v01 val)
{
  imsa_service_status_ind_msg_v01 *ptr = (imsa_service_status_ind_msg_v01 *) payload;
  ptr->ut_service_rat_valid = TRUE;
  ptr->ut_service_rat = val;
}

/*========================================================
  QMI_IMSA_PDP_STATUS_IND_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_imsa_pdp_status_ind(uint8_t is_ims_pdp_connected)
{
  payload = (void *) malloc(sizeof(imsa_pdp_status_ind_msg_v01));
  payload_len = sizeof(imsa_pdp_status_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_IMSA;
  msg_id = QMI_IMSA_PDP_STATUS_IND_V01;
  imsa_pdp_status_ind_msg_v01 *ptr = (imsa_pdp_status_ind_msg_v01 *) payload;
  ptr->is_ims_pdp_connected = is_ims_pdp_connected;
}

/*========================================================
  QMI_IMSA_IMS_BLOCK_STATUS_IND_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_imsa_ims_block_status_ind()
{
  payload = (void *) malloc(sizeof(imsa_ims_block_status_ind_msg_v01));
  payload_len = sizeof(imsa_ims_block_status_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_IMSA;
  msg_id = QMI_IMSA_IMS_BLOCK_STATUS_IND_V01;
}
imsa_ims_failure_reason_type_v01 *ril_utf_qmiril_if::set_qmi_imsa_ims_block_status_ind_ims_failure_on_wwan()
{
  imsa_ims_block_status_ind_msg_v01 *ptr = (imsa_ims_block_status_ind_msg_v01 *) payload;
  ptr->ims_failure_on_wwan_valid = TRUE;
  return &(ptr->ims_failure_on_wwan);
}
imsa_ims_failure_reason_type_v01 *ril_utf_qmiril_if::set_qmi_imsa_ims_block_status_ind_ims_failure_on_wlan()
{
  imsa_ims_block_status_ind_msg_v01 *ptr = (imsa_ims_block_status_ind_msg_v01 *) payload;
  ptr->ims_failure_on_wlan_valid = TRUE;
  return &(ptr->ims_failure_on_wlan);
}

/*========================================================
  QMI_IMSA_DIGITS_INFO_PENDING_IND_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_imsa_digits_info_pending_ind(void)
{
  payload = (void *) malloc(sizeof(imsa_digits_info_pending_ind_msg_v01));
  payload_len = sizeof(imsa_digits_info_pending_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_IMSA;
  msg_id = QMI_IMSA_DIGITS_INFO_PENDING_IND_V01;
}

/*========================================================
  QMI_IMSA_DIGITS_UPDATE_IND_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_imsa_digits_update_ind(void)
{
  payload = (void *) malloc(sizeof(imsa_digits_update_ind_msg_v01));
  payload_len = sizeof(imsa_digits_update_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_IMSA;
  msg_id = QMI_IMSA_DIGITS_UPDATE_IND_V01;
}
void ril_utf_qmiril_if::set_qmi_imsa_digits_update_ind_no_of_lines(uint8_t no_of_lines)
{
  imsa_digits_update_ind_msg_v01 *ptr = (imsa_digits_update_ind_msg_v01 *) payload;
  ptr->no_of_lines_valid = TRUE;
  ptr->no_of_lines = no_of_lines;
}
imsa_digits_line_info_v01 *ril_utf_qmiril_if::set_qmi_imsa_digits_update_ind_line_info_list(uint32_t line_info_list_len)
{
  imsa_digits_update_ind_msg_v01 *ptr = (imsa_digits_update_ind_msg_v01 *) payload;
  ptr->line_info_list_valid = TRUE;
  ptr->line_info_list_len = line_info_list_len;
  return (ptr->line_info_list);
}

/*========================================================
  QMI_IMSA_VOICE_INFO_IND_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_imsa_voice_info_ind(imsa_voice_info_enum_v01 voice_info)
{
  payload = (void *) malloc(sizeof(imsa_voice_info_ind_msg_v01));
  payload_len = sizeof(imsa_voice_info_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_IMSA;
  msg_id = QMI_IMSA_VOICE_INFO_IND_V01;
  imsa_voice_info_ind_msg_v01 *ptr = (imsa_voice_info_ind_msg_v01 *) payload;
  ptr->voice_info = voice_info;
}

#endif
/*========================================================
  QMI_IMSA_RAT_HANDOVER_STATUS_IND_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_imsa_rat_handover_status_ind(void)
{
  payload = (void *) malloc(sizeof(imsa_rat_handover_status_ind_msg_v01));
  payload_len = sizeof(imsa_rat_handover_status_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_IMSA;
  msg_id = QMI_IMSA_RAT_HANDOVER_STATUS_IND_V01;
}
void ril_utf_qmiril_if::set_qmi_imsa_rat_handover_status_ind_rat_ho_status(
        imsa_rat_handover_status_enum_v01 rat_ho_status,
        imsa_service_rat_enum_v01 source_rat,
        imsa_service_rat_enum_v01 target_rat,
        const char *cause_code,
        int cause_code_len)
{
  imsa_rat_handover_status_ind_msg_v01 *ptr = (imsa_rat_handover_status_ind_msg_v01 *) payload;
  ptr->rat_ho_status_info_valid = TRUE;
  ptr->rat_ho_status_info.rat_ho_status = rat_ho_status;
  ptr->rat_ho_status_info.source_rat = source_rat;
  ptr->rat_ho_status_info.target_rat = target_rat;
  if (cause_code && cause_code_len > 0 && cause_code_len < IMSA_HO_FAILURE_CAUSE_CODE_STR_LEN_V01)
  {
    strlcpy(ptr->rat_ho_status_info.cause_code, cause_code,
            sizeof(ptr->rat_ho_status_info.cause_code));
  }
}

/*========================================================
  QMI_IMSA_SET_GEO_LOC_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_imsa_set_geo_loc_req(void)
{
  payload = (void *) malloc(sizeof(imsa_set_geo_loc_req_msg_v01));
  payload_len = sizeof(imsa_set_geo_loc_req_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSA;
  msg_id = QMI_IMSA_SET_GEO_LOC_REQ_V01;
}

void ril_utf_qmiril_if::set_qmi_imsa_set_geo_loc_latitude(double latitude)
{
  imsa_set_geo_loc_req_msg_v01 *msg = (imsa_set_geo_loc_req_msg_v01 *)payload;
  msg->latitude_valid = TRUE;
  msg->latitude = latitude;
}

void ril_utf_qmiril_if::set_qmi_imsa_set_geo_loc_longitude(double longitude)
{
  imsa_set_geo_loc_req_msg_v01 *msg = (imsa_set_geo_loc_req_msg_v01 *)payload;
  msg->longitude_valid = TRUE;
  msg->longitude = longitude;
}

void ril_utf_qmiril_if::set_qmi_imsa_set_geo_loc_city(const char *city)
{
  imsa_set_geo_loc_req_msg_v01 *msg = (imsa_set_geo_loc_req_msg_v01 *)payload;
  msg->city_valid = TRUE;
  strlcpy(msg->city, city, sizeof(msg->city));
}

void ril_utf_qmiril_if::set_qmi_imsa_set_geo_loc_zipCode(const char *zipcode)
{
  imsa_set_geo_loc_req_msg_v01 *msg = (imsa_set_geo_loc_req_msg_v01 *)payload;
  msg->zipCode_valid = TRUE;
  strlcpy(msg->zipCode, zipcode, sizeof(msg->zipCode));
}

void ril_utf_qmiril_if::set_qmi_imsa_set_geo_loc_state(const char *state)
{
  imsa_set_geo_loc_req_msg_v01 *msg = (imsa_set_geo_loc_req_msg_v01 *)payload;
  msg->state_valid = TRUE;
  strlcpy(msg->state, state, sizeof(msg->state));
}

void ril_utf_qmiril_if::set_qmi_imsa_set_geo_loc_countryName(const char *countryName)
{
  imsa_set_geo_loc_req_msg_v01 *msg = (imsa_set_geo_loc_req_msg_v01 *)payload;
  msg->countryName_valid = TRUE;
  strlcpy(msg->countryName, countryName, sizeof(msg->countryName));
}

void ril_utf_qmiril_if::set_qmi_imsa_set_geo_loc_countryCode(const char *countryCode)
{
  imsa_set_geo_loc_req_msg_v01 *msg = (imsa_set_geo_loc_req_msg_v01 *)payload;
  msg->countryCode_valid = TRUE;
  strlcpy(msg->countryCode, countryCode, sizeof(msg->countryCode));
}

void ril_utf_qmiril_if::set_qmi_imsa_set_geo_loc_street(const char *street)
{
  imsa_set_geo_loc_req_msg_v01 *msg = (imsa_set_geo_loc_req_msg_v01 *)payload;
  msg->street_valid = TRUE;
  strlcpy(msg->street, street, sizeof(msg->street));
}

void ril_utf_qmiril_if::set_qmi_imsa_set_geo_loc_housenumber(const char *housenumber)
{
  imsa_set_geo_loc_req_msg_v01 *msg = (imsa_set_geo_loc_req_msg_v01 *)payload;
  msg->housenumber_valid = TRUE;
  strlcpy(msg->housenumber, housenumber, sizeof(msg->housenumber));
}

/*
  QMI_IMSA_SET_GEO_LOC_RSP_V01
 */
void ril_utf_qmiril_if::update_default_qmi_imsa_set_geo_loc_rsp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(imsa_set_geo_loc_rsp_msg_v01));
  imsa_set_geo_loc_rsp_msg_v01 *msg = (imsa_set_geo_loc_rsp_msg_v01 *)payload;
  payload_len = sizeof(imsa_set_geo_loc_rsp_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSA;
  msg_id = QMI_IMSA_SET_GEO_LOC_RSP_V01;
  msg->resp.result = result;
  msg->resp.error = error;
}

/*
  QMI_IMSA_GEO_LOC_STATUS_IND_V01
 */
void ril_utf_qmiril_if::update_default_qmi_imsa_geo_loc_status_ind()
{
  payload = (void *) malloc(sizeof(imsa_geo_loc_status_ind_msg_v01));
  imsa_geo_loc_status_ind_msg_v01 *msg = (imsa_geo_loc_status_ind_msg_v01 *)payload;
  payload_len = sizeof(imsa_geo_loc_status_ind_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_IMSA;
  msg_id = QMI_IMSA_GEO_LOC_STATUS_IND_V01;
}
void ril_utf_qmiril_if::set_qmi_imsa_geo_loc_status_ind_latitude(double val)
{
  imsa_geo_loc_status_ind_msg_v01 *msg = (imsa_geo_loc_status_ind_msg_v01 *)payload;
  msg->latitude_valid = TRUE;
  msg->latitude = val;
}
void ril_utf_qmiril_if::set_qmi_imsa_geo_loc_status_ind_longitude(double val)
{
  imsa_geo_loc_status_ind_msg_v01 *msg = (imsa_geo_loc_status_ind_msg_v01 *)payload;
  msg->longitude_valid = TRUE;
  msg->longitude = val;
}

/*========================================================
  QMI_CSVT_SET_EVENT_REPORT_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_csvt_set_event_report_req_v01(void)
{
  payload = (void*) malloc(sizeof(csvt_set_event_report_req_msg_v01));
  payload_len = sizeof(csvt_set_event_report_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_CSVT;
  msg_id = QMI_CSVT_SET_EVENT_REPORT_REQ_V01;

  csvt_set_event_report_req_msg_v01 *ptr = (csvt_set_event_report_req_msg_v01*) payload;
  ptr->report_call_events_valid = TRUE;
  ptr->report_call_events = 1;
  ptr->call_types_valid = TRUE;
  ptr->call_types = 0x0b;
}

/*========================================================
  QMI_CSVT_SET_EVENT_REPORT_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_csvt_set_event_report_resp_v01(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void*) malloc(sizeof(csvt_set_event_report_resp_msg_v01));
  payload_len = sizeof(csvt_set_event_report_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_CSVT;
  msg_id = QMI_CSVT_SET_EVENT_REPORT_RESP_V01;

  csvt_set_event_report_resp_msg_v01 *ptr = (csvt_set_event_report_resp_msg_v01*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}
/*========================================================
QMI_CSVT_SET_SUBSCRIPTION_BINDING_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_csvt_set_subscription_binding_req_v01
(csvt_bind_subscription_enum_v01 bind_subs)
{
  payload = (void*) malloc(sizeof(csvt_set_subscription_binding_req_msg_v01));
  payload_len = sizeof(csvt_set_subscription_binding_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_CSVT;
  msg_id = QMI_CSVT_SET_SUBSCRIPTION_BINDING_REQ_V01;

  csvt_set_subscription_binding_req_msg_v01 *ptr = (csvt_set_subscription_binding_req_msg_v01 *) payload;
  ptr->bind_subs = bind_subs;

}

/*========================================================
  QMI_CSVT_SET_SUBSCRIPTION_BINDING_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_csvt_set_subscription_binding_resp_v01(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void*) malloc(sizeof(csvt_set_subscription_binding_resp_msg_v01));
  payload_len = sizeof(csvt_set_subscription_binding_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_CSVT;
  msg_id = QMI_CSVT_SET_SUBSCRIPTION_BINDING_RESP_V01;

  csvt_set_subscription_binding_resp_msg_v01 *ptr = (csvt_set_subscription_binding_resp_msg_v01*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
  QMI_CSVT_ORIGINATE_CALL_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_csvt_originate_call_req
            (uint32_t instance_id,
             csvt_call_mode_type_enum_v01 call_mode,
             char *dial_string)
{
  payload = (void*) malloc(sizeof(csvt_originate_call_req_msg_v01));
  payload_len = sizeof(csvt_originate_call_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_CSVT;
  msg_id = QMI_CSVT_ORIGINATE_CALL_REQ_V01;

  csvt_originate_call_req_msg_v01 *ptr = (csvt_originate_call_req_msg_v01 *) payload;
  ptr->instance_id = instance_id;
  ptr->call_mode = call_mode;
  strlcpy (ptr->dial_string, dial_string, CSVT_MAX_DIAL_STRING_LEN_V01);
}
/*========================================================
  QMI_CSVT_ORIGINATE_CALL_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_csvt_originate_call_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void*) malloc(sizeof(csvt_originate_call_resp_msg_v01));
  payload_len = sizeof(csvt_originate_call_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_CSVT;
  msg_id = QMI_CSVT_ORIGINATE_CALL_RESP_V01;

  csvt_originate_call_resp_msg_v01 *ptr = (csvt_originate_call_resp_msg_v01*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}
/*========================================================
  QMI_CSVT_EVENT_REPORT_IND_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_csvt_event_report_ind
            (uint32_t instance_id,
             csvt_event_type_enum_v01 event_type)
{
  payload = (void*) malloc(sizeof(csvt_event_report_ind_msg_v01));
  payload_len = sizeof(csvt_event_report_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_CSVT;
  msg_id = QMI_CSVT_EVENT_REPORT_IND_V01;

  csvt_event_report_ind_msg_v01 *ptr = (csvt_event_report_ind_msg_v01*) payload;
  ptr->instance_id = instance_id;
  ptr->event_type = event_type;
}

void ril_utf_qmiril_if::set_qmi_csvt_event_report_ind_call_end_cause (uint32_t val)
{
  csvt_event_report_ind_msg_v01 *ptr = (csvt_event_report_ind_msg_v01*) payload;
  ptr->call_end_cause_valid = TRUE;
  ptr->call_end_cause = val;
}

void ril_utf_qmiril_if::set_qmi_csvt_event_report_ind_incoming_number(char *num)
{
  csvt_event_report_ind_msg_v01 *ptr = (csvt_event_report_ind_msg_v01*) payload;
  ptr->incoming_number_valid = TRUE;
  strlcpy(ptr->incoming_number, num, CSVT_MAX_INCOM_NUM_LEN_V01);
}

/*========================================================
  QMI_CSVT_END_CALL_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_csvt_end_call_req (uint32_t instance_id)
{
  payload = (void*) malloc(sizeof(csvt_end_call_req_msg_v01));
  payload_len = sizeof(csvt_end_call_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_CSVT;
  msg_id = QMI_CSVT_END_CALL_REQ_V01;

  csvt_end_call_req_msg_v01 *ptr = (csvt_end_call_req_msg_v01 *) payload;
  ptr->instance_id = instance_id;
}

/*========================================================
  QMI_CSVT_END_CALL_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_csvt_end_call_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void*) malloc(sizeof(csvt_end_call_resp_msg_v01));
  payload_len = sizeof(csvt_end_call_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_CSVT;
  msg_id = QMI_CSVT_END_CALL_RESP_V01;

  csvt_end_call_resp_msg_v01 *ptr = (csvt_end_call_resp_msg_v01*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
  QMI_CSVT_CONFIRM_CALL_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_csvt_confirm_call_req (uint32_t instance_id, uint8_t confirm_call)
{
  payload = (void*) malloc(sizeof(csvt_confirm_call_req_msg_v01));
  payload_len = sizeof(csvt_confirm_call_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_CSVT;
  msg_id = QMI_CSVT_CONFIRM_CALL_REQ_V01;

  csvt_confirm_call_req_msg_v01 *ptr = (csvt_confirm_call_req_msg_v01 *) payload;
  ptr->instance_id = instance_id;
  ptr->confirm_call = confirm_call;
}

/*========================================================
  QMI_CSVT_CONFIRM_CALL_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_csvt_confirm_call_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void*) malloc(sizeof(csvt_confirm_call_resp_msg_v01));
  payload_len = sizeof(csvt_confirm_call_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_CSVT;
  msg_id = QMI_CSVT_CONFIRM_CALL_RESP_V01;

  csvt_confirm_call_resp_msg_v01 *ptr = (csvt_confirm_call_resp_msg_v01*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
  QMI_CSVT_ANSWER_CALL_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_csvt_answer_call_req (uint32_t instance_id, uint8_t answer_call)
{
  payload = (void*) malloc(sizeof(csvt_answer_call_req_msg_v01));
  payload_len = sizeof(csvt_answer_call_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_CSVT;
  msg_id = QMI_CSVT_ANSWER_CALL_REQ_V01;

  csvt_answer_call_req_msg_v01 *ptr = (csvt_answer_call_req_msg_v01 *) payload;
  ptr->instance_id = instance_id;
  ptr->answer_call = answer_call;
}

/*========================================================
  QMI_CSVT_ANSWER_CALL_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_csvt_answer_call_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void*) malloc(sizeof(csvt_answer_call_resp_msg_v01));
  payload_len = sizeof(csvt_answer_call_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_CSVT;
  msg_id = QMI_CSVT_ANSWER_CALL_RESP_V01;

  csvt_answer_call_resp_msg_v01 *ptr = (csvt_answer_call_resp_msg_v01*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}
/*========================================================
  QMI_CSVT_RESET_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_csvt_reset_req()
{
  payload = malloc(1);          // for the char placeholder
  payload_len = 1;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_CSVT;
  msg_id = QMI_CSVT_RESET_REQ_V01;
}

/*========================================================
  QMI_CSVT_RESET_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_csvt_reset_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void*) malloc(sizeof(csvt_reset_resp_msg_v01));
  payload_len = sizeof(csvt_reset_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_CSVT;
  msg_id = QMI_CSVT_RESET_RESP_V01;

  csvt_reset_resp_msg_v01 *ptr = (csvt_reset_resp_msg_v01*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
  QMI_VOICE_SETUP_ANSWER_REQ_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_setup_answer_req(uint8_t call_id)
{
  payload = (void *) malloc (sizeof(voice_setup_answer_req_msg_v02));
  payload_len = sizeof (voice_setup_answer_req_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_SETUP_ANSWER_REQ_V02;

  voice_setup_answer_req_msg_v02 *ptr = (voice_setup_answer_req_msg_v02 *)payload;
  ptr->call_id = call_id;
}
void ril_utf_qmiril_if::set_qmi_voice_setup_answer_req_reject_setup(boolean val)
{
  voice_setup_answer_req_msg_v02 *ptr = (voice_setup_answer_req_msg_v02*) payload;
  ptr->reject_setup_valid = TRUE;
  ptr->reject_setup = val;
}

/*========================================================
  QMI_VOICE_SETUP_ANSWER_RESP_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_setup_answer_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void*) malloc(sizeof(voice_setup_answer_resp_msg_v02));
  payload_len = sizeof(voice_setup_answer_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_SETUP_ANSWER_RESP_V02;

  voice_setup_answer_resp_msg_v02 *ptr = (voice_setup_answer_resp_msg_v02*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

#ifndef UTF_TARGET_BUILD
/*========================================================
  QMI_VOICE_ADDITIONAL_CALL_INFO_IND_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_additional_call_info_ind_msg(uint8_t call_id)
{
  payload = (void *) malloc(sizeof(voice_additional_call_info_ind_msg_v02));
  payload_len = sizeof(voice_additional_call_info_ind_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_ADDITIONAL_CALL_INFO_IND_V02;

  voice_additional_call_info_ind_msg_v02 *ptr = (voice_additional_call_info_ind_msg_v02 *) payload;
  ptr->call_id = call_id;
}

uint8_t* ril_utf_qmiril_if::set_qmi_voice_additional_call_info_ind_extension_header_info(uint16_t sequence, uint32_t additional_call_info_len, uint32_t total_size)
{
  voice_additional_call_info_ind_msg_v02 *ptr = (voice_additional_call_info_ind_msg_v02 *) payload;
  ptr->extension_header_info_valid = TRUE;
  ptr->extension_header_info.sequence = sequence;
  ptr->extension_header_info.additional_call_info_len = additional_call_info_len;
  ptr->extension_header_info.total_size = total_size;

  return (ptr->extension_header_info.additional_call_info);
}
#endif

/*========================================================
  QMI_VOICE_CONF_PARTICIPANT_STATUS_INFO_IND_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_voice_conf_participant_status_info_ind_msg(uint8_t call_id, char *participant_uri)
{
  payload = (void *) malloc(sizeof(voice_conf_participant_status_info_ind_msg_v02));
  payload_len = sizeof(voice_conf_participant_status_info_ind_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_VOICE;
  msg_id = QMI_VOICE_CONF_PARTICIPANT_STATUS_INFO_IND_V02;

  voice_conf_participant_status_info_ind_msg_v02 *ptr = (voice_conf_participant_status_info_ind_msg_v02 *) payload;
  ptr->call_id = call_id;
  strlcpy(ptr->participant_uri, participant_uri, sizeof(ptr->participant_uri));
}

void ril_utf_qmiril_if::set_qmi_voice_conf_participant_status_info_ind_op_status(conf_participant_operation_enum_v02 operation, uint16_t sip_status)
{
  voice_conf_participant_status_info_ind_msg_v02 *ptr = (voice_conf_participant_status_info_ind_msg_v02 *) payload;
  ptr->op_status_valid = TRUE;
  ptr->op_status.operation = operation;
  ptr->op_status.sip_status = sip_status;
}

void ril_utf_qmiril_if::set_qmi_voice_conf_participant_status_info_ind_is_qmi_voice_transfer(uint8_t is_qmi_voice_transfer)
{
  voice_conf_participant_status_info_ind_msg_v02 *ptr = (voice_conf_participant_status_info_ind_msg_v02 *) payload;
  ptr->is_qmi_voice_transfer_valid = TRUE;
  ptr->is_qmi_voice_transfer = is_qmi_voice_transfer;
}

/*========================================================
  QMI_IMSA_GET_REGISTRATION_STATUS_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_imsa_get_registration_status_req()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSA;
  msg_id = QMI_IMSA_GET_REGISTRATION_STATUS_REQ_V01;
}

/*========================================================
  QMI_IMSA_GET_REGISTRATION_STATUS_RSP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_imsa_get_registration_status_rsp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void*) malloc(sizeof(imsa_get_registration_status_resp_msg_v01));
  payload_len = sizeof(imsa_get_registration_status_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSA;
  msg_id = QMI_IMSA_GET_REGISTRATION_STATUS_RSP_V01;

  imsa_get_registration_status_resp_msg_v01 *ptr = (imsa_get_registration_status_resp_msg_v01*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}
void ril_utf_qmiril_if::set_qmi_imsa_get_registration_status_rsp_ims_registered(uint8_t ims_registered)
{
  imsa_get_registration_status_resp_msg_v01 *ptr = (imsa_get_registration_status_resp_msg_v01*) payload;
  ptr->ims_registered_valid = TRUE;
  ptr->ims_registered = ims_registered;
}
void ril_utf_qmiril_if::set_qmi_imsa_get_registration_status_rsp_ims_registration_failure_error_code(uint16_t ims_registration_failure_error_code)
{
  imsa_get_registration_status_resp_msg_v01 *ptr = (imsa_get_registration_status_resp_msg_v01*) payload;
  ptr->ims_registration_failure_error_code_valid = TRUE;
  ptr->ims_registration_failure_error_code = ims_registration_failure_error_code;
}
void ril_utf_qmiril_if::set_qmi_imsa_get_registration_status_rsp_ims_reg_status(imsa_ims_registration_status_enum_v01 ims_reg_status)
{
  imsa_get_registration_status_resp_msg_v01 *ptr = (imsa_get_registration_status_resp_msg_v01*) payload;
  ptr->ims_reg_status_valid = TRUE;
  ptr->ims_reg_status = ims_reg_status;
}
void ril_utf_qmiril_if::set_qmi_imsa_get_registration_status_rsp_registration_network(imsa_service_rat_enum_v01 registration_network)
{
  imsa_get_registration_status_resp_msg_v01 *ptr = (imsa_get_registration_status_resp_msg_v01*) payload;
  ptr->registration_network_valid = TRUE;
  ptr->registration_network = registration_network;
}
p_associated_uri_v01 *ril_utf_qmiril_if::set_qmi_imsa_get_registration_status_rsp_uri_list(uint32_t uri_list_len)
{
  imsa_get_registration_status_resp_msg_v01 *ptr = (imsa_get_registration_status_resp_msg_v01*) payload;
  ptr->uri_list_valid = TRUE;
  ptr->uri_list_len = uri_list_len;
  return (ptr->uri_list);
}

/*========================================================
  QMI_IMSA_VoWIFI_STATUS_IND_V01
========================================================*/
imsa_vowifiprofile_status_ind_msg_v01 *
ril_utf_qmiril_if::update_default_qmi_imsa_vowifiprofile_status_ind
(
  imsa_vowifiprofile_status_enum_v01 vowifiprofile_status
)
{
  payload = (void *) malloc(sizeof(imsa_vowifiprofile_status_ind_msg_v01));
  payload_len = sizeof(imsa_vowifiprofile_status_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_IMSA;
  msg_id = QMI_IMSA_VoWIFI_STATUS_IND_V01;

  imsa_vowifiprofile_status_ind_msg_v01* ptr = (imsa_vowifiprofile_status_ind_msg_v01*) payload;
  ptr->vowifiprofile_status = vowifiprofile_status;

  return ptr;
}

/*========================================================
  QMI_IMSA_GET_VoWIFI_STATUS_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_imsa_get_vowifi_status_req(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSA;
  msg_id = QMI_IMSA_GET_VoWIFI_STATUS_REQ_V01;
}

/*========================================================
  QMI_IMSA_GET_VoWIFI_STATUSRSP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_imsa_get_vowifi_status_resp(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (imsa_get_vowifi_status_resp_msg_v01 *)malloc(sizeof(imsa_get_vowifi_status_resp_msg_v01));
  payload_len = sizeof(imsa_get_vowifi_status_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSA;
  msg_id = QMI_IMSA_GET_VoWIFI_STATUSRSP_V01;

  imsa_get_vowifi_status_resp_msg_v01 *ptr = (imsa_get_vowifi_status_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_imsa_get_vowifi_status_resp_vowifiprofile_status(imsa_vowifiprofile_status_enum_v01 vowifiprofile_status)
{
  imsa_get_vowifi_status_resp_msg_v01 *ptr = (imsa_get_vowifi_status_resp_msg_v01*) payload;
  ptr->vowifiprofile_status_valid = TRUE;
  ptr->vowifiprofile_status = vowifiprofile_status;
}

/*========================================================
  QMI_IMSA_GET_VIRTUAL_LINE_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_imsa_get_virtual_line_req(const char *msisdn)
{
  payload = (imsa_get_virtual_line_req_msg_v01 *)qcril_malloc(sizeof(imsa_get_virtual_line_req_msg_v01));
  payload_len = sizeof(imsa_get_virtual_line_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSA;
  msg_id = QMI_IMSA_GET_VIRTUAL_LINE_REQ_V01;
  imsa_get_virtual_line_req_msg_v01 *ptr = (imsa_get_virtual_line_req_msg_v01 *) payload;
  strlcpy(ptr->msisdn, msisdn, sizeof(ptr->msisdn));
}

/*========================================================
  QMI_IMSA_GET_VIRTUAL_LINE_RSP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_imsa_get_virtual_line_resp(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (imsa_get_virtual_line_rsp_msg_v01 *)qcril_malloc(sizeof(imsa_get_virtual_line_rsp_msg_v01));
  payload_len = sizeof(imsa_get_virtual_line_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSA;
  msg_id = QMI_IMSA_GET_VIRTUAL_LINE_RSP_V01;

  imsa_get_virtual_line_rsp_msg_v01 *ptr = (imsa_get_virtual_line_rsp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
p_associated_uri_v01 *ril_utf_qmiril_if::set_qmi_imsa_get_virtual_line_resp_virtual_line_info(uint32_t uri_list_len)
{
  imsa_get_virtual_line_rsp_msg_v01 *ptr = (imsa_get_virtual_line_rsp_msg_v01*) payload;
  ptr->virtual_line_info_valid = TRUE;
  ptr->virtual_line_info.uri_list_len = uri_list_len;
  return (ptr->virtual_line_info.uri_list);
}


/*========================================================
  QMI_WMS_GET_TRANSPORT_NW_REG_INFO_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_get_transport_nw_reg_info_req(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_GET_TRANSPORT_NW_REG_INFO_REQ_V01;
}

/*========================================================
  QMI_WMS_GET_TRANSPORT_NW_REG_INFO_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_get_transport_nw_reg_info_resp(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (wms_get_transport_nw_reg_resp_msg_v01 *)malloc(sizeof(wms_get_transport_nw_reg_resp_msg_v01));
  payload_len = sizeof(wms_get_transport_nw_reg_resp_msg_v01);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_GET_TRANSPORT_NW_REG_INFO_RESP_V01;

  wms_get_transport_nw_reg_resp_msg_v01 *ptr = (wms_get_transport_nw_reg_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

void ril_utf_qmiril_if::set_qmi_wms_get_transport_nw_reg_status(uint8_t transport_nw_reg_status_valid, wms_transport_nw_reg_status_enum_v01 transport_nw_reg_status)
{
  wms_get_transport_nw_reg_resp_msg_v01 *ptr = (wms_get_transport_nw_reg_resp_msg_v01 *)payload;

  ptr->transport_nw_reg_status_valid = transport_nw_reg_status_valid;
  ptr->transport_nw_reg_status = transport_nw_reg_status;
}

/*========================================================
  QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_get_transport_layer_info_req(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ_V01;
}

/*========================================================
  QMI_WMS_GET_TRANSPORT_LAYER_INFO_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_wms_get_transport_layer_info_resp(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (wms_get_transport_layer_resp_msg_v01 *)malloc(sizeof(wms_get_transport_layer_resp_msg_v01));
  payload_len = sizeof(wms_get_transport_layer_resp_msg_v01);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_WMS;
  msg_id = QMI_WMS_GET_TRANSPORT_LAYER_INFO_RESP_V01;

  wms_get_transport_layer_resp_msg_v01 *ptr = (wms_get_transport_layer_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

void ril_utf_qmiril_if::set_qmi_wms_get_transport_layer_register_ind_info(uint8_t registered_ind_valid, uint8_t registered_ind)
{
  wms_get_transport_layer_resp_msg_v01 *ptr = (wms_get_transport_layer_resp_msg_v01 *)payload;

  ptr->registered_ind_valid = registered_ind_valid;
  ptr->registered_ind = registered_ind;
}

void ril_utf_qmiril_if::set_qmi_wms_get_transport_layer_info_type_cap(uint8_t transport_layer_info_valid, wms_transport_type_enum_v01 transport_type, wms_transport_capability_enum_v01 transport_cap)
{
  wms_get_transport_layer_resp_msg_v01 *ptr = (wms_get_transport_layer_resp_msg_v01 *)payload;

  ptr->transport_layer_info_valid = transport_layer_info_valid;
  ptr->transport_layer_info.transport_type = transport_type;
  ptr->transport_layer_info.transport_cap = transport_cap;
}

/*========================================================
  QMI_NAS_GET_SUBSCRIPTION_INFO_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_subscription_info_req_msg()
{
  payload = NULL;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_SUBSCRIPTION_INFO_REQ_MSG_V01;
}

/*========================================================
  QMI_NAS_GET_SUBSCRIPTION_INFO_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_subscription_info_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_get_subscription_info_resp_msg_v01));
  payload_len = sizeof(nas_get_subscription_info_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_SUBSCRIPTION_INFO_RESP_MSG_V01;

  nas_get_subscription_info_resp_msg_v01 *ptr = (nas_get_subscription_info_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
  return;
}

void ril_utf_qmiril_if::set_priority_subs_qmi_nas_get_subscription_info_resp_msg(uint8_t is_priority_subs_valid, nas_is_priority_subs_enum_v01 is_priority_subs)
{
  nas_get_subscription_info_resp_msg_v01 *ptr = (nas_get_subscription_info_resp_msg_v01*)payload;
  ptr->is_priority_subs_valid = is_priority_subs_valid;
  ptr->is_priority_subs = is_priority_subs;
}

void ril_utf_qmiril_if::set_active_sub_qmi_nas_get_subscription_info_resp_msg(uint8_t is_active_valid, nas_active_subs_info_enum_v01 is_active)
{
  nas_get_subscription_info_resp_msg_v01 *ptr = (nas_get_subscription_info_resp_msg_v01*)payload;
  ptr->is_active_valid = is_active_valid;
  ptr->is_active = is_active;
}

void ril_utf_qmiril_if::set_default_data_sub_qmi_nas_get_subscription_info_resp_msg(uint8_t is_default_data_subs_valid, uint8_t is_default_data_subs)
{
  nas_get_subscription_info_resp_msg_v01 *ptr = (nas_get_subscription_info_resp_msg_v01*)payload;
  ptr->is_default_data_subs_valid = is_default_data_subs_valid;
  ptr->is_default_data_subs = is_default_data_subs;
}

void ril_utf_qmiril_if::set_voice_system_id_qmi_nas_get_subscription_info_resp_msg(uint8_t voice_system_id_valid, uint32_t voice_system_id)
{
  nas_get_subscription_info_resp_msg_v01 *ptr = (nas_get_subscription_info_resp_msg_v01*)payload;
  ptr->voice_system_id_valid = voice_system_id_valid;
  ptr->voice_system_id = voice_system_id;
}

void ril_utf_qmiril_if::set_lte_voice_system_id_qmi_nas_get_subscription_info_resp_msg(uint8_t lte_voice_system_id_valid, uint32_t lte_voice_system_id)
{
  nas_get_subscription_info_resp_msg_v01 *ptr = (nas_get_subscription_info_resp_msg_v01*)payload;
  ptr->lte_voice_system_id_valid = lte_voice_system_id_valid;
  ptr->lte_voice_system_id = lte_voice_system_id;
}

void ril_utf_qmiril_if::set_wlan_voice_system_id_qmi_nas_get_subscription_info_resp_msg(uint8_t wlan_voice_system_id_valid, uint32_t wlan_voice_system_id)
{
  nas_get_subscription_info_resp_msg_v01 *ptr = (nas_get_subscription_info_resp_msg_v01*)payload;
  ptr->wlan_voice_system_id_valid = wlan_voice_system_id_valid;
  ptr->wlan_voice_system_id = wlan_voice_system_id;
}
/*========================================================
  QMI_UIM_EVENT_REG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_uim_event_reg_req(uint32_t event_mask)
{
  payload = (void *) malloc (sizeof(uim_event_reg_req_msg_v01));
  payload_len = sizeof (uim_event_reg_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_EVENT_REG_REQ_V01;

  uim_event_reg_req_msg_v01 *ptr = (uim_event_reg_req_msg_v01*) payload;
  ptr->event_mask = event_mask;
}

/*========================================================
  QMI_UIM_EVENT_REG_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_uim_event_reg_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void*) malloc(sizeof(uim_event_reg_resp_msg_v01));
  payload_len = sizeof(uim_event_reg_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_EVENT_REG_RESP_V01;

  uim_event_reg_resp_msg_v01 *ptr = (uim_event_reg_resp_msg_v01*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}
void ril_utf_qmiril_if::set_qmi_uim_event_req_resp_event_mask(uint32_t event_mask)
{
  uim_event_reg_resp_msg_v01 *ptr = (uim_event_reg_resp_msg_v01*) payload;
  ptr->event_mask_valid = TRUE;
  ptr->event_mask = event_mask;
}

/*========================================================
  QMI_UIM_GET_CONFIGURATION_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_uim_get_configuration_req()
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_GET_CONFIGURATION_REQ_V01;
}

/*========================================================
  QMI_UIM_GET_CONFIGURATION_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_uim_get_configuration_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void*) malloc(sizeof(uim_get_configuration_resp_msg_v01));
  payload_len = sizeof(uim_get_configuration_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_GET_CONFIGURATION_RESP_V01;

  uim_get_configuration_resp_msg_v01 *ptr = (uim_get_configuration_resp_msg_v01*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
  QMI_UIM_GET_CARD_STATUS_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_uim_get_card_status_req()
{
  payload = (void *) malloc (sizeof(uim_get_card_status_req_msg_v01));
  payload_len = sizeof(uim_get_card_status_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_GET_CARD_STATUS_REQ_V01;
}
void ril_utf_qmiril_if::set_qmi_uim_get_card_status_req_extended_card_status(uint8_t extended_card_status)
{
  uim_get_card_status_req_msg_v01 *ptr = (uim_get_card_status_req_msg_v01*) payload;
  ptr->extended_card_status_valid = TRUE;
  ptr->extended_card_status = extended_card_status;
}

/*========================================================
  QMI_UIM_GET_CARD_STATUS_RESP_V01
========================================================*/
uim_get_card_status_resp_msg_v01* ril_utf_qmiril_if::update_default_qmi_uim_get_card_status_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void*) malloc(sizeof(uim_get_card_status_resp_msg_v01));
  payload_len = sizeof(uim_get_card_status_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_GET_CARD_STATUS_RESP_V01;

  uim_get_card_status_resp_msg_v01 *ptr = (uim_get_card_status_resp_msg_v01*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;

  return ptr;
}
/*========================================================
  QMI_UIM_STATUS_CHANGE_IND_V01
  ========================================================*/
uim_status_change_ind_msg_v01* ril_utf_qmiril_if::update_default_qmi_uim_status_change_ind_v01()
{
  payload = (void *) malloc (sizeof(uim_status_change_ind_msg_v01));
  payload_len = sizeof(uim_status_change_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_STATUS_CHANGE_IND_V01;
  return (uim_status_change_ind_msg_v01 *)payload;
}


/*========================================================
  QMI_UIM_GET_SERVICE_STATUS_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_uim_get_service_status_req(uim_session_information_type_v01 session_information, uint32_t mask)
{
  payload = (void *) malloc (sizeof(uim_get_service_status_req_msg_v01));
  payload_len = sizeof(uim_get_service_status_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_GET_SERVICE_STATUS_REQ_V01;

  uim_get_service_status_req_msg_v01 *ptr = (uim_get_service_status_req_msg_v01*) payload;
  ptr->mask = mask;
  ptr->session_information = session_information;
}
/*========================================================
  QMI_UIM_GET_SERVICE_STATUS_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_uim_get_service_status_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void*) malloc(sizeof(uim_get_service_status_resp_msg_v01));
  payload_len = sizeof(uim_get_service_status_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_GET_SERVICE_STATUS_RESP_V01;

  uim_get_service_status_resp_msg_v01 *ptr = (uim_get_service_status_resp_msg_v01*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;

  //return ptr;
}

void ril_utf_qmiril_if::set_qmi_uim_get_service_status_resp_fdn_status(uim_fdn_status_enum_v01 fdn_status)
{
  uim_get_service_status_resp_msg_v01 *ptr = (uim_get_service_status_resp_msg_v01*) payload;
  ptr->fdn_status_valid = TRUE;
  ptr->fdn_status = fdn_status;
}

/*========================================================
  QMI_UIM_GET_FILE_ATTRIBUTES_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_uim_get_file_attributes_req
  (uim_session_information_type_v01 session_information, uim_file_id_type_v01 file_id)
{
  payload = (void *) malloc (sizeof(uim_get_file_attributes_req_msg_v01));
  payload_len = sizeof(uim_get_file_attributes_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_GET_FILE_ATTRIBUTES_REQ_V01;

  uim_get_file_attributes_req_msg_v01 *ptr = (uim_get_file_attributes_req_msg_v01*) payload;
  ptr->session_information = session_information;
  ptr->file_id = file_id;
}
/*========================================================
  QMI_UIM_GET_FILE_ATTRIBUTES_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_uim_get_file_attributes_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void*) malloc(sizeof(uim_get_file_attributes_resp_msg_v01));
  payload_len = sizeof(uim_get_file_attributes_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_GET_FILE_ATTRIBUTES_RESP_V01;

  uim_get_file_attributes_resp_msg_v01 *ptr = (uim_get_file_attributes_resp_msg_v01*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}
void ril_utf_qmiril_if::set_qmi_uim_get_file_attributes_resp_card_result(uim_card_result_type_v01 card_result)
{
  uim_get_file_attributes_resp_msg_v01 *ptr = (uim_get_file_attributes_resp_msg_v01*) payload;
  ptr->card_result_valid = TRUE;
  ptr->card_result = card_result;
}
void ril_utf_qmiril_if::set_qmi_uim_get_file_attribues_resp_file_status(uim_file_status_type_v01 file_status)
{
  uim_get_file_attributes_resp_msg_v01 *ptr = (uim_get_file_attributes_resp_msg_v01*) payload;
  ptr->file_status_valid;
  ptr->file_status = file_status;
}
uim_file_attributes_type_v01* ril_utf_qmiril_if::set_qmi_uim_get_file_attributes_resp_file_attributes()
{
  uim_get_file_attributes_resp_msg_v01 *ptr = (uim_get_file_attributes_resp_msg_v01*) payload;
  ptr->file_attributes_valid = TRUE;
  return &(ptr->file_attributes);
}

/*========================================================
  QMI_UIM_REFRESH_REGISTER_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_uim_get_file_attributes_req
  (uim_session_information_type_v01 session_information, uim_register_refresh_type_v01 register_refresh)
{
  payload = (void *) malloc (sizeof(uim_refresh_register_req_msg_v01));
  payload_len = sizeof(uim_refresh_register_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_REFRESH_REGISTER_REQ_V01;

  uim_refresh_register_req_msg_v01 *ptr = (uim_refresh_register_req_msg_v01*) payload;
  ptr->session_information = session_information;
  ptr->register_refresh = register_refresh;
}
/*========================================================
  QMI_UIM_REFRESH_REGISTER_REQ_V01 (change function name)
========================================================*/
uim_refresh_register_req_msg_v01 *ril_utf_qmiril_if::update_default_qmi_uim_refresh_register_req()
{
  payload = (void *) malloc (sizeof(uim_refresh_register_req_msg_v01));
  payload_len = sizeof(uim_refresh_register_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_REFRESH_REGISTER_REQ_V01;

  uim_refresh_register_req_msg_v01 *ptr = (uim_refresh_register_req_msg_v01*) payload;
  return ptr;
}
/*========================================================
  QMI_UIM_REFRESH_REGISTER_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_uim_refresh_register_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void*) malloc(sizeof(uim_refresh_register_resp_msg_v01));
  payload_len = sizeof(uim_refresh_register_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_REFRESH_REGISTER_RESP_V01;

  uim_refresh_register_resp_msg_v01 *ptr = (uim_refresh_register_resp_msg_v01*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
  QMI_UIM_READ_RECORD_REQ_V01
========================================================*/
uim_read_record_req_msg_v01* ril_utf_qmiril_if::update_default_qmi_uim_read_record_reg()
{
  payload = (void *) malloc (sizeof(uim_read_record_req_msg_v01));
  payload_len = sizeof(uim_read_record_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_READ_RECORD_REQ_V01;

  uim_read_record_req_msg_v01 *ptr = (uim_read_record_req_msg_v01*) payload;
  return ptr;
}
/*========================================================
  QMI_UIM_READ_RECORD_RESP_V01
========================================================*/
uim_read_record_resp_msg_v01* ril_utf_qmiril_if::update_default_qmi_uim_read_record_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void*) malloc(sizeof(uim_read_record_resp_msg_v01));
  payload_len = sizeof(uim_read_record_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_READ_RECORD_RESP_V01;

  uim_read_record_resp_msg_v01 *ptr = (uim_read_record_resp_msg_v01*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;

  return ptr;
}
/*========================================================
  QMI_UIM_READ_TRANSPARENT_REQ_V01
========================================================*/
uim_read_transparent_req_msg_v01* ril_utf_qmiril_if::update_default_qmi_uim_read_transparent_reg()
{
  payload = (void *) malloc (sizeof(uim_read_transparent_req_msg_v01));
  payload_len = sizeof(uim_read_transparent_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_READ_TRANSPARENT_REQ_V01;

  uim_read_transparent_req_msg_v01 *ptr = (uim_read_transparent_req_msg_v01*) payload;
  return ptr;
}
/*========================================================
  QMI_UIM_READ_TRANSPARENT_RESP_V01
========================================================*/
uim_read_transparent_resp_msg_v01* ril_utf_qmiril_if::update_default_qmi_uim_read_transparent_resp
  (qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void*) malloc(sizeof(uim_read_transparent_resp_msg_v01));
  payload_len = sizeof(uim_read_transparent_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_READ_TRANSPARENT_RESP_V01;

  uim_read_transparent_resp_msg_v01 *ptr = (uim_read_transparent_resp_msg_v01*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;

  return ptr;
}

/*========================================================
  QMI_UIM_GET_LABEL_REQ_V01
========================================================*/
uim_get_label_req_msg_v01* ril_utf_qmiril_if::update_default_qmi_uim_get_label_req()
{
  payload = (void*) malloc(sizeof(uim_get_label_req_msg_v01));
  payload_len = sizeof(uim_get_label_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_GET_LABEL_REQ_V01;

  uim_get_label_req_msg_v01 *ptr = (uim_get_label_req_msg_v01*) payload;

  return ptr;
}

/*========================================================
  QMI_UIM_GET_LABEL_RESP_V01
========================================================*/
uim_get_label_resp_msg_v01* ril_utf_qmiril_if::update_default_qmi_uim_get_label_resp()
{
  payload = (void*) malloc(sizeof(uim_get_label_resp_msg_v01));
  payload_len = sizeof(uim_get_label_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_GET_LABEL_RESP_V01;

  uim_get_label_resp_msg_v01 *ptr = (uim_get_label_resp_msg_v01*) payload;

  return ptr;
}

/*========================================================
  QMI_UIM_OPEN_LOGICAL_CHANNEL_REQ_V01
========================================================*/
uim_open_logical_channel_req_msg_v01* ril_utf_qmiril_if::update_default_qmi_uim_open_channel_req()
{
  payload = malloc(sizeof(uim_open_logical_channel_req_msg_v01));
  payload_len = sizeof(uim_open_logical_channel_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_OPEN_LOGICAL_CHANNEL_REQ_V01 ;

  return (uim_open_logical_channel_req_msg_v01*)payload;
}

/*========================================================
  QMI_UIM_OPEN_LOGICAL_CHANNEL_RESP_V01
========================================================*/
uim_open_logical_channel_resp_msg_v01* ril_utf_qmiril_if::update_default_qmi_uim_open_channel_resp()
{
  payload = malloc(sizeof(uim_open_logical_channel_resp_msg_v01));
  payload_len = sizeof(uim_open_logical_channel_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_OPEN_LOGICAL_CHANNEL_RESP_V01;

  return (uim_open_logical_channel_resp_msg_v01*)payload;
}

/*========================================================
  QMI_UIM_VERIFY_PIN_REQ_V01
========================================================*/
uim_verify_pin_req_msg_v01* ril_utf_qmiril_if::update_default_qmi_uim_verify_pin_req()
{
   payload = malloc(sizeof(uim_verify_pin_req_msg_v01));
   payload_len = sizeof(uim_verify_pin_req_msg_v01);
   memset(payload, 0x0, payload_len);
   payload_type = ril_utf_qmi_request;
   service_id = QMI_UIM;
   msg_id = QMI_UIM_VERIFY_PIN_REQ_V01;

   return (uim_verify_pin_req_msg_v01*) payload;
}

/*========================================================
  QMI_UIM_VERIFY_PIN_RESP_V01
========================================================*/
uim_verify_pin_resp_msg_v01* ril_utf_qmiril_if::update_default_qmi_uim_verify_pin_resp()
{
   payload = malloc(sizeof(uim_verify_pin_resp_msg_v01));
   payload_len = sizeof(uim_verify_pin_resp_msg_v01);
   memset(payload, 0x0, payload_len);
   payload_type = ril_utf_qmi_response;
   service_id = QMI_UIM;
   msg_id = QMI_UIM_VERIFY_PIN_RESP_V01;

   return (uim_verify_pin_resp_msg_v01*) payload;
}


/*========================================================
  QMI_UIM_UNBLOCK_PIN_REQ_V01
========================================================*/
uim_unblock_pin_req_msg_v01* ril_utf_qmiril_if::update_default_qmi_uim_unblock_pin_req()
{
   payload = malloc(sizeof(uim_unblock_pin_req_msg_v01));
   payload_len = sizeof(uim_unblock_pin_req_msg_v01);
   memset(payload, 0x0, payload_len);
   payload_type = ril_utf_qmi_request;
   service_id = QMI_UIM;
   msg_id = QMI_UIM_UNBLOCK_PIN_REQ_V01;

   return (uim_unblock_pin_req_msg_v01*) payload;
}

/*========================================================
  QMI_UIM_VERIFY_PIN_RESP_V01
========================================================*/
uim_unblock_pin_resp_msg_v01* ril_utf_qmiril_if::update_default_qmi_uim_unblock_pin_resp()
{
   payload = malloc(sizeof(uim_unblock_pin_resp_msg_v01));
   payload_len = sizeof(uim_unblock_pin_resp_msg_v01);
   memset(payload, 0x0, payload_len);
   payload_type = ril_utf_qmi_response;
   service_id = QMI_UIM;
   msg_id = QMI_UIM_UNBLOCK_PIN_RESP_V01;

   return (uim_unblock_pin_resp_msg_v01*) payload;
}

/*========================================================
  QMI_UIM_DEPERSONALIZATION_REQ_V01
========================================================*/
uim_depersonalization_req_msg_v01* ril_utf_qmiril_if::update_default_qmi_uim_depersonalization_req()
{
   payload = malloc(sizeof(uim_depersonalization_req_msg_v01));
   payload_len = sizeof(uim_depersonalization_req_msg_v01);
   memset(payload, 0x0, payload_len);
   payload_type = ril_utf_qmi_request;
   service_id = QMI_UIM;
   msg_id = QMI_UIM_DEPERSONALIZATION_REQ_V01;

   return (uim_depersonalization_req_msg_v01*) payload;
}

/*========================================================
  QMI_UIM_DEPERSONALIZATION_RESP_V01
========================================================*/
uim_depersonalization_resp_msg_v01* ril_utf_qmiril_if::update_default_qmi_uim_depersonalization_resp()
{
   payload = malloc(sizeof(uim_depersonalization_resp_msg_v01));
   payload_len = sizeof(uim_depersonalization_resp_msg_v01);
   memset(payload, 0x0, payload_len);
   payload_type = ril_utf_qmi_response;
   service_id = QMI_UIM;
   msg_id = QMI_UIM_DEPERSONALIZATION_RESP_V01;

   return (uim_depersonalization_resp_msg_v01*) payload;
}
/*
#define QMI_UIM_RESET_REQ_V01 0x0000
#define QMI_UIM_RESET_RESP_V01 0x0000
#define QMI_UIM_GET_SUPPORTED_MSGS_REQ_V01 0x001E
#define QMI_UIM_GET_SUPPORTED_MSGS_RESP_V01 0x001E
#define QMI_UIM_GET_SUPPORTED_FIELDS_REQ_V01 0x001F
#define QMI_UIM_GET_SUPPORTED_FIELDS_RESP_V01 0x001F
#define QMI_UIM_READ_TRANSPARENT_REQ_V01 0x0020
#define QMI_UIM_READ_TRANSPARENT_RESP_V01 0x0020
#define QMI_UIM_READ_TRANSPARENT_IND_V01 0x0020
#define QMI_UIM_READ_RECORD_IND_V01 0x0021
#define QMI_UIM_WRITE_TRANSPARENT_REQ_V01 0x0022
#define QMI_UIM_WRITE_TRANSPARENT_RESP_V01 0x0022
#define QMI_UIM_WRITE_TRANSPARENT_IND_V01 0x0022
#define QMI_UIM_WRITE_RECORD_REQ_V01 0x0023
#define QMI_UIM_WRITE_RECORD_RESP_V01 0x0023
#define QMI_UIM_WRITE_RECORD_IND_V01 0x0023
#define QMI_UIM_GET_FILE_ATTRIBUTES_IND_V01 0x0024
#define QMI_UIM_SET_PIN_PROTECTION_REQ_V01 0x0025
#define QMI_UIM_SET_PIN_PROTECTION_RESP_V01 0x0025
#define QMI_UIM_SET_PIN_PROTECTION_IND_V01 0x0025
#define QMI_UIM_VERIFY_PIN_REQ_V01 0x0026
#define QMI_UIM_VERIFY_PIN_RESP_V01 0x0026
#define QMI_UIM_VERIFY_PIN_IND_V01 0x0026
#define QMI_UIM_UNBLOCK_PIN_REQ_V01 0x0027
#define QMI_UIM_UNBLOCK_PIN_RESP_V01 0x0027
#define QMI_UIM_UNBLOCK_PIN_IND_V01 0x0027
#define QMI_UIM_CHANGE_PIN_REQ_V01 0x0028
#define QMI_UIM_CHANGE_PIN_RESP_V01 0x0028
#define QMI_UIM_CHANGE_PIN_IND_V01 0x0028
#define QMI_UIM_DEPERSONALIZATION_REQ_V01 0x0029
#define QMI_UIM_DEPERSONALIZATION_RESP_V01 0x0029
#define QMI_UIM_REFRESH_OK_REQ_V01 0x002B
#define QMI_UIM_REFRESH_OK_RESP_V01 0x002B
#define QMI_UIM_REFRESH_COMPLETE_REQ_V01 0x002C
#define QMI_UIM_REFRESH_COMPLETE_RESP_V01 0x002C
#define QMI_UIM_REFRESH_GET_LAST_EVENT_REQ_V01 0x002D
#define QMI_UIM_REFRESH_GET_LAST_EVENT_RESP_V01 0x002D
#define QMI_UIM_POWER_DOWN_REQ_V01 0x0030
#define QMI_UIM_POWER_DOWN_RESP_V01 0x0030
#define QMI_UIM_POWER_UP_REQ_V01 0x0031
#define QMI_UIM_POWER_UP_RESP_V01 0x0031
#define QMI_UIM_STATUS_CHANGE_IND_V01 0x0032
#define QMI_UIM_REFRESH_IND_V01 0x0033
#define QMI_UIM_AUTHENTICATE_REQ_V01 0x0034
#define QMI_UIM_AUTHENTICATE_RESP_V01 0x0034
#define QMI_UIM_AUTHENTICATE_IND_V01 0x0034
#define QMI_UIM_CLOSE_SESSION_REQ_V01 0x0035
#define QMI_UIM_CLOSE_SESSION_RESP_V01 0x0035
#define QMI_UIM_SET_SERVICE_STATUS_REQ_V01 0x0037
#define QMI_UIM_SET_SERVICE_STATUS_RESP_V01 0x0037
#define QMI_UIM_CHANGE_PROVISIONING_SESSION_REQ_V01 0x0038
#define QMI_UIM_CHANGE_PROVISIONING_SESSION_RESP_V01 0x0038
#define QMI_UIM_GET_LABEL_REQ_V01 0x0039
#define QMI_UIM_GET_LABEL_RESP_V01 0x0039
#define QMI_UIM_GET_CONFIGURATION_REQ_V01 0x003A
#define QMI_UIM_GET_CONFIGURATION_RESP_V01 0x003A
#define QMI_UIM_SEND_APDU_REQ_V01 0x003B
#define QMI_UIM_SEND_APDU_RESP_V01 0x003B
#define QMI_UIM_SEND_APDU_IND_V01 0x003B
#define QMI_UIM_SAP_CONNECTION_REQ_V01 0x003C
#define QMI_UIM_SAP_CONNECTION_RESP_V01 0x003C
#define QMI_UIM_SAP_REQUEST_REQ_V01 0x003D
#define QMI_UIM_SAP_REQUEST_RESP_V01 0x003D
#define QMI_UIM_SAP_CONNECTION_IND_V01 0x003E
#define QMI_UIM_LOGICAL_CHANNEL_REQ_V01 0x003F
#define QMI_UIM_LOGICAL_CHANNEL_RESP_V01 0x003F
#define QMI_UIM_SUBSCRIPTION_OK_REQ_V01 0x0040
#define QMI_UIM_SUBSCRIPTION_OK_RESP_V01 0x0040
#define QMI_UIM_GET_ATR_REQ_V01 0x0041
#define QMI_UIM_GET_ATR_RESP_V01 0x0041
#define QMI_UIM_OPEN_LOGICAL_CHANNEL_REQ_V01 0x0042
#define QMI_UIM_OPEN_LOGICAL_CHANNEL_RESP_V01 0x0042
#define QMI_UIM_SESSION_CLOSED_IND_V01 0x0043
#define QMI_UIM_REFRESH_REGISTER_ALL_REQ_V01 0x0044
#define QMI_UIM_REFRESH_REGISTER_ALL_RESP_V01 0x0044
#define QMI_UIM_SET_FILE_STATUS_REQ_V01 0x0045
#define QMI_UIM_SET_FILE_STATUS_RESP_V01 0x0045
#define QMI_UIM_SWITCH_SLOT_REQ_V01 0x0046
#define QMI_UIM_SWITCH_SLOT_RESP_V01 0x0046
#define QMI_UIM_GET_SLOTS_STATUS_REQ_V01 0x0047
#define QMI_UIM_GET_SLOTS_STATUS_RESP_V01 0x0047
#define QMI_UIM_SLOT_STATUS_CHANGE_IND_V01 0x0048
#define QMI_UIM_READ_TRANSPARENT_LONG_IND_V01 0x0049
#define QMI_UIM_SIM_BUSY_STATUS_IND_V01 0x004A
#define QMI_UIM_GET_PLMN_NAME_TABLE_INFO_REQ_V01 0x004B
#define QMI_UIM_GET_PLMN_NAME_TABLE_INFO_RESP_V01 0x004B
#define QMI_UIM_PERSONALIZATION_REQ_V01 0x004C
#define QMI_UIM_PERSONALIZATION_RESP_V01 0x004C
#define QMI_UIM_INCREASE_REQ_V01 0x004D
#define QMI_UIM_INCREASE_RESP_V01 0x004D
#define QMI_UIM_INCREASE_IND_V01 0x004D
*/

/*========================================================
  QMI_CAT_GET_CONFIGURATION_REQ_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_cat_get_configuration_req()
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_CAT;
  msg_id = QMI_CAT_GET_CONFIGURATION_REQ_V02;
}

/*========================================================
  QMI_CAT_GET_CONFIGURATION_RESP_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_cat_get_configuration_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void*) malloc(sizeof(cat_get_configuration_resp_msg_v02));
  payload_len = sizeof(cat_get_configuration_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_CAT;
  msg_id = QMI_CAT_GET_CONFIGURATION_RESP_V02;

  cat_get_configuration_resp_msg_v02 *ptr = (cat_get_configuration_resp_msg_v02*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}


/*========================================================
  QMI_CAT_SET_EVENT_REPORT_REQ_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_cat_set_event_report_req()
{
  payload = (void *) malloc (sizeof(cat_set_event_report_req_msg_v02));
  payload_len = sizeof(cat_set_event_report_req_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_CAT;
  msg_id = QMI_CAT_SET_EVENT_REPORT_REQ_V02;
}
void ril_utf_qmiril_if::set_qmi_cat_set_event_report_req_pc_evt_report_req_mask(uint32_t pc_evt_report_req_mask)
{
  cat_set_event_report_req_msg_v02 *ptr = (cat_set_event_report_req_msg_v02*) payload;
  ptr->pc_evt_report_req_mask_valid = TRUE;
  ptr->pc_evt_report_req_mask = pc_evt_report_req_mask;
}
void ril_utf_qmiril_if::set_qmi_cat_set_event_report_req_pc_dec_evt_report_req_mask(uint32_t pc_dec_evt_report_req_mask)
{
  cat_set_event_report_req_msg_v02 *ptr = (cat_set_event_report_req_msg_v02*) payload;
  ptr->pc_dec_evt_report_req_mask_valid = TRUE;
  ptr->pc_dec_evt_report_req_mask = pc_dec_evt_report_req_mask;
}
void ril_utf_qmiril_if::set_qmi_cat_set_event_report_req_slot_mask(cat_set_event_report_slot_mask_v02 slot_mask)
{
  cat_set_event_report_req_msg_v02 *ptr = (cat_set_event_report_req_msg_v02*) payload;
  ptr->slot_mask_valid = TRUE;
  ptr->slot_mask = slot_mask;
}
void ril_utf_qmiril_if::set_qmi_cat_set_event_report_req_pc_full_func_evt_report_req_mask(cat_set_event_report_full_func_mask_v02 pc_full_func_evt_report_req_mask)
{
  cat_set_event_report_req_msg_v02 *ptr = (cat_set_event_report_req_msg_v02*) payload;
  ptr->pc_full_func_evt_report_req_mask_valid = TRUE;
  ptr->pc_full_func_evt_report_req_mask = pc_full_func_evt_report_req_mask;
}

/*========================================================
  QMI_CAT_SET_EVENT_REPORT_RESP_V02
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_cat_set_event_report_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void*) malloc(sizeof(cat_set_event_report_resp_msg_v02));
  payload_len = sizeof(cat_set_event_report_resp_msg_v02);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_CAT;
  msg_id = QMI_CAT_SET_EVENT_REPORT_RESP_V02;

  cat_set_event_report_resp_msg_v02 *ptr = (cat_set_event_report_resp_msg_v02*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;

  //return ptr;
}
void ril_utf_qmiril_if::set_qmi_cat_set_event_report_resp_pc_event_report_req_err_mask(uint32_t pc_evt_report_req_err_mask)
{
  cat_set_event_report_resp_msg_v02 *ptr = (cat_set_event_report_resp_msg_v02*) payload;
  ptr->pc_evt_report_req_err_mask_valid = TRUE;
  ptr->pc_evt_report_req_err_mask = pc_evt_report_req_err_mask;
}
void ril_utf_qmiril_if::set_qmi_cat_set_event_report_resp_pc_dec_evt_report_req_err_mask(uint32_t pc_dec_evt_report_req_err_mask)
{
  cat_set_event_report_resp_msg_v02 *ptr = (cat_set_event_report_resp_msg_v02*) payload;
  ptr->pc_dec_evt_report_req_err_mask_valid = TRUE;
  ptr->pc_dec_evt_report_req_err_mask = pc_dec_evt_report_req_err_mask;
}
void ril_utf_qmiril_if::set_qmi_cat_set_event_report_resp_pc_full_func_evt_report_err_mask(cat_set_event_report_full_func_mask_v02 pc_full_func_evt_report_err_mask)
{
  cat_set_event_report_resp_msg_v02 *ptr = (cat_set_event_report_resp_msg_v02*) payload;
  ptr->pc_full_func_evt_report_err_mask_valid = TRUE;
  ptr->pc_full_func_evt_report_err_mask = pc_full_func_evt_report_err_mask;
}

/*========================================================
  QMI_NAS_AVOID_TUNEAWAY_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_avoid_tune_away_req_msg(uint8_t trm_priority)
{
  payload = (void *) malloc(sizeof(nas_avoid_tuneaway_req_msg_v01));
  payload_len = sizeof(nas_avoid_tuneaway_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_AVOID_TUNEAWAY_REQ_MSG_V01;

  nas_avoid_tuneaway_req_msg_v01 *ptr = (nas_avoid_tuneaway_req_msg_v01 *) payload;
  ptr->trm_priority = trm_priority;
}
/*========================================================
  QMI_NAS_AVOID_TUNEAWAY_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_avoid_tune_away_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_avoid_tuneaway_resp_msg_v01));
  payload_len = sizeof(nas_avoid_tuneaway_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_AVOID_TUNEAWAY_RESP_MSG_V01;

  nas_avoid_tuneaway_resp_msg_v01 *ptr = (nas_avoid_tuneaway_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

/*========================================================
  QMI_NAS_DUAL_STANDBY_PREF_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_dual_standby_pref_req_msg(nas_standby_pref_enum_v01 standby_pref)
{
  payload = (void *) malloc(sizeof(nas_set_dual_standby_pref_req_msg_v01));
  payload_len = sizeof(nas_set_dual_standby_pref_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_DUAL_STANDBY_PREF_REQ_MSG_V01;

  nas_set_dual_standby_pref_req_msg_v01 *ptr = (nas_set_dual_standby_pref_req_msg_v01 *) payload;
  ptr->standby_pref_valid = TRUE;
  ptr->standby_pref = standby_pref;
}

void ril_utf_qmiril_if::update_default_qmi_nas_dual_standby_pref_req_msg(void)
{
  payload = (void *) malloc(sizeof(nas_set_dual_standby_pref_req_msg_v01));
  payload_len = sizeof(nas_set_dual_standby_pref_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_DUAL_STANDBY_PREF_REQ_MSG_V01;
}

void ril_utf_qmiril_if::update_qmi_nas_dual_standby_pref_data_subs(nas_subs_type_enum_v01 default_data_subs)
{
  nas_set_dual_standby_pref_req_msg_v01 *ptr = (nas_set_dual_standby_pref_req_msg_v01 *) payload;
  ptr->default_data_subs_valid = TRUE;
  ptr->default_data_subs = default_data_subs;
}

void ril_utf_qmiril_if::update_qmi_nas_dual_standby_pref_dds_duration(nas_dds_duration_enum_v01 dds_duration)
{
  nas_set_dual_standby_pref_req_msg_v01 *ptr = (nas_set_dual_standby_pref_req_msg_v01*) payload;
  ptr->dds_duration_valid = TRUE;
  ptr->dds_duration = dds_duration;
}

void ril_utf_qmiril_if::update_qmi_nas_dual_standby_pref_active_subs_mask(nas_active_subs_mask_type_v01 val)
{
  nas_set_dual_standby_pref_req_msg_v01 *ptr = (nas_set_dual_standby_pref_req_msg_v01*) payload;
  ptr->active_subs_mask_valid = TRUE;
  ptr->active_subs_mask = val;
}

/*========================================================
  QMI_NAS_DUAL_STANDBY_PREF_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_dual_standby_pref_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(nas_set_dual_standby_pref_resp_msg_v01));
  payload_len = sizeof(nas_set_dual_standby_pref_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_DUAL_STANDBY_PREF_RESP_MSG_V01;

  nas_set_dual_standby_pref_resp_msg_v01 *ptr = (nas_set_dual_standby_pref_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

dsd_system_status_ind_msg_v01* ril_utf_qmiril_if::update_default_qmi_dsd_system_status_ind(void)
{
  payload = (void*) malloc(sizeof(dsd_system_status_ind_msg_v01));
  payload_len = sizeof(dsd_system_status_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_DSD;
  msg_id = QMI_DSD_SYSTEM_STATUS_IND_V01;

  return (dsd_system_status_ind_msg_v01*) payload;
}

void ril_utf_qmiril_if::set_qmi_dsd_system_status_ind_avail_sys(dsd_sys_network_enum_v01 technology,
dsd_sys_rat_ex_enum_v01 rat_value, dsd_sys_so_mask_v01 so_mask)
{
  dsd_system_status_ind_msg_v01* dsd_info = (dsd_system_status_ind_msg_v01 *)payload;
  uint32_t index = 0;
  index = dsd_info->avail_sys_len;
  dsd_info->avail_sys[index].technology = technology;
  dsd_info->avail_sys[index].rat_value = rat_value;
  dsd_info->avail_sys[index].so_mask = so_mask;
  dsd_info->avail_sys_len += 1;
}

/*========================================================
  QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_client_provisioning_config_req_v01(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_REQ_V01;
}
/*========================================================
  QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_RSP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_client_provisioning_config_rsp_v01(void)
{
  payload = (void *) malloc(sizeof(ims_settings_get_client_provisioning_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_get_client_provisioning_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_RSP_V01;
}

void ril_utf_qmiril_if::set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_result(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  ims_settings_get_client_provisioning_config_rsp_msg_v01 *ptr = (ims_settings_get_client_provisioning_config_rsp_msg_v01*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

void ril_utf_qmiril_if::set_wifi_call_get_client_provisioning_config_rsp_v01(uint8_t wifi_call_valid, ims_settings_wfc_status_enum_v01 wifi_call)
{
  ims_settings_get_client_provisioning_config_rsp_msg_v01 *ptr = (ims_settings_get_client_provisioning_config_rsp_msg_v01*) payload;
  ptr->wifi_call_valid = wifi_call_valid;
  ptr->wifi_call = wifi_call;
}

void ril_utf_qmiril_if::set_wifi_call_preference_get_client_provisioning_config_rsp_v01(uint8_t wifi_call_preference_valid,
ims_settings_wfc_preference_v01 wifi_call_preference)
{
  ims_settings_get_client_provisioning_config_rsp_msg_v01 *ptr = (ims_settings_get_client_provisioning_config_rsp_msg_v01*) payload;
  ptr->wifi_call_preference_valid = wifi_call_preference_valid;
  ptr->wifi_call_preference = wifi_call_preference;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_settings_resp(uint8_t val)
{
  ims_settings_get_client_provisioning_config_rsp_msg_v01 *ptr = (ims_settings_get_client_provisioning_config_rsp_msg_v01 *) payload;
  ptr->settings_resp_valid = TRUE;
  ptr->settings_resp = (ims_settings_rsp_enum_v01)val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_enable_volte(uint8_t val)
{
  ims_settings_get_client_provisioning_config_rsp_msg_v01 *ptr = (ims_settings_get_client_provisioning_config_rsp_msg_v01 *) payload;
  ptr->enable_volte_valid = TRUE;
  ptr->enable_volte = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_enable_vt(uint8_t val)
{
  ims_settings_get_client_provisioning_config_rsp_msg_v01 *ptr = (ims_settings_get_client_provisioning_config_rsp_msg_v01 *) payload;
  ptr->enable_vt_valid = TRUE;
  ptr->enable_vt = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_enable_presence(uint8_t val)
{
  ims_settings_get_client_provisioning_config_rsp_msg_v01 *ptr = (ims_settings_get_client_provisioning_config_rsp_msg_v01 *) payload;
  ptr->enable_presence_valid = TRUE;
  ptr->enable_presence = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_wifi_call(ims_settings_wfc_status_enum_v01 val)
{
  ims_settings_get_client_provisioning_config_rsp_msg_v01 *ptr = (ims_settings_get_client_provisioning_config_rsp_msg_v01 *) payload;
  ptr->wifi_call_valid = TRUE;
  ptr->wifi_call = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_wifi_call_roaming(ims_settings_wfc_roaming_enum_v01 val)
{
  ims_settings_get_client_provisioning_config_rsp_msg_v01 *ptr = (ims_settings_get_client_provisioning_config_rsp_msg_v01 *) payload;
  ptr->wifi_call_roaming_valid = TRUE;
  ptr->wifi_call_roaming = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_wifi_call_preference(ims_settings_wfc_preference_v01 val)
{
  ims_settings_get_client_provisioning_config_rsp_msg_v01 *ptr = (ims_settings_get_client_provisioning_config_rsp_msg_v01 *) payload;
  ptr->wifi_call_preference_valid = TRUE;
  ptr->wifi_call_preference = val;
}

/*========================================================
  QMI_IMS_SETTINGS_CONFIG_IND_REG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_config_ind_reg_req_v01(void)
{
  payload = (void *) malloc(sizeof(ims_settings_config_ind_reg_req_msg_v01));
  payload_len = sizeof(ims_settings_config_ind_reg_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_CONFIG_IND_REG_REQ_V01;

  ims_settings_config_ind_reg_req_msg_v01 *ptr = (ims_settings_config_ind_reg_req_msg_v01*) payload;
  ptr->client_provisioning_config_valid = TRUE;
  ptr->client_provisioning_config = 1;
  ptr->reg_mgr_config_valid = TRUE;
  ptr->reg_mgr_config = 1;
  ptr->ims_service_enable_config_enabled_valid = TRUE;
  ptr->ims_service_enable_config_enabled = 1;
  ptr->rtt_config_enabled_valid = TRUE;
  ptr->rtt_config_enabled = 1;
}

/*========================================================
  QMI_IMS_SETTINGS_CONFIG_IND_REG_RSP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_config_ind_reg_rsp_v01(void)
{
  payload = (void *) malloc(sizeof(ims_settings_config_ind_reg_rsp_msg_v01));
  payload_len = sizeof(ims_settings_config_ind_reg_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_CONFIG_IND_REG_RSP_V01;
}
void ril_utf_qmiril_if::set_qmi_ims_config_ind_req_rsp_v01_result(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  ims_settings_config_ind_reg_rsp_msg_v01 *ptr = (ims_settings_config_ind_reg_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
  QMI_IMS_SETTINGS_GET_IMS_SERVICE_ENABLE_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_ims_service_enable_config_req(void)
{
  //payload = (void *) malloc(sizeof(ims_settings_get_ims_service_enable_config_req_msg_v01));
  //payload_len = sizeof(ims_settings_get_ims_service_enable_config_req_msg_v01);
  //memset(payload, 0x00, payload_len);
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_IMS_SERVICE_ENABLE_CONFIG_REQ_V01;
}

/*========================================================
  QMI_IMS_SETTINGS_GET_IMS_SERVICE_ENABLE_CONFIG_RSP_V01
========================================================*/
ims_settings_get_ims_service_enable_config_rsp_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_get_ims_service_enable_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error, ims_settings_get_ims_service_enable_config_rsp_msg_v01 *resp)
{
  payload = (void *) malloc(sizeof(ims_settings_get_ims_service_enable_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_get_ims_service_enable_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_IMS_SERVICE_ENABLE_CONFIG_RSP_V01;
  if (resp) {
    memcpy(payload, resp, payload_len);
  }

  ims_settings_get_ims_service_enable_config_rsp_msg_v01 *ptr = (ims_settings_get_ims_service_enable_config_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
  return ptr;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_ims_service_enable_config_resp_volte_enabled(uint8_t val)
{
  ims_settings_get_ims_service_enable_config_rsp_msg_v01 *ptr = (ims_settings_get_ims_service_enable_config_rsp_msg_v01 *) payload;
  ptr->volte_enabled_valid = TRUE;
  ptr->volte_enabled = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_ims_service_enable_config_resp_videotelephony_enabled(uint8_t val)
{
  ims_settings_get_ims_service_enable_config_rsp_msg_v01 *ptr = (ims_settings_get_ims_service_enable_config_rsp_msg_v01 *) payload;
  ptr->videotelephony_enabled_valid = TRUE;
  ptr->videotelephony_enabled = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_ims_service_enable_config_resp_ims_service_enabled(uint8_t val)
{
  ims_settings_get_ims_service_enable_config_rsp_msg_v01 *ptr = (ims_settings_get_ims_service_enable_config_rsp_msg_v01 *) payload;
  ptr->ims_service_enabled_valid = TRUE;
  ptr->ims_service_enabled = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_ims_service_enable_config_resp_wifi_calling_enabled(uint8_t val)
{
  ims_settings_get_ims_service_enable_config_rsp_msg_v01 *ptr = (ims_settings_get_ims_service_enable_config_rsp_msg_v01 *) payload;
  ptr->wifi_calling_enabled_valid = TRUE;
  ptr->wifi_calling_enabled = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_ims_service_enable_config_resp_call_mode_preference(ims_settings_call_mode_option_enum_v01 val)
{
  ims_settings_get_ims_service_enable_config_rsp_msg_v01 *ptr = (ims_settings_get_ims_service_enable_config_rsp_msg_v01 *) payload;
  ptr->call_mode_preference_valid = TRUE;
  ptr->call_mode_preference = val;
}

/*========================================================
  QMI_IMS_SETTINGS_SET_IMS_SERVICE_ENABLE_CONFIG_REQ_V01
========================================================*/
ims_settings_set_ims_service_enable_config_req_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_set_ims_service_enable_config_req(ims_settings_set_ims_service_enable_config_req_msg_v01 *req)
{
  payload = (void *) malloc(sizeof(ims_settings_set_ims_service_enable_config_req_msg_v01));
  payload_len = sizeof(ims_settings_set_ims_service_enable_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_IMS_SERVICE_ENABLE_CONFIG_REQ_V01;
  if (req) {
    memcpy(payload, req, payload_len);
  }
  ims_settings_set_ims_service_enable_config_req_msg_v01 *ptr = (ims_settings_set_ims_service_enable_config_req_msg_v01 *) payload;
  return ptr;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_ims_service_enable_config_req_volte_enabled(uint8_t val)
{
  ims_settings_set_ims_service_enable_config_req_msg_v01 *ptr = (ims_settings_set_ims_service_enable_config_req_msg_v01 *) payload;
  ptr->volte_enabled_valid = TRUE;
  ptr->volte_enabled = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_ims_service_enable_config_req_videotelephony_enabled(uint8_t val)
{
  ims_settings_set_ims_service_enable_config_req_msg_v01 *ptr = (ims_settings_set_ims_service_enable_config_req_msg_v01 *) payload;
  ptr->videotelephony_enabled_valid = TRUE;
  ptr->videotelephony_enabled = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_ims_service_enable_config_req_ims_service_enabled(uint8_t val)
{
  ims_settings_set_ims_service_enable_config_req_msg_v01 *ptr = (ims_settings_set_ims_service_enable_config_req_msg_v01 *) payload;
  ptr->ims_service_enabled_valid = TRUE;
  ptr->ims_service_enabled = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_ims_service_enable_config_req_ut_enabled(uint8_t val)
{
  ims_settings_set_ims_service_enable_config_req_msg_v01 *ptr = (ims_settings_set_ims_service_enable_config_req_msg_v01 *) payload;
  ptr->ut_enabled_valid = TRUE;
  ptr->ut_enabled = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_ims_service_enable_config_req_wifi_calling_enabled(uint8_t val)
{
  ims_settings_set_ims_service_enable_config_req_msg_v01 *ptr = (ims_settings_set_ims_service_enable_config_req_msg_v01 *) payload;
  ptr->wifi_calling_enabled_valid = TRUE;
  ptr->wifi_calling_enabled = val;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_ims_service_enable_config_req_call_mode_preference(ims_settings_call_mode_option_enum_v01 val)
{
  ims_settings_set_ims_service_enable_config_req_msg_v01 *ptr = (ims_settings_set_ims_service_enable_config_req_msg_v01 *) payload;
  ptr->call_mode_preference_valid = TRUE;
  ptr->call_mode_preference = val;
}

/*========================================================
  QMI_IMS_SETTINGS_SET_IMS_SERVICE_ENABLE_CONFIG_RSP_V01
========================================================*/
ims_settings_set_ims_service_enable_config_rsp_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_set_ims_service_enable_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(ims_settings_set_ims_service_enable_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_set_ims_service_enable_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_IMS_SERVICE_ENABLE_CONFIG_RSP_V01;

  ims_settings_set_ims_service_enable_config_rsp_msg_v01 *ptr = (ims_settings_set_ims_service_enable_config_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
  return ptr;
}

/*========================================================
  QMI_IMS_SETTINGS_SET_CODEC_DYNAMIC_CONFIG_REQ_V01
========================================================*/
ims_settings_set_codec_dynamic_config_req_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_set_codec_dynamic_config_req(ims_settings_set_codec_dynamic_config_req_msg_v01 *req)
{
  payload = (void *) malloc(sizeof(ims_settings_set_codec_dynamic_config_req_msg_v01));
  payload_len = sizeof(ims_settings_set_codec_dynamic_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_CODEC_DYNAMIC_CONFIG_REQ_V01;
  if (req) {
    memcpy(payload, req, payload_len);
  }
  ims_settings_set_codec_dynamic_config_req_msg_v01 *ptr = (ims_settings_set_codec_dynamic_config_req_msg_v01 *) payload;
  return ptr;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_codec_dynamic_config_req_amr_mode_set_mask(ims_settings_amr_nb_codec_mode_type_mask_v01 val)
{
  ims_settings_set_codec_dynamic_config_req_msg_v01 *ptr = (ims_settings_set_codec_dynamic_config_req_msg_v01 *) payload;
  ptr->amr_mode_set_mask_valid = TRUE;
  ptr->amr_mode_set_mask = val;
}
/*========================================================
  QMI_IMS_SETTINGS_SET_CODEC_DYNAMIC_CONFIG_RSP_V01
========================================================*/
ims_settings_set_codec_dynamic_config_rsp_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_set_codec_dynamic_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(ims_settings_set_codec_dynamic_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_set_codec_dynamic_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_CODEC_DYNAMIC_CONFIG_RSP_V01;

  ims_settings_set_codec_dynamic_config_rsp_msg_v01 *ptr = (ims_settings_set_codec_dynamic_config_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
  return ptr;
}

/*========================================================
  QMI_IMS_SETTINGS_GET_CODEC_DYNAMIC_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_codec_dynamic_config_req(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_CODEC_DYNAMIC_CONFIG_REQ_V01;
}

/*========================================================
  QMI_IMS_SETTINGS_GET_CODEC_DYNAMIC_CONFIG_RSP_V01
========================================================*/
ims_settings_get_codec_dynamic_config_rsp_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_get_codec_dynamic_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error, ims_settings_get_codec_dynamic_config_rsp_msg_v01 *resp)
{
  payload = (void *) malloc(sizeof(ims_settings_get_codec_dynamic_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_get_codec_dynamic_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_CODEC_DYNAMIC_CONFIG_RSP_V01;
  if (resp) {
    memcpy(payload, resp, payload_len);
  }

  ims_settings_get_codec_dynamic_config_rsp_msg_v01 *ptr = (ims_settings_get_codec_dynamic_config_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
  return ptr;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_get_codec_dynamic_config_resp_amr_mode_set_mask(ims_settings_amr_nb_codec_mode_type_mask_v01 val)
{
  ims_settings_get_codec_dynamic_config_rsp_msg_v01 *ptr = (ims_settings_get_codec_dynamic_config_rsp_msg_v01 *) payload;
  ptr->amr_mode_set_mask_valid = TRUE;
  ptr->amr_mode_set_mask = val;
}

/*========================================================
  QMI_IMS_SETTINGS_SET_VOICE_DYNAMIC_CONFIG_REQ_V01
========================================================*/
ims_settings_set_voice_dynamic_config_req_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_set_voice_dynamic_config_req(ims_settings_set_voice_dynamic_config_req_msg_v01 *req)
{
  payload = (void *) malloc(sizeof(ims_settings_set_voice_dynamic_config_req_msg_v01));
  payload_len = sizeof(ims_settings_set_voice_dynamic_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_VOICE_DYNAMIC_CONFIG_REQ_V01;
  if (req) {
    memcpy(payload, req, payload_len);
  }
  ims_settings_set_voice_dynamic_config_req_msg_v01 *ptr = (ims_settings_set_voice_dynamic_config_req_msg_v01 *) payload;
  return ptr;
}
/*========================================================
  QMI_IMS_SETTINGS_SET_VOICE_DYNAMIC_CONFIG_RSP_V01
========================================================*/
ims_settings_set_voice_dynamic_config_rsp_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_set_voice_dynamic_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(ims_settings_set_voice_dynamic_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_set_voice_dynamic_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_VOICE_DYNAMIC_CONFIG_RSP_V01;

  ims_settings_set_voice_dynamic_config_rsp_msg_v01 *ptr = (ims_settings_set_voice_dynamic_config_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
  return ptr;
}
/*========================================================
  QMI_IMS_SETTINGS_GET_VOICE_DYNAMIC_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_voice_dynamic_config_req(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_VOICE_DYNAMIC_CONFIG_REQ_V01;
}

/*========================================================
  QMI_IMS_SETTINGS_GET_VOICE_DYNAMIC_CONFIG_RSP_V01
========================================================*/
ims_settings_get_voice_dynamic_config_rsp_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_get_voice_dynamic_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error, ims_settings_get_voice_dynamic_config_rsp_msg_v01 *resp)
{
  payload = (void *) malloc(sizeof(ims_settings_get_voice_dynamic_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_get_voice_dynamic_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_VOICE_DYNAMIC_CONFIG_RSP_V01;
  if (resp) {
    memcpy(payload, resp, payload_len);
  }

  ims_settings_get_voice_dynamic_config_rsp_msg_v01 *ptr = (ims_settings_get_voice_dynamic_config_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
  return ptr;
}

/*========================================================
  QMI_IMS_SETTINGS_SET_REG_MGR_NEW_CONFIG_REQ_V01
========================================================*/
ims_settings_set_reg_mgr_new_config_req_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_set_reg_mgr_new_config_req(ims_settings_set_reg_mgr_new_config_req_msg_v01 *req)
{
  payload = (void *) malloc(sizeof(ims_settings_set_reg_mgr_new_config_req_msg_v01));
  payload_len = sizeof(ims_settings_set_reg_mgr_new_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_REG_MGR_NEW_CONFIG_REQ_V01;
  if (req) {
    memcpy(payload, req, payload_len);
  }
  ims_settings_set_reg_mgr_new_config_req_msg_v01 *ptr = (ims_settings_set_reg_mgr_new_config_req_msg_v01 *) payload;
  return ptr;
}
/*========================================================
  QMI_IMS_SETTINGS_SET_REG_MGR_NEW_CONFIG_RSP_V01
========================================================*/
ims_settings_set_reg_mgr_new_config_rsp_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_set_reg_mgr_new_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(ims_settings_set_reg_mgr_new_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_set_reg_mgr_new_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_REG_MGR_NEW_CONFIG_RSP_V01;

  ims_settings_set_reg_mgr_new_config_rsp_msg_v01 *ptr = (ims_settings_set_reg_mgr_new_config_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
  return ptr;
}
/*========================================================
  QMI_IMS_SETTINGS_GET_REG_MGR_NEW_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_reg_mgr_new_config_req(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_REG_MGR_NEW_CONFIG_REQ_V01;
}
/*========================================================
  QMI_IMS_SETTINGS_GET_REG_MGR_NEW_CONFIG_RSP_V01
========================================================*/
ims_settings_get_reg_mgr_new_config_rsp_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_get_reg_mgr_new_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error, ims_settings_get_reg_mgr_new_config_rsp_msg_v01 *resp)
{
  payload = (void *) malloc(sizeof(ims_settings_get_reg_mgr_new_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_get_reg_mgr_new_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_REG_MGR_NEW_CONFIG_RSP_V01;
  if (resp) {
    memcpy(payload, resp, payload_len);
  }

  ims_settings_get_reg_mgr_new_config_rsp_msg_v01 *ptr = (ims_settings_get_reg_mgr_new_config_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
  return ptr;
}

/*========================================================
  QMI_IMS_SETTINGS_SET_SMS_NEW_CONFIG_REQ_V01
========================================================*/
ims_settings_set_sms_new_config_req_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_set_sms_new_config_req(ims_settings_set_sms_new_config_req_msg_v01 *req)
{
  payload = (void *) malloc(sizeof(ims_settings_set_sms_new_config_req_msg_v01));
  payload_len = sizeof(ims_settings_set_sms_new_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_SMS_NEW_CONFIG_REQ_V01;
  if (req) {
    memcpy(payload, req, payload_len);
  }
  ims_settings_set_sms_new_config_req_msg_v01 *ptr = (ims_settings_set_sms_new_config_req_msg_v01 *) payload;
  return ptr;
}
/*========================================================
  QMI_IMS_SETTINGS_SET_SMS_NEW_CONFIG_RSP_V01
========================================================*/
ims_settings_set_sms_new_config_rsp_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_set_sms_new_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(ims_settings_set_sms_new_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_set_sms_new_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_SMS_NEW_CONFIG_RSP_V01;

  ims_settings_set_sms_new_config_rsp_msg_v01 *ptr = (ims_settings_set_sms_new_config_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
  return ptr;
}
/*========================================================
  QMI_IMS_SETTINGS_GET_SMS_NEW_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_sms_new_config_req(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_SMS_NEW_CONFIG_REQ_V01;
}
/*========================================================
  QMI_IMS_SETTINGS_GET_SMS_NEW_CONFIG_RSP_V01
========================================================*/
ims_settings_get_sms_new_config_rsp_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_get_sms_new_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error, ims_settings_get_sms_new_config_rsp_msg_v01 *resp)
{
  payload = (void *) malloc(sizeof(ims_settings_get_sms_new_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_get_sms_new_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_SMS_NEW_CONFIG_RSP_V01;
  if (resp) {
    memcpy(payload, resp, payload_len);
  }

  ims_settings_get_sms_new_config_rsp_msg_v01 *ptr = (ims_settings_get_sms_new_config_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
  return ptr;
}
/*========================================================
  QMI_IMS_SETTINGS_SET_RTP_DYNAMIC_CONFIG_REQ_V01
========================================================*/
ims_settings_set_rtp_dynamic_config_req_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_set_rtp_dynamic_config_req(ims_settings_set_rtp_dynamic_config_req_msg_v01 *req)
{
  payload = (void *) malloc(sizeof(ims_settings_set_rtp_dynamic_config_req_msg_v01));
  payload_len = sizeof(ims_settings_set_rtp_dynamic_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_RTP_DYNAMIC_CONFIG_REQ_V01;
  if (req) {
    memcpy(payload, req, payload_len);
  }
  ims_settings_set_rtp_dynamic_config_req_msg_v01 *ptr = (ims_settings_set_rtp_dynamic_config_req_msg_v01 *) payload;
  return ptr;
}
/*========================================================
  QMI_IMS_SETTINGS_SET_RTP_DYNAMIC_CONFIG_RSP_V01
========================================================*/
ims_settings_set_rtp_dynamic_config_rsp_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_set_rtp_dynamic_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(ims_settings_set_rtp_dynamic_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_set_rtp_dynamic_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_RTP_DYNAMIC_CONFIG_RSP_V01;

  ims_settings_set_rtp_dynamic_config_rsp_msg_v01 *ptr = (ims_settings_set_rtp_dynamic_config_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
  return ptr;
}
/*========================================================
  QMI_IMS_SETTINGS_GET_RTP_DYNAMIC_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_rtp_dynamic_config_req(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_RTP_DYNAMIC_CONFIG_REQ_V01;
}
/*========================================================
  QMI_IMS_SETTINGS_GET_RTP_DYNAMIC_CONFIG_RSP_V01
========================================================*/
ims_settings_get_rtp_dynamic_config_rsp_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_get_rtp_dynamic_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error, ims_settings_get_rtp_dynamic_config_rsp_msg_v01 *resp)
{
  payload = (void *) malloc(sizeof(ims_settings_get_rtp_dynamic_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_get_rtp_dynamic_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_RTP_DYNAMIC_CONFIG_RSP_V01;
  if (resp) {
    memcpy(payload, resp, payload_len);
  }

  ims_settings_get_rtp_dynamic_config_rsp_msg_v01 *ptr = (ims_settings_get_rtp_dynamic_config_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
  return ptr;
}



/*========================================================
  QMI_IMS_SETTINGS_SET_SIP_NEW_CONFIG_REQ_V01
========================================================*/
ims_settings_set_sip_new_config_req_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_set_sip_new_config_req(ims_settings_set_sip_new_config_req_msg_v01 *req)
{
  payload = (void *) malloc(sizeof(ims_settings_set_sip_new_config_req_msg_v01));
  payload_len = sizeof(ims_settings_set_sip_new_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_SIP_NEW_CONFIG_REQ_V01;
  if (req) {
    memcpy(payload, req, payload_len);
  }
  ims_settings_set_sip_new_config_req_msg_v01 *ptr = (ims_settings_set_sip_new_config_req_msg_v01 *) payload;
  return ptr;
}
/*========================================================
  QMI_IMS_SETTINGS_SET_SIP_NEW_CONFIG_RSP_V01
========================================================*/
ims_settings_set_sip_new_config_rsp_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_set_sip_new_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(ims_settings_set_sip_new_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_set_sip_new_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_SIP_NEW_CONFIG_RSP_V01;

  ims_settings_set_sip_new_config_rsp_msg_v01 *ptr = (ims_settings_set_sip_new_config_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
  return ptr;
}

/*========================================================
  QMI_IMS_SETTINGS_GET_SIP_NEW_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_sip_new_config_req(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_SIP_NEW_CONFIG_REQ_V01;
}

/*========================================================
  QMI_IMS_SETTINGS_GET_SIP_NEW_CONFIG_RSP_V01
========================================================*/
ims_settings_get_sip_new_config_rsp_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_get_sip_new_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error, ims_settings_get_sip_new_config_rsp_msg_v01 *resp)
{
  payload = (void *) malloc(sizeof(ims_settings_get_sip_new_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_get_sip_new_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_SIP_NEW_CONFIG_RSP_V01;
  if (resp) {
    memcpy(payload, resp, payload_len);
  }

  ims_settings_get_sip_new_config_rsp_msg_v01 *ptr = (ims_settings_get_sip_new_config_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
  return ptr;
}

/*========================================================
  QMI_IMS_SETTINGS_SET_PRESENCE_NEW_CONFIG_REQ_V01
========================================================*/
ims_settings_set_presence_new_config_req_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_set_presence_new_config_req(ims_settings_set_presence_new_config_req_msg_v01 *req)
{
  payload = (void *) malloc(sizeof(ims_settings_set_presence_new_config_req_msg_v01));
  payload_len = sizeof(ims_settings_set_presence_new_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_PRESENCE_NEW_CONFIG_REQ_V01;
  if (req) {
    memcpy(payload, req, payload_len);
  }
  ims_settings_set_presence_new_config_req_msg_v01 *ptr = (ims_settings_set_presence_new_config_req_msg_v01 *) payload;
  return ptr;
}
/*========================================================
  QMI_IMS_SETTINGS_SET_PRESENCE_NEW_CONFIG_RSP_V01
========================================================*/
ims_settings_set_presence_new_config_rsp_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_set_presence_new_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(ims_settings_set_presence_new_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_set_presence_new_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_PRESENCE_NEW_CONFIG_RSP_V01;

  ims_settings_set_presence_new_config_rsp_msg_v01 *ptr = (ims_settings_set_presence_new_config_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
  return ptr;
}
/*========================================================
  QMI_IMS_SETTINGS_GET_PRESENCE_NEW_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_presence_new_config_req(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_PRESENCE_NEW_CONFIG_REQ_V01;
}
/*========================================================
  QMI_IMS_SETTINGS_GET_PRESENCE_NEW_CONFIG_RSP_V01
========================================================*/
ims_settings_get_presence_new_config_rsp_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_get_presence_new_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error, ims_settings_get_presence_new_config_rsp_msg_v01 *resp)
{
  payload = (void *) malloc(sizeof(ims_settings_get_presence_new_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_get_presence_new_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_PRESENCE_NEW_CONFIG_RSP_V01;
  if (resp) {
    memcpy(payload, resp, payload_len);
  }

  ims_settings_get_presence_new_config_rsp_msg_v01 *ptr = (ims_settings_get_presence_new_config_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
  return ptr;
}

/*========================================================
  QMI_IMS_SETTINGS_SET_HANDOVER_CONFIG_REQ_V01
========================================================*/
ims_settings_set_handover_config_req_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_set_handover_config_req(ims_settings_set_handover_config_req_msg_v01 *req)
{
  payload = (void *) malloc(sizeof(ims_settings_set_handover_config_req_msg_v01));
  payload_len = sizeof(ims_settings_set_handover_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_HANDOVER_CONFIG_REQ_V01;
  if (req) {
    memcpy(payload, req, payload_len);
  }
  ims_settings_set_handover_config_req_msg_v01 *ptr = (ims_settings_set_handover_config_req_msg_v01 *) payload;
  return ptr;
}
/*========================================================
  QMI_IMS_SETTINGS_SET_HANDOVER_CONFIG_RSP_V01
========================================================*/
ims_settings_set_handover_config_rsp_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_set_handover_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(ims_settings_set_handover_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_set_handover_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_HANDOVER_CONFIG_RSP_V01;

  ims_settings_set_handover_config_rsp_msg_v01 *ptr = (ims_settings_set_handover_config_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
  return ptr;
}
/*========================================================
  QMI_IMS_SETTINGS_GET_HANDOVER_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_handover_config_req(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_HANDOVER_CONFIG_REQ_V01;
}
/*========================================================
  QMI_IMS_SETTINGS_GET_HANDOVER_CONFIG_RSP_V01
========================================================*/
ims_settings_get_handover_config_rsp_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_get_handover_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error, ims_settings_get_handover_config_rsp_msg_v01 *resp)
{
  payload = (void *) malloc(sizeof(ims_settings_get_handover_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_get_handover_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_HANDOVER_CONFIG_RSP_V01;
  if (resp) {
    memcpy(payload, resp, payload_len);
  }

  ims_settings_get_handover_config_rsp_msg_v01 *ptr = (ims_settings_get_handover_config_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
  return ptr;
}

/*========================================================
  QMI_IMS_SETTINGS_SET_IMS_NEW_CONFIG_REQ_V01
========================================================*/
ims_settings_set_ims_new_config_req_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_set_ims_new_config_req(ims_settings_set_ims_new_config_req_msg_v01 *req)
{
  payload = (void *) malloc(sizeof(ims_settings_set_ims_new_config_req_msg_v01));
  payload_len = sizeof(ims_settings_set_ims_new_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_IMS_NEW_CONFIG_REQ_V01;
  if (req) {
    memcpy(payload, req, payload_len);
  }
  ims_settings_set_ims_new_config_req_msg_v01 *ptr = (ims_settings_set_ims_new_config_req_msg_v01 *) payload;
  return ptr;
}
/*========================================================
  QMI_IMS_SETTINGS_SET_IMS_NEW_CONFIG_RSP_V01
========================================================*/
ims_settings_set_ims_new_config_rsp_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_set_ims_new_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(ims_settings_set_ims_new_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_set_ims_new_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_IMS_NEW_CONFIG_RSP_V01;

  ims_settings_set_ims_new_config_rsp_msg_v01 *ptr = (ims_settings_set_ims_new_config_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
  return ptr;
}
/*========================================================
  QMI_IMS_SETTINGS_GET_IMS_NEW_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_ims_new_config_req(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_IMS_NEW_CONFIG_REQ_V01;
}
/*========================================================
  QMI_IMS_SETTINGS_GET_IMS_NEW_CONFIG_RSP_V01
========================================================*/
ims_settings_get_ims_new_config_rsp_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_get_ims_new_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error, ims_settings_get_ims_new_config_rsp_msg_v01 *resp)
{
  payload = (void *) malloc(sizeof(ims_settings_get_ims_new_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_get_ims_new_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_IMS_NEW_CONFIG_RSP_V01;
  if (resp) {
    memcpy(payload, resp, payload_len);
  }

  ims_settings_get_ims_new_config_rsp_msg_v01 *ptr = (ims_settings_get_ims_new_config_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
  return ptr;
}

/*========================================================
  QMI_IMS_SETTINGS_SET_EMER_DYNAMIC_CONFIG_REQ_V01
========================================================*/
ims_settings_set_emer_dynamic_config_req_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_set_emer_dynamic_config_req(ims_settings_set_emer_dynamic_config_req_msg_v01 *req)
{
  payload = (void *) malloc(sizeof(ims_settings_set_emer_dynamic_config_req_msg_v01));
  payload_len = sizeof(ims_settings_set_emer_dynamic_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_EMER_DYNAMIC_CONFIG_REQ_V01;
  if (req) {
    memcpy(payload, req, payload_len);
  }
  ims_settings_set_emer_dynamic_config_req_msg_v01 *ptr = (ims_settings_set_emer_dynamic_config_req_msg_v01 *) payload;
  return ptr;
}
/*========================================================
  QMI_IMS_SETTINGS_SET_EMER_DYNAMIC_CONFIG_RSP_V01
========================================================*/
ims_settings_set_emer_dynamic_config_rsp_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_set_emer_dynamic_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(ims_settings_set_emer_dynamic_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_set_emer_dynamic_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_EMER_DYNAMIC_CONFIG_RSP_V01;

  ims_settings_set_emer_dynamic_config_rsp_msg_v01 *ptr = (ims_settings_set_emer_dynamic_config_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
  return ptr;
}
/*========================================================
  QMI_IMS_SETTINGS_GET_EMER_DYNAMIC_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_emer_dynamic_config_req(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_EMER_DYNAMIC_CONFIG_REQ_V01;
}
/*========================================================
  QMI_IMS_SETTINGS_GET_EMER_DYNAMIC_CONFIG_RSP_V01
========================================================*/
ims_settings_get_emer_dynamic_config_rsp_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_get_emer_dynamic_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error, ims_settings_get_emer_dynamic_config_rsp_msg_v01 *resp)
{
  payload = (void *) malloc(sizeof(ims_settings_get_emer_dynamic_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_get_emer_dynamic_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_EMER_DYNAMIC_CONFIG_RSP_V01;
  if (resp) {
    memcpy(payload, resp, payload_len);
  }

  ims_settings_get_emer_dynamic_config_rsp_msg_v01 *ptr = (ims_settings_get_emer_dynamic_config_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
  return ptr;
}

/*========================================================
  QMI_IMS_SETTINGS_SET_VIDEO_DYNAMIC_CONFIG_REQ_V01
========================================================*/
ims_settings_set_video_dynamic_config_req_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_set_video_dynamic_config_req(ims_settings_set_video_dynamic_config_req_msg_v01 *req)
{
  payload = (void *) malloc(sizeof(ims_settings_set_video_dynamic_config_req_msg_v01));
  payload_len = sizeof(ims_settings_set_video_dynamic_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_VIDEO_DYNAMIC_CONFIG_REQ_V01;
  if (req) {
    memcpy(payload, req, payload_len);
  }
  ims_settings_set_video_dynamic_config_req_msg_v01 *ptr = (ims_settings_set_video_dynamic_config_req_msg_v01 *) payload;
  return ptr;
}
/*========================================================
  QMI_IMS_SETTINGS_SET_VIDEO_DYNAMIC_CONFIG_RSP_V01
========================================================*/
ims_settings_set_video_dynamic_config_rsp_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_set_video_dynamic_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(ims_settings_set_video_dynamic_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_set_video_dynamic_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_VIDEO_DYNAMIC_CONFIG_RSP_V01;

  ims_settings_set_video_dynamic_config_rsp_msg_v01 *ptr = (ims_settings_set_video_dynamic_config_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
  return ptr;
}
/*========================================================
  QMI_IMS_SETTINGS_GET_VIDEO_DYNAMIC_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_video_dynamic_config_req(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_VIDEO_DYNAMIC_CONFIG_REQ_V01;
}
/*========================================================
  QMI_IMS_SETTINGS_GET_VIDEO_DYNAMIC_CONFIG_RSP_V01
========================================================*/
ims_settings_get_video_dynamic_config_rsp_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_get_video_dynamic_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error, ims_settings_get_video_dynamic_config_rsp_msg_v01 *resp)
{
  payload = (void *) malloc(sizeof(ims_settings_get_video_dynamic_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_get_video_dynamic_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_VIDEO_DYNAMIC_CONFIG_RSP_V01;
  if (resp) {
    memcpy(payload, resp, payload_len);
  }

  ims_settings_get_video_dynamic_config_rsp_msg_v01 *ptr = (ims_settings_get_video_dynamic_config_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
  return ptr;
}

/*========================================================
  QMI_IMS_SETTINGS_SET_RCS_SM_NEW_CONFIG_REQ_V01
========================================================*/
ims_settings_set_rcs_new_config_req_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_set_rcs_new_config_req(ims_settings_set_rcs_new_config_req_msg_v01 *req)
{
  payload = (void *) malloc(sizeof(ims_settings_set_rcs_new_config_req_msg_v01));
  payload_len = sizeof(ims_settings_set_rcs_new_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_RCS_SM_NEW_CONFIG_REQ_V01;
  if (req) {
    memcpy(payload, req, payload_len);
  }
  ims_settings_set_rcs_new_config_req_msg_v01 *ptr = (ims_settings_set_rcs_new_config_req_msg_v01 *) payload;
  return ptr;
}
/*========================================================
  QMI_IMS_SETTINGS_SET_RCS_SM_NEW_CONFIG_RSP_V01
========================================================*/
ims_settings_set_rcs_new_config_rsp_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_set_rcs_new_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(ims_settings_set_rcs_new_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_set_rcs_new_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_RCS_SM_NEW_CONFIG_RSP_V01;

  ims_settings_set_rcs_new_config_rsp_msg_v01 *ptr = (ims_settings_set_rcs_new_config_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
  return ptr;
}
/*========================================================
  QMI_IMS_SETTINGS_GET_RCS_SM_NEW_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_rcs_new_config_req(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_RCS_SM_NEW_CONFIG_REQ_V01;
}
/*========================================================
  QMI_IMS_SETTINGS_GET_RCS_SM_NEW_CONFIG_RSP_V01
========================================================*/
ims_settings_get_rcs_new_config_rsp_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_get_rcs_new_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error, ims_settings_get_rcs_new_config_rsp_msg_v01 *resp)
{
  payload = (void *) malloc(sizeof(ims_settings_get_rcs_new_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_get_rcs_new_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_RCS_SM_NEW_CONFIG_RSP_V01;
  if (resp) {
    memcpy(payload, resp, payload_len);
  }

  ims_settings_get_rcs_new_config_rsp_msg_v01 *ptr = (ims_settings_get_rcs_new_config_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
  return ptr;
}
/*========================================================
  QMI_IMS_SETTINGS_SET_CONCURRENCY_CONFIG_REQ_V01
========================================================*/
ims_settings_set_concurrency_config_req_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_set_concurrency_config_req(ims_settings_set_concurrency_config_req_msg_v01 *req)
{
  payload = (void *) malloc(sizeof(ims_settings_set_concurrency_config_req_msg_v01));
  payload_len = sizeof(ims_settings_set_concurrency_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_CONCURRENCY_CONFIG_REQ_V01;
  if (req) {
    memcpy(payload, req, payload_len);
  }
  ims_settings_set_concurrency_config_req_msg_v01 *ptr = (ims_settings_set_concurrency_config_req_msg_v01 *) payload;
  return ptr;
}
/*========================================================
  QMI_IMS_SETTINGS_SET_CONCURRENCY_CONFIG_RSP_V01
========================================================*/
ims_settings_set_concurrency_config_rsp_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_set_concurrency_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(ims_settings_set_concurrency_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_set_concurrency_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_CONCURRENCY_CONFIG_RSP_V01;

  ims_settings_set_concurrency_config_rsp_msg_v01 *ptr = (ims_settings_set_concurrency_config_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
  return ptr;
}
#if 0
/*========================================================
  QMI_IMS_SETTINGS_GET_CONCURRENCY_CONFIG_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_get_concurrency_config_req(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_CONCURRENCY_CONFIG_REQ_V01;
}
/*========================================================
  QMI_IMS_SETTINGS_GET_CONCURRENCY_CONFIG_RSP_V01
========================================================*/
ims_settings_get_concurrency_config_rsp_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_get_concurrency_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error, ims_settings_get_concurrency_config_rsp_msg_v01 *resp)
{
  payload = (void *) malloc(sizeof(ims_settings_get_concurrency_config_rsp_msg_v01));
  payload_len = sizeof(ims_settings_get_concurrency_config_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_GET_CONCURRENCY_CONFIG_RSP_V01;
  if (resp) {
    memcpy(payload, resp, payload_len);
  }

  ims_settings_get_concurrency_config_rsp_msg_v01 *ptr = (ims_settings_get_concurrency_config_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
  return ptr;
}
#endif

/*========================================================
  QMI_IMS_SETTINGS_SEND_RTT_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_send_rtt_message_req(void)
{
  payload = (void *) malloc(sizeof(ims_settings_send_rtt_req_msg_v01));
  payload_len = sizeof(ims_settings_send_rtt_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SEND_RTT_REQ_V01;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_ims_service_rtt_message(const char* val)
{
  ims_settings_send_rtt_req_msg_v01 *qmi_req = (ims_settings_send_rtt_req_msg_v01 *) payload;
  qmi_req->rtt_data_len = strlen(val);
  strlcpy((char*)qmi_req->rtt_data,
            val,
            IMS_SETTINGS_RTT_DATA_LEN_MAX_V01);
}

/*========================================================
  QMI_IMS_SETTINGS_SEND_RTT_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_set_ims_service_send_rtt_message_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(ims_settings_send_rtt_resp_msg_v01));
  payload_len = sizeof(ims_settings_send_rtt_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SEND_RTT_RESP_V01;

  ims_settings_send_rtt_resp_msg_v01 *ptr = (ims_settings_send_rtt_resp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*
  QMI_IMS_SETTINGS_RTT_RCVD_DATA_IND_V01
*/
void ril_utf_qmiril_if::update_default_qmi_imss_rtt_message_ind_v01()
{
  payload = (void *) malloc(sizeof(ims_settings_rtt_ind_msg_v01));
  ims_settings_rtt_ind_msg_v01 *msg = (ims_settings_rtt_ind_msg_v01 *)payload;
  payload_len = sizeof(ims_settings_rtt_ind_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_RTT_RCVD_DATA_IND_V01;

  char * rtt = "Hi";
  msg->rtt_data_len = strlen(rtt);
  memcpy(msg->rtt_data,rtt,msg->rtt_data_len);
}
/*
  QMI_IMS_SETTINGS_IMS_SERVICE_ENABLE_CONFIG_IND_V01
*/
ims_settings_ims_service_enable_config_ind_msg_v01 *ril_utf_qmiril_if::update_default_qmi_ims_settings_ims_service_enable_config_ind_msg()
{
  payload = (void *) malloc(sizeof(ims_settings_ims_service_enable_config_ind_msg_v01));
  payload_len = sizeof(ims_settings_ims_service_enable_config_ind_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_IMS_SERVICE_ENABLE_CONFIG_IND_V01;
  ims_settings_ims_service_enable_config_ind_msg_v01 *ptr = (ims_settings_ims_service_enable_config_ind_msg_v01 *)payload;
  return ptr;
}

/*========================================================
  QMI_IMS_SETTINGS_SET_DIGITS_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_set_digits_req(ims_settings_digits_feature_status_v01 is_digits_enabled)
{
  payload = (void *) malloc(sizeof(ims_settings_set_digits_req_msg_v01));
  payload_len = sizeof(ims_settings_set_digits_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_DIGITS_REQ_V01;
  ims_settings_set_digits_req_msg_v01 *ptr = (ims_settings_set_digits_req_msg_v01 *) payload;
  ptr->is_digits_enabled = is_digits_enabled;
}
void ril_utf_qmiril_if::set_qmi_ims_settings_set_digits_req_no_of_lines(uint8_t val)
{
  ims_settings_set_digits_req_msg_v01 *ptr = (ims_settings_set_digits_req_msg_v01 *) payload;
  ptr->no_of_lines_valid = TRUE;
  ptr->no_of_lines = val;
}
ims_settings_digits_line_info_v01 *ril_utf_qmiril_if::set_qmi_ims_settings_set_digits_req_line_info_list(uint32_t line_info_list_len)
{
  ims_settings_set_digits_req_msg_v01 *ptr = (ims_settings_set_digits_req_msg_v01 *) payload;
  ptr->line_info_list_valid = TRUE;
  ptr->line_info_list_len = line_info_list_len;
  return (ptr->line_info_list);
}

/*========================================================
  QMI_IMS_SETTINGS_SET_DIGITS_RSP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_ims_settings_set_digits_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(ims_settings_set_digits_req_rsp_msg_v01));
  payload_len = sizeof(ims_settings_set_digits_req_rsp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SETTINGS_SET_DIGITS_RSP_V01;

  ims_settings_set_digits_req_rsp_msg_v01 *ptr = (ims_settings_set_digits_req_rsp_msg_v01 *) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}


/*========================================================
  QMI_UIM_CHANGE_PROVISIONING_SESSION_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_uim_change_provisioning_session_req(uim_session_type_enum_v01 session_type, uint8_t activate)
{
  payload = (void *) malloc(sizeof(uim_change_provisioning_session_req_msg_v01));
  payload_len = sizeof(uim_change_provisioning_session_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_CHANGE_PROVISIONING_SESSION_REQ_V01;

  uim_change_provisioning_session_req_msg_v01 *ptr = (uim_change_provisioning_session_req_msg_v01 *) payload;
  ptr->session_change.session_type = session_type;
  ptr->session_change.activate = activate;
}

void ril_utf_qmiril_if::set_qmi_uim_change_provisioning_session_req_application_information(uim_slot_enum_v01 slot, uint32_t aid_len, uint8_t* aid)
{
  uim_change_provisioning_session_req_msg_v01 *ptr = (uim_change_provisioning_session_req_msg_v01 *) payload;
  ptr->application_information_valid = 1;
  ptr->application_information.slot = slot;
  ptr->application_information.aid_len = aid_len;
  memcpy(ptr->application_information.aid, aid, aid_len);
}

/*========================================================
  QMI_UIM_CHANGE_PROVISIONING_SESSION_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_uim_change_provisioning_session_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(uim_change_provisioning_session_resp_msg_v01));
  payload_len = sizeof(uim_change_provisioning_session_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_CHANGE_PROVISIONING_SESSION_RESP_V01;

  uim_change_provisioning_session_resp_msg_v01 *ptr = (uim_change_provisioning_session_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

/*========================================================
  QMI_UIM_POWER_UP_REQ
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_uim_power_up_req(int slot)
{
  payload = (void *) malloc(sizeof(uim_power_up_req_msg_v01));
  payload_len = sizeof(uim_power_up_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_POWER_UP_REQ_V01;
  uim_power_up_req_msg_v01*ptr = (uim_power_up_req_msg_v01*) payload;
  ptr->slot = (uim_slot_enum_v01)slot;
}
/*========================================================
  QMI_UIM_POWER_UP_RESP
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_uim_power_up_resp()
{
  payload = (void *) malloc(sizeof(uim_power_up_resp_msg_v01));
  payload_len = sizeof(uim_power_up_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_POWER_UP_RESP_V01;
}

/*========================================================
  QMI_NAS_GET_SERV_CELL_SIB_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_serv_cell_sib_req(void)
{
  payload = (void *) malloc(sizeof(nas_get_serv_cell_sib_req_msg_v01));
  payload_len = sizeof(nas_get_serv_cell_sib_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_SERV_CELL_SIB_REQ_MSG_V01;
}
void ril_utf_qmiril_if::set_qmi_nas_get_serv_cell_sib_req_sib_num(uint8_t sib_num)
{
  nas_get_serv_cell_sib_req_msg_v01* ptr = (nas_get_serv_cell_sib_req_msg_v01*) payload;
  ptr->sib_num = sib_num;
}

/*========================================================
  QMI_NAS_GET_SERV_CELL_SIB_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_serv_cell_sib_resp(void)
{
  payload = (void *) malloc(sizeof(nas_get_serv_cell_sib_resp_msg_v01));
  payload_len = sizeof(nas_get_serv_cell_sib_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_SERV_CELL_SIB_RESP_MSG_V01;
}
void ril_utf_qmiril_if::set_qmi_nas_get_serv_cell_sib_resp_total_size(uint16_t total_size)
{
  nas_get_serv_cell_sib_resp_msg_v01* ptr = (nas_get_serv_cell_sib_resp_msg_v01*) payload;
  ptr->total_size_valid = TRUE;
  ptr->total_size = total_size;
}

/*========================================================
  QMI_NAS_GET_SERV_CELL_SIB_IND_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_serv_cell_sib_ind(void)
{
  payload = (void *) malloc(sizeof(nas_get_serv_cell_sib_ind_msg_v01));
  payload_len = sizeof(nas_get_serv_cell_sib_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_SERV_CELL_SIB_IND_MSG_V01;
}
void ril_utf_qmiril_if::set_qmi_nas_get_serv_cell_sib_ind_sequence(uint8_t sequence)
{
  nas_get_serv_cell_sib_ind_msg_v01* ptr = (nas_get_serv_cell_sib_ind_msg_v01*) payload;
  ptr->sequence = sequence;
}
void ril_utf_qmiril_if::set_qmi_nas_get_serv_cell_sib_ind_sib_pkt(uint8_t sib_pkt[], uint32_t sib_pkt_len)
{
  nas_get_serv_cell_sib_ind_msg_v01* ptr = (nas_get_serv_cell_sib_ind_msg_v01*) payload;
  ptr->sib_pkt_len = sib_pkt_len;
  memcpy(ptr->sib_pkt, sib_pkt, sib_pkt_len);
}

/*========================================================
  QMI_NAS_SET_LTE_BAND_PRIORITY_REQ_MSG_V01
========================================================*/
nas_set_lte_band_priority_req_msg_v01* ril_utf_qmiril_if::update_default_qmi_nas_set_lte_band_priority_req(void)
{
  payload = (void*)malloc(sizeof(nas_set_lte_band_priority_req_msg_v01));
  payload_len = sizeof(nas_set_lte_band_priority_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_SET_LTE_BAND_PRIORITY_REQ_MSG_V01;

  nas_set_lte_band_priority_req_msg_v01* ptr = (nas_set_lte_band_priority_req_msg_v01*)payload;
  return ptr;
}

/*========================================================
  QMI_NAS_SET_LTE_BAND_PRIORITY_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_set_lte_band_priority_resp(void)
{
  payload = (void*)malloc(sizeof(nas_set_lte_band_priority_resp_msg_v01));
  payload_len = sizeof(nas_set_lte_band_priority_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  msg_id = QMI_NAS_SET_LTE_BAND_PRIORITY_RESP_MSG_V01;
  service_id = QMI_NAS;
}

/*========================================================
  QMI_NAS_GET_LTE_BAND_PRIORITY_LIST_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_get_lte_band_priority_list_req(void)
{
  payload = (void*)malloc(sizeof(nas_get_lte_band_priority_list_req_msg_v01));
  payload_len = sizeof(nas_get_lte_band_priority_list_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_GET_LTE_BAND_PRIORITY_LIST_REQ_MSG_V01;
}

/*========================================================
  QMI_NAS_GET_LTE_BAND_PRIORITY_LIST_RESP_MSG_V01
========================================================*/
nas_get_lte_band_priority_list_resp_msg_v01* ril_utf_qmiril_if::update_default_qmi_nas_get_lte_band_priority_list_resp(void)
{
  payload = (void*)malloc(sizeof(nas_get_lte_band_priority_list_resp_msg_v01));
  payload_len = sizeof(nas_get_lte_band_priority_list_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  msg_id = QMI_NAS_GET_LTE_BAND_PRIORITY_LIST_RESP_MSG_V01;
  service_id = QMI_NAS;

  nas_get_lte_band_priority_list_resp_msg_v01* ptr = (nas_get_lte_band_priority_list_resp_msg_v01*)payload;
  return ptr;
}

/*========================================================
  QMI_NAS_PERFORM_INCREMENTAL_NETWORK_SCAN_REQ_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_perform_incremental_network_scan_req(nas_perform_incremental_network_scan_req_msg_v01 *req)
{
  payload = (void*)malloc(sizeof(nas_perform_incremental_network_scan_req_msg_v01));
  payload_len = sizeof(nas_perform_incremental_network_scan_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_PERFORM_INCREMENTAL_NETWORK_SCAN_REQ_MSG_V01;
  if (req) {
    memcpy(payload, req, payload_len);
  }
}
void ril_utf_qmiril_if::set_qmi_nas_perform_incremental_network_scan_req_network_type(nas_network_type_mask_type_v01 val)
{
  nas_perform_incremental_network_scan_req_msg_v01* ptr = (nas_perform_incremental_network_scan_req_msg_v01*)payload;
  ptr->network_type_valid = TRUE;
  ptr->network_type = val;
}
void ril_utf_qmiril_if::set_qmi_nas_perform_incremental_network_scan_req_scan_type(nas_nw_scan_type_enum_v01 val)
{
  nas_perform_incremental_network_scan_req_msg_v01* ptr = (nas_perform_incremental_network_scan_req_msg_v01*)payload;
  ptr->scan_type_valid = TRUE;
  ptr->scan_type = val;
}
void ril_utf_qmiril_if::set_qmi_nas_perform_incremental_network_scan_req_band_pref(nas_band_pref_mask_type_v01 val)
{
  nas_perform_incremental_network_scan_req_msg_v01* ptr = (nas_perform_incremental_network_scan_req_msg_v01*)payload;
  ptr->band_pref_valid = TRUE;
  ptr->band_pref = val;
}
void ril_utf_qmiril_if::set_qmi_nas_perform_incremental_network_scan_req_lte_band_pref(lte_band_pref_mask_type_v01 val)
{
  nas_perform_incremental_network_scan_req_msg_v01* ptr = (nas_perform_incremental_network_scan_req_msg_v01*)payload;
  ptr->lte_band_pref_valid = TRUE;
  ptr->lte_band_pref = val;
}
void ril_utf_qmiril_if::set_qmi_nas_perform_incremental_network_scan_req_tdscdma_band_pref(nas_tdscdma_band_pref_mask_type_v01 val)
{
  nas_perform_incremental_network_scan_req_msg_v01* ptr = (nas_perform_incremental_network_scan_req_msg_v01*)payload;
  ptr->tdscdma_band_pref_valid = TRUE;
  ptr->tdscdma_band_pref = val;
}
void ril_utf_qmiril_if::set_qmi_nas_perform_incremental_network_scan_req_wcdma_arfcn(uint32_t len, uint16_t val[])
{
  nas_perform_incremental_network_scan_req_msg_v01* ptr = (nas_perform_incremental_network_scan_req_msg_v01*)payload;
  ptr->wcdma_arfcn_valid = TRUE;
  ptr->wcdma_arfcn_len = len;
  for (uint32_t i = 0; i < len && i < NAS_WCDMA_ARFCN_LIST_MAX_V01; i++) {
    ptr->wcdma_arfcn[i] = val[i];
  }
}
void ril_utf_qmiril_if::set_qmi_nas_perform_incremental_network_scan_req_lte_arfcn(uint32_t len, uint32_t val[])
{
  nas_perform_incremental_network_scan_req_msg_v01* ptr = (nas_perform_incremental_network_scan_req_msg_v01*)payload;
  ptr->lte_arfcn_valid = TRUE;
  ptr->lte_arfcn_len = len;
  for (uint32_t i = 0; i < len && i < NAS_WCDMA_ARFCN_LIST_MAX_V01; i++) {
    ptr->lte_arfcn[i] = val[i];
  }
}
void ril_utf_qmiril_if::set_qmi_nas_perform_incremental_network_scan_req_search_periodicity(uint8_t val)
{
  nas_perform_incremental_network_scan_req_msg_v01* ptr = (nas_perform_incremental_network_scan_req_msg_v01*)payload;
  ptr->search_periodicity_valid = TRUE;
  ptr->search_periodicity = val;
}
void ril_utf_qmiril_if::set_qmi_nas_perform_incremental_network_scan_req_max_search_time(uint16_t val)
{
  nas_perform_incremental_network_scan_req_msg_v01* ptr = (nas_perform_incremental_network_scan_req_msg_v01*)payload;
  ptr->max_search_time_valid = TRUE;
  ptr->max_search_time = val;
}
void ril_utf_qmiril_if::set_qmi_nas_perform_incremental_network_scan_req_incremental_results(uint8_t val)
{
  nas_perform_incremental_network_scan_req_msg_v01* ptr = (nas_perform_incremental_network_scan_req_msg_v01*)payload;
  ptr->incremental_results_valid = TRUE;
  ptr->incremental_results = val;
}
void ril_utf_qmiril_if::set_qmi_nas_perform_incremental_network_scan_req_incremental_results_periodicity(uint8_t val)
{
  nas_perform_incremental_network_scan_req_msg_v01* ptr = (nas_perform_incremental_network_scan_req_msg_v01*)payload;
  ptr->incremental_results_periodicity_valid = TRUE;
  ptr->incremental_results_periodicity = val;
}
void ril_utf_qmiril_if::set_qmi_nas_perform_incremental_network_scan_req_preferred_plmn_list(uint32_t len, nas_plmn_id_ext_type_v01 val[])
{
  nas_perform_incremental_network_scan_req_msg_v01* ptr = (nas_perform_incremental_network_scan_req_msg_v01*)payload;
  ptr->preferred_plmn_list_valid = TRUE;
  ptr->preferred_plmn_list_len = len;
  for (uint32_t i = 0; i < len && i < NAS_PREFERRED_PLMN_LIST_MAX_V01; i++) {
    ptr->preferred_plmn_list[i] = val[i];
  }
}
void ril_utf_qmiril_if::set_qmi_nas_perform_incremental_network_scan_req_nr5g_band_pref(nas_nr5g_band_pref_mask_type_v01 val)
{
  nas_perform_incremental_network_scan_req_msg_v01* ptr = (nas_perform_incremental_network_scan_req_msg_v01*)payload;
  ptr->nr5g_band_pref_valid = TRUE;
  ptr->nr5g_band_pref = val;
}
void ril_utf_qmiril_if::set_qmi_nas_perform_incremental_network_scan_req_nr5g_arfcn(uint32_t len, uint32_t val[])
{
  nas_perform_incremental_network_scan_req_msg_v01* ptr = (nas_perform_incremental_network_scan_req_msg_v01*)payload;
  ptr->nr5g_arfcn_valid = TRUE;
  ptr->nr5g_arfcn_len = len;
  for (uint32_t i = 0; i < len && i < NAS_NR5G_ARFCN_LIST_MAX_V01; i++) {
    ptr->nr5g_arfcn[i] = val[i];
  }
}
void ril_utf_qmiril_if::set_qmi_nas_perform_incremental_network_scan_req_lte_band_pref_ext(lte_band_pref_mask_ext_type_v01 val)
{
  nas_perform_incremental_network_scan_req_msg_v01* ptr = (nas_perform_incremental_network_scan_req_msg_v01*)payload;
  ptr->lte_band_pref_ext_valid = TRUE;
  ptr->lte_band_pref_ext = val;
}

/*========================================================
  QMI_NAS_PERFORM_INCREMENTAL_NETWORK_SCAN_RESP_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_perform_incremental_network_scan_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void*)malloc(sizeof(nas_perform_incremental_network_scan_resp_msg_v01));
  payload_len = sizeof(nas_perform_incremental_network_scan_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  msg_id = QMI_NAS_PERFORM_INCREMENTAL_NETWORK_SCAN_RESP_MSG_V01;
  service_id = QMI_NAS;

  nas_perform_incremental_network_scan_resp_msg_v01* ptr = (nas_perform_incremental_network_scan_resp_msg_v01*)payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
  QMI_NAS_ABORT_SCAN_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_abort_scan_req(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_ABORT_SCAN_REQ_V01;
}

/*========================================================
  QMI_NAS_ABORT_SCAN_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_abort_scan_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void*)malloc(sizeof(nas_abort_scan_resp_msg_v01));
  payload_len = sizeof(nas_abort_scan_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  msg_id = QMI_NAS_ABORT_SCAN_RESP_V01;
  service_id = QMI_NAS;

  nas_abort_scan_resp_msg_v01* ptr = (nas_abort_scan_resp_msg_v01*)payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
  QMI_NAS_PERFORM_INCREMENTAL_NETWORK_SCAN_IND_MSG_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_nas_perform_incremental_network_scan_ind(nas_scan_status_enum_v01 scan_status)
{
  payload = (void *) malloc(sizeof(nas_perform_incremental_network_scan_ind_msg_v01));
  payload_len = sizeof(nas_perform_incremental_network_scan_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_NAS;
  msg_id = QMI_NAS_PERFORM_INCREMENTAL_NETWORK_SCAN_IND_MSG_V01;

  nas_perform_incremental_network_scan_ind_msg_v01* ptr = (nas_perform_incremental_network_scan_ind_msg_v01*) payload;
  ptr->scan_status = scan_status;
}
nas_3gpp_network_scan_result_type_v01 *ril_utf_qmiril_if::set_qmi_nas_perform_incremental_network_scan_ind_nas_network_scan_info(uint32_t len)
{
  nas_perform_incremental_network_scan_ind_msg_v01* ptr = (nas_perform_incremental_network_scan_ind_msg_v01*)payload;
  ptr->nas_network_scan_info_valid = TRUE;
  ptr->nas_network_scan_info_len = len;
  return ptr->nas_network_scan_info;
}
nas_csg_nw_info_type_v01 *ril_utf_qmiril_if::set_qmi_nas_perform_incremental_network_scan_ind_csg_info(uint32_t len)
{
  nas_perform_incremental_network_scan_ind_msg_v01* ptr = (nas_perform_incremental_network_scan_ind_msg_v01*)payload;
  ptr->csg_info_valid = TRUE;
  ptr->csg_info_len = len;
  return ptr->csg_info;
}
nas_nw_name_source_enum_type_v01 *ril_utf_qmiril_if::set_qmi_nas_perform_incremental_network_scan_ind_nw_name_source(uint32_t len, nas_nw_name_source_enum_type_v01 val[])
{
  nas_perform_incremental_network_scan_ind_msg_v01* ptr = (nas_perform_incremental_network_scan_ind_msg_v01*)payload;
  ptr->nw_name_source_valid = TRUE;
  ptr->nw_name_source_len = len;
  for (uint32_t i = 0; i < len && i < NAS_3GPP_NETWORK_INFO_LIST_MAX_V01; i++) {
    ptr->nw_name_source[i] = val[i];
  }
  return ptr->nw_name_source;
}
nas_signal_quality_e_type_v01 *ril_utf_qmiril_if::set_qmi_nas_perform_incremental_network_scan_ind_signal_quality(uint32_t len, nas_signal_quality_e_type_v01 val[])
{
  nas_perform_incremental_network_scan_ind_msg_v01* ptr = (nas_perform_incremental_network_scan_ind_msg_v01*)payload;
  ptr->signal_quality_valid = TRUE;
  ptr->signal_quality_len = len;
  for (uint32_t i = 0; i < len && i < NAS_3GPP_NETWORK_INFO_LIST_MAX_V01; i++) {
    ptr->signal_quality[i] = val[i];
  }
  return ptr->signal_quality;
}
int32_t *ril_utf_qmiril_if::set_qmi_nas_perform_incremental_network_scan_ind_signal_strength(uint32_t len, int32_t val[])
{
  nas_perform_incremental_network_scan_ind_msg_v01* ptr = (nas_perform_incremental_network_scan_ind_msg_v01*)payload;
  ptr->signal_strength_valid = TRUE;
  ptr->signal_strength_len = len;
  for (uint32_t i = 0; i < len && i < NAS_3GPP_NETWORK_INFO_LIST_MAX_V01; i++) {
    ptr->signal_strength[i] = val[i];
  }
  return ptr->signal_strength;
}
uint8_t *ril_utf_qmiril_if::set_qmi_nas_perform_incremental_network_scan_ind_sib2_upr_layer_ind_value(uint32_t len, uint8_t val[])
{
  nas_perform_incremental_network_scan_ind_msg_v01* ptr = (nas_perform_incremental_network_scan_ind_msg_v01*)payload;
  ptr->sib2_upr_layer_ind_value_valid = TRUE;
  ptr->sib2_upr_layer_ind_value_len = len;
  for (uint32_t i = 0; i < len && i < NAS_3GPP_NETWORK_INFO_LIST_MAX_V01; i++) {
    ptr->sib2_upr_layer_ind_value[i] = val[i];
  }
  return ptr->sib2_upr_layer_ind_value;
}


/*========================================================
  QMI_PDC_SET_FEATURE_REQ
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_pdc_set_feature_req()
{
    update_default_qmi_pdc_set_feature_req(0);
}
void ril_utf_qmiril_if::update_default_qmi_pdc_set_feature_req(uint32_t slot_id)
{
    payload = (void *) malloc(sizeof(pdc_set_feature_req_msg_v01));
    payload_len = sizeof(pdc_set_feature_req_msg_v01);
    memset(payload, 0x00, payload_len);
    payload_type = ril_utf_qmi_request;
    service_id = QMI_PDC;
    msg_id = QMI_PDC_SET_FEATURE_REQ_V01;

    pdc_set_feature_req_msg_v01* ptr = (pdc_set_feature_req_msg_v01*)payload;
    ptr->slot_id = slot_id;
}
void ril_utf_qmiril_if::set_default_qmi_pdc_set_feature_req_selection_mode(pdc_selection_mode_enum_v01 selection_mode)
{
    pdc_set_feature_req_msg_v01* ptr = (pdc_set_feature_req_msg_v01*)payload;
    ptr->selection_mode_valid = true;
    ptr->selection_mode = selection_mode;
    ptr->flex_mapping_valid = true;
    ptr->flex_mapping = false;
    ptr->carrier_valid = true;
    ptr->carrier = PDC_SELECTION_OPEN_MARKET_V01;
    ptr->refresh_mode_valid = true;
    ptr->refresh_mode = PDC_REFRESH_MODE_ENABLED_V01;
}

/*========================================================
  QMI_PDC_SET_FEATURE_RESP
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_pdc_set_feature_resp()
{
    update_default_qmi_pdc_set_feature_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
}
void ril_utf_qmiril_if::update_default_qmi_pdc_set_feature_resp(qmi_result_type_v01 result,qmi_error_type_v01 error)
{

    payload = (void *) malloc(sizeof(pdc_set_feature_resp_msg_v01));
    payload_len = sizeof(pdc_set_feature_resp_msg_v01);
    memset(payload, 0x00, payload_len);
    payload_type = ril_utf_qmi_response;
    service_id = QMI_PDC;
    msg_id = QMI_PDC_SET_FEATURE_RESP_V01;

    pdc_set_feature_resp_msg_v01* ptr = (pdc_set_feature_resp_msg_v01*) payload;
    ptr->resp.result = result;
    ptr->resp.error = error;
}

/*========================================================
  QMI_PDC_SET_FEATURE_IND
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_pdc_set_feature_ind()
{
    update_default_qmi_pdc_set_feature_ind(QMI_ERR_NONE_V01);
}
void ril_utf_qmiril_if::update_default_qmi_pdc_set_feature_ind(qmi_error_type_v01 error)
{
    payload = (void *) malloc(sizeof(pdc_set_feature_ind_msg_v01));
    payload_len = sizeof(pdc_set_feature_ind_msg_v01);
    memset(payload, 0x00, payload_len);
    payload_type = ril_utf_qmi_indication;
    service_id = QMI_PDC;
    msg_id = QMI_PDC_SET_FEATURE_IND_V01;

    pdc_set_feature_ind_msg_v01* ptr = (pdc_set_feature_ind_msg_v01*) payload;
    ptr->error = error;
}

/*========================================================
  QMI_PDC_GET_SELECTED_CONFIG_REQ
========================================================*/
pdc_get_selected_config_req_msg_v01* ril_utf_qmiril_if::update_default_qmi_pdc_get_selected_config_req()
{
  payload = (void *) malloc(sizeof(pdc_get_selected_config_req_msg_v01));
  payload_len = sizeof(pdc_get_selected_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_PDC;
  msg_id = QMI_PDC_GET_SELECTED_CONFIG_REQ_V01;

  pdc_get_selected_config_req_msg_v01* ptr = (pdc_get_selected_config_req_msg_v01*)payload;
  return ptr;
}
void ril_utf_qmiril_if::set_qmi_pdc_get_selected_config_req_config_type(pdc_config_type_enum_v01 config_type)
{
  pdc_get_selected_config_req_msg_v01* ptr = (pdc_get_selected_config_req_msg_v01*)payload;
  ptr->config_type = config_type;
}
void ril_utf_qmiril_if::set_qmi_pdc_get_selected_config_req_subscription_id(uint8_t subscription_id)
{
  pdc_get_selected_config_req_msg_v01* ptr = (pdc_get_selected_config_req_msg_v01*)payload;
  ptr->subscription_id_valid = 1;
  ptr->subscription_id = subscription_id;
}


/*========================================================
  QMI_PDC_GET_SELECTED_CONFIG_RESP
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_pdc_get_selected_config_resp(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(pdc_get_selected_config_resp_msg_v01));
  payload_len = sizeof(pdc_get_selected_config_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_PDC;
  msg_id = QMI_PDC_GET_SELECTED_CONFIG_RESP_V01;

  pdc_get_selected_config_resp_msg_v01* ptr = (pdc_get_selected_config_resp_msg_v01*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}
void ril_utf_qmiril_if::update_default_qmi_pdc_get_selected_config_resp()
{
  update_default_qmi_pdc_get_selected_config_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
}

/*========================================================
  QMI_PDC_GET_SELECTED_CONFIG_IND
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_pdc_get_selected_config_ind()
{
  payload = (void *) malloc(sizeof(pdc_get_selected_config_ind_msg_v01));
  payload_len = sizeof(pdc_get_selected_config_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_PDC;
  msg_id = QMI_PDC_GET_SELECTED_CONFIG_IND_V01;
}

void ril_utf_qmiril_if::set_qmi_pdc_get_selected_config_ind_active_config_id(const uint8_t config_id[], uint32_t config_id_len)
{
  pdc_get_selected_config_ind_msg_v01* ptr = (pdc_get_selected_config_ind_msg_v01*)payload;
  ptr->active_config_id_valid = 1;
  ptr->active_config_id_len = config_id_len;
  memcpy(ptr->active_config_id, config_id, config_id_len);
}

void ril_utf_qmiril_if::set_qmi_pdc_get_selected_config_ind_pending_config_id(const uint8_t config_id[], uint32_t config_id_len)
{
  pdc_get_selected_config_ind_msg_v01* ptr = (pdc_get_selected_config_ind_msg_v01*)payload;
  ptr->pending_config_id_valid = 1;
  ptr->pending_config_id_len = config_id_len;
  memcpy(ptr->pending_config_id, config_id, config_id_len);
}

/*========================================================
  QMI_PDC_DEACTIVATE_CONFIG_REQ
========================================================*/
pdc_deactivate_config_req_msg_v01* ril_utf_qmiril_if::update_default_qmi_pdc_deactivate_config_req()
{
  payload = (void *) malloc(sizeof(pdc_deactivate_config_req_msg_v01));
  payload_len = sizeof(pdc_deactivate_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_PDC;
  msg_id = QMI_PDC_DEACTIVATE_CONFIG_REQ_V01;
  pdc_deactivate_config_req_msg_v01* ptr = (pdc_deactivate_config_req_msg_v01*) payload;
  return ptr;
}
void ril_utf_qmiril_if::set_qmi_pdc_deactivate_config_req_subscription_id(uint8_t subscription_id)
{
  pdc_deactivate_config_req_msg_v01* ptr = (pdc_deactivate_config_req_msg_v01*) payload;
  ptr->subscription_id_valid = 1;
  ptr->subscription_id = subscription_id;
}
void ril_utf_qmiril_if::set_qmi_pdc_deactivate_config_req_config_type(pdc_config_type_enum_v01 config_type)
{
  pdc_deactivate_config_req_msg_v01* ptr = (pdc_deactivate_config_req_msg_v01*) payload;
  ptr->config_type = config_type;
}

/*========================================================
  QMI_PDC_DEACTIVATE_CONFIG_RESP
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_pdc_deactivate_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(pdc_deactivate_config_resp_msg_v01));
  payload_len = sizeof(pdc_deactivate_config_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_PDC;
  msg_id = QMI_PDC_DEACTIVATE_CONFIG_RESP_V01;

  pdc_deactivate_config_resp_msg_v01* ptr = (pdc_deactivate_config_resp_msg_v01*)payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}
void ril_utf_qmiril_if::update_default_qmi_pdc_deactivate_config_resp()
{
  update_default_qmi_pdc_deactivate_config_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
}

/*========================================================
  QMI_PDC_DEACTIVATE_CONFIG_IND
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_pdc_deactivate_config_ind()
{
  payload = (void *) malloc(sizeof(pdc_deactivate_config_ind_msg_v01));
  payload_len = sizeof(pdc_deactivate_config_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_PDC;
  msg_id = QMI_PDC_DEACTIVATE_CONFIG_IND_V01;
}

/*========================================================
  QMI_PDC_DELETE_CONFIG_REQ
========================================================*/
pdc_delete_config_req_msg_v01* ril_utf_qmiril_if::update_default_qmi_pdc_delete_config_req()
{
  payload = (void *) malloc(sizeof(pdc_delete_config_req_msg_v01));
  payload_len = sizeof(pdc_delete_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_PDC;
  msg_id = QMI_PDC_DELETE_CONFIG_REQ_V01;

  pdc_delete_config_req_msg_v01* ptr = (pdc_delete_config_req_msg_v01*)payload;
  return ptr;
}
void ril_utf_qmiril_if::set_qmi_pdc_delete_config_req_config_type(pdc_config_type_enum_v01 config_type)
{
  pdc_delete_config_req_msg_v01* ptr = (pdc_delete_config_req_msg_v01*)payload;
  ptr->config_type = config_type;
}
void ril_utf_qmiril_if::set_qmi_pdc_delete_config_req_config_id(uint8_t config_id[], uint32_t config_id_len)
{
  pdc_delete_config_req_msg_v01* ptr = (pdc_delete_config_req_msg_v01*) payload;
  if (config_id_len == 0) {
    ptr->config_id_valid =0;
  } else {
    ptr->config_id_valid = 1;
    ptr->config_id_len = config_id_len;
    memcpy(ptr->config_id, config_id, config_id_len);
  }
}

/*========================================================
  QMI_PDC_DELETE_CONFIG_RESP
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_pdc_delete_config_resp(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(pdc_delete_config_resp_msg_v01));
  payload_len = sizeof(pdc_delete_config_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_PDC;
  msg_id = QMI_PDC_DELETE_CONFIG_RESP_V01;

  pdc_delete_config_resp_msg_v01* ptr = (pdc_delete_config_resp_msg_v01*)payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}
void ril_utf_qmiril_if::update_default_qmi_pdc_delete_config_resp()
{
  update_default_qmi_pdc_delete_config_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
}

/*========================================================
  QMI_PDC_DELETE_CONFIG_IND
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_pdc_delete_config_ind()
{
  payload = (void *) malloc(sizeof(pdc_delete_config_ind_msg_v01));
  payload_len = sizeof(pdc_delete_config_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_PDC;
  msg_id = QMI_PDC_DELETE_CONFIG_IND_V01;
}

/*========================================================
  QMI_PDC_LOAD_CONFIG_REQ
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_pdc_load_config_req()
{
  payload = (void *) malloc(sizeof(pdc_load_config_req_msg_v01));
  payload_len = sizeof(pdc_load_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_PDC;
  msg_id = QMI_PDC_LOAD_CONFIG_REQ_V01;
}
void ril_utf_qmiril_if::set_qmi_pdc_load_config_req_config_id(uint8_t config_id[], uint32_t config_id_len)
{
  pdc_load_config_req_msg_v01* ptr = (pdc_load_config_req_msg_v01*) payload;
  ptr->load_config_info.config_id_len = config_id_len;
  memcpy(ptr->load_config_info.config_id, config_id, config_id_len);
}
void ril_utf_qmiril_if::set_qmi_pdc_load_config_req_config_type(pdc_config_type_enum_v01 config_type)
{
  pdc_load_config_req_msg_v01* ptr = (pdc_load_config_req_msg_v01*) payload;
  ptr->load_config_info.config_type = config_type;
}
void ril_utf_qmiril_if::set_qmi_pdc_load_config_req_config_size(uint32_t config_size)
{
  pdc_load_config_req_msg_v01* ptr = (pdc_load_config_req_msg_v01*) payload;
  ptr->load_config_info.total_config_size = config_size;
}
void ril_utf_qmiril_if::set_qmi_pdc_load_config_req_config_frame(uint8_t config_frame[], uint32_t config_frame_len)
{
  pdc_load_config_req_msg_v01* ptr = (pdc_load_config_req_msg_v01*) payload;
  ptr->load_config_info.config_frame_len = config_frame_len;
  memcpy(ptr->load_config_info.config_frame, config_frame, config_frame_len);
}

/*========================================================
  QMI_PDC_LOAD_CONFIG_RESP
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_pdc_load_config_resp(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(pdc_load_config_resp_msg_v01));
  payload_len = sizeof(pdc_load_config_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_PDC;
  msg_id = QMI_PDC_LOAD_CONFIG_RESP_V01;

  pdc_load_config_resp_msg_v01* ptr = (pdc_load_config_resp_msg_v01*)payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}
void ril_utf_qmiril_if::update_default_qmi_pdc_load_config_resp()
{
  update_default_qmi_pdc_load_config_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
}

/*========================================================
  QMI_PDC_LOAD_CONFIG_IND
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_pdc_load_config_ind()
{
  payload = (void *) malloc(sizeof(pdc_load_config_ind_msg_v01));
  payload_len = sizeof(pdc_load_config_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_PDC;
  msg_id = QMI_PDC_LOAD_CONFIG_IND_V01;
}

/*========================================================
  QMI_PDC_SET_SELECTED_CONFIG_REQ
========================================================*/
pdc_set_selected_config_req_msg_v01* ril_utf_qmiril_if::update_default_qmi_pdc_set_selected_config_req()
{
  payload = (void *) malloc(sizeof(pdc_set_selected_config_req_msg_v01));
  payload_len = sizeof(pdc_set_selected_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_PDC;
  msg_id = QMI_PDC_SET_SELECTED_CONFIG_REQ_V01;

  pdc_set_selected_config_req_msg_v01* ptr = (pdc_set_selected_config_req_msg_v01*)payload;
  return ptr;
}
void ril_utf_qmiril_if::set_qmi_pdc_set_selected_config_req_config_type(pdc_config_type_enum_v01 config_type)
{
  pdc_set_selected_config_req_msg_v01* ptr = (pdc_set_selected_config_req_msg_v01*)payload;
  ptr->new_config_info.config_type = config_type;
}
void ril_utf_qmiril_if::set_qmi_pdc_set_selected_config_req_subscription_id(uint8_t subscription_id)
{
  pdc_set_selected_config_req_msg_v01* ptr = (pdc_set_selected_config_req_msg_v01*)payload;
  ptr->subscription_id_valid = 1;
  ptr->subscription_id = subscription_id;
}
void ril_utf_qmiril_if::set_qmi_pdc_set_selected_config_req_config_id(uint8_t config_id[], uint32_t config_id_len)
{
  pdc_set_selected_config_req_msg_v01* ptr = (pdc_set_selected_config_req_msg_v01*)payload;
  ptr->new_config_info.config_id_len = config_id_len;
  memcpy(ptr->new_config_info.config_id, config_id, config_id_len);
}

/*========================================================
  QMI_PDC_SET_SELECTED_CONFIG_RESP
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_pdc_set_selected_config_resp(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(pdc_set_selected_config_resp_msg_v01));
  payload_len = sizeof(pdc_set_selected_config_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_PDC;
  msg_id = QMI_PDC_SET_SELECTED_CONFIG_RESP_V01;

  pdc_set_selected_config_resp_msg_v01* ptr = (pdc_set_selected_config_resp_msg_v01*)payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}
void ril_utf_qmiril_if::update_default_qmi_pdc_set_selected_config_resp()
{
  update_default_qmi_pdc_set_selected_config_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
}

/*========================================================
  QMI_PDC_SET_SELECTED_CONFIG_IND
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_pdc_set_selected_config_ind()
{
  payload = (void *) malloc(sizeof(pdc_set_selected_config_ind_msg_v01));
  payload_len = sizeof(pdc_set_selected_config_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_PDC;
  msg_id = QMI_PDC_SET_SELECTED_CONFIG_IND_V01;
}

/*========================================================
  QMI_PDC_ACTIVATE_CONFIG_REQ
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_pdc_activate_config_req()
{
  payload = (void *) malloc(sizeof(pdc_activate_config_req_msg_v01));
  payload_len = sizeof(pdc_activate_config_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_PDC;
  msg_id = QMI_PDC_ACTIVATE_CONFIG_REQ_V01;
}

void ril_utf_qmiril_if::update_qmi_pdc_activate_config_req_subscription_id(uint8_t subscription_id)
{
  pdc_activate_config_req_msg_v01* ptr = (pdc_activate_config_req_msg_v01*)payload;
  ptr->subscription_id_valid = 1;
  ptr->subscription_id = subscription_id;
}
void ril_utf_qmiril_if::update_qmi_pdc_activate_config_req_config_type(pdc_config_type_enum_v01 config_type)
{
  pdc_activate_config_req_msg_v01* ptr = (pdc_activate_config_req_msg_v01*)payload;
  ptr->config_type = config_type;
}

/*========================================================
  QMI_PDC_ACTIVATE_CONFIG_RESP
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_pdc_activate_config_resp(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(pdc_activate_config_resp_msg_v01));
  payload_len = sizeof(pdc_activate_config_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_PDC;
  msg_id = QMI_PDC_ACTIVATE_CONFIG_RESP_V01;

  pdc_activate_config_resp_msg_v01* ptr = (pdc_activate_config_resp_msg_v01*)payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}
void ril_utf_qmiril_if::update_default_qmi_pdc_activate_config_resp()
{
  update_default_qmi_pdc_activate_config_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
}

/*========================================================
  QMI_PDC_ACTIVATE_CONFIG_IND
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_pdc_activate_config_ind()
{
  payload = (void *) malloc(sizeof(pdc_activate_config_ind_msg_v01));
  payload_len = sizeof(pdc_activate_config_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_PDC;
  msg_id = QMI_PDC_ACTIVATE_CONFIG_IND_V01;
}


/*========================================================
  QMI_PDC_LIST_CONFIGS_REQ
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_pdc_list_config_req()
{
    payload = (void *) malloc(sizeof(pdc_list_configs_req_msg_v01));
    payload_len = sizeof(pdc_list_configs_req_msg_v01);
    memset(payload, 0x00, payload_len);
    payload_type = ril_utf_qmi_request;
    service_id = QMI_PDC;
    msg_id = QMI_PDC_LIST_CONFIGS_REQ_V01;
}

void ril_utf_qmiril_if::update_qmi_pdc_list_config_req_config_type(pdc_config_type_enum_v01 config_type)
{
  pdc_list_configs_req_msg_v01* ptr = (pdc_list_configs_req_msg_v01*)payload;
  ptr->config_type_valid = true;
  ptr->config_type = config_type;
}

/*========================================================
  QMI_PDC_LIST_CONFIGS_RSP
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_pdc_list_config_resp()
{
    payload = (void *) malloc(sizeof(pdc_list_configs_resp_msg_v01));
    payload_len = sizeof(pdc_list_configs_resp_msg_v01);
    memset(payload, 0x00, payload_len);
    payload_type = ril_utf_qmi_response;
    service_id = QMI_PDC;
    msg_id = QMI_PDC_LIST_CONFIGS_RESP_V01;
}

/*========================================================
  QMI_PDC_LIST_CONFIGS_IND
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_pdc_list_config_ind()
{
    payload = (void *) malloc(sizeof(pdc_list_configs_ind_msg_v01));
    payload_len = sizeof(pdc_list_configs_ind_msg_v01);
    memset(payload, 0x00, payload_len);
    payload_type = ril_utf_qmi_indication;
    service_id = QMI_PDC;
    msg_id = QMI_PDC_LIST_CONFIGS_IND_V01;
}

/*========================================================
  QMI_PDC_GET_CONFIG_INFO_REQ
========================================================*/
pdc_get_config_info_req_msg_v01* ril_utf_qmiril_if::update_default_qmi_pdc_get_config_info_req()
{
  payload = (void *) malloc(sizeof(pdc_get_config_info_req_msg_v01));
  payload_len = sizeof(pdc_get_config_info_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_PDC;
  msg_id = QMI_PDC_GET_CONFIG_INFO_REQ_V01;

  pdc_get_config_info_req_msg_v01* ptr = (pdc_get_config_info_req_msg_v01*)payload;
  return ptr;
}
void ril_utf_qmiril_if::set_qmi_pdc_get_config_info_req_config_type(pdc_config_type_enum_v01 config_type)
{
  pdc_get_config_info_req_msg_v01* ptr = (pdc_get_config_info_req_msg_v01*)payload;
  ptr->new_config_info.config_type = config_type;
}
void ril_utf_qmiril_if::set_qmi_pdc_get_config_info_req_config_id(uint8_t config_id[], uint32_t config_id_len)
{
  pdc_get_config_info_req_msg_v01* ptr = (pdc_get_config_info_req_msg_v01*)payload;
  ptr->new_config_info.config_id_len = config_id_len;
  memcpy(ptr->new_config_info.config_id, config_id, config_id_len);
}

/*========================================================
  QMI_PDC_GET_CONFIG_INFO_RESP
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_pdc_get_config_info_resp(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(pdc_get_config_info_resp_msg_v01));
  payload_len = sizeof(pdc_get_config_info_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_PDC;
  msg_id = QMI_PDC_GET_CONFIG_INFO_RESP_V01;

  pdc_get_config_info_resp_msg_v01* ptr = (pdc_get_config_info_resp_msg_v01*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}
void ril_utf_qmiril_if::update_default_qmi_pdc_get_config_info_resp()
{
  update_default_qmi_pdc_get_config_info_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
}

/*========================================================
  QMI_PDC_GET_CONFIG_INFO_IND
========================================================*/
pdc_get_config_info_ind_msg_v01* ril_utf_qmiril_if::update_default_qmi_pdc_get_config_info_ind()
{
  payload = (void *) malloc(sizeof(pdc_get_config_info_ind_msg_v01));
  payload_len = sizeof(pdc_get_config_info_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_PDC;
  msg_id = QMI_PDC_GET_CONFIG_INFO_IND_V01;

  pdc_get_config_info_ind_msg_v01* ptr = (pdc_get_config_info_ind_msg_v01*) payload;
  return ptr;
}

/*========================================================
  QMI_LTE_INDICATION_REGISTER_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_lte_indication_register_req_msg(void)
{
  payload = (void *) malloc(sizeof(qmi_lte_indication_reg_req_msg_v01));
  payload_len = sizeof(qmi_lte_indication_reg_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_LTE;
  msg_id = QMI_LTE_INDICATION_REGISTER_REQ_V01;
}
void ril_utf_qmiril_if::set_qmi_lte_indication_register_req_indication_bitmask(qmi_lte_indication_reg_mask_t_v01 indication_bitmask)
{
  qmi_lte_indication_reg_req_msg_v01 *ptr = (qmi_lte_indication_reg_req_msg_v01 *)payload;
  ptr->indication_bitmask = indication_bitmask;
}

/*========================================================
  QMI_LTE_INDICATION_REGISTER_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_lte_indication_register_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(qmi_lte_indication_reg_resp_msg_v01));
  payload_len = sizeof(qmi_lte_indication_reg_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_LTE;
  msg_id = QMI_LTE_INDICATION_REGISTER_RESP_V01;

  qmi_lte_indication_reg_resp_msg_v01 *ptr = (qmi_lte_indication_reg_resp_msg_v01 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

/*========================================================
  QMI_LTE_DISC_TERMINATE_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_lte_disc_terminate_req_msg(void)
{
  payload = (void *) malloc(sizeof(qmi_lte_disc_terminate_req_msg_v01));
  payload_len = sizeof(qmi_lte_disc_terminate_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_LTE;
  msg_id = QMI_LTE_DISC_TERMINATE_REQ_V01;
}
void ril_utf_qmiril_if::set_qmi_lte_disc_terminate_req_os_app_id(char * os_app_id)
{
  qmi_lte_disc_terminate_req_msg_v01 *ptr = (qmi_lte_disc_terminate_req_msg_v01 *)payload;
  memcpy(ptr->OsAppId, os_app_id, strlen(os_app_id));
}

/*========================================================
  QMI_LTE_DISC_TERMINATE_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_lte_disc_terminate_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(qmi_lte_disc_terminate_resp_msg_v01));
  payload_len = sizeof(qmi_lte_disc_terminate_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_LTE;
  msg_id = QMI_LTE_DISC_TERMINATE_RESP_V01;

  qmi_lte_disc_terminate_resp_msg_v01 *ptr = (qmi_lte_disc_terminate_resp_msg_v01 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

/*========================================================
  QMI_LTE_DISC_GET_SERVICE_STATUS_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_lte_disc_get_service_status_req_msg(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_LTE;
  msg_id = QMI_LTE_DISC_GET_SERVICE_STATUS_REQ_V01;
}

/*========================================================
  QMI_LTE_DISC_GET_SERVICE_STATUS_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_lte_disc_get_service_status_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(qmi_lte_disc_get_service_status_resp_msg_v01));
  payload_len = sizeof(qmi_lte_disc_get_service_status_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_LTE;
  msg_id = QMI_LTE_DISC_GET_SERVICE_STATUS_RESP_V01;

  qmi_lte_disc_get_service_status_resp_msg_v01 *ptr = (qmi_lte_disc_get_service_status_resp_msg_v01 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_lte_disc_get_service_status_resp_publish_allowed(boolean publish_allowed)
{
  qmi_lte_disc_get_service_status_resp_msg_v01 *ptr = (qmi_lte_disc_get_service_status_resp_msg_v01 *)payload;
  ptr->PublishAllowed_valid = TRUE;
  ptr->PublishAllowed = publish_allowed;
}
void ril_utf_qmiril_if::set_qmi_lte_disc_get_service_status_resp_subscribe_allowed(boolean subscribe_allowed)
{
  qmi_lte_disc_get_service_status_resp_msg_v01 *ptr = (qmi_lte_disc_get_service_status_resp_msg_v01 *)payload;
  ptr->SubscribeAllowed_valid = TRUE;
  ptr->SubscribeAllowed = subscribe_allowed;
}

/*========================================================
  QMI_LTE_DISC_PUBLISH_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_lte_disc_publish_req_msg(void)
{
  payload = (void *) malloc(sizeof(qmi_lte_disc_publish_req_msg_v01));
  payload_len = sizeof(qmi_lte_disc_publish_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_LTE;
  msg_id = QMI_LTE_DISC_PUBLISH_REQ_V01;
}
void ril_utf_qmiril_if::set_qmi_lte_disc_publish_req_os_app_id(char * os_app_id)
{
  qmi_lte_disc_publish_req_msg_v01 *ptr = (qmi_lte_disc_publish_req_msg_v01 *)payload;
  strlcpy((ptr->OsAppId), os_app_id, sizeof(ptr->OsAppId));
}
void ril_utf_qmiril_if::set_qmi_lte_disc_publish_req_discovery_type(qmi_lte_discovery_type_enum_v01 discovery_type)
{
  qmi_lte_disc_publish_req_msg_v01 *ptr = (qmi_lte_disc_publish_req_msg_v01 *)payload;
  ptr->DiscoveryType_valid = TRUE;
  ptr->DiscoveryType = discovery_type;
}
void ril_utf_qmiril_if::set_qmi_lte_disc_publish_req_expression(char * expression)
{
  qmi_lte_disc_publish_req_msg_v01 *ptr = (qmi_lte_disc_publish_req_msg_v01 *)payload;
  ptr->Expression_valid = TRUE;
  strlcpy((ptr->Expression.prose_app_id_name), expression, sizeof(ptr->Expression.prose_app_id_name));
}
void ril_utf_qmiril_if::set_qmi_lte_disc_publish_req_publish_duration(uint32_t publish_duration)
{
  qmi_lte_disc_publish_req_msg_v01 *ptr = (qmi_lte_disc_publish_req_msg_v01 *)payload;
  ptr->PublishDuration_valid = TRUE;
  ptr->PublishDuration = publish_duration;
}
void ril_utf_qmiril_if::set_qmi_lte_disc_publish_req_expression_validity_time(uint32_t expression_validity_time)
{
  qmi_lte_disc_publish_req_msg_v01 *ptr = (qmi_lte_disc_publish_req_msg_v01 *)payload;
  ptr->ExpressionValidityTime_valid = TRUE;
  ptr->ExpressionValidityTime = expression_validity_time;
}
void ril_utf_qmiril_if::set_qmi_lte_disc_publish_req_meta_data(char * meta_data)
{
  qmi_lte_disc_publish_req_msg_v01 *ptr = (qmi_lte_disc_publish_req_msg_v01 *)payload;
  ptr->MetaData_valid = TRUE;
  strlcpy((ptr->MetaData), meta_data, sizeof(ptr->MetaData));
}

/*========================================================
  QMI_LTE_DISC_PUBLISH_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_lte_disc_publish_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(qmi_lte_disc_publish_resp_msg_v01));
  payload_len = sizeof(qmi_lte_disc_publish_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_LTE;
  msg_id = QMI_LTE_DISC_PUBLISH_RESP_V01;

  qmi_lte_disc_publish_resp_msg_v01 *ptr = (qmi_lte_disc_publish_resp_msg_v01 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_lte_disc_publish_resp_os_app_id(char * os_app_id)
{
  qmi_lte_disc_publish_resp_msg_v01 *ptr = (qmi_lte_disc_publish_resp_msg_v01 *)payload;
  ptr->OsAppId_valid = TRUE;
  memcpy(ptr->OsAppId, os_app_id, strlen(os_app_id));
}
void ril_utf_qmiril_if::set_qmi_lte_disc_publish_resp_expression(char * expression)
{
  qmi_lte_disc_publish_resp_msg_v01 *ptr = (qmi_lte_disc_publish_resp_msg_v01 *)payload;
  ptr->Expression_valid = TRUE;
  memcpy(ptr->Expression.prose_app_id_name, expression, strlen(expression));
}
void ril_utf_qmiril_if::set_qmi_lte_disc_publish_resp_expression_result(qmi_lte_disc_result_type_enum_v01 expression_result)
{
  qmi_lte_disc_publish_resp_msg_v01 *ptr = (qmi_lte_disc_publish_resp_msg_v01 *)payload;
  ptr->ExpressionResult_valid = TRUE;
  ptr->ExpressionResult = expression_result;
}

/*========================================================
  QMI_LTE_DISC_SUBSCRIBE_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_lte_disc_subscribe_req_msg(void)
{
  payload = (void *) malloc(sizeof(qmi_lte_disc_subscribe_req_msg_v01));
  payload_len = sizeof(qmi_lte_disc_subscribe_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_LTE;
  msg_id = QMI_LTE_DISC_SUBSCRIBE_REQ_V01;
}
void ril_utf_qmiril_if::set_qmi_lte_disc_subscribe_req_os_app_id(char * os_app_id)
{
  qmi_lte_disc_subscribe_req_msg_v01 *ptr = (qmi_lte_disc_subscribe_req_msg_v01 *)payload;
  memcpy(ptr->OsAppId, os_app_id, strlen(os_app_id));
}
void ril_utf_qmiril_if::set_qmi_lte_disc_subscribe_req_discovery_type(qmi_lte_discovery_type_enum_v01 discovery_type)
{
  qmi_lte_disc_subscribe_req_msg_v01 *ptr = (qmi_lte_disc_subscribe_req_msg_v01 *)payload;
  ptr->DiscoveryType_valid = TRUE;
  ptr->DiscoveryType = discovery_type;
}
void ril_utf_qmiril_if::set_qmi_lte_disc_subscribe_req_expression(char * expression)
{
  qmi_lte_disc_subscribe_req_msg_v01 *ptr = (qmi_lte_disc_subscribe_req_msg_v01 *)payload;
  ptr->Expression_valid = TRUE;
  memcpy(ptr->Expression.prose_app_id_name, expression, strlen(expression));
}
void ril_utf_qmiril_if::set_qmi_lte_disc_subscribe_req_subscribe_duration(uint32_t subscribe_duration)
{
  qmi_lte_disc_subscribe_req_msg_v01 *ptr = (qmi_lte_disc_subscribe_req_msg_v01 *)payload;
  ptr->SubscribeDuration_valid = TRUE;
  ptr->SubscribeDuration = subscribe_duration;
}
void ril_utf_qmiril_if::set_qmi_lte_disc_subscribe_req_expression_validity_time(uint32_t expression_validity_time)
{
  qmi_lte_disc_subscribe_req_msg_v01 *ptr = (qmi_lte_disc_subscribe_req_msg_v01 *)payload;
  ptr->ExpressionValidityTime_valid = TRUE;
  ptr->ExpressionValidityTime = expression_validity_time;
}

/*========================================================
  QMI_LTE_DISC_SUBSCRIBE_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_lte_disc_subscribe_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(qmi_lte_disc_subscribe_resp_msg_v01));
  payload_len = sizeof(qmi_lte_disc_subscribe_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_LTE;
  msg_id = QMI_LTE_DISC_SUBSCRIBE_RESP_V01;

  qmi_lte_disc_subscribe_resp_msg_v01 *ptr = (qmi_lte_disc_subscribe_resp_msg_v01 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_lte_disc_subscribe_resp_os_app_id(char * os_app_id)
{
  qmi_lte_disc_subscribe_resp_msg_v01 *ptr = (qmi_lte_disc_subscribe_resp_msg_v01 *)payload;
  ptr->OsAppId_valid = TRUE;
  memcpy(ptr->OsAppId, os_app_id, strlen(os_app_id));
}
void ril_utf_qmiril_if::set_qmi_lte_disc_subscribe_resp_expression(char * expression)
{
  qmi_lte_disc_subscribe_resp_msg_v01 *ptr = (qmi_lte_disc_subscribe_resp_msg_v01 *)payload;
  ptr->Expression_valid = TRUE;
  memcpy(ptr->Expression.prose_app_id_name, expression, strlen(expression));
}
void ril_utf_qmiril_if::set_qmi_lte_disc_subscribe_resp_expression_result(qmi_lte_disc_result_type_enum_v01 expression_result)
{
  qmi_lte_disc_subscribe_resp_msg_v01 *ptr = (qmi_lte_disc_subscribe_resp_msg_v01 *)payload;
  ptr->ExpressionResult_valid = TRUE;
  ptr->ExpressionResult = expression_result;
}

/*========================================================
  QMI_LTE_DISC_CANCEL_PUBLISH_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_lte_disc_cancel_publish_req_msg(void)
{
  payload = (void *) malloc(sizeof(qmi_lte_disc_cancel_publish_req_msg_v01));
  payload_len = sizeof(qmi_lte_disc_cancel_publish_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_LTE;
  msg_id = QMI_LTE_DISC_CANCEL_PUBLISH_REQ_V01;
}
void ril_utf_qmiril_if::set_qmi_lte_disc_cancel_publish_req_os_app_id(char * os_app_id)
{
  qmi_lte_disc_cancel_publish_req_msg_v01 *ptr = (qmi_lte_disc_cancel_publish_req_msg_v01 *)payload;
  memcpy(ptr->OsAppId, os_app_id, strlen(os_app_id));
}
void ril_utf_qmiril_if::set_qmi_lte_disc_cancel_publish_req_expression(char * expression)
{
  qmi_lte_disc_cancel_publish_req_msg_v01 *ptr = (qmi_lte_disc_cancel_publish_req_msg_v01 *)payload;
  ptr->Expression_valid = TRUE;
  memcpy(ptr->Expression.prose_app_id_name, expression, strlen(expression));
}

/*========================================================
  QMI_LTE_DISC_CANCEL_PUBLISH_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_lte_disc_cancel_publish_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(qmi_lte_disc_cancel_publish_resp_msg_v01));
  payload_len = sizeof(qmi_lte_disc_cancel_publish_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_LTE;
  msg_id = QMI_LTE_DISC_CANCEL_PUBLISH_RESP_V01;

  qmi_lte_disc_cancel_publish_resp_msg_v01 *ptr = (qmi_lte_disc_cancel_publish_resp_msg_v01 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_lte_disc_cancel_publish_resp_os_app_id(char * os_app_id)
{
  qmi_lte_disc_cancel_publish_resp_msg_v01 *ptr = (qmi_lte_disc_cancel_publish_resp_msg_v01 *)payload;
  ptr->OsAppId_valid = TRUE;
  memcpy(ptr->OsAppId, os_app_id, strlen(os_app_id));
}

/*========================================================
  QMI_LTE_DISC_CANCEL_SUBSCRIBE_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_lte_disc_cancel_subscribe_req_msg(void)
{
  payload = (void *) malloc(sizeof(qmi_lte_disc_cancel_subscribe_req_msg_v01));
  payload_len = sizeof(qmi_lte_disc_cancel_subscribe_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_LTE;
  msg_id = QMI_LTE_DISC_CANCEL_SUBSCRIBE_REQ_V01;
}
void ril_utf_qmiril_if::set_qmi_lte_disc_cancel_subscribe_req_os_app_id(char * os_app_id)
{
  qmi_lte_disc_cancel_subscribe_req_msg_v01 *ptr = (qmi_lte_disc_cancel_subscribe_req_msg_v01 *)payload;
  memcpy(ptr->OsAppId, os_app_id, strlen(os_app_id));
}
void ril_utf_qmiril_if::set_qmi_lte_disc_cancel_subscribe_req_expression(char * expression)
{
  qmi_lte_disc_cancel_subscribe_req_msg_v01 *ptr = (qmi_lte_disc_cancel_subscribe_req_msg_v01 *)payload;
  ptr->Expression_valid = TRUE;
  memcpy(ptr->Expression.prose_app_id_name, expression, strlen(expression));
}

/*========================================================
  QMI_LTE_DISC_CANCEL_SUBSCRIBE_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_lte_disc_cancel_subscribe_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(qmi_lte_disc_cancel_subscribe_resp_msg_v01));
  payload_len = sizeof(qmi_lte_disc_cancel_subscribe_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_LTE;
  msg_id = QMI_LTE_DISC_CANCEL_SUBSCRIBE_RESP_V01;

  qmi_lte_disc_cancel_subscribe_resp_msg_v01 *ptr = (qmi_lte_disc_cancel_subscribe_resp_msg_v01 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_lte_disc_cancel_subscribe_resp_os_app_id(char * os_app_id)
{
  qmi_lte_disc_cancel_subscribe_resp_msg_v01 *ptr = (qmi_lte_disc_cancel_subscribe_resp_msg_v01 *)payload;
  ptr->OsAppId_valid = TRUE;
  memcpy(ptr->OsAppId, os_app_id, strlen(os_app_id));
}

/*========================================================
  QMI_LTE_BIND_SUBSCRIPTION_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_lte_bind_subscription_req_msg(qmi_lte_bind_subscription_type_enum_v01 subscription)
{
  payload = (void *) malloc(sizeof(qmi_lte_bind_subscription_req_msg_v01));
  payload_len = sizeof(qmi_lte_bind_subscription_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_LTE;
  msg_id = QMI_LTE_BIND_SUBSCRIPTION_REQ_V01;

  qmi_lte_bind_subscription_req_msg_v01 *ptr = (qmi_lte_bind_subscription_req_msg_v01 *)payload;
  ptr->subscription = subscription;
}

/*========================================================
  QMI_LTE_BIND_SUBSCRIPTION_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_lte_bind_subscription_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(qmi_lte_bind_subscription_resp_msg_v01));
  payload_len = sizeof(qmi_lte_bind_subscription_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_LTE;
  msg_id = QMI_LTE_BIND_SUBSCRIPTION_RESP_V01;

  qmi_lte_bind_subscription_resp_msg_v01 *ptr = (qmi_lte_bind_subscription_resp_msg_v01 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

/*========================================================
  QMI_LTE_GET_SUBSCRIPTION_INFO_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_lte_get_subscription_info_req_msg(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_qmi_request;
  service_id = QMI_LTE;
  msg_id = QMI_LTE_GET_SUBSCRIPTION_INFO_REQ_V01;
}

/*========================================================
  QMI_LTE_GET_SUBSCRIPTION_INFO_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_lte_get_subscription_info_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(qmi_lte_get_subscription_info_resp_msg_v01));
  payload_len = sizeof(qmi_lte_get_subscription_info_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_LTE;
  msg_id = QMI_LTE_GET_SUBSCRIPTION_INFO_RESP_V01;

  qmi_lte_get_subscription_info_resp_msg_v01 *ptr = (qmi_lte_get_subscription_info_resp_msg_v01 *)payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}
void ril_utf_qmiril_if::set_qmi_lte_get_subscription_info_resp_subscription(qmi_lte_bind_subscription_type_enum_v01 subscription)
{
  qmi_lte_get_subscription_info_resp_msg_v01 *ptr = (qmi_lte_get_subscription_info_resp_msg_v01 *)payload;
  ptr->subscription_valid = TRUE;
  ptr->subscription = subscription;
}
void ril_utf_qmiril_if::set_qmi_lte_get_subscription_info_resp_lte_disc_capability(boolean lte_disc_capability)
{
  qmi_lte_get_subscription_info_resp_msg_v01 *ptr = (qmi_lte_get_subscription_info_resp_msg_v01 *)payload;
  ptr->LteDiscCapability_valid = TRUE;
  ptr->LteDiscCapability = lte_disc_capability;
}

/*========================================================
  QMI_LTE_DISC_NOTIFICATION_IND_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_lte_disc_notification_ind_msg(void)
{
  payload = (void *) malloc(sizeof(qmi_lte_disc_notification_ind_msg_v01));
  payload_len = sizeof(qmi_lte_disc_notification_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_LTE;
  msg_id = QMI_LTE_DISC_NOTIFICATION_IND_V01;
}
void ril_utf_qmiril_if::set_qmi_lte_disc_notification_ind_os_app_id(char * os_app_id)
{
  qmi_lte_disc_notification_ind_msg_v01 *ptr = (qmi_lte_disc_notification_ind_msg_v01 *)payload;
  ptr->OsAppId_valid = TRUE;
  memcpy(ptr->OsAppId, os_app_id, strlen(os_app_id));
}
void ril_utf_qmiril_if::set_qmi_lte_disc_notification_ind_authorization_result(qmi_lte_disc_result_type_enum_v01 authorization_result)
{
  qmi_lte_disc_notification_ind_msg_v01 *ptr = (qmi_lte_disc_notification_ind_msg_v01 *)payload;
  ptr->AuthorizationResult_valid = TRUE;
  ptr->AuthorizationResult = authorization_result;
}
void ril_utf_qmiril_if::set_qmi_lte_disc_notification_ind_expression(char * expression)
{
  qmi_lte_disc_notification_ind_msg_v01 *ptr = (qmi_lte_disc_notification_ind_msg_v01 *)payload;
  ptr->Expression_valid = TRUE;
  memcpy(ptr->Expression.prose_app_id_name, expression, strlen(expression));
}
void ril_utf_qmiril_if::set_qmi_lte_disc_notification_ind_expression_result(qmi_lte_disc_result_type_enum_v01 expression_result)
{
  qmi_lte_disc_notification_ind_msg_v01 *ptr = (qmi_lte_disc_notification_ind_msg_v01 *)payload;
  ptr->ExpressionResult_valid = TRUE;
  ptr->ExpressionResult = expression_result;
}
void ril_utf_qmiril_if::set_qmi_lte_disc_notification_ind_publish_status_per_expr(uint8_t publish_status_per_expr)
{
  qmi_lte_disc_notification_ind_msg_v01 *ptr = (qmi_lte_disc_notification_ind_msg_v01 *)payload;
  ptr->PublishStatusPerExpr_valid = TRUE;
  ptr->PublishStatusPerExpr = publish_status_per_expr;
}
void ril_utf_qmiril_if::set_qmi_lte_disc_notification_ind_subscribe_status_per_expr(uint8_t subscribe_status_per_expr)
{
  qmi_lte_disc_notification_ind_msg_v01 *ptr = (qmi_lte_disc_notification_ind_msg_v01 *)payload;
  ptr->SubscribeStatusPerExpr_valid = TRUE;
  ptr->SubscribeStatusPerExpr = subscribe_status_per_expr;
}

/*========================================================
  QMI_LTE_DISC_BROADCAST_NOTIFICATION_IND_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_lte_disc_broadcast_notification_ind_msg(void)
{
  payload = (void *) malloc(sizeof(qmi_lte_disc_broadcast_notification_ind_msg_v01));
  payload_len = sizeof(qmi_lte_disc_broadcast_notification_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_LTE;
  msg_id = QMI_LTE_DISC_BROADCAST_NOTIFICATION_IND_V01;
}
void ril_utf_qmiril_if::set_qmi_lte_disc_broadcast_notification_ind_publish_allowed(boolean publish_allowed)
{
  qmi_lte_disc_broadcast_notification_ind_msg_v01 *ptr = (qmi_lte_disc_broadcast_notification_ind_msg_v01 *)payload;
  ptr->PublishAllowed_valid = TRUE;
  ptr->PublishAllowed = publish_allowed;
}
void ril_utf_qmiril_if::set_qmi_lte_disc_broadcast_notification_ind_subscribe_allowed(boolean subscribe_allowed)
{
  qmi_lte_disc_broadcast_notification_ind_msg_v01 *ptr = (qmi_lte_disc_broadcast_notification_ind_msg_v01 *)payload;
  ptr->SubscribeAllowed_valid = TRUE;
  ptr->SubscribeAllowed = subscribe_allowed;
}

/*========================================================
  QMI_LTE_DISC_MATCH_NOTIFICATION_IND_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_lte_disc_match_notification_ind_msg(void)
{
  payload = (void *) malloc(sizeof(qmi_lte_disc_match_notification_ind_msg_v01));
  payload_len = sizeof(qmi_lte_disc_match_notification_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_LTE;
  msg_id = QMI_LTE_DISC_MATCH_NOTIFICATION_IND_V01;
}
void ril_utf_qmiril_if::set_qmi_lte_disc_match_notification_ind_os_app_id(char * os_app_id)
{
  qmi_lte_disc_match_notification_ind_msg_v01 *ptr = (qmi_lte_disc_match_notification_ind_msg_v01 *)payload;
  ptr->OsAppId_valid = TRUE;
  memcpy(ptr->OsAppId, os_app_id, strlen(os_app_id));
}
void ril_utf_qmiril_if::set_qmi_lte_disc_match_notification_ind_expression(char * expression)
{
  qmi_lte_disc_match_notification_ind_msg_v01 *ptr = (qmi_lte_disc_match_notification_ind_msg_v01 *)payload;
  ptr->Expression_valid = TRUE;
  memcpy(ptr->Expression.prose_app_id_name, expression, strlen(expression));
}
void ril_utf_qmiril_if::set_qmi_lte_disc_match_notification_ind_matched_expression(char * expression)
{
  qmi_lte_disc_match_notification_ind_msg_v01 *ptr = (qmi_lte_disc_match_notification_ind_msg_v01 *)payload;
  ptr->MatchedExpression_valid = TRUE;
  memcpy(ptr->MatchedExpression.prose_app_id_name, expression, strlen(expression));
}
void ril_utf_qmiril_if::set_qmi_lte_disc_match_notification_ind_match_event_state(uint8_t match_event_state)
{
  qmi_lte_disc_match_notification_ind_msg_v01 *ptr = (qmi_lte_disc_match_notification_ind_msg_v01 *)payload;
  ptr->MatchEventState_valid = TRUE;
  ptr->MatchEventState = match_event_state;
}
void ril_utf_qmiril_if::set_qmi_lte_disc_match_notification_ind_meta_data_index(uint16_t meta_data_index)
{
  qmi_lte_disc_match_notification_ind_msg_v01 *ptr = (qmi_lte_disc_match_notification_ind_msg_v01 *)payload;
  ptr->MetaDataIndex_valid = TRUE;
  ptr->MetaDataIndex = meta_data_index;
}
void ril_utf_qmiril_if::set_qmi_lte_disc_match_notification_ind_meta_data(char * meta_data)
{
  qmi_lte_disc_match_notification_ind_msg_v01 *ptr = (qmi_lte_disc_match_notification_ind_msg_v01 *)payload;
  // TODO copy correctly
  ptr->MetaData_valid = TRUE;
  memcpy(ptr->MetaData, meta_data, strlen(meta_data));
}

/*========================================================
  QMI_LTE_SUBSCRIPTION_INFO_IND_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_lte_subscription_info_ind_msg(void)
{
  payload = (void *) malloc(sizeof(qmi_lte_subscription_info_ind_msg_v01));
  payload_len = sizeof(qmi_lte_subscription_info_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  service_id = QMI_LTE;
  msg_id = QMI_LTE_SUBSCRIPTION_INFO_IND_V01;
}
void ril_utf_qmiril_if::set_qmi_lte_subscription_info_ind_subscription(qmi_lte_bind_subscription_type_enum_v01 subscription)
{
  qmi_lte_subscription_info_ind_msg_v01 *ptr = (qmi_lte_subscription_info_ind_msg_v01 *)payload;
  ptr->subscription_valid = TRUE;
  ptr->subscription = subscription;
}
void ril_utf_qmiril_if::set_qmi_lte_subscription_info_ind_lte_disc_capability(boolean lte_disc_capability)
{
  qmi_lte_subscription_info_ind_msg_v01 *ptr = (qmi_lte_subscription_info_ind_msg_v01 *)payload;
  ptr->LteDiscCapability_valid = TRUE;
  ptr->LteDiscCapability = lte_disc_capability;
}

/*========================================================
  QMI_IMSA_SET_BIND_SUBSCRIPTION_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_imsa_bind_subscription_req_msg(imsa_bind_subscription_type_enum_v01 subs_type)
{
  payload = (void *) malloc(sizeof(imsa_set_bind_subscription_req_msg_v01));
  payload_len = sizeof(imsa_set_bind_subscription_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSA;
  msg_id = QMI_IMSA_SET_BIND_SUBSCRIPTION_REQ_V01;

  imsa_set_bind_subscription_req_msg_v01 *ptr = (imsa_set_bind_subscription_req_msg_v01 *) payload;
  ptr->subscription_type_valid = TRUE;
  ptr->subscription_type = subs_type;
}

/*========================================================
  QMI_IMSA_SET_BIND_SUBSCRIPTION_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_imsa_bind_subscription_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(imsa_set_bind_subscription_resp_msg_v01));
  payload_len = sizeof(imsa_set_bind_subscription_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSA;
  msg_id = QMI_IMSA_SET_BIND_SUBSCRIPTION_RESP_V01;

  imsa_set_bind_subscription_resp_msg_v01 *ptr = (imsa_set_bind_subscription_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

/*========================================================
  QMI_IMS_SET_BIND_SUBSCRIPTION_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_imss_bind_subscription_req_msg(ims_subscription_type_enum_v01 subs_type)
{
  payload = (void *) malloc(sizeof(ims_set_bind_subscription_req_msg_v01));
  payload_len = sizeof(ims_set_bind_subscription_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SET_BIND_SUBSCRIPTION_REQ_V01;

  ims_set_bind_subscription_req_msg_v01 *ptr = (ims_set_bind_subscription_req_msg_v01 *) payload;
  ptr->subscription_type = subs_type;
}

/*========================================================
  QMI_IMS_SET_BIND_SUBSCRIPTION_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_imss_bind_subscription_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error)
{
  payload = (void *) malloc(sizeof(ims_set_bind_subscription_resp_msg_v01));
  payload_len = sizeof(ims_set_bind_subscription_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_IMSS;
  msg_id = QMI_IMS_SET_BIND_SUBSCRIPTION_RESP_V01;

  ims_set_bind_subscription_resp_msg_v01 *ptr = (ims_set_bind_subscription_resp_msg_v01 *) payload;
  ptr->resp.error = error;
  ptr->resp.result= result;
}

/*========================================================
  QMI_UIM_HTTP_RESET_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_uim_http_reset_req()
{
  payload = (void*) malloc(sizeof(uim_http_reset_req_msg_v01));
  payload_len = sizeof(uim_http_reset_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_UIM_HTTP;
  msg_id = QMI_UIM_HTTP_RESET_REQ_V01;
}

/*========================================================
  QMI_UIM_HTTP_RESET_RESP_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_uim_http_reset_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void*) malloc(sizeof(uim_http_reset_resp_msg_v01));
  payload_len = sizeof(uim_http_reset_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_UIM_HTTP;
  msg_id = QMI_UIM_HTTP_RESET_RESP_V01;

  uim_http_reset_resp_msg_v01 *ptr = (uim_http_reset_resp_msg_v01*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;
}

/*========================================================
  QMI_UIM_GET_SLOTS_STATUS_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_uim_get_slots_status_req()
{
  payload = (void *) malloc (sizeof(uim_get_slots_status_req_msg_v01));
  payload_len = sizeof(uim_get_slots_status_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_GET_SLOTS_STATUS_REQ_V01;
}

/*========================================================
  QMI_UIM_GET_SLOTS_STATUS_RESP_V01
========================================================*/
uim_get_slots_status_resp_msg_v01* ril_utf_qmiril_if::update_default_qmi_uim_get_slots_status_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void*) malloc(sizeof(uim_get_slots_status_resp_msg_v01));
  payload_len = sizeof(uim_get_slots_status_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_GET_SLOTS_STATUS_RESP_V01;

  uim_get_slots_status_resp_msg_v01 *ptr = (uim_get_slots_status_resp_msg_v01*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;

  return ptr;
}

/*========================================================
  QMI_UIM_GET_ATR_REQ_V01
========================================================*/
void ril_utf_qmiril_if::update_default_qmi_uim_get_atr_req()
{
  payload = (void *) malloc (sizeof(uim_get_atr_req_msg_v01));
  payload_len = sizeof(uim_get_atr_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_GET_ATR_REQ_V01;
}

/*========================================================
  QMI_UIM_GET_ATR_RESP_V01
========================================================*/
uim_get_atr_resp_msg_v01* ril_utf_qmiril_if::update_default_qmi_uim_get_atr_resp(qmi_result_type_v01 result, qmi_error_type_v01 error)
{
  payload = (void*) malloc(sizeof(uim_get_atr_resp_msg_v01));
  payload_len = sizeof(uim_get_atr_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  service_id = QMI_UIM;
  msg_id = QMI_UIM_GET_ATR_RESP_V01;

  uim_get_atr_resp_msg_v01 *ptr = (uim_get_atr_resp_msg_v01*) payload;
  ptr->resp.result = result;
  ptr->resp.error = error;

  return ptr;
}

/*=========================================================
   QMI_UIM_REFRESH_IND_V01
=========================================================*/
uim_refresh_ind_msg_v01* ril_utf_qmiril_if::update_default_qmi_uim_refresh_ind_msg_v01(void)
{
  payload = (uim_refresh_ind_msg_v01* )malloc(sizeof(uim_refresh_ind_msg_v01));
  payload_len = sizeof(uim_refresh_ind_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_indication;
  msg_id = QMI_UIM_REFRESH_IND_V01;
  service_id = QMI_UIM;

  return (uim_refresh_ind_msg_v01*) payload;
}

/*=========================================================
   QMI_UIM_REFRESH_COMPLETE_REQ_V01
=========================================================*/
uim_refresh_complete_req_msg_v01* ril_utf_qmiril_if::update_default_qmi_uim_refresh_complete_req_msg_v01(void)
{
  payload = (uim_refresh_complete_req_msg_v01* )malloc(sizeof(uim_refresh_complete_req_msg_v01));
  payload_len = sizeof(uim_refresh_complete_req_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_request;
  msg_id = QMI_UIM_REFRESH_COMPLETE_REQ_V01;
  service_id = QMI_UIM;

  return (uim_refresh_complete_req_msg_v01*) payload;
}

/*=========================================================
   QMI_UIM_REFRESH_COMPLETE_RESP_V01
=========================================================*/
uim_refresh_complete_resp_msg_v01* ril_utf_qmiril_if::update_default_qmi_uim_refresh_complete_resp_msg_v01(void)
{
  payload = (uim_refresh_complete_resp_msg_v01* )malloc(sizeof(uim_refresh_complete_resp_msg_v01));
  payload_len = sizeof(uim_refresh_complete_resp_msg_v01);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_qmi_response;
  msg_id = QMI_UIM_REFRESH_COMPLETE_RESP_V01;
  service_id = QMI_UIM;

  return (uim_refresh_complete_resp_msg_v01*) payload;
}
