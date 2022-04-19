/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"

class DeviceInteractiveInfoIndMessage : public UnSolicitedMessage,
        public add_message_id<DeviceInteractiveInfoIndMessage> {
  private:
    bool mReportDeviceInteractiveInfo;

  public:
    static constexpr const char *MESSAGE_NAME =
            "RIL_REQUEST_DEVICE_INTERACTIVE_INFO_INDICATION";
    DeviceInteractiveInfoIndMessage() : UnSolicitedMessage(get_class_message_id()) {
      mName = MESSAGE_NAME;
    }

    ~DeviceInteractiveInfoIndMessage() = default;

    DeviceInteractiveInfoIndMessage(bool reportDeviceInteractiveInfo)
            : UnSolicitedMessage(get_class_message_id()) {
      mReportDeviceInteractiveInfo = reportDeviceInteractiveInfo;
      mName = MESSAGE_NAME;
    }

    std::shared_ptr<UnSolicitedMessage> clone() {
      std::shared_ptr<DeviceInteractiveInfoIndMessage> msg =
              std::make_shared<DeviceInteractiveInfoIndMessage>();
      if (msg) {
          msg->setReportDeviceInteractiveInfo(mReportDeviceInteractiveInfo);
      }
      return msg;
    }

    string dump() {
      return mName + "{" + " Report device interactive info: " +
              std::to_string(mReportDeviceInteractiveInfo) + "}";
    }

    bool isDeviceInteractiveInfoSupported() {
      return mReportDeviceInteractiveInfo;
    }

    void setReportDeviceInteractiveInfo(bool reportDeviceInteractiveInfo) {
      mReportDeviceInteractiveInfo = reportDeviceInteractiveInfo;
    }
};
