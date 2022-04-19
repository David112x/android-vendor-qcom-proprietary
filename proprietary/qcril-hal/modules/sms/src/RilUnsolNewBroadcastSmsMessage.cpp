/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilUnsolNewBroadcastSmsMessage.h"

RilUnsolNewBroadcastSmsMessage::~RilUnsolNewBroadcastSmsMessage() {
}

std::shared_ptr<UnSolicitedMessage> RilUnsolNewBroadcastSmsMessage::clone() {
  return std::make_shared<RilUnsolNewBroadcastSmsMessage>(mPayload);
}

string RilUnsolNewBroadcastSmsMessage::dump() {
  return mName + ": payload length " + std::to_string(mPayload.size());
}

std::vector<uint8_t>& RilUnsolNewBroadcastSmsMessage::getPayload() {
  return mPayload;
}

