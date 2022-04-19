/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to query 5G status
   @Receiver: NasModule
   */

class RilRequestQuery5GStatusMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestQuery5GStatusMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_QUERY_5G_STATUS";

    RilRequestQuery5GStatusMessage() = delete;
    ~RilRequestQuery5GStatusMessage() {}

    inline RilRequestQuery5GStatusMessage(std::shared_ptr<MessageContext> context)
        : QcRilRequestMessage(get_class_message_id(), context) {
      mName = MESSAGE_NAME;
    }
};
