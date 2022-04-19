/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/add_message_id.h>
#include <interfaces/QcRilRequestMessage.h>

/*
 * Request to cancel call upgrade from voice to video for a specific call id
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : nullptr
 */
class QcRilRequestImsCancelModifyCallMessage
    : public QcRilRequestMessage,
      public add_message_id<QcRilRequestImsCancelModifyCallMessage> {
 private:
  std::optional<uint32_t> mCallId;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestImsCancelModifyCallMessage";

  QcRilRequestImsCancelModifyCallMessage() = delete;

  ~QcRilRequestImsCancelModifyCallMessage() {}

  inline QcRilRequestImsCancelModifyCallMessage(std::shared_ptr<MessageContext> context)
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
