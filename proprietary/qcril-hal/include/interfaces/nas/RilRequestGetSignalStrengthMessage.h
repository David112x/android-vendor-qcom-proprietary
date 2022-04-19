/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to get signal strength message
   @Receiver: NasModule
   */

class RilRequestGetSignalStrengthMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestGetSignalStrengthMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_SIGNAL_STRENGTH";

    RilRequestGetSignalStrengthMessage() = delete;
    ~RilRequestGetSignalStrengthMessage() {}

    inline RilRequestGetSignalStrengthMessage(std::shared_ptr<MessageContext> context)
        : QcRilRequestMessage(get_class_message_id(), context) {
      mName = MESSAGE_NAME;
    }
};
