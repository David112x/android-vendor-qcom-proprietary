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

class RilRequestSetSysSelChannelsMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestSetSysSelChannelsMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_SET_SYSTEM_SELECTION_CHANNELS";

    RilRequestSetSysSelChannelsMessage() = delete;
    ~RilRequestSetSysSelChannelsMessage() {}

    inline RilRequestSetSysSelChannelsMessage(std::shared_ptr<MessageContext> context,
        RIL_SysSelChannels &bandInfo) : QcRilRequestMessage(get_class_message_id(), context),
        mBandInfo(bandInfo) {
      mName = MESSAGE_NAME;
    }
    inline RIL_SysSelChannels& getSystemSelectionChannelInfo() { return mBandInfo; }
private:
    RIL_SysSelChannels mBandInfo;
};
