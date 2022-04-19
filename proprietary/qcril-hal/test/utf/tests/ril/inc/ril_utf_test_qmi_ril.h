/******************************************************************************
#  Copyright (c) 2013, 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_test_qmi_ril.h
  @brief   RIL UTF test case declarations

  DESCRIPTION
  All test case declarations should be in this file
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef _RIL_UTF_TEST_QMI_RIL
#define _RIL_UTF_TEST_QMI_RIL

#include "ril_utf_test_includes.h"

#define DEFAULT_BOOT 0
#define SSR_BOOT 1
#define PDC_BOOT 2

//==================================================
// Bootup tests
//  Description:
//   "Boot" here refers to the sequence of actions
//   performed after the rild, main, and qmi_cb threads
//   are started and qmi client initialized has been
//   completed
//==================================================
void ril_utf_bootup_helper(Testcase &t, int boot_config);
void ril_utf_msim_bootup_helper(Testcase &t, int boot_config);
void ril_utf_msim_bootup_helper_1(Testcase &t, int boot_config, int card0_active, int card1_active);
void ril_utf_msim_bootup_helper_2(Testcase &t, int boot_config, int card0_active, int card1_active);
void qmi_ril_partial_bootup_helper_001(Testcase &t);
test_result qmi_ril_test_bootup_001();
test_result qmi_ril_msim_bootup_primary_001();
test_result qmi_ril_msim_bootup_primary_002();
test_result qmi_ril_msim_bootup_secondary_001();
test_result qmi_ril_dummy_bootup();
test_result qmi_ril_msim_dummy_bootup();



//==================================================
// Tests
//  Description:
//  All tests that are run to test a specific call
//  flow.  Boot tests above.
//==================================================

//==================================================
// Test Case for FR11933: Proactive call control for Voice
//==================================================
test_result qmi_ril_test_proactive_call_control_Voice_001(user_specified_attrib_t attr);

//==================================================
// Test Case for FR11933: Proactive call control for WMS
//==================================================
test_result qmi_ril_test_proactive_call_control_WMS_001(user_specified_attrib_t attr);

//==================================================
// UIM tests
//  Description: UIM related tests
//   NOTE: These methods are located in ril_utf_test_qmi_ril_uim.cpp
//==================================================

void load_qmi_card_status(uim_status_change_ind_msg_v01 *init_card_status, Testcase *t, QMIRILTestnode *QMI_node, AndroidTelTestnode *RIL_node);

test_result ril_utf_test_get_card_status_1_1_1(user_specified_attrib_t attr);
test_result ril_utf_test_get_card_status_1_1_2(user_specified_attrib_t attr);
test_result ril_utf_test_get_card_status_1_1_3(user_specified_attrib_t attr);
test_result ril_utf_test_get_card_status_1_1_4(user_specified_attrib_t attr);
test_result ril_utf_test_get_card_status_1_1_5(user_specified_attrib_t attr);
test_result ril_utf_test_get_card_status_1_1_6(user_specified_attrib_t attr);
test_result ril_utf_test_get_card_status_1_1_7(user_specified_attrib_t attr);
test_result ril_utf_test_get_card_status_1_1_8(user_specified_attrib_t attr);
test_result ril_utf_test_get_card_status_1_1_9(user_specified_attrib_t attr);
test_result ril_utf_test_get_card_status_1_3_1(user_specified_attrib_t attr);
test_result ril_utf_test_get_card_status_1_3_2(user_specified_attrib_t attr);
test_result ril_utf_test_get_card_status_1_3_3(user_specified_attrib_t attr);
test_result ril_utf_test_get_card_status_1_3_4(user_specified_attrib_t attr);
test_result ril_utf_test_get_card_status_1_3_5(user_specified_attrib_t attr);
test_result ril_utf_test_get_card_status_1_3_6(user_specified_attrib_t attr);
test_result ril_utf_test_get_card_status_1_3_7(user_specified_attrib_t attr);
test_result ril_utf_test_get_card_status_1_3_8(user_specified_attrib_t attr);
test_result ril_utf_test_get_card_status_1_3_9(user_specified_attrib_t attr);
test_result ril_utf_test_card_status_change(user_specified_attrib_t attr);

//==================================================
// Voice tests
//  Description:
//   TODO: add description for Voice tests
//==================================================
test_result ril_utf_test_mo_call_001(user_specified_attrib_t attr);
test_result ril_utf_test_mo_call_002(user_specified_attrib_t attr);
test_result ril_utf_test_mo_call_003(user_specified_attrib_t attr);
test_result ril_utf_test_mo_call_004(user_specified_attrib_t attr);
test_result ril_utf_test_mo_call_005(user_specified_attrib_t attr);
test_result ril_utf_test_mo_call_006(user_specified_attrib_t attr);
test_result ril_utf_test_mo_call_007(user_specified_attrib_t attr);
test_result ril_utf_test_mo_call_008(user_specified_attrib_t attr);
test_result ril_utf_test_mo_call_009(user_specified_attrib_t attr);
test_result ril_utf_test_mo_call_hangup_before_dial_resp(user_specified_attrib_t attr);
test_result ril_utf_test_mo_call_hangup_before_cc_in_prog(user_specified_attrib_t attr);
test_result ril_utf_test_mt_call_001(user_specified_attrib_t attr);
test_result ril_utf_test_mt_call_002(user_specified_attrib_t attr);
test_result ril_utf_test_mt_call_reject_001(user_specified_attrib_t attr);
test_result ril_utf_test_mo_call_delay_in_origination_indication(user_specified_attrib_t attr);
test_result ril_utf_test_mo_call_delay_in_dial_resp(user_specified_attrib_t attr);
test_result ril_utf_test_mo_call_no_origination_indication(user_specified_attrib_t attr);
test_result ril_utf_test_mo_call_no_origination_indication_for_70_secs(user_specified_attrib_t attr);
test_result ril_utf_test_mo_call_no_dial_resp_for_70_secs(user_specified_attrib_t attr);
test_result ril_utf_test_mo_call_no_origination_indication_for_74_secs(user_specified_attrib_t attr);
test_result ril_utf_test_mo_call_no_dial_resp_for_74_secs(user_specified_attrib_t attr);
test_result ril_utf_test_mo_call_redir_num(user_specified_attrib_t attr);
test_result ril_utf_test_mo_call_speech_codec_ind(user_specified_attrib_t attr);
test_result ril_utf_test_mo_call_dial_modified_to_dial(user_specified_attrib_t attr);
test_result ril_utf_test_reject_cause_21_001(user_specified_attrib_t attr);
test_result ril_utf_test_reject_cause_21_501(user_specified_attrib_t attr);
test_result ril_utf_test_reject_cause_21_002(user_specified_attrib_t attr);
test_result ril_utf_test_reject_cause_21_003(user_specified_attrib_t attr);
test_result ril_utf_test_reject_cause_21_503(user_specified_attrib_t attr);
test_result ril_utf_test_reject_cause_21_004(user_specified_attrib_t attr);
test_result ril_utf_test_reject_cause_21_504(user_specified_attrib_t attr);
test_result ril_utf_test_cs_call_number_handle_in_conn_party_num_tlv(user_specified_attrib_t attr);
test_result ril_utf_test_mo_call_fail_001(user_specified_attrib_t attr);
test_result ril_utf_test_mo_call_fail_002(user_specified_attrib_t attr);
test_result ril_utf_test_mo_call_fail_003(user_specified_attrib_t attr);
test_result ril_utf_test_mo_call_fail_004(user_specified_attrib_t attr);
test_result ril_utf_test_mo_call_fail_005(user_specified_attrib_t attr);
test_result ril_utf_test_mo_call_fail_006(user_specified_attrib_t attr);
test_result ril_utf_test_hangup_fail_001(user_specified_attrib_t attr);
test_result ril_utf_test_timeout_dial_001(user_specified_attrib_t attr);
test_result ril_utf_mo_call_with_handover_ind(user_specified_attrib_t attr);
test_result ril_utf_test_wrong_fail_cause_001(user_specified_attrib_t attr);
test_result ril_utf_set_call_forward_status(user_specified_attrib_t attr);
test_result ril_utf_query_call_forward_status_001(user_specified_attrib_t attr);
test_result ril_utf_query_call_forward_status_002(user_specified_attrib_t attr);
test_result ril_utf_query_call_forward_status_003(user_specified_attrib_t attr);
test_result ril_utf_query_call_forward_status_004(user_specified_attrib_t attr);
test_result ril_utf_query_call_forward_status_005(user_specified_attrib_t attr);
test_result ril_utf_ril_set_tty_mode_001(user_specified_attrib_t attr);
test_result ril_utf_ril_set_tty_mode_002(user_specified_attrib_t attr);
test_result ril_utf_ril_get_tty_mode_001(user_specified_attrib_t attr);
test_result ril_utf_ril_get_tty_mode_002(user_specified_attrib_t attr);
test_result ril_utf_get_clir_error(user_specified_attrib_t attr);
test_result ril_utf_test_mo_call_screen_state_change(user_specified_attrib_t attr);
test_result ril_utf_test_add_participant_failure(user_specified_attrib_t attr);
test_result ril_utf_test_last_call_failcause(user_specified_attrib_t attr);
test_result ril_utf_test_hangup_001(user_specified_attrib_t attr);

// CDMA Voice calls
test_result ril_utf_test_cdma_mo_call_remote_name(user_specified_attrib_t attr);

test_result ril_utf_test_mute_001(user_specified_attrib_t attr);
test_result ril_utf_test_set_unsol_cell_info_list_rate_001(user_specified_attrib_t attr);

//==================================================
// IMS tests
//  Description:
//   TODO: add description for IMS tests
//==================================================
test_result ril_utf_ims_mt_call_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_call_002(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_call_003(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_call_004(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_call_005(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_call_006(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_call_007(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_call_008(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_call_009(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_call_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_call_002(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_call_003(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_call_004(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_call_005(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_call_006(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_call_007(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_call_008(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_call_009(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_call_010(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_call_011(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_call_012(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_call_013(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_emergency_dial_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_emergency_dial_002(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_emergency_dial_003(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_conf_uri_call_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_conf_uri_call_002(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_conf_uri_call_003(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_mt_call_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_multi_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_conf_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_conf_002(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_conf_003(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_conf_004(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_conf_005(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_conf_006(user_specified_attrib_t attr);
test_result ril_utf_ims_volte_conf_call_upgrade_vt_001(user_specified_attrib_t attr);
test_result ril_utf_ims_volte_conf_call_remote_upgrade_vt_001(user_specified_attrib_t attr);
test_result ril_utf_ims_vt_conf_call_downgrade_voip_001(user_specified_attrib_t attr);
test_result ril_utf_ims_vt_conf_call_remote_downgrade_voip_001(user_specified_attrib_t attr);
test_result ril_utf_ims_vice_dialog_info_001(user_specified_attrib_t attr);
test_result ril_utf_ims_auto_rejected_incoming_call_end_ind_001(user_specified_attrib_t attr);
test_result ril_utf_ims_vt_call_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_long_number_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_long_number_001(user_specified_attrib_t attr);
test_result ril_utf_ims_srvcc_mo_call_001(user_specified_attrib_t attr);
test_result ril_utf_ims_srvcc_mo_call_002(user_specified_attrib_t attr);
test_result ril_utf_ims_srvcc_mo_call_003(user_specified_attrib_t attr);
test_result ril_utf_ims_srvcc_conf_call_001(user_specified_attrib_t attr);
test_result ril_utf_ims_srvcc_conf_call_002(user_specified_attrib_t attr);
test_result ril_utf_ims_srvcc_conf_call_003(user_specified_attrib_t attr);
test_result ril_utf_ims_srvcc_conf_call_004(user_specified_attrib_t attr);
test_result ril_utf_ims_srvcc_conf_call_005(user_specified_attrib_t attr);
test_result ril_utf_ims_exit_ecbm_001(user_specified_attrib_t attr);
test_result ril_utf_ims_emergency_call_001(user_specified_attrib_t attr);
test_result ril_utf_ims_emergency_call_002(user_specified_attrib_t attr);
test_result ril_utf_ims_emergency_call_003(user_specified_attrib_t attr);
test_result ril_utf_ims_callfailcauseresponse_001(user_specified_attrib_t attr);
test_result ril_utf_ims_callfailcauseresponse_002(user_specified_attrib_t attr);
test_result ril_utf_ims_callfailcauseresponse_003(user_specified_attrib_t attr);
test_result ril_utf_ims_callfailcauseresponse_004(user_specified_attrib_t attr);
test_result ril_utf_ims_callfailcauseresponse_005(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_vt_nodir(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_vt_nodir(user_specified_attrib_t attr);
test_result ril_utf_ims_get_clir_success(user_specified_attrib_t attr);
test_result ril_utf_ims_get_clir_error(user_specified_attrib_t attr);
test_result ril_utf_ims_colr_success(user_specified_attrib_t attr);
test_result ril_utf_ims_colr_failure(user_specified_attrib_t attr);
test_result ril_utf_ims_mwi_ind_01(user_specified_attrib_t attr);
test_result ril_utf_ims_mwi_ind_02(user_specified_attrib_t attr);
test_result ril_utf_ims_mwi_ind_03(user_specified_attrib_t attr);
test_result ril_utf_feature_autom_001(user_specified_attrib_t attr);
test_result ril_utf_ims_sups_notification_ind_001(user_specified_attrib_t attr);
test_result ril_utf_ims_sups_notification_ind_002(user_specified_attrib_t attr);
test_result ril_utf_ims_voice_privacy_ind_002(user_specified_attrib_t attr);
test_result ril_utf_ims_tty_ind_001(user_specified_attrib_t attr);
test_result ril_utf_ims_conf_info_ind_001(user_specified_attrib_t attr);
test_result ril_utf_ims_conf_info_ind_002(user_specified_attrib_t attr);
test_result ril_utf_ims_conf_info_ind_003(user_specified_attrib_t attr);
test_result ril_utf_ims_wlan_mo_call_001(user_specified_attrib_t attr);
test_result ril_utf_ims_wlan_mo_call_002(user_specified_attrib_t attr);
test_result ril_utf_ims_wlan_mo_vt_call_001(user_specified_attrib_t attr);
test_result ril_utf_ims_wlan_mo_vt_tx_call_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_precond_vt(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_precond_volte(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_precond_vt_tx(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_precond_vt_rx(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_precond_vt(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_precond_volte(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_precond_vt_tx(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_precond_vt_rx(user_specified_attrib_t attr);
test_result ril_utf_ims_timer_call_forwarding_status(user_specified_attrib_t attr);
test_result ril_utf_ims_call_forwarding_status(user_specified_attrib_t attr);
test_result ril_utf_ims_set_tty_mode(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_call_end_imm_orig(user_specified_attrib_t attr);
test_result ril_utf_test_set_local_call_hold_001(user_specified_attrib_t attr);
test_result ril_utf_oemhook_unsol_cdma_burst_dtmf_001(user_specified_attrib_t attr);
test_result ril_utf_oemhook_unsol_cdma_cont_dtmf_001(user_specified_attrib_t attr);
test_result ril_utf_oemhook_unsol_nss_release_001(user_specified_attrib_t attr);
test_result ril_utf_oemhook_unsol_extended_dbm_intl_001(user_specified_attrib_t attr);
test_result ril_utf_ims_supp_service_status_001(user_specified_attrib_t attr);
test_result ril_utf_ims_supp_service_status_002(user_specified_attrib_t attr);
test_result ril_utf_ims_supp_service_status_003(user_specified_attrib_t attr);
test_result ril_utf_ims_supp_service_status_004(user_specified_attrib_t attr);
test_result ril_utf_ims_supp_service_status_005(user_specified_attrib_t attr);
test_result ril_utf_ims_supp_service_status_006(user_specified_attrib_t attr);
test_result ril_utf_ims_supp_service_status_007(user_specified_attrib_t attr);
test_result ril_utf_ims_supp_service_status_008(user_specified_attrib_t attr);
test_result ril_utf_ims_supp_service_status_009(user_specified_attrib_t attr);
test_result ril_utf_ims_supp_service_status_010(user_specified_attrib_t attr);
test_result ril_utf_ims_set_supp_svc_notification_001(user_specified_attrib_t attr);
test_result ril_utf_ims_ussd_failed_notification_001(user_specified_attrib_t attr);
test_result ril_utf_ims_ussd_failed_notification_002(user_specified_attrib_t attr);

test_result ril_utf_stk_cc_dial_modified_to_ss_001(user_specified_attrib_t attr);
test_result ril_utf_stk_cc_dial_modified_to_ss_002(user_specified_attrib_t attr);
test_result ril_utf_stk_cc_dial_modified_to_ss_003(user_specified_attrib_t attr);
test_result ril_utf_stk_cc_set_call_forward_ss_modified_to_ss_001(user_specified_attrib_t attr);
test_result ril_utf_stk_cc_set_call_forward_ss_modified_to_ss_002(user_specified_attrib_t attr);
test_result ril_utf_stk_cc_set_call_forward_ss_modified_to_ussd_001(user_specified_attrib_t attr);
test_result ril_utf_stk_cc_set_call_forward_ss_modified_to_ussd_002(user_specified_attrib_t attr);
test_result ril_utf_stk_cc_query_clip_ss_modified_to_ussd_001(user_specified_attrib_t attr);
test_result ril_utf_stk_cc_query_clip_ss_modified_to_ussd_002(user_specified_attrib_t attr);
test_result ril_utf_stk_cc_send_ussd_ussd_modified_to_ss_001(user_specified_attrib_t attr);

//==================================================
// SMS tests
//  Description:
//   TODO: add description for SMS tests
//==================================================
test_result ril_utf_sms_mo_001(user_specified_attrib_t attr);
test_result ril_utf_sms_mt_001(user_specified_attrib_t attr);
test_result ril_utf_sms_mt_002(user_specified_attrib_t attr);
test_result ril_utf_sms_mt_003(user_specified_attrib_t attr);
test_result ril_utf_sms_mt_004(user_specified_attrib_t attr);
test_result ril_utf_sms_delete_sms_on_sim_001(user_specified_attrib_t attr);
test_result ril_utf_sms_delete_cdma_sms_on_ruim_001(user_specified_attrib_t attr);
test_result ril_utf_sms_verify_single_page_gsm_cmas(user_specified_attrib_t attr);
test_result ril_utf_sms_verify_single_page_lte_cmas(user_specified_attrib_t attr);
test_result ril_utf_sms_verify_single_page_lte_cmas_with_wac(user_specified_attrib_t attr);
test_result ril_utf_sms_verify_multi_page_gsm_cmas(user_specified_attrib_t attr);
test_result ril_utf_sms_verify_multi_page_lte_cmas(user_specified_attrib_t attr);

test_result ril_utf_ims_mo_3gpp_sms_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_3gpp_sms_002(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_3gpp_sms_003(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_3gpp_sms_001(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_3gpp_sms_002(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_3gpp_sms_003(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_3gpp_sms_004(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_3gpp_sms_005(user_specified_attrib_t attr);

test_result ril_utf_get_smsc_address_001(user_specified_attrib_t attr);
test_result ril_utf_gsm_sms_broadcast_activation_001(user_specified_attrib_t attr);
test_result ril_utf_cdma_sms_broadcast_activation_001(user_specified_attrib_t attr);
test_result ril_utf_report_sms_memory_status_001(user_specified_attrib_t attr);
test_result ril_utf_get_gsm_broadcast_config_001(user_specified_attrib_t attr);
test_result ril_utf_set_gsm_broadcast_config_001(user_specified_attrib_t attr);
test_result ril_utf_get_cdma_broadcast_config_001(user_specified_attrib_t attr);
test_result ril_utf_set_cdma_broadcast_config_001(user_specified_attrib_t attr);
test_result ril_utf_write_sms_to_sim_001(user_specified_attrib_t attr);
test_result ril_utf_unsol_sms_memory_full_001(user_specified_attrib_t attr);
test_result ril_utf_unsol_sms_memory_full_002(user_specified_attrib_t attr);


//==================================================
// Retrospective Gerrits for Boulder, SD tests
//  Description: Testing qcril master branch past gerrits
//==================================================
test_result ril_utf_retro_gerrit_355643_001(user_specified_attrib_t attr);
test_result ril_utf_retro_gerrit_355643_002(user_specified_attrib_t attr);
test_result ril_utf_retro_gerrit_355643_003(user_specified_attrib_t attr);
test_result ril_utf_retro_gerrit_355643_004(user_specified_attrib_t attr);
test_result ril_utf_retro_gerrit_391289_001(user_specified_attrib_t attr);
test_result ril_utf_retro_gerrit_391289_002(user_specified_attrib_t attr);
test_result ril_utf_retro_gerrit_355735_001(user_specified_attrib_t attr);
test_result ril_utf_retro_gerrit_260696_001(user_specified_attrib_t attr);
test_result ril_utf_retro_gerrit_318335_001(user_specified_attrib_t attr);
test_result ril_utf_retro_gerrit_368730_001(user_specified_attrib_t attr);
test_result ril_utf_retro_gerrit_368730_002(user_specified_attrib_t attr);
test_result ril_utf_retro_gerrit_284454_001(user_specified_attrib_t attr);
test_result ril_utf_retro_gerrit_347631_001(user_specified_attrib_t attr);
test_result ril_utf_retro_gerrit_338798_001(user_specified_attrib_t attr);
test_result ril_utf_retro_gerrit_338798_002(user_specified_attrib_t attr);
test_result ril_utf_retro_gerrit_334432_001(user_specified_attrib_t attr);
test_result ril_utf_retro_gerrit_334432_002(user_specified_attrib_t attr);
test_result ril_utf_retro_gerrit_275073_001(user_specified_attrib_t attr);
test_result ril_utf_retro_gerrit_275073_002(user_specified_attrib_t attr);
test_result ril_utf_retro_gerrit_219098_001(user_specified_attrib_t attr);
test_result ril_utf_retro_gerrit_253102_001(user_specified_attrib_t attr);
test_result ril_utf_retro_gerrit_253102_002(user_specified_attrib_t attr);
test_result ril_utf_retro_gerrit_141083_001(user_specified_attrib_t attr);
test_result ril_utf_retro_gerrit_338792_001(user_specified_attrib_t attr);
test_result ril_utf_retro_gerrit_312517_001(user_specified_attrib_t attr);

//==================================================
// NAS tests
//  Description:
//   TODO: add description for NAS tests
//==================================================
test_result ril_utf_test_nw_state_chg_001(user_specified_attrib_t attr);
test_result ril_utf_test_system_selection_preference(user_specified_attrib_t attr);

//==================================================
// OEM tests
//  Description:
//   TODO: add description for OEM tests
//==================================================
test_result ril_utf_oem_testcase_001(user_specified_attrib_t attr);

test_result ril_utf_clip_testcase_001(user_specified_attrib_t attr);
test_result ril_utf_clip_testcase_002(user_specified_attrib_t attr);
test_result ril_utf_clip_testcase_003(user_specified_attrib_t attr);
test_result ril_utf_facility_lock_testcase_001(user_specified_attrib_t attr);
test_result ril_utf_facility_lock_testcase_002(user_specified_attrib_t attr);
test_result ril_utf_facility_lock_testcase_003(user_specified_attrib_t attr);
test_result ril_utf_facility_lock_testcase_004(user_specified_attrib_t attr);
test_result ril_utf_facility_lock_testcase_005(user_specified_attrib_t attr);
test_result ril_utf_facility_lock_testcase_006(user_specified_attrib_t attr);
test_result ril_utf_facility_lock_testcase_007(user_specified_attrib_t attr);
test_result ril_utf_setup_answer_001(user_specified_attrib_t attr);
test_result ril_utf_setup_answer_002(user_specified_attrib_t attr);

test_result ril_utf_ims_reg_state_change_001(user_specified_attrib_t attr);
test_result ril_utf_ims_reg_state_change_002(user_specified_attrib_t attr);
test_result ril_utf_ims_reg_state_change_003(user_specified_attrib_t attr);
test_result ril_utf_ims_reg_state_change_004(user_specified_attrib_t attr);
test_result ril_utf_ims_reg_state_change_005(user_specified_attrib_t attr);
test_result ril_utf_ims_reg_state_change_006(user_specified_attrib_t attr);
test_result ril_utf_ims_reg_state_change_007(user_specified_attrib_t attr);
test_result ril_utf_ims_reg_state_change_008(user_specified_attrib_t attr);
test_result ril_utf_ims_reg_state_change_009(user_specified_attrib_t attr);
test_result ril_utf_ims_set_service_status_001(user_specified_attrib_t attr);
test_result ril_utf_ims_set_service_status_002(user_specified_attrib_t attr);
test_result ril_utf_ims_set_service_status_003(user_specified_attrib_t attr);
test_result ril_utf_ims_set_service_status_004(user_specified_attrib_t attr);
test_result ril_utf_ims_set_service_status_005(user_specified_attrib_t attr);
test_result ril_utf_ims_set_service_status_006(user_specified_attrib_t attr);
test_result ril_utf_ims_set_service_status_007(user_specified_attrib_t attr);
test_result ril_utf_ims_set_service_status_008(user_specified_attrib_t attr);
test_result ril_utf_ims_set_service_status_009(user_specified_attrib_t attr);
test_result ril_utf_ims_set_service_status_010(user_specified_attrib_t attr);
test_result ril_utf_ims_set_service_status_011(user_specified_attrib_t attr);
test_result ril_utf_ims_set_service_status_012(user_specified_attrib_t attr);
test_result ril_utf_ims_set_service_status_013(user_specified_attrib_t attr);
test_result ril_utf_ims_set_service_status_014(user_specified_attrib_t attr);
test_result ril_utf_ims_set_service_status_015(user_specified_attrib_t attr);
test_result ril_utf_ims_set_service_status_016(user_specified_attrib_t attr);
test_result ril_utf_ims_registration_state_001(user_specified_attrib_t attr);
test_result ril_utf_ims_registration_state_002(user_specified_attrib_t attr);
test_result ril_utf_ims_block_status_ind_001(user_specified_attrib_t attr);
test_result ril_utf_ims_block_status_ind_002(user_specified_attrib_t attr);

//==================================================
// MSIM tests
//  Description:
//   Test cases for multi sim scenarios
//==================================================
test_result ril_utf_msim_dsds_001(user_specified_attrib_t attr);
test_result ril_utf_msim_dsds_secondary_001(user_specified_attrib_t attr);

//==================================================
// DB tests
//  Description:
//   Test cases for db support
//==================================================
test_result ril_utf_db_001(user_specified_attrib_t attr);

//==================================================
// SSR tests
//  Description:
//   Test cases for sub system restart
//==================================================
test_result ril_utf_ssr_001(user_specified_attrib_t attr);
test_result ril_utf_ssr_002(user_specified_attrib_t attr);
//==================================================
// Test Case for FR18752: Voice call converted to ussd
//==================================================
test_result ril_utf_test_voice_call_converted_ussd_in_alpha_null_case(user_specified_attrib_t attr);

//==================================================
// Test Case for FR18752: Voice call converted to ss
//==================================================
test_result ril_utf_test_voice_call_converted_ss_in_alpha_null_case(user_specified_attrib_t attr);
test_result ril_utf_test_oem_request_cdma_set_subscription_source_testcase_1(user_specified_attrib_t attr);
test_result ril_utf_test_oem_request_cdma_set_subscription_source_testcase_2(user_specified_attrib_t attr);
test_result ril_utf_test_cdma_set_subscription_source_1 (user_specified_attrib_t attr );
test_result ril_utf_test_cdma_set_subscription_source_2 (user_specified_attrib_t attr );
test_result ril_utf_test_cdma_set_subscription_source_3 (user_specified_attrib_t attr );
test_result ril_utf_test_cdma_set_subscription_source_4 (user_specified_attrib_t attr );
test_result ril_utf_test_cdma_get_subscription_source_1 (user_specified_attrib_t attr );
test_result ril_utf_test_cdma_get_subscription_source_2 (user_specified_attrib_t attr );
test_result ril_utf_test_cdma_get_subscription_source_3 (user_specified_attrib_t attr );
test_result ril_utf_test_cdma_get_subscription_source_4 (user_specified_attrib_t attr );
test_result ril_utf_test_cdma_set_roaming_preference_1  (user_specified_attrib_t attr );
test_result ril_utf_test_cdma_set_roaming_preference_2  (user_specified_attrib_t attr );
test_result ril_utf_test_cdma_set_roaming_preference_3  (user_specified_attrib_t attr );
test_result ril_utf_test_cdma_set_roaming_preference_4  (user_specified_attrib_t attr );
test_result ril_utf_test_cdma_query_roaming_preference_1(user_specified_attrib_t attr );
test_result ril_utf_test_cdma_query_roaming_preference_2(user_specified_attrib_t attr );
test_result ril_utf_test_cdma_query_roaming_preference_3(user_specified_attrib_t attr );
test_result ril_utf_test_cdma_query_roaming_preference_4(user_specified_attrib_t attr );
test_result ril_utf_test_get_preferred_network_type(user_specified_attrib_t attr);
test_result ril_utf_test_get_preferred_network_type_failure(user_specified_attrib_t attr);
test_result ril_utf_test_consecutive_set_preferred_network_type_testcase_1( user_specified_attrib_t attr );
test_result ril_utf_test_consecutive_set_preferred_network_type_testcase_2( user_specified_attrib_t attr );
test_result ril_utf_test_set_preferred_network_type_resp_failure( user_specified_attrib_t attr );
test_result ril_utf_test_set_preferred_network_type_without_ind( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_1( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_2( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_3( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_4( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_5( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_6( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_7( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_8( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_9( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_10( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_11( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_12( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_13( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_14( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_15( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_16( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_17( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_18( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_19( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_20( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_21( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_22( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_23( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_24( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_25( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_26( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_27( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_28_1( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_28_2( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_28_3( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_28_4( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_29_1( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_29_2( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_29_3( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_29_4( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_30( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_31( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_32_1( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_32_2( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_33_1( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_33_2( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_34( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_35( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_36( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_37( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_38( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_39( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_40( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_41( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_42( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_43( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_44( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_45( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_46( user_specified_attrib_t attr );
test_result ril_utf_test_voice_radio_tech_testcase_47( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_1( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_2( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_3( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_4( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_5( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_6( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_7( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_8( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_9( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_10( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_11( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_12( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_13( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_14( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_15( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_16( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_17( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_18( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_19( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_20( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_21( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_22( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_23( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_24( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_25( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_26( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_27( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_28( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_29( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_30( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_31( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_32( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_33( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_34( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_35( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_36( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_37( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_38( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_39( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_40( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_41( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_42( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_43( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_44( user_specified_attrib_t attr );
test_result ril_utf_test_voice_registration_state_testcase_45( user_specified_attrib_t attr );
test_result ril_utf_test_data_registration_state_testcase_1( user_specified_attrib_t attr );
test_result ril_utf_test_data_registration_state_testcase_2( user_specified_attrib_t attr );
test_result ril_utf_test_data_registration_state_testcase_3( user_specified_attrib_t attr );
test_result ril_utf_test_data_registration_state_testcase_4( user_specified_attrib_t attr );
test_result ril_utf_test_data_registration_state_testcase_5( user_specified_attrib_t attr );
test_result ril_utf_test_data_registration_state_testcase_6( user_specified_attrib_t attr );
test_result ril_utf_test_data_registration_state_testcase_7( user_specified_attrib_t attr );
test_result ril_utf_test_data_registration_state_testcase_8( user_specified_attrib_t attr );
test_result ril_utf_test_data_registration_state_testcase_9( user_specified_attrib_t attr );
test_result ril_utf_test_data_registration_state_testcase_10( user_specified_attrib_t attr );
test_result ril_utf_test_data_registration_state_testcase_11( user_specified_attrib_t attr );
test_result ril_utf_test_data_registration_state_testcase_12( user_specified_attrib_t attr );
test_result ril_utf_test_data_registration_state_testcase_13( user_specified_attrib_t attr );
test_result ril_utf_test_data_registration_state_testcase_14( user_specified_attrib_t attr );
test_result ril_utf_test_data_registration_state_testcase_15( user_specified_attrib_t attr );
test_result ril_utf_test_data_registration_state_testcase_16( user_specified_attrib_t attr );
test_result ril_utf_test_data_registration_state_testcase_17( user_specified_attrib_t attr );
test_result ril_utf_test_data_registration_state_testcase_18( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_1( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_2( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_3( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_4( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_5( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_6( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_7( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_8( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_9( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_10( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_11( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_12( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_13( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_14( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_15( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_16( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_17( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_18( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_19( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_20( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_21( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_22( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_23( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_24( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_25( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_26( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_27( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_28( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_29( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_30( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_31( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_32( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_33( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_34( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_35( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_36( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_37( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_38( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_39( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_40( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_41( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_42( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_43( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_44( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_45( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_46( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_47( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_48( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_49( user_specified_attrib_t attr );
test_result ril_utf_test_operator_name_testcase_50( user_specified_attrib_t attr );
test_result ril_utf_test_query_network_scan_testcase_1( user_specified_attrib_t attr );
test_result ril_utf_test_query_network_scan_testcase_2( user_specified_attrib_t attr );
test_result ril_utf_test_query_network_scan_testcase_3( user_specified_attrib_t attr );
test_result ril_utf_test_query_network_scan_testcase_4( user_specified_attrib_t attr );
test_result ril_utf_test_query_network_scan_testcase_5( user_specified_attrib_t attr );
test_result ril_utf_test_query_network_scan_testcase_6( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_1( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_2( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_3( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_4( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_5( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_6( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_7( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_8( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_9( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_10( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_11( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_12( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_13( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_14( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_15( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_16( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_17( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_18( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_19( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_20( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_21( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_22( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_23( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_24( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_25( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_26( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_27( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_28( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_29( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_30( user_specified_attrib_t attr );
test_result ril_utf_test_signal_strength_testcase_31( user_specified_attrib_t attr );
test_result ril_utf_test_multisim_config_testcase_1( user_specified_attrib_t attr );
test_result ril_utf_test_set_uicc_subscription_testcase_1( user_specified_attrib_t attr );
test_result ril_utf_test_set_uicc_subscription_testcase_2( user_specified_attrib_t attr );
test_result ril_utf_test_set_uicc_subscription_testcase_3( user_specified_attrib_t attr );
test_result ril_utf_test_set_network_selection_manual_test_case1( user_specified_attrib_t attr );
test_result ril_utf_test_ims_registration_state_testcase_1( user_specified_attrib_t attr );
test_result ril_utf_test_ims_registration_state_testcase_2( user_specified_attrib_t attr );
test_result ril_utf_test_ims_registration_state_testcase_3( user_specified_attrib_t attr );
test_result ril_utf_test_ims_registration_state_testcase_4( user_specified_attrib_t attr );
test_result ril_utf_test_ims_registration_state_testcase_5( user_specified_attrib_t attr );
test_result ril_utf_test_ims_registration_state_testcase_6( user_specified_attrib_t attr );
test_result ril_utf_test_ims_registration_state_testcase_7( user_specified_attrib_t attr );
test_result ril_utf_test_ims_send_sms_testcase_1( user_specified_attrib_t attr );
test_result ril_utf_test_ims_send_sms_testcase_2( user_specified_attrib_t attr );
test_result ril_utf_test_ims_send_sms_testcase_3( user_specified_attrib_t attr );
test_result ril_utf_test_ims_send_sms_testcase_4( user_specified_attrib_t attr );
test_result ril_utf_test_ims_send_sms_testcase_5( user_specified_attrib_t attr );
test_result ril_utf_test_ims_send_sms_testcase_6( user_specified_attrib_t attr );
test_result ril_utf_test_ims_send_sms_testcase_7( user_specified_attrib_t attr );
test_result ril_utf_test_ims_send_sms_testcase_8( user_specified_attrib_t attr );
test_result ril_utf_test_screen_state_1( user_specified_attrib_t attr );
test_result ril_utf_test_screen_state_2( user_specified_attrib_t attr );
test_result ril_utf_ims_mt_call_display_text(user_specified_attrib_t attr);
test_result ril_utf_test_set_lte_tune_away_testcase_1(user_specified_attrib_t attr);
test_result ril_utf_test_set_lte_tune_away_testcase_2(user_specified_attrib_t attr);
test_result ril_utf_test_set_lte_tune_away_testcase_3(user_specified_attrib_t attr);
test_result ril_utf_test_set_lte_tune_away_testcase_4(user_specified_attrib_t attr);
test_result ril_utf_ims_mt_call_voip_caller_name(user_specified_attrib_t attr);
test_result ril_utf_test_lte_apn_detach_then_attach(user_specified_attrib_t attr);
test_result ril_utf_test_lte_apn_no_reattach_needed(user_specified_attrib_t attr);
test_result ril_utf_test_lte_apn_detach_error (user_specified_attrib_t attr);
test_result ril_utf_test_lte_apn_attach_error (user_specified_attrib_t attr);
test_result ril_utf_test_lte_apn_lte_ps_mode_detach_no_service_domain (user_specified_attrib_t attr);
test_result ril_utf_test_lte_apn_detach_two_indications(user_specified_attrib_t attr);
test_result ril_utf_test_lte_apn_multiple_set_init_attach_apn_requests(user_specified_attrib_t attr);
test_result ril_utf_test_lte_apn_detach_no_indication (user_specified_attrib_t attr);
test_result ril_utf_test_lte_apn_attach_no_indication (user_specified_attrib_t attr);
test_result ril_utf_test_lte_apn_detach_delayed_indication (user_specified_attrib_t attr);
test_result ril_utf_test_lte_apn_attach_delayed_indication (user_specified_attrib_t attr);
test_result ril_utf_test_lte_apn_empty_apn_name_gerrit_1075551(user_specified_attrib_t attr);
test_result ril_utf_ims_mo_call_end_reason_text(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_msim_success(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_msim_success_with_iccid_only(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_no_sim_without_hw_mbn_init(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_no_sim_with_hw_mbn_init(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_one_sim_001(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_msim_with_only_card_0_active(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_msim_with_only_card_1_active(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_hw_get_config_fail(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_hw_get_config_ind_with_no_config_id(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_hw_deactivate_config_fail(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_hw_delete_config_fail(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_hw_load_config_fail(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_hw_set_config_fail(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_hw_activate_config_fail(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_1st_sw_get_config_fail(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_1st_sw_get_config_info_fail(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_1st_sw_get_config_info_ind_with_different_version(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_1st_sw_get_config_info_ind_with_different_minor_version(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_1st_sw_get_config_info_ind_with_different_same_version(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_1st_sw_deactivate_config_fail(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_1st_sw_2nd_delete_config_fail(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_1st_sw_load_config_fail(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_1st_sw_set_config_fail(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_2nd_sw_get_config_fail(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_2nd_sw_get_config_info_fail(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_2nd_sw_deactivate_config_fail(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_2nd_sw_2nd_delete_config_fail(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_2nd_sw_load_config_fail(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_2nd_sw_set_config_fail(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_2nd_sw_activate_config_fail(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_no_mbn_config(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_no_hw_mbn_file(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_no_sw_mbn_file(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_copy_not_complete(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_nova_sim_with_11_digit_iin(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_nova_sim_with_mcc_mnc(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_msim_wild(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_msim_mcc_mnc_list(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_msim_capability_with_mcc_mnc(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_msim_capability_with_iccid(user_specified_attrib_t attr);
test_result ril_utf_test_mbn_msim_disable_modem_update_fail(user_specified_attrib_t attr);
test_result ril_utf_test_embms_enable_disable_001( user_specified_attrib_t attr );
test_result ril_utf_test_sglte_property( user_specified_attrib_t attr );
test_result ril_utf_test_gerrit_1270060_001( user_specified_attrib_t attr);
test_result ril_utf_test_get_activity_info_001( user_specified_attrib_t attr);
test_result ril_utf_test_set_lte_band_pref_001( user_specified_attrib_t attr);
test_result ril_utf_test_set_lte_band_pref_002( user_specified_attrib_t attr);
test_result ril_utf_test_set_lte_band_pref_003( user_specified_attrib_t attr);
test_result ril_utf_test_set_lte_band_pref_004( user_specified_attrib_t attr);
test_result ril_utf_test_send_ussd_001(user_specified_attrib_t attr);
test_result ril_utf_test_send_ussd_002(user_specified_attrib_t attr);
test_result ril_utf_test_send_ussd_003(user_specified_attrib_t attr);
test_result ril_utf_test_send_ussd_004(user_specified_attrib_t attr);
test_result ril_utf_test_send_ussd_005(user_specified_attrib_t attr);
test_result ril_utf_test_send_ussd_006(user_specified_attrib_t attr);
test_result ril_utf_test_send_ussd_007(user_specified_attrib_t attr);
test_result ril_utf_test_send_ussd_008(user_specified_attrib_t attr);
test_result ril_utf_test_nv_reset_config_001(user_specified_attrib_t attr);
test_result ril_utf_test_shutdown_001(user_specified_attrib_t attr);
test_result ril_utf_test_shutdown_002(user_specified_attrib_t attr);
test_result ril_utf_test_shutdown_003(user_specified_attrib_t attr);
test_result ril_utf_test_query_network_selection_mode_1(user_specified_attrib_t attr);
test_result ril_utf_test_query_network_selection_mode_2(user_specified_attrib_t attr);
test_result ril_utf_test_set_band_mode(user_specified_attrib_t attr);
test_result ril_utf_test_get_available_band_modes(user_specified_attrib_t attr);
test_result ril_utf_test_device_identity(user_specified_attrib_t attr);
test_result ril_utf_test_get_base_band_version(user_specified_attrib_t attr);
test_result ril_utf_test_emergency_list_unsol_1(user_specified_attrib_t attr);
test_result ril_utf_test_emergency_list_unsol_2(user_specified_attrib_t attr);

test_result ril_utf_test_dtmf_001(user_specified_attrib_t attr);
test_result ril_utf_test_dtmf_002(user_specified_attrib_t attr);
test_result ril_utf_test_dtmf_003(user_specified_attrib_t attr);
test_result ril_utf_test_dtmf_004(user_specified_attrib_t attr);
test_result ril_utf_test_dtmf_wo_flow_control_pair(user_specified_attrib_t attr);
test_result ril_utf_test_supp_srv_notify_001(user_specified_attrib_t attr);
test_result ril_utf_test_supp_srv_notify_002(user_specified_attrib_t attr);
test_result ril_utf_test_voice_privacy_001(user_specified_attrib_t attr);
test_result ril_utf_test_uus_info_ind_001(user_specified_attrib_t attr);

//UIM test cases
test_result ril_utf_test_send_status_command_testcase_1(user_specified_attrib_t attr);
test_result ril_utf_test_refresh_init_full_fcn_non_prov_1(user_specified_attrib_t attr);
test_result ril_utf_test_get_card_status_1(user_specified_attrib_t attr);
test_result ril_utf_test_open_logical_channel_request_1(user_specified_attrib_t attr);


//5g related
test_result ril_ut_test_enable_5g_inclusive_01(user_specified_attrib_t attr);

// enable modem
test_result ril_utf_test_enable_modem_on(user_specified_attrib_t attr);
test_result ril_utf_test_enable_modem_off(user_specified_attrib_t attr);
test_result ril_utf_test_enable_modem_timeout(user_specified_attrib_t attr);
test_result ril_ut_test_get_modem_stack_status_on(user_specified_attrib_t attr);
test_result ril_ut_test_get_modem_stack_status_off(user_specified_attrib_t attr);
// Network scan
test_result ril_ut_test_network_scan_001(user_specified_attrib_t attr);
test_result ril_ut_test_network_scan_002(user_specified_attrib_t attr);
test_result ril_ut_test_network_scan_003(user_specified_attrib_t attr);
test_result ril_ut_test_network_scan_004(user_specified_attrib_t attr);
test_result ril_ut_test_network_scan_005(user_specified_attrib_t attr);
test_result ril_ut_test_network_scan_006(user_specified_attrib_t attr);
test_result ril_ut_test_network_scan_007(user_specified_attrib_t attr);

test_result ril_ut_test_get_neighboring_cell_ids_001(user_specified_attrib_t attr);
test_result ril_ut_test_get_neighboring_cell_ids_002(user_specified_attrib_t attr);
test_result ril_ut_test_get_neighboring_cell_ids_003(user_specified_attrib_t attr);
test_result ril_ut_test_get_neighboring_cell_ids_004(user_specified_attrib_t attr);
test_result ril_ut_test_get_cell_info_list_001(user_specified_attrib_t attr);

test_result ril_utf_test_carrier_aggregation_ind_001(user_specified_attrib_t attr);
test_result ril_utf_test_consecutive_set_get_preferred_network_type_testcase_1( user_specified_attrib_t attr );
test_result ril_utf_test_consecutive_set_get_preferred_network_type_testcase_2( user_specified_attrib_t attr );
#endif //_RIL_UTF_TEST_QMI_RIL

