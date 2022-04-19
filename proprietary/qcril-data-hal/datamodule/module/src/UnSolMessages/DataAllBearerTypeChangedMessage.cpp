/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include "UnSolMessages/DataAllBearerTypeChangedMessage.h"

using namespace rildata;

DataAllBearerTypeChangedMessage::DataAllBearerTypeChangedMessage
(
  const AllocatedBearer_t &bearerInfo
) : UnSolicitedMessage(get_class_message_id())
{
  mName = MESSAGE_NAME;
  mBearerInfo = bearerInfo;
}

DataAllBearerTypeChangedMessage::~DataAllBearerTypeChangedMessage()
{
}

const AllocatedBearer_t & DataAllBearerTypeChangedMessage::getBearerInfo()
{
  return mBearerInfo;
}

std::shared_ptr<UnSolicitedMessage> DataAllBearerTypeChangedMessage::clone()
{
  return std::make_shared<DataAllBearerTypeChangedMessage>(mBearerInfo);
}

string DataAllBearerTypeChangedMessage::dump()
{
  return MESSAGE_NAME;
}
