/*===========================================================================

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#pragma once
#include <framework/legacy.h>
#include "framework/Message.h"
#include "framework/SolicitedMessage.h"
#include "DataCommon.h"
#include "framework/GenericCallback.h"
#include "framework/add_message_id.h"
#include "framework/message_translator.h"
#include <modules/android/RilRequestMessage.h>

namespace rildata {

class RilRequestSetCarrierInfoImsiEncryptionMessage : public SolicitedMessage<generic_callback_payload>,
                                  public add_message_id<RilRequestSetCarrierInfoImsiEncryptionMessage> {
    private:

    legacy_request_payload params;

    public:

    static constexpr const char* MESSAGE_NAME = "RIL_REQUEST_SET_CARRIER_INFO_IMSI_ENCRYPTION";
    RilRequestSetCarrierInfoImsiEncryptionMessage() = delete;
    void deepCopy(qcril_request_params_type&);
    RilRequestSetCarrierInfoImsiEncryptionMessage(const qcril_request_params_type &p)
          :SolicitedMessage<generic_callback_payload>(get_class_message_id()), params(p) {
      mName = MESSAGE_NAME;
      deepCopy(params.get_params());
    }
    ~RilRequestSetCarrierInfoImsiEncryptionMessage();

    qcril_request_params_type &get_params() {
      return params.get_params();
    }
    string dump();
};

}//namespace
