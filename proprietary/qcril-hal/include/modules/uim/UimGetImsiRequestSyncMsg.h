/******************************************************************************
#  Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/SolicitedSyncMessage.h"
#include "framework/add_message_id.h"
#include "modules/uim/qcril_uim_types.h"


/*===========================================================================*/
/*!
    @brief
    Provides IMSI of requested aid of a slot.

    mSlot     - In Slot on which IMSI needed
    mAidPtr   - AID string on which IMSI is needed

    Retruns IMSI string, NULL if failed to get IMSI
*/
/*==========================================================================*/
class UimGetImsiRequestSyncMsg : public SolicitedSyncMessage<RIL_UIM_IMSI_Response>,
                                 public add_message_id<UimGetImsiRequestSyncMsg>
{
  private:
    uint8_t   mSlot;
    string    mAid;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.get_imsi_sync_request";
    UimGetImsiRequestSyncMsg() = delete;
    inline ~UimGetImsiRequestSyncMsg() {}

    UimGetImsiRequestSyncMsg(uint8_t slot, const string &aid):
                             SolicitedSyncMessage<RIL_UIM_IMSI_Response>(get_class_message_id())
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
};
