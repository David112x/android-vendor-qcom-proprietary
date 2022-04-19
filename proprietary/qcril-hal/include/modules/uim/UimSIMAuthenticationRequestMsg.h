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
     Sends SIM authentication request

    @params
     auth_context   - P2 of SIM authnetication request
     auth_data      - Authentication request data
     aid            - AID value
*/
/*==========================================================================*/
class UimSIMAuthenticationRequestMsg : public SolicitedMessage<RIL_UIM_SIM_IO_Response>,
                                       public add_message_id<UimSIMAuthenticationRequestMsg>
{
  private:
    int32_t  mAuthContext;
    string  mAid;
    string  mAuthData;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.uim.enter_sim_puk2_request";
    UimSIMAuthenticationRequestMsg() = delete;
    ~UimSIMAuthenticationRequestMsg() {}

    UimSIMAuthenticationRequestMsg(int32_t auth_context,
                                   const string &auth_data,
                                   const string &aid) :
                                   SolicitedMessage<RIL_UIM_SIM_IO_Response>(get_class_message_id())
    {
      mAuthContext = auth_context;
      mAuthData = auth_data;
      mAid = aid;
      mName = MESSAGE_NAME;
    }

    inline int32_t get_auth_context(void)
    {
      return mAuthContext;
    }

    inline const string &get_auth_data(void)
    {
      return mAuthData;
    }

    inline const string &get_aid(void)
    {
      return mAid;
    }

    inline string dump()
    {
      return mName;
    }
}; /* UimSIMAuthenticationRequestMsg */
