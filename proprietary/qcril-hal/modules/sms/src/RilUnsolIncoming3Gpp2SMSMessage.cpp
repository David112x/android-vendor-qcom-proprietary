/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilUnsolIncoming3Gpp2SMSMessage.h"

RilUnsolIncoming3Gpp2SMSMessage::~RilUnsolIncoming3Gpp2SMSMessage() {
}

std::shared_ptr<UnSolicitedMessage> RilUnsolIncoming3Gpp2SMSMessage::clone() {
  return std::make_shared<RilUnsolIncoming3Gpp2SMSMessage>(mCdmaSms);
}

string RilUnsolIncoming3Gpp2SMSMessage::dump() {
  return mName;
}

RIL_CDMA_SMS_Message& RilUnsolIncoming3Gpp2SMSMessage::getCdmaSms() {
  return mCdmaSms;
}

