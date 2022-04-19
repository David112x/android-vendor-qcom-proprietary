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
 * Connects the two calls and disconnects the subscriber from both calls.
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : nullptr
 */
class QcRilRequestExplicitCallTransferMessage
    : public QcRilRequestMessage,
      public add_message_id<QcRilRequestExplicitCallTransferMessage> {
 private:
 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestExplicitCallTransferMessage";

  QcRilRequestExplicitCallTransferMessage() = delete;

  ~QcRilRequestExplicitCallTransferMessage() {}

  inline QcRilRequestExplicitCallTransferMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() + "{}";
  }
};
