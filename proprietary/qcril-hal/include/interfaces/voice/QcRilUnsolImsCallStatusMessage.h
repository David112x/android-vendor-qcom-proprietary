/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>
#include <interfaces/voice/voice.h>

/*
 * Unsol message to notify call state change
 */
class QcRilUnsolImsCallStatusMessage : public UnSolicitedMessage,
                                       public add_message_id<QcRilUnsolImsCallStatusMessage> {
 private:
  std::vector<qcril::interfaces::CallInfo> mCallInfo;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolImsCallStatusMessage";

  ~QcRilUnsolImsCallStatusMessage() {}

  QcRilUnsolImsCallStatusMessage() : UnSolicitedMessage(get_class_message_id()) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolImsCallStatusMessage> msg =
        std::make_shared<QcRilUnsolImsCallStatusMessage>();
    msg->setCallInfo(mCallInfo);
    return msg;
  }

  void addToCallInfo(qcril::interfaces::CallInfo callInfo) { mCallInfo.push_back(callInfo); }

  void setCallInfo(std::vector<qcril::interfaces::CallInfo> callInfo) { mCallInfo = callInfo; }

  std::vector<qcril::interfaces::CallInfo> getCallInfo() { return mCallInfo; };

  std::string dump() { return mName + "{}"; }
};
