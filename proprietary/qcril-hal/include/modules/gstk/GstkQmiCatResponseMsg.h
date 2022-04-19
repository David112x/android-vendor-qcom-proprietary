/******************************************************************************
#  Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/SolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"
#include "modules/gstk/qcril_gstk_srvc.h"

class GstkQmiCatResponseMsg : public SolicitedMessage<void>,
                              public add_message_id<GstkQmiCatResponseMsg>
{
  private:
    std::shared_ptr<qmi_cat_rsp_data_type>   mRspData;
    void                                   * mUserData;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.gstk.qmi_cat_command_callback";
    GstkQmiCatResponseMsg() = delete;
    inline ~GstkQmiCatResponseMsg()
    {
      mRspData = nullptr;
    }

    inline GstkQmiCatResponseMsg(std::shared_ptr<qmi_cat_rsp_data_type>   rsp_data,
                                 void                                   * user_data) :
    SolicitedMessage<void>(get_class_message_id())
    {
      mUserData = user_data;
      mRspData = rsp_data;
    }

    inline string dump(void)
    {
      return mName + "{}";
    }

    inline qmi_cat_rsp_data_type *get_message(void)
    {
      return mRspData.get();
    }

    inline void *get_user_data(void)
    {
      return mUserData;
    }
};

