/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/SolicitedMessage.h"

/*===========================================================================

  CLASS:  UimRefreshRegisterReqMsg

===========================================================================*/
/*!
    @brief
    Refresh request message used internal, not sent to any module

    @return
    None.
*/
/*=========================================================================*/
class UimRefreshRegisterReqMsg : public SolicitedMessage<void>,
                                 public add_message_id<UimRefreshRegisterReqMsg>
{
  private:
    qmi_uim_session_type                          mSession;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.refresh_register_msg";
    UimRefreshRegisterReqMsg() = delete;
    ~UimRefreshRegisterReqMsg() = default;

    inline UimRefreshRegisterReqMsg(qmi_uim_session_type   session):
                                   SolicitedMessage<void>(get_class_message_id())
    {
      mSession = session;
      mName = MESSAGE_NAME;
    }

    inline string dump()
    {
      return mName + "{}";
    }

    inline qmi_uim_session_type get_session_type()
    {
      return mSession;
    }
};
