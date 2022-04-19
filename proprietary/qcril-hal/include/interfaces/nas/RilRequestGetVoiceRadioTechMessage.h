/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to get voice radio tech Message
   @Receiver: NasModule
   */

class RilRequestGetVoiceRadioTechMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestGetVoiceRadioTechMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_VOICE_RADIO_TECH";

    RilRequestGetVoiceRadioTechMessage() = delete;
    ~RilRequestGetVoiceRadioTechMessage() {}

    inline RilRequestGetVoiceRadioTechMessage(std::shared_ptr<MessageContext> context)
        : QcRilRequestMessage(get_class_message_id(), context) {
      mName = MESSAGE_NAME;
    }
};
