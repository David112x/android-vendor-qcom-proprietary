/******************************************************************************
#  Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/SolicitedSyncMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"
#include "modules/gstk/qcril_gstk_srvc.h"

class GstkQmiCatRequestMsg : public SolicitedSyncMessage<int>,
                             public add_message_id<GstkQmiCatRequestMsg>
{
  private:
    qcril_gstk_request_type                         mMsgId;
    std::shared_ptr<qcril_gstk_request_data_type>   mMsgPtr;
    std::shared_ptr<qmi_cat_sync_rsp_data_type>     mRspDataPtr;
    void                                          * mUserData;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.gstk.qmi_cat_request_msg";
    GstkQmiCatRequestMsg() = delete;
    inline ~GstkQmiCatRequestMsg()
    {
      mMsgPtr = nullptr;
      mUserData = nullptr;
      mRspDataPtr = nullptr;
    }

    inline GstkQmiCatRequestMsg(qcril_gstk_request_type                              msg_id,
                                std::shared_ptr<qcril_gstk_request_data_type>        dataPtr,
                                std::shared_ptr<qmi_cat_sync_rsp_data_type>          rsp_data_ptr,
                                void                                               * user_data):
                             SolicitedSyncMessage<int>(get_class_message_id())
    {
      mMsgId = msg_id;
      mMsgPtr = dataPtr;
      mUserData = user_data;
      mRspDataPtr = rsp_data_ptr;
      mName = MESSAGE_NAME;
    }

    inline qcril_gstk_request_data_type * get_message()
    {
      return mMsgPtr.get();
    }

    inline void * get_user_data(void)
    {
      return mUserData;
    }

    inline qcril_gstk_request_type get_msg_id(void)
    {
      return mMsgId;
    }

    inline qmi_cat_sync_rsp_data_type *get_rsp_data()
    {
      if (mRspDataPtr != NULL)
      {
        return mRspDataPtr.get();
      }
      return NULL;
    }

    inline string dump()
    {
      return mName + "{ msg_id: " + std::to_string(mMsgId) + "}";
    }
};
