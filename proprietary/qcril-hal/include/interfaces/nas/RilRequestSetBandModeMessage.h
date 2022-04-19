/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to set band mode message
   @Receiver: NasModule
   */

class RilRequestSetBandModeMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestSetBandModeMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_SET_BAND_MODE";

    RilRequestSetBandModeMessage() = delete;
    ~RilRequestSetBandModeMessage() {}

    inline RilRequestSetBandModeMessage(std::shared_ptr<MessageContext> context,
        int mode) : QcRilRequestMessage(get_class_message_id(), context), mBandMode(mode) {
      mName = MESSAGE_NAME;
    }
    inline int getBandMode() { return mBandMode; }
private:
    int mBandMode;
};
