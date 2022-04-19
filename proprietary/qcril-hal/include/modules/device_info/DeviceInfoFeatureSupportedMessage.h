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

class DeviceInfoFeatureSupportedMessage : public SolicitedMessage<int>,
        public add_message_id<DeviceInfoFeatureSupportedMessage>
{
  private:
    bool mDevicePowerConfigSupported;
    bool mDeviceInteractiveStateSupported;

  public:
    static constexpr const char *MESSAGE_NAME =
            "RIL_REQUEST_DEVICE_INFO_FEATURE_SUPPORTED";
    DeviceInfoFeatureSupportedMessage(bool devicePowerConfigSupported,
            bool deviceInteractiveStateSupported) : SolicitedMessage<int>(
            get_class_message_id()) {
      mName = MESSAGE_NAME;
      mDevicePowerConfigSupported = devicePowerConfigSupported;
      mDeviceInteractiveStateSupported = deviceInteractiveStateSupported;
    }

    ~DeviceInfoFeatureSupportedMessage() {}

    string dump() {
      return mName + ": Device power config supported " +
              std::to_string(mDevicePowerConfigSupported) +
              " Device interactive state supported " +
              std::to_string(mDeviceInteractiveStateSupported) + " }";
    }

    bool getDevicePowerConfigSupported() {
        return mDevicePowerConfigSupported;
    }

    bool getDeviceInteractiveStateSupported() {
        return mDeviceInteractiveStateSupported;
    }
};
