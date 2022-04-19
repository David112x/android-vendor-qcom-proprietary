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
 * Sets the COLR (Connected Line Identification Restriction) / TIR (Terminating Identification
 * Restriction) parameter
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : std::shared_ptr<qcril::interfaces::SuppServiceStatusInfo>
 */
class QcRilRequestSetColrMessage : public QcRilRequestMessage,
                                   public add_message_id<QcRilRequestSetColrMessage> {
 private:
  std::optional<qcril::interfaces::Presentation> mPresentation;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestSetColrMessage";

  QcRilRequestSetColrMessage() = delete;

  ~QcRilRequestSetColrMessage() {}

  inline QcRilRequestSetColrMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  bool hasPresentation() { return mPresentation ? true : false; }
  qcril::interfaces::Presentation getPresentation() { return *mPresentation; }
  void setPresentation(qcril::interfaces::Presentation val) { mPresentation = val; }
  inline string dumpPresentation() {
    return " mPresentation = " + (mPresentation ? toString(*mPresentation) : "<invalid>");
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() +
          "{" + dumpPresentation() + "}";
  }
};
