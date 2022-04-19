/******************************************************************************
#  Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/GenericCallback.h"
#include "framework/SolicitedSyncMessage.h"
#include "framework/add_message_id.h"


/*===========================================================================*/
/*!
    @brief
    Provides ICCID of requested slot.

    mSlot       - In Slot on which IMSI needed

    Retruns iccid string
*/
/*==========================================================================*/
class UimGetIccIdRequestSyncMsg : public SolicitedSyncMessage<string>,
                                  public add_message_id<UimGetIccIdRequestSyncMsg>
{
  private:
    uint8_t    mSlot;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.get_iccid_sync_request";
    UimGetIccIdRequestSyncMsg() = delete;
    ~UimGetIccIdRequestSyncMsg() = default;

    UimGetIccIdRequestSyncMsg(uint8_t slot) :
                                SolicitedSyncMessage<string>(get_class_message_id())
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
