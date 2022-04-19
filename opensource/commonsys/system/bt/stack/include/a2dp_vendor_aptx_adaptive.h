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

//
// A2DP Codec API for aptX-adaptive
//

#ifndef A2DP_VENDOR_APTX_ADAPTIVE_H
#define A2DP_VENDOR_APTX_ADAPTIVE_H

#include "a2dp_codec_api.h"
#include "a2dp_vendor_aptx_adaptive_constants.h"
#include "avdt_api.h"

class A2dpCodecConfigAptxAdaptive: public A2dpCodecConfig {
 public:
  A2dpCodecConfigAptxAdaptive(btav_a2dp_codec_priority_t codec_priority);
  virtual ~A2dpCodecConfigAptxAdaptive();

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

  btav_a2dp_codec_config_t previous_codec_config;

  void debug_codec_dump(int fd) override;
};

// data type for the aptX-adaptive Codec Information Element */

typedef struct {
  uint8_t ttp_ll_0;
  uint8_t ttp_ll_1;
  uint8_t ttp_hq_0;
  uint8_t ttp_hq_1;
  uint8_t ttp_tws_0;
  uint8_t ttp_tws_1;
  uint8_t reserved_15thbyte;

  //Below are the additions for R2
  uint8_t cap_ext_ver_num;             //capability extension ver number
  uint32_t aptx_adaptive_sup_features; //aptx-adaptive supported features
  uint8_t first_setup_pref;            //first setup preference
  uint8_t second_setup_pref;           //second setup preference
  uint8_t third_setup_pref;            //third setup preference
  uint8_t fourth_setup_pref;           //fourth setup preference
  uint8_t eoc0;                        //no further expansion
  uint8_t eoc1;                        //capability extension end
} tA2DP_APTX_ADAPTIVE_VENDOR_DATA;
// This is due to be updated
typedef struct {
  uint32_t vendorId;
  uint16_t codecId;    /* Codec ID for aptX-adaptive */
  uint8_t sampleRate;  /* Sampling Frequency */
  uint8_t sourceType;  /* AVRCP CUSTOM, Q2Q OR NQ2Q, RESERVED */
  uint8_t channelMode;
  tA2DP_APTX_ADAPTIVE_VENDOR_DATA aptx_data;
  btav_a2dp_codec_bits_per_sample_t bits_per_sample;
  uint8_t reserved_data[A2DP_APTX_ADAPTIVE_RESERVED_DATA];  // reserved for future exp
} tA2DP_APTX_ADAPTIVE_CIE;
// parses the given codec info and copies the needed info
// to Codec Information Element and returns same CIE.
bool A2DP_GetAptxAdaptiveCIE(const uint8_t* p_codec_info,
                        tA2DP_APTX_ADAPTIVE_CIE *cfg_cie);
// Checks whether the codec capabilities contain a valid A2DP aptX-adaptive Source
// codec.
// NOTE: only codecs that are implemented are considered valid.
// Returns true if |p_codec_info| contains information about a valid aptX-adaptive
// codec, otherwise false.
bool A2DP_IsVendorSourceCodecValidAptxAdaptive(const uint8_t* p_codec_info);

// Checks whether the codec capabilities contain a valid peer A2DP aptX-adaptive Sink
// codec.
// NOTE: only codecs that are implemented are considered valid.
// Returns true if |p_codec_info| contains information about a valid aptX-adaptive
// codec, otherwise false.
bool A2DP_IsVendorPeerSinkCodecValidAptxAdaptive(const uint8_t* p_codec_info);

// Checks whether the A2DP data packets should contain RTP header.
// |content_protection_enabled| is true if Content Protection is
// enabled. |p_codec_info| contains information about the codec capabilities.
// Returns true if the A2DP data packets should contain RTP header, otherwise
// false.
bool A2DP_VendorUsesRtpHeaderAptxAdaptive(bool content_protection_enabled,
                                    const uint8_t* p_codec_info);

// Gets the A2DP aptX-adaptive codec name for a given |p_codec_info|.
const char* A2DP_VendorCodecNameAptxAdaptive(const uint8_t* p_codec_info);

// Checks whether two A2DP aptX-adaptive codecs |p_codec_info_a| and |p_codec_info_b|
// have the same type.
// Returns true if the two codecs have the same type, otherwise false.
bool A2DP_VendorCodecTypeEqualsAptxAdaptive(const uint8_t* p_codec_info_a,
                                      const uint8_t* p_codec_info_b);

// Checks whether two A2DP aptX-adaptive codecs |p_codec_info_a| and |p_codec_info_b|
// are exactly the same.
// Returns true if the two codecs are exactly the same, otherwise false.
// If the codec type is not aptX-adaptive, the return value is false.
bool A2DP_VendorCodecEqualsAptxAdaptive(const uint8_t* p_codec_info_a,
                                  const uint8_t* p_codec_info_b);

// Gets the track sample rate value for the A2DP aptX-adaptive codec.
// |p_codec_info| is a pointer to the aptX-adaptive codec_info to decode.
// Returns the track sample rate on success, or -1 if |p_codec_info|
// contains invalid codec information.
int A2DP_VendorGetTrackSampleRateAptxAdaptive(const uint8_t* p_codec_info);

// Gets the bits per audio sample for the A2DP aptX-adaptive codec.
// |p_codec_info| is a pointer to the aptX-adaptive codec_info to decode.
// Returns the bits per audio sample on success, or -1 if |p_codec_info|
// contains invalid codec information.
int A2DP_VendorGetTrackBitsPerSampleAptxAdaptive(const uint8_t* p_codec_info);

// Gets the channel count for the A2DP aptX-adaptive codec.
// |p_codec_info| is a pointer to the aptX-adaptive codec_info to decode.
// Returns the channel count on success, or -1 if |p_codec_info|
// contains invalid codec information.
int A2DP_VendorGetTrackChannelCountAptxAdaptive(const uint8_t* p_codec_info);

// Gets the A2DP aptX-adaptive audio data timestamp from an audio packet.
// |p_codec_info| contains the codec information.
// |p_data| contains the audio data.
// The timestamp is stored in |p_timestamp|.
// Returns true on success, otherwise false.
bool A2DP_VendorGetPacketTimestampAptxAdaptive(const uint8_t* p_codec_info,
                                         const uint8_t* p_data,
                                         uint32_t* p_timestamp);

// Builds A2DP aptX-adaptive codec header for audio data.
// |p_codec_info| contains the codec information.
// |p_buf| contains the audio data.
// |frames_per_packet| is the number of frames in this packet.
// Returns true on success, otherwise false.
bool A2DP_VendorBuildCodecHeaderAptxAdaptive(const uint8_t* p_codec_info,
                                       BT_HDR* p_buf,
                                       uint16_t frames_per_packet);

// Decodes and displays aptX-adaptive codec info (for debugging).
// |p_codec_info| is a pointer to the aptX-adaptive codec_info to decode and display.
bool A2DP_VendorDumpCodecInfoAptxAdaptive(const uint8_t* p_codec_info);

// Gets the A2DP aptX-adaptive encoder interface that can be used to encode and
// prepare A2DP packets for transmission - see |tA2DP_ENCODER_INTERFACE|.
// |p_codec_info| contains the codec information.
// Returns the A2DP aptX-adaptive encoder interface if the |p_codec_info| is valid
// and supported, otherwise NULL.
const tA2DP_ENCODER_INTERFACE* A2DP_VendorGetEncoderInterfaceAptxAdaptive(
    const uint8_t* p_codec_info);

// Adjusts the A2DP aptX-adaptive codec, based on local support and Bluetooth
// specification.
// |p_codec_info| contains the codec information to adjust.
// Returns true if |p_codec_info| is valid and supported, otherwise false.
bool A2DP_VendorAdjustCodecAptxAdaptive(uint8_t* p_codec_info);

// Gets the A2DP aptX-adaptive Source codec index for a given |p_codec_info|.
// Returns the corresponding |btav_a2dp_codec_index_t| on success,
// otherwise |BTAV_A2DP_CODEC_INDEX_MAX|.
btav_a2dp_codec_index_t A2DP_VendorSourceCodecIndexAptxAdaptive(
    const uint8_t* p_codec_info);

// Gets the A2DP aptX-adaptive Source codec name.
const char* A2DP_VendorCodecIndexStrAptxAdaptive(void);

// Initializes A2DP aptX-adaptive Source codec information into |tAVDT_CFG|
// configuration entry pointed by |p_cfg|.
bool A2DP_VendorInitCodecConfigAptxAdaptive(tAVDT_CFG* p_cfg);

#endif  // A2DP_VENDOR_APTX_ADAPTIVE_H
