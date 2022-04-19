/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_test_qmi_ril.cpp
  @brief   IOE UTF test case definitions

  DESCRIPTION
  All test case definitions should be in this file
  ---------------------------------------------------------------------------
******************************************************************************/

#include "ril_utf_test_ioe_ril.h"

test_result ioe_ril_test_bootup_001(user_specified_attrib_t attr)
{
  // this part need to be included for all test cases
  // test ID and attributes for each test case are set here

    Testcase t;
  IOETestnode IOE_node;
  QMIRILTestnode QMI_node;

  t.set_test_id("ioe_ril_test_bootup_001");
  t.set_test_attribute("ALL");
  if (!t.isAttributeMatch(attr)) {
 test_result ret;
 return ret;
  }


   //DMS
   QMI_node.update_default_qmi_dms_set_event_report_req_msg();
   QMI_node.set_qmi_dsm_set_event_report_req_report_oprt_mode_state(1);
   t.expect(QMI_node);

   QMI_node.update_default_qmi_dms_set_event_report_resp_msg();
   QMI_node.set_qmi_dms_set_event_report_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
   t.inject(QMI_node);
//NAS
  QMI_node.update_default_qmi_nas_config_sig_info2_req_msg_v01();
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_cdma_ecio_delta(20);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_cdma_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_gsm_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_ecio_delta(20);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_io_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_sinr_delta(3);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rsrp_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rsrq_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_wcdma_ecio_delta(20);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_wcdma_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_ecio_delta(20);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_rscp_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_sinr_delta(3);
int16_t * lte_snr_threshold_list
        = QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_snr_threshold_list(NAS_SIG_STR_THRESHOLD_LIST_MAX_V01);
int iter_i;
for( iter_i = 0; iter_i < 6 ; iter_i++ )
{
   lte_snr_threshold_list[iter_i] = (-200)  + iter_i * 40;
}
for( iter_i = 6 ; iter_i < NAS_SIG_STR_THRESHOLD_LIST_MAX_V01; iter_i++ )
{
   lte_snr_threshold_list[iter_i] = 30  + (iter_i - 6) * 30 ;
}
t.expect(QMI_node);

QMI_node.update_default_qmi_nas_config_sig_info2_resp_msg_v01();
QMI_node.set_qmi_nas_config_sig_info2_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
t.inject(QMI_node);



QMI_node.update_default_qmi_nas_indication_register_req_msg();
QMI_node.set_qmi_nas_indication_register_req_msg_reg_sys_sel_pref(TRUE);
QMI_node.set_qmi_nas_indication_register_req_msg_dual_standby_pref(TRUE);
QMI_node.set_qmi_nas_indication_register_req_msg_subscription_info(TRUE);
QMI_node.set_qmi_nas_indication_register_req_msg_reg_network_time(FALSE);
QMI_node.set_qmi_nas_indication_register_reg_msg_reg_rtre_cfg(TRUE);
QMI_node.set_qmi_nas_indication_register_reg_msg_reg_embms_status(FALSE); //Always enable action
QMI_node.set_qmi_nas_indication_register_req_msg_sig_info(TRUE);
QMI_node.set_qmi_nas_indication_register_reg_msg_err_rate(TRUE);
QMI_node.set_qmi_nas_indication_register_req_msg_req_serving_system(FALSE);
QMI_node.set_qmi_nas_indication_register_req_msg_reg_current_plmn_name(FALSE);
QMI_node.set_qmi_nas_indication_register_req_sys_info(TRUE);
QMI_node.set_qmi_nas_indication_register_req_msg_reg_rf_band_info(FALSE);
t.expect(QMI_node);

QMI_node.update_default_qmi_nas_indication_register_resp_msg_v01();
QMI_node.set_qmi_nas_nas_indication_register_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
t.inject(QMI_node);

//nas sys info depecting a WCDMA scenario
QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
t.expect(QMI_node);

QMI_node.update_qmi_nas_get_sys_info_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);

nas_3gpp2_srv_status_info_type_v01 * cdma_hdr_srv_status_info =
   QMI_node.set_qmi_nas_get_sys_info_resp_cdma_srv_status_info();
cdma_hdr_srv_status_info->is_pref_data_path = 0x00;
cdma_hdr_srv_status_info->srv_status = 0x00; //NAS_SYS_SRV_STATUS_NO_SRV

 cdma_hdr_srv_status_info =
     QMI_node.set_qmi_nas_get_sys_info_resp_hdr_srv_status_info();
   cdma_hdr_srv_status_info->is_pref_data_path = 0x00;
   cdma_hdr_srv_status_info->srv_status = 0x00; //NAS_SYS_SRV_STATUS_NO_SRV

 nas_3gpp_srv_status_info_type_v01 * gw_lte_srv_status_info =
     QMI_node.set_qmi_nas_get_sys_info_resp_gsm_srv_status_info();
 gw_lte_srv_status_info->is_pref_data_path = 0x00;
 gw_lte_srv_status_info->srv_status = 0x00; //No SRV
 gw_lte_srv_status_info->true_srv_status = 0x00; //no srv

 gw_lte_srv_status_info =
     QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_srv_status_info();
 gw_lte_srv_status_info->is_pref_data_path = 0x00;
 gw_lte_srv_status_info->srv_status = 0x02; //srv_status_srv
 gw_lte_srv_status_info->true_srv_status = 0x02; //srv_status_srv

 gw_lte_srv_status_info =
     QMI_node.set_qmi_nas_get_sys_info_resp_lte_srv_status_info();
 gw_lte_srv_status_info->is_pref_data_path = 0x00;
 gw_lte_srv_status_info->srv_status = 0x00;
 gw_lte_srv_status_info->srv_status = 0x00;

 gw_lte_srv_status_info =
     QMI_node.set_qmi_nas_get_sys_info_resp_tdscdma_srv_status_info();
 gw_lte_srv_status_info->is_pref_data_path = 0x00;
 gw_lte_srv_status_info->srv_status = 0x00;
 gw_lte_srv_status_info->true_srv_status = 0x00;

 nas_wcdma_sys_info_type_v01 * wcdma_sys_info =
     QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_sys_info();
 wcdma_sys_info->common_sys_info.srv_capability_valid = 1;
 wcdma_sys_info->common_sys_info.srv_capability = 3; //CS_PS
 wcdma_sys_info->common_sys_info.srv_domain_valid = 1;
 wcdma_sys_info->common_sys_info.srv_domain = 3; //CS_PS
 wcdma_sys_info->common_sys_info.roam_status_valid = 1;
 wcdma_sys_info->common_sys_info.roam_status = 0; //Off
 wcdma_sys_info->threegpp_specific_sys_info.cell_id_valid = 1;
 wcdma_sys_info->threegpp_specific_sys_info.cell_id = 145621054;
 wcdma_sys_info->threegpp_specific_sys_info.lac_valid = 1;
 wcdma_sys_info->threegpp_specific_sys_info.lac = 1818;
 wcdma_sys_info->threegpp_specific_sys_info.network_id_valid = 1;
 wcdma_sys_info->threegpp_specific_sys_info.network_id.mcc = { 3, 1, 0};
 wcdma_sys_info->threegpp_specific_sys_info.network_id.mnc = {0, 0};
 wcdma_sys_info->threegpp_specific_sys_info.reg_reject_info_valid = 0;
 wcdma_sys_info->wcdma_specific_sys_info.hs_call_status_valid = 1;
 wcdma_sys_info->wcdma_specific_sys_info.hs_call_status = 0;
 wcdma_sys_info->wcdma_specific_sys_info.hs_ind_valid = 1;
 wcdma_sys_info->wcdma_specific_sys_info.hs_ind = 3;
 wcdma_sys_info->wcdma_specific_sys_info.psc_valid = 1;
 wcdma_sys_info->wcdma_specific_sys_info.psc = 62;

 nas_wcdma_sys_info2_type_v01 * wcdma_sys_info2 =
     QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_sys_info2();
 wcdma_sys_info2->geo_sys_idx = 65535;
 wcdma_sys_info2->cell_broadcast_cap = 2; //On

 nas_gw_sys_info3_type_v01 * wcdma_sys_info3 =
     QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_sys_info3();
 wcdma_sys_info3->cs_bar_status = 3; // access all calls
 wcdma_sys_info3->ps_bar_status = 3;

 QMI_node.set_qmi_nas_get_sys_info_resp_sim_rej_info(1); //sim available

 t.inject(QMI_node);

 QMI_node.update_default_qmi_nas_get_sig_info_req_msg_v01();
 t.expect(QMI_node);

 QMI_node.update_default_qmi_nas_get_sig_info_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
 nas_common_sig_info_param_type_v01 * wcdma_sig_info
                           = QMI_node.set_qmi_nas_get_sig_info_resp_wcdma_sig_info();
 wcdma_sig_info->rssi = -51;
 wcdma_sig_info->ecio = 3;
 t.inject(QMI_node);

 QMI_node.update_default_qmi_nas_limit_sys_info_ind_reporting_req_msg(NULL);
 t.expect(QMI_node);

 QMI_node.update_default_qmi_nas_limit_sys_info_ind_reporting_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
 t.inject(QMI_node);


QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
t.expect(QMI_node);

QMI_node.update_qmi_nas_get_sys_info_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);

cdma_hdr_srv_status_info =
   QMI_node.set_qmi_nas_get_sys_info_resp_cdma_srv_status_info();
cdma_hdr_srv_status_info->is_pref_data_path = 0x00;
cdma_hdr_srv_status_info->srv_status = 0x00; //NAS_SYS_SRV_STATUS_NO_SRV

 cdma_hdr_srv_status_info =
     QMI_node.set_qmi_nas_get_sys_info_resp_hdr_srv_status_info();
   cdma_hdr_srv_status_info->is_pref_data_path = 0x00;
   cdma_hdr_srv_status_info->srv_status = 0x00; //NAS_SYS_SRV_STATUS_NO_SRV

 gw_lte_srv_status_info =
     QMI_node.set_qmi_nas_get_sys_info_resp_gsm_srv_status_info();
 gw_lte_srv_status_info->is_pref_data_path = 0x00;
 gw_lte_srv_status_info->srv_status = 0x00; //No SRV
 gw_lte_srv_status_info->true_srv_status = 0x00; //no srv

 gw_lte_srv_status_info =
     QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_srv_status_info();
 gw_lte_srv_status_info->is_pref_data_path = 0x00;
 gw_lte_srv_status_info->srv_status = 0x02; //srv_status_srv
 gw_lte_srv_status_info->true_srv_status = 0x02; //srv_status_srv

 gw_lte_srv_status_info =
     QMI_node.set_qmi_nas_get_sys_info_resp_lte_srv_status_info();
 gw_lte_srv_status_info->is_pref_data_path = 0x00;
 gw_lte_srv_status_info->srv_status = 0x00;
 gw_lte_srv_status_info->srv_status = 0x00;

 gw_lte_srv_status_info =
     QMI_node.set_qmi_nas_get_sys_info_resp_tdscdma_srv_status_info();
 gw_lte_srv_status_info->is_pref_data_path = 0x00;
 gw_lte_srv_status_info->srv_status = 0x00;
 gw_lte_srv_status_info->true_srv_status = 0x00;

 wcdma_sys_info =
     QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_sys_info();
 wcdma_sys_info->common_sys_info.srv_capability_valid = 1;
 wcdma_sys_info->common_sys_info.srv_capability = 3; //CS_PS
 wcdma_sys_info->common_sys_info.srv_domain_valid = 1;
 wcdma_sys_info->common_sys_info.srv_domain = 3; //CS_PS
 wcdma_sys_info->common_sys_info.roam_status_valid = 1;
 wcdma_sys_info->common_sys_info.roam_status = 0; //Off
 wcdma_sys_info->threegpp_specific_sys_info.cell_id_valid = 1;
 wcdma_sys_info->threegpp_specific_sys_info.cell_id = 145621054;
 wcdma_sys_info->threegpp_specific_sys_info.lac_valid = 1;
 wcdma_sys_info->threegpp_specific_sys_info.lac = 1818;
 wcdma_sys_info->threegpp_specific_sys_info.network_id_valid = 1;
 wcdma_sys_info->threegpp_specific_sys_info.network_id.mcc = { 3, 1, 0};
 wcdma_sys_info->threegpp_specific_sys_info.network_id.mnc = {0, 0};
 wcdma_sys_info->threegpp_specific_sys_info.reg_reject_info_valid = 0;
 wcdma_sys_info->wcdma_specific_sys_info.hs_call_status_valid = 1;
 wcdma_sys_info->wcdma_specific_sys_info.hs_call_status = 0;
 wcdma_sys_info->wcdma_specific_sys_info.hs_ind_valid = 1;
 wcdma_sys_info->wcdma_specific_sys_info.hs_ind = 3;
 wcdma_sys_info->wcdma_specific_sys_info.psc_valid = 1;
 wcdma_sys_info->wcdma_specific_sys_info.psc = 62;

 wcdma_sys_info2 =
     QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_sys_info2();
 wcdma_sys_info2->geo_sys_idx = 65535;
 wcdma_sys_info2->cell_broadcast_cap = 2; //On

 wcdma_sys_info3 =
     QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_sys_info3();
 wcdma_sys_info3->cs_bar_status = 3; // access all calls
 wcdma_sys_info3->ps_bar_status = 3;

 QMI_node.set_qmi_nas_get_sys_info_resp_sim_rej_info(1); //sim available

 t.inject(QMI_node);


   //Voice

      uint8_t qmi_voice_temp = TRUE;
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_reg_dtmf_events(qmi_voice_temp);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_ind_register_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_reg_voice_privacy_events(qmi_voice_temp);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_ind_register_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_ext_brst_intl_events(qmi_voice_temp);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_ind_register_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_speech_events(qmi_voice_temp);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_ind_register_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_handover_events(qmi_voice_temp);
  QMI_node.set_qmi_voice_ind_req_conference_events(qmi_voice_temp);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_ind_register_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  //WMS
  QMI_node.update_default_qmi_wms_set_primary_client_req_v01();
  QMI_node.set_qmi_wms_set_primary_client_req_v01_primary_client(TRUE);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_wms_set_primary_client_resp_v01();
  QMI_node.set_qmi_wms_set_primary_client_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  wms_set_route_list_tuple_type_v01* list_tuple =
                         QMI_node.update_default_qmi_wms_set_routes_req_msg(6);
  for(iter_i = 0; iter_i < 6; iter_i++)
  {
   list_tuple[iter_i].message_type = WMS_MESSAGE_TYPE_POINT_TO_POINT_V01;
   list_tuple[iter_i].message_class = iter_i;
   list_tuple[iter_i].route_storage = WMS_STORAGE_TYPE_NONE_V01;
   list_tuple[iter_i].receipt_action = WMS_TRANSFER_ONLY_V01;
  }
  list_tuple[2].message_type = WMS_MESSAGE_TYPE_POINT_TO_POINT_V01;
  list_tuple[2].message_class = WMS_MESSAGE_CLASS_2_V01;
  list_tuple[2].route_storage = WMS_STORAGE_TYPE_UIM_V01;
  list_tuple[2].receipt_action = WMS_STORE_AND_NOTIFY_V01;
  QMI_node.set_qmi_wms_set_routes_req_transfer_ind(WMS_TRANSFER_IND_CLIENT_V01);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_wms_set_routes_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  QMI_node.update_default_qmi_wms_set_event_report_req_msg();
  QMI_node.set_qmi_wms_set_event_report_report_mt_message(1);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_wms_set_event_report_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  QMI_node.update_deafult_qmi_wms_set_broadcast_config_req_msg(WMS_MESSAGE_MODE_GW_V01);
  wms_3gpp_broadcast_config_info_type_v01* wms_3gpp_broadcast_config_info
                            = QMI_node.set_qmi_wms_set_broadcast_config_req_wms_3gpp_broadcast_config_info(3);

  wms_3gpp_broadcast_config_info[0].from_service_id = 0;
  wms_3gpp_broadcast_config_info[0].to_service_id = 999;
  wms_3gpp_broadcast_config_info[0].selected = 1;

  wms_3gpp_broadcast_config_info[1].from_service_id = 4352;
  wms_3gpp_broadcast_config_info[1].to_service_id = 4359;
  wms_3gpp_broadcast_config_info[1].selected = 1;

  wms_3gpp_broadcast_config_info[2].from_service_id = 4370;
  wms_3gpp_broadcast_config_info[2].to_service_id = 4399;
  wms_3gpp_broadcast_config_info[2].selected = 1;

  t.expect(QMI_node);

  QMI_node.update_default_qmi_wms_set_broadcast_config_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
   t.inject(QMI_node);

   wms_broadcast_activation_info_type_v01 * wms_3gpp_broadcast_activation
                           = QMI_node.update_default_qmi_wms_set_broadcast_activation_req_msg();

  wms_3gpp_broadcast_activation->message_mode = WMS_MESSAGE_MODE_GW_V01;
  wms_3gpp_broadcast_activation->bc_activate = 1;
  QMI_node.set_qmi_wms_set_activation_req_msg_activate_all(1);
  t.expect(QMI_node);

  QMI_node.update_qmi_wms_set_broadcast_activation_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  QMI_node.update_deafult_qmi_wms_set_broadcast_config_req_msg(WMS_MESSAGE_MODE_CDMA_V01);
  wms_3gpp2_broadcast_config_info_type_v01* wms_3gpp2_broadcast_config
                           = QMI_node.set_qmi_wms_set_broadcast_config_req_wms_3gpp2_broadcast_config_info(36);

    for (iter_i = 0; iter_i < 32; iter_i++)
    {
        wms_3gpp2_broadcast_config[iter_i].service_category = iter_i;
        wms_3gpp2_broadcast_config[iter_i].language = WMS_LANGUAGE_ENGLISH_V01;
        wms_3gpp2_broadcast_config[iter_i].selected = 1;
    }

    for (iter_i = 0; iter_i < 5; iter_i++)
    {
        wms_3gpp2_broadcast_config[iter_i].service_category = 0x1000 + iter_i;
        wms_3gpp2_broadcast_config[iter_i].language = WMS_LANGUAGE_ENGLISH_V01;
        wms_3gpp2_broadcast_config[iter_i].selected = 1;
    }

  t.expect(QMI_node);
   QMI_node.update_default_qmi_wms_set_broadcast_config_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
   t.inject(QMI_node);

   wms_broadcast_activation_info_type_v01 * wms_3gpp2_broadcast_activation
                           = QMI_node.update_default_qmi_wms_set_broadcast_activation_req_msg();

  wms_3gpp2_broadcast_activation->message_mode = WMS_MESSAGE_MODE_CDMA_V01;
  wms_3gpp2_broadcast_activation->bc_activate = 1;
  QMI_node.set_qmi_wms_set_activation_req_msg_activate_all(1);
  t.expect(QMI_node);
  QMI_node.update_qmi_wms_set_broadcast_activation_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);


  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Calling execute tests!");
  t.execute();
  t.print_summary();
  return t.get_test_result();
}

test_result ioe_ril_test_send_sms_001(user_specified_attrib_t attr)
{

 Testcase t;
  IOETestnode IOE_node;
  QMIRILTestnode QMI_node;

  t.set_test_id("ioe_ril_test_send_sms_001");
  t.set_test_attribute("ALL");
  if (!t.isAttributeMatch(attr)) {
 test_result ret;
 return ret;
  }
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Send_SMS test!");
  IOE_node.update_mcm_sms_send_mo_msg_req_msg(1, "hello", "8586582450");
  t.inject(IOE_node);

  wms_send_raw_message_data_type_v01* raw_message_data =
                      QMI_node.update_default_qmi_wms_raw_send_req_msg();
  raw_message_data->format = WMS_MESSAGE_FORMAT_GW_PP_V01;
  raw_message_data->raw_message_len = 18;
  raw_message_data->raw_message[0] = 0x00;
  raw_message_data->raw_message[1] = 0x01;
  raw_message_data->raw_message[2] = 0x00;
  raw_message_data->raw_message[3] = 0x0A;
  raw_message_data->raw_message[4] = 0x81;
  raw_message_data->raw_message[5] = 0x58;
  raw_message_data->raw_message[6] = 0x68;
  raw_message_data->raw_message[7] = 0x85;
  raw_message_data->raw_message[8] = 0x42;
  raw_message_data->raw_message[9] = 0x05;
  raw_message_data->raw_message[10] = 0x00;
  raw_message_data->raw_message[11] = 0x00;
  raw_message_data->raw_message[12] = 0x05;
  raw_message_data->raw_message[13] = 0xE8;
  raw_message_data->raw_message[14] = 0x32;
  raw_message_data->raw_message[15] = 0x9B;
  raw_message_data->raw_message[16] = 0xFD;
  raw_message_data->raw_message[17] = 0x06;
  t.expect(QMI_node);

  (void)QMI_node.update_default_qmi_wms_raw_send_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, 0x00);
  t.inject(QMI_node);

  IOE_node.update_mcm_sms_send_mo_msg_resp_msg(MCM_RESULT_SUCCESS_V01, MCM_SUCCESS_V01);
  t.expect(IOE_node);


  t.execute();
  t.print_summary();
  return t.get_test_result();
}

test_result ioe_ril_test_get_radio_mode_001(user_specified_attrib_t attr)
{
  Testcase t; //linked list of nodes
  IOETestnode IOE_node; //mcm_reqs/resp/ind apis
  QMIRILTestnode QMI_node; //qmi_reqs/resp/ind apis

  t.set_test_id("ioe_ril_test_get_radio_mode_001");
  t.set_test_attribute("ALL");
  if (!t.isAttributeMatch(attr)) {
      test_result ret;
      return ret;
  }

  IOE_node.update_mcm_dm_get_radio_mode_req_msg();
  t.inject(IOE_node);

  QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
  QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_ONLINE_V01);
  t.inject(QMI_node);

  IOE_node.update_mcm_dm_get_radio_mode_resp_msg(MCM_RESULT_SUCCESS_V01, MCM_SUCCESS_V01);
  IOE_node.set_mcm_dm_get_radio_mode_resp_msg_radio_mode(MCM_DM_RADIO_MODE_ONLINE_V01);
  t.expect(IOE_node);

  t.execute();
  t.print_summary();
  return t.get_test_result();
}
