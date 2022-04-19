/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to set radio cap Message
   @Receiver: NasModule
   */

class RilRequestSetRadioCapMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestSetRadioCapMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_SET_RADIO_CAPABILITY";

    RilRequestSetRadioCapMessage() = delete;
    ~RilRequestSetRadioCapMessage() {}

    inline RilRequestSetRadioCapMessage(std::shared_ptr<MessageContext> context,
        RIL_RadioCapability cap) : QcRilRequestMessage(get_class_message_id(), context),
        mRadioCap(cap) {
      mName = MESSAGE_NAME;
    }
    inline RIL_RadioCapability& getRadioCap() { return mRadioCap; }
private:
    RIL_RadioCapability mRadioCap;
};
