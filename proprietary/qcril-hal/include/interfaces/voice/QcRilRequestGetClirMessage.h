/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/add_message_id.h>
#include <interfaces/QcRilRequestMessage.h>

/*
 * Request to queries the status of the CLIR (Calling Line Identification Restriction) / OIR
 * (Originating Indentification Restriction)
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : std::shared_ptr<qcril::interfaces::ClirInfo>
 */
class QcRilRequestGetClirMessage : public QcRilRequestMessage,
                                   public add_message_id<QcRilRequestGetClirMessage> {
 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestGetClirMessage";

  QcRilRequestGetClirMessage() = delete;

  ~QcRilRequestGetClirMessage() {}

  inline QcRilRequestGetClirMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() + "{}";
  }
};
