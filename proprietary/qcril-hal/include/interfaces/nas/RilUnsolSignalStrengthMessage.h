/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>
#include "telephony/ril.h"

/*
 * Unsol message to report signal strength
 */

class RilUnsolSignalStrengthMessage : public UnSolicitedMessage,
            public add_message_id<RilUnsolSignalStrengthMessage> {
 private:
  RIL_SignalStrength mSigInfo;

 public:
  static constexpr const char *MESSAGE_NAME = "RIL_UNSOL_SIGNAL_STRENGTH";
  ~RilUnsolSignalStrengthMessage() { }

  explicit RilUnsolSignalStrengthMessage(RIL_SignalStrength sigInfo)
      : UnSolicitedMessage(get_class_message_id()), mSigInfo(sigInfo) { }

  std::shared_ptr<UnSolicitedMessage> clone() {
    return std::make_shared<RilUnsolSignalStrengthMessage>(mSigInfo);
  }

  const RIL_SignalStrength& getSignalStrength() { return mSigInfo; }

  string dump() {
    return RilUnsolSignalStrengthMessage::MESSAGE_NAME;
  }
};
