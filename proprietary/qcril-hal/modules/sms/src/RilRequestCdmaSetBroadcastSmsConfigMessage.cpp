/******************************************************************************
#  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilRequestCdmaSetBroadcastSmsConfigMessage.h"
#include "telephony/ril.h"

RilRequestCdmaSetBroadcastSmsConfigMessage::~RilRequestCdmaSetBroadcastSmsConfigMessage() {
}

const std::vector<RIL_CDMA_BroadcastSmsConfigInfo>&
RilRequestCdmaSetBroadcastSmsConfigMessage::getConfigList() {
  return mConfigList;
}

string RilRequestCdmaSetBroadcastSmsConfigMessage::dump() {
  return mName + "{ size = " + std::to_string(mConfigList.size()) + " }";
}
