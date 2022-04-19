/******************************************************************************
#  Copyright (c) 2016 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_test_qmi_ril_lte_direct.cpp
  @brief   RIL UTF test case definitions

  DESCRIPTION
  LTE Direct Discovery test cases
  ---------------------------------------------------------------------------
******************************************************************************/

#include "ril_utf_test_qmi_ril.h"
#include "ril_utf_test_qmi_ril_L.h"

/**==
@testname: ril_utf_lte_direct_disc_get_device_capability_001
@description:
==**/
test_result ril_utf_lte_direct_disc_get_device_capability_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;
  OEMTestnode OEM_node;

  t.set_test_id(__func__);
  t.set_test_attribute("LTED");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: LteDirectDiscovery_MsgId_REQUEST_GET_DEVICE_CAPABILITY
  OEM_node.update_default_lte_direct_discovery_get_device_capability_req();
  t.inject(OEM_node);

  // Expect: QMI_LTE_GET_SUBSCRIPTION_INFO_REQ_V01
  QMI_node.update_default_qmi_lte_get_subscription_info_req_msg();
  t.expect(QMI_node);

  // Inject: QMI_LTE_GET_SUBSCRIPTION_INFO_RESP_V01
  QMI_node.update_default_qmi_lte_get_subscription_info_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_lte_get_subscription_info_resp_lte_disc_capability(TRUE);
  t.inject(QMI_node);

  // Expect: LteDirectDiscovery_MsgId_REQUEST_GET_DEVICE_CAPABILITY (Resp)
  OEM_node.update_default_lte_direct_discovery_get_device_capability_resp();
  OEM_node.set_lte_direct_discovery_get_device_capability_resp_capability(TRUE);
  t.expect(OEM_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}


/**==
@testname: ril_utf_lte_direct_disc_get_service_status_001
@description:
==**/
test_result ril_utf_lte_direct_disc_get_service_status_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;
  OEMTestnode OEM_node;

  t.set_test_id(__func__);
  t.set_test_attribute("LTED");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }
  // Inject: LteDirectDiscovery_MsgId_REQUEST_GET_SERVICE_STATUS
  OEM_node.update_default_lte_direct_discovery_get_service_status_req();
  t.inject(OEM_node);

  // Expect: QMI_LTE_DISC_GET_SERVICE_STATUS_REQ_V01
  QMI_node.update_default_qmi_lte_disc_get_service_status_req_msg();
  t.expect(QMI_node);

  // Inject: QMI_LTE_DISC_GET_SERVICE_STATUS_RESP_V01
  QMI_node.update_default_qmi_lte_disc_get_service_status_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_lte_disc_get_service_status_resp_publish_allowed(TRUE);
  QMI_node.set_qmi_lte_disc_get_service_status_resp_subscribe_allowed(TRUE);
  t.inject(QMI_node);

  // Expect: LteDirectDiscovery_MsgId_REQUEST_GET_SERVICE_STATUS (Response)
  OEM_node.update_default_lte_direct_discovery_get_service_status_resp();
  OEM_node.set_lte_direct_discovery_get_service_status_resp_publish_allowed(1);
  OEM_node.set_lte_direct_discovery_get_service_status_resp_subscribe_allowed(1);
  t.expect(OEM_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_lte_direct_disc_publish_001
@description:
==**/
test_result ril_utf_lte_direct_disc_publish_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;
  OEMTestnode OEM_node;

  t.set_test_id(__func__);
  t.set_test_attribute("LTED");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: LteDirectDiscovery_MsgId_REQUEST_PUBLISH
  OEM_node.update_default_lte_direct_discovery_publish_req();
  OEM_node.set_lte_direct_discovery_publish_req_os_app_id("test os app id");
  OEM_node.set_lte_direct_discovery_publish_req_expression("test expression");
  OEM_node.set_lte_direct_discovery_publish_req_expression_validity_time(100);
  OEM_node.set_lte_direct_discovery_publish_req_duration(10);
  OEM_node.set_lte_direct_discovery_publish_req_discovery_type(1);
  OEM_node.set_lte_direct_discovery_publish_req_meta_data("test meta data");
  t.inject(OEM_node);

  // Expect: QMI_LTE_DISC_PUBLISH_REQ_V01
  QMI_node.update_default_qmi_lte_disc_publish_req_msg();
  QMI_node.set_qmi_lte_disc_publish_req_os_app_id("test os app id");
  QMI_node.set_qmi_lte_disc_publish_req_expression("test expression");
  QMI_node.set_qmi_lte_disc_publish_req_expression_validity_time(100);
  QMI_node.set_qmi_lte_disc_publish_req_publish_duration(10);
  QMI_node.set_qmi_lte_disc_publish_req_discovery_type(QMI_LTE_DISC_DISCOVERY_OPEN_V01);
  QMI_node.set_qmi_lte_disc_publish_req_meta_data("test meta data");
  t.expect(QMI_node);

  // Inject: QMI_LTE_DISC_PUBLISH_RESP_V01
  QMI_node.update_default_qmi_lte_disc_publish_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_lte_disc_publish_resp_os_app_id("test os app id");
  QMI_node.set_qmi_lte_disc_publish_resp_expression("test expression");
  QMI_node.set_qmi_lte_disc_publish_resp_expression_result(QMI_LTE_DISC_SUCCESS_V01);
  t.inject(QMI_node);

  // Expect: LteDirectDiscovery_MsgId_REQUEST_PUBLISH (Resp)
  OEM_node.update_default_lte_direct_discovery_publish_resp();
  t.expect(OEM_node);

  // Expect: LteDirectDiscovery_MsgId_UNSOL_RESPONSE_EXPRESSION_STATUS
  OEM_node.update_default_lte_direct_discovery_unsol_response_expression_status();
  OEM_node.set_lte_direct_discovery_unsol_response_expression_status_os_app_id("test os app id");
  OEM_node.set_lte_direct_discovery_unsol_response_expression_status_expression("test expression");
  OEM_node.set_lte_direct_discovery_unsol_response_expression_status_result(LteDirectDiscovery_Result_SUCCESS);
  t.expect(OEM_node);

  // Inject: QMI_LTE_DISC_NOTIFICATION_IND_V01
  QMI_node.update_default_qmi_lte_disc_notification_ind_msg();
  QMI_node.set_qmi_lte_disc_notification_ind_os_app_id("test os app id");
  QMI_node.set_qmi_lte_disc_notification_ind_authorization_result(QMI_LTE_DISC_SUCCESS_V01);
  QMI_node.set_qmi_lte_disc_notification_ind_expression("test expression");
  t.inject(QMI_node);

  // Expect: LteDirectDiscovery_MsgId_UNSOL_RESPONSE_AUTHORIZATION_RESULT
  OEM_node.update_default_lte_direct_discovery_unsol_response_authorization_result();
  OEM_node.set_lte_direct_discovery_unsol_response_authorization_result_os_app_id("test os app id");
  OEM_node.set_lte_direct_discovery_unsol_response_authorization_result_result(LteDirectDiscovery_Result_SUCCESS);
  t.expect(OEM_node);

  // Inject: QMI_LTE_DISC_NOTIFICATION_IND_V01
  QMI_node.update_default_qmi_lte_disc_notification_ind_msg();
  QMI_node.set_qmi_lte_disc_notification_ind_os_app_id("test os app id");
  QMI_node.set_qmi_lte_disc_notification_ind_expression("test expression");
  QMI_node.set_qmi_lte_disc_notification_ind_publish_status_per_expr(1); // PENDING
  t.inject(QMI_node);

  // Expect: LteDirectDiscovery_MsgId_UNSOL_RESPONSE_TRANSMISSION_STATUS
  OEM_node.update_default_lte_direct_discovery_unsol_response_transmission_status();
  OEM_node.set_lte_direct_discovery_unsol_response_transmission_status_os_app_id("test os app id");
  OEM_node.set_lte_direct_discovery_unsol_response_transmission_status_expression("test expression");
  OEM_node.set_lte_direct_discovery_unsol_response_transmission_status_status(1);  // TODO check value
  t.expect(OEM_node);

  // Inject: LteDirectDiscovery_MsgId_REQUEST_CANCEL_PUBLISH
  OEM_node.update_default_lte_direct_discovery_cancel_publish_req();
  OEM_node.set_lte_direct_discovery_cancel_publish_req_os_app_id("test os app id");
  OEM_node.set_lte_direct_discovery_cancel_publish_req_expression("test expression");
  t.inject(OEM_node);

  // Expect: QMI_LTE_DISC_CANCEL_PUBLISH_REQ_V01
  QMI_node.update_default_qmi_lte_disc_cancel_publish_req_msg();
  QMI_node.set_qmi_lte_disc_cancel_publish_req_os_app_id("test os app id");
  QMI_node.set_qmi_lte_disc_cancel_publish_req_expression("test expression");
  t.expect(QMI_node);

  // Inject: QMI_LTE_DISC_PUBLISH_RESP_V01
  QMI_node.update_default_qmi_lte_disc_cancel_publish_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_lte_disc_cancel_publish_resp_os_app_id("test os app id");
  t.inject(QMI_node);

  // Expect: LteDirectDiscovery_MsgId_REQUEST_CANCEL_PUBLISH (Resp)
  OEM_node.update_default_lte_direct_discovery_cancel_publish_resp();
  OEM_node.update_default_lte_direct_discovery_cancel_publish_resp();
  t.expect(OEM_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_lte_direct_disc_subscribe_001
@description:
==**/
test_result ril_utf_lte_direct_disc_subscribe_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;
  OEMTestnode OEM_node;

  t.set_test_id(__func__);
  t.set_test_attribute("LTED");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: LteDirectDiscovery_MsgId_REQUEST_SUBSCRIBE
  OEM_node.update_default_lte_direct_discovery_subscribe_req();
  OEM_node.set_lte_direct_discovery_subscribe_req_os_app_id("test os app id");
  OEM_node.set_lte_direct_discovery_subscribe_req_expression("test expression subscribe");
  OEM_node.set_lte_direct_discovery_subscribe_req_expression_validity_time(100);
  OEM_node.set_lte_direct_discovery_subscribe_req_duration(10);
  OEM_node.set_lte_direct_discovery_subscribe_req_discovery_type(1);
  t.inject(OEM_node);

  // Expect: QMI_LTE_DISC_SUBSCRIBE_REQ_V01
  QMI_node.update_default_qmi_lte_disc_subscribe_req_msg();
  QMI_node.set_qmi_lte_disc_subscribe_req_os_app_id("test os app id");
  QMI_node.set_qmi_lte_disc_subscribe_req_expression("test expression subscribe");
  QMI_node.set_qmi_lte_disc_subscribe_req_expression_validity_time(100);
  QMI_node.set_qmi_lte_disc_subscribe_req_subscribe_duration(10);
  QMI_node.set_qmi_lte_disc_subscribe_req_discovery_type(QMI_LTE_DISC_DISCOVERY_OPEN_V01);
  t.expect(QMI_node);

  // Inject: QMI_LTE_DISC_SUBSCRIBE_RESP_V01
  QMI_node.update_default_qmi_lte_disc_subscribe_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_lte_disc_subscribe_resp_os_app_id("test os app id");
  QMI_node.set_qmi_lte_disc_subscribe_resp_expression("test expression subscribe");
  QMI_node.set_qmi_lte_disc_subscribe_resp_expression_result(QMI_LTE_DISC_SUCCESS_V01);
  t.inject(QMI_node);

  // Expect: LteDirectDiscovery_MsgId_REQUEST_SUBSCRIBE (Resp)
  OEM_node.update_default_lte_direct_discovery_subscribe_resp();
  t.expect(OEM_node);

  // Expect: LteDirectDiscovery_MsgId_UNSOL_RESPONSE_EXPRESSION_STATUS
  OEM_node.update_default_lte_direct_discovery_unsol_response_expression_status();
  OEM_node.set_lte_direct_discovery_unsol_response_expression_status_os_app_id("test os app id");
  OEM_node.set_lte_direct_discovery_unsol_response_expression_status_expression("test expression subscribe");
  OEM_node.set_lte_direct_discovery_unsol_response_expression_status_result(LteDirectDiscovery_Result_SUCCESS);
  t.expect(OEM_node);

  // Inject: QMI_LTE_DISC_NOTIFICATION_IND_V01
  QMI_node.update_default_qmi_lte_disc_notification_ind_msg();
  QMI_node.set_qmi_lte_disc_notification_ind_os_app_id("test os app id");
  QMI_node.set_qmi_lte_disc_notification_ind_authorization_result(QMI_LTE_DISC_SUCCESS_V01);
  QMI_node.set_qmi_lte_disc_notification_ind_expression("test expression subscribe");
  t.inject(QMI_node);

  // Expect: LteDirectDiscovery_MsgId_UNSOL_RESPONSE_AUTHORIZATION_RESULT
  OEM_node.update_default_lte_direct_discovery_unsol_response_authorization_result();
  OEM_node.set_lte_direct_discovery_unsol_response_authorization_result_os_app_id("test os app id");
  OEM_node.set_lte_direct_discovery_unsol_response_authorization_result_result(LteDirectDiscovery_Result_SUCCESS);
  t.expect(OEM_node);

  // Inject: QMI_LTE_DISC_NOTIFICATION_IND_V01
  QMI_node.update_default_qmi_lte_disc_notification_ind_msg();
  QMI_node.set_qmi_lte_disc_notification_ind_os_app_id("test os app id");
  QMI_node.set_qmi_lte_disc_notification_ind_expression("test expression subscribe");
  QMI_node.set_qmi_lte_disc_notification_ind_subscribe_status_per_expr(1); // PENDING
  t.inject(QMI_node);

  // Expect: LteDirectDiscovery_MsgId_UNSOL_RESPONSE_RECEPTION_STATUS
  OEM_node.update_default_lte_direct_discovery_unsol_response_reception_status();
  OEM_node.set_lte_direct_discovery_unsol_response_reception_status_os_app_id("test os app id");
  OEM_node.set_lte_direct_discovery_unsol_response_reception_status_expression("test expression subscribe");
  OEM_node.set_lte_direct_discovery_unsol_response_reception_status_status(1);  // TODO check value
  t.expect(OEM_node);

  // Inject: QMI_LTE_DISC_MATCH_NOTIFICATION_IND_V01
  QMI_node.update_default_qmi_lte_disc_match_notification_ind_msg();
  QMI_node.set_qmi_lte_disc_match_notification_ind_os_app_id("test os app id");
  QMI_node.set_qmi_lte_disc_match_notification_ind_expression("test expression subscribe");
  QMI_node.set_qmi_lte_disc_match_notification_ind_matched_expression("test expression subscribe match");
  QMI_node.set_qmi_lte_disc_match_notification_ind_match_event_state(0); // MATCH START
  QMI_node.set_qmi_lte_disc_match_notification_ind_meta_data_index(0); // TODO?
  QMI_node.set_qmi_lte_disc_match_notification_ind_meta_data("meta data subscribe");
  t.inject(QMI_node);

  // Expect: LteDirectDiscovery_MsgId_UNSOL_RESPONSE_MATCH_EVENT
  OEM_node.update_default_lte_direct_discovery_unsol_response_match_event();
  OEM_node.set_lte_direct_discovery_unsol_response_match_event_os_app_id("test os app id");
  OEM_node.set_lte_direct_discovery_unsol_response_match_event_expression("test expression subscribe");
  OEM_node.set_lte_direct_discovery_unsol_response_match_event_state(0);  // TODO check value
  OEM_node.set_lte_direct_discovery_unsol_response_match_event_metadata("meta data subscribe");
  t.expect(OEM_node);

  // Inject: QMI_LTE_DISC_BROADCAST_NOTIFICATION_IND_V01
  QMI_node.update_default_qmi_lte_disc_broadcast_notification_ind_msg();
  QMI_node.set_qmi_lte_disc_broadcast_notification_ind_publish_allowed(1);
  QMI_node.set_qmi_lte_disc_broadcast_notification_ind_subscribe_allowed(1);
  t.inject(QMI_node);

  // Expect: LteDirectDiscovery_MsgId_UNSOL_RESPONSE_SERVICE_STATUS
  OEM_node.update_default_lte_direct_discovery_unsol_response_service_status();
  OEM_node.set_lte_direct_discovery_unsol_response_service_status_publish_allowed(1);
  OEM_node.set_lte_direct_discovery_unsol_response_service_status_subscribe_allowed(1);
  t.expect(OEM_node);

  // Inject: QMI_LTE_DISC_NOTIFICATION_IND_V01
  QMI_node.update_default_qmi_lte_disc_notification_ind_msg();
  QMI_node.set_qmi_lte_disc_notification_ind_os_app_id("test os app id");
  QMI_node.set_qmi_lte_disc_notification_ind_expression("test expression subscribe");
  QMI_node.set_qmi_lte_disc_notification_ind_subscribe_status_per_expr(0); // COMPLETED
  t.inject(QMI_node);

  // Expect: LteDirectDiscovery_MsgId_UNSOL_RESPONSE_RECEPTION_STATUS
  OEM_node.update_default_lte_direct_discovery_unsol_response_reception_status();
  OEM_node.set_lte_direct_discovery_unsol_response_reception_status_os_app_id("test os app id");
  OEM_node.set_lte_direct_discovery_unsol_response_reception_status_expression("test expression subscribe");
  OEM_node.set_lte_direct_discovery_unsol_response_reception_status_status(0);  // TODO check value
  t.expect(OEM_node);

  // Inject: LteDirectDiscovery_MsgId_REQUEST_CANCEL_SUBSCRIBE
  OEM_node.update_default_lte_direct_discovery_cancel_subscribe_req();
  OEM_node.set_lte_direct_discovery_cancel_subscribe_req_os_app_id("test os app id");
  OEM_node.set_lte_direct_discovery_cancel_subscribe_req_expression("test expression");
  t.inject(OEM_node);

  // Expect: QMI_LTE_DISC_CANCEL_SUBSCRIBE_REQ_V01
  QMI_node.update_default_qmi_lte_disc_cancel_subscribe_req_msg();
  QMI_node.set_qmi_lte_disc_cancel_subscribe_req_os_app_id("test os app id");
  QMI_node.set_qmi_lte_disc_cancel_subscribe_req_expression("test expression");
  t.expect(QMI_node);

  // Inject: QMI_LTE_DISC_CANCEL_SUBSCRIBE_RESP_V01
  QMI_node.update_default_qmi_lte_disc_cancel_subscribe_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_lte_disc_cancel_subscribe_resp_os_app_id("test os app id");
  t.inject(QMI_node);

  // Expect: LteDirectDiscovery_MsgId_REQUEST_CANCEL_SUBSCRIBE (Resp)
  OEM_node.update_default_lte_direct_discovery_cancel_subscribe_resp();
  t.expect(OEM_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_lte_direct_disc_terminate_001
@description:
==**/
test_result ril_utf_lte_direct_disc_terminate_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;
  OEMTestnode OEM_node;

  t.set_test_id(__func__);
  t.set_test_attribute("LTED");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: LteDirectDiscovery_MsgId_REQUEST_TERMINATE
  OEM_node.update_default_lte_direct_discovery_terminate_req();
  OEM_node.set_lte_direct_discovery_terminate_req_os_app_id("test os app id");
  t.inject(OEM_node);

  // Expect: QMI_LTE_DISC_TERMINATE_REQ_V01
  QMI_node.update_default_qmi_lte_disc_terminate_req_msg();
  QMI_node.set_qmi_lte_disc_terminate_req_os_app_id("test os app id");
  t.expect(QMI_node);

  // Inject: QMI_LTE_DISC_TERMINATE_RESP_V01
  QMI_node.update_default_qmi_lte_disc_terminate_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: LteDirectDiscovery_MsgId_REQUEST_TERMINATE (Resp)
  OEM_node.update_default_lte_direct_discovery_terminate_resp();
  t.expect(OEM_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}
