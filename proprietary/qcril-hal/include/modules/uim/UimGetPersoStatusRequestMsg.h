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
    Provides perso status.

    Retruns perso status
*/
/*==========================================================================*/
class UimGetPersoStatusRequestMsg : public SolicitedMessage<RIL_UIM_PersonalizationStatusResp>,
                                    public add_message_id<UimGetPersoStatusRequestMsg>
{
  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.perso_status";
    ~UimGetPersoStatusRequestMsg() = default;

    UimGetPersoStatusRequestMsg(void) :
                        SolicitedMessage<RIL_UIM_PersonalizationStatusResp>(get_class_message_id())
    {
      mName = MESSAGE_NAME;
    }

    inline string dump()
    {
      return mName;
    }
};
