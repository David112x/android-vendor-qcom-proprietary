/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#include <string.h>
#include "request/RilRequestEmbmsGetActiveTmgiMessage.h"

namespace rildata {

RilRequestEmbmsGetActiveTmgiMessage::~RilRequestEmbmsGetActiveTmgiMessage() {
  if (mCallback) {
    delete mCallback;
    mCallback = nullptr;
  }
}

string RilRequestEmbmsGetActiveTmgiMessage::dump() {
  return "<TODO>";
}

}// namespace
