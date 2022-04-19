/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include "UnSolMessages/CleanDataCallStateMessage.h"

CleanDataCallStateMessage::CleanDataCallStateMessage(int setCid):
      UnSolicitedMessage(get_class_message_id()), cid(setCid)
{
}

CleanDataCallStateMessage::~CleanDataCallStateMessage()
{
}

int CleanDataCallStateMessage::getcid()
{
  return cid;
}

void CleanDataCallStateMessage::setcid(int setCid)
{
  cid = setCid;
}

std::shared_ptr<UnSolicitedMessage> CleanDataCallStateMessage::clone()
{
  return std::make_shared<CleanDataCallStateMessage>(cid);
}

string CleanDataCallStateMessage::dump()
{
  return MESSAGE_NAME;
}
