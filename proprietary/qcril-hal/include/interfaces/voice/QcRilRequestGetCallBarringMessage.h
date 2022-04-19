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
 * Request to query status of a facility lock state
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : std::shared_ptr<qcril::interfaces::SuppServiceStatusInfo>
 */
class QcRilRequestGetCallBarringMessage
    : public QcRilRequestMessage,
      public add_message_id<QcRilRequestGetCallBarringMessage> {
 private:
  std::optional<qcril::interfaces::FacilityType> mFacilityType;
  std::optional<uint32_t> mServiceClass;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestGetCallBarringMessage";

  QcRilRequestGetCallBarringMessage() = delete;

  ~QcRilRequestGetCallBarringMessage() {}

  inline QcRilRequestGetCallBarringMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  bool hasFacilityType() { return mFacilityType ? true : false; }
  qcril::interfaces::FacilityType getFacilityType() { return *mFacilityType; }
  void setFacilityType(qcril::interfaces::FacilityType val) { mFacilityType = val; }
  inline string dumpFacilityType() {
    return "mFacilityType = " + (mFacilityType ? toString(*mFacilityType) : "<invalid>");
  }

  bool hasServiceClass() { return mServiceClass ? true : false; }
  uint32_t getServiceClass() { return *mServiceClass; }
  void setServiceClass(uint32_t val) { mServiceClass = val; }
  inline string dumpServiceClass() {
    return "mServiceClass = " + (mServiceClass ? std::to_string(*mServiceClass) : "<invalid>");
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() +
          "{" + dumpFacilityType() + dumpServiceClass() + "}";
  }
};
