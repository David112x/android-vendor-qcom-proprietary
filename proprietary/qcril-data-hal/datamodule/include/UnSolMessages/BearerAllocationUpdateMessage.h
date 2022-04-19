/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
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
class BearerAllocationUpdateMessage: public UnSolicitedMessage,
                                     public add_message_id<BearerAllocationUpdateMessage> {

private:
  std::shared_ptr<AllocatedBearerResult_t> mBearers;

public:
  static constexpr const char *MESSAGE_NAME = "BearerAllocationUpdateMessage";

  BearerAllocationUpdateMessage() = delete;
  ~BearerAllocationUpdateMessage();
  BearerAllocationUpdateMessage(std::shared_ptr<AllocatedBearerResult_t>);

  std::shared_ptr<UnSolicitedMessage> clone();
  std::shared_ptr<AllocatedBearerResult_t> getBearers();
  string dump();
};

} //namespace
