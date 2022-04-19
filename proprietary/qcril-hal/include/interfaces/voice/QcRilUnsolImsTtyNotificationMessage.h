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
 * Unsol message to notify TTY
 */
class QcRilUnsolImsTtyNotificationMessage : public UnSolicitedMessage,
                                       public add_message_id<QcRilUnsolImsTtyNotificationMessage> {
 private:
  std::optional<qcril::interfaces::TtyMode> mTtyMode;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolImsTtyNotificationMessage";
  ~QcRilUnsolImsTtyNotificationMessage() {}

  QcRilUnsolImsTtyNotificationMessage() : UnSolicitedMessage(get_class_message_id()) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolImsTtyNotificationMessage> msg =
        std::make_shared<QcRilUnsolImsTtyNotificationMessage>();
    if (msg) {
      if (mTtyMode) {
        msg->setTtyMode(*mTtyMode);
      }
    }
    return msg;
  }

  bool hasTtyMode() { return mTtyMode ? true : false; }
  qcril::interfaces::TtyMode getTtyMode() { return *mTtyMode; }
  void setTtyMode(qcril::interfaces::TtyMode val) { mTtyMode = val; }
  inline std::string dumpTtyMode() {
    return " mTtyMode = " + (mTtyMode ? toString(*mTtyMode) : "<invalid>");
  }

  string dump() { return mName + "{" + dumpTtyMode() + "}"; }
};
