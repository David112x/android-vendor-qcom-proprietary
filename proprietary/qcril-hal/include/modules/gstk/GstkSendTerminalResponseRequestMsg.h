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
#include "string"

/*===========================================================================*/
/*!
    @brief
    Sends Terminal response comand to the SIM card.

    @params
    mToken            - Token used to identify the envelope request
    mData             - Terminal response data
*/
/*==========================================================================*/
class GstkSendTerminalResponseRequestMsg : public SolicitedMessage<RIL_GSTK_Errno>,
                                           public add_message_id<GstkSendTerminalResponseRequestMsg>
{
  private:
    int32_t              mToken;
    const std::string    mData;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.gstk.send_terminal_response_request";
    GstkSendTerminalResponseRequestMsg() = delete;
    ~GstkSendTerminalResponseRequestMsg() {}

    GstkSendTerminalResponseRequestMsg(int32_t token, const std::string &data) :
                                       SolicitedMessage<RIL_GSTK_Errno>(get_class_message_id()), mData(data)
    {
      mToken = token;
      mName = MESSAGE_NAME;
    }

    inline int32_t get_token(void)
    {
      return mToken;
    }

    inline const std::string &get_tr_data(void)
    {
      return mData;
    }

    inline string dump()
    {
      return mName;
    }
}; /* GstkSendTerminalResponseRequestMsg */
