/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"
#include "qcril_uim_types.h"

/*===========================================================================*/
/*!
    @brief
    UIM voltage status indication message.

    @params
    None

    @return
    None.
*/
/*=========================================================================*/
class UimVoltageStatusInd : public UnSolicitedMessage,
                            public add_message_id<UimVoltageStatusInd>
{
  private:
    RIL_UIM_UiccVoltageStatus  mStatus;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.voltage_status_indication";
    UimVoltageStatusInd() = delete;
    ~UimVoltageStatusInd() = default;

    inline UimVoltageStatusInd(RIL_UIM_UiccVoltageStatus status):
                       UnSolicitedMessage(get_class_message_id())
    {
      mStatus = status;
      mName = MESSAGE_NAME;
    }

    inline RIL_UIM_UiccVoltageStatus get_status(void)
    {
      return mStatus;
    }

    inline string dump()
    {
      return mName;
    }

    inline std::shared_ptr<UnSolicitedMessage> clone()
    {
      return std::static_pointer_cast<UimVoltageStatusInd>(std::make_shared<UimVoltageStatusInd>(mStatus));
    }
}; /* UimVoltageStatusInd */
