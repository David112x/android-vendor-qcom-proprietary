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
 * Send a USSD message
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : nullptr
 */
class QcRilRequestSendUssdMessage : public QcRilRequestMessage,
                                    public add_message_id<QcRilRequestSendUssdMessage> {
 private:
  std::optional<std::string> mUssd;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestSendUssdMessage";

  QcRilRequestSendUssdMessage() = delete;

  ~QcRilRequestSendUssdMessage() {}

  inline QcRilRequestSendUssdMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  bool hasUssd() { return mUssd ? true : false; }
  std::string getUssd() { return mUssd ? *mUssd : ""; }
  void setUssd(std::string val) { mUssd = val; }
  inline std::string dumpUssd() { return "Ussd = " + (mUssd ? *mUssd : "<invalid>"); }

  virtual string dump() {
    return QcRilRequestMessage::dump() + "{" + dumpUssd() + "}";
  }
};
