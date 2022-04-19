/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/add_message_id.h>
#include <interfaces/QcRilRequestMessage.h>

/*
 * Request to queries the status of the CLIP (Calling Line Identification Restriction) / OIP
 * (Originating Identification Presentation) supplementary service
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : std::shared_ptr<qcril::interfaces::ClipInfo>
 */
class QcRilRequestGetClipMessage : public QcRilRequestMessage,
                                   public add_message_id<QcRilRequestGetClipMessage> {
 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestGetClipMessage";

  QcRilRequestGetClipMessage() = delete;

  ~QcRilRequestGetClipMessage() {}

  inline QcRilRequestGetClipMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() + "{}";
  }
};
