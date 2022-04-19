/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_test_qmi_ril_boot_cases.cpp
  @brief   RIL UTF test case definitions for all boot cases off target

  DESCRIPTION
  All test case definitions should be in this file
  ---------------------------------------------------------------------------
******************************************************************************/

#include "ril_utf_test_qmi_ril.h"
#include "ril_utf_msim_rild.h"
#include "ril_utf_i_ims_radio_sim.h"

#ifdef QMI_RIL_HAL_UTF
void ril_utf_bootup_helper(Testcase &t, int boot_config)
{
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  IMSTestnode IMS_node;
  OEMTestnode OEM_node;
  DBTestnode DB_node;
  uim_session_information_type_v01 session_info;

  if ((boot_config == DEFAULT_BOOT) || (boot_config == PDC_BOOT))
  {
  // Expect: RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED
  RIL_node.update_default_ril_unsol_resp_radio_state_changed();
  t.expect(RIL_node);
  }

  // Update qcril.db property table
  util_ril_utf_db_property_init(t, DB_node);

  // Expect: QMI_DMS_SET_EVENT_REPORT_REQ_V01
  QMI_node.update_default_qmi_dms_set_event_report_req_msg();
  QMI_node.set_qmi_dsm_set_event_report_req_report_oprt_mode_state(0x01);
  QMI_node.set_qmi_dsm_set_event_report_req_report_prl_init(0x01);
  t.expect(QMI_node);

  // Inject: QMI_DMS_SET_EVENT_REPORT_RESP_V01
  QMI_node.update_default_qmi_dms_set_event_report_resp_msg();
  QMI_node.set_qmi_dms_set_event_report_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_GET_DEVICE_CAP_REQ_V01
  QMI_node.update_default_qmi_dms_get_device_cap_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_DEVICE_CAP_RESP_V01
  QMI_node.update_default_qmi_dms_get_device_cap_resp_v01();
  if (boot_config == PDC_BOOT)
  {
    dms_multisim_capability_type_v01 multisim_capability;
    memset(&multisim_capability, 0, sizeof(multisim_capability));
    multisim_capability.max_subscriptions = 2;
    QMI_node.set_default_qmi_dms_get_device_cap_resp_v01_multisim_capability(multisim_capability);
  }
  QMI_node.set_default_qmi_dms_get_device_cap_resp_v01_device_capabilities();
  QMI_node.set_qmi_dms_get_device_cap_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  /*
  // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
  QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_SHUTTING_DOWN_V01);
  QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);
  */

  // Expect: QMI_DMS_QMI_DMS_TRIGGER_MODEM_ACTIVITY_INFO_CALCULATION_REQ_V01
  QMI_node.update_default_qmi_dms_trigger_modem_activity_info_calculation_req(TRUE, 1);
  t.expect(QMI_node);

  // Inject: QMI_DMS_QMI_DMS_TRIGGER_MODEM_ACTIVITY_INFO_CALCULATION_RESP_V01
  QMI_node.update_default_qmi_dms_trigger_modem_activity_info_calculation_resp();
  t.inject(QMI_node);

  // Expect: QMI_LTE_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_lte_indication_register_req_msg();
  QMI_node.set_qmi_lte_indication_register_req_indication_bitmask(
          QMI_LTE_DISC_NOTIFICATION_IND_MASK_V01 |
          QMI_LTE_DISC_BROADCAST_NOTIFICATION_IND_MASK_V01 |
          QMI_LTE_DISC_MATCH_NOTIFICATION_IND_MASK_V01 |
          QMI_LTE_DISC_PSK_EXPIRED_IND_MASK_V01);
  t.expect(QMI_node);

  // Inject: QMI_LTE_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_lte_indication_register_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_IMSP_SET_EVENT_REPORT_REQ_V01
  QMI_node.update_default_qmi_imsp_set_event_report_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_IMSP_SET_EVENT_REPORT_RESP_V01
  QMI_node.update_default_qmi_imsp_set_event_report_resp_v01();
  QMI_node.set_qmi_imsp_set_event_report_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_GET_DEVICE_CAP_REQ_V01
  QMI_node.update_default_qmi_dms_get_device_cap_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_DEVICE_CAP_RESP_V01
  QMI_node.update_default_qmi_dms_get_device_cap_resp_v01();
  QMI_node.set_default_qmi_dms_get_device_cap_resp_v01_device_capabilities();
  QMI_node.set_qmi_dms_get_device_cap_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_GET_DEVICE_MFR_REQ_V01
  QMI_node.update_default_qmi_dms_get_device_mfr_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_DEVICE_MFR_RESP_V01
  QMI_node.update_default_qmi_dms_get_device_mfr_resp_v01();
  QMI_node.set_default_qmi_dms_get_device_mfr_resp_v01_device_manufacturer();
  QMI_node.set_qmi_dms_get_device_mfr_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);
#if 0
  // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
  t.expect(QMI_node);

  // Inject:QMI_DMS_GET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
  QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_SHUTTING_DOWN_V01);
  QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);
#endif
  // Expect: RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED
  RIL_node.update_default_ril_unsol_resp_radio_state_changed();
  t.expect(RIL_node);

  //t.set_start();

  // Expect: QMI_IMS_SET_BIND_SUBSCRIPTION_REQ_V01
  QMI_node.update_default_qmi_imss_bind_subscription_req_msg(IMS_SUBSCRIPTION_TYPE_PRIMARY_V01);
  t.expect(QMI_node);

  // Inject: QMI_IMS_SET_BIND_SUBSCRIPTION_RESP_V01
  QMI_node.update_default_qmi_imss_bind_subscription_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_IMSA_SET_BIND_SUBSCRIPTION_REQ_V01
  QMI_node.update_default_qmi_imsa_bind_subscription_req_msg(IMSA_BIND_SUBSCRIPTION_TYPE_PRIMARY_V01);
  t.expect(QMI_node);

  // Inject: QMI_IMSA_SET_BIND_SUBSCRIPTION_RESP_V01
  QMI_node.update_default_qmi_imsa_bind_subscription_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_reg_voice_privacy_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_ext_brst_intl_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_speech_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_handover_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_conference_events(0x01);
  //QMI_node.set_qmi_voice_ind_req_tty_info_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_tty_info_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_cc_result_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_additional_call_info_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_audio_rat_change_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_vice_dialog_event(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_conf_participants_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_GET_SUPPORTED_MSGS_REQ_V02
  QMI_node.update_default_qmi_voice_get_supported_msgs_req_msg();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_GET_SUPPORTED_MSGS_RESP_V02
  QMI_node.update_default_qmi_voice_get_supported_msgs_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_INDICATION_REGISTER_REQ_MSG_V01
  QMI_node.update_default_qmi_dms_indication_register_req_v01();
  QMI_node.set_qmi_dms_ind_req_report_ims_capability(0x01);
  t.expect(QMI_node);

  // Inject: QMI_DMS_INDICATION_REGISTER_RESP_MSG_V01
  QMI_node.update_default_qmi_dms_indication_register_resp_v01();
  QMI_node.set_qmi_dms_indication_register_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);


  if(SSR_BOOT != boot_config)
  {
    // Expect: QMI_IMS_SETTINGS_GET_SUPPORTED_MSGS_REQ_V01
    QMI_node.update_default_qmi_ims_settings_get_supported_msgs_req();
    t.expect(QMI_node);

    // Inject: QMI_IMS_SETTINGS_GET_SUPPORTED_MSGS_RESP_V01
    QMI_node.update_default_qmi_ims_settings_get_supported_msgs_resp();
    t.inject(QMI_node);

    // Expect: QMI_IMS_SETTINGS_GET_SUPPORTED_FIELDS_REQ_V01
    QMI_node.update_default_qmi_ims_settings_get_supported_fields_req();
    t.expect(QMI_node);

    // Inject: QMI_IMS_SETTINGS_GET_SUPPORTED_FIELDS_RESP_V01
    QMI_node.update_default_qmi_ims_settings_get_supported_fields_resp();
    t.inject(QMI_node);
  }

#ifdef RIL_UTF_L_MASTER
  // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
  t.expect(QMI_node);

  // Inject:QMI_DMS_GET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
  QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_SHUTTING_DOWN_V01);
  QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);
#endif

  // Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_V01
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_emergency_mode(0);
  mode_pref_mask_type_v01 mode_pref1 = QMI_NAS_RAT_MODE_PREF_UMTS_V01;
  mode_pref1 |= QMI_NAS_RAT_MODE_PREF_GSM_V01;
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(mode_pref1);
  // Set band_pref
  nas_band_pref_mask_type_v01 band_pref;
  band_pref |= QMI_NAS_BAND_CLASS_0_A_SYSTEM_V01;
  band_pref |= QMI_NAS_BAND_CLASS_0_B_AB_GSM850_V01;
  band_pref |= QMI_NAS_BAND_CLASS_1_ALL_BLOCKS_V01;
  band_pref |= QMI_NAS_BAND_CLASS_2_PLACEHOLDER_V01;
  band_pref |= QMI_NAS_BAND_CLASS_3_A_SYSTEM_V01;
  band_pref |= QMI_NAS_BAND_CLASS_4_ALL_BLOCKS_V01;
  band_pref |= QMI_NAS_BAND_CLASS_5_ALL_BLOCKS_V01;
  band_pref |= QMI_NAS_GSM_DCS_1800_BAND_V01;
  band_pref |= QMI_NAS_E_GSM_900_BAND_V01;
  band_pref |= QMI_NAS_P_GSM_900_BAND_V01;
  band_pref |= QMI_NAS_BAND_CLASS_6_V01;
  band_pref |= QMI_NAS_BAND_CLASS_7_V01;
  band_pref |= QMI_NAS_BAND_CLASS_8_V01;
  band_pref |= QMI_NAS_BAND_CLASS_9_V01;
  band_pref |= QMI_NAS_BAND_CLASS_10_V01;
  band_pref |= QMI_NAS_BAND_CLASS_11_V01;
  band_pref |= QMI_NAS_GSM_BAND_450_V01;
  band_pref |= QMI_NAS_GSM_BAND_480_V01;
  band_pref |= QMI_NAS_GSM_BAND_750_V01;
  band_pref |= QMI_NAS_GSM_BAND_850_V01;
  band_pref |= QMI_NAS_GSM_BAND_RAILWAYS_900_BAND_V01;
  band_pref |= QMI_NAS_GSM_BAND_PCS_1900_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_EU_J_CH_IMT_2100_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_US_PCS_1900_BAND_V01;
  band_pref |= QMI_NAS_EU_CH_DCS_1800_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_US_1700_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_JAPAN_800_BAND_V01;
  band_pref |= QMI_NAS_BAND_CLASS_12_V01;
  band_pref |= QMI_NAS_BAND_CLASS_14_V01;
  band_pref |= QMI_NAS_RESERVED_V01;
  band_pref |= QMI_NAS_BAND_CLASS_15_V01;
  band_pref |= QMI_NAS_WCDMA_EU_2600_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_EU_J_900_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_J_1700_BAND_V01;
  band_pref |= QMI_NAS_BAND_CLASS_16_V01;
  band_pref |= QMI_NAS_BAND_CLASS_17_V01;
  band_pref |= QMI_NAS_BAND_CLASS_18_V01;
  band_pref |= QMI_NAS_BAND_CLASS_19_V01;
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_band_pref(band_pref);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_prl_pref(NAS_PRL_PREF_ANY_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_roam_pref(NAS_ROAMING_PREF_ANY_V01);
  // Set band_pref_ext
  uint64_t band_pref_ext;
  band_pref_ext |= E_UTRA_OPERATING_BAND_1_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_2_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_3_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_4_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_5_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_6_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_7_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_8_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_9_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_10_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_11_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_12_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_13_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_14_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_17_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_18_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_19_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_20_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_21_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_24_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_25_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_33_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_34_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_35_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_36_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_37_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_38_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_39_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_40_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_41_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_42_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_43_V01;
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_band_pref_ext(band_pref_ext);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_net_sel_pref(NAS_NET_SEL_PREF_AUTOMATIC_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_PS_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_gw_acq_order_pref(NAS_GW_ACQ_ORDER_PREF_AUTOMATIC_V01);
  // Set tdscdma_band_pref
  nas_tdscdma_band_pref_mask_type_v01 tdscdma_band_pref;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_A_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_B_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_C_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_D_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_E_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_F_V01;
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_tdscdma_band_pref(tdscdma_band_pref);
  QMI_node.set_default_qmi_nas_get_system_selection_preference_resp_acq_order();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_reg_restriction(NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_usage_setting(NAS_USAGE_VOICE_CENTRIC_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  //t.set_end(); //Taiyab


  // Start set of unordered expects
 // t.set_start(); //Taiyab

  // Expect: QMI_NAS_CONFIG_SIG_INFO2_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_config_sig_info2_req_msg_v01();
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_cdma_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_cdma_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_gsm_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_sinr_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rsrp_delta(60);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rsrq_delta(20);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_snr_delta(40);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rpt_rate_avg_period(NAS_LTE_SIG_RPT_RATE_3_SEC_V01, NAS_LTE_SIG_AVG_PRD_5_SEC_V01);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_ecio_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_rscp_delta(40);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_rssi_delta(5);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_sinr_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_wcdma_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_wcdma_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_nr5g_rsrp_delta(60);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_nr5g_snr_delta(40);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_nr5g_rpt_rate_avg_period(
        NR5G_COMMON_RSSI_REPORTING_RATE_3_SEC_V01, NR5G_COMMON_RSSI_AVERAGING_PERIOD_5_SEC_V01);
  t.expect(QMI_node);

  // Inject: QMI_NAS_CONFIG_SIG_INFO2_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_config_sig_info2_resp_msg_v01();
  QMI_node.set_qmi_nas_config_sig_info2_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_CONFIG_SIG_INFO2_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_config_sig_info2_req_msg_v01();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_NAS_CONFIG_SIG_INFO2_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_config_sig_info2_resp_msg_v01();
  QMI_node.set_qmi_nas_config_sig_info2_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_SET_EVENT_REPORT_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_set_event_report_req_msg_v01();
  QMI_node.set_qmi_nas_set_event_report_req_report_reg_reject(TRUE);
  t.expect(QMI_node);

  // Inject: QMI_NAS_SET_EVENT_REPORT_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_set_event_report_resp_msg_v01();
  QMI_node.set_qmi_nas_set_event_report_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_LIMIT_SYS_INFO_IND_REPORTING_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_limit_sys_info_ind_reporting_req_msg(0);
  t.expect(QMI_node);

  // Inject: QMI_NAS_LIMIT_SYS_INFO_IND_REPORTING_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_limit_sys_info_ind_reporting_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_indication_register_req_msg_v01();
  QMI_node.set_qmi_nas_indication_register_req_msg_reg_sys_sel_pref(1);
  QMI_node.set_qmi_nas_indication_register_req_msg_dual_standby_pref(1);
  QMI_node.set_qmi_nas_indication_register_req_msg_subscription_info(1);
  QMI_node.set_qmi_nas_indication_register_reg_msg_reg_rtre_cfg(1);
  QMI_node.set_qmi_nas_indication_register_req_msg_req_serving_system(0);
  QMI_node.set_qmi_nas_indication_register_req_msg_reg_network_time(0);
  QMI_node.set_qmi_nas_indication_register_reg_msg_reg_embms_status(0);
  QMI_node.set_qmi_nas_indication_register_req_msg_sig_info(0);
  QMI_node.set_qmi_nas_indication_register_reg_msg_err_rate(0);
  QMI_node.set_qmi_nas_indication_register_req_msg_reg_current_plmn_name(0);
  QMI_node.set_qmi_nas_indication_register_req_msg_reg_rf_band_info(0);
  t.expect(QMI_node);

  // Inject: QMI_NAS_INDICATION_REGISTER_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_indication_register_resp_msg_v01();
  QMI_node.set_qmi_nas_nas_indication_register_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_config_plmn_name_ind_reporting_req_msg_v01();
  QMI_node.set_qmi_nas_config_plmn_name_ind_reporting_req_msg_send_all_information(1);
  t.expect(QMI_node);

  // Inject: QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_config_plmn_name_ind_reporting_resp_msg_v01();
  QMI_node.set_qmi_nas_config_plmn_name_ind_reporting_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);


#if 0
  // Expect: QMI_NAS_GET_CENTRALIZED_EONS_SUPPORT_STATUS_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_centralized_eons_support_status_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_CENTRALIZED_EONS_SUPPORT_STATUS_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_centralized_eons_support_status_resp_msg_v01();
  QMI_node.set_qmi_nas_get_centralized_eons_support_status_resp_msg_centralized_eons_supported(1);
  QMI_node.set_qmi_nas_get_centralized_eons_support_status_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);
#endif

  // Expect: QMI_NAS_GET_LTE_SIB16_NETWORK_TIME_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_lte_sib16_network_time_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_LTE_SIB16_NETWORK_TIME_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_lte_sib16_network_time_resp_msg_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
  QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_SHUTTING_DOWN_V01);
  QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_PBM_GET_CONFIGURATION_REQ_V01
  QMI_node.update_default_qmi_pbm_get_configuration_req_msg();
  QMI_node.set_qmi_pbm_get_configuration_pbm_feature_status_mask(0x01);
  t.expect(QMI_node);

  // Inject: QMI_PBM_GET_CONFIGURATION_RESP_V01
  QMI_node.update_default_qmi_pbm_get_configuration_resp_msg();
  QMI_node.set_qmi_pbm_get_configuration_resp_centralized_ecc(FALSE, FALSE);
  QMI_node.set_qmi_pbm_get_configuration_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_PBM_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_pbm_indication_register_req_msg();
  QMI_node.set_qmi_pbm_indication_register_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01);
  t.expect(QMI_node);

  // Inject: QMI_PBM_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_pbm_indication_register_resp_msg();
  QMI_node.set_qmi_pbm_indication_register_resp_v01_reg_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01);
  QMI_node.set_qmi_pbm_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_PBM_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_pbm_indication_register_req_msg();
  QMI_node.set_qmi_pbm_indication_register_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01|PBM_REG_RECORD_UPDATE_EVENTS_V01|PBM_REG_PHONEBOOK_READY_EVENTS_V01);
  t.expect(QMI_node);

  // Inject: QMI_PBM_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_pbm_indication_register_resp_msg();
  QMI_node.set_qmi_pbm_indication_register_resp_v01_reg_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01|PBM_REG_RECORD_UPDATE_EVENTS_V01|PBM_REG_PHONEBOOK_READY_EVENTS_V01);
  QMI_node.set_qmi_pbm_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_HTTP_RESET_REQ_V01
  QMI_node.update_default_qmi_uim_http_reset_req();
  t.expect(QMI_node);

  // Inject: QMI_UIM_HTTP_RESET_RESP_V01
  QMI_node.update_default_qmi_uim_http_reset_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

#if 1
  // Expect: QMI_UIM_EVENT_REG_REQ_V01
  QMI_node.update_default_qmi_uim_event_reg_req(0x07);
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_EVENT_REG_RESP_V01
  QMI_node.update_default_qmi_uim_event_reg_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_GET_CARD_STATUS_REQ_V01
  QMI_node.update_default_qmi_uim_get_card_status_req();
  QMI_node.set_qmi_uim_get_card_status_req_extended_card_status(0x01);
  t.expect(QMI_node);

  // Inject: QMI_UIM_GET_CARD_STATUS_RESP_V01
  uim_get_card_status_resp_msg_v01* card_status = QMI_node.update_default_qmi_uim_get_card_status_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  card_status->sim_busy_status_valid = TRUE;
  card_status->sim_busy_status_len = 0x01;
  card_status->sim_busy_status[0] = 0x00;
  card_status->extended_hot_swap_valid = TRUE;
  card_status->extended_hot_swap_len = 0x01;
  card_status->extended_hot_swap[0] = UIM_HOT_SWAP_NOT_SUPPORTED_V01;
  card_status->extended_card_status_valid = TRUE;
  card_status->extended_card_status.index_gw_len = 0x01;
  card_status->extended_card_status.index_1x_len = 0x01;
  card_status->extended_card_status.index_1x[0] = 0x0002;
  card_status->extended_card_status.card_info_len = 0x01;
  card_status->extended_card_status.card_info[0].card_state = UIM_EXTENDED_CARD_STATE_PRESENT_V01;
  card_status->extended_card_status.card_info[0].upin.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].upin.pin_retries = 0x00;
  card_status->extended_card_status.card_info[0].upin.puk_retries = 0x00;
  card_status->extended_card_status.card_info[0].error_code = UIM_CARD_ERROR_CODE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info_len = 0x04;
  // App slot 0
  card_status->extended_card_status.card_info[0].app_info[0].app_type = UIM_APP_TYPE_USIM_V01;
  card_status->extended_card_status.card_info[0].app_info[0].app_state = UIM_APP_STATE_READY_V01;
  card_status->extended_card_status.card_info[0].app_info[0].perso_state = UIM_PERSO_STATE_READY_V01;
  card_status->extended_card_status.card_info[0].app_info[0].perso_feature = UIM_PERSO_FEATURE_STATUS_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[0].perso_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[0].perso_unblock_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[0].aid_value_len = 0x10;
  uint8_t app_value_1[30] = {0xA0,0x00,0x00,0x00,0x87,0x10,0x02,0xF3,0x10,0xFF,0xFF,0x89,0x08,0x00,0x00,0xFF};
  memcpy(card_status->extended_card_status.card_info[0].app_info[0].aid_value, app_value_1, 16);
  card_status->extended_card_status.card_info[0].app_info[0].univ_pin = UIM_UNIV_PIN_PIN1_USED_V01;
  card_status->extended_card_status.card_info[0].app_info[0].pin1.pin_state = UIM_PIN_STATE_DISABLED_V01;
  card_status->extended_card_status.card_info[0].app_info[0].pin1.pin_retries = 0x03;
  card_status->extended_card_status.card_info[0].app_info[0].pin1.puk_retries = 0x0A;
  card_status->extended_card_status.card_info[0].app_info[0].pin2.pin_state = UIM_PIN_STATE_ENABLED_NOT_VERIFIED_V01;
  card_status->extended_card_status.card_info[0].app_info[0].pin2.pin_retries = 0x03;
  card_status->extended_card_status.card_info[0].app_info[0].pin2.puk_retries = 0x0A;
  // App slot 1
  card_status->extended_card_status.card_info[0].app_info[1].app_type = UIM_APP_TYPE_ISIM_V01;
  card_status->extended_card_status.card_info[0].app_info[1].app_state = UIM_APP_STATE_DETECTED_V01;
  card_status->extended_card_status.card_info[0].app_info[1].perso_state = UIM_PERSO_STATE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[1].perso_feature = UIM_PERSO_FEATURE_STATUS_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[1].perso_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[1].perso_unblock_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[1].aid_value_len = 0x10;
  uint8_t app_value_2[30] = {0xA0,0x00,0x00,0x00,0x87,0x10,0x04,0xF3,0x10,0xFF,0xFF,0x89,0x08,0x00,0x00,0xFF};
  memcpy(card_status->extended_card_status.card_info[0].app_info[1].aid_value, app_value_2, 16);
  card_status->extended_card_status.card_info[0].app_info[1].univ_pin = UIM_UNIV_PIN_PIN1_USED_V01;
  card_status->extended_card_status.card_info[0].app_info[1].pin1.pin_state = UIM_PIN_STATE_DISABLED_V01;
  card_status->extended_card_status.card_info[0].app_info[1].pin1.pin_retries = 0x03;
  card_status->extended_card_status.card_info[0].app_info[1].pin1.puk_retries = 0x0A;
  card_status->extended_card_status.card_info[0].app_info[1].pin2.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[1].pin2.pin_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[1].pin2.puk_retries = 0x00;
  // App slot 2
  card_status->extended_card_status.card_info[0].app_info[2].app_type = UIM_APP_TYPE_CSIM_V01;
  card_status->extended_card_status.card_info[0].app_info[2].app_state = UIM_APP_STATE_READY_V01;
  card_status->extended_card_status.card_info[0].app_info[2].perso_state = UIM_PERSO_STATE_READY_V01;
  card_status->extended_card_status.card_info[0].app_info[2].perso_feature = UIM_PERSO_FEATURE_STATUS_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[2].perso_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[2].perso_unblock_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[2].aid_value_len = 0x10;
  uint8_t app_value_3[30] = {0xA0,0x00,0x00,0x03,0x43,0x10,0x02,0xF3,0x10,0xFF,0xFF,0x89,0x02,0x00,0x00,0xFF};
  memcpy(card_status->extended_card_status.card_info[0].app_info[2].aid_value, app_value_3, 16);
  card_status->extended_card_status.card_info[0].app_info[2].univ_pin = UIM_UNIV_PIN_PIN1_USED_V01;
  card_status->extended_card_status.card_info[0].app_info[2].pin1.pin_state = UIM_PIN_STATE_DISABLED_V01;
  card_status->extended_card_status.card_info[0].app_info[2].pin1.pin_retries = 0x03;
  card_status->extended_card_status.card_info[0].app_info[2].pin1.puk_retries = 0x0A;
  card_status->extended_card_status.card_info[0].app_info[2].pin2.pin_state = UIM_PIN_STATE_ENABLED_NOT_VERIFIED_V01;
  card_status->extended_card_status.card_info[0].app_info[2].pin2.pin_retries = 0x03;
  card_status->extended_card_status.card_info[0].app_info[2].pin2.puk_retries = 0x0A;
  // App slot 3
  card_status->extended_card_status.card_info[0].app_info[3].app_type = UIM_APP_TYPE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[3].app_state = UIM_APP_STATE_DETECTED_V01;
  card_status->extended_card_status.card_info[0].app_info[3].perso_state = UIM_PERSO_STATE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[3].perso_feature = UIM_PERSO_FEATURE_STATUS_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[3].perso_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[3].perso_unblock_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[3].aid_value_len = 0x0c;
  uint8_t app_value_4[30] = {0xA0,0x00,0x00,0x00,0x63,0x50,0x4B,0x43,0x53,0x2D,0x31,0x35};
  memcpy(card_status->extended_card_status.card_info[0].app_info[3].aid_value, app_value_4, 12);
  card_status->extended_card_status.card_info[0].app_info[3].univ_pin = UIM_UNIV_PIN_PIN1_USED_V01;
  card_status->extended_card_status.card_info[0].app_info[3].pin1.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[3].pin1.pin_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[3].pin1.puk_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[3].pin2.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[3].pin2.pin_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[3].pin2.puk_retries = 0x00;
  t.inject(QMI_node);

  // Expect: RIL_UNSOL_OEM_HOOK_RAW
  //RIL_node.update_default_ril_unsol_oem_hook_raw();
  //RIL_node.set_expect_skip_verification_status(false);
  //t.expect(RIL_node);

  // Expect: QMI_UIM_GET_CONFIGURATION_REQ_V01
  QMI_node.update_default_qmi_uim_get_configuration_req();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_GET_CONFIGURATION_RESP_V01
  QMI_node.update_default_qmi_uim_get_configuration_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_CAT_GET_CONFIGURATION_REQ_V02
  QMI_node.update_default_qmi_cat_get_configuration_req();
  t.expect(QMI_node);

  // Inject: QMI_CAT_GET_CONFIGURATION_RESP_V02
  QMI_node.update_default_qmi_cat_get_configuration_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_expect_skip_verification_status(false);
  t.inject(QMI_node);

  // Expect: QMI_CAT_SET_EVENT_REPORT_REQ_V02
  QMI_node.update_default_qmi_cat_set_event_report_req();
#ifdef RIL_UTF_L_MASTER
  QMI_node.set_qmi_cat_set_event_report_req_pc_evt_report_req_mask(0x0b7FFFFF);
#else
  QMI_node.set_qmi_cat_set_event_report_req_pc_evt_report_req_mask(0x037FFFFF);
#endif
  QMI_node.set_qmi_cat_set_event_report_req_pc_dec_evt_report_req_mask(0x00800000);
  QMI_node.set_qmi_cat_set_event_report_req_slot_mask(CAT_SET_EVENT_REPORT_SLOT_1_V02);
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_CAT_SET_EVENT_REPORT_RESP_V02
  QMI_node.update_default_qmi_cat_set_event_report_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_GET_SLOTS_STATUS_REQ_V01
  QMI_node.update_default_qmi_uim_get_slots_status_req();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_GET_SLOTS_STATUS_RESP_V01
  QMI_node.update_default_qmi_uim_get_slots_status_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_GET_ATR_REQ_V01
  QMI_node.update_default_qmi_uim_get_atr_req();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_GET_ATR_RESP_V01
  uim_get_atr_resp_msg_v01* atr_resp_ptr =
    QMI_node.update_default_qmi_uim_get_atr_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);

  atr_resp_ptr->atr_value_valid          = true;
  atr_resp_ptr->atr_value_len            = QMI_UIM_ATR_MAX_V01;

  //dummy ATR
  uint8_t atr[QMI_UIM_ATR_MAX_V01] = {59, 159, 151, 128, 63, 199, 130, 128,49, 224, 115, 254,
                                      33, 31, 100, 9, 6, 1, 0, 130, 144, 0, 104, 0, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0};

  memcpy(atr_resp_ptr->atr_value, atr, QMI_UIM_ATR_MAX_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_READ_TRANSPARENT_REQ_V01
  QMI_node.update_default_qmi_uim_read_transparent_reg();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_READ_TRANSPARENT_RESP_V01
  uim_read_transparent_resp_msg_v01* read_transparent_resp_ptr =
    QMI_node.update_default_qmi_uim_read_transparent_resp(QMI_RESULT_SUCCESS_V01,QMI_ERR_NONE_V01);

  read_transparent_resp_ptr->card_result_valid = true;
  read_transparent_resp_ptr->card_result.sw1   = 0x90;
  read_transparent_resp_ptr->card_result.sw2   = 0x00;
  read_transparent_resp_ptr->read_result_valid = true;
  read_transparent_resp_ptr->read_result.content_len = QMI_UIM_ICCID_LEN_MAX_V01;

  //dummy ICCID
  uint8_t iccid[QMI_UIM_ICCID_LEN_MAX_V01] = {0x98, 0x10,0x99,0x09, 0x10, 0x08,
                                              0x00, 0x00, 0x28, 0xF2};

  memcpy(read_transparent_resp_ptr->read_result.content, iccid, QMI_UIM_ICCID_LEN_MAX_V01);
  t.inject(QMI_node);

#if 0
  // Expect: QMI_CAT_SET_EVENT_REPORT_REQ_V02
  QMI_node.update_default_qmi_cat_set_event_report_req();
  QMI_node.set_qmi_cat_set_event_report_req_pc_dec_evt_report_req_mask(0x00800000);
  QMI_node.set_qmi_cat_set_event_report_req_slot_mask(CAT_SET_EVENT_REPORT_SLOT_1_V02);
  t.expect(QMI_node);

  // Inject: QMI_CAT_SET_EVENT_REPORT_RESP_V02
  QMI_node.update_default_qmi_cat_set_event_report_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);
#endif
#endif

  // Expect: RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED
  RIL_node.update_default_ril_unsol_response_sim_status_changed();
  t.expect(RIL_node);

  // Expect: QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ_V01
  QMI_node.update_default_qmi_wms_get_transport_layer_info_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_WMS_GET_TRANSPORT_LAYER_INFO_RESP_V01
  QMI_node.update_default_qmi_wms_get_transport_layer_info_resp_v01();
  QMI_node.set_qmi_wms_get_transport_layer_info_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  if ((boot_config == DEFAULT_BOOT) || (boot_config == PDC_BOOT))
  {
#if 0
      // Expect: QMI_IMS_SETTINGS_GET_REG_MGR_CONFIG_REQ_V01
      QMI_node.update_default_qmi_ims_settings_get_reg_mgr_config_req_v01();
      t.expect(QMI_node);

      // Inject: QMI_IMS_SETTINGS_GET_REG_MGR_CONFIG_RSP_V01
      QMI_node.update_default_qmi_ims_settings_get_reg_mgr_config_rsp_v01();
      QMI_node.set_qmi_ims_settings_get_reg_mgr_config_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
      t.inject(QMI_node);
#endif
#if 0
      // Expect: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_REQ_V01
      QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_req_v01();
      t.expect(QMI_node);

      // Inject: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_RSP_V01
      QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_rsp_v01();
      QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
      t.inject(QMI_node);
#endif
      // Expect: QMI_IMS_SETTINGS_GET_IMS_SERVICE_ENABLE_CONFIG_REQ_V01
      QMI_node.update_default_qmi_ims_settings_get_ims_service_enable_config_req();
      t.expect(QMI_node);

      // Inject: QMI_IMS_SETTINGS_GET_IMS_SERVICE_ENABLE_CONFIG_RSP_V01
      QMI_node.update_default_qmi_ims_settings_get_ims_service_enable_config_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
      t.inject(QMI_node);

      // Expect: QMI_IMSA_IND_REG_REQ_V01
      QMI_node.update_default_qmi_imsa_ind_reg_req_v01();
      t.expect(QMI_node);

      // Inject: QMI_IMSA_IND_REG_RSP_V01
      QMI_node.update_default_qmi_imsa_ind_reg_rsp_v01();
      QMI_node.set_qmi_imsa_ind_req_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
      t.inject(QMI_node);

      // Expect: QMI_IMS_SETTINGS_CONFIG_IND_REG_REQ_V01
      QMI_node.update_default_qmi_ims_config_ind_reg_req_v01();
      t.expect(QMI_node);

      // Inject: QMI_IMS_SETTINGS_CONFIG_IND_REG_RSP_V01
      QMI_node.update_default_qmi_ims_config_ind_reg_rsp_v01();
      QMI_node.set_qmi_ims_config_ind_req_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
      t.inject(QMI_node);
  }

  //QMI_NAS_GET_SUBSCRIPTION_INFO_REQ_MSG_V01
  util_ril_utf_get_subscription_info(t, QMI_node, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, TRUE, NAS_PRIORITY_SUBSCRIPTION_TRUE_V01, TRUE, NAS_SUBSCRIPTION_ACTIVE_V01, TRUE, 0, TRUE, 0, FALSE, 0, FALSE, 0);

  if (boot_config == SSR_BOOT)
  {
      // Expect: QMI_EMBMS_UNSOL_EMBMS_STATUS_IND
      OEM_node.update_default_qmi_embms_unsol_embms_status_ind();
      OEM_node.set_qmi_embms_unsol_embms_status(0);
      t.expect(OEM_node);
  }

  // Expect: QMI_DMS_GET_CURRENT_PRL_INFO_REQ_V01
  QMI_node.update_default_qmi_dms_get_current_prl_info_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_CURRENT_PRL_INFO_RESP_V01
  QMI_node.update_default_qmi_dms_get_current_prl_info_resp_v01();
  QMI_node.set_qmi_dms_get_current_prl_info_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_READ_TRANSPARENT_REQ_V01
  QMI_node.update_default_qmi_uim_read_transparent_reg();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_READ_TRANSPARENT_RESP_V01
  QMI_node.update_default_qmi_uim_read_transparent_resp(QMI_RESULT_SUCCESS_V01,QMI_ERR_NONE_V01);
  t.inject(QMI_node);
#if 0
  // Expect: RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED
  RIL_node.update_default_ril_unsol_resp_radio_state_changed();
  t.expect(RIL_node);
#endif

//  // Expect: ims_MsgId_UNSOL_RADIO_STATE_CHANGED
//  IMS_node.update_default_ims_unsol_radio_state_changed(ims_RadioState_RADIO_STATE_OFF);
//  t.expect(IMS_node);

  if (boot_config == SSR_BOOT)
  {
//  // Expect: ims_MsgId_UNSOL_RADIO_STATE_CHANGED
//  IMS_node.update_default_ims_unsol_radio_state_changed(ims_RadioState_RADIO_STATE_OFF);
//  t.expect(IMS_node);

  // Expect: LteDirectDiscovery_MsgId_UNSOL_RESPONSE_SERVICE_STATUS
  OEM_node.update_default_lte_direct_discovery_unsol_response_service_status();
  OEM_node.set_lte_direct_discovery_unsol_response_service_status_publish_allowed(0);
  OEM_node.set_lte_direct_discovery_unsol_response_service_status_subscribe_allowed(0);
  t.expect(OEM_node);
  }

  // Expect: QMI_WMS_SET_PRIMARY_CLIENT_REQ_V01
  QMI_node.update_default_qmi_wms_set_primary_client_req_v01();
  QMI_node.set_qmi_wms_set_primary_client_req_v01_primary_client(1);
  t.expect(QMI_node);

  // Inject: QMI_WMS_SET_PRIMARY_CLIENT_RESP_V01
  QMI_node.update_default_qmi_wms_set_primary_client_resp_v01();
  QMI_node.set_qmi_wms_set_primary_client_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_SET_ROUTES_REQ_V01
  QMI_node.update_default_qmi_wms_set_routes_req_v01();
  // TODO:  Add sane values here and enable payload validation
  t.expect(QMI_node);

  // Inject: QMI_WMS_SET_ROUTES_RESP_V01
  QMI_node.update_default_qmi_wms_set_routes_resp_v01();
  QMI_node.set_qmi_wms_set_routes_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_SET_EVENT_REPORT_REQ_V01
  QMI_node.update_default_qmi_wms_set_event_report_req_v01();
  // TODO:  Add sane values here and enable payload validation
  t.expect(QMI_node);

  // Inject: QMI_WMS_SET_EVENT_REPORT_RESP_V01
  QMI_node.update_default_qmi_wms_set_event_report_resp_v01();
  QMI_node.set_qmi_wms_set_event_report_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_READ_TRANSPARENT_REQ_V01
  //QMI_node.update_default_qmi_uim_read_transparent_reg();
  //QMI_node.set_expect_skip_verification_status(false);
  //t.expect(QMI_node);

  // Inject: QMI_UIM_READ_TRANSPARENT_RESP_V01
  //QMI_node.update_default_qmi_uim_read_transparent_resp(QMI_RESULT_SUCCESS_V01,QMI_ERR_NONE_V01);
  //t.inject(QMI_node);

#ifdef RIL_UTF_L_MASTER
  if (boot_config == PDC_BOOT)
  {
    /*
    // Expect: QMI_UIM_READ_TRANSPARENT_REQ_V01
    uim_read_transparent_req_msg_v01* rt_req_ptr = QMI_node.update_default_qmi_uim_read_transparent_reg();
    rt_req_ptr->session_information.session_type = UIM_SESSION_TYPE_CARD_ON_SLOT_1_V01;
    rt_req_ptr->file_id.file_id = 0x2FE2;
    rt_req_ptr->file_id.path_len = 0x02;
    rt_req_ptr->file_id.path[0] = 0x00;
    rt_req_ptr->file_id.path[1] = 0x3F;
    t.expect(QMI_node);

    // Inject: QMI_UIM_READ_TRANSPARENT_RESP_V01
    uim_read_transparent_resp_msg_v01* rt_resp_ptr = QMI_node.update_default_qmi_uim_read_transparent_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    rt_resp_ptr->read_result_valid = 1;
    rt_resp_ptr->read_result.content_len = 10;
    memcpy(rt_resp_ptr->read_result.content, tmp_uim_id, 10);
    t.inject(QMI_node);

    // Expect: QMI_DMS_UIM_GET_ICCID_REQ_V01
    QMI_node.update_default_qmi_dms_get_iccid_req();
    t.expect(QMI_node);

    // Inject: QMI_DMS_UIM_GET_ICCID_RESP_V01
    uim_id = QMI_node.update_default_qmi_dms_get_iccid_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    memcpy(uim_id, tmp_uim_id, 10);
    t.inject(QMI_node);
    */
  }
  else
  {
    /*
    // Expect: QMI_UIM_READ_TRANSPARENT_REQ_V01
    QMI_node.update_default_qmi_uim_read_transparent_reg();
    QMI_node.set_expect_skip_verification_status(false);
    t.expect(QMI_node);

    // Inject: QMI_UIM_READ_TRANSPARENT_RESP_V01
    QMI_node.update_default_qmi_uim_read_transparent_resp(QMI_RESULT_FAILURE_V01, QMI_ERR_GENERAL_V01);
    t.inject(QMI_node);

    // Expect: QMI_UIM_READ_TRANSPARENT_REQ_V01
    QMI_node.update_default_qmi_uim_read_transparent_reg();
    QMI_node.set_expect_skip_verification_status(false);
    t.expect(QMI_node);

    // Inject: QMI_UIM_READ_TRANSPARENT_RESP_V01
    QMI_node.update_default_qmi_uim_read_transparent_resp(QMI_RESULT_FAILURE_V01, QMI_ERR_GENERAL_V01);
    t.inject(QMI_node);

    // Expect: QMI_DMS_UIM_GET_ICCID_REQ_V01
    QMI_node.update_default_qmi_dms_get_iccid_req();
    t.expect(QMI_node);

    // Inject: QMI_DMS_UIM_GET_ICCID_RESP_V01
    uim_id = QMI_node.update_default_qmi_dms_get_iccid_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    memcpy(uim_id, tmp_uim_id, 10);
    t.inject(QMI_node);

    // Expect: QMI_DMS_UIM_GET_ICCID_REQ_V01
    QMI_node.update_default_qmi_dms_get_iccid_req();
    t.expect(QMI_node);

    // Inject: QMI_DMS_UIM_GET_ICCID_RESP_V01
    uim_id = QMI_node.update_default_qmi_dms_get_iccid_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    memcpy(uim_id, tmp_uim_id, 10);
    t.inject(QMI_node);
    */
  }
#endif

  // Expect: QMI_WMS_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_wms_indication_register_req_v01();
  QMI_node.set_qmi_wms_indication_register_req_v01_reg_transport_layer_info_events(1);
  QMI_node.set_qmi_wms_indication_register_req_v01_reg_transport_layer_mwi_info_events(1);
  t.expect(QMI_node);

  // Inject: QMI_WMS_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_wms_indication_register_resp_v01();
  QMI_node.set_qmi_wms_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_GET_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_wms_get_indication_register_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_WMS_GET_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_wms_get_indication_register_resp_v01();
  QMI_node.set_qmi_wms_get_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  //TODO: Fill with sane values to send to qmi-ril
  t.inject(QMI_node);

#if 0
  // Expect: QMI_UIM_GET_SERVICE_STATUS_REQ_V01
  uim_session_information_type_v01 session_info;
  memset(&session_info, 0, sizeof(session_info));
  session_info.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
  session_info.aid_len = 0;
  QMI_node.update_default_qmi_uim_get_service_status_req(session_info, 0x01);
  t.expect(QMI_node);

  // Inject: QMI_UIM_GET_SERVICE_STATUS_RESP_V01
  QMI_node.update_default_qmi_uim_get_service_status_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_uim_get_service_status_resp_fdn_status(UIM_FDN_STATUS_AVAILABLE_DISABLED_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_GET_SERVICE_STATUS_REQ_V01
  memset(&session_info, 0, sizeof(session_info));
  session_info.session_type = UIM_SESSION_TYPE_PRIMARY_1X_V01;
  session_info.aid_len = 0;
  QMI_node.update_default_qmi_uim_get_service_status_req(session_info, 0x01);
  t.expect(QMI_node);

  // Inject: QMI_UIM_GET_SERVICE_STATUS_RESP_V01
  QMI_node.update_default_qmi_uim_get_service_status_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_uim_get_service_status_resp_fdn_status(UIM_FDN_STATUS_AVAILABLE_DISABLED_V01);
  t.inject(QMI_node);

  if (boot_config == SSR_BOOT)
  {
  // Expect: QMI_UIM_READ_TRANSPARENT_REQ_V01
  QMI_node.update_default_qmi_uim_read_transparent_reg();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_READ_TRANSPARENT_RESP_V01
  QMI_node.update_default_qmi_uim_read_transparent_resp(QMI_RESULT_FAILURE_V01, QMI_ERR_GENERAL_V01);
  t.inject(QMI_node);
  }
#endif

  // Expect: QMI_DMS_SET_AP_VERSION_REQ_V01
  QMI_node.update_default_qmi_dms_set_ap_version_req_v01();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_DMS_SET_AP_VERSION_RESP_V01
  QMI_node.update_default_qmi_dms_set_ap_version_resp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

#if 1
  // Expect: QMI_UIM_GET_FILE_ATTRIBUTES_REQ_V01
  memset(&session_info, 0, sizeof(session_info));
  session_info.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
  session_info.aid_len = 0x00;
  uim_file_id_type_v01 file_id;
  memset(&file_id, 0, sizeof(file_id));
  uint8_t path[10] = {0x00,0x3F,0x10,0x7F,0x3A,0x5F};
  file_id.file_id = 0x4f30;
  file_id.path_len = 0x06;
  memcpy(file_id.path, path, 0x06);
  QMI_node.update_default_qmi_uim_get_file_attributes_req(session_info, file_id);
  t.expect(QMI_node);

  // Inject: QMI_UIM_GET_FILE_ATTRIBUTES_RESP_V01
  QMI_node.update_default_qmi_uim_get_file_attributes_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  uim_card_result_type_v01 card_result;
  card_result.sw1 = 0x90;
  card_result.sw2 = 0x00;
  QMI_node.set_qmi_uim_get_file_attributes_resp_card_result(card_result);
  QMI_node.set_qmi_uim_get_file_attribues_resp_file_status(UIM_FILE_STATUS_ACTIVATED_V01);
  uim_file_attributes_type_v01* file_attributes = QMI_node.set_qmi_uim_get_file_attributes_resp_file_attributes();
  file_attributes->file_size = 0x4c;
  file_attributes->file_id = 0x4f30;
  file_attributes->file_type = UIM_FILE_TYPE_LINEAR_FIXED_V01;
  file_attributes->rec_size = 0x4c;
  file_attributes->rec_count = 0x01;
  file_attributes->sec_read = UIM_SEC_CONDITION_SINGLE_COND_V01;
  file_attributes->sec_read_mask = 0x01;
  file_attributes->sec_write = UIM_SEC_CONDITION_SINGLE_COND_V01;
  file_attributes->sec_write_mask = 0x08;
  file_attributes->sec_increase = UIM_SEC_CONDITION_ALWAYS_V01;
  file_attributes->sec_increase_mask = 0x00;
  file_attributes->sec_deactivate = UIM_SEC_CONDITION_SINGLE_COND_V01;
  file_attributes->sec_deactivate_mask = 0x08;
  file_attributes->sec_activate = UIM_SEC_CONDITION_SINGLE_COND_V01;
  file_attributes->sec_activate_mask = 0x08;
  file_attributes->raw_value_len = 0x24;
  uint8_t temp_raw_value[40] = {0x62,0x22,0x82,0x05,0x42,0x21,0x00,0x4C,0x01,0x83,0x02,0x4F,0x30,0xA5,0x03,
    0xC0,0x01,0x00,0x8A,0x01,0x05,0x8B,0x03,0x6F,0x06,0x03,0x80,0x02,0x00,0x4C,0x81,0x02,0x00,0x60,0x88,0x00};
  memcpy(file_attributes->raw_value, temp_raw_value, 36);
  t.inject(QMI_node);

  // Expect: QMI_UIM_READ_RECORD_REQ_V01
  uim_read_record_req_msg_v01* read_req = QMI_node.update_default_qmi_uim_read_record_reg();
  read_req->session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
  read_req->session_information.aid_len = 0x00;
  read_req->file_id.file_id = 0x4f30;
  read_req->file_id.path_len = 0x06;
  uint8_t tmp_path[] = {0x00,0x3F,0x10,0x7F,0x3A,0x5F};
  memcpy(read_req->file_id.path, tmp_path, 0x06);
  read_req->read_record.record = 0x01;
  read_req->read_record.length = 0x4c;
  t.expect(QMI_node);

  // Inject: QMI_UIM_READ_RECORD_RESP_V01
  uim_read_record_resp_msg_v01* read_resp = QMI_node.update_default_qmi_uim_read_record_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  read_resp->read_result_valid = TRUE;
  read_resp->read_result.content_len = 0x4e;
  uint8_t tmp_content[] = {
    0x4C, 0x00, 0xA8, 0x23, 0xC0, 0x03, 0x4F, 0x3A, 0x01, 0xC5, 0x03, 0x4F, 0x09, 0x02, 0xC4, 0x03, 0x4F,
    0x11, 0x04, 0xC4, 0x03, 0x4F, 0x13, 0x05, 0xC4, 0x03, 0x4F, 0x15, 0x06, 0xCA, 0x03, 0x4F, 0x50, 0x09,
    0xC9, 0x03, 0x4F, 0x21, 0x0C, 0xAA, 0x0F, 0xC2, 0x03, 0x4F, 0x4A, 0x08, 0xC7, 0x03, 0x4F, 0x4B, 0x14,
    0xCB, 0x03, 0x4F, 0x3D, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
  };
  memcpy(read_resp->read_result.content, tmp_content, 0x4e);
  read_resp->card_result_valid = TRUE;
  read_resp->card_result.sw1 = 0x90;
  read_resp->card_result.sw2 = 0x00;
  t.inject(QMI_node);

  // Expect: QMI_UIM_REFRESH_REGISTER_REQ_V01
  uim_register_refresh_type_v01 reg_ref;
  QMI_node.update_default_qmi_uim_get_file_attributes_req(session_info, reg_ref);
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_REFRESH_REGISTER_RESP_V01
  QMI_node.update_default_qmi_uim_refresh_register_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_REFRESH_REGISTER_REQ_V01
  QMI_node.update_default_qmi_uim_get_file_attributes_req(session_info, reg_ref);
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_REFRESH_REGISTER_RESP_V01
  QMI_node.update_default_qmi_uim_refresh_register_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_REFRESH_REGISTER_REQ_V01
  QMI_node.update_default_qmi_uim_get_file_attributes_req(session_info, reg_ref);
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_REFRESH_REGISTER_RESP_V01
  QMI_node.update_default_qmi_uim_refresh_register_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED
  //RIL_node.update_default_ril_unsol_response_sim_status_changed();
  //t.expect(RIL_node);
#endif

  /*
  // Inject: RIL_REQUEST_GET_SIM_STATUS
  RIL_node.update_default_ril_request_get_sim_status();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_GET_SIM_STATUS (resp)
  RIL_CardStatus_v6* status = RIL_node.update_default_ril_request_get_sim_status_resp();
  status->card_state = RIL_CARDSTATE_PRESENT;
  t.expect(RIL_node);
  */

#if 1
  // Expect: QMI_DMS_GET_CURRENT_PRL_INFO_REQ_V01
  QMI_node.update_default_qmi_dms_get_current_prl_info_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_CURRENT_PRL_INFO_RESP_V01
  QMI_node.update_default_qmi_dms_get_current_prl_info_resp_v01();
  QMI_node.set_qmi_dms_get_current_prl_info_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);
#endif

  //t.set_end(); Taiyab

  if (boot_config == PDC_BOOT)
  {
    util_ril_utf_init_hw_mbn(t, 1);

    util_ril_utf_qmi_pdc_disable_modem_update(t);

    util_ril_utf_init_sw_mbn(t, 0);

    util_ril_utf_qmi_pdc_get_selected_config(t, NULL, (const uint8_t*)"pending_config_id_0", PDC_CONFIG_TYPE_MODEM_SW_V01, 0);

    util_ril_utf_qmi_pdc_activate_config(t, PDC_CONFIG_TYPE_MODEM_SW_V01, 0, 1);

    util_ril_utf_qmi_pdc_cleanup_config(t, PDC_CONFIG_TYPE_MODEM_SW_V01, 0);
  }

  if ((boot_config == DEFAULT_BOOT) || (boot_config == PDC_BOOT))
  {
  // Start the ims threads for ims
//  t.add_func_exec(utf_start_ims_threads);

//  // Expect: ims_MsgId_UNSOL_RADIO_STATE_CHANGED
//  IMS_node.update_default_ims_unsol_radio_state_changed(ims_RadioState_RADIO_STATE_OFF);
//  t.expect(IMS_node);
  }

  // Inject: RIL_REQUEST_RADIO_POWER
  RIL_node.update_default_ril_request_radio_power();
  t.inject(RIL_node);

 // t.set_start(); Taiyab

  /*
  if (boot_config != SSR_BOOT)
  {
    // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
    QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
    t.expect(QMI_node);

    // Inject: QMI_DMS_GET_OPERATING_MODE_RESP_V01
    QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
    QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_LOW_POWER_V01);
    QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);
  }
  */

  // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
  QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_LOW_POWER_V01);
  QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_SET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_set_operating_mod_req_v01(DMS_OP_MODE_ONLINE_V01);
  t.expect(QMI_node);

  // Inject: QMI_DMS_SET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_set_operating_mode_resp_v01();
  QMI_node.set_qmi_dms_set_operating_mode_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject: QMI_DMS_EVENT_REPORT_IND_V01
  QMI_node.update_default_qmi_dms_event_report_ind_v01();
  QMI_node.set_qmi_dms_event_report_ind_v01_operating_mode(DMS_OP_MODE_ONLINE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_SUB_BLOCKED_STATUS_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_sub_blocked_status_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_SUB_BLOCKED_STATUS_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_sub_blocked_status_resp_msg_v01();
  QMI_node.set_qmi_nas_get_sub_blocked_status_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_nas_get_sub_blocked_status_resp_msg_is_blocked(1,0);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
  QMI_node.set_qmi_nas_get_sys_info_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);

  // Expect: RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_voice_network_state_changed();
  t.expect(RIL_node);

  // Expect: RIL_UNSOL_SIGNAL_STRENGTH
  RIL_node.update_default_ril_unsol_signal_strength();
  t.expect(RIL_node);

  // Expect: QMI_NAS_CONFIG_SIG_INFO2_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_config_sig_info2_req_msg_v01();
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_cdma_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_cdma_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_gsm_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_sinr_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rsrp_delta(60);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rsrq_delta(20);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_snr_delta(40);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rpt_rate_avg_period(NAS_LTE_SIG_RPT_RATE_3_SEC_V01, NAS_LTE_SIG_AVG_PRD_5_SEC_V01);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_ecio_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_rscp_delta(40);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_rssi_delta(5);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_sinr_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_wcdma_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_wcdma_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_nr5g_rsrp_delta(60);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_nr5g_snr_delta(40);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_nr5g_rpt_rate_avg_period(
        NR5G_COMMON_RSSI_REPORTING_RATE_3_SEC_V01, NR5G_COMMON_RSSI_AVERAGING_PERIOD_5_SEC_V01);
  t.expect(QMI_node);

  // Inject: QMI_NAS_CONFIG_SIG_INFO2_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_config_sig_info2_resp_msg_v01();
  QMI_node.set_qmi_nas_config_sig_info2_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_SET_EVENT_REPORT_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_set_event_report_req_msg_v01();
  QMI_node.set_qmi_nas_set_event_report_req_report_reg_reject(TRUE);
  t.expect(QMI_node);

  // Inject: QMI_NAS_SET_EVENT_REPORT_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_set_event_report_resp_msg_v01();
  QMI_node.set_qmi_nas_set_event_report_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

#if 0
  // Expect: RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED
  RIL_node.update_default_ril_unsol_resp_radio_state_changed();
  t.expect(RIL_node);
#endif

//  // Expect: ims_MsgId_UNSOL_RADIO_STATE_CHANGED
//  IMS_node.update_default_ims_unsol_radio_state_changed(ims_RadioState_RADIO_STATE_ON);
//  t.expect(IMS_node);

  // Expect: QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_indication_register_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_INDICATION_REGISTER_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_indication_register_resp_msg_v01();
  QMI_node.set_qmi_nas_nas_indication_register_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_config_plmn_name_ind_reporting_req_msg_v01();
  QMI_node.set_qmi_nas_config_plmn_name_ind_reporting_req_msg_send_all_information(1);
  t.expect(QMI_node);

  // Inject: QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_config_plmn_name_ind_reporting_resp_msg_v01();
  QMI_node.set_qmi_nas_config_plmn_name_ind_reporting_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: RIL_REQUEST_RADIO_POWER (resp)
  RIL_node.update_default_ril_request_radio_power_resp();
  t.expect(RIL_node);

  // Expect: RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED
  RIL_node.update_default_ril_unsol_resp_radio_state_changed();
  t.expect(RIL_node);

  // Expect: RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_voice_network_state_changed();
  t.expect(RIL_node);

  // Expect: QMI_NAS_GET_SUB_BLOCKED_STATUS_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_sub_blocked_status_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_SUB_BLOCKED_STATUS_RESP_MSG_V01
  nas_get_sub_blocked_status_resp_msg_v01 *get_sub_blocked_ptr =  QMI_node.update_default_qmi_nas_get_sub_blocked_status_resp_msg_v01();
  QMI_node.set_qmi_nas_get_sub_blocked_status_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_nas_get_sub_blocked_status_resp_msg_is_blocked(1,0);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
  nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
  // Set values for data
  ptr->gsm_srv_status_info_valid = TRUE;
  ptr->gsm_srv_status_info.srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
  ptr->gsm_srv_status_info.true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
  ptr->gsm_srv_status_info.is_pref_data_path = 0;
  ptr->wcdma_srv_status_info_valid = TRUE;
  ptr->wcdma_srv_status_info.srv_status = NAS_SYS_SRV_STATUS_SRV_V01;
  ptr->wcdma_srv_status_info.true_srv_status = SYS_SRV_STATUS_SRV_V01;
  ptr->wcdma_srv_status_info.is_pref_data_path = 0;
  ptr->wcdma_sys_info.common_sys_info.srv_domain_valid = TRUE;
  ptr->wcdma_sys_info.common_sys_info.srv_domain = SYS_SRV_DOMAIN_CS_PS_V01;
  ptr->wcdma_sys_info.common_sys_info.srv_capability_valid = TRUE;
  ptr->wcdma_sys_info.common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
  ptr->wcdma_sys_info.common_sys_info.roam_status_valid = TRUE;
  ptr->wcdma_sys_info.common_sys_info.roam_status = NAS_SYS_ROAM_STATUS_OFF_V01;
  ptr->wcdma_sys_info.common_sys_info.is_sys_forbidden_valid = TRUE;
  ptr->wcdma_sys_info.common_sys_info.is_sys_forbidden = 0;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.lac_valid = TRUE;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.lac = 43024;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.cell_id_valid = TRUE;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.cell_id = 13548179;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.network_id_valid = TRUE;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.hs_call_status_valid = TRUE;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.hs_call_status = SYS_HS_IND_HSDPA_HSUPA_UNSUPP_CELL_V01;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.hs_ind_valid = TRUE;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.hs_ind = SYS_HS_IND_HSDPA_HSUPA_UNSUPP_CELL_V01;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.psc_valid = TRUE;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.psc = 426;
  ptr->wcdma_cipher_domain_valid = TRUE;
  ptr->wcdma_cipher_domain = SYS_SRV_DOMAIN_CS_ONLY_V01;
  ptr->sim_rej_info_valid = TRUE;
  ptr->sim_rej_info = NAS_SIM_AVAILABLE_V01;

  QMI_node.set_qmi_nas_get_sys_info_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);

  // Expect: QMI_NAS_GET_SIG_INFO_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_sig_info_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_SIG_INFO_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_sig_info_resp_msg_v01();
  QMI_node.set_qmi_nas_get_sig_info_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_ERR_RATE_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_err_rate_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_ERR_RATE_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_err_rate_resp_msg_v01();
  QMI_node.set_qmi_nas_get_err_rate_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_EMBMS_STATUS_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_embms_status_req_msg_v01();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_EMBMS_STATUS_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_embms_status_resp_msg_v01();
  QMI_node.set_qmi_nas_get_embms_status_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_PBM_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_pbm_indication_register_req_msg();
  QMI_node.set_qmi_pbm_indication_register_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01|PBM_REG_RECORD_UPDATE_EVENTS_V01|PBM_REG_PHONEBOOK_READY_EVENTS_V01);
  t.expect(QMI_node);

  // Inject: QMI_PBM_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_pbm_indication_register_resp_msg();
  QMI_node.set_qmi_pbm_indication_register_resp_v01_reg_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01|PBM_REG_RECORD_UPDATE_EVENTS_V01|PBM_REG_PHONEBOOK_READY_EVENTS_V01);
  QMI_node.set_qmi_pbm_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  //Inject: QMI_NAS_SUBSCRIPTION_INFO_IND_MSG_V01
  QMI_node.update_default_qmi_nas_subscription_info_ind_msg();
  QMI_node.set_qmi_nas_subscription_info_ind_voice_system_id(0x10c01000);
  QMI_node.set_qmi_nas_subscription_info_ind_lte_voice_system_id(0x10c01000);
  QMI_node.set_qmi_nas_subscription_info_ind_wlan_voice_system_id(0x10c01000);
  t.inject(QMI_node);

  // Expect: RIL_UNSOL_OEM_HOOK_RAW
  RIL_node.update_default_ril_unsol_oem_hook_raw();
  RIL_node.set_expect_skip_verification_status(false);
  t.expect(RIL_node);

  // Expect: RIL_UNSOL_VOICE_RADIO_TECH_CHANGED
  RIL_node.update_default_ril_unsol_voice_radio_tech_changed();
  t.expect(RIL_node);

  // Expect: RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_voice_network_state_changed();
  t.expect(RIL_node);

   // Expect: RIL_UNSOL_SIGNAL_STRENGTH
  RIL_node.update_default_ril_unsol_signal_strength();
  t.expect(RIL_node);
}
#else
void ril_utf_bootup_helper(Testcase &t, int boot_config)
{
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  IMSTestnode IMS_node;
  OEMTestnode OEM_node;
  DBTestnode DB_node;

  if ((boot_config == DEFAULT_BOOT) || (boot_config == PDC_BOOT))
  {
  // Expect: RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED
  RIL_node.update_default_ril_unsol_resp_radio_state_changed();
  t.expect(RIL_node);
  }

  // Update qcril.db property table
  util_ril_utf_db_property_init(t, DB_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_reg_voice_privacy_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_ext_brst_intl_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_speech_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_handover_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_conference_events(0x01);
  //QMI_node.set_qmi_voice_ind_req_tty_info_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_tty_info_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_cc_result_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_additional_call_info_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_audio_rat_change_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_vice_dialog_event(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_conf_participants_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_INDICATION_REGISTER_REQ_MSG_V01
  QMI_node.update_default_qmi_dms_indication_register_req_v01();
  QMI_node.set_qmi_dms_ind_req_report_ims_capability(0x01);
  t.expect(QMI_node);

  // Inject: QMI_DMS_INDICATION_REGISTER_RESP_MSG_V01
  QMI_node.update_default_qmi_dms_indication_register_resp_v01();
  QMI_node.set_qmi_dms_indication_register_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_SET_EVENT_REPORT_REQ_V01
  QMI_node.update_default_qmi_dms_set_event_report_req_msg();
  QMI_node.set_qmi_dsm_set_event_report_req_report_oprt_mode_state(0x01);
  QMI_node.set_qmi_dsm_set_event_report_req_report_prl_init(0x01);
  t.expect(QMI_node);

  // Inject: QMI_DMS_SET_EVENT_REPORT_RESP_V01
  QMI_node.update_default_qmi_dms_set_event_report_resp_msg();
  QMI_node.set_qmi_dms_set_event_report_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_GET_DEVICE_CAP_REQ_V01
  QMI_node.update_default_qmi_dms_get_device_cap_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_DEVICE_CAP_RESP_V01
  QMI_node.update_default_qmi_dms_get_device_cap_resp_v01();
  if (boot_config == PDC_BOOT)
  {
    dms_multisim_capability_type_v01 multisim_capability;
    memset(&multisim_capability, 0, sizeof(multisim_capability));
    multisim_capability.max_subscriptions = 2;
    QMI_node.set_default_qmi_dms_get_device_cap_resp_v01_multisim_capability(multisim_capability);
  }
  QMI_node.set_default_qmi_dms_get_device_cap_resp_v01_device_capabilities();
  QMI_node.set_qmi_dms_get_device_cap_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  /*
  // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
  QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_SHUTTING_DOWN_V01);
  QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);
  */

  // Expect: QMI_DMS_QMI_DMS_TRIGGER_MODEM_ACTIVITY_INFO_CALCULATION_REQ_V01
  QMI_node.update_default_qmi_dms_trigger_modem_activity_info_calculation_req(TRUE, 1);
  t.expect(QMI_node);

  // Inject: QMI_DMS_QMI_DMS_TRIGGER_MODEM_ACTIVITY_INFO_CALCULATION_RESP_V01
  QMI_node.update_default_qmi_dms_trigger_modem_activity_info_calculation_resp();
  t.inject(QMI_node);

  t.set_start();

  if(SSR_BOOT != boot_config)
  {
    // Expect: QMI_IMS_SETTINGS_GET_SUPPORTED_MSGS_REQ_V01
    QMI_node.update_default_qmi_ims_settings_get_supported_msgs_req();
    t.expect(QMI_node);

    // Inject: QMI_IMS_SETTINGS_GET_SUPPORTED_MSGS_RESP_V01
    QMI_node.update_default_qmi_ims_settings_get_supported_msgs_resp();
    t.inject(QMI_node);
  }

#ifdef RIL_UTF_L_MASTER
  // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
  t.expect(QMI_node);

  // Inject:QMI_DMS_GET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
  QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_SHUTTING_DOWN_V01);
  QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);
#endif

  // Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_V01
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_emergency_mode(0);
  mode_pref_mask_type_v01 mode_pref1 = QMI_NAS_RAT_MODE_PREF_UMTS_V01;
  mode_pref1 |= QMI_NAS_RAT_MODE_PREF_GSM_V01;
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(mode_pref1);
  // Set band_pref
  nas_band_pref_mask_type_v01 band_pref;
  band_pref |= QMI_NAS_BAND_CLASS_0_A_SYSTEM_V01;
  band_pref |= QMI_NAS_BAND_CLASS_0_B_AB_GSM850_V01;
  band_pref |= QMI_NAS_BAND_CLASS_1_ALL_BLOCKS_V01;
  band_pref |= QMI_NAS_BAND_CLASS_2_PLACEHOLDER_V01;
  band_pref |= QMI_NAS_BAND_CLASS_3_A_SYSTEM_V01;
  band_pref |= QMI_NAS_BAND_CLASS_4_ALL_BLOCKS_V01;
  band_pref |= QMI_NAS_BAND_CLASS_5_ALL_BLOCKS_V01;
  band_pref |= QMI_NAS_GSM_DCS_1800_BAND_V01;
  band_pref |= QMI_NAS_E_GSM_900_BAND_V01;
  band_pref |= QMI_NAS_P_GSM_900_BAND_V01;
  band_pref |= QMI_NAS_BAND_CLASS_6_V01;
  band_pref |= QMI_NAS_BAND_CLASS_7_V01;
  band_pref |= QMI_NAS_BAND_CLASS_8_V01;
  band_pref |= QMI_NAS_BAND_CLASS_9_V01;
  band_pref |= QMI_NAS_BAND_CLASS_10_V01;
  band_pref |= QMI_NAS_BAND_CLASS_11_V01;
  band_pref |= QMI_NAS_GSM_BAND_450_V01;
  band_pref |= QMI_NAS_GSM_BAND_480_V01;
  band_pref |= QMI_NAS_GSM_BAND_750_V01;
  band_pref |= QMI_NAS_GSM_BAND_850_V01;
  band_pref |= QMI_NAS_GSM_BAND_RAILWAYS_900_BAND_V01;
  band_pref |= QMI_NAS_GSM_BAND_PCS_1900_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_EU_J_CH_IMT_2100_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_US_PCS_1900_BAND_V01;
  band_pref |= QMI_NAS_EU_CH_DCS_1800_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_US_1700_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_JAPAN_800_BAND_V01;
  band_pref |= QMI_NAS_BAND_CLASS_12_V01;
  band_pref |= QMI_NAS_BAND_CLASS_14_V01;
  band_pref |= QMI_NAS_RESERVED_V01;
  band_pref |= QMI_NAS_BAND_CLASS_15_V01;
  band_pref |= QMI_NAS_WCDMA_EU_2600_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_EU_J_900_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_J_1700_BAND_V01;
  band_pref |= QMI_NAS_BAND_CLASS_16_V01;
  band_pref |= QMI_NAS_BAND_CLASS_17_V01;
  band_pref |= QMI_NAS_BAND_CLASS_18_V01;
  band_pref |= QMI_NAS_BAND_CLASS_19_V01;
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_band_pref(band_pref);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_prl_pref(NAS_PRL_PREF_ANY_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_roam_pref(NAS_ROAMING_PREF_ANY_V01);
  // Set band_pref_ext
  uint64_t band_pref_ext;
  band_pref_ext |= E_UTRA_OPERATING_BAND_1_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_2_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_3_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_4_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_5_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_6_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_7_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_8_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_9_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_10_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_11_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_12_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_13_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_14_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_17_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_18_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_19_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_20_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_21_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_24_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_25_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_33_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_34_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_35_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_36_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_37_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_38_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_39_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_40_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_41_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_42_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_43_V01;
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_band_pref_ext(band_pref_ext);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_net_sel_pref(NAS_NET_SEL_PREF_AUTOMATIC_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_PS_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_gw_acq_order_pref(NAS_GW_ACQ_ORDER_PREF_AUTOMATIC_V01);
  // Set tdscdma_band_pref
  nas_tdscdma_band_pref_mask_type_v01 tdscdma_band_pref;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_A_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_B_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_C_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_D_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_E_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_F_V01;
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_tdscdma_band_pref(tdscdma_band_pref);
  QMI_node.set_default_qmi_nas_get_system_selection_preference_resp_acq_order();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_reg_restriction(NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_usage_setting(NAS_USAGE_VOICE_CENTRIC_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  t.set_end();


  // Start set of unordered expects
  t.set_start();

  // Expect: QMI_NAS_CONFIG_SIG_INFO2_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_config_sig_info2_req_msg_v01();
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_cdma_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_cdma_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_gsm_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_sinr_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rsrp_delta(60);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rsrq_delta(20);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_snr_delta(40);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_ecio_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_rscp_delta(40);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_rssi_delta(5);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_sinr_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_wcdma_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_wcdma_rssi_delta(50);
  t.expect(QMI_node);

  // Inject: QMI_NAS_CONFIG_SIG_INFO2_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_config_sig_info2_resp_msg_v01();
  QMI_node.set_qmi_nas_config_sig_info2_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_SET_EVENT_REPORT_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_set_event_report_req_msg_v01();
  QMI_node.set_qmi_nas_set_event_report_req_report_reg_reject(TRUE);
  t.expect(QMI_node);

  // Inject: QMI_NAS_SET_EVENT_REPORT_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_set_event_report_resp_msg_v01();
  QMI_node.set_qmi_nas_set_event_report_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_indication_register_req_msg_v01();
  QMI_node.set_qmi_nas_indication_register_req_msg_reg_sys_sel_pref(1);
  QMI_node.set_qmi_nas_indication_register_req_msg_dual_standby_pref(1);
  QMI_node.set_qmi_nas_indication_register_req_msg_subscription_info(1);
  QMI_node.set_qmi_nas_indication_register_reg_msg_reg_rtre_cfg(1);
  QMI_node.set_qmi_nas_indication_register_req_msg_req_serving_system(0);
  QMI_node.set_qmi_nas_indication_register_req_msg_reg_network_time(0);
  QMI_node.set_qmi_nas_indication_register_reg_msg_reg_embms_status(0);
  QMI_node.set_qmi_nas_indication_register_req_msg_sig_info(0);
  QMI_node.set_qmi_nas_indication_register_reg_msg_err_rate(0);
  QMI_node.set_qmi_nas_indication_register_req_msg_reg_current_plmn_name(0);
  QMI_node.set_qmi_nas_indication_register_req_msg_reg_rf_band_info(0);
  t.expect(QMI_node);

  // Inject: QMI_NAS_INDICATION_REGISTER_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_indication_register_resp_msg_v01();
  QMI_node.set_qmi_nas_nas_indication_register_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_config_plmn_name_ind_reporting_req_msg_v01();
  QMI_node.set_qmi_nas_config_plmn_name_ind_reporting_req_msg_send_all_information(1);
  t.expect(QMI_node);

  // Inject: QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_config_plmn_name_ind_reporting_resp_msg_v01();
  QMI_node.set_qmi_nas_config_plmn_name_ind_reporting_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_CENTRALIZED_EONS_SUPPORT_STATUS_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_centralized_eons_support_status_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_CENTRALIZED_EONS_SUPPORT_STATUS_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_centralized_eons_support_status_resp_msg_v01();
  QMI_node.set_qmi_nas_get_centralized_eons_support_status_resp_msg_centralized_eons_supported(1);
  QMI_node.set_qmi_nas_get_centralized_eons_support_status_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_LTE_SIB16_NETWORK_TIME_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_lte_sib16_network_time_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_LTE_SIB16_NETWORK_TIME_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_lte_sib16_network_time_resp_msg_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
  QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_SHUTTING_DOWN_V01);
  QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_PBM_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_pbm_indication_register_req_msg();
  QMI_node.set_qmi_pbm_indication_register_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01);
  t.expect(QMI_node);

  // Inject: QMI_PBM_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_pbm_indication_register_resp_msg();
  QMI_node.set_qmi_pbm_indication_register_resp_v01_reg_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01);
  QMI_node.set_qmi_pbm_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_PBM_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_pbm_indication_register_req_msg();
  QMI_node.set_qmi_pbm_indication_register_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01|PBM_REG_RECORD_UPDATE_EVENTS_V01|PBM_REG_PHONEBOOK_READY_EVENTS_V01);
  t.expect(QMI_node);

  // Inject: QMI_PBM_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_pbm_indication_register_resp_msg();
  QMI_node.set_qmi_pbm_indication_register_resp_v01_reg_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01|PBM_REG_RECORD_UPDATE_EVENTS_V01|PBM_REG_PHONEBOOK_READY_EVENTS_V01);
  QMI_node.set_qmi_pbm_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_LTE_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_lte_indication_register_req_msg();
  QMI_node.set_qmi_lte_indication_register_req_indication_bitmask(
          QMI_LTE_DISC_NOTIFICATION_IND_MASK_V01 |
          QMI_LTE_DISC_BROADCAST_NOTIFICATION_IND_MASK_V01 |
          QMI_LTE_DISC_MATCH_NOTIFICATION_IND_MASK_V01 |
          QMI_LTE_DISC_PSK_EXPIRED_IND_MASK_V01);
  t.expect(QMI_node);

  // Inject: QMI_LTE_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_lte_indication_register_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_EVENT_REG_REQ_V01
  QMI_node.update_default_qmi_uim_event_reg_req(0x07);
  t.expect(QMI_node);

  // Inject: QMI_UIM_EVENT_REG_RESP_V01
  QMI_node.update_default_qmi_uim_event_reg_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_GET_CARD_STATUS_REQ_V01
  QMI_node.update_default_qmi_uim_get_card_status_req();
  QMI_node.set_qmi_uim_get_card_status_req_extended_card_status(0x01);
  t.expect(QMI_node);

  // Inject: QMI_UIM_GET_CARD_STATUS_RESP_V01
  uim_get_card_status_resp_msg_v01* card_status = QMI_node.update_default_qmi_uim_get_card_status_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  card_status->sim_busy_status_valid = TRUE;
  card_status->sim_busy_status_len = 0x01;
  card_status->sim_busy_status[0] = 0x00;
  card_status->extended_hot_swap_valid = TRUE;
  card_status->extended_hot_swap_len = 0x01;
  card_status->extended_hot_swap[0] = UIM_HOT_SWAP_NOT_SUPPORTED_V01;
  card_status->extended_card_status_valid = TRUE;
  card_status->extended_card_status.index_gw_len = 0x01;
  card_status->extended_card_status.index_1x_len = 0x01;
  card_status->extended_card_status.index_1x[0] = 0x0002;
  card_status->extended_card_status.card_info_len = 0x01;
  card_status->extended_card_status.card_info[0].card_state = UIM_EXTENDED_CARD_STATE_PRESENT_V01;
  card_status->extended_card_status.card_info[0].upin.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].upin.pin_retries = 0x00;
  card_status->extended_card_status.card_info[0].upin.puk_retries = 0x00;
  card_status->extended_card_status.card_info[0].error_code = UIM_CARD_ERROR_CODE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info_len = 0x04;
  // App slot 0
  card_status->extended_card_status.card_info[0].app_info[0].app_type = UIM_APP_TYPE_USIM_V01;
  card_status->extended_card_status.card_info[0].app_info[0].app_state = UIM_APP_STATE_READY_V01;
  card_status->extended_card_status.card_info[0].app_info[0].perso_state = UIM_PERSO_STATE_READY_V01;
  card_status->extended_card_status.card_info[0].app_info[0].perso_feature = UIM_PERSO_FEATURE_STATUS_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[0].perso_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[0].perso_unblock_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[0].aid_value_len = 0x10;
  uint8_t app_value_1[30] = {0xA0,0x00,0x00,0x00,0x87,0x10,0x02,0xF3,0x10,0xFF,0xFF,0x89,0x08,0x00,0x00,0xFF};
  memcpy(card_status->extended_card_status.card_info[0].app_info[0].aid_value, app_value_1, 16);
  card_status->extended_card_status.card_info[0].app_info[0].univ_pin = UIM_UNIV_PIN_PIN1_USED_V01;
  card_status->extended_card_status.card_info[0].app_info[0].pin1.pin_state = UIM_PIN_STATE_DISABLED_V01;
  card_status->extended_card_status.card_info[0].app_info[0].pin1.pin_retries = 0x03;
  card_status->extended_card_status.card_info[0].app_info[0].pin1.puk_retries = 0x0A;
  card_status->extended_card_status.card_info[0].app_info[0].pin2.pin_state = UIM_PIN_STATE_ENABLED_NOT_VERIFIED_V01;
  card_status->extended_card_status.card_info[0].app_info[0].pin2.pin_retries = 0x03;
  card_status->extended_card_status.card_info[0].app_info[0].pin2.puk_retries = 0x0A;
  // App slot 1
  card_status->extended_card_status.card_info[0].app_info[1].app_type = UIM_APP_TYPE_ISIM_V01;
  card_status->extended_card_status.card_info[0].app_info[1].app_state = UIM_APP_STATE_DETECTED_V01;
  card_status->extended_card_status.card_info[0].app_info[1].perso_state = UIM_PERSO_STATE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[1].perso_feature = UIM_PERSO_FEATURE_STATUS_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[1].perso_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[1].perso_unblock_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[1].aid_value_len = 0x10;
  uint8_t app_value_2[30] = {0xA0,0x00,0x00,0x00,0x87,0x10,0x04,0xF3,0x10,0xFF,0xFF,0x89,0x08,0x00,0x00,0xFF};
  memcpy(card_status->extended_card_status.card_info[0].app_info[1].aid_value, app_value_2, 16);
  card_status->extended_card_status.card_info[0].app_info[1].univ_pin = UIM_UNIV_PIN_PIN1_USED_V01;
  card_status->extended_card_status.card_info[0].app_info[1].pin1.pin_state = UIM_PIN_STATE_DISABLED_V01;
  card_status->extended_card_status.card_info[0].app_info[1].pin1.pin_retries = 0x03;
  card_status->extended_card_status.card_info[0].app_info[1].pin1.puk_retries = 0x0A;
  card_status->extended_card_status.card_info[0].app_info[1].pin2.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[1].pin2.pin_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[1].pin2.puk_retries = 0x00;
  // App slot 2
  card_status->extended_card_status.card_info[0].app_info[2].app_type = UIM_APP_TYPE_CSIM_V01;
  card_status->extended_card_status.card_info[0].app_info[2].app_state = UIM_APP_STATE_READY_V01;
  card_status->extended_card_status.card_info[0].app_info[2].perso_state = UIM_PERSO_STATE_READY_V01;
  card_status->extended_card_status.card_info[0].app_info[2].perso_feature = UIM_PERSO_FEATURE_STATUS_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[2].perso_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[2].perso_unblock_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[2].aid_value_len = 0x10;
  uint8_t app_value_3[30] = {0xA0,0x00,0x00,0x03,0x43,0x10,0x02,0xF3,0x10,0xFF,0xFF,0x89,0x02,0x00,0x00,0xFF};
  memcpy(card_status->extended_card_status.card_info[0].app_info[2].aid_value, app_value_3, 16);
  card_status->extended_card_status.card_info[0].app_info[2].univ_pin = UIM_UNIV_PIN_PIN1_USED_V01;
  card_status->extended_card_status.card_info[0].app_info[2].pin1.pin_state = UIM_PIN_STATE_DISABLED_V01;
  card_status->extended_card_status.card_info[0].app_info[2].pin1.pin_retries = 0x03;
  card_status->extended_card_status.card_info[0].app_info[2].pin1.puk_retries = 0x0A;
  card_status->extended_card_status.card_info[0].app_info[2].pin2.pin_state = UIM_PIN_STATE_ENABLED_NOT_VERIFIED_V01;
  card_status->extended_card_status.card_info[0].app_info[2].pin2.pin_retries = 0x03;
  card_status->extended_card_status.card_info[0].app_info[2].pin2.puk_retries = 0x0A;
  // App slot 3
  card_status->extended_card_status.card_info[0].app_info[3].app_type = UIM_APP_TYPE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[3].app_state = UIM_APP_STATE_DETECTED_V01;
  card_status->extended_card_status.card_info[0].app_info[3].perso_state = UIM_PERSO_STATE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[3].perso_feature = UIM_PERSO_FEATURE_STATUS_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[3].perso_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[3].perso_unblock_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[3].aid_value_len = 0x0c;
  uint8_t app_value_4[30] = {0xA0,0x00,0x00,0x00,0x63,0x50,0x4B,0x43,0x53,0x2D,0x31,0x35};
  memcpy(card_status->extended_card_status.card_info[0].app_info[3].aid_value, app_value_4, 12);
  card_status->extended_card_status.card_info[0].app_info[3].univ_pin = UIM_UNIV_PIN_PIN1_USED_V01;
  card_status->extended_card_status.card_info[0].app_info[3].pin1.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[3].pin1.pin_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[3].pin1.puk_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[3].pin2.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[3].pin2.pin_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[3].pin2.puk_retries = 0x00;
  t.inject(QMI_node);

  // Expect: QMI_CAT_GET_CONFIGURATION_REQ_V02
  QMI_node.update_default_qmi_cat_get_configuration_req();
  t.expect(QMI_node);

  // Inject: QMI_CAT_GET_CONFIGURATION_RESP_V02
  QMI_node.update_default_qmi_cat_get_configuration_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_CAT_SET_EVENT_REPORT_REQ_V02
  QMI_node.update_default_qmi_cat_set_event_report_req();
#ifdef RIL_UTF_L_MASTER
  QMI_node.set_qmi_cat_set_event_report_req_pc_evt_report_req_mask(0x0b7FFFFF);
#else
  QMI_node.set_qmi_cat_set_event_report_req_pc_evt_report_req_mask(0x037FFFFF);
#endif
  QMI_node.set_qmi_cat_set_event_report_req_slot_mask(CAT_SET_EVENT_REPORT_SLOT_1_V02);
  t.expect(QMI_node);

  // Inject: QMI_CAT_SET_EVENT_REPORT_RESP_V02
  QMI_node.update_default_qmi_cat_set_event_report_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_CAT_SET_EVENT_REPORT_REQ_V02
  QMI_node.update_default_qmi_cat_set_event_report_req();
  QMI_node.set_qmi_cat_set_event_report_req_pc_dec_evt_report_req_mask(0x00800000);
  QMI_node.set_qmi_cat_set_event_report_req_slot_mask(CAT_SET_EVENT_REPORT_SLOT_1_V02);
  t.expect(QMI_node);

  // Inject: QMI_CAT_SET_EVENT_REPORT_RESP_V02
  QMI_node.update_default_qmi_cat_set_event_report_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ_V01
  QMI_node.update_default_qmi_wms_get_transport_layer_info_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_WMS_GET_TRANSPORT_LAYER_INFO_RESP_V01
  QMI_node.update_default_qmi_wms_get_transport_layer_info_resp_v01();
  QMI_node.set_qmi_wms_get_transport_layer_info_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_IMSP_SET_EVENT_REPORT_REQ_V01
  QMI_node.update_default_qmi_imsp_set_event_report_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_IMSP_SET_EVENT_REPORT_RESP_V01
  QMI_node.update_default_qmi_imsp_set_event_report_resp_v01();
  QMI_node.set_qmi_imsp_set_event_report_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  if ((boot_config == DEFAULT_BOOT) || (boot_config == PDC_BOOT))
  {
      // Expect: QMI_IMS_SETTINGS_GET_REG_MGR_CONFIG_REQ_V01
      QMI_node.update_default_qmi_ims_settings_get_reg_mgr_config_req_v01();
      t.expect(QMI_node);

      // Inject: QMI_IMS_SETTINGS_GET_REG_MGR_CONFIG_RSP_V01
      QMI_node.update_default_qmi_ims_settings_get_reg_mgr_config_rsp_v01();
      QMI_node.set_qmi_ims_settings_get_reg_mgr_config_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
      t.inject(QMI_node);

      // Expect: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_REQ_V01
      QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_req_v01();
      t.expect(QMI_node);

      // Inject: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_RSP_V01
      QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_rsp_v01();
      QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
      t.inject(QMI_node);

      // Expect: QMI_IMSA_IND_REG_REQ_V01
      QMI_node.update_default_qmi_imsa_ind_reg_req_v01();
      t.expect(QMI_node);

      // Inject: QMI_IMSA_IND_REG_RSP_V01
      QMI_node.update_default_qmi_imsa_ind_reg_rsp_v01();
      QMI_node.set_qmi_imsa_ind_req_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
      t.inject(QMI_node);

      // Expect: QMI_IMS_SETTINGS_CONFIG_IND_REG_REQ_V01
      QMI_node.update_default_qmi_ims_config_ind_reg_req_v01();
      t.expect(QMI_node);

      // Inject: QMI_IMS_SETTINGS_CONFIG_IND_REG_RSP_V01
      QMI_node.update_default_qmi_ims_config_ind_reg_rsp_v01();
      QMI_node.set_qmi_ims_config_ind_req_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
      t.inject(QMI_node);
  }

  //QMI_NAS_GET_SUBSCRIPTION_INFO_REQ_MSG_V01
  util_ril_utf_get_subscription_info(t, QMI_node, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, TRUE, NAS_PRIORITY_SUBSCRIPTION_TRUE_V01, TRUE, NAS_SUBSCRIPTION_ACTIVE_V01, TRUE, 0, TRUE, 0, FALSE, 0, FALSE, 0);

  // Expect: QMI_DMS_GET_DEVICE_CAP_REQ_V01
  QMI_node.update_default_qmi_dms_get_device_cap_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_DEVICE_CAP_RESP_V01
  QMI_node.update_default_qmi_dms_get_device_cap_resp_v01();
  QMI_node.set_default_qmi_dms_get_device_cap_resp_v01_device_capabilities();
  QMI_node.set_qmi_dms_get_device_cap_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  if (boot_config == SSR_BOOT)
  {
      // Expect: QMI_EMBMS_UNSOL_EMBMS_STATUS_IND
      OEM_node.update_default_qmi_embms_unsol_embms_status_ind();
      OEM_node.set_qmi_embms_unsol_embms_status(0);
      t.expect(OEM_node);
  }

  // Expect: QMI_DMS_GET_CURRENT_PRL_INFO_REQ_V01
  QMI_node.update_default_qmi_dms_get_current_prl_info_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_CURRENT_PRL_INFO_RESP_V01
  QMI_node.update_default_qmi_dms_get_current_prl_info_resp_v01();
  QMI_node.set_qmi_dms_get_current_prl_info_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED
  RIL_node.update_default_ril_unsol_resp_radio_state_changed();
  t.expect(RIL_node);

//  // Expect: ims_MsgId_UNSOL_RADIO_STATE_CHANGED
//  IMS_node.update_default_ims_unsol_radio_state_changed(ims_RadioState_RADIO_STATE_OFF);
//  t.expect(IMS_node);

  if (boot_config == SSR_BOOT)
  {
//  // Expect: ims_MsgId_UNSOL_RADIO_STATE_CHANGED
//  IMS_node.update_default_ims_unsol_radio_state_changed(ims_RadioState_RADIO_STATE_OFF);
//  t.expect(IMS_node);

  // Expect: LteDirectDiscovery_MsgId_UNSOL_RESPONSE_SERVICE_STATUS
  OEM_node.update_default_lte_direct_discovery_unsol_response_service_status();
  OEM_node.set_lte_direct_discovery_unsol_response_service_status_publish_allowed(0);
  OEM_node.set_lte_direct_discovery_unsol_response_service_status_subscribe_allowed(0);
  t.expect(OEM_node);
  }

  // Expect: QMI_DMS_GET_DEVICE_MFR_REQ_V01
  QMI_node.update_default_qmi_dms_get_device_mfr_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_DEVICE_MFR_RESP_V01
  QMI_node.update_default_qmi_dms_get_device_mfr_resp_v01();
  QMI_node.set_default_qmi_dms_get_device_mfr_resp_v01_device_manufacturer();
  QMI_node.set_qmi_dms_get_device_mfr_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_SET_PRIMARY_CLIENT_REQ_V01
  QMI_node.update_default_qmi_wms_set_primary_client_req_v01();
  QMI_node.set_qmi_wms_set_primary_client_req_v01_primary_client(1);
  t.expect(QMI_node);

  // Inject: QMI_WMS_SET_PRIMARY_CLIENT_RESP_V01
  QMI_node.update_default_qmi_wms_set_primary_client_resp_v01();
  QMI_node.set_qmi_wms_set_primary_client_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_SET_ROUTES_REQ_V01
  QMI_node.update_default_qmi_wms_set_routes_req_v01();
  // TODO:  Add sane values here and enable payload validation
  t.expect(QMI_node);

  // Inject: QMI_WMS_SET_ROUTES_RESP_V01
  QMI_node.update_default_qmi_wms_set_routes_resp_v01();
  QMI_node.set_qmi_wms_set_routes_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_SET_EVENT_REPORT_REQ_V01
  QMI_node.update_default_qmi_wms_set_event_report_req_v01();
  // TODO:  Add sane values here and enable payload validation
  t.expect(QMI_node);

  // Inject: QMI_WMS_SET_EVENT_REPORT_RESP_V01
  QMI_node.update_default_qmi_wms_set_event_report_resp_v01();
  QMI_node.set_qmi_wms_set_event_report_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_READ_TRANSPARENT_REQ_V01
  QMI_node.update_default_qmi_uim_read_transparent_reg();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_READ_TRANSPARENT_RESP_V01
  QMI_node.update_default_qmi_uim_read_transparent_resp(QMI_RESULT_SUCCESS_V01,QMI_ERR_NONE_V01);
  t.inject(QMI_node);

#ifdef RIL_UTF_L_MASTER
  if (boot_config == PDC_BOOT)
  {
    /*
    // Expect: QMI_UIM_READ_TRANSPARENT_REQ_V01
    uim_read_transparent_req_msg_v01* rt_req_ptr = QMI_node.update_default_qmi_uim_read_transparent_reg();
    rt_req_ptr->session_information.session_type = UIM_SESSION_TYPE_CARD_ON_SLOT_1_V01;
    rt_req_ptr->file_id.file_id = 0x2FE2;
    rt_req_ptr->file_id.path_len = 0x02;
    rt_req_ptr->file_id.path[0] = 0x00;
    rt_req_ptr->file_id.path[1] = 0x3F;
    t.expect(QMI_node);

    // Inject: QMI_UIM_READ_TRANSPARENT_RESP_V01
    uim_read_transparent_resp_msg_v01* rt_resp_ptr = QMI_node.update_default_qmi_uim_read_transparent_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    rt_resp_ptr->read_result_valid = 1;
    rt_resp_ptr->read_result.content_len = 10;
    memcpy(rt_resp_ptr->read_result.content, tmp_uim_id, 10);
    t.inject(QMI_node);

    // Expect: QMI_DMS_UIM_GET_ICCID_REQ_V01
    QMI_node.update_default_qmi_dms_get_iccid_req();
    t.expect(QMI_node);

    // Inject: QMI_DMS_UIM_GET_ICCID_RESP_V01
    uim_id = QMI_node.update_default_qmi_dms_get_iccid_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    memcpy(uim_id, tmp_uim_id, 10);
    t.inject(QMI_node);
    */
  }
  else
  {
    /*
    // Expect: QMI_UIM_READ_TRANSPARENT_REQ_V01
    QMI_node.update_default_qmi_uim_read_transparent_reg();
    QMI_node.set_expect_skip_verification_status(false);
    t.expect(QMI_node);

    // Inject: QMI_UIM_READ_TRANSPARENT_RESP_V01
    QMI_node.update_default_qmi_uim_read_transparent_resp(QMI_RESULT_FAILURE_V01, QMI_ERR_GENERAL_V01);
    t.inject(QMI_node);

    // Expect: QMI_UIM_READ_TRANSPARENT_REQ_V01
    QMI_node.update_default_qmi_uim_read_transparent_reg();
    QMI_node.set_expect_skip_verification_status(false);
    t.expect(QMI_node);

    // Inject: QMI_UIM_READ_TRANSPARENT_RESP_V01
    QMI_node.update_default_qmi_uim_read_transparent_resp(QMI_RESULT_FAILURE_V01, QMI_ERR_GENERAL_V01);
    t.inject(QMI_node);

    // Expect: QMI_DMS_UIM_GET_ICCID_REQ_V01
    QMI_node.update_default_qmi_dms_get_iccid_req();
    t.expect(QMI_node);

    // Inject: QMI_DMS_UIM_GET_ICCID_RESP_V01
    uim_id = QMI_node.update_default_qmi_dms_get_iccid_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    memcpy(uim_id, tmp_uim_id, 10);
    t.inject(QMI_node);

    // Expect: QMI_DMS_UIM_GET_ICCID_REQ_V01
    QMI_node.update_default_qmi_dms_get_iccid_req();
    t.expect(QMI_node);

    // Inject: QMI_DMS_UIM_GET_ICCID_RESP_V01
    uim_id = QMI_node.update_default_qmi_dms_get_iccid_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    memcpy(uim_id, tmp_uim_id, 10);
    t.inject(QMI_node);
    */
  }
#endif

  // Expect: QMI_WMS_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_wms_indication_register_req_v01();
  QMI_node.set_qmi_wms_indication_register_req_v01_reg_transport_layer_info_events(1);
  QMI_node.set_qmi_wms_indication_register_req_v01_reg_transport_layer_mwi_info_events(1);
  t.expect(QMI_node);

  // Inject: QMI_WMS_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_wms_indication_register_resp_v01();
  QMI_node.set_qmi_wms_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_GET_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_wms_get_indication_register_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_WMS_GET_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_wms_get_indication_register_resp_v01();
  QMI_node.set_qmi_wms_get_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  //TODO: Fill with sane values to send to qmi-ril
  t.inject(QMI_node);

  // Expect: QMI_UIM_GET_SERVICE_STATUS_REQ_V01
  uim_session_information_type_v01 session_info;
  memset(&session_info, 0, sizeof(session_info));
  session_info.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
  session_info.aid_len = 0;
  QMI_node.update_default_qmi_uim_get_service_status_req(session_info, 0x01);
  t.expect(QMI_node);

  // Inject: QMI_UIM_GET_SERVICE_STATUS_RESP_V01
  QMI_node.update_default_qmi_uim_get_service_status_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_uim_get_service_status_resp_fdn_status(UIM_FDN_STATUS_AVAILABLE_DISABLED_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_GET_SERVICE_STATUS_REQ_V01
  memset(&session_info, 0, sizeof(session_info));
  session_info.session_type = UIM_SESSION_TYPE_PRIMARY_1X_V01;
  session_info.aid_len = 0;
  QMI_node.update_default_qmi_uim_get_service_status_req(session_info, 0x01);
  t.expect(QMI_node);

  // Inject: QMI_UIM_GET_SERVICE_STATUS_RESP_V01
  QMI_node.update_default_qmi_uim_get_service_status_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_uim_get_service_status_resp_fdn_status(UIM_FDN_STATUS_AVAILABLE_DISABLED_V01);
  t.inject(QMI_node);

  if (boot_config == SSR_BOOT)
  {
  // Expect: QMI_UIM_READ_TRANSPARENT_REQ_V01
  QMI_node.update_default_qmi_uim_read_transparent_reg();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_READ_TRANSPARENT_RESP_V01
  QMI_node.update_default_qmi_uim_read_transparent_resp(QMI_RESULT_FAILURE_V01, QMI_ERR_GENERAL_V01);
  t.inject(QMI_node);
  }

  // Expect: QMI_DMS_SET_AP_SW_VERSION_REQ_V01
  QMI_node.update_default_qmi_dms_set_ap_sw_version_req_v01();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_DMS_SET_AP_SW_VERSION_RESP_V01
  QMI_node.update_default_qmi_dms_set_ap_sw_version_resp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_GET_FILE_ATTRIBUTES_REQ_V01
  memset(&session_info, 0, sizeof(session_info));
  session_info.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
  session_info.aid_len = 0x00;
  uim_file_id_type_v01 file_id;
  memset(&file_id, 0, sizeof(file_id));
  uint8_t path[10] = {0x00,0x3F,0x10,0x7F,0x3A,0x5F};
  file_id.file_id = 0x4f30;
  file_id.path_len = 0x06;
  memcpy(file_id.path, path, 0x06);
  QMI_node.update_default_qmi_uim_get_file_attributes_req(session_info, file_id);
  t.expect(QMI_node);

  // Inject: QMI_UIM_GET_FILE_ATTRIBUTES_RESP_V01
  QMI_node.update_default_qmi_uim_get_file_attributes_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  uim_card_result_type_v01 card_result;
  card_result.sw1 = 0x90;
  card_result.sw2 = 0x00;
  QMI_node.set_qmi_uim_get_file_attributes_resp_card_result(card_result);
  QMI_node.set_qmi_uim_get_file_attribues_resp_file_status(UIM_FILE_STATUS_ACTIVATED_V01);
  uim_file_attributes_type_v01* file_attributes = QMI_node.set_qmi_uim_get_file_attributes_resp_file_attributes();
  file_attributes->file_size = 0x4c;
  file_attributes->file_id = 0x4f30;
  file_attributes->file_type = UIM_FILE_TYPE_LINEAR_FIXED_V01;
  file_attributes->rec_size = 0x4c;
  file_attributes->rec_count = 0x01;
  file_attributes->sec_read = UIM_SEC_CONDITION_SINGLE_COND_V01;
  file_attributes->sec_read_mask = 0x01;
  file_attributes->sec_write = UIM_SEC_CONDITION_SINGLE_COND_V01;
  file_attributes->sec_write_mask = 0x08;
  file_attributes->sec_increase = UIM_SEC_CONDITION_ALWAYS_V01;
  file_attributes->sec_increase_mask = 0x00;
  file_attributes->sec_deactivate = UIM_SEC_CONDITION_SINGLE_COND_V01;
  file_attributes->sec_deactivate_mask = 0x08;
  file_attributes->sec_activate = UIM_SEC_CONDITION_SINGLE_COND_V01;
  file_attributes->sec_activate_mask = 0x08;
  file_attributes->raw_value_len = 0x24;
  uint8_t temp_raw_value[40] = {0x62,0x22,0x82,0x05,0x42,0x21,0x00,0x4C,0x01,0x83,0x02,0x4F,0x30,0xA5,0x03,
    0xC0,0x01,0x00,0x8A,0x01,0x05,0x8B,0x03,0x6F,0x06,0x03,0x80,0x02,0x00,0x4C,0x81,0x02,0x00,0x60,0x88,0x00};
  memcpy(file_attributes->raw_value, temp_raw_value, 36);
  t.inject(QMI_node);

  // Expect: QMI_UIM_READ_RECORD_REQ_V01
  uim_read_record_req_msg_v01* read_req = QMI_node.update_default_qmi_uim_read_record_reg();
  read_req->session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
  read_req->session_information.aid_len = 0x00;
  read_req->file_id.file_id = 0x4f30;
  read_req->file_id.path_len = 0x06;
  uint8_t tmp_path[] = {0x00,0x3F,0x10,0x7F,0x3A,0x5F};
  memcpy(read_req->file_id.path, tmp_path, 0x06);
  read_req->read_record.record = 0x01;
  read_req->read_record.length = 0x4c;
  t.expect(QMI_node);

  // Inject: QMI_UIM_READ_RECORD_RESP_V01
  uim_read_record_resp_msg_v01* read_resp = QMI_node.update_default_qmi_uim_read_record_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  read_resp->read_result_valid = TRUE;
  read_resp->read_result.content_len = 0x4e;
  uint8_t tmp_content[] = {
    0x4C, 0x00, 0xA8, 0x23, 0xC0, 0x03, 0x4F, 0x3A, 0x01, 0xC5, 0x03, 0x4F, 0x09, 0x02, 0xC4, 0x03, 0x4F,
    0x11, 0x04, 0xC4, 0x03, 0x4F, 0x13, 0x05, 0xC4, 0x03, 0x4F, 0x15, 0x06, 0xCA, 0x03, 0x4F, 0x50, 0x09,
    0xC9, 0x03, 0x4F, 0x21, 0x0C, 0xAA, 0x0F, 0xC2, 0x03, 0x4F, 0x4A, 0x08, 0xC7, 0x03, 0x4F, 0x4B, 0x14,
    0xCB, 0x03, 0x4F, 0x3D, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
  };
  memcpy(read_resp->read_result.content, tmp_content, 0x4e);
  read_resp->card_result_valid = TRUE;
  read_resp->card_result.sw1 = 0x90;
  read_resp->card_result.sw2 = 0x00;
  t.inject(QMI_node);

  // Expect: QMI_UIM_REFRESH_REGISTER_REQ_V01
  uim_register_refresh_type_v01 reg_ref;
  QMI_node.update_default_qmi_uim_get_file_attributes_req(session_info, reg_ref);
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_REFRESH_REGISTER_RESP_V01
  QMI_node.update_default_qmi_uim_refresh_register_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_REFRESH_REGISTER_REQ_V01
  QMI_node.update_default_qmi_uim_get_file_attributes_req(session_info, reg_ref);
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_REFRESH_REGISTER_RESP_V01
  QMI_node.update_default_qmi_uim_refresh_register_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED
  RIL_node.update_default_ril_unsol_response_sim_status_changed();
  t.expect(RIL_node);

  /*
  // Inject: RIL_REQUEST_GET_SIM_STATUS
  RIL_node.update_default_ril_request_get_sim_status();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_GET_SIM_STATUS (resp)
  RIL_CardStatus_v6* status = RIL_node.update_default_ril_request_get_sim_status_resp();
  status->card_state = RIL_CARDSTATE_PRESENT;
  t.expect(RIL_node);
  */

  // Expect: QMI_DMS_GET_CURRENT_PRL_INFO_REQ_V01
  QMI_node.update_default_qmi_dms_get_current_prl_info_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_CURRENT_PRL_INFO_RESP_V01
  QMI_node.update_default_qmi_dms_get_current_prl_info_resp_v01();
  QMI_node.set_qmi_dms_get_current_prl_info_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  t.set_end();

  if (boot_config == PDC_BOOT)
  {
    util_ril_utf_init_hw_mbn(t, 1);

    util_ril_utf_qmi_pdc_disable_modem_update(t);

    util_ril_utf_init_sw_mbn(t, 0);

    util_ril_utf_qmi_pdc_get_selected_config(t, NULL, (const uint8_t*)"pending_config_id_0", PDC_CONFIG_TYPE_MODEM_SW_V01, 0);

    util_ril_utf_qmi_pdc_activate_config(t, PDC_CONFIG_TYPE_MODEM_SW_V01, 0, 1);

    util_ril_utf_qmi_pdc_cleanup_config(t, PDC_CONFIG_TYPE_MODEM_SW_V01, 0);
  }

  if ((boot_config == DEFAULT_BOOT) || (boot_config == PDC_BOOT))
  {
  // Start the ims threads for ims
//  t.add_func_exec(utf_start_ims_threads);

//  // Expect: ims_MsgId_UNSOL_RADIO_STATE_CHANGED
//  IMS_node.update_default_ims_unsol_radio_state_changed(ims_RadioState_RADIO_STATE_OFF);
//  t.expect(IMS_node);
  }

  // Inject: RIL_REQUEST_RADIO_POWER
  RIL_node.update_default_ril_request_radio_power();
  t.inject(RIL_node);

  t.set_start();

  /*
  if (boot_config != SSR_BOOT)
  {
    // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
    QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
    t.expect(QMI_node);

    // Inject: QMI_DMS_GET_OPERATING_MODE_RESP_V01
    QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
    QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_LOW_POWER_V01);
    QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);
  }
  */

  // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
  QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_LOW_POWER_V01);
  QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_SET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_set_operating_mod_req_v01(DMS_OP_MODE_ONLINE_V01);
  t.expect(QMI_node);

  // Inject: QMI_DMS_SET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_set_operating_mode_resp_v01();
  QMI_node.set_qmi_dms_set_operating_mode_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject: QMI_DMS_EVENT_REPORT_IND_V01
  QMI_node.update_default_qmi_dms_event_report_ind_v01();
  QMI_node.set_qmi_dms_event_report_ind_v01_operating_mode(DMS_OP_MODE_ONLINE_V01);
  t.inject(QMI_node);

  // Expect: RIL_UNSOL_SIGNAL_STRENGTH
  RIL_node.update_default_ril_unsol_signal_strength();
  t.expect(RIL_node);

  // Expect: QMI_NAS_CONFIG_SIG_INFO2_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_config_sig_info2_req_msg_v01();
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_cdma_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_cdma_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_gsm_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_sinr_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rsrp_delta(60);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rsrq_delta(20);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_snr_delta(40);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_ecio_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_rscp_delta(40);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_rssi_delta(5);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_sinr_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_wcdma_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_wcdma_rssi_delta(50);
  t.expect(QMI_node);

  // Inject: QMI_NAS_CONFIG_SIG_INFO2_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_config_sig_info2_resp_msg_v01();
  QMI_node.set_qmi_nas_config_sig_info2_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_SET_EVENT_REPORT_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_set_event_report_req_msg_v01();
  QMI_node.set_qmi_nas_set_event_report_req_report_reg_reject(TRUE);
  t.expect(QMI_node);

  // Inject: QMI_NAS_SET_EVENT_REPORT_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_set_event_report_resp_msg_v01();
  QMI_node.set_qmi_nas_set_event_report_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED
  RIL_node.update_default_ril_unsol_resp_radio_state_changed();
  t.expect(RIL_node);

//  // Expect: ims_MsgId_UNSOL_RADIO_STATE_CHANGED
//  IMS_node.update_default_ims_unsol_radio_state_changed(ims_RadioState_RADIO_STATE_ON);
//  t.expect(IMS_node);

  // Expect: QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_indication_register_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_INDICATION_REGISTER_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_indication_register_resp_msg_v01();
  QMI_node.set_qmi_nas_nas_indication_register_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_config_plmn_name_ind_reporting_req_msg_v01();
  QMI_node.set_qmi_nas_config_plmn_name_ind_reporting_req_msg_send_all_information(1);
  t.expect(QMI_node);

  // Inject: QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_config_plmn_name_ind_reporting_resp_msg_v01();
  QMI_node.set_qmi_nas_config_plmn_name_ind_reporting_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: RIL_REQUEST_RADIO_POWER (resp)
  RIL_node.update_default_ril_request_radio_power_resp();
  t.expect(RIL_node);

  // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
  nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
  // Set values for data
  ptr->gsm_srv_status_info_valid = TRUE;
  ptr->gsm_srv_status_info.srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
  ptr->gsm_srv_status_info.true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
  ptr->gsm_srv_status_info.is_pref_data_path = 0;
  ptr->wcdma_srv_status_info_valid = TRUE;
  ptr->wcdma_srv_status_info.srv_status = NAS_SYS_SRV_STATUS_SRV_V01;
  ptr->wcdma_srv_status_info.true_srv_status = SYS_SRV_STATUS_SRV_V01;
  ptr->wcdma_srv_status_info.is_pref_data_path = 0;
  ptr->wcdma_sys_info.common_sys_info.srv_domain_valid = TRUE;
  ptr->wcdma_sys_info.common_sys_info.srv_domain = SYS_SRV_DOMAIN_CS_PS_V01;
  ptr->wcdma_sys_info.common_sys_info.srv_capability_valid = TRUE;
  ptr->wcdma_sys_info.common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
  ptr->wcdma_sys_info.common_sys_info.roam_status_valid = TRUE;
  ptr->wcdma_sys_info.common_sys_info.roam_status = NAS_SYS_ROAM_STATUS_OFF_V01;
  ptr->wcdma_sys_info.common_sys_info.is_sys_forbidden_valid = TRUE;
  ptr->wcdma_sys_info.common_sys_info.is_sys_forbidden = 0;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.lac_valid = TRUE;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.lac = 43024;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.cell_id_valid = TRUE;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.cell_id = 13548179;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.network_id_valid = TRUE;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.hs_call_status_valid = TRUE;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.hs_call_status = SYS_HS_IND_HSDPA_HSUPA_UNSUPP_CELL_V01;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.hs_ind_valid = TRUE;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.hs_ind = SYS_HS_IND_HSDPA_HSUPA_UNSUPP_CELL_V01;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.psc_valid = TRUE;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.psc = 426;
  ptr->wcdma_cipher_domain_valid = TRUE;
  ptr->wcdma_cipher_domain = SYS_SRV_DOMAIN_CS_ONLY_V01;
  ptr->sim_rej_info_valid = TRUE;
  ptr->sim_rej_info = NAS_SIM_AVAILABLE_V01;

  QMI_node.set_qmi_nas_get_sys_info_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_SIG_INFO_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_sig_info_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_SIG_INFO_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_sig_info_resp_msg_v01();
  QMI_node.set_qmi_nas_get_sig_info_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_ERR_RATE_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_err_rate_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_ERR_RATE_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_err_rate_resp_msg_v01();
  QMI_node.set_qmi_nas_get_err_rate_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_EMBMS_STATUS_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_embms_status_req_msg_v01();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_EMBMS_STATUS_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_embms_status_resp_msg_v01();
  QMI_node.set_qmi_nas_get_embms_status_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_PBM_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_pbm_indication_register_req_msg();
  QMI_node.set_qmi_pbm_indication_register_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01|PBM_REG_RECORD_UPDATE_EVENTS_V01|PBM_REG_PHONEBOOK_READY_EVENTS_V01);
  t.expect(QMI_node);

  // Inject: QMI_PBM_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_pbm_indication_register_resp_msg();
  QMI_node.set_qmi_pbm_indication_register_resp_v01_reg_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01|PBM_REG_RECORD_UPDATE_EVENTS_V01|PBM_REG_PHONEBOOK_READY_EVENTS_V01);
  QMI_node.set_qmi_pbm_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: RIL_UNSOL_VOICE_RADIO_TECH_CHANGED
  RIL_node.update_default_ril_unsol_voice_radio_tech_changed();
  t.expect(RIL_node);

  // Expect: RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_voice_network_state_changed();
  t.expect(RIL_node);

   // Expect: RIL_UNSOL_SIGNAL_STRENGTH
  RIL_node.update_default_ril_unsol_signal_strength();
  t.expect(RIL_node);

  // initialize ims radio
  t.add_func_exec(connect_to_i_ims_radio);

  // Expect: ims_MsgId_UNSOL_RADIO_STATE_CHANGED
  IMS_node.update_default_ims_unsol_radio_state_changed(ims_RadioState_RADIO_STATE_ON);
  t.expect(IMS_node);

  // End unordered expect set
  t.set_end();

}
#endif

test_result qmi_ril_test_bootup_001()
{
  // this part need to be included for all test cases
  // test ID and attributes for each test case are set here

  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  IMSTestnode IMS_node;
  OEMTestnode OEM_node;

  t.set_test_id(__func__);
  t.set_test_description("This test is necessary to boot qmi_ril"
    "test must be run before any other tests are run");
  t.set_test_attribute("ALL");
  ril_utf_set_primary_rild(); // set argument instance id for RIL_init
  // SINCE THIS IS A BOOT TEST, IT SHOULD NOT HAVE A DEFINED BOOT CASE
  // ALL BOOT TESTS NEED TO HAVE THEIR BOOT CASE SET TO NULL
  t.set_boot_case(NULL);

  t.set_start();
  ril_utf_bootup_helper(t, DEFAULT_BOOT);
  t.set_end();

  // Small delay before connecting to IMS hidl to make sure
  // the radio state indication is reported only once
  t.sleep(20);

  // initialize ims radio
  t.add_func_exec(connect_to_i_ims_radio);

  t.set_start();

  // Expect: ims_MsgId_UNSOL_RADIO_STATE_CHANGED
  IMS_node.update_default_ims_unsol_radio_state_changed(ims_RadioState_RADIO_STATE_ON);
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_WFC_ROAMING_MODE_CONFIG_SUPPORT
  IMS_node.update_default_ims_unsol_wfc_roaming_config_support(true);
  t.expect(IMS_node);

  // ----------begin: HANGUP ALL CALLS on BOOT UP/CRASH RESTART cases----------
  // Expect: QMI_VOICE_MANAGE_IP_CALLS_REQ_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_req_msg(VOIP_SUPS_TYPE_END_ALL_CALLS_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_MANAGE_IP_CALLS_RESP_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_DISCONNECTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_WLAN_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_WLAN_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  voice_call_end_reason_type_v02* end = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end[0].call_id = 1;
  end[0].call_end_reason = CALL_END_CAUSE_NORMAL_CALL_CLEARING_V02;
  t.inject(QMI_node);
  // ----------end: HANGUP ALL CALLS on BOOT UP/CRASH RESTART cases----------

  t.set_end();

  // Start oem threads for oem socket
  t.add_func_exec(utf_start_oem_threads);

  // Execute the test case
  t.execute();

  t.print_summary();

  return t.get_test_result();
}

void ril_utf_msim_bootup_helper_1(Testcase &t, int boot_config, int card0_active, int card1_active)
{
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  IMSTestnode IMS_node;
  OEMTestnode OEM_node;
  MSIMTestnode MSIM_node;

  // Here begins the "real" test case
  // Expect: RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED
  RIL_node.update_default_ril_unsol_resp_radio_state_changed();
  t.expect(RIL_node);

  // Expect: QMI_NAS_BIND_SUBSCRIPTION_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_bind_subscription_req_msg(NAS_PRIMARY_SUBSCRIPTION_V01);
  t.expect(QMI_node);

  // Inject: QMI_NAS_BIND_SUBSCRIPTION_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_bind_subscription_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_BIND_SUBSCRIPTION_REQ_V02
  QMI_node.update_default_qmi_voice_bind_subscription_req_msg(VOICE_SUBS_TYPE_PRIMARY_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_BIND_SUBSCRIPTION_RESP_V02
  QMI_node.update_default_qmi_voice_bind_subscription_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_PBM_BIND_SUBSCRIPTION_REQ_V01
  QMI_node.update_default_qmi_pbm_bind_subscription_req_msg(PBM_SUBS_TYPE_PRIMARY_V01);
  t.expect(QMI_node);

  // Inject: QMI_PBM_BIND_SUBSCRIPTION_RESP_V01
  QMI_node.update_default_qmi_pbm_bind_subscription_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_BIND_SUBSCRIPTION_REQ_V01
  QMI_node.update_default_qmi_wms_bind_subscription_req(WMS_PRIMARY_SUBSCRIPTION_V01);
  t.expect(QMI_node);

  // Inject: QMI_WMS_BIND_SUBSCRIPTION_RESP_V01
  QMI_node.update_default_qmi_wms_bind_subscription_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_BIND_SUBSCRIPTION_REQ_V01
  QMI_node.update_default_qmi_dms_bind_subscription_req_msg(DMS_PRIMARY_SUBS_V01);
  t.expect(QMI_node);

  // Inject: QMI_DMS_BIND_SUBSCRIPTION_RESP_V01
  QMI_node.update_default_qmi_dms_bind_subscription_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_LTE_BIND_SUBSCRIPTION_REQ_V01
  QMI_node.update_default_qmi_lte_bind_subscription_req_msg(LTE_PRIMARY_SUBSCRIPTION_V01);
  t.expect(QMI_node);

  // Inject: QMI_LTE_BIND_SUBSCRIPTION_RESP_V01
  QMI_node.update_default_qmi_lte_bind_subscription_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_reg_voice_privacy_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_ext_brst_intl_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_speech_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_handover_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_conference_events(0x01);
  //QMI_node.set_qmi_voice_ind_req_tty_info_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_tty_info_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_cc_result_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_additional_call_info_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_audio_rat_change_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_vice_dialog_event(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_conf_participants_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_INDICATION_REGISTER_REQ_MSG_V01
  QMI_node.update_default_qmi_dms_indication_register_req_v01();
  QMI_node.set_qmi_dms_ind_req_report_ims_capability(0x01);
  t.expect(QMI_node);

  // Inject: QMI_DMS_INDICATION_REGISTER_RESP_MSG_V01
  QMI_node.update_default_qmi_dms_indication_register_resp_v01();
  QMI_node.set_qmi_dms_indication_register_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_SET_EVENT_REPORT_REQ_V01
  QMI_node.update_default_qmi_dms_set_event_report_req_msg();
  QMI_node.set_qmi_dsm_set_event_report_req_report_oprt_mode_state(0x01);
  QMI_node.set_qmi_dsm_set_event_report_req_report_prl_init(0x01);
  QMI_node.set_qmi_dms_set_event_report_req_report_device_multisim_info(0x01);
  t.expect(QMI_node);

  // Inject: QMI_DMS_SET_EVENT_REPORT_RESP_V01
  QMI_node.update_default_qmi_dms_set_event_report_resp_msg();
  QMI_node.set_qmi_dms_set_event_report_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_GET_DEVICE_CAP_REQ_V01
  QMI_node.update_default_qmi_dms_get_device_cap_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_DEVICE_CAP_RESP_V01
  QMI_node.update_default_qmi_dms_get_device_cap_resp_v01();
  dms_multisim_capability_type_v01 multisim_capability;
  memset(&multisim_capability, 0, sizeof(multisim_capability));
  if (boot_config == PDC_BOOT)
  {
    multisim_capability.max_subscriptions = 1;
  }
  else
  {
    multisim_capability.max_subscriptions = 2;
  }
  QMI_node.set_default_qmi_dms_get_device_cap_resp_v01_multisim_capability(multisim_capability);
  QMI_node.set_default_qmi_dms_get_device_cap_resp_v01_device_capabilities();
  QMI_node.set_qmi_dms_get_device_cap_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

#ifdef RIL_UTF_L_MASTER
  t.set_start();

  // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
  t.expect(QMI_node);

  // Inject:QMI_DMS_GET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
  QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_SHUTTING_DOWN_V01);
  QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_QMI_DMS_TRIGGER_MODEM_ACTIVITY_INFO_CALCULATION_REQ_V01
  QMI_node.update_default_qmi_dms_trigger_modem_activity_info_calculation_req(TRUE, 1);
  t.expect(QMI_node);

  // Inject: QMI_DMS_QMI_DMS_TRIGGER_MODEM_ACTIVITY_INFO_CALCULATION_RESP_V01
  QMI_node.update_default_qmi_dms_trigger_modem_activity_info_calculation_resp();
  t.inject(QMI_node);
  t.set_end();
#endif

  /*
  // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
  t.expect(QMI_node);

  // Inject:QMI_DMS_GET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
  QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_SHUTTING_DOWN_V01);
  QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);
  */

  // Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_V01
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_emergency_mode(0);
  mode_pref_mask_type_v01 mode_pref1 = QMI_NAS_RAT_MODE_PREF_UMTS_V01;
  mode_pref1 |= QMI_NAS_RAT_MODE_PREF_GSM_V01;
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(mode_pref1);
  // Set band_pref
  nas_band_pref_mask_type_v01 band_pref;
  band_pref |= QMI_NAS_BAND_CLASS_0_A_SYSTEM_V01;
  band_pref |= QMI_NAS_BAND_CLASS_0_B_AB_GSM850_V01;
  band_pref |= QMI_NAS_BAND_CLASS_1_ALL_BLOCKS_V01;
  band_pref |= QMI_NAS_BAND_CLASS_2_PLACEHOLDER_V01;
  band_pref |= QMI_NAS_BAND_CLASS_3_A_SYSTEM_V01;
  band_pref |= QMI_NAS_BAND_CLASS_4_ALL_BLOCKS_V01;
  band_pref |= QMI_NAS_BAND_CLASS_5_ALL_BLOCKS_V01;
  band_pref |= QMI_NAS_GSM_DCS_1800_BAND_V01;
  band_pref |= QMI_NAS_E_GSM_900_BAND_V01;
  band_pref |= QMI_NAS_P_GSM_900_BAND_V01;
  band_pref |= QMI_NAS_BAND_CLASS_6_V01;
  band_pref |= QMI_NAS_BAND_CLASS_7_V01;
  band_pref |= QMI_NAS_BAND_CLASS_8_V01;
  band_pref |= QMI_NAS_BAND_CLASS_9_V01;
  band_pref |= QMI_NAS_BAND_CLASS_10_V01;
  band_pref |= QMI_NAS_BAND_CLASS_11_V01;
  band_pref |= QMI_NAS_GSM_BAND_450_V01;
  band_pref |= QMI_NAS_GSM_BAND_480_V01;
  band_pref |= QMI_NAS_GSM_BAND_750_V01;
  band_pref |= QMI_NAS_GSM_BAND_850_V01;
  band_pref |= QMI_NAS_GSM_BAND_RAILWAYS_900_BAND_V01;
  band_pref |= QMI_NAS_GSM_BAND_PCS_1900_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_EU_J_CH_IMT_2100_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_US_PCS_1900_BAND_V01;
  band_pref |= QMI_NAS_EU_CH_DCS_1800_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_US_1700_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_JAPAN_800_BAND_V01;
  band_pref |= QMI_NAS_BAND_CLASS_12_V01;
  band_pref |= QMI_NAS_BAND_CLASS_14_V01;
  band_pref |= QMI_NAS_RESERVED_V01;
  band_pref |= QMI_NAS_BAND_CLASS_15_V01;
  band_pref |= QMI_NAS_WCDMA_EU_2600_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_EU_J_900_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_J_1700_BAND_V01;
  band_pref |= QMI_NAS_BAND_CLASS_16_V01;
  band_pref |= QMI_NAS_BAND_CLASS_17_V01;
  band_pref |= QMI_NAS_BAND_CLASS_18_V01;
  band_pref |= QMI_NAS_BAND_CLASS_19_V01;
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_band_pref(band_pref);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_prl_pref(NAS_PRL_PREF_ANY_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_roam_pref(NAS_ROAMING_PREF_ANY_V01);
  // Set band_pref_ext
  uint64_t band_pref_ext;
  band_pref_ext |= E_UTRA_OPERATING_BAND_1_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_2_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_3_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_4_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_5_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_6_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_7_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_8_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_9_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_10_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_11_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_12_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_13_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_14_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_17_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_18_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_19_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_20_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_21_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_24_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_25_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_33_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_34_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_35_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_36_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_37_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_38_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_39_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_40_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_41_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_42_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_43_V01;
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_band_pref_ext(band_pref_ext);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_net_sel_pref(NAS_NET_SEL_PREF_AUTOMATIC_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_PS_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_gw_acq_order_pref(NAS_GW_ACQ_ORDER_PREF_AUTOMATIC_V01);
  // Set tdscdma_band_pref
  nas_tdscdma_band_pref_mask_type_v01 tdscdma_band_pref;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_A_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_B_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_C_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_D_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_E_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_F_V01;
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_tdscdma_band_pref(tdscdma_band_pref);
  QMI_node.set_default_qmi_nas_get_system_selection_preference_resp_acq_order();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_reg_restriction(NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_usage_setting(NAS_USAGE_VOICE_CENTRIC_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_CONFIG_SIG_INFO2_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_config_sig_info2_req_msg_v01();
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_cdma_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_cdma_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_gsm_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_sinr_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rsrp_delta(60);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rsrq_delta(20);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_snr_delta(40);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_ecio_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_rscp_delta(40);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_rssi_delta(5);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_sinr_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_wcdma_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_wcdma_rssi_delta(50);
  t.expect(QMI_node);

  // Inject: QMI_NAS_CONFIG_SIG_INFO2_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_config_sig_info2_resp_msg_v01();
  QMI_node.set_qmi_nas_config_sig_info2_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);


  // Expect: QMI_NAS_SET_EVENT_REPORT_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_set_event_report_req_msg_v01();
  QMI_node.set_qmi_nas_set_event_report_req_report_reg_reject(TRUE);
  t.expect(QMI_node);

  // Inject: QMI_NAS_SET_EVENT_REPORT_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_set_event_report_resp_msg_v01();
  QMI_node.set_qmi_nas_set_event_report_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_indication_register_req_msg_v01();
  QMI_node.set_qmi_nas_indication_register_req_msg_reg_sys_sel_pref(1);
  QMI_node.set_qmi_nas_indication_register_req_msg_dual_standby_pref(1);
  QMI_node.set_qmi_nas_indication_register_req_msg_subscription_info(1);
  QMI_node.set_qmi_nas_indication_register_reg_msg_reg_rtre_cfg(1);
  QMI_node.set_qmi_nas_indication_register_req_msg_req_serving_system(0);
  QMI_node.set_qmi_nas_indication_register_req_msg_reg_network_time(0);
  QMI_node.set_qmi_nas_indication_register_reg_msg_reg_embms_status(0);
  QMI_node.set_qmi_nas_indication_register_req_msg_sig_info(0);
  QMI_node.set_qmi_nas_indication_register_reg_msg_err_rate(0);
  QMI_node.set_qmi_nas_indication_register_req_msg_reg_current_plmn_name(0);
  QMI_node.set_qmi_nas_indication_register_req_msg_reg_rf_band_info(0);
  t.expect(QMI_node);

  // Inject: QMI_NAS_INDICATION_REGISTER_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_indication_register_resp_msg_v01();
  QMI_node.set_qmi_nas_nas_indication_register_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

    // Expect: QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_config_plmn_name_ind_reporting_req_msg_v01();
  QMI_node.set_qmi_nas_config_plmn_name_ind_reporting_req_msg_send_all_information(1);
  t.expect(QMI_node);

  // Inject: QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_config_plmn_name_ind_reporting_resp_msg_v01();
  QMI_node.set_qmi_nas_config_plmn_name_ind_reporting_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_CENTRALIZED_EONS_SUPPORT_STATUS_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_centralized_eons_support_status_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_CENTRALIZED_EONS_SUPPORT_STATUS_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_centralized_eons_support_status_resp_msg_v01();
  QMI_node.set_qmi_nas_get_centralized_eons_support_status_resp_msg_centralized_eons_supported(1);
  QMI_node.set_qmi_nas_get_centralized_eons_support_status_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_LTE_SIB16_NETWORK_TIME_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_lte_sib16_network_time_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_LTE_SIB16_NETWORK_TIME_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_lte_sib16_network_time_resp_msg_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
  QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_SHUTTING_DOWN_V01);
  QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_PBM_INDICATON_REGISTER_REQ_V01
  QMI_node.update_default_qmi_pbm_indication_register_req_msg();
  QMI_node.set_qmi_pbm_indication_register_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01);
  t.expect(QMI_node);

  // Inject: QMI_PBM_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_pbm_indication_register_resp_msg();
  QMI_node.set_qmi_pbm_indication_register_resp_v01_reg_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01);
  QMI_node.set_qmi_pbm_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_PBM_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_pbm_indication_register_req_msg();
  QMI_node.set_qmi_pbm_indication_register_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01|PBM_REG_RECORD_UPDATE_EVENTS_V01|PBM_REG_PHONEBOOK_READY_EVENTS_V01);
  t.expect(QMI_node);

  // Inject: QMI_PBM_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_pbm_indication_register_resp_msg();
  QMI_node.set_qmi_pbm_indication_register_resp_v01_reg_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01|PBM_REG_RECORD_UPDATE_EVENTS_V01|PBM_REG_PHONEBOOK_READY_EVENTS_V01);
  QMI_node.set_qmi_pbm_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_LTE_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_lte_indication_register_req_msg();
  QMI_node.set_qmi_lte_indication_register_req_indication_bitmask(
          QMI_LTE_DISC_NOTIFICATION_IND_MASK_V01 |
          QMI_LTE_DISC_BROADCAST_NOTIFICATION_IND_MASK_V01 |
          QMI_LTE_DISC_MATCH_NOTIFICATION_IND_MASK_V01 |
          QMI_LTE_DISC_PSK_EXPIRED_IND_MASK_V01);
  t.expect(QMI_node);

  // Inject: QMI_LTE_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_lte_indication_register_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Start set of unordered expects
  t.set_start();

  // Expect: QMI_IMS_SETTINGS_GET_SUPPORTED_MSGS_REQ_V01
  QMI_node.update_default_qmi_ims_settings_get_supported_msgs_req();
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_GET_SUPPORTED_MSGS_RESP_V01
  QMI_node.update_default_qmi_ims_settings_get_supported_msgs_resp();
  t.inject(QMI_node);

  // Expect: QMI_UIM_EVENT_REG_REQ_V01
  QMI_node.update_default_qmi_uim_event_reg_req(0x07);
  t.expect(QMI_node);

  // Inject: QMI_UIM_EVENT_REG_RESP_V01
  QMI_node.update_default_qmi_uim_event_reg_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_GET_CARD_STATUS_REQ_V01
  QMI_node.update_default_qmi_uim_get_card_status_req();
  QMI_node.set_qmi_uim_get_card_status_req_extended_card_status(0x01);
  t.expect(QMI_node);

  // Inject: QMI_UIM_GET_CARD_STATUS_RESP_V01
  uim_get_card_status_resp_msg_v01* status = QMI_node.update_default_qmi_uim_get_card_status_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  status->extended_card_status_valid = TRUE;
  status->extended_card_status.card_info_len = 2;
  if (card0_active && card1_active)
  {
    status->extended_card_status.index_gw[0] = 0x0;
    status->extended_card_status.index_gw[1] = 0x100;
    status->extended_card_status.index_gw_len = 2;
    status->extended_card_status.index_1x_len = 0;
  }
  else if (card0_active || card1_active)
  {
    status->extended_card_status.index_gw[0] = 0x0;
    status->extended_card_status.index_gw_len = 1;
    status->extended_card_status.index_1x_len = 0;
  }
  else if (card1_active)
  {
    status->extended_card_status.index_gw[0] = 0x100;
    status->extended_card_status.index_gw_len = 1;
    status->extended_card_status.index_1x_len = 0;
  }

  uint8_t aid_value_1[16] = {0xA0,0x00,0x00,0x00,0x87,0x10,0x02,0xF3,0x10,0xFF,0xFF,0x89,0x08,0x00,0x00,0xFF};
  if (card0_active)
  {
    // Card 0
    status->extended_card_status.card_info[0].card_state = UIM_EXTENDED_CARD_STATE_PRESENT_V01;
    status->extended_card_status.card_info[0].upin.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
    status->extended_card_status.card_info[0].upin.pin_retries = 0x00;
    status->extended_card_status.card_info[0].upin.puk_retries = 0x00;
    status->extended_card_status.card_info[0].app_info_len = 0x01;
    // Card 0 App 0
    status->extended_card_status.card_info[0].app_info[0].app_type = UIM_APP_TYPE_USIM_V01;
    status->extended_card_status.card_info[0].app_info[0].app_state = UIM_APP_STATE_READY_V01;
    status->extended_card_status.card_info[0].app_info[0].perso_state = UIM_PERSO_STATE_READY_V01;
    status->extended_card_status.card_info[0].app_info[0].perso_feature = UIM_PERSO_FEATURE_STATUS_UNKNOWN_V01;
    status->extended_card_status.card_info[0].app_info[0].perso_retries = 0x00;
    status->extended_card_status.card_info[0].app_info[0].perso_unblock_retries = 0x00;
    status->extended_card_status.card_info[0].app_info[0].aid_value_len = 0x10;
    memcpy(status->extended_card_status.card_info[0].app_info[0].aid_value, aid_value_1, 16);
    status->extended_card_status.card_info[0].app_info[0].univ_pin = UIM_UNIV_PIN_PIN1_USED_V01;
    status->extended_card_status.card_info[0].app_info[0].pin1.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
    status->extended_card_status.card_info[0].app_info[0].pin1.pin_retries = 0x00;
    status->extended_card_status.card_info[0].app_info[0].pin1.puk_retries = 0x00;
    status->extended_card_status.card_info[0].app_info[0].pin2.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
    status->extended_card_status.card_info[0].app_info[0].pin2.pin_retries = 0x00;
    status->extended_card_status.card_info[0].app_info[0].pin2.puk_retries = 0x00;
  }
  else
  {
    status->extended_card_status.card_info[0].card_state = UIM_EXTENDED_CARD_STATE_ABSENT_V01;
  }

  if (card1_active)
  {
    // Card 1
    status->extended_card_status.card_info[1].card_state = UIM_EXTENDED_CARD_STATE_PRESENT_V01;
    status->extended_card_status.card_info[1].upin.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
    status->extended_card_status.card_info[1].upin.pin_retries = 0x00;
    status->extended_card_status.card_info[1].upin.puk_retries = 0x00;
    status->extended_card_status.card_info[1].app_info_len = 0x01;
    // Card 1 App 0
    status->extended_card_status.card_info[1].app_info[0].app_type = UIM_APP_TYPE_USIM_V01;
    status->extended_card_status.card_info[1].app_info[0].app_state = UIM_APP_STATE_READY_V01;
    status->extended_card_status.card_info[1].app_info[0].perso_state = UIM_PERSO_STATE_READY_V01;
    status->extended_card_status.card_info[1].app_info[0].perso_feature = UIM_PERSO_FEATURE_STATUS_UNKNOWN_V01;
    status->extended_card_status.card_info[1].app_info[0].perso_retries = 0x00;
    status->extended_card_status.card_info[1].app_info[0].perso_unblock_retries = 0x00;
    status->extended_card_status.card_info[1].app_info[0].aid_value_len = 0x10;
    memcpy(status->extended_card_status.card_info[1].app_info[0].aid_value, aid_value_1, 16);
    status->extended_card_status.card_info[1].app_info[0].univ_pin = UIM_UNIV_PIN_PIN1_USED_V01;
    status->extended_card_status.card_info[1].app_info[0].pin1.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
    status->extended_card_status.card_info[1].app_info[0].pin1.pin_retries = 0x00;
    status->extended_card_status.card_info[1].app_info[0].pin1.puk_retries = 0x00;
    status->extended_card_status.card_info[1].app_info[0].pin2.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
    status->extended_card_status.card_info[1].app_info[0].pin2.pin_retries = 0x00;
    status->extended_card_status.card_info[1].app_info[0].pin2.puk_retries = 0x00;
  }
  else
  {
    status->extended_card_status.card_info[1].card_state = UIM_EXTENDED_CARD_STATE_ABSENT_V01;
  }
  t.inject(QMI_node);

  // Expect: QMI_CAT_GET_CONFIGURATION_REQ_V02
  QMI_node.update_default_qmi_cat_get_configuration_req();
  t.expect(QMI_node);

  // Inject: QMI_CAT_GET_CONFIGURATION_RESP_V02
  QMI_node.update_default_qmi_cat_get_configuration_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_CAT_SET_EVENT_REPORT_REQ_V02
  QMI_node.update_default_qmi_cat_set_event_report_req();
#ifdef RIL_UTF_L_MASTER
  QMI_node.set_qmi_cat_set_event_report_req_pc_evt_report_req_mask(0x0b7FFFFF);
#else
  QMI_node.set_qmi_cat_set_event_report_req_pc_evt_report_req_mask(0x037FFFFF);
#endif
  QMI_node.set_qmi_cat_set_event_report_req_slot_mask(CAT_SET_EVENT_REPORT_SLOT_1_V02);
  t.expect(QMI_node);

  // Inject: QMI_CAT_SET_EVENT_REPORT_RESP_V02
  QMI_node.update_default_qmi_cat_set_event_report_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_CAT_SET_EVENT_REPORT_REQ_V02
  QMI_node.update_default_qmi_cat_set_event_report_req();
  QMI_node.set_qmi_cat_set_event_report_req_pc_dec_evt_report_req_mask(0x00800000);
  QMI_node.set_qmi_cat_set_event_report_req_slot_mask(CAT_SET_EVENT_REPORT_SLOT_1_V02);
  t.expect(QMI_node);

  // Inject: QMI_CAT_SET_EVENT_REPORT_RESP_V02
  QMI_node.update_default_qmi_cat_set_event_report_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_IMSP_SET_EVENT_REPORT_REQ_V01
  QMI_node.update_default_qmi_imsp_set_event_report_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_IMSP_SET_EVENT_REPORT_RESP_V01
  QMI_node.update_default_qmi_imsp_set_event_report_resp_v01();
  QMI_node.set_qmi_imsp_set_event_report_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_IMS_SETTINGS_GET_REG_MGR_CONFIG_REQ_V01
  QMI_node.update_default_qmi_ims_settings_get_reg_mgr_config_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_GET_REG_MGR_CONFIG_RSP_V01
  QMI_node.update_default_qmi_ims_settings_get_reg_mgr_config_rsp_v01();
  QMI_node.set_qmi_ims_settings_get_reg_mgr_config_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_REQ_V01
  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_RSP_V01
  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_rsp_v01();
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_IMSA_IND_REG_REQ_V01
  QMI_node.update_default_qmi_imsa_ind_reg_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_IMSA_IND_REG_RSP_V01
  QMI_node.update_default_qmi_imsa_ind_reg_rsp_v01();
  QMI_node.set_qmi_imsa_ind_req_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_IMS_SETTINGS_CONFIG_IND_REG_REQ_V01
  QMI_node.update_default_qmi_ims_config_ind_reg_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_CONFIG_IND_REG_RSP_V01
  QMI_node.update_default_qmi_ims_config_ind_reg_rsp_v01();
  QMI_node.set_qmi_ims_config_ind_req_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_GET_DEVICE_CAP_REQ_V01
  QMI_node.update_default_qmi_dms_get_device_cap_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_DEVICE_CAP_RESP_V01
  QMI_node.update_default_qmi_dms_get_device_cap_resp_v01();
  memset(&multisim_capability, 0, sizeof(multisim_capability));
  if (boot_config == PDC_BOOT)
  {
    multisim_capability.max_subscriptions = 1;
  }
  else
  {
    multisim_capability.max_subscriptions = 2;
  }
  QMI_node.set_default_qmi_dms_get_device_cap_resp_v01_multisim_capability(multisim_capability);
  QMI_node.set_default_qmi_dms_get_device_cap_resp_v01_device_capabilities();
  QMI_node.set_qmi_dms_get_device_cap_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  //QMI_NAS_GET_SUBSCRIPTION_INFO_REQ_MSG_V01
  util_ril_utf_get_subscription_info(t, QMI_node, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, TRUE, NAS_PRIORITY_SUBSCRIPTION_TRUE_V01, TRUE, NAS_SUBSCRIPTION_ACTIVE_V01, TRUE, 0, TRUE, 0, FALSE, 0, FALSE, 0);

  // Expect: QMI_DMS_GET_CURRENT_PRL_INFO_REQ_V01
  QMI_node.update_default_qmi_dms_get_current_prl_info_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_CURRENT_PRL_INFO_RESP_V01
  QMI_node.update_default_qmi_dms_get_current_prl_info_resp_v01();
  QMI_node.set_qmi_dms_get_current_prl_info_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED
  RIL_node.update_default_ril_unsol_resp_radio_state_changed();
  t.expect(RIL_node);

  // Expect: QMI_DMS_GET_DEVICE_MFR_REQ_V01
  QMI_node.update_default_qmi_dms_get_device_mfr_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_DEVICE_MFR_RESP_V01
  QMI_node.update_default_qmi_dms_get_device_mfr_resp_v01();
  QMI_node.set_default_qmi_dms_get_device_mfr_resp_v01_device_manufacturer();
  QMI_node.set_qmi_dms_get_device_mfr_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ_V01
  QMI_node.update_default_qmi_wms_get_transport_layer_info_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_WMS_GET_TRANSPORT_LAYER_INFO_RESP_V01
  QMI_node.update_default_qmi_wms_get_transport_layer_info_resp_v01();
  QMI_node.set_qmi_wms_get_transport_layer_info_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_SET_ROUTES_REQ_V01
  QMI_node.update_default_qmi_wms_set_routes_req_v01();
//TODO:  Add sane values here and enable payload validation
  t.expect(QMI_node);

  // Inject: QMI_WMS_SET_ROUTES_RESP_V01
  QMI_node.update_default_qmi_wms_set_routes_resp_v01();
  QMI_node.set_qmi_wms_set_routes_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_SET_EVENT_REPORT_REQ_V01
  QMI_node.update_default_qmi_wms_set_event_report_req_v01();
//TODO:  Add sane values here and enable payload validation
  t.expect(QMI_node);

  // Inject: QMI_WMS_SET_EVENT_REPORT_RESP_V01
  QMI_node.update_default_qmi_wms_set_event_report_resp_v01();
  QMI_node.set_qmi_wms_set_event_report_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_wms_indication_register_req_v01();
  QMI_node.set_qmi_wms_indication_register_req_v01_reg_transport_layer_info_events(1);
  QMI_node.set_qmi_wms_indication_register_req_v01_reg_transport_layer_mwi_info_events(1);
  t.expect(QMI_node);

  // Inject: QMI_WMS_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_wms_indication_register_resp_v01();
  QMI_node.set_qmi_wms_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_GET_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_wms_get_indication_register_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_WMS_GET_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_wms_get_indication_register_resp_v01();
  QMI_node.set_qmi_wms_get_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  //TODO: Fill with sane values to send to qmi-ril
  t.inject(QMI_node);
  // Expect: QMI_WMS_SET_PRIMARY_CLIENT_REQ_V01
  QMI_node.update_default_qmi_wms_set_primary_client_req_v01();
  QMI_node.set_qmi_wms_set_primary_client_req_v01_primary_client(1);
  t.expect(QMI_node);

  // Inject: QMI_WMS_SET_PRIMARY_CLIENT_RESP_V01
  QMI_node.update_default_qmi_wms_set_primary_client_resp_v01();
  QMI_node.set_qmi_wms_set_primary_client_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_GET_CURRENT_PRL_INFO_REQ_V01
  QMI_node.update_default_qmi_dms_get_current_prl_info_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_CURRENT_PRL_INFO_RESP_V01
  QMI_node.update_default_qmi_dms_get_current_prl_info_resp_v01();
  QMI_node.set_qmi_dms_get_current_prl_info_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  if (card0_active || card1_active)
  {
    if (card0_active)
    {
        // Expect: QMI_UIM_READ_TRANSPARENT_REQ_V01
        QMI_node.update_default_qmi_uim_read_transparent_reg();
        QMI_node.set_expect_skip_verification_status(false);
        t.expect(QMI_node);

        // Inject: QMI_UIM_READ_TRANSPARENT_RESP_V01
        QMI_node.update_default_qmi_uim_read_transparent_resp(QMI_RESULT_SUCCESS_V01,QMI_ERR_NONE_V01);
        t.inject(QMI_node);

    }

#ifdef RIL_UTF_L_MASTER
    if (boot_config == PDC_BOOT)
    {
      if (card0_active)
      {
        /*
        // Expect: QMI_UIM_READ_TRANSPARENT_REQ_V01
        uim_read_transparent_req_msg_v01* rt_req_ptr = QMI_node.update_default_qmi_uim_read_transparent_reg();
        rt_req_ptr->session_information.session_type = UIM_SESSION_TYPE_CARD_ON_SLOT_1_V01;
        rt_req_ptr->file_id.file_id = 0x2FE2;
        rt_req_ptr->file_id.path_len = 0x02;
        rt_req_ptr->file_id.path[0] = 0x00;
        rt_req_ptr->file_id.path[1] = 0x3F;
        t.expect(QMI_node);

        // Inject: QMI_UIM_READ_TRANSPARENT_RESP_V01
        uim_read_transparent_resp_msg_v01* rt_resp_ptr = QMI_node.update_default_qmi_uim_read_transparent_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
        rt_resp_ptr->read_result_valid = 1;
        rt_resp_ptr->read_result.content_len = 10;
        memcpy(rt_resp_ptr->read_result.content, tmp_uim_id, 10);
        t.inject(QMI_node);

        // Expect: QMI_DMS_UIM_GET_ICCID_REQ_V01
        QMI_node.update_default_qmi_dms_get_iccid_req();
        t.expect(QMI_node);

        // Inject: QMI_DMS_UIM_GET_ICCID_RESP_V01
        uim_id = QMI_node.update_default_qmi_dms_get_iccid_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
        memcpy(uim_id, tmp_uim_id, 10);
        t.inject(QMI_node);
        */
      }
      else
      {
        /*
        // Expect: QMI_UIM_READ_TRANSPARENT_REQ_V01
        QMI_node.update_default_qmi_uim_read_transparent_reg();
        QMI_node.set_expect_skip_verification_status(false);
        t.expect(QMI_node);

        // Inject: QMI_UIM_READ_TRANSPARENT_RESP_V01
        QMI_node.update_default_qmi_uim_read_transparent_resp(QMI_RESULT_FAILURE_V01, QMI_ERR_GENERAL_V01);
        t.inject(QMI_node);
        */
      }
    }
    else
    {
      /*
      // Expect: QMI_UIM_READ_TRANSPARENT_REQ_V01
      QMI_node.update_default_qmi_uim_read_transparent_reg();
      QMI_node.set_expect_skip_verification_status(false);
      t.expect(QMI_node);

      // Inject: QMI_UIM_READ_TRANSPARENT_RESP_V01
      QMI_node.update_default_qmi_uim_read_transparent_resp(QMI_RESULT_FAILURE_V01, QMI_ERR_GENERAL_V01);
      t.inject(QMI_node);

      if (card0_active && card1_active)
      {
        // Expect: QMI_UIM_READ_TRANSPARENT_REQ_V01
        QMI_node.update_default_qmi_uim_read_transparent_reg();
        QMI_node.set_expect_skip_verification_status(false);
        t.expect(QMI_node);

        // Inject: QMI_UIM_READ_TRANSPARENT_RESP_V01
        QMI_node.update_default_qmi_uim_read_transparent_resp(QMI_RESULT_FAILURE_V01, QMI_ERR_GENERAL_V01);
        t.inject(QMI_node);

        // Expect: QMI_DMS_UIM_GET_ICCID_REQ_V01
        QMI_node.update_default_qmi_dms_get_iccid_req();
        t.expect(QMI_node);

        // Inject: QMI_DMS_UIM_GET_ICCID_RESP_V01
        uim_id = QMI_node.update_default_qmi_dms_get_iccid_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
        memcpy(uim_id, tmp_uim_id, 10);
        t.inject(QMI_node);
      }

      if (card0_active)
      {
        // Expect: QMI_DMS_UIM_GET_ICCID_REQ_V01
        QMI_node.update_default_qmi_dms_get_iccid_req();
        t.expect(QMI_node);

        // Inject: QMI_DMS_UIM_GET_ICCID_RESP_V01
        uim_id = QMI_node.update_default_qmi_dms_get_iccid_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
        memcpy(uim_id, tmp_uim_id, 10);
        t.inject(QMI_node);
      }
      */
    }
#endif

    uim_session_information_type_v01 session_info;
    if (card0_active)
    {
      // Expect: QMI_UIM_GET_SERVICE_STATUS_REQ_V01
      memset(&session_info, 0, sizeof(session_info));
      session_info.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
      session_info.aid_len = 0;
      QMI_node.update_default_qmi_uim_get_service_status_req(session_info, 0x01);
      t.expect(QMI_node);

      // Inject: QMI_UIM_GET_SERVICE_STATUS_RESP_V01
      QMI_node.update_default_qmi_uim_get_service_status_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
      QMI_node.set_qmi_uim_get_service_status_resp_fdn_status(UIM_FDN_STATUS_AVAILABLE_DISABLED_V01);
      t.inject(QMI_node);
    }
  }

  // Expect: QMI_DMS_SET_AP_SW_VERSION_REQ_V01
  QMI_node.update_default_qmi_dms_set_ap_sw_version_req_v01();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_DMS_SET_AP_SW_VERSION_RESP_V01
  QMI_node.update_default_qmi_dms_set_ap_sw_version_resp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  if (card0_active || card1_active)
  {
    if (card0_active)
    {
      uim_session_information_type_v01 session_info;
      // Expect: QMI_UIM_GET_FILE_ATTRIBUTES_REQ_V01
      memset(&session_info, 0, sizeof(session_info));
      session_info.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
      session_info.aid_len = 0x00;
      uim_file_id_type_v01 file_id;
      memset(&file_id, 0, sizeof(file_id));
      uint8_t path[10] = {0x00,0x3F,0x10,0x7F,0x3A,0x5F};
      file_id.file_id = 0x4f30;
      file_id.path_len = 0x06;
      memcpy(file_id.path, path, 0x06);
      QMI_node.update_default_qmi_uim_get_file_attributes_req(session_info, file_id);
      t.expect(QMI_node);

      // Inject: QMI_UIM_GET_FILE_ATTRIBUTES_RESP_V01
      QMI_node.update_default_qmi_uim_get_file_attributes_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
      uim_card_result_type_v01 card_result;
      card_result.sw1 = 0x90;
      card_result.sw2 = 0x00;
      QMI_node.set_qmi_uim_get_file_attributes_resp_card_result(card_result);
      QMI_node.set_qmi_uim_get_file_attribues_resp_file_status(UIM_FILE_STATUS_ACTIVATED_V01);
      uim_file_attributes_type_v01* file_attributes = QMI_node.set_qmi_uim_get_file_attributes_resp_file_attributes();
      file_attributes->file_size = 0x4c;
      file_attributes->file_id = 0x4f30;
      file_attributes->file_type = UIM_FILE_TYPE_LINEAR_FIXED_V01;
      file_attributes->rec_size = 0x4c;
      file_attributes->rec_count = 0x01;
      file_attributes->sec_read = UIM_SEC_CONDITION_SINGLE_COND_V01;
      file_attributes->sec_read_mask = 0x01;
      file_attributes->sec_write = UIM_SEC_CONDITION_SINGLE_COND_V01;
      file_attributes->sec_write_mask = 0x08;
      file_attributes->sec_increase = UIM_SEC_CONDITION_ALWAYS_V01;
      file_attributes->sec_increase_mask = 0x00;
      file_attributes->sec_deactivate = UIM_SEC_CONDITION_SINGLE_COND_V01;
      file_attributes->sec_deactivate_mask = 0x08;
      file_attributes->sec_activate = UIM_SEC_CONDITION_SINGLE_COND_V01;
      file_attributes->sec_activate_mask = 0x08;
      file_attributes->raw_value_len = 0x24;
      uint8_t temp_raw_value[40] = {0x62,0x22,0x82,0x05,0x42,0x21,0x00,0x4C,0x01,0x83,0x02,0x4F,0x30,0xA5,0x03,
        0xC0,0x01,0x00,0x8A,0x01,0x05,0x8B,0x03,0x6F,0x06,0x03,0x80,0x02,0x00,0x4C,0x81,0x02,0x00,0x60,0x88,0x00};
      memcpy(file_attributes->raw_value, temp_raw_value, 36);
      t.inject(QMI_node);

      // Expect: QMI_UIM_READ_RECORD_REQ_V01
      uim_read_record_req_msg_v01* read_req = QMI_node.update_default_qmi_uim_read_record_reg();
      read_req->session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
      read_req->session_information.aid_len = 0x00;
      read_req->file_id.file_id = 0x4f30;
      read_req->file_id.path_len = 0x06;
      uint8_t tmp_path[] = {0x00,0x3F,0x10,0x7F,0x3A,0x5F};
      memcpy(read_req->file_id.path, tmp_path, 0x06);
      read_req->read_record.record = 0x01;
      read_req->read_record.length = 0x4c;
      t.expect(QMI_node);

      // Inject: QMI_UIM_READ_RECORD_RESP_V01
      uim_read_record_resp_msg_v01* read_resp = QMI_node.update_default_qmi_uim_read_record_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
      read_resp->read_result_valid = TRUE;
      read_resp->read_result.content_len = 0x4e;
      uint8_t tmp_content[] = {
        0x4C, 0x00, 0xA8, 0x23, 0xC0, 0x03, 0x4F, 0x3A, 0x01, 0xC5, 0x03, 0x4F, 0x09, 0x02, 0xC4, 0x03, 0x4F,
        0x11, 0x04, 0xC4, 0x03, 0x4F, 0x13, 0x05, 0xC4, 0x03, 0x4F, 0x15, 0x06, 0xCA, 0x03, 0x4F, 0x50, 0x09,
        0xC9, 0x03, 0x4F, 0x21, 0x0C, 0xAA, 0x0F, 0xC2, 0x03, 0x4F, 0x4A, 0x08, 0xC7, 0x03, 0x4F, 0x4B, 0x14,
        0xCB, 0x03, 0x4F, 0x3D, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
      };
      memcpy(read_resp->read_result.content, tmp_content, 0x4e);
      read_resp->card_result_valid = TRUE;
      read_resp->card_result.sw1 = 0x90;
      read_resp->card_result.sw2 = 0x00;
      t.inject(QMI_node);

      /*
      if (card1_active)
      {
        // Expect: QMI_UIM_GET_FILE_ATTRIBUTES_REQ_V01
        memset(&session_info, 0, sizeof(session_info));
        session_info.session_type = UIM_SESSION_TYPE_SECONDARY_GW_V01;
        session_info.aid_len = 0x00;
        memset(&file_id, 0, sizeof(file_id));
        uint8_t path2[10] = {0x00,0x3F,0x10,0x7F,0x3A,0x5F};
        file_id.file_id = 0x4f30;
        file_id.path_len = 0x06;
        memcpy(file_id.path, path2, 0x06);
        QMI_node.update_default_qmi_uim_get_file_attributes_req(session_info, file_id);
        t.expect(QMI_node);

        // Inject: QMI_UIM_GET_FILE_ATTRIBUTES_RESP_V01
        QMI_node.update_default_qmi_uim_get_file_attributes_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
        card_result.sw1 = 0x90;
        card_result.sw2 = 0x00;
        QMI_node.set_qmi_uim_get_file_attributes_resp_card_result(card_result);
        QMI_node.set_qmi_uim_get_file_attribues_resp_file_status(UIM_FILE_STATUS_ACTIVATED_V01);
        file_attributes = QMI_node.set_qmi_uim_get_file_attributes_resp_file_attributes();
        file_attributes->file_size = 0x4c;
        file_attributes->file_id = 0x4f30;
        file_attributes->file_type = UIM_FILE_TYPE_LINEAR_FIXED_V01;
        file_attributes->rec_size = 0x4c;
        file_attributes->rec_count = 0x01;
        file_attributes->sec_read = UIM_SEC_CONDITION_SINGLE_COND_V01;
        file_attributes->sec_read_mask = 0x01;
        file_attributes->sec_write = UIM_SEC_CONDITION_SINGLE_COND_V01;
        file_attributes->sec_write_mask = 0x08;
        file_attributes->sec_increase = UIM_SEC_CONDITION_ALWAYS_V01;
        file_attributes->sec_increase_mask = 0x00;
        file_attributes->sec_deactivate = UIM_SEC_CONDITION_SINGLE_COND_V01;
        file_attributes->sec_deactivate_mask = 0x08;
        file_attributes->sec_activate = UIM_SEC_CONDITION_SINGLE_COND_V01;
        file_attributes->sec_activate_mask = 0x08;
        file_attributes->raw_value_len = 0x24;
        uint8_t temp_raw_value2[40] = {0x62,0x22,0x82,0x05,0x42,0x21,0x00,0x4C,0x01,0x83,0x02,0x4F,0x30,0xA5,0x03,
          0xC0,0x01,0x00,0x8A,0x01,0x05,0x8B,0x03,0x6F,0x06,0x03,0x80,0x02,0x00,0x4C,0x81,0x02,0x00,0x60,0x88,0x00};
        memcpy(file_attributes->raw_value, temp_raw_value2, 36);
        t.inject(QMI_node);

        // Expect: QMI_UIM_READ_RECORD_REQ_V01
        read_req = QMI_node.update_default_qmi_uim_read_record_reg();
        read_req->session_information.session_type = UIM_SESSION_TYPE_SECONDARY_GW_V01;
        read_req->session_information.aid_len = 0x00;
        read_req->file_id.file_id = 0x4f30;
        read_req->file_id.path_len = 0x06;
        uint8_t tmp_path2[] = {0x00,0x3F,0x10,0x7F,0x3A,0x5F};
        memcpy(read_req->file_id.path, tmp_path2, 0x06);
        read_req->read_record.record = 0x01;
        read_req->read_record.length = 0x4c;
        t.expect(QMI_node);

        // Inject: QMI_UIM_READ_RECORD_RESP_V01
        read_resp = QMI_node.update_default_qmi_uim_read_record_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
        read_resp->read_result_valid = TRUE;
        read_resp->read_result.content_len = 0x4e;
        uint8_t tmp_content2[] = {
          0x4C, 0x00, 0xA8, 0x23, 0xC0, 0x03, 0x4F, 0x3A, 0x01, 0xC5, 0x03, 0x4F, 0x09, 0x02, 0xC4, 0x03, 0x4F,
          0x11, 0x04, 0xC4, 0x03, 0x4F, 0x13, 0x05, 0xC4, 0x03, 0x4F, 0x15, 0x06, 0xCA, 0x03, 0x4F, 0x50, 0x09,
          0xC9, 0x03, 0x4F, 0x21, 0x0C, 0xAA, 0x0F, 0xC2, 0x03, 0x4F, 0x4A, 0x08, 0xC7, 0x03, 0x4F, 0x4B, 0x14,
          0xCB, 0x03, 0x4F, 0x3D, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
        };
        memcpy(read_resp->read_result.content, tmp_content2, 0x4e);
        read_resp->card_result_valid = TRUE;
        read_resp->card_result.sw1 = 0x90;
        read_resp->card_result.sw2 = 0x00;
        t.inject(QMI_node);
      }
      */
    }

    uim_register_refresh_type_v01 reg_ref;
    uim_session_information_type_v01 session_info;
    if (card0_active || card1_active)
    {
      // Expect: QMI_UIM_REFRESH_REGISTER_REQ_V01
      QMI_node.update_default_qmi_uim_get_file_attributes_req(session_info, reg_ref);
      QMI_node.set_expect_skip_verification_status(false);
      t.expect(QMI_node);

      // Inject: QMI_UIM_REFRESH_REGISTER_RESP_V01
      QMI_node.update_default_qmi_uim_refresh_register_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
      t.inject(QMI_node);
    }

    /*
    if (card0_active && card1_active)
    {
      // Expect: QMI_UIM_REFRESH_REGISTER_REQ_V01
      QMI_node.update_default_qmi_uim_get_file_attributes_req(session_info, reg_ref);
      QMI_node.set_expect_skip_verification_status(false);
      t.expect(QMI_node);

      // Inject: QMI_UIM_REFRESH_REGISTER_RESP_V01
      QMI_node.update_default_qmi_uim_refresh_register_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
      t.inject(QMI_node);
    }
    */
  }

  // Expect: RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED
  RIL_node.update_default_ril_unsol_response_sim_status_changed();
  t.expect(RIL_node);

  t.set_end();
}

void ril_utf_msim_bootup_helper_2(Testcase &t, int boot_config, int card0_active, int card1_active)
{
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  IMSTestnode IMS_node;
  OEMTestnode OEM_node;
  MSIMTestnode MSIM_node;

  // Inject: RIL_REQUEST_GET_SIM_STATUS
  RIL_node.update_default_ril_request_get_sim_status();
  t.inject(RIL_node);

  if (!card0_active && card1_active)
  {
    t.set_start();
  }

  /*
  // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
  QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_LOW_POWER_V01);
  QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);
  */

  uim_get_label_req_msg_v01* label_req1;
  uim_get_label_resp_msg_v01* response1;
  uint8_t aid_value_1[16] = {0xA0,0x00,0x00,0x00,0x87,0x10,0x02,0xF3,0x10,0xFF,0xFF,0x89,0x08,0x00,0x00,0xFF};
  if (card0_active || card1_active)
  {
    if (card0_active)
    {
      //Expect: QMI_UIM_GET_LABEL_REQ
      label_req1 = QMI_node.update_default_qmi_uim_get_label_req();
      label_req1->application_information.slot = (uim_slot_enum_v01)0x01;
      label_req1->application_information.aid_len = 0x10;
      memcpy(label_req1->application_information.aid, aid_value_1, 16);
      t.expect(QMI_node);

      //Inject: QMI_UIM_GET_LABEL_RESP
      response1 = QMI_node.update_default_qmi_uim_get_label_resp();
      response1->resp.result = QMI_RESULT_SUCCESS_V01;
      strcpy(response1->label, "USIM");
      response1->label_len = 4;
      response1->label_valid = TRUE;
      t.inject(QMI_node);
    }

    // Expect: RIL_REQUEST_GET_SIM_STATUS (resp)
    RIL_CardStatus_v6* status_ptr = RIL_node.update_default_ril_request_get_sim_status_resp();
    RIL_node.set_expect_skip_verification_status(false);
    status_ptr->card_state = RIL_CARDSTATE_PRESENT;
    status_ptr->universal_pin_state = RIL_PINSTATE_UNKNOWN;
    status_ptr->gsm_umts_subscription_app_index = 0;
    status_ptr->cdma_subscription_app_index = 2;
    status_ptr->ims_subscription_app_index = 1;
    status_ptr->num_applications = 4;

    status_ptr->applications[0].app_type = RIL_APPTYPE_USIM;
    status_ptr->applications[0].app_state = RIL_APPSTATE_READY;
    status_ptr->applications[0].perso_substate = RIL_PERSOSUBSTATE_READY;
    status_ptr->applications[0].pin1 = RIL_PINSTATE_DISABLED;
    status_ptr->applications[0].pin2 = RIL_PINSTATE_ENABLED_NOT_VERIFIED;
    status_ptr->applications[0].aid_ptr = (char *)malloc(33);
    strcpy(status_ptr->applications[0].aid_ptr, "a0000000871002f310ffff89080000ff");
    status_ptr->applications[0].app_label_ptr = (char *)malloc(5);
    strcpy(status_ptr->applications[0].app_label_ptr, "USIM");
    t.expect(RIL_node);
  }
  else
  {
    // Expect: RIL_REQUEST_GET_SIM_STATUS (resp)
    RIL_CardStatus_v6* status_ptr = RIL_node.update_default_ril_request_get_sim_status_resp();
    status_ptr->card_state = RIL_CARDSTATE_ABSENT;
    status_ptr->universal_pin_state = (RIL_PinState)0x0;
    status_ptr->cdma_subscription_app_index = -1;
    status_ptr->gsm_umts_subscription_app_index = -1;
    status_ptr->ims_subscription_app_index = -1;
    t.expect(RIL_node);
  }

  if (!card0_active && card1_active)
  {
    t.set_end();
  }

  // Inject: RIL_REQUEST_RADIO_POWER
  RIL_node.update_default_ril_request_radio_power();
  t.inject(RIL_node);

  t.set_start();


  // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
  QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_LOW_POWER_V01);
  QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  if (!card0_active)
  {
    // Expect: QMI_UIM_POWER_UP_REQ
    QMI_node.update_default_qmi_uim_power_up_req(1);
    t.expect(QMI_node);

    // Inject: QMI_UIM_POWER_UP_RESP
    QMI_node.update_default_qmi_uim_power_up_resp();
  }

  // Expect: IPC_MESSAGE_RADIO_POWER (recv)
  MSIM_node.update_default_ipc_message_radio_power_recv(1);
  MSIM_node.set_ipc_message_radio_power_recv_rild_instance(0);
  t.expect(MSIM_node);

  // Inject: IPC_MESSAGE_RADIO_POWER (send)
  MSIM_node.update_default_ipc_message_radio_power_send(1);
  MSIM_node.set_ipc_message_radio_power_send_rild_instance(1);
  t.inject(MSIM_node);

  /*
  // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
  QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_LOW_POWER_V01);
  QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);
  */

  // Expect: QMI_DMS_SET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_set_operating_mod_req_v01(DMS_OP_MODE_ONLINE_V01);
  t.expect(QMI_node);

  // Inject: QMI_DMS_SET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_set_operating_mode_resp_v01();
  QMI_node.set_qmi_dms_set_operating_mode_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject: QMI_DMS_EVENT_REPORT_IND_V01
  QMI_node.update_default_qmi_dms_event_report_ind_v01();
  QMI_node.set_qmi_dms_event_report_ind_v01_operating_mode(DMS_OP_MODE_ONLINE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_CONFIG_SIG_INFO2_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_config_sig_info2_req_msg_v01();
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_cdma_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_cdma_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_gsm_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_sinr_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rsrp_delta(60);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rsrq_delta(20);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_snr_delta(40);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_ecio_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_rscp_delta(40);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_rssi_delta(5);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_sinr_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_wcdma_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_wcdma_rssi_delta(50);
  t.expect(QMI_node);

  // Inject: QMI_NAS_CONFIG_SIG_INFO2_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_config_sig_info2_resp_msg_v01();
  QMI_node.set_qmi_nas_config_sig_info2_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);


  // Expect: QMI_NAS_SET_EVENT_REPORT_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_set_event_report_req_msg_v01();
  QMI_node.set_qmi_nas_set_event_report_req_report_reg_reject(TRUE);
  t.expect(QMI_node);

  // Inject: QMI_NAS_SET_EVENT_REPORT_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_set_event_report_resp_msg_v01();
  QMI_node.set_qmi_nas_set_event_report_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: RIL_REQUEST_RADIO_POWER (resp)
  RIL_node.update_default_ril_request_radio_power_resp();
  t.expect(RIL_node);

  // Expect: RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED
  RIL_node.update_default_ril_unsol_resp_radio_state_changed();
  t.expect(RIL_node);

  // Expect: QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_indication_register_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_INDICATION_REGISTER_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_indication_register_resp_msg_v01();
  QMI_node.set_qmi_nas_nas_indication_register_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_config_plmn_name_ind_reporting_req_msg_v01();
  QMI_node.set_qmi_nas_config_plmn_name_ind_reporting_req_msg_send_all_information(1);
  t.expect(QMI_node);

  // Inject: QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_config_plmn_name_ind_reporting_resp_msg_v01();
  QMI_node.set_qmi_nas_config_plmn_name_ind_reporting_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
  nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
  // Set values for data
  ptr->gsm_srv_status_info_valid = TRUE;
  ptr->gsm_srv_status_info.srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
  ptr->gsm_srv_status_info.true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
  ptr->gsm_srv_status_info.is_pref_data_path = 0;
  ptr->wcdma_srv_status_info_valid = TRUE;
  ptr->wcdma_srv_status_info.srv_status = NAS_SYS_SRV_STATUS_SRV_V01;
  ptr->wcdma_srv_status_info.true_srv_status = SYS_SRV_STATUS_SRV_V01;
  ptr->wcdma_srv_status_info.is_pref_data_path = 0;
  ptr->wcdma_sys_info.common_sys_info.srv_domain_valid = TRUE;
  ptr->wcdma_sys_info.common_sys_info.srv_domain = SYS_SRV_DOMAIN_CS_PS_V01;
  ptr->wcdma_sys_info.common_sys_info.srv_capability_valid = TRUE;
  ptr->wcdma_sys_info.common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
  ptr->wcdma_sys_info.common_sys_info.roam_status_valid = TRUE;
  ptr->wcdma_sys_info.common_sys_info.roam_status = NAS_SYS_ROAM_STATUS_OFF_V01;
  ptr->wcdma_sys_info.common_sys_info.is_sys_forbidden_valid = TRUE;
  ptr->wcdma_sys_info.common_sys_info.is_sys_forbidden = 0;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.lac_valid = TRUE;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.lac = 43024;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.cell_id_valid = TRUE;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.cell_id = 13548179;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.network_id_valid = TRUE;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.hs_call_status_valid = TRUE;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.hs_call_status = SYS_HS_IND_HSDPA_HSUPA_UNSUPP_CELL_V01;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.hs_ind_valid = TRUE;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.hs_ind = SYS_HS_IND_HSDPA_HSUPA_UNSUPP_CELL_V01;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.psc_valid = TRUE;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.psc = 426;
  ptr->wcdma_cipher_domain_valid = TRUE;
  ptr->wcdma_cipher_domain = SYS_SRV_DOMAIN_CS_ONLY_V01;
  ptr->sim_rej_info_valid = TRUE;
  ptr->sim_rej_info = NAS_SIM_AVAILABLE_V01;
  QMI_node.set_qmi_nas_get_sys_info_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_ERR_RATE_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_err_rate_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_ERR_RATE_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_err_rate_resp_msg_v01();
  QMI_node.set_qmi_nas_get_err_rate_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_SIG_INFO_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_sig_info_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_SIG_INFO_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_sig_info_resp_msg_v01();
  QMI_node.set_qmi_nas_get_sig_info_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_EMBMS_STATUS_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_embms_status_req_msg_v01();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_EMBMS_STATUS_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_embms_status_resp_msg_v01();
  QMI_node.set_qmi_nas_get_embms_status_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_PBM_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_pbm_indication_register_req_msg();
  QMI_node.set_qmi_pbm_indication_register_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01|PBM_REG_RECORD_UPDATE_EVENTS_V01|PBM_REG_PHONEBOOK_READY_EVENTS_V01);
  t.expect(QMI_node);

  // Inject: QMI_PBM_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_pbm_indication_register_resp_msg();
  QMI_node.set_qmi_pbm_indication_register_resp_v01_reg_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01|PBM_REG_RECORD_UPDATE_EVENTS_V01|PBM_REG_PHONEBOOK_READY_EVENTS_V01);
  QMI_node.set_qmi_pbm_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // End unordered expect set
  t.set_end();

  if (card0_active)
  {
    // Inject: RIL_REQUEST_SET_UICC_SUBSCRIPTION
    RIL_SelectUiccSub* uicc_subs_data = RIL_node.update_default_ril_request_set_uicc_subscription();
    RIL_node.set_ril_request_set_uicc_subscription_app_index(0);
    RIL_node.set_ril_request_set_uicc_subscription_slot(0);
    RIL_node.set_ril_request_set_uicc_subscription_act_status(RIL_UICC_SUBSCRIPTION_ACTIVATE);
    RIL_node.set_ril_request_set_uicc_subscription_sub_type(RIL_SUBSCRIPTION_1);
    t.inject(RIL_node);

    //Expect: QMI_NAS_GET_MODE_PREF_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_mode_pref_req();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_MODE_PREF_RESP_MSG_V01
    QMI_node.update_default_qmi_nas_get_mode_pref_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    //QMI_node.update_default_qmi_nas_get_mode_pref_resp_msg(1, 3);
    QMI_node.set_qmi_nas_get_mode_pref_resp_idx0_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM);
    QMI_node.set_qmi_nas_get_mode_pref_resp_idx1_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM);
    QMI_node.set_qmi_nas_get_mode_pref_resp_idx2_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM);
    t.inject(QMI_node);

    //Expect: QMI_UIM_GET_LABEL_REQ
    label_req1 = QMI_node.update_default_qmi_uim_get_label_req();
    label_req1->application_information.slot = (uim_slot_enum_v01)0x01;
    label_req1->application_information.aid_len = 0x10;
    memcpy(label_req1->application_information.aid, aid_value_1, 16);
    t.expect(QMI_node);

    //Inject: QMI_UIM_GET_LABEL_RESP
    response1 = QMI_node.update_default_qmi_uim_get_label_resp();
    response1->resp.result = QMI_RESULT_SUCCESS_V01;
    strcpy(response1->label, "USIM");
    response1->label_len = 4;
    response1->label_valid = TRUE;
    t.inject(QMI_node);

    //Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ
    QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01
    QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
    QMI_node.set_qmi_nas_get_system_selection_preference_resp_emergency_mode(0);
    mode_pref_mask_type_v01 mode_pref1 = QMI_NAS_RAT_MODE_PREF_UMTS_V01;
    mode_pref1 |= QMI_NAS_RAT_MODE_PREF_GSM_V01;
    QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(mode_pref1);
    // Set band_pref
    nas_band_pref_mask_type_v01 band_pref;
    band_pref |= QMI_NAS_BAND_CLASS_0_A_SYSTEM_V01;
    band_pref |= QMI_NAS_BAND_CLASS_0_B_AB_GSM850_V01;
    band_pref |= QMI_NAS_BAND_CLASS_1_ALL_BLOCKS_V01;
    band_pref |= QMI_NAS_BAND_CLASS_2_PLACEHOLDER_V01;
    band_pref |= QMI_NAS_BAND_CLASS_3_A_SYSTEM_V01;
    band_pref |= QMI_NAS_BAND_CLASS_4_ALL_BLOCKS_V01;
    band_pref |= QMI_NAS_BAND_CLASS_5_ALL_BLOCKS_V01;
    band_pref |= QMI_NAS_GSM_DCS_1800_BAND_V01;
    band_pref |= QMI_NAS_E_GSM_900_BAND_V01;
    band_pref |= QMI_NAS_P_GSM_900_BAND_V01;
    band_pref |= QMI_NAS_BAND_CLASS_6_V01;
    band_pref |= QMI_NAS_BAND_CLASS_7_V01;
    band_pref |= QMI_NAS_BAND_CLASS_8_V01;
    band_pref |= QMI_NAS_BAND_CLASS_9_V01;
    band_pref |= QMI_NAS_BAND_CLASS_10_V01;
    band_pref |= QMI_NAS_BAND_CLASS_11_V01;
    band_pref |= QMI_NAS_GSM_BAND_450_V01;
    band_pref |= QMI_NAS_GSM_BAND_480_V01;
    band_pref |= QMI_NAS_GSM_BAND_750_V01;
    band_pref |= QMI_NAS_GSM_BAND_850_V01;
    band_pref |= QMI_NAS_GSM_BAND_RAILWAYS_900_BAND_V01;
    band_pref |= QMI_NAS_GSM_BAND_PCS_1900_BAND_V01;
    band_pref |= QMI_NAS_WCDMA_EU_J_CH_IMT_2100_BAND_V01;
    band_pref |= QMI_NAS_WCDMA_US_PCS_1900_BAND_V01;
    band_pref |= QMI_NAS_EU_CH_DCS_1800_BAND_V01;
    band_pref |= QMI_NAS_WCDMA_US_1700_BAND_V01;
    band_pref |= QMI_NAS_WCDMA_JAPAN_800_BAND_V01;
    band_pref |= QMI_NAS_BAND_CLASS_12_V01;
    band_pref |= QMI_NAS_BAND_CLASS_14_V01;
    band_pref |= QMI_NAS_RESERVED_V01;
    band_pref |= QMI_NAS_BAND_CLASS_15_V01;
    band_pref |= QMI_NAS_WCDMA_EU_2600_BAND_V01;
    band_pref |= QMI_NAS_WCDMA_EU_J_900_BAND_V01;
    band_pref |= QMI_NAS_WCDMA_J_1700_BAND_V01;
    band_pref |= QMI_NAS_BAND_CLASS_16_V01;
    band_pref |= QMI_NAS_BAND_CLASS_17_V01;
    band_pref |= QMI_NAS_BAND_CLASS_18_V01;
    band_pref |= QMI_NAS_BAND_CLASS_19_V01;
    QMI_node.set_qmi_nas_get_system_selection_preference_resp_band_pref(band_pref);
    QMI_node.set_qmi_nas_get_system_selection_preference_resp_prl_pref(NAS_PRL_PREF_ANY_V01);
    QMI_node.set_qmi_nas_get_system_selection_preference_resp_roam_pref(NAS_ROAMING_PREF_ANY_V01);
    // Set band_pref_ext
    uint64_t band_pref_ext;
    band_pref_ext |= E_UTRA_OPERATING_BAND_1_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_2_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_3_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_4_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_5_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_6_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_7_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_8_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_9_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_10_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_11_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_12_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_13_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_14_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_17_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_18_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_19_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_20_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_21_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_24_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_25_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_33_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_34_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_35_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_36_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_37_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_38_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_39_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_40_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_41_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_42_V01;
    band_pref_ext |= E_UTRA_OPERATING_BAND_43_V01;
    QMI_node.set_qmi_nas_get_system_selection_preference_resp_band_pref_ext(band_pref_ext);
    QMI_node.set_qmi_nas_get_system_selection_preference_resp_net_sel_pref(NAS_NET_SEL_PREF_AUTOMATIC_V01);
    QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_PS_V01);
    QMI_node.set_qmi_nas_get_system_selection_preference_resp_gw_acq_order_pref(NAS_GW_ACQ_ORDER_PREF_AUTOMATIC_V01);
    // Set tdscdma_band_pref
    nas_tdscdma_band_pref_mask_type_v01 tdscdma_band_pref;
    tdscdma_band_pref |= NAS_TDSCDMA_BAND_A_V01;
    tdscdma_band_pref |= NAS_TDSCDMA_BAND_B_V01;
    tdscdma_band_pref |= NAS_TDSCDMA_BAND_C_V01;
    tdscdma_band_pref |= NAS_TDSCDMA_BAND_D_V01;
    tdscdma_band_pref |= NAS_TDSCDMA_BAND_E_V01;
    tdscdma_band_pref |= NAS_TDSCDMA_BAND_F_V01;
    QMI_node.set_qmi_nas_get_system_selection_preference_resp_tdscdma_band_pref(tdscdma_band_pref);
    QMI_node.set_default_qmi_nas_get_system_selection_preference_resp_acq_order();
    QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_reg_restriction(NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01);
    QMI_node.set_qmi_nas_get_system_selection_preference_resp_usage_setting(NAS_USAGE_VOICE_CENTRIC_V01);
    QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    // Expect: RIL_REQUEST_SET_UICC_SUBSCRIPTION_RESP
    RIL_node.update_default_ril_request_set_uicc_subscription_resp();
    RIL_node.set_ril_error_value((RIL_Errno)0);
    t.expect(RIL_node);

    //QMI_NAS_GET_SUBSCRIPTION_INFO_REQ_MSG_V01
    util_ril_utf_get_subscription_info(t, QMI_node, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, TRUE, NAS_PRIORITY_SUBSCRIPTION_TRUE_V01, TRUE, NAS_SUBSCRIPTION_ACTIVE_V01, TRUE, 0, TRUE, 0, FALSE, 0, FALSE, 0);

    // Expect: RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED
    RIL_node.update_default_ril_unsol_uicc_subscription_status_changed(1);
    t.expect(RIL_node);

  #ifdef RIL_UTF_L_MASTER
    // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
    nas_get_sys_info_resp_msg_v01 *ptr1 = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
    t.inject(QMI_node);

    // Expect: RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED
    RIL_node.update_default_ril_unsol_response_voice_network_state_changed();
    t.expect(RIL_node);

    // Expect: RIL_UNSOL_SIGNAL_STRENGTH
    RIL_node.update_default_ril_unsol_signal_strength();
    t.expect(RIL_node);
  #endif

    // Expect: RIL_UNSOL_VOICE_RADIO_TECH_CHANGED
    RIL_node.update_default_ril_unsol_voice_radio_tech_changed();
    t.expect(RIL_node);

    // Expect: RIL_UNSOL_SIGNAL_STRENGTH
    RIL_node.update_default_ril_unsol_signal_strength();
    t.expect(RIL_node);
  }
}

void ril_utf_msim_bootup_helper(Testcase &t, int boot_config)
{
  ril_utf_msim_bootup_helper_1(t, boot_config, TRUE, TRUE);
  ril_utf_msim_bootup_helper_2(t, boot_config, TRUE, TRUE);
}

test_result qmi_ril_dummy_bootup()
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  IMSTestnode IMS_node;
  OEMTestnode OEM_node;
  MSIMTestnode MSIM_node;

  t.set_test_id(__func__);
  t.set_test_description("This test is necessary to boot qmi_ril"
    " for multi sim primary sim test cases");
  t.set_test_attribute("ALL");
  // SINCE THIS IS A BOOT TEST, IT SHOULD NOT HAVE A DEFINED BOOT CASE
  // ALL BOOT TESTS NEED TO HAVE THEIR BOOT CASE SET TO NULL
  t.set_boot_case(NULL);

  // Execute the test case
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result qmi_ril_msim_dummy_bootup()
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  IMSTestnode IMS_node;
  OEMTestnode OEM_node;
  MSIMTestnode MSIM_node;

  t.set_test_id(__func__);
  t.set_test_description("This test is necessary to boot qmi_ril"
    " for multi sim primary sim test cases");
  t.set_test_attribute("ALL");
  // SINCE THIS IS A BOOT TEST, IT SHOULD NOT HAVE A DEFINED BOOT CASE
  // ALL BOOT TESTS NEED TO HAVE THEIR BOOT CASE SET TO NULL
  t.set_boot_case(NULL);

  ril_utf_set_primary_rild();

  // Start MSIM emulation threads
  utf_start_msim_threads();

  // Execute the test case
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result qmi_ril_msim_bootup_primary_001()
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  IMSTestnode IMS_node;
  OEMTestnode OEM_node;
  MSIMTestnode MSIM_node;

  t.set_test_id(__func__);
  t.set_test_description("This test is necessary to boot qmi_ril"
    " for multi sim primary sim test cases");
  t.set_test_attribute("ALL");
  // SINCE THIS IS A BOOT TEST, IT SHOULD NOT HAVE A DEFINED BOOT CASE
  // ALL BOOT TESTS NEED TO HAVE THEIR BOOT CASE SET TO NULL
  t.set_boot_case(NULL);

  // Set msim config
  property_set("persist.radio.multisim.config", "dsds");
  property_set("ro.baseband","dsda");
  ril_utf_set_primary_rild();


  // Start MSIM emulation threads
  utf_start_msim_threads();

  ril_utf_msim_bootup_helper(t, DEFAULT_BOOT);

  // Start oem threads for oem socket
  t.add_func_exec(utf_start_oem_threads);

  // Start the ims threads for ims
  t.add_func_exec(utf_start_ims_threads);

  // Expect: ims_MsgId_UNSOL_RADIO_STATE_CHANGED
  IMS_node.update_default_ims_unsol_radio_state_changed(ims_RadioState_RADIO_STATE_ON );
  t.expect(IMS_node);

  // Execute the test case
  t.execute();

  t.print_summary();

  return t.get_test_result();
}

test_result qmi_ril_msim_bootup_secondary_001()
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  IMSTestnode IMS_node;
  OEMTestnode OEM_node;
  MSIMTestnode MSIM_node;

  t.set_test_id(__func__);
  t.set_test_description("This test is necessary to boot qmi_ril"
    " for multi sim primary sim test cases");
  t.set_test_attribute("ALL");
  // SINCE THIS IS A BOOT TEST, IT SHOULD NOT HAVE A DEFINED BOOT CASE
  // ALL BOOT TESTS NEED TO HAVE THEIR BOOT CASE SET TO NULL
  t.set_boot_case(NULL);

  // Set msim config
  property_set("persist.radio.multisim.config", "dsds");
  property_set("ro.baseband","dsda");
  ril_utf_set_secondary_rild();


  // Start MSIM emulation threads
  utf_start_msim_threads();

  // Here begins the "real" test case
  // Expect: RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED
  RIL_node.update_default_ril_unsol_resp_radio_state_changed();
  t.expect(RIL_node);

  // Expect: QMI_NAS_BIND_SUBSCRIPTION_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_bind_subscription_req_msg(NAS_SECONDARY_SUBSCRIPTION_V01);
  t.expect(QMI_node);

  // Inject: QMI_NAS_BIND_SUBSCRIPTION_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_bind_subscription_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_BIND_SUBSCRIPTION_REQ_V02
  QMI_node.update_default_qmi_voice_bind_subscription_req_msg(VOICE_SUBS_TYPE_SECONDARY_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_BIND_SUBSCRIPTION_RESP_V02
  QMI_node.update_default_qmi_voice_bind_subscription_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_PBM_BIND_SUBSCRIPTION_REQ_V01
  QMI_node.update_default_qmi_pbm_bind_subscription_req_msg(PBM_SUBS_TYPE_SECONDARY_V01);
  t.expect(QMI_node);

  // Inject: QMI_PBM_BIND_SUBSCRIPTION_RESP_V01
  QMI_node.update_default_qmi_pbm_bind_subscription_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_BIND_SUBSCRIPTION_REQ_V01
  QMI_node.update_default_qmi_wms_bind_subscription_req(WMS_SECONDARY_SUBSCRIPTION_V01);
  t.expect(QMI_node);

  // Inject: QMI_WMS_BIND_SUBSCRIPTION_RESP_V01
  QMI_node.update_default_qmi_wms_bind_subscription_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_BIND_SUBSCRIPTION_REQ_V01
  QMI_node.update_default_qmi_dms_bind_subscription_req_msg(DMS_SECONDARY_SUBS_V01);
  t.expect(QMI_node);

  // Inject: QMI_DMS_BIND_SUBSCRIPTION_RESP_V01
  QMI_node.update_default_qmi_dms_bind_subscription_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_LTE_BIND_SUBSCRIPTION_REQ_V01
  QMI_node.update_default_qmi_lte_bind_subscription_req_msg(LTE_SECONDARY_SUBSCRIPTION_V01);
  t.expect(QMI_node);

  // Inject: QMI_LTE_BIND_SUBSCRIPTION_RESP_V01
  QMI_node.update_default_qmi_lte_bind_subscription_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_reg_voice_privacy_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_ext_brst_intl_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_speech_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_handover_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_conference_events(0x01);
  //QMI_node.set_qmi_voice_ind_req_tty_info_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_tty_info_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_cc_result_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_additional_call_info_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_audio_rat_change_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_vice_dialog_event(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_conf_participants_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_INDICATION_REGISTER_REQ_MSG_V01
  QMI_node.update_default_qmi_dms_indication_register_req_v01();
  QMI_node.set_qmi_dms_ind_req_report_ims_capability(0x01);
  t.expect(QMI_node);

  // Inject: QMI_DMS_INDICATION_REGISTER_RESP_MSG_V01
  QMI_node.update_default_qmi_dms_indication_register_resp_v01();
  QMI_node.set_qmi_dms_indication_register_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_SET_EVENT_REPORT_REQ_V01
  QMI_node.update_default_qmi_dms_set_event_report_req_msg();
  QMI_node.set_qmi_dsm_set_event_report_req_report_oprt_mode_state(0x01);
  QMI_node.set_qmi_dsm_set_event_report_req_report_prl_init(0x01);
  QMI_node.set_qmi_dms_set_event_report_req_report_device_multisim_info(0x01);
  t.expect(QMI_node);

  // Inject: QMI_DMS_SET_EVENT_REPORT_RESP_V01
  QMI_node.update_default_qmi_dms_set_event_report_resp_msg();
  QMI_node.set_qmi_dms_set_event_report_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_GET_DEVICE_CAP_REQ_V01
  QMI_node.update_default_qmi_dms_get_device_cap_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_DEVICE_CAP_RESP_V01
  QMI_node.update_default_qmi_dms_get_device_cap_resp_v01();
  dms_multisim_capability_type_v01 multisim_capability;
  memset(&multisim_capability, 0, sizeof(multisim_capability));
  multisim_capability.max_subscriptions = 2;
  QMI_node.set_default_qmi_dms_get_device_cap_resp_v01_multisim_capability(multisim_capability);
  QMI_node.set_default_qmi_dms_get_device_cap_resp_v01_device_capabilities();
  QMI_node.set_qmi_dms_get_device_cap_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  t.set_start();

#ifdef RIL_UTF_L_MASTER
  // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
  t.expect(QMI_node);

  // Inject:QMI_DMS_GET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
  QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_SHUTTING_DOWN_V01);
  QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);
#endif

  // Expect: QMI_DMS_QMI_DMS_TRIGGER_MODEM_ACTIVITY_INFO_CALCULATION_REQ_V01
  QMI_node.update_default_qmi_dms_trigger_modem_activity_info_calculation_req(TRUE, 1);
  t.expect(QMI_node);

  // Inject: QMI_DMS_QMI_DMS_TRIGGER_MODEM_ACTIVITY_INFO_CALCULATION_RESP_V01
  QMI_node.update_default_qmi_dms_trigger_modem_activity_info_calculation_resp();
  t.inject(QMI_node);
  t.set_end();

  /*
  // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
  t.expect(QMI_node);

  // Inject:QMI_DMS_GET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
  QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_SHUTTING_DOWN_V01);
  QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);
  */

  // Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_V01
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_emergency_mode(0);
  mode_pref_mask_type_v01 mode_pref1 = QMI_NAS_RAT_MODE_PREF_UMTS_V01;
  mode_pref1 |= QMI_NAS_RAT_MODE_PREF_GSM_V01;
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(mode_pref1);
  // Set band_pref
  nas_band_pref_mask_type_v01 band_pref;
  band_pref |= QMI_NAS_BAND_CLASS_0_A_SYSTEM_V01;
  band_pref |= QMI_NAS_BAND_CLASS_0_B_AB_GSM850_V01;
  band_pref |= QMI_NAS_BAND_CLASS_1_ALL_BLOCKS_V01;
  band_pref |= QMI_NAS_BAND_CLASS_2_PLACEHOLDER_V01;
  band_pref |= QMI_NAS_BAND_CLASS_3_A_SYSTEM_V01;
  band_pref |= QMI_NAS_BAND_CLASS_4_ALL_BLOCKS_V01;
  band_pref |= QMI_NAS_BAND_CLASS_5_ALL_BLOCKS_V01;
  band_pref |= QMI_NAS_GSM_DCS_1800_BAND_V01;
  band_pref |= QMI_NAS_E_GSM_900_BAND_V01;
  band_pref |= QMI_NAS_P_GSM_900_BAND_V01;
  band_pref |= QMI_NAS_BAND_CLASS_6_V01;
  band_pref |= QMI_NAS_BAND_CLASS_7_V01;
  band_pref |= QMI_NAS_BAND_CLASS_8_V01;
  band_pref |= QMI_NAS_BAND_CLASS_9_V01;
  band_pref |= QMI_NAS_BAND_CLASS_10_V01;
  band_pref |= QMI_NAS_BAND_CLASS_11_V01;
  band_pref |= QMI_NAS_GSM_BAND_450_V01;
  band_pref |= QMI_NAS_GSM_BAND_480_V01;
  band_pref |= QMI_NAS_GSM_BAND_750_V01;
  band_pref |= QMI_NAS_GSM_BAND_850_V01;
  band_pref |= QMI_NAS_GSM_BAND_RAILWAYS_900_BAND_V01;
  band_pref |= QMI_NAS_GSM_BAND_PCS_1900_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_EU_J_CH_IMT_2100_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_US_PCS_1900_BAND_V01;
  band_pref |= QMI_NAS_EU_CH_DCS_1800_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_US_1700_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_JAPAN_800_BAND_V01;
  band_pref |= QMI_NAS_BAND_CLASS_12_V01;
  band_pref |= QMI_NAS_BAND_CLASS_14_V01;
  band_pref |= QMI_NAS_RESERVED_V01;
  band_pref |= QMI_NAS_BAND_CLASS_15_V01;
  band_pref |= QMI_NAS_WCDMA_EU_2600_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_EU_J_900_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_J_1700_BAND_V01;
  band_pref |= QMI_NAS_BAND_CLASS_16_V01;
  band_pref |= QMI_NAS_BAND_CLASS_17_V01;
  band_pref |= QMI_NAS_BAND_CLASS_18_V01;
  band_pref |= QMI_NAS_BAND_CLASS_19_V01;
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_band_pref(band_pref);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_prl_pref(NAS_PRL_PREF_ANY_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_roam_pref(NAS_ROAMING_PREF_ANY_V01);
  // Set band_pref_ext
  uint64_t band_pref_ext;
  band_pref_ext |= E_UTRA_OPERATING_BAND_1_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_2_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_3_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_4_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_5_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_6_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_7_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_8_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_9_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_10_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_11_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_12_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_13_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_14_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_17_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_18_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_19_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_20_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_21_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_24_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_25_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_33_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_34_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_35_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_36_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_37_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_38_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_39_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_40_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_41_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_42_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_43_V01;
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_band_pref_ext(band_pref_ext);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_net_sel_pref(NAS_NET_SEL_PREF_AUTOMATIC_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_PS_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_gw_acq_order_pref(NAS_GW_ACQ_ORDER_PREF_AUTOMATIC_V01);
  // Set tdscdma_band_pref
  nas_tdscdma_band_pref_mask_type_v01 tdscdma_band_pref;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_A_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_B_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_C_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_D_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_E_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_F_V01;
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_tdscdma_band_pref(tdscdma_band_pref);
  QMI_node.set_default_qmi_nas_get_system_selection_preference_resp_acq_order();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_reg_restriction(NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_usage_setting(NAS_USAGE_VOICE_CENTRIC_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_CONFIG_SIG_INFO2_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_config_sig_info2_req_msg_v01();
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_cdma_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_cdma_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_gsm_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_sinr_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rsrp_delta(60);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rsrq_delta(20);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_snr_delta(40);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_ecio_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_rscp_delta(40);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_rssi_delta(5);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_sinr_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_wcdma_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_wcdma_rssi_delta(50);
  t.expect(QMI_node);

  // Inject: QMI_NAS_CONFIG_SIG_INFO2_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_config_sig_info2_resp_msg_v01();
  QMI_node.set_qmi_nas_config_sig_info2_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);


  // Expect: QMI_NAS_SET_EVENT_REPORT_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_set_event_report_req_msg_v01();
  QMI_node.set_qmi_nas_set_event_report_req_report_reg_reject(TRUE);
  t.expect(QMI_node);

  // Inject: QMI_NAS_SET_EVENT_REPORT_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_set_event_report_resp_msg_v01();
  QMI_node.set_qmi_nas_set_event_report_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_indication_register_req_msg_v01();
  QMI_node.set_qmi_nas_indication_register_req_msg_reg_sys_sel_pref(1);
  QMI_node.set_qmi_nas_indication_register_req_msg_dual_standby_pref(1);
  QMI_node.set_qmi_nas_indication_register_req_msg_subscription_info(1);
  QMI_node.set_qmi_nas_indication_register_reg_msg_reg_rtre_cfg(1);
  QMI_node.set_qmi_nas_indication_register_req_msg_req_serving_system(0);
  QMI_node.set_qmi_nas_indication_register_req_msg_reg_network_time(0);
  QMI_node.set_qmi_nas_indication_register_reg_msg_reg_embms_status(0);
  QMI_node.set_qmi_nas_indication_register_req_msg_sig_info(0);
  QMI_node.set_qmi_nas_indication_register_reg_msg_err_rate(0);
  QMI_node.set_qmi_nas_indication_register_req_msg_reg_current_plmn_name(0);
  QMI_node.set_qmi_nas_indication_register_req_msg_reg_rf_band_info(0);
  t.expect(QMI_node);

  // Inject: QMI_NAS_INDICATION_REGISTER_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_indication_register_resp_msg_v01();
  QMI_node.set_qmi_nas_nas_indication_register_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_config_plmn_name_ind_reporting_req_msg_v01();
  QMI_node.set_qmi_nas_config_plmn_name_ind_reporting_req_msg_send_all_information(1);
  t.expect(QMI_node);

  // Inject: QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_config_plmn_name_ind_reporting_resp_msg_v01();
  QMI_node.set_qmi_nas_config_plmn_name_ind_reporting_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_CENTRALIZED_EONS_SUPPORT_STATUS_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_centralized_eons_support_status_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_CENTRALIZED_EONS_SUPPORT_STATUS_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_centralized_eons_support_status_resp_msg_v01();
  QMI_node.set_qmi_nas_get_centralized_eons_support_status_resp_msg_centralized_eons_supported(1);
  QMI_node.set_qmi_nas_get_centralized_eons_support_status_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_LTE_SIB16_NETWORK_TIME_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_lte_sib16_network_time_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_LTE_SIB16_NETWORK_TIME_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_lte_sib16_network_time_resp_msg_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
  QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_LOW_POWER_V01);
  QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_PBM_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_pbm_indication_register_req_msg();
  QMI_node.set_qmi_pbm_indication_register_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01);
  t.expect(QMI_node);

  // Inject: QMI_PBM_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_pbm_indication_register_resp_msg();
  QMI_node.set_qmi_pbm_indication_register_resp_v01_reg_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01);
  QMI_node.set_qmi_pbm_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_PBM_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_pbm_indication_register_req_msg();
  QMI_node.set_qmi_pbm_indication_register_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01|PBM_REG_RECORD_UPDATE_EVENTS_V01|PBM_REG_PHONEBOOK_READY_EVENTS_V01);
  t.expect(QMI_node);

  // Inject: QMI_PBM_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_pbm_indication_register_resp_msg();
  QMI_node.set_qmi_pbm_indication_register_resp_v01_reg_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01|PBM_REG_RECORD_UPDATE_EVENTS_V01|PBM_REG_PHONEBOOK_READY_EVENTS_V01);
  QMI_node.set_qmi_pbm_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_LTE_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_lte_indication_register_req_msg();
  QMI_node.set_qmi_lte_indication_register_req_indication_bitmask(
          QMI_LTE_DISC_NOTIFICATION_IND_MASK_V01 |
          QMI_LTE_DISC_BROADCAST_NOTIFICATION_IND_MASK_V01 |
          QMI_LTE_DISC_MATCH_NOTIFICATION_IND_MASK_V01 |
          QMI_LTE_DISC_PSK_EXPIRED_IND_MASK_V01);
  t.expect(QMI_node);

  // Inject: QMI_LTE_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_lte_indication_register_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Start set of unordered expects
  t.set_start();

  // Expect: QMI_UIM_EVENT_REG_REQ_V01
  QMI_node.update_default_qmi_uim_event_reg_req(0x07);
  t.expect(QMI_node);

  // Inject: QMI_UIM_EVENT_REG_RESP_V01
  QMI_node.update_default_qmi_uim_event_reg_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_GET_CARD_STATUS_REQ_V01
  QMI_node.update_default_qmi_uim_get_card_status_req();
  QMI_node.set_qmi_uim_get_card_status_req_extended_card_status(0x01);
  t.expect(QMI_node);

  // Inject: QMI_UIM_GET_CARD_STATUS_RESP_V01
  uim_get_card_status_resp_msg_v01* status = QMI_node.update_default_qmi_uim_get_card_status_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  status->extended_card_status_valid = TRUE;
  status->extended_card_status.card_info_len = 2;
  status->extended_card_status.index_gw[0] = 0x0;
  status->extended_card_status.index_gw[1] = 0x100;
  status->extended_card_status.index_gw_len = 2;
  status->extended_card_status.index_1x_len = 0;

  uint8_t aid_value_1[16] = {0xA0,0x00,0x00,0x00,0x87,0x10,0x02,0xF3,0x10,0xFF,0xFF,0x89,0x08,0x00,0x00,0xFF};
  // Card 0
  status->extended_card_status.card_info[0].card_state = UIM_EXTENDED_CARD_STATE_PRESENT_V01;
  status->extended_card_status.card_info[0].upin.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
  status->extended_card_status.card_info[0].upin.pin_retries = 0x00;
  status->extended_card_status.card_info[0].upin.puk_retries = 0x00;
  status->extended_card_status.card_info[0].app_info_len = 0x01;
  // Card 0 App 0
  status->extended_card_status.card_info[0].app_info[0].app_type = UIM_APP_TYPE_USIM_V01;
  status->extended_card_status.card_info[0].app_info[0].app_state = UIM_APP_STATE_READY_V01;
  status->extended_card_status.card_info[0].app_info[0].perso_state = UIM_PERSO_STATE_READY_V01;
  status->extended_card_status.card_info[0].app_info[0].perso_feature = UIM_PERSO_FEATURE_STATUS_UNKNOWN_V01;
  status->extended_card_status.card_info[0].app_info[0].perso_retries = 0x00;
  status->extended_card_status.card_info[0].app_info[0].perso_unblock_retries = 0x00;
  status->extended_card_status.card_info[0].app_info[0].aid_value_len = 0x10;
  memcpy(status->extended_card_status.card_info[0].app_info[0].aid_value, aid_value_1, 16);
  status->extended_card_status.card_info[0].app_info[0].univ_pin = UIM_UNIV_PIN_PIN1_USED_V01;
  status->extended_card_status.card_info[0].app_info[0].pin1.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
  status->extended_card_status.card_info[0].app_info[0].pin1.pin_retries = 0x00;
  status->extended_card_status.card_info[0].app_info[0].pin1.puk_retries = 0x00;
  status->extended_card_status.card_info[0].app_info[0].pin2.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
  status->extended_card_status.card_info[0].app_info[0].pin2.pin_retries = 0x00;
  status->extended_card_status.card_info[0].app_info[0].pin2.puk_retries = 0x00;

  // Card 1
  status->extended_card_status.card_info[1].card_state = UIM_EXTENDED_CARD_STATE_PRESENT_V01;
  status->extended_card_status.card_info[1].upin.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
  status->extended_card_status.card_info[1].upin.pin_retries = 0x00;
  status->extended_card_status.card_info[1].upin.puk_retries = 0x00;
  status->extended_card_status.card_info[1].app_info_len = 0x01;
  // Card 1 App 0
  status->extended_card_status.card_info[1].app_info[0].app_type = UIM_APP_TYPE_USIM_V01;
  status->extended_card_status.card_info[1].app_info[0].app_state = UIM_APP_STATE_READY_V01;
  status->extended_card_status.card_info[1].app_info[0].perso_state = UIM_PERSO_STATE_READY_V01;
  status->extended_card_status.card_info[1].app_info[0].perso_feature = UIM_PERSO_FEATURE_STATUS_UNKNOWN_V01;
  status->extended_card_status.card_info[1].app_info[0].perso_retries = 0x00;
  status->extended_card_status.card_info[1].app_info[0].perso_unblock_retries = 0x00;
  status->extended_card_status.card_info[1].app_info[0].aid_value_len = 0x10;
  memcpy(status->extended_card_status.card_info[1].app_info[0].aid_value, aid_value_1, 16);
  status->extended_card_status.card_info[1].app_info[0].univ_pin = UIM_UNIV_PIN_PIN1_USED_V01;
  status->extended_card_status.card_info[1].app_info[0].pin1.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
  status->extended_card_status.card_info[1].app_info[0].pin1.pin_retries = 0x00;
  status->extended_card_status.card_info[1].app_info[0].pin1.puk_retries = 0x00;
  status->extended_card_status.card_info[1].app_info[0].pin2.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
  status->extended_card_status.card_info[1].app_info[0].pin2.pin_retries = 0x00;
  status->extended_card_status.card_info[1].app_info[0].pin2.puk_retries = 0x00;
  t.inject(QMI_node);


  // Expect: QMI_CAT_GET_CONFIGURATION_REQ_V02
  QMI_node.update_default_qmi_cat_get_configuration_req();
  t.expect(QMI_node);

  // Inject: QMI_CAT_GET_CONFIGURATION_RESP_V02
  QMI_node.update_default_qmi_cat_get_configuration_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_CAT_SET_EVENT_REPORT_REQ_V02
  QMI_node.update_default_qmi_cat_set_event_report_req();
#ifdef RIL_UTF_L_MASTER
  QMI_node.set_qmi_cat_set_event_report_req_pc_evt_report_req_mask(0x0b7FFFFF);
#else
  QMI_node.set_qmi_cat_set_event_report_req_pc_evt_report_req_mask(0x037FFFFF);
#endif
  QMI_node.set_qmi_cat_set_event_report_req_slot_mask(CAT_SET_EVENT_REPORT_SLOT_2_V02);
  t.expect(QMI_node);

  // Inject: QMI_CAT_SET_EVENT_REPORT_RESP_V02
  QMI_node.update_default_qmi_cat_set_event_report_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_CAT_SET_EVENT_REPORT_REQ_V02
  QMI_node.update_default_qmi_cat_set_event_report_req();
  QMI_node.set_qmi_cat_set_event_report_req_pc_dec_evt_report_req_mask(0x00800000);
  QMI_node.set_qmi_cat_set_event_report_req_slot_mask(CAT_SET_EVENT_REPORT_SLOT_2_V02);
  t.expect(QMI_node);

  // Inject: QMI_CAT_SET_EVENT_REPORT_RESP_V02
  QMI_node.update_default_qmi_cat_set_event_report_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_IMSP_SET_EVENT_REPORT_REQ_V01
  QMI_node.update_default_qmi_imsp_set_event_report_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_IMSP_SET_EVENT_REPORT_RESP_V01
  QMI_node.update_default_qmi_imsp_set_event_report_resp_v01();
  QMI_node.set_qmi_imsp_set_event_report_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_IMS_SETTINGS_GET_REG_MGR_CONFIG_REQ_V01
  QMI_node.update_default_qmi_ims_settings_get_reg_mgr_config_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_GET_REG_MGR_CONFIG_RSP_V01
  QMI_node.update_default_qmi_ims_settings_get_reg_mgr_config_rsp_v01();
  QMI_node.set_qmi_ims_settings_get_reg_mgr_config_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_REQ_V01
  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_RSP_V01
  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_rsp_v01();
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_IMSA_IND_REG_REQ_V01
  QMI_node.update_default_qmi_imsa_ind_reg_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_IMSA_IND_REG_RSP_V01
  QMI_node.update_default_qmi_imsa_ind_reg_rsp_v01();
  QMI_node.set_qmi_imsa_ind_req_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_IMS_SETTINGS_CONFIG_IND_REG_REQ_V01
  QMI_node.update_default_qmi_ims_config_ind_reg_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_CONFIG_IND_REG_RSP_V01
  QMI_node.update_default_qmi_ims_config_ind_reg_rsp_v01();
  QMI_node.set_qmi_ims_config_ind_req_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_GET_DEVICE_CAP_REQ_V01
  QMI_node.update_default_qmi_dms_get_device_cap_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_DEVICE_CAP_RESP_V01
  QMI_node.update_default_qmi_dms_get_device_cap_resp_v01();
  memset(&multisim_capability, 0, sizeof(multisim_capability));
  multisim_capability.max_subscriptions = 2;
  QMI_node.set_default_qmi_dms_get_device_cap_resp_v01_multisim_capability(multisim_capability);
  QMI_node.set_default_qmi_dms_get_device_cap_resp_v01_device_capabilities();
  QMI_node.set_qmi_dms_get_device_cap_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  //QMI_NAS_GET_SUBSCRIPTION_INFO_REQ_MSG_V01
  util_ril_utf_get_subscription_info(t, QMI_node, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, TRUE, NAS_PRIORITY_SUBSCRIPTION_TRUE_V01, TRUE, NAS_SUBSCRIPTION_ACTIVE_V01, TRUE, 0, TRUE, 0, FALSE, 0, FALSE, 0);

  // Expect: QMI_DMS_GET_CURRENT_PRL_INFO_REQ_V01
  QMI_node.update_default_qmi_dms_get_current_prl_info_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_CURRENT_PRL_INFO_RESP_V01
  QMI_node.update_default_qmi_dms_get_current_prl_info_resp_v01();
  QMI_node.set_qmi_dms_get_current_prl_info_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED
  RIL_node.update_default_ril_unsol_resp_radio_state_changed();
  t.expect(RIL_node);

  // Expect: QMI_DMS_GET_DEVICE_MFR_REQ_V01
  QMI_node.update_default_qmi_dms_get_device_mfr_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_DEVICE_MFR_RESP_V01
  QMI_node.update_default_qmi_dms_get_device_mfr_resp_v01();
  QMI_node.set_default_qmi_dms_get_device_mfr_resp_v01_device_manufacturer();
  QMI_node.set_qmi_dms_get_device_mfr_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ_V01
  QMI_node.update_default_qmi_wms_get_transport_layer_info_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_WMS_GET_TRANSPORT_LAYER_INFO_RESP_V01
  QMI_node.update_default_qmi_wms_get_transport_layer_info_resp_v01();
  QMI_node.set_qmi_wms_get_transport_layer_info_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_SET_ROUTES_REQ_V01
  QMI_node.update_default_qmi_wms_set_routes_req_v01();
//TODO:  Add sane values here and enable payload validation
  t.expect(QMI_node);

  // Inject: QMI_WMS_SET_ROUTES_RESP_V01
  QMI_node.update_default_qmi_wms_set_routes_resp_v01();
  QMI_node.set_qmi_wms_set_routes_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_SET_EVENT_REPORT_REQ_V01
  QMI_node.update_default_qmi_wms_set_event_report_req_v01();
//TODO:  Add sane values here and enable payload validation
  t.expect(QMI_node);

  // Inject: QMI_WMS_SET_EVENT_REPORT_RESP_V01
  QMI_node.update_default_qmi_wms_set_event_report_resp_v01();
  QMI_node.set_qmi_wms_set_event_report_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_wms_indication_register_req_v01();
  QMI_node.set_qmi_wms_indication_register_req_v01_reg_transport_layer_info_events(1);
  QMI_node.set_qmi_wms_indication_register_req_v01_reg_transport_layer_mwi_info_events(1);
  t.expect(QMI_node);

  // Inject: QMI_WMS_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_wms_indication_register_resp_v01();
  QMI_node.set_qmi_wms_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_GET_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_wms_get_indication_register_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_WMS_GET_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_wms_get_indication_register_resp_v01();
  QMI_node.set_qmi_wms_get_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  //TODO: Fill with sane values to send to qmi-ril
  t.inject(QMI_node);

  // Expect: QMI_WMS_SET_PRIMARY_CLIENT_REQ_V01
  QMI_node.update_default_qmi_wms_set_primary_client_req_v01();
  QMI_node.set_qmi_wms_set_primary_client_req_v01_primary_client(1);
  t.expect(QMI_node);

  // Inject: QMI_WMS_SET_PRIMARY_CLIENT_RESP_V01
  QMI_node.update_default_qmi_wms_set_primary_client_resp_v01();
  QMI_node.set_qmi_wms_set_primary_client_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);


  // Expect: QMI_DMS_GET_CURRENT_PRL_INFO_REQ_V01
  QMI_node.update_default_qmi_dms_get_current_prl_info_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_CURRENT_PRL_INFO_RESP_V01
  QMI_node.update_default_qmi_dms_get_current_prl_info_resp_v01();
  QMI_node.set_qmi_dms_get_current_prl_info_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_READ_TRANSPARENT_REQ_V01
  QMI_node.update_default_qmi_uim_read_transparent_reg();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_READ_TRANSPARENT_RESP_V01
  QMI_node.update_default_qmi_uim_read_transparent_resp(QMI_RESULT_SUCCESS_V01,QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  /*
#ifdef RIL_UTF_L_MASTER
  // Expect: QMI_UIM_READ_TRANSPARENT_REQ_V01
  QMI_node.update_default_qmi_uim_read_transparent_reg();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_READ_TRANSPARENT_RESP_V01
  QMI_node.update_default_qmi_uim_read_transparent_resp(QMI_RESULT_FAILURE_V01, QMI_ERR_GENERAL_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_READ_TRANSPARENT_REQ_V01
  QMI_node.update_default_qmi_uim_read_transparent_reg();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_READ_TRANSPARENT_RESP_V01
  QMI_node.update_default_qmi_uim_read_transparent_resp(QMI_RESULT_FAILURE_V01, QMI_ERR_GENERAL_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_UIM_GET_ICCID_REQ_V01
  QMI_node.update_default_qmi_dms_get_iccid_req();
  t.expect(QMI_node);

  // Inject: QMI_DMS_UIM_GET_ICCID_RESP_V01
  uim_id = QMI_node.update_default_qmi_dms_get_iccid_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  memcpy(uim_id, tmp_uim_id, 20);
  t.inject(QMI_node);

  // Expect: QMI_DMS_UIM_GET_ICCID_REQ_V01
  QMI_node.update_default_qmi_dms_get_iccid_req();
  t.expect(QMI_node);

  // Inject: QMI_DMS_UIM_GET_ICCID_RESP_V01
  uim_id = QMI_node.update_default_qmi_dms_get_iccid_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  memcpy(uim_id, tmp_uim_id, 20);
  t.inject(QMI_node);
#endif
  */

  // Expect: QMI_UIM_GET_SERVICE_STATUS_REQ_V01
  uim_session_information_type_v01 session_info;
  memset(&session_info, 0, sizeof(session_info));
  session_info.session_type = UIM_SESSION_TYPE_SECONDARY_GW_V01;
  session_info.aid_len = 0;
  QMI_node.update_default_qmi_uim_get_service_status_req(session_info, 0x01);
  t.expect(QMI_node);

  // Inject: QMI_UIM_GET_SERVICE_STATUS_RESP_V01
  QMI_node.update_default_qmi_uim_get_service_status_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_uim_get_service_status_resp_fdn_status(UIM_FDN_STATUS_AVAILABLE_DISABLED_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_SET_AP_SW_VERSION_REQ_V01
  QMI_node.update_default_qmi_dms_set_ap_sw_version_req_v01();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_DMS_SET_AP_SW_VERSION_RESP_V01
  QMI_node.update_default_qmi_dms_set_ap_sw_version_resp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_GET_FILE_ATTRIBUTES_REQ_V01
  memset(&session_info, 0, sizeof(session_info));
  session_info.session_type = UIM_SESSION_TYPE_SECONDARY_GW_V01;
  session_info.aid_len = 0x00;
  uim_file_id_type_v01 file_id;
  memset(&file_id, 0, sizeof(file_id));
  uint8_t path[10] = {0x00,0x3F,0x10,0x7F,0x3A,0x5F};
  file_id.file_id = 0x4f30;
  file_id.path_len = 0x06;
  memcpy(file_id.path, path, 0x06);
  QMI_node.update_default_qmi_uim_get_file_attributes_req(session_info, file_id);
  t.expect(QMI_node);

  // Inject: QMI_UIM_GET_FILE_ATTRIBUTES_RESP_V01
  QMI_node.update_default_qmi_uim_get_file_attributes_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  uim_card_result_type_v01 card_result;
  card_result.sw1 = 0x90;
  card_result.sw2 = 0x00;
  QMI_node.set_qmi_uim_get_file_attributes_resp_card_result(card_result);
  QMI_node.set_qmi_uim_get_file_attribues_resp_file_status(UIM_FILE_STATUS_ACTIVATED_V01);
  uim_file_attributes_type_v01* file_attributes = QMI_node.set_qmi_uim_get_file_attributes_resp_file_attributes();
  file_attributes->file_size = 0x4c;
  file_attributes->file_id = 0x4f30;
  file_attributes->file_type = UIM_FILE_TYPE_LINEAR_FIXED_V01;
  file_attributes->rec_size = 0x4c;
  file_attributes->rec_count = 0x01;
  file_attributes->sec_read = UIM_SEC_CONDITION_SINGLE_COND_V01;
  file_attributes->sec_read_mask = 0x01;
  file_attributes->sec_write = UIM_SEC_CONDITION_SINGLE_COND_V01;
  file_attributes->sec_write_mask = 0x08;
  file_attributes->sec_increase = UIM_SEC_CONDITION_ALWAYS_V01;
  file_attributes->sec_increase_mask = 0x00;
  file_attributes->sec_deactivate = UIM_SEC_CONDITION_SINGLE_COND_V01;
  file_attributes->sec_deactivate_mask = 0x08;
  file_attributes->sec_activate = UIM_SEC_CONDITION_SINGLE_COND_V01;
  file_attributes->sec_activate_mask = 0x08;
  file_attributes->raw_value_len = 0x24;
  uint8_t temp_raw_value[40] = {0x62,0x22,0x82,0x05,0x42,0x21,0x00,0x4C,0x01,0x83,0x02,0x4F,0x30,0xA5,0x03,
    0xC0,0x01,0x00,0x8A,0x01,0x05,0x8B,0x03,0x6F,0x06,0x03,0x80,0x02,0x00,0x4C,0x81,0x02,0x00,0x60,0x88,0x00};
  memcpy(file_attributes->raw_value, temp_raw_value, 36);
  t.inject(QMI_node);

  // Expect: QMI_UIM_READ_RECORD_REQ_V01
  uim_read_record_req_msg_v01* read_req = QMI_node.update_default_qmi_uim_read_record_reg();
  read_req->session_information.session_type = UIM_SESSION_TYPE_SECONDARY_GW_V01;
  read_req->session_information.aid_len = 0x00;
  read_req->file_id.file_id = 0x4f30;
  read_req->file_id.path_len = 0x06;
  uint8_t tmp_path[] = {0x00,0x3F,0x10,0x7F,0x3A,0x5F};
  memcpy(read_req->file_id.path, tmp_path, 0x06);
  read_req->read_record.record = 0x01;
  read_req->read_record.length = 0x4c;
  t.expect(QMI_node);

  // Inject: QMI_UIM_READ_RECORD_RESP_V01
  uim_read_record_resp_msg_v01* read_resp = QMI_node.update_default_qmi_uim_read_record_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  read_resp->read_result_valid = TRUE;
  read_resp->read_result.content_len = 0x4e;
  uint8_t tmp_content[] = {
    0x4C, 0x00, 0xA8, 0x23, 0xC0, 0x03, 0x4F, 0x3A, 0x01, 0xC5, 0x03, 0x4F, 0x09, 0x02, 0xC4, 0x03, 0x4F,
    0x11, 0x04, 0xC4, 0x03, 0x4F, 0x13, 0x05, 0xC4, 0x03, 0x4F, 0x15, 0x06, 0xCA, 0x03, 0x4F, 0x50, 0x09,
    0xC9, 0x03, 0x4F, 0x21, 0x0C, 0xAA, 0x0F, 0xC2, 0x03, 0x4F, 0x4A, 0x08, 0xC7, 0x03, 0x4F, 0x4B, 0x14,
    0xCB, 0x03, 0x4F, 0x3D, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
  };
  memcpy(read_resp->read_result.content, tmp_content, 0x4e);
  read_resp->card_result_valid = TRUE;
  read_resp->card_result.sw1 = 0x90;
  read_resp->card_result.sw2 = 0x00;
  t.inject(QMI_node);

  /*
  // Expect: QMI_UIM_GET_FILE_ATTRIBUTES_REQ_V01
  memset(&session_info, 0, sizeof(session_info));
  session_info.session_type = UIM_SESSION_TYPE_SECONDARY_GW_V01;
  session_info.aid_len = 0x00;
  memset(&file_id, 0, sizeof(file_id));
  uint8_t path2[10] = {0x00,0x3F,0x10,0x7F,0x3A,0x5F};
  file_id.file_id = 0x4f30;
  file_id.path_len = 0x06;
  memcpy(file_id.path, path2, 0x06);
  QMI_node.update_default_qmi_uim_get_file_attributes_req(session_info, file_id);
  t.expect(QMI_node);

  // Inject: QMI_UIM_GET_FILE_ATTRIBUTES_RESP_V01
  QMI_node.update_default_qmi_uim_get_file_attributes_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  card_result.sw1 = 0x90;
  card_result.sw2 = 0x00;
  QMI_node.set_qmi_uim_get_file_attributes_resp_card_result(card_result);
  QMI_node.set_qmi_uim_get_file_attribues_resp_file_status(UIM_FILE_STATUS_ACTIVATED_V01);
  file_attributes = QMI_node.set_qmi_uim_get_file_attributes_resp_file_attributes();
  file_attributes->file_size = 0x4c;
  file_attributes->file_id = 0x4f30;
  file_attributes->file_type = UIM_FILE_TYPE_LINEAR_FIXED_V01;
  file_attributes->rec_size = 0x4c;
  file_attributes->rec_count = 0x01;
  file_attributes->sec_read = UIM_SEC_CONDITION_SINGLE_COND_V01;
  file_attributes->sec_read_mask = 0x01;
  file_attributes->sec_write = UIM_SEC_CONDITION_SINGLE_COND_V01;
  file_attributes->sec_write_mask = 0x08;
  file_attributes->sec_increase = UIM_SEC_CONDITION_ALWAYS_V01;
  file_attributes->sec_increase_mask = 0x00;
  file_attributes->sec_deactivate = UIM_SEC_CONDITION_SINGLE_COND_V01;
  file_attributes->sec_deactivate_mask = 0x08;
  file_attributes->sec_activate = UIM_SEC_CONDITION_SINGLE_COND_V01;
  file_attributes->sec_activate_mask = 0x08;
  file_attributes->raw_value_len = 0x24;
  uint8_t temp_raw_value2[40] = {0x62,0x22,0x82,0x05,0x42,0x21,0x00,0x4C,0x01,0x83,0x02,0x4F,0x30,0xA5,0x03,
    0xC0,0x01,0x00,0x8A,0x01,0x05,0x8B,0x03,0x6F,0x06,0x03,0x80,0x02,0x00,0x4C,0x81,0x02,0x00,0x60,0x88,0x00};
  memcpy(file_attributes->raw_value, temp_raw_value2, 36);
  t.inject(QMI_node);

  // Expect: QMI_UIM_READ_RECORD_REQ_V01
  read_req = QMI_node.update_default_qmi_uim_read_record_reg();
  read_req->session_information.session_type = UIM_SESSION_TYPE_SECONDARY_GW_V01;
  read_req->session_information.aid_len = 0x00;
  read_req->file_id.file_id = 0x4f30;
  read_req->file_id.path_len = 0x06;
  uint8_t tmp_path2[] = {0x00,0x3F,0x10,0x7F,0x3A,0x5F};
  memcpy(read_req->file_id.path, tmp_path2, 0x06);
  read_req->read_record.record = 0x01;
  read_req->read_record.length = 0x4c;
  t.expect(QMI_node);

  // Inject: QMI_UIM_READ_RECORD_RESP_V01
  read_resp = QMI_node.update_default_qmi_uim_read_record_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  read_resp->read_result_valid = TRUE;
  read_resp->read_result.content_len = 0x4e;
  uint8_t tmp_content2[] = {
    0x4C, 0x00, 0xA8, 0x23, 0xC0, 0x03, 0x4F, 0x3A, 0x01, 0xC5, 0x03, 0x4F, 0x09, 0x02, 0xC4, 0x03, 0x4F,
    0x11, 0x04, 0xC4, 0x03, 0x4F, 0x13, 0x05, 0xC4, 0x03, 0x4F, 0x15, 0x06, 0xCA, 0x03, 0x4F, 0x50, 0x09,
    0xC9, 0x03, 0x4F, 0x21, 0x0C, 0xAA, 0x0F, 0xC2, 0x03, 0x4F, 0x4A, 0x08, 0xC7, 0x03, 0x4F, 0x4B, 0x14,
    0xCB, 0x03, 0x4F, 0x3D, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
  };
  memcpy(read_resp->read_result.content, tmp_content2, 0x4e);
  read_resp->card_result_valid = TRUE;
  read_resp->card_result.sw1 = 0x90;
  read_resp->card_result.sw2 = 0x00;
  t.inject(QMI_node);
  */

  // Expect: QMI_UIM_REFRESH_REGISTER_REQ_V01
  uim_register_refresh_type_v01 reg_ref;
  QMI_node.update_default_qmi_uim_get_file_attributes_req(session_info, reg_ref);
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_REFRESH_REGISTER_RESP_V01
  QMI_node.update_default_qmi_uim_refresh_register_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  /*
  // Expect: QMI_UIM_REFRESH_REGISTER_REQ_V01
  QMI_node.update_default_qmi_uim_get_file_attributes_req(session_info, reg_ref);
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_REFRESH_REGISTER_RESP_V01
  QMI_node.update_default_qmi_uim_refresh_register_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);
  */

  // Expect: RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED
  RIL_node.update_default_ril_unsol_response_sim_status_changed();
  t.expect(RIL_node);

  // Expect: QMI_IMS_SETTINGS_GET_SUPPORTED_MSGS_REQ_V01
  QMI_node.update_default_qmi_ims_settings_get_supported_msgs_req();
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_GET_SUPPORTED_MSGS_RESP_V01
  QMI_node.update_default_qmi_ims_settings_get_supported_msgs_resp();
  t.inject(QMI_node);

  t.set_end();

  // Inject: RIL_REQUEST_GET_SIM_STATUS
  RIL_node.update_default_ril_request_get_sim_status();
  t.inject(RIL_node);

  /*
  // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
  QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_LOW_POWER_V01);
  QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);
  */

  //Expect: QMI_UIM_GET_LABEL_REQ
  uim_get_label_req_msg_v01 *label_req1 = QMI_node.update_default_qmi_uim_get_label_req();
  label_req1->application_information.slot = (uim_slot_enum_v01)0x02;
  label_req1->application_information.aid_len = 0x10;
  memcpy(label_req1->application_information.aid, aid_value_1, 16);
  t.expect(QMI_node);

  //Inject: QMI_UIM_GET_LABEL_RESP
  uim_get_label_resp_msg_v01 *response1 = QMI_node.update_default_qmi_uim_get_label_resp();
  response1->resp.result = QMI_RESULT_SUCCESS_V01;
  strcpy(response1->label, "USIM");
  response1->label_len = 4;
  response1->label_valid = TRUE;
  t.inject(QMI_node);

  // Expect: RIL_REQUEST_GET_SIM_STATUS (resp)
  RIL_CardStatus_v6* status_ptr = RIL_node.update_default_ril_request_get_sim_status_resp();
  RIL_node.set_expect_skip_verification_status(false);
  t.expect(RIL_node);

  // Inject: RIL_REQUEST_RADIO_POWER
  RIL_node.update_default_ril_request_radio_power();
  t.inject(RIL_node);

  t.set_start();


  // Expect: IPC_MESSAGE_RADIO_POWER (recv)
  MSIM_node.update_default_ipc_message_radio_power_recv(1);
  MSIM_node.set_ipc_message_radio_power_recv_rild_instance(1);
  t.expect(MSIM_node);

  // Inject: IPC_MESSAGE_RADIO_POWER (send)
  MSIM_node.update_default_ipc_message_radio_power_send(1);
  MSIM_node.set_ipc_message_radio_power_send_rild_instance(0);
  t.inject(MSIM_node);

  // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
  QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_LOW_POWER_V01);
  QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_SET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_set_operating_mod_req_v01(DMS_OP_MODE_ONLINE_V01);
  t.expect(QMI_node);

  // Inject: QMI_DMS_SET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_set_operating_mode_resp_v01();
  QMI_node.set_qmi_dms_set_operating_mode_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject: QMI_DMS_EVENT_REPORT_IND_V01
  QMI_node.update_default_qmi_dms_event_report_ind_v01();
  QMI_node.set_qmi_dms_event_report_ind_v01_operating_mode(DMS_OP_MODE_ONLINE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_CONFIG_SIG_INFO2_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_config_sig_info2_req_msg_v01();
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_cdma_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_cdma_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_gsm_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_sinr_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rsrp_delta(60);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rsrq_delta(20);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_snr_delta(40);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_ecio_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_rscp_delta(40);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_rssi_delta(5);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_sinr_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_wcdma_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_wcdma_rssi_delta(50);
  t.expect(QMI_node);

  // Inject: QMI_NAS_CONFIG_SIG_INFO2_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_config_sig_info2_resp_msg_v01();
  QMI_node.set_qmi_nas_config_sig_info2_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);


  // Expect: QMI_NAS_SET_EVENT_REPORT_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_set_event_report_req_msg_v01();
  QMI_node.set_qmi_nas_set_event_report_req_report_reg_reject(TRUE);
  t.expect(QMI_node);

  // Inject: QMI_NAS_SET_EVENT_REPORT_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_set_event_report_resp_msg_v01();
  QMI_node.set_qmi_nas_set_event_report_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: RIL_REQUEST_RADIO_POWER (resp)
  RIL_node.update_default_ril_request_radio_power_resp();
  t.expect(RIL_node);

  // Expect: RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED
  RIL_node.update_default_ril_unsol_resp_radio_state_changed();
  t.expect(RIL_node);

  // Expect: QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_indication_register_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_INDICATION_REGISTER_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_indication_register_resp_msg_v01();
  QMI_node.set_qmi_nas_nas_indication_register_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_config_plmn_name_ind_reporting_req_msg_v01();
  QMI_node.set_qmi_nas_config_plmn_name_ind_reporting_req_msg_send_all_information(1);
  t.expect(QMI_node);

  // Inject: QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_config_plmn_name_ind_reporting_resp_msg_v01();
  QMI_node.set_qmi_nas_config_plmn_name_ind_reporting_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
  nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
  // Set values for data
  ptr->gsm_srv_status_info_valid = TRUE;
  ptr->gsm_srv_status_info.srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
  ptr->gsm_srv_status_info.true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
  ptr->gsm_srv_status_info.is_pref_data_path = 0;
  ptr->wcdma_srv_status_info_valid = TRUE;
  ptr->wcdma_srv_status_info.srv_status = NAS_SYS_SRV_STATUS_SRV_V01;
  ptr->wcdma_srv_status_info.true_srv_status = SYS_SRV_STATUS_SRV_V01;
  ptr->wcdma_srv_status_info.is_pref_data_path = 0;
  ptr->wcdma_sys_info.common_sys_info.srv_domain_valid = TRUE;
  ptr->wcdma_sys_info.common_sys_info.srv_domain = SYS_SRV_DOMAIN_CS_PS_V01;
  ptr->wcdma_sys_info.common_sys_info.srv_capability_valid = TRUE;
  ptr->wcdma_sys_info.common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
  ptr->wcdma_sys_info.common_sys_info.roam_status_valid = TRUE;
  ptr->wcdma_sys_info.common_sys_info.roam_status = NAS_SYS_ROAM_STATUS_OFF_V01;
  ptr->wcdma_sys_info.common_sys_info.is_sys_forbidden_valid = TRUE;
  ptr->wcdma_sys_info.common_sys_info.is_sys_forbidden = 0;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.lac_valid = TRUE;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.lac = 43024;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.cell_id_valid = TRUE;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.cell_id = 13548179;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.network_id_valid = TRUE;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.hs_call_status_valid = TRUE;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.hs_call_status = SYS_HS_IND_HSDPA_HSUPA_UNSUPP_CELL_V01;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.hs_ind_valid = TRUE;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.hs_ind = SYS_HS_IND_HSDPA_HSUPA_UNSUPP_CELL_V01;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.psc_valid = TRUE;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.psc = 426;
  ptr->wcdma_cipher_domain_valid = TRUE;
  ptr->wcdma_cipher_domain = SYS_SRV_DOMAIN_CS_ONLY_V01;
  ptr->sim_rej_info_valid = TRUE;
  ptr->sim_rej_info = NAS_SIM_AVAILABLE_V01;
  QMI_node.set_qmi_nas_get_sys_info_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_ERR_RATE_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_err_rate_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_ERR_RATE_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_err_rate_resp_msg_v01();
  QMI_node.set_qmi_nas_get_err_rate_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_SIG_INFO_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_sig_info_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_SIG_INFO_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_sig_info_resp_msg_v01();
  QMI_node.set_qmi_nas_get_sig_info_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_EMBMS_STATUS_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_embms_status_req_msg_v01();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_EMBMS_STATUS_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_embms_status_resp_msg_v01();
  QMI_node.set_qmi_nas_get_embms_status_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_PBM_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_pbm_indication_register_req_msg();
  QMI_node.set_qmi_pbm_indication_register_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01|PBM_REG_RECORD_UPDATE_EVENTS_V01|PBM_REG_PHONEBOOK_READY_EVENTS_V01);
  t.expect(QMI_node);

  // Inject: QMI_PBM_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_pbm_indication_register_resp_msg();
  QMI_node.set_qmi_pbm_indication_register_resp_v01_reg_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01|PBM_REG_RECORD_UPDATE_EVENTS_V01|PBM_REG_PHONEBOOK_READY_EVENTS_V01);
  QMI_node.set_qmi_pbm_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // End unordered expect set
  t.set_end();

  // Inject: RIL_REQUEST_SET_UICC_SUBSCRIPTION
  RIL_SelectUiccSub* uicc_subs_data = RIL_node.update_default_ril_request_set_uicc_subscription();
  RIL_node.set_ril_request_set_uicc_subscription_app_index(0);
  RIL_node.set_ril_request_set_uicc_subscription_slot(1);
  RIL_node.set_ril_request_set_uicc_subscription_act_status(RIL_UICC_SUBSCRIPTION_ACTIVATE);
  RIL_node.set_ril_request_set_uicc_subscription_sub_type(RIL_SUBSCRIPTION_2);
  t.inject(RIL_node);

  //Expect: QMI_NAS_GET_MODE_PREF_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_mode_pref_req();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_MODE_PREF_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_mode_pref_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  //QMI_node.update_default_qmi_nas_get_mode_pref_resp_msg(1, 3);
  QMI_node.set_qmi_nas_get_mode_pref_resp_idx0_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM);
  QMI_node.set_qmi_nas_get_mode_pref_resp_idx1_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM);
  QMI_node.set_qmi_nas_get_mode_pref_resp_idx2_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM);
  t.inject(QMI_node);

  //Expect: QMI_UIM_GET_LABEL_REQ
  label_req1 = QMI_node.update_default_qmi_uim_get_label_req();
  label_req1->application_information.slot = (uim_slot_enum_v01)0x02;
  label_req1->application_information.aid_len = 0x10;
  memcpy(label_req1->application_information.aid, aid_value_1, 16);
  t.expect(QMI_node);

  //Inject: QMI_UIM_GET_LABEL_RESP
  response1 = QMI_node.update_default_qmi_uim_get_label_resp();
  response1->resp.result = QMI_RESULT_SUCCESS_V01;
  strcpy(response1->label, "USIM");
  response1->label_len = 4;
  response1->label_valid = TRUE;
  t.inject(QMI_node);

  //Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_emergency_mode(0);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(mode_pref1);
  // Set band_pref
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_band_pref(band_pref);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_prl_pref(NAS_PRL_PREF_ANY_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_roam_pref(NAS_ROAMING_PREF_ANY_V01);
  // Set band_pref_ext
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_band_pref_ext(band_pref_ext);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_net_sel_pref(NAS_NET_SEL_PREF_AUTOMATIC_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_PS_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_gw_acq_order_pref(NAS_GW_ACQ_ORDER_PREF_AUTOMATIC_V01);
  // Set tdscdma_band_pref
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_tdscdma_band_pref(tdscdma_band_pref);
  QMI_node.set_default_qmi_nas_get_system_selection_preference_resp_acq_order();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_reg_restriction(NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_usage_setting(NAS_USAGE_VOICE_CENTRIC_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: RIL_REQUEST_SET_UICC_SUBSCRIPTION_RESP
  RIL_node.update_default_ril_request_set_uicc_subscription_resp();
  RIL_node.set_ril_error_value((RIL_Errno)0);
  t.expect(RIL_node);

  //QMI_NAS_GET_SUBSCRIPTION_INFO_REQ_MSG_V01
  util_ril_utf_get_subscription_info(t, QMI_node, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, TRUE, NAS_PRIORITY_SUBSCRIPTION_TRUE_V01, TRUE, NAS_SUBSCRIPTION_ACTIVE_V01, TRUE, 0, TRUE, 0, FALSE, 0, FALSE, 0);

  // Expect: RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED
  RIL_node.update_default_ril_unsol_uicc_subscription_status_changed(1);
  t.expect(RIL_node);

  // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
  ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
  QMI_node.set_qmi_nas_get_sys_info_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_voice_network_state_changed();
  t.expect(RIL_node);

  // Expect: RIL_UNSOL_SIGNAL_STRENGTH
  RIL_node.update_default_ril_unsol_signal_strength();
  t.expect(RIL_node);

  // Expect: RIL_UNSOL_VOICE_RADIO_TECH_CHANGED
  RIL_node.update_default_ril_unsol_voice_radio_tech_changed();
  t.expect(RIL_node);

  // Expect: RIL_UNSOL_SIGNAL_STRENGTH
  RIL_node.update_default_ril_unsol_signal_strength();
  t.expect(RIL_node);

  // Start oem threads for oem socket
  t.add_func_exec(utf_start_oem_threads);

  // Start the ims threads for ims
  t.add_func_exec(utf_start_ims_threads);

  // Expect: ims_MsgId_UNSOL_RADIO_STATE_CHANGED
  IMS_node.update_default_ims_unsol_radio_state_changed(ims_RadioState_RADIO_STATE_ON );
  t.expect(IMS_node);

  // Execute the test case
  t.execute();

  t.print_summary();


  return t.get_test_result();
}

void qmi_ril_partial_bootup_helper_001(Testcase &t)
{
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  IMSTestnode IMS_node;
  OEMTestnode OEM_node;
  MSIMTestnode MSIM_node;

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_reg_voice_privacy_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_ext_brst_intl_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_speech_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_handover_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_conference_events(0x01);
  //QMI_node.set_qmi_voice_ind_req_tty_info_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_tty_info_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_cc_result_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_additional_call_info_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_audio_rat_change_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_vice_dialog_event(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_conf_participants_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_INDICATION_REGISTER_REQ_MSG_V01
  QMI_node.update_default_qmi_dms_indication_register_req_v01();
  QMI_node.set_qmi_dms_ind_req_report_ims_capability(0x01);
  t.expect(QMI_node);

  // Inject: QMI_DMS_INDICATION_REGISTER_RESP_MSG_V01
  QMI_node.update_default_qmi_dms_indication_register_resp_v01();
  QMI_node.set_qmi_dms_indication_register_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_SET_EVENT_REPORT_REQ_V01
  QMI_node.update_default_qmi_dms_set_event_report_req_msg();
  QMI_node.set_qmi_dsm_set_event_report_req_report_oprt_mode_state(0x01);
  QMI_node.set_qmi_dsm_set_event_report_req_report_prl_init(0x01);
  t.expect(QMI_node);

  // Inject: QMI_DMS_SET_EVENT_REPORT_RESP_V01
  QMI_node.update_default_qmi_dms_set_event_report_resp_msg();
  QMI_node.set_qmi_dms_set_event_report_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_GET_DEVICE_CAP_REQ_V01
  QMI_node.update_default_qmi_dms_get_device_cap_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_DEVICE_CAP_RESP_V01
  QMI_node.update_default_qmi_dms_get_device_cap_resp_v01();
  QMI_node.set_default_qmi_dms_get_device_cap_resp_v01_device_capabilities();
  QMI_node.set_qmi_dms_get_device_cap_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);


#ifdef RIL_UTF_L_MASTER
  t.set_start();

  // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
  t.expect(QMI_node);

  // Inject:QMI_DMS_GET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
  QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_SHUTTING_DOWN_V01);
  QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_QMI_DMS_TRIGGER_MODEM_ACTIVITY_INFO_CALCULATION_REQ_V01
  QMI_node.update_default_qmi_dms_trigger_modem_activity_info_calculation_req(TRUE, 1);
  t.expect(QMI_node);

  // Inject: QMI_DMS_QMI_DMS_TRIGGER_MODEM_ACTIVITY_INFO_CALCULATION_RESP_V01
  QMI_node.update_default_qmi_dms_trigger_modem_activity_info_calculation_resp();
  t.inject(QMI_node);
  t.set_end();
#endif

  /*
  // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
  t.expect(QMI_node);

  // Inject:QMI_DMS_GET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
  QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_SHUTTING_DOWN_V01);
  QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);
  */

  t.set_start();

  // Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_V01
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  t.set_end();

  // Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_emergency_mode(0);
  mode_pref_mask_type_v01 mode_pref1 = QMI_NAS_RAT_MODE_PREF_UMTS_V01;
  mode_pref1 |= QMI_NAS_RAT_MODE_PREF_GSM_V01;
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(mode_pref1);
  // Set band_pref
  nas_band_pref_mask_type_v01 band_pref;
  band_pref |= QMI_NAS_BAND_CLASS_0_A_SYSTEM_V01;
  band_pref |= QMI_NAS_BAND_CLASS_0_B_AB_GSM850_V01;
  band_pref |= QMI_NAS_BAND_CLASS_1_ALL_BLOCKS_V01;
  band_pref |= QMI_NAS_BAND_CLASS_2_PLACEHOLDER_V01;
  band_pref |= QMI_NAS_BAND_CLASS_3_A_SYSTEM_V01;
  band_pref |= QMI_NAS_BAND_CLASS_4_ALL_BLOCKS_V01;
  band_pref |= QMI_NAS_BAND_CLASS_5_ALL_BLOCKS_V01;
  band_pref |= QMI_NAS_GSM_DCS_1800_BAND_V01;
  band_pref |= QMI_NAS_E_GSM_900_BAND_V01;
  band_pref |= QMI_NAS_P_GSM_900_BAND_V01;
  band_pref |= QMI_NAS_BAND_CLASS_6_V01;
  band_pref |= QMI_NAS_BAND_CLASS_7_V01;
  band_pref |= QMI_NAS_BAND_CLASS_8_V01;
  band_pref |= QMI_NAS_BAND_CLASS_9_V01;
  band_pref |= QMI_NAS_BAND_CLASS_10_V01;
  band_pref |= QMI_NAS_BAND_CLASS_11_V01;
  band_pref |= QMI_NAS_GSM_BAND_450_V01;
  band_pref |= QMI_NAS_GSM_BAND_480_V01;
  band_pref |= QMI_NAS_GSM_BAND_750_V01;
  band_pref |= QMI_NAS_GSM_BAND_850_V01;
  band_pref |= QMI_NAS_GSM_BAND_RAILWAYS_900_BAND_V01;
  band_pref |= QMI_NAS_GSM_BAND_PCS_1900_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_EU_J_CH_IMT_2100_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_US_PCS_1900_BAND_V01;
  band_pref |= QMI_NAS_EU_CH_DCS_1800_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_US_1700_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_JAPAN_800_BAND_V01;
  band_pref |= QMI_NAS_BAND_CLASS_12_V01;
  band_pref |= QMI_NAS_BAND_CLASS_14_V01;
  band_pref |= QMI_NAS_RESERVED_V01;
  band_pref |= QMI_NAS_BAND_CLASS_15_V01;
  band_pref |= QMI_NAS_WCDMA_EU_2600_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_EU_J_900_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_J_1700_BAND_V01;
  band_pref |= QMI_NAS_BAND_CLASS_16_V01;
  band_pref |= QMI_NAS_BAND_CLASS_17_V01;
  band_pref |= QMI_NAS_BAND_CLASS_18_V01;
  band_pref |= QMI_NAS_BAND_CLASS_19_V01;
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_band_pref(band_pref);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_prl_pref(NAS_PRL_PREF_ANY_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_roam_pref(NAS_ROAMING_PREF_ANY_V01);
  // Set band_pref_ext
  uint64_t band_pref_ext;
  band_pref_ext |= E_UTRA_OPERATING_BAND_1_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_2_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_3_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_4_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_5_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_6_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_7_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_8_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_9_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_10_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_11_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_12_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_13_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_14_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_17_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_18_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_19_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_20_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_21_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_24_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_25_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_33_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_34_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_35_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_36_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_37_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_38_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_39_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_40_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_41_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_42_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_43_V01;
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_band_pref_ext(band_pref_ext);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_net_sel_pref(NAS_NET_SEL_PREF_AUTOMATIC_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_PS_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_gw_acq_order_pref(NAS_GW_ACQ_ORDER_PREF_AUTOMATIC_V01);
  // Set tdscdma_band_pref
  nas_tdscdma_band_pref_mask_type_v01 tdscdma_band_pref;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_A_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_B_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_C_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_D_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_E_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_F_V01;
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_tdscdma_band_pref(tdscdma_band_pref);
  QMI_node.set_default_qmi_nas_get_system_selection_preference_resp_acq_order();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_reg_restriction(NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_usage_setting(NAS_USAGE_VOICE_CENTRIC_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Start set of unordered expects
  t.set_start();

  // Expect: QMI_NAS_CONFIG_SIG_INFO2_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_config_sig_info2_req_msg_v01();
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_cdma_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_cdma_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_gsm_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_sinr_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rsrp_delta(60);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rsrq_delta(20);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_snr_delta(40);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_ecio_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_rscp_delta(40);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_rssi_delta(5);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_sinr_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_wcdma_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_wcdma_rssi_delta(50);
  t.expect(QMI_node);

  // Inject: QMI_NAS_CONFIG_SIG_INFO2_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_config_sig_info2_resp_msg_v01();
  QMI_node.set_qmi_nas_config_sig_info2_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_SET_EVENT_REPORT_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_set_event_report_req_msg_v01();
  QMI_node.set_qmi_nas_set_event_report_req_report_reg_reject(TRUE);
  t.expect(QMI_node);

  // Inject: QMI_NAS_SET_EVENT_REPORT_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_set_event_report_resp_msg_v01();
  QMI_node.set_qmi_nas_set_event_report_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_indication_register_req_msg_v01();
  QMI_node.set_qmi_nas_indication_register_req_msg_reg_sys_sel_pref(1);
  QMI_node.set_qmi_nas_indication_register_req_msg_dual_standby_pref(1);
  QMI_node.set_qmi_nas_indication_register_req_msg_subscription_info(1);
  QMI_node.set_qmi_nas_indication_register_reg_msg_reg_rtre_cfg(1);
  QMI_node.set_qmi_nas_indication_register_req_msg_req_serving_system(0);
  QMI_node.set_qmi_nas_indication_register_req_msg_reg_network_time(0);
  QMI_node.set_qmi_nas_indication_register_reg_msg_reg_embms_status(0);
  QMI_node.set_qmi_nas_indication_register_req_msg_sig_info(0);
  QMI_node.set_qmi_nas_indication_register_reg_msg_err_rate(0);
  QMI_node.set_qmi_nas_indication_register_req_msg_reg_current_plmn_name(0);
  QMI_node.set_qmi_nas_indication_register_req_msg_reg_rf_band_info(0);
  t.expect(QMI_node);

  // Inject: QMI_NAS_INDICATION_REGISTER_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_indication_register_resp_msg_v01();
  QMI_node.set_qmi_nas_nas_indication_register_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_config_plmn_name_ind_reporting_req_msg_v01();
  QMI_node.set_qmi_nas_config_plmn_name_ind_reporting_req_msg_send_all_information(1);
  t.expect(QMI_node);

  // Inject: QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_config_plmn_name_ind_reporting_resp_msg_v01();
  QMI_node.set_qmi_nas_config_plmn_name_ind_reporting_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_CENTRALIZED_EONS_SUPPORT_STATUS_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_centralized_eons_support_status_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_CENTRALIZED_EONS_SUPPORT_STATUS_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_centralized_eons_support_status_resp_msg_v01();
  QMI_node.set_qmi_nas_get_centralized_eons_support_status_resp_msg_centralized_eons_supported(1);
  QMI_node.set_qmi_nas_get_centralized_eons_support_status_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_LTE_SIB16_NETWORK_TIME_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_lte_sib16_network_time_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_LTE_SIB16_NETWORK_TIME_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_lte_sib16_network_time_resp_msg_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
  QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_SHUTTING_DOWN_V01);
  QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_PBM_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_pbm_indication_register_req_msg();
  QMI_node.set_qmi_pbm_indication_register_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01);
  t.expect(QMI_node);

  // Inject: QMI_PBM_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_pbm_indication_register_resp_msg();
  QMI_node.set_qmi_pbm_indication_register_resp_v01_reg_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01);
  QMI_node.set_qmi_pbm_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_PBM_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_pbm_indication_register_req_msg();
  QMI_node.set_qmi_pbm_indication_register_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01|PBM_REG_RECORD_UPDATE_EVENTS_V01|PBM_REG_PHONEBOOK_READY_EVENTS_V01);
  t.expect(QMI_node);

  // Inject: QMI_PBM_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_pbm_indication_register_resp_msg();
  QMI_node.set_qmi_pbm_indication_register_resp_v01_reg_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01|PBM_REG_RECORD_UPDATE_EVENTS_V01|PBM_REG_PHONEBOOK_READY_EVENTS_V01);
  QMI_node.set_qmi_pbm_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_LTE_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_lte_indication_register_req_msg();
  QMI_node.set_qmi_lte_indication_register_req_indication_bitmask(
          QMI_LTE_DISC_NOTIFICATION_IND_MASK_V01 |
          QMI_LTE_DISC_BROADCAST_NOTIFICATION_IND_MASK_V01 |
          QMI_LTE_DISC_MATCH_NOTIFICATION_IND_MASK_V01 |
          QMI_LTE_DISC_PSK_EXPIRED_IND_MASK_V01);
  t.expect(QMI_node);

  // Inject: QMI_LTE_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_lte_indication_register_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_IMSP_SET_EVENT_REPORT_REQ_V01
  QMI_node.update_default_qmi_imsp_set_event_report_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_IMSP_SET_EVENT_REPORT_RESP_V01
  QMI_node.update_default_qmi_imsp_set_event_report_resp_v01();
  QMI_node.set_qmi_imsp_set_event_report_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  //QMI_NAS_GET_SUBSCRIPTION_INFO_REQ_MSG_V01
  util_ril_utf_get_subscription_info(t, QMI_node, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, TRUE, NAS_PRIORITY_SUBSCRIPTION_TRUE_V01, TRUE, NAS_SUBSCRIPTION_ACTIVE_V01, TRUE, 0, TRUE, 0, FALSE, 0, FALSE, 0);

  // Expect: QMI_DMS_GET_DEVICE_MFR_REQ_V01
  QMI_node.update_default_qmi_dms_get_device_mfr_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_DEVICE_MFR_RESP_V01
  QMI_node.update_default_qmi_dms_get_device_mfr_resp_v01();
  QMI_node.set_default_qmi_dms_get_device_mfr_resp_v01_device_manufacturer();
  QMI_node.set_qmi_dms_get_device_mfr_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_EMBMS_UNSOL_EMBMS_STATUS_IND
  OEM_node.update_default_qmi_embms_unsol_embms_status_ind();
  OEM_node.set_qmi_embms_unsol_embms_status(0);
  t.expect(OEM_node);

  // Expect: QMI_UIM_EVENT_REG_REQ_V01
  QMI_node.update_default_qmi_uim_event_reg_req(0x07);
  t.expect(QMI_node);

  // Inject: QMI_UIM_EVENT_REG_RESP_V01
  QMI_node.update_default_qmi_uim_event_reg_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_GET_CARD_STATUS_REQ_V01
  QMI_node.update_default_qmi_uim_get_card_status_req();
  QMI_node.set_qmi_uim_get_card_status_req_extended_card_status(0x01);
  t.expect(QMI_node);

  // Inject: QMI_UIM_GET_CARD_STATUS_RESP_V01
  uim_get_card_status_resp_msg_v01* card_status = QMI_node.update_default_qmi_uim_get_card_status_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  card_status->sim_busy_status_valid = TRUE;
  card_status->sim_busy_status_len = 0x01;
  card_status->sim_busy_status[0] = 0x00;
  card_status->extended_hot_swap_valid = TRUE;
  card_status->extended_hot_swap_len = 0x01;
  card_status->extended_hot_swap[0] = UIM_HOT_SWAP_NOT_SUPPORTED_V01;
  card_status->extended_card_status_valid = TRUE;
  card_status->extended_card_status.index_gw_len = 0x01;
  card_status->extended_card_status.index_1x_len = 0x01;
  card_status->extended_card_status.index_1x[0] = 0x0002;
  card_status->extended_card_status.card_info_len = 0x01;
  card_status->extended_card_status.card_info[0].card_state = UIM_EXTENDED_CARD_STATE_PRESENT_V01;
  card_status->extended_card_status.card_info[0].upin.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].upin.pin_retries = 0x00;
  card_status->extended_card_status.card_info[0].upin.puk_retries = 0x00;
  card_status->extended_card_status.card_info[0].error_code = UIM_CARD_ERROR_CODE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info_len = 0x04;
  // App slot 0
  card_status->extended_card_status.card_info[0].app_info[0].app_type = UIM_APP_TYPE_USIM_V01;
  card_status->extended_card_status.card_info[0].app_info[0].app_state = UIM_APP_STATE_READY_V01;
  card_status->extended_card_status.card_info[0].app_info[0].perso_state = UIM_PERSO_STATE_READY_V01;
  card_status->extended_card_status.card_info[0].app_info[0].perso_feature = UIM_PERSO_FEATURE_STATUS_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[0].perso_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[0].perso_unblock_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[0].aid_value_len = 0x10;
  uint8_t app_value_1[30] = {0xA0,0x00,0x00,0x00,0x87,0x10,0x02,0xF3,0x10,0xFF,0xFF,0x89,0x08,0x00,0x00,0xFF};
  memcpy(card_status->extended_card_status.card_info[0].app_info[0].aid_value, app_value_1, 16);
  card_status->extended_card_status.card_info[0].app_info[0].univ_pin = UIM_UNIV_PIN_PIN1_USED_V01;
  card_status->extended_card_status.card_info[0].app_info[0].pin1.pin_state = UIM_PIN_STATE_DISABLED_V01;
  card_status->extended_card_status.card_info[0].app_info[0].pin1.pin_retries = 0x03;
  card_status->extended_card_status.card_info[0].app_info[0].pin1.puk_retries = 0x0A;
  card_status->extended_card_status.card_info[0].app_info[0].pin2.pin_state = UIM_PIN_STATE_ENABLED_NOT_VERIFIED_V01;
  card_status->extended_card_status.card_info[0].app_info[0].pin2.pin_retries = 0x03;
  card_status->extended_card_status.card_info[0].app_info[0].pin2.puk_retries = 0x0A;
  // App slot 1
  card_status->extended_card_status.card_info[0].app_info[1].app_type = UIM_APP_TYPE_ISIM_V01;
  card_status->extended_card_status.card_info[0].app_info[1].app_state = UIM_APP_STATE_DETECTED_V01;
  card_status->extended_card_status.card_info[0].app_info[1].perso_state = UIM_PERSO_STATE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[1].perso_feature = UIM_PERSO_FEATURE_STATUS_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[1].perso_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[1].perso_unblock_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[1].aid_value_len = 0x10;
  uint8_t app_value_2[30] = {0xA0,0x00,0x00,0x00,0x87,0x10,0x04,0xF3,0x10,0xFF,0xFF,0x89,0x08,0x00,0x00,0xFF};
  memcpy(card_status->extended_card_status.card_info[0].app_info[1].aid_value, app_value_2, 16);
  card_status->extended_card_status.card_info[0].app_info[1].univ_pin = UIM_UNIV_PIN_PIN1_USED_V01;
  card_status->extended_card_status.card_info[0].app_info[1].pin1.pin_state = UIM_PIN_STATE_DISABLED_V01;
  card_status->extended_card_status.card_info[0].app_info[1].pin1.pin_retries = 0x03;
  card_status->extended_card_status.card_info[0].app_info[1].pin1.puk_retries = 0x0A;
  card_status->extended_card_status.card_info[0].app_info[1].pin2.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[1].pin2.pin_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[1].pin2.puk_retries = 0x00;
  // App slot 2
  card_status->extended_card_status.card_info[0].app_info[2].app_type = UIM_APP_TYPE_CSIM_V01;
  card_status->extended_card_status.card_info[0].app_info[2].app_state = UIM_APP_STATE_READY_V01;
  card_status->extended_card_status.card_info[0].app_info[2].perso_state = UIM_PERSO_STATE_READY_V01;
  card_status->extended_card_status.card_info[0].app_info[2].perso_feature = UIM_PERSO_FEATURE_STATUS_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[2].perso_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[2].perso_unblock_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[2].aid_value_len = 0x10;
  uint8_t app_value_3[30] = {0xA0,0x00,0x00,0x03,0x43,0x10,0x02,0xF3,0x10,0xFF,0xFF,0x89,0x02,0x00,0x00,0xFF};
  memcpy(card_status->extended_card_status.card_info[0].app_info[2].aid_value, app_value_3, 16);
  card_status->extended_card_status.card_info[0].app_info[2].univ_pin = UIM_UNIV_PIN_PIN1_USED_V01;
  card_status->extended_card_status.card_info[0].app_info[2].pin1.pin_state = UIM_PIN_STATE_DISABLED_V01;
  card_status->extended_card_status.card_info[0].app_info[2].pin1.pin_retries = 0x03;
  card_status->extended_card_status.card_info[0].app_info[2].pin1.puk_retries = 0x0A;
  card_status->extended_card_status.card_info[0].app_info[2].pin2.pin_state = UIM_PIN_STATE_ENABLED_NOT_VERIFIED_V01;
  card_status->extended_card_status.card_info[0].app_info[2].pin2.pin_retries = 0x03;
  card_status->extended_card_status.card_info[0].app_info[2].pin2.puk_retries = 0x0A;
  // App slot 3
  card_status->extended_card_status.card_info[0].app_info[3].app_type = UIM_APP_TYPE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[3].app_state = UIM_APP_STATE_DETECTED_V01;
  card_status->extended_card_status.card_info[0].app_info[3].perso_state = UIM_PERSO_STATE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[3].perso_feature = UIM_PERSO_FEATURE_STATUS_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[3].perso_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[3].perso_unblock_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[3].aid_value_len = 0x0c;
  uint8_t app_value_4[30] = {0xA0,0x00,0x00,0x00,0x63,0x50,0x4B,0x43,0x53,0x2D,0x31,0x35};
  memcpy(card_status->extended_card_status.card_info[0].app_info[3].aid_value, app_value_4, 12);
  card_status->extended_card_status.card_info[0].app_info[3].univ_pin = UIM_UNIV_PIN_PIN1_USED_V01;
  card_status->extended_card_status.card_info[0].app_info[3].pin1.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[3].pin1.pin_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[3].pin1.puk_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[3].pin2.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[3].pin2.pin_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[3].pin2.puk_retries = 0x00;
  t.inject(QMI_node);

  // Expect: QMI_CAT_GET_CONFIGURATION_REQ_V02
  QMI_node.update_default_qmi_cat_get_configuration_req();
  t.expect(QMI_node);

  // Inject: QMI_CAT_GET_CONFIGURATION_RESP_V02
  QMI_node.update_default_qmi_cat_get_configuration_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_CAT_SET_EVENT_REPORT_REQ_V02
  QMI_node.update_default_qmi_cat_set_event_report_req();
#ifdef RIL_UTF_L_MASTER
  QMI_node.set_qmi_cat_set_event_report_req_pc_evt_report_req_mask(0x0b7FFFFF);
#else
  QMI_node.set_qmi_cat_set_event_report_req_pc_evt_report_req_mask(0x037FFFFF);
#endif
  QMI_node.set_qmi_cat_set_event_report_req_slot_mask(CAT_SET_EVENT_REPORT_SLOT_1_V02);
  t.expect(QMI_node);

  // Inject: QMI_CAT_SET_EVENT_REPORT_RESP_V02
  QMI_node.update_default_qmi_cat_set_event_report_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_CAT_SET_EVENT_REPORT_REQ_V02
  QMI_node.update_default_qmi_cat_set_event_report_req();
  QMI_node.set_qmi_cat_set_event_report_req_pc_dec_evt_report_req_mask(0x00800000);
  QMI_node.set_qmi_cat_set_event_report_req_slot_mask(CAT_SET_EVENT_REPORT_SLOT_1_V02);
  t.expect(QMI_node);

  // Inject: QMI_CAT_SET_EVENT_REPORT_RESP_V02
  QMI_node.update_default_qmi_cat_set_event_report_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED
  RIL_node.update_default_ril_unsol_resp_radio_state_changed();
  t.expect(RIL_node);

  // Expect: ims_MsgId_UNSOL_RADIO_STATE_CHANGED
  IMS_node.update_default_ims_unsol_radio_state_changed(ims_RadioState_RADIO_STATE_OFF);
  t.expect(IMS_node);

  // Expect: LteDirectDiscovery_MsgId_UNSOL_RESPONSE_SERVICE_STATUS
  OEM_node.update_default_lte_direct_discovery_unsol_response_service_status();
  OEM_node.set_lte_direct_discovery_unsol_response_service_status_publish_allowed(0);
  OEM_node.set_lte_direct_discovery_unsol_response_service_status_subscribe_allowed(0);
  t.expect(OEM_node);

  // Expect: QMI_DMS_GET_DEVICE_CAP_REQ_V01
  QMI_node.update_default_qmi_dms_get_device_cap_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_DEVICE_CAP_RESP_V01
  QMI_node.update_default_qmi_dms_get_device_cap_resp_v01();
  QMI_node.set_default_qmi_dms_get_device_cap_resp_v01_device_capabilities();
  QMI_node.set_qmi_dms_get_device_cap_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_GET_CURRENT_PRL_INFO_REQ_V01
  QMI_node.update_default_qmi_dms_get_current_prl_info_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_CURRENT_PRL_INFO_RESP_V01
  QMI_node.update_default_qmi_dms_get_current_prl_info_resp_v01();
  QMI_node.set_qmi_dms_get_current_prl_info_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_SET_PRIMARY_CLIENT_REQ_V01
  QMI_node.update_default_qmi_wms_set_primary_client_req_v01();
  QMI_node.set_qmi_wms_set_primary_client_req_v01_primary_client(1);
  t.expect(QMI_node);

  // Inject: QMI_WMS_SET_PRIMARY_CLIENT_RESP_V01
  QMI_node.update_default_qmi_wms_set_primary_client_resp_v01();
  QMI_node.set_qmi_wms_set_primary_client_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_SET_ROUTES_REQ_V01
  QMI_node.update_default_qmi_wms_set_routes_req_v01();
//TODO:  Add sane values here and enable payload validation
  t.expect(QMI_node);

  // Inject: QMI_WMS_SET_ROUTES_RESP_V01
  QMI_node.update_default_qmi_wms_set_routes_resp_v01();
  QMI_node.set_qmi_wms_set_routes_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_SET_EVENT_REPORT_REQ_V01
  QMI_node.update_default_qmi_wms_set_event_report_req_v01();
//TODO:  Add sane values here and enable payload validation
  t.expect(QMI_node);

  // Inject: QMI_WMS_SET_EVENT_REPORT_RESP_V01
  QMI_node.update_default_qmi_wms_set_event_report_resp_v01();
  QMI_node.set_qmi_wms_set_event_report_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_wms_indication_register_req_v01();
  QMI_node.set_qmi_wms_indication_register_req_v01_reg_transport_layer_info_events(1);
  QMI_node.set_qmi_wms_indication_register_req_v01_reg_transport_layer_mwi_info_events(1);
  t.expect(QMI_node);

  // Inject: QMI_WMS_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_wms_indication_register_resp_v01();
  QMI_node.set_qmi_wms_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_GET_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_wms_get_indication_register_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_WMS_GET_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_wms_get_indication_register_resp_v01();
  QMI_node.set_qmi_wms_get_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  //TODO: Fill with sane values to send to qmi-ril
  t.inject(QMI_node);

  // Expect: QMI_UIM_GET_SERVICE_STATUS_REQ_V01
  uim_session_information_type_v01 session_info;
  memset(&session_info, 0, sizeof(session_info));
  session_info.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
  session_info.aid_len = 0;
  QMI_node.update_default_qmi_uim_get_service_status_req(session_info, 0x01);
  t.expect(QMI_node);

  // Inject: QMI_UIM_GET_SERVICE_STATUS_RESP_V01
  QMI_node.update_default_qmi_uim_get_service_status_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_uim_get_service_status_resp_fdn_status(UIM_FDN_STATUS_AVAILABLE_DISABLED_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_GET_SERVICE_STATUS_REQ_V01
  memset(&session_info, 0, sizeof(session_info));
  session_info.session_type = UIM_SESSION_TYPE_PRIMARY_1X_V01;
  session_info.aid_len = 0;
  QMI_node.update_default_qmi_uim_get_service_status_req(session_info, 0x01);
  t.expect(QMI_node);

  // Inject: QMI_UIM_GET_SERVICE_STATUS_RESP_V01
  QMI_node.update_default_qmi_uim_get_service_status_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_uim_get_service_status_resp_fdn_status(UIM_FDN_STATUS_AVAILABLE_DISABLED_V01);
  t.inject(QMI_node);

  /*
#ifdef RIL_UTF_L_MASTER
  // Expect: QMI_UIM_READ_TRANSPARENT_REQ_V01
  QMI_node.update_default_qmi_uim_read_transparent_reg();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_READ_TRANSPARENT_RESP_V01
  QMI_node.update_default_qmi_uim_read_transparent_resp(QMI_RESULT_FAILURE_V01, QMI_ERR_GENERAL_V01);
  t.inject(QMI_node);
#endif

  // Expect: QMI_UIM_READ_TRANSPARENT_REQ_V01
  QMI_node.update_default_qmi_uim_read_transparent_reg();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_READ_TRANSPARENT_RESP_V01
  QMI_node.update_default_qmi_uim_read_transparent_resp(QMI_RESULT_FAILURE_V01, QMI_ERR_GENERAL_V01);
  t.inject(QMI_node);
  */

  // Expect: QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ_V01
  QMI_node.update_default_qmi_wms_get_transport_layer_info_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_WMS_GET_TRANSPORT_LAYER_INFO_RESP_V01
  QMI_node.update_default_qmi_wms_get_transport_layer_info_resp_v01();
  QMI_node.set_qmi_wms_get_transport_layer_info_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_SET_AP_SW_VERSION_REQ_V01
  QMI_node.update_default_qmi_dms_set_ap_sw_version_req_v01();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_DMS_SET_AP_SW_VERSION_RESP_V01
  QMI_node.update_default_qmi_dms_set_ap_sw_version_resp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_GET_FILE_ATTRIBUTES_REQ_V01
  memset(&session_info, 0, sizeof(session_info));
  session_info.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
  session_info.aid_len = 0x00;
  uim_file_id_type_v01 file_id;
  memset(&file_id, 0, sizeof(file_id));
  uint8_t path[10] = {0x00,0x3F,0x10,0x7F,0x3A,0x5F};
  file_id.file_id = 0x4f30;
  file_id.path_len = 0x06;
  memcpy(file_id.path, path, 0x06);
  QMI_node.update_default_qmi_uim_get_file_attributes_req(session_info, file_id);
  t.expect(QMI_node);

  // Inject: QMI_UIM_GET_FILE_ATTRIBUTES_RESP_V01
  QMI_node.update_default_qmi_uim_get_file_attributes_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  uim_card_result_type_v01 card_result;
  card_result.sw1 = 0x90;
  card_result.sw2 = 0x00;
  QMI_node.set_qmi_uim_get_file_attributes_resp_card_result(card_result);
  QMI_node.set_qmi_uim_get_file_attribues_resp_file_status(UIM_FILE_STATUS_ACTIVATED_V01);
  uim_file_attributes_type_v01* file_attributes = QMI_node.set_qmi_uim_get_file_attributes_resp_file_attributes();
  file_attributes->file_size = 0x4c;
  file_attributes->file_id = 0x4f30;
  file_attributes->file_type = UIM_FILE_TYPE_LINEAR_FIXED_V01;
  file_attributes->rec_size = 0x4c;
  file_attributes->rec_count = 0x01;
  file_attributes->sec_read = UIM_SEC_CONDITION_SINGLE_COND_V01;
  file_attributes->sec_read_mask = 0x01;
  file_attributes->sec_write = UIM_SEC_CONDITION_SINGLE_COND_V01;
  file_attributes->sec_write_mask = 0x08;
  file_attributes->sec_increase = UIM_SEC_CONDITION_ALWAYS_V01;
  file_attributes->sec_increase_mask = 0x00;
  file_attributes->sec_deactivate = UIM_SEC_CONDITION_SINGLE_COND_V01;
  file_attributes->sec_deactivate_mask = 0x08;
  file_attributes->sec_activate = UIM_SEC_CONDITION_SINGLE_COND_V01;
  file_attributes->sec_activate_mask = 0x08;
  file_attributes->raw_value_len = 0x24;
  uint8_t temp_raw_value[40] = {0x62,0x22,0x82,0x05,0x42,0x21,0x00,0x4C,0x01,0x83,0x02,0x4F,0x30,0xA5,0x03,
    0xC0,0x01,0x00,0x8A,0x01,0x05,0x8B,0x03,0x6F,0x06,0x03,0x80,0x02,0x00,0x4C,0x81,0x02,0x00,0x60,0x88,0x00};
  memcpy(file_attributes->raw_value, temp_raw_value, 36);
  t.inject(QMI_node);

  // Expect: QMI_UIM_READ_RECORD_REQ_V01
  uim_read_record_req_msg_v01* read_req = QMI_node.update_default_qmi_uim_read_record_reg();
  read_req->session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
  read_req->session_information.aid_len = 0x00;
  read_req->file_id.file_id = 0x4f30;
  read_req->file_id.path_len = 0x06;
  uint8_t tmp_path[] = {0x00,0x3F,0x10,0x7F,0x3A,0x5F};
  memcpy(read_req->file_id.path, tmp_path, 0x06);
  read_req->read_record.record = 0x01;
  read_req->read_record.length = 0x4c;
  t.expect(QMI_node);

  // Inject: QMI_UIM_READ_RECORD_RESP_V01
  uim_read_record_resp_msg_v01* read_resp = QMI_node.update_default_qmi_uim_read_record_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  read_resp->read_result_valid = TRUE;
  read_resp->read_result.content_len = 0x4e;
  uint8_t tmp_content[] = {
    0x4C, 0x00, 0xA8, 0x23, 0xC0, 0x03, 0x4F, 0x3A, 0x01, 0xC5, 0x03, 0x4F, 0x09, 0x02, 0xC4, 0x03, 0x4F,
    0x11, 0x04, 0xC4, 0x03, 0x4F, 0x13, 0x05, 0xC4, 0x03, 0x4F, 0x15, 0x06, 0xCA, 0x03, 0x4F, 0x50, 0x09,
    0xC9, 0x03, 0x4F, 0x21, 0x0C, 0xAA, 0x0F, 0xC2, 0x03, 0x4F, 0x4A, 0x08, 0xC7, 0x03, 0x4F, 0x4B, 0x14,
    0xCB, 0x03, 0x4F, 0x3D, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
  };
  memcpy(read_resp->read_result.content, tmp_content, 0x4e);
  read_resp->card_result_valid = TRUE;
  read_resp->card_result.sw1 = 0x90;
  read_resp->card_result.sw2 = 0x00;
  t.inject(QMI_node);

  // Expect: QMI_UIM_REFRESH_REGISTER_REQ_V01
  uim_register_refresh_type_v01 reg_ref;
  QMI_node.update_default_qmi_uim_get_file_attributes_req(session_info, reg_ref);
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_REFRESH_REGISTER_RESP_V01
  QMI_node.update_default_qmi_uim_refresh_register_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_REFRESH_REGISTER_REQ_V01
  QMI_node.update_default_qmi_uim_get_file_attributes_req(session_info, reg_ref);
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_REFRESH_REGISTER_RESP_V01
  QMI_node.update_default_qmi_uim_refresh_register_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED
  RIL_node.update_default_ril_unsol_response_sim_status_changed();
  t.expect(RIL_node);

  /*
  // Expect: QMI_DMS_UIM_GET_ICCID_REQ_V01
  QMI_node.update_default_qmi_dms_get_iccid_req();
  t.expect(QMI_node);

  // Inject: QMI_DMS_UIM_GET_ICCID_RESP_V01
  char* uim_id = QMI_node.update_default_qmi_dms_get_iccid_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  char tmp_uim_id[30] = {0x38,0x39,0x30,0x30,0x33,0x31,0x30,0x30,0x30,0x38,0x35,0x38,0x36,0x35,0x31,0x33,0x31,0x31,0x34,0x36};
  memcpy(uim_id, tmp_uim_id, 20);
  t.inject(QMI_node);
  */

#ifdef RIL_UTF_L_MASTER
  // Expect: QMI_UIM_READ_TRANSPARENT_REQ_V01
  QMI_node.update_default_qmi_uim_read_transparent_reg();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_READ_TRANSPARENT_RESP_V01
  QMI_node.update_default_qmi_uim_read_transparent_resp(QMI_RESULT_FAILURE_V01, QMI_ERR_GENERAL_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_READ_TRANSPARENT_REQ_V01
  QMI_node.update_default_qmi_uim_read_transparent_reg();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_READ_TRANSPARENT_RESP_V01
  QMI_node.update_default_qmi_uim_read_transparent_resp(QMI_RESULT_SUCCESS_V01,QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  /*
  // Expect: QMI_DMS_UIM_GET_ICCID_REQ_V01
  QMI_node.update_default_qmi_dms_get_iccid_req();
  t.expect(QMI_node);

  // Inject: QMI_DMS_UIM_GET_ICCID_RESP_V01
  uim_id = QMI_node.update_default_qmi_dms_get_iccid_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  memcpy(uim_id, tmp_uim_id, 20);
  t.inject(QMI_node);
  */
#endif

  // Expect: QMI_DMS_GET_CURRENT_PRL_INFO_REQ_V01
  QMI_node.update_default_qmi_dms_get_current_prl_info_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_CURRENT_PRL_INFO_RESP_V01
  QMI_node.update_default_qmi_dms_get_current_prl_info_resp_v01();
  QMI_node.set_qmi_dms_get_current_prl_info_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Start set of unordered expects
  t.set_end();
}

test_result qmi_ril_msim_bootup_primary_002()
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  IMSTestnode IMS_node;
  OEMTestnode OEM_node;
  MSIMTestnode MSIM_node;
  DBTestnode DB_node;

  t.set_test_id(__func__);
  t.set_test_description("This test is necessary to boot qmi_ril"
    " for multi sim primary sim test cases with 4 sim apps on card 1"
    " and 1 sim app on card 2. Used for allow data test cases");
  t.set_test_attribute("ALL");
  // SINCE THIS IS A BOOT TEST, IT SHOULD NOT HAVE A DEFINED BOOT CASE
  // ALL BOOT TESTS NEED TO HAVE THEIR BOOT CASE SET TO NULL
  t.set_boot_case(NULL);

  // Set msim config
  property_set("persist.radio.multisim.config", "dsds");
  property_set("ro.baseband","dsda");
  ril_utf_set_primary_rild();

  // Start MSIM emulation threads
  utf_start_msim_threads();

  // Here begins the "real" test case
  // Expect: RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED
  RIL_node.update_default_ril_unsol_resp_radio_state_changed();
  t.expect(RIL_node);

  // Update qcril.db property table
  util_ril_utf_db_property_init(t, DB_node);

  // Expect: QMI_NAS_BIND_SUBSCRIPTION_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_bind_subscription_req_msg(NAS_PRIMARY_SUBSCRIPTION_V01);
  t.expect(QMI_node);

  // Inject: QMI_NAS_BIND_SUBSCRIPTION_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_bind_subscription_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_BIND_SUBSCRIPTION_REQ_V02
  QMI_node.update_default_qmi_voice_bind_subscription_req_msg(VOICE_SUBS_TYPE_PRIMARY_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_BIND_SUBSCRIPTION_RESP_V02
  QMI_node.update_default_qmi_voice_bind_subscription_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_PBM_BIND_SUBSCRIPTION_REQ_V01
  QMI_node.update_default_qmi_pbm_bind_subscription_req_msg(PBM_SUBS_TYPE_PRIMARY_V01);
  t.expect(QMI_node);

  // Inject: QMI_PBM_BIND_SUBSCRIPTION_RESP_V01
  QMI_node.update_default_qmi_pbm_bind_subscription_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_BIND_SUBSCRIPTION_REQ_V01
  QMI_node.update_default_qmi_wms_bind_subscription_req(WMS_PRIMARY_SUBSCRIPTION_V01);
  t.expect(QMI_node);

  // Inject: QMI_WMS_BIND_SUBSCRIPTION_RESP_V01
  QMI_node.update_default_qmi_wms_bind_subscription_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_BIND_SUBSCRIPTION_REQ_V01
  QMI_node.update_default_qmi_dms_bind_subscription_req_msg(DMS_PRIMARY_SUBS_V01);
  t.expect(QMI_node);

  // Inject: QMI_DMS_BIND_SUBSCRIPTION_RESP_V01
  QMI_node.update_default_qmi_dms_bind_subscription_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_LTE_BIND_SUBSCRIPTION_REQ_V01
  QMI_node.update_default_qmi_lte_bind_subscription_req_msg(LTE_PRIMARY_SUBSCRIPTION_V01);
  t.expect(QMI_node);

  // Inject: QMI_LTE_BIND_SUBSCRIPTION_RESP_V01
  QMI_node.update_default_qmi_lte_bind_subscription_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_reg_voice_privacy_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_ext_brst_intl_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_speech_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_handover_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_conference_events(0x01);
  //QMI_node.set_qmi_voice_ind_req_tty_info_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_tty_info_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_cc_result_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_additional_call_info_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_audio_rat_change_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_vice_dialog_event(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_conf_participants_events(0x01);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_INDICATION_REGISTER_REQ_MSG_V01
  QMI_node.update_default_qmi_dms_indication_register_req_v01();
  QMI_node.set_qmi_dms_ind_req_report_ims_capability(0x01);
  t.expect(QMI_node);

  // Inject: QMI_DMS_INDICATION_REGISTER_RESP_MSG_V01
  QMI_node.update_default_qmi_dms_indication_register_resp_v01();
  QMI_node.set_qmi_dms_indication_register_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_SET_EVENT_REPORT_REQ_V01
  QMI_node.update_default_qmi_dms_set_event_report_req_msg();
  QMI_node.set_qmi_dsm_set_event_report_req_report_oprt_mode_state(0x01);
  QMI_node.set_qmi_dsm_set_event_report_req_report_prl_init(0x01);
  QMI_node.set_qmi_dms_set_event_report_req_report_device_multisim_info(0x01);
  t.expect(QMI_node);

  // Inject: QMI_DMS_SET_EVENT_REPORT_RESP_V01
  QMI_node.update_default_qmi_dms_set_event_report_resp_msg();
  QMI_node.set_qmi_dms_set_event_report_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_GET_DEVICE_CAP_REQ_V01
  QMI_node.update_default_qmi_dms_get_device_cap_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_DEVICE_CAP_RESP_V01
  QMI_node.update_default_qmi_dms_get_device_cap_resp_v01();
  dms_multisim_capability_type_v01 multisim_capability;
  memset(&multisim_capability, 0, sizeof(multisim_capability));
  multisim_capability.max_subscriptions = 2;
  QMI_node.set_default_qmi_dms_get_device_cap_resp_v01_multisim_capability(multisim_capability);
  QMI_node.set_default_qmi_dms_get_device_cap_resp_v01_device_capabilities();
  QMI_node.set_qmi_dms_get_device_cap_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

#ifdef RIL_UTF_L_MASTER
  t.set_start();
  // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
  t.expect(QMI_node);

  // Inject:QMI_DMS_GET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
  QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_SHUTTING_DOWN_V01);
  QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_QMI_DMS_TRIGGER_MODEM_ACTIVITY_INFO_CALCULATION_REQ_V01
  QMI_node.update_default_qmi_dms_trigger_modem_activity_info_calculation_req(TRUE, 1);
  t.expect(QMI_node);

  // Inject: QMI_DMS_QMI_DMS_TRIGGER_MODEM_ACTIVITY_INFO_CALCULATION_RESP_V01
  QMI_node.update_default_qmi_dms_trigger_modem_activity_info_calculation_resp();
  t.inject(QMI_node);
  t.set_end();
#endif

  /*
  // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
  t.expect(QMI_node);

  // Inject:QMI_DMS_GET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
  QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_SHUTTING_DOWN_V01);
  QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);
  */

  // Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_V01
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_emergency_mode(0);
  mode_pref_mask_type_v01 mode_pref1 = QMI_NAS_RAT_MODE_PREF_UMTS_V01;
  mode_pref1 |= QMI_NAS_RAT_MODE_PREF_GSM_V01;
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(mode_pref1);
  // Set band_pref
  nas_band_pref_mask_type_v01 band_pref;
  band_pref |= QMI_NAS_BAND_CLASS_0_A_SYSTEM_V01;
  band_pref |= QMI_NAS_BAND_CLASS_0_B_AB_GSM850_V01;
  band_pref |= QMI_NAS_BAND_CLASS_1_ALL_BLOCKS_V01;
  band_pref |= QMI_NAS_BAND_CLASS_2_PLACEHOLDER_V01;
  band_pref |= QMI_NAS_BAND_CLASS_3_A_SYSTEM_V01;
  band_pref |= QMI_NAS_BAND_CLASS_4_ALL_BLOCKS_V01;
  band_pref |= QMI_NAS_BAND_CLASS_5_ALL_BLOCKS_V01;
  band_pref |= QMI_NAS_GSM_DCS_1800_BAND_V01;
  band_pref |= QMI_NAS_E_GSM_900_BAND_V01;
  band_pref |= QMI_NAS_P_GSM_900_BAND_V01;
  band_pref |= QMI_NAS_BAND_CLASS_6_V01;
  band_pref |= QMI_NAS_BAND_CLASS_7_V01;
  band_pref |= QMI_NAS_BAND_CLASS_8_V01;
  band_pref |= QMI_NAS_BAND_CLASS_9_V01;
  band_pref |= QMI_NAS_BAND_CLASS_10_V01;
  band_pref |= QMI_NAS_BAND_CLASS_11_V01;
  band_pref |= QMI_NAS_GSM_BAND_450_V01;
  band_pref |= QMI_NAS_GSM_BAND_480_V01;
  band_pref |= QMI_NAS_GSM_BAND_750_V01;
  band_pref |= QMI_NAS_GSM_BAND_850_V01;
  band_pref |= QMI_NAS_GSM_BAND_RAILWAYS_900_BAND_V01;
  band_pref |= QMI_NAS_GSM_BAND_PCS_1900_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_EU_J_CH_IMT_2100_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_US_PCS_1900_BAND_V01;
  band_pref |= QMI_NAS_EU_CH_DCS_1800_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_US_1700_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_JAPAN_800_BAND_V01;
  band_pref |= QMI_NAS_BAND_CLASS_12_V01;
  band_pref |= QMI_NAS_BAND_CLASS_14_V01;
  band_pref |= QMI_NAS_RESERVED_V01;
  band_pref |= QMI_NAS_BAND_CLASS_15_V01;
  band_pref |= QMI_NAS_WCDMA_EU_2600_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_EU_J_900_BAND_V01;
  band_pref |= QMI_NAS_WCDMA_J_1700_BAND_V01;
  band_pref |= QMI_NAS_BAND_CLASS_16_V01;
  band_pref |= QMI_NAS_BAND_CLASS_17_V01;
  band_pref |= QMI_NAS_BAND_CLASS_18_V01;
  band_pref |= QMI_NAS_BAND_CLASS_19_V01;
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_band_pref(band_pref);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_prl_pref(NAS_PRL_PREF_ANY_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_roam_pref(NAS_ROAMING_PREF_ANY_V01);
  // Set band_pref_ext
  uint64_t band_pref_ext;
  band_pref_ext |= E_UTRA_OPERATING_BAND_1_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_2_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_3_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_4_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_5_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_6_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_7_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_8_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_9_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_10_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_11_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_12_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_13_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_14_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_17_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_18_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_19_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_20_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_21_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_24_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_25_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_33_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_34_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_35_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_36_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_37_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_38_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_39_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_40_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_41_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_42_V01;
  band_pref_ext |= E_UTRA_OPERATING_BAND_43_V01;
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_band_pref_ext(band_pref_ext);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_net_sel_pref(NAS_NET_SEL_PREF_AUTOMATIC_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_PS_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_gw_acq_order_pref(NAS_GW_ACQ_ORDER_PREF_AUTOMATIC_V01);
  // Set tdscdma_band_pref
  nas_tdscdma_band_pref_mask_type_v01 tdscdma_band_pref;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_A_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_B_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_C_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_D_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_E_V01;
  tdscdma_band_pref |= NAS_TDSCDMA_BAND_F_V01;
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_tdscdma_band_pref(tdscdma_band_pref);
  QMI_node.set_default_qmi_nas_get_system_selection_preference_resp_acq_order();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_reg_restriction(NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_usage_setting(NAS_USAGE_VOICE_CENTRIC_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_CONFIG_SIG_INFO2_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_config_sig_info2_req_msg_v01();
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_cdma_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_cdma_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_gsm_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_sinr_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rsrp_delta(60);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rsrq_delta(20);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_snr_delta(40);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_ecio_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_rscp_delta(40);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_rssi_delta(5);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_sinr_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_wcdma_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_wcdma_rssi_delta(50);
  t.expect(QMI_node);

  // Inject: QMI_NAS_CONFIG_SIG_INFO2_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_config_sig_info2_resp_msg_v01();
  QMI_node.set_qmi_nas_config_sig_info2_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_SET_EVENT_REPORT_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_set_event_report_req_msg_v01();
  QMI_node.set_qmi_nas_set_event_report_req_report_reg_reject(TRUE);
  t.expect(QMI_node);

  // Inject: QMI_NAS_SET_EVENT_REPORT_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_set_event_report_resp_msg_v01();
  QMI_node.set_qmi_nas_set_event_report_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_indication_register_req_msg_v01();
  QMI_node.set_qmi_nas_indication_register_req_msg_reg_sys_sel_pref(1);
  QMI_node.set_qmi_nas_indication_register_req_msg_dual_standby_pref(1);
  QMI_node.set_qmi_nas_indication_register_req_msg_subscription_info(1);
  QMI_node.set_qmi_nas_indication_register_reg_msg_reg_rtre_cfg(1);
  QMI_node.set_qmi_nas_indication_register_req_msg_req_serving_system(0);
  QMI_node.set_qmi_nas_indication_register_req_msg_reg_network_time(0);
  QMI_node.set_qmi_nas_indication_register_reg_msg_reg_embms_status(0);
  QMI_node.set_qmi_nas_indication_register_req_msg_sig_info(0);
  QMI_node.set_qmi_nas_indication_register_reg_msg_err_rate(0);
  QMI_node.set_qmi_nas_indication_register_req_msg_reg_current_plmn_name(0);
  QMI_node.set_qmi_nas_indication_register_req_msg_reg_rf_band_info(0);
  t.expect(QMI_node);

  // Inject: QMI_NAS_INDICATION_REGISTER_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_indication_register_resp_msg_v01();
  QMI_node.set_qmi_nas_nas_indication_register_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_config_plmn_name_ind_reporting_req_msg_v01();
  QMI_node.set_qmi_nas_config_plmn_name_ind_reporting_req_msg_send_all_information(1);
  t.expect(QMI_node);

  // Inject: QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_config_plmn_name_ind_reporting_resp_msg_v01();
  QMI_node.set_qmi_nas_config_plmn_name_ind_reporting_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_CENTRALIZED_EONS_SUPPORT_STATUS_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_centralized_eons_support_status_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_CENTRALIZED_EONS_SUPPORT_STATUS_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_centralized_eons_support_status_resp_msg_v01();
  QMI_node.set_qmi_nas_get_centralized_eons_support_status_resp_msg_centralized_eons_supported(1);
  QMI_node.set_qmi_nas_get_centralized_eons_support_status_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_LTE_SIB16_NETWORK_TIME_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_lte_sib16_network_time_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_LTE_SIB16_NETWORK_TIME_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_lte_sib16_network_time_resp_msg_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
  QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_SHUTTING_DOWN_V01);
  QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_PBM_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_pbm_indication_register_req_msg();
  QMI_node.set_qmi_pbm_indication_register_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01);
  t.expect(QMI_node);

  // Inject: QMI_PBM_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_pbm_indication_register_resp_msg();
  QMI_node.set_qmi_pbm_indication_register_resp_v01_reg_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01);
  QMI_node.set_qmi_pbm_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_PBM_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_pbm_indication_register_req_msg();
  QMI_node.set_qmi_pbm_indication_register_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01|PBM_REG_RECORD_UPDATE_EVENTS_V01|PBM_REG_PHONEBOOK_READY_EVENTS_V01);
  t.expect(QMI_node);

  // Inject: QMI_PBM_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_pbm_indication_register_resp_msg();
  QMI_node.set_qmi_pbm_indication_register_resp_v01_reg_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01|PBM_REG_RECORD_UPDATE_EVENTS_V01|PBM_REG_PHONEBOOK_READY_EVENTS_V01);
  QMI_node.set_qmi_pbm_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_LTE_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_lte_indication_register_req_msg();
  QMI_node.set_qmi_lte_indication_register_req_indication_bitmask(
          QMI_LTE_DISC_NOTIFICATION_IND_MASK_V01 |
          QMI_LTE_DISC_BROADCAST_NOTIFICATION_IND_MASK_V01 |
          QMI_LTE_DISC_MATCH_NOTIFICATION_IND_MASK_V01 |
          QMI_LTE_DISC_PSK_EXPIRED_IND_MASK_V01);
  t.expect(QMI_node);

  // Inject: QMI_LTE_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_lte_indication_register_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Start set of unordered expects
  t.set_start();

  // Expect: QMI_UIM_EVENT_REG_REQ_V01
  QMI_node.update_default_qmi_uim_event_reg_req(0x07);
  t.expect(QMI_node);

  // Inject: QMI_UIM_EVENT_REG_RESP_V01
  QMI_node.update_default_qmi_uim_event_reg_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_GET_CARD_STATUS_REQ_V01
  QMI_node.update_default_qmi_uim_get_card_status_req();
  QMI_node.set_qmi_uim_get_card_status_req_extended_card_status(0x01);
  t.expect(QMI_node);

  // Inject: QMI_UIM_GET_CARD_STATUS_RESP_V01
  uim_get_card_status_resp_msg_v01* card_status = QMI_node.update_default_qmi_uim_get_card_status_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  card_status->sim_busy_status_valid = TRUE;
  card_status->sim_busy_status_len = 0x01;
  card_status->sim_busy_status[0] = 0x00;
  card_status->extended_hot_swap_valid = TRUE;
  card_status->extended_hot_swap_len = 0x01;
  card_status->extended_hot_swap[0] = UIM_HOT_SWAP_NOT_SUPPORTED_V01;
  card_status->extended_card_status_valid = TRUE;
  card_status->extended_card_status.index_gw_len = 0x01;
  card_status->extended_card_status.index_1x_len = 0x01;
  card_status->extended_card_status.index_1x[0] = 0x0002;
  card_status->extended_card_status.card_info_len = 0x01;
  card_status->extended_card_status.card_info[0].card_state = UIM_EXTENDED_CARD_STATE_PRESENT_V01;
  card_status->extended_card_status.card_info[0].upin.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].upin.pin_retries = 0x00;
  card_status->extended_card_status.card_info[0].upin.puk_retries = 0x00;
  card_status->extended_card_status.card_info[0].error_code = UIM_CARD_ERROR_CODE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info_len = 0x04;
  // App slot 0
  card_status->extended_card_status.card_info[0].app_info[0].app_type = UIM_APP_TYPE_USIM_V01;
  card_status->extended_card_status.card_info[0].app_info[0].app_state = UIM_APP_STATE_READY_V01;
  card_status->extended_card_status.card_info[0].app_info[0].perso_state = UIM_PERSO_STATE_READY_V01;
  card_status->extended_card_status.card_info[0].app_info[0].perso_feature = UIM_PERSO_FEATURE_STATUS_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[0].perso_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[0].perso_unblock_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[0].aid_value_len = 0x10;
  uint8_t app_value_1[30] = {0xA0,0x00,0x00,0x00,0x87,0x10,0x02,0xF3,0x10,0xFF,0xFF,0x89,0x08,0x00,0x00,0xFF};
  memcpy(card_status->extended_card_status.card_info[0].app_info[0].aid_value, app_value_1, 16);
  card_status->extended_card_status.card_info[0].app_info[0].univ_pin = UIM_UNIV_PIN_PIN1_USED_V01;
  card_status->extended_card_status.card_info[0].app_info[0].pin1.pin_state = UIM_PIN_STATE_DISABLED_V01;
  card_status->extended_card_status.card_info[0].app_info[0].pin1.pin_retries = 0x03;
  card_status->extended_card_status.card_info[0].app_info[0].pin1.puk_retries = 0x0A;
  card_status->extended_card_status.card_info[0].app_info[0].pin2.pin_state = UIM_PIN_STATE_ENABLED_NOT_VERIFIED_V01;
  card_status->extended_card_status.card_info[0].app_info[0].pin2.pin_retries = 0x03;
  card_status->extended_card_status.card_info[0].app_info[0].pin2.puk_retries = 0x0A;
  // App slot 1
  card_status->extended_card_status.card_info[0].app_info[1].app_type = UIM_APP_TYPE_ISIM_V01;
  card_status->extended_card_status.card_info[0].app_info[1].app_state = UIM_APP_STATE_DETECTED_V01;
  card_status->extended_card_status.card_info[0].app_info[1].perso_state = UIM_PERSO_STATE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[1].perso_feature = UIM_PERSO_FEATURE_STATUS_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[1].perso_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[1].perso_unblock_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[1].aid_value_len = 0x10;
  uint8_t app_value_2[30] = {0xA0,0x00,0x00,0x00,0x87,0x10,0x04,0xF3,0x10,0xFF,0xFF,0x89,0x08,0x00,0x00,0xFF};
  memcpy(card_status->extended_card_status.card_info[0].app_info[1].aid_value, app_value_2, 16);
  card_status->extended_card_status.card_info[0].app_info[1].univ_pin = UIM_UNIV_PIN_PIN1_USED_V01;
  card_status->extended_card_status.card_info[0].app_info[1].pin1.pin_state = UIM_PIN_STATE_DISABLED_V01;
  card_status->extended_card_status.card_info[0].app_info[1].pin1.pin_retries = 0x03;
  card_status->extended_card_status.card_info[0].app_info[1].pin1.puk_retries = 0x0A;
  card_status->extended_card_status.card_info[0].app_info[1].pin2.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[1].pin2.pin_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[1].pin2.puk_retries = 0x00;
  // App slot 2
  card_status->extended_card_status.card_info[0].app_info[2].app_type = UIM_APP_TYPE_CSIM_V01;
  card_status->extended_card_status.card_info[0].app_info[2].app_state = UIM_APP_STATE_READY_V01;
  card_status->extended_card_status.card_info[0].app_info[2].perso_state = UIM_PERSO_STATE_READY_V01;
  card_status->extended_card_status.card_info[0].app_info[2].perso_feature = UIM_PERSO_FEATURE_STATUS_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[2].perso_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[2].perso_unblock_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[2].aid_value_len = 0x10;
  uint8_t app_value_3[30] = {0xA0,0x00,0x00,0x03,0x43,0x10,0x02,0xF3,0x10,0xFF,0xFF,0x89,0x02,0x00,0x00,0xFF};
  memcpy(card_status->extended_card_status.card_info[0].app_info[2].aid_value, app_value_3, 16);
  card_status->extended_card_status.card_info[0].app_info[2].univ_pin = UIM_UNIV_PIN_PIN1_USED_V01;
  card_status->extended_card_status.card_info[0].app_info[2].pin1.pin_state = UIM_PIN_STATE_DISABLED_V01;
  card_status->extended_card_status.card_info[0].app_info[2].pin1.pin_retries = 0x03;
  card_status->extended_card_status.card_info[0].app_info[2].pin1.puk_retries = 0x0A;
  card_status->extended_card_status.card_info[0].app_info[2].pin2.pin_state = UIM_PIN_STATE_ENABLED_NOT_VERIFIED_V01;
  card_status->extended_card_status.card_info[0].app_info[2].pin2.pin_retries = 0x03;
  card_status->extended_card_status.card_info[0].app_info[2].pin2.puk_retries = 0x0A;
  // App slot 3
  card_status->extended_card_status.card_info[0].app_info[3].app_type = UIM_APP_TYPE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[3].app_state = UIM_APP_STATE_DETECTED_V01;
  card_status->extended_card_status.card_info[0].app_info[3].perso_state = UIM_PERSO_STATE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[3].perso_feature = UIM_PERSO_FEATURE_STATUS_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[3].perso_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[3].perso_unblock_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[3].aid_value_len = 0x0c;
  uint8_t app_value_4[30] = {0xA0,0x00,0x00,0x00,0x63,0x50,0x4B,0x43,0x53,0x2D,0x31,0x35};
  memcpy(card_status->extended_card_status.card_info[0].app_info[3].aid_value, app_value_4, 12);
  card_status->extended_card_status.card_info[0].app_info[3].univ_pin = UIM_UNIV_PIN_PIN1_USED_V01;
  card_status->extended_card_status.card_info[0].app_info[3].pin1.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[3].pin1.pin_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[3].pin1.puk_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[3].pin2.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
  card_status->extended_card_status.card_info[0].app_info[3].pin2.pin_retries = 0x00;
  card_status->extended_card_status.card_info[0].app_info[3].pin2.puk_retries = 0x00;
  t.inject(QMI_node);

  // Expect: QMI_CAT_GET_CONFIGURATION_REQ_V02
  QMI_node.update_default_qmi_cat_get_configuration_req();
  t.expect(QMI_node);

  // Inject: QMI_CAT_GET_CONFIGURATION_RESP_V02
  QMI_node.update_default_qmi_cat_get_configuration_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_CAT_SET_EVENT_REPORT_REQ_V02
  QMI_node.update_default_qmi_cat_set_event_report_req();
#ifdef RIL_UTF_L_MASTER
  QMI_node.set_qmi_cat_set_event_report_req_pc_evt_report_req_mask(0x0b7FFFFF);
#else
  QMI_node.set_qmi_cat_set_event_report_req_pc_evt_report_req_mask(0x037FFFFF);
#endif
  QMI_node.set_qmi_cat_set_event_report_req_slot_mask(CAT_SET_EVENT_REPORT_SLOT_1_V02);
  t.expect(QMI_node);

  // Inject: QMI_CAT_SET_EVENT_REPORT_RESP_V02
  QMI_node.update_default_qmi_cat_set_event_report_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_CAT_SET_EVENT_REPORT_REQ_V02
  QMI_node.update_default_qmi_cat_set_event_report_req();
  QMI_node.set_qmi_cat_set_event_report_req_pc_dec_evt_report_req_mask(0x00800000);
  QMI_node.set_qmi_cat_set_event_report_req_slot_mask(CAT_SET_EVENT_REPORT_SLOT_1_V02);
  t.expect(QMI_node);

  // Inject: QMI_CAT_SET_EVENT_REPORT_RESP_V02
  QMI_node.update_default_qmi_cat_set_event_report_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_IMSP_SET_EVENT_REPORT_REQ_V01
  QMI_node.update_default_qmi_imsp_set_event_report_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_IMSP_SET_EVENT_REPORT_RESP_V01
  QMI_node.update_default_qmi_imsp_set_event_report_resp_v01();
  QMI_node.set_qmi_imsp_set_event_report_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_IMS_SETTINGS_GET_REG_MGR_CONFIG_REQ_V01
  QMI_node.update_default_qmi_ims_settings_get_reg_mgr_config_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_GET_REG_MGR_CONFIG_RSP_V01
  QMI_node.update_default_qmi_ims_settings_get_reg_mgr_config_rsp_v01();
  QMI_node.set_qmi_ims_settings_get_reg_mgr_config_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_REQ_V01
  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_RSP_V01
  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_rsp_v01();
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_IMSA_IND_REG_REQ_V01
  QMI_node.update_default_qmi_imsa_ind_reg_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_IMSA_IND_REG_RSP_V01
  QMI_node.update_default_qmi_imsa_ind_reg_rsp_v01();
  QMI_node.set_qmi_imsa_ind_req_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_IMS_SETTINGS_CONFIG_IND_REG_REQ_V01
  QMI_node.update_default_qmi_ims_config_ind_reg_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_CONFIG_IND_REG_RSP_V01
  QMI_node.update_default_qmi_ims_config_ind_reg_rsp_v01();
  QMI_node.set_qmi_ims_config_ind_req_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_GET_DEVICE_CAP_REQ_V01
  QMI_node.update_default_qmi_dms_get_device_cap_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_DEVICE_CAP_RESP_V01
  QMI_node.update_default_qmi_dms_get_device_cap_resp_v01();
  memset(&multisim_capability, 0, sizeof(multisim_capability));
  multisim_capability.max_subscriptions = 2;
  QMI_node.set_default_qmi_dms_get_device_cap_resp_v01_multisim_capability(multisim_capability);
  QMI_node.set_default_qmi_dms_get_device_cap_resp_v01_device_capabilities();
  QMI_node.set_qmi_dms_get_device_cap_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  //QMI_NAS_GET_SUBSCRIPTION_INFO_REQ_MSG_V01
  util_ril_utf_get_subscription_info(t, QMI_node, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, TRUE, NAS_PRIORITY_SUBSCRIPTION_TRUE_V01, TRUE, NAS_SUBSCRIPTION_ACTIVE_V01, TRUE, 0, TRUE, 0, FALSE, 0, FALSE, 0);

  // Expect: QMI_DMS_GET_CURRENT_PRL_INFO_REQ_V01
  QMI_node.update_default_qmi_dms_get_current_prl_info_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_CURRENT_PRL_INFO_RESP_V01
  QMI_node.update_default_qmi_dms_get_current_prl_info_resp_v01();
  QMI_node.set_qmi_dms_get_current_prl_info_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED
  RIL_node.update_default_ril_unsol_resp_radio_state_changed();
  t.expect(RIL_node);

  // Expect: QMI_DMS_GET_DEVICE_MFR_REQ_V01
  QMI_node.update_default_qmi_dms_get_device_mfr_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_DEVICE_MFR_RESP_V01
  QMI_node.update_default_qmi_dms_get_device_mfr_resp_v01();
  QMI_node.set_default_qmi_dms_get_device_mfr_resp_v01_device_manufacturer();
  QMI_node.set_qmi_dms_get_device_mfr_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_GET_CURRENT_PRL_INFO_REQ_V01
  QMI_node.update_default_qmi_dms_get_current_prl_info_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_CURRENT_PRL_INFO_RESP_V01
  QMI_node.update_default_qmi_dms_get_current_prl_info_resp_v01();
  QMI_node.set_qmi_dms_get_current_prl_info_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_READ_TRANSPARENT_REQ_V01
  QMI_node.update_default_qmi_uim_read_transparent_reg();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_READ_TRANSPARENT_RESP_V01
  QMI_node.update_default_qmi_uim_read_transparent_resp(QMI_RESULT_SUCCESS_V01,QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  /*
#ifdef RIL_UTF_L_MASTER
  // Expect: QMI_UIM_READ_TRANSPARENT_REQ_V01
  QMI_node.update_default_qmi_uim_read_transparent_reg();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_READ_TRANSPARENT_RESP_V01
  QMI_node.update_default_qmi_uim_read_transparent_resp(QMI_RESULT_FAILURE_V01, QMI_ERR_GENERAL_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_READ_TRANSPARENT_REQ_V01
  QMI_node.update_default_qmi_uim_read_transparent_reg();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_READ_TRANSPARENT_RESP_V01
  QMI_node.update_default_qmi_uim_read_transparent_resp(QMI_RESULT_FAILURE_V01, QMI_ERR_GENERAL_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_UIM_GET_ICCID_REQ_V01
  QMI_node.update_default_qmi_dms_get_iccid_req();
  t.expect(QMI_node);

  // Inject: QMI_DMS_UIM_GET_ICCID_RESP_V01
  uim_id = QMI_node.update_default_qmi_dms_get_iccid_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  memcpy(uim_id, tmp_uim_id, 20);
  t.inject(QMI_node);

  // Expect: QMI_DMS_UIM_GET_ICCID_REQ_V01
  QMI_node.update_default_qmi_dms_get_iccid_req();
  t.expect(QMI_node);

  // Inject: QMI_DMS_UIM_GET_ICCID_RESP_V01
  uim_id = QMI_node.update_default_qmi_dms_get_iccid_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  memcpy(uim_id, tmp_uim_id, 20);
  t.inject(QMI_node);
#endif
  */

  // Expect: QMI_WMS_SET_ROUTES_REQ_V01
  QMI_node.update_default_qmi_wms_set_routes_req_v01();
//TODO:  Add sane values here and enable payload validation
  t.expect(QMI_node);

  // Inject: QMI_WMS_SET_ROUTES_RESP_V01
  QMI_node.update_default_qmi_wms_set_routes_resp_v01();
  QMI_node.set_qmi_wms_set_routes_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_SET_EVENT_REPORT_REQ_V01
  QMI_node.update_default_qmi_wms_set_event_report_req_v01();
//TODO:  Add sane values here and enable payload validation
  t.expect(QMI_node);

  // Inject: QMI_WMS_SET_EVENT_REPORT_RESP_V01
  QMI_node.update_default_qmi_wms_set_event_report_resp_v01();
  QMI_node.set_qmi_wms_set_event_report_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_wms_indication_register_req_v01();
  QMI_node.set_qmi_wms_indication_register_req_v01_reg_transport_layer_info_events(1);
  QMI_node.set_qmi_wms_indication_register_req_v01_reg_transport_layer_mwi_info_events(1);
  t.expect(QMI_node);

  // Inject: QMI_WMS_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_wms_indication_register_resp_v01();
  QMI_node.set_qmi_wms_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_GET_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_wms_get_indication_register_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_WMS_GET_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_wms_get_indication_register_resp_v01();
  QMI_node.set_qmi_wms_get_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  //TODO: Fill with sane values to send to qmi-ril
  t.inject(QMI_node);

  // Expect: QMI_WMS_SET_PRIMARY_CLIENT_REQ_V01
  QMI_node.update_default_qmi_wms_set_primary_client_req_v01();
  QMI_node.set_qmi_wms_set_primary_client_req_v01_primary_client(1);
  t.expect(QMI_node);

  // Inject: QMI_WMS_SET_PRIMARY_CLIENT_RESP_V01
  QMI_node.update_default_qmi_wms_set_primary_client_resp_v01();
  QMI_node.set_qmi_wms_set_primary_client_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ_V01
  QMI_node.update_default_qmi_wms_get_transport_layer_info_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_WMS_GET_TRANSPORT_LAYER_INFO_RESP_V01
  QMI_node.update_default_qmi_wms_get_transport_layer_info_resp_v01();
  QMI_node.set_qmi_wms_get_transport_layer_info_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_GET_SERVICE_STATUS_REQ_V01
  uim_session_information_type_v01 session_info;
  memset(&session_info, 0, sizeof(session_info));
  session_info.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
  session_info.aid_len = 0;
  QMI_node.update_default_qmi_uim_get_service_status_req(session_info, 0x01);
  t.expect(QMI_node);

  // Inject: QMI_UIM_GET_SERVICE_STATUS_RESP_V01
  QMI_node.update_default_qmi_uim_get_service_status_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_uim_get_service_status_resp_fdn_status(UIM_FDN_STATUS_AVAILABLE_DISABLED_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_GET_SERVICE_STATUS_REQ_V01
  memset(&session_info, 0, sizeof(session_info));
  session_info.session_type = UIM_SESSION_TYPE_PRIMARY_1X_V01;
  session_info.aid_len = 0;
  QMI_node.update_default_qmi_uim_get_service_status_req(session_info, 0x01);
  t.expect(QMI_node);

  // Inject: QMI_UIM_GET_SERVICE_STATUS_RESP_V01
  QMI_node.update_default_qmi_uim_get_service_status_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_uim_get_service_status_resp_fdn_status(UIM_FDN_STATUS_AVAILABLE_DISABLED_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_SET_AP_SW_VERSION_REQ_V01
  QMI_node.update_default_qmi_dms_set_ap_sw_version_req_v01();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_DMS_SET_AP_SW_VERSION_RESP_V01
  QMI_node.update_default_qmi_dms_set_ap_sw_version_resp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_GET_FILE_ATTRIBUTES_REQ_V01
  memset(&session_info, 0, sizeof(session_info));
  session_info.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
  session_info.aid_len = 0x00;
  uim_file_id_type_v01 file_id;
  memset(&file_id, 0, sizeof(file_id));
  uint8_t path[10] = {0x00,0x3F,0x10,0x7F,0x3A,0x5F};
  file_id.file_id = 0x4f30;
  file_id.path_len = 0x06;
  memcpy(file_id.path, path, 0x06);
  QMI_node.update_default_qmi_uim_get_file_attributes_req(session_info, file_id);
  t.expect(QMI_node);

  // Inject: QMI_UIM_GET_FILE_ATTRIBUTES_RESP_V01
  QMI_node.update_default_qmi_uim_get_file_attributes_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  uim_card_result_type_v01 card_result;
  card_result.sw1 = 0x90;
  card_result.sw2 = 0x00;
  QMI_node.set_qmi_uim_get_file_attributes_resp_card_result(card_result);
  QMI_node.set_qmi_uim_get_file_attribues_resp_file_status(UIM_FILE_STATUS_ACTIVATED_V01);
  uim_file_attributes_type_v01* file_attributes = QMI_node.set_qmi_uim_get_file_attributes_resp_file_attributes();
  file_attributes->file_size = 0x4c;
  file_attributes->file_id = 0x4f30;
  file_attributes->file_type = UIM_FILE_TYPE_LINEAR_FIXED_V01;
  file_attributes->rec_size = 0x4c;
  file_attributes->rec_count = 0x01;
  file_attributes->sec_read = UIM_SEC_CONDITION_SINGLE_COND_V01;
  file_attributes->sec_read_mask = 0x01;
  file_attributes->sec_write = UIM_SEC_CONDITION_SINGLE_COND_V01;
  file_attributes->sec_write_mask = 0x08;
  file_attributes->sec_increase = UIM_SEC_CONDITION_ALWAYS_V01;
  file_attributes->sec_increase_mask = 0x00;
  file_attributes->sec_deactivate = UIM_SEC_CONDITION_SINGLE_COND_V01;
  file_attributes->sec_deactivate_mask = 0x08;
  file_attributes->sec_activate = UIM_SEC_CONDITION_SINGLE_COND_V01;
  file_attributes->sec_activate_mask = 0x08;
  file_attributes->raw_value_len = 0x24;
  uint8_t temp_raw_value[40] = {0x62,0x22,0x82,0x05,0x42,0x21,0x00,0x4C,0x01,0x83,0x02,0x4F,0x30,0xA5,0x03,
    0xC0,0x01,0x00,0x8A,0x01,0x05,0x8B,0x03,0x6F,0x06,0x03,0x80,0x02,0x00,0x4C,0x81,0x02,0x00,0x60,0x88,0x00};
  memcpy(file_attributes->raw_value, temp_raw_value, 36);
  t.inject(QMI_node);

  // Expect: QMI_UIM_READ_RECORD_REQ_V01
  uim_read_record_req_msg_v01* read_req = QMI_node.update_default_qmi_uim_read_record_reg();
  read_req->session_information.session_type = UIM_SESSION_TYPE_PRIMARY_GW_V01;
  read_req->session_information.aid_len = 0x00;
  read_req->file_id.file_id = 0x4f30;
  read_req->file_id.path_len = 0x06;
  uint8_t tmp_path[] = {0x00,0x3F,0x10,0x7F,0x3A,0x5F};
  memcpy(read_req->file_id.path, tmp_path, 0x06);
  read_req->read_record.record = 0x01;
  read_req->read_record.length = 0x4c;
  t.expect(QMI_node);

  // Inject: QMI_UIM_READ_RECORD_RESP_V01
  uim_read_record_resp_msg_v01* read_resp = QMI_node.update_default_qmi_uim_read_record_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  read_resp->read_result_valid = TRUE;
  read_resp->read_result.content_len = 0x4e;
  uint8_t tmp_content[] = {
    0x4C, 0x00, 0xA8, 0x23, 0xC0, 0x03, 0x4F, 0x3A, 0x01, 0xC5, 0x03, 0x4F, 0x09, 0x02, 0xC4, 0x03, 0x4F,
    0x11, 0x04, 0xC4, 0x03, 0x4F, 0x13, 0x05, 0xC4, 0x03, 0x4F, 0x15, 0x06, 0xCA, 0x03, 0x4F, 0x50, 0x09,
    0xC9, 0x03, 0x4F, 0x21, 0x0C, 0xAA, 0x0F, 0xC2, 0x03, 0x4F, 0x4A, 0x08, 0xC7, 0x03, 0x4F, 0x4B, 0x14,
    0xCB, 0x03, 0x4F, 0x3D, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
  };
  memcpy(read_resp->read_result.content, tmp_content, 0x4e);
  read_resp->card_result_valid = TRUE;
  read_resp->card_result.sw1 = 0x90;
  read_resp->card_result.sw2 = 0x00;
  t.inject(QMI_node);

  // Expect: QMI_UIM_REFRESH_REGISTER_REQ_V01
  uim_register_refresh_type_v01 reg_ref;
  QMI_node.update_default_qmi_uim_get_file_attributes_req(session_info, reg_ref);
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_REFRESH_REGISTER_RESP_V01
  QMI_node.update_default_qmi_uim_refresh_register_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_UIM_REFRESH_REGISTER_REQ_V01
  QMI_node.update_default_qmi_uim_get_file_attributes_req(session_info, reg_ref);
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_UIM_REFRESH_REGISTER_RESP_V01
  QMI_node.update_default_qmi_uim_refresh_register_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED
  RIL_node.update_default_ril_unsol_response_sim_status_changed();
  t.expect(RIL_node);

  // Expect: QMI_IMS_SETTINGS_GET_SUPPORTED_MSGS_REQ_V01
  QMI_node.update_default_qmi_ims_settings_get_supported_msgs_req();
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_GET_SUPPORTED_MSGS_RESP_V01
  QMI_node.update_default_qmi_ims_settings_get_supported_msgs_resp();
  t.inject(QMI_node);

  t.set_end();

  // Inject: RIL_REQUEST_GET_SIM_STATUS
  RIL_node.update_default_ril_request_get_sim_status();
  t.inject(RIL_node);

  /*
  // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
  QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_LOW_POWER_V01);
  QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);
  */

  //Expect: QMI_UIM_GET_LABEL_REQ
  uim_get_label_req_msg_v01 *label_req1 = QMI_node.update_default_qmi_uim_get_label_req();
  label_req1->application_information.slot = (uim_slot_enum_v01)0x01;
  label_req1->application_information.aid_len = 0x10;
  memcpy(label_req1->application_information.aid, app_value_1, 16);//sizeof(label_req->application_information.aid));
  t.expect(QMI_node);

  //Inject: QMI_UIM_GET_LABEL_RESP
  uim_get_label_resp_msg_v01 *response1 = QMI_node.update_default_qmi_uim_get_label_resp();
  response1->resp.result = QMI_RESULT_SUCCESS_V01;
  strcpy(response1->label, "USIM");
  response1->label_len = 4;
  response1->label_valid = TRUE;
  t.inject(QMI_node);

  //Expect: QMI_UIM_GET_LABEL_REQ
  uim_get_label_req_msg_v01 *label_req2 = QMI_node.update_default_qmi_uim_get_label_req();
  label_req2->application_information.slot = (uim_slot_enum_v01)0x01;
  label_req2->application_information.aid_len = 0x10;
  memcpy(label_req2->application_information.aid, app_value_2, 16);//sizeof(label_req->application_information.aid));
  t.expect(QMI_node);

  //Inject: QMI_UIM_GET_LABEL_RESP
  uim_get_label_resp_msg_v01 *response2 = QMI_node.update_default_qmi_uim_get_label_resp();
  response2->resp.result = QMI_RESULT_SUCCESS_V01;
  strcpy(response2->label, "ISIM");
  response2->label_len = 4;
  response2->label_valid = TRUE;
  t.inject(QMI_node);

  //Expect: QMI_UIM_GET_LABEL_REQ
  uim_get_label_req_msg_v01 *label_req3 = QMI_node.update_default_qmi_uim_get_label_req();
  label_req3->application_information.slot = (uim_slot_enum_v01)0x01;
  label_req3->application_information.aid_len = 0x10;
  memcpy(label_req3->application_information.aid, app_value_3, 16);//sizeof(label_req->application_information.aid));
  t.expect(QMI_node);

  //Inject: QMI_UIM_GET_LABEL_RESP
  uim_get_label_resp_msg_v01 *response3 = QMI_node.update_default_qmi_uim_get_label_resp();
  response3->resp.result = QMI_RESULT_SUCCESS_V01;
  strcpy(response3->label, "CSIM");
  response3->label_len = 4;
  response3->label_valid = TRUE;
  t.inject(QMI_node);

  //Expect: QMI_UIM_GET_LABEL_REQ
  uim_get_label_req_msg_v01 *label_req4 = QMI_node.update_default_qmi_uim_get_label_req();
  label_req4->application_information.slot = (uim_slot_enum_v01)0x01;
  label_req4->application_information.aid_len = 0x0C;
  memcpy(label_req4->application_information.aid, app_value_4, 16);//sizeof(label_req->application_information.aid));
  t.expect(QMI_node);

  //Inject: QMI_UIM_GET_LABEL_RESP
  uim_get_label_resp_msg_v01 *response4 = QMI_node.update_default_qmi_uim_get_label_resp();
  response4->resp.result = QMI_RESULT_SUCCESS_V01;
  strcpy(response4->label, "PKCS");
  response4->label_len = 4;
  response4->label_valid = TRUE;
  t.inject(QMI_node);

  // Expect: RIL_REQUEST_GET_SIM_STATUS (resp)
  RIL_CardStatus_v6* status_ptr = RIL_node.update_default_ril_request_get_sim_status_resp();
  status_ptr->card_state = RIL_CARDSTATE_PRESENT;
  status_ptr->universal_pin_state = RIL_PINSTATE_UNKNOWN;
  status_ptr->gsm_umts_subscription_app_index = 0;
  status_ptr->cdma_subscription_app_index = 2;
  status_ptr->ims_subscription_app_index = 1;
  status_ptr->num_applications = 4;

  status_ptr->applications[0].app_type = RIL_APPTYPE_USIM;
  status_ptr->applications[0].app_state = RIL_APPSTATE_READY;
  status_ptr->applications[0].perso_substate = RIL_PERSOSUBSTATE_READY;
  status_ptr->applications[0].pin1 = RIL_PINSTATE_DISABLED;
  status_ptr->applications[0].pin2 = RIL_PINSTATE_ENABLED_NOT_VERIFIED;
  status_ptr->applications[0].aid_ptr = (char *)malloc(33);
  strcpy(status_ptr->applications[0].aid_ptr, "a0000000871002f310ffff89080000ff");
  status_ptr->applications[0].app_label_ptr = (char *)malloc(5);
  strcpy(status_ptr->applications[0].app_label_ptr, "USIM");

  status_ptr->applications[1].app_type = RIL_APPTYPE_ISIM;
  status_ptr->applications[1].app_state = RIL_APPSTATE_READY;
  status_ptr->applications[1].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
  status_ptr->applications[1].pin1 = RIL_PINSTATE_DISABLED;
  status_ptr->applications[1].pin2 = RIL_PINSTATE_UNKNOWN;
  status_ptr->applications[1].aid_ptr = (char *)malloc(33);
  strcpy(status_ptr->applications[1].aid_ptr, "a0000000871004f310ffff89080000ff");
  status_ptr->applications[1].app_label_ptr = (char *)malloc(5);
  strcpy(status_ptr->applications[1].app_label_ptr, "ISIM");

  status_ptr->applications[2].app_type = RIL_APPTYPE_CSIM;
  status_ptr->applications[2].app_state = RIL_APPSTATE_READY;
  status_ptr->applications[2].perso_substate = RIL_PERSOSUBSTATE_READY;
  status_ptr->applications[2].pin1 = RIL_PINSTATE_DISABLED;
  status_ptr->applications[2].pin2 = RIL_PINSTATE_ENABLED_NOT_VERIFIED;
  status_ptr->applications[2].aid_ptr = (char *)malloc(33);
  strcpy(status_ptr->applications[2].aid_ptr, "a0000003431002f310ffff89020000ff");
  status_ptr->applications[2].app_label_ptr = (char *)malloc(5);
  strcpy(status_ptr->applications[2].app_label_ptr, "CSIM");

  status_ptr->applications[3].app_type = RIL_APPTYPE_UNKNOWN;
  status_ptr->applications[3].app_state = RIL_APPSTATE_DETECTED;
  status_ptr->applications[3].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
  status_ptr->applications[3].pin1 = RIL_PINSTATE_UNKNOWN;
  status_ptr->applications[3].pin2 = RIL_PINSTATE_UNKNOWN;
  status_ptr->applications[3].aid_ptr = (char *)malloc(33);
  strcpy(status_ptr->applications[3].aid_ptr, "a000000063504b43532d3135");
  status_ptr->applications[3].app_label_ptr = (char *)malloc(5);
  strcpy(status_ptr->applications[3].app_label_ptr, "PKCS");
  t.expect(RIL_node);

  // Inject: RIL_REQUEST_RADIO_POWER
  RIL_node.update_default_ril_request_radio_power();
  t.inject(RIL_node);

  t.set_start();

  // Expect: IPC_MESSAGE_RADIO_POWER (recv)
  MSIM_node.update_default_ipc_message_radio_power_recv(1);
  MSIM_node.set_ipc_message_radio_power_recv_rild_instance(0);
  t.expect(MSIM_node);

  // Inject: IPC_MESSAGE_RADIO_POWER (send)
  MSIM_node.update_default_ipc_message_radio_power_send(1);
  MSIM_node.set_ipc_message_radio_power_send_rild_instance(1);
  t.inject(MSIM_node);

  // Expect: QMI_DMS_GET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_req_msg();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_get_operating_mode_resp_msg();
  QMI_node.set_qmi_dms_get_operating_mode_resp_operating_mode(DMS_OP_MODE_LOW_POWER_V01);
  QMI_node.set_qmi_dms_get_operating_mode_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_DMS_SET_OPERATING_MODE_REQ_V01
  QMI_node.update_default_qmi_dms_set_operating_mod_req_v01(DMS_OP_MODE_ONLINE_V01);
  t.expect(QMI_node);

  // Inject: QMI_DMS_SET_OPERATING_MODE_RESP_V01
  QMI_node.update_default_qmi_dms_set_operating_mode_resp_v01();
  QMI_node.set_qmi_dms_set_operating_mode_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject: QMI_DMS_EVENT_REPORT_IND_V01
  QMI_node.update_default_qmi_dms_event_report_ind_v01();
  QMI_node.set_qmi_dms_event_report_ind_v01_operating_mode(DMS_OP_MODE_ONLINE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_CONFIG_SIG_INFO2_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_config_sig_info2_req_msg_v01();
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_cdma_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_cdma_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_gsm_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_hdr_sinr_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rsrp_delta(60);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rsrq_delta(20);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_rssi_delta(50);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_lte_snr_delta(40);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_ecio_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_rscp_delta(40);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_rssi_delta(5);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_tdscdma_sinr_delta(1);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_wcdma_ecio_delta(10);
  QMI_node.set_default_qmi_nas_config_sig_info2_req_msg_v01_wcdma_rssi_delta(50);
  t.expect(QMI_node);

  // Inject: QMI_NAS_CONFIG_SIG_INFO2_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_config_sig_info2_resp_msg_v01();
  QMI_node.set_qmi_nas_config_sig_info2_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_SET_EVENT_REPORT_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_set_event_report_req_msg_v01();
  QMI_node.set_qmi_nas_set_event_report_req_report_reg_reject(TRUE);
  t.expect(QMI_node);

  // Inject: QMI_NAS_SET_EVENT_REPORT_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_set_event_report_resp_msg_v01();
  QMI_node.set_qmi_nas_set_event_report_resp_msg_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: RIL_REQUEST_RADIO_POWER (resp)
  RIL_node.update_default_ril_request_radio_power_resp();
  t.expect(RIL_node);

  // Expect: RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED
  RIL_node.update_default_ril_unsol_resp_radio_state_changed();
  t.expect(RIL_node);

  // Expect: QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_indication_register_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_INDICATION_REGISTER_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_indication_register_resp_msg_v01();
  QMI_node.set_qmi_nas_nas_indication_register_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_config_plmn_name_ind_reporting_req_msg_v01();
  QMI_node.set_qmi_nas_config_plmn_name_ind_reporting_req_msg_send_all_information(1);
  t.expect(QMI_node);

  // Inject: QMI_NAS_CONFIG_PLMN_NAME_IND_REPORTING_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_config_plmn_name_ind_reporting_resp_msg_v01();
  QMI_node.set_qmi_nas_config_plmn_name_ind_reporting_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
  nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
  // Set values for data
  ptr->gsm_srv_status_info_valid = TRUE;
  ptr->gsm_srv_status_info.srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
  ptr->gsm_srv_status_info.true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
  ptr->gsm_srv_status_info.is_pref_data_path = 0;
  ptr->wcdma_srv_status_info_valid = TRUE;
  ptr->wcdma_srv_status_info.srv_status = NAS_SYS_SRV_STATUS_SRV_V01;
  ptr->wcdma_srv_status_info.true_srv_status = SYS_SRV_STATUS_SRV_V01;
  ptr->wcdma_srv_status_info.is_pref_data_path = 0;
  ptr->wcdma_sys_info.common_sys_info.srv_domain_valid = TRUE;
  ptr->wcdma_sys_info.common_sys_info.srv_domain = SYS_SRV_DOMAIN_CS_PS_V01;
  ptr->wcdma_sys_info.common_sys_info.srv_capability_valid = TRUE;
  ptr->wcdma_sys_info.common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
  ptr->wcdma_sys_info.common_sys_info.roam_status_valid = TRUE;
  ptr->wcdma_sys_info.common_sys_info.roam_status = NAS_SYS_ROAM_STATUS_OFF_V01;
  ptr->wcdma_sys_info.common_sys_info.is_sys_forbidden_valid = TRUE;
  ptr->wcdma_sys_info.common_sys_info.is_sys_forbidden = 0;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.lac_valid = TRUE;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.lac = 43024;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.cell_id_valid = TRUE;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.cell_id = 13548179;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.network_id_valid = TRUE;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.hs_call_status_valid = TRUE;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.hs_call_status = SYS_HS_IND_HSDPA_HSUPA_UNSUPP_CELL_V01;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.hs_ind_valid = TRUE;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.hs_ind = SYS_HS_IND_HSDPA_HSUPA_UNSUPP_CELL_V01;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.psc_valid = TRUE;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.psc = 426;
  ptr->wcdma_cipher_domain_valid = TRUE;
  ptr->wcdma_cipher_domain = SYS_SRV_DOMAIN_CS_ONLY_V01;
  ptr->sim_rej_info_valid = TRUE;
  ptr->sim_rej_info = NAS_SIM_AVAILABLE_V01;
  QMI_node.set_qmi_nas_get_sys_info_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_ERR_RATE_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_err_rate_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_ERR_RATE_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_err_rate_resp_msg_v01();
  QMI_node.set_qmi_nas_get_err_rate_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_SIG_INFO_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_sig_info_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_SIG_INFO_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_sig_info_resp_msg_v01();
  QMI_node.set_qmi_nas_get_sig_info_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_GET_EMBMS_STATUS_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_embms_status_req_msg_v01();
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_EMBMS_STATUS_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_embms_status_resp_msg_v01();
  QMI_node.set_qmi_nas_get_embms_status_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_PBM_INDICATION_REGISTER_REQ_V01
  QMI_node.update_default_qmi_pbm_indication_register_req_msg();
  QMI_node.set_qmi_pbm_indication_register_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01|PBM_REG_RECORD_UPDATE_EVENTS_V01|PBM_REG_PHONEBOOK_READY_EVENTS_V01);
  t.expect(QMI_node);

  // Inject: QMI_PBM_INDICATION_REGISTER_RESP_V01
  QMI_node.update_default_qmi_pbm_indication_register_resp_msg();
  QMI_node.set_qmi_pbm_indication_register_resp_v01_reg_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01|PBM_REG_RECORD_UPDATE_EVENTS_V01|PBM_REG_PHONEBOOK_READY_EVENTS_V01);
  QMI_node.set_qmi_pbm_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // End unordered expect set
  t.set_end();

  // Inject: RIL_REQUEST_SET_UICC_SUBSCRIPTION
  RIL_SelectUiccSub* uicc_subs_data = RIL_node.update_default_ril_request_set_uicc_subscription();
  RIL_node.set_ril_request_set_uicc_subscription_app_index(0);
  RIL_node.set_ril_request_set_uicc_subscription_slot(0);
  RIL_node.set_ril_request_set_uicc_subscription_act_status(RIL_UICC_SUBSCRIPTION_ACTIVATE);
  RIL_node.set_ril_request_set_uicc_subscription_sub_type(RIL_SUBSCRIPTION_1);
  t.inject(RIL_node);

  //Expect: QMI_NAS_GET_MODE_PREF_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_mode_pref_req();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_MODE_PREF_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_mode_pref_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  //QMI_node.update_default_qmi_nas_get_mode_pref_resp_msg(1, 3);
  QMI_node.set_qmi_nas_get_mode_pref_resp_idx0_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM);
  QMI_node.set_qmi_nas_get_mode_pref_resp_idx1_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM);
  QMI_node.set_qmi_nas_get_mode_pref_resp_idx2_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM);
  t.inject(QMI_node);

  //Expect: QMI_UIM_GET_LABEL_REQ
  label_req1 = QMI_node.update_default_qmi_uim_get_label_req();
  label_req1->application_information.slot = (uim_slot_enum_v01)0x01;
  label_req1->application_information.aid_len = 0x10;
  memcpy(label_req1->application_information.aid, app_value_1, 16);//sizeof(label_req->application_information.aid));
  t.expect(QMI_node);

  //Inject: QMI_UIM_GET_LABEL_RESP
  response1 = QMI_node.update_default_qmi_uim_get_label_resp();
  response1->resp.result = QMI_RESULT_SUCCESS_V01;
  strcpy(response1->label, "USIM");
  response1->label_len = 4;
  response1->label_valid = TRUE;
  t.inject(QMI_node);

  //Expect: QMI_UIM_GET_LABEL_REQ
  label_req2 = QMI_node.update_default_qmi_uim_get_label_req();
  label_req2->application_information.slot = (uim_slot_enum_v01)0x01;
  label_req2->application_information.aid_len = 0x10;
  memcpy(label_req2->application_information.aid, app_value_2, 16);//sizeof(label_req->application_information.aid));
  t.expect(QMI_node);

  //Inject: QMI_UIM_GET_LABEL_RESP
  response2 = QMI_node.update_default_qmi_uim_get_label_resp();
  response2->resp.result = QMI_RESULT_SUCCESS_V01;
  strcpy(response2->label, "ISIM");
  response2->label_len = 4;
  response2->label_valid = TRUE;
  t.inject(QMI_node);

  //Expect: QMI_UIM_GET_LABEL_REQ
  label_req3 = QMI_node.update_default_qmi_uim_get_label_req();
  label_req3->application_information.slot = (uim_slot_enum_v01)0x01;
  label_req3->application_information.aid_len = 0x10;
  memcpy(label_req3->application_information.aid, app_value_3, 16);//sizeof(label_req->application_information.aid));
  t.expect(QMI_node);

  //Inject: QMI_UIM_GET_LABEL_RESP
  response3 = QMI_node.update_default_qmi_uim_get_label_resp();
  response3->resp.result = QMI_RESULT_SUCCESS_V01;
  strcpy(response3->label, "CSIM");
  response3->label_len = 4;
  response3->label_valid = TRUE;
  t.inject(QMI_node);

  //Expect: QMI_UIM_GET_LABEL_REQ
  label_req4 = QMI_node.update_default_qmi_uim_get_label_req();
  label_req4->application_information.slot = (uim_slot_enum_v01)0x01;
  label_req4->application_information.aid_len = 0x0C;
  memcpy(label_req4->application_information.aid, app_value_4, 16);//sizeof(label_req->application_information.aid));
  t.expect(QMI_node);

  //Inject: QMI_UIM_GET_LABEL_RESP
  response4 = QMI_node.update_default_qmi_uim_get_label_resp();
  response4->resp.result = QMI_RESULT_SUCCESS_V01;
  strcpy(response4->label, "PKCS");
  response4->label_len = 4;
  response4->label_valid = TRUE;
  t.inject(QMI_node);

  //Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_emergency_mode(0);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(mode_pref1);
  // Set band_pref
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_band_pref(band_pref);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_prl_pref(NAS_PRL_PREF_ANY_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_roam_pref(NAS_ROAMING_PREF_ANY_V01);
  // Set band_pref_ext
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_band_pref_ext(band_pref_ext);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_net_sel_pref(NAS_NET_SEL_PREF_AUTOMATIC_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_PS_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_gw_acq_order_pref(NAS_GW_ACQ_ORDER_PREF_AUTOMATIC_V01);
  // Set tdscdma_band_pref
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_tdscdma_band_pref(tdscdma_band_pref);
  QMI_node.set_default_qmi_nas_get_system_selection_preference_resp_acq_order();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_reg_restriction(NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_usage_setting(NAS_USAGE_VOICE_CENTRIC_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: RIL_REQUEST_SET_UICC_SUBSCRIPTION_RESP
  RIL_node.update_default_ril_request_set_uicc_subscription_resp();
  RIL_node.set_ril_error_value((RIL_Errno)0);
  t.expect(RIL_node);

  //QMI_NAS_GET_SUBSCRIPTION_INFO_REQ_MSG_V01
  util_ril_utf_get_subscription_info(t, QMI_node, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, TRUE, NAS_PRIORITY_SUBSCRIPTION_TRUE_V01, TRUE, NAS_SUBSCRIPTION_ACTIVE_V01, TRUE, 0, TRUE, 0, FALSE, 0, FALSE, 0);

  // Expect: RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED
  RIL_node.update_default_ril_unsol_uicc_subscription_status_changed(1);
  t.expect(RIL_node);

  // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
  ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
  // Set values for data
  ptr->gsm_srv_status_info_valid = TRUE;
  ptr->gsm_srv_status_info.srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
  ptr->gsm_srv_status_info.true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
  ptr->gsm_srv_status_info.is_pref_data_path = 0;
  ptr->wcdma_srv_status_info_valid = TRUE;
  ptr->wcdma_srv_status_info.srv_status = NAS_SYS_SRV_STATUS_SRV_V01;
  ptr->wcdma_srv_status_info.true_srv_status = SYS_SRV_STATUS_SRV_V01;
  ptr->wcdma_srv_status_info.is_pref_data_path = 0;
  ptr->wcdma_sys_info.common_sys_info.srv_domain_valid = TRUE;
  ptr->wcdma_sys_info.common_sys_info.srv_domain = SYS_SRV_DOMAIN_CS_PS_V01;
  ptr->wcdma_sys_info.common_sys_info.srv_capability_valid = TRUE;
  ptr->wcdma_sys_info.common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
  ptr->wcdma_sys_info.common_sys_info.roam_status_valid = TRUE;
  ptr->wcdma_sys_info.common_sys_info.roam_status = NAS_SYS_ROAM_STATUS_OFF_V01;
  ptr->wcdma_sys_info.common_sys_info.is_sys_forbidden_valid = TRUE;
  ptr->wcdma_sys_info.common_sys_info.is_sys_forbidden = 0;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.lac_valid = TRUE;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.lac = 43024;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.cell_id_valid = TRUE;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.cell_id = 13548179;
  ptr->wcdma_sys_info.threegpp_specific_sys_info.network_id_valid = TRUE;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.hs_call_status_valid = TRUE;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.hs_call_status = SYS_HS_IND_HSDPA_HSUPA_UNSUPP_CELL_V01;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.hs_ind_valid = TRUE;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.hs_ind = SYS_HS_IND_HSDPA_HSUPA_UNSUPP_CELL_V01;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.psc_valid = TRUE;
  ptr->wcdma_sys_info.wcdma_specific_sys_info.psc = 426;
  ptr->wcdma_cipher_domain_valid = TRUE;
  ptr->wcdma_cipher_domain = SYS_SRV_DOMAIN_CS_ONLY_V01;
  ptr->sim_rej_info_valid = TRUE;
  ptr->sim_rej_info = NAS_SIM_AVAILABLE_V01;
  QMI_node.set_qmi_nas_get_sys_info_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_voice_network_state_changed();
  t.expect(RIL_node);

  // Expect: RIL_UNSOL_SIGNAL_STRENGTH
  RIL_node.update_default_ril_unsol_signal_strength();
  t.expect(RIL_node);

  // Expect: RIL_UNSOL_VOICE_RADIO_TECH_CHANGED
  RIL_node.update_default_ril_unsol_voice_radio_tech_changed();
  t.expect(RIL_node);

  // Expect: RIL_UNSOL_SIGNAL_STRENGTH
  RIL_node.update_default_ril_unsol_signal_strength();
  t.expect(RIL_node);

  // Start oem threads for oem socket
  t.add_func_exec(utf_start_oem_threads);

  // Start the ims threads for ims
  t.add_func_exec(utf_start_ims_threads);

  // Expect: ims_MsgId_UNSOL_RADIO_STATE_CHANGED
  IMS_node.update_default_ims_unsol_radio_state_changed(ims_RadioState_RADIO_STATE_ON );
  t.expect(IMS_node);

  // Execute the test case
  t.execute();

  t.print_summary();

  return t.get_test_result();
}
