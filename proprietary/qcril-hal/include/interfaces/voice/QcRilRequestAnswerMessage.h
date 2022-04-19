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
 * Request to answer an incoming call
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : nullptr
 */
class QcRilRequestAnswerMessage : public QcRilRequestMessage {
 private:
  std::optional<qcril::interfaces::CallType> mCallType;
  std::optional<qcril::interfaces::Presentation> mPresentation;
  std::optional<qcril::interfaces::RttMode> mRttMode;

 public:
  QcRilRequestAnswerMessage() = delete;

  ~QcRilRequestAnswerMessage() {}

  inline QcRilRequestAnswerMessage(message_id_t msg_id, std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(msg_id, context) {}

  bool hasCallType() { return mCallType ? true : false; }
  qcril::interfaces::CallType getCallType() { return *mCallType; }
  void setCallType(qcril::interfaces::CallType val) { mCallType = val; }
  inline string dumpCallType() {
    return "mCallType = " + (mCallType ? toString(*mCallType) : "<invalid>");
  }

  bool hasPresentation() { return mPresentation ? true : false; }
  qcril::interfaces::Presentation getPresentation() { return *mPresentation; }
  void setPresentation(qcril::interfaces::Presentation val) { mPresentation = val; }
  inline string dumpPresentation() {
    return "mPresentation = " + (mPresentation ? toString(*mPresentation) : "<invalid>");
  }

  bool hasRttMode() { return mRttMode ? true : false; }
  qcril::interfaces::RttMode getRttMode() { return *mRttMode; }
  void setRttMode(qcril::interfaces::RttMode val) { mRttMode = val; }
  inline string dumpRttMode() {
    return "mRttMode = " + (mRttMode ? toString(*mRttMode) : "<invalid>");
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() +
          "{" + dumpCallType() + dumpPresentation() + dumpRttMode() + "}";
  }
};
