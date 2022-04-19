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
 * Unsol message to notify change in radio_state
 *
 */
class QcRilUnsolImsRadioStateIndication : public UnSolicitedMessage,
                                     public add_message_id<QcRilUnsolImsRadioStateIndication> {
 private:
  qcril::interfaces::RadioState mState;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolImsRadioStateIndication";
  ~QcRilUnsolImsRadioStateIndication() { }

  QcRilUnsolImsRadioStateIndication(qcril::interfaces::RadioState status)
      : UnSolicitedMessage(get_class_message_id()), mState(status) {}

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolImsRadioStateIndication> msg =
        std::make_shared<QcRilUnsolImsRadioStateIndication>(mState);
    return msg;
  }

  qcril::interfaces::RadioState getRadioState() { return mState; };

  string dump() {
    return QcRilUnsolImsRadioStateIndication::MESSAGE_NAME;
  }
};
