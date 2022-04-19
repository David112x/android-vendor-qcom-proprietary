/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to query 5G config info
   @Receiver: NasModule
   */

class RilRequestQuery5gConfigInfoMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestQuery5gConfigInfoMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_QUERY_5G_CONFIG_INFO";

    RilRequestQuery5gConfigInfoMessage() = delete;
    ~RilRequestQuery5gConfigInfoMessage() {}

    inline RilRequestQuery5gConfigInfoMessage(std::shared_ptr<MessageContext> context)
        : QcRilRequestMessage(get_class_message_id(), context) {
      mName = MESSAGE_NAME;
    }
};
