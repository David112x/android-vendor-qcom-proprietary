/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilRequestGsmSmsBroadcastActivateMessage.h"
#include "telephony/ril.h"

RilRequestGsmSmsBroadcastActivateMessage::~RilRequestGsmSmsBroadcastActivateMessage() {
}

string RilRequestGsmSmsBroadcastActivateMessage::dump() {
  return mName + std::string(" { activate = ")  +
        (mActivate ? "true" : "false") + std::string(" }");
}

bool RilRequestGsmSmsBroadcastActivateMessage::getActivateStatus() {
  return mActivate;
}
