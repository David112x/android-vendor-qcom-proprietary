/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "telephony/ril.h"
#include "modules/sms/RilUnsolNewImsSmsStatusReportMessage.h"

RilUnsolNewImsSmsStatusReportMessage::~RilUnsolNewImsSmsStatusReportMessage() {
}

std::shared_ptr<UnSolicitedMessage> RilUnsolNewImsSmsStatusReportMessage::clone() {
  auto msg = std::make_shared<RilUnsolNewImsSmsStatusReportMessage>(mMsgRef, mRadioTech);
  if (mRadioTech == RADIO_TECH_3GPP)
    msg->setGsmPayload(mGsmPayload);
  else
    msg->setCdmaPayload(mCdmaPayload);

  return msg;
}

string RilUnsolNewImsSmsStatusReportMessage::dump() {
  return mName + " message ref: " + std::to_string(mMsgRef) +
        " radio tech: " + ((mRadioTech == RADIO_TECH_3GPP) ? "3GPP" : "3GPP2");
}

void RilUnsolNewImsSmsStatusReportMessage::setCdmaPayload(const RIL_CDMA_SMS_Message& payload) {
  if (mRadioTech == RADIO_TECH_3GPP2)
    mCdmaPayload = payload;
}

uint32_t RilUnsolNewImsSmsStatusReportMessage::getMessageRef() {
  return mMsgRef;
}

RIL_RadioTechnologyFamily RilUnsolNewImsSmsStatusReportMessage::getRadioTech() {
  return mRadioTech;
}

std::vector<uint8_t>& RilUnsolNewImsSmsStatusReportMessage::getGsmPayload() {
  return mGsmPayload;
}

RIL_CDMA_SMS_Message& RilUnsolNewImsSmsStatusReportMessage::getCdmaPayload() {
  return mCdmaPayload;
}
