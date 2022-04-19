/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to set UICC subscription Message
   @Receiver: NasModule
   */

class RilRequestSetUiccSubsMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestSetUiccSubsMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_SET_UICC_SUBSCRIPTION";

    RilRequestSetUiccSubsMessage() = delete;
    ~RilRequestSetUiccSubsMessage() {}

    inline RilRequestSetUiccSubsMessage(std::shared_ptr<MessageContext> context,
        RIL_SelectUiccSub subInfo) : QcRilRequestMessage(get_class_message_id(), context),
        mSubInfo(subInfo) {
      mName = MESSAGE_NAME;
    }
    inline RIL_SelectUiccSub getUiccSubInfo() { return mSubInfo; }
private:
    RIL_SelectUiccSub mSubInfo;
};
