/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>

/*
 * Unsol message to notify network state change
 *
 */
class RilUnsolNetworkStateChangedMessage : public UnSolicitedMessage,
            public add_message_id<RilUnsolNetworkStateChangedMessage> {

 public:
  static constexpr const char *MESSAGE_NAME = "RIL_UNSOL_RESPONSE_NETWORK_STATE_CHANGED";
  ~RilUnsolNetworkStateChangedMessage() {}

  explicit inline RilUnsolNetworkStateChangedMessage()
        : UnSolicitedMessage(get_class_message_id()) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    return std::make_shared<RilUnsolNetworkStateChangedMessage>();
  }

  string dump() {
    return RilUnsolNetworkStateChangedMessage::MESSAGE_NAME;
  }
};
