/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/add_message_id.h>
#include <interfaces/QcRilRequestMessage.h>

/*
 * Request to send rtt message to lower layers
 *
 * Response:
 *   errorCode    : Valid error codes
 *                   SUCCESS
 *                   GENERIC_FAILURE
 *   responseData : none
 */


class QcRilRequestImsSendRttMessage
    : public QcRilRequestMessage,
      public add_message_id<QcRilRequestImsSendRttMessage> {
private:
  string mRttMessage;

public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestImsSendRttMessage";

  QcRilRequestImsSendRttMessage() = delete;

  ~QcRilRequestImsSendRttMessage() {};

  inline QcRilRequestImsSendRttMessage(std::shared_ptr<MessageContext> context,string rttMessage)
      : QcRilRequestMessage(get_class_message_id(), context), mRttMessage(rttMessage) {
    mName = MESSAGE_NAME;
  }

  string& getRttMessage() {
    return mRttMessage;
  }
};
