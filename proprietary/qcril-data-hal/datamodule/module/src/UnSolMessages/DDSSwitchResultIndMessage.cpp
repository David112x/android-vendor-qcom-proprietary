/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "UnSolMessages/DDSSwitchResultIndMessage.h"

using namespace rildata;

DDSSwitchResultIndMessage::DDSSwitchResultIndMessage():
    UnSolicitedMessage(get_class_message_id()), allowed(false), failed(false), error(TriggerDDSSwitchError::NO_ERROR)
{
}

DDSSwitchResultIndMessage::~DDSSwitchResultIndMessage()
{
}

bool DDSSwitchResultIndMessage::getAllowed()
{
  return allowed;
}

void DDSSwitchResultIndMessage::setAllowed(bool setAllowed)
{
  allowed = setAllowed;
}

bool DDSSwitchResultIndMessage::getFailed()
{
  return failed;
}

void DDSSwitchResultIndMessage::setFailed(bool setFailed)
{
  failed = setFailed;
}

TriggerDDSSwitchError DDSSwitchResultIndMessage::getError()
{
  return error;
}

void DDSSwitchResultIndMessage::setError(TriggerDDSSwitchError setError)
{
  error = setError;
}

std::shared_ptr<UnSolicitedMessage> DDSSwitchResultIndMessage::clone()
{
  return std::make_shared<DDSSwitchResultIndMessage>();
}

string DDSSwitchResultIndMessage::dump()
{
  return MESSAGE_NAME;
}
