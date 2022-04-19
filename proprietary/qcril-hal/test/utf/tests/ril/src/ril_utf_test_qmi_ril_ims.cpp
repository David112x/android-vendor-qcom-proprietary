/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_test_qmi_ril_ims.cpp
  @brief   RIL UTF test case definitions

  DESCRIPTION
  All test case definitions should be in this file
  ---------------------------------------------------------------------------
******************************************************************************/

#include "ril_utf_test_qmi_ril.h"
#include "ril_utf_test_qmi_ril_L.h"
#include "imsIF.pb.h"

extern uint8_t imss_0x008F_supported_fields[QMI_GET_SUPPORTED_FIELDS_ARRAY_MAX_V01];

/**==
@testname: ril_utf_ims_service_enable_config_ind_001
@description:
==**/
test_result ril_utf_ims_service_enable_config_ind_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  ims_settings_omadm_services_type_mask_v01 omadm_mask =
    (IMS_SETTINGS_OMADM_SERVICES_BIT_MASK_NONE_V01 |
     IMS_SETTINGS_OMADM_SERVICES_ENABLE_BIT_MASK_V01 |
     IMS_SETTINGS_OMADM_SERVICES_VOLTE_ENABLE_BIT_MASK_V01 |
     IMS_SETTINGS_OMADM_SERVICES_VT_ENABLE_BIT_MASK_V01 |
     IMS_SETTINGS_OMADM_SERVICES_VOWIFI_ENABLE_BIT_MASK_V01 |
     IMS_SETTINGS_OMADM_SERVICES_PRESENCE_ENABLE_BIT_MASK_V01);


  // Inject: QMI_IMS_SETTINGS_IMS_SERVICE_ENABLE_CONFIG_IND_V01
  ims_settings_ims_service_enable_config_ind_msg_v01 *ind_msg = QMI_node.update_default_qmi_ims_settings_ims_service_enable_config_ind_msg();
  ind_msg->service_mask_by_network_enabled_valid = TRUE;
  ind_msg->service_mask_by_network_enabled = omadm_mask;
  ind_msg->wifi_calling_enabled_valid = TRUE;
  ind_msg->wifi_calling_enabled = TRUE;
  ind_msg->call_mode_preference_ext_valid = TRUE;
  ind_msg->call_mode_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_CELLULAR_ONLY_V01;
  t.inject(QMI_node);

  // Inject: QMI_IMS_SETTINGS_IMS_SERVICE_ENABLE_CONFIG_IND_V01
  ind_msg = QMI_node.update_default_qmi_ims_settings_ims_service_enable_config_ind_msg();
  ind_msg->service_mask_by_network_enabled_valid = TRUE;
  ind_msg->service_mask_by_network_enabled = omadm_mask;
  ind_msg->wifi_calling_enabled_valid = TRUE;
  ind_msg->wifi_calling_enabled = TRUE;
  ind_msg->call_mode_preference_ext_valid = TRUE;
  ind_msg->call_mode_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_CELLULAR_V01;
  t.inject(QMI_node);

  // Inject: QMI_IMS_SETTINGS_IMS_SERVICE_ENABLE_CONFIG_IND_V01
  ind_msg = QMI_node.update_default_qmi_ims_settings_ims_service_enable_config_ind_msg();
  ind_msg->service_mask_by_network_enabled_valid = TRUE;
  ind_msg->service_mask_by_network_enabled = omadm_mask;
  ind_msg->wifi_calling_enabled_valid = TRUE;
  ind_msg->wifi_calling_enabled = TRUE;
  ind_msg->call_mode_preference_ext_valid = TRUE;
  ind_msg->call_mode_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_WIFI_V01;
  t.inject(QMI_node);

  // Inject: QMI_IMS_SETTINGS_IMS_SERVICE_ENABLE_CONFIG_IND_V01
  ind_msg = QMI_node.update_default_qmi_ims_settings_ims_service_enable_config_ind_msg();
  ind_msg->service_mask_by_network_enabled_valid = TRUE;
  ind_msg->service_mask_by_network_enabled = omadm_mask;
  ind_msg->wifi_calling_enabled_valid = TRUE;
  ind_msg->wifi_calling_enabled = TRUE;
  ind_msg->call_mode_preference_ext_valid = TRUE;
  ind_msg->call_mode_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_IMS_V01;
  t.inject(QMI_node);

  // Inject: QMI_IMS_SETTINGS_IMS_SERVICE_ENABLE_CONFIG_IND_V01
  ind_msg = QMI_node.update_default_qmi_ims_settings_ims_service_enable_config_ind_msg();
  ind_msg->service_mask_by_network_enabled_valid = TRUE;
  ind_msg->service_mask_by_network_enabled = omadm_mask;
  ind_msg->wifi_calling_enabled_valid = TRUE;
  ind_msg->wifi_calling_enabled = TRUE;
  ind_msg->call_mode_preference_ext_valid = TRUE;
  ind_msg->call_mode_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_WLAN_ONLY_V01;
  t.inject(QMI_node);

  // Inject: QMI_IMS_SETTINGS_IMS_SERVICE_ENABLE_CONFIG_IND_V01
  ind_msg = QMI_node.update_default_qmi_ims_settings_ims_service_enable_config_ind_msg();
  ind_msg->service_mask_by_network_enabled_valid = TRUE;
  ind_msg->service_mask_by_network_enabled = omadm_mask;
  ind_msg->wifi_calling_enabled_valid = TRUE;
  ind_msg->wifi_calling_enabled = FALSE;
  ind_msg->call_mode_preference_ext_valid = TRUE;
  ind_msg->call_mode_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_NONE_V01;
  t.inject(QMI_node);

  // Inject: QMI_IMS_SETTINGS_IMS_SERVICE_ENABLE_CONFIG_IND_V01
  ind_msg = QMI_node.update_default_qmi_ims_settings_ims_service_enable_config_ind_msg();
  ind_msg->service_mask_by_network_enabled_valid = TRUE;
  ind_msg->service_mask_by_network_enabled = omadm_mask;
  ind_msg->wifi_calling_enabled_valid = TRUE;
  ind_msg->wifi_calling_enabled = 140;
  ind_msg->call_mode_preference_ext_valid = TRUE;
  ind_msg->call_mode_preference_ext = (ims_settings_call_mode_option_enum_v01)100;;
  t.inject(QMI_node);



  ims_ConfigMsg* ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VLT_SETTING_ENABLED;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 0;
  t.inject(IMS_node);

  ims_settings_set_ims_service_enable_config_req_msg_v01 *qmi_req = QMI_node.update_default_qmi_ims_settings_set_ims_service_enable_config_req();
  qmi_req->service_mask_by_network_enabled_valid = TRUE;
  qmi_req->service_mask_by_network_enabled =
      (omadm_mask & ~(IMS_SETTINGS_OMADM_SERVICES_VOLTE_ENABLE_BIT_MASK_V01));
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_ims_service_enable_config_resp(
      QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VLT_SETTING_ENABLED;
  t.expect(IMS_node);


  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_service_enable_config_ind_old_002
@description:
==**/
test_result ril_utf_ims_service_enable_config_ind_old_002(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  // disable support for call_mode_preference_ext
  imss_0x008F_supported_fields[2] = 0x00;

  t.set_test_id(__func__);
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  ims_settings_omadm_services_type_mask_v01 omadm_mask =
    (IMS_SETTINGS_OMADM_SERVICES_BIT_MASK_NONE_V01 |
     IMS_SETTINGS_OMADM_SERVICES_ENABLE_BIT_MASK_V01 |
     IMS_SETTINGS_OMADM_SERVICES_VOLTE_ENABLE_BIT_MASK_V01 |
     IMS_SETTINGS_OMADM_SERVICES_VT_ENABLE_BIT_MASK_V01 |
     IMS_SETTINGS_OMADM_SERVICES_VOWIFI_ENABLE_BIT_MASK_V01 |
     IMS_SETTINGS_OMADM_SERVICES_PRESENCE_ENABLE_BIT_MASK_V01);


  // Inject: QMI_IMS_SETTINGS_IMS_SERVICE_ENABLE_CONFIG_IND_V01
  ims_settings_ims_service_enable_config_ind_msg_v01 *ind_msg = QMI_node.update_default_qmi_ims_settings_ims_service_enable_config_ind_msg();
  ind_msg->service_mask_by_network_enabled_valid = TRUE;
  ind_msg->service_mask_by_network_enabled = omadm_mask;
  ind_msg->wifi_calling_enabled_valid = TRUE;
  ind_msg->wifi_calling_enabled = TRUE;
  ind_msg->call_mode_preference_valid = TRUE;
  ind_msg->call_mode_preference = IMS_SETTINGS_CALL_MODE_PREFERRED_CELLULAR_ONLY_V01;
  t.inject(QMI_node);

  // Inject: QMI_IMS_SETTINGS_IMS_SERVICE_ENABLE_CONFIG_IND_V01
  ind_msg = QMI_node.update_default_qmi_ims_settings_ims_service_enable_config_ind_msg();
  ind_msg->service_mask_by_network_enabled_valid = TRUE;
  ind_msg->service_mask_by_network_enabled = omadm_mask;
  ind_msg->wifi_calling_enabled_valid = TRUE;
  ind_msg->wifi_calling_enabled = TRUE;
  ind_msg->call_mode_preference_valid = TRUE;
  ind_msg->call_mode_preference = IMS_SETTINGS_CALL_MODE_PREFERRED_CELLULAR_V01;
  t.inject(QMI_node);

  // Inject: QMI_IMS_SETTINGS_IMS_SERVICE_ENABLE_CONFIG_IND_V01
  ind_msg = QMI_node.update_default_qmi_ims_settings_ims_service_enable_config_ind_msg();
  ind_msg->service_mask_by_network_enabled_valid = TRUE;
  ind_msg->service_mask_by_network_enabled = omadm_mask;
  ind_msg->wifi_calling_enabled_valid = TRUE;
  ind_msg->wifi_calling_enabled = TRUE;
  ind_msg->call_mode_preference_valid = TRUE;
  ind_msg->call_mode_preference = IMS_SETTINGS_CALL_MODE_PREFERRED_WIFI_V01;
  t.inject(QMI_node);

  // Inject: QMI_IMS_SETTINGS_IMS_SERVICE_ENABLE_CONFIG_IND_V01
  ind_msg = QMI_node.update_default_qmi_ims_settings_ims_service_enable_config_ind_msg();
  ind_msg->service_mask_by_network_enabled_valid = TRUE;
  ind_msg->service_mask_by_network_enabled = omadm_mask;
  ind_msg->wifi_calling_enabled_valid = TRUE;
  ind_msg->wifi_calling_enabled = TRUE;
  ind_msg->call_mode_preference_valid = TRUE;
  ind_msg->call_mode_preference = IMS_SETTINGS_CALL_MODE_PREFERRED_IMS_V01;
  t.inject(QMI_node);

  // Inject: QMI_IMS_SETTINGS_IMS_SERVICE_ENABLE_CONFIG_IND_V01
  ind_msg = QMI_node.update_default_qmi_ims_settings_ims_service_enable_config_ind_msg();
  ind_msg->service_mask_by_network_enabled_valid = TRUE;
  ind_msg->service_mask_by_network_enabled = omadm_mask;
  ind_msg->wifi_calling_enabled_valid = TRUE;
  ind_msg->wifi_calling_enabled = TRUE;
  ind_msg->call_mode_preference_valid = TRUE;
  ind_msg->call_mode_preference = IMS_SETTINGS_CALL_MODE_PREFERRED_WLAN_ONLY_V01;
  t.inject(QMI_node);

  // Inject: QMI_IMS_SETTINGS_IMS_SERVICE_ENABLE_CONFIG_IND_V01
  ind_msg = QMI_node.update_default_qmi_ims_settings_ims_service_enable_config_ind_msg();
  ind_msg->service_mask_by_network_enabled_valid = TRUE;
  ind_msg->service_mask_by_network_enabled = omadm_mask;
  ind_msg->wifi_calling_enabled_valid = TRUE;
  ind_msg->wifi_calling_enabled = FALSE;
  ind_msg->call_mode_preference_valid = TRUE;
  ind_msg->call_mode_preference = IMS_SETTINGS_CALL_MODE_PREFERRED_NONE_V01;
  t.inject(QMI_node);

  // Inject: QMI_IMS_SETTINGS_IMS_SERVICE_ENABLE_CONFIG_IND_V01
  ind_msg = QMI_node.update_default_qmi_ims_settings_ims_service_enable_config_ind_msg();
  ind_msg->service_mask_by_network_enabled_valid = TRUE;
  ind_msg->service_mask_by_network_enabled = omadm_mask;
  ind_msg->wifi_calling_enabled_valid = TRUE;
  ind_msg->wifi_calling_enabled = 140;
  ind_msg->call_mode_preference_valid = TRUE;
  ind_msg->call_mode_preference = (ims_settings_call_mode_option_enum_v01)100;;
  t.inject(QMI_node);



  ims_ConfigMsg* ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VLT_SETTING_ENABLED;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 0;
  t.inject(IMS_node);

  ims_settings_set_ims_service_enable_config_req_msg_v01 *qmi_req = QMI_node.update_default_qmi_ims_settings_set_ims_service_enable_config_req();
  qmi_req->service_mask_by_network_enabled_valid = TRUE;
  qmi_req->service_mask_by_network_enabled =
      (omadm_mask & ~(IMS_SETTINGS_OMADM_SERVICES_VOLTE_ENABLE_BIT_MASK_V01));
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_ims_service_enable_config_resp(
      QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VLT_SETTING_ENABLED;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_ims_service_enable_config_old_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  // disable support for call_mode_preference_ext
  imss_0x008F_supported_fields[2] = 0x00;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items ims_service_enable_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_ims_service_enable_config_req_msg_v01 qmiReq;
    ims_settings_get_ims_service_enable_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_ims_service_enable_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_ims_service_enable_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };

  ims_settings_omadm_services_type_mask_v01 omadm_mask =
    (IMS_SETTINGS_OMADM_SERVICES_BIT_MASK_NONE_V01 |
     IMS_SETTINGS_OMADM_SERVICES_ENABLE_BIT_MASK_V01);

  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_MODE;
        req.has_intValue = TRUE;
        req.intValue = 0; // WIFI_ONLY/WLAN_ONLY
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.call_mode_preference_valid = TRUE;
        req.call_mode_preference = IMS_SETTINGS_CALL_MODE_PREFERRED_WLAN_ONLY_V01;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.call_mode_preference_valid = TRUE;
        resp.call_mode_preference = IMS_SETTINGS_CALL_MODE_PREFERRED_WLAN_ONLY_V01;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_MODE;
        req.has_intValue = TRUE;
        req.intValue = 1;  // CELLULAR_PREFERRED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.call_mode_preference_valid = TRUE;
        req.call_mode_preference = IMS_SETTINGS_CALL_MODE_PREFERRED_CELLULAR_V01;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.call_mode_preference_valid = TRUE;
        resp.call_mode_preference = IMS_SETTINGS_CALL_MODE_PREFERRED_CELLULAR_V01;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_MODE;
        req.has_intValue = TRUE;
        req.intValue = 2;  // WIFI_PREFERRED/WLAN_PREFERRED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.call_mode_preference_valid = TRUE;
        req.call_mode_preference = IMS_SETTINGS_CALL_MODE_PREFERRED_WIFI_V01;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.call_mode_preference_valid = TRUE;
        resp.call_mode_preference = IMS_SETTINGS_CALL_MODE_PREFERRED_WIFI_V01;
      }),

     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE;
        req.has_intValue = TRUE;
        req.intValue = 0; // WIFI_ONLY/WLAN_ONLY
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.call_mode_roam_preference_valid = TRUE;
        req.call_mode_roam_preference = IMS_SETTINGS_CALL_MODE_PREFERRED_WLAN_ONLY_V01;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.call_mode_roam_preference_valid = TRUE;
        resp.call_mode_roam_preference = IMS_SETTINGS_CALL_MODE_PREFERRED_WLAN_ONLY_V01;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE;
        req.has_intValue = TRUE;
        req.intValue = 1;  // CELLULAR_PREFERRED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.call_mode_roam_preference_valid = TRUE;
        req.call_mode_roam_preference = IMS_SETTINGS_CALL_MODE_PREFERRED_CELLULAR_V01;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.call_mode_roam_preference_valid = TRUE;
        resp.call_mode_roam_preference = IMS_SETTINGS_CALL_MODE_PREFERRED_CELLULAR_V01;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE;
        req.has_intValue = TRUE;
        req.intValue = 2;  // WIFI_PREFERRED/WLAN_PREFERRED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.call_mode_roam_preference_valid = TRUE;
        req.call_mode_roam_preference = IMS_SETTINGS_CALL_MODE_PREFERRED_WIFI_V01;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.call_mode_roam_preference_valid = TRUE;
        resp.call_mode_roam_preference = IMS_SETTINGS_CALL_MODE_PREFERRED_WIFI_V01;
      }),
  };


  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_set_ims_service_enable_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_ims_service_enable_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.expect(IMS_node);

    // TODO: no support for get ims config for CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE
    if (testReq.imsReq.item != ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE) {

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_ims_service_enable_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_ims_service_enable_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    *ims_ptr = testReq.imsReq;
    t.expect(IMS_node);

    }
  }

  // Failure cases:  IMS preferred is not supported

  ims_ConfigMsg* ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_MODE;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 10;  // IMS_PREFERRED
  t.inject(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 10;  // IMS_PREFERRED
  t.inject(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_config_set_ims_config_001
@description:
==**/
test_result ril_utf_ims_config_set_ims_config_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  /* 500 - invalid value */
  ims_ConfigMsg* ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = (ims_ConfigItem)500;  // Invalid value
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 0x8;
  t.inject(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);


  /* Invalid config_item_type */
  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOCODER_AMRMODESET;
  //ims_ptr->stringValue.arg = strdup("3");
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 0x8;
  t.inject(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  //ims_ptr->has_item = TRUE;
  //ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOCODER_AMRMODESET;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_config_set_ims_config_002
@description:
==**/
test_result ril_utf_ims_config_set_ims_config_002(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  property_set("persist.vendor.radio.disable_modem_cfg", "true");

  /* ims_ConfigItem_CONFIG_ITEM_VOCODER_AMRMODESET */
  ims_ConfigMsg* ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOCODER_AMRMODESET;
  ims_ptr->stringValue.arg = strdup("3");
  t.inject(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOCODER_AMRMODESET;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_config_get_ims_config_001
@description:
==**/
test_result ril_utf_ims_config_get_ims_config_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  /* 500 - invalid value */
  ims_ConfigMsg* ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = (ims_ConfigItem)500;  // Invalid value
  t.inject(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_codec_dynamic_config_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items codec_dynamic_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_codec_dynamic_config_req_msg_v01 qmiReq;
    ims_settings_get_codec_dynamic_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_codec_dynamic_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_codec_dynamic_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };

  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOCODER_AMRMODESET;
        req.stringValue.arg = strdup("3");
      },
      [](ims_settings_set_codec_dynamic_config_req_msg_v01 &req) -> void {
        req.amr_mode_set_mask_valid = TRUE;
        req.amr_mode_set_mask = 0x8;
      },
      [](ims_settings_get_codec_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.amr_mode_set_mask_valid = TRUE;
        resp.amr_mode_set_mask = 0x8;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOCODER_AMRWBMODESET;
        req.stringValue.arg = strdup("3");
      },
      [](ims_settings_set_codec_dynamic_config_req_msg_v01 &req) -> void {
        req.amr_wb_mode_set_mask_valid = TRUE;
        req.amr_wb_mode_set_mask = 0x8;
      },
      [](ims_settings_get_codec_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.amr_wb_mode_set_mask_valid = TRUE;
        resp.amr_wb_mode_set_mask = 0x8;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT;
        req.has_intValue = TRUE;
        req.intValue = 100;
      },
      [](ims_settings_set_codec_dynamic_config_req_msg_v01 &req) -> void {
        req.amr_wb_octet_aligned_dynamic_pt_valid = TRUE;
        req.amr_wb_octet_aligned_dynamic_pt = 100;
      },
      [](ims_settings_get_codec_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.amr_wb_octet_aligned_dynamic_pt_valid = TRUE;
        resp.amr_wb_octet_aligned_dynamic_pt = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_AMR_WB_BANDWIDTH_EFFICIENT_PT;
        req.has_intValue = TRUE;
        req.intValue = 100;
      },
      [](ims_settings_set_codec_dynamic_config_req_msg_v01 &req) -> void {
        req.amr_wb_bandwidth_efficient_dynamic_pt_valid = TRUE;
        req.amr_wb_bandwidth_efficient_dynamic_pt = 100;
      },
      [](ims_settings_get_codec_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.amr_wb_bandwidth_efficient_dynamic_pt_valid = TRUE;
        resp.amr_wb_bandwidth_efficient_dynamic_pt = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_AMR_OCTET_ALIGNED_PT;
        req.has_intValue = TRUE;
        req.intValue = 100;
      },
      [](ims_settings_set_codec_dynamic_config_req_msg_v01 &req) -> void {
        req.amr_octet_aligned_dynamic_pt_valid = TRUE;
        req.amr_octet_aligned_dynamic_pt = 100;
      },
      [](ims_settings_get_codec_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.amr_octet_aligned_dynamic_pt_valid = TRUE;
        resp.amr_octet_aligned_dynamic_pt = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_AMR_BANDWIDTH_EFFICIENT_PT;
        req.has_intValue = TRUE;
        req.intValue = 100;
      },
      [](ims_settings_set_codec_dynamic_config_req_msg_v01 &req) -> void {
        req.amr_bandwidth_efficient_dynamic_pt_valid = TRUE;
        req.amr_bandwidth_efficient_dynamic_pt = 100;
      },
      [](ims_settings_get_codec_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.amr_bandwidth_efficient_dynamic_pt_valid = TRUE;
        resp.amr_bandwidth_efficient_dynamic_pt = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_DTMF_WB_PT;
        req.has_intValue = TRUE;
        req.intValue = 100;
      },
      [](ims_settings_set_codec_dynamic_config_req_msg_v01 &req) -> void {
        req.dtmf_wb_dynamic_pt_valid = TRUE;
        req.dtmf_wb_dynamic_pt = 100;
      },
      [](ims_settings_get_codec_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.dtmf_wb_dynamic_pt_valid = TRUE;
        resp.dtmf_wb_dynamic_pt = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_DTMF_NB_PT;
        req.has_intValue = TRUE;
        req.intValue = 100;
      },
      [](ims_settings_set_codec_dynamic_config_req_msg_v01 &req) -> void {
        req.dtmf_nb_dynamic_pt_valid = TRUE;
        req.dtmf_nb_dynamic_pt = 100;
      },
      [](ims_settings_get_codec_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.dtmf_nb_dynamic_pt_valid = TRUE;
        resp.dtmf_nb_dynamic_pt = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_AMR_DEFAULT_MODE;
        req.has_intValue = TRUE;
        req.intValue = 1;
      },
      [](ims_settings_set_codec_dynamic_config_req_msg_v01 &req) -> void {
        req.amr_wb_default_mode_valid = TRUE;
        req.amr_wb_default_mode = IMS_SETTINGS_AMR_WB_CODEC_MODE_8_V01;
      },
      [](ims_settings_get_codec_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.amr_wb_default_mode_valid = TRUE;
        resp.amr_wb_default_mode = IMS_SETTINGS_AMR_WB_CODEC_MODE_8_V01;
      })
  };


  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_set_codec_dynamic_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_codec_dynamic_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.expect(IMS_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_codec_dynamic_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_codec_dynamic_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    *ims_ptr = testReq.imsReq;
    t.expect(IMS_node);
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_codec_dynamic_config_002(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items codec_dynamic_config: GET FAILURE.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_codec_dynamic_config_req_msg_v01 qmiReq;
    ims_settings_get_codec_dynamic_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_codec_dynamic_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_codec_dynamic_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };

  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOCODER_AMRMODESET;
        req.stringValue.arg = strdup("3");
      },
      [](ims_settings_set_codec_dynamic_config_req_msg_v01 &req) -> void {
        req.amr_mode_set_mask_valid = TRUE;
        req.amr_mode_set_mask = 0x8;
      },
      [](ims_settings_get_codec_dynamic_config_rsp_msg_v01 &resp) -> void {
        //resp.amr_mode_set_mask_valid = TRUE;
        //resp.amr_mode_set_mask = 0x8;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOCODER_AMRWBMODESET;
        req.stringValue.arg = strdup("3");
      },
      [](ims_settings_set_codec_dynamic_config_req_msg_v01 &req) -> void {
        req.amr_wb_mode_set_mask_valid = TRUE;
        req.amr_wb_mode_set_mask = 0x8;
      },
      [](ims_settings_get_codec_dynamic_config_rsp_msg_v01 &resp) -> void {
        //resp.amr_wb_mode_set_mask_valid = TRUE;
        //resp.amr_wb_mode_set_mask = 0x8;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT;
        req.has_intValue = TRUE;
        req.intValue = 100;
      },
      [](ims_settings_set_codec_dynamic_config_req_msg_v01 &req) -> void {
        req.amr_wb_octet_aligned_dynamic_pt_valid = TRUE;
        req.amr_wb_octet_aligned_dynamic_pt = 100;
      },
      [](ims_settings_get_codec_dynamic_config_rsp_msg_v01 &resp) -> void {
        //resp.amr_wb_octet_aligned_dynamic_pt_valid = TRUE;
        //resp.amr_wb_octet_aligned_dynamic_pt = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_AMR_WB_BANDWIDTH_EFFICIENT_PT;
        req.has_intValue = TRUE;
        req.intValue = 100;
      },
      [](ims_settings_set_codec_dynamic_config_req_msg_v01 &req) -> void {
        req.amr_wb_bandwidth_efficient_dynamic_pt_valid = TRUE;
        req.amr_wb_bandwidth_efficient_dynamic_pt = 100;
      },
      [](ims_settings_get_codec_dynamic_config_rsp_msg_v01 &resp) -> void {
        //resp.amr_wb_bandwidth_efficient_dynamic_pt_valid = TRUE;
        //resp.amr_wb_bandwidth_efficient_dynamic_pt = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_AMR_OCTET_ALIGNED_PT;
        req.has_intValue = TRUE;
        req.intValue = 100;
      },
      [](ims_settings_set_codec_dynamic_config_req_msg_v01 &req) -> void {
        req.amr_octet_aligned_dynamic_pt_valid = TRUE;
        req.amr_octet_aligned_dynamic_pt = 100;
      },
      [](ims_settings_get_codec_dynamic_config_rsp_msg_v01 &resp) -> void {
        //resp.amr_octet_aligned_dynamic_pt_valid = TRUE;
        //resp.amr_octet_aligned_dynamic_pt = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_AMR_BANDWIDTH_EFFICIENT_PT;
        req.has_intValue = TRUE;
        req.intValue = 100;
      },
      [](ims_settings_set_codec_dynamic_config_req_msg_v01 &req) -> void {
        req.amr_bandwidth_efficient_dynamic_pt_valid = TRUE;
        req.amr_bandwidth_efficient_dynamic_pt = 100;
      },
      [](ims_settings_get_codec_dynamic_config_rsp_msg_v01 &resp) -> void {
        //resp.amr_bandwidth_efficient_dynamic_pt_valid = TRUE;
        //resp.amr_bandwidth_efficient_dynamic_pt = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_DTMF_WB_PT;
        req.has_intValue = TRUE;
        req.intValue = 100;
      },
      [](ims_settings_set_codec_dynamic_config_req_msg_v01 &req) -> void {
        req.dtmf_wb_dynamic_pt_valid = TRUE;
        req.dtmf_wb_dynamic_pt = 100;
      },
      [](ims_settings_get_codec_dynamic_config_rsp_msg_v01 &resp) -> void {
        //resp.dtmf_wb_dynamic_pt_valid = TRUE;
        //resp.dtmf_wb_dynamic_pt = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_DTMF_NB_PT;
        req.has_intValue = TRUE;
        req.intValue = 100;
      },
      [](ims_settings_set_codec_dynamic_config_req_msg_v01 &req) -> void {
        req.dtmf_nb_dynamic_pt_valid = TRUE;
        req.dtmf_nb_dynamic_pt = 100;
      },
      [](ims_settings_get_codec_dynamic_config_rsp_msg_v01 &resp) -> void {
        //resp.dtmf_nb_dynamic_pt_valid = TRUE;
        //resp.dtmf_nb_dynamic_pt = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_AMR_DEFAULT_MODE;
        req.has_intValue = TRUE;
        req.intValue = 1;
      },
      [](ims_settings_set_codec_dynamic_config_req_msg_v01 &req) -> void {
        req.amr_wb_default_mode_valid = TRUE;
        req.amr_wb_default_mode = IMS_SETTINGS_AMR_WB_CODEC_MODE_8_V01;
      },
      [](ims_settings_get_codec_dynamic_config_rsp_msg_v01 &resp) -> void {
        //resp.amr_wb_default_mode_valid = TRUE;
        //resp.amr_wb_default_mode = IMS_SETTINGS_AMR_WB_CODEC_MODE_8_V01;
      })
  };


  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_set_codec_dynamic_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_codec_dynamic_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.expect(IMS_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_codec_dynamic_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_codec_dynamic_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    //*ims_ptr = testReq.imsReq;
    IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
    t.expect(IMS_node);
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_codec_dynamic_config_003(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items codec_dynamic_config: GET FAILURE.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 100;
  t.inject(IMS_node);

  ims_settings_set_codec_dynamic_config_req_msg_v01 *set_req =
    QMI_node.update_default_qmi_ims_settings_set_codec_dynamic_config_req();
  set_req->amr_wb_octet_aligned_dynamic_pt_valid = TRUE;
  set_req->amr_wb_octet_aligned_dynamic_pt = 100;
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_codec_dynamic_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 100;
  t.inject(IMS_node);

  set_req = QMI_node.update_default_qmi_ims_settings_set_codec_dynamic_config_req();
  set_req->amr_wb_octet_aligned_dynamic_pt_valid = TRUE;
  set_req->amr_wb_octet_aligned_dynamic_pt = 100;
  t.expect(QMI_node);

  ims_settings_set_codec_dynamic_config_rsp_msg_v01 *set_resp =
    QMI_node.update_default_qmi_ims_settings_set_codec_dynamic_config_resp(
        QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 100;
  t.inject(IMS_node);

  set_req = QMI_node.update_default_qmi_ims_settings_set_codec_dynamic_config_req();
  set_req->amr_wb_octet_aligned_dynamic_pt_valid = TRUE;
  set_req->amr_wb_octet_aligned_dynamic_pt = 100;
  t.expect(QMI_node);

  set_resp = QMI_node.update_default_qmi_ims_settings_set_codec_dynamic_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  set_resp->settings_resp_valid = TRUE;
  set_resp->settings_resp = IMS_SETTINGS_MSG_WRITE_FAILED_V01;
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT;
  ims_ptr->has_errorCause = TRUE;
  ims_ptr->errorCause = ims_ConfigFailureCause_CONFIG_WRITE_FAILED;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);


  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_codec_dynamic_config_req();
  t.expect(QMI_node);

  ims_settings_get_codec_dynamic_config_rsp_msg_v01 *get_resp =
    QMI_node.update_default_qmi_ims_settings_get_codec_dynamic_config_resp(
        QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_codec_dynamic_config_req();
  t.expect(QMI_node);

  get_resp = QMI_node.update_default_qmi_ims_settings_get_codec_dynamic_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_codec_dynamic_config_req();
  t.expect(QMI_node);

  get_resp = QMI_node.update_default_qmi_ims_settings_get_codec_dynamic_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  get_resp->settings_resp_valid = TRUE;
  get_resp->settings_resp = IMS_SETTINGS_MSG_READ_FAILED_V01;
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT;
  ims_ptr->has_errorCause = TRUE;
  ims_ptr->errorCause = ims_ConfigFailureCause_CONFIG_READ_FAILED;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_codec_dynamic_config_004(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items codec_dynamic_config: FAILURE.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_codec_dynamic_config_req_msg_v01 qmiReq;
    ims_settings_get_codec_dynamic_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_codec_dynamic_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_codec_dynamic_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };

  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOCODER_AMRMODESET;
        // Expected STRING but pass INT
        //req.stringValue.arg = strdup("3");
        req.has_intValue = TRUE;
        req.intValue = 100;
      },
      [](ims_settings_set_codec_dynamic_config_req_msg_v01 &req) -> void {
        req.amr_mode_set_mask_valid = FALSE;
        //req.amr_mode_set_mask = 0x8;
      },
      [](ims_settings_get_codec_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.amr_mode_set_mask_valid = FALSE;
        //resp.amr_mode_set_mask = 0x8;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOCODER_AMRWBMODESET;
        // Expected STRING but pass INT
        //req.stringValue.arg = strdup("3");
        req.has_intValue = TRUE;
        req.intValue = 100;
      },
      [](ims_settings_set_codec_dynamic_config_req_msg_v01 &req) -> void {
        req.amr_wb_mode_set_mask_valid = FALSE;
        //req.amr_wb_mode_set_mask = 0x8;
      },
      [](ims_settings_get_codec_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.amr_wb_mode_set_mask_valid = FALSE;
        //resp.amr_wb_mode_set_mask = 0x8;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT;
        req.has_intValue = TRUE;
        req.intValue = UINT16_MAX+100;
      },
      [](ims_settings_set_codec_dynamic_config_req_msg_v01 &req) -> void {
        req.amr_wb_octet_aligned_dynamic_pt_valid = FALSE;
        //req.amr_wb_octet_aligned_dynamic_pt = 100;
      },
      [](ims_settings_get_codec_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.amr_wb_octet_aligned_dynamic_pt_valid = FALSE;
        //resp.amr_wb_octet_aligned_dynamic_pt = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_AMR_WB_BANDWIDTH_EFFICIENT_PT;
        req.has_intValue = TRUE;
        req.intValue = UINT16_MAX+100;
      },
      [](ims_settings_set_codec_dynamic_config_req_msg_v01 &req) -> void {
        req.amr_wb_bandwidth_efficient_dynamic_pt_valid = FALSE;
        //req.amr_wb_bandwidth_efficient_dynamic_pt = 100;
      },
      [](ims_settings_get_codec_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.amr_wb_bandwidth_efficient_dynamic_pt_valid = FALSE;
        //resp.amr_wb_bandwidth_efficient_dynamic_pt = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_AMR_OCTET_ALIGNED_PT;
        req.has_intValue = TRUE;
        req.intValue = UINT16_MAX+100;
      },
      [](ims_settings_set_codec_dynamic_config_req_msg_v01 &req) -> void {
        req.amr_octet_aligned_dynamic_pt_valid = FALSE;
        //req.amr_octet_aligned_dynamic_pt = 100;
      },
      [](ims_settings_get_codec_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.amr_octet_aligned_dynamic_pt_valid = FALSE;
        //resp.amr_octet_aligned_dynamic_pt = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_AMR_BANDWIDTH_EFFICIENT_PT;
        req.has_intValue = TRUE;
        req.intValue = UINT16_MAX+100;
      },
      [](ims_settings_set_codec_dynamic_config_req_msg_v01 &req) -> void {
        req.amr_bandwidth_efficient_dynamic_pt_valid = FALSE;
        //req.amr_bandwidth_efficient_dynamic_pt = 100;
      },
      [](ims_settings_get_codec_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.amr_bandwidth_efficient_dynamic_pt_valid = FALSE;
        //resp.amr_bandwidth_efficient_dynamic_pt = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_DTMF_WB_PT;
        req.has_intValue = TRUE;
        req.intValue = UINT16_MAX+100;
      },
      [](ims_settings_set_codec_dynamic_config_req_msg_v01 &req) -> void {
        req.dtmf_wb_dynamic_pt_valid = FALSE;
        //req.dtmf_wb_dynamic_pt = 100;
      },
      [](ims_settings_get_codec_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.dtmf_wb_dynamic_pt_valid = FALSE;
        //resp.dtmf_wb_dynamic_pt = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_DTMF_NB_PT;
        req.has_intValue = TRUE;
        req.intValue = UINT16_MAX+100;
      },
      [](ims_settings_set_codec_dynamic_config_req_msg_v01 &req) -> void {
        req.dtmf_nb_dynamic_pt_valid = FALSE;
        //req.dtmf_nb_dynamic_pt = 100;
      },
      [](ims_settings_get_codec_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.dtmf_nb_dynamic_pt_valid = FALSE;
        //resp.dtmf_nb_dynamic_pt = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_AMR_DEFAULT_MODE;
        req.has_intValue = TRUE;
        req.intValue = UINT16_MAX+1;
      },
      [](ims_settings_set_codec_dynamic_config_req_msg_v01 &req) -> void {
        req.amr_wb_default_mode_valid = FALSE;
        //req.amr_wb_default_mode = IMS_SETTINGS_AMR_WB_CODEC_MODE_8_V01;
      },
      [](ims_settings_get_codec_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.amr_wb_default_mode_valid = FALSE;
        //resp.amr_wb_default_mode = IMS_SETTINGS_AMR_WB_CODEC_MODE_8_V01;
      })
  };


  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

#if 0
    QMI_node.update_default_qmi_ims_settings_set_codec_dynamic_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_codec_dynamic_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);
#endif

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    //ims_ptr->has_item = TRUE;
    //ims_ptr->item = testReq.imsReq.item;
    IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
    t.expect(IMS_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_codec_dynamic_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_codec_dynamic_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    //*ims_ptr = testReq.imsReq;
    IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
    t.expect(IMS_node);
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_voice_dynamic_config_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items voice_dynamic_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_voice_dynamic_config_req_msg_v01 qmiReq;
    ims_settings_get_voice_dynamic_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_voice_dynamic_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_voice_dynamic_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };

  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SIP_SESSION_TIMER;
        req.has_intValue = TRUE;
        req.intValue = 100;
      },
      [](ims_settings_set_voice_dynamic_config_req_msg_v01 &req) -> void {
        req.session_expires_valid = TRUE;
        req.session_expires = 100;
      },
      [](ims_settings_get_voice_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.session_expires_valid = TRUE;
        resp.session_expires = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_MIN_SESSION_EXPIRY;
        req.has_intValue = TRUE;
        req.intValue = 100;
      },
      [](ims_settings_set_voice_dynamic_config_req_msg_v01 &req) -> void {
        req.min_session_expiry_valid = TRUE;
        req.min_session_expiry = 100;
      },
      [](ims_settings_get_voice_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.min_session_expiry_valid = TRUE;
        resp.min_session_expiry = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SILENT_REDIAL_ENABLE;
        req.has_intValue = TRUE;
        req.intValue = FALSE;
      },
      [](ims_settings_set_voice_dynamic_config_req_msg_v01 &req) -> void {
        req.silent_redial_enable_valid = TRUE;
        req.silent_redial_enable = FALSE;
      },
      [](ims_settings_get_voice_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.silent_redial_enable_valid = TRUE;
        resp.silent_redial_enable = FALSE;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VCE_SETTING_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = TRUE;
      },
      [](ims_settings_set_voice_dynamic_config_req_msg_v01 &req) -> void {
        req.vice_enabled_valid = TRUE;
        req.vice_enabled = TRUE;
      },
      [](ims_settings_get_voice_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.vice_enabled_valid = TRUE;
        resp.vice_enabled = TRUE;
      }),
  };


  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_set_voice_dynamic_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_voice_dynamic_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.expect(IMS_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_voice_dynamic_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_voice_dynamic_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    *ims_ptr = testReq.imsReq;
    t.expect(IMS_node);
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_voice_dynamic_config_002(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items voice_dynamic_config FAILURE cases.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_voice_dynamic_config_req_msg_v01 qmiReq;
    ims_settings_get_voice_dynamic_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_voice_dynamic_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_voice_dynamic_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };

  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SIP_SESSION_TIMER;
        req.has_intValue = TRUE;
        req.intValue = 100;
      },
      [](ims_settings_set_voice_dynamic_config_req_msg_v01 &req) -> void {
        req.session_expires_valid = TRUE;
        req.session_expires = 100;
      },
      [](ims_settings_get_voice_dynamic_config_rsp_msg_v01 &resp) -> void {
        //resp.session_expires_valid = TRUE;
        //resp.session_expires = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_MIN_SESSION_EXPIRY;
        req.has_intValue = TRUE;
        req.intValue = 100;
      },
      [](ims_settings_set_voice_dynamic_config_req_msg_v01 &req) -> void {
        req.min_session_expiry_valid = TRUE;
        req.min_session_expiry = 100;
      },
      [](ims_settings_get_voice_dynamic_config_rsp_msg_v01 &resp) -> void {
        //resp.min_session_expiry_valid = TRUE;
        //resp.min_session_expiry = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SILENT_REDIAL_ENABLE;
        req.has_intValue = TRUE;
        req.intValue = FALSE;
      },
      [](ims_settings_set_voice_dynamic_config_req_msg_v01 &req) -> void {
        req.silent_redial_enable_valid = TRUE;
        req.silent_redial_enable = FALSE;
      },
      [](ims_settings_get_voice_dynamic_config_rsp_msg_v01 &resp) -> void {
        //resp.silent_redial_enable_valid = TRUE;
        //resp.silent_redial_enable = FALSE;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VCE_SETTING_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = TRUE;
      },
      [](ims_settings_set_voice_dynamic_config_req_msg_v01 &req) -> void {
        req.vice_enabled_valid = TRUE;
        req.vice_enabled = TRUE;
      },
      [](ims_settings_get_voice_dynamic_config_rsp_msg_v01 &resp) -> void {
        //resp.vice_enabled_valid = TRUE;
        //resp.vice_enabled = TRUE;
      }),
  };


  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_set_voice_dynamic_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_voice_dynamic_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.expect(IMS_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_voice_dynamic_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_voice_dynamic_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    //*ims_ptr = testReq.imsReq;
    IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
    t.expect(IMS_node);
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_voice_dynamic_config_003(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items voice_dynamic_config: GET FAILURE.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_SESSION_TIMER;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 100;
  t.inject(IMS_node);

  ims_settings_set_voice_dynamic_config_req_msg_v01 *set_req =
    QMI_node.update_default_qmi_ims_settings_set_voice_dynamic_config_req();
  set_req->session_expires_valid = TRUE;
  set_req->session_expires = 100;
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_voice_dynamic_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_SESSION_TIMER;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 100;
  t.inject(IMS_node);

  set_req = QMI_node.update_default_qmi_ims_settings_set_voice_dynamic_config_req();
  set_req->session_expires_valid = TRUE;
  set_req->session_expires = 100;
  t.expect(QMI_node);

  ims_settings_set_voice_dynamic_config_rsp_msg_v01 *set_resp =
    QMI_node.update_default_qmi_ims_settings_set_voice_dynamic_config_resp(
        QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_SESSION_TIMER;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 100;
  t.inject(IMS_node);

  set_req = QMI_node.update_default_qmi_ims_settings_set_voice_dynamic_config_req();
  set_req->session_expires_valid = TRUE;
  set_req->session_expires = 100;
  t.expect(QMI_node);

  set_resp = QMI_node.update_default_qmi_ims_settings_set_voice_dynamic_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  set_resp->settings_resp_valid = TRUE;
  set_resp->settings_resp = IMS_SETTINGS_MSG_WRITE_FAILED_V01;
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_SESSION_TIMER;
  ims_ptr->has_errorCause = TRUE;
  ims_ptr->errorCause = ims_ConfigFailureCause_CONFIG_WRITE_FAILED;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);


  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_SESSION_TIMER;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_voice_dynamic_config_req();
  t.expect(QMI_node);

  ims_settings_get_voice_dynamic_config_rsp_msg_v01 *get_resp =
    QMI_node.update_default_qmi_ims_settings_get_voice_dynamic_config_resp(
        QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_SESSION_TIMER;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_voice_dynamic_config_req();
  t.expect(QMI_node);

  get_resp = QMI_node.update_default_qmi_ims_settings_get_voice_dynamic_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_SESSION_TIMER;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_voice_dynamic_config_req();
  t.expect(QMI_node);

  get_resp = QMI_node.update_default_qmi_ims_settings_get_voice_dynamic_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  get_resp->settings_resp_valid = TRUE;
  get_resp->settings_resp = IMS_SETTINGS_MSG_NO_ERR_V01;
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_SESSION_TIMER;
  ims_ptr->has_errorCause = TRUE;
  ims_ptr->errorCause = ims_ConfigFailureCause_CONFIG_NO_ERR;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_voice_dynamic_config_004(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items voice_dynamic_config FAILURE cases.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_voice_dynamic_config_req_msg_v01 qmiReq;
    ims_settings_get_voice_dynamic_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_voice_dynamic_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_voice_dynamic_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };

  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SIP_SESSION_TIMER;
        req.has_intValue = TRUE;
        req.intValue = UINT16_MAX+100;
      },
      [](ims_settings_set_voice_dynamic_config_req_msg_v01 &req) -> void {
        req.session_expires_valid = FALSE;
        //req.session_expires = 100;
      },
      [](ims_settings_get_voice_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.session_expires_valid = FALSE;
        //resp.session_expires = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_MIN_SESSION_EXPIRY;
        req.has_intValue = TRUE;
        req.intValue = UINT16_MAX+100;
      },
      [](ims_settings_set_voice_dynamic_config_req_msg_v01 &req) -> void {
        req.min_session_expiry_valid = FALSE;
        //req.min_session_expiry = 100;
      },
      [](ims_settings_get_voice_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.min_session_expiry_valid = FALSE;
        //resp.min_session_expiry = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SILENT_REDIAL_ENABLE;
        // Expected BOOLEAN but passing STRING
        //req.has_intValue = TRUE;
        //req.intValue = TRUE;
        req.stringValue.arg = strdup("1");
      },
      [](ims_settings_set_voice_dynamic_config_req_msg_v01 &req) -> void {
        req.silent_redial_enable_valid = FALSE;
        //req.silent_redial_enable = FALSE;
      },
      [](ims_settings_get_voice_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.silent_redial_enable_valid = FALSE;
        //resp.silent_redial_enable = FALSE;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VCE_SETTING_ENABLED;
        // Expected BOOLEAN but passing STRING
        //req.has_intValue = TRUE;
        //req.intValue = FALSE;
        req.stringValue.arg = strdup("1");
      },
      [](ims_settings_set_voice_dynamic_config_req_msg_v01 &req) -> void {
        req.vice_enabled_valid = FALSE;
        //req.vice_enabled = TRUE;
      },
      [](ims_settings_get_voice_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.vice_enabled_valid = FALSE;
        //resp.vice_enabled = TRUE;
      }),
  };


  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

#if 0
    QMI_node.update_default_qmi_ims_settings_set_voice_dynamic_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_voice_dynamic_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);
#endif

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    //ims_ptr->has_item = TRUE;
    //ims_ptr->item = testReq.imsReq.item;
    IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
    t.expect(IMS_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_voice_dynamic_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_voice_dynamic_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    //*ims_ptr = testReq.imsReq;
    IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
    t.expect(IMS_node);
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_reg_mgr_new_config_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items reg_mgr_new_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_reg_mgr_new_config_req_msg_v01 qmiReq;
    ims_settings_get_reg_mgr_new_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_reg_mgr_new_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_reg_mgr_new_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };

  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_T_DELAY;
        req.has_intValue = TRUE;
        req.intValue = 100;
      },
      [](ims_settings_set_reg_mgr_new_config_req_msg_v01 &req) -> void {
        req.reg_irat_hystersis_timer_valid = TRUE;
        req.reg_irat_hystersis_timer = 100;
      },
      [](ims_settings_get_reg_mgr_new_config_rsp_msg_v01 &resp) -> void {
        resp.reg_irat_hystersis_timer_valid = TRUE;
        resp.reg_irat_hystersis_timer = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_REGISTRATION_RETRY_BASE_TIME_SEC;
        req.has_intValue = TRUE;
        req.intValue = 100;
      },
      [](ims_settings_set_reg_mgr_new_config_req_msg_v01 &req) -> void {
        req.reg_retry_base_time_valid = TRUE;
        req.reg_retry_base_time = 100;
      },
      [](ims_settings_get_reg_mgr_new_config_rsp_msg_v01 &resp) -> void {
        resp.reg_retry_base_time_valid = TRUE;
        resp.reg_retry_base_time = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_REGISTRATION_RETRY_MAX_TIME_SEC;
        req.has_intValue = TRUE;
        req.intValue = 100;
      },
      [](ims_settings_set_reg_mgr_new_config_req_msg_v01 &req) -> void {
        req.reg_retry_max_time_valid = TRUE;
        req.reg_retry_max_time = 100;
      },
      [](ims_settings_get_reg_mgr_new_config_rsp_msg_v01 &resp) -> void {
        resp.reg_retry_max_time_valid = TRUE;
        resp.reg_retry_max_time = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_LBO_PCSCF_ADDRESS;
        req.stringValue.arg = strdup("test regmgr_primary_pcscf");
      },
      [](ims_settings_set_reg_mgr_new_config_req_msg_v01 &req) -> void {
        req.regmgr_primary_pcscf_valid = TRUE;
        req.regmgr_primary_pcscf;
        strlcpy((char *)req.regmgr_primary_pcscf,
                "test regmgr_primary_pcscf", strlen("test regmgr_primary_pcscf")+1);
      },
      [](ims_settings_get_reg_mgr_new_config_rsp_msg_v01 &resp) -> void {
        resp.regmgr_primary_pcscf_valid = TRUE;
        strlcpy((char *)resp.regmgr_primary_pcscf,
                "test regmgr_primary_pcscf", strlen("test regmgr_primary_pcscf")+1);
      }),
  };


  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_set_reg_mgr_new_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_reg_mgr_new_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.expect(IMS_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_reg_mgr_new_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_reg_mgr_new_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    *ims_ptr = testReq.imsReq;
    t.expect(IMS_node);
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_reg_mgr_new_config_002(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items reg_mgr_new_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_reg_mgr_new_config_req_msg_v01 qmiReq;
    ims_settings_get_reg_mgr_new_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_reg_mgr_new_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_reg_mgr_new_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };

  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_T_DELAY;
        req.has_intValue = TRUE;
        req.intValue = 100;
      },
      [](ims_settings_set_reg_mgr_new_config_req_msg_v01 &req) -> void {
        req.reg_irat_hystersis_timer_valid = TRUE;
        req.reg_irat_hystersis_timer = 100;
      },
      [](ims_settings_get_reg_mgr_new_config_rsp_msg_v01 &resp) -> void {
        //resp.reg_irat_hystersis_timer_valid = TRUE;
        //resp.reg_irat_hystersis_timer = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_REGISTRATION_RETRY_BASE_TIME_SEC;
        req.has_intValue = TRUE;
        req.intValue = 100;
      },
      [](ims_settings_set_reg_mgr_new_config_req_msg_v01 &req) -> void {
        req.reg_retry_base_time_valid = TRUE;
        req.reg_retry_base_time = 100;
      },
      [](ims_settings_get_reg_mgr_new_config_rsp_msg_v01 &resp) -> void {
        //resp.reg_retry_base_time_valid = TRUE;
        //resp.reg_retry_base_time = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_REGISTRATION_RETRY_MAX_TIME_SEC;
        req.has_intValue = TRUE;
        req.intValue = 100;
      },
      [](ims_settings_set_reg_mgr_new_config_req_msg_v01 &req) -> void {
        req.reg_retry_max_time_valid = TRUE;
        req.reg_retry_max_time = 100;
      },
      [](ims_settings_get_reg_mgr_new_config_rsp_msg_v01 &resp) -> void {
        //resp.reg_retry_max_time_valid = TRUE;
        //resp.reg_retry_max_time = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_LBO_PCSCF_ADDRESS;
        req.stringValue.arg = strdup("test regmgr_primary_pcscf");
      },
      [](ims_settings_set_reg_mgr_new_config_req_msg_v01 &req) -> void {
        req.regmgr_primary_pcscf_valid = TRUE;
        req.regmgr_primary_pcscf;
        strlcpy((char *)req.regmgr_primary_pcscf,
                "test regmgr_primary_pcscf", strlen("test regmgr_primary_pcscf")+1);
      },
      [](ims_settings_get_reg_mgr_new_config_rsp_msg_v01 &resp) -> void {
        //resp.regmgr_primary_pcscf_valid = TRUE;
        //strlcpy((char *)resp.regmgr_primary_pcscf,
        //        "test regmgr_primary_pcscf", strlen("test regmgr_primary_pcscf")+1);
      }),
  };


  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_set_reg_mgr_new_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_reg_mgr_new_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.expect(IMS_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_reg_mgr_new_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_reg_mgr_new_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    //*ims_ptr = testReq.imsReq;
    IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
    t.expect(IMS_node);
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_reg_mgr_new_config_003(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items reg_mgr_new_config: GET FAILURE.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_T_DELAY;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 100;
  t.inject(IMS_node);

  ims_settings_set_reg_mgr_new_config_req_msg_v01 *set_req =
    QMI_node.update_default_qmi_ims_settings_set_reg_mgr_new_config_req();
  set_req->reg_irat_hystersis_timer_valid = TRUE;
  set_req->reg_irat_hystersis_timer = 100;
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_reg_mgr_new_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_T_DELAY;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 100;
  t.inject(IMS_node);

  set_req = QMI_node.update_default_qmi_ims_settings_set_reg_mgr_new_config_req();
  set_req->reg_irat_hystersis_timer_valid = TRUE;
  set_req->reg_irat_hystersis_timer = 100;
  t.expect(QMI_node);

  ims_settings_set_reg_mgr_new_config_rsp_msg_v01 *set_resp =
    QMI_node.update_default_qmi_ims_settings_set_reg_mgr_new_config_resp(
        QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_T_DELAY;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 100;
  t.inject(IMS_node);

  set_req = QMI_node.update_default_qmi_ims_settings_set_reg_mgr_new_config_req();
  set_req->reg_irat_hystersis_timer_valid = TRUE;
  set_req->reg_irat_hystersis_timer = 100;
  t.expect(QMI_node);

  set_resp = QMI_node.update_default_qmi_ims_settings_set_reg_mgr_new_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  set_resp->settings_resp_valid = TRUE;
  set_resp->settings_resp = IMS_SETTINGS_MSG_WRITE_FAILED_V01;
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_T_DELAY;
  ims_ptr->has_errorCause = TRUE;
  ims_ptr->errorCause = ims_ConfigFailureCause_CONFIG_WRITE_FAILED;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);


  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_T_DELAY;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_reg_mgr_new_config_req();
  t.expect(QMI_node);

  ims_settings_get_reg_mgr_new_config_rsp_msg_v01 *get_resp =
    QMI_node.update_default_qmi_ims_settings_get_reg_mgr_new_config_resp(
        QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_T_DELAY;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_reg_mgr_new_config_req();
  t.expect(QMI_node);

  get_resp = QMI_node.update_default_qmi_ims_settings_get_reg_mgr_new_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_T_DELAY;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_reg_mgr_new_config_req();
  t.expect(QMI_node);

  get_resp = QMI_node.update_default_qmi_ims_settings_get_reg_mgr_new_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  get_resp->settings_resp_valid = TRUE;
  get_resp->settings_resp = IMS_SETTINGS_MSG_IMS_NOT_READY_V01;
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_T_DELAY;
  ims_ptr->has_errorCause = TRUE;
  ims_ptr->errorCause = ims_ConfigFailureCause_CONFIG_IMS_NOT_READY;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_reg_mgr_new_config_004(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items reg_mgr_new_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_reg_mgr_new_config_req_msg_v01 qmiReq;
    ims_settings_get_reg_mgr_new_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_reg_mgr_new_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_reg_mgr_new_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };

  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_T_DELAY;
        req.has_intValue = TRUE;
        req.intValue = UINT16_MAX+100;
      },
      [](ims_settings_set_reg_mgr_new_config_req_msg_v01 &req) -> void {
        req.reg_irat_hystersis_timer_valid = FALSE;
        //req.reg_irat_hystersis_timer = 100;
      },
      [](ims_settings_get_reg_mgr_new_config_rsp_msg_v01 &resp) -> void {
        resp.reg_irat_hystersis_timer_valid = FALSE;
        //resp.reg_irat_hystersis_timer = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_REGISTRATION_RETRY_BASE_TIME_SEC;
        req.has_intValue = TRUE;
        req.intValue = UINT16_MAX+100;
      },
      [](ims_settings_set_reg_mgr_new_config_req_msg_v01 &req) -> void {
        req.reg_retry_base_time_valid = FALSE;
        //req.reg_retry_base_time = 100;
      },
      [](ims_settings_get_reg_mgr_new_config_rsp_msg_v01 &resp) -> void {
        resp.reg_retry_base_time_valid = FALSE;
        //resp.reg_retry_base_time = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_REGISTRATION_RETRY_MAX_TIME_SEC;
        req.has_intValue = TRUE;
        req.intValue = UINT16_MAX+100;
      },
      [](ims_settings_set_reg_mgr_new_config_req_msg_v01 &req) -> void {
        req.reg_retry_max_time_valid = FALSE;
        //req.reg_retry_max_time = 100;
      },
      [](ims_settings_get_reg_mgr_new_config_rsp_msg_v01 &resp) -> void {
        resp.reg_retry_max_time_valid = FALSE;
        //resp.reg_retry_max_time = 100;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_LBO_PCSCF_ADDRESS;
        req.has_intValue = TRUE;
        req.intValue = 100;  // Expected string but passing INT
        //req.stringValue.arg = strdup("test regmgr_primary_pcscf");
      },
      [](ims_settings_set_reg_mgr_new_config_req_msg_v01 &req) -> void {
        req.regmgr_primary_pcscf_valid = FALSE;
        //req.regmgr_primary_pcscf;
        //strlcpy((char *)req.regmgr_primary_pcscf,
        //        "test regmgr_primary_pcscf", strlen("test regmgr_primary_pcscf")+1);
      },
      [](ims_settings_get_reg_mgr_new_config_rsp_msg_v01 &resp) -> void {
        //resp.regmgr_primary_pcscf_valid = TRUE;
        //strlcpy((char *)resp.regmgr_primary_pcscf,
        //        "test regmgr_primary_pcscf", strlen("test regmgr_primary_pcscf")+1);
      }),
  };


  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

#if 0
    QMI_node.update_default_qmi_ims_settings_set_reg_mgr_new_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_reg_mgr_new_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);
#endif

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    //ims_ptr->has_item = TRUE;
    //ims_ptr->item = testReq.imsReq.item;
    IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
    t.expect(IMS_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_reg_mgr_new_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_reg_mgr_new_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    //*ims_ptr = testReq.imsReq;
    IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
    t.expect(IMS_node);
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}


test_result ril_utf_ims_config_sms_new_config_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items sms_new_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_sms_new_config_req_msg_v01 qmiReq;
    ims_settings_get_sms_new_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_sms_new_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_sms_new_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };

  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SMS_FORMAT;
        req.has_intValue = TRUE;
        req.intValue = 1;  //3GPP
      },
      [](ims_settings_set_sms_new_config_req_msg_v01 &req) -> void {
        req.sms_format_valid = TRUE;
        req.sms_format = IMS_SETTINGS_SMS_FORMAT_3GPP_V01;
      },
      [](ims_settings_get_sms_new_config_rsp_msg_v01 &resp) -> void {
        resp.sms_format_valid = TRUE;
        resp.sms_format = IMS_SETTINGS_SMS_FORMAT_3GPP_V01;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SMS_FORMAT;
        req.has_intValue = TRUE;
        req.intValue = 0;  //3GPP2
      },
      [](ims_settings_set_sms_new_config_req_msg_v01 &req) -> void {
        req.sms_format_valid = TRUE;
        req.sms_format = IMS_SETTINGS_SMS_FORMAT_3GPP2_V01;
      },
      [](ims_settings_get_sms_new_config_rsp_msg_v01 &resp) -> void {
        resp.sms_format_valid = TRUE;
        resp.sms_format = IMS_SETTINGS_SMS_FORMAT_3GPP2_V01;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SMS_PSI;
        req.stringValue.arg = strdup("test sms_psi");
      },
      [](ims_settings_set_sms_new_config_req_msg_v01 &req) -> void {
        req.sms_psi_valid = TRUE;
        req.sms_psi;
        strlcpy((char *)req.sms_psi,
                "test sms_psi", strlen("test sms_psi")+1);
      },
      [](ims_settings_get_sms_new_config_rsp_msg_v01 &resp) -> void {
        resp.sms_psi_valid = TRUE;
        strlcpy((char *)resp.sms_psi,
                "test sms_psi", strlen("test sms_psi")+1);
      }),
  };


  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_set_sms_new_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_sms_new_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.expect(IMS_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_sms_new_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_sms_new_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    *ims_ptr = testReq.imsReq;
    t.expect(IMS_node);
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_sms_new_config_002(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items sms_new_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_sms_new_config_req_msg_v01 qmiReq;
    ims_settings_get_sms_new_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_sms_new_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_sms_new_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };

  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SMS_FORMAT;
        req.has_intValue = TRUE;
        req.intValue = 1;  //3GPP
      },
      [](ims_settings_set_sms_new_config_req_msg_v01 &req) -> void {
        req.sms_format_valid = TRUE;
        req.sms_format = IMS_SETTINGS_SMS_FORMAT_3GPP_V01;
      },
      [](ims_settings_get_sms_new_config_rsp_msg_v01 &resp) -> void {
        //resp.sms_format_valid = TRUE;
        //resp.sms_format = IMS_SETTINGS_SMS_FORMAT_3GPP_V01;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SMS_FORMAT;
        req.has_intValue = TRUE;
        req.intValue = 0;  //3GPP2
      },
      [](ims_settings_set_sms_new_config_req_msg_v01 &req) -> void {
        req.sms_format_valid = TRUE;
        req.sms_format = IMS_SETTINGS_SMS_FORMAT_3GPP2_V01;
      },
      [](ims_settings_get_sms_new_config_rsp_msg_v01 &resp) -> void {
        //resp.sms_format_valid = TRUE;
        //resp.sms_format = IMS_SETTINGS_SMS_FORMAT_3GPP2_V01;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SMS_PSI;
        req.stringValue.arg = strdup("test sms_psi");
      },
      [](ims_settings_set_sms_new_config_req_msg_v01 &req) -> void {
        req.sms_psi_valid = TRUE;
        req.sms_psi;
        strlcpy((char *)req.sms_psi,
                "test sms_psi", strlen("test sms_psi")+1);
      },
      [](ims_settings_get_sms_new_config_rsp_msg_v01 &resp) -> void {
        //resp.sms_psi_valid = TRUE;
        //strlcpy((char *)resp.sms_psi,
        //        "test sms_psi", strlen("test sms_psi")+1);
      }),
  };


  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_set_sms_new_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_sms_new_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.expect(IMS_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_sms_new_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_sms_new_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    //*ims_ptr = testReq.imsReq;
    IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
    t.expect(IMS_node);
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_sms_new_config_003(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items sms_new_config: GET FAILURE.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SMS_FORMAT;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 1;
  t.inject(IMS_node);

  ims_settings_set_sms_new_config_req_msg_v01 *set_req =
    QMI_node.update_default_qmi_ims_settings_set_sms_new_config_req();
  set_req->sms_format_valid = TRUE;
  set_req->sms_format = IMS_SETTINGS_SMS_FORMAT_3GPP_V01;
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_sms_new_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SMS_FORMAT;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 1;
  t.inject(IMS_node);

  set_req = QMI_node.update_default_qmi_ims_settings_set_sms_new_config_req();
  set_req->sms_format_valid = TRUE;
  set_req->sms_format = IMS_SETTINGS_SMS_FORMAT_3GPP_V01;
  t.expect(QMI_node);

  ims_settings_set_sms_new_config_rsp_msg_v01 *set_resp =
    QMI_node.update_default_qmi_ims_settings_set_sms_new_config_resp(
        QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SMS_FORMAT;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 1;
  t.inject(IMS_node);

  set_req = QMI_node.update_default_qmi_ims_settings_set_sms_new_config_req();
  set_req->sms_format_valid = TRUE;
  set_req->sms_format = IMS_SETTINGS_SMS_FORMAT_3GPP_V01;
  t.expect(QMI_node);

  set_resp = QMI_node.update_default_qmi_ims_settings_set_sms_new_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  set_resp->settings_resp_valid = TRUE;
  set_resp->settings_resp = IMS_SETTINGS_MSG_WRITE_FAILED_V01;
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SMS_FORMAT;
  ims_ptr->has_errorCause = TRUE;
  ims_ptr->errorCause = ims_ConfigFailureCause_CONFIG_WRITE_FAILED;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);


  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SMS_FORMAT;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_sms_new_config_req();
  t.expect(QMI_node);

  ims_settings_get_sms_new_config_rsp_msg_v01 *get_resp =
    QMI_node.update_default_qmi_ims_settings_get_sms_new_config_resp(
        QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SMS_FORMAT;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_sms_new_config_req();
  t.expect(QMI_node);

  get_resp = QMI_node.update_default_qmi_ims_settings_get_sms_new_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SMS_FORMAT;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_sms_new_config_req();
  t.expect(QMI_node);

  get_resp = QMI_node.update_default_qmi_ims_settings_get_sms_new_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  get_resp->settings_resp_valid = TRUE;
  get_resp->settings_resp = IMS_SETTINGS_MSG_READ_FAILED_V01;
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SMS_FORMAT;
  ims_ptr->has_errorCause = TRUE;
  ims_ptr->errorCause = ims_ConfigFailureCause_CONFIG_READ_FAILED;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_sms_new_config_004(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items sms_new_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_sms_new_config_req_msg_v01 qmiReq;
    ims_settings_get_sms_new_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_sms_new_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_sms_new_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };

  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SMS_FORMAT;
        req.has_intValue = TRUE;
        req.intValue = 100;  //invalid value
      },
      [](ims_settings_set_sms_new_config_req_msg_v01 &req) -> void {
        req.sms_format_valid = FALSE;
        //req.sms_format = IMS_SETTINGS_SMS_FORMAT_3GPP_V01;
      },
      [](ims_settings_get_sms_new_config_rsp_msg_v01 &resp) -> void {
        resp.sms_format_valid = FALSE;
        //resp.sms_format = IMS_SETTINGS_SMS_FORMAT_3GPP_V01;
      }),
#if 0
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SMS_FORMAT;
        req.has_intValue = TRUE;
        req.intValue = 0;  //3GPP2
      },
      [](ims_settings_set_sms_new_config_req_msg_v01 &req) -> void {
        req.sms_format_valid = TRUE;
        req.sms_format = IMS_SETTINGS_SMS_FORMAT_3GPP2_V01;
      },
      [](ims_settings_get_sms_new_config_rsp_msg_v01 &resp) -> void {
        //resp.sms_format_valid = TRUE;
        //resp.sms_format = IMS_SETTINGS_SMS_FORMAT_3GPP2_V01;
      }),
#endif
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SMS_PSI;
        req.has_intValue = TRUE;
        req.intValue = 100;  // Expected string but pass int
        //req.stringValue.arg = strdup("test sms_psi");
      },
      [](ims_settings_set_sms_new_config_req_msg_v01 &req) -> void {
        req.sms_psi_valid = FALSE;
        //req.sms_psi;
        //strlcpy((char *)req.sms_psi,
        //        "test sms_psi", strlen("test sms_psi")+1);
      },
      [](ims_settings_get_sms_new_config_rsp_msg_v01 &resp) -> void {
        //resp.sms_psi_valid = TRUE;
        //strlcpy((char *)resp.sms_psi,
        //        "test sms_psi", strlen("test sms_psi")+1);
      }),
  };


  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

#if 0
    QMI_node.update_default_qmi_ims_settings_set_sms_new_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_sms_new_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);
#endif

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    //ims_ptr->has_item = TRUE;
    //ims_ptr->item = testReq.imsReq.item;
    IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
    t.expect(IMS_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_sms_new_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_sms_new_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    //*ims_ptr = testReq.imsReq;
    IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
    t.expect(IMS_node);
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_rtp_dynamic_config_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items rtp_dynamic_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_rtp_dynamic_config_req_msg_v01 qmiReq;
    ims_settings_get_rtp_dynamic_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_rtp_dynamic_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_rtp_dynamic_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };

  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SPEECH_START_PORT;
        req.has_intValue = TRUE;
        req.intValue = 2;
      },
      [](ims_settings_set_rtp_dynamic_config_req_msg_v01 &req) -> void {
        req.speech_start_port_valid = 1;
        req.speech_start_port = 2;
      },
      [](ims_settings_get_rtp_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.speech_start_port_valid = 1;
        resp.speech_start_port = 2;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SPEECH_END_PORT;
        req.has_intValue = TRUE;
        req.intValue = 3;
      },
      [](ims_settings_set_rtp_dynamic_config_req_msg_v01 &req) -> void {
        req.speech_stop_port_valid = 1;
        req.speech_stop_port = 3;
      },
      [](ims_settings_get_rtp_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.speech_stop_port_valid = 1;
        resp.speech_stop_port = 3;
      }),
  };


  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_set_rtp_dynamic_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_rtp_dynamic_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.expect(IMS_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_rtp_dynamic_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_rtp_dynamic_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    *ims_ptr = testReq.imsReq;
    t.expect(IMS_node);
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_rtp_dynamic_config_002(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items rtp_dynamic_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_rtp_dynamic_config_req_msg_v01 qmiReq;
    ims_settings_get_rtp_dynamic_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_rtp_dynamic_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_rtp_dynamic_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };

  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SPEECH_START_PORT;
        req.has_intValue = TRUE;
        req.intValue = UINT16_MAX+2;
      },
      [](ims_settings_set_rtp_dynamic_config_req_msg_v01 &req) -> void {
        req.speech_start_port_valid = 0;
        //req.speech_start_port_valid = 1;
        //req.speech_start_port = 2;
      },
      [](ims_settings_get_rtp_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.speech_start_port_valid = 0;
        //resp.speech_start_port_valid = 1;
        //resp.speech_start_port = 2;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SPEECH_END_PORT;
        req.has_intValue = TRUE;
        req.intValue = UINT16_MAX+3;
      },
      [](ims_settings_set_rtp_dynamic_config_req_msg_v01 &req) -> void {
        req.speech_stop_port_valid = 0;
        //req.speech_stop_port_valid = 1;
        //req.speech_stop_port = 3;
      },
      [](ims_settings_get_rtp_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.speech_stop_port_valid = 0;
        //resp.speech_stop_port_valid = 1;
        //resp.speech_stop_port = 3;
      }),
  };


  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

#if 0
    QMI_node.update_default_qmi_ims_settings_set_rtp_dynamic_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_rtp_dynamic_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);
#endif

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    //ims_ptr->has_item = TRUE;
    //ims_ptr->item = testReq.imsReq.item;
    IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
    t.expect(IMS_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_rtp_dynamic_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_rtp_dynamic_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    //*ims_ptr = testReq.imsReq;
    IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
    t.expect(IMS_node);
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_rtp_dynamic_config_003(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items rtp_dynamic_config: GET FAILURE.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SPEECH_START_PORT;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 100;
  t.inject(IMS_node);

  ims_settings_set_rtp_dynamic_config_req_msg_v01 *set_req =
    QMI_node.update_default_qmi_ims_settings_set_rtp_dynamic_config_req();
  set_req->speech_start_port_valid = TRUE;
  set_req->speech_start_port = 100;
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_rtp_dynamic_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SPEECH_START_PORT;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 100;
  t.inject(IMS_node);

  set_req = QMI_node.update_default_qmi_ims_settings_set_rtp_dynamic_config_req();
  set_req->speech_start_port_valid = TRUE;
  set_req->speech_start_port = 100;
  t.expect(QMI_node);

  ims_settings_set_rtp_dynamic_config_rsp_msg_v01 *set_resp =
    QMI_node.update_default_qmi_ims_settings_set_rtp_dynamic_config_resp(
        QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SPEECH_START_PORT;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 100;
  t.inject(IMS_node);

  set_req = QMI_node.update_default_qmi_ims_settings_set_rtp_dynamic_config_req();
  set_req->speech_start_port_valid = TRUE;
  set_req->speech_start_port = 100;
  t.expect(QMI_node);

  set_resp = QMI_node.update_default_qmi_ims_settings_set_rtp_dynamic_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  set_resp->settings_resp_valid = TRUE;
  set_resp->settings_resp = IMS_SETTINGS_MSG_WRITE_FAILED_V01;
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SPEECH_START_PORT;
  ims_ptr->has_errorCause = TRUE;
  ims_ptr->errorCause = ims_ConfigFailureCause_CONFIG_WRITE_FAILED;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);


  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SPEECH_START_PORT;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_rtp_dynamic_config_req();
  t.expect(QMI_node);

  ims_settings_get_rtp_dynamic_config_rsp_msg_v01 *get_resp =
    QMI_node.update_default_qmi_ims_settings_get_rtp_dynamic_config_resp(
        QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SPEECH_START_PORT;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_rtp_dynamic_config_req();
  t.expect(QMI_node);

  get_resp = QMI_node.update_default_qmi_ims_settings_get_rtp_dynamic_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SPEECH_START_PORT;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_rtp_dynamic_config_req();
  t.expect(QMI_node);

  get_resp = QMI_node.update_default_qmi_ims_settings_get_rtp_dynamic_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  get_resp->settings_resp_valid = TRUE;
  get_resp->settings_resp = IMS_SETTINGS_MSG_FILE_NOT_AVAILABLE_V01;
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SPEECH_START_PORT;
  ims_ptr->has_errorCause = TRUE;
  ims_ptr->errorCause = ims_ConfigFailureCause_CONFIG_FILE_NOT_AVAILABLE;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_ims_service_enable_config_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items ims_service_enable_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_ims_service_enable_config_req_msg_v01 qmiReq;
    ims_settings_get_ims_service_enable_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_ims_service_enable_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_ims_service_enable_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };

  ims_settings_omadm_services_type_mask_v01 omadm_mask =
    (IMS_SETTINGS_OMADM_SERVICES_BIT_MASK_NONE_V01 |
     IMS_SETTINGS_OMADM_SERVICES_ENABLE_BIT_MASK_V01);

  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VLT_SETTING_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 1;
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.service_mask_by_network_enabled_valid = TRUE;
        req.service_mask_by_network_enabled =
          (IMS_SETTINGS_OMADM_SERVICES_BIT_MASK_NONE_V01 |
           IMS_SETTINGS_OMADM_SERVICES_VOLTE_ENABLE_BIT_MASK_V01 |
           IMS_SETTINGS_OMADM_SERVICES_ENABLE_BIT_MASK_V01);
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.service_mask_by_network_enabled_valid = TRUE;
        resp.service_mask_by_network_enabled =
          (IMS_SETTINGS_OMADM_SERVICES_BIT_MASK_NONE_V01 |
           IMS_SETTINGS_OMADM_SERVICES_VOLTE_ENABLE_BIT_MASK_V01 |
           IMS_SETTINGS_OMADM_SERVICES_ENABLE_BIT_MASK_V01);
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VLT_SETTING_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 0;
      },
      [omadm_mask](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.service_mask_by_network_enabled_valid = TRUE;
        req.service_mask_by_network_enabled =
          (omadm_mask &   // IMS_SETTINGS_OMADM_SERVICES_ENABLE_BIT_MASK_V01 is set in the above set resp
           ~(IMS_SETTINGS_OMADM_SERVICES_VOLTE_ENABLE_BIT_MASK_V01));
      },
      [omadm_mask](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.service_mask_by_network_enabled_valid = TRUE;
        resp.service_mask_by_network_enabled =
          (omadm_mask &
           ~(IMS_SETTINGS_OMADM_SERVICES_VOLTE_ENABLE_BIT_MASK_V01));
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_LVC_SETTING_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 1;
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.service_mask_by_network_enabled_valid = TRUE;
        req.service_mask_by_network_enabled =
          (IMS_SETTINGS_OMADM_SERVICES_BIT_MASK_NONE_V01 |
           IMS_SETTINGS_OMADM_SERVICES_VT_ENABLE_BIT_MASK_V01 |
           IMS_SETTINGS_OMADM_SERVICES_ENABLE_BIT_MASK_V01);
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.service_mask_by_network_enabled_valid = TRUE;
        resp.service_mask_by_network_enabled =
          (IMS_SETTINGS_OMADM_SERVICES_BIT_MASK_NONE_V01 |
           IMS_SETTINGS_OMADM_SERVICES_VT_ENABLE_BIT_MASK_V01 |
           IMS_SETTINGS_OMADM_SERVICES_ENABLE_BIT_MASK_V01);
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_LVC_SETTING_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 0;
      },
      [omadm_mask](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.service_mask_by_network_enabled_valid = TRUE;
        req.service_mask_by_network_enabled =
          (omadm_mask &   // IMS_SETTINGS_OMADM_SERVICES_ENABLE_BIT_MASK_V01 is set in the above set resp
           ~(IMS_SETTINGS_OMADM_SERVICES_VT_ENABLE_BIT_MASK_V01));
      },
      [omadm_mask](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.service_mask_by_network_enabled_valid = TRUE;
        resp.service_mask_by_network_enabled =
          (omadm_mask &
           ~(IMS_SETTINGS_OMADM_SERVICES_VT_ENABLE_BIT_MASK_V01));
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VWF_SETTING_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 1;
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.service_mask_by_network_enabled_valid = TRUE;
        req.service_mask_by_network_enabled =
          (IMS_SETTINGS_OMADM_SERVICES_BIT_MASK_NONE_V01 |
           IMS_SETTINGS_OMADM_SERVICES_VOWIFI_ENABLE_BIT_MASK_V01 |
           IMS_SETTINGS_OMADM_SERVICES_ENABLE_BIT_MASK_V01);
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.service_mask_by_network_enabled_valid = TRUE;
        resp.service_mask_by_network_enabled =
          (IMS_SETTINGS_OMADM_SERVICES_BIT_MASK_NONE_V01 |
           IMS_SETTINGS_OMADM_SERVICES_VOWIFI_ENABLE_BIT_MASK_V01 |
           IMS_SETTINGS_OMADM_SERVICES_ENABLE_BIT_MASK_V01);
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VWF_SETTING_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 0;
      },
      [omadm_mask](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.service_mask_by_network_enabled_valid = TRUE;
        req.service_mask_by_network_enabled =
          (omadm_mask &   // IMS_SETTINGS_OMADM_SERVICES_ENABLE_BIT_MASK_V01 is set in the above set resp
           ~(IMS_SETTINGS_OMADM_SERVICES_VOWIFI_ENABLE_BIT_MASK_V01));
      },
      [omadm_mask](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.service_mask_by_network_enabled_valid = TRUE;
        resp.service_mask_by_network_enabled =
          (omadm_mask &
           ~(IMS_SETTINGS_OMADM_SERVICES_VOWIFI_ENABLE_BIT_MASK_V01));
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_EAB_SETTING_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 1;
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.service_mask_by_network_enabled_valid = TRUE;
        req.service_mask_by_network_enabled =
          (IMS_SETTINGS_OMADM_SERVICES_BIT_MASK_NONE_V01 |
           IMS_SETTINGS_OMADM_SERVICES_PRESENCE_ENABLE_BIT_MASK_V01 |
           IMS_SETTINGS_OMADM_SERVICES_ENABLE_BIT_MASK_V01);
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.service_mask_by_network_enabled_valid = TRUE;
        resp.service_mask_by_network_enabled =
          (IMS_SETTINGS_OMADM_SERVICES_BIT_MASK_NONE_V01 |
           IMS_SETTINGS_OMADM_SERVICES_PRESENCE_ENABLE_BIT_MASK_V01 |
           IMS_SETTINGS_OMADM_SERVICES_ENABLE_BIT_MASK_V01);
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_EAB_SETTING_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 0;
      },
      [omadm_mask](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.service_mask_by_network_enabled_valid = TRUE;
        req.service_mask_by_network_enabled =
          (omadm_mask &   // IMS_SETTINGS_OMADM_SERVICES_ENABLE_BIT_MASK_V01 is set in the above set resp
           ~(IMS_SETTINGS_OMADM_SERVICES_PRESENCE_ENABLE_BIT_MASK_V01));
      },
      [omadm_mask](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.service_mask_by_network_enabled_valid = TRUE;
        resp.service_mask_by_network_enabled =
          (omadm_mask &
           ~(IMS_SETTINGS_OMADM_SERVICES_PRESENCE_ENABLE_BIT_MASK_V01));
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 1; // WIFI_CALLING_ON
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.wifi_calling_enabled_valid = TRUE;
        req.wifi_calling_enabled = TRUE;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.wifi_calling_enabled_valid = TRUE;
        resp.wifi_calling_enabled = TRUE;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 2;  // WIFI_CALLING_OFF
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.wifi_calling_enabled_valid = TRUE;
        req.wifi_calling_enabled = FALSE;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.wifi_calling_enabled_valid = TRUE;
        resp.wifi_calling_enabled = FALSE;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING;
        req.has_intValue = TRUE;
        req.intValue = 0; // ROAMING_DISABLED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.wifi_calling_in_roaming_enabled_valid = TRUE;
        req.wifi_calling_in_roaming_enabled = FALSE;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.wifi_calling_in_roaming_enabled_valid = TRUE;
        resp.wifi_calling_in_roaming_enabled = FALSE;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING;
        req.has_intValue = TRUE;
        req.intValue = 1;  // ROAMING_ENABLED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.wifi_calling_in_roaming_enabled_valid = TRUE;
        req.wifi_calling_in_roaming_enabled = TRUE;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.wifi_calling_in_roaming_enabled_valid = TRUE;
        resp.wifi_calling_in_roaming_enabled = TRUE;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_MODE;
        req.has_intValue = TRUE;
        req.intValue = 0; // WIFI_ONLY/WLAN_ONLY
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.call_mode_preference_ext_valid = TRUE;
        req.call_mode_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_WLAN_ONLY_V01;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.call_mode_preference_ext_valid = TRUE;
        resp.call_mode_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_WLAN_ONLY_V01;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_MODE;
        req.has_intValue = TRUE;
        req.intValue = 1;  // CELLULAR_PREFERRED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.call_mode_preference_ext_valid = TRUE;
        req.call_mode_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_CELLULAR_V01;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.call_mode_preference_ext_valid = TRUE;
        resp.call_mode_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_CELLULAR_V01;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_MODE;
        req.has_intValue = TRUE;
        req.intValue = 2;  // WIFI_PREFERRED/WLAN_PREFERRED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.call_mode_preference_ext_valid = TRUE;
        req.call_mode_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_WIFI_V01;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.call_mode_preference_ext_valid = TRUE;
        resp.call_mode_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_WIFI_V01;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_MODE;
        req.has_intValue = TRUE;
        req.intValue = 10;  // IMS_PREFERRED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.call_mode_preference_ext_valid = TRUE;
        req.call_mode_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_IMS_V01;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.call_mode_preference_ext_valid = TRUE;
        resp.call_mode_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_IMS_V01;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_RTT_SETTING_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 0; // DISABLED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.rtt_enabled_valid = TRUE;
        req.rtt_enabled = FALSE;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.rtt_enabled_valid = TRUE;
        resp.rtt_enabled = FALSE;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_RTT_SETTING_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 1;  // ENABLED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.rtt_enabled_valid = TRUE;
        req.rtt_enabled = TRUE;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.rtt_enabled_valid = TRUE;
        resp.rtt_enabled = TRUE;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SMS_OVER_IP;
        req.has_intValue = TRUE;
        req.intValue = 0; // DISABLED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.sms_enabled_valid = TRUE;
        req.sms_enabled = FALSE;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.sms_enabled_valid = TRUE;
        resp.sms_enabled = FALSE;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SMS_OVER_IP;
        req.has_intValue = TRUE;
        req.intValue = 1;  // ENABLED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.sms_enabled_valid = TRUE;
        req.sms_enabled = TRUE;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.sms_enabled_valid = TRUE;
        resp.sms_enabled = TRUE;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_MOBILE_DATA_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 0; // DISABLED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.mobile_data_enabled_valid = TRUE;
        req.mobile_data_enabled = FALSE;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.mobile_data_enabled_valid = TRUE;
        resp.mobile_data_enabled = FALSE;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_MOBILE_DATA_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 1;  // ENABLED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.mobile_data_enabled_valid = TRUE;
        req.mobile_data_enabled = TRUE;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.mobile_data_enabled_valid = TRUE;
        resp.mobile_data_enabled = TRUE;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS;
        req.has_intValue = TRUE;
        req.intValue = 0; // DISABLED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.presence_enabled_valid = TRUE;
        req.presence_enabled = FALSE;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.presence_enabled_valid = TRUE;
        resp.presence_enabled = FALSE;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS;
        req.has_intValue = TRUE;
        req.intValue = 1;  // ENABLED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.presence_enabled_valid = TRUE;
        req.presence_enabled = TRUE;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.presence_enabled_valid = TRUE;
        resp.presence_enabled = TRUE;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE;
        req.has_intValue = TRUE;
        req.intValue = 0; // WIFI_ONLY/WLAN_ONLY
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.call_mode_roam_preference_ext_valid = TRUE;
        req.call_mode_roam_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_WLAN_ONLY_V01;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.call_mode_roam_preference_ext_valid = TRUE;
        resp.call_mode_roam_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_WLAN_ONLY_V01;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE;
        req.has_intValue = TRUE;
        req.intValue = 1;  // CELLULAR_PREFERRED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.call_mode_roam_preference_ext_valid = TRUE;
        req.call_mode_roam_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_CELLULAR_V01;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.call_mode_roam_preference_ext_valid = TRUE;
        resp.call_mode_roam_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_CELLULAR_V01;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE;
        req.has_intValue = TRUE;
        req.intValue = 2;  // WIFI_PREFERRED/WLAN_PREFERRED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.call_mode_roam_preference_ext_valid = TRUE;
        req.call_mode_roam_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_WIFI_V01;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.call_mode_roam_preference_ext_valid = TRUE;
        resp.call_mode_roam_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_WIFI_V01;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE;
        req.has_intValue = TRUE;
        req.intValue = 10;  // IMS_PREFERRED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.call_mode_roam_preference_ext_valid = TRUE;
        req.call_mode_roam_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_IMS_V01;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        resp.call_mode_roam_preference_ext_valid = TRUE;
        resp.call_mode_roam_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_IMS_V01;
      }),
  };


  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_set_ims_service_enable_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_ims_service_enable_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.expect(IMS_node);

    // TODO: no support for get ims config for CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE
    if (testReq.imsReq.item != ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE) {

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_ims_service_enable_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_ims_service_enable_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    *ims_ptr = testReq.imsReq;
    t.expect(IMS_node);

    }
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_ims_service_enable_config_002(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items ims_service_enable_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_ims_service_enable_config_req_msg_v01 qmiReq;
    ims_settings_get_ims_service_enable_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_ims_service_enable_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_ims_service_enable_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };

  ims_settings_omadm_services_type_mask_v01 omadm_mask = IMS_SETTINGS_OMADM_SERVICES_BIT_MASK_NONE_V01;
    //(IMS_SETTINGS_OMADM_SERVICES_BIT_MASK_NONE_V01 |
    // IMS_SETTINGS_OMADM_SERVICES_ENABLE_BIT_MASK_V01);

  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VLT_SETTING_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 1;
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.service_mask_by_network_enabled_valid = TRUE;
        req.service_mask_by_network_enabled =
          (IMS_SETTINGS_OMADM_SERVICES_BIT_MASK_NONE_V01 |
           IMS_SETTINGS_OMADM_SERVICES_VOLTE_ENABLE_BIT_MASK_V01 |
           IMS_SETTINGS_OMADM_SERVICES_ENABLE_BIT_MASK_V01);
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        //resp.service_mask_by_network_enabled_valid = TRUE;
        //resp.service_mask_by_network_enabled =
        //  (IMS_SETTINGS_OMADM_SERVICES_BIT_MASK_NONE_V01 |
        //   IMS_SETTINGS_OMADM_SERVICES_VOLTE_ENABLE_BIT_MASK_V01 |
        //   IMS_SETTINGS_OMADM_SERVICES_ENABLE_BIT_MASK_V01);
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VLT_SETTING_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 0;
      },
      [omadm_mask](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.service_mask_by_network_enabled_valid = TRUE;
        req.service_mask_by_network_enabled =
          (omadm_mask &   // IMS_SETTINGS_OMADM_SERVICES_ENABLE_BIT_MASK_V01 is set in the above set resp
           ~(IMS_SETTINGS_OMADM_SERVICES_VOLTE_ENABLE_BIT_MASK_V01));
      },
      [omadm_mask](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        //resp.service_mask_by_network_enabled_valid = TRUE;
        //resp.service_mask_by_network_enabled =
        //  (omadm_mask &
        //   ~(IMS_SETTINGS_OMADM_SERVICES_VOLTE_ENABLE_BIT_MASK_V01));
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_LVC_SETTING_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 1;
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.service_mask_by_network_enabled_valid = TRUE;
        req.service_mask_by_network_enabled =
          (IMS_SETTINGS_OMADM_SERVICES_BIT_MASK_NONE_V01 |
           IMS_SETTINGS_OMADM_SERVICES_VT_ENABLE_BIT_MASK_V01 |
           IMS_SETTINGS_OMADM_SERVICES_ENABLE_BIT_MASK_V01);
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        //resp.service_mask_by_network_enabled_valid = TRUE;
        //resp.service_mask_by_network_enabled =
        //  (IMS_SETTINGS_OMADM_SERVICES_BIT_MASK_NONE_V01 |
        //   IMS_SETTINGS_OMADM_SERVICES_VT_ENABLE_BIT_MASK_V01 |
        //   IMS_SETTINGS_OMADM_SERVICES_ENABLE_BIT_MASK_V01);
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_LVC_SETTING_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 0;
      },
      [omadm_mask](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.service_mask_by_network_enabled_valid = TRUE;
        req.service_mask_by_network_enabled =
          (omadm_mask &   // IMS_SETTINGS_OMADM_SERVICES_ENABLE_BIT_MASK_V01 is set in the above set resp
           ~(IMS_SETTINGS_OMADM_SERVICES_VT_ENABLE_BIT_MASK_V01));
      },
      [omadm_mask](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        //resp.service_mask_by_network_enabled_valid = TRUE;
        //resp.service_mask_by_network_enabled =
        //  (omadm_mask &
        //   ~(IMS_SETTINGS_OMADM_SERVICES_VT_ENABLE_BIT_MASK_V01));
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VWF_SETTING_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 1;
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.service_mask_by_network_enabled_valid = TRUE;
        req.service_mask_by_network_enabled =
          (IMS_SETTINGS_OMADM_SERVICES_BIT_MASK_NONE_V01 |
           IMS_SETTINGS_OMADM_SERVICES_VOWIFI_ENABLE_BIT_MASK_V01 |
           IMS_SETTINGS_OMADM_SERVICES_ENABLE_BIT_MASK_V01);
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        //resp.service_mask_by_network_enabled_valid = TRUE;
        //resp.service_mask_by_network_enabled =
        //  (IMS_SETTINGS_OMADM_SERVICES_BIT_MASK_NONE_V01 |
        //   IMS_SETTINGS_OMADM_SERVICES_VOWIFI_ENABLE_BIT_MASK_V01 |
        //   IMS_SETTINGS_OMADM_SERVICES_ENABLE_BIT_MASK_V01);
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VWF_SETTING_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 0;
      },
      [omadm_mask](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.service_mask_by_network_enabled_valid = TRUE;
        req.service_mask_by_network_enabled =
          (omadm_mask &   // IMS_SETTINGS_OMADM_SERVICES_ENABLE_BIT_MASK_V01 is set in the above set resp
           ~(IMS_SETTINGS_OMADM_SERVICES_VOWIFI_ENABLE_BIT_MASK_V01));
      },
      [omadm_mask](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        //resp.service_mask_by_network_enabled_valid = TRUE;
        //resp.service_mask_by_network_enabled =
        //  (omadm_mask &
        //   ~(IMS_SETTINGS_OMADM_SERVICES_VOWIFI_ENABLE_BIT_MASK_V01));
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_EAB_SETTING_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 1;
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.service_mask_by_network_enabled_valid = TRUE;
        req.service_mask_by_network_enabled =
          (IMS_SETTINGS_OMADM_SERVICES_BIT_MASK_NONE_V01 |
           IMS_SETTINGS_OMADM_SERVICES_PRESENCE_ENABLE_BIT_MASK_V01 |
           IMS_SETTINGS_OMADM_SERVICES_ENABLE_BIT_MASK_V01);
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        //resp.service_mask_by_network_enabled_valid = TRUE;
        //resp.service_mask_by_network_enabled =
        //  (IMS_SETTINGS_OMADM_SERVICES_BIT_MASK_NONE_V01 |
        //   IMS_SETTINGS_OMADM_SERVICES_PRESENCE_ENABLE_BIT_MASK_V01 |
        //   IMS_SETTINGS_OMADM_SERVICES_ENABLE_BIT_MASK_V01);
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_EAB_SETTING_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 0;
      },
      [omadm_mask](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.service_mask_by_network_enabled_valid = TRUE;
        req.service_mask_by_network_enabled =
          (omadm_mask &   // IMS_SETTINGS_OMADM_SERVICES_ENABLE_BIT_MASK_V01 is set in the above set resp
           ~(IMS_SETTINGS_OMADM_SERVICES_PRESENCE_ENABLE_BIT_MASK_V01));
      },
      [omadm_mask](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        //resp.service_mask_by_network_enabled_valid = TRUE;
        //resp.service_mask_by_network_enabled =
        //  (omadm_mask &
        //   ~(IMS_SETTINGS_OMADM_SERVICES_PRESENCE_ENABLE_BIT_MASK_V01));
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 1; // WIFI_CALLING_ON
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.wifi_calling_enabled_valid = TRUE;
        req.wifi_calling_enabled = TRUE;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        //resp.wifi_calling_enabled_valid = TRUE;
        //resp.wifi_calling_enabled = TRUE;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 2;  // WIFI_CALLING_OFF
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.wifi_calling_enabled_valid = TRUE;
        req.wifi_calling_enabled = FALSE;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        //resp.wifi_calling_enabled_valid = TRUE;
        //resp.wifi_calling_enabled = FALSE;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING;
        req.has_intValue = TRUE;
        req.intValue = 0; // ROAMING_DISABLED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.wifi_calling_in_roaming_enabled_valid = TRUE;
        req.wifi_calling_in_roaming_enabled = FALSE;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        //resp.wifi_calling_in_roaming_enabled_valid = TRUE;
        //resp.wifi_calling_in_roaming_enabled = FALSE;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING;
        req.has_intValue = TRUE;
        req.intValue = 1;  // ROAMING_ENABLED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.wifi_calling_in_roaming_enabled_valid = TRUE;
        req.wifi_calling_in_roaming_enabled = TRUE;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        //resp.wifi_calling_in_roaming_enabled_valid = TRUE;
        //resp.wifi_calling_in_roaming_enabled = TRUE;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_MODE;
        req.has_intValue = TRUE;
        req.intValue = 0; // WIFI_ONLY/WLAN_ONLY
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.call_mode_preference_ext_valid = TRUE;
        req.call_mode_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_WLAN_ONLY_V01;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        //resp.call_mode_preference_ext_valid = TRUE;
        //resp.call_mode_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_WLAN_ONLY_V01;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_MODE;
        req.has_intValue = TRUE;
        req.intValue = 1;  // CELLULAR_PREFERRED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.call_mode_preference_ext_valid = TRUE;
        req.call_mode_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_CELLULAR_V01;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        //resp.call_mode_preference_ext_valid = TRUE;
        //resp.call_mode_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_CELLULAR_V01;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_MODE;
        req.has_intValue = TRUE;
        req.intValue = 2;  // WIFI_PREFERRED/WLAN_PREFERRED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.call_mode_preference_ext_valid = TRUE;
        req.call_mode_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_WIFI_V01;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        //resp.call_mode_preference_ext_valid = TRUE;
        //resp.call_mode_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_WIFI_V01;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_RTT_SETTING_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 0; // DISABLED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.rtt_enabled_valid = TRUE;
        req.rtt_enabled = FALSE;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        //resp.rtt_enabled_valid = TRUE;
        //resp.rtt_enabled = FALSE;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_RTT_SETTING_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 1;  // ENABLED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.rtt_enabled_valid = TRUE;
        req.rtt_enabled = TRUE;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        //resp.rtt_enabled_valid = TRUE;
        //resp.rtt_enabled = TRUE;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SMS_OVER_IP;
        req.has_intValue = TRUE;
        req.intValue = 0; // DISABLED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.sms_enabled_valid = TRUE;
        req.sms_enabled = FALSE;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        //resp.sms_enabled_valid = TRUE;
        //resp.sms_enabled = FALSE;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SMS_OVER_IP;
        req.has_intValue = TRUE;
        req.intValue = 1;  // ENABLED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.sms_enabled_valid = TRUE;
        req.sms_enabled = TRUE;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        //resp.sms_enabled_valid = TRUE;
        //resp.sms_enabled = TRUE;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_MOBILE_DATA_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 0; // DISABLED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.mobile_data_enabled_valid = TRUE;
        req.mobile_data_enabled = FALSE;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        //resp.mobile_data_enabled_valid = TRUE;
        //resp.mobile_data_enabled = FALSE;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_MOBILE_DATA_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 1;  // ENABLED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.mobile_data_enabled_valid = TRUE;
        req.mobile_data_enabled = TRUE;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        //resp.mobile_data_enabled_valid = TRUE;
        //resp.mobile_data_enabled = TRUE;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS;
        req.has_intValue = TRUE;
        req.intValue = 0; // DISABLED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.presence_enabled_valid = TRUE;
        req.presence_enabled = FALSE;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        //resp.presence_enabled_valid = TRUE;
        //resp.presence_enabled = FALSE;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS;
        req.has_intValue = TRUE;
        req.intValue = 1;  // ENABLED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.presence_enabled_valid = TRUE;
        req.presence_enabled = TRUE;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        //resp.presence_enabled_valid = TRUE;
        //resp.presence_enabled = TRUE;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE;
        req.has_intValue = TRUE;
        req.intValue = 0; // WIFI_ONLY/WLAN_ONLY
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.call_mode_roam_preference_ext_valid = TRUE;
        req.call_mode_roam_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_WLAN_ONLY_V01;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        //resp.call_mode_roam_preference_ext_valid = TRUE;
        //resp.call_mode_roam_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_WLAN_ONLY_V01;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE;
        req.has_intValue = TRUE;
        req.intValue = 1;  // CELLULAR_PREFERRED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.call_mode_roam_preference_ext_valid = TRUE;
        req.call_mode_roam_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_CELLULAR_V01;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        //resp.call_mode_roam_preference_ext_valid = TRUE;
        //resp.call_mode_roam_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_CELLULAR_V01;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE;
        req.has_intValue = TRUE;
        req.intValue = 2;  // WIFI_PREFERRED/WLAN_PREFERRED
      },
      [](ims_settings_set_ims_service_enable_config_req_msg_v01 &req) -> void {
        req.call_mode_roam_preference_ext_valid = TRUE;
        req.call_mode_roam_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_WIFI_V01;
      },
      [](ims_settings_get_ims_service_enable_config_rsp_msg_v01 &resp) -> void {
        //resp.call_mode_roam_preference_ext_valid = TRUE;
        //resp.call_mode_roam_preference_ext = IMS_SETTINGS_CALL_MODE_PREFERRED_WIFI_V01;
      }),
  };


  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_set_ims_service_enable_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_ims_service_enable_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.expect(IMS_node);

    // TODO: no support for get ims config for CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE
    if (testReq.imsReq.item != ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE) {

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_ims_service_enable_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_ims_service_enable_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    //*ims_ptr = testReq.imsReq;
    IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
    t.expect(IMS_node);

    }
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_ims_service_enable_config_003(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items service_enable_config: GET FAILURE.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 1; // WIFI_CALLING_ON
  t.inject(IMS_node);

  ims_settings_set_ims_service_enable_config_req_msg_v01 *set_req =
    QMI_node.update_default_qmi_ims_settings_set_ims_service_enable_config_req();
  set_req->wifi_calling_enabled_valid = TRUE;
  set_req->wifi_calling_enabled = TRUE;
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_ims_service_enable_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 1; // WIFI_CALLING_ON
  t.inject(IMS_node);

  set_req = QMI_node.update_default_qmi_ims_settings_set_ims_service_enable_config_req();
  set_req->wifi_calling_enabled_valid = TRUE;
  set_req->wifi_calling_enabled = TRUE;
  t.expect(QMI_node);

  ims_settings_set_ims_service_enable_config_rsp_msg_v01 *set_resp =
    QMI_node.update_default_qmi_ims_settings_set_ims_service_enable_config_resp(
        QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 1; // WIFI_CALLING_ON
  t.inject(IMS_node);

  set_req = QMI_node.update_default_qmi_ims_settings_set_ims_service_enable_config_req();
  set_req->wifi_calling_enabled_valid = TRUE;
  set_req->wifi_calling_enabled = TRUE;
  t.expect(QMI_node);

  set_resp = QMI_node.update_default_qmi_ims_settings_set_ims_service_enable_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  set_resp->settings_resp_valid = TRUE;
  set_resp->settings_resp = IMS_SETTINGS_MSG_WRITE_FAILED_V01;
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
  ims_ptr->has_errorCause = TRUE;
  ims_ptr->errorCause = ims_ConfigFailureCause_CONFIG_WRITE_FAILED;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);


  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_ims_service_enable_config_req();
  t.expect(QMI_node);

  ims_settings_get_ims_service_enable_config_rsp_msg_v01 *get_resp =
    QMI_node.update_default_qmi_ims_settings_get_ims_service_enable_config_resp(
        QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_ims_service_enable_config_req();
  t.expect(QMI_node);

  get_resp = QMI_node.update_default_qmi_ims_settings_get_ims_service_enable_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_ims_service_enable_config_req();
  t.expect(QMI_node);

  get_resp = QMI_node.update_default_qmi_ims_settings_get_ims_service_enable_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  get_resp->settings_resp_valid = TRUE;
  get_resp->settings_resp = IMS_SETTINGS_MSG_OTHER_INTERNAL_ERR_V01;
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
  ims_ptr->has_errorCause = TRUE;
  ims_ptr->errorCause = ims_ConfigFailureCause_CONFIG_OTHER_INTERNAL_ERR;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_sip_new_config_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items sip_new_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_sip_new_config_req_msg_v01 qmiReq;
    ims_settings_get_sip_new_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_sip_new_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_sip_new_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };


  std::vector<ImsConfigRequest> testReqVector {
#if 0
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_CANCELLATION_TIMER;
        req.has_intValue = TRUE;
        req.intValue = 30;
      },
      [](ims_settings_set_sip_new_config_req_msg_v01 &req) -> void {
        req.sip_timer_operator_mode_a_valid = TRUE;
        req.sip_timer_operator_mode_a = 30;
      },
      [](ims_settings_get_sip_new_config_rsp_msg_v01 &resp) -> void {
        resp.sip_timer_operator_mode_a_valid = TRUE;
        resp.sip_timer_operator_mode_a = 30;
      }),
#endif
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SIP_T1_TIMER;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_sip_new_config_req_msg_v01 &req) -> void {
        req.timer_t1_valid = TRUE;
        req.timer_t1 = 105;
      },
      [](ims_settings_get_sip_new_config_rsp_msg_v01 &resp) -> void {
        resp.timer_t1_valid = TRUE;
        resp.timer_t1 = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SIP_T2_TIMER;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_sip_new_config_req_msg_v01 &req) -> void {
        req.timer_t2_valid = TRUE;
        req.timer_t2 = 105;
      },
      [](ims_settings_get_sip_new_config_rsp_msg_v01 &resp) -> void {
        resp.timer_t2_valid = TRUE;
        resp.timer_t2 = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SIP_TF_TIMER;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_sip_new_config_req_msg_v01 &req) -> void {
        req.timer_tf_valid = TRUE;
        req.timer_tf = 105;
      },
      [](ims_settings_get_sip_new_config_rsp_msg_v01 &resp) -> void {
        resp.timer_tf_valid = TRUE;
        resp.timer_tf = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SIP_INVITE_REQ_RETX_INTERVAL_MSEC;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_sip_new_config_req_msg_v01 &req) -> void {
        req.timer_ta_valid = TRUE;
        req.timer_ta = 105;
      },
      [](ims_settings_get_sip_new_config_rsp_msg_v01 &resp) -> void {
        resp.timer_ta_valid = TRUE;
        resp.timer_ta = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SIP_INVITE_RSP_WAIT_TIME_MSEC;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_sip_new_config_req_msg_v01 &req) -> void {
        req.timer_tb_valid = TRUE;
        req.timer_tb = 105;
      },
      [](ims_settings_get_sip_new_config_rsp_msg_v01 &resp) -> void {
        resp.timer_tb_valid = TRUE;
        resp.timer_tb = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_sip_new_config_req_msg_v01 &req) -> void {
        req.timer_td_valid = TRUE;
        req.timer_td = 105;
      },
      [](ims_settings_get_sip_new_config_rsp_msg_v01 &resp) -> void {
        resp.timer_td_valid = TRUE;
        resp.timer_td = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_sip_new_config_req_msg_v01 &req) -> void {
        req.timer_te_valid = TRUE;
        req.timer_te = 105;
      },
      [](ims_settings_get_sip_new_config_rsp_msg_v01 &resp) -> void {
        resp.timer_te_valid = TRUE;
        resp.timer_te = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SIP_INVITE_RSP_RETX_INTERVAL_MSEC;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_sip_new_config_req_msg_v01 &req) -> void {
        req.timer_tg_valid = TRUE;
        req.timer_tg = 105;
      },
      [](ims_settings_get_sip_new_config_rsp_msg_v01 &resp) -> void {
        resp.timer_tg_valid = TRUE;
        resp.timer_tg = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SIP_ACK_RECEIPT_WAIT_TIME_MSEC;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_sip_new_config_req_msg_v01 &req) -> void {
        req.timer_th_valid = TRUE;
        req.timer_th = 105;
      },
      [](ims_settings_get_sip_new_config_rsp_msg_v01 &resp) -> void {
        resp.timer_th_valid = TRUE;
        resp.timer_th = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SIP_ACK_RETX_WAIT_TIME_MSEC;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_sip_new_config_req_msg_v01 &req) -> void {
        req.timer_ti_valid = TRUE;
        req.timer_ti = 105;
      },
      [](ims_settings_get_sip_new_config_rsp_msg_v01 &resp) -> void {
        resp.timer_ti_valid = TRUE;
        resp.timer_ti = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_sip_new_config_req_msg_v01 &req) -> void {
        req.timer_tj_valid = TRUE;
        req.timer_tj = 105;
      },
      [](ims_settings_get_sip_new_config_rsp_msg_v01 &resp) -> void {
        resp.timer_tj_valid = TRUE;
        resp.timer_tj = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_sip_new_config_req_msg_v01 &req) -> void {
        req.timer_tk_valid = TRUE;
        req.timer_tk = 105;
      },
      [](ims_settings_get_sip_new_config_rsp_msg_v01 &resp) -> void {
        resp.timer_tk_valid = TRUE;
        resp.timer_tk = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_KEEP_ALIVE_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 1;
//        req.has_boolValue = TRUE;
//        req.boolValue = 1;
      },
      [](ims_settings_set_sip_new_config_req_msg_v01 &req) -> void {
        req.keepalive_enabled_valid = TRUE;
        req.keepalive_enabled = TRUE;
      },
      [](ims_settings_get_sip_new_config_rsp_msg_v01 &resp) -> void {
        resp.keepalive_enabled_valid = TRUE;
        resp.keepalive_enabled = TRUE;
      }),
  };


  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_set_sip_new_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_sip_new_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.expect(IMS_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_sip_new_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_sip_new_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    *ims_ptr = testReq.imsReq;
    t.expect(IMS_node);
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_sip_new_config_002(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items sip_new_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_sip_new_config_req_msg_v01 qmiReq;
    ims_settings_get_sip_new_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_sip_new_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_sip_new_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };


  std::vector<ImsConfigRequest> testReqVector {
#if 0
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_CANCELLATION_TIMER;
        req.has_intValue = TRUE;
        req.intValue = 30;
      },
      [](ims_settings_set_sip_new_config_req_msg_v01 &req) -> void {
        req.sip_timer_operator_mode_a_valid = TRUE;
        req.sip_timer_operator_mode_a = 30;
      },
      [](ims_settings_get_sip_new_config_rsp_msg_v01 &resp) -> void {
        //resp.sip_timer_operator_mode_a_valid = TRUE;
        //resp.sip_timer_operator_mode_a = 30;
      }),
#endif
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SIP_T1_TIMER;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_sip_new_config_req_msg_v01 &req) -> void {
        req.timer_t1_valid = TRUE;
        req.timer_t1 = 105;
      },
      [](ims_settings_get_sip_new_config_rsp_msg_v01 &resp) -> void {
        //resp.timer_t1_valid = TRUE;
        //resp.timer_t1 = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SIP_T2_TIMER;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_sip_new_config_req_msg_v01 &req) -> void {
        req.timer_t2_valid = TRUE;
        req.timer_t2 = 105;
      },
      [](ims_settings_get_sip_new_config_rsp_msg_v01 &resp) -> void {
        //resp.timer_t2_valid = TRUE;
        //resp.timer_t2 = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SIP_TF_TIMER;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_sip_new_config_req_msg_v01 &req) -> void {
        req.timer_tf_valid = TRUE;
        req.timer_tf = 105;
      },
      [](ims_settings_get_sip_new_config_rsp_msg_v01 &resp) -> void {
        //resp.timer_tf_valid = TRUE;
        //resp.timer_tf = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SIP_INVITE_REQ_RETX_INTERVAL_MSEC;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_sip_new_config_req_msg_v01 &req) -> void {
        req.timer_ta_valid = TRUE;
        req.timer_ta = 105;
      },
      [](ims_settings_get_sip_new_config_rsp_msg_v01 &resp) -> void {
        //resp.timer_ta_valid = TRUE;
        //resp.timer_ta = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SIP_INVITE_RSP_WAIT_TIME_MSEC;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_sip_new_config_req_msg_v01 &req) -> void {
        req.timer_tb_valid = TRUE;
        req.timer_tb = 105;
      },
      [](ims_settings_get_sip_new_config_rsp_msg_v01 &resp) -> void {
        //resp.timer_tb_valid = TRUE;
        //resp.timer_tb = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_sip_new_config_req_msg_v01 &req) -> void {
        req.timer_td_valid = TRUE;
        req.timer_td = 105;
      },
      [](ims_settings_get_sip_new_config_rsp_msg_v01 &resp) -> void {
        //resp.timer_td_valid = TRUE;
        //resp.timer_td = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_sip_new_config_req_msg_v01 &req) -> void {
        req.timer_te_valid = TRUE;
        req.timer_te = 105;
      },
      [](ims_settings_get_sip_new_config_rsp_msg_v01 &resp) -> void {
        //resp.timer_te_valid = TRUE;
        //resp.timer_te = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SIP_INVITE_RSP_RETX_INTERVAL_MSEC;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_sip_new_config_req_msg_v01 &req) -> void {
        req.timer_tg_valid = TRUE;
        req.timer_tg = 105;
      },
      [](ims_settings_get_sip_new_config_rsp_msg_v01 &resp) -> void {
        //resp.timer_tg_valid = TRUE;
        //resp.timer_tg = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SIP_ACK_RECEIPT_WAIT_TIME_MSEC;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_sip_new_config_req_msg_v01 &req) -> void {
        req.timer_th_valid = TRUE;
        req.timer_th = 105;
      },
      [](ims_settings_get_sip_new_config_rsp_msg_v01 &resp) -> void {
        //resp.timer_th_valid = TRUE;
        //resp.timer_th = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SIP_ACK_RETX_WAIT_TIME_MSEC;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_sip_new_config_req_msg_v01 &req) -> void {
        req.timer_ti_valid = TRUE;
        req.timer_ti = 105;
      },
      [](ims_settings_get_sip_new_config_rsp_msg_v01 &resp) -> void {
        //resp.timer_ti_valid = TRUE;
        //resp.timer_ti = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_sip_new_config_req_msg_v01 &req) -> void {
        req.timer_tj_valid = TRUE;
        req.timer_tj = 105;
      },
      [](ims_settings_get_sip_new_config_rsp_msg_v01 &resp) -> void {
        //resp.timer_tj_valid = TRUE;
        //resp.timer_tj = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_sip_new_config_req_msg_v01 &req) -> void {
        req.timer_tk_valid = TRUE;
        req.timer_tk = 105;
      },
      [](ims_settings_get_sip_new_config_rsp_msg_v01 &resp) -> void {
        //resp.timer_tk_valid = TRUE;
        //resp.timer_tk = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_KEEP_ALIVE_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 1;
//        req.has_boolValue = TRUE;
//        req.boolValue = 1;
      },
      [](ims_settings_set_sip_new_config_req_msg_v01 &req) -> void {
        req.keepalive_enabled_valid = TRUE;
        req.keepalive_enabled = TRUE;
      },
      [](ims_settings_get_sip_new_config_rsp_msg_v01 &resp) -> void {
        //resp.keepalive_enabled_valid = TRUE;
        //resp.keepalive_enabled = TRUE;
      }),
  };


  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_set_sip_new_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_sip_new_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.expect(IMS_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_sip_new_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_sip_new_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    //*ims_ptr = testReq.imsReq;
    IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
    t.expect(IMS_node);
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_sip_new_config_003(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items sip_new_config: GET FAILURE.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_T1_TIMER;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 100;
  t.inject(IMS_node);

  ims_settings_set_sip_new_config_req_msg_v01 *set_req =
    QMI_node.update_default_qmi_ims_settings_set_sip_new_config_req();
  set_req->timer_t1_valid = TRUE;
  set_req->timer_t1 = 100;
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_sip_new_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_T1_TIMER;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 100;
  t.inject(IMS_node);

  set_req = QMI_node.update_default_qmi_ims_settings_set_sip_new_config_req();
  set_req->timer_t1_valid = TRUE;
  set_req->timer_t1 = 100;
  t.expect(QMI_node);

  ims_settings_set_sip_new_config_rsp_msg_v01 *set_resp =
    QMI_node.update_default_qmi_ims_settings_set_sip_new_config_resp(
        QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_T1_TIMER;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 100;
  t.inject(IMS_node);

  set_req = QMI_node.update_default_qmi_ims_settings_set_sip_new_config_req();
  set_req->timer_t1_valid = TRUE;
  set_req->timer_t1 = 100;
  t.expect(QMI_node);

  set_resp = QMI_node.update_default_qmi_ims_settings_set_sip_new_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  set_resp->settings_resp_valid = TRUE;
  set_resp->settings_resp = IMS_SETTINGS_MSG_WRITE_FAILED_V01;
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_T1_TIMER;
  ims_ptr->has_errorCause = TRUE;
  ims_ptr->errorCause = ims_ConfigFailureCause_CONFIG_WRITE_FAILED;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);


  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_T1_TIMER;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_sip_new_config_req();
  t.expect(QMI_node);

  ims_settings_get_sip_new_config_rsp_msg_v01 *get_resp =
    QMI_node.update_default_qmi_ims_settings_get_sip_new_config_resp(
        QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_T1_TIMER;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_sip_new_config_req();
  t.expect(QMI_node);

  get_resp = QMI_node.update_default_qmi_ims_settings_get_sip_new_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_T1_TIMER;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_sip_new_config_req();
  t.expect(QMI_node);

  get_resp = QMI_node.update_default_qmi_ims_settings_get_sip_new_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  get_resp->settings_resp_valid = TRUE;
  get_resp->settings_resp = IMS_SETTINGS_MSG_READ_FAILED_V01;
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_T1_TIMER;
  ims_ptr->has_errorCause = TRUE;
  ims_ptr->errorCause = ims_ConfigFailureCause_CONFIG_READ_FAILED;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_presence_new_config_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items presence_new_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_presence_new_config_req_msg_v01 qmiReq;
    ims_settings_get_presence_new_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_presence_new_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_presence_new_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };


  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_PUBLISH_TIMER;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_presence_new_config_req_msg_v01 &req) -> void {
        req.publish_timer_valid = TRUE;
        req.publish_timer = 105;
      },
      [](ims_settings_get_presence_new_config_rsp_msg_v01 &resp) -> void {
        resp.publish_timer_valid = TRUE;
        resp.publish_timer = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_PUBLISH_TIMER_EXTENDED;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_presence_new_config_req_msg_v01 &req) -> void {
        req.publish_extended_timer_valid = TRUE;
        req.publish_extended_timer = 105;
      },
      [](ims_settings_get_presence_new_config_rsp_msg_v01 &resp) -> void {
        resp.publish_extended_timer_valid = TRUE;
        resp.publish_extended_timer = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_presence_new_config_req_msg_v01 &req) -> void {
        req.capabilites_cache_expiry_valid = TRUE;
        req.capabilites_cache_expiry = 105;
      },
      [](ims_settings_get_presence_new_config_rsp_msg_v01 &resp) -> void {
        resp.capabilites_cache_expiry_valid = TRUE;
        resp.capabilites_cache_expiry = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_presence_new_config_req_msg_v01 &req) -> void {
        req.availability_cache_expiry_valid = TRUE;
        req.availability_cache_expiry = 105;
      },
      [](ims_settings_get_presence_new_config_rsp_msg_v01 &resp) -> void {
        resp.availability_cache_expiry_valid = TRUE;
        resp.availability_cache_expiry = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_presence_new_config_req_msg_v01 &req) -> void {
        req.capability_poll_interval_valid = TRUE;
        req.capability_poll_interval = 105;
      },
      [](ims_settings_get_presence_new_config_rsp_msg_v01 &resp) -> void {
        resp.capability_poll_interval_valid = TRUE;
        resp.capability_poll_interval = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_presence_new_config_req_msg_v01 &req) -> void {
        req.publish_src_throttle_timer_valid = TRUE;
        req.publish_src_throttle_timer = 105;
      },
      [](ims_settings_get_presence_new_config_rsp_msg_v01 &resp) -> void {
        resp.publish_src_throttle_timer_valid = TRUE;
        resp.publish_src_throttle_timer = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_presence_new_config_req_msg_v01 &req) -> void {
        req.max_enties_in_list_subscribe_valid = TRUE;
        req.max_enties_in_list_subscribe = 105;
      },
      [](ims_settings_get_presence_new_config_rsp_msg_v01 &resp) -> void {
        resp.max_enties_in_list_subscribe_valid = TRUE;
        resp.max_enties_in_list_subscribe = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_presence_new_config_req_msg_v01 &req) -> void {
        req.list_subscription_expiry_valid = TRUE;
        req.list_subscription_expiry = 105;
      },
      [](ims_settings_get_presence_new_config_rsp_msg_v01 &resp) -> void {
        resp.list_subscription_expiry_valid = TRUE;
        resp.list_subscription_expiry = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_GZIP_FLAG;
        req.has_intValue = TRUE;
        req.intValue = 1;
      },
      [](ims_settings_set_presence_new_config_req_msg_v01 &req) -> void {
        req.gzip_enabled_valid = TRUE;
        req.gzip_enabled = 1;
      },
      [](ims_settings_get_presence_new_config_rsp_msg_v01 &resp) -> void {
        resp.gzip_enabled_valid = TRUE;
        resp.gzip_enabled = 1;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_CAPABILITY_DISCOVERY_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 1;
      },
      [](ims_settings_set_presence_new_config_req_msg_v01 &req) -> void {
        req.capability_discovery_enabled_valid = TRUE;
        req.capability_discovery_enabled = 1;
      },
      [](ims_settings_get_presence_new_config_rsp_msg_v01 &resp) -> void {
        resp.capability_discovery_enabled_valid = TRUE;
        resp.capability_discovery_enabled = 1;
      }),
  };


  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_set_presence_new_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_presence_new_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.expect(IMS_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_presence_new_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_presence_new_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    *ims_ptr = testReq.imsReq;
    t.expect(IMS_node);
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_presence_new_config_002(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items presence_new_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_presence_new_config_req_msg_v01 qmiReq;
    ims_settings_get_presence_new_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_presence_new_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_presence_new_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };


  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_PUBLISH_TIMER;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_presence_new_config_req_msg_v01 &req) -> void {
        req.publish_timer_valid = TRUE;
        req.publish_timer = 105;
      },
      [](ims_settings_get_presence_new_config_rsp_msg_v01 &resp) -> void {
        //resp.publish_timer_valid = TRUE;
        //resp.publish_timer = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_PUBLISH_TIMER_EXTENDED;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_presence_new_config_req_msg_v01 &req) -> void {
        req.publish_extended_timer_valid = TRUE;
        req.publish_extended_timer = 105;
      },
      [](ims_settings_get_presence_new_config_rsp_msg_v01 &resp) -> void {
        //resp.publish_extended_timer_valid = TRUE;
        //resp.publish_extended_timer = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_presence_new_config_req_msg_v01 &req) -> void {
        req.capabilites_cache_expiry_valid = TRUE;
        req.capabilites_cache_expiry = 105;
      },
      [](ims_settings_get_presence_new_config_rsp_msg_v01 &resp) -> void {
        //resp.capabilites_cache_expiry_valid = TRUE;
        //resp.capabilites_cache_expiry = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_presence_new_config_req_msg_v01 &req) -> void {
        req.availability_cache_expiry_valid = TRUE;
        req.availability_cache_expiry = 105;
      },
      [](ims_settings_get_presence_new_config_rsp_msg_v01 &resp) -> void {
        //resp.availability_cache_expiry_valid = TRUE;
        //resp.availability_cache_expiry = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_presence_new_config_req_msg_v01 &req) -> void {
        req.capability_poll_interval_valid = TRUE;
        req.capability_poll_interval = 105;
      },
      [](ims_settings_get_presence_new_config_rsp_msg_v01 &resp) -> void {
        //resp.capability_poll_interval_valid = TRUE;
        //resp.capability_poll_interval = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_presence_new_config_req_msg_v01 &req) -> void {
        req.publish_src_throttle_timer_valid = TRUE;
        req.publish_src_throttle_timer = 105;
      },
      [](ims_settings_get_presence_new_config_rsp_msg_v01 &resp) -> void {
        //resp.publish_src_throttle_timer_valid = TRUE;
        //resp.publish_src_throttle_timer = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_presence_new_config_req_msg_v01 &req) -> void {
        req.max_enties_in_list_subscribe_valid = TRUE;
        req.max_enties_in_list_subscribe = 105;
      },
      [](ims_settings_get_presence_new_config_rsp_msg_v01 &resp) -> void {
        //resp.max_enties_in_list_subscribe_valid = TRUE;
        //resp.max_enties_in_list_subscribe = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_presence_new_config_req_msg_v01 &req) -> void {
        req.list_subscription_expiry_valid = TRUE;
        req.list_subscription_expiry = 105;
      },
      [](ims_settings_get_presence_new_config_rsp_msg_v01 &resp) -> void {
        //resp.list_subscription_expiry_valid = TRUE;
        //resp.list_subscription_expiry = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_GZIP_FLAG;
        req.has_intValue = TRUE;
        req.intValue = 1;
      },
      [](ims_settings_set_presence_new_config_req_msg_v01 &req) -> void {
        req.gzip_enabled_valid = TRUE;
        req.gzip_enabled = 1;
      },
      [](ims_settings_get_presence_new_config_rsp_msg_v01 &resp) -> void {
        //resp.gzip_enabled_valid = TRUE;
        //resp.gzip_enabled = 1;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_CAPABILITY_DISCOVERY_ENABLED;
        req.has_intValue = TRUE;
        req.intValue = 1;
      },
      [](ims_settings_set_presence_new_config_req_msg_v01 &req) -> void {
        req.capability_discovery_enabled_valid = TRUE;
        req.capability_discovery_enabled = 1;
      },
      [](ims_settings_get_presence_new_config_rsp_msg_v01 &resp) -> void {
        //resp.capability_discovery_enabled_valid = TRUE;
        //resp.capability_discovery_enabled = 1;
      }),
  };


  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_set_presence_new_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_presence_new_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.expect(IMS_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_presence_new_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_presence_new_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    //*ims_ptr = testReq.imsReq;
    IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
    t.expect(IMS_node);
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_presence_new_config_003(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items presence_new_config: GET FAILURE.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_PUBLISH_TIMER;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 100;
  t.inject(IMS_node);

  ims_settings_set_presence_new_config_req_msg_v01 *set_req =
    QMI_node.update_default_qmi_ims_settings_set_presence_new_config_req();
  set_req->publish_timer_valid = TRUE;
  set_req->publish_timer = 100;
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_presence_new_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_PUBLISH_TIMER;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 100;
  t.inject(IMS_node);

  set_req = QMI_node.update_default_qmi_ims_settings_set_presence_new_config_req();
  set_req->publish_timer_valid = TRUE;
  set_req->publish_timer = 100;
  t.expect(QMI_node);

  ims_settings_set_presence_new_config_rsp_msg_v01 *set_resp =
    QMI_node.update_default_qmi_ims_settings_set_presence_new_config_resp(
        QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_PUBLISH_TIMER;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 100;
  t.inject(IMS_node);

  set_req = QMI_node.update_default_qmi_ims_settings_set_presence_new_config_req();
  set_req->publish_timer_valid = TRUE;
  set_req->publish_timer = 100;
  t.expect(QMI_node);

  set_resp = QMI_node.update_default_qmi_ims_settings_set_presence_new_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  set_resp->settings_resp_valid = TRUE;
  set_resp->settings_resp = IMS_SETTINGS_MSG_WRITE_FAILED_V01;
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_PUBLISH_TIMER;
  ims_ptr->has_errorCause = TRUE;
  ims_ptr->errorCause = ims_ConfigFailureCause_CONFIG_WRITE_FAILED;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);


  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_PUBLISH_TIMER;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_presence_new_config_req();
  t.expect(QMI_node);

  ims_settings_get_presence_new_config_rsp_msg_v01 *get_resp =
    QMI_node.update_default_qmi_ims_settings_get_presence_new_config_resp(
        QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_PUBLISH_TIMER;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_presence_new_config_req();
  t.expect(QMI_node);

  get_resp = QMI_node.update_default_qmi_ims_settings_get_presence_new_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_PUBLISH_TIMER;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_presence_new_config_req();
  t.expect(QMI_node);

  get_resp = QMI_node.update_default_qmi_ims_settings_get_presence_new_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  get_resp->settings_resp_valid = TRUE;
  get_resp->settings_resp = IMS_SETTINGS_MSG_READ_FAILED_V01;
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_PUBLISH_TIMER;
  ims_ptr->has_errorCause = TRUE;
  ims_ptr->errorCause = ims_ConfigFailureCause_CONFIG_READ_FAILED;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_handover_config_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items handover_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_handover_config_req_msg_v01 qmiReq;
    ims_settings_get_handover_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_handover_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_handover_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };


  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_THRESHOLD_LTE1;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_handover_config_req_msg_v01 &req) -> void {
        req.iIMSHOLTEQualTH1_valid = TRUE;
        req.iIMSHOLTEQualTH1 = 105;
      },
      [](ims_settings_get_handover_config_rsp_msg_v01 &resp) -> void {
        resp.iIMSHOLTEQualTH1_valid = TRUE;
        resp.iIMSHOLTEQualTH1 = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_THRESHOLD_LTE2;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_handover_config_req_msg_v01 &req) -> void {
        req.iIMSHOLTEQualTH2_valid = TRUE;
        req.iIMSHOLTEQualTH2 = 105;
      },
      [](ims_settings_get_handover_config_rsp_msg_v01 &resp) -> void {
        resp.iIMSHOLTEQualTH2_valid = TRUE;
        resp.iIMSHOLTEQualTH2 = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_THRESHOLD_LTE3;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_handover_config_req_msg_v01 &req) -> void {
        req.iIMSHOLTEQualTH3_valid = TRUE;
        req.iIMSHOLTEQualTH3 = 105;
      },
      [](ims_settings_get_handover_config_rsp_msg_v01 &resp) -> void {
        resp.iIMSHOLTEQualTH3_valid = TRUE;
        resp.iIMSHOLTEQualTH3 = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_THRESHOLD_1x;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_handover_config_req_msg_v01 &req) -> void {
        req.iIMSHO1xQualTH_valid = TRUE;
        req.iIMSHO1xQualTH = 105;
      },
      [](ims_settings_get_handover_config_rsp_msg_v01 &resp) -> void {
        resp.iIMSHO1xQualTH_valid = TRUE;
        resp.iIMSHO1xQualTH = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_THRESHOLD_WIFI_A;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_handover_config_req_msg_v01 &req) -> void {
        req.iIMSHOWIFIQualTHA_valid = TRUE;
        req.iIMSHOWIFIQualTHA = 105;
      },
      [](ims_settings_get_handover_config_rsp_msg_v01 &resp) -> void {
        resp.iIMSHOWIFIQualTHA_valid = TRUE;
        resp.iIMSHOWIFIQualTHA = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_THRESHOLD_WIFI_B;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_handover_config_req_msg_v01 &req) -> void {
        req.iIMSHOWIFIQualTHB_valid = TRUE;
        req.iIMSHOWIFIQualTHB = 105;
      },
      [](ims_settings_get_handover_config_rsp_msg_v01 &resp) -> void {
        resp.iIMSHOWIFIQualTHB_valid = TRUE;
        resp.iIMSHOWIFIQualTHB = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_T_EPDG_LTE;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_handover_config_req_msg_v01 &req) -> void {
        req.wlan_to_wwan_hys_timer_valid = TRUE;
        req.wlan_to_wwan_hys_timer = 105;
      },
      [](ims_settings_get_handover_config_rsp_msg_v01 &resp) -> void {
        resp.wlan_to_wwan_hys_timer_valid = TRUE;
        resp.wlan_to_wwan_hys_timer = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_T_EPDG_1x;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_handover_config_req_msg_v01 &req) -> void {
        req.wlan_to_1x_hys_timer_valid = TRUE;
        req.wlan_to_1x_hys_timer = 105;
      },
      [](ims_settings_get_handover_config_rsp_msg_v01 &resp) -> void {
        resp.wlan_to_1x_hys_timer_valid = TRUE;
        resp.wlan_to_1x_hys_timer = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_T_EPDG_WIFI;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_handover_config_req_msg_v01 &req) -> void {
        req.wwan_to_wlan_hys_timer_valid = TRUE;
        req.wwan_to_wlan_hys_timer = 105;
      },
      [](ims_settings_get_handover_config_rsp_msg_v01 &resp) -> void {
        resp.wwan_to_wlan_hys_timer_valid = TRUE;
        resp.wwan_to_wlan_hys_timer = 105;
      }),
  };


  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_set_handover_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_handover_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.expect(IMS_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_handover_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_handover_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    *ims_ptr = testReq.imsReq;
    t.expect(IMS_node);
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_handover_config_002(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items handover_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_handover_config_req_msg_v01 qmiReq;
    ims_settings_get_handover_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_handover_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_handover_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };


  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_THRESHOLD_LTE1;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_handover_config_req_msg_v01 &req) -> void {
        req.iIMSHOLTEQualTH1_valid = TRUE;
        req.iIMSHOLTEQualTH1 = 105;
      },
      [](ims_settings_get_handover_config_rsp_msg_v01 &resp) -> void {
        //resp.iIMSHOLTEQualTH1_valid = TRUE;
        //resp.iIMSHOLTEQualTH1 = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_THRESHOLD_LTE2;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_handover_config_req_msg_v01 &req) -> void {
        req.iIMSHOLTEQualTH2_valid = TRUE;
        req.iIMSHOLTEQualTH2 = 105;
      },
      [](ims_settings_get_handover_config_rsp_msg_v01 &resp) -> void {
        //resp.iIMSHOLTEQualTH2_valid = TRUE;
        //resp.iIMSHOLTEQualTH2 = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_THRESHOLD_LTE3;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_handover_config_req_msg_v01 &req) -> void {
        req.iIMSHOLTEQualTH3_valid = TRUE;
        req.iIMSHOLTEQualTH3 = 105;
      },
      [](ims_settings_get_handover_config_rsp_msg_v01 &resp) -> void {
        //resp.iIMSHOLTEQualTH3_valid = TRUE;
        //resp.iIMSHOLTEQualTH3 = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_THRESHOLD_1x;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_handover_config_req_msg_v01 &req) -> void {
        req.iIMSHO1xQualTH_valid = TRUE;
        req.iIMSHO1xQualTH = 105;
      },
      [](ims_settings_get_handover_config_rsp_msg_v01 &resp) -> void {
        //resp.iIMSHO1xQualTH_valid = TRUE;
        //resp.iIMSHO1xQualTH = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_THRESHOLD_WIFI_A;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_handover_config_req_msg_v01 &req) -> void {
        req.iIMSHOWIFIQualTHA_valid = TRUE;
        req.iIMSHOWIFIQualTHA = 105;
      },
      [](ims_settings_get_handover_config_rsp_msg_v01 &resp) -> void {
        //resp.iIMSHOWIFIQualTHA_valid = TRUE;
        //resp.iIMSHOWIFIQualTHA = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_THRESHOLD_WIFI_B;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_handover_config_req_msg_v01 &req) -> void {
        req.iIMSHOWIFIQualTHB_valid = TRUE;
        req.iIMSHOWIFIQualTHB = 105;
      },
      [](ims_settings_get_handover_config_rsp_msg_v01 &resp) -> void {
        //resp.iIMSHOWIFIQualTHB_valid = TRUE;
        //resp.iIMSHOWIFIQualTHB = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_T_EPDG_LTE;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_handover_config_req_msg_v01 &req) -> void {
        req.wlan_to_wwan_hys_timer_valid = TRUE;
        req.wlan_to_wwan_hys_timer = 105;
      },
      [](ims_settings_get_handover_config_rsp_msg_v01 &resp) -> void {
        //resp.wlan_to_wwan_hys_timer_valid = TRUE;
        //resp.wlan_to_wwan_hys_timer = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_T_EPDG_1x;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_handover_config_req_msg_v01 &req) -> void {
        req.wlan_to_1x_hys_timer_valid = TRUE;
        req.wlan_to_1x_hys_timer = 105;
      },
      [](ims_settings_get_handover_config_rsp_msg_v01 &resp) -> void {
        //resp.wlan_to_1x_hys_timer_valid = TRUE;
        //resp.wlan_to_1x_hys_timer = 105;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_T_EPDG_WIFI;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_handover_config_req_msg_v01 &req) -> void {
        req.wwan_to_wlan_hys_timer_valid = TRUE;
        req.wwan_to_wlan_hys_timer = 105;
      },
      [](ims_settings_get_handover_config_rsp_msg_v01 &resp) -> void {
        //resp.wwan_to_wlan_hys_timer_valid = TRUE;
        //resp.wwan_to_wlan_hys_timer = 105;
      }),
  };


  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_set_handover_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_handover_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.expect(IMS_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_handover_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_handover_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    //*ims_ptr = testReq.imsReq;
    IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
    t.expect(IMS_node);
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_handover_config_003(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items handover_config: GET FAILURE.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_THRESHOLD_LTE1;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 100;
  t.inject(IMS_node);

  ims_settings_set_handover_config_req_msg_v01 *set_req =
    QMI_node.update_default_qmi_ims_settings_set_handover_config_req();
  set_req->iIMSHOLTEQualTH1_valid = TRUE;
  set_req->iIMSHOLTEQualTH1 = 100;
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_handover_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_THRESHOLD_LTE1;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 100;
  t.inject(IMS_node);

  set_req = QMI_node.update_default_qmi_ims_settings_set_handover_config_req();
  set_req->iIMSHOLTEQualTH1_valid = TRUE;
  set_req->iIMSHOLTEQualTH1 = 100;
  t.expect(QMI_node);

  ims_settings_set_handover_config_rsp_msg_v01 *set_resp =
    QMI_node.update_default_qmi_ims_settings_set_handover_config_resp(
        QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_THRESHOLD_LTE1;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 100;
  t.inject(IMS_node);

  set_req = QMI_node.update_default_qmi_ims_settings_set_handover_config_req();
  set_req->iIMSHOLTEQualTH1_valid = TRUE;
  set_req->iIMSHOLTEQualTH1 = 100;
  t.expect(QMI_node);

  set_resp = QMI_node.update_default_qmi_ims_settings_set_handover_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  set_resp->settings_resp_valid = TRUE;
  set_resp->settings_resp = IMS_SETTINGS_MSG_WRITE_FAILED_V01;
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_THRESHOLD_LTE1;
  ims_ptr->has_errorCause = TRUE;
  ims_ptr->errorCause = ims_ConfigFailureCause_CONFIG_WRITE_FAILED;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);


  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_THRESHOLD_LTE1;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_handover_config_req();
  t.expect(QMI_node);

  ims_settings_get_handover_config_rsp_msg_v01 *get_resp =
    QMI_node.update_default_qmi_ims_settings_get_handover_config_resp(
        QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_THRESHOLD_LTE1;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_handover_config_req();
  t.expect(QMI_node);

  get_resp = QMI_node.update_default_qmi_ims_settings_get_handover_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_THRESHOLD_LTE1;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_handover_config_req();
  t.expect(QMI_node);

  get_resp = QMI_node.update_default_qmi_ims_settings_get_handover_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  get_resp->settings_resp_valid = TRUE;
  get_resp->settings_resp = IMS_SETTINGS_MSG_READ_FAILED_V01;
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_THRESHOLD_LTE1;
  ims_ptr->has_errorCause = TRUE;
  ims_ptr->errorCause = ims_ConfigFailureCause_CONFIG_READ_FAILED;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_ims_new_config_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items ims_new_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_ims_new_config_req_msg_v01 qmiReq;
    ims_settings_get_ims_new_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_ims_new_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_ims_new_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };

  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_DOMAIN_NAME;
        req.stringValue.arg = strdup("test ims_user_agent");
      },
      [](ims_settings_set_ims_new_config_req_msg_v01 &req) -> void {
        //req.ims_user_agent_valid = TRUE;
        strlcpy((char *)req.ims_user_agent,
                "test ims_user_agent", strlen("test ims_user_agent")+1);
      },
      [](ims_settings_get_ims_new_config_rsp_msg_v01 &resp) -> void {
        resp.ims_user_agent_valid = TRUE;
        strlcpy((char *)resp.ims_user_agent,
                "test ims_user_agent", strlen("test ims_user_agent")+1);
      }),
  };

  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_set_ims_new_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_ims_new_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.expect(IMS_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_ims_new_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_ims_new_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    *ims_ptr = testReq.imsReq;
    t.expect(IMS_node);
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_ims_new_config_002(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items ims_new_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_ims_new_config_req_msg_v01 qmiReq;
    ims_settings_get_ims_new_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_ims_new_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_ims_new_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };

  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_DOMAIN_NAME;
        req.stringValue.arg = strdup("test ims_user_agent");
      },
      [](ims_settings_set_ims_new_config_req_msg_v01 &req) -> void {
        //req.ims_user_agent_valid = TRUE;
        strlcpy((char *)req.ims_user_agent,
                "test ims_user_agent", strlen("test ims_user_agent")+1);
      },
      [](ims_settings_get_ims_new_config_rsp_msg_v01 &resp) -> void {
        //resp.ims_user_agent_valid = TRUE;
        //strlcpy((char *)resp.ims_user_agent,
        //        "test ims_user_agent", strlen("test ims_user_agent")+1);
      }),
  };

  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_set_ims_new_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_ims_new_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.expect(IMS_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_ims_new_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_ims_new_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    //*ims_ptr = testReq.imsReq;
    IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
    t.expect(IMS_node);
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_ims_new_config_003(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items ims_new_config: GET FAILURE.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_DOMAIN_NAME;
  ims_ptr->stringValue.arg = strdup("test ims_user_agent");
  t.inject(IMS_node);

  ims_settings_set_ims_new_config_req_msg_v01 *set_req =
    QMI_node.update_default_qmi_ims_settings_set_ims_new_config_req();
  strlcpy((char *)set_req->ims_user_agent,
      "test ims_user_agent", strlen("test ims_user_agent")+1);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_ims_new_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_DOMAIN_NAME;
  ims_ptr->stringValue.arg = strdup("test ims_user_agent");
  t.inject(IMS_node);

  set_req = QMI_node.update_default_qmi_ims_settings_set_ims_new_config_req();
  strlcpy((char *)set_req->ims_user_agent,
      "test ims_user_agent", strlen("test ims_user_agent")+1);
  t.expect(QMI_node);

  ims_settings_set_ims_new_config_rsp_msg_v01 *set_resp =
    QMI_node.update_default_qmi_ims_settings_set_ims_new_config_resp(
        QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_DOMAIN_NAME;
  ims_ptr->stringValue.arg = strdup("test ims_user_agent");
  t.inject(IMS_node);

  set_req = QMI_node.update_default_qmi_ims_settings_set_ims_new_config_req();
  strlcpy((char *)set_req->ims_user_agent,
      "test ims_user_agent", strlen("test ims_user_agent")+1);
  t.expect(QMI_node);

  set_resp = QMI_node.update_default_qmi_ims_settings_set_ims_new_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  set_resp->settings_resp_valid = TRUE;
  set_resp->settings_resp = IMS_SETTINGS_MSG_WRITE_FAILED_V01;
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_DOMAIN_NAME;
  ims_ptr->stringValue.arg = strdup("test ims_user_agent");
  ims_ptr->has_errorCause = TRUE;
  ims_ptr->errorCause = ims_ConfigFailureCause_CONFIG_WRITE_FAILED;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);


  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_DOMAIN_NAME;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_ims_new_config_req();
  t.expect(QMI_node);

  ims_settings_get_ims_new_config_rsp_msg_v01 *get_resp =
    QMI_node.update_default_qmi_ims_settings_get_ims_new_config_resp(
        QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_DOMAIN_NAME;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_ims_new_config_req();
  t.expect(QMI_node);

  get_resp = QMI_node.update_default_qmi_ims_settings_get_ims_new_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_DOMAIN_NAME;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_ims_new_config_req();
  t.expect(QMI_node);

  get_resp = QMI_node.update_default_qmi_ims_settings_get_ims_new_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  get_resp->settings_resp_valid = TRUE;
  get_resp->settings_resp = IMS_SETTINGS_MSG_READ_FAILED_V01;
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_DOMAIN_NAME;
  ims_ptr->has_errorCause = TRUE;
  ims_ptr->errorCause = ims_ConfigFailureCause_CONFIG_READ_FAILED;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_emer_dynamic_config_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items emer_dynamic_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_emer_dynamic_config_req_msg_v01 qmiReq;
    ims_settings_get_emer_dynamic_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_emer_dynamic_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_emer_dynamic_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };

  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_EMERGENCY_CALL_TIMER;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_emer_dynamic_config_req_msg_v01 &req) -> void {
        req.e911_call_setup_timer_valid = TRUE;
        req.e911_call_setup_timer = 105;
      },
      [](ims_settings_get_emer_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.e911_call_setup_timer_valid = TRUE;
        resp.e911_call_setup_timer = 105;
      }),
  };

  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_set_emer_dynamic_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_emer_dynamic_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.expect(IMS_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_emer_dynamic_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_emer_dynamic_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    *ims_ptr = testReq.imsReq;
    t.expect(IMS_node);
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_emer_dynamic_config_002(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items emer_dynamic_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_emer_dynamic_config_req_msg_v01 qmiReq;
    ims_settings_get_emer_dynamic_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_emer_dynamic_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_emer_dynamic_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };

  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_EMERGENCY_CALL_TIMER;
        req.has_intValue = TRUE;
        req.intValue = 105;
      },
      [](ims_settings_set_emer_dynamic_config_req_msg_v01 &req) -> void {
        req.e911_call_setup_timer_valid = TRUE;
        req.e911_call_setup_timer = 105;
      },
      [](ims_settings_get_emer_dynamic_config_rsp_msg_v01 &resp) -> void {
        //resp.e911_call_setup_timer_valid = TRUE;
        //resp.e911_call_setup_timer = 105;
      }),
  };

  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_set_emer_dynamic_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_emer_dynamic_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.expect(IMS_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_emer_dynamic_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_emer_dynamic_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    //*ims_ptr = testReq.imsReq;
    IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
    t.expect(IMS_node);
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_emer_dynamic_config_003(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items emer_dynamic_config: GET FAILURE.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_EMERGENCY_CALL_TIMER;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 100;
  t.inject(IMS_node);

  ims_settings_set_emer_dynamic_config_req_msg_v01 *set_req =
    QMI_node.update_default_qmi_ims_settings_set_emer_dynamic_config_req();
  set_req->e911_call_setup_timer_valid = TRUE;
  set_req->e911_call_setup_timer = 100;
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_emer_dynamic_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_EMERGENCY_CALL_TIMER;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 100;
  t.inject(IMS_node);

  set_req = QMI_node.update_default_qmi_ims_settings_set_emer_dynamic_config_req();
  set_req->e911_call_setup_timer_valid = TRUE;
  set_req->e911_call_setup_timer = 100;
  t.expect(QMI_node);

  ims_settings_set_emer_dynamic_config_rsp_msg_v01 *set_resp =
    QMI_node.update_default_qmi_ims_settings_set_emer_dynamic_config_resp(
        QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_EMERGENCY_CALL_TIMER;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 100;
  t.inject(IMS_node);

  set_req = QMI_node.update_default_qmi_ims_settings_set_emer_dynamic_config_req();
  set_req->e911_call_setup_timer_valid = TRUE;
  set_req->e911_call_setup_timer = 100;
  t.expect(QMI_node);

  set_resp = QMI_node.update_default_qmi_ims_settings_set_emer_dynamic_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  set_resp->settings_resp_valid = TRUE;
  set_resp->settings_resp = IMS_SETTINGS_MSG_WRITE_FAILED_V01;
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_EMERGENCY_CALL_TIMER;
  ims_ptr->has_errorCause = TRUE;
  ims_ptr->errorCause = ims_ConfigFailureCause_CONFIG_WRITE_FAILED;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);


  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_EMERGENCY_CALL_TIMER;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_emer_dynamic_config_req();
  t.expect(QMI_node);

  ims_settings_get_emer_dynamic_config_rsp_msg_v01 *get_resp =
    QMI_node.update_default_qmi_ims_settings_get_emer_dynamic_config_resp(
        QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_EMERGENCY_CALL_TIMER;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_emer_dynamic_config_req();
  t.expect(QMI_node);

  get_resp = QMI_node.update_default_qmi_ims_settings_get_emer_dynamic_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_EMERGENCY_CALL_TIMER;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_emer_dynamic_config_req();
  t.expect(QMI_node);

  get_resp = QMI_node.update_default_qmi_ims_settings_get_emer_dynamic_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  get_resp->settings_resp_valid = TRUE;
  get_resp->settings_resp = IMS_SETTINGS_MSG_READ_FAILED_V01;
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_EMERGENCY_CALL_TIMER;
  ims_ptr->has_errorCause = TRUE;
  ims_ptr->errorCause = ims_ConfigFailureCause_CONFIG_READ_FAILED;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_video_dynamic_config_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items video_dynamic_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_video_dynamic_config_req_msg_v01 qmiReq;
    ims_settings_get_video_dynamic_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_video_dynamic_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_video_dynamic_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };

  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VIDEO_QUALITY;
        req.has_intValue = TRUE;
        req.intValue = 0;
      },
      [](ims_settings_set_video_dynamic_config_req_msg_v01 &req) -> void {
        req.vt_quality_selector_valid = TRUE;
        req.vt_quality_selector = IMS_SETTINGS_VT_QUALITY_LEVEL_2_V01;
      },
      [](ims_settings_get_video_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.vt_quality_selector_valid = TRUE;
        resp.vt_quality_selector = IMS_SETTINGS_VT_QUALITY_LEVEL_2_V01;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VIDEO_QUALITY;
        req.has_intValue = TRUE;
        req.intValue = 1;
      },
      [](ims_settings_set_video_dynamic_config_req_msg_v01 &req) -> void {
        req.vt_quality_selector_valid = TRUE;
        req.vt_quality_selector = IMS_SETTINGS_VT_QUALITY_LEVEL_1_V01;
      },
      [](ims_settings_get_video_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.vt_quality_selector_valid = TRUE;
        resp.vt_quality_selector = IMS_SETTINGS_VT_QUALITY_LEVEL_1_V01;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VIDEO_QUALITY;
        req.has_intValue = TRUE;
        req.intValue = 2;
      },
      [](ims_settings_set_video_dynamic_config_req_msg_v01 &req) -> void {
        req.vt_quality_selector_valid = TRUE;
        req.vt_quality_selector = IMS_SETTINGS_VT_QUALITY_LEVEL_0_V01;
      },
      [](ims_settings_get_video_dynamic_config_rsp_msg_v01 &resp) -> void {
        resp.vt_quality_selector_valid = TRUE;
        resp.vt_quality_selector = IMS_SETTINGS_VT_QUALITY_LEVEL_0_V01;
      }),
  };

  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_set_video_dynamic_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_video_dynamic_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.expect(IMS_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_video_dynamic_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_video_dynamic_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    *ims_ptr = testReq.imsReq;
    t.expect(IMS_node);
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_video_dynamic_config_002(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items video_dynamic_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_video_dynamic_config_req_msg_v01 qmiReq;
    ims_settings_get_video_dynamic_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_video_dynamic_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_video_dynamic_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };

  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VIDEO_QUALITY;
        req.has_intValue = TRUE;
        req.intValue = 0;
      },
      [](ims_settings_set_video_dynamic_config_req_msg_v01 &req) -> void {
        req.vt_quality_selector_valid = TRUE;
        req.vt_quality_selector = IMS_SETTINGS_VT_QUALITY_LEVEL_2_V01;
      },
      [](ims_settings_get_video_dynamic_config_rsp_msg_v01 &resp) -> void {
        //resp.vt_quality_selector_valid = TRUE;
        //resp.vt_quality_selector = IMS_SETTINGS_VT_QUALITY_LEVEL_2_V01;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VIDEO_QUALITY;
        req.has_intValue = TRUE;
        req.intValue = 1;
      },
      [](ims_settings_set_video_dynamic_config_req_msg_v01 &req) -> void {
        req.vt_quality_selector_valid = TRUE;
        req.vt_quality_selector = IMS_SETTINGS_VT_QUALITY_LEVEL_1_V01;
      },
      [](ims_settings_get_video_dynamic_config_rsp_msg_v01 &resp) -> void {
        //resp.vt_quality_selector_valid = TRUE;
        //resp.vt_quality_selector = IMS_SETTINGS_VT_QUALITY_LEVEL_1_V01;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VIDEO_QUALITY;
        req.has_intValue = TRUE;
        req.intValue = 2;
      },
      [](ims_settings_set_video_dynamic_config_req_msg_v01 &req) -> void {
        req.vt_quality_selector_valid = TRUE;
        req.vt_quality_selector = IMS_SETTINGS_VT_QUALITY_LEVEL_0_V01;
      },
      [](ims_settings_get_video_dynamic_config_rsp_msg_v01 &resp) -> void {
        //resp.vt_quality_selector_valid = TRUE;
        //resp.vt_quality_selector = IMS_SETTINGS_VT_QUALITY_LEVEL_0_V01;
      }),
  };

  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_set_video_dynamic_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_video_dynamic_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.expect(IMS_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_video_dynamic_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_video_dynamic_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    //*ims_ptr = testReq.imsReq;
    IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
    t.expect(IMS_node);
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_video_dynamic_config_003(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items video_dynamic_config: GET FAILURE.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VIDEO_QUALITY;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 0;
  t.inject(IMS_node);

  ims_settings_set_video_dynamic_config_req_msg_v01 *set_req =
    QMI_node.update_default_qmi_ims_settings_set_video_dynamic_config_req();
  set_req->vt_quality_selector_valid = TRUE;
  set_req->vt_quality_selector = IMS_SETTINGS_VT_QUALITY_LEVEL_2_V01;
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_video_dynamic_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VIDEO_QUALITY;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 0;
  t.inject(IMS_node);

  set_req = QMI_node.update_default_qmi_ims_settings_set_video_dynamic_config_req();
  set_req->vt_quality_selector_valid = TRUE;
  set_req->vt_quality_selector = IMS_SETTINGS_VT_QUALITY_LEVEL_2_V01;
  t.expect(QMI_node);

  ims_settings_set_video_dynamic_config_rsp_msg_v01 *set_resp =
    QMI_node.update_default_qmi_ims_settings_set_video_dynamic_config_resp(
        QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VIDEO_QUALITY;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 0;
  t.inject(IMS_node);

  set_req = QMI_node.update_default_qmi_ims_settings_set_video_dynamic_config_req();
  set_req->vt_quality_selector_valid = TRUE;
  set_req->vt_quality_selector = IMS_SETTINGS_VT_QUALITY_LEVEL_2_V01;
  t.expect(QMI_node);

  set_resp = QMI_node.update_default_qmi_ims_settings_set_video_dynamic_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  set_resp->settings_resp_valid = TRUE;
  set_resp->settings_resp = IMS_SETTINGS_MSG_WRITE_FAILED_V01;
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VIDEO_QUALITY;
  ims_ptr->has_errorCause = TRUE;
  ims_ptr->errorCause = ims_ConfigFailureCause_CONFIG_WRITE_FAILED;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);


  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VIDEO_QUALITY;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_video_dynamic_config_req();
  t.expect(QMI_node);

  ims_settings_get_video_dynamic_config_rsp_msg_v01 *get_resp =
    QMI_node.update_default_qmi_ims_settings_get_video_dynamic_config_resp(
        QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VIDEO_QUALITY;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_video_dynamic_config_req();
  t.expect(QMI_node);

  get_resp = QMI_node.update_default_qmi_ims_settings_get_video_dynamic_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VIDEO_QUALITY;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_video_dynamic_config_req();
  t.expect(QMI_node);

  get_resp = QMI_node.update_default_qmi_ims_settings_get_video_dynamic_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  get_resp->settings_resp_valid = TRUE;
  get_resp->settings_resp = IMS_SETTINGS_MSG_READ_FAILED_V01;
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VIDEO_QUALITY;
  ims_ptr->has_errorCause = TRUE;
  ims_ptr->errorCause = ims_ConfigFailureCause_CONFIG_READ_FAILED;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_rcs_new_config_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items rcs_new_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_rcs_new_config_req_msg_v01 qmiReq;
    ims_settings_get_rcs_new_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_rcs_new_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_rcs_new_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };

  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SMS_APP;
        req.has_intValue = TRUE;
        req.intValue = 0;
      },
      [](ims_settings_set_rcs_new_config_req_msg_v01 &req) -> void {
        req.default_sms_app_valid = TRUE;
        req.default_sms_app = 0;
      },
      [](ims_settings_get_rcs_new_config_rsp_msg_v01 &resp) -> void {
        resp.default_sms_app_valid = TRUE;
        resp.default_sms_app = 0;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VVM_APP;
        req.has_intValue = TRUE;
        req.intValue = 1;
      },
      [](ims_settings_set_rcs_new_config_req_msg_v01 &req) -> void {
        req.default_vvm_app_valid = TRUE;
        req.default_vvm_app = 1;
      },
      [](ims_settings_get_rcs_new_config_rsp_msg_v01 &resp) -> void {
        resp.default_vvm_app_valid = TRUE;
        resp.default_vvm_app = 1;
      }),
  };

  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_set_rcs_new_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_rcs_new_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.expect(IMS_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_rcs_new_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_rcs_new_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    *ims_ptr = testReq.imsReq;
    t.expect(IMS_node);
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_rcs_new_config_002(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items rcs_new_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_rcs_new_config_req_msg_v01 qmiReq;
    ims_settings_get_rcs_new_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_rcs_new_config_req_msg_v01 &)> getQmiReq,
        std::function<void(ims_settings_get_rcs_new_config_rsp_msg_v01 &)> getQmiResp) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      getQmiResp(qmiResp);
      qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };

  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SMS_APP;
        req.has_intValue = TRUE;
        req.intValue = 0;
      },
      [](ims_settings_set_rcs_new_config_req_msg_v01 &req) -> void {
        req.default_sms_app_valid = TRUE;
        req.default_sms_app = 0;
      },
      [](ims_settings_get_rcs_new_config_rsp_msg_v01 &resp) -> void {
        //resp.default_sms_app_valid = TRUE;
        //resp.default_sms_app = 0;
      }),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_VVM_APP;
        req.has_intValue = TRUE;
        req.intValue = 1;
      },
      [](ims_settings_set_rcs_new_config_req_msg_v01 &req) -> void {
        req.default_vvm_app_valid = TRUE;
        req.default_vvm_app = 1;
      },
      [](ims_settings_get_rcs_new_config_rsp_msg_v01 &resp) -> void {
        //resp.default_vvm_app_valid = TRUE;
        //resp.default_vvm_app = 1;
      }),
  };

  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_set_rcs_new_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_rcs_new_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.expect(IMS_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_rcs_new_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_rcs_new_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    //*ims_ptr = testReq.imsReq;
    IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
    t.expect(IMS_node);
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_rcs_new_config_003(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items rcs_new_config: GET FAILURE.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SMS_APP;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 0;
  t.inject(IMS_node);

  ims_settings_set_rcs_new_config_req_msg_v01 *set_req =
    QMI_node.update_default_qmi_ims_settings_set_rcs_new_config_req();
  set_req->default_sms_app_valid = TRUE;
  set_req->default_sms_app = 0;
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_rcs_new_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SMS_APP;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 0;
  t.inject(IMS_node);

  set_req = QMI_node.update_default_qmi_ims_settings_set_rcs_new_config_req();
  set_req->default_sms_app_valid = TRUE;
  set_req->default_sms_app = 0;
  t.expect(QMI_node);

  ims_settings_set_rcs_new_config_rsp_msg_v01 *set_resp =
    QMI_node.update_default_qmi_ims_settings_set_rcs_new_config_resp(
        QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SMS_APP;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 0;
  t.inject(IMS_node);

  set_req = QMI_node.update_default_qmi_ims_settings_set_rcs_new_config_req();
  set_req->default_sms_app_valid = TRUE;
  set_req->default_sms_app = 0;
  t.expect(QMI_node);

  set_resp = QMI_node.update_default_qmi_ims_settings_set_rcs_new_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  set_resp->settings_resp_valid = TRUE;
  set_resp->settings_resp = IMS_SETTINGS_MSG_WRITE_FAILED_V01;
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SMS_APP;
  ims_ptr->has_errorCause = TRUE;
  ims_ptr->errorCause = ims_ConfigFailureCause_CONFIG_WRITE_FAILED;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);


  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SMS_APP;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_rcs_new_config_req();
  t.expect(QMI_node);

  ims_settings_get_rcs_new_config_rsp_msg_v01 *get_resp =
    QMI_node.update_default_qmi_ims_settings_get_rcs_new_config_resp(
        QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SMS_APP;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_rcs_new_config_req();
  t.expect(QMI_node);

  get_resp = QMI_node.update_default_qmi_ims_settings_get_rcs_new_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SMS_APP;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_rcs_new_config_req();
  t.expect(QMI_node);

  get_resp = QMI_node.update_default_qmi_ims_settings_get_rcs_new_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  get_resp->settings_resp_valid = TRUE;
  get_resp->settings_resp = IMS_SETTINGS_MSG_READ_FAILED_V01;
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SMS_APP;
  ims_ptr->has_errorCause = TRUE;
  ims_ptr->errorCause = ims_ConfigFailureCause_CONFIG_READ_FAILED;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_concurrency_config_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items concurrency_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  struct ImsConfigRequest {
    ims_ConfigMsg imsReq;
    ims_settings_set_concurrency_config_req_msg_v01 qmiReq;
    //ims_settings_get_concurrency_config_rsp_msg_v01 qmiResp;
    ImsConfigRequest(
        std::function<void(ims_ConfigMsg &)> getImsReq,
        std::function<void(ims_settings_set_concurrency_config_req_msg_v01 &)> getQmiReq /*,
        std::function<void(ims_settings_get_concurrency_config_rsp_msg_v01 &)> getQmiResp*/) {
      memset(&imsReq, 0x00, sizeof(imsReq));
      memset(&qmiReq, 0x00, sizeof(qmiReq));
      //memset(&qmiResp, 0x00, sizeof(qmiResp));
      getImsReq(imsReq);
      getQmiReq(qmiReq);
      //getQmiResp(qmiResp);
      //qmiResp.resp.result = QMI_RESULT_SUCCESS_V01;
      //qmiResp.resp.error = QMI_ERR_NONE_V01;
    }
  };

  std::vector<ImsConfigRequest> testReqVector {
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SET_AUTO_REJECT_CALL_MODE_CONFIG;
        req.has_intValue = TRUE;
        req.intValue = 0;
      },
      [](ims_settings_set_concurrency_config_req_msg_v01 &req) -> void {
        req.auto_reject_call_enabled_valid = TRUE;
        req.auto_reject_call_enabled = 0;
      }/*,
      [](ims_settings_get_concurrency_config_rsp_msg_v01 &resp) -> void {
        resp.auto_reject_call_enabled_valid = TRUE;
        resp.auto_reject_call_enabled = 0;
      }*/),
     ImsConfigRequest(
      [](ims_ConfigMsg &req) -> void {
        req.has_item = TRUE;
        req.item = ims_ConfigItem_CONFIG_ITEM_SET_AUTO_REJECT_CALL_MODE_CONFIG;
        req.has_intValue = TRUE;
        req.intValue = 1;
      },
      [](ims_settings_set_concurrency_config_req_msg_v01 &req) -> void {
        req.auto_reject_call_enabled_valid = TRUE;
        req.auto_reject_call_enabled = 1;
      }/*,
      [](ims_settings_get_concurrency_config_rsp_msg_v01 &resp) -> void {
        resp.auto_reject_call_enabled_valid = TRUE;
        resp.auto_reject_call_enabled = 1;
      }*/),
  };

  for (auto testReq : testReqVector) {
    IMS_node.update_default_ims_request_set_config(&testReq.imsReq);
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_set_concurrency_config_req(&testReq.qmiReq);
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_set_concurrency_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
    t.inject(QMI_node);

    ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.expect(IMS_node);

#if 0
    if (testReq.imsReq.item != ims_ConfigItem_CONFIG_ITEM_SET_AUTO_REJECT_CALL_MODE_CONFIG) {
    ims_ptr = IMS_node.update_default_ims_request_get_config();
    ims_ptr->has_item = TRUE;
    ims_ptr->item = testReq.imsReq.item;
    t.inject(IMS_node);

    QMI_node.update_default_qmi_ims_settings_get_concurrency_config_req();
    t.expect(QMI_node);

    QMI_node.update_default_qmi_ims_settings_get_concurrency_config_resp(
        QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, &testReq.qmiResp);
    t.inject(QMI_node);

    ims_ptr = IMS_node.update_default_ims_request_get_config_resp();
    *ims_ptr = testReq.imsReq;
    t.expect(IMS_node);
    }
#endif
  }

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_config_concurrency_config_003(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items concurrency_config: FAILURE.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  ims_ConfigMsg *ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SET_AUTO_REJECT_CALL_MODE_CONFIG;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 0;
  t.inject(IMS_node);

  ims_settings_set_concurrency_config_req_msg_v01 *set_req =
    QMI_node.update_default_qmi_ims_settings_set_concurrency_config_req();
  set_req->auto_reject_call_enabled_valid = TRUE;
  set_req->auto_reject_call_enabled = 0;
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_concurrency_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SET_AUTO_REJECT_CALL_MODE_CONFIG;
  ims_ptr->has_errorCause = TRUE;
  ims_ptr->errorCause = ims_ConfigFailureCause_CONFIG_NO_ERR;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SET_AUTO_REJECT_CALL_MODE_CONFIG;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 0;
  t.inject(IMS_node);

  set_req = QMI_node.update_default_qmi_ims_settings_set_concurrency_config_req();
  set_req->auto_reject_call_enabled_valid = TRUE;
  set_req->auto_reject_call_enabled = 0;
  t.expect(QMI_node);

  ims_settings_set_concurrency_config_rsp_msg_v01 *set_resp =
    QMI_node.update_default_qmi_ims_settings_set_concurrency_config_resp(
        QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SET_AUTO_REJECT_CALL_MODE_CONFIG;
  ims_ptr->has_errorCause = TRUE;
  ims_ptr->errorCause = ims_ConfigFailureCause_CONFIG_NO_ERR;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

#if 0
  ims_ptr = IMS_node.update_default_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SET_AUTO_REJECT_CALL_MODE_CONFIG;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 0;
  t.inject(IMS_node);

  set_req = QMI_node.update_default_qmi_ims_settings_set_concurrency_config_req();
  set_req->auto_reject_call_enabled_valid = TRUE;
  set_req->auto_reject_call_enabled = 0;
  t.expect(QMI_node);

  set_resp = QMI_node.update_default_qmi_ims_settings_set_concurrency_config_resp(
      QMI_RESULT_FAILURE_V01, QMI_ERR_CAUSE_CODE_V01);
  set_resp->settings_resp_valid = TRUE;
  set_resp->settings_resp = IMS_SETTINGS_MSG_WRITE_FAILED_V01;
  t.inject(QMI_node);

  ims_ptr = IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SET_AUTO_REJECT_CALL_MODE_CONFIG;
  ims_ptr->has_errorCause = TRUE;
  ims_ptr->errorCause = ims_ConfigFailureCause_CONFIG_WRITE_FAILED;
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);
#endif

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_set_and_get_voip_config_items_L(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id("ril_utf_ims_set_and_get_voip_config_items_L");
  t.set_test_description("Tests Ims Config items related to qmi_imss_voip_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }



  /* ims_ConfigItem_CONFIG_ITEM_VOCODER_AMRMODESET */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ConfigMsg* ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOCODER_AMRMODESET;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 0x8;
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_voip_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_voip_config_req_v01_amr_mode(0x8);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_voip_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOCODER_AMRMODESET;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOCODER_AMRMODESET;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_voip_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_voip_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_voip_config_rsp_v01_amr_mode(0x8);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOCODER_AMRMODESET;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 0x8;
  t.expect(IMS_node);


  /* ims_ConfigItem_CONFIG_ITEM_VOCODER_AMRWBMODESET */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOCODER_AMRWBMODESET;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 0x80;
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_voip_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_voip_config_req_v01_amr_wb_mode(0x80);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_voip_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOCODER_AMRWBMODESET;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOCODER_AMRWBMODESET;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_voip_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_voip_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_voip_config_rsp_v01_amr_wb_mode(0x80);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOCODER_AMRWBMODESET;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 0x80;
  t.expect(IMS_node);

  /* ims_ConfigItem_CONFIG_ITEM_SIP_SESSION_TIMER */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_SESSION_TIMER;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 5000; //5secs
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_voip_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_voip_config_req_v01_session_expiry_timer(5000);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_voip_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_SESSION_TIMER;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_SESSION_TIMER;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_voip_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_voip_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_voip_config_rsp_v01_session_expiry_timer(5000);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_SESSION_TIMER;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 5000;
  t.expect(IMS_node);

  /* ims_ConfigItem_CONFIG_ITEM_MIN_SESSION_EXPIRY */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_MIN_SESSION_EXPIRY;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 10000; //10 secs
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_voip_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_voip_config_req_v01_min_session_expiry(10000);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_voip_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_MIN_SESSION_EXPIRY;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_MIN_SESSION_EXPIRY;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_voip_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_voip_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_voip_config_rsp_v01_min_session_expiry(10000);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_MIN_SESSION_EXPIRY;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 10000;
  t.expect(IMS_node);

  /* ims_ConfigItem_CONFIG_ITEM_SILENT_REDIAL_ENABLE */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SILENT_REDIAL_ENABLE;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 1; //TRUE
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_voip_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_voip_config_req_v01_voip_silent_redial_enabled(TRUE);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_voip_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SILENT_REDIAL_ENABLE;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SILENT_REDIAL_ENABLE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_voip_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_voip_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_voip_config_rsp_v01_voip_silent_redial_enabled(TRUE);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SILENT_REDIAL_ENABLE;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 1;
  t.expect(IMS_node);

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_set_and_get_user_config_items_L(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id("ril_utf_ims_set_and_get_user_config_items_L");
  t.set_test_description("Tests Ims Config items related to qmi_imss_voip_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  /* ims_ConfigItem_CONFIG_ITEM_DOMAIN_NAME */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ConfigMsg* ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_DOMAIN_NAME;
  ims_ptr->has_intValue = FALSE;
  ims_ptr->has_boolValue = FALSE;
  //ims_ptr->stringValue = "ims.apn";
  ims_ptr->stringValue.arg = qcril_malloc(strlen("ims.apn")+1);
  strlcpy((char *)ims_ptr->stringValue.arg, "ims.apn", strlen("ims.apn")+1);
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_user_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_user_config_req_v01_domain_name("ims.apn");
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_user_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_DOMAIN_NAME;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_DOMAIN_NAME;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_user_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_user_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_user_config_rsp_v01_domain_name("ims.apn");
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_DOMAIN_NAME;
  ims_ptr->has_intValue = FALSE;
  ims_ptr->has_boolValue = FALSE;
  //ims_ptr->stringValue = "ims.apn";
  ims_ptr->stringValue.arg = qcril_malloc(strlen("ims.apn")+1);
  strlcpy((char *)ims_ptr->stringValue.arg, "ims.apn", strlen("ims.apn")+1);
  t.expect(IMS_node);

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_set_and_get_sip_config_items_L(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id("ril_utf_ims_set_and_get_sip_config_items_L");
  t.set_test_description("Tests Ims Config items related to qmi_imss_sip_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  /* ims_ConfigItem_CONFIG_ITEM_CANCELLATION_TIMER */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ConfigMsg* ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_CANCELLATION_TIMER;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 10000; //10 secs
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_sip_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_sip_config_req_v01_sip_timer_operator_mode_a(10000);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_sip_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_CANCELLATION_TIMER;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_CANCELLATION_TIMER;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_sip_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_sip_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_sip_config_rsp_v01_sip_timer_operator_mode_a(10000);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_CANCELLATION_TIMER;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 10000; //10 secs
  ims_ptr->has_boolValue = FALSE;
  t.expect(IMS_node);

  /* ims_ConfigItem_CONFIG_ITEM_SIP_T1_TIMER */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_T1_TIMER;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 800; //10 secs
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_sip_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_sip_config_req_v01_timer_t1(800);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_sip_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_T1_TIMER;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_T1_TIMER;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_sip_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_sip_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_sip_config_rsp_v01_timer_t1(800);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_T1_TIMER;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 800;
  ims_ptr->has_boolValue = FALSE;
  t.expect(IMS_node);

  /* ims_ConfigItem_CONFIG_ITEM_SIP_T2_TIMER */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_T2_TIMER;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 20000; //10 secs
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_sip_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_sip_config_req_v01_timer_t2(20000);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_sip_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_T2_TIMER;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_T2_TIMER;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_sip_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_sip_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_sip_config_rsp_v01_timer_t2(20000);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_T2_TIMER;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 20000;
  ims_ptr->has_boolValue = FALSE;
  t.expect(IMS_node);

  /* ims_ConfigItem_CONFIG_ITEM_SIP_TF_TIMER */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_TF_TIMER;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 50;
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_sip_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_sip_config_req_v01_timer_tf(50);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_sip_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_TF_TIMER;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_TF_TIMER;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_sip_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_sip_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_sip_config_rsp_v01_timer_tf(18000);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SIP_TF_TIMER;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 18000;
  ims_ptr->has_boolValue = FALSE;
  t.expect(IMS_node);

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_set_and_get_reg_mgr_extended_config_items_L(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id("ril_utf_ims_set_and_get_reg_mgr_extended_config_items_L");
  t.set_test_description("Tests Ims Config items related to qmi_imss_reg_mgr_extended_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  /* ims_ConfigItem_CONFIG_ITEM_T_DELAY */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ConfigMsg* ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_T_DELAY;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 1500;
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_reg_mgr_extended_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_reg_mgr_extended_config_req_v01_t_delay(1500);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_reg_mgr_extended_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_T_DELAY;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_T_DELAY;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_reg_mgr_extended_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_reg_mgr_extended_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_reg_mgr_extended_config_rsp_v01_t_delay(1500);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_T_DELAY;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 1500;
  ims_ptr->has_boolValue = FALSE;
  t.expect(IMS_node);

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_set_and_get_wifi_call_pref(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_description("Tests Ims Config items related to get/set wifi_call_preference.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  /* ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED */
  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ConfigMsg* ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 1; //IMS_SETTINGS_WFC_STATUS_ON_V01
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  /* QMI_IMS_SETTINGS_SET_IMS_SERVICE_ENABLE_CONFIG_REQ_V01 */
  QMI_node.update_default_qmi_ims_settings_set_ims_service_enable_config_req();
  QMI_node.set_qmi_ims_settings_set_ims_service_enable_config_req_wifi_calling_enabled(TRUE);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_ims_service_enable_config_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
  t.expect(IMS_node);


  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_ims_service_enable_config_req();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_ims_service_enable_config_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_ims_service_enable_config_resp_wifi_calling_enabled(TRUE);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 1;
  ims_ptr->has_boolValue = FALSE;
  t.expect(IMS_node);


#if 0
  // OLD IMSS APIs


  /* ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED */
  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ConfigMsg* ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 1; //IMS_SETTINGS_WFC_STATUS_ON_V01
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_client_provisioning_config_req_v01_wifi_call(IMS_SETTINGS_WFC_STATUS_ON_V01);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
  t.expect(IMS_node);

  /* ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_MODE */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ConfigMsg* ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_MODE;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 1; //cellular preferred
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_client_provisioning_config_req_v01_wifi_call_preference(IMS_SETTINGS_WFC_CELLULAR_PREFERRED_V01);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_MODE;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_MODE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_rsp_v01();
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_wifi_call_preference(IMS_SETTINGS_WFC_CELLULAR_PREFERRED_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_MODE;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 1;
  ims_ptr->has_boolValue = FALSE;
  t.expect(IMS_node);
#endif


  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_set_and_get_client_provisioning_config_items_L(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id("ril_utf_ims_set_and_get_client_provisioning_config_items_L");
  t.set_test_description("Tests Ims Config items related to qmi_imss_client_provisioning_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  /* ims_ConfigItem_CONFIG_ITEM_VLT_SETTING_ENABLED */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ConfigMsg* ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VLT_SETTING_ENABLED;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 0;
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_client_provisioning_config_req_v01_enable_volte(FALSE);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VLT_SETTING_ENABLED;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VLT_SETTING_ENABLED;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_rsp_v01();
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_enable_volte(FALSE);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VLT_SETTING_ENABLED;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 0;
  ims_ptr->has_boolValue = FALSE;
  t.expect(IMS_node);

  /* ims_ConfigItem_CONFIG_ITEM_LVC_SETTING_ENABLED */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_LVC_SETTING_ENABLED;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 10;
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_client_provisioning_config_req_v01_enable_vt(TRUE);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_LVC_SETTING_ENABLED;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_LVC_SETTING_ENABLED;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_rsp_v01();
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_enable_vt(TRUE);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_LVC_SETTING_ENABLED;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 1;
  ims_ptr->has_boolValue = FALSE;
  t.expect(IMS_node);

  /* ims_ConfigItem_CONFIG_ITEM_EAB_SETTING_ENABLED */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_EAB_SETTING_ENABLED;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 1;
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_client_provisioning_config_req_v01_enable_presence(TRUE);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_EAB_SETTING_ENABLED;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_EAB_SETTING_ENABLED;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_rsp_v01();
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_enable_presence(TRUE);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_EAB_SETTING_ENABLED;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 1;
  ims_ptr->has_boolValue = FALSE;
  t.expect(IMS_node);


  /* ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 1; //wifi calling on
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_client_provisioning_config_req_v01_wifi_call(IMS_SETTINGS_WFC_STATUS_ON_V01);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_rsp_v01();
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_wifi_call(IMS_SETTINGS_WFC_STATUS_OFF_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 2;
  ims_ptr->has_boolValue = FALSE;
  t.expect(IMS_node);

  /* ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 1; //wifi calling on
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_client_provisioning_config_req_v01_wifi_call(IMS_SETTINGS_WFC_STATUS_ON_V01);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_rsp_v01();
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_wifi_call(IMS_SETTINGS_WFC_STATUS_OFF_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 2;
  ims_ptr->has_boolValue = FALSE;
  t.expect(IMS_node);

  /* ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 2; //wifi roaming disabled
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_client_provisioning_config_req_v01_wifi_call_roaming(IMS_SETTINGS_WFC_ROAMING_DISABLED_V01);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_rsp_v01();
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_wifi_call_roaming(IMS_SETTINGS_WFC_ROAMING_NOT_SUPPORTED_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 0;
  ims_ptr->has_boolValue = FALSE;
  t.expect(IMS_node);

  /* ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_MODE */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_MODE;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 3; //cellular preferred
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_client_provisioning_config_req_v01_wifi_call_preference(IMS_SETTINGS_WFC_CELLULAR_PREFERRED_V01);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_MODE;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_MODE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_rsp_v01();
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_wifi_call_preference(IMS_SETTINGS_WFC_CELLULAR_ONLY_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_VOICE_OVER_WIFI_MODE;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 4;
  ims_ptr->has_boolValue = FALSE;
  t.expect(IMS_node);


  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_set_and_get_qipcall_config_items_L(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id("ril_utf_ims_set_and_get_qipcall_config_items_L");
  t.set_test_description("Tests Ims Config items related to qmi_imss_qipcall_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  /* ims_ConfigItem_CONFIG_ITEM_MOBILE_DATA_ENABLED */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ConfigMsg* ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_MOBILE_DATA_ENABLED;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 10;
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_qipcall_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_qipcall_config_req_v01_mobile_data_enabled(TRUE);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_qipcall_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_MOBILE_DATA_ENABLED;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_MOBILE_DATA_ENABLED;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_qipcall_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_qipcall_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_qipcall_config_rsp_v01_mobile_data_enabled(FALSE);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_MOBILE_DATA_ENABLED;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 0;
  ims_ptr->has_boolValue = FALSE;
  t.expect(IMS_node);

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_set_and_get_sms_config_items_L(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id("ril_utf_ims_set_and_get_sms_config_items_L");
  t.set_test_description("Tests Ims Config items related to qmi_imss_sms_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  /* ims_ConfigItem_CONFIG_ITEM_SMS_FORMAT */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ConfigMsg* ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SMS_FORMAT;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 0;
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_sms_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_sms_config_req_v01_sms_format(IMS_SETTINGS_SMS_FORMAT_3GPP2_V01);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_sms_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SMS_FORMAT;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SMS_FORMAT;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_sms_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_sms_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_sms_config_rsp_v01_sms_format(IMS_SETTINGS_SMS_FORMAT_3GPP_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SMS_FORMAT;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 1;
  ims_ptr->has_boolValue = FALSE;
  t.expect(IMS_node);

  /* ims_ConfigItem_CONFIG_ITEM_SMS_FORMAT */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SMS_OVER_IP;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 0;
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_sms_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_sms_config_req_v01_sms_over_ip_network_indication(FALSE);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_sms_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SMS_OVER_IP;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SMS_OVER_IP;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_sms_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_sms_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_sms_config_rsp_v01_sms_over_ip_network_indication(TRUE);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SMS_OVER_IP;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 1;
  ims_ptr->has_boolValue = FALSE;
  t.expect(IMS_node);

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

test_result ril_utf_ims_set_and_get_presence_config_items_L(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id("ril_utf_ims_set_and_get_presence_config_items_L");
  t.set_test_description("Tests Ims Config items related to qmi_imss_presence_config.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  /* ims_ConfigItem_CONFIG_ITEM_PUBLISH_TIMER */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ConfigMsg* ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_PUBLISH_TIMER;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 45;
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_presence_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_presence_config_req_v01_publish_expiry_timer(45);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_presence_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_PUBLISH_TIMER;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_PUBLISH_TIMER;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_presence_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_presence_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_presence_config_rsp_v01_publish_expiry_timer(5);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_PUBLISH_TIMER;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 5;
  ims_ptr->has_boolValue = FALSE;
  t.expect(IMS_node);

  /* ims_ConfigItem_CONFIG_ITEM_PUBLISH_TIMER_EXTENDED */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_PUBLISH_TIMER_EXTENDED;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 40000;
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_presence_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_presence_config_req_v01_publish_extended_expiry_timer(40000);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_presence_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_PUBLISH_TIMER_EXTENDED;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_PUBLISH_TIMER_EXTENDED;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_presence_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_presence_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_presence_config_rsp_v01_publish_extended_expiry_timer(2000);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_PUBLISH_TIMER_EXTENDED;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 2000;
  ims_ptr->has_boolValue = FALSE;
  t.expect(IMS_node);

  /* ims_ConfigItem_CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 0;
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_presence_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_presence_config_req_v01_capabilites_cache_expiration(0);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_presence_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_presence_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_presence_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_presence_config_rsp_v01_capabilites_cache_expiration(10);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 10;
  ims_ptr->has_boolValue = FALSE;
  t.expect(IMS_node);

  /* ims_ConfigItem_CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 100;
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_presence_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_presence_config_req_v01_availability_cache_expiration(100);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_presence_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_presence_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_presence_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_presence_config_rsp_v01_availability_cache_expiration(1000);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 1000;
  ims_ptr->has_boolValue = FALSE;
  t.expect(IMS_node);

  /* ims_ConfigItem_CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 4567;
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_presence_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_presence_config_req_v01_capability_poll_interval(4567);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_presence_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_presence_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_presence_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_presence_config_rsp_v01_capability_poll_interval(876543);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 876543;
  ims_ptr->has_boolValue = FALSE;
  t.expect(IMS_node);

  /* ims_ConfigItem_CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 4567;
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_presence_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_presence_config_req_v01_minimum_publish_interval(4567);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_presence_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_presence_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_presence_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_presence_config_rsp_v01_minimum_publish_interval(111111);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 111111;
  ims_ptr->has_boolValue = FALSE;
  t.expect(IMS_node);

  /* ims_ConfigItem_CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 18000;
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_presence_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_presence_config_req_v01_max_subcription_list_entries(18000);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_presence_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_presence_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_presence_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_presence_config_rsp_v01_max_subcription_list_entries(19000);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 19000;
  ims_ptr->has_boolValue = FALSE;
  t.expect(IMS_node);

  /* ims_ConfigItem_CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 18000;
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_presence_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_presence_config_req_v01_capability_poll_list_subscription_expiry_timer(18000);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_presence_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_presence_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_presence_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_presence_config_rsp_v01_capability_poll_list_subscription_expiry_timer(19000);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 19000;
  ims_ptr->has_boolValue = FALSE;
  t.expect(IMS_node);

  /* ims_ConfigItem_CONFIG_ITEM_GZIP_FLAG */

  //set
  IMS_node.update_default_ims_request_set_config();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_GZIP_FLAG;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 1;
  ims_ptr->has_boolValue = FALSE;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_set_presence_config_req_v01();
  QMI_node.set_qmi_ims_settings_set_presence_config_req_v01_gzip_enabled(TRUE);
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_set_presence_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_set_config_resp();
  ims_ptr = IMS_node.set_ims_request_set_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_GZIP_FLAG;
  t.expect(IMS_node);

  //get
  IMS_node.update_default_ims_request_get_config();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_GZIP_FLAG;
  t.inject(IMS_node);

  QMI_node.update_default_qmi_ims_settings_get_presence_config_req_v01();
  t.expect(QMI_node);

  QMI_node.update_default_qmi_ims_settings_get_presence_config_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_presence_config_rsp_v01_gzip_enabled(FALSE);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_get_config_resp();
  ims_ptr = IMS_node.set_ims_request_get_config();
  ims_ptr->has_item = TRUE;
  ims_ptr->item = ims_ConfigItem_CONFIG_ITEM_GZIP_FLAG;
  ims_ptr->has_intValue = TRUE;
  ims_ptr->intValue = 0;
  ims_ptr->has_boolValue = FALSE;
  t.expect(IMS_node);

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}
