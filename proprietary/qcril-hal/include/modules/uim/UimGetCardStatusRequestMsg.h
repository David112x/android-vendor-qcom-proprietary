/******************************************************************************
#  Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
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
    Provides Card status of requested slot.

    @params
    mSlot            - In Slot on which Card status needed
*/
/*==========================================================================*/
class UimGetCardStatusRequestMsg : public SolicitedMessage<RIL_UIM_CardStatus>,
                                   public add_message_id<UimGetCardStatusRequestMsg>
{
  private:
    uint8_t              mSlot;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.get_card_status_request";
    UimGetCardStatusRequestMsg() = delete;
    ~UimGetCardStatusRequestMsg() {}

    UimGetCardStatusRequestMsg(uint8_t slot) :
                               SolicitedMessage<RIL_UIM_CardStatus>(get_class_message_id())
    {
      mSlot = slot;
      mName = MESSAGE_NAME;
    }

    inline uint8_t get_slot(void)
    {
      return mSlot;
    }

    inline string dump()
    {
      return mName + "{ slot :" + std::to_string(mSlot) + "}";
    }
}; /* UimGetCardStatusRequestMsg */
