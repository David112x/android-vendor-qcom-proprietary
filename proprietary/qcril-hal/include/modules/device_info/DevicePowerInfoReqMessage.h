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

class DevicePowerInfoReqMessage : public SolicitedMessage<int>,
        public add_message_id<DevicePowerInfoReqMessage> {
  private:
    bool mPowerSaveMode;
    uint64_t mChargingMode;
    uint32_t mTotalBatteryCapacity;
    uint8_t mBatteryLevel;
    uint8_t mBatteryLevelIndex;

  public:
    static constexpr const char *MESSAGE_NAME = "DEVICE_POWER_INFO_REQ";
    DevicePowerInfoReqMessage(bool power_save_mode, uint64_t charging_mode,
            uint32_t total_battery_capacity, uint8_t battery_level,
            uint8_t battery_level_index) : SolicitedMessage<int>(
            get_class_message_id()) {
      mPowerSaveMode = power_save_mode;
      mChargingMode = charging_mode;
      mTotalBatteryCapacity = total_battery_capacity;
      mBatteryLevel = battery_level;
      mBatteryLevelIndex = battery_level_index;
      mName = MESSAGE_NAME;
    }

    ~DevicePowerInfoReqMessage() {}

    string dump() {
      return mName + " Device power info:" + std::to_string(mPowerSaveMode) + " "
          + std::to_string(mChargingMode) + " " + std::to_string(mChargingMode)
          + " " + std::to_string(mTotalBatteryCapacity) + " "
          + std::to_string(mBatteryLevel) + std::to_string(mBatteryLevelIndex);
    }

    bool getPowerSaveMode() {
        return mPowerSaveMode;
    }

    uint64_t getChargingMode() {
        return mChargingMode;
    }

    uint32_t getTotalBatteryCapacity() {
        return  mTotalBatteryCapacity;
    }

    uint8_t getBatteryLevel() {
        return mBatteryLevel;
    }

    uint8_t getBatteryLevelIndex() {
        return mBatteryLevelIndex;
    }
};
