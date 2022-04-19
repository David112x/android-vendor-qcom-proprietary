/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/add_message_id.h>
#include <interfaces/QcRilRequestMessage.h>
#include <interfaces/voice/QcRilRequestDialMessage.h>
#include <interfaces/voice/voice.h>
#include <optional>

/*
 * Request to make a IMS MO call
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : nullptr
 */
class QcRilRequestImsDialMessage : public QcRilRequestDialMessage,
                                   public add_message_id<QcRilRequestImsDialMessage> {
 private:
 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestImsDialMessage";

  QcRilRequestImsDialMessage() = delete;

  ~QcRilRequestImsDialMessage() {}

  inline QcRilRequestImsDialMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestDialMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }
};
