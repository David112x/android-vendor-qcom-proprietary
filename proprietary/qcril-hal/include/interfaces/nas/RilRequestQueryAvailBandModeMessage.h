/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to get operator Message
   @Receiver: NasModule
   */

class RilRequestQueryAvailBandModeMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestQueryAvailBandModeMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE";

    RilRequestQueryAvailBandModeMessage() = delete;
    ~RilRequestQueryAvailBandModeMessage() {}

    inline RilRequestQueryAvailBandModeMessage(std::shared_ptr<MessageContext> context)
        : QcRilRequestMessage(get_class_message_id(), context) {
      mName = MESSAGE_NAME;
    }
};
