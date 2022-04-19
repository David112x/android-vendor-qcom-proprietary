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

/* Request to activate GSM SMS broadcast config Message
   @Receiver: SmsModule
*/

class RilRequestGsmSmsBroadcastActivateMessage : public SolicitedMessage<RIL_Errno>,
                                        public add_message_id<RilRequestGsmSmsBroadcastActivateMessage>
{
  public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION";

    RilRequestGsmSmsBroadcastActivateMessage() = delete;
    ~RilRequestGsmSmsBroadcastActivateMessage();

    inline RilRequestGsmSmsBroadcastActivateMessage(bool activate):
            SolicitedMessage<RIL_Errno>(get_class_message_id()), mActivate(activate) {
      mName = MESSAGE_NAME;
    }

    bool getActivateStatus();

    string dump();

  private:
    bool mActivate;
};
