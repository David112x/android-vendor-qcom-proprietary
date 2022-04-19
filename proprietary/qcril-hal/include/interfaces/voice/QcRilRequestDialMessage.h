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
 * Request to make a MO call
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : nullptr
 */
class QcRilRequestDialMessage : public QcRilRequestMessage {
 private:
  // Calling number/address
  std::optional<std::string> mAddress;
  std::optional<int> mClir;
  std::optional<std::shared_ptr<qcril::interfaces::UusInfo>> mUusInfo;
  std::optional<qcril::interfaces::CallType> mCallType;
  std::optional<qcril::interfaces::CallDomain> mCallDomain;
  // std::optional<Presentation> mPresentation;
  std::optional<bool> mIsConferenceUri;
  std::optional<bool> mIsCallPull;
  std::optional<bool> mIsEncrypted;
  std::optional<std::string> mDisplayText;
  std::optional<qcril::interfaces::RttMode> mRttMode;
  std::optional<std::string> mOriginatingNumber;
  std::optional<bool> mIsSecondary;
  std::optional<bool> mIsEmergency;
  std::optional<unsigned int> mCategories;
  std::vector<std::string> mUrns;
  std::optional<qcril::interfaces::EmergencyCallRouting> mRouting;
  std::optional<bool> mIsIntentionEcc;  /* For resolving ambiguity */
  std::optional<bool> mIsForEccTesting; /* For test purpose */
  std::optional<qcril::interfaces::CallFailCause> mRetryCallFailReason; /* Reason for which previous call failed */
  std::optional<RIL_RadioTechnology> mRetryCallFailMode; /* Mode on which previous call failed */

 public:
  QcRilRequestDialMessage() = delete;

  ~QcRilRequestDialMessage() {}

  inline QcRilRequestDialMessage(message_id_t msg_id, std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(msg_id, context) {}

  bool hasAddress() { return mAddress ? true : false; }
  std::string getAddress() { return mAddress ? *mAddress : ""; }
  void setAddress(std::string val) { mAddress = val; }
  inline string dumpAddress() { return " mAddress = " + (mAddress ? (*mAddress) : "<invalid>"); }

  bool hasClir() { return mClir ? true : false; }
  int getClir() { return *mClir; }
  void setClir(int val) { mClir = val; }
  inline string dumpClir() { return " mClir = " + (mClir ? std::to_string(*mClir) : "<invalid>"); }

  bool hasUusInfo() { return mUusInfo ? true : false; }
  std::shared_ptr<qcril::interfaces::UusInfo> getUusInfo() { return *mUusInfo; }
  void setUusInfo(std::shared_ptr<qcril::interfaces::UusInfo> val) { mUusInfo = val; }
  //inline string dumpUusInfo() { return " mUusInfo = " + (mUusInfo ? toString(*mUusInfo) : "<invalid>"); }

  bool hasCallType() { return mCallType ? true : false; }
  qcril::interfaces::CallType getCallType() { return *mCallType; }
  void setCallType(qcril::interfaces::CallType val) { mCallType = val; }
  inline string dumpCallType() {
    return " mCallType = " + (mCallType ? toString(*mCallType) : "<invalid>");
  }

  bool hasCallDomain() { return mCallDomain ? true : false; }
  qcril::interfaces::CallDomain getCallDomain() { return *mCallDomain; }
  void setCallDomain(qcril::interfaces::CallDomain val) { mCallDomain = val; }
  inline string dumpCallDomain() {
    return " mCallDomain = " + (mCallDomain ? toString(*mCallDomain) : "<invalid>");
  }

  bool hasIsConferenceUri() { return mIsConferenceUri ? true : false; }
  bool getIsConferenceUri() { return *mIsConferenceUri; }
  void setIsConferenceUri(bool val) { mIsConferenceUri = val; }
  inline string dumpIsConferenceUri() {
    return std::string(" mIsConferenceUri = ") +
           (mIsConferenceUri ? (*mIsConferenceUri ? "true" : "false") : "<invalid>");
  }

  bool hasIsCallPull() { return mIsCallPull ? true : false; }
  bool getIsCallPull() { return *mIsCallPull; }
  void setIsCallPull(bool val) { mIsCallPull = val; }
  inline string dumpIsCallPull() {
    return std::string(" mIsCallPull = ") +
           (mIsCallPull ? (*mIsCallPull ? "true" : "false") : "<invalid>");
  }

  bool hasIsEncrypted() { return mIsEncrypted ? true : false; }
  bool getIsEncrypted() { return *mIsEncrypted; }
  void setIsEncrypted(bool val) { mIsEncrypted = val; }
  inline string dumpIsEncrypted() {
    return std::string(" mIsEncrypted = ") +
           (mIsEncrypted ? (*mIsEncrypted ? "true" : "false") : "<invalid>");
  }

  bool hasDisplayText() { return mDisplayText ? true : false; }
  std::string getDisplayText() { return mDisplayText ? *mDisplayText : ""; }
  void setDisplayText(std::string val) { mDisplayText = val; }
  inline string dumpDisplayText() {
    return " mDisplayText = " + (mDisplayText ? (*mDisplayText) : "<invalid>");
  }

  bool hasRttMode() { return mRttMode ? true : false; }
  qcril::interfaces::RttMode getRttMode() { return *mRttMode; }
  void setRttMode(qcril::interfaces::RttMode val) { mRttMode = val; }
  inline string dumpRttMode() {
    return " mRttMode = " + (mRttMode ? toString(*mRttMode) : "<invalid>");
  }

  bool hasIsSecondary() { return mIsSecondary ? true : false; }
  bool getIsSecondary() { return *mIsSecondary; }
  void setIsSecondary(bool val) { mIsSecondary = val; }
  inline string dumpIsSecondary() {
    return std::string(" mIsSecondary = ") +
           (mIsSecondary ? (*mIsSecondary ? "true" : "false") : "<invalid>");
  }

  bool hasOriginatingNumber() { return mOriginatingNumber ? true : false; }
  std::string getOriginatingNumber() { return mOriginatingNumber ? *mOriginatingNumber : ""; }
  void setOriginatingNumber(std::string val) { mOriginatingNumber = val; }
  inline string dumpOriginatingNumber() {
    return " mOriginatingNumber = " + (mOriginatingNumber ? (*mOriginatingNumber) : "<invalid>");
  }

  bool hasIsEmergency() { return mIsEmergency ? true : false; }
  bool getIsEmergency() { return *mIsEmergency; }
  void setIsEmergency(bool val) { mIsEmergency = val; }
  inline string dumpIsEmergency() {
     return std::string(" mIsEmergency = ") +
           (mIsEmergency ? (*mIsEmergency ? "true" : "false") : "<invalid>");
  }

  bool hasIsForEccTesting() { return mIsForEccTesting ? true : false; }
  bool getIsForEccTesting() { return *mIsForEccTesting; }
  void setIsForEccTesting(bool val) { mIsForEccTesting = val; }
  inline string dumpIsForEccTesting() {
     return std::string(" mIsForEccTesting = ") +
           (mIsForEccTesting ? (*mIsForEccTesting ? "true" : "false") : "<invalid>");
  }

  bool hasIsIntentionEcc() { return mIsIntentionEcc ? true : false; }
  bool getIsIntentionEcc() { return *mIsIntentionEcc; }
  void setIsIntentionEcc(bool val) { mIsIntentionEcc = val; }
  inline string dumpIsIntentionEcc() {
     return std::string(" mIsIntentionEcc = ") +
           (mIsIntentionEcc ? (*mIsIntentionEcc ? "true" : "false") : "<invalid>");
  }

  bool hasCategories() { return mCategories ? true : false; }
  unsigned int getCategories() { return *mCategories; }
  void setCategories(unsigned int val) { mCategories = val; }
  inline string dumpCategories() {
     return " mCategories = " + (mCategories ? std::to_string(*mCategories) : "<invalid>");
  }

  bool hasRouting() { return mCategories ? true : false; }
  qcril::interfaces::EmergencyCallRouting getRouting() { return *mRouting; }
  void setRouting(qcril::interfaces::EmergencyCallRouting val) { mRouting = val; }
  inline string dumpRouting() {
    return " mRouting= " + (mRouting ? toString(*mRouting) : "<invalid>");
  }

  bool hasRetryCallFailReason() { return mRetryCallFailReason ? true : false; }
  qcril::interfaces::CallFailCause getRetryCallFailReason() { return *mRetryCallFailReason; }
  void setRetryCallFailReason(qcril::interfaces::CallFailCause val) { mRetryCallFailReason = val; }
  inline string dumpRetryCallFailReason() {
    return " mRetryCallFailReason = " +
           (mRetryCallFailReason ? toString(*mRetryCallFailReason) : "<invalid>");
  }

  bool hasRetryCallFailMode() { return mRetryCallFailMode ? true : false; }
  RIL_RadioTechnology getRetryCallFailMode() { return *mRetryCallFailMode; }
  void setRetryCallFailMode(RIL_RadioTechnology val) { mRetryCallFailMode = val; }
  inline string dumpRetryCallFailMode() {
    return " mRetryCallFailMode = " +
           (mRetryCallFailMode ? std::to_string(*mRetryCallFailMode) : "<invalid>");
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() +
          "{" + dumpAddress() + dumpClir() /* + dumpUusInfo() */ + dumpCallType() +
          dumpCallDomain() + dumpIsConferenceUri() + dumpIsCallPull() + dumpIsEncrypted() +
          dumpDisplayText() + dumpRttMode() + dumpIsSecondary() + dumpOriginatingNumber() + 
          dumpIsEmergency() + dumpIsForEccTesting() + dumpIsIntentionEcc() + dumpCategories() +
          dumpRouting() + dumpRetryCallFailReason() + dumpRetryCallFailMode() + "}";
  }
};
