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
 * Unsol message to notify supplementary service notifications on IMS
 */
class QcRilUnsolSuppSvcNotificationMessage : public UnSolicitedMessage,
                                       public add_message_id<QcRilUnsolSuppSvcNotificationMessage> {
 private:
  /** Notification Type */
  std::optional<qcril::interfaces::NotificationType> mNotificationType;
  /**
   * NotificationType::MO = MO intermediate result code
   * NotificationType::MT = MT unsolicited result code
   */

  /** Code : See 27.007 7.17*/
  std::optional<uint32_t> mCode;
  /**
   * code1
   *  0  unconditional call forwarding is active
   *  1  some of the conditional call forwardings are active
   *  2  call has been forwarded
   *  3  call is waiting
   *  4  this is a CUG call (also <index> present)
   *  5  outgoing calls are barred
   *  6  incoming calls are barred
   *  7  CLIR suppression rejected
   *  8  call has been deflected
   * code2
   *  0 this is a forwarded call (MT call setup)
   *  1 this is a CUG call (also <index> present) (MT call setup)
   *  2 call has been put on hold (during a voice call)
   *  3 call has been retrieved (during a voice call)
   *  4 multiparty call entered (during a voice call)
   *  5 call on hold has been released (this is not a SS notification) (during a voice call)
   *  6 forward check SS message received (can be received whenever)
   *  7 call is being connected (alerting) with the remote party in alerting state in explicit call
   *    transfer operation (during a voice call)
   *  8 call has been connected with the other remote party in explicit call transfer operation
   *    (also number and subaddress parameters may be present) (during a voice call or MT call
   *    setup)
   *  9 this is a deflected call (MT call setup)
   *  10  additional incoming call forwarded
   */

  /** CUG Index */
  std::optional<uint32_t> mIndex;
  /**
   * Index of the CUG call.
   */

  std::optional<uint32_t> mType;
  std::optional<std::string> mNumber;
  std::optional<uint32_t> mConnectionId;
  std::optional<std::string> mHistoryInfo;
  std::optional<bool> mHoldTone;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolSuppSvcNotificationMessage";
  ~QcRilUnsolSuppSvcNotificationMessage() {}

  QcRilUnsolSuppSvcNotificationMessage() : UnSolicitedMessage(get_class_message_id()) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolSuppSvcNotificationMessage> msg =
        std::make_shared<QcRilUnsolSuppSvcNotificationMessage>();
    if (msg) {
      if (mNotificationType) {
        msg->setNotificationType(*mNotificationType);
      }
      if (mCode) {
        msg->setCode(*mCode);
      }
      if (mIndex) {
        msg->setIndex(*mIndex);
      }
      if (mType) {
        msg->setType(*mType);
      }
      if (mNumber) {
        msg->setNumber(*mNumber);
      }
      if (mConnectionId) {
        msg->setConnectionId(*mConnectionId);
      }
      if (mHistoryInfo) {
        msg->setHistoryInfo(*mHistoryInfo);
      }
      if (mHoldTone) {
        msg->setHoldTone(*mHoldTone);
      }
    }
    return msg;
  }

  bool hasNotificationType() { return mNotificationType ? true : false; }
  qcril::interfaces::NotificationType getNotificationType() { return *mNotificationType; }
  void setNotificationType(qcril::interfaces::NotificationType val) { mNotificationType = val; }
  inline std::string dumpNotificationType() {
    return " mNotificationType = " +
           (mNotificationType ? toString(*mNotificationType) : "<invalid>");
  }

  bool hasCode() { return mCode ? true : false; }
  uint32_t getCode() { return *mCode; }
  void setCode(uint32_t val) { mCode = val; }
  inline std::string dumpCode() {
    return " mCode = " + (mCode ? std::to_string(*mCode) : "<invalid>");
  }

  bool hasIndex() { return mIndex ? true : false; }
  uint32_t getIndex() { return *mIndex; }
  void setIndex(uint32_t val) { mIndex = val; }
  inline std::string dumpIndex() {
    return " mIndex = " + (mIndex ? std::to_string(*mIndex) : "<invalid>");
  }

  bool hasType() { return mType ? true : false; }
  uint32_t getType() { return *mType; }
  void setType(uint32_t val) { mType = val; }
  inline std::string dumpType() {
    return " mType = " + (mType ? std::to_string(*mType) : "<invalid>");
  }

  bool hasNumber() { return mNumber ? true : false; }
  std::string getNumber() { return *mNumber; }
  void setNumber(std::string val) { mNumber = val; }
  inline std::string dumpNumber() { return " mNumber = " + (mNumber ? (*mNumber) : "<invalid>"); }

  bool hasConnectionId() { return mConnectionId ? true : false; }
  uint32_t getConnectionId() { return *mConnectionId; }
  void setConnectionId(uint32_t val) { mConnectionId = val; }
  inline std::string dumpConnectionId() {
    return " mConnectionId = " + (mConnectionId ? std::to_string(*mConnectionId) : "<invalid>");
  }

  bool hasHistoryInfo() { return mHistoryInfo ? true : false; }
  std::string getHistoryInfo() { return *mHistoryInfo; }
  void setHistoryInfo(std::string val) { mHistoryInfo = val; }
  inline std::string dumpHistoryInfo() {
    return " mHistoryInfo = " + (mHistoryInfo ? (*mHistoryInfo) : "<invalid>");
  }

  bool hasHoldTone() { return mHoldTone ? true : false; }
  bool getHoldTone() { return *mHoldTone; }
  void setHoldTone(bool val) { mHoldTone = val; }
  inline std::string dumpHoldTone() {
    return std::string(" mHoldTone = ") +
           (mHoldTone ? (*mHoldTone ? "true" : "false") : "<invalid>");
  }

  std::string dump() {
    return mName + "{isIms=" + (mIsIms ? "true" : "false") + dumpNotificationType() + dumpCode() +
           dumpIndex() + dumpType() + dumpNumber() + dumpConnectionId() + dumpHistoryInfo() +
           dumpHoldTone() + "}";
  }

  // To distinguish IMS or CS indication
 private:
  bool mIsIms = false;
 public:
  bool isIms() { return mIsIms; }
  void setIsIms(bool val) { mIsIms = val; }
};
