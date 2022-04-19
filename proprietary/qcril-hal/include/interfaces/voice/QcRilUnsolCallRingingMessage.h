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
 * Unsol message to notify call ringing indication
 */
class QcRilUnsolCallRingingMessage : public UnSolicitedMessage,
                                     public add_message_id<QcRilUnsolCallRingingMessage> {
 private:
  std::optional<std::shared_ptr<RIL_CDMA_SignalInfoRecord>> mCdmaSignalInfoRecord;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolCallRingingMessage";

  ~QcRilUnsolCallRingingMessage() {}

  QcRilUnsolCallRingingMessage() : UnSolicitedMessage(get_class_message_id()) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolCallRingingMessage> msg =
        std::make_shared<QcRilUnsolCallRingingMessage>();
    return msg;
  }

  bool hasCdmaSignalInfoRecord() { return mCdmaSignalInfoRecord ? true : false; }
  std::shared_ptr<RIL_CDMA_SignalInfoRecord> getCdmaSignalInfoRecord() {
    return *mCdmaSignalInfoRecord;
  }
  void setCdmaSignalInfoRecord(std::shared_ptr<RIL_CDMA_SignalInfoRecord> val) {
    mCdmaSignalInfoRecord = val;
  }

  string dump() { return mName + "{isIms=" + (mIsIms ? "true" : "false") + "}"; }

  // To distinguish IMS or CS indication
 private:
  bool mIsIms = false;

 public:
  bool isIms() { return mIsIms; }
  void setIsIms(bool val) { mIsIms = val; }
};
