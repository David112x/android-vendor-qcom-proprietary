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
 * Request to stop playing a currently playing DTMF tone.
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : nullptr
 */
class QcRilRequestStopDtmfMessage : public QcRilRequestMessage,
                                    public add_message_id<QcRilRequestStopDtmfMessage> {
 private:
  // std::optional<uint8_t> mDigit;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestStopDtmfMessage";

  QcRilRequestStopDtmfMessage() = delete;

  ~QcRilRequestStopDtmfMessage() {}

  inline QcRilRequestStopDtmfMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() + "{}";
  }
};
