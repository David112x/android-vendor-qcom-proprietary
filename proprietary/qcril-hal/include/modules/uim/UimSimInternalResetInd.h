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
    UIM SIM internal reset (recovery) indication message.

    @params
    None

    @return
    None.
*/
/*=========================================================================*/
class UimSimInternalResetInd : public UnSolicitedMessage,
                               public add_message_id<UimSimInternalResetInd>
{
  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.sim_internal_reset";
    ~UimSimInternalResetInd() = default;

    inline UimSimInternalResetInd(void):UnSolicitedMessage(get_class_message_id())
    {
      mName = MESSAGE_NAME;
    }

    inline string dump()
    {
      return mName;
    }

    inline std::shared_ptr<UnSolicitedMessage> clone()
    {
      return std::static_pointer_cast<UimSimInternalResetInd>(std::make_shared<UimSimInternalResetInd>());
    }
}; /* UimSimInternalResetInd */
