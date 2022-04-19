/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "modules/uim/UimTransmitAPDURequestMsg.h"

/*===========================================================================

  CLASS:  UimReselectReqMsg

===========================================================================*/
/*!
    @brief
    Reselect request message used internal, not sent to any module

    @return
    None.
*/
/*=========================================================================*/
class UimReselectReqMsg : public SolicitedMessage<void>,
                          public add_message_id<UimReselectReqMsg>
{
  private:
    const std::shared_ptr<UimTransmitAPDURequestMsg>   mMsg;
    qmi_uim_send_apdu_params_type                      mApduParams;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.reselect_request";
    UimReselectReqMsg() = delete;
    ~UimReselectReqMsg() = default;

    inline UimReselectReqMsg(const std::shared_ptr<UimTransmitAPDURequestMsg> msg_ptr,
                             qmi_uim_send_apdu_params_type  &apdu_params):
                             SolicitedMessage<void>(get_class_message_id()), mMsg(msg_ptr)
    {
      mApduParams = apdu_params;
      mName = MESSAGE_NAME;
    }

    inline string dump()
    {
      return mName + "{}";
    }

    inline qmi_uim_send_apdu_params_type *get_params()
    {
      return &mApduParams;
    }

    inline const std::shared_ptr<UimTransmitAPDURequestMsg> get_request_msg(void)
    {
      return mMsg;
    }
};
