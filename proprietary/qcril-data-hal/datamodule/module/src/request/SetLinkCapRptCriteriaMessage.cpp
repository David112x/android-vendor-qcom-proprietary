/**
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#include <string.h>
#include "modules/android/ril_message_factory.h"
#include "request/SetLinkCapRptCriteriaMessage.h"

namespace rildata {

SetLinkCapRptCriteriaMessage::~SetLinkCapRptCriteriaMessage() {
  if (mCallback) {
    delete mCallback;
    mCallback = nullptr;
  }
}

string SetLinkCapRptCriteriaMessage::dump() {
  return mName + "{ }";
}

}// namespace