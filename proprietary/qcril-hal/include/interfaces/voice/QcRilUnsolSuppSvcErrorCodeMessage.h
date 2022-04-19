/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>
#include <interfaces/voice/voice.h>
#include <optional>

/*
 * Unsol message to Supplementary service error code
 */
class QcRilUnsolSuppSvcErrorCodeMessage : public UnSolicitedMessage,
                                          public add_message_id<QcRilUnsolSuppSvcErrorCodeMessage> {
 private:
  uint8_t mCallId;
  qmi_sups_errors_enum_v02 mFailureCause;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolSuppSvcErrorCodeMessage";
  ~QcRilUnsolSuppSvcErrorCodeMessage() {}

  QcRilUnsolSuppSvcErrorCodeMessage(uint8_t callId, qmi_sups_errors_enum_v02 failureCause)
      : UnSolicitedMessage(get_class_message_id()), mCallId(callId), mFailureCause(failureCause) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolSuppSvcErrorCodeMessage> msg =
        std::make_shared<QcRilUnsolSuppSvcErrorCodeMessage>(mCallId, mFailureCause);
    return msg;
  }

  uint8_t getCallId() { return mCallId; }
  void setCallId(uint8_t val) { mCallId = val; }
  inline std::string dumpCallId() {
    return " mCallId = " + std::to_string(mCallId);
  }

  qmi_sups_errors_enum_v02 getFailureCause() { return mFailureCause; }
  void setFailureCause(qmi_sups_errors_enum_v02 val) { mFailureCause = val; }
  inline std::string dumpFailureCause() {
    return " mFailureCause = " + std::to_string(mFailureCause);
  }

  std::string dump() { return mName + "{" + dumpCallId() + dumpFailureCause() + "}"; }
};
