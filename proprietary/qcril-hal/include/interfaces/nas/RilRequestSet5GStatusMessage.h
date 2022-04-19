/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to set 5G status message
   @Receiver: NasModule
   */

class RilRequestSet5GStatusMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestSet5GStatusMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_SET_5G_STATUS";

    RilRequestSet5GStatusMessage() = delete;
    ~RilRequestSet5GStatusMessage() {}

    inline RilRequestSet5GStatusMessage(std::shared_ptr<MessageContext> context,
        RIL_5G_MODE status) : QcRilRequestMessage(get_class_message_id(), context), mStatus(status) {
      mName = MESSAGE_NAME;
    }
    inline RIL_5G_MODE getStatus() { return mStatus; }
private:
    RIL_5G_MODE mStatus;
};
