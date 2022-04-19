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
    Gets physical slot mapping

    Retruns RIL_UIM_PhySlotMap shared pointer
*/
/*==========================================================================*/
class UimGetPhysicalSlotMappingRequestMsg : public SolicitedMessage<RIL_UIM_PhySlotMap>,
                                            public add_message_id<UimGetPhysicalSlotMappingRequestMsg>
{
  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.get_physical_slot_mapping_request";
    ~UimGetPhysicalSlotMappingRequestMsg() = default;

    UimGetPhysicalSlotMappingRequestMsg():
                          SolicitedMessage<RIL_UIM_PhySlotMap>(get_class_message_id())
    {
      mName = MESSAGE_NAME;
    }

    inline string dump()
    {
      return mName;
    }
};
