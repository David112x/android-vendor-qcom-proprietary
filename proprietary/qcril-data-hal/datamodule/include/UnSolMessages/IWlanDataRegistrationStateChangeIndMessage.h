/*===========================================================================

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#pragma once
#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"

using namespace std;

namespace rildata {

class IWlanDataRegistrationStateChangeIndMessage : public UnSolicitedMessage,
                          public add_message_id<IWlanDataRegistrationStateChangeIndMessage> {
  public:
    static constexpr const char *MESSAGE_NAME = "IWlanDataRegistrationStateChangeIndMessage";
    inline IWlanDataRegistrationStateChangeIndMessage():
      UnSolicitedMessage(get_class_message_id()) {
      mName = MESSAGE_NAME;
    }
    ~IWlanDataRegistrationStateChangeIndMessage() = default;

    string dump(){return mName;}

    std::shared_ptr<UnSolicitedMessage> clone(){
      return std::make_shared<IWlanDataRegistrationStateChangeIndMessage>();
    };
};

}
