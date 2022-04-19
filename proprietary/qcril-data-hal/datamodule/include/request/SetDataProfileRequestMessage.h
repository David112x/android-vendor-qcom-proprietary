/**
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **/

#pragma once
#include "framework/Message.h"
#include "framework/SolicitedMessage.h"
#include "framework/GenericCallback.h"
#include "framework/add_message_id.h"
#include "framework/message_translator.h"
#include <modules/android/RilRequestMessage.h>
#include "telephony/ril.h"
#include <vector>
#include "MessageCommon.h"

using namespace std;

namespace rildata {

/**
 * @brief Class for ril request set data profile message
 */
class SetDataProfileRequestMessage : public SolicitedMessage<RIL_Errno>,
                              public add_message_id<SetDataProfileRequestMessage> {
    private:
    vector<DataProfileInfo_t> mProfiles;

    public:
    static constexpr const char* MESSAGE_NAME = "com.qualcomm.qti.qcril.data.SetDataProfileRequestMessage";
    SetDataProfileRequestMessage() = delete;
    SetDataProfileRequestMessage(
        const vector<DataProfileInfo_t> profiles
        ):SolicitedMessage<RIL_Errno>(get_class_message_id()) {
      mName = MESSAGE_NAME;
      mProfiles = profiles;
    }
    ~SetDataProfileRequestMessage() = default;
    string dump(){return mName;}
    vector<DataProfileInfo_t> getProfiles() {return mProfiles;}
};

}
