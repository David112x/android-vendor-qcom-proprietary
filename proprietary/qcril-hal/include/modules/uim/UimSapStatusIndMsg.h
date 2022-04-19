/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"
#include "modules/uim/qcril_uim_types.h"

class UimSapStatusIndMsg : public UnSolicitedMessage,
                           public add_message_id<UimSapStatusIndMsg>
{
  private:
    RIL_UIM_SAP_StatusType       mStatus;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.sap_status_ind";
    UimSapStatusIndMsg() = delete;
    ~UimSapStatusIndMsg() = default;

    inline UimSapStatusIndMsg(RIL_UIM_SAP_StatusType  status):
                       UnSolicitedMessage(get_class_message_id())
    {
      mStatus = status;
      mName = MESSAGE_NAME;
    }

    inline std::shared_ptr<UnSolicitedMessage> clone()
    {
      return std::static_pointer_cast<UnSolicitedMessage>(std::make_shared<UimSapStatusIndMsg>(mStatus));
    }

    inline string dump()
    {
      return mName + "{}" + "Status: " + std::to_string(mStatus);
    }

    inline RIL_UIM_SAP_StatusType get_status(void)
    {
      return mStatus;
    }
};
