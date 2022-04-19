/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to get Cdma Subscription info
   @Receiver: NasModule
   */

class RilRequestCdmaSubscriptionMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestCdmaSubscriptionMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_CDMA_SUBSCRIPTION";

    RilRequestCdmaSubscriptionMessage() = delete;
    ~RilRequestCdmaSubscriptionMessage() {}

    inline RilRequestCdmaSubscriptionMessage(std::shared_ptr<MessageContext> context)
        : QcRilRequestMessage(get_class_message_id(), context) {
      mName = MESSAGE_NAME;
    }
};
