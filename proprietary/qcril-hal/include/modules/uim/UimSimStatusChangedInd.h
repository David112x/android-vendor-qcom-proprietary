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
    UIM SIM status indication message.

    @params
    None

    @return
    None.
*/
/*=========================================================================*/
class UimSimStatusChangedInd : public UnSolicitedMessage,
                               public add_message_id<UimSimStatusChangedInd>
{
  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.sim_status_change_indication";
    ~UimSimStatusChangedInd() = default;

    inline UimSimStatusChangedInd(void): UnSolicitedMessage(get_class_message_id())
    {
      mName = MESSAGE_NAME;
    }

    inline string dump()
    {
      return mName;
    }

    inline std::shared_ptr<UnSolicitedMessage> clone()
    {
      return std::static_pointer_cast<UimSimStatusChangedInd>(std::make_shared<UimSimStatusChangedInd>());
    }
}; /* UimSimStatusChangedInd */
