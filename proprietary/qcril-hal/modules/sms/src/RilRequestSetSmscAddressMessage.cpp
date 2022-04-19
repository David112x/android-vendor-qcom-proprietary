/******************************************************************************
#  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilRequestSetSmscAddressMessage.h"
#include "telephony/ril.h"

RilRequestSetSmscAddressMessage::~RilRequestSetSmscAddressMessage() {
}

string RilRequestSetSmscAddressMessage::dump() {
  return mName + std::string(" { smscAddress  = ")  +
        mSmscAddr + std::string(" }");
}

const string& RilRequestSetSmscAddressMessage::getSmscAddr() {
  return mSmscAddr;
}
