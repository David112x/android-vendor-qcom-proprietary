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
#include <string>

/*
 * Request to sets the CLIR (Calling Line Identification Restriction) / OIR (Originating
 * Indentification Restriction) parameter
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : nullptr
 */
class QcRilRequestSetClirMessage : public QcRilRequestMessage,
                                   public add_message_id<QcRilRequestSetClirMessage> {
 private:
  std::optional<uint32_t> mParamN;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestSetClirMessage";

  QcRilRequestSetClirMessage() = delete;

  ~QcRilRequestSetClirMessage() {}

  inline QcRilRequestSetClirMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  bool hasParamN() { return mParamN ? true : false; }
  uint32_t getParamN() { return *mParamN; }
  void setParamN(uint32_t val) { mParamN = val; }
  inline string dumpParamN() {
    return " mParamN = " + (mParamN ? std::to_string(*mParamN) : "<invalid>");
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() +
          "{" + dumpParamN() + "}";
  }
};
