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

/* Request to acknowlege a GSM SMS MT message
   @Receiver: SmsModule
*/

class RilRequestAckGsmSmsMessage : public SolicitedMessage<RIL_Errno>,
                                     public add_message_id<RilRequestAckGsmSmsMessage>
{
  public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_SMS_ACKNOWLEDGE";

    RilRequestAckGsmSmsMessage() = delete;
    ~RilRequestAckGsmSmsMessage();

    inline RilRequestAckGsmSmsMessage(bool success, int32_t cause):
            SolicitedMessage<RIL_Errno>(get_class_message_id()),
            mSuccess(success), mCause(cause) {
      mName = MESSAGE_NAME;
    }

    bool isSuccess();
    int32_t getCause();

    string dump();

  private:
    bool mSuccess;
    int32_t mCause;
};
