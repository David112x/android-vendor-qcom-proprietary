/******************************************************************************
#  Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
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
class UimGBAGetIMPIRequestMsg : public SolicitedMessage<RIL_UIM_GBA_IMPI_Response>,
                                public add_message_id<UimGBAGetIMPIRequestMsg>
{
  private:
    RIL_UIM_GBA_IMPI_Params    mParams;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.gba_get_impi_request";
    UimGBAGetIMPIRequestMsg() = delete;
    ~UimGBAGetIMPIRequestMsg() = default;

    UimGBAGetIMPIRequestMsg(RIL_UIM_GBA_IMPI_Params &params):
                            SolicitedMessage<RIL_UIM_GBA_IMPI_Response>(get_class_message_id())
    {
      mName = MESSAGE_NAME;
      mParams = params;
    }

    inline RIL_UIM_GBA_IMPI_Params &get_params(void)
    {
      return mParams;
    }

    inline string dump()
    {
      return mName;
    }
};
