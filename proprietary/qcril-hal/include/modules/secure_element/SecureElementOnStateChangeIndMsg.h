/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>

/**
  * SecureElementOnStateChangeIndMsg: Sent by the Uim module when UIM module is ready
  * handling requests
  *
  */

class SecureElementOnStateChangeIndMsg: public UnSolicitedMessage,
                      public add_message_id<SecureElementOnStateChangeIndMsg>
{
  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.se.on_state_change";

    string dump()
    {
      return mName + "{}";
    }
    SecureElementOnStateChangeIndMsg():
        UnSolicitedMessage(get_class_message_id())
    {
      mName = MESSAGE_NAME;
    }

    std::shared_ptr<UnSolicitedMessage> clone()
    {
      return (std::make_shared<SecureElementOnStateChangeIndMsg>());
    }

    inline ~SecureElementOnStateChangeIndMsg() {}
};
