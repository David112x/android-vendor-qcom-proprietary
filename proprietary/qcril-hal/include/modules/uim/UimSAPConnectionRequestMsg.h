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
#include "modules/uim/qcril_uim_types.h"


/*===========================================================================*/
/*!
    @brief
    SAP connection request

    Retruns RIL_UIM_SAP_ConnectionResponse shared pointer
*/
/*==========================================================================*/
class UimSAPConnectionRequestMsg : public SolicitedMessage<RIL_UIM_SAP_ConnectionResponse>,
                                   public add_message_id<UimSAPConnectionRequestMsg>
{
  private:
    RIL_UIM_SAP_ConnectionType   mReqType;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.sap_connection_request";
    UimSAPConnectionRequestMsg() = delete;
    ~UimSAPConnectionRequestMsg() = default;

    UimSAPConnectionRequestMsg(RIL_UIM_SAP_ConnectionType type):
                               SolicitedMessage<RIL_UIM_SAP_ConnectionResponse>(get_class_message_id())
    {
      mName = MESSAGE_NAME;
      mReqType = type;
    }

    inline RIL_UIM_SAP_ConnectionType get_req_type(void)
    {
      return mReqType;
    }

    inline string dump()
    {
      return mName;
    }
};
