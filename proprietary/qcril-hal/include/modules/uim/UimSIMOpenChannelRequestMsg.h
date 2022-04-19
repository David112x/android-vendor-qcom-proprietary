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
    Opens channel for the given AID.

    @params
    p2        - P2 of open channel request
    mAidPtr   - AID string
*/
/*==========================================================================*/
class UimSIMOpenChannelRequestMsg : public SolicitedMessage<RIL_UIM_OpenChannelResponse>,
                                    public add_message_id<UimSIMOpenChannelRequestMsg>
{
  private:
    int32_t    p2;
    string     mAid;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.open_channel_request";
    UimSIMOpenChannelRequestMsg() = delete;
    inline ~UimSIMOpenChannelRequestMsg() {}

    UimSIMOpenChannelRequestMsg(const string &aid, int32_t in_p2):
                                SolicitedMessage<RIL_UIM_OpenChannelResponse>(get_class_message_id())
    {
      p2 = in_p2;
      mAid = aid;
      mName = MESSAGE_NAME;
    }

    inline int32_t get_p2(void)
    {
      return p2;
    }

    inline const string &get_aid(void)
    {
      return mAid;
    }

    inline string dump()
    {
      return mName + "Aid: " + ((mAid.empty()) ? "NULL" : mAid.c_str());
    }
}; /* UimSIMOpenChannelRequestMsg */
