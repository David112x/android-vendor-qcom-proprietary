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
#include "btaudio_offload.h"
#include "btaudio_offload_qti.h"
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

#include "BluetoothAudioSessionControl.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "btaudio_offload"

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

/*****************************************************************************
**  Local type definitions
******************************************************************************/

struct a2dp_stream_common audio_stream;
static volatile unsigned char ack_recvd = 0;
pthread_cond_t ack_cond = PTHREAD_COND_INITIALIZER;
int  max_waittime = 4500; // in ms
int wait_for_stack_response(int time_to_wait);
bool qti_audio_hal = true;

using PcmConfiguration = ::android::hardware::bluetooth::audio::V2_0::PcmParameters;
using CodecConfiguration = ::android::hardware::bluetooth::audio::V2_0::CodecConfiguration;
using LdacData = ::android::hardware::bluetooth::audio::V2_0::LdacParameters;

using ::android::hardware::bluetooth::audio::V2_0::SampleRate;
using ::android::hardware::bluetooth::audio::V2_0::BitsPerSample;
using ::android::hardware::bluetooth::audio::V2_0::ChannelMode;
using ::android::hardware::bluetooth::audio::V2_0::LdacChannelMode;
using ::android::hardware::bluetooth::audio::V2_0::SbcAllocMethod;
using ::android::hardware::bluetooth::audio::V2_0::SbcBlockLength;
using ::android::hardware::bluetooth::audio::V2_0::SbcChannelMode;
using ::android::hardware::bluetooth::audio::V2_0::SbcNumSubbands;
using ::android::hardware::bluetooth::audio::V2_0::CodecType;
using ::android::hardware::bluetooth::audio::V2_0::SessionType;
using ::android::hardware::bluetooth::audio::V2_0::IBluetoothAudioPort;
using ::android::hardware::bluetooth::audio::V2_0::AudioConfiguration;
using ::android::hardware::bluetooth::audio::V2_0::LdacQualityIndex;

using ::android::bluetooth::audio::BluetoothAudioSessionControl;


/*****************************************************************************
**  Static functions
******************************************************************************/

audio_sbc_encoder_config_t sbc_codec;
audio_aptx_encoder_config_t aptx_codec;
audio_aac_encoder_config_t aac_codec;
audio_ldac_encoder_config_t ldac_codec;
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
        CASE_RETURN_STR(A2DP_CTRL_ACK_DISCONNECT_IN_PROGRESS)
        CASE_RETURN_STR(A2DP_CTRL_SKT_DISCONNECTED)
        CASE_RETURN_STR(A2DP_CTRL_ACK_UNSUPPORTED)
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
    CodecConfiguration codecConfig =  codec_cfg->codecConfig();
    ALOGI("%s: codec_type = %x", __func__, codecConfig.codecType);
    if (codecConfig.codecType == CodecType::SBC) {
        memset(&sbc_codec, 0, sizeof(audio_sbc_encoder_config_t));

        switch (codecConfig.config.sbcConfig().sampleRate) {
            case SampleRate::RATE_48000:
                 sbc_codec.sampling_rate = 48000;
                 break;
            case SampleRate::RATE_44100:
                 sbc_codec.sampling_rate = 44100;
                 break;
            default:
                 ALOGE("%s: SBC: unknown sampling rate:%d", __func__,
                       codecConfig.config.sbcConfig().sampleRate);
                 break;
        }

        switch (codecConfig.config.sbcConfig().channelMode) {
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
                       (unsigned)codecConfig.config.sbcConfig().channelMode);
                 break;
        }

        switch (codecConfig.config.sbcConfig().blockLength) {
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
                      (unsigned int)codecConfig.config.sbcConfig().blockLength);
                break;
        }

        switch (codecConfig.config.sbcConfig().numSubbands) {
            case SbcNumSubbands::SUBBAND_8:
                sbc_codec.subband = 8;
                break;
            case SbcNumSubbands::SUBBAND_4:
                sbc_codec.subband = 4;
                break;
            default:
                ALOGE("%s: SBC: unknown subband:%u", __func__,
                      (unsigned int)codecConfig.config.sbcConfig().numSubbands);
                break;
        }
        switch (codecConfig.config.sbcConfig().allocMethod) {
            case SbcAllocMethod::ALLOC_MD_L:
                sbc_codec.alloc = 1;
                break;
            case SbcAllocMethod::ALLOC_MD_S:
                sbc_codec.alloc = 2;
                break;
            default:
                ALOGE("%s: SBC: unknown alloc method:%u", __func__,
                      (unsigned int)codecConfig.config.sbcConfig().allocMethod);
                break;
        }
        switch (codecConfig.config.sbcConfig().bitsPerSample) {
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

        sbc_codec.min_bitpool = codecConfig.config.sbcConfig().minBitpool;
        sbc_codec.max_bitpool = codecConfig.config.sbcConfig().maxBitpool;
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
        switch (codecConfig.config.aacConfig().sampleRate) {
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
                      codecConfig.config.aacConfig().sampleRate);
                break;
        }
        switch (codecConfig.config.aacConfig().channelMode) {
            case ChannelMode::MONO:
                 aac_codec.channels = 1;
                 break;
            case ChannelMode::STEREO:
                 aac_codec.channels = 2;
                 break;
            default:
                 ALOGE("%s: AAC: unknown channel mode:%u", __func__,
                       (unsigned)codecConfig.config.aacConfig().channelMode);
                 break;
        }

        /*switch (byte & A2D_AAC_IE_VBR_MSK) {
            case A2D_AAC_IE_VBR:
                break;
            default:
                ALOGE("%s: AAC: VBR not supported", __func__);
                break;
        }*/

        switch (codecConfig.config.aacConfig().bitsPerSample) {
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
        aac_codec.bitrate = (mtu - AAC_LATM_HEADER) * (8 * aac_codec.sampling_rate/AAC_SAMPLE_SIZE);
        // Make sure bitrate is within the limit negotiated by stack
        if (aac_codec.bitrate > bitrate_hal)
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
        switch (codecConfig.config.aptxConfig().sampleRate) {
            case SampleRate::RATE_48000:
                 aptx_codec.sampling_rate = 48000;
                 break;
            case SampleRate::RATE_44100:
                 aptx_codec.sampling_rate = 44100;
                 break;
            default:
                 ALOGE("%s: APTX: unknown sampling rate:%d", __func__,
                       codecConfig.config.aptxConfig().sampleRate);
                 break;
        }
        switch (codecConfig.config.aptxConfig().channelMode) {
            case ChannelMode::STEREO:
                 aptx_codec.channels = 2;
                 break;
            case ChannelMode::MONO:
                 aptx_codec.channels = 1;
                 break;
            default:
                 ALOGE("%s: APTX: unknown channel mode:%u", __func__,
                       (unsigned)codecConfig.config.aptxConfig().channelMode);
                 break;
        }
        switch (codecConfig.config.aptxConfig().bitsPerSample) {
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
    }
    return NULL;
}

/*****************************************************************************
**
** AUDIO DATA PATH
**
*****************************************************************************/

void stack_resp_cb (const uint16_t& cntrl_key __unused, const bool& start_resp,
                     const BluetoothAudioStatus&  status) {

    ALOGI("%s: status = %hhu", __func__, status);
    pthread_mutex_lock(&audio_stream.ack_lock);
    audio_stream.ack_status = (tA2DP_CTRL_ACK)(status);
    if (!ack_recvd) {
        ack_recvd = 1;
        pthread_cond_signal(&ack_cond);
    }
    pthread_mutex_unlock(&audio_stream.ack_lock);
}

void session_resp_cb (const uint16_t& cntrl_key)
{
    ALOGI("%s", __func__);
    pthread_mutex_lock(&audio_stream.ack_lock);
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
    ::android::bluetooth::audio::PortStatusCallbacks cbacks = {
      .control_result_cb_ = audio_stream.control_result_cb,
      .session_changed_cb_ = audio_stream.session_changed_cb};
      audio_stream.ctrl_key = BluetoothAudioSessionControl::RegisterControlResultCback(
         audio_stream.sessionType, cbacks);
}

static void btapoffload_port_deinit() {
   ALOGI("%s start", __func__);
   BluetoothAudioSessionControl::UnregisterControlResultCback(audio_stream.sessionType,audio_stream.ctrl_key);
}
void a2dp_stream_common_init(struct a2dp_stream_common *common)
{
    pthread_mutexattr_t lock_attr;
    ALOGI("%s", __func__);

    pthread_mutexattr_init(&lock_attr);
    pthread_mutexattr_settype(&lock_attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&common->lock, &lock_attr);
    pthread_mutexattr_destroy(&lock_attr);
    memset(common,0,sizeof(a2dp_stream_common));
    common->sink_latency = A2DP_DEFAULT_SINK_LATENCY;
    common->sessionType = SessionType::A2DP_HARDWARE_OFFLOAD_DATAPATH;
    btapoffload_port_init();
}

int wait_for_stack_response(int time_to_wait)
{
    ALOGD("%s", __func__);
    struct timespec now;
    int ret = 0;
    pthread_mutex_lock(&audio_stream.ack_lock);
    if (audio_stream.session_ready== 0) {
        ALOGE("%s: stack deinitialized", __func__);
        pthread_mutex_unlock(&audio_stream.ack_lock);
        return ret;
    }
    ALOGW("%s: entering conditional wait: ack_recvd = %d",
              __func__,ack_recvd);
    clock_gettime(CLOCK_REALTIME, &now);
    now.tv_sec += (time_to_wait / 1000);
    if (!ack_recvd) {
       pthread_cond_timedwait(&ack_cond, &audio_stream.ack_lock, &now);
    }
    if (ack_recvd) {
        ALOGV("%s: ack received", __func__);
    }
    else {
        clock_gettime(CLOCK_REALTIME, &now);
        now.tv_nsec += ((time_to_wait % 1000) * 1000000);
        //ALOGW("%s: entering conditional wait for ms = %l",
          //      __func__,now.tv_nsec);
        pthread_cond_timedwait(&ack_cond, &audio_stream.ack_lock, &now);
    }
    pthread_mutex_unlock(&audio_stream.ack_lock);
    return ret;
}

int audio_start_stream()
{
  return audio_stream_start();
}

int audio_stream_start()
{
    if(qti_audio_hal) return vendor::qti::btoffload::audio_stream_start_qti();
    int ack_ret;
    tA2DP_CTRL_ACK status = A2DP_CTRL_ACK_SUCCESS;
    ALOGD("%s: state = %s", __func__, dump_a2dp_hal_state(audio_stream.state));
    pthread_mutex_lock(&audio_stream.lock);
    if (audio_stream.state == AUDIO_A2DP_STATE_SUSPENDED) {
        ALOGW("%s: stream suspended", __func__);
        pthread_mutex_unlock(&audio_stream.lock);
        return -1;
    }
    if (audio_stream.session_ready) {
       audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
       auto ret = BluetoothAudioSessionControl::StartStream(audio_stream.sessionType);
       if (!ret) {
          ALOGE("%s: client has died",__func__);
          goto end;
       }
       ack_recvd = 0;
       audio_stream.state = AUDIO_A2DP_STATE_STARTING;
       pthread_mutex_lock(&audio_stream.ack_lock);
       status = audio_stream.ack_status;
       pthread_mutex_unlock(&audio_stream.ack_lock);
       if (status == A2DP_CTRL_ACK_UNKNOWN) {
          ack_ret = wait_for_stack_response(max_waittime);
          pthread_mutex_lock(&audio_stream.ack_lock);
          status = audio_stream.ack_status;
          ALOGD("%s: status = %s", __func__, dump_a2dp_ctrl_ack(status));
       }
       if (status == A2DP_CTRL_ACK_SUCCESS) {
          ALOGI("%s: a2dp stream started successfully", __func__);
          audio_stream.state = AUDIO_A2DP_STATE_STARTED;
          pthread_mutex_unlock(&audio_stream.ack_lock);
          goto end;
       }
       if (status == A2DP_CTRL_ACK_FAILURE  ||
             status == A2DP_CTRL_ACK_INCALL_FAILURE ||
             status == A2DP_CTRL_ACK_UNSUPPORTED ||
             status == A2DP_CTRL_ACK_DISCONNECT_IN_PROGRESS ||
             status == A2DP_CTRL_ACK_UNKNOWN) {
          ALOGW("%s: a2dp stream start failed: status = %s", __func__,
                  dump_a2dp_ctrl_ack(status));
          pthread_mutex_unlock(&audio_stream.ack_lock);
          audio_stream.state = AUDIO_A2DP_STATE_STANDBY;
          goto end;
       }
    }
end:
    pthread_mutex_unlock(&audio_stream.lock);
    return status;
}

int audio_stream_open()
{
    int vnd_session = vendor::qti::btoffload::audio_stream_open_qti();
    if(vnd_session) {
        qti_audio_hal = true;
        return 0;
    }
    ALOGI("%s", __func__);
    a2dp_stream_common_init(&audio_stream);
    audio_stream.session_ready =
          BluetoothAudioSessionControl::IsSessionReady(audio_stream.sessionType);
    if (audio_stream.session_ready) {
        ALOGI("%s: success", __func__);
        qti_audio_hal = false;
        return 0;
    }
    ALOGE("%s: failed", __func__);
    return 0;
}

int audio_stream_close()
{
    if(qti_audio_hal) return vendor::qti::btoffload::audio_stream_close_qti();
    ALOGI("%s", __func__);
    tA2DP_CTRL_ACK status = A2DP_CTRL_ACK_UNKNOWN;
    pthread_mutex_lock(&audio_stream.lock);
    if (audio_stream.state == AUDIO_A2DP_STATE_STARTED ||
        audio_stream.state == AUDIO_A2DP_STATE_STOPPING) {
        audio_stream.state = AUDIO_A2DP_STATE_STOPPED;
        if (audio_stream.session_ready) {
            audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
            ack_recvd = 0;
            ALOGW("%s: suspending audio stream", __func__);
            auto ret = BluetoothAudioSessionControl::SuspendStream(audio_stream.sessionType);
            if (ret == false) {
                ALOGE("%s: client has died",__func__);
                pthread_mutex_unlock(&audio_stream.lock);
                return -1;
            }
            pthread_mutex_lock(&audio_stream.ack_lock);
            status = audio_stream.ack_status;
            pthread_mutex_unlock(&audio_stream.ack_lock);
            ALOGI("%s: ack status = %s", __func__, dump_a2dp_ctrl_ack(status));
            if (status == A2DP_CTRL_ACK_UNKNOWN) {
                wait_for_stack_response(max_waittime);
                pthread_mutex_lock(&audio_stream.ack_lock);
                status = audio_stream.ack_status;
                pthread_mutex_unlock(&audio_stream.ack_lock);
            }
        }
    }
    btapoffload_port_deinit();
    pthread_mutex_unlock(&audio_stream.lock);
    if (status == A2DP_CTRL_ACK_UNKNOWN) {
        ALOGE("%s: failed to get ack from stack", __func__);
        return -1;
    } else {
      ALOGI("%s: return success", __func__);
      return 1;
    }
}

int audio_stop_stream()
{
  return audio_stream_stop();
}

int audio_stream_stop()
{
    if(qti_audio_hal) return vendor::qti::btoffload::audio_stream_stop_qti();
    ALOGI("%s", __func__);
    int ret = 0;
    tA2DP_CTRL_ACK status;
    pthread_mutex_lock(&audio_stream.lock);
    if (audio_stream.session_ready) {
        if (audio_stream.state != AUDIO_A2DP_STATE_SUSPENDED) {
            ack_recvd = 0;
            audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
            auto ret = BluetoothAudioSessionControl::SuspendStream(audio_stream.sessionType);
            if (ret == false) {
                ALOGE("%s: client has died",__func__);
                pthread_mutex_unlock(&audio_stream.lock);
                return -1;
            }
            pthread_mutex_lock(&audio_stream.ack_lock);
            status = audio_stream.ack_status;
            pthread_mutex_unlock(&audio_stream.ack_lock);
            ALOGI("%s: ack status = %s", __func__, dump_a2dp_ctrl_ack(status));
            if (status == A2DP_CTRL_ACK_UNKNOWN) {
                audio_stream.state = AUDIO_A2DP_STATE_STOPPING;
                wait_for_stack_response(max_waittime);
                pthread_mutex_lock(&audio_stream.ack_lock);
                status = audio_stream.ack_status;
                pthread_mutex_unlock(&audio_stream.ack_lock);
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
    } else {
       ret = -1;
       ALOGW("%s: stack is down", __func__);
    }
    audio_stream.state = AUDIO_A2DP_STATE_STOPPED;
    pthread_mutex_unlock(&audio_stream.lock);
    return ret;
}

int audio_suspend_stream()
{
  return audio_stream_suspend();
}


int audio_stream_suspend()
{
    if(qti_audio_hal) return vendor::qti::btoffload::audio_stream_suspend_qti();
    ALOGI("%s", __func__);
    tA2DP_CTRL_ACK status;

    pthread_mutex_lock(&audio_stream.lock);
    if (audio_stream.session_ready) {
        if (audio_stream.state != AUDIO_A2DP_STATE_SUSPENDED) {
            ack_recvd = 0;
            audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
            auto ret = BluetoothAudioSessionControl::SuspendStream(audio_stream.sessionType);
            if (ret == false) {
                ALOGE("%s: client has died",__func__);
                pthread_mutex_unlock(&audio_stream.lock);
                return -1;
            }
            pthread_mutex_lock(&audio_stream.ack_lock);
            status = audio_stream.ack_status;
            pthread_mutex_unlock(&audio_stream.ack_lock);
            ALOGW("%s: ack status = %s", __func__, dump_a2dp_ctrl_ack(status));
            if (status == A2DP_CTRL_ACK_UNKNOWN) {
                wait_for_stack_response(max_waittime);
                pthread_mutex_lock(&audio_stream.ack_lock);
                status = audio_stream.ack_status;
                pthread_mutex_unlock(&audio_stream.ack_lock);
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
    } else {
        ALOGW("%s: stack is down", __func__);
    }
    pthread_mutex_unlock(&audio_stream.lock);
    return -1;
}

void audio_handoff_triggered()
{
    if(qti_audio_hal) {
        vendor::qti::btoffload::audio_handoff_triggered_qti();
        return;
    }
    ALOGI("%s: state = %s", __func__, dump_a2dp_hal_state(audio_stream.state));
    pthread_mutex_lock(&audio_stream.lock);
    if (audio_stream.state != AUDIO_A2DP_STATE_STOPPED ||
        audio_stream.state != AUDIO_A2DP_STATE_STOPPING) {
        audio_stream.state = AUDIO_A2DP_STATE_STOPPED;
    }
    pthread_mutex_unlock(&audio_stream.lock);
}

void clear_a2dpsuspend_flag()
{
    if(qti_audio_hal) {
        vendor::qti::btoffload::clear_a2dp_suspend_flag_qti();
        return;
    }
    ALOGI("%s: state = %s", __func__, dump_a2dp_hal_state(audio_stream.state));
    pthread_mutex_lock(&audio_stream.lock);
    if (audio_stream.state == AUDIO_A2DP_STATE_SUSPENDED) {
        audio_stream.state = AUDIO_A2DP_STATE_STOPPED;
    }
    pthread_mutex_unlock(&audio_stream.lock);
}

void * audio_get_codec_config(uint8_t* multicast_status, uint8_t* num_dev,
                              audio_format_t *codec_type)
{
    if(qti_audio_hal) {
        return vendor::qti::btoffload::audio_get_codec_config_qti(multicast_status, num_dev, codec_type);
    }
    ALOGI("%s: state = %s", __func__, dump_a2dp_hal_state(audio_stream.state));
    if (audio_stream.session_ready) {
      return (a2dp_codec_parser(&audio_stream.codec_cfg, codec_type, NULL));
    }
    return NULL;
}

int audio_check_a2dp_ready()
{
    if(qti_audio_hal) return vendor::qti::btoffload::audio_check_a2dp_ready_qti();
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
    return status;
}

uint16_t audio_get_a2dp_sink_latency()
{
    if(qti_audio_hal) return vendor::qti::btoffload::audio_get_a2dp_sink_latency_qti();
    struct timespec* data_position = { };
    uint64_t total_bytes_readed;
    uint64_t remote_delay_ns =0;
    pthread_mutex_lock(&audio_stream.lock);
    if (audio_stream.session_ready) {
        BluetoothAudioSessionControl::GetPresentationPosition
          (audio_stream.sessionType,&remote_delay_ns, &total_bytes_readed,
             data_position);
        audio_stream.sink_latency = (remote_delay_ns/1000000);
    } else {
        audio_stream.sink_latency = A2DP_DEFAULT_SINK_LATENCY;
    }
    pthread_mutex_unlock(&audio_stream.lock);
    return audio_stream.sink_latency;
}

uint16_t audio_sink_get_a2dp_latency()
{
    return  audio_get_a2dp_sink_latency();
}

void ldac_codec_parser(AudioConfiguration *codec_cfg)
{
    memset(&ldac_codec, 0, sizeof(audio_ldac_encoder_config_t));
    CodecConfiguration codecConfig =  codec_cfg->codecConfig();
    const LdacData ldacconfig =   codecConfig.config.ldacConfig();
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

bool audio_is_scrambling_enabled(void)
{
    if(qti_audio_hal)
        return vendor::qti::btoffload::audio_is_scrambling_enabled_qti();

    return true;
}
void bt_audio_pre_init() {
    vendor::qti::btoffload::bt_audio_pre_init_qti();
}
