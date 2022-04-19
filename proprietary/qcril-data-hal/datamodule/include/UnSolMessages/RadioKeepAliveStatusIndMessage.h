/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include "MessageCommon.h"

using namespace std;

namespace rildata {

class RadioKeepAliveStatusIndMessage : public UnSolicitedMessage,
                          public add_message_id<RadioKeepAliveStatusIndMessage> {
  private:
    KeepaliveStatusCode mStatusCode;
    uint32_t mHandle;

  public:
    static constexpr const char *MESSAGE_NAME = "RadioKeepAliveStatusIndMessage";

    inline RadioKeepAliveStatusIndMessage(KeepaliveStatusCode status, uint32_t handle):
      UnSolicitedMessage(get_class_message_id())
    {
      mName = MESSAGE_NAME;
      mStatusCode = status;
      mHandle = handle;
    }
    ~RadioKeepAliveStatusIndMessage() {}

    string dump(){return mName;};

    KeepaliveStatusCode getStatusCode(){
      return mStatusCode;
    }

    uint32_t getHandle(){
      return mHandle;
    }

    std::shared_ptr<UnSolicitedMessage> clone() {
      return std::make_shared<RadioKeepAliveStatusIndMessage>(mStatusCode, mHandle);
    }

};

}
