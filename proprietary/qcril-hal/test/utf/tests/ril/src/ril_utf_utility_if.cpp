/******************************************************************************
#  Copyright (c) 2014 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_utility_if.cpp
  @brief   RIL UTF utility functions definitions for NAS

  DESCRIPTION
  All NAS related utility functions definitions should be in this file
  ---------------------------------------------------------------------------
******************************************************************************/

#include "ril_utf_utility_if.h"
#include "ril_utf_test_qmi_ril.h"

#define NOTUSED(x) (void) x

void util_ril_utf_test_cdma_set_subscription_source_with_spc (
     Testcase &t,
     OEMTestnode &OEM_node,
     QMIRILTestnode &QMI_node,
     int sub_source,
     char *spc,
     int spc_len,
     int qmi_resp,
     int qmi_error,
     int res,
     int resp_data
)
{
    char tags[MAX_TAG_SIZE];
    nas_rtre_cfg_enum_v01 rtre_cfg_pref;
    if ( sub_source == SUBSCRIPTION_SOURCE_RUIM_SIM )
    {
        rtre_cfg_pref = NAS_RTRE_CFG_RUIM_ONLY_V01;
    }
    else if (  sub_source == CDMA_SUBSCRIPTION_SOURCE_NV )
    {
        rtre_cfg_pref = NAS_RTRE_CFG_INTERNAL_SETTINGS_ONLY_V01;
    }
    else if ( sub_source == SUBSCRIPTION_SOURCE_RUIM_IF_AVAIL )
    {
        rtre_cfg_pref = NAS_RTRE_CFG_RUIM_IF_AVAIL_V01;
    }

    t.set_start();

    // Inject: QCRIL_EVT_HOOK_SET_CDMA_SUB_SRC_WITH_SPC
    OEM_node.update_default_oem_request_cdma_set_subscription_source((oem_hook_cdma_subscription_source_e_type)sub_source, spc, spc_len);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "Android SUB Source = %d, spc_len = %d", sub_source, spc_len);
    OEM_node.add_callflow_tag(tags);
    t.inject(OEM_node);

    if( !( (spc_len > 6) || (spc_len < 1) ) )
    {
       // Expect: QMI_NAS_SET_RTRE_CONFIG_REQ_MSG_V01
       QMI_node.update_default_qmi_nas_set_rtre_config_req_msg(rtre_cfg_pref, TRUE, spc, spc_len);
       memset(tags,'\0', MAX_TAG_SIZE);
       snprintf(tags, MAX_TAG_SIZE, "With SPC TLV, rtre_cfg_pref = %d", rtre_cfg_pref);
       QMI_node.add_callflow_tag(tags);
       t.expect(QMI_node);

       // Inject: QMI_NAS_SET_RTRE_CONFIG_RESP_MSG_V01
       QMI_node.update_default_qmi_nas_set_rtre_config_resp_msg((qmi_result_type_v01)qmi_resp, (qmi_error_type_v01)qmi_error);
       memset(tags,'\0', MAX_TAG_SIZE);
       snprintf(tags, MAX_TAG_SIZE, "RESP = %d, ERROR = %d", qmi_resp, qmi_error);
       QMI_node.add_callflow_tag(tags);
       t.inject(QMI_node);
    }

    // Expect: QCRIL_EVT_HOOK_SET_CDMA_SUB_SRC_WITH_SPC_RESP
    OEM_node.update_default_oem_request_cdma_set_subscription_source_resp(resp_data);
    OEM_node.set_ril_error_value((RIL_Errno)res);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "Response Result = %d", res);
    OEM_node.add_callflow_tag(tags);
    t.expect(OEM_node);

    t.set_end();
}
void util_ril_utf_test_cdma_set_roaming_preference (
     Testcase &t,
     AndroidTelTestnode &RIL_node,
     QMIRILTestnode &QMI_node,
     ril_cdma_roaming_preference_enum_type cdma_roam_pref,
     int res
)
{
  char tags[MAX_TAG_SIZE];
  nas_roam_pref_enum_v01 roam_pref;
  switch(cdma_roam_pref)
  {
    case 0:
       roam_pref = NAS_ROAMING_PREF_OFF_V01;
       break;
    case 1:
       roam_pref = NAS_ROAMING_PREF_NOT_FLASING_V01;
       break;
    case 2:
       roam_pref = NAS_ROAMING_PREF_ANY_V01;
       break;
    default:
       roam_pref = NAS_ROAMING_PREF_OFF_V01;
  }

  t.set_start();

  // Inject: RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE
  ril_request_cdma_set_roaming_preference_t* cdma_set_roam_pref = RIL_node.update_default_ril_request_cdma_set_roaming_preference();
  cdma_set_roam_pref->roaming_preference = cdma_roam_pref;
  memset(tags,'\0', MAX_TAG_SIZE);
  snprintf(tags, MAX_TAG_SIZE, "Android Roam Preference = %d", cdma_roam_pref);
  RIL_node.add_callflow_tag(tags);
  t.inject(RIL_node);

  // Expect: QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_set_system_selection_preference_req_msg();
  QMI_node.set_qmi_nas_system_selection_preference_req_roam_pref(roam_pref);
  memset(tags,'\0', MAX_TAG_SIZE);
  snprintf(tags, MAX_TAG_SIZE, "Modem Roam Preference = %d", roam_pref);
  QMI_node.add_callflow_tag(tags);
  t.expect(QMI_node);

  if ( res == RIL_E_SUCCESS )
  {
    // Inject: QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01
    QMI_node.update_default_qmi_nas_set_system_selection_preference_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "Response Success");
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    // Expect: RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE_RESP
    RIL_node.update_default_ril_request_cdma_set_roaming_preference_resp();
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "Android Response Success");
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);
  }
  else
  {
    // Inject: QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01
    QMI_node.update_default_qmi_nas_set_system_selection_preference_resp_msg(QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "Response Failure");
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    // Expect: RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE_RESP
    RIL_node.update_default_ril_request_cdma_set_roaming_preference_resp();
    RIL_node.set_ril_error_value(RIL_E_MODEM_ERR);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "Android Response Failure");
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);
  }

  t.set_end();
}

void util_ril_utf_test_cdma_query_roaming_preference (
     Testcase &t,
     AndroidTelTestnode &RIL_node,
     QMIRILTestnode &QMI_node,
     nas_roam_pref_enum_v01 roam_pref,
     int res
)
{
  char tags[MAX_TAG_SIZE];
  ril_cdma_roaming_preference_enum_type cdma_roam_pref;
  switch(roam_pref)
  {
    case NAS_ROAMING_PREF_OFF_V01:
      cdma_roam_pref = (ril_cdma_roaming_preference_enum_type)0;
      break;
    case NAS_ROAMING_PREF_NOT_FLASING_V01:
      cdma_roam_pref = (ril_cdma_roaming_preference_enum_type)1;
      break;
    case NAS_ROAMING_PREF_ANY_V01:
      cdma_roam_pref = (ril_cdma_roaming_preference_enum_type)2;
      break;
    default:
      cdma_roam_pref = (ril_cdma_roaming_preference_enum_type)-1;
   }

   t.set_start();

  // Inject: RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE
  RIL_node.update_default_ril_request_cdma_query_roaming_preference();
  t.inject(RIL_node);

  // Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_V01
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  if ( res == RIL_E_SUCCESS )
  {
    // Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01
    QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
    QMI_node.set_qmi_nas_get_system_selection_preference_resp_roam_pref(roam_pref);
    QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "Modem Response Suzzess: Roam_Pref = %d", roam_pref);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    if( cdma_roam_pref != -1 )
    {
      //Expect: RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE_RESP
      ril_request_cdma_query_roaming_preference_resp_t* cdma_set_roam_pref = RIL_node.update_default_ril_request_cdma_query_roaming_preference_resp();
      cdma_set_roam_pref->roaming_preference = cdma_roam_pref;
      memset(tags,'\0', MAX_TAG_SIZE);
      snprintf(tags, MAX_TAG_SIZE, "Android Response Suzzess: Roam_Pref = %d", cdma_roam_pref);
      RIL_node.add_callflow_tag(tags);
      t.expect(RIL_node);
    }
    else
    {
      // Expect: RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE_RESP
      RIL_node.update_default_ril_request_cdma_query_roaming_preference_resp_failure();
      RIL_node.set_ril_error_value(RIL_E_MODEM_ERR);
      memset(tags,'\0', MAX_TAG_SIZE);
      snprintf(tags, MAX_TAG_SIZE, "Android Response Failure");
      RIL_node.add_callflow_tag(tags);
      t.expect(RIL_node);
    }
  }
  else
  {
    // Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01
    QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
    QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "Modem Response Failure");
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    // Expect: RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE_RESP
    RIL_node.update_default_ril_request_cdma_query_roaming_preference_resp_failure();
    RIL_node.set_ril_error_value(RIL_E_MODEM_ERR);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "Android Response Failure");
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);
  }

  t.set_end();
}

void util_ril_utf_test_cdma_set_subscription_source (
  Testcase &t,
  AndroidTelTestnode &RIL_node,
  QMIRILTestnode &QMI_node,
  int sub_source,
  char *spc,
  int spc_len,
  int res
)
{
   char tags[MAX_TAG_SIZE];
   nas_rtre_cfg_enum_v01 rtre_cfg_pref;
   if ( sub_source == CDMA_SUBSCRIPTION_SOURCE_RUIM_SIM )
   {
     rtre_cfg_pref = NAS_RTRE_CFG_RUIM_ONLY_V01;
   }
   else if (  sub_source == CDMA_SUBSCRIPTION_SOURCE_NV )
   {
     rtre_cfg_pref = NAS_RTRE_CFG_INTERNAL_SETTINGS_ONLY_V01;
   }

   t.set_start();

  // Inject: RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE
   RIL_CdmaSubscriptionSource* cdma_sub_source = RIL_node.update_default_ril_request_cdma_set_subscription_source();
   *cdma_sub_source = (RIL_CdmaSubscriptionSource)sub_source;
   memset(tags,'\0', MAX_TAG_SIZE);
   snprintf(tags, MAX_TAG_SIZE, "Android sub_source = %d", sub_source);
   RIL_node.add_callflow_tag(tags);
   t.inject(RIL_node);

   // Expect: QMI_NAS_SET_RTRE_CONFIG_REQ_MSG_V01
   QMI_node.update_default_qmi_nas_set_rtre_config_req_msg(rtre_cfg_pref,FALSE, spc, spc_len);
   memset(tags,'\0', MAX_TAG_SIZE);
   snprintf(tags, MAX_TAG_SIZE, "RTRE_CFG_PREF = %d, without SPC TLV", rtre_cfg_pref);
   QMI_node.add_callflow_tag(tags);
   t.expect(QMI_node);

  if ( res == RIL_E_SUCCESS )
   {
     // Inject: QMI_NAS_SET_RTRE_CONFIG_RESP_MSG_V01
     QMI_node.update_default_qmi_nas_set_rtre_config_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
     memset(tags,'\0', MAX_TAG_SIZE);
     snprintf(tags, MAX_TAG_SIZE, "Response Success");
     QMI_node.add_callflow_tag(tags);
     t.inject(QMI_node);

     // Expect: RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE_RESP
     RIL_node.update_default_ril_request_cdma_set_subscription_source_resp();
     memset(tags,'\0', MAX_TAG_SIZE);
     snprintf(tags, MAX_TAG_SIZE, "Android Response Success");
     RIL_node.add_callflow_tag(tags);
     t.expect(RIL_node);
   }
   else
   {
     // Inject: QMI_NAS_SET_RTRE_CONFIG_RESP_MSG_V01
     QMI_node.update_default_qmi_nas_set_rtre_config_resp_msg(QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
     memset(tags,'\0', MAX_TAG_SIZE);
     snprintf(tags, MAX_TAG_SIZE, "Response Failure");
     QMI_node.add_callflow_tag(tags);
     t.inject(QMI_node);

    // Expect: RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE_RESP
    RIL_node.update_default_ril_request_cdma_set_subscription_source_resp();
    RIL_node.set_ril_error_value(RIL_E_MODEM_ERR);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "Android Response Failure");
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);
   }

   t.set_end();
}

void util_ril_utf_test_cdma_get_subscription_source (
  Testcase &t,
  AndroidTelTestnode &RIL_node,
  QMIRILTestnode &QMI_node,
  nas_rtre_cfg_enum_v01 rtre_cfg_pref,
  int res
)
{
   char tags[MAX_TAG_SIZE];
   int sub_source;
   if ( rtre_cfg_pref == NAS_RTRE_CFG_RUIM_ONLY_V01 )
   {
     sub_source = CDMA_SUBSCRIPTION_SOURCE_RUIM_SIM;
   }
   else if (  rtre_cfg_pref == NAS_RTRE_CFG_INTERNAL_SETTINGS_ONLY_V01 )
   {
     sub_source = CDMA_SUBSCRIPTION_SOURCE_NV;
   }
   else
   {
     sub_source = CDMA_SUBSCRIPTION_SOURCE_RUIM_SIM;
   }

   t.set_start();

  // Inject: RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE
   RIL_node.update_default_ril_request_cdma_get_subscription_source();
   t.inject(RIL_node);

  // Expect: QMI_NAS_GET_RTRE_CONFIG_REQ_MSG_V01
   QMI_node.update_default_qmi_nas_get_rtre_config_req_msg();
   t.expect(QMI_node);

  if ( res == RIL_E_SUCCESS )
  {
     // Inject: QMI_NAS_GET_RTRE_CONFIG_RESP_MSG_V01
     QMI_node.update_default_qmi_nas_get_rtre_config_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
     QMI_node.set_qmi_nas_get_rtre_config_resp_rtre_cfg(rtre_cfg_pref);
     memset(tags,'\0', MAX_TAG_SIZE);
     snprintf(tags, MAX_TAG_SIZE, "Modem RTRE Confg Pref = %d",rtre_cfg_pref);
     QMI_node.add_callflow_tag(tags);
     t.inject(QMI_node);

     if (! ( ( rtre_cfg_pref == NAS_RTRE_CFG_RUIM_ONLY_V01) || ( rtre_cfg_pref == NAS_RTRE_CFG_INTERNAL_SETTINGS_ONLY_V01) ) )
     {
        // Expect: RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE_RESP
        RIL_node.update_default_ril_request_cdma_get_subscription_source_resp_failure();
        RIL_node.set_ril_error_value(RIL_E_MODEM_ERR);
        memset(tags,'\0', MAX_TAG_SIZE);
        snprintf(tags, MAX_TAG_SIZE, "Android Response Failure");
        RIL_node.add_callflow_tag(tags);
        t.expect(RIL_node);
     }
     else
     {
        //Expect: RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE_RESP
        RIL_CdmaSubscriptionSource* cdma_sub_source = RIL_node.update_default_ril_request_cdma_get_subscription_source_resp();
        RIL_node.update_default_ril_request_cdma_get_subscription_source_resp(sub_source);
        memset(tags,'\0', MAX_TAG_SIZE);
        snprintf(tags, MAX_TAG_SIZE, "Android CDMA SUB SRC = %d", sub_source);
        RIL_node.add_callflow_tag(tags);
        t.expect(RIL_node);
    }
  }
  else
  {
    // Inject: QMI_NAS_GET_RTRE_CONFIG_RESP_MSG_V01
    QMI_node.update_default_qmi_nas_get_rtre_config_resp_msg(QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "Response Failure");
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    // Expect: RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE_RESP
    RIL_node.update_default_ril_request_cdma_get_subscription_source_resp_failure();
    RIL_node.set_ril_error_value(RIL_E_MODEM_ERR);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "Android Response Failure");
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);
  }

  t.set_end();
}
void conv_android_mode_pref_to_qmi_mode_pref(
     uint16_t android_mode_pref,
     uint16_t *qmi_mode_pref,
     uint8_t *qmi_gw_acq_order_pref_valid,
     uint16_t *qmi_gw_acq_order_pref
)
{
    switch ( android_mode_pref )
    {
        case QCRIL_PREF_NET_TYPE_GSM_WCDMA:
            *qmi_mode_pref = QMI_NAS_RAT_MODE_PREF_GSM_UMTS;
            *qmi_gw_acq_order_pref_valid = TRUE;
            *qmi_gw_acq_order_pref = NAS_GW_ACQ_ORDER_PREF_WCDMA_GSM_V01;
            break;

        case QCRIL_PREF_NET_TYPE_GSM_ONLY:
            *qmi_mode_pref = QMI_NAS_RAT_MODE_PREF_GSM;
            break;

        case QCRIL_PREF_NET_TYPE_WCDMA:
            *qmi_mode_pref = QMI_NAS_RAT_MODE_PREF_UMTS;
            break;

        case QCRIL_PREF_NET_TYPE_GSM_WCDMA_AUTO:
            *qmi_mode_pref = QMI_NAS_RAT_MODE_PREF_GSM_UMTS;
            *qmi_gw_acq_order_pref_valid = TRUE;
            *qmi_gw_acq_order_pref = NAS_GW_ACQ_ORDER_PREF_AUTOMATIC_V01;
            break;

        case QCRIL_PREF_NET_TYPE_CDMA_EVDO_AUTO:
            *qmi_mode_pref = QMI_NAS_RAT_MODE_PREF_CDMA_HRPD;
            break;

        case QCRIL_PREF_NET_TYPE_CDMA_ONLY:
            *qmi_mode_pref = QMI_NAS_RAT_MODE_PREF_CDMA;
            break;

        case QCRIL_PREF_NET_TYPE_EVDO_ONLY:
            *qmi_mode_pref = QMI_NAS_RAT_MODE_PREF_HRPD;
            break;

        case QCRIL_PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
            *qmi_mode_pref = QMI_NAS_RAT_MODE_PREF_GSM_UMTS_CDMA_HRPD;
            break;

        case QCRIL_PREF_NET_TYPE_LTE_CDMA_EVDO:
            *qmi_mode_pref = QMI_NAS_RAT_MODE_PREF_CDMA_HRPD_LTE;
            break;

        case QCRIL_PREF_NET_TYPE_LTE_GSM_WCDMA:
            *qmi_mode_pref = QMI_NAS_RAT_MODE_PREF_GSM_UMTS_LTE;
            break;

        case QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
            *qmi_mode_pref = QMI_NAS_RAT_MODE_PREF_GSM_UMTS_CDMA_HRPD_LTE;
            break;

        case QCRIL_PREF_NET_TYPE_LTE_ONLY:
            *qmi_mode_pref = QMI_NAS_RAT_MODE_PREF_LTE;
            break;

        case QCRIL_PREF_NET_TYPE_LTE_WCDMA:
            *qmi_mode_pref = QMI_NAS_RAT_MODE_PREF_UMTS_LTE;
            break;

        case QCRIL_PREF_NET_TYPE_TD_SCDMA_ONLY:
            *qmi_mode_pref = QMI_NAS_RAT_MODE_PREF_TDSCDMA;
            break;

        case QCRIL_PREF_NET_TYPE_TD_SCDMA_WCDMA:
            *qmi_mode_pref = QMI_NAS_RAT_MODE_PREF_UMTS_TDSCDMA;
            break;

        case QCRIL_PREF_NET_TYPE_TD_SCDMA_LTE:
            *qmi_mode_pref = QMI_NAS_RAT_MODE_PREF_TDSCDMA_LTE;
            break;

        case QCRIL_PREF_NET_TYPE_TD_SCDMA_GSM:
            *qmi_mode_pref = QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA;
            break;

        case QCRIL_PREF_NET_TYPE_TD_SCDMA_WCDMA_LTE:
            *qmi_mode_pref = QMI_NAS_RAT_MODE_PREF_UMTS_TDSCDMA_LTE;
            break;

        case QCRIL_PREF_NET_TYPE_TD_SCDMA_GSM_LTE:
            *qmi_mode_pref = QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA_LTE;
            break;

        case QCRIL_PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA:
            *qmi_mode_pref = QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA;
            break;

        case QCRIL_PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_LTE:
            *qmi_mode_pref = QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_LTE;
            break;

        case QCRIL_PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA:
            *qmi_mode_pref = QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_CDMA_HRPD_LTE;
            break;

        case QCRIL_PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO:
            *qmi_mode_pref = QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA_CDMA_HRPD_UMTS;
            break;

        default:
            *qmi_mode_pref = QMI_NAS_RAT_MODE_PREF_GSM_UMTS;
            break;
    }
}

void conv_qmi_mode_pref_to_android_mode_pref(
     uint16_t mode_pref,
     uint8_t gw_acq_order_pref_valid,
     uint16_t gw_acq_order_pref,
     RIL_PreferredNetworkType *android_mode_pref
)
{
    switch ( mode_pref )
    {
        case QMI_NAS_RAT_MODE_PREF_CDMA:
            *android_mode_pref = QCRIL_PREF_NET_TYPE_CDMA_ONLY;
            break;

        case QMI_NAS_RAT_MODE_PREF_HRPD:
            *android_mode_pref = QCRIL_PREF_NET_TYPE_EVDO_ONLY;
            break;

        case QMI_NAS_RAT_MODE_PREF_GSM:
            *android_mode_pref = QCRIL_PREF_NET_TYPE_GSM_ONLY;
            break;

        case QMI_NAS_RAT_MODE_PREF_UMTS:
            *android_mode_pref = QCRIL_PREF_NET_TYPE_WCDMA;
            break;

        case QMI_NAS_RAT_MODE_PREF_LTE:
            *android_mode_pref = QCRIL_PREF_NET_TYPE_LTE_ONLY;
            break;

        case QMI_NAS_RAT_MODE_PREF_GSM_UMTS:
            *android_mode_pref = QCRIL_PREF_NET_TYPE_GSM_WCDMA_AUTO;
            if(gw_acq_order_pref_valid == TRUE)
            {
                if (gw_acq_order_pref == NAS_GW_ACQ_ORDER_PREF_WCDMA_GSM_V01)
                {
                    *android_mode_pref = QCRIL_PREF_NET_TYPE_GSM_WCDMA;
                }
            }
            break;

        case QMI_NAS_RAT_MODE_PREF_CDMA_HRPD:
            *android_mode_pref = QCRIL_PREF_NET_TYPE_CDMA_EVDO_AUTO;
            break;

        case QMI_NAS_RAT_MODE_PREF_GSM_UMTS_CDMA_HRPD:
            *android_mode_pref = QCRIL_PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO;
            break;

        case QMI_NAS_RAT_MODE_PREF_CDMA_HRPD_LTE:
            *android_mode_pref = QCRIL_PREF_NET_TYPE_LTE_CDMA_EVDO;
            break;

        case QMI_NAS_RAT_MODE_PREF_GSM_UMTS_LTE:
            *android_mode_pref = QCRIL_PREF_NET_TYPE_LTE_GSM_WCDMA;
            break;

        case QMI_NAS_RAT_MODE_PREF_GSM_UMTS_CDMA_HRPD_LTE:
        case QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA_CDMA_HRPD_LTE:
            *android_mode_pref = QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA;
            break;

        case QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_CDMA_HRPD_LTE:
            *android_mode_pref = (RIL_PreferredNetworkType)QCRIL_PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA;
            break;

        case QMI_NAS_RAT_MODE_PREF_UMTS_LTE:
            *android_mode_pref = (RIL_PreferredNetworkType)QCRIL_PREF_NET_TYPE_LTE_WCDMA;
            break;

        case QMI_NAS_RAT_MODE_PREF_TDSCDMA:
            *android_mode_pref = (RIL_PreferredNetworkType)QCRIL_PREF_NET_TYPE_TD_SCDMA_ONLY;
            break;

        case QMI_NAS_RAT_MODE_PREF_UMTS_TDSCDMA:
            *android_mode_pref = (RIL_PreferredNetworkType)QCRIL_PREF_NET_TYPE_TD_SCDMA_WCDMA;
            break;

        case QMI_NAS_RAT_MODE_PREF_TDSCDMA_LTE:
            *android_mode_pref = (RIL_PreferredNetworkType)QCRIL_PREF_NET_TYPE_TD_SCDMA_LTE;
            break;

        case QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA:
            *android_mode_pref = (RIL_PreferredNetworkType)QCRIL_PREF_NET_TYPE_TD_SCDMA_GSM;
            break;

        case QMI_NAS_RAT_MODE_PREF_UMTS_TDSCDMA_LTE:
            *android_mode_pref = (RIL_PreferredNetworkType)QCRIL_PREF_NET_TYPE_TD_SCDMA_WCDMA_LTE;
            break;

        case QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA_LTE:
            *android_mode_pref = (RIL_PreferredNetworkType)QCRIL_PREF_NET_TYPE_TD_SCDMA_GSM_LTE;
            break;

        case QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA:
            *android_mode_pref = (RIL_PreferredNetworkType)QCRIL_PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA;
            break;

        case QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_LTE:
            *android_mode_pref = (RIL_PreferredNetworkType)QCRIL_PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_LTE;
            break;

        case QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA_CDMA_HRPD_UMTS:
            *android_mode_pref = (RIL_PreferredNetworkType)QCRIL_PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO;
            break;

        default:
            *android_mode_pref = QCRIL_PREF_NET_TYPE_GSM_WCDMA;
            break;
    }
}

void util_set_pref_net_type(
     Testcase &t,
     AndroidTelTestnode &RIL_node,
     QMIRILTestnode &QMI_node,
     uint16_t android_mode_pref,
     int rat,
     int qmi_error,
     int android_resp,
     uint32_t timeout,
     uint8_t is_ind_came,
     uint8_t is_sig_needed,
     uint8_t is_err_needed,
     uint8_t is_net_sel_pref,
     uint8_t uim_get_label_num
)
{
  uint16_t qmi_mode_pref;
  uint8 qmi_gw_acq_order_pref_valid = FALSE;
  uint16_t qmi_gw_acq_order_pref = 0 ;
  int net_sel_pref = -1;
  char tags[MAX_TAG_SIZE];

  /* Convert Android mode_pref to RIL mode_pref */
  conv_android_mode_pref_to_qmi_mode_pref(android_mode_pref, &qmi_mode_pref, &qmi_gw_acq_order_pref_valid, &qmi_gw_acq_order_pref);

  t.set_start();

  // Inject: RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE
  RIL_PreferredNetworkType* mode_pref = RIL_node.update_default_ril_request_set_preferred_network_type();
  *mode_pref = (RIL_PreferredNetworkType)android_mode_pref;
  memset(tags,'\0', MAX_TAG_SIZE);
  snprintf(tags, MAX_TAG_SIZE, "android_mode_pref = %d", android_mode_pref);
  RIL_node.add_callflow_tag(tags);
  t.inject(RIL_node);

  // Expect: QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_set_system_selection_preference_req_msg();
  QMI_node.set_qmi_nas_system_selection_preference_req_mode_pref(qmi_mode_pref, qmi_gw_acq_order_pref_valid, qmi_gw_acq_order_pref);
  if(TRUE == is_net_sel_pref)
  {
    nas_net_sel_pref_type_v01* ptr = QMI_node.set_qmi_nas_system_selection_preference_req_net_sel_pref();
    ptr->net_sel_pref = NAS_NET_SEL_PREF_AUTOMATIC_V01;
    net_sel_pref = NAS_NET_SEL_PREF_AUTOMATIC_V01;
  }
  memset(tags,'\0', MAX_TAG_SIZE);
  snprintf(tags, MAX_TAG_SIZE, "modem_mode_pref = %d, net_sel_pref = %d", qmi_mode_pref, net_sel_pref);
  QMI_node.add_callflow_tag(tags);
  t.expect(QMI_node);

  if( (RIL_E_SUCCESS != android_resp) && ( 0 == timeout ) )
  {
    // Inject: QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01
    QMI_node.update_default_qmi_nas_set_system_selection_preference_resp_msg(QMI_RESULT_FAILURE_V01, (qmi_error_type_v01)qmi_error);
    QMI_node.set_expect_timeout(timeout);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "RESP = %d, Error = %d", QMI_RESULT_FAILURE_V01, qmi_error);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    // Expect: RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE_RESP
    RIL_node.update_default_ril_request_set_preferred_network_type_resp();
    RIL_node.set_ril_error_value((RIL_Errno)android_resp);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE_RESP = %d", android_resp);
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);
  }
  else
  {
    // Inject: QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01
    QMI_node.update_default_qmi_nas_set_system_selection_preference_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "RESP = %d, Error = %d, Waiting For Indication since Response Success",
            QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

#if 0 //We have disabled this unsol from qcril to handle delay in UTF execution
    // Expect: RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED
    RIL_node.update_default_ril_unsol_response_voice_network_state_changed();
    t.expect(RIL_node);
#endif

    if(!is_ind_came)
    {
      //Timer to wait for IND
      QMI_node.set_expect_timeout(timeout);
    }

    if( 60000 > timeout )
    {
       //Inject:QMI_NAS_SYSTEM_SELECTION_PREFERENCE_IND_MSG_V01
       QMI_node.update_default_qmi_nas_system_selection_preference_ind_msg();
       QMI_node.set_qmi_nas_system_selection_preference_ind_mode_pref(qmi_mode_pref);
       memset(tags,'\0', MAX_TAG_SIZE);
       snprintf(tags, MAX_TAG_SIZE, "Modem_Mode_Pref = %d", qmi_mode_pref);
       QMI_node.add_callflow_tag(tags);
       t.inject(QMI_node);
    }

    // Expect: RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE_RESP
    RIL_node.update_default_ril_request_set_preferred_network_type_resp();
    RIL_node.set_ril_error_value((RIL_Errno)android_resp);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE_RESP = %d", android_resp);
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);

#ifdef RIL_UTF_L_MASTER
    if (uim_get_label_num == 1 and (android_mode_pref == QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA))
    {
      util_ril_utf_insert_1_qmi_uim_get_label_call(QMI_node, t);
    }
    else if (uim_get_label_num == 4 and (android_mode_pref == QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA))
    {
      util_ril_utf_insert_4_qmi_uim_get_label_calls(QMI_node, t);
    }
#endif

    if( (RIL_E_SUCCESS == android_resp) && (RADIO_TECH_UNKNOWN != rat))
    {
       // Expect: RIL_UNSOL_VOICE_RADIO_TECH_CHANGED
       RIL_node.update_default_ril_unsol_voice_radio_tech_changed();
       RIL_node.set_ril_unsol_voice_radio_tech_changed_radio_tech((RIL_RadioTechnology)rat);
       memset(tags,'\0', MAX_TAG_SIZE);
       snprintf(tags, MAX_TAG_SIZE, "RAT = %d", rat);
       RIL_node.add_callflow_tag(tags);
       t.expect(RIL_node);

       if(is_sig_needed)
       {
           // Expect: QMI_NAS_GET_SIG_INFO_REQ_MSG_V01
           QMI_node.update_default_qmi_nas_get_sig_info_req_msg_v01();
           t.expect(QMI_node);

           // Inject: QMI_NAS_GET_SIG_INFO_RESP_MSG_V01
           QMI_node.update_default_qmi_nas_get_sig_info_resp_msg_v01();
           QMI_node.set_qmi_nas_get_sig_info_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
           t.inject(QMI_node);
       }

       if(is_err_needed)
       {
           // Expect: QMI_NAS_GET_ERR_RATE_REQ_MSG_V01
           QMI_node.update_default_qmi_nas_get_err_rate_req_msg_v01();
           t.expect(QMI_node);

           // Inject: QMI_NAS_GET_ERR_RATE_RESP_MSG_V01
           QMI_node.update_default_qmi_nas_get_err_rate_resp_msg_v01();
           QMI_node.set_qmi_nas_get_err_rate_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
           t.inject(QMI_node);
       }

       // Expect: RIL_UNSOL_SIGNAL_STRENGTH
       RIL_node.update_default_ril_unsol_signal_strength();
       t.expect(RIL_node);
    }
  }

  t.set_end();
}

void util_set_pref_net_type(
     Testcase &t,
     AndroidTelTestnode &RIL_node,
     QMIRILTestnode &QMI_node,
     uint16_t android_mode_pref,
     int rat,
     int qmi_error,
     int android_resp,
     uint32_t timeout,
     uint8_t is_ind_came,
     uint8_t is_sig_needed,
     uint8_t is_err_needed,
     uint8_t is_net_sel_pref
)
{
  util_set_pref_net_type(t, RIL_node, QMI_node, android_mode_pref, rat, qmi_error, android_resp, timeout, is_ind_came, is_sig_needed, is_err_needed, is_net_sel_pref, 4);
}

void util_get_pref_network_type(
     Testcase &t,
     AndroidTelTestnode &RIL_node,
     QMIRILTestnode &QMI_node,
     uint16_t qmi_mode_pref,
     int android_resp,
     int qmi_error,
     uint8_t qmi_gw_acq_order_pref_valid,
     uint16_t qmi_gw_acq_order_pref
)
{
  char tags[MAX_TAG_SIZE];
  t.set_start();

  // Inject: RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE
  RIL_node.update_default_ril_request_get_preferred_network_type();
  t.inject(RIL_node);

  //Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  if(RIL_E_SUCCESS == android_resp)
  {
    //Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP
    QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
    QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(qmi_mode_pref);
    QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "Response Success, Modem Mode Pref = %d", qmi_mode_pref);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    // Expect: RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE_RESP
    RIL_PreferredNetworkType* pref_net_type = RIL_node.update_default_ril_request_get_preferred_network_type_resp();
    /* Convert RIL mode_pref to Android mode_pref */
    conv_qmi_mode_pref_to_android_mode_pref(qmi_mode_pref, qmi_gw_acq_order_pref_valid, qmi_gw_acq_order_pref, pref_net_type);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "Response Success, Android Mode Pref = %p", pref_net_type);
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);
  }
  else
  {
    //Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP
    QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
    QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_FAILURE_V01, (qmi_error_type_v01)qmi_error);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "Response Failure");
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    // Expect: RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE_RESP
    RIL_node.update_default_ril_request_get_preferred_network_type_resp_failure();
    RIL_node.set_ril_error_value((RIL_Errno)android_resp);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "Response Failure");
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);
  }

  t.set_end();
}

void util_voice_radio_tech_oos(
     Testcase &t,
     QMIRILTestnode &QMI_node,
     AndroidTelTestnode &RIL_node,
     int is_rat_change,
     int rat,
     uint8_t uim_get_label_num
)
{
  char tags[MAX_TAG_SIZE];
  t.set_start();

  //Inject: QMI_NAS_SYS_INFO_IND_MSG_V01
  QMI_node.update_default_sys_info_ind_msg();
  nas_3gpp_srv_status_info_type_v01* gsm_srv_status_info = QMI_node.set_qmi_nas_sys_info_ind_gsm_srv_status_info();
  gsm_srv_status_info->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
  gsm_srv_status_info->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
  gsm_srv_status_info->is_pref_data_path = 0;
  nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_info = QMI_node.set_qmi_nas_sys_info_ind_wcdma_srv_status_info();
  wcdma_srv_status_info->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
  wcdma_srv_status_info->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
  nas_3gpp2_srv_status_info_type_v01* cdma_srv_status_info_ind = QMI_node.set_qmi_nas_sys_info_ind_cdma_srv_status_info();
  cdma_srv_status_info_ind->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
  cdma_srv_status_info_ind->is_pref_data_path = 0;
  nas_3gpp_srv_status_info_type_v01* lte_srv_status_info_ind = QMI_node.set_qmi_nas_sys_info_ind_lte_srv_status_info();
  lte_srv_status_info_ind->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
  lte_srv_status_info_ind->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
  lte_srv_status_info_ind->is_pref_data_path = 0;
  memset(tags,'\0', MAX_TAG_SIZE);
  snprintf(tags, MAX_TAG_SIZE, "All Rats are OOS");
  QMI_node.add_callflow_tag(tags);
  t.inject(QMI_node);

  if(is_rat_change)
  {
    util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);

    // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
    nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
    nas_3gpp_srv_status_info_type_v01* gsm_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_gsm_srv_status_info();
    gsm_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_srv_status_info();
    wcdma_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp2_srv_status_info_type_v01* cdma_srv_status_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_cdma_srv_status_info();
    cdma_srv_status_info_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    cdma_srv_status_info_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* lte_srv_status_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_lte_srv_status_info();
    lte_srv_status_info_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    lte_srv_status_info_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    lte_srv_status_info_resp->is_pref_data_path = 0;
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "All Rats are OOS");
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);
  }

  // Expect: RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_voice_network_state_changed();
  t.expect(RIL_node);

#ifdef RIL_UTF_L_MASTER
  if (uim_get_label_num == 1)
  {
    util_ril_utf_insert_1_qmi_uim_get_label_call(QMI_node, t);
  }
  else if (uim_get_label_num == 4)
  {
    util_ril_utf_insert_4_qmi_uim_get_label_calls(QMI_node, t);
  }
#endif

  if( RADIO_TECH_UNKNOWN != rat )
  {
    // Expect: RIL_UNSOL_VOICE_RADIO_TECH_CHANGED
    RIL_node.update_default_ril_unsol_voice_radio_tech_changed();
    RIL_node.set_ril_unsol_voice_radio_tech_changed_radio_tech((RIL_RadioTechnology)rat);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "RAT = %d", rat);
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);

    // Expect: RIL_UNSOL_SIGNAL_STRENGTH
    RIL_node.update_default_ril_unsol_signal_strength();
    t.expect(RIL_node);
  }

  t.set_end();
}

void util_voice_radio_tech_oos(
     Testcase &t,
     QMIRILTestnode &QMI_node,
     AndroidTelTestnode &RIL_node,
     int is_rat_change,
     int rat
)
{
  util_voice_radio_tech_oos(t, QMI_node, RIL_node, is_rat_change, rat, 4);
}

void util_voice_radio_tech_wcdma_in_service(
     Testcase &t,
     QMIRILTestnode &QMI_node,
     AndroidTelTestnode &RIL_node,
     int is_rat_change,
     int rat,
     nas_service_status_enum_type_v01 srv_status,
     nas_true_service_status_enum_type_v01 true_srv_status,
     nas_service_domain_enum_type_v01 domain,
     nas_roam_status_enum_type_v01 roam_status,
     uint8_t net_id_valid,
     const char* mcc,
     const char* mnc
)
{
  char tags[MAX_TAG_SIZE];
  t.set_start();

  //Inject: QMI_NAS_SYS_INFO_IND_MSG_V01
  QMI_node.update_default_sys_info_ind_msg();
  nas_3gpp_srv_status_info_type_v01* gsm_srv_status_info = QMI_node.set_qmi_nas_sys_info_ind_gsm_srv_status_info();
  gsm_srv_status_info->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
  gsm_srv_status_info->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
  gsm_srv_status_info->is_pref_data_path = 0;
  nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_info = QMI_node.set_qmi_nas_sys_info_ind_wcdma_srv_status_info();
  wcdma_srv_status_info->srv_status = srv_status;
  wcdma_srv_status_info->true_srv_status = true_srv_status;
  wcdma_srv_status_info->is_pref_data_path = 0;
  nas_wcdma_sys_info_type_v01* wcdma_sys_info_ind = QMI_node.set_qmi_nas_sys_info_ind_wcdma_sys_info();
  wcdma_sys_info_ind->common_sys_info.srv_domain_valid = TRUE;
  wcdma_sys_info_ind->common_sys_info.srv_domain = domain;
  wcdma_sys_info_ind->common_sys_info.srv_capability_valid = TRUE;
  wcdma_sys_info_ind->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
  wcdma_sys_info_ind->common_sys_info.roam_status_valid = TRUE;
  wcdma_sys_info_ind->common_sys_info.roam_status = roam_status;
  wcdma_sys_info_ind->threegpp_specific_sys_info.network_id_valid = net_id_valid;
  if(NULL != mcc)
  {
    strcpy(wcdma_sys_info_ind->threegpp_specific_sys_info.network_id.mcc,mcc);
  }
  if(NULL != mnc)
  {
    strcpy(wcdma_sys_info_ind->threegpp_specific_sys_info.network_id.mnc,mnc);
  }
  memset(tags,'\0', MAX_TAG_SIZE);
  snprintf(tags, MAX_TAG_SIZE, "WCDMA: srv_status = %d, true_srv_status = %d, srv_domain = %d, roam_status = %d",
        srv_status, true_srv_status, domain, roam_status);
  QMI_node.add_callflow_tag(tags);
  t.inject(QMI_node);

  if(is_rat_change)
  {
    util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);

    // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
    nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
    nas_3gpp_srv_status_info_type_v01* gsm_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_gsm_srv_status_info();
    gsm_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_srv_status_info();
    wcdma_srv_status_resp->srv_status = srv_status;
    wcdma_srv_status_resp->true_srv_status = true_srv_status;
    wcdma_srv_status_resp->is_pref_data_path = 0;
    nas_wcdma_sys_info_type_v01* wcdma_sys_info_resp= QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_sys_info();
    wcdma_sys_info_resp->common_sys_info.srv_domain_valid = TRUE;
    wcdma_sys_info_resp->common_sys_info.srv_domain = domain;
    wcdma_sys_info_resp->common_sys_info.srv_capability_valid = TRUE;
    wcdma_sys_info_resp->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
    wcdma_sys_info_resp->common_sys_info.roam_status_valid = TRUE;
    wcdma_sys_info_resp->common_sys_info.roam_status = roam_status;
    wcdma_sys_info_resp->threegpp_specific_sys_info.network_id_valid = net_id_valid;
    if(NULL != mcc)
    {
      strcpy(wcdma_sys_info_resp->threegpp_specific_sys_info.network_id.mcc,mcc);
    }
    if(NULL != mnc)
    {
      strcpy(wcdma_sys_info_resp->threegpp_specific_sys_info.network_id.mnc,mnc);
    }
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "WCDMA: srv_status = %d, true_srv_status = %d, srv_domain = %d, roam_status = %d",
            srv_status, true_srv_status, domain, roam_status);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);
  }

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

  // Inject: QMI_NAS_GET_ERR_RATE_RSP_MSG_V01
  QMI_node.update_default_qmi_nas_get_err_rate_resp_msg_v01();
  QMI_node.set_qmi_nas_get_err_rate_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: RIL_UNSOL_SIGNAL_STRENGTH
  RIL_node.update_default_ril_unsol_signal_strength();
  t.expect(RIL_node);

  // Expect: RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_voice_network_state_changed();
  t.expect(RIL_node);

  if(RADIO_TECH_UNKNOWN != rat)
  {
    // Expect: RIL_UNSOL_VOICE_RADIO_TECH_CHANGED
    RIL_node.update_default_ril_unsol_voice_radio_tech_changed();
    RIL_node.set_ril_unsol_voice_radio_tech_changed_radio_tech((RIL_RadioTechnology)rat);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "RAT = %d", rat);
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);

    // Expect: RIL_UNSOL_SIGNAL_STRENGTH
    RIL_node.update_default_ril_unsol_signal_strength();
    t.expect(RIL_node);

  }

  t.set_end();
}

void util_voice_radio_tech_wcdma_in_service(
     Testcase &t,
     QMIRILTestnode &QMI_node,
     AndroidTelTestnode &RIL_node,
     int is_rat_change,
     int rat,
     nas_service_status_enum_type_v01 srv_status,
     nas_true_service_status_enum_type_v01 true_srv_status,
     nas_service_domain_enum_type_v01 domain,
     nas_roam_status_enum_type_v01 roam_status
)
{
  util_voice_radio_tech_wcdma_in_service(t, QMI_node, RIL_node, is_rat_change, rat, srv_status, true_srv_status, domain, roam_status, FALSE, NULL, NULL);
}

void util_voice_radio_tech_gsm_in_service(
     Testcase &t,
     QMIRILTestnode &QMI_node,
     AndroidTelTestnode &RIL_node,
     int is_rat_change,
     int rat,
     nas_service_status_enum_type_v01 srv_status,
     nas_true_service_status_enum_type_v01 true_srv_status,
     nas_service_domain_enum_type_v01 domain,
     nas_roam_status_enum_type_v01 roam_status,
     uint8_t is_needed
)
{
  char tags[MAX_TAG_SIZE];
  t.set_start();

  //Inject: QMI_NAS_SYS_INFO_IND_MSG_V01
  QMI_node.update_default_sys_info_ind_msg();
  nas_3gpp_srv_status_info_type_v01* gsm_srv_status_info = QMI_node.set_qmi_nas_sys_info_ind_gsm_srv_status_info();
  gsm_srv_status_info->srv_status = srv_status;
  gsm_srv_status_info->true_srv_status = true_srv_status;
  gsm_srv_status_info->is_pref_data_path = 0;
  nas_gsm_sys_info_type_v01* gsm_sys_info_ind = QMI_node.set_qmi_nas_sys_info_ind_gsm_sys_info();
  gsm_sys_info_ind->common_sys_info.srv_domain_valid = TRUE;
  gsm_sys_info_ind->common_sys_info.srv_domain = domain;
  gsm_sys_info_ind->common_sys_info.srv_capability_valid = TRUE;
  gsm_sys_info_ind->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
  gsm_sys_info_ind->common_sys_info.roam_status_valid = TRUE;
  gsm_sys_info_ind->common_sys_info.roam_status = roam_status;
  nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_info = QMI_node.set_qmi_nas_sys_info_ind_wcdma_srv_status_info();
  wcdma_srv_status_info->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
  wcdma_srv_status_info->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
  wcdma_srv_status_info->is_pref_data_path = 0;
  memset(tags,'\0', MAX_TAG_SIZE);
  snprintf(tags, MAX_TAG_SIZE, "GSM: srv_status = %d, true_srv_status = %d, srv_domain = %d, roam_status = %d",
        srv_status, true_srv_status, domain, roam_status);
  QMI_node.add_callflow_tag(tags);
  t.inject(QMI_node);

  if(is_rat_change)
  {
    util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);

    // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
    nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
    nas_3gpp_srv_status_info_type_v01* gsm_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_gsm_srv_status_info();
    gsm_srv_status_resp->srv_status = srv_status;
    gsm_srv_status_resp->true_srv_status = true_srv_status;
    gsm_srv_status_resp->is_pref_data_path = 0;
    nas_gsm_sys_info_type_v01* gsm_sys_info_resp= QMI_node.set_qmi_nas_get_sys_info_resp_gsm_sys_info();
    gsm_sys_info_resp->common_sys_info.srv_domain_valid = TRUE;
    gsm_sys_info_resp->common_sys_info.srv_domain = domain;
    gsm_sys_info_resp->common_sys_info.srv_capability_valid = TRUE;
    gsm_sys_info_resp->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
    gsm_sys_info_resp->common_sys_info.roam_status_valid = TRUE;
    gsm_sys_info_resp->common_sys_info.roam_status = roam_status;
    nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_srv_status_info();
    wcdma_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->is_pref_data_path = 0;
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "GSM: srv_status = %d, true_srv_status = %d, srv_domain = %d, roam_status = %d",
            srv_status, true_srv_status, domain, roam_status);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);
  }

  if(is_needed)
  {
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

    // Expect: RIL_UNSOL_SIGNAL_STRENGTH
    RIL_node.update_default_ril_unsol_signal_strength();
    t.expect(RIL_node);
  }

  // Expect: RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_voice_network_state_changed();
  t.expect(RIL_node);

  if(RADIO_TECH_UNKNOWN != rat)
  {
    // Expect: RIL_UNSOL_VOICE_RADIO_TECH_CHANGED
    RIL_node.update_default_ril_unsol_voice_radio_tech_changed();
    RIL_node.set_ril_unsol_voice_radio_tech_changed_radio_tech((RIL_RadioTechnology)rat);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "RAT = %d", rat);
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);

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

    // Expect: RIL_UNSOL_SIGNAL_STRENGTH
    RIL_node.update_default_ril_unsol_signal_strength();
    t.expect(RIL_node);

  }

  t.set_end();
}

void util_voice_radio_tech_tdscdma_in_service(
     Testcase &t,
     QMIRILTestnode &QMI_node,
     AndroidTelTestnode &RIL_node,
     int is_rat_change,
     int rat,
     nas_service_status_enum_type_v01 srv_status,
     nas_true_service_status_enum_type_v01 true_srv_status,
     nas_service_domain_enum_type_v01 domain,
     nas_roam_status_enum_type_v01 roam_status,
     uint8_t is_needed
)
{
  char tags[MAX_TAG_SIZE];
  t.set_start();

  //Inject: QMI_NAS_SYS_INFO_IND_MSG_V01
  QMI_node.update_default_sys_info_ind_msg();
  nas_3gpp_srv_status_info_type_v01* gsm_srv_status_info = QMI_node.set_qmi_nas_sys_info_ind_gsm_srv_status_info();
  gsm_srv_status_info->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
  gsm_srv_status_info->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
  gsm_srv_status_info->is_pref_data_path = 0;
  nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_info = QMI_node.set_qmi_nas_sys_info_ind_wcdma_srv_status_info();
  wcdma_srv_status_info->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
  wcdma_srv_status_info->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
  wcdma_srv_status_info->is_pref_data_path = 0;
  nas_3gpp_srv_status_info_type_v01* tdscdma_srv_status_info = QMI_node.set_qmi_nas_sys_info_ind_tdscdma_srv_status_info();
  tdscdma_srv_status_info->srv_status = srv_status;
  tdscdma_srv_status_info->true_srv_status = true_srv_status;
  tdscdma_srv_status_info->is_pref_data_path = 0;
  nas_tdscdma_sys_info_type_v01* tdscdma_sys_info_ind = QMI_node.set_qmi_nas_sys_info_ind_tdscdma_sys_info();
  tdscdma_sys_info_ind->common_sys_info.srv_domain_valid = TRUE;
  tdscdma_sys_info_ind->common_sys_info.srv_domain = domain;
  tdscdma_sys_info_ind->common_sys_info.srv_capability_valid = TRUE;
  tdscdma_sys_info_ind->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
  tdscdma_sys_info_ind->common_sys_info.roam_status_valid = TRUE;
  tdscdma_sys_info_ind->common_sys_info.roam_status = roam_status;
  memset(tags,'\0', MAX_TAG_SIZE);
  snprintf(tags, MAX_TAG_SIZE, "GSM: srv_status = %d, true_srv_status = %d, srv_domain = %d, roam_status = %d",
        srv_status, true_srv_status, domain, roam_status);
  QMI_node.add_callflow_tag(tags);
  t.inject(QMI_node);

  if(is_rat_change)
  {
    util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);

    // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
    nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
    nas_3gpp_srv_status_info_type_v01* gsm_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_gsm_srv_status_info();
    gsm_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_srv_status_info();
    wcdma_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* tdscdma_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_tdscdma_srv_status_info();
    tdscdma_srv_status_resp->srv_status = srv_status;
    tdscdma_srv_status_resp->true_srv_status = true_srv_status;
    tdscdma_srv_status_resp->is_pref_data_path = 0;
    nas_tdscdma_sys_info_type_v01* tdscdma_sys_info_resp= QMI_node.set_qmi_nas_get_sys_info_resp_tdscdma_sys_info();
    tdscdma_sys_info_resp->common_sys_info.srv_domain_valid = TRUE;
    tdscdma_sys_info_resp->common_sys_info.srv_domain = domain;
    tdscdma_sys_info_resp->common_sys_info.srv_capability_valid = TRUE;
    tdscdma_sys_info_resp->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
    tdscdma_sys_info_resp->common_sys_info.roam_status_valid = TRUE;
    tdscdma_sys_info_resp->common_sys_info.roam_status = roam_status;
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "TDSCDMA: srv_status = %d, true_srv_status = %d, srv_domain = %d, roam_status = %d",
            srv_status, true_srv_status, domain, roam_status);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);
  }

  if(is_needed)
  {
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

    // Expect: RIL_UNSOL_SIGNAL_STRENGTH
    RIL_node.update_default_ril_unsol_signal_strength();
    t.expect(RIL_node);
  }

  // Expect: RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_voice_network_state_changed();
  t.expect(RIL_node);

  if(RADIO_TECH_UNKNOWN != rat)
  {
    // Expect: RIL_UNSOL_VOICE_RADIO_TECH_CHANGED
    RIL_node.update_default_ril_unsol_voice_radio_tech_changed();
    RIL_node.set_ril_unsol_voice_radio_tech_changed_radio_tech((RIL_RadioTechnology)rat);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "RAT = %d", rat);
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);

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

    // Expect: RIL_UNSOL_SIGNAL_STRENGTH
    RIL_node.update_default_ril_unsol_signal_strength();
    t.expect(RIL_node);
  }

  t.set_end();
}

void util_voice_radio_tech_cdma_in_service(
     Testcase &t,
     QMIRILTestnode &QMI_node,
     AndroidTelTestnode &RIL_node,
     int is_rat_change,
     int rat,
     nas_service_status_enum_type_v01 srv_status,
     nas_service_domain_enum_type_v01 domain,
     nas_roam_status_enum_type_v01 roam_status,
     uint8_t net_id_valid,
     const char *mcc,
     const char *mnc,
     uint8_t is_needed
)
{
  char tags[MAX_TAG_SIZE];
  t.set_start();

  //Inject: QMI_NAS_SYS_INFO_IND_MSG_V01
  QMI_node.update_default_sys_info_ind_msg();
  nas_3gpp_srv_status_info_type_v01* gsm_srv_status_info = QMI_node.set_qmi_nas_sys_info_ind_gsm_srv_status_info();
  gsm_srv_status_info->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
  gsm_srv_status_info->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
  gsm_srv_status_info->is_pref_data_path = 0;
  nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_info = QMI_node.set_qmi_nas_sys_info_ind_wcdma_srv_status_info();
  wcdma_srv_status_info->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
  wcdma_srv_status_info->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
  wcdma_srv_status_info->is_pref_data_path = 0;
  nas_3gpp2_srv_status_info_type_v01* cdma_srv_status_info_ind = QMI_node.set_qmi_nas_sys_info_ind_cdma_srv_status_info();
  cdma_srv_status_info_ind->srv_status = srv_status;
  cdma_srv_status_info_ind->is_pref_data_path = 0;
  nas_cdma_sys_info_type_v01* cdma_sys_info_ind = QMI_node.set_qmi_nas_sys_info_ind_cdma_sys_info();
  cdma_sys_info_ind->common_sys_info.srv_domain_valid = TRUE;
  cdma_sys_info_ind->common_sys_info.srv_domain = domain;
  cdma_sys_info_ind->common_sys_info.srv_capability_valid = TRUE;
  cdma_sys_info_ind->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
  cdma_sys_info_ind->common_sys_info.roam_status_valid = TRUE;
  cdma_sys_info_ind->common_sys_info.roam_status = roam_status;
  cdma_sys_info_ind->cdma_specific_sys_info.network_id_valid = net_id_valid;
  if(NULL != mcc)
  {
    strcpy(cdma_sys_info_ind->cdma_specific_sys_info.network_id.mcc,mcc);
  }
  if(NULL != mnc)
  {
    strcpy(cdma_sys_info_ind->cdma_specific_sys_info.network_id.mnc,mnc);
  }
  memset(tags,'\0', MAX_TAG_SIZE);
  snprintf(tags, MAX_TAG_SIZE, "CDMA: srv_status = %d, srv_domain = %d, roam_status = %d",
        srv_status, domain, roam_status);
  QMI_node.add_callflow_tag(tags);
  t.inject(QMI_node);

  if(is_rat_change)
  {
    util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);

    // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
    nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
    nas_3gpp_srv_status_info_type_v01* gsm_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_gsm_srv_status_info();
    gsm_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_srv_status_info();
    wcdma_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp2_srv_status_info_type_v01* cdma_srv_status_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_cdma_srv_status_info();
    cdma_srv_status_info_resp->srv_status = srv_status;
    cdma_srv_status_info_resp->is_pref_data_path = 0;
    nas_cdma_sys_info_type_v01* cdma_sys_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_cdma_sys_info();
    cdma_sys_info_resp->common_sys_info.srv_domain_valid = TRUE;
    cdma_sys_info_resp->common_sys_info.srv_domain = domain;
    cdma_sys_info_resp->common_sys_info.srv_capability_valid = TRUE;
    cdma_sys_info_resp->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
    cdma_sys_info_resp->common_sys_info.roam_status_valid = TRUE;
    cdma_sys_info_resp->common_sys_info.roam_status = roam_status;
    cdma_sys_info_resp->cdma_specific_sys_info.network_id_valid = net_id_valid;
    if(NULL != mcc)
    {
      strcpy(cdma_sys_info_resp->cdma_specific_sys_info.network_id.mcc,mcc);
    }
    if(NULL != mnc)
    {
      strcpy(cdma_sys_info_resp->cdma_specific_sys_info.network_id.mnc,mnc);
    }
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "CDMA: srv_status = %d, srv_domain = %d, roam_status = %d",
            srv_status, domain, roam_status);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);
  }

  if(is_needed)
  {
    // Expect: QMI_NAS_GET_SIG_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sig_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SIG_INFO_RESP_MSG_V01
    QMI_node.update_default_qmi_nas_get_sig_info_resp_msg_v01();
    QMI_node.set_qmi_nas_get_sig_info_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    // Expect: RIL_UNSOL_SIGNAL_STRENGTH
    RIL_node.update_default_ril_unsol_signal_strength();
    t.expect(RIL_node);
  }

  // Expect: RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_voice_network_state_changed();
  t.expect(RIL_node);

  if(RADIO_TECH_UNKNOWN != rat)
  {
    // Expect: RIL_UNSOL_VOICE_RADIO_TECH_CHANGED
    RIL_node.update_default_ril_unsol_voice_radio_tech_changed();
    RIL_node.set_ril_unsol_voice_radio_tech_changed_radio_tech((RIL_RadioTechnology)rat);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "RAT = %d", rat);
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);

    // Expect: QMI_NAS_GET_SIG_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sig_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SIG_INFO_RESP_MSG_V01
    QMI_node.update_default_qmi_nas_get_sig_info_resp_msg_v01();
    QMI_node.set_qmi_nas_get_sig_info_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    // Expect: RIL_UNSOL_SIGNAL_STRENGTH
    RIL_node.update_default_ril_unsol_signal_strength();
    t.expect(RIL_node);
  }

  t.set_end();
}

void util_voice_radio_tech_cdma_in_service(
     Testcase &t,
     QMIRILTestnode &QMI_node,
     AndroidTelTestnode &RIL_node,
     int is_rat_change,
     int rat,
     nas_service_status_enum_type_v01 srv_status,
     nas_service_domain_enum_type_v01 domain,
     nas_roam_status_enum_type_v01 roam_status,
     uint8_t is_needed
)
{
  util_voice_radio_tech_cdma_in_service(t, QMI_node, RIL_node, is_rat_change, rat, srv_status, domain, roam_status, FALSE, NULL, NULL, is_needed);
}

void util_voice_radio_tech_lte_in_service(
     Testcase &t,
     QMIRILTestnode &QMI_node,
     AndroidTelTestnode &RIL_node,
     IMSTestnode &IMS_node,
     uint8_t is_vops_ind_expected,
     int rat,
     nas_service_status_enum_type_v01 srv_status,
     nas_true_service_status_enum_type_v01 true_srv_status,
     nas_service_domain_enum_type_v01 domain,
     nas_roam_status_enum_type_v01 roam_status,
     nas_lte_voice_status_enum_type_v01 lte_voice_status,
     nas_sms_status_enum_type_v01 lte_sms_status,
     uint8_t voice_support_on_lte,
     uint8_t net_id_valid,
     const char *mcc,
     const char *mnc,
     uint8_t is_rat_change,
     uint8_t is_needed,
     uint8_t is_data_reg_needed,
     bool is_data_rat_change
)
{
  char tags[MAX_TAG_SIZE];
  t.set_start();

  //Inject: QMI_NAS_SYS_INFO_IND_MSG_V01
  QMI_node.update_default_sys_info_ind_msg();
  nas_3gpp2_srv_status_info_type_v01* cdma_srv_status_info_ind = QMI_node.set_qmi_nas_sys_info_ind_cdma_srv_status_info();
  cdma_srv_status_info_ind->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
  cdma_srv_status_info_ind->is_pref_data_path = 0;
  nas_3gpp_srv_status_info_type_v01* gsm_srv_status_info = QMI_node.set_qmi_nas_sys_info_ind_gsm_srv_status_info();
  gsm_srv_status_info->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
  gsm_srv_status_info->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
  gsm_srv_status_info->is_pref_data_path = 0;
  nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_info = QMI_node.set_qmi_nas_sys_info_ind_wcdma_srv_status_info();
  wcdma_srv_status_info->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
  wcdma_srv_status_info->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
  wcdma_srv_status_info->is_pref_data_path = 0;
  nas_3gpp_srv_status_info_type_v01 *lte_srv_status_info = QMI_node.set_qmi_nas_sys_info_ind_lte_srv_status_info();
  lte_srv_status_info->srv_status = srv_status;
  lte_srv_status_info->true_srv_status = true_srv_status;
  lte_srv_status_info->is_pref_data_path = 0;
  nas_lte_sys_info_type_v01 *lte_sys_info = QMI_node.set_qmi_nas_sys_info_ind_lte_sys_info();
  lte_sys_info->common_sys_info.srv_domain_valid = TRUE;
  lte_sys_info->common_sys_info.srv_domain = domain;
  lte_sys_info->common_sys_info.srv_capability_valid = TRUE;
  lte_sys_info->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
  lte_sys_info->common_sys_info.roam_status_valid = TRUE;
  lte_sys_info->common_sys_info.roam_status = roam_status;
  lte_sys_info->threegpp_specific_sys_info.network_id_valid = net_id_valid;
  if(NULL != mcc)
  {
    strcpy(lte_sys_info->threegpp_specific_sys_info.network_id.mcc,mcc);
  }

  if(NULL != mnc)
  {
    strcpy(lte_sys_info->threegpp_specific_sys_info.network_id.mnc,mnc);
  }
  QMI_node.set_qmi_nas_sys_info_ind_lte_voice_status(lte_voice_status);
  QMI_node.set_qmi_nas_sys_info_ind_lte_sms_status(lte_sms_status);
  QMI_node.set_qmi_nas_sys_info_ind_voice_support_on_lte(voice_support_on_lte);
  memset(tags,'\0', MAX_TAG_SIZE);
  snprintf(tags, MAX_TAG_SIZE, "LTE: srv_status = %d, true_srv_status = %d, srv_domain = %d, "
                              "roam_status = %d, voice_support_on_lte = %d, lte_voice_status = %d, "
                              "lte_sms_status = %d", srv_status, true_srv_status, domain, roam_status,
                              voice_support_on_lte, lte_voice_status, lte_sms_status);
  QMI_node.add_callflow_tag(tags);
  t.inject(QMI_node);

  if(is_vops_ind_expected)
  {
#if 0 // enable this once the indication is added to IMS HIDL
    //Expect: ims_MsgId_UNSOL_VOPS_CHANGED
    IMS_node.update_default_ims_unsol_vops_changed(voice_support_on_lte);
    t.expect(IMS_node);
#endif
  }

  if(is_rat_change)
  {
    if (is_data_rat_change)
    {
      util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);
    }

    // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
    nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
    nas_3gpp2_srv_status_info_type_v01* cdma_srv_status_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_cdma_srv_status_info();
    cdma_srv_status_info_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    cdma_srv_status_info_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* gsm_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_gsm_srv_status_info();
    gsm_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_srv_status_info();
    wcdma_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* lte_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_lte_srv_status_info();
    lte_srv_status_resp->srv_status = srv_status;
    lte_srv_status_resp->true_srv_status = true_srv_status;
    lte_srv_status_resp->is_pref_data_path = 0;
    nas_lte_sys_info_type_v01* lte_sys_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_lte_sys_info();
    lte_sys_info_resp->common_sys_info.srv_domain_valid = TRUE;
    lte_sys_info_resp->common_sys_info.srv_domain = domain;
    lte_sys_info_resp->common_sys_info.srv_capability_valid = TRUE;
    lte_sys_info_resp->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
    lte_sys_info_resp->common_sys_info.roam_status_valid = TRUE;
    lte_sys_info_resp->common_sys_info.roam_status = roam_status;
    lte_sys_info_resp->threegpp_specific_sys_info.network_id_valid = net_id_valid;
    if(NULL != mcc)
    {
      strcpy(lte_sys_info_resp->threegpp_specific_sys_info.network_id.mcc,mcc);
    }

    if(NULL != mnc)
    {
      strcpy(lte_sys_info_resp->threegpp_specific_sys_info.network_id.mnc,mnc);
    }
    QMI_node.set_qmi_nas_get_sys_info_resp_lte_voice_status(lte_voice_status);
    QMI_node.set_qmi_nas_get_sys_info_resp_lte_sms_status(lte_sms_status);
    QMI_node.set_qmi_nas_get_sys_info_resp_voice_support_on_lte(voice_support_on_lte);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "LTE: srv_status = %d, true_srv_status = %d, srv_domain = %d, "
                                "roam_status = %d, voice_support_on_lte = %d, lte_voice_status = %d, "
                                "lte_sms_status = %d", srv_status, true_srv_status, domain, roam_status,
                                voice_support_on_lte, lte_voice_status, lte_sms_status);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);
  }

  if(is_needed)
  {
    // Expect: QMI_NAS_GET_SIG_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sig_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SIG_INFO_RESP_MSG_V01
    QMI_node.update_default_qmi_nas_get_sig_info_resp_msg_v01();
    QMI_node.set_qmi_nas_get_sig_info_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    // Expect: RIL_UNSOL_SIGNAL_STRENGTH
    RIL_node.update_default_ril_unsol_signal_strength();
    t.expect(RIL_node);
  }

  // Expect: RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_voice_network_state_changed();
  t.expect(RIL_node);

  if (is_data_reg_needed)
  {
    // Inject: RIL_REQUEST_DATA_REGISTRATION_STATE
    RIL_node.update_default_ril_request_data_registration_state();
    t.inject(RIL_node);

    // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
    nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
    nas_3gpp2_srv_status_info_type_v01* cdma_srv_status_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_cdma_srv_status_info();
    cdma_srv_status_info_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    cdma_srv_status_info_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* gsm_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_gsm_srv_status_info();
    gsm_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_srv_status_info();
    wcdma_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* lte_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_lte_srv_status_info();
    lte_srv_status_resp->srv_status = srv_status;
    lte_srv_status_resp->true_srv_status = true_srv_status;
    lte_srv_status_resp->is_pref_data_path = 0;
    nas_lte_sys_info_type_v01* lte_sys_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_lte_sys_info();
    lte_sys_info_resp->common_sys_info.srv_domain_valid = TRUE;
    lte_sys_info_resp->common_sys_info.srv_domain = domain;
    lte_sys_info_resp->common_sys_info.srv_capability_valid = TRUE;
    lte_sys_info_resp->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
    lte_sys_info_resp->common_sys_info.roam_status_valid = TRUE;
    lte_sys_info_resp->common_sys_info.roam_status = roam_status;
    lte_sys_info_resp->threegpp_specific_sys_info.network_id_valid = net_id_valid;
    if(NULL != mcc)
    {
      strcpy(lte_sys_info_resp->threegpp_specific_sys_info.network_id.mcc,mcc);
    }

    if(NULL != mnc)
    {
      strcpy(lte_sys_info_resp->threegpp_specific_sys_info.network_id.mnc,mnc);
    }
    QMI_node.set_qmi_nas_get_sys_info_resp_lte_voice_status(lte_voice_status);
    QMI_node.set_qmi_nas_get_sys_info_resp_lte_sms_status(lte_sms_status);
    QMI_node.set_qmi_nas_get_sys_info_resp_voice_support_on_lte(voice_support_on_lte);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "LTE: srv_status = %d, true_srv_status = %d, srv_domain = %d, "
                                "roam_status = %d, voice_support_on_lte = %d, lte_voice_status = %d, "
                                "lte_sms_status = %d", srv_status, true_srv_status, domain, roam_status,
                                voice_support_on_lte, lte_voice_status, lte_sms_status);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    QMI_node.update_default_qmi_nas_get_cell_location_info_req_msg();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_nas_get_cell_location_info_resp_msg((qmi_result_type_v01)0, (qmi_error_type_v01)0);
    t.inject(QMI_node);

    //Expect: RIL_REQUEST_DATA_REGISTRATION_STATE_RESP
    RIL_DataRegistrationStateResponse* reg_state_resp = RIL_node.update_default_ril_request_data_registration_state_resp();
    reg_state_resp->regState = (RIL_RegState)1;
    reg_state_resp->rat = (RIL_RadioTechnology)RADIO_TECH_LTE;
    t.expect(RIL_node);

    // Expect: RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED
    RIL_node.update_default_ril_unsol_response_voice_network_state_changed();
    t.expect(RIL_node);
  }

  if( RADIO_TECH_UNKNOWN != rat )
  {
    // Expect: RIL_UNSOL_VOICE_RADIO_TECH_CHANGED
    RIL_node.update_default_ril_unsol_voice_radio_tech_changed();
    RIL_node.set_ril_unsol_voice_radio_tech_changed_radio_tech((RIL_RadioTechnology)rat);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "RAT = %d", rat);
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);

    // Expect: QMI_NAS_GET_SIG_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sig_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SIG_INFO_RESP_MSG_V01
    QMI_node.update_default_qmi_nas_get_sig_info_resp_msg_v01();
    QMI_node.set_qmi_nas_get_sig_info_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    // Expect: RIL_UNSOL_SIGNAL_STRENGTH
    RIL_node.update_default_ril_unsol_signal_strength();
    t.expect(RIL_node);
  }

  t.set_end();
}

void util_voice_radio_tech_lte_in_service(
     Testcase &t,
     QMIRILTestnode &QMI_node,
     AndroidTelTestnode &RIL_node,
     IMSTestnode &IMS_node,
     uint8_t is_vops_ind_expected,
     int rat,
     nas_service_status_enum_type_v01 srv_status,
     nas_true_service_status_enum_type_v01 true_srv_status,
     nas_service_domain_enum_type_v01 domain,
     nas_roam_status_enum_type_v01 roam_status,
     nas_lte_voice_status_enum_type_v01 lte_voice_status,
     nas_sms_status_enum_type_v01 lte_sms_status,
     uint8_t voice_support_on_lte,
     uint8_t net_id_valid,
     const char *mcc,
     const char *mnc,
     uint8_t is_rat_change,
     uint8_t is_needed,
     uint8_t is_data_rat_change
)
{
  util_voice_radio_tech_lte_in_service(t, QMI_node, RIL_node, IMS_node, is_vops_ind_expected, rat, srv_status, true_srv_status, domain, roam_status, lte_voice_status, lte_sms_status, voice_support_on_lte, net_id_valid, mcc, mnc, is_rat_change, is_needed, FALSE, static_cast<bool>(is_data_rat_change));
}

void util_voice_radio_tech_in_sglte_case(
     Testcase &t,
     QMIRILTestnode &QMI_node,
     AndroidTelTestnode &RIL_node,
     IMSTestnode &IMS_node,
     uint8_t is_vops_ind_expected,
     int rat,
     nas_service_status_enum_type_v01 gsm_srv_status,
     nas_true_service_status_enum_type_v01 gsm_true_srv_status,
     nas_service_domain_enum_type_v01 gsm_domain,
     nas_roam_status_enum_type_v01 gsm_roam_status,
     nas_service_status_enum_type_v01 lte_srv_status,
     nas_true_service_status_enum_type_v01 lte_true_srv_status,
     nas_service_domain_enum_type_v01 lte_domain,
     nas_roam_status_enum_type_v01 lte_roam_status,
     nas_lte_voice_status_enum_type_v01 lte_voice_status,
     nas_sms_status_enum_type_v01 lte_sms_status,
     int is_rat_change,
     int is_data_rat_change,
     int is_sig_info_needed,
     int is_err_rate_needed,
     uint8_t voice_support_on_lte
)
{
  char tags[MAX_TAG_SIZE];
  t.set_start();

  //Inject: QMI_NAS_SYS_INFO_IND_MSG_V01
  QMI_node.update_default_sys_info_ind_msg();
  nas_3gpp_srv_status_info_type_v01* gsm_srv_status_info = QMI_node.set_qmi_nas_sys_info_ind_gsm_srv_status_info();
  gsm_srv_status_info->srv_status = gsm_srv_status;
  gsm_srv_status_info->true_srv_status = gsm_true_srv_status;
  gsm_srv_status_info->is_pref_data_path = 0;
  nas_gsm_sys_info_type_v01 *gsm_sys_info = QMI_node.set_qmi_nas_sys_info_ind_gsm_sys_info();
  gsm_sys_info->common_sys_info.srv_domain_valid = TRUE;
  gsm_sys_info->common_sys_info.srv_domain = gsm_domain;
  gsm_sys_info->common_sys_info.srv_capability_valid = TRUE;
  gsm_sys_info->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
  gsm_sys_info->common_sys_info.roam_status_valid = TRUE;
  gsm_sys_info->common_sys_info.roam_status = gsm_roam_status;
  nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_info = QMI_node.set_qmi_nas_sys_info_ind_wcdma_srv_status_info();
  wcdma_srv_status_info->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
  wcdma_srv_status_info->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
  wcdma_srv_status_info->is_pref_data_path = 0;
  nas_3gpp_srv_status_info_type_v01 *lte_srv_status_info = QMI_node.set_qmi_nas_sys_info_ind_lte_srv_status_info();
  lte_srv_status_info->srv_status = lte_srv_status;
  lte_srv_status_info->true_srv_status = lte_true_srv_status;
  lte_srv_status_info->is_pref_data_path = 0;
  nas_lte_sys_info_type_v01 *lte_sys_info = QMI_node.set_qmi_nas_sys_info_ind_lte_sys_info();
  lte_sys_info->common_sys_info.srv_domain_valid = TRUE;
  lte_sys_info->common_sys_info.srv_domain = lte_domain;
  lte_sys_info->common_sys_info.srv_capability_valid = TRUE;
  lte_sys_info->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
  lte_sys_info->common_sys_info.roam_status_valid = TRUE;
  lte_sys_info->common_sys_info.roam_status = lte_roam_status;
  QMI_node.set_qmi_nas_sys_info_ind_lte_voice_status(lte_voice_status);
  QMI_node.set_qmi_nas_sys_info_ind_lte_sms_status(lte_sms_status);
  QMI_node.set_qmi_nas_sys_info_ind_voice_support_on_lte(voice_support_on_lte);
  memset(tags,'\0', MAX_TAG_SIZE);
  snprintf(tags, MAX_TAG_SIZE, "GSM: gsm_srv_status = %d, gsm_srv_domain = %d, LTE: lte_srv_status = %d, "
                              "lte_srv_domain = %d, voice_support_on_lte = %d", gsm_srv_status, gsm_domain,
                              lte_srv_status, lte_domain, voice_support_on_lte);
  QMI_node.add_callflow_tag(tags);
  t.inject(QMI_node);

  if(is_vops_ind_expected)
  {
#if 0 // enable this once the indication is added to IMS HIDL
    //Expect: ims_MsgId_UNSOL_VOPS_CHANGED
    IMS_node.update_default_ims_unsol_vops_changed(voice_support_on_lte);
    t.expect(IMS_node);
#endif
  }

  if(TRUE == is_rat_change)
  {
    if (is_data_rat_change == TRUE)
    {
      util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);
    }

    // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
    nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
    nas_3gpp_srv_status_info_type_v01* gsm_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_gsm_srv_status_info();
    gsm_srv_status_resp->srv_status = gsm_srv_status;
    gsm_srv_status_resp->true_srv_status = gsm_true_srv_status;
    gsm_srv_status_resp->is_pref_data_path = 0;
    nas_gsm_sys_info_type_v01* gsm_sys_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_gsm_sys_info();
    gsm_sys_info_resp->common_sys_info.srv_domain_valid = TRUE;
    gsm_sys_info_resp->common_sys_info.srv_domain = gsm_domain;
    gsm_sys_info_resp->common_sys_info.srv_capability_valid = TRUE;
    gsm_sys_info_resp->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
    gsm_sys_info_resp->common_sys_info.roam_status_valid = TRUE;
    gsm_sys_info_resp->common_sys_info.roam_status = gsm_roam_status;
    nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_srv_status_info();
    wcdma_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* lte_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_lte_srv_status_info();
    lte_srv_status_resp->srv_status = lte_srv_status;
    lte_srv_status_resp->true_srv_status = lte_true_srv_status;
    lte_srv_status_resp->is_pref_data_path = 0;
    nas_lte_sys_info_type_v01* lte_sys_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_lte_sys_info();
    lte_sys_info_resp->common_sys_info.srv_domain_valid = TRUE;
    lte_sys_info_resp->common_sys_info.srv_domain = lte_domain;
    lte_sys_info_resp->common_sys_info.srv_capability_valid = TRUE;
    lte_sys_info_resp->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
    lte_sys_info_resp->common_sys_info.roam_status_valid = TRUE;
    lte_sys_info_resp->common_sys_info.roam_status = lte_roam_status;
    QMI_node.set_qmi_nas_get_sys_info_resp_lte_voice_status(lte_voice_status);
    QMI_node.set_qmi_nas_get_sys_info_resp_lte_sms_status(lte_sms_status);
    QMI_node.set_qmi_nas_get_sys_info_resp_voice_support_on_lte(voice_support_on_lte);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "GSM: gsm_srv_status = %d, gsm_srv_domain = %d, LTE: lte_srv_status = %d, "
                                "lte_srv_domain = %d, voice_support_on_lte = %d", gsm_srv_status, gsm_domain,
                                lte_srv_status, lte_domain, voice_support_on_lte);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);
  }

  // Expect: RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_voice_network_state_changed();
  t.expect(RIL_node);

  if( RADIO_TECH_UNKNOWN != rat )
  {
    // Expect: RIL_UNSOL_VOICE_RADIO_TECH_CHANGED
    RIL_node.update_default_ril_unsol_voice_radio_tech_changed();
    RIL_node.set_ril_unsol_voice_radio_tech_changed_radio_tech((RIL_RadioTechnology)rat);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "RAT = %d", rat);
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);

    if(is_sig_info_needed)
    {
       // Expect: QMI_NAS_GET_SIG_INFO_REQ_MSG_V01
       QMI_node.update_default_qmi_nas_get_sig_info_req_msg_v01();
       t.expect(QMI_node);

       // Inject: QMI_NAS_GET_SIG_INFO_RESP_MSG_V01
       QMI_node.update_default_qmi_nas_get_sig_info_resp_msg_v01();
       QMI_node.set_qmi_nas_get_sig_info_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
       t.inject(QMI_node);
    }

    if(is_err_rate_needed)
    {
       // Expect: QMI_NAS_GET_ERR_RATE_REQ_MSG_V01
       QMI_node.update_default_qmi_nas_get_err_rate_req_msg_v01();
       t.expect(QMI_node);

       // Inject: QMI_NAS_GET_ERR_RATE_RESP_MSG_V01
       QMI_node.update_default_qmi_nas_get_err_rate_resp_msg_v01();
       QMI_node.set_qmi_nas_get_err_rate_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
       t.inject(QMI_node);
    }

    // Expect: RIL_UNSOL_SIGNAL_STRENGTH
    RIL_node.update_default_ril_unsol_signal_strength();
    t.expect(RIL_node);
  }

  t.set_end();
}
void util_voice_radio_tech_in_svlte_case(
     Testcase &t,
     QMIRILTestnode &QMI_node,
     AndroidTelTestnode &RIL_node,
     IMSTestnode &IMS_node,
     uint8_t is_vops_ind_expected,
     int rat,
     nas_service_status_enum_type_v01 cdma_srv_status,
     nas_service_domain_enum_type_v01 cdma_domain,
     nas_roam_status_enum_type_v01 cdma_roam_status,
     nas_service_status_enum_type_v01 lte_srv_status,
     nas_true_service_status_enum_type_v01 lte_true_srv_status,
     nas_service_domain_enum_type_v01 lte_domain,
     nas_roam_status_enum_type_v01 lte_roam_status,
     nas_lte_voice_status_enum_type_v01 lte_voice_status,
     nas_sms_status_enum_type_v01 lte_sms_status,
     int is_rat_change,
     int is_data_rat_change,
     int is_sig_info_needed,
     int is_err_rate_needed,
     uint8_t voice_support_on_lte
)
{
  char tags[MAX_TAG_SIZE];
  t.set_start();

  //Inject: QMI_NAS_SYS_INFO_IND_MSG_V01
  QMI_node.update_default_sys_info_ind_msg();
  nas_3gpp_srv_status_info_type_v01* gsm_srv_status_info = QMI_node.set_qmi_nas_sys_info_ind_gsm_srv_status_info();
  gsm_srv_status_info->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
  gsm_srv_status_info->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
  gsm_srv_status_info->is_pref_data_path = 0;
  nas_3gpp2_srv_status_info_type_v01* cdma_srv_status_info_ind = QMI_node.set_qmi_nas_sys_info_ind_cdma_srv_status_info();
  cdma_srv_status_info_ind->srv_status = cdma_srv_status;
  cdma_srv_status_info_ind->is_pref_data_path = 0;
  nas_cdma_sys_info_type_v01* cdma_sys_info_ind = QMI_node.set_qmi_nas_sys_info_ind_cdma_sys_info();
  cdma_sys_info_ind->common_sys_info.srv_domain_valid = TRUE;
  cdma_sys_info_ind->common_sys_info.srv_domain = cdma_domain;
  cdma_sys_info_ind->common_sys_info.srv_capability_valid = TRUE;
  cdma_sys_info_ind->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
  cdma_sys_info_ind->common_sys_info.roam_status_valid = TRUE;
  cdma_sys_info_ind->common_sys_info.roam_status = cdma_roam_status;
  nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_info = QMI_node.set_qmi_nas_sys_info_ind_wcdma_srv_status_info();
  wcdma_srv_status_info->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
  wcdma_srv_status_info->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
  wcdma_srv_status_info->is_pref_data_path = 0;
  nas_3gpp_srv_status_info_type_v01 *lte_srv_status_info = QMI_node.set_qmi_nas_sys_info_ind_lte_srv_status_info();
  lte_srv_status_info->srv_status = lte_srv_status;
  lte_srv_status_info->true_srv_status = lte_true_srv_status;
  lte_srv_status_info->is_pref_data_path = 0;
  nas_lte_sys_info_type_v01 *lte_sys_info = QMI_node.set_qmi_nas_sys_info_ind_lte_sys_info();
  lte_sys_info->common_sys_info.srv_domain_valid = TRUE;
  lte_sys_info->common_sys_info.srv_domain = lte_domain;
  lte_sys_info->common_sys_info.srv_capability_valid = TRUE;
  lte_sys_info->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
  lte_sys_info->common_sys_info.roam_status_valid = TRUE;
  lte_sys_info->common_sys_info.roam_status = lte_roam_status;
  QMI_node.set_qmi_nas_sys_info_ind_lte_voice_status(lte_voice_status);
  QMI_node.set_qmi_nas_sys_info_ind_lte_sms_status(lte_sms_status);
  QMI_node.set_qmi_nas_sys_info_ind_voice_support_on_lte(voice_support_on_lte);
  memset(tags,'\0', MAX_TAG_SIZE);
  snprintf(tags, MAX_TAG_SIZE, "CDMA: cdma_srv_status = %d, cdma_srv_domain = %d, LTE: lte_srv_status = %d, "
                              "lte_srv_domain = %d, voice_support_on_lte = %d", cdma_srv_status, cdma_domain,
                              lte_srv_status, lte_domain, voice_support_on_lte);
  QMI_node.add_callflow_tag(tags);
  t.inject(QMI_node);

  if(is_vops_ind_expected)
  {
#if 0 // enable this once the indication is added to IMS HIDL
    //Expect: ims_MsgId_UNSOL_VOPS_CHANGED
    IMS_node.update_default_ims_unsol_vops_changed(voice_support_on_lte);
    t.expect(IMS_node);
#endif
  }

  if(TRUE == is_rat_change)
  {
    if (is_data_rat_change == TRUE)
    {
      util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);
    }

    // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
    nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
    nas_3gpp_srv_status_info_type_v01* gsm_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_gsm_srv_status_info();
    gsm_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp2_srv_status_info_type_v01* cdma_srv_status_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_cdma_srv_status_info();
    cdma_srv_status_info_resp->srv_status = cdma_srv_status;
    cdma_srv_status_info_resp->is_pref_data_path = 0;
    nas_cdma_sys_info_type_v01* cdma_sys_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_cdma_sys_info();
    cdma_sys_info_resp->common_sys_info.srv_domain_valid = TRUE;
    cdma_sys_info_resp->common_sys_info.srv_domain = cdma_domain;
    cdma_sys_info_resp->common_sys_info.srv_capability_valid = TRUE;
    cdma_sys_info_resp->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
    cdma_sys_info_resp->common_sys_info.roam_status_valid = TRUE;
    cdma_sys_info_resp->common_sys_info.roam_status = cdma_roam_status;
    nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_srv_status_info();
    wcdma_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* lte_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_lte_srv_status_info();
    lte_srv_status_resp->srv_status = lte_srv_status;
    lte_srv_status_resp->true_srv_status = lte_true_srv_status;
    lte_srv_status_resp->is_pref_data_path = 0;
    nas_lte_sys_info_type_v01* lte_sys_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_lte_sys_info();
    lte_sys_info_resp->common_sys_info.srv_domain_valid = TRUE;
    lte_sys_info_resp->common_sys_info.srv_domain = lte_domain;
    lte_sys_info_resp->common_sys_info.srv_capability_valid = TRUE;
    lte_sys_info_resp->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
    lte_sys_info_resp->common_sys_info.roam_status_valid = TRUE;
    lte_sys_info_resp->common_sys_info.roam_status = lte_roam_status;
    QMI_node.set_qmi_nas_get_sys_info_resp_lte_voice_status(lte_voice_status);
    QMI_node.set_qmi_nas_get_sys_info_resp_lte_sms_status(lte_sms_status);
    QMI_node.set_qmi_nas_get_sys_info_resp_voice_support_on_lte(voice_support_on_lte);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "CDMA: cdma_srv_status = %d, cdma_srv_domain = %d, LTE: lte_srv_status = %d, "
                                "lte_srv_domain = %d, voice_support_on_lte = %d", cdma_srv_status, cdma_domain,
                                lte_srv_status, lte_domain, voice_support_on_lte);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);
  }

  // Expect: RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_voice_network_state_changed();
  t.expect(RIL_node);

  if( RADIO_TECH_UNKNOWN != rat )
  {
    // Expect: RIL_UNSOL_VOICE_RADIO_TECH_CHANGED
    RIL_node.update_default_ril_unsol_voice_radio_tech_changed();
    RIL_node.set_ril_unsol_voice_radio_tech_changed_radio_tech((RIL_RadioTechnology)rat);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "RAT = %d", rat);
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);

    if(is_sig_info_needed)
    {
       // Expect: QMI_NAS_GET_SIG_INFO_REQ_MSG_V01
       QMI_node.update_default_qmi_nas_get_sig_info_req_msg_v01();
       t.expect(QMI_node);

       // Inject: QMI_NAS_GET_SIG_INFO_RESP_MSG_V01
       QMI_node.update_default_qmi_nas_get_sig_info_resp_msg_v01();
       QMI_node.set_qmi_nas_get_sig_info_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
       t.inject(QMI_node);
    }

    if(is_err_rate_needed)
    {
       // Expect: QMI_NAS_GET_ERR_RATE_REQ_MSG_V01
       QMI_node.update_default_qmi_nas_get_err_rate_req_msg_v01();
       t.expect(QMI_node);

       // Inject: QMI_NAS_GET_ERR_RATE_RESP_MSG_V01
       QMI_node.update_default_qmi_nas_get_err_rate_resp_msg_v01();
       QMI_node.set_qmi_nas_get_err_rate_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
       t.inject(QMI_node);
    }

    // Expect: RIL_UNSOL_SIGNAL_STRENGTH
    RIL_node.update_default_ril_unsol_signal_strength();
    t.expect(RIL_node);
  }

  t.set_end();
}
void util_voice_radio_tech_in_svdo_case(
     Testcase &t,
     QMIRILTestnode &QMI_node,
     AndroidTelTestnode &RIL_node,
     int rat,
     nas_service_status_enum_type_v01 cdma_srv_status,
     nas_service_domain_enum_type_v01 cdma_domain,
     nas_roam_status_enum_type_v01 cdma_roam_status,
     nas_service_status_enum_type_v01 hdr_srv_status,
     nas_service_domain_enum_type_v01 hdr_domain,
     nas_roam_status_enum_type_v01 hdr_roam_status,
     uint8_t hdr_personality_valid,
     nas_hdr_personality_enum_type_v01 hdr_personality,
     uint8_t hdr_active_prot_valid,
     nas_hdr_active_prot_enum_type_v01 hdr_active_prot,
     int is_rat_change,
     int is_sig_info_needed,
     int is_err_rate_needed
)
{
  char tags[MAX_TAG_SIZE];
  t.set_start();

  //Inject: QMI_NAS_SYS_INFO_IND_MSG_V01
  QMI_node.update_default_sys_info_ind_msg();
  nas_3gpp_srv_status_info_type_v01* gsm_srv_status_info = QMI_node.set_qmi_nas_sys_info_ind_gsm_srv_status_info();
  gsm_srv_status_info->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
  gsm_srv_status_info->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
  gsm_srv_status_info->is_pref_data_path = 0;
  nas_3gpp2_srv_status_info_type_v01* cdma_srv_status_info_ind = QMI_node.set_qmi_nas_sys_info_ind_cdma_srv_status_info();
  cdma_srv_status_info_ind->srv_status = cdma_srv_status;
  cdma_srv_status_info_ind->is_pref_data_path = 0;
  nas_cdma_sys_info_type_v01* cdma_sys_info_ind = QMI_node.set_qmi_nas_sys_info_ind_cdma_sys_info();
  cdma_sys_info_ind->common_sys_info.srv_domain_valid = TRUE;
  cdma_sys_info_ind->common_sys_info.srv_domain = cdma_domain;
  cdma_sys_info_ind->common_sys_info.srv_capability_valid = TRUE;
  cdma_sys_info_ind->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
  cdma_sys_info_ind->common_sys_info.roam_status_valid = TRUE;
  cdma_sys_info_ind->common_sys_info.roam_status = cdma_roam_status;
  nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_info = QMI_node.set_qmi_nas_sys_info_ind_wcdma_srv_status_info();
  wcdma_srv_status_info->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
  wcdma_srv_status_info->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
  wcdma_srv_status_info->is_pref_data_path = 0;
  nas_3gpp_srv_status_info_type_v01 *lte_srv_status_info = QMI_node.set_qmi_nas_sys_info_ind_lte_srv_status_info();
  lte_srv_status_info->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
  lte_srv_status_info->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
  lte_srv_status_info->is_pref_data_path = 0;
  nas_3gpp2_srv_status_info_type_v01* hdr_srv_status_info_ind = QMI_node.set_qmi_nas_sys_info_ind_hdr_srv_status_info();
  hdr_srv_status_info_ind->srv_status = hdr_srv_status;
  hdr_srv_status_info_ind->is_pref_data_path = 0;
  nas_hdr_sys_info_type_v01* hdr_sys_info_ind = QMI_node.set_qmi_nas_sys_info_ind_hdr_sys_info();
  hdr_sys_info_ind->common_sys_info.srv_domain_valid = TRUE;
  hdr_sys_info_ind->common_sys_info.srv_domain = hdr_domain;
  hdr_sys_info_ind->common_sys_info.srv_capability_valid = TRUE;
  hdr_sys_info_ind->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
  hdr_sys_info_ind->common_sys_info.roam_status_valid = TRUE;
  hdr_sys_info_ind->common_sys_info.roam_status = hdr_roam_status;
  hdr_sys_info_ind->hdr_specific_sys_info.hdr_personality_valid = hdr_personality_valid;
  hdr_sys_info_ind->hdr_specific_sys_info.hdr_personality = hdr_personality;
  hdr_sys_info_ind->hdr_specific_sys_info.hdr_active_prot_valid = hdr_active_prot_valid;
  hdr_sys_info_ind->hdr_specific_sys_info.hdr_active_prot = hdr_active_prot;
  memset(tags,'\0', MAX_TAG_SIZE);
  snprintf(tags, MAX_TAG_SIZE, "CDMA: cdma_srv_status = %d, cdma_srv_domain = %d, HDR: hdr_srv_status = %d, "
                              "hdr_srv_domain = %d", cdma_srv_status, cdma_domain, hdr_srv_status, hdr_domain);
  QMI_node.add_callflow_tag(tags);
  t.inject(QMI_node);

  if(TRUE == is_rat_change)
  {
    util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);

    // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
    nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
    nas_3gpp_srv_status_info_type_v01* gsm_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_gsm_srv_status_info();
    gsm_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp2_srv_status_info_type_v01* cdma_srv_status_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_cdma_srv_status_info();
    cdma_srv_status_info_resp->srv_status = cdma_srv_status;
    cdma_srv_status_info_resp->is_pref_data_path = 0;
    nas_cdma_sys_info_type_v01* cdma_sys_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_cdma_sys_info();
    cdma_sys_info_resp->common_sys_info.srv_domain_valid = TRUE;
    cdma_sys_info_resp->common_sys_info.srv_domain = cdma_domain;
    cdma_sys_info_resp->common_sys_info.srv_capability_valid = TRUE;
    cdma_sys_info_resp->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
    cdma_sys_info_resp->common_sys_info.roam_status_valid = TRUE;
    cdma_sys_info_resp->common_sys_info.roam_status = cdma_roam_status;
    nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_srv_status_info();
    wcdma_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* lte_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_lte_srv_status_info();
    lte_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    lte_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    lte_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp2_srv_status_info_type_v01* hdr_srv_status_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_hdr_srv_status_info();
    hdr_srv_status_info_resp->srv_status = hdr_srv_status;
    hdr_srv_status_info_resp->is_pref_data_path = 0;
    nas_hdr_sys_info_type_v01* hdr_sys_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_hdr_sys_info();
    hdr_sys_info_resp->common_sys_info.srv_domain_valid = TRUE;
    hdr_sys_info_resp->common_sys_info.srv_domain = hdr_domain;
    hdr_sys_info_resp->common_sys_info.srv_capability_valid = TRUE;
    hdr_sys_info_resp->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
    hdr_sys_info_resp->common_sys_info.roam_status_valid = TRUE;
    hdr_sys_info_resp->common_sys_info.roam_status = hdr_roam_status;
    hdr_sys_info_resp->hdr_specific_sys_info.hdr_personality_valid = hdr_personality_valid;
    hdr_sys_info_resp->hdr_specific_sys_info.hdr_personality = hdr_personality;
    hdr_sys_info_resp->hdr_specific_sys_info.hdr_active_prot_valid = hdr_active_prot_valid;
    hdr_sys_info_resp->hdr_specific_sys_info.hdr_active_prot = hdr_active_prot;
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "CDMA: cdma_srv_status = %d, cdma_srv_domain = %d, HDR: hdr_srv_status = %d, "
                                "hdr_srv_domain = %d", cdma_srv_status, cdma_domain, hdr_srv_status, hdr_domain);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);
  }

  // Expect: RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_voice_network_state_changed();
  t.expect(RIL_node);

  if( RADIO_TECH_UNKNOWN != rat )
  {
    // Expect: RIL_UNSOL_VOICE_RADIO_TECH_CHANGED
    RIL_node.update_default_ril_unsol_voice_radio_tech_changed();
    RIL_node.set_ril_unsol_voice_radio_tech_changed_radio_tech((RIL_RadioTechnology)rat);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "RAT = %d", rat);
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);

    if(is_sig_info_needed)
    {
       // Expect: QMI_NAS_GET_SIG_INFO_REQ_MSG_V01
       QMI_node.update_default_qmi_nas_get_sig_info_req_msg_v01();
       t.expect(QMI_node);

       // Inject: QMI_NAS_GET_SIG_INFO_RESP_MSG_V01
       QMI_node.update_default_qmi_nas_get_sig_info_resp_msg_v01();
       QMI_node.set_qmi_nas_get_sig_info_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
       t.inject(QMI_node);
    }

    if(is_err_rate_needed)
    {
       // Expect: QMI_NAS_GET_ERR_RATE_REQ_MSG_V01
       QMI_node.update_default_qmi_nas_get_err_rate_req_msg_v01();
       t.expect(QMI_node);

       // Inject: QMI_NAS_GET_ERR_RATE_RESP_MSG_V01
       QMI_node.update_default_qmi_nas_get_err_rate_resp_msg_v01();
       QMI_node.set_qmi_nas_get_err_rate_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
       t.inject(QMI_node);
    }

    // Expect: RIL_UNSOL_SIGNAL_STRENGTH
    RIL_node.update_default_ril_unsol_signal_strength();
    t.expect(RIL_node);
  }

  t.set_end();
}
void util_voice_registration_state_failure(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int qmi_error, int android_resp)
{
    char tags[MAX_TAG_SIZE];
    t.set_start();

    //Inject: RIL_REQUEST_REGISTRATION_STATE
    RIL_node.update_default_ril_request_registration_state();
    t.inject(RIL_node);

    // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
    nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
    QMI_node.set_qmi_nas_get_sys_info_resp_msg_v01_result(QMI_RESULT_FAILURE_V01, (qmi_error_type_v01)qmi_error);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "RESP = %d, ERROR = %d", QMI_RESULT_FAILURE_V01, qmi_error);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    //Expect: RIL_REQUEST_REGISTRATION_STATE_RESP
    RIL_node.update_default_ril_request_registration_state_resp_in_error_case();
    RIL_node.set_ril_error_value((RIL_Errno)android_resp);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "Response Failure");
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);

    t.set_end();
}

void util_voice_registration_state_for_wcdma_success(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, OEMTestnode &OEM_node, int rat, int reg_state, nas_service_status_enum_type_v01 srv_status, nas_true_service_status_enum_type_v01 true_srv_status, nas_service_domain_enum_type_v01 domain, nas_roam_status_enum_type_v01 roam_status, uint8_t reg_reject_info_valid, nas_service_domain_enum_type_v01 reject_srv_domain, uint8_t rej_cause, uint8_t net_id_valid, const char *mcc, const char *mnc, uint8_t csg_info_valid, uint32_t cid, uint32_t cid_name_len, uint16_t *cid_name, bool is_data_rat_change)
{
    char tags[MAX_TAG_SIZE];
    t.set_start();

    //Inject: RIL_REQUEST_REGISTRATION_STATE
    RIL_node.update_default_ril_request_registration_state();
    t.inject(RIL_node);

    // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
    nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
    nas_3gpp_srv_status_info_type_v01* gsm_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_gsm_srv_status_info();
    gsm_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_srv_status_info();
    wcdma_srv_status_resp->srv_status = srv_status;
    wcdma_srv_status_resp->true_srv_status = true_srv_status;
    wcdma_srv_status_resp->is_pref_data_path = 0;
    nas_wcdma_sys_info_type_v01* wcdma_sys_info_resp= QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_sys_info();
    wcdma_sys_info_resp->common_sys_info.srv_domain_valid = TRUE;
    wcdma_sys_info_resp->common_sys_info.srv_domain = domain;
    wcdma_sys_info_resp->common_sys_info.srv_capability_valid = TRUE;
    wcdma_sys_info_resp->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
    wcdma_sys_info_resp->common_sys_info.roam_status_valid = TRUE;
    wcdma_sys_info_resp->common_sys_info.roam_status = roam_status;
    wcdma_sys_info_resp->threegpp_specific_sys_info.reg_reject_info_valid = reg_reject_info_valid;
    wcdma_sys_info_resp->threegpp_specific_sys_info.reg_reject_info.reject_srv_domain = reject_srv_domain;
    wcdma_sys_info_resp->threegpp_specific_sys_info.reg_reject_info.rej_cause = rej_cause;
    wcdma_sys_info_resp->threegpp_specific_sys_info.network_id_valid = net_id_valid;
    if(NULL != mcc)
    {
        strcpy(wcdma_sys_info_resp->threegpp_specific_sys_info.network_id.mcc,mcc);
    }

    if(NULL != mnc)
    {
        strcpy(wcdma_sys_info_resp->threegpp_specific_sys_info.network_id.mnc,mnc);
    }

    QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_csg_info(csg_info_valid, cid, cid_name_len, cid_name);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "WCDMA, srv_status = %d, true_srv_status = %d, srv_domain = %d, "
                                "roam_status = %d, reject_info_valid = %d, reject_srv_domain = %d, "
                                "rej_cause = %d", srv_status, true_srv_status, domain, roam_status,
                                reg_reject_info_valid, reject_srv_domain, rej_cause);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    if (is_data_rat_change)
    {
      util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);
    }

    if ( csg_info_valid )
    {
        // Expect: RIL_UNSOL_OEM_HOOK_RAW
        OEM_node.update_default_qcril_evt_hook_unsol_csg_id();
        OEM_node.update_qcril_evt_hook_unsol_with_csg_id(cid);
        OEM_node.set_expect_skip_verification_status(false);
        t.expect(OEM_node);

        // Expect: RIL_UNSOL_OEM_HOOK_RAW
        RIL_node.update_default_ril_unsol_oem_hook_raw();
        RIL_node.set_expect_skip_verification_status(false);
        t.expect(RIL_node);
    }

    if(RADIO_TECH_UMTS == rat)
    {
        //Expect: QMI_NAS_GET_CELL_LOCATION_INFO_REQ_MSG_V01
        QMI_node.update_default_qmi_nas_get_cell_location_info_req_msg();
        t.expect(QMI_node);

        // Inject: QMI_NAS_GET_CELL_LOCATION_INFO_RESP_MSG_V01
        QMI_node.update_default_qmi_nas_get_cell_location_info_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
        t.inject(QMI_node);
    }

    //Expect: RIL_REQUEST_REGISTRATION_STATE_RESP
    RIL_VoiceRegistrationStateResponse* reg_state_resp = RIL_node.update_default_ril_request_registration_state_resp();
    reg_state_resp->regState = (RIL_RegState)reg_state;
    reg_state_resp->rat = (RIL_RadioTechnology)rat;
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "RAT = %d, REG_STATE= %d", rat, reg_state);
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);

    t.set_end();
}

void util_data_registration_state_failure(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int qmi_error, int android_resp)
{
    t.set_start();

    // Inject: RIL_REQUEST_DATA_REGISTRATION_STATE
    RIL_node.update_default_ril_request_data_registration_state();
    t.inject(RIL_node);

    // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
    nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
    QMI_node.set_qmi_nas_get_sys_info_resp_msg_v01_result(QMI_RESULT_FAILURE_V01, (qmi_error_type_v01)qmi_error);
    t.inject(QMI_node);

    // Expect: RIL_REQUEST_DATA_REGISTRATION_STATE_RESP
    RIL_node.update_default_ril_request_data_registration_state_in_error_case();
    RIL_node.set_ril_error_value((RIL_Errno)android_resp);
    t.expect(RIL_node);

    t.set_end();
}

void util_data_registration_state_for_wcdma_success(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int rat, int reg_state, nas_service_status_enum_type_v01 srv_status, nas_true_service_status_enum_type_v01 true_srv_status, nas_service_domain_enum_type_v01 domain, nas_roam_status_enum_type_v01 roam_status, uint8_t reg_reject_info_valid, nas_service_domain_enum_type_v01 reject_srv_domain, uint8_t rej_cause, uint8_t is_cell_loc_needed)
{
    t.set_start();

    // Inject: RIL_REQUEST_DATA_REGISTRATION_STATE
    RIL_node.update_default_ril_request_data_registration_state();
    t.inject(RIL_node);

    // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
    nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
    nas_3gpp_srv_status_info_type_v01* gsm_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_gsm_srv_status_info();
    gsm_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_srv_status_info();
    wcdma_srv_status_resp->srv_status = srv_status;
    wcdma_srv_status_resp->true_srv_status = true_srv_status;
    wcdma_srv_status_resp->is_pref_data_path = 0;
    nas_wcdma_sys_info_type_v01* wcdma_sys_info_resp= QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_sys_info();
    wcdma_sys_info_resp->common_sys_info.srv_domain_valid = TRUE;
    wcdma_sys_info_resp->common_sys_info.srv_domain = domain;
    wcdma_sys_info_resp->common_sys_info.srv_capability_valid = TRUE;
    wcdma_sys_info_resp->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
    wcdma_sys_info_resp->common_sys_info.roam_status_valid = TRUE;
    wcdma_sys_info_resp->common_sys_info.roam_status = roam_status;
    wcdma_sys_info_resp->threegpp_specific_sys_info.reg_reject_info_valid = reg_reject_info_valid;
    wcdma_sys_info_resp->threegpp_specific_sys_info.reg_reject_info.reject_srv_domain = reject_srv_domain;
    wcdma_sys_info_resp->threegpp_specific_sys_info.reg_reject_info.rej_cause = rej_cause;
    t.inject(QMI_node);

    if(is_cell_loc_needed)
    {
        //Expect: QMI_NAS_GET_CELL_LOCATION_INFO_REQ_MSG_V01
        QMI_node.update_default_qmi_nas_get_cell_location_info_req_msg();
        t.expect(QMI_node);

        // Inject: QMI_NAS_GET_CELL_LOCATION_INFO_RESP_MSG_V01
        QMI_node.update_default_qmi_nas_get_cell_location_info_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
        t.inject(QMI_node);
    }

    //Expect: RIL_REQUEST_DATA_REGISTRATION_STATE_RESP
    RIL_DataRegistrationStateResponse* reg_state_resp = RIL_node.update_default_ril_request_data_registration_state_resp();
    reg_state_resp->regState = (RIL_RegState)reg_state;
    reg_state_resp->rat = (RIL_RadioTechnology)rat;
    t.expect(RIL_node);

    if(RADIO_TECH_UNKNOWN != rat)
    {
        // Expect: RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED
        RIL_node.update_default_ril_unsol_response_voice_network_state_changed();
        t.expect(RIL_node);
    }

    t.set_end();
}

void util_data_registration_state_for_lte_success(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int rat, int reg_state, uint8_t is_cell_loc_needed)
{
    t.set_start();

    // Inject: RIL_REQUEST_DATA_REGISTRATION_STATE
    RIL_node.update_default_ril_request_data_registration_state();
    t.inject(RIL_node);

    // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
    nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
    t.inject(QMI_node);

    if(is_cell_loc_needed)
    {
        //Expect: QMI_NAS_GET_CELL_LOCATION_INFO_REQ_MSG_V01
        QMI_node.update_default_qmi_nas_get_cell_location_info_req_msg();
        t.expect(QMI_node);

        // Inject: QMI_NAS_GET_CELL_LOCATION_INFO_RESP_MSG_V01
        QMI_node.update_default_qmi_nas_get_cell_location_info_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
        t.inject(QMI_node);
    }

    //Expect: RIL_REQUEST_DATA_REGISTRATION_STATE_RESP
    RIL_DataRegistrationStateResponse* reg_state_resp = RIL_node.update_default_ril_request_data_registration_state_resp();
    reg_state_resp->regState = (RIL_RegState)reg_state;
    reg_state_resp->rat = (RIL_RadioTechnology)rat;
    t.expect(RIL_node);

    if(RADIO_TECH_UNKNOWN != rat)
    {
        // Expect: RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED
        RIL_node.update_default_ril_unsol_response_voice_network_state_changed();
        t.expect(RIL_node);
    }

    t.set_end();
}

void util_voice_registration_state_for_gsm_success(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int rat, int reg_state, nas_service_status_enum_type_v01 srv_status, nas_true_service_status_enum_type_v01 true_srv_status, nas_service_domain_enum_type_v01 domain, nas_roam_status_enum_type_v01 roam_status, uint8_t reg_reject_info_valid, nas_service_domain_enum_type_v01 reject_srv_domain, uint8_t rej_cause,
uint8_t net_id_valid, const char *mcc, const char *mnc, bool is_data_rat_change)
{
    char tags[MAX_TAG_SIZE];
    t.set_start();

    //Inject: RIL_REQUEST_REGISTRATION_STATE
    RIL_node.update_default_ril_request_registration_state();
    t.inject(RIL_node);

    // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
    nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
    nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_srv_status_info();
    wcdma_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* gsm_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_gsm_srv_status_info();
    gsm_srv_status_resp->srv_status = srv_status;
    gsm_srv_status_resp->true_srv_status = true_srv_status;
    gsm_srv_status_resp->is_pref_data_path = 0;
    nas_gsm_sys_info_type_v01* gsm_sys_info_resp= QMI_node.set_qmi_nas_get_sys_info_resp_gsm_sys_info();
    gsm_sys_info_resp->common_sys_info.srv_domain_valid = TRUE;
    gsm_sys_info_resp->common_sys_info.srv_domain = domain;
    gsm_sys_info_resp->common_sys_info.srv_capability_valid = TRUE;
    gsm_sys_info_resp->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
    gsm_sys_info_resp->common_sys_info.roam_status_valid = TRUE;
    gsm_sys_info_resp->common_sys_info.roam_status = roam_status;
    gsm_sys_info_resp->threegpp_specific_sys_info.reg_reject_info_valid = reg_reject_info_valid;
    gsm_sys_info_resp->threegpp_specific_sys_info.reg_reject_info.reject_srv_domain = reject_srv_domain;
    gsm_sys_info_resp->threegpp_specific_sys_info.reg_reject_info.rej_cause = rej_cause;
    gsm_sys_info_resp->threegpp_specific_sys_info.network_id_valid = net_id_valid;
    if(NULL != mcc)
    {
        strcpy(gsm_sys_info_resp->threegpp_specific_sys_info.network_id.mcc,mcc);
    }

    if(NULL != mnc)
    {
        strcpy(gsm_sys_info_resp->threegpp_specific_sys_info.network_id.mnc,mnc);
    }
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "GSM, srv_status = %d, true_srv_status = %d, srv_domain = %d, "
                                "roam_status = %d, reg_reject_info_valid = %d, "
                                "reject_srv_domain = %d, rej_cause = %d", srv_status, true_srv_status,
                                domain, roam_status, reg_reject_info_valid, reject_srv_domain, rej_cause);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    if (is_data_rat_change) {
      util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);
    }

    if(NAS_SYS_SRV_STATUS_SRV_V01 == srv_status)
    {
        //Expect: QMI_NAS_GET_CELL_LOCATION_INFO_REQ_MSG_V01
        QMI_node.update_default_qmi_nas_get_cell_location_info_req_msg();
        t.expect(QMI_node);

        // Inject: QMI_NAS_GET_CELL_LOCATION_INFO_RESP_MSG_V01
        QMI_node.update_default_qmi_nas_get_cell_location_info_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
        t.inject(QMI_node);
    }

    //Expect: RIL_REQUEST_REGISTRATION_STATE_RESP
    RIL_VoiceRegistrationStateResponse* reg_state_resp = RIL_node.update_default_ril_request_registration_state_resp();
    reg_state_resp->regState = (RIL_RegState)reg_state;
    reg_state_resp->rat = (RIL_RadioTechnology)rat;
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "RAT = %d, REG_STATE= %d", rat, reg_state);
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);

    t.set_end();
}

void util_voice_registration_state_for_tdscdma_success(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int rat, int reg_state, nas_service_status_enum_type_v01 srv_status, nas_true_service_status_enum_type_v01 true_srv_status, nas_service_domain_enum_type_v01 domain, nas_roam_status_enum_type_v01 roam_status, uint8_t reg_reject_info_valid, nas_service_domain_enum_type_v01 reject_srv_domain, uint8_t rej_cause, bool is_data_rat_change)
{
    char tags[MAX_TAG_SIZE];
    t.set_start();

    //Inject: RIL_REQUEST_REGISTRATION_STATE
    RIL_node.update_default_ril_request_registration_state();
    t.inject(RIL_node);

    // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
    nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
    nas_3gpp_srv_status_info_type_v01* gsm_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_gsm_srv_status_info();
    gsm_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_srv_status_info();
    wcdma_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* tdscdma_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_tdscdma_srv_status_info();
    tdscdma_srv_status_resp->srv_status = srv_status;
    tdscdma_srv_status_resp->true_srv_status = true_srv_status;
    tdscdma_srv_status_resp->is_pref_data_path = 0;
    nas_tdscdma_sys_info_type_v01* tdscdma_sys_info_resp= QMI_node.set_qmi_nas_get_sys_info_resp_tdscdma_sys_info();
    tdscdma_sys_info_resp->common_sys_info.srv_domain_valid = TRUE;
    tdscdma_sys_info_resp->common_sys_info.srv_domain = domain;
    tdscdma_sys_info_resp->common_sys_info.srv_capability_valid = TRUE;
    tdscdma_sys_info_resp->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
    tdscdma_sys_info_resp->common_sys_info.roam_status_valid = TRUE;
    tdscdma_sys_info_resp->common_sys_info.roam_status = roam_status;
    tdscdma_sys_info_resp->threegpp_specific_sys_info.reg_reject_info_valid = reg_reject_info_valid;
    tdscdma_sys_info_resp->threegpp_specific_sys_info.reg_reject_info.reject_srv_domain = reject_srv_domain;
    tdscdma_sys_info_resp->threegpp_specific_sys_info.reg_reject_info.rej_cause = rej_cause;
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "TDSCDMA, srv_status = %d, true_srv_status = %d, srv_domain = %d, "
                                "roam_status = %d, reject_info_valid = %d, reject_srv_domain = %d, "
                                "rej_cause = %d", srv_status, true_srv_status, domain, roam_status,
                                reg_reject_info_valid, reject_srv_domain, rej_cause);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    if (is_data_rat_change) {
      util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);
    }

    if (NAS_SYS_SRV_STATUS_SRV_V01 == srv_status)
    {
        //Expect: QMI_NAS_GET_CELL_LOCATION_INFO_REQ_MSG_V01
        QMI_node.update_default_qmi_nas_get_cell_location_info_req_msg();
        t.expect(QMI_node);

        // Inject: QMI_NAS_GET_CELL_LOCATION_INFO_RESP_MSG_V01
        QMI_node.update_default_qmi_nas_get_cell_location_info_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
        t.inject(QMI_node);
    }

    //Expect: RIL_REQUEST_REGISTRATION_STATE_RESP
    RIL_VoiceRegistrationStateResponse* reg_state_resp = RIL_node.update_default_ril_request_registration_state_resp();
    reg_state_resp->regState = (RIL_RegState)reg_state;
    reg_state_resp->rat = (RIL_RadioTechnology)rat;
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "RAT = %d, REG_STATE= %d", rat, reg_state);
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);

    t.set_end();
}

void util_voice_registration_state_for_lte(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, OEMTestnode &OEM_node, int rat, int reg_state, nas_service_status_enum_type_v01 srv_status, nas_true_service_status_enum_type_v01 true_srv_status, nas_service_domain_enum_type_v01 domain, nas_roam_status_enum_type_v01 roam_status, uint8_t voice_support_on_lte, uint8_t reg_reject_info_valid, nas_service_domain_enum_type_v01 reject_srv_domain, uint8_t rej_cause, uint8_t net_id_valid, const char *mcc, const char *mnc, uint8_t csg_info_valid, uint32_t cid, uint32_t cid_name_len, uint16_t *cid_name, uint8_t lte_is_eb_supported_valid, nas_tri_state_boolean_type_v01 lte_is_eb_supported, bool is_data_rat_change)
{
    char tags[MAX_TAG_SIZE];
    t.set_start();

    //Inject: RIL_REQUEST_REGISTRATION_STATE
    RIL_node.update_default_ril_request_registration_state();
    t.inject(RIL_node);

    // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
    nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
    nas_3gpp2_srv_status_info_type_v01* cdma_srv_status_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_cdma_srv_status_info();
    cdma_srv_status_info_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    cdma_srv_status_info_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* gsm_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_gsm_srv_status_info();
    gsm_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_srv_status_info();
    wcdma_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* lte_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_lte_srv_status_info();
    lte_srv_status_resp->srv_status = srv_status;
    lte_srv_status_resp->true_srv_status = true_srv_status;
    lte_srv_status_resp->is_pref_data_path = 0;
    nas_lte_sys_info_type_v01* lte_sys_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_lte_sys_info();
    lte_sys_info_resp->common_sys_info.srv_domain_valid = TRUE;
    lte_sys_info_resp->common_sys_info.srv_domain = domain;
    lte_sys_info_resp->common_sys_info.srv_capability_valid = TRUE;
    lte_sys_info_resp->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
    lte_sys_info_resp->common_sys_info.roam_status_valid = TRUE;
    lte_sys_info_resp->common_sys_info.roam_status = roam_status;
    lte_sys_info_resp->threegpp_specific_sys_info.reg_reject_info_valid = reg_reject_info_valid;
    lte_sys_info_resp->threegpp_specific_sys_info.reg_reject_info.reject_srv_domain = reject_srv_domain;
    lte_sys_info_resp->threegpp_specific_sys_info.reg_reject_info.rej_cause = rej_cause;
    QMI_node.set_qmi_nas_get_sys_info_resp_voice_support_on_lte(voice_support_on_lte);
    lte_sys_info_resp->threegpp_specific_sys_info.network_id_valid = net_id_valid;
    if(NULL != mcc)
    {
        strcpy(lte_sys_info_resp->threegpp_specific_sys_info.network_id.mcc,mcc);
    }

    if(NULL != mnc)
    {
        strcpy(lte_sys_info_resp->threegpp_specific_sys_info.network_id.mnc,mnc);
    }

    ptr->lte_is_eb_supported_valid = lte_is_eb_supported_valid;
    ptr->lte_is_eb_supported = lte_is_eb_supported;

    QMI_node.set_qmi_nas_get_sys_info_resp_lte_csg_info(csg_info_valid, cid, cid_name_len, cid_name);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "LTE, srv_status = %d, true_srv_status = %d, srv_domain = %d, "
                                "roam_status = %d, reject_info_valid = %d, reject_srv_domain = %d, "
                                "rej_cause = %d, voice_support_on_lte = %d", srv_status,
                                true_srv_status, domain, roam_status, reg_reject_info_valid,
                                reject_srv_domain, rej_cause, voice_support_on_lte);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    if (is_data_rat_change)
    {
      util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);
    }

    if ( csg_info_valid )
    {
        // Expect: RIL_UNSOL_OEM_HOOK_RAW
        OEM_node.update_default_qcril_evt_hook_unsol_csg_id();
        OEM_node.update_qcril_evt_hook_unsol_with_csg_id(cid);
        OEM_node.set_expect_skip_verification_status(false);
        t.expect(OEM_node);

        // Expect: RIL_UNSOL_OEM_HOOK_RAW
        RIL_node.update_default_ril_unsol_oem_hook_raw();
        RIL_node.set_expect_skip_verification_status(false);
        t.expect(RIL_node);
    }

    if(NAS_SYS_SRV_STATUS_SRV_V01 == srv_status)
    {
        //Expect: QMI_NAS_GET_CELL_LOCATION_INFO_REQ_MSG_V01
        QMI_node.update_default_qmi_nas_get_cell_location_info_req_msg();
        t.expect(QMI_node);

        // Inject: QMI_NAS_GET_CELL_LOCATION_INFO_RESP_MSG_V01
        QMI_node.update_default_qmi_nas_get_cell_location_info_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
        t.inject(QMI_node);
    }

    //Expect: RIL_REQUEST_REGISTRATION_STATE_RESP
    RIL_VoiceRegistrationStateResponse* reg_state_resp = RIL_node.update_default_ril_request_registration_state_resp();
    reg_state_resp->regState = (RIL_RegState)reg_state;
    reg_state_resp->rat = (RIL_RadioTechnology)rat;
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "RAT = %d, REG_STATE= %d", rat, reg_state);
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);

    t.set_end();
}

void util_voice_registration_state_in_sglte_case(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int rat, int reg_state, nas_service_status_enum_type_v01 gsm_srv_status, nas_true_service_status_enum_type_v01 gsm_true_srv_status, nas_service_domain_enum_type_v01 gsm_domain, nas_roam_status_enum_type_v01 gsm_roam_status, uint8_t gsm_reg_reject_info_valid, nas_service_domain_enum_type_v01 gsm_reject_srv_domain, uint8_t gsm_rej_cause, nas_service_status_enum_type_v01 lte_srv_status, nas_true_service_status_enum_type_v01 lte_true_srv_status, nas_service_domain_enum_type_v01 lte_domain, nas_roam_status_enum_type_v01 lte_roam_status, uint8_t voice_support_on_lte, uint8_t lte_reg_reject_info_valid, nas_service_domain_enum_type_v01 lte_reject_srv_domain, uint8_t lte_rej_cause, uint8_t gsm_net_id_valid, const char *gsm_mcc, const char *gsm_mnc, uint8_t lte_net_id_valid, const char *lte_mcc, const char *lte_mnc, bool is_data_rat_change)
{
    char tags[MAX_TAG_SIZE];
    t.set_start();

    //Inject: RIL_REQUEST_REGISTRATION_STATE
    RIL_node.update_default_ril_request_registration_state();
    t.inject(RIL_node);

    // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
    nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
    nas_3gpp_srv_status_info_type_v01* gsm_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_gsm_srv_status_info();
    gsm_srv_status_resp->srv_status = gsm_srv_status;
    gsm_srv_status_resp->true_srv_status = gsm_true_srv_status;
    gsm_srv_status_resp->is_pref_data_path = 0;
    nas_gsm_sys_info_type_v01* gsm_sys_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_gsm_sys_info();
    gsm_sys_info_resp->common_sys_info.srv_domain_valid = TRUE;
    gsm_sys_info_resp->common_sys_info.srv_domain = gsm_domain;
    gsm_sys_info_resp->common_sys_info.srv_capability_valid = TRUE;
    gsm_sys_info_resp->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
    gsm_sys_info_resp->common_sys_info.roam_status_valid = TRUE;
    gsm_sys_info_resp->common_sys_info.roam_status = gsm_roam_status;
    gsm_sys_info_resp->threegpp_specific_sys_info.reg_reject_info_valid = gsm_reg_reject_info_valid;
    gsm_sys_info_resp->threegpp_specific_sys_info.reg_reject_info.reject_srv_domain = gsm_reject_srv_domain;
    gsm_sys_info_resp->threegpp_specific_sys_info.reg_reject_info.rej_cause = gsm_rej_cause;
    gsm_sys_info_resp->threegpp_specific_sys_info.network_id_valid = gsm_net_id_valid;
    if(NULL != gsm_mcc)
    {
        strcpy(gsm_sys_info_resp->threegpp_specific_sys_info.network_id.mcc,gsm_mcc);
    }

    if(NULL != gsm_mnc)
    {
        strcpy(gsm_sys_info_resp->threegpp_specific_sys_info.network_id.mnc,gsm_mnc);
    }
    nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_srv_status_info();
    wcdma_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* lte_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_lte_srv_status_info();
    lte_srv_status_resp->srv_status = lte_srv_status;
    lte_srv_status_resp->true_srv_status = lte_true_srv_status;
    lte_srv_status_resp->is_pref_data_path = 0;
    nas_lte_sys_info_type_v01* lte_sys_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_lte_sys_info();
    lte_sys_info_resp->common_sys_info.srv_domain_valid = TRUE;
    lte_sys_info_resp->common_sys_info.srv_domain = lte_domain;
    lte_sys_info_resp->common_sys_info.srv_capability_valid = TRUE;
    lte_sys_info_resp->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
    lte_sys_info_resp->common_sys_info.roam_status_valid = TRUE;
    lte_sys_info_resp->common_sys_info.roam_status = lte_roam_status;
    lte_sys_info_resp->threegpp_specific_sys_info.reg_reject_info_valid = lte_reg_reject_info_valid;
    lte_sys_info_resp->threegpp_specific_sys_info.reg_reject_info.reject_srv_domain = lte_reject_srv_domain;
    lte_sys_info_resp->threegpp_specific_sys_info.reg_reject_info.rej_cause = lte_rej_cause;
    QMI_node.set_qmi_nas_get_sys_info_resp_voice_support_on_lte(voice_support_on_lte);
    lte_sys_info_resp->threegpp_specific_sys_info.network_id_valid = lte_net_id_valid;
    if(NULL != lte_mcc)
    {
        strcpy(lte_sys_info_resp->threegpp_specific_sys_info.network_id.mcc,lte_mcc);
    }

    if(NULL != lte_mnc)
    {
        strcpy(lte_sys_info_resp->threegpp_specific_sys_info.network_id.mnc,lte_mnc);
    }
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "GSM: gsm_srv_status = %d, gsm_srv_domain = %d. "
                                "LTE: lte_srv_status = %d, lte_srv_domain = %d.",
                                gsm_srv_status, gsm_domain, lte_srv_status, lte_domain);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    if (is_data_rat_change) {
      util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);
    }

    if ((NAS_SYS_SRV_STATUS_SRV_V01 == gsm_srv_status) || (NAS_SYS_SRV_STATUS_SRV_V01 == lte_srv_status))
    {
        //Expect: QMI_NAS_GET_CELL_LOCATION_INFO_REQ_MSG_V01
        QMI_node.update_default_qmi_nas_get_cell_location_info_req_msg();
        t.expect(QMI_node);

        // Inject: QMI_NAS_GET_CELL_LOCATION_INFO_RESP_MSG_V01
        QMI_node.update_default_qmi_nas_get_cell_location_info_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
        t.inject(QMI_node);
    }

    //Expect: RIL_REQUEST_REGISTRATION_STATE_RESP
    RIL_VoiceRegistrationStateResponse* reg_state_resp = RIL_node.update_default_ril_request_registration_state_resp();
    reg_state_resp->regState = (RIL_RegState)reg_state;
    reg_state_resp->rat = (RIL_RadioTechnology)rat;
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "RAT = %d, REG_STATE= %d", rat, reg_state);
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);

    t.set_end();
}

void util_voice_registration_state_in_svlte_case(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int rat, int reg_state, nas_service_status_enum_type_v01 cdma_srv_status, nas_service_domain_enum_type_v01 cdma_domain, nas_roam_status_enum_type_v01 cdma_roam_status, nas_service_status_enum_type_v01 lte_srv_status, nas_true_service_status_enum_type_v01 lte_true_srv_status, nas_service_domain_enum_type_v01 lte_domain, nas_roam_status_enum_type_v01 lte_roam_status, uint8_t voice_support_on_lte, uint8_t lte_reg_reject_info_valid, nas_service_domain_enum_type_v01 lte_reject_srv_domain, uint8_t lte_rej_cause, bool is_data_rat_change)
{
    char tags[MAX_TAG_SIZE];
    t.set_start();

    //Inject: RIL_REQUEST_REGISTRATION_STATE
    RIL_node.update_default_ril_request_registration_state();
    t.inject(RIL_node);

    // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
    nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
    nas_3gpp_srv_status_info_type_v01* gsm_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_gsm_srv_status_info();
    gsm_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_srv_status_info();
    wcdma_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp2_srv_status_info_type_v01* cdma_srv_status_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_cdma_srv_status_info();
    cdma_srv_status_info_resp->srv_status = cdma_srv_status;
    cdma_srv_status_info_resp->is_pref_data_path = 0;
    nas_cdma_sys_info_type_v01* cdma_sys_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_cdma_sys_info();
    cdma_sys_info_resp->common_sys_info.srv_domain_valid = TRUE;
    cdma_sys_info_resp->common_sys_info.srv_domain = cdma_domain;
    cdma_sys_info_resp->common_sys_info.srv_capability_valid = TRUE;
    cdma_sys_info_resp->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
    cdma_sys_info_resp->common_sys_info.roam_status_valid = TRUE;
    cdma_sys_info_resp->common_sys_info.roam_status = cdma_roam_status;
    nas_3gpp_srv_status_info_type_v01* lte_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_lte_srv_status_info();
    lte_srv_status_resp->srv_status = lte_srv_status;
    lte_srv_status_resp->true_srv_status = lte_true_srv_status;
    lte_srv_status_resp->is_pref_data_path = 0;
    nas_lte_sys_info_type_v01* lte_sys_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_lte_sys_info();
    lte_sys_info_resp->common_sys_info.srv_domain_valid = TRUE;
    lte_sys_info_resp->common_sys_info.srv_domain = lte_domain;
    lte_sys_info_resp->common_sys_info.srv_capability_valid = TRUE;
    lte_sys_info_resp->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
    lte_sys_info_resp->common_sys_info.roam_status_valid = TRUE;
    lte_sys_info_resp->common_sys_info.roam_status = lte_roam_status;
    lte_sys_info_resp->threegpp_specific_sys_info.reg_reject_info_valid = lte_reg_reject_info_valid;
    lte_sys_info_resp->threegpp_specific_sys_info.reg_reject_info.reject_srv_domain = lte_reject_srv_domain;
    lte_sys_info_resp->threegpp_specific_sys_info.reg_reject_info.rej_cause = lte_rej_cause;
    QMI_node.set_qmi_nas_get_sys_info_resp_voice_support_on_lte(voice_support_on_lte);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "CDMA: cdma_srv_status = %d, cdma_srv_domain = %d. "
                                "LTE: lte_srv_status = %d, lte_srv_domain = %d",
                                cdma_srv_status, cdma_domain, lte_srv_status, lte_domain);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    if (is_data_rat_change)
    {
      util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);
    }

    if((NAS_SYS_SRV_STATUS_SRV_V01 == lte_srv_status) && (RADIO_TECH_LTE == rat))
    {
        //Expect: QMI_NAS_GET_CELL_LOCATION_INFO_REQ_MSG_V01
        QMI_node.update_default_qmi_nas_get_cell_location_info_req_msg();
        t.expect(QMI_node);

        // Inject: QMI_NAS_GET_CELL_LOCATION_INFO_RESP_MSG_V01
        QMI_node.update_default_qmi_nas_get_cell_location_info_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
        t.inject(QMI_node);
    }

    //Expect: RIL_REQUEST_REGISTRATION_STATE_RESP
    RIL_VoiceRegistrationStateResponse* reg_state_resp = RIL_node.update_default_ril_request_registration_state_resp();
    reg_state_resp->regState = (RIL_RegState)reg_state;
    reg_state_resp->rat = (RIL_RadioTechnology)rat;
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "RAT = %d, REG_STATE= %d", rat, reg_state);
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);

    t.set_end();
}

void util_voice_registration_state_in_svdo_case(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int rat, int reg_state, nas_service_status_enum_type_v01 cdma_srv_status, nas_service_domain_enum_type_v01 cdma_domain, nas_roam_status_enum_type_v01 cdma_roam_status, nas_service_status_enum_type_v01 hdr_srv_status, nas_service_domain_enum_type_v01 hdr_domain, nas_roam_status_enum_type_v01 hdr_roam_status, uint8_t hdr_personality_valid, nas_hdr_personality_enum_type_v01 hdr_personality, uint8_t hdr_active_prot_valid, nas_hdr_active_prot_enum_type_v01 hdr_active_prot, bool is_data_rat_change)
{
    char tags[MAX_TAG_SIZE];
    t.set_start();

    //Inject: RIL_REQUEST_REGISTRATION_STATE
    RIL_node.update_default_ril_request_registration_state();
    t.inject(RIL_node);

    // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
    nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
    nas_3gpp_srv_status_info_type_v01* gsm_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_gsm_srv_status_info();
    gsm_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_srv_status_info();
    wcdma_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp2_srv_status_info_type_v01* cdma_srv_status_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_cdma_srv_status_info();
    cdma_srv_status_info_resp->srv_status = cdma_srv_status;
    cdma_srv_status_info_resp->is_pref_data_path = 0;
    nas_cdma_sys_info_type_v01* cdma_sys_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_cdma_sys_info();
    cdma_sys_info_resp->common_sys_info.srv_domain_valid = TRUE;
    cdma_sys_info_resp->common_sys_info.srv_domain = cdma_domain;
    cdma_sys_info_resp->common_sys_info.srv_capability_valid = TRUE;
    cdma_sys_info_resp->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
    cdma_sys_info_resp->common_sys_info.roam_status_valid = TRUE;
    cdma_sys_info_resp->common_sys_info.roam_status = cdma_roam_status;
    nas_3gpp_srv_status_info_type_v01* lte_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_lte_srv_status_info();
    lte_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    lte_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    lte_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp2_srv_status_info_type_v01* hdr_srv_status_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_hdr_srv_status_info();
    hdr_srv_status_info_resp->srv_status = hdr_srv_status;
    hdr_srv_status_info_resp->is_pref_data_path = 0;
    nas_hdr_sys_info_type_v01* hdr_sys_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_hdr_sys_info();
    hdr_sys_info_resp->common_sys_info.srv_domain_valid = TRUE;
    hdr_sys_info_resp->common_sys_info.srv_domain = hdr_domain;
    hdr_sys_info_resp->common_sys_info.srv_capability_valid = TRUE;
    hdr_sys_info_resp->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
    hdr_sys_info_resp->common_sys_info.roam_status_valid = TRUE;
    hdr_sys_info_resp->common_sys_info.roam_status = hdr_roam_status;
    hdr_sys_info_resp->hdr_specific_sys_info.hdr_personality_valid = hdr_personality_valid;
    hdr_sys_info_resp->hdr_specific_sys_info.hdr_personality = hdr_personality;
    hdr_sys_info_resp->hdr_specific_sys_info.hdr_active_prot_valid = hdr_active_prot_valid;
    hdr_sys_info_resp->hdr_specific_sys_info.hdr_active_prot = hdr_active_prot;
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "CDMA: cdma_srv_status = %d, cdma_srv_domain = %d. "
                                "HDE: hdr_srv_status = %d, hdr_srv_domain = %d",
                                cdma_srv_status, cdma_domain, hdr_srv_status, hdr_domain);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    if (is_data_rat_change)
    {
      util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);
    }

    //Expect: RIL_REQUEST_REGISTRATION_STATE_RESP
    RIL_VoiceRegistrationStateResponse* reg_state_resp = RIL_node.update_default_ril_request_registration_state_resp();
    reg_state_resp->regState = (RIL_RegState)reg_state;
    reg_state_resp->rat = (RIL_RadioTechnology)rat;
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "RAT = %d, REG_STATE= %d", rat, reg_state);
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);

    t.set_end();
}

void util_voice_registration_state_for_cdma(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int rat, int reg_state, nas_service_status_enum_type_v01 srv_status, nas_service_domain_enum_type_v01 domain, nas_roam_status_enum_type_v01 roam_status, bool is_data_rat_change)
{
    char tags[MAX_TAG_SIZE];
    t.set_start();

    //Inject: RIL_REQUEST_REGISTRATION_STATE
    RIL_node.update_default_ril_request_registration_state();
    t.inject(RIL_node);

    // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
    nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
    nas_3gpp_srv_status_info_type_v01* gsm_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_gsm_srv_status_info();
    gsm_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_srv_status_info();
    wcdma_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp2_srv_status_info_type_v01* cdma_srv_status_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_cdma_srv_status_info();
    cdma_srv_status_info_resp->srv_status = srv_status;
    cdma_srv_status_info_resp->is_pref_data_path = 0;
    nas_cdma_sys_info_type_v01* cdma_sys_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_cdma_sys_info();
    cdma_sys_info_resp->common_sys_info.srv_domain_valid = TRUE;
    cdma_sys_info_resp->common_sys_info.srv_domain = domain;
    cdma_sys_info_resp->common_sys_info.srv_capability_valid = TRUE;
    cdma_sys_info_resp->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
    cdma_sys_info_resp->common_sys_info.roam_status_valid = TRUE;
    cdma_sys_info_resp->common_sys_info.roam_status = roam_status;
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "CDMA: cdma_srv_status = %d, cdma_srv_domain = %d, cdma_roam_status = %d",
            srv_status, domain, roam_status);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    if (is_data_rat_change) {
      util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);
    }

    //Expect: RIL_REQUEST_REGISTRATION_STATE_RESP
    RIL_VoiceRegistrationStateResponse* reg_state_resp = RIL_node.update_default_ril_request_registration_state_resp();
    reg_state_resp->regState = (RIL_RegState)reg_state;
    reg_state_resp->rat = (RIL_RadioTechnology)rat;
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "RAT = %d, REG_STATE= %d", rat, reg_state);
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);

    t.set_end();
}

void util_voice_registration_state_in_oos( Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, bool is_data_rat_change)
{
    char tags[MAX_TAG_SIZE];
    t.set_start();

    //Inject: RIL_REQUEST_REGISTRATION_STATE
    RIL_node.update_default_ril_request_registration_state();
    t.inject(RIL_node);

    // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
    nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
    nas_3gpp_srv_status_info_type_v01* gsm_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_gsm_srv_status_info();
    gsm_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_srv_status_info();
    wcdma_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp2_srv_status_info_type_v01* cdma_srv_status_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_cdma_srv_status_info();
    cdma_srv_status_info_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    cdma_srv_status_info_resp->is_pref_data_path = 0;
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "All Rats are OOS");
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    if (is_data_rat_change)
    {
      util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);
    }

    //Expect: RIL_REQUEST_REGISTRATION_STATE_RESP
    RIL_VoiceRegistrationStateResponse* reg_state_resp = RIL_node.update_default_ril_request_registration_state_resp();
    reg_state_resp->regState = (RIL_RegState)RIL_VAL_REG_NOT_REGISTERED_SEARCHING;
    reg_state_resp->rat = (RIL_RadioTechnology)RADIO_TECH_UNKNOWN;
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "RAT = %d, REG_STATE= %d",
            RADIO_TECH_UNKNOWN, RIL_VAL_REG_NOT_REGISTERED_SEARCHING);
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);

    t.set_end();
}

void util_data_registration_state_for_hdr(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, int rat, int reg_state, nas_service_status_enum_type_v01 srv_status, nas_service_domain_enum_type_v01 domain, nas_roam_status_enum_type_v01 roam_status, uint8_t hdr_personality_valid, nas_hdr_personality_enum_type_v01 hdr_personality, uint8_t hdr_active_prot_valid, nas_hdr_active_prot_enum_type_v01 hdr_active_prot )
{
    t.set_start();

    //Inject: RIL_REQUEST_REGISTRATION_STATE
    RIL_node.update_default_ril_request_registration_state();
    t.inject(RIL_node);

    // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
    nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
    nas_3gpp_srv_status_info_type_v01* gsm_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_gsm_srv_status_info();
    gsm_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_srv_status_info();
    wcdma_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp2_srv_status_info_type_v01* cdma_srv_status_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_cdma_srv_status_info();
    cdma_srv_status_info_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    cdma_srv_status_info_resp->is_pref_data_path = 0;
    nas_3gpp2_srv_status_info_type_v01* hdr_srv_status_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_hdr_srv_status_info();
    hdr_srv_status_info_resp->srv_status = srv_status;
    hdr_srv_status_info_resp->is_pref_data_path = 0;
    nas_hdr_sys_info_type_v01* hdr_sys_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_hdr_sys_info();
    hdr_sys_info_resp->common_sys_info.srv_domain_valid = TRUE;
    hdr_sys_info_resp->common_sys_info.srv_domain = domain;
    hdr_sys_info_resp->common_sys_info.srv_capability_valid = TRUE;
    hdr_sys_info_resp->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
    hdr_sys_info_resp->common_sys_info.roam_status_valid = TRUE;
    hdr_sys_info_resp->common_sys_info.roam_status = roam_status;
    hdr_sys_info_resp->hdr_specific_sys_info.hdr_personality_valid = hdr_personality_valid;
    hdr_sys_info_resp->hdr_specific_sys_info.hdr_personality = hdr_personality;
    hdr_sys_info_resp->hdr_specific_sys_info.hdr_active_prot_valid = hdr_active_prot_valid;
    hdr_sys_info_resp->hdr_specific_sys_info.hdr_active_prot = hdr_active_prot;
    t.inject(QMI_node);

    //Expect: RIL_REQUEST_REGISTRATION_STATE_RESP
    RIL_VoiceRegistrationStateResponse* reg_state_resp = RIL_node.update_default_ril_request_registration_state_resp();
    reg_state_resp->regState = (RIL_RegState)reg_state;
    reg_state_resp->rat = (RIL_RadioTechnology)rat;
    t.expect(RIL_node);

    t.set_end();
}

void util_ril_utf_network_scan ( Testcase &t, AndroidTelTestnode &RIL_node, QMIRILTestnode &QMI_node, int scan_net_type, int qmi_resp, int qmi_error, int res, int nw_cnt, int is_legacy, int is_combine, int is_plmn_diff_nw_status)
{
    int j = 0, i =0;
    uint16_t mcc = 123, mnc = 45, t_mcc = 0, t_mnc =0, mccmnc=12345, t_duplicate_plmn = FALSE;;
    uint8_t network_status = 2;//available
    uint8_t rat = 5, t_rat = 0;//WCDMA
    char plmn_short_name[NAS_PLMN_NAME_MAX_V01], plmn_long_name[NAS_PLMN_NAME_MAX_V01], mcc_mnc[NAS_NW_SCAN_RESP_MCCMNC_SIZE];
    char tags[MAX_TAG_SIZE];

    t_mcc = mcc;
    t_mnc = mnc;
    t_rat = rat;

    t.set_start();

    // Inject: RIL_REQUEST_QUERY_AVAILABLE_NETWORKS
    RIL_node.update_default_ril_request_query_available_networks();
    t.inject(RIL_node);

    // Expect: QMI_NAS_PERFORM_NETWORK_SCAN_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_perfrom_network_scan_req_msg();
    QMI_node.set_qmi_nas_perform_network_scan_req_msg_network_type(scan_net_type);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "scan_network_type = %d", scan_net_type);
    QMI_node.add_callflow_tag(tags);
    t.expect(QMI_node);

    if(RIL_E_SUCCESS == res)
    {
        //Inject: QMI_NAS_PERFORM_NETWORK_SCAN_RESP_MSG_V01
        QMI_node.update_default_qmi_nas_perform_network_scan_resp_msg((qmi_result_type_v01)qmi_resp, (qmi_error_type_v01)qmi_error);
        for(j=0;j<nw_cnt;j++)
        {
            if((FALSE == is_plmn_diff_nw_status) || (TRUE == t_duplicate_plmn))
            {
                QMI_node.set_qmi_nas_3gpp_network_info_tlv_network_scan_resp_msg(nw_cnt, t_mcc, t_mnc, network_status, NULL);
            }
            else
            {
                t_duplicate_plmn = TRUE;
                QMI_node.set_qmi_nas_3gpp_network_info_tlv_network_scan_resp_msg(nw_cnt, t_mcc, t_mnc, 16, NULL);
            }
            QMI_node.set_qmi_nas_radio_access_technology_tlv_network_scan_resp_msg(t_mcc, t_mnc, t_rat);
            QMI_node.set_qmi_nas_mnc_pcs_digit_tlv_network_scan_resp_msg(t_mcc, t_mnc, FALSE);
            t_mcc;
            if(FALSE == is_plmn_diff_nw_status)
            {
                t_mnc++;
            }
            t_rat;
        }
        QMI_node.set_qmi_nas_scan_result_tlv_network_scan_resp_msg(NAS_SCAN_SUCCESS_V01);
        memset(tags,'\0', MAX_TAG_SIZE);
        snprintf(tags, MAX_TAG_SIZE, "Response Success: scan_network_count = %d", nw_cnt);
        QMI_node.add_callflow_tag(tags);
        t.inject(QMI_node);

        t_mcc = mcc;
        t_mnc = mnc;
        t_rat = rat;

        for(j=0;j<nw_cnt;j++)
        {
            // Expect: QMI_NAS_GET_PLMN_NAME_REQ_MSG_V01
            QMI_node.update_default_qmi_nas_get_plmn_name_req_msg(t_mcc, t_mnc);
            QMI_node.set_qmi_nas_get_plmn_name_req_suppress_sim_error(TRUE);
            QMI_node.set_qmi_nas_get_plmn_name_req_rat((nas_radio_if_enum_v01)t_rat);
            memset(tags,'\0', MAX_TAG_SIZE);
            snprintf(tags, MAX_TAG_SIZE, "MCC = %d, MNC = %d, RAT = %d", t_mcc, t_mnc, t_rat);
            QMI_node.add_callflow_tag(tags);
            t.expect(QMI_node);

            // Inject: QMI_NAS_GET_PLMN_NAME_RESP_MSG_V01
            QMI_node.update_default_qmi_nas_plmn_name_resp_msg(QMI_RESULT_SUCCESS_V01, (qmi_error_type_v01)0);
            nas_3gpp_eons_plmn_name_type_v01* eons_plmn_name = QMI_node.set_qmi_nas_get_plmn_name_resp_eons_plmn_name_3gpp();
            bzero(plmn_long_name,NAS_PLMN_NAME_MAX_V01);
            bzero(plmn_short_name,NAS_PLMN_NAME_MAX_V01);
            snprintf(plmn_long_name, NAS_PLMN_NAME_MAX_V01, "airtel%d", j);
            strcpy(eons_plmn_name->plmn_long_name,plmn_long_name);
            eons_plmn_name->plmn_long_name_len = strlen(eons_plmn_name->plmn_long_name);
            snprintf(plmn_short_name, NAS_PLMN_NAME_MAX_V01, "vodafone%d", j);
            strcpy(eons_plmn_name->plmn_short_name,plmn_short_name);
            eons_plmn_name->plmn_short_name_len = strlen(eons_plmn_name->plmn_short_name);
            memset(tags,'\0', MAX_TAG_SIZE);
            snprintf(tags, MAX_TAG_SIZE, "LONG_PLMN_NAME = %s, SHORT_PLMN_NAME = %s",
                    plmn_long_name, plmn_short_name);
            QMI_node.add_callflow_tag(tags);
            t.inject(QMI_node);

            t_mcc;
            if(FALSE == is_plmn_diff_nw_status)
            {
                t_mnc++;
            }
            t_rat;
        }

        if(TRUE == is_plmn_diff_nw_status)
        {
            nw_cnt=1;
        }

        //Expect: RIL_REQUEST_QUERY_AVAILABLE_NETWORKS
        ril_request_query_available_networks_resp_t* ptr= RIL_node.update_default_ril_request_query_available_networks_resp(nw_cnt, is_legacy);
        j=0;
        for(i=0;i<nw_cnt;i++)
        {
            bzero(plmn_long_name,NAS_PLMN_NAME_MAX_V01);
            bzero(plmn_short_name,NAS_PLMN_NAME_MAX_V01);
            snprintf(plmn_long_name, NAS_PLMN_NAME_MAX_V01, "airtel%d", i);
            strcpy(ptr->nw_scan_info_array[j++],plmn_long_name);
            snprintf(plmn_short_name, NAS_PLMN_NAME_MAX_V01, "vodafone%d", i);
            strcpy(ptr->nw_scan_info_array[j++],plmn_short_name);
            snprintf(mcc_mnc, NAS_NW_SCAN_RESP_MCCMNC_SIZE, "%d", mccmnc);
            strcpy(ptr->nw_scan_info_array[j++],mcc_mnc);
            if(is_combine)
            {
                strcat(ptr->nw_scan_info_array[j-1],"+");
                strcat(ptr->nw_scan_info_array[j-1],"3");
            }
            strcpy(ptr->nw_scan_info_array[j++],"available");
            if(is_legacy)
            {
                strcpy(ptr->nw_scan_info_array[j++],"3");
            }

            mccmnc++;
        }
        memset(tags,'\0', MAX_TAG_SIZE);
        if(is_legacy)
        {
            snprintf(tags, MAX_TAG_SIZE, "LEGACY: Long = %s, Short = %s, MCCMNC = %s, "
                                        "Net_Status = %s, RAT = %s", ptr->nw_scan_info_array[0],
                                        ptr->nw_scan_info_array[1], ptr->nw_scan_info_array[2],
                                        ptr->nw_scan_info_array[3],ptr->nw_scan_info_array[4]);
        }
        else if(is_combine)
        {
            snprintf(tags, MAX_TAG_SIZE, "COMBINE: Long = %s, Short = %s, MCCMNC+RAT = %s, "
                                        "Net_Status = %s", ptr->nw_scan_info_array[0],
                                        ptr->nw_scan_info_array[1], ptr->nw_scan_info_array[2],
                                        ptr->nw_scan_info_array[3]);
        }
        else
        {
            snprintf(tags, MAX_TAG_SIZE, "Without RAT: Long = %s, Short = %s, MCCMNC = %s, "
                                        "Net_Status = %s", ptr->nw_scan_info_array[0],
                                        ptr->nw_scan_info_array[1], ptr->nw_scan_info_array[2],
                                        ptr->nw_scan_info_array[3]);
        }
        RIL_node.add_callflow_tag(tags);
        t.expect(RIL_node);
    }
    else
    {
        //Inject: QMI_NAS_PERFORM_NETWORK_SCAN_RESP_MSG_V01
        QMI_node.update_default_qmi_nas_perform_network_scan_resp_msg((qmi_result_type_v01)qmi_resp, (qmi_error_type_v01)qmi_error);
        if(QMI_RESULT_SUCCESS_V01 == qmi_resp)
        {
            QMI_node.set_qmi_nas_3gpp_network_info_tlv_network_scan_resp_msg(nw_cnt, 0, 0, 0, NULL);
            QMI_node.set_qmi_nas_scan_result_tlv_network_scan_resp_msg(NAS_SCAN_SUCCESS_V01);
        }
        memset(tags,'\0', MAX_TAG_SIZE);
        snprintf(tags, MAX_TAG_SIZE, "Response = %d, Error = %d, NW_CNT = %d",qmi_resp, qmi_error, nw_cnt);
        QMI_node.add_callflow_tag(tags);
        t.inject(QMI_node);

        //Expect: RIL_REQUEST_QUERY_AVAILABLE_NETWORKS
        RIL_node.update_default_ril_request_query_available_networks_resp_failure();
        RIL_node.set_ril_error_value((RIL_Errno)res);
        memset(tags,'\0', MAX_TAG_SIZE);
        snprintf(tags, MAX_TAG_SIZE, "Response Failure");
        RIL_node.add_callflow_tag(tags);
        t.expect(RIL_node);
    }

    t.set_end();
}

void util_to_get_operator_name_oos_limited_case(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, uint8_t is_gsm,
const char * mcc, const char * mnc, const char *long_eons, const char *short_eons)
{
    char tags[MAX_TAG_SIZE];
    t.set_start();

#ifdef QMI_RIL_HAL_UTF
    NOTUSED(QMI_node);
#endif

    // Inject: RIL_REQUEST_OPERATOR
    RIL_node.update_default_ril_request_operator();
    t.inject(RIL_node);

#ifndef QMI_RIL_HAL_UTF
    if(FALSE == is_gsm)
    {
        // Expect: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_REQ_V01
        QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_req_v01();
        t.expect(QMI_node);

        // Inject: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_RSP_V01
        QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_rsp_v01();
        QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
        t.inject(QMI_node);
    }
#endif

    // Expect: RIL_REQUEST_OPERATOR_RESP
    ril_request_operator_resp_from_qcril_t* ril_req_operator_name = RIL_node.update_default_ril_request_operator_resp();
    if(is_gsm)
    {
        if(NULL != long_eons)
        {
            strcpy(ril_req_operator_name->operator_info_array[0],long_eons);
        }
        else
        {
            strcat(ril_req_operator_name->operator_info_array[0],mcc);
            strcat(ril_req_operator_name->operator_info_array[0],mnc);
        }

        if(NULL != short_eons)
        {
            strcpy(ril_req_operator_name->operator_info_array[1],short_eons);
        }

        strcat(ril_req_operator_name->operator_info_array[2],mcc);
        strcat(ril_req_operator_name->operator_info_array[2],mnc);
    }

    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "long_name = %s, short_name = %s, mccmnc = %s",
            ril_req_operator_name->operator_info_array[0],
            ril_req_operator_name->operator_info_array[1],
            ril_req_operator_name->operator_info_array[2]);
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);

    t.set_end();
}

void util_to_update_cache_current_plmn_ind(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, uint8_t plmn_valid, uint16_t mcc, uint16_t mnc, uint8_t pcs_digit, uint8_t spn_valid, nas_coding_scheme_enum_v01 spn_enc, uint32_t spn_len, const char *spn, uint8_t short_name_valid, nas_coding_scheme_enum_v01 short_plmn_name_enc, uint32_t short_plmn_name_len, const char *short_name, uint8_t long_name_valid, nas_coding_scheme_enum_v01 long_plmn_name_enc, uint32_t long_plmn_name_len, const char *long_name)
{
    char tags[MAX_TAG_SIZE];

    t.set_start();

    // Inject: QMI_NAS_CURRENT_PLMN_NAME_IND_V01
    QMI_node.update_default_qmi_nas_current_plmn_name_ind_msg();
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "PLMN Valid = %d, MCC = %d, MNC = %d, SPN Valid = %d, "
                                "Short Name Valid = %d, Long Name Valid = %d", plmn_valid,
                                mcc, mnc, spn_valid, short_name_valid, long_name_valid);
    QMI_node.set_qmi_nas_current_plmn_name_ind_plmn_id(plmn_valid, mcc, mnc, pcs_digit);
    QMI_node.set_qmi_nas_current_plmn_name_ind_spn(spn_valid, spn_enc, spn_len, spn);
    if(short_name_valid)
    {
        nas_plmn_name_type_v01* short_name_data = QMI_node.set_qmi_nas_current_plmn_name_ind_short_name();
        short_name_data->plmn_name_enc = short_plmn_name_enc;
        short_name_data->plmn_name_len = short_plmn_name_len;
        memcpy(short_name_data->plmn_name, short_name, short_plmn_name_len);
    }

    if(long_name_valid)
    {
        nas_plmn_name_type_v01* long_name_data = QMI_node.set_qmi_nas_current_plmn_name_ind_long_name();
        long_name_data->plmn_name_enc = long_plmn_name_enc;
        long_name_data->plmn_name_len = long_plmn_name_len;
        memcpy(long_name_data->plmn_name, long_name, long_plmn_name_len);
    }
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    // Expect: RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED
    if(plmn_valid || spn_valid || short_name_valid || long_name_valid)
    {
        RIL_node.update_default_ril_unsol_response_voice_network_state_changed();
        t.expect(RIL_node);
    }

    t.set_end();
}

void util_to_get_operator_name_3gpp2(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, const char *mcc,
const char *mnc, uint16_t sid, uint16_t nid, const char *long_plmn_name, const char * short_plmn_name, uint8_t is_3gpp2_subs_info_needed, qmi_result_type_v01 result,qmi_error_type_v01 error, uint8_t is_fill_eons_needed )
{
    int used_nam_name_len = 0;
    char mcc_mnc[16];
    char tags[MAX_TAG_SIZE];

    t.set_start();

    // Inject: RIL_REQUEST_OPERATOR
    RIL_node.update_default_ril_request_operator();
    t.inject(RIL_node);

    // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
    nas_get_sys_info_resp_msg_v01 *ptr = QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
    nas_3gpp_srv_status_info_type_v01* gsm_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_gsm_srv_status_info();
    gsm_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    gsm_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp_srv_status_info_type_v01* wcdma_srv_status_resp = QMI_node.set_qmi_nas_get_sys_info_resp_wcdma_srv_status_info();
    wcdma_srv_status_resp->srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->true_srv_status = SYS_SRV_STATUS_NO_SRV_V01;
    wcdma_srv_status_resp->is_pref_data_path = 0;
    nas_3gpp2_srv_status_info_type_v01* cdma_srv_status_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_cdma_srv_status_info();
    cdma_srv_status_info_resp->srv_status = NAS_SYS_SRV_STATUS_SRV_V01;
    cdma_srv_status_info_resp->is_pref_data_path = 0;
    nas_cdma_sys_info_type_v01* cdma_sys_info_resp = QMI_node.set_qmi_nas_get_sys_info_resp_cdma_sys_info();
    cdma_sys_info_resp->common_sys_info.srv_domain_valid = TRUE;
    cdma_sys_info_resp->common_sys_info.srv_domain = SYS_SRV_DOMAIN_CS_ONLY_V01;
    cdma_sys_info_resp->common_sys_info.srv_capability_valid = TRUE;
    cdma_sys_info_resp->common_sys_info.srv_capability = SYS_SRV_DOMAIN_CS_PS_V01;
    cdma_sys_info_resp->common_sys_info.roam_status_valid = TRUE;
    cdma_sys_info_resp->common_sys_info.roam_status = NAS_SYS_ROAM_STATUS_OFF_V01;
    cdma_sys_info_resp->cdma_specific_sys_info.network_id_valid = TRUE;
    strcpy(cdma_sys_info_resp->cdma_specific_sys_info.network_id.mcc,mcc);
    strcpy(cdma_sys_info_resp->cdma_specific_sys_info.network_id.mnc,mnc);
    cdma_sys_info_resp->cdma_specific_sys_info.cdma_sys_id_valid = TRUE;
    cdma_sys_info_resp->cdma_specific_sys_info.cdma_sys_id.sid = sid;
    cdma_sys_info_resp->cdma_specific_sys_info.cdma_sys_id.nid = nid;
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "CDMA: srv_status = %d, srv_domain = %d, MCC = %s,"
                                "MNC = %s, SID = %d, NID = %d", NAS_SYS_SRV_STATUS_SRV_V01,
                                SYS_SRV_DOMAIN_CS_ONLY_V01, mcc, mnc, sid, nid);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    if(is_3gpp2_subs_info_needed)
    {
        // Expect: QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO_REQ_MSG_V01
        QMI_node.update_default_qmi_nas_get_3gpp2_subscription_info_req_msg(0xFF);
        QMI_node.set_qmi_nas_get_3gpp2_subscription_info_req_get_3gpp2_info_mask(QMI_NAS_GET_3GPP2_SUBS_INFO_NAM_NAME_V01);
        memset(tags,'\0', MAX_TAG_SIZE);
        snprintf(tags, MAX_TAG_SIZE, "GET_NAM_NAME_SUBS_INFO_REQ");
        QMI_node.add_callflow_tag(tags);
        t.expect(QMI_node);

        // Inject: QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO_RESP_MSG_V01
        QMI_node.upate_default_qmi_nas_get_3gpp2_subscription_info_resp_msg(result, error);
        memset(tags,'\0', MAX_TAG_SIZE);
        if( ( QMI_RESULT_SUCCESS_V01 == result) && (NULL != long_plmn_name) )
        {
           char* nam_name = QMI_node.set_qmi_nas_get_3gpp2_subscription_info_resp_nam_name(strlen(long_plmn_name)+1);
           strcpy(nam_name,long_plmn_name);
           snprintf(tags, MAX_TAG_SIZE, "NAM_NAME = %s", long_plmn_name);
        }
        else
        {
           snprintf(tags, MAX_TAG_SIZE, "Response Failure");
        }
        QMI_node.add_callflow_tag(tags);
        t.inject(QMI_node);
    }

    // Expect: RIL_REQUEST_OPERATOR_RESP
    ril_request_operator_resp_from_qcril_t* ril_req_operator_name = RIL_node.update_default_ril_request_operator_resp();
    if ( (NULL != long_plmn_name) && (is_3gpp2_subs_info_needed))
    {
        used_nam_name_len = MIN((strlen(long_plmn_name)+1), MIN(NAS_MAX_NAM_NAME_LEN_V01,(NAS_OPERATOR_RESP_MAX_EONS_LEN-1) ) );
        strncpy(ril_req_operator_name->operator_info_array[0],long_plmn_name, used_nam_name_len);
    }
    else if(NULL != long_plmn_name)
    {
        strcpy(ril_req_operator_name->operator_info_array[0],long_plmn_name);
    }

    if(NULL != short_plmn_name)
    {
        strcpy(ril_req_operator_name->operator_info_array[1],short_plmn_name);
    }
    else if(is_fill_eons_needed)
    {
        strcpy(ril_req_operator_name->operator_info_array[1],ril_req_operator_name->operator_info_array[0]);
    }

    memset(mcc_mnc, '\0', 16);
    strcat(mcc_mnc, mcc);
    strcat(mcc_mnc, mnc);
    strcpy( ril_req_operator_name->operator_info_array[2], mcc_mnc );

    if(NULL == long_plmn_name)
    {
        strcpy(ril_req_operator_name->operator_info_array[0],mcc_mnc);
    }
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "long_name = %s, short_name = %s, mccmnc = %s",
            ril_req_operator_name->operator_info_array[0],
            ril_req_operator_name->operator_info_array[1],
            ril_req_operator_name->operator_info_array[2]);
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);

    t.set_end();
}
void util_to_get_operator_name_3gpp(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, const char *mcc_ptr, const char *mnc_ptr, uint8_t csg_id_valid, uint32_t csg_id, nas_radio_if_enum_v01 rat, const char *long_plmn_name, const char * short_plmn_name, uint8_t is_fill_eons_needed, uint8_t is_static_table, uint8_t is_default_oprtr_name, uint8_t is_always_send_plmn_name, uint8_t is_se_table, uint8_t is_current_plmn)
{
    char tags[MAX_TAG_SIZE];
    uint16_t mcc = 0;
    uint16_t mnc = 0;

    if(NULL != mcc_ptr)
    {
        mcc = atoi(mcc_ptr);
    }

    if(NULL != mnc_ptr)
    {
        mnc = atoi(mnc_ptr);
    }

    t.set_start();

    // Inject: RIL_REQUEST_OPERATOR
    RIL_node.update_default_ril_request_operator();
    t.inject(RIL_node);

    if(FALSE == is_current_plmn)
    {
        // Expect: QMI_NAS_GET_PLMN_NAME_REQ_MSG_V01
        QMI_node.update_default_qmi_nas_get_plmn_name_req_msg(mcc, mnc);
        QMI_node.set_qmi_nas_get_plmn_name_req_suppress_sim_error(TRUE);
        QMI_node.set_qmi_nas_get_plmn_name_req_rat(rat);
        memset(tags,'\0', MAX_TAG_SIZE);
        if(csg_id_valid)
        {
            QMI_node.set_qmi_nas_get_plmn_name_req_csg_id(csg_id);
        }
        snprintf(tags, MAX_TAG_SIZE, "MCC = %s, MNC = %s, RAT = %d, CSG_ID_VALID = %d",
                mcc_ptr, mnc_ptr, rat, csg_id_valid);
        QMI_node.add_callflow_tag(tags);
        t.expect(QMI_node);

        // Inject: QMI_NAS_GET_PLMN_NAME_RESP_MSG_V01
        QMI_node.update_default_qmi_nas_plmn_name_resp_msg(QMI_RESULT_SUCCESS_V01, (qmi_error_type_v01)0);
        memset(tags,'\0', MAX_TAG_SIZE);
        if(is_default_oprtr_name)
        {
            nas_3gpp_eons_plmn_name_type_v01* eons_plmn_name = QMI_node.set_qmi_nas_get_plmn_name_resp_eons_plmn_name_3gpp();
            if( (NULL != long_plmn_name) && (!is_static_table) )
            {
                eons_plmn_name->plmn_long_name_len = strlen(long_plmn_name);
                strcpy(eons_plmn_name->plmn_long_name, long_plmn_name);
                strcat(tags, "Long PLMN = ");
                strcat(tags, long_plmn_name);
            }

            if( (NULL != short_plmn_name) && (!is_static_table) )
            {
                eons_plmn_name->plmn_short_name_len = strlen(short_plmn_name);
                strcpy(eons_plmn_name->plmn_short_name, short_plmn_name);
                strcat(tags, "Short PLMN = ");
                strcat(tags, short_plmn_name);
            }
        }
        else
        {
            snprintf(tags, MAX_TAG_SIZE, "is_default_oprtr_name = %d",is_default_oprtr_name);
        }
        QMI_node.add_callflow_tag(tags);
        t.inject(QMI_node);

        if(!is_default_oprtr_name)
        {
            // Expect: QMI_NAS_GET_PLMN_NAME_REQ_MSG_V01
            QMI_node.update_default_qmi_nas_get_plmn_name_req_msg(mcc, mnc);
            QMI_node.set_qmi_nas_get_plmn_name_req_suppress_sim_error(TRUE);
            QMI_node.set_qmi_nas_get_plmn_name_req_always_send_plmn_name(TRUE);
            QMI_node.set_qmi_nas_get_plmn_name_req_rat(rat);
            if(csg_id_valid)
            {
                QMI_node.set_qmi_nas_get_plmn_name_req_csg_id(csg_id);
            }
            memset(tags,'\0', MAX_TAG_SIZE);
            snprintf(tags, MAX_TAG_SIZE, "MCC = %s, MNC = %s, RAT = %d, CSG_ID_VALID = %d",
                    mcc_ptr, mnc_ptr, rat, csg_id_valid);
            QMI_node.add_callflow_tag(tags);
            t.expect(QMI_node);

            // Inject: QMI_NAS_GET_PLMN_NAME_RESP_MSG_V01
            QMI_node.update_default_qmi_nas_plmn_name_resp_msg(QMI_RESULT_SUCCESS_V01, (qmi_error_type_v01)0);
            memset(tags,'\0', MAX_TAG_SIZE);
            if(is_always_send_plmn_name)
            {
                nas_3gpp_eons_plmn_name_type_v01* eons_plmn_name = QMI_node.set_qmi_nas_get_plmn_name_resp_eons_plmn_name_3gpp();
                if( (NULL != long_plmn_name) && (!is_static_table) )
                {
                    eons_plmn_name->plmn_long_name_len = strlen(long_plmn_name);
                    strcpy(eons_plmn_name->plmn_long_name, long_plmn_name);
                    strcat(tags, "Long PLMN = ");
                    strcat(tags, long_plmn_name);
                }

                if( (NULL != short_plmn_name) && (!is_static_table) )
                {
                    eons_plmn_name->plmn_short_name_len = strlen(short_plmn_name);
                    strcpy(eons_plmn_name->plmn_short_name, short_plmn_name);
                    strcat(tags, "Short PLMN = ");
                    strcat(tags, short_plmn_name);
                }
            }
            else
            {
                snprintf(tags, MAX_TAG_SIZE, "is_always_send_plmn_name = %d", is_always_send_plmn_name);
            }
            QMI_node.add_callflow_tag(tags);
            t.inject(QMI_node);

            if(!is_always_send_plmn_name)
            {
                // Expect: QMI_NAS_GET_PLMN_NAME_REQ_MSG_V01
                QMI_node.update_default_qmi_nas_get_plmn_name_req_msg(mcc, mnc);
                QMI_node.set_qmi_nas_get_plmn_name_req_suppress_sim_error(TRUE);
                QMI_node.set_qmi_nas_get_plmn_name_req_use_static_table_only(TRUE);
                QMI_node.set_qmi_nas_get_plmn_name_req_rat(rat);
                if(csg_id_valid)
                {
                    QMI_node.set_qmi_nas_get_plmn_name_req_csg_id(csg_id);
                }
                memset(tags,'\0', MAX_TAG_SIZE);
                snprintf(tags, MAX_TAG_SIZE, "MCC = %s, MNC = %s, RAT = %d, CSG_ID_VALID = %d",
                        mcc_ptr, mnc_ptr, rat, csg_id_valid);
                QMI_node.add_callflow_tag(tags);
                t.expect(QMI_node);

                // Inject: QMI_NAS_GET_PLMN_NAME_RESP_MSG_V01
                QMI_node.update_default_qmi_nas_plmn_name_resp_msg(QMI_RESULT_SUCCESS_V01, (qmi_error_type_v01)0);
                memset(tags,'\0', MAX_TAG_SIZE);
                if(is_se_table)
                {
                    nas_3gpp_eons_plmn_name_type_v01* eons_plmn_name = QMI_node.set_qmi_nas_get_plmn_name_resp_eons_plmn_name_3gpp();
                    if( (NULL != long_plmn_name) && (!is_static_table) )
                    {
                        eons_plmn_name->plmn_long_name_len = strlen(long_plmn_name);
                        strcpy(eons_plmn_name->plmn_long_name, long_plmn_name);
                        strcat(tags, "Long PLMN = ");
                        strcat(tags, long_plmn_name);
                    }

                    if( (NULL != short_plmn_name) && (!is_static_table) )
                    {
                        eons_plmn_name->plmn_short_name_len = strlen(short_plmn_name);
                        strcpy(eons_plmn_name->plmn_short_name, short_plmn_name);
                        strcat(tags, "Short PLMN = ");
                        strcat(tags, short_plmn_name);
                    }
                }
                else
                {
                    snprintf(tags, MAX_TAG_SIZE, "is_se_table = %d",is_se_table);
                }
                QMI_node.add_callflow_tag(tags);
                t.inject(QMI_node);
            }
        }
    }

    // Expect: RIL_REQUEST_OPERATOR_RESP
    ril_request_operator_resp_from_qcril_t* ril_req_operator_name = RIL_node.update_default_ril_request_operator_resp();
    if(NULL != long_plmn_name)
    {
        strcpy(ril_req_operator_name->operator_info_array[0],long_plmn_name);
    }
    else if( (NULL != short_plmn_name) && (is_fill_eons_needed) )
    {
        strcpy(ril_req_operator_name->operator_info_array[0],short_plmn_name);
    }
    else if(is_static_table)
    {
        snprintf(ril_req_operator_name->operator_info_array[0],
                NAS_OPERATOR_RESP_MAX_EONS_LEN, "%d%d", mcc, mnc);
    }

    if(NULL != short_plmn_name)
    {
        strcpy(ril_req_operator_name->operator_info_array[1],short_plmn_name);
    }
    else if ( (NULL != long_plmn_name) && (is_fill_eons_needed) )
    {
        strcpy(ril_req_operator_name->operator_info_array[1],ril_req_operator_name->operator_info_array[0]);
    }
    else if(is_static_table && is_fill_eons_needed)
    {
        snprintf(ril_req_operator_name->operator_info_array[1],
                NAS_OPERATOR_RESP_MAX_EONS_LEN, "%d%d", mcc, mnc);
    }

    snprintf( ril_req_operator_name->operator_info_array[2],
            NAS_OPERATOR_RESP_MAX_MCC_MNC_LEN, "%s%s", mcc_ptr, mnc_ptr );
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "long_name = %s, short_name = %s, mccmnc = %s",
            ril_req_operator_name->operator_info_array[0],
            ril_req_operator_name->operator_info_array[1],
            ril_req_operator_name->operator_info_array[2]);
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);

    t.set_end();
}

void util_ril_utf_wcdma_signal_strength (
    Testcase &t,
    AndroidTelTestnode &RIL_node,
    QMIRILTestnode &QMI_node,
    int8_t rssi,
    int16_t ecio,
    uint8_t is_unsol,
    uint8_t is_default
)
{
    int signal_strength=0;
    char tags[MAX_TAG_SIZE];

    conv_qmi_rssi_to_gw_signal_strength((rssi)*(-1), &signal_strength);

    t.set_start();

    // Inject: QMI_NAS_SIG_INFO_IND_MSG_V01
    QMI_node.update_default_qmi_nas_sig_info_ind_msg();
    QMI_node.set_qmi_nas_sig_info_ind_wcdma_sig_info(rssi, ecio);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "WCDMA: RSSI = %d, ECIO = %d", rssi, ecio);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    if(TRUE == is_unsol)
    {
        // Expect: RIL_UNSOL_SIGNAL_STRENGTH
        RIL_node.update_default_ril_unsol_signal_strength();
        memset(tags,'\0', MAX_TAG_SIZE);
        if(TRUE == is_default)
        {
            snprintf(tags, MAX_TAG_SIZE, "WCDMA: Default Signal Strength = %d",0x63);
            RIL_node.set_ril_unsol_signal_strength_gw_signal_strength(0x63, 0);
        }
        else
        {
            RIL_node.set_ril_unsol_signal_strength_gw_signal_strength(signal_strength, 0);
            snprintf(tags, MAX_TAG_SIZE, "WCDMA: Correct Signal Strength = %d",signal_strength);
        }
        RIL_node.add_callflow_tag(tags);
        RIL_node.set_ril_unsol_signal_strength_cdma_signal_strength(-1, -1);
        RIL_node.set_ril_unsol_signal_strength_evdo_signal_strength(-1, -1, -1);
        RIL_node.set_ril_unsol_signal_strength_lte_signal_strength(0x63, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF);
        RIL_node.set_ril_unsol_signal_strength_td_scdma_signal_strength(0x7FFFFFFF);
        RIL_node.set_expect_skip_verification_status(false);
        t.expect(RIL_node);
    }

    t.set_end();
}

void util_ril_utf_gsm_signal_strength (
    Testcase &t,
    AndroidTelTestnode &RIL_node,
    QMIRILTestnode &QMI_node,
    int8_t rssi,
    uint8_t is_unsol,
    uint8_t is_err_rate,
    uint8_t is_default
)
{
    int signal_strength=0;
    char tags[MAX_TAG_SIZE];

    conv_qmi_rssi_to_gw_signal_strength((rssi)*(-1), &signal_strength);

    t.set_start();

    // Inject: QMI_NAS_SIG_INFO_IND_MSG_V01
    QMI_node.update_default_qmi_nas_sig_info_ind_msg();
    QMI_node.set_qmi_nas_sig_info_ind_gsm_sig_info(rssi);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "GSM: RSSI = %d",rssi);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    if((TRUE == is_err_rate) && (TRUE == is_unsol))
    {
        // Expect: QMI_NAS_GET_ERR_RATE_REQ_MSG_V01
        QMI_node.update_default_qmi_nas_get_err_rate_req_msg_v01();
        t.expect(QMI_node);

        // Inject: QMI_NAS_GET_ERR_RATE_RESP_MSG_V01
        QMI_node.update_default_qmi_nas_get_err_rate_resp_msg_v01();
        QMI_node.set_qmi_nas_get_err_rate_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
        t.inject(QMI_node);
    }

    if(TRUE == is_unsol)
    {
        // Expect: RIL_UNSOL_SIGNAL_STRENGTH
        RIL_node.update_default_ril_unsol_signal_strength();
        memset(tags,'\0', MAX_TAG_SIZE);
        if(TRUE == is_default)
        {
            snprintf(tags, MAX_TAG_SIZE, "GSM: Default Signal Strength = %d", 0x63);
            RIL_node.set_ril_unsol_signal_strength_gw_signal_strength(0x63, 0);
        }
        else
        {
            snprintf(tags, MAX_TAG_SIZE, "GSM: Correct Signal Strength = %d", signal_strength);
            RIL_node.set_ril_unsol_signal_strength_gw_signal_strength(signal_strength, 0);
        }
        RIL_node.add_callflow_tag(tags);
        RIL_node.set_ril_unsol_signal_strength_cdma_signal_strength(-1, -1);
        RIL_node.set_ril_unsol_signal_strength_evdo_signal_strength(-1, -1, -1);
        RIL_node.set_ril_unsol_signal_strength_lte_signal_strength(0x63, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF);
        RIL_node.set_ril_unsol_signal_strength_td_scdma_signal_strength(0x7FFFFFFF);
        RIL_node.set_expect_skip_verification_status(false);
        t.expect(RIL_node);
    }

    t.set_end();
}

void util_ril_utf_tdscdma_signal_strength (
    Testcase &t,
    AndroidTelTestnode &RIL_node,
    QMIRILTestnode &QMI_node,
    int8_t rscp,
    uint8_t is_unsol,
    uint8_t is_err_rate,
    uint8_t is_default
)
{
    uint16 signal_strength=0;
    char tags[MAX_TAG_SIZE];

    signal_strength = (rscp)*(-1);

    t.set_start();

    // Inject: QMI_NAS_SIG_INFO_IND_MSG_V01
    QMI_node.update_default_qmi_nas_sig_info_ind_msg();
    QMI_node.set_qmi_nas_sig_info_ind_rscp(rscp);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "TDSCDMA: RSCP = %d", rscp);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    if((TRUE == is_err_rate) && (TRUE == is_unsol))
    {
        // Expect: QMI_NAS_GET_ERR_RATE_REQ_MSG_V01
        QMI_node.update_default_qmi_nas_get_err_rate_req_msg_v01();
        t.expect(QMI_node);

        // Inject: QMI_NAS_GET_ERR_RATE_RESP_MSG_V01
        QMI_node.update_default_qmi_nas_get_err_rate_resp_msg_v01();
        QMI_node.set_qmi_nas_get_err_rate_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
        t.inject(QMI_node);
    }

    if(TRUE == is_unsol)
    {
        // Expect: RIL_UNSOL_SIGNAL_STRENGTH
        RIL_node.update_default_ril_unsol_signal_strength();
        memset(tags,'\0', MAX_TAG_SIZE);
        RIL_node.set_ril_unsol_signal_strength_gw_signal_strength(0x63, 0);
        RIL_node.set_ril_unsol_signal_strength_cdma_signal_strength(-1, -1);
        RIL_node.set_ril_unsol_signal_strength_evdo_signal_strength(-1, -1, -1);
        RIL_node.set_ril_unsol_signal_strength_lte_signal_strength(0x63, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF);
        if(TRUE == is_default)
        {
            RIL_node.set_ril_unsol_signal_strength_td_scdma_signal_strength(0x7FFFFFFF);
            snprintf(tags, MAX_TAG_SIZE, "TDSCDMA: Default Signal Strength = %ld", 0x7FFFFFFFL);
        }
        else
        {
            RIL_node.set_ril_unsol_signal_strength_td_scdma_signal_strength(signal_strength);
            snprintf(tags, MAX_TAG_SIZE, "TDSCDMA: Correct Signal Strength = %ld",
                    (long)signal_strength);
        }
        RIL_node.set_expect_skip_verification_status(false);
        RIL_node.add_callflow_tag(tags);
        t.expect(RIL_node);
    }

    t.set_end();
}

void util_ril_utf_lte_signal_strength (
    Testcase &t,
    AndroidTelTestnode &RIL_node,
    QMIRILTestnode &QMI_node,
    int8_t rssi,
    int8_t rsrq,
    int16_t rsrp,
    int16_t snr,
    uint8_t is_unsol,
    uint8_t is_default
)
{
    int ril_rssnr=0;
    int ril_rsrp=0;
    int ril_rsrq=0;
    int signal_strength=0;
    char tags[MAX_TAG_SIZE];

    conv_qmi_rssi_to_gw_signal_strength((rssi)*(-1), &signal_strength);
    ril_rssnr = snr;
    ril_rsrp = (rsrp)*(-1);
    ril_rsrq = (rsrq)*(-1);

    t.set_start();

    // Inject: QMI_NAS_SIG_INFO_IND_MSG_V01
    QMI_node.update_default_qmi_nas_sig_info_ind_msg();
    QMI_node.set_qmi_nas_sig_info_ind_lte_sig_info(rssi, rsrq, rsrp, snr);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "LTE: RSSI = %d, RSRQ = %d, RSRP = %d, SNR = %d",
            rssi, rsrq, rsrp, snr);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    if(TRUE == is_unsol)
    {
        // Expect: RIL_UNSOL_SIGNAL_STRENGTH
        RIL_node.update_default_ril_unsol_signal_strength();
        memset(tags,'\0', MAX_TAG_SIZE);
        RIL_node.set_ril_unsol_signal_strength_gw_signal_strength(0x63, 0);
        RIL_node.set_ril_unsol_signal_strength_cdma_signal_strength(-1,-1);
        RIL_node.set_ril_unsol_signal_strength_evdo_signal_strength(-1, -1, -1);
        if(TRUE == is_default)
        {
            snprintf(tags, MAX_TAG_SIZE, "LTE: Default Signal Strength %d", 0x63);
            RIL_node.set_ril_unsol_signal_strength_lte_signal_strength(0x63, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF);
        }
        else
        {
            snprintf(tags, MAX_TAG_SIZE, "LTE: Correct Signal Strength = %d", signal_strength);
            RIL_node.set_ril_unsol_signal_strength_lte_signal_strength(signal_strength, ril_rsrp, ril_rsrq, ril_rssnr, 0x7FFFFFFF, 0x7FFFFFFF);
        }
        RIL_node.add_callflow_tag(tags);
        RIL_node.set_ril_unsol_signal_strength_td_scdma_signal_strength(0x7FFFFFFF);
        RIL_node.set_expect_skip_verification_status(false);
        t.expect(RIL_node);
    }

    t.set_end();
}

void util_ril_utf_cdma_signal_strength (
    Testcase &t,
    AndroidTelTestnode &RIL_node,
    QMIRILTestnode &QMI_node,
    int8_t rssi,
    int16_t ecio,
    uint8_t is_unsol,
    uint8_t is_default
)
{
    int ril_rssi=0;
    int ril_ecio=0;
    char tags[MAX_TAG_SIZE];

    ril_rssi = (rssi)*(-1);
    ril_ecio = (ecio)*(5);

    t.set_start();

    // Inject: QMI_NAS_SIG_INFO_IND_MSG_V01
    QMI_node.update_default_qmi_nas_sig_info_ind_msg();
    QMI_node.set_qmi_nas_sig_info_ind_cdma_sig_info(rssi,ecio);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "CDMA: RSSI = %d, ECIO = %d", rssi, ecio);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    if(TRUE == is_unsol)
    {
        // Expect: RIL_UNSOL_SIGNAL_STRENGTH
        RIL_node.update_default_ril_unsol_signal_strength();
        memset(tags,'\0', MAX_TAG_SIZE);
        RIL_node.set_ril_unsol_signal_strength_gw_signal_strength(0x63, 0);
        if(TRUE == is_default)
        {
            snprintf(tags, MAX_TAG_SIZE, "CDMA: Defualt Signal Strength RSSI = -1, ECIO = -1");
            RIL_node.set_ril_unsol_signal_strength_cdma_signal_strength(-1,-1);
        }
        else
        {
            snprintf(tags, MAX_TAG_SIZE, "CDMA: Correct Signal Strength RSSI = %d, ECIO = %d",
                    ril_rssi, ril_ecio);
            RIL_node.set_ril_unsol_signal_strength_cdma_signal_strength(ril_rssi, ril_ecio);
        }
        RIL_node.add_callflow_tag(tags);
        RIL_node.set_ril_unsol_signal_strength_evdo_signal_strength(-1, -1, -1);
        RIL_node.set_ril_unsol_signal_strength_lte_signal_strength(0x63, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF);
        RIL_node.set_ril_unsol_signal_strength_td_scdma_signal_strength(0x7FFFFFFF);
        RIL_node.set_expect_skip_verification_status(false);
        t.expect(RIL_node);
    }

    t.set_end();
}

void conv_qmi_rssi_to_gw_signal_strength
(
  uint16 rssi,
  int *signal_strength_ptr
)
{
  if(signal_strength_ptr != NULL)
  {
    if ( ( QCRIL_CM_RSSI_MIN < rssi ) && ( QCRIL_CM_RSSI_MAX > rssi ) )
    {
      *signal_strength_ptr = (int) ( floor( ( ( rssi * QCRIL_CM_RSSI_SLOPE + QCRIL_CM_RSSI_OFFSET ) *
                                            QCRIL_CM_RSSI_TOOHI_CODE ) / 100 + 0.5 ) );
    }
    else if ( ( QCRIL_CM_RSSI_MAX <= rssi ) && ( QCRIL_CM_RSSI_NO_SIGNAL != rssi ) )
    {
      *signal_strength_ptr = QCRIL_CM_RSSI_TOOLO_CODE;
    }
    else if ( QCRIL_CM_RSSI_MIN >= rssi )
    {
      *signal_strength_ptr = QCRIL_CM_RSSI_TOOHI_CODE;
    }
    else
    {
      *signal_strength_ptr = QCRIL_CM_GW_SIGNAL_STRENGTH_UNKNOWN;
    }
  }
}

void util_ril_utf_set_multisim_prop (
    Testcase &t,
    AndroidTelTestnode &RIL_node,
    QMIRILTestnode &QMI_node,
    uint8_t max_subscriptions,
    uint32_t subs_config_list_len,
    uint8_t max_active_dsds,
    uint8_t max_active_dsda
)
{
    char tags[MAX_TAG_SIZE];
    NOTUSED(QMI_node);
    NOTUSED(RIL_node);
    t.set_start();

    // Inject: QMI_DMS_EVENT_REPORT_IND_V01
    QMI_node.update_default_qmi_dms_event_report_ind_v01();
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "Max_Sub = %d, Active_Sub=%d i.e. DSDS",
            max_subscriptions, max_active_dsds);
    QMI_node.add_callflow_tag(tags);
    QMI_node.set_qmi_dms_event_report_ind_v01_multisim_capability(max_subscriptions, subs_config_list_len, max_active_dsds);
    t.inject(QMI_node);

    // Inject: QMI_DMS_EVENT_REPORT_IND_V01
    QMI_node.update_default_qmi_dms_event_report_ind_v01();
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "Max_Sub = %d, Active_Sub=%d i.e DSDS",
            max_subscriptions, max_active_dsds);
    QMI_node.add_callflow_tag(tags);
    QMI_node.set_qmi_dms_event_report_ind_v01_multisim_capability(max_subscriptions, subs_config_list_len, max_active_dsds);
    t.inject(QMI_node);

    // Inject: QMI_DMS_EVENT_REPORT_IND_V01
    QMI_node.update_default_qmi_dms_event_report_ind_v01();
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "Max_Sub = %d, Active_Sub=%d i.e. DSDA",
            max_subscriptions, max_active_dsda);
    QMI_node.add_callflow_tag(tags);
    QMI_node.set_qmi_dms_event_report_ind_v01_multisim_capability(max_subscriptions, subs_config_list_len, max_active_dsda);
    t.inject(QMI_node);

    // Inject: QMI_DMS_EVENT_REPORT_IND_V01
    QMI_node.update_default_qmi_dms_event_report_ind_v01();
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "Max_Sub = %d, Active_Sub=%d i.e. DSDA",
            max_subscriptions, max_active_dsda);
    QMI_node.add_callflow_tag(tags);
    QMI_node.set_qmi_dms_event_report_ind_v01_multisim_capability(max_subscriptions, subs_config_list_len, max_active_dsda);
    t.inject(QMI_node);

    // Inject: QMI_DMS_EVENT_REPORT_IND_V01
    QMI_node.update_default_qmi_dms_event_report_ind_v01();
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "Max_Sub = %d, Active_Sub=%d i.e. DSDS",
            max_subscriptions, max_active_dsds);
    QMI_node.add_callflow_tag(tags);
    QMI_node.set_qmi_dms_event_report_ind_v01_multisim_capability(max_subscriptions, subs_config_list_len, max_active_dsds);
    t.inject(QMI_node);

    // Inject: QMI_DMS_EVENT_REPORT_IND_V01
    QMI_node.update_default_qmi_dms_event_report_ind_v01();
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "Max_Sub = %d, Active_Sub=%d i.e. DSDS",
            max_subscriptions, max_active_dsds);
    QMI_node.add_callflow_tag(tags);
    QMI_node.set_qmi_dms_event_report_ind_v01_multisim_capability(max_subscriptions, subs_config_list_len, max_active_dsds);
    t.inject(QMI_node);

    t.set_end();
}

void util_ril_utf_set_uicc_subs ( Testcase &t, AndroidTelTestnode &RIL_node, QMIRILTestnode &QMI_node, int app_index, int slot,
RIL_UiccSubActStatus act_status, RIL_SubscriptionType sub_type, uint8_t is_dsda2, uint8_t is_subs_info, int res, qmi_result_type_v01 result, qmi_error_type_v01 error)
{
    char tags[MAX_TAG_SIZE];
    t.set_start();

    // Inject: RIL_REQUEST_SET_UICC_SUBSCRIPTION
    RIL_SelectUiccSub* uicc_subs_data = RIL_node.update_default_ril_request_set_uicc_subscription();
    RIL_node.set_ril_request_set_uicc_subscription_app_index(app_index);
    RIL_node.set_ril_request_set_uicc_subscription_slot(slot);
    RIL_node.set_ril_request_set_uicc_subscription_act_status(act_status);
    RIL_node.set_ril_request_set_uicc_subscription_sub_type(sub_type);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "app_index = %d, slot = %d, act_status = %d, "
                                "sub_type = %d", app_index, slot, act_status,
                                sub_type);
    RIL_node.add_callflow_tag(tags);
    t.inject(RIL_node);

    if(FALSE == is_dsda2)
    {
        //Expect: QMI_NAS_GET_MODE_PREF_REQ_MSG_V01
        QMI_node.update_default_qmi_nas_get_mode_pref_req();
        t.expect(QMI_node);

        // Inject: QMI_NAS_GET_MODE_PREF_RESP_MSG_V01
        QMI_node.update_default_qmi_nas_get_mode_pref_resp_msg(result, error);
        QMI_node.set_qmi_nas_get_mode_pref_resp_idx0_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM);
        QMI_node.set_qmi_nas_get_mode_pref_resp_idx1_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM);
        QMI_node.set_qmi_nas_get_mode_pref_resp_idx2_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM);
        memset(tags,'\0', MAX_TAG_SIZE);
        snprintf(tags, MAX_TAG_SIZE, "GET_MODE_PREF_RESP = %d, GET_MODE_PREF_ERROR = %d, "
                                    "IDX0_Mode_Pref = %d", result, error,
                                    QMI_NAS_RAT_MODE_PREF_GSM);
        QMI_node.add_callflow_tag(tags);
        t.inject(QMI_node);
    }

    if(FALSE == is_subs_info)
    {
        // Expect: RIL_REQUEST_SET_UICC_SUBSCRIPTION_RESP
        RIL_node.update_default_ril_request_set_uicc_subscription_resp();
        RIL_node.set_ril_error_value((RIL_Errno)res);
        memset(tags,'\0', MAX_TAG_SIZE);
        snprintf(tags, MAX_TAG_SIZE, "RIL_REQUEST_SET_UICC_SUBSCRIPTION_RESP = %d", res);
        RIL_node.add_callflow_tag(tags);
        t.expect(RIL_node);
    }

    if(TRUE == is_subs_info)
    {
        //Inject: QMI_NAS_SUBSCRIPTION_INFO_IND_MSG_V01
        QMI_node.update_default_qmi_nas_subscription_info_ind_msg();
        QMI_node.set_qmi_nas_subscription_info_ind_is_priority_subs((nas_is_priority_subs_enum_v01)0);
        QMI_node.set_qmi_nas_subscription_info_ind_is_active((nas_active_subs_info_enum_v01)1);
        memset(tags,'\0', MAX_TAG_SIZE);
        snprintf(tags, MAX_TAG_SIZE, "Priority Sub = %d, Active = %d", 0, 1);
        QMI_node.add_callflow_tag(tags);
        t.inject(QMI_node);
    }

    t.set_end();
}

void util_wms_service_ready(Testcase &t, AndroidTelTestnode &RIL_node, OEMTestnode &OEM_node, QMIRILTestnode &QMI_node, wms_service_ready_status_enum_v01 wms_tech)
{
  char tags[MAX_TAG_SIZE];
  t.set_start();

  // INJECT: QMI_WMS_SERVICE_READY_IND_V01
  QMI_node.update_default_qmi_wms_service_ready_ind(wms_tech);
  memset(tags,'\0', MAX_TAG_SIZE);
  snprintf(tags, MAX_TAG_SIZE, "WMS Service Available on tech %d",wms_tech);
  QMI_node.add_callflow_tag(tags);
  t.inject(QMI_node);

  // Expect: QCRIL_EVT_HOOK_UNSOL_WMS_READY
  OEM_node.update_default_qcril_evt_hook_unsol_wms_ready();
  OEM_node.set_expect_skip_verification_status(false);
  t.expect(OEM_node);

  // Expect: RIL_UNSOL_OEM_HOOK_RAW
  RIL_node.update_default_ril_unsol_oem_hook_raw();
  RIL_node.set_expect_skip_verification_status(false);
  t.expect(RIL_node);

  t.set_end();
}

void util_sms_over_ims_reg_state(Testcase &t, AndroidTelTestnode &RIL_node, QMIRILTestnode &QMI_node, uint8_t registered_ind, qmi_result_type_v01 nw_result, qmi_error_type_v01 nw_error, uint8_t nw_reg_status_valid, wms_transport_nw_reg_status_enum_v01 nw_reg_status, uint8_t is_transport_layer_info_needed, qmi_result_type_v01 layer_result, qmi_error_type_v01 layer_error, uint8_t layer_registered_ind_valid, uint8_t layer_registered_ind, uint8_t transport_layer_info_valid, wms_transport_type_enum_v01 transport_type, wms_transport_capability_enum_v01 transport_cap, uint8_t is_empty, int reg_status, int rat, int atel_status)
{
  IMSTestnode IMS_node;
  char tags[MAX_TAG_SIZE];
  t.set_start();

  // INJECT: QMI_WMS_TRANSPORT_LAYER_INFO_IND
  QMI_node.update_default_qmi_wms_transport_layer_info_ind_msg(registered_ind);
  memset(tags,'\0', MAX_TAG_SIZE);
  snprintf(tags, MAX_TAG_SIZE, "Is IMS Registered = %d", registered_ind);
  QMI_node.add_callflow_tag(tags);
  t.inject(QMI_node);

  // Expect: RIL_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED
  RIL_node.update_default_ril_unsol_resp_ims_network_state_changed();
  t.expect(RIL_node);

// Expect: QMI_WMS_GET_TRANSPORT_NW_REG_INFO_REQ_V01
  QMI_node.update_default_qmi_wms_get_transport_nw_reg_info_req();
  t.expect(QMI_node);

  // Inject: QMI_WMS_GET_TRANSPORT_NW_REG_INFO_RESP_V01
  QMI_node.update_default_qmi_wms_get_transport_nw_reg_info_resp(nw_result, nw_error);
  QMI_node.set_qmi_wms_get_transport_nw_reg_status(nw_reg_status_valid, nw_reg_status);
  memset(tags,'\0', MAX_TAG_SIZE);
  snprintf(tags, MAX_TAG_SIZE, "NW Result = %d, NW Error = %d, NW Reg Status Valid = %d, "
                              "NW Reg Status = %d",nw_result, nw_error,
                              nw_reg_status_valid, nw_reg_status);
  QMI_node.add_callflow_tag(tags);
  t.inject(QMI_node);

  if(TRUE == is_transport_layer_info_needed)
  {
    // Expect: QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ_V01
    QMI_node.update_default_qmi_wms_get_transport_layer_info_req();
    t.expect(QMI_node);

    // Inject: QMI_WMS_GET_TRANSPORT_LAYER_INFO_RESP_V01
    QMI_node.update_default_qmi_wms_get_transport_layer_info_resp(layer_result, layer_error);
    QMI_node.set_qmi_wms_get_transport_layer_register_ind_info(layer_registered_ind_valid, layer_registered_ind);
    QMI_node.set_qmi_wms_get_transport_layer_info_type_cap(transport_layer_info_valid, transport_type, transport_cap);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "Layer Result = %d, Layer Error = %d, "
            "Layer Reg IND Valid = %d, Layer Reg IND = %d, Layer Info Valid = %d, "
            "Transport Type = %d, Transport Capability = %d", layer_result,
            layer_error, layer_registered_ind_valid, layer_registered_ind,
            transport_layer_info_valid, transport_type, transport_cap);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    if(QMI_RESULT_SUCCESS_V01 == layer_result)
    {
        // Expect: ims_MsgId_UNSOL_SRV_STATUS_UPDATE
        IMS_node.update_default_ims_unsol_srv_status_update(6);
        IMS_node.set_expect_skip_verification_status(false);
        t.expect(IMS_node);
    }
  }
  else if(nw_reg_status_valid)
  {
    // Expect: ims_MsgId_UNSOL_SRV_STATUS_UPDATE
    IMS_node.update_default_ims_unsol_srv_status_update(6);
    IMS_node.set_expect_skip_verification_status(false);
    t.expect(IMS_node);
  }

  // INJECT: RIL_REQUEST_IMS_REGISTRATION_STATE
  RIL_node.update_default_ril_request_ims_reg_state();
  t.inject(RIL_node);

  // Expect: QMI_WMS_GET_TRANSPORT_NW_REG_INFO_REQ_V01
  QMI_node.update_default_qmi_wms_get_transport_nw_reg_info_req();
  t.expect(QMI_node);

  // Inject: QMI_WMS_GET_TRANSPORT_NW_REG_INFO_RESP_V01
  QMI_node.update_default_qmi_wms_get_transport_nw_reg_info_resp(nw_result, nw_error);
  QMI_node.set_qmi_wms_get_transport_nw_reg_status(nw_reg_status_valid, nw_reg_status);
  memset(tags,'\0', MAX_TAG_SIZE);
  snprintf(tags, MAX_TAG_SIZE, "NW Result = %d, NW Error = %d, NW Reg Status Valid = %d, "
                              "NW Reg Status = %d", nw_result, nw_error,
                              nw_reg_status_valid, nw_reg_status);
  QMI_node.add_callflow_tag(tags);
  t.inject(QMI_node);

  if(TRUE == is_transport_layer_info_needed)
  {
    // Expect: QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ_V01
    QMI_node.update_default_qmi_wms_get_transport_layer_info_req();
    t.expect(QMI_node);

    // Inject: QMI_WMS_GET_TRANSPORT_LAYER_INFO_RESP_V01
    QMI_node.update_default_qmi_wms_get_transport_layer_info_resp(layer_result, layer_error);
    QMI_node.set_qmi_wms_get_transport_layer_register_ind_info(layer_registered_ind_valid, layer_registered_ind);
    QMI_node.set_qmi_wms_get_transport_layer_info_type_cap(transport_layer_info_valid, transport_type, transport_cap);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "Layer Result = %d, Layer Error = %d, "
            "Layer Reg IND Valid = %d, Layer Reg IND = %d, Layer Info Valid = %d, "
            "Transport Type = %d, Transport Capability = %d",layer_result,
            layer_error, layer_registered_ind_valid, layer_registered_ind,
            transport_layer_info_valid, transport_type, transport_cap);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);
  }

  // Expect: RIL_REQUEST_IMS_REGISTRATION_STATE_RESP
  RIL_node.update_default_ril_request_ims_reg_state_resp(is_empty, reg_status, rat);
  RIL_node.set_ril_error_value((RIL_Errno)atel_status);
  memset(tags,'\0', MAX_TAG_SIZE);
  if(is_empty)
  {
    snprintf(tags, MAX_TAG_SIZE, "ATEL Result = %d, With Empty Payload", atel_status);
  }
  else
  {
    snprintf(tags, MAX_TAG_SIZE, "ATEL Result = %d, Reg Status = %d, RAT = %d",
            atel_status, reg_status, rat);
  }
  RIL_node.add_callflow_tag(tags);
  t.expect(RIL_node);

  t.set_end();
}

void util_sms_over_ims_send_sms_3gpp(Testcase &t, AndroidTelTestnode &RIL_node, QMIRILTestnode &QMI_node, unsigned char retry, int messageRef, int sms_over_ims, int atel_status, qmi_result_type_v01 result, qmi_error_type_v01 error, uint16_t msgRef, uint8_t is_delivery_failure, wms_message_delivery_failure_type_enum_v01 message_delivery_failure_type, uint8_t is_retry_wms_available)
{
  char tags[MAX_TAG_SIZE];
  t.set_start();

  // Inject: RIL_REQUEST_IMS_SEND_SMS
  RIL_node.update_default_ril_request_ims_send_sms(FALSE);
  ims_send_sms_3gpp_payload* ims_send_sms = RIL_node.set_ril_request_ims_send_sms_3gpp_tech_retry_msgref(RADIO_TECH_3GPP, retry, messageRef);
  strlcpy(ims_send_sms->gsmMessage[0], "8012452313", strlen("8012452313")+1);
  strlcpy(ims_send_sms->gsmMessage[1], "sometext", strlen("sometext")+1);
  memset(tags,'\0', MAX_TAG_SIZE);
  snprintf(tags, MAX_TAG_SIZE, "Tech = %d, Retry = %d", RADIO_TECH_3GPP, retry);
  RIL_node.add_callflow_tag(tags);
  t.inject(RIL_node);

  if(is_retry_wms_available)
  {
    // Expect: QMI_WMS_RAW_SEND_REQ_V01
    QMI_node.update_default_qmi_wms_raw_send_req_msg((char *)"8012452313", (char *)"sometext", WMS_MESSAGE_FORMAT_GW_PP_V01);
    QMI_node.set_qmi_wms_raw_send_req_sms_on_ims(sms_over_ims);
    if(retry)
    {
      QMI_node.set_qmi_wms_raw_send_req_retry_message(WMS_MESSAGE_IS_A_RETRY_V01);
      QMI_node.set_qmi_wms_raw_send_req_retry_message_id(messageRef);
    }
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "sms_over_ims = %d", sms_over_ims);
    QMI_node.add_callflow_tag(tags);
    t.expect(QMI_node);

    // Inject: QMI_WMS_RAW_SEND_RESP_V01
    (void)QMI_node.update_default_qmi_wms_raw_send_resp_msg(result, error, msgRef);
    if(is_delivery_failure)
    {
      QMI_node.set_qmi_wms_raw_send_resp_message_delivery_failure_type(message_delivery_failure_type);
    }
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "Result = %d, Error = %d, Msg Ref = %d",
            result, error, msgRef);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);
  }

  // Expect: RIL_REQUEST_IMS_SEND_SMS_RESP
  if(FALSE == is_retry_wms_available)
  {
    RIL_node.update_default_ril_request_ims_send_sms_resp_empty();
  }
  else
  {
    RIL_SMS_Response* resp_ptr = RIL_node.update_default_ril_request_ims_send_sms_resp();
  }
  RIL_node.set_ril_error_value((RIL_Errno)atel_status);
  memset(tags,'\0', MAX_TAG_SIZE);
  snprintf(tags, MAX_TAG_SIZE, "Response Status = %d", atel_status);
  RIL_node.add_callflow_tag(tags);
  t.expect(RIL_node);

  t.set_end();
}

void util_sms_over_ims_send_sms_3gpp2(Testcase &t, AndroidTelTestnode &RIL_node, QMIRILTestnode &QMI_node, unsigned char retry, int messageRef, int sms_over_ims, int atel_status, qmi_result_type_v01 result, qmi_error_type_v01 error, uint16_t msgRef, uint8_t is_delivery_failure, wms_message_delivery_failure_type_enum_v01 message_delivery_failure_type, uint8_t is_retry_wms_available)
{
  char tags[MAX_TAG_SIZE];
  RIL_CDMA_SMS_Message cdma_sms_msg;
  t.set_start();

  // Inject: RIL_REQUEST_IMS_SEND_SMS
  RIL_node.update_default_ril_request_ims_send_sms(TRUE);
  ims_send_sms_3gpp2_payload* ims_send_sms = RIL_node.set_ril_request_ims_send_sms_3gpp2_tech_retry_msgref(RADIO_TECH_3GPP2, retry, messageRef);
  ims_send_sms->cdmaMessage[0].uTeleserviceID = QCRIL_SMS_TELESERVICE_BROADCAST;
  ims_send_sms->cdmaMessage[0].bIsServicePresent = TRUE;
  ims_send_sms->cdmaMessage[0].sAddress.number_of_digits = 0;
  ims_send_sms->cdmaMessage[0].sSubAddress.number_of_digits = 0;
  strlcpy((char *)ims_send_sms->cdmaMessage[0].aBearerData, "sometext",  RIL_CDMA_SMS_BEARER_DATA_MAX);
  ims_send_sms->cdmaMessage[0].uBearerDataLen = strlen("sometext");
  memset(&cdma_sms_msg, 0, sizeof(RIL_CDMA_SMS_Message));
  memcpy(&cdma_sms_msg,&ims_send_sms->cdmaMessage[0],sizeof(RIL_CDMA_SMS_Message));
  memset(tags,'\0', MAX_TAG_SIZE);
  snprintf(tags, MAX_TAG_SIZE, "Tech = %d, Retry = %d, Length = %d",
        RADIO_TECH_3GPP2, retry, ims_send_sms->cdmaMessage[0].uBearerDataLen);
  RIL_node.add_callflow_tag(tags);
  t.inject(RIL_node);

  if(is_retry_wms_available)
  {
    // Expect: QMI_WMS_RAW_SEND_REQ_V01
    QMI_node.update_default_qmi_wms_raw_send_req_msg_3gpp2(&cdma_sms_msg,WMS_MESSAGE_FORMAT_CDMA_V01,sms_over_ims);
    QMI_node.set_qmi_wms_raw_send_req_sms_on_ims(sms_over_ims);
    if(retry)
    {
      QMI_node.set_qmi_wms_raw_send_req_retry_message(WMS_MESSAGE_IS_A_RETRY_V01);
      QMI_node.set_qmi_wms_raw_send_req_retry_message_id(messageRef);
    }
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "sms_over_ims = %d", sms_over_ims);
    QMI_node.add_callflow_tag(tags);
    t.expect(QMI_node);

    // Inject: QMI_WMS_RAW_SEND_RESP_V01
    (void)QMI_node.update_default_qmi_wms_raw_send_resp_msg(result, error, msgRef);
    if(is_delivery_failure)
    {
      QMI_node.set_qmi_wms_raw_send_resp_message_delivery_failure_type(message_delivery_failure_type);
    }
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "Result = %d, Error = %d, Msg Ref = %d",
            result, error, msgRef);
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);
  }

  // Expect: RIL_REQUEST_IMS_SEND_SMS_RESP
  if(FALSE == is_retry_wms_available)
  {
    RIL_node.update_default_ril_request_ims_send_sms_resp_empty();
  }
  else
  {
    RIL_SMS_Response* resp_ptr = RIL_node.update_default_ril_request_ims_send_sms_resp();
  }
  RIL_node.set_ril_error_value((RIL_Errno)atel_status);
  memset(tags,'\0', MAX_TAG_SIZE);
  snprintf(tags, MAX_TAG_SIZE, "Response Status = %d", atel_status);
  RIL_node.add_callflow_tag(tags);
  t.expect(RIL_node);

  t.set_end();
}

void util_ril_utf_test_screen_state(Testcase &t, AndroidTelTestnode &RIL_node, QMIRILTestnode &QMI_node, int screen_state, boolean include_voice, uint8_t is_sig_info_needed, uint8_t is_err_needed)
{
  char tags[MAX_TAG_SIZE];

  t.set_start();

  // Inject: RIL_REQUEST_SCREEN_STATE
  ril_request_screen_state_t* state = RIL_node.update_default_ril_request_screen_state();
  state->screen_state = (ril_screen_state_enum_type)screen_state;
  memset(tags,'\0', MAX_TAG_SIZE);
  snprintf(tags, MAX_TAG_SIZE, "Screen State = %d", screen_state);
  RIL_node.add_callflow_tag(tags);
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_SCREEN_STATE
  RIL_node.update_default_ril_request_screen_state_resp();
  t.expect(RIL_node);

  // Expect: QMI_NAS_CONFIG_SIG_INFO2_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_config_sig_info2_req_msg_v01();
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

  if (screen_state == 1)
  {
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

   if(is_sig_info_needed)
  {
    // Expect: QMI_NAS_GET_SIG_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sig_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SIG_INFO_RESP_MSG_V01
    QMI_node.update_default_qmi_nas_get_sig_info_resp_msg_v01();
    QMI_node.set_qmi_nas_get_sig_info_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);
  }

  if(is_err_needed)
  {
    // Expect: QMI_NAS_GET_ERR_RATE_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_err_rate_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_ERR_RATE_RESP_MSG_V01
    QMI_node.update_default_qmi_nas_get_err_rate_resp_msg_v01();
    QMI_node.set_qmi_nas_get_err_rate_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);
  }

   // Expect: QMI_NAS_GET_EMBMS_STATUS_REQ_MSG_V01
   QMI_node.update_default_qmi_nas_get_embms_status_req_msg_v01();
   QMI_node.set_expect_skip_verification_status(false);
   t.expect(QMI_node);

   // Inject: QMI_NAS_GET_EMBMS_STATUS_RESP_MSG_V01
   QMI_node.update_default_qmi_nas_get_embms_status_resp_msg_v01();
   QMI_node.set_qmi_nas_get_embms_status_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
   t.inject(QMI_node);

   // Expect: RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED
   RIL_node.update_default_ril_unsol_response_voice_network_state_changed();
   t.expect(RIL_node);

   // Expect: RIL_UNSOL_SIGNAL_STRENGTH
   RIL_node.update_default_ril_unsol_signal_strength();
   t.expect(RIL_node);

   // Expect: QMI_PBM_INDICATION_REGISTER_REQ_V01
   QMI_node.update_default_qmi_pbm_indication_register_req_msg();
   QMI_node.set_qmi_pbm_indication_register_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01|PBM_REG_RECORD_UPDATE_EVENTS_V01|PBM_REG_PHONEBOOK_READY_EVENTS_V01);
   t.expect(QMI_node);

   // Inject: QMI_PBM_INDICATION_REGISTER_RESP_V01
   QMI_node.update_default_qmi_pbm_indication_register_resp_msg();
   QMI_node.set_qmi_pbm_indication_register_resp_v01_reg_mask(PBM_REG_EMERGENCY_NUMBER_LIST_EVENTS_V01|PBM_REG_RECORD_UPDATE_EVENTS_V01|PBM_REG_PHONEBOOK_READY_EVENTS_V01);
   QMI_node.set_qmi_pbm_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
   t.inject(QMI_node);

#ifdef RIL_UTF_L_MASTER
   if (include_voice)
   {
    // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
    QMI_node.update_default_qmi_voice_ind_reg_req_msg();
    QMI_node.set_qmi_voice_ind_req_handover_events(1);
    QMI_node.set_qmi_voice_ind_req_speech_events(1);
    t.expect(QMI_node);

    // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
    QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
    QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);
   }
#endif
  }
  if (screen_state == 0)
  {
   // Expect: QMI_NAS_LIMIT_SYS_INFO_IND_REPORTING_REQ_MSG_V01
   QMI_node.update_default_qmi_nas_limit_sys_info_ind_reporting_req_msg(0x1000003);
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

   // Expect: QMI_PBM_INDICATION_REGISTER_REQ_V01
   QMI_node.update_default_qmi_pbm_indication_register_req_msg();
   QMI_node.set_qmi_pbm_indication_register_mask(PBM_REG_RECORD_UPDATE_EVENTS_V01|PBM_REG_PHONEBOOK_READY_EVENTS_V01);
   t.expect(QMI_node);

   // Inject: QMI_PBM_INDICATION_REGISTER_RESP_V01
   QMI_node.update_default_qmi_pbm_indication_register_resp_msg();
   QMI_node.set_qmi_pbm_indication_register_resp_v01_reg_mask(PBM_REG_RECORD_UPDATE_EVENTS_V01|PBM_REG_PHONEBOOK_READY_EVENTS_V01);
   QMI_node.set_qmi_pbm_indication_register_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
   t.inject(QMI_node);

#ifdef RIL_UTF_L_MASTER
   if (include_voice)
   {
    // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
    QMI_node.update_default_qmi_voice_ind_reg_req_msg();
    QMI_node.set_qmi_voice_ind_req_handover_events(0);
    QMI_node.set_qmi_voice_ind_req_speech_events(0);
    t.expect(QMI_node);

    // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
    QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
    QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);
   }
#endif
  }

  t.set_end();
}

void util_ril_utf_get_subscription_info(Testcase &t, QMIRILTestnode &QMI_node, qmi_result_type_v01 result, qmi_error_type_v01 error, uint8_t is_priority_subs_valid, nas_is_priority_subs_enum_v01 is_priority_subs, uint8_t is_active_valid, nas_active_subs_info_enum_v01 is_active, uint8_t is_default_data_subs_valid, uint8_t is_default_data_subs, uint8_t voice_system_id_valid, uint32_t voice_system_id, uint8_t lte_voice_system_id_valid, uint32_t lte_voice_system_id, uint8_t wlan_voice_system_id_valid, uint32_t wlan_voice_system_id)
{
  // Expect: QMI_NAS_GET_SUBSCRIPTION_INFO_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_subscription_info_req_msg();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_SUBSCRIPTION_INFO_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_subscription_info_resp_msg(result, error);
  QMI_node.set_priority_subs_qmi_nas_get_subscription_info_resp_msg(is_priority_subs_valid, is_priority_subs);
  QMI_node.set_active_sub_qmi_nas_get_subscription_info_resp_msg(is_active_valid, is_active);
  QMI_node.set_default_data_sub_qmi_nas_get_subscription_info_resp_msg(is_default_data_subs_valid, is_default_data_subs);
  QMI_node.set_voice_system_id_qmi_nas_get_subscription_info_resp_msg(voice_system_id_valid, voice_system_id);
  QMI_node.set_lte_voice_system_id_qmi_nas_get_subscription_info_resp_msg(lte_voice_system_id_valid, lte_voice_system_id);
  QMI_node.set_wlan_voice_system_id_qmi_nas_get_subscription_info_resp_msg(wlan_voice_system_id_valid, wlan_voice_system_id);
  t.inject(QMI_node);
}

void util_ril_utf_test_set_lte_tune_away (
     Testcase &t,
     OEMTestnode &OEM_node,
     QMIRILTestnode &QMI_node,
     boolean lte_tune_away,
     int is_modem_comm_needed,
     int qmi_resp,
     int qmi_error,
     int res
)
{
    char tags[MAX_TAG_SIZE];

    t.set_start();

    // Inject: QCRIL_EVT_HOOK_SET_LTE_TUNE_AWAY
    OEM_node.update_default_oem_request_set_lte_tune_away_req(lte_tune_away);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "LTE TUNE AWAY = %d", lte_tune_away);
    OEM_node.add_callflow_tag(tags);
    t.inject(OEM_node);

    if(is_modem_comm_needed)
    {
        uint8_t trm_priority = !lte_tune_away;

        // Expect: QMI_NAS_AVOID_TUNEAWAY_REQ_MSG_V01
        QMI_node.update_default_qmi_nas_avoid_tune_away_req_msg(trm_priority);
        memset(tags,'\0', MAX_TAG_SIZE);
        snprintf(tags, MAX_TAG_SIZE, "TRM Priority = %d", trm_priority);
        QMI_node.add_callflow_tag(tags);
        t.expect(QMI_node);

        // Inject: QMI_NAS_AVOID_TUNEAWAY_RESP_MSG_V01
        QMI_node.update_default_qmi_nas_avoid_tune_away_resp_msg((qmi_result_type_v01)qmi_resp, (qmi_error_type_v01)qmi_error);
        memset(tags,'\0', MAX_TAG_SIZE);
        snprintf(tags, MAX_TAG_SIZE, "RESP = %d, ERROR = %d", qmi_resp, qmi_error);
        QMI_node.add_callflow_tag(tags);
        t.inject(QMI_node);
    }

    // Expect: QCRIL_EVT_HOOK_SET_LTE_TUNE_AWAY_RESP
    OEM_node.update_default_oem_request_set_lte_tune_away_resp();
    OEM_node.set_ril_error_value((RIL_Errno)res);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "Response Result = %d", res);
    OEM_node.add_callflow_tag(tags);
    t.expect(OEM_node);

    t.set_end();
}

void util_ril_utf_get_sys_info_for_lte_attached(Testcase &t, QMIRILTestnode &QMI_node, nas_service_domain_enum_type_v01 lte_srv_domain)
{
  // Expect: QMI_NAS_GET_SYS_INFO_REQ_MSG_V01
  QMI_node.update_default_qmi_nas_get_sys_info_req_msg_v01();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_SYS_INFO_RESP_MSG_V01
  QMI_node.update_default_qmi_nas_get_sys_info_resp_msg_v01();
  QMI_node.set_qmi_nas_get_sys_info_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_nas_get_sys_info_resp_msg_lte_attached(lte_srv_domain);
  t.inject(QMI_node);
}

void util_ril_utf_detach_apn(Testcase &t, QMIRILTestnode &QMI_node, qmi_result_type_v01 result, qmi_error_type_v01 error, nas_srv_domain_pref_enum_type_v01 srv_domain_pref)
{
  // Expect: QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ
  QMI_node.update_default_qmi_nas_set_system_selection_preference_req_msg();
  QMI_node.set_qmi_nas_system_selection_preference_req_srv_domain_pref(QMI_SRV_DOMAIN_PREF_PS_DETACH_V01);
  t.expect(QMI_node);

  // Inject: QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_RESP
  QMI_node.update_default_qmi_nas_set_system_selection_preference_resp_msg(result, error);
  t.inject(QMI_node);

  // Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_IND
  QMI_node.update_default_qmi_nas_system_selection_preference_ind_msg();
  QMI_node.set_qmi_nas_system_selection_preference_ind_srv_domain_pref(srv_domain_pref);
  t.inject(QMI_node);
}

void util_ril_utf_attach_apn(Testcase &t, QMIRILTestnode &QMI_node, qmi_result_type_v01 result, qmi_error_type_v01 error, nas_srv_domain_pref_enum_type_v01 srv_domain_pref)
{
  // Expect: QMI_NAS_SET_SYSTEM_SELECTION_PREFERENE_REQ
  QMI_node.update_default_qmi_nas_set_system_selection_preference_req_msg();
  QMI_node.set_qmi_nas_system_selection_preference_req_srv_domain_pref(QMI_SRV_DOMAIN_PREF_PS_ATTACH_V01);
  t.expect(QMI_node);

  // Inject: QMI_NAS_SET_SYSTEM_SELECTION_PREFERENE_RESP
  QMI_node.update_default_qmi_nas_set_system_selection_preference_resp_msg(result, error);
  t.inject(QMI_node);

  // Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENE_IND
  QMI_node.update_default_qmi_nas_system_selection_preference_ind_msg();
  QMI_node.set_qmi_nas_system_selection_preference_ind_srv_domain_pref(srv_domain_pref);
  t.inject(QMI_node);
}

void util_ril_utf_on_demand_attach_apn(Testcase &t, QMIRILTestnode &QMI_node, qmi_result_type_v01 result, qmi_error_type_v01 error, nas_srv_domain_pref_enum_type_v01 srv_domain_pref)
{
  util_ril_utf_on_demand_attach_apn(t, QMI_node,result, error, srv_domain_pref, 0);
}

void util_ril_utf_on_demand_attach_apn(Testcase &t, QMIRILTestnode &QMI_node, qmi_result_type_v01 result, qmi_error_type_v01 error, nas_srv_domain_pref_enum_type_v01 srv_domain_pref, mode_pref_mask_type_v01 mask)
{
  // Expect: QMI_NAS_SET_SYSTEM_SELECTION_PREFERENE_REQ
  QMI_node.update_default_qmi_nas_set_system_selection_preference_req_msg();
  QMI_node.set_qmi_nas_system_selection_preference_req_srv_domain_pref(QMI_SRV_DOMAIN_PREF_ON_DEMAND_PS_ATTACH_V01);
  t.expect(QMI_node);

  // Inject: QMI_NAS_SET_SYSTEM_SELECTION_PREFERENE_RESP
  QMI_node.update_default_qmi_nas_set_system_selection_preference_resp_msg(result, error);
  t.inject(QMI_node);

  // Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENE_IND
  QMI_node.update_default_qmi_nas_system_selection_preference_ind_msg();
  QMI_node.set_qmi_nas_system_selection_preference_ind_srv_domain_pref(srv_domain_pref);
#ifdef RIL_UTF_L_MASTER
  if (mask)
  {
    QMI_node.set_qmi_nas_system_selection_preference_ind_rat_disabled_mask(mask);
  }
#endif
  t.inject(QMI_node);
}

void util_ril_utf_dsd_ind_info(Testcase &t, AndroidTelTestnode &RIL_node, OEMTestnode &OEM_node, QMIRILTestnode &QMI_node, uint8_t avail_sys_valid, uint32_t avail_sys_len, dsd_sys_network_enum_v01 technology, dsd_sys_rat_ex_enum_v01 rat_value, dsd_sys_so_mask_v01 so_mask, uint8_t is_dsd_diff, uint8_t is_wlan_status, uint8_t is_sig_info_needed, uint8_t is_err_needed, int rat, uint8_t is_multi_rat,
uint8_t is_wlan_imss)
{
  char tags[MAX_TAG_SIZE];
  NOTUSED(avail_sys_len);
  t.set_start();

  // Inject: QMI_DSD_SYSTEM_STATUS_IND_MSG
  dsd_system_status_ind_msg_v01* dsd_info = QMI_node.update_default_qmi_dsd_system_status_ind();
  dsd_info->avail_sys_valid = avail_sys_valid;
  QMI_node.set_qmi_dsd_system_status_ind_avail_sys(technology, rat_value, so_mask);
  if(is_multi_rat)
  {
    QMI_node.set_qmi_dsd_system_status_ind_avail_sys(DSD_SYS_NETWORK_WLAN_V01, DSD_SYS_RAT_EX_3GPP_WLAN_V01, QMI_DSD_SO_MASK_UNSPECIFIED_V01);
  }
  memset(tags,'\0', MAX_TAG_SIZE);
  snprintf(tags, MAX_TAG_SIZE, "tech = %d, rat = %d, mask = %x",
        technology, rat_value, (int)so_mask);
  QMI_node.add_callflow_tag(tags);
  t.inject(QMI_node);

  if(is_wlan_imss)
  {
    // Expect: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_REQ_V01
    QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_req_v01();
    t.expect(QMI_node);

    // Inject: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_RSP_V01
    QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_rsp_v01();
    QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    QMI_node.set_wifi_call_preference_get_client_provisioning_config_rsp_v01(TRUE, IMS_SETTINGS_WFC_WLAN_PREFERRED_V01);
    t.inject(QMI_node);
  }

  if(is_dsd_diff)
  {
    // Expect: QCRIL_EVT_HOOK_UNSOL_WWAN_AVAILABLE
    OEM_node.update_default_qcril_evt_hook_unsol_wlan_available(is_wlan_status);
    OEM_node.set_expect_skip_verification_status(false);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "wlan_status = %d", is_wlan_status);
    OEM_node.add_callflow_tag(tags);
    t.expect(OEM_node);

    // Expect: RIL_UNSOL_OEM_HOOK_RAW
    RIL_node.update_default_ril_unsol_oem_hook_raw();
    RIL_node.set_expect_skip_verification_status(false);
    t.expect(RIL_node);

    util_ril_utf_physical_channel_configuration_query_sequence(t, QMI_node);
  }

  if(is_sig_info_needed)
  {
    // Expect: QMI_NAS_GET_SIG_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_sig_info_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_SIG_INFO_RESP_MSG_V01
    QMI_node.update_default_qmi_nas_get_sig_info_resp_msg_v01();
    QMI_node.set_qmi_nas_get_sig_info_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);
  }

  if(is_err_needed)
  {
    // Expect: QMI_NAS_GET_ERR_RATE_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_err_rate_req_msg_v01();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_ERR_RATE_RESP_MSG_V01
    QMI_node.update_default_qmi_nas_get_err_rate_resp_msg_v01();
    QMI_node.set_qmi_nas_get_err_rate_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);
  }

  if(is_dsd_diff)
  {
    // Expect: RIL_UNSOL_SIGNAL_STRENGTH
    RIL_node.update_default_ril_unsol_signal_strength();
    t.expect(RIL_node);
  }

  if( RADIO_TECH_UNKNOWN != rat )
  {
    // Expect: RIL_UNSOL_VOICE_RADIO_TECH_CHANGED
    RIL_node.update_default_ril_unsol_voice_radio_tech_changed();
    RIL_node.set_ril_unsol_voice_radio_tech_changed_radio_tech((RIL_RadioTechnology)rat);
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "RAT = %d",rat);
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);

    if(is_sig_info_needed)
    {
      // Expect: QMI_NAS_GET_SIG_INFO_REQ_MSG_V01
      QMI_node.update_default_qmi_nas_get_sig_info_req_msg_v01();
      t.expect(QMI_node);

      // Inject: QMI_NAS_GET_SIG_INFO_RESP_MSG_V01
      QMI_node.update_default_qmi_nas_get_sig_info_resp_msg_v01();
      QMI_node.set_qmi_nas_get_sig_info_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
      t.inject(QMI_node);
    }

    if(is_err_needed)
    {
      // Expect: QMI_NAS_GET_ERR_RATE_REQ_MSG_V01
      QMI_node.update_default_qmi_nas_get_err_rate_req_msg_v01();
      t.expect(QMI_node);

      // Inject: QMI_NAS_GET_ERR_RATE_RESP_MSG_V01
      QMI_node.update_default_qmi_nas_get_err_rate_resp_msg_v01();
      QMI_node.set_qmi_nas_get_err_rate_resp_msg_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
      t.inject(QMI_node);
    }

    // Expect: RIL_UNSOL_SIGNAL_STRENGTH
    RIL_node.update_default_ril_unsol_signal_strength();
    t.expect(RIL_node);
  }

  if(is_dsd_diff)
  {
    // Expect: RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED
    RIL_node.update_default_ril_unsol_response_voice_network_state_changed();
    t.expect(RIL_node);
  }

  t.set_end();
}

void util_ril_utf_set_dual_standby_pref(Testcase &t, QMIRILTestnode &QMI_node, qmi_result_type_v01 result, qmi_error_type_v01 error, nas_srv_domain_pref_enum_type_v01 srv_domain_pref, nas_dds_duration_enum_v01 dds_duration)
{
  // Expect: QMI_NAS_DUAL_STANDBY_PREF_REQ_MSG_V01_REQ
  QMI_node.update_default_qmi_nas_dual_standby_pref_req_msg();
  QMI_node.update_qmi_nas_dual_standby_pref_data_subs(NAS_PRIMARY_SUBSCRIPTION_V01);
  QMI_node.update_qmi_nas_dual_standby_pref_dds_duration(dds_duration);
  t.expect(QMI_node);

  // Inject: QMI_NAS_DUAL_STANDBY_PREF_REQ_MSG_V01_RESP
  QMI_node.update_default_qmi_nas_dual_standby_pref_resp_msg(result, error);
  t.inject(QMI_node);

  // Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENE_IND
  QMI_node.update_default_qmi_nas_system_selection_preference_ind_msg();
  QMI_node.set_qmi_nas_system_selection_preference_ind_srv_domain_pref(srv_domain_pref);
  t.inject(QMI_node);
}

struct qcril_test_property_type db_properties[QCRIL_TEST_MAX_PROPERTIES] = {};

int db_property_set(const char *key, const char *value)
{
  int found = 0;
  int prop_idx=0;
  int prop_tbl_len = sizeof(db_properties) / sizeof(struct qcril_test_property_type);

  while (!found && prop_idx <  prop_tbl_len) {
       if( strcmp( db_properties[prop_idx].key, "") == 0 ) //If first index is empty, then we have to make an entry in first index.
       {
         break;
       }

       if( strcmp( db_properties[prop_idx].key, key) == 0 ) {
           RIL_UTF_DEBUG("\n QMI_RIL_UTL: Found property %s", key);
           RIL_UTF_DEBUG("\n QMI_RIL_UTL: updating %s to %s\n",
                             db_properties[prop_idx].key,
                             value);

            strlcpy(db_properties[prop_idx].prop_val, value, QCRIL_TEST_PROPERTY_VALUE_LEN);
            return E_SUCCESS;
       }
       else
       {
            prop_idx++;
       }
  }
  RIL_UTF_DEBUG("\n\nprop_idx %d\n\n", prop_idx);
  if ( prop_idx < QCRIL_TEST_MAX_PROPERTIES )
  {
    memset(db_properties[prop_idx].key,'\0',QCRIL_TEST_PROPERTY_KEY_LEN);
    memset(db_properties[prop_idx].prop_val,'\0',QCRIL_TEST_PROPERTY_VALUE_LEN);
    strlcpy(db_properties[prop_idx].key, key, QCRIL_TEST_PROPERTY_KEY_LEN);
    strlcpy(db_properties[prop_idx].prop_val, value, QCRIL_TEST_PROPERTY_VALUE_LEN);
    return E_SUCCESS;
  }
  else
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Could not find property %s", key);
    return E_FAILURE;//fail
  }
}

void util_ril_utf_db_property_init(Testcase &t, DBTestnode &DB_node)
{
  int prop_idx=0;
  int prop_tbl_len = sizeof(db_properties) / sizeof(struct qcril_test_property_type);
  while (prop_idx <  prop_tbl_len) {
    if( strcmp( db_properties[prop_idx].key, "") == 0 )
    {
      break;
    }
    DB_node.utf_db_insert_properties_table(db_properties[prop_idx].key, db_properties[prop_idx].prop_val, 0);
    t.inject(DB_node);
    prop_idx++;
  }

  // clean up db property table after initilization
  int i = 0;
  for(i=0; i<QCRIL_TEST_MAX_PROPERTIES; i++)
  {
    if( strcmp( db_properties[i].key, "") == 0 )
    {
      break;
    }
    memset(db_properties[i].key,'\0',QCRIL_TEST_PROPERTY_KEY_LEN);
    memset(db_properties[i].prop_val,'\0',QCRIL_TEST_PROPERTY_VALUE_LEN);
  }
}

void util_ril_utf_insert_1_qmi_uim_get_label_call(QMIRILTestnode &QMI_node, Testcase &t)
{
  uint8_t app_value_1[30] = {0xA0,0x00,0x00,0x00,0x87,0x10,0x02,0xF3,0x10,0xFF,0xFF,0x89,0x08,0x00,0x00,0xFF};

  t.set_start();
  //Expect: QMI_UIM_GET_LABEL_REQ
  uim_get_label_req_msg_v01 *label_req1 = QMI_node.update_default_qmi_uim_get_label_req();
  label_req1->application_information.slot = (uim_slot_enum_v01)0x01;
  label_req1->application_information.aid_len = 0x10;
  memcpy(label_req1->application_information.aid, app_value_1, 16);
  t.expect(QMI_node);

  //Inject: QMI_UIM_GET_LABEL_RESP
  uim_get_label_resp_msg_v01 *response1 = QMI_node.update_default_qmi_uim_get_label_resp();
  response1->resp.result = QMI_RESULT_SUCCESS_V01;
  strcpy(response1->label, "USIM");
  response1->label_len = 4;
  response1->label_valid = TRUE;
  t.inject(QMI_node);
  t.set_end();
}

void util_ril_utf_insert_4_qmi_uim_get_label_calls(QMIRILTestnode &QMI_node, Testcase &t)
{
  uint8_t app_value_1[30] = {0xA0,0x00,0x00,0x00,0x87,0x10,0x02,0xF3,0x10,0xFF,0xFF,0x89,0x08,0x00,0x00,0xFF};
  uint8_t app_value_2[30] = {0xA0,0x00,0x00,0x00,0x87,0x10,0x04,0xF3,0x10,0xFF,0xFF,0x89,0x08,0x00,0x00,0xFF};
  uint8_t app_value_3[30] = {0xA0,0x00,0x00,0x03,0x43,0x10,0x02,0xF3,0x10,0xFF,0xFF,0x89,0x02,0x00,0x00,0xFF};
  uint8_t app_value_4[30] = {0xA0,0x00,0x00,0x00,0x63,0x50,0x4B,0x43,0x53,0x2D,0x31,0x35};

  t.set_start();
  //Expect: QMI_UIM_GET_LABEL_REQ
  uim_get_label_req_msg_v01 *label_req1 = QMI_node.update_default_qmi_uim_get_label_req();
  label_req1->application_information.slot = (uim_slot_enum_v01)0x01;
  label_req1->application_information.aid_len = 0x10;
  memcpy(label_req1->application_information.aid, app_value_1, 16);
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
  memcpy(label_req2->application_information.aid, app_value_2, 16);
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
  memcpy(label_req3->application_information.aid, app_value_3, 16);
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
  memcpy(label_req4->application_information.aid, app_value_4, 16);
  t.expect(QMI_node);

  //Inject: QMI_UIM_GET_LABEL_RESP
  uim_get_label_resp_msg_v01 *response4 = QMI_node.update_default_qmi_uim_get_label_resp();
  response4->resp.result = QMI_RESULT_SUCCESS_V01;
  strcpy(response4->label, "PKCS");
  response4->label_len = 4;
  response4->label_valid = TRUE;
  t.inject(QMI_node);
  t.set_end();
}

void util_ril_utf_init_3gpp_sim_app(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode& RIL_node, uim_app_state_enum_v01 app_state)
{
  uim_status_change_ind_msg_v01 *init_status;
  uim_get_label_req_msg_v01 *label_req;
  uint8_t aid_value_1[16] = {0xA0,0x00,0x00,0x00,0x87,0x10,0x02,0xF3,0x10,0xFF,0xFF,0x89,0x08,0x00,0x00,0xFF};

  t.set_start();
  //Set up initial card status for test case
  init_status = (uim_status_change_ind_msg_v01 *)malloc(sizeof(uim_status_change_ind_msg_v01));
  memset(init_status, 0x0, sizeof(uim_status_change_ind_msg_v01));
  init_status->extended_card_status_valid = TRUE;
  init_status->extended_card_status.card_info_len = 1;
  init_status->extended_card_status.card_info[0].card_state = UIM_EXTENDED_CARD_STATE_PRESENT_V01;
  init_status->extended_card_status.card_info[0].upin.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
  init_status->extended_card_status.card_info[0].upin.pin_retries = 0x00;
  init_status->extended_card_status.card_info[0].upin.puk_retries = 0x00;
  init_status->extended_card_status.card_info[0].app_info_len = 0x01;
  init_status->extended_card_status.index_gw[0] = 0x0;
  init_status->extended_card_status.index_gw_len = 1;
  //init_status->extended_card_status.index_1x[0] = 0x2;
  init_status->extended_card_status.index_1x_len = 0;

  // App 0
  init_status->extended_card_status.card_info[0].app_info[0].app_type = UIM_APP_TYPE_USIM_V01;
  init_status->extended_card_status.card_info[0].app_info[0].app_state = app_state;
  init_status->extended_card_status.card_info[0].app_info[0].perso_state = UIM_PERSO_STATE_UNKNOWN_V01;
  init_status->extended_card_status.card_info[0].app_info[0].perso_feature = UIM_PERSO_FEATURE_STATUS_UNKNOWN_V01;
  init_status->extended_card_status.card_info[0].app_info[0].perso_retries = 0x00;
  init_status->extended_card_status.card_info[0].app_info[0].perso_unblock_retries = 0x00;
  init_status->extended_card_status.card_info[0].app_info[0].aid_value_len = 0x10;
  memcpy(init_status->extended_card_status.card_info[0].app_info[0].aid_value, aid_value_1, 16);
  load_qmi_card_status(init_status, &t, &QMI_node, &RIL_node);

  //Inject: RIL_REQUEST_GET_SIM_STATUS
  RIL_node.update_default_ril_request_get_sim_status();
  t.inject(RIL_node);

  //Expect: QMI_UIM_GET_LABEL_REQ
  uim_get_label_req_msg_v01 *label_req1 = QMI_node.update_default_qmi_uim_get_label_req();
  label_req1->application_information.slot = (uim_slot_enum_v01)0x01;
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

  //Expect: RIL_REQUEST_GET_SIM_STATUS_RESPONSE
  RIL_CardStatus_v6* status_ptr = RIL_node.update_default_ril_request_get_sim_status_resp();
  status_ptr->card_state = RIL_CARDSTATE_PRESENT;
  status_ptr->universal_pin_state = RIL_PINSTATE_UNKNOWN;
  status_ptr->cdma_subscription_app_index = -1;
  status_ptr->gsm_umts_subscription_app_index = 0;
  status_ptr->ims_subscription_app_index = -1;
  status_ptr->num_applications = 1;

  status_ptr->applications[0].app_type = RIL_APPTYPE_USIM;
  status_ptr->applications[0].app_state = (RIL_AppState)app_state;
  status_ptr->applications[0].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
  status_ptr->applications[0].aid_ptr = (char *)malloc(33);
  strcpy(status_ptr->applications[0].aid_ptr, "a0000000871002f310ffff89080000ff");
  status_ptr->applications[0].app_label_ptr = (char *)malloc(5);
  strcpy(status_ptr->applications[0].app_label_ptr, "USIM");
  t.expect(RIL_node);
  t.set_end();
}

void util_ril_utf_init_3gpp2_sim_app(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode& RIL_node)
{
  uim_status_change_ind_msg_v01 *init_status;
  uim_get_label_req_msg_v01 *label_req;
  uint8_t aid_value_1[16] = {0xA0,0x00,0x00,0x00,0x87,0x10,0x02,0xF3,0x10,0xFF,0xFF,0x89,0x08,0x00,0x00,0xFF};

  t.set_start();
  //Set up initial card status for test case
  init_status = (uim_status_change_ind_msg_v01 *)malloc(sizeof(uim_status_change_ind_msg_v01));
  memset(init_status, 0x0, sizeof(uim_status_change_ind_msg_v01));
  init_status->extended_card_status_valid = TRUE;
  init_status->extended_card_status.card_info_len = 1;
  init_status->extended_card_status.card_info[0].card_state = UIM_EXTENDED_CARD_STATE_PRESENT_V01;
  init_status->extended_card_status.card_info[0].upin.pin_state = UIM_PIN_STATE_UNKNOWN_V01;
  init_status->extended_card_status.card_info[0].upin.pin_retries = 0x00;
  init_status->extended_card_status.card_info[0].upin.puk_retries = 0x00;
  init_status->extended_card_status.card_info[0].app_info_len = 0x01;
  init_status->extended_card_status.index_1x[0] = 0x0;
  init_status->extended_card_status.index_1x_len = 1;

  // App 0
  init_status->extended_card_status.card_info[0].app_info[0].app_type = UIM_APP_TYPE_CSIM_V01;
  init_status->extended_card_status.card_info[0].app_info[0].app_state = UIM_APP_STATE_DETECTED_V01;
  init_status->extended_card_status.card_info[0].app_info[0].perso_state = UIM_PERSO_STATE_UNKNOWN_V01;
  init_status->extended_card_status.card_info[0].app_info[0].perso_feature = UIM_PERSO_FEATURE_STATUS_UNKNOWN_V01;
  init_status->extended_card_status.card_info[0].app_info[0].perso_retries = 0x00;
  init_status->extended_card_status.card_info[0].app_info[0].perso_unblock_retries = 0x00;
  init_status->extended_card_status.card_info[0].app_info[0].aid_value_len = 0x10;
  memcpy(init_status->extended_card_status.card_info[0].app_info[0].aid_value, aid_value_1, 16);
  load_qmi_card_status(init_status, &t, &QMI_node, &RIL_node);

  //Inject: RIL_REQUEST_GET_SIM_STATUS
  RIL_node.update_default_ril_request_get_sim_status();
  t.inject(RIL_node);

  //Expect: QMI_UIM_GET_LABEL_REQ
  uim_get_label_req_msg_v01 *label_req1 = QMI_node.update_default_qmi_uim_get_label_req();
  label_req1->application_information.slot = (uim_slot_enum_v01)0x01;
  label_req1->application_information.aid_len = 0x10;
  memcpy(label_req1->application_information.aid, aid_value_1, 16);
  t.expect(QMI_node);

  //Inject: QMI_UIM_GET_LABEL_RESP
  uim_get_label_resp_msg_v01 *response1 = QMI_node.update_default_qmi_uim_get_label_resp();
  response1->resp.result = QMI_RESULT_SUCCESS_V01;
  strcpy(response1->label, "CSIM");
  response1->label_len = 4;
  response1->label_valid = TRUE;
  t.inject(QMI_node);

  //Expect: RIL_REQUEST_GET_SIM_STATUS_RESPONSE
  RIL_CardStatus_v6* status_ptr = RIL_node.update_default_ril_request_get_sim_status_resp();
  status_ptr->card_state = RIL_CARDSTATE_PRESENT;
  status_ptr->universal_pin_state = RIL_PINSTATE_UNKNOWN;
  status_ptr->cdma_subscription_app_index = 0;
  status_ptr->gsm_umts_subscription_app_index = -1;
  status_ptr->ims_subscription_app_index = -1;
  status_ptr->num_applications = 1;

  status_ptr->applications[0].app_type = RIL_APPTYPE_CSIM;
  status_ptr->applications[0].app_state = RIL_APPSTATE_DETECTED;
  status_ptr->applications[0].perso_substate = RIL_PERSOSUBSTATE_UNKNOWN;
  status_ptr->applications[0].aid_ptr = (char *)malloc(33);
  //strcpy(status_ptr->applications[0].aid_ptr, "a0000003431002f310ffff89020000ff");
  strcpy(status_ptr->applications[0].aid_ptr, "a0000000871002f310ffff89080000ff");
  status_ptr->applications[0].app_label_ptr = (char *)malloc(5);
  strcpy(status_ptr->applications[0].app_label_ptr, "CSIM");
  t.expect(RIL_node);
  t.set_end();
}

void util_ril_utf_init_hw_mbn(Testcase &t, int sim_num)
{
  QMIRILTestnode QMI_node;

  uint8_t* hw_config_id_0 = (uint8_t *)"dummy/config/id";
  uint32_t hw_config_id_len_0 = strlen((char *)hw_config_id_0);
  uint8_t* hw_config_id_1 = (uint8_t *)"data/modem_config/mcfg_hw/generic/common/MSM8994/LA_DS/mcfg_hw.mbn";
  uint32_t hw_config_id_len_1 = strlen((char *)hw_config_id_1);
  uint8_t* hw_config_id_2 = (uint8_t *)"data/modem_config/mcfg_hw/generic/common/MSM8994/LA_SS/mcfg_hw.mbn";
  uint32_t hw_config_id_len_2 = strlen((char *)hw_config_id_2);
  const uint32_t hw_config_len_1 = 8724;
  const uint32_t hw_config_len_2 = 8716;

  util_ril_utf_qmi_pdc_get_selected_config(t, hw_config_id_0, NULL, PDC_CONFIG_TYPE_MODEM_PLATFORM_V01, 0);

  util_ril_utf_qmi_pdc_deactivate_config(t, PDC_CONFIG_TYPE_MODEM_PLATFORM_V01, 0);

  util_ril_utf_qmi_pdc_delete_config(t, hw_config_id_0, PDC_CONFIG_TYPE_MODEM_PLATFORM_V01);

  if (sim_num == 2)
  {
    util_ril_utf_qmi_pdc_load_config(t, (char *)hw_config_id_1, hw_config_id_1, hw_config_len_1, PDC_CONFIG_TYPE_MODEM_PLATFORM_V01);

    util_ril_utf_qmi_pdc_set_selected_config(t, hw_config_id_1, PDC_CONFIG_TYPE_MODEM_PLATFORM_V01, 0);
  }
  else
  {
    util_ril_utf_qmi_pdc_load_config(t, (char *)hw_config_id_2, hw_config_id_2, hw_config_len_2, PDC_CONFIG_TYPE_MODEM_PLATFORM_V01);

    util_ril_utf_qmi_pdc_set_selected_config(t, hw_config_id_2, PDC_CONFIG_TYPE_MODEM_PLATFORM_V01, 0);
  }

  util_ril_utf_qmi_pdc_activate_config(t, PDC_CONFIG_TYPE_MODEM_PLATFORM_V01, 0, 1);
}

void util_ril_utf_init_sw_mbn(Testcase &t, int rild_num)
{
  uint8_t* sw_config_id_0 = (uint8_t *)"data/modem_config/mcfg_hw/generic/common/Default/Default/mcfg_sw.mbn";
  uint8_t sw_config_id_1[100];
  uint8_t* sw_config_file_1 = (uint8_t *)"data/modem_config/mcfg_sw/generic/common/W_One/Commercial/mcfg_sw.mbn";
  const uint32_t sw_config_len_1 = 20664;
  snprintf((char *)sw_config_id_1, sizeof(sw_config_id_1), "%s%d", sw_config_file_1, rild_num);

  util_ril_utf_init_sw_mbn(t, rild_num, (char *)sw_config_id_0, (char *)sw_config_file_1, (char *)sw_config_id_1, sw_config_len_1);
}

void util_ril_utf_init_sw_mbn(Testcase &t, int rild_num, const char* sw_config_id_0 , char *sw_config_file_1, const char* sw_config_id_1, uint32_t sw_config_len_1)
{
  QMIRILTestnode QMI_node;

  util_ril_utf_qmi_pdc_get_selected_config(t, (uint8_t *)sw_config_id_0, NULL, PDC_CONFIG_TYPE_MODEM_SW_V01, rild_num);

  util_ril_utf_qmi_pdc_get_config_info(t, (uint8_t *)sw_config_id_0);

  util_ril_utf_qmi_pdc_deactivate_config(t, PDC_CONFIG_TYPE_MODEM_SW_V01, rild_num);

  util_ril_utf_qmi_pdc_delete_config(t, (uint8_t *)sw_config_id_1, PDC_CONFIG_TYPE_MODEM_SW_V01);

  util_ril_utf_qmi_pdc_load_config(t, sw_config_file_1, (uint8_t *)sw_config_id_1, sw_config_len_1, PDC_CONFIG_TYPE_MODEM_SW_V01);

  util_ril_utf_qmi_pdc_set_selected_config(t, (uint8_t *)sw_config_id_1, PDC_CONFIG_TYPE_MODEM_SW_V01, rild_num);

  if (rild_num)
  {
    util_ril_utf_qmi_pdc_get_selected_config(t, NULL, (uint8_t *)sw_config_id_1, PDC_CONFIG_TYPE_MODEM_SW_V01, 0);
    util_ril_utf_qmi_pdc_get_selected_config(t, NULL, (uint8_t *)sw_config_id_1, PDC_CONFIG_TYPE_MODEM_SW_V01, 1);
    util_ril_utf_qmi_pdc_activate_config(t, PDC_CONFIG_TYPE_MODEM_SW_V01, rild_num, 2);

    util_ril_utf_qmi_pdc_cleanup_config(t, PDC_CONFIG_TYPE_MODEM_SW_V01, rild_num);
  }
}

void util_ril_utf_init_msim_mbn_properties(int mbn_loaded)
{
  property_set("persist.radio.multisim.config", "dsds");
  property_set("ro.baseband","dsda");
  property_set("persist.vendor.radio.hw_mbn_update", "1");
  property_set("persist.vendor.radio.sw_mbn_update", "1");
  property_set("persist.vendor.radio.iccid_1", "89003100085865131146");
  property_set("persist.vendor.radio.stack_id_0", "0");
  property_set("persist.vendor.radio.stack_id_1", "1");
  if (mbn_loaded)
  {
    property_set("persist.vendor.radio.hw_mbn_loaded", "1");
    property_set("persist.vendor.radio.sw_mbn_loaded", "1");
  }
}

void util_ril_utf_init_no_sim_mbn_properties(int mbn_loaded)
{
  property_set("persist.radio.multisim.config", "dsds");
  property_set("ro.baseband","dsda");
  property_set("persist.vendor.radio.hw_mbn_update", "1");
  property_set("persist.vendor.radio.sw_mbn_update", "1");
  if (mbn_loaded)
  {
    property_set("persist.vendor.radio.hw_mbn_loaded", "1");
    property_set("persist.vendor.radio.sw_mbn_loaded", "1");
  }
}

void util_ril_utf_qmi_pdc_disable_modem_update(Testcase &t)
{
  QMIRILTestnode QMI_node;

  t.set_start();
  // Expect: QMI_PDC_SET_FEATURE_REQ_V01
  QMI_node.update_default_qmi_pdc_set_feature_req(0xFFFFFFFF);
  QMI_node.set_default_qmi_pdc_set_feature_req_selection_mode(PDC_SELECTION_MODE_DISABLED_V01);
  t.expect(QMI_node);

  // Inject: QMI_PDC_SET_FEATURE_RESP_V01
  QMI_node.update_default_qmi_pdc_set_feature_resp();
  t.inject(QMI_node);

  // Inject: QMI_PDC_SET_FEATURE_IND_V01
  QMI_node.update_default_qmi_pdc_set_feature_ind();
  t.inject(QMI_node);
  t.set_end();
}

void util_ril_utf_qmi_pdc_get_selected_config(Testcase &t, const uint8_t* active_config_id,
        const uint8_t* pending_config_id, pdc_config_type_enum_v01 type, int rild_num)
{
  QMIRILTestnode QMI_node;
  uint32_t active_config_id_len = 0, pending_config_id_len = 0;

  if (active_config_id)
    active_config_id_len = strlen((char *)active_config_id);
  if (pending_config_id)
    pending_config_id_len = strlen((char *)pending_config_id);

  t.set_start();
  // Expect: QMI_PDC_GET_SELECTED_CONFIG_REQ
  QMI_node.update_default_qmi_pdc_get_selected_config_req();
  QMI_node.set_qmi_pdc_get_selected_config_req_config_type(type);
  if (type == PDC_CONFIG_TYPE_MODEM_SW_V01)
  {
    QMI_node.set_qmi_pdc_get_selected_config_req_subscription_id(rild_num);
  }
  t.expect(QMI_node);

  // Inject: QMI_PDC_GET_SELECTED_CONFIG_RESP
  QMI_node.update_default_qmi_pdc_get_selected_config_resp();
  t.inject(QMI_node);

  // Inject: QMI_PDC_GET_SELECTED_CONFIG_IND
  QMI_node.update_default_qmi_pdc_get_selected_config_ind();
  if (active_config_id_len > 0)
    QMI_node.set_qmi_pdc_get_selected_config_ind_active_config_id(
            active_config_id, active_config_id_len);
  if (pending_config_id_len > 0)
    QMI_node.set_qmi_pdc_get_selected_config_ind_pending_config_id(
            pending_config_id, pending_config_id_len);
  t.inject(QMI_node);

  t.set_end();
}

void util_ril_utf_qmi_pdc_deactivate_config(Testcase &t, pdc_config_type_enum_v01 type, int rild_num)
{
  QMIRILTestnode QMI_node;

  t.set_start();
  // Expect: QMI_PDC_DEACTIVATE_CONFIG_REQ
  QMI_node.update_default_qmi_pdc_deactivate_config_req();
  QMI_node.set_qmi_pdc_get_selected_config_req_config_type(type);
  if (type == PDC_CONFIG_TYPE_MODEM_SW_V01)
  {
    QMI_node.set_qmi_pdc_get_selected_config_req_subscription_id(rild_num);
  }
  t.expect(QMI_node);

  // Inject: QMI_PDC_DEACTIVATE_CONFIG_RESP
  QMI_node.update_default_qmi_pdc_deactivate_config_resp();
  t.inject(QMI_node);

  // Inject: QMI_PDC_DEACTIVATE_CONFIG_IND
  QMI_node.update_default_qmi_pdc_deactivate_config_ind();
  t.inject(QMI_node);
  t.set_end();
}

void util_ril_utf_qmi_pdc_delete_config(Testcase &t, uint8_t* config_id, pdc_config_type_enum_v01 type)
{
  QMIRILTestnode QMI_node;
  uint32_t config_id_len = strlen((char *)config_id);

  t.set_start();
  // Expect: QMI_PDC_DELETE_CONFIG_REQ
  QMI_node.update_default_qmi_pdc_delete_config_req();
  QMI_node.set_qmi_pdc_delete_config_req_config_id(config_id, config_id_len);
  QMI_node.set_qmi_pdc_delete_config_req_config_type(type);
  t.expect(QMI_node);

  // Inject: QMI_PDC_DELETE_CONFIG_RESP
  QMI_node.update_default_qmi_pdc_delete_config_resp();
  t.inject(QMI_node);

  // Inject: QMI_PDC_DELETE_CONFIG_IND
  QMI_node.update_default_qmi_pdc_delete_config_ind();
  t.inject(QMI_node);

  t.set_end();
}

void util_ril_utf_qmi_pdc_load_config(Testcase &t, char *file_name, uint8_t* config_id, const uint32_t config_len, pdc_config_type_enum_v01 type)
{
  QMIRILTestnode QMI_node;
  uint32_t config_id_len = strlen((char *)config_id);
  const uint32_t pdc_frame_size = 900;
  FILE *mbn_file_ptr;
  char config_frame[PDC_CONFIG_FRAME_SIZE_MAX_V01];
  int config_frame_len = 0;
  int read_success = 0;
  mbn_file_ptr = fopen(file_name, "rb");

  t.set_start();
  for (unsigned int i = 0; i < config_len/pdc_frame_size + 1 ; ++i)
  {
    // Expect: QMI_PDC_LOAD_CONFIG_REQ
    memset(config_frame, 0, sizeof(config_frame));
    fseek(mbn_file_ptr, pdc_frame_size*i, SEEK_SET);
    read_success = fread(&config_frame, pdc_frame_size, 1, mbn_file_ptr);
    if (!read_success) // Reached EoF
    {
      config_frame_len = config_len - pdc_frame_size*i;
    }
    else
    {
      config_frame_len = pdc_frame_size;
    }
    QMI_node.update_default_qmi_pdc_load_config_req();
    QMI_node.set_qmi_pdc_load_config_req_config_id(config_id, config_id_len);
    QMI_node.set_qmi_pdc_load_config_req_config_type(type);
    QMI_node.set_qmi_pdc_load_config_req_config_size(config_len);
    QMI_node.set_qmi_pdc_load_config_req_config_frame((uint8_t *)config_frame, config_frame_len);
    t.expect(QMI_node);

    // Inject: QMI_PDC_LOAD_CONFIG_RESP
    QMI_node.update_default_qmi_pdc_load_config_resp();
    t.inject(QMI_node);

    // Inject: QMI_PDC_LOAD_CONFIG_IND
    QMI_node.update_default_qmi_pdc_load_config_ind();
    t.inject(QMI_node);
  }
  t.set_end();
  fclose(mbn_file_ptr);
}

void util_ril_utf_qmi_pdc_set_selected_config(Testcase &t, uint8_t* config_id, pdc_config_type_enum_v01 type, int rild_num)
{
  QMIRILTestnode QMI_node;
  uint32_t config_id_len = strlen((char *)config_id);

  t.set_start();
  // Expect: QMI_PDC_SET_SELECTED_CONFIG_REQ
  QMI_node.update_default_qmi_pdc_set_selected_config_req();
  QMI_node.set_qmi_pdc_set_selected_config_req_config_id(config_id, config_id_len);
  QMI_node.set_qmi_pdc_set_selected_config_req_config_type(type);
  if (type)
  {
    QMI_node.set_qmi_pdc_set_selected_config_req_subscription_id(rild_num);
  }
  t.expect(QMI_node);

  // Inject: QMI_PDC_SET_SELECTED_CONFIG_RESP
  QMI_node.update_default_qmi_pdc_set_selected_config_resp();
  t.inject(QMI_node);

  // Inject: QMI_PDC_SET_SELECTED_CONFIG_IND
  QMI_node.update_default_qmi_pdc_set_selected_config_ind();
  t.inject(QMI_node);
  t.set_end();
}

void util_ril_utf_qmi_pdc_activate_config(Testcase &t, pdc_config_type_enum_v01 type, int rild_num, int n_pendings)
{
  QMIRILTestnode QMI_node;

  t.set_start();
  // Expect: QMI_PDC_ACTIVATE_CONFIG_REQ
  QMI_node.update_default_qmi_pdc_activate_config_req();
  QMI_node.update_qmi_pdc_activate_config_req_config_type(type);
  if (type)
  {
    QMI_node.update_qmi_pdc_activate_config_req_subscription_id(rild_num);
  }
  else
  {
    // for HW mbns, force the pending MBN as 1
    n_pendings = 1;
  }
  t.expect(QMI_node);

  // Inject: QMI_PDC_ACTIVATE_CONFIG_RESP
  QMI_node.update_default_qmi_pdc_activate_config_resp();
  t.inject(QMI_node);

  for (int i = 0; i < n_pendings; i++)
  {
    // Inject: QMI_PDC_ACTIVATE_CONFIG_IND
    QMI_node.update_default_qmi_pdc_activate_config_ind();
    t.inject(QMI_node);
  }
  t.set_end();
}

void util_ril_utf_qmi_pdc_cleanup_config(Testcase &t, pdc_config_type_enum_v01 type, int rild_num)
{
  QMIRILTestnode QMI_node;

  t.set_start();
  // Expect: QMI_PDC_DELETE_CONFIG_REQ
  QMI_node.update_default_qmi_pdc_delete_config_req();
  QMI_node.set_qmi_pdc_delete_config_req_config_id(NULL, 0);
  QMI_node.set_qmi_pdc_delete_config_req_config_type(type);
  t.expect(QMI_node);

  // Inject: QMI_PDC_DELETE_CONFIG_RESP
  QMI_node.update_default_qmi_pdc_delete_config_resp();
  t.inject(QMI_node);

  // Inject: QMI_PDC_DELETE_CONFIG_IND
  QMI_node.update_default_qmi_pdc_delete_config_ind();
  t.inject(QMI_node);

  t.set_end();
}

void util_ril_utf_qmi_pdc_get_config_info(Testcase &t, uint8_t* config_id)
{
  QMIRILTestnode QMI_node;
  uint32_t config_id_len = strlen((char *)config_id);

  t.set_start();
  // Expect: QMI_PDC_GET_CONFIG_INFO_REQ
  QMI_node.update_default_qmi_pdc_get_config_info_req();
  QMI_node.set_qmi_pdc_get_config_info_req_config_type(PDC_CONFIG_TYPE_MODEM_SW_V01);
  QMI_node.set_qmi_pdc_get_config_info_req_config_id(config_id, config_id_len);
  t.expect(QMI_node);

  // Inject: QMI_PDC_GET_CONFIG_INFO_RESP
  QMI_node.update_default_qmi_pdc_get_config_info_resp();
  t.inject(QMI_node);

  // Inject: QMI_PDC_GET_CONFIG_INFO_IND
  QMI_node.update_default_qmi_pdc_get_config_info_ind();
  t.inject(QMI_node);
  t.set_end();
}

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
)
{
  t.set_start();

  // Inject: QMI_IMSA_SERVICE_STATUS_IND_V01
  QMI_node.update_default_qmi_imsa_service_status_ind_v01();
  QMI_node.set_qmi_imsa_service_status_ind_v01_sms_service_status(sms_status);
  QMI_node.set_qmi_imsa_service_status_ind_v01_voip_service_status(voip_status);
  QMI_node.set_qmi_imsa_service_status_ind_v01_vt_service_status(vt_status);
  QMI_node.set_qmi_imsa_service_status_ind_v01_sms_service_rat(sms_rat);
  QMI_node.set_qmi_imsa_service_status_ind_v01_voip_service_rat(voip_rat);
  QMI_node.set_qmi_imsa_service_status_ind_v01_vt_service_rat(vt_rat);
  t.inject(QMI_node);

  // Expect: IMS__MSG_ID__UNSOL_SRV_STATUS_UPDATE
  IMS_node.update_default_ims_unsol_srv_status_update(5);
  IMS_node.set_ims_unsol_srv_status_update_sms_service(0, 2, 1, 0);
  IMS_node.set_ims_unsol_srv_status_update_voip_service(1, 2, 1, 0);
  IMS_node.set_ims_unsol_srv_status_update_vt_service(2, 2, 1, 1);
  t.expect(IMS_node);

  t.set_end();
}

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
)
{
  t.set_start();

  // Inject: QMI_IMSA_REGISTRATION_STATUS_IND_V01
  QMI_node.update_default_qmi_imsa_registration_status_ind();
  QMI_node.set_qmi_imsa_registration_status_ind_ims_registered(ims_registered);
  QMI_node.set_qmi_imsa_registration_status_ind_ims_registration_failure_error_code(ims_registration_failure_error_code_valid,
    ims_registration_failure_error_code);
  QMI_node.set_qmi_imsa_registration_status_ind_ims_reg_status(ims_reg_status_valid, ims_reg_status);
  QMI_node.set_qmi_imsa_registration_status_ind_registration_error_string(registration_error_string_valid, registration_error_string);
  t.inject(QMI_node);

  // Expect: IMS__MSG_ID__UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED
  IMS_node.update_default_ims_unsol_response_ims_network_state_changed();
  IMS_node.set_ims_unsol_response_ims_network_state_changed_reg_state(has_state, state);
  IMS_node.set_ims_unsol_response_ims_network_state_changed_errorcode(ims_registration_failure_error_code_valid, ims_registration_failure_error_code,
    registration_error_string);
  t.expect(IMS_node);

  t.set_end();
}

void util_to_get_operator_name_3gpp_oprtr_mcc_mnc(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, const char *mcc_ptr, const char *mnc_ptr, uint8_t csg_id_valid, uint32_t csg_id, nas_radio_if_enum_v01 rat, const char *modem_long_plmn_name, const char * modem_short_plmn_name, const char *atel_long_plmn_name, const char * atel_short_plmn_name, uint8_t is_fill_eons_needed, uint8_t is_default_oprtr_name, uint8_t is_always_send_plmn_name, uint8_t is_se_table)
{
    char tags[MAX_TAG_SIZE];
    uint16_t mcc = 0;
    uint16_t mnc = 0;

    if(NULL != mcc_ptr)
    {
        mcc = atoi(mcc_ptr);
    }

    if(NULL != mnc_ptr)
    {
        mnc = atoi(mnc_ptr);
    }

    t.set_start();

    // Inject: RIL_REQUEST_OPERATOR
    RIL_node.update_default_ril_request_operator();
    t.inject(RIL_node);

    // Expect: QMI_NAS_GET_PLMN_NAME_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_plmn_name_req_msg(mcc, mnc);
    QMI_node.set_qmi_nas_get_plmn_name_req_suppress_sim_error(TRUE);
    QMI_node.set_qmi_nas_get_plmn_name_req_rat(rat);
    memset(tags,'\0', MAX_TAG_SIZE);
    if(csg_id_valid)
    {
        QMI_node.set_qmi_nas_get_plmn_name_req_csg_id(csg_id);
    }
    QMI_node.add_callflow_tag(tags);
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_PLMN_NAME_RESP_MSG_V01
    QMI_node.update_default_qmi_nas_plmn_name_resp_msg(QMI_RESULT_SUCCESS_V01, (qmi_error_type_v01)0);
    memset(tags,'\0', MAX_TAG_SIZE);
    if(is_default_oprtr_name)
    {
        nas_3gpp_eons_plmn_name_type_v01* eons_plmn_name = QMI_node.set_qmi_nas_get_plmn_name_resp_eons_plmn_name_3gpp();
        if(NULL != modem_long_plmn_name)
        {
            eons_plmn_name->plmn_long_name_len = strlen(modem_long_plmn_name);
            strcpy(eons_plmn_name->plmn_long_name, modem_long_plmn_name);
            strcat(tags, "Long PLMN = ");
            strcat(tags, modem_long_plmn_name);
        }

        if(NULL != modem_short_plmn_name)
        {
            eons_plmn_name->plmn_short_name_len = strlen(modem_short_plmn_name);
            strcpy(eons_plmn_name->plmn_short_name, modem_short_plmn_name);
            strcat(tags, "Short PLMN = ");
            strcat(tags, modem_short_plmn_name);
        }
    }
    else
    {
        snprintf(tags, MAX_TAG_SIZE, "is_default_oprtr_name = %d",is_default_oprtr_name);
    }
    QMI_node.add_callflow_tag(tags);
    t.inject(QMI_node);

    if(!is_default_oprtr_name)
    {
        // Expect: QMI_NAS_GET_PLMN_NAME_REQ_MSG_V01
        QMI_node.update_default_qmi_nas_get_plmn_name_req_msg(mcc, mnc);
        QMI_node.set_qmi_nas_get_plmn_name_req_suppress_sim_error(TRUE);
        QMI_node.set_qmi_nas_get_plmn_name_req_always_send_plmn_name(TRUE);
        QMI_node.set_qmi_nas_get_plmn_name_req_rat(rat);
        if(csg_id_valid)
        {
            QMI_node.set_qmi_nas_get_plmn_name_req_csg_id(csg_id);
        }
        memset(tags,'\0', MAX_TAG_SIZE);
        snprintf(tags, MAX_TAG_SIZE, "MCC = %s, MNC = %s, RAT = %d, CSG_ID_VALID = %d",
                mcc_ptr, mnc_ptr, rat, csg_id_valid);
        QMI_node.add_callflow_tag(tags);
        t.expect(QMI_node);

        // Inject: QMI_NAS_GET_PLMN_NAME_RESP_MSG_V01
        QMI_node.update_default_qmi_nas_plmn_name_resp_msg(QMI_RESULT_SUCCESS_V01, (qmi_error_type_v01)0);
        memset(tags,'\0', MAX_TAG_SIZE);
        if(is_always_send_plmn_name)
        {
            nas_3gpp_eons_plmn_name_type_v01* eons_plmn_name = QMI_node.set_qmi_nas_get_plmn_name_resp_eons_plmn_name_3gpp();
            if(NULL != modem_long_plmn_name)
            {
                eons_plmn_name->plmn_long_name_len = strlen(modem_long_plmn_name);
                strcpy(eons_plmn_name->plmn_long_name, modem_long_plmn_name);
                strcat(tags, "Long PLMN = ");
                strcat(tags, modem_long_plmn_name);
            }

            if(NULL != modem_short_plmn_name)
            {
                eons_plmn_name->plmn_short_name_len = strlen(modem_short_plmn_name);
                strcpy(eons_plmn_name->plmn_short_name, modem_short_plmn_name);
                strcat(tags, "Short PLMN = ");
                strcat(tags, modem_short_plmn_name);
            }
        }
        else
        {
            snprintf(tags, MAX_TAG_SIZE, "is_always_send_plmn_name = %d",is_always_send_plmn_name);
        }
        QMI_node.add_callflow_tag(tags);
        t.inject(QMI_node);

        if(!is_always_send_plmn_name)
        {
            // Expect: QMI_NAS_GET_PLMN_NAME_REQ_MSG_V01
            QMI_node.update_default_qmi_nas_get_plmn_name_req_msg(mcc, mnc);
            QMI_node.set_qmi_nas_get_plmn_name_req_suppress_sim_error(TRUE);
            QMI_node.set_qmi_nas_get_plmn_name_req_use_static_table_only(TRUE);
            QMI_node.set_qmi_nas_get_plmn_name_req_rat(rat);
            if(csg_id_valid)
            {
                QMI_node.set_qmi_nas_get_plmn_name_req_csg_id(csg_id);
            }
            memset(tags,'\0', MAX_TAG_SIZE);
            snprintf(tags, MAX_TAG_SIZE, "MCC = %s, MNC = %s, RAT = %d, CSG_ID_VALID = %d", mcc_ptr, mnc_ptr, rat, csg_id_valid);
            QMI_node.add_callflow_tag(tags);
            t.expect(QMI_node);

            // Inject: QMI_NAS_GET_PLMN_NAME_RESP_MSG_V01
            QMI_node.update_default_qmi_nas_plmn_name_resp_msg(QMI_RESULT_SUCCESS_V01, (qmi_error_type_v01)0);
            memset(tags,'\0', MAX_TAG_SIZE);
            if(is_se_table)
            {
                nas_3gpp_eons_plmn_name_type_v01* eons_plmn_name = QMI_node.set_qmi_nas_get_plmn_name_resp_eons_plmn_name_3gpp();
                if(NULL != modem_long_plmn_name)
                {
                    eons_plmn_name->plmn_long_name_len = strlen(modem_long_plmn_name);
                    strcpy(eons_plmn_name->plmn_long_name, modem_long_plmn_name);
                    strcat(tags, "Long PLMN = ");
                    strcat(tags, modem_long_plmn_name);
                }

                if(NULL != modem_short_plmn_name)
                {
                    eons_plmn_name->plmn_short_name_len = strlen(modem_short_plmn_name);
                    strcpy(eons_plmn_name->plmn_short_name, modem_short_plmn_name);
                    strcat(tags, "Short PLMN = ");
                    strcat(tags, modem_short_plmn_name);
                }
            }
            else
            {
                snprintf(tags, MAX_TAG_SIZE, "is_se_table = %d", is_se_table);
            }
            QMI_node.add_callflow_tag(tags);
            t.inject(QMI_node);
        }
    }

    // Expect: RIL_REQUEST_OPERATOR_RESP
    ril_request_operator_resp_from_qcril_t* ril_req_operator_name = RIL_node.update_default_ril_request_operator_resp();
    if(NULL != atel_long_plmn_name)
    {
        strcpy(ril_req_operator_name->operator_info_array[0],atel_long_plmn_name);
    }
    else if( (NULL != atel_short_plmn_name) && (is_fill_eons_needed) )
    {
        strcpy(ril_req_operator_name->operator_info_array[0],atel_short_plmn_name);
    }

    if(NULL != atel_short_plmn_name)
    {
        strcpy(ril_req_operator_name->operator_info_array[1],atel_short_plmn_name);
    }
    else if ( (NULL != atel_long_plmn_name) && (is_fill_eons_needed) )
    {
        strcpy(ril_req_operator_name->operator_info_array[1],ril_req_operator_name->operator_info_array[0]);
    }

    snprintf( ril_req_operator_name->operator_info_array[2], 16, "%s%s", mcc_ptr, mnc_ptr );
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "long_name = %s, short_name = %s, mccmnc = %s",
            ril_req_operator_name->operator_info_array[0],
            ril_req_operator_name->operator_info_array[1],
            ril_req_operator_name->operator_info_array[2]);
    RIL_node.add_callflow_tag(tags);
    t.expect(RIL_node);

    t.set_end();
}

void util_ril_utf_dms_event_ind_info(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, uint8_t max_active_data_subscriptions_valid, uint8_t max_active_data_subscriptions)
{
    char tags[MAX_TAG_SIZE];
    NOTUSED(RIL_node);
    t.set_start();

    // Inject: QMI_DMS_EVENT_REPORT_IND_V01
    QMI_node.update_default_qmi_dms_event_report_ind_v01();
    memset(tags,'\0', MAX_TAG_SIZE);
    snprintf(tags, MAX_TAG_SIZE, "Max_active_data_Sub_valid = %d, Max_active_data_Sub=%d",
            max_active_data_subscriptions_valid, max_active_data_subscriptions);
    QMI_node.add_callflow_tag(tags);
    QMI_node.set_qmi_dms_event_report_ind_v01_max_active_data_subscriptions(max_active_data_subscriptions_valid, max_active_data_subscriptions);
    t.inject(QMI_node);

    t.set_end();
}

void util_ril_utf_get_device_cap_info(Testcase &t, QMIRILTestnode &QMI_node, AndroidTelTestnode &RIL_node, OEMTestnode &OEM_node, uint8_t max_active_data_subscriptions_valid, uint8_t max_active_data_subscriptions, uint8_t is_max_data_change)
{
    // Expect: QMI_DMS_GET_DEVICE_CAP_REQ_V01
    NOTUSED(RIL_node);
    QMI_node.update_default_qmi_dms_get_device_cap_req_v01();
    t.expect(QMI_node);

    // Inject: QMI_DMS_GET_DEVICE_CAP_RESP_V01
    QMI_node.update_default_qmi_dms_get_device_cap_resp_v01();
    QMI_node.set_default_qmi_dms_get_device_cap_resp_v01_max_active_data_subscriptions(max_active_data_subscriptions_valid, max_active_data_subscriptions);
    QMI_node.set_qmi_dms_get_device_cap_resp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    if(is_max_data_change)
    {
        // Expect: QCRIL_EVT_HOOK_UNSOL_MAX_DATA_CHANGE_IND
        OEM_node.update_default_qcril_evt_hook_unsol_max_data_sub_change();
        OEM_node.update_qcril_evt_hook_unsol_with_max_data_sub(max_active_data_subscriptions);
        t.expect(OEM_node);

        // Expect: RIL_UNSOL_OEM_HOOK_RAW
        RIL_node.update_default_ril_unsol_oem_hook_raw();
        RIL_node.set_expect_skip_verification_status(false);
        t.expect(RIL_node);
    }
}

void util_ril_utf_physical_channel_configuration_query_sequence(
        Testcase &t, QMIRILTestnode &QMI_node) {
    // Expect: QMI_NAS_GET_LTE_CPHY_CA_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_lte_cphy_ca_info_req_msg();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_LTE_CPHY_CA_INFO_RESP_MSG_V01
    QMI_node.update_default_nas_get_lte_cphy_ca_info_resp_msg();
    t.inject(QMI_node);

    // Expect: QMI_NAS_GET_RF_BAND_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_rf_band_info_req_msg();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_RF_BAND_INFO_RESP_MSG_V01
    QMI_node.update_default_qmi_nas_get_rf_band_info_resp_msg();
    t.inject(QMI_node);

    // Expect: QMI_NAS_GET_CELL_LOCATION_INFO_REQ_MSG_V01
    QMI_node.update_default_qmi_nas_get_cell_location_info_req_msg();
    t.expect(QMI_node);

    // Inject: QMI_NAS_GET_CELL_LOCATION_INFO_RESP_MSG_V01
    QMI_node.update_default_qmi_nas_get_cell_location_info_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);
}

