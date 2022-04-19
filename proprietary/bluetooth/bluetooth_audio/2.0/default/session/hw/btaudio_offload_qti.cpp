/****************************************************************************
 * Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 ****************************************************************************/
/*****************************************************************************
 *  Copyright (C) 2009-2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/
/*     btaudio_offload.c
 *
 *  Description:   Implements IPC interface between HAL and BT host
 *
 *****************************************************************************/
#include <time.h>
#include <unistd.h>

#include "ldac_level_bit_rate_lookup.h"
#include "btaudio_offload_param.h"
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <system/audio.h>
#include <utils/Log.h>
#include <cutils/properties.h>
#include <hidl/LegacySupport.h>
#include <dlfcn.h>

#include "vendor/qti/hardware/bluetooth_audio/2.0/types.h"
#include "vendor/qti/hardware/bluetooth_audio/2.0/IBluetoothAudioProvidersFactory.h"
#include "BluetoothAudioSessionControl.h"
#include "BluetoothA2dpControl.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "btaudio_offload_qti"

/*****************************************************************************
**  Constants & Macros
******************************************************************************/
/* Below two values adds up to 8 sec retry to address IOT issues*/
#define STREAM_START_MAX_RETRY_COUNT 10
#define STREAM_START_MAX_RETRY_LOOPER 8
#define CTRL_CHAN_RETRY_COUNT 3
/* 2DH5 Max (679) = 663 + 12 (AVDTP header) + 4 (L2CAP header) */
#define MAX_2MBPS_A2DP_STREAM_MTU 663
/* 3DH5 Max (1021) = 1005 + 12 (AVDTP header) + 4 (L2CAP header) */
#define MAX_3MBPS_A2DP_STREAM_MTU 1005

#define IN_CALL_TRIAL_COUNT 5
#define FAIL_TRIAL_COUNT 20
#define LW_WAIT_TRIAL_COUNT 5
#define AAC_SAMPLE_SIZE  1024
#define AAC_LATM_HEADER  12

#define CASE_RETURN_STR(const) case const: return #const;

#define FNLOG()         ALOGW(LOG_TAG, "%s", __FUNCTION__);
#define DEBUG(fmt, ...) ALOGD(LOG_TAG, "%s: " fmt,__FUNCTION__, ## __VA_ARGS__)
#define INFO(fmt, ...)  ALOGI(LOG_TAG, "%s: " fmt,__FUNCTION__, ## __VA_ARGS__)
#define WARN(fmt, ...)  ALOGW(LOG_TAG, "%s: " fmt,__FUNCTION__, ## __VA_ARGS__)
#define ERROR(fmt, ...) ALOGE(LOG_TAG, "%s: " fmt,__FUNCTION__, ## __VA_ARGS__)

#define ASSERTC(cond, msg, val) \
    if (!(cond)) { \
      ERROR("### ASSERT : %s line %d %s (%d) ###", __FILE__, __LINE__, msg, val); \
    }

namespace vendor {
namespace qti {
namespace btoffload {

/*****************************************************************************
**  Local type definitions
******************************************************************************/

struct a2dp_stream_common audio_stream;
static volatile unsigned char ack_recvd = 0;
pthread_cond_t ack_cond = PTHREAD_COND_INITIALIZER;
int  max_waittime = 4500; // in ms
int  max_waittime_pending_ack = 15000; // in ms
int wait_for_stack_response(int time_to_wait);
static bool hal_registered = false;
static bool is_value_adds_supported();

static void *vndr_fwk_lib_handle = NULL;
typedef int (*vndr_fwk_enhanced_info_t)();
static vndr_fwk_enhanced_info_t vndr_fwk_enhanced_info;

#define VNDR_FWK_DETECT_LIB "libqti_vndfwk_detect.so"

using PcmConfiguration = ::vendor::qti::hardware::bluetooth_audio::V2_0::PcmParameters;
using CodecConfiguration = ::vendor::qti::hardware::bluetooth_audio::V2_0::CodecConfiguration;
using LdacData = ::vendor::qti::hardware::bluetooth_audio::V2_0::LdacParameters;

using ::vendor::qti::hardware::bluetooth_audio::V2_0::SampleRate;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::BitsPerSample;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::ChannelMode;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::SbcBlockLength;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::SbcNumSubbands;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::SbcAllocMethod;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::LdacChannelMode;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::SbcChannelMode;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::CodecType;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::SessionType;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::IBluetoothAudioPort;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::AudioConfiguration;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::LdacQualityIndex;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::AptxAdaptiveChannelMode;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::AptxMode;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::AptxSinkBuffering;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::AptxAdaptive_TTP;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::AptxTwsParameters;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::BaCeltParameters;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::InputMode;

using ::vendor::qti::bluetooth_audio::BluetoothAudioSessionControl;
using ::vendor::qti::bluetooth_audio::BluetoothA2dpControl;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::SessionParams;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::SessionParamType;



/*****************************************************************************
**  Static functions
******************************************************************************/

audio_sbc_encoder_config_t sbc_codec;
audio_aptx_encoder_config_t aptx_codec;
audio_aac_encoder_config_t aac_codec;
audio_ldac_encoder_config_t ldac_codec;
audio_aptx_adaptive_encoder_config_t aptx_adaptive_codec;
audio_aptx_tws_encoder_config_t aptx_tws_codec;
audio_ba_celt_encoder_config_t ba_celt_codec;
/*****************************************************************************
**  Functions
******************************************************************************/
void a2dp_open_ctrl_path(struct a2dp_stream_common *common);
void ldac_codec_parser(AudioConfiguration*codec_cfg);
/*****************************************************************************
**   Miscellaneous helper functions
******************************************************************************/
static const char* dump_a2dp_ctrl_ack(tA2DP_CTRL_ACK ack)
{
    switch (ack) {
        CASE_RETURN_STR(A2DP_CTRL_ACK_SUCCESS)
        CASE_RETURN_STR(A2DP_CTRL_ACK_FAILURE)
        CASE_RETURN_STR(A2DP_CTRL_ACK_INCALL_FAILURE)
        CASE_RETURN_STR(A2DP_CTRL_ACK_PENDING)
        CASE_RETURN_STR(A2DP_CTRL_ACK_SW_ERROR)
        CASE_RETURN_STR(A2DP_CTRL_ACK_LW_ERROR)
        CASE_RETURN_STR(A2DP_CTRL_ACK_UNSUPPORTED)
        CASE_RETURN_STR(A2DP_CTRL_ACK_FAILURE_DISC_IN_PROGRESS)
        CASE_RETURN_STR(A2DP_CTRL_ACK_FAILURE_NO_WAIT)
        CASE_RETURN_STR(A2DP_CTRL_ACK_UNKNOWN)
    }
    return "UNKNOWN A2DP_CTRL_ACK";
}

static const char* dump_a2dp_hal_state(tA2DP_STATE state)
{
    switch (state) {
        CASE_RETURN_STR(AUDIO_A2DP_STATE_STARTING)
        CASE_RETURN_STR(AUDIO_A2DP_STATE_STARTED)
        CASE_RETURN_STR(AUDIO_A2DP_STATE_STOPPING)
        CASE_RETURN_STR(AUDIO_A2DP_STATE_STOPPED)
        CASE_RETURN_STR(AUDIO_A2DP_STATE_SUSPENDED)
        CASE_RETURN_STR(AUDIO_A2DP_STATE_STANDBY)
    }
    return "UNKNOWN A2DP_STATE";
}

static void* a2dp_codec_parser(AudioConfiguration*codec_cfg,
                               audio_format_t *codec_type,
                               uint32_t *sample_freq)
{
    CodecConfiguration codecConfig =  codec_cfg->codecConfig;
    ALOGI("%s: codec_type = %x", __func__, codecConfig.codecType);
    if (codecConfig.codecType == CodecType::SBC) {
        memset(&sbc_codec, 0, sizeof(audio_sbc_encoder_config_t));

        switch (codecConfig.config.sbcConfig.sampleRate) {
            case SampleRate::RATE_48000:
                 sbc_codec.sampling_rate = 48000;
                 break;
            case SampleRate::RATE_44100:
                 sbc_codec.sampling_rate = 44100;
                 break;
            default:
                 ALOGE("%s: SBC: unknown sampling rate:%d", __func__,
                       codecConfig.config.sbcConfig.sampleRate);
                 break;
        }

        switch (codecConfig.config.sbcConfig.channelMode) {
            case SbcChannelMode::STEREO:
                 sbc_codec.channels = A2D_SBC_CHANNEL_STEREO;
                 break;
            case SbcChannelMode::JOINT_STEREO:
                 sbc_codec.channels = A2D_SBC_CHANNEL_JOINT_STEREO;
                 break;
            case SbcChannelMode::DUAL:
                 sbc_codec.channels = A2D_SBC_CHANNEL_DUAL_MONO;
                 break;
            case SbcChannelMode::MONO:
                 sbc_codec.channels = A2D_SBC_CHANNEL_MONO;
                 break;
            default:
                 ALOGE("%s: SBC: unknown channel mode:%u", __func__,
                       (unsigned)codecConfig.config.sbcConfig.channelMode);
                 break;
        }

        switch (codecConfig.config.sbcConfig.blockLength) {
            case SbcBlockLength::BLOCKS_16:
                sbc_codec.blk_len = 16;
                break;
            case SbcBlockLength::BLOCKS_12:
                sbc_codec.blk_len = 12;
                break;
            case SbcBlockLength::BLOCKS_8:
                sbc_codec.blk_len = 8;
                break;
            case SbcBlockLength::BLOCKS_4:
                sbc_codec.blk_len = 4;
                break;
            default:
                ALOGE("%s: SBC: unknown block length:%u", __func__,
                      (unsigned int)codecConfig.config.sbcConfig.blockLength);
                break;
        }

        switch (codecConfig.config.sbcConfig.numSubbands) {
            case SbcNumSubbands::SUBBAND_8:
                sbc_codec.subband = 8;
                break;
            case SbcNumSubbands::SUBBAND_4:
                sbc_codec.subband = 4;
                break;
            default:
                ALOGE("%s: SBC: unknown subband:%u", __func__,
                      (unsigned int)codecConfig.config.sbcConfig.numSubbands);
                break;
        }

        switch (codecConfig.config.sbcConfig.allocMethod) {
            case SbcAllocMethod::ALLOC_MD_L:
                sbc_codec.alloc = 1;
                break;
            case SbcAllocMethod::ALLOC_MD_S:
                sbc_codec.alloc = 2;
                break;
            default:
                ALOGE("%s: SBC: unknown alloc method:%u", __func__,
                      (unsigned int)codecConfig.config.sbcConfig.allocMethod);
                break;
        }

        switch (codecConfig.config.sbcConfig.bitsPerSample) {
            case BitsPerSample::BITS_16:
                sbc_codec.bits_per_sample = 16;
                break;
            case BitsPerSample::BITS_24:
                sbc_codec.bits_per_sample = 24;
                break;
            case BitsPerSample::BITS_32:
                sbc_codec.bits_per_sample = 32;
                break;
            default:
                ALOGE("%s: SBC: unknown bits per sample", __func__);
                break;
        }

        sbc_codec.min_bitpool = codecConfig.config.sbcConfig.minBitpool;
        sbc_codec.max_bitpool = codecConfig.config.sbcConfig.maxBitpool;
        ALOGD("%s: SBC: copied codec config: min_bitpool:%d max_bitpool:%d",
              __func__, sbc_codec.min_bitpool, sbc_codec.max_bitpool);

        if (codecConfig.encodedAudioBitrate == 0) {
            ALOGW("%s: SBC: bitrate is zero", __func__);
            sbc_codec.bitrate = 328000;
        } else if (codecConfig.encodedAudioBitrate >= 0x00000001 &&
                   codecConfig.encodedAudioBitrate <= 0x00FFFFFF) {
            sbc_codec.bitrate = codecConfig.encodedAudioBitrate;
        }

        ALOGI("%s: SBC: bitrate:%d i/p bitrate:%d", __func__, sbc_codec.bitrate,
              codecConfig.encodedAudioBitrate);
        *codec_type = AUDIO_FORMAT_SBC;

        if (sample_freq) *sample_freq = sbc_codec.sampling_rate;

        ALOGI("%s: SBC: done copying full codec config", __func__);
        return ((void *)(&sbc_codec));

    }else if (codecConfig.codecType == CodecType::AAC) {
        memset(&aac_codec, 0, sizeof(audio_aac_encoder_config_t));

        //USE 0 (AAC_LC) as hardcoded value till Audio
        //define constants
        aac_codec.enc_mode = 0;

        //USE LOAS(1) or LATM(4) hardcoded values till
        //Audio define proper constants
        aac_codec.format_flag = 4;

        switch (codecConfig.config.aacConfig.sampleRate) {
            case SampleRate::RATE_44100:
                aac_codec.sampling_rate = 44100;
                break;
            case SampleRate::RATE_48000:
                aac_codec.sampling_rate = 48000;
                break;
            case SampleRate::RATE_88200:
                aac_codec.sampling_rate = 88200;
                break;
            case SampleRate::RATE_96000:
                aac_codec.sampling_rate = 96000;
                break;
            default:
                ALOGE("%s: AAC: invalid sample rate:%d", __func__,
                      codecConfig.config.aacConfig.sampleRate);
                break;
        }

        switch (codecConfig.config.aacConfig.channelMode) {
            case ChannelMode::MONO:
                 aac_codec.channels = 1;
                 break;
            case ChannelMode::STEREO:
                 aac_codec.channels = 2;
                 break;
            default:
                 ALOGE("%s: AAC: unknown channel mode:%u", __func__,
                       (unsigned)codecConfig.config.aacConfig.channelMode);
                 break;
        }

        /*switch (byte & A2D_AAC_IE_VBR_MSK) {
            case A2D_AAC_IE_VBR:
                break;
            default:
                ALOGE("%s: AAC: VBR not supported", __func__);
                break;
        }*/

        switch (codecConfig.config.aacConfig.bitsPerSample) {
            case BitsPerSample::BITS_16:
                aac_codec.bits_per_sample = 16;
                break;
            case BitsPerSample::BITS_24:
                aac_codec.bits_per_sample = 24;
                break;
            case BitsPerSample::BITS_32:
                aac_codec.bits_per_sample = 32;
                break;
            default:
                ALOGE("%s: AAC: unknown bits per sample", __func__);
                break;
        }

        uint32_t bitrate_hal = 0;
        if (codecConfig.encodedAudioBitrate == 0) {
            ALOGW("%s: AAC: bitrate is zero", __func__);
        } else if (codecConfig.encodedAudioBitrate >= 0x00000001 &&
                   codecConfig.encodedAudioBitrate <= 0x00FFFFFF) {
            bitrate_hal = codecConfig.encodedAudioBitrate;
        }

        *codec_type = AUDIO_FORMAT_AAC;
        ALOGI("%s: AAC HAL: bitrate:%" PRIu32 " peermtu:%" PRIu16, __func__,
              bitrate_hal, codecConfig.peerMtu);
        uint16_t mtu = codecConfig.peerMtu;
        if ((mtu == 0) || (mtu > MAX_3MBPS_A2DP_STREAM_MTU))
            mtu = MAX_3MBPS_A2DP_STREAM_MTU;

        aac_codec.bitrate = bitrate_hal;
        // Configure DSP for peak MTU
        aac_codec.frame_ctl.ctl_type = A2D_AAC_FRAME_PEAK_MTU;
        aac_codec.frame_ctl.ctl_value = mtu;
        ALOGI("%s: AAC HW: sampling_rate:%" PRIu32 " bitrate:%" PRIu32 " mtu:%d ctl_type:%d",
                __func__, aac_codec.sampling_rate, aac_codec.bitrate, mtu,
                aac_codec.frame_ctl.ctl_type);
        if (sample_freq) *sample_freq = aac_codec.sampling_rate;
        ALOGI("%s: AAC: done copying full codec config", __func__);
        return ((void *)(&aac_codec));

    } else if (codecConfig.codecType == CodecType::APTX ||
               codecConfig.codecType == CodecType::APTX_HD) {
        memset(&aptx_codec, 0, sizeof(audio_aptx_encoder_config_t));

        switch (codecConfig.config.aptxConfig.sampleRate) {
            case SampleRate::RATE_48000:
                 aptx_codec.sampling_rate = 48000;
                 break;
            case SampleRate::RATE_44100:
                 aptx_codec.sampling_rate = 44100;
                 break;
            default:
                 ALOGE("%s: APTX: unknown sampling rate:%d", __func__,
                       codecConfig.config.aptxConfig.sampleRate);
                 break;
        }

        switch (codecConfig.config.aptxConfig.channelMode) {
            case ChannelMode::STEREO:
                 aptx_codec.channels = 2;
                 break;
            case ChannelMode::MONO:
                 aptx_codec.channels = 1;
                 break;
            default:
                 ALOGE("%s: APTX: unknown channel mode:%u", __func__,
                       (unsigned)codecConfig.config.aptxConfig.channelMode);
                 break;
        }

        switch (codecConfig.config.aptxConfig.bitsPerSample) {
            case BitsPerSample::BITS_16:
                aptx_codec.bits_per_sample = 16;
                break;
            case BitsPerSample::BITS_24:
                aptx_codec.bits_per_sample = 24;
                break;
            case BitsPerSample::BITS_32:
                aptx_codec.bits_per_sample = 32;
                break;
            default:
                ALOGE("%s: APTX: unknown bits per sample", __func__);
                break;
        }

        ALOGD("%s: APTX: codec config copied", __func__);

        if (codecConfig.encodedAudioBitrate == 0) {
            ALOGW("%s: APTX: bitrate is zero", __func__);
        } else if (codecConfig.encodedAudioBitrate >= 0x00000001 &&
                   codecConfig.encodedAudioBitrate <= 0x00FFFFFF) {
            aptx_codec.bitrate = codecConfig.encodedAudioBitrate;
        }

        if (codecConfig.codecType == CodecType::APTX) {
           *codec_type = AUDIO_FORMAT_APTX;
           ALOGI("%s: APTX: done copying full codec config", __func__);
        } else if (codecConfig.codecType == CodecType::APTX_HD) {
           *codec_type = AUDIO_FORMAT_APTX_HD;
           ALOGI("%s: APTXHD: done copying full codec config", __func__);
        }

        if (sample_freq) *sample_freq = aptx_codec.sampling_rate;
        return ((void *)&aptx_codec);

    } else if (codecConfig.codecType == CodecType::LDAC) {
        *codec_type = AUDIO_FORMAT_LDAC;
        ldac_codec_parser(codec_cfg);
        return ((void *)&ldac_codec);
    } else if (codecConfig.codecType == CodecType::APTX_ADAPTIVE) {
        *codec_type = static_cast<audio_format_t>(ENC_CODEC_TYPE_APTX_ADAPTIVE);
        memset(&aptx_adaptive_codec, 0, sizeof(audio_aptx_adaptive_encoder_config_t));

        switch (codecConfig.config.aptxAdaptiveConfig.sampleRate) {
            case SampleRate::RATE_96000:
                 aptx_adaptive_codec.sampling_rate = 0x3;
                 if(sample_freq) *sample_freq = 96000;
                 break;
            case SampleRate::RATE_48000:
                 aptx_adaptive_codec.sampling_rate = 0x1;
                 if(sample_freq) *sample_freq = 48000;
                 break;
            case SampleRate::RATE_44100:
                 aptx_adaptive_codec.sampling_rate = 0x2;
                 if(sample_freq) *sample_freq = 44100;
                 break;
            default:
                 ALOGE("%s: APTX: unknown sampling rate:%d", __func__,
                       codecConfig.config.aptxAdaptiveConfig.sampleRate);
                 break;
        }

        switch ((int)codecConfig.config.aptxAdaptiveConfig.channelMode) {
            case A2DP_APTX_ADAPTIVE_CHANNELS_MONO:
                 aptx_adaptive_codec.channel_mode = (int32_t)AptxAdaptiveChannelMode::MONO;
                 break;
            case A2DP_APTX_ADAPTIVE_CHANNELS_STEREO:
                 aptx_adaptive_codec.channel_mode = (int32_t)AptxAdaptiveChannelMode_STEREO;
                 break;
            case A2DP_APTX_ADAPTIVE_CHANNELS_TWS_STEREO:
                 aptx_adaptive_codec.channel_mode = (int32_t)AptxAdaptiveChannelMode::TWS_STEREO;
                 break;
            case A2DP_APTX_ADAPTIVE_CHANNELS_JOINT_STEREO:
                 aptx_adaptive_codec.channel_mode = (int32_t)AptxAdaptiveChannelMode::JOINT_STEREO;
                 break;
            case A2DP_APTX_ADAPTIVE_CHANNELS_TWS_MONO:
                 aptx_adaptive_codec.channel_mode = (int32_t)AptxAdaptiveChannelMode::DUAL_MONO;
                 break;
            case A2DP_APTX_ADAPTIVE_CHANNELS_EARBUD:
                 aptx_adaptive_codec.channel_mode = (int32_t)AptxAdaptiveChannelMode::EARBUD;
                 break;
            default:
                 ALOGE("%s: APTX: unknown channel mode:%u", __func__,
                  (unsigned)codecConfig.config.aptxAdaptiveConfig.channelMode);
                 break;
        }

        switch (codecConfig.config.aptxAdaptiveConfig.bitsPerSample) {
            case BitsPerSample::BITS_16:
                aptx_adaptive_codec.bits_per_sample = 16;
                break;
            case BitsPerSample::BITS_24:
                aptx_adaptive_codec.bits_per_sample = 24;
                break;
            case BitsPerSample::BITS_32:
                aptx_adaptive_codec.bits_per_sample = 32;
                break;
            default:
                ALOGE("%s: APTX: unknown bits per sample", __func__);
                break;
        }

        switch (codecConfig.config.aptxAdaptiveConfig.aptxMode) {
            case AptxMode::HQ:
                aptx_adaptive_codec.aptx_mode = 0x1000;
                break;
            case AptxMode::LL:
                aptx_adaptive_codec.aptx_mode = 0x2000;
                break;
            case AptxMode::ULL:
                aptx_adaptive_codec.aptx_mode = 0x4000;
                break;
            case AptxMode::SCAN_CONTROL:
                aptx_adaptive_codec.aptx_mode = 0x8000;
                break;
            default:
                ALOGE("%s: APTX-Adaptive: unknown Aptx-AD mode", __func__);
                break;
        }

        /*Set default Input Mode*/
        aptx_adaptive_codec.input_mode = (uint32_t)InputMode::STEREO;

        aptx_adaptive_codec.min_sink_buffering_LL = codecConfig.config.aptxAdaptiveConfig.sinkBuffering.minSinkBuff_LL;
        aptx_adaptive_codec.max_sink_buffering_LL = codecConfig.config.aptxAdaptiveConfig.sinkBuffering.maxSinkBuff_LL;
        aptx_adaptive_codec.min_sink_buffering_HQ = codecConfig.config.aptxAdaptiveConfig.sinkBuffering.minSinkBuff_HQ;
        aptx_adaptive_codec.max_sink_buffering_HQ = codecConfig.config.aptxAdaptiveConfig.sinkBuffering.maxSinkBuff_HQ;
        aptx_adaptive_codec.min_sink_buffering_TWS = codecConfig.config.aptxAdaptiveConfig.sinkBuffering.minSinkBuff_TWS;
        aptx_adaptive_codec.max_sink_buffering_TWS = codecConfig.config.aptxAdaptiveConfig.sinkBuffering.maxSinkBuff_TWS;

        uint16_t mtu = codecConfig.peerMtu;
        aptx_adaptive_codec.mtu = mtu;

        aptx_adaptive_codec.TTP_LL_low = codecConfig.config.aptxAdaptiveConfig.ttp.TTP_LL_low;
        aptx_adaptive_codec.TTP_LL_high = codecConfig.config.aptxAdaptiveConfig.ttp.TTP_LL_high;
        aptx_adaptive_codec.TTP_HQ_low = codecConfig.config.aptxAdaptiveConfig.ttp.TTP_HQ_low;
        aptx_adaptive_codec.TTP_HQ_high = codecConfig.config.aptxAdaptiveConfig.ttp.TTP_HQ_high;
        aptx_adaptive_codec.TTP_TWS_low = codecConfig.config.aptxAdaptiveConfig.ttp.TTP_TWS_low;
        aptx_adaptive_codec.TTP_TWS_high = codecConfig.config.aptxAdaptiveConfig.ttp.TTP_TWS_high;
        aptx_adaptive_codec.input_fade_duration =
                               codecConfig.config.aptxAdaptiveConfig.inputFadeDuration;
        for (int i = 0; i < sizeof(aptx_adaptive_codec.sink_capabilities); i ++) {
          aptx_adaptive_codec.sink_capabilities[i] =
                               codecConfig.config.aptxAdaptiveConfig.aptxAdaptiveConfigStream[i];
          ALOGW("%s: ## aptXAdaptive ## SinkCaps[%d] =  %d ",
                               __func__, i, aptx_adaptive_codec.sink_capabilities[i]);
        }

        ALOGW("%s: ## aptXAdaptive ## sampleRate 0x%x", __func__, aptx_adaptive_codec.sampling_rate);
        ALOGW("%s: ## aptXAdaptive ## channelMode 0x%x", __func__, aptx_adaptive_codec.channel_mode);
        ALOGW("%s: ## aptXAdaptive ## min_sink_buffering_LL 0x%x", __func__, aptx_adaptive_codec.min_sink_buffering_LL);
        ALOGW("%s: ## aptXAdaptive ## max_sink_buffering_LL 0x%x", __func__, aptx_adaptive_codec.max_sink_buffering_LL);
        ALOGW("%s: ## aptXAdaptive ## min_sink_buffering_HQ 0x%x", __func__, aptx_adaptive_codec.min_sink_buffering_HQ);
        ALOGW("%s: ## aptXAdaptive ## max_sink_buffering_HQ 0x%x", __func__, aptx_adaptive_codec.max_sink_buffering_HQ);
        ALOGW("%s: ## aptXAdaptive ## min_sink_buffering_TWS 0x%x", __func__, aptx_adaptive_codec.min_sink_buffering_TWS);
        ALOGW("%s: ## aptXAdaptive ## max_sink_buffering_TWS 0x%x", __func__, aptx_adaptive_codec.max_sink_buffering_TWS);
        ALOGW("%s: ## aptXAdaptive ## ttp_ll_0 0x%x", __func__, aptx_adaptive_codec.TTP_LL_low);
        ALOGW("%s: ## aptXAdaptive ## ttp_ll_1 0x%x", __func__, aptx_adaptive_codec.TTP_LL_high);
        ALOGW("%s: ## aptXAdaptive ## ttp_hq_0 0x%x", __func__, aptx_adaptive_codec.TTP_HQ_low);
        ALOGW("%s: ## aptXAdaptive ## ttp_hq_1 0x%x", __func__, aptx_adaptive_codec.TTP_HQ_high);
        ALOGW("%s: ## aptXAdaptive ## ttp_tws_0 0x%x", __func__, aptx_adaptive_codec.TTP_TWS_low);
        ALOGW("%s: ## aptXAdaptive ## ttp_tws_1 0x%x", __func__, aptx_adaptive_codec.TTP_TWS_high);
        ALOGW("%s: ## aptXAdaptive ## MTU =  %d", __func__, aptx_adaptive_codec.mtu);
        ALOGW("%s: ## aptXAdaptive ## Bits Per Sample =  %d", __func__, aptx_adaptive_codec.bits_per_sample);
        ALOGW("%s: ## aptXAdaptive ## Mode =  %d", __func__, aptx_adaptive_codec.aptx_mode);
        ALOGW("%s: ## aptXAdaptive ## Input mode =  %d", __func__, aptx_adaptive_codec.input_mode);
        ALOGW("%s: ## aptXAdaptive ## input_fade_duriation =  %d",
                                                __func__, aptx_adaptive_codec.input_fade_duration);

        //aptx_adaptive_codec.input_fade_duration = codecConfig.config.aptxAdaptiveConfig.inputFadeDuration;
        //memcpy(aptx_adaptive_codec.sink_capabilities,
                //codecConfig.config.aptxAdaptiveConfig.aptxAdaptiveConfigStream.data(),
                //sizeof(aptx_adaptive_codec.sink_capabilities));
        return ((void *)&aptx_adaptive_codec);
    } else if (codecConfig.codecType == CodecType::APTX_TWS) {
        *codec_type = static_cast<audio_format_t>(ENC_CODEC_TYPE_APTX_DUAL_MONO);

        memset(&aptx_tws_codec, 0, sizeof(audio_aptx_tws_encoder_config_t));
        switch (codecConfig.config.aptxTwsConfig.sampleRate) {
            case SampleRate::RATE_48000:
                aptx_tws_codec.sampling_rate = 48000;
                break;
            case SampleRate::RATE_44100:
                aptx_tws_codec.sampling_rate = 44100;
                break;
            default:
                ALOGE("%s: aptX-TWS: unknown sampling rate",__func__);
        }
        aptx_tws_codec.channels = 2;//Channel mode is alwasy mono = 8;
        aptx_tws_codec.bitrate = codecConfig.encodedAudioBitrate;
        aptx_tws_codec.sync_mode = codecConfig.config.aptxTwsConfig.syncMode;

        return ((void *)&aptx_tws_codec);
    } else if (codecConfig.codecType == CodecType::BA_CELT) {
       //TODO not tested
        *codec_type = static_cast<audio_format_t>(AUDIO_CODEC_TYPE_CELT);

        memset(&ba_celt_codec,0,sizeof(audio_ba_celt_encoder_config_t));
        switch (codecConfig.config.baCeltConfig.sampleRate) {
            case SampleRate::RATE_48000:
                 ba_celt_codec.sampling_rate = 48000;
                 break;
            case SampleRate::RATE_44100:
                 ba_celt_codec.sampling_rate = 44100;
                 break;
            case SampleRate::RATE_32000:
                 ba_celt_codec.sampling_rate = 32000;
                 break;
            default:
                 ALOGE("%s: CELT: unknown sampling rate:%d", __func__,
                       (uint16_t)codecConfig.config.baCeltConfig.sampleRate);
                 break;
        }

        switch (codecConfig.config.baCeltConfig.channelMode) {
            case ChannelMode::STEREO:
                 ba_celt_codec.channels = 2;
                 break;
            case ChannelMode::MONO:
                 ba_celt_codec.channels = 1;
                 break;
            default:
                 ALOGE("%s: CELT: unknown channel mode:%u", __func__,
                       (uint8_t)codecConfig.config.baCeltConfig.channelMode);
                 break;
        }

        //ba_celt_codec.sampling_rate = (uint16_t)codecConfig.config.baCeltConfig.sampleRate;
        //ba_celt_codec.channels = (uint8_t)codecConfig.config.baCeltConfig.channelMode;
        ba_celt_codec.frame_size = codecConfig.config.baCeltConfig.frameSize;
        ba_celt_codec.complexity = codecConfig.config.baCeltConfig.complexity;
        ba_celt_codec.prediction_mode = codecConfig.config.baCeltConfig.predictionMode;
        ba_celt_codec.vbr_flag = codecConfig.config.baCeltConfig.vbrFlag;
        ba_celt_codec.bitrate = codecConfig.encodedAudioBitrate;

        ALOGE("CELT Bitrate: 0%x", ba_celt_codec.bitrate);
        ALOGE("CELT channel: 0%x", ba_celt_codec.channels);
        ALOGE("CELT complexity: 0%x", ba_celt_codec.complexity);
        ALOGE("CELT frame_size: 0%x", ba_celt_codec.frame_size);
        ALOGE("CELT prediction_mode: 0%x", ba_celt_codec.prediction_mode);
        ALOGE("CELT sampl_freq: 0%x", ba_celt_codec.sampling_rate);
        ALOGE("CELT vbr_flag: 0%x", ba_celt_codec.vbr_flag);

        return ((void *)&ba_celt_codec);
    }

    return NULL;
}

/*****************************************************************************
**
** AUDIO DATA PATH
**
*****************************************************************************/

void stack_resp_cb (const uint16_t& cntrl_key __unused,
            const bool& start_resp __unused, const BluetoothAudioStatus&  status __unused) {

    ALOGI("%s: status = %hhu", __func__, status);
    pthread_mutex_lock(&audio_stream.ack_lock);
    audio_stream.ack_status = (tA2DP_CTRL_ACK)(status);
    if (!ack_recvd) {
        ack_recvd = 1;
        pthread_cond_signal(&ack_cond);
    }
    pthread_mutex_unlock(&audio_stream.ack_lock);
}

void session_resp_cb (const uint16_t& cntrl_key __unused)
{
    ALOGI("%s", __func__);
    pthread_mutex_lock(&audio_stream.ack_lock);
  //  if (cntrl_key == audio_stream.ctrl_key)  {
       ALOGI("%s: session ready  = %d", __func__, audio_stream.session_ready);
       if (audio_stream.session_ready || audio_stream.state == AUDIO_A2DP_STATE_STARTED
                     || audio_stream.state == AUDIO_A2DP_STATE_STARTING)
       {
         audio_stream.state = AUDIO_A2DP_STATE_STANDBY;
         audio_stream.session_ready = 0;
         ALOGI("%s:  end session session ready  = %d", __func__, audio_stream.session_ready);
       } else if ((!audio_stream.session_ready)) {
          audio_stream.session_ready = 1;
          audio_stream.state = AUDIO_A2DP_STATE_STANDBY;
          memset(&audio_stream.codec_cfg, 0, sizeof(CodecConfiguration));
          audio_stream.codec_cfg = BluetoothAudioSessionControl::GetAudioConfig(audio_stream.sessionType);
       }
       ALOGD("%s: state = %s", __func__, dump_a2dp_hal_state(audio_stream.state));
       if (!ack_recvd) {
          ack_recvd = 1;
          pthread_cond_signal(&ack_cond);
       }
    //}
    pthread_mutex_unlock(&audio_stream.ack_lock);
}

void session_params_cb (const uint16_t& cntrl_key __unused,
                        const SessionParams &session_params)
{
    ALOGI("%s", __func__);
    pthread_mutex_lock(&audio_stream.ack_lock);
    ALOGI("%s: session ready  = %d", __func__, audio_stream.session_ready);
    if (audio_stream.session_ready) {
      if (session_params.paramType == SessionParamType::MTU) {
        audio_stream.codec_cfg.codecConfig.peerMtu = session_params.param.mtu;
      }
    }
    ALOGD("%s: state = %s", __func__, dump_a2dp_hal_state(audio_stream.state));
    pthread_mutex_unlock(&audio_stream.ack_lock);
}

static void btapoffload_port_init()
{
    ALOGI("%s", __func__);
    if (BluetoothAudioSessionControl::IsSessionReady(audio_stream.sessionType)) {
       audio_stream.state = AUDIO_A2DP_STATE_STANDBY;
       audio_stream.session_ready = 1;
       memset(&audio_stream.codec_cfg, 0, sizeof(CodecConfiguration));
       audio_stream.codec_cfg = BluetoothAudioSessionControl::GetAudioConfig(audio_stream.sessionType);
    } else
        ALOGE("%s, bluetooth provider session is not avail", __func__);

    audio_stream.control_result_cb = stack_resp_cb;
    audio_stream.session_changed_cb = session_resp_cb;
    audio_stream.session_params_cb = session_params_cb;
    ::vendor::qti::bluetooth_audio::PortStatusCallbacks cbacks = {
      .control_result_cb_ = audio_stream.control_result_cb,
      .session_changed_cb_ = audio_stream.session_changed_cb,
      .session_params_cb_ = audio_stream.session_params_cb};
      audio_stream.ctrl_key = BluetoothAudioSessionControl::RegisterControlResultCback(
         audio_stream.sessionType, cbacks);
}

static void btapoffload_port_deinit() {
   ALOGI("%s start", __func__);
   BluetoothAudioSessionControl::UnregisterControlResultCback(
                audio_stream.sessionType,audio_stream.ctrl_key);
}
void a2dp_stream_common_init(struct a2dp_stream_common *common)
{
    pthread_mutexattr_t lock_attr;
    ALOGI("%s", __func__);

    pthread_mutexattr_init(&lock_attr);
    pthread_mutexattr_settype(&lock_attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&common->lock, &lock_attr);
    memset(common,0,sizeof(a2dp_stream_common));
    pthread_mutex_init(&common->ack_lock, &lock_attr);
    pthread_mutexattr_destroy(&lock_attr);
    common->sink_latency = A2DP_DEFAULT_SINK_LATENCY;
    common->sessionType = SessionType::A2DP_HARDWARE_OFFLOAD_DATAPATH;
    btapoffload_port_init();
}

int wait_for_stack_response(int time_to_wait)
{
    struct timespec now;
    int ret = 0;
    ALOGW("%s: conditional wait: ack_recvd = %d", __func__, ack_recvd);
    if (!ack_recvd) {
        clock_gettime(CLOCK_REALTIME, &now);
        now.tv_sec += (time_to_wait / 1000);
        now.tv_nsec += ((time_to_wait % 1000) * 1000000);
        // Avoid nanosec overflow
        now.tv_sec += (now.tv_nsec / 1000000000L);
        now.tv_nsec = (now.tv_nsec % 1000000000L);
        ret = pthread_cond_timedwait(&ack_cond, &audio_stream.ack_lock, &now);
        ALOGI("%s:ret = %d(%s)",__func__, ret, strerror(ret));
    } else {
        ALOGD("%s: ack already received", __func__);
    }
    ALOGW("%s: conditional wait done : ack_recvd = %d", __func__, ack_recvd);
    return ret;
}

tHIDL_REQUEST_STATUS handle_hidl_request(tA2DP_CTRL_CMD cmd,
                    tA2DP_CTRL_ACK *status, int wait_time,
                    int max_pending_wait_time) {
    bool ret = false;
    tHIDL_REQUEST_STATUS hidl_status = HIDL_REQUEST_STATUS_SUCCESS;
    pthread_mutex_lock(&audio_stream.ack_lock);
    *status = A2DP_CTRL_ACK_UNKNOWN;
    if (!audio_stream.session_ready) {
        ALOGE("%s: stack deinitialized", __func__);
        hidl_status = HIDL_REQUEST_STATUS_CLIENT_NOT_READY;
        goto end;
    }
    // reset the ack_recvd
    ack_recvd = 0;
    audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
    pthread_mutex_unlock(&audio_stream.ack_lock);
    switch(cmd) {
        case A2DP_CTRL_CMD_START: {
            ALOGW("%s: starting audio stream", __func__);
            ret = BluetoothAudioSessionControl::StartStream(
                    audio_stream.sessionType);
            break;
        }
        case A2DP_CTRL_CMD_SUSPEND:
        case A2DP_CTRL_CMD_STOP: {
            ALOGW("%s: suspending audio stream", __func__);
            ret = BluetoothAudioSessionControl::SuspendStream(
                    audio_stream.sessionType);
            break;
        }
        default:
            break;
    }
    pthread_mutex_lock(&audio_stream.ack_lock);
    if (!ret) {
        hidl_status = HIDL_REQUEST_STATUS_CLIENT_DEAD;
        goto end;
    }

    if (!audio_stream.session_ready) {
        ALOGE("%s: stack deinitialized after session control", __func__);
        hidl_status = HIDL_REQUEST_STATUS_CLIENT_NOT_READY;
        goto end;
    }

    // wait for stack response
    if (audio_stream.ack_status == A2DP_CTRL_ACK_UNKNOWN) {
        wait_for_stack_response(wait_time);
    }

    // check if ack is recieved or not
    if(!ack_recvd) {
        hidl_status = HIDL_REQUEST_STATUS_TIMEOUT;
        goto end;
    }
    ALOGI("%s: ack status = %s", __func__,
          dump_a2dp_ctrl_ack(audio_stream.ack_status));
    // wait for some more time to get final status
    // if interim status is pending
    if(audio_stream.ack_status == A2DP_CTRL_ACK_PENDING) {
        audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
        ack_recvd = 0;
        wait_for_stack_response(max_pending_wait_time);
        if(!ack_recvd) {
            hidl_status = HIDL_REQUEST_STATUS_TIMEOUT;
        } else {
            *status = audio_stream.ack_status;
            hidl_status = HIDL_REQUEST_STATUS_SUCCESS;
        }
    } else {
        *status = audio_stream.ack_status;
        hidl_status = HIDL_REQUEST_STATUS_SUCCESS;
    }

end:
    // set the ack_recvd to true to avoid false signalling
    ack_recvd = 1;
    pthread_mutex_unlock(&audio_stream.ack_lock);
    return hidl_status;
}

int audio_stream_start_qti()
{
    int retry_count = 0;
    int wait_time = 0;
    tA2DP_CTRL_ACK status = A2DP_CTRL_ACK_UNKNOWN;
    ALOGD("%s: state = %s", __func__, dump_a2dp_hal_state(audio_stream.state));
    pthread_mutex_lock(&audio_stream.lock);
    if (audio_stream.state == AUDIO_A2DP_STATE_SUSPENDED) {
        ALOGW("%s: stream suspended", __func__);
        pthread_mutex_unlock(&audio_stream.lock);
        return -1;
    }

    auto ret = handle_hidl_request(A2DP_CTRL_CMD_START, &status,
                                   max_waittime, max_waittime_pending_ack);
    if (ret != HIDL_REQUEST_STATUS_SUCCESS) {
       ALOGE("%s: hidl request failed ret:%d ",__func__, ret);
       goto end;
    }
    audio_stream.state = AUDIO_A2DP_STATE_STARTING;

    if (status == A2DP_CTRL_ACK_SUCCESS) {
         ALOGI("%s: a2dp stream started successfully", __func__);
         goto end;
    } else if(status == A2DP_CTRL_ACK_LW_ERROR) {
         ALOGD("%s: status = %s", __func__, dump_a2dp_ctrl_ack(status));
         retry_count = LW_WAIT_TRIAL_COUNT;
         wait_time = 500;
    } else if (status == A2DP_CTRL_ACK_INCALL_FAILURE) {
         retry_count = IN_CALL_TRIAL_COUNT;
         wait_time = 200;
    } else if (status == A2DP_CTRL_ACK_FAILURE) {
         retry_count = FAIL_TRIAL_COUNT;
         wait_time = 200;
    } else if( status == A2DP_CTRL_ACK_FAILURE_DISC_IN_PROGRESS) {
         ALOGD("%s: BT stack is disabling", __func__);
         goto end;
    }
    for (int i = 0; i < retry_count; i++) {
        ALOGI("%s:retry StartStream",__func__);
        auto ret = handle_hidl_request(A2DP_CTRL_CMD_START, &status,
                                      wait_time, max_waittime_pending_ack);
        if (ret != HIDL_REQUEST_STATUS_SUCCESS) {
           ALOGE("%s: hidl request failed ret:%d ",__func__, ret);
           break;
        }
        if(status == A2DP_CTRL_ACK_SUCCESS) {
            ALOGI("%s: a2dp stream started successfully", __func__);
            break;
        } else if(status == A2DP_CTRL_ACK_FAILURE ||
                  status == A2DP_CTRL_ACK_INCALL_FAILURE ||
                  status == A2DP_CTRL_ACK_LW_ERROR) {
            //wait for 200msec before retry for A2DP_CTRL_ACK_FAILURE
            //and A2DP_CTRL_ACK_INCALL_FAILURE but 1sec for
            //A2DP_CTRL_ACK_LW_ERROR
            ALOGI("%s:status = %s, wait for %dmsec",__func__,
                                   dump_a2dp_ctrl_ack(status), wait_time);
            usleep(wait_time *1000);
        } else if(status == A2DP_CTRL_ACK_FAILURE_DISC_IN_PROGRESS) {
            ALOGD("%s: status = %s", __func__, dump_a2dp_ctrl_ack(status));
            break;
        }
    }

end:
    if (status == A2DP_CTRL_ACK_SUCCESS) {
        audio_stream.state = AUDIO_A2DP_STATE_STARTED;
    } else {
        ALOGW("%s: a2dp stream start failed: status = %s", __func__,
                dump_a2dp_ctrl_ack(status));
        audio_stream.state = AUDIO_A2DP_STATE_STANDBY;
    }

    pthread_mutex_unlock(&audio_stream.lock);
    return status;
}

static bool is_value_adds_supported() {
    int vendor_info = -1;
    if(vndr_fwk_lib_handle == NULL) {
        ALOGE("%s: openning the lib %s", __func__, VNDR_FWK_DETECT_LIB);
        vndr_fwk_lib_handle = dlopen(VNDR_FWK_DETECT_LIB, RTLD_NOW | RTLD_LOCAL);
        if (vndr_fwk_lib_handle == NULL) {
            ALOGE("%s: vndr_fwk_lib_handle is null", __func__);
            return false;
        }
    }
    vndr_fwk_enhanced_info = (vndr_fwk_enhanced_info_t)
                dlsym(vndr_fwk_lib_handle, "getVendorEnhancedInfo");
    if (vndr_fwk_enhanced_info == NULL) {
        ALOGE("%s: vndr_fwk_enhanced_info symbol not found", __func__);
        return false;
    }

    vendor_info = vndr_fwk_enhanced_info();
    // 0: Pure AOSP for both system and odm;
    // 1: Pure AOSP for system and QC Value-adds for odm
    // 2: QC value-adds for system and Pure AOSP for odm
    // 3: QC value-adds for both system and odm.
    ALOGD("%s: vendor_info = %d", __func__, vendor_info);

    if(vendor_info != 1 && vendor_info != 3) {
        ALOGE("%s: Build is not running with QC Value-adds for odm ", __func__);
        return false;
    } else {
        ALOGD("%s: Build is running with QC Value-adds for odm ", __func__);
        return true;
    }
}

void bt_audio_pre_init_qti()
{
    pthread_mutex_lock(&audio_stream.lock);
    if(!is_value_adds_supported()) {
      ALOGE("%s: Not registering QC IBluetoothAudioProvidersFactory", __func__);
      pthread_mutex_unlock(&audio_stream.lock);
      return;
    }
    if(!hal_registered) {
      bool fail = android::hardware::registerPassthroughServiceImplementation
          <vendor::qti::hardware::bluetooth_audio::
          V2_0::IBluetoothAudioProvidersFactory>() != android::OK;
      if(fail) {
        ALOGE("%s: HIDL IBluetoothAudioProvidersFactory registration failed", __func__);
      } else {
        ALOGI("%s: IBluetoothAudioProvidersFactory registration succeded", __func__);
        hal_registered = true;
      }
    } else {
      ALOGE("%s: HIDL IBluetoothAudioProvidersFactory already registered", __func__);
    }
    pthread_mutex_unlock(&audio_stream.lock);
}

int audio_stream_open_qti()
{
    ALOGI("%s", __func__);
    a2dp_stream_common_init(&audio_stream);
    audio_stream.session_ready =
          BluetoothAudioSessionControl::IsSessionReady(audio_stream.sessionType);
    if (audio_stream.session_ready) {
        ALOGI("%s: success", __func__);
        return 1;
    }
    ALOGE("%s: failed", __func__);
    return 0;
}

int audio_stream_close_qti()
{
    ALOGI("%s", __func__);
    tA2DP_CTRL_ACK status;
    pthread_mutex_lock(&audio_stream.lock);
    if (audio_stream.state == AUDIO_A2DP_STATE_STARTED ||
        audio_stream.state == AUDIO_A2DP_STATE_STOPPING) {
        audio_stream.state = AUDIO_A2DP_STATE_STOPPED;
        auto ret = handle_hidl_request(A2DP_CTRL_CMD_SUSPEND, &status,
                                   max_waittime, max_waittime_pending_ack);
        if (ret != HIDL_REQUEST_STATUS_SUCCESS) {
           ALOGE("%s: hidl request failed ret:%d ",__func__, ret);
        }
    }
    btapoffload_port_deinit();
    pthread_mutex_unlock(&audio_stream.lock);
    ALOGW("%s: return", __func__);
    return 1;
}

int audio_stream_stop_qti()
{
    ALOGI("%s", __func__);
    int ret = 0;
    tA2DP_CTRL_ACK status = A2DP_CTRL_ACK_UNKNOWN;
    pthread_mutex_lock(&audio_stream.lock);
    if (audio_stream.state != AUDIO_A2DP_STATE_SUSPENDED) {
        auto ret = handle_hidl_request(A2DP_CTRL_CMD_STOP, &status,
                                   max_waittime, max_waittime_pending_ack);
        if (ret != HIDL_REQUEST_STATUS_SUCCESS) {
           ALOGE("%s: hidl request failed ret:%d ",__func__, ret);
        }
        if (status == A2DP_CTRL_ACK_SUCCESS) {
            ALOGD("%s: success", __func__);
            audio_stream.state = AUDIO_A2DP_STATE_STANDBY;
            pthread_mutex_unlock(&audio_stream.lock);
            return 0;
        } else {
            ALOGW("%s: failed", __func__);
            audio_stream.state = AUDIO_A2DP_STATE_STOPPED;
            pthread_mutex_unlock(&audio_stream.lock);
            return -1;
        }
    }
    audio_stream.state = AUDIO_A2DP_STATE_STOPPED;
    pthread_mutex_unlock(&audio_stream.lock);
    ALOGW("%s: return", __func__);
    return ret;
}

int audio_stream_suspend_qti()
{
    ALOGI("%s", __func__);
    tA2DP_CTRL_ACK status;

    pthread_mutex_lock(&audio_stream.lock);

    if (audio_stream.state != AUDIO_A2DP_STATE_SUSPENDED) {
        auto ret = handle_hidl_request(A2DP_CTRL_CMD_SUSPEND, &status,
                                   max_waittime, max_waittime_pending_ack);
        if (ret != HIDL_REQUEST_STATUS_SUCCESS) {
           ALOGE("%s: hidl request failed ret:%d ",__func__, ret);
        }
        if (status == A2DP_CTRL_ACK_SUCCESS) {
            ALOGD("%s: success", __func__);
            audio_stream.state = AUDIO_A2DP_STATE_SUSPENDED;
            pthread_mutex_unlock(&audio_stream.lock);
            return 0;
        } else {
            ALOGW("%s: failed", __func__);
            pthread_mutex_unlock(&audio_stream.lock);
            return -1;
        }
    }
    pthread_mutex_unlock(&audio_stream.lock);
    ALOGW("%s: return", __func__);
    return -1;
}

void audio_handoff_triggered_qti()
{
    ALOGI("%s: state = %s", __func__, dump_a2dp_hal_state(audio_stream.state));
    pthread_mutex_lock(&audio_stream.lock);
    if (audio_stream.state != AUDIO_A2DP_STATE_STOPPED ||
        audio_stream.state != AUDIO_A2DP_STATE_STOPPING) {
        audio_stream.state = AUDIO_A2DP_STATE_STOPPED;
    }
    pthread_mutex_unlock(&audio_stream.lock);
    ALOGW("%s: return", __func__);
}

void clear_a2dp_suspend_flag_qti()
{
    ALOGI("%s: state = %s", __func__, dump_a2dp_hal_state(audio_stream.state));
    pthread_mutex_lock(&audio_stream.lock);
    if (audio_stream.state == AUDIO_A2DP_STATE_SUSPENDED) {
        audio_stream.state = AUDIO_A2DP_STATE_STOPPED;
    }
    pthread_mutex_unlock(&audio_stream.lock);
    ALOGW("%s: return", __func__);
}

void * audio_get_codec_config_qti(uint8_t* /* multicast_status */, uint8_t* /* num_dev */,
                              audio_format_t *codec_type)
{
    ALOGI("%s: state = %s", __func__, dump_a2dp_hal_state(audio_stream.state));
    if (audio_stream.session_ready) {
      return (a2dp_codec_parser(&audio_stream.codec_cfg, codec_type, NULL));
    }
    ALOGW("%s: return", __func__);
    return NULL;
}

int audio_check_a2dp_ready_qti()
{
    ALOGI("%s: state = %s", __func__, dump_a2dp_hal_state(audio_stream.state));
    int status;
    pthread_mutex_lock(&audio_stream.lock);
    ALOGI("%s: session ready  = %d", __func__, audio_stream.session_ready);
    if (audio_stream.session_ready) {
      status = 1;
    } else {
      status = 0;
    }
    pthread_mutex_unlock(&audio_stream.lock);
    ALOGW("%s: return", __func__);
    return status;
}

uint16_t audio_get_a2dp_sink_latency_qti()
{
    struct timespec* data_position = { };
    uint64_t total_bytes_readed;
    uint64_t remoteDelay =0;
    pthread_mutex_lock(&audio_stream.lock);
    audio_stream.sink_latency = A2DP_DEFAULT_SINK_LATENCY;
    if (audio_stream.session_ready) {
      BluetoothA2dpControl *a2dpControl = BluetoothA2dpControl::getA2DPControl();
      if(a2dpControl) {
        a2dpControl->getSinkLatency(audio_stream.sessionType,&remoteDelay,
                 &total_bytes_readed, data_position);
        audio_stream.sink_latency = (remoteDelay/10);
        ALOGI("%s: Sink Latency = %d", __func__, audio_stream.sink_latency);
      }
    }
    pthread_mutex_unlock(&audio_stream.lock);
    return audio_stream.sink_latency;
}

void ldac_codec_parser(AudioConfiguration *codec_cfg)
{
    memset(&ldac_codec, 0, sizeof(audio_ldac_encoder_config_t));
    CodecConfiguration codecConfig =  codec_cfg->codecConfig;
    const LdacData ldacconfig =   codecConfig.config.ldacConfig;
    switch (ldacconfig.sampleRate) {
        case SampleRate::RATE_44100:
             ldac_codec.sampling_rate = 44100;
             break;
        case SampleRate::RATE_48000:
             ldac_codec.sampling_rate = 48000;
             break;
        case SampleRate::RATE_88200:
             ldac_codec.sampling_rate = 88200;
             break;
        case SampleRate::RATE_96000:
             ldac_codec.sampling_rate = 96000;
             break;
        case SampleRate::RATE_176400:
             ldac_codec.sampling_rate = 176400;
             break;
        case SampleRate::RATE_192000:
             ldac_codec.sampling_rate = 192000;
             break;
        default:
             ALOGE("%s: LDAC: unknown sampling rate:%d", __func__,
                   ldacconfig.sampleRate);
             break;
    }

    switch (ldacconfig.channelMode) {
        case LdacChannelMode::DUAL:
            ldac_codec.channel_mode = A2D_LDAC_CHANNEL_DUAL;
            break;
        case LdacChannelMode::STEREO:
            ldac_codec.channel_mode = A2D_LDAC_CHANNEL_STEREO;
            break;
        case LdacChannelMode::MONO:
            ldac_codec.channel_mode = A2D_LDAC_CHANNEL_MONO;
            break;
        default:
             ALOGE("%s: LDAC: unknown channel mode:%u", __func__,
                   (unsigned)ldacconfig.channelMode);
             break;
    }
    ALOGD("%s: LDAC: channel mode: %d", __func__,ldac_codec.channel_mode);
    ALOGD("%s: LDAC: codec config copied", __func__);
    uint16_t mtu = codecConfig.peerMtu;
    if ((mtu == 0) || (mtu > MAX_2MBPS_A2DP_STREAM_MTU))
        mtu = MAX_2MBPS_A2DP_STREAM_MTU;
    ldac_codec.mtu = mtu;

    if (codecConfig.encodedAudioBitrate == 0) {
        ALOGW("%s: LDAC: bitrate is zero", __func__);
        // ldac_codec.bitrate = ?
    } else if (codecConfig.encodedAudioBitrate >= 0x00000001 &&
               codecConfig.encodedAudioBitrate <= 0x00FFFFFF) {
        ldac_codec.bitrate = codecConfig.encodedAudioBitrate;
    }

    switch (ldacconfig.bitsPerSample) {
        case BitsPerSample::BITS_16:
            ldac_codec.bits_per_sample = 16;
            break;
        case BitsPerSample::BITS_24:
            ldac_codec.bits_per_sample = 24;
            break;
        case BitsPerSample::BITS_32:
            ldac_codec.bits_per_sample = 32;
            break;
        default:
            ALOGE("%s: LDAC: unknown bits per sample", __func__);
            break;
    }

    ALOGD("%s: LDAC: bits per sample:%d", __func__, ldac_codec.bits_per_sample);

    ldac_codec.is_abr_enabled = (ldacconfig.qualityIndex == LdacQualityIndex::QUALITY_ABR);

    ALOGD("%s: LDAC: create lookup for %d with ABR %d", __func__,
          ldacconfig.sampleRate, ldac_codec.is_abr_enabled);
    ALOGD("%s: LDAC: codec index value %u", __func__,
          (unsigned int)ldacconfig.qualityIndex);

    if (ldacconfig.sampleRate == SampleRate::RATE_44100 ||
        ldacconfig.sampleRate == SampleRate::RATE_88200) {
        int num_of_level_entries =
            sizeof(bit_rate_level_44_1k_88_2k_database) / sizeof(bit_rate_level_44_1k_88_2k_table_t);
        ldac_codec.level_to_bitrate_map.num_levels = num_of_level_entries;
        if (ldac_codec.is_abr_enabled) {
            ldac_codec.bitrate = bit_rate_level_44_1k_88_2k_database[0].bit_rate_value;
            ALOGI("%s: LDAC: send start highest bitrate value %d", __func__, ldac_codec.bitrate);
        }
        for (int i = 0; i < num_of_level_entries; i++) {
            ldac_codec.level_to_bitrate_map.bit_rate_level_map[i].link_quality_level =
                bit_rate_level_44_1k_88_2k_database[i].level_value;
            ldac_codec.level_to_bitrate_map.bit_rate_level_map[i].bitrate =
                bit_rate_level_44_1k_88_2k_database[i].bit_rate_value;
            ALOGI("%s: LDAC: level:%d bitrate:%d", __func__,
                  ldac_codec.level_to_bitrate_map.bit_rate_level_map[i].link_quality_level,
                  ldac_codec.level_to_bitrate_map.bit_rate_level_map[i].bitrate);
        }
    } else if (ldacconfig.sampleRate == SampleRate::RATE_48000 ||
               ldacconfig.sampleRate == SampleRate::RATE_96000) {
        int num_of_level_entries =
            sizeof(bit_rate_level_48k_96k_database) / sizeof(bit_rate_level_48k_96k_table_t);
        ldac_codec.level_to_bitrate_map.num_levels = num_of_level_entries;
        if (ldac_codec.is_abr_enabled) {
            ldac_codec.bitrate = bit_rate_level_48k_96k_database[0].bit_rate_value;
            ALOGI("%s: LDAC: send start highest bitrate value %d", __func__, ldac_codec.bitrate);
        }
        for (int i = 0; i < num_of_level_entries; i++) {
            ldac_codec.level_to_bitrate_map.bit_rate_level_map[i].link_quality_level =
                bit_rate_level_48k_96k_database[i].level_value;
            ldac_codec.level_to_bitrate_map.bit_rate_level_map[i].bitrate =
                bit_rate_level_48k_96k_database[i].bit_rate_value;
            ALOGI("%s: LDAC: level:%d bitrate:%d", __func__,
                  ldac_codec.level_to_bitrate_map.bit_rate_level_map[i].link_quality_level,
                  ldac_codec.level_to_bitrate_map.bit_rate_level_map[i].bitrate);
        }
    } else {
        ALOGW("%s: LDAC: unsupported sample rate", __func__);
    }

    ALOGD("%s: LDAC: bitrate: %u", __func__, ldac_codec.bitrate);
    ALOGD("%s: LDAC: done copying full codec config", __func__);
}

bool audio_is_scrambling_enabled_qti(void)
{
    bool isScrambling;
    pthread_mutex_lock(&audio_stream.ack_lock);
    isScrambling = audio_stream.codec_cfg.codecConfig.isScramblingEnabled;
    pthread_mutex_unlock(&audio_stream.ack_lock);

    audio_format_t codec_type = AUDIO_FORMAT_INVALID;
    ALOGW("%s: isSession_ready =  %d isScrambling = %d",__func__, audio_stream.session_ready, isScrambling);
    pthread_mutex_lock(&audio_stream.lock);
    if (audio_stream.session_ready) {
      a2dp_codec_parser(&audio_stream.codec_cfg, &codec_type, NULL);
    }
    ALOGW("%s: codec_type = %d",__func__, codec_type);
    if (codec_type == ENC_CODEC_TYPE_APTX_DUAL_MONO) {
      ALOGW("%s:TWSP codec, return false",__func__);
      pthread_mutex_unlock(&audio_stream.lock);
      return false;
    }
    if (codec_type == ENC_CODEC_TYPE_APTX_ADAPTIVE) {
      ALOGW("%s:aptX Adaptive codec, return false",__func__);
      pthread_mutex_unlock(&audio_stream.lock);
      return false;
    }
    if (codec_type == AUDIO_CODEC_TYPE_CELT) {
      ALOGW("%s:Celt codec, return false",__func__);
      pthread_mutex_unlock(&audio_stream.lock);
      return false;
    }
    pthread_mutex_unlock(&audio_stream.lock);
    ALOGW("%s: return", __func__);
    return isScrambling;
}

}  // namespace btoffload
}  // qti
}  // vendor
