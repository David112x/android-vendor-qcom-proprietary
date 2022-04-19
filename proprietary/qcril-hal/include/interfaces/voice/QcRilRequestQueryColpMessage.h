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
#include <vector>

/*
 * Request to query status of COLP (Connected Line Identification Presentation) / TIP
 * (Terminating Identification Presentation)
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : std::shared_ptr<qcril::interfaces::SuppServiceStatusInfo>
 */
class QcRilRequestQueryColpMessage
    : public QcRilRequestMessage,
      public add_message_id<QcRilRequestQueryColpMessage> {
 private:

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestQueryColpMessage";

  QcRilRequestQueryColpMessage() = delete;

  ~QcRilRequestQueryColpMessage() {}

  inline QcRilRequestQueryColpMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() + "{}";
  }
};
