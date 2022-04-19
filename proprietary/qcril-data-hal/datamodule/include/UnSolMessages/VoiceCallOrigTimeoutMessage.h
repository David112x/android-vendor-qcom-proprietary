/**
* Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#pragma once
#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"

/********************** Class Definitions *************************/
class VoiceCallOrigTimeoutMessage: public UnSolicitedMessage,
                           public add_message_id<VoiceCallOrigTimeoutMessage> {
public:
  static constexpr const char *MESSAGE_NAME = "VoiceCallOrigTimeoutMessage";

  VoiceCallOrigTimeoutMessage():UnSolicitedMessage(get_class_message_id()){}
  ~VoiceCallOrigTimeoutMessage() {}

  std::shared_ptr<UnSolicitedMessage> clone()
  {
    return std::make_shared<VoiceCallOrigTimeoutMessage>();
  }

  string dump()
  {
    return MESSAGE_NAME;
  }
};
