/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/GenericCallback.h"
#include "framework/SolicitedMessage.h"
#include "framework/add_message_id.h"
#include "qcril_uim_types.h"


/*===========================================================================*/
/*!
    @brief
    Provides IMSI of requested aid of a slot.

    @params
    mAidPtr   - ISIM challenge string
*/
/*==========================================================================*/
class UimISIMAuthenticationRequestMsg : public SolicitedMessage<RIL_UIM_SIM_IO_Response>,
                                        public add_message_id<UimISIMAuthenticationRequestMsg>
{
  private:
    string     mData;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.ISIM_Authentication_request";
    UimISIMAuthenticationRequestMsg() = delete;
    inline ~UimISIMAuthenticationRequestMsg() {}

    UimISIMAuthenticationRequestMsg(const string &data):
                                    SolicitedMessage<RIL_UIM_SIM_IO_Response>(get_class_message_id())
    {
      mData = data;
      mName = MESSAGE_NAME;
    }

    inline const string &get_ISIM_data(void)
    {
      return mData;
    }

    inline string dump()
    {
      return mName;
    }
}; /* UimISIMAuthenticationRequestMsg */
