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
class UimGetAttributesReqMsg : public SolicitedMessage<void>,
                               public add_message_id<UimGetAttributesReqMsg>
{
  private:
    const std::shared_ptr<Message>            mMsg;
    qmi_uim_get_file_attributes_params_type   mGetAttrParams;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.get_attributes_request";
    UimGetAttributesReqMsg() = delete;
    ~UimGetAttributesReqMsg() = default;

    inline UimGetAttributesReqMsg(const std::shared_ptr<Message> msg_ptr,
                                  qmi_uim_get_file_attributes_params_type  &params):
                                  SolicitedMessage<void>(get_class_message_id()), mMsg(msg_ptr)
    {
      mGetAttrParams = params;
      mName = MESSAGE_NAME;
    }

    inline string dump()
    {
      return mName + "{}";
    }

    inline uint16_t get_file_id(void)
    {
      return mGetAttrParams.file_id.file_id;
    }

    inline qmi_uim_get_file_attributes_params_type *get_params()
    {
      return &mGetAttrParams;
    }

    inline const std::shared_ptr<Message> get_request_msg(void)
    {
      return mMsg;
    }
};
