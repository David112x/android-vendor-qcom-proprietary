/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilUnsolNewSmsOnSimMessage.h"

RilUnsolNewSmsOnSimMessage::~RilUnsolNewSmsOnSimMessage() {
}

std::shared_ptr<UnSolicitedMessage> RilUnsolNewSmsOnSimMessage::clone() {
  return std::make_shared<RilUnsolNewSmsOnSimMessage>(mRecordNumber);
}

string RilUnsolNewSmsOnSimMessage::dump() {
  return mName + "{ record number = " + std::to_string(mRecordNumber) +
        " }";
}

int32_t RilUnsolNewSmsOnSimMessage::getRecordNumber() {
  return mRecordNumber;
}
