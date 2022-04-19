/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilRequestImsSendSmsMessage.h"
#include "telephony/ril.h"

void RilRequestImsSendSmsMessage::setCdmaPayload(
        const RIL_CDMA_SMS_Message& payload) {
    // memory copy
    if (mTech == RADIO_TECH_3GPP2)
        mCdmaPayload = payload;
}

RIL_CDMA_SMS_Message& RilRequestImsSendSmsMessage::getCdmaPayload() {
    return mCdmaPayload;
}

const string& RilRequestImsSendSmsMessage::getGsmSmscPdu() {
    return mGsmSmscPdu;
}

const string& RilRequestImsSendSmsMessage::getGsmPdu() {
    return mGsmPdu;
}

uint32_t RilRequestImsSendSmsMessage::getMessageReference() {
    return mRef;
}

bool RilRequestImsSendSmsMessage::isRetry() {
    return mRetry;
}

RIL_RadioTechnologyFamily RilRequestImsSendSmsMessage::getRadioTechFamily() {
    return mTech;
}

string RilRequestImsSendSmsMessage::dump() {
  return mName + std::string(" { ref = ") + std::to_string(mRef) +
    std::string(" tech = ") + std::string((mTech == RADIO_TECH_3GPP) ? "3GPP" : "3GPP2") +
    std::string(" retry = ") + std::string(mRetry ? "true" : "false") + std::string(" }");
}
