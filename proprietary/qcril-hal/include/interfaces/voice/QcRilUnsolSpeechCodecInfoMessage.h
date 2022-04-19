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
 * Unsol message to speech codec info
 */
class QcRilUnsolSpeechCodecInfoMessage : public UnSolicitedMessage,
                                         public add_message_id<QcRilUnsolSpeechCodecInfoMessage> {
 private:
  uint8_t mCallId;
  voice_speech_codec_enum_v02 mSpeechCodec;
  voice_network_mode_enum_v02 mNetworkMode;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolSpeechCodecInfoMessage";
  ~QcRilUnsolSpeechCodecInfoMessage() {}

  QcRilUnsolSpeechCodecInfoMessage(uint8_t callId, voice_speech_codec_enum_v02 speechCodec,
                                   voice_network_mode_enum_v02 networkMode)
      : UnSolicitedMessage(get_class_message_id()),
        mCallId(callId),
        mSpeechCodec(speechCodec),
        mNetworkMode(networkMode) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolSpeechCodecInfoMessage> msg =
        std::make_shared<QcRilUnsolSpeechCodecInfoMessage>(mCallId, mSpeechCodec, mNetworkMode);
    return msg;
  }

  uint8_t getCallId() { return mCallId; }
  voice_speech_codec_enum_v02 getSpeechCodec() { return mSpeechCodec; }
  voice_network_mode_enum_v02 getNetworkMode() { return mNetworkMode; }

  std::string dump() {
    return mName + "{ mCallId = " + std::to_string(mCallId) +
           " mSpeechCodec = " + std::to_string(mSpeechCodec) +
           " mNetworkMode = " + std::to_string(mNetworkMode) + "}";
  }
};
