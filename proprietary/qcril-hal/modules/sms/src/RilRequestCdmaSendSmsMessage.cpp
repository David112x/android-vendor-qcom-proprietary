/******************************************************************************
#  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilRequestCdmaSendSmsMessage.h"
#include "telephony/ril.h"

RilRequestCdmaSendSmsMessage::~RilRequestCdmaSendSmsMessage() {
}

string RilRequestCdmaSendSmsMessage::dump() {
  return mName;
}

const RIL_CDMA_SMS_Message& RilRequestCdmaSendSmsMessage::getCdmaSms() {
  return mCdmaSms;
}
