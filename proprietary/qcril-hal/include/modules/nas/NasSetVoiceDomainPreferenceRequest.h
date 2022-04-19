/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "network_access_service_v01.h"
#include "framework/GenericCallback.h"
#include "framework/SolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"

class NasSetVoiceDomainPreferenceRequest : public SolicitedMessage<RIL_Errno>,
  public add_message_id<NasSetVoiceDomainPreferenceRequest> {
  private:
    nas_voice_domain_pref_enum_type_v01 mVoiceDomainPref;

  public:
    using cb_t = GenericCallback<RIL_Errno>::cb_t;
    static constexpr const char *MESSAGE_NAME = "NasSetVoiceDomainPreferenceRequest";

    NasSetVoiceDomainPreferenceRequest() = delete;

    ~NasSetVoiceDomainPreferenceRequest() {}

    inline NasSetVoiceDomainPreferenceRequest(const nas_voice_domain_pref_enum_type_v01& pref,
                                              cb_t callbackfn)
        : SolicitedMessage<RIL_Errno>(get_class_message_id()),
          mVoiceDomainPref(pref) {
      mName = MESSAGE_NAME;
      GenericCallback<RIL_Errno> cb(callbackfn);
      setCallback(&cb);
    }

    inline nas_voice_domain_pref_enum_type_v01 getVoiceDomainPreference() {
      return mVoiceDomainPref;
    }

    string dump() {
      return mName + "{" + "mVoiceDomainPref=" + std::to_string(mVoiceDomainPref) + "}";
    }
};
