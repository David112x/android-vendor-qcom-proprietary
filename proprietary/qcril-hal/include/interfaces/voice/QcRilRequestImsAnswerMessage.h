/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/add_message_id.h>
#include <interfaces/QcRilRequestMessage.h>
#include <interfaces/voice/QcRilRequestAnswerMessage.h>
#include <interfaces/voice/voice.h>
#include <optional>

/*
 * Answer incoming or waiting call
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : nullptr
 */
class QcRilRequestImsAnswerMessage : public QcRilRequestAnswerMessage,
                                    public add_message_id<QcRilRequestImsAnswerMessage> {
 private:
 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestImsAnswerMessage";

  QcRilRequestImsAnswerMessage() = delete;

  ~QcRilRequestImsAnswerMessage() {}

  inline QcRilRequestImsAnswerMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestAnswerMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() + "{}";
  }
};
