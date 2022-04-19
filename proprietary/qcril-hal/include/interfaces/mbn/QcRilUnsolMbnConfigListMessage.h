/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/UnSolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"

class QcRilUnsolMbnConfigListMessage : public UnSolicitedMessage,
                public add_message_id<QcRilUnsolMbnConfigListMessage> {
 private:
  std::vector<uint8_t>  mResultArray;

 public:
  static constexpr const char *MESSAGE_NAME =
        "com.qualcomm.qti.qcril.mbn.mbn_config_list";
  QcRilUnsolMbnConfigListMessage() = delete;
  ~QcRilUnsolMbnConfigListMessage() {};

  explicit inline QcRilUnsolMbnConfigListMessage(std::vector<uint8_t> resultArray)
      : UnSolicitedMessage(get_class_message_id()), mResultArray(resultArray) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    return std::make_shared<QcRilUnsolMbnConfigListMessage>(mResultArray);
  }

  virtual string dump() {
    return mName + "{" +  "result array length = " + std::to_string(mResultArray.size()) + "}";
  }

  std::vector<uint8_t> getResult() {
    return mResultArray;
  }
};
