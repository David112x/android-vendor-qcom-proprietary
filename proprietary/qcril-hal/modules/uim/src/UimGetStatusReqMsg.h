/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "modules/uim/UimSIMIORequestMsg.h"

/*===========================================================================

  CLASS:  UimGetStatusReqMsg

===========================================================================*/
/*!
    @brief
    status request message used internal, not sent to any module

    @return
    None.
*/
/*=========================================================================*/
class UimGetStatusReqMsg : public SolicitedMessage<void>,
                           public add_message_id<UimGetStatusReqMsg>
{
  private:
    const std::shared_ptr<UimSIMIORequestMsg>   mMsg;
    qmi_uim_status_cmd_params_type              mParams;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.get_status_request";
    UimGetStatusReqMsg() = delete;
    ~UimGetStatusReqMsg() = default;

    inline UimGetStatusReqMsg(const std::shared_ptr<UimSIMIORequestMsg> msg_ptr,
                              qmi_uim_status_cmd_params_type  &params):
                              SolicitedMessage<void>(get_class_message_id()), mMsg(msg_ptr)
    {
      mParams = params;
      mName = MESSAGE_NAME;
    }

    inline string dump()
    {
      return mName + "{}";
    }

    inline qmi_uim_status_cmd_params_type *get_params()
    {
      return &mParams;
    }

    inline const std::shared_ptr<UimSIMIORequestMsg> get_request_msg(void)
    {
      return mMsg;
    }
};
