/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/GenericCallback.h"
#include "framework/SolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"
#include "framework/message_translator.h"
#include "framework/legacy.h"
#include "modules/android/ril_request_info.h"
#include "modules/sms/qcril_qmi_sms_types.h"

/* Request to activate CDMA SMS broadcast config Message
   @Receiver: SmsModule
*/

class RilRequestCdmaSmsBroadcastActivateMessage : public SolicitedMessage<RIL_Errno>,
                                        public add_message_id<RilRequestCdmaSmsBroadcastActivateMessage>
{
  public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION";

    RilRequestCdmaSmsBroadcastActivateMessage() = delete;
    ~RilRequestCdmaSmsBroadcastActivateMessage();

    inline RilRequestCdmaSmsBroadcastActivateMessage(bool activate):
            SolicitedMessage<RIL_Errno>(get_class_message_id()), mActivate(activate) {
      mName = MESSAGE_NAME;
    }

    bool getActivateStatus();

    string dump();

  private:
    bool mActivate;
};
