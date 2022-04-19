/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once
#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"
#include "MessageCommon.h"

namespace rildata {

/********************** Class Definitions *************************/
class RadioConfigClientConnectedMessage: public UnSolicitedMessage,
                                         public add_message_id<RadioConfigClientConnectedMessage> {
public:
  static constexpr const char *MESSAGE_NAME = "RadioConfigClientConnectedMessage";

  RadioConfigClientConnectedMessage();
  ~RadioConfigClientConnectedMessage();

  std::shared_ptr<UnSolicitedMessage> clone();
  string dump();
};

} //namespace
