/**
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#include <string.h>
#include "modules/android/ril_message_factory.h"
#include "request/GetBearerAllocationRequestMessage.h"

namespace rildata {

GetBearerAllocationRequestMessage::~GetBearerAllocationRequestMessage() {
  if (mCallback) {
    delete mCallback;
    mCallback = nullptr;
  }
}

string GetBearerAllocationRequestMessage::dump() {
  return mName + "{ }";
}

}// namespace
