/******************************************************************************
#  Copyright (c) 2020 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/add_message_id.h>
#include <interfaces/QcRilRequestMessage.h>

/*
 * Request to query the current state of the uplink mute setting
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : std::shared_ptr<qcril::interfaces::MuteInfo>
 */
class QcRilRequestGetMuteMessage : public QcRilRequestMessage,
                                   public add_message_id<QcRilRequestGetMuteMessage> {
 public:
  static constexpr const char* MESSAGE_NAME = "QcRilRequestGetMuteMessage";

  QcRilRequestGetMuteMessage() = delete;

  ~QcRilRequestGetMuteMessage() {
  }

  inline QcRilRequestGetMuteMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  virtual string dump() {
    std::string os;
    os += QcRilRequestMessage::dump();
    os += "{}";
    return os;
  }
};
