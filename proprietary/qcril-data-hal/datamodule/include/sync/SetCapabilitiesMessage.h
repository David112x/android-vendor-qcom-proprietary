/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include "framework/SolicitedSyncMessage.h"
#include "framework/add_message_id.h"

#include "MessageCommon.h"

using namespace std;

namespace rildata {

class SetCapabilitiesMessage : public SolicitedSyncMessage<int>,
                      public add_message_id<SetCapabilitiesMessage>
{
 private:
  bool systemStatusV2;
  bool uiInfoV2;
 public:
  static constexpr const char *MESSAGE_NAME = "SetCapabilities";
  inline SetCapabilitiesMessage() : SolicitedSyncMessage<int>(get_class_message_id())
  {
    mName = MESSAGE_NAME;
  }
  ~SetCapabilitiesMessage() {}

  void setSystemStatusV2(bool enabled) { systemStatusV2 = enabled; }
  void setUiInfoV2(bool enabled) { uiInfoV2 = enabled; }

  bool getSystemStatusV2() { return systemStatusV2; }
  bool getUiInfoV2() { return uiInfoV2; }

  string dump() {
    stringstream ss;
    ss << mName << boolalpha << "{ systemStatusV2=" << systemStatusV2 << ", uiInfoV2=" << uiInfoV2;
    return ss.str();
  }
};

}