/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"

class NasEndcDcnrIndMessage : public UnSolicitedMessage,
                              public add_message_id<NasEndcDcnrIndMessage>
{
  private:
    bool mEndcAvailable;
    bool mRestrictDcnr;

  public:
    static constexpr const char *MESSAGE_NAME =
        "com.qualcomm.qti.qcril.nas.endc_dcnr_indication";
    NasEndcDcnrIndMessage() = delete;
    ~NasEndcDcnrIndMessage() = default;

    inline NasEndcDcnrIndMessage(bool endc_available, bool restrict_dcnr):
                       UnSolicitedMessage(get_class_message_id())
    {
      mEndcAvailable = endc_available;
      mRestrictDcnr = restrict_dcnr;
      mName = MESSAGE_NAME;
    }

    inline std::shared_ptr<UnSolicitedMessage> clone()
    {
      return std::static_pointer_cast<UnSolicitedMessage>(
              std::make_shared<NasEndcDcnrIndMessage>(mEndcAvailable, mRestrictDcnr));
    }

    inline string dump()
    {
      return mName + "{}" + "ENDC Available: " + std::to_string(mEndcAvailable) +
          " DCNR Restricted: " + std::to_string(mRestrictDcnr);
    }

    inline bool isEndcAvailable(void)
    {
      return mEndcAvailable;
    }

    inline bool isDcnrRestricted(void)
    {
      return mRestrictDcnr;
    }
};
