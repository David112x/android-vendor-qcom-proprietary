/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "framework/Dispatcher.h"
#include <framework/legacy.h>
#include "framework/Message.h"
#include "framework/SolicitedMessage.h"
#include "framework/Util.h"

class DeviceInteractiveStateMessage : public SolicitedMessage<int>,
        public add_message_id<DeviceInteractiveStateMessage> {
  private:
    uint8_t mDeviceInteractiveState;

  public:
    static constexpr const char *MESSAGE_NAME =
            "RIL_REQUEST_DEVICE_INTERACTIVE_STATE";
    DeviceInteractiveStateMessage(int device_interactive_state) : SolicitedMessage<int>(
            get_class_message_id()) {
      mDeviceInteractiveState = device_interactive_state;
      mName = MESSAGE_NAME;
    }

    ~DeviceInteractiveStateMessage() {}

    string dump() {
      return mName + "{" + " Device interactive state " + std::to_string(mDeviceInteractiveState) + " }";
    }

    uint8_t getDeviceInteractiveState() {
        return mDeviceInteractiveState;
    }
};
