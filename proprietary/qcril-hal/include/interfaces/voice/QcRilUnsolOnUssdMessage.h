/******************************************************************************
#  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>
#include <interfaces/voice/voice.h>
#include <string.h>
#include <optional>

/*
 * Unsol message to notify USSD
 */
class QcRilUnsolOnUssdMessage : public UnSolicitedMessage,
                                public add_message_id<QcRilUnsolOnUssdMessage> {
 private:
  bool mIsImsUpdate = false;
  std::optional<qcril::interfaces::UssdModeType> mMode;
  std::optional<std::string> mMessage;
  std::shared_ptr<qcril::interfaces::SipErrorInfo> mErrorDetails = nullptr;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolOnUssdMessage";
  ~QcRilUnsolOnUssdMessage() {}

  QcRilUnsolOnUssdMessage() : UnSolicitedMessage(get_class_message_id()) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolOnUssdMessage> msg = std::make_shared<QcRilUnsolOnUssdMessage>();
    if (msg) {
      msg->setIsImsUpdate(mIsImsUpdate);
      if (mMode) {
        msg->setMode(*mMode);
      }
      if (mMessage) {
        msg->setMessage(*mMessage);
      }
      if (mErrorDetails) {
        msg->setErrorDetails(mErrorDetails);
      }
    }
    return msg;
  }

  bool isImsUpdate() { return mIsImsUpdate; }
  void setIsImsUpdate(bool val) { mIsImsUpdate = val; }
  inline string dumpIsImsUpdate() {
    return std::string(" mIsImsUpdate = ") + (mIsImsUpdate ? "true" : "false");
  }

  bool hasMode() { return mMode ? true : false; }
  qcril::interfaces::UssdModeType getMode() { return *mMode; }
  void setMode(qcril::interfaces::UssdModeType val) { mMode = val; }
  inline std::string dumpMode() { return " mMode = " + (mMode ? toString(*mMode) : "<invalid>"); }

  bool hasMessage() { return mMessage ? true : false; }
  std::string getMessage() { return *mMessage; }
  void setMessage(std::string val) { mMessage = val; }
  inline std::string dumpMessage() { return " mMessage = " + (mMessage ? *mMessage : "<invalid>"); }

  bool hasErrorDetails() { return mErrorDetails ? true : false; }
  std::shared_ptr<qcril::interfaces::SipErrorInfo> getErrorDetails() { return mErrorDetails; }
  void setErrorDetails(std::shared_ptr<qcril::interfaces::SipErrorInfo> val)
      { mErrorDetails = val; }

  std::string dump() { return mName + "{" + dumpMode() + dumpMessage() + dumpIsImsUpdate() + "}"; }
};
