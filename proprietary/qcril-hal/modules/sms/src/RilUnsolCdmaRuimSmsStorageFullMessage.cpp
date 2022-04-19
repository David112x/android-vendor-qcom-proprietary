/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <string.h>
#include "modules/sms/RilUnsolCdmaRuimSmsStorageFullMessage.h"

RilUnsolCdmaRuimSmsStorageFullMessage::~RilUnsolCdmaRuimSmsStorageFullMessage() {
}

std::shared_ptr<UnSolicitedMessage> RilUnsolCdmaRuimSmsStorageFullMessage::clone() {
  return std::make_shared<RilUnsolCdmaRuimSmsStorageFullMessage>();
}

string RilUnsolCdmaRuimSmsStorageFullMessage::dump() {
  return mName;
}
