/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>
#include "telephony/ril.h"

/*
 * Unsol message to report radio capabilities
 */

class RilUnsolRadioCapabilityMessage : public UnSolicitedMessage,
            public add_message_id<RilUnsolRadioCapabilityMessage> {
 public:
  static constexpr const char *MESSAGE_NAME = "RIL_UNSOL_RADIO_CAPABILITY";
  ~RilUnsolRadioCapabilityMessage() { }

  explicit RilUnsolRadioCapabilityMessage(RIL_RadioCapability radioCap)
      : UnSolicitedMessage(get_class_message_id()), mRadioCap(radioCap) { }

  std::shared_ptr<UnSolicitedMessage> clone() {
    return std::make_shared<RilUnsolRadioCapabilityMessage>(mRadioCap);
  }

  RIL_RadioCapability& getRadioCapability() { return mRadioCap; }

  string dump() {
    return RilUnsolRadioCapabilityMessage::MESSAGE_NAME;
  }
 private:
  RIL_RadioCapability mRadioCap;
};
