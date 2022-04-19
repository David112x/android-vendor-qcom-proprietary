/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to exit emergency callback mode Message
   @Receiver: NasModule
   */

class RilRequestExitEmergencyCallbackMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestExitEmergencyCallbackMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE";

    RilRequestExitEmergencyCallbackMessage() = delete;
    ~RilRequestExitEmergencyCallbackMessage() {}

    inline RilRequestExitEmergencyCallbackMessage(std::shared_ptr<MessageContext> context)
        : QcRilRequestMessage(get_class_message_id(), context) {
      mName = MESSAGE_NAME;
    }
};
