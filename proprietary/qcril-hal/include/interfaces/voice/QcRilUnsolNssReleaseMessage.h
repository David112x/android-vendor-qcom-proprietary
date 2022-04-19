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

#include "voice_service_v02.h"

/*
 * Unsol message to National Supplementary Services - Release
 */
class QcRilUnsolNssReleaseMessage : public UnSolicitedMessage,
                                    public add_message_id<QcRilUnsolNssReleaseMessage> {
 private:
  uint8_t mCallId;
  voice_nss_release_enum_v02 mNssRelease;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolNssReleaseMessage";
  ~QcRilUnsolNssReleaseMessage() {}

  QcRilUnsolNssReleaseMessage(uint8_t callId, voice_nss_release_enum_v02 nssRelease)
      : UnSolicitedMessage(get_class_message_id()), mCallId(callId), mNssRelease(nssRelease) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolNssReleaseMessage> msg =
        std::make_shared<QcRilUnsolNssReleaseMessage>(mCallId, mNssRelease);
    return msg;
  }

  uint8_t getCallId() { return mCallId; }
  voice_nss_release_enum_v02 getNssRelease() { return mNssRelease; }

  std::string dump() {
    return mName + "{ mCallId = " + std::to_string(mCallId) +
           " mNssRelease = " + std::to_string(mNssRelease) + "}";
  }
};
