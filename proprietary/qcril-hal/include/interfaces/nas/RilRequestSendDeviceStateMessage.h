/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to send device state Message
   @Receiver: NasModule
   */

class RilRequestSendDeviceStateMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestSendDeviceStateMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_SEND_DEVICE_STATE";

    RilRequestSendDeviceStateMessage() = delete;
    ~RilRequestSendDeviceStateMessage() {}

    inline RilRequestSendDeviceStateMessage(std::shared_ptr<MessageContext> context,
        RIL_DeviceStateType type, bool value) : QcRilRequestMessage(get_class_message_id(),
        context), mType(type), mValue(value) {
      mName = MESSAGE_NAME;
    }
    inline RIL_DeviceStateType getType() { return mType; }
    inline bool getValue() { return mValue; }
private:
    RIL_DeviceStateType mType;
    bool mValue;
};
