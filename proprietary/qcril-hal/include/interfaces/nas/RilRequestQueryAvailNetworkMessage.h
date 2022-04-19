/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to query available network message
   @Receiver: NasModule
   */

class RilRequestQueryAvailNetworkMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestQueryAvailNetworkMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_QUERY_AVAILABLE_NETWORKS";

    RilRequestQueryAvailNetworkMessage() = delete;
    ~RilRequestQueryAvailNetworkMessage() {}

    inline RilRequestQueryAvailNetworkMessage(std::shared_ptr<MessageContext> context)
        : QcRilRequestMessage(get_class_message_id(), context) {
      mName = MESSAGE_NAME;
    }
};
