/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/add_message_id.h>
#include <interfaces/QcRilRequestMessage.h>
#include <interfaces/voice/voice.h>
#include <optional>

/*
 * Request to set the TTY mode
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : nullptr
 */
class QcRilRequestSetTtyModeMessage : public QcRilRequestMessage,
                                      public add_message_id<QcRilRequestSetTtyModeMessage> {
 private:
  std::optional<qcril::interfaces::TtyMode> mTtyMode;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestSetTtyModeMessage";

  QcRilRequestSetTtyModeMessage() = delete;

  ~QcRilRequestSetTtyModeMessage() {}

  inline QcRilRequestSetTtyModeMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  bool hasTtyMode() { return mTtyMode ? true : false; }
  qcril::interfaces::TtyMode getTtyMode() { return *mTtyMode; }
  void setTtyMode(qcril::interfaces::TtyMode val) { mTtyMode = val; }
  inline string dumpTtyMode() {
    return " mTtyMode = " + (mTtyMode ? toString(*mTtyMode) : "<invalid>");
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() +
          "{" + dumpTtyMode() + "}";
  }
};
