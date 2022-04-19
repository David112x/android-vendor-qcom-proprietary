/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/add_message_id.h>
#include <interfaces/QcRilRequestMessage.h>
#include "interfaces/ims/ims.h"

/*
 * Request to get the ims config from lower layers
 *
 * Response:
 *   errorCode    : Valid error codes
 *                   SUCCESS
 *                   GENERIC_FAILURE
 *   responseData : std::shared_ptr<qcril::interfaces::ConfigInfo>
 */

class QcRilRequestImsGetConfigMessage
    : public QcRilRequestMessage,
      public add_message_id<QcRilRequestImsGetConfigMessage> {
private:
  qcril::interfaces::ConfigInfo mConfigInfo;

public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestImsGetConfigMessage";

  QcRilRequestImsGetConfigMessage() = delete;

  ~QcRilRequestImsGetConfigMessage() {};

  inline QcRilRequestImsGetConfigMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  qcril::interfaces::ConfigInfo& getConfigInfo() {
    return mConfigInfo;
  }

};
