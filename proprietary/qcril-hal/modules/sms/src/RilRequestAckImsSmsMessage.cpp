/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilRequestAckImsSmsMessage.h"
#include "telephony/ril.h"

RilRequestAckImsSmsMessage::~RilRequestAckImsSmsMessage() {
}

string RilRequestAckImsSmsMessage::dump() {
  return mName + std::string(" { message ref = ") + std::to_string(
        mMessageRef) + std::string(" { status = ")
        + std::to_string(static_cast<int>(mDeliverStatus)) + std::string(" }");
}

uint32_t RilRequestAckImsSmsMessage::getMessageRef() {
  return mMessageRef;
}

qcril::interfaces::DeliverStatus
RilRequestAckImsSmsMessage::getDeliverStatus() {
  return mDeliverStatus;
}
