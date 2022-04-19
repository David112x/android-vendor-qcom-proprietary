/******************************************************************************
#  Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"
#include "modules/uim/qcril_uim_types.h"

class UimCardAppStatusIndMsg : public UnSolicitedMessage,
                               public add_message_id<UimCardAppStatusIndMsg>
{
  private:
    RIL_UIM_AppStatus       mAppInfo;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.card_app_status_change_ind";
    UimCardAppStatusIndMsg() = delete;
    ~UimCardAppStatusIndMsg() = default;

    inline UimCardAppStatusIndMsg(const RIL_UIM_AppStatus &app_info) :
                                  UnSolicitedMessage(get_class_message_id())
    {
      mAppInfo = app_info;
      mName = MESSAGE_NAME;
    }

    inline std::shared_ptr<UnSolicitedMessage> clone()
    {
      return std::static_pointer_cast<UnSolicitedMessage>(std::make_shared<UimCardAppStatusIndMsg>(mAppInfo));
    }

    inline string dump()
    {
      return mName + "{}";
    }

    inline const RIL_UIM_AppStatus &get_app_info(void)
    {
      return mAppInfo;
    }
};
