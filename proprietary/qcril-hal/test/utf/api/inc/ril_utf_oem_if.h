/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_oem_if.h
  @brief   oem API's for test case dev

  DESCRIPTION

  API's for modifying OEM messages used in RIL UTF tests.
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef RIL_UTF_OEM_IF_H_
#define RIL_UTF_OEM_IF_H_

#include "ril_utf_if.h"
#include "modules/voice/qcril_qmi_voice.h"
#include "modules/oemhook/qcril_qmi_oemhook_service.h"

//RIL interfaces extracted from ril_utf_if.h
typedef class ril_utf_oem_if : public ril_utf_node_base_t {
public:
//==================================================
// OEM Interfaces
//==================================================

////////////////// Presence ////////////////////////
  /*
    QCRIL_EVT_HOOK_UNSOL_WMS_READY
  */
  void update_default_qcril_evt_hook_unsol_wms_ready(void);

  /*
    QCRIL_EVT_HOOK_UNSOL_CSG_ID_CHANGE_IND
  */
  void update_default_qcril_evt_hook_unsol_csg_id();
  void update_qcril_evt_hook_unsol_with_csg_id(uint32_t);

  /*
    QCRIL_EVT_HOOK_UNSOL_MAX_DATA_CHANGE_IND
  */
  void update_default_qcril_evt_hook_unsol_max_data_sub_change();
  void update_qcril_evt_hook_unsol_with_max_data_sub(uint8_t max_data_sub);

  /*
    QCRIL_EVT_HOOK_UNSOL_SS_ERROR_CODE
  */
  void update_default_qcril_evt_hook_unsol_ss_error_code(void);

//////////////////// EMBMS /////////////////////////
  /*
    QMI_EMBMS_ENABLE_EMBMS_REQ_V01
  */
  void update_default_qmi_embms_enable_embms_req(void);

  /*
    QMI_EMBMS_UNSOL_SIB16_COVERAGE_IND_V01
  */
  void update_default_qmi_embms_unsol_sib16_coverage_ind_v01(void);
  void set_qmi_embms_unsol_sib16_coverage_ind_v01_in_converage(uint8_t in_coverage);

  /*
    QMI_EMBMS_GET_SIB16_COVERAGE_REQ_V01
  */
  void update_default_qmi_embms_get_sib16_coverage_req_v01(void);

  /*
    QMI_EMBMS_GET_SIB16_COVERAGE_RESP_V01
  */
  void update_default_qmi_embms_get_sib16_coverage_resp_v01(void);
  void set_qmi_embms_get_sib16_coverage_resp_v01_in_converage(uint8_t in_coverage);

  /*
    QMI_EMBMS_ENABLE_EMBMS_RESP_V01
  */
  void update_default_qmi_embms_enable_embms_resp_v01(void);
  void set_qmi_embms_enable_embms_resp_resp_code(int32_t resp_code);
  void set_qmi_embms_enable_embms_resp_call_id(uint8_t call_id);
  void set_qmi_embms_enable_embms_resp_interface_name(char *name);
  void set_qmi_embms_enable_embms_resp_if_index(uint32_t if_index);

  /*
    QMI_EMBMS_DISABLE_EMBMS_REQ_V01
  */
  void update_default_qmi_embms_disable_embms_req_v01(uint8_t call_id);

  /*
    QMI_EMBMS_DISABLE_EMBMS_RESP_V01
  */
  void update_default_qmi_embms_disable_embms_resp_v01(void);
  void set_qmi_embms_disable_embms_resp_resp_code(int32_t resp_code);
  void set_qmi_embms_disable_embms_resp_call_id(uint8_t call_id);

  /*
    QMI_EMBMS_ACTIVATE_TMGI_REQ_V01
  */
  void update_default_qmi_embms_activate_tmgi_req_v01(void);
  void set_qmi_embms_activate_tmgi_req_call_id(uint8_t call_id);
  void set_qmi_embms_activate_tmgi_req_tmgi_info(embms_tmgi_type_v01 tmgi_info);
  void set_qmi_embms_activate_tmgi_req_preemption_priority(int32_t preemption_priority);

  /*
    QMI_EMBMS_ACTIVATE_TMGI_RESP_V01
  */
  void update_default_qmi_embms_activate_tmgi_resp_v01(void);
  void set_qmi_embms_activate_tmgi_resp_resp_code(int32_t resp_code);
  void set_qmi_embms_activate_tmgi_resp_call_id(uint8_t call_id);
  void set_qmi_embms_activate_tmgi_resp_tmgi_info(embms_tmgi_type_v01 tmgi_info);

  /*
    QMI_EMBMS_DEACTIVATE_TMGI_REQ_V01
  */
  void update_default_qmi_embms_deactivate_tmgi_req_v01(void);
  void set_qmi_embms_deactivate_tmgi_req_call_id(uint8_t call_id);

  /*
    QMI_EMBMS_DEACTIVATE_TMGI_RESP_V01
  */
  void update_default_qmi_embms_deactivate_tmgi_resp_v01(void);
  void set_qmi_embms_deactivate_tmgi_resp_resp_code(int32_t resp_code);
  void set_qmi_embms_deactivate_tmgi_resp_call_id(uint8_t call_id);
  void set_qmi_embms_deactivate_tmgi_resp_tmgi_info(embms_tmgi_type_v01 tmgi_info);

  /*
    QMI_EMBMS_GET_AVAILABLE_TMGI_REQ_V01
  */
  void update_default_qmi_embms_get_available_tmgi_req_v01(void);
  void set_qmi_embms_get_available_tmgi_req_call_id(int8_t call_id);

  /*
    QMI_EMBMS_GET_AVAILABLE_TMGI_RESP_V01
  */
  void update_default_qmi_embms_get_available_tmgi_resp_v01(void);
  void set_qmi_embms_get_available_tmgi_resp(int32_t resp_code);

  /*
    QMI_EMBMS_GET_ACTIVE_TMGI_REQ_V01
  */
  void update_default_qmi_embms_get_active_tmgi_req_v01(void);
  void set_default_qmi_embms_get_active_tmgi_req_call_id(uint8_t call_id);

  /*
    QMI_EMBMS_GET_ACTIVE_TMGI_RESP_V01
  */
  void update_default_qmi_embms_get_active_tmgi_resp_v01(void);
  void set_qmi_embms_get_active_tmgi_resp(int32_t resp_code);

  /*
    QMI_EMBMS_ENABLE_RSSI_REQ_V01
  */
  void update_default_qmi_embms_enable_rssi_req_v01(void);

  /*
    QMI_EMBMS_ENABLE_RSSI_RESP_V01
  */
  void update_default_qmi_embms_enable_rssi_resp_v01(void);

  /*
    QMI_EMBMS_DISABLE_RSSI_REQ_V01
  */
  void update_default_qmi_embms_disable_rssi_req_v01(void);

  /*
    QMI_EMBMS_DISABLE_RSSI_RESP_V01
  */
  void update_default_qmi_embms_disable_rssi_resp_v01(void);

  /*
    QMI_EMBMS_GET_COVERAGE_STATE_REQ_V01
  */
  void update_default_qmi_embms_get_coverage_state_req_v01(void);

  /*
    QMI_EMBMS_GET_COVERAGE_STATE_RESP_V01
  */
  void update_default_qmi_embms_get_coverage_state_resp_v01(void);
  void set_qmi_embms_get_coverage_state_resp_resp_code(int32_t resp_code);
  void set_qmi_embms_get_coverage_state_resp_coverage_state(uint32_t coverage_state);

  /*
    QMI_EMBMS_GET_RSSI_REQ_V01
  */
  void update_default_qmi_embms_get_rssi_req_v01(int32_t dbg_trace_id);

  /*
    QMI_EMBMS_GET_RSSI_RESP_V01
  */
  embms_get_rssi_resp_msg_v01 *update_default_qmi_embms_get_rssi_resp_v01(void);

  /*
    QMI_EMBMS_GET_EMBMS_SERVICE_STATE_REQ_V01
  */
  void update_default_qmi_embms_get_embms_service_state_req_v01(void);

  /*
    QMI_EMBMS_GET_EMBMS_SERVICE_STATE_RESP_V01
  */
  void update_default_qmi_embms_get_embms_service_state_resp_v01(void);
  void set_qmi_embms_get_embms_service_state_resp_resp_code(int32_t resp_code);
  void set_qmi_embms_get_embms_service_state_resp_embms_service_state(uint32_t embms_service_state);

  /*
    QMI_EMBMS_UNSOL_EMBMS_SERVICE_STATE_IND_V01
  */
  void update_default_qmi_embms_unsol_embms_service_state_ind_v01(void);
  void set_qmi_embms_unsol_embms_service_state_ind_state(uint32_t state);
  void state_qmi_embms_unsol_embms_service_state_ind_interface_name(char *interface_name);

  /*
    QMI_EMBMS_ACTIVE_TMGI_IND_V01
  */
  void update_default_qmi_embms_active_tmgi_ind_v01(void);

  /*
    QMI_EMBMS_UNSOL_BROADCAST_COVERAGE_IND_V01
  */
  void update_default_qmi_embms_unsol_broadcast_coverage_ind_v01(void);
  void set_qmi_embms_unsol_broadcast_coverage_ind_broadcast_coverage(uint32_t broadcast_coverage);

  /*
    QMI_EMBMS_UNSOL_RSSI_IND_V01
  */
  void update_default_qmi_embms_unsol_rssi_ind_v01(void);
  void set_qmi_embms_unsol_rssi_ind_rssi(uint32_t rssi);

  /*
    QMI_EMBMS_AVAILABLE_TMGI_IND_V01
  */
  void update_default_qmi_embms_available_tmgi_ind_v01(void);

  /*
    QMI_EMBMS_OOS_WARNING_IND_V01
  */
  void update_default_qmi_embms_oos_warning_ind_v01(void);

  /*
    QMI_EMBMS_ACTIVATE_DEACTIVATE_TMGI_REQ_V01
  */
  void update_default_qmi_embms_activate_deactivate_tmgi_req_v01(void);
  void set_qmi_embms_activate_deactivate_tmgi_req_call_id(uint8_t call_id);
  void set_qmi_embms_activate_deactivate_tmgi_req_act_tmgi_info(embms_tmgi_type_v01 act_tmgi_info);
  void set_qmi_embms_activate_deactivate_tmgi_req_deact_tmgi_info(embms_tmgi_type_v01 deact_tmgi_info);
  void set_qmi_embms_activate_deactivate_tmgi_req_preemption_priority(int32_t preemption_priority);
  int32_t* set_qmi_embms_activate_deactivate_tmgi_req_earfcnlist(uint32_t earfcnlist_len);

  /*
    QMI_EMBMS_ACTIVATE_DEACTIVATE_TMGI_RESP_V01
  */
  void update_default_qmi_embms_activate_deactivate_tmgi_resp_v01(void);
  void set_qmi_embms_activate_deactivate_tmgi_resp_act_resp_code(uint16 act_resp_code);
  void set_qmi_embms_activate_deactivate_tmgi_resp_deact_resp_code(uint16_t deact_resp_code);
  void set_qmi_embms_activate_deactivate_tmgi_resp_call_id(uint8_t call_id);
  void set_qmi_embms_activate_deactivate_tmgi_resp_act_tmgi_info(embms_tmgi_type_v01 act_tmgi_info);
  void set_qmi_embms_activate_deactivate_tmgi_resp_deact_tmgi_info(embms_tmgi_type_v01 deact_tmgi_info);

  /*
    QMI_EMBMS_CELL_INFO_CHANGED_IND_V01
  */
  void update_default_qmi_embms_cell_info_changed_ind_v01(void);

  /*
    QMI_EMBMS_RADIO_STATE_IND_V01
  */
  void update_default_qmi_embms_radio_state_ind_v01(void);

  /*
    QMI_EMBMS_SAI_IND_V01
  */
  void update_default_qmi_embms_sai_ind_v01(void);

  /*
    QMI_EMBMS_GET_ACTIVE_LOG_PACKET_IDS_REQ_V01
  */
  void update_default_qmi_embms_get_active_log_packet_ids_req_v01(void);
  int32_t* set_qmi_embms_get_active_log_packet_ids_req_supported_log_packet_id_list(uint32_t supported_log_packet_id_list_len);

  /*
    QMI_EMBMS_GET_ACTIVE_LOG_PACKET_IDS_RESP_V01
  */
  void update_default_qmi_embms_get_active_log_packet_ids_resp_v01(void);
  int32_t* set_qmi_embms_get_active_log_packet_ids_resp_active_log_packet_id_list(uint32_t active_log_packet_id_list_len);

  /*
    QMI_EMBMS_DELIVER_LOG_PACKET_REQ_V01
  */
  void update_default_qmi_embms_deliver_log_packet_req_v01(void);
  void set_qmi_embms_deliver_log_packet_req_packet_id(int32_t packet_id);
  int8_t* set_qmi_embms_deliver_log_packet_req_log_packet(uint32_t log_packet_len);

  /*
    QMI_EMBMS_DELIVER_LOG_PACKET_RESP_V01
  */
  void update_default_qmi_embms_deliver_log_packet_resp_v01(void);

  /*
    QMI_EMBMS_SET_SNTP_TIME_REQ_V01
  */
  void update_default_qmi_embms_set_sntp_time_req_v01(void);
  void set_qmi_embms_set_sntp_time_req_sntp_available(uint8_t sntp_available);
  void set_qmi_embms_set_sntp_time_req_sntp_time_milli_sec(uint64_t sntp_time_milli_sec);
  void set_qmi_embms_set_sntp_time_req_time_stamp_milli_sec(uint64_t time_stamp_milli_sec);

  /*
    QMI_EMBMS_SET_SNTP_TIME_RESP_V01
  */
  void update_default_qmi_embms_set_sntp_time_resp_v01(void);

  /*
    QMI_EMBMS_GET_UTC_TIME_REQ_V01
  */
  void update_default_qmi_embms_get_utc_time_req_v01(void);

  /*
    QMI_EMBMS_GET_UTC_TIME_RESP_V01
  */
  void update_default_qmi_embms_get_utc_time_resp_v01(void);
  void set_qmi_embms_get_utc_time_resp_resp_code(int32_t resp_code);
  void set_qmi_embms_get_utc_time_resp_milli_sec(uint64_t milli_sec);
  void set_qmi_embms_get_utc_time_resp_day_light_saving(uint8_t day_light_saving);
  void set_qmi_embms_get_utc_time_resp_leap_seconds(int8_t leap_seconds);
  void set_qmi_embms_get_utc_time_resp_local_time_offset(int8_t local_time_offset);

  /*
    QMI_EMBMS_UNSOL_E911_STATUS_IND_V01
  */
  void update_default_qmi_embms_unsol_e911_state_ind_v01(void);
  void set_qmi_embms_unsol_e911_state_ind_v01_e911_state(embms_e911_mode_v01 e911_state);

#ifdef RIL_UTF_L_MASTER
  /*
    QMI_EMBMS_GET_SIB_PLMN_REQ_V01
  */
  void update_default_qmi_embms_get_sib_plmn_req_v01(void);
  void set_qmi_embms_get_sib_plmn_req_dbg_trace_id(int32_t dbg_trace_id);

  /*
    QMI_EMBMS_GET_SIB_PLMN_RESP_V01
  */
  void update_default_qmi_embms_get_sib_plmn_resp_v01(void);
  void update_default_qmi_embms_get_sib_plmn_resp_v01(RIL_Errno err);
  void set_qmi_embms_get_sib_plmn_resp_dbg_trace_id(int32_t dbg_trace_id);
  void set_qmi_embms_get_sib_plmn_resp_plmn(embms_plmn_type_v01 plmn[], uint32_t plmn_len);
  /*
    QMI_EMBMS_UNSOL_EMBMS_STATUS_IND_V01
  */
  void update_default_qmi_embms_unsol_embms_status_ind(void);
  void set_qmi_embms_unsol_embms_status(uint8_t is_available);

  /*
    QMI_EMBMS_GET_EMBMS_STATUS_REQ_V01
  */
  void update_default_qmi_embms_get_embms_status_req_v01(void);
  void set_qmi_embms_get_embms_status_req_dbg_trace_id(int32_t dbg_trace_id);

  /*
    QMI_EMBMS_GET_EMBMS_STATUS_RESP_V01
  */
  void update_default_qmi_embms_get_embms_status_resp_v01(void);
  void update_default_qmi_embms_get_embms_status_resp_v01(RIL_Errno err);
  void set_qmi_embms_get_embms_status_resp_dbg_trace_id(int32_t dbg_trace_id);
  void set_qmi_embms_get_embms_status_resp_available(uint8_t is_available);
#endif

#ifndef UTF_TARGET_BUILD
  /*
    QCRIL_EVT_HOOK_GET_CURRENT_SETUP_CALLS
  */
  void update_default_qcril_evt_hook_get_current_setup_calls();
  /*
    QCRIL_EVT_HOOK_GET_CURRENT_SETUP_CALLS (resp)
  */
  qcril_qmi_voice_setup_call_info *update_default_qcril_evt_hook_get_current_setup_calls_resp (boolean call_present);
  /*
    QCRIL_EVT_HOOK_REQUEST_SETUP_ANSWER
  */
  void update_default_qcril_evt_hook_request_setup_answer(boolean rejection);
  /*
   QCRIL_EVT_HOOK_REQUEST_SETUP_ANSWER (resp)
  */
  void update_default_qcril_evt_hook_request_setup_answer_resp();
  /*
   QCRIL_REQ_HOOK_REJECT_INCOMING_CALL_CAUSE_21
  */
  void update_default_qcril_evt_hook_reject_incoming_call_cause_21();
  /*
   QCRIL_REQ_HOOK_REJECT_INCOMING_CALL_CAUSE_21 (resp)
  */
  void update_default_qcril_evt_hook_reject_incoming_call_cause_21_resp();
  /*
    QCRIL_REQ_HOOK_SET_LOCAL_CALL_HOLD_v01
  */
  void update_default_qcril_evt_hook_request_set_local_call_hold(int lch);
  /*
    QCRIL_REQ_HOOK_SET_LOCAL_CALL_HOLD_v01 (resp)
  */
  void update_default_qcril_evt_hook_request_set_local_call_hold_resp();

  /*========================================================
    QCRIL_REQ_HOOK_UNSOL_CDMA_BURST_DTMF
    ========================================================*/
  void update_default_qcril_evt_hook_cdma_burst_dtmf(char *digits, char *on_length, char *off_length);

  /*========================================================
    QCRIL_REQ_HOOK_UNSOL_CDMA_CONT_DTMF_START
    ========================================================*/
  void update_default_qcril_evt_hook_cdma_cont_dtmf_start(char digits);

  /*========================================================
    QCRIL_REQ_HOOK_UNSOL_CDMA_CONT_DTMF_STOP
    ========================================================*/
  void update_default_qcril_evt_hook_cdma_cont_dtmf_stop();

  /*========================================================
    QCRIL_REQ_HOOK_UNSOL_SPEECH_CODEC_INFO
    ========================================================*/
  void update_default_qcril_evt_hook_unsol_speech_codec_info(int call_id, int codec, int mode);

  /*========================================================
    QCRIL_REQ_HOOK_UNSOL_NSS_RELEASE
    ========================================================*/
  void update_default_qcril_evt_hook_unsol_nss_release(uint8_t callId, voice_nss_release_enum_v02 nssRelease);

  /*========================================================
    QCRIL_REQ_HOOK_UNSOL_EXTENDED_DBM_INTL
    ========================================================*/
  void update_default_qcril_evt_hook_unsol_extended_dbm_intl(uint16_t mcc, uint8_t db_subtype, uint8_t chg_ind, uint8_t sub_unit, uint8_t unit);
  /*
TODO:
   QCRIL_REQ_HOOK_UNSOL_SS_ERROR_CODE
  */


 /*========================================================
  QCRIL_EVT_HOOK_SET_CDMA_SUB_SRC_WITH_SPC
 ========================================================*/
  void update_default_oem_request_cdma_set_subscription_source(oem_hook_cdma_subscription_source_e_type sub_source, char *spc, int spc_len);
  void update_default_oem_request_cdma_set_subscription_source_resp(boolean result);

  /*========================================================
  QCRIL_EVT_HOOK_SET_LTE_TUNE_AWAY
 ========================================================*/
  void update_default_oem_request_set_lte_tune_away_req(boolean lte_tune_away);
  void update_default_oem_request_set_lte_tune_away_resp();

#endif

  /*
   QCRIL_EVT_HOOK_UNSOL_WWAN_AVAILABLE
  */
  void update_default_qcril_evt_hook_unsol_wlan_available(uint8_t is_wlan_status);

/*========================================================
  QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION
========================================================*/
#ifdef RIL_UTF_L_MASTER
  void update_default_oem_request_set_data_subscription_req(int data_subsciption);
  void update_default_oem_request_set_data_subscription_resp();
#endif
/*
  QCRIL_EVT_HOOK_UNSOL_MODEM_CAPABILITY
*/
void update_default_qcril_evt_hook_unsol_modem_capability(uint8_t *data, size_t len);


/*========================================================
  LteDirectDiscovery_MsgId_REQUEST_GET_DEVICE_CAPABILITY
========================================================*/
void update_default_lte_direct_discovery_get_device_capability_req(void);

/*========================================================
  LteDirectDiscovery_MsgId_REQUEST_GET_DEVICE_CAPABILITY(Response)
========================================================*/
void update_default_lte_direct_discovery_get_device_capability_resp(void);
void set_lte_direct_discovery_get_device_capability_resp_capability(uint32_t capability);

/*========================================================
  LteDirectDiscovery_MsgId_REQUEST_TERMINATE
========================================================*/
void update_default_lte_direct_discovery_terminate_req(void);
void set_lte_direct_discovery_terminate_req_os_app_id(char * os_app_id);

/*========================================================
  LteDirectDiscovery_MsgId_REQUEST_TERMINATE(Response)
========================================================*/
void update_default_lte_direct_discovery_terminate_resp(void);

/*========================================================
  LteDirectDiscovery_MsgId_REQUEST_GET_SERVICE_STATUS
========================================================*/
void update_default_lte_direct_discovery_get_service_status_req(void);

/*========================================================
  LteDirectDiscovery_MsgId_REQUEST_GET_SERVICE_STATUS(Response)
========================================================*/
void update_default_lte_direct_discovery_get_service_status_resp(void);
void set_lte_direct_discovery_get_service_status_resp_publish_allowed(uint32_t publish_allowed);
void set_lte_direct_discovery_get_service_status_resp_subscribe_allowed(uint32_t subscribe_allowed);

/*========================================================
  LteDirectDiscovery_MsgId_REQUEST_PUBLISH
========================================================*/
void update_default_lte_direct_discovery_publish_req(void);
void set_lte_direct_discovery_publish_req_os_app_id(char * os_app_id);
void set_lte_direct_discovery_publish_req_expression(char * expression);
void set_lte_direct_discovery_publish_req_expression_validity_time(uint32_t expression_validity_time);
void set_lte_direct_discovery_publish_req_meta_data(char * meta_data);
void set_lte_direct_discovery_publish_req_discovery_type(uint32_t discovery_type);
void set_lte_direct_discovery_publish_req_duration(uint32_t duration);

/*========================================================
  LteDirectDiscovery_MsgId_REQUEST_PUBLISH(Response)
========================================================*/
void update_default_lte_direct_discovery_publish_resp(void);

/*========================================================
  LteDirectDiscovery_MsgId_REQUEST_CANCEL_PUBLISH
========================================================*/
void update_default_lte_direct_discovery_cancel_publish_req(void);
void set_lte_direct_discovery_cancel_publish_req_os_app_id(char * os_app_id);
void set_lte_direct_discovery_cancel_publish_req_expression(char * expression);

/*========================================================
  LteDirectDiscovery_MsgId_REQUEST_CANCEL_PUBLISH(Response)
========================================================*/
void update_default_lte_direct_discovery_cancel_publish_resp(void);

/*========================================================
  LteDirectDiscovery_MsgId_REQUEST_SUBSCRIBE
========================================================*/
void update_default_lte_direct_discovery_subscribe_req(void);
void set_lte_direct_discovery_subscribe_req_os_app_id(char * os_app_id);
void set_lte_direct_discovery_subscribe_req_expression(char * expression);
void set_lte_direct_discovery_subscribe_req_expression_validity_time(uint32_t expression_validity_time);
void set_lte_direct_discovery_subscribe_req_discovery_type(uint32_t discovery_type);
void set_lte_direct_discovery_subscribe_req_duration(uint32_t duration);

/*========================================================
  LteDirectDiscovery_MsgId_REQUEST_SUBSCRIBE(Response)
========================================================*/
void update_default_lte_direct_discovery_subscribe_resp(void);

/*========================================================
  LteDirectDiscovery_MsgId_REQUEST_CANCEL_SUBSCRIBE
========================================================*/
void update_default_lte_direct_discovery_cancel_subscribe_req(void);
void set_lte_direct_discovery_cancel_subscribe_req_os_app_id(char * os_app_id);
void set_lte_direct_discovery_cancel_subscribe_req_expression(char * expression);

/*========================================================
  LteDirectDiscovery_MsgId_REQUEST_CANCEL_SUBSCRIBE(Response)
========================================================*/
void update_default_lte_direct_discovery_cancel_subscribe_resp(void);

/*========================================================
  LteDirectDiscovery_MsgId_UNSOL_RESPONSE_DEVICE_CAPABILITY_CHANGED
========================================================*/
void update_default_lte_direct_discovery_unsol_response_device_capability_changed(void);
void set_lte_direct_discovery_unsol_response_device_capability_changed_capability(uint32_t capability);

/*========================================================
  LteDirectDiscovery_MsgId_UNSOL_RESPONSE_SERVICE_STATUS
========================================================*/
void update_default_lte_direct_discovery_unsol_response_service_status(void);
void set_lte_direct_discovery_unsol_response_service_status_publish_allowed(uint32_t publish_allowed);
void set_lte_direct_discovery_unsol_response_service_status_subscribe_allowed(uint32_t subscribe_allowed);

/*========================================================
  LteDirectDiscovery_MsgId_UNSOL_RESPONSE_AUTHORIZATION_RESULT
========================================================*/
void update_default_lte_direct_discovery_unsol_response_authorization_result(void);
void set_lte_direct_discovery_unsol_response_authorization_result_os_app_id(char * os_app_id);
void set_lte_direct_discovery_unsol_response_authorization_result_result(LteDirectDiscovery_Result result);

/*========================================================
  LteDirectDiscovery_MsgId_UNSOL_RESPONSE_EXPRESSION_STATUS
========================================================*/
void update_default_lte_direct_discovery_unsol_response_expression_status(void);
void set_lte_direct_discovery_unsol_response_expression_status_os_app_id(char * os_app_id);
void set_lte_direct_discovery_unsol_response_expression_status_expression(char * expression);
void set_lte_direct_discovery_unsol_response_expression_status_result(LteDirectDiscovery_Result result);

/*========================================================
  LteDirectDiscovery_MsgId_UNSOL_RESPONSE_TRANSMISSION_STATUS
========================================================*/
void update_default_lte_direct_discovery_unsol_response_transmission_status(void);
void set_lte_direct_discovery_unsol_response_transmission_status_os_app_id(char * os_app_id);
void set_lte_direct_discovery_unsol_response_transmission_status_expression(char * expression);
void set_lte_direct_discovery_unsol_response_transmission_status_status(uint32_t status);

/*========================================================
  LteDirectDiscovery_MsgId_UNSOL_RESPONSE_RECEPTION_STATUS
========================================================*/
void update_default_lte_direct_discovery_unsol_response_reception_status(void);
void set_lte_direct_discovery_unsol_response_reception_status_os_app_id(char * os_app_id);
void set_lte_direct_discovery_unsol_response_reception_status_expression(char * expression);
void set_lte_direct_discovery_unsol_response_reception_status_status(uint32_t status);

/*========================================================
  LteDirectDiscovery_MsgId_UNSOL_RESPONSE_MATCH_EVENT
========================================================*/
void update_default_lte_direct_discovery_unsol_response_match_event(void);
void set_lte_direct_discovery_unsol_response_match_event_os_app_id(char * os_app_id);
void set_lte_direct_discovery_unsol_response_match_event_expression(char * expression);
void set_lte_direct_discovery_unsol_response_match_event_state(uint32_t state);
void set_lte_direct_discovery_unsol_response_match_event_metadata(char * metadata);

void update_default_qcril_evt_hook_unsol_card_state_change_ind(void);

/*=======================================================
  SIM REFRESH OEM HOOK INDICATION
=======================================================*/
RIL_Hook_SimRefreshResponse* update_default_sim_refresh_unsol_event(void);
} OEMTestnode;

#endif /* RIL_UTF_OEM_IF_H_*/
