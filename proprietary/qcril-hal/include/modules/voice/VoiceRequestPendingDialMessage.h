/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/SolicitedMessage.h"
#include "framework/add_message_id.h"
#include "modules/voice/qcril_qmi_voice.h"

class VoiceRequestPendingDialMessage : public SolicitedMessage<void>,
                                       public add_message_id<VoiceRequestPendingDialMessage> {
 private:
  qcril_qmi_pending_call_info_type mCallInfo;

 public:
  static constexpr const char *MESSAGE_NAME = "VoiceRequestPendingDialMessage";
  VoiceRequestPendingDialMessage() = delete;

  inline ~VoiceRequestPendingDialMessage() {}

  inline VoiceRequestPendingDialMessage(qcril_qmi_pending_call_info_type &callInfo)
      : SolicitedMessage<void>(get_class_message_id()) {
    mCallInfo.req_id = callInfo.req_id;
    mCallInfo.responseCb = callInfo.responseCb;
    mCallInfo.commandOversightCompletionHandler = callInfo.commandOversightCompletionHandler;
    mCallInfo.elaboration = callInfo.elaboration;
    memcpy(&(mCallInfo.dial_call_req), &(callInfo.dial_call_req), sizeof(mCallInfo.dial_call_req));
  }
  qcril_qmi_pending_call_info_type &getPendingCallInfo() { return mCallInfo; }

  string dump() { return mName; }
};
