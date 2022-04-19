/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"

/*===========================================================================*/
/*!
    @brief
    UIM SIMLOCK temperory unlock expiry indication message.

    @params
    None

    @return
    None.
*/
/*=========================================================================*/
class UimSimlockTempUnlockExpireInd : public UnSolicitedMessage,
                                      public add_message_id<UimSimlockTempUnlockExpireInd>
{
  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.simlock_temp_unlock_expiry_ind";
    ~UimSimlockTempUnlockExpireInd() = default;

    inline UimSimlockTempUnlockExpireInd() : UnSolicitedMessage(get_class_message_id())
    {
      mName = MESSAGE_NAME;
    }

    inline string dump()
    {
      return mName;
    }

    inline std::shared_ptr<UnSolicitedMessage> clone()
    {
      return std::static_pointer_cast<UimSimlockTempUnlockExpireInd>(std::make_shared<UimSimlockTempUnlockExpireInd>());
    }
}; /* UimSimlockTempUnlockExpireInd */
