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
    Get LPA terminal capability

    Retruns RIL_UIM_TerminalCapabilityLPATLV shared pointer
*/
/*==========================================================================*/
class UimGetLPATermialCapabilitesRequestMsg : public SolicitedMessage<RIL_UIM_TerminalCapabilityLPATLV>,
                                              public add_message_id<UimGetLPATermialCapabilitesRequestMsg>
{
  private:
    uint8_t          mSlot;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.get_lpa_terminal_capability_request";
    UimGetLPATermialCapabilitesRequestMsg() = delete;
    ~UimGetLPATermialCapabilitesRequestMsg() = default;

    UimGetLPATermialCapabilitesRequestMsg(uint8_t slot):
                                         SolicitedMessage<RIL_UIM_TerminalCapabilityLPATLV>(get_class_message_id())
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
