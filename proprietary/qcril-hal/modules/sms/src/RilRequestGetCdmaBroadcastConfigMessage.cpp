/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilRequestGetCdmaBroadcastConfigMessage.h"
#include "telephony/ril.h"

RilRequestGetCdmaBroadcastConfigMessage::~RilRequestGetCdmaBroadcastConfigMessage() {
}

string RilRequestGetCdmaBroadcastConfigMessage::dump() {
  return mName;
}
