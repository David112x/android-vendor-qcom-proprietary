/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to set mode preference Message
   @Receiver: NasModule
   */

class RilRequestEnableModemMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestEnableModemMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_ENABLE_MODEM_STACK";

    RilRequestEnableModemMessage() = delete;
    ~RilRequestEnableModemMessage() {}

    inline RilRequestEnableModemMessage(std::shared_ptr<MessageContext> context,
        bool on) : QcRilRequestMessage(get_class_message_id(), context), mEnable(on) {
      mName = MESSAGE_NAME;
    }
    inline bool getEnableModem() { return mEnable; }

private:
    bool mEnable;
};
