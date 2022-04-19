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
 * Request the setting of preferred voice privacy mode
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : std::shared_ptr<qcril::interfaces::PrivacyModeResp>
 */
class QcRilRequestGetPreferredVoicePrivacyMessage
    : public QcRilRequestMessage,
      public add_message_id<QcRilRequestGetPreferredVoicePrivacyMessage> {
 private:
 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestGetPreferredVoicePrivacyMessage";

  QcRilRequestGetPreferredVoicePrivacyMessage() = delete;

  ~QcRilRequestGetPreferredVoicePrivacyMessage() {}

  inline QcRilRequestGetPreferredVoicePrivacyMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() + "{}";
  }
};
