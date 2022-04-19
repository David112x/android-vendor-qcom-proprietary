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
 * Request to activate/deactivate/query the supplementary service of an IMS Service
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : std::shared_ptr<qcril::interfaces::SipErrorInfo>
 */
class QcRilRequestSetSupsServiceMessage
    : public QcRilRequestMessage,
      public add_message_id<QcRilRequestSetSupsServiceMessage> {
 private:
  std::optional<uint32_t> mOperationType;
  std::optional<qcril::interfaces::FacilityType> mFacilityType;
  std::optional<uint32_t> mServiceClass;
  /*  Call Barring Password */
  std::optional<std::string> mPassword;
  std::optional<std::vector<std::string>> mCallBarringNumberList;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestSetSupsServiceMessage";

  QcRilRequestSetSupsServiceMessage() = delete;

  ~QcRilRequestSetSupsServiceMessage() {}

  inline QcRilRequestSetSupsServiceMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  bool hasOperationType() { return mOperationType ? true : false; }
  uint32_t getOperationType() { return *mOperationType; }
  void setOperationType(uint32_t val) { mOperationType = val; }
  inline string dumpOperationType() {
    return " mOperationType = " + (mOperationType ? std::to_string(*mOperationType) : "<invalid>");
  }

  bool hasFacilityType() { return mFacilityType ? true : false; }
  qcril::interfaces::FacilityType getFacilityType() { return *mFacilityType; }
  void setFacilityType(qcril::interfaces::FacilityType val) { mFacilityType = val; }
  inline string dumpFacilityType() {
    return " mFacilityType = " + (mFacilityType ? toString(*mFacilityType) : "<invalid>");
  }

  bool hasServiceClass() { return mServiceClass ? true : false; }
  uint32_t getServiceClass() { return *mServiceClass; }
  void setServiceClass(uint32_t val) { mServiceClass = val; }
  inline string dumpServiceClass() {
    return " mServiceClass = " + (mServiceClass ? std::to_string(*mServiceClass) : "<invalid>");
  }

  bool hasPassword() { return mPassword ? true : false; }
  std::string getPassword() { return *mPassword; }
  void setPassword(std::string val) { mPassword = val; }
  inline string dumpPassword() {
    return " mPassword = " + (mPassword ? (*mPassword) : "<invalid>");
  }

  bool hasCallBarringNumberList() { return mCallBarringNumberList ? true : false; }
  std::vector<std::string> getCallBarringNumberList() { return *mCallBarringNumberList; }
  void setCallBarringNumberList(std::vector<std::string> val) { mCallBarringNumberList = val; }
#if 0
  inline string dumpCallBarringNumberList() {
    return "mCallBarringNumberList = " +
           (mCallBarringNumberList ? std::to_string(*mCallBarringNumberList) : "<invalid>");
  }
#endif

  virtual string dump() {
    return QcRilRequestMessage::dump() +
          "{" + dumpOperationType() + dumpFacilityType() + dumpServiceClass() + dumpPassword() +
          "}";
  }
};
