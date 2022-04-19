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
#include "framework/legacy.h"
#include "modules/sms/qcril_qmi_sms_types.h"


/* Request to write CDMA SMS to SIM card
   @Receiver: SmsModule
*/
class RilRequestCdmaWriteSmsToRuimMessage : public SolicitedMessage<RilWriteSmsToSimResult_t>,
                                       public add_message_id<RilRequestCdmaWriteSmsToRuimMessage>
{
  public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM";

    RilRequestCdmaWriteSmsToRuimMessage() = delete;
    ~RilRequestCdmaWriteSmsToRuimMessage() {}

    explicit RilRequestCdmaWriteSmsToRuimMessage(const RIL_CDMA_SMS_Message& cdmaSms, int status):
        SolicitedMessage<RilWriteSmsToSimResult_t>(get_class_message_id()), mStatus(status) {
      mName = MESSAGE_NAME;
      mCdmaSms = cdmaSms; // structure shallow copy
    }

    int getStatus();
    RIL_CDMA_SMS_Message& getCdmaSms();

    string dump();

  private:
    RIL_CDMA_SMS_Message mCdmaSms;
    const int mStatus;
};
