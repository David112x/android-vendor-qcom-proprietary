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
     Sends depersonalization request to the modem

    @params
     unlock_code   - PIN for depersonalization
     perso_type    - perso type
*/
/*==========================================================================*/
class UimEnterDePersoRequestMsg : public SolicitedMessage<RIL_UIM_PersoResponse>,
                                  public add_message_id<UimEnterDePersoRequestMsg>
{
  private:
    string                  mUnlockCode;
    RIL_UIM_PersoSubstate   mPersoType;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.enter_deperso_request";
    UimEnterDePersoRequestMsg() = delete;
    ~UimEnterDePersoRequestMsg() {}

    UimEnterDePersoRequestMsg(const string &unlock_code,
                              RIL_UIM_PersoSubstate perso_type) :
                              SolicitedMessage<RIL_UIM_PersoResponse>(get_class_message_id())
    {
      mUnlockCode = unlock_code;
      mPersoType = perso_type;
      mName = MESSAGE_NAME;
    }

    inline const string &get_unlock_code(void)
    {
      return mUnlockCode;
    }

    inline RIL_UIM_PersoSubstate get_perso_type(void)
    {
      return mPersoType;
    }

    inline string dump()
    {
      return mName;
    }
}; /* UimEnterDePersoRequestMsg */
