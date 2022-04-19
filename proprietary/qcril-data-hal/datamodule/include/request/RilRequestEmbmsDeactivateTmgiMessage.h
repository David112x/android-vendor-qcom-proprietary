#pragma once

/*===========================================================================

  Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#include "framework/Message.h"
#include <framework/legacy.h>
#include "framework/SolicitedMessage.h"
#include "framework/GenericCallback.h"
#include "framework/add_message_id.h"
#include "framework/message_translator.h"
#include "DataCommon.h"
#include "legacy/qmi_embms_v01.h"
#include <modules/android/RilRequestMessage.h>

namespace rildata {

class RilRequestEmbmsDeactivateTmgiMessage : public SolicitedMessage<generic_callback_payload>,
                                             public add_message_id<RilRequestEmbmsDeactivateTmgiMessage>  {
    private:

    legacy_request_payload params;

    public:

    static constexpr const char* MESSAGE_NAME = "HOOK_EMBMS_DEACTIVATE_TMGI";
    RilRequestEmbmsDeactivateTmgiMessage() = delete;
    ~RilRequestEmbmsDeactivateTmgiMessage();

    RilRequestEmbmsDeactivateTmgiMessage(const qcril_request_params_type &request)
    :SolicitedMessage<generic_callback_payload>(get_class_message_id()), params(request){
        mName = MESSAGE_NAME;
    }

    qcril_request_params_type &get_params() {
          return params.get_params();
    }
    string dump();
};

}//namespace
