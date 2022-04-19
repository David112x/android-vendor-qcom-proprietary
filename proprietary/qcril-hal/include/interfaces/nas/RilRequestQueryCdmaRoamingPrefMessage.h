/******************************************************************************
#  Copyright (c) 2020 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to query CDMA roaming preference Message
   @Receiver: NasModule
   */

class RilRequestQueryCdmaRoamingPrefMessage
    : public QcRilRequestMessage,
      public add_message_id<RilRequestQueryCdmaRoamingPrefMessage> {
 public:
  static constexpr const char* MESSAGE_NAME = "RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE";

  RilRequestQueryCdmaRoamingPrefMessage() = delete;
  ~RilRequestQueryCdmaRoamingPrefMessage() {
  }

  inline RilRequestQueryCdmaRoamingPrefMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }
};
