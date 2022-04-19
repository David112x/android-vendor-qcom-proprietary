/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to set location updates
   @Receiver: NasModule
   */

class RilRequestSetLocationUpdateMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestSetLocationUpdateMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_SET_LOCATION_UPDATES";

    RilRequestSetLocationUpdateMessage() = delete;
    ~RilRequestSetLocationUpdateMessage() {}

    inline RilRequestSetLocationUpdateMessage(std::shared_ptr<MessageContext> context,
        bool enable) : QcRilRequestMessage(get_class_message_id(), context), mEnable(enable) {
      mName = MESSAGE_NAME;
    }
    inline bool getEnableStatus() { return mEnable; }
private:
    bool mEnable;
};
