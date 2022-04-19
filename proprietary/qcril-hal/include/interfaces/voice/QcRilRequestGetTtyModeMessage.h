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
 *   responseData : std::shared_ptr<qcril::interfaces::TtyModeResp>
 */
class QcRilRequestGetTtyModeMessage : public QcRilRequestMessage,
                                      public add_message_id<QcRilRequestGetTtyModeMessage> {
 private:

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestGetTtyModeMessage";

  QcRilRequestGetTtyModeMessage() = delete;

  ~QcRilRequestGetTtyModeMessage() {}

  inline QcRilRequestGetTtyModeMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() + "{}";
  }
};
