/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/UnSolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"

class QcRilUnsolMbnConfigResultMessage : public UnSolicitedMessage,
                public add_message_id<QcRilUnsolMbnConfigResultMessage> {
 private:
  char mResult;

 public:
  static constexpr const char *MESSAGE_NAME =
        "com.qualcomm.qti.qcril.mbn.mbn_config_result";
  QcRilUnsolMbnConfigResultMessage() = delete;
  ~QcRilUnsolMbnConfigResultMessage() {};

  explicit inline QcRilUnsolMbnConfigResultMessage(const char errorCode)
      : UnSolicitedMessage(get_class_message_id()), mResult(errorCode) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    return std::make_shared<QcRilUnsolMbnConfigResultMessage>(mResult);
  }

  virtual string dump() {
    return mName;
  }

  char getResult() {
    return mResult;
  }
};
