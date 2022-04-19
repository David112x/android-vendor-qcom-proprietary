/******************************************************************************
#  Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <cstring>
#include "VoiceCallModemEndPointModule.h"
#include <modules/qmi/QmiIndMessage.h>

using namespace rildata;

VoiceCallModemEndPointModule::VoiceCallModemEndPointModule(string name,
      ModemEndPoint &owner, SubId subId) : ModemEndPointModule(name, owner)
{
  mServiceObject = nullptr;
  mSub = subId;
  mLooper = std::unique_ptr<ModuleLooper>(new ModuleLooper);

  using std::placeholders::_1;
  if(subId == SubId::SECONDARY)
  {
    mMessageHandler = {
        {REG_MSG("VoiceSub1EndPoint_QMI_IND"),
               std::bind(&VoiceCallModemEndPointModule::handleVoiceIndMessage, this, _1)},
    };
  }
  else
  {
    mMessageHandler = {
        {REG_MSG("VoiceSub0EndPoint_QMI_IND"),
              std::bind(&VoiceCallModemEndPointModule::handleVoiceIndMessage, this, _1)},
    };
  }
}

VoiceCallModemEndPointModule::~VoiceCallModemEndPointModule()
{
  mLooper = nullptr;
}

void VoiceCallModemEndPointModule::init()
{
  ModemEndPointModule::init();
  mStackId[0] = 0;
}

qmi_idl_service_object_type VoiceCallModemEndPointModule::getServiceObject()
{
  return voice_get_service_object_v02();
}

bool VoiceCallModemEndPointModule::handleQmiBinding
(
  qcril_instance_id_e_type instanceId, int8_t stackId
)
{
  Log::getInstance().d("[VoiceCallModemEndPointModule] handleQmiBinding");

  (void)instanceId;
  if (stackId < 0)
    return false;

  voice_bind_subscription_req_msg_v02 voice_bind_request;
  voice_bind_subscription_resp_msg_v02 voice_bind_resp;
  memset(&voice_bind_request, 0, sizeof(voice_bind_request));
  memset(&voice_bind_resp, 0, sizeof(voice_bind_resp));

  if (mSub == SubId::PRIMARY) {
    voice_bind_request.subs_type = VOICE_SUBS_TYPE_PRIMARY_V02;
  } else if(mSub == SubId::SECONDARY) {
    voice_bind_request.subs_type = VOICE_SUBS_TYPE_SECONDARY_V02;
  } else if (mSub == SubId::TERTIARY) {
    voice_bind_request.subs_type = VOICE_SUBS_TYPE_TERTIARY_V02;
  } else
    return false;

  int ntries = 0;
  do {
    qmi_client_error_type res = qmi_client_send_msg_sync(
         mQmiSvcClient, QMI_VOICE_BIND_SUBSCRIPTION_REQ_V02,
         (void*)&voice_bind_request, sizeof(voice_bind_request),
         (void*)&voice_bind_resp, sizeof(voice_bind_resp),
         ModemEndPoint::DEFAULT_SYNC_TIMEOUT);
    if (QMI_NO_ERR == res && voice_bind_resp.resp.result ==
            QMI_RESULT_SUCCESS_V01) {
      Log::getInstance().d(
       "[VoiceCallModemEndPointModule]: QMI binding succeeds. instanceId: "
       + std::to_string((int)instanceId) + " stackId: " + std::to_string(stackId));
       return true;
    }
    usleep(500*1000);
  } while(++ntries < 10);

  return false;
}

void VoiceCallModemEndPointModule::handleVoiceIndMessage
(
  std::shared_ptr<Message> msg
)
{
  auto shared_indMsg(
        std::static_pointer_cast<QmiIndMessage>(msg));
  Log::getInstance().d("[VoiceCallModemEndPointModule]: Handling msg = " + msg->dump());

  QmiIndMsgDataStruct *indData = shared_indMsg->getData();
  if(indData != nullptr)
  {
    VoiceIndHdlr(indData->msgId, indData->indData, indData->indSize);
  } else
  {
    Log::getInstance().d("Unexpected, null data from message");
  }
}

void VoiceCallModemEndPointModule::VoiceIndHdlr
(
  unsigned int   msg_id,
  unsigned char *decoded_payload,
  uint32_t       decoded_payload_len
)
{
  (void)(msg_id);
  (void)(decoded_payload_len);

  Log::getInstance().d("[VoiceCallModemEndPointModule]: VoiceIndHdlr ENTRY");
  switch(msg_id)
  {
    case QMI_VOICE_ALL_CALL_STATUS_IND_V02:
      if(decoded_payload != NULL)
      {
        voice_all_call_status_ind_msg_v02 *resp = (voice_all_call_status_ind_msg_v02 *)(decoded_payload);
        if(resp != NULL)
        {
          uint32_t i;
          VoiceIndResp *voiceResp = nullptr;
          for(i = 0; i < resp->call_info_len; i++)
          {
            voiceResp = new VoiceIndResp;
            voiceResp->voiceSubId = mSub;
            voiceResp->voiceCallState = (VoiceCallStateEnum)((int)resp->call_info[i].call_state);
            voiceResp->voiceCallType = (VoiceCallTypeEnum)((int)resp->call_info[i].call_type);
            voiceResp->voiceCallRat = (VoiceCallRATEnum)((int)resp->call_info[i].mode);

            Log::getInstance().d("sub"+ std::to_string((int)mSub));
            Log::getInstance().d("callState"+ std::to_string((int)voiceResp->voiceCallState));
            Log::getInstance().d("CallType"+std::to_string((int)voiceResp->voiceCallType));
            Log::getInstance().d("CallRAT"+std::to_string((int)voiceResp->voiceCallRat));

            if(voiceResp->voiceCallState ==  VoiceCallStateEnum::CALL_STATE_END)
            {
              break;
            }
          }

          if(voiceResp != nullptr)
          {
            auto msg = std::make_shared<rildata::VoiceIndMessage>(*voiceResp);
            msg->broadcast();
          }
          delete voiceResp;
        }
      }
      break;
    default:
      Log::getInstance().d("Ignoring the Voice indication");
      break;
  }
}
