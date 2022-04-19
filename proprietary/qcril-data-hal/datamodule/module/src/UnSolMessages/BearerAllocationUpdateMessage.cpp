/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "UnSolMessages/BearerAllocationUpdateMessage.h"

using namespace rildata;

BearerAllocationUpdateMessage::BearerAllocationUpdateMessage
(
  std::shared_ptr<AllocatedBearerResult_t> bearers
) : UnSolicitedMessage(get_class_message_id()),
    mBearers(bearers)
{
}

BearerAllocationUpdateMessage::~BearerAllocationUpdateMessage()
{
}

std::shared_ptr<AllocatedBearerResult_t> BearerAllocationUpdateMessage::getBearers()
{
  return mBearers;
}

std::shared_ptr<UnSolicitedMessage> BearerAllocationUpdateMessage::clone()
{
  return std::make_shared<BearerAllocationUpdateMessage>(mBearers);
}

string BearerAllocationUpdateMessage::dump()
{
  return MESSAGE_NAME;
}
