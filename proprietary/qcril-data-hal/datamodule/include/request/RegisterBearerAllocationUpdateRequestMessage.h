/**
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#pragma once
#include "framework/SolicitedMessage.h"
#include "framework/add_message_id.h"
#include "MessageCommon.h"

namespace rildata {

class RegisterBearerAllocationUpdateRequestMessage : public SolicitedMessage<ResponseError_t>,
                                 public add_message_id<RegisterBearerAllocationUpdateRequestMessage> {
  private:
    bool mToggleSwitch;
  public:
    static constexpr const char *MESSAGE_NAME = "RegisterBearerAllocationUpdateRequestMessage";
    RegisterBearerAllocationUpdateRequestMessage() = delete;
    inline RegisterBearerAllocationUpdateRequestMessage(bool const & toggleSwitch):
                SolicitedMessage<ResponseError_t>(get_class_message_id()) {
        mName = MESSAGE_NAME;
        mToggleSwitch = toggleSwitch;
    }
    ~RegisterBearerAllocationUpdateRequestMessage(){};

    bool getToggleSwitch() {return mToggleSwitch;}

    string dump(){return mName;};
};

}
