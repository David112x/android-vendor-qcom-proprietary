/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>
#include <interfaces/voice/voice.h>
#include <optional>

/**
 * Unsol message to notify supplementary service indications
 */
class QcRilUnsolSupplementaryServiceMessage
    : public UnSolicitedMessage,
      public add_message_id<QcRilUnsolSupplementaryServiceMessage> {
 private:

  std::optional<RIL_SsServiceType> mServiceType;
  std::optional<RIL_SsRequestType> mRequestType;
  std::optional<RIL_SsTeleserviceType> mTeleserviceType;
  std::optional<int> mServiceClass;
  std::optional<RIL_Errno> mResult;
  // SS_INFO_MAX 4
  std::optional<std::vector<int>> mSuppSrvInfoList;
  // NUM_SERVICE_CLASSES 7
  std::optional<std::vector<qcril::interfaces::CallForwardInfo>> mCallForwardInfoList;
  std::optional<std::vector<std::string>> mBarredNumberList;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolSupplementaryServiceMessage";
  ~QcRilUnsolSupplementaryServiceMessage() {}

  QcRilUnsolSupplementaryServiceMessage() : UnSolicitedMessage(get_class_message_id()) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolSupplementaryServiceMessage> msg =
        std::make_shared<QcRilUnsolSupplementaryServiceMessage>();
    return msg;
  }

  bool hasServiceType() { return mServiceType ? true : false; }
  RIL_SsServiceType getServiceType() { return *mServiceType; }
  void setServiceType(RIL_SsServiceType val) { mServiceType = val; }
  inline std::string dumpServiceType() {
    return " mServiceType = " + (mServiceType ? std::to_string(*mServiceType) : "<invalid>");
  }

  bool hasRequestType() { return mRequestType ? true : false; }
  RIL_SsRequestType getRequestType() { return *mRequestType; }
  void setRequestType(RIL_SsRequestType val) { mRequestType = val; }
  inline std::string dumpRequestType() {
    return " mRequestType = " + (mRequestType ? std::to_string(*mRequestType) : "<invalid>");
  }

  bool hasTeleserviceType() { return mTeleserviceType ? true : false; }
  RIL_SsTeleserviceType getTeleserviceType() { return *mTeleserviceType; }
  void setTeleserviceType(RIL_SsTeleserviceType val) { mTeleserviceType = val; }
  inline std::string dumpTeleserviceType() {
    return " mTeleserviceType = " +
           (mTeleserviceType ? std::to_string(*mTeleserviceType) : "<invalid>");
  }

  bool hasServiceClass() { return mServiceClass ? true : false; }
  int getServiceClass() { return *mServiceClass; }
  void setServiceClass(int val) { mServiceClass = val; }
  inline std::string dumpServiceClass() {
    return " mServiceClass = " + (mServiceClass ? std::to_string(*mServiceClass) : "<invalid>");
  }

  bool hasResult() { return mResult ? true : false; }
  RIL_Errno getResult() { return *mResult; }
  void setResult(RIL_Errno val) { mResult = val; }
  inline std::string dumpResult() {
    return " mResult = " + (mResult ? std::to_string(*mResult) : "<invalid>");
  }

  bool hasSuppSrvInfoList() { return mSuppSrvInfoList ? true : false; }
  std::vector<int> getSuppSrvInfoList() { return *mSuppSrvInfoList; }
  void setSuppSrvInfoList(std::vector<int> val) { mSuppSrvInfoList = val; }
#if 0
  inline std::string dumpSuppSrvInfoList() {
    return " mSuppSrvInfoList = " +
           (mSuppSrvInfoList ? (*mSuppSrvInfoList) : "<invalid>");
  }
#endif

  bool hasCallForwardInfoList() { return mCallForwardInfoList ? true : false; }
  std::vector<qcril::interfaces::CallForwardInfo> getCallForwardInfoList() { return *mCallForwardInfoList; }
  void setCallForwardInfoList(std::vector<qcril::interfaces::CallForwardInfo> val) { mCallForwardInfoList = val; }
#if 0
  inline std::string dumpCallForwardInfoList() {
    return " mCallForwardInfoList = " +
           (mCallForwardInfoList ? (*mCallForwardInfoList) : "<invalid>");
  }
#endif

  bool hasBarredNumberList() { return mBarredNumberList ? true : false; }
  std::vector<std::string> getBarredNumberList() { return *mBarredNumberList; }
  void setBarredNumberList(std::vector<std::string> val) { mBarredNumberList = val; }
#if 0
  inline std::string dumpBarredNumberList() {
    return " mBarredNumberList = " +
           (mBarredNumberList ? (*mBarredNumberList) : "<invalid>");
  }
#endif

  std::string dump() {
    return mName + "{isIms=" + (mIsIms ? "true" : "false") + dumpServiceType() + dumpRequestType() +
           dumpTeleserviceType() + dumpServiceClass() + dumpResult() + "}";
  }

  // To distinguish IMS or CS indication
 private:
  bool mIsIms = false;

 public:
  bool isIms() { return mIsIms; }
  void setIsIms(bool val) { mIsIms = val; }
};
