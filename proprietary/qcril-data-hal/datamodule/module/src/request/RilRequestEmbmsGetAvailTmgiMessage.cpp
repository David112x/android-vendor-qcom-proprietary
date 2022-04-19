/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#include <string.h>
#include "request/RilRequestEmbmsGetAvailTmgiMessage.h"

namespace rildata {

RilRequestEmbmsGetAvailTmgiMessage::~RilRequestEmbmsGetAvailTmgiMessage() {
  if (mCallback) {
    delete mCallback;
    mCallback = nullptr;
  }
}

string RilRequestEmbmsGetAvailTmgiMessage::dump() {
  return "<TODO>";
}

}// namespace
