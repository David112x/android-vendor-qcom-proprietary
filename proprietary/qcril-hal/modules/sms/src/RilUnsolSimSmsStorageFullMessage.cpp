/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilUnsolSimSmsStorageFullMessage.h"

RilUnsolSimSmsStorageFullMessage::~RilUnsolSimSmsStorageFullMessage() {
}

std::shared_ptr<UnSolicitedMessage> RilUnsolSimSmsStorageFullMessage::clone() {
  return std::make_shared<RilUnsolSimSmsStorageFullMessage>();
}

string RilUnsolSimSmsStorageFullMessage::dump() {
  return mName;
}
