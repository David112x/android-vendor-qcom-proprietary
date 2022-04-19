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

/* Request to set SMSC address Message
   @Receiver: SmsModule
*/

class RilRequestSetSmscAddressMessage : public SolicitedMessage<RIL_Errno>,
                                        public add_message_id<RilRequestSetSmscAddressMessage>
{
  public:
    static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_SET_SMSC_ADDRESS";

    RilRequestSetSmscAddressMessage() = delete;
    ~RilRequestSetSmscAddressMessage();

    template<typename T>
    explicit RilRequestSetSmscAddressMessage(T&& addr):
            SolicitedMessage<RIL_Errno>(get_class_message_id()),
            mSmscAddr(std::forward<T>(addr)) {
      mName = MESSAGE_NAME;
    }

    const string& getSmscAddr();

    string dump();

  private:
    string mSmscAddr;
};
