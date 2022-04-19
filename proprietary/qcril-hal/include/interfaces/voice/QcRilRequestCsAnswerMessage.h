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
class QcRilRequestCsAnswerMessage : public QcRilRequestAnswerMessage,
                                    public add_message_id<QcRilRequestCsAnswerMessage> {
 private:
 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestCsAnswerMessage";

  QcRilRequestCsAnswerMessage() = delete;

  ~QcRilRequestCsAnswerMessage() {}

  inline QcRilRequestCsAnswerMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestAnswerMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

#if 0
  virtual string dump() {
    return QcRilRequestAnswerMessage::dump();
  }
#endif
};
