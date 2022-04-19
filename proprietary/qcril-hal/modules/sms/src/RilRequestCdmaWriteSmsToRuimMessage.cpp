/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilRequestCdmaWriteSmsToRuimMessage.h"
#include "telephony/ril.h"

RIL_CDMA_SMS_Message& RilRequestCdmaWriteSmsToRuimMessage::getCdmaSms() {
    return mCdmaSms;
}

int RilRequestCdmaWriteSmsToRuimMessage::getStatus() {
    return mStatus;
}

string RilRequestCdmaWriteSmsToRuimMessage::dump() {
  return mName + " { status = " + std::to_string(mStatus) + " }";
}
