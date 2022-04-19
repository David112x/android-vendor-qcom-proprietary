/**
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#include <string.h>
#include "modules/android/ril_message_factory.h"
#include "request/GetAllBearerAllocationsRequestMessage.h"

namespace rildata {

GetAllBearerAllocationsRequestMessage::~GetAllBearerAllocationsRequestMessage() {
  if (mCallback) {
    delete mCallback;
    mCallback = nullptr;
  }
}

string GetAllBearerAllocationsRequestMessage::dump() {
  return mName + "{ }";
}

}// namespace
