/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/SolicitedMessage.h"
#include "modules/uim/qcril_uim_srvc.h"
#include "qcril_uim.h"

/*===========================================================================

  CLASS:  UimPBRRefreshRegisterReqMsg

===========================================================================*/
/*!
    @brief
    PBR request message used internal, not sent to any module

    @return
    None.
*/
/*=========================================================================*/
class UimPBRRefreshRegisterReqMsg : public SolicitedMessage<void>,
                                    public add_message_id<UimPBRRefreshRegisterReqMsg>
{
  private:
    qcril_uim_pbr_refresh_reg_info_type           mPBRParams;
    qmi_uim_session_type                          mSession;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.PBR_refresh_register_msg";
    UimPBRRefreshRegisterReqMsg() = delete;
    ~UimPBRRefreshRegisterReqMsg() = default;

    inline UimPBRRefreshRegisterReqMsg(qcril_uim_pbr_refresh_reg_info_type  &pbr_params,
                                       qmi_uim_session_type                  session): 
                                       SolicitedMessage<void>(get_class_message_id())
    {
      mPBRParams = pbr_params;
      mSession = session;
      mName = MESSAGE_NAME;
    }

    inline string dump()
    {
      return mName + "{}";
    }

    inline qcril_uim_pbr_refresh_reg_info_type &get_params()
    {
      return mPBRParams;
    }

    inline qmi_uim_session_type get_session_type()
    {
      return mSession;
    }
};
