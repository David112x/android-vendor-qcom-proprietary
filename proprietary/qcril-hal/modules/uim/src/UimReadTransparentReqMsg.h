/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/SolicitedMessage.h"

/*===========================================================================

  CLASS:  UimReadTransparentReqMsg

===========================================================================*/
/*!
    @brief
    Read transparent request message used internal, not sent to any module

    @return
    None.
*/
/*=========================================================================*/
class UimReadTransparentReqMsg : public SolicitedMessage<void>,
                                 public add_message_id<UimReadTransparentReqMsg>
{
  private:
    const std::shared_ptr<Message>            mMsg;
    qmi_uim_read_transparent_params_type      mReadParams;
    uint8_t                                   mNumMNCDigits;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.read_transparent_request";
    UimReadTransparentReqMsg() = delete;
    ~UimReadTransparentReqMsg() = default;

    inline UimReadTransparentReqMsg(const std::shared_ptr<Message> msg_ptr,
                                    qmi_uim_read_transparent_params_type  &read_params):
                                    SolicitedMessage<void>(get_class_message_id()), mMsg(msg_ptr)
    {
      mReadParams = read_params;
      mNumMNCDigits = 0;
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

    inline qmi_uim_read_transparent_params_type *get_params()
    {
      return &mReadParams;
    }

    inline uint8_t get_num_mnc_digits(void)
    {
      return mNumMNCDigits;
    }

    inline void set_num_mnc_digits(uint8_t digits)
    {
      mNumMNCDigits = digits;
    }

    inline const std::shared_ptr<Message> get_request_msg(void)
    {
      return mMsg;
    }
};
