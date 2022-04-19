/**
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#include <string.h>
#include "modules/android/ril_message_factory.h"
#include "request/GetDataNrIconTypeMessage.h"

namespace rildata {

GetDataNrIconTypeMessage::~GetDataNrIconTypeMessage() {
  if (mCallback) {
    delete mCallback;
    mCallback = nullptr;
  }
}

string GetDataNrIconTypeMessage::dump() {
  return mName + "{ }";
}

}// namespace
