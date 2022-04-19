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

/* Request to report SMS memory status Message
   @Receiver: SmsModule
*/

class RilRequestReportSmsMemoryStatusMessage : public SolicitedMessage<RIL_Errno>,
                                        public add_message_id<RilRequestReportSmsMemoryStatusMessage>
{
  public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_REPORT_SMS_MEMORY_STATUS";

    RilRequestReportSmsMemoryStatusMessage() = delete;
    ~RilRequestReportSmsMemoryStatusMessage();

    inline RilRequestReportSmsMemoryStatusMessage(bool available):
            SolicitedMessage<RIL_Errno>(get_class_message_id()), mAvailable(available) {
      mName = MESSAGE_NAME;
    }

    bool isAvailable();

    string dump();

  private:
    bool mAvailable;
};
