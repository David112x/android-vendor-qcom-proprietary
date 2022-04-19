/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/GenericCallback.h"
#include "framework/SolicitedMessage.h"
#include "framework/add_message_id.h"
#include "modules/uim/qcril_uim_types.h"


/*===========================================================================*/
/*!
    @brief
    Get GBA IMPI

    Retruns RIL_UIM_Errno shared pointer
*/
/*==========================================================================*/
class UimGBAInitRequestMsg : public SolicitedMessage<RIL_UIM_GBA_InitResponse>,
                                public add_message_id<UimGBAInitRequestMsg>
{
  private:
    RIL_UIM_GBA_InitParams    mParams;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.gba_init_request";
    UimGBAInitRequestMsg() = delete;
    ~UimGBAInitRequestMsg() = default;

    UimGBAInitRequestMsg(RIL_UIM_GBA_InitParams &params):
                            SolicitedMessage<RIL_UIM_GBA_InitResponse>(get_class_message_id())
    {
      mName = MESSAGE_NAME;
      mParams = params;
    }

    inline RIL_UIM_GBA_InitParams &get_params(void)
    {
      return mParams;
    }

    inline string dump()
    {
      return mName;
    }
};
