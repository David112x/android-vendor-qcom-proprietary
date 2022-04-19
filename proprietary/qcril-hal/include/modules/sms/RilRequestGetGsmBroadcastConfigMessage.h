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

/* Request to get GSM SMS broadcast config Message
   @Receiver: SmsModule
*/

class RilRequestGetGsmBroadcastConfigMessage : public SolicitedMessage<RilGetGsmBroadcastConfigResult_t>,
                                        public add_message_id<RilRequestGetGsmBroadcastConfigMessage>
{
  public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG";

    ~RilRequestGetGsmBroadcastConfigMessage();

    inline RilRequestGetGsmBroadcastConfigMessage():
            SolicitedMessage<RilGetGsmBroadcastConfigResult_t>(get_class_message_id()) {
      mName = MESSAGE_NAME;
    }

    string dump();
};
