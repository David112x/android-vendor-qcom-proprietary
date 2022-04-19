/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to query 5G Dc parameters
   @Receiver: NasModule
   */

class RilRequestQueryNrDcParamMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestQueryNrDcParamMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_QUERY_ENDC_DCNR";

    RilRequestQueryNrDcParamMessage() = delete;
    ~RilRequestQueryNrDcParamMessage() {}

    inline RilRequestQueryNrDcParamMessage(std::shared_ptr<MessageContext> context)
        : QcRilRequestMessage(get_class_message_id(), context) {
      mName = MESSAGE_NAME;
    }
};
