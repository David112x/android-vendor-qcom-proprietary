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

#define LOG_TAG "a2dp_vendor_aptx_tws_encoder"

#include "a2dp_vendor_aptx_tws_encoder.h"

#include <dlfcn.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "a2dp_vendor.h"
#include "a2dp_vendor_aptx_tws.h"
#include "bt_common.h"
#include "osi/include/log.h"
#include "osi/include/osi.h"

#if (TWS_ENABLED == TRUE)
//
// Encoder for aptX-TWS Source Codec
//

typedef int (*tAPTX_TWS_ENCODER_INIT)(void* state, short endian);

typedef int (*tAPTX_TWS_ENCODER_ENCODE_STEREO)(void* state, void* pcmL,
                                              void* pcmR, void* buffer);

typedef int (*tAPTX_TWS_ENCODER_SIZEOF_PARAMS)(void);

// offset
#if (BTA_AV_CO_CP_SCMS_T == TRUE)
#define A2DP_APTX_TWS_OFFSET (AVDT_MEDIA_OFFSET + 1)
#else
#define A2DP_APTX_TWS_OFFSET AVDT_MEDIA_OFFSET
#endif

#define A2DP_APTX_TWS_MAX_PCM_BYTES_PER_READ 1024

typedef struct {
  uint64_t sleep_time_ns;
  uint32_t pcm_reads;
  uint32_t pcm_bytes_per_read;
  uint32_t aptx_hd_bytes;
  uint32_t frame_size_counter;
} tAPTX_TWS_FRAMING_PARAMS;

typedef struct {
  uint64_t session_start_us;

  size_t media_read_total_expected_packets;
  size_t media_read_total_expected_reads_count;
  size_t media_read_total_expected_read_bytes;

  size_t media_read_total_dropped_packets;
  size_t media_read_total_actual_reads_count;
  size_t media_read_total_actual_read_bytes;
} a2dp_aptx_tws_encoder_stats_t;

typedef struct {
  a2dp_source_read_callback_t read_callback;
  a2dp_source_enqueue_callback_t enqueue_callback;

  bool use_SCMS_T;
  bool is_peer_edr;          // True if the peer device supports EDR
  bool peer_supports_3mbps;  // True if the peer device supports 3Mbps EDR
  uint16_t peer_mtu;         // // MTU of the A2DP peer
  uint32_t timestamp;        // Timestamp for the A2DP frames

  tA2DP_FEEDING_PARAMS feeding_params;
  tAPTX_TWS_FRAMING_PARAMS framing_params;
  void* aptx_tws_encoder_state;
  a2dp_aptx_tws_encoder_stats_t stats;
} tA2DP_APTX_TWS_ENCODER_CB;

static tA2DP_APTX_TWS_ENCODER_CB a2dp_aptx_tws_encoder_cb;

bool A2DP_VendorLoadEncoderAptxTWS(void) {
  LOG_ERROR(LOG_TAG,"A2DP_VendorLoadEncoderAptxTWS: Not supported in legacy a2dp");
  return false;
}

void A2DP_VendorUnloadEncoderAptxTWS(void) {
  LOG_ERROR(LOG_TAG,"A2DP_VendorUnloadEncoderAptxTWS: Not supported in legacy a2dp");
  return;
}

void a2dp_vendor_aptx_tws_encoder_init(
    const tA2DP_ENCODER_INIT_PEER_PARAMS* p_peer_params,
    A2dpCodecConfig* a2dp_codec_config,
    a2dp_source_read_callback_t read_callback,
    a2dp_source_enqueue_callback_t enqueue_callback) {
  if (A2DP_IsCodecEnabledInOffload((btav_a2dp_codec_index_t)
                                  BTAV_A2DP_CODEC_INDEX_SOURCE_APTX_TWS)) {
    LOG_INFO(LOG_TAG,"aptX-TWS is running in offload mode");
    return;
  }
}

bool A2dpCodecConfigAptxTWS::updateEncoderUserConfig(
    const tA2DP_ENCODER_INIT_PEER_PARAMS* p_peer_params, bool* p_restart_input,
    bool* p_restart_output, bool* p_config_updated) {
  LOG_ERROR(LOG_TAG,"updateEncoderUserConfig: Not surpporte in legacy mode");
  return true;
}

void a2dp_vendor_aptx_tws_encoder_cleanup(void) {
  LOG_ERROR(LOG_TAG,"a2dp_vendor_aptx_tws_encoder_cleanup: Not supported in legacy mode");
  return;
}

void a2dp_vendor_aptx_tws_feeding_reset(void) {
  LOG_ERROR(LOG_TAG,"a2dp_vendor_aptx_tws_feeding_reset: Not supported in legacy mode");
  return;

  if (A2DP_IsCodecEnabledInOffload((btav_a2dp_codec_index_t)
                                  BTAV_A2DP_CODEC_INDEX_SOURCE_APTX_TWS)) {
    LOG_INFO(LOG_TAG,"a2dp_vendor_aptx_tws_feeding_reset: aptx-TWS is in offload mode");
    return;
  } else {
   LOG_ERROR(LOG_TAG,"a2dp_vendor_aptx_tws_feeding_reset: Not supported in legacy mode");
   return;
  }
}

void a2dp_vendor_aptx_tws_feeding_flush(void) {
  if (A2DP_IsCodecEnabledInOffload((btav_a2dp_codec_index_t)
                                  BTAV_A2DP_CODEC_INDEX_SOURCE_APTX_TWS)) {
    LOG_INFO(LOG_TAG,"a2dp_vendor_aptx_tws_feeding_flush: aptx-TWS is in offload mode");
    return;
  } else {
    LOG_ERROR(LOG_TAG,"a2dp_vendor_aptx_hd_feeding_flush: Not supported in legacy mode");
    return;
  }
}

period_ms_t a2dp_vendor_aptx_tws_get_encoder_interval_ms(void) {
  if (A2DP_IsCodecEnabledInOffload((btav_a2dp_codec_index_t)
                                  BTAV_A2DP_CODEC_INDEX_SOURCE_APTX_TWS)) {
    LOG_INFO(LOG_TAG,"a2dp_vendor_aptx_tws_get_encoder_interval_ms:"
             "aptx-TWS is in offload mode");
    return 0;
  }
  return 0;
}

void a2dp_vendor_aptx_tws_send_frames(uint64_t timestamp_us) {
  if (A2DP_IsCodecEnabledInOffload((btav_a2dp_codec_index_t)
                                  BTAV_A2DP_CODEC_INDEX_SOURCE_APTX_TWS)) {
    LOG_INFO(LOG_TAG,"a2dp_vendor_aptx_hd_send_frames: aptx-HD is in offload mode");
    return;
  }
}

period_ms_t A2dpCodecConfigAptxTWS::encoderIntervalMs() const {
  return a2dp_vendor_aptx_tws_get_encoder_interval_ms();
}

void A2dpCodecConfigAptxTWS::debug_codec_dump(int fd) {
  a2dp_aptx_tws_encoder_stats_t* stats = &a2dp_aptx_tws_encoder_cb.stats;

  A2dpCodecConfig::debug_codec_dump(fd);

  dprintf(fd,
          "  Packet counts (expected/dropped)                        : %zu / "
          "%zu\n",
          stats->media_read_total_expected_packets,
          stats->media_read_total_dropped_packets);

  dprintf(fd,
          "  PCM read counts (expected/actual)                       : %zu / "
          "%zu\n",
          stats->media_read_total_expected_reads_count,
          stats->media_read_total_actual_reads_count);

  dprintf(fd,
          "  PCM read bytes (expected/actual)                        : %zu / "
          "%zu\n",
          stats->media_read_total_expected_read_bytes,
          stats->media_read_total_actual_read_bytes);
}
#endif //TWS_ENABLED
