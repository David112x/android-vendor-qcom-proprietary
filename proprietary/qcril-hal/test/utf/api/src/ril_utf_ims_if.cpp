/******************************************************************************
#  Copyright (c) 2013, 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_ims_if.cpp
  @brief   ims API's for test case dev

  DESCRIPTION

  API's for modifying ims messages used in RIL UTF tests.
  ---------------------------------------------------------------------------
******************************************************************************/
#include "ril_utf_ims_if.h"
using namespace std;
//---------------------------------------------------------------------------------------
//==================================================
// IMS Interfaces
//==================================================

/*========================================================
  ims_MsgId_REQUEST_IMS_REGISTRATION_STATE
========================================================*/
void ril_utf_ims_if::update_default_ims_request_registration_state(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_IMS_REGISTRATION_STATE;

}
ims_Registration *ril_utf_ims_if::update_default_ims_request_registration_state_resp(void)
{
  payload = (void *) malloc(sizeof(ims_Registration));
  payload_len = sizeof(ims_Registration);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_IMS_REGISTRATION_STATE;

  ims_Registration *ptr = (ims_Registration*) payload;
  return ptr;
}
void ril_utf_ims_if::set_ims_request_registration_state_resp_state(ims_Registration_RegState state)
{
  ims_Registration *ptr = (ims_Registration*) payload;
  ptr->has_state = TRUE;
  ptr->state = state;
}
void ril_utf_ims_if::set_ims_request_registration_state_resp_radioTech(ims_RadioTechType  val)
{
  ims_Registration *ptr = (ims_Registration*)payload;
  ptr->has_radioTech = TRUE;
  ptr->radioTech = val;
}
void ril_utf_ims_if::set_ims_request_registration_state_resp_pAssociatedUris(const char *val)
{
  ims_Registration *ptr = (ims_Registration*)payload;
  if(val) {
    size_t sz = strlen(val) + 1;
    ptr->pAssociatedUris.arg = qcril_malloc(sz);
    strlcpy((char*)ptr->pAssociatedUris.arg, val, sz);
  }
}
void ril_utf_ims_if::set_ims_request_registration_state_resp_errorcode(int has_errorcode, uint32_t errorcode, char *errormessage)
{
  ims_Registration *ptr = (ims_Registration*)payload;
  ptr->has_errorCode = has_errorcode;
  ptr->errorCode = errorcode;

  if(errormessage)
  {
    size_t sz = strlen(errormessage) + 1;
    ptr->errorMessage.arg = malloc(sz);
    strlcpy((char*)ptr->errorMessage.arg, errormessage, sz);
  }
}

/*========================================================
  ims_MsgId_REQUEST_DIAL
========================================================*/
ims_Dial *ril_utf_ims_if::update_default_ims_request_dial(char *address)
{
  payload = (void *) malloc(sizeof(ims_Dial));
  payload_len = sizeof(ims_Dial);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_DIAL;

  ims_Dial* ptr = (ims_Dial*) payload;
  ptr->address.arg = (char *) malloc(strlen(address)+1);
  strlcpy((char *)ptr->address.arg, address, strlen(address)+1);
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = ims_CallDomain_CALL_DOMAIN_PS;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = ims_CallType_CALL_TYPE_VOICE;
  return ptr;
}
ims_CallDetails *ril_utf_ims_if::set_ims_request_dial_call_details()
{
  ims_Dial* ptr = (ims_Dial*) payload;
  return &ptr->callDetails;
}
void ril_utf_ims_if::update_default_ims_request_dial_resp(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_DIAL;
}
void ril_utf_ims_if::set_ims_request_dial_isconferenceuri()
{
  ims_Dial* ptr = (ims_Dial*) payload;
  ptr->has_isConferenceUri = TRUE;
  ptr->isConferenceUri = TRUE;
}
void ril_utf_ims_if::set_ims_request_dial_isencrypted(bool isEncrypted)
{
  ims_Dial* ptr = (ims_Dial*) payload;
  ptr->has_isEncrypted = TRUE;
  ptr->isEncrypted = isEncrypted;
}

/*========================================================
  ims_MsgId_REQUEST_ADD_PARTICIPANT
========================================================*/
ims_Dial* ril_utf_ims_if::update_default_ims_request_add_participant(char *address)
{
  payload = (void *) malloc(sizeof(ims_Dial));
  payload_len = sizeof(ims_Dial);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_ADD_PARTICIPANT;

  ims_Dial* ptr = (ims_Dial*) payload;
  ptr->address.arg = (char *) malloc(QMI_VOICE_NUMBER_MAX_V02);
  strlcpy((char *)ptr->address.arg, address, QMI_VOICE_NUMBER_MAX_V02);
  return ptr;
}
ims_CallDetails *ril_utf_ims_if::set_ims_request_add_participant_call_details()
{
  ims_Dial* ptr = (ims_Dial*) payload;
  return &ptr->callDetails;
}
void ril_utf_ims_if::update_default_ims_request_add_participant_resp(void)
{
  payload = (void *) malloc(sizeof(_ims_SipErrorInfo));;
  payload_len = sizeof(_ims_SipErrorInfo);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_ADD_PARTICIPANT;
}

/*========================================================
  ims_MsgId_REQUEST_ANSWER
========================================================*/
void ril_utf_ims_if::update_default_ims_request_answer(void)
{
  payload = (void *) malloc(sizeof(ims_Answer));
  payload_len = sizeof(ims_Answer);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_ANSWER;

  ims_Answer *ptr = (ims_Answer*) payload;
}
void ril_utf_ims_if::set_ims_request_answer_call_type(ims_CallType call_type)
{
  ims_Answer *ptr = (ims_Answer*) payload;
  ptr->has_call_type = TRUE;
  ptr->call_type = call_type;
}
void ril_utf_ims_if::set_ims_request_answer_presentation(ims_IpPresentation pres)
{
  ims_Answer *ptr = (ims_Answer*) payload;
  ptr->has_presentation = TRUE;
  ptr->presentation = pres;
}
void ril_utf_ims_if::update_default_ims_request_answer_resp(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_ANSWER;
}

/*========================================================
  ims_MsgId_REQUEST_HANGUP
========================================================*/
ims_Hangup *ril_utf_ims_if::update_default_ims_request_hangup(void)
{
  payload = (void *) malloc(sizeof(ims_Hangup));
  payload_len = sizeof(ims_Hangup);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_HANGUP;

  ims_Hangup*ptr = (ims_Hangup*) payload;

  return ptr;
}
void ril_utf_ims_if::set_ims_request_hangup_conn_index(uint32_t conn_index)
{
  ims_Hangup*ptr = (ims_Hangup*) payload;
  ptr->has_conn_index = TRUE;
  ptr->conn_index = conn_index;
}
void ril_utf_ims_if::set_ims_request_hangup_conn_uri(char *uri)
{
  ims_Hangup*ptr = (ims_Hangup*) payload;
  ptr->conn_uri.arg = (char *) malloc (QMI_VOICE_CONF_URI_MAX_LEN_V02);
  strlcpy((char *)ptr->conn_uri.arg, uri, QMI_VOICE_CONF_URI_MAX_LEN_V02);
}
void ril_utf_ims_if::set_ims_request_hangup_multi_party(bool multi_party)
{
  ims_Hangup*ptr = (ims_Hangup*) payload;
  ptr->has_multi_party = TRUE;
  ptr->multi_party = multi_party;
}
void ril_utf_ims_if::set_ims_request_hangup_conf_id(uint32_t conf_id)
{
  ims_Hangup*ptr = (ims_Hangup*) payload;
  ptr->has_conf_id = TRUE;
  ptr->conf_id = conf_id;
}
void ril_utf_ims_if::set_ims_request_hangup_callfailcauseresponse_errorinfo(char *val)
{
  ims_Hangup *ptr = (ims_Hangup*) payload;
  qcril_binary_data_type *bin_data = (qcril_binary_data_type *)qcril_malloc(sizeof(qcril_binary_data_type));
  bin_data->len = strlen(val);
  bin_data->data = (uint8_t *)strdup(val);
  ptr->has_failCauseResponse = TRUE;
  ptr->failCauseResponse.has_failcause = TRUE;
  ptr->failCauseResponse.failcause = ims_CallFailCause_CALL_FAIL_MISC;
  ptr->failCauseResponse.errorinfo.arg = bin_data;
}
void ril_utf_ims_if::set_ims_request_hangup_callfailcauseresponse_failcause(ims_CallFailCause val)
{
  ims_Hangup *ptr = (ims_Hangup*) payload;
  ptr->has_failCauseResponse = TRUE;
  ptr->failCauseResponse.has_failcause = TRUE;
  ptr->failCauseResponse.failcause = val;
}
#ifdef RIL_UTF_L_MASTER
ims_CallFailCauseResponse* ril_utf_ims_if::set_ims_callfailcauseresponse (ims_CallFailCause failcause, uint8_t val)
{
  uint8_t *data;
  data = (uint8_t *)malloc (sizeof (*data));
  *data = val;
  ims_CallFailCauseResponse *imsfailcause;
  imsfailcause = (ims_CallFailCauseResponse *)malloc (sizeof (*imsfailcause));
  imsfailcause->has_failcause = TRUE;
  imsfailcause->failcause = failcause;
  imsfailcause->errorinfo.arg = data;
  return imsfailcause;
}
ims_CallFailCauseResponse* ril_utf_ims_if::set_ims_callfailcauseresponse_no_cause ()
{
  ims_CallFailCauseResponse *imsfailcause;
  imsfailcause = (ims_CallFailCauseResponse *)malloc (sizeof (*imsfailcause));
  imsfailcause->has_failcause = FALSE;
  return imsfailcause;
}
#endif
void ril_utf_ims_if::update_default_ims_request_hangup_resp(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_HANGUP;
}

/*========================================================
  ims_MsgId_REQUEST_GET_CURRENT_CALLS
========================================================*/
void ril_utf_ims_if::update_default_ims_request_get_current_calls(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_GET_CURRENT_CALLS ;
}
ims_CallList_Call** ril_utf_ims_if::update_default_ims_request_get_current_calls_resp(int num_calls)
{
  int i;

  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_GET_CURRENT_CALLS;

  if (num_calls == 0)
  {
    payload = NULL;
    payload_len = 0;

    return NULL;
  }
  else
  {
    payload = (void *) malloc(sizeof(ims_CallList));
    payload_len = sizeof(ims_CallList);
    memset(payload, 0x00, payload_len);

    ims_CallList *ptr = (ims_CallList *) payload;
    ptr->callAttributes.arg = (ims_CallList_Call**) malloc(sizeof(ims_CallList_Call*) * (num_calls+1));

    for (i = 0; i < num_calls; ++i)
    {
      ((ims_CallList_Call**)ptr->callAttributes.arg)[i] = (ims_CallList_Call*) malloc(sizeof(ims_CallList_Call));
    }

    return ((ims_CallList_Call**)ptr->callAttributes.arg);
  }

}
#ifdef RIL_UTF_L_MASTER
ims_CallList_Call** ril_utf_ims_if::update_default_ims_unsol_response_call_state_changed_L(int num_calls)
{
  int i;

  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED;

  if (num_calls == 0)
  {
    payload = NULL;
    payload_len = 0;

    return NULL;
  }
  else
  {
    payload = (void *) malloc(sizeof(ims_CallList));
    payload_len = sizeof(ims_CallList);
    memset(payload, 0x00, payload_len);

    ims_CallList *ptr = (ims_CallList *) payload;
    ptr->callAttributes.arg = (ims_CallList_Call**) malloc(sizeof(ims_CallList_Call*) * (num_calls+1));
    ims_CallList_Call** calls = (ims_CallList_Call**)ptr->callAttributes.arg;

    for (i = 0; i < num_calls; ++i)
    {
      calls[i] = (ims_CallList_Call*) malloc(sizeof(ims_CallList_Call));
      memset(calls[i], 0x00, sizeof(ims_CallList_Call));

      calls[i]->has_numberPresentation = TRUE;
      calls[i]->numberPresentation     = 0;
      calls[i]->has_namePresentation = TRUE;
      calls[i]->namePresentation     = 2;

      calls[i]->has_callDetails = TRUE;

      // Initialize mediaid to invalid (-1)
      calls[i]->callDetails.has_mediaId = TRUE;
      calls[i]->callDetails.mediaId = -1;
      // Initialize callsubstate to invalid (0 - UNDEFINED/NONE)
      calls[i]->callDetails.has_callSubstate = TRUE;
      calls[i]->callDetails.callSubstate = (ims_CallSubstate)0;
      // Initialize isEncrypted
      calls[i]->has_isEncrypted = TRUE;
      calls[i]->isEncrypted     = FALSE;
    }

    return ((ims_CallList_Call**)ptr->callAttributes.arg);
  }

}
#endif
/*========================================================
  ims_MsgId_REQUEST_HANGUP_WAITING_OR_BACKGROUND
========================================================*/
void ril_utf_ims_if::update_default_ims_request_hangup_waiting_or_background(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_HANGUP_WAITING_OR_BACKGROUND;
}
void ril_utf_ims_if::update_default_ims_request_hangup_waiting_or_background_resp(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_HANGUP_WAITING_OR_BACKGROUND;
}

/*========================================================
  ims_MsgId_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND
========================================================*/
void ril_utf_ims_if::update_default_ims_request_hangup_foreground_resume_background(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND;
}
void ril_utf_ims_if::update_default_ims_request_hangup_foreground_resume_background_resp(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND;
}

/*========================================================
  ims_MsgId_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE
========================================================*/
void ril_utf_ims_if::update_default_ims_request_switch_waiting_or_holding_and_active(void)
{
  payload = malloc(sizeof(ims_SwitchWaitingOrHoldingAndActive));
  payload_len = sizeof(ims_SwitchWaitingOrHoldingAndActive);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE;

  ims_SwitchWaitingOrHoldingAndActive *ptr = (ims_SwitchWaitingOrHoldingAndActive*) payload;
}
void ril_utf_ims_if::set_ims_request_switch_waiting_or_holding_and_active_call_type(ims_CallType call_type)
{
  ims_SwitchWaitingOrHoldingAndActive *ptr = (ims_SwitchWaitingOrHoldingAndActive*) payload;
  ptr->has_call_type = TRUE;
  ptr->call_type = call_type;
}
void ril_utf_ims_if::update_default_ims_request_switch_waiting_or_holding_and_active_resp(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE;
}

/*========================================================
  ims_MsgId_REQUEST_CONFERENCE
========================================================*/
void ril_utf_ims_if::update_default_ims_request_conference(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_CONFERENCE;
}
ims_SipErrorInfo *ril_utf_ims_if::update_default_ims_request_conference_resp(void)
{
  payload = (void *) malloc(sizeof(_ims_SipErrorInfo));
  payload_len = sizeof(_ims_SipErrorInfo);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_CONFERENCE;

  ims_SipErrorInfo *ptr = (ims_SipErrorInfo *) payload;
  return  ptr;
}
void ril_utf_ims_if::update_sip_error_info(uint32_t code, char *message)
{
  _ims_SipErrorInfo *info = (_ims_SipErrorInfo *)payload;
  info->has_sipErrorCode = TRUE;
  info->sipErrorCode = code;
  info->sipErrorString.arg = message;
}
/*========================================================
  ims_MsgId_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE
========================================================*/
void ril_utf_ims_if::update_default_ims_request_exit_emergency_callback_mode(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE;
}
void ril_utf_ims_if::update_default_ims_request_exit_emergency_callback_mode_resp(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE;
}

/*========================================================
  ims_MsgId_REQUEST_EXPLICIT_CALL_TRANSFER
========================================================*/
ims_ExplicitCallTransfer *ril_utf_ims_if::update_default_ims_request_explicit_call_transfer(void)
{
  payload = (void *) malloc(sizeof(ims_ExplicitCallTransfer));
  payload_len = sizeof(ims_ExplicitCallTransfer);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_EXPLICIT_CALL_TRANSFER;
  ims_ExplicitCallTransfer *ptr = (ims_ExplicitCallTransfer*) payload;
  return ptr;
}
void ril_utf_ims_if::update_default_ims_request_explicit_call_transfer_resp(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_EXPLICIT_CALL_TRANSFER;
}

/*========================================================
  ims_MsgId_REQUEST_DEFLECT_CALL
========================================================*/
ims_DeflectCall *ril_utf_ims_if::update_default_ims_request_deflect_call(void)
{
  payload = (void *) malloc(sizeof(ims_DeflectCall));
  payload_len = sizeof(ims_DeflectCall);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_DEFLECT_CALL;
  ims_DeflectCall *ptr = (ims_DeflectCall*) payload;
  return ptr;
}
void ril_utf_ims_if::update_default_ims_request_deflect_call_resp(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_DEFLECT_CALL;
}

/*========================================================
  ims_MsgId_REQUEST_DTMF
========================================================*/
void ril_utf_ims_if::update_default_ims_request_dtmf(char *dtmf)
{
  payload = (void*) malloc(sizeof(ims_Dtmf));
  payload_len = sizeof(ims_Dtmf);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_DTMF;

  ims_Dtmf *ptr = (ims_Dtmf*) payload;
  ptr->dtmf.arg = (char*) malloc(strlen(dtmf)+1);
  strlcpy((char *)ptr->dtmf.arg, dtmf, strlen(dtmf)+1);
}
void ril_utf_ims_if::update_default_ims_request_dtmf_resp(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_DTMF;
}

/*========================================================
  ims_MsgId_REQUEST_DTMF_START
========================================================*/
void ril_utf_ims_if::update_default_ims_request_dtmf_start(char *dtmf)
{
  payload = (void*) malloc(sizeof(ims_Dtmf));
  payload_len = sizeof(ims_Dtmf);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_DTMF_START;

  ims_Dtmf *ptr = (ims_Dtmf*) payload;
  ptr->dtmf.arg = (char*) malloc(strlen(dtmf)+1);
  strlcpy((char *)ptr->dtmf.arg, dtmf, strlen(dtmf)+1);
}
void ril_utf_ims_if::update_default_ims_request_dtmf_start_resp(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_DTMF_START;
}

/*========================================================
  ims_MsgId_REQUEST_DTMF_STOP
========================================================*/
void ril_utf_ims_if::update_default_ims_request_dtmf_stop(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_DTMF_STOP;
}
void ril_utf_ims_if::update_default_ims_request_dtmf_stop_resp(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_DTMF_STOP;
}

/*========================================================
  ims_MsgId_REQUEST_MODIFY_CALL_INITIATE
========================================================*/
void ril_utf_ims_if::update_default_ims_request_modify_call_initiate(void)
{
  payload = (void*) malloc(sizeof(ims_CallModify));
  payload_len = sizeof(ims_CallModify);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_MODIFY_CALL_INITIATE;

  ims_CallModify *ptr = (ims_CallModify*) payload;
}
void ril_utf_ims_if::set_ims_request_modify_call_initiate_call_index(uint32_t callindex)
{
  ims_CallModify *ptr = (ims_CallModify*) payload;
  ptr->has_callIndex = TRUE;
  ptr->callIndex = callindex;
}
void ril_utf_ims_if::set_ims_request_modify_call_initiate_error(ims_Error error)
{
  ims_CallModify *ptr = (ims_CallModify*) payload;
  ptr->has_error = TRUE;
  ptr->error = error;
}
void ril_utf_ims_if::set_ims_request_modify_call_initiate_call_details(ims_CallType calltype, ims_CallDomain calldomain)
{
  ims_CallModify *ptr = (ims_CallModify*) payload;

  ptr->has_callDetails = TRUE;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = calldomain;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = calltype;
}
void ril_utf_ims_if::set_ims_request_modify_call_initiate_rtt_mode(ims_RttMode rttMode)
{
  ims_CallModify *ptr = (ims_CallModify*) payload;

  ptr->has_callDetails = TRUE;
  ptr->callDetails.has_rttMode = TRUE;
  ptr->callDetails.rttMode = rttMode;
}
void ril_utf_ims_if::update_default_ims_request_modify_call_initiate_resp(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_MODIFY_CALL_INITIATE;
}

/*========================================================
  ims_MsgId_REQUEST_MODIFY_CALL_CONFIRM
========================================================*/
void ril_utf_ims_if::update_default_ims_request_modify_call_confirm(void)
{
  payload = (void*) malloc(sizeof(ims_CallModify));
  payload_len = sizeof(ims_CallModify);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_MODIFY_CALL_CONFIRM;

  ims_CallModify *ptr = (ims_CallModify*) payload;
}
void ril_utf_ims_if::set_ims_request_modify_call_confirm_call_index(uint32_t callindex)
{
  ims_CallModify *ptr = (ims_CallModify*) payload;
  ptr->has_callIndex = TRUE;
  ptr->callIndex = callindex;
}
void ril_utf_ims_if::set_ims_request_modify_call_confirm_error(ims_Error error)
{
  ims_CallModify *ptr = (ims_CallModify*) payload;
  ptr->has_error = TRUE;
  ptr->error = error;
}
void ril_utf_ims_if::set_ims_request_modify_call_confirm_call_details(ims_CallType calltype, ims_CallDomain calldomain)
{
  ims_CallModify *ptr = (ims_CallModify*) payload;

  ptr->has_callDetails = TRUE;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = calldomain;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = calltype;
}
void ril_utf_ims_if::update_default_ims_request_modify_call_confirm_resp(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_MODIFY_CALL_CONFIRM;
}

/*========================================================
  ims_MsgId_REQUEST_CANCEL_MODIFY_CALL
========================================================*/
void ril_utf_ims_if::update_default_ims_request_cancel_modify_call(void)
{
  payload = (void*) malloc(sizeof(ims_CancelModifyCall));
  payload_len = sizeof(ims_CancelModifyCall);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_CANCEL_MODIFY_CALL;

  ims_CancelModifyCall *ptr = (ims_CancelModifyCall*) payload;
}
void ril_utf_ims_if::set_ims_request_cancel_modify_call_call_id(uint32_t callId)
{
  ims_CancelModifyCall *ptr = (ims_CancelModifyCall*) payload;
  ptr->has_callId = TRUE;
  ptr->callId = callId;
}
void ril_utf_ims_if::update_default_ims_request_cancel_modify_call_resp(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_CANCEL_MODIFY_CALL;
}

/*========================================================
  ims_MsgId_REQUEST_QUERY_CLIP
========================================================*/
void ril_utf_ims_if::update_default_ims_request_query_clip(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_QUERY_CLIP;

}
void ril_utf_ims_if::update_default_ims_request_query_clip_resp(ims_ClipStatus clip_status)
{
  payload = (void *) malloc(sizeof(ims_ClipProvisionStatus));
  payload_len = sizeof(ims_ClipProvisionStatus);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_QUERY_CLIP;

  ims_ClipProvisionStatus *ptr = (ims_ClipProvisionStatus*) payload;
  ptr->has_clip_status = TRUE;
  ptr->clip_status=clip_status;
}

ims_ClipProvisionStatus *ril_utf_ims_if::update_default_ims_request_query_clip_resp_no_status(void)
{
  payload = (void *) malloc(sizeof(ims_ClipProvisionStatus));
  payload_len = sizeof(ims_ClipProvisionStatus);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_QUERY_CLIP;

  ims_ClipProvisionStatus *ptr = (ims_ClipProvisionStatus*) payload;
  return ptr;
}

/*========================================================
  ims_MsgId_REQUEST_GET_CLIR
========================================================*/
void ril_utf_ims_if::update_default_ims_request_get_clir(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_GET_CLIR;
}
void ril_utf_ims_if::update_default_ims_request_get_clir_resp(void)
{
  payload = (ims_Clir*) malloc(sizeof(ims_Clir));
  payload_len = sizeof(ims_Clir);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_GET_CLIR;

  ims_Clir *ptr = (ims_Clir*) payload;
}
void ril_utf_ims_if::set_ims_request_get_clir_resp_param_m(uint32_t param_m)
{
  ims_Clir *ptr = (ims_Clir*) payload;
  ptr->has_param_m = TRUE;
  ptr->param_m = param_m;
}
void ril_utf_ims_if::set_ims_request_get_clir_resp_param_n(uint32_t param_n)
{
  ims_Clir *ptr = (ims_Clir*) payload;
  ptr->has_param_n = TRUE;
  ptr->param_n = param_n;
}

/*========================================================
  ims_MsgId_REQUEST_SET_CLIR
========================================================*/
void ril_utf_ims_if::update_default_ims_request_set_clir(void)
{
  payload = (ims_Clir*) malloc(sizeof(ims_Clir));
  payload_len = sizeof(ims_Clir);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_SET_CLIR;

  ims_Clir *ptr = (ims_Clir*) payload;
}
void ril_utf_ims_if::set_ims_request_set_clir_param_m(uint32_t param_m)
{
  ims_Clir *ptr = (ims_Clir*) payload;
  ptr->has_param_m = TRUE;
  ptr->param_m = param_m;
}
void ril_utf_ims_if::set_ims_request_set_clir_param_n(uint32_t param_n)
{
  ims_Clir *ptr = (ims_Clir*) payload;
  ptr->has_param_n = TRUE;
  ptr->param_n = param_n;
}
void ril_utf_ims_if::update_default_ims_request_set_clir_resp(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_SET_CLIR;
}

/*========================================================
  ims_MsgId_REQUEST_QUERY_CALL_FORWARD_STATUS
========================================================*/
ims_CallForwardInfoList_CallForwardInfo**
ril_utf_ims_if::update_default_ims_request_query_call_forward_status(int num_entries)
{
  int i;
  payload = (ims_CallForwardInfoList*) malloc(sizeof(ims_CallForwardInfoList));
  payload_len = sizeof(ims_CallForwardInfoList);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_QUERY_CALL_FORWARD_STATUS;

  ims_CallForwardInfoList *ptr = (ims_CallForwardInfoList*) payload;

  ptr->info.arg = (ims_CallForwardInfoList_CallForwardInfo **)
      malloc(sizeof(ims_CallForwardInfoList_CallForwardInfo*) * (num_entries + 1));
  memset(ptr->info.arg, 0x00,
          sizeof(ims_CallForwardInfoList_CallForwardInfo*) * (num_entries + 1));

  for (i = 0; i < num_entries; ++i)
  {
    ((ims_CallForwardInfoList_CallForwardInfo **) ptr->info.arg)[i] =
        (ims_CallForwardInfoList_CallForwardInfo*)
        malloc(sizeof(ims_CallForwardInfoList_CallForwardInfo));

    memset(((ims_CallForwardInfoList_CallForwardInfo **)ptr->info.arg)[i],
            0x00,
            sizeof(ims_CallForwardInfoList_CallForwardInfo));
  }

  return ((ims_CallForwardInfoList_CallForwardInfo **)ptr->info.arg);
}

ims_CallForwardInfoList_CallForwardInfo**
ril_utf_ims_if::update_default_ims_request_query_call_forward_status_resp(int num_entries)
{
  int i;
  payload = (ims_CallForwardInfoList*) malloc(sizeof(ims_CallForwardInfoList));
  payload_len = sizeof(ims_CallForwardInfoList);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_QUERY_CALL_FORWARD_STATUS;

  ims_CallForwardInfoList *ptr = (ims_CallForwardInfoList*) payload;

  ptr->info.arg = (ims_CallForwardInfoList_CallForwardInfo **)
    malloc(sizeof(ims_CallForwardInfoList_CallForwardInfo*) * (num_entries+1));
  memset(ptr->info.arg, 0x00,
          sizeof(ims_CallForwardInfoList_CallForwardInfo*) * (num_entries + 1));

  for (i = 0; i < num_entries; ++i)
  {
    ((ims_CallForwardInfoList_CallForwardInfo **)ptr->info.arg)[i] =
        (ims_CallForwardInfoList_CallForwardInfo*)
        malloc(sizeof(ims_CallForwardInfoList_CallForwardInfo));

    memset(((ims_CallForwardInfoList_CallForwardInfo **)ptr->info.arg)[i],
            0x00,
            sizeof(ims_CallForwardInfoList_CallForwardInfo));
  }

  return ((ims_CallForwardInfoList_CallForwardInfo **)ptr->info.arg);
}

ims_SipErrorInfo *ril_utf_ims_if::set_ims_request_query_call_forward_status_resp_error_details()
{
  ims_CallForwardInfoList *ptr = (ims_CallForwardInfoList*) payload;
  ptr->has_errorDetails = TRUE;
  return &(ptr->errorDetails);
}

void ril_utf_ims_if::set_ims_request_query_call_forward_status_callfwdtimerstart(
        uint32_t year, uint32_t month, uint32_t day, uint32_t hour, uint32_t minute,
        uint32_t second, uint32_t timezone, uint32_t index)
{
  ims_CallForwardInfoList *ptr = (ims_CallForwardInfoList*) payload;
  ims_CallForwardInfoList_CallForwardInfo *cfi = ((ims_CallForwardInfoList_CallForwardInfo **)
                                                 ptr->info.arg)[index];

  cfi->has_callFwdTimerStart = TRUE;
  cfi->callFwdTimerStart.has_year = TRUE;
  cfi->callFwdTimerStart.year = year;
  cfi->callFwdTimerStart.has_month = TRUE;
  cfi->callFwdTimerStart.month = month;
  cfi->callFwdTimerStart.has_day = TRUE;
  cfi->callFwdTimerStart.day = day;
  cfi->callFwdTimerStart.has_hour = TRUE;
  cfi->callFwdTimerStart.hour = hour;
  cfi->callFwdTimerStart.has_minute = TRUE;
  cfi->callFwdTimerStart.minute = minute;
  cfi->callFwdTimerStart.has_second = TRUE;
  cfi->callFwdTimerStart.second = second;
  cfi->callFwdTimerStart.has_timezone = TRUE;
  cfi->callFwdTimerStart.timezone = timezone;
}

void ril_utf_ims_if::set_ims_request_query_call_forward_status_callfwdtimerend(
        uint32_t year, uint32_t month, uint32_t day, uint32_t hour, uint32_t minute,
        uint32_t second, uint32_t timezone, uint32_t index)
{
  ims_CallForwardInfoList *ptr = (ims_CallForwardInfoList*) payload;
  ims_CallForwardInfoList_CallForwardInfo *cfi = ((ims_CallForwardInfoList_CallForwardInfo **)
                                                 ptr->info.arg)[index];
  cfi->has_callFwdTimerEnd = TRUE;
  cfi->callFwdTimerEnd.has_year = TRUE;
  cfi->callFwdTimerEnd.year = year;
  cfi->callFwdTimerEnd.has_month = TRUE;
  cfi->callFwdTimerEnd.month = month;
  cfi->callFwdTimerEnd.has_day = TRUE;
  cfi->callFwdTimerEnd.day = day;
  cfi->callFwdTimerEnd.has_hour = TRUE;
  cfi->callFwdTimerEnd.hour = hour;
  cfi->callFwdTimerEnd.has_minute = TRUE;
  cfi->callFwdTimerEnd.minute = minute;
  cfi->callFwdTimerEnd.has_second = TRUE;
  cfi->callFwdTimerEnd.second = second;
  cfi->callFwdTimerEnd.has_timezone = TRUE;
  cfi->callFwdTimerEnd.timezone = timezone;
}

/*========================================================
  ims_MsgId_REQUEST_SET_CALL_FORWARD_STATUS
========================================================*/
ims_CallForwardInfoList_CallForwardInfo**
ril_utf_ims_if::update_default_ims_request_set_call_forward_status(int num_entries)
{
  int i;
  payload = (ims_CallForwardInfoList*) malloc(sizeof(ims_CallForwardInfoList));
  payload_len = sizeof(ims_CallForwardInfoList);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_SET_CALL_FORWARD_STATUS;

  ims_CallForwardInfoList *ptr = (ims_CallForwardInfoList*) payload;

  ptr->info.arg = (ims_CallForwardInfoList_CallForwardInfo **)
    malloc(sizeof(ims_CallForwardInfoList_CallForwardInfo*) * (num_entries + 1));
  memset(ptr->info.arg, 0x00,
          sizeof(ims_CallForwardInfoList_CallForwardInfo*) * (num_entries + 1));

  for (i = 0; i < num_entries; ++i)
  {
    ((ims_CallForwardInfoList_CallForwardInfo **)ptr->info.arg)[i] =
        (ims_CallForwardInfoList_CallForwardInfo*)
        malloc(sizeof(ims_CallForwardInfoList_CallForwardInfo));

    memset(((ims_CallForwardInfoList_CallForwardInfo **)ptr->info.arg)[i],
            0x00,
            sizeof(ims_CallForwardInfoList_CallForwardInfo));
  }

  return ((ims_CallForwardInfoList_CallForwardInfo **)ptr->info.arg);
}

void ril_utf_ims_if::set_ims_request_set_call_forward_status_callfwdtimerstart(
        uint32_t year, uint32_t month, uint32_t day, uint32_t hour, uint32_t minute,
        uint32_t second, uint32_t timezone)
{
  ims_CallForwardInfoList *ptr = (ims_CallForwardInfoList*) payload;
  ims_CallForwardInfoList_CallForwardInfo *cfi = ((ims_CallForwardInfoList_CallForwardInfo **)
                                                 ptr->info.arg)[0];

  cfi->has_callFwdTimerStart = TRUE;
  cfi->callFwdTimerStart.has_year = TRUE;
  cfi->callFwdTimerStart.year = year;
  cfi->callFwdTimerStart.has_month = TRUE;
  cfi->callFwdTimerStart.month = month;
  cfi->callFwdTimerStart.has_day = TRUE;
  cfi->callFwdTimerStart.day = day;
  cfi->callFwdTimerStart.has_hour = TRUE;
  cfi->callFwdTimerStart.hour = hour;
  cfi->callFwdTimerStart.has_minute = TRUE;
  cfi->callFwdTimerStart.minute = minute;
  cfi->callFwdTimerStart.has_second = TRUE;
  cfi->callFwdTimerStart.second = second;
  cfi->callFwdTimerStart.has_timezone = TRUE;
  cfi->callFwdTimerStart.timezone = timezone;
}
void ril_utf_ims_if::set_ims_request_set_call_forward_status_callfwdtimerend(
        uint32_t year, uint32_t month, uint32_t day, uint32_t hour, uint32_t minute,
        uint32_t second, uint32_t timezone)
{
  ims_CallForwardInfoList *ptr = (ims_CallForwardInfoList*) payload;
  ims_CallForwardInfoList_CallForwardInfo *cfi = ((ims_CallForwardInfoList_CallForwardInfo **)
                                                 ptr->info.arg)[0];
  cfi->has_callFwdTimerEnd = TRUE;
  cfi->callFwdTimerEnd.has_year = TRUE;
  cfi->callFwdTimerEnd.year = year;
  cfi->callFwdTimerEnd.has_month = TRUE;
  cfi->callFwdTimerEnd.month = month;
  cfi->callFwdTimerEnd.has_day = TRUE;
  cfi->callFwdTimerEnd.day = day;
  cfi->callFwdTimerEnd.has_hour = TRUE;
  cfi->callFwdTimerEnd.hour = hour;
  cfi->callFwdTimerEnd.has_minute = TRUE;
  cfi->callFwdTimerEnd.minute = minute;
  cfi->callFwdTimerEnd.has_second = TRUE;
  cfi->callFwdTimerEnd.second = second;
  cfi->callFwdTimerEnd.has_timezone = TRUE;
  cfi->callFwdTimerEnd.timezone = timezone;
}

ims_CallForwardStatusInfo *ril_utf_ims_if::update_default_ims_request_set_call_forward_status_resp(void)
{
  payload = (ims_CallForwardStatusInfo*) malloc(sizeof(ims_CallForwardStatusInfo));
  payload_len = sizeof(ims_CallForwardStatusInfo);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_SET_CALL_FORWARD_STATUS;

  ims_CallForwardStatusInfo *ptr = (ims_CallForwardStatusInfo*) payload;
  return ptr;
}

/*========================================================
  ims_MsgId_REQUEST_QUERY_CALL_WAITING
========================================================*/
void ril_utf_ims_if::update_default_ims_request_query_call_waiting(void)
{
  payload = (ims_ServiceClass*) malloc(sizeof(ims_ServiceClass));
  payload_len = sizeof(ims_ServiceClass);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_QUERY_CALL_WAITING;

  ims_ServiceClass *ptr = (ims_ServiceClass*) payload;
}
void ril_utf_ims_if::set_ims_request_query_call_waiting_service_class(uint32_t service_class)
{
  ims_ServiceClass *ptr = (ims_ServiceClass*) payload;
  ptr->has_service_Class = TRUE;
  ptr->service_Class = service_class;
}
void ril_utf_ims_if::update_default_ims_request_query_call_waiting_resp(void)
{
  payload = (ims_CallWaitingInfo*) malloc(sizeof(ims_CallWaitingInfo));
  payload_len = sizeof(ims_CallWaitingInfo);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_QUERY_CALL_WAITING;

  ims_CallWaitingInfo *ptr = (ims_CallWaitingInfo*) payload;
}
void ril_utf_ims_if::set_ims_request_query_call_waiting_resp_service_status(ims_ServiceClassStatus service_status)
{
  ims_CallWaitingInfo *ptr = (ims_CallWaitingInfo*) payload;
  ptr->has_service_status = TRUE;
  ptr->service_status = service_status;
}
void ril_utf_ims_if::set_ims_request_query_call_waiting_resp_service_class(uint32_t service_class)
{
  ims_CallWaitingInfo *ptr = (ims_CallWaitingInfo*) payload;
  ptr->has_service_class = TRUE;
  ptr->service_class.has_service_Class = TRUE;
  ptr->service_class.service_Class = service_class;
}
ims_SipErrorInfo *ril_utf_ims_if::set_ims_request_query_call_waiting_resp_error_details()
{
  ims_CallWaitingInfo *ptr = (ims_CallWaitingInfo*) payload;
  ptr->has_errorDetails = TRUE;
  return &(ptr->errorDetails);
}

/*========================================================
  ims_MsgId_REQUEST_SET_CALL_WAITING
========================================================*/
ims_CallWaitingInfo *ril_utf_ims_if::update_default_ims_request_set_call_waiting(void)
{
  payload = (ims_CallWaitingInfo*) malloc(sizeof(ims_CallWaitingInfo));
  payload_len = sizeof(ims_CallWaitingInfo);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_SET_CALL_WAITING;

  ims_CallWaitingInfo *ptr = (ims_CallWaitingInfo*) payload;
  return ptr;
}
void ril_utf_ims_if::set_ims_request_set_call_waiting_service_status(ims_ServiceClassStatus service_status)
{
  ims_CallWaitingInfo *ptr = (ims_CallWaitingInfo*) payload;
  ptr->has_service_status = TRUE;
  ptr->service_status = service_status;
}
ims_SuppSvcResponse *ril_utf_ims_if::update_default_ims_request_set_call_waiting_resp(void)
{
  payload = (ims_SuppSvcResponse*) malloc(sizeof(ims_SuppSvcResponse));
  payload_len = sizeof(ims_SuppSvcResponse);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_SET_CALL_WAITING;

  ims_SuppSvcResponse *ptr = (ims_SuppSvcResponse*) payload;
  return ptr;
}

/*========================================================
  ims_MsgId_REQUEST_IMS_REG_STATE_CHANGE
========================================================*/
void ril_utf_ims_if::update_default_ims_request_ims_reg_state_change(void)
{
  payload = (ims_Registration*) malloc(sizeof(ims_Registration));
  payload_len = sizeof(ims_Registration);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_IMS_REG_STATE_CHANGE;

  ims_Registration *ptr = (ims_Registration*) payload;
}
void ril_utf_ims_if::set_ims_request_ims_reg_state_change_state(ims_Registration_RegState state)
{
  ims_Registration *ptr = (ims_Registration*) payload;
  ptr->has_state = TRUE;
  ptr->state = state;
}
void ril_utf_ims_if::update_default_ims_request_ims_reg_state_change_resp(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_IMS_REG_STATE_CHANGE;

}

/*========================================================
  ims_MsgId_REQUEST_SET_SUPP_SVC_NOTIFICATION
========================================================*/
void ril_utf_ims_if::update_default_ims_request_set_supp_svc_notification(void)
{
  payload = (ims_SuppSvcStatus*) malloc(sizeof(ims_SuppSvcStatus));
  payload_len = sizeof(ims_SuppSvcStatus);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_SET_SUPP_SVC_NOTIFICATION;

  ims_SuppSvcStatus *ptr = (ims_SuppSvcStatus*) payload;
}
void ril_utf_ims_if::set_ims_request_set_supp_svc_notification_status(ims_ServiceClassStatus status)
{
  ims_SuppSvcStatus *ptr = (ims_SuppSvcStatus*) payload;
  ptr->has_status = TRUE;
  ptr->status = status;
}
void ril_utf_ims_if::update_default_ims_request_set_supp_svc_notification_resp(void)
{
  payload = (ims_SuppSvcStatus*) malloc(sizeof(ims_SuppSvcStatus));
  payload_len = sizeof(ims_SuppSvcStatus);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_SET_SUPP_SVC_NOTIFICATION;

  ims_SuppSvcStatus *ptr = (ims_SuppSvcStatus*) payload;
}
void ril_utf_ims_if::set_ims_request_set_supp_svc_notification_resp_status(ims_ServiceClassStatus status)
{
  ims_SuppSvcStatus *ptr = (ims_SuppSvcStatus*) payload;
  ptr->has_status = TRUE;
  ptr->status = status;
}

/*========================================================
  ims_MsgId_REQUEST_LAST_CALL_FAIL_CAUSE
========================================================*/
void ril_utf_ims_if::update_default_ims_request_last_call_fail_cause(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_LAST_CALL_FAIL_CAUSE;
}
void ril_utf_ims_if::update_default_ims_request_last_call_fail_cause_resp(void)
{
  payload = (void *) malloc(sizeof(ims_CallFailCauseResponse));
  payload_len = sizeof(ims_CallFailCauseResponse);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_LAST_CALL_FAIL_CAUSE;
}
void ril_utf_ims_if::set_ims_request_last_call_fail_cause_failcause(ims_CallFailCause failcause)
{
  ims_CallFailCauseResponse *ptr = (ims_CallFailCauseResponse*) payload;
  ptr->has_failcause = TRUE;
  ptr->failcause = failcause;
}
#ifdef RIL_UTF_L_MASTER
void ril_utf_ims_if::set_ims_request_last_call_fail_cause_networkerrorstring(char *networkerrorstring)
{
  ims_CallFailCauseResponse *ptr = (ims_CallFailCauseResponse*) payload;
  ptr->networkErrorString.arg = (char*)malloc(strlen(networkerrorstring)+1);
  strlcpy(((char*)ptr->networkErrorString.arg), networkerrorstring, strlen(networkerrorstring)+1);
}
#endif

/*========================================================
  ims_MsgId_REQUEST_SET_SERVICE_STATUS
========================================================*/
void ril_utf_ims_if::update_default_ims_request_set_service_status(void)
{
  payload = (ims_SetServiceStatusType*) malloc(sizeof(ims_SetServiceStatusType));
  payload_len = sizeof(ims_SetServiceStatusType);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_SET_SERVICE_STATUS;
}
void ril_utf_ims_if::set_ims_request_set_service_status_volte_enabled(bool val)
{
  ims_SetServiceStatusType *ptr = (ims_SetServiceStatusType *) payload;
  ptr->has_volteEnabled = TRUE;
  ptr->volteEnabled = val;
}
void ril_utf_ims_if::set_ims_request_set_service_status_video_telephony_enabled(bool val)
{
  ims_SetServiceStatusType *ptr = (ims_SetServiceStatusType *) payload;
  ptr->has_videoTelephonyEnabled = TRUE;
  ptr->videoTelephonyEnabled = val;
}
void ril_utf_ims_if::set_ims_request_set_service_status_wifi_calling_enabled(bool val)
{
  ims_SetServiceStatusType *ptr = (ims_SetServiceStatusType *) payload;
  ptr->has_wifiCallingEnabled = TRUE;
  ptr->wifiCallingEnabled = val;
}
void ril_utf_ims_if::set_ims_request_set_service_status_ut_enabled(bool val)
{
  ims_SetServiceStatusType *ptr = (ims_SetServiceStatusType *) payload;
  ptr->has_utEnabled = TRUE;
  ptr->utEnabled = val;
}
void ril_utf_ims_if::update_default_ims_request_set_service_status_resp(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_SET_SERVICE_STATUS;
}
/*========================================================
  ims_MsgId_REQUEST_QUERY_SERVICE_STATUS
  use array_idx to index the srvstatusinfo
========================================================*/
void ril_utf_ims_if::update_default_ims_request_query_service_status(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_QUERY_SERVICE_STATUS;
}
void ril_utf_ims_if::update_default_ims_request_query_service_status_resp(int num_entries)
{
  int i;
  payload = (void *) malloc(sizeof(ims_SrvStatusList));
  payload_len = sizeof (ims_SrvStatusList);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_QUERY_SERVICE_STATUS;

  ims_SrvStatusList *ptr = (ims_SrvStatusList *) payload;
  ptr->SrvStatusInfo.arg = (ims_Info **) qcril_malloc (sizeof (ims_Info *) * (num_entries+1));
  ims_Info** tmp_SrvStatusInfo = (ims_Info **)ptr->SrvStatusInfo.arg;
  for (i = 0; i < num_entries; i++)
  {
    tmp_SrvStatusInfo[i] = (ims_Info *) qcril_malloc(sizeof (ims_Info));
    tmp_SrvStatusInfo[i]->accTechStatus.arg = qcril_malloc(sizeof(ims_StatusForAccessTech*) * (1+1));
    ims_StatusForAccessTech **tmp_accTechStatus = (ims_StatusForAccessTech **)tmp_SrvStatusInfo[i]->accTechStatus.arg;
    tmp_accTechStatus[0] = (ims_StatusForAccessTech *)qcril_malloc(sizeof(ims_StatusForAccessTech));
  }
}
void ril_utf_ims_if::set_ims_request_query_service_status_setup_resp_sms_service(int array_idx, uint8_t sms_val, boolean rat_valid, uint8_t rat_val)
{
  ims_SrvStatusList *ptr = (ims_SrvStatusList *) payload;
  ((ims_Info **)ptr->SrvStatusInfo.arg)[array_idx]->has_isValid = TRUE;
  ((ims_Info **)ptr->SrvStatusInfo.arg)[array_idx]->isValid = TRUE;
  ((ims_Info **)ptr->SrvStatusInfo.arg)[array_idx]->has_callType = TRUE;
  ((ims_Info **)ptr->SrvStatusInfo.arg)[array_idx]->callType = ims_CallType_CALL_TYPE_SMS;
  ims_StatusForAccessTech **tmp_accTechStatus = (ims_StatusForAccessTech **)((ims_Info **)ptr->SrvStatusInfo.arg)[array_idx]->accTechStatus.arg;
  tmp_accTechStatus[0]->has_status = TRUE;
  tmp_accTechStatus[0]->has_registered = TRUE;
  tmp_accTechStatus[0]->registered.has_state = TRUE;
  switch (sms_val)
  {
    case IMSA_NO_SERVICE_V01:
      tmp_accTechStatus[0]->status = ims_StatusType_STATUS_DISABLED;
      tmp_accTechStatus[0]->registered.state = ims_Registration_RegState_NOT_REGISTERED;
      break;
    case IMSA_LIMITED_SERVICE_V01:
      tmp_accTechStatus[0]->status = ims_StatusType_STATUS_PARTIALLY_ENABLED;
      tmp_accTechStatus[0]->registered.state = ims_Registration_RegState_REGISTERED;
      break;
    case IMSA_FULL_SERVICE_V01:
      tmp_accTechStatus[0]->status = ims_StatusType_STATUS_ENABLED;
      tmp_accTechStatus[0]->registered.state = ims_Registration_RegState_REGISTERED;
      break;
  }
  if (rat_valid)
  {
     switch (rat_val)
     {
       case IMSA_WLAN_V01:
         tmp_accTechStatus[0]->has_networkMode = TRUE;
         tmp_accTechStatus[0]->networkMode = ims_RadioTechType_RADIO_TECH_WIFI;
         break;
       case IMSA_IWLAN_V01:
         tmp_accTechStatus[0]->has_networkMode = TRUE;
         tmp_accTechStatus[0]->networkMode = ims_RadioTechType_RADIO_TECH_IWLAN;
         break;
       case IMSA_WWAN_V01:
         tmp_accTechStatus[0]->has_networkMode = TRUE;
         tmp_accTechStatus[0]->networkMode = ims_RadioTechType_RADIO_TECH_LTE;
         break;
     }
  }
}
void ril_utf_ims_if::set_ims_request_query_service_status_setup_resp_voip_service(int array_idx, uint8_t voip_val, boolean rat_valid, uint8_t rat_val)
{
  ims_SrvStatusList *ptr = (ims_SrvStatusList *) payload;
  ims_Info **tmp_SrvStatusInfo = (ims_Info **)ptr->SrvStatusInfo.arg;
  tmp_SrvStatusInfo[array_idx]->has_isValid = TRUE;
  tmp_SrvStatusInfo[array_idx]->isValid = TRUE;
  tmp_SrvStatusInfo[array_idx]->has_callType = TRUE;
  tmp_SrvStatusInfo[array_idx]->callType = ims_CallType_CALL_TYPE_VOICE;
  ims_StatusForAccessTech **tmp_accTechStatus = (ims_StatusForAccessTech **)tmp_SrvStatusInfo[array_idx]->accTechStatus.arg;
  tmp_accTechStatus[0]->has_registered = TRUE;
  tmp_accTechStatus[0]->has_status = TRUE;
  tmp_accTechStatus[0]->registered.has_state = TRUE;
  switch (voip_val)
  {
    case IMSA_NO_SERVICE_V01:
      tmp_accTechStatus[0]->status = ims_StatusType_STATUS_DISABLED;
      tmp_accTechStatus[0]->registered.state = ims_Registration_RegState_NOT_REGISTERED;
      break;
    case IMSA_LIMITED_SERVICE_V01:
      tmp_accTechStatus[0]->status = ims_StatusType_STATUS_PARTIALLY_ENABLED;
      tmp_accTechStatus[0]->registered.state = ims_Registration_RegState_REGISTERED;
      break;
    case IMSA_FULL_SERVICE_V01:
      tmp_accTechStatus[0]->status = ims_StatusType_STATUS_ENABLED;
      tmp_accTechStatus[0]->registered.state = ims_Registration_RegState_REGISTERED;
      break;
  }
  if (rat_valid)
  {
     switch (rat_val)
     {
       case IMSA_WLAN_V01:
         tmp_accTechStatus[0]->has_networkMode = TRUE;
         tmp_accTechStatus[0]->networkMode = ims_RadioTechType_RADIO_TECH_WIFI;
         break;
       case IMSA_IWLAN_V01:
         tmp_accTechStatus[0]->has_networkMode = TRUE;
         tmp_accTechStatus[0]->networkMode = ims_RadioTechType_RADIO_TECH_IWLAN;
         break;
       case IMSA_WWAN_V01:
         tmp_accTechStatus[0]->has_networkMode = TRUE;
         tmp_accTechStatus[0]->networkMode = ims_RadioTechType_RADIO_TECH_LTE;
         break;
     }
  }
}
void ril_utf_ims_if::set_ims_request_query_service_status_setup_resp_vt_service(int array_idx, uint8_t vt_val, boolean rat_valid, uint8_t rat_val)
{
  int i;
  ims_CallType call_type[] =
                        {
                            ims_CallType_CALL_TYPE_VT,
                            ims_CallType_CALL_TYPE_VT_TX,
                            ims_CallType_CALL_TYPE_VT_RX,
                        };
  ims_SrvStatusList *ptr = (ims_SrvStatusList *) payload;
  ims_Info **tmp_SrvStatusInfo = (ims_Info **)ptr->SrvStatusInfo.arg;
  for (i = 0; i < sizeof (call_type) / sizeof (ims_CallType); i++)
  {
      tmp_SrvStatusInfo[array_idx + i]->has_isValid = TRUE;
      tmp_SrvStatusInfo[array_idx + i]->isValid = TRUE;
      tmp_SrvStatusInfo[array_idx + i]->has_callType = TRUE;
      tmp_SrvStatusInfo[array_idx + i]->callType = call_type[i];
      ims_StatusForAccessTech **tmp_accTechStatus = (ims_StatusForAccessTech **)tmp_SrvStatusInfo[array_idx + i]->accTechStatus.arg;
      tmp_accTechStatus[0]->has_status = TRUE;
      tmp_accTechStatus[0]->has_registered = TRUE;
      tmp_accTechStatus[0]->registered.has_state = TRUE;
      switch (vt_val)
      {
        case IMSA_NO_SERVICE_V01:
          tmp_accTechStatus[0]->status = ims_StatusType_STATUS_DISABLED;
          tmp_accTechStatus[0]->registered.state = ims_Registration_RegState_NOT_REGISTERED;
          break;
        case IMSA_LIMITED_SERVICE_V01:
          tmp_accTechStatus[0]->status = ims_StatusType_STATUS_PARTIALLY_ENABLED;
          tmp_accTechStatus[0]->registered.state = ims_Registration_RegState_REGISTERED;
          break;
        case IMSA_FULL_SERVICE_V01:
          tmp_accTechStatus[0]->status = ims_StatusType_STATUS_ENABLED;
          tmp_accTechStatus[0]->registered.state = ims_Registration_RegState_REGISTERED;
          break;
      }
      if (rat_valid)
      {
         switch (rat_val)
         {
           case IMSA_WLAN_V01:
             tmp_accTechStatus[0]->has_networkMode = TRUE;
             tmp_accTechStatus[0]->networkMode = ims_RadioTechType_RADIO_TECH_WIFI;
             break;
           case IMSA_IWLAN_V01:
             tmp_accTechStatus[0]->has_networkMode = TRUE;
             tmp_accTechStatus[0]->networkMode = ims_RadioTechType_RADIO_TECH_IWLAN;
             break;
           case IMSA_WWAN_V01:
             tmp_accTechStatus[0]->has_networkMode = TRUE;
             tmp_accTechStatus[0]->networkMode = ims_RadioTechType_RADIO_TECH_LTE;
             break;
         }
      }
  }
}
#ifdef RIL_UTF_L_MASTER
void ril_utf_ims_if::set_ims_request_query_service_status_setup_resp_ut_service(int array_idx, uint8_t ut_val, boolean rat_valid, uint8_t rat_val)
{
  ims_SrvStatusList *ptr = (ims_SrvStatusList *) payload;
  ims_Info **tmp_SrvStatusInfo = (ims_Info **)ptr->SrvStatusInfo.arg;
  tmp_SrvStatusInfo[array_idx]->has_isValid = TRUE;
  tmp_SrvStatusInfo[array_idx]->isValid = TRUE;
  tmp_SrvStatusInfo[array_idx]->has_callType = TRUE;
  tmp_SrvStatusInfo[array_idx]->callType = ims_CallType_CALL_TYPE_UT;
  ims_StatusForAccessTech **tmp_accTechStatus = (ims_StatusForAccessTech **)tmp_SrvStatusInfo[array_idx]->accTechStatus.arg;
  tmp_accTechStatus[0]->has_status = TRUE;
  tmp_accTechStatus[0]->has_registered = TRUE;
  tmp_accTechStatus[0]->registered.has_state = TRUE;
  switch (ut_val)
  {
    case IMSA_NO_SERVICE_V01:
      tmp_accTechStatus[0]->status = ims_StatusType_STATUS_DISABLED;
      tmp_accTechStatus[0]->registered.state = ims_Registration_RegState_NOT_REGISTERED;
      break;
    case IMSA_LIMITED_SERVICE_V01:
      tmp_accTechStatus[0]->status = ims_StatusType_STATUS_PARTIALLY_ENABLED;
      tmp_accTechStatus[0]->registered.state = ims_Registration_RegState_REGISTERED;
      break;
    case IMSA_FULL_SERVICE_V01:
      tmp_accTechStatus[0]->status = ims_StatusType_STATUS_ENABLED;
      tmp_accTechStatus[0]->registered.state = ims_Registration_RegState_REGISTERED;
      break;
  }
  if (rat_valid)
  {
     switch (rat_val)
     {
       case IMSA_WLAN_V01:
         tmp_accTechStatus[0]->has_networkMode = TRUE;
         tmp_accTechStatus[0]->networkMode = ims_RadioTechType_RADIO_TECH_WIFI;
         break;
       case IMSA_IWLAN_V01:
         tmp_accTechStatus[0]->has_networkMode = TRUE;
         tmp_accTechStatus[0]->networkMode = ims_RadioTechType_RADIO_TECH_IWLAN;
         break;
       case IMSA_WWAN_V01:
         tmp_accTechStatus[0]->has_networkMode = TRUE;
         tmp_accTechStatus[0]->networkMode = ims_RadioTechType_RADIO_TECH_LTE;
         break;
     }
  }
}
#endif

/*========================================================
  ims_MsgId_REQUEST_SUPP_SVC_STATUS
========================================================*/
void ril_utf_ims_if::update_default_ims_request_supp_svc_status( void )
{
  payload = (ims_SuppSvcRequest*) malloc( sizeof( ims_SuppSvcRequest) );
  payload_len = sizeof( ims_SuppSvcRequest );
  memset( payload, 0x00, payload_len );
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_SUPP_SVC_STATUS;

  ims_SuppSvcRequest *ptr = ( ims_SuppSvcRequest* ) payload;
}
void ril_utf_ims_if::set_ims_request_supp_svc_status(ims_SuppSvcOperationType operationType, ims_SuppSvcFacilityType facilityType)
{
  ims_SuppSvcRequest *ptr = ( ims_SuppSvcRequest* ) payload;
  ptr->has_operationType = TRUE;
  ptr->operationType = operationType;
  ptr->has_facilityType = TRUE;
  ptr->facilityType = facilityType;
}
void ril_utf_ims_if::set_ims_request_supp_svc_status_service_class(uint32_t service_Class)
{
  ims_SuppSvcRequest *ptr = ( ims_SuppSvcRequest* ) payload;
  ptr->has_cbNumListType = TRUE;
  ptr->cbNumListType.has_serviceClass = TRUE;
  ptr->cbNumListType.serviceClass.has_service_Class = TRUE;
  ptr->cbNumListType.serviceClass.service_Class = service_Class;
}
ims_CbNumList **ril_utf_ims_if::set_ims_request_supp_svc_status_cb_num_list(uint32_t num_list_len)
{
  ims_SuppSvcRequest *ptr = ( ims_SuppSvcRequest* ) payload;
  ptr->has_cbNumListType = TRUE;
  //ptr->cbNumListType.cb_num_list.arg = (ims_CbNumList **) qcril_malloc ((num_list_len+1) * sizeof (ims_CbNumList *));
  ptr->cbNumListType.cb_num_list.arg = qcril_malloc((num_list_len+1) * sizeof (ims_CbNumList *));
  return (ims_CbNumList **)(ptr->cbNumListType.cb_num_list.arg);
}
void ril_utf_ims_if::set_ims_request_supp_svc_status_password(char *passwd) {
  ims_SuppSvcRequest *ptr = (ims_SuppSvcRequest *)payload;
  if (passwd) {
    strlcpy(ptr->password, passwd, sizeof(ptr->password));
  }
}

ims_SuppSvcResponse *ril_utf_ims_if::update_default_ims_request_supp_svc_status_resp( void )
{
  payload = (ims_SuppSvcResponse*) malloc( sizeof( ims_SuppSvcResponse ) );
  payload_len = sizeof( ims_SuppSvcResponse );
  memset( payload, 0x00, payload_len );
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_SUPP_SVC_STATUS;

  ims_SuppSvcResponse *ptr = ( ims_SuppSvcResponse* ) payload;
  return ptr;
}

void ril_utf_ims_if::set_ims_request_set_supp_svc_response_resp_status(ims_ServiceClassStatus status, uint32 facility) {
  ims_SuppSvcResponse *ptr = (ims_SuppSvcResponse *)payload;
  ptr->has_status = TRUE;
  ptr->status = status;
  ptr->has_facilityType = TRUE;
  ptr->facilityType = facility;
}
void ril_utf_ims_if::set_ims_request_set_supp_svc_response_resp_provisionStatus(ims_ServiceClassProvisionStatus status) {
  ims_SuppSvcResponse *ptr = (ims_SuppSvcResponse *)payload;
  ptr->has_provisionStatus = TRUE;
  ptr->provisionStatus = status;
}
#ifndef UTF_TARGET_BUILD
/*========================================================
  ims_MsgId_REQUEST_GET_COLR
========================================================*/
void ril_utf_ims_if::update_default_ims_request_get_colr(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_GET_COLR;
}
void ril_utf_ims_if::update_default_ims_request_get_colr_resp(void)
{
  payload = (ims_Colr*) malloc(sizeof(ims_Colr));
  payload_len = sizeof(ims_Colr);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_GET_COLR;

  ims_Colr *ptr = (ims_Colr*) payload;
}
void ril_utf_ims_if::set_ims_request_get_colr_resp_colr(ims_IpPresentation colr)
{
  ims_Colr *ptr = (ims_Colr*) payload;
  ptr->has_presentation = TRUE;
  ptr->presentation = colr;
}

/*========================================================
  ims_MsgId_REQUEST_SET_COLR
========================================================*/
void ril_utf_ims_if::update_default_ims_request_set_colr( void )
{
  payload = (ims_Colr*) malloc(sizeof(ims_Colr));
  payload_len = sizeof(ims_Colr);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_SET_COLR;

  ims_Colr *ptr = (ims_Colr*) payload;
}
void ril_utf_ims_if::set_ims_request_set_colr_req_colr(ims_IpPresentation colr)
{
  ims_Colr *ptr = (ims_Colr*) payload;
  ptr->has_presentation = TRUE;
  ptr->presentation = colr;
}
void ril_utf_ims_if::update_default_ims_request_set_colr_resp(void)
{
  payload = (ims_SuppSvcResponse*) malloc(sizeof(ims_SuppSvcResponse));
  payload_len = sizeof(ims_SuppSvcResponse);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_SET_COLR;
}
void ril_utf_ims_if::set_sups_sip_error_info(uint32_t code, char* message)
{
  ims_SuppSvcResponse *ptr = (ims_SuppSvcResponse *)payload;
  ptr->has_errorDetails = TRUE;
  ptr->errorDetails.has_sipErrorCode = TRUE;
  ptr->errorDetails.sipErrorCode = code;
  ptr->errorDetails.sipErrorString.arg = message;
}
#endif

/*========================================================
  ims_MsgId_REQUEST_SET_TTY_MODE
========================================================*/
void ril_utf_ims_if::update_default_ims_request_set_tty_mode (ims_Tty_Mode_Type mode)
{
  int i;
  payload = (ims_TtyNotify*) malloc(sizeof(ims_TtyNotify));
  payload_len = sizeof(ims_TtyNotify);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
#ifdef RIL_UTF_L_MASTER
  msg_id = ims_MsgId_REQUEST_SEND_UI_TTY_MODE;
#else
  msg_id = ims_MsgId_REQUEST_SET_TTY_MODE;
#endif

  ims_TtyNotify *ptr = (ims_TtyNotify*) payload;

  ptr->has_mode = TRUE;
  ptr->mode = mode;
}
void ril_utf_ims_if::update_default_ims_request_set_tty_mode_resp(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
#ifdef RIL_UTF_L_MASTER
  msg_id = ims_MsgId_REQUEST_SEND_UI_TTY_MODE;
#else
  msg_id = ims_MsgId_REQUEST_SET_TTY_MODE;
#endif
}

/*========================================================
  ims_MsgId_UNSOL_RSP_BASE
========================================================*/
void ril_utf_ims_if::update_default_ims_unsol_rsp_base(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_RSP_BASE;
}

/*========================================================
  ims_MsgId_UNSOL_CALL_RING
========================================================*/
void ril_utf_ims_if::update_default_ims_unsol_call_ring(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_CALL_RING;
}

/*========================================================
  ims_MsgId_UNSOL_RINGBACK_TONE
========================================================*/
void ril_utf_ims_if::update_default_ims_unsol_ringback_tone(void)
{
  payload = (ims_RingBackTone*) malloc(sizeof(ims_RingBackTone));
  payload_len = sizeof(ims_RingBackTone);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_RINGBACK_TONE;

  ims_RingBackTone *ptr = (ims_RingBackTone*) payload;
}
void ril_utf_ims_if::set_ims_unsol_ringback_tone_flag(ims_RingBackTone_ToneFlag flag)
{
  ims_RingBackTone *ptr = (ims_RingBackTone*) payload;
  ptr->has_flag = TRUE;
  ptr->flag = flag;
}

/*========================================================
  ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
========================================================*/
void ril_utf_ims_if::update_default_ims_unsol_response_call_state_changed(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED;
}

/*========================================================
  ims_MsgId_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED
========================================================*/
void ril_utf_ims_if::update_default_ims_unsol_response_ims_network_state_changed(void)
{
  payload = (void *) malloc(sizeof(ims_Registration));
  payload_len = sizeof(ims_Registration);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED;
}

void ril_utf_ims_if::set_ims_unsol_response_ims_network_state_changed_reg_state(int has_state, ims_Registration_RegState state)
{
  ims_Registration *ptr = (ims_Registration*)payload;
  ptr->has_state = has_state;
  ptr->state = state;
}

void ril_utf_ims_if::set_ims_unsol_response_ims_network_state_changed_reg_radioTech(ims_RadioTechType  val)
{
  ims_Registration *ptr = (ims_Registration*)payload;
  ptr->has_radioTech = TRUE;
  ptr->radioTech = val;
}

void ril_utf_ims_if::set_ims_unsol_response_ims_network_state_changed_reg_pAssociatedUris(const char *val)
{
  ims_Registration *ptr = (ims_Registration*)payload;
  if(val) {
    size_t sz = strlen(val) + 1;
    ptr->pAssociatedUris.arg = qcril_malloc(sz);
    strlcpy((char*)ptr->pAssociatedUris.arg, val, sz);
  }
}

void ril_utf_ims_if::set_ims_unsol_response_ims_network_state_changed_errorcode(int has_errorcode, uint32_t errorcode, char *errormessage)
{
  ims_Registration *ptr = (ims_Registration*)payload;
  ptr->has_errorCode = has_errorcode;
  ptr->errorCode = errorcode;

  if(errormessage)
  {
    size_t sz = strlen(errormessage) + 1;
    ptr->errorMessage.arg = malloc(sz);
    strlcpy((char*)ptr->errorMessage.arg, errormessage, sz);
  }
}

/*========================================================
  ims_MsgId_UNSOL_ENTER_EMERGENCY_CALLBACK_MODE
========================================================*/
void ril_utf_ims_if::update_default_ims_unsol_enter_emergency_callback_mode(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_ENTER_EMERGENCY_CALLBACK_MODE;
}

/*========================================================
  ims_MsgId_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE
========================================================*/
void ril_utf_ims_if::update_default_ims_unsol_exit_emergency_callback_mode(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE;
}

/*========================================================
  ims_MsgId_UNSOL_MODIFY_CALL
========================================================*/
void ril_utf_ims_if::update_default_ims_unsol_modify_call(void)
{
  payload = (ims_CallModify*) malloc(sizeof(ims_CallModify));
  payload_len = sizeof(ims_CallModify);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_MODIFY_CALL;

  ims_CallModify *ptr = (ims_CallModify*) payload;
}
void ril_utf_ims_if::set_ims_unsol_modify_call_index(uint32_t callindex)
{
  ims_CallModify *ptr = (ims_CallModify*) payload;
  ptr->has_callIndex = TRUE;
  ptr->callIndex = callindex;
}
void ril_utf_ims_if::set_ims_unsol_modify_call_error(ims_Error error)
{
  ims_CallModify *ptr = (ims_CallModify*) payload;
  ptr->has_error = TRUE;
  ptr->error = error;
}
void ril_utf_ims_if::set_ims_unsol_modify_call_call_details(ims_CallType calltype, ims_CallDomain calldomain)
{
  ims_CallModify *ptr = (ims_CallModify*) payload;

  ptr->has_callDetails = TRUE;
  ptr->callDetails.has_callDomain = TRUE;
  ptr->callDetails.callDomain = calldomain;
  ptr->callDetails.has_callType = TRUE;
  ptr->callDetails.callType = calltype;
}

/*========================================================
  ims_MsgId_UNSOL_RESPONSE_HANDOVER
========================================================*/
void ril_utf_ims_if::update_default_ims_unsol_response_handover(void)
{
  payload = (ims_Handover*) malloc(sizeof(ims_Handover));
  payload_len = sizeof(ims_Handover);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_RESPONSE_HANDOVER;

  ims_Handover *ptr = (ims_Handover*) payload;

}
void ril_utf_ims_if::set_ims_unsol_response_handover_type(ims_Handover_Msg_Type type)
{
  ims_Handover *ptr = (ims_Handover*) payload;
  ptr->has_type = TRUE;
  ptr->type = type;
}

void ril_utf_ims_if::set_ims_unsol_response_srctech(ims_RadioTechType srctech)
{
  ims_Handover *ptr = (ims_Handover*) payload;
  ptr->has_srcTech = TRUE;
  ptr->srcTech = srctech;
}
void ril_utf_ims_if::set_ims_unsol_response_targettech(ims_RadioTechType targettech)
{
  ims_Handover *ptr = (ims_Handover*) payload;
  ptr->has_targetTech = TRUE;
  ptr->targetTech = targettech;
}
/*========================================================
  ims_MsgId_UNSOL_TTY_NOTIFICATION
========================================================*/
void ril_utf_ims_if::update_default_ims_unsol_response_tty(ims_Tty_Mode_Type tty_mode)
{
  payload = (ims_TtyNotify*) malloc(sizeof(ims_TtyNotify));
  payload_len = sizeof(ims_TtyNotify);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_TTY_NOTIFICATION;

  ims_TtyNotify *ptr = (ims_TtyNotify*) payload;
  ptr->has_mode = TRUE;
  ptr->mode = tty_mode;
}

/*========================================================
  ims_MsgId_UNSOL_REFRESH_CONF_INFO
========================================================*/
void ril_utf_ims_if::update_default_ims_unsol_refresh_conf_info(void)
{
  payload = (ims_ConfInfo*) malloc(sizeof(ims_ConfInfo));
  payload_len = sizeof(ims_ConfInfo);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_REFRESH_CONF_INFO;

  ims_ConfInfo *ptr = (ims_ConfInfo*) payload;
}
//TODO: figure out what nanopb equivalent is for this data type
/*
void ril_utf_ims_if::set_ims_unsol_refresh_conf_info_conf_info_uri(ProtobufCBinaryData conf_info_uri)
{
  ims_ConfInfo *ptr = (ims_ConfInfo*) payload;
  ptr->has_conf_info_uri = TRUE;
  ptr->conf_info_uri = conf_info_uri;
}
*/
void ril_utf_ims_if::update_ims_unsol_refresh_conf_info_conf_info_uri_data(size_t len, uint8_t* data)
{
  payload = (ims_ConfInfo*) malloc(sizeof(ims_ConfInfo));
  payload_len = sizeof(ims_ConfInfo);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_REFRESH_CONF_INFO;

  ims_ConfInfo *ptr = (ims_ConfInfo*) payload;
  qcril_binary_data_type *uri_ptr =
    (qcril_binary_data_type *)qcril_malloc(sizeof(qcril_binary_data_type));
  uri_ptr->len = len;
  uri_ptr->data = (uint8_t *)malloc(len);
  memcpy(uri_ptr->data, data, len);
  ptr->conf_info_uri.arg = uri_ptr;
}

void ril_utf_ims_if::set_ims_unsol_refresh_conf_info_callstate(ims_ConfCallState conf_call_state)
{
  ims_ConfInfo *ptr = (ims_ConfInfo*) payload;
  ptr->has_confCallState = TRUE;
  ptr->confCallState = conf_call_state;
}

/*========================================================
    ims_MsgId_UNSOL_REFRESH_VICE_INFO
========================================================*/
void ril_utf_ims_if::update_ims_unsol_refresh_vice_info_conf_info_uri_data(size_t len, uint8_t* data)
{
  payload = (ims_ViceInfo*) malloc(sizeof(ims_ViceInfo));
  payload_len = sizeof(ims_ViceInfo);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_REFRESH_VICE_INFO;

  ims_ViceInfo *ptr = (ims_ViceInfo*) payload;
  qcril_binary_data_type *uri_ptr =
    (qcril_binary_data_type *)qcril_malloc(sizeof(qcril_binary_data_type));
  uri_ptr->len = len;
  uri_ptr->data = (uint8_t *)malloc(len);
  memcpy(uri_ptr->data, data, len);
  ptr->vice_info_uri.arg = uri_ptr;
}

/*========================================================
    ims_MsgId_UNSOL_AUTO_CALL_REJECTION_IND
========================================================*/
void ril_utf_ims_if::update_ims_unsol_auto_call_rejection_ind(
    ims_CallType callType, ims_CallFailCause autoRejectionCause, uint16_t sipErrorCode)
{
  payload = (ims_AutoCallRejectionInfo*) malloc(sizeof(ims_AutoCallRejectionInfo));
  payload_len = sizeof(ims_AutoCallRejectionInfo);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_AUTO_CALL_REJECTION_IND;

  ims_AutoCallRejectionInfo *ptr = (ims_AutoCallRejectionInfo*) payload;
  ptr->callType = callType;
  ptr->autoRejectionCause = autoRejectionCause;
  ptr->sipErrorCode = sipErrorCode;
}
void ril_utf_ims_if::set_ims_unsol_auto_call_rejection_ind_number(char *number)
{
  ims_AutoCallRejectionInfo *ptr = (ims_AutoCallRejectionInfo*) payload;
  ptr->hasNumber = TRUE;
  ptr->number.arg = qcril_malloc(strlen(number)+1);
  strlcpy((char*)ptr->number.arg, number, strlen(number)+1);
}

/*========================================================
  ims_MsgId_UNSOL_SRV_STATUS_UPDATE
  use array_idx to index the srvstatusinfo
========================================================*/
void ril_utf_ims_if::update_default_ims_unsol_srv_status_update(int num_entries)
{
  int i;
  payload = (void *) malloc(sizeof(ims_SrvStatusList));
  payload_len = sizeof (ims_SrvStatusList);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_SRV_STATUS_UPDATE;

  ims_SrvStatusList *ptr = (ims_SrvStatusList *) payload;
  ptr->SrvStatusInfo.arg = (ims_Info **) qcril_malloc (sizeof (ims_Info *) * (num_entries+1));
  ims_Info** tmp_SrvStatusInfo = (ims_Info **)ptr->SrvStatusInfo.arg;
  for (i = 0; i < num_entries; i++)
  {
    tmp_SrvStatusInfo[i] = (ims_Info *) qcril_malloc(sizeof (ims_Info));
    tmp_SrvStatusInfo[i]->accTechStatus.arg = qcril_malloc(sizeof(ims_StatusForAccessTech*) * (1+1));
    ims_StatusForAccessTech **tmp_accTechStatus = (ims_StatusForAccessTech **)tmp_SrvStatusInfo[i]->accTechStatus.arg;
    tmp_accTechStatus[0] = (ims_StatusForAccessTech *)qcril_malloc(sizeof(ims_StatusForAccessTech));
  }
}
void ril_utf_ims_if::set_ims_unsol_srv_status_update_sms_service(int array_idx, uint8_t sms_val, boolean rat_valid, uint8_t rat_val)
{
  ims_SrvStatusList *ptr = (ims_SrvStatusList *) payload;
  ims_Info **tmp_SrvStatusInfo = (ims_Info **)ptr->SrvStatusInfo.arg;
  tmp_SrvStatusInfo[array_idx]->has_isValid = TRUE;
  tmp_SrvStatusInfo[array_idx]->isValid = TRUE;
  tmp_SrvStatusInfo[array_idx]->has_callType = TRUE;
  tmp_SrvStatusInfo[array_idx]->callType = ims_CallType_CALL_TYPE_SMS;
  ims_StatusForAccessTech **tmp_accTechStatus = (ims_StatusForAccessTech **)tmp_SrvStatusInfo[array_idx]->accTechStatus.arg;
  tmp_accTechStatus[0]->has_status = TRUE;
  tmp_accTechStatus[0]->has_registered = TRUE;
  tmp_accTechStatus[0]->registered.has_state = TRUE;
  switch (sms_val)
  {
    case IMSA_NO_SERVICE_V01:
      tmp_accTechStatus[0]->status = ims_StatusType_STATUS_DISABLED;
      tmp_accTechStatus[0]->registered.state = ims_Registration_RegState_NOT_REGISTERED;
      break;
    case IMSA_LIMITED_SERVICE_V01:
      tmp_accTechStatus[0]->status = ims_StatusType_STATUS_PARTIALLY_ENABLED;
      tmp_accTechStatus[0]->registered.state = ims_Registration_RegState_REGISTERED;
      break;
    case IMSA_FULL_SERVICE_V01:
      tmp_accTechStatus[0]->status = ims_StatusType_STATUS_ENABLED;
      tmp_accTechStatus[0]->registered.state = ims_Registration_RegState_REGISTERED;
      break;
  }
  if (rat_valid)
  {
    switch (rat_val)
    {
      case IMSA_WLAN_V01:
        tmp_accTechStatus[0]->has_networkMode = TRUE;
        tmp_accTechStatus[0]->networkMode = ims_RadioTechType_RADIO_TECH_WIFI;
        break;
      case IMSA_IWLAN_V01:
        tmp_accTechStatus[0]->has_networkMode = TRUE;
        tmp_accTechStatus[0]->networkMode = ims_RadioTechType_RADIO_TECH_IWLAN;
        break;
      case IMSA_WWAN_V01:
        tmp_accTechStatus[0]->has_networkMode = TRUE;
        tmp_accTechStatus[0]->networkMode = ims_RadioTechType_RADIO_TECH_LTE;
        break;
    }
  }
}
void ril_utf_ims_if::set_ims_unsol_srv_status_update_voip_service(int array_idx, uint8_t voip_val, boolean rat_valid, uint8_t rat_val)
{
  ims_SrvStatusList *ptr = (ims_SrvStatusList *) payload;
  ims_Info **tmp_SrvStatusInfo = (ims_Info **)ptr->SrvStatusInfo.arg;
  tmp_SrvStatusInfo[array_idx]->has_isValid = TRUE;
  tmp_SrvStatusInfo[array_idx]->isValid = TRUE;
  tmp_SrvStatusInfo[array_idx]->has_callType = TRUE;
  tmp_SrvStatusInfo[array_idx]->callType = ims_CallType_CALL_TYPE_VOICE;
  ims_StatusForAccessTech **tmp_accTechStatus = (ims_StatusForAccessTech **)tmp_SrvStatusInfo[array_idx]->accTechStatus.arg;
  tmp_accTechStatus[0]->has_status = TRUE;
  tmp_accTechStatus[0]->has_registered = TRUE;
  tmp_accTechStatus[0]->registered.has_state = TRUE;
  switch (voip_val)
  {
    case IMSA_NO_SERVICE_V01:
      tmp_accTechStatus[0]->status = ims_StatusType_STATUS_DISABLED;
      tmp_accTechStatus[0]->registered.state = ims_Registration_RegState_NOT_REGISTERED;
      break;
    case IMSA_LIMITED_SERVICE_V01:
      tmp_accTechStatus[0]->status = ims_StatusType_STATUS_PARTIALLY_ENABLED;
      tmp_accTechStatus[0]->registered.state = ims_Registration_RegState_REGISTERED;
      break;
    case IMSA_FULL_SERVICE_V01:
      tmp_accTechStatus[0]->status = ims_StatusType_STATUS_ENABLED;
      tmp_accTechStatus[0]->registered.state = ims_Registration_RegState_REGISTERED;
      break;
  }
  if (rat_valid)
  {
    switch (rat_val)
    {
      case IMSA_WLAN_V01:
        tmp_accTechStatus[0]->has_networkMode = TRUE;
        tmp_accTechStatus[0]->networkMode = ims_RadioTechType_RADIO_TECH_WIFI;
        break;
      case IMSA_IWLAN_V01:
        tmp_accTechStatus[0]->has_networkMode = TRUE;
        tmp_accTechStatus[0]->networkMode = ims_RadioTechType_RADIO_TECH_IWLAN;
        break;
      case IMSA_WWAN_V01:
        tmp_accTechStatus[0]->has_networkMode = TRUE;
        tmp_accTechStatus[0]->networkMode = ims_RadioTechType_RADIO_TECH_LTE;
        break;
    }
  }
}
void ril_utf_ims_if::set_ims_unsol_srv_status_update_vt_service(int array_idx, uint8_t vt_val, boolean rat_valid, uint8_t rat_val)
{
  int i;
  ims_CallType call_type[] =
  { ims_CallType_CALL_TYPE_VT,
    ims_CallType_CALL_TYPE_VT_TX,
    ims_CallType_CALL_TYPE_VT_RX,
  };
  ims_SrvStatusList *ptr = (ims_SrvStatusList *) payload;
  ims_Info **tmp_SrvStatusInfo = (ims_Info **)ptr->SrvStatusInfo.arg;
  for (i = 0; i < sizeof (call_type) / sizeof (ims_CallType); i++)
  {
    tmp_SrvStatusInfo[array_idx + i]->has_isValid = TRUE;
    tmp_SrvStatusInfo[array_idx + i]->isValid = TRUE;
    tmp_SrvStatusInfo[array_idx + i]->has_callType = TRUE;
    tmp_SrvStatusInfo[array_idx + i]->callType = call_type[i];
    ims_StatusForAccessTech **tmp_accTechStatus = (ims_StatusForAccessTech **)tmp_SrvStatusInfo[array_idx + i]->accTechStatus.arg;
    tmp_accTechStatus[0]->has_status = TRUE;
    tmp_accTechStatus[0]->has_registered = TRUE;
    tmp_accTechStatus[0]->registered.has_state = TRUE;
    switch (vt_val)
    {
      case IMSA_NO_SERVICE_V01:
        tmp_accTechStatus[0]->status = ims_StatusType_STATUS_DISABLED;
        tmp_accTechStatus[0]->registered.state = ims_Registration_RegState_NOT_REGISTERED;
        break;
      case IMSA_LIMITED_SERVICE_V01:
        tmp_accTechStatus[0]->status = ims_StatusType_STATUS_PARTIALLY_ENABLED;
        tmp_accTechStatus[0]->registered.state = ims_Registration_RegState_REGISTERED;
        break;
      case IMSA_FULL_SERVICE_V01:
        tmp_accTechStatus[0]->status = ims_StatusType_STATUS_ENABLED;
        tmp_accTechStatus[0]->registered.state = ims_Registration_RegState_REGISTERED;
        break;
    }
    if (rat_valid)
    {
      switch (rat_val)
      {
        case IMSA_WLAN_V01:
          tmp_accTechStatus[0]->has_networkMode = TRUE;
          tmp_accTechStatus[0]->networkMode = ims_RadioTechType_RADIO_TECH_WIFI;
          break;
        case IMSA_IWLAN_V01:
          tmp_accTechStatus[0]->has_networkMode = TRUE;
          tmp_accTechStatus[0]->networkMode = ims_RadioTechType_RADIO_TECH_IWLAN;
          break;
        case IMSA_WWAN_V01:
          tmp_accTechStatus[0]->has_networkMode = TRUE;
          tmp_accTechStatus[0]->networkMode = ims_RadioTechType_RADIO_TECH_LTE;
          break;
      }
    }
  }
}
#ifdef RIL_UTF_L_MASTER
void ril_utf_ims_if::set_ims_unsol_srv_status_update_ut_service(int array_idx, uint8_t ut_val, boolean rat_valid, uint8_t rat_val)
{
  ims_SrvStatusList *ptr = (ims_SrvStatusList *) payload;
  ims_Info **tmp_SrvStatusInfo = (ims_Info **)ptr->SrvStatusInfo.arg;
  tmp_SrvStatusInfo[array_idx]->has_isValid = TRUE;
  tmp_SrvStatusInfo[array_idx]->isValid = TRUE;
  tmp_SrvStatusInfo[array_idx]->has_callType = TRUE;
  tmp_SrvStatusInfo[array_idx]->callType = ims_CallType_CALL_TYPE_UT;
  ims_StatusForAccessTech **tmp_accTechStatus = (ims_StatusForAccessTech **) tmp_SrvStatusInfo[array_idx]->accTechStatus.arg;
  tmp_accTechStatus[0]->has_status = TRUE;
  tmp_accTechStatus[0]->has_registered = TRUE;
  tmp_accTechStatus[0]->registered.has_state = TRUE;
  switch (ut_val)
  {
    case IMSA_NO_SERVICE_V01:
      tmp_accTechStatus[0]->status = ims_StatusType_STATUS_DISABLED;
      tmp_accTechStatus[0]->registered.state = ims_Registration_RegState_NOT_REGISTERED;
      break;
    case IMSA_LIMITED_SERVICE_V01:
      tmp_accTechStatus[0]->status = ims_StatusType_STATUS_PARTIALLY_ENABLED;
      tmp_accTechStatus[0]->registered.state = ims_Registration_RegState_REGISTERED;
      break;
    case IMSA_FULL_SERVICE_V01:
      tmp_accTechStatus[0]->status = ims_StatusType_STATUS_ENABLED;
      tmp_accTechStatus[0]->registered.state = ims_Registration_RegState_REGISTERED;
      break;
  }
  if (rat_valid)
  {
    switch (rat_val)
    {
      case IMSA_WLAN_V01:
        tmp_accTechStatus[0]->has_networkMode = TRUE;
        tmp_accTechStatus[0]->networkMode = ims_RadioTechType_RADIO_TECH_WIFI;
        break;
      case IMSA_IWLAN_V01:
        tmp_accTechStatus[0]->has_networkMode = TRUE;
        tmp_accTechStatus[0]->networkMode = ims_RadioTechType_RADIO_TECH_IWLAN;
        break;
      case IMSA_WWAN_V01:
        tmp_accTechStatus[0]->has_networkMode = TRUE;
        tmp_accTechStatus[0]->networkMode = ims_RadioTechType_RADIO_TECH_LTE;
        break;
    }
  }
}
#endif
/*========================================================
  ims_MsgId_UNSOL_SUPP_SVC_NOTIFICATION
========================================================*/
void ril_utf_ims_if::update_default_ims_unsol_supp_svc_notification(void)
{
  payload = (ims_SuppSvcNotification*) malloc(sizeof(ims_SuppSvcNotification));
  payload_len = sizeof(ims_SuppSvcNotification);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_SUPP_SVC_NOTIFICATION;

  ims_SuppSvcNotification *ptr = (ims_SuppSvcNotification*) payload;
}
void ril_utf_ims_if::set_ims_unsol_supp_svc_notification_number(char *number)
{
  ims_SuppSvcNotification *ptr = (ims_SuppSvcNotification*) payload;
  size_t sz = strlen(number) + 1;
  ptr->number.arg = (char*)malloc(sz);
  strlcpy( ((char*)ptr->number.arg), number, sz );
}
void ril_utf_ims_if::set_ims_unsol_supp_svc_notification_code(uint32_t code)
{
  ims_SuppSvcNotification *ptr = (ims_SuppSvcNotification*) payload;
  ptr->has_code = TRUE;
  ptr->code = code;
}
void ril_utf_ims_if::set_ims_unsol_supp_svc_notification_index(uint32_t index)
{
  ims_SuppSvcNotification *ptr = (ims_SuppSvcNotification*) payload;
  ptr->has_index = TRUE;
  ptr->index = index;
}
void ril_utf_ims_if::set_ims_unsol_supp_svc_notification_type(uint32_t type)
{
  ims_SuppSvcNotification *ptr = (ims_SuppSvcNotification*) payload;
  ptr->has_type = TRUE;
  ptr->type = type;
}
void ril_utf_ims_if::set_ims_unsol_supp_svc_notification_connid(uint32_t connid)
{
  ims_SuppSvcNotification *ptr = (ims_SuppSvcNotification*) payload;
  ptr->has_connId = TRUE;
  ptr->connId = connid;
}
void ril_utf_ims_if::set_ims_unsol_supp_svc_notification_notificationtype(ims_NotificationType notificationtype)
{
  ims_SuppSvcNotification *ptr = (ims_SuppSvcNotification*) payload;
  ptr->has_notificationType = TRUE;
  ptr->notificationType = notificationtype;
}
#ifdef RIL_UTF_L_MASTER
void ril_utf_ims_if::set_ims_unsol_supp_svc_notification_history_info(char *history_info)
{
  ims_SuppSvcNotification *ptr = (ims_SuppSvcNotification*) payload;
  ptr->history_info.arg = qmi_ril_util_str_clone(history_info);
}
#endif
void ril_utf_ims_if::set_ims_unsol_supp_svc_notification_hold_tone(bool hold_tone)
{
  ims_SuppSvcNotification *ptr = (ims_SuppSvcNotification*) payload;
  ptr->has_hold_tone = TRUE;
  ptr->hold_tone = hold_tone;
}

/*========================================================
  ims_MsgId_UNSOL_RADIO_STATE_CHANGED
========================================================*/
void ril_utf_ims_if::update_default_ims_unsol_radio_state_changed(ims_RadioState state)
{
  payload = (ims_RadioStateChanged*) malloc(sizeof(ims_RadioStateChanged));
  payload_len = sizeof(ims_RadioStateChanged);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_RADIO_STATE_CHANGED;
  ims_RadioStateChanged* ptr = (ims_RadioStateChanged*)payload;
  ptr->has_state = true;
  ptr->state = state;
}

/*========================================================
  ims_MsgId_UNSOL_WFC_ROAMING_MODE_CONFIG_SUPPORT
========================================================*/
void ril_utf_ims_if::update_default_ims_unsol_wfc_roaming_config_support(boolean wfcRoamingConfigSupport)
{
  payload = (ims_WfcRoamingConfigSupportInfo*) malloc(sizeof(ims_WfcRoamingConfigSupportInfo));
  payload_len = sizeof(ims_WfcRoamingConfigSupportInfo);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_WFC_ROAMING_MODE_CONFIG_SUPPORT;
  ims_WfcRoamingConfigSupportInfo* ptr = (ims_WfcRoamingConfigSupportInfo*)payload;
  ptr->wfcRoamingConfigSupport = wfcRoamingConfigSupport;
}

/*========================================================
  ims_MsgId_UNSOL_USSD_FAILED
========================================================*/
void ril_utf_ims_if::update_default_ims_unsol_ussd_failed(
        ims_UssdModeType type, int has_errorcode, uint32_t code, char* message)
{
  payload = (ims_UssdMessageFailedInfo*) malloc(sizeof(ims_UssdMessageFailedInfo));
  payload_len = sizeof(ims_UssdMessageFailedInfo);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_USSD_FAILED;
  ims_UssdMessageFailedInfo* ptr = (ims_UssdMessageFailedInfo*)payload;
  ptr->type = type;
  ptr->errorDetails.has_sipErrorCode = has_errorcode;
  ptr->errorDetails.sipErrorCode = code;
  if (message)
  {
    size_t sz = strlen(message) + 1;
    ptr->errorDetails.sipErrorString.arg = malloc(sz);
    strlcpy((char*)ptr->errorDetails.sipErrorString.arg, message, sz);
  }
}

#ifndef UTF_TARGET_BUILD
/*========================================================
  ims_MsgId_REQUEST_QUERY_VT_CALL_QUALITY
========================================================*/
void ril_utf_ims_if::update_default_ims_request_query_vt_call_quality(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_QUERY_VT_CALL_QUALITY;
}
void ril_utf_ims_if::update_default_ims_request_query_vt_call_quality_resp(void)
{
  payload = (ims_VideoCallQuality*) malloc(sizeof(ims_VideoCallQuality));
  payload_len = sizeof(ims_VideoCallQuality);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_QUERY_VT_CALL_QUALITY;

  ims_VideoCallQuality *ptr = (ims_VideoCallQuality*) payload;
}
void ril_utf_ims_if::set_ims_request_query_vt_call_quality_resp_video_quality(ims_Quality quality)
{
  ims_VideoCallQuality *ptr = (ims_VideoCallQuality*) payload;
  ptr->has_quality = TRUE;
  ptr->quality = quality;
}

/*========================================================
  ims_MsgId_REQUEST_SET_VT_CALL_QUALITY
========================================================*/
void ril_utf_ims_if::update_default_ims_request_set_vt_call_quality(void)
{
  payload = (ims_VideoCallQuality*) malloc(sizeof(ims_VideoCallQuality));
  payload_len = sizeof(ims_VideoCallQuality);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_SET_VT_CALL_QUALITY;

  ims_VideoCallQuality *ptr = (ims_VideoCallQuality*) payload;
}
void ril_utf_ims_if::set_ims_request_set_vt_call_quality_resp_video_quality(ims_Quality quality)
{
  ims_VideoCallQuality *ptr = (ims_VideoCallQuality*) payload;
  ptr->has_quality = TRUE;
  ptr->quality = quality;
}
void ril_utf_ims_if::update_default_ims_request_set_vt_call_quality_resp(void)
{
  payload = NULL;
  payload_len = 0;
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_SET_VT_CALL_QUALITY;
}

/*========================================================
  ims_MsgId_UNSOL_MWI
  use array_idx to index the srvstatusinfo
========================================================*/
void ril_utf_ims_if::update_default_ims_unsol_mwi(int n_mwimsgsummary, int n_mwimsgdetail)
{
  int i;
  payload = (void *) malloc(sizeof(ims_Mwi));
  payload_len = sizeof (ims_Mwi);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_MWI;

  ims_Mwi *ptr = (ims_Mwi *) payload;
  ptr->mwiMsgSummary.arg = (ims_MwiMessageSummary **) malloc (
          sizeof (ims_MwiMessageSummary *) * n_mwimsgsummary);
  ims_MwiMessageSummary **tmp_mwiMsgSummary = (ims_MwiMessageSummary **)ptr->mwiMsgSummary.arg;
  for (i = 0; i < n_mwimsgsummary; i++)
  {
      tmp_mwiMsgSummary[i] = (ims_MwiMessageSummary *) malloc (
              sizeof (ims_MwiMessageSummary));
  }
  if (n_mwimsgdetail > 0)
  {
    ptr->mwiMsgDetail.arg = (ims_MwiMessageDetails **) malloc (
              sizeof (ims_MwiMessageDetails *) * n_mwimsgdetail);
    ims_MwiMessageDetails **tmp_mwiMsgDetail = (ims_MwiMessageDetails **)ptr->mwiMsgDetail.arg;
    for (i = 0; i < n_mwimsgdetail; i++)
    {
      tmp_mwiMsgDetail[i] = (ims_MwiMessageDetails *) malloc (
              sizeof (ims_MwiMessageDetails));
    }
  }
}

void ril_utf_ims_if::set_ims_unsol_mwi_message_summary(
    int array_idx,
    ims_MwiMessageType messagetype,
    uint32_t newmessage,
    uint32_t oldmessage,
    uint32_t newurgent,
    uint32_t oldurgent)
{
  ims_Mwi *ptr = (ims_Mwi *) payload;
  ims_MwiMessageSummary **tmp_mwiMsgSummary = (ims_MwiMessageSummary **)ptr->mwiMsgSummary.arg;

  tmp_mwiMsgSummary[array_idx]->has_messageType = TRUE;
  tmp_mwiMsgSummary[array_idx]->messageType     = messagetype;
  tmp_mwiMsgSummary[array_idx]->has_newMessage = TRUE;
  tmp_mwiMsgSummary[array_idx]->newMessage     = newmessage;
  tmp_mwiMsgSummary[array_idx]->has_oldMessage = TRUE;
  tmp_mwiMsgSummary[array_idx]->oldMessage     = oldmessage;
  tmp_mwiMsgSummary[array_idx]->has_newUrgent = TRUE;
  tmp_mwiMsgSummary[array_idx]->newUrgent     = newurgent;
  tmp_mwiMsgSummary[array_idx]->has_oldUrgent = TRUE;
  tmp_mwiMsgSummary[array_idx]->oldUrgent     = oldurgent;
}

void ril_utf_ims_if::set_ims_unsol_mwi_ueaddress(char *ueaddress)
{
  ims_Mwi *ptr = (ims_Mwi *) payload;
  ptr->ueAddress.arg = qmi_ril_util_str_clone(ueaddress);
}

void ril_utf_ims_if::set_ims_unsol_mwi_message_details(
    int array_idx,
    char *toaddress,
    char *fromaddress,
    char *subject,
    char *date,
    ims_MwiPriority priority,
    char *messageid,
    ims_MwiMessageType messagetype)
{
  ims_Mwi *ptr = (ims_Mwi *) payload;
  ims_MwiMessageDetails **tmp_mwiMsgDetail = (ims_MwiMessageDetails **)ptr->mwiMsgDetail.arg;

  tmp_mwiMsgDetail[array_idx]->toAddress.arg = qmi_ril_util_str_clone(toaddress);
  tmp_mwiMsgDetail[array_idx]->fromAddress.arg = qmi_ril_util_str_clone(fromaddress);
  tmp_mwiMsgDetail[array_idx]->subject.arg = qmi_ril_util_str_clone(subject);
  tmp_mwiMsgDetail[array_idx]->date.arg = qmi_ril_util_str_clone(date);
  tmp_mwiMsgDetail[array_idx]->has_priority = TRUE;
  tmp_mwiMsgDetail[array_idx]->priority     = priority;
  tmp_mwiMsgDetail[array_idx]->messageId.arg = qmi_ril_util_str_clone(messageid);
  tmp_mwiMsgDetail[array_idx]->has_messageType = TRUE;
  tmp_mwiMsgDetail[array_idx]->messageType     = messagetype;
}

#endif

#ifdef RIL_UTF_L_MASTER
/*========================================================
  ims_MsgId_REQUEST_HOLD
========================================================*/
ims_Hold* ril_utf_ims_if::update_default_ims_request_hold(int callId)
{
  payload = (void *) malloc(sizeof(ims_Hold));
  payload_len = sizeof(ims_Hold);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_HOLD;

  ims_Hold* ptr = (ims_Hold*) payload;
  ptr->has_callId = TRUE;
  ptr->callId = callId;
  return ptr;
}

ims_SipErrorInfo *ril_utf_ims_if::update_default_ims_request_hold_resp()
{
  payload = (void *) malloc(sizeof(_ims_SipErrorInfo));
  payload_len = sizeof(_ims_SipErrorInfo);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_HOLD;

  ims_SipErrorInfo *ptr = (ims_SipErrorInfo *) payload;
  return  ptr;
}

/*========================================================
  ims_MsgId_REQUEST_RESUME
========================================================*/
ims_Resume* ril_utf_ims_if::update_default_ims_request_resume(int callId)
{
  payload = (void *) malloc(sizeof(ims_Resume));
  payload_len = sizeof(ims_Resume);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_RESUME;

  ims_Resume* ptr = (ims_Resume*) payload;
  ptr->has_callId = TRUE;
  ptr->callId = callId;
  return ptr;
}

void ril_utf_ims_if::update_default_ims_request_resume_resp()
{
  payload = (void *) malloc(sizeof(_ims_SipErrorInfo));;
  payload_len = sizeof(_ims_SipErrorInfo);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_RESUME;
}

/*========================================================
  ims_MsgId_REQUEST_GET_WIFI_CALLING_STATUS
========================================================*/
void ril_utf_ims_if::update_default_ims_request_get_wifi_calling_status(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_GET_WIFI_CALLING_STATUS;
}
ims_WifiCallingInfo *ril_utf_ims_if::update_default_ims_request_get_wifi_calling_status_resp(void)
{
  payload = (ims_WifiCallingInfo*) malloc(sizeof(ims_WifiCallingInfo));
  payload_len = sizeof(ims_WifiCallingInfo);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_GET_WIFI_CALLING_STATUS;

  ims_WifiCallingInfo *ptr = (ims_WifiCallingInfo*) payload;
  return ptr;
}
void ril_utf_ims_if::set_ims_request_get_wifi_calling_status_resp_wificallinginfo(
          ims_WifiCallingStatus status,
          ims_WifiCallingPreference preference)
{
  ims_WifiCallingInfo *ptr = (ims_WifiCallingInfo*) payload;
  ptr->has_status = TRUE;
  ptr->status = status;
  ptr->has_preference = TRUE;
  ptr->preference = preference;
}

/*========================================================
  ims_MsgId_REQUEST_SET_WIFI_CALLING_STATUS
========================================================*/
ims_WifiCallingInfo *ril_utf_ims_if::update_default_ims_request_set_wifi_calling_status(void)
{
  payload = (ims_WifiCallingInfo*) malloc(sizeof(ims_WifiCallingInfo));
  payload_len = sizeof(ims_WifiCallingInfo);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_SET_WIFI_CALLING_STATUS;

  ims_WifiCallingInfo *ptr = (ims_WifiCallingInfo*) payload;
  return ptr;
}
void ril_utf_ims_if::set_ims_request_set_wifi_calling_status_wificallinginfo(
          ims_WifiCallingStatus status,
          ims_WifiCallingPreference preference)
{
  ims_WifiCallingInfo *ptr = (ims_WifiCallingInfo*) payload;
  ptr->has_status = TRUE;
  ptr->status = status;
  ptr->has_preference = TRUE;
  ptr->preference = preference;
}
void ril_utf_ims_if::update_default_ims_request_set_wifi_calling_status_resp(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_SET_WIFI_CALLING_STATUS;
}

/*========================================================
  ims_MsgId_REQUEST_SET_IMS_CONFIG
========================================================*/
ims_ConfigMsg *ril_utf_ims_if::update_default_ims_request_set_config(ims_ConfigMsg *req)
{
  payload = (void *) malloc(sizeof(ims_ConfigMsg));
  payload_len = sizeof(ims_ConfigMsg);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_SET_IMS_CONFIG;
  if (req) {
    memcpy(payload, req, payload_len);
  }
  ims_ConfigMsg *ptr = (ims_ConfigMsg *)payload;
  return ptr;
}

ims_ConfigMsg *ril_utf_ims_if::update_default_ims_request_set_config_resp()
{
  payload = (void *) malloc(sizeof(ims_ConfigMsg));
  payload_len = sizeof(ims_ConfigMsg);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_SET_IMS_CONFIG;

  ims_ConfigMsg *ptr = (ims_ConfigMsg *)payload;
  return ptr;
}

ims_ConfigMsg* ril_utf_ims_if::set_ims_request_set_config()
{
  payload = (void *) malloc(sizeof(ims_ConfigMsg));
  payload_len = sizeof(ims_ConfigMsg);
  memset(payload, 0x00, payload_len);
  ims_ConfigMsg *ptr = (ims_ConfigMsg *)payload;
  return ptr;
}

/*========================================================
  ims_MsgId_REQUEST_GET_IMS_CONFIG
========================================================*/

ims_ConfigMsg *ril_utf_ims_if::update_default_ims_request_get_config()
{
  payload = (void *) malloc(sizeof(ims_ConfigMsg));
  payload_len = sizeof(ims_ConfigMsg);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_GET_IMS_CONFIG;

  ims_ConfigMsg *ptr = (ims_ConfigMsg *)payload;
  return ptr;
}

ims_ConfigMsg *ril_utf_ims_if::update_default_ims_request_get_config_resp()
{
  payload = (void *) malloc(sizeof(ims_ConfigMsg));
  payload_len = sizeof(ims_ConfigMsg);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_GET_IMS_CONFIG;
  ims_ConfigMsg *ptr = (ims_ConfigMsg *)payload;
  return ptr;
}
ims_ConfigMsg* ril_utf_ims_if::set_ims_request_get_config()
{
  payload = (void *) malloc(sizeof(ims_ConfigMsg));
  payload_len = sizeof(ims_ConfigMsg);
  memset(payload, 0x00, payload_len);
  ims_ConfigMsg *ptr = (ims_ConfigMsg *)payload;
  return ptr;
}

/*========================================================
  ims_MsgId_UNSOL_REQUEST_GEOLOCATION
========================================================*/

void ril_utf_ims_if::update_default_ims_unsol_request_geolocation()
{
  payload = malloc(sizeof(ims_GeoLocationInfo));
  ims_GeoLocationInfo *info = (ims_GeoLocationInfo *)payload;
  payload_len = sizeof(ims_GeoLocationInfo);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_REQUEST_GEOLOCATION;
}

/*========================================================
  ims_MsgId_REQUEST_SEND_RTT_MSG
========================================================*/
void ril_utf_ims_if::update_default_ims_request_send_rtt_message()
{
  payload = qcril_malloc(sizeof(ims_RttMessage));
  ims_RttMessage *info = (ims_RttMessage *)payload;
  payload_len = sizeof(ims_RttMessage);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_SEND_RTT_MSG;
}

void ril_utf_ims_if::set_ims_rtt_message_text(const char *msg)
{
  ims_RttMessage *ptr = (ims_RttMessage*) payload;
  ptr->rttMessage.arg = (char *) malloc(strlen(msg)+1);
  strlcpy((char *)ptr->rttMessage.arg, msg, strlen(msg)+1);
}

void ril_utf_ims_if::update_default_ims_request_send_rtt_message_resp(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_SEND_RTT_MSG;
}

/*========================================================
  ims_MsgId_UNSOL_RESPONSE_RTT_MSG_RECEIVED
========================================================*/
void ril_utf_ims_if::update_default_ims_unsol_rtt_message()
{
  payload = malloc(sizeof(ims_RttMessage));
  ims_RttMessage *info = (ims_RttMessage *)payload;
  payload_len = sizeof(ims_RttMessage);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_RESPONSE_RTT_MSG_RECEIVED;
}

/*========================================================
  ims_MsgId_REQUEST_SEND_GEOLOCATION_INFO
========================================================*/

void ril_utf_ims_if::update_default_ims_request_send_geolocation_info()
{
  payload = qcril_malloc(sizeof(ims_GeoLocationInfo));
  ims_GeoLocationInfo *info = (ims_GeoLocationInfo *)payload;
  payload_len = sizeof(ims_GeoLocationInfo);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_SEND_GEOLOCATION_INFO;
}

void ril_utf_ims_if::set_ims_geolocation_info_lat(double lat)
{
  ims_GeoLocationInfo *info = (ims_GeoLocationInfo *)payload;
  info->has_lat = TRUE;
  info->lat = lat;
}

void ril_utf_ims_if::set_ims_geolocation_info_lon(double lon)
{
  ims_GeoLocationInfo *info = (ims_GeoLocationInfo *)payload;
  info->has_lon = TRUE;
  info->lon = lon;
}

void ril_utf_ims_if::set_ims_geolocation_info_city(const char *city)
{
  ims_GeoLocationInfo *info = (ims_GeoLocationInfo *)payload;
  size_t sz = strlen(city) + 1;
  info->has_addressInfo = TRUE;
  info->addressInfo.city.arg = qcril_malloc(sz);
  strlcpy((char *)info->addressInfo.city.arg, city, sz);
}
void ril_utf_ims_if::set_ims_geolocation_info_state(const char *state)
{
  ims_GeoLocationInfo *info = (ims_GeoLocationInfo *)payload;
  size_t sz = strlen(state) + 1;
  info->has_addressInfo = TRUE;
  info->addressInfo.state.arg = qcril_malloc(sz);
  strlcpy((char *)info->addressInfo.state.arg, state, sz);
}
void ril_utf_ims_if::set_ims_geolocation_info_country(const char *country)
{
  ims_GeoLocationInfo *info = (ims_GeoLocationInfo *)payload;
  size_t sz = strlen(country) + 1;
  info->has_addressInfo = TRUE;
  info->addressInfo.country.arg = qcril_malloc(sz);
  strlcpy((char *)info->addressInfo.country.arg, country, sz);
}
void ril_utf_ims_if::set_ims_geolocation_info_postalCode(const char *postalCode)
{
  ims_GeoLocationInfo *info = (ims_GeoLocationInfo *)payload;
  size_t sz = strlen(postalCode) + 1;
  info->has_addressInfo = TRUE;
  info->addressInfo.postalCode.arg = qcril_malloc(sz);
  strlcpy((char *)info->addressInfo.postalCode.arg, postalCode, sz);
}
void ril_utf_ims_if::set_ims_geolocation_info_countryCode(const char *countryCode)
{
  ims_GeoLocationInfo *info = (ims_GeoLocationInfo *)payload;
  size_t sz = strlen(countryCode) + 1;
  info->has_addressInfo = TRUE;
  info->addressInfo.countryCode.arg = qcril_malloc(sz);
  strlcpy((char *)info->addressInfo.countryCode.arg, countryCode, sz);
}
void ril_utf_ims_if::set_ims_geolocation_info_street(const char *street)
{
  ims_GeoLocationInfo *info = (ims_GeoLocationInfo *)payload;
  size_t sz = strlen(street) + 1;
  info->has_addressInfo = TRUE;
  info->addressInfo.street.arg = qcril_malloc(sz);
  strlcpy((char *)info->addressInfo.street.arg, street, sz);
}
void ril_utf_ims_if::set_ims_geolocation_info_houseNumber(const char *houseNumber)
{
  ims_GeoLocationInfo *info = (ims_GeoLocationInfo *)payload;
  size_t sz = strlen(houseNumber) + 1;
  info->has_addressInfo = TRUE;
  info->addressInfo.houseNumber.arg = qcril_malloc(sz);
  strlcpy((char *)info->addressInfo.houseNumber.arg, houseNumber, sz);
}

void ril_utf_ims_if::update_default_ims_request_send_geolocation_info_resp()
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_SEND_GEOLOCATION_INFO;
}

/*========================================================
  ims_MsgId_REQUEST_GET_IMS_SUB_CONFIG
========================================================*/
void ril_utf_ims_if::update_default_ims_request_get_ims_sub_config()
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_GET_IMS_SUB_CONFIG;
}
ims_ImsSubConfig *ril_utf_ims_if::update_default_ims_request_get_ims_sub_config_resp()
{
  payload = qcril_malloc(sizeof(ims_ImsSubConfig));
  payload_len = sizeof(ims_ImsSubConfig);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_GET_IMS_SUB_CONFIG;
  ims_ImsSubConfig *ptr = (ims_ImsSubConfig *)payload;
  return ptr;
}
void ril_utf_ims_if::set_ims_request_get_ims_sub_config_resp_simultStackCount(uint32_t val)
{
  ims_ImsSubConfig *ptr = (ims_ImsSubConfig *)payload;
  ptr->has_simultStackCount = TRUE;
  ptr->simultStackCount = val;
}
void ril_utf_ims_if::set_ims_request_get_ims_sub_config_resp_imsStackEnabled(uint64_t *val, size_t len)
{
  ims_ImsSubConfig *ptr = (ims_ImsSubConfig *)payload;
  uint64_t **stackEnabled = (uint64_t **)qcril_malloc(sizeof(uint64_t*) * (len+1));
  for (size_t i = 0; i < len; i++) {
    stackEnabled[i] = (uint64_t *)qcril_malloc(sizeof(uint64_t));
    *(stackEnabled[i]) = val[i];
  }
  ptr->imsStackEnabled.arg = stackEnabled;
}

/*========================================================
  ims_MsgId_UNSOL_IMS_SUB_CONFIG_CHANGED
========================================================*/
ims_ImsSubConfig *ril_utf_ims_if::update_default_ims_unsol_ims_sub_config_changed()
{
  payload = qcril_malloc(sizeof(ims_ImsSubConfig));
  payload_len = sizeof(ims_ImsSubConfig);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_IMS_SUB_CONFIG_CHANGED;
  ims_ImsSubConfig *ptr = (ims_ImsSubConfig *)payload;
  return ptr;
}
void ril_utf_ims_if::set_ims_unsol_ims_sub_config_changed_simultStackCount(uint32_t val)
{
  ims_ImsSubConfig *ptr = (ims_ImsSubConfig *)payload;
  ptr->has_simultStackCount = TRUE;
  ptr->simultStackCount = val;
}
void ril_utf_ims_if::set_ims_unsol_ims_sub_config_changed_imsStackEnabled(uint64_t *val, size_t len)
{
  ims_ImsSubConfig *ptr = (ims_ImsSubConfig *)payload;
  uint64_t **stackEnabled = (uint64_t **)qcril_malloc(sizeof(uint64_t*) * (len+1));
  for (size_t i = 0; i < len; i++) {
    stackEnabled[i] = (uint64_t *)qcril_malloc(sizeof(uint64_t));
    *(stackEnabled[i]) = val[i];
  }
  ptr->imsStackEnabled.arg = stackEnabled;
}

/*========================================================
  ims_MsgId_REQUEST_REGISTER_MULTI_IDENTITY_LINES
========================================================*/
ims_MultiIdentityLineInfo *ril_utf_ims_if::update_default_ims_request_register_multi_identity_lines(uint32_t lineInfo_len)
{
  payload = qcril_malloc(sizeof(ims_MultiIdentityStatus));
  payload_len = sizeof(ims_MultiIdentityStatus);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_REGISTER_MULTI_IDENTITY_LINES;
  ims_MultiIdentityStatus *ptr = (ims_MultiIdentityStatus *)payload;
  ptr->lineInfo_len = lineInfo_len;
  return (ptr->lineInfo);
}
void ril_utf_ims_if::update_default_ims_request_register_multi_identity_lines_resp()
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_REGISTER_MULTI_IDENTITY_LINES;
}

/*========================================================
  ims_MsgId_REQUEST_QUERY_VIRTUAL_LINE_INFO
========================================================*/
void ril_utf_ims_if::update_default_ims_request_query_virtual_line_info(const char *msisdn)
{
  payload = qcril_malloc(sizeof(ims_VirtualLineInfoReq));
  payload_len = sizeof(ims_VirtualLineInfoReq);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_QUERY_VIRTUAL_LINE_INFO;
  ims_VirtualLineInfoReq *ptr = (ims_VirtualLineInfoReq *)payload;
  strlcpy(ptr->msisdn, msisdn, strlen(msisdn)+1);
}
void ril_utf_ims_if::update_default_ims_request_query_virtual_line_info_resp(const char *msisdn, uint32_t lines_len)
{
  payload = qcril_malloc(sizeof(ims_VirtualLineInfoResp));
  payload_len = sizeof(ims_VirtualLineInfoResp);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_QUERY_VIRTUAL_LINE_INFO;
  ims_VirtualLineInfoResp *ptr = (ims_VirtualLineInfoResp *)payload;
  strlcpy(ptr->msisdn, msisdn, strlen(msisdn)+1);
  ptr->lines_len = lines_len;
}
void ril_utf_ims_if::set_ims_request_query_virtual_line_info_resp_lines(uint32_t i, char *line)
{
  ims_VirtualLineInfoResp *ptr = (ims_VirtualLineInfoResp *)payload;
  if (i < 10) {
    strlcpy(ptr->lines[i], line, strlen(line)+1);
  }
}

/*========================================================
  ims_MsgId_UNSOL_MULTI_IDENTITY_PENDING_INFO
========================================================*/
void ril_utf_ims_if::update_default_ims_unsol_multi_identity_pending_info(void)
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_MULTI_IDENTITY_PENDING_INFO;
}

/*========================================================
  ims_MsgId_UNSOL_MULTI_IDENTITY_REGISTRATION_STATUS
========================================================*/
ims_MultiIdentityLineInfo *ril_utf_ims_if::update_default_ims_unsol_multi_identity_registration_status(uint32_t lineInfo_len)
{
  payload = malloc(sizeof(ims_MultiIdentityStatus));
  payload_len = sizeof(ims_MultiIdentityStatus);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_MULTI_IDENTITY_REGISTRATION_STATUS;
  ims_MultiIdentityStatus *ptr = (ims_MultiIdentityStatus*) payload;
  ptr->lineInfo_len = lineInfo_len;
  return(ptr->lineInfo);
}

/*========================================================
  ims_MsgId_UNSOL_VOICE_INFO
========================================================*/
void ril_utf_ims_if::update_default_ims_unsol_voice_info(ims_VoiceInfo voiceInfo)
{
  payload = malloc(sizeof(ims_VoiceInfoData));
  payload_len = sizeof(ims_VoiceInfoData);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_VOICE_INFO;
  ims_VoiceInfoData *ptr = (ims_VoiceInfoData*) payload;
  ptr->voiceInfo = voiceInfo;
}

/*========================================================
  ims_MsgId_UNSOL_VOWIFI_CALL_QUALITY
========================================================*/
void ril_utf_ims_if::update_default_ims_unsol_vowifi_call_quality(ims_voWiFiQuality quality)
{
  payload = malloc(sizeof(ims_VoWiFiCallQuality));
  payload_len = sizeof(ims_VoWiFiCallQuality);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_VOWIFI_CALL_QUALITY;

  ims_VoWiFiCallQuality *ptr = (ims_VoWiFiCallQuality*) payload;
  ptr->has_quality = TRUE;
  ptr->quality = quality;
}

/*========================================================
  ims_MsgId_REQUEST_GET_VOWIFI_CALL_QUALITY
========================================================*/

void ril_utf_ims_if::update_default_ims_request_get_vowifi_call_quality()
{
  payload = NULL;
  payload_len = 0;
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_request;
  msg_id = ims_MsgId_REQUEST_GET_VOWIFI_CALL_QUALITY;
}

ims_VoWiFiCallQuality *ril_utf_ims_if::update_default_ims_request_get_vowifi_call_quality_resp()
{
  payload = (ims_VoWiFiCallQuality*) malloc(sizeof(ims_VoWiFiCallQuality));
  payload_len = sizeof(ims_VoWiFiCallQuality);
  memset(payload, 0x00, payload_len);
  payload_type = ril_utf_ims_response;
  msg_id = ims_MsgId_REQUEST_GET_VOWIFI_CALL_QUALITY;

  ims_VoWiFiCallQuality *ptr = (ims_VoWiFiCallQuality*) payload;
  return ptr;
}
void ril_utf_ims_if::set_ims_request_get_vowifi_call_quality_resp_quality(ims_voWiFiQuality quality)
{
  ims_VoWiFiCallQuality *ptr = (ims_VoWiFiCallQuality*) payload;
  ptr->has_quality = TRUE;
  ptr->quality = quality;
}
void ril_utf_ims_if::update_default_ims_unsol_vops_changed(bool value)
{
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_VOPS_CHANGED;

  payload = (void *) malloc(sizeof(ims_VopsInfo));
  payload_len = sizeof(ims_VopsInfo);
  memset(payload, 0x00, payload_len);
  ims_VopsInfo* ptr = (ims_VopsInfo*) payload;
  ptr->has_isVopsEnabled = true;
  ptr->isVopsEnabled = value;
}

/*
  ims_MsgId_UNSOL_PARTICIPANT_STATUS_INFO
*/
ims_ParticipantStatusInfo* ril_utf_ims_if::update_default_ims_unsol_participant_status_info()
{
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_PARTICIPANT_STATUS_INFO;

  payload = (void *) malloc(sizeof(ims_ParticipantStatusInfo));
  payload_len = sizeof(ims_ParticipantStatusInfo);
  memset(payload, 0x00, payload_len);
  ims_ParticipantStatusInfo* ptr = (ims_ParticipantStatusInfo*) payload;
  return ptr;
}
void ril_utf_ims_if::set_ims_unsol_participant_status_info_call_id(uint32_t callId)
{
  ims_ParticipantStatusInfo *ptr = (ims_ParticipantStatusInfo*) payload;
  ptr->has_callId = TRUE;
  ptr->callId = callId;
}
void ril_utf_ims_if::set_ims_unsol_participant_status_info_operation(ims_ConfParticipantOperation operation)
{
  ims_ParticipantStatusInfo *ptr = (ims_ParticipantStatusInfo*) payload;
  ptr->has_operation = TRUE;
  ptr->operation = operation;
}
void ril_utf_ims_if::set_ims_unsol_participant_status_info_sip_status(uint32_t sipStatus)
{
  ims_ParticipantStatusInfo *ptr = (ims_ParticipantStatusInfo*) payload;
  ptr->has_sipStatus = TRUE;
  ptr->sipStatus = sipStatus;
}
void ril_utf_ims_if::set_ims_unsol_participant_status_info_is_ect(bool isEct)
{
  ims_ParticipantStatusInfo *ptr = (ims_ParticipantStatusInfo*) payload;
  ptr->has_isEct = TRUE;
  ptr->isEct = isEct;
}
void ril_utf_ims_if::set_ims_unsol_participant_status_info_participant_uri(char *participantUri)
{
  ims_ParticipantStatusInfo *ptr = (ims_ParticipantStatusInfo*) payload;
  ptr->participantUri.arg = (char*) malloc(strlen(participantUri)+1);
  strlcpy((char *)ptr->participantUri.arg, participantUri, strlen(participantUri)+1);
}


/*
   ims_MsgId_UNSOL_ON_SS
*/
ims_ril_ss_ind_type* ril_utf_ims_if::update_default_ims_unsol_on_ss()
{
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_ON_SS;

  payload = (void *) malloc(sizeof(ims_ril_ss_ind_type));
  payload_len = sizeof(ims_ril_ss_ind_type);
  memset(payload, 0x00, payload_len);
  ims_ril_ss_ind_type* ptr = (ims_ril_ss_ind_type*) payload;
  return ptr;
}

  /*
     ims_MsgId_UNSOL_RESPONSE_REGISTRATION_BLOCK_STATUS
  */
ims_RegistrationBlockStatus* ril_utf_ims_if::update_default_ims_unsol_registration_block_status()
{
  payload_type = ril_utf_ims_indication;
  msg_id = ims_MsgId_UNSOL_RESPONSE_REGISTRATION_BLOCK_STATUS;

  payload = (void *) malloc(sizeof(ims_RegistrationBlockStatus));
  payload_len = sizeof(ims_RegistrationBlockStatus);
  memset(payload, 0x00, payload_len);
  ims_RegistrationBlockStatus* ptr = (ims_RegistrationBlockStatus*) payload;
  return ptr;
}
ims_BlockStatus* ril_utf_ims_if::set_ims_unsol_registration_block_status_BlockStatusOnWwan(
      ims_BlockReasonType block, ims_RegFailureReasonType regFailure, uint32_t regFailureCode)
{
  ims_RegistrationBlockStatus* ptr = (ims_RegistrationBlockStatus*) payload;
  ptr->has_imsBlockStatusOnWwan = TRUE;
  ptr->imsBlockStatusOnWwan.has_blockReason = TRUE;
  ptr->imsBlockStatusOnWwan.blockReason = block;
  ptr->imsBlockStatusOnWwan.has_blockReasonDetails = TRUE;
  ptr->imsBlockStatusOnWwan.blockReasonDetails.has_regFailureReasonType = TRUE;
  ptr->imsBlockStatusOnWwan.blockReasonDetails.regFailureReasonType = regFailure;
  ptr->imsBlockStatusOnWwan.blockReasonDetails.has_RegFailureReason = TRUE;
  ptr->imsBlockStatusOnWwan.blockReasonDetails.RegFailureReason = regFailureCode;
  return &(ptr->imsBlockStatusOnWwan);
}
ims_BlockStatus* ril_utf_ims_if::set_ims_unsol_registration_block_status_BlockStatusOnWlan(
      ims_BlockReasonType block, ims_RegFailureReasonType regFailure, uint32_t regFailureCode)
{
  ims_RegistrationBlockStatus* ptr = (ims_RegistrationBlockStatus*) payload;
  ptr->has_imsBlockStatusOnWlan = TRUE;
  ptr->imsBlockStatusOnWlan.has_blockReason = TRUE;
  ptr->imsBlockStatusOnWlan.blockReason = block;
  ptr->imsBlockStatusOnWlan.has_blockReasonDetails = TRUE;
  ptr->imsBlockStatusOnWlan.blockReasonDetails.has_regFailureReasonType = TRUE;
  ptr->imsBlockStatusOnWlan.blockReasonDetails.regFailureReasonType = regFailure;
  ptr->imsBlockStatusOnWlan.blockReasonDetails.has_RegFailureReason = TRUE;
  ptr->imsBlockStatusOnWlan.blockReasonDetails.RegFailureReason = regFailureCode;
  return &(ptr->imsBlockStatusOnWlan);
}

#endif

/*
   IMS SEND SMS
*/
RIL_IMS_SMS_Message* ril_utf_ims_if::update_default_ims_request_send_sms()
{
   payload_type = ril_utf_ims_request;
   msg_id = ims_MsgId_REQUEST_SEND_SMS;

   payload = (void *)malloc(sizeof(RIL_IMS_SMS_Message));
   assert(payload != nullptr);
   payload_len = sizeof(RIL_IMS_SMS_Message);
   memset(payload, 0x00, payload_len);

   return static_cast<RIL_IMS_SMS_Message*>(payload);
}

void ril_utf_ims_if::set_ims_sms_gsm_payload(const char* smsc, const char* content)
{
    auto ptr = static_cast<RIL_IMS_SMS_Message*>(payload);
    if (ptr->tech == RADIO_TECH_3GPP) {
        ptr->message.gsmMessage = (char**)malloc(2 * sizeof(char*));
        assert(ptr->message.gsmMessage != nullptr);
        if (smsc)
            ptr->message.gsmMessage[0] = strndup(smsc, 100);
        assert(content != nullptr);
        ptr->message.gsmMessage[1] = strndup(content, 100);
    }
}

void ril_utf_ims_if::update_default_ims_send_sms_response()
{
    payload = nullptr;
    payload_len = 0;
    payload_type = ril_utf_ims_response;
    msg_id = ims_MsgId_REQUEST_SEND_SMS;
}

utf_ims_mt_sms_t* ril_utf_ims_if::update_default_ims_mt_sms()
{
    payload_type = ril_utf_ims_indication;
    msg_id = ims_MsgId_UNSOL_INCOMING_IMS_SMS;

    payload = malloc(sizeof(utf_ims_mt_sms_t));
    assert(payload != nullptr);
    payload_len = sizeof(utf_ims_mt_sms_t);
    memset(payload, 0x00, payload_len);

    return static_cast<utf_ims_mt_sms_t*>(payload);
}

void ril_utf_ims_if::set_ims_mt_sms_payload(const std::initializer_list<uint8_t> &list)
{
    size_t length = list.size();
    auto ptr = static_cast<utf_ims_mt_sms_t*>(payload);
    ptr->size = length;
    ptr->pdu = (uint8_t*)malloc(length);
    assert(ptr->pdu != nullptr);
    int idx = 0;
    for (auto &value: list)
    {
        ptr->pdu[idx++] = value;
    }
}

utf_ims_ack_sms_t* ril_utf_ims_if::update_default_ims_ack_sms()
{
   payload_type = ril_utf_ims_request;
   msg_id = ims_MsgId_REQUEST_ACK_SMS;

   payload = (void *)malloc(sizeof(utf_ims_ack_sms_t));
   assert(payload != nullptr);
   payload_len = sizeof(utf_ims_ack_sms_t);
   memset(payload, 0x00, payload_len);

   return static_cast<utf_ims_ack_sms_t*>(payload);
}

ims_StatusForAccessTech *ril_utf_ims_if::create_ims_StatusForAccessTech(
    bool has_networkMode,
    ims_RadioTechType networkMode,
    bool has_status,
    ims_StatusType status,
    bool has_restrictionCause,
    uint32_t restrictionCause,
    bool has_registered,
    ims_Registration registered)
{
  ims_StatusForAccessTech *acc_tech_status = (ims_StatusForAccessTech *)
      qcril_malloc(sizeof(ims_StatusForAccessTech));
  if (acc_tech_status)
  {
    acc_tech_status->has_networkMode = has_networkMode;
    if (has_networkMode)
      acc_tech_status->networkMode = networkMode;
    acc_tech_status->has_status = has_status;
    if (has_status)
      acc_tech_status->status = status;
    acc_tech_status->has_restrictionCause = has_restrictionCause;
    if (has_restrictionCause)
      acc_tech_status->restrictionCause = restrictionCause;
    acc_tech_status->has_registered = has_registered;
    if (has_registered)
      acc_tech_status->registered = registered;
  }
  return acc_tech_status;
}
ims_Info *ril_utf_ims_if::create_ims_Info(
    bool has_isValid,
    bool isValid,
    bool has_type,
    ims_SrvType type,
    bool has_callType,
    ims_CallType callType,
    bool has_status,
    ims_StatusType status,
    void *userdata,
    bool has_restrictCause,
    uint32_t restrictCause,
    ims_StatusForAccessTech **accTechStatus)
{
  ims_Info *ims_info = (ims_Info *)qcril_malloc(sizeof(ims_Info));
  if (ims_info)
  {
    ims_info->has_isValid = has_isValid;
    if (has_isValid)
      ims_info->isValid = isValid;
    ims_info->has_type = has_type;
    if (has_type)
      ims_info->type = type;
    ims_info->has_callType = has_callType;
    if (has_callType)
      ims_info->callType = callType;
    ims_info->has_status = has_status;
    if (has_status)
      ims_info->status = status;
    ims_info->userdata.arg = userdata;
    ims_info->has_restrictCause = has_restrictCause;
    if (has_restrictCause)
      ims_info->restrictCause = restrictCause;
    ims_info->accTechStatus.arg = accTechStatus;
  }
  return ims_info;
}

