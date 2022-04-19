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

//
// A2DP Codec API for aptX-TWS
//

#ifndef A2DP_VENDOR_APTX_TWS_H
#define A2DP_VENDOR_APTX_TWS_H

#include "a2dp_codec_api.h"
#include "a2dp_vendor_aptx_tws_constants.h"
#include "avdt_api.h"
#include <hardware/bt_av.h>
class A2dpCodecConfigAptxTWS : public A2dpCodecConfig {
 public:
  A2dpCodecConfigAptxTWS(btav_a2dp_codec_priority_t codec_priority);
  virtual ~A2dpCodecConfigAptxTWS();

  bool init() override;
  period_ms_t encoderIntervalMs() const override;
  bool setCodecConfig(const uint8_t* p_peer_codec_info, bool is_capability,
                      uint8_t* p_result_codec_config) override;

 private:
  bool useRtpHeaderMarkerBit() const override;
  bool updateEncoderUserConfig(
      const tA2DP_ENCODER_INIT_PEER_PARAMS* p_peer_params,
      bool* p_restart_input, bool* p_restart_output,
      bool* p_config_updated) override;
  void debug_codec_dump(int fd) override;
};

// Checks whether the codec capabilities contain a valid A2DP aptX-TWS Source
// codec.
// NOTE: only codecs that are implemented are considered valid.
// Returns true if |p_codec_info| contains information about a valid aptX-TWS
// codec, otherwise false.
bool A2DP_IsVendorSourceCodecValidAptxTWS(const uint8_t* p_codec_info);

// Checks whether the codec capabilities contain a valid peer A2DP aptX-TWS Sink
// codec.
// NOTE: only codecs that are implemented are considered valid.
// Returns true if |p_codec_info| contains information about a valid aptX-TWS
// codec, otherwise false.
bool A2DP_IsVendorPeerSinkCodecValidAptxTWS(const uint8_t* p_codec_info);

// Checks whether the A2DP data packets should contain RTP header.
// |content_protection_enabled| is true if Content Protection is
// enabled. |p_codec_info| contains information about the codec capabilities.
// Returns true if the A2DP data packets should contain RTP header, otherwise
// false.
bool A2DP_VendorUsesRtpHeaderAptxTWS(bool content_protection_enabled,
                                    const uint8_t* p_codec_info);

// Gets the A2DP aptX-TWS codec name for a given |p_codec_info|.
const char* A2DP_VendorCodecNameAptxTWS(const uint8_t* p_codec_info);

// Checks whether two A2DP aptX-TWS codecs |p_codec_info_a| and |p_codec_info_b|
// have the same type.
// Returns true if the two codecs have the same type, otherwise false.
bool A2DP_VendorCodecTypeEqualsAptxTWS(const uint8_t* p_codec_info_a,
                                      const uint8_t* p_codec_info_b);

// Checks whether two A2DP aptX-TWS codecs |p_codec_info_a| and |p_codec_info_b|
// are exactly the same.
// Returns true if the two codecs are exactly the same, otherwise false.
// If the codec type is not aptX-TWS, the return value is false.
bool A2DP_VendorCodecEqualsAptxTWS(const uint8_t* p_codec_info_a,
                                  const uint8_t* p_codec_info_b);

// Gets the track sample rate value for the A2DP aptX-TWS codec.
// |p_codec_info| is a pointer to the aptX-TWS codec_info to decode.
// Returns the track sample rate on success, or -1 if |p_codec_info|
// contains invalid codec information.
int A2DP_VendorGetTrackSampleRateAptxTWS(const uint8_t* p_codec_info);

// Gets the bits per audio sample for the A2DP aptX-TWS codec.
// |p_codec_info| is a pointer to the aptX-TWS codec_info to decode.
// Returns the bits per audio sample on success, or -1 if |p_codec_info|
// contains invalid codec information.
int A2DP_VendorGetTrackBitsPerSampleAptxTWS(const uint8_t* p_codec_info);

// Gets the channel count for the A2DP aptX-TWS codec.
// |p_codec_info| is a pointer to the aptX-TWS codec_info to decode.
// Returns the channel count on success, or -1 if |p_codec_info|
// contains invalid codec information.
int A2DP_VendorGetTrackChannelCountAptxTWS(const uint8_t* p_codec_info);

// Gets the A2DP aptX-TWS audio data timestamp from an audio packet.
// |p_codec_info| contains the codec information.
// |p_data| contains the audio data.
// The timestamp is stored in |p_timestamp|.
// Returns true on success, otherwise false.
bool A2DP_VendorGetPacketTimestampAptxTWS(const uint8_t* p_codec_info,
                                         const uint8_t* p_data,
                                         uint32_t* p_timestamp);

// Builds A2DP aptX-TWS codec header for audio data.
// |p_codec_info| contains the codec information.
// |p_buf| contains the audio data.
// |frames_per_packet| is the number of frames in this packet.
// Returns true on success, otherwise false.
bool A2DP_VendorBuildCodecHeaderAptxTWS(const uint8_t* p_codec_info,
                                       BT_HDR* p_buf,
                                       uint16_t frames_per_packet);

// Decodes and displays aptX-TWS codec info (for debugging).
// |p_codec_info| is a pointer to the aptX-TWS codec_info to decode and display.
void A2DP_VendorDumpCodecInfoAptxTWS(const uint8_t* p_codec_info);

// Gets the A2DP aptX-TWS encoder interface that can be used to encode and
// prepare A2DP packets for transmission - see |tA2DP_ENCODER_INTERFACE|.
// |p_codec_info| contains the codec information.
// Returns the A2DP aptX-TWS encoder interface if the |p_codec_info| is valid
// and supported, otherwise NULL.
const tA2DP_ENCODER_INTERFACE* A2DP_VendorGetEncoderInterfaceAptxTWS(
    const uint8_t* p_codec_info);

// Adjusts the A2DP aptX-TWS codec, based on local support and Bluetooth
// specification.
// |p_codec_info| contains the codec information to adjust.
// Returns true if |p_codec_info| is valid and supported, otherwise false.
bool A2DP_VendorAdjustCodecAptxTWS(uint8_t* p_codec_info);

// Gets the A2DP aptX-TWS Source codec index for a given |p_codec_info|.
// Returns the corresponding |btav_a2dp_codec_index_t| on success,
// otherwise |BTAV_A2DP_CODEC_INDEX_MAX|.
btav_a2dp_codec_index_t A2DP_VendorSourceCodecIndexAptxTWS(
    const uint8_t* p_codec_info);

// Gets the A2DP aptX-TWS Source codec name.
const char* A2DP_VendorCodecIndexStrAptxTWS(void);

// Initializes A2DP aptX-TWS Source codec information into |tAVDT_CFG|
// configuration entry pointed by |p_cfg|.
bool A2DP_VendorInitCodecConfigAptxTWS(tAVDT_CFG* p_cfg);

#endif  // A2DP_VENDOR_APTX_TWS_H
