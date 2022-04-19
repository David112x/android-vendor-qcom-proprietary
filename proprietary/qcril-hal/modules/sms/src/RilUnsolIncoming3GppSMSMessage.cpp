/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilUnsolIncoming3GppSMSMessage.h"

RilUnsolIncoming3GppSMSMessage::~RilUnsolIncoming3GppSMSMessage() {
}

std::shared_ptr<UnSolicitedMessage> RilUnsolIncoming3GppSMSMessage::clone() {
  return std::make_shared<RilUnsolIncoming3GppSMSMessage>(mPayload);
}

string RilUnsolIncoming3GppSMSMessage::dump() {
  return mName + ": payload length " + std::to_string(mPayload.size());
}

std::vector<uint8_t>& RilUnsolIncoming3GppSMSMessage::getPayload() {
  return mPayload;
}

