/******************************************************************************
#  Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/GenericCallback.h"
#include "framework/SolicitedMessage.h"
#include "framework/add_message_id.h"
#include "modules/uim/qcril_uim_types.h"


typedef struct
{
  RIL_UIM_Errno err;
  string        atr;
} UimAtrRspParam;

/*===========================================================================*/
/*!
    @brief
    Provides Atr of requested slot.

    Retruns RIL_Errno shared pointer
*/
/*==========================================================================*/
class UimGetAtrRequestMsg : public SolicitedMessage<UimAtrRspParam>,
                            public add_message_id<UimGetAtrRequestMsg>
{
  private:
    uint8_t          mSlot;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.get_atr_request";
    UimGetAtrRequestMsg() = delete;
    ~UimGetAtrRequestMsg() = default;

    UimGetAtrRequestMsg(uint8_t slot):
                          SolicitedMessage<UimAtrRspParam>(get_class_message_id())
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
