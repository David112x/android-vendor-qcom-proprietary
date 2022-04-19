/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/UnSolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"

class RilUnsolNewBroadcastSmsMessage : public UnSolicitedMessage,
                public add_message_id<RilUnsolNewBroadcastSmsMessage> {
 private:
  std::vector<uint8_t> mPayload;

 public:
  static constexpr const char *MESSAGE_NAME =
        "com.qualcomm.qti.qcril.sms.new_broadcast_sms";
  RilUnsolNewBroadcastSmsMessage() = delete;
  ~RilUnsolNewBroadcastSmsMessage();

  template<typename T>
  explicit inline RilUnsolNewBroadcastSmsMessage(T&& payload)
      : UnSolicitedMessage(get_class_message_id()), mPayload(std::forward<T>(payload)) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() override;
  string dump() override;

  std::vector<uint8_t>& getPayload();
};
