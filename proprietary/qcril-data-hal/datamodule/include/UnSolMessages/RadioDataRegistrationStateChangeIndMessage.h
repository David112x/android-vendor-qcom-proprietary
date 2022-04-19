/*===========================================================================

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#pragma once

#include "MessageCommon.h"

using namespace std;

namespace rildata {

class RadioDataRegistrationStateChangeIndMessage : public UnSolicitedMessage,
                          public add_message_id<RadioDataRegistrationStateChangeIndMessage> {
  public:
    static constexpr const char *MESSAGE_NAME = "RadioDataRegistrationStateChangeIndMessage";
    inline RadioDataRegistrationStateChangeIndMessage():
      UnSolicitedMessage(get_class_message_id()) {
      mName = MESSAGE_NAME;
    }
    ~RadioDataRegistrationStateChangeIndMessage() = default;

    string dump(){return mName;}

    std::shared_ptr<UnSolicitedMessage> clone(){
      return std::make_shared<RadioDataRegistrationStateChangeIndMessage>();
    };
};

}
