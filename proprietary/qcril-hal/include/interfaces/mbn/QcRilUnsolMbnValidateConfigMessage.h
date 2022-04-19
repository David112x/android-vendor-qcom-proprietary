/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/UnSolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"

class QcRilUnsolMbnValidateConfigMessage : public UnSolicitedMessage,
                public add_message_id<QcRilUnsolMbnValidateConfigMessage> {
 private:
  std::vector<uint8_t>   mMbnDiff;

 public:
  static constexpr const char *MESSAGE_NAME =
        "com.qualcomm.qti.qcril.mbn.mbn_validate_config";
  QcRilUnsolMbnValidateConfigMessage() = delete;
  ~QcRilUnsolMbnValidateConfigMessage() {};

  explicit inline QcRilUnsolMbnValidateConfigMessage(std::vector<uint8_t>  mbnDiff)
      : UnSolicitedMessage(get_class_message_id()), mMbnDiff(mbnDiff) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    return std::make_shared<QcRilUnsolMbnValidateConfigMessage>(mMbnDiff);
  }

  virtual string dump() {
    return mName + "{" +  "mbn differences length = " + std::to_string(mMbnDiff.size())+ "}";
  }

  std::vector<uint8_t>  getMbnDiff() {
    return mMbnDiff;
  }
};
