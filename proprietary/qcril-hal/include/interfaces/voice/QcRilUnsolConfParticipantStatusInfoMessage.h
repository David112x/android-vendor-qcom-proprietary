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
 * Unsol message to notify status of operations on a participant in a conference call
 */
class QcRilUnsolConfParticipantStatusInfoMessage
    : public UnSolicitedMessage,
      public add_message_id<QcRilUnsolConfParticipantStatusInfoMessage> {
 private:
  std::optional<uint32_t> mCallId;
  std::optional<qcril::interfaces::ConfParticipantOperation> mOperation;
  std::optional<uint32_t> mSipStatus;
  std::optional<std::string> mParticipantUri;
  std::optional<bool> mIsEct;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolConfParticipantStatusInfoMessage";
  ~QcRilUnsolConfParticipantStatusInfoMessage() {}

  QcRilUnsolConfParticipantStatusInfoMessage() : UnSolicitedMessage(get_class_message_id()) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolConfParticipantStatusInfoMessage> msg =
        std::make_shared<QcRilUnsolConfParticipantStatusInfoMessage>();
    if (msg) {
      if (mCallId) {
        msg->setCallId(*mCallId);
      }
      if (mOperation) {
        msg->setOperation(*mOperation);
      }
      if (mSipStatus) {
        msg->setSipStatus(*mSipStatus);
      }
      if (mParticipantUri) {
        msg->setParticipantUri(*mParticipantUri);
      }
      if (mIsEct) {
        msg->setIsEct(*mIsEct);
      }
    }
    return msg;
  }

  bool hasCallId() { return mCallId ? true : false; }
  uint32_t getCallId() { return *mCallId; }
  void setCallId(uint32_t val) { mCallId = val; }
  inline string dumpCallId() {
    return " mCallId = " + (mCallId ? std::to_string(*mCallId) : "<invalid>");
  }

  bool hasOperation() { return mOperation ? true : false; }
  qcril::interfaces::ConfParticipantOperation getOperation() { return *mOperation; }
  void setOperation(qcril::interfaces::ConfParticipantOperation val) { mOperation = val; }
  inline string dumpOperation() {
    return " mOperation = " + (mOperation ? toString(*mOperation) : "<invalid>");
  }

  bool hasSipStatus() { return mSipStatus ? true : false; }
  uint32_t getSipStatus() { return *mSipStatus; }
  void setSipStatus(uint32_t val) { mSipStatus = val; }
  inline string dumpSipStatus() {
    return " mSipStatus = " + (mSipStatus ? std::to_string(*mSipStatus) : "<invalid>");
  }

  bool hasParticipantUri() { return mParticipantUri ? true : false; }
  string getParticipantUri() { return *mParticipantUri; }
  void setParticipantUri(string val) { mParticipantUri = val; }
  inline string dumpParticipantUri() {
    return " mParticipantUri = " + (mParticipantUri ? *mParticipantUri : "<invalid>");
  }

  bool hasIsEct() { return mIsEct ? true : false; }
  uint32_t getIsEct() { return *mIsEct; }
  void setIsEct(uint32_t val) { mIsEct = val; }
  inline string dumpIsEct() {
    return " mIsEct = " + (mIsEct ? std::to_string(*mIsEct) : "<invalid>");
  }

  std::string dump() {
    return mName +
          "{" + dumpCallId() + dumpOperation() + dumpSipStatus() + dumpParticipantUri() +
                dumpIsEct() + "}";
  }
};
