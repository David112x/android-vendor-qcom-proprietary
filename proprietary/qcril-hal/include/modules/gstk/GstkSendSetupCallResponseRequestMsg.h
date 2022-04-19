/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/GenericCallback.h"
#include "framework/SolicitedMessage.h"
#include "framework/add_message_id.h"
#include "qcril_gstk_types.h"

/*===========================================================================*/
/*!
    @brief
    Sends Setup call request response to the modem.

    @params
    mRsp            - Setup call response
*/
/*==========================================================================*/
class GstkSendSetupCallResponseRequestMsg : public SolicitedMessage<RIL_GSTK_Errno>,
                                            public add_message_id<GstkSendSetupCallResponseRequestMsg>
{
  private:
    bool  mRsp;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.gstk.send_setup_call_response_request";
    GstkSendSetupCallResponseRequestMsg() = delete;
    ~GstkSendSetupCallResponseRequestMsg() {}

    GstkSendSetupCallResponseRequestMsg(bool rsp) :
                                        SolicitedMessage<RIL_GSTK_Errno>(get_class_message_id())
    {
      mRsp = rsp;
      mName = MESSAGE_NAME;
    }

    inline bool get_response(void)
    {
      return mRsp;
    }

    inline string dump()
    {
      return mName;
    }
}; /* GstkSendSetupCallResponseRequestMsg */
