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
 * Separate a party from a multiparty call placing the multiparty call (less the specified party) on
 * hold and leaving the specified party as the only other member of the current (active) call
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : nullptr
 */
class QcRilRequestSeparateConnectionMessage : public QcRilRequestMessage,
                                      public add_message_id<QcRilRequestSeparateConnectionMessage> {
 private:
  std::optional<uint32_t> mCallId;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestSeparateConnectionMessage";

  QcRilRequestSeparateConnectionMessage() = delete;

  ~QcRilRequestSeparateConnectionMessage() {}

  inline QcRilRequestSeparateConnectionMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  bool hasCallId() { return mCallId ? true : false; }
  uint32_t getCallId() { return *mCallId; }
  void setCallId(uint32_t val) { mCallId = val; }
  inline string dumpCallId() {
    return "mCallId = " + (mCallId ? std::to_string(*mCallId) : "<invalid>");
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() +
          "{" + dumpCallId() + "}";
  }
};
