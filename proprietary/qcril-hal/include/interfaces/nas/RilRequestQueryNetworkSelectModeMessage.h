/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to query network selection message
   @Receiver: NasModule
   */

class RilRequestQueryNetworkSelectModeMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestQueryNetworkSelectModeMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE";

    RilRequestQueryNetworkSelectModeMessage() = delete;
    ~RilRequestQueryNetworkSelectModeMessage() {}

    inline RilRequestQueryNetworkSelectModeMessage(std::shared_ptr<MessageContext> context)
        : QcRilRequestMessage(get_class_message_id(), context) {
      mName = MESSAGE_NAME;
    }
    bool isSameMessage(std::shared_ptr<Message> msg) override {
      if (msg) {
        return (get_message_id() == msg->get_message_id());
      }
      return false;
    }
};
