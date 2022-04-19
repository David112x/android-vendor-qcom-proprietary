/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilRequestGetImsRegistrationMessage.h"
#include "telephony/ril.h"

RilRequestGetImsRegistrationMessage::~RilRequestGetImsRegistrationMessage() {
}

string RilRequestGetImsRegistrationMessage::dump() {
  return mName;
}
