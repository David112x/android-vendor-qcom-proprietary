/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/SolicitedMessage.h"

/*===========================================================================

  CLASS:  UimUpdateRecordReqMsg

===========================================================================*/
/*!
    @brief
    update record request message used internal, not sent to any module

    @return
    None.
*/
/*=========================================================================*/
class UimUpdateRecordReqMsg : public SolicitedMessage<void>,
                              public add_message_id<UimUpdateRecordReqMsg>
{
  private:
    const std::shared_ptr<Message>            mMsg;
    qmi_uim_write_record_params_type          mWriteParams;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.update_record_request";
    UimUpdateRecordReqMsg() = delete;
    ~UimUpdateRecordReqMsg() = default;

    inline UimUpdateRecordReqMsg(const std::shared_ptr<Message> msg_ptr,
                                 qmi_uim_write_record_params_type  &write_params):
                                 SolicitedMessage<void>(get_class_message_id()), mMsg(msg_ptr)
    {
      mWriteParams = write_params;
      mName = MESSAGE_NAME;
    }

    inline string dump()
    {
      return mName + "{}";
    }

    inline uint16_t get_file_id(void)
    {
      return mWriteParams.file_id.file_id;
    }

    inline qmi_uim_write_record_params_type *get_params()
    {
      return &mWriteParams;
    }

    inline const std::shared_ptr<Message> get_request_msg(void)
    {
      return mMsg;
    }
};
