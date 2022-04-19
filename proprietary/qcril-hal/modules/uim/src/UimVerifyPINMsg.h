/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/SolicitedMessage.h"

/*===========================================================================

  CLASS:  UimVerifyPIN2Msg

===========================================================================*/
/*!
    @brief
    verify PIN request message used internal, not sent to any module

    @return
    None.
*/
/*=========================================================================*/
class UimVerifyPINMsg : public SolicitedMessage<void>,
                        public add_message_id<UimVerifyPINMsg>
{
  private:
    const std::shared_ptr<Message>   mMsg;
    qmi_uim_session_info_type        mSession;
    RIL_UIM_PIN_Type                 mPinType;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.verify_pin";
    UimVerifyPINMsg() = delete;
    ~UimVerifyPINMsg() = default;

    inline UimVerifyPINMsg(const std::shared_ptr<Message> msg_ptr,
                           qmi_uim_session_info_type &session,
                           RIL_UIM_PIN_Type pin_type):
                           SolicitedMessage<void>(get_class_message_id()), mMsg(msg_ptr)
    {
      mSession = session;
      mPinType = pin_type;
      mName = MESSAGE_NAME;
    }

    inline qmi_uim_session_info_type &get_session_info(void)
    {
      return mSession;
    }

    inline RIL_UIM_PIN_Type get_pin_type(void)
    {
      return mPinType;
    }

    inline string dump()
    {
      return mName + "{}";
    }

    inline const std::shared_ptr<Message> get_request_msg(void)
    {
      return mMsg;
    }
};
