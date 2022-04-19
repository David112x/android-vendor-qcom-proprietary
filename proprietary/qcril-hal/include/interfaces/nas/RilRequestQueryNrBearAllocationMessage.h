/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to query 5G bear allocation status
   @Receiver: NasModule
   */

class RilRequestQueryNrBearAllocationMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestQueryNrBearAllocationMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_QUERY_NR_BEARER_ALLOCATION";

    RilRequestQueryNrBearAllocationMessage() = delete;
    ~RilRequestQueryNrBearAllocationMessage() {}

    inline RilRequestQueryNrBearAllocationMessage(std::shared_ptr<MessageContext> context)
        : QcRilRequestMessage(get_class_message_id(), context) {
      mName = MESSAGE_NAME;
    }
};
