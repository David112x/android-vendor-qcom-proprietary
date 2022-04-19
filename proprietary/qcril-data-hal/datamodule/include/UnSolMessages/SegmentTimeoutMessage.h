/*
 * Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once
#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"
#include "MessageCommon.h"
#include "DataCommon.h"
#include "SegmentTracker.h"

namespace rildata {

/********************** Class Definitions *************************/
class SegmentTimeoutMessage: public UnSolicitedMessage,
                             public add_message_id<SegmentTimeoutMessage> {

private:
  uint16_t mtxId;
  SegmentTracker::MessageId_t mMsgId;
  SegmentationStatus_t mStatus;
  EndPointName_t mEndpointname;

public:
  static constexpr const char *MESSAGE_NAME = "SegmentTimeoutMessage";

  SegmentTimeoutMessage() = delete;
  ~SegmentTimeoutMessage(){}
  SegmentTimeoutMessage(uint16_t txId, SegmentTracker::MessageId_t msgId, SegmentationStatus_t status, EndPointName_t name)
   :UnSolicitedMessage(get_class_message_id()) {
     mtxId = txId;
     mMsgId = msgId;
     mStatus = status;
     mEndpointname = name;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    return std::make_shared<SegmentTimeoutMessage>(mtxId, mMsgId, mStatus, mEndpointname);
  }

  string dump() {
    return MESSAGE_NAME;
  }

  uint16_t getTransactionId() {return mtxId;}

  SegmentTracker::MessageId_t getMessageId() {return mMsgId;}

  SegmentationStatus_t getStatus() {return mStatus;}

  EndPointName_t getEndPointName() {return mEndpointname;}
};

} //namespace
