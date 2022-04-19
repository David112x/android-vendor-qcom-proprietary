/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/GenericCallback.h"
#include "framework/SolicitedMessage.h"
#include "framework/add_message_id.h"
#include "qcril_uim_types.h"


/*===========================================================================*/
/*!
    @brief
    Set APDU behavior

    Retruns RIL_UIM_Errno shared pointer
*/
/*==========================================================================*/
class UimSetAPDUBehaviorRequestMsg : public SolicitedMessage<RIL_UIM_Errno>,
                                     public add_message_id<UimSetAPDUBehaviorRequestMsg>
{
  private:
    uint8_t                    mSlot;
    RIL_UIM_SetAPDUBehavior    mValue;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.set_apdu_behavior_request";
    UimSetAPDUBehaviorRequestMsg() = delete;
    ~UimSetAPDUBehaviorRequestMsg() = default;

    UimSetAPDUBehaviorRequestMsg(uint8_t slot, RIL_UIM_SetAPDUBehavior &value):
                                 SolicitedMessage<RIL_UIM_Errno>(get_class_message_id())
    {
      mName = MESSAGE_NAME;
      mSlot = slot;
      mValue = value;
    }

    inline uint8_t get_slot(void)
    {
      return mSlot;
    }

    inline RIL_UIM_SetAPDUBehavior &get_value(void)
    {
      return mValue;
    }

    inline string dump()
    {
      return mName;
    }
};
