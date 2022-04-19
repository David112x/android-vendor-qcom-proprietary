/******************************************************************************
#  Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include "UnSolMessages/RilPCODataMessage.h"

using namespace rildata;

RilPCODataMessage::RilPCODataMessage
(
  const PcoDataInfo_t &pcoData
) : UnSolicitedMessage(get_class_message_id())
{
  mName = MESSAGE_NAME;
  mPcoData = pcoData;
}

RilPCODataMessage::~RilPCODataMessage()
{
}

std::shared_ptr<UnSolicitedMessage> RilPCODataMessage::clone()
{
  return std::make_shared<RilPCODataMessage>(mPcoData);
}
