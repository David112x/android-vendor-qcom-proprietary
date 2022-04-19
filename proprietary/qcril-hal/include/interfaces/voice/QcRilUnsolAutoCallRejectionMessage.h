/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>
#include <interfaces/voice/voice.h>
#include <optional>

/*
 * Unsol message to notify clients about a MT call being automatically rejected by modem
 */
class QcRilUnsolAutoCallRejectionMessage
    : public UnSolicitedMessage,
      public add_message_id<QcRilUnsolAutoCallRejectionMessage> {
 private:
  std::optional<qcril::interfaces::CallType> mCallType;
  std::optional<qcril::interfaces::CallFailCause> mCallFailCause;
  std::optional<uint16_t> mSipErrorCode;
  std::optional<std::string> mNumber;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolAutoCallRejectionMessage";
  ~QcRilUnsolAutoCallRejectionMessage() {}

  QcRilUnsolAutoCallRejectionMessage() : UnSolicitedMessage(get_class_message_id()) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolAutoCallRejectionMessage> msg =
        std::make_shared<QcRilUnsolAutoCallRejectionMessage>();
    if (msg) {
      if (mCallType) {
        msg->setCallType(*mCallType);
      }
      if (mCallFailCause) {
        msg->setCallFailCause(*mCallFailCause);
      }
      if (mSipErrorCode) {
        msg->setSipErrorCode(*mSipErrorCode);
      }
      if (mNumber) {
        msg->setNumber(*mNumber);
      }
    }
    return msg;
  }

  bool hasCallType() { return mCallType ? true : false; }
  qcril::interfaces::CallType getCallType() { return *mCallType; }
  void setCallType(qcril::interfaces::CallType val) { mCallType = val; }
  inline string dumpCallType() {
    return "mCallType = " + (mCallType ? toString(*mCallType) : "<invalid>");
  }

  bool hasCallFailCause() { return mCallFailCause ? true : false; }
  qcril::interfaces::CallFailCause getCallFailCause() { return *mCallFailCause; }
  void setCallFailCause(qcril::interfaces::CallFailCause val) { mCallFailCause = val; }
  inline string dumpCallFailCause() {
    return "mCallFailCause = " + (mCallFailCause ? toString(*mCallFailCause) : "<invalid>");
  }

  bool hasSipErrorCode() { return mSipErrorCode ? true : false; }
  uint16_t getSipErrorCode() { return *mSipErrorCode; }
  void setSipErrorCode(uint16_t val) { mSipErrorCode = val; }
  inline string dumpSipErrorCode() {
    return "mSipErrorCode = " + (mSipErrorCode ? std::to_string(*mSipErrorCode) : "<invalid>");
  }

  bool hasNumber() { return mNumber ? true : false; }
  std::string getNumber() { return *mNumber; }
  void setNumber(std::string val) { mNumber = val; }
  inline std::string dumpNumber() { return "mNumber = " + (mNumber ? *mNumber : "<invalid>"); }

  virtual string dump() {
    return mName +
          "{" + dumpCallType() + dumpCallFailCause() + dumpSipErrorCode() + dumpNumber() + "}";
  }
};
