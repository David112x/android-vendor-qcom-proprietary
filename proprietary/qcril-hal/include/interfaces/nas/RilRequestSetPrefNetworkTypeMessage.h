/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/add_message_id.h"
#include "interfaces/QcRilRequestMessage.h"

/* Request to set mode preference Message
   @Receiver: NasModule
   */

class RilRequestSetPrefNetworkTypeMessage : public QcRilRequestMessage,
    public add_message_id<RilRequestSetPrefNetworkTypeMessage> {
public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE";

    RilRequestSetPrefNetworkTypeMessage() = delete;
    ~RilRequestSetPrefNetworkTypeMessage() {}

    inline RilRequestSetPrefNetworkTypeMessage(std::shared_ptr<MessageContext> context,
        int pref) : QcRilRequestMessage(get_class_message_id(), context), mPref(pref) {
      mName = MESSAGE_NAME;
    }
    inline int getModePref() { return mPref; }
private:
    int mPref;
};
