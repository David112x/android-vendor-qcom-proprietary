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
 * Request to set call barring password
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : std::shared_ptr<qcril::interfaces::SipErrorInfo>
 */
class QcRilRequestSetCallBarringPasswordMessage
    : public QcRilRequestMessage,
      public add_message_id<QcRilRequestSetCallBarringPasswordMessage> {
 private:
  std::optional<qcril::interfaces::FacilityType> mFacilityType;
  std::optional<std::string> mOldPassword;
  std::optional<std::string> mNewPassword;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestSetCallBarringPasswordMessage";

  QcRilRequestSetCallBarringPasswordMessage() = delete;

  ~QcRilRequestSetCallBarringPasswordMessage() {}

  inline QcRilRequestSetCallBarringPasswordMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  bool hasFacilityType() { return mFacilityType ? true : false; }
  qcril::interfaces::FacilityType getFacilityType() { return *mFacilityType; }
  void setFacilityType(qcril::interfaces::FacilityType val) { mFacilityType = val; }
  inline string dumpFacilityType() {
    return "mFacilityType = " + (mFacilityType ? toString(*mFacilityType) : "<invalid>");
  }

  bool hasOldPassword() { return mOldPassword ? true : false; }
  std::string getOldPassword() { return *mOldPassword; }
  void setOldPassword(std::string val) { mOldPassword = val; }
  inline string dumpOldPassword() {
    return "mOldPassword = " + (mOldPassword ? *mOldPassword : "<invalid>");
  }

  bool hasNewPassword() { return mNewPassword ? true : false; }
  std::string getNewPassword() { return *mNewPassword; }
  void setNewPassword(std::string val) { mNewPassword = val; }
  inline string dumpNewPassword() {
    return "mNewPassword = " + (mNewPassword ? *mNewPassword : "<invalid>");
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() +
          "{" + dumpFacilityType() + dumpOldPassword() + dumpNewPassword() + "}";
  }
};
