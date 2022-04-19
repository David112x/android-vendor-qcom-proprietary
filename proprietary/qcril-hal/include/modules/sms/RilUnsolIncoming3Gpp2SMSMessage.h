/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/UnSolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"
#include "telephony/ril.h"


class RilUnsolIncoming3Gpp2SMSMessage : public UnSolicitedMessage,
                public add_message_id<RilUnsolIncoming3Gpp2SMSMessage> {
 private:
  RIL_CDMA_SMS_Message mCdmaSms;

 public:
  static constexpr const char *MESSAGE_NAME =
        "com.qualcomm.qti.qcril.sms.incoming_3gpp2_message";
  RilUnsolIncoming3Gpp2SMSMessage() = delete;
  ~RilUnsolIncoming3Gpp2SMSMessage();

  explicit inline RilUnsolIncoming3Gpp2SMSMessage(const RIL_CDMA_SMS_Message& cdmaSms)
      : UnSolicitedMessage(get_class_message_id()) {
    mName = MESSAGE_NAME;
    mCdmaSms = cdmaSms; // copy
  }

  std::shared_ptr<UnSolicitedMessage> clone() override;
  string dump() override;

  RIL_CDMA_SMS_Message& getCdmaSms();
};
