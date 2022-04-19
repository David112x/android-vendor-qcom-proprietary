/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once
#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"
#include "MessageCommon.h"

namespace rildata {

class DeregisterWdsIndCompletedMessage: public UnSolicitedMessage,
                                        public add_message_id<DeregisterWdsIndCompletedMessage> {
private:
  int mCid;
  IpFamilyType_t mIpFamilyType;

public:
  static constexpr const char *MESSAGE_NAME = "DeregisterWdsIndCompletedMessage";

  DeregisterWdsIndCompletedMessage() = delete;
  DeregisterWdsIndCompletedMessage(int cid,IpFamilyType_t ipType) : UnSolicitedMessage(get_class_message_id()) {
    mName = MESSAGE_NAME;
    mCid = cid;
    mIpFamilyType = ipType;
  }

  ~DeregisterWdsIndCompletedMessage(){}
  int getCid() {return mCid;}
  IpFamilyType_t getIpFamilyType() {return mIpFamilyType;}

  string dump() {
    return MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    return std::make_shared<DeregisterWdsIndCompletedMessage>(mCid, mIpFamilyType);
  }





};
} //namespace
