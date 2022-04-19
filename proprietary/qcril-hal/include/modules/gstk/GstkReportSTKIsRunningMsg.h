/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/GenericCallback.h"
#include "framework/SolicitedMessage.h"
#include "framework/add_message_id.h"
#include "qcril_gstk_types.h"

/*===========================================================================*/
/*!
    @brief
    Reports STK app is running

    @params
    None
*/
/*==========================================================================*/
class GstkReportSTKIsRunningMsg : public SolicitedMessage<RIL_GSTK_Errno>,
                                  public add_message_id<GstkReportSTKIsRunningMsg>
{
  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.gstk.report_stk_is_running_request";
    ~GstkReportSTKIsRunningMsg() {}

    GstkReportSTKIsRunningMsg() :SolicitedMessage<RIL_GSTK_Errno>(get_class_message_id())
    {
      mName = MESSAGE_NAME;
    }

    inline string dump()
    {
      return mName;
    }
}; /* GstkReportSTKIsRunningMsg */
