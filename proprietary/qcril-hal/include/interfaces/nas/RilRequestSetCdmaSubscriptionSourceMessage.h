/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to set cdma subscription source Message
   @Receiver: NasModule
   */

class RilRequestSetCdmaSubscriptionSourceMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestSetCdmaSubscriptionSourceMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE";
    enum class SubscriptionSource {
        SIM, NV
    };

    RilRequestSetCdmaSubscriptionSourceMessage() = delete;
    ~RilRequestSetCdmaSubscriptionSourceMessage() {}

    inline RilRequestSetCdmaSubscriptionSourceMessage(std::shared_ptr<MessageContext> context,
        SubscriptionSource source) : QcRilRequestMessage(get_class_message_id(), context),
        mSource(source) {
      mName = MESSAGE_NAME;
    }
    inline SubscriptionSource getSubcriptionSource() { return mSource; }
private:
    SubscriptionSource mSource;
};
