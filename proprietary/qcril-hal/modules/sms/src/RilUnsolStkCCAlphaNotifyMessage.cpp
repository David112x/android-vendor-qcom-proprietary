/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilUnsolStkCCAlphaNotifyMessage.h"

RilUnsolStkCCAlphaNotifyMessage::~RilUnsolStkCCAlphaNotifyMessage() {
}

std::shared_ptr<UnSolicitedMessage> RilUnsolStkCCAlphaNotifyMessage::clone() {
  return std::make_shared<RilUnsolStkCCAlphaNotifyMessage>(mPayload);
}

string RilUnsolStkCCAlphaNotifyMessage::dump() {
  return mName + ": payload length " + std::to_string(mPayload.size());
}

const std::string& RilUnsolStkCCAlphaNotifyMessage::getPayload() {
  return mPayload;
}

