/******************************************************************************
#  Copyright (c) 2009-2010,2014 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_utility_if.h
  @brief   Interface file for the utility functions

  DESCRIPTION
  File contains the prototype of the utility functions used in test cases
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef _RIL_UTF_TEST_UTILITY_H
#define _RIL_UTF_TEST_UTILITY_H
#include "ril_utf_test_if.h"
#include "ril_utf_if.h"
#include "ril_utf_test_includes.h"

#define QMI_NAS_RAT_MODE_PREF_CDMA                          ( 1 << QMI_NAS_RAT_MODE_PREF_CDMA2000_1X_BIT_V01 )
#define QMI_NAS_RAT_MODE_PREF_HRPD                          ( 1 << QMI_NAS_RAT_MODE_PREF_CDMA2000_HRPD_BIT_V01 )
#define QMI_NAS_RAT_MODE_PREF_GSM                           ( 1 << QMI_NAS_RAT_MODE_PREF_GSM_BIT_V01 )
#define QMI_NAS_RAT_MODE_PREF_UMTS                          ( 1 << QMI_NAS_RAT_MODE_PREF_UMTS_BIT_V01 )
#define QMI_NAS_RAT_MODE_PREF_TDSCDMA                       ( 1 << QMI_NAS_RAT_MODE_PREF_TDSCDMA_BIT_V01 )
#define QMI_NAS_RAT_MODE_PREF_LTE                           ( 1 << QMI_NAS_RAT_MODE_PREF_LTE_BIT_V01 )
#define QMI_NAS_RAT_MODE_PREF_CDMA_HRPD                     ( QMI_NAS_RAT_MODE_PREF_CDMA + QMI_NAS_RAT_MODE_PREF_HRPD )
#define QMI_NAS_RAT_MODE_PREF_GSM_UMTS                      ( QMI_NAS_RAT_MODE_PREF_GSM + QMI_NAS_RAT_MODE_PREF_UMTS )
#define QMI_NAS_RAT_MODE_PREF_UMTS_TDSCDMA                  ( QMI_NAS_RAT_MODE_PREF_UMTS + QMI_NAS_RAT_MODE_PREF_TDSCDMA )
#define QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA                   ( QMI_NAS_RAT_MODE_PREF_GSM + QMI_NAS_RAT_MODE_PREF_TDSCDMA )
#define QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA_LTE               ( QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA + QMI_NAS_RAT_MODE_PREF_LTE )
#define QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA              ( QMI_NAS_RAT_MODE_PREF_GSM + QMI_NAS_RAT_MODE_PREF_UMTS + QMI_NAS_RAT_MODE_PREF_TDSCDMA )
#define QMI_NAS_RAT_MODE_PREF_UMTS_LTE                      ( QMI_NAS_RAT_MODE_PREF_UMTS + QMI_NAS_RAT_MODE_PREF_LTE )
#define QMI_NAS_RAT_MODE_PREF_TDSCDMA_LTE                   ( QMI_NAS_RAT_MODE_PREF_TDSCDMA + QMI_NAS_RAT_MODE_PREF_LTE )
#define QMI_NAS_RAT_MODE_PREF_GSM_UMTS_LTE                  ( QMI_NAS_RAT_MODE_PREF_GSM_UMTS + QMI_NAS_RAT_MODE_PREF_LTE )
#define QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_LTE          ( QMI_NAS_RAT_MODE_PREF_GSM_UMTS + QMI_NAS_RAT_MODE_PREF_TDSCDMA + QMI_NAS_RAT_MODE_PREF_LTE )
#define QMI_NAS_RAT_MODE_PREF_UMTS_TDSCDMA_LTE              ( QMI_NAS_RAT_MODE_PREF_UMTS + QMI_NAS_RAT_MODE_PREF_TDSCDMA_LTE )
#define QMI_NAS_RAT_MODE_PREF_GSM_UMTS_CDMA_HRPD            ( QMI_NAS_RAT_MODE_PREF_GSM_UMTS + QMI_NAS_RAT_MODE_PREF_CDMA_HRPD )
#define QMI_NAS_RAT_MODE_PREF_CDMA_HRPD_LTE                 ( QMI_NAS_RAT_MODE_PREF_CDMA_HRPD + QMI_NAS_RAT_MODE_PREF_LTE )
#define QMI_NAS_RAT_MODE_PREF_GSM_UMTS_CDMA_HRPD_LTE        ( QMI_NAS_RAT_MODE_PREF_GSM_UMTS_LTE + QMI_NAS_RAT_MODE_PREF_CDMA_HRPD )
#define QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA_CDMA_HRPD_LTE     ( QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA_LTE + QMI_NAS_RAT_MODE_PREF_CDMA_HRPD )
#define QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA_CDMA_HRPD_UMTS     ( QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA + QMI_NAS_RAT_MODE_PREF_CDMA_HRPD )
#define QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_CDMA_HRPD_LTE ( QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_LTE + QMI_NAS_RAT_MODE_PREF_CDMA_HRPD )
#define RIL_VAL_REG_UNKNOWN                                 4
#define RIL_VAL_REG_REGISTERED_HOME_NET                     1
#define RIL_VAL_REG_NOT_REGISTERED_SEARCHING                2
#define RIL_VAL_REG_NOT_REGISTERED_NOT_SEARCHING            0
#define RIL_VAL_REG_REGISTRATION_DENIED                     3
#define RIL_VAL_REG_REGISTERED_ROAMING                      5
#define RIL_VAL_REG_NOT_REGISTERED_SEARCHING_LTD_SRV        12
#define RIL_VAL_REG_REGISTRATION_DENIED_LTD_SRV             13
//Permanent Reg Rejection Causes
#define IMSI_UNKNOWN_IN_HLR                             0x02
#define ILLEGAL_MS                                      0x03
#define ILLEGAL_ME                                      0x06
#define GPRS_SERVICES_NOT_ALLOWED                       0x07
#define GPRS_SERVICES_AND_NON_GPRS_SERVICES_NOT_ALLOWED 0x08
#define PLMN_NOT_ALLOWED                                0x0B
#define LA_NOT_ALLOWED                                  0x0C
#define NATIONAL_ROAMING_NOT_ALLOWED                    0x0D
#define GPRS_SERVICES_NOT_ALLOWED_IN_THIS_PLMN          0x0E
#define NO_SUITABLE_CELLS_IN_LA                         0x0F
#define CSG_NOT_AUTHORIZED                              0x19

/* Signal strength unknown */
#define QCRIL_CM_GW_SIGNAL_STRENGTH_UNKNOWN 99
#define QCRIL_CM_SIGNAL_STRENGTH_UNKNOWN    -1

/* Signal level conversion result codes */
#define QCRIL_CM_RSSI_TOOLO_CODE   0
#define QCRIL_CM_RSSI_TOOHI_CODE   31

/* RSSI range conversion */
#define QCRIL_CM_RSSI_MIN        51   /* per 3GPP 27.007  (negative value) */
#define QCRIL_CM_RSSI_MAX        113  /* per 3GPP 27.007  (negative value) */
#define QCRIL_CM_RSSI_NO_SIGNAL  125
#define QCRIL_CM_RSSI_OFFSET     182.26
#define QCRIL_CM_RSSI_SLOPE      (-100.0/62.0)

/* MAX TAG SIZE */
#define MAX_TAG_SIZE 1000

void util_ril_utf_test_cdma_set_subscription_source_with_spc ( Testcase &t, OEMTestnode &OEM_node, QMIRILTestnode &QMI_node, int sub_source, char *spc, int spc_len, int qmi_resp, int qmi_error, int res, int resp_data);

void util_ril_utf_test_cdma_set_roaming_preference ( Testcase &t,AndroidTelTestnode &RIL_node, QMIRILTestnode &QMI_node, ril_cdma_roaming_preference_enum_type cdma_roam_pref, int res);

void util_ril_utf_test_cdma_query_roaming_preference ( Testcase &t, AndroidTelTestnode &RIL_node, QMIRILTestnode &QMI_node, nas_roam_pref_enum_v01 roam_pref, int res);

void util_ril_utf_test_cdma_set_subscription_source ( Testcase &t, AndroidTelTestnode &RIL_node, QMIRILTestnode &QMI_node, int sub_source, char *spc, int spc_len, int res);

void util_ril_utf_test_cdma_get_subscription_source ( Testcase &t, AndroidTelTestnode &RIL_node, QMIRILTestnode &QMI_node, nas_rtre_cfg_enum_v01 rtre_cfg_pref, int res);

void conv_android_mode_pref_to_qmi_mode_pref(uint16_t android_mode_pref, uint16_t *qmi_mode_pref, uint8_t *qmi_gw_acq_order_pref_valid, uint16_t *qmi_gw_acq_order_pref);

void conv_qmi_mode_pref_to_android_mode_pref(uint16_t mode_pref, uint8_t gw_acq_order_pref_valid, uint16_t gw_acq_order_pref, RIL_PreferredNetworkType *android_mode_pref);

void util_set_pref_net_type(Testcase &t, AndroidTelTestnode &RIL_node, QMIRILTestnode &QMI_node, uint16_t android_mode_pref, int rat,int qmi_error, int android_resp, uint32_t timeout, uint8_t is_ind_came, uint8_t is_sig_needed, uint8_t is_err_needed, uint8_t is_net_sel_pref);

void util_set_pref_net_type(Testcase &t, AndroidTelTestnode &RIL_node, QMIRILTestnode &QMI_node, uint16_t android_mode_pref, int rat,int qmi_error, int android_resp, uint32_t timeout, uint8_t is_ind_came, uint8_t is_sig_needed, uint8_t is_err_needed, uint8_t is_net_sel_pref, uint8_t uim_get_label_num);

void util_get_pref_network_type(Testcase &t, AndroidTelTestnode &RIL_node, QMIRILTestnode &QMI_node, uint16_t qmi_mode_pref, int android_resp, int qmi_error, uint8_t qmi_gw_acq_order_pref_valid, uint16_t qmi_gw_acq_order_pref);

void util_voice_radio_tech_oos(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int is_rat_change, int rat);

void util_voice_radio_tech_oos(Testcase &t, QMIRILTestnode &QMI_node,
        AndroidTelTestnode &RIL_node, int is_rat_change, int rat, uint8_t uim_get_label_num);

void util_voice_radio_tech_wcdma_in_service(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int is_rat_change, int rat, nas_service_status_enum_type_v01 srv_status, nas_true_service_status_enum_type_v01 true_srv_status, nas_service_domain_enum_type_v01 domain,nas_roam_status_enum_type_v01 roam_status);

void util_voice_radio_tech_wcdma_in_service(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int is_rat_change, int rat, nas_service_status_enum_type_v01 srv_status, nas_true_service_status_enum_type_v01 true_srv_status, nas_service_domain_enum_type_v01 domain,nas_roam_status_enum_type_v01 roam_status, uint8_t net_id_valid, const char *mcc, const char *mnc);

void util_voice_radio_tech_gsm_in_service(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int is_rat_change, int rat, nas_service_status_enum_type_v01 srv_status, nas_true_service_status_enum_type_v01 true_srv_status, nas_service_domain_enum_type_v01 domain,nas_roam_status_enum_type_v01 roam_status, uint8_t is_needed);

void util_voice_radio_tech_tdscdma_in_service(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int is_rat_change, int rat, nas_service_status_enum_type_v01 srv_status, nas_true_service_status_enum_type_v01 true_srv_status, nas_service_domain_enum_type_v01 domain,nas_roam_status_enum_type_v01 roam_status, uint8_t is_needed);

void util_voice_radio_tech_cdma_in_service(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int is_rat_change, int rat, nas_service_status_enum_type_v01 srv_status, nas_service_domain_enum_type_v01 domain, nas_roam_status_enum_type_v01 roam_status, uint8_t is_needed );

void util_voice_radio_tech_cdma_in_service(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int is_rat_change, int rat, nas_service_status_enum_type_v01 srv_status, nas_service_domain_enum_type_v01 domain, nas_roam_status_enum_type_v01 roam_status, uint8_t net_id_valid, const char *mcc, const char *mnc, uint8_t is_needed );

void util_voice_radio_tech_lte_in_service(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, IMSTestnode &IMS_node, uint8_t vops_expected, int rat,nas_service_status_enum_type_v01 srv_status, nas_true_service_status_enum_type_v01 true_srv_status, nas_service_domain_enum_type_v01 domain, nas_roam_status_enum_type_v01 roam_status, nas_lte_voice_status_enum_type_v01 lte_voice_status, nas_sms_status_enum_type_v01 lte_sms_status, uint8_t voice_support_on_lte, uint8_t net_id_valid, const char *mcc, const char *mnc, uint8_t is_rat_change, uint8_t is_needed, uint8_t is_data_rat_change = TRUE);

void util_voice_radio_tech_lte_in_service(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, IMSTestnode &IMS_node, uint8_t is_vops_ind_expected, int rat,nas_service_status_enum_type_v01 srv_status, nas_true_service_status_enum_type_v01 true_srv_status, nas_service_domain_enum_type_v01 domain, nas_roam_status_enum_type_v01 roam_status, nas_lte_voice_status_enum_type_v01 lte_voice_status, nas_sms_status_enum_type_v01 lte_sms_status, uint8_t voice_support_on_lte, uint8_t net_id_valid, const char *mcc, const char *mnc, uint8_t is_rat_change, uint8_t is_needed, uint8_t is_data_reg_needed, bool is_data_rat_change);

void util_voice_radio_tech_in_sglte_case(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, IMSTestnode &IMS_node, uint8_t is_vops_ind_expected, int rat,nas_service_status_enum_type_v01 gsm_srv_status, nas_true_service_status_enum_type_v01 gsm_true_srv_status, nas_service_domain_enum_type_v01 gsm_domain, nas_roam_status_enum_type_v01 gsm_roam_status, nas_service_status_enum_type_v01 lte_srv_status, nas_true_service_status_enum_type_v01 lte_true_srv_status, nas_service_domain_enum_type_v01 lte_domain, nas_roam_status_enum_type_v01 lte_roam_status, nas_lte_voice_status_enum_type_v01 lte_voice_status, nas_sms_status_enum_type_v01 lte_sms_status, int is_rat_change, int is_data_rat_change, int is_sig_info_needed, int is_err_rate_needed, uint8_t voice_support_on_lte);

void util_voice_radio_tech_in_svlte_case(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, IMSTestnode &IMS_node, uint8_t is_vops_ind_expected, int rat,nas_service_status_enum_type_v01 cdma_srv_status, nas_service_domain_enum_type_v01 cdma_domain, nas_roam_status_enum_type_v01 cdma_roam_status, nas_service_status_enum_type_v01 lte_srv_status, nas_true_service_status_enum_type_v01 lte_true_srv_status, nas_service_domain_enum_type_v01 lte_domain, nas_roam_status_enum_type_v01 lte_roam_status, nas_lte_voice_status_enum_type_v01 lte_voice_status, nas_sms_status_enum_type_v01 lte_sms_status, int is_rat_change, int is_data_rat_change, int is_sig_info_needed, int is_err_rate_needed, uint8_t voice_support_on_lte);

void util_voice_radio_tech_in_svdo_case(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int rat, nas_service_status_enum_type_v01 cdma_srv_status, nas_service_domain_enum_type_v01 cdma_domain, nas_roam_status_enum_type_v01 cdma_roam_status, nas_service_status_enum_type_v01 hdr_srv_status, nas_service_domain_enum_type_v01 hdr_domain, nas_roam_status_enum_type_v01 hdr_roam_status, uint8_t hdr_personality_valid, nas_hdr_personality_enum_type_v01 hdr_personality, uint8_t hdr_active_prot_valid, nas_hdr_active_prot_enum_type_v01 hdr_active_prot, int is_rat_change, int is_sig_info_needed, int is_err_rate_needed);

void util_voice_registration_state_failure(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int qmi_error, int android_resp);

void util_voice_registration_state_in_oos( Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, bool is_data_rat_change = true);

void util_voice_registration_state_for_cdma(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int rat, int reg_state, nas_service_status_enum_type_v01 srv_status, nas_service_domain_enum_type_v01 domain, nas_roam_status_enum_type_v01 roam_status, bool is_data_rat_change = true);

void util_voice_registration_state_for_wcdma_success(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, OEMTestnode &OEM_node, int rat, int reg_state, nas_service_status_enum_type_v01 srv_status, nas_true_service_status_enum_type_v01 true_srv_status, nas_service_domain_enum_type_v01 domain, nas_roam_status_enum_type_v01 roam_status, uint8_t reg_reject_info_valid, nas_service_domain_enum_type_v01 reject_srv_domain, uint8_t rej_cause, uint8_t net_id_valid, const char *mcc, const char *mnc, uint8_t csg_info_valid, uint32_t cid, uint32_t cid_name_len, uint16_t *cid_name, bool is_data_rat_change = true);

void util_voice_registration_state_for_gsm_success(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int rat, int reg_state, nas_service_status_enum_type_v01 srv_status, nas_true_service_status_enum_type_v01 true_srv_status, nas_service_domain_enum_type_v01 domain, nas_roam_status_enum_type_v01 roam_status, uint8_t reg_reject_info_valid, nas_service_domain_enum_type_v01 reject_srv_domain, uint8_t rej_cause, uint8_t net_id_valid, const char *mcc, const char *mnc, bool is_data_rat_change = true);

void util_voice_registration_state_for_tdscdma_success(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int rat, int reg_state, nas_service_status_enum_type_v01 srv_status, nas_true_service_status_enum_type_v01 true_srv_status, nas_service_domain_enum_type_v01 domain, nas_roam_status_enum_type_v01 roam_status, uint8_t reg_reject_info_valid, nas_service_domain_enum_type_v01 reject_srv_domain, uint8_t rej_cause, bool is_data_rat_change = true);

void util_voice_registration_state_for_lte(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, OEMTestnode &OEM_node, int rat, int reg_state, nas_service_status_enum_type_v01 srv_status, nas_true_service_status_enum_type_v01 true_srv_status, nas_service_domain_enum_type_v01 domain, nas_roam_status_enum_type_v01 roam_status, uint8_t voice_support_on_lte, uint8_t reg_reject_info_valid, nas_service_domain_enum_type_v01 reject_srv_domain, uint8_t rej_cause, uint8_t net_id_valid, const char *mcc, const char *mnc, uint8_t csg_info_valid, uint32_t cid, uint32_t cid_name_len, uint16_t *cid_name, uint8_t lte_is_eb_supported_valid, nas_tri_state_boolean_type_v01 lte_is_eb_supported, bool is_data_rat_change = true);

void util_voice_registration_state_in_sglte_case(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int rat, int reg_state, nas_service_status_enum_type_v01 gsm_srv_status, nas_true_service_status_enum_type_v01 gsm_true_srv_status, nas_service_domain_enum_type_v01 gsm_domain, nas_roam_status_enum_type_v01 gsm_roam_status, uint8_t gsm_reg_reject_info_valid, nas_service_domain_enum_type_v01 gsm_reject_srv_domain, uint8_t gsm_rej_cause, nas_service_status_enum_type_v01 lte_srv_status, nas_true_service_status_enum_type_v01 lte_true_srv_status, nas_service_domain_enum_type_v01 lte_domain, nas_roam_status_enum_type_v01 lte_roam_status, uint8_t voice_support_on_lte, uint8_t lte_reg_reject_info_valid, nas_service_domain_enum_type_v01 lte_reject_srv_domain, uint8_t lte_rej_cause, uint8_t gsm_net_id_valid, const char *gsm_mcc, const char *gsm_mnc, uint8_t lte_net_id_valid, const char *lte_mcc, const char *lte_mnc, bool is_data_rat_change = true);

void util_voice_registration_state_in_svlte_case(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int rat, int reg_state, nas_service_status_enum_type_v01 cdma_srv_status, nas_service_domain_enum_type_v01 cdma_domain, nas_roam_status_enum_type_v01 cdma_roam_status, nas_service_status_enum_type_v01 lte_srv_status, nas_true_service_status_enum_type_v01 lte_true_srv_status, nas_service_domain_enum_type_v01 lte_domain, nas_roam_status_enum_type_v01 lte_roam_status, uint8_t voice_support_on_lte, uint8_t lte_reg_reject_info_valid, nas_service_domain_enum_type_v01 lte_reject_srv_domain, uint8_t lte_rej_cause, bool is_data_rat_change = true);

void util_voice_registration_state_in_svdo_case(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int rat, int reg_state, nas_service_status_enum_type_v01 cdma_srv_status, nas_service_domain_enum_type_v01 cdma_domain, nas_roam_status_enum_type_v01 cdma_roam_status, nas_service_status_enum_type_v01 hdr_srv_status, nas_service_domain_enum_type_v01 hdr_domain, nas_roam_status_enum_type_v01 hdr_roam_status, uint8_t hdr_personality_valid, nas_hdr_personality_enum_type_v01 hdr_personality, uint8_t hdr_active_prot_valid, nas_hdr_active_prot_enum_type_v01 hdr_active_prot, bool is_data_rat_change = true);

void util_data_registration_state_failure(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int qmi_error, int android_resp);

void util_data_registration_state_for_lte_success(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int rat, int reg_state, uint8_t is_cell_loc_needed);

void util_data_registration_state_for_hdr(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int rat, int reg_state, nas_service_status_enum_type_v01 srv_status, nas_service_domain_enum_type_v01 domain, nas_roam_status_enum_type_v01 roam_status, uint8_t hdr_personality_valid, nas_hdr_personality_enum_type_v01 hdr_personality, uint8_t hdr_active_prot_valid, nas_hdr_active_prot_enum_type_v01 hdr_active_prot );

void util_data_registration_state_for_wcdma_success(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int rat, int reg_state, nas_service_status_enum_type_v01 srv_status, nas_true_service_status_enum_type_v01 true_srv_status, nas_service_domain_enum_type_v01 domain, nas_roam_status_enum_type_v01 roam_status, uint8_t reg_reject_info_valid, nas_service_domain_enum_type_v01 reject_srv_domain, uint8_t rej_cause, uint8_t is_cell_loc_needed);

void util_to_get_operator_name_3gpp(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, const char *mcc_ptr, const char *mnc_ptr, uint8_t csg_id_valid, uint32_t csg_id, nas_radio_if_enum_v01 rat, const char *long_plmn_name, const char * short_plmn_name, uint8_t is_fill_eons_needed, uint8_t is_static_table, uint8_t is_default_oprtr_name, uint8_t is_always_send_plmn_name, uint8_t is_se_table, uint8_t is_current_plmn);

void util_to_get_operator_name_3gpp2(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, const char *mcc,
const char *mnc, uint16_t sid, uint16_t nid, const char *long_plmn_name, const char * short_plmn_name, uint8_t is_3gpp2_subs_info_needed, qmi_result_type_v01 result,qmi_error_type_v01 error, uint8_t is_fill_eons_needed );

void util_to_get_operator_name_oos_limited_case(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, uint8_t is_gsm,
const char * mcc, const char * mnc, const char *long_eons, const char *short_eons);

void util_ril_utf_network_scan ( Testcase &t, AndroidTelTestnode &RIL_node, QMIRILTestnode &QMI_node, int scan_net_type, int qmi_resp, int qmi_error,int res, int nw_cnt, int is_legacy, int is_combine, int is_plmn_diff_nw_status);

void util_ril_utf_wcdma_signal_strength ( Testcase &t, AndroidTelTestnode &RIL_node, QMIRILTestnode &QMI_node, int8_t rssi, int16_t ecio, uint8_t is_unsol, uint8_t is_default);

void util_ril_utf_gsm_signal_strength ( Testcase &t, AndroidTelTestnode &RIL_node, QMIRILTestnode &QMI_node, int8_t rssi, uint8_t is_unsol, uint8_t is_err_rate, uint8_t is_default);

void util_ril_utf_tdscdma_signal_strength ( Testcase &t, AndroidTelTestnode &RIL_node, QMIRILTestnode &QMI_node, int8_t rscp, uint8_t is_unsol, uint8_t is_err_rate, uint8_t is_default);

void util_ril_utf_lte_signal_strength ( Testcase &t, AndroidTelTestnode &RIL_node, QMIRILTestnode &QMI_node, int8_t rssi, int8_t rsrq,
int16_t rsrp, int16_t snr, uint8_t is_unsol, uint8_t is_default);

void util_ril_utf_cdma_signal_strength ( Testcase &t, AndroidTelTestnode &RIL_node, QMIRILTestnode &QMI_node, int8_t rssi, int16_t ecio, uint8_t is_unsol, uint8_t is_default);

void conv_qmi_rssi_to_gw_signal_strength (uint16 rssi, int *signal_strength_ptr);

void util_ril_utf_set_multisim_prop ( Testcase &t, AndroidTelTestnode &RIL_node, QMIRILTestnode &QMI_node, uint8_t max_subscriptions,
uint32_t subs_config_list_len, uint8_t max_active_dsds, uint8 max_active_dsda);

void util_ril_utf_set_uicc_subs ( Testcase &t, AndroidTelTestnode &RIL_node, QMIRILTestnode &QMI_node, int app_index, int slot,
RIL_UiccSubActStatus act_status, RIL_SubscriptionType sub_type, uint8_t is_dsda2, uint8_t is_subs_info, int res, qmi_result_type_v01 result, qmi_error_type_v01 error);

void util_wms_service_ready(Testcase &t, AndroidTelTestnode &RIL_node, OEMTestnode &OEM_node, QMIRILTestnode &QMI_node, wms_service_ready_status_enum_v01 wms_tech);

void util_sms_over_ims_reg_state(Testcase &t, AndroidTelTestnode &RIL_node, QMIRILTestnode &QMI_node, uint8_t registered_ind, qmi_result_type_v01 nw_result, qmi_error_type_v01 nw_error, uint8_t nw_reg_status_valid, wms_transport_nw_reg_status_enum_v01 nw_reg_status, uint8_t is_transport_layer_info_needed, qmi_result_type_v01 layer_result, qmi_error_type_v01 layer_error, uint8_t layer_registered_ind_valid, uint8_t layer_registered_ind, uint8_t transport_layer_info_valid, wms_transport_type_enum_v01 transport_type,
wms_transport_capability_enum_v01 transport_cap, uint8_t is_empty, int reg_status, int rat, int atel_status);

void util_sms_over_ims_send_sms_3gpp(Testcase &t, AndroidTelTestnode &RIL_node, QMIRILTestnode &QMI_node, unsigned char retry, int messageRef, int sms_over_ims, int atel_status, qmi_result_type_v01 result, qmi_error_type_v01 error, uint16_t msgRef, uint8_t is_delivery_failure, wms_message_delivery_failure_type_enum_v01 message_delivery_failure_type, uint8_t is_wms_available);

void util_sms_over_ims_send_sms_3gpp2(Testcase &t, AndroidTelTestnode &RIL_node, QMIRILTestnode &QMI_node, unsigned char retry, int messageRef, int sms_over_ims, int atel_status, qmi_result_type_v01 result, qmi_error_type_v01 error, uint16_t msgRef, uint8_t is_delivery_failure, wms_message_delivery_failure_type_enum_v01 message_delivery_failure_type, uint8_t is_retry_wms_available);

void util_to_update_cache_current_plmn_ind(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, uint8_t plmn_valid, uint16_t mcc, uint16_t mnc, uint8_t pcs_digit, uint8_t spn_valid, nas_coding_scheme_enum_v01 spn_enc, uint32_t spn_len, const char *spn, uint8_t short_name_valid, nas_coding_scheme_enum_v01 short_plmn_name_enc, uint32_t short_plmn_name_len, const char *short_name, uint8_t long_name_valid, nas_coding_scheme_enum_v01 long_plmn_name_enc, uint32_t long_plmn_name_len, const char *long_name);

void util_ril_utf_test_screen_state(Testcase &t, AndroidTelTestnode &RIL_node, QMIRILTestnode &QMI_node, int screen_state, boolean include_voice, uint8_t is_sig_info_needed, uint8_t is_err_needed);

void util_ril_utf_get_subscription_info(Testcase &t, QMIRILTestnode &QMI_node, qmi_result_type_v01 result, qmi_error_type_v01 error, uint8_t is_priority_subs_valid, nas_is_priority_subs_enum_v01 is_priority_subs, uint8_t is_active_valid, nas_active_subs_info_enum_v01 is_active, uint8_t is_default_data_subs_valid, uint8_t is_default_data_subs, uint8_t voice_system_id_valid, uint32_t voice_system_id, uint8_t lte_voice_system_id_valid, uint32_t lte_voice_system_id, uint8_t wlan_voice_system_id_valid, uint32_t wlan_voice_system_id);

void util_ril_utf_test_set_lte_tune_away(Testcase &t, OEMTestnode &OEM_node, QMIRILTestnode &QMI_node, boolean lte_tune_away, int is_modem_comm_needed, int qmi_resp, int qmi_error, int res);

void util_ril_utf_get_sys_info_for_lte_attached(Testcase &t, QMIRILTestnode &QMI_node, nas_service_domain_enum_type_v01 lte_srv_domain);

void util_ril_utf_detach_apn(Testcase &t, QMIRILTestnode &QMI_node, qmi_result_type_v01 result, qmi_error_type_v01 error, nas_srv_domain_pref_enum_type_v01 srv_domain_pref);

void util_ril_utf_attach_apn(Testcase &t, QMIRILTestnode &QMI_node, qmi_result_type_v01 result, qmi_error_type_v01 error, nas_srv_domain_pref_enum_type_v01 srv_domain_pref);

void util_ril_utf_on_demand_attach_apn(Testcase &t, QMIRILTestnode &QMI_node, qmi_result_type_v01 result, qmi_error_type_v01 error, nas_srv_domain_pref_enum_type_v01 srv_domain_pref);

void util_ril_utf_on_demand_attach_apn(Testcase &t, QMIRILTestnode &QMI_node, qmi_result_type_v01 result, qmi_error_type_v01 error, nas_srv_domain_pref_enum_type_v01 srv_domain_pref, mode_pref_mask_type_v01 mask);

void util_ril_utf_dsd_ind_info(Testcase &t, AndroidTelTestnode &RIL_node, OEMTestnode &OEM_node, QMIRILTestnode &QMI_node, uint8_t avail_sys_valid, uint32_t avail_sys_len, dsd_sys_network_enum_v01 technology, dsd_sys_rat_ex_enum_v01 rat_value, dsd_sys_so_mask_v01 so_mask, uint8_t is_dsd_diff, uint8_t is_wlan_status, uint8_t is_sig_info_needed, uint8_t is_err_needed, int rat, uint8_t is_multi_rat,
uint8_t is_wlan_imss);

void util_ril_utf_set_dual_standby_pref(Testcase &t, QMIRILTestnode &QMI_node, qmi_result_type_v01 result, qmi_error_type_v01 error, nas_srv_domain_pref_enum_type_v01 srv_domain_pref, nas_dds_duration_enum_v01 dds_duration);

int db_property_set(const char *key, const char *value);

void util_ril_utf_db_property_init(Testcase &t, DBTestnode &DB_node);

void util_ril_utf_insert_1_qmi_uim_get_label_call(QMIRILTestnode &QMI_node, Testcase &t);

void util_ril_utf_insert_4_qmi_uim_get_label_calls(QMIRILTestnode &QMI_node, Testcase &t);

void util_ril_utf_init_3gpp_sim_app(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode& RIL_node, uim_app_state_enum_v01 app_state);

void util_ril_utf_init_3gpp2_sim_app(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode& RIL_node);

void util_ril_utf_init_hw_mbn(Testcase &t, int sim_num);

void util_ril_utf_init_sw_mbn(Testcase &t, int rild_num);

void util_ril_utf_init_sw_mbn(Testcase &t, int rild_num, const char* sw_config_id_0 , char *sw_config_file_1, const char* sw_config_id_1, uint32_t sw_config_len_1);

void util_ril_utf_init_msim_mbn_properties(int mbn_loaded);

void util_ril_utf_init_no_sim_mbn_properties(int mbn_loaded);

void util_ril_utf_qmi_pdc_disable_modem_update(Testcase &t);

void util_ril_utf_qmi_pdc_get_selected_config(Testcase &t, const uint8_t* active_config_id, const uint8_t* pending_config_id, pdc_config_type_enum_v01 type, int rild_num);

void util_ril_utf_qmi_pdc_deactivate_config(Testcase &t, pdc_config_type_enum_v01 type, int rild_num);

void util_ril_utf_qmi_pdc_delete_config(Testcase &t, uint8_t* config_id, pdc_config_type_enum_v01 type);

void util_ril_utf_qmi_pdc_load_config(Testcase &t, char *file_name, uint8_t* config_id, const uint32_t config_len, pdc_config_type_enum_v01 type);

void util_ril_utf_qmi_pdc_set_selected_config(Testcase &t, uint8_t* config_id, pdc_config_type_enum_v01 type, int rild_num);

void util_ril_utf_qmi_pdc_activate_config(Testcase &t, pdc_config_type_enum_v01 type, int rild_num, int n_pendings);

void util_ril_utf_qmi_pdc_cleanup_config(Testcase &t, pdc_config_type_enum_v01 type, int rild_num);

void util_ril_utf_qmi_pdc_get_config_info(Testcase &t, uint8_t* config_id);

void util_ril_utf_imsa_registration_status(
     Testcase &t,
     IMSTestnode &IMS_node,
     QMIRILTestnode &QMI_node,
     uint8_t ims_registered,
     uint8_t ims_registration_failure_error_code_valid,
     uint16_t ims_registration_failure_error_code,
     uint8_t ims_reg_status_valid,
     imsa_ims_registration_status_enum_v01 ims_reg_status,
     uint8_t registration_error_string_valid,
     char* registration_error_string,
     int has_state,
     ims_Registration_RegState state
);

void util_ril_utf_imsa_service_status(
     Testcase &t,
     IMSTestnode &IMS_node,
     QMIRILTestnode &QMI_node,
     imsa_service_status_enum_v01 sms_status,
     imsa_service_status_enum_v01 voip_status,
     imsa_service_status_enum_v01 vt_status,
     imsa_service_rat_enum_v01 sms_rat,
     imsa_service_rat_enum_v01 voip_rat,
     imsa_service_rat_enum_v01 vt_rat
);

void util_to_get_operator_name_3gpp_oprtr_mcc_mnc(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, const char *mcc_ptr, const char *mnc_ptr, uint8_t csg_id_valid, uint32_t csg_id, nas_radio_if_enum_v01 rat, const char *modem_long_plmn_name, const char * modem_short_plmn_name, const char *atel_long_plmn_name, const char * atel_short_plmn_name, uint8_t is_fill_eons_needed, uint8_t is_default_oprtr_name, uint8_t is_always_send_plmn_name, uint8_t is_se_table);
void util_ril_utf_dms_event_ind_info(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, uint8_t max_active_data_subscriptions_valid, uint8_t max_active_data_subscriptions);
void util_ril_utf_get_device_cap_info(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, OEMTestnode &OEM_node, uint8_t max_active_data_subscriptions_valid, uint8_t max_active_data_subscriptions, uint8_t is_max_data_change);

void util_ril_utf_physical_channel_configuration_query_sequence(Testcase &t, QMIRILTestnode &QMI_node);

#endif //_RIL_UTF_TEST_UTILITY_H
