/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"
#include "MessageCommon.h"

namespace rildata {

enum class KeepAliveResult_t : int{
  KEEP_ALIVE_RESULT_OK = 0,
  KEEP_ALIVE_RESULT_ERROR = 1,
  KEEP_ALIVE_NETWORK_DOWN_ERROR = 2,
};

struct KeepAliveInfo_t {
  KeepAliveResult_t status;
  uint32_t handle;
};

/********************** Class Definitions *************************/
class KeepAliveIndMessage: public UnSolicitedMessage,
                           public add_message_id<KeepAliveIndMessage> {
private:
  KeepAliveInfo_t mKeepalive;

public:
  static constexpr const char *MESSAGE_NAME = "KeepAliveIndMessage";
  KeepAliveIndMessage() = delete;
  KeepAliveIndMessage(KeepAliveInfo_t keepalive);
  ~KeepAliveIndMessage();

  std::shared_ptr<UnSolicitedMessage> clone();
  string dump();

  KeepAliveResult_t get_status();
  uint32_t get_handle();
};
}
