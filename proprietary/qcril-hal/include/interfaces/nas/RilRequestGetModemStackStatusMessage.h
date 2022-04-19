/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to send a set system selection channels message.
   @Receiver: NasModule
*/

class RilRequestGetModemStackStatusMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestGetModemStackStatusMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_GET_MODEM_STACK_STATUS";

    RilRequestGetModemStackStatusMessage() = delete;
    ~RilRequestGetModemStackStatusMessage() {}

    inline RilRequestGetModemStackStatusMessage(std::shared_ptr<MessageContext> context)
        : QcRilRequestMessage(get_class_message_id(), context) {
      mName = MESSAGE_NAME;
    }
};
