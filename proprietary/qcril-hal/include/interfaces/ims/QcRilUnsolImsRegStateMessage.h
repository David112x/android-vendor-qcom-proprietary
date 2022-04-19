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
 * Unsol message to notify change in IMS registration state
 *
 */
class QcRilUnsolImsRegStateMessage : public UnSolicitedMessage,
                                     public add_message_id<QcRilUnsolImsRegStateMessage> {
 private:
  std::shared_ptr<qcril::interfaces::Registration> mRegistration;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolImsRegStateMessage";
  ~QcRilUnsolImsRegStateMessage() { mRegistration = nullptr; }

  QcRilUnsolImsRegStateMessage(std::shared_ptr<qcril::interfaces::Registration> reg)
      : UnSolicitedMessage(get_class_message_id()), mRegistration(reg) {}

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolImsRegStateMessage> msg =
        std::make_shared<QcRilUnsolImsRegStateMessage>(mRegistration);
    return msg;
  }

  void setRegistration(std::shared_ptr<qcril::interfaces::Registration> reg) {
    mRegistration = reg;
  }

  std::shared_ptr<qcril::interfaces::Registration> getRegistration() { return mRegistration; };

  string dump() {
    // TODO log Registration also
    return QcRilUnsolImsRegStateMessage::MESSAGE_NAME;
  }
};
