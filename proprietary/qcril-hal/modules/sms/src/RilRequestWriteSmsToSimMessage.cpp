/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilRequestWriteSmsToSimMessage.h"
#include "telephony/ril.h"

const string& RilRequestWriteSmsToSimMessage::getSmscPdu() {
    return mSmscPdu;
}

const string& RilRequestWriteSmsToSimMessage::getPdu() {
    return mPdu;
}

int RilRequestWriteSmsToSimMessage::getStatus() {
    return mStatus;
}

string RilRequestWriteSmsToSimMessage::dump() {
  return mName + " { status = " + std::to_string(mStatus) +
         " pdu = " + mPdu + " smsc = " + mSmscPdu + " }";
}
