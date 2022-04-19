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
#include "vector"

/*===========================================================================*/
/*!
    @brief
    SAP connection request

    Retruns RIL_UIM_SAP_Response shared pointer
*/
/*==========================================================================*/
class UimSAPRequestMsg : public SolicitedMessage<RIL_UIM_SAP_Response>,
                         public add_message_id<UimSAPRequestMsg>
{
  private:
    RIL_UIM_SAP_RequestType         mReqType;
    bool                            mPoweState;
    const std::vector<uint8_t>      mApduData;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.sap_request";
    UimSAPRequestMsg() = delete;
    ~UimSAPRequestMsg() = default;

    UimSAPRequestMsg(RIL_UIM_SAP_RequestType type):
                    SolicitedMessage<RIL_UIM_SAP_Response>(get_class_message_id()), mApduData()
    {
      mName = MESSAGE_NAME;
      mReqType = type;
      mPoweState = true;
    }

    UimSAPRequestMsg(RIL_UIM_SAP_RequestType type, const std::vector<uint8_t> &apdu):
                    SolicitedMessage<RIL_UIM_SAP_Response>(get_class_message_id()), mApduData(apdu)
    {
      mName = MESSAGE_NAME;
      mReqType = type;
      mPoweState = true;
    }

    UimSAPRequestMsg(RIL_UIM_SAP_RequestType type, bool power_state):
                    SolicitedMessage<RIL_UIM_SAP_Response>(get_class_message_id()), mApduData()
    {
      mName = MESSAGE_NAME;
      mReqType = type;
      mPoweState = power_state;
    }

    inline RIL_UIM_SAP_RequestType get_req_type(void)
    {
      return mReqType;
    }

    inline bool get_power_state(void)
    {
      return mPoweState;
    }

    inline const std::vector<uint8_t> &get_apdu(void)
    {
      return mApduData;
    }

    inline string dump()
    {
      return mName;
    }
};
