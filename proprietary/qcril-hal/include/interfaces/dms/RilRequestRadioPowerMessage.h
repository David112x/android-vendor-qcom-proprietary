/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to change Radio State (APM on/off)
   @Receiver: DmsModule
   */

class RilRequestRadioPowerMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestRadioPowerMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_RADIO_POWER";

    RilRequestRadioPowerMessage() = delete;
    ~RilRequestRadioPowerMessage() {}

    explicit inline RilRequestRadioPowerMessage(std::shared_ptr<MessageContext> context, bool state)
        : QcRilRequestMessage(get_class_message_id(), context), mRadioState(state) {
      mName = MESSAGE_NAME;
    }

    bool getRadioState() { return mRadioState; }

private:
    bool mRadioState;
};
