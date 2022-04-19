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
 * Hang up waiting or held
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : nullptr
 */
class QcRilRequestHangupWaitingOrBackgroundMessage
    : public QcRilRequestMessage,
      public add_message_id<QcRilRequestHangupWaitingOrBackgroundMessage> {
 private:
 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestHangupWaitingOrBackgroundMessage";

  QcRilRequestHangupWaitingOrBackgroundMessage() = delete;

  ~QcRilRequestHangupWaitingOrBackgroundMessage() {}

  inline QcRilRequestHangupWaitingOrBackgroundMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() + "{}";
  }
};
