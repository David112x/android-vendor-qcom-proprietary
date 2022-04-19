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
#include <string>
/*
 * Request to query multi identity lines
 *
 * Response:
 *   errorCode    : Valid error codes
 *                   SUCCESS
 *                   GENERIC_FAILURE
 *   responseData : qcril::interfaces::VirtualLineInfo
 */
class QcRilRequestImsQueryVirtualLineInfo
    : public QcRilRequestMessage,
      public add_message_id<QcRilRequestImsQueryVirtualLineInfo> {
 private:
  std::string mMsisdn;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestImsQueryVirtualLineInfo";

  QcRilRequestImsQueryVirtualLineInfo() = delete;

  ~QcRilRequestImsQueryVirtualLineInfo() {}

  inline QcRilRequestImsQueryVirtualLineInfo(std::shared_ptr<MessageContext> context,
                                   std::string msisdn)
      : QcRilRequestMessage(get_class_message_id(), context), mMsisdn(msisdn) {
    mName = MESSAGE_NAME;
  }
  std::string& getMsisdn() {
    return mMsisdn;
  }
};
