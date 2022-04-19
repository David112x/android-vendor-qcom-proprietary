/**
* Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#include <string.h>
#include "request/SetRatPrefMessage.h"

namespace rildata {

SetRatPrefMessage::~SetRatPrefMessage() {
  if (mCallback) {
    delete mCallback;
    mCallback = nullptr;
  }
}

std::string convertToString(RatPreference pref) {
  switch (pref) {
    case RatPreference::Inactive:
      return "Inactive";
    case RatPreference::CellularOnly:
      return "CellularOnly";
    case RatPreference::WifiOnly:
      return "WifiOnly";
    case RatPreference::CellularPreferred:
      return "CellularPreferred";
    case RatPreference::WifiPreferred:
      return "WifiPreferred";
    case RatPreference::ImsPreferred:
      return "ImsPreferred";
    default:
      return "Invalid";
  }
}

string SetRatPrefMessage::dump() {
  return mName + "{" + convertToString(ratPrefType) + "}";
}

RatPreference SetRatPrefMessage::getRatPreference() {
  return ratPrefType;
}

}// namespace
