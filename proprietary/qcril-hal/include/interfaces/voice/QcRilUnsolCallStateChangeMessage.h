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
class QcRilUnsolCallStateChangeMessage : public UnSolicitedMessage,
                                         public add_message_id<QcRilUnsolCallStateChangeMessage> {
 private:
 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolCallStateChangeMessage";

  ~QcRilUnsolCallStateChangeMessage() {}

  QcRilUnsolCallStateChangeMessage() : UnSolicitedMessage(get_class_message_id()) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolCallStateChangeMessage> msg =
        std::make_shared<QcRilUnsolCallStateChangeMessage>();
    return msg;
  }

  string dump() { return mName + "{" + (mIsIms ? "isIms=true" : "") + "}"; }

  // To distinguish IMS or CS indication
 private:
  bool mIsIms = false;

 public:
  bool isIms() { return mIsIms; }
  void setIsIms(bool val) { mIsIms = val; }
};
