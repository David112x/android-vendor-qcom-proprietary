/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>
#include <interfaces/ims/ims.h>

/*
 * Unsol message to notify change in sub config
 *
 */
class QcRilUnsolImsRegBlockStatusMessage : public UnSolicitedMessage,
                                     public add_message_id<QcRilUnsolImsRegBlockStatusMessage> {
 private:
  std::shared_ptr<qcril::interfaces::RegistrationBlockStatus> mBlockStatus;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolImsRegBlockStatusMessage";
  ~QcRilUnsolImsRegBlockStatusMessage() { mBlockStatus = nullptr; }

  QcRilUnsolImsRegBlockStatusMessage(std::shared_ptr<qcril::interfaces::RegistrationBlockStatus> info)
      : UnSolicitedMessage(get_class_message_id()), mBlockStatus(info) {}

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolImsRegBlockStatusMessage> msg =
        std::make_shared<QcRilUnsolImsRegBlockStatusMessage>(mBlockStatus);
    return msg;
  }

  std::shared_ptr<qcril::interfaces::RegistrationBlockStatus> getBlockStatus() { return mBlockStatus; };

  void setBlockStatus(std::shared_ptr<qcril::interfaces::RegistrationBlockStatus> info) {
    mBlockStatus = info;
  }

  string dump() {
    return QcRilUnsolImsRegBlockStatusMessage::MESSAGE_NAME;
  }
};
