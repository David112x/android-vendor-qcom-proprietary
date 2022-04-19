/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilRequestGetGsmBroadcastConfigMessage.h"
#include "telephony/ril.h"

RilRequestGetGsmBroadcastConfigMessage::~RilRequestGetGsmBroadcastConfigMessage() {
}

string RilRequestGetGsmBroadcastConfigMessage::dump() {
  return mName;
}
