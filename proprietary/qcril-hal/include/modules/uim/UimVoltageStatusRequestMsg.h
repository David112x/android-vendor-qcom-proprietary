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
    Sends voltage status request message

    Retruns RIL_UIM_Errno shared pointer
*/
/*==========================================================================*/
class UimVoltageStatusRequestMsg : public SolicitedMessage<RIL_UIM_Errno>,
                                   public add_message_id<UimVoltageStatusRequestMsg>
{
  private:
    uint8_t          mSlot;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.voltage_status_request";
    UimVoltageStatusRequestMsg() = delete;
    ~UimVoltageStatusRequestMsg() = default;

    UimVoltageStatusRequestMsg(uint8_t slot):
                          SolicitedMessage<RIL_UIM_Errno>(get_class_message_id())
    {
      mName = MESSAGE_NAME;
      mSlot = slot;
    }

    inline uint8_t get_slot(void)
    {
      return mSlot;
    }

    inline string dump()
    {
      return mName;
    }
};
