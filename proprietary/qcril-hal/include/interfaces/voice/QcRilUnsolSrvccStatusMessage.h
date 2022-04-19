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
 * Unsol message to notify call ringing indication
 */
class QcRilUnsolSrvccStatusMessage : public UnSolicitedMessage,
                                     public add_message_id<QcRilUnsolSrvccStatusMessage> {
 private:
  std::optional<RIL_SrvccState> mState;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolSrvccStatusMessage";

  ~QcRilUnsolSrvccStatusMessage() {}

  QcRilUnsolSrvccStatusMessage() : UnSolicitedMessage(get_class_message_id()) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolSrvccStatusMessage> msg =
        std::make_shared<QcRilUnsolSrvccStatusMessage>();
    return msg;
  }

  bool hasState() { return mState ? true : false; }
  RIL_SrvccState getState() {
    return *mState;
  }
  void setState(RIL_SrvccState val) {
    mState = val;
  }
  inline std::string dumpState() {
    return " mState = " + (mState ? std::to_string(*mState) : "<invalid>");
  }

  std::string dump() { return mName + "{" + dumpState() + "}"; }
};
