/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#include <string.h>
#include "request/RilRequestEmbmsActivateTmgiMessage.h"

namespace rildata {

RilRequestEmbmsActivateTmgiMessage::~RilRequestEmbmsActivateTmgiMessage() {
  if (mCallback) {
    delete mCallback;
    mCallback = nullptr;
  }
}

string RilRequestEmbmsActivateTmgiMessage::dump() {
  return "<TODO>";
}

}// namespace
