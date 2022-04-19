/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to get baseband version
   @Receiver: DmsModule
   */

class RilRequestGetBaseBandVersionMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestGetBaseBandVersionMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_BASEBAND_VERSION";

    RilRequestGetBaseBandVersionMessage() = delete;
    ~RilRequestGetBaseBandVersionMessage() {}

    explicit inline RilRequestGetBaseBandVersionMessage(std::shared_ptr<MessageContext> context)
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
