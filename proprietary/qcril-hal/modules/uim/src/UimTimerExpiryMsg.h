/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/SolicitedMessage.h"
#include "framework/add_message_id.h"
#include "string"

typedef void (*UimTimerExpiryCallback)(void *user_data);

/*===========================================================================*/
/*!
    @brief
    Message for timer expiry
*/
/*==========================================================================*/
class UimTimerExpiryMsg : public SolicitedMessage<void>,
                          public add_message_id<UimTimerExpiryMsg>
{
  private:
    void                    *mUserData;
    UimTimerExpiryCallback   mFuncCb;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.timer_expiry";
    UimTimerExpiryMsg() = delete;
    ~UimTimerExpiryMsg() {}

    UimTimerExpiryMsg(UimTimerExpiryCallback cb, void *user_data) :
      SolicitedMessage<void>(get_class_message_id())
    {
      mFuncCb = cb;
      mUserData = user_data;
    }

    UimTimerExpiryCallback get_cb_func(void)
    {
      return mFuncCb;
    }

    void * get_user_data(void)
    {
      return mUserData;
    }

    inline string dump()
    {
      return mName;
    }
}; /* UimTimerExpiryMsg */
