/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilRequestCdmaDeleteSmsOnRuimMessage.h"
#include "telephony/ril.h"

RilRequestCdmaDeleteSmsOnRuimMessage::~RilRequestCdmaDeleteSmsOnRuimMessage() {
}

string RilRequestCdmaDeleteSmsOnRuimMessage::dump() {
  return mName + std::string(" { index = ")  +
        std::to_string(mIndex) + std::string(" }");
}

int RilRequestCdmaDeleteSmsOnRuimMessage::getIndex() {
  return mIndex;
}
