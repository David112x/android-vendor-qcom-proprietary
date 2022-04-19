/**
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **/

#pragma once
#include "framework/UnSolicitedMessage.h"
#include "DataCommon.h"
#include "framework/add_message_id.h"
#include "framework/message_translator.h"

namespace rildata {

class SetRadioClientCapUnSolMessage : public UnSolicitedMessage,public add_message_id<SetRadioClientCapUnSolMessage> {
    private:
    bool mIsAPAssistCapable;

    public:

    static constexpr const char *MESSAGE_NAME = "QCRIL_DATA_SET_RADIO_CLIENT_CAP_UNSOL";

    SetRadioClientCapUnSolMessage() = delete;

    inline SetRadioClientCapUnSolMessage( bool isAPAssistCapable ):UnSolicitedMessage(get_class_message_id()) {
        mName = MESSAGE_NAME;
        mIsAPAssistCapable = isAPAssistCapable;
    }
    ~SetRadioClientCapUnSolMessage(){ }

    bool getParams() {
        return mIsAPAssistCapable;
    }

    std::shared_ptr<UnSolicitedMessage> clone() {
        return std::make_shared<SetRadioClientCapUnSolMessage>( mIsAPAssistCapable );
    }

    string dump() {
        return mName;
    }
};
}
