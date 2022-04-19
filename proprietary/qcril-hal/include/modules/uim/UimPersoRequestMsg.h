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
     Sends perso request to the modem to reactiveate

    @params
     perso_data: perso activate data
*/
/*==========================================================================*/
class UimPersoRequestMsg : public SolicitedMessage<RIL_UIM_PersoResponse>,
                           public add_message_id<UimPersoRequestMsg>
{
  private:
    RIL_UIM_PersonalizationReq   mPersoData;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.perso_request";
    UimPersoRequestMsg() = delete;
    ~UimPersoRequestMsg() {}

    UimPersoRequestMsg(RIL_UIM_PersonalizationReq &perso_data) :
                        SolicitedMessage<RIL_UIM_PersoResponse>(get_class_message_id())
    {
      mPersoData = perso_data;
      mName = MESSAGE_NAME;
    }

    inline RIL_UIM_PersonalizationReq *get_data(void)
    {
      return &mPersoData;
    }

    inline string dump()
    {
      return mName;
    }
}; /* UimPersoRequestMsg */
