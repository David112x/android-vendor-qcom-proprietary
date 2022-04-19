/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to query 5G upper layer indication info
   @Receiver: NasModule
   */

class RilRequestQueryUpperLayerIndInfoMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestQueryUpperLayerIndInfoMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_QUERY_UPPER_LAYER_IND_INFO";

    RilRequestQueryUpperLayerIndInfoMessage() = delete;
    ~RilRequestQueryUpperLayerIndInfoMessage() {}

    inline RilRequestQueryUpperLayerIndInfoMessage(std::shared_ptr<MessageContext> context)
        : QcRilRequestMessage(get_class_message_id(), context) {
      mName = MESSAGE_NAME;
    }
};
