/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilUnsolIncomingImsSMSMessage.h"

RilUnsolIncomingImsSMSMessage::~RilUnsolIncomingImsSMSMessage() {
}

std::shared_ptr<UnSolicitedMessage> RilUnsolIncomingImsSMSMessage::clone() {
  auto msg = std::make_shared<RilUnsolIncomingImsSMSMessage>(mTech);
  if (mTech == RADIO_TECH_3GPP)
    msg->setGsmPayload(mGsmPayload);
  else
    msg->setCdmaPayload(mCdmaPayload);

  return msg;
}

string RilUnsolIncomingImsSMSMessage::dump() {
  return mName + "{ format: "  + (mTech == RADIO_TECH_3GPP ?
        "3GPP" : "3GPP2") + " }";
}

RIL_RadioTechnologyFamily RilUnsolIncomingImsSMSMessage::getTech() {
  return mTech;
}

void RilUnsolIncomingImsSMSMessage::setCdmaPayload(const RIL_CDMA_SMS_Message& payload) {
  if (mTech == RADIO_TECH_3GPP2)
    mCdmaPayload = payload;
}

void RilUnsolIncomingImsSMSMessage::setVerificationStatus(
        qcril::interfaces::VerificationStatus status) {
  mVerStatus = status;
}

qcril::interfaces::VerificationStatus
RilUnsolIncomingImsSMSMessage::getVerificationStatus() {
  return mVerStatus;
}

const std::vector<uint8_t>& RilUnsolIncomingImsSMSMessage::getGsmPayload() {
  return mGsmPayload;
}

RIL_CDMA_SMS_Message& RilUnsolIncomingImsSMSMessage::getCdmaPayload() {
  return mCdmaPayload;
}
