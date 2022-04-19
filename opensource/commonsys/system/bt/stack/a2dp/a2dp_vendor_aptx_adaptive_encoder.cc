/******************************************************************************
    Copyright (c) 2018, The Linux Foundation. All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted (subject to the limitations in the
    disclaimer below) provided that the following conditions are met:

       * Redistributions of source code must retain the above copyright
         notice, this list of conditions and the following disclaimer.

       * Redistributions in binary form must reproduce the above
         copyright notice, this list of conditions and the following
         disclaimer in the documentation and/or other materials provided
         with the distribution.

       * Neither the name of The Linux Foundation nor the names of its
         contributors may be used to endorse or promote products derived
         from this software without specific prior written permission.

    NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
    GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
    HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
    WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
    IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
    ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
    IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
    OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
    IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

#define LOG_TAG "a2dp_vendor_aptx_adaptive_encoder"

#include "a2dp_vendor_aptx_adaptive_encoder.h"

#include <dlfcn.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <cmath>

#include "a2dp_vendor.h"
#include "a2dp_vendor_aptx_adaptive.h"
#include "bt_common.h"
#include "osi/include/log.h"

#include <time.h>

typedef struct {
  uint64_t session_start_us;

  size_t media_read_total_expected_packets;
  size_t media_read_total_expected_reads_count;
  size_t media_read_total_expected_read_bytes;

  size_t media_read_total_dropped_packets;
  size_t media_read_total_actual_reads_count;
  size_t media_read_total_actual_read_bytes;
} a2dp_aptx_adaptive_encoder_stats_t;



typedef struct {
  a2dp_source_read_callback_t read_callback;
  a2dp_source_enqueue_callback_t enqueue_callback;

  bool use_SCMS_T;
  bool is_peer_edr;          // True if the peer device supports EDR
  bool peer_supports_3mbps;  // True if the peer device supports 3Mbps EDR
  uint16_t peer_mtu;         // // MTU of the A2DP peer
  uint32_t timestamp;        // Timestamp for the A2DP frames

  a2dp_aptx_adaptive_encoder_stats_t stats;

} tA2DP_APTX_ADAPTIVE_ENCODER_CB;

static tA2DP_APTX_ADAPTIVE_ENCODER_CB a2dp_aptx_adaptive_encoder_cb;


bool A2DP_VendorLoadEncoderAptxAdaptive(void) {
  if (A2DP_IsCodecEnabledInOffload(BTAV_A2DP_CODEC_INDEX_SOURCE_APTX_ADAPTIVE)) {
    LOG_INFO(LOG_TAG,"aptX-Adaptive is running in offload mode");
    return true;
  }

  return true;
}

void A2DP_VendorUnloadEncoderAptxAdaptive(void) {
  if (A2DP_IsCodecEnabledInOffload(BTAV_A2DP_CODEC_INDEX_SOURCE_APTX_ADAPTIVE)) {
    LOG_INFO(LOG_TAG,"aptX-Adaptive is running in offload mode");
    return;
  }
}

void a2dp_vendor_aptx_adaptive_encoder_init(
    const tA2DP_ENCODER_INIT_PEER_PARAMS* p_peer_params,
    A2dpCodecConfig* a2dp_codec_config,
    a2dp_source_read_callback_t read_callback,
    a2dp_source_enqueue_callback_t enqueue_callback) {
  if (A2DP_IsCodecEnabledInOffload(BTAV_A2DP_CODEC_INDEX_SOURCE_APTX_ADAPTIVE)) {
    LOG_INFO(LOG_TAG,"aptX-Adaptive is running in offload mode");
    return;
  }
}



bool A2dpCodecConfigAptxAdaptive::updateEncoderUserConfig(
    const tA2DP_ENCODER_INIT_PEER_PARAMS* p_peer_params, bool* p_restart_input,
    bool* p_restart_output, bool* p_config_updated) {

  btav_a2dp_codec_config_t codec_config = getCodecConfig();

  a2dp_aptx_adaptive_encoder_cb.is_peer_edr = p_peer_params->is_peer_edr;
  a2dp_aptx_adaptive_encoder_cb.peer_supports_3mbps =
      p_peer_params->peer_supports_3mbps;
  a2dp_aptx_adaptive_encoder_cb.peer_mtu = p_peer_params->peer_mtu;

  if (a2dp_aptx_adaptive_encoder_cb.peer_mtu == 0) {
    LOG_ERROR(LOG_TAG,
              "%s: Cannot update the codec encoder for %s: "
              "invalid peer MTU",
              __func__, name().c_str());
    return false;
  }


  if (!isCodecConfigEmpty(previous_codec_config)) {
     if (previous_codec_config.codec_priority == codec_config.codec_priority &&
         previous_codec_config.sample_rate == codec_config.sample_rate &&
         previous_codec_config.bits_per_sample == codec_config.bits_per_sample &&
         previous_codec_config.channel_mode == codec_config.channel_mode) {

       if (previous_codec_config.codec_specific_4 != codec_config.codec_specific_4 ) {
         // this is just our private config that has changed, no need to send
         // a BTIF_AV_SOURCE_CONFIG_UPDATED_EVT, which restarts split audio
         *p_config_updated = false;
       }
     }
  }

  previous_codec_config = codec_config;

  return true;
}


void a2dp_vendor_aptx_adaptive_feeding_reset(void) {
  if (A2DP_IsCodecEnabledInOffload(BTAV_A2DP_CODEC_INDEX_SOURCE_APTX_ADAPTIVE)) {
    LOG_INFO(LOG_TAG,"a2dp_vendor_aptx_adaptive_send_frames"
                                  "aptX-Adaptive is running in offload mode");
    return;
  }
}


void a2dp_vendor_aptx_adaptive_feeding_flush(void) {
  if (A2DP_IsCodecEnabledInOffload(BTAV_A2DP_CODEC_INDEX_SOURCE_APTX_ADAPTIVE)) {
    LOG_INFO(LOG_TAG,"a2dp_vendor_aptx_adaptive_send_frames"
                                  "aptX-Adaptive is running in offload mode");
    return;
  }
}

period_ms_t a2dp_vendor_aptx_adaptive_get_encoder_interval_ms(void) {
  return 0;
}


void a2dp_vendor_aptx_adaptive_send_frames(uint64_t timestamp_us) {
  if (A2DP_IsCodecEnabledInOffload(BTAV_A2DP_CODEC_INDEX_SOURCE_APTX_ADAPTIVE)) {
    LOG_INFO(LOG_TAG,"a2dp_vendor_aptx_adaptive_send_frames"
        "aptX-Adaptive is running in offload mode");
    return;
  }
}

void a2dp_vendor_aptx_adaptive_encoder_cleanup(void) {
  //osi_free(a2dp_aptx_adaptive_encoder_cb.aptx_adaptive_encoder_state);
  memset(&a2dp_aptx_adaptive_encoder_cb, 0, sizeof(a2dp_aptx_adaptive_encoder_cb));
}


period_ms_t A2dpCodecConfigAptxAdaptive::encoderIntervalMs() const {
  return 0;
}


void A2dpCodecConfigAptxAdaptive::debug_codec_dump(int fd) {
  a2dp_aptx_adaptive_encoder_stats_t* stats = &a2dp_aptx_adaptive_encoder_cb.stats;

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

  /*dprintf(fd,
          "  Connection count                                        : %u\n",
		  a2dp_aptx_adaptive_encoder_cb.enc_params.connection_count);*/
}
