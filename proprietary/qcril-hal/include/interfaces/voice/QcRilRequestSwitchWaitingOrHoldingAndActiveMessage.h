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
 * Switch waiting or holding call and active call
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : nullptr
 */
class QcRilRequestSwitchWaitingOrHoldingAndActiveMessage
    : public QcRilRequestMessage,
      public add_message_id<QcRilRequestSwitchWaitingOrHoldingAndActiveMessage> {
 private:
 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestSwitchWaitingOrHoldingAndActiveMessage";

  QcRilRequestSwitchWaitingOrHoldingAndActiveMessage() = delete;

  ~QcRilRequestSwitchWaitingOrHoldingAndActiveMessage() {}

  inline QcRilRequestSwitchWaitingOrHoldingAndActiveMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() + "{}";
  }
};
