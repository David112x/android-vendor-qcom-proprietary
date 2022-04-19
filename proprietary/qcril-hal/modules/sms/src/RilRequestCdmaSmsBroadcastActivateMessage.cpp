/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilRequestCdmaSmsBroadcastActivateMessage.h"
#include "telephony/ril.h"

RilRequestCdmaSmsBroadcastActivateMessage::~RilRequestCdmaSmsBroadcastActivateMessage() {
}

string RilRequestCdmaSmsBroadcastActivateMessage::dump() {
  return mName + std::string(" { activate = ")  +
        (mActivate ? "true" : "false") + std::string(" }");
}

bool RilRequestCdmaSmsBroadcastActivateMessage::getActivateStatus() {
  return mActivate;
}
