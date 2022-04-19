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
    Send SIM IO request to the card.

    @params
    mData            - SIM IO data
*/
/*==========================================================================*/
class UimSIMIORequestMsg : public SolicitedMessage<RIL_UIM_SIM_IO_Response>,
                           public add_message_id<UimSIMIORequestMsg>
{
  private:
    RIL_UIM_SIM_IO              mData;
    uint8_t                     mSlot;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.SIM_IO_request";
    UimSIMIORequestMsg() = delete;
    ~UimSIMIORequestMsg() {}

    UimSIMIORequestMsg(uint8_t slot, const RIL_UIM_SIM_IO &data) :
                       SolicitedMessage<RIL_UIM_SIM_IO_Response>(get_class_message_id())
    {
      mData = data;
      mSlot = slot;
      mName = MESSAGE_NAME;
    }

    inline const RIL_UIM_SIM_IO *get_sim_io_data(void)
    {
      return &mData;
    }

    inline uint8_t get_slot(void)
    {
      return mSlot;
    }

    inline string dump()
    {
      return mName;
    }
}; /* UimSIMIORequestMsg */
