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

/* Request to get SMSC address Message
   @Receiver: SmsModule
*/

class RilRequestGetSmscAddressMessage : public SolicitedMessage<RilGetSmscAddrResult_t>,
                                        public add_message_id<RilRequestGetSmscAddressMessage>
{
  public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_GET_SMSC_ADDRESS";

    ~RilRequestGetSmscAddressMessage();

    inline RilRequestGetSmscAddressMessage():
            SolicitedMessage<RilGetSmscAddrResult_t>(get_class_message_id()) {
      mName = MESSAGE_NAME;
    }

    string dump();
};
