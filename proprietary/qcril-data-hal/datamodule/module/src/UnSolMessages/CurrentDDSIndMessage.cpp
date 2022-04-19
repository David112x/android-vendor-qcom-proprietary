/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "UnSolMessages/CurrentDDSIndMessage.h"

using namespace rildata;

CurrentDDSIndMessage::CurrentDDSIndMessage(int32_t setSubId, dsd_dds_switch_type setSwitch):
    UnSolicitedMessage(get_class_message_id()), subId(setSubId), switchType(setSwitch)
{
}

CurrentDDSIndMessage::~CurrentDDSIndMessage()
{
}

int32_t CurrentDDSIndMessage::getSubId()
{
  return subId;
}

dsd_dds_switch_type CurrentDDSIndMessage::getSwitchType()
{
  return switchType;
}

std::shared_ptr<UnSolicitedMessage> CurrentDDSIndMessage::clone()
{
  return std::make_shared<CurrentDDSIndMessage>(subId, switchType);
}

string CurrentDDSIndMessage::dump()
{
  return MESSAGE_NAME;
}
