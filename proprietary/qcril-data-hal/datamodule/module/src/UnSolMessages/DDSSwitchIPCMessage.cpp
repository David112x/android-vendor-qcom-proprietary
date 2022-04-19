/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "UnSolMessages/DDSSwitchIPCMessage.h"

using namespace rildata;

DDSSwitchIPCMessage::DDSSwitchIPCMessage(DDSSwitchIPCMessageState setState,
                                         int setSubId):
  IPCMessage(get_class_message_id()), state(setState), subId(setSubId)
{}

DDSSwitchIPCMessage::~DDSSwitchIPCMessage() {}

void DDSSwitchIPCMessage::serialize(IPCOStream &os)
{
  os << static_cast<int>(state) << "," << subId;
}

void DDSSwitchIPCMessage::deserialize(IPCIStream &is)
{
  std::stringstream &iss = static_cast<std::stringstream&>(is);
  Log::getInstance().d("DDSSwitchIPCMessage deserialize iss=[" + iss.str() + "]");
  int inState;
  iss >> inState;
  iss.ignore(1, ',');
  iss >> subId;
  state = static_cast<DDSSwitchIPCMessageState>(inState);
}

DDSSwitchIPCMessageState DDSSwitchIPCMessage::getState()
{
  return state;
}

int DDSSwitchIPCMessage::getSubId()
{
  return subId;
}

std::shared_ptr<UnSolicitedMessage> DDSSwitchIPCMessage::clone()
{
  auto copy = std::make_shared<DDSSwitchIPCMessage>(state, subId);
  copy->setIsRemote(getIsRemote());
  return copy;
}

string DDSSwitchIPCMessage::dump()
{
  return std::string(MESSAGE_NAME) + " isRemote: " +
         std::to_string(getIsRemote()) + " subId: " + std::to_string(subId);
}
