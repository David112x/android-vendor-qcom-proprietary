/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/add_message_id.h>
#include <interfaces/QcRilRequestMessage.h>
#include <interfaces/voice/voice.h>
#include <optional>

/*
 * Request to add a participant to a call (to the current active call or to the held call if there
 * is no active calls).
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : nullptr
 */
class QcRilRequestImsAddParticipantMessage
    : public QcRilRequestMessage,
      public add_message_id<QcRilRequestImsAddParticipantMessage> {
 private:
  // sip uri
  std::optional<std::string> mAddress;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestImsAddParticipantMessage";

  QcRilRequestImsAddParticipantMessage() = delete;

  ~QcRilRequestImsAddParticipantMessage() {}

  inline QcRilRequestImsAddParticipantMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  bool hasAddress() { return mAddress ? true : false; }
  std::string getAddress() { return mAddress ? *mAddress : ""; }
  void setAddress(std::string val) { mAddress = val; }
  string dumpAddress() { return "mAddress = " + (mAddress ? *mAddress : "<invalid>"); }

  virtual string dump() {
    return QcRilRequestMessage::dump() +
          "{" + dumpAddress() + "}";
  }
};
