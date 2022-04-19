/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to select network mode automatically
   @Receiver: NasModule
   */

class RilRequestSetNetworkSelectionAutoMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestSetNetworkSelectionAutoMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC";

    RilRequestSetNetworkSelectionAutoMessage() = delete;
    ~RilRequestSetNetworkSelectionAutoMessage() {}

    inline RilRequestSetNetworkSelectionAutoMessage(std::shared_ptr<MessageContext> context)
        : QcRilRequestMessage(get_class_message_id(), context) {
      mName = MESSAGE_NAME;
    }
};
