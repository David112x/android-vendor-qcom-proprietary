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
// A2DP constants for aptX-adaptive codec
//

#ifndef A2DP_VENDOR_APTX_ADAPTIVE_CONSTANTS_H
#define A2DP_VENDOR_APTX_ADAPTIVE_CONSTANTS_H

/* aptX-adaptive codec specific settings */
#define A2DP_APTX_ADAPTIVE_CODEC_LEN 42
#define A2DP_APTX_ADAPTIVE_CODEC_FEATURE_LEN 26
#define A2DP_APTX_ADAPTIVE_RESERVED_DATA (A2DP_APTX_ADAPTIVE_CODEC_LEN \
        - (A2DP_APTX_ADAPTIVE_CODEC_FEATURE_LEN + 2))
// actual id's ..
#define A2DP_APTX_ADAPTIVE_VENDOR_ID               (0x000000D7)
#define A2DP_APTX_ADAPTIVE_CODEC_ID_BLUETOOTH      (0x00AD)

#define A2DP_APTX_ADAPTIVE_SAMPLERATE_44100        (0x08)
#define A2DP_APTX_ADAPTIVE_SAMPLERATE_48000        (0x10)
#define A2DP_APTX_ADAPTIVE_SAMPLERATE_96000        (0x20)

#define A2DP_APTX_ADAPTIVE_SOURCE_TYPE_1           (0x00) //(0x00)
#define A2DP_APTX_ADAPTIVE_SOURCE_TYPE_2           (0x02) //(0x02)
#define A2DP_APTX_ADAPTIVE_SOURCE_TYPE_3           (0x04) //(0x04)
#define A2DP_APTX_ADAPTIVE_SOURCE_TYPE_4           (0x06) //(0x06)
/*
#define A2DP_APTX_ADAPTIVE_AVRCP_CUSTOM_UNSUPPORTED(0x00)
#define A2DP_APTX_ADAPTIVE_AVRCP_CUSTOM_SUPPORTED  (0x01)
#define A2DP_APTX_ADAPTIVE_Q2Q                     (0x00)
#define A2DP_APTX_ADAPTIVE_NQ2Q                    (0x01)
#define A2DP_APTX_ADAPTIVE_RESERVED0               (0x00)
*/
#define A2DP_APTX_ADAPTIVE_CHANNELS_MONO          (0x01)
#define A2DP_APTX_ADAPTIVE_CHANNELS_STEREO        (0x02)
#define A2DP_APTX_ADAPTIVE_CHANNELS_TWS_STEREO    (0x04)
#define A2DP_APTX_ADAPTIVE_CHANNELS_JOINT_STEREO  (0x08)
#define A2DP_APTX_ADAPTIVE_CHANNELS_TWS_MONO      (0x10)
#define A2DP_APTX_ADAPTIVE_CHANNELS_TWS_PLUS      (0x20)

#define A2DP_APTX_ADAPTIVE_TTP_LL_0  0x50
#define A2DP_APTX_ADAPTIVE_TTP_LL_1  0x64
#define A2DP_APTX_ADAPTIVE_TTP_HQ_0  0x64
#define A2DP_APTX_ADAPTIVE_TTP_HQ_1  0x64
#define A2DP_APTX_ADAPTIVE_TTP_TWS_0 0xFF
#define A2DP_APTX_ADAPTIVE_TTP_TWS_1 0xFF
#define A2DP_APTX_ADAPTIVE_RESERVED_13THBYTE 0x00
#define A2DP_APTX_ADAPTIVE_RESERVED_14THBYTE 0x00
#define A2DP_APTX_ADAPTIVE_RESERVED_15THBYTE 0x00
#define A2DP_APTX_ADAPTIVE_EOC0 0x00
#define A2DP_APTX_ADAPTIVE_EOC1 0xAA

#define A2DP_APTX_ADAPTIVE_CAP_EXT_VER_NUM         (0x01)
#define A2DP_APTX_ADAPTIVE_SUPPORTED_FEATURES      (0x0F000000)
#define A2DP_APTX_ADAPTIVE_FIRST_SETUP_PREF        (0x02)
#define A2DP_APTX_ADAPTIVE_SECOND_SETUP_PREF       (0x03)
#define A2DP_APTX_ADAPTIVE_THIRD_SETUP_PREF        (0x03)
#define A2DP_APTX_ADAPTIVE_FOURTH_SETUP_PREF       (0x03)

#define A2DP_APTX_ADAPTIVE_OTHER_FEATURES_NONE     (0x00000000)
#define QCOM_AV_APTX_ADAPTIVE_AUDIO                (0x00)
#define QCOM_CODEC_APTX_ADAPTIVE_ID                (0xff)
#define A2DP_APTX_ADAPTIVE_CHANNEL                 (0x0001)
#define A2DP_APTX_ADAPTIVE_SAMPLERATE              (0x22)
#define CHANNEL_MODE_BACK_CHANNEL_MASK             (0x1F000000)

#endif  // A2DP_VENDOR_APTX_ADAPTIVE_CONSTANTS_H
