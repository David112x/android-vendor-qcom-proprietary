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
 * Request to enables/disables supplementary service related notifications from the network
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : nullptr
 */
class QcRilRequestSetSuppSvcNotificationMessage
    : public QcRilRequestMessage,
      public add_message_id<QcRilRequestSetSuppSvcNotificationMessage> {
 private:
  std::optional<bool> mStatus;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestSetSuppSvcNotificationMessage";

  QcRilRequestSetSuppSvcNotificationMessage() = delete;

  ~QcRilRequestSetSuppSvcNotificationMessage() {}

  inline QcRilRequestSetSuppSvcNotificationMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  bool hasStatus() { return mStatus ? true : false; }
  bool getStatus() { return *mStatus; }
  void setStatus(bool val) { mStatus = val; }
  inline string dumpStatus() {
    return std::string(" mStatus = ") + (mStatus ? (*mStatus ? "true" : "false") : "<invalid>");
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() +
          "{" + dumpStatus() + "}";
  }
};
