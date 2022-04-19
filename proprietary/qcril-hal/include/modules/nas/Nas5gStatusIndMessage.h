/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"

class Nas5gStatusIndMessage : public UnSolicitedMessage,
                              public add_message_id<Nas5gStatusIndMessage>
{
  private:
    bool m5gEnabled;

  public:
    static constexpr const char *MESSAGE_NAME =
        "com.qualcomm.qti.qcril.nas.5g_status_indication";
    Nas5gStatusIndMessage() = delete;
    ~Nas5gStatusIndMessage() = default;

    inline Nas5gStatusIndMessage(bool five_g_enabled):
                       UnSolicitedMessage(get_class_message_id())
    {
      m5gEnabled = five_g_enabled;
      mName = MESSAGE_NAME;
    }

    inline std::shared_ptr<UnSolicitedMessage> clone()
    {
      return std::static_pointer_cast<UnSolicitedMessage>(
              std::make_shared<Nas5gStatusIndMessage>(m5gEnabled));
    }

    inline string dump()
    {
      return mName + "{}" + "5G Enabled: " + std::to_string(m5gEnabled);
    }

    inline bool is5gEnabled(void)
    {
      return m5gEnabled;
    }
};
