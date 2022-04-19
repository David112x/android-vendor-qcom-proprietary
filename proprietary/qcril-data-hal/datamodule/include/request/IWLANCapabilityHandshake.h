#pragma once

/*===========================================================================

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#include "MessageCommon.h"

namespace rildata {

enum ModemIWLANCapability_t {
  present,
  not_present
};

/**
 * @brief      Class for ril request set link capability reporting criteria message.
 */
class IWLANCapabilityHandshake : public SolicitedMessage<ModemIWLANCapability_t>,
                                            public add_message_id<IWLANCapabilityHandshake> {
    private:
    bool iwlanEnabled;

    public:
    static constexpr const char* MESSAGE_NAME = "IWLANCapabilityHandshake";
    ~IWLANCapabilityHandshake(){
      if (mCallback) {
        delete mCallback;
        mCallback = nullptr;
      }
    };
    IWLANCapabilityHandshake()= delete;
    inline IWLANCapabilityHandshake(bool enabled):SolicitedMessage<ModemIWLANCapability_t>(get_class_message_id()) {
      mName = MESSAGE_NAME;
      iwlanEnabled = enabled;
    }
    string dump() {return mName;}
    bool isIWLANEnabled() { return iwlanEnabled;}
};

}//namespace
