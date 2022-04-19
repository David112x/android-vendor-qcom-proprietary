/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/add_message_id.h>
#include <interfaces/QcRilRequestMessage.h>

/*
 * Request to get current IMS registration state
 *
 * Response:
 *   errorCode    : Valid error codes
 *                   SUCCESS
 *                   GENERIC_FAILURE
 *   responseData : std::shared_ptr<qcril::interfaces::Registration>
 */
class QcRilRequestImsGetRegStateMessage : public QcRilRequestMessage,
                                          public add_message_id<QcRilRequestImsGetRegStateMessage> {
 private:
 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestImsGetRegStateMessage";

  QcRilRequestImsGetRegStateMessage() = delete;

  ~QcRilRequestImsGetRegStateMessage() {}

  inline QcRilRequestImsGetRegStateMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }
};
