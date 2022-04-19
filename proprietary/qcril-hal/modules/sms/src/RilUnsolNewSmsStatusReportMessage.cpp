/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilUnsolNewSmsStatusReportMessage.h"

RilUnsolNewSmsStatusReportMessage::~RilUnsolNewSmsStatusReportMessage() {
}

std::shared_ptr<UnSolicitedMessage> RilUnsolNewSmsStatusReportMessage::clone() {
  return std::make_shared<RilUnsolNewSmsStatusReportMessage>(mPayload);
}

string RilUnsolNewSmsStatusReportMessage::dump() {
  return mName + ": payload length " + std::to_string(mPayload.size());
}

std::vector<uint8_t>& RilUnsolNewSmsStatusReportMessage::getPayload() {
  return mPayload;
}

