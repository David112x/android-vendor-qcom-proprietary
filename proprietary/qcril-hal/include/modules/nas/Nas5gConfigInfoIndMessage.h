/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"

class Nas5gConfigInfoIndMessage : public UnSolicitedMessage,
                              public add_message_id<Nas5gConfigInfoIndMessage>
{
  private:
    bool m5gisSA;

  public:
    static constexpr const char *MESSAGE_NAME =
        "com.qualcomm.qti.qcril.nas.5g_config_info_indication";
    Nas5gConfigInfoIndMessage() = delete;
    ~Nas5gConfigInfoIndMessage() = default;

    inline Nas5gConfigInfoIndMessage(bool isSA):
                       UnSolicitedMessage(get_class_message_id())
    {
      m5gisSA = isSA;
      mName = MESSAGE_NAME;
    }

    inline std::shared_ptr<UnSolicitedMessage> clone()
    {
      return std::static_pointer_cast<UnSolicitedMessage>(
              std::make_shared<Nas5gConfigInfoIndMessage>(m5gisSA));
    }

    inline string dump()
    {
      return mName + "{}" + " 5G SA config : " + std::to_string(m5gisSA);
    }

    inline bool is5gSA(void)
    {
      return m5gisSA;
    }
};
