/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/SolicitedMessage.h"
#include "framework/add_message_id.h"
#include "string"

typedef void (*GstkTimerExpiryCallback)(void *user_data);

/*===========================================================================*/
/*!
    @brief
    Message for timer expiry
*/
/*==========================================================================*/
class GstkTimerExpiryMsg : public SolicitedMessage<void>,
                           public add_message_id<GstkTimerExpiryMsg>
{
  private:
    void                    *mUserData;
    GstkTimerExpiryCallback  mFuncCb;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.gstk.timer_request";
    GstkTimerExpiryMsg() = delete;
    ~GstkTimerExpiryMsg() {}

    GstkTimerExpiryMsg(GstkTimerExpiryCallback cb, void *user_data) :
      SolicitedMessage<void>(get_class_message_id())
    {
      mFuncCb = cb;
      mUserData = user_data;
    }

    GstkTimerExpiryCallback get_cb_func(void)
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
}; /* GstkTimerExpiryMsg */
