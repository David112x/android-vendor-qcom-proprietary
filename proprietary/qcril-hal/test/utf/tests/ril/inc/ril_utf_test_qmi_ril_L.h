/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_test_qmi_ril.h
  @brief   RIL UTF test case declarations

  DESCRIPTION
  All test case declarations should be in this file
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef _RIL_UTF_TEST_QMI_RIL_L
#define _RIL_UTF_TEST_QMI_RIL_L

#include "ril_utf_test_includes.h"

//==================================================
// Bootup tests
//  Description:
//   "Boot" here refers to the sequence of actions
//   performed after the rild, main, and qmi_cb threads
//   are started and qmi client initialized has been
//   completed
//==================================================
test_result qmi_ril_test_bootup_001();
test_result qmi_ril_msim_bootup_primary_001();
test_result qmi_ril_msim_bootup_secondary_001();
test_result qmi_ril_msim_bootup_primary_002();

test_result ril_utf_test_set_barring_password_001(user_specified_attrib_t attr);
test_result ril_utf_test_set_barring_password_002(user_specified_attrib_t attr);
test_result ril_utf_test_set_barring_password_003(user_specified_attrib_t attr);
test_result ril_utf_test_set_barring_password_004(user_specified_attrib_t attr);

test_result ril_utf_test_set_preferred_voice_privacy_001(user_specified_attrib_t attr);
test_result ril_utf_test_set_preferred_voice_privacy_002(user_specified_attrib_t attr);
test_result ril_utf_test_get_preferred_voice_privacy_001(user_specified_attrib_t attr);
test_result ril_utf_test_get_preferred_voice_privacy_002(user_specified_attrib_t attr);
test_result ril_utf_test_set_supp_service_notifications_001(user_specified_attrib_t attr);
test_result ril_utf_test_set_supp_service_notifications_002(user_specified_attrib_t attr);
test_result ril_utf_test_cdma_burst_dtmf_001(user_specified_attrib_t attr);
test_result ril_utf_test_cdma_burst_dtmf_002(user_specified_attrib_t attr);
test_result ril_utf_test_explicit_call_transfer_001(user_specified_attrib_t attr);

//==================================================
// IMS tests
//  Description:
//   TODO: add description for IMS tests
//==================================================

test_result ril_utf_ims_mt_call_001_L(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_mt_call_001_L(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_call_001_L(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_call_b2b_001_L(user_specified_attrib_t attr);
test_result ril_utf_ims_drvcc_mo_call_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_multi_001_L(user_specified_attrib_t attr);
test_result ril_utf_ims_call_fail_cause_001(user_specified_attrib_t attr);
test_result ril_utf_ims_call_fail_cause_q850_error_001(user_specified_attrib_t attr);
test_result ril_utf_ims_call_fail_cause_q850_error_002(user_specified_attrib_t attr);
test_result ril_utf_ims_call_fail_cause_q850_error_003(user_specified_attrib_t attr);
test_result ril_utf_ims_call_fail_cause_q850_error_004(user_specified_attrib_t attr);
test_result ril_utf_ims_call_fail_cause(user_specified_attrib_t attr);

test_result ril_utf_ims_mo_conf_001_L(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_conf_002_L(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_conf_003_L(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_conf_004_L(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_conf_005_L(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_call_screen_state_change_L(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_call_screen_state_change_L(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_call_srvcc_screen_state_change_L(user_specified_attrib_t attr);
test_result ril_utf_ims_add_participant(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_call_called_party_alerting(user_specified_attrib_t attr);
test_result ril_utf_ims_conf_participant_status_001(user_specified_attrib_t attr);
test_result ril_utf_ims_conf_participant_status_002(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_call_mid_call_update_001_L(user_specified_attrib_t attr);
test_result ril_utf_ims_alternate_emergency_call_001(user_specified_attrib_t attr);
test_result ril_utf_ims_alternate_emergency_call_002(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_upgrade_vt_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_upgrade_vt_002(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_upgrade_vt_003(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_upgrade_vt_004(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_upgrade_vt_005(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_upgrade_vt_006(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_upgrade_vt_tx_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_upgrade_vt_rx_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_upgrade_rtt_001(user_specified_attrib_t attr);
test_result ril_utf_ims_cancel_modify_call_001(user_specified_attrib_t attr);
test_result ril_utf_ims_cancel_modify_call_002(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_remote_upgrade_vt_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_remote_upgrade_vt_002(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_remote_upgrade_vt_003(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_remote_upgrade_vt_004(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_remote_upgrade_vt_005(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_remote_upgrade_vt_006(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_vt_downgrade_volte_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_vt_downgrade_volte_002(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_vt_downgrade_vt_tx_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_vt_downgrade_vt_rx_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_vt_downgrade_vt_nodir_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_vt_remote_downgrade_volte_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_vt_remote_downgrade_volte_002(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_vt_remote_downgrade_vt_tx_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_vt_remote_downgrade_vt_rx_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_vt_remote_downgrade_vt_nodir_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_hold_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_hold_002(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_hold_003(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_hold_004(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_hold_005(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_hold_006(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_resume_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_resume_002(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_resume_003(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_resume_004(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_resume_005(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_handover_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_handover_002(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_display_text_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_dial_retries_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_additional_call_info_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_vice_call_pull_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_call_sip_uri_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_call_sip_uri_002(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_vt_call_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_vt_call_002(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_vt_call_003(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_vt_call_004(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_vt_call_005(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_vt_call_006(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_vt_call_007(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_vt_call_008(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_vt_call_009(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_vt_call_010(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_vt_call_011(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_call_display_text_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_call_child_number_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_call_child_number_002(user_specified_attrib_t attr);
test_result ril_utf_ims_set_call_waiting_001(user_specified_attrib_t attr);
test_result ril_utf_ims_set_call_waiting_002(user_specified_attrib_t attr);
test_result ril_utf_ims_set_call_waiting_003(user_specified_attrib_t attr);
test_result ril_utf_ims_query_call_waiting_001(user_specified_attrib_t attr);
test_result ril_utf_ims_query_call_waiting_002(user_specified_attrib_t attr);
test_result ril_utf_ims_query_call_waiting_003(user_specified_attrib_t attr);
test_result ril_utf_ims_query_call_waiting_004(user_specified_attrib_t attr);
test_result ril_utf_ims_set_call_forward_status_001(user_specified_attrib_t attr);
test_result ril_utf_ims_set_call_forward_status_002(user_specified_attrib_t attr);
test_result ril_utf_ims_set_call_forward_status_003(user_specified_attrib_t attr);
test_result ril_utf_ims_set_call_forward_status_004(user_specified_attrib_t attr);
test_result ril_utf_ims_set_call_forward_status_005(user_specified_attrib_t attr);
test_result ril_utf_ims_set_call_forward_status_006(user_specified_attrib_t attr);
test_result ril_utf_ims_set_call_forward_status_007(user_specified_attrib_t attr);
test_result ril_utf_ims_query_call_forward_status_001(user_specified_attrib_t attr);
test_result ril_utf_ims_query_call_forward_status_002(user_specified_attrib_t attr);
test_result ril_utf_ims_query_call_forward_status_003(user_specified_attrib_t attr);
test_result ril_utf_ims_query_call_forward_status_004(user_specified_attrib_t attr);
test_result ril_utf_ims_query_call_forward_status_005(user_specified_attrib_t attr);
test_result ril_utf_ims_query_call_forward_status_006(user_specified_attrib_t attr);
test_result ril_utf_ims_query_call_forward_status_007(user_specified_attrib_t attr);
test_result ril_utf_ims_ims_supp_svc_status_001(user_specified_attrib_t attr);
test_result ril_utf_ims_explicit_call_transfer_001(user_specified_attrib_t attr);
test_result ril_utf_ims_explicit_call_transfer_002(user_specified_attrib_t attr);
test_result ril_utf_ims_explicit_call_transfer_003(user_specified_attrib_t attr);
test_result ril_utf_ims_explicit_call_transfer_004(user_specified_attrib_t attr);
test_result ril_utf_ims_explicit_call_transfer_005(user_specified_attrib_t attr);
test_result ril_utf_ims_explicit_call_transfer_006(user_specified_attrib_t attr);
test_result ril_utf_ims_deflect_call_001(user_specified_attrib_t attr);
test_result ril_utf_ims_deflect_call_002(user_specified_attrib_t attr);
test_result ril_utf_ims_dtmf_001(user_specified_attrib_t attr);
test_result ril_utf_ims_dtmf_002(user_specified_attrib_t attr);

test_result ril_utf_set_call_waiting_001(user_specified_attrib_t attr);
test_result ril_utf_set_call_waiting_002(user_specified_attrib_t attr);
test_result ril_utf_set_call_waiting_003(user_specified_attrib_t attr);
test_result ril_utf_query_call_waiting_001(user_specified_attrib_t attr);
test_result ril_utf_query_call_waiting_002(user_specified_attrib_t attr);
test_result ril_utf_query_call_waiting_003(user_specified_attrib_t attr);

// CSVT test cases
test_result ril_utf_mo_csvt_call_L(user_specified_attrib_t attr);
test_result ril_utf_mt_csvt_call_L(user_specified_attrib_t attr);

// Active CS/IMS/CSVT call + SSR test cases
test_result ril_utf_ssr_active_mo_csvt(user_specified_attrib_t attr);
test_result ril_utf_ssr_active_mo_volte(user_specified_attrib_t attr);
test_result ril_utf_ssr_active_mo_cs_call(user_specified_attrib_t attr);

test_result ril_utf_ims_no_srv_dial_fail_L(user_specified_attrib_t attr);
test_result ril_utf_ims_dial_fail_L(user_specified_attrib_t attr);
test_result ril_utf_ims_srv_status_update_001(user_specified_attrib_t attr);
test_result ril_utf_ims_srv_status_update_002(user_specified_attrib_t attr);
test_result ril_utf_ims_srv_status_update_003(user_specified_attrib_t attr);
test_result ril_utf_ims_srv_status_update_004(user_specified_attrib_t attr);
test_result ril_utf_ims_get_clip_001(user_specified_attrib_t attr);
test_result ril_utf_ims_get_clip_002(user_specified_attrib_t attr);
test_result ril_utf_ims_get_clip_003(user_specified_attrib_t attr);
test_result ril_utf_ims_get_colp_001(user_specified_attrib_t attr);
test_result ril_utf_ims_get_colp_002(user_specified_attrib_t attr);
test_result ril_utf_ims_get_colp_003(user_specified_attrib_t attr);
test_result ril_utf_ims_get_colp_004(user_specified_attrib_t attr);
test_result ril_utf_ims_sups_notification_ind_L(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_call_with_tir_mode_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_call_with_tir_mode_002(user_specified_attrib_t attr);


//Ims Config test cases - success
test_result ril_utf_ims_set_and_get_voip_config_items_L(user_specified_attrib_t attr);
test_result ril_utf_ims_set_and_get_user_config_items_L(user_specified_attrib_t attr);
test_result ril_utf_ims_set_and_get_sip_config_items_L(user_specified_attrib_t attr);
test_result ril_utf_ims_set_and_get_reg_mgr_extended_config_items_L(user_specified_attrib_t attr);
test_result ril_utf_ims_set_and_get_client_provisioning_config_items_L(user_specified_attrib_t attr);
test_result ril_utf_ims_set_and_get_qipcall_config_items_L(user_specified_attrib_t attr);
test_result ril_utf_ims_set_and_get_sms_config_items_L(user_specified_attrib_t attr);
test_result ril_utf_ims_set_and_get_presence_config_items_L(user_specified_attrib_t attr);
test_result ril_utf_ims_set_and_get_wifi_call_pref(user_specified_attrib_t attr);
test_result ril_utf_ims_service_enable_config_ind_001(user_specified_attrib_t attr);
test_result ril_utf_ims_config_set_ims_config_001(user_specified_attrib_t attr);
test_result ril_utf_ims_config_set_ims_config_002(user_specified_attrib_t attr);
test_result ril_utf_ims_config_get_ims_config_001(user_specified_attrib_t attr);
test_result ril_utf_ims_config_codec_dynamic_config_001(user_specified_attrib_t attr);
test_result ril_utf_ims_config_codec_dynamic_config_002(user_specified_attrib_t attr);
test_result ril_utf_ims_config_codec_dynamic_config_003(user_specified_attrib_t attr);
test_result ril_utf_ims_config_codec_dynamic_config_004(user_specified_attrib_t attr);
test_result ril_utf_ims_config_voice_dynamic_config_001(user_specified_attrib_t attr);
test_result ril_utf_ims_config_voice_dynamic_config_002(user_specified_attrib_t attr);
test_result ril_utf_ims_config_voice_dynamic_config_003(user_specified_attrib_t attr);
test_result ril_utf_ims_config_voice_dynamic_config_004(user_specified_attrib_t attr);
test_result ril_utf_ims_config_reg_mgr_new_config_001(user_specified_attrib_t attr);
test_result ril_utf_ims_config_reg_mgr_new_config_002(user_specified_attrib_t attr);
test_result ril_utf_ims_config_reg_mgr_new_config_003(user_specified_attrib_t attr);
test_result ril_utf_ims_config_reg_mgr_new_config_004(user_specified_attrib_t attr);
test_result ril_utf_ims_config_sms_new_config_001(user_specified_attrib_t attr);
test_result ril_utf_ims_config_sms_new_config_002(user_specified_attrib_t attr);
test_result ril_utf_ims_config_sms_new_config_003(user_specified_attrib_t attr);
test_result ril_utf_ims_config_sms_new_config_004(user_specified_attrib_t attr);
test_result ril_utf_ims_config_rtp_dynamic_config_001(user_specified_attrib_t attr);
test_result ril_utf_ims_config_rtp_dynamic_config_002(user_specified_attrib_t attr);
test_result ril_utf_ims_config_rtp_dynamic_config_003(user_specified_attrib_t attr);
test_result ril_utf_ims_config_ims_service_enable_config_001(user_specified_attrib_t attr);
test_result ril_utf_ims_config_ims_service_enable_config_002(user_specified_attrib_t attr);
test_result ril_utf_ims_config_ims_service_enable_config_003(user_specified_attrib_t attr);
test_result ril_utf_ims_config_sip_new_config_001(user_specified_attrib_t attr);
test_result ril_utf_ims_config_sip_new_config_002(user_specified_attrib_t attr);
test_result ril_utf_ims_config_sip_new_config_003(user_specified_attrib_t attr);
test_result ril_utf_ims_config_presence_new_config_001(user_specified_attrib_t attr);
test_result ril_utf_ims_config_presence_new_config_002(user_specified_attrib_t attr);
test_result ril_utf_ims_config_presence_new_config_003(user_specified_attrib_t attr);
test_result ril_utf_ims_config_ims_new_config_001(user_specified_attrib_t attr);
test_result ril_utf_ims_config_ims_new_config_002(user_specified_attrib_t attr);
test_result ril_utf_ims_config_ims_new_config_003(user_specified_attrib_t attr);
test_result ril_utf_ims_config_emer_dynamic_config_001(user_specified_attrib_t attr);
test_result ril_utf_ims_config_emer_dynamic_config_002(user_specified_attrib_t attr);
test_result ril_utf_ims_config_emer_dynamic_config_003(user_specified_attrib_t attr);
test_result ril_utf_ims_config_video_dynamic_config_001(user_specified_attrib_t attr);
test_result ril_utf_ims_config_video_dynamic_config_002(user_specified_attrib_t attr);
test_result ril_utf_ims_config_video_dynamic_config_003(user_specified_attrib_t attr);
test_result ril_utf_ims_config_rcs_new_config_001(user_specified_attrib_t attr);
test_result ril_utf_ims_config_rcs_new_config_002(user_specified_attrib_t attr);
test_result ril_utf_ims_config_rcs_new_config_003(user_specified_attrib_t attr);
test_result ril_utf_ims_config_concurrency_config_001(user_specified_attrib_t attr);
test_result ril_utf_ims_config_concurrency_config_003(user_specified_attrib_t attr);
//same as old
test_result ril_utf_ims_config_handover_config_001(user_specified_attrib_t attr);
test_result ril_utf_ims_config_handover_config_002(user_specified_attrib_t attr);
test_result ril_utf_ims_config_handover_config_003(user_specified_attrib_t attr);
test_result ril_utf_ims_service_enable_config_ind_old_002(user_specified_attrib_t attr);
test_result ril_utf_ims_config_ims_service_enable_config_old_001(user_specified_attrib_t attr);

// Allow Data test cases
test_result ril_utf_msim_wcdma_default_allow_data_true(user_specified_attrib_t attr);
test_result ril_utf_msim_wcdma_default_allow_data_true_fail(user_specified_attrib_t attr);
test_result ril_utf_msim_wcdma_default_allow_data_false(user_specified_attrib_t attr);
test_result ril_utf_msim_wcdma_default_allow_data_true_then_false(user_specified_attrib_t attr);
test_result ril_utf_msim_wcdma_default_allow_data_false_then_true(user_specified_attrib_t attr);
test_result ril_utf_msim_wcdma_default_allow_data_true_then_true(user_specified_attrib_t attr);
test_result ril_utf_msim_wcdma_default_indication_first_allow_data_true(user_specified_attrib_t attr);
test_result ril_utf_msim_wcdma_on_demand_allow_data_true(user_specified_attrib_t attr);
test_result ril_utf_msim_wcdma_on_demand_allow_data_true_fail(user_specified_attrib_t attr);
test_result ril_utf_msim_wcdma_on_demand_allow_data_false(user_specified_attrib_t attr);
test_result ril_utf_msim_wcdma_on_demand_allow_data_true_then_false(user_specified_attrib_t attr);
test_result ril_utf_msim_wcdma_on_demand_allow_data_false_then_true(user_specified_attrib_t attr);
test_result ril_utf_msim_wcdma_on_demand_allow_data_true_then_true(user_specified_attrib_t attr);
test_result ril_utf_msim_wcdma_on_demand_indication_first_allow_data_true(user_specified_attrib_t attr);
test_result ril_utf_msim_1x_default_allow_data_true(user_specified_attrib_t attr);
test_result ril_utf_msim_1x_default_allow_data_false(user_specified_attrib_t attr);
test_result ril_utf_msim_1x_on_demand_allow_data_true(user_specified_attrib_t attr);
test_result ril_utf_msim_1x_on_demand_allow_data_false(user_specified_attrib_t attr);
test_result ril_utf_msim_1x_on_demand_indication_first_allow_data_ture_1(user_specified_attrib_t attr);
test_result ril_utf_msim_1x_on_demand_indication_first_allow_data_ture_2(user_specified_attrib_t attr);
test_result ril_utf_msim_1x_on_demand_indication_first_allow_data_ture_3(user_specified_attrib_t attr);
test_result ril_utf_msim_1x_on_demand_indication_first_allow_data_ture_4(user_specified_attrib_t attr);
test_result ril_utf_msim_oos_default_allow_data_true(user_specified_attrib_t attr);
test_result ril_utf_msim_oos_default_allow_data_false(user_specified_attrib_t attr);
test_result ril_utf_msim_oos_on_demand_allow_data_true(user_specified_attrib_t attr);
test_result ril_utf_msim_oos_on_demand_allow_data_false(user_specified_attrib_t attr);

test_result ril_utf_ims_mt_reject_cases(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_precond_vt_L(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_precond_vt_media_paused_L(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_precond_vt_retry_needed_L(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_precond_voice_audio_suspended_L(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_vt_nodir_media_paused_L(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_volte_alert_cap(user_specified_attrib_t attr);
test_result ril_utf_ims_vt_emergency_call(user_specified_attrib_t attr);
test_result ril_utf_ims_em_vt_connected_as_em_voip(user_specified_attrib_t attr);
test_result ril_utf_ims_em_vt_connected_as_em_cs(user_specified_attrib_t attr);
test_result ril_utf_ims_em_vt_downgrade_em_voip(user_specified_attrib_t attr);
test_result ril_utf_ims_vt_downgrade_voip(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_call_end_ho_not_feasible(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_reject_low_battery(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_reject_low_battery(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_call_end_fallback_to_cs(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_call_end_dead_battery(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_call_end_low_battery(user_specified_attrib_t attr);
test_result ril_utf_ims_modem_silent_redial_001_L(user_specified_attrib_t attr);
test_result ril_utf_ims_modem_silent_redial_002_L(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_call_silent_redial_w_orig_fail(user_specified_attrib_t attr);
test_result ril_utf_ims_end_call_err_malformed_001(user_specified_attrib_t attr);
test_result ril_utf_ims_end_call_err_malformed_002(user_specified_attrib_t attr);
test_result ril_utf_ims_encrypted_call_001(user_specified_attrib_t attr);
test_result ril_utf_ims_encrypted_call_002(user_specified_attrib_t attr);
test_result ril_utf_ims_encrypted_call_003(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_call_peer_info_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_call_peer_info_002(user_specified_attrib_t attr);
test_result ril_utf_ims_voice_info_change_001(user_specified_attrib_t attr);
test_result ril_utf_ims_multi_identity_lines_001(user_specified_attrib_t attr);
test_result ril_utf_ims_multi_identity_lines_002(user_specified_attrib_t attr);
test_result ril_utf_ims_multi_identity_lines_003(user_specified_attrib_t attr);
test_result ril_utf_ims_multi_identity_lines_004(user_specified_attrib_t attr);

test_result ril_utf_test_hangup_waiting_or_background_001(user_specified_attrib_t attr);
test_result ril_utf_test_hangup_waiting_or_background_002(user_specified_attrib_t attr);

test_result ril_utf_ims_wifi_calling_status_success(user_specified_attrib_t attr);
test_result ril_utf_ims_wifi_calling_status_failure(user_specified_attrib_t attr);

test_result ril_utf_test_imsa_geolocation_001(user_specified_attrib_t attr);
test_result ril_utf_test_imsa_geolocation_002(user_specified_attrib_t attr);
test_result ril_utf_test_imsa_geolocation_003(user_specified_attrib_t attr);
test_result ril_utf_test_ims_sub_config_001(user_specified_attrib_t attr);
test_result ril_utf_test_ims_sub_config_002(user_specified_attrib_t attr);
test_result ril_utf_test_ims_sub_config_003(user_specified_attrib_t attr);

test_result ril_utf_test_imsa_send_rtt_message(user_specified_attrib_t attr);
test_result ril_utf_test_imsa_send_rtt_message_fail(user_specified_attrib_t attr);
test_result ril_utf_test_imsa_send_rtt_message_fail_empty(user_specified_attrib_t attr);
test_result ril_utf_test_imsa_send_rtt_message_fail_length(user_specified_attrib_t attr);
test_result ril_utf_test_imsa_unsol_rtt_message(user_specified_attrib_t attr);
// CDMA Voice calls
test_result ril_utf_test_cdma_mt_call_001(user_specified_attrib_t attr);
test_result ril_utf_test_cdma_info_rec_001(user_specified_attrib_t attr);
test_result ril_utf_test_cdma_mo_call_and_eme_call_001(user_specified_attrib_t attr);
test_result ril_utf_test_cdma_mo_call_and_eme_call_002(user_specified_attrib_t attr);
test_result ril_utf_test_cdma_mo_call_and_eme_call_003(user_specified_attrib_t attr);
test_result ril_utf_test_cdma_otasp_001(user_specified_attrib_t attr);

//==================================================
// eMBMS tests
//  Description:
//   Test cases for eMBMS
//==================================================
test_result ril_utf_test_embms_get_sib_plmn_001(user_specified_attrib_t attr);
test_result ril_utf_test_embms_get_sib_plmn_002(user_specified_attrib_t attr);
test_result ril_utf_test_embms_get_sib_plmn_003(user_specified_attrib_t attr);
test_result ril_utf_test_embms_get_sib_plmn_004(user_specified_attrib_t attr);
test_result ril_utf_test_embms_get_sib_plmn_005(user_specified_attrib_t attr);
test_result ril_utf_test_embms_get_sib_plmn_006(user_specified_attrib_t attr);
test_result ril_utf_test_embms_get_sib_plmn_007(user_specified_attrib_t attr);

//==================================================
// LTE Direct Discovery tests
//  Description:
//   Test cases for LTE Direct Discovery
//==================================================
test_result ril_utf_lte_direct_disc_get_device_capability_001(user_specified_attrib_t attr);
test_result ril_utf_lte_direct_disc_get_service_status_001(user_specified_attrib_t attr);
test_result ril_utf_lte_direct_disc_publish_001(user_specified_attrib_t attr);
test_result ril_utf_lte_direct_disc_subscribe_001(user_specified_attrib_t attr);
test_result ril_utf_lte_direct_disc_terminate_001(user_specified_attrib_t attr);

#endif //_RIL_UTF_TEST_QMI_RIL_L
