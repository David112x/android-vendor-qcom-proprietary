/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilRequestAckCdmaSmsMessage.h"
#include "telephony/ril.h"

RilRequestAckCdmaSmsMessage::~RilRequestAckCdmaSmsMessage() {
}

string RilRequestAckCdmaSmsMessage::dump() {
  return mName + std::string(" { success = ") + std::string(
        mSuccess ? "true" : "false") + std::string(" { cause = ")
        + std::to_string(mCause) + std::string(" }");
}

bool RilRequestAckCdmaSmsMessage::isSuccess() {
  return mSuccess;
}

int32_t RilRequestAckCdmaSmsMessage::getCause() {
  return mCause;
}
