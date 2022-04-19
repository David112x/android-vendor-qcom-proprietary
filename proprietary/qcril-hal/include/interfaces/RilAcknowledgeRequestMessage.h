/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/UnSolicitedMessage.h"
#include "framework/Message.h"
#include "framework/MessageContext.h"
#include "framework/add_message_id.h"

class RilAcknowledgeRequestMessage : public UnSolicitedMessage,
                public add_message_id<RilAcknowledgeRequestMessage> {
 private:
  std::shared_ptr<MessageContext> mContext;

 public:
  static constexpr const char *MESSAGE_NAME =
        "com.qualcomm.qti.acknowledge_request";
  RilAcknowledgeRequestMessage() = delete;
  ~RilAcknowledgeRequestMessage() {}

  explicit inline RilAcknowledgeRequestMessage(std::shared_ptr<MessageContext> ctx)
      : UnSolicitedMessage(get_class_message_id()), mContext(ctx) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() override {
    return std::make_shared<RilAcknowledgeRequestMessage>(mContext);
  }

  string dump() override {
    return mName;
  }

  std::shared_ptr<MessageContext> getContext() {
    return mContext;
  }
};
