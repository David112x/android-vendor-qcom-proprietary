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

/* Request to delete a CDMA SMS message on SIM card
   @Receiver: SmsModule
*/

class RilRequestCdmaDeleteSmsOnRuimMessage : public SolicitedMessage<RIL_Errno>,
                                        public add_message_id<RilRequestCdmaDeleteSmsOnRuimMessage>
{
  public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM";

    RilRequestCdmaDeleteSmsOnRuimMessage() = delete;
    ~RilRequestCdmaDeleteSmsOnRuimMessage();

    inline RilRequestCdmaDeleteSmsOnRuimMessage(int index):
            SolicitedMessage<RIL_Errno>(get_class_message_id()), mIndex(index) {
      mName = MESSAGE_NAME;
    }

    int getIndex();

    string dump();

  private:
    int mIndex;
};
