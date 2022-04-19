/**
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#include <string.h>
#include "modules/android/ril_message_factory.h"
#include "request/SetPreferredDataModemRequestMessage.h"

namespace rildata {

SetPreferredDataModemRequestMessage::~SetPreferredDataModemRequestMessage() {
  if (mCallback) {
    delete mCallback;
    mCallback = nullptr;
  }
}

string SetPreferredDataModemRequestMessage::dump() {
  return mName + "{ " + std::to_string(modemId) + " }";
}

}// namespace
