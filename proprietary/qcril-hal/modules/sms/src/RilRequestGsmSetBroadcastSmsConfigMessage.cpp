/******************************************************************************
#  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilRequestGsmSetBroadcastSmsConfigMessage.h"
#include "telephony/ril.h"

RilRequestGsmSetBroadcastSmsConfigMessage::~RilRequestGsmSetBroadcastSmsConfigMessage() {
}

const std::vector<RIL_GSM_BroadcastSmsConfigInfo>&
RilRequestGsmSetBroadcastSmsConfigMessage::getConfigList() {
  return mConfigList;
}

string RilRequestGsmSetBroadcastSmsConfigMessage::dump() {
  return mName + "{ size = " + std::to_string(mConfigList.size()) + " }";
}
