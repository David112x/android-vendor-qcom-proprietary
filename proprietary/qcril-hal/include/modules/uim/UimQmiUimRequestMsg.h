/******************************************************************************
#  Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/SolicitedSyncMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"
#include "modules/uim/qcril_uim_srvc.h"

class UimQmiUimRequestMsg : public SolicitedSyncMessage<int>,
                            public add_message_id<UimQmiUimRequestMsg>
                            
{
  private:
    uint32_t                          mMsgId;
    void                            * mMsgPtr;
    const void                      * mOrigPtr;
    void                            * mRspData;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.qmi_uim_request_msg";
    UimQmiUimRequestMsg() = delete;
    ~UimQmiUimRequestMsg();

    UimQmiUimRequestMsg(uint32_t    msg_id, 
                        const void *dataPtr,
                        void       *rsp_data,
                        const void *orig_ptr);

    inline void * get_message()
    {
      return mMsgPtr;
    }

    inline const void * get_user_data(void)
    {
      return mOrigPtr;
    }

    inline uint32_t get_msg_id(void)
    {
      return mMsgId;
    }

    inline void *get_rsp_data()
    {
      return mRspData;
    }

    inline string dump()
    {
      return mName + "{ " + std::to_string(mMsgId) + "}";
    }
};
