/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/SolicitedMessage.h"
#include "framework/add_message_id.h"

/*===========================================================================*/
/*!
    @brief
    Message for timer handling and callback will get called after timer expiry
*/
/*==========================================================================*/
class UimTimerRequestMsg : public SolicitedMessage<int>,
                           public add_message_id<UimTimerRequestMsg>
{
  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.timer_request";
    ~UimTimerRequestMsg() {}

    UimTimerRequestMsg(TimeKeeper::millisec timeout) :
      SolicitedMessage<int>(get_class_message_id(), timeout) {}

    inline string dump()
    {
      return mName;
    }
}; /* UimTimerRequestMsg */
