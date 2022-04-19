/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>

/*
 * Unsol message to notify voice rat change
 *
 */
class RilUnsolVoiceRadioTechChangedMessage : public UnSolicitedMessage,
            public add_message_id<RilUnsolVoiceRadioTechChangedMessage> {
 private:
  unsigned int mVoiceRat;

 public:
  static constexpr const char *MESSAGE_NAME = "RIL_UNSOL_VOICE_RADIO_TECH_CHANGED";
  ~RilUnsolVoiceRadioTechChangedMessage() { }

  RilUnsolVoiceRadioTechChangedMessage(unsigned int rat)
      : UnSolicitedMessage(get_class_message_id()), mVoiceRat(rat) {}

  std::shared_ptr<UnSolicitedMessage> clone() {
    return std::make_shared<RilUnsolVoiceRadioTechChangedMessage>(mVoiceRat);
  }

  int getVoiceRat() { return mVoiceRat; }

  string dump() {
    return RilUnsolVoiceRadioTechChangedMessage::MESSAGE_NAME;
  }
};
