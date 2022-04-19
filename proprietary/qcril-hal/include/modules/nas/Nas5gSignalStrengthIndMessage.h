/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"

class Nas5gSignalStrengthIndMessage : public UnSolicitedMessage,
    public add_message_id<Nas5gSignalStrengthIndMessage>
{
  private:
    int32_t mRsrp;
    int32_t mSnr;

  public:
    static constexpr const char *MESSAGE_NAME =
        "com.qualcomm.qti.qcril.nas.5g_signal_strength_indication";
    Nas5gSignalStrengthIndMessage() = delete;
    ~Nas5gSignalStrengthIndMessage() = default;

    inline Nas5gSignalStrengthIndMessage(int32_t rsrp, int32_t snr):
                       UnSolicitedMessage(get_class_message_id())
    {
      mRsrp = rsrp;
      mSnr = snr;
      mName = MESSAGE_NAME;
    }

    inline std::shared_ptr<UnSolicitedMessage> clone()
    {
      return std::static_pointer_cast<UnSolicitedMessage>(
              std::make_shared<Nas5gSignalStrengthIndMessage>(mRsrp, mSnr));
    }

    inline string dump()
    {
      return mName + "{}" + "RSRP: " + std::to_string(mRsrp) +
          " SNR: " + std::to_string(mSnr);
    }

    inline int32_t getRsrp(void)
    {
      return mRsrp;
    }

    inline int32_t getSnr(void)
    {
      return mSnr;
    }
};
