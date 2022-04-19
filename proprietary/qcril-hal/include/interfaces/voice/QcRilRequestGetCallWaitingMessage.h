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
 * Request to query current call waiting status
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : std::shared_ptr<qcril::interfaces::CallWaitingInfo>
 */
class QcRilRequestGetCallWaitingMessage : public QcRilRequestMessage,
                                          public add_message_id<QcRilRequestGetCallWaitingMessage> {
 private:
  std::optional<uint32_t> mServiceClass;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestGetCallWaitingMessage";

  QcRilRequestGetCallWaitingMessage() = delete;

  ~QcRilRequestGetCallWaitingMessage() {}

  inline QcRilRequestGetCallWaitingMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  bool hasServiceClass() { return mServiceClass ? true : false; }
  uint32_t getServiceClass() { return *mServiceClass; }
  void setServiceClass(uint32_t val) { mServiceClass = val; }
  inline string dumpServiceClass() {
    return "mServiceClass = " + (mServiceClass ? std::to_string(*mServiceClass) : "<invalid>");
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() +
          "{" + dumpServiceClass() + "}";
  }
};
