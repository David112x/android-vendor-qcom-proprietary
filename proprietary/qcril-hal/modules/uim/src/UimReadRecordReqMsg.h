/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/SolicitedMessage.h"

/*===========================================================================

  CLASS:  UimReadRecordReqMsg

===========================================================================*/
/*!
    @brief
    Read record request message used internal, not sent to any module

    @return
    None.
*/
/*=========================================================================*/
class UimReadRecordReqMsg : public SolicitedMessage<void>,
                            public add_message_id<UimReadRecordReqMsg>
{
  private:
    const std::shared_ptr<Message>            mMsg;
    qmi_uim_read_record_params_type           mReadParams;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.read_record_request";
    UimReadRecordReqMsg() = delete;
    ~UimReadRecordReqMsg() = default;

    inline UimReadRecordReqMsg(const std::shared_ptr<Message>   msg_ptr,
                               qmi_uim_read_record_params_type  &read_params):
                               SolicitedMessage<void>(get_class_message_id()), mMsg(msg_ptr)
    {
      mReadParams = read_params;
      mName = MESSAGE_NAME;
    }

    inline string dump()
    {
      return mName + "{}";
    }

    inline uint16_t get_file_id(void)
    {
      return mReadParams.file_id.file_id;
    }

    inline qmi_uim_read_record_params_type *get_params()
    {
      return &mReadParams;
    }

    inline const std::shared_ptr<Message> get_request_msg(void)
    {
      return mMsg;
    }
};
