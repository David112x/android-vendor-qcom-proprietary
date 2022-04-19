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
 * Request to hangup a call
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : nullptr
 */
class QcRilRequestHangupMessage : public QcRilRequestMessage {
 private:
  bool mRetryAttempted;

  std::optional<uint32_t> mCallIndex;
  std::optional<bool> mIsMultiParty;
  std::optional<std::string> mConnectionUri;
  std::optional<uint32_t> mConferenceId;
  std::optional<qcril::interfaces::CallFailCause> mRejectCause;
  std::optional<uint32_t> mRejectCauseRaw;

 public:
  QcRilRequestHangupMessage() = delete;

  ~QcRilRequestHangupMessage() {}

  inline QcRilRequestHangupMessage(message_id_t msg_id, std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(msg_id, context), mRetryAttempted(false) {}

  bool hasCallIndex() { return mCallIndex ? true : false; }
  uint32_t getCallIndex() { return *mCallIndex; }
  void setCallIndex(uint32_t val) { mCallIndex = val; }
  inline string dumpCallIndex() {
    return " mCallIndex = " + (mCallIndex ? std::to_string(*mCallIndex) : "<invalid>");
  }

  bool hasIsMultiParty() { return mIsMultiParty ? true : false; }
  bool getIsMultiParty() { return *mIsMultiParty; }
  void setIsMultiParty(bool val) { mIsMultiParty = val; }
  inline string dumpIsMultiParty() {
    return std::string(" mIsMultiParty = ") +
           (mIsMultiParty ? (*mIsMultiParty ? "true" : "false") : "<invalid>");
  }

  bool hasConnectionUri() { return mConnectionUri ? true : false; }
  std::string getConnectionUri() { return mConnectionUri ? *mConnectionUri : ""; }
  void setConnectionUri(std::string val) { mConnectionUri = val; }
  inline string dumpConnectionUri() {
    return " mConnectionUri = " + (mConnectionUri ? (*mConnectionUri) : "<invalid>");
  }

  bool hasConferenceId() { return mConferenceId ? true : false; }
  uint32_t getConferenceId() { return *mConferenceId; }
  void setConferenceId(uint32_t val) { mConferenceId = val; }
  inline string dumpConferenceId() {
    return " mConferenceId = " + (mConferenceId ? std::to_string(*mConferenceId) : "<invalid>");
  }

  bool hasRejectCause() { return mRejectCause ? true : false; }
  qcril::interfaces::CallFailCause getRejectCause() { return *mRejectCause; }
  void setRejectCause(qcril::interfaces::CallFailCause val) { mRejectCause = val; }
  inline string dumpRejectCause() {
    return " mRejectCause = " + (mRejectCause ? toString(*mRejectCause) : "<invalid>");
  }

  bool hasRejectCauseRaw() { return mRejectCauseRaw ? true : false; }
  uint32_t getRejectCauseRaw() { return *mRejectCauseRaw; }
  void setRejectCauseRaw(uint32_t val) { mRejectCauseRaw = val; }
  inline string dumpRejectCauseRaw() {
    return " mRejectCauseRaw = " + (mRejectCauseRaw ? std::to_string(*mRejectCauseRaw) : "<invalid>");
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() +
          "{" + dumpCallIndex() + dumpIsMultiParty() + dumpConnectionUri() + dumpConferenceId() +
                dumpRejectCause() + dumpRejectCauseRaw() +
          "}";
  }

  // For Internal use
  bool isHangupRetryAttempted() { return mRetryAttempted; }
  void setHangupRetryAttempted() { mRetryAttempted = true; }
};
