/*
 * Copyright (C) 2016 The Android Open Source Project
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

/******************************************************************************
 *
 *  Utility functions to help build and parse the aptX Codec Information
 *  Element and Media Payload.
 *
 ******************************************************************************/

#define LOG_TAG "a2dp_vendor_aptx"

#include "bt_target.h"

#include "a2dp_vendor_aptx.h"

#include <string.h>

#include <base/logging.h>
#include "a2dp_vendor.h"
#include "a2dp_vendor_aptx_encoder.h"
#include "bt_utils.h"
#include "osi/include/log.h"
#include "osi/include/osi.h"

// data type for the aptX Codec Information Element */
typedef struct {
  uint32_t vendorId;
  uint16_t codecId;    /* Codec ID for aptX */
  uint8_t sampleRate;  /* Sampling Frequency */
  uint8_t channelMode; /* STEREO/DUAL/MONO */
  uint8_t future1;
  uint8_t future2;
  btav_a2dp_codec_bits_per_sample_t bits_per_sample;
} tA2DP_APTX_CIE;

/* aptX Source codec capabilities */
static const tA2DP_APTX_CIE a2dp_aptx_src_caps = {
    A2DP_APTX_VENDOR_ID,                                       /* vendorId */
    A2DP_APTX_CODEC_ID_BLUETOOTH,                              /* codecId */
    A2DP_APTX_SAMPLERATE_44100,                                /* sampleRate */
    A2DP_APTX_CHANNELS_STEREO,                                 /* channelMode */
    A2DP_APTX_FUTURE_1,                                        /* future1 */
    A2DP_APTX_FUTURE_2,                                        /* future2 */
    BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16                         /* bits_per_sample */
};
/* aptX offload codec capabilities */
static const tA2DP_APTX_CIE a2dp_aptx_offload_caps = {
    A2DP_APTX_VENDOR_ID,                                       /* vendorId */
    A2DP_APTX_CODEC_ID_BLUETOOTH,                              /* codecId */
    A2DP_APTX_SAMPLERATE_48000,                              /* sampleRate */
    A2DP_APTX_CHANNELS_STEREO,                                 /* channelMode */
    A2DP_APTX_FUTURE_1,                                        /* future1 */
    A2DP_APTX_FUTURE_2,                                        /* future2 */
    BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16                         /* bits_per_sample */
};
/* Default aptX codec configuration */
static const tA2DP_APTX_CIE a2dp_aptx_src_default_config = {
    A2DP_APTX_VENDOR_ID,               /* vendorId */
    A2DP_APTX_CODEC_ID_BLUETOOTH,      /* codecId */
    A2DP_APTX_SAMPLERATE_44100,        /* sampleRate */
    A2DP_APTX_CHANNELS_STEREO,         /* channelMode */
    A2DP_APTX_FUTURE_1,                /* future1 */
    A2DP_APTX_FUTURE_2,                /* future2 */
    BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16 /* bits_per_sample */
};
/* Default aptX offload codec configuration */
static const tA2DP_APTX_CIE a2dp_aptx_offload_default_config = {
    A2DP_APTX_VENDOR_ID,               /* vendorId */
    A2DP_APTX_CODEC_ID_BLUETOOTH,      /* codecId */
    A2DP_APTX_SAMPLERATE_48000,        /* sampleRate */
    A2DP_APTX_CHANNELS_STEREO,         /* channelMode */
    A2DP_APTX_FUTURE_1,                /* future1 */
    A2DP_APTX_FUTURE_2,                /* future2 */
    BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16 /* bits_per_sample */
};
tA2DP_APTX_CIE a2dp_aptx_caps, a2dp_aptx_default_config;

static const tA2DP_ENCODER_INTERFACE a2dp_encoder_interface_aptx = {
    a2dp_vendor_aptx_encoder_init,
    a2dp_vendor_aptx_encoder_cleanup,
    a2dp_vendor_aptx_feeding_reset,
    a2dp_vendor_aptx_feeding_flush,
    a2dp_vendor_aptx_get_encoder_interval_ms,
    a2dp_vendor_aptx_send_frames,
    nullptr  // set_transmit_queue_length
};

UNUSED_ATTR static tA2DP_STATUS A2DP_CodecInfoMatchesCapabilityAptx(
    const tA2DP_APTX_CIE* p_cap, const uint8_t* p_codec_info,
    bool is_peer_codec_info);

// Builds the aptX Media Codec Capabilities byte sequence beginning from the
// LOSC octet. |media_type| is the media type |AVDT_MEDIA_TYPE_*|.
// |p_ie| is a pointer to the aptX Codec Information Element information.
// The result is stored in |p_result|. Returns A2DP_SUCCESS on success,
// otherwise the corresponding A2DP error status code.
static tA2DP_STATUS A2DP_BuildInfoAptx(uint8_t media_type,
                                       const tA2DP_APTX_CIE* p_ie,
                                       uint8_t* p_result) {
  if (p_ie == NULL || p_result == NULL) {
    return A2DP_INVALID_PARAMS;
  }

  *p_result++ = A2DP_APTX_CODEC_LEN;
  *p_result++ = (media_type << 4);
  *p_result++ = A2DP_MEDIA_CT_NON_A2DP;
  *p_result++ = (uint8_t)(p_ie->vendorId & 0x000000FF);
  *p_result++ = (uint8_t)((p_ie->vendorId & 0x0000FF00) >> 8);
  *p_result++ = (uint8_t)((p_ie->vendorId & 0x00FF0000) >> 16);
  *p_result++ = (uint8_t)((p_ie->vendorId & 0xFF000000) >> 24);
  *p_result++ = (uint8_t)(p_ie->codecId & 0x00FF);
  *p_result++ = (uint8_t)((p_ie->codecId & 0xFF00) >> 8);
  *p_result++ = p_ie->sampleRate | p_ie->channelMode;

  return A2DP_SUCCESS;
}

// Parses the aptX Media Codec Capabilities byte sequence beginning from the
// LOSC octet. The result is stored in |p_ie|. The byte sequence to parse is
// |p_codec_info|. If |is_capability| is true, the byte sequence is
// codec capabilities, otherwise is codec configuration.
// Returns A2DP_SUCCESS on success, otherwise the corresponding A2DP error
// status code.
static tA2DP_STATUS A2DP_ParseInfoAptx(tA2DP_APTX_CIE* p_ie,
                                       const uint8_t* p_codec_info,
                                       bool is_capability) {
  uint8_t losc;
  uint8_t media_type;
  tA2DP_CODEC_TYPE codec_type;

  if (p_ie == NULL || p_codec_info == NULL) return A2DP_INVALID_PARAMS;

  // Check the codec capability length
  losc = *p_codec_info++;
  if (losc != A2DP_APTX_CODEC_LEN) return A2DP_WRONG_CODEC;

  media_type = (*p_codec_info++) >> 4;
  codec_type = *p_codec_info++;
  /* Check the Media Type and Media Codec Type */
  if (media_type != AVDT_MEDIA_TYPE_AUDIO ||
      codec_type != A2DP_MEDIA_CT_NON_A2DP) {
    return A2DP_WRONG_CODEC;
  }

  // Check the Vendor ID and Codec ID */
  p_ie->vendorId = (*p_codec_info & 0x000000FF) |
                   (*(p_codec_info + 1) << 8 & 0x0000FF00) |
                   (*(p_codec_info + 2) << 16 & 0x00FF0000) |
                   (*(p_codec_info + 3) << 24 & 0xFF000000);
  p_codec_info += 4;
  p_ie->codecId =
      (*p_codec_info & 0x00FF) | (*(p_codec_info + 1) << 8 & 0xFF00);
  p_codec_info += 2;
  if (p_ie->vendorId != A2DP_APTX_VENDOR_ID ||
      p_ie->codecId != A2DP_APTX_CODEC_ID_BLUETOOTH) {
    return A2DP_WRONG_CODEC;
  }

  p_ie->channelMode = *p_codec_info & 0x0F;
  p_ie->sampleRate = *p_codec_info & 0xF0;
  p_codec_info++;

  if (is_capability) return A2DP_SUCCESS;

  if (A2DP_BitsSet(p_ie->sampleRate) != A2DP_SET_ONE_BIT)
    return A2DP_BAD_SAMP_FREQ;
  if (A2DP_BitsSet(p_ie->channelMode) != A2DP_SET_ONE_BIT)
    return A2DP_BAD_CH_MODE;

  return A2DP_SUCCESS;
}

bool A2DP_IsVendorSourceCodecValidAptx(const uint8_t* p_codec_info) {
  tA2DP_APTX_CIE cfg_cie;

  /* Use a liberal check when parsing the codec info */
  return (A2DP_ParseInfoAptx(&cfg_cie, p_codec_info, false) == A2DP_SUCCESS) ||
         (A2DP_ParseInfoAptx(&cfg_cie, p_codec_info, true) == A2DP_SUCCESS);
}

bool A2DP_IsVendorPeerSinkCodecValidAptx(const uint8_t* p_codec_info) {
  tA2DP_APTX_CIE cfg_cie;

  /* Use a liberal check when parsing the codec info */
  return (A2DP_ParseInfoAptx(&cfg_cie, p_codec_info, false) == A2DP_SUCCESS) ||
         (A2DP_ParseInfoAptx(&cfg_cie, p_codec_info, true) == A2DP_SUCCESS);
}

// Checks whether A2DP aptX codec configuration matches with a device's codec
// capabilities. |p_cap| is the aptX codec configuration. |p_codec_info| is
// the device's codec capabilities.
// If |is_capability| is true, the byte sequence is codec capabilities,
// otherwise is codec configuration.
// |p_codec_info| contains the codec capabilities for a peer device that
// is acting as an A2DP source.
// Returns A2DP_SUCCESS if the codec configuration matches with capabilities,
// otherwise the corresponding A2DP error status code.
static tA2DP_STATUS A2DP_CodecInfoMatchesCapabilityAptx(
    const tA2DP_APTX_CIE* p_cap, const uint8_t* p_codec_info,
    bool is_capability) {
  tA2DP_STATUS status;
  tA2DP_APTX_CIE cfg_cie;

  /* parse configuration */
  status = A2DP_ParseInfoAptx(&cfg_cie, p_codec_info, is_capability);
  if (status != A2DP_SUCCESS) {
    LOG_ERROR(LOG_TAG, "%s: parsing failed %d", __func__, status);
    return status;
  }

  /* verify that each parameter is in range */

  LOG_VERBOSE(LOG_TAG, "%s: FREQ peer: 0x%x, capability 0x%x", __func__,
              cfg_cie.sampleRate, p_cap->sampleRate);
  LOG_VERBOSE(LOG_TAG, "%s: CH_MODE peer: 0x%x, capability 0x%x", __func__,
              cfg_cie.channelMode, p_cap->channelMode);

  /* sampling frequency */
  if ((cfg_cie.sampleRate & p_cap->sampleRate) == 0) return A2DP_NS_SAMP_FREQ;

  /* channel mode */
  if ((cfg_cie.channelMode & p_cap->channelMode) == 0) return A2DP_NS_CH_MODE;

  return A2DP_SUCCESS;
}

bool A2DP_VendorUsesRtpHeaderAptx(UNUSED_ATTR bool content_protection_enabled,
                                  UNUSED_ATTR const uint8_t* p_codec_info) {
#if (BTA_AV_CO_CP_SCMS_T == TRUE)
  return true;
#else
  // no RTP header for aptX classic and no Copy Protection byte
  return false;
#endif
}

const char* A2DP_VendorCodecNameAptx(UNUSED_ATTR const uint8_t* p_codec_info) {
  return "aptX";
}

bool A2DP_VendorCodecTypeEqualsAptx(const uint8_t* p_codec_info_a,
                                    const uint8_t* p_codec_info_b) {
  tA2DP_APTX_CIE aptx_cie_a;
  tA2DP_APTX_CIE aptx_cie_b;

  // Check whether the codec info contains valid data
  tA2DP_STATUS a2dp_status =
      A2DP_ParseInfoAptx(&aptx_cie_a, p_codec_info_a, true);
  if (a2dp_status != A2DP_SUCCESS) {
    LOG_ERROR(LOG_TAG, "%s: cannot decode codec information: %d", __func__,
              a2dp_status);
    return false;
  }
  a2dp_status = A2DP_ParseInfoAptx(&aptx_cie_b, p_codec_info_b, true);
  if (a2dp_status != A2DP_SUCCESS) {
    LOG_ERROR(LOG_TAG, "%s: cannot decode codec information: %d", __func__,
              a2dp_status);
    return false;
  }

  return true;
}

bool A2DP_VendorCodecEqualsAptx(const uint8_t* p_codec_info_a,
                                const uint8_t* p_codec_info_b) {
  tA2DP_APTX_CIE aptx_cie_a;
  tA2DP_APTX_CIE aptx_cie_b;

  // Check whether the codec info contains valid data
  tA2DP_STATUS a2dp_status =
      A2DP_ParseInfoAptx(&aptx_cie_a, p_codec_info_a, true);
  if (a2dp_status != A2DP_SUCCESS) {
    LOG_ERROR(LOG_TAG, "%s: cannot decode codec information: %d", __func__,
              a2dp_status);
    return false;
  }
  a2dp_status = A2DP_ParseInfoAptx(&aptx_cie_b, p_codec_info_b, true);
  if (a2dp_status != A2DP_SUCCESS) {
    LOG_ERROR(LOG_TAG, "%s: cannot decode codec information: %d", __func__,
              a2dp_status);
    return false;
  }

  return (aptx_cie_a.sampleRate == aptx_cie_b.sampleRate) &&
         (aptx_cie_a.channelMode == aptx_cie_b.channelMode);
}

int A2DP_VendorGetTrackSampleRateAptx(const uint8_t* p_codec_info) {
  tA2DP_APTX_CIE aptx_cie;

  // Check whether the codec info contains valid data
  tA2DP_STATUS a2dp_status = A2DP_ParseInfoAptx(&aptx_cie, p_codec_info, false);
  if (a2dp_status != A2DP_SUCCESS) {
    LOG_ERROR(LOG_TAG, "%s: cannot decode codec information: %d", __func__,
              a2dp_status);
    return -1;
  }

  if (aptx_cie.sampleRate == A2DP_APTX_SAMPLERATE_44100) return 44100;
  if (aptx_cie.sampleRate == A2DP_APTX_SAMPLERATE_48000) return 48000;

  return -1;
}

int A2DP_VendorGetTrackChannelCountAptx(const uint8_t* p_codec_info) {
  tA2DP_APTX_CIE aptx_cie;

  // Check whether the codec info contains valid data
  tA2DP_STATUS a2dp_status = A2DP_ParseInfoAptx(&aptx_cie, p_codec_info, false);
  if (a2dp_status != A2DP_SUCCESS) {
    LOG_ERROR(LOG_TAG, "%s: cannot decode codec information: %d", __func__,
              a2dp_status);
    return -1;
  }

  switch (aptx_cie.channelMode) {
    case A2DP_APTX_CHANNELS_MONO:
      return 1;
    case A2DP_APTX_CHANNELS_STEREO:
      return 2;
  }

  return -1;
}

bool A2DP_VendorGetPacketTimestampAptx(UNUSED_ATTR const uint8_t* p_codec_info,
                                       const uint8_t* p_data,
                                       uint32_t* p_timestamp) {
  // TODO: Is this function really codec-specific?
  *p_timestamp = *(const uint32_t*)p_data;
  return true;
}

bool A2DP_VendorBuildCodecHeaderAptx(UNUSED_ATTR const uint8_t* p_codec_info,
                                     UNUSED_ATTR BT_HDR* p_buf,
                                     UNUSED_ATTR uint16_t frames_per_packet) {
  // Nothing to do
  return true;
}

bool A2DP_VendorDumpCodecInfoAptx(const uint8_t* p_codec_info) {
  tA2DP_STATUS a2dp_status;
  tA2DP_APTX_CIE aptx_cie;

  LOG_VERBOSE(LOG_TAG, "%s", __func__);

  a2dp_status = A2DP_ParseInfoAptx(&aptx_cie, p_codec_info, true);
  if (a2dp_status != A2DP_SUCCESS) {
    LOG_ERROR(LOG_TAG, "%s: A2DP_ParseInfoAptx fail:%d", __func__, a2dp_status);
    return false;
  }

  LOG_VERBOSE(LOG_TAG, "\tsamp_freq: 0x%x", aptx_cie.sampleRate);
  if (aptx_cie.sampleRate & A2DP_APTX_SAMPLERATE_44100) {
    LOG_VERBOSE(LOG_TAG, "\tsamp_freq: (44100)");
  }
  if (aptx_cie.sampleRate & A2DP_APTX_SAMPLERATE_48000) {
    LOG_VERBOSE(LOG_TAG, "\tsamp_freq: (48000)");
  }

  LOG_VERBOSE(LOG_TAG, "\tch_mode: 0x%x", aptx_cie.channelMode);
  if (aptx_cie.channelMode & A2DP_APTX_CHANNELS_MONO) {
    LOG_VERBOSE(LOG_TAG, "\tch_mode: (Mono)");
  }
  if (aptx_cie.channelMode & A2DP_APTX_CHANNELS_STEREO) {
    LOG_VERBOSE(LOG_TAG, "\tch_mode: (Stereo)");
  }

  return true;
}

const tA2DP_ENCODER_INTERFACE* A2DP_VendorGetEncoderInterfaceAptx(
    const uint8_t* p_codec_info) {
  if (!A2DP_IsVendorSourceCodecValidAptx(p_codec_info)) return NULL;

  return &a2dp_encoder_interface_aptx;
}

bool A2DP_VendorAdjustCodecAptx(uint8_t* p_codec_info) {
  tA2DP_APTX_CIE cfg_cie;

  // Nothing to do: just verify the codec info is valid
  if (A2DP_ParseInfoAptx(&cfg_cie, p_codec_info, true) != A2DP_SUCCESS)
    return false;

  return true;
}

btav_a2dp_codec_index_t A2DP_VendorSourceCodecIndexAptx(
    UNUSED_ATTR const uint8_t* p_codec_info) {
  return BTAV_A2DP_CODEC_INDEX_SOURCE_APTX;
}

const char* A2DP_VendorCodecIndexStrAptx(void) { return "aptX"; }

bool A2DP_VendorInitCodecConfigAptx(tAVDT_CFG* p_cfg) {
  if (!A2DP_IsCodecEnabled(BTAV_A2DP_CODEC_INDEX_SOURCE_APTX)) {
    LOG_ERROR(LOG_TAG, "%s: APTX disabled in both SW and HW mode", __func__);
    return false;
  }

  if (A2DP_BuildInfoAptx(AVDT_MEDIA_TYPE_AUDIO, &a2dp_aptx_caps,
                         p_cfg->codec_info) != A2DP_SUCCESS) {
    return false;
  }

#if (BTA_AV_CO_CP_SCMS_T == TRUE)
  /* Content protection info - support SCMS-T */
  uint8_t* p = p_cfg->protect_info;
  *p++ = AVDT_CP_LOSC;
  UINT16_TO_STREAM(p, AVDT_CP_SCMS_T_ID);
  p_cfg->num_protect = 1;
#endif

  return true;
}

A2dpCodecConfigAptx::A2dpCodecConfigAptx(
    btav_a2dp_codec_priority_t codec_priority)
    : A2dpCodecConfig(BTAV_A2DP_CODEC_INDEX_SOURCE_APTX, "aptX",
                      codec_priority) {
  // Compute the local capability
    if (A2DP_IsCodecEnabledInOffload(BTAV_A2DP_CODEC_INDEX_SOURCE_APTX)&&
      !(A2DP_IsScramblingSupported() || A2DP_Is44p1kFreqSupported())) {
      a2dp_aptx_caps = a2dp_aptx_offload_caps;
      a2dp_aptx_default_config = a2dp_aptx_offload_default_config;
    } else {
      a2dp_aptx_caps = a2dp_aptx_src_caps;
      a2dp_aptx_default_config = a2dp_aptx_src_default_config;
    }
  if (a2dp_aptx_caps.sampleRate & A2DP_APTX_SAMPLERATE_44100) {
    codec_local_capability_.sample_rate |= BTAV_A2DP_CODEC_SAMPLE_RATE_44100;
  }
  if (a2dp_aptx_caps.sampleRate & A2DP_APTX_SAMPLERATE_48000) {
    codec_local_capability_.sample_rate |= BTAV_A2DP_CODEC_SAMPLE_RATE_48000;
  }
  codec_local_capability_.bits_per_sample = a2dp_aptx_caps.bits_per_sample;
  if (a2dp_aptx_caps.channelMode & A2DP_APTX_CHANNELS_MONO) {
    codec_local_capability_.channel_mode |= BTAV_A2DP_CODEC_CHANNEL_MODE_MONO;
  }
  if (a2dp_aptx_caps.channelMode & A2DP_APTX_CHANNELS_STEREO) {
    codec_local_capability_.channel_mode |= BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO;
  }
}

A2dpCodecConfigAptx::~A2dpCodecConfigAptx() {}

bool A2dpCodecConfigAptx::init() {
  if (!isValid()) return false;

  if (A2DP_IsCodecEnabledInOffload(BTAV_A2DP_CODEC_INDEX_SOURCE_APTX)) {
    LOG_DEBUG(LOG_TAG, "%s: APTX enabled in HW mode", __func__);
    return true;
  } else if(!A2DP_IsCodecEnabledInSoftware(BTAV_A2DP_CODEC_INDEX_SOURCE_APTX)){
    LOG_DEBUG(LOG_TAG, "%s: APTX disabled in both SW and HW mode", __func__);
    return false;
  } else {
    LOG_DEBUG(LOG_TAG, "%s: APTX enabled in SW mode", __func__);
  }

  // Load the encoder
  if (!A2DP_VendorLoadEncoderAptx()) {
    LOG_ERROR(LOG_TAG, "%s: cannot load the encoder", __func__);
    return false;
  }

  return true;
}

bool A2dpCodecConfigAptx::useRtpHeaderMarkerBit() const { return false; }

//
// Selects the best sample rate from |sampleRate|.
// The result is stored in |p_result| and p_codec_config|.
// Returns true if a selection was made, otherwise false.
//
static bool select_best_sample_rate(uint8_t sampleRate,
                                    tA2DP_APTX_CIE* p_result,
                                    btav_a2dp_codec_config_t* p_codec_config) {
  if (sampleRate & A2DP_APTX_SAMPLERATE_48000) {
    p_result->sampleRate = A2DP_APTX_SAMPLERATE_48000;
    p_codec_config->sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_48000;
    return true;
  }
  if (sampleRate & A2DP_APTX_SAMPLERATE_44100) {
    p_result->sampleRate = A2DP_APTX_SAMPLERATE_44100;
    p_codec_config->sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_44100;
    return true;
  }
  return false;
}

//
// Selects the audio sample rate from |p_codec_audio_config|.
// |sampleRate| contains the capability.
// The result is stored in |p_result| and |p_codec_config|.
// Returns true if a selection was made, otherwise false.
//
static bool select_audio_sample_rate(
    const btav_a2dp_codec_config_t* p_codec_audio_config, uint8_t sampleRate,
    tA2DP_APTX_CIE* p_result, btav_a2dp_codec_config_t* p_codec_config) {
  switch (p_codec_audio_config->sample_rate) {
    case BTAV_A2DP_CODEC_SAMPLE_RATE_44100:
      if (sampleRate & A2DP_APTX_SAMPLERATE_44100) {
        p_result->sampleRate = A2DP_APTX_SAMPLERATE_44100;
        p_codec_config->sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_44100;
        return true;
      }
      break;
    case BTAV_A2DP_CODEC_SAMPLE_RATE_48000:
      if (sampleRate & A2DP_APTX_SAMPLERATE_48000) {
        p_result->sampleRate = A2DP_APTX_SAMPLERATE_48000;
        p_codec_config->sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_48000;
        return true;
      }
      break;
    case BTAV_A2DP_CODEC_SAMPLE_RATE_88200:
    case BTAV_A2DP_CODEC_SAMPLE_RATE_96000:
    case BTAV_A2DP_CODEC_SAMPLE_RATE_176400:
    case BTAV_A2DP_CODEC_SAMPLE_RATE_192000:
    case BTAV_A2DP_CODEC_SAMPLE_RATE_NONE:
    default:
      break;
  }
  return false;
}

//
// Selects the best bits per sample.
// The result is stored in |p_codec_config|.
// Returns true if a selection was made, otherwise false.
//
static bool select_best_bits_per_sample(
    btav_a2dp_codec_config_t* p_codec_config) {
  p_codec_config->bits_per_sample = BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16;
  return true;
}

//
// Selects the audio bits per sample from |p_codec_audio_config|.
// The result is stored in |p_codec_config|.
// Returns true if a selection was made, otherwise false.
//
static bool select_audio_bits_per_sample(
    const btav_a2dp_codec_config_t* p_codec_audio_config,
    btav_a2dp_codec_config_t* p_codec_config) {
  switch (p_codec_audio_config->bits_per_sample) {
    case BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16:
      p_codec_config->bits_per_sample = BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16;
      return true;
    case BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24:
    case BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32:
    case BTAV_A2DP_CODEC_BITS_PER_SAMPLE_NONE:
      break;
  }
  return false;
}

//
// Selects the best channel mode from |channelMode|.
// The result is stored in |p_result| and |p_codec_config|.
// Returns true if a selection was made, otherwise false.
//
static bool select_best_channel_mode(uint8_t channelMode,
                                     tA2DP_APTX_CIE* p_result,
                                     btav_a2dp_codec_config_t* p_codec_config) {
  if (channelMode & A2DP_APTX_CHANNELS_STEREO) {
    p_result->channelMode = A2DP_APTX_CHANNELS_STEREO;
    p_codec_config->channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO;
    return true;
  }
  if (channelMode & A2DP_APTX_CHANNELS_MONO) {
    p_result->channelMode = A2DP_APTX_CHANNELS_MONO;
    p_codec_config->channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_MONO;
    return true;
  }
  return false;
}

//
// Selects the audio channel mode from |p_codec_audio_config|.
// |channelMode| contains the capability.
// The result is stored in |p_result| and |p_codec_config|.
// Returns true if a selection was made, otherwise false.
//
static bool select_audio_channel_mode(
    const btav_a2dp_codec_config_t* p_codec_audio_config, uint8_t channelMode,
    tA2DP_APTX_CIE* p_result, btav_a2dp_codec_config_t* p_codec_config) {
  switch (p_codec_audio_config->channel_mode) {
    case BTAV_A2DP_CODEC_CHANNEL_MODE_MONO:
      if (channelMode & A2DP_APTX_CHANNELS_MONO) {
        p_result->channelMode = A2DP_APTX_CHANNELS_MONO;
        p_codec_config->channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_MONO;
        return true;
      }
      break;
    case BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO:
      if (channelMode & A2DP_APTX_CHANNELS_STEREO) {
        p_result->channelMode = A2DP_APTX_CHANNELS_STEREO;
        p_codec_config->channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO;
        return true;
      }
      break;
    case BTAV_A2DP_CODEC_CHANNEL_MODE_NONE:
      break;
  }

  return false;
}

bool A2dpCodecConfigAptx::setCodecConfig(const uint8_t* p_peer_codec_info,
                                         bool is_capability,
                                         uint8_t* p_result_codec_config) {
  std::lock_guard<std::recursive_mutex> lock(codec_mutex_);
  tA2DP_APTX_CIE sink_info_cie;
  tA2DP_APTX_CIE result_config_cie;
  uint8_t sampleRate;
  uint8_t channelMode;

  // Save the internal state
  btav_a2dp_codec_config_t saved_codec_config = codec_config_;
  btav_a2dp_codec_config_t saved_codec_capability = codec_capability_;
  btav_a2dp_codec_config_t saved_codec_selectable_capability =
      codec_selectable_capability_;
  btav_a2dp_codec_config_t saved_codec_user_config = codec_user_config_;
  btav_a2dp_codec_config_t saved_codec_audio_config = codec_audio_config_;
  uint8_t saved_ota_codec_config[AVDT_CODEC_SIZE];
  uint8_t saved_ota_codec_peer_capability[AVDT_CODEC_SIZE];
  uint8_t saved_ota_codec_peer_config[AVDT_CODEC_SIZE];
  memcpy(saved_ota_codec_config, ota_codec_config_, sizeof(ota_codec_config_));
  memcpy(saved_ota_codec_peer_capability, ota_codec_peer_capability_,
         sizeof(ota_codec_peer_capability_));
  memcpy(saved_ota_codec_peer_config, ota_codec_peer_config_,
         sizeof(ota_codec_peer_config_));

  tA2DP_STATUS status =
      A2DP_ParseInfoAptx(&sink_info_cie, p_peer_codec_info, is_capability);
  if (status != A2DP_SUCCESS) {
    LOG_ERROR(LOG_TAG, "%s: can't parse peer's Sink capabilities: error = %d",
              __func__, status);
    goto fail;
  }

  //
  // Build the preferred configuration
  //
  memset(&result_config_cie, 0, sizeof(result_config_cie));
  result_config_cie.vendorId = a2dp_aptx_caps.vendorId;
  result_config_cie.codecId = a2dp_aptx_caps.codecId;

  //
  // Select the sample frequency
  //
  sampleRate = a2dp_aptx_caps.sampleRate & sink_info_cie.sampleRate;
  codec_config_.sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_NONE;
  switch (codec_user_config_.sample_rate) {
    case BTAV_A2DP_CODEC_SAMPLE_RATE_44100:
      if (sampleRate & A2DP_APTX_SAMPLERATE_44100) {
        result_config_cie.sampleRate = A2DP_APTX_SAMPLERATE_44100;
        codec_capability_.sample_rate = codec_user_config_.sample_rate;
        codec_config_.sample_rate = codec_user_config_.sample_rate;
      }
      break;
    case BTAV_A2DP_CODEC_SAMPLE_RATE_48000:
      if (sampleRate & A2DP_APTX_SAMPLERATE_48000) {
        result_config_cie.sampleRate = A2DP_APTX_SAMPLERATE_48000;
        codec_capability_.sample_rate = codec_user_config_.sample_rate;
        codec_config_.sample_rate = codec_user_config_.sample_rate;
      }
      break;
    case BTAV_A2DP_CODEC_SAMPLE_RATE_88200:
    case BTAV_A2DP_CODEC_SAMPLE_RATE_96000:
    case BTAV_A2DP_CODEC_SAMPLE_RATE_176400:
    case BTAV_A2DP_CODEC_SAMPLE_RATE_192000:
    case BTAV_A2DP_CODEC_SAMPLE_RATE_NONE:
      codec_capability_.sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_NONE;
      codec_config_.sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_NONE;
      break;
    default:
      break;
  }

  // Select the sample frequency if there is no user preference
  do {
    // Compute the selectable capability
    if (sampleRate & A2DP_APTX_SAMPLERATE_44100) {
      codec_selectable_capability_.sample_rate |=
          BTAV_A2DP_CODEC_SAMPLE_RATE_44100;
    }
    if (sampleRate & A2DP_APTX_SAMPLERATE_48000) {
      codec_selectable_capability_.sample_rate |=
          BTAV_A2DP_CODEC_SAMPLE_RATE_48000;
    }

    if (codec_config_.sample_rate != BTAV_A2DP_CODEC_SAMPLE_RATE_NONE) break;

    // Compute the common capability
    if (sampleRate & A2DP_APTX_SAMPLERATE_44100)
      codec_capability_.sample_rate |= BTAV_A2DP_CODEC_SAMPLE_RATE_44100;
    if (sampleRate & A2DP_APTX_SAMPLERATE_48000)
      codec_capability_.sample_rate |= BTAV_A2DP_CODEC_SAMPLE_RATE_48000;

    // No user preference - try the codec audio config
    if (select_audio_sample_rate(&codec_audio_config_, sampleRate,
                                 &result_config_cie, &codec_config_)) {
      break;
    }

    // No user preference - try the default config
    if (select_best_sample_rate(
            a2dp_aptx_default_config.sampleRate & sink_info_cie.sampleRate,
            &result_config_cie, &codec_config_)) {
      break;
    }

    // No user preference - use the best match
    if (select_best_sample_rate(sampleRate, &result_config_cie,
                                &codec_config_)) {
      break;
    }
  } while (false);
  if (codec_config_.sample_rate == BTAV_A2DP_CODEC_SAMPLE_RATE_NONE) {
    LOG_ERROR(LOG_TAG,
              "%s: cannot match sample frequency: source caps = 0x%x "
              "sink info = 0x%x",
              __func__, a2dp_aptx_caps.sampleRate, sink_info_cie.sampleRate);
    goto fail;
  }

  //
  // Select the bits per sample
  //
  // NOTE: this information is NOT included in the aptX A2DP codec
  // description that is sent OTA.
  codec_config_.bits_per_sample = BTAV_A2DP_CODEC_BITS_PER_SAMPLE_NONE;
  switch (codec_user_config_.bits_per_sample) {
    case BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16:
      codec_capability_.bits_per_sample = codec_user_config_.bits_per_sample;
      codec_config_.bits_per_sample = codec_user_config_.bits_per_sample;
      break;
    case BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24:
    case BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32:
    case BTAV_A2DP_CODEC_BITS_PER_SAMPLE_NONE:
      codec_capability_.bits_per_sample = BTAV_A2DP_CODEC_BITS_PER_SAMPLE_NONE;
      codec_config_.bits_per_sample = BTAV_A2DP_CODEC_BITS_PER_SAMPLE_NONE;
      break;
  }

  // Select the bits per sample if there is no user preference
  do {
    // Compute the selectable capability
    codec_selectable_capability_.bits_per_sample =
        a2dp_aptx_caps.bits_per_sample;

    if (codec_config_.bits_per_sample != BTAV_A2DP_CODEC_BITS_PER_SAMPLE_NONE)
      break;

    // Compute the common capability
    codec_capability_.bits_per_sample = BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16;

    // No user preference - try the codec audio config
    if (select_audio_bits_per_sample(&codec_audio_config_, &codec_config_)) {
      break;
    }

    // No user preference - try the default config
    if (select_best_bits_per_sample(&codec_config_)) {
      break;
    }

    // No user preference - use the best match
    // NOTE: no-op - kept here for consistency
    if (select_best_bits_per_sample(&codec_config_)) {
      break;
    }
  } while (false);
  if (codec_config_.bits_per_sample == BTAV_A2DP_CODEC_BITS_PER_SAMPLE_NONE) {
    LOG_ERROR(LOG_TAG,
              "%s: cannot match bits per sample: user preference = 0x%x",
              __func__, codec_user_config_.bits_per_sample);
    goto fail;
  }

  //
  // Select the channel mode
  //
  channelMode = a2dp_aptx_caps.channelMode & sink_info_cie.channelMode;
  codec_config_.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_NONE;
  switch (codec_user_config_.channel_mode) {
    case BTAV_A2DP_CODEC_CHANNEL_MODE_MONO:
      if (channelMode & A2DP_APTX_CHANNELS_MONO) {
        result_config_cie.channelMode = A2DP_APTX_CHANNELS_MONO;
        codec_capability_.channel_mode = codec_user_config_.channel_mode;
        codec_config_.channel_mode = codec_user_config_.channel_mode;
      }
      break;
    case BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO:
      if (channelMode & A2DP_APTX_CHANNELS_STEREO) {
        result_config_cie.channelMode = A2DP_APTX_CHANNELS_STEREO;
        codec_capability_.channel_mode = codec_user_config_.channel_mode;
        codec_config_.channel_mode = codec_user_config_.channel_mode;
      }
      break;
    case BTAV_A2DP_CODEC_CHANNEL_MODE_NONE:
      codec_capability_.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_NONE;
      codec_config_.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_NONE;
      break;
  }

  // Select the channel mode if there is no user preference
  do {
    // Compute the selectable capability
    if (channelMode & A2DP_APTX_CHANNELS_MONO) {
      codec_selectable_capability_.channel_mode |=
          BTAV_A2DP_CODEC_CHANNEL_MODE_MONO;
    }
    if (channelMode & A2DP_APTX_CHANNELS_STEREO) {
      codec_selectable_capability_.channel_mode |=
          BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO;
    }

    if (codec_config_.channel_mode != BTAV_A2DP_CODEC_CHANNEL_MODE_NONE) break;

    // Compute the common capability
    if (channelMode & A2DP_APTX_CHANNELS_MONO)
      codec_capability_.channel_mode |= BTAV_A2DP_CODEC_CHANNEL_MODE_MONO;
    if (channelMode & A2DP_APTX_CHANNELS_STEREO)
      codec_capability_.channel_mode |= BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO;

    // No user preference - try the codec audio config
    if (select_audio_channel_mode(&codec_audio_config_, channelMode,
                                  &result_config_cie, &codec_config_)) {
      break;
    }

    // No user preference - try the default config
    if (select_best_channel_mode(
            a2dp_aptx_default_config.channelMode & sink_info_cie.channelMode,
            &result_config_cie, &codec_config_)) {
      break;
    }

    // No user preference - use the best match
    if (select_best_channel_mode(channelMode, &result_config_cie,
                                 &codec_config_)) {
      break;
    }
  } while (false);
  if (codec_config_.channel_mode == BTAV_A2DP_CODEC_CHANNEL_MODE_NONE) {
    LOG_ERROR(LOG_TAG,
              "%s: cannot match channel mode: source caps = 0x%x "
              "sink info = 0x%x",
              __func__, a2dp_aptx_caps.channelMode, sink_info_cie.channelMode);
    goto fail;
  }

  //
  // Set the rest of the fields as bit-wise AND operation
  //
  result_config_cie.future1 = a2dp_aptx_caps.future1 & sink_info_cie.future1;
  result_config_cie.future2 = a2dp_aptx_caps.future2 & sink_info_cie.future2;

  if (A2DP_BuildInfoAptx(AVDT_MEDIA_TYPE_AUDIO, &result_config_cie,
                         p_result_codec_config) != A2DP_SUCCESS) {
    goto fail;
  }

  //
  // Copy the codec-specific fields if they are not zero
  //
  if (codec_user_config_.codec_specific_1 != 0)
    codec_config_.codec_specific_1 = codec_user_config_.codec_specific_1;
  if (codec_user_config_.codec_specific_2 != 0)
    codec_config_.codec_specific_2 = codec_user_config_.codec_specific_2;
  if (codec_user_config_.codec_specific_3 != 0)
    codec_config_.codec_specific_3 = codec_user_config_.codec_specific_3;
  if (codec_user_config_.codec_specific_4 != 0)
    codec_config_.codec_specific_4 = codec_user_config_.codec_specific_4;

  // Create a local copy of the peer codec capability/config, and the
  // result codec config.
  if (is_capability) {
    status = A2DP_BuildInfoAptx(AVDT_MEDIA_TYPE_AUDIO, &sink_info_cie,
                                ota_codec_peer_capability_);
  } else {
    status = A2DP_BuildInfoAptx(AVDT_MEDIA_TYPE_AUDIO, &sink_info_cie,
                                ota_codec_peer_config_);
  }
  CHECK(status == A2DP_SUCCESS);
  status = A2DP_BuildInfoAptx(AVDT_MEDIA_TYPE_AUDIO, &result_config_cie,
                              ota_codec_config_);
  CHECK(status == A2DP_SUCCESS);

  return true;

fail:
  // Restore the internal state
  codec_config_ = saved_codec_config;
  codec_capability_ = saved_codec_capability;
  codec_selectable_capability_ = saved_codec_selectable_capability;
  codec_user_config_ = saved_codec_user_config;
  codec_audio_config_ = saved_codec_audio_config;
  memcpy(ota_codec_config_, saved_ota_codec_config, sizeof(ota_codec_config_));
  memcpy(ota_codec_peer_capability_, saved_ota_codec_peer_capability,
         sizeof(ota_codec_peer_capability_));
  memcpy(ota_codec_peer_config_, saved_ota_codec_peer_config,
         sizeof(ota_codec_peer_config_));
  return false;
}
