/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>
#include <interfaces/voice/voice.h>
#include <optional>

/*
 * Unsol message to notify when there is an ALPHA from UICC during Call Control.
 */
class QcRilUnsolStkCcAlphaNotifyMessage : public UnSolicitedMessage,
                                          public add_message_id<QcRilUnsolStkCcAlphaNotifyMessage> {
 private:
  std::optional<std::string> mAlpha;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolStkCcAlphaNotifyMessage";
  ~QcRilUnsolStkCcAlphaNotifyMessage() {}

  QcRilUnsolStkCcAlphaNotifyMessage() : UnSolicitedMessage(get_class_message_id()) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolStkCcAlphaNotifyMessage> msg =
        std::make_shared<QcRilUnsolStkCcAlphaNotifyMessage>();
    if (msg) {
      if (mAlpha) {
        msg->setAlpha(*mAlpha);
      }
    }
    return msg;
  }

  bool hasAlpha() { return mAlpha ? true : false; }
  std::string getAlpha() { return *mAlpha; }
  void setAlpha(std::string val) { mAlpha = val; }
  inline std::string dumpAlpha() { return " mAlpha = " + (mAlpha ? *mAlpha : "<invalid>"); }

  std::string dump() { return mName + "{" + dumpAlpha() + "}"; }
};
