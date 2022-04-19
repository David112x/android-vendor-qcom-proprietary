/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to enable Endc message
   @Receiver: NasModule
   */

class RilRequestEnableEndcMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestEnableEndcMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_ENABLE_ENDC";

    RilRequestEnableEndcMessage() = delete;
    ~RilRequestEnableEndcMessage() {}

    inline RilRequestEnableEndcMessage(std::shared_ptr<MessageContext> context,
        bool enable) : QcRilRequestMessage(get_class_message_id(), context), mEnable(enable)
    {
      mName = MESSAGE_NAME;
    }
    inline bool getEndc() { return mEnable; }
private:
    bool mEnable;
};
