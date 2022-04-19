/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to set screen state Message
   @Receiver: NasModule
   */

class RilRequestScreenStateMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestScreenStateMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_SCREEN_STATE";

    RilRequestScreenStateMessage() = delete;
    ~RilRequestScreenStateMessage() {}

    inline RilRequestScreenStateMessage(std::shared_ptr<MessageContext> context,
        bool state) : QcRilRequestMessage(get_class_message_id(), context), mState(state) {
      mName = MESSAGE_NAME;
    }
    inline bool getState() { return mState; }
private:
    bool mState;
};
