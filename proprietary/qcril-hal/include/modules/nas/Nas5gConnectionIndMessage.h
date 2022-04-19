/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"

class Nas5gConnectionIndMessage : public UnSolicitedMessage,
                                  public add_message_id<Nas5gConnectionIndMessage>
{
  private:
    five_g_bearer_status m5gBearerStatus;

  public:
    static constexpr const char *MESSAGE_NAME =
        "com.qualcomm.qti.qcril.nas.5g_connection_indication";
    Nas5gConnectionIndMessage() = delete;
    ~Nas5gConnectionIndMessage() = default;

    inline Nas5gConnectionIndMessage(five_g_bearer_status five_g_bearer_status):
                       UnSolicitedMessage(get_class_message_id())
    {
      m5gBearerStatus = five_g_bearer_status;
      mName = MESSAGE_NAME;
    }

    inline std::shared_ptr<UnSolicitedMessage> clone()
    {
      return std::static_pointer_cast<UnSolicitedMessage>(
              std::make_shared<Nas5gConnectionIndMessage>(m5gBearerStatus));
    }

    inline string dump()
    {
      return mName + "{}" + "5G Connected: " + std::to_string(m5gBearerStatus);
    }

    inline five_g_bearer_status is5gBearerStatus(void)
    {
      return m5gBearerStatus;
    }
};
