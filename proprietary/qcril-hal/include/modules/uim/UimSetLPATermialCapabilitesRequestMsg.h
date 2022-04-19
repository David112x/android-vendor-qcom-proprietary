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
    Set LPA terminal capability

    Retruns RIL_UIM_Errno shared pointer
*/
/*==========================================================================*/
class UimSetLPATermialCapabilitesRequestMsg : public SolicitedMessage<RIL_UIM_Errno>,
                                              public add_message_id<UimSetLPATermialCapabilitesRequestMsg>
{
  private:
    uint8_t          mSlot;
    uint8_t          mValue;
    boolean          mTlvPresent;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.set_lpa_terminal_capability_request";
    UimSetLPATermialCapabilitesRequestMsg() = delete;
    ~UimSetLPATermialCapabilitesRequestMsg() = default;

    UimSetLPATermialCapabilitesRequestMsg(uint8_t slot, boolean tlv_present, uint8_t value):
                                         SolicitedMessage<RIL_UIM_Errno>(get_class_message_id())
    {
      mName = MESSAGE_NAME;
      mSlot = slot;
      mValue = value;
      mTlvPresent = tlv_present;
    }

    inline uint8_t get_slot(void)
    {
      return mSlot;
    }

    inline uint8_t get_value(void)
    {
      return mValue;
    }

    inline boolean is_tlv_present()
    {
      return mTlvPresent;
    }

    inline string dump()
    {
      return mName;
    }
};
