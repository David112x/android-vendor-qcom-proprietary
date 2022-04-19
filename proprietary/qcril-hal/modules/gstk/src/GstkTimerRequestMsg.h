/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/SolicitedMessage.h"
#include "framework/add_message_id.h"
#include "string"

/*===========================================================================*/
/*!
    @brief
    Message for timer handling and callback will get called after timer expiry
*/
/*==========================================================================*/
class GstkTimerRequestMsg : public SolicitedMessage<int>,
                            public add_message_id<GstkTimerRequestMsg>
{
  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.gstk.timer_request";
    ~GstkTimerRequestMsg() {}

    GstkTimerRequestMsg(TimeKeeper::millisec timeout) :
      SolicitedMessage<int>(get_class_message_id(), timeout) {}

    inline string dump()
    {
      return mName;
    }
}; /* GstkTimerRequestMsg */
