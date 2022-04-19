/******************************************************************************
#  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>
#include <interfaces/sms/sms.h>
#include <optional>

/*
 * Unsol message to notify clients about conference information
 */
class QcRilUnsolMessageWaitingInfoMessage
    : public UnSolicitedMessage,
      public add_message_id<QcRilUnsolMessageWaitingInfoMessage> {
 private:
  std::optional<std::vector<qcril::interfaces::MessageSummary>> mMessageSummary;
  std::optional<std::string> mUeAddress;
  std::optional<std::vector<qcril::interfaces::MessageDetails>> mMessageDetails;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolMessageWaitingInfoMessage";
  ~QcRilUnsolMessageWaitingInfoMessage() {}

  QcRilUnsolMessageWaitingInfoMessage() : UnSolicitedMessage(get_class_message_id()) {}

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolMessageWaitingInfoMessage> msg =
        std::make_shared<QcRilUnsolMessageWaitingInfoMessage>();
#if 0
    if (msg) {
      if (mMessageSummary) {
        msg->setMessageSummary(*mMessageSummary);
      }
      if (mUeAddress) {
        msg->setUeAddress(*mUeAddress);
      }
      if (mMessageDetails) {
        msg->setMessageDetails(*mMessageDetails);
      }
    }
#endif
    return msg;
  }

  bool hasMessageSummary() { return mMessageSummary ? true : false; }
  std::vector<qcril::interfaces::MessageSummary> getMessageSummary() { return *mMessageSummary; }
  void setMessageSummary(std::vector<qcril::interfaces::MessageSummary> val) {
    mMessageSummary = val;
  }
#if 0
  inline string dumpMessageSummary() {
    return " mMessageSummary = " + (mMessageSummary ? *mMessageSummary : "<invalid>");
  }
#endif

  bool hasUeAddress() { return mUeAddress ? true : false; }
  std::string getUeAddress() { return *mUeAddress; }
  void setUeAddress(std::string val) { mUeAddress = val; }
  inline string dumpUeAddress() {
    return " mUeAddress = " + (mUeAddress ? (*mUeAddress) : "<invalid>");
  }

  bool hasMessageDetails() { return mMessageDetails ? true : false; }
  std::vector<qcril::interfaces::MessageDetails> getMessageDetails() { return *mMessageDetails; }
  void setMessageDetails(std::vector<qcril::interfaces::MessageDetails> val) {
    mMessageDetails = val;
  }
#if 0
  inline string dumpMessageDetails() {
    return " mMessageDetails = " + (mMessageDetails ? *mMessageDetails : "<invalid>");
  }
#endif

  string dump() { return mName + "{" + dumpUeAddress() + "}"; }
};
