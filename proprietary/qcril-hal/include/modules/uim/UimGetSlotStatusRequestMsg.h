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
    Gets physical slot info

    Retruns RIL_UIM_SlotsStatusInfo shared pointer
*/
/*==========================================================================*/
class UimGetSlotStatusRequestMsg : public SolicitedMessage<RIL_UIM_SlotsStatusInfo>,
                                   public add_message_id<UimGetSlotStatusRequestMsg>
{
  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.get_slot_status_request";
    ~UimGetSlotStatusRequestMsg() = default;

    UimGetSlotStatusRequestMsg():
                          SolicitedMessage<RIL_UIM_SlotsStatusInfo>(get_class_message_id())
    {
      mName = MESSAGE_NAME;
    }

    inline string dump()
    {
      return mName;
    }
};
