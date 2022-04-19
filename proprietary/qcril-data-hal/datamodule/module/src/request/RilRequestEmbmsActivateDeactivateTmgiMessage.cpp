/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#include <string.h>
#include "request/RilRequestEmbmsActivateDeactivateTmgiMessage.h"

namespace rildata {

RilRequestEmbmsActivateDeactivateTmgiMessage::~RilRequestEmbmsActivateDeactivateTmgiMessage() {
  if (mCallback) {
    delete mCallback;
    mCallback = nullptr;
  }
}

string RilRequestEmbmsActivateDeactivateTmgiMessage::dump() {
  return "<TODO>";
}

}// namespace
