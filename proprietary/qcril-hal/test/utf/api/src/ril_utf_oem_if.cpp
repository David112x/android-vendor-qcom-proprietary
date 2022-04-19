/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
  @file    ril_utf_oem_if.cpp
  @brief   ims API's for test case dev

  DESCRIPTION

  API's for modifying ims messages used in RIL UTF tests.
  ---------------------------------------------------------------------------
******************************************************************************/
#include "ril_utf_oem_if.h"
#ifdef QMI_RIL_HAL_UTF
#include "qcril_qmi_oem_reqlist.h"
#endif
//---------------------------------------------------------------------------------------
//==================================================
// OEM Interfaces
//==================================================

/*========================================================
  QCRIL_EVT_HOOK_UNSOL_WMS_READY
========================================================*/
void ril_utf_oem_if::update_default_qcril_evt_hook_unsol_wms_ready(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_oem_indication;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_UNSOL_WMS_READY;
#else
  oem_command_id = QCRIL_EVT_HOOK_UNSOL_WMS_READY;
#endif
}

/*========================================================
  QCRIL_EVT_HOOK_UNSOL_CSG_ID_CHANGE_IND
========================================================*/
void ril_utf_oem_if::update_default_qcril_evt_hook_unsol_csg_id()
{
  payload = malloc(sizeof(uint32_t));
  payload_len = sizeof(uint32_t);
  memset(payload,0,payload_len);
  payload_type = ril_utf_oem_indication;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_UNSOL_CSG_ID_CHANGE_IND;
#else
  oem_command_id = QCRIL_EVT_HOOK_UNSOL_CSG_ID_CHANGE_IND;
#endif
}

void ril_utf_oem_if::update_qcril_evt_hook_unsol_with_csg_id(uint32_t csg_id)
{
	uint32_t *ptr = (uint32_t*) payload;
	*ptr = csg_id;
}

/*========================================================
  QCRIL_EVT_HOOK_UNSOL_MAX_DATA_CHANGE_IND
========================================================*/
void ril_utf_oem_if::update_default_qcril_evt_hook_unsol_max_data_sub_change()
{
  payload = malloc(sizeof(uint8_t));
  payload_len = sizeof(uint8_t);
  memset(payload,0,payload_len);
  payload_type = ril_utf_oem_indication;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_UNSOL_MAX_DATA_CHANGE_IND;
#else
  oem_command_id = QCRIL_EVT_HOOK_UNSOL_MAX_DATA_CHANGE_IND;
#endif
}

void ril_utf_oem_if::update_qcril_evt_hook_unsol_with_max_data_sub(uint8_t max_data_sub)
{
	uint8_t *ptr = (uint8_t*) payload;
	*ptr = max_data_sub;
}

/*========================================================
  QCRIL_EVT_HOOK_UNSOL_SS_ERROR_CODE
========================================================*/
void ril_utf_oem_if::update_default_qcril_evt_hook_unsol_ss_error_code(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_oem_indication;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_UNSOL_SS_ERROR_CODE;
#else
  oem_command_id = QCRIL_EVT_HOOK_UNSOL_SS_ERROR_CODE;
#endif
}

/*========================================================
  QMI_EMBMS_ENABLE_EMBMS_REQ_V01
========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_enable_embms_req(void)
{
  payload = malloc(sizeof(embms_enable_embms_req_msg_v01));
  payload_len = sizeof(embms_enable_embms_req_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_request;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_ENABLE_EMBMS_REQ_V01;
  token_id = 2;
}

/*========================================================
  QMI_EMBMS_GET_SIB16_COVERAGE_REQ_V01
========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_get_sib16_coverage_req_v01(void)
{
  payload = malloc(sizeof(embms_get_sib16_coverage_req_msg_v01));
  payload_len = sizeof(embms_get_sib16_coverage_req_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_request;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_GET_SIB16_COVERAGE_REQ_V01;
  token_id = 2;
}

/*========================================================
  QMI_EMBMS_GET_SIB16_COVERAGE_RESP_V01
========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_get_sib16_coverage_resp_v01(void)
{
  payload = malloc(sizeof(embms_get_sib16_coverage_resp_msg_v01));
  payload_len = sizeof(embms_get_sib16_coverage_resp_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_response;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_GET_SIB16_COVERAGE_RESP_V01;
  token_id = 2;
}
void ril_utf_oem_if::set_qmi_embms_get_sib16_coverage_resp_v01_in_converage(uint8_t in_coverage)
{
  embms_get_sib16_coverage_resp_msg_v01 *ptr = (embms_get_sib16_coverage_resp_msg_v01*) payload;
  ptr->in_coverage = in_coverage;
}

/*========================================================
  QMI_EMBMS_UNSOL_SIB16_COVERAGE_IND_V01
========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_unsol_sib16_coverage_ind_v01(void)
{
  payload = malloc(sizeof(embms_unsol_sib16_coverage_ind_msg_v01));
  payload_len = sizeof(embms_unsol_sib16_coverage_ind_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_indication;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_UNSOL_SIB16_COVERAGE_IND_V01;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_UNSOL_GENERIC;
#else
  oem_command_id = QCRIL_EVT_HOOK_UNSOL_GENERIC;
#endif
}
void ril_utf_oem_if::set_qmi_embms_unsol_sib16_coverage_ind_v01_in_converage(uint8_t in_coverage)
{
  embms_unsol_sib16_coverage_ind_msg_v01 *ptr = (embms_unsol_sib16_coverage_ind_msg_v01 *) payload;
  ptr->in_coverage = in_coverage;
}


/*========================================================
  QMI_EMBMS_STATUS_E911_STATE_IND_V01
========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_unsol_e911_state_ind_v01(void)
{
  payload = malloc(sizeof(embms_unsol_e911_state_ind_msg_v01));
  payload_len = sizeof(embms_unsol_e911_state_ind_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_indication;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_E911_STATE_IND_V01;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_UNSOL_GENERIC;
#else
  oem_command_id = QCRIL_EVT_HOOK_UNSOL_GENERIC;
#endif
}

void ril_utf_oem_if::set_qmi_embms_unsol_e911_state_ind_v01_e911_state(embms_e911_mode_v01 e911_state)
{
  embms_unsol_e911_state_ind_msg_v01 *ptr = (embms_unsol_e911_state_ind_msg_v01 *) payload;
  ptr->dbg_trace_id = 0xffffffff;
  ptr->e911_state = e911_state;
}


/*=======================================================
  QMI_EMBMS_ENABLE_EMBMS_RESP_V01
========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_enable_embms_resp_v01(void)
{
  payload = malloc(sizeof(embms_enable_embms_resp_msg_v01));
  payload_len = sizeof(embms_enable_embms_resp_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_response;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_ENABLE_EMBMS_RESP_V01;
}
void ril_utf_oem_if::set_qmi_embms_enable_embms_resp_resp_code(int32_t resp_code)
{
  embms_enable_embms_resp_msg_v01 *ptr = (embms_enable_embms_resp_msg_v01*) payload;
  ptr->resp_code = resp_code;
}
void ril_utf_oem_if::set_qmi_embms_enable_embms_resp_call_id(uint8_t call_id)
{
  embms_enable_embms_resp_msg_v01 *ptr = (embms_enable_embms_resp_msg_v01*) payload;
  ptr->call_id_valid = TRUE;
  ptr->call_id = call_id;
}
void ril_utf_oem_if::set_qmi_embms_enable_embms_resp_interface_name(char *name)
{
  embms_enable_embms_resp_msg_v01 *ptr = (embms_enable_embms_resp_msg_v01*) payload;
  ptr->interface_name_valid = TRUE;
  strlcpy(ptr->interface_name, name, strlen(name));
}
void ril_utf_oem_if::set_qmi_embms_enable_embms_resp_if_index(uint32_t if_index)
{
  embms_enable_embms_resp_msg_v01 *ptr = (embms_enable_embms_resp_msg_v01*) payload;
  ptr->if_index_valid = TRUE;
  ptr->if_index = if_index;
}

/*========================================================
  QMI_EMBMS_DISABLE_EMBMS_REQ_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_disable_embms_req_v01(uint8_t call_id)
{
  payload = malloc(sizeof(embms_disable_embms_req_msg_v01));
  payload_len = sizeof(embms_disable_embms_req_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_request;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_DISABLE_EMBMS_REQ_V01;

  embms_disable_embms_req_msg_v01 *ptr = (embms_disable_embms_req_msg_v01*) payload;
  ptr->call_id = call_id;
}

/*========================================================
  QMI_EMBMS_DISABLE_EMBMS_RESP_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_disable_embms_resp_v01(void)
{
  payload = malloc(sizeof(embms_disable_embms_resp_msg_v01));
  payload_len = sizeof(embms_disable_embms_resp_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_response;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_DISABLE_EMBMS_RESP_V01;
}

void ril_utf_oem_if::set_qmi_embms_disable_embms_resp_resp_code(int32_t resp_code)
{
  embms_disable_embms_resp_msg_v01 *ptr = (embms_disable_embms_resp_msg_v01*) payload;
  ptr->resp_code = resp_code;
}
void ril_utf_oem_if::set_qmi_embms_disable_embms_resp_call_id(uint8_t call_id)
{
  embms_disable_embms_resp_msg_v01 *ptr = (embms_disable_embms_resp_msg_v01*) payload;
  ptr->call_id_valid = TRUE;
  ptr->call_id = call_id;
}

/*========================================================
  QMI_EMBMS_ACTIVATE_TMGI_REQ_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_activate_tmgi_req_v01(void)
{
  payload = malloc(sizeof(embms_activate_tmgi_req_msg_v01));
  payload_len = sizeof(embms_activate_tmgi_req_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_request;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_ACTIVATE_TMGI_REQ_V01;
}
void ril_utf_oem_if::set_qmi_embms_activate_tmgi_req_call_id(uint8_t call_id)
{
  embms_activate_tmgi_req_msg_v01 *ptr = (embms_activate_tmgi_req_msg_v01*) payload;
  ptr->call_id = call_id;
}
void ril_utf_oem_if::set_qmi_embms_activate_tmgi_req_tmgi_info(embms_tmgi_type_v01 tmgi_info)
{
  embms_activate_tmgi_req_msg_v01 *ptr = (embms_activate_tmgi_req_msg_v01*) payload;
  ptr->tmgi_info = tmgi_info;
}
void ril_utf_oem_if::set_qmi_embms_activate_tmgi_req_preemption_priority(int32_t preemption_priority)
{
  embms_activate_tmgi_req_msg_v01 *ptr = (embms_activate_tmgi_req_msg_v01*) payload;
  ptr->preemption_priority = preemption_priority;
}


/*========================================================
  QMI_EMBMS_ACTIVATE_TMGI_RESP_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_activate_tmgi_resp_v01(void)
{
  payload = malloc(sizeof(embms_activate_tmgi_resp_msg_v01));
  payload_len = sizeof(embms_activate_tmgi_resp_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_response;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_ACTIVATE_TMGI_RESP_V01;
}
void ril_utf_oem_if::set_qmi_embms_activate_tmgi_resp_resp_code(int32_t resp_code)
{
  embms_activate_tmgi_resp_msg_v01 *ptr = (embms_activate_tmgi_resp_msg_v01*) payload;
  ptr->resp_code = resp_code;
}
void ril_utf_oem_if::set_qmi_embms_activate_tmgi_resp_call_id(uint8_t call_id)
{
  embms_activate_tmgi_resp_msg_v01 *ptr = (embms_activate_tmgi_resp_msg_v01*) payload;
  ptr->call_id_valid = TRUE;
  ptr->call_id = call_id;
}
void ril_utf_oem_if::set_qmi_embms_activate_tmgi_resp_tmgi_info(embms_tmgi_type_v01 tmgi_info)
{
  embms_activate_tmgi_resp_msg_v01 *ptr = (embms_activate_tmgi_resp_msg_v01*) payload;
  ptr->tmgi_info_valid = TRUE;
  ptr->tmgi_info = tmgi_info;
}

/*========================================================
  QMI_EMBMS_DEACTIVATE_TMGI_REQ_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_deactivate_tmgi_req_v01(void)
{
  payload = malloc(sizeof(embms_deactivate_tmgi_req_msg_v01));
  payload_len = sizeof(embms_deactivate_tmgi_req_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_request;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_DEACTIVATE_TMGI_REQ_V01;
}
void ril_utf_oem_if::set_qmi_embms_deactivate_tmgi_req_call_id(uint8_t call_id)
{
  embms_deactivate_tmgi_req_msg_v01 *ptr = (embms_deactivate_tmgi_req_msg_v01*) payload;
  ptr->call_id = call_id;
}

/*========================================================
  QMI_EMBMS_DEACTIVATE_TMGI_RESP_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_deactivate_tmgi_resp_v01(void)
{
  payload = malloc(sizeof(embms_deactivate_tmgi_resp_msg_v01));
  payload_len = sizeof(embms_deactivate_tmgi_resp_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_response;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_DEACTIVATE_TMGI_RESP_V01;
}
void ril_utf_oem_if::set_qmi_embms_deactivate_tmgi_resp_resp_code(int32_t resp_code)
{
  embms_deactivate_tmgi_resp_msg_v01 *ptr = (embms_deactivate_tmgi_resp_msg_v01*) payload;
  ptr->resp_code = resp_code;
}
void ril_utf_oem_if::set_qmi_embms_deactivate_tmgi_resp_call_id(uint8_t call_id)
{
  embms_deactivate_tmgi_resp_msg_v01 *ptr = (embms_deactivate_tmgi_resp_msg_v01*) payload;
  ptr->call_id_valid = TRUE;
  ptr->call_id = call_id;
}
void ril_utf_oem_if::set_qmi_embms_deactivate_tmgi_resp_tmgi_info(embms_tmgi_type_v01 tmgi_info)
{
  embms_deactivate_tmgi_resp_msg_v01 *ptr = (embms_deactivate_tmgi_resp_msg_v01*) payload;
  ptr->tmgi_info_valid = TRUE;
  ptr->tmgi_info = tmgi_info;
}

/*========================================================
  QMI_EMBMS_GET_AVAILABLE_TMGI_REQ_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_get_available_tmgi_req_v01(void)
{
  payload = malloc(sizeof(embms_get_available_tmgi_req_msg_v01));
  payload_len = sizeof(embms_get_available_tmgi_req_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_request;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_GET_AVAILABLE_TMGI_REQ_V01;
}
void ril_utf_oem_if::set_qmi_embms_get_available_tmgi_req_call_id(int8_t call_id)
{
  embms_get_available_tmgi_req_msg_v01 *ptr = (embms_get_available_tmgi_req_msg_v01*) payload;
  ptr->call_id = call_id;
}

/*========================================================
  QMI_EMBMS_GET_AVAILABLE_TMGI_RESP_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_get_available_tmgi_resp_v01(void)
{
  payload = malloc(sizeof(embms_get_available_tmgi_resp_msg_v01));
  payload_len = sizeof(embms_get_available_tmgi_resp_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_response;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_GET_AVAILABLE_TMGI_RESP_V01;
}
void ril_utf_oem_if::set_qmi_embms_get_available_tmgi_resp(int32_t resp_code)
{
  embms_get_available_tmgi_resp_msg_v01 *ptr = (embms_get_available_tmgi_resp_msg_v01*) payload;
  ptr->resp_code = resp_code;
}

/*========================================================
  QMI_EMBMS_GET_ACTIVE_TMGI_REQ_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_get_active_tmgi_req_v01(void)
{
  payload = malloc(sizeof(embms_get_active_tmgi_req_msg_v01));
  payload_len = sizeof(embms_get_active_tmgi_req_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_request;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_GET_ACTIVE_TMGI_REQ_V01;
}
void ril_utf_oem_if::set_default_qmi_embms_get_active_tmgi_req_call_id(uint8_t call_id)
{
  embms_get_active_tmgi_req_msg_v01 *ptr = (embms_get_active_tmgi_req_msg_v01*)payload;
  ptr->call_id = call_id;
}

/*========================================================
  QMI_EMBMS_GET_ACTIVE_TMGI_RESP_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_get_active_tmgi_resp_v01(void)
{
  payload = malloc(sizeof(embms_get_active_tmgi_resp_msg_v01));
  payload_len = sizeof(embms_get_active_tmgi_resp_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_response;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_GET_ACTIVE_TMGI_RESP_V01;
}
void ril_utf_oem_if::set_qmi_embms_get_active_tmgi_resp(int32_t resp_code)
{
  embms_get_active_tmgi_resp_msg_v01 *ptr = (embms_get_active_tmgi_resp_msg_v01*) payload;
  ptr->resp_code = resp_code;
}

/*========================================================
  QMI_EMBMS_ENABLE_RSSI_REQ_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_enable_rssi_req_v01(void)
{
  payload = malloc(sizeof(embms_enable_rssi_req_msg_v01));
  payload_len = sizeof(embms_enable_rssi_req_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_request;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_ENABLE_RSSI_REQ_V01;
}

/*========================================================
  QMI_EMBMS_ENABLE_RSSI_RESP_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_enable_rssi_resp_v01(void)
{
  payload = malloc(sizeof(embms_enable_rssi_resp_msg_v01));
  payload_len = sizeof(embms_enable_rssi_resp_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_response;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_ENABLE_RSSI_RESP_V01;
}

/*========================================================
  QMI_EMBMS_DISABLE_RSSI_REQ_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_disable_rssi_req_v01(void)
{
  payload = malloc(sizeof(embms_disable_rssi_req_msg_v01));
  payload_len = sizeof(embms_disable_rssi_req_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_request;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_DISABLE_RSSI_REQ_V01;
}

/*========================================================
  QMI_EMBMS_DISABLE_RSSI_RESP_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_disable_rssi_resp_v01(void)
{
  payload = malloc(sizeof(embms_disable_rssi_resp_msg_v01));
  payload_len = sizeof(embms_disable_rssi_resp_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_response;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_DISABLE_RSSI_RESP_V01;
}

/*========================================================
  QMI_EMBMS_GET_COVERAGE_STATE_REQ_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_get_coverage_state_req_v01(void)
{
  payload = malloc(sizeof(embms_get_coverage_state_req_msg_v01));
  payload_len = sizeof(embms_get_coverage_state_req_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_request;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_GET_COVERAGE_STATE_REQ_V01;
}

/*========================================================
  QMI_EMBMS_GET_COVERAGE_STATE_RESP_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_get_coverage_state_resp_v01(void)
{
  payload = malloc(sizeof(embms_get_coverage_state_resp_msg_v01));
  payload_len = sizeof(embms_get_coverage_state_resp_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_response;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_GET_COVERAGE_STATE_RESP_V01;
}
void ril_utf_oem_if::set_qmi_embms_get_coverage_state_resp_resp_code(int32_t resp_code)
{
  embms_get_coverage_state_resp_msg_v01* ptr = (embms_get_coverage_state_resp_msg_v01*) payload;
  ptr->resp_code = resp_code;
}
void ril_utf_oem_if::set_qmi_embms_get_coverage_state_resp_coverage_state(uint32_t coverage_state)
{
  embms_get_coverage_state_resp_msg_v01* ptr = (embms_get_coverage_state_resp_msg_v01*) payload;
  ptr->coverage_state_valid = TRUE;
  ptr->coverage_state = coverage_state;
}

/*========================================================
  QMI_EMBMS_GET_RSSI_REQ_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_get_rssi_req_v01(int32_t dbg_trace_id)
{
  payload = malloc(sizeof(embms_get_rssi_req_msg_v01));
  payload_len = sizeof(embms_get_rssi_req_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_request;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_GET_RSSI_REQ_V01;

  embms_get_rssi_req_msg_v01 *ptr = (embms_get_rssi_req_msg_v01 *) payload;
  ptr->dbg_trace_id = dbg_trace_id;
}

/*========================================================
  QMI_EMBMS_GET_RSSI_RESP_V01
=========================================================*/
embms_get_rssi_resp_msg_v01 *ril_utf_oem_if::update_default_qmi_embms_get_rssi_resp_v01(void)
{
  payload = malloc(sizeof(embms_get_rssi_resp_msg_v01));
  payload_len = sizeof(embms_get_rssi_resp_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_response;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_GET_RSSI_RESP_V01;

  return (embms_get_rssi_resp_msg_v01 *)payload;
}

/*========================================================
  QMI_EMBMS_GET_EMBMS_SERVICE_STATE_REQ_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_get_embms_service_state_req_v01(void)
{
  payload = malloc(sizeof(embms_get_embms_service_state_req_msg_v01));
  payload_len = sizeof(embms_get_embms_service_state_req_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_request;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_GET_EMBMS_SERVICE_STATE_REQ_V01;
}

/*========================================================
  QMI_EMBMS_GET_EMBMS_SERVICE_STATE_RESP_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_get_embms_service_state_resp_v01(void)
{
  payload = malloc(sizeof(embms_get_embms_service_state_resp_msg_v01));
  payload_len = sizeof(embms_get_embms_service_state_resp_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_response;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_GET_EMBMS_SERVICE_STATE_RESP_V01;
}
void ril_utf_oem_if::set_qmi_embms_get_embms_service_state_resp_resp_code(int32_t resp_code)
{
  embms_get_embms_service_state_resp_msg_v01 *ptr = (embms_get_embms_service_state_resp_msg_v01*) payload;
  ptr->resp_code = resp_code;
}
void ril_utf_oem_if::set_qmi_embms_get_embms_service_state_resp_embms_service_state(uint32_t embms_service_state)
{
  embms_get_embms_service_state_resp_msg_v01 *ptr = (embms_get_embms_service_state_resp_msg_v01*) payload;
  ptr->embms_service_state_valid = TRUE;
  ptr->embms_service_state = embms_service_state;
}

/*========================================================
  QMI_EMBMS_UNSOL_EMBMS_SERVICE_STATE_IND_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_unsol_embms_service_state_ind_v01(void)
{
  payload = malloc(sizeof(embms_unsol_embms_service_state_ind_msg_v01));
  payload_len = sizeof(embms_unsol_embms_service_state_ind_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_indication;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_UNSOL_EMBMS_SERVICE_STATE_IND_V01;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_UNSOL_GENERIC;
#else
  oem_command_id = QCRIL_EVT_HOOK_UNSOL_GENERIC;
#endif
}
void ril_utf_oem_if::set_qmi_embms_unsol_embms_service_state_ind_state(uint32_t state)
{
  embms_unsol_embms_service_state_ind_msg_v01 *ptr = (embms_unsol_embms_service_state_ind_msg_v01*) payload;
  ptr->state = state;
}
void ril_utf_oem_if::state_qmi_embms_unsol_embms_service_state_ind_interface_name(char *interface_name)
{
  embms_unsol_embms_service_state_ind_msg_v01 *ptr = (embms_unsol_embms_service_state_ind_msg_v01*) payload;
  strlcpy(ptr->interface_name, interface_name, NUMBER_MAX_V01);
}

/*========================================================
  QMI_EMBMS_ACTIVE_TMGI_IND_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_active_tmgi_ind_v01(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_indication;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_ACTIVE_TMGI_IND_V01;
}

/*========================================================
  QMI_EMBMS_UNSOL_BROADCAST_COVERAGE_IND_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_unsol_broadcast_coverage_ind_v01(void)
{
  payload = malloc(sizeof(embms_unsol_broadcast_coverage_ind_msg_v01));
  payload_len = sizeof(embms_unsol_broadcast_coverage_ind_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_indication;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_UNSOL_BROADCAST_COVERAGE_IND_V01;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_UNSOL_GENERIC;
#else
  oem_command_id = QCRIL_EVT_HOOK_UNSOL_GENERIC;
#endif
}
void ril_utf_oem_if::set_qmi_embms_unsol_broadcast_coverage_ind_broadcast_coverage(uint32_t broadcast_coverage)
{
  embms_unsol_broadcast_coverage_ind_msg_v01 *ptr = (embms_unsol_broadcast_coverage_ind_msg_v01*) payload;
  ptr->broadcast_coverage = broadcast_coverage;
}

/*========================================================
  QMI_EMBMS_UNSOL_RSSI_IND_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_unsol_rssi_ind_v01(void)
{
  payload = malloc(sizeof(embms_unsol_rssi_ind_msg_v01));
  payload_len = sizeof(embms_unsol_rssi_ind_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_indication;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_UNSOL_RSSI_IND_V01;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_UNSOL_GENERIC;
#else
  oem_command_id = QCRIL_EVT_HOOK_UNSOL_GENERIC;
#endif
}
void ril_utf_oem_if::set_qmi_embms_unsol_rssi_ind_rssi(uint32_t rssi)
{
  embms_unsol_rssi_ind_msg_v01 *ptr = (embms_unsol_rssi_ind_msg_v01*) payload;
  ptr->rssi = rssi;
}

/*========================================================
  QMI_EMBMS_AVAILABLE_TMGI_IND_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_available_tmgi_ind_v01(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_indication;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_AVAILABLE_TMGI_IND_V01;
}

/*========================================================
  QMI_EMBMS_OOS_WARNING_IND_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_oos_warning_ind_v01(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_indication;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_OOS_WARNING_IND_V01;
}

/*========================================================
  QMI_EMBMS_ACTIVATE_DEACTIVATE_TMGI_REQ_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_activate_deactivate_tmgi_req_v01(void)
{
  payload = malloc(sizeof(embms_activate_deactivate_tmgi_req_msg_v01));
  payload_len = sizeof(embms_activate_deactivate_tmgi_req_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_request;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_ACTIVATE_DEACTIVATE_TMGI_REQ_V01;
}
void ril_utf_oem_if::set_qmi_embms_activate_deactivate_tmgi_req_call_id(uint8_t call_id)
{
  embms_activate_deactivate_tmgi_req_msg_v01 *ptr = (embms_activate_deactivate_tmgi_req_msg_v01*) payload;
  ptr->call_id = call_id;
}
void ril_utf_oem_if::set_qmi_embms_activate_deactivate_tmgi_req_act_tmgi_info(embms_tmgi_type_v01 act_tmgi_info)
{
  embms_activate_deactivate_tmgi_req_msg_v01 *ptr = (embms_activate_deactivate_tmgi_req_msg_v01*) payload;
  ptr->act_tmgi_info = act_tmgi_info;
}
void ril_utf_oem_if::set_qmi_embms_activate_deactivate_tmgi_req_deact_tmgi_info(embms_tmgi_type_v01 deact_tmgi_info)
{
  embms_activate_deactivate_tmgi_req_msg_v01 *ptr = (embms_activate_deactivate_tmgi_req_msg_v01*) payload;
  ptr->deact_tmgi_info = deact_tmgi_info;
}
void ril_utf_oem_if::set_qmi_embms_activate_deactivate_tmgi_req_preemption_priority(int32_t preemption_priority)
{
  embms_activate_deactivate_tmgi_req_msg_v01 *ptr = (embms_activate_deactivate_tmgi_req_msg_v01*) payload;
  ptr->preemption_priority = preemption_priority;
}
int32_t* ril_utf_oem_if::set_qmi_embms_activate_deactivate_tmgi_req_earfcnlist(uint32_t earfcnlist_len)
{
  embms_activate_deactivate_tmgi_req_msg_v01 *ptr = (embms_activate_deactivate_tmgi_req_msg_v01*) payload;
  ptr->earfcnlist_len = earfcnlist_len;
  return ptr->earfcnlist;
}

/*========================================================
  QMI_EMBMS_ACTIVATE_DEACTIVATE_TMGI_RESP_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_activate_deactivate_tmgi_resp_v01(void)
{
  payload = malloc(sizeof(embms_activate_deactivate_tmgi_resp_msg_v01));
  payload_len = sizeof(embms_activate_deactivate_tmgi_resp_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_response;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_ACTIVATE_DEACTIVATE_TMGI_RESP_V01;
}
void ril_utf_oem_if::set_qmi_embms_activate_deactivate_tmgi_resp_act_resp_code(uint16 act_resp_code)
{
  embms_activate_deactivate_tmgi_resp_msg_v01 *ptr = (embms_activate_deactivate_tmgi_resp_msg_v01*) payload;
  ptr->act_resp_code = act_resp_code;
}
void ril_utf_oem_if::set_qmi_embms_activate_deactivate_tmgi_resp_deact_resp_code(uint16_t deact_resp_code)
{
  embms_activate_deactivate_tmgi_resp_msg_v01 *ptr = (embms_activate_deactivate_tmgi_resp_msg_v01*) payload;
  ptr->deact_resp_code = deact_resp_code;
}
void ril_utf_oem_if::set_qmi_embms_activate_deactivate_tmgi_resp_call_id(uint8_t call_id)
{
  embms_activate_deactivate_tmgi_resp_msg_v01 *ptr = (embms_activate_deactivate_tmgi_resp_msg_v01*) payload;
  ptr->call_id_valid = TRUE;
  ptr->call_id = call_id;
}
void ril_utf_oem_if::set_qmi_embms_activate_deactivate_tmgi_resp_act_tmgi_info(embms_tmgi_type_v01 act_tmgi_info)
{
  embms_activate_deactivate_tmgi_resp_msg_v01 *ptr = (embms_activate_deactivate_tmgi_resp_msg_v01*) payload;
  ptr->act_tmgi_info_valid = TRUE;
  ptr->act_tmgi_info = act_tmgi_info;
}
void ril_utf_oem_if::set_qmi_embms_activate_deactivate_tmgi_resp_deact_tmgi_info(embms_tmgi_type_v01 deact_tmgi_info)
{
  embms_activate_deactivate_tmgi_resp_msg_v01 *ptr = (embms_activate_deactivate_tmgi_resp_msg_v01*) payload;
  ptr->deact_tmgi_info_valid = TRUE;
  ptr->deact_tmgi_info = deact_tmgi_info;
}

/*========================================================
  QMI_EMBMS_CELL_INFO_CHANGED_IND_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_cell_info_changed_ind_v01(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_indication;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_CELL_INFO_CHANGED_IND_V01;
}

/*========================================================
  QMI_EMBMS_RADIO_STATE_IND_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_radio_state_ind_v01(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_indication;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_RADIO_STATE_IND_V01;
}

/*========================================================
  QMI_EMBMS_SAI_IND_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_sai_ind_v01(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_indication;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_SAI_IND_V01;
}

/*========================================================
  QMI_EMBMS_GET_ACTIVE_LOG_PACKET_IDS_REQ_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_get_active_log_packet_ids_req_v01(void)
{
  payload = malloc(sizeof(embms_get_active_log_packet_ids_req_msg_v01));
  payload_len = sizeof(embms_get_active_log_packet_ids_req_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_request;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_GET_ACTIVE_LOG_PACKET_IDS_REQ_V01;
}
int32_t* ril_utf_oem_if::set_qmi_embms_get_active_log_packet_ids_req_supported_log_packet_id_list(uint32_t supported_log_packet_id_list_len)
{
  embms_get_active_log_packet_ids_req_msg_v01 *ptr = (embms_get_active_log_packet_ids_req_msg_v01*) payload;
  ptr->supported_log_packet_id_list_len = supported_log_packet_id_list_len;
  return ptr->supported_log_packet_id_list;
}

/*========================================================
  QMI_EMBMS_GET_ACTIVE_LOG_PACKET_IDS_RESP_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_get_active_log_packet_ids_resp_v01(void)
{
  payload = malloc(sizeof(embms_get_active_log_packet_ids_resp_msg_v01));
  payload_len = sizeof(embms_get_active_log_packet_ids_resp_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_response;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_GET_ACTIVE_LOG_PACKET_IDS_RESP_V01;
}
int32_t* ril_utf_oem_if::set_qmi_embms_get_active_log_packet_ids_resp_active_log_packet_id_list(uint32_t active_log_packet_id_list_len)
{
  embms_get_active_log_packet_ids_resp_msg_v01 *ptr = (embms_get_active_log_packet_ids_resp_msg_v01*) payload;
  ptr->active_log_packet_id_list_len = active_log_packet_id_list_len;
  return ptr->active_log_packet_id_list;
}

/*========================================================
  QMI_EMBMS_DELIVER_LOG_PACKET_REQ_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_deliver_log_packet_req_v01(void)
{
  payload = malloc(sizeof(embms_deliver_log_packet_req_msg_v01));
  payload_len = sizeof(embms_deliver_log_packet_req_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_request;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_DELIVER_LOG_PACKET_REQ_V01;
}
void ril_utf_oem_if::set_qmi_embms_deliver_log_packet_req_packet_id(int32_t packet_id)
{
  embms_deliver_log_packet_req_msg_v01 *ptr = (embms_deliver_log_packet_req_msg_v01*) payload;
  ptr->packet_id = packet_id;
}
int8_t* ril_utf_oem_if::set_qmi_embms_deliver_log_packet_req_log_packet(uint32_t log_packet_len)
{
  embms_deliver_log_packet_req_msg_v01 *ptr = (embms_deliver_log_packet_req_msg_v01*) payload;
  ptr->log_packet_len = log_packet_len;
  return ptr->log_packet;
}

/*========================================================
  QMI_EMBMS_DELIVER_LOG_PACKET_RESP_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_deliver_log_packet_resp_v01(void)
{
  payload = malloc(sizeof(embms_deliver_log_packet_resp_msg_v01));
  payload_len = sizeof(embms_deliver_log_packet_resp_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_response;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_DELIVER_LOG_PACKET_RESP_V01;
}

/*========================================================
  QMI_EMBMS_SET_SNTP_TIME_REQ_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_set_sntp_time_req_v01(void)
{
  payload = malloc(sizeof(embms_set_sntp_time_req_msg_v01));
  payload_len = sizeof(embms_set_sntp_time_req_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_request;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_SET_SNTP_TIME_REQ_V01;
}
void ril_utf_oem_if::set_qmi_embms_set_sntp_time_req_sntp_available(uint8_t sntp_available)
{
  embms_set_sntp_time_req_msg_v01 *ptr = (embms_set_sntp_time_req_msg_v01*) payload;
  ptr->sntp_available = sntp_available;
}
void ril_utf_oem_if::set_qmi_embms_set_sntp_time_req_sntp_time_milli_sec(uint64_t sntp_time_milli_sec)
{
  embms_set_sntp_time_req_msg_v01 *ptr = (embms_set_sntp_time_req_msg_v01*) payload;
  ptr->sntp_time_milli_sec_valid = TRUE;
  ptr->sntp_time_milli_sec = sntp_time_milli_sec;
}
void ril_utf_oem_if::set_qmi_embms_set_sntp_time_req_time_stamp_milli_sec(uint64_t time_stamp_milli_sec)
{
  embms_set_sntp_time_req_msg_v01 *ptr = (embms_set_sntp_time_req_msg_v01*) payload;
  ptr->time_stamp_milli_sec_valid = TRUE;
  ptr->time_stamp_milli_sec = time_stamp_milli_sec;
}

/*========================================================
  QMI_EMBMS_SET_SNTP_TIME_RESP_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_set_sntp_time_resp_v01(void)
{
  payload = malloc(sizeof(embms_set_sntp_time_resp_msg_v01));
  payload_len = sizeof(embms_set_sntp_time_resp_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_response;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_SET_SNTP_TIME_RESP_V01;
}

/*========================================================
  QMI_EMBMS_GET_UTC_TIME_REQ_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_get_utc_time_req_v01(void)
{
  payload = malloc(sizeof(embms_get_utc_time_req_msg_v01));
  payload_len = sizeof(embms_get_utc_time_req_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_request;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_GET_UTC_TIME_REQ_V01;
}

/*========================================================
  QMI_EMBMS_GET_UTC_TIME_RESP_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_get_utc_time_resp_v01(void)
{
  payload = malloc(sizeof(embms_get_utc_time_resp_msg_v01));
  payload_len = sizeof(embms_get_utc_time_resp_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_response;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_GET_UTC_TIME_RESP_V01;
}
void ril_utf_oem_if::set_qmi_embms_get_utc_time_resp_resp_code(int32_t resp_code)
{
  embms_get_utc_time_resp_msg_v01 *ptr = (embms_get_utc_time_resp_msg_v01*) payload;
  ptr->resp_code = resp_code;
}
void ril_utf_oem_if::set_qmi_embms_get_utc_time_resp_milli_sec(uint64_t milli_sec)
{
  embms_get_utc_time_resp_msg_v01 *ptr = (embms_get_utc_time_resp_msg_v01*) payload;
  ptr->milli_sec = milli_sec;
}
void ril_utf_oem_if::set_qmi_embms_get_utc_time_resp_day_light_saving(uint8_t day_light_saving)
{
  embms_get_utc_time_resp_msg_v01 *ptr = (embms_get_utc_time_resp_msg_v01*) payload;
  ptr->day_light_saving_valid = TRUE;
  ptr->day_light_saving = day_light_saving;
}
void ril_utf_oem_if::set_qmi_embms_get_utc_time_resp_leap_seconds(int8_t leap_seconds)
{
  embms_get_utc_time_resp_msg_v01 *ptr = (embms_get_utc_time_resp_msg_v01*) payload;
  ptr->leap_seconds_valid = TRUE;
  ptr->leap_seconds = leap_seconds;
}
void ril_utf_oem_if::set_qmi_embms_get_utc_time_resp_local_time_offset(int8_t local_time_offset)
{
  embms_get_utc_time_resp_msg_v01 *ptr = (embms_get_utc_time_resp_msg_v01*) payload;
  ptr->local_time_offset_valid = TRUE;
  ptr->local_time_offset = local_time_offset;
}

#ifdef RIL_UTF_L_MASTER
/*========================================================
  QMI_EMBMS_GET_SIB_PLMN_REQ_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_get_sib_plmn_req_v01(void)
{
  payload = malloc(sizeof(embms_get_sib_plmn_req_msg_v01 ));
  payload_len = sizeof(embms_get_sib_plmn_req_msg_v01 );
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_request;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_GET_SIB_PLMN_REQ_V01;
}
void ril_utf_oem_if::set_qmi_embms_get_sib_plmn_req_dbg_trace_id(int32_t dbg_trace_id)
{
  embms_get_sib_plmn_req_msg_v01 * ptr = (embms_get_sib_plmn_req_msg_v01 *) payload;
  ptr->dbg_trace_id = dbg_trace_id;
}

/*========================================================
  QMI_EMBMS_GET_SIB_PLMN_RESP_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_get_sib_plmn_resp_v01(void)
{
  update_default_qmi_embms_get_sib_plmn_resp_v01(RIL_E_SUCCESS);
}
void ril_utf_oem_if::update_default_qmi_embms_get_sib_plmn_resp_v01(RIL_Errno err)
{
  payload = malloc(sizeof(embms_get_sib_plmn_resp_msg_v01));
  payload_len = sizeof(embms_get_sib_plmn_resp_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_response;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_GET_SIB_PLMN_RESP_V01;
  error = err;
}
void ril_utf_oem_if::set_qmi_embms_get_sib_plmn_resp_dbg_trace_id(int32_t dbg_trace_id)
{
  embms_get_sib_plmn_resp_msg_v01 * ptr = (embms_get_sib_plmn_resp_msg_v01 *) payload;
  ptr->dbg_trace_id = dbg_trace_id;
}
void ril_utf_oem_if::set_qmi_embms_get_sib_plmn_resp_plmn(embms_plmn_type_v01 plmn[], uint32_t plmn_len)
{
  embms_get_sib_plmn_resp_msg_v01 * ptr = (embms_get_sib_plmn_resp_msg_v01 *) payload;
  ptr->plmn_len = plmn_len;
  memcpy(ptr->plmn, plmn, sizeof(ptr->plmn));
}

/*========================================================
  QMI_EMBMS_UNSOL_EMBMS_STATUS_IND_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_unsol_embms_status_ind(void)
{
  payload = malloc(sizeof(embms_unsol_embms_status_ind_msg_v01));
  payload_len = sizeof(embms_unsol_embms_status_ind_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_indication;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_UNSOL_EMBMS_STATUS_IND_V01;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_UNSOL_GENERIC;
#else
  oem_command_id = QCRIL_EVT_HOOK_UNSOL_GENERIC;
#endif
}
void ril_utf_oem_if::set_qmi_embms_unsol_embms_status(uint8_t is_available)
{
  embms_unsol_embms_status_ind_msg_v01* ptr = (embms_unsol_embms_status_ind_msg_v01*) payload;
  ptr->is_available = is_available;
}

/*========================================================
  QMI_EMBMS_GET_EMBMS_STATUS_REQ_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_get_embms_status_req_v01(void)
{
  payload = malloc(sizeof(embms_get_embms_status_req_msg_v01));
  payload_len = sizeof(embms_get_embms_status_req_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_request;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_GET_EMBMS_STATUS_REQ_V01;
}
void ril_utf_oem_if::set_qmi_embms_get_embms_status_req_dbg_trace_id(int32_t dbg_trace_id)
{
  embms_get_embms_status_req_msg_v01* ptr = (embms_get_embms_status_req_msg_v01*) payload;
  ptr->dbg_trace_id = dbg_trace_id;
}

/*========================================================
  QMI_EMBMS_GET_EMBMS_STATUS_RESP_V01
=========================================================*/
void ril_utf_oem_if::update_default_qmi_embms_get_embms_status_resp_v01(void)
{
  update_default_qmi_embms_get_embms_status_resp_v01(RIL_E_SUCCESS);
}
void ril_utf_oem_if::update_default_qmi_embms_get_embms_status_resp_v01(RIL_Errno err)
{
  payload = malloc(sizeof(embms_get_embms_status_resp_msg_v01));
  payload_len = sizeof(embms_get_embms_status_resp_msg_v01);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_response;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS;
  msg_id = QMI_EMBMS_GET_EMBMS_STATUS_RESP_V01;
  error = err;
}
void ril_utf_oem_if::set_qmi_embms_get_embms_status_resp_dbg_trace_id(int32_t dbg_trace_id)
{
  embms_get_embms_status_resp_msg_v01 * ptr = (embms_get_embms_status_resp_msg_v01 *) payload;
  ptr->dbg_trace_id = dbg_trace_id;
}
void ril_utf_oem_if::set_qmi_embms_get_embms_status_resp_available(uint8_t is_available)
{
  embms_get_embms_status_resp_msg_v01 * ptr = (embms_get_embms_status_resp_msg_v01 *) payload;
  ptr->is_available = is_available;
}
#endif

#ifndef UTF_TARGET_BUILD
/*========================================================
  QCRIL_EVT_HOOK_GET_CURRENT_SETUP_CALLS
=========================================================*/
void ril_utf_oem_if::update_default_qcril_evt_hook_get_current_setup_calls()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_oem_request;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_GET_CURRENT_SETUP_CALLS;
#else
  oem_command_id = QCRIL_EVT_HOOK_GET_CURRENT_SETUP_CALLS;
#endif
}

/*========================================================
  QCRIL_EVT_HOOK_GET_CURRENT_SETUP_CALLS (resp)
=========================================================*/
qcril_qmi_voice_setup_call_info *ril_utf_oem_if::update_default_qcril_evt_hook_get_current_setup_calls_resp(boolean call_present)
{
  if (FALSE == call_present)
  {
    payload = NULL;
    payload_len = 0;
  }
  else
  {
    payload = malloc(sizeof(qcril_qmi_voice_setup_call_info));
    payload_len = sizeof(qcril_qmi_voice_setup_call_info);
    memset(payload, 0, payload_len);
  }
  payload_type = ril_utf_oem_response;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_GET_CURRENT_SETUP_CALLS;
#else
  oem_command_id = QCRIL_EVT_HOOK_GET_CURRENT_SETUP_CALLS;
#endif
  qcril_qmi_voice_setup_call_info *dptr = (qcril_qmi_voice_setup_call_info *) payload;
  return dptr;
}
/*========================================================
  QCRIL_EVT_HOOK_REQUEST_SETUP_ANSWER
=========================================================*/
void ril_utf_oem_if::update_default_qcril_evt_hook_request_setup_answer(boolean rejection)
{
  payload = malloc(sizeof(qcril_qmi_voice_setup_answer_data_type));
  payload_len = sizeof(qcril_qmi_voice_setup_answer_data_type);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_request;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_REQUEST_SETUP_ANSWER;
#else
  oem_command_id = QCRIL_EVT_HOOK_REQUEST_SETUP_ANSWER;
#endif
  qcril_qmi_voice_setup_answer_data_type *ptr = (qcril_qmi_voice_setup_answer_data_type *) payload;
  ptr->rejection = rejection;
}
/*========================================================
  QCRIL_EVT_HOOK_REQUEST_SETUP_ANSWER (resp)
=========================================================*/
void ril_utf_oem_if::update_default_qcril_evt_hook_request_setup_answer_resp()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_oem_response;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_REQUEST_SETUP_ANSWER;
#else
  oem_command_id = QCRIL_EVT_HOOK_REQUEST_SETUP_ANSWER;
#endif
}

/*========================================================
  QCRIL_REQ_HOOK_REJECT_INCOMING_CALL_CAUSE_21
=========================================================*/
void ril_utf_oem_if::update_default_qcril_evt_hook_reject_incoming_call_cause_21()
{
  payload = malloc(sizeof(qcril_qmi_voice_setup_answer_data_type));
  payload_len = 0;
  payload_type = ril_utf_oem_request;
  oem_command_id = QCRIL_REQ_HOOK_REJECT_INCOMING_CALL_CAUSE_21;
}
/*========================================================
  QCRIL_REQ_HOOK_REJECT_INCOMING_CALL_CAUSE_21 (resp)
=========================================================*/
void ril_utf_oem_if::update_default_qcril_evt_hook_reject_incoming_call_cause_21_resp()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_oem_response;
  oem_command_id = QCRIL_REQ_HOOK_REJECT_INCOMING_CALL_CAUSE_21;
}

/*========================================================
  QCRIL_REQ_HOOK_SET_LOCAL_CALL_HOLD_v01
=========================================================*/
void ril_utf_oem_if::update_default_qcril_evt_hook_request_set_local_call_hold(int lch)
{
  payload = malloc(sizeof(int));
  payload_len = sizeof(int);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_request;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_SET_LOCAL_CALL_HOLD_v01;
#else
  oem_command_id = QCRIL_EVT_HOOK_SET_LOCAL_CALL_HOLD_v01;
#endif
  int *ptr = (int *) payload;
  *ptr = lch;
}
/*========================================================
  QCRIL_REQ_HOOK_SET_LOCAL_CALL_HOLD_v01 (resp)
=========================================================*/
void ril_utf_oem_if::update_default_qcril_evt_hook_request_set_local_call_hold_resp()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_oem_response;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_SET_LOCAL_CALL_HOLD_v01;
#else
  oem_command_id = QCRIL_EVT_HOOK_SET_LOCAL_CALL_HOLD_v01;
#endif
}

/*========================================================
  QCRIL_EVT_HOOK_SET_CDMA_SUB_SRC_WITH_SPC
========================================================*/
void ril_utf_oem_if::update_default_oem_request_cdma_set_subscription_source(oem_hook_cdma_subscription_source_e_type sub_source, char *spc, int spc_len)
{
  payload = (char *)malloc((spc_len+1)*sizeof(char));
  payload_len = (spc_len+1);
  memset(payload, 0, payload_len);
  payload_type = ril_utf_oem_request;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_SET_CDMA_SUB_SRC_WITH_SPC;
#else
  oem_command_id = QCRIL_EVT_HOOK_SET_CDMA_SUB_SRC_WITH_SPC;
#endif
  char *ptr = (char *)payload;
  ptr[0] = (oem_hook_cdma_subscription_source_e_type)sub_source;
  strncpy(&ptr[1],spc, spc_len);
}

/*========================================================
  QCRIL_EVT_HOOK_SET_CDMA_SUB_SRC_WITH_SPC_RESP
========================================================*/
void ril_utf_oem_if::update_default_oem_request_cdma_set_subscription_source_resp(boolean result)
{
  payload = NULL;
  payload_len = 0;

  payload = (boolean *)malloc(1 *sizeof(boolean));
  memset(payload, 0, sizeof(boolean));
  payload_len = sizeof(boolean);
  boolean *ptr = (boolean *)payload;
  *ptr = result;

  payload_type = ril_utf_oem_response;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_SET_CDMA_SUB_SRC_WITH_SPC;
#else
  oem_command_id = QCRIL_EVT_HOOK_SET_CDMA_SUB_SRC_WITH_SPC;
#endif
}

/*========================================================
  QCRIL_EVT_HOOK_SET_LTE_TUNE_AWAY
========================================================*/
void ril_utf_oem_if::update_default_oem_request_set_lte_tune_away_req(boolean lte_tune_away)
{
  payload = NULL;
  payload_len = 0;
  payload = (boolean *)malloc(1 *sizeof(boolean));
  memset(payload, 0, sizeof(boolean));
  payload_len = sizeof(boolean);
  payload_type = ril_utf_oem_request;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_SET_LTE_TUNE_AWAY;
#else
  oem_command_id = QCRIL_EVT_HOOK_SET_LTE_TUNE_AWAY;
#endif
  boolean *ptr = (boolean *)payload;
  *ptr = lte_tune_away;
}

/*========================================================
  QCRIL_EVT_HOOK_SET_LTE_TUNE_AWAY_RESP
========================================================*/
void ril_utf_oem_if::update_default_oem_request_set_lte_tune_away_resp()
{
  payload = NULL;
  payload_len = 0;

  payload_type = ril_utf_oem_response;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_SET_LTE_TUNE_AWAY;
#else
  oem_command_id = QCRIL_EVT_HOOK_SET_LTE_TUNE_AWAY;
#endif
}

/*========================================================
  QCRIL_EVT_HOOK_UNSOL_WWAN_AVAILABLE
========================================================*/
void ril_utf_oem_if::update_default_qcril_evt_hook_unsol_wlan_available(uint8_t is_wlan_status)
{
  payload = NULL;
  payload_len = 0;

  payload = (uint8_t *)malloc(1 *sizeof(uint8_t));
  memset(payload, 0, sizeof(uint8_t));
  payload_len = sizeof(uint8_t);
  uint8_t *ptr = (uint8_t *)payload;
  *ptr = is_wlan_status;

  payload_type = ril_utf_oem_indication;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_UNSOL_WWAN_AVAILABLE;
#else
  oem_command_id = QCRIL_EVT_HOOK_UNSOL_WWAN_AVAILABLE;
#endif
}

#ifdef RIL_UTF_L_MASTER
/*========================================================
  QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_REQ
========================================================*/
void ril_utf_oem_if::update_default_oem_request_set_data_subscription_req(int data_subsciption)
{
  payload = NULL;
  payload_len = 0;
  payload = (boolean *)malloc(1 *sizeof(int));
  memset(payload, 0, sizeof(int));
  payload_len = sizeof(int);
  payload_type = ril_utf_oem_request;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_SET_DATA_SUBSCRIPTION;
#else
  oem_command_id = QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION;
#endif
  int *ptr = (int *)payload;
  *ptr = data_subsciption;
}

/*========================================================
  QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_RESP
========================================================*/
void ril_utf_oem_if::update_default_oem_request_set_data_subscription_resp()
{
  payload = NULL;
  payload_len = 0;

  payload_type = ril_utf_oem_response;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_SET_DATA_SUBSCRIPTION;
#else
  oem_command_id = QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION;
#endif
}
#endif
/*========================================================
  QCRIL_EVT_HOOK_UNSOL_MODEM_CAPABILITY
========================================================*/
void ril_utf_oem_if::update_default_qcril_evt_hook_unsol_modem_capability(uint8_t *data, size_t len)
{
  payload = malloc (len);
  payload_len = len;
  payload_type = ril_utf_oem_indication;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_UNSOL_MODEM_CAPABILITY;
#else
  oem_command_id = QCRIL_EVT_HOOK_UNSOL_MODEM_CAPABILITY;
#endif

  memcpy (payload, data, len);
}

/*========================================================
  LteDirectDiscovery_MsgId_REQUEST_GET_DEVICE_CAPABILITY
========================================================*/
void ril_utf_oem_if::update_default_lte_direct_discovery_get_device_capability_req(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_oem_request;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_LTE_DIRECT_DISC_REQ;
#else
  oem_command_id = QCRIL_EVT_HOOK_LTE_DIRECT_DISC_REQ;
#endif
  msg_id = LteDirectDiscovery_MsgId_REQUEST_GET_DEVICE_CAPABILITY;
}

/*========================================================
  LteDirectDiscovery_MsgId_REQUEST_GET_DEVICE_CAPABILITY(Response)
========================================================*/
void ril_utf_oem_if::update_default_lte_direct_discovery_get_device_capability_resp(void)
{
  payload = (void *) malloc(sizeof(LteDirectDiscovery_DeviceCapability));
  payload_len = sizeof(LteDirectDiscovery_DeviceCapability);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_oem_response;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_LTE_DIRECT_DISC_REQ;
#else
  oem_command_id = QCRIL_EVT_HOOK_LTE_DIRECT_DISC_REQ;
#endif
  msg_id = LteDirectDiscovery_MsgId_REQUEST_GET_DEVICE_CAPABILITY;
}
void ril_utf_oem_if::set_lte_direct_discovery_get_device_capability_resp_capability(uint32_t capability)
{
  LteDirectDiscovery_DeviceCapability *ptr = (LteDirectDiscovery_DeviceCapability *)payload;
  ptr->has_capability = TRUE;
  ptr->capability = capability;
}

/*========================================================
  LteDirectDiscovery_MsgId_REQUEST_TERMINATE
========================================================*/
void ril_utf_oem_if::update_default_lte_direct_discovery_terminate_req(void)
{
  payload = (void *) malloc(sizeof(LteDirectDiscovery_Terminate));
  payload_len = sizeof(LteDirectDiscovery_Terminate);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_oem_request;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_LTE_DIRECT_DISC_REQ;
#else
  oem_command_id = QCRIL_EVT_HOOK_LTE_DIRECT_DISC_REQ;
#endif
  msg_id = LteDirectDiscovery_MsgId_REQUEST_TERMINATE;
}
void ril_utf_oem_if::set_lte_direct_discovery_terminate_req_os_app_id(char * os_app_id)
{
  LteDirectDiscovery_Terminate *ptr = (LteDirectDiscovery_Terminate *)payload;
  int len = strlen(os_app_id);
  ptr->osAppId.arg = malloc(len + 1);
  memset(ptr->osAppId.arg, 0, len + 1);
  memcpy (ptr->osAppId.arg, os_app_id, len);
}

/*========================================================
  LteDirectDiscovery_MsgId_REQUEST_TERMINATE(Response)
========================================================*/
void ril_utf_oem_if::update_default_lte_direct_discovery_terminate_resp(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_oem_response;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_LTE_DIRECT_DISC_REQ;
#else
  oem_command_id = QCRIL_EVT_HOOK_LTE_DIRECT_DISC_REQ;
#endif
  msg_id = LteDirectDiscovery_MsgId_REQUEST_TERMINATE;
}

/*========================================================
  LteDirectDiscovery_MsgId_REQUEST_GET_SERVICE_STATUS
========================================================*/
void ril_utf_oem_if::update_default_lte_direct_discovery_get_service_status_req(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_oem_request;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_LTE_DIRECT_DISC_REQ;
#else
  oem_command_id = QCRIL_EVT_HOOK_LTE_DIRECT_DISC_REQ;
#endif
  msg_id = LteDirectDiscovery_MsgId_REQUEST_GET_SERVICE_STATUS;
}

/*========================================================
  LteDirectDiscovery_MsgId_REQUEST_GET_SERVICE_STATUS(Response)
========================================================*/
void ril_utf_oem_if::update_default_lte_direct_discovery_get_service_status_resp(void)
{
  payload = (void *) malloc(sizeof(LteDirectDiscovery_ServiceStatus));
  payload_len = sizeof(LteDirectDiscovery_ServiceStatus);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_oem_response;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_LTE_DIRECT_DISC_REQ;
#else
  oem_command_id = QCRIL_EVT_HOOK_LTE_DIRECT_DISC_REQ;
#endif
  msg_id = LteDirectDiscovery_MsgId_REQUEST_GET_SERVICE_STATUS;
}
void ril_utf_oem_if::set_lte_direct_discovery_get_service_status_resp_publish_allowed(uint32_t publish_allowed)
{
  LteDirectDiscovery_ServiceStatus *ptr = (LteDirectDiscovery_ServiceStatus *)payload;
  ptr->has_publishAllowed = TRUE;
  ptr->publishAllowed = publish_allowed;
}
void ril_utf_oem_if::set_lte_direct_discovery_get_service_status_resp_subscribe_allowed(uint32_t subscribe_allowed)
{
  LteDirectDiscovery_ServiceStatus *ptr = (LteDirectDiscovery_ServiceStatus *)payload;
  ptr->has_subscribeAllowed = TRUE;
  ptr->subscribeAllowed = subscribe_allowed;
}

/*========================================================
  LteDirectDiscovery_MsgId_REQUEST_PUBLISH
========================================================*/
void ril_utf_oem_if::update_default_lte_direct_discovery_publish_req(void)
{
  payload = (void *) malloc(sizeof(LteDirectDiscovery_Publish));
  payload_len = sizeof(LteDirectDiscovery_Publish);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_oem_request;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_LTE_DIRECT_DISC_REQ;
#else
  oem_command_id = QCRIL_EVT_HOOK_LTE_DIRECT_DISC_REQ;
#endif
  msg_id = LteDirectDiscovery_MsgId_REQUEST_PUBLISH;
}
void ril_utf_oem_if::set_lte_direct_discovery_publish_req_os_app_id(char * os_app_id)
{
  LteDirectDiscovery_Publish *ptr = (LteDirectDiscovery_Publish *)payload;
  int len = strlen(os_app_id);
  ptr->osAppId.arg = malloc(len + 1);
  memset(ptr->osAppId.arg, 0, len + 1);
  memcpy (ptr->osAppId.arg, os_app_id, len);
}
void ril_utf_oem_if::set_lte_direct_discovery_publish_req_expression(char * expression)
{
  LteDirectDiscovery_Publish *ptr = (LteDirectDiscovery_Publish *)payload;
  int len = strlen(expression);
  ptr->expression.arg = malloc(len + 1);
  memset(ptr->expression.arg, 0, len + 1);
  memcpy (ptr->expression.arg, expression, len);
}
void ril_utf_oem_if::set_lte_direct_discovery_publish_req_expression_validity_time(uint32_t expression_validity_time)
{
  LteDirectDiscovery_Publish *ptr = (LteDirectDiscovery_Publish *)payload;
  ptr->has_expressionValidityTime = TRUE;
  ptr->expressionValidityTime = expression_validity_time;
}
void ril_utf_oem_if::set_lte_direct_discovery_publish_req_meta_data(char * meta_data)
{
  LteDirectDiscovery_Publish *ptr = (LteDirectDiscovery_Publish *)payload;
  int len = strlen(meta_data);
  ptr->metaData.arg = malloc(len + 1);
  memset(ptr->metaData.arg, 0, len + 1);
  memcpy (ptr->metaData.arg, meta_data, len);
}
void ril_utf_oem_if::set_lte_direct_discovery_publish_req_discovery_type(uint32_t discovery_type)
{
  LteDirectDiscovery_Publish *ptr = (LteDirectDiscovery_Publish *)payload;
  ptr->has_discoveryType = TRUE;
  ptr->discoveryType = (LteDirectDiscovery_DiscoveryType)discovery_type;
}
void ril_utf_oem_if::set_lte_direct_discovery_publish_req_duration(uint32_t duration)
{
  LteDirectDiscovery_Publish *ptr = (LteDirectDiscovery_Publish *)payload;
  ptr->has_duration = TRUE;
  ptr->duration = duration;
}

/*========================================================
  LteDirectDiscovery_MsgId_REQUEST_PUBLISH(Response)
========================================================*/
void ril_utf_oem_if::update_default_lte_direct_discovery_publish_resp(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_oem_response;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_LTE_DIRECT_DISC_REQ;
#else
  oem_command_id = QCRIL_EVT_HOOK_LTE_DIRECT_DISC_REQ;
#endif
  msg_id = LteDirectDiscovery_MsgId_REQUEST_PUBLISH;
}

/*========================================================
  LteDirectDiscovery_MsgId_REQUEST_CANCEL_PUBLISH
========================================================*/
void ril_utf_oem_if::update_default_lte_direct_discovery_cancel_publish_req(void)
{
  payload = (void *) malloc(sizeof(LteDirectDiscovery_CancelPublish));
  payload_len = sizeof(LteDirectDiscovery_CancelPublish);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_oem_request;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_LTE_DIRECT_DISC_REQ;
#else
  oem_command_id = QCRIL_EVT_HOOK_LTE_DIRECT_DISC_REQ;
#endif
  msg_id = LteDirectDiscovery_MsgId_REQUEST_CANCEL_PUBLISH;
}
void ril_utf_oem_if::set_lte_direct_discovery_cancel_publish_req_os_app_id(char * os_app_id)
{
  LteDirectDiscovery_CancelPublish *ptr = (LteDirectDiscovery_CancelPublish *)payload;
  int len = strlen(os_app_id);
  ptr->osAppId.arg = malloc(len + 1);
  memset(ptr->osAppId.arg, 0, len + 1);
  memcpy (ptr->osAppId.arg, os_app_id, len);
}
void ril_utf_oem_if::set_lte_direct_discovery_cancel_publish_req_expression(char * expression)
{
  LteDirectDiscovery_CancelPublish *ptr = (LteDirectDiscovery_CancelPublish *)payload;
  int len = strlen(expression);
  ptr->expression.arg = malloc(len + 1);
  memset(ptr->expression.arg, 0, len + 1);
  memcpy (ptr->expression.arg, expression, len);
}

/*========================================================
  LteDirectDiscovery_MsgId_REQUEST_CANCEL_PUBLISH(Response)
========================================================*/
void ril_utf_oem_if::update_default_lte_direct_discovery_cancel_publish_resp(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_oem_response;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_LTE_DIRECT_DISC_REQ;
#else
  oem_command_id = QCRIL_EVT_HOOK_LTE_DIRECT_DISC_REQ;
#endif
  msg_id = LteDirectDiscovery_MsgId_REQUEST_CANCEL_PUBLISH;
}

/*========================================================
  LteDirectDiscovery_MsgId_REQUEST_SUBSCRIBE
========================================================*/
void ril_utf_oem_if::update_default_lte_direct_discovery_subscribe_req(void)
{
  payload = (void *) malloc(sizeof(LteDirectDiscovery_Subscribe));
  payload_len = sizeof(LteDirectDiscovery_Subscribe);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_oem_request;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_LTE_DIRECT_DISC_REQ;
#else
  oem_command_id = QCRIL_EVT_HOOK_LTE_DIRECT_DISC_REQ;
#endif
  msg_id = LteDirectDiscovery_MsgId_REQUEST_SUBSCRIBE;
}
void ril_utf_oem_if::set_lte_direct_discovery_subscribe_req_os_app_id(char * os_app_id)
{
  LteDirectDiscovery_Subscribe *ptr = (LteDirectDiscovery_Subscribe *)payload;
  int len = strlen(os_app_id);
  ptr->osAppId.arg = malloc(len + 1);
  memset(ptr->osAppId.arg, 0, len + 1);
  memcpy (ptr->osAppId.arg, os_app_id, len);
}
void ril_utf_oem_if::set_lte_direct_discovery_subscribe_req_expression(char * expression)
{
  LteDirectDiscovery_Subscribe *ptr = (LteDirectDiscovery_Subscribe *)payload;
  int len = strlen(expression);
  ptr->expression.arg = malloc(len + 1);
  memset(ptr->expression.arg, 0, len + 1);
  memcpy (ptr->expression.arg, expression, len);
}
void ril_utf_oem_if::set_lte_direct_discovery_subscribe_req_expression_validity_time(uint32_t expression_validity_time)
{
  LteDirectDiscovery_Subscribe *ptr = (LteDirectDiscovery_Subscribe *)payload;
  ptr->has_expressionValidityTime = TRUE;
  ptr->expressionValidityTime = expression_validity_time;
}
void ril_utf_oem_if::set_lte_direct_discovery_subscribe_req_discovery_type(uint32_t discovery_type)
{
  LteDirectDiscovery_Subscribe *ptr = (LteDirectDiscovery_Subscribe *)payload;
  ptr->has_discoveryType = TRUE;
  ptr->discoveryType = (LteDirectDiscovery_DiscoveryType)discovery_type;
}
void ril_utf_oem_if::set_lte_direct_discovery_subscribe_req_duration(uint32_t duration)
{
  LteDirectDiscovery_Subscribe *ptr = (LteDirectDiscovery_Subscribe *)payload;
  ptr->has_duration = TRUE;
  ptr->duration = duration;
}

/*========================================================
  LteDirectDiscovery_MsgId_REQUEST_SUBSCRIBE(Response)
========================================================*/
void ril_utf_oem_if::update_default_lte_direct_discovery_subscribe_resp(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_oem_response;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_LTE_DIRECT_DISC_REQ;
#else
  oem_command_id = QCRIL_EVT_HOOK_LTE_DIRECT_DISC_REQ;
#endif
  msg_id = LteDirectDiscovery_MsgId_REQUEST_SUBSCRIBE;
}

/*========================================================
  LteDirectDiscovery_MsgId_REQUEST_CANCEL_SUBSCRIBE
========================================================*/
void ril_utf_oem_if::update_default_lte_direct_discovery_cancel_subscribe_req(void)
{
  payload = (void *) malloc(sizeof(LteDirectDiscovery_CancelSubscribe));
  payload_len = sizeof(LteDirectDiscovery_CancelSubscribe);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_oem_request;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_LTE_DIRECT_DISC_REQ;
#else
  oem_command_id = QCRIL_EVT_HOOK_LTE_DIRECT_DISC_REQ;
#endif
  msg_id = LteDirectDiscovery_MsgId_REQUEST_CANCEL_SUBSCRIBE;
}
void ril_utf_oem_if::set_lte_direct_discovery_cancel_subscribe_req_os_app_id(char * os_app_id)
{
  LteDirectDiscovery_CancelSubscribe *ptr = (LteDirectDiscovery_CancelSubscribe *)payload;
  int len = strlen(os_app_id);
  ptr->osAppId.arg = malloc(len + 1);
  memset(ptr->osAppId.arg, 0, len + 1);
  memcpy (ptr->osAppId.arg, os_app_id, len);
}
void ril_utf_oem_if::set_lte_direct_discovery_cancel_subscribe_req_expression(char * expression)
{
  LteDirectDiscovery_CancelSubscribe *ptr = (LteDirectDiscovery_CancelSubscribe *)payload;
  int len = strlen(expression);
  ptr->expression.arg = malloc(len + 1);
  memset(ptr->expression.arg, 0, len + 1);
  memcpy (ptr->expression.arg, expression, len);
}

/*========================================================
  LteDirectDiscovery_MsgId_REQUEST_CANCEL_SUBSCRIBE(Response)
========================================================*/
void ril_utf_oem_if::update_default_lte_direct_discovery_cancel_subscribe_resp(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_oem_response;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_LTE_DIRECT_DISC_REQ;
#else
  oem_command_id = QCRIL_EVT_HOOK_LTE_DIRECT_DISC_REQ;
#endif
  msg_id = LteDirectDiscovery_MsgId_REQUEST_CANCEL_SUBSCRIBE;
}

/*========================================================
  LteDirectDiscovery_MsgId_UNSOL_RESPONSE_DEVICE_CAPABILITY_CHANGED
========================================================*/
void ril_utf_oem_if::update_default_lte_direct_discovery_unsol_response_device_capability_changed(void)
{
  payload = (void *) malloc(sizeof(LteDirectDiscovery_DeviceCapability));
  payload_len = sizeof(LteDirectDiscovery_DeviceCapability);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_oem_indication;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_UNSOL_LTE_DIRECT_DISC;
#else
  oem_command_id = QCRIL_EVT_HOOK_UNSOL_LTE_DIRECT_DISC;
#endif
  msg_id = LteDirectDiscovery_MsgId_UNSOL_RESPONSE_DEVICE_CAPABILITY_CHANGED;
}
void ril_utf_oem_if::set_lte_direct_discovery_unsol_response_device_capability_changed_capability(uint32_t capability)
{
  LteDirectDiscovery_DeviceCapability *ptr = (LteDirectDiscovery_DeviceCapability *)payload;
  ptr->has_capability = TRUE;
  ptr->capability = capability;
}

/*========================================================
  LteDirectDiscovery_MsgId_UNSOL_RESPONSE_SERVICE_STATUS
========================================================*/
void ril_utf_oem_if::update_default_lte_direct_discovery_unsol_response_service_status(void)
{
  payload = (void *) malloc(sizeof(LteDirectDiscovery_ServiceStatus));
  payload_len = sizeof(LteDirectDiscovery_ServiceStatus);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_oem_indication;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_UNSOL_LTE_DIRECT_DISC;
#else
  oem_command_id = QCRIL_EVT_HOOK_UNSOL_LTE_DIRECT_DISC;
#endif
  msg_id = LteDirectDiscovery_MsgId_UNSOL_RESPONSE_SERVICE_STATUS;
}
void ril_utf_oem_if::set_lte_direct_discovery_unsol_response_service_status_publish_allowed(uint32_t publish_allowed)
{
  LteDirectDiscovery_ServiceStatus *ptr = (LteDirectDiscovery_ServiceStatus *)payload;
  ptr->has_publishAllowed = TRUE;
  ptr->publishAllowed = publish_allowed;
}
void ril_utf_oem_if::set_lte_direct_discovery_unsol_response_service_status_subscribe_allowed(uint32_t subscribe_allowed)
{
  LteDirectDiscovery_ServiceStatus *ptr = (LteDirectDiscovery_ServiceStatus *)payload;
  ptr->has_subscribeAllowed = TRUE;
  ptr->subscribeAllowed = subscribe_allowed;
}

/*========================================================
  LteDirectDiscovery_MsgId_UNSOL_RESPONSE_AUTHORIZATION_RESULT
========================================================*/
void ril_utf_oem_if::update_default_lte_direct_discovery_unsol_response_authorization_result(void)
{
  payload = (void *) malloc(sizeof(LteDirectDiscovery_AuthorizationResult));
  payload_len = sizeof(LteDirectDiscovery_AuthorizationResult);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_oem_indication;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_UNSOL_LTE_DIRECT_DISC;
#else
  oem_command_id = QCRIL_EVT_HOOK_UNSOL_LTE_DIRECT_DISC;
#endif
  msg_id = LteDirectDiscovery_MsgId_UNSOL_RESPONSE_AUTHORIZATION_RESULT;
}
void ril_utf_oem_if::set_lte_direct_discovery_unsol_response_authorization_result_os_app_id(char * os_app_id)
{
  LteDirectDiscovery_AuthorizationResult *ptr = (LteDirectDiscovery_AuthorizationResult *)payload;
  int len = strlen(os_app_id);
  ptr->osAppId.arg = malloc(len + 1);
  memset(ptr->osAppId.arg, 0, len + 1);
  memcpy (ptr->osAppId.arg, os_app_id, len);
}
void ril_utf_oem_if::set_lte_direct_discovery_unsol_response_authorization_result_result(LteDirectDiscovery_Result result)
{
  LteDirectDiscovery_AuthorizationResult *ptr = (LteDirectDiscovery_AuthorizationResult *)payload;
  ptr->has_result = TRUE;
  ptr->result = result;
}

/*========================================================
  LteDirectDiscovery_MsgId_UNSOL_RESPONSE_EXPRESSION_STATUS
========================================================*/
void ril_utf_oem_if::update_default_lte_direct_discovery_unsol_response_expression_status(void)
{
  payload = (void *) malloc(sizeof(LteDirectDiscovery_ExpressionStatus));
  payload_len = sizeof(LteDirectDiscovery_ExpressionStatus);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_oem_indication;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_UNSOL_LTE_DIRECT_DISC;
#else
  oem_command_id = QCRIL_EVT_HOOK_UNSOL_LTE_DIRECT_DISC;
#endif
  msg_id = LteDirectDiscovery_MsgId_UNSOL_RESPONSE_EXPRESSION_STATUS;
}
void ril_utf_oem_if::set_lte_direct_discovery_unsol_response_expression_status_os_app_id(char * os_app_id)
{
  LteDirectDiscovery_ExpressionStatus *ptr = (LteDirectDiscovery_ExpressionStatus *)payload;
  int len = strlen(os_app_id);
  ptr->osAppId.arg = malloc(len + 1);
  memset(ptr->osAppId.arg, 0, len + 1);
  memcpy (ptr->osAppId.arg, os_app_id, len);
}
void ril_utf_oem_if::set_lte_direct_discovery_unsol_response_expression_status_expression(char * expression)
{
  LteDirectDiscovery_ExpressionStatus *ptr = (LteDirectDiscovery_ExpressionStatus *)payload;
  int len = strlen(expression);
  ptr->expression.arg = malloc(len + 1);
  memset(ptr->expression.arg, 0, len + 1);
  memcpy (ptr->expression.arg, expression, len);
}
void ril_utf_oem_if::set_lte_direct_discovery_unsol_response_expression_status_result(LteDirectDiscovery_Result result)
{
  LteDirectDiscovery_ExpressionStatus *ptr = (LteDirectDiscovery_ExpressionStatus *)payload;
  ptr->has_result = TRUE;
  ptr->result = result;
}

/*========================================================
  LteDirectDiscovery_MsgId_UNSOL_RESPONSE_TRANSMISSION_STATUS
========================================================*/
void ril_utf_oem_if::update_default_lte_direct_discovery_unsol_response_transmission_status(void)
{
  payload = (void *) malloc(sizeof(LteDirectDiscovery_TransmissionStatus));
  payload_len = sizeof(LteDirectDiscovery_TransmissionStatus);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_oem_indication;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_UNSOL_LTE_DIRECT_DISC;
#else
  oem_command_id = QCRIL_EVT_HOOK_UNSOL_LTE_DIRECT_DISC;
#endif
  msg_id = LteDirectDiscovery_MsgId_UNSOL_RESPONSE_TRANSMISSION_STATUS;
}
void ril_utf_oem_if::set_lte_direct_discovery_unsol_response_transmission_status_os_app_id(char * os_app_id)
{
  LteDirectDiscovery_TransmissionStatus *ptr = (LteDirectDiscovery_TransmissionStatus *)payload;
  int len = strlen(os_app_id);
  ptr->osAppId.arg = malloc(len + 1);
  memset(ptr->osAppId.arg, 0, len + 1);
  memcpy (ptr->osAppId.arg, os_app_id, len);
}
void ril_utf_oem_if::set_lte_direct_discovery_unsol_response_transmission_status_expression(char * expression)
{
  LteDirectDiscovery_TransmissionStatus *ptr = (LteDirectDiscovery_TransmissionStatus *)payload;
  int len = strlen(expression);
  ptr->expression.arg = malloc(len + 1);
  memset(ptr->expression.arg, 0, len + 1);
  memcpy (ptr->expression.arg, expression, len);
}
void ril_utf_oem_if::set_lte_direct_discovery_unsol_response_transmission_status_status(uint32_t status)
{
  LteDirectDiscovery_TransmissionStatus *ptr = (LteDirectDiscovery_TransmissionStatus *)payload;
  ptr->has_status = TRUE;
  ptr->status = status;
}

/*========================================================
  LteDirectDiscovery_MsgId_UNSOL_RESPONSE_RECEPTION_STATUS
========================================================*/
void ril_utf_oem_if::update_default_lte_direct_discovery_unsol_response_reception_status(void)
{
  payload = (void *) malloc(sizeof(LteDirectDiscovery_TransmissionStatus));
  payload_len = sizeof(LteDirectDiscovery_TransmissionStatus);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_oem_indication;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_UNSOL_LTE_DIRECT_DISC;
#else
  oem_command_id = QCRIL_EVT_HOOK_UNSOL_LTE_DIRECT_DISC;
#endif
  msg_id = LteDirectDiscovery_MsgId_UNSOL_RESPONSE_RECEPTION_STATUS;
}
void ril_utf_oem_if::set_lte_direct_discovery_unsol_response_reception_status_os_app_id(char * os_app_id)
{
  LteDirectDiscovery_TransmissionStatus *ptr = (LteDirectDiscovery_TransmissionStatus *)payload;
  int len = strlen(os_app_id);
  ptr->osAppId.arg = malloc(len + 1);
  memset(ptr->osAppId.arg, 0, len + 1);
  memcpy (ptr->osAppId.arg, os_app_id, len);
}
void ril_utf_oem_if::set_lte_direct_discovery_unsol_response_reception_status_expression(char * expression)
{
  LteDirectDiscovery_TransmissionStatus *ptr = (LteDirectDiscovery_TransmissionStatus *)payload;
  int len = strlen(expression);
  ptr->expression.arg = malloc(len + 1);
  memset(ptr->expression.arg, 0, len + 1);
  memcpy (ptr->expression.arg, expression, len);
}
void ril_utf_oem_if::set_lte_direct_discovery_unsol_response_reception_status_status(uint32_t status)
{
  LteDirectDiscovery_TransmissionStatus *ptr = (LteDirectDiscovery_TransmissionStatus *)payload;
  ptr->has_status = TRUE;
  ptr->status = status;
}

/*========================================================
  LteDirectDiscovery_MsgId_UNSOL_RESPONSE_MATCH_EVENT
========================================================*/
void ril_utf_oem_if::update_default_lte_direct_discovery_unsol_response_match_event(void)
{
  payload = (void *) malloc(sizeof(LteDirectDiscovery_MatchEvent));
  payload_len = sizeof(LteDirectDiscovery_MatchEvent);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_oem_indication;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_UNSOL_LTE_DIRECT_DISC;
#else
  oem_command_id = QCRIL_EVT_HOOK_UNSOL_LTE_DIRECT_DISC;
#endif
  msg_id = LteDirectDiscovery_MsgId_UNSOL_RESPONSE_MATCH_EVENT;
}
void ril_utf_oem_if::set_lte_direct_discovery_unsol_response_match_event_os_app_id(char * os_app_id)
{
  LteDirectDiscovery_MatchEvent *ptr = (LteDirectDiscovery_MatchEvent *)payload;
  int len = strlen(os_app_id);
  ptr->osAppId.arg = malloc(len + 1);
  memset(ptr->osAppId.arg, 0, len + 1);
  memcpy (ptr->osAppId.arg, os_app_id, len);
}
void ril_utf_oem_if::set_lte_direct_discovery_unsol_response_match_event_expression(char * expression)
{
  LteDirectDiscovery_MatchEvent *ptr = (LteDirectDiscovery_MatchEvent *)payload;
  int len = strlen(expression);
  ptr->expression.arg = malloc(len + 1);
  memset(ptr->expression.arg, 0, len + 1);
  memcpy (ptr->expression.arg, expression, len);
}
void ril_utf_oem_if::set_lte_direct_discovery_unsol_response_match_event_state(uint32_t state)
{
  LteDirectDiscovery_MatchEvent *ptr = (LteDirectDiscovery_MatchEvent *)payload;
  ptr->has_state = TRUE;
  ptr->state = state;
}
void ril_utf_oem_if::set_lte_direct_discovery_unsol_response_match_event_metadata(char * metadata)
{
  LteDirectDiscovery_MatchEvent *ptr = (LteDirectDiscovery_MatchEvent *)payload;
  int len = strlen(metadata);
  ptr->metaData.arg = malloc(len + 1);
  memset(ptr->metaData.arg, 0, len + 1);
  memcpy (ptr->metaData.arg, metadata, len);
}

/*========================================================
    QCRIL_REQ_HOOK_UNSOL_CDMA_BURST_DTMF
========================================================*/
void ril_utf_oem_if::update_default_qcril_evt_hook_cdma_burst_dtmf(char *digits, char *on_length, char *off_length)
{
  char data[QCRIL_QMI_VOICE_DTMF_FWD_BURST_PAYLOAD_LENGTH] = {0};
  if (on_length) {
    strlcpy(data, on_length, 4);
  }
  if (off_length) {
    strlcpy(data+4, off_length, 4);
  }
  if (digits) {
    strlcpy(data+8, digits, strlen(digits)+1);
  }
  int len = sizeof(data);
  payload = malloc (len);
  memcpy(payload, data, len);
  payload_len = len;
  payload_type = ril_utf_oem_indication;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_UNSOL_CDMA_BURST_DTMF;
#else
  oem_command_id = QCRIL_EVT_HOOK_UNSOL_CDMA_BURST_DTMF;
#endif
}
/*========================================================
  QCRIL_REQ_HOOK_UNSOL_CDMA_CONT_DTMF_START
 ========================================================*/
void ril_utf_oem_if::update_default_qcril_evt_hook_cdma_cont_dtmf_start(char digit)
{
  int len = sizeof(digit);
  payload = malloc (len);
  memcpy(payload, &digit, len);
  payload_len = len;
  payload_type = ril_utf_oem_indication;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_UNSOL_CDMA_CONT_DTMF_START;
#else
  oem_command_id = QCRIL_EVT_HOOK_UNSOL_CDMA_CONT_DTMF_START;
#endif
}

/*========================================================
    QCRIL_REQ_HOOK_UNSOL_CDMA_CONT_DTMF_STOP
========================================================*/
void ril_utf_oem_if::update_default_qcril_evt_hook_cdma_cont_dtmf_stop()
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_oem_indication;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_UNSOL_CDMA_CONT_DTMF_STOP;
#else
  oem_command_id = QCRIL_EVT_HOOK_UNSOL_CDMA_CONT_DTMF_STOP;
#endif
}
/*========================================================
  QCRIL_EVT_HOOK_UNSOL_SPEECH_CODEC_INFO
========================================================*/
void ril_utf_oem_if::update_default_qcril_evt_hook_unsol_speech_codec_info(int call_id, int codec, int mode)
{
  int data[3] = {0};
  int len = sizeof(data);
  payload = malloc (len);
  payload_len = len;
  payload_type = ril_utf_oem_indication;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_UNSOL_SPEECH_CODEC_INFO;
#else
  oem_command_id = QCRIL_EVT_HOOK_UNSOL_SPEECH_CODEC_INFO;
#endif

  data[0] = call_id;
  data[1] = codec;
  data[2] = mode;
  memcpy(payload, data, len);
}
/*========================================================
  QCRIL_REQ_HOOK_UNSOL_NSS_RELEASE
  ========================================================*/
void ril_utf_oem_if::update_default_qcril_evt_hook_unsol_nss_release(uint8_t callId, voice_nss_release_enum_v02 nssRelease)
{
  char data[50] = {0};
  memcpy(data, &nssRelease, sizeof(nssRelease));
  memcpy((data + sizeof(nssRelease)), &callId, sizeof(callId));
  size_t len = sizeof(callId) + sizeof(nssRelease);
  payload = malloc (len);
  memcpy(payload, data, len);
  payload_len = len;
  payload_type = ril_utf_oem_indication;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_UNSOL_NSS_RELEASE;
#else
  oem_command_id = QCRIL_EVT_HOOK_UNSOL_NSS_RELEASE;
#endif
}

/*========================================================
  QCRIL_REQ_HOOK_UNSOL_EXTENDED_DBM_INTL
  ========================================================*/
void ril_utf_oem_if::update_default_qcril_evt_hook_unsol_extended_dbm_intl(uint16_t mcc, uint8_t db_subtype, uint8_t chg_ind, uint8_t sub_unit, uint8_t unit)
{
  int data[QCRIL_QMI_VOICE_EXT_BRST_INTL_PAYLOAD_LENGTH];
  data[0] = mcc;
  data[1] = db_subtype;
  data[2] = chg_ind;
  data[3] = sub_unit;
  data[4] = unit;

  size_t len = sizeof(data);
  payload = malloc (sizeof(data));
  memcpy(payload, data, len);
  payload_len = len;
  payload_type = ril_utf_oem_indication;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_UNSOL_EXTENDED_DBM_INTL;
#else
  oem_command_id = QCRIL_EVT_HOOK_UNSOL_EXTENDED_DBM_INTL;
#endif
}

/*========================================================
  QCRIL_REQ_HOOK_UNSOL_CARD_STATE_CHANGE_IND
========================================================*/
void ril_utf_oem_if::update_default_qcril_evt_hook_unsol_card_state_change_ind(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_oem_indication;
#ifdef QMI_RIL_HAL_UTF
  oem_command_id = QCRIL_REQ_HOOK_UNSOL_CARD_STATE_CHANGE_IND;
#else
  oem_command_id = QCRIL_EVT_HOOK_UNSOL_CARD_STATE_CHANGE_IND;
#endif
}

/*========================================================
  QCRIL_REQ_HOOK_UNSOL_SIM_REFRESH
========================================================*/
RIL_Hook_SimRefreshResponse* ril_utf_oem_if::update_default_sim_refresh_unsol_event(void)
{
  payload = malloc(sizeof(RIL_Hook_SimRefreshResponse));
  payload_len = sizeof(RIL_Hook_SimRefreshResponse) ;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_oem_indication;
  oem_command_id = QCRIL_REQ_HOOK_UNSOL_SIM_REFRESH;
  service_id = QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_NONE;

  return (RIL_Hook_SimRefreshResponse*)payload;
}

#endif
