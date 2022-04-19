/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/SolicitedMessage.h"

/*===========================================================================

  CLASS:  UimVerifyPIN2Msg

===========================================================================*/
/*!
    @brief
    verify PIN2 request message used internal, not sent to any module

    @return
    None.
*/
/*=========================================================================*/
class UimVerifyPIN2Msg : public SolicitedMessage<void>,
                         public add_message_id<UimVerifyPIN2Msg>
{
  private:
    const std::shared_ptr<Message>   mMsg;
    qmi_uim_session_info_type        mSession;
    qmi_uim_data_type                mPath;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.verify_pin2";
    UimVerifyPIN2Msg() = delete;
    ~UimVerifyPIN2Msg()
    {
      if (mPath.data_ptr != NULL && mPath.data_len != 0)
      {
        delete[] mPath.data_ptr;
        mPath.data_ptr = NULL;
        mPath.data_len = 0;
      }
    }

    inline UimVerifyPIN2Msg(std::shared_ptr<Message> msg_ptr,
                            qmi_uim_session_info_type &session):
                            SolicitedMessage<void>(get_class_message_id()), mMsg(msg_ptr)
    {
      qmi_uim_data_type *path = NULL;
      mSession = session;
      mName = MESSAGE_NAME;
      mPath.data_ptr = NULL;
      mPath.data_len = 0;

      /* Update the path with heap data */
      if (string(UimReadTransparentReqMsg::MESSAGE_NAME) == msg_ptr->get_message_name())
      {
        auto msg = std::static_pointer_cast<UimReadTransparentReqMsg>(msg_ptr);

        if (msg != NULL)
        {
          qmi_uim_read_transparent_params_type *params =  msg->get_params();

          if (params != NULL)
          {
            path = &(params->file_id.path);
          }
        }
      }
      else if (string(UimReadRecordReqMsg::MESSAGE_NAME) == msg_ptr->get_message_name())
      {
        auto msg = std::static_pointer_cast<UimReadRecordReqMsg>(msg_ptr);

        if (msg != NULL)
        {
          qmi_uim_read_record_params_type *params =  msg->get_params();

          if (params != NULL)
          {
            path = &(params->file_id.path);
          }
        }
      }
      else if (string(UimUpdateBinaryReqMsg::MESSAGE_NAME) == msg_ptr->get_message_name())
      {
        auto msg = std::static_pointer_cast<UimUpdateBinaryReqMsg>(msg_ptr);

        if (msg != NULL)
        {
          qmi_uim_write_transparent_params_type *params =  msg->get_params();

          if (params != NULL)
          {
            path = &(params->file_id.path);
          }
        }
      }
      else if (string(UimUpdateRecordReqMsg::MESSAGE_NAME) == msg_ptr->get_message_name())
      {
        auto msg = std::static_pointer_cast<UimUpdateRecordReqMsg>(msg_ptr);

        if (msg != NULL)
        {
          qmi_uim_write_record_params_type *params =  msg->get_params();

          if (params != NULL)
          {
            path = &(params->file_id.path);
          }
        }
      }

      if (path != NULL && path->data_ptr != NULL && path->data_len != 0)
      {
        mPath.data_ptr = new unsigned char[path->data_len];

        if (mPath.data_ptr != NULL)
        {
          memcpy(mPath.data_ptr, path->data_ptr, path->data_len);
          mPath.data_len = path->data_len;
        }
        else
        {
          path->data_len = 0;
          path->data_ptr = NULL;
        }
        path->data_ptr = mPath.data_ptr;
      }
    }

    inline qmi_uim_session_info_type &get_session_info(void)
    {
      return mSession;
    }

    inline string dump()
    {
      return mName + "{}";
    }

    inline const std::shared_ptr<Message> get_request_msg(void)
    {
      return mMsg;
    }
};
