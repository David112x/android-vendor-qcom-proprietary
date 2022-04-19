/*===========================================================================

  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#include <cstring>
#include <string.h>
#include <framework/Log.h>
#include "modules/qmi/ModemEndPointFactory.h"
#include "modules/qmi/QmiSetupRequestCallback.h"

#include "legacy/qcril_event_facade.h"
#include "MessageCommon.h"
#include "DataModule.h"
#include "CallInfo.h"
#include "EmbmsCallHandler.h"
#include "CallManager.h"
#include "CallSetupProfile.h"
#include "qdp.h"

#include "request/SetupDataCallRequestMessage.h"
#include "request/DeactivateDataCallRequestMessage.h"
#include "UnSolMessages/HandoverInformationIndMessage.h"
#include "sync/GetCallBringUpCapabilitySyncMessage.h"
#include "event/CallBringupFallbackMessage.h"

#include "DataCommon.h"

using namespace rildata;

unordered_map<int, EmbmsDataCallFailCause_t> EmbmsCallEndReason::dsiCallEndReasonMap =
{
  {WDS_TMGI_ACTIVATE_FAILURE_UNKNOWN_V01,                           EmbmsDataCallFailCause_t::ERROR_UNKNOWN},              /*1*/
  {DSI_EMBMS_TMGI_DEACTIVATED_TYPE_ACT_FAIL_RADIO_CONFIG,           EmbmsDataCallFailCause_t::ERROR_UNKNOWN},              /*1*/
  {DSI_EMBMS_TMGI_DEACTIVATED_TYPE_ACT_FAIL_CHANNEL_UNAVAIL,        EmbmsDataCallFailCause_t::ERROR_UNKNOWN},              /*1*/
  {DSI_EMBMS_TMGI_DEACTIVATED_TYPE_ACT_FAIL_EMBMS_NOT_ENABLED,      EmbmsDataCallFailCause_t::ERROR_UNKNOWN},              /*1*/
  {DSI_EMBMS_TMGI_DEACTIVATED_TYPE_ACT_FAIL_OUT_OF_COVERAGE,        EmbmsDataCallFailCause_t::ERROR_UNKNOWN},              /*1*/
  {WDS_TMGI_ACTIVATE_SUCCESS_DUPLICATE_V01,                         EmbmsDataCallFailCause_t::ERROR_ALREADY_DONE},         /*2*/
  {WDS_TMGI_ACTIVATE_FAILURE_NOT_ALLOWED_V01,                       EmbmsDataCallFailCause_t::ERROR_NOT_ALLOWED},          /*3*/
  {WDS_TMGI_ACTIVATE_FAILURE_MISSING_CONTROL_INFO_V01,              EmbmsDataCallFailCause_t::ERROR_MISSING_CONTROL_INFO}, /*4*/
  {WDS_TMGI_ACTIVATE_FAILURE_MISSING_TMGI_V01,                      EmbmsDataCallFailCause_t::ERROR_MISSING_TMGI},         /*5*/
  {WDS_TMGI_ACTIVATE_FAILURE_MCAST_OOS_V01,                         EmbmsDataCallFailCause_t::ERROR_MCAST_OOC},            /*6*/
  {WDS_TMGI_ACTIVATE_FAILURE_UCAST_OOS_V01,                         EmbmsDataCallFailCause_t::ERROR_UCAST_OOS},            /*7*/
  {WDS_TMGI_ACTIVATE_FAILURE_CAMPED_ON_OTHER_FREQ_V01,              EmbmsDataCallFailCause_t::ERROR_FREQUENCY_CONFLICT},   /*8*/
  {WDS_EMBMS_ECC_TMGI_ACTIVATION_IN_PROGRESS_V01,                   EmbmsDataCallFailCause_t::ERROR_ALREADY_DONE},         /*2*/
  {WDS_EMBMS_EEC_TMGI_DEACTIVATION_IN_PROGRESS_V01,                 EmbmsDataCallFailCause_t::ERROR_UNKNOWN},              /*1*/
  {WDS_EMBMS_EEC_TMGI_NOT_SUPPORTED_V01,                            EmbmsDataCallFailCause_t::ERROR_UNKNOWN},
  {WDS_EMBMS_EEC_TMGI_INVALID_V01,                                  EmbmsDataCallFailCause_t::ERROR_ALREADY_DONE},
  {WDS_EMBMS_EEC_TMGI_DEACTIVATION_IN_PROGRESS_V01,                 EmbmsDataCallFailCause_t::ERROR_UNKNOWN},
  {WDS_TMGI_ACTIVATE_FAILURE_SAI_MISMATCH_V01,                      EmbmsDataCallFailCause_t::ERROR_UNKNOWN},
  {WDS_TMGI_ACTIVATION_FAILURE_MAX_TMGI_ALREADY_ACTIVE_V01,         EmbmsDataCallFailCause_t::ERROR_MAX_TMGI_ALREADY_ACTIVE},
  {WDS_TMGI_ACTIVATE_SUCCESS_IDLE_RADIO_TUNE_V01,                   EmbmsDataCallFailCause_t::SUCCESS_RADIO_TUNE_IN_PROGRESS_UCAST_IDLE},
  {WDS_TMGI_ACTIVATE_SUCCESS_CONN_RADIO_TUNE_V01,                   EmbmsDataCallFailCause_t::SUCCESS_RADIO_TUNE_IN_PROGRESS_UCAST_CONNECTED}
};

EmbmsDataCallFailCause_t EmbmsCallEndReason::getFailCause(int dsiReason) {
  if (dsiCallEndReasonMap.find(dsiReason)== dsiCallEndReasonMap.end()) {
     Log::getInstance().d("[EmbmsCallEndReason]: getCallEndReason code unknown");
     return EmbmsDataCallFailCause_t::ERROR_UNKNOWN;
  }
  return dsiCallEndReasonMap[dsiReason];
}

EmbmsCallHandler::EmbmsCallHandler(LocalLogBuffer& setLogBuffer): logBuffer(setLogBuffer) {
  Log::getInstance().d("[EmbmsCallHandler]: EmbmsCallHandler");
  dsiHandle = 0;
}

EmbmsCallHandler::~EmbmsCallHandler() {
  Log::getInstance().d("[EmbmsCallHandler]: ~EmbmsCallHandler");
  txnList.clear();
}

void EmbmsRilResponse::sendRilResponse() {
  Log::getInstance().d("[EmbmsRilResponse]: sendRilResponse");
  qcril_request_resp_params_type resp;

  memset(&resp, 0, sizeof(qcril_request_resp_params_type));
  resp.instance_id        = mInstanceId;
  resp.t                  = mRiltoken;
  resp.request_id         = mRequestId;
  resp.request_id_android = mRequestIdAndroid;
  resp.ril_err_no         = mRilResponse;
  resp.logstr             = NULL;
  resp.rild_sock_oem_req  = 0;
  resp.resp_pkt           = mRespPacket;
  resp.resp_len           = mRespPktLen;
  qcril_send_request_response(&resp);
}

void EmbmsRilResponse::sendActivateTmgiResponse(embms_tmgi_type_v01 *tmgi_info, int32_t dbg_trace_id,
                                                uint8_t call_id, EmbmsDataCallFailCause_t failCause) {
  Log::getInstance().d("[EmbmsRilResponse]: sendActivateTmgiResponse");
  mRespPktLen = sizeof(embms_activate_tmgi_resp_msg_v01);
  mRespPacket = (embms_activate_tmgi_resp_msg_v01 *)malloc(mRespPktLen);
  embms_activate_tmgi_resp_msg_v01 *pRespPacket= (embms_activate_tmgi_resp_msg_v01 *)mRespPacket;

  if (nullptr != pRespPacket) {
      memset(pRespPacket, 0, mRespPktLen);
      pRespPacket->call_id_valid      = TRUE;
      pRespPacket->call_id            = call_id;;
      pRespPacket->resp_code          = (int32_t)failCause;
      pRespPacket->dbg_trace_id       = dbg_trace_id;
      pRespPacket->tmgi_info_valid    = TRUE;
      pRespPacket->tmgi_info.tmgi_len = TMGI_LENGTH_MAX_V01;
      memcpy(&pRespPacket->tmgi_info.tmgi[0],
             &tmgi_info->tmgi[0],
             TMGI_LENGTH_MAX_V01);
  }
  sendRilResponse();
}

void EmbmsRilResponse::sendDeactivateTmgiResponse(embms_tmgi_type_v01 *tmgi_info, int32_t dbg_trace_id,
                                                uint8_t call_id, EmbmsDataCallFailCause_t failCause) {
  Log::getInstance().d("[EmbmsRilResponse]: sendDeactivateTmgiResponse");
  mRespPktLen = sizeof(embms_deactivate_tmgi_resp_msg_v01);
  mRespPacket = (embms_deactivate_tmgi_resp_msg_v01 *)malloc(mRespPktLen);
  embms_deactivate_tmgi_resp_msg_v01 *pRespPacket = (embms_deactivate_tmgi_resp_msg_v01 *)mRespPacket;

  if (nullptr != pRespPacket) {
      memset(pRespPacket, 0, mRespPktLen);
      pRespPacket->call_id_valid      = TRUE;
      pRespPacket->call_id            = call_id;;
      pRespPacket->resp_code          = (int32_t)failCause;
      pRespPacket->dbg_trace_id       = dbg_trace_id;
      pRespPacket->tmgi_info_valid    = TRUE;
      pRespPacket->tmgi_info.tmgi_len = TMGI_LENGTH_MAX_V01;
      memcpy(&pRespPacket->tmgi_info.tmgi[0],
             &tmgi_info->tmgi[0],
             TMGI_LENGTH_MAX_V01);
  }
  sendRilResponse();
}

void EmbmsRilResponse::sendActivateDeactivateTmgiResponse(embms_tmgi_type_v01 *act_tmgi_info, embms_tmgi_type_v01 *deact_tmgi_info,
                                                          int32_t dbg_trace_id, uint8_t call_id, EmbmsDataCallFailCause_t failCause) {
  Log::getInstance().d("[EmbmsRilResponse]: sendActivateDeactivateTmgiResponse");
  mRespPktLen = sizeof(embms_activate_deactivate_tmgi_resp_msg_v01);
  mRespPacket = (embms_activate_deactivate_tmgi_resp_msg_v01 *)malloc(mRespPktLen);
  embms_activate_deactivate_tmgi_resp_msg_v01 *pRespPacket= (embms_activate_deactivate_tmgi_resp_msg_v01 *)mRespPacket;

  if (nullptr != pRespPacket) {
      memset(pRespPacket, 0, mRespPktLen);
      pRespPacket->call_id_valid      = TRUE;
      pRespPacket->call_id            = call_id;;
      pRespPacket->act_resp_code      = (uint16_t)failCause;
      pRespPacket->deact_resp_code    = (uint16_t)failCause;
      pRespPacket->dbg_trace_id       = dbg_trace_id;

      pRespPacket->act_tmgi_info_valid    = TRUE;
      pRespPacket->act_tmgi_info.tmgi_len = TMGI_LENGTH_MAX_V01;
      memcpy(&pRespPacket->act_tmgi_info.tmgi[0],
             &act_tmgi_info[0],
             TMGI_LENGTH_MAX_V01);

      pRespPacket->deact_tmgi_info_valid    = TRUE;
      pRespPacket->deact_tmgi_info.tmgi_len = TMGI_LENGTH_MAX_V01;
      memcpy(&pRespPacket->deact_tmgi_info.tmgi[0],
             &deact_tmgi_info[0],
             TMGI_LENGTH_MAX_V01);
  }
  sendRilResponse();
}

void EmbmsRilResponse::sendContentDescUpdateResponse(int32_t dbg_trace_id, EmbmsDataCallFailCause_t failCause) {
  Log::getInstance().d("[EmbmsRilResponse]: sendContentDescUpdateResponse");
  mRespPktLen = sizeof(embms_update_content_desc_resp_msg_v01);
  mRespPacket = (embms_update_content_desc_resp_msg_v01 *)malloc(mRespPktLen);
  embms_update_content_desc_resp_msg_v01 *pRespPacket = (embms_update_content_desc_resp_msg_v01 *)mRespPacket;

  if (nullptr != pRespPacket) {
      memset(pRespPacket, 0, mRespPktLen);
      pRespPacket->resp_code          = (int32_t)failCause;
      pRespPacket->dbg_trace_id       = dbg_trace_id;
  }
  sendRilResponse();
}

void EmbmsRilResponse::sendGetActiveTmgiResponse(int32_t dbg_trace_id, wds_embms_tmgi_list_query_resp_msg_v01 *embms_tmgi,
                                                 EmbmsDataCallFailCause_t failCause) {
  Log::getInstance().d("[EmbmsRilResponse]: sendGetActiveTmgiResponse");
  mRespPktLen = sizeof(embms_get_active_tmgi_resp_msg_v01);
  mRespPacket = (embms_get_active_tmgi_resp_msg_v01 *)malloc(mRespPktLen);
  embms_get_active_tmgi_resp_msg_v01 *pRespPacket= (embms_get_active_tmgi_resp_msg_v01 *)mRespPacket;

  if (nullptr != pRespPacket) {
      memset(pRespPacket, 0, mRespPktLen);
      pRespPacket->resp_code          = (int32_t)failCause;
      pRespPacket->dbg_trace_id       = dbg_trace_id;
      if (EmbmsDataCallFailCause_t::ERROR_NONE == failCause) {
        pRespPacket->tmgi_info_valid = TRUE;

        /* validate length of tmgi_list */
        pRespPacket->tmgi_info_len =
           (embms_tmgi->tmgi_list.tmgi_list_len > NUMBER_MAX_V01) ?
           NUMBER_MAX_V01 : embms_tmgi->tmgi_list.tmgi_list_len;

        for (int j = 0; j < pRespPacket->tmgi_info_len; j++)
        {
          pRespPacket->tmgi_info[j].tmgi_len = TMGI_LENGTH_MAX_V01;
          memcpy(&(pRespPacket->tmgi_info[j].tmgi[0]),
                 &(embms_tmgi->tmgi_list.tmgi_list[j].tmgi[0]),
                 TMGI_LENGTH_MAX_V01);
        }
      } else {
        pRespPacket->tmgi_info_valid = FALSE;
      }
  }
  sendRilResponse();
}

void EmbmsRilResponse::sendInterestedListResponse(int32_t dbg_trace_id) {
  Log::getInstance().d("[EmbmsRilResponse]: sendInterestedList");
  mRespPktLen = sizeof(embms_get_interested_tmgi_list_resp_ack_msg_v01);
  mRespPacket = (embms_get_interested_tmgi_list_resp_ack_msg_v01 *)malloc(mRespPktLen);
  embms_get_interested_tmgi_list_resp_ack_msg_v01 *pRespPacket = (embms_get_interested_tmgi_list_resp_ack_msg_v01 *)mRespPacket;

  if (nullptr != pRespPacket) {
     memset(pRespPacket, 0, mRespPktLen);
     pRespPacket->dbg_trace_id = dbg_trace_id;
  }
  sendRilResponse();
}

void EmbmsRilResponse::sendGetAvailableTmgiResponse(int32_t dbg_trace_id, wds_embms_tmgi_list_query_resp_msg_v01 *embms_tmgi,
                                    EmbmsDataCallFailCause_t failCause) {
  Log::getInstance().d("[EmbmsRilResponse]: sendGetAvailableTmgiResponse");
  mRespPktLen = sizeof(embms_get_available_tmgi_resp_msg_v01);
  mRespPacket = (embms_get_available_tmgi_resp_msg_v01 *)malloc(mRespPktLen);
  embms_get_available_tmgi_resp_msg_v01 *pRespPacket = (embms_get_available_tmgi_resp_msg_v01 *)mRespPacket;

  if(nullptr != pRespPacket)
  {
    memset(pRespPacket, 0, mRespPktLen);
    pRespPacket->resp_code = (int32_t)failCause;
    pRespPacket->dbg_trace_id = dbg_trace_id;
    if (EmbmsDataCallFailCause_t::ERROR_NONE == failCause) {
      pRespPacket->tmgi_info_valid = TRUE;

      /* validate length of tmgi_list */
      pRespPacket->tmgi_info_len = (embms_tmgi->tmgi_list.tmgi_list_len > NUMBER_MAX_V01) ?
                                   NUMBER_MAX_V01 : embms_tmgi->tmgi_list.tmgi_list_len;

      for (int j = 0; j < pRespPacket->tmgi_info_len; j++)
      {
        pRespPacket->tmgi_info[j].tmgi_len = TMGI_LENGTH_MAX_V01;
        memcpy(&(pRespPacket->tmgi_info[j].tmgi[0]),
             &(embms_tmgi->tmgi_list.tmgi_list[j].tmgi[0]),
             TMGI_LENGTH_MAX_V01);
      }
    } else {
      pRespPacket->tmgi_info_valid = FALSE;
    }
  }
  sendRilResponse();
}

void EmbmsCallHandler::activateTmgi(qcril_request_params_type *params_ptr) {
  Log::getInstance().d("[EmbmsCallHandler]: activateTmgi");
  embms_activate_tmgi_req_msg_v01       *activate_req;
  int tmgiDeactivateReason;
  EmbmsDataCallFailCause_t failCause = EmbmsDataCallFailCause_t::ERROR_UNKNOWN;

  if (!params_ptr) {
     Log::getInstance().d("Invalid param passed");
     return;
  }

  std::shared_ptr<ril_data::tmgiRequestType> tmgiInfo = std::make_shared<ril_data::tmgiRequestType>();

  if (tmgiInfo == nullptr) {
    Log::getInstance().d("Unable to allocate tmgiInfo memory block");
    return;
  }

  activate_req = (embms_activate_tmgi_req_msg_v01  *)params_ptr->data;

  tmgiInfo->cId = activate_req->call_id;
  tmgiInfo->instance_id = params_ptr->instance_id;
  tmgiInfo->pend_req = QCRIL_EVT_HOOK_EMBMS_ACTIVATE_TMGI;
  tmgiInfo->event_id_android = params_ptr->event_id_android;
  tmgiInfo->pend_tok = params_ptr->t;

  memcpy((char *)(&tmgiInfo->tmgi.tmgi),
         (char *)(&activate_req->tmgi_info.tmgi),
         EMBMS_TMGI_LEN );

  txnList.push_back(tmgiInfo);

  if (DSI_SUCCESS != dsiTmgiActivate(dsiHandle,
                                     (char *)(&activate_req->tmgi_info.tmgi[0]),
                                     (unsigned int*)&activate_req->earfcnlist[0],
                                     activate_req->earfcnlist_len,
                                     (unsigned long)activate_req->preemption_priority,
                                     (unsigned int*)&activate_req->saiList[0],
                                     (unsigned char)activate_req->saiList_len,
                                     activate_req->dbg_trace_id)) {
     Log::getInstance().d("unable to activate TMGI");

     dsi_embms_tmgi_status_field_type status_field = CHECK_ACTIVATE_STATUS;
     if ( DSI_SUCCESS == dsiGetTmgiDeactivateReason(dsiHandle, status_field,
                                                &tmgiDeactivateReason) )
     {
       Log::getInstance().d("TMGI activate failure reason QMI_WDS"+std::to_string(tmgiDeactivateReason));
       failCause = EmbmsCallEndReason::getFailCause(tmgiDeactivateReason);
     }

     std::shared_ptr<EmbmsRilResponse> rilResponse = std::make_shared<EmbmsRilResponse>(params_ptr->event_id, params_ptr->instance_id,
                                                      params_ptr->event_id_android, params_ptr->t, RIL_E_GENERIC_FAILURE);
     if(rilResponse != nullptr)
     {
       rilResponse->sendActivateTmgiResponse(&activate_req->tmgi_info, activate_req->dbg_trace_id, activate_req->call_id,
                                           failCause);
     }
  }
}

void EmbmsCallHandler::deactivateTmgi(qcril_request_params_type *params_ptr) {
  Log::getInstance().d("[EmbmsCallHandler]: deactivateTmgi");
  embms_deactivate_tmgi_req_msg_v01       *deactivate_req;
  int tmgiDeactivateReason;
  EmbmsDataCallFailCause_t failCause = EmbmsDataCallFailCause_t::ERROR_UNKNOWN;

  if (params_ptr == nullptr) {
     Log::getInstance().d("Invalid param passed");
     return;
  }

  std::shared_ptr<ril_data::tmgiRequestType> tmgiInfo = std::make_shared<ril_data::tmgiRequestType>();

  if (tmgiInfo == nullptr) {
    Log::getInstance().d("Unable to allocate tmgiInfo memory block");
    return;
  }

  deactivate_req = (embms_deactivate_tmgi_req_msg_v01 *)params_ptr->data;

  tmgiInfo->cId = deactivate_req->call_id;
  tmgiInfo->instance_id = params_ptr->instance_id;
  tmgiInfo->event_id_android = params_ptr->event_id_android;
  tmgiInfo->pend_req = QCRIL_EVT_HOOK_EMBMS_DEACTIVATE_TMGI;
  tmgiInfo->pend_tok = params_ptr->t;

  memcpy((char *)(&tmgiInfo->deact_tmgi.tmgi),
         (char *)(&deactivate_req->tmgi_info.tmgi),
         EMBMS_TMGI_LEN );

  txnList.push_back(tmgiInfo);

  if (DSI_SUCCESS != dsiTmgiDeactivate(dsiHandle,
                                       (char *)(&deactivate_req->tmgi_info.tmgi[0]),
                                       deactivate_req->dbg_trace_id)) {
     Log::getInstance().d("unable to deactivate TMGI");

     dsi_embms_tmgi_status_field_type status_field = CHECK_DEACTIVATE_STATUS;
     if ( DSI_SUCCESS == dsiGetTmgiDeactivateReason(dsiHandle, status_field,
                                                &tmgiDeactivateReason) )
     {
       Log::getInstance().d("TMGI activate failure reason QMI_WDS:"+ std::to_string(tmgiDeactivateReason));
       failCause = EmbmsCallEndReason::getFailCause(tmgiDeactivateReason);
     }

     std::shared_ptr<EmbmsRilResponse> rilResponse = std::make_shared<EmbmsRilResponse>(params_ptr->event_id, params_ptr->instance_id,
                                                          params_ptr->event_id_android, params_ptr->t, RIL_E_GENERIC_FAILURE);
     if(rilResponse != nullptr)
     {
       rilResponse->sendDeactivateTmgiResponse(&deactivate_req->tmgi_info, deactivate_req->dbg_trace_id, deactivate_req->call_id,
                                             failCause);
     }
  }
}

void EmbmsCallHandler::activateDeactivateTmgi(qcril_request_params_type *params_ptr) {
  Log::getInstance().d("[EmbmsCallHandler]: activateDeactivateTmgi");
  embms_activate_deactivate_tmgi_req_msg_v01  *act_deactivate_req;
  int tmgiDeactivateReason;
  EmbmsDataCallFailCause_t failCause = EmbmsDataCallFailCause_t::ERROR_UNKNOWN;

  if (params_ptr == nullptr) {
     Log::getInstance().d("Invalid param passed");
     return;
  }

  std::shared_ptr<ril_data::tmgiRequestType> tmgiInfo = std::make_shared<ril_data::tmgiRequestType>();

  if (tmgiInfo == nullptr) {
    Log::getInstance().d("Unable to allocate tmgiInfo memory block");
    return;
  }

  act_deactivate_req = (embms_activate_deactivate_tmgi_req_msg_v01 *)params_ptr->data;

  tmgiInfo->cId = act_deactivate_req->call_id;
  tmgiInfo->instance_id = params_ptr->instance_id;
  tmgiInfo->event_id_android = params_ptr->event_id_android;
  tmgiInfo->pend_req = QCRIL_EVT_HOOK_EMBMS_ACTIVATE_DEACTIVATE_TMGI;
  tmgiInfo->pend_tok = params_ptr->t;

  memcpy((char *)(&tmgiInfo->tmgi.tmgi),
         (char *)(&act_deactivate_req->act_tmgi_info.tmgi),
         EMBMS_TMGI_LEN);
  memcpy((char *)(&tmgiInfo->deact_tmgi.tmgi),
         (char *)(&act_deactivate_req->deact_tmgi_info.tmgi),
         EMBMS_TMGI_LEN);

  txnList.push_back(tmgiInfo);

  if (DSI_SUCCESS != dsiTmgiActivateDeactivate(dsiHandle,
                                     (char *)(&act_deactivate_req->act_tmgi_info.tmgi[0]),
                                     (unsigned int*)&act_deactivate_req->earfcnlist[0],
                                     act_deactivate_req->earfcnlist_len,
                                     (unsigned long)act_deactivate_req->preemption_priority,
                                     (char *)(&act_deactivate_req->deact_tmgi_info.tmgi[0]),
                                     (unsigned int*)&act_deactivate_req->saiList[0],
                                     (unsigned char)act_deactivate_req->saiList_len,
                                     act_deactivate_req->dbg_trace_id)) {
     Log::getInstance().d("unable to activate TMGI");

     dsi_embms_tmgi_status_field_type status_field = CHECK_ACTIVATE_DEACTIVATE_STATUS;
     if ( DSI_SUCCESS == dsiGetTmgiDeactivateReason(dsiHandle, status_field,
                                                &tmgiDeactivateReason) )
     {
       Log::getInstance().d("TMGI activate failure reason QMI_WDS:" + std::to_string(tmgiDeactivateReason));
       failCause = EmbmsCallEndReason::getFailCause(tmgiDeactivateReason);
     }

    std::shared_ptr<EmbmsRilResponse> rilResponse = std::make_shared<EmbmsRilResponse>(params_ptr->event_id, params_ptr->instance_id,
                                                         params_ptr->event_id_android, params_ptr->t, RIL_E_GENERIC_FAILURE);
    if(rilResponse != nullptr)
    {
      rilResponse->sendActivateDeactivateTmgiResponse(&act_deactivate_req->act_tmgi_info, &act_deactivate_req->deact_tmgi_info,
                                                  act_deactivate_req->dbg_trace_id, act_deactivate_req->call_id, failCause);
    }
  }
}

void EmbmsCallHandler::contentDescUpdate(qcril_request_params_type *params_ptr) {
  Log::getInstance().d("[EmbmsCallHandler]: contentDescUpdate");
  embms_update_content_desc_req_msg_v01 *get_update_req = NULL;
  RIL_Errno rilResponse = RIL_E_SUCCESS;
  EmbmsDataCallFailCause_t failCause = EmbmsDataCallFailCause_t::ERROR_NONE;

  if (params_ptr == nullptr) {
     Log::getInstance().d("Invalid param passed");
     return;
  }

  get_update_req = (embms_update_content_desc_req_msg_v01 *)params_ptr->data;

  if (DSI_SUCCESS != dsiContentDescUpdate(dsiHandle,
                                          (char *)(&get_update_req->tmgi_info.tmgi[0]),
                                          (unsigned char)get_update_req->content_desc_valid,
                                          get_update_req->content_desc_len,
                                          (dsi_embms_content_desc_type *)get_update_req->content_desc,
                                          get_update_req->dbg_trace_id)) {
     Log::getInstance().d("unable to deactivate TMGI");
     failCause = EmbmsDataCallFailCause_t::ERROR_UNKNOWN;
     rilResponse = RIL_E_GENERIC_FAILURE;
  }

  std::shared_ptr<EmbmsRilResponse> pRilResponse = std::make_shared<EmbmsRilResponse>(params_ptr->event_id, params_ptr->instance_id,
                                                        params_ptr->event_id_android, params_ptr->t, rilResponse);
  if(pRilResponse != nullptr)
  {
    pRilResponse->sendContentDescUpdateResponse(get_update_req->dbg_trace_id, failCause);
  }
}

void EmbmsCallHandler::getActiveTmgi(qcril_request_params_type *params_ptr) {
  Log::getInstance().d("[EmbmsCallHandler]: getActiveTmgi");
  embms_get_active_tmgi_req_msg_v01 *get_active_req = NULL;
  RIL_Errno rilResponse = RIL_E_SUCCESS;
  wds_embms_tmgi_list_query_resp_msg_v01 embms_tmgi;
  EmbmsDataCallFailCause_t failCause = EmbmsDataCallFailCause_t::ERROR_NONE;

  memset(&embms_tmgi, 0, sizeof(wds_embms_tmgi_list_query_resp_msg_v01));
  if (params_ptr == nullptr) {
     Log::getInstance().d("Invalid param passed");
     return;
  }

  get_active_req = (embms_get_active_tmgi_req_msg_v01 *)params_ptr->data;

  if (DSI_SUCCESS !=  dsiGetEmbmsTmgiListquery(dsiHandle,
                                                WDS_EMBMS_TMGI_LIST_ACTIVE_V01,
                                                &embms_tmgi,
                                                get_active_req->dbg_trace_id)) {
     Log::getInstance().d("unable to deactivate TMGI");
     failCause = EmbmsDataCallFailCause_t::ERROR_UNKNOWN;
     rilResponse = RIL_E_GENERIC_FAILURE;
  }

  std::shared_ptr<EmbmsRilResponse> pRilResponse = std::make_shared<EmbmsRilResponse>(params_ptr->event_id, params_ptr->instance_id,
                                                        params_ptr->event_id_android, params_ptr->t, rilResponse);
  if(pRilResponse != nullptr)
  {
    pRilResponse->sendGetActiveTmgiResponse(get_active_req->dbg_trace_id, &embms_tmgi,
                                            failCause);
  }
}

void EmbmsCallHandler::getAvailableTmgi(qcril_request_params_type *params_ptr) {
  Log::getInstance().d("[EmbmsCallHandler]: getAvailableTmgi");
  embms_get_available_tmgi_req_msg_v01 *get_available_req = NULL;
  RIL_Errno rilResponse = RIL_E_SUCCESS;
  wds_embms_tmgi_list_query_resp_msg_v01 embms_tmgi;
  EmbmsDataCallFailCause_t failCause = EmbmsDataCallFailCause_t::ERROR_NONE;

  memset(&embms_tmgi, 0, sizeof(wds_embms_tmgi_list_query_resp_msg_v01));
  if (params_ptr == nullptr) {
     Log::getInstance().d("Invalid param passed");
     return;
  }

  get_available_req = (embms_get_available_tmgi_req_msg_v01 *)params_ptr->data;

  if (DSI_SUCCESS !=  dsiGetEmbmsTmgiListquery(dsiHandle,
                                                WDS_EMBMS_TMGI_LIST_AVAILABLE_V01,
                                                &embms_tmgi,
                                                get_available_req->dbg_trace_id)) {
     Log::getInstance().d("unable to deactivate TMGI");
     failCause = EmbmsDataCallFailCause_t::ERROR_UNKNOWN;
     rilResponse = RIL_E_GENERIC_FAILURE;
  }

  std::shared_ptr<EmbmsRilResponse> pRilResponse = std::make_shared<EmbmsRilResponse>(params_ptr->event_id, params_ptr->instance_id,
                                                        params_ptr->event_id_android, params_ptr->t, rilResponse);
  if(pRilResponse != nullptr)
  {
    pRilResponse->sendGetAvailableTmgiResponse(get_available_req->dbg_trace_id, &embms_tmgi,
                                               failCause);
  }
}

void EmbmsCallHandler::sendInterestedList(qcril_request_params_type *params_ptr) {
  Log::getInstance().d("[EmbmsCallHandler]: sendInterestedList");
  embms_get_interested_tmgi_list_resp_msg_v01 *interest_list_resp = NULL;
  RIL_Errno rilResponse = RIL_E_SUCCESS;
  wds_embms_tmgi_list_query_resp_msg_v01 embms_tmgi;
  EmbmsDataCallFailCause_t failCause = EmbmsDataCallFailCause_t::ERROR_NONE;
  char **tmgi_ptr_list = NULL;

  memset(&embms_tmgi, 0, sizeof(wds_embms_tmgi_list_query_resp_msg_v01));
  if (params_ptr == nullptr) {
     Log::getInstance().d("Invalid param passed");
     return;
  }

  interest_list_resp = (embms_get_interested_tmgi_list_resp_msg_v01 *)params_ptr->data;

  if(interest_list_resp->tmgi_info_len > 0)
  {
    tmgi_ptr_list = (char **)malloc(interest_list_resp->tmgi_info_len * sizeof(char *));
    if(tmgi_ptr_list == NULL)
    {
      Log::getInstance().d( "failed to allocate memory" );
      return;
    }

    /* Send the tmgi list as a ptr to an array of tmgi ptrs */
    memset(tmgi_ptr_list, 0, interest_list_resp->tmgi_info_len * sizeof(char *));
    for(int j=0; j<interest_list_resp->tmgi_info_len; j++)
    {
      tmgi_ptr_list[j] = (char *)&interest_list_resp->tmgi_info[j].tmgi[0];
    }
  }

  if (DSI_SUCCESS != dsiEmbmsSvcInterestList(dsiHandle,
                                             tmgi_ptr_list,
                                             interest_list_resp->tmgi_info_len,
                                             interest_list_resp->dbg_trace_id)) {
     Log::getInstance().d("unable to deactivate TMGI");
     failCause = EmbmsDataCallFailCause_t::ERROR_UNKNOWN;
     rilResponse = RIL_E_GENERIC_FAILURE;
  }

  std::shared_ptr<EmbmsRilResponse> pRilResponse = std::make_shared<EmbmsRilResponse>(params_ptr->event_id, params_ptr->instance_id,
                                                        params_ptr->event_id_android, params_ptr->t, rilResponse);
  if(pRilResponse != nullptr)
  {
    pRilResponse->sendInterestedListResponse(interest_list_resp->dbg_trace_id);
  }
}

int EmbmsCallHandler::isEmbmsActive(void) {
  Log::getInstance().d("[EmbmsCallHandler]: isEmbmsActive");
  dsi_call_tech_type callTech;

  if ((DSI_SUCCESS == dsi_get_call_tech(dsiHandle, &callTech)) &&
      (callTech == DSI_EXT_TECH_EMBMS)) {
        return TRUE;
  }
  return FALSE;
}

void EmbmsCallHandler::processTmgiActivatedEvent(dsi_embms_tmgi_info_type *embms_payload) {
  Log::getInstance().d("[EmbmsCallHandler]: processTmgiActivatedEvent");
  std::list<std::shared_ptr<ril_data::tmgiRequestType>>::iterator it = txnList.begin();

  while(it != txnList.end()) {
    std::shared_ptr<ril_data::tmgiRequestType> req = *it;

    if (req->pend_req == QCRIL_EVT_HOOK_EMBMS_ACTIVATE_TMGI) {
       if (memcmp(req->tmgi.tmgi,
                 embms_payload->embms_act_info.tmgi.tmgi,
                 sizeof(req->tmgi.tmgi)) == 0) {
          Log::getInstance().d("ACTIVATED EVENT, TMGI match occured");
          std::shared_ptr<EmbmsRilResponse> pRilResponse = std::make_shared<EmbmsRilResponse>( req->pend_req , req->instance_id,
                                                                 req-> event_id_android, req->pend_tok, RIL_E_SUCCESS);
          embms_tmgi_type_v01 tmgi_val;
          memcpy(&tmgi_val.tmgi, embms_payload->embms_act_info.tmgi.tmgi, EMBMS_TMGI_LEN);
          tmgi_val.tmgi_len = sizeof(tmgi_val.tmgi);
          if(pRilResponse != nullptr)
          {
            pRilResponse->sendActivateTmgiResponse(&tmgi_val, embms_payload->embms_act_info.tranx_id,
                                                     req->cId, EmbmsDataCallFailCause_t::ERROR_NONE);
          }
          it = txnList.erase(it);
          return;
       } else {
         it++;
       }
    } else if (req->pend_req == QCRIL_EVT_HOOK_EMBMS_DEACTIVATE_TMGI) {
        if ((memcmp(req->deact_tmgi.tmgi,
                   embms_payload->embms_deact_info.tmgi.tmgi,
                   EMBMS_TMGI_LEN)) == 0) {
          Log::getInstance().d( "Activated received when expecting a "
                           "deactivated indication, Dropping Indication");
          return;
        } else {
          it++;
        }
    } else {
      it++;
    }
  }
}


void EmbmsCallHandler::processTmgiDeactivatedEvent(dsi_embms_tmgi_info_type *embms_payload) {
  Log::getInstance().d("[EmbmsCallHandler]: processTmgiDeactivatedEvent");
  std::list<std::shared_ptr<ril_data::tmgiRequestType>>::iterator it = txnList.begin();

  while(it != txnList.end()) {
    std::shared_ptr<ril_data::tmgiRequestType> req = *it;

    if (req->pend_req == QCRIL_EVT_HOOK_EMBMS_DEACTIVATE_TMGI) {
       Log::getInstance().d("DEACTIVATED EVENT, TMGI match occured");
       if ((memcmp(req->deact_tmgi.tmgi,
                   embms_payload->embms_deact_info.tmgi.tmgi,
                   EMBMS_TMGI_LEN)) == 0) {
          std::shared_ptr<EmbmsRilResponse> pRilResponse = std::make_shared<EmbmsRilResponse>(req->pend_req , req->instance_id,
                                                                req->event_id_android, req->pend_tok, RIL_E_SUCCESS);
          if(pRilResponse != nullptr)
          {
            Log::getInstance().d("sending response");
            pRilResponse->sendDeactivateTmgiResponse(&req->deact_tmgi, embms_payload->embms_deact_info.tranx_id,
                                                     req->cId, EmbmsDataCallFailCause_t::ERROR_NONE);
          }
          it = txnList.erase(it);
          return;
       } else {
         it++;
       }
    } else if (req->pend_req == QCRIL_EVT_HOOK_EMBMS_ACTIVATE_TMGI) {
        Log::getInstance().d("Found a Tmgi Activated Event");
        if ((memcmp(req->tmgi.tmgi,
                   embms_payload->embms_act_info.tmgi.tmgi,
                   EMBMS_TMGI_LEN)) == 0) {
          EmbmsDataCallFailCause_t failCause = EmbmsCallEndReason::getFailCause(embms_payload->embms_act_info.activate_status);
          std::shared_ptr<EmbmsRilResponse> pRilResponse = std::make_shared<EmbmsRilResponse>(req->pend_req , req->instance_id,
                                                                req->event_id_android, req->pend_tok, RIL_E_GENERIC_FAILURE);
          if(pRilResponse != nullptr)
          {
            Log::getInstance().d("sending response");
            pRilResponse->sendActivateTmgiResponse(&req->tmgi, embms_payload->embms_act_info.tranx_id,
                                                     req->cId, failCause);
          }
          it = txnList.erase(it);
          return;
        } else {
          it++;
        }
    } else {
      it++;
    }
  }
}

void EmbmsCallHandler::processTmgiActivatedDeactivatedEvent(dsi_embms_tmgi_info_type *embms_payload) {
   Log::getInstance().d("[EmbmsCallHandler]: processTmgiActivatedDeactivatedEvent");
  std::list<std::shared_ptr<ril_data::tmgiRequestType>>::iterator it = txnList.begin();

  while(it != txnList.end()) {
    std::shared_ptr<ril_data::tmgiRequestType> req = *it;

    if (req->pend_req == QCRIL_EVT_HOOK_EMBMS_ACTIVATE_DEACTIVATE_TMGI) {
       Log::getInstance().d("ACTIVATED_DEACTIVATED EVENT, TMGI match occured");
       EmbmsDataCallFailCause_t failCause = EmbmsDataCallFailCause_t::ERROR_UNKNOWN;
       if ( ((memcmp(req->deact_tmgi.tmgi,
                     embms_payload->embms_act_deact_info.deact_tmgi.tmgi,
                     EMBMS_TMGI_LEN)) == 0) &&
              ((memcmp(req->tmgi.tmgi,
                     embms_payload->embms_act_deact_info.act_tmgi.tmgi,
                     EMBMS_TMGI_LEN)) == 0) ) {
           RIL_Errno err_no;
           if((WDS_TMGI_ACTIVATE_SUCCESS_V01 ==
               embms_payload->embms_act_deact_info.act_status) &&
              (WDS_TMGI_DEACTIVATE_SUCCESS_V01 ==
               embms_payload->embms_act_deact_info.deact_status)
              )
           {
             err_no = RIL_E_SUCCESS;
           }
           else
           {
             failCause = EmbmsCallEndReason::getFailCause(embms_payload->embms_act_deact_info.act_status);
             err_no = RIL_E_GENERIC_FAILURE;
           }

           std::shared_ptr<EmbmsRilResponse> pRilResponse = std::make_shared<EmbmsRilResponse>(req->pend_req , req->instance_id,
                                                                req->event_id_android, req->pend_tok, err_no);

           if(pRilResponse != nullptr)
           {
             pRilResponse->sendActivateDeactivateTmgiResponse(&req->tmgi, &req->deact_tmgi,
                               embms_payload->embms_deact_info.tranx_id, req->cId, failCause);
           }
           it = txnList.erase(it);
           return;
       } else {
          it++;
       }
    } else {
      it ++;
    }
  }
}


void EmbmsCallHandler::processEvents(dsi_net_evt_t evt, dsi_embms_tmgi_info_type *embmsPayload) {
   Log::getInstance().d("[EmbmsCallHandler]: processEvents");
  embms_unsol_active_tmgi_ind_msg_v01 active_indication;

   switch(evt)
   {
     case DSI_NET_SAI_LIST_CHANGED:
     {
       embms_unsol_sai_ind_msg_v01  sai_indication;
       unsigned int list_index;
       unsigned int freq_index;
       unsigned int camped_index;
       unsigned int avail_index;
       unsigned short  list_len;
       unsigned char   sai_size = 0;
       unsigned short  camped_sai_list_size = 0;
       unsigned short  available_sai_list_size = 0;

       Log::getInstance().d(">>>DSI_NET_SAI_LIST_CHANGED: START>>>");

       list_len = embmsPayload->embms_sai_info.freq_sai_list_len;
       if(embmsPayload->embms_sai_info.freq_sai_list_valid != TRUE)
       {
         Log::getInstance().d("invalid sai_list, Drop SAI_LIST indication: "+ std::to_string(list_len));
         return;
       }

       /* validate number of freqencies */
       list_len = (unsigned short)((list_len > QMI_WDS_EMBMS_FREQ_SAI_MAX_V01)?
                                   QMI_WDS_EMBMS_FREQ_SAI_MAX_V01: list_len);

       /* calculate the size for arrays */
       for (list_index = 0; list_index < list_len; list_index ++)
       {
         sai_size = embmsPayload->embms_sai_info.freq_sai_list[list_index].sai_list_len;
         /* validate sai_size */
         if (sai_size > QMI_WDS_EMBMS_SAI_LIST_MAX_V01)
         {
           Log::getInstance().d("sai_list_len_per_freq is invalid, Drop SAI_LIST indication: " + std::to_string(sai_size));
           return;
         }

         available_sai_list_size = (unsigned short) (available_sai_list_size + sai_size);
         if (TRUE == embmsPayload->embms_sai_info.freq_sai_list[list_index].is_serving_frequency)
         {
           camped_sai_list_size = (unsigned short) (camped_sai_list_size + sai_size);
         }
       }

       memset(&sai_indication, 0, sizeof(sai_indication));
       sai_indication.available_sai_list_len = available_sai_list_size;
       sai_indication.camped_sai_list_len = camped_sai_list_size;
       sai_indication.num_of_sai_per_group_len = list_len;

       /* validate available_sai_list_len */
       if (sai_indication.available_sai_list_len > SAI_MAX_V01)
       {
         Log::getInstance().d(" Oversize available_sai_list_len, Drop SAI_LIST indication: "+
                         std::to_string(sai_indication.available_sai_list_len));
         return;
       }
       /* validate camped_sai_list_len */
       if (sai_indication.camped_sai_list_len > SAI_PER_FREQ_MAX_V01)
       {
         Log::getInstance().d(" Oversize camped_sai_list_len, Drop SAI_LIST indication: " +
                         std::to_string(sai_indication.camped_sai_list_len));
         return;
       }
       /* validate num_of_sai_per_group_len */
       if (sai_indication.num_of_sai_per_group_len > FREQ_MAX_V01)
       {
         Log::getInstance().d(" Oversize num_of_sai_per_group_len, Drop SAI_LIST indication: " +
                         std::to_string(sai_indication.num_of_sai_per_group_len));
         return;
       }

       camped_index = avail_index = 0;
       for (list_index = 0; list_index < sai_indication.num_of_sai_per_group_len; list_index ++)
       {
         sai_size = embmsPayload->embms_sai_info.freq_sai_list[list_index].sai_list_len;
         for(freq_index = 0; freq_index < sai_size; freq_index++)
         {
           sai_indication.available_sai_list[ avail_index ++ ] =
               embmsPayload->embms_sai_info.freq_sai_list[list_index].sai_list[freq_index];
           if(TRUE == embmsPayload->embms_sai_info.freq_sai_list[list_index].is_serving_frequency)
           {
             sai_indication.camped_sai_list[ camped_index ++ ] =
               embmsPayload->embms_sai_info.freq_sai_list[list_index].sai_list[freq_index];
           }
         }
         sai_indication.num_of_sai_per_group[list_index] = sai_size;
       }

       if(embmsPayload->embms_sai_info.tranx_id_valid)
       {
         sai_indication.dbg_trace_id = embmsPayload->embms_sai_info.tranx_id;
       }

       Log::getInstance().d( "EMBMS SAI list indication, available_sai_list_len: " + std::to_string(sai_indication.available_sai_list_len) +
                       "camped_sai_list_len: " + std::to_string(sai_indication.camped_sai_list_len) +
                       "num_of_sai_per_group_len: " + std::to_string(sai_indication.num_of_sai_per_group_len) +
                       "dbg_trace_id: " + std::to_string(sai_indication.dbg_trace_id));

       unsigned int i;
       for (i = 0; i < sai_indication.available_sai_list_len; i++)
       {
          Log::getInstance().d("available_sai_list: " + std::to_string(sai_indication.available_sai_list[i]));
       }
       for (i = 0; i < sai_indication.camped_sai_list_len; i++)
       {
          Log::getInstance().d("camped_sai_list: " + std::to_string(sai_indication.camped_sai_list[i]));
       }
       for (i = 0; i < sai_indication.num_of_sai_per_group_len; i++)
       {
          Log::getInstance().d("num_of_sai_per_group: " + std::to_string(sai_indication.num_of_sai_per_group[i]));
       }
       qcril_hook_unsol_response((qcril_instance_id_e_type)instanceId,
                                 QCRIL_EVT_HOOK_EMBMS_UNSOL_SAI_LIST,
                                 (char*)&sai_indication,
                                 sizeof(sai_indication));
     }
     break;

     case DSI_NET_TMGI_LIST_CHANGED:
     {
       Log::getInstance().d(">>>DSI_NET_TMGI_LIST_CHANGED: START>>> list indication type :" +
                            std::to_string(embmsPayload->embms_list_info.tmgi_list.list_type));

       if(embmsPayload->embms_list_info.tmgi_list_valid != TRUE)
       {
         Log::getInstance().d( "invalid tmgi_list, Dropping TMGI list indication" );
         return;
       }

       switch(embmsPayload->embms_list_info.tmgi_list.list_type)
       {
         case WDS_EMBMS_TMGI_LIST_ACTIVE_V01:
           {
             int index = 0;
             memset(&active_indication, 0, sizeof(active_indication));
             active_indication.tmgi_info_len =
               embmsPayload->embms_list_info.tmgi_list.tmgi_list_len;

             /* validate tmgi_info_len */
             if(active_indication.tmgi_info_len > NUMBER_MAX_V01)
             {
               Log::getInstance().d( "Oversize tmgi_info_len, Dropping TMGI list indication: " +
                                  std::to_string(active_indication.tmgi_info_len));
               return;
             }

             for (index = 0; index < (int)active_indication.tmgi_info_len; index++)
             {
               active_indication.tmgi_info[index].tmgi_len = TMGI_LENGTH_MAX_V01;
               memcpy(&(active_indication.tmgi_info[index].tmgi),
                      &(embmsPayload->embms_list_info.tmgi_list.tmgi_list[index].tmgi),
                      TMGI_LENGTH_MAX_V01);
             }

             if(embmsPayload->embms_list_info.tranx_id_valid)
             {
               active_indication.dbg_trace_id =
                 embmsPayload->embms_list_info.tranx_id;
             }

             Log::getInstance().d( "EMBMS ACTIVE list ind debug_id" + std::to_string(active_indication.dbg_trace_id) +
                                   "num TMGI's " + std::to_string(active_indication.tmgi_info_len));

             qcril_hook_unsol_response((qcril_instance_id_e_type)instanceId,
                                       QCRIL_EVT_HOOK_EMBMS_UNSOL_ACTIVE_TMGI,
                                       (char*)&active_indication,
                                       sizeof(active_indication));
           }
           break;

         case WDS_EMBMS_TMGI_LIST_OOS_WARNING_V01:
           {
             int index = 0;
             embms_unsol_oos_warning_ind_msg_v01   oos_warning_ind;
             memset(&oos_warning_ind, 0, sizeof(oos_warning_ind));
             oos_warning_ind.tmgi_info_len =
               embmsPayload->embms_list_info.tmgi_list.tmgi_list_len;

             /* validate tmgi_info_len */
             if(oos_warning_ind.tmgi_info_len > NUMBER_MAX_V01)
             {
               Log::getInstance().d( "Oversize tmgi_info_len, Dropping TMGI list indication:" +
                                  std::to_string(oos_warning_ind.tmgi_info_len) );
               return;
             }

             for (index = 0; index < (int)oos_warning_ind.tmgi_info_len; index++)
             {
               oos_warning_ind.tmgi_info[index].tmgi_len = TMGI_LENGTH_MAX_V01;
               memcpy(&(oos_warning_ind.tmgi_info[index].tmgi),
                      &(embmsPayload->embms_list_info.tmgi_list.tmgi_list[index].tmgi),
                      TMGI_LENGTH_MAX_V01);
             }

             if(embmsPayload->embms_list_info.tranx_id_valid)
             {
               oos_warning_ind.dbg_trace_id =
                 embmsPayload->embms_list_info.tranx_id;
             }
             oos_warning_ind.reason = embmsPayload->embms_list_info.warn_reason;

             Log::getInstance().d( "EMBMS OOS warning ind " + std::to_string(oos_warning_ind.reason) +
                                   "debug_id " + std::to_string(oos_warning_ind.dbg_trace_id) +
                                  "num TMGI's " + std::to_string(oos_warning_ind.tmgi_info_len));

             qcril_hook_unsol_response((qcril_instance_id_e_type)instanceId,
                                       QCRIL_EVT_HOOK_EMBMS_UNSOL_OSS_WARNING,
                                       (char*)&oos_warning_ind,
                                       sizeof(oos_warning_ind));
           }
           break;

         case WDS_EMBMS_TMGI_LIST_AVAILABLE_V01:
           {
             embms_unsol_available_tmgi_ind_msg_v01 available_tmgi_list;
             int index = 0;

             memset(&available_tmgi_list, 0, sizeof(available_tmgi_list));
             available_tmgi_list.tmgi_info_len =
               embmsPayload->embms_list_info.tmgi_list.tmgi_list_len;

             /* validate tmgi_info_len */
             if(available_tmgi_list.tmgi_info_len > NUMBER_MAX_V01)
             {
               Log::getInstance().d( "Oversize tmgi_info_len, Dropping TMGI list indication:" +
                                  std::to_string(available_tmgi_list.tmgi_info_len));
               return;
             }

             for (index = 0; index < (int)available_tmgi_list.tmgi_info_len; index++)
             {
               available_tmgi_list.tmgi_info[index].tmgi_len = TMGI_LENGTH_MAX_V01;
               memcpy(&(available_tmgi_list.tmgi_info[index].tmgi),
                      &(embmsPayload->embms_list_info.tmgi_list.tmgi_list[index].tmgi),
                      TMGI_LENGTH_MAX_V01);
             }

             if(embmsPayload->embms_list_info.tranx_id_valid)
             {
               available_tmgi_list.dbg_trace_id = embmsPayload->embms_list_info.tranx_id;
             }

             Log::getInstance().d( "EMBMS AVAILABLE list ind debug_id" + std::to_string(available_tmgi_list.dbg_trace_id) +
                                   "num TMGI's" + std::to_string(available_tmgi_list.tmgi_info_len));

             qcril_hook_unsol_response((qcril_instance_id_e_type)instanceId,
                                       QCRIL_EVT_HOOK_EMBMS_UNSOL_AVAILABLE_TMGI,
                                       (char*)&available_tmgi_list,
                                       sizeof(available_tmgi_list));
           }
           break;

         default:
           break;
       }
     }
     break;

     case DSI_NET_TMGI_DEACTIVATED:
       processTmgiDeactivatedEvent(embmsPayload);
       break;

     case DSI_NET_TMGI_ACTIVATED_DEACTIVATED:
       processTmgiActivatedDeactivatedEvent(embmsPayload);
       break;

     case DSI_NET_TMGI_ACTIVATED:
       processTmgiActivatedEvent(embmsPayload);
       break;

     case DSI_NET_CONTENT_DESC_CONTROL:
     {
       embms_unsol_content_desc_update_per_obj_ind_msg_v01 content_desc_ctrl_ind;
       Log::getInstance().d(">>>DSI_NET_CONTENT_DESC_CONTROL: START>>>");

       memset(&content_desc_ctrl_ind, 0, sizeof(content_desc_ctrl_ind));
       memcpy(&(content_desc_ctrl_ind.tmgi_info.tmgi),
              &(embmsPayload->embms_content_desc_info.tmgi.tmgi),
              TMGI_LENGTH_MAX_V01);

       content_desc_ctrl_ind.tmgi_info.tmgi_len = TMGI_LENGTH_MAX_V01;
       if(embmsPayload->embms_content_desc_info.tranx_id_valid)
       {
         content_desc_ctrl_ind.dbg_trace_id = embmsPayload->embms_content_desc_info.tranx_id;
       }
       if (embmsPayload->embms_content_desc_info.content_control_valid)
       {
         content_desc_ctrl_ind.per_object_content_ctrl_valid = TRUE;
         content_desc_ctrl_ind.per_object_content_ctrl =
             embmsPayload->embms_content_desc_info.content_control;
       }
       if (embmsPayload->embms_content_desc_info.status_control_valid)
       {
         content_desc_ctrl_ind.per_object_status_ctrl_valid = TRUE;
         content_desc_ctrl_ind.per_object_status_ctrl =
             embmsPayload->embms_content_desc_info.status_control;
       }
       Log::getInstance().d( "EMBMS content desc control indication status_control: " + std::to_string(content_desc_ctrl_ind.per_object_status_ctrl) +
                       "valid: " + std::to_string(content_desc_ctrl_ind.per_object_status_ctrl_valid) +
                       "content_control: " + std::to_string(content_desc_ctrl_ind.per_object_content_ctrl) +
                       "valid: " + std::to_string(content_desc_ctrl_ind.per_object_content_ctrl_valid) +
                       "dbg_trace_id: " + std::to_string(content_desc_ctrl_ind.dbg_trace_id) +
                       "tmgi_list len " + std::to_string(content_desc_ctrl_ind.tmgi_info.tmgi_len) +
                       "tmgi_list :" + std::to_string(content_desc_ctrl_ind.tmgi_info.tmgi[0]) +
                       std::to_string(content_desc_ctrl_ind.tmgi_info.tmgi[1]) +
                       std::to_string(content_desc_ctrl_ind.tmgi_info.tmgi[2]) +
                       std::to_string(content_desc_ctrl_ind.tmgi_info.tmgi[3]) +
                       std::to_string(content_desc_ctrl_ind.tmgi_info.tmgi[4]) +
                       std::to_string(content_desc_ctrl_ind.tmgi_info.tmgi[5]));

       qcril_hook_unsol_response((qcril_instance_id_e_type)instanceId,
                                 QCRIL_EVT_HOOK_EMBMS_UNSOL_CONTENT_DESC_CONTROL,
                                 (char*)&content_desc_ctrl_ind,
                                 sizeof(content_desc_ctrl_ind));
     }
     break;

     case DSI_NET_TMGI_SERVICE_INTERESTED:
     {
       embms_unsol_get_interested_tmgi_list_req_msg_v01 get_interested_tmgi_list;
       Log::getInstance().d(">>>DSI_NET_TMGI_SERVICE_INTERESTED: START>>>");
       memset(&get_interested_tmgi_list, 0, sizeof(get_interested_tmgi_list));

       if(embmsPayload->embms_svc_interest_info.tranx_id_valid)
       {
         get_interested_tmgi_list.dbg_trace_id =
             embmsPayload->embms_svc_interest_info.tranx_id;
       }

       Log::getInstance().d("EMBMS get interested TMGI list, dbg_trace_id: "+
                             std::to_string(get_interested_tmgi_list.dbg_trace_id));

       qcril_hook_unsol_response((qcril_instance_id_e_type)instanceId,
                                 QCRIL_EVT_HOOK_EMBMS_UNSOL_GET_INTERESTED_TMGI_LIST,
                                 (char*)&get_interested_tmgi_list,
                                 sizeof(get_interested_tmgi_list));
     }
     break;

     default:
       Log::getInstance().d("Unknown embms event received: " + std::to_string(evt));
       break;
   } /*switch(evt)*/
}
