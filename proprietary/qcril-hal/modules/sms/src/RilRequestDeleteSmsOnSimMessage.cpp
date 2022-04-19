/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilRequestDeleteSmsOnSimMessage.h"
#include "telephony/ril.h"

RilRequestDeleteSmsOnSimMessage::~RilRequestDeleteSmsOnSimMessage() {
}

string RilRequestDeleteSmsOnSimMessage::dump() {
  return mName + std::string(" { index = ")  +
        std::to_string(mIndex) + std::string(" }");
}

int RilRequestDeleteSmsOnSimMessage::getIndex() {
  return mIndex;
}
