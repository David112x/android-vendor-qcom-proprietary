/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_target_test_qmi_ril.cpp
  @brief   RIL UTF test case definitions

  DESCRIPTION
  All test case definitions should be in this file
  ---------------------------------------------------------------------------
******************************************************************************/

#include "ril_utf_target_test_qmi_ril.h"


test_result qmi_ril_target_bootup_001()
{
  // this part need to be included for all test cases
  // test ID and attributes for each test case are set here

  Testcase t;
  AndroidTelTestnode RIL_node;
  IMSTestnode IMS_node;
  OEMTestnode OEM_node;

  t.set_test_id("qmi_ril_target_bootup_001");
  t.set_test_description("This test is necessary to boot qmi_ril"
    "test must be run before any other tests are run");
  t.set_test_attribute("ALL");

  // Inject: RIL_REQUEST_RADIO_POWER
  RIL_node.update_default_ril_request_radio_power();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_RADIO_POWER (resp)
  RIL_node.update_default_ril_request_radio_power_resp();
  t.expect(RIL_node);

  // Inject: RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE
  RIL_PreferredNetworkType* net_type = RIL_node.update_default_ril_request_set_preferred_network_type();
  *net_type = PREF_NET_TYPE_LTE_ONLY;
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE (resp)
  RIL_node.update_default_ril_request_set_preferred_network_type_resp();
  t.expect(RIL_node);

/*
  // Start polling loop (we want to repeat these messages until reg state is correct)
  t.loop_start();

  // Expect: RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_voice_network_state_changed();
  t.expect(RIL_node);

  // Inject: RIL_REQUEST_VOICE_REGISTRATION_STATE
  RIL_node.update_default_ril_request_voice_registration_state();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_VOICE_REGISTRATION_STATE (resp)
  char **reg_data = RIL_node.update_default_ril_request_voice_registration_state_resp();
  strlcpy(reg_data[0], "1", 2);
  t.expect(RIL_node);

  // End polling loop
  t.loop_end();
  */

  // Execute the test case
  t.execute();

  t.print_summary();

  // Start oem threads for oem socket
  //utf_start_oem_threads();

  // Start the ims threads for ims
  utf_start_ims_threads();

  //////////////  IMS init test //////////////////////
  Testcase ims_init;
  ims_init.set_test_id("qmi_ril_target_bootup_001");

  // Expect: IMS__MSG_ID__UNSOL_RADIO_STATE_CHANGED
  IMS_node.update_default_ims_unsol_radio_state_changed();
  ims_init.expect(IMS_node);

  ims_init.loop_start();

  // Expect: IMS__MSG_ID__UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED
  IMS_node.update_default_ims_unsol_response_ims_network_state_changed();
  ims_init.expect(IMS_node);

  // Inject: IMS__MSG_ID__REQUEST_IMS_REGISTRATION_STATE
  IMS_node.update_default_ims_request_registration_state();
  ims_init.inject(IMS_node);

  // Expect: IMS__MSG_ID__REQUEST_IMS_REGISTRATION_STATE
  IMS_node.update_default_ims_request_registration_state_resp();
  IMS_node.set_ims_request_registration_state_resp_state(IMS__REGISTRATION__REG_STATE__REGISTERED);
  ims_init.expect(IMS_node);

  ims_init.loop_end();

  ims_init.execute();
  ims_init.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_target_mo_call_001
@description: This test creates an MO call through a dial request. Modem then steps through call states to active. The call is then torn down with hangup request.
==**/
test_result ril_utf_target_mo_call_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;

  t.set_test_id("ril_utf_target_mo_call_001");
  t.set_test_description("This test creates an MO call through an ATEL dial request and then ends the call with an ATEL hangup request");
  t.set_test_attribute("VOICE");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: RIL_REQUEST_DIAL
  RIL_node.update_default_ril_request_dial("9702151484");
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_DIAL (resp)
  RIL_node.update_default_ril_request_dial_resp();
  t.expect(RIL_node);

  // Expect: RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_call_state_changed();
  t.expect(RIL_node);

  // Inject: RIL_REQUEST_GET_CURRENT_CALLS
  RIL_node.update_default_ril_request_get_current_calls();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_GET_CURRENT_CALLS (resp)
  RIL_Call** call = RIL_node.update_default_ril_request_get_current_calls_resp(1);
  call[0]->state = RIL_CALL_DIALING;
  call[0]->toa = 129;
  call[0]->isVoice = 1;
  call[0]->index = 1;
  t.expect(RIL_node);

  // Expect: RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_call_state_changed();
  t.expect(RIL_node);

  // Inject: RIL_REQUEST_GET_CURRENT_CALLS
  RIL_node.update_default_ril_request_get_current_calls();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_GET_CURRENT_CALLS (resp)
  call = RIL_node.update_default_ril_request_get_current_calls_resp(1);
  call[0]->state = RIL_CALL_DIALING;
  call[0]->toa = 129;
  call[0]->isVoice = 1;
  call[0]->index = 1;
  t.expect(RIL_node);

  // Expect: RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_call_state_changed();
  t.expect(RIL_node);

  // Inject: RIL_REQUEST_GET_CURRENT_CALLS
  RIL_node.update_default_ril_request_get_current_calls();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_GET_CURRENT_CALLS (resp)
  call = RIL_node.update_default_ril_request_get_current_calls_resp(1);
  call[0]->state = RIL_CALL_ALERTING;
  call[0]->toa = 129;
  call[0]->isVoice = 1;
  call[0]->index = 1;
  t.expect(RIL_node);

  // Expect: RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_call_state_changed();
  t.expect(RIL_node);

  // Inject: RIL_REQUEST_GET_CURRENT_CALLS
  RIL_node.update_default_ril_request_get_current_calls();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_GET_CURRENT_CALLS (resp)
  call = RIL_node.update_default_ril_request_get_current_calls_resp(1);
  call[0]->state = RIL_CALL_ACTIVE;
  call[0]->toa = 129;
  call[0]->isVoice = 1;
  call[0]->index = 1;
  t.expect(RIL_node);

  // Delay to allow call to exist for a moment
  t.sleep(5000);

  // Inject: RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND
  RIL_node.update_default_ril_request_hangup_foreground_resume_background();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND
  RIL_node.update_default_ril_request_hangup_foreground_resume_background_resp();
  t.expect(RIL_node);

  // Expect: RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_call_state_changed();
  t.expect(RIL_node);

  // Inject: RIL_REQUEST_GET_CURRENT_CALLS
  RIL_node.update_default_ril_request_get_current_calls();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_GET_CURRENT_CALLS (resp)
  RIL_node.update_default_ril_request_get_current_calls_resp(0);
  t.expect(RIL_node);

/*
  // Inject: RIL_REQUEST_LAST_CALL_FAIL_CAUSE
  RIL_node.update_default_ril_request_last_call_fail_cause();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_LAST_CALL_FAIL_CAUSE
  ril_request_last_call_fail_cause_resp_t *fail_cause =
    RIL_node.update_default_ril_request_last_call_fail_cause_resp();
  fail_cause->failure_reason = CALL_FAIL_NORMAL;
  t.expect(RIL_node);
  */

  t.execute();

  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_target_mt_call_001
@description: This test creates an MT call. Modem then steps through call states to active. The call is then torn down with hangup request.
==**/
test_result ril_utf_target_mt_call_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;

  t.set_test_id(__func__);
  t.set_test_attribute("VOICE");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Expect: RIL_UNSOL_CALL_RING
  RIL_node.update_default_ril_unsol_call_ring();
  t.expect(RIL_node);

  // Expect: RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_call_state_changed();
  t.expect(RIL_node);

  // Inject: RIL_REQUEST_GET_CURRENT_CALLS
  RIL_node.update_default_ril_request_get_current_calls();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_GET_CURRENT_CALLS (resp)
  RIL_node.update_default_ril_request_get_current_calls_resp(0);
  t.expect(RIL_node);

  // Expect: RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_call_state_changed();
  t.expect(RIL_node);

  // Inject: RIL_REQUEST_GET_CURRENT_CALLS
  RIL_node.update_default_ril_request_get_current_calls();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_GET_CURRENT_CALLS (resp)
  RIL_Call** call = RIL_node.update_default_ril_request_get_current_calls_resp(1);
  call[0]->state = RIL_CALL_INCOMING;
  call[0]->toa = 129;
  call[0]->isVoice = 1;
  call[0]->isMT = 1;
  call[0]->index = 1;
  t.expect(RIL_node);

  // Inject: RIL_REQUEST_ANSWER
  RIL_node.update_default_ril_request_answer();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_ANSWER
  RIL_node.update_default_ril_request_answer_resp();
  t.expect(RIL_node);

  // Expect: RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_call_state_changed();
  t.expect(RIL_node);

  // Inject: RIL_REQUEST_GET_CURRENT_CALLS
  RIL_node.update_default_ril_request_get_current_calls();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_GET_CURRENT_CALLS (resp)
  call = RIL_node.update_default_ril_request_get_current_calls_resp(1);
  call[0]->state = RIL_CALL_ACTIVE;
  call[0]->toa = 129;
  call[0]->isVoice = 1;
  call[0]->isMT = 1;
  call[0]->index = 1;
  t.expect(RIL_node);

  // Delay to allow call to exist for a moment
  t.sleep(5000);

  // Inject: RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND
  RIL_node.update_default_ril_request_hangup_foreground_resume_background();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND
  RIL_node.update_default_ril_request_hangup_foreground_resume_background_resp();
  t.expect(RIL_node);

  // Expect: RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_call_state_changed();
  t.expect(RIL_node);

  // Inject: RIL_REQUEST_GET_CURRENT_CALLS
  RIL_node.update_default_ril_request_get_current_calls();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_GET_CURRENT_CALLS (resp)
  RIL_node.update_default_ril_request_get_current_calls_resp(0);
  t.expect(RIL_node);

/*
  // Inject: RIL_REQUEST_LAST_CALL_FAIL_CAUSE
  RIL_node.update_default_ril_request_last_call_fail_cause();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_LAST_CALL_FAIL_CAUSE
  ril_request_last_call_fail_cause_resp_t *fail_cause =
    RIL_node.update_default_ril_request_last_call_fail_cause_resp();
  fail_cause->failure_reason = CALL_FAIL_NORMAL;
  t.expect(RIL_node);
  */


  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}


/**==
@testname: ril_utf_target_ims_mo_call_001
@description: MO LTE call through IMS
==**/
test_result ril_utf_target_ims_mo_call_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id("ril_utf_target_ims_mo_call_001");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

/*
  // Inject: IMS__MSG_ID__REQUEST_DIAL
  Ims__Dial* ptr = IMS_node.update_default_ims_request_dial("55555");
  t.inject(IMS_node);

  // Expect: IMS__MSG_ID__REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: IMS__MSG_ID__UNSOL_RESPONSE_CALL_STATE_CHANGED
  IMS_node.update_default_ims_unsol_response_call_state_changed();
  t.expect(IMS_node);

  // Inject: IMS__MSG_ID__REQUEST_GET_CURRENT_CALLS
  IMS_node.update_default_ims_request_get_current_calls();
  t.inject(IMS_node);

  // Expect: IMS__MSG_ID__REQUEST_GET_CURRENT_CALLS (resp)
  Ims__CallList__Call** calls = IMS_node.update_default_ims_request_get_current_calls_resp(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = IMS__CALL_STATE__CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_ismpty = TRUE;
  calls[0]->has_ismt = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isvoice = TRUE;
  calls[0]->isvoice = 1;
  calls[0]->has_isvoiceprivacy = TRUE;
  calls[0]->isvoiceprivacy = 0;
  calls[0]->calldetails->has_calldomain = TRUE;
  calls[0]->calldetails->calldomain = IMS__CALL_DOMAIN__CALL_DOMAIN_PS;
  calls[0]->calldetails->has_calltype = TRUE;
  calls[0]->calldetails->calltype = IMS__CALL_TYPE__CALL_TYPE_VOICE;
  t.expect(IMS_node);


  // Expect: IMS__MSG_ID__UNSOL_RESPONSE_CALL_STATE_CHANGED
  IMS_node.update_default_ims_unsol_response_call_state_changed();
  t.expect(IMS_node);

  // Inject: IMS__MSG_ID__REQUEST_GET_CURRENT_CALLS
  IMS_node.update_default_ims_request_get_current_calls();
  t.inject(IMS_node);

  // Expect: IMS__MSG_ID__REQUEST_GET_CURRENT_CALLS (resp)
  calls = IMS_node.update_default_ims_request_get_current_calls_resp(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = IMS__CALL_STATE__CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_ismpty = TRUE;
  calls[0]->has_ismt = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isvoice = TRUE;
  calls[0]->isvoice = 1;
  calls[0]->has_isvoiceprivacy = TRUE;
  calls[0]->isvoiceprivacy = 0;
  calls[0]->calldetails->has_calldomain = TRUE;
  calls[0]->calldetails->calldomain = IMS__CALL_DOMAIN__CALL_DOMAIN_PS;
  calls[0]->calldetails->has_calltype = TRUE;
  calls[0]->calldetails->calltype = IMS__CALL_TYPE__CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Expect: IMS__MSG_ID__UNSOL_RESPONSE_CALL_STATE_CHANGED
  IMS_node.update_default_ims_unsol_response_call_state_changed();
  t.expect(IMS_node);

  // Inject: IMS__MSG_ID__REQUEST_GET_CURRENT_CALLS
  IMS_node.update_default_ims_request_get_current_calls();
  t.inject(IMS_node);

  // Expect: IMS__MSG_ID__REQUEST_GET_CURRENT_CALLS (resp)
  calls = IMS_node.update_default_ims_request_get_current_calls_resp(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = IMS__CALL_STATE__CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_ismpty = TRUE;
  calls[0]->has_ismt = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isvoice = TRUE;
  calls[0]->isvoice = 1;
  calls[0]->has_isvoiceprivacy = TRUE;
  calls[0]->isvoiceprivacy = 0;
  calls[0]->calldetails->has_calldomain = TRUE;
  calls[0]->calldetails->calldomain = IMS__CALL_DOMAIN__CALL_DOMAIN_PS;
  calls[0]->calldetails->has_calltype = TRUE;
  calls[0]->calldetails->calltype = IMS__CALL_TYPE__CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Delay to allow call to exist for a moment
  t.sleep(5000);

  // Inject: IMS__MSG_ID__REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND
  IMS_node.update_default_ims_request_hangup_foreground_resume_background();
  t.inject(IMS_node);

  // Expect: IMS__MSG_ID__REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND (resp)
  IMS_node.update_default_ims_request_hangup_foreground_resume_background_resp();
  t.expect(IMS_node);

  // Expect: IMS__MSG_ID__UNSOL_RESPONSE_CALL_STATE_CHANGED
  IMS_node.update_default_ims_unsol_response_call_state_changed();
  t.expect(IMS_node);

  // Inject: IMS__MSG_ID__REQUEST_GET_CURRENT_CALLS
  IMS_node.update_default_ims_request_get_current_calls();
  t.inject(IMS_node);

  // Expect: IMS__MSG_ID__REQUEST_GET_CURRENT_CALLS (resp)
  calls = IMS_node.update_default_ims_request_get_current_calls_resp(0);
  t.expect(IMS_node);

  // Inject: IMS__MSG_ID__REQUEST_LAST_CALL_FAIL_CAUSE
  IMS_node.update_default_ims_request_last_call_fail_cause();
  t.inject(IMS_node);

  // Expect: IMS__MSG_ID__REQUEST_LAST_CALL_FAIL_CAUSE (resp)
  IMS_node.update_default_ims_request_last_call_fail_cause_resp();
  IMS_node.set_ims_request_last_call_fail_cause_failcause(IMS__CALL_FAIL_CAUSE__CALL_FAIL_NORMAL);
  t.expect(IMS_node);
  */

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_mt_call_001
@description: MT LTE call through IMS
==**/
test_result ril_utf_target_ims_mt_call_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id("ril_utf_target_ims_mt_call_001");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }


  // Expect: IMS__MSG_ID__UNSOL_CALL_RING
  IMS_node.update_default_ims_unsol_call_ring();
  t.expect(IMS_node);

  // Expect: IMS__MSG_ID__UNSOL_RESPONSE_CALL_STATE_CHANGED
  IMS_node.update_default_ims_unsol_response_call_state_changed();
  t.expect(IMS_node);

  // Inject: IMS__MSG_ID__REQUEST_GET_CURRENT_CALLS
  IMS_node.update_default_ims_request_get_current_calls();
  t.inject(IMS_node);

  // Expect: IMS__MSG_ID__REQUEST_GET_CURRENT_CALLS (resp)
  Ims__CallList__Call** calls = IMS_node.update_default_ims_request_get_current_calls_resp(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = IMS__CALL_STATE__CALL_INCOMING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_ismpty = TRUE;
  calls[0]->has_ismt = TRUE;
  calls[0]->ismt = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isvoice = TRUE;
  calls[0]->isvoice = 1;
  calls[0]->has_isvoiceprivacy = TRUE;
  calls[0]->isvoiceprivacy = 0;
  calls[0]->calldetails->has_calldomain = TRUE;
  calls[0]->calldetails->calldomain = IMS__CALL_DOMAIN__CALL_DOMAIN_PS;
  calls[0]->calldetails->has_calltype = TRUE;
  calls[0]->calldetails->calltype = IMS__CALL_TYPE__CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: IMS__MSG_ID__REQUEST_ANSWER
  IMS_node.update_default_ims_request_answer();
  t.inject(IMS_node);

  // Expect: IMS__MSG_ID__REQUEST_ANSWER (resp)
  IMS_node.update_default_ims_request_answer_resp();
  t.expect(IMS_node);

  // Expect: IMS__MSG_ID__UNSOL_RESPONSE_CALL_STATE_CHANGED
  IMS_node.update_default_ims_unsol_response_call_state_changed();
  t.expect(IMS_node);

  // Inject: IMS__MSG_ID__REQUEST_GET_CURRENT_CALLS
  IMS_node.update_default_ims_request_get_current_calls();
  t.inject(IMS_node);

  // Expect: IMS__MSG_ID__REQUEST_GET_CURRENT_CALLS (resp)
  calls = IMS_node.update_default_ims_request_get_current_calls_resp(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = IMS__CALL_STATE__CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_ismpty = TRUE;
  calls[0]->has_ismt = TRUE;
  calls[0]->ismt = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isvoice = TRUE;
  calls[0]->isvoice = 1;
  calls[0]->has_isvoiceprivacy = TRUE;
  calls[0]->isvoiceprivacy = 0;
  calls[0]->calldetails->has_calldomain = TRUE;
  calls[0]->calldetails->calldomain = IMS__CALL_DOMAIN__CALL_DOMAIN_PS;
  calls[0]->calldetails->has_calltype = TRUE;
  calls[0]->calldetails->calltype = IMS__CALL_TYPE__CALL_TYPE_VOICE;
  t.expect(IMS_node);


  // Delay to allow call to exist for a moment
  t.sleep(5000);

  // Inject: IMS__MSG_ID__REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND
  IMS_node.update_default_ims_request_hangup_foreground_resume_background();
  t.inject(IMS_node);

  // Expect: IMS__MSG_ID__REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND (resp)
  IMS_node.update_default_ims_request_hangup_foreground_resume_background_resp();
  t.expect(IMS_node);

  // Expect: IMS__MSG_ID__UNSOL_RESPONSE_CALL_STATE_CHANGED
  IMS_node.update_default_ims_unsol_response_call_state_changed();
  t.expect(IMS_node);

  // Inject: IMS__MSG_ID__REQUEST_GET_CURRENT_CALLS
  IMS_node.update_default_ims_request_get_current_calls();
  t.inject(IMS_node);

  // Expect: IMS__MSG_ID__REQUEST_GET_CURRENT_CALLS (resp)
  IMS_node.update_default_ims_request_get_current_calls_resp(0);
  t.expect(IMS_node);

  // Inject: IMS__MSG_ID__REQUEST_LAST_CALL_FAIL_CAUSE
  IMS_node.update_default_ims_request_last_call_fail_cause();
  t.inject(IMS_node);

  // Expect: IMS__MSG_ID__REQUEST_LAST_CALL_FAIL_CAUSE (resp)
  IMS_node.update_default_ims_request_last_call_fail_cause_resp();
  IMS_node.set_ims_request_last_call_fail_cause_failcause(IMS__CALL_FAIL_CAUSE__CALL_FAIL_NORMAL);
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}


#ifdef QCRIL_L_MASTER
/**==
@testname: ril_utf_target_ims_L_mt_call_001
@description: MT LTE call through IMS
==**/
test_result ril_utf_target_L_ims_mt_call_001(user_specified_attrib_t attr)
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

  // Expect: IMS__MSG_ID__UNSOL_CALL_RING
  IMS_node.update_default_ims_unsol_call_ring();
  t.expect(IMS_node);

  // Expect: IMS__MSG_ID__UNSOL_RESPONSE_CALL_STATE_CHANGED
  IMS_node.update_default_ims_unsol_response_call_state_changed();
  Ims__CallList__Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = IMS__CALL_STATE__CALL_INCOMING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_ismpty = TRUE;
  calls[0]->has_ismt = TRUE;
  calls[0]->ismt = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isvoice = TRUE;
  calls[0]->isvoice = 1;
  calls[0]->has_isvoiceprivacy = TRUE;
  calls[0]->isvoiceprivacy = 0;
  calls[0]->calldetails->has_calldomain = TRUE;
  calls[0]->calldetails->calldomain = IMS__CALL_DOMAIN__CALL_DOMAIN_PS;
  calls[0]->calldetails->has_calltype = TRUE;
  calls[0]->calldetails->calltype = IMS__CALL_TYPE__CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: IMS__MSG_ID__REQUEST_ANSWER
  IMS_node.update_default_ims_request_answer();
  t.inject(IMS_node);

  // Expect: IMS__MSG_ID__REQUEST_ANSWER (resp)
  IMS_node.update_default_ims_request_answer_resp();
  t.expect(IMS_node);

  // Expect: IMS__MSG_ID__UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = IMS__CALL_STATE__CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_ismpty = TRUE;
  calls[0]->has_ismt = TRUE;
  calls[0]->ismt = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isvoice = TRUE;
  calls[0]->isvoice = 1;
  calls[0]->has_isvoiceprivacy = TRUE;
  calls[0]->isvoiceprivacy = 0;
  calls[0]->calldetails->has_calldomain = TRUE;
  calls[0]->calldetails->calldomain = IMS__CALL_DOMAIN__CALL_DOMAIN_PS;
  calls[0]->calldetails->has_calltype = TRUE;
  calls[0]->calldetails->calltype = IMS__CALL_TYPE__CALL_TYPE_VOICE;
  t.expect(IMS_node);


  // Delay to allow call to exist for a moment
  t.sleep(1000);

  // Inject: IMS__MSG_ID__REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND
  IMS_node.update_default_ims_request_hangup_foreground_resume_background();
  t.inject(IMS_node);

  // Expect: IMS__MSG_ID__REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND (resp)
  IMS_node.update_default_ims_request_hangup_foreground_resume_background_resp();
  t.expect(IMS_node);

  // Expect: IMS__MSG_ID__UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(0);
  t.expect(IMS_node);

  // Inject: IMS__MSG_ID__REQUEST_LAST_CALL_FAIL_CAUSE
  IMS_node.update_default_ims_request_last_call_fail_cause();
  t.inject(IMS_node);

  // Expect: IMS__MSG_ID__REQUEST_LAST_CALL_FAIL_CAUSE (resp)
  IMS_node.update_default_ims_request_last_call_fail_cause_resp();
  IMS_node.set_ims_request_last_call_fail_cause_failcause(IMS__CALL_FAIL_CAUSE__CALL_FAIL_NORMAL);
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_target_ims_L_mo_call_001
@description: MO LTE call through IMS
==**/
test_result ril_utf_target_ims_L_mo_call_001(user_specified_attrib_t attr)
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

  // Inject: IMS__MSG_ID__REQUEST_DIAL
  Ims__Dial* ptr = IMS_node.update_default_ims_request_dial("51515");
  t.inject(IMS_node);

  // Expect: IMS__MSG_ID__REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: IMS__MSG_ID__UNSOL_RESPONSE_CALL_STATE_CHANGED
  Ims__CallList__Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = IMS__CALL_STATE__CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_ismpty = TRUE;
  calls[0]->has_ismt = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isvoice = TRUE;
  calls[0]->isvoice = 1;
  calls[0]->has_isvoiceprivacy = TRUE;
  calls[0]->isvoiceprivacy = 0;
  calls[0]->calldetails->has_calldomain = TRUE;
  calls[0]->calldetails->calldomain = IMS__CALL_DOMAIN__CALL_DOMAIN_PS;
  calls[0]->calldetails->has_calltype = TRUE;
  calls[0]->calldetails->calltype = IMS__CALL_TYPE__CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Expect: IMS__MSG_ID__UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = IMS__CALL_STATE__CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_ismpty = TRUE;
  calls[0]->has_ismt = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isvoice = TRUE;
  calls[0]->isvoice = 1;
  calls[0]->has_isvoiceprivacy = TRUE;
  calls[0]->isvoiceprivacy = 0;
  calls[0]->calldetails->has_calldomain = TRUE;
  calls[0]->calldetails->calldomain = IMS__CALL_DOMAIN__CALL_DOMAIN_PS;
  calls[0]->calldetails->has_calltype = TRUE;
  calls[0]->calldetails->calltype = IMS__CALL_TYPE__CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Expect: IMS__MSG_ID__UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = IMS__CALL_STATE__CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_ismpty = TRUE;
  calls[0]->has_ismt = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isvoice = TRUE;
  calls[0]->isvoice = 1;
  calls[0]->has_isvoiceprivacy = TRUE;
  calls[0]->isvoiceprivacy = 0;
  calls[0]->calldetails->has_calldomain = TRUE;
  calls[0]->calldetails->calldomain = IMS__CALL_DOMAIN__CALL_DOMAIN_PS;
  calls[0]->calldetails->has_calltype = TRUE;
  calls[0]->calldetails->calltype = IMS__CALL_TYPE__CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Delay to allow call to exist for a moment
  t.sleep(1000);

  // Inject: IMS__MSG_ID__REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND
  IMS_node.update_default_ims_request_hangup_foreground_resume_background();
  t.inject(IMS_node);

  // Expect: IMS__MSG_ID__REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND (resp)
  IMS_node.update_default_ims_request_hangup_foreground_resume_background_resp();
  t.expect(IMS_node);

  // Expect: IMS__MSG_ID__UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(0);
  t.expect(IMS_node);

  // Inject: IMS__MSG_ID__REQUEST_LAST_CALL_FAIL_CAUSE
  IMS_node.update_default_ims_request_last_call_fail_cause();
  t.inject(IMS_node);

  // Expect: IMS__MSG_ID__REQUEST_LAST_CALL_FAIL_CAUSE (resp)
  IMS_node.update_default_ims_request_last_call_fail_cause_resp();
  IMS_node.set_ims_request_last_call_fail_cause_failcause(IMS__CALL_FAIL_CAUSE__CALL_FAIL_NORMAL);
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_target_ims_L_mo_call_002
@description: MO LTE call through IMS and hold request
==**/
test_result ril_utf_target_ims_L_mo_call_002(user_specified_attrib_t attr)
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

  // Inject: IMS__MSG_ID__REQUEST_DIAL
  Ims__Dial* ptr = IMS_node.update_default_ims_request_dial("51515");
  t.inject(IMS_node);

  // Expect: IMS__MSG_ID__REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: IMS__MSG_ID__UNSOL_RESPONSE_CALL_STATE_CHANGED
  Ims__CallList__Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = IMS__CALL_STATE__CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_ismpty = TRUE;
  calls[0]->has_ismt = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isvoice = TRUE;
  calls[0]->isvoice = 1;
  calls[0]->has_isvoiceprivacy = TRUE;
  calls[0]->isvoiceprivacy = 0;
  calls[0]->calldetails->has_calldomain = TRUE;
  calls[0]->calldetails->calldomain = IMS__CALL_DOMAIN__CALL_DOMAIN_PS;
  calls[0]->calldetails->has_calltype = TRUE;
  calls[0]->calldetails->calltype = IMS__CALL_TYPE__CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Expect: IMS__MSG_ID__UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = IMS__CALL_STATE__CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_ismpty = TRUE;
  calls[0]->has_ismt = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isvoice = TRUE;
  calls[0]->isvoice = 1;
  calls[0]->has_isvoiceprivacy = TRUE;
  calls[0]->isvoiceprivacy = 0;
  calls[0]->calldetails->has_calldomain = TRUE;
  calls[0]->calldetails->calldomain = IMS__CALL_DOMAIN__CALL_DOMAIN_PS;
  calls[0]->calldetails->has_calltype = TRUE;
  calls[0]->calldetails->calltype = IMS__CALL_TYPE__CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Expect: IMS__MSG_ID__UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = IMS__CALL_STATE__CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_ismpty = TRUE;
  calls[0]->has_ismt = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isvoice = TRUE;
  calls[0]->isvoice = 1;
  calls[0]->has_isvoiceprivacy = TRUE;
  calls[0]->isvoiceprivacy = 0;
  calls[0]->calldetails->has_calldomain = TRUE;
  calls[0]->calldetails->calldomain = IMS__CALL_DOMAIN__CALL_DOMAIN_PS;
  calls[0]->calldetails->has_calltype = TRUE;
  calls[0]->calldetails->calltype = IMS__CALL_TYPE__CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Delay to allow call to exist for a moment
  t.sleep(1000);

  // Inject: IMS__MSG_ID__REQUEST_HOLD
  IMS_node.update_default_ims_request_hold(1);
  t.inject(IMS_node);

  // Expect: IMS__MSG_ID__REQUEST_HOLD (resp)
  IMS_node.update_default_ims_request_hold_resp(IMS__ERROR__E_SUCCESS);
  t.expect(IMS_node);

  // Delay to allow call to exist for a moment
  t.sleep(1000);

  // Inject: IMS__MSG_ID__REQUEST_RESUME
  IMS_node.update_default_ims_request_resume(1);
  t.inject(IMS_node);

  // Expect: IMS__MSG_ID__REQUEST_RESUME (resp)
  IMS_node.update_default_ims_request_resume_resp(IMS__ERROR__E_SUCCESS);
  t.expect(IMS_node);

  // Delay to allow call to exist for a moment
  t.sleep(1000);

  // Inject: IMS__MSG_ID__REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND
  IMS_node.update_default_ims_request_hangup_foreground_resume_background();
  t.inject(IMS_node);

  // Expect: IMS__MSG_ID__REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND (resp)
  IMS_node.update_default_ims_request_hangup_foreground_resume_background_resp();
  t.expect(IMS_node);

  // Expect: IMS__MSG_ID__UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(0);
  t.expect(IMS_node);

  // Inject: IMS__MSG_ID__REQUEST_LAST_CALL_FAIL_CAUSE
  IMS_node.update_default_ims_request_last_call_fail_cause();
  t.inject(IMS_node);

  // Expect: IMS__MSG_ID__REQUEST_LAST_CALL_FAIL_CAUSE (resp)
  IMS_node.update_default_ims_request_last_call_fail_cause_resp();
  IMS_node.set_ims_request_last_call_fail_cause_failcause(IMS__CALL_FAIL_CAUSE__CALL_FAIL_NORMAL);
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}
#endif



