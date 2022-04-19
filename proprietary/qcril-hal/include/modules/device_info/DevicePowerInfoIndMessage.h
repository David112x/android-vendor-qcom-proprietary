/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"

class DevicePowerInfoIndMessage : public UnSolicitedMessage,
        public add_message_id<DevicePowerInfoIndMessage> {
  private:
    bool mReportDevicePowerInfo;
    uint32_t* mBatteryThresholdArray;
    uint32_t mBatteryThresholdLen;

  public:
    static constexpr const char *MESSAGE_NAME = "DEVICE_POWER_INFO_INDICATION";
    DevicePowerInfoIndMessage() : UnSolicitedMessage(get_class_message_id()) {
      mName = MESSAGE_NAME;
    }

    ~DevicePowerInfoIndMessage() = default;

    DevicePowerInfoIndMessage(bool reportDevicePowerInfo, uint32_t* thresholdArray,
            uint32_t thresholdArrayLen)
            : UnSolicitedMessage(get_class_message_id()) {
      mReportDevicePowerInfo = reportDevicePowerInfo;
      mBatteryThresholdArray = thresholdArray;
      mBatteryThresholdLen = thresholdArrayLen;
      mName = MESSAGE_NAME;
      dump();
    }

    std::shared_ptr<UnSolicitedMessage> clone() {
      std::shared_ptr<DevicePowerInfoIndMessage> msg =
              std::make_shared<DevicePowerInfoIndMessage>();
      if (msg) {
        msg->setReportDevicePowerInfo(mReportDevicePowerInfo);
        msg->setBatteryThresholdArray(mBatteryThresholdArray);
        msg->setBatteryThresholdLen(mBatteryThresholdLen);
      }
      return msg;
    }

    void setReportDevicePowerInfo(bool reportDevicePowerInfo) {
      mReportDevicePowerInfo = reportDevicePowerInfo;
    }

    void setBatteryThresholdArray(uint32_t* thresholdArray) {
      mBatteryThresholdArray = thresholdArray;
    }

    void setBatteryThresholdLen(uint32_t thresholdArrayLen) {
      mBatteryThresholdLen = thresholdArrayLen;
    }

    string dump() {
      std::string str;
      size_t i = 0;
      while (i < mBatteryThresholdLen) {
        str.append(std::to_string(mBatteryThresholdArray[i]));
        str.append(" ");
        i++;
      }
      return mName + "{" + " Report device power info: " +
              std::to_string(mReportDevicePowerInfo) + " " + std::to_string(mBatteryThresholdLen)
              + " " + str + " }";
    }

    bool isDevicePowerInfoSupported() {
      return mReportDevicePowerInfo;
    }

    uint32_t* getBatteryThresholdArray() {
      return mBatteryThresholdArray;
    }

    uint32_t getBatteryThresholdLength() {
      return mBatteryThresholdLen;
    }
};
