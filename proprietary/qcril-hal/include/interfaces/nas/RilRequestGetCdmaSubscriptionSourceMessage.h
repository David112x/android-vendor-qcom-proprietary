/******************************************************************************
#  Copyright (c) 2020 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to query the location where the CDMA subscription shall
 * be retrieved
 @Receiver: NasModule
 */

class RilRequestGetCdmaSubscriptionSourceMessage
    : public QcRilRequestMessage,
      public add_message_id<RilRequestGetCdmaSubscriptionSourceMessage> {
 public:
  static constexpr const char* MESSAGE_NAME = "RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE";

  RilRequestGetCdmaSubscriptionSourceMessage() = delete;
  ~RilRequestGetCdmaSubscriptionSourceMessage() {
  }

  inline RilRequestGetCdmaSubscriptionSourceMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }
};
