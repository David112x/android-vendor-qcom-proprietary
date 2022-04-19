/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/UnSolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"

class RilUnsolNewSmsOnSimMessage : public UnSolicitedMessage,
                public add_message_id<RilUnsolNewSmsOnSimMessage> {
 private:
  int32_t mRecordNumber;

 public:
  static constexpr const char *MESSAGE_NAME =
        "com.qualcomm.qti.qcril.sms.new_sms_on_sim";
  RilUnsolNewSmsOnSimMessage() = delete;
  ~RilUnsolNewSmsOnSimMessage();

  explicit inline RilUnsolNewSmsOnSimMessage(const int32_t recordNumber)
      : UnSolicitedMessage(get_class_message_id()), mRecordNumber(recordNumber) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() override;
  string dump() override;

  int32_t getRecordNumber();
};
