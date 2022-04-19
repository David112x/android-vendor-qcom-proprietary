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
 * Unsol message to notify that an upgrade of a call was triggered from the remote party
 */
class QcRilUnsolImsModifyCallMessage : public UnSolicitedMessage,
                                       public add_message_id<QcRilUnsolImsModifyCallMessage> {
 private:
  std::optional<uint32_t> mCallIndex;
  std::optional<qcril::interfaces::CallType> mCallType;
  std::optional<qcril::interfaces::CallDomain> mCallDomain;
  std::optional<qcril::interfaces::RttMode> mRttMode;
  std::optional<qcril::interfaces::CallModifyFailCause> mCallModifyFailCause;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolImsModifyCallMessage";
  ~QcRilUnsolImsModifyCallMessage() {}

  QcRilUnsolImsModifyCallMessage() : UnSolicitedMessage(get_class_message_id()) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolImsModifyCallMessage> msg =
        std::make_shared<QcRilUnsolImsModifyCallMessage>();
    if (msg) {
      if (mCallIndex) {
        msg->setCallIndex(*mCallIndex);
      }
      if (mCallType) {
        msg->setCallType(*mCallType);
      }
      if (mCallDomain) {
        msg->setCallDomain(*mCallDomain);
      }
      if (mRttMode) {
        msg->setRttMode(*mRttMode);
      }
    }
    return msg;
  }

  bool hasCallIndex() { return mCallIndex ? true : false; }
  uint32_t getCallIndex() { return *mCallIndex; }
  void setCallIndex(uint32_t val) { mCallIndex = val; }
  inline std::string dumpCallIndex() {
    return " mCallIndex = " + (mCallIndex ? std::to_string(*mCallIndex) : "<invalid>");
  }

  bool hasCallType() { return mCallType ? true : false; }
  qcril::interfaces::CallType getCallType() { return *mCallType; }
  void setCallType(qcril::interfaces::CallType val) { mCallType = val; }
  inline std::string dumpCallType() {
    return " mCallType = " + (mCallType ? toString(*mCallType) : "<invalid>");
  }

  bool hasCallDomain() { return mCallDomain ? true : false; }
  qcril::interfaces::CallDomain getCallDomain() { return *mCallDomain; }
  void setCallDomain(qcril::interfaces::CallDomain val) { mCallDomain = val; }
  inline std::string dumpCallDomain() {
    return " mCallDomain = " + (mCallDomain ? toString(*mCallDomain) : "<invalid>");
  }

  bool hasRttMode() { return mRttMode ? true : false; }
  qcril::interfaces::RttMode getRttMode() { return *mRttMode; }
  void setRttMode(qcril::interfaces::RttMode val) { mRttMode = val; }
  inline std::string dumpRttMode() {
    return " mRttMode = " + (mRttMode ? toString(*mRttMode) : "<invalid>");
  }

  bool hasCallModifyFailCause() { return mCallModifyFailCause ? true : false; }
  qcril::interfaces::CallModifyFailCause getCallModifyFailCause() { return *mCallModifyFailCause; }
  void setCallModifyFailCause(qcril::interfaces::CallModifyFailCause val) {
    mCallModifyFailCause = val;
  }
  inline std::string dumpCallModifyFailCause() {
    return " mCallModifyFailCause = " +
           (mCallModifyFailCause ? toString(*mCallModifyFailCause) : "<invalid>");
  }

  string dump() {
    return mName + "{" + dumpCallIndex() + dumpCallType() + dumpCallDomain() + dumpRttMode() +
           dumpCallModifyFailCause() + "}";
  }
};
