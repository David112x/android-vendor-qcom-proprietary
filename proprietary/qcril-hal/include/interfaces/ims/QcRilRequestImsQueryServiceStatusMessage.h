/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include <framework/add_message_id.h>
#include <interfaces/QcRilRequestMessage.h>

/*
 * Request to get the status of IMS Services
 *
 * Response:
 *   errorCode    : Valid error codes
 *                   SUCCESS
 *                   GENERIC_FAILURE
 *   responseData : std::shared_ptr<qcril::interfaces::ServiceStatusInfoList>
 */
class QcRilRequestImsQueryServiceStatusMessage
    : public QcRilRequestMessage,
      public add_message_id<QcRilRequestImsQueryServiceStatusMessage> {
 private:
 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestImsQueryServiceStatusMessage";

  QcRilRequestImsQueryServiceStatusMessage() = delete;

  ~QcRilRequestImsQueryServiceStatusMessage() {}

  inline QcRilRequestImsQueryServiceStatusMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }
};
