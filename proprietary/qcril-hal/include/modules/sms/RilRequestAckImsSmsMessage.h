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
#include "interfaces/sms/sms.h"

/* Request to acknowlege a IMS SMS MT message (including GSM and CDMA)
   @Receiver: SmsModule
*/

class RilRequestAckImsSmsMessage : public SolicitedMessage<RIL_Errno>,
                                   public add_message_id<RilRequestAckImsSmsMessage>
{
  public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_IMS_SMS_ACKNOWLEDGE";

    RilRequestAckImsSmsMessage() = delete;
    ~RilRequestAckImsSmsMessage();

    inline RilRequestAckImsSmsMessage(uint32_t messageRef, qcril::interfaces::DeliverStatus status):
            SolicitedMessage<RIL_Errno>(get_class_message_id()),
            mMessageRef(messageRef), mDeliverStatus(status) {
      mName = MESSAGE_NAME;
    }

    uint32_t getMessageRef();
    qcril::interfaces::DeliverStatus getDeliverStatus();

    string dump();

  private:
    uint32_t mMessageRef;
    qcril::interfaces::DeliverStatus mDeliverStatus;
};
