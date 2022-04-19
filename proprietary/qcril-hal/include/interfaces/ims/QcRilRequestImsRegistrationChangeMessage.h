/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include <framework/MessageContext.h>
#include <framework/add_message_id.h>
#include <interfaces/QcRilRequestMessage.h>
#include <interfaces/ims/ims.h>

/*
 * Request to change IMS Registration (enable/disable IMS)
 *
 * Response:
 *   errorCode    : Valid error codes
 *                   SUCCESS
 *                   GENERIC_FAILURE
 *   responseData : nullptr
 */
class QcRilRequestImsRegistrationChangeMessage
    : public QcRilRequestMessage,
      public add_message_id<QcRilRequestImsRegistrationChangeMessage> {
 private:
  qcril::interfaces::RegState mRegState;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestImsRegistrationChangeMessage";

  QcRilRequestImsRegistrationChangeMessage() = delete;

  ~QcRilRequestImsRegistrationChangeMessage() {}

  inline QcRilRequestImsRegistrationChangeMessage(std::shared_ptr<MessageContext> context,
                                                  qcril::interfaces::RegState regState)
      : QcRilRequestMessage(get_class_message_id(), context), mRegState(regState) {
    mName = MESSAGE_NAME;
  }
  qcril::interfaces::RegState getRegState() { return mRegState; }
};
