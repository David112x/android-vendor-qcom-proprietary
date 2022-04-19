/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>

/*
 * Unsol message to notify restricted state change
 *
 */
class RilUnsolRestrictedStateChangedMessage : public UnSolicitedMessage,
            public add_message_id<RilUnsolRestrictedStateChangedMessage> {
 public:
  static constexpr const char *MESSAGE_NAME = "RIL_UNSOL_RESTRICTED_STATE_CHANGED";
  ~RilUnsolRestrictedStateChangedMessage() { }

  RilUnsolRestrictedStateChangedMessage(int32_t state)
      : UnSolicitedMessage(get_class_message_id()), mState(state) {}

  std::shared_ptr<UnSolicitedMessage> clone() {
    return std::make_shared<RilUnsolRestrictedStateChangedMessage>(mState);
  }

  int32_t getState() { return mState; }

  string dump() {
    return RilUnsolRestrictedStateChangedMessage::MESSAGE_NAME;
  }
 private:
  int32_t mState;
};
