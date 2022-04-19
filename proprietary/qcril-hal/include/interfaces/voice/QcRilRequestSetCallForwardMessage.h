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
 * Request to configure call forward rule
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : std::shared_ptr<qcril::interfaces::SetCallForwardRespData>
 */
class QcRilRequestSetCallForwardMessage : public QcRilRequestMessage,
                                          public add_message_id<QcRilRequestSetCallForwardMessage> {
 private:
  std::optional<uint32_t> mStatus;
  std::optional<uint32_t> mReason;
  std::optional<uint32_t> mServiceClass;
  std::optional<uint32_t> mToa;
  std::optional<std::string> mNumber;
  std::optional<uint32_t> mTimeSeconds;
  std::optional<std::shared_ptr<qcril::interfaces::CallFwdTimerInfo>> mCallFwdTimerStart;
  std::optional<std::shared_ptr<qcril::interfaces::CallFwdTimerInfo>> mCallFwdTimerEnd;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestSetCallForwardMessage";

  QcRilRequestSetCallForwardMessage() = delete;

  ~QcRilRequestSetCallForwardMessage() {}

  inline QcRilRequestSetCallForwardMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  bool hasStatus() { return mStatus ? true : false; }
  uint32_t getStatus() { return *mStatus; }
  void setStatus(uint32_t val) { mStatus = val; }
  inline string dumpStatus() {
    return " mStatus = " + (mStatus ? std::to_string(*mStatus) : "<invalid>");
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

  bool hasToa() { return mToa ? true : false; }
  uint32_t getToa() { return *mToa; }
  void setToa(uint32_t val) { mToa = val; }
  inline string dumpToa() { return " mToa = " + (mToa ? std::to_string(*mToa) : "<invalid>"); }

  bool hasNumber() { return mNumber ? true : false; }
  std::string getNumber() { return *mNumber; }
  void setNumber(std::string val) { mNumber = val; }
  inline string dumpNumber() { return " mNumber = " + (mNumber ? *mNumber : "<invalid>"); }

  bool hasTimeSeconds() { return mTimeSeconds ? true : false; }
  uint32_t getTimeSeconds() { return *mTimeSeconds; }
  void setTimeSeconds(uint32_t val) { mTimeSeconds = val; }
  inline string dumpTimeSeconds() {
    return " mTimeSeconds = " + (mTimeSeconds ? std::to_string(*mTimeSeconds) : "<invalid>");
  }

  bool hasCallFwdTimerStart() { return mCallFwdTimerStart ? true : false; }
  std::shared_ptr<qcril::interfaces::CallFwdTimerInfo> getCallFwdTimerStart() {
    return *mCallFwdTimerStart;
  }
  void setCallFwdTimerStart(std::shared_ptr<qcril::interfaces::CallFwdTimerInfo> val) {
    mCallFwdTimerStart = val;
  }
  inline string dumpCallFwdTimerStart() {
    return " mCallFwdTimerStart = " + (mCallFwdTimerStart && *mCallFwdTimerStart
                                          ? qcril::interfaces::toString(**mCallFwdTimerStart)
                                          : "<invalid>");
  }

  bool hasCallFwdTimerEnd() { return mCallFwdTimerEnd ? true : false; }
  std::shared_ptr<qcril::interfaces::CallFwdTimerInfo> getCallFwdTimerEnd() {
    return *mCallFwdTimerEnd;
  }
  void setCallFwdTimerEnd(std::shared_ptr<qcril::interfaces::CallFwdTimerInfo> val) {
    mCallFwdTimerEnd = val;
  }
  inline string dumpCallFwdTimerEnd() {
    return " mCallFwdTimerEnd = " + (mCallFwdTimerEnd && *mCallFwdTimerEnd
                                        ? qcril::interfaces::toString(**mCallFwdTimerEnd)
                                        : "<invalid>");
  }
  virtual string dump() {
    return QcRilRequestMessage::dump() +
          "{" + dumpStatus() + dumpReason() + dumpServiceClass() + dumpToa() + dumpNumber() +
                dumpTimeSeconds() + dumpCallFwdTimerStart() + dumpCallFwdTimerEnd() +
          "}";
  }
};
