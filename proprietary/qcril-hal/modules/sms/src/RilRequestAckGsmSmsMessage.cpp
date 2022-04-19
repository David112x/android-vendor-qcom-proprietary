/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilRequestAckGsmSmsMessage.h"
#include "telephony/ril.h"

RilRequestAckGsmSmsMessage::~RilRequestAckGsmSmsMessage() {
}

string RilRequestAckGsmSmsMessage::dump() {
  return mName + std::string(" { success = ") + std::string(
        mSuccess ? "true" : "false") + std::string(" { cause = ")
        + std::to_string(mCause) + std::string(" }");
}

bool RilRequestAckGsmSmsMessage::isSuccess() {
  return mSuccess;
}

int32_t RilRequestAckGsmSmsMessage::getCause() {
  return mCause;
}
