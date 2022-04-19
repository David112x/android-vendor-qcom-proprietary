/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/UnSolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"

class QcRilUnsolAdnInitDoneMessage : public UnSolicitedMessage,
                public add_message_id<QcRilUnsolAdnInitDoneMessage> {
 private:

 public:
  static constexpr const char *MESSAGE_NAME =
        "com.qualcomm.qti.qcril.pbm.adn_init_done";
  ~QcRilUnsolAdnInitDoneMessage() {};

  explicit inline QcRilUnsolAdnInitDoneMessage()
      : UnSolicitedMessage(get_class_message_id()) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    return std::make_shared<QcRilUnsolAdnInitDoneMessage>();
  }

  virtual string dump() {
    return mName;
  }
};
