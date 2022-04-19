/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to set CDMA roaming preference Message
   @Receiver: NasModule
   */

class RilRequestSetCdmaRoamingPrefMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestSetCdmaRoamingPrefMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE";

    RilRequestSetCdmaRoamingPrefMessage() = delete;
    ~RilRequestSetCdmaRoamingPrefMessage() {}

    inline RilRequestSetCdmaRoamingPrefMessage(std::shared_ptr<MessageContext> context,
        int pref) : QcRilRequestMessage(get_class_message_id(), context), mPref(pref) {
      mName = MESSAGE_NAME;
    }
    inline int getRoamingPref() { return mPref; }
private:
    int mPref;
};
