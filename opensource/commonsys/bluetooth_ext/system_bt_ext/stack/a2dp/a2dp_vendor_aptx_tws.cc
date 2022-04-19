/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 * "Not a contribution"
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.

    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 * GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
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
 *  Utility functions to help build and parse the aptX-TWS Codec Information
 *  Element and Media Payload.
 *
 ******************************************************************************/

#define LOG_TAG "a2dp_vendor_aptx_tws"

#include "bt_target.h"

#include "a2dp_vendor_aptx_tws.h"
#include <hardware/bt_av.h>

#include <string.h>

#include <base/logging.h>
#include "a2dp_vendor.h"
#include "a2dp_vendor_aptx_tws_encoder.h"
#include "bt_utils.h"
#include "osi/include/log.h"
#include "osi/include/osi.h"

#if (TWS_ENABLED == TRUE)
// data type for the aptX-TWS Codec Information Element */
typedef struct {
  uint32_t vendorId;
  uint16_t codecId;    /* Codec ID for aptX-TWS */
  uint8_t sampleRate;  /* Sampling Frequency */
  uint8_t channelMode; /* STEREO/DUAL/MONO */
  btav_a2dp_codec_bits_per_sample_t bits_per_sample;
} tA2DP_APTX_TWS_CIE;

#if 0
/* aptX-TWS Source codec capabilities */
static const tA2DP_APTX_TWS_CIE a2dp_aptx_tws_src_caps = {
//static const tA2DP_APTX_TWS_CIE a2dp_aptx_tws_caps = {
    A2DP_APTX_TWS_VENDOR_ID,          /* vendorId */
    A2DP_APTX_TWS_CODEC_ID_BLUETOOTH, /* codecId */
    (A2DP_APTX_TWS_SAMPLERATE_44100 |
     A2DP_APTX_TWS_SAMPLERATE_48000),   /* sampleRate */
    A2DP_APTX_TWS_CHANNELS_MONO,      /* channelMode */
    BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16 /* bits_per_sample */
};
#endif
/* aptX-TWS offload codec capabilities */
static const tA2DP_APTX_TWS_CIE a2dp_aptx_tws_offload_caps = {
//static const tA2DP_APTX_TWS_CIE a2dp_aptx_tws_caps = {
    A2DP_APTX_TWS_VENDOR_ID,          /* vendorId */
    A2DP_APTX_TWS_CODEC_ID_BLUETOOTH, /* codecId */
    A2DP_APTX_TWS_SAMPLERATE_48000,   /* sampleRate */
    A2DP_APTX_TWS_CHANNELS_MONO,      /* channelMode */
    BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16 /* bits_per_sample */
};
#if 0
/* Default aptX-TWS codec configuration */
static const tA2DP_APTX_TWS_CIE a2dp_aptx_tws_default_src_config = {
//static const tA2DP_APTX_TWS_CIE a2dp_aptx_tws_default_config = {
    A2DP_APTX_TWS_VENDOR_ID,            /* vendorId */
    A2DP_APTX_TWS_CODEC_ID_BLUETOOTH,   /* codecId */
    A2DP_APTX_TWS_SAMPLERATE_44100,     /* sampleRate */
    A2DP_APTX_TWS_CHANNELS_MONO,      /* channelMode */
    BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16 /* bits_per_sample */
};
#endif
/* Default aptX-TWS offload codec configuration */
static const tA2DP_APTX_TWS_CIE a2dp_aptx_tws_default_offload_config = {
//static const tA2DP_APTX_TWS_CIE a2dp_aptx_tws_default_config = {
    A2DP_APTX_TWS_VENDOR_ID,            /* vendorId */
    A2DP_APTX_TWS_CODEC_ID_BLUETOOTH,   /* codecId */
    A2DP_APTX_TWS_SAMPLERATE_48000,     /* sampleRate */
    A2DP_APTX_TWS_CHANNELS_MONO,      /* channelMode */
    BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16 /* bits_per_sample */
};
tA2DP_APTX_TWS_CIE a2dp_aptx_tws_caps, a2dp_aptx_tws_default_config;

static const tA2DP_ENCODER_INTERFACE a2dp_encoder_interface_aptx_tws = {
    a2dp_vendor_aptx_tws_encoder_init,
    a2dp_vendor_aptx_tws_encoder_cleanup,
    a2dp_vendor_aptx_tws_feeding_reset,
    a2dp_vendor_aptx_tws_feeding_flush,
    a2dp_vendor_aptx_tws_get_encoder_interval_ms,
    a2dp_vendor_aptx_tws_send_frames,
    nullptr  // set_transmit_queue_length
};

UNUSED_ATTR static tA2DP_STATUS A2DP_CodecInfoMatchesCapabilityAptxTWS(
    const tA2DP_APTX_TWS_CIE* p_cap, const uint8_t* p_codec_info,
    bool is_peer_codec_info);

// Builds the aptX-TWS Media Codec Capabilities byte sequence beginning from the
// LOSC octet. |media_type| is the media type |AVDT_MEDIA_TYPE_*|.
// |p_ie| is a pointer to the aptX-TWS Codec Information Element information.
// The result is stored in |p_result|. Returns A2DP_SUCCESS on success,
// otherwise the corresponding A2DP error status code.
static tA2DP_STATUS A2DP_BuildInfoAptxTWS(uint8_t media_type,
                                         const tA2DP_APTX_TWS_CIE* p_ie,
                                         uint8_t* p_result) {
  if (p_ie == NULL || p_result == NULL) {
    return A2DP_INVALID_PARAMS;
  }

  *p_result++ = A2DP_APTX_TWS_CODEC_LEN;
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

// Parses the aptX-TWS Media Codec Capabilities byte sequence beginning from the
// LOSC octet. The result is stored in |p_ie|. The byte sequence to parse is
// |p_codec_info|. If |is_capability| is true, the byte sequence is
// codec capabilities, otherwise is codec configuration.
// Returns A2DP_SUCCESS on success, otherwise the corresponding A2DP error
// status code.
static tA2DP_STATUS A2DP_ParseInfoAptxTWS(tA2DP_APTX_TWS_CIE* p_ie,
                                         const uint8_t* p_codec_info,
                                         bool is_capability) {
  uint8_t losc;
  uint8_t media_type;
  tA2DP_CODEC_TYPE codec_type;

  if (p_ie == NULL || p_codec_info == NULL) return A2DP_INVALID_PARAMS;

  // Check the codec capability length
  losc = *p_codec_info++;
  if (losc != A2DP_APTX_TWS_CODEC_LEN) return A2DP_WRONG_CODEC;

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
  if (p_ie->vendorId != A2DP_APTX_TWS_VENDOR_ID ||
      p_ie->codecId != A2DP_APTX_TWS_CODEC_ID_BLUETOOTH) {
    return A2DP_WRONG_CODEC;
  }

  p_ie->channelMode = *p_codec_info & 0x0F;
  p_ie->sampleRate = *p_codec_info & 0xF0;
  p_codec_info++;

//  p_ie->acl_sprint_reserved0 = *(p_codec_info++);
//  p_ie->acl_sprint_reserved1 = *(p_codec_info++);
//  p_ie->acl_sprint_reserved2 = *(p_codec_info++);
//  p_ie->acl_sprint_reserved3 = *(p_codec_info++);

  if (is_capability) return A2DP_SUCCESS;

  if (A2DP_BitsSet(p_ie->sampleRate) != A2DP_SET_ONE_BIT)
    return A2DP_BAD_SAMP_FREQ;
  if (A2DP_BitsSet(p_ie->channelMode) != A2DP_SET_ONE_BIT)
    return A2DP_BAD_CH_MODE;

  return A2DP_SUCCESS;
}

bool A2DP_IsVendorSourceCodecValidAptxTWS(const uint8_t* p_codec_info) {
  tA2DP_APTX_TWS_CIE cfg_cie;

  /* Use a liberal check when parsing the codec info */
  return (A2DP_ParseInfoAptxTWS(&cfg_cie, p_codec_info, false) ==
          A2DP_SUCCESS) ||
         (A2DP_ParseInfoAptxTWS(&cfg_cie, p_codec_info, true) == A2DP_SUCCESS);
}

bool A2DP_IsVendorPeerSinkCodecValidAptxTWS(const uint8_t* p_codec_info) {
  tA2DP_APTX_TWS_CIE cfg_cie;

  /* Use a liberal check when parsing the codec info */
  return (A2DP_ParseInfoAptxTWS(&cfg_cie, p_codec_info, false) ==
          A2DP_SUCCESS) ||
         (A2DP_ParseInfoAptxTWS(&cfg_cie, p_codec_info, true) == A2DP_SUCCESS);
}

// Checks whether A2DP aptX-TWS codec configuration matches with a device's
// codec capabilities. |p_cap| is the aptX-TWS codec configuration.
// |p_codec_info| is the device's codec capabilities.
// If |is_capability| is true, the byte sequence is codec capabilities,
// otherwise is codec configuration.
// |p_codec_info| contains the codec capabilities for a peer device that
// is acting as an A2DP source.
// Returns A2DP_SUCCESS if the codec configuration matches with capabilities,
// otherwise the corresponding A2DP error status code.
static tA2DP_STATUS A2DP_CodecInfoMatchesCapabilityAptxTWS(
    const tA2DP_APTX_TWS_CIE* p_cap, const uint8_t* p_codec_info,
    bool is_capability) {
  tA2DP_STATUS status;
  tA2DP_APTX_TWS_CIE cfg_cie;

  /* parse configuration */
  status = A2DP_ParseInfoAptxTWS(&cfg_cie, p_codec_info, is_capability);
  if (status != A2DP_SUCCESS) {
    LOG_ERROR(LOG_TAG, "%s: parsing failed %d", __func__, status);
    return status;
  }

  /* verify that each parameter is in range */

  LOG_DEBUG(LOG_TAG, "%s: FREQ peer: 0x%x, capability 0x%x", __func__,
            cfg_cie.sampleRate, p_cap->sampleRate);
  LOG_DEBUG(LOG_TAG, "%s: CH_MODE peer: 0x%x, capability 0x%x", __func__,
            cfg_cie.channelMode, p_cap->channelMode);

  /* sampling frequency */
  if ((cfg_cie.sampleRate & p_cap->sampleRate) == 0) return A2DP_NS_SAMP_FREQ;

  /* channel mode */
  if ((cfg_cie.channelMode & p_cap->channelMode) == 0) return A2DP_NS_CH_MODE;

  return A2DP_SUCCESS;
}

bool A2DP_VendorUsesRtpHeaderAptxTWS(UNUSED_ATTR bool content_protection_enabled,
                                    UNUSED_ATTR const uint8_t* p_codec_info) {
  return true;
}

const char* A2DP_VendorCodecNameAptxTWS(
    UNUSED_ATTR const uint8_t* p_codec_info) {
  return "aptX-TWS";
}

bool A2DP_VendorCodecTypeEqualsAptxTWS(const uint8_t* p_codec_info_a,
                                      const uint8_t* p_codec_info_b) {
  tA2DP_APTX_TWS_CIE aptx_tws_cie_a;
  tA2DP_APTX_TWS_CIE aptx_tws_cie_b;

  // Check whether the codec info contains valid data
  tA2DP_STATUS a2dp_status =
      A2DP_ParseInfoAptxTWS(&aptx_tws_cie_a, p_codec_info_a, true);
  if (a2dp_status != A2DP_SUCCESS) {
    LOG_ERROR(LOG_TAG, "%s: cannot decode codec information: %d", __func__,
              a2dp_status);
    return false;
  }
  a2dp_status = A2DP_ParseInfoAptxTWS(&aptx_tws_cie_b, p_codec_info_b, true);
  if (a2dp_status != A2DP_SUCCESS) {
    LOG_ERROR(LOG_TAG, "%s: cannot decode codec information: %d", __func__,
              a2dp_status);
    return false;
  }

  return true;
}

bool A2DP_VendorCodecEqualsAptxTWS(const uint8_t* p_codec_info_a,
                                  const uint8_t* p_codec_info_b) {
  tA2DP_APTX_TWS_CIE aptx_tws_cie_a;
  tA2DP_APTX_TWS_CIE aptx_tws_cie_b;

  // Check whether the codec info contains valid data
  tA2DP_STATUS a2dp_status =
      A2DP_ParseInfoAptxTWS(&aptx_tws_cie_a, p_codec_info_a, true);
  if (a2dp_status != A2DP_SUCCESS) {
    LOG_ERROR(LOG_TAG, "%s: cannot decode codec information: %d", __func__,
              a2dp_status);
    return false;
  }
  a2dp_status = A2DP_ParseInfoAptxTWS(&aptx_tws_cie_b, p_codec_info_b, true);
  if (a2dp_status != A2DP_SUCCESS) {
    LOG_ERROR(LOG_TAG, "%s: cannot decode codec information: %d", __func__,
              a2dp_status);
    return false;
  }

  return (aptx_tws_cie_a.sampleRate == aptx_tws_cie_b.sampleRate) &&
         (aptx_tws_cie_a.channelMode == aptx_tws_cie_b.channelMode);
}

int A2DP_VendorGetTrackSampleRateAptxTWS(const uint8_t* p_codec_info) {
  tA2DP_APTX_TWS_CIE aptx_tws_cie;

  // Check whether the codec info contains valid data
  tA2DP_STATUS a2dp_status =
      A2DP_ParseInfoAptxTWS(&aptx_tws_cie, p_codec_info, false);
  if (a2dp_status != A2DP_SUCCESS) {
    LOG_ERROR(LOG_TAG, "%s: cannot decode codec information: %d", __func__,
              a2dp_status);
    return -1;
  }

  if (aptx_tws_cie.sampleRate == A2DP_APTX_TWS_SAMPLERATE_44100) return 44100;
  if (aptx_tws_cie.sampleRate == A2DP_APTX_TWS_SAMPLERATE_48000) return 48000;

  return -1;
}

int A2DP_VendorGetTrackBitsPerSampleAptxTWS(const uint8_t* p_codec_info) {
  tA2DP_APTX_TWS_CIE aptx_tws_cie;

  // Check whether the codec info contains valid data
  tA2DP_STATUS a2dp_status =
      A2DP_ParseInfoAptxTWS(&aptx_tws_cie, p_codec_info, false);
  if (a2dp_status != A2DP_SUCCESS) {
    LOG_ERROR(LOG_TAG, "%s: cannot decode codec information: %d", __func__,
              a2dp_status);
    return -1;
  }

  return 24;  // For aptX-TWS we always use 24 bits per audio sample
}

int A2DP_VendorGetTrackChannelCountAptxTWS(const uint8_t* p_codec_info) {
  tA2DP_APTX_TWS_CIE aptx_tws_cie;

  // Check whether the codec info contains valid data
  tA2DP_STATUS a2dp_status =
      A2DP_ParseInfoAptxTWS(&aptx_tws_cie, p_codec_info, false);
  if (a2dp_status != A2DP_SUCCESS) {
    LOG_ERROR(LOG_TAG, "%s: cannot decode codec information: %d", __func__,
              a2dp_status);
    return -1;
  }

  switch (aptx_tws_cie.channelMode) {
    case A2DP_APTX_TWS_CHANNELS_MONO:
    //case A2DP_APTX_TWS_CHANNELS_STEREO:
      return 2;//aptX-TWS uses dual mono config, channel count is 2
  }

  return -1;
}

bool A2DP_VendorGetPacketTimestampAptxTWS(
    UNUSED_ATTR const uint8_t* p_codec_info, const uint8_t* p_data,
    uint32_t* p_timestamp) {
  // TODO: Is this function really codec-specific?
  *p_timestamp = *(const uint32_t*)p_data;
  return true;
}

bool A2DP_VendorBuildCodecHeaderAptxTWS(UNUSED_ATTR const uint8_t* p_codec_info,
                                       UNUSED_ATTR BT_HDR* p_buf,
                                       UNUSED_ATTR uint16_t frames_per_packet) {
  // Nothing to do
  return true;
}

void A2DP_VendorDumpCodecInfoAptxTWS(const uint8_t* p_codec_info) {
  tA2DP_STATUS a2dp_status;
  tA2DP_APTX_TWS_CIE aptx_tws_cie;

  LOG_DEBUG(LOG_TAG, "%s", __func__);

  a2dp_status = A2DP_ParseInfoAptxTWS(&aptx_tws_cie, p_codec_info, true);
  if (a2dp_status != A2DP_SUCCESS) {
    LOG_ERROR(LOG_TAG, "%s: A2DP_ParseInfoAptxTWS fail:%d", __func__,
              a2dp_status);
    return;
  }

  LOG_DEBUG(LOG_TAG, "\tsamp_freq: 0x%x", aptx_tws_cie.sampleRate);
  if (aptx_tws_cie.sampleRate & A2DP_APTX_TWS_SAMPLERATE_44100) {
    LOG_DEBUG(LOG_TAG, "\tsamp_freq: (44100)");
  }
  if (aptx_tws_cie.sampleRate & A2DP_APTX_TWS_SAMPLERATE_48000) {
    LOG_DEBUG(LOG_TAG, "\tsamp_freq: (48000)");
  }

  LOG_DEBUG(LOG_TAG, "\tch_mode: 0x%x", aptx_tws_cie.channelMode);
  if (aptx_tws_cie.channelMode & A2DP_APTX_TWS_CHANNELS_MONO) {
    LOG_DEBUG(LOG_TAG, "\tch_mode: (Mono)");
  }
//  if (aptx_tws_cie.channelMode & A2DP_APTX_TWS_CHANNELS_STEREO) {
//    LOG_DEBUG(LOG_TAG, "\tch_mode: (Stereo)");
// }
}

const tA2DP_ENCODER_INTERFACE* A2DP_VendorGetEncoderInterfaceAptxTWS(
    const uint8_t* p_codec_info) {
  if (!A2DP_IsVendorSourceCodecValidAptxTWS(p_codec_info)) return NULL;

  return &a2dp_encoder_interface_aptx_tws;
}

bool A2DP_VendorAdjustCodecAptxTWS(uint8_t* p_codec_info) {
  tA2DP_APTX_TWS_CIE cfg_cie;

  // Nothing to do: just verify the codec info is valid
  if (A2DP_ParseInfoAptxTWS(&cfg_cie, p_codec_info, true) != A2DP_SUCCESS)
    return false;

  return true;
}

btav_a2dp_codec_index_t A2DP_VendorSourceCodecIndexAptxTWS(
    const uint8_t* p_codec_info) {
  return (btav_a2dp_codec_index_t)BTAV_A2DP_CODEC_INDEX_SOURCE_APTX_TWS;
}

const char* A2DP_VendorCodecIndexStrAptxTWS(void) { return "aptX-TWS"; }

bool A2DP_VendorInitCodecConfigAptxTWS(tAVDT_CFG* p_cfg) {
  if (!A2DP_IsCodecEnabled((btav_a2dp_codec_index_t)
                                  BTAV_A2DP_CODEC_INDEX_SOURCE_APTX_TWS)) {
    LOG_ERROR(LOG_TAG, "%s: APTX-TWS disabled in both SW and HW mode", __func__);
    return false;
  }

  if (A2DP_BuildInfoAptxTWS(AVDT_MEDIA_TYPE_AUDIO, &a2dp_aptx_tws_caps,
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

A2dpCodecConfigAptxTWS::A2dpCodecConfigAptxTWS(
    btav_a2dp_codec_priority_t codec_priority)
    : A2dpCodecConfig((btav_a2dp_codec_index_t)BTAV_A2DP_CODEC_INDEX_SOURCE_APTX_TWS,
                      "aptX-TWS", codec_priority) {
  // Compute the local capability
    if (A2DP_IsCodecEnabledInOffload((btav_a2dp_codec_index_t)
                                  BTAV_A2DP_CODEC_INDEX_SOURCE_APTX_TWS)) {
      a2dp_aptx_tws_caps = a2dp_aptx_tws_offload_caps;
      a2dp_aptx_tws_default_config = a2dp_aptx_tws_default_offload_config;
    } else {
      LOG_ERROR(LOG_TAG,"aptX-TWS supported only in offload mode");
//      a2dp_aptx_tws_caps = a2dp_aptx_tws_src_caps;
//      a2dp_aptx_tws_default_config = a2dp_aptx_tws_default_src_config;
      return;
    }
  if (a2dp_aptx_tws_caps.sampleRate & A2DP_APTX_TWS_SAMPLERATE_44100) {
    codec_local_capability_.sample_rate |= BTAV_A2DP_CODEC_SAMPLE_RATE_44100;
  }
  if (a2dp_aptx_tws_caps.sampleRate & A2DP_APTX_TWS_SAMPLERATE_48000) {
    codec_local_capability_.sample_rate |= BTAV_A2DP_CODEC_SAMPLE_RATE_48000;
  }
  codec_local_capability_.bits_per_sample = a2dp_aptx_tws_caps.bits_per_sample;
  if (a2dp_aptx_tws_caps.channelMode & A2DP_APTX_TWS_CHANNELS_MONO) {
    codec_local_capability_.channel_mode |= BTAV_A2DP_CODEC_CHANNEL_MODE_MONO;
  }
}

A2dpCodecConfigAptxTWS::~A2dpCodecConfigAptxTWS() {}

bool A2dpCodecConfigAptxTWS::init() {
  if (!isValid()) return false;

  if (A2DP_IsCodecEnabledInOffload((btav_a2dp_codec_index_t)
                                 BTAV_A2DP_CODEC_INDEX_SOURCE_APTX_TWS)) {
    LOG_ERROR(LOG_TAG, "%s: APTX-Tws enabled in HW mode", __func__);
    return true;
  } else if(!A2DP_IsCodecEnabledInSoftware((btav_a2dp_codec_index_t)
                                 BTAV_A2DP_CODEC_INDEX_SOURCE_APTX_TWS)){
    LOG_ERROR(LOG_TAG, "%s: APTX-Tws disabled in both SW and HW mode", __func__);
    return false;
  }
  return true;
}

bool A2dpCodecConfigAptxTWS::useRtpHeaderMarkerBit() const { return false; }

//
// Selects the best sample rate from |sampleRate|.
// The result is stored in |p_result| and p_codec_config|.
// Returns true if a selection was made, otherwise false.
//
static bool select_best_sample_rate(uint8_t sampleRate,
                                    tA2DP_APTX_TWS_CIE* p_result,
                                    btav_a2dp_codec_config_t* p_codec_config) {
  if (sampleRate & A2DP_APTX_TWS_SAMPLERATE_48000) {
    p_result->sampleRate = A2DP_APTX_TWS_SAMPLERATE_48000;
    p_codec_config->sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_48000;
    return true;
  }
  if (sampleRate & A2DP_APTX_TWS_SAMPLERATE_44100) {
    p_result->sampleRate = A2DP_APTX_TWS_SAMPLERATE_44100;
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
    tA2DP_APTX_TWS_CIE* p_result, btav_a2dp_codec_config_t* p_codec_config) {
  switch (p_codec_audio_config->sample_rate) {
    case BTAV_A2DP_CODEC_SAMPLE_RATE_44100:
      if (sampleRate & A2DP_APTX_TWS_SAMPLERATE_44100) {
        p_result->sampleRate = A2DP_APTX_TWS_SAMPLERATE_44100;
        p_codec_config->sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_44100;
        return true;
      }
      break;
    case BTAV_A2DP_CODEC_SAMPLE_RATE_48000:
      if (sampleRate & A2DP_APTX_TWS_SAMPLERATE_48000) {
        p_result->sampleRate = A2DP_APTX_TWS_SAMPLERATE_48000;
        p_codec_config->sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_48000;
        return true;
      }
      break;
    case BTAV_A2DP_CODEC_SAMPLE_RATE_88200:
    case BTAV_A2DP_CODEC_SAMPLE_RATE_96000:
    case BTAV_A2DP_CODEC_SAMPLE_RATE_176400:
    case BTAV_A2DP_CODEC_SAMPLE_RATE_192000:
    case BTAV_A2DP_CODEC_SAMPLE_RATE_NONE:
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
                                     tA2DP_APTX_TWS_CIE* p_result,
                                     btav_a2dp_codec_config_t* p_codec_config) {
  /*if (channelMode & A2DP_APTX_TWS_CHANNELS_STEREO) {
    p_result->channelMode = A2DP_APTX_TWS_CHANNELS_STEREO;
    p_codec_config->channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO;
    return true;
  }*/
  if (channelMode & A2DP_APTX_TWS_CHANNELS_MONO) {
    p_result->channelMode = A2DP_APTX_TWS_CHANNELS_MONO;
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
    tA2DP_APTX_TWS_CIE* p_result, btav_a2dp_codec_config_t* p_codec_config) {
  switch (p_codec_audio_config->channel_mode) {
    case BTAV_A2DP_CODEC_CHANNEL_MODE_MONO:
      if (channelMode & A2DP_APTX_TWS_CHANNELS_MONO) {
        p_result->channelMode = A2DP_APTX_TWS_CHANNELS_MONO;
        p_codec_config->channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_MONO;
        return true;
      }
      break;
    case BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO:
    case BTAV_A2DP_CODEC_CHANNEL_MODE_NONE:
      break;
  }

  return false;
}

bool A2dpCodecConfigAptxTWS::setCodecConfig(const uint8_t* p_peer_codec_info,
                                           bool is_capability,
                                           uint8_t* p_result_codec_config) {
  std::lock_guard<std::recursive_mutex> lock(codec_mutex_);
  tA2DP_APTX_TWS_CIE sink_info_cie;
  tA2DP_APTX_TWS_CIE result_config_cie;
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
      A2DP_ParseInfoAptxTWS(&sink_info_cie, p_peer_codec_info, is_capability);
  if (status != A2DP_SUCCESS) {
    LOG_ERROR(LOG_TAG, "%s: can't parse peer's Sink capabilities: error = %d",
              __func__, status);
    goto fail;
  }

  //
  // Build the preferred configuration
  //
  memset(&result_config_cie, 0, sizeof(result_config_cie));
  result_config_cie.vendorId = a2dp_aptx_tws_caps.vendorId;
  result_config_cie.codecId = a2dp_aptx_tws_caps.codecId;

  //
  // Select the sample frequency
  //
  sampleRate = a2dp_aptx_tws_caps.sampleRate & sink_info_cie.sampleRate;
  codec_config_.sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_NONE;
  switch (codec_user_config_.sample_rate) {
    case BTAV_A2DP_CODEC_SAMPLE_RATE_44100:
      if (sampleRate & A2DP_APTX_TWS_SAMPLERATE_44100) {
        result_config_cie.sampleRate = A2DP_APTX_TWS_SAMPLERATE_44100;
        codec_capability_.sample_rate = codec_user_config_.sample_rate;
        codec_config_.sample_rate = codec_user_config_.sample_rate;
      }
      break;
    case BTAV_A2DP_CODEC_SAMPLE_RATE_48000:
      if (sampleRate & A2DP_APTX_TWS_SAMPLERATE_48000) {
        result_config_cie.sampleRate = A2DP_APTX_TWS_SAMPLERATE_48000;
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
  }

  // Select the sample frequency if there is no user preference
  do {
    // Compute the selectable capability
    if (sampleRate & A2DP_APTX_TWS_SAMPLERATE_44100) {
      codec_selectable_capability_.sample_rate |=
          BTAV_A2DP_CODEC_SAMPLE_RATE_44100;
    }
    if (sampleRate & A2DP_APTX_TWS_SAMPLERATE_48000) {
      codec_selectable_capability_.sample_rate |=
          BTAV_A2DP_CODEC_SAMPLE_RATE_48000;
    }

    if (codec_config_.sample_rate != BTAV_A2DP_CODEC_SAMPLE_RATE_NONE) break;

    // Compute the common capability
    if (sampleRate & A2DP_APTX_TWS_SAMPLERATE_44100)
      codec_capability_.sample_rate |= BTAV_A2DP_CODEC_SAMPLE_RATE_44100;
    if (sampleRate & A2DP_APTX_TWS_SAMPLERATE_48000)
      codec_capability_.sample_rate |= BTAV_A2DP_CODEC_SAMPLE_RATE_48000;

    // No user preference - try the codec audio config
    if (select_audio_sample_rate(&codec_audio_config_, sampleRate,
                                 &result_config_cie, &codec_config_)) {
      break;
    }

    // No user preference - try the default config
    if (select_best_sample_rate(
            a2dp_aptx_tws_default_config.sampleRate & sink_info_cie.sampleRate,
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
              __func__, a2dp_aptx_tws_caps.sampleRate, sink_info_cie.sampleRate);
    goto fail;
  }

  //
  // Select the bits per sample
  //
  // NOTE: this information is NOT included in the aptX-TWS A2DP codec
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
        a2dp_aptx_tws_caps.bits_per_sample;

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
  channelMode = a2dp_aptx_tws_caps.channelMode & sink_info_cie.channelMode;
  codec_config_.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_NONE;
  switch (codec_user_config_.channel_mode) {
    case BTAV_A2DP_CODEC_CHANNEL_MODE_MONO:
      if (channelMode & A2DP_APTX_TWS_CHANNELS_MONO) {
        result_config_cie.channelMode = A2DP_APTX_TWS_CHANNELS_MONO;
        codec_capability_.channel_mode = codec_user_config_.channel_mode;
        codec_config_.channel_mode = codec_user_config_.channel_mode;
      }
      break;
    case BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO:
    case BTAV_A2DP_CODEC_CHANNEL_MODE_NONE:
      codec_capability_.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_NONE;
      codec_config_.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_NONE;
      break;
  }

  // Select the channel mode if there is no user preference
  do {
    // Compute the selectable capability
    if (channelMode & A2DP_APTX_TWS_CHANNELS_MONO) {
      codec_selectable_capability_.channel_mode |=
          BTAV_A2DP_CODEC_CHANNEL_MODE_MONO;
    }
    /*if (channelMode & A2DP_APTX_TWS_CHANNELS_STEREO) {
      codec_selectable_capability_.channel_mode |=
          BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO;
    }*/

    if (codec_config_.channel_mode != BTAV_A2DP_CODEC_CHANNEL_MODE_NONE) break;

    // Compute the common capability
    if (channelMode & A2DP_APTX_TWS_CHANNELS_MONO)
      codec_capability_.channel_mode |= BTAV_A2DP_CODEC_CHANNEL_MODE_MONO;
    //if (channelMode & A2DP_APTX_TWS_CHANNELS_STEREO)
    //  codec_capability_.channel_mode |= BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO;

    // No user preference - try the codec audio config
    if (select_audio_channel_mode(&codec_audio_config_, channelMode,
                                  &result_config_cie, &codec_config_)) {
      break;
    }

    // No user preference - try the default config
    if (select_best_channel_mode(
            a2dp_aptx_tws_default_config.channelMode & sink_info_cie.channelMode,
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
              __func__, a2dp_aptx_tws_caps.channelMode,
              sink_info_cie.channelMode);
    goto fail;
  }

  //
  // Set the rest of the fields as bit-wise AND operation
  //

  if (A2DP_BuildInfoAptxTWS(AVDT_MEDIA_TYPE_AUDIO, &result_config_cie,
                           p_result_codec_config) != A2DP_SUCCESS) {
    goto fail;
  }

  // Create a local copy of the peer codec capability/config, and the
  // result codec config.
  if (is_capability) {
    status = A2DP_BuildInfoAptxTWS(AVDT_MEDIA_TYPE_AUDIO, &sink_info_cie,
                                  ota_codec_peer_capability_);
  } else {
    status = A2DP_BuildInfoAptxTWS(AVDT_MEDIA_TYPE_AUDIO, &sink_info_cie,
                                  ota_codec_peer_config_);
  }
  CHECK(status == A2DP_SUCCESS);
  status = A2DP_BuildInfoAptxTWS(AVDT_MEDIA_TYPE_AUDIO, &result_config_cie,
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
#endif //TWS_ENABLED
