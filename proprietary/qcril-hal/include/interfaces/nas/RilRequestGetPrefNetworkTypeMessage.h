/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to get mode preference Message
   @Receiver: NasModule
   */

class RilRequestGetPrefNetworkTypeMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestGetPrefNetworkTypeMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE";

    RilRequestGetPrefNetworkTypeMessage() = delete;
    ~RilRequestGetPrefNetworkTypeMessage() {}

    inline RilRequestGetPrefNetworkTypeMessage(std::shared_ptr<MessageContext> context)
        : QcRilRequestMessage(get_class_message_id(), context) {
      mName = MESSAGE_NAME;
    }
};
