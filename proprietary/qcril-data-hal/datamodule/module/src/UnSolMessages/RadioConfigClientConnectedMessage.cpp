/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "UnSolMessages/RadioConfigClientConnectedMessage.h"

using namespace rildata;

RadioConfigClientConnectedMessage::RadioConfigClientConnectedMessage():
    UnSolicitedMessage(get_class_message_id())
{
}

RadioConfigClientConnectedMessage::~RadioConfigClientConnectedMessage()
{
}

std::shared_ptr<UnSolicitedMessage> RadioConfigClientConnectedMessage::clone()
{
  return std::make_shared<RadioConfigClientConnectedMessage>();
}

string RadioConfigClientConnectedMessage::dump()
{
  return MESSAGE_NAME;
}
