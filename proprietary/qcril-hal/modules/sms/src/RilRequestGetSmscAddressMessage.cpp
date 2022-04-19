/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilRequestGetSmscAddressMessage.h"
#include "telephony/ril.h"

RilRequestGetSmscAddressMessage::~RilRequestGetSmscAddressMessage() {
}

string RilRequestGetSmscAddressMessage::dump() {
  return mName;
}
