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

enum class DDSTimeOutSwitchType : int {
    DDSTimeOutSwitch = 0,
    TempDDSTimeOutSwitch,
};

/********************** Class Definitions *************************/
class DDSSwitchTimeoutMessage: public UnSolicitedMessage,
                               public add_message_id<DDSSwitchTimeoutMessage> {
public:
  static constexpr const char *MESSAGE_NAME = "DDSSwitchTimeoutMessage";

  DDSSwitchTimeoutMessage(DDSTimeOutSwitchType temp);
  ~DDSSwitchTimeoutMessage();

  DDSTimeOutSwitchType getType();
  std::shared_ptr<UnSolicitedMessage> clone();
  string dump();

private:
  DDSTimeOutSwitchType mType;
};

} //namespace
