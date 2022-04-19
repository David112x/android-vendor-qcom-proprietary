/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"
#include "qcril_uim_types.h"

/*===========================================================================*/
/*!
    @brief
    UIM card state indication message.

    @params
    None

    @return
    None.
*/
/*=========================================================================*/
class UimCardStateChangeInd : public UnSolicitedMessage,
                              public add_message_id<UimCardStateChangeInd>
{
  private:
    uint8_t            mSlot;
    RIL_UIM_CardState  mCardState;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.card_state_change_indication";
    UimCardStateChangeInd() = delete;
    ~UimCardStateChangeInd() = default;

    inline UimCardStateChangeInd(uint8_t slot, RIL_UIM_CardState card_state):
                                        UnSolicitedMessage(get_class_message_id())
    {
      mSlot = slot;
      mCardState = card_state;
      mName = MESSAGE_NAME;
    }

    inline RIL_UIM_CardState get_card_state(void)
    {
      return mCardState;
    }

    inline uint8_t get_slot(void)
    {
      return mSlot;
    }

    inline string dump()
    {
      return mName;
    }

    inline std::shared_ptr<UnSolicitedMessage> clone()
    {
      return std::static_pointer_cast<UimCardStateChangeInd>(std::make_shared<UimCardStateChangeInd>(mSlot, mCardState));
    }
}; /* UimCardStateChangeInd */
