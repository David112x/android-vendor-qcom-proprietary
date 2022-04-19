/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/add_message_id.h>
#include <interfaces/QcRilRequestMessage.h>
#include <optional>

/*
 * Request to set local call hold (only applicable for DSDA).
 *
 * Response:
 *   errorCode    : Valid error codes
 *                   SUCCESS
 *                   GENERIC_FAILURE
 *   responseData : nullptr
 */
class QcRilRequestSetLocalCallHoldMessage
    : public QcRilRequestMessage,
      public add_message_id<QcRilRequestSetLocalCallHoldMessage> {
 public:
  enum class LocalHoldType {
    UNKNOWN,
    HOLD,
    UNHOLD,
  };

 private:
  bool mAudioUpdateCompleted;
  std::optional<LocalHoldType> mLocalHoldType;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestSetLocalCallHoldMessage";

  QcRilRequestSetLocalCallHoldMessage() = delete;

  ~QcRilRequestSetLocalCallHoldMessage() {}

  inline QcRilRequestSetLocalCallHoldMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context), mAudioUpdateCompleted(false) {
    mName = MESSAGE_NAME;
  }

  bool hasLocalHoldType() { return mLocalHoldType ? true : false; }
  LocalHoldType getLocalHoldType() { return *mLocalHoldType; }
  void setLocalHoldType(LocalHoldType val) { mLocalHoldType = val; }
  inline string dumpLocalHoldType() {
    return std::string(" mLocalHoldType = ") +
           (mLocalHoldType ? (*mLocalHoldType == LocalHoldType::HOLD ? "HOLD" : "UNHOLD")
                           : "<invalid>");
  }

  virtual string dump() { return QcRilRequestMessage::dump() + "{" + dumpLocalHoldType() + "}"; }

  bool isAudioStateUpdateCompleted() { return mAudioUpdateCompleted; }
  void setAudioStateUpdateCompleted(bool status) { mAudioUpdateCompleted = status; }
};
