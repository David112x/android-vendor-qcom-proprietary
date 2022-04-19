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
 * Unsol message to notify incoming RTT message
 *
 */
class QcRilUnsolImsRttMessage : public UnSolicitedMessage,
                                     public add_message_id<QcRilUnsolImsRttMessage> {
 private:
   string mRttMessage;
 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolImsRttMessage";
  ~QcRilUnsolImsRttMessage() { }

  QcRilUnsolImsRttMessage(string rttMsg)
      : UnSolicitedMessage(get_class_message_id()), mRttMessage(rttMsg) {}

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolImsRttMessage> msg =
        std::make_shared<QcRilUnsolImsRttMessage>(mRttMessage);
    return msg;
  }

  string getRttMessage() { return mRttMessage; };

  string dump() {
    return QcRilUnsolImsRttMessage::MESSAGE_NAME;
  }
};
