/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to get radio capability message
   @Receiver: NasModule
   */

class RilRequestGetRadioCapMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestGetRadioCapMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_GET_RADIO_CAPABILITY";

    RilRequestGetRadioCapMessage() = delete;
    ~RilRequestGetRadioCapMessage() {}

    inline RilRequestGetRadioCapMessage(std::shared_ptr<MessageContext> context)
        : QcRilRequestMessage(get_class_message_id(), context) {
      mName = MESSAGE_NAME;
    }
};
