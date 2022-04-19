/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once
#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"

/********************** Class Definitions *************************/
class DeregisterWdsIndMessage: public UnSolicitedMessage,
                               public add_message_id<DeregisterWdsIndMessage> {
private:
  int mCid;

public:
  static constexpr const char *MESSAGE_NAME = "DeregisterWdsIndMessage";

  DeregisterWdsIndMessage() = delete;
  DeregisterWdsIndMessage(int cid) : UnSolicitedMessage(get_class_message_id()) {
    mName = MESSAGE_NAME;
    mCid = cid;
  }

  ~DeregisterWdsIndMessage(){}
  int getCid() {return mCid;}

  string dump() {
    return MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    return std::make_shared<DeregisterWdsIndMessage>(mCid);
  }
};
