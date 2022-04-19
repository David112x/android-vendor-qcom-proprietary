/******************************************************************************
#  Copyright (c) 2014 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_test_qmi_ril_L.cpp
  @brief   RIL UTF test case definitions

  DESCRIPTION
  All test case definitions should be in this file
  ---------------------------------------------------------------------------
******************************************************************************/

#include "ril_utf_test_qmi_ril.h"
#include "ril_utf_test_qmi_ril_L.h"
#include "imsIF.pb.h"


/**==
@testname: ril_utf_ims_mo_call_001_L
@description: MO LTE call through IMS
==**/
test_result ril_utf_ims_mo_call_001_L(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;
  char *extra[2] = {0, 0};
  char extras_codec[] = "Codec=AMR_WB";

  t.set_test_id("ril_utf_ims_mo_call_001_L");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  IMS_node.set_ims_request_dial_isencrypted(FALSE);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  QMI_node.set_qmi_voice_dial_call_req_is_secure_call(FALSE);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CC_IN_PROGRESS_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // Media ID
  voice_media_id_with_id_type_v02 *media_id = QMI_node.set_qmi_voice_all_call_status_ind_media_id(1);
  media_id[0].call_id = 1;
  media_id[0].media_id = 2;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  t.expect(IMS_node);


  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
 // IMS_node.set_expect_skip_verification_status(false); // TODO
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_SPEECH_CODEC_INFO_IND_V02
  QMI_node.update_default_qmi_voice_speech_codec_info_ind_msg();
  QMI_node.set_qmi_voice_speech_codec_info_ind_call_id(1);
  QMI_node.set_qmi_voice_speech_codec_info_ind_network_mode(VOICE_NETWORK_MODE_LTE_V02);
  QMI_node.set_qmi_voice_speech_codec_info_ind_speech_codec(VOICE_SPEECH_CODEC_AMR_WB_V02);
  t.inject(QMI_node);

  // Codec and Child number expected in extras
  extra[0] = extras_codec;

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  calls[0]->callDetails.extras.arg = (void *)extra;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  calls[0]->callDetails.extras.arg = (void *)extra;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(1);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  voice_call_end_reason_type_v02* end = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end[0].call_id = 1;
  end[0].call_end_reason = CALL_END_CAUSE_NORMAL_CALL_CLEARING_V02;
  // End Reason Text "No RTCP Packets" in UTF-16 format
  int i = 0;
  uint16_t text[20] = {0x4e, 0x6f, 0x20, 0x52, 0x54, 0x43, 0x50, 0x20, 0x50, 0x61, 0x63, 0x6b, 0x65, 0x74, 0x73};
  voice_ip_end_reason_text_type_v02* end_reason_text = QMI_node.set_qmi_voice_all_call_status_ind_end_reason_text(1);
  end_reason_text[0].call_id = 1;
  end_reason_text[0].end_reason_text_len = 16;
  for(i=0; i<end_reason_text[0].end_reason_text_len; i++)
    end_reason_text[0].end_reason_text[i] = text[i];
  voice_ip_end_sip_code_type_v02* sip_error_code = QMI_node.set_qmi_voice_all_call_status_ind_sip_error_code(1);
  sip_error_code[0].call_id = 1;
  sip_error_code[0].sip_error_code = 100;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 0;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 2;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  calls[0]->callDetails.extras.arg = (void *)extra;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  calls[0]->failCause.errorDetails.has_sipErrorCode = TRUE;
  calls[0]->failCause.errorDetails.sipErrorCode = 100;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}
/**==
@testname: ril_utf_ims_mt_call_001_L
@description: MT LTE call through IMS
==**/
test_result ril_utf_ims_mt_call_001_L(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;
  char *extra[2] = {0, 0};
  char extras_codec[] = "Codec=AMR_WB";

  t.set_test_id("ril_utf_ims_mt_call_001_L");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_INCOMING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MT_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // Media ID
  voice_media_id_with_id_type_v02 *media_id = QMI_node.set_qmi_voice_all_call_status_ind_media_id(1);
  media_id[0].call_id = 1;
  media_id[0].media_id = 0;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_CALL_RING
  IMS_node.update_default_ims_unsol_call_ring();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_INCOMING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 0;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_ANSWER
  IMS_node.update_default_ims_request_answer();
  t.inject(IMS_node);

  // Expect: QMI_VOICE_ANSWER_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_answer_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_ANSWER_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_answer_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_SPEECH_CODEC_INFO_IND_V02
  QMI_node.update_default_qmi_voice_speech_codec_info_ind_msg();
  QMI_node.set_qmi_voice_speech_codec_info_ind_call_id(1);
  QMI_node.set_qmi_voice_speech_codec_info_ind_network_mode(VOICE_NETWORK_MODE_LTE_V02);
  QMI_node.set_qmi_voice_speech_codec_info_ind_speech_codec(VOICE_SPEECH_CODEC_AMR_WB_V02);
  t.inject(QMI_node);

  // Codec and Child number expected in extras
  extra[0] = extras_codec;

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_INCOMING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 0;
  calls[0]->callDetails.extras.arg = (void *)extra;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MT_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_ANSWER (resp)
  IMS_node.update_default_ims_request_answer_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 0;
  calls[0]->callDetails.extras.arg = (void *)extra;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(1);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MT_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
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

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 0;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  calls[0]->callDetails.extras.arg = (void *)extra;

  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}
/**==
@testname: ril_utf_ims_mt_call_mid_call_update_001_L
@description: MT LTE call through IMS with mid call capability update
==**/
test_result ril_utf_ims_mt_call_mid_call_update_001_L(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;
  char *extra[2] = {0, 0};
  char extras_codec[] = "Codec=AMR_WB";

  t.set_test_id("ril_utf_ims_mt_call_mid_call_update_001_L");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_INCOMING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MT_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // Media ID
  voice_media_id_with_id_type_v02 *media_id = QMI_node.set_qmi_voice_all_call_status_ind_media_id(1);
  media_id[0].call_id = 1;
  media_id[0].media_id = 0;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_CALL_RING
  IMS_node.update_default_ims_unsol_call_ring();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_INCOMING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 0;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_ANSWER
  IMS_node.update_default_ims_request_answer();
  t.inject(IMS_node);

  // Expect: QMI_VOICE_ANSWER_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_answer_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_ANSWER_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_answer_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_SPEECH_CODEC_INFO_IND_V02
  QMI_node.update_default_qmi_voice_speech_codec_info_ind_msg();
  QMI_node.set_qmi_voice_speech_codec_info_ind_call_id(1);
  QMI_node.set_qmi_voice_speech_codec_info_ind_network_mode(VOICE_NETWORK_MODE_LTE_V02);
  QMI_node.set_qmi_voice_speech_codec_info_ind_speech_codec(VOICE_SPEECH_CODEC_AMR_WB_V02);
  t.inject(QMI_node);

  // Codec and Child number expected in extras
  extra[0] = extras_codec;

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_INCOMING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 0;
  calls[0]->callDetails.extras.arg = (void *)extra;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MT_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  QMI_node.set_qmi_voice_all_call_status_ind_audio_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  // local_call_capabilities_info
  voice_ip_call_capabilities_info_type_v02 *local_call_cap = QMI_node.set_qmi_voice_all_call_status_ind_local_call_capabilities_info(1);
  local_call_cap[0].call_id      = 1;
  local_call_cap[0].audio_attrib = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
  local_call_cap[0].audio_cause  = VOICE_RESTRICT_CAUSE_NONE_V02;
  local_call_cap[0].video_attrib =  0x0;
  local_call_cap[0].video_cause  = VOICE_RESTRICT_CAUSE_HD_V02;

  // peer_call_capabilities_info
  voice_ip_call_capabilities_info_type_v02 *peer_call_cap = QMI_node.set_qmi_voice_all_call_status_ind_peer_call_capabilities_info(1);
  peer_call_cap[0].call_id      = 1;
  peer_call_cap[0].audio_attrib = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
  peer_call_cap[0].audio_cause  = VOICE_RESTRICT_CAUSE_NONE_V02;
  peer_call_cap[0].video_attrib =  0x0;
  peer_call_cap[0].video_cause  =  VOICE_RESTRICT_CAUSE_HD_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_ANSWER (resp)
  IMS_node.update_default_ims_request_answer_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 0;
  calls[0]->callDetails.extras.arg = (void *)extra;
  t.expect(IMS_node);

  // Inject QMI_VOICE_MODIFIED_IND_V02 VIDEO OFF
  QMI_node.update_default_qmi_voice_modified_ind_msg(1);
  QMI_node.set_qmi_voice_modified_ind_call_type(CALL_TYPE_VOICE_IP_V02);
  QMI_node.set_qmi_voice_modified_ind_audio_attrib(
      VOICE_CALL_ATTRIB_TX_V02|VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_modified_ind_video_attrib(0);
  QMI_node.set_qmi_voice_modified_ind_call_attrib_status(VOICE_CALL_ATTRIB_STATUS_OK_V02);
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index     = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state     = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa     = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT = TRUE;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->has_als = TRUE;
  calls[0]->als     = 0;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain     = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType     = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.extras.arg = (void *)extra;
  t.expect(IMS_node);


  // Inject QMI_VOICE_MODIFIED_IND_V02 VIDEO ON
  QMI_node.update_default_qmi_voice_modified_ind_msg(1);
  QMI_node.set_qmi_voice_modified_ind_call_type(CALL_TYPE_VOICE_IP_V02);
  QMI_node.set_qmi_voice_modified_ind_local_attrib(1, VOICE_CALL_ATTRIB_TX_V02|VOICE_CALL_ATTRIB_RX_V02
                                           ,VOICE_RESTRICT_CAUSE_NONE_V02,VOICE_CALL_ATTRIB_TX_V02
                                           |VOICE_CALL_ATTRIB_RX_V02, VOICE_RESTRICT_CAUSE_NONE_V02);
  QMI_node.set_qmi_voice_modified_ind_peer_attrib(1, VOICE_CALL_ATTRIB_TX_V02|VOICE_CALL_ATTRIB_RX_V02
                                           ,VOICE_RESTRICT_CAUSE_NONE_V02,VOICE_CALL_ATTRIB_TX_V02
                                           |VOICE_CALL_ATTRIB_RX_V02, VOICE_RESTRICT_CAUSE_NONE_V02);
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index     = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state     = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa     = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT = TRUE;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->has_als = TRUE;
  calls[0]->als     = 0;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain     = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType     = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.extras.arg = (void *)extra;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(1);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MT_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  voice_call_end_reason_type_v02* end=QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end[0].call_id = 1;
  end[0].call_end_reason = CALL_END_CAUSE_NORMAL_CALL_CLEARING_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 0;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  calls[0]->callDetails.extras.arg = (void *)extra;

  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}
/**==
@testname: ril_utf_ims_mo_call_b2b_001_L
@description: back to back MO LTE call through IMS
@ 1st dial on 2428536960 full cycle from DIALING to ACTIVE, then END
@ 2nd dial on 2428536961 full cycle from DIALING to ACTIVE, then END
==**/
test_result ril_utf_ims_mo_call_b2b_001_L(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id("ril_utf_ims_mo_call_b2b_001_L");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }
  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_IP_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(1);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
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

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);


  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  t.expect(IMS_node);


  // Start 2nd dial call
  // Inject: ims_MsgId_REQUEST_DIAL
  ptr = IMS_node.update_default_ims_request_dial("2428536961");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536961");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_IP_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info_2 = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info_2[0].call_id = 1;
  call_info_2[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info_2[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info_2[0].direction = CALL_DIRECTION_MO_V02;
  call_info_2[0].is_mpty = 0;
  call_info_2[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list_2 = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list_2[0].call_id = 1;
  number_list_2[0].number_len = strlen("2428536961");
  strlcpy(number_list_2[0].number, "2428536961", strlen("2428536961")+1);
  number_list_2[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls_2 = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls_2[0]->has_index = TRUE;
  calls_2[0]->index = 1;
  calls_2[0]->has_state = TRUE;
  calls_2[0]->state = ims_CallState_CALL_DIALING;
  calls_2[0]->has_toa = TRUE;
  calls_2[0]->toa = 129;
  calls_2[0]->has_isMpty = TRUE;
  calls_2[0]->has_isMT = TRUE;
  calls_2[0]->has_als = TRUE;
  calls_2[0]->has_isVoice = TRUE;
  calls_2[0]->isVoice = 1;
  calls_2[0]->has_isVoicePrivacy = TRUE;
  calls_2[0]->isVoicePrivacy = 0;
  calls_2[0]->callDetails.has_callDomain = TRUE;
  calls_2[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls_2[0]->callDetails.has_callType = TRUE;
  calls_2[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info_2 = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info_2[0].call_id = 1;
  call_info_2[0].call_state = CALL_STATE_ALERTING_V02;
  call_info_2[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info_2[0].direction = CALL_DIRECTION_MO_V02;
  call_info_2[0].is_mpty = 0;
  call_info_2[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list_2 = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list_2[0].call_id = 1;
  number_list_2[0].number_len = strlen("2428536961");
  strlcpy(number_list_2[0].number, "2428536961", strlen("2428536961")+1);
  number_list_2[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls_2 = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls_2[0]->has_index = TRUE;
  calls_2[0]->index = 1;
  calls_2[0]->has_state = TRUE;
  calls_2[0]->state = ims_CallState_CALL_ALERTING;
  calls_2[0]->has_toa = TRUE;
  calls_2[0]->toa = 129;
  calls_2[0]->has_isMpty = TRUE;
  calls_2[0]->has_isMT = TRUE;
  calls_2[0]->has_als = TRUE;
  calls_2[0]->has_isVoice = TRUE;
  calls_2[0]->isVoice = 1;
  calls_2[0]->has_isVoicePrivacy = TRUE;
  calls_2[0]->isVoicePrivacy = 0;
  calls_2[0]->callDetails.has_callDomain = TRUE;
  calls_2[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls_2[0]->callDetails.has_callType = TRUE;
  calls_2[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info_2 = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info_2[0].call_id = 1;
  call_info_2[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info_2[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info_2[0].direction = CALL_DIRECTION_MO_V02;
  call_info_2[0].is_mpty = 0;
  call_info_2[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list_2 = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list_2[0].call_id = 1;
  number_list_2[0].number_len = strlen("2428536961");
  strlcpy(number_list_2[0].number, "2428536961", strlen("2428536961")+1);
  number_list_2[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls_2 = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls_2[0]->has_index = TRUE;
  calls_2[0]->index = 1;
  calls_2[0]->has_state = TRUE;
  calls_2[0]->state = ims_CallState_CALL_ACTIVE;
  calls_2[0]->has_toa = TRUE;
  calls_2[0]->toa = 129;
  calls_2[0]->has_isMpty = TRUE;
  calls_2[0]->has_isMT = TRUE;
  calls_2[0]->has_als = TRUE;
  calls_2[0]->has_isVoice = TRUE;
  calls_2[0]->isVoice = 1;
  calls_2[0]->has_isVoicePrivacy = TRUE;
  calls_2[0]->isVoicePrivacy = 0;
  calls_2[0]->callDetails.has_callDomain = TRUE;
  calls_2[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls_2[0]->callDetails.has_callType = TRUE;
  calls_2[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(1);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list_2 = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list_2[0].call_id = 1;
  number_list_2[0].number_len = strlen("2428536961");
  strlcpy(number_list_2[0].number, "2428536961", strlen("2428536961")+1);
  number_list_2[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  voice_call_end_reason_type_v02* end_2 = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end_2[0].call_id = 1;
  end_2[0].call_end_reason = CALL_END_CAUSE_NORMAL_CALL_CLEARING_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls_2 = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls_2[0]->has_index = TRUE;
  calls_2[0]->index = 1;
  calls_2[0]->has_state = TRUE;
  calls_2[0]->state = ims_CallState_CALL_END;
  calls_2[0]->has_toa = TRUE;
  calls_2[0]->toa = 129;
  calls_2[0]->has_isMpty = TRUE;
  calls_2[0]->has_isMT = TRUE;
  calls_2[0]->has_als = TRUE;
  calls_2[0]->has_isVoice = TRUE;
  calls_2[0]->isVoice = 1;
  calls_2[0]->has_isVoicePrivacy = TRUE;
  calls_2[0]->isVoicePrivacy = 0;
  calls_2[0]->callDetails.has_callDomain = TRUE;
  calls_2[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls_2[0]->callDetails.has_callType = TRUE;
  calls_2[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls_2[0]->has_failCause = TRUE;
  calls_2[0]->failCause.has_failcause = TRUE;
  calls_2[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_mo_multi_001_L
@description: 2 MO LTE calls through IMS
==**/
test_result ril_utf_ims_mo_multi_001_L(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id("ril_utf_ims_mo_multi_001_L");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_IP_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HOLD
  IMS_node.update_default_ims_request_hold(1);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_MANAGE_IP_CALLS_REQ_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_req_msg((voip_sups_type_enum_v02)VOIP_SUPS_TYPE_CALL_HOLD_V02);
  QMI_node.set_qmi_voice_manage_ip_calls_req_call_id(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_MANAGE_IP_CALLS_RESP_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_HOLD (Resp)
  IMS_node.update_default_ims_request_hold_resp();
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_DIAL
  ptr = IMS_node.update_default_ims_request_dial("2428536961");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536961");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_IP_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(2);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_HOLD_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  call_info[1].call_id = 2;
  call_info[1].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 0;
  call_info[1].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[1].call_id = 2;
  number_list[1].number_len = strlen("2428536961");
  strlcpy(number_list[1].number, "2428536961", strlen("2428536961")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  calls[0]->has_index = TRUE;
  calls[0]->index = 2;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 1;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_HOLDING;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_HOLD_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  call_info[1].call_id = 2;
  call_info[1].call_state = CALL_STATE_ALERTING_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 0;
  call_info[1].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[1].call_id = 2;
  number_list[1].number_len = strlen("2428536961");
  strlcpy(number_list[1].number, "2428536961", strlen("2428536961")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  calls[0]->has_index = TRUE;
  calls[0]->index = 2;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 1;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_HOLDING;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_HOLD_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  call_info[1].call_id = 2;
  call_info[1].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 0;
  call_info[1].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[1].call_id = 2;
  number_list[1].number_len = strlen("2428536961");
  strlcpy(number_list[1].number, "2428536961", strlen("2428536961")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  calls[0]->has_index = TRUE;
  calls[0]->index = 2;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 1;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_HOLDING;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(2);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(2);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(2);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_HOLD_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  call_info[1].call_id = 2;
  call_info[1].call_state = CALL_STATE_END_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 0;
  call_info[1].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[1].call_id = 2;
  number_list[1].number_len = strlen("2428536961");
  strlcpy(number_list[1].number, "2428536961", strlen("2428536961")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  voice_call_end_reason_type_v02* end = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end[0].call_id = 2;
  end[0].call_end_reason = CALL_END_CAUSE_NORMAL_CALL_CLEARING_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  calls[0]->has_index = TRUE;
  calls[0]->index = 2;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  calls[1]->has_index = TRUE;
  calls[1]->index = 1;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_HOLDING;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(1);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  end = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end[0].call_id = 1;
  end[0].call_end_reason = CALL_END_CAUSE_NORMAL_CALL_CLEARING_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_mo_mt_call_001_L
@description: MO call with MT call waiting
==**/
test_result ril_utf_ims_mo_mt_call_001_L(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id("ril_utf_ims_mo_mt_call_001_L");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }
  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_IP_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  call_info[1].call_id = 2;
  call_info[1].call_state = CALL_STATE_WAITING_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MT_V02;
  call_info[1].is_mpty = 0;
  call_info[1].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[1].call_id = 2;
  number_list[1].number_len = strlen("2428536961");
  strlcpy(number_list[1].number, "2428536961", strlen("2428536961")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  calls[0]->has_index = TRUE;
  calls[0]->index = 2;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_WAITING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT = 1;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 1;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_ACTIVE;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_ANSWER
  IMS_node.update_default_ims_request_answer();
  t.inject(IMS_node);

  // Expect: QMI_VOICE_ANSWER_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_answer_call_req_msg(2);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_ANSWER_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_answer_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_HOLD_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  call_info[1].call_id = 2;
  call_info[1].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MT_V02;
  call_info[1].is_mpty = 0;
  call_info[1].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[1].call_id = 2;
  number_list[1].number_len = strlen("2428536961");
  strlcpy(number_list[1].number, "2428536961", strlen("2428536961")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_ANSWER (resp)
  IMS_node.update_default_ims_request_answer_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  calls[0]->has_index = TRUE;
  calls[0]->index = 2;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT = 1;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 1;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_HOLDING;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(2);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(2);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(2);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  call_info[1].call_id = 2;
  call_info[1].call_state = CALL_STATE_END_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MT_V02;
  call_info[1].is_mpty = 0;
  call_info[1].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[1].call_id = 2;
  number_list[1].number_len = strlen("2428536961");
  strlcpy(number_list[1].number, "2428536961", strlen("2428536961")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  voice_call_end_reason_type_v02* end = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end[0].call_id = 2;
  end[0].call_end_reason = CALL_END_CAUSE_NORMAL_CALL_CLEARING_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  calls[0]->has_index = TRUE;
  calls[0]->index = 2;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT = 1;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  calls[1]->has_index = TRUE;
  calls[1]->index = 1;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_ACTIVE;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(1);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  end = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end[0].call_id = 1;
  end[0].call_end_reason = CALL_END_CAUSE_NORMAL_CALL_CLEARING_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}


/**==
@testname: ril_utf_ims_mo_conf_001
@description:
==**/
test_result ril_utf_ims_mo_conf_001_L(user_specified_attrib_t attr)
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

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_IP_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HOLD
  IMS_node.update_default_ims_request_hold(1);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_MANAGE_IP_CALLS_REQ_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_req_msg((voip_sups_type_enum_v02)VOIP_SUPS_TYPE_CALL_HOLD_V02);
  QMI_node.set_qmi_voice_manage_ip_calls_req_call_id(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_MANAGE_IP_CALLS_RESP_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_HOLD (Resp)
  IMS_node.update_default_ims_request_hold_resp();
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* dial = IMS_node.update_default_ims_request_dial("2428536961");
  dial->has_callDetails = true;
  dial->callDetails.has_callDomain = TRUE;
  dial->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  dial->callDetails.has_callType = TRUE;
  dial->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536961");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_IP_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(2);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  call_info[1].call_id = 1;
  call_info[1].call_state = CALL_STATE_HOLD_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 0;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 2;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  number_list[1].call_id = 1;
  number_list[1].number_len = strlen("2428536960");
  strlcpy(number_list[1].number, "2428536960", strlen("2428536960")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 2;
  number_list[0].number_len = strlen("2428536961");
  strlcpy(number_list[0].number, "2428536961", strlen("2428536961")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  calls[0]->has_index = TRUE;
  calls[0]->index = 2;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 1;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_HOLDING;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  call_info[1].call_id = 1;
  call_info[1].call_state = CALL_STATE_HOLD_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 0;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 2;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  number_list[1].call_id = 1;
  number_list[1].number_len = strlen("2428536960");
  strlcpy(number_list[1].number, "2428536960", strlen("2428536960")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 2;
  number_list[0].number_len = strlen("2428536961");
  strlcpy(number_list[0].number, "2428536961", strlen("2428536961")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  calls[0]->has_index = TRUE;
  calls[0]->index = 2;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 1;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_HOLDING;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  call_info[1].call_id = 1;
  call_info[1].call_state = CALL_STATE_HOLD_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 0;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 2;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  number_list[1].call_id = 1;
  number_list[1].number_len = strlen("2428536960");
  strlcpy(number_list[1].number, "2428536960", strlen("2428536960")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 2;
  number_list[0].number_len = strlen("2428536961");
  strlcpy(number_list[0].number, "2428536961", strlen("2428536961")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  calls[0]->has_index = TRUE;
  calls[0]->index = 2;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 1;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_HOLDING;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_CONFERENCE
  IMS_node.update_default_ims_request_conference();
  t.inject(IMS_node);

  // Expect: QMI_VOICE_MANAGE_IP_CALLS_REQ_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_req_msg(VOIP_SUPS_TYPE_MAKE_CONFERENCE_CALL_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(3);
  call_info[2].call_id = 1;
  call_info[2].call_state = CALL_STATE_HOLD_V02;
  call_info[2].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[2].direction = CALL_DIRECTION_MO_V02;
  call_info[2].is_mpty = 0;
  call_info[2].mode = CALL_MODE_LTE_V02;
  call_info[1].call_id = 2;
  call_info[1].call_state = CALL_STATE_HOLD_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 0;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 3;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 1;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(3);
  number_list[2].call_id = 1;
  number_list[2].number_len = strlen("2428536960");
  strlcpy(number_list[2].number, "2428536960", strlen("2428536960")+1);
  number_list[2].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[1].call_id = 2;
  number_list[1].number_len = strlen("2428536961");
  strlcpy(number_list[1].number, "2428536961", strlen("2428536961")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 3;
  number_list[0].number_len = strlen("n_way_voice");
  strlcpy(number_list[0].number, "n_way_voice", strlen("n_way_voice")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(3);
  calls[0]->has_index = TRUE;
  calls[0]->index = 3;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->isMpty = 1;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 2;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_HOLDING;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[2]->has_index = TRUE;
  calls[2]->index = 1;
  calls[2]->has_state = TRUE;
  calls[2]->state = ims_CallState_CALL_HOLDING;
  calls[2]->has_toa = TRUE;
  calls[2]->toa = 129;
  calls[2]->has_isMpty = TRUE;
  calls[2]->has_isMT = TRUE;
  calls[2]->has_als = TRUE;
  calls[2]->has_isVoice = TRUE;
  calls[2]->isVoice     = 1;
  calls[2]->has_isVoicePrivacy = TRUE;
  calls[2]->isVoicePrivacy     = 0;
  calls[2]->has_numberPresentation = TRUE;
  calls[2]->numberPresentation     = 1;
  calls[2]->has_namePresentation = TRUE;
  calls[2]->namePresentation     = 0;
  calls[2]->callDetails.has_callDomain = TRUE;
  calls[2]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[2]->callDetails.has_callType = TRUE;
  calls[2]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(3);
  call_info[2].call_id = 1;
  call_info[2].call_state = CALL_STATE_HOLD_V02;
  call_info[2].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[2].direction = CALL_DIRECTION_MO_V02;
  call_info[2].is_mpty = 0;
  call_info[2].mode = CALL_MODE_LTE_V02;
  call_info[1].call_id = 2;
  call_info[1].call_state = CALL_STATE_HOLD_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 0;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 3;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 1;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(3);
  number_list[2].call_id = 1;
  number_list[2].number_len = strlen("2428536960");
  strlcpy(number_list[2].number, "2428536960", strlen("2428536960")+1);
  number_list[2].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[1].call_id = 2;
  number_list[1].number_len = strlen("2428536961");
  strlcpy(number_list[1].number, "2428536961", strlen("2428536961")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 3;
  number_list[0].number_len = strlen("n_way_voice");
  strlcpy(number_list[0].number, "n_way_voice", strlen("n_way_voice")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(3);
  calls[0]->has_index = TRUE;
  calls[0]->index = 3;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->isMpty = 1;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 2;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_HOLDING;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[2]->has_index = TRUE;
  calls[2]->index = 1;
  calls[2]->has_state = TRUE;
  calls[2]->state = ims_CallState_CALL_HOLDING;
  calls[2]->has_toa = TRUE;
  calls[2]->toa = 129;
  calls[2]->has_isMpty = TRUE;
  calls[2]->has_isMT = TRUE;
  calls[2]->has_als = TRUE;
  calls[2]->has_isVoice = TRUE;
  calls[2]->isVoice     = 1;
  calls[2]->has_isVoicePrivacy = TRUE;
  calls[2]->isVoicePrivacy     = 0;
  calls[2]->has_numberPresentation = TRUE;
  calls[2]->numberPresentation     = 1;
  calls[2]->has_namePresentation = TRUE;
  calls[2]->namePresentation     = 0;
  calls[2]->callDetails.has_callDomain = TRUE;
  calls[2]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[2]->callDetails.has_callType = TRUE;
  calls[2]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(3);
  call_info[2].call_id = 1;
  call_info[2].call_state = CALL_STATE_HOLD_V02;
  call_info[2].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[2].direction = CALL_DIRECTION_MO_V02;
  call_info[2].is_mpty = 0;
  call_info[2].mode = CALL_MODE_LTE_V02;
  call_info[1].call_id = 2;
  call_info[1].call_state = CALL_STATE_HOLD_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 0;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 3;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 1;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(3);
  number_list[2].call_id = 1;
  number_list[2].number_len = strlen("2428536960");
  strlcpy(number_list[2].number, "2428536960", strlen("2428536960")+1);
  number_list[2].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[1].call_id = 2;
  number_list[1].number_len = strlen("2428536961");
  strlcpy(number_list[1].number, "2428536961", strlen("2428536961")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 3;
  number_list[0].number_len = strlen("n_way_voice");
  strlcpy(number_list[0].number, "n_way_voice", strlen("n_way_voice")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(3);
  calls[0]->has_index = TRUE;
  calls[0]->index = 3;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->isMpty = 1;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 2;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_HOLDING;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[2]->has_index = TRUE;
  calls[2]->index = 1;
  calls[2]->has_state = TRUE;
  calls[2]->state = ims_CallState_CALL_HOLDING;
  calls[2]->has_toa = TRUE;
  calls[2]->toa = 129;
  calls[2]->has_isMpty = TRUE;
  calls[2]->has_isMT = TRUE;
  calls[2]->has_als = TRUE;
  calls[2]->has_isVoice = TRUE;
  calls[2]->isVoice     = 1;
  calls[2]->has_isVoicePrivacy = TRUE;
  calls[2]->isVoicePrivacy     = 0;
  calls[2]->has_numberPresentation = TRUE;
  calls[2]->numberPresentation     = 1;
  calls[2]->has_namePresentation = TRUE;
  calls[2]->namePresentation     = 0;
  calls[2]->callDetails.has_callDomain = TRUE;
  calls[2]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[2]->callDetails.has_callType = TRUE;
  calls[2]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(3);
  call_info[2].call_id = 1;
  call_info[2].call_state = CALL_STATE_END_V02;
  call_info[2].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[2].direction = CALL_DIRECTION_MO_V02;
  call_info[2].is_mpty = 0;
  call_info[2].mode = CALL_MODE_LTE_V02;
  call_info[1].call_id = 2;
  call_info[1].call_state = CALL_STATE_HOLD_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 0;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 3;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 1;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(3);
  number_list[2].call_id = 1;
  number_list[2].number_len = strlen("2428536960");
  strlcpy(number_list[2].number, "2428536960", strlen("2428536960")+1);
  number_list[2].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[1].call_id = 2;
  number_list[1].number_len = strlen("2428536961");
  strlcpy(number_list[1].number, "2428536961", strlen("2428536961")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 3;
  number_list[0].number_len = strlen("n_way_voice");
  strlcpy(number_list[0].number, "n_way_voice", strlen("n_way_voice")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(3);
  calls[0]->has_index = TRUE;
  calls[0]->index = 3;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->isMpty = 1;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 2;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_HOLDING;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[2]->has_index = TRUE;
  calls[2]->index = 1;
  calls[2]->has_state = TRUE;
  calls[2]->state = ims_CallState_CALL_END;
  calls[2]->has_toa = TRUE;
  calls[2]->toa = 129;
  calls[2]->has_isMpty = TRUE;
  calls[2]->has_isMT = TRUE;
  calls[2]->has_als = TRUE;
  calls[2]->has_isVoice = TRUE;
  calls[2]->isVoice     = 1;
  calls[2]->has_isVoicePrivacy = TRUE;
  calls[2]->isVoicePrivacy     = 0;
  calls[2]->has_numberPresentation = TRUE;
  calls[2]->numberPresentation     = 1;
  calls[2]->has_namePresentation = TRUE;
  calls[2]->namePresentation     = 0;
  calls[2]->callDetails.has_callDomain = TRUE;
  calls[2]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[2]->callDetails.has_callType = TRUE;
  calls[2]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  call_info[1].call_id = 2;
  call_info[1].call_state = CALL_STATE_END_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 0;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 3;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 1;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  number_list[1].call_id = 2;
  number_list[1].number_len = strlen("2428536961");
  strlcpy(number_list[1].number, "2428536961", strlen("2428536961")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 3;
  number_list[0].number_len = strlen("n_way_voice");
  strlcpy(number_list[0].number, "n_way_voice", strlen("n_way_voice")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  calls[0]->has_index = TRUE;
  calls[0]->index = 3;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->isMpty = 1;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 2;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_END;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_MANAGE_IP_CALLS_RESP_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_CONFERENCE (resp)
  IMS_node.update_default_ims_request_conference_resp();
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HOLD
  IMS_node.update_default_ims_request_hold(3);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_MANAGE_IP_CALLS_REQ_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_req_msg((voip_sups_type_enum_v02)VOIP_SUPS_TYPE_CALL_HOLD_V02);
  QMI_node.set_qmi_voice_manage_ip_calls_req_call_id(3);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_MANAGE_IP_CALLS_RESP_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_HOLD (Resp)
  IMS_node.update_default_ims_request_hold_resp();
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_DIAL
  dial = IMS_node.update_default_ims_request_dial("2428536962");
  dial->has_callDetails = true;
  dial->callDetails.has_callDomain = TRUE;
  dial->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  dial->callDetails.has_callType = TRUE;
  dial->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536962");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_IP_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  call_info[1].call_id = 3;
  call_info[1].call_state = CALL_STATE_HOLD_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 1;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CC_IN_PROGRESS_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = (call_mode_enum_v02)6;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  number_list[1].call_id = 3;
  number_list[1].number_len = strlen("n_way_voice");
  strlcpy(number_list[1].number, "n_way_voice", strlen("n_way_voice")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536962");
  strlcpy(number_list[0].number, "2428536962", strlen("2428536962")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 3;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_HOLDING;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->isMpty = 1;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  call_info[1].call_id = 3;
  call_info[1].call_state = CALL_STATE_HOLD_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 1;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  number_list[1].call_id = 3;
  number_list[1].number_len = strlen("n_way_voice");
  strlcpy(number_list[1].number, "n_way_voice", strlen("n_way_voice")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536962");
  strlcpy(number_list[0].number, "2428536962", strlen("2428536962")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 3;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_HOLDING;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->isMpty = 1;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  call_info[1].call_id = 3;
  call_info[1].call_state = CALL_STATE_HOLD_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 1;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  number_list[1].call_id = 3;
  number_list[1].number_len = strlen("n_way_voice");
  strlcpy(number_list[1].number, "n_way_voice", strlen("n_way_voice")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536962");
  strlcpy(number_list[0].number, "2428536962", strlen("2428536962")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 3;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_HOLDING;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->isMpty = 1;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  call_info[1].call_id = 3;
  call_info[1].call_state = CALL_STATE_HOLD_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 1;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  number_list[1].call_id = 3;
  number_list[1].number_len = strlen("n_way_voice");
  strlcpy(number_list[1].number, "n_way_voice", strlen("n_way_voice")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536962");
  strlcpy(number_list[0].number, "2428536962", strlen("2428536962")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 3;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_HOLDING;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->isMpty = 1;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_CONFERENCE
  IMS_node.update_default_ims_request_conference();
  t.inject(IMS_node);

  // Expect: QMI_VOICE_MANAGE_IP_CALLS_REQ_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_req_msg(VOIP_SUPS_TYPE_MAKE_CONFERENCE_CALL_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  call_info[1].call_id = 1;
  call_info[1].call_state = CALL_STATE_HOLD_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 0;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 3;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 1;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  number_list[1].call_id = 1;
  number_list[1].number_len = strlen("2428536962");
  strlcpy(number_list[1].number, "2428536962", strlen("2428536962")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 3;
  number_list[0].number_len = strlen("n_way_voice");
  strlcpy(number_list[0].number, "n_way_voice", strlen("n_way_voice")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_HOLDING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 3;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_ACTIVE;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->isMpty = 1;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(3);
  call_info[1].call_id = 1;
  call_info[1].call_state = CALL_STATE_END_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 0;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 3;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 1;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(3);
  number_list[1].call_id = 1;
  number_list[1].number_len = strlen("2428536962");
  strlcpy(number_list[1].number, "2428536962", strlen("2428536962")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 3;
  number_list[0].number_len = strlen("n_way_voice");
  strlcpy(number_list[0].number, "n_way_voice", strlen("n_way_voice")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(3);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 3;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_ACTIVE;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->isMpty = 1;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_MANAGE_IP_CALLS_RESP_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_CONFERENCE (resp)
  IMS_node.update_default_ims_request_conference_resp();
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(3);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(3);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(3);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 3;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 1;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 3;
  number_list[0].number_len = strlen("n_way_voice");
  strlcpy(number_list[0].number, "n_way_voice", strlen("n_way_voice")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  voice_call_end_reason_type_v02* end = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end[0].call_id = 1;
  end[0].call_end_reason = CALL_END_CAUSE_NORMAL_CALL_CLEARING_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 3;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->has_failCause = true;
  calls[0]->failCause.has_failcause = true;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_mo_conf_002_L
@description: MO LTE call through IMS that is conferenced 1:1
==**/
test_result ril_utf_ims_mo_conf_002_L(user_specified_attrib_t attr)
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

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_ADD_PARTICIPANT
  IMS_node.update_default_ims_request_add_participant("2428536960");
  t.inject(IMS_node);

  // Expect: QMI_VOICE_MANAGE_IP_CALLS_REQ_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_req_msg((voip_sups_type_enum_v02)0x0a);
  QMI_node.set_qmi_voice_manage_ip_calls_req_call_id(1);
  char *sip_uri = QMI_node.set_qmi_voice_manage_ip_calls_sip_uri();
  strlcpy(sip_uri, "2428536960", strlen("2428536960")+1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_MANAGE_IP_CALLS_RESP_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_ADD_PARTICIPANT
  IMS_node.update_default_ims_request_add_participant_resp();
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 1;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(1);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  voice_call_end_reason_type_v02* end_reason = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end_reason[0].call_end_reason = CALL_END_CAUSE_NETWORK_END_V02;
  end_reason[0].call_id = 1;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->isMpty = FALSE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_mo_conf_003_L
@description: test attempts 1:1 conference by adding participant
   with no active call
//TODO: Dheeraj correct me plox
==**/
test_result ril_utf_ims_mo_conf_003_L(user_specified_attrib_t attr)
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

  // Inject: ims_MsgId_REQUEST_ADD_PARTICIPANT
  IMS_node.update_default_ims_request_add_participant("2428536960");
  t.inject(IMS_node);

  // Expect: ims_MsgId_REQUEST_ADD_PARTICIPANT
  IMS_node.update_default_ims_request_add_participant_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);


  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_mo_conf_004_L
@description: test attempts 1:1 conference with add participant
  with incorrect call id
==**/
test_result ril_utf_ims_mo_conf_004_L(user_specified_attrib_t attr)
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

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_ADD_PARTICIPANT
  IMS_node.update_default_ims_request_add_participant("3038536960");
  t.inject(IMS_node);

  // Expect: QMI_VOICE_MANAGE_IP_CALLS_REQ_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_req_msg((voip_sups_type_enum_v02)0x0a);
  QMI_node.set_qmi_voice_manage_ip_calls_req_call_id(1);
  char *sip_uri = QMI_node.set_qmi_voice_manage_ip_calls_sip_uri();
  strlcpy(sip_uri, "3038536960", strlen("3038536960")+1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_MANAGE_IP_CALLS_RESP_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_resp_msg(QMI_RESULT_FAILURE_V01, QMI_ERR_GENERAL_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_ADD_PARTICIPANT
  IMS_node.update_default_ims_request_add_participant_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_ADD_PARTICIPANT
  IMS_node.update_default_ims_request_add_participant("2428536961");
  t.inject(IMS_node);

  // Expect: QMI_VOICE_MANAGE_IP_CALLS_REQ_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_req_msg((voip_sups_type_enum_v02)0x0a);
  QMI_node.set_qmi_voice_manage_ip_calls_req_call_id(1);
  sip_uri = QMI_node.set_qmi_voice_manage_ip_calls_sip_uri();
  strlcpy(sip_uri, "2428536961", strlen("2428536961")+1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_MANAGE_IP_CALLS_RESP_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_ADD_PARTICIPANT
  IMS_node.update_default_ims_request_add_participant_resp();
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 1;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(1);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  voice_call_end_reason_type_v02* end_reason = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end_reason[0].call_end_reason = CALL_END_CAUSE_NETWORK_END_V02;
  end_reason[0].call_id = 1;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->isMpty = FALSE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_add_participant
@description:
==**/
test_result ril_utf_ims_add_participant(user_specified_attrib_t attr)
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

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_ADD_PARTICIPANT
  IMS_node.update_default_ims_request_add_participant("3333-33");
  t.inject(IMS_node);

  // Expect: QMI_VOICE_MANAGE_IP_CALLS_REQ_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_req_msg((voip_sups_type_enum_v02)0x0a);
  QMI_node.set_qmi_voice_manage_ip_calls_req_call_id(1);
  char *sip_uri = QMI_node.set_qmi_voice_manage_ip_calls_sip_uri();
  strlcpy(sip_uri, "3333-33", strlen("3333-33")+1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_MANAGE_IP_CALLS_RESP_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_ADD_PARTICIPANT
  IMS_node.update_default_ims_request_add_participant_resp();
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 1;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HOLD
  IMS_node.update_default_ims_request_hold(1);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_MANAGE_IP_CALLS_REQ_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_req_msg((voip_sups_type_enum_v02)VOIP_SUPS_TYPE_CALL_HOLD_V02);
  QMI_node.set_qmi_voice_manage_ip_calls_req_call_id(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_MANAGE_IP_CALLS_RESP_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_HOLD (Resp)
  IMS_node.update_default_ims_request_hold_resp();
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_HOLD_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 1;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_HOLDING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_ADD_PARTICIPANT
  IMS_node.update_default_ims_request_add_participant("4444-444");
  t.inject(IMS_node);

  // Expect: QMI_VOICE_MANAGE_IP_CALLS_REQ_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_req_msg((voip_sups_type_enum_v02)0x0a);
  QMI_node.set_qmi_voice_manage_ip_calls_req_call_id(1);
  sip_uri = QMI_node.set_qmi_voice_manage_ip_calls_sip_uri();
  strlcpy(sip_uri, "4444-444", strlen("4444-444")+1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_MANAGE_IP_CALLS_RESP_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_ADD_PARTICIPANT
  IMS_node.update_default_ims_request_add_participant_resp();
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_DIAL
  ptr = IMS_node.update_default_ims_request_dial("2428536961");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536961");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(2);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  call_info[1].call_id = 1;
  call_info[1].call_state = CALL_STATE_HOLD_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 1;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 2;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  number_list[1].call_id = 1;
  number_list[1].number_len = strlen("2428536960");
  strlcpy(number_list[1].number, "2428536960", strlen("2428536960")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 2;
  number_list[0].number_len = strlen("2428536961");
  strlcpy(number_list[0].number, "2428536961", strlen("2428536961")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  calls[0]->has_index = TRUE;
  calls[0]->index = 2;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 1;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_HOLDING;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->isMpty = TRUE;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  call_info[1].call_id = 1;
  call_info[1].call_state = CALL_STATE_HOLD_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 1;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 2;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  number_list[1].call_id = 1;
  number_list[1].number_len = strlen("2428536960");
  strlcpy(number_list[1].number, "2428536960", strlen("2428536960")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 2;
  number_list[0].number_len = strlen("2428536961");
  strlcpy(number_list[0].number, "2428536961", strlen("2428536961")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  calls[0]->has_index = TRUE;
  calls[0]->index = 2;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 1;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_HOLDING;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->isMpty = TRUE;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  call_info[1].call_id = 1;
  call_info[1].call_state = CALL_STATE_HOLD_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 1;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 2;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  number_list[1].call_id = 1;
  number_list[1].number_len = strlen("2428536960");
  strlcpy(number_list[1].number, "2428536960", strlen("2428536960")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 2;
  number_list[0].number_len = strlen("2428536961");
  strlcpy(number_list[0].number, "2428536961", strlen("2428536961")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  calls[0]->has_index = TRUE;
  calls[0]->index = 2;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 1;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_HOLDING;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->isMpty = TRUE;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_ADD_PARTICIPANT
  IMS_node.update_default_ims_request_add_participant("2428536961");
  t.inject(IMS_node);

  // Expect: QMI_VOICE_MANAGE_IP_CALLS_REQ_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_req_msg((voip_sups_type_enum_v02)0x0a);
  QMI_node.set_qmi_voice_manage_ip_calls_req_call_id(2);
  sip_uri = QMI_node.set_qmi_voice_manage_ip_calls_sip_uri();
  strlcpy(sip_uri, "2428536961", strlen("2428536961")+1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_MANAGE_IP_CALLS_RESP_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_ADD_PARTICIPANT
  IMS_node.update_default_ims_request_add_participant_resp();
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  call_info[1].call_id = 1;
  call_info[1].call_state = CALL_STATE_HOLD_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 1;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 2;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 1;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  number_list[1].call_id = 1;
  number_list[1].number_len = strlen("2428536960");
  strlcpy(number_list[1].number, "2428536960", strlen("2428536960")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 2;
  number_list[0].number_len = strlen("2428536961");
  strlcpy(number_list[0].number, "2428536961", strlen("2428536961")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  calls[0]->has_index = TRUE;
  calls[0]->index = 2;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 1;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_HOLDING;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->isMpty = TRUE;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;

  t.expect(IMS_node);
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_mo_conf_005_L
@description:
==**/
test_result ril_utf_ims_mo_conf_005_L(user_specified_attrib_t attr)
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

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_IP_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HOLD
  IMS_node.update_default_ims_request_hold(1);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_MANAGE_IP_CALLS_REQ_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_req_msg((voip_sups_type_enum_v02)VOIP_SUPS_TYPE_CALL_HOLD_V02);
  QMI_node.set_qmi_voice_manage_ip_calls_req_call_id(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_MANAGE_IP_CALLS_RESP_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_HOLD (Resp)
  IMS_node.update_default_ims_request_hold_resp();
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* dial = IMS_node.update_default_ims_request_dial("2428536961");
  dial->has_callDetails = true;
  dial->callDetails.has_callDomain = TRUE;
  dial->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  dial->callDetails.has_callType = TRUE;
  dial->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536961");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_IP_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(2);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  call_info[1].call_id = 1;
  call_info[1].call_state = CALL_STATE_HOLD_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 0;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 2;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  number_list[1].call_id = 1;
  number_list[1].number_len = strlen("2428536960");
  strlcpy(number_list[1].number, "2428536960", strlen("2428536960")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 2;
  number_list[0].number_len = strlen("2428536961");
  strlcpy(number_list[0].number, "2428536961", strlen("2428536961")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  calls[0]->has_index = TRUE;
  calls[0]->index = 2;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 1;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_HOLDING;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  call_info[1].call_id = 1;
  call_info[1].call_state = CALL_STATE_HOLD_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 0;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 2;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  number_list[1].call_id = 1;
  number_list[1].number_len = strlen("2428536960");
  strlcpy(number_list[1].number, "2428536960", strlen("2428536960")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 2;
  number_list[0].number_len = strlen("2428536961");
  strlcpy(number_list[0].number, "2428536961", strlen("2428536961")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  calls[0]->has_index = TRUE;
  calls[0]->index = 2;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 1;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_HOLDING;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  call_info[1].call_id = 1;
  call_info[1].call_state = CALL_STATE_HOLD_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 0;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 2;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  number_list[1].call_id = 1;
  number_list[1].number_len = strlen("2428536960");
  strlcpy(number_list[1].number, "2428536960", strlen("2428536960")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 2;
  number_list[0].number_len = strlen("2428536961");
  strlcpy(number_list[0].number, "2428536961", strlen("2428536961")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  calls[0]->has_index = TRUE;
  calls[0]->index = 2;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 1;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_HOLDING;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_CONFERENCE
  IMS_node.update_default_ims_request_conference();
  t.inject(IMS_node);

  // Expect: QMI_VOICE_MANAGE_IP_CALLS_REQ_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_req_msg(VOIP_SUPS_TYPE_MAKE_CONFERENCE_CALL_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(3);
  call_info[2].call_id = 1;
  call_info[2].call_state = CALL_STATE_HOLD_V02;
  call_info[2].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[2].direction = CALL_DIRECTION_MO_V02;
  call_info[2].is_mpty = 0;
  call_info[2].mode = CALL_MODE_LTE_V02;
  call_info[1].call_id = 2;
  call_info[1].call_state = CALL_STATE_HOLD_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 0;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 3;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 1;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(3);
  number_list[2].call_id = 1;
  number_list[2].number_len = strlen("2428536960");
  strlcpy(number_list[2].number, "2428536960", strlen("2428536960")+1);
  number_list[2].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[1].call_id = 2;
  number_list[1].number_len = strlen("2428536961");
  strlcpy(number_list[1].number, "2428536961", strlen("2428536961")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 3;
  number_list[0].number_len = strlen("n_way_voice");
  strlcpy(number_list[0].number, "n_way_voice", strlen("n_way_voice")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(3);
  calls[0]->has_index = TRUE;
  calls[0]->index = 3;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->isMpty = 1;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 2;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_HOLDING;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[2]->has_index = TRUE;
  calls[2]->index = 1;
  calls[2]->has_state = TRUE;
  calls[2]->state = ims_CallState_CALL_HOLDING;
  calls[2]->has_toa = TRUE;
  calls[2]->toa = 129;
  calls[2]->has_isMpty = TRUE;
  calls[2]->has_isMT = TRUE;
  calls[2]->has_als = TRUE;
  calls[2]->has_isVoice = TRUE;
  calls[2]->isVoice     = 1;
  calls[2]->has_isVoicePrivacy = TRUE;
  calls[2]->isVoicePrivacy     = 0;
  calls[2]->has_numberPresentation = TRUE;
  calls[2]->numberPresentation     = 1;
  calls[2]->has_namePresentation = TRUE;
  calls[2]->namePresentation     = 0;
  calls[2]->callDetails.has_callDomain = TRUE;
  calls[2]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[2]->callDetails.has_callType = TRUE;
  calls[2]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(3);
  call_info[2].call_id = 1;
  call_info[2].call_state = CALL_STATE_HOLD_V02;
  call_info[2].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[2].direction = CALL_DIRECTION_MO_V02;
  call_info[2].is_mpty = 0;
  call_info[2].mode = CALL_MODE_LTE_V02;
  call_info[1].call_id = 2;
  call_info[1].call_state = CALL_STATE_HOLD_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 0;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 3;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 1;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(3);
  number_list[2].call_id = 1;
  number_list[2].number_len = strlen("2428536960");
  strlcpy(number_list[2].number, "2428536960", strlen("2428536960")+1);
  number_list[2].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[1].call_id = 2;
  number_list[1].number_len = strlen("2428536961");
  strlcpy(number_list[1].number, "2428536961", strlen("2428536961")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 3;
  number_list[0].number_len = strlen("n_way_voice");
  strlcpy(number_list[0].number, "n_way_voice", strlen("n_way_voice")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(3);
  calls[0]->has_index = TRUE;
  calls[0]->index = 3;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->isMpty = 1;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 2;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_HOLDING;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[2]->has_index = TRUE;
  calls[2]->index = 1;
  calls[2]->has_state = TRUE;
  calls[2]->state = ims_CallState_CALL_HOLDING;
  calls[2]->has_toa = TRUE;
  calls[2]->toa = 129;
  calls[2]->has_isMpty = TRUE;
  calls[2]->has_isMT = TRUE;
  calls[2]->has_als = TRUE;
  calls[2]->has_isVoice = TRUE;
  calls[2]->isVoice     = 1;
  calls[2]->has_isVoicePrivacy = TRUE;
  calls[2]->isVoicePrivacy     = 0;
  calls[2]->has_numberPresentation = TRUE;
  calls[2]->numberPresentation     = 1;
  calls[2]->has_namePresentation = TRUE;
  calls[2]->namePresentation     = 0;
  calls[2]->callDetails.has_callDomain = TRUE;
  calls[2]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[2]->callDetails.has_callType = TRUE;
  calls[2]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(3);
  call_info[2].call_id = 1;
  call_info[2].call_state = CALL_STATE_HOLD_V02;
  call_info[2].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[2].direction = CALL_DIRECTION_MO_V02;
  call_info[2].is_mpty = 0;
  call_info[2].mode = CALL_MODE_LTE_V02;
  call_info[1].call_id = 2;
  call_info[1].call_state = CALL_STATE_HOLD_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 0;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 3;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 1;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(3);
  number_list[2].call_id = 1;
  number_list[2].number_len = strlen("2428536960");
  strlcpy(number_list[2].number, "2428536960", strlen("2428536960")+1);
  number_list[2].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[1].call_id = 2;
  number_list[1].number_len = strlen("2428536961");
  strlcpy(number_list[1].number, "2428536961", strlen("2428536961")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 3;
  number_list[0].number_len = strlen("n_way_voice");
  strlcpy(number_list[0].number, "n_way_voice", strlen("n_way_voice")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(3);
  calls[0]->has_index = TRUE;
  calls[0]->index = 3;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->isMpty = 1;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 2;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_HOLDING;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[2]->has_index = TRUE;
  calls[2]->index = 1;
  calls[2]->has_state = TRUE;
  calls[2]->state = ims_CallState_CALL_HOLDING;
  calls[2]->has_toa = TRUE;
  calls[2]->toa = 129;
  calls[2]->has_isMpty = TRUE;
  calls[2]->has_isMT = TRUE;
  calls[2]->has_als = TRUE;
  calls[2]->has_isVoice = TRUE;
  calls[2]->isVoice     = 1;
  calls[2]->has_isVoicePrivacy = TRUE;
  calls[2]->isVoicePrivacy     = 0;
  calls[2]->has_numberPresentation = TRUE;
  calls[2]->numberPresentation     = 1;
  calls[2]->has_namePresentation = TRUE;
  calls[2]->namePresentation     = 0;
  calls[2]->callDetails.has_callDomain = TRUE;
  calls[2]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[2]->callDetails.has_callType = TRUE;
  calls[2]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(3);
  call_info[2].call_id = 1;
  call_info[2].call_state = CALL_STATE_END_V02;
  call_info[2].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[2].direction = CALL_DIRECTION_MO_V02;
  call_info[2].is_mpty = 0;
  call_info[2].mode = CALL_MODE_LTE_V02;
  call_info[1].call_id = 2;
  call_info[1].call_state = CALL_STATE_HOLD_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 0;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 3;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 1;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(3);
  number_list[2].call_id = 1;
  number_list[2].number_len = strlen("2428536960");
  strlcpy(number_list[2].number, "2428536960", strlen("2428536960")+1);
  number_list[2].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[1].call_id = 2;
  number_list[1].number_len = strlen("2428536961");
  strlcpy(number_list[1].number, "2428536961", strlen("2428536961")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 3;
  number_list[0].number_len = strlen("n_way_voice");
  strlcpy(number_list[0].number, "n_way_voice", strlen("n_way_voice")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(3);
  calls[0]->has_index = TRUE;
  calls[0]->index = 3;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->isMpty = 1;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 2;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_HOLDING;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[2]->has_index = TRUE;
  calls[2]->index = 1;
  calls[2]->has_state = TRUE;
  calls[2]->state = ims_CallState_CALL_END;
  calls[2]->has_toa = TRUE;
  calls[2]->toa = 129;
  calls[2]->has_isMpty = TRUE;
  calls[2]->has_isMT = TRUE;
  calls[2]->has_als = TRUE;
  calls[2]->has_isVoice = TRUE;
  calls[2]->isVoice     = 1;
  calls[2]->has_isVoicePrivacy = TRUE;
  calls[2]->isVoicePrivacy     = 0;
  calls[2]->has_numberPresentation = TRUE;
  calls[2]->numberPresentation     = 1;
  calls[2]->has_namePresentation = TRUE;
  calls[2]->namePresentation     = 0;
  calls[2]->callDetails.has_callDomain = TRUE;
  calls[2]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[2]->callDetails.has_callType = TRUE;
  calls[2]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  call_info[1].call_id = 2;
  call_info[1].call_state = CALL_STATE_END_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 0;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 3;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 1;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  number_list[1].call_id = 2;
  number_list[1].number_len = strlen("2428536961");
  strlcpy(number_list[1].number, "2428536961", strlen("2428536961")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 3;
  number_list[0].number_len = strlen("n_way_voice");
  strlcpy(number_list[0].number, "n_way_voice", strlen("n_way_voice")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  calls[0]->has_index = TRUE;
  calls[0]->index = 3;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->isMpty = 1;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 2;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_END;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_MANAGE_IP_CALLS_RESP_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_CONFERENCE (resp)
  IMS_node.update_default_ims_request_conference_resp();
  t.expect(IMS_node);

  // ADD MT call here

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  call_info[1].call_id = 3;
  call_info[1].call_state = CALL_STATE_HOLD_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 1;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_WAITING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  number_list[1].call_id = 3;
  number_list[1].number_len = strlen("n_way_voice");
  strlcpy(number_list[1].number, "n_way_voice", strlen("n_way_voice")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536962");
  strlcpy(number_list[0].number, "2428536962", strlen("2428536962")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_WAITING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 3;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_HOLDING;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->isMpty = 1;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_ANSWER
  IMS_node.update_default_ims_request_answer();
  t.inject(IMS_node);

  // Expect: QMI_VOICE_ANSWER_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_answer_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_ANSWER_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_answer_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  call_info[1].call_id = 3;
  call_info[1].call_state = CALL_STATE_HOLD_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 1;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  number_list[1].call_id = 3;
  number_list[1].number_len = strlen("n_way_voice");
  strlcpy(number_list[1].number, "n_way_voice", strlen("n_way_voice")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536962");
  strlcpy(number_list[0].number, "2428536962", strlen("2428536962")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_ANSWER (resp)
  IMS_node.update_default_ims_request_answer_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 3;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_HOLDING;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->isMpty = 1;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_CONFERENCE
  IMS_node.update_default_ims_request_conference();
  t.inject(IMS_node);

  // Expect: QMI_VOICE_MANAGE_IP_CALLS_REQ_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_req_msg(VOIP_SUPS_TYPE_MAKE_CONFERENCE_CALL_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(3);
  call_info[1].call_id = 1;
  call_info[1].call_state = CALL_STATE_HOLD_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 0;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 3;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 1;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(3);
  number_list[1].call_id = 1;
  number_list[1].number_len = strlen("2428536962");
  strlcpy(number_list[1].number, "2428536962", strlen("2428536962")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 3;
  number_list[0].number_len = strlen("n_way_voice");
  strlcpy(number_list[0].number, "n_way_voice", strlen("n_way_voice")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(3);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_HOLDING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 3;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_ACTIVE;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->isMpty = 1;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(3);
  call_info[1].call_id = 1;
  call_info[1].call_state = CALL_STATE_END_V02;
  call_info[1].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[1].direction = CALL_DIRECTION_MO_V02;
  call_info[1].is_mpty = 0;
  call_info[1].mode = CALL_MODE_LTE_V02;
  call_info[0].call_id = 3;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 1;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(3);
  number_list[1].call_id = 1;
  number_list[1].number_len = strlen("2428536962");
  strlcpy(number_list[1].number, "2428536962", strlen("2428536962")+1);
  number_list[1].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  number_list[0].call_id = 3;
  number_list[0].number_len = strlen("n_way_voice");
  strlcpy(number_list[0].number, "n_way_voice", strlen("n_way_voice")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(3);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[1]->has_index = TRUE;
  calls[1]->index = 3;
  calls[1]->has_state = TRUE;
  calls[1]->state = ims_CallState_CALL_ACTIVE;
  calls[1]->has_toa = TRUE;
  calls[1]->toa = 129;
  calls[1]->has_isMpty = TRUE;
  calls[1]->isMpty = 1;
  calls[1]->has_isMT = TRUE;
  calls[1]->has_als = TRUE;
  calls[1]->has_isVoice = TRUE;
  calls[1]->isVoice     = 1;
  calls[1]->has_isVoicePrivacy = TRUE;
  calls[1]->isVoicePrivacy     = 0;
  calls[1]->has_numberPresentation = TRUE;
  calls[1]->numberPresentation     = 1;
  calls[1]->has_namePresentation = TRUE;
  calls[1]->namePresentation     = 0;
  calls[1]->callDetails.has_callDomain = TRUE;
  calls[1]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[1]->callDetails.has_callType = TRUE;
  calls[1]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_MANAGE_IP_CALLS_RESP_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_CONFERENCE (resp)
  IMS_node.update_default_ims_request_conference_resp();
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(3);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(3);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(3);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 3;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 1;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 3;
  number_list[0].number_len = strlen("n_way_voice");
  strlcpy(number_list[0].number, "n_way_voice", strlen("n_way_voice")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  voice_call_end_reason_type_v02* end = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end[0].call_id = 1;
  end[0].call_end_reason = CALL_END_CAUSE_NORMAL_CALL_CLEARING_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 3;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->has_failCause = true;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_mo_csvt_call_L
@description: MO CSVT call
==**/
test_result ril_utf_mo_csvt_call_L(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id("ril_utf_mo_csvt_call_L");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_CS;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  t.inject(IMS_node);

  // Expect: QMI_CSVT_ORIGINATE_CALL_REQ_V01
  QMI_node.update_default_qmi_csvt_originate_call_req(1, CSVT_CALL_MODE_DATA_V01, "2428536960");
  t.expect(QMI_node);

  // Inject: QMI_CSVT_ORIGINATE_CALL_RESP_V01
  QMI_node.update_default_qmi_csvt_originate_call_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject: QMI_CSVT_EVENT_REPORT_IND_V01
  QMI_node.update_default_qmi_csvt_event_report_ind(1, CSVT_EVENT_TYPE_CONFIRM_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_CS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  calls[0]->callDetails.has_mediaId = FALSE;
  calls[0]->callDetails.has_callSubstate = FALSE;
  calls[0]->has_isEncrypted = FALSE;
  t.expect(IMS_node);

  // Inject: QMI_CSVT_EVENT_REPORT_IND_V01
  QMI_node.update_default_qmi_csvt_event_report_ind(1, CSVT_EVENT_TYPE_PROGRESS_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_CS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  calls[0]->callDetails.has_mediaId = FALSE;
  calls[0]->callDetails.has_callSubstate = FALSE;
  calls[0]->has_isEncrypted = FALSE;
  t.expect(IMS_node);

  // Inject: QMI_CSVT_EVENT_REPORT_IND_V01
  QMI_node.update_default_qmi_csvt_event_report_ind(1, CSVT_EVENT_TYPE_CONNECT_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_CS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  calls[0]->callDetails.has_mediaId = FALSE;
  calls[0]->callDetails.has_callSubstate = FALSE;
  calls[0]->has_isEncrypted = FALSE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(1);
  t.inject(IMS_node);

  // Expect: QMI_CSVT_END_CALL_REQ_V01
  QMI_node.update_default_qmi_csvt_end_call_req(1);
  t.expect(QMI_node);

  // Inject: QMI_CSVT_END_CALL_RESP_V01
  QMI_node.update_default_qmi_csvt_end_call_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject: QMI_CSVT_EVENT_REPORT_IND_V01
  QMI_node.update_default_qmi_csvt_event_report_ind(1, CSVT_EVENT_TYPE_END_V01);
  QMI_node.set_qmi_csvt_event_report_ind_call_end_cause(16);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_CS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  calls[0]->callDetails.has_mediaId = FALSE;
  calls[0]->callDetails.has_callSubstate = FALSE;
  //calls[0]->has_failCause = true;
  //calls[0]->failCause.has_failcause = TRUE;
  //calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_MISC;
  calls[0]->has_isEncrypted = FALSE;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_mt_csvt_call_L
@description: MT CSVT Call
==**/
test_result ril_utf_mt_csvt_call_L(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id("ril_utf_mt_csvt_call_L");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  ims_CallList_Call** calls;

  // Inject: QMI_CSVT_EVENT_REPORT_IND_V01
  QMI_node.update_default_qmi_csvt_event_report_ind(1, CSVT_EVENT_TYPE_SETUP_V01);
  t.inject(QMI_node);

  // Expect: QMI_CSVT_CONFIRM_CALL_REQ_V01
  QMI_node.update_default_qmi_csvt_confirm_call_req(1, 1);
  t.expect(QMI_node);

  // Inject: QMI_CSVT_CONFIRM_CALL_RESP_V01
  QMI_node.update_default_qmi_csvt_confirm_call_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject: QMI_CSVT_EVENT_REPORT_IND_V01
  QMI_node.update_default_qmi_csvt_event_report_ind(1, CSVT_EVENT_TYPE_INCOMING_V01);
  // incoming number 18682569385 in BCD format
  char incoming_number[] = {0x21, (char )0x83, (char )0x81, (char )0x86, 0x52, (char )0x96, (char )0x83, (char )0xF5, 0x00};
  QMI_node.set_qmi_csvt_event_report_ind_incoming_number(incoming_number);
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_INCOMING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_CS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  calls[0]->callDetails.has_mediaId = FALSE;
  calls[0]->callDetails.has_callSubstate = FALSE;
  calls[0]->has_isEncrypted = FALSE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_ANSWER
  IMS_node.update_default_ims_request_answer();
  t.inject(IMS_node);

  // Expect: QMI_CSVT_ANSWER_CALL_REQ_V01
  QMI_node.update_default_qmi_csvt_answer_call_req(1, 1);
  t.expect(QMI_node);

  // Inject: QMI_CSVT_ANSWER_CALL_RESP_V01
  QMI_node.update_default_qmi_csvt_answer_call_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject: QMI_CSVT_EVENT_REPORT_IND_V01
  QMI_node.update_default_qmi_csvt_event_report_ind(1, CSVT_EVENT_TYPE_CONNECT_V01);
  QMI_node.set_qmi_csvt_event_report_ind_incoming_number(incoming_number);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_ANSWER (resp)
  IMS_node.update_default_ims_request_answer_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_CS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  calls[0]->callDetails.has_mediaId = FALSE;
  calls[0]->callDetails.has_callSubstate = FALSE;
  calls[0]->has_isEncrypted = FALSE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(1);
  t.inject(IMS_node);

  // Expect: QMI_CSVT_END_CALL_REQ_V01
  QMI_node.update_default_qmi_csvt_end_call_req(1);
  t.expect(QMI_node);

  // Inject: QMI_CSVT_END_CALL_RESP_V01
  QMI_node.update_default_qmi_csvt_end_call_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject: QMI_CSVT_EVENT_REPORT_IND_V01
  QMI_node.update_default_qmi_csvt_event_report_ind(1, CSVT_EVENT_TYPE_END_V01);
  QMI_node.set_qmi_csvt_event_report_ind_call_end_cause(16);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_CS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  calls[0]->callDetails.has_mediaId = FALSE;
  calls[0]->callDetails.has_callSubstate = FALSE;
  //calls[0]->has_failCause = TRUE;
  //calls[0]->failCause.has_failcause = TRUE;
  //calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_MISC;
  calls[0]->has_isEncrypted = FALSE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_LAST_CALL_FAIL_CAUSE
  IMS_node.update_default_ims_request_last_call_fail_cause();
  t.inject(IMS_node);

  // Expect: ims_MsgId_REQUEST_LAST_CALL_FAIL_CAUSE (resp)
  IMS_node.update_default_ims_request_last_call_fail_cause_resp();
  IMS_node.set_ims_request_last_call_fail_cause_failcause(ims_CallFailCause_CALL_FAIL_MISC);
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ssr_active_mo_csvt
@description: Trigger SSR when active CSVT call in progress.
==**/
test_result ril_utf_ssr_active_mo_csvt(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;
  AndroidTelTestnode RIL_node;
  SSRTestnode SSR_node;


  t.set_test_id(__func__);
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_CS;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  t.inject(IMS_node);

  // Expect: QMI_CSVT_ORIGINATE_CALL_REQ_V01
  QMI_node.update_default_qmi_csvt_originate_call_req(1, CSVT_CALL_MODE_DATA_V01, "2428536960");
  t.expect(QMI_node);

  // Inject: QMI_CSVT_ORIGINATE_CALL_RESP_V01
  QMI_node.update_default_qmi_csvt_originate_call_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject: QMI_CSVT_EVENT_REPORT_IND_V01
  QMI_node.update_default_qmi_csvt_event_report_ind(1, CSVT_EVENT_TYPE_CONFIRM_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_CS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  calls[0]->callDetails.has_mediaId = FALSE;
  calls[0]->callDetails.has_callSubstate = FALSE;
  t.expect(IMS_node);

  // Inject: QMI_CSVT_EVENT_REPORT_IND_V01
  QMI_node.update_default_qmi_csvt_event_report_ind(1, CSVT_EVENT_TYPE_PROGRESS_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_CS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  calls[0]->callDetails.has_mediaId = FALSE;
  calls[0]->callDetails.has_callSubstate = FALSE;
  t.expect(IMS_node);

  // Inject: QMI_CSVT_EVENT_REPORT_IND_V01
  QMI_node.update_default_qmi_csvt_event_report_ind(1, CSVT_EVENT_TYPE_CONNECT_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_CS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  calls[0]->callDetails.has_mediaId = FALSE;
  calls[0]->callDetails.has_callSubstate = FALSE;
  t.expect(IMS_node);


  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_VOICE);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_NAS);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_WMS);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_DMS);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_PBM);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_RF_SAR);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_RFPE);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_PDC);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_DSD);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_IMSA);
  t.inject(SSR_node);

  // Expect: RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED
  RIL_node.update_default_ril_unsol_resp_radio_state_changed();
  t.expect(RIL_node);

  // Expect: ims_MsgId_UNSOL_RADIO_STATE_CHANGED
  IMS_node.update_default_ims_unsol_radio_state_changed(ims_RadioState_RADIO_STATE_UNAVAILABLE);
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_CS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  calls[0]->callDetails.has_mediaId = FALSE;
  calls[0]->callDetails.has_callSubstate = FALSE;
  //calls[0]->failcause = IMS_node.set_ims_callfailcauseresponse(ims_CallFailCause_CALL_FAIL_MISC, 0);
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_MISC;
  t.expect(IMS_node);

  // Wait for notify cb requests
  t.add_func_exec(utf_wait_for_notify_reg_cb);

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_VOICE);
  t.inject(SSR_node);

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_NAS);
  t.inject(SSR_node);

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_WMS);
  t.inject(SSR_node);

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_DMS);
  t.inject(SSR_node);

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_PBM);
  t.inject(SSR_node);

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_RF_SAR);
  t.inject(SSR_node);

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_RFPE);
  t.inject(SSR_node);

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_PDC);
  t.inject(SSR_node);

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_DSD);
  t.inject(SSR_node);

  t.set_start();

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_IMSA);
  t.inject(SSR_node);

  // Expect: QMI_IMSA_IND_REG_REQ_V01
  QMI_node.update_default_qmi_imsa_ind_reg_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_IMSA_IND_REG_RSP_V01
  QMI_node.update_default_qmi_imsa_ind_reg_rsp_v01();
  QMI_node.set_qmi_imsa_ind_req_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  t.set_end();

  ril_utf_bootup_helper(t, SSR_BOOT);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ssr_active_mo_volte
@description: Trigger SSR when active IMS call in progress.
==**/
test_result ril_utf_ssr_active_mo_volte(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;
  AndroidTelTestnode RIL_node;
  SSRTestnode SSR_node;


  t.set_test_id(__func__);
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }


  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CC_IN_PROGRESS_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // Media ID
  voice_media_id_with_id_type_v02 *media_id = QMI_node.set_qmi_voice_all_call_status_ind_media_id(1);
  media_id[0].call_id = 1;
  media_id[0].media_id = 2;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_VOICE);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_NAS);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_WMS);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_DMS);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_PBM);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_RF_SAR);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_RFPE);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_PDC);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_DSD);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_IMS_SETTING);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_IMSA);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_LTE);
  t.inject(SSR_node);

  // Expect: RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED
  RIL_node.update_default_ril_unsol_resp_radio_state_changed();
  t.expect(RIL_node);

  // Expect: ims_MsgId_UNSOL_RADIO_STATE_CHANGED
  IMS_node.update_default_ims_unsol_radio_state_changed(ims_RadioState_RADIO_STATE_UNAVAILABLE);
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
//  calls[0]->has_failCause = TRUE;
//  calls[0]->failCause.has_failcause = TRUE;
//  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_ERROR_UNSPECIFIED;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Wait for notify cb requests
  t.add_func_exec(utf_wait_for_notify_reg_cb);

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_VOICE);
  t.inject(SSR_node);

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_NAS);
  t.inject(SSR_node);

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_WMS);
  t.inject(SSR_node);

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_DMS);
  t.inject(SSR_node);

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_PBM);
  t.inject(SSR_node);

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_RF_SAR);
  t.inject(SSR_node);

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_RFPE);
  t.inject(SSR_node);

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_PDC);
  t.inject(SSR_node);

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_DSD);
  t.inject(SSR_node);

  t.set_start();

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_IMSA);
  t.inject(SSR_node);

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_IMS_SETTING);
  t.inject(SSR_node);

  // Expect: QMI_IMS_SETTINGS_GET_SUPPORTED_MSGS_REQ_V01
  QMI_node.update_default_qmi_ims_settings_get_supported_msgs_req();
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_GET_SUPPORTED_MSGS_RESP_V01
  QMI_node.update_default_qmi_ims_settings_get_supported_msgs_resp();
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

  t.set_end();

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_LTE);
  t.inject(SSR_node);

  ril_utf_bootup_helper(t, SSR_BOOT);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ssr_active_mo_cs_call
@description: Trigger SSR when active CS call in progress.
==**/
test_result ril_utf_ssr_active_mo_cs_call(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;
  AndroidTelTestnode RIL_node;
  SSRTestnode SSR_node;


  t.set_test_id(__func__);
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }


  // Inject: RIL_REQUEST_DIAL
  RIL_node.update_default_ril_request_dial("2428536960");
  t.inject(RIL_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_V02);
#ifdef RIL_UTF_L_MASTER
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_CS_ONLY_V02);
#else
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_GSM_WCDMA_TDSCDMA_V02);
#endif
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_GSM_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

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
  RIL_node.set_ril_request_get_current_calls_resp_number(0, "2428536960");
  call[0]->numberPresentation = 0;
  call[0]->namePresentation = 2;
  t.expect(RIL_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_GSM_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

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
  RIL_node.set_ril_request_get_current_calls_resp_number(0, "2428536960");
  call[0]->numberPresentation = 0;
  call[0]->namePresentation = 2;
  t.expect(RIL_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_GSM_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

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
  RIL_node.set_ril_request_get_current_calls_resp_number(0, "2428536960");
  call[0]->numberPresentation = 0;
  call[0]->namePresentation = 2;
  t.expect(RIL_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_VOICE);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_NAS);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_WMS);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_DMS);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_PBM);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_RF_SAR);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_RFPE);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_PDC);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_DSD);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_IMS_SETTING);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_IMSA);
  t.inject(SSR_node);

  // Inject: CLIENT_ERROR_CALLBACK
  SSR_node.update_default_ssr_client_error_callback(QCRIL_QMI_CLIENT_LTE);
  t.inject(SSR_node);

  // Expect: RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED
  RIL_node.update_default_ril_unsol_resp_radio_state_changed();
  t.expect(RIL_node);

  // Expect: ims_MsgId_UNSOL_RADIO_STATE_CHANGED
  IMS_node.update_default_ims_unsol_radio_state_changed(ims_RadioState_RADIO_STATE_UNAVAILABLE);
  t.expect(IMS_node);

  // Wait for notify cb requests
  t.add_func_exec(utf_wait_for_notify_reg_cb);

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_VOICE);
  t.inject(SSR_node);

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_NAS);
  t.inject(SSR_node);

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_WMS);
  t.inject(SSR_node);

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_DMS);
  t.inject(SSR_node);

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_PBM);
  t.inject(SSR_node);

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_RF_SAR);
  t.inject(SSR_node);

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_RFPE);
  t.inject(SSR_node);

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_PDC);
  t.inject(SSR_node);

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_DSD);
  t.inject(SSR_node);

  t.set_start();

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_IMSA);
  t.inject(SSR_node);

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_IMS_SETTING);
  t.inject(SSR_node);

  // Expect: QMI_IMS_SETTINGS_GET_SUPPORTED_MSGS_REQ_V01
  QMI_node.update_default_qmi_ims_settings_get_supported_msgs_req();
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_GET_SUPPORTED_MSGS_RESP_V01
  QMI_node.update_default_qmi_ims_settings_get_supported_msgs_resp();
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

  t.set_end();

  // Inject: CLIENT_NOTIFY_CALLBACK
  SSR_node.update_default_ssr_client_notify_callback(QCRIL_QMI_CLIENT_LTE);
  t.inject(SSR_node);

  ril_utf_bootup_helper(t, SSR_BOOT);

  // Inject: RIL_REQUEST_GET_CURRENT_CALLS
  RIL_node.update_default_ril_request_get_current_calls();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_GET_CURRENT_CALLS (resp)
  RIL_node.update_default_ril_request_get_current_calls_resp(0);
  t.expect(RIL_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_srv_status_update_001
@description: for qcril gerrit 469424.
==**/
test_result ril_utf_ims_srv_status_update_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;
  AndroidTelTestnode RIL_node;

  t.set_test_id(__func__);
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  util_sms_over_ims_reg_state(t, RIL_node, QMI_node, TRUE, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01,
      TRUE, WMS_TRANSPORT_NW_REG_STATUS_FULL_SERVICE_V01, TRUE, QMI_RESULT_SUCCESS_V01,
      QMI_ERR_NONE_V01, TRUE, TRUE, TRUE, WMS_TRANSPORT_TYPE_IMS_V01, WMS_TRANSPORT_CAP_GW_V01,
      FALSE, TRUE, RADIO_TECH_3GPP, RIL_E_SUCCESS);

  // Inject: ims_MsgId_REQUEST_QUERY_SERVICE_STATUS
  IMS_node.update_default_ims_request_query_service_status();
  t.inject(IMS_node);

  // Expect: QMI_IMSA_GET_SERVICE_STATUS_REQ_V01
  QMI_node.update_default_qmi_imsa_get_service_status_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_IMSA_GET_SERVICE_STATUS_RSP_V01
  QMI_node.update_default_qmi_imsa_get_service_status_rsp_v01();
  QMI_node.set_qmi_imsa_get_service_status_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_imsa_get_service_status_rsp_v01_sms_service_status(IMSA_FULL_SERVICE_V01);
  QMI_node.set_qmi_imsa_get_service_status_rsp_v01_voip_service_status(IMSA_NO_SERVICE_V01);
  QMI_node.set_qmi_imsa_get_service_status_rsp_v01_vt_service_status(IMSA_NO_SERVICE_V01);
  QMI_node.set_qmi_imsa_get_service_status_rsp_v01_ut_service_status(IMSA_NO_SERVICE_V01);
  QMI_node.set_qmi_imsa_get_service_status_rsp_v01_sms_service_rat(IMSA_WWAN_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_QUERY_SERVICE_STATUS (resp)
  IMS_node.update_default_ims_request_query_service_status_resp(6);
  IMS_node.set_ims_request_query_service_status_setup_resp_sms_service(0, IMSA_FULL_SERVICE_V01, TRUE, IMSA_WWAN_V01);
  IMS_node.set_ims_request_query_service_status_setup_resp_voip_service(1, IMSA_NO_SERVICE_V01, FALSE, 0);
  IMS_node.set_ims_request_query_service_status_setup_resp_vt_service(2, IMSA_NO_SERVICE_V01, FALSE, 0);
  IMS_node.set_ims_request_query_service_status_setup_resp_ut_service(5, IMSA_NO_SERVICE_V01, FALSE, 0);
  t.expect(IMS_node);

  // Inject: QMI_IMSA_SERVICE_STATUS_IND_V01
  QMI_node.update_default_qmi_imsa_service_status_ind_v01();
  QMI_node.set_qmi_imsa_service_status_ind_v01_sms_service_status(IMSA_FULL_SERVICE_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_voip_service_status(IMSA_LIMITED_SERVICE_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_vt_service_status(IMSA_LIMITED_SERVICE_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_ut_service_status(IMSA_LIMITED_SERVICE_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_sms_service_rat(IMSA_WWAN_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_voip_service_rat(IMSA_WWAN_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_vt_service_rat(IMSA_WWAN_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_ut_service_rat(IMSA_WWAN_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_SRV_STATUS_UPDATE
  IMS_node.update_default_ims_unsol_srv_status_update(6);
  IMS_node.set_ims_unsol_srv_status_update_sms_service(0, IMSA_FULL_SERVICE_V01, TRUE, IMSA_WWAN_V01);
  IMS_node.set_ims_unsol_srv_status_update_voip_service(1, IMSA_LIMITED_SERVICE_V01, TRUE, IMSA_WWAN_V01);
  IMS_node.set_ims_unsol_srv_status_update_vt_service(2, IMSA_LIMITED_SERVICE_V01, TRUE, IMSA_WWAN_V01);
  IMS_node.set_ims_unsol_srv_status_update_ut_service(5, IMSA_LIMITED_SERVICE_V01, TRUE, IMSA_WWAN_V01);
  t.expect(IMS_node);


  // Inject: QMI_IMSA_SERVICE_STATUS_IND_V01
  QMI_node.update_default_qmi_imsa_service_status_ind_v01();
  QMI_node.set_qmi_imsa_service_status_ind_v01_sms_service_status(IMSA_FULL_SERVICE_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_voip_service_status(IMSA_FULL_SERVICE_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_vt_service_status(IMSA_FULL_SERVICE_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_ut_service_status(IMSA_FULL_SERVICE_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_sms_service_rat(IMSA_WWAN_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_voip_service_rat(IMSA_WWAN_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_vt_service_rat(IMSA_WWAN_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_ut_service_rat(IMSA_WWAN_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_SRV_STATUS_UPDATE
  IMS_node.update_default_ims_unsol_srv_status_update(6);
  IMS_node.set_ims_unsol_srv_status_update_sms_service(0, IMSA_FULL_SERVICE_V01, TRUE, IMSA_WWAN_V01);
  IMS_node.set_ims_unsol_srv_status_update_voip_service(1, IMSA_FULL_SERVICE_V01, TRUE, IMSA_WWAN_V01);
  IMS_node.set_ims_unsol_srv_status_update_vt_service(2, IMSA_FULL_SERVICE_V01, TRUE, IMSA_WWAN_V01);
  IMS_node.set_ims_unsol_srv_status_update_ut_service(5, IMSA_FULL_SERVICE_V01, TRUE, IMSA_WWAN_V01);
  t.expect(IMS_node);

#ifndef QMI_RIL_HAL_UTF
  // Expect: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_REQ_V01
  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_RSP_V01
  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_rsp_v01();
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);
#endif

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_srv_status_update_002
@description: IMS UT service update : on IMSA_WLAN_V01
==**/
test_result ril_utf_ims_srv_status_update_002(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;
  AndroidTelTestnode RIL_node;

  t.set_test_id(__func__);
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: ims_MsgId_REQUEST_QUERY_SERVICE_STATUS
  IMS_node.update_default_ims_request_query_service_status();
  t.inject(IMS_node);

  // Expect: QMI_IMSA_GET_SERVICE_STATUS_REQ_V01
  QMI_node.update_default_qmi_imsa_get_service_status_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_IMSA_GET_SERVICE_STATUS_RSP_V01
  QMI_node.update_default_qmi_imsa_get_service_status_rsp_v01();
  QMI_node.set_qmi_imsa_get_service_status_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_imsa_get_service_status_rsp_v01_sms_service_status(IMSA_FULL_SERVICE_V01);
  QMI_node.set_qmi_imsa_get_service_status_rsp_v01_voip_service_status(IMSA_NO_SERVICE_V01);
  QMI_node.set_qmi_imsa_get_service_status_rsp_v01_vt_service_status(IMSA_NO_SERVICE_V01);
  QMI_node.set_qmi_imsa_get_service_status_rsp_v01_ut_service_status(IMSA_NO_SERVICE_V01);
  QMI_node.set_qmi_imsa_get_service_status_rsp_v01_sms_service_rat(IMSA_WLAN_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_GET_TRANSPORT_NW_REG_INFO_REQ_V01
  QMI_node.update_default_qmi_wms_get_transport_nw_reg_info_req();
  t.expect(QMI_node);

  // Inject: QMI_WMS_GET_TRANSPORT_NW_REG_INFO_RESP_V01
  QMI_node.update_default_qmi_wms_get_transport_nw_reg_info_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_wms_get_transport_nw_reg_status(TRUE, WMS_TRANSPORT_NW_REG_STATUS_FULL_SERVICE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_GET_TRANSPORT_LAYER_INFO_REQ_V01
  QMI_node.update_default_qmi_wms_get_transport_layer_info_req();
  t.expect(QMI_node);

  // Inject: QMI_WMS_GET_TRANSPORT_LAYER_INFO_RESP_V01
  QMI_node.update_default_qmi_wms_get_transport_layer_info_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_wms_get_transport_layer_register_ind_info(TRUE, TRUE);
  QMI_node.set_qmi_wms_get_transport_layer_info_type_cap(TRUE, WMS_TRANSPORT_TYPE_IMS_V01, WMS_TRANSPORT_CAP_GW_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_QUERY_SERVICE_STATUS (resp)
  IMS_node.update_default_ims_request_query_service_status_resp(6);
  IMS_node.set_ims_request_query_service_status_setup_resp_sms_service(0, IMSA_FULL_SERVICE_V01, TRUE, IMSA_WLAN_V01);
  IMS_node.set_ims_request_query_service_status_setup_resp_voip_service(1, IMSA_NO_SERVICE_V01, FALSE, 0);
  IMS_node.set_ims_request_query_service_status_setup_resp_vt_service(2, IMSA_NO_SERVICE_V01, FALSE, 0);
  IMS_node.set_ims_request_query_service_status_setup_resp_ut_service(5, IMSA_NO_SERVICE_V01, FALSE, 0);
  t.expect(IMS_node);

  // Inject: QMI_IMSA_SERVICE_STATUS_IND_V01
  QMI_node.update_default_qmi_imsa_service_status_ind_v01();
  QMI_node.set_qmi_imsa_service_status_ind_v01_sms_service_status(IMSA_FULL_SERVICE_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_voip_service_status(IMSA_LIMITED_SERVICE_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_vt_service_status(IMSA_LIMITED_SERVICE_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_ut_service_status(IMSA_LIMITED_SERVICE_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_sms_service_rat(IMSA_WLAN_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_voip_service_rat(IMSA_WLAN_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_vt_service_rat(IMSA_WLAN_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_ut_service_rat(IMSA_WLAN_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_SRV_STATUS_UPDATE
  IMS_node.update_default_ims_unsol_srv_status_update(6);
  IMS_node.set_ims_unsol_srv_status_update_sms_service(0, IMSA_FULL_SERVICE_V01, TRUE, IMSA_WLAN_V01);
  IMS_node.set_ims_unsol_srv_status_update_voip_service(1, IMSA_LIMITED_SERVICE_V01, TRUE, IMSA_WLAN_V01);
  IMS_node.set_ims_unsol_srv_status_update_vt_service(2, IMSA_LIMITED_SERVICE_V01, TRUE, IMSA_WLAN_V01);
  IMS_node.set_ims_unsol_srv_status_update_ut_service(5, IMSA_LIMITED_SERVICE_V01, TRUE, IMSA_WLAN_V01);
  t.expect(IMS_node);


  // Inject: QMI_IMSA_SERVICE_STATUS_IND_V01
  QMI_node.update_default_qmi_imsa_service_status_ind_v01();
  QMI_node.set_qmi_imsa_service_status_ind_v01_sms_service_status(IMSA_FULL_SERVICE_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_voip_service_status(IMSA_FULL_SERVICE_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_vt_service_status(IMSA_FULL_SERVICE_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_ut_service_status(IMSA_FULL_SERVICE_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_sms_service_rat(IMSA_WLAN_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_voip_service_rat(IMSA_WLAN_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_vt_service_rat(IMSA_WLAN_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_ut_service_rat(IMSA_WLAN_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_SRV_STATUS_UPDATE
  IMS_node.update_default_ims_unsol_srv_status_update(6);
  IMS_node.set_ims_unsol_srv_status_update_sms_service(0, IMSA_FULL_SERVICE_V01, TRUE, IMSA_WLAN_V01);
  IMS_node.set_ims_unsol_srv_status_update_voip_service(1, IMSA_FULL_SERVICE_V01, TRUE, IMSA_WLAN_V01);
  IMS_node.set_ims_unsol_srv_status_update_vt_service(2, IMSA_FULL_SERVICE_V01, TRUE, IMSA_WLAN_V01);
  IMS_node.set_ims_unsol_srv_status_update_ut_service(5, IMSA_FULL_SERVICE_V01, TRUE, IMSA_WLAN_V01);
  t.expect(IMS_node);

#ifndef QMI_RIL_HAL_UTF
  // Expect: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_REQ_V01
  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_RSP_V01
  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_rsp_v01();
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);
#endif

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_srv_status_update_003
@description: IMS UT service update : on IMSA_WLAN_V01
==**/
test_result ril_utf_ims_srv_status_update_003(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;
  AndroidTelTestnode RIL_node;

  t.set_test_id(__func__);
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: ims_MsgId_REQUEST_QUERY_SERVICE_STATUS
  IMS_node.update_default_ims_request_query_service_status();
  t.inject(IMS_node);

  // Expect: QMI_IMSA_GET_SERVICE_STATUS_REQ_V01
  QMI_node.update_default_qmi_imsa_get_service_status_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_IMSA_GET_SERVICE_STATUS_RSP_V01
  QMI_node.update_default_qmi_imsa_get_service_status_rsp_v01();
  QMI_node.set_qmi_imsa_get_service_status_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_imsa_get_service_status_rsp_v01_sms_service_status(IMSA_LIMITED_SERVICE_V01);
  QMI_node.set_qmi_imsa_get_service_status_rsp_v01_voip_service_status(IMSA_NO_SERVICE_V01);
  QMI_node.set_qmi_imsa_get_service_status_rsp_v01_vt_service_status(IMSA_NO_SERVICE_V01);
  QMI_node.set_qmi_imsa_get_service_status_rsp_v01_ut_service_status(IMSA_NO_SERVICE_V01);
  QMI_node.set_qmi_imsa_get_service_status_rsp_v01_sms_service_rat(IMSA_IWLAN_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_GET_TRANSPORT_NW_REG_INFO_REQ_V01
  QMI_node.update_default_qmi_wms_get_transport_nw_reg_info_req();
  t.expect(QMI_node);

  // Inject: QMI_WMS_GET_TRANSPORT_NW_REG_INFO_RESP_V01
  QMI_node.update_default_qmi_wms_get_transport_nw_reg_info_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_wms_get_transport_nw_reg_status(TRUE, WMS_TRANSPORT_NW_REG_STATUS_LIMITED_SERVICE_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_QUERY_SERVICE_STATUS (resp)
  IMS_node.update_default_ims_request_query_service_status_resp(6);
  IMS_node.set_ims_request_query_service_status_setup_resp_sms_service(0, IMSA_LIMITED_SERVICE_V01, TRUE, IMSA_IWLAN_V01);
  IMS_node.set_ims_request_query_service_status_setup_resp_voip_service(1, IMSA_NO_SERVICE_V01, FALSE, 0);
  IMS_node.set_ims_request_query_service_status_setup_resp_vt_service(2, IMSA_NO_SERVICE_V01, FALSE, 0);
  IMS_node.set_ims_request_query_service_status_setup_resp_ut_service(5, IMSA_NO_SERVICE_V01, FALSE, 0);
  t.expect(IMS_node);

  util_sms_over_ims_reg_state(t, RIL_node, QMI_node, TRUE, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01,
      TRUE, WMS_TRANSPORT_NW_REG_STATUS_FULL_SERVICE_V01, TRUE, QMI_RESULT_SUCCESS_V01,
      QMI_ERR_NONE_V01, TRUE, TRUE, TRUE, WMS_TRANSPORT_TYPE_IMS_V01, WMS_TRANSPORT_CAP_GW_V01,
      FALSE, TRUE, RADIO_TECH_3GPP, RIL_E_SUCCESS);

  // Inject: QMI_IMSA_SERVICE_STATUS_IND_V01
  QMI_node.update_default_qmi_imsa_service_status_ind_v01();
  QMI_node.set_qmi_imsa_service_status_ind_v01_sms_service_status(IMSA_FULL_SERVICE_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_voip_service_status(IMSA_LIMITED_SERVICE_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_vt_service_status(IMSA_LIMITED_SERVICE_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_ut_service_status(IMSA_LIMITED_SERVICE_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_sms_service_rat(IMSA_IWLAN_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_voip_service_rat(IMSA_IWLAN_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_vt_service_rat(IMSA_IWLAN_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_ut_service_rat(IMSA_IWLAN_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_SRV_STATUS_UPDATE
  IMS_node.update_default_ims_unsol_srv_status_update(6);
  IMS_node.set_ims_unsol_srv_status_update_sms_service(0, IMSA_FULL_SERVICE_V01, TRUE, IMSA_IWLAN_V01);
  IMS_node.set_ims_unsol_srv_status_update_voip_service(1, IMSA_LIMITED_SERVICE_V01, TRUE, IMSA_IWLAN_V01);
  IMS_node.set_ims_unsol_srv_status_update_vt_service(2, IMSA_LIMITED_SERVICE_V01, TRUE, IMSA_IWLAN_V01);
  IMS_node.set_ims_unsol_srv_status_update_ut_service(5, IMSA_LIMITED_SERVICE_V01, TRUE, IMSA_IWLAN_V01);
  t.expect(IMS_node);


  // Inject: QMI_IMSA_SERVICE_STATUS_IND_V01
  QMI_node.update_default_qmi_imsa_service_status_ind_v01();
  QMI_node.set_qmi_imsa_service_status_ind_v01_sms_service_status(IMSA_FULL_SERVICE_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_voip_service_status(IMSA_FULL_SERVICE_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_vt_service_status(IMSA_FULL_SERVICE_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_ut_service_status(IMSA_FULL_SERVICE_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_sms_service_rat(IMSA_IWLAN_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_voip_service_rat(IMSA_IWLAN_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_vt_service_rat(IMSA_IWLAN_V01);
  QMI_node.set_qmi_imsa_service_status_ind_v01_ut_service_rat(IMSA_IWLAN_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_SRV_STATUS_UPDATE
  IMS_node.update_default_ims_unsol_srv_status_update(6);
  IMS_node.set_ims_unsol_srv_status_update_sms_service(0, IMSA_FULL_SERVICE_V01, TRUE, IMSA_IWLAN_V01);
  IMS_node.set_ims_unsol_srv_status_update_voip_service(1, IMSA_FULL_SERVICE_V01, TRUE, IMSA_IWLAN_V01);
  IMS_node.set_ims_unsol_srv_status_update_vt_service(2, IMSA_FULL_SERVICE_V01, TRUE, IMSA_IWLAN_V01);
  IMS_node.set_ims_unsol_srv_status_update_ut_service(5, IMSA_FULL_SERVICE_V01, TRUE, IMSA_IWLAN_V01);
  t.expect(IMS_node);

#ifndef QMI_RIL_HAL_UTF
  // Expect: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_REQ_V01
  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_RSP_V01
  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_rsp_v01();
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_v01_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);
#endif

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_srv_status_update_004
@description:
==**/
test_result ril_utf_ims_srv_status_update_004(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;
  AndroidTelTestnode RIL_node;

  t.set_test_id(__func__);
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: ims_MsgId_REQUEST_QUERY_SERVICE_STATUS
  IMS_node.update_default_ims_request_query_service_status();
  t.inject(IMS_node);

  // Expect: QMI_IMSA_GET_SERVICE_STATUS_REQ_V01
  QMI_node.update_default_qmi_imsa_get_service_status_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_IMSA_GET_SERVICE_STATUS_RSP_V01
  QMI_node.update_default_qmi_imsa_get_service_status_rsp_v01();
  QMI_node.set_qmi_imsa_get_service_status_rsp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_imsa_get_service_status_rsp_v01_sms_service_status(IMSA_NO_SERVICE_V01);
  QMI_node.set_qmi_imsa_get_service_status_rsp_v01_voip_service_status(IMSA_NO_SERVICE_V01);
  QMI_node.set_qmi_imsa_get_service_status_rsp_v01_vt_service_status(IMSA_NO_SERVICE_V01);
  QMI_node.set_qmi_imsa_get_service_status_rsp_v01_ut_service_status(IMSA_NO_SERVICE_V01);
  t.inject(QMI_node);

  // Expect: QMI_WMS_GET_TRANSPORT_NW_REG_INFO_REQ_V01
  QMI_node.update_default_qmi_wms_get_transport_nw_reg_info_req();
  t.expect(QMI_node);

  // Inject: QMI_WMS_GET_TRANSPORT_NW_REG_INFO_RESP_V01
  QMI_node.update_default_qmi_wms_get_transport_nw_reg_info_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_wms_get_transport_nw_reg_status(TRUE, WMS_TRANSPORT_NW_REG_STATUS_NO_SERVICE_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_QUERY_SERVICE_STATUS (resp)
  IMS_node.update_default_ims_request_query_service_status_resp(6);
  IMS_node.set_ims_request_query_service_status_setup_resp_sms_service(0, IMSA_NO_SERVICE_V01, FALSE, 0);
  IMS_node.set_ims_request_query_service_status_setup_resp_voip_service(1, IMSA_NO_SERVICE_V01, FALSE, 0);
  IMS_node.set_ims_request_query_service_status_setup_resp_vt_service(2, IMSA_NO_SERVICE_V01, FALSE, 0);
  IMS_node.set_ims_request_query_service_status_setup_resp_ut_service(5, IMSA_NO_SERVICE_V01, FALSE, 0);
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_no_srv_dial_fail_L
@description: DIAL_REQ failure scenario when limited/no service
==**/
test_result ril_utf_ims_no_srv_dial_fail_L(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  OEMTestnode OEM_node;
  IMSTestnode IMS_node;
  AndroidTelTestnode RIL_node;

  t.set_test_id("ril_utf_ims_no_srv_dial_fail_L");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Set WCDMA limited service / No service
  util_voice_registration_state_for_wcdma_success(t,
          QMI_node,
          RIL_node,
          OEM_node,
          RADIO_TECH_UNKNOWN,
          RIL_VAL_REG_NOT_REGISTERED_SEARCHING_LTD_SRV,
          NAS_SYS_SRV_STATUS_LIMITED_V01,
          SYS_SRV_STATUS_LIMITED_V01,
          SYS_SRV_DOMAIN_CS_ONLY_V01,
          NAS_SYS_ROAM_STATUS_OFF_V01,
          FALSE,
          SYS_SRV_DOMAIN_NO_SRV_V01,
          0,
          FALSE,
          NULL,
          NULL,
          FALSE,
          0,
          0,
          NULL);

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_IP_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CC_IN_PROGRESS_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_ERROR_UNSPECIFIED;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_dial_fail_L
@description: DIAL_REQ failure scenario
==**/
test_result ril_utf_ims_dial_fail_L(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id("ril_utf_ims_dial_fail_L");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }
  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_IP_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CC_IN_PROGRESS_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_ERROR_UNSPECIFIED;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_mo_call_screen_state_change_L
@description: Screen state change ON and OFF while IMS call active
==**/
test_result ril_utf_ims_mo_call_screen_state_change_L(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;
  AndroidTelTestnode RIL_node;

  t.set_test_id("ril_utf_ims_mo_call_screen_state_change_L");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_IP_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  util_ril_utf_test_screen_state(t, RIL_node, QMI_node, 0, FALSE, FALSE, FALSE);

  util_ril_utf_test_screen_state(t, RIL_node, QMI_node, 1, FALSE, TRUE, TRUE);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  util_ril_utf_test_screen_state(t, RIL_node, QMI_node, 0, FALSE, FALSE, FALSE);

  util_ril_utf_test_screen_state(t, RIL_node, QMI_node, 1, FALSE, TRUE, TRUE);

  // Inject: ims_MsgId_REQUEST_HANGUP
  IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(1);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
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

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);


  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  t.expect(IMS_node);

  util_ril_utf_test_screen_state(t, RIL_node, QMI_node, 0, TRUE, FALSE, FALSE);

  util_ril_utf_test_screen_state(t, RIL_node, QMI_node, 1, TRUE, TRUE, TRUE);

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();
}


//==================================================
// Allow Data tests
//  Description:
//   Test cases for allow data scenarios
//==================================================
/**==
@testname: ril_utf_msim_wcdma_default_allow_data_true
@description: enable allow data on a SIM with default mode
==**/
test_result ril_utf_msim_wcdma_default_allow_data_true(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  OEMTestnode OEM_node;

  t.set_test_attribute("AllowData");
  // DSDS tests require one of the msim boot cases
  t.set_boot_case(qmi_ril_msim_bootup_primary_002);

  t.set_test_id(__func__);
  t.set_test_attribute("MSIM");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Enable WCDMA
  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  util_set_pref_net_type(t, RIL_node, QMI_node, QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA, RADIO_TECH_UNKNOWN, 0, RIL_E_SUCCESS, 0, TRUE, FALSE, FALSE, FALSE);
  util_voice_radio_tech_wcdma_in_service(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN, NAS_SYS_SRV_STATUS_SRV_V01, SYS_SRV_STATUS_SRV_V01, SYS_SRV_DOMAIN_CS_PS_V01, NAS_SYS_ROAM_STATUS_OFF_V01);

  // Inject: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_REQ
  OEM_node.update_default_oem_request_set_data_subscription_req(0);
  t.inject(OEM_node);

  t.set_start();
  // Expect: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_RESP
  OEM_node.update_default_oem_request_set_data_subscription_resp();
  t.expect(OEM_node);

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(TRUE);
  t.inject(RIL_node);

  // Expect: QMI_EMBMS_UNSOL_EMBMS_STATUS_IND
  OEM_node.update_default_qmi_embms_unsol_embms_status_ind();
  OEM_node.set_qmi_embms_unsol_embms_status(0);
  t.expect(OEM_node);

  //Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  //Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_ONLY_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_CDMA_HRPD_LTE);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  util_ril_utf_get_device_cap_info(t, QMI_node, RIL_node, OEM_node, 1,1, TRUE);//DSDS

  // set dual standby pref
  util_ril_utf_set_dual_standby_pref(t, QMI_node, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, QMI_SRV_DOMAIN_PREF_CS_PS_V01, NAS_DDS_DURATION_PERMANANT_V01);
  t.set_end();

  // DSD IND
  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  util_ril_utf_dsd_ind_info(t, RIL_node, OEM_node, QMI_node, TRUE, 1, DSD_SYS_NETWORK_3GPP_V01, DSD_SYS_RAT_EX_3GPP_LTE_V01, 0ull, TRUE, FALSE, TRUE, FALSE, RADIO_TECH_LTE, FALSE, FALSE);

  // Expect: RIL_REQUEST_ALLOW_DATA_RESP
  t.set_start();
  RIL_node.update_default_ril_request_allow_data_resp();
  t.expect(RIL_node);
  t.set_end();

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_msim_wcdma_default_allow_data_true_fail
@description: enable allow data on a SIM with default mode, but allow data fail because dual standby pref req fails
==**/
test_result ril_utf_msim_wcdma_default_allow_data_true_fail(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  OEMTestnode OEM_node;

  t.set_test_attribute("AllowData");
  // DSDS tests require one of the msim boot cases
  t.set_boot_case(qmi_ril_msim_bootup_primary_002);

  t.set_test_id(__func__);
  t.set_test_attribute("MSIM");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Enable WCDMA
  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  util_set_pref_net_type(t, RIL_node, QMI_node, QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA, RADIO_TECH_UNKNOWN, 0, RIL_E_SUCCESS, 0, TRUE, FALSE, FALSE, FALSE);
  util_voice_radio_tech_wcdma_in_service(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN, NAS_SYS_SRV_STATUS_SRV_V01, SYS_SRV_STATUS_SRV_V01, SYS_SRV_DOMAIN_CS_PS_V01, NAS_SYS_ROAM_STATUS_OFF_V01);

  // Inject: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_REQ
  OEM_node.update_default_oem_request_set_data_subscription_req(0);
  t.inject(OEM_node);

  t.set_start();
  // Expect: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_RESP
  OEM_node.update_default_oem_request_set_data_subscription_resp();
  t.expect(OEM_node);

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(TRUE);
  t.inject(RIL_node);

  // Expect: QMI_EMBMS_UNSOL_EMBMS_STATUS_IND
  OEM_node.update_default_qmi_embms_unsol_embms_status_ind();
  OEM_node.set_qmi_embms_unsol_embms_status(0);
  t.expect(OEM_node);

  //Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  //Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_ONLY_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_CDMA_HRPD_LTE);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_DUAL_STANDBY_PREF_REQ_MSG_V01_REQ
  QMI_node.update_default_qmi_nas_dual_standby_pref_req_msg();
  QMI_node.update_qmi_nas_dual_standby_pref_data_subs(NAS_PRIMARY_SUBSCRIPTION_V01);
  QMI_node.update_qmi_nas_dual_standby_pref_dds_duration(NAS_DDS_DURATION_PERMANANT_V01);
  t.expect(QMI_node);

  // Inject: QMI_NAS_DUAL_STANDBY_PREF_REQ_MSG_V01_RESP
  QMI_node.update_default_qmi_nas_dual_standby_pref_resp_msg(QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  // Expect: RIL_REQUEST_ALLOW_DATA_RESP
  RIL_node.update_default_ril_request_allow_data_resp(RIL_E_MODEM_ERR);
  t.expect(RIL_node);
  t.set_end();

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_msim_wcdma_default_allow_data_false
@description: disable allow data on a SIM with default mode
==**/
test_result ril_utf_msim_wcdma_default_allow_data_false(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  OEMTestnode OEM_node;

  t.set_test_attribute("AllowData");
  // DSDS tests require one of the msim boot cases
  t.set_boot_case(qmi_ril_msim_bootup_primary_002);

  t.set_test_id(__func__);
  t.set_test_attribute("MSIM");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }
  // Enable WCDMA
  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  util_set_pref_net_type(t, RIL_node, QMI_node, QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA, RADIO_TECH_UNKNOWN, 0, RIL_E_SUCCESS, 0, TRUE, FALSE, FALSE, FALSE);
  util_voice_radio_tech_wcdma_in_service(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN, NAS_SYS_SRV_STATUS_SRV_V01, SYS_SRV_STATUS_SRV_V01, SYS_SRV_DOMAIN_CS_PS_V01, NAS_SYS_ROAM_STATUS_OFF_V01);

  // Inject: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_REQ
  OEM_node.update_default_oem_request_set_data_subscription_req(0);
  t.inject(OEM_node);

  t.set_start();
  // Expect: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_RESP
  OEM_node.update_default_oem_request_set_data_subscription_resp();
  t.expect(OEM_node);

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(FALSE);
  t.inject(RIL_node);

  // Expect: QMI_EMBMS_UNSOL_EMBMS_STATUS_IND
  OEM_node.update_default_qmi_embms_unsol_embms_status_ind();
  OEM_node.set_qmi_embms_unsol_embms_status(0);
  t.expect(OEM_node);

  RIL_node.update_default_ril_request_allow_data_resp();
  t.expect(RIL_node);
  t.set_end();

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_msim_wcdma_default_allow_data_true_then_false
@description: enable then disable allow data on a SIM with default mode
==**/
test_result ril_utf_msim_wcdma_default_allow_data_true_then_false(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  OEMTestnode OEM_node;

  t.set_test_attribute("AllowData");
  // DSDS tests require one of the msim boot cases
  t.set_boot_case(qmi_ril_msim_bootup_primary_002);

  t.set_test_id(__func__);
  t.set_test_attribute("MSIM");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Enable WCDMA
  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  util_set_pref_net_type(t, RIL_node, QMI_node, QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA, RADIO_TECH_UNKNOWN, 0, RIL_E_SUCCESS, 0, TRUE, FALSE, FALSE, FALSE);
  util_voice_radio_tech_wcdma_in_service(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN, NAS_SYS_SRV_STATUS_SRV_V01, SYS_SRV_STATUS_SRV_V01, SYS_SRV_DOMAIN_CS_PS_V01, NAS_SYS_ROAM_STATUS_OFF_V01);

  // Inject: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_REQ
  OEM_node.update_default_oem_request_set_data_subscription_req(0);
  t.inject(OEM_node);

  t.set_start();
  // Expect: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_RESP
  OEM_node.update_default_oem_request_set_data_subscription_resp();
  t.expect(OEM_node);

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(TRUE);
  t.inject(RIL_node);

  // Expect: QMI_EMBMS_UNSOL_EMBMS_STATUS_IND
  OEM_node.update_default_qmi_embms_unsol_embms_status_ind();
  OEM_node.set_qmi_embms_unsol_embms_status(0);
  t.expect(OEM_node);

  //Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  //Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_ONLY_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_CDMA_HRPD_LTE);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // set dual standby pref
  util_ril_utf_set_dual_standby_pref(t, QMI_node, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, QMI_SRV_DOMAIN_PREF_CS_PS_V01, NAS_DDS_DURATION_PERMANANT_V01);
  t.set_end();

  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  // DSD IND
  util_ril_utf_dsd_ind_info(t, RIL_node, OEM_node, QMI_node, TRUE, 1, DSD_SYS_NETWORK_3GPP_V01, DSD_SYS_RAT_EX_3GPP_LTE_V01, 0ull, TRUE, FALSE, TRUE, FALSE, RADIO_TECH_LTE, FALSE, FALSE);

  // Expect: RIL_REQUEST_ALLOW_DATA_RESP
  t.set_start();
  RIL_node.update_default_ril_request_allow_data_resp();
  t.expect(RIL_node);
  t.set_end();

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(FALSE);
  t.inject(RIL_node);

  util_ril_utf_detach_apn(t, QMI_node, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, QMI_SRV_DOMAIN_PREF_CS_ONLY_V01);

  // Expect: RIL_REQUEST_ALLOW_DATA_RESP
  RIL_node.update_default_ril_request_allow_data_resp();

  t.expect(RIL_node);
  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_msim_wcdma_default_allow_data_false_then_true
@description: disable then enable allow data on a SIM with default mode
==**/
test_result ril_utf_msim_wcdma_default_allow_data_false_then_true(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  OEMTestnode OEM_node;

  t.set_test_attribute("AllowData");
  // DSDS tests require one of the msim boot cases
  t.set_boot_case(qmi_ril_msim_bootup_primary_002);

  t.set_test_id(__func__);
  t.set_test_attribute("MSIM");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Enable WCDMA
  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  util_set_pref_net_type(t, RIL_node, QMI_node, QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA, RADIO_TECH_UNKNOWN, 0, RIL_E_SUCCESS, 0, TRUE, FALSE, FALSE, FALSE);
  util_voice_radio_tech_wcdma_in_service(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN, NAS_SYS_SRV_STATUS_SRV_V01, SYS_SRV_STATUS_SRV_V01, SYS_SRV_DOMAIN_CS_PS_V01, NAS_SYS_ROAM_STATUS_OFF_V01);

  // Inject: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_REQ
  OEM_node.update_default_oem_request_set_data_subscription_req(0);
  t.inject(OEM_node);

  t.set_start();
  // Expect: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_RESP
  OEM_node.update_default_oem_request_set_data_subscription_resp();
  t.expect(OEM_node);

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(FALSE);
  t.inject(RIL_node);

  // Expect: QMI_EMBMS_UNSOL_EMBMS_STATUS_IND
  OEM_node.update_default_qmi_embms_unsol_embms_status_ind();
  OEM_node.set_qmi_embms_unsol_embms_status(0);
  t.expect(OEM_node);

  // Expect: RIL_REQUEST_ALLOW_DATA_RESP
  RIL_node.update_default_ril_request_allow_data_resp();
  t.expect(RIL_node);

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(TRUE);
  t.inject(RIL_node);

  //Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  //Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_ONLY_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_CDMA_HRPD_LTE);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // set dual standby pref
  util_ril_utf_set_dual_standby_pref(t, QMI_node, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, QMI_SRV_DOMAIN_PREF_CS_PS_V01, NAS_DDS_DURATION_PERMANANT_V01);
  t.set_end();

  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  // DSD IND
  util_ril_utf_dsd_ind_info(t, RIL_node, OEM_node, QMI_node, TRUE, 1, DSD_SYS_NETWORK_3GPP_V01, DSD_SYS_RAT_EX_3GPP_LTE_V01, 0ull, TRUE, FALSE, TRUE, FALSE, RADIO_TECH_LTE, FALSE, FALSE);

  // Expect: RIL_REQUEST_ALLOW_DATA_RESP
  t.set_start();
  RIL_node.update_default_ril_request_allow_data_resp();
  t.expect(RIL_node);
  t.set_end();

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_msim_wcdma_default_allow_data_true_then_true
@description: enable allow data on a SIM with on demand mode, and send another allow data true request afterwards
==**/
test_result ril_utf_msim_wcdma_default_allow_data_true_then_true(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  OEMTestnode OEM_node;

  t.set_test_attribute("AllowData");
  // DSDS tests require one of the msim boot cases
  t.set_boot_case(qmi_ril_msim_bootup_primary_002);

  t.set_test_id(__func__);
  t.set_test_attribute("MSIM");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Enable WCDMA
  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  util_set_pref_net_type(t, RIL_node, QMI_node, QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA, RADIO_TECH_UNKNOWN, 0, RIL_E_SUCCESS, 0, TRUE, FALSE, FALSE, FALSE);
  util_voice_radio_tech_wcdma_in_service(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN, NAS_SYS_SRV_STATUS_SRV_V01, SYS_SRV_STATUS_SRV_V01, SYS_SRV_DOMAIN_CS_PS_V01, NAS_SYS_ROAM_STATUS_OFF_V01);

  // Inject: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_REQ
  OEM_node.update_default_oem_request_set_data_subscription_req(0);
  t.inject(OEM_node);

  t.set_start();
  // Expect: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_RESP
  OEM_node.update_default_oem_request_set_data_subscription_resp();
  t.expect(OEM_node);

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(TRUE);
  t.inject(RIL_node);

  // Expect: QMI_EMBMS_UNSOL_EMBMS_STATUS_IND
  OEM_node.update_default_qmi_embms_unsol_embms_status_ind();
  OEM_node.set_qmi_embms_unsol_embms_status(0);
  t.expect(OEM_node);

  //Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  //Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_ONLY_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_CDMA_HRPD_LTE);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // set dual standby pref
  util_ril_utf_set_dual_standby_pref(t, QMI_node, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, QMI_SRV_DOMAIN_PREF_CS_PS_V01, NAS_DDS_DURATION_PERMANANT_V01);
  t.set_end();

  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  // DSD IND
  util_ril_utf_dsd_ind_info(t, RIL_node, OEM_node, QMI_node, TRUE, 1, DSD_SYS_NETWORK_3GPP_V01, DSD_SYS_RAT_EX_3GPP_LTE_V01, 0ull, TRUE, FALSE, TRUE, FALSE, RADIO_TECH_LTE, FALSE, FALSE);

  // Expect: RIL_REQUEST_ALLOW_DATA_RESP
  t.set_start();
  RIL_node.update_default_ril_request_allow_data_resp();
  t.expect(RIL_node);
  t.set_end();

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(TRUE);
  t.inject(RIL_node);

  // Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_PS_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_CDMA_HRPD_LTE);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_DUAL_STANDBY_PREF_REQ_MSG_V01_REQ
  QMI_node.update_default_qmi_nas_dual_standby_pref_req_msg();
  QMI_node.update_qmi_nas_dual_standby_pref_data_subs(NAS_PRIMARY_SUBSCRIPTION_V01);
  QMI_node.update_qmi_nas_dual_standby_pref_dds_duration(NAS_DDS_DURATION_PERMANANT_V01);
  t.expect(QMI_node);

  // Inject: QMI_NAS_DUAL_STANDBY_PREF_REQ_MSG_V01_RESP
  QMI_node.update_default_qmi_nas_dual_standby_pref_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: RIL_REQUEST_ALLOW_DATA_RESP
  RIL_node.update_default_ril_request_allow_data_resp();
  t.expect(RIL_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_msim_wcdma_default_indication_first_allow_data_true
@description: receive a CS_PS indication first, then enable allow data on a SIM in on demand mode
==**/
test_result ril_utf_msim_wcdma_default_indication_first_allow_data_true(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  OEMTestnode OEM_node;

  t.set_test_attribute("AllowData");
  // DSDS tests require one of the msim boot cases
  t.set_boot_case(qmi_ril_msim_bootup_primary_002);

  t.set_test_id(__func__);
  t.set_test_attribute("MSIM");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Enable WCDMA
  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  util_set_pref_net_type(t, RIL_node, QMI_node, QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA, RADIO_TECH_UNKNOWN, 0, RIL_E_SUCCESS, 0, TRUE, FALSE, FALSE, FALSE);
  util_voice_radio_tech_wcdma_in_service(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN, NAS_SYS_SRV_STATUS_SRV_V01, SYS_SRV_STATUS_SRV_V01, SYS_SRV_DOMAIN_CS_PS_V01, NAS_SYS_ROAM_STATUS_OFF_V01);

  // Inject: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_REQ
  OEM_node.update_default_oem_request_set_data_subscription_req(0);
  t.inject(OEM_node);

  t.set_start();
  // Expect: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_RESP
  OEM_node.update_default_oem_request_set_data_subscription_resp();
  t.expect(OEM_node);

  // Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENE_IND
  QMI_node.update_default_qmi_nas_system_selection_preference_ind_msg();
  QMI_node.set_qmi_nas_system_selection_preference_ind_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_PS_V01);
  t.inject(QMI_node);

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(TRUE);
  t.inject(RIL_node);

  // Expect: QMI_EMBMS_UNSOL_EMBMS_STATUS_IND
  OEM_node.update_default_qmi_embms_unsol_embms_status_ind();
  OEM_node.set_qmi_embms_unsol_embms_status(0);
  t.expect(OEM_node);

  //Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  //Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_PS_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_CDMA_HRPD_LTE);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_DUAL_STANDBY_PREF_REQ_MSG_V01_REQ
  QMI_node.update_default_qmi_nas_dual_standby_pref_req_msg();
  QMI_node.update_qmi_nas_dual_standby_pref_data_subs(NAS_PRIMARY_SUBSCRIPTION_V01);
  QMI_node.update_qmi_nas_dual_standby_pref_dds_duration(NAS_DDS_DURATION_PERMANANT_V01);
  t.expect(QMI_node);

  // Inject: QMI_NAS_DUAL_STANDBY_PREF_REQ_MSG_V01_RESP
  QMI_node.update_default_qmi_nas_dual_standby_pref_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  RIL_node.update_default_ril_request_allow_data_resp();
  t.expect(RIL_node);
  t.set_end();

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_msim_wcdma_on_demand_allow_data_true
@description: enable allow data on a SIM with on-demand mode
==**/
test_result ril_utf_msim_wcdma_on_demand_allow_data_true(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  OEMTestnode OEM_node;

  t.set_test_attribute("AllowData");
  // DSDS tests require one of the msim boot cases
  t.set_boot_case(qmi_ril_msim_bootup_primary_002);

  t.set_test_id(__func__);
  t.set_test_attribute("MSIM");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Enable WCDMA
  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  util_set_pref_net_type(t, RIL_node, QMI_node, QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA, RADIO_TECH_UNKNOWN, 0, RIL_E_SUCCESS, 0, TRUE, FALSE, FALSE, FALSE);
  util_voice_radio_tech_wcdma_in_service(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN, NAS_SYS_SRV_STATUS_SRV_V01, SYS_SRV_STATUS_SRV_V01, SYS_SRV_DOMAIN_CS_PS_V01, NAS_SYS_ROAM_STATUS_OFF_V01);

  // Inject: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_REQ
  OEM_node.update_default_oem_request_set_data_subscription_req(1);
  t.inject(OEM_node);

  t.set_start();
  // Expect: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_RESP
  OEM_node.update_default_oem_request_set_data_subscription_resp();
  t.expect(OEM_node);

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(TRUE);
  t.inject(RIL_node);

  // Expect: QMI_EMBMS_UNSOL_EMBMS_STATUS_IND
  OEM_node.update_default_qmi_embms_unsol_embms_status_ind();
  OEM_node.set_qmi_embms_unsol_embms_status(0);
  t.expect(OEM_node);

  //Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  //Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_ONLY_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_CDMA_HRPD_LTE);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // set dual standby pref
  util_ril_utf_set_dual_standby_pref(t, QMI_node, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, QMI_SRV_DOMAIN_PREF_CS_PS_V01, NAS_DDS_DURATION_TEMPORARY_V01);
  t.set_end();

  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  // DSD IND
  util_ril_utf_dsd_ind_info(t, RIL_node, OEM_node, QMI_node, TRUE, 1, DSD_SYS_NETWORK_3GPP_V01, DSD_SYS_RAT_EX_3GPP_LTE_V01, 0ull, TRUE, FALSE, TRUE, FALSE, RADIO_TECH_LTE, FALSE, FALSE);

  // Expect: RIL_REQUEST_ALLOW_DATA_RESP
  t.set_start();
  RIL_node.update_default_ril_request_allow_data_resp();
  t.expect(RIL_node);
  t.set_end();

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_msim_wcdma_on_demand_allow_data_true_fail
@description: enable allow data on a SIM with on-demand mode, but the on demand attach fails
==**/
test_result ril_utf_msim_wcdma_on_demand_allow_data_true_fail(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  OEMTestnode OEM_node;

  t.set_test_attribute("AllowData");
  // DSDS tests require one of the msim boot cases
  t.set_boot_case(qmi_ril_msim_bootup_primary_002);

  t.set_test_id(__func__);
  t.set_test_attribute("MSIM");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Enable WCDMA
  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  util_set_pref_net_type(t, RIL_node, QMI_node, QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA, RADIO_TECH_UNKNOWN, 0, RIL_E_SUCCESS, 0, TRUE, FALSE, FALSE, FALSE);
  util_voice_radio_tech_wcdma_in_service(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN, NAS_SYS_SRV_STATUS_SRV_V01, SYS_SRV_STATUS_SRV_V01, SYS_SRV_DOMAIN_CS_PS_V01, NAS_SYS_ROAM_STATUS_OFF_V01);

  // Inject: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_REQ
  OEM_node.update_default_oem_request_set_data_subscription_req(1);
  t.inject(OEM_node);

  t.set_start();
  // Expect: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_RESP
  OEM_node.update_default_oem_request_set_data_subscription_resp();
  t.expect(OEM_node);

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(TRUE);
  t.inject(RIL_node);

  // Expect: QMI_EMBMS_UNSOL_EMBMS_STATUS_IND
  OEM_node.update_default_qmi_embms_unsol_embms_status_ind();
  OEM_node.set_qmi_embms_unsol_embms_status(0);
  t.expect(OEM_node);

  //Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  //Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_ONLY_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_CDMA_HRPD_LTE);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: QMI_NAS_DUAL_STANDBY_PREF_REQ_MSG_V01_REQ
  QMI_node.update_default_qmi_nas_dual_standby_pref_req_msg();
  QMI_node.update_qmi_nas_dual_standby_pref_data_subs(NAS_PRIMARY_SUBSCRIPTION_V01);
  QMI_node.update_qmi_nas_dual_standby_pref_dds_duration(NAS_DDS_DURATION_TEMPORARY_V01);
  t.expect(QMI_node);

  // Inject: QMI_NAS_DUAL_STANDBY_PREF_REQ_MSG_V01_RESP
  QMI_node.update_default_qmi_nas_dual_standby_pref_resp_msg(QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  // Expect: RIL_REQUEST_ALLOW_DATA_RESP
  RIL_node.update_default_ril_request_allow_data_resp(RIL_E_MODEM_ERR);
  t.expect(RIL_node);
  t.set_end();

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_msim_wcdma_on_demand_allow_data_false
@description: disable allow data on a SIM with on-demand mode
==**/
test_result ril_utf_msim_wcdma_on_demand_allow_data_false(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  OEMTestnode OEM_node;

  t.set_test_attribute("AllowData");
  // DSDS tests require one of the msim boot cases
  t.set_boot_case(qmi_ril_msim_bootup_primary_002);

  t.set_test_id(__func__);
  t.set_test_attribute("MSIM");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Enable WCDMA
  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  util_set_pref_net_type(t, RIL_node, QMI_node, QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA, RADIO_TECH_UNKNOWN, 0, RIL_E_SUCCESS, 0, TRUE, FALSE, FALSE, FALSE);
  util_voice_radio_tech_wcdma_in_service(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN, NAS_SYS_SRV_STATUS_SRV_V01, SYS_SRV_STATUS_SRV_V01, SYS_SRV_DOMAIN_CS_PS_V01, NAS_SYS_ROAM_STATUS_OFF_V01);

  // Inject: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_REQ
  OEM_node.update_default_oem_request_set_data_subscription_req(1);
  t.inject(OEM_node);

  t.set_start();
  // Expect: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_RESP
  OEM_node.update_default_oem_request_set_data_subscription_resp();
  t.expect(OEM_node);

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(FALSE);
  t.inject(RIL_node);

  // Expect: QMI_EMBMS_UNSOL_EMBMS_STATUS_IND
  OEM_node.update_default_qmi_embms_unsol_embms_status_ind();
  OEM_node.set_qmi_embms_unsol_embms_status(0);
  t.expect(OEM_node);

  // Expect: RIL_REQUEST_ALLOW_DATA_RESP
  RIL_node.update_default_ril_request_allow_data_resp();
  t.expect(RIL_node);
  t.set_end();

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_msim_wcdma_on_demand_allow_data_true_then_false
@description: enable then disable allow data on a SIM with on-demand mode
==**/
test_result ril_utf_msim_wcdma_on_demand_allow_data_true_then_false(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  OEMTestnode OEM_node;

  t.set_test_attribute("AllowData");
  // DSDS tests require one of the msim boot cases
  t.set_boot_case(qmi_ril_msim_bootup_primary_002);

  t.set_test_id(__func__);
  t.set_test_attribute("MSIM");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Enable WCDMA
  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  util_set_pref_net_type(t, RIL_node, QMI_node, QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA, RADIO_TECH_UNKNOWN, 0, RIL_E_SUCCESS, 0, TRUE, FALSE, FALSE, FALSE);
  util_voice_radio_tech_wcdma_in_service(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN, NAS_SYS_SRV_STATUS_SRV_V01, SYS_SRV_STATUS_SRV_V01, SYS_SRV_DOMAIN_CS_PS_V01, NAS_SYS_ROAM_STATUS_OFF_V01);

  // Inject: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_REQ
  OEM_node.update_default_oem_request_set_data_subscription_req(1);
  t.inject(OEM_node);

  t.set_start();
  // Expect: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_RESP
  OEM_node.update_default_oem_request_set_data_subscription_resp();
  t.expect(OEM_node);

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(TRUE);
  t.inject(RIL_node);

  // Expect: QMI_EMBMS_UNSOL_EMBMS_STATUS_IND
  OEM_node.update_default_qmi_embms_unsol_embms_status_ind();
  OEM_node.set_qmi_embms_unsol_embms_status(0);
  t.expect(OEM_node);

  //Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  //Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_ONLY_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_CDMA_HRPD_LTE);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // set dual standby pref
  util_ril_utf_set_dual_standby_pref(t, QMI_node, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, QMI_SRV_DOMAIN_PREF_CS_PS_V01, NAS_DDS_DURATION_TEMPORARY_V01);
  t.set_end();

  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  // DSD IND
  util_ril_utf_dsd_ind_info(t, RIL_node, OEM_node, QMI_node, TRUE, 1, DSD_SYS_NETWORK_3GPP_V01, DSD_SYS_RAT_EX_3GPP_LTE_V01, 0ull, TRUE, FALSE, TRUE, FALSE, RADIO_TECH_LTE, FALSE, FALSE);

  // Expect: RIL_REQUEST_ALLOW_DATA_RESP
  t.set_start();
  RIL_node.update_default_ril_request_allow_data_resp();
  t.expect(RIL_node);
  t.set_end();

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(FALSE);
  t.inject(RIL_node);

  // Detach APN
  util_ril_utf_detach_apn(t, QMI_node, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, QMI_SRV_DOMAIN_PREF_CS_ONLY_V01);

  // Expect: RIL_REQUEST_ALLOW_DATA_RESP
  RIL_node.update_default_ril_request_allow_data_resp();
  t.expect(RIL_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_msim_wcdma_on_demand_allow_data_false_then_true
@description: disable then enable allow data on a SIM with on-demand mode
==**/
test_result ril_utf_msim_wcdma_on_demand_allow_data_false_then_true(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  OEMTestnode OEM_node;

  t.set_test_attribute("AllowData");
  // DSDS tests require one of the msim boot cases
  t.set_boot_case(qmi_ril_msim_bootup_primary_002);

  t.set_test_id(__func__);
  t.set_test_attribute("MSIM");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Enable WCDMA
  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  util_set_pref_net_type(t, RIL_node, QMI_node, QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA, RADIO_TECH_UNKNOWN, 0, RIL_E_SUCCESS, 0, TRUE, FALSE, FALSE, FALSE);
  util_voice_radio_tech_wcdma_in_service(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN, NAS_SYS_SRV_STATUS_SRV_V01, SYS_SRV_STATUS_SRV_V01, SYS_SRV_DOMAIN_CS_PS_V01, NAS_SYS_ROAM_STATUS_OFF_V01);

  // Inject: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_REQ
  OEM_node.update_default_oem_request_set_data_subscription_req(1);
  t.inject(OEM_node);

  t.set_start();
  // Expect: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_RESP
  OEM_node.update_default_oem_request_set_data_subscription_resp();
  t.expect(OEM_node);

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(FALSE);
  t.inject(RIL_node);

  // Expect: QMI_EMBMS_UNSOL_EMBMS_STATUS_IND
  OEM_node.update_default_qmi_embms_unsol_embms_status_ind();
  OEM_node.set_qmi_embms_unsol_embms_status(0);
  t.expect(OEM_node);

  // Expect: RIL_REQUEST_ALLOW_DATA_RESP
  RIL_node.update_default_ril_request_allow_data_resp();
  t.expect(RIL_node);

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(TRUE);
  t.inject(RIL_node);

  //Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  //Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_ONLY_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_CDMA_HRPD_LTE);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // set dual standby pref
  util_ril_utf_set_dual_standby_pref(t, QMI_node, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, QMI_SRV_DOMAIN_PREF_CS_PS_V01, NAS_DDS_DURATION_TEMPORARY_V01);
  t.set_end();

  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  // DSD IND
  util_ril_utf_dsd_ind_info(t, RIL_node, OEM_node, QMI_node, TRUE, 1, DSD_SYS_NETWORK_3GPP_V01, DSD_SYS_RAT_EX_3GPP_LTE_V01, 0ull, TRUE, FALSE, TRUE, FALSE, RADIO_TECH_LTE, FALSE, FALSE);

  // Expect: RIL_REQUEST_ALLOW_DATA_RESP
  t.set_start();
  RIL_node.update_default_ril_request_allow_data_resp();
  t.expect(RIL_node);
  t.set_end();

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_msim_wcdma_on_demand_allow_data_true_then_true
@description: enable allow data on a SIM with on demand mode, and send another allow data true request afterwards
==**/
test_result ril_utf_msim_wcdma_on_demand_allow_data_true_then_true(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  OEMTestnode OEM_node;

  t.set_test_attribute("AllowData");
  // DSDS tests require one of the msim boot cases
  t.set_boot_case(qmi_ril_msim_bootup_primary_002);

  t.set_test_id(__func__);
  t.set_test_attribute("MSIM");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Enable WCDMA
  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  util_set_pref_net_type(t, RIL_node, QMI_node, QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA, RADIO_TECH_UNKNOWN, 0, RIL_E_SUCCESS, 0, TRUE, FALSE, FALSE, FALSE);
  util_voice_radio_tech_wcdma_in_service(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN, NAS_SYS_SRV_STATUS_SRV_V01, SYS_SRV_STATUS_SRV_V01, SYS_SRV_DOMAIN_CS_PS_V01, NAS_SYS_ROAM_STATUS_OFF_V01);

  // Inject: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_REQ
  OEM_node.update_default_oem_request_set_data_subscription_req(1);
  t.inject(OEM_node);

  t.set_start();
  // Expect: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_RESP
  OEM_node.update_default_oem_request_set_data_subscription_resp();
  t.expect(OEM_node);

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(TRUE);
  t.inject(RIL_node);

  // Expect: QMI_EMBMS_UNSOL_EMBMS_STATUS_IND
  OEM_node.update_default_qmi_embms_unsol_embms_status_ind();
  OEM_node.set_qmi_embms_unsol_embms_status(0);
  t.expect(OEM_node);

  //Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  //Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_ONLY_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_CDMA_HRPD_LTE);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // set dual standby pref
  util_ril_utf_set_dual_standby_pref(t, QMI_node, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, QMI_SRV_DOMAIN_PREF_CS_PS_V01, NAS_DDS_DURATION_TEMPORARY_V01);
  t.set_end();

  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  // DSD IND
  util_ril_utf_dsd_ind_info(t, RIL_node, OEM_node, QMI_node, TRUE, 1, DSD_SYS_NETWORK_3GPP_V01, DSD_SYS_RAT_EX_3GPP_LTE_V01, 0ull, TRUE, FALSE, TRUE, FALSE, RADIO_TECH_LTE, FALSE, FALSE);

  // Expect: RIL_REQUEST_ALLOW_DATA_RESP
  t.set_start();
  RIL_node.update_default_ril_request_allow_data_resp();
  t.expect(RIL_node);
  t.set_end();

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(TRUE);
  t.inject(RIL_node);

  // Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  // Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_PS_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_CDMA_HRPD_LTE);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // set dual standby pref
  util_ril_utf_set_dual_standby_pref(t, QMI_node, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, QMI_SRV_DOMAIN_PREF_CS_PS_V01, NAS_DDS_DURATION_TEMPORARY_V01);

  // Expect: RIL_REQUEST_ALLOW_DATA_RESP
  RIL_node.update_default_ril_request_allow_data_resp();
  t.expect(RIL_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_msim_wcdma_on_demand_indication_first_allow_data_true
@description: receive a CS_PS indication first, then enable allow data on a SIM in on demand mode
==**/
test_result ril_utf_msim_wcdma_on_demand_indication_first_allow_data_true(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  OEMTestnode OEM_node;

  t.set_test_attribute("AllowData");
  // DSDS tests require one of the msim boot cases
  t.set_boot_case(qmi_ril_msim_bootup_primary_002);

  t.set_test_id(__func__);
  t.set_test_attribute("MSIM");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Enable WCDMA
  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  util_set_pref_net_type(t, RIL_node, QMI_node, QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA, RADIO_TECH_UNKNOWN, 0, RIL_E_SUCCESS, 0, TRUE, FALSE, FALSE, FALSE);
  util_voice_radio_tech_wcdma_in_service(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN, NAS_SYS_SRV_STATUS_SRV_V01, SYS_SRV_STATUS_SRV_V01, SYS_SRV_DOMAIN_CS_PS_V01, NAS_SYS_ROAM_STATUS_OFF_V01);

  // Inject: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_REQ
  OEM_node.update_default_oem_request_set_data_subscription_req(1);
  t.inject(OEM_node);

  t.set_start();
  // Expect: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_RESP
  OEM_node.update_default_oem_request_set_data_subscription_resp();
  t.expect(OEM_node);

  // Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENE_IND
  QMI_node.update_default_qmi_nas_system_selection_preference_ind_msg();
  QMI_node.set_qmi_nas_system_selection_preference_ind_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_PS_V01);
  t.inject(QMI_node);

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(TRUE);
  t.inject(RIL_node);

  // Expect: QMI_EMBMS_UNSOL_EMBMS_STATUS_IND
  OEM_node.update_default_qmi_embms_unsol_embms_status_ind();
  OEM_node.set_qmi_embms_unsol_embms_status(0);
  t.expect(OEM_node);

  //Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  //Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_PS_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_CDMA_HRPD_LTE);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // set dual standby pref
  util_ril_utf_set_dual_standby_pref(t, QMI_node, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, QMI_SRV_DOMAIN_PREF_CS_PS_V01, NAS_DDS_DURATION_TEMPORARY_V01);

  RIL_node.update_default_ril_request_allow_data_resp();
  t.expect(RIL_node);
  t.set_end();

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_msim_1x_default_allow_data_true
@description: enable allow data on a SIM with default mode, and then send allow data true request again
==**/
test_result ril_utf_msim_1x_default_allow_data_true(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  OEMTestnode OEM_node;

  t.set_test_attribute("AllowData");
  // DSDS tests require one of the msim boot cases
  t.set_boot_case(qmi_ril_msim_bootup_primary_002);

  t.set_test_id(__func__);
  t.set_test_attribute("MSIM");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }
  // Enable 1x
  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  util_set_pref_net_type(t, RIL_node, QMI_node, QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA, RADIO_TECH_UNKNOWN, 0, RIL_E_SUCCESS, 0, TRUE, FALSE, FALSE, FALSE);
  util_voice_radio_tech_cdma_in_service(t, QMI_node, RIL_node, TRUE, RADIO_TECH_IS95A, NAS_SYS_SRV_STATUS_SRV_V01, SYS_SRV_DOMAIN_CS_PS_V01, NAS_SYS_ROAM_STATUS_OFF_V01, TRUE);

  // Inject: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_REQ
  OEM_node.update_default_oem_request_set_data_subscription_req(0);
  t.inject(OEM_node);

  t.set_start();
  // Expect: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_RESP
  OEM_node.update_default_oem_request_set_data_subscription_resp();
  t.expect(OEM_node);

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(TRUE);
  t.inject(RIL_node);

  // Expect: QMI_EMBMS_UNSOL_EMBMS_STATUS_IND
  OEM_node.update_default_qmi_embms_unsol_embms_status_ind();
  OEM_node.set_qmi_embms_unsol_embms_status(0);
  t.expect(OEM_node);

  //Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  //Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_ONLY_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_CDMA_HRPD_LTE);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // set dual standby pref
  util_ril_utf_set_dual_standby_pref(t, QMI_node, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, QMI_SRV_DOMAIN_PREF_CS_PS_V01, NAS_DDS_DURATION_PERMANANT_V01);
  t.set_end();

  // DSD IND
  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  util_ril_utf_dsd_ind_info(t, RIL_node, OEM_node, QMI_node, TRUE, 1, DSD_SYS_NETWORK_3GPP_V01, DSD_SYS_RAT_EX_3GPP_LTE_V01, 0ull, TRUE, FALSE, TRUE, FALSE, RADIO_TECH_LTE, FALSE, FALSE);

  // Expect: RIL_REQUEST_ALLOW_DATA_RESP
  t.set_start();
  RIL_node.update_default_ril_request_allow_data_resp();
  t.expect(RIL_node);
  t.set_end();

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_msim_1x_default_allow_data_false
@description: disable allow data on a SIM with default mode
==**/
test_result ril_utf_msim_1x_default_allow_data_false(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  OEMTestnode OEM_node;

  t.set_test_attribute("AllowData");
  // DSDS tests require one of the msim boot cases
  t.set_boot_case(qmi_ril_msim_bootup_primary_002);

  t.set_test_id(__func__);
  t.set_test_attribute("MSIM");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }
  // Enable 1x
  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  util_set_pref_net_type(t, RIL_node, QMI_node, QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA, RADIO_TECH_UNKNOWN, 0, RIL_E_SUCCESS, 0, TRUE, FALSE, FALSE, FALSE);
  util_voice_radio_tech_cdma_in_service(t, QMI_node, RIL_node, TRUE, RADIO_TECH_IS95A, NAS_SYS_SRV_STATUS_SRV_V01, SYS_SRV_DOMAIN_CS_PS_V01, NAS_SYS_ROAM_STATUS_OFF_V01, TRUE);

  // Inject: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_REQ
  OEM_node.update_default_oem_request_set_data_subscription_req(0);
  t.inject(OEM_node);

  t.set_start();
  // Expect: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_RESP
  OEM_node.update_default_oem_request_set_data_subscription_resp();
  t.expect(OEM_node);

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(FALSE);
  t.inject(RIL_node);

  // Expect: QMI_EMBMS_UNSOL_EMBMS_STATUS_IND
  OEM_node.update_default_qmi_embms_unsol_embms_status_ind();
  OEM_node.set_qmi_embms_unsol_embms_status(0);
  t.expect(OEM_node);

  // Expect: RIL_REQUEST_ALLOW_DATA_RESP
  RIL_node.update_default_ril_request_allow_data_resp();
  t.expect(RIL_node);
  t.set_end();

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_msim_1x_on_demand_allow_data_true
@description: enable allow data on a SIM with on-demand mode
==**/
test_result ril_utf_msim_1x_on_demand_allow_data_true(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  OEMTestnode OEM_node;

  t.set_test_attribute("AllowData");
  // DSDS tests require one of the msim boot cases
  t.set_boot_case(qmi_ril_msim_bootup_primary_002);

  t.set_test_id(__func__);
  t.set_test_attribute("MSIM");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Enable 1x
  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  util_set_pref_net_type(t, RIL_node, QMI_node, QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA, RADIO_TECH_UNKNOWN, 0, RIL_E_SUCCESS, 0, TRUE, FALSE, FALSE, FALSE);
  util_voice_radio_tech_cdma_in_service(t, QMI_node, RIL_node, TRUE, RADIO_TECH_IS95A, NAS_SYS_SRV_STATUS_SRV_V01, SYS_SRV_DOMAIN_CS_PS_V01, NAS_SYS_ROAM_STATUS_OFF_V01, TRUE);

  // Inject: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_REQ
  OEM_node.update_default_oem_request_set_data_subscription_req(1);
  t.inject(OEM_node);

  t.set_start();
  // Expect: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_RESP
  OEM_node.update_default_oem_request_set_data_subscription_resp();
  t.expect(OEM_node);

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(TRUE);
  t.inject(RIL_node);

  // Expect: QMI_EMBMS_UNSOL_EMBMS_STATUS_IND
  OEM_node.update_default_qmi_embms_unsol_embms_status_ind();
  OEM_node.set_qmi_embms_unsol_embms_status(0);
  t.expect(OEM_node);

  //Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  //Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_ONLY_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_CDMA_HRPD_LTE);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // set dual standby pref
  util_ril_utf_set_dual_standby_pref(t, QMI_node, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, QMI_SRV_DOMAIN_PREF_CS_PS_V01, NAS_DDS_DURATION_TEMPORARY_V01);
  t.set_end();

  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  // DSD IND
  util_ril_utf_dsd_ind_info(t, RIL_node, OEM_node, QMI_node, TRUE, 1, DSD_SYS_NETWORK_3GPP_V01, DSD_SYS_RAT_EX_3GPP_LTE_V01, 0ull, TRUE, FALSE, TRUE, FALSE, RADIO_TECH_LTE, FALSE, FALSE);

  // Expect: RIL_REQUEST_ALLOW_DATA_RESP
  t.set_start();
  RIL_node.update_default_ril_request_allow_data_resp();
  t.expect(RIL_node);
  t.set_end();

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_msim_1x_on_demand_allow_data_false
@description: disable allow data on a SIM with on-demand mode
==**/
test_result ril_utf_msim_1x_on_demand_allow_data_false(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  OEMTestnode OEM_node;

  t.set_test_attribute("AllowData");
  // DSDS tests require one of the msim boot cases
  t.set_boot_case(qmi_ril_msim_bootup_primary_002);

  t.set_test_id(__func__);
  t.set_test_attribute("MSIM");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Enable 1x
  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  util_set_pref_net_type(t, RIL_node, QMI_node, QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA, RADIO_TECH_UNKNOWN, 0, RIL_E_SUCCESS, 0, TRUE, FALSE, FALSE, FALSE);
  util_voice_radio_tech_cdma_in_service(t, QMI_node, RIL_node, TRUE, RADIO_TECH_IS95A, NAS_SYS_SRV_STATUS_SRV_V01, SYS_SRV_DOMAIN_CS_PS_V01, NAS_SYS_ROAM_STATUS_OFF_V01, TRUE);

  // Inject: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_REQ
  OEM_node.update_default_oem_request_set_data_subscription_req(1);
  t.inject(OEM_node);

  t.set_start();
  // Expect: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_RESP
  OEM_node.update_default_oem_request_set_data_subscription_resp();
  t.expect(OEM_node);

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(FALSE);
  t.inject(RIL_node);

  // Expect: QMI_EMBMS_UNSOL_EMBMS_STATUS_IND
  OEM_node.update_default_qmi_embms_unsol_embms_status_ind();
  OEM_node.set_qmi_embms_unsol_embms_status(0);
  t.expect(OEM_node);

  // Expect: RIL_REQUEST_ALLOW_DATA_RESP
  RIL_node.update_default_ril_request_allow_data_resp();
  t.expect(RIL_node);
  t.set_end();

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_msim_1x_on_demand_indication_first_allow_data_ture_1
@description: for an on-demand sim, receive a CS_PS indication, then enable allow data, expecting an 1x dsd indication
==**/
test_result ril_utf_msim_1x_on_demand_indication_first_allow_data_ture_1(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  OEMTestnode OEM_node;

  t.set_test_attribute("AllowData");
  // DSDS tests require one of the msim boot cases
  t.set_boot_case(qmi_ril_msim_bootup_primary_002);

  t.set_test_id(__func__);
  t.set_test_attribute("MSIM");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Enable 1x
  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  util_set_pref_net_type(t, RIL_node, QMI_node, QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA, RADIO_TECH_UNKNOWN, 0, RIL_E_SUCCESS, 0, TRUE, FALSE, FALSE, FALSE);
  util_voice_radio_tech_cdma_in_service(t, QMI_node, RIL_node, TRUE, RADIO_TECH_IS95A, NAS_SYS_SRV_STATUS_SRV_V01, SYS_SRV_DOMAIN_CS_PS_V01, NAS_SYS_ROAM_STATUS_OFF_V01, TRUE);

  // Inject: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_REQ
  OEM_node.update_default_oem_request_set_data_subscription_req(1);
  t.inject(OEM_node);

  t.set_start();
  // Expect: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_RESP
  OEM_node.update_default_oem_request_set_data_subscription_resp();
  t.expect(OEM_node);

  // Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENE_IND
  QMI_node.update_default_qmi_nas_system_selection_preference_ind_msg();
  QMI_node.set_qmi_nas_system_selection_preference_ind_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_PS_V01);
  t.inject(QMI_node);

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(TRUE);
  t.inject(RIL_node);

  // Expect: QMI_EMBMS_UNSOL_EMBMS_STATUS_IND
  OEM_node.update_default_qmi_embms_unsol_embms_status_ind();
  OEM_node.set_qmi_embms_unsol_embms_status(0);
  t.expect(OEM_node);

  //Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  //Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_PS_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_CDMA_HRPD_LTE);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // set dual standby pref
  util_ril_utf_set_dual_standby_pref(t, QMI_node, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, QMI_SRV_DOMAIN_PREF_CS_PS_V01, NAS_DDS_DURATION_TEMPORARY_V01);
  t.set_end();

  // DSD IND
  util_ril_utf_dsd_ind_info(t, RIL_node, OEM_node, QMI_node, TRUE, 1, DSD_SYS_NETWORK_3GPP2_V01, DSD_SYS_RAT_EX_3GPP2_1X_V01, 0ull, TRUE, FALSE, TRUE, FALSE, RADIO_TECH_UNKNOWN, FALSE, FALSE);

  // Expect: RIL_REQUEST_ALLOW_DATA_RESP
  t.set_start();
  RIL_node.update_default_ril_request_allow_data_resp();
  t.expect(RIL_node);
  t.set_end();

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_msim_1x_on_demand_indication_first_allow_data_ture_2
@description: for an on-demand sim, receive a CS_PS indication, then enable allow data, do not set any mask, expecting a null dsd ind followed by an lte dsd ind
==**/
test_result ril_utf_msim_1x_on_demand_indication_first_allow_data_ture_2(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  OEMTestnode OEM_node;

  t.set_test_attribute("AllowData");
  // DSDS tests require one of the msim boot cases
  t.set_boot_case(qmi_ril_msim_bootup_primary_002);

  t.set_test_id(__func__);
  t.set_test_attribute("MSIM");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Enable 1x
  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  util_set_pref_net_type(t, RIL_node, QMI_node, QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA, RADIO_TECH_UNKNOWN, 0, RIL_E_SUCCESS, 0, TRUE, FALSE, FALSE, FALSE);
  util_voice_radio_tech_cdma_in_service(t, QMI_node, RIL_node, TRUE, RADIO_TECH_IS95A, NAS_SYS_SRV_STATUS_SRV_V01, SYS_SRV_DOMAIN_CS_PS_V01, NAS_SYS_ROAM_STATUS_OFF_V01, TRUE);

  // Inject: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_REQ
  OEM_node.update_default_oem_request_set_data_subscription_req(1);
  t.inject(OEM_node);

  t.set_start();
  // Expect: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_RESP
  OEM_node.update_default_oem_request_set_data_subscription_resp();
  t.expect(OEM_node);

  // Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENE_IND
  QMI_node.update_default_qmi_nas_system_selection_preference_ind_msg();
  QMI_node.set_qmi_nas_system_selection_preference_ind_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_PS_V01);
  t.inject(QMI_node);

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(TRUE);
  t.inject(RIL_node);

  // Expect: QMI_EMBMS_UNSOL_EMBMS_STATUS_IND
  OEM_node.update_default_qmi_embms_unsol_embms_status_ind();
  OEM_node.set_qmi_embms_unsol_embms_status(0);
  t.expect(OEM_node);

  //Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  //Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_PS_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_CDMA_HRPD_LTE);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // set dual standby pref
  util_ril_utf_set_dual_standby_pref(t, QMI_node, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, QMI_SRV_DOMAIN_PREF_CS_PS_V01, NAS_DDS_DURATION_TEMPORARY_V01);
  t.set_end();

  // DSD IND
  util_ril_utf_dsd_ind_info(t, RIL_node, OEM_node, QMI_node, TRUE, 1, DSD_SYS_NETWORK_3GPP2_V01, DSD_SYS_RAT_EX_NULL_BEARER_V01 , 0ull, TRUE, FALSE, TRUE, FALSE, RADIO_TECH_UNKNOWN, FALSE, FALSE);
  util_ril_utf_dsd_ind_info(t, RIL_node, OEM_node, QMI_node, TRUE, 1, DSD_SYS_NETWORK_3GPP2_V01, DSD_SYS_RAT_EX_3GPP_LTE_V01, 0ull, TRUE, FALSE, TRUE, FALSE, RADIO_TECH_UNKNOWN, FALSE, FALSE);

  // Expect: RIL_REQUEST_ALLOW_DATA_RESP
  t.set_start();
  RIL_node.update_default_ril_request_allow_data_resp();
  t.expect(RIL_node);
  t.set_end();

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_msim_1x_on_demand_indication_first_allow_data_ture_3
@description: for an on-demand sim, receive a CS_PS indication, then enable allow data, set rat_disabled_mask to LTE+HRPD for get_sys_sel_pref, and remove LTE bit for on demand attach ind
              expecting an lte dsd ind
==**/
test_result ril_utf_msim_1x_on_demand_indication_first_allow_data_ture_3(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  OEMTestnode OEM_node;

  t.set_test_attribute("AllowData");
  // DSDS tests require one of the msim boot cases
  t.set_boot_case(qmi_ril_msim_bootup_primary_002);

  t.set_test_id(__func__);
  t.set_test_attribute("MSIM");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Enable 1x
  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  util_set_pref_net_type(t, RIL_node, QMI_node, QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA, RADIO_TECH_UNKNOWN, 0, RIL_E_SUCCESS, 0, TRUE, FALSE, FALSE, FALSE);
  util_voice_radio_tech_cdma_in_service(t, QMI_node, RIL_node, TRUE, RADIO_TECH_IS95A, NAS_SYS_SRV_STATUS_SRV_V01, SYS_SRV_DOMAIN_CS_PS_V01, NAS_SYS_ROAM_STATUS_OFF_V01, TRUE);

  // Inject: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_REQ
  OEM_node.update_default_oem_request_set_data_subscription_req(1);
  t.inject(OEM_node);

  t.set_start();
  // Expect: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_RESP
  OEM_node.update_default_oem_request_set_data_subscription_resp();
  t.expect(OEM_node);

  // Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENE_IND
  QMI_node.update_default_qmi_nas_system_selection_preference_ind_msg();
  QMI_node.set_qmi_nas_system_selection_preference_ind_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_PS_V01);
  t.inject(QMI_node);

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(TRUE);
  t.inject(RIL_node);

  // Expect: QMI_EMBMS_UNSOL_EMBMS_STATUS_IND
  OEM_node.update_default_qmi_embms_unsol_embms_status_ind();
  OEM_node.set_qmi_embms_unsol_embms_status(0);
  t.expect(OEM_node);

  //Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  //Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_PS_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_CDMA_HRPD_LTE);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  mode_pref_mask_type_v01 mask = QMI_NAS_RAT_MODE_PREF_LTE + QMI_NAS_RAT_MODE_PREF_HRPD;
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_rat_disabled_mask(mask);
  t.inject(QMI_node);

  // set dual standby pref
  util_ril_utf_set_dual_standby_pref(t, QMI_node, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, QMI_SRV_DOMAIN_PREF_CS_PS_V01, NAS_DDS_DURATION_TEMPORARY_V01);
  t.set_end();

  // DSD IND
  util_ril_utf_dsd_ind_info(t, RIL_node, OEM_node, QMI_node, TRUE, 1, DSD_SYS_NETWORK_3GPP2_V01, DSD_SYS_RAT_EX_3GPP_LTE_V01, 0ull, TRUE, FALSE, TRUE, FALSE, RADIO_TECH_UNKNOWN, FALSE, FALSE);

  // Expect: RIL_REQUEST_ALLOW_DATA_RESP
  t.set_start();
  RIL_node.update_default_ril_request_allow_data_resp();
  t.expect(RIL_node);
  t.set_end();

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_msim_1x_on_demand_indication_first_allow_data_ture_4
@description: for an on-demand sim, receive a CS_PS indication, then enable allow data, set rat_disabled_mask to LTE+HRPD for get_sys_sel_pref, do not remove any mask bit in on demand attach ind
              expecting allow data response without dsd ind needed
==**/
test_result ril_utf_msim_1x_on_demand_indication_first_allow_data_ture_4(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  OEMTestnode OEM_node;

  t.set_test_attribute("AllowData");
  // DSDS tests require one of the msim boot cases
  t.set_boot_case(qmi_ril_msim_bootup_primary_002);

  t.set_test_id(__func__);
  t.set_test_attribute("MSIM");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Enable 1x
  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  util_set_pref_net_type(t, RIL_node, QMI_node, QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA, RADIO_TECH_UNKNOWN, 0, RIL_E_SUCCESS, 0, TRUE, FALSE, FALSE, FALSE);
  util_voice_radio_tech_cdma_in_service(t, QMI_node, RIL_node, TRUE, RADIO_TECH_IS95A, NAS_SYS_SRV_STATUS_SRV_V01, SYS_SRV_DOMAIN_CS_PS_V01, NAS_SYS_ROAM_STATUS_OFF_V01, TRUE);

  // Inject: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_REQ
  OEM_node.update_default_oem_request_set_data_subscription_req(1);
  t.inject(OEM_node);

  t.set_start();
  // Expect: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_RESP
  OEM_node.update_default_oem_request_set_data_subscription_resp();
  t.expect(OEM_node);

  // Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENE_IND
  QMI_node.update_default_qmi_nas_system_selection_preference_ind_msg();
  QMI_node.set_qmi_nas_system_selection_preference_ind_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_PS_V01);
  t.inject(QMI_node);

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(TRUE);
  t.inject(RIL_node);

  // Expect: QMI_EMBMS_UNSOL_EMBMS_STATUS_IND
  OEM_node.update_default_qmi_embms_unsol_embms_status_ind();
  OEM_node.set_qmi_embms_unsol_embms_status(0);
  t.expect(OEM_node);

  //Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  //Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_PS_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_CDMA_HRPD_LTE);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  mode_pref_mask_type_v01 mask = QMI_NAS_RAT_MODE_PREF_LTE + QMI_NAS_RAT_MODE_PREF_HRPD;
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_rat_disabled_mask(mask);
  t.inject(QMI_node);

  // set dual standby pref
  util_ril_utf_set_dual_standby_pref(t, QMI_node, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, QMI_SRV_DOMAIN_PREF_CS_PS_V01, NAS_DDS_DURATION_TEMPORARY_V01);

  // Expect: RIL_REQUEST_ALLOW_DATA_RESP
  RIL_node.update_default_ril_request_allow_data_resp();
  t.expect(RIL_node);
  t.set_end();

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_msim_oos_default_allow_data_false
@description: disable allow data on a SIM with default mode
==**/
test_result ril_utf_msim_oos_default_allow_data_true(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  OEMTestnode OEM_node;

  t.set_test_attribute("AllowData");
  // DSDS tests require one of the msim boot cases
  t.set_boot_case(qmi_ril_msim_bootup_primary_002);

  t.set_test_id(__func__);
  t.set_test_attribute("MSIM");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  util_set_pref_net_type(t, RIL_node, QMI_node, QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA, RADIO_TECH_UNKNOWN, 0, RIL_E_SUCCESS, 0, TRUE, FALSE, FALSE, FALSE);

  // Inject: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_REQ
  OEM_node.update_default_oem_request_set_data_subscription_req(0);
  t.inject(OEM_node);

  t.set_start();
  // Expect: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_RESP
  OEM_node.update_default_oem_request_set_data_subscription_resp();
  t.expect(OEM_node);

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(TRUE);
  t.inject(RIL_node);

  // Expect: QMI_EMBMS_UNSOL_EMBMS_STATUS_IND
  OEM_node.update_default_qmi_embms_unsol_embms_status_ind();
  OEM_node.set_qmi_embms_unsol_embms_status(0);
  t.expect(OEM_node);

  //Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  //Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_ONLY_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_CDMA_HRPD_LTE);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // set dual standby pref
  util_ril_utf_set_dual_standby_pref(t, QMI_node, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, QMI_SRV_DOMAIN_PREF_CS_PS_V01, NAS_DDS_DURATION_PERMANANT_V01);
  t.set_end();

  // DSD IND
  util_ril_utf_dsd_ind_info(t, RIL_node, OEM_node, QMI_node, TRUE, 1, DSD_SYS_NETWORK_3GPP_V01, DSD_SYS_RAT_EX_3GPP_LTE_V01, 0ull, TRUE, FALSE, TRUE, FALSE, RADIO_TECH_LTE, FALSE, FALSE);

  // Expect: RIL_REQUEST_ALLOW_DATA_RESP
  t.set_start();
  RIL_node.update_default_ril_request_allow_data_resp();
  t.expect(RIL_node);
  t.set_end();

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_msim_oos_default_allow_data_false
@description: disable allow data on a SIM with default mode
==**/
test_result ril_utf_msim_oos_default_allow_data_false(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  OEMTestnode OEM_node;

  t.set_test_attribute("AllowData");
  // DSDS tests require one of the msim boot cases
  t.set_boot_case(qmi_ril_msim_bootup_primary_002);

  t.set_test_id(__func__);
  t.set_test_attribute("MSIM");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }
  // Enable 1x
  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  util_set_pref_net_type(t, RIL_node, QMI_node, QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA, RADIO_TECH_UNKNOWN, 0, RIL_E_SUCCESS, 0, TRUE, FALSE, FALSE, FALSE);

  // Inject: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_REQ
  OEM_node.update_default_oem_request_set_data_subscription_req(0);
  t.inject(OEM_node);

  t.set_start();
  // Expect: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_RESP
  OEM_node.update_default_oem_request_set_data_subscription_resp();
  t.expect(OEM_node);

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(FALSE);
  t.inject(RIL_node);

  // Expect: QMI_EMBMS_UNSOL_EMBMS_STATUS_IND
  OEM_node.update_default_qmi_embms_unsol_embms_status_ind();
  OEM_node.set_qmi_embms_unsol_embms_status(0);
  t.expect(OEM_node);

  // Expect: RIL_REQUEST_ALLOW_DATA_RESP
  RIL_node.update_default_ril_request_allow_data_resp();
  t.expect(RIL_node);
  t.set_end();

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_msim_oos_on_demand_allow_data_true
@description: enable allow data on a SIM with on-demand mode
==**/
test_result ril_utf_msim_oos_on_demand_allow_data_true(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  OEMTestnode OEM_node;

  t.set_test_attribute("AllowData");
  // DSDS tests require one of the msim boot cases
  t.set_boot_case(qmi_ril_msim_bootup_primary_002);

  t.set_test_id(__func__);
  t.set_test_attribute("MSIM");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  util_set_pref_net_type(t, RIL_node, QMI_node, QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA, RADIO_TECH_UNKNOWN, 0, RIL_E_SUCCESS, 0, TRUE, FALSE, FALSE, FALSE);

  // Inject: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_REQ
  OEM_node.update_default_oem_request_set_data_subscription_req(1);
  t.inject(OEM_node);

  t.set_start();
  // Expect: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_RESP
  OEM_node.update_default_oem_request_set_data_subscription_resp();
  t.expect(OEM_node);

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(TRUE);
  t.inject(RIL_node);

  // Expect: QMI_EMBMS_UNSOL_EMBMS_STATUS_IND
  OEM_node.update_default_qmi_embms_unsol_embms_status_ind();
  OEM_node.set_qmi_embms_unsol_embms_status(0);
  t.expect(OEM_node);

  //Expect: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ
  QMI_node.update_default_qmi_nas_get_system_selection_preference_req_msg();
  t.expect(QMI_node);

  //Inject: QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_RESP
  QMI_node.update_default_qmi_nas_get_system_selection_preference_resp_msg();
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_srv_domain_pref(QMI_SRV_DOMAIN_PREF_CS_ONLY_V01);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_mode_pref(QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_CDMA_HRPD_LTE);
  QMI_node.set_qmi_nas_get_system_selection_preference_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // set dual standby pref
  util_ril_utf_set_dual_standby_pref(t, QMI_node, QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01, QMI_SRV_DOMAIN_PREF_CS_PS_V01, NAS_DDS_DURATION_TEMPORARY_V01);
  t.set_end();

  // DSD IND
  util_ril_utf_dsd_ind_info(t, RIL_node, OEM_node, QMI_node, TRUE, 1, DSD_SYS_NETWORK_3GPP_V01, DSD_SYS_RAT_EX_3GPP_LTE_V01, 0ull, TRUE, FALSE, TRUE, FALSE, RADIO_TECH_LTE, FALSE, FALSE);

  // Expect: RIL_REQUEST_ALLOW_DATA_RESP
  t.set_start();
  RIL_node.update_default_ril_request_allow_data_resp();
  t.expect(RIL_node);
  t.set_end();

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_msim_oos_on_demand_allow_data_false
@description: disable allow data on a SIM with on-demand mode
==**/
test_result ril_utf_msim_oos_on_demand_allow_data_false(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  AndroidTelTestnode RIL_node;
  OEMTestnode OEM_node;

  t.set_test_attribute("AllowData");
  // DSDS tests require one of the msim boot cases
  t.set_boot_case(qmi_ril_msim_bootup_primary_002);

  t.set_test_id(__func__);
  t.set_test_attribute("MSIM");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  util_voice_radio_tech_oos(t, QMI_node, RIL_node, TRUE, RADIO_TECH_UNKNOWN);
  util_set_pref_net_type(t, RIL_node, QMI_node, QCRIL_PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA, RADIO_TECH_UNKNOWN, 0, RIL_E_SUCCESS, 0, TRUE, FALSE, FALSE, FALSE);

  // Inject: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_REQ
  OEM_node.update_default_oem_request_set_data_subscription_req(1);
  t.inject(OEM_node);

  t.set_start();
  // Expect: QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION_RESP
  OEM_node.update_default_oem_request_set_data_subscription_resp();
  t.expect(OEM_node);

  // Inject: RIL_REQUEST_ALLOW_DATA_REQ
  RIL_node.update_default_ril_request_allow_data_req(FALSE);
  t.inject(RIL_node);

  // Expect: QMI_EMBMS_UNSOL_EMBMS_STATUS_IND
  OEM_node.update_default_qmi_embms_unsol_embms_status_ind();
  OEM_node.set_qmi_embms_unsol_embms_status(0);
  t.expect(OEM_node);

  // Expect: RIL_REQUEST_ALLOW_DATA_RESP
  RIL_node.update_default_ril_request_allow_data_resp();
  t.expect(RIL_node);
  t.set_end();

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_sups_notification_ind_L
@description: Dial an IMS call. Update ATEL with call_id received in an IMS_sups_notification indication.
==**/
test_result ril_utf_ims_sups_notification_ind_L(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id("ril_utf_ims_sups_notification_ind_L");
  t.set_test_description("Dial an IMS call. Update ATEL with call_id received in an IMS_sups_notification indication.");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_expect_skip_verification_status(false);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_SUPS_NOTIFICATION_IND
  int i=0;
  uint16_t ip_hist_info[512] = { 0x3c, 0x73, 0x69, 0x70, 0x3a, 0x2b, 0x34, 0x39, 0x31, 0x35, 0x31, 0x35, 0x39, 0x37, 0x30, 0x38, 0x32, 0x34, 0x32, 0x40, 0x69, 0x6d, 0x73, 0x2e, 0x74, 0x65, 0x73, 0x74, 0x2e, 0x74, 0x2d, 0x6d, 0x6f, 0x62, 0x69, 0x6c, 0x65, 0x2e, 0x64, 0x65, 0x3b, 0x75, 0x73, 0x65, 0x72, 0x3d, 0x70, 0x68, 0x6f, 0x6e, 0x65, 0x3e, 0x3b, 0x69, 0x6e, 0x64, 0x65, 0x78, 0x3d, 0x31, 0x3c, 0x73, 0x69, 0x70, 0x3a, 0x2b, 0x34, 0x39, 0x31, 0x35, 0x31, 0x35, 0x39, 0x37, 0x30, 0x38, 0x32, 0x31, 0x37, 0x40, 0x64, 0x74, 0x61, 0x67, 0x2e, 0x64, 0x65, 0x3b, 0x75, 0x73, 0x65, 0x72, 0x3d, 0x70, 0x68, 0x6f, 0x6e, 0x65, 0x3b, 0x63, 0x61, 0x75, 0x73, 0x65, 0x3d, 0x33, 0x30, 0x32, 0x3e, 0x3b, 0x69, 0x6e, 0x64, 0x65, 0x78, 0x3d, 0x31, 0x2e, 0x31, };
  QMI_node.update_default_qmi_voice_sups_notification_ind_msg(1, NOTIFICATION_TYPE_OUTGOING_CALL_IS_FORWARDED_V02);
  uint16_t *hist_info = QMI_node.set_qmi_voice_sups_notification_ind_history_info(238);
  for(i=0; i<238; i++)
    hist_info[i] = ip_hist_info[i];
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_SUPP_SVC_NOTIFICATION
  char tel_hist_info[256] = {"<sip:+4915159708242@ims.test.t-mobile.de;user=phone>;index=1<sip:+4915159708217@dtag.de;user=phone;cause=302>;index=1.1"};
  IMS_node.update_default_ims_unsol_supp_svc_notification();
  //IMS_node.set_ims_unsol_supp_svc_notification_number(NULL);
  IMS_node.set_ims_unsol_supp_svc_notification_notificationtype(ims_NotificationType_MO); // Value: 0.
  IMS_node.set_ims_unsol_supp_svc_notification_code(2);
  IMS_node.set_ims_unsol_supp_svc_notification_connid(1);
  //IMS_node.set_ims_unsol_supp_svc_notification_index(0);
  //IMS_node.set_ims_unsol_supp_svc_notification_type(0);
  IMS_node.set_ims_unsol_supp_svc_notification_history_info(tel_hist_info);
  t.expect(IMS_node);

  // Execute test
  t.execute();
  t.print_summary();

  return t.get_test_result();

} // ril_utf_ims_sups_notification_ind_L

void set_qmi_voice_all_call_status_ind_default_voip_call(
        QMIRILTestnode         &QMI_node,
        int                     index,
        uint8_t                 call_id,
        call_state_enum_v02     call_state,
        call_direction_enum_v02 direction,
        uint8_t                 is_mpty,
        const char             *number)
{
  voice_all_call_status_ind_msg_v02 *ptr = (voice_all_call_status_ind_msg_v02 *)QMI_node.get_payload();

  if (ptr->call_info_len >= index)
  {
    voice_call_info2_type_v02 *call_info = &(ptr->call_info[index]);
    call_info->call_id = call_id;
    call_info->call_state = call_state;
    call_info->call_type = CALL_TYPE_VOICE_IP_V02;
    call_info->direction = direction;
    call_info->is_mpty = is_mpty;
    call_info->mode = CALL_MODE_LTE_V02;
  }
  if (ptr->remote_party_number_len >= index)
  {
    // remote_party_number
    voice_remote_party_number2_type_v02 *number_list = &(ptr->remote_party_number[index]);
    number_list->call_id = call_id;
    number_list->number_len = strlen(number);
    strlcpy(number_list->number, number, sizeof(number_list->number));
    number_list->number_pi = PRESENTATION_NUM_ALLOWED_V02;
  }
}

void set_ims_unsol_response_call_state_changed_default_voip_call(
        IMSTestnode          &IMS_node,
        ims_CallList_Call **calls,
        int                   i,
        uint32_t              call_id,
        ims_CallState        state,
        boolean               ismt,
        boolean               ismpty,
        int32_t               mediaid)
{
  calls[i]->has_index = TRUE;
  calls[i]->index = call_id;
  calls[i]->has_state = TRUE;
  calls[i]->state = state;
  calls[i]->has_toa = TRUE;
  calls[i]->toa = 129;
  calls[i]->has_isMpty = TRUE;
  calls[i]->isMpty = ismpty;
  calls[i]->has_isMT = TRUE;
  calls[i]->isMT = ismt;
  calls[i]->has_als = TRUE;
  calls[i]->has_isVoice = TRUE;
  calls[i]->isVoice = 1;
  calls[i]->has_isVoicePrivacy = TRUE;
  calls[i]->isVoicePrivacy = 0;
  calls[i]->callDetails.has_callDomain = TRUE;
  calls[i]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[i]->callDetails.has_callType = TRUE;
  calls[i]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[i]->callDetails.has_mediaId = TRUE;
  calls[i]->callDetails.mediaId = mediaid;
}


/**==
@testname: ril_utf_ims_mt_reject_cases
@description: User reject test cases for INCOMING, WAITING and 3rd INCOMING calls
==**/
test_result ril_utf_ims_mt_reject_cases(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id("ril_utf_ims_mt_reject_cases");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // ----------------------------------------
  // 1. INCOMING call
  // Reject INCOMING call
  // ----------------------------------------

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 1, CALL_STATE_INCOMING_V02, CALL_DIRECTION_MT_V02, FALSE, "2428536960");
  // Media ID
  voice_media_id_with_id_type_v02 *media_id = QMI_node.set_qmi_voice_all_call_status_ind_media_id(1);
  media_id[0].call_id = 1;
  media_id[0].media_id = 1;
  t.inject(QMI_node);


  // Expect: ims_MsgId_UNSOL_CALL_RING
  IMS_node.update_default_ims_unsol_call_ring();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 1, ims_CallState_CALL_INCOMING, TRUE, FALSE, 1);
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  ims_Hangup *hangup_ptr = IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(1);
  hangup_ptr->has_failCauseResponse = TRUE;
  hangup_ptr->failCauseResponse.has_failcause = TRUE;
  hangup_ptr->failCauseResponse.failcause = ims_CallFailCause_CALL_FAIL_USER_REJECT;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_ANSWER_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_answer_call_req_msg(1);
  QMI_node.set_qmi_voice_answer_call_req_reject_call(1);
  QMI_node.set_qmi_voice_answer_call_req_reject_cause(VOICE_REJECT_CAUSE_USER_BUSY_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_ANSWER_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_answer_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 1, CALL_STATE_END_V02, CALL_DIRECTION_MT_V02, FALSE, "2428536960");
  voice_call_end_reason_type_v02* end = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end[0].call_id = 1;
  end[0].call_end_reason = CALL_END_CAUSE_NORMAL_CALL_CLEARING_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 1, ims_CallState_CALL_END, TRUE, FALSE, 1);
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  t.expect(IMS_node);

  // ----------------------------------------
  // 2. INCOMING call
  // Answer INCOMING call
  // ----------------------------------------

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 1, CALL_STATE_INCOMING_V02, CALL_DIRECTION_MT_V02, FALSE, "2428536960");
  // Media ID
  media_id = QMI_node.set_qmi_voice_all_call_status_ind_media_id(1);
  media_id[0].call_id = 1;
  media_id[0].media_id = 1;
  t.inject(QMI_node);


  // Expect: ims_MsgId_UNSOL_CALL_RING
  IMS_node.update_default_ims_unsol_call_ring();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 1, ims_CallState_CALL_INCOMING, TRUE, FALSE, 1);
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_ANSWER
  IMS_node.update_default_ims_request_answer();
  t.inject(IMS_node);

  // Expect: QMI_VOICE_ANSWER_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_answer_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_ANSWER_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_answer_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 1, CALL_STATE_CONVERSATION_V02, CALL_DIRECTION_MT_V02, FALSE, "2428536960");
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_ANSWER (resp)
  IMS_node.update_default_ims_request_answer_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 1, ims_CallState_CALL_ACTIVE, TRUE, FALSE, 1);
  t.expect(IMS_node);

  // ----------------------------------------
  // 3. WAITING call
  // Reject WAITING call
  // ----------------------------------------

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 1, CALL_STATE_CONVERSATION_V02, CALL_DIRECTION_MT_V02, FALSE, "2428536960");
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 1, 2, CALL_STATE_WAITING_V02, CALL_DIRECTION_MT_V02, FALSE, "242850000");
  // Media ID
  media_id = QMI_node.set_qmi_voice_all_call_status_ind_media_id(2);
  media_id[1].call_id = 2;
  media_id[1].media_id = 0;
  t.inject(QMI_node);


  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 2, ims_CallState_CALL_WAITING, TRUE, FALSE, 0);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 1, 1, ims_CallState_CALL_ACTIVE, TRUE, FALSE, 1);
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  hangup_ptr = IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(2);
  hangup_ptr->has_failCauseResponse = TRUE;
  hangup_ptr->failCauseResponse.has_failcause = TRUE;
  hangup_ptr->failCauseResponse.failcause = ims_CallFailCause_CALL_FAIL_USER_REJECT;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_ANSWER_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_answer_call_req_msg(2);
  QMI_node.set_qmi_voice_answer_call_req_reject_call(1);
  QMI_node.set_qmi_voice_answer_call_req_reject_cause(VOICE_REJECT_CAUSE_USER_BUSY_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_ANSWER_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_answer_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 1, CALL_STATE_CONVERSATION_V02, CALL_DIRECTION_MT_V02, FALSE, "2428536960");
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 1, 2, CALL_STATE_END_V02, CALL_DIRECTION_MT_V02, FALSE, "242850000");
  end = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end[0].call_id = 2;
  end[0].call_end_reason = CALL_END_CAUSE_NORMAL_CALL_CLEARING_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);


  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 2, ims_CallState_CALL_END, TRUE, FALSE, 0);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 1, 1, ims_CallState_CALL_ACTIVE, TRUE, FALSE, 1);
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  t.expect(IMS_node);

  // ----------------------------------------
  // 4. WAITING call
  // HOLD current ACTIVE call & ANSWER WAITING call
  // ----------------------------------------

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 1, CALL_STATE_CONVERSATION_V02, CALL_DIRECTION_MT_V02, FALSE, "2428536960");
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 1, 2, CALL_STATE_WAITING_V02, CALL_DIRECTION_MT_V02, FALSE, "242850000");
  // Media ID
  media_id = QMI_node.set_qmi_voice_all_call_status_ind_media_id(2);
  media_id[1].call_id = 2;
  media_id[1].media_id = 0;
  t.inject(QMI_node);


  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 2, ims_CallState_CALL_WAITING, TRUE, FALSE, 0);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 1, 1, ims_CallState_CALL_ACTIVE, TRUE, FALSE, 1);
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HOLD
  IMS_node.update_default_ims_request_hold(1);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_MANAGE_IP_CALLS_REQ_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_req_msg((voip_sups_type_enum_v02)VOIP_SUPS_TYPE_CALL_HOLD_V02);
  QMI_node.set_qmi_voice_manage_ip_calls_req_call_id(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_MANAGE_IP_CALLS_RESP_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject: ims_MsgId_REQUEST_HOLD (Resp)
  IMS_node.update_default_ims_request_hold_resp();
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_ANSWER
  IMS_node.update_default_ims_request_answer();
  t.inject(IMS_node);

  // Expect: QMI_VOICE_ANSWER_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_answer_call_req_msg(2);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_ANSWER_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_answer_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 1, CALL_STATE_HOLD_V02, CALL_DIRECTION_MT_V02, FALSE, "2428536960");
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 1, 2, CALL_STATE_CONVERSATION_V02, CALL_DIRECTION_MT_V02, FALSE, "242850000");
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_ANSWER (resp)
  IMS_node.update_default_ims_request_answer_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 2, ims_CallState_CALL_ACTIVE, TRUE, FALSE, 0);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 1, 1, ims_CallState_CALL_HOLDING, TRUE, FALSE, 1);
  t.expect(IMS_node);

  // ----------------------------------------
  // 5. Third INCOMING call
  // Reject waiting call
  // ----------------------------------------

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  QMI_node.update_default_qmi_voice_all_call_status_ind_msg(3);
  QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(3);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 1, CALL_STATE_HOLD_V02, CALL_DIRECTION_MT_V02, FALSE, "2428536960");
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 1, 2, CALL_STATE_CONVERSATION_V02, CALL_DIRECTION_MT_V02, FALSE, "242850000");
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 2, 3, CALL_STATE_WAITING_V02, CALL_DIRECTION_MT_V02, FALSE, "242866666");
  // Media ID
  media_id = QMI_node.set_qmi_voice_all_call_status_ind_media_id(3);
  media_id[2].call_id = 3;
  media_id[2].media_id = 10;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(3);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 3, ims_CallState_CALL_WAITING, TRUE, FALSE, 10);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 1, 2, ims_CallState_CALL_ACTIVE, TRUE, FALSE, 0);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 2, 1, ims_CallState_CALL_HOLDING, TRUE, FALSE, 1);
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  hangup_ptr = IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(3);
  hangup_ptr->has_failCauseResponse = TRUE;
  hangup_ptr->failCauseResponse.has_failcause = TRUE;
  hangup_ptr->failCauseResponse.failcause = ims_CallFailCause_CALL_FAIL_USER_REJECT;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_ANSWER_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_answer_call_req_msg(3);
  QMI_node.set_qmi_voice_answer_call_req_reject_call(1);
  QMI_node.set_qmi_voice_answer_call_req_reject_cause(VOICE_REJECT_CAUSE_USER_BUSY_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_ANSWER_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_answer_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  QMI_node.update_default_qmi_voice_all_call_status_ind_msg(3);
  QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(3);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 1, CALL_STATE_HOLD_V02, CALL_DIRECTION_MT_V02, FALSE, "2428536960");
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 1, 2, CALL_STATE_CONVERSATION_V02, CALL_DIRECTION_MT_V02, FALSE, "242850000");
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 2, 3, CALL_STATE_END_V02, CALL_DIRECTION_MT_V02, FALSE, "242866666");
  end = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end[0].call_id = 3;
  end[0].call_end_reason = CALL_END_CAUSE_NORMAL_CALL_CLEARING_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(3);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 3, ims_CallState_CALL_END, TRUE, FALSE, 10);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 1, 2, ims_CallState_CALL_ACTIVE, TRUE, FALSE, 0);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 2, 1, ims_CallState_CALL_HOLDING, TRUE, FALSE, 1);
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  t.expect(IMS_node);

  // ----------------------------------------
  // 6. Third INCOMING call
  // HANGUP ACTIVE call & ANSWER waiting call
  // ----------------------------------------

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  QMI_node.update_default_qmi_voice_all_call_status_ind_msg(3);
  QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(3);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 1, CALL_STATE_HOLD_V02, CALL_DIRECTION_MT_V02, FALSE, "2428536960");
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 1, 2, CALL_STATE_CONVERSATION_V02, CALL_DIRECTION_MT_V02, FALSE, "242850000");
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 2, 3, CALL_STATE_WAITING_V02, CALL_DIRECTION_MT_V02, FALSE, "242866666");
  // Media ID
  media_id = QMI_node.set_qmi_voice_all_call_status_ind_media_id(3);
  media_id[2].call_id = 3;
  media_id[2].media_id = 10;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(3);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 3, ims_CallState_CALL_WAITING, TRUE, FALSE, 10);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 1, 2, ims_CallState_CALL_ACTIVE, TRUE, FALSE, 0);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 2, 1, ims_CallState_CALL_HOLDING, TRUE, FALSE, 1);
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  hangup_ptr = IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(2);
  hangup_ptr->has_failCauseResponse = TRUE;
  hangup_ptr->failCauseResponse.has_failcause = TRUE;
  hangup_ptr->failCauseResponse.failcause = ims_CallFailCause_CALL_FAIL_USER_REJECT;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(2);
  QMI_node.set_qmi_voice_end_call_req_end_cause(VOICE_REJECT_CAUSE_USER_BUSY_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(2);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  QMI_node.update_default_qmi_voice_all_call_status_ind_msg(3);
  QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(3);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 1, CALL_STATE_HOLD_V02, CALL_DIRECTION_MT_V02, FALSE, "2428536960");
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 1, 2, CALL_STATE_END_V02, CALL_DIRECTION_MT_V02, FALSE, "242850000");
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 2, 3, CALL_STATE_WAITING_V02, CALL_DIRECTION_MT_V02, FALSE, "242866666");
  end = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end[0].call_id = 2;
  end[0].call_end_reason = CALL_END_CAUSE_NORMAL_CALL_CLEARING_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(3);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 3, ims_CallState_CALL_WAITING, TRUE, FALSE, 10);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 1, 2, ims_CallState_CALL_END, TRUE, FALSE, 0);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 2, 1, ims_CallState_CALL_HOLDING, TRUE, FALSE, 1);
  calls[1]->has_failCause = TRUE;
  calls[1]->failCause.has_failcause = TRUE;
  calls[1]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_ANSWER
  IMS_node.update_default_ims_request_answer();
  t.inject(IMS_node);

  // Expect: QMI_VOICE_ANSWER_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_answer_call_req_msg(3);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_ANSWER_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_answer_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 1, CALL_STATE_HOLD_V02, CALL_DIRECTION_MT_V02, FALSE, "2428536960");
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 1, 3, CALL_STATE_CONVERSATION_V02, CALL_DIRECTION_MT_V02, FALSE, "242866666");
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_ANSWER (resp)
  IMS_node.update_default_ims_request_answer_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 3, ims_CallState_CALL_ACTIVE, TRUE, FALSE, 10);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 1, 1, ims_CallState_CALL_HOLDING, TRUE, FALSE, 1);
  t.expect(IMS_node);

  // ----------------------------------------
  // 7. Third INCOMING call
  // HANGUP HELD call, HOLD ACTIVE call & ANSWER waiting call
  // ----------------------------------------

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  QMI_node.update_default_qmi_voice_all_call_status_ind_msg(3);
  QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(3);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 1, CALL_STATE_HOLD_V02, CALL_DIRECTION_MT_V02, FALSE, "2428536960");
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 1, 2, CALL_STATE_WAITING_V02, CALL_DIRECTION_MT_V02, FALSE, "242877777");
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 2, 3, CALL_STATE_CONVERSATION_V02, CALL_DIRECTION_MT_V02, FALSE, "242866666");
  // Media ID
  media_id = QMI_node.set_qmi_voice_all_call_status_ind_media_id(3);
  media_id[1].call_id = 2;
  media_id[1].media_id = 100;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(3);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 2, ims_CallState_CALL_WAITING, TRUE, FALSE, 100);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 1, 3, ims_CallState_CALL_ACTIVE, TRUE, FALSE, 10);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 2, 1, ims_CallState_CALL_HOLDING, TRUE, FALSE, 1);
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  hangup_ptr = IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(1);
  hangup_ptr->has_failCauseResponse = TRUE;
  hangup_ptr->failCauseResponse.has_failcause = TRUE;
  hangup_ptr->failCauseResponse.failcause = ims_CallFailCause_CALL_FAIL_USER_REJECT;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(1);
  QMI_node.set_qmi_voice_end_call_req_end_cause(VOICE_REJECT_CAUSE_USER_BUSY_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  QMI_node.update_default_qmi_voice_all_call_status_ind_msg(3);
  QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(3);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 1, CALL_STATE_END_V02, CALL_DIRECTION_MT_V02, FALSE, "2428536960");
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 1, 2, CALL_STATE_WAITING_V02, CALL_DIRECTION_MT_V02, FALSE, "242877777");
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 2, 3, CALL_STATE_CONVERSATION_V02, CALL_DIRECTION_MT_V02, FALSE, "242866666");
  end = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end[0].call_id = 1;
  end[0].call_end_reason = CALL_END_CAUSE_NORMAL_CALL_CLEARING_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(3);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 2, ims_CallState_CALL_WAITING, TRUE, FALSE, 100);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 1, 3, ims_CallState_CALL_ACTIVE, TRUE, FALSE, 10);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 2, 1, ims_CallState_CALL_END, TRUE, FALSE, 1);
  calls[2]->has_failCause = TRUE;
  calls[2]->failCause.has_failcause = TRUE;
  calls[2]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HOLD
  IMS_node.update_default_ims_request_hold(3);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_MANAGE_IP_CALLS_REQ_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_req_msg((voip_sups_type_enum_v02)VOIP_SUPS_TYPE_CALL_HOLD_V02);
  QMI_node.set_qmi_voice_manage_ip_calls_req_call_id(3);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_MANAGE_IP_CALLS_RESP_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject: ims_MsgId_REQUEST_HOLD (Resp)
  IMS_node.update_default_ims_request_hold_resp();
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 2, CALL_STATE_WAITING_V02, CALL_DIRECTION_MT_V02, FALSE, "242877777");
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 1, 3, CALL_STATE_HOLD_V02, CALL_DIRECTION_MT_V02, FALSE, "242866666");
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 2, ims_CallState_CALL_WAITING, TRUE, FALSE, 100);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 1, 3, ims_CallState_CALL_HOLDING, TRUE, FALSE, 10);
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_ANSWER
  IMS_node.update_default_ims_request_answer();
  t.inject(IMS_node);

  // Expect: QMI_VOICE_ANSWER_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_answer_call_req_msg(2);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_ANSWER_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_answer_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  QMI_node.update_default_qmi_voice_all_call_status_ind_msg(2);
  QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(2);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 2, CALL_STATE_CONVERSATION_V02, CALL_DIRECTION_MT_V02, FALSE, "242877777");
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 1, 3, CALL_STATE_HOLD_V02, CALL_DIRECTION_MT_V02, FALSE, "242866666");
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_ANSWER (resp)
  IMS_node.update_default_ims_request_answer_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(2);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 2, ims_CallState_CALL_ACTIVE, TRUE, FALSE, 100);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 1, 3, ims_CallState_CALL_HOLDING, TRUE, FALSE, 10);
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_mo_reject_low_battery
@description: User reject test cases for INCOMING call with LOW_BATTERY
==**/
test_result ril_utf_ims_mo_reject_low_battery(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id("ril_utf_ims_mo_reject_low_battery");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // ----------------------------------------
  // Hangup ALERTING call with LOW_BATTERY
  // ----------------------------------------

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("9876543210");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("9876543210");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  QMI_node.set_qmi_voice_dial_call_resp_media_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  // remote_party_number
  QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 1, CALL_STATE_ORIGINATING_V02, CALL_DIRECTION_MO_V02, FALSE, "9876543210");
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call **calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 1, ims_CallState_CALL_DIALING, FALSE, FALSE, 1);
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  // remote_party_number
  QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 1, CALL_STATE_ALERTING_V02, CALL_DIRECTION_MO_V02, FALSE, "9876543210");
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 1, ims_CallState_CALL_ALERTING, FALSE, FALSE, 1);
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  ims_Hangup *hangup_ptr = IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(1);
  hangup_ptr->has_failCauseResponse = TRUE;
  hangup_ptr->failCauseResponse.has_failcause = TRUE;
  hangup_ptr->failCauseResponse.failcause = ims_CallFailCause_CALL_FAIL_LOW_BATTERY;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(1);
  QMI_node.set_qmi_voice_end_call_req_end_cause(VOICE_REJECT_CAUSE_LOW_BATTERY_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  // remote_party_number
  QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 1, CALL_STATE_END_V02, CALL_DIRECTION_MO_V02, FALSE, "9876543210");
  voice_call_end_reason_type_v02* end = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end[0].call_id = 1;
  end[0].call_end_reason = CALL_END_CAUSE_FALLBACK_TO_CS_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 1, ims_CallState_CALL_END, FALSE, FALSE, 1);
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_CS_RETRY_REQUIRED;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_mt_reject_low_battery
@description: User reject test cases for INCOMING call with LOW_BATTERY
==**/
test_result ril_utf_ims_mt_reject_low_battery(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id("ril_utf_ims_mt_reject_low_battery");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // ----------------------------------------
  // Reject INCOMING call with LOW_BATTERY
  // ----------------------------------------

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 1, CALL_STATE_INCOMING_V02, CALL_DIRECTION_MT_V02, FALSE, "2428536960");
  // Media ID
  voice_media_id_with_id_type_v02 *media_id = QMI_node.set_qmi_voice_all_call_status_ind_media_id(1);
  media_id[0].call_id = 1;
  media_id[0].media_id = 1;
  t.inject(QMI_node);


  // Expect: ims_MsgId_UNSOL_CALL_RING
  IMS_node.update_default_ims_unsol_call_ring();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 1, ims_CallState_CALL_INCOMING, TRUE, FALSE, 1);
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  ims_Hangup *hangup_ptr = IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(1);
  hangup_ptr->has_failCauseResponse = TRUE;
  hangup_ptr->failCauseResponse.has_failcause = TRUE;
  hangup_ptr->failCauseResponse.failcause = ims_CallFailCause_CALL_FAIL_LOW_BATTERY;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_ANSWER_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_answer_call_req_msg(1);
  QMI_node.set_qmi_voice_answer_call_req_reject_call(1);
  QMI_node.set_qmi_voice_answer_call_req_reject_cause(VOICE_REJECT_CAUSE_LOW_BATTERY_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_ANSWER_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_answer_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 1, CALL_STATE_END_V02, CALL_DIRECTION_MT_V02, FALSE, "2428536960");
  voice_call_end_reason_type_v02* end = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end[0].call_id = 1;
  end[0].call_end_reason = CALL_END_CAUSE_NORMAL_CALL_CLEARING_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 1, ims_CallState_CALL_END, TRUE, FALSE, 1);
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}


/**==
@testname: ril_utf_ims_mo_call_end_fallback_to_cs
@description: MO call getting ENDed with end reason FALLBACK_TO_CS
==**/
test_result ril_utf_ims_mo_call_end_fallback_to_cs(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id("ril_utf_ims_mo_call_end_fallback_to_cs");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("9876543210");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("9876543210");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  QMI_node.set_qmi_voice_dial_call_resp_media_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  // remote_party_number
  QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 1, CALL_STATE_ORIGINATING_V02, CALL_DIRECTION_MO_V02, FALSE, "9876543210");
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call **calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 1, ims_CallState_CALL_DIALING, FALSE, FALSE, 1);
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  // remote_party_number
  QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 1, CALL_STATE_ALERTING_V02, CALL_DIRECTION_MO_V02, FALSE, "9876543210");
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 1, ims_CallState_CALL_ALERTING, FALSE, FALSE, 1);
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  // remote_party_number
  QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 1, CALL_STATE_END_V02, CALL_DIRECTION_MO_V02, FALSE, "9876543210");
  voice_call_end_reason_type_v02* end = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end[0].call_id = 1;
  end[0].call_end_reason = CALL_END_CAUSE_FALLBACK_TO_CS_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 1, ims_CallState_CALL_END, FALSE, FALSE, 1);
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_CS_RETRY_REQUIRED;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_mo_call_end_low_battery
@description: MO call getting ENDed with end reason LOW_BATTERY
==**/
test_result ril_utf_ims_mo_call_end_low_battery(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id("ril_utf_ims_mo_call_end_low_battery");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("9876543210");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("9876543210");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  QMI_node.set_qmi_voice_dial_call_resp_media_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  // remote_party_number
  QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 1, CALL_STATE_ORIGINATING_V02, CALL_DIRECTION_MO_V02, FALSE, "9876543210");
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call **calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 1, ims_CallState_CALL_DIALING, FALSE, FALSE, 1);
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  // remote_party_number
  QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 1, CALL_STATE_ALERTING_V02, CALL_DIRECTION_MO_V02, FALSE, "9876543210");
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 1, ims_CallState_CALL_ALERTING, FALSE, FALSE, 1);
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  // remote_party_number
  QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 1, CALL_STATE_END_V02, CALL_DIRECTION_MO_V02, FALSE, "9876543210");
  voice_call_end_reason_type_v02* end = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end[0].call_id = 1;
  end[0].call_end_reason = CALL_END_CAUSE_LOW_BATTERY_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 1, ims_CallState_CALL_END, FALSE, FALSE, 1);
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_LOW_BATTERY;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}


/**==
@testname: ril_utf_ims_mo_call_end_dead_battery
@description: MO call getting ENDed with end reason DEAD_BATTERY
==**/
test_result ril_utf_ims_mo_call_end_dead_battery(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id("ril_utf_ims_mo_call_end_dead_battery");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("9876543210");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("9876543210");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  QMI_node.set_qmi_voice_dial_call_resp_media_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  // remote_party_number
  QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 1, CALL_STATE_ORIGINATING_V02, CALL_DIRECTION_MO_V02, FALSE, "9876543210");
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call **calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 1, ims_CallState_CALL_DIALING, FALSE, FALSE, 1);
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  // remote_party_number
  QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 1, CALL_STATE_ALERTING_V02, CALL_DIRECTION_MO_V02, FALSE, "9876543210");
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 1, ims_CallState_CALL_ALERTING, FALSE, FALSE, 1);
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  // remote_party_number
  QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  set_qmi_voice_all_call_status_ind_default_voip_call(QMI_node, 0, 1, CALL_STATE_END_V02, CALL_DIRECTION_MO_V02, FALSE, "9876543210");
  voice_call_end_reason_type_v02* end = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end[0].call_id = 1;
  end[0].call_end_reason = CALL_END_CAUSE_DEAD_BATTERY_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  set_ims_unsol_response_call_state_changed_default_voip_call(IMS_node, calls, 0, 1, ims_CallState_CALL_END, FALSE, FALSE, 1);
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_LOW_BATTERY;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

static test_result ril_utf_ims_mo_precond_template_L
(
 user_specified_attrib_t attr,
 const char *test_id,
 ims_CallType initiate_as_call_type,
 ims_CallType connect_as_call_type,
 boolean call_attrib_status_valid,
 voice_call_attrib_status_enum_v02 call_attrib_status
)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;
  voice_call_attributes_type_v02 *video_attrib = NULL;
  voice_call_attrib_status_type_v02 *call_attrib_status_ptr = NULL;
  ims_CallList_Call** calls = NULL;
  call_type_enum_v02 initiate_modem_call_type;
  call_type_enum_v02 connect_modem_call_type;
  ims_CallSubstate callsubstate;

  t.set_test_id(test_id);
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }
  if (initiate_as_call_type == ims_CallType_CALL_TYPE_VT)
  {
    initiate_modem_call_type = CALL_TYPE_VT_V02;
  }
  else
  {
    initiate_modem_call_type = CALL_TYPE_VOICE_IP_V02;
  }
  if (connect_as_call_type == ims_CallType_CALL_TYPE_VT ||
      connect_as_call_type == ims_CallType_CALL_TYPE_VT_TX ||
      connect_as_call_type == ims_CallType_CALL_TYPE_VT_RX)
  {
    connect_modem_call_type = CALL_TYPE_VT_V02;
  }
  else
  {
    connect_modem_call_type = CALL_TYPE_VOICE_IP_V02;
  }

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = initiate_as_call_type;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(initiate_modem_call_type);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  if (initiate_as_call_type == ims_CallType_CALL_TYPE_VT)
  {
    QMI_node.set_qmi_voice_dial_call_req_video_attrib(
            VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  }
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id    = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type  = initiate_modem_call_type;
  call_info[0].direction  = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty    = 0;
  call_info[0].mode       = CALL_MODE_LTE_V02;
  // Remote party number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id    = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi  = PRESENTATION_NUM_ALLOWED_V02;
  // Audio attributes
  voice_call_attributes_type_v02 *audio_attrib = QMI_node.set_qmi_voice_all_call_status_ind_audio_attrib(1);
  audio_attrib[0].call_id         = 1;
  audio_attrib[0].call_attributes = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
  // Video attributes
  if (initiate_as_call_type == ims_CallType_CALL_TYPE_VT)
  {
    video_attrib = QMI_node.set_qmi_voice_all_call_status_ind_video_attrib(1);
    video_attrib[0].call_id         = 1;
    video_attrib[0].call_attributes = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
  }
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index     = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state     = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa     = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->isMpty     = 0;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT     = 0;
  calls[0]->has_als = TRUE;
  calls[0]->als     = 0;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 0;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 2;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain     = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType     = initiate_as_call_type;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate     = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id    = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type  = initiate_modem_call_type;
  call_info[0].direction  = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty    = 0;
  call_info[0].mode       = CALL_MODE_LTE_V02;
  // Remote party number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id    = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi  = PRESENTATION_NUM_ALLOWED_V02;
  // Audio attributes
  audio_attrib = QMI_node.set_qmi_voice_all_call_status_ind_audio_attrib(1);
  audio_attrib[0].call_id         = 1;
  audio_attrib[0].call_attributes = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
  // Video attributes
  if (initiate_as_call_type == ims_CallType_CALL_TYPE_VT)
  {
    video_attrib = QMI_node.set_qmi_voice_all_call_status_ind_video_attrib(1);
    video_attrib[0].call_id         = 1;
    video_attrib[0].call_attributes = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
  }
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index     = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state     = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa     = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->isMpty     = 0;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT     = 0;
  calls[0]->has_als = TRUE;
  calls[0]->als     = 0;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 0;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 2;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain     = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType     = initiate_as_call_type;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate     = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  // With audio and video attributes set to 0
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id    = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = connect_modem_call_type;
  call_info[0].direction  = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty    = 0;
  call_info[0].mode       = CALL_MODE_LTE_V02;
  // Remote party number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id    = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi  = PRESENTATION_NUM_ALLOWED_V02;
  // Audio attributes
  audio_attrib = QMI_node.set_qmi_voice_all_call_status_ind_audio_attrib(1);
  audio_attrib[0].call_id         = 1;
  audio_attrib[0].call_attributes = call_attrib_status_valid ?
          (VOICE_CALL_ATTRIB_TX_V02|VOICE_CALL_ATTRIB_RX_V02) : 0;
  // Video attributes
  if (connect_modem_call_type == CALL_TYPE_VT_V02)
  {
    video_attrib = QMI_node.set_qmi_voice_all_call_status_ind_video_attrib(1);
    video_attrib[0].call_id         = 1;
    video_attrib[0].call_attributes = 0;
  }
  if (call_attrib_status_valid)
  {
    call_attrib_status_ptr = QMI_node.set_qmi_voice_all_call_status_ind_call_attrib_status(1);
    call_attrib_status_ptr[0].call_id         = 1;
    call_attrib_status_ptr[0].call_attrib_status = call_attrib_status;
  }
  t.inject(QMI_node);

  if (call_attrib_status_valid)
  {
      switch (call_attrib_status)
      {
         case VOICE_CALL_ATTRIB_STATUS_RETRY_NEEDED_V02:
             callsubstate = ims_CallSubstate_CALL_SUBSTATE_AVP_RETRY;
             break;
         case VOICE_CALL_ATTRIB_STATUS_MEDIA_PAUSED_V02:
             callsubstate = ims_CallSubstate_CALL_SUBSTATE_MEDIA_PAUSED;
             break;
         case VOICE_CALL_ATTRIB_STATUS_OK_V02:
             callsubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
             break;
      }
      initiate_as_call_type = ims_CallType_CALL_TYPE_VT_NODIR;
  }
  else if(connect_modem_call_type == CALL_TYPE_VT_V02) //only VT call
  {
      callsubstate = (ims_CallSubstate)(ims_CallSubstate_CALL_SUBSTATE_AUDIO_CONNECTED_SUSPENDED |
                      ims_CallSubstate_CALL_SUBSTATE_VIDEO_CONNECTED_SUSPENDED);
  }
  else //not VT call
  {
      callsubstate = ims_CallSubstate_CALL_SUBSTATE_AUDIO_CONNECTED_SUSPENDED;
  }

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index     = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state     = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa     = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->isMpty     = 0;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT     = 0;
  calls[0]->has_als = TRUE;
  calls[0]->als     = 0;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 0;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 2;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain     = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType     = initiate_as_call_type;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = callsubstate;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();
  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_mo_precond_vt
@description:
MO VT call with CallSubState as AUDIO | VIDEO CONNECTED_SUSPENDED
==**/
test_result ril_utf_ims_mo_precond_vt_L(user_specified_attrib_t attr)
{
  return ril_utf_ims_mo_precond_template_L(attr, "ril_utf_ims_mo_precond_vt_L",
      ims_CallType_CALL_TYPE_VT,
      ims_CallType_CALL_TYPE_VT,
      FALSE, (voice_call_attrib_status_enum_v02)0);
}

/**==
@testname: ril_utf_ims_mo_precond_vt_media_paused_L
@description:
MO VT call with callSubState MEDIA_PAUSED
==**/
test_result ril_utf_ims_mo_precond_vt_media_paused_L(user_specified_attrib_t attr)
{
  return ril_utf_ims_mo_precond_template_L(attr, "ril_utf_ims_mo_precond_vt_media_paused_L",
      ims_CallType_CALL_TYPE_VT,
      ims_CallType_CALL_TYPE_VT,
      TRUE, VOICE_CALL_ATTRIB_STATUS_MEDIA_PAUSED_V02);
}

/**==
@testname: ril_utf_ims_mo_precond_vt_retry_needed_L
@description:
MO VT call with callSubState AVP RETRY
==**/
test_result ril_utf_ims_mo_precond_vt_retry_needed_L(user_specified_attrib_t attr)
{
  return ril_utf_ims_mo_precond_template_L(attr, "ril_utf_ims_mo_precond_vt_retry_needed_L",
      ims_CallType_CALL_TYPE_VT,
      ims_CallType_CALL_TYPE_VT,
      TRUE, VOICE_CALL_ATTRIB_STATUS_RETRY_NEEDED_V02);
}

/**==
@testname: ril_utf_ims_mo_precond_voice_audio_suspended_L
@description:
MO voice call with callSubState AUDIO SUSPENDED
==**/
test_result ril_utf_ims_mo_precond_voice_audio_suspended_L(user_specified_attrib_t attr)
{
  return ril_utf_ims_mo_precond_template_L(attr, "ril_utf_ims_mo_precond_voice_audio_suspended_L",
      ims_CallType_CALL_TYPE_VOICE,
      ims_CallType_CALL_TYPE_VOICE,
      FALSE, (voice_call_attrib_status_enum_v02)0);
}

/**==
@testname: ril_utf_ims_mo_vt_nodir_media_paused_L
@description:
1. MO VT call
2. User initiates Video Pause (VT_NODIR)
3. Returns Call substate MEDIA PAUSED
3. User changes to VT
4. End call

==**/
test_result ril_utf_ims_mo_vt_nodir_media_paused_L(user_specified_attrib_t attr)
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

  // Inject: ims_MsgId_REQUEST_DIAL
  // VT call
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VT_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id    = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type  = CALL_TYPE_VT_V02;
  call_info[0].direction  = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty    = 0;
  call_info[0].mode       = CALL_MODE_LTE_V02;
  // Remote party number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id    = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi  = PRESENTATION_NUM_ALLOWED_V02;
  // Audio attributes
  voice_call_attributes_type_v02 *audio_attrib = QMI_node.set_qmi_voice_all_call_status_ind_audio_attrib(1);
  audio_attrib[0].call_id         = 1;
  audio_attrib[0].call_attributes = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
  // Video attributes
  voice_call_attributes_type_v02 *video_attrib = QMI_node.set_qmi_voice_all_call_status_ind_video_attrib(1);
  video_attrib[0].call_id         = 1;
  video_attrib[0].call_attributes = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index     = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state     = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa     = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->isMpty     = 0;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT     = 0;
  calls[0]->has_als = TRUE;
  calls[0]->als     = 0;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain     = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType     = ims_CallType_CALL_TYPE_VT;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id    = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type  = CALL_TYPE_VT_V02;
  call_info[0].direction  = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty    = 0;
  call_info[0].mode       = CALL_MODE_LTE_V02;
  // Remote party number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id    = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi  = PRESENTATION_NUM_ALLOWED_V02;
  // Audio attributes
  audio_attrib = QMI_node.set_qmi_voice_all_call_status_ind_audio_attrib(1);
  audio_attrib[0].call_id         = 1;
  audio_attrib[0].call_attributes = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
  // Video attributes
  video_attrib = QMI_node.set_qmi_voice_all_call_status_ind_video_attrib(1);
  video_attrib[0].call_id         = 1;
  video_attrib[0].call_attributes = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index     = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state     = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa     = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->isMpty     = 0;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT     = 0;
  calls[0]->has_als = TRUE;
  calls[0]->als     = 0;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain     = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType     = ims_CallType_CALL_TYPE_VT;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id    = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type  = CALL_TYPE_VT_V02;
  call_info[0].direction  = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty    = 0;
  call_info[0].mode       = CALL_MODE_LTE_V02;
  // Remote party number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id    = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi  = PRESENTATION_NUM_ALLOWED_V02;
  // Audio attributes
  audio_attrib = QMI_node.set_qmi_voice_all_call_status_ind_audio_attrib(1);
  audio_attrib[0].call_id         = 1;
  audio_attrib[0].call_attributes = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
  // Video attributes
  video_attrib = QMI_node.set_qmi_voice_all_call_status_ind_video_attrib(1);
  video_attrib[0].call_id         = 1;
  video_attrib[0].call_attributes = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index     = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state     = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa     = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->isMpty     = 0;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT     = 0;
  calls[0]->has_als = TRUE;
  calls[0]->als     = 0;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain     = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType     = ims_CallType_CALL_TYPE_VT;
  t.expect(IMS_node);

  // Inject ims_MsgId_REQUEST_MODIFY_CALL_INITIATE
  IMS_node.update_default_ims_request_modify_call_initiate();
  IMS_node.set_ims_request_modify_call_initiate_call_index(1);
  IMS_node.set_ims_request_modify_call_initiate_call_details(
      ims_CallType_CALL_TYPE_VT_NODIR,
      ims_CallDomain_CALL_DOMAIN_PS);
  t.inject(IMS_node);


  // Expect: QMI_VOICE_MANAGE_IP_CALLS_REQ_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_req_msg(VOIP_SUPS_TYPE_MODIFY_CALL_V02);
  QMI_node.set_qmi_voice_manage_ip_calls_req_call_id(1);
  QMI_node.set_qmi_voice_manage_ip_calls_req_call_type(CALL_TYPE_VT_V02);
  QMI_node.set_qmi_voice_manage_ip_calls_req_audio_attrib(
      VOICE_CALL_ATTRIB_TX_V02|VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_manage_ip_calls_req_video_attrib(0);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_MANAGE_IP_CALLS_RESP_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_resp_msg(QMI_RESULT_SUCCESS_V01,
      QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject QMI_VOICE_MODIFIED_IND_V02
  QMI_node.update_default_qmi_voice_modified_ind_msg(1);
  QMI_node.set_qmi_voice_modified_ind_call_type(CALL_TYPE_VT_V02);
  QMI_node.set_qmi_voice_modified_ind_audio_attrib(
      VOICE_CALL_ATTRIB_TX_V02|VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_modified_ind_video_attrib(0);
  QMI_node.set_qmi_voice_modified_ind_call_attrib_status(VOICE_CALL_ATTRIB_STATUS_MEDIA_PAUSED_V02);
  t.inject(QMI_node);

  // Expect ims_MsgId_REQUEST_MODIFY_CALL_INITIATE (Resp)
  IMS_node.update_default_ims_request_modify_call_initiate_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index     = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state     = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa     = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->isMpty     = 0;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT     = 0;
  calls[0]->has_als = TRUE;
  calls[0]->als     = 0;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain     = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType     = ims_CallType_CALL_TYPE_VT_NODIR;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate     = ims_CallSubstate_CALL_SUBSTATE_MEDIA_PAUSED;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_MODIFY_CALL_INITIATE
  IMS_node.update_default_ims_request_modify_call_initiate();
  IMS_node.set_ims_request_modify_call_initiate_call_index(1);
  IMS_node.set_ims_request_modify_call_initiate_call_details(
      ims_CallType_CALL_TYPE_VT,
      ims_CallDomain_CALL_DOMAIN_PS);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_MANAGE_IP_CALLS_REQ_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_req_msg(VOIP_SUPS_TYPE_MODIFY_CALL_V02);
  QMI_node.set_qmi_voice_manage_ip_calls_req_call_id(1);
  QMI_node.set_qmi_voice_manage_ip_calls_req_call_type(CALL_TYPE_VT_V02);
  QMI_node.set_qmi_voice_manage_ip_calls_req_audio_attrib(
      VOICE_CALL_ATTRIB_TX_V02|VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_manage_ip_calls_req_video_attrib(
      VOICE_CALL_ATTRIB_TX_V02|VOICE_CALL_ATTRIB_RX_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_MANAGE_IP_CALLS_RESP_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_resp_msg(QMI_RESULT_SUCCESS_V01,
      QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject QMI_VOICE_MODIFIED_IND_V02
  QMI_node.update_default_qmi_voice_modified_ind_msg(1);
  QMI_node.set_qmi_voice_modified_ind_call_type(CALL_TYPE_VT_V02);
  QMI_node.set_qmi_voice_modified_ind_audio_attrib(
      VOICE_CALL_ATTRIB_TX_V02|VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_modified_ind_video_attrib(
      VOICE_CALL_ATTRIB_TX_V02|VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_modified_ind_call_attrib_status(VOICE_CALL_ATTRIB_STATUS_OK_V02);
  t.inject(QMI_node);

  // Expect ims_MsgId_REQUEST_MODIFY_CALL_INITIATE (Resp)
  IMS_node.update_default_ims_request_modify_call_initiate_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index     = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state     = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa     = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->isMpty     = 0;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT     = 0;
  calls[0]->has_als = TRUE;
  calls[0]->als     = 0;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain     = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType     = ims_CallType_CALL_TYPE_VT;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate     = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(1);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id    = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type  = CALL_TYPE_VT_V02;
  call_info[0].direction  = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty    = 0;
  call_info[0].mode       = CALL_MODE_LTE_V02;
  // Remote party number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id    = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi  = PRESENTATION_NUM_ALLOWED_V02;
  // Call end reason
  voice_call_end_reason_type_v02* end = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end[0].call_id         = 1;
  end[0].call_end_reason = CALL_END_CAUSE_NORMAL_CALL_CLEARING_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->isMpty     = 0;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT     = 0;
  calls[0]->has_als = TRUE;
  calls[0]->als     = 0;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();
  return t.get_test_result();
}


/**==
@testname: ril_utf_ims_mo_volte_alert_cap
@description: MO LTE call with Local/Peer capabilities and Alerting type
==**/
test_result ril_utf_ims_mo_volte_alert_cap(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id("ril_utf_ims_mo_volte_alert_cap");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CC_IN_PROGRESS_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // Media ID
  voice_media_id_with_id_type_v02 *media_id = QMI_node.set_qmi_voice_all_call_status_ind_media_id(1);
  media_id[0].call_id = 1;
  media_id[0].media_id = 2;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // alerting_type
  voice_alerting_type_type_v02 *alerting_type = QMI_node.set_qmi_voice_all_call_status_ind_alerting_type(1);
  alerting_type[0].call_id = 1;
  alerting_type[0].alerting_type = ALERTING_REMOTE_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RINGBACK_TONE
  IMS_node.update_default_ims_unsol_ringback_tone();
  IMS_node.set_ims_unsol_ringback_tone_flag(ims_RingBackTone_ToneFlag_START);
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // local_call_capabilities_info
  voice_ip_call_capabilities_info_type_v02 *local_call_cap = QMI_node.set_qmi_voice_all_call_status_ind_local_call_capabilities_info(1);
  local_call_cap[0].call_id      = 1;
  local_call_cap[0].audio_attrib = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
  local_call_cap[0].audio_cause  = VOICE_RESTRICT_CAUSE_NONE_V02;
  local_call_cap[0].video_attrib = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
  local_call_cap[0].video_cause  = VOICE_RESTRICT_CAUSE_NONE_V02;
  // peer_call_capabilities_info
  voice_ip_call_capabilities_info_type_v02 *peer_call_cap = QMI_node.set_qmi_voice_all_call_status_ind_peer_call_capabilities_info(1);
  peer_call_cap[0].call_id      = 1;
  peer_call_cap[0].audio_attrib = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
  peer_call_cap[0].audio_cause  = VOICE_RESTRICT_CAUSE_NONE_V02;
  peer_call_cap[0].video_attrib = 0;
  peer_call_cap[0].video_cause  = VOICE_RESTRICT_CAUSE_NONE_V02;
  // alerting_type
  alerting_type = QMI_node.set_qmi_voice_all_call_status_ind_alerting_type(1);
  alerting_type[0].call_id = 1;
  alerting_type[0].alerting_type = ALERTING_LOCAL_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RINGBACK_TONE
  IMS_node.update_default_ims_unsol_ringback_tone();
  IMS_node.set_ims_unsol_ringback_tone_flag(ims_RingBackTone_ToneFlag_STOP);
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  calls[0]->callDetails.has_localAbility = true;
  calls[0]->callDetails.localAbility.SrvStatusInfo.arg = qcril_malloc(3 * sizeof(ims_Info*));
  ims_Info ** tmp_SrvStatusInfo = (ims_Info **)calls[0]->callDetails.localAbility.SrvStatusInfo.arg;
  tmp_SrvStatusInfo[0] = (ims_Info *)qcril_malloc(sizeof(ims_Info));
  tmp_SrvStatusInfo[0]->has_isValid = TRUE;
  tmp_SrvStatusInfo[0]->isValid = TRUE;
  tmp_SrvStatusInfo[0]->has_callType = TRUE;
  tmp_SrvStatusInfo[0]->callType = ims_CallType_CALL_TYPE_VOICE;
  tmp_SrvStatusInfo[0]->has_status = TRUE;
  tmp_SrvStatusInfo[0]->status = ims_StatusType_STATUS_ENABLED;
  tmp_SrvStatusInfo[0]->has_restrictCause = TRUE;
  tmp_SrvStatusInfo[0]->restrictCause = VOICE_RESTRICT_CAUSE_NONE_V02;
  tmp_SrvStatusInfo[0]->accTechStatus.arg = qcril_malloc(2 * sizeof(ims_StatusForAccessTech*));
  ims_StatusForAccessTech** tmp_accTechStatus = (ims_StatusForAccessTech**) tmp_SrvStatusInfo[0]->accTechStatus.arg;
  tmp_accTechStatus[0] = (ims_StatusForAccessTech *)qcril_malloc(sizeof(ims_StatusForAccessTech));
  tmp_accTechStatus[0]->has_networkMode = TRUE;
  tmp_accTechStatus[0]->networkMode = ims_RadioTechType_RADIO_TECH_LTE;
  tmp_accTechStatus[0]->has_status = TRUE;
  tmp_accTechStatus[0]->status = ims_StatusType_STATUS_ENABLED;
  tmp_accTechStatus[0]->has_restrictionCause = TRUE;
  tmp_accTechStatus[0]->restrictionCause = VOICE_RESTRICT_CAUSE_NONE_V02;
  tmp_accTechStatus[1] = NULL;
  tmp_SrvStatusInfo[1] = (ims_Info *)qcril_malloc(sizeof(ims_Info));
  tmp_SrvStatusInfo[1]->has_isValid = TRUE;
  tmp_SrvStatusInfo[1]->isValid = TRUE;
  tmp_SrvStatusInfo[1]->has_restrictCause = TRUE;
  tmp_SrvStatusInfo[1]->restrictCause = VOICE_RESTRICT_CAUSE_NONE_V02;
  tmp_SrvStatusInfo[1]->has_callType = TRUE;
  tmp_SrvStatusInfo[1]->callType = ims_CallType_CALL_TYPE_VT;
  tmp_SrvStatusInfo[1]->has_status = TRUE;
  tmp_SrvStatusInfo[1]->status = ims_StatusType_STATUS_ENABLED;
  tmp_SrvStatusInfo[2] = NULL;

  calls[0]->callDetails.has_peerAbility = TRUE;
  calls[0]->callDetails.peerAbility.SrvStatusInfo.arg = qcril_malloc(3 * sizeof(ims_Info*));
  tmp_SrvStatusInfo = (ims_Info**) calls[0]->callDetails.peerAbility.SrvStatusInfo.arg;
  tmp_SrvStatusInfo[0] = (ims_Info *)qcril_malloc(sizeof(ims_Info));
  tmp_SrvStatusInfo[0]->has_isValid = TRUE;
  tmp_SrvStatusInfo[0]->isValid = TRUE;
  tmp_SrvStatusInfo[0]->has_restrictCause = TRUE;
  tmp_SrvStatusInfo[0]->restrictCause = VOICE_RESTRICT_CAUSE_NONE_V02;
  tmp_SrvStatusInfo[0]->has_callType = TRUE;
  tmp_SrvStatusInfo[0]->callType = ims_CallType_CALL_TYPE_VOICE;
  tmp_SrvStatusInfo[0]->has_status = TRUE;
  tmp_SrvStatusInfo[0]->status = ims_StatusType_STATUS_ENABLED;
  tmp_SrvStatusInfo[1] = (ims_Info *)qcril_malloc(sizeof(ims_Info));
  tmp_SrvStatusInfo[1]->has_isValid = TRUE;
  tmp_SrvStatusInfo[1]->isValid = TRUE;
  tmp_SrvStatusInfo[1]->has_restrictCause = TRUE;
  tmp_SrvStatusInfo[1]->restrictCause = VOICE_RESTRICT_CAUSE_NONE_V02;
  tmp_SrvStatusInfo[1]->has_callType = TRUE;
  tmp_SrvStatusInfo[1]->callType = ims_CallType_CALL_TYPE_VT;
  tmp_SrvStatusInfo[1]->has_status = TRUE;
  tmp_SrvStatusInfo[1]->status = ims_StatusType_STATUS_DISABLED;
  tmp_SrvStatusInfo[2] = NULL;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(1);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
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

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);

  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_vt_downgrade_voip
@description:
==**/
test_result ril_utf_ims_vt_downgrade_voip(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;
  OEMTestnode OEM_node;

  t.set_test_id("ril_utf_ims_vt_downgrade_voip");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("9876543210");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("9876543210");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VT_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VT_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("9876543210");
  strlcpy(number_list[0].number, "9876543210", strlen("9876543210")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  QMI_node.set_qmi_voice_all_call_status_ind_audio_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_all_call_status_ind_video_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = -1;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VT_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("9876543210");
  strlcpy(number_list[0].number, "9876543210", strlen("9876543210")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  QMI_node.set_qmi_voice_all_call_status_ind_audio_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_all_call_status_ind_video_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VT_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("9876543210");
  strlcpy(number_list[0].number, "9876543210", strlen("9876543210")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  QMI_node.set_qmi_voice_all_call_status_ind_audio_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_all_call_status_ind_video_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  // local_call_capabilities_info
  voice_ip_call_capabilities_info_type_v02 *local_call_cap = QMI_node.set_qmi_voice_all_call_status_ind_local_call_capabilities_info(1);
  local_call_cap[0].call_id      = 1;
  local_call_cap[0].audio_attrib = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
  local_call_cap[0].audio_cause  = VOICE_RESTRICT_CAUSE_NONE_V02;
  local_call_cap[0].video_attrib = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
  local_call_cap[0].video_cause  = VOICE_RESTRICT_CAUSE_NONE_V02;

  // peer_call_capabilities_info
  voice_ip_call_capabilities_info_type_v02 *peer_call_cap = QMI_node.set_qmi_voice_all_call_status_ind_peer_call_capabilities_info(1);
  peer_call_cap[0].call_id      = 1;
  peer_call_cap[0].audio_attrib = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
  peer_call_cap[0].audio_cause  = VOICE_RESTRICT_CAUSE_NONE_V02;
  peer_call_cap[0].video_attrib = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
  peer_call_cap[0].video_cause  = VOICE_RESTRICT_CAUSE_NONE_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->has_callDetails = TRUE;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VT;

  calls[0]->callDetails.has_localAbility = TRUE;
  int ims_info_count = 2;
  ims_Info **list_SrvStatusInfo = (ims_Info **)qcril_malloc((ims_info_count+1) * sizeof(ims_Info*));

  // ims_CallType_CALL_TYPE_VOICE,
  list_SrvStatusInfo[0] = IMS_node.create_ims_Info(
      TRUE, TRUE,  // isValid
      FALSE, ims_SrvType_VOIP, // type
      TRUE, ims_CallType_CALL_TYPE_VOICE, // callType,
      TRUE, ims_StatusType_STATUS_ENABLED, // has_status,
      NULL, //userdata,
      TRUE, (uint32_t)VOICE_RESTRICT_CAUSE_NONE_V02, // restrictCause,
      NULL); // accTechStatus

  // ims_CallType_CALL_TYPE_VT
  int ims_StatusForAccessTech_count = 1;
  ims_StatusForAccessTech **list_accTechStatus = (ims_StatusForAccessTech **)
    qcril_malloc((ims_StatusForAccessTech_count+1) * sizeof(ims_StatusForAccessTech *));
  ims_Registration ims_reg;
  memset(&ims_reg, 0x00, sizeof(ims_reg));
  list_accTechStatus[0] = IMS_node.create_ims_StatusForAccessTech(
      TRUE, ims_RadioTechType_RADIO_TECH_LTE, // networkMode,
      TRUE, ims_StatusType_STATUS_ENABLED, // status,
      TRUE, (uint32_t)VOICE_RESTRICT_CAUSE_NONE_V02, //restrictionCause,
      FALSE, ims_reg);
  list_SrvStatusInfo[1] = IMS_node.create_ims_Info(
      TRUE, TRUE,  // isValid
      FALSE, ims_SrvType_VOIP, // type
      TRUE, ims_CallType_CALL_TYPE_VT, // callType,
      TRUE, ims_StatusType_STATUS_ENABLED, // has_status,
      NULL, //userdata,
      TRUE, (uint32_t)VOICE_RESTRICT_CAUSE_NONE_V02, // restrictCause,
      list_accTechStatus); // accTechStatus

  calls[0]->callDetails.localAbility.SrvStatusInfo.arg = list_SrvStatusInfo;

  calls[0]->callDetails.has_peerAbility = TRUE;
  ims_info_count = 2;
  list_SrvStatusInfo = (ims_Info **)qcril_malloc((ims_info_count+1) * sizeof(ims_Info*));

  // ims_CallType_CALL_TYPE_VOICE,
  list_SrvStatusInfo[0] = IMS_node.create_ims_Info(
      TRUE, TRUE,  // isValid
      FALSE, ims_SrvType_VOIP, // type
      TRUE, ims_CallType_CALL_TYPE_VOICE, // callType,
      TRUE, ims_StatusType_STATUS_ENABLED, // has_status,
      NULL, //userdata,
      TRUE, (uint32_t)VOICE_RESTRICT_CAUSE_NONE_V02, // restrictCause,
      NULL); // accTechStatus
  list_SrvStatusInfo[1] = IMS_node.create_ims_Info(
      TRUE, TRUE,  // isValid
      FALSE, ims_SrvType_VOIP, // type
      TRUE, ims_CallType_CALL_TYPE_VT, // callType,
      TRUE, ims_StatusType_STATUS_ENABLED, // has_status,
      NULL, //userdata,
      TRUE, (uint32_t)VOICE_RESTRICT_CAUSE_NONE_V02, // restrictCause,
      NULL); // accTechStatus
  calls[0]->callDetails.peerAbility.SrvStatusInfo.arg = list_SrvStatusInfo;

  t.expect(IMS_node);

  // Inject ims_MsgId_REQUEST_MODIFY_CALL_INITIATE
  IMS_node.update_default_ims_request_modify_call_initiate();
  IMS_node.set_ims_request_modify_call_initiate_call_index(1);
  IMS_node.set_ims_request_modify_call_initiate_call_details(
      ims_CallType_CALL_TYPE_VOICE,
      ims_CallDomain_CALL_DOMAIN_PS);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_MANAGE_IP_CALLS_REQ_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_req_msg(VOIP_SUPS_TYPE_MODIFY_CALL_V02);
  QMI_node.set_qmi_voice_manage_ip_calls_req_call_id(1);
  QMI_node.set_qmi_voice_manage_ip_calls_req_call_type(CALL_TYPE_VOICE_IP_V02);
  QMI_node.set_qmi_voice_manage_ip_calls_req_audio_attrib(
      VOICE_CALL_ATTRIB_TX_V02|VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_manage_ip_calls_req_video_attrib(0);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_MANAGE_IP_CALLS_RESP_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_resp_msg(QMI_RESULT_SUCCESS_V01,
      QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject QMI_VOICE_MODIFIED_IND_V02
  QMI_node.update_default_qmi_voice_modified_ind_msg(1);
  QMI_node.set_qmi_voice_modified_ind_call_type(CALL_TYPE_VOICE_IP_V02);
  QMI_node.set_qmi_voice_modified_ind_audio_attrib(
      VOICE_CALL_ATTRIB_TX_V02|VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_modified_ind_video_attrib(0);
  t.inject(QMI_node);

  // Expect ims_MsgId_REQUEST_MODIFY_CALL_INITIATE (Resp)
  IMS_node.update_default_ims_request_modify_call_initiate_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("9876543210");
  strlcpy(number_list[0].number, "9876543210", strlen("9876543210")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  voice_call_end_reason_type_v02* end_reason = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end_reason[0].call_end_reason = CALL_END_CAUSE_NORMAL_CALL_CLEARING_V02;
  end_reason[0].call_id = 1;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  t.expect(IMS_node);

  t.execute();

  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_vt_emergency_call
@description:
==**/
test_result ril_utf_ims_vt_emergency_call(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;
  OEMTestnode OEM_node;

  t.set_test_id("ril_utf_ims_vt_emergency_call");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("119");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("119");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VT_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_EMERGENCY_VT_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("119");
  strlcpy(number_list[0].number, "119", strlen("119")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  QMI_node.set_qmi_voice_all_call_status_ind_audio_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_all_call_status_ind_video_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = -1;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_EMERGENCY_VT_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("119");
  strlcpy(number_list[0].number, "119", strlen("119")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  QMI_node.set_qmi_voice_all_call_status_ind_audio_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_all_call_status_ind_video_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_EMERGENCY_VT_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("119");
  strlcpy(number_list[0].number, "119", strlen("119")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  QMI_node.set_qmi_voice_all_call_status_ind_audio_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_all_call_status_ind_video_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_EMERGENCY_VT_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("119");
  strlcpy(number_list[0].number, "119", strlen("119")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  QMI_node.set_qmi_voice_all_call_status_ind_audio_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_all_call_status_ind_video_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  voice_call_end_reason_type_v02* end_reason = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end_reason[0].call_end_reason = CALL_END_CAUSE_NORMAL_CALL_CLEARING_V02;
  end_reason[0].call_id = 1;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  t.expect(IMS_node);

  t.execute();

  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_em_vt_connected_as_em_voip
@description: User dials emergency VT call, but connected as emergency VoIP.
==**/
test_result ril_utf_ims_em_vt_connected_as_em_voip(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;
  OEMTestnode OEM_node;

  t.set_test_id("ril_utf_ims_em_vt_connected_as_em_voip");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("119");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("119");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VT_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_EMERGENCY_VT_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("119");
  strlcpy(number_list[0].number, "119", strlen("119")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  QMI_node.set_qmi_voice_all_call_status_ind_audio_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_all_call_status_ind_video_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = -1;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_EMERGENCY_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("119");
  strlcpy(number_list[0].number, "119", strlen("119")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_EMERGENCY_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("119");
  strlcpy(number_list[0].number, "119", strlen("119")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_EMERGENCY_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("119");
  strlcpy(number_list[0].number, "119", strlen("119")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  voice_call_end_reason_type_v02* end_reason = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end_reason[0].call_end_reason = CALL_END_CAUSE_NORMAL_CALL_CLEARING_V02;
  end_reason[0].call_id = 1;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  t.expect(IMS_node);

  t.execute();

  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_em_vt_connected_as_em_cs
@description: User dials emergency VT call, but connected as emergency call on CS.
==**/
test_result ril_utf_ims_em_vt_connected_as_em_cs(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;
  OEMTestnode OEM_node;
  AndroidTelTestnode RIL_node;

  t.set_test_id("ril_utf_ims_em_vt_connected_as_em_cs");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("119");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("119");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VT_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_EMERGENCY_VT_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("119");
  strlcpy(number_list[0].number, "119", strlen("119")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  QMI_node.set_qmi_voice_all_call_status_ind_audio_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_all_call_status_ind_video_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = -1;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_EMERGENCY_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_GSM_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("119");
  strlcpy(number_list[0].number, "119", strlen("119")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_EMERGENCY_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_GSM_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("119");
  strlcpy(number_list[0].number, "119", strlen("119")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  t.set_start();
  // Expect: RIL_UNSOL_SRVCC_STATE_NOTIFY
  RIL_node.update_default_ril_unsol_srvcc_state_notify(HANDOVER_STARTED);
  t.expect(RIL_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_HANDOVER
  IMS_node.update_default_ims_unsol_response_handover();
  IMS_node.set_ims_unsol_response_handover_type(ims_Handover_Msg_Type_START);
  t.expect(IMS_node);

  // Expect: RIL_UNSOL_SRVCC_STATE_NOTIFY
  RIL_node.update_default_ril_unsol_srvcc_state_notify(HANDOVER_COMPLETED);
  t.expect(RIL_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_HANDOVER
  IMS_node.update_default_ims_unsol_response_handover();
  IMS_node.set_ims_unsol_response_handover_type(ims_Handover_Msg_Type_COMPLETE_SUCCESS);
  t.expect(IMS_node);

  // Inform ATEL of SRVCC
  // Expect: RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_call_state_changed();
  t.expect(RIL_node);
  t.set_end();

  // Inject: RIL_REQUEST_GET_CURRENT_CALLS
  RIL_node.update_default_ril_request_get_current_calls();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_GET_CURRENT_CALLS (resp)
  RIL_Call** call = RIL_node.update_default_ril_request_get_current_calls_resp(1);
  call[0]->state = RIL_CALL_ACTIVE;
  call[0]->toa = 129;
  call[0]->isVoice = 1;
  call[0]->index = 1;
  RIL_node.set_ril_request_get_current_calls_resp_number(0, "119");
  call[0]->numberPresentation = 0;
  call[0]->namePresentation = 2;
  t.expect(RIL_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_EMERGENCY_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("119");
  strlcpy(number_list[0].number, "119", strlen("119")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  voice_call_end_reason_type_v02* end_reason = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end_reason[0].call_end_reason = CALL_END_CAUSE_NORMAL_CALL_CLEARING_V02;
  end_reason[0].call_id = 1;
  t.inject(QMI_node);

  // Expect: RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_call_state_changed();
  t.expect(RIL_node);

  // Inject: RIL_REQUEST_GET_CURRENT_CALLS
  RIL_node.update_default_ril_request_get_current_calls();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_GET_CURRENT_CALLS (resp)
  RIL_node.update_default_ril_request_get_current_calls_resp(0);
  t.expect(RIL_node);

  // Inject: RIL_REQUEST_LAST_CALL_FAIL_CAUSE
  RIL_node.update_default_ril_request_last_call_fail_cause();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_LAST_CALL_FAIL_CAUSE
  RIL_LastCallFailCauseInfo *fail_cause =
    RIL_node.update_default_ril_request_last_call_fail_cause_resp();
  fail_cause->cause_code = CALL_FAIL_NORMAL;
  t.expect(RIL_node);

  t.execute();

  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_em_vt_downgrade_em_voip
@description:
==**/
test_result ril_utf_ims_em_vt_downgrade_em_voip(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;
  OEMTestnode OEM_node;

  t.set_test_id("ril_utf_ims_em_vt_downgrade_em_voip");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("119");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("119");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VT_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_EMERGENCY_VT_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("119");
  strlcpy(number_list[0].number, "119", strlen("119")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  QMI_node.set_qmi_voice_all_call_status_ind_audio_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_all_call_status_ind_video_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = -1;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_EMERGENCY_VT_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("119");
  strlcpy(number_list[0].number, "119", strlen("119")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  QMI_node.set_qmi_voice_all_call_status_ind_audio_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_all_call_status_ind_video_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_EMERGENCY_VT_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("119");
  strlcpy(number_list[0].number, "119", strlen("119")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  QMI_node.set_qmi_voice_all_call_status_ind_audio_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_all_call_status_ind_video_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  t.expect(IMS_node);

  // Inject ims_MsgId_REQUEST_MODIFY_CALL_INITIATE
  IMS_node.update_default_ims_request_modify_call_initiate();
  IMS_node.set_ims_request_modify_call_initiate_call_index(1);
  IMS_node.set_ims_request_modify_call_initiate_call_details(
      ims_CallType_CALL_TYPE_VOICE,
      ims_CallDomain_CALL_DOMAIN_PS);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_MANAGE_IP_CALLS_REQ_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_req_msg(VOIP_SUPS_TYPE_MODIFY_CALL_V02);
  QMI_node.set_qmi_voice_manage_ip_calls_req_call_id(1);
  QMI_node.set_qmi_voice_manage_ip_calls_req_call_type(CALL_TYPE_EMERGENCY_IP_V02);
  QMI_node.set_qmi_voice_manage_ip_calls_req_audio_attrib(
      VOICE_CALL_ATTRIB_TX_V02|VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_manage_ip_calls_req_video_attrib(0);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_MANAGE_IP_CALLS_RESP_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_resp_msg(QMI_RESULT_SUCCESS_V01,
      QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject QMI_VOICE_MODIFIED_IND_V02
  QMI_node.update_default_qmi_voice_modified_ind_msg(1);
  QMI_node.set_qmi_voice_modified_ind_call_type(CALL_TYPE_EMERGENCY_IP_V02);
  QMI_node.set_qmi_voice_modified_ind_audio_attrib(
      VOICE_CALL_ATTRIB_TX_V02|VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_modified_ind_video_attrib(0);
  t.inject(QMI_node);

  // Expect ims_MsgId_REQUEST_MODIFY_CALL_INITIATE (Resp)
  IMS_node.update_default_ims_request_modify_call_initiate_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_EMERGENCY_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("119");
  strlcpy(number_list[0].number, "119", strlen("119")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  voice_call_end_reason_type_v02* end_reason = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end_reason[0].call_end_reason = CALL_END_CAUSE_NORMAL_CALL_CLEARING_V02;
  end_reason[0].call_id = 1;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  t.expect(IMS_node);

  t.execute();

  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_modem_silent_redial_001_L
@description: MO call on IMS socket, silently redialed by modem and connected as CS
==**/
test_result ril_utf_ims_modem_silent_redial_001_L(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;
  AndroidTelTestnode RIL_node;

  t.set_test_id(__func__);
  t.set_test_description("MO call on IMS socket, silently redialed by modem and connected as CS");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }
  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CC_IN_PROGRESS_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // Media ID
  voice_media_id_with_id_type_v02 *media_id = QMI_node.set_qmi_voice_all_call_status_ind_media_id(1);
  media_id[0].call_id = 1;
  media_id[0].media_id = 2;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_GSM_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_GSM_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  t.set_start();
  // Expect: RIL_UNSOL_SRVCC_STATE_NOTIFY
  RIL_node.update_default_ril_unsol_srvcc_state_notify(HANDOVER_STARTED);
  t.expect(RIL_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_HANDOVER
  IMS_node.update_default_ims_unsol_response_handover();
  IMS_node.set_ims_unsol_response_handover_type(ims_Handover_Msg_Type_START);
  t.expect(IMS_node);

  // Expect: RIL_UNSOL_SRVCC_STATE_NOTIFY
  RIL_node.update_default_ril_unsol_srvcc_state_notify(HANDOVER_COMPLETED);
  t.expect(RIL_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_HANDOVER
  IMS_node.update_default_ims_unsol_response_handover();
  IMS_node.set_ims_unsol_response_handover_type(ims_Handover_Msg_Type_COMPLETE_SUCCESS);
  t.expect(IMS_node);

  // Inform ATEL of SRVCC
  // Expect: RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_call_state_changed();
  t.expect(RIL_node);
  t.set_end();

  // Inject: RIL_REQUEST_GET_CURRENT_CALLS
  RIL_node.update_default_ril_request_get_current_calls();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_GET_CURRENT_CALLS (resp)
  RIL_Call** call = RIL_node.update_default_ril_request_get_current_calls_resp(1);
  call[0]->state = RIL_CALL_ACTIVE;
  call[0]->toa = 129;
  call[0]->isVoice = 1;
  call[0]->index = 1;
  RIL_node.set_ril_request_get_current_calls_resp_number(0, "2428536960");
  call[0]->numberPresentation = 0;
  call[0]->namePresentation = 2;
  t.expect(RIL_node);


  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_EMERGENCY_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  voice_call_end_reason_type_v02* end_reason = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end_reason[0].call_end_reason = CALL_END_CAUSE_NORMAL_CALL_CLEARING_V02;
  end_reason[0].call_id = 1;
  t.inject(QMI_node);

  // Expect: RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_call_state_changed();
  t.expect(RIL_node);

  // Inject: RIL_REQUEST_GET_CURRENT_CALLS
  RIL_node.update_default_ril_request_get_current_calls();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_GET_CURRENT_CALLS (resp)
  RIL_node.update_default_ril_request_get_current_calls_resp(0);
  t.expect(RIL_node);

  // Inject: RIL_REQUEST_LAST_CALL_FAIL_CAUSE
  RIL_node.update_default_ril_request_last_call_fail_cause();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_LAST_CALL_FAIL_CAUSE
  RIL_LastCallFailCauseInfo *fail_cause =
    RIL_node.update_default_ril_request_last_call_fail_cause_resp();
  fail_cause->cause_code = CALL_FAIL_NORMAL;
  t.expect(RIL_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_modem_silent_redial_002_L
@description: MO call on IMS socket, silently redialed by modem and connected as CS
==**/
test_result ril_utf_ims_modem_silent_redial_002_L(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;
  AndroidTelTestnode RIL_node;

  t.set_test_id(__func__);
  t.set_test_description("MO call on IMS socket, silently redialed by modem and connected as CS");
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }
  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("17820500204");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("17820500204");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CC_IN_PROGRESS_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("17820500204");
  strlcpy(number_list[0].number, "17820500204", strlen("17820500204")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // Media ID
  voice_media_id_with_id_type_v02 *media_id = QMI_node.set_qmi_voice_all_call_status_ind_media_id(1);
  media_id[0].call_id = 1;
  media_id[0].media_id = 2;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("17820500204");
  strlcpy(number_list[0].number, "17820500204", strlen("17820500204")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_GSM_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("17820500204");
  strlcpy(number_list[0].number, "17820500204", strlen("17820500204")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // conn_party_number
  voice_conn_num_with_id_type_v02 *conn_num_list = QMI_node.set_qmi_voice_all_call_status_ind_conn_party_number(1);
  conn_num_list[0].call_id = 1;
  conn_num_list[0].conn_num_pi = PRESENTATION_NUM_ALLOWED_V02;
  char *conn_num = "+8617820500204";
  conn_num_list[0].conn_num_len = strlen(conn_num);
  strlcpy(conn_num_list[0].conn_num, conn_num, strlen(conn_num)+1);
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 145;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_GSM_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("17820500204");
  strlcpy(number_list[0].number, "17820500204", strlen("17820500204")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // conn_party_number
  conn_num_list = QMI_node.set_qmi_voice_all_call_status_ind_conn_party_number(1);
  conn_num_list[0].call_id = 1;
  conn_num_list[0].conn_num_pi = PRESENTATION_NUM_ALLOWED_V02;
  conn_num = "+8617820500204";
  conn_num_list[0].conn_num_len = strlen(conn_num);
  strlcpy(conn_num_list[0].conn_num, conn_num, strlen(conn_num)+1);
  t.inject(QMI_node);

  t.set_start();
  // Expect: RIL_UNSOL_SRVCC_STATE_NOTIFY
  RIL_node.update_default_ril_unsol_srvcc_state_notify(HANDOVER_STARTED);
  t.expect(RIL_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_HANDOVER
  IMS_node.update_default_ims_unsol_response_handover();
  IMS_node.set_ims_unsol_response_handover_type(ims_Handover_Msg_Type_START);
  t.expect(IMS_node);

  // Expect: RIL_UNSOL_SRVCC_STATE_NOTIFY
  RIL_node.update_default_ril_unsol_srvcc_state_notify(HANDOVER_COMPLETED);
  t.expect(RIL_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_HANDOVER
  IMS_node.update_default_ims_unsol_response_handover();
  IMS_node.set_ims_unsol_response_handover_type(ims_Handover_Msg_Type_COMPLETE_SUCCESS);
  t.expect(IMS_node);

  // Inform ATEL of SRVCC
  // Expect: RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_call_state_changed();
  t.expect(RIL_node);
  t.set_end();

  // Inject: RIL_REQUEST_GET_CURRENT_CALLS
  RIL_node.update_default_ril_request_get_current_calls();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_GET_CURRENT_CALLS (resp)
  RIL_Call** call = RIL_node.update_default_ril_request_get_current_calls_resp(1);
  call[0]->state = RIL_CALL_ACTIVE;
  call[0]->toa = 145;
  call[0]->isVoice = 1;
  call[0]->index = 1;
  RIL_node.set_ril_request_get_current_calls_resp_number(0, "+8617820500204");
  call[0]->numberPresentation = 0;
  call[0]->namePresentation = 2;
  t.expect(RIL_node);


  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_EMERGENCY_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("17820500204");
  strlcpy(number_list[0].number, "17820500204", strlen("17820500204")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // conn_party_number
  conn_num_list = QMI_node.set_qmi_voice_all_call_status_ind_conn_party_number(1);
  conn_num_list[0].call_id = 1;
  conn_num_list[0].conn_num_pi = PRESENTATION_NUM_ALLOWED_V02;
  conn_num = "+8617820500204";
  conn_num_list[0].conn_num_len = strlen(conn_num);
  strlcpy(conn_num_list[0].conn_num, conn_num, strlen(conn_num)+1);
  voice_call_end_reason_type_v02* end_reason = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end_reason[0].call_end_reason = CALL_END_CAUSE_NORMAL_CALL_CLEARING_V02;
  end_reason[0].call_id = 1;
  t.inject(QMI_node);

  // Expect: RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_call_state_changed();
  t.expect(RIL_node);

  // Inject: RIL_REQUEST_GET_CURRENT_CALLS
  RIL_node.update_default_ril_request_get_current_calls();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_GET_CURRENT_CALLS (resp)
  RIL_node.update_default_ril_request_get_current_calls_resp(0);
  t.expect(RIL_node);

  // Inject: RIL_REQUEST_LAST_CALL_FAIL_CAUSE
  RIL_node.update_default_ril_request_last_call_fail_cause();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_LAST_CALL_FAIL_CAUSE
  RIL_LastCallFailCauseInfo *fail_cause =
    RIL_node.update_default_ril_request_last_call_fail_cause_resp();
  fail_cause->cause_code = CALL_FAIL_NORMAL;
  t.expect(RIL_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_mo_call_end_ho_not_feasible
@description: MO call getting ENDed with end reason HO_NOT_FEASIBLE
==**/
test_result ril_utf_ims_mo_call_end_ho_not_feasible(user_specified_attrib_t attr)
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

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("9876543210");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("9876543210");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VT_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(
          VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  QMI_node.set_qmi_voice_dial_call_resp_media_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id    = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type  = CALL_TYPE_VT_V02;
  call_info[0].direction  = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty    = 0;
  call_info[0].mode       = CALL_MODE_LTE_V02;
  // Remote party number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id    = 1;
  number_list[0].number_len = strlen("9876543210");
  strlcpy(number_list[0].number, "9876543210", strlen("9876543210")+1);
  number_list[0].number_pi  = PRESENTATION_NUM_ALLOWED_V02;
  // Audio attributes
  voice_call_attributes_type_v02 *audio_attrib = QMI_node.set_qmi_voice_all_call_status_ind_audio_attrib(1);
  audio_attrib[0].call_id         = 1;
  audio_attrib[0].call_attributes = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
  // Video attributes
  voice_call_attributes_type_v02 *video_attrib = QMI_node.set_qmi_voice_all_call_status_ind_video_attrib(1);
  video_attrib[0].call_id         = 1;
  video_attrib[0].call_attributes = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call **calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 1;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id    = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type  = CALL_TYPE_VT_V02;
  call_info[0].direction  = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty    = 0;
  call_info[0].mode       = CALL_MODE_LTE_V02;
  // Remote party number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id    = 1;
  number_list[0].number_len = strlen("9876543210");
  strlcpy(number_list[0].number, "9876543210", strlen("9876543210")+1);
  number_list[0].number_pi  = PRESENTATION_NUM_ALLOWED_V02;
  // Audio attributes
  audio_attrib = QMI_node.set_qmi_voice_all_call_status_ind_audio_attrib(1);
  audio_attrib[0].call_id         = 1;
  audio_attrib[0].call_attributes = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
  // Video attributes
  video_attrib = QMI_node.set_qmi_voice_all_call_status_ind_video_attrib(1);
  video_attrib[0].call_id         = 1;
  video_attrib[0].call_attributes = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
  // Call end reason
  voice_call_end_reason_type_v02* end = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end[0].call_id         = 1;
  end[0].call_end_reason = CALL_END_CAUSE_HO_NOT_FEASIBLE_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VT;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 1;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_HO_NOT_FEASIBLE;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_mt_call_screen_state_change_L
@description: MT LTE call through IMS with screen state change
==**/
test_result ril_utf_ims_mt_call_screen_state_change_L(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;
  AndroidTelTestnode RIL_node;

  t.set_test_id(__func__);
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  util_ril_utf_test_screen_state(t, RIL_node, QMI_node, 0, TRUE, FALSE, FALSE);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_INCOMING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MT_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // Media ID
  voice_media_id_with_id_type_v02 *media_id = QMI_node.set_qmi_voice_all_call_status_ind_media_id(1);
  media_id[0].call_id = 1;
  media_id[0].media_id = 0;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_CALL_RING
  IMS_node.update_default_ims_unsol_call_ring();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_INCOMING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 0;
  t.expect(IMS_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_handover_events(1);
  QMI_node.set_qmi_voice_ind_req_speech_events(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);


  util_ril_utf_test_screen_state(t, RIL_node, QMI_node, 1, FALSE, TRUE, TRUE);

  // Inject: ims_MsgId_REQUEST_ANSWER
  IMS_node.update_default_ims_request_answer();
  t.inject(IMS_node);

  // Expect: QMI_VOICE_ANSWER_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_answer_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_ANSWER_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_answer_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MT_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_ANSWER (resp)
  IMS_node.update_default_ims_request_answer_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 0;
  t.expect(IMS_node);

  util_ril_utf_test_screen_state(t, RIL_node, QMI_node, 0, FALSE, FALSE, FALSE);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MT_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
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

  t.set_start();

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 0;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  t.expect(IMS_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_handover_events(0);
  QMI_node.set_qmi_voice_ind_req_speech_events(0);
  t.expect(QMI_node);

  t.set_end();

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  util_ril_utf_test_screen_state(t, RIL_node, QMI_node, 1, TRUE, TRUE, TRUE);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_mt_call_srvcc_screen_state_change_L
@description: SRVCC L to G
==**/
test_result ril_utf_ims_mt_call_srvcc_screen_state_change_L(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;
  AndroidTelTestnode RIL_node;

  t.set_test_id(__func__);
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }


  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_INCOMING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MT_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // Media ID
  voice_media_id_with_id_type_v02 *media_id = QMI_node.set_qmi_voice_all_call_status_ind_media_id(1);
  media_id[0].call_id = 1;
  media_id[0].media_id = 0;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_CALL_RING
  IMS_node.update_default_ims_unsol_call_ring();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_INCOMING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 0;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_ANSWER
  IMS_node.update_default_ims_request_answer();
  t.inject(IMS_node);

  // Expect: QMI_VOICE_ANSWER_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_answer_call_req_msg(1);
  t.expect(QMI_node);

  util_ril_utf_test_screen_state(t, RIL_node, QMI_node, 0, FALSE, FALSE, FALSE);

  // Inject: QMI_VOICE_ANSWER_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_answer_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MT_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_ANSWER (resp)
  IMS_node.update_default_ims_request_answer_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 0;
  t.expect(IMS_node);

  // Get SRVCC_Start Indication
  // Inject: QMI_VOICE_HANDOVER_IND_V02
  QMI_node.update_qmi_voice_handover_ind_msg(VOICE_HANDOVER_START_V02, VOICE_HO_SRVCC_L_2_G_V02);
  t.inject(QMI_node);

  t.set_start();
  // Expect: RIL_UNSOL_SRVCC_STATE_NOTIFY
  RIL_node.update_default_ril_unsol_srvcc_state_notify(HANDOVER_STARTED);
  t.expect(RIL_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_HANDOVER
  IMS_node.update_default_ims_unsol_response_handover();
  IMS_node.set_ims_unsol_response_handover_type(ims_Handover_Msg_Type_START);
  t.expect(IMS_node);
  t.set_end();

  // Get SRVCC_Complete_Success Indication
  // Inject: QMI_VOICE_HANDOVER_IND_V02
  QMI_node.update_qmi_voice_handover_ind_msg(VOICE_HANDOVER_COMPLETE_V02, VOICE_HO_SRVCC_L_2_G_V02);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_INDICATION_REGISTER_REQ
  QMI_node.update_default_qmi_voice_ind_reg_req_msg();
  QMI_node.set_qmi_voice_ind_req_handover_events(0);
  QMI_node.set_qmi_voice_ind_req_speech_events(0);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_INDICATION_REGISTER_RESP
  QMI_node.update_default_qmi_voice_ind_reg_resp_msg();
  QMI_node.set_qmi_voice_indication_register_resp_result(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  t.set_start();
  // Expect: RIL_UNSOL_SRVCC_STATE_NOTIFY
  RIL_node.update_default_ril_unsol_srvcc_state_notify(HANDOVER_COMPLETED);
  t.expect(RIL_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_HANDOVER
  IMS_node.update_default_ims_unsol_response_handover();
  IMS_node.set_ims_unsol_response_handover_type(ims_Handover_Msg_Type_COMPLETE_SUCCESS);
  t.expect(IMS_node);
  t.set_end();

  // Updated call info for GSM call after SRVCC from LTE.
  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_GSM_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // SRVCC Info
  voice_is_srvcc_call_with_id_type_v02 *srvcc_info = QMI_node.set_qmi_voice_all_call_status_ind_srvcc_attrib(1);
  srvcc_info[0].call_id = 1;
  srvcc_info[0].is_srvcc_call = 1;
  t.inject(QMI_node);

  // Inform ATEL of SRVCC
  // Expect: RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_call_state_changed();
  t.expect(RIL_node);

  // Inject: RIL_REQUEST_GET_CURRENT_CALLS
  RIL_node.update_default_ril_request_get_current_calls();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_GET_CURRENT_CALLS (resp)
  RIL_Call** call = RIL_node.update_default_ril_request_get_current_calls_resp(1);
  call[0]->state = RIL_CALL_ACTIVE;
  call[0]->toa = 129;
  call[0]->isVoice = 1;
  call[0]->index = 1;
  RIL_node.set_ril_request_get_current_calls_resp_number(0, "2428536960");
  call[0]->numberPresentation = 0;
  call[0]->namePresentation = 2;
  t.expect(RIL_node);

  // Hangup sequence for ATEL and IMS
  // Inject: RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND
  RIL_node.update_default_ril_request_hangup_foreground_resume_background();
  t.inject(RIL_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_DISCONNECTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_GSM_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
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
  call_info[0].mode = CALL_MODE_GSM_V02;
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

  // Inject: RIL_REQUEST_LAST_CALL_FAIL_CAUSE
  RIL_node.update_default_ril_request_last_call_fail_cause();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_LAST_CALL_FAIL_CAUSE
  RIL_LastCallFailCauseInfo *fail_cause =
    RIL_node.update_default_ril_request_last_call_fail_cause_resp();
  fail_cause->cause_code = CALL_FAIL_NORMAL;
  t.expect(RIL_node);

  util_ril_utf_test_screen_state(t, RIL_node, QMI_node, 1, TRUE, TRUE, TRUE);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_mo_call_silent_redial_w_orig_fail
@description:
MO call is originated over IMS and receives failure response from network; Modem
does silent redial over CS and modem passes intermediate IMS error code to APP.
==**/
test_result ril_utf_ims_mo_call_silent_redial_w_orig_fail(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;
  AndroidTelTestnode RIL_node;

  t.set_test_id(__func__);
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  QMI_node.set_qmi_voice_dial_call_resp_media_id(10);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call **calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT = FALSE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 10;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_GSM_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // Orig fail reason
  voice_orig_fail_reason_with_id_type_v02 *orig_fail_reason = QMI_node.set_qmi_voice_all_call_status_ind_orig_fail_reason(1);
  orig_fail_reason[0].call_id = 1;
  orig_fail_reason[0].orig_fail_reason = CALL_END_CAUSE_SERVER_TIME_OUT_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->isMT = FALSE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 10;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_SIP_SERVER_TIMEOUT;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_GSM_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  t.set_start();

  // Expect: RIL_UNSOL_SRVCC_STATE_NOTIFY
  RIL_node.update_default_ril_unsol_srvcc_state_notify(HANDOVER_STARTED);
  t.expect(RIL_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_HANDOVER
  IMS_node.update_default_ims_unsol_response_handover();
  IMS_node.set_ims_unsol_response_handover_type(ims_Handover_Msg_Type_START);
  t.expect(IMS_node);

  // Expect: RIL_UNSOL_SRVCC_STATE_NOTIFY
  RIL_node.update_default_ril_unsol_srvcc_state_notify(HANDOVER_COMPLETED);
  t.expect(RIL_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_HANDOVER
  IMS_node.update_default_ims_unsol_response_handover();
  IMS_node.set_ims_unsol_response_handover_type(ims_Handover_Msg_Type_COMPLETE_SUCCESS);
  t.expect(IMS_node);

  // Expect: RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED
  RIL_node.update_default_ril_unsol_response_call_state_changed();
  t.expect(RIL_node);

  t.set_end();

  // Inject: RIL_REQUEST_GET_CURRENT_CALLS
  RIL_node.update_default_ril_request_get_current_calls();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_GET_CURRENT_CALLS (resp)
  RIL_Call** call = RIL_node.update_default_ril_request_get_current_calls_resp(1);
  call[0]->state = RIL_CALL_ACTIVE;
  call[0]->toa = 129;
  call[0]->isVoice = 1;
  call[0]->index = 1;
  RIL_node.set_ril_request_get_current_calls_resp_number(0, "2428536960");
  call[0]->numberPresentation = 0;
  call[0]->namePresentation = 2;
  t.expect(RIL_node);

  // Hangup sequence for ATEL and IMS
  // Inject: RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND
  RIL_node.update_default_ril_request_hangup_foreground_resume_background();
  t.inject(RIL_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_DISCONNECTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_GSM_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
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
  call_info[0].mode = CALL_MODE_GSM_V02;
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

  // Inject: RIL_REQUEST_LAST_CALL_FAIL_CAUSE
  RIL_node.update_default_ril_request_last_call_fail_cause();
  t.inject(RIL_node);

  // Expect: RIL_REQUEST_LAST_CALL_FAIL_CAUSE
  RIL_LastCallFailCauseInfo *fail_cause =
    RIL_node.update_default_ril_request_last_call_fail_cause_resp();
  fail_cause->cause_code = CALL_FAIL_NORMAL;
  t.expect(RIL_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_wifi_calling_status_success
@description:
GET/SET WIFI_CALLINS_STATUS success cases
==**/
test_result ril_utf_ims_wifi_calling_status_success(user_specified_attrib_t attr)
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

  // ------------------------------------------------------------------------------------

  // Inject: ims_MsgId_REQUEST_GET_WIFI_CALLING_STATUS
  IMS_node.update_default_ims_request_get_wifi_calling_status();
  t.inject(IMS_node);

  // Expect: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_REQ_V01
  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_req();
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_RSP_V01
  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_rsp(
          QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_wifi_call(
          IMS_SETTINGS_WFC_STATUS_NOT_SUPPORTED_V01);
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_wifi_call_preference(
          IMS_SETTINGS_WFC_CALL_PREF_NONE_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_GET_WIFI_CALLING_STATUS (resp)
  IMS_node.update_default_ims_request_get_wifi_calling_status_resp();
  IMS_node.set_ims_request_get_wifi_calling_status_resp_wificallinginfo(
          ims_WifiCallingStatus_WIFI_NOT_SUPPORTED,
          ims_WifiCallingPreference_WIFI_PREF_NONE);
  t.expect(IMS_node);

  // ------------------------------------------------------------------------------------

  // Inject: ims_MsgId_REQUEST_SET_WIFI_CALLING_STATUS
  IMS_node.update_default_ims_request_set_wifi_calling_status();
  IMS_node.set_ims_request_set_wifi_calling_status_wificallinginfo(
          ims_WifiCallingStatus_WIFI_STATUS_ON,
          ims_WifiCallingPreference_WIFI_PREFERRED);
  t.inject(IMS_node);

  // Expect: QMI_IMS_SETTINGS_SET_CLIENT_PROVISIONING_CONFIG_REQ_V01
  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_req();
  QMI_node.set_qmi_ims_settings_set_client_provisioning_config_req_wifi_call(
          IMS_SETTINGS_WFC_STATUS_ON_V01);
  QMI_node.set_qmi_ims_settings_set_client_provisioning_config_req_wifi_call_preference(
          IMS_SETTINGS_WFC_WLAN_PREFERRED_V01);
  t.expect(QMI_node);


  // Inject: QMI_IMS_SETTINGS_SET_CLIENT_PROVISIONING_CONFIG_RSP_V01
  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_rsp(
          QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_SET_WIFI_CALLING_STATUS (resp)
  IMS_node.update_default_ims_request_set_wifi_calling_status_resp();
  t.expect(IMS_node);

  // ------------------------------------------------------------------------------------

  // Inject: ims_MsgId_REQUEST_GET_WIFI_CALLING_STATUS
  IMS_node.update_default_ims_request_get_wifi_calling_status();
  t.inject(IMS_node);

  // Expect: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_REQ_V01
  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_req();
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_RSP_V01
  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_rsp(
          QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_wifi_call(
          IMS_SETTINGS_WFC_STATUS_ON_V01);
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_wifi_call_preference(
          IMS_SETTINGS_WFC_WLAN_PREFERRED_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_GET_WIFI_CALLING_STATUS (resp)
  IMS_node.update_default_ims_request_get_wifi_calling_status_resp();
  IMS_node.set_ims_request_get_wifi_calling_status_resp_wificallinginfo(
          ims_WifiCallingStatus_WIFI_STATUS_ON,
          ims_WifiCallingPreference_WIFI_PREFERRED);
  t.expect(IMS_node);

  // ------------------------------------------------------------------------------------

  // Inject: ims_MsgId_REQUEST_SET_WIFI_CALLING_STATUS
  IMS_node.update_default_ims_request_set_wifi_calling_status();
  IMS_node.set_ims_request_set_wifi_calling_status_wificallinginfo(
          ims_WifiCallingStatus_WIFI_STATUS_ON,
          ims_WifiCallingPreference_WIFI_ONLY);
  t.inject(IMS_node);

  // Expect: QMI_IMS_SETTINGS_SET_CLIENT_PROVISIONING_CONFIG_REQ_V01
  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_req();
  QMI_node.set_qmi_ims_settings_set_client_provisioning_config_req_wifi_call(
          IMS_SETTINGS_WFC_STATUS_ON_V01);
  QMI_node.set_qmi_ims_settings_set_client_provisioning_config_req_wifi_call_preference(
          IMS_SETTINGS_WFC_WLAN_ONLY_V01);
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_SET_CLIENT_PROVISIONING_CONFIG_RSP_V01
  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_rsp(
          QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_SET_WIFI_CALLING_STATUS (resp)
  IMS_node.update_default_ims_request_set_wifi_calling_status_resp();
  t.expect(IMS_node);

  // ------------------------------------------------------------------------------------

  // Inject: ims_MsgId_REQUEST_GET_WIFI_CALLING_STATUS
  IMS_node.update_default_ims_request_get_wifi_calling_status();
  t.inject(IMS_node);

  // Expect: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_REQ_V01
  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_req();
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_RSP_V01
  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_rsp(
          QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_wifi_call(
          IMS_SETTINGS_WFC_STATUS_ON_V01);
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_wifi_call_preference(
          IMS_SETTINGS_WFC_WLAN_ONLY_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_GET_WIFI_CALLING_STATUS (resp)
  IMS_node.update_default_ims_request_get_wifi_calling_status_resp();
  IMS_node.set_ims_request_get_wifi_calling_status_resp_wificallinginfo(
          ims_WifiCallingStatus_WIFI_STATUS_ON,
          ims_WifiCallingPreference_WIFI_ONLY);
  t.expect(IMS_node);

  // ------------------------------------------------------------------------------------

  // Inject: ims_MsgId_REQUEST_SET_WIFI_CALLING_STATUS
  IMS_node.update_default_ims_request_set_wifi_calling_status();
  IMS_node.set_ims_request_set_wifi_calling_status_wificallinginfo(
          ims_WifiCallingStatus_WIFI_STATUS_ON,
          ims_WifiCallingPreference_CELLULAR_PREFERRED);
  t.inject(IMS_node);

  // Expect: QMI_IMS_SETTINGS_SET_CLIENT_PROVISIONING_CONFIG_REQ_V01
  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_req();
  QMI_node.set_qmi_ims_settings_set_client_provisioning_config_req_wifi_call(
          IMS_SETTINGS_WFC_STATUS_ON_V01);
  QMI_node.set_qmi_ims_settings_set_client_provisioning_config_req_wifi_call_preference(
          IMS_SETTINGS_WFC_CELLULAR_PREFERRED_V01);
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_SET_CLIENT_PROVISIONING_CONFIG_RSP_V01
  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_rsp(
          QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_SET_WIFI_CALLING_STATUS (resp)
  IMS_node.update_default_ims_request_set_wifi_calling_status_resp();
  t.expect(IMS_node);

  // ------------------------------------------------------------------------------------

  // Inject: ims_MsgId_REQUEST_GET_WIFI_CALLING_STATUS
  IMS_node.update_default_ims_request_get_wifi_calling_status();
  t.inject(IMS_node);

  // Expect: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_REQ_V01
  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_req();
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_RSP_V01
  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_rsp(
          QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_wifi_call(
          IMS_SETTINGS_WFC_STATUS_ON_V01);
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_wifi_call_preference(
          IMS_SETTINGS_WFC_CELLULAR_PREFERRED_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_GET_WIFI_CALLING_STATUS (resp)
  IMS_node.update_default_ims_request_get_wifi_calling_status_resp();
  IMS_node.set_ims_request_get_wifi_calling_status_resp_wificallinginfo(
          ims_WifiCallingStatus_WIFI_STATUS_ON,
          ims_WifiCallingPreference_CELLULAR_PREFERRED);
  t.expect(IMS_node);

  // ------------------------------------------------------------------------------------

  // Inject: ims_MsgId_REQUEST_SET_WIFI_CALLING_STATUS
  IMS_node.update_default_ims_request_set_wifi_calling_status();
  IMS_node.set_ims_request_set_wifi_calling_status_wificallinginfo(
          ims_WifiCallingStatus_WIFI_STATUS_ON,
          ims_WifiCallingPreference_CELLULAR_ONLY);
  t.inject(IMS_node);

  // Expect: QMI_IMS_SETTINGS_SET_CLIENT_PROVISIONING_CONFIG_REQ_V01
  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_req();
  QMI_node.set_qmi_ims_settings_set_client_provisioning_config_req_wifi_call(
          IMS_SETTINGS_WFC_STATUS_ON_V01);
  QMI_node.set_qmi_ims_settings_set_client_provisioning_config_req_wifi_call_preference(
          IMS_SETTINGS_WFC_CELLULAR_ONLY_V01);
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_SET_CLIENT_PROVISIONING_CONFIG_RSP_V01
  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_rsp(
          QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_SET_WIFI_CALLING_STATUS (resp)
  IMS_node.update_default_ims_request_set_wifi_calling_status_resp();
  t.expect(IMS_node);

  // ------------------------------------------------------------------------------------

  // Inject: ims_MsgId_REQUEST_GET_WIFI_CALLING_STATUS
  IMS_node.update_default_ims_request_get_wifi_calling_status();
  t.inject(IMS_node);

  // Expect: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_REQ_V01
  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_req();
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_RSP_V01
  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_rsp(
          QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_wifi_call(
          IMS_SETTINGS_WFC_STATUS_ON_V01);
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_wifi_call_preference(
          IMS_SETTINGS_WFC_CELLULAR_ONLY_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_GET_WIFI_CALLING_STATUS (resp)
  IMS_node.update_default_ims_request_get_wifi_calling_status_resp();
  IMS_node.set_ims_request_get_wifi_calling_status_resp_wificallinginfo(
          ims_WifiCallingStatus_WIFI_STATUS_ON,
          ims_WifiCallingPreference_CELLULAR_ONLY);
  t.expect(IMS_node);

  // ------------------------------------------------------------------------------------

  // Inject: ims_MsgId_REQUEST_SET_WIFI_CALLING_STATUS
  IMS_node.update_default_ims_request_set_wifi_calling_status();
  IMS_node.set_ims_request_set_wifi_calling_status_wificallinginfo(
          ims_WifiCallingStatus_WIFI_STATUS_OFF,
          ims_WifiCallingPreference_CELLULAR_ONLY);
  t.inject(IMS_node);

  // Expect: QMI_IMS_SETTINGS_SET_CLIENT_PROVISIONING_CONFIG_REQ_V01
  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_req();
  QMI_node.set_qmi_ims_settings_set_client_provisioning_config_req_wifi_call(
          IMS_SETTINGS_WFC_STATUS_OFF_V01);
  QMI_node.set_qmi_ims_settings_set_client_provisioning_config_req_wifi_call_preference(
          IMS_SETTINGS_WFC_CELLULAR_ONLY_V01);
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_SET_CLIENT_PROVISIONING_CONFIG_RSP_V01
  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_rsp(
          QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_SET_WIFI_CALLING_STATUS (resp)
  IMS_node.update_default_ims_request_set_wifi_calling_status_resp();
  t.expect(IMS_node);

  // ------------------------------------------------------------------------------------

  // Inject: ims_MsgId_REQUEST_GET_WIFI_CALLING_STATUS
  IMS_node.update_default_ims_request_get_wifi_calling_status();
  t.inject(IMS_node);

  // Expect: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_REQ_V01
  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_req();
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_RSP_V01
  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_rsp(
          QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_wifi_call(
          IMS_SETTINGS_WFC_STATUS_OFF_V01);
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_wifi_call_preference(
          IMS_SETTINGS_WFC_CELLULAR_ONLY_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_GET_WIFI_CALLING_STATUS (resp)
  IMS_node.update_default_ims_request_get_wifi_calling_status_resp();
  IMS_node.set_ims_request_get_wifi_calling_status_resp_wificallinginfo(
          ims_WifiCallingStatus_WIFI_STATUS_OFF,
          ims_WifiCallingPreference_CELLULAR_ONLY);
  t.expect(IMS_node);

  // ------------------------------------------------------------------------------------

  // Inject: ims_MsgId_REQUEST_SET_WIFI_CALLING_STATUS
  ims_WifiCallingInfo *wci = IMS_node.update_default_ims_request_set_wifi_calling_status();
  wci->has_status = TRUE;
  wci->status = ims_WifiCallingStatus_WIFI_STATUS_OFF;
  t.inject(IMS_node);

  // Expect: QMI_IMS_SETTINGS_SET_CLIENT_PROVISIONING_CONFIG_REQ_V01
  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_req();
  QMI_node.set_qmi_ims_settings_set_client_provisioning_config_req_wifi_call(
          IMS_SETTINGS_WFC_STATUS_OFF_V01);
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_SET_CLIENT_PROVISIONING_CONFIG_RSP_V01
  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_rsp(
          QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_SET_WIFI_CALLING_STATUS (resp)
  IMS_node.update_default_ims_request_set_wifi_calling_status_resp();
  t.expect(IMS_node);

  // ------------------------------------------------------------------------------------

  // Inject: ims_MsgId_REQUEST_GET_WIFI_CALLING_STATUS
  IMS_node.update_default_ims_request_get_wifi_calling_status();
  t.inject(IMS_node);

  // Expect: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_REQ_V01
  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_req();
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_RSP_V01
  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_rsp(
          QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_wifi_call(
          IMS_SETTINGS_WFC_STATUS_OFF_V01);
  QMI_node.set_qmi_ims_settings_get_client_provisioning_config_rsp_wifi_call_preference(
          IMS_SETTINGS_WFC_CALL_PREF_NONE_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_GET_WIFI_CALLING_STATUS (resp)
  IMS_node.update_default_ims_request_get_wifi_calling_status_resp();
  IMS_node.set_ims_request_get_wifi_calling_status_resp_wificallinginfo(
          ims_WifiCallingStatus_WIFI_STATUS_OFF,
          ims_WifiCallingPreference_WIFI_PREF_NONE);
  t.expect(IMS_node);

  // ------------------------------------------------------------------------------------

  t.execute();
  t.print_summary();
  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_wifi_calling_status_failure
@description:
GET/SET WIFI_CALLINS_STATUS failure cases
==**/
test_result ril_utf_ims_wifi_calling_status_failure(user_specified_attrib_t attr)
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

  // ------------------------------------------------------------------------------------
  // Inject: ims_MsgId_REQUEST_GET_WIFI_CALLING_STATUS
  IMS_node.update_default_ims_request_get_wifi_calling_status();
  t.inject(IMS_node);

  // Expect: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_REQ_V01
  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_req();
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_RSP_V01
  QMI_node.update_default_qmi_ims_settings_get_client_provisioning_config_rsp(
          QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_GET_WIFI_CALLING_STATUS (resp)
  IMS_node.update_default_ims_request_get_wifi_calling_status_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  // ------------------------------------------------------------------------------------

  // Inject: ims_MsgId_REQUEST_SET_WIFI_CALLING_STATUS
  IMS_node.update_default_ims_request_set_wifi_calling_status();
  IMS_node.set_ims_request_set_wifi_calling_status_wificallinginfo(
          ims_WifiCallingStatus_WIFI_STATUS_OFF,
          ims_WifiCallingPreference_WIFI_PREFERRED);
  t.inject(IMS_node);

  // Expect: QMI_IMS_SETTINGS_SET_CLIENT_PROVISIONING_CONFIG_REQ_V01
  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_req();
  QMI_node.set_qmi_ims_settings_set_client_provisioning_config_req_wifi_call(
          IMS_SETTINGS_WFC_STATUS_OFF_V01);
  QMI_node.set_qmi_ims_settings_set_client_provisioning_config_req_wifi_call_preference(
          IMS_SETTINGS_WFC_WLAN_PREFERRED_V01);
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_SET_CLIENT_PROVISIONING_CONFIG_RSP_V01
  QMI_node.update_default_qmi_ims_settings_set_client_provisioning_config_rsp(
          QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_SET_WIFI_CALLING_STATUS (resp)
  IMS_node.update_default_ims_request_set_wifi_calling_status_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  // ------------------------------------------------------------------------------------

  // Inject: ims_MsgId_REQUEST_SET_WIFI_CALLING_STATUS
  IMS_node.update_default_ims_request_set_wifi_calling_status();
  t.inject(IMS_node);

  // Expect: ims_MsgId_REQUEST_SET_WIFI_CALLING_STATUS (resp)
  IMS_node.update_default_ims_request_set_wifi_calling_status_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  // ------------------------------------------------------------------------------------

  t.execute();
  t.print_summary();
  return t.get_test_result();
}

/**==
@testname: ril_utf_test_imsa_unsol_rtt_message
@description:
Test rtt_message interfaces
==**/
test_result ril_utf_test_imsa_unsol_rtt_message(user_specified_attrib_t attr)
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

  // ------------------------------------------------------------------------------------
  // Inject: QMI_IMS_SETTINGS_RTT_RCVD_DATA_IND_V01
  QMI_node.update_default_qmi_imss_rtt_message_ind_v01();
  t.inject(QMI_node);

  IMS_node.update_default_ims_unsol_rtt_message();
  IMS_node.set_ims_rtt_message_text("Hi");
  t.expect(IMS_node);

  t.execute();
  t.print_summary();
  return t.get_test_result();
}


/**==
@testname: ril_utf_test_imsa_send_rtt_message
@description:
Test rtt_message interfaces
==**/
test_result ril_utf_test_imsa_send_rtt_message(user_specified_attrib_t attr)
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

  // ------------------------------------------------------------------------------------
  // Inject: ims_MsgId_REQUEST_SEND_RTT_MSG
  IMS_node.update_default_ims_request_send_rtt_message();
  IMS_node.set_ims_rtt_message_text("Hi");
  t.inject(IMS_node);

  // Expect: QMI_IMS_SETTINGS_SEND_RTT_REQ_V01
  QMI_node.update_default_qmi_ims_settings_send_rtt_message_req();
  QMI_node.set_qmi_ims_settings_set_ims_service_rtt_message("Hi");
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_SEND_RTT_RESP_V01
  QMI_node.update_default_qmi_ims_settings_set_ims_service_send_rtt_message_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_send_rtt_message_resp();
  t.expect(IMS_node);

  t.execute();
  t.print_summary();
  return t.get_test_result();
}

/**==
@testname: ril_utf_test_imsa_send_rtt_message_fail
@description:
Test rtt_message interfaces
==**/
test_result ril_utf_test_imsa_send_rtt_message_fail(user_specified_attrib_t attr)
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

  // ------------------------------------------------------------------------------------
  // Inject: ims_MsgId_REQUEST_SEND_RTT_MSG
  IMS_node.update_default_ims_request_send_rtt_message();
  IMS_node.set_ims_rtt_message_text("Hi");
  t.inject(IMS_node);

  // Expect: QMI_IMS_SETTINGS_SEND_RTT_REQ_V01
  QMI_node.update_default_qmi_ims_settings_send_rtt_message_req();
  QMI_node.set_qmi_ims_settings_set_ims_service_rtt_message("Hi");
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_SEND_RTT_RESP_V01
  QMI_node.update_default_qmi_ims_settings_set_ims_service_send_rtt_message_resp(QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  IMS_node.update_default_ims_request_send_rtt_message_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  t.execute();
  t.print_summary();
  return t.get_test_result();
}

/**==
@testname: ril_utf_test_imsa_send_rtt_message_fail_length
@description:
Test rtt_message interfaces
==**/
test_result ril_utf_test_imsa_send_rtt_message_fail_length(user_specified_attrib_t attr)
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

  // ------------------------------------------------------------------------------------
  // Inject: ims_MsgId_REQUEST_SEND_RTT_MSG
  IMS_node.update_default_ims_request_send_rtt_message();
  std::string msg(300,'h');
  IMS_node.set_ims_rtt_message_text(msg.c_str());
  t.inject(IMS_node);

  IMS_node.update_default_ims_request_send_rtt_message_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  t.execute();
  t.print_summary();
  return t.get_test_result();
}

/**==
@testname: ril_utf_test_imsa_send_rtt_message_fail_empty
@description:
Test rtt_message interfaces
==**/
test_result ril_utf_test_imsa_send_rtt_message_fail_empty(user_specified_attrib_t attr)
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

  // ------------------------------------------------------------------------------------
  // Inject: ims_MsgId_REQUEST_SEND_RTT_MSG
  IMS_node.update_default_ims_request_send_rtt_message();
  t.inject(IMS_node);

  IMS_node.update_default_ims_request_send_rtt_message_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  t.execute();
  t.print_summary();
  return t.get_test_result();
}


/**==
@testname: ril_utf_imsa_geolocation
@description:
Test geolocation interfaces
==**/
test_result ril_utf_test_imsa_geolocation_001(user_specified_attrib_t attr)
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

  // ------------------------------------------------------------------------------------
  // Inject: QMI_IMSA_GEO_LOC_STATUS_IND_V01
  QMI_node.update_default_qmi_imsa_geo_loc_status_ind();
  QMI_node.set_qmi_imsa_geo_loc_status_ind_latitude(32.7152778);
  QMI_node.set_qmi_imsa_geo_loc_status_ind_longitude(-117.1563889);
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_REQUEST_GEOLOCATION
  IMS_node.update_default_ims_unsol_request_geolocation();
  IMS_node.set_ims_geolocation_info_lat(32.7152778);
  IMS_node.set_ims_geolocation_info_lon(-117.1563889);
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_SEND_GEOLOCATION_INFO
  IMS_node.update_default_ims_request_send_geolocation_info();
  IMS_node.set_ims_geolocation_info_lat(32.7152778);
  IMS_node.set_ims_geolocation_info_lon(-117.1563889);
  IMS_node.set_ims_geolocation_info_city("San Diego");
  t.inject(IMS_node);

  // Expect: QMI_IMSA_SET_GEO_LOC_REQ_V01
  QMI_node.update_default_qmi_imsa_set_geo_loc_req();
  QMI_node.set_qmi_imsa_set_geo_loc_latitude(32.7152778);
  QMI_node.set_qmi_imsa_set_geo_loc_longitude(-117.1563889);
  QMI_node.set_qmi_imsa_set_geo_loc_city("San Diego");
  t.expect(QMI_node);

  // Inject: QMI_IMSA_SET_GEO_LOC_RSP_V01
  QMI_node.update_default_qmi_imsa_set_geo_loc_rsp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_SEND_GEOLOCATION_INFO
  IMS_node.update_default_ims_request_send_geolocation_info_resp();
  t.expect(IMS_node);

  t.execute();
  t.print_summary();
  return t.get_test_result();
}

/**==
@testname: ril_utf_imsa_geolocation
@description:
Test geolocation interfaces
==**/
test_result ril_utf_test_imsa_geolocation_002(user_specified_attrib_t attr)
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

  // ------------------------------------------------------------------------------------
  // Inject: QMI_IMSA_GEO_LOC_STATUS_IND_V01
  QMI_node.update_default_qmi_imsa_geo_loc_status_ind();
  QMI_node.set_qmi_imsa_geo_loc_status_ind_latitude(17.440334558486942);
  QMI_node.set_qmi_imsa_geo_loc_status_ind_longitude(78.380584716796889);
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_REQUEST_GEOLOCATION
  IMS_node.update_default_ims_unsol_request_geolocation();
  IMS_node.set_ims_geolocation_info_lat(17.440334558486942);
  IMS_node.set_ims_geolocation_info_lon(78.380584716796889);
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_SEND_GEOLOCATION_INFO
  IMS_node.update_default_ims_request_send_geolocation_info();
  IMS_node.set_ims_geolocation_info_lat(17.440334558486942);
  IMS_node.set_ims_geolocation_info_lon(78.380584716796889);
  IMS_node.set_ims_geolocation_info_countryCode("IN");
  IMS_node.set_ims_geolocation_info_country("India");
  IMS_node.set_ims_geolocation_info_state("Telangana");
  IMS_node.set_ims_geolocation_info_city("Hyderabad");
  IMS_node.set_ims_geolocation_info_postalCode("500081");
  IMS_node.set_ims_geolocation_info_street("HITEC City");
  IMS_node.set_ims_geolocation_info_houseNumber("10000");
  t.inject(IMS_node);

  // Expect: QMI_IMSA_SET_GEO_LOC_REQ_V01
  QMI_node.update_default_qmi_imsa_set_geo_loc_req();
  QMI_node.set_qmi_imsa_set_geo_loc_latitude(17.440334558486942);
  QMI_node.set_qmi_imsa_set_geo_loc_longitude(78.380584716796889);
  QMI_node.set_qmi_imsa_set_geo_loc_countryCode("IN");
  QMI_node.set_qmi_imsa_set_geo_loc_countryName("India");
  QMI_node.set_qmi_imsa_set_geo_loc_state("Telangana");
  QMI_node.set_qmi_imsa_set_geo_loc_city("Hyderabad");
  QMI_node.set_qmi_imsa_set_geo_loc_zipCode("500081");
  QMI_node.set_qmi_imsa_set_geo_loc_street("HITEC City");
  QMI_node.set_qmi_imsa_set_geo_loc_housenumber("10000");
  t.expect(QMI_node);

  // Inject: QMI_IMSA_SET_GEO_LOC_RSP_V01
  QMI_node.update_default_qmi_imsa_set_geo_loc_rsp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_SEND_GEOLOCATION_INFO
  IMS_node.update_default_ims_request_send_geolocation_info_resp();
  t.expect(IMS_node);

  t.execute();
  t.print_summary();
  return t.get_test_result();
}

/**==
@testname: ril_utf_test_imsa_geolocation_003
@description:
Test geolocation interfaces
Failure case
==**/
test_result ril_utf_test_imsa_geolocation_003(user_specified_attrib_t attr)
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

  // ------------------------------------------------------------------------------------
  // Inject: QMI_IMSA_GEO_LOC_STATUS_IND_V01
  QMI_node.update_default_qmi_imsa_geo_loc_status_ind();
  QMI_node.set_qmi_imsa_geo_loc_status_ind_latitude(32.7152778);
  QMI_node.set_qmi_imsa_geo_loc_status_ind_longitude(-117.1563889);
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_REQUEST_GEOLOCATION
  IMS_node.update_default_ims_unsol_request_geolocation();
  IMS_node.set_ims_geolocation_info_lat(32.7152778);
  IMS_node.set_ims_geolocation_info_lon(-117.1563889);
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_SEND_GEOLOCATION_INFO
  IMS_node.update_default_ims_request_send_geolocation_info();
  IMS_node.set_ims_geolocation_info_lat(32.7152778);
  IMS_node.set_ims_geolocation_info_lon(-117.1563889);
  IMS_node.set_ims_geolocation_info_city("San Diego");
  t.inject(IMS_node);

  // Expect: QMI_IMSA_SET_GEO_LOC_REQ_V01
  QMI_node.update_default_qmi_imsa_set_geo_loc_req();
  QMI_node.set_qmi_imsa_set_geo_loc_latitude(32.7152778);
  QMI_node.set_qmi_imsa_set_geo_loc_longitude(-117.1563889);
  QMI_node.set_qmi_imsa_set_geo_loc_city("San Diego");
  t.expect(QMI_node);

  // Inject: QMI_IMSA_SET_GEO_LOC_RSP_V01
  QMI_node.update_default_qmi_imsa_set_geo_loc_rsp(QMI_RESULT_FAILURE_V01, QMI_ERR_INTERNAL_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_SEND_GEOLOCATION_INFO
  IMS_node.update_default_ims_request_send_geolocation_info_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  t.execute();
  t.print_summary();
  return t.get_test_result();
}


/**==
@testname: ril_utf_test_ims_sub_config_001
@description:
Test getImsSubConfig
==**/
test_result ril_utf_test_ims_sub_config_001(user_specified_attrib_t attr)
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

  // Inject: ims_MsgId_REQUEST_GET_IMS_SUB_CONFIG
  IMS_node.update_default_ims_request_get_ims_sub_config();
  t.inject(IMS_node);

  // Expect: QMI_DMS_GET_DEVICE_CAP_REQ_V01
  QMI_node.update_default_qmi_dms_get_device_cap_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_DEVICE_CAP_RESP_V01
  QMI_node.update_default_qmi_dms_get_device_cap_resp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_default_qmi_dms_get_device_cap_resp_v01_max_ims_instances(1);
  dms_ims_capability_type_v01 *ims_cap = QMI_node.set_default_qmi_dms_get_device_cap_resp_v01_ims_capability(1);
  ims_cap[0].subscription = DMS_PRIMARY_SUBS_V01;
  ims_cap[0].enabled = 1;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_GET_IMS_SUB_CONFIG (Resp)
  IMS_node.update_default_ims_request_get_ims_sub_config_resp();
  IMS_node.set_ims_request_get_ims_sub_config_resp_simultStackCount(1);
  uint64_t ims_stack[2] = {1, 0};
  IMS_node.set_ims_request_get_ims_sub_config_resp_imsStackEnabled(ims_stack, 1);
  t.expect(IMS_node);

  t.execute();
  t.print_summary();
  return t.get_test_result();
}

/**==
@testname: ril_utf_test_ims_sub_config_002
@description:
Test getImsSubConfig
==**/
test_result ril_utf_test_ims_sub_config_002(user_specified_attrib_t attr)
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

  // Inject: ims_MsgId_REQUEST_GET_IMS_SUB_CONFIG
  IMS_node.update_default_ims_request_get_ims_sub_config();
  t.inject(IMS_node);

  // Expect: QMI_DMS_GET_DEVICE_CAP_REQ_V01
  QMI_node.update_default_qmi_dms_get_device_cap_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_DEVICE_CAP_RESP_V01
  QMI_node.update_default_qmi_dms_get_device_cap_resp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_default_qmi_dms_get_device_cap_resp_v01_max_ims_instances(1);
  dms_ims_capability_type_v01 *ims_cap = QMI_node.set_default_qmi_dms_get_device_cap_resp_v01_ims_capability(2);
  ims_cap[0].subscription = DMS_PRIMARY_SUBS_V01;
  ims_cap[0].enabled = 1;
  ims_cap[1].subscription = DMS_SECONDARY_SUBS_V01;
  ims_cap[1].enabled = 1;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_GET_IMS_SUB_CONFIG (Resp)
  IMS_node.update_default_ims_request_get_ims_sub_config_resp();
  IMS_node.set_ims_request_get_ims_sub_config_resp_simultStackCount(1);
  uint64_t ims_stack[2] = {1, 1};
  IMS_node.set_ims_request_get_ims_sub_config_resp_imsStackEnabled(ims_stack, 2);
  t.expect(IMS_node);

  // Inject: QMI_DMS_IMS_CAPABILITY_IND_V01
  dms_ims_capability_type_v01 *qmi_ims_cap = QMI_node.update_default_qmi_dms_ims_capability_ind_v01(2);
  qmi_ims_cap[0].subscription = DMS_PRIMARY_SUBS_V01;
  qmi_ims_cap[0].enabled = 0;
  qmi_ims_cap[1].subscription = DMS_SECONDARY_SUBS_V01;
  qmi_ims_cap[1].enabled = 1;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_IMS_SUB_CONFIG_CHANGED
  IMS_node.update_default_ims_unsol_ims_sub_config_changed();
  IMS_node.set_ims_unsol_ims_sub_config_changed_simultStackCount(1);
  uint64_t ims_stack_1[2] = {0, 1};
  IMS_node.set_ims_unsol_ims_sub_config_changed_imsStackEnabled(ims_stack_1, 2);
  t.expect(IMS_node);

  t.execute();
  t.print_summary();
  return t.get_test_result();
}

/**==
@testname: ril_utf_test_ims_sub_config_003
@description:
Test getImsSubConfig
==**/
test_result ril_utf_test_ims_sub_config_003(user_specified_attrib_t attr)
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

  // Inject: QMI_DMS_IMS_CAPABILITY_IND_V01
  dms_ims_capability_type_v01 *qmi_ims_cap = QMI_node.update_default_qmi_dms_ims_capability_ind_v01(2);
  qmi_ims_cap[0].subscription = DMS_PRIMARY_SUBS_V01;
  qmi_ims_cap[0].enabled = 0;
  qmi_ims_cap[1].subscription = DMS_SECONDARY_SUBS_V01;
  qmi_ims_cap[1].enabled = 1;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_IMS_SUB_CONFIG_CHANGED
  IMS_node.update_default_ims_unsol_ims_sub_config_changed();
  //IMS_node.set_ims_unsol_ims_sub_config_changed_simultStackCount(1);
  uint64_t ims_stack_1[2] = {0, 1};
  IMS_node.set_ims_unsol_ims_sub_config_changed_imsStackEnabled(ims_stack_1, 2);
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_GET_IMS_SUB_CONFIG
  IMS_node.update_default_ims_request_get_ims_sub_config();
  t.inject(IMS_node);

  // Expect: QMI_DMS_GET_DEVICE_CAP_REQ_V01
  QMI_node.update_default_qmi_dms_get_device_cap_req_v01();
  t.expect(QMI_node);

  // Inject: QMI_DMS_GET_DEVICE_CAP_RESP_V01
  QMI_node.update_default_qmi_dms_get_device_cap_resp_v01(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_default_qmi_dms_get_device_cap_resp_v01_max_ims_instances(1);
  dms_ims_capability_type_v01 *ims_cap = QMI_node.set_default_qmi_dms_get_device_cap_resp_v01_ims_capability(2);
  ims_cap[0].subscription = DMS_PRIMARY_SUBS_V01;
  ims_cap[0].enabled = 1;
  ims_cap[1].subscription = DMS_SECONDARY_SUBS_V01;
  ims_cap[1].enabled = 1;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_GET_IMS_SUB_CONFIG (Resp)
  IMS_node.update_default_ims_request_get_ims_sub_config_resp();
  IMS_node.set_ims_request_get_ims_sub_config_resp_simultStackCount(1);
  uint64_t ims_stack[2] = {1, 1};
  IMS_node.set_ims_request_get_ims_sub_config_resp_imsStackEnabled(ims_stack, 2);
  t.expect(IMS_node);

  // Inject: QMI_DMS_IMS_CAPABILITY_IND_V01
  qmi_ims_cap = QMI_node.update_default_qmi_dms_ims_capability_ind_v01(2);
  qmi_ims_cap[0].subscription = DMS_PRIMARY_SUBS_V01;
  qmi_ims_cap[0].enabled = 1;
  qmi_ims_cap[1].subscription = DMS_SECONDARY_SUBS_V01;
  qmi_ims_cap[1].enabled = 0;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_IMS_SUB_CONFIG_CHANGED
  IMS_node.update_default_ims_unsol_ims_sub_config_changed();
  IMS_node.set_ims_unsol_ims_sub_config_changed_simultStackCount(1);
  uint64_t ims_stack_2[2] = {1, 0};
  IMS_node.set_ims_unsol_ims_sub_config_changed_imsStackEnabled(ims_stack_2, 2);
  t.expect(IMS_node);

  t.execute();
  t.print_summary();
  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_end_call_err_malformed_001
@description:
Modem returns END_CALL response with QMI_ERR_MALFORMED_MSG
RIL retries END_CALL request
==**/
test_result ril_utf_ims_end_call_err_malformed_001(user_specified_attrib_t attr)
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
  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CC_IN_PROGRESS_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // Media ID
  voice_media_id_with_id_type_v02 *media_id = QMI_node.set_qmi_voice_all_call_status_ind_media_id(1);
  media_id[0].call_id = 1;
  media_id[0].media_id = 2;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(1);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_FAILURE_V01, QMI_ERR_MALFORMED_MSG_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(1);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
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

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);

  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_end_call_err_malformed_002
@description:
Back to back CS and IMS MO calls
IMS call with conn_index = 2
Modem returns END_CALL response with QMI_ERR_MALFORMED_MSG
RIL retries END_CALL request
==**/
test_result ril_utf_ims_end_call_err_malformed_002(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;
  AndroidTelTestnode RIL_node;

  t.set_test_id(__func__);
  t.set_test_attribute("IMS");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: RIL_REQUEST_DIAL
  RIL_node.update_default_ril_request_dial("2428536960");
  t.inject(RIL_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_V02);
#ifdef RIL_UTF_L_MASTER
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_CS_ONLY_V02);
#else
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_GSM_WCDMA_TDSCDMA_V02);
#endif
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_GSM_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

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
#ifdef RIL_UTF_L_MASTER
  RIL_node.set_ril_request_get_current_calls_resp_number(0, "2428536960");
  call[0]->numberPresentation = 0;
  call[0]->namePresentation = 2;
#endif
  t.expect(RIL_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_GSM_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

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
#ifdef RIL_UTF_L_MASTER
  RIL_node.set_ril_request_get_current_calls_resp_number(0, "2428536960");
  call[0]->numberPresentation = 0;
  call[0]->namePresentation = 2;
#endif
  t.expect(RIL_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_GSM_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

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
#ifdef RIL_UTF_L_MASTER
  RIL_node.set_ril_request_get_current_calls_resp_number(0, "2428536960");
  call[0]->numberPresentation = 0;
  call[0]->namePresentation = 2;
#endif
  t.expect(RIL_node);

  // Inject: RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND
  RIL_node.update_default_ril_request_hangup_foreground_resume_background();
  t.inject(RIL_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_DISCONNECTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_GSM_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
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
  call_info[0].mode = CALL_MODE_GSM_V02;
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


  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CC_IN_PROGRESS_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // Media ID
  voice_media_id_with_id_type_v02 *media_id = QMI_node.set_qmi_voice_all_call_status_ind_media_id(1);
  media_id[0].call_id = 1;
  media_id[0].media_id = 2;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 2;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 2;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 2;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 2;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(2);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_FAILURE_V01, QMI_ERR_MALFORMED_MSG_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(1);
  t.inject(QMI_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  end = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end[0].call_id = 1;
  end[0].call_end_reason = CALL_END_CAUSE_NORMAL_CALL_CLEARING_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);

  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 2;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_mo_call_called_party_alerting
@description: MO LTE call through IMS with called party alerting ind
==**/
test_result ril_utf_ims_mo_call_called_party_alerting(user_specified_attrib_t attr)
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
  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CC_IN_PROGRESS_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // Media ID
  voice_media_id_with_id_type_v02 *media_id = QMI_node.set_qmi_voice_all_call_status_ind_media_id(1);
  media_id[0].call_id = 1;
  media_id[0].media_id = 2;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  voice_volte_is_called_party_ringing_type_v02 *called_party_ringing =
      QMI_node.set_qmi_voice_all_call_status_ind_is_called_party_ringing(1);
  called_party_ringing[0].call_id = 1;
  called_party_ringing[0].is_called_party_ringing = 1;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->has_isCalledPartyRinging = TRUE;
  calls[0]->isCalledPartyRinging = 1;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  /*
  called_party_ringing = QMI_node.set_qmi_voice_all_call_status_ind_is_called_party_ringing(1);
  called_party_ringing[0].call_id = 1;
  called_party_ringing[0].is_called_party_ringing = 0;
  */
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
//  calls[0]->has_isCalledPartyRinging = TRUE;
//  calls[0]->isCalledPartyRinging = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(1);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
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

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);


  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_conf_participant_status_001
@description:
==**/
test_result ril_utf_ims_conf_participant_status_001(user_specified_attrib_t attr)
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

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_ADD_PARTICIPANT
  IMS_node.update_default_ims_request_add_participant("3333-33");
  t.inject(IMS_node);

  // Expect: QMI_VOICE_MANAGE_IP_CALLS_REQ_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_req_msg((voip_sups_type_enum_v02)0x0a);
  QMI_node.set_qmi_voice_manage_ip_calls_req_call_id(1);
  char *sip_uri = QMI_node.set_qmi_voice_manage_ip_calls_sip_uri();
  strlcpy(sip_uri, "3333-33", strlen("3333-33")+1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_CONF_PARTICIPANT_STATUS_INFO_IND_V02
  QMI_node.update_default_qmi_voice_conf_participant_status_info_ind_msg(1, "tel:3333-33;phone-context=vzwims.com");
  QMI_node.set_qmi_voice_conf_participant_status_info_ind_op_status(
          VOICE_CONF_PARTICIPANT_ADD_V02, 180);  // Ringing - 180
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_PARTICIPANT_STATUS_INFO
  IMS_node.update_default_ims_unsol_participant_status_info();
  IMS_node.set_ims_unsol_participant_status_info_call_id(1);
  IMS_node.set_ims_unsol_participant_status_info_operation(ims_ConfParticipantOperation_ADD);
  IMS_node.set_ims_unsol_participant_status_info_sip_status(180);
  IMS_node.set_ims_unsol_participant_status_info_participant_uri("tel:3333-33;phone-context=vzwims.com");
  t.expect(IMS_node);

  // Inject: QMI_VOICE_CONF_PARTICIPANT_STATUS_INFO_IND_V02
  QMI_node.update_default_qmi_voice_conf_participant_status_info_ind_msg(1, "tel:3333-33;phone-context=vzwims.com");
  QMI_node.set_qmi_voice_conf_participant_status_info_ind_op_status(
          VOICE_CONF_PARTICIPANT_ADD_V02, 200);  // Accept (200 OK)
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_PARTICIPANT_STATUS_INFO
  IMS_node.update_default_ims_unsol_participant_status_info();
  IMS_node.set_ims_unsol_participant_status_info_call_id(1);
  IMS_node.set_ims_unsol_participant_status_info_operation(ims_ConfParticipantOperation_ADD);
  IMS_node.set_ims_unsol_participant_status_info_sip_status(200);
  IMS_node.set_ims_unsol_participant_status_info_participant_uri("tel:3333-33;phone-context=vzwims.com");
  t.expect(IMS_node);

  // Inject: QMI_VOICE_MANAGE_IP_CALLS_RESP_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_ADD_PARTICIPANT
  IMS_node.update_default_ims_request_add_participant_resp();
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_conf_participant_status_002
@description:
==**/
test_result ril_utf_ims_conf_participant_status_002(user_specified_attrib_t attr)
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

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice     = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy     = 0;
  calls[0]->has_numberPresentation = TRUE;
  calls[0]->numberPresentation     = 1;
  calls[0]->has_namePresentation = TRUE;
  calls[0]->namePresentation     = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_ADD_PARTICIPANT
  IMS_node.update_default_ims_request_add_participant("3333-33");
  t.inject(IMS_node);

  // Expect: QMI_VOICE_MANAGE_IP_CALLS_REQ_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_req_msg((voip_sups_type_enum_v02)0x0a);
  QMI_node.set_qmi_voice_manage_ip_calls_req_call_id(1);
  char *sip_uri = QMI_node.set_qmi_voice_manage_ip_calls_sip_uri();
  strlcpy(sip_uri, "3333-33", strlen("3333-33")+1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_CONF_PARTICIPANT_STATUS_INFO_IND_V02
  QMI_node.update_default_qmi_voice_conf_participant_status_info_ind_msg(1, "tel:3333-33;phone-context=vzwims.com");
  QMI_node.set_qmi_voice_conf_participant_status_info_ind_op_status(
          VOICE_CONF_PARTICIPANT_ADD_V02, 180);   // Ringing - 180
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_PARTICIPANT_STATUS_INFO
  IMS_node.update_default_ims_unsol_participant_status_info();
  IMS_node.set_ims_unsol_participant_status_info_call_id(1);
  IMS_node.set_ims_unsol_participant_status_info_operation(ims_ConfParticipantOperation_ADD);
  IMS_node.set_ims_unsol_participant_status_info_sip_status(180);
  IMS_node.set_ims_unsol_participant_status_info_participant_uri("tel:3333-33;phone-context=vzwims.com");
  t.expect(IMS_node);

  // Inject: QMI_VOICE_CONF_PARTICIPANT_STATUS_INFO_IND_V02
  QMI_node.update_default_qmi_voice_conf_participant_status_info_ind_msg(1, "tel:3333-33;phone-context=vzwims.com");
  QMI_node.set_qmi_voice_conf_participant_status_info_ind_op_status(
          VOICE_CONF_PARTICIPANT_ADD_V02, 603);  // Reject - sip603
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_PARTICIPANT_STATUS_INFO
  IMS_node.update_default_ims_unsol_participant_status_info();
  IMS_node.set_ims_unsol_participant_status_info_call_id(1);
  IMS_node.set_ims_unsol_participant_status_info_operation(ims_ConfParticipantOperation_ADD);
  IMS_node.set_ims_unsol_participant_status_info_sip_status(603);
  IMS_node.set_ims_unsol_participant_status_info_participant_uri("tel:3333-33;phone-context=vzwims.com");
  t.expect(IMS_node);

  // Inject: QMI_VOICE_MANAGE_IP_CALLS_RESP_V02
  QMI_node.update_default_qmi_voice_manage_ip_calls_resp_msg(QMI_RESULT_FAILURE_V01, QMI_ERR_GENERAL_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_ADD_PARTICIPANT
  IMS_node.update_default_ims_request_add_participant_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_encrypted_call_001
@description: MO VoLTE encrypted call
==**/
test_result ril_utf_ims_encrypted_call_001(user_specified_attrib_t attr)
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

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  IMS_node.set_ims_request_dial_isencrypted(TRUE);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  QMI_node.set_qmi_voice_dial_call_req_is_secure_call(TRUE);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CC_IN_PROGRESS_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // Media ID
  voice_media_id_with_id_type_v02 *media_id = QMI_node.set_qmi_voice_all_call_status_ind_media_id(1);
  media_id[0].call_id = 1;
  media_id[0].media_id = 2;
  // Is secure call
  voice_volte_is_secure_call_type_v02 *secure_call = QMI_node.set_qmi_voice_all_call_status_ind_is_secure_call(1);
  secure_call[0].call_id = 1;
  secure_call[0].is_secure_call = TRUE;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->has_isEncrypted = TRUE;
  calls[0]->isEncrypted = TRUE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // Is secure call
  secure_call = QMI_node.set_qmi_voice_all_call_status_ind_is_secure_call(1);
  secure_call[0].call_id = 1;
  secure_call[0].is_secure_call = TRUE;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  calls[0]->has_isEncrypted = TRUE;
  calls[0]->isEncrypted = TRUE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // Is secure call
  secure_call = QMI_node.set_qmi_voice_all_call_status_ind_is_secure_call(1);
  secure_call[0].call_id = 1;
  secure_call[0].is_secure_call = TRUE;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  calls[0]->has_isEncrypted = TRUE;
  calls[0]->isEncrypted = TRUE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // Is secure call
  secure_call = QMI_node.set_qmi_voice_all_call_status_ind_is_secure_call(1);
  secure_call[0].call_id = 1;
  secure_call[0].is_secure_call = TRUE;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  calls[0]->has_isEncrypted = TRUE;
  calls[0]->isEncrypted = TRUE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(1);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // Is secure call
  secure_call = QMI_node.set_qmi_voice_all_call_status_ind_is_secure_call(1);
  secure_call[0].call_id = 1;
  secure_call[0].is_secure_call = TRUE;
  // End reason
  voice_call_end_reason_type_v02* end = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end[0].call_id = 1;
  end[0].call_end_reason = CALL_END_CAUSE_NORMAL_CALL_CLEARING_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);


  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  calls[0]->has_isEncrypted = TRUE;
  calls[0]->isEncrypted = TRUE;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_encrypted_call_002
@description: MO VoLTE encrypted call
==**/
test_result ril_utf_ims_encrypted_call_002(user_specified_attrib_t attr)
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

  property_set("vendor.radio.support.encrypted_calls", "0");

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  IMS_node.set_ims_request_dial_isencrypted(TRUE);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CC_IN_PROGRESS_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // Media ID
  voice_media_id_with_id_type_v02 *media_id = QMI_node.set_qmi_voice_all_call_status_ind_media_id(1);
  media_id[0].call_id = 1;
  media_id[0].media_id = 2;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(1);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
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

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);


  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_encrypted_call_003
@description: MO VoLTE encrypted call
==**/
test_result ril_utf_ims_encrypted_call_003(user_specified_attrib_t attr)
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

  property_set("vendor.radio.support.encrypted_calls", "1");

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  IMS_node.set_ims_request_dial_isencrypted(TRUE);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  QMI_node.set_qmi_voice_dial_call_req_is_secure_call(TRUE);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CC_IN_PROGRESS_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // Media ID
  voice_media_id_with_id_type_v02 *media_id = QMI_node.set_qmi_voice_all_call_status_ind_media_id(1);
  media_id[0].call_id = 1;
  media_id[0].media_id = 2;
  // Is secure call
  voice_volte_is_secure_call_type_v02 *secure_call = QMI_node.set_qmi_voice_all_call_status_ind_is_secure_call(1);
  secure_call[0].call_id = 1;
  secure_call[0].is_secure_call = TRUE;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->has_isEncrypted = TRUE;
  calls[0]->isEncrypted = TRUE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // Is secure call
  secure_call = QMI_node.set_qmi_voice_all_call_status_ind_is_secure_call(1);
  secure_call[0].call_id = 1;
  secure_call[0].is_secure_call = TRUE;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  calls[0]->has_isEncrypted = TRUE;
  calls[0]->isEncrypted = TRUE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // Is secure call
  secure_call = QMI_node.set_qmi_voice_all_call_status_ind_is_secure_call(1);
  secure_call[0].call_id = 1;
  secure_call[0].is_secure_call = TRUE;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  calls[0]->has_isEncrypted = TRUE;
  calls[0]->isEncrypted = TRUE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // Is secure call
  secure_call = QMI_node.set_qmi_voice_all_call_status_ind_is_secure_call(1);
  secure_call[0].call_id = 1;
  secure_call[0].is_secure_call = TRUE;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  calls[0]->has_isEncrypted = TRUE;
  calls[0]->isEncrypted = TRUE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(1);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // Is secure call
  secure_call = QMI_node.set_qmi_voice_all_call_status_ind_is_secure_call(1);
  secure_call[0].call_id = 1;
  secure_call[0].is_secure_call = TRUE;
  // End reason
  voice_call_end_reason_type_v02* end = QMI_node.set_qmi_voice_all_call_status_ind_call_end_reason(1);
  end[0].call_id = 1;
  end[0].call_end_reason = CALL_END_CAUSE_NORMAL_CALL_CLEARING_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);


  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  calls[0]->has_isEncrypted = TRUE;
  calls[0]->isEncrypted = TRUE;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_mo_call_peer_info_001
@description: IMS MO call with peer info (Remote Party Conf Capability with conf_is_enabled TRUE)
==**/
test_result ril_utf_ims_mo_call_peer_info_001(user_specified_attrib_t attr)
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

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  IMS_node.set_ims_request_dial_isencrypted(FALSE);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  QMI_node.set_qmi_voice_dial_call_req_is_secure_call(FALSE);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CC_IN_PROGRESS_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // Media ID
  voice_media_id_with_id_type_v02 *media_id = QMI_node.set_qmi_voice_all_call_status_ind_media_id(1);
  media_id[0].call_id = 1;
  media_id[0].media_id = 2;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // remote_party_conf_capability
  voice_remote_party_conf_cap_type_v02 *remote_party_conf_capability =
    QMI_node.set_qmi_voice_all_call_status_ind_remote_party_conf_capability(1);
  remote_party_conf_capability[0].call_id = 1;
  remote_party_conf_capability[0].conf_is_enabled = TRUE;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  calls[0]->has_peerConfAbility = TRUE;
  calls[0]->peerConfAbility.has_isVideoConfSupported = TRUE;
  calls[0]->peerConfAbility.isVideoConfSupported = TRUE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(1);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
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

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->has_peerConfAbility = TRUE;
  calls[0]->peerConfAbility.has_isVideoConfSupported = TRUE;
  calls[0]->peerConfAbility.isVideoConfSupported = TRUE;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_mo_call_peer_info_002
@description: IMS MO call with peer info (Remote Party Conf Capability with conf_is_enabled FALSE)
==**/
test_result ril_utf_ims_mo_call_peer_info_002(user_specified_attrib_t attr)
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

  // Inject: ims_MsgId_REQUEST_DIAL
  ims_Dial* ptr = IMS_node.update_default_ims_request_dial("2428536960");
  ptr->has_callDetails = true;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  IMS_node.set_ims_request_dial_isencrypted(FALSE);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_DIAL_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_dial_call_req_msg("2428536960");
  QMI_node.set_qmi_voice_dial_call_req_call_type(CALL_TYPE_VOICE_V02);
  QMI_node.set_qmi_voice_dial_call_req_audio_attrib(3);
  QMI_node.set_qmi_voice_dial_call_req_video_attrib(0);
  QMI_node.set_qmi_voice_dial_call_req_service_type(VOICE_DIAL_CALL_SRV_TYPE_AUTOMATIC_V02);
  QMI_node.set_qmi_voice_dial_call_req_is_secure_call(FALSE);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_DIAL_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_dial_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_dial_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  voice_call_info2_type_v02 *call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CC_IN_PROGRESS_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  voice_remote_party_number2_type_v02 *number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // Media ID
  voice_media_id_with_id_type_v02 *media_id = QMI_node.set_qmi_voice_all_call_status_ind_media_id(1);
  media_id[0].call_id = 1;
  media_id[0].media_id = 2;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  ims_CallList_Call** calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ORIGINATING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_DIAL (resp)
  IMS_node.update_default_ims_request_dial_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_DIALING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_ALERTING_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ALERTING;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_AUTOMATIC;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_CONVERSATION_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
  // remote_party_number
  number_list = QMI_node.set_qmi_voice_all_call_status_ind_remote_party_number(1);
  number_list[0].call_id = 1;
  number_list[0].number_len = strlen("2428536960");
  strlcpy(number_list[0].number, "2428536960", strlen("2428536960")+1);
  number_list[0].number_pi = PRESENTATION_NUM_ALLOWED_V02;
  // remote_party_conf_capability
  voice_remote_party_conf_cap_type_v02 *remote_party_conf_capability =
    QMI_node.set_qmi_voice_all_call_status_ind_remote_party_conf_capability(1);
  remote_party_conf_capability[0].call_id = 1;
  remote_party_conf_capability[0].conf_is_enabled = FALSE;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_ACTIVE;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  calls[0]->has_peerConfAbility = TRUE;
  calls[0]->peerConfAbility.has_isVideoConfSupported = TRUE;
  calls[0]->peerConfAbility.isVideoConfSupported = FALSE;
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_HANGUP
  IMS_node.update_default_ims_request_hangup();
  IMS_node.set_ims_request_hangup_conn_index(1);
  t.inject(IMS_node);

  // Expect: QMI_VOICE_END_CALL_REQ_V02
  QMI_node.update_default_qmi_voice_end_call_req_msg(1);
  t.expect(QMI_node);

  // Inject: QMI_VOICE_END_CALL_RESP_V02
  QMI_node.update_default_qmi_voice_end_call_resp_msg(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  QMI_node.set_qmi_voice_end_call_resp_call_id(1);
  t.inject(QMI_node);

  // Inject: QMI_VOICE_ALL_CALL_STATUS_IND_V02
  call_info = QMI_node.update_default_qmi_voice_all_call_status_ind_msg(1);
  call_info[0].call_id = 1;
  call_info[0].call_state = CALL_STATE_END_V02;
  call_info[0].call_type = CALL_TYPE_VOICE_IP_V02;
  call_info[0].direction = CALL_DIRECTION_MO_V02;
  call_info[0].is_mpty = 0;
  call_info[0].mode = CALL_MODE_LTE_V02;
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

  // Expect: ims_MsgId_REQUEST_HANGUP (resp)
  IMS_node.update_default_ims_request_hangup_resp();
  t.expect(IMS_node);

  // Expect: ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
  calls = IMS_node.update_default_ims_unsol_response_call_state_changed_L(1);
  calls[0]->has_index = TRUE;
  calls[0]->index = 1;
  calls[0]->has_state = TRUE;
  calls[0]->state = ims_CallState_CALL_END;
  calls[0]->has_toa = TRUE;
  calls[0]->toa = 129;
  calls[0]->has_isMpty = TRUE;
  calls[0]->has_isMT = TRUE;
  calls[0]->has_als = TRUE;
  calls[0]->has_isVoice = TRUE;
  calls[0]->isVoice = 1;
  calls[0]->has_isVoicePrivacy = TRUE;
  calls[0]->isVoicePrivacy = 0;
  calls[0]->callDetails.has_callDomain = TRUE;
  calls[0]->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  calls[0]->callDetails.has_callType = TRUE;
  calls[0]->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  calls[0]->callDetails.has_mediaId = TRUE;
  calls[0]->callDetails.mediaId = 2;
  calls[0]->has_peerConfAbility = TRUE;
  calls[0]->peerConfAbility.has_isVideoConfSupported = TRUE;
  calls[0]->peerConfAbility.isVideoConfSupported = FALSE;
  calls[0]->has_failCause = TRUE;
  calls[0]->failCause.has_failcause = TRUE;
  calls[0]->failCause.failcause = ims_CallFailCause_CALL_FAIL_NORMAL;
  calls[0]->callDetails.has_callSubstate = TRUE;
  calls[0]->callDetails.callSubstate = ims_CallSubstate_CALL_SUBSTATE_NONE;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_voice_info_change_001
@description: onVoiceInfoChanged
==**/
test_result ril_utf_ims_voice_info_change_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_attribute("IMS");
  t.set_test_description("onVoiceInfoChanged");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: QMI_IMSA_VOICE_INFO_IND_V01
  QMI_node.update_default_qmi_imsa_voice_info_ind(IMSA_VOICE_INFO_SILENT_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_VOICE_INFO
  IMS_node.update_default_ims_unsol_voice_info(ims_Voice_Info_SILENT);
  t.expect(IMS_node);

  // Inject: QMI_IMSA_VOICE_INFO_IND_V01
  QMI_node.update_default_qmi_imsa_voice_info_ind(IMSA_VOICE_INFO_SPEECH_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_VOICE_INFO
  IMS_node.update_default_ims_unsol_voice_info(ims_Voice_Info_SPEECH);
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_multi_identity_lines_001
@description: registerMultiIdentityLines
==**/
test_result ril_utf_ims_multi_identity_lines_001(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_attribute("IMS");
  t.set_test_description("registerMultiIdentityLines");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: QMI_IMSA_DIGITS_INFO_PENDING_IND_V01
  QMI_node.update_default_qmi_imsa_digits_info_pending_ind();
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_MULTI_IDENTITY_PENDING_INFO
  IMS_node.update_default_ims_unsol_multi_identity_pending_info();
  t.expect(IMS_node);

  // Inject: ims_MsgId_REQUEST_REGISTER_MULTI_IDENTITY_LINES
  ims_MultiIdentityLineInfo *miInfo = IMS_node.update_default_ims_request_register_multi_identity_lines(2);
  strlcpy(miInfo[0].msisdn, "121212", strlen("121212")+1);
  miInfo[0].registrationStatus = ims_MultiIdentityRegistrationStatus_STATUS_DISABLE;
  miInfo[0].lineType = ims_MultiIdentityLineType_LINE_TYPE_PRIMARY;
  strlcpy(miInfo[1].msisdn, "909090", strlen("909090")+1);
  miInfo[1].registrationStatus = ims_MultiIdentityRegistrationStatus_STATUS_ENABLE;
  miInfo[1].lineType = ims_MultiIdentityLineType_LINE_TYPE_SECONDARY;
  t.inject(IMS_node);

  // Expect: QMI_IMS_SETTINGS_SET_DIGITS_REQ_V01
  QMI_node.update_default_qmi_ims_settings_set_digits_req(IMS_SETTINGS_DIGITS_FEATURE_ENABLED_V01);
  QMI_node.set_qmi_ims_settings_set_digits_req_no_of_lines(2);
  ims_settings_digits_line_info_v01 *line_info_list = QMI_node.set_qmi_ims_settings_set_digits_req_line_info_list(2);
  strlcpy(line_info_list[0].msisdn, "121212", strlen("121212")+1);
  //line_info_list[0].token
  line_info_list[0].uri_line_type = IMS_SETTINGS_URI_LINE_PRIMARY_V01;
  strlcpy(line_info_list[1].msisdn, "909090", strlen("909090")+1);
  line_info_list[1].uri_line_type = IMS_SETTINGS_URI_LINE_SECONDARY_V01;
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_SET_DIGITS_RESP_V01
  QMI_node.update_default_qmi_ims_settings_set_digits_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_REGISTER_MULTI_IDENTITY_LINES (resp)
  IMS_node.update_default_ims_request_register_multi_identity_lines_resp();
  t.expect(IMS_node);

  // Inject: QMI_IMSA_DIGITS_UPDATE_IND_V01
  QMI_node.update_default_qmi_imsa_digits_update_ind();
  QMI_node.set_qmi_imsa_digits_update_ind_no_of_lines(2);
  imsa_digits_line_info_v01 *line_info = QMI_node.set_qmi_imsa_digits_update_ind_line_info_list(2);
  strlcpy(line_info[0].msisdn, "121212", strlen("121212")+1);
  line_info[0].uri_line_status = IMSA_INACTIVE_V01;
  strlcpy(line_info[1].msisdn, "909090", strlen("909090")+1);
  line_info[1].uri_line_status = IMSA_ACTIVE_V01;
  t.inject(QMI_node);

  // Expect: ims_MsgId_UNSOL_MULTI_IDENTITY_REGISTRATION_STATUS
  ims_MultiIdentityLineInfo *multiIdLineInfo = IMS_node.update_default_ims_unsol_multi_identity_registration_status(2);
  strlcpy(multiIdLineInfo[0].msisdn, "121212", strlen("121212")+1);
  multiIdLineInfo[0].registrationStatus = ims_MultiIdentityRegistrationStatus_STATUS_DISABLE;
  //multiIdLineInfo[0].lineType = ims_MultiIdentityLineType_LINE_TYPE_PRIMARY;
  strlcpy(multiIdLineInfo[1].msisdn, "909090", strlen("909090")+1);
  multiIdLineInfo[1].registrationStatus = ims_MultiIdentityRegistrationStatus_STATUS_ENABLE;
  //multiIdLineInfo[1].lineType = ims_MultiIdentityLineType_LINE_TYPE_SECONDARY;
  t.expect(IMS_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_multi_identity_lines_002
@description: registerMultiIdentityLines
==**/
test_result ril_utf_ims_multi_identity_lines_002(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_attribute("IMS");
  t.set_test_description("registerMultiIdentityLines");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: ims_MsgId_REQUEST_REGISTER_MULTI_IDENTITY_LINES
  ims_MultiIdentityLineInfo *miInfo = IMS_node.update_default_ims_request_register_multi_identity_lines(2);
  strlcpy(miInfo[0].msisdn, "121212", strlen("121212")+1);
  miInfo[0].registrationStatus = ims_MultiIdentityRegistrationStatus_STATUS_DISABLE;
  miInfo[0].lineType = ims_MultiIdentityLineType_LINE_TYPE_PRIMARY;
  strlcpy(miInfo[1].msisdn, "909090", strlen("909090")+1);
  miInfo[1].registrationStatus = ims_MultiIdentityRegistrationStatus_STATUS_ENABLE;
  miInfo[1].lineType = ims_MultiIdentityLineType_LINE_TYPE_SECONDARY;
  t.inject(IMS_node);

  // Expect: QMI_IMS_SETTINGS_SET_DIGITS_REQ_V01
  QMI_node.update_default_qmi_ims_settings_set_digits_req(IMS_SETTINGS_DIGITS_FEATURE_ENABLED_V01);
  QMI_node.set_qmi_ims_settings_set_digits_req_no_of_lines(2);
  ims_settings_digits_line_info_v01 *line_info_list = QMI_node.set_qmi_ims_settings_set_digits_req_line_info_list(2);
  strlcpy(line_info_list[0].msisdn, "121212", strlen("121212")+1);
  //line_info_list[0].token
  line_info_list[0].uri_line_type = IMS_SETTINGS_URI_LINE_PRIMARY_V01;
  strlcpy(line_info_list[1].msisdn, "909090", strlen("909090")+1);
  line_info_list[1].uri_line_type = IMS_SETTINGS_URI_LINE_SECONDARY_V01;
  t.expect(QMI_node);

  // Inject: QMI_IMS_SETTINGS_SET_DIGITS_RESP_V01
  QMI_node.update_default_qmi_ims_settings_set_digits_resp(QMI_RESULT_FAILURE_V01, QMI_ERR_GENERAL_V01);
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_REGISTER_MULTI_IDENTITY_LINES (resp)
  IMS_node.update_default_ims_request_register_multi_identity_lines_resp();
  IMS_node.set_ims_error_value(ims_Error_E_GENERIC_FAILURE);
  t.expect(IMS_node);


  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_multi_identity_lines_003
@description: queryVirtualLineInfo
==**/
test_result ril_utf_ims_multi_identity_lines_003(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_attribute("IMS");
  t.set_test_description("queryVirtualLineInfo");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: ims_MsgId_REQUEST_QUERY_VIRTUAL_LINE_INFO
  IMS_node.update_default_ims_request_query_virtual_line_info("1234567890");
  t.inject(IMS_node);

  // Expect: QMI_IMSA_GET_VIRTUAL_LINE_REQ_V01
  QMI_node.update_default_qmi_imsa_get_virtual_line_req("1234567890");
  t.expect(QMI_node);

  // Inject: QMI_IMSA_GET_VIRTUAL_LINE_RSP_V01
  QMI_node.update_default_qmi_imsa_get_virtual_line_resp(QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01);
  p_associated_uri_v01 *uri_list = QMI_node.set_qmi_imsa_get_virtual_line_resp_virtual_line_info(2);
  strlcpy(uri_list[0].uri, "1111111", sizeof(uri_list[0].uri));
  strlcpy(uri_list[1].uri, "2222222", sizeof(uri_list[0].uri));
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_QUERY_VIRTUAL_LINE_INFO (resp)
  IMS_node.update_default_ims_request_query_virtual_line_info_resp("1234567890", 2);
  IMS_node.set_ims_request_query_virtual_line_info_resp_lines(0, "1111111");
  IMS_node.set_ims_request_query_virtual_line_info_resp_lines(1, "2222222");
  t.expect(IMS_node);


  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_ims_multi_identity_lines_004
@description: queryVirtualLineInfo failure case
==**/
test_result ril_utf_ims_multi_identity_lines_004(user_specified_attrib_t attr)
{
  Testcase t;
  QMIRILTestnode QMI_node;
  IMSTestnode IMS_node;

  t.set_test_id(__func__);
  t.set_test_attribute("IMS");
  t.set_test_description("queryVirtualLineInfo failure case");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: ims_MsgId_REQUEST_QUERY_VIRTUAL_LINE_INFO
  IMS_node.update_default_ims_request_query_virtual_line_info("1234567890");
  t.inject(IMS_node);

  // Expect: QMI_IMSA_GET_VIRTUAL_LINE_REQ_V01
  QMI_node.update_default_qmi_imsa_get_virtual_line_req("1234567890");
  t.expect(QMI_node);

  // Inject: QMI_IMSA_GET_VIRTUAL_LINE_RSP_V01
  QMI_node.update_default_qmi_imsa_get_virtual_line_resp(QMI_RESULT_FAILURE_V01, QMI_ERR_GENERAL_V01);
  //p_associated_uri_v01 *uri_list = QMI_node.set_qmi_imsa_get_virtual_line_resp_virtual_line_info(2);
  //strlcpy(uri_list[0].uri, "1111111", sizeof(uri_list[0].uri));
  //strlcpy(uri_list[1].uri, "2222222", sizeof(uri_list[0].uri));
  t.inject(QMI_node);

  // Expect: ims_MsgId_REQUEST_QUERY_VIRTUAL_LINE_INFO (resp)
  IMS_node.update_default_ims_request_query_virtual_line_info_resp("", 0);
  //IMS_node.set_ims_request_query_virtual_line_info_resp_lines(0, "1111111");
  //IMS_node.set_ims_request_query_virtual_line_info_resp_lines(1, "2222222");
  t.expect(IMS_node);


  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_facility_lock_testcase_006
@description:
==**/
test_result ril_utf_facility_lock_testcase_006(user_specified_attrib_t attr)
{
  Testcase t;
  AndroidTelTestnode RIL_node;
  QMIRILTestnode QMI_node;

  t.set_test_id(__func__);
  t.set_test_attribute("VOICE");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: RIL_REQUEST_SET_FACILITY_LOCK
  char **data = RIL_node.update_default_ril_request_set_facility_lock();
  strlcpy(data[0], "OI", strlen("OI")+1);
  strlcpy(data[1], "1", strlen("1")+1);
  strlcpy(data[2], "", strlen("")+1);
  strlcpy(data[3], "0", strlen("0")+1);
  t.inject(RIL_node);

  // Expect: QMI_VOICE_SET_SUPS_SERVICE_REQ_V02
  QMI_node.update_default_qmi_voice_set_sups_service_req_msg((voice_service_enum_v02)0x01,
      (voice_reason_enum_v02)0x08);
  //QMI_node.set_qmi_voice_set_sups_service_req_password("");
  t.expect(QMI_node);

  uint16_t description[] = {85, 110, 97, 98, 108, 101, 32, 116,
                          111, 32, 102, 105, 110, 100, 32, 112,
                          97, 114, 101, 110, 116, 32, 111, 102,
                          32, 110, 111, 100, 101, 0};

  // Inject: QMI_VOICE_SET_SUPS_SERVICE_RSEP_V02
  QMI_node.update_default_qmi_voice_set_sups_service_resp_msg(QMI_RESULT_FAILURE_V01,
      QMI_ERR_SUPS_FAILURE_CAUSE_V01);
  QMI_node.set_qmi_voice_set_sups_service_sip_error_code(409);
  QMI_node.set_qmi_voice_set_sups_service_failure_cause_description(30, description);
  t.inject(QMI_node);

  // Expect: RIL_REQUEST_SET_FACILITY_LOCK
  RIL_node.update_default_ril_request_set_facility_lock_resp();
  RIL_node.set_ril_error_value(RIL_E_MODEM_ERR);
  t.expect(RIL_node);

  t.execute();
  t.print_summary();

  return t.get_test_result();
}

/**==
@testname: ril_utf_facility_lock_testcase_007
@description:
==**/
test_result ril_utf_facility_lock_testcase_007(user_specified_attrib_t attr)
{
  Testcase t;
  AndroidTelTestnode RIL_node;
  QMIRILTestnode QMI_node;
  OEMTestnode OEM_node;

  t.set_test_id(__func__);
  t.set_test_attribute("VOICE");
  if (!t.isAttributeMatch(attr)) {
    test_result ret;
    return ret;
  }

  // Inject: RIL_REQUEST_SET_FACILITY_LOCK
  char **data = RIL_node.update_default_ril_request_set_facility_lock();
  strlcpy(data[0], "OI", strlen("OI")+1);
  strlcpy(data[1], "1", strlen("1")+1);
  strlcpy(data[2], "1212", strlen("1212")+1);
  strlcpy(data[3], "0", strlen("0")+1);
  t.inject(RIL_node);

  // Expect: QMI_VOICE_SET_SUPS_SERVICE_REQ_V02
  QMI_node.update_default_qmi_voice_set_sups_service_req_msg((voice_service_enum_v02)0x01,
      (voice_reason_enum_v02)0x08);
  QMI_node.set_qmi_voice_set_sups_service_req_password("1212");
  t.expect(QMI_node);

  uint16_t description[] = {85, 110, 97, 98, 108, 101, 32, 116,
                          111, 32, 102, 105, 110, 100, 32, 112,
                          97, 114, 101, 110, 116, 32, 111, 102,
                          32, 110, 111, 100, 101, 0};

  // Inject: QMI_VOICE_SET_SUPS_SERVICE_RSEP_V02
  QMI_node.update_default_qmi_voice_set_sups_service_resp_msg(QMI_RESULT_FAILURE_V01,
      QMI_ERR_SUPS_FAILURE_CAUSE_V01);
  QMI_node.set_qmi_voice_set_sups_service_resp_failure_cause(QMI_FAILURE_CAUSE_NEGATIVE_PWD_CHECK_V02);
  QMI_node.set_qmi_voice_set_sups_service_sip_error_code(409);
  QMI_node.set_qmi_voice_set_sups_service_failure_cause_description(30, description);
  t.inject(QMI_node);

  t.set_start();
  // Expect: QCRIL_REQ_HOOK_UNSOL_SS_ERROR_CODE
  OEM_node.update_default_qcril_evt_hook_unsol_ss_error_code();
  OEM_node.set_expect_skip_verification_status(false);
  t.expect(OEM_node);

  // Expect: RIL_REQUEST_SET_FACILITY_LOCK
  RIL_node.update_default_ril_request_set_facility_lock_resp();
  RIL_node.set_ril_error_value(RIL_E_PASSWORD_INCORRECT);
  t.expect(RIL_node);
  t.set_end();

  t.execute();
  t.print_summary();

  return t.get_test_result();
}
