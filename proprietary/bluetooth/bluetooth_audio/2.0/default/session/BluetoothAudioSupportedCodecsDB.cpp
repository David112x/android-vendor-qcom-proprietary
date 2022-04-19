/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 */

 /*
 * Copyright 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "BTAudioProviderSessionCodecsDB"

#include "BluetoothAudioSupportedCodecsDB.h"

#include <android-base/logging.h>

namespace vendor {
namespace qti {
namespace bluetooth_audio {

using ::vendor::qti::hardware::bluetooth_audio::V2_0::AacObjectType;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::AacParameters;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::AacVariableBitRate;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::AptxParameters;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::BitsPerSample;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::ChannelMode;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::CodecType;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::LdacChannelMode;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::LdacParameters;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::LdacQualityIndex;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::SampleRate;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::SbcAllocMethod;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::SbcBlockLength;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::SbcChannelMode;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::SbcNumSubbands;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::SbcParameters;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::AptxAdaptiveParameters;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::AptxTwsParameters;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::BaCeltParameters;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::AptxAdaptiveChannelMode;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::AptxMode;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::AptxSinkBuffering;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::AptxAdaptive_TTP;

// Default Supported PCM Parameters
static const PcmParameters kDefaultSoftwarePcmCapabilities = {
    .sampleRate = static_cast<SampleRate>(
        SampleRate::RATE_44100 | SampleRate::RATE_48000 |
        SampleRate::RATE_88200 | SampleRate::RATE_96000 |
        SampleRate::RATE_16000 | SampleRate::RATE_24000),
    .channelMode =
        static_cast<ChannelMode>(ChannelMode::MONO | ChannelMode::STEREO),
    .bitsPerSample = static_cast<BitsPerSample>(BitsPerSample::BITS_16 |
                                                BitsPerSample::BITS_24 |
                                                BitsPerSample::BITS_32)};

// Default Supported Codecs
// SBC: mSampleRate:(44100), mBitsPerSample:(16), mChannelMode:(MONO|STEREO)
//      all blocks | subbands 8 | Loudness
static const SbcParameters kDefaultOffloadSbcCapability = {
    .sampleRate = static_cast<SampleRate> (
        SampleRate::RATE_44100 | SampleRate::RATE_48000),
    .channelMode = static_cast<SbcChannelMode>(SbcChannelMode::MONO |
                                               SbcChannelMode::JOINT_STEREO),
    .blockLength = static_cast<SbcBlockLength>(
        SbcBlockLength::BLOCKS_4 | SbcBlockLength::BLOCKS_8 |
        SbcBlockLength::BLOCKS_12 | SbcBlockLength::BLOCKS_16),
    .numSubbands = SbcNumSubbands::SUBBAND_8,
    .allocMethod = SbcAllocMethod::ALLOC_MD_L,
    .bitsPerSample = BitsPerSample::BITS_16,
    .minBitpool = 2,
    .maxBitpool = 250};

// AAC: mSampleRate:(44100), mBitsPerSample:(16), mChannelMode:(STEREO)
static const AacParameters kDefaultOffloadAacCapability = {
    .objectType = AacObjectType::MPEG2_LC,
    .sampleRate = static_cast<SampleRate> (
        SampleRate::RATE_44100 | SampleRate::RATE_48000),
    .channelMode = ChannelMode::STEREO,
    .variableBitRateEnabled = AacVariableBitRate::DISABLED,
    .bitsPerSample = BitsPerSample::BITS_16};

// LDAC: mSampleRate:(44100|48000|88200|96000), mBitsPerSample:(16|24|32),
//       mChannelMode:(DUAL|STEREO)
static const LdacParameters kDefaultOffloadLdacCapability = {
    .sampleRate = static_cast<SampleRate>(
        SampleRate::RATE_44100 | SampleRate::RATE_48000 |
        SampleRate::RATE_88200 | SampleRate::RATE_96000),
    .channelMode = static_cast<LdacChannelMode>(LdacChannelMode::DUAL |
                                                LdacChannelMode::MONO |
                                                LdacChannelMode::STEREO),
    .qualityIndex = LdacQualityIndex::QUALITY_HIGH,
    .bitsPerSample = static_cast<BitsPerSample>(BitsPerSample::BITS_16 |
                                                BitsPerSample::BITS_24 |
                                                BitsPerSample::BITS_32)};

// aptX: mSampleRate:(44100|48000), mBitsPerSample:(16), mChannelMode:(STEREO)
static const AptxParameters kDefaultOffloadAptxCapability = {
    .sampleRate = static_cast<SampleRate>(SampleRate::RATE_44100 |
                                          SampleRate::RATE_48000),
    .bitsPerSample = BitsPerSample::BITS_16,
    .channelMode = ChannelMode::STEREO};

// aptX HD: mSampleRate:(44100|48000), mBitsPerSample:(24),
//          mChannelMode:(STEREO)
static const AptxParameters kDefaultOffloadAptxHdCapability = {
    .sampleRate = static_cast<SampleRate>(SampleRate::RATE_44100 |
                                          SampleRate::RATE_48000),
    .bitsPerSample = BitsPerSample::BITS_24,
    .channelMode = ChannelMode::STEREO};

// aptX Adaptive Default Sink Buffering and TTL
static const AptxSinkBuffering kDefaultAptxAdaptiveSinkBuffering = {
    .minSinkBuff_LL = 20,
    .maxSinkBuff_LL = 50,
    .minSinkBuff_HQ = 20,
    .maxSinkBuff_HQ = 50,
    .minSinkBuff_TWS = 20,
    .maxSinkBuff_TWS = 50
};

static const AptxAdaptive_TTP kDefaultAptxAdaptive_TTP = {
    .TTP_LL_low = 69,
    .TTP_LL_high = 69,
    .TTP_HQ_low = 200,
    .TTP_HQ_high = 200,
    .TTP_TWS_low = 200,
    .TTP_TWS_high = 200
};

// aptX: mSampleRate:(44100|48000), mBitsPerSample:(16), mChannelMode:(STEREO)
static const AptxTwsParameters kDefaultOffloadAptxTwsCapability = {
    .sampleRate = static_cast<SampleRate>(SampleRate::RATE_44100 |
                                          SampleRate::RATE_48000),
    .channelMode = ChannelMode::MONO,
    .syncMode = 0x02,
};

// aptX: mSampleRate:(44100|48000), mBitsPerSample:(16), mChannelMode:(STEREO)
static const BaCeltParameters kDefaultOffloadBaCeltCapability = {
    .sampleRate = static_cast<SampleRate>(SampleRate::RATE_44100 |
                                          SampleRate::RATE_48000),
    .channelMode = ChannelMode::STEREO,
    .frameSize = 512,
    .complexity = 1,
    .predictionMode = 0,
    .vbrFlag = 0,
};

// aptX Adaptive: mSampleRate:(44100|48000), mBitsPerSample:(24),
//          mChannelMode:(JOINT_STEREO)
static const AptxAdaptiveParameters kDefaultOffloadAptxAdaptiveCapability = {
    .sampleRate = static_cast<SampleRate>(SampleRate::RATE_44100 |
                                          SampleRate::RATE_48000 |
                                          SampleRate::RATE_96000),
    .channelMode = static_cast<AptxAdaptiveChannelMode>(AptxAdaptiveChannelMode::JOINT_STEREO),
    .bitsPerSample = BitsPerSample::BITS_24,
    .aptxMode = AptxMode::HQ,
    .sinkBuffering = kDefaultAptxAdaptiveSinkBuffering,
    .ttp = kDefaultAptxAdaptive_TTP
};

const std::vector<CodecCapabilities> kDefaultOffloadA2dpCodecCapabilities = {
    {.codecType = CodecType::SBC, .capabilities = {}},
    {.codecType = CodecType::AAC, .capabilities = {}},
    {.codecType = CodecType::LDAC, .capabilities = {}},
    {.codecType = CodecType::APTX, .capabilities = {}},
    {.codecType = CodecType::APTX_HD, .capabilities = {}},
    {.codecType = CodecType::APTX_ADAPTIVE, .capabilities = {}}
};

static bool IsSingleBit(uint32_t bitmasks, uint32_t bitfield) {
  bool single = false;
  uint32_t test_bit = 0x00000001;
  while (test_bit <= bitmasks && test_bit <= bitfield) {
    if (bitfield & test_bit && bitmasks & test_bit) {
      if (single) return false;
      single = true;
    }
    if (test_bit == 0x80000000) break;
    test_bit <<= 1;
  }
  return single;
}

static bool IsOffloadSbcConfigurationValid(
    const CodecConfiguration::CodecSpecific& codec_specific);
static bool IsOffloadAacConfigurationValid(
    const CodecConfiguration::CodecSpecific& codec_specific);
static bool IsOffloadLdacConfigurationValid(
    const CodecConfiguration::CodecSpecific& codec_specific);
static bool IsOffloadAptxConfigurationValid(
    const CodecConfiguration::CodecSpecific& codec_specific);
static bool IsOffloadAptxHdConfigurationValid(
    const CodecConfiguration::CodecSpecific& codec_specific);
static bool IsOffloadAptxAdaptiveConfigurationValid(
    const CodecConfiguration::CodecSpecific& codec_specific);
static bool IsOffloadAptxTwsConfigurationValid(
    const CodecConfiguration::CodecSpecific& codec_specific);
static bool IsOffloadBaCeltConfigurationValid(
    const CodecConfiguration::CodecSpecific& codec_specific);

static bool IsOffloadSbcConfigurationValid(
    const CodecConfiguration::CodecSpecific& codec_specific) {
  const SbcParameters sbc_data = codec_specific.sbcConfig;
  if (!IsSingleBit(static_cast<uint32_t>(sbc_data.sampleRate), 0xff) ||
      !IsSingleBit(static_cast<uint32_t>(sbc_data.channelMode), 0x0f) ||
      !IsSingleBit(static_cast<uint32_t>(sbc_data.blockLength), 0xf0) ||
      !IsSingleBit(static_cast<uint32_t>(sbc_data.numSubbands), 0x0c) ||
      !IsSingleBit(static_cast<uint32_t>(sbc_data.allocMethod), 0x03) ||
      !IsSingleBit(static_cast<uint32_t>(sbc_data.bitsPerSample), 0x07) ||
      sbc_data.minBitpool > sbc_data.maxBitpool) {
    LOG(WARNING) << __func__
                 << ": Invalid CodecSpecific=" << toString(codec_specific);
    return false;
  } else if ((sbc_data.sampleRate & kDefaultOffloadSbcCapability.sampleRate) &&
             (sbc_data.channelMode &
              kDefaultOffloadSbcCapability.channelMode) &&
             (sbc_data.blockLength &
              kDefaultOffloadSbcCapability.blockLength) &&
             (sbc_data.numSubbands &
              kDefaultOffloadSbcCapability.numSubbands) &&
             (sbc_data.allocMethod &
              kDefaultOffloadSbcCapability.allocMethod) &&
             (sbc_data.bitsPerSample &
              kDefaultOffloadSbcCapability.bitsPerSample) &&
             (kDefaultOffloadSbcCapability.minBitpool <= sbc_data.minBitpool &&
              sbc_data.maxBitpool <= kDefaultOffloadSbcCapability.maxBitpool)) {
    return true;
  }
  LOG(WARNING) << __func__
               << ": Unsupported CodecSpecific=" << toString(codec_specific);
  return false;
}

static bool IsOffloadAacConfigurationValid(
    const CodecConfiguration::CodecSpecific& codec_specific) {
  const AacParameters aac_data = codec_specific.aacConfig;
  if (!IsSingleBit(static_cast<uint32_t>(aac_data.objectType), 0xf0) ||
      !IsSingleBit(static_cast<uint32_t>(aac_data.sampleRate), 0xff) ||
      !IsSingleBit(static_cast<uint32_t>(aac_data.channelMode), 0x03) ||
      !IsSingleBit(static_cast<uint32_t>(aac_data.bitsPerSample), 0x07)) {
    LOG(WARNING) << __func__
                 << ": Invalid CodecSpecific=" << toString(codec_specific);
    return false;
  } else if ((aac_data.objectType & kDefaultOffloadAacCapability.objectType) &&
             (aac_data.sampleRate & kDefaultOffloadAacCapability.sampleRate) &&
             (aac_data.channelMode &
              kDefaultOffloadAacCapability.channelMode) &&
             (aac_data.variableBitRateEnabled == AacVariableBitRate::DISABLED ||
              kDefaultOffloadAacCapability.variableBitRateEnabled ==
                  AacVariableBitRate::ENABLED) &&
             (aac_data.bitsPerSample &
              kDefaultOffloadAacCapability.bitsPerSample)) {
    return true;
  }
  LOG(WARNING) << __func__
               << ": Unsupported CodecSpecific=" << toString(codec_specific);
  return false;
}

static bool IsOffloadLdacConfigurationValid(
    const CodecConfiguration::CodecSpecific& codec_specific) {
  const LdacParameters ldac_data = codec_specific.ldacConfig;
  if (!IsSingleBit(static_cast<uint32_t>(ldac_data.sampleRate), 0xff) ||
      !IsSingleBit(static_cast<uint32_t>(ldac_data.channelMode), 0x07) ||
      (ldac_data.qualityIndex > LdacQualityIndex::QUALITY_LOW &&
       ldac_data.qualityIndex != LdacQualityIndex::QUALITY_ABR) ||
      !IsSingleBit(static_cast<uint32_t>(ldac_data.bitsPerSample), 0x07)) {
    LOG(WARNING) << __func__
                 << ": Invalid CodecSpecific=" << toString(codec_specific);
    return false;
  } else if ((ldac_data.sampleRate &
              kDefaultOffloadLdacCapability.sampleRate) &&
             (ldac_data.channelMode &
              kDefaultOffloadLdacCapability.channelMode) &&
             (ldac_data.bitsPerSample &
              kDefaultOffloadLdacCapability.bitsPerSample)) {
    return true;
  }
  LOG(WARNING) << __func__
               << ": Unsupported CodecSpecific=" << toString(codec_specific);
  return false;
}

static bool IsOffloadAptxConfigurationValid(
    const CodecConfiguration::CodecSpecific& codec_specific) {
  const AptxParameters aptx_data = codec_specific.aptxConfig;
  if (!IsSingleBit(static_cast<uint32_t>(aptx_data.sampleRate), 0xff) ||
      !IsSingleBit(static_cast<uint32_t>(aptx_data.channelMode), 0x03) ||
      !IsSingleBit(static_cast<uint32_t>(aptx_data.bitsPerSample), 0x07)) {
    LOG(WARNING) << __func__
                 << ": Invalid CodecSpecific=" << toString(codec_specific);
    return false;
  } else if ((aptx_data.sampleRate &
              kDefaultOffloadAptxCapability.sampleRate) &&
             (aptx_data.channelMode &
              kDefaultOffloadAptxCapability.channelMode) &&
             (aptx_data.bitsPerSample &
              kDefaultOffloadAptxCapability.bitsPerSample)) {
    return true;
  }
  LOG(WARNING) << __func__
               << ": Unsupported CodecSpecific=" << toString(codec_specific);
  return false;
}

static bool IsOffloadAptxHdConfigurationValid(
    const CodecConfiguration::CodecSpecific& codec_specific) {
  const AptxParameters aptx_data = codec_specific.aptxConfig;
  if (!IsSingleBit(static_cast<uint32_t>(aptx_data.sampleRate), 0xff) ||
      !IsSingleBit(static_cast<uint32_t>(aptx_data.channelMode), 0x03) ||
      !IsSingleBit(static_cast<uint32_t>(aptx_data.bitsPerSample), 0x07)) {
    LOG(WARNING) << __func__
                 << ": Invalid CodecSpecific=" << toString(codec_specific);
    return false;
  } else if ((aptx_data.sampleRate &
              kDefaultOffloadAptxHdCapability.sampleRate) &&
             (aptx_data.channelMode &
              kDefaultOffloadAptxHdCapability.channelMode) &&
             (aptx_data.bitsPerSample &
              kDefaultOffloadAptxHdCapability.bitsPerSample)) {
    return true;
  }
  LOG(WARNING) << __func__
               << ": Unsupported CodecSpecific=" << toString(codec_specific);
  return false;
}

static bool IsOffloadAptxAdaptiveConfigurationValid(
    const CodecConfiguration::CodecSpecific& codec_specific) {
  const AptxAdaptiveParameters aptxAdaptive_data = codec_specific.aptxAdaptiveConfig;
  LOG(WARNING) << __func__
               << ": Sampling rate:" << static_cast<uint32_t>(aptxAdaptive_data.sampleRate);

  LOG(WARNING) << __func__
               << ": sample rate is isvalid:" << IsSingleBit(static_cast<uint32_t>(aptxAdaptive_data.sampleRate), 0xff);
  LOG(WARNING) << __func__
               << ": channel mode:" << static_cast<uint32_t>(aptxAdaptive_data.channelMode);
  LOG(WARNING) << __func__
               << ": channel mode is isvalid:" << IsSingleBit(static_cast<uint32_t>(aptxAdaptive_data.channelMode), 0x03);
  LOG(WARNING) << __func__
               << ": bits per sample:" << static_cast<uint32_t>(aptxAdaptive_data.bitsPerSample);
  LOG(WARNING) << __func__
               << ": bits per sample is isvalid:" << IsSingleBit(static_cast<uint32_t>(aptxAdaptive_data.bitsPerSample), 0x07);
  LOG(WARNING) << __func__
               << ": Default Sampling rate:" << static_cast<uint32_t>(kDefaultOffloadAptxAdaptiveCapability.sampleRate);
  LOG(WARNING) << __func__
               << ": Default channel mode:" << static_cast<uint32_t>(kDefaultOffloadAptxAdaptiveCapability.channelMode);
  LOG(WARNING) << __func__
               << ": Default bitspersample :" << static_cast<uint32_t>(kDefaultOffloadAptxAdaptiveCapability.bitsPerSample);

  if (!IsSingleBit(static_cast<uint32_t>(aptxAdaptive_data.sampleRate), 0xff) ||
      /*TODO bitfield has to be changed for channel mode*/
      !IsSingleBit(static_cast<uint32_t>(aptxAdaptive_data.channelMode), 0xff) ||
      !IsSingleBit(static_cast<uint32_t>(aptxAdaptive_data.bitsPerSample), 0x07)) {
    LOG(WARNING) << __func__
                 << ": Invalid CodecSpecific2=" << toString(codec_specific);
    return false;
  } else if ((aptxAdaptive_data.sampleRate &
              kDefaultOffloadAptxAdaptiveCapability.sampleRate) &&
             (aptxAdaptive_data.bitsPerSample &
              kDefaultOffloadAptxAdaptiveCapability.bitsPerSample)) {
    return true;
  }
  LOG(WARNING) << __func__
               << ": Unsupported CodecSpecific=" << toString(codec_specific);
  return false;
}

static bool IsOffloadAptxTwsConfigurationValid(
    const CodecConfiguration::CodecSpecific& codec_specific) {
  const AptxTwsParameters aptxTws_data = codec_specific.aptxTwsConfig;
  if (!IsSingleBit(static_cast<uint32_t>(aptxTws_data.sampleRate), 0xff) ||
      !IsSingleBit(static_cast<uint32_t>(aptxTws_data.channelMode), 0x03)) {
    LOG(WARNING) << __func__
                 << ": Invalid CodecSpecific=" << toString(codec_specific);
    return false;
  } else if ((aptxTws_data.sampleRate &
              kDefaultOffloadAptxTwsCapability.sampleRate) &&
             (aptxTws_data.channelMode &
              kDefaultOffloadAptxTwsCapability.channelMode)) {
    return true;
  }
  LOG(WARNING) << __func__
               << ": Unsupported CodecSpecific=" << toString(codec_specific);
  return false;
}

static bool IsOffloadBaCeltConfigurationValid(
    const CodecConfiguration::CodecSpecific& codec_specific) {
  const BaCeltParameters baCelt_data = codec_specific.baCeltConfig;
  if (!IsSingleBit(static_cast<uint32_t>(baCelt_data.sampleRate), 0xff) ||
      !IsSingleBit(static_cast<uint32_t>(baCelt_data.channelMode), 0x03)) {
    LOG(WARNING) << __func__
                 << ": Invalid CodecSpecific=" << toString(codec_specific);
    return false;
  } else if ((baCelt_data.sampleRate &
              kDefaultOffloadBaCeltCapability.sampleRate) &&
             (baCelt_data.channelMode &
              kDefaultOffloadBaCeltCapability.channelMode)) {
    return true;
  }
  LOG(WARNING) << __func__
               << ": Unsupported CodecSpecific=" << toString(codec_specific);
  return false;
}

std::vector<PcmParameters> GetSoftwarePcmCapabilities() {
  return std::vector<PcmParameters>(1, kDefaultSoftwarePcmCapabilities);
}

std::vector<CodecCapabilities> GetOffloadCodecCapabilities(
    const SessionType& session_type) {
  if (session_type != SessionType::A2DP_HARDWARE_OFFLOAD_DATAPATH) {
    return std::vector<CodecCapabilities>(0);
  }
  std::vector<CodecCapabilities> offload_a2dp_codec_capabilities =
      kDefaultOffloadA2dpCodecCapabilities;
  for (auto& codec_capability : offload_a2dp_codec_capabilities) {
    switch (codec_capability.codecType) {
      case CodecType::SBC:
        codec_capability.capabilities.sbcCapabilities = kDefaultOffloadSbcCapability;
        break;
      case CodecType::AAC:
        codec_capability.capabilities.aacCapabilities = kDefaultOffloadAacCapability;
        break;
      case CodecType::LDAC:
        codec_capability.capabilities.ldacCapabilities = kDefaultOffloadLdacCapability;
        break;
      case CodecType::APTX:
        codec_capability.capabilities.aptxCapabilities = kDefaultOffloadAptxCapability;
        break;
      case CodecType::APTX_HD:
        codec_capability.capabilities.aptxCapabilities = kDefaultOffloadAptxHdCapability;
        break;
      case CodecType::APTX_ADAPTIVE:
        codec_capability.capabilities.aptxAdaptiveCapabilities = kDefaultOffloadAptxAdaptiveCapability;
        break;
      case CodecType::APTX_TWS:
        codec_capability.capabilities.aptxTwsCapabilities = kDefaultOffloadAptxTwsCapability;
        break;
      case CodecType::BA_CELT:
        codec_capability.capabilities.baCeltCapabilities = kDefaultOffloadBaCeltCapability;
        break;
      case CodecType::UNKNOWN:
        codec_capability = {};
        break;
    }
  }
  return offload_a2dp_codec_capabilities;
}

bool IsSoftwarePcmConfigurationValid(const PcmParameters& pcm_config) {
  if ((pcm_config.sampleRate != SampleRate::RATE_44100 &&
       pcm_config.sampleRate != SampleRate::RATE_48000 &&
       pcm_config.sampleRate != SampleRate::RATE_88200 &&
       pcm_config.sampleRate != SampleRate::RATE_96000 &&
       pcm_config.sampleRate != SampleRate::RATE_16000 &&
       pcm_config.sampleRate != SampleRate::RATE_24000) ||
      (pcm_config.bitsPerSample != BitsPerSample::BITS_16 &&
       pcm_config.bitsPerSample != BitsPerSample::BITS_24 &&
       pcm_config.bitsPerSample != BitsPerSample::BITS_32) ||
      (pcm_config.channelMode != ChannelMode::MONO &&
       pcm_config.channelMode != ChannelMode::STEREO)) {
    LOG(WARNING) << __func__
                 << ": Invalid PCM Configuration=" << toString(pcm_config);
    return false;
  } else if (pcm_config.sampleRate &
                 kDefaultSoftwarePcmCapabilities.sampleRate &&
             pcm_config.bitsPerSample &
                 kDefaultSoftwarePcmCapabilities.bitsPerSample &&
             pcm_config.channelMode &
                 kDefaultSoftwarePcmCapabilities.channelMode) {
    return true;
  }
  LOG(WARNING) << __func__
               << ": Unsupported PCM Configuration=" << toString(pcm_config);
  return false;
}

bool IsOffloadCodecConfigurationValid(const SessionType& session_type,
                                      const CodecConfiguration& codec_config) {
  if (session_type != SessionType::A2DP_HARDWARE_OFFLOAD_DATAPATH) {
    LOG(ERROR) << __func__
               << ": Invalid SessionType=" << toString(session_type);
    return false;
  } else if (0x00ffffff < codec_config.encodedAudioBitrate) {
    LOG(ERROR) << __func__ << ": Unsupported Codec Configuration="
               << toString(codec_config);
    return false;
  }
  const CodecConfiguration::CodecSpecific& codec_specific = codec_config.config;
  switch (codec_config.codecType) {
    case CodecType::SBC:
      if (IsOffloadSbcConfigurationValid(codec_specific)) {
        return true;
      }
      return false;
    case CodecType::AAC:
      if (IsOffloadAacConfigurationValid(codec_specific)) {
        return true;
      }
      return false;
    case CodecType::LDAC:
      if (IsOffloadLdacConfigurationValid(codec_specific)) {
        return true;
      }
      return false;
    case CodecType::APTX:
      if (IsOffloadAptxConfigurationValid(codec_specific)) {
        return true;
      }
      return false;
    case CodecType::APTX_HD:
      if (IsOffloadAptxHdConfigurationValid(codec_specific)) {
        return true;
      }
      return false;
    case CodecType::APTX_ADAPTIVE:
      if (IsOffloadAptxAdaptiveConfigurationValid(codec_specific)) {
        return true;
      }
      return false;
    case CodecType::APTX_TWS:
      if (IsOffloadAptxTwsConfigurationValid(codec_specific)) {
        return true;
      }
      return false;
    case CodecType::BA_CELT:
      if (IsOffloadBaCeltConfigurationValid(codec_specific)) {
        return true;
      }
      return false;
    case CodecType::UNKNOWN:
      return false;
  }
  return false;
}

}  // namespace bluetooth_audio
}  // namespace qti
}  // namespace vendor
