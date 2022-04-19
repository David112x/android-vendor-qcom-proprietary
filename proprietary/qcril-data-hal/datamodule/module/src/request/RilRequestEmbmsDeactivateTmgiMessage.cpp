/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#include <string.h>
#include "request/RilRequestEmbmsDeactivateTmgiMessage.h"

namespace rildata {

RilRequestEmbmsDeactivateTmgiMessage::~RilRequestEmbmsDeactivateTmgiMessage() {
  if (mCallback) {
    delete mCallback;
    mCallback = nullptr;
  }
}

string RilRequestEmbmsDeactivateTmgiMessage::dump() {
  return "<TODO>";
}

}// namespace
