/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#include <string.h>
#include "request/RilRequestEmbmsContentDescUpdateMessage.h"

namespace rildata {

RilRequestEmbmsContentDescUpdateMessage::~RilRequestEmbmsContentDescUpdateMessage() {
  if (mCallback) {
    delete mCallback;
    mCallback = nullptr;
  }
}

string RilRequestEmbmsContentDescUpdateMessage::dump() {
  return "<TODO>";
}

}// namespace
