/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"

class NasSrvDomainPrefIndMessage : public UnSolicitedMessage,
                            public add_message_id<NasSrvDomainPrefIndMessage>
{
  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.nas.srv_domain_ind";
    NasSrvDomainPrefIndMessage() = delete;
    ~NasSrvDomainPrefIndMessage() = default;

    inline NasSrvDomainPrefIndMessage(uint8_t domainPrefValid,
                       nas_srv_domain_pref_enum_type_v01 domainPref):
                       UnSolicitedMessage(get_class_message_id())
    {
      mDomainPrefValid = domainPrefValid;
      mDomainPref = domainPref;
      mName = MESSAGE_NAME;
    }

    inline std::shared_ptr<UnSolicitedMessage> clone()
    {
      return std::make_shared<NasSrvDomainPrefIndMessage>(mDomainPrefValid, mDomainPref);
    }

    inline string dump()
    {
      return mName;
    }

    inline void getDomainPref(uint8_t& domainPrefValid, nas_srv_domain_pref_enum_type_v01& domainPref)
    {
      domainPrefValid = mDomainPrefValid;
      domainPref = mDomainPref;
    }

  private:
    uint8_t mDomainPrefValid;
    nas_srv_domain_pref_enum_type_v01 mDomainPref;
};
