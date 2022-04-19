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
 * Request to set the ims sub config from lower layers
 *
 * Response:
 *   errorCode    : Valid error codes
 *                   SUCCESS
 *                   GENERIC_FAILURE
 *   responseData : none
 */


class QcRilRequestImsSetConfigMessage
    : public QcRilRequestMessage,
      public add_message_id<QcRilRequestImsSetConfigMessage> {
private:
  qcril::interfaces::ConfigInfo mConfigInfo;

public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestImsSetConfigMessage";

  QcRilRequestImsSetConfigMessage() = delete;

  ~QcRilRequestImsSetConfigMessage() {};

  inline QcRilRequestImsSetConfigMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  qcril::interfaces::ConfigInfo& getConfigInfo() {
    return mConfigInfo;
  }

  virtual string dump() {
    std::string os;
    os += QcRilRequestMessage::dump();
    os += "{";
    os += ".mConfigInfo=" + toString(mConfigInfo);
    os += "}";
    return os;
  }
};
