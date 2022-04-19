/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"
#include "qcril_uim_types.h"
#include <vector>

/*===========================================================================*/
/*!
    @brief
    UIM slot status indication message.

    @params
    None

    @return
    None.
*/
/*=========================================================================*/
class UimSlotStatusInd : public UnSolicitedMessage,
                         public add_message_id<UimSlotStatusInd>
{
  private:
    std::vector<RIL_UIM_SlotStatus>  mStatus;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.slot_status_indication";
    UimSlotStatusInd() = delete;
    ~UimSlotStatusInd() = default;

    inline UimSlotStatusInd(std::vector<RIL_UIM_SlotStatus> &status):
               UnSolicitedMessage(get_class_message_id())
    {
      mStatus = status;
      mName = MESSAGE_NAME;
    }

    inline std::vector<RIL_UIM_SlotStatus> &get_status(void)
    {
      return mStatus;
    }

    inline string dump()
    {
      return mName;
    }

    inline std::shared_ptr<UnSolicitedMessage> clone()
    {
      return std::static_pointer_cast<UnSolicitedMessage>(std::make_shared<UimSlotStatusInd>(mStatus));
    }
}; /* RIL_UIM_SlotStatus */
