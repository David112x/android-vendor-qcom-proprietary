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

/*
 * Unsol message to notify SRVCC or RAT handover changes
 */
class QcRilUnsolImsHandoverMessage : public UnSolicitedMessage,
                                       public add_message_id<QcRilUnsolImsHandoverMessage> {
 private:
  std::optional<qcril::interfaces::HandoverType> mHandoverType;
  std::optional<RIL_RadioTechnology> mSourceTech;
  std::optional<RIL_RadioTechnology> mTargetTech;
  std::optional<std::string> mCauseCode;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolImsHandoverMessage";
  inline static const std::string WLAN_HANDOVER_NO_LTE_FAILURE_CODE_STRING = "CD-04";
  ~QcRilUnsolImsHandoverMessage() {}

  QcRilUnsolImsHandoverMessage() : UnSolicitedMessage(get_class_message_id()) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolImsHandoverMessage> msg =
        std::make_shared<QcRilUnsolImsHandoverMessage>();
    if (msg) {
      if (mHandoverType) {
        msg->setHandoverType(*mHandoverType);
      }
      if (mSourceTech) {
        msg->setSourceTech(*mSourceTech);
      }
      if (mTargetTech) {
        msg->setTargetTech(*mTargetTech);
      }
      if (mCauseCode) {
        msg->setCauseCode(*mCauseCode);
      }
    }
    return msg;
  }

  bool hasHandoverType() { return mHandoverType ? true : false; }
  qcril::interfaces::HandoverType getHandoverType() { return *mHandoverType; }
  void setHandoverType(qcril::interfaces::HandoverType val) { mHandoverType = val; }
  inline std::string dumpHandoverType() {
    return " mHandoverType = " + (mHandoverType ? toString(*mHandoverType) : "<invalid>");
  }

  bool hasSourceTech() { return mSourceTech ? true : false; }
  RIL_RadioTechnology getSourceTech() { return *mSourceTech; }
  void setSourceTech(RIL_RadioTechnology val) { mSourceTech = val; }
  inline std::string dumpSourceTech() {
    return " mSourceTech = " + (mSourceTech ? std::to_string(*mSourceTech) : "<invalid>");
  }

  bool hasTargetTech() { return mTargetTech ? true : false; }
  RIL_RadioTechnology getTargetTech() { return *mTargetTech; }
  void setTargetTech(RIL_RadioTechnology val) { mTargetTech = val; }
  inline std::string dumpTargetTech() {
    return " mTargetTech = " + (mTargetTech ? std::to_string(*mTargetTech) : "<invalid>");
  }

  bool hasCauseCode() { return mCauseCode ? true : false; }
  std::string getCauseCode() { return *mCauseCode; }
  void setCauseCode(std::string val) { mCauseCode = val; }
  inline std::string dumpCauseCode() {
    return " mCauseCode = " + (mCauseCode ? *mCauseCode : "<invalid>");
  }

  string dump() {
    return mName + "{" + dumpHandoverType() + dumpSourceTech() + dumpTargetTech() +
           dumpCauseCode() + "}";
  }
};
