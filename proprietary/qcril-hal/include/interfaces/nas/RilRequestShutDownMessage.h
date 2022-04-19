/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to shut down message
   @Receiver: NasModule
   */

class RilRequestShutDownMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestShutDownMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_SHUTDOWN";

    RilRequestShutDownMessage() = delete;
    ~RilRequestShutDownMessage() {}

    inline RilRequestShutDownMessage(std::shared_ptr<MessageContext> context)
        : QcRilRequestMessage(get_class_message_id(), context) {
      mName = MESSAGE_NAME;
    }
};
