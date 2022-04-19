/******************************************************************************
#  Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/SolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"
#include "qcril_uim_types.h"

/*===========================================================================*/
/*!
    @brief
    UIM Message request for card power up and power down.

    @params
    mSlot       - Slot for which request should go
    mCardPower  - Power request type

    @return
    None.
*/
/*=========================================================================*/
class UimCardPowerReqMsg : public SolicitedMessage<RIL_UIM_Errno>,
                           public add_message_id<UimCardPowerReqMsg>
{
  private:
    uint8_t                      mSlot;
    RIL_UIM_CardPowerState       mCardPower;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.card_power_request";
    UimCardPowerReqMsg() = delete;
    ~UimCardPowerReqMsg() = default;

    inline UimCardPowerReqMsg(uint8_t slot, RIL_UIM_CardPowerState card_power) :
                              SolicitedMessage<RIL_UIM_Errno>(get_class_message_id())
    {
      mCardPower = card_power;
      mSlot = slot;
      mName = MESSAGE_NAME;
    }

    inline string dump()
    {
      return mName + "{}" + "slot: " + std::to_string(mSlot) + " Power Req:" + std::to_string(mCardPower);
    }

    inline RIL_UIM_CardPowerState get_card_power_req(void)
    {
      return mCardPower;
    }

    inline uint8_t get_slot(void)
    {
      return mSlot;
    }
}; /* UimCardPowerReqMsg */
