/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/UnSolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"

class QcRilUnsolMbnConfigClearedMessage : public UnSolicitedMessage,
                public add_message_id<QcRilUnsolMbnConfigClearedMessage> {
 private:
  char mResult;

 public:
  static constexpr const char *MESSAGE_NAME =
        "com.qualcomm.qti.qcril.mbn.mbn_config_cleared";
  QcRilUnsolMbnConfigClearedMessage() = delete;
  ~QcRilUnsolMbnConfigClearedMessage() {};

  explicit inline QcRilUnsolMbnConfigClearedMessage(const char errorCode)
      : UnSolicitedMessage(get_class_message_id()), mResult(errorCode) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    return std::make_shared<QcRilUnsolMbnConfigClearedMessage>(mResult);
  }

  virtual string dump() {
    return mName;
  }

  char getResult(){
    return mResult;
  }
};
