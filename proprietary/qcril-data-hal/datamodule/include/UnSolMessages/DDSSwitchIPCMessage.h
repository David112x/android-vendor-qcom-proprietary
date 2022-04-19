/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once
#include "framework/Message.h"
#include "qtibus/IPCMessage.h"
#include "framework/add_message_id.h"
#include "MessageCommon.h"

namespace rildata {

enum class DDSSwitchIPCMessageState : int {
    ApStarted = 0,
    MpStarted = 1,
    Completed = 2,
};

/********************** Class Definitions *************************/
class DDSSwitchIPCMessage: public IPCMessage,
                           public add_message_id<DDSSwitchIPCMessage> {

private:
  DDSSwitchIPCMessageState state;
  int subId;

public:
  static constexpr const char *MESSAGE_NAME = "DDSSwitchIPCMessage";

  DDSSwitchIPCMessage(DDSSwitchIPCMessageState setState, int setSubId);
  ~DDSSwitchIPCMessage();

  void serialize(IPCOStream &os) override;
  void deserialize(IPCIStream &is) override;

  DDSSwitchIPCMessageState getState();
  int getSubId();

  std::shared_ptr<UnSolicitedMessage> clone();
  string dump();
};

} //namespace
