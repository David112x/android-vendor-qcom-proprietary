/******************************************************************************
#  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
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

/* Request to send a CDMA SMS message.
   @Receiver: SmsModule
*/

class RilRequestCdmaSendSmsMessage : public SolicitedMessage<RilSendSmsResult_t>,
                                     public add_message_id<RilRequestCdmaSendSmsMessage>
{
  public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_CDMA_SEND_SMS";

    RilRequestCdmaSendSmsMessage() = delete;
    ~RilRequestCdmaSendSmsMessage();

    inline RilRequestCdmaSendSmsMessage(const RIL_CDMA_SMS_Message& cdmaSms):
        SolicitedMessage<RilSendSmsResult_t>(get_class_message_id()) {
      mName = MESSAGE_NAME;
      mCdmaSms = cdmaSms; // structure shallow copy
    }

    const RIL_CDMA_SMS_Message& getCdmaSms();

    string dump();

  private:
    RIL_CDMA_SMS_Message mCdmaSms;
};
