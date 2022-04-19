/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/add_message_id.h>
#include <interfaces/QcRilRequestMessage.h>

/*
 * Request to queries the status of the COLR (Connected Line Identification Restriction) / TIR
 * (Terminating Identification Restriction)
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : std::shared_ptr<qcril::interfaces::ColrInfo>
 */
class QcRilRequestGetColrMessage : public QcRilRequestMessage,
                                   public add_message_id<QcRilRequestGetColrMessage> {
 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestGetColrMessage";

  QcRilRequestGetColrMessage() = delete;

  ~QcRilRequestGetColrMessage() {}

  inline QcRilRequestGetColrMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() + "{}";
  }
};
