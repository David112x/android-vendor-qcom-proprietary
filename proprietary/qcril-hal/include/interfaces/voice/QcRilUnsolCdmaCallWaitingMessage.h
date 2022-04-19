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
class QcRilUnsolCdmaCallWaitingMessage : public UnSolicitedMessage,
                                         public add_message_id<QcRilUnsolCdmaCallWaitingMessage> {
 private:
  std::optional<std::string> mNumber;
  std::optional<int> mNumberPresentation;
  std::optional<std::string> mCwName;
  std::optional<std::shared_ptr<RIL_CDMA_SignalInfoRecord>> mSignalInfoRecord;
  std::optional<int> mNumberType;
  std::optional<int> mNumberPlan;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolCdmaCallWaitingMessage";
  ~QcRilUnsolCdmaCallWaitingMessage() {}

  QcRilUnsolCdmaCallWaitingMessage() : UnSolicitedMessage(get_class_message_id()) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolCdmaCallWaitingMessage> msg =
        std::make_shared<QcRilUnsolCdmaCallWaitingMessage>();
    return msg;
  }

  bool hasNumber() { return mNumber ? true : false; }
  std::string getNumber() { return *mNumber; }
  void setNumber(std::string val) { mNumber = val; }

  bool hasNumberPresentation() { return mNumberPresentation ? true : false; }
  int getNumberPresentation() { return *mNumberPresentation; }
  void setNumberPresentation(int val) { mNumberPresentation = val; }

  bool hasName() { return mCwName ? true : false; }
  std::string getName() { return *mCwName; }
  void setName(std::string val) { mCwName = val; }

  bool hasSignalInfoRecord() { return mSignalInfoRecord ? true : false; }
  std::shared_ptr<RIL_CDMA_SignalInfoRecord> getSignalInfoRecord() { return *mSignalInfoRecord; }
  void setSignalInfoRecord(std::shared_ptr<RIL_CDMA_SignalInfoRecord> val) {
    mSignalInfoRecord = val;
  }

  bool hasNumberType() { return mNumberType ? true : false; }
  int getNumberType() { return *mNumberType; }
  void setNumberType(int val) { mNumberType = val; }

  bool hasNumberPlan() { return mNumberPlan ? true : false; }
  int getNumberPlan() { return *mNumberPlan; }
  void setNumberPlan(int val) { mNumberPlan = val; }

  string dump() { return mName + "{" + (mIsIms ? "isIms=true" : "") + "}"; }

  // To distinguish IMS or CS indication
 private:
  bool mIsIms = false;

 public:
  bool isIms() { return mIsIms; }
  void setIsIms(bool val) { mIsIms = val; }
};
