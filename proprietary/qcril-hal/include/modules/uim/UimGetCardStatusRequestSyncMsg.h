/******************************************************************************
#  Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/SolicitedSyncMessage.h"
#include "framework/add_message_id.h"
#include "qcril_uim_types.h"

/*===========================================================================*/
/*!
    @brief
    Provides ICCID of requested slot.

    mSlot            - In Slot on which IMSI needed

    Retruns RIL_UIM_CardStatus shared pointer
*/
/*==========================================================================*/
class UimGetCardStatusRequestSyncMsg : public SolicitedSyncMessage<RIL_UIM_CardStatus>,
                                       public add_message_id<UimGetCardStatusRequestSyncMsg>
{
  private:
    uint8_t                              mSlot;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.get_card_status_sync_request";
    UimGetCardStatusRequestSyncMsg() = delete;
    ~UimGetCardStatusRequestSyncMsg() {}

    UimGetCardStatusRequestSyncMsg(uint8_t slot) :
                                   SolicitedSyncMessage<RIL_UIM_CardStatus>(get_class_message_id())
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
};
