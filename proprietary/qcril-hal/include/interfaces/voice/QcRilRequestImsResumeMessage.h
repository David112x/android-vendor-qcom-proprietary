/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/add_message_id.h>
#include <interfaces/QcRilRequestMessage.h>
#include <optional>

/*
 * Request to resume the call with specific call id
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : std::shared_ptr<qcril::interfaces::SipErrorInfo>
 */
class QcRilRequestImsResumeMessage : public QcRilRequestMessage,
                                     public add_message_id<QcRilRequestImsResumeMessage> {
 private:
  std::optional<uint32_t> mCallId;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestImsResumeMessage";

  QcRilRequestImsResumeMessage() = delete;

  ~QcRilRequestImsResumeMessage() {}

  inline QcRilRequestImsResumeMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }
  bool hasCallId() { return mCallId ? true : false; }
  int getCallId() { return *mCallId; }
  void setCallId(int val) { mCallId = val; }
  inline string dumpCallId() {
    return "mCallId = " + (mCallId ? std::to_string(*mCallId) : "<invalid>");
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() +
          "{" + dumpCallId() + "}";
  }
};
