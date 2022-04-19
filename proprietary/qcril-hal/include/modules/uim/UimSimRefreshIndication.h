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
    UIM Refresh indication message.

    @params
    mRefreshInd  - Refresh indication data

    @return
    None.
*/
/*=========================================================================*/
class UimSimRefreshIndication : public UnSolicitedMessage,
                                public add_message_id<UimSimRefreshIndication>
{
  private:
    RIL_UIM_SIM_RefreshIndication       mRefreshInd;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.sim_refresh_indication";
    UimSimRefreshIndication() = delete;
    ~UimSimRefreshIndication() = default;

    inline UimSimRefreshIndication(const RIL_UIM_SIM_RefreshIndication  &refresh_ind):
                    UnSolicitedMessage(get_class_message_id())
    {
      mRefreshInd = refresh_ind;
      mName = MESSAGE_NAME;
    }

    inline string dump()
    {
      return mName;
    }

    inline std::shared_ptr<UnSolicitedMessage> clone()
    {
      return std::static_pointer_cast<UnSolicitedMessage>(std::make_shared<UimSimRefreshIndication>(mRefreshInd));
    }

    inline RIL_UIM_SIM_RefreshIndication &get_refresh_ind(void)
    {
      return mRefreshInd;
    }
}; /* UimSimRefreshIndication */
