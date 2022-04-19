/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilUnsolImsNetworkStateChangedMessage.h"

RilUnsolImsNetworkStateChangedMessage::~RilUnsolImsNetworkStateChangedMessage() {
}

std::shared_ptr<UnSolicitedMessage> RilUnsolImsNetworkStateChangedMessage::clone() {
  return std::make_shared<RilUnsolImsNetworkStateChangedMessage>();
}

string RilUnsolImsNetworkStateChangedMessage::dump() {
  return mName;
}
