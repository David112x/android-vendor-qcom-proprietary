/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>

/*
 * Unsol message to notify radio state change
 *
 */
class RilUnsolRadioStateChangedMessage : public UnSolicitedMessage,
            public add_message_id<RilUnsolRadioStateChangedMessage> {

 public:
  static constexpr const char *MESSAGE_NAME = "RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED";
  ~RilUnsolRadioStateChangedMessage() {}

  explicit inline RilUnsolRadioStateChangedMessage()
        : UnSolicitedMessage(get_class_message_id()) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    return std::make_shared<RilUnsolRadioStateChangedMessage>();
  }

  string dump() {
    return RilUnsolRadioStateChangedMessage::MESSAGE_NAME;
  }
};
