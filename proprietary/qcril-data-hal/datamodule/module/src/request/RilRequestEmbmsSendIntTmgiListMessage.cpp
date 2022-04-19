/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#include <string.h>
#include "request/RilRequestEmbmsSendIntTmgiListMessage.h"

namespace rildata {

RilRequestEmbmsSendIntTmgiListMessage::~RilRequestEmbmsSendIntTmgiListMessage() {
  if (mCallback) {
    delete mCallback;
    mCallback = nullptr;
  }
}

string RilRequestEmbmsSendIntTmgiListMessage::dump() {
  return "<TODO>";
}

}// namespace
