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
 * Unsol message to notify that nework doesn't have in-band information, need to play out-band tone
 */
class QcRilUnsolRingbackToneMessage : public UnSolicitedMessage,
                                      public add_message_id<QcRilUnsolRingbackToneMessage> {
 private:
  std::optional<qcril::interfaces::RingBackToneOperation> mRingBackToneOperation;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolRingbackToneMessage";
  ~QcRilUnsolRingbackToneMessage() {}

  QcRilUnsolRingbackToneMessage() : UnSolicitedMessage(get_class_message_id()) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolRingbackToneMessage> msg =
        std::make_shared<QcRilUnsolRingbackToneMessage>();
    if (msg) {
      if (mRingBackToneOperation) {
        msg->setRingBackToneOperation(*mRingBackToneOperation);
      }
    }
    return msg;
  }

  bool hasRingBackToneOperation() { return mRingBackToneOperation ? true : false; }
  qcril::interfaces::RingBackToneOperation getRingBackToneOperation() {
    return *mRingBackToneOperation;
  }
  void setRingBackToneOperation(qcril::interfaces::RingBackToneOperation val) {
    mRingBackToneOperation = val;
  }
  inline std::string dumpRingBackToneOperation() {
    return " mRingBackToneOperation = " +
           (mRingBackToneOperation ? toString(*mRingBackToneOperation) : "<invalid>");
  }

  std::string dump() {
    return mName + "{" + (mIsIms ? "isIms=true" : "") + dumpRingBackToneOperation() + "}";
  }

  // To distinguish IMS or CS indication
 private:
  bool mIsIms = false;

 public:
  bool isIms() { return mIsIms; }
  void setIsIms(bool val) { mIsIms = val; }
};
