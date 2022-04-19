/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/add_message_id.h>
#include <interfaces/QcRilRequestMessage.h>

/*
 * Request to upgrade/downgrade existing IMS calls
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : nullptr
 */
class QcRilRequestImsModifyCallInitiateMessage
    : public QcRilRequestMessage,
      public add_message_id<QcRilRequestImsModifyCallInitiateMessage> {
 private:
  std::optional<uint32_t> mCallId;
  std::optional<qcril::interfaces::CallType> mCallType;
  std::optional<qcril::interfaces::CallDomain> mCallDomain;
  std::optional<qcril::interfaces::RttMode> mRttMode;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestImsModifyCallInitiateMessage";

  QcRilRequestImsModifyCallInitiateMessage() = delete;

  ~QcRilRequestImsModifyCallInitiateMessage() {}

  inline QcRilRequestImsModifyCallInitiateMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  bool hasCallId() { return mCallId ? true : false; }
  int getCallId() { return *mCallId; }
  void setCallId(int val) { mCallId = val; }
  inline string dumpCallId() { return "mCallId = " + (mCallId ? std::to_string(*mCallId) : "<invalid>"); }

  bool hasCallType() { return mCallType ? true : false; }
  qcril::interfaces::CallType getCallType() { return *mCallType; }
  void setCallType(qcril::interfaces::CallType val) { mCallType = val; }
  inline string dumpCallType() {
    return "mCallType = " + (mCallType ? toString(*mCallType) : "<invalid>");
  }

  bool hasCallDomain() { return mCallDomain ? true : false; }
  qcril::interfaces::CallDomain getCallDomain() { return *mCallDomain; }
  void setCallDomain(qcril::interfaces::CallDomain val) { mCallDomain = val; }
  inline string dumpCallDomain() {
    return "mCallDomain = " + (mCallDomain ? toString(*mCallDomain) : "<invalid>");
  }

  bool hasRttMode() { return mRttMode ? true : false; }
  qcril::interfaces::RttMode getRttMode() { return *mRttMode; }
  void setRttMode(qcril::interfaces::RttMode val) { mRttMode = val; }
  inline string dumpRttMode() {
    return "mRttMode = " + (mRttMode ? toString(*mRttMode) : "<invalid>");
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() +
          "{" + dumpCallId() + dumpCallType() + dumpCallDomain() + dumpRttMode() + "}";
  }
};
