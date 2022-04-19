/******************************************************************************
#  Copyright (c) 2020 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/add_message_id.h>
#include <interfaces/QcRilRequestMessage.h>
#include <interfaces/voice/voice.h>
#include <optional>
#include <string>

/*
 * Request to turn ON or OFF uplink (microphone) mute.
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : nullptr
 */
class QcRilRequestSetMuteMessage : public QcRilRequestMessage,
                                   public add_message_id<QcRilRequestSetMuteMessage> {
 private:
  std::optional<bool> mEnable;

 public:
  static constexpr const char* MESSAGE_NAME = "QcRilRequestSetMuteMessage";

  QcRilRequestSetMuteMessage() = delete;

  ~QcRilRequestSetMuteMessage() {
  }

  inline QcRilRequestSetMuteMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  bool hasEnable() {
    return mEnable ? true : false;
  }
  uint32_t getEnable() {
    return *mEnable;
  }
  void setEnable(uint32_t val) {
    mEnable = val;
  }

  virtual string dump() {
    std::string os;
    os += QcRilRequestMessage::dump();
    os += "{";
    os += ".mEnable = ";
    os += mEnable ? *mEnable ? "true" : "false" : "<invalid";
    os += "}";
    return os;
  }
};
