/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include "UnSolMessages/DataBearerTypeChangedMessage.h"

using namespace rildata;

DataBearerTypeChangedMessage::DataBearerTypeChangedMessage
(
  int32_t cid,
  const BearerInfo_t &bearerInfo
) : UnSolicitedMessage(get_class_message_id())
{
  mName = MESSAGE_NAME;
  mCid = cid;
  mBearerInfo = bearerInfo;
}

DataBearerTypeChangedMessage::~DataBearerTypeChangedMessage()
{
}

const BearerInfo_t & DataBearerTypeChangedMessage::getBearerInfo()
{
  return mBearerInfo;
}

std::shared_ptr<UnSolicitedMessage> DataBearerTypeChangedMessage::clone()
{
  return std::make_shared<DataBearerTypeChangedMessage>(mCid, mBearerInfo);
}

int32_t DataBearerTypeChangedMessage::getCid()
{
  return mCid;
}

string DataBearerTypeChangedMessage::dump()
{
  return MESSAGE_NAME;
}
