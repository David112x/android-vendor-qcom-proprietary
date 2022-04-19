/******************************************************************************
#  Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
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
    Provides IMSI of requested aid of a slot.

    @params
    mSlot     - Slot on which IMSI needed
    mAidPtr   - AID string
*/
/*==========================================================================*/
class UimGetImsiRequestMsg : public SolicitedMessage<RIL_UIM_IMSI_Response>,
                             public add_message_id<UimGetImsiRequestMsg>
{
  private:
    uint8_t    mSlot;
    string     mAid;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.get_imsi_request";
    UimGetImsiRequestMsg() = delete;
    inline ~UimGetImsiRequestMsg() {}

    UimGetImsiRequestMsg(uint8_t slot, const string &aid):
                         SolicitedMessage<RIL_UIM_IMSI_Response>(get_class_message_id())
    {
      mSlot = slot;
      mAid = aid;
      mName = MESSAGE_NAME;
    }

    inline uint8_t get_slot(void)
    {
      return mSlot;
    }

    inline const string &get_aid(void)
    {
      return mAid;
    }

    inline string dump()
    {
      return mName + "{ slot :" + std::to_string(mSlot) + "} Aid: " + mAid;
    }
}; /* UimGetImsiRequestMsg */
