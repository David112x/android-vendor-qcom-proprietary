/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>

/*
 * Unsol message to notify NITZ time
 *
 */
class RilUnsolNitzTimeReceivedMessage : public UnSolicitedMessage,
            public add_message_id<RilUnsolNitzTimeReceivedMessage> {
 private:
  std::string mNitzTime;

 public:
  static constexpr const char *MESSAGE_NAME = "RIL_UNSOL_NITZ_TIME_RECEIVED";
  ~RilUnsolNitzTimeReceivedMessage() { }

  RilUnsolNitzTimeReceivedMessage(const char* time)
      : UnSolicitedMessage(get_class_message_id()) {
    mNitzTime = time;
  }
  template<typename T>
  RilUnsolNitzTimeReceivedMessage(T time)
      : UnSolicitedMessage(get_class_message_id()),
        mNitzTime(std::forward<T>(time)) { }

  std::shared_ptr<UnSolicitedMessage> clone() {
    return std::make_shared<RilUnsolNitzTimeReceivedMessage>(mNitzTime);
  }

  const std::string& getNitzTime() { return mNitzTime; }

  string dump() {
    return RilUnsolNitzTimeReceivedMessage::MESSAGE_NAME;
  }
};
