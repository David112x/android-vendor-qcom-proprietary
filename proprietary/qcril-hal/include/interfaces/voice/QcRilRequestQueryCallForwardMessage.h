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
#include <vector>

/*
 * Query call forwarding status
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : std::shared_ptr<qcril::interfaces::GetCallForwardRespData>
 */
class QcRilRequestQueryCallForwardMessage
    : public QcRilRequestMessage,
      public add_message_id<QcRilRequestQueryCallForwardMessage> {
 private:
  std::optional<uint32_t> mReason;
  std::optional<uint32_t> mServiceClass;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestQueryCallForwardMessage";

  QcRilRequestQueryCallForwardMessage() = delete;

  ~QcRilRequestQueryCallForwardMessage() {}

  inline QcRilRequestQueryCallForwardMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  bool hasReason() { return mReason ? true : false; }
  uint32_t getReason() { return *mReason; }
  void setReason(uint32_t val) { mReason = val; }
  inline string dumpReason() {
    return " mReason = " + (mReason ? std::to_string(*mReason) : "<invalid>");
  }

  bool hasServiceClass() { return mServiceClass ? true : false; }
  uint32_t getServiceClass() { return *mServiceClass; }
  void setServiceClass(uint32_t val) { mServiceClass = val; }
  inline string dumpServiceClass() {
    return " mServiceClass = " + (mServiceClass ? std::to_string(*mServiceClass) : "<invalid>");
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() +
          "{" + dumpReason() + dumpServiceClass() + "}";
  }
};
