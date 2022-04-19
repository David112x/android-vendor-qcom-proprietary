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
 * Configure call waiting state
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : std::shared_ptr<qcril::interfaces::SipErrorInfo>
 */
class QcRilRequestSetCallWaitingMessage : public QcRilRequestMessage,
                                          public add_message_id<QcRilRequestSetCallWaitingMessage> {
 private:
  std::optional<qcril::interfaces::ServiceClassStatus> mServiceStatus;
  std::optional<uint32_t> mServiceClass;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestSetCallWaitingMessage";

  QcRilRequestSetCallWaitingMessage() = delete;

  ~QcRilRequestSetCallWaitingMessage() {}

  inline QcRilRequestSetCallWaitingMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  bool hasServiceStatus() { return mServiceStatus ? true : false; }
  qcril::interfaces::ServiceClassStatus getServiceStatus() { return *mServiceStatus; }
  void setServiceStatus(qcril::interfaces::ServiceClassStatus val) { mServiceStatus = val; }
  inline string dumpServiceStatus() {
    return " mServiceStatus = " +
           (mServiceStatus ? qcril::interfaces::toString(*mServiceStatus) : "<invalid>");
  }

  bool hasServiceClass() { return mServiceClass ? true : false; }
  uint32_t getServiceClass() { return *mServiceClass; }
  void setServiceClass(uint32_t val) { mServiceClass = val; }
  inline string dumpServiceClass() {
    return " mServiceClass = " + (mServiceClass ? std::to_string(*mServiceClass) : "<invalid>");
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() +
          "{" + dumpServiceStatus() + ", " + dumpServiceClass() + "}";
  }
};
