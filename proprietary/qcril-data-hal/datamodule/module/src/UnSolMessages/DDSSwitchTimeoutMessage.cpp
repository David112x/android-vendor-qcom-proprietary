/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "UnSolMessages/DDSSwitchTimeoutMessage.h"

using namespace rildata;

DDSSwitchTimeoutMessage::DDSSwitchTimeoutMessage(DDSTimeOutSwitchType temp):
    UnSolicitedMessage(get_class_message_id())
{
  mType = temp;
}

DDSSwitchTimeoutMessage::~DDSSwitchTimeoutMessage()
{
}

std::shared_ptr<UnSolicitedMessage> DDSSwitchTimeoutMessage::clone()
{
  return std::make_shared<DDSSwitchTimeoutMessage>(mType);
}

string DDSSwitchTimeoutMessage::dump()
{
  return MESSAGE_NAME;
}

DDSTimeOutSwitchType DDSSwitchTimeoutMessage::getType()
{
	return mType;
}