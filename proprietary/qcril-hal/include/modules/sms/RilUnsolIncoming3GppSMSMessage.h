/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/UnSolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"

class RilUnsolIncoming3GppSMSMessage : public UnSolicitedMessage,
                public add_message_id<RilUnsolIncoming3GppSMSMessage> {
 private:
  std::vector<uint8_t> mPayload;

 public:
  static constexpr const char *MESSAGE_NAME =
        "com.qualcomm.qti.qcril.sms.incoming_3gpp_message";
  RilUnsolIncoming3GppSMSMessage() = delete;
  ~RilUnsolIncoming3GppSMSMessage();

#if 0
  explicit inline RilUnsolIncoming3GppSMSMessage(const std::vector<uint8_t>& payload)
      : UnSolicitedMessage(get_class_message_id()), mPayload(payload) {
    mName = MESSAGE_NAME;
  }

  explicit inline RilUnsolIncoming3GppSMSMessage(std::vector<uint8_t>&& payload)
      : UnSolicitedMessage(get_class_message_id()), mPayload(std::move(payload)) {
    mName = MESSAGE_NAME;
  }
#endif
  template<typename T>
  explicit inline RilUnsolIncoming3GppSMSMessage(T&& payload)
      : UnSolicitedMessage(get_class_message_id()), mPayload(std::forward<T>(payload)) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() override;
  string dump() override;

  std::vector<uint8_t>& getPayload();
};
