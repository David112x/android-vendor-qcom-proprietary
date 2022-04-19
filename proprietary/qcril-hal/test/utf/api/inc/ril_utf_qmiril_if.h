/******************************************************************************
#  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#ifndef RIL_UTF_QMIRIL_IF_H_
#define RIL_UTF_QMIRIL_IF_H_

#include "ril_utf_if.h"
#define MASK_B(offset, len) \
  ((0xff >> offset) & (0xff << (8 - (offset + len))))

//QMI interfaces extracted from ril_utf_if.h
typedef class ril_utf_qmiril_if : public ril_utf_node_base_t {
public:
  //==================================================
  // QMI interfaces
  //==================================================
  /*
          QMI_CTL:
          - QMI_CTL_GET_CLIENT_ID_REQ
  */

  void update_default_qmi_get_client_id_req_msg(uint8_t qmi_svc_type);

  /*
    QMI_CTL_GET_CLIENT_ID_RESP
  */
  void update_default_qmi_get_client_id_resp_msg(uint8_t qmi_svc_type, uint8_t clnt_id);
  void set_qmi_get_client_id_resp_result(int result, int error);

  /*
     QMI_VOICE_INDICATION_REGISTER_REQ             // Replaced by newer version in the following code..

  void update_default_qmi_voice_ind_reg_req_msg(void);
  void set_qmi_voice_ind_req_reg_voice_privacy_events(int val);
  void set_qmi_voice_ind_req_ext_brst_intl_events(int val);
  void set_qmi_voice_ind_req_speech_events(int val);
 */
  /*
     QMI_VOICE_INDICATION_REGISTER_RESP      // Replaced by newer version in the following code..

  void update_default_qmi_voice_ind_reg_resp_msg(void);

  */

  void set_qmi_client_id(int id);
  void set_qmi_voice_indication_register_resp_result(qmi_result_type_v01 result, qmi_error_type_v01 error);


  /*
    QMI_DMS_SET_EVENT_REPORT_REQ_V01
  */
  void update_default_qmi_dms_set_event_report_req_msg(void);
  void set_qmi_dsm_set_event_report_req_report_oprt_mode_state(int val);
  void set_qmi_dsm_set_event_report_req_report_prl_init(int val);
  void set_qmi_dms_set_event_report_req_report_device_multisim_info(int val);

  /*
    QMI_DMS_SET_EVENT_REPORT_RESP_V01
  */
  void update_default_qmi_dms_set_event_report_resp_msg(void);
  void set_qmi_dms_set_event_report_resp_result(qmi_result_type_v01 result, qmi_error_type_v01 error);

  /*
    QMI_DMS_EVENT_REPORT_IND_V01
  */
  void update_default_qmi_dms_event_report_ind_v01(void);
  void set_qmi_dms_event_report_ind_v01_operating_mode(dms_operating_mode_enum_v01 operating_mode);
  void set_qmi_dms_event_report_ind_v01_prl_init(dms_prl_init_enum_v01 prl_init);
  void set_qmi_dms_event_report_ind_v01_multisim_capability(uint8_t max_subscriptions, uint32_t subs_config_list_len, uint8_t max_active);
  void set_qmi_dms_event_report_ind_v01_max_active_data_subscriptions(uint8_t max_active_data_subscriptions_valid, uint8_t max_active_data_subscriptions);

  /*
    QMI_DMS_IMS_CAPABILITY_IND_V01
  */
  dms_ims_capability_type_v01 *update_default_qmi_dms_ims_capability_ind_v01(uint32_t ims_capability_len);

  /*
    QMI_DMS_GET_DEVICE_CAP_REQ_V01
  */
  void update_default_qmi_dms_get_device_cap_req_v01(void);

  /*
    QMI_DMS_GET_DEVICE_CAP_RESP_V01
  */
  void update_default_qmi_dms_get_device_cap_resp_v01(void);
  void update_default_qmi_dms_get_device_cap_resp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error);
  void set_default_qmi_dms_get_device_cap_resp_v01_multisim_capability(dms_multisim_capability_type_v01 multisim_capability);
  void set_default_qmi_dms_get_device_cap_resp_v01_device_capabilities(void);
  void set_qmi_dms_get_device_cap_resp_v01_result(qmi_result_type_v01 result, qmi_error_type_v01 error);
  void set_default_qmi_dms_get_device_cap_resp_v01_max_active_data_subscriptions(uint8_t max_active_data_subscriptions_valid, uint8_t max_active_data_subscriptions);
  void set_default_qmi_dms_get_device_cap_resp_v01_max_ims_instances(uint8_t val);
  dms_ims_capability_type_v01 *set_default_qmi_dms_get_device_cap_resp_v01_ims_capability(uint32_t len);

  /*
     QMI_DMS_GET_OPERATING_MODE_REQ_V01
  */
  void update_default_qmi_dms_get_operating_mode_req_msg(void);

  /*
     QMI_DMS_GET_OPERATING_MODE_RESP_V01
  */
  void update_default_qmi_dms_get_operating_mode_resp_msg(void);
  void set_qmi_dms_get_operating_mode_resp_result(qmi_result_type_v01 result, qmi_error_type_v01 error);
  void set_qmi_dms_get_operating_mode_resp_operating_mode(dms_operating_mode_enum_v01 mode);

  /*
    QMI_DMS_SET_OPERATING_MODE_REQ_V01
  */
  void update_default_qmi_dms_set_operating_mod_req_v01(dms_operating_mode_enum_v01 o_mode);

  /*
    QMI_DMS_SET_OPERATING_MODE_RESP_V01
  */
  void update_default_qmi_dms_set_operating_mode_resp_v01(void);
  void set_qmi_dms_set_operating_mode_resp_v01_result(qmi_result_type_v01 result, qmi_error_type_v01 error);

  /*
    QMI_DMS_GET_DEVICE_MFR_REQ_V01
  */
  void update_default_qmi_dms_get_device_mfr_req_v01(void);

  /*
    QMI_DMS_GET_DEVICE_MFR_RESP_V01
  */
  void update_default_qmi_dms_get_device_mfr_resp_v01(void);
  void set_default_qmi_dms_get_device_mfr_resp_v01_device_manufacturer(void);
  void set_qmi_dms_get_device_mfr_resp_v01_result(qmi_result_type_v01 result, qmi_error_type_v01 error);

  /*
    QMI_DMS_GET_CURRENT_PRL_INFO_REQ_V01
  */
  void update_default_qmi_dms_get_current_prl_info_req_v01(void);

  /*
    QMI_DMS_GET_CURRENT_PRL_INFO_RESP_V01
  */
  void update_default_qmi_dms_get_current_prl_info_resp_v01(void);
  void set_qmi_dms_get_current_prl_info_resp_v01_result(qmi_result_type_v01 result, qmi_error_type_v01 error);

  /*
    QMI_DMS_TRIGGER_MODEM_ACTIVITY_INFO_CALCULATION_REQ_V01
  */
  void update_default_qmi_dms_trigger_modem_activity_info_calculation_req(int enable_statistics_valid , int enable_statistics);

  /*
    QMI_DMS_TRIGGER_MODEM_ACTIVITY_INFO_CALCULATION_RESP_V01
  */
  void update_default_qmi_dms_trigger_modem_activity_info_calculation_resp(void);

  /*
    QMI_DMS_GET_MODEM_ACTIVITY_INFO_REQ_V01
  */
  void update_default_qmi_dms_get_modem_activity_info_req(void);

  /*
    QMI_DMS_GET_MODEM_ACTIVITY_INFO_RESP_V01
  */
  void update_default_qmi_dms_get_modem_activity_info_resp(void);

  /*
    QMI_DMS_MODEM_ACTIVITY_INFO_IND_V01
  */
  void update_default_qmi_dms_get_modem_activity_info_ind(void);
  void set_qmi_dms_get_modem_activity_info_ind_is_newly_calculated_values(uint8_t is_newly_calculated_values);

  /*
    QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_V01
  */
  void update_default_qmi_nas_get_system_selection_preference_req_msg(void);

  /*
    QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01
  */
  void update_default_qmi_nas_get_system_selection_preference_resp_msg(void);
// void set_qmi_nas_get_system_selection_preference_resp_emergency_mode(uint8_t emergency_mode);
//  void set_qmi_nas_get_system_selection_preference_resp_mode_pref(mode_pref_mask_type_v01 mode_pref);
// void set_qmi_nas_get_system_selection_preference_resp_band_pref(nas_band_pref_mask_type_v01 band_pref);
//  void set_qmi_nas_get_system_selection_preference_resp_prl_pref(nas_prl_pref_enum_v01 prl_pref); // had to be removed due to existence of these functions in the below written code
//  void set_qmi_nas_get_system_selection_preference_resp_roam_pref(nas_roam_pref_enum_v01 roam_pref);
  void set_qmi_nas_get_system_selection_preference_resp_band_pref_ext(uint64_t band_pref_ext);
//  void set_qmi_nas_get_system_selection_preference_resp_net_sel_pref(nas_net_sel_pref_enum_v01 net_sel_pref);
  void set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(nas_srv_domain_pref_enum_type_v01 srv_domain_pref);
  void set_qmi_nas_get_system_selection_preference_resp_gw_acq_order_pref(nas_gw_acq_order_pref_enum_type_v01 gw_acq_order_pref);
  void set_qmi_nas_get_system_selection_preference_resp_tdscdma_band_pref(nas_tdscdma_band_pref_mask_type_v01 tdscdma_band_pref);
  void set_default_qmi_nas_get_system_selection_preference_resp_acq_order(void);
 // void set_qmi_nas_get_system_selection_preference_resp_srv_reg_restriction(nas_srv_reg_restriction_enum_v01 srv_reg_restriction);
  void set_qmi_nas_get_system_selection_preference_resp_usage_setting(nas_usage_pref_enum_v01 usage_setting);
  void set_qmi_nas_get_system_selection_preference_resp_result(qmi_result_type_v01 result, qmi_error_type_v01 error);
#ifdef RIL_UTF_L_MASTER
  void set_qmi_nas_get_system_selection_preference_resp_rat_disabled_mask(mode_pref_mask_type_v01 mask);
#endif

  /*
    QMI_NAS_SET_EVENT_REPORT_REQ_MSG_V01
  */
  void update_default_qmi_nas_set_event_report_req_msg_v01(void);
  void set_default_qmi_nas_set_event_report_req_signal_strength(void);
 // void set_qmi_nas_set_event_report_req_report_rf_band_info(uint8_t report_rf_band_info);
 // void set_qmi_nas_set_event_report_req_report_reg_reject(uint8_t report_reg_reject);
  void set_qmi_nas_set_event_report_req_rssi_indicator(uint8_t report_rssi, uint8_t rssi_delta);
  void set_qmi_nas_set_event_report_req_ecio_indicator(uint8_t report_ecio, uint8_t ecio_delta);
  void set_qmi_nas_set_event_report_req_io_indicator(uint8_t report_io, uint8_t io_delta);
  void set_qmi_nas_set_event_report_req_sinr_indicator(uint8_t report_sinr, uint8_t sinr_delta);
//  void set_qmi_nas_set_event_report_req_report_error_rate(uint8_t report_error_rate);
  void set_qmi_nas_set_event_report_req_rsrq_indicator(uint8_t report_rsrq, uint8_t rsrq_delta);

  /*
    QMI_NAS_SET_EVENT_REPORT_RESP_MSG_V01
  */
  void update_default_qmi_nas_set_event_report_resp_msg_v01(void);
  void set_qmi_nas_set_event_report_resp_msg_result(qmi_result_type_v01 result, qmi_error_type_v01 error);

  /*
    QMI_NAS_CONFIG_SIG_INFO_REQ_MSG_V01
  */
  void update_default_qmi_nas_config_sig_info_req_msg_v01(void);
  void set_default_qmi_nas_config_sig_info_req_msg_v01_ecio_threshold_list(void);

  /*
    QMI_NAS_CONFIG_SIG_INFO_RESP_MSG_V01
  */
  void update_default_qmi_nas_config_sig_info_resp_msg_v01(void);
  void set_qmi_nas_config_sig_info_resp_msg_result(qmi_result_type_v01 result, qmi_error_type_v01 error);

    /*
    QMI_NAS_CONFIG_SIG_INFO2_REQ_MSG_V01
  */
  void update_default_qmi_nas_config_sig_info2_req_msg_v01(void);
  void set_default_qmi_nas_config_sig_info2_req_msg_v01_cdma_ecio_delta(uint16_t cdma_ecio_delta);
  void set_default_qmi_nas_config_sig_info2_req_msg_v01_cdma_rssi_delta(uint16_t cdma_rssi_delta);
  void set_default_qmi_nas_config_sig_info2_req_msg_v01_gsm_rssi_delta(uint16_t gsm_rssi_delta);
  void set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_ecio_delta(uint16_t hdr_ecio_delta);
  void set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_io_delta(uint16_t hdr_io_delta);
  void set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_rssi_delta(uint16_t hdr_rssi_delta);
  void set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_sinr_delta(uint16_t hdr_sinr_delta);
  void set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rsrp_delta(uint16_t lte_rsrp_delta);
  void set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rsrq_delta(uint16_t lte_rsrq_delta);
  void set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rssi_delta(uint16_t lte_rssi_delta);
  void set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_snr_delta(uint16_t lte_snr_delta);
  void set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rpt_rate_avg_period(uint16_t lte_rpt_rate, uint16_t lte_avg_period);
  void set_default_qmi_nas_config_sig_info2_req_msg_v01_wcdma_ecio_delta(uint16_t wcdma_ecio_delta);
  void set_default_qmi_nas_config_sig_info2_req_msg_v01_wcdma_rssi_delta(uint16_t wcdma_rssi_delta);
  void set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_ecio_delta(uint16_t tdscdma_ecio_delta);
  void set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_rscp_delta(uint16_t tdscdma_rscp_delta);
  void set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_rssi_delta(uint16_t tdscdma_rssi_delta);
  void set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_sinr_delta(uint16_t tdscdma_sinr_delta);
  int16_t* set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_snr_threshold_list(uint32_t lte_snr_threshold_list_len);
  void set_default_qmi_nas_config_sig_info2_req_msg_v01_nr5g_rsrp_delta(uint16_t nr5g_rsrp_delta);
  void set_default_qmi_nas_config_sig_info2_req_msg_v01_nr5g_snr_delta(uint16_t nr5g_snr_delta);
  void set_default_qmi_nas_config_sig_info2_req_msg_v01_nr5g_rpt_rate_avg_period(
    nas_nr5g_common_rssi_reporting_rate_enum_type_v01 nr5g_rpt_rate,
    nas_nr5g_common_rssi_averaging_period_enum_type_v01 nr5g_avg_period);

  /*
    QMI_NAS_CONFIG_SIG_INFO2_RESP_MSG_V01
  */
  void update_default_qmi_nas_config_sig_info2_resp_msg_v01(void);
  void set_qmi_nas_config_sig_info2_resp_msg_result(qmi_result_type_v01 result, qmi_error_type_v01 error);

  /*
    QMI_NAS_GET_ERR_RATE_REQ_MSG_V01
  */
  void update_default_qmi_nas_get_err_rate_req_msg_v01(void);

  /*
    QMI_NAS_GET_ERR_RATE_RESP_MSG_V01
  */
  void update_default_qmi_nas_get_err_rate_resp_msg_v01(void);
  void set_qmi_nas_get_err_rate_resp_msg_v01_result(qmi_result_type_v01 result, qmi_error_type_v01 error);

#ifndef RIL_UTF_IOE
  /*
    QMI_NAS_GET_EMBMS_SIG_EXT_REQ_MSG_V01
  */
  void update_default_qmi_nas_get_embms_sig_ext_req(void);
  void set_qmi_nas_get_embms_sig_ext_req_trace_id(int16_t trace_id);

  /*
    QMI_NAS_GET_EMBMS_SIG_EXT_RESP_MSG_V01
  */
  void update_default_qmi_nas_get_embms_sig_ext_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);
  void set_qmi_nas_get_embms_sig_ext_resp_trace_id(int16_t);
  nas_lte_embms_signal_type_v01 *set_qmi_nas_get_embms_sig_ext_resp_snr_and_tmgi_list(int len);
  /*
    QMI_NAS_GET_EMBMS_STATUS_REQ_MSG_V01
  */
  void update_default_qmi_nas_get_embms_status_req_msg_v01(void);

  /*
    QMI_NAS_GET_EMBMS_STATUS_RESP_MSG_V01
  */
  void update_default_qmi_nas_get_embms_status_resp_msg_v01(void);
  void set_qmi_nas_get_embms_status_resp_result(qmi_result_type_v01 result, qmi_error_type_v01 error);
#endif
  /*
    QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01
  */
  void update_default_qmi_nas_indication_register_req_msg_v01(void);
  void set_qmi_nas_indication_register_req_msg_reg_sys_sel_pref(uint8_t reg_sys_sel_pref);
  void set_qmi_nas_indication_register_req_msg_reg_sys_info_ind(uint8_t reg_sys_info_ind);
  void set_qmi_nas_indication_register_req_msg_dual_standby_pref(uint8_t dual_standby_pref);
  void set_qmi_nas_indication_register_req_msg_subscription_info(uint8_t subscription_info);
  void set_qmi_nas_indication_register_req_msg_reg_network_time(uint8_t reg_network_time);
  void set_qmi_nas_indication_register_reg_msg_reg_rtre_cfg(uint8_t reg_rtre_cfg);
  void set_qmi_nas_indication_register_reg_msg_reg_embms_status(uint8_t reg_embms_status);
  void set_qmi_nas_indication_register_req_msg_req_serving_system(uint8_t req_serving_system);
  void set_qmi_nas_indication_register_req_msg_reg_managed_roaming(uint8_t reg_managed_roaming);
  void set_qmi_nas_indication_register_req_msg_sig_info(uint8_t sig_info);
  void set_qmi_nas_indication_register_reg_msg_err_rate(uint8_t err_rate);
  void set_qmi_nas_indication_register_req_msg_reg_current_plmn_name(uint8_t reg_current_plmn_name);
  void set_qmi_nas_indication_register_req_msg_reg_rf_band_info(uint8_t reg_rf_band_info);
  void set_qmi_nas_indication_register_req_msg_network_reject(uint8_t reg_network_reject, uint8_t suppress_sys_info);
  /*
    QMI_NAS_INDICATION_REGISTER_RESP_MSG_V01
  */
  void update_default_qmi_nas_indication_register_resp_msg_v01(void);
  void set_qmi_nas_nas_indication_register_resp_msg_v01_result(qmi_result_type_v01 result, qmi_error_type_v01 error);

  /*
    QMI_NAS_GET_CENTRALIZED_EONS_SUPPORT_STATUS_REQ_MSG_V01
  */
  void update_default_qmi_nas_get_centralized_eons_support_status_req_msg_v01(void);

  /*
  QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_REQ_MSG_V01
  */
  void update_default_qmi_nas_config_plmn_name_ind_reporting_req_msg_v01(void);
  void set_qmi_nas_config_plmn_name_ind_reporting_req_msg_send_all_information(uint8_t send_all_information);

  /*
  QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_RESP_MSG_V01
  */
  void update_default_qmi_nas_config_plmn_name_ind_reporting_resp_msg_v01(void);
  void set_qmi_nas_config_plmn_name_ind_reporting_resp_msg_v01_result(qmi_result_type_v01 result, qmi_error_type_v01 error);

  /*
    QMI_NAS_GET_CENTRALIZED_EONS_SUPPORT_STATUS_RESP_MSG_V01
  */
  void update_default_qmi_nas_get_centralized_eons_support_status_resp_msg_v01(void);
  void set_qmi_nas_get_centralized_eons_support_status_resp_msg_centralized_eons_supported(uint8_t centralized_eons_supported);
  void set_qmi_nas_get_centralized_eons_support_status_resp_msg_result(qmi_result_type_v01 result, qmi_error_type_v01 error);

  /*
    QMI_NAS_GET_SERVING_SYSTEM_REQ_MSG_V01
  */
  void update_default_qmi_nas_get_serving_system_req_msg_v01(void);

  /*
    QMI_NAS_GET_SERVING_SYSTEM_RESP_MSG_V01
  */
  void update_default_qmi_nas_get_serving_system_resp_msg_v01(void);
  void set_qmi_nas_get_serving_system_resp_msg_serving_system(nas_serving_system_type_v01 serving_system);
  void set_qmi_nas_get_serving_system_resp_msg_result(qmi_result_type_v01 result, qmi_error_type_v01 error);

  /*
    QMI_NAS_GET_SUB_BLOCKED_STATUS_REQ_MSG_V01
  */
  void update_default_qmi_nas_get_sub_blocked_status_req_msg_v01(void);

  /*
    QMI_NAS_GET_SUB_BLOCKED_STATUS_RESP_MSG_V01
  */
  nas_get_sub_blocked_status_resp_msg_v01* update_default_qmi_nas_get_sub_blocked_status_resp_msg_v01(void);
  void set_qmi_nas_get_sub_blocked_status_resp_msg_v01_result(qmi_result_type_v01 result, qmi_error_type_v01 error);
  void set_qmi_nas_get_sub_blocked_status_resp_msg_is_blocked(uint8_t is_blocked_valid, uint8_t is_blocked);

  /*
    QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
  */
  void update_default_qmi_nas_get_sys_info_req_msg_v01(void);

  /*
    QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
  */
  nas_get_sys_info_resp_msg_v01* update_default_qmi_nas_get_sys_info_resp_msg_v01(void);
  void set_qmi_nas_get_sys_info_resp_msg_v01_result(qmi_result_type_v01 result, qmi_error_type_v01 error);
  void set_qmi_nas_get_sys_info_resp_msg_lte_attached(nas_service_domain_enum_type_v01 srv_domain);

  /*
    QMI_NAS_GET_SIG_INFO_REQ_MSG_V01
  */
  void update_default_qmi_nas_get_sig_info_req_msg_v01(void);

  /*
    QMI_NAS_GET_SIG_INFO_RESP_MSG_V01
  */
  void update_default_qmi_nas_get_sig_info_resp_msg_v01(void);
  void set_qmi_nas_get_sig_info_resp_msg_v01_result(qmi_result_type_v01 result, qmi_error_type_v01 error);

  /*
    QMI_PBM_INDICATION_REGISTER_REQ_V01
  */
  void update_default_qmi_pbm_indication_register_req_msg(void);
  void set_qmi_pbm_indication_register_mask(pbm_reg_mask_type_v01 reg_mask);

  /*
    QMI_PBM_INDICATION_REGISTER_RESP_V01
  */
  void update_default_qmi_pbm_indication_register_resp_msg(void);
  void set_qmi_pbm_indication_register_resp_v01_reg_mask(pbm_reg_mask_type_v01 reg_mask);
  void set_qmi_pbm_indication_register_resp_v01_result(qmi_result_type_v01 result, qmi_error_type_v01 error);

  /*
    QMI_PBM_GET_CONFIGURATION_REQ_V01
  */
  void update_default_qmi_pbm_get_configuration_req_msg(void);
  void set_qmi_pbm_get_configuration_pbm_feature_status_mask(phonebook_feature_status_mask_type_v01 pbm_feature_status_mask);

  /*
    QMI_PBM_GET_CONFIGURATION_RESP_V01
  */
  void update_default_qmi_pbm_get_configuration_resp_msg(void);
  void set_qmi_pbm_get_configuration_resp_centralized_ecc(uint8_t centralized_ecc_valid, uint8_t centralized_ecc);
  void set_qmi_pbm_get_configuration_resp_result(qmi_result_type_v01 result, qmi_error_type_v01 error);

  /*
    QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ_V01
  */
  void update_default_qmi_wms_get_transport_layer_info_req_v01(void);

  /*
    QMI_WMS_GET_TRANSPORT_LAYER_INFO_RESP_V01
  */
  void update_default_qmi_wms_get_transport_layer_info_resp_v01(void);
  void set_qmi_wms_get_transport_layer_info_resp_result(qmi_result_type_v01 result, qmi_error_type_v01 error);

  /*
  QMI_NAS_SET_LTE_BAND_PRIORITY_REQ_MSG_V01
  */
  void update_default_qmi_nas_set_lte_band_priority_req_msg_v01(void);

//---------------------------------------------------------------------------------------------------------------------

/*
  QMI_VOICE_SET_CONFIG_REQ_V02
 */

  void update_default_qmi_voice_set_config_req_msg(void);
  void set_qmi_voice_set_config_req_tty_mode(tty_mode_enum_v02 val);
  void set_qmi_voice_set_config_req_ui_tty_setting(tty_mode_enum_v02 tty_setting);

/*
  QMI_VOICE_SET_CONFIG_RESP_V02

*/
  void update_default_qmi_voice_set_config_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);
/*
  QMI_VOICE_GET_CONFIG_REQ_V02
*/
  void update_default_qmi_voice_get_config_req_msg(void);
  void set_qmi_voice_get_config_req_voice_privacy(uint8_t val);
  void set_qmi_voice_get_config_req_auto_answer(uint8_t val);
  void set_qmi_voice_get_config_req_roam_timer(uint8_t val);
  void set_qmi_voice_get_config_req_pref_voice_so(uint8_t val);
  void set_qmi_voice_get_config_req_tty_mode(uint8_t val);

/*
  QMI_VOICE_GET_CONFIG_RESP_V02
*/
  void update_default_qmi_voice_get_config_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);
  void set_qmi_voice_get_config_resp_current_tty_mode(tty_mode_enum_v02 val);
  void set_qmi_voice_get_config_resp_current_voice_privacy_pref(voice_privacy_enum_v02 val);

/*
  QMI_VOICE_DIAL_CALL_REQ_V02
  structure : voice_dial_call_req_msg_v02
*/
  voice_dial_call_req_msg_v02 *update_default_qmi_voice_dial_call_req_msg(char *address);
  void set_qmi_voice_dial_call_req_call_type(call_type_enum_v02 val);
  void set_qmi_voice_dial_call_req_clir_type(clir_type_enum_v02 val);
  voice_uus_type_v02* set_qmi_voice_dial_call_req_uus();
  char* set_qmi_voice_dial_call_req_sip_uri_overflow(char *val);
  void set_qmi_voice_dial_call_req_audio_attrib(uint64_t val);
  void set_qmi_voice_dial_call_req_video_attrib(uint64_t val);
  void set_qmi_voice_dial_call_req_pi(ip_pi_enum_v02 val);
  void set_qmi_voice_dial_call_req_service_type(voice_dial_call_service_type_enum_v02 val);
  void set_qmi_voice_dial_call_req_conf_uri_list(char *val);
  void set_qmi_voice_dial_call_req_called_party_subaddress_extension_bit(uint8_t val);
  void set_qmi_voice_dial_call_req_called_party_subaddress_subaddress_type(subaddress_type_enum_v02 val);
  void set_qmi_voice_dial_call_req_called_party_subaddress_odd_even_ind(uint8_t val);
  void set_qmi_voice_dial_call_req_called_party_subaddress_subaddress(char *val);
  void set_qmi_voice_dial_call_req_is_secure_call(uint8_t is_secure_call);
  uint16_t *set_qmi_voice_dial_call_req_display_text(uint32_t display_text_len);
  void set_qmi_voice_dial_call_req_emer_cat(uint8_t emer_cat);
  void set_qmi_voice_dial_call_req_origination_number(char *val);
  void set_qmi_voice_dial_call_req_is_secondary(uint8_t val);
  void set_qmi_voice_dial_call_req_call_pull(uint8_t val);
  void set_qmi_voice_dial_call_req_call_failure_reason(call_end_reason_enum_v02 val);
  void set_qmi_voice_dial_call_req_call_failure_mode(call_mode_enum_v02 val);

/*
  QMI_VOICE_DIAL_CALL_RESP_V02
  structure : voice_dial_call_resp_msg_v02
*/
  void update_default_qmi_voice_dial_call_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);
  void set_qmi_voice_dial_call_resp_call_id (uint8_t call_id);
  void set_qmi_voice_dial_call_resp_call_type(int val);
  voice_alpha_ident_type_v02* set_qmi_voice_dial_call_resp_alpha_ident();
  void set_qmi_voice_dial_call_cc_result_type(voice_cc_result_type_enum_v02 val);
  voice_cc_sups_result_type_v02* set_qmi_voice_dial_call_resp_cc_sups_result();
  void set_qmi_voice_dial_call_resp_media_id(uint8_t media_id);
  void set_qmi_voice_dial_call_resp_end_reason(call_end_reason_enum_v02  val);

/*
  QMI_VOICE_ANSWER_CALL_REQ_V02
  structure : voice_answer_call_req_msg_v02
*/

  void update_default_qmi_voice_answer_call_req_msg(uint8_t call_id);
  void set_qmi_voice_answer_call_req_call_type(call_type_enum_v02 val);
  void set_qmi_voice_answer_call_req_audio_attrib(uint64_t val);
  void set_qmi_voice_answer_call_req_video_attrib(uint64_t val);
  void set_qmi_voice_answer_call_req_pi(ip_pi_enum_v02 val);
  void set_qmi_voice_answer_call_req_reject_call(uint8_t val);
  void set_qmi_voice_answer_call_req_reject_cause(voice_reject_cause_enum_v02 val);

/*
  QMI_VOICE_ANSWER_CALL_RESP_V02
  structure : voice_answer_call_resp_msg_v02
*/
  void update_default_qmi_voice_answer_call_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_VOICE_END_CALL_REQ_V02
  structure : voice_end_call_req_msg_v02
*/

  void update_default_qmi_voice_end_call_req_msg(uint8_t call_id_val);
  void set_qmi_voice_end_call_req_end_cause(voice_reject_cause_enum_v02 end_cause);

/*
  QMI_VOICE_END_CALL_RESP_V02
  structure : voice_end_call_resp_msg_v02
*/
  void update_default_qmi_voice_end_call_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);
  void set_qmi_voice_end_call_resp_call_id(int call_id);

/*
  QMI_VOICE_MANAGE_CALLS_REQ_V02
  structure : voice_manage_calls_req_msg_v02
*/
void update_default_qmi_voice_manage_calls_req_v02(sups_type_enum_v02 sups_type);
void set_qmi_voice_manage_calls_req_v02_call_id(uint8_t call_id);

/*
  QMI_VOICE_MANAGE_CALLS_RESP_V02
  structure : voice_manage_calls_resp_msg_v02
*/
void update_default_qmi_voice_manage_calls_resp_v02(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_VOICE_MANAGE_IP_CALLS_REQ_V02
  structure : voice_manage_ip_calls_req_msg_v02
*/

  void update_default_qmi_voice_manage_ip_calls_req_msg(voip_sups_type_enum_v02 sups_type_val);
  void set_qmi_voice_manage_ip_calls_req_call_id(uint8_t val);
  void set_qmi_voice_manage_ip_calls_req_call_type(call_type_enum_v02 val);
  void set_qmi_voice_manage_ip_calls_req_audio_attrib(uint64_t val);
  void set_qmi_voice_manage_ip_calls_req_video_attrib(uint64_t val);
  void set_qmi_voice_manage_ip_calls_req_rtt_mode(rtt_mode_type_v02 val);
  void set_qmi_voice_manage_ip_calls_req_ect_type(voip_ect_type_enum_v02 val);
  void set_qmi_voice_manage_ip_calls_req_transfer_target_call_id(uint8_t val);
  char *set_qmi_voice_manage_ip_calls_sip_uri();
/*
  QMI_VOICE_MANAGE_IP_CALLS_RESP_V02
  structure : voice_manage_ip_calls_resp_msg_v02
*/
  void update_default_qmi_voice_manage_ip_calls_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);
  void set_qmi_voice_manage_ip_calls_failure_cause(qmi_sups_errors_enum_v02 val);
  void set_qmi_voice_manage_ip_calls_call_modified_cause(voice_call_modified_cause_enum_v02 val);
  void set_qmi_voice_manage_ip_calls_call_id(uint8_t val);
  void set_qmi_voice_manage_ip_calls_sip_error_code(uint32_t sip_error_code);
  voice_ip_end_reason_text_type_v02* set_qmi_voice_manage_ip_calls_end_reason_text(
                                                   uint32_t end_reason_text_len);

/*
  QMI_VOICE_IMS_CALL_CANCEL_REQ_V02
  structure : voice_ims_call_cancel_req_msg_v02
*/
  void update_default_qmi_voice_ims_call_cancel_req_msg(uint8_t call_id, voip_cancel_type_enum_v02 cancel_type);

/*
  QMI_VOICE_IMS_CALL_CANCEL_RESP_V02
  structure : voice_ims_call_cancel_resp_msg_v02
*/
  void update_default_qmi_voice_ims_call_cancel_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);
  void set_qmi_voice_ims_call_cancel_resp(uint8_t call_i, voip_cancel_type_enum_v02 cancel_type);

/*
  QMI_VOICE_START_CONT_DTMF_REQ_V02
  structure : voice_start_cont_dtmf_req_msg_v02
*/
  void update_default_qmi_voice_start_cont_dtmf_req_msg(uint8_t call_id_val, uint8_t digit_val);

/*
  QMI_VOICE_START_CONT_DTMF_RESP_V02
  structure : voice_start_cont_dtmf_resp_msg_v02
*/
  void update_default_qmi_voice_start_cont_dtmf_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_VOICE_STOP_CONT_DTMF_REQ_V02
  structure : voice_stop_cont_dtmf_req_msg_v02
*/
  void update_default_qmi_voice_stop_cont_dtmf_req_msg(uint8_t call_id_val);


/*
  QMI_VOICE_STOP_CONT_DTMF_RESP_V02
  structure : voice_stop_cont_dtmf_resp_msg_v02
*/
  void update_default_qmi_voice_stop_cont_dtmf_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);
/*
  QMI_VOICE_SEND_FLASH_REQ_V02
  structure : voice_send_flash_req_msg_v02
*/
  void update_default_qmi_voice_send_flash_req_msg(uint8_t call_id_val);
  void set_qmi_voice_send_flash_req_flash_payload(char *flash_payload);
  void set_qmi_voice_send_flash_req_flash_type(voice_send_flash_type_enum_v02 val);

/*
  QMI_VOICE_SEND_FLASH_RESP_V02
  structure : voice_send_flash_resp_msg_v02
*/
  void update_default_qmi_voice_send_flash_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_VOICE_MANAGE_CALLS_REQ_V02
  structure : voice_manage_calls_req_msg_v02
*/
  void update_default_voice_manage_calls_req_msg(sups_type_enum_v02 sups_type);

/*
  QMI_VOICE_MANAGE_CALLS_RESP_V02
  structure : voice_manage_calls_resp_msg_v02
*/
  void update_default_voice_manage_calls_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_VOICE_GET_SUPPORTED_MSGS_REQ_V02
*/
  void update_default_qmi_voice_get_supported_msgs_req_msg(void);

/*
 QMI_VOICE_GET_SUPPORTED_MSGS_RESP_V02
*/
  void update_default_qmi_voice_get_supported_msgs_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_VOICE_INDICATION_REGISTER_REQ
*/
  void update_default_qmi_voice_ind_reg_req_msg(void);
  void set_qmi_voice_ind_req_reg_voice_privacy_events(uint8_t val);
  void set_qmi_voice_ind_req_ext_brst_intl_events(uint8_t val);
  void set_qmi_voice_ind_req_speech_events(uint8_t val);
  void set_qmi_voice_ind_req_handover_events(uint8_t val);
  void set_qmi_voice_ind_req_conference_events(uint8_t val);
  void set_qmi_voice_ind_req_supps_notification_events(uint8_t val);
  void set_qmi_voice_ind_req_tty_info_events (uint8_t tty_info_events);
  void set_qmi_voice_ind_req_cc_result_events(uint8_t cc_result_events);
  void set_qmi_voice_ind_req_reg_dtmf_events (uint8_t reg_dtmf_events);
  void set_qmi_voice_ind_req_additional_call_info_events (uint8_t additional_call_info_events);
  void set_qmi_voice_ind_req_audio_rat_change_events(uint8_t audio_rat_change_events);
  void set_qmi_voice_ind_req_vice_dialog_event(uint8_t vice_dialog_event);
  void set_qmi_voice_ind_req_conf_participants_events(uint8_t conf_participants_events);

/*
  QMI_VOICE_INDICATION_REGISTER_RESP
*/
  //void update_default_qmi_voice_ind_reg_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);     This is the correct one.. has to be replaced with the below one, once the test cases are re framed..
  void update_default_qmi_voice_ind_reg_resp_msg(void);
  void set_qmi_voice_ind_register_resp_msg
  (qmi_result_type_v01 result, qmi_error_type_v01 error);
#ifndef UTF_TARGET_BUILD
/*
  QMI_VOICE_SET_SUPS_SERVICE_REQ_V02
  structure : voice_set_sups_service_req_msg_v02
*/
  void update_default_qmi_voice_set_sups_service_req_msg(voice_service_enum_v02 voice_service, voice_reason_enum_v02 reason);
  void set_qmi_voice_set_sups_service_req_service_class(uint8_t val);
  void set_qmi_voice_set_sups_service_req_password(const char *p);
  char* set_qmi_voice_set_sups_service_req_number();
  void set_qmi_voice_set_sups_service_req_timer_value(uint8_t val);
  void set_qmi_voice_set_sups_service_req_colr_pi(pi_colr_enum_v02 val);
  voice_barred_number_type_v02 *set_qmi_voice_set_sups_service_req_call_barring_numbers_list(uint32_t len);
  void set_qmi_voice_set_sups_service_req_call_fwd_start_time(
          uint16_t year, uint8_t month, uint8_t day, uint8_t hour,
          uint8_t minute, uint8_t second, int8_t time_zone);
  void set_qmi_voice_set_sups_service_req_call_fwd_end_time(
          uint16_t year, uint8_t month, uint8_t day, uint8_t hour,
          uint8_t minute, uint8_t second, int8_t time_zone);
#endif

/*
  QMI_VOICE_SET_SUPS_SERVICE_RSEP_V02
  structure : voice_set_sups_service_resp_msg_v02
*/
  void update_default_qmi_voice_set_sups_service_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);
  void set_qmi_voice_set_sups_service_cc_result_type(voice_cc_result_type_enum_v02 val);
  void set_qmi_voice_set_sups_service_cc_sups_result(voice_cc_sups_result_service_type_enum_v02 service_type, voice_cc_sups_result_reason_enum_v02 reason);
  void set_qmi_voice_set_sups_service_call_id(uint8_t  val);
  voice_alpha_ident_type_v02 *set_qmi_voice_set_sups_service_alpha_ident();
  void set_qmi_voice_set_sups_service_sip_error_code(uint32_t code);
  void set_qmi_voice_set_sups_service_resp_failure_cause(qmi_sups_errors_enum_v02 val);
  void set_qmi_voice_set_sups_service_failure_cause_description(uint32_t len, uint16_t val[]);

/*
  QMI_VOICE_SET_ALL_CALL_FWD_SUPS_REQ_V02
*/
  void update_default_qmi_voice_set_all_call_fwd_sups_req_msg(voice_service_enum_v02 voice_service, voice_reason_enum_v02 reason);
  void set_qmi_voice_set_all_call_fwd_sups_req_service_class(voice_service_class_enum_v02 val);
  void set_qmi_voice_set_all_call_fwd_sups_req_number(char *number);
  void set_qmi_voice_set_all_call_fwd_sups_req_timer_value(uint8_t val);
  void set_qmi_voice_set_all_call_fwd_sups_req_call_fwd_start_time(
          uint16_t year, uint8_t month, uint8_t day, uint8_t hour,
          uint8_t minute, uint8_t second, int8_t time_zone);
  void set_qmi_voice_set_all_call_fwd_sups_req_call_fwd_end_time(
          uint16_t year, uint8_t month, uint8_t day, uint8_t hour,
          uint8_t minute, uint8_t second, int8_t time_zone);

/*
  QMI_VOICE_SET_ALL_CALL_FWD_SUPS_RESP_V02
*/
  void update_default_qmi_voice_set_all_call_fwd_sups_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);
  void set_qmi_voice_set_all_call_fwd_sups_resp_failure_cause(qmi_sups_errors_enum_v02 val);
  void set_qmi_voice_set_all_call_fwd_sups_resp_failure_cause_description(uint16_t failure_cause_description[QMI_VOICE_FAILURE_CAUSE_DESC_MAX_LEN_V02+1]);
  void set_qmi_voice_set_all_call_fwd_sups_resp_sip_error_code(uint32_t code);
  void set_qmi_voice_set_all_call_fwd_sups_resp_cfu_result(
      uint8_t cfu_result_valid, qmi_result_type_v01 cfu_result,
      uint8_t cfu_failure_cause_valid, qmi_sups_errors_enum_v02 cfu_failure_cause,
      uint8_t cfu_failure_cause_description_valid, uint16_t cfu_failure_cause_description[QMI_VOICE_FAILURE_CAUSE_DESC_MAX_LEN_V02+1],
      uint8_t cfu_retry_duration_valid, uint16_t cfu_retry_duration,
      uint8_t cfu_sip_error_code_valid, uint16_t cfu_sip_error_code);
  void set_qmi_voice_set_all_call_fwd_sups_resp_cfb_result(
      uint8_t cfb_result_valid, qmi_result_type_v01 cfb_result,
      uint8_t cfb_failure_cause_valid, qmi_sups_errors_enum_v02 cfb_failure_cause,
      uint8_t cfb_failure_cause_description_valid, uint16_t cfb_failure_cause_description[QMI_VOICE_FAILURE_CAUSE_DESC_MAX_LEN_V02+1],
      uint8_t cfb_retry_duration_valid, uint16_t cfb_retry_duration,
      uint8_t cfb_sip_error_code_valid, uint16_t cfb_sip_error_code);
  void set_qmi_voice_set_all_call_fwd_sups_resp_cfnry_result(
      uint8_t cfnry_result_valid, qmi_result_type_v01 cfnry_result,
      uint8_t cfnry_failure_cause_valid, qmi_sups_errors_enum_v02 cfnry_failure_cause,
      uint8_t cfnry_failure_cause_description_valid, uint16_t cfnry_failure_cause_description[QMI_VOICE_FAILURE_CAUSE_DESC_MAX_LEN_V02+1],
      uint8_t cfnry_retry_duration_valid, uint16_t cfnry_retry_duration,
      uint8_t cfnry_sip_error_code_valid, uint16_t cfnry_sip_error_code);
  void set_qmi_voice_set_all_call_fwd_sups_resp_cfnrc_result(
      uint8_t cfnrc_result_valid, qmi_result_type_v01 cfnrc_result,
      uint8_t cfnrc_failure_cause_valid, qmi_sups_errors_enum_v02 cfnrc_failure_cause,
      uint8_t cfnrc_failure_cause_description_valid, uint16_t cfnrc_failure_cause_description[QMI_VOICE_FAILURE_CAUSE_DESC_MAX_LEN_V02+1],
      uint8_t cfnrc_retry_duration_valid, uint16_t cfnrc_retry_duration,
      uint8_t cfnrc_sip_error_code_valid, uint16_t cfnrc_sip_error_code);
  voice_alpha_ident_type_v02 *set_qmi_voice_set_all_call_fwd_sups_resp_alpha_ident();
  void set_qmi_voice_set_all_call_fwd_sups_resp_cc_result_type(voice_cc_result_type_enum_v02 val);
  void set_qmi_voice_set_all_call_fwd_sups_resp_call_id(uint8_t  val);
  void set_qmi_voice_set_all_call_fwd_sups_resp_cc_sups_result(voice_cc_sups_result_service_type_enum_v02 service_type, voice_cc_sups_result_reason_enum_v02 reason);

/*
  QMI_VOICE_GET_CLIP_REQ_V02
*/
  void update_default_qmi_voice_get_clip_req_v02();
/*
  QMI_VOICE_GET_CLIP_RESP_V02
*/
  void update_default_qmi_voice_get_clip_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);
  void set_qmi_voice_get_clip_resp_msg_clip_response (active_status_enum_v02 active_status, provision_status_enum_v02 provision_status);
  void set_qmi_voice_get_clip_resp_msg_sip_error_code(uint16_t val);
  void set_qmi_voice_get_clip_resp_msg_failure_cause_description(uint32_t len, uint16_t val[]);
  void set_qmi_voice_get_clip_resp_msg_cc_result_type(voice_cc_result_type_enum_v02  val);
  void set_qmi_voice_get_clip_resp_msg_cc_sups_result(voice_cc_sups_result_service_type_enum_v02 service_type, voice_cc_sups_result_reason_enum_v02 reason);

  /*
    QMI_VOICE_GET_COLP_REQ_V02
  */
  void update_default_qmi_voice_get_colp_req_v02(void);

  /*
    QMI_VOICE_GET_COLP_RESP_V02
  */
  void update_default_qmi_voice_get_colp_resp_msg(qmi_result_type_v01 result,
                                                  qmi_error_type_v01 error);
  void set_qmi_voice_get_colp_resp_msg_colp_response(active_status_enum_v02 active_status,
                                                     provision_status_enum_v02 provision_status);
  void set_qmi_voice_get_colp_resp_failure_cause_description(uint32_t len, uint16_t val[]);
  void set_qmi_voice_get_colp_resp_sip_error_code(uint16_t val);

  /*
    QMI_VOICE_GET_CLIR_REQ_V02
  */
  void update_default_qmi_voice_get_clir_req_msg();

/*
  QMI_VOICE_GET_CLIR_RESP_V02
*/
  void update_default_qmi_voice_get_clir_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);
  void set_qmi_void_get_clir_resp_clir_response(active_status_enum_v02 active_status,
                                                provision_status_enum_v02 provision_status);
  void set_qmi_void_get_clir_resp_failure_cause (qmi_sups_errors_enum_v02 failure_cause);

/*
  QMI_VOICE_GET_CALL_WAITING_REQ_V02
*/
  void update_default_qmi_voice_get_call_waiting_req_msg(void);
  void set_qmi_voice_get_call_waiting_req_service_class(uint8_t val);

/*
  QMI_VOICE_GET_CALL_WAITING_RESP_V02
*/
  void update_default_qmi_voice_get_call_waiting_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);
  void set_qmi_voice_get_call_waiting_resp_service_class(uint8_t val);
  void set_qmi_voice_get_call_waiting_resp_sip_error_code(uint16_t val);
  void set_qmi_voice_get_call_waiting_resp_failure_cause_description(uint32_t len, uint16_t val[]);

/*
  QMI_VOICE_BURST_DTMF_REQ_V02
*/
  voice_burst_dtmf_info_type_v02 *update_default_qmi_voice_burst_dtmf_req_msg_msg();
  void set_qmi_voice_burst_dtmf_req_dtmf_lengths(dtmf_onlength_enum_v02 on, dtmf_offlength_enum_v02 off);

/*
  QMI_VOICE_BURST_DTMF_RESP_V02
*/
  void update_default_qmi_voice_burst_dtmf_resp_msg_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_VOICE_SET_PREFERRED_PRIVACY_REQ_V02
*/
  void update_default_qmi_voice_set_preferred_privacy_req_msg();
  void set_qmi_voice_set_preferred_privacy_req_privacy_pref(voice_privacy_enum_v02 val);

/*
  QMI_VOICE_SET_PREFERRED_PRIVACY_RESP_V02
*/
  void update_default_qmi_voice_set_preferred_privacy_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_VOICE_SET_CALL_BARRING_PASSWORD_REQ_V02
*/
  voice_call_barring_password_info_type_v02* update_default_qmi_voice_set_call_barring_password_req_msg();

/*
  QMI_VOICE_SET_CALL_BARRING_PASSWORD_RESP_V02
*/
  void update_default_qmi_voice_set_call_barring_password_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);
  void set_qmi_voice_set_call_barring_password_resp_failure_cause(qmi_sups_errors_enum_v02 val);

/*
  QMI_VOICE_GET_CALL_BARRING_REQ_V02
*/
  void update_default_qmi_voice_get_call_barring_req_msg(voice_reason_enum_v02 reason);
  void set_voice_get_call_barring_req_service_class(uint8_t val);
/*
  QMI_VOICE_GET_CALL_BARRING_RESP_V02
*/
  void update_default_qmi_voice_get_call_barring_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);
  void set_qmi_voice_get_call_barring_resp_msg_service_class(uint8_t service_class);
  void set_qmi_voice_get_call_barring_resp_msg_SS_service_type(voice_cc_sups_result_reason_enum_v02 reason);
  void set_qmi_voice_get_call_barring_resp_msg_sip_error_code(uint16_t val);
  void set_qmi_voice_get_call_barring_resp_msg_failure_cause_description(uint32_t len, uint16_t val[]);
  voice_serviceclass_barred_number_list_type_v02 *
  set_qmi_voice_get_call_barring_resp_msg_sc_barred_numbers_status_list(
      uint32_t len);

  /*
    QMI_VOICE_GET_CALL_FORWARDING_REQ_V02
  */
  void update_default_qmi_voice_get_call_forwarding_req_msg(voice_reason_enum_v02 reason);
  void set_voice_get_call_forwarding_req_service_class(uint8_t val);
/*
  QMI_VOICE_GET_CALL_FORWARDING_RESP_V02
*/
  void update_default_qmi_voice_get_call_forwarding_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);
  voice_get_call_forwarding_info_type_v02 *set_qmi_voice_get_call_forwarding_resp_get_call_forwarding_info (uint32_t get_call_forwarding_info_len);
  void set_qmi_voice_get_call_forwarding_resp_call_fwd_start_time(
          uint16_t year, uint8_t month, uint8_t day, uint8_t hour,
          uint8_t minute, uint8_t second, int8_t time_zone);
  void set_qmi_voice_get_call_forwarding_resp_call_fwd_end_time(
          uint16_t year, uint8_t month, uint8_t day, uint8_t hour,
          uint8_t minute, uint8_t second, int8_t time_zone);
  void set_qmi_voice_get_call_forwarding_resp_sip_error_code(uint16_t sip_error_code);
  void set_qmi_voice_get_call_forwarding_resp_failure_cause_description(uint32_t len, uint16_t val[]);

/*
  QMI_VOICE_ORIG_USSD_REQ_V02
*/
  voice_uss_info_type_v02* update_default_qmi_voice_orig_ussd_req_msg();


/*
  QMI_VOICE_ORIG_USSD_RESP_V02
*/
  void update_default_qmi_voice_orig_ussd_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);
  void set_qmi_voice_orig_ussd_resp_alpha_id(
          alpha_dcs_enum_v02 alpha_dcs, uint32_t alpha_text_len, uint8_t *alpha_text);
  void set_qmi_voice_orig_ussd_resp_cc_result_type(voice_cc_result_type_enum_v02  val);
  void set_qmi_voice_orig_ussd_resp_cc_sups_result(voice_cc_sups_result_service_type_enum_v02 service_type, voice_cc_sups_result_reason_enum_v02 reason);
  void set_qmi_voice_orig_ussd_resp_sip_error_code(uint16_t code);
  void set_qmi_voice_orig_ussd_resp_ussd_error_description(uint32_t len, uint16_t val[]);

/*
  QMI_VOICE_ANSWER_USSD_REQ_V02
*/
  voice_uss_info_type_v02* update_default_qmi_voice_answer_ussd_req_msg();

/*
  QMI_VOICE_ANSWER_USSD_RESP_V02
*/
  void update_default_qmi_voice_answer_ussd_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_VOICE_CANCEL_USSD_REQ_V02
*/
voice_cancel_ussd_req_msg_v02* update_default_qmi_voice_cancel_ussd_req_msg();


/*
  QMI_VOICE_CANCEL_USSD_RESP_V02
*/

  void update_default_qmi_voice_cancel_ussd_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_VOICE_BIND_SUBSCRIPTION_REQ_V02
*/

  void update_default_qmi_voice_bind_subscription_req_msg(voice_subs_type_enum_v02 subs_type);

/*
  QMI_VOICE_BIND_SUBSCRIPTION_RESP_V02
*/

  void update_default_qmi_voice_bind_subscription_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);

  /*
    QMI_VOICE_GET_ALL_CALL_INFO_REQ_V02
  */
  void update_default_qmi_voice_get_all_call_info_req(void);

  /*
    QMI_VOICE_GET_ALL_CALL_INFO_RESP_V02
  */
  void update_default_qmi_voice_get_all_call_info_resp(qmi_result_type_v01 result,
                                                  qmi_error_type_v01 error);
  voice_call_info2_type_v02 *set_qmi_voice_get_all_call_info_resp_call_info(uint32_t call_info_len);


/*
  QMI_VOICE_ALL_CALL_STATUS_IND_V02
*/
  voice_call_info2_type_v02* update_default_qmi_voice_all_call_status_ind_msg(uint32_t call_info_len);
  voice_remote_party_number2_type_v02* set_qmi_voice_all_call_status_ind_remote_party_number(uint32_t remote_party_number_len);
  voice_ip_num_id_type_v02* set_qmi_voice_all_call_status_ind_ip_num_info(uint32_t ip_num_info_len);
  voice_remote_party_name2_type_v02* set_qmi_voice_all_call_status_ind_remote_party_name(uint32_t remote_party_name_len);
  voice_conn_num_with_id_type_v02* set_qmi_voice_all_call_status_ind_conn_party_number(uint32_t conn_party_num_len);
  voice_conn_ip_num_with_id_type_v02* set_qmi_voice_all_call_status_ind_conn_party_extension(uint32_t conn_party_extension_len);
  voice_alerting_type_type_v02* set_qmi_voice_all_call_status_ind_alerting_type(uint32_t alerting_type_len);
  voice_srv_opt_type_v02* set_qmi_voice_all_call_status_ind_srv_opt(uint32_t srv_opt_len);
  voice_call_end_reason_type_v02* set_qmi_voice_all_call_status_ind_call_end_reason(uint32_t call_end_reason_len);
  voice_ip_end_sip_code_type_v02* set_qmi_voice_all_call_status_ind_sip_error_code(
                                                        uint32_t sip_error_code_len);
  voice_orig_fail_reason_with_id_type_v02* set_qmi_voice_all_call_status_ind_orig_fail_reason(uint32_t orig_fail_reason_len);
  voice_alpha_ident_with_id_type_v02* set_qmi_voice_all_call_status_ind_alpha_id(uint32_t alpha_id_len);
  voice_conn_num_with_id_type_v02* set_qmi_voice_all_call_status_ind_conn_party_num(uint32_t conn_party_num_len);
  voice_diagnostic_info_with_id_type_v02* set_qmi_voice_all_call_status_ind_diagnostic_info(uint32_t diagnostic_info_len);
  voice_num_with_id_type_v02* set_qmi_voice_all_call_status_ind_called_party_num(uint32_t called_party_num_len);
  voice_num_with_id_type_v02* set_qmi_voice_all_call_status_ind_redirecting_party_num(uint32_t redirecting_party_num_len);
  voice_call_attributes_type_v02* set_qmi_voice_all_call_status_ind_audio_attrib(uint32_t audio_attrib_len);
  voice_call_attributes_type_v02* set_qmi_voice_all_call_status_ind_video_attrib(uint32_t video_attrib_len);
  voice_call_attrib_status_type_v02* set_qmi_voice_all_call_status_ind_call_attrib_status(uint32_t call_attrib_len);
#ifndef UTF_TARGET_BUILD
  voice_is_add_info_present_with_id_type_v02* set_qmi_voice_all_call_status_ind_is_add_info_present(uint32_t is_add_info_present_len);
#endif
  voice_is_srvcc_call_with_id_type_v02* set_qmi_voice_all_call_status_ind_srvcc_attrib(uint32_t is_srvcc_len);
  voice_srvcc_parent_call_id_type_v02* set_qmi_voice_all_call_status_ind_srvcc_parent_call_info(uint32_t srvcc_parent_call_info_len);
  voice_display_text_info_type_v02* set_qmi_voice_all_call_status_ind_display_text(uint32_t display_text_len);
  voice_child_number_info_type_v02* set_qmi_voice_all_call_status_ind_child_number(uint32_t child_number_info_len);
  voice_ip_call_capabilities_info_type_v02* set_qmi_voice_all_call_status_ind_local_call_capabilities_info(uint32_t local_call_capabilities_info_len);
  voice_ip_call_capabilities_info_type_v02* set_qmi_voice_all_call_status_ind_peer_call_capabilities_info(uint32_t peer_call_capabilities_info_len);
  voice_ip_caller_name_info_type_v02 *set_qmi_voice_all_call_status_ind_ip_caller_name(uint32_t ip_caller_name_len);
  voice_media_id_with_id_type_v02 *set_qmi_voice_all_call_status_ind_media_id(uint32_t media_id_len);
  voice_ip_end_reason_text_type_v02* set_qmi_voice_all_call_status_ind_end_reason_text(uint32_t end_reason_text_len);
  voice_volte_is_called_party_ringing_type_v02* set_qmi_voice_all_call_status_ind_is_called_party_ringing(uint32_t is_called_party_ringing_len);
  voice_volte_is_secure_call_type_v02* set_qmi_voice_all_call_status_ind_is_secure_call(uint32_t is_secure_call_len);
  voice_remote_party_conf_cap_type_v02* set_qmi_voice_all_call_status_ind_remote_party_conf_capability(uint32_t remote_party_conf_capability_len);
  voice_call_rtt_mode_status_type_v02* set_qmi_voice_all_call_status_ind_rtt_mode(uint32_t rtt_mode_len);
  voice_emerg_srv_categ_type_v02* set_qmi_voice_all_call_status_ind_emerg_srv_categ(uint32_t emerg_srv_categ_len);
  voice_MT_call_verstat_with_id_type_v02* set_qmi_voice_all_call_status_ind_mt_call_verstat(uint32_t mt_call_verstat_len);
  voice_ip_hist_info_type_v02* set_qmi_voice_all_call_status_ind_ip_hist_info(uint32_t len);
  voice_sip_uri_with_id_type_v02* set_qmi_voice_all_call_status_ind_alternate_sip_uris(uint32_t len);
  voice_num_with_id_type_v02* set_qmi_voice_all_call_status_ind_terminating_num(uint32_t len);
  void set_qmi_voice_all_call_status_ind_is_secondary(uint8_t val);
  voice_tir_mode_type_v02* set_qmi_voice_all_call_status_ind_tir_mode(uint32_t tir_mode_len);
/*
  QMI_VOICE_HANDOVER_IND_V02
*/
  void update_qmi_voice_handover_ind_msg(voice_handover_state_enum_v02 ho_state, voice_handover_type_enum_v02 ho_type);

/*
  QMI_VOICE_TTY_IND_V02
*/
  void update_qmi_voice_tty_ind_msg(tty_mode_enum_v02 tty_mode);

/*
  QMI_VOICE_OTASP_STATUS_IND_V02
*/

  void update_default_qmi_voice_otasp_status_ind_msg(uint8_t call_id, otasp_status_enum_v02 otasp_status);

/*
  QMI_VOICE_PRIVACY_IND_V02
*/
  void update_default_qmi_voice_privacy_ind_msg(uint8_t call_id, voice_privacy_enum_v02 voice_privacy);

/*
  QMI_VOICE_SUPS_NOTIFICATION_IND_V02

*/
  void update_default_qmi_voice_sups_notification_ind_msg(uint8_t call_id, notification_type_enum_v02 notification_type);
  void set_qmi_voice_sups_notification_ind_index(uint16_t val);
  voice_ect_number_type_v02* set_qmi_voice_sups_notification_ind_ect_number();
  uint16_t *set_qmi_voice_sups_notification_ind_history_info(uint32_t hist_info_len);
  void set_qmi_voice_sups_notification_ind_media_direction_hold(voice_call_attribute_type_mask_v02 val);

/*
  QMI_VOICE_INFO_REC_IND_V02
*/
  void update_default_qmi_voice_info_rec_ind_msg(uint8_t call_id);
  void set_qmi_voice_info_rec_ind_call_waiting(call_waiting_enum_v02 call_waiting);
  voice_caller_id_info_type_v02* set_qmi_voice_info_rec_ind_caller_id_info();
  voice_num_info_type_v02* set_qmi_voice_info_rec_ind_calling_party_info();
  char* set_qmi_voice_info_rec_ind_display_buffer();
  voice_ext_display_info_type_v02* set_qmi_voice_info_rec_ind_ext_display_record();
  char* set_qmi_voice_info_rec_ind_ext_display_buffer();
  char* set_qmi_voice_info_rec_ind_caller_name();
  voice_nss_audio_control_info_type_v02* set_qmi_voice_info_rec_ind_audio_control();
  void set_qmi_voice_info_rec_ind_clir_cause(voice_nss_clir_cause_enum_v02 clir_cause);
  void set_qmi_voice_info_rec_ind_nss_release(voice_nss_release_enum_v02 nss_release);
  voice_redirecting_num_info_type_v02* set_qmi_voice_info_rec_ind_redirecting_num_info();
  voice_line_control_info_type_v02* set_qmi_voice_info_rec_ind_line_control();
  voice_num_info_type_v02* set_qmi_voice_info_rec_ind_conn_num_info();
  voice_signal_info_type_v02* set_qmi_voice_info_rec_ind_signal_info();

/*
  QMI_VOICE_USSD_IND_V02
*/
  void update_default_qmi_voice_ussd_ind_msg(further_user_action_enum_v02 notification_type);
  voice_uss_info_type_v02* set_qmi_voice_voice_uss_info();
  void set_qmi_voice_orig_ussd_ind_sip_error_code(uint16_t code);
  void set_qmi_voice_orig_ussd_ind_ussd_error_description(uint32_t len, uint16_t val[]);
  uint16_t* set_qmi_voice_uss_info_utf16(uint32_t uss_info_utf16_len);
/*
  QMI_VOICE_USSD_RELEASE_IND_V02
*/
  void update_default_qmi_voice_ussd_release_ind_msg();

/*
  QMI_VOICE_SUPS_IND_V02
*/
  void update_default_qmi_voice_sups_ind_msg(service_type_enum_v02 service_type, uint8_t is_modified_by_call_control);
  void set_qmi_voice_sups_ind_call_id(uint8_t val);
  void set_qmi_voice_sups_ind_data_source(voice_sups_data_source_enum_v02 data_source);
  voice_alpha_ident_type_v02* set_qmi_voice_sups_ind_alpha_ident();
  voice_uss_info_type_v02* set_qmi_voice_sups_ind_uss_info();
  uint16_t* set_qmi_voice_sups_ind_uss_info(uint32_t uss_info_utf16_len);
  void set_qmi_voice_sups_ind_failure_cause(qmi_sups_errors_enum_v02 failure_cause);
  void set_qmi_voice_sups_ind_reason(voice_sups_ind_reason_enum_v02 reason);
  void set_qmi_voice_sups_ind_service_class(uint8_t val);
  voice_clip_response_type_v02* set_qmi_voice_sups_ind_clip_status();
  voice_clir_response_type_v02* set_qmi_voice_sups_ind_clir_status();
  voice_get_call_forwarding_info_type_v02 *set_qmi_voice_sups_ind_call_forwarding_info(uint32_t  len);

/*
  QMI_VOICE_DTMF_IND_V02
*/
  voice_dtmf_info_type_v02* update_default_qmi_voice_dtmf_ind_msg();
  void set_qmi_voice_dtmf_ind_on_length(dtmf_onlength_enum_v02 val);
  void set_qmi_voice_dtmf_ind_off_length(dtmf_offlength_enum_v02 val);

/*
  QMI_VOICE_EXT_BRST_INTL_IND_V02
*/
  void update_default_qmi_voice_ext_brst_intl_ind_msg(uint16_t mcc, uint8_t db_subtype, uint8_t chg_ind, uint8_t sub_unit, uint8_t unit);

/*
  QMI_VOICE_UUS_IND_V02
*/
  voice_uus_info_type_v02* update_default_qmi_voice_uus_ind_msg();
/*
QMI_VOICE_MODIFIED_IND_V02
*/
  void update_default_qmi_voice_modified_ind_msg(uint8_t call_id);
  void set_qmi_voice_modified_ind_call_type(call_type_enum_v02 call_type);
  void set_qmi_voice_modified_ind_audio_attrib(voice_call_attribute_type_mask_v02 audio_attrib);
  void set_qmi_voice_modified_ind_video_attrib(voice_call_attribute_type_mask_v02 video_attrib);
  void set_qmi_voice_modified_ind_local_attrib(uint8 call_id,voice_call_attribute_type_mask_v02 audio_attrib,
                                          voice_capability_restrict_cause_enum_v02 audio_cause,
                                          voice_call_attribute_type_mask_v02 video_attrib,
                                          voice_capability_restrict_cause_enum_v02 video_cause);
  void set_qmi_voice_modified_ind_peer_attrib(uint8 call_id,voice_call_attribute_type_mask_v02 audio_attrib,
                                          voice_capability_restrict_cause_enum_v02 audio_cause,
                                          voice_call_attribute_type_mask_v02 video_attrib,
                                          voice_capability_restrict_cause_enum_v02 video_cause);
  void set_qmi_voice_modified_ind_call_attrib_status(voice_call_attrib_status_enum_v02 call_attrib_status);
  void set_qmi_voice_modified_ind_failure_cause(qmi_sups_errors_enum_v02 failure_cause);
  void set_qmi_voice_modified_ind_rtt_mode(rtt_mode_type_v02 rtt_mode);
  void set_qmi_voice_modified_ind_call_modified_cause(voice_call_modified_cause_enum_v02 val);


/*
QMI_VOICE_MODIFY_ACCEPT_IND_V02
*/

  void udpate_default_qmi_voice_modify_accept_ind_msg(uint8_t call_id);
  void set_qmi_voice_modified_accept_ind_call_type(call_type_enum_v02 call_type);
  void set_qmi_voice_modified_accept_ind_audio_attrib(voice_call_attribute_type_mask_v02 video_attrib);
  void set_qmi_voice_modified_accept_ind_video_attrib(voice_call_attribute_type_mask_v02 video_attrib);
/*
QMI_VOICE_SPEECH_CODEC_INFO_IND_V02
*/
  void update_default_qmi_voice_speech_codec_info_ind_msg();
  void set_qmi_voice_speech_codec_info_ind_network_mode(voice_network_mode_enum_v02 network_mode);
  void set_qmi_voice_speech_codec_info_ind_speech_codec(voice_speech_codec_enum_v02 speech_codec);
  void set_qmi_voice_speech_codec_info_ind_call_id(uint8_t call_id);

/*
  QMI_VOICE_HANDOVER_IND_V02
*/
  void update_default_qmi_voice_handover_ind_msg(voice_handover_state_enum_v02 ho_state);
/*
  QMI_VOICE_VICE_DIALOG_INFO_IND_V02
*/
uint8_t* update_default_qmi_voice_vice_dialog_info_ind_msg(uint32_t len, uint32_t sequence);
void set_qmi_voice_vice_dialog_info_ind_total_size(uint32_t total_size);

/*
  QMI_VOICE_CONFERENCE_INFO_IND_V02
*/
  uint8_t* update_default_qmi_voice_conference_info_ind_msg(uint32_t conference_xml_len,uint32_t sequence);
  void set_qmi_voice_conference_info_ind_total_size(uint32_t total_size);
  void set_qmi_voice_conference_info_ind_call_id(uint32_t call_id);
/*
  QMI_AUTO_REJECTED_INCOMING_CALL_END_IND_V02
*/
void update_default_qmi_voice_auto_rejected_incoming_call_end_ind_msg(
    call_type_enum_v02 call_type, call_end_reason_enum_v02 call_end_reason, uint16_t sip_error_code);
void set_qmi_voice_auto_rejected_incoming_call_end_ind_num(char *number);

/*
  QMI_VOICE_GET_COLR_REQ_V02
*/
  void update_default_qmi_voice_get_colr_req_v02();
#ifndef UTF_TARGET_BUILD
/*
  QMI_VOICE_GET_COLR_RESP_V02
*/
  void update_default_qmi_voice_get_colr_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);
  void set_qmi_voice_get_colr_resp_msg_colr_response (active_status_enum_v02 active_status, provision_status_enum_v02 provision_status);
  void set_qmi_voice_get_colr_resp_msg_colr_pi (pi_colr_enum_v02 colr_pi);
#endif

//--------------------------------------------------------------------------------
  /*
    QMI_WMS_SET_PRIMARY_CLIENT_REQ_V01
  */
  void update_default_qmi_wms_set_primary_client_req_v01(void);
  void set_qmi_wms_set_primary_client_req_v01_primary_client(uint8_t primary_client);

  /*
    QMI_WMS_SET_PRIMARY_CLIENT_RESP_V01
  */
  void update_default_qmi_wms_set_primary_client_resp_v01(void);
  void set_qmi_wms_set_primary_client_resp_v01_result(qmi_result_type_v01 result, qmi_error_type_v01 error);

  /*
    QMI_WMS_SET_ROUTES_REQ_V01
  */
  void update_default_qmi_wms_set_routes_req_v01(void);

  /*
    QMI_WMS_SET_ROUTES_RESP_V01
  */
  void update_default_qmi_wms_set_routes_resp_v01(void);
  void set_qmi_wms_set_routes_resp_v01_result(qmi_result_type_v01 result, qmi_error_type_v01 error);

  /*
    QMI_WMS_SET_EVENT_REPORT_REQ_V01
  */
  void update_default_qmi_wms_set_event_report_req_v01(void);

  /*
    QMI_WMS_SET_EVENT_REPORT_RESP_V01
  */
  void update_default_qmi_wms_set_event_report_resp_v01(void);
  void set_qmi_wms_set_event_report_resp_v01_result(qmi_result_type_v01 result, qmi_error_type_v01 error);

  /*
    QMI_WMS_INDICATION_REGISTER_REQ_V01
  */
  void update_default_qmi_wms_indication_register_req_v01(void);
  void set_qmi_wms_indication_register_req_v01_reg_transport_layer_info_events(uint8_t reg_transport_layer_info_events);
  void set_qmi_wms_indication_register_req_v01_reg_transport_layer_mwi_info_events(uint8_t reg_transport_layer_mwi_info_events);

  /*
    QMI_WMS_INDICATION_REGISTER_RESP_V01
  */
  void update_default_qmi_wms_indication_register_resp_v01(void);
  void set_qmi_wms_indication_register_resp_v01_result(qmi_result_type_v01 result, qmi_error_type_v01 error);

  /*
    QMI_WMS_GET_INDICATION_REGISTER_REQ_V01
  */
  void update_default_qmi_wms_get_indication_register_req_v01(void);

  /*
    QMI_WMS_GET_INDICATION_REGISTER_RESP_V01
  */
  void update_default_qmi_wms_get_indication_register_resp_v01(void);
  void set_qmi_wms_get_indication_register_resp_v01_result(qmi_result_type_v01 result, qmi_error_type_v01 error);


/*
  QMI_NAS_PERFORM_NETWORK_SCAN_REQ_MSG_V01
*/
  void update_default_qmi_nas_perfrom_network_scan_req_msg();
  void set_qmi_nas_perform_network_scan_req_msg_network_type(nas_network_type_mask_type_v01 val);

/*
  QMI_NAS_PERFORM_NETWORK_SCAN_REQ_MSG_V01
*/
   void update_default_qmi_nas_perform_network_scan_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);
  void set_qmi_nas_3gpp_network_info_tlv_network_scan_resp_msg(int nw_cnt, uint16_t mcc, uint16_t mnc, uint8_t network_status, char *network_description);
  void set_qmi_nas_radio_access_technology_tlv_network_scan_resp_msg(uint16_t mcc, uint16_t mnc, uint8_t rat);
  void set_qmi_nas_mnc_pcs_digit_tlv_network_scan_resp_msg(uint16_t mcc, uint16_t mnc, uint8_t mnc_includes_pcs_digit);
  void set_qmi_nas_scan_result_tlv_network_scan_resp_msg(nas_scan_result_enum_v01 scan_result);
  void set_qmi_nas_csg_nw_info_tlv_network_scan_resp_msg(uint16_t mcc, uint16_t mnc, nas_csg_list_cat_enum_v01 csg_list_cat, uint32_t id, uint32_t name_len, uint16_t *name);
  void set_qmi_nas_csg_sig_info_tlv_network_scan_resp_msg(uint16_t mcc, uint16_t mnc, uint32_t csg_id, uint32_t signal_strength);

/*
  QMI_NAS_GET_DUAL_STANDBY_PREF_REQ_MSG_V01
*/
  void update_default_qmi_nas_get_dual_standby_pref_req_msg();
  void update_default_qmi_nas_get_dual_standby_pref_resp_msg(
          qmi_result_type_v01 result, qmi_error_type_v01 error);
  void set_qmi_nas_get_dual_standby_pref_resp_active_subs_mask(nas_active_subs_mask_type_v01 val);

/*
  QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01
*/
  void update_default_qmi_nas_set_system_selection_preference_req_msg();
  nas_band_pref_mask_type_v01* set_qmi_nas_system_selection_preference_req_band_pref();
  void set_qmi_nas_system_selection_preference_req_mnc_includes_pcs_digit(uint8_t val);
  void set_qmi_nas_system_selection_preference_req_roam_pref(nas_roam_pref_enum_v01 roam_pref);
  nas_net_sel_pref_type_v01* set_qmi_nas_system_selection_preference_req_net_sel_pref();
  void set_qmi_nas_system_selection_preference_req_mode_pref(uint16_t val, uint8 gw_acq_order_pref_valid, uint16_t gw_acq_order_pref);
  void set_qmi_nas_system_selection_preference_req_emergency_mode(uint8_t val);
  void set_qmi_nas_system_selection_preference_req_srv_reg_restriction(nas_srv_reg_restriction_enum_v01 val);
  void set_qmi_nas_system_selection_preference_req_srv_domain_pref(nas_srv_domain_pref_enum_type_v01 val);
  void set_qmi_nas_system_selection_preference_req_voice_domain_pref(nas_voice_domain_pref_enum_type_v01 val);

/*
  QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01
*/
  void update_default_qmi_nas_set_system_selection_preference_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_NAS_GET_CELL_LOCATION_INFO_REQ_MSG_V01
*/
  void update_default_qmi_nas_get_cell_location_info_req_msg();

/*
  QMI_NAS_GET_CELL_LOCATION_INFO_RESP_MSG_V01
*/
  void update_default_qmi_nas_get_cell_location_info_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
  nas_geran_cell_info_type_v01 *set_qmi_nas_get_cell_location_info_resp_geran_info();
  nas_umts_cell_info_type_v01 *set_qmi_nas_get_cell_location_info_resp_umts_info();


/*
  QMI_NAS_DUAL_STANDBY_PREF_REQ_MSG_V01
*/
  void update_default_qmi_nas_standby_pref_req_msg();
  void set_qmi_nas_dual_standby_pref_req_standby_pref(nas_standby_pref_enum_v01 val);
  void set_qmi_nas_dual_standby_pref_req_default_data_subs(nas_subs_type_enum_v01 val);
  void set_qmi_nas_dual_standby_pref_req_priority_subs(nas_subs_type_enum_v01 val);

/*
  QMI_NAS_DUAL_STANDBY_PREF_RESP_MSG_V01
*/
  void update_default_qmi_nas_standby_pref_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);

/*
  QMI_NAS_CONFIG_EMBMS_REQ_MSG_V01
*/
void update_default_qmi_nas_config_embms_req_msg(uint8_t enable);
void set_qmi_nas_config_ebmbms_req_trace_id(int16_t val);
/*
  QMI_NAS_CONFIG_EMBMS_RESP_MSG_V01
*/
void update_default_qmi_nas_config_embms_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);

/*
  QMI_NAS_GET_EMBMS_SIG_REQ_MSG_V01
*/
void update_default_qmi_nas_get_embms_sig_req_msg();
void set_qmi_nas_get_embms_sig_req_trace_id(int16_t val);

/*
  QMI_NAS_GET_EMBMS_SIG_RESP_MSG_V01
*/
void update_default_qmi_nas_get_embms_sig_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
void set_qmi_nas_get_embms_sig_resp_trace_id(int16_t val);
nas_lte_cphy_mbsfn_area_signal_strength_type_v01* set_qmi_nas_get_embms_sig_resp_sig_list(uint32_t sig_list_len);


/*
  QMI_NAS_GET_CENTRALIZED_EONS_SUPPORT_STATUS_REQ_MSG_V01
*/

/*
  QMI_NAS_GET_CENTRALIZED_EONS_SUPPORT_STATUS_RESP_MSG_V01
*/

void update_default_qmi_nas_get_centralized_eons_support_status_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
void set_qmi_nas_get_centralized_eons_support_status_resp_centralized_eons_supported(uint8_t val);

/*
  QMI_NAS_GET_RF_BAND_INFO_REQ_MSG_V01
*/
void update_default_qmi_nas_get_rf_band_info_req_msg();

/*
  QMI_NAS_GET_RF_BAND_INFO_RESP_MSG_V01
*/
void update_default_qmi_nas_get_rf_band_info_resp_msg();
nas_rf_band_info_type_v01* update_default_qmi_nas_get_rf_band_info_resp_msg(uint32_t rf_band_info_list_len,nas_radio_if_enum_v01 radio_if,nas_active_band_enum_v01 active_band,uint16_t active_channel,qmi_result_type_v01 result,qmi_error_type_v01 error);

/*
  QMI_NAS_LTE_CPHY_CA_IND_V01
*/
void update_default_qmi_nas_lte_cphy_ca_ind(nas_lte_cphy_ca_indicator_type_v01 cphy_ca);
/*
  QMI_NAS_GET_LTE_CPHY_CA_INFO_REQ_MSG_V01
*/
void update_default_qmi_nas_get_lte_cphy_ca_info_req_msg();

/*
  QMI_NAS_GET_LTE_CPHY_CA_INFO_RESP_MSG_V01
*/
void update_default_nas_get_lte_cphy_ca_info_resp_msg();

/*
  QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01
*/

/*
  QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01
*/
void update_default_qmi_nas_get_system_selection_preference_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
void set_qmi_nas_get_system_selection_preference_resp_emergency_mode(uint8_t val);
void set_qmi_nas_get_system_selection_preference_resp_mode_pref(uint16_t val);
void set_qmi_nas_get_system_selection_preference_resp_band_pref(uint64_t val);
void set_qmi_nas_get_system_selection_preference_resp_prl_pref(nas_prl_pref_enum_v01 val);
void set_qmi_nas_get_system_selection_preference_resp_roam_pref(nas_roam_pref_enum_v01 val);
void set_qmi_nas_get_system_selection_preference_resp_net_sel_pref(nas_net_sel_pref_enum_v01 val);
void set_qmi_nas_get_system_selection_preference_resp_srv_reg_restriction(nas_srv_reg_restriction_enum_v01 val);

/*
  QMI_NAS_GET_SIGNAL_STRENGTH_REQ_MSG_V01
*/
void update_default_qmi_nas_get_signal_strength_req_msg();
void set_qmi_nas_get_signal_strength_req_request_mask(uint16_t val);
/*
  QMI_NAS_GET_SIGNAL_STRENGTH_RESP_MSG_V01
*/
void update_default_qmi_nas_get_signal_strength_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error,int8_t sig_strength, nas_radio_if_enum_v01 radio_if);
nas_rssi_type_v01* set_qmi_nas_get_signal_strength_resp_rssi(uint32_t rssi_len);
nas_signal_strength_list_type_v01* set_qmi_nas_get_signal_strength_resp_signal_strength_list(uint32_t signal_strength_list_len);
void set_qmi_nas_get_signal_strength_resp_io(uint32_t val);
void set_qmi_nas_get_signal_strength_resp_snr(int16_t val);
nas_error_rate_type_v01* set_qmi_nas_get_signal_strength_resp_error_rate(uint32_t error_rate_len) ;
nas_ecio_type_v01* set_qmi_nas_get_signal_strength_resp_ecio(uint32_t ecio_len) ;
void set_qmi_nas_get_signal_strength_resp_rsrq(int8_t rsrq,uint8_t radio_if) ;
/*
  QMI_NAS_GET_SIG_INFO_REQ_MSG_V01
*/

/*
  QMI_NAS_GET_SIG_INFO_RESP_MSG_V01
*/
void update_default_qmi_nas_get_sig_info_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
nas_common_sig_info_param_type_v01* set_qmi_nas_get_sig_info_resp_cdma_sig_info();
nas_hdr_sig_info_type_v01* set_qmi_nas_get_sig_info_resp_hdr_sig_info();
void set_qmi_nas_get_sig_info_resp_gsm_sig_info(int8_t val);
nas_common_sig_info_param_type_v01* set_qmi_nas_get_sig_info_resp_wcdma_sig_info();
void set_qmi_nas_get_sig_info_resp_rscp(int8_t val);
nas_lte_sig_info_type_v01* set_qmi_nas_get_sig_info_resp_lte_sig_info();

/*
  QMI_NAS_GET_ERR_RATE_REQ_MSG_V01
*/

/*
  QMI_NAS_GET_ERR_RATE_RESP_MSG_V01
*/
void update_qmi_nas_get_err_rate_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
void set_qmi_nas_get_err_rate_resp_gsm_bit_err_rate(uint8_t val);
void set_qmi_nas_get_err_rate_resp_wcdma_block_err_rate(uint8_t val);
void set_qmi_nas_get_err_rate_resp_tdscdma_block_err_rate(uint8_t val);
/*
  QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
*/

/*
  QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
*/
void update_qmi_nas_get_sys_info_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
nas_3gpp2_srv_status_info_type_v01* set_qmi_nas_get_sys_info_resp_cdma_srv_status_info();
nas_3gpp2_srv_status_info_type_v01* set_qmi_nas_get_sys_info_resp_hdr_srv_status_info();
nas_3gpp_srv_status_info_type_v01* set_qmi_nas_get_sys_info_resp_gsm_srv_status_info();
nas_3gpp_srv_status_info_type_v01* set_qmi_nas_get_sys_info_resp_wcdma_srv_status_info();
nas_3gpp_srv_status_info_type_v01* set_qmi_nas_get_sys_info_resp_tdscdma_srv_status_info();
nas_3gpp_srv_status_info_type_v01* set_qmi_nas_get_sys_info_resp_lte_srv_status_info();
nas_cdma_sys_info_type_v01* set_qmi_nas_get_sys_info_resp_cdma_sys_info();
nas_hdr_sys_info_type_v01* set_qmi_nas_get_sys_info_resp_hdr_sys_info();
nas_gsm_sys_info_type_v01* set_qmi_nas_get_sys_info_resp_gsm_sys_info();
nas_wcdma_sys_info_type_v01* set_qmi_nas_get_sys_info_resp_wcdma_sys_info();
nas_tdscdma_sys_info_type_v01* set_qmi_nas_get_sys_info_resp_tdscdma_sys_info();
nas_lte_sys_info_type_v01* set_qmi_nas_get_sys_info_resp_lte_sys_info();
nas_cdma_sys_info2_type_v01* set_qmi_nas_get_sys_info_resp_cdma_sys_info2();
nas_hdr_sys_info2_type_v01* set_qmi_nas_get_sys_info_resp_hdr_sys_info2();
nas_gsm_sys_info2_type_v01* set_qmi_nas_get_sys_info_resp_gsm_sys_info2();
nas_wcdma_sys_info2_type_v01* set_qmi_nas_get_sys_info_resp_wcdma_sys_info2();
nas_lte_sys_info2_type_v01* set_qmi_nas_get_sys_info_resp_lte_sys_info2();
nas_gw_sys_info3_type_v01* set_qmi_nas_get_sys_info_resp_gsm_sys_info3();
nas_gw_sys_info3_type_v01* set_qmi_nas_get_sys_info_resp_wcdma_sys_info3();
void set_qmi_nas_get_sys_info_resp_voice_support_on_lte(uint8_t val);
void set_qmi_nas_get_sys_info_resp_lte_embms_coverage(uint8_t val);
void set_qmi_nas_get_sys_info_resp_sim_rej_info(nas_sim_rej_info_enum_type_v01 val);
void set_qmi_nas_get_sys_info_resp_lte_voice_status(nas_lte_voice_status_enum_type_v01 lte_voice_status);
void set_qmi_nas_get_sys_info_resp_lte_sms_status(nas_sms_status_enum_type_v01 lte_sms_status);
void set_qmi_nas_get_sys_info_resp_wcdma_csg_info(uint8_t csg_info_valid, uint32_t id, uint32_t name_len, uint16_t *name);
void set_qmi_nas_get_sys_info_resp_lte_csg_info(uint8_t csg_info_valid, uint32_t id, uint32_t name_len, uint16_t *name);

/*
  QMI_NAS_GET_SERVING_SYSTEM_REQ_MSG_V01
*/

/*
  QMI_NAS_GET_SERVING_SYSTEM_RESP_MSG_V01
*/
nas_serving_system_type_v01* update_qmi_nas_get_serving_system_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
nas_cdma_base_station_info_type_v01* set_qmi_nas_get_serving_system_resp_cdma_base_station_info();
nas_cdma_system_id_type_v01* set_qmi_nas_get_serving_system_resp_cdma_system_id();
void set_qmi_nas_get_serving_system_resp_roaming_indicator(nas_roaming_indicator_enum_v01 val);
void set_qmi_nas_get_serving_system_resp_lac(uint16_t val);
void set_qmi_nas_get_serving_system_resp_cell_id(uint32_t val);
void set_qmi_nas_get_serving_system_resp_ccs(uint8_t val);
void set_qmi_nas_get_serving_system_resp_prl_ind(uint8_t val);
nas_ss_detailed_service_info_type_v01* set_qmi_nas_get_serving_system_resp_detailed_service_info();
nas_cdma_system_id_ext_type_v01* set_qmi_nas_get_serving_system_resp_cdma_system_id_ext();
void set_qmi_nas_get_serving_system_resp_hdr_personality(nas_hdr_personality_enum_v01 val);
nas_data_capabilites_enum_v01* set_qmi_nas_get_serving_system_resp_data_capabilities(uint32_t data_capabilities_len) ;
nas_plmn_type_v01* set_qmi_nas_get_serving_system_resp_current_plmn();
void set_qmi_nas_get_serving_system_resp_p_rev_in_use(uint8_t val);
void set_qmi_nas_get_serving_system_resp_tac(uint16_t val);
nas_gw_sys_info3_type_v01* set_qmi_nas_get_serving_system_resp_call_barring_status();
void set_qmi_nas_get_serving_system_resp_umts_psc(uint16_t val);
nas_mnc_pcs_digit_include_status_type_v01* set_qmi_nas_get_serving_system_resp_mnc_includes_pcs_digit();
void set_qmi_nas_get_serving_system_resp_hs_call_status(nas_hs_support_enum_type_v01 val);
void set_qmi_nas_get_serving_system_resp_def_roam_ind(nas_roaming_indicator_enum_v01 val);

/*
  QMI_NAS_GET_OPERATOR_NAME_DATA_REQ_MSG_V01
*/
/*
  QMI_NAS_GET_OPERATOR_NAME_DATA_RESP_MSG_V01
*/
void update_default_qmi_nas_get_operator_name_data_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);

/*
  QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO_REQ_MSG_V01
*/
void update_default_qmi_nas_get_3gpp2_subscription_info_req_msg(uint8_t nam_id);
void set_qmi_nas_get_3gpp2_subscription_info_req_get_3gpp2_info_mask(get_3gpp2_info_mask_enum_type_v01 val);
/*
  QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO_RESP_MSG_V01
*/
void upate_default_qmi_nas_get_3gpp2_subscription_info_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
char* set_qmi_nas_get_3gpp2_subscription_info_resp_nam_name(uint32_t nam_name_len);
char* set_qmi_nas_get_3gpp2_subscription_info_resp_mdn(uint32_t mdn_len);
nas_3gpp2_home_sid_nid_info_type_v01* set_qmi_nas_get_3gpp2_subscription_info_resp_cdma_sys_id(uint32_t cdma_sys_id_len) ;
nas_3gpp2_min_based_info_type_v01* set_qmi_nas_get_3gpp2_subscription_info_resp_min_based_info();

/*
  QMI_NAS_GET_PLMN_NAME_REQ_MSG_V01
*/
void update_default_qmi_nas_get_plmn_name_req_msg(uint16_t mcc, uint16_t mnc);
void set_qmi_nas_get_plmn_name_req_mnc_includes_pcs_digit(uint8_t val);
void set_qmi_nas_get_plmn_name_req_suppress_sim_error(uint8_t val);
void set_qmi_nas_get_plmn_name_req_use_static_table_only(uint8_t val);
void set_qmi_nas_get_plmn_name_req_rat(nas_radio_if_enum_v01 val);
void set_qmi_nas_get_plmn_name_req_always_send_plmn_name(uint8_t val);
void set_qmi_nas_get_plmn_name_req_csg_id(uint32_t csg_id);
/*
  QMI_NAS_GET_PLMN_NAME_RESP_MSG_V01
*/
void update_default_qmi_nas_plmn_name_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
nas_3gpp_eons_plmn_name_type_v01* set_qmi_nas_get_plmn_name_resp_eons_plmn_name_3gpp() ;

/*
  QMI_NAS_BIND_SUBSCRIPTION_REQ_MSG_V01
*/
void update_default_qmi_nas_bind_subscription_req_msg(nas_subs_type_enum_v01 subs_type);

/*
  QMI_NAS_BIND_SUBSCRIPTION_RESP_MSG_V01
*/
void update_default_qmi_nas_bind_subscription_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
/*
  QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01
*/

void update_default_qmi_nas_indication_register_req_msg();
void set_qmi_nas_indication_register_req_reg_sys_sel_pref(uint8_t val);
void set_qmi_nas_indication_register_req_reg_ddtm_events(uint8_t val);
void set_qmi_nas_indication_register_req_dual_standby_pref(uint8_t val);
void set_qmi_nas_indication_register_req_subscription_info(uint8_t val);
void set_qmi_nas_indication_register_req_reg_network_time(uint8_t val);
void set_qmi_nas_indication_register_req_reg_rtre_cfg(uint8_t val);
void set_qmi_nas_indication_register_req_reg_embms_status(uint8_t val);
void set_qmi_nas_indication_register_req_sig_info(uint8_t val);
void set_qmi_nas_indication_register_req_err_rate(uint8_t val);
void set_qmi_nas_indication_register_req_reg_managed_roaming(uint8_t val);
void set_qmi_nas_indication_register_req_reg_hdr_session_close(uint8_t val);
void set_qmi_nas_indication_register_req_reg_hdr_uati(uint8_t val);
void set_qmi_nas_indication_register_req_reg_current_plmn_name(uint8_t val);
void set_qmi_nas_indication_register_req_sys_info(uint8_t val);
void set_qmi_nas_indication_register_req_req_serving_system(uint8_t val);
void set_qmi_nas_indication_register_req_reg_rf_band_info(uint8_t val);

/*
  QMI_NAS_INDICATION_REGISTER_RESP_MSG_V01
*/
void update_default_qmi_nas_indication_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);

/*
  QMI_NAS_FORCE_NETWORK_SEARCH_REQ_MSG_V01
*/

/*
  QMI_NAS_FORCE_NETWORK_SEARCH_RESP_MSG_V01
*/
void update_default_qmi_nas_force_network_search_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);

/*
  QMI_NAS_SET_EVENT_REPORT_REQ_MSG_V01
*/

void update_default_qmi_nas_set_event_report_req_msg();
nas_signal_stregth_indicator_type_v01* set_qmi_nas_set_event_report_req_signal_strength();
void set_qmi_nas_set_event_report_req_report_rf_band_info(uint8_t val);
void set_qmi_nas_set_event_report_req_report_reg_reject(uint8_t val);
nas_rssi_indicator_type_v01* set_qmi_nas_set_event_report_req_rssi_indicator();
nas_ecio_indicator_type_v01* set_qmi_nas_set_event_report_req_ecio_indicator();
nas_io_indicator_type_v01* set_qmi_nas_set_event_report_req_io_indicator();
nas_sinr_indicator_type_v01* set_qmi_nas_set_event_report_req_sinr_indicator();
void set_qmi_nas_set_event_report_req_report_error_rate(uint8_t val);
nas_rsrq_indicator_type_v01* set_qmi_nas_set_event_report_req_rsrq_indicator();

/*
  QMI_NAS_SET_EVENT_REPORT_RESP_MSG_V01
*/
void update_default_qmi_nas_set_event_report_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);

/*
  QMI_NAS_CONFIG_SIG_INFO_REQ_MSG_V01
*/
void update_default_qmi_nas_config_sig_info_req_msg();
int8_t* set_qmi_nas_config_sig_info_req_rssi_threshold_list(uint32_t rssi_threshold_list_len);
int16_t* set_qmi_nas_config_sig_info_req_ecio_threshold_list(uint32_t ecio_threshold_list_len);
uint8_t* set_qmi_nas_config_sig_info_req_hdr_sinr_threshold_list(uint32_t hdr_sinr_threshold_list_len);
int16_t* set_qmi_nas_config_sig_info_req_lte_snr_threshold_list(uint32_t lte_snr_threshold_list_len);
int8_t* set_qmi_nas_config_sig_info_req_lte_rsrq_threshold_list(uint32_t lte_rsrq_threshold_list_len);
int16_t* set_qmi_nas_config_sig_info_req_lte_rsrp_threshold_list(uint32_t lte_rsrp_threshold_list_len);
int8_t* set_qmi_nas_config_sig_info_req_rscp_threshold_list(uint32_t rscp_threshold_list_len);

/*
  QMI_NAS_CONFIG_SIG_INFO_RESP_MSG_V01
*/
void update_default_qmi_nas_config_sig_info_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
/*
  QMI_NAS_LIMIT_SYS_INFO_IND_REPORTING_REQ_MSG_V01
*/
void update_default_qmi_nas_limit_sys_info_ind_reporting_req_msg(nas_limit_sys_info_ind_mask_type_v01 limit_sys_info_chg_rpt);
/*
  QMI_NAS_LIMIT_SYS_INFO_IND_REPORTING_RESP_MSG_V01
*/
void update_default_qmi_nas_limit_sys_info_ind_reporting_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
/*
  QMI_NAS_GET_MODE_PREF_REQ_MSG_V01
*/
void update_default_qmi_nas_get_mode_pref_req();
/*
  QMI_NAS_GET_MODE_PREF_RESP_MSG_V01
*/
void update_default_qmi_nas_get_mode_pref_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
void set_qmi_nas_get_mode_pref_resp_idx0_mode_pref(mode_pref_mask_type_v01 val);
void set_qmi_nas_get_mode_pref_resp_idx1_mode_pref(mode_pref_mask_type_v01 val);
void set_qmi_nas_get_mode_pref_resp_idx2_mode_pref(mode_pref_mask_type_v01 val);
/*
  QMI_NAS_SET_RTRE_CONFIG_REQ_MSG_V01
*/
void update_default_qmi_nas_set_rtre_config_req_msg(nas_rtre_cfg_enum_v01 rtre_cfg_pref, int spc_valid, char *spc, int spc_len);

/*
  QMI_NAS_SET_RTRE_CONFIG_RESP_MSG_V01
*/
void update_default_qmi_nas_set_rtre_config_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
/*
  QMI_NAS_GET_RTRE_CONFIG_REQ_MSG_V01
*/
void update_default_qmi_nas_get_rtre_config_req_msg (void);
/*
  QMI_NAS_GET_RTRE_CONFIG_RESP_MSG_V01
*/
void update_default_qmi_nas_get_rtre_config_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
void set_qmi_nas_get_rtre_config_resp_rtre_cfg(nas_rtre_cfg_enum_v01 val);

/*
  QMI_NAS_UPDATE_AKEY_REQ_MSG_V01
*/
uint8_t* update_default_qmi_nas_akey_req_msg();
/*
  QMI_NAS_UPDATE_AKEY_REQ_MSG_V01
*/
void update_default_qmi_nas_akey_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);

/*
  QMI_NAS_GET_HOME_NETWORK_REQ_MSG_V01
*/
/*
  QMI_NAS_GET_HOME_NETWORK_RESP_MSG_V01
*/
nas_plmn_type_v01* update_default_qmi_nas_get_home_network_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
/*
  QMI_NAS_GET_MOB_CAI_REV_REQ_MSG_V01
*/
/*
  QMI_NAS_GET_MOB_CAI_REV_RESP_MSG_V01
*/
void update_default_qmi_nas_get_mob_cai_rev_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);

/*
  QMI_PBM_EMERGENCY_LIST_IND_V01
*/
void update_default_qmi_pbm_list_ind_msg();
void update_nw_from_qmi_pbm_list_ind_msg(const char *number);
void update_elenl_from_qmi_pbm_list_ind_msg(const char *number, const char *urn);
void update_card_ecc_from_qmi_pbm_list_ind_msg(const char *number);
void update_hardcoded_ecc_from_qmi_pbm_list_ind_msg(const char *number);
void update_nv_ecc_from_qmi_pbm_list_ind_msg(const char *number);
void update_mcc_ecc_from_qmi_pbm_list_ind_msg(const char *number);

/*
  QMI_NAS_EVENT_REPORT_IND_MSG_V01
*/
void update_default_qmi_nas_event_report_ind_msg();
void set_qmi_nas_even_report_ind_msg();
void set_qmi_nas_even_report_ind_signal_strength(int8_t sig_strength, nas_radio_if_enum_v01 radio_if);
void set_qmi_nas_even_report_ind_rssi(uint8_t rssi, nas_radio_if_enum_v01 radio_if);
void set_qmi_nas_even_report_ind_error_rate(uint16_t error_rate, nas_radio_if_enum_v01 radio_if);
void set_qmi_nas_even_report_ind_ecio(uint8_t ecio, nas_radio_if_enum_v01 radio_if);
void set_qmi_nas_even_report_ind_io(int32_t val);
void set_qmi_nas_even_report_ind_sinr(nas_sinr_enum_v01 sinr);
void set_qmi_nas_even_report_ind_rsrq(int8_t rsrq, uint8_t radio_if);
void set_qmi_nas_even_report_ind_registration_reject_reason(nas_network_service_domain_enum_v01 service_domain, uint16_t reject_cause);
nas_rf_band_info_type_v01* set_qmi_nas_even_report_ind_rf_band_info_list(uint32_t rf_band_info_list_len);

/*
  QMI_NAS_SIG_INFO_IND_MSG_V01
*/
void update_default_qmi_nas_sig_info_ind_msg();
void set_qmi_nas_sig_info_ind_cdma_sig_info (int8_t rssi, int16_t ecio);
nas_hdr_sig_info_type_v01* set_qmi_nas_sig_info_ind_hdr_sig_info();
void set_qmi_nas_sig_info_ind_gsm_sig_info(int8_t val);
void set_qmi_nas_sig_info_ind_wcdma_sig_info(int8_t rssi, int16_t ecio);
void set_qmi_nas_sig_info_ind_rscp(int8_t val);
void set_qmi_nas_sig_info_ind_lte_sig_info(int8_t rssi, int8_t rsrq, int16_t rsrp, int16_t snr);
/*
  QMI_NAS_ERR_RATE_IND_MSG_V01
*/
void update_default_qmi_nas_err_rate_ind_msg();
void set_qmi_nas_err_rate_ind_gsm_bit_err_rate(uint8_t val);
void set_qmi_nas_err_rate_ind_wcdma_block_err_rate(uint8_t val);
void set_qmi_nas_err_rate_ind_tdscdma_block_err_rate(uint8_t val);
/*
  QMI_NAS_SERVING_SYSTEM_IND_MSG_V01
*/
nas_serving_system_type_v01* update_default_qmi_nas_serving_system_ind_msg();
nas_data_capabilites_enum_v01* set_qmi_nas_serving_system_ind_data_capabilities(uint32_t data_capabilities_len);
nas_cdma_system_id_type_v01* set_qmi_nas_serving_system_ind_cdma_system_id();
nas_cdma_base_station_info_type_v01* set_qmi_nas_serving_system_ind_cdma_base_station_info();
nas_ss_detailed_service_info_type_v01* set_qmi_nas_serving_system_ind_detailed_service_info();
void set_qmi_nas_serving_system_ind_hdr_personality(nas_hdr_personality_enum_v01 val);
void set_qmi_nas_serving_system_ind_cell_id(uint32_t val);
void set_qmi_nas_serving_system_ind_lac(uint16_t val);
void set_qmi_nas_serving_system_ind_tac(uint16_t val);
void set_qmi_nas_serving_system_ind_srv_sys_no_change(uint8_t val);

/*
  QMI_NAS_SYSTEM_SELECTION_PREFERENCE_IND_MSG_V01
*/
void update_default_qmi_nas_system_selection_preference_ind_msg();
void set_qmi_nas_system_selection_preference_ind_srv_domain_pref(nas_srv_domain_pref_enum_type_v01 srv_domain_pref);
void set_qmi_nas_system_selection_preference_ind_emergency_mode(uint8_t val);
void set_qmi_nas_system_selection_preference_ind_mode_pref(mode_pref_mask_type_v01 val);
void set_qmi_nas_system_selection_preference_ind_band_pref(nas_band_pref_mask_type_v01 val);
void set_qmi_nas_system_selection_preference_ind_prl_pref(nas_prl_pref_enum_v01 val);
void set_qmi_nas_system_selection_preference_ind_roam_pref(nas_roam_pref_enum_v01 val);
void set_qmi_nas_system_selection_preference_ind_lte_band_pref(lte_band_pref_mask_type_v01 val);
void set_qmi_nas_system_selection_preference_ind_net_sel_pref( nas_net_sel_pref_enum_v01 val);
#ifdef RIL_UTF_L_MASTER
void set_qmi_nas_system_selection_preference_ind_rat_disabled_mask(mode_pref_mask_type_v01 mask);
#endif

/*
  QMI_NAS_SYS_INFO_IND_MSG_V01
*/
void update_default_sys_info_ind_msg();
void set_qmi_nas_sys_info_ind_sys_info_no_change();
void set_qmi_nas_sys_info_ind_wcdma_eutra_status(nas_eutra_cell_status_enum_type_v01 val);
nas_gsm_sys_info_type_v01* set_qmi_nas_sys_info_ind_gsm_sys_info();
nas_wcdma_sys_info_type_v01* set_qmi_nas_sys_info_ind_wcdma_sys_info();
nas_tdscdma_sys_info_type_v01* set_qmi_nas_sys_info_ind_tdscdma_sys_info();
nas_lte_sys_info_type_v01* set_qmi_nas_sys_info_ind_lte_sys_info();
void set_qmi_nas_sys_info_ind_lte_voice_status(nas_lte_voice_status_enum_type_v01 lte_voice_status);
void set_qmi_nas_sys_info_ind_lte_sms_status(nas_sms_status_enum_type_v01 lte_sms_status);
void set_qmi_nas_sys_info_ind_lte_cell_status(nas_cell_access_status_e_type_v01 lte_cell_status);
nas_cdma_sys_info_type_v01* set_qmi_nas_sys_info_ind_cdma_sys_info();
nas_hdr_sys_info_type_v01* set_qmi_nas_sys_info_ind_hdr_sys_info();
nas_3gpp2_srv_status_info_type_v01* set_qmi_nas_sys_info_ind_cdma_srv_status_info();
nas_3gpp2_srv_status_info_type_v01* set_qmi_nas_sys_info_ind_hdr_srv_status_info();
nas_3gpp_srv_status_info_type_v01* set_qmi_nas_sys_info_ind_gsm_srv_status_info();
nas_3gpp_srv_status_info_type_v01* set_qmi_nas_sys_info_ind_wcdma_srv_status_info();
nas_3gpp_srv_status_info_type_v01* set_qmi_nas_sys_info_ind_lte_srv_status_info();
nas_3gpp_srv_status_info_type_v01* set_qmi_nas_sys_info_ind_tdscdma_srv_status_info();
nas_cdma_sys_info2_type_v01* set_qmi_nas_sys_info_ind_cdma_sys_info2();
nas_hdr_sys_info2_type_v01* set_qmi_nas_sys_info_ind_hdr_sys_info2();
nas_gsm_sys_info2_type_v01* set_qmi_nas_sys_info_ind_gsm_sys_info2();
nas_wcdma_sys_info2_type_v01* set_qmi_nas_sys_info_ind_wcdma_sys_info2();
nas_lte_sys_info2_type_v01* set_qmi_nas_sys_info_ind_lte_sys_info2();
nas_gw_sys_info3_type_v01* set_qmi_nas_sys_info_ind_gsm_sys_info3();
nas_gw_sys_info3_type_v01* set_qmi_nas_sys_info_ind_wcdma_sys_info3();
void set_qmi_nas_sys_info_ind_voice_support_on_lte(uint8_t val);
void set_qmi_nas_sys_info_ind_lte_embms_coverage(uint8_t val);
void set_qmi_nas_sys_info_ind_sim_rej_info(nas_sim_rej_info_enum_type_v01 val);
void set_qmi_nas_sys_info_ind_srv_reg_restriction(nas_srv_reg_restriction_enum_v01 val);
/*
  QMI_NAS_OPERATOR_NAME_DATA_IND_MSG_V01
*/
void update_default_qmi_nas_operator_name_data_ind_msg();
nas_service_provider_name_type_v01* set_qmi_nas_operator_name_data_ind_service_provider_name ();
nas_operator_plmn_list_type_v01* set_qmi_nas_operator_name_data_ind_operator_plmn_list(uint32_t operator_plmn_list_len);
nas_plmn_network_name_type_v01* set_qmi_nas_operator_name_data_ind_plmn_network_name(uint32_t plmn_network_name_len);
char* set_qmi_nas_operator_name_data_ind_plmn_name();
nas_plmn_network_name_type_v01* set_qmi_nas_operator_name_data_ind_nitz_information();
/*
  QMI_NAS_CURRENT_PLMN_NAME_IND_V01
*/
void update_default_qmi_nas_current_plmn_name_ind_msg();
void set_qmi_nas_current_plmn_name_ind_plmn_id(uint8_t plmn_valid, uint16_t mcc, uint16_t mnc, uint8_t mnc_includes_pcs_digit);
void set_qmi_nas_current_plmn_name_ind_spn(uint8_t spn_valid, nas_coding_scheme_enum_v01 spn_enc, uint32_t spn_len, const char *spn);
nas_plmn_name_type_v01* set_qmi_nas_current_plmn_name_ind_short_name();
nas_plmn_name_type_v01* set_qmi_nas_current_plmn_name_ind_long_name();
/*
  QMI_NAS_CSP_PLMN_MODE_BIT_IND_MSG_V01
*/
void update_default_qmi_nas_csp_plmn_mode_bit_ind_msg();
/*
  QMI_NAS_DUAL_STANDBY_PREF_IND_MSG_V01
*/
void update_default_qmi_nas_dual_standby_pref_ind_msg();
nas_standby_pref_type_v01* set_qmi_nas_standby_pref_ind_standby_pref();
void set_qmi_nas_standby_pref_ind_active_subs_mask(nas_active_subs_mask_type_v01 val);
/*
  QMI_NAS_SUBSCRIPTION_INFO_IND_MSG_V01
*/
void update_default_qmi_nas_subscription_info_ind_msg();
void set_qmi_nas_subscription_info_ind_is_priority_subs(nas_is_priority_subs_enum_v01 val);
void set_qmi_nas_subscription_info_ind_is_active(nas_active_subs_info_enum_v01 val);
void set_qmi_nas_subscription_info_ind_voice_system_id(uint32_t val);
void set_qmi_nas_subscription_info_ind_lte_voice_system_id(uint32_t val);
void set_qmi_nas_subscription_info_ind_wlan_voice_system_id(uint32_t val);
/*
  QMI_NAS_NETWORK_TIME_IND_MSG_V01
*/
void update_default_qmi_nas_network_time_ind_msg(uint16_t year,uint8_t month,uint8_t day,uint8_t hour,uint8_t minute,uint8_t second,uint8_t day_of_week);
void set_qmi_nas_network_time_ind_time_zone(int8_t val);
void set_qmi_nas_network_time_ind_daylt_sav_adj(uint8_t val);

/*
  QMI_NAS_RTRE_CONFIG_IND_V01
*/
void update_default_qmi_nas_rtre_config_ind_msg();
void set_qmi_nas_rtre_config_ind_rtre_cfg(nas_rtre_cfg_enum_v01 val);
/*
  QMI_NAS_MANAGED_ROAMING_IND_MSG_V01
*/
void update_default_qmi_nas_managed_roaming_ind_msg();
/*
  QMI_NAS_EMBMS_STATUS_IND_V01
*/
void update_default_qmi_nas_embms_status_ind_msg(uint8_t enabled);
/*
  QMI_NAS_RF_BAND_INFO_IND_V01
*/
nas_rf_band_info_type_v01* update_default_qmi_nas_rf_band_info_ind_msg();

/*
  QMI_NAS_LTE_SIB16_NETWORK_TIME_IND_V01
*/
void update_default_qmi_nas_lte_sib16_network_time_ind();
void set_qmi_nas_lte_sib16_network_time_ind_lte_sib16_acquired(nas_tri_state_boolean_type_v01 lte_sib16_acquired);
void set_qmi_nas_lte_sib16_network_time_ind_abs_time(uint64_t abs_time);

/*
  QMI_NAS_GET_LTE_SIB16_NETWORK_TIME_REQ_MSG_V01
*/
void update_default_qmi_nas_get_lte_sib16_network_time_req_msg_v01();
/*
  QMI_NAS_GET_LTE_SIB16_NETWORK_TIME_RESP_MSG_V01
*/
void update_default_qmi_nas_get_lte_sib16_network_time_resp_msg_v01(qmi_result_type_v01 result,qmi_error_type_v01 error);

//--------------------------------------------------------------------------------

/*
  QMI_IMSP_SET_EVENT_REPORT_REQ_V01
*/
void update_default_qmi_imsp_set_event_report_req_v01(void);

/*
  QMI_IMSP_SET_EVENT_REPORT_RESP_V01
*/
void update_default_qmi_imsp_set_event_report_resp_v01(void);
void set_qmi_imsp_set_event_report_resp_v01_result(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_IMS_SETTINGS_GET_REG_MGR_CONFIG_REQ_V01
*/
void update_default_qmi_ims_settings_get_reg_mgr_config_req_v01(void);

/*
  QMI_IMS_SETTINGS_GET_REG_MGR_CONFIG_RSP_V01
*/
void update_default_qmi_ims_settings_get_reg_mgr_config_rsp_v01(void);
void set_qmi_ims_settings_get_reg_mgr_config_rsp_v01_result(qmi_result_type_v01 result, qmi_error_type_v01 error);
#ifndef RIL_UTF_IOE
/*
  QMI_IMS_SETTINGS_GET_QIPCALL_CONFIG_REQ_V01
*/
void update_default_qmi_ims_settings_get_qipcall_config_req_v01(void);

#ifndef UTF_TARGET_BUILD
/*
QMI_IMS_SETTINGS_GET_QIPCALL_CONFIG_RSP_V01
*/
void update_default_qmi_ims_settings_get_qipcall_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error);
void set_qmi_ims_settings_get_qipcall_config_rsp_v01_settings_resp(uint8_t val);
void set_qmi_ims_settings_get_qipcall_config_rsp_v01_vt_quality_selector(ims_settings_qipcall_vt_quality_enum_v01 val);
void set_qmi_ims_settings_get_qipcall_config_rsp_v01_mobile_data_enabled(uint8_t val);

/*
  QMI_IMS_SETTINGS_SET_QIPCALL_CONFIG_REQ_V01
*/
void update_default_qmi_ims_settings_set_qipcall_config_req_v01(void);
void set_qmi_ims_settings_set_qipcall_config_req_v01_vt_calling_enabled(uint8_t val);
void set_qmi_ims_settings_set_qipcall_config_req_v01_mobile_data_enabled(uint8_t val);
void set_qmi_ims_settings_set_qipcall_config_req_v01_volte_enabled(uint8_t val);
void set_qmi_ims_settings_set_qipcall_config_req_v01_vt_quality_selector(ims_settings_qipcall_vt_quality_enum_v01 val);
#endif

/*
QMI_IMS_SETTINGS_SET_QIPCALL_CONFIG_RSP_V01
*/
void update_default_qmi_ims_settings_set_qipcall_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error);
void set_qmi_ims_settings_set_qipcall_config_rsp_v01_settings_resp(uint8_t val);
#endif

/*
  QMI_IMS_SETTINGS_SET_REG_MGR_CONFIG_REQ_V01
*/
void update_default_qmi_ims_settings_set_reg_mgr_config_req(void);
void set_qmi_ims_settings_set_reg_mgr_config_req_ims_test_mode_enabled(uint8_t val);
/*
  QMI_IMS_SETTINGS_SET_REG_MGR_CONFIG_RSP_V01
*/
void update_default_qmi_ims_settings_set_reg_mgr_config_rsp(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_REQ_V01
 */
void update_default_qmi_ims_settings_get_client_provisioning_config_req(void);
/*
  QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_RSP_V01
 */
void update_default_qmi_ims_settings_get_client_provisioning_config_rsp(
        qmi_result_type_v01 result,
        qmi_error_type_v01 error);
void set_qmi_ims_settings_get_client_provisioning_config_rsp_wifi_call(
            ims_settings_wfc_status_enum_v01 wifi_call);
void set_qmi_ims_settings_get_client_provisioning_config_rsp_wifi_call_preference(
            ims_settings_wfc_preference_v01 wifi_call_preference);
/*
  QMI_IMS_SETTINGS_SET_CLIENT_PROVISIONING_CONFIG_REQ_V01
 */
void update_default_qmi_ims_settings_set_client_provisioning_config_req(void);
void set_qmi_ims_settings_set_client_provisioning_config_req_wifi_call(
            ims_settings_wfc_status_enum_v01 wifi_call);
void set_qmi_ims_settings_set_client_provisioning_config_req_wifi_call_preference(
            ims_settings_wfc_preference_v01 wifi_call_preference);
/*
  QMI_IMS_SETTINGS_SET_CLIENT_PROVISIONING_CONFIG_RSP_V01
 */
void update_default_qmi_ims_settings_set_client_provisioning_config_rsp(
        qmi_result_type_v01 result,
        qmi_error_type_v01 error);

//=======================================================================================
/*
QMI_IMS_SETTINGS_GET_VOIP_CONFIG_REQ_V01
*/
void update_default_qmi_ims_settings_get_voip_config_req_v01(void);

/*
QMI_IMS_SETTINGS_GET_VOIP_CONFIG_RSP_V01
*/
void update_default_qmi_ims_settings_get_voip_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error);
void set_qmi_ims_settings_get_voip_config_rsp_v01_settings_resp(uint8_t val);
void set_qmi_ims_settings_get_voip_config_rsp_v01_amr_mode(uint8_t val);
void set_qmi_ims_settings_get_voip_config_rsp_v01_amr_wb_mode(uint16_t val);
void set_qmi_ims_settings_get_voip_config_rsp_v01_session_expiry_timer(uint16_t val);
void set_qmi_ims_settings_get_voip_config_rsp_v01_min_session_expiry(uint16_t val);
void set_qmi_ims_settings_get_voip_config_rsp_v01_voip_silent_redial_enabled(uint8_t val);


/*
  QMI_IMS_SETTINGS_SET_VOIP_CONFIG_REQ_V01
*/
void update_default_qmi_ims_settings_set_voip_config_req_v01(void);
void set_qmi_ims_settings_set_voip_config_req_v01_amr_mode(uint8_t val);
void set_qmi_ims_settings_set_voip_config_req_v01_amr_wb_mode(uint16_t val);
void set_qmi_ims_settings_set_voip_config_req_v01_session_expiry_timer(uint16_t val);
void set_qmi_ims_settings_set_voip_config_req_v01_min_session_expiry(uint16_t val);
void set_qmi_ims_settings_set_voip_config_req_v01_voip_silent_redial_enabled(uint8_t val);

/*
QMI_IMS_SETTINGS_SET_VOIP_CONFIG_RSP_V01
*/
void update_default_qmi_ims_settings_set_voip_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error);
void set_qmi_ims_settings_set_voip_config_rsp_v01_settings_resp(uint8_t val);

//============================================================================================

/*
QMI_IMS_SETTINGS_GET_SIP_CONFIG_REQ_V01
*/
void update_default_qmi_ims_settings_get_sip_config_req_v01(void);

/*
QMI_IMS_SETTINGS_GET_SIP_CONFIG_RSP_V01
*/
void update_default_qmi_ims_settings_get_sip_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error);
void set_qmi_ims_settings_get_sip_config_rsp_v01_settings_resp(uint8_t val);
void set_qmi_ims_settings_get_sip_config_rsp_v01_sip_timer_operator_mode_a(uint32_t val);
void set_qmi_ims_settings_get_sip_config_rsp_v01_timer_t1(uint32_t val);
void set_qmi_ims_settings_get_sip_config_rsp_v01_timer_t2(uint32_t val);
void set_qmi_ims_settings_get_sip_config_rsp_v01_timer_tf(uint32_t val);


/*
  QMI_IMS_SETTINGS_SET_SIP_CONFIG_REQ_V01
*/
void update_default_qmi_ims_settings_set_sip_config_req_v01(void);
void set_qmi_ims_settings_set_sip_config_req_v01_sip_timer_operator_mode_a(uint32_t val);
void set_qmi_ims_settings_set_sip_config_req_v01_timer_t1(uint32_t val);
void set_qmi_ims_settings_set_sip_config_req_v01_timer_t2(uint32_t val);
void set_qmi_ims_settings_set_sip_config_req_v01_timer_tf(uint32_t val);

/*
QMI_IMS_SETTINGS_SET_SIP_CONFIG_RSP_V01
*/
void update_default_qmi_ims_settings_set_sip_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error);
void set_qmi_ims_settings_set_sip_config_rsp_v01_settings_resp(uint8_t val);

//============================================================================================

/*
QMI_IMS_SETTINGS_GET_REG_MGR_EXTENDED_CONFIG_REQ_V01
*/
void update_default_qmi_ims_settings_get_reg_mgr_extended_config_req_v01(void);

/*
QMI_IMS_SETTINGS_GET_REG_MGR_EXTENDED_CONFIG_RSP_V01
*/
void update_default_qmi_ims_settings_get_reg_mgr_extended_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error);
void set_qmi_ims_settings_get_reg_mgr_extended_config_rsp_v01_settings_resp(uint8_t val);
void set_qmi_ims_settings_get_reg_mgr_extended_config_rsp_v01_t_delay(uint32_t val);


/*
  QMI_IMS_SETTINGS_SET_REG_MGR_EXTENDED_CONFIG_REQ_V01
*/
void update_default_qmi_ims_settings_set_reg_mgr_extended_config_req_v01(void);
void set_qmi_ims_settings_set_reg_mgr_extended_config_req_v01_t_delay(uint32_t val);

/*
QMI_IMS_SETTINGS_SET_REG_MGR_EXTENDED_CONFIG_RSP_V01
*/
void update_default_qmi_ims_settings_set_reg_mgr_extended_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error);
void set_qmi_ims_settings_set_reg_mgr_extended_config_rsp_v01_settings_resp(uint8_t val);

//============================================================================================

/*
QMI_IMS_SETTINGS_GET_USER_CONFIG_REQ_V01
*/
void update_default_qmi_ims_settings_get_user_config_req_v01(void);

/*
QMI_IMS_SETTINGS_GET_USER_CONFIG_RSP_V01
*/
void update_default_qmi_ims_settings_get_user_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error);
void set_qmi_ims_settings_get_user_config_rsp_v01_settings_resp(uint8_t val);
void set_qmi_ims_settings_get_user_config_rsp_v01_domain_name(char* val);


/*
  QMI_IMS_SETTINGS_SET_USER_CONFIG_REQ_V01
*/
void update_default_qmi_ims_settings_set_user_config_req_v01(void);
void set_qmi_ims_settings_set_user_config_req_v01_domain_name(char* val);

/*
QMI_IMS_SETTINGS_SET_USER_CONFIG_RSP_V01
*/
void update_default_qmi_ims_settings_set_user_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error);
void set_qmi_ims_settings_set_user_config_rsp_v01_settings_resp(uint8_t val);

//============================================================================================

/*
QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_REQ_V01
*/
//void update_default_qmi_ims_settings_get_client_provisioning_config_req_v01(void);

/*
QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_RSP_V01
*/
//void update_default_qmi_ims_settings_get_client_provisioning_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error);



/*
  QMI_IMS_SETTINGS_SET_CLIENT_PROVISIONING_CONFIG_REQ_V01
*/
void update_default_qmi_ims_settings_set_client_provisioning_config_req_v01(void);
void set_qmi_ims_settings_set_client_provisioning_config_req_v01_enable_volte(uint8_t val);
void set_qmi_ims_settings_set_client_provisioning_config_req_v01_enable_vt(uint8_t val);
void set_qmi_ims_settings_set_client_provisioning_config_req_v01_enable_presence(uint8_t val);
void set_qmi_ims_settings_set_client_provisioning_config_req_v01_wifi_call(ims_settings_wfc_status_enum_v01 val);
void set_qmi_ims_settings_set_client_provisioning_config_req_v01_wifi_call_roaming(ims_settings_wfc_roaming_enum_v01 val);
void set_qmi_ims_settings_set_client_provisioning_config_req_v01_wifi_call_preference(ims_settings_wfc_preference_v01 val);


/*
QMI_IMS_SETTINGS_SET_CLIENT_PROVISIONING_CONFIG_RSP_V01
*/
void update_default_qmi_ims_settings_set_client_provisioning_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error);
void set_qmi_ims_settings_set_client_provisioning_config_rsp_v01_settings_resp(uint8_t val);

//============================================================================================

/*
QMI_IMS_SETTINGS_GET_SMS_CONFIG_REQ_V01
*/
void update_default_qmi_ims_settings_get_sms_config_req_v01(void);

/*
QMI_IMS_SETTINGS_GET_SMS_CONFIG_RSP_V01
*/
void update_default_qmi_ims_settings_get_sms_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error);
void set_qmi_ims_settings_get_sms_config_rsp_v01_settings_resp(uint8_t val);
void set_qmi_ims_settings_get_sms_config_rsp_v01_sms_format(ims_settings_sms_format_enum_v01 val);
void set_qmi_ims_settings_get_sms_config_rsp_v01_sms_over_ip_network_indication(uint8_t val);


/*
  QMI_IMS_SETTINGS_SET_SMS_CONFIG_REQ_V01
*/
void update_default_qmi_ims_settings_set_sms_config_req_v01(void);
void set_qmi_ims_settings_set_sms_config_req_v01_sms_format(ims_settings_sms_format_enum_v01 val);
void set_qmi_ims_settings_set_sms_config_req_v01_sms_over_ip_network_indication(uint8_t val);

/*
QMI_IMS_SETTINGS_SET_SMS_CONFIG_RSP_V01
*/
void update_default_qmi_ims_settings_set_sms_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error);
void set_qmi_ims_settings_set_sms_config_rsp_v01_settings_resp(uint8_t val);

//============================================================================================

/*
QMI_IMS_SETTINGS_GET_PRESENCE_CONFIG_REQ_V01
*/
void update_default_qmi_ims_settings_get_presence_config_req_v01(void);

/*
QMI_IMS_SETTINGS_GET_PRESENCE_CONFIG_RSP_V01
*/
void update_default_qmi_ims_settings_get_presence_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error);
void set_qmi_ims_settings_get_presence_config_rsp_v01_settings_resp(uint8_t val);
void set_qmi_ims_settings_get_presence_config_rsp_v01_publish_expiry_timer(uint32_t val);
void set_qmi_ims_settings_get_presence_config_rsp_v01_publish_extended_expiry_timer(uint32_t val);
void set_qmi_ims_settings_get_presence_config_rsp_v01_capabilites_cache_expiration(uint32_t val);
void set_qmi_ims_settings_get_presence_config_rsp_v01_availability_cache_expiration(uint32_t val);
void set_qmi_ims_settings_get_presence_config_rsp_v01_capability_poll_interval(uint32_t val);
void set_qmi_ims_settings_get_presence_config_rsp_v01_minimum_publish_interval(uint32_t val);
void set_qmi_ims_settings_get_presence_config_rsp_v01_max_subcription_list_entries(uint32_t val);
void set_qmi_ims_settings_get_presence_config_rsp_v01_capability_poll_list_subscription_expiry_timer(uint32_t val);
void set_qmi_ims_settings_get_presence_config_rsp_v01_gzip_enabled(uint8_t val);


/*
  QMI_IMS_SETTINGS_SET_PRESENCE_CONFIG_REQ_V01
*/
void update_default_qmi_ims_settings_set_presence_config_req_v01(void);
void set_qmi_ims_settings_set_presence_config_req_v01_publish_expiry_timer(uint32_t val);
void set_qmi_ims_settings_set_presence_config_req_v01_publish_extended_expiry_timer(uint32_t val);
void set_qmi_ims_settings_set_presence_config_req_v01_capabilites_cache_expiration(uint32_t val);
void set_qmi_ims_settings_set_presence_config_req_v01_availability_cache_expiration(uint32_t val);
void set_qmi_ims_settings_set_presence_config_req_v01_capability_poll_interval(uint32_t val);
void set_qmi_ims_settings_set_presence_config_req_v01_minimum_publish_interval(uint32_t val);
void set_qmi_ims_settings_set_presence_config_req_v01_max_subcription_list_entries(uint32_t val);
void set_qmi_ims_settings_set_presence_config_req_v01_capability_poll_list_subscription_expiry_timer(uint32_t val);
void set_qmi_ims_settings_set_presence_config_req_v01_gzip_enabled(uint8_t val);

/*
QMI_IMS_SETTINGS_SET_PRESENCE_CONFIG_RSP_V01
*/
void update_default_qmi_ims_settings_set_presence_config_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error);
void set_qmi_ims_settings_set_presence_config_rsp_v01_settings_resp(uint8_t val);

/*
QMI_IMS_SETTINGS_GET_IMS_SERVICE_ENABLE_CONFIG_REQ_V01
*/
void update_default_qmi_ims_settings_get_ims_service_enable_config_req(void);

/*
QMI_IMS_SETTINGS_GET_IMS_SERVICE_ENABLE_CONFIG_RSP_V01
*/
ims_settings_get_ims_service_enable_config_rsp_msg_v01 *update_default_qmi_ims_settings_get_ims_service_enable_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error, ims_settings_get_ims_service_enable_config_rsp_msg_v01 *resp=nullptr);
void set_qmi_ims_settings_get_ims_service_enable_config_resp_volte_enabled(uint8_t val);
void set_qmi_ims_settings_get_ims_service_enable_config_resp_videotelephony_enabled(uint8_t val);
void set_qmi_ims_settings_get_ims_service_enable_config_resp_ims_service_enabled(uint8_t val);
void set_qmi_ims_settings_get_ims_service_enable_config_resp_wifi_calling_enabled(uint8_t val);
void set_qmi_ims_settings_get_ims_service_enable_config_resp_call_mode_preference(ims_settings_call_mode_option_enum_v01 val);

/*
  QMI_IMS_SETTINGS_SET_IMS_SERVICE_ENABLE_CONFIG_REQ_V01
*/
ims_settings_set_ims_service_enable_config_req_msg_v01 *update_default_qmi_ims_settings_set_ims_service_enable_config_req(ims_settings_set_ims_service_enable_config_req_msg_v01 *req=nullptr);
void set_qmi_ims_settings_set_ims_service_enable_config_req_volte_enabled(uint8_t val);
void set_qmi_ims_settings_set_ims_service_enable_config_req_videotelephony_enabled(uint8_t val);
void set_qmi_ims_settings_set_ims_service_enable_config_req_ut_enabled(uint8_t val);
void set_qmi_ims_settings_set_ims_service_enable_config_req_ims_service_enabled(uint8_t val);
void set_qmi_ims_settings_set_ims_service_enable_config_req_wifi_calling_enabled(uint8_t val);
void set_qmi_ims_settings_set_ims_service_enable_config_req_call_mode_preference(ims_settings_call_mode_option_enum_v01 val);

/*
  QMI_IMS_SETTINGS_SET_IMS_SERVICE_ENABLE_CONFIG_RSP_V01
*/
ims_settings_set_ims_service_enable_config_rsp_msg_v01 *update_default_qmi_ims_settings_set_ims_service_enable_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_IMS_SETTINGS_SET_CODEC_DYNAMIC_CONFIG_REQ_V01
*/
ims_settings_set_codec_dynamic_config_req_msg_v01 *update_default_qmi_ims_settings_set_codec_dynamic_config_req(ims_settings_set_codec_dynamic_config_req_msg_v01 *req=nullptr);
void set_qmi_ims_settings_set_codec_dynamic_config_req_amr_mode_set_mask(ims_settings_amr_nb_codec_mode_type_mask_v01 val);

/*
  QMI_IMS_SETTINGS_SET_CODEC_DYNAMIC_CONFIG_RSP_V01
*/
ims_settings_set_codec_dynamic_config_rsp_msg_v01 *update_default_qmi_ims_settings_set_codec_dynamic_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_IMS_SETTINGS_GET_CODEC_DYNAMIC_CONFIG_REQ_V01
*/
void update_default_qmi_ims_settings_get_codec_dynamic_config_req(void);

/*
QMI_IMS_SETTINGS_GET_CODEC_DYNAMIC_CONFIG_RSP_V01
*/
ims_settings_get_codec_dynamic_config_rsp_msg_v01  *update_default_qmi_ims_settings_get_codec_dynamic_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error, ims_settings_get_codec_dynamic_config_rsp_msg_v01 *resp=nullptr);
void set_qmi_ims_settings_get_codec_dynamic_config_resp_amr_mode_set_mask(ims_settings_amr_nb_codec_mode_type_mask_v01 val);

/*
  QMI_IMS_SETTINGS_SET_VOICE_DYNAMIC_CONFIG_REQ_V01
*/
ims_settings_set_voice_dynamic_config_req_msg_v01 *update_default_qmi_ims_settings_set_voice_dynamic_config_req(ims_settings_set_voice_dynamic_config_req_msg_v01 *req=nullptr);

/*
  QMI_IMS_SETTINGS_SET_VOICE_DYNAMIC_CONFIG_RSP_V01
*/
ims_settings_set_voice_dynamic_config_rsp_msg_v01 *update_default_qmi_ims_settings_set_voice_dynamic_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_IMS_SETTINGS_GET_VOICE_DYNAMIC_CONFIG_REQ_V01
*/
void update_default_qmi_ims_settings_get_voice_dynamic_config_req(void);

/*
  QMI_IMS_SETTINGS_GET_VOICE_DYNAMIC_CONFIG_RSP_V01
*/
ims_settings_get_voice_dynamic_config_rsp_msg_v01  *update_default_qmi_ims_settings_get_voice_dynamic_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error, ims_settings_get_voice_dynamic_config_rsp_msg_v01 *resp=nullptr);

/*
  QMI_IMS_SETTINGS_SET_REG_MGR_NEW_CONFIG_REQ_V01
*/
ims_settings_set_reg_mgr_new_config_req_msg_v01 *update_default_qmi_ims_settings_set_reg_mgr_new_config_req(ims_settings_set_reg_mgr_new_config_req_msg_v01 *req=nullptr);
/*
  QMI_IMS_SETTINGS_SET_REG_MGR_NEW_CONFIG_RSP_V01
*/
ims_settings_set_reg_mgr_new_config_rsp_msg_v01 *update_default_qmi_ims_settings_set_reg_mgr_new_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);
/*
  QMI_IMS_SETTINGS_GET_REG_MGR_NEW_CONFIG_REQ_V01
*/
void update_default_qmi_ims_settings_get_reg_mgr_new_config_req(void);
/*
  QMI_IMS_SETTINGS_GET_REG_MGR_NEW_CONFIG_RSP_V01
*/
ims_settings_get_reg_mgr_new_config_rsp_msg_v01 *update_default_qmi_ims_settings_get_reg_mgr_new_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error, ims_settings_get_reg_mgr_new_config_rsp_msg_v01 *resp=nullptr);
/*
  QMI_IMS_SETTINGS_SET_SMS_NEW_CONFIG_REQ_V01
*/
ims_settings_set_sms_new_config_req_msg_v01 *update_default_qmi_ims_settings_set_sms_new_config_req(ims_settings_set_sms_new_config_req_msg_v01 *req=nullptr);
/*
  QMI_IMS_SETTINGS_SET_SMS_NEW_CONFIG_RSP_V01
*/
ims_settings_set_sms_new_config_rsp_msg_v01 *update_default_qmi_ims_settings_set_sms_new_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);
/*
  QMI_IMS_SETTINGS_GET_SMS_NEW_CONFIG_REQ_V01
*/
void update_default_qmi_ims_settings_get_sms_new_config_req(void);
/*
  QMI_IMS_SETTINGS_GET_SMS_NEW_CONFIG_RSP_V01
*/
ims_settings_get_sms_new_config_rsp_msg_v01 *update_default_qmi_ims_settings_get_sms_new_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error, ims_settings_get_sms_new_config_rsp_msg_v01 *resp=nullptr);
/*
  QMI_IMS_SETTINGS_SET_RTP_DYNAMIC_CONFIG_REQ_V01
*/
ims_settings_set_rtp_dynamic_config_req_msg_v01 *update_default_qmi_ims_settings_set_rtp_dynamic_config_req(ims_settings_set_rtp_dynamic_config_req_msg_v01 *req = nullptr);
/*
  QMI_IMS_SETTINGS_SET_RTP_DYNAMIC_CONFIG_RSP_V01
*/
ims_settings_set_rtp_dynamic_config_rsp_msg_v01 *update_default_qmi_ims_settings_set_rtp_dynamic_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);
/*
  QMI_IMS_SETTINGS_GET_RTP_DYNAMIC_CONFIG_REQ_V01
*/
void update_default_qmi_ims_settings_get_rtp_dynamic_config_req(void);
/*
  QMI_IMS_SETTINGS_GET_RTP_DYNAMIC_CONFIG_RSP_V01
*/
ims_settings_get_rtp_dynamic_config_rsp_msg_v01 *update_default_qmi_ims_settings_get_rtp_dynamic_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error, ims_settings_get_rtp_dynamic_config_rsp_msg_v01 *resp=nullptr);


/*========================================================
  QMI_IMS_SETTINGS_SET_SIP_NEW_CONFIG_REQ_V01
========================================================*/
ims_settings_set_sip_new_config_req_msg_v01 *update_default_qmi_ims_settings_set_sip_new_config_req(ims_settings_set_sip_new_config_req_msg_v01 *req=nullptr);
/*========================================================
  QMI_IMS_SETTINGS_SET_SIP_NEW_CONFIG_RSP_V01
========================================================*/
ims_settings_set_sip_new_config_rsp_msg_v01 *update_default_qmi_ims_settings_set_sip_new_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);
/*========================================================
  QMI_IMS_SETTINGS_GET_SIP_NEW_CONFIG_REQ_V01
========================================================*/
void update_default_qmi_ims_settings_get_sip_new_config_req(void);
/*========================================================
  QMI_IMS_SETTINGS_GET_SIP_NEW_CONFIG_RSP_V01
========================================================*/
ims_settings_get_sip_new_config_rsp_msg_v01 *update_default_qmi_ims_settings_get_sip_new_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error, ims_settings_get_sip_new_config_rsp_msg_v01 *resp=nullptr);

/*========================================================
  QMI_IMS_SETTINGS_SET_PRESENCE_NEW_CONFIG_REQ_V01
========================================================*/
ims_settings_set_presence_new_config_req_msg_v01 *update_default_qmi_ims_settings_set_presence_new_config_req(ims_settings_set_presence_new_config_req_msg_v01 *req=nullptr);
/*========================================================
  QMI_IMS_SETTINGS_SET_PRESENCE_NEW_CONFIG_RSP_V01
========================================================*/
ims_settings_set_presence_new_config_rsp_msg_v01 *update_default_qmi_ims_settings_set_presence_new_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);
/*========================================================
  QMI_IMS_SETTINGS_GET_PRESENCE_NEW_CONFIG_REQ_V01
========================================================*/
void update_default_qmi_ims_settings_get_presence_new_config_req(void);
/*========================================================
  QMI_IMS_SETTINGS_GET_PRESENCE_NEW_CONFIG_RSP_V01
========================================================*/
ims_settings_get_presence_new_config_rsp_msg_v01 *update_default_qmi_ims_settings_get_presence_new_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error, ims_settings_get_presence_new_config_rsp_msg_v01 *resp=nullptr);

/*========================================================
  QMI_IMS_SETTINGS_SET_HANDOVER_CONFIG_REQ_V01
========================================================*/
ims_settings_set_handover_config_req_msg_v01 *update_default_qmi_ims_settings_set_handover_config_req(ims_settings_set_handover_config_req_msg_v01 *req=nullptr);
/*========================================================
  QMI_IMS_SETTINGS_SET_HANDOVER_CONFIG_RSP_V01
========================================================*/
ims_settings_set_handover_config_rsp_msg_v01 *update_default_qmi_ims_settings_set_handover_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);
/*========================================================
  QMI_IMS_SETTINGS_GET_HANDOVER_CONFIG_REQ_V01
========================================================*/
void update_default_qmi_ims_settings_get_handover_config_req(void);
/*========================================================
  QMI_IMS_SETTINGS_GET_HANDOVER_CONFIG_RSP_V01
========================================================*/
ims_settings_get_handover_config_rsp_msg_v01 *update_default_qmi_ims_settings_get_handover_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error, ims_settings_get_handover_config_rsp_msg_v01 *resp=nullptr);

/*
   QMI_IMS_SETTINGS_SET_IMS_NEW_CONFIG_REQ_V01
*/
ims_settings_set_ims_new_config_req_msg_v01 *update_default_qmi_ims_settings_set_ims_new_config_req(ims_settings_set_ims_new_config_req_msg_v01 *req=nullptr);
/*
   QMI_IMS_SETTINGS_SET_IMS_NEW_CONFIG_RSP_V01
*/
ims_settings_set_ims_new_config_rsp_msg_v01 *update_default_qmi_ims_settings_set_ims_new_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);
/*
   QMI_IMS_SETTINGS_GET_IMS_NEW_CONFIG_REQ_V01
*/
void update_default_qmi_ims_settings_get_ims_new_config_req();
/*
   QMI_IMS_SETTINGS_GET_IMS_NEW_CONFIG_RSP_V01
*/
ims_settings_get_ims_new_config_rsp_msg_v01 *update_default_qmi_ims_settings_get_ims_new_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error, ims_settings_get_ims_new_config_rsp_msg_v01 *resp=nullptr);
/*
  QMI_IMS_SETTINGS_SET_EMER_DYNAMIC_CONFIG_REQ_V01
*/
ims_settings_set_emer_dynamic_config_req_msg_v01 *update_default_qmi_ims_settings_set_emer_dynamic_config_req(ims_settings_set_emer_dynamic_config_req_msg_v01 *req=nullptr);
/*
  QMI_IMS_SETTINGS_SET_EMER_DYNAMIC_CONFIG_RSP_V01
*/
ims_settings_set_emer_dynamic_config_rsp_msg_v01 *update_default_qmi_ims_settings_set_emer_dynamic_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);
/*
  QMI_IMS_SETTINGS_GET_EMER_DYNAMIC_CONFIG_REQ_V01
*/
void update_default_qmi_ims_settings_get_emer_dynamic_config_req(void);
/*
  QMI_IMS_SETTINGS_GET_EMER_DYNAMIC_CONFIG_RSP_V01
*/
ims_settings_get_emer_dynamic_config_rsp_msg_v01 *update_default_qmi_ims_settings_get_emer_dynamic_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error, ims_settings_get_emer_dynamic_config_rsp_msg_v01 *resp=nullptr);
/*
  QMI_IMS_SETTINGS_SET_VIDEO_DYNAMIC_CONFIG_REQ_V01
*/
ims_settings_set_video_dynamic_config_req_msg_v01 *update_default_qmi_ims_settings_set_video_dynamic_config_req(ims_settings_set_video_dynamic_config_req_msg_v01 *req=nullptr);
/*
  QMI_IMS_SETTINGS_SET_VIDEO_DYNAMIC_CONFIG_RSP_V01
*/
ims_settings_set_video_dynamic_config_rsp_msg_v01 *update_default_qmi_ims_settings_set_video_dynamic_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);
/*
  QMI_IMS_SETTINGS_GET_VIDEO_DYNAMIC_CONFIG_REQ_V01
*/
void update_default_qmi_ims_settings_get_video_dynamic_config_req(void);
/*
  QMI_IMS_SETTINGS_GET_VIDEO_DYNAMIC_CONFIG_RSP_V01
*/
ims_settings_get_video_dynamic_config_rsp_msg_v01 *update_default_qmi_ims_settings_get_video_dynamic_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error, ims_settings_get_video_dynamic_config_rsp_msg_v01 *resp=nullptr);
/*
  QMI_IMS_SETTINGS_SET_RCS_SM_NEW_CONFIG_REQ_V01
*/
ims_settings_set_rcs_new_config_req_msg_v01 *update_default_qmi_ims_settings_set_rcs_new_config_req(ims_settings_set_rcs_new_config_req_msg_v01 *req=nullptr);
/*
  QMI_IMS_SETTINGS_SET_RCS_SM_NEW_CONFIG_RSP_V01
*/
ims_settings_set_rcs_new_config_rsp_msg_v01 *update_default_qmi_ims_settings_set_rcs_new_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);
/*
  QMI_IMS_SETTINGS_GET_RCS_SM_NEW_CONFIG_REQ_V01
*/
void update_default_qmi_ims_settings_get_rcs_new_config_req(void);
/*
  QMI_IMS_SETTINGS_GET_RCS_SM_NEW_CONFIG_RSP_V01
*/
ims_settings_get_rcs_new_config_rsp_msg_v01 *update_default_qmi_ims_settings_get_rcs_new_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error, ims_settings_get_rcs_new_config_rsp_msg_v01 *resp=nullptr);
/*
  QMI_IMS_SETTINGS_SET_CONCURRENCY_CONFIG_REQ_V01
*/
ims_settings_set_concurrency_config_req_msg_v01 *update_default_qmi_ims_settings_set_concurrency_config_req(ims_settings_set_concurrency_config_req_msg_v01 *req=nullptr);
/*
  QMI_IMS_SETTINGS_SET_CONCURRENCY_CONFIG_RSP_V01
*/
ims_settings_set_concurrency_config_rsp_msg_v01 *update_default_qmi_ims_settings_set_concurrency_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);
#if 0
/*
  QMI_IMS_SETTINGS_GET_CONCURRENCY_CONFIG_REQ_V01
*/
void update_default_qmi_ims_settings_get_concurrency_config_req(void);
/*
  QMI_IMS_SETTINGS_GET_CONCURRENCY_CONFIG_RSP_V01
*/
ims_settings_get_concurrency_config_rsp_msg_v01 *update_default_qmi_ims_settings_get_concurrency_config_resp(qmi_result_type_v01 result, qmi_error_type_v01 error, ims_settings_get_concurrency_config_rsp_msg_v01 *resp=nullptr);
#endif


/*
QMI_IMS_SETTINGS_SEND_RTT_REQ_V01
*/
void update_default_qmi_ims_settings_send_rtt_message_req(void);
void set_qmi_ims_settings_set_ims_service_rtt_message(const char* val);
/*
QMI_IMS_SETTINGS_SEND_RTT_RESP_V01
*/
void update_default_qmi_ims_settings_set_ims_service_send_rtt_message_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_IMS_SETTINGS_RTT_RCVD_DATA_IND_V01
*/
void update_default_qmi_imss_rtt_message_ind_v01();

/*
  QMI_IMS_SETTINGS_IMS_SERVICE_ENABLE_CONFIG_IND_V01
*/
ims_settings_ims_service_enable_config_ind_msg_v01 *update_default_qmi_ims_settings_ims_service_enable_config_ind_msg();

/*
QMI_IMS_SETTINGS_SET_DIGITS_REQ_V01
*/
void update_default_qmi_ims_settings_set_digits_req(ims_settings_digits_feature_status_v01 is_digits_enabled);
void set_qmi_ims_settings_set_digits_req_no_of_lines(uint8_t val);
ims_settings_digits_line_info_v01 *set_qmi_ims_settings_set_digits_req_line_info_list(uint32_t line_info_list_len);
/*
QMI_IMS_SETTINGS_SET_DIGITS_REQ_V01
*/
void update_default_qmi_ims_settings_set_digits_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);

//============================================================================================

/*
  QMI_DMS_GET_DEVICE_MFR_REQ_V01
*/
void update_default_qmi_dms_get_device_mfr_req_msg();
/*
  QMI_DMS_GET_DEVICE_MFR_RESP_V01
*/
char* update_default_qmi_dms_get_device_mfr_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
/*
  QMI_DMS_BIND_SUBSCRIPTION_REQ_V01
*/
void update_default_qmi_dms_bind_subscription_req_msg(dms_bind_subscription_enum_v01 bind_subs);
/*
  QMI_DMS_BIND_SUBSCRIPTION_RESP_V01
*/
void update_default_qmi_dms_bind_subscription_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
/*
  QMI_DMS_GET_DEVICE_CAP_REQ_V01
*/
/*
  QMI_DMS_GET_DEVICE_CAP_RESP_V01
*/
dms_device_capabilities_type_v01* update_default_qmi_dms_get_device_cap_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);

/*
  QMI_DMS_SET_EVENT_REPORT_REQ_V01
*/
  /*
void update_default_qmi_dms_set_event_report_req_msg();
void set_qmi_dms_set_event_report_req_report_oprt_mode_state(int val);
void set_qmi_dms_set_event_report_req_report_prl_init(int val);
*/
/*
  QMI_DMS_SET_EVENT_REPORT_RESP_V01
*/
  //implemented above already...
/*
  QMI_DMS_GET_CURRENT_PRL_INFO_REQ_V01
*/
/*
  QMI_DMS_GET_CURRENT_PRL_INFO_RESP_V01
*/
void update_default_qmi_dms_get_current_prl_info_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
void set_qmi_dms_get_current_prl_info_resp_prl_version(uint16_t val);

/*
  QMI_DMS_SET_OPERATING_MODE_REQ_V01
*/
void update_default_qmi_dms_set_operating_mode_req_msg(dms_operating_mode_enum_v01 operating_mode);
/*
  QMI_DMS_SET_OPERATING_MODE_RESP_V01
*/
void update_default_qmi_dms_set_operating_mode_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
/*
  QMI_DMS_GET_OPERATING_MODE_REQ_V01
*/
/*
  QMI_DMS_GET_OPERATING_MODE_RESP_V01
*/
void update_default_qmi_dms_get_operating_mode_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error, dms_operating_mode_enum_v01 operating_mode);
/*
  QMI_DMS_GET_BAND_CAPABILITY_REQ_V01
*/
void update_default_qmi_dms_get_band_capability_req_msg();
/*
  QMI_DMS_GET_BAND_CAPABILITY_RESP_V01
*/
void update_default_qmi_dms_get_band_capability_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error, uint64_t band_capability);
/*
  QMI_DMS_GET_MSISDN_REQ_V01
*/
/*
  QMI_DMS_GET_MSISDN_RESP_V01
*/
char* update_default_qmi_dms_get_msisdn_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
char* set_qmi_dms_get_msisdn_resp_imsi();
/*
  QMI_DMS_UIM_GET_ICCID_REQ_V01
*/
void update_default_qmi_dms_get_iccid_req();
/*
  QMI_DMS_UIM_GET_ICCID_RESP_V01
*/
char* update_default_qmi_dms_get_iccid_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);

/*
  QMI_DMS_GET_DEVICE_SERIAL_NUMBERS_REQ_V01
*/
void update_default_qmi_dms_get_device_serial_numbers_req();

/*
  QMI_DMS_GET_DEVICE_SERIAL_NUMBERS_RESP_V01
*/
void update_deafult_qmi_dms_get_device_serial_numbers_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
char* set_qmi_dms_get_device_serial_numbers_resp_esn();
char* set_qmi_dms_get_device_serial_numbers_resp_imei();
char* set_qmi_dms_get_device_serial_numbers_resp_meid();
char* set_qmi_dms_get_device_serial_numbers_resp_imeisv_svn();
/*
  QMI_DMS_SET_AP_SW_VERSION_REQ_V01
*/
void update_default_qmi_dms_set_ap_sw_version_req_v01(void);
/*
  QMI_DMS_SET_AP_SW_VERSION_RESP_V01
*/
void update_default_qmi_dms_set_ap_sw_version_resp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error);
/*
  QMI_DMS_GET_SW_VERSION_REQ_V01
*/
void update_default_qmi_dms_get_sw_version_req_v01(void);
/*
  QMI_DMS_GET_SW_VERSION_RESP_V01
*/
char* update_default_qmi_dms_get_sw_version_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);

/*
  QMI_DMS_SET_AP_VERSION_REQ_V01
*/
void update_default_qmi_dms_set_ap_version_req_v01(void);
/*
  QMI_DMS_SET_AP_VERSION_RESP_V01
*/
void update_default_qmi_dms_set_ap_version_resp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_DMS_INDICATION_REGISTER_REQ_V01
*/
void update_default_qmi_dms_indication_register_req_v01(void);
void set_qmi_dms_ind_req_report_ims_capability(uint8_t ims_capability);

/*
  QMI_DMS_INDICATION_REGISTER_RESP_V01
*/
void update_default_qmi_dms_indication_register_resp_v01(void);
void set_qmi_dms_indication_register_resp_msg_v01_result
  (qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_WMS_RAW_SEND_REQ_V01
*/
#ifndef RIL_UTF_IOE
void update_default_qmi_wms_raw_send_req_msg(char* smsc_address, char *message, wms_message_format_enum_v01 format);
void update_default_qmi_wms_raw_send_req_msg_3gpp2(RIL_CDMA_SMS_Message *cdma_sms_msg, wms_message_format_enum_v01 format, uint8_t sms_on_ims);
#else
wms_send_raw_message_data_type_v01* update_default_qmi_wms_raw_send_req_msg();
#endif
void set_qmi_wms_raw_send_req_link_timer(uint8_t link_timer);
void set_qmi_wms_raw_send_req_sms_on_ims(uint8_t sms_on_ims);
void set_qmi_wms_raw_send_req_retry_message(wms_retry_message_enum_v01 retry_message);
void set_qmi_wms_raw_send_req_retry_message_id(uint32_t retry_message_id);

/*
  QMI_WMS_RAW_SEND_RESP_V01
*/
wms_raw_send_resp_msg_v01* update_default_qmi_wms_raw_send_resp_msg(
        qmi_result_type_v01 result, qmi_error_type_v01 error, uint16_t message_id);
void set_qmi_wms_raw_send_resp_cause_code(wms_tl_cause_code_enum_v01 cause_code);
wms_gw_cause_info_type_v01* set_qmi_wms_raw_send_resp_gw_cause_info();
void set_qmi_wms_raw_send_resp_error_class(wms_error_class_send_enum_v01 error_class);
void set_qmi_wms_raw_send_resp_message_delivery_failure_type(wms_message_delivery_failure_type_enum_v01 message_delivery_failure_type);
void set_call_control_flag_in_raw_send_resp_mag(uint8_t flag);
void set_call_control_alpha_text_in_raw_send_resp_mag(uint8_t text);

/*
  QMI_WMS_GET_SMSC_ADDRESS_REQ_V01
*/
void update_default_qmi_wms_get_smsc_address_req();
/*
  QMI_WMS_GET_SMSC_ADDRESS_RESP_V01
*/
wms_smsc_address_type_v01* update_default_qmi_wms_get_smsc_address_resp(qmi_result_type_v01 result,qmi_error_type_v01 error);

/*
  QMI_WMS_SET_SMSC_ADDRESS_REQ_V01
*/
void update_deafult_qmi_wms_set_smsc_address_req_msg(char *val);
void set_qmi_wms_set_smsc_address_req_msg_smsc_address_type(char *val);
/*
  QMI_WMS_SET_SMSC_ADDRESS_RESP_V01
*/
void update_default_qmi_wms_set_smsc_address_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
/*
  QMI_WMS_SET_MEMORY_STATUS_REQ_V01
*/
void update_default_qmi_wms_set_memory_status_req_msg(uint8_t memory_available);
/*
  QMI_WMS_SET_MEMORY_STATUS_RESP_V01
*/
void update_default_qmi_wms_set_memory_status_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
/*
  QMI_WMS_SEND_ACK_REQ_V01
*/
wms_send_ack_req_msg_v01* update_default_qmi_wms_send_ack_req_msg();
wms_3gpp_failure_information_type_v01* set_qmi_wms_send_ack_req_wms_3gpp_failure_information();
void set_qmi_wms_send_ack_req_sms_on_ims(uint8_t sms_on_ims);
wms_3gpp2_failure_information_type_v01* set_qmi_wms_send_ack_req_wms_3gpp2_failure_information();

/*
  QMI_WMS_SERVICE_READY_IND_V01
*/
void update_default_qmi_wms_service_ready_ind(wms_service_ready_status_enum_v01 ready_status);

/*
  QMI_WMS_SEND_ACK_RESP_V01
*/
void update_default_qmi_wms_send_ack_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);

/*
QMI_WMS_BIND_SUBSCRIPTION_REQ_V01
*/
void update_default_qmi_wms_bind_subscription_req(wms_subscription_type_enum_v01 subs_type);
/*
QMI_WMS_BIND_SUBSCRIPTION_RESP_V01
*/
void update_default_qmi_wms_bind_subscription_resp(qmi_result_type_v01 result,qmi_error_type_v01 error);

/*
  QMI_WMS_DELETE_REQ_V01
*/
void update_default_qmi_wms_delete_req_msg(wms_storage_type_enum_v01 storage_type);
void set_qmi_wms_delete_req_index(uint32_t index);
void set_qmi_wms_delete_req_message_mode(wms_message_mode_enum_v01 message_mode);
/*
  QMI_WMS_DELETE_RESP_V01
*/
void update_default_qmi_wms_delete_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);

/*
  QMI_WMS_GET_BROADCAST_CONFIG_REQ_V01
*/
void update_default_qmi_wms_get_broadcast_config_req_msg(wms_message_mode_enum_v01 message_mode);
/*
  QMI_WMS_GET_BROADCAST_CONFIG_RESP_V01
*/
void update_default_qmi_wms_get_broadcast_config_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
void set_wms_get_broadcast_config_resp(wms_get_broadcast_config_resp_msg_v01 &in);

/*
  QMI_WMS_SET_BROADCAST_CONFIG_REQ_V01
*/
void update_deafult_qmi_wms_set_broadcast_config_req_msg(wms_message_mode_enum_v01 message_mode);
wms_3gpp_broadcast_config_info_type_v01* set_qmi_wms_set_broadcast_config_req_wms_3gpp_broadcast_config_info(uint32_t wms_3gpp_broadcast_config_info_len);
wms_3gpp2_broadcast_config_info_type_v01* set_qmi_wms_set_broadcast_config_req_wms_3gpp2_broadcast_config_info(uint32_t wms_3gpp2_broadcast_config_info_len);
void set_wms_set_broadcast_config_req(wms_set_broadcast_config_req_msg_v01 &in);

/*
  QMI_WMS_SET_BROADCAST_CONFIG_RESP_V01
*/
void update_default_qmi_wms_set_broadcast_config_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
/*
  QMI_WMS_SET_BROADCAST_ACTIVATION_REQ_V01
*/
wms_broadcast_activation_info_type_v01* update_default_qmi_wms_set_broadcast_activation_req_msg();
void set_qmi_wms_set_activation_req_msg_activate_all(uint8_t val);
/*
  QMI_WMS_SET_BROADCAST_ACTIVATION_RESP_V01
*/
void update_qmi_wms_set_broadcast_activation_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);

/*
  QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ_V01
*/

/*
  QMI_WMS_GET_TRANSPORT_LAYER_INFO_RESP_V01
*/
void update_default_qmi_wms_get_transport_layer_info_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
void set_qmi_wms_get_transport_layer_info_resp_registered_ind(uint8_t registered_ind);
/*
  QMI_WMS_SET_PRIMARY_CLIENT_REQ_V01
*/
void update_default_qmi_wms_set_primary_client_req_msg(uint8_t primary_client);
/*
  QMI_WMS_SET_PRIMARY_CLIENT_RESP_V01
*/
void update_default_qmi_wms_set_primary_client_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
/*
  QMI_WMS_SET_ROUTES_REQ_V01
*/
wms_set_route_list_tuple_type_v01* update_default_qmi_wms_set_routes_req_msg(uint32_t route_list_tuple_len);
void set_qmi_wms_set_routes_req_transfer_ind(wms_transfer_ind_enum_v01 transfer_ind);
/*
  QMI_WMS_SET_ROUTES_RESP_V01
*/
void update_default_qmi_wms_set_routes_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);

/*
  QMI_WMS_SET_EVENT_REPORT_REQ_V01
*/

void update_default_qmi_wms_set_event_report_req_msg();

void set_qmi_wms_set_event_report_report_mt_message(uint8_t report_mt_message);
void set_qmi_wms_set_event_report_report_mwi_message(uint8_t report_mwi_message);
/*
  QMI_WMS_SET_EVENT_REPORT_RESP_V01
*/
void update_default_qmi_wms_set_event_report_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
/*
  QMI_WMS_INDICATION_REGISTER_REQ_V01
*/
void update_default_qmi_wms_indication_register_req_msg();
void set_qmi_wms_indication_register_req_reg_transport_layer_info_events(uint8_t reg_transport_layer_info_events);
/*
  QMI_WMS_INDICATION_REGISTER_RESP_V01
*/
void update_default_qmi_wms_indication_register_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
/*
  QMI_WMS_RAW_WRITE_REQ_V01
*/
void update_default_qmi_wms_raw_write_req_msg
(char* smsc_address, char *message, wms_message_format_enum_v01 format, wms_message_tag_type_enum_v01 tag_type);

/*
  QMI_WMS_RAW_WRITE_RESP_V01
*/
void update_default_qmi_wms_raw_write_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error, uint32_t storage_index);
/*
  QMI_WMS_MODIFY_TAG_REQ_V01
*/
wms_message_tag_type_v01* update_default_qmi_wms_modify_tag_req_msg();
void set_qmi_wms_modify_tag_req_message_mode(wms_message_mode_enum_v01 message_mode);

/*
  QMI_WMS_MODIFY_TAG_RESP_V01
*/
void update_default_qmi_wms_modify_tag_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
/*
  QMI_WMS_RAW_READ_REQ_V01
*/
wms_message_memory_storage_identification_type_v01* update_default_qmi_wms_raw_read_req_msg();
void set_qmi_wms_raw_read_req_message_mode(wms_message_mode_enum_v01 message_mode);
/*
  QMI_WMS_RAW_READ_RESP_V01
*/
wms_read_raw_message_data_type_v01* update_default_qmi_wms_raw_read_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);

/*
  QMI_WMS_EVENT_REPORT_IND_V01
*/
void update_default_qmi_wms_event_report_ind_msg();
#ifndef RIL_UTF_IOE
void set_qmi_wms_event_report_ind_transfer_route_mt_message( wms_ack_indicator_enum_v01 ack_indicator,
  uint32_t transaction_id, wms_message_format_enum_v01 format, char* message);
#else
wms_transfer_route_mt_message_type_v01* set_qmi_wms_event_report_ind_transfer_route_mt_message();
#endif
void set_qmi_wms_event_report_ind_transfer_route_bc_message(wms_message_format_enum_v01 format,
  char* message);
void set_sms_rat(wms_bc_rat_info_enum_v01 rat);
void set_warning_area_coordinate(unsigned char* wac, size_t len);
void set_qmi_wms_event_report_ind_wac_message(char* message);
void set_qmi_wms_event_report_ind_message_mode(wms_message_mode_enum_v01 message_mode);
wms_mt_message_type_v01* set_qmi_wms_event_report_ind_mt_message();
void set_qmi_wms_event_report_ind_sms_on_ims(uint8_t sms_on_ims);
wms_etws_message_type_v01* set_qmi_wms_event_report_ind_etws_message();
void set_call_control_flag_in_event_report_ind_mag(uint8_t flag);
void set_call_control_alpha_text_in_event_report_ind_mag(uint8_t text);
/*
  QMI_WMS_MEMORY_FULL_IND_V01
*/
wms_memory_full_info_type_v01* update_default_qmi_wms_memory_full_ind_msg();

/*
  QMI_WMS_TRANSPORT_LAYER_INFO_IND_V01
*/
void update_default_qmi_wms_transport_layer_info_ind_msg(uint8_t registered_ind);

#ifndef UTF_TARGET_BUILD
/*
  QMI_WMS_TRANSPORT_LAYER_MWI_IND_V01
*/
void update_default_qmi_wms_transport_layer_mwi_ind_msg();
wms_mwi_summary_type_v01 *set_qmi_wms_transport_layer_mwi_ind_message_waiting_summary_info(uint32_t len);
void set_qmi_wms_transport_layer_mwi_ind_UE_address(char *val);
wms_mwi_detail_type_v01 *set_qmi_wms_transport_layer_mwi_ind_message_waiting_detail_info(uint32_t len);
#endif

/*
  QMI_PBM_BIND_SUBSCRIPTION_REQ_V01
*/
void update_default_qmi_pbm_bind_subscription_req_msg(pbm_subs_type_enum_v01 subs_type);
/*
  QMI_PBM_BIND_SUBSCRIPTION_RESP_V01
*/
void update_default_qmi_pbm_bind_subscription_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);

/*
  QMI_IMSA_IND_REG_REQ_V01
*/
void update_default_qmi_imsa_ind_reg_req_v01(void);

/*
  QMI_IMSA_IND_REG_RSP_V01
*/
void update_default_qmi_imsa_ind_reg_rsp_v01(void);
void set_qmi_imsa_ind_req_rsp_v01_result(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_IMSA_GET_SERVICE_STATUS_REQ_V01
*/
void update_default_qmi_imsa_get_service_status_req_v01(void);

/*
  QMI_IMSA_GET_SERVICE_STATUS_RSP_V01
*/
void update_default_qmi_imsa_get_service_status_rsp_v01(void);
void set_qmi_imsa_get_service_status_rsp_v01(qmi_result_type_v01 result, qmi_error_type_v01 error);
#ifndef RIL_UTF_IOE
void set_qmi_imsa_get_service_status_rsp_v01_sms_service_status(imsa_service_status_enum_v01 val);
void set_qmi_imsa_get_service_status_rsp_v01_voip_service_status(imsa_service_status_enum_v01 val);
void set_qmi_imsa_get_service_status_rsp_v01_vt_service_status(imsa_service_status_enum_v01 val);
void set_qmi_imsa_get_service_status_rsp_v01_sms_service_rat(imsa_service_rat_enum_v01 val);
void set_qmi_imsa_get_service_status_rsp_v01_voip_service_rat(imsa_service_rat_enum_v01 val);
void set_qmi_imsa_get_service_status_rsp_v01_vt_service_rat(imsa_service_rat_enum_v01 val);
void set_qmi_imsa_get_service_status_rsp_v01_ut_service_rat(imsa_service_rat_enum_v01 val);
void set_qmi_imsa_get_service_status_rsp_v01_ut_service_status(imsa_service_status_enum_v01 val);

/*
  QMI_IMSA_REGISTRATION_STATUS_IND_V01
*/
void update_default_qmi_imsa_registration_status_ind(void);
void set_qmi_imsa_registration_status_ind_ims_registered(uint8_t ims_registered);
void set_qmi_imsa_registration_status_ind_ims_registration_failure_error_code(uint8_t ims_registration_failure_error_code_valid,
uint16_t ims_registration_failure_error_code);
void set_qmi_imsa_registration_status_ind_ims_reg_status(uint8_t ims_reg_status_valid, imsa_ims_registration_status_enum_v01 ims_reg_status);
void set_qmi_imsa_registration_status_ind_registration_network(imsa_service_rat_enum_v01 val);
void set_qmi_imsa_registration_status_ind_registration_error_string(uint8_t registration_error_string_valid, char* registration_error_string);
p_associated_uri_v01 *set_qmi_imsa_registration_status_ind_uri_list(uint32_t uri_list_len);

/*
  QMI_IMSA_SERVICE_STATUS_IND_V01
*/
void update_default_qmi_imsa_service_status_ind_v01(void);
void set_qmi_imsa_service_status_ind_v01_sms_service_status(imsa_service_status_enum_v01 val);
void set_qmi_imsa_service_status_ind_v01_voip_service_status(imsa_service_status_enum_v01 val);
void set_qmi_imsa_service_status_ind_v01_vt_service_status(imsa_service_status_enum_v01 val);
void set_qmi_imsa_service_status_ind_v01_sms_service_rat(imsa_service_rat_enum_v01 val);
void set_qmi_imsa_service_status_ind_v01_voip_service_rat(imsa_service_rat_enum_v01 val);
void set_qmi_imsa_service_status_ind_v01_vt_service_rat(imsa_service_rat_enum_v01 val);
void set_qmi_imsa_service_status_ind_v01_ut_service_status(imsa_service_status_enum_v01 val);
void set_qmi_imsa_service_status_ind_v01_ut_service_rat(imsa_service_rat_enum_v01 val);

/*
  QMI_IMSA_PDP_STATUS_IND_V01
*/
void update_default_qmi_imsa_pdp_status_ind(uint8_t is_ims_pdp_connected);

/*
  QMI_IMSA_IMS_BLOCK_STATUS_IND_V01
*/
void update_default_qmi_imsa_ims_block_status_ind(void);
imsa_ims_failure_reason_type_v01 *set_qmi_imsa_ims_block_status_ind_ims_failure_on_wwan();
imsa_ims_failure_reason_type_v01 *set_qmi_imsa_ims_block_status_ind_ims_failure_on_wlan();

/*
  QMI_IMSA_DIGITS_INFO_PENDING_IND_V01
*/
void update_default_qmi_imsa_digits_info_pending_ind(void);

/*
  QMI_IMSA_DIGITS_UPDATE_IND_V01
*/
void update_default_qmi_imsa_digits_update_ind(void);
void set_qmi_imsa_digits_update_ind_no_of_lines(uint8_t no_of_lines);
imsa_digits_line_info_v01 *set_qmi_imsa_digits_update_ind_line_info_list(uint32_t line_info_list_len);

/*
  QMI_IMSA_VOICE_INFO_IND_V01
*/
void update_default_qmi_imsa_voice_info_ind(imsa_voice_info_enum_v01 voice_info);

#endif

/*
  QMI_IMSA_RAT_HANDOVER_STATUS_IND_V01
*/
void update_default_qmi_imsa_rat_handover_status_ind(void);
void set_qmi_imsa_rat_handover_status_ind_rat_ho_status(
        imsa_rat_handover_status_enum_v01 rat_ho_status,
        imsa_service_rat_enum_v01 source_rat,
        imsa_service_rat_enum_v01 target_rat,
        const char *cause_code,
        int cause_code_len);

/*
  QMI_IMSA_SET_GEO_LOC_REQ_V01
 */
void update_default_qmi_imsa_set_geo_loc_req(void);
void set_qmi_imsa_set_geo_loc_latitude(double latitude);
void set_qmi_imsa_set_geo_loc_longitude(double longitude);
void set_qmi_imsa_set_geo_loc_city(const char *city);
void set_qmi_imsa_set_geo_loc_zipCode(const char *zipcode);
void set_qmi_imsa_set_geo_loc_state(const char *state);
void set_qmi_imsa_set_geo_loc_countryName(const char *countryName);
void set_qmi_imsa_set_geo_loc_countryCode(const char *countryCode);
void set_qmi_imsa_set_geo_loc_street(const char *street);
void set_qmi_imsa_set_geo_loc_housenumber(const char *housenumber);

/*
  QMI_IMSA_SET_GEO_LOC_RSP_V01
 */
void update_default_qmi_imsa_set_geo_loc_rsp(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_IMSA_GEO_LOC_STATUS_IND_V01
 */
void update_default_qmi_imsa_geo_loc_status_ind();
void set_qmi_imsa_geo_loc_status_ind_latitude(double val);
void set_qmi_imsa_geo_loc_status_ind_longitude(double val);

/*
  QMI_IMSA_VoWIFI_STATUS_IND_V01
*/
imsa_vowifiprofile_status_ind_msg_v01 *update_default_qmi_imsa_vowifiprofile_status_ind(imsa_vowifiprofile_status_enum_v01 vowifiprofile_status);

/*
  QMI_IMSA_GET_VoWIFI_STATUS_REQ_V01
 */
void update_default_qmi_imsa_get_vowifi_status_req(void);

/*
  QMI_IMSA_GET_VoWIFI_STATUSRSP_V01
 */
void update_default_qmi_imsa_get_vowifi_status_resp(qmi_result_type_v01 result,qmi_error_type_v01 error);
void set_qmi_imsa_get_vowifi_status_resp_vowifiprofile_status(imsa_vowifiprofile_status_enum_v01 vowifiprofile_status);

/*
  QMI_IMSA_GET_VIRTUAL_LINE_REQ_V01
 */
void update_default_qmi_imsa_get_virtual_line_req(const char *msisdn);

/*
  QMI_IMSA_GET_VIRTUAL_LINE_RSP_V01
 */
void update_default_qmi_imsa_get_virtual_line_resp(qmi_result_type_v01 result,qmi_error_type_v01 error);
p_associated_uri_v01 *set_qmi_imsa_get_virtual_line_resp_virtual_line_info(uint32_t uri_list_len);


/*
  QMI_IMSA_SET_BIND_SUBSCRIPTION_REQ_V01
*/
void update_default_qmi_imsa_bind_subscription_req_msg(imsa_bind_subscription_type_enum_v01 subs_type);
void update_default_qmi_imsa_bind_subscription_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);

/*
  QMI_IMS_SET_BIND_SUBSCRIPTION_REQ_V01
*/
void update_default_qmi_imss_bind_subscription_req_msg(ims_subscription_type_enum_v01 subs_type);
void update_default_qmi_imss_bind_subscription_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);

/*
  QMI_CSVT_SET_EVENT_REPORT_REQ_V01
*/
void update_default_qmi_csvt_set_event_report_req_v01(void);

/*
QMI_CSVT_SET_SUBSCRIPTION_BINDING_REQ_V01
*/
void update_default_qmi_csvt_set_subscription_binding_req_v01(csvt_bind_subscription_enum_v01 bind_subs);

/*
  QMI_CSVT_SET_SUBSCRIPTION_BINDING_RESP_V01
*/
void update_default_qmi_csvt_set_subscription_binding_resp_v01(qmi_result_type_v01 result,qmi_error_type_v01 error);

/*
  QMI_CSVT_SET_EVENT_REPORT_RESP_V01
*/
void update_default_qmi_csvt_set_event_report_resp_v01(qmi_result_type_v01 result,qmi_error_type_v01 error);
/*
  QMI_VOICE_CALL_CONTROL_RESULT_INFO_IND_V02
*/
voice_call_control_result_info_ind_msg_v02* update_default_qmi_voice_call_control_result_ind_msg();

/*
  QMI_CSVT_ORIGINATE_CALL_REQ_V01
*/
void update_default_qmi_csvt_originate_call_req
            (uint32_t instance_id,
             csvt_call_mode_type_enum_v01 call_mode,
             char *dial_string);
/*
QMI_CSVT_ORIGINATE_CALL_RESP_V01
*/
void update_default_qmi_csvt_originate_call_resp(qmi_result_type_v01 result,qmi_error_type_v01 error);
/*
QMI_CSVT_EVENT_REPORT_IND_V01
*/
void update_default_qmi_csvt_event_report_ind (uint32_t instance_id, csvt_event_type_enum_v01 event_type);
void set_qmi_csvt_event_report_ind_call_end_cause (uint32_t val);
void set_qmi_csvt_event_report_ind_incoming_number(char *num);
/*
QMI_CSVT_END_CALL_REQ_V01
*/
void update_default_qmi_csvt_end_call_req (uint32_t instance_id);
/*
QMI_CSVT_END_CALL_RESP_V01
*/
void update_default_qmi_csvt_end_call_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);
/*
QMI_CSVT_CONFIRM_CALL_REQ_V01
*/
void update_default_qmi_csvt_confirm_call_req (uint32_t instance_id, uint8_t confirm_call);
/*
QMI_CSVT_CONFIRM_CALL_RESP_V01
*/
void update_default_qmi_csvt_confirm_call_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);
/*
QMI_CSVT_ANSWER_CALL_REQ_V01
*/
void update_default_qmi_csvt_answer_call_req (uint32_t instance_id, uint8_t answer_call);
/*
QMI_CSVT_ANSWER_CALL_RESP_V01
*/
void update_default_qmi_csvt_answer_call_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);
/*
QMI_CSVT_RESET_REQ_V01
*/
void update_default_qmi_csvt_reset_req();
void update_default_qmi_csvt_reset_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);
/*
  QMI_VOICE_SETUP_ANSWER_REQ_V02
*/
void update_default_qmi_voice_setup_answer_req(uint8_t call_id);
void set_qmi_voice_setup_answer_req_reject_setup(boolean val);
/*
  QMI_VOICE_SETUP_ANSWER_RESP_V02
*/
void update_default_qmi_voice_setup_answer_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_VOICE_ADDITIONAL_CALL_INFO_IND_V02
*/
void update_default_qmi_voice_additional_call_info_ind_msg(uint8_t call_id);
uint8_t* set_qmi_voice_additional_call_info_ind_extension_header_info(uint16_t sequence, uint32_t additional_call_info_len, uint32_t total_size);

/*
  QMI_VOICE_CONF_PARTICIPANT_STATUS_INFO_IND_V02
*/
  void update_default_qmi_voice_conf_participant_status_info_ind_msg(uint8_t call_id, char *participant_uri);
  void set_qmi_voice_conf_participant_status_info_ind_op_status(conf_participant_operation_enum_v02 operation, uint16_t sip_status);
  void set_qmi_voice_conf_participant_status_info_ind_is_qmi_voice_transfer(uint8_t is_qmi_voice_transfer);

/*========================================================
  QMI_WMS_GET_TRANSPORT_NW_REG_INFO_REQ_V01
========================================================*/
void update_default_qmi_wms_get_transport_nw_reg_info_req(void);

/*========================================================
  QMI_WMS_GET_TRANSPORT_NW_REG_INFO_RESP_V01
========================================================*/
void update_default_qmi_wms_get_transport_nw_reg_info_resp(qmi_result_type_v01 result,qmi_error_type_v01 error);
void set_qmi_wms_get_transport_nw_reg_status(uint8_t transport_nw_reg_status_valid, wms_transport_nw_reg_status_enum_v01 transport_nw_reg_status);

/*========================================================
  QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ_V01
========================================================*/
void update_default_qmi_wms_get_transport_layer_info_req(void);

/*========================================================
  QMI_WMS_GET_TRANSPORT_LAYER_INFO_RESP_V01
========================================================*/
void update_default_qmi_wms_get_transport_layer_info_resp(qmi_result_type_v01 result,qmi_error_type_v01 error);
void set_qmi_wms_get_transport_layer_register_ind_info(uint8_t registered_ind_valid, uint8_t registered_ind);
void set_qmi_wms_get_transport_layer_info_type_cap(uint8_t transport_layer_info_valid, wms_transport_type_enum_v01 transport_type, wms_transport_capability_enum_v01 transport_cap);
/*
QMI_IMSA_GET_REGISTRATION_STATUS_REQ_V01
*/
void update_default_qmi_imsa_get_registration_status_req();
/*
QMI_IMSA_GET_REGISTRATION_STATUS_RSP_V01
*/
void update_default_qmi_imsa_get_registration_status_rsp(qmi_result_type_v01 result, qmi_error_type_v01 error);
void set_qmi_imsa_get_registration_status_rsp_ims_registered(uint8_t ims_registered);
void set_qmi_imsa_get_registration_status_rsp_ims_registration_failure_error_code(uint16_t ims_registration_failure_error_code);
void set_qmi_imsa_get_registration_status_rsp_ims_reg_status(imsa_ims_registration_status_enum_v01 ims_reg_status);
void set_qmi_imsa_get_registration_status_rsp_registration_network(imsa_service_rat_enum_v01 registration_network);
p_associated_uri_v01 *set_qmi_imsa_get_registration_status_rsp_uri_list(uint32_t uri_list_len);

/*
QMI_NAS_GET_SUBSCRIPTION_INFO_REQ_MSG_V01
*/
void update_default_qmi_nas_get_subscription_info_req_msg();
/*
QMI_NAS_GET_SUBSCRIPTION_INFO_RESP_MSG_V01
*/
void update_default_qmi_nas_get_subscription_info_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
void set_priority_subs_qmi_nas_get_subscription_info_resp_msg(uint8_t is_priority_subs_valid, nas_is_priority_subs_enum_v01 is_priority_subs);
void set_active_sub_qmi_nas_get_subscription_info_resp_msg(uint8_t is_active_valid, nas_active_subs_info_enum_v01 is_active);
void set_default_data_sub_qmi_nas_get_subscription_info_resp_msg(uint8_t is_default_data_subs_valid, uint8_t is_default_data_subs);
void set_voice_system_id_qmi_nas_get_subscription_info_resp_msg(uint8_t voice_system_id_valid, uint32_t voice_system_id);
void set_lte_voice_system_id_qmi_nas_get_subscription_info_resp_msg(uint8_t lte_voice_system_id_valid, uint32_t lte_voice_system_id);
void set_wlan_voice_system_id_qmi_nas_get_subscription_info_resp_msg(uint8_t wlan_voice_system_id_valid, uint32_t wlan_voice_system_id);

/*
  QMI_UIM_EVENT_REG_REQ
*/
void update_default_qmi_uim_event_reg_req(uint32_t event_mask);
void update_default_qmi_uim_event_reg_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);
void set_qmi_uim_event_req_resp_event_mask(uint32_t event_mask);
/*
  QMI_UIM_GET_CARD_STATUS_REQ_V01
*/
void update_default_qmi_uim_get_card_status_req();
void set_qmi_uim_get_card_status_req_extended_card_status(uint8_t extended_card_status);

/* QMI_UIM_STATUS_CHANGE_IND_V01*/
uim_status_change_ind_msg_v01* update_default_qmi_uim_status_change_ind_v01();

/*
  QMI_UIM_GET_CARD_STATUS_RESP_V01
*/
uim_get_card_status_resp_msg_v01* update_default_qmi_uim_get_card_status_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_UIM_GET_CONFIGURATION_REQ_V01
*/
void update_default_qmi_uim_get_configuration_req();
/*
  QMI_UIM_GET_CONFIGURATION_RESP_V01
*/
void update_default_qmi_uim_get_configuration_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_UIM_GET_SLOTS_STATUS_REQ_V01
*/
void update_default_qmi_uim_get_slots_status_req();
/*
  QMI_UIM_GET_SLOTS_STATUS_RESP_V01
*/
uim_get_slots_status_resp_msg_v01* update_default_qmi_uim_get_slots_status_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_UIM_GET_ATR_REQ_V01
*/
void update_default_qmi_uim_get_atr_req();
/*
  QMI_UIM_GET_ATR_RESP_V01
*/
uim_get_atr_resp_msg_v01* update_default_qmi_uim_get_atr_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_UIM_GET_SERVICE_STATUS_REQ_V01
*/
void update_default_qmi_uim_get_service_status_req(uim_session_information_type_v01 session_information, uint32_t mask);
/*
  QMI_UIM_GET_SERVICE_STATUS_RESP_V01
*/
void update_default_qmi_uim_get_service_status_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);
void set_qmi_uim_get_service_status_resp_fdn_status(uim_fdn_status_enum_v01 fdn_status);
/*
  QMI_UIM_GET_FILE_ATTRIBUTES_REQ_V01
*/
void update_default_qmi_uim_get_file_attributes_req(uim_session_information_type_v01 session_information, uim_file_id_type_v01 file_id);
/*
  QMI_UIM_GET_FILE_ATTRIBUTES_RESP_V01
*/
void update_default_qmi_uim_get_file_attributes_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);
void set_qmi_uim_get_file_attributes_resp_card_result(uim_card_result_type_v01 card_result);
void set_qmi_uim_get_file_attribues_resp_file_status(uim_file_status_type_v01 file_status);
uim_file_attributes_type_v01* set_qmi_uim_get_file_attributes_resp_file_attributes();
/*
  QMI_UIM_REFRESH_REGISTER_REQ_V01
*/
void update_default_qmi_uim_get_file_attributes_req
  (uim_session_information_type_v01 session_information, uim_register_refresh_type_v01 register_refresh);
uim_refresh_register_req_msg_v01 *update_default_qmi_uim_refresh_register_req();
/*
  QMI_UIM_REFRESH_REGISTER_RESP_V01
*/
void update_default_qmi_uim_refresh_register_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);
/*
  QMI_UIM_READ_RECORD_REQ_V01
*/
uim_read_record_req_msg_v01* update_default_qmi_uim_read_record_reg();
uim_read_record_resp_msg_v01* update_default_qmi_uim_read_record_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_UIM_READ_TRANSPARENT_REQ_V01
*/
uim_read_transparent_req_msg_v01* update_default_qmi_uim_read_transparent_reg();
uim_read_transparent_resp_msg_v01* update_default_qmi_uim_read_transparent_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);
/*
  QMI_UIM_GET_LABEL_REQ_V01
*/
uim_get_label_req_msg_v01* update_default_qmi_uim_get_label_req();

/*
  QMI_UIM_GET_LABEL_RESP_V01
*/
uim_get_label_resp_msg_v01* update_default_qmi_uim_get_label_resp();

/*
  QMI_UIM_OPEN_LOGICAL_CHANNEL_REQ_V01
*/
uim_open_logical_channel_req_msg_v01* update_default_qmi_uim_open_channel_req();

/*
  QMI_UIM_OPEN_LOGICAL_CHANNEL_RESP_V01
*/
uim_open_logical_channel_resp_msg_v01* update_default_qmi_uim_open_channel_resp();

/*
  QMI_UIM_VERIFY_PIN_REQ_V01
*/
uim_verify_pin_req_msg_v01* update_default_qmi_uim_verify_pin_req();

/*
  QMI_UIM_VERIFY_PIN_RESP_V01
*/
uim_verify_pin_resp_msg_v01* update_default_qmi_uim_verify_pin_resp();

/*
  QMI_UIM_UNBLOCK_PIN_REQ_V01
*/
uim_unblock_pin_req_msg_v01* update_default_qmi_uim_unblock_pin_req();

/*
  QMI_UIM_UNBLOCK_PIN_RESP_V01
*/
uim_unblock_pin_resp_msg_v01* update_default_qmi_uim_unblock_pin_resp();

/*
  QMI_UIM_DEPERSONALIZATION_REQ_V01
*/
uim_depersonalization_req_msg_v01* update_default_qmi_uim_depersonalization_req();

/*
  QMI_UIM_DEPERSONALIZATION_RESP_V01
*/
uim_depersonalization_resp_msg_v01* update_default_qmi_uim_depersonalization_resp();

/*
  QMI_UIM_CHANGE_PROVISIONING_SESSION_REQ_V01
*/
void update_default_qmi_uim_change_provisioning_session_req(uim_session_type_enum_v01 session_type, uint8_t activate);
void set_qmi_uim_change_provisioning_session_req_application_information(uim_slot_enum_v01 slot, uint32_t aid_len, uint8_t* aid);

/*
  QMI_UIM_CHANGE_PROVISIONING_SESSION_RESP_V01
*/
void update_default_qmi_uim_change_provisioning_session_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_UIM_POWER_UP_REQ
*/
void update_default_qmi_uim_power_up_req(int slot);

/*
  QMI_UIM_POWER_UP_RESP
*/
void update_default_qmi_uim_power_up_resp();

/*
  QMI_UIM_HTTP_RESET_REQ_V01
*/
void update_default_qmi_uim_http_reset_req();
/*
  QMI_UIM_HTTP_RESET_RESP_V01
*/
void update_default_qmi_uim_http_reset_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_CAT_GET_CONFIGURATION_REQ_V02
*/
void update_default_qmi_cat_get_configuration_req();

/*
  QMI_CAT_GET_CONFIGURATION_RESP_V02
*/
void update_default_qmi_cat_get_configuration_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_CAT_SET_EVENT_REPORT_REQ_V02
*/
void update_default_qmi_cat_set_event_report_req();
void set_qmi_cat_set_event_report_req_pc_evt_report_req_mask(uint32_t pc_evt_report_req_mask);
void set_qmi_cat_set_event_report_req_pc_dec_evt_report_req_mask(uint32_t pc_dec_evt_report_req_mask);
void set_qmi_cat_set_event_report_req_slot_mask(cat_set_event_report_slot_mask_v02 slot_mask);
void set_qmi_cat_set_event_report_req_pc_full_func_evt_report_req_mask(cat_set_event_report_full_func_mask_v02 pc_full_func_evt_report_req_mask);
/*
  QMI_CAT_SET_EVENT_REPORT_RESP_V02
*/
void update_default_qmi_cat_set_event_report_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);
void set_qmi_cat_set_event_report_resp_pc_event_report_req_err_mask(uint32_t pc_evt_report_req_err_mask);
void set_qmi_cat_set_event_report_resp_pc_dec_evt_report_req_err_mask(uint32_t pc_dec_evt_report_req_err_mask);
void set_qmi_cat_set_event_report_resp_pc_full_func_evt_report_err_mask(cat_set_event_report_full_func_mask_v02 pc_full_func_evt_report_err_mask);

/*
  QMI_NAS_AVOID_TUNEAWAY_REQ_MSG_V01
*/
void update_default_qmi_nas_avoid_tune_away_req_msg(uint8_t trm_priority);
void update_default_qmi_nas_avoid_tune_away_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);

/*
  QMI_NAS_DUAL_STANDBY_PREF_REQ_MSG_V01
*/
void update_default_qmi_nas_dual_standby_pref_req_msg(void);
void update_default_qmi_nas_dual_standby_pref_req_msg(nas_standby_pref_enum_v01 standby_pref);
void update_qmi_nas_dual_standby_pref_data_subs(nas_subs_type_enum_v01 default_data_subs);
void update_qmi_nas_dual_standby_pref_dds_duration(nas_dds_duration_enum_v01 dds_duration);
void update_default_qmi_nas_dual_standby_pref_resp_msg(qmi_result_type_v01 result,qmi_error_type_v01 error);
void update_qmi_nas_dual_standby_pref_active_subs_mask(nas_active_subs_mask_type_v01 val);

/*
  QMI_DSD_SYSTEM_STATUS_IND_MSG
*/
dsd_system_status_ind_msg_v01* update_default_qmi_dsd_system_status_ind(void);
void set_qmi_dsd_system_status_ind_avail_sys(dsd_sys_network_enum_v01 technology, dsd_sys_rat_ex_enum_v01 rat_value, dsd_sys_so_mask_v01 so_mask);

/*
  QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_REQ_V01
*/
void update_default_qmi_ims_settings_get_client_provisioning_config_req_v01(void);

/*
  QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_RSP_V01
*/
void update_default_qmi_ims_settings_get_client_provisioning_config_rsp_v01(void);
void set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_result(qmi_result_type_v01 result, qmi_error_type_v01 error);
void set_wifi_call_get_client_provisioning_config_rsp_v01(uint8_t wifi_call_valid, ims_settings_wfc_status_enum_v01 wifi_call);
void set_wifi_call_preference_get_client_provisioning_config_rsp_v01(uint8_t wifi_call_preference_valid, ims_settings_wfc_preference_v01 wifi_call_preference);
void set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_settings_resp(uint8_t val);
void set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_enable_volte(uint8_t val);
void set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_enable_vt(uint8_t val);
void set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_enable_presence(uint8_t val);
void set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_wifi_call(ims_settings_wfc_status_enum_v01 val);
void set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_wifi_call_roaming(ims_settings_wfc_roaming_enum_v01 val);
void set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_wifi_call_preference(ims_settings_wfc_preference_v01 val);
/*
  QMI_IMS_SETTINGS_CONFIG_IND_REG_REQ_V01
*/
void update_default_qmi_ims_config_ind_reg_req_v01(void);

/*
  QMI_IMS_SETTINGS_CONFIG_IND_REG_RSP_V01
*/
void update_default_qmi_ims_config_ind_reg_rsp_v01(void);
void set_qmi_ims_config_ind_req_rsp_v01_result(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*
  QMI_NAS_GET_SERV_CELL_SIB_REQ_MSG_V01
*/
void update_default_qmi_nas_get_serv_cell_sib_req(void);
void set_qmi_nas_get_serv_cell_sib_req_sib_num(uint8_t sib_num);

/*
  QMI_NAS_GET_SERV_CELL_SIB_RESP_MSG_V01
*/
void update_default_qmi_nas_get_serv_cell_sib_resp(void);
void set_qmi_nas_get_serv_cell_sib_resp_total_size(uint16_t total_size);

/*
  QMI_NAS_GET_SERV_CELL_SIB_IND_MSG_V01
*/
void update_default_qmi_nas_get_serv_cell_sib_ind(void);
void set_qmi_nas_get_serv_cell_sib_ind_sequence(uint8_t sequence);
void set_qmi_nas_get_serv_cell_sib_ind_sib_pkt(uint8_t sib_pkt[], uint32_t sib_pkt_len);

/*========================================================
  QMI_NAS_SET_LTE_BAND_PRIORITY_REQ_V01
========================================================*/
nas_set_lte_band_priority_req_msg_v01* update_default_qmi_nas_set_lte_band_priority_req(void);

/*========================================================
  QMI_NAS_SET_LTE_BAND_PRIORITY_RESP_V01
========================================================*/
void update_default_qmi_nas_set_lte_band_priority_resp(void);

/*========================================================
  QMI_NAS_GET_LTE_BAND_PRIORITY_LIST_REQ_MSG_V01
========================================================*/
void update_default_qmi_nas_get_lte_band_priority_list_req(void);

/*========================================================
  QMI_NAS_GET_LTE_BAND_PRIORITY_LIST_RESP_MSG_V01
========================================================*/
nas_get_lte_band_priority_list_resp_msg_v01* update_default_qmi_nas_get_lte_band_priority_list_resp(void);

/*========================================================
  QMI_NAS_PERFORM_INCREMENTAL_NETWORK_SCAN_REQ_MSG_V01
========================================================*/
void update_default_qmi_nas_perform_incremental_network_scan_req(nas_perform_incremental_network_scan_req_msg_v01 *req = nullptr);
void set_qmi_nas_perform_incremental_network_scan_req_network_type(nas_network_type_mask_type_v01 val);
void set_qmi_nas_perform_incremental_network_scan_req_scan_type(nas_nw_scan_type_enum_v01 val);
void set_qmi_nas_perform_incremental_network_scan_req_band_pref(nas_band_pref_mask_type_v01 val);
void set_qmi_nas_perform_incremental_network_scan_req_lte_band_pref(lte_band_pref_mask_type_v01 val);
void set_qmi_nas_perform_incremental_network_scan_req_tdscdma_band_pref(nas_tdscdma_band_pref_mask_type_v01 val);
void set_qmi_nas_perform_incremental_network_scan_req_wcdma_arfcn(uint32_t len, uint16_t val[]);
void set_qmi_nas_perform_incremental_network_scan_req_lte_arfcn(uint32_t len, uint32_t val[]);
void set_qmi_nas_perform_incremental_network_scan_req_search_periodicity(uint8_t val);
void set_qmi_nas_perform_incremental_network_scan_req_max_search_time(uint16_t val);
void set_qmi_nas_perform_incremental_network_scan_req_incremental_results(uint8_t val);
void set_qmi_nas_perform_incremental_network_scan_req_incremental_results_periodicity(uint8_t val);
void set_qmi_nas_perform_incremental_network_scan_req_preferred_plmn_list(uint32_t len, nas_plmn_id_ext_type_v01 val[]);
void set_qmi_nas_perform_incremental_network_scan_req_nr5g_band_pref(nas_nr5g_band_pref_mask_type_v01 val);
void set_qmi_nas_perform_incremental_network_scan_req_nr5g_arfcn(uint32_t len, uint32_t val[]);
void set_qmi_nas_perform_incremental_network_scan_req_lte_band_pref_ext(lte_band_pref_mask_ext_type_v01 val);

/*========================================================
  QMI_NAS_PERFORM_INCREMENTAL_NETWORK_SCAN_RESP_MSG_V01
========================================================*/
void update_default_qmi_nas_perform_incremental_network_scan_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*========================================================
  QMI_NAS_ABORT_SCAN_REQ_V01
========================================================*/
void update_default_qmi_nas_abort_scan_req(void);

/*========================================================
  QMI_NAS_ABORT_SCAN_RESP_V01
========================================================*/
void update_default_qmi_nas_abort_scan_resp(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*========================================================
  QMI_NAS_PERFORM_INCREMENTAL_NETWORK_SCAN_IND_MSG_V01
========================================================*/
void update_default_qmi_nas_perform_incremental_network_scan_ind(nas_scan_status_enum_v01 scan_status);
nas_3gpp_network_scan_result_type_v01 *set_qmi_nas_perform_incremental_network_scan_ind_nas_network_scan_info(uint32_t len);
nas_csg_nw_info_type_v01 *set_qmi_nas_perform_incremental_network_scan_ind_csg_info(uint32_t len);
nas_nw_name_source_enum_type_v01 *set_qmi_nas_perform_incremental_network_scan_ind_nw_name_source(uint32_t len, nas_nw_name_source_enum_type_v01 val[]);
nas_signal_quality_e_type_v01 *set_qmi_nas_perform_incremental_network_scan_ind_signal_quality(uint32_t len, nas_signal_quality_e_type_v01 val[]);
int32_t *set_qmi_nas_perform_incremental_network_scan_ind_signal_strength(uint32_t len, int32_t val[]);
uint8_t *set_qmi_nas_perform_incremental_network_scan_ind_sib2_upr_layer_ind_value(uint32_t len, uint8_t val[]);

/*========================================================
  QMI_PDC_SET_FEATURE
========================================================*/
void update_default_qmi_pdc_set_feature_req();
void update_default_qmi_pdc_set_feature_req(uint32_t slot_id);
void set_default_qmi_pdc_set_feature_req_selection_mode(pdc_selection_mode_enum_v01 selection_mode);
void update_default_qmi_pdc_set_feature_resp();
void update_default_qmi_pdc_set_feature_resp(qmi_result_type_v01 result,qmi_error_type_v01 error);
void update_default_qmi_pdc_set_feature_ind();
void update_default_qmi_pdc_set_feature_ind(qmi_error_type_v01 error);

/*========================================================
  QMI_PDC_GET_SELECTED_CONFIG
========================================================*/
pdc_get_selected_config_req_msg_v01* update_default_qmi_pdc_get_selected_config_req();
void set_qmi_pdc_get_selected_config_req_config_type(pdc_config_type_enum_v01 config_type);
void set_qmi_pdc_get_selected_config_req_subscription_id(uint8_t subscription_id);
void update_default_qmi_pdc_get_selected_config_resp();
void update_default_qmi_pdc_get_selected_config_resp(qmi_result_type_v01 result,qmi_error_type_v01 error);
void update_default_qmi_pdc_get_selected_config_ind();
void set_qmi_pdc_get_selected_config_ind_active_config_id(const uint8_t config_id[], uint32_t config_id_len);
void set_qmi_pdc_get_selected_config_ind_pending_config_id(const uint8_t config_id[], uint32_t config_id_len);

/*========================================================
  QMI_PDC_DEACTIVATE_CONFIG
========================================================*/
pdc_deactivate_config_req_msg_v01* update_default_qmi_pdc_deactivate_config_req();
void set_qmi_pdc_deactivate_config_req_subscription_id(uint8_t subscription_id);
void set_qmi_pdc_deactivate_config_req_config_type(pdc_config_type_enum_v01 config_type);
void update_default_qmi_pdc_deactivate_config_resp();
void update_default_qmi_pdc_deactivate_config_resp(qmi_result_type_v01 result,qmi_error_type_v01 error);
void update_default_qmi_pdc_deactivate_config_ind();

/*========================================================
  QMI_PDC_DELETE_CONFIG
========================================================*/
pdc_delete_config_req_msg_v01* update_default_qmi_pdc_delete_config_req();
void set_qmi_pdc_delete_config_req_config_type(pdc_config_type_enum_v01 config_type);
void set_qmi_pdc_delete_config_req_config_id(uint8_t config_id[], uint32_t config_id_len);
void update_default_qmi_pdc_delete_config_resp();
void update_default_qmi_pdc_delete_config_resp(qmi_result_type_v01 result,qmi_error_type_v01 error);
void update_default_qmi_pdc_delete_config_ind();

/*========================================================
  QMI_PDC_LOAD_CONFIG
========================================================*/
void update_default_qmi_pdc_load_config_req();
void set_qmi_pdc_load_config_req_config_id(uint8_t config_id[], uint32_t config_id_len);
void set_qmi_pdc_load_config_req_config_type(pdc_config_type_enum_v01 config_type);
void set_qmi_pdc_load_config_req_config_size(uint32_t config_size);
void set_qmi_pdc_load_config_req_config_frame(uint8_t config_frame[], uint32_t config_frame_len);
void update_default_qmi_pdc_load_config_resp();
void update_default_qmi_pdc_load_config_resp(qmi_result_type_v01 result,qmi_error_type_v01 error);
void update_default_qmi_pdc_load_config_ind();

/*========================================================
  QMI_PDC_SET_SELECTED_CONFIG
========================================================*/
pdc_set_selected_config_req_msg_v01* update_default_qmi_pdc_set_selected_config_req();
void set_qmi_pdc_set_selected_config_req_subscription_id(uint8_t subscription_id);
void set_qmi_pdc_set_selected_config_req_config_type(pdc_config_type_enum_v01 config_type);
void set_qmi_pdc_set_selected_config_req_config_id(uint8_t config_id[], uint32_t config_id_len);
void update_default_qmi_pdc_set_selected_config_resp();
void update_default_qmi_pdc_set_selected_config_resp(qmi_result_type_v01 result,qmi_error_type_v01 error);
void update_default_qmi_pdc_set_selected_config_ind();

/*========================================================
  QMI_PDC_ACTIVATE_CONFIG
========================================================*/
void update_default_qmi_pdc_activate_config_req();
void update_qmi_pdc_activate_config_req_subscription_id(uint8_t subscription_id);
void update_qmi_pdc_activate_config_req_config_type(pdc_config_type_enum_v01 config_type);
void update_default_qmi_pdc_activate_config_resp();
void update_default_qmi_pdc_activate_config_resp(qmi_result_type_v01 result,qmi_error_type_v01 error);
void update_default_qmi_pdc_activate_config_ind();

/*========================================================
  QMI_PDC_LIST_CONFIGS
========================================================*/
void update_default_qmi_pdc_list_config_req();
void update_qmi_pdc_list_config_req_config_type(pdc_config_type_enum_v01 config_type);
void update_default_qmi_pdc_list_config_resp();
void update_default_qmi_pdc_list_config_ind();

/*========================================================
  QMI_PDC_GET_CONFIG_INFO
========================================================*/
pdc_get_config_info_req_msg_v01* update_default_qmi_pdc_get_config_info_req();
void set_qmi_pdc_get_config_info_req_config_type(pdc_config_type_enum_v01 config_type);
void set_qmi_pdc_get_config_info_req_config_id(uint8_t config_id[], uint32_t config_id_len);
void update_default_qmi_pdc_get_config_info_resp();
void update_default_qmi_pdc_get_config_info_resp(qmi_result_type_v01 result,qmi_error_type_v01 error);
pdc_get_config_info_ind_msg_v01* update_default_qmi_pdc_get_config_info_ind();

/*========================================================
  QMI_LTE_INDICATION_REGISTER_REQ_V01
========================================================*/
void update_default_qmi_lte_indication_register_req_msg(void);
void set_qmi_lte_indication_register_req_indication_bitmask(qmi_lte_indication_reg_mask_t_v01 indication_bitmask);

/*========================================================
  QMI_LTE_INDICATION_REGISTER_RESP_V01
========================================================*/
void update_default_qmi_lte_indication_register_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*========================================================
  QMI_LTE_DISC_TERMINATE_REQ_V01
========================================================*/
void update_default_qmi_lte_disc_terminate_req_msg(void);
void set_qmi_lte_disc_terminate_req_os_app_id(char * os_app_id);

/*========================================================
  QMI_LTE_DISC_TERMINATE_RESP_V01
========================================================*/
void update_default_qmi_lte_disc_terminate_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*========================================================
  QMI_LTE_DISC_GET_SERVICE_STATUS_REQ_V01
========================================================*/
void update_default_qmi_lte_disc_get_service_status_req_msg(void);

/*========================================================
  QMI_LTE_DISC_GET_SERVICE_STATUS_RESP_V01
========================================================*/
void update_default_qmi_lte_disc_get_service_status_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);
void set_qmi_lte_disc_get_service_status_resp_publish_allowed(boolean publish_allowed);
void set_qmi_lte_disc_get_service_status_resp_subscribe_allowed(boolean subscribe_allowed);

/*========================================================
  QMI_LTE_DISC_PUBLISH_REQ_V01
========================================================*/
void update_default_qmi_lte_disc_publish_req_msg(void);
void set_qmi_lte_disc_publish_req_os_app_id(char * os_app_id);
void set_qmi_lte_disc_publish_req_discovery_type(qmi_lte_discovery_type_enum_v01 discovery_type);
void set_qmi_lte_disc_publish_req_expression(char * expression);
void set_qmi_lte_disc_publish_req_publish_duration(uint32_t publish_duration);
void set_qmi_lte_disc_publish_req_expression_validity_time(uint32_t expression_validity_time);
void set_qmi_lte_disc_publish_req_meta_data(char * meta_data);

/*========================================================
  QMI_LTE_DISC_PUBLISH_RESP_V01
========================================================*/
void update_default_qmi_lte_disc_publish_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);
void set_qmi_lte_disc_publish_resp_os_app_id(char * os_app_id);
void set_qmi_lte_disc_publish_resp_expression(char * expression);
void set_qmi_lte_disc_publish_resp_expression_result(qmi_lte_disc_result_type_enum_v01 expression_result);

/*========================================================
  QMI_LTE_DISC_SUBSCRIBE_REQ_V01
========================================================*/
void update_default_qmi_lte_disc_subscribe_req_msg(void);
void set_qmi_lte_disc_subscribe_req_os_app_id(char * os_app_id);
void set_qmi_lte_disc_subscribe_req_discovery_type(qmi_lte_discovery_type_enum_v01 discovery_type);
void set_qmi_lte_disc_subscribe_req_expression(char * expression);
void set_qmi_lte_disc_subscribe_req_subscribe_duration(uint32_t subscribe_duration);
void set_qmi_lte_disc_subscribe_req_expression_validity_time(uint32_t expression_validity_time);

/*========================================================
  QMI_LTE_DISC_SUBSCRIBE_RESP_V01
========================================================*/
void update_default_qmi_lte_disc_subscribe_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);
void set_qmi_lte_disc_subscribe_resp_os_app_id(char * os_app_id);
void set_qmi_lte_disc_subscribe_resp_expression(char * expression);
void set_qmi_lte_disc_subscribe_resp_expression_result(qmi_lte_disc_result_type_enum_v01 expression_result);

/*========================================================
  QMI_LTE_DISC_CANCEL_PUBLISH_REQ_V01
========================================================*/
void update_default_qmi_lte_disc_cancel_publish_req_msg(void);
void set_qmi_lte_disc_cancel_publish_req_os_app_id(char * os_app_id);
void set_qmi_lte_disc_cancel_publish_req_expression(char * expression);

/*========================================================
  QMI_LTE_DISC_CANCEL_PUBLISH_RESP_V01
========================================================*/
void update_default_qmi_lte_disc_cancel_publish_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);
void set_qmi_lte_disc_cancel_publish_resp_os_app_id(char * os_app_id);

/*========================================================
  QMI_LTE_DISC_CANCEL_SUBSCRIBE_REQ_V01
========================================================*/
void update_default_qmi_lte_disc_cancel_subscribe_req_msg(void);
void set_qmi_lte_disc_cancel_subscribe_req_os_app_id(char * os_app_id);
void set_qmi_lte_disc_cancel_subscribe_req_expression(char * expression);

/*========================================================
  QMI_LTE_DISC_CANCEL_SUBSCRIBE_RESP_V01
========================================================*/
void update_default_qmi_lte_disc_cancel_subscribe_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);
void set_qmi_lte_disc_cancel_subscribe_resp_os_app_id(char * os_app_id);

/*========================================================
  QMI_LTE_BIND_SUBSCRIPTION_REQ_V01
========================================================*/
void update_default_qmi_lte_bind_subscription_req_msg(qmi_lte_bind_subscription_type_enum_v01 subscription);

/*========================================================
  QMI_LTE_BIND_SUBSCRIPTION_RESP_V01
========================================================*/
void update_default_qmi_lte_bind_subscription_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);

/*========================================================
  QMI_LTE_GET_SUBSCRIPTION_INFO_REQ_V01
========================================================*/
void update_default_qmi_lte_get_subscription_info_req_msg(void);

/*========================================================
  QMI_LTE_GET_SUBSCRIPTION_INFO_RESP_V01
========================================================*/
void update_default_qmi_lte_get_subscription_info_resp_msg(qmi_result_type_v01 result, qmi_error_type_v01 error);
void set_qmi_lte_get_subscription_info_resp_subscription(qmi_lte_bind_subscription_type_enum_v01 subscription);
void set_qmi_lte_get_subscription_info_resp_lte_disc_capability(boolean lte_disc_capability);

/*========================================================
  QMI_LTE_DISC_NOTIFICATION_IND_V01
========================================================*/
void update_default_qmi_lte_disc_notification_ind_msg(void);
void set_qmi_lte_disc_notification_ind_os_app_id(char * os_app_id);
void set_qmi_lte_disc_notification_ind_authorization_result(qmi_lte_disc_result_type_enum_v01 authorization_result);
void set_qmi_lte_disc_notification_ind_expression(char * expression);
void set_qmi_lte_disc_notification_ind_expression_result(qmi_lte_disc_result_type_enum_v01 expression_result);
void set_qmi_lte_disc_notification_ind_publish_status_per_expr(uint8_t publish_status_per_expr);
void set_qmi_lte_disc_notification_ind_subscribe_status_per_expr(uint8_t subscribe_status_per_expr);

/*========================================================
  QMI_LTE_DISC_BROADCAST_NOTIFICATION_IND_V01
========================================================*/
void update_default_qmi_lte_disc_broadcast_notification_ind_msg(void);
void set_qmi_lte_disc_broadcast_notification_ind_publish_allowed(boolean publish_allowed);
void set_qmi_lte_disc_broadcast_notification_ind_subscribe_allowed(boolean subscribe_allowed);

/*========================================================
  QMI_LTE_DISC_MATCH_NOTIFICATION_IND_V01
========================================================*/
void update_default_qmi_lte_disc_match_notification_ind_msg(void);
void set_qmi_lte_disc_match_notification_ind_os_app_id(char * os_app_id);
void set_qmi_lte_disc_match_notification_ind_expression(char * expression);
void set_qmi_lte_disc_match_notification_ind_matched_expression(char * expression);
void set_qmi_lte_disc_match_notification_ind_match_event_state(uint8_t match_event_state);
void set_qmi_lte_disc_match_notification_ind_meta_data_index(uint16_t meta_data_index);
void set_qmi_lte_disc_match_notification_ind_meta_data(char * meta_data);

/*========================================================
  QMI_LTE_SUBSCRIPTION_INFO_IND_V01
========================================================*/
void update_default_qmi_lte_subscription_info_ind_msg(void);
void set_qmi_lte_subscription_info_ind_subscription(qmi_lte_bind_subscription_type_enum_v01 subscription);
void set_qmi_lte_subscription_info_ind_lte_disc_capability(boolean lte_disc_capability);

void update_default_qmi_ims_settings_get_supported_msgs_req(void);
void update_default_qmi_ims_settings_get_supported_msgs_resp(void);

void update_default_qmi_ims_settings_get_supported_fields_req(void);
void update_default_qmi_ims_settings_get_supported_fields_resp(void);

/*========================================================
  QMI_UIM_SEND_STATUS_REQ_V01
========================================================*/
uim_send_status_cmd_req_msg_v01*  update_default_qmi_uim_send_status_req_v01(void);

/*========================================================
  QMI_UIM_SEND_STATUS_RESP_V01
========================================================*/
uim_send_status_cmd_resp_msg_v01* update_default_qmi_uim_send_status_resp_v01(void);

/*========================================================
  QMI_UIM_REFRESH_IND_V01
========================================================*/
uim_refresh_ind_msg_v01* update_default_qmi_uim_refresh_ind_msg_v01(void);

/*========================================================
  QMI_UIM_REFRESH_COMPLETE_REQ_V01
========================================================*/
uim_refresh_complete_req_msg_v01* update_default_qmi_uim_refresh_complete_req_msg_v01(void);

/*========================================================
  QMI_UIM_REFRESH_COMPLETE_RESP_V01
========================================================*/
uim_refresh_complete_resp_msg_v01* update_default_qmi_uim_refresh_complete_resp_msg_v01(void);

} QMIRILTestnode;

#endif /* RIL_UTF_QMIRIL_IF_H_ */
