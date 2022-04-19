/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/GenericCallback.h"
#include "framework/SolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"
#include "framework/legacy.h"
#include "modules/sms/qcril_qmi_sms_types.h"


/* Request to write GSM SMS to SIM card Message
   @Receiver: SmsModule
*/
class RilRequestWriteSmsToSimMessage : public SolicitedMessage<RilWriteSmsToSimResult_t>,
                                       public add_message_id<RilRequestWriteSmsToSimMessage>
{
  public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_WRITE_SMS_TO_SIM";

    RilRequestWriteSmsToSimMessage() = delete;
    ~RilRequestWriteSmsToSimMessage() {}

    template<typename T1, typename T2>
    explicit RilRequestWriteSmsToSimMessage(T1&& smscPdu, T2&& pdu, int status):
            SolicitedMessage<RilWriteSmsToSimResult_t>(get_class_message_id()),
            mSmscPdu(std::forward<T1>(smscPdu)), mPdu(std::forward<T2>(pdu)),
            mStatus(status) {
        mName = MESSAGE_NAME;
    }

    int getStatus();
    const string& getSmscPdu();
    const string& getPdu();

    string dump();

  private:
    const string mSmscPdu;
    const string mPdu;
    const int mStatus;
};
