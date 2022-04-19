/******************************************************************************
#  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/add_message_id.h>
#include <interfaces/QcRilRequestMessage.h>
#include <interfaces/voice/voice.h>
#include <optional>

/*
 * Request to answer a setup call
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : nullptr
 */
class QcRilRequestSetupAnswerMessage
    : public QcRilRequestMessage,
      public add_message_id<QcRilRequestSetupAnswerMessage> {
 private:
  std::optional<bool> mRejectSetup;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestSetupAnswerMessage";

  QcRilRequestSetupAnswerMessage() = delete;

  ~QcRilRequestSetupAnswerMessage() {}

  inline QcRilRequestSetupAnswerMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  bool hasRejectSetup() { return mRejectSetup ? true : false; }
  bool getRejectSetup() { return *mRejectSetup; }
  void setRejectSetup(bool val) { mRejectSetup = val; }
  inline std::string dumpRejectSetup() {
    return std::string(" mRejectSetup = ") +
           (mRejectSetup ? (*mRejectSetup ? "true" : "fase") : "<invalid>");
  }

  virtual string dump() { return QcRilRequestMessage::dump() + "{" + dumpRejectSetup() + "}"; }
};
