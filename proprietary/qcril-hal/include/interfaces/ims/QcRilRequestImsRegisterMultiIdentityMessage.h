/******************************************************************************
#  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include <framework/MessageContext.h>
#include <framework/add_message_id.h>
#include <interfaces/QcRilRequestMessage.h>
#include <interfaces/ims/ims.h>
#include <vector>
/*
 * Request to register multi identity lines
 *
 * Response:
 *   errorCode    : Valid error codes
 *                   SUCCESS
 *                   GENERIC_FAILURE
 *   responseData : nullptr
 */
class QcRilRequestImsRegisterMultiIdentityMessage
    : public QcRilRequestMessage,
      public add_message_id<QcRilRequestImsRegisterMultiIdentityMessage> {
 private:
  std::vector<qcril::interfaces::MultiIdentityInfo> mLineInfo;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestImsRegisterMultiIdentityMessage";

  QcRilRequestImsRegisterMultiIdentityMessage() = delete;

  ~QcRilRequestImsRegisterMultiIdentityMessage() {}

  inline QcRilRequestImsRegisterMultiIdentityMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }
  std::vector<qcril::interfaces::MultiIdentityInfo> &getLineInfo() {
    return mLineInfo;
  }
};
