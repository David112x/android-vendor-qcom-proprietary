/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to allow data Message
   @Receiver: NasModule
   */

class RilRequestAllowDataMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestAllowDataMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_ALLOW_DATA";

    RilRequestAllowDataMessage() = delete;
    ~RilRequestAllowDataMessage() {}

    explicit inline RilRequestAllowDataMessage(std::shared_ptr<MessageContext> context, bool allow = false)
        : QcRilRequestMessage(get_class_message_id(), context), mAllow(allow), mIsWaitForCard(false) {
      mName = MESSAGE_NAME;
    }

    bool isAllowData() { return mAllow; }
    void setWaitForCard(bool wait) { mIsWaitForCard = wait; }
    bool isWaitForCard() { return mIsWaitForCard; }
private:
    bool mAllow;
    bool mIsWaitForCard;
};
