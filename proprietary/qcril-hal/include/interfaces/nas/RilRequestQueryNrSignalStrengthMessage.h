/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to query 5G signal strength
   @Receiver: NasModule
   */

class RilRequestQueryNrSignalStrengthMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestQueryNrSignalStrengthMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_QUERY_NR_SIGNAL_STRENGTH";

    RilRequestQueryNrSignalStrengthMessage() = delete;
    ~RilRequestQueryNrSignalStrengthMessage() {}

    inline RilRequestQueryNrSignalStrengthMessage(std::shared_ptr<MessageContext> context)
        : QcRilRequestMessage(get_class_message_id(), context) {
      mName = MESSAGE_NAME;
    }
};
