/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/GenericCallback.h"
#include "framework/SolicitedMessage.h"
#include "framework/add_message_id.h"
#include <vector>


/*===========================================================================*/
/*!
    @brief
    Switch slot request message

    Retruns RIL_UIM_Errno shared pointer
*/
/*==========================================================================*/
class UimSwitchSlotRequestMsg : public SolicitedMessage<RIL_UIM_Errno>,
                                public add_message_id<UimSwitchSlotRequestMsg>
{
  private:
    std::vector<uint32_t>          mSlotMap;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.switch_slot_request";
    UimSwitchSlotRequestMsg() = delete;
    ~UimSwitchSlotRequestMsg() = default;

    UimSwitchSlotRequestMsg(const std::vector<uint32_t> &slot_map):
                          SolicitedMessage<RIL_UIM_Errno>(get_class_message_id()), mSlotMap(slot_map)
    {
      mName = MESSAGE_NAME;
    }

    inline const std::vector<uint32_t> &get_slot_mapping(void)
    {
      return mSlotMap;
    }

    inline string dump()
    {
      return mName;
    }
};
