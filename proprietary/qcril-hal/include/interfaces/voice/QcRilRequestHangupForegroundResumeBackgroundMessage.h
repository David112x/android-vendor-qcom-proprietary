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
 * Hang up waiting or held and resume background
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : nullptr
 */
class QcRilRequestHangupForegroundResumeBackgroundMessage
    : public QcRilRequestMessage,
      public add_message_id<QcRilRequestHangupForegroundResumeBackgroundMessage> {
 private:
 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestHangupForegroundResumeBackgroundMessage";

  QcRilRequestHangupForegroundResumeBackgroundMessage() = delete;

  ~QcRilRequestHangupForegroundResumeBackgroundMessage() {}

  inline QcRilRequestHangupForegroundResumeBackgroundMessage(
      std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  virtual string dump() { return QcRilRequestMessage::dump() + "{}"; }
};
