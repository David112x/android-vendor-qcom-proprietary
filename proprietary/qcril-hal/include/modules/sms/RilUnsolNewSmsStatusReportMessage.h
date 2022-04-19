/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/UnSolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"

class RilUnsolNewSmsStatusReportMessage : public UnSolicitedMessage,
                public add_message_id<RilUnsolNewSmsStatusReportMessage> {
 private:
  std::vector<uint8_t> mPayload;

 public:
  static constexpr const char *MESSAGE_NAME =
        "com.qualcomm.qti.qcril.sms.unsol_new_sms_status_report";
  RilUnsolNewSmsStatusReportMessage() = delete;
  ~RilUnsolNewSmsStatusReportMessage();

  template<typename T>
  explicit inline RilUnsolNewSmsStatusReportMessage(T&& payload)
      : UnSolicitedMessage(get_class_message_id()), mPayload(std::forward<T>(payload)) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() override;
  string dump() override;

  std::vector<uint8_t>& getPayload();
};
