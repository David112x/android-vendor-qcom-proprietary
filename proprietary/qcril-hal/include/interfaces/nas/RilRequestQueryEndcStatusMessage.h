/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to query ENDC status
   @Receiver: NasModule
   */

class RilRequestQueryEndcStatusMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestQueryEndcStatusMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_QUERY_ENDC_STATUS";

    RilRequestQueryEndcStatusMessage() = delete;
    ~RilRequestQueryEndcStatusMessage() {}

    inline RilRequestQueryEndcStatusMessage(std::shared_ptr<MessageContext> context)
        : QcRilRequestMessage(get_class_message_id(), context) {
      mName = MESSAGE_NAME;
    }
};
