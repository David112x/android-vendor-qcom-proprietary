/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/UnSolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"

class RilUnsolStkCCAlphaNotifyMessage : public UnSolicitedMessage,
                public add_message_id<RilUnsolStkCCAlphaNotifyMessage> {
 private:
  std::string mPayload;

 public:
  static constexpr const char *MESSAGE_NAME =
        "com.qualcomm.qti.qcril.sms.stk_cc_alpha_notify";
  RilUnsolStkCCAlphaNotifyMessage() = delete;
  ~RilUnsolStkCCAlphaNotifyMessage();

  template<typename T>
  explicit inline RilUnsolStkCCAlphaNotifyMessage(T&& payload)
      : UnSolicitedMessage(get_class_message_id()), mPayload(std::forward<T>(payload)) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() override;
  string dump() override;

  const std::string& getPayload();
};
