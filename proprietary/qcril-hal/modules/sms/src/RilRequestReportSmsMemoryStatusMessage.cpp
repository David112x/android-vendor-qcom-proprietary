/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilRequestReportSmsMemoryStatusMessage.h"
#include "telephony/ril.h"

RilRequestReportSmsMemoryStatusMessage::~RilRequestReportSmsMemoryStatusMessage() {
}

string RilRequestReportSmsMemoryStatusMessage::dump() {
  return mName + std::string(" { available = ")  +
        (mAvailable ? "true" : "false") + std::string(" }");
}

bool RilRequestReportSmsMemoryStatusMessage::isAvailable() {
  return mAvailable;
}
