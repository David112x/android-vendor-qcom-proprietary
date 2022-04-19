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
 * Request to deflect the call
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : nullptr
 */
class QcRilRequestImsDeflectCallMessage : public QcRilRequestMessage,
                                          public add_message_id<QcRilRequestImsDeflectCallMessage> {
 private:
  std::optional<uint32_t> mCallId;
  std::optional<std::string> mNumber;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestImsDeflectCallMessage";

  QcRilRequestImsDeflectCallMessage() = delete;

  ~QcRilRequestImsDeflectCallMessage() {}

  inline QcRilRequestImsDeflectCallMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  bool hasCallId() { return mCallId ? true : false; }
  uint32_t getCallId() { return *mCallId; }
  void setCallId(uint32_t val) { mCallId = val; }
  inline string dumpCallId() {
    return "mCallId = " + (mCallId ? std::to_string(*mCallId) : "<invalid>");
  }

  bool hasNumber() { return mNumber ? true : false; }
  std::string getNumber() { return *mNumber; }
  void setNumber(std::string val) { mNumber = val; }
  inline string dumpNumber() {
    return "mNumber = " + (mNumber ? (*mNumber) : "<invalid>");
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() +
          "{" + dumpCallId() + dumpNumber() + "}";
  }
};
