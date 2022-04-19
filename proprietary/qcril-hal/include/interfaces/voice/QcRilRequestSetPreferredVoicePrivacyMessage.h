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
 * Request to set the preferred voice privacy mode used in voice scrambling
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : nullptr
 */
class QcRilRequestSetPreferredVoicePrivacyMessage
    : public QcRilRequestMessage,
      public add_message_id<QcRilRequestSetPreferredVoicePrivacyMessage> {
 private:
  std::optional<qcril::interfaces::PrivacyMode> mPrivacyMode;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestSetPreferredVoicePrivacyMessage";

  QcRilRequestSetPreferredVoicePrivacyMessage() = delete;

  ~QcRilRequestSetPreferredVoicePrivacyMessage() {}

  inline QcRilRequestSetPreferredVoicePrivacyMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  bool hasPrivacyMode() { return mPrivacyMode ? true : false; }
  qcril::interfaces::PrivacyMode getPrivacyMode() { return *mPrivacyMode; }
  void setPrivacyMode(qcril::interfaces::PrivacyMode val) { mPrivacyMode = val; }
  inline string dumpPrivacyMode() {
    return " mPrivacyMode = " + (mPrivacyMode ? toString(*mPrivacyMode) : "<invalid>");
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() +
          "{" + dumpPrivacyMode() + "}";
  }
};
