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
 * Request to start playing a DTMF tone.
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : nullptr
 */
class QcRilRequestStartDtmfMessage : public QcRilRequestMessage,
                                     public add_message_id<QcRilRequestStartDtmfMessage> {
 private:
  std::optional<uint8_t> mDigit;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestStartDtmfMessage";

  QcRilRequestStartDtmfMessage() = delete;

  ~QcRilRequestStartDtmfMessage() {}

  inline QcRilRequestStartDtmfMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  bool hasDigit() { return mDigit ? true : false; }
  uint32_t getDigit() { return *mDigit; }
  void setDigit(uint32_t val) { mDigit = val; }
  inline string dumpDigit() {
    return "mDigit = " + (mDigit ? std::to_string(*mDigit) : "<invalid>");
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() +
          "{" + dumpDigit() + "}";
  }
};
