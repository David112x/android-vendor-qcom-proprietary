/****************************************************************************
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
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
/*      bthost_ipc.c
 *
 *  Description:   Implements IPC interface between HAL and BT host
 *
 *****************************************************************************/
#include <time.h>
#include <unistd.h>

#include "ldac_level_bit_rate_lookup.h"
#include "bthost_ipc.h"
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

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "bthost_ipc"

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
bt_lib_callback_t *stack_cb = NULL;
static volatile unsigned char ack_recvd = 0;
pthread_cond_t ack_cond = PTHREAD_COND_INITIALIZER;
static bool update_initial_sink_latency = false;
int wait_for_stack_response(uint8_t time_to_wait);
bool resp_received = false;
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
void ldac_codec_parser(CodecConfiguration *codec_cfg);
/*****************************************************************************
**   Miscellaneous helper functions
******************************************************************************/
static const char* dump_a2dp_ctrl_event(tA2DP_CTRL_CMD cmd)
{
    switch (cmd) {
        CASE_RETURN_STR(A2DP_CTRL_CMD_NONE)
        CASE_RETURN_STR(A2DP_CTRL_CMD_CHECK_READY)
        CASE_RETURN_STR(A2DP_CTRL_CMD_CHECK_STREAM_STARTED)
        CASE_RETURN_STR(A2DP_CTRL_CMD_START)
        CASE_RETURN_STR(A2DP_CTRL_CMD_STOP)
        CASE_RETURN_STR(A2DP_CTRL_CMD_SUSPEND)
        CASE_RETURN_STR(A2DP_CTRL_GET_AUDIO_CONFIG)
        CASE_RETURN_STR(A2DP_CTRL_CMD_OFFLOAD_START)
        CASE_RETURN_STR(A2DP_CTRL_CMD_OFFLOAD_SUPPORTED)
        CASE_RETURN_STR(A2DP_CTRL_CMD_OFFLOAD_NOT_SUPPORTED)
    }
    return "UNKNOWN A2DP_CTRL_CMD";
}

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
typedef enum {
    BT_SOC_DEFAULT = 0,
    BT_SOC_SMD = BT_SOC_DEFAULT,
    BT_SOC_AR3K,
    BT_SOC_ROME,
    BT_SOC_CHEROKEE,
    BT_SOC_NAPIER,
    BT_SOC_HASTINGS,
    BT_SOC_GENOA,
    /* Add chipset type here */
    BT_SOC_RESERVED
} bt_soc_type;

static int get_bt_soc_type(void)
{
    int ret = 0;
    char bt_soc_type[PROPERTY_VALUE_MAX];

    ALOGI("bt-hci: get_bt_soc_type");
    ret = property_get("vendor.qcom.bluetooth.soc", bt_soc_type, NULL);
    ALOGI("vendor.qcom.bluetooth.soc set to %s\n", bt_soc_type);
    if (!strncasecmp(bt_soc_type, "rome", sizeof("rome"))) {
        return BT_SOC_ROME;
    }
    else if (!strncasecmp(bt_soc_type, "cherokee", sizeof("cherokee"))) {
        return BT_SOC_CHEROKEE;
    }
    else if (!strncasecmp(bt_soc_type, "pronto", sizeof("pronto"))) {
        return BT_SOC_DEFAULT;
    }
    else if (!strncasecmp(bt_soc_type, "ath3k", sizeof("ath3k"))) {
        return BT_SOC_AR3K;
    }
    else if (!strncasecmp(bt_soc_type, "napier", sizeof("napier"))) {
        return BT_SOC_NAPIER;
    }
    else if (!strncasecmp(bt_soc_type, "hastings", sizeof("hastings"))) {
        return BT_SOC_HASTINGS;
    }
    else if (!strncasecmp(bt_soc_type, "genoa", sizeof("genoa"))) {
        return BT_SOC_GENOA;
    }
    else {
        ALOGI("vendor.qcom.bluetooth.soc not set, so using default.\n");
        return BT_SOC_DEFAULT;
    }
    return ret;
}
static void* a2dp_codec_parser(CodecConfiguration *codec_cfg,
                               audio_format_t *codec_type,
                               uint32_t *sample_freq)
{
    CodecConfiguration *p_cfg = codec_cfg;
    ALOGI("%s: codec_type = %x", __func__, codec_cfg->codecType);
    if (get_bt_soc_type() == BT_SOC_SMD) {
        *codec_type = AUDIO_FORMAT_PCM_16_BIT;
        return NULL;
    }
    if (codec_cfg->codecType == CodecType::SBC) {
        memset(&sbc_codec, 0, sizeof(audio_sbc_encoder_config_t));

        switch (codec_cfg->sampleRate) {
            case SampleRate::RATE_48000:
                 sbc_codec.sampling_rate = 48000;
                 break;
            case SampleRate::RATE_44100:
                 sbc_codec.sampling_rate = 44100;
                 break;
            default:
                 ALOGE("%s: SBC: unknown sampling rate:%d", __func__,
                       codec_cfg->sampleRate);
                 break;
        }

        switch (codec_cfg->channelMode) {
            case ChannelMode::STEREO:
                 // Use JOINT STEREO instead of STEREO as default
                 sbc_codec.channels = A2D_SBC_CHANNEL_JOINT_STEREO;
                 break;
            case ChannelMode::MONO:
                 sbc_codec.channels = A2D_SBC_CHANNEL_MONO;
                 break;
            default:
                 ALOGE("%s: SBC: unknown channel mode:%d", __func__,
                       codec_cfg->channelMode);
                 break;
        }

        switch ((codec_cfg->codecSpecific.sbcData.codecParameters & A2D_SBC_BLK_MASK)) {
            case A2D_SBC_BLOCKS_16:
                sbc_codec.blk_len = 16;
                break;
            case A2D_SBC_BLOCKS_12:
                sbc_codec.blk_len = 12;
                break;
            case A2D_SBC_BLOCKS_8:
                sbc_codec.blk_len = 8;
                break;
            case A2D_SBC_BLOCKS_4:
                sbc_codec.blk_len = 4;
                break;
            default:
                ALOGE("%s: SBC: unknown block length:%d", __func__,
                      codec_cfg->codecSpecific.sbcData.codecParameters & A2D_SBC_BLK_MASK);
                break;
        }

        switch ((codec_cfg->codecSpecific.sbcData.codecParameters & A2D_SBC_SUBBAND_MASK)) {
            case A2D_SBC_SUBBAND_8:
                sbc_codec.subband = 8;
                break;
            case A2D_SBC_SUBBAND_4:
                sbc_codec.subband = 4;
                break;
            default:
                ALOGE("%s: SBC: unknown subband:%d", __func__,
                      codec_cfg->codecSpecific.sbcData.codecParameters &
                      A2D_SBC_SUBBAND_MASK);
                break;
        }

        switch ((codec_cfg->codecSpecific.sbcData.codecParameters & A2D_SBC_ALLOC_MASK)) {
            case A2D_SBC_ALLOC_MD_L:
                sbc_codec.alloc = 1;
                break;
            case A2D_SBC_ALLOC_MD_S:
                sbc_codec.alloc = 2;
                break;
            default:
                ALOGE("%s: SBC: unknown alloc method:%d", __func__,
                      codec_cfg->codecSpecific.sbcData.codecParameters &
                      A2D_SBC_ALLOC_MASK);
                break;
        }

        sbc_codec.min_bitpool = codec_cfg->codecSpecific.sbcData.minBitpool;
        sbc_codec.max_bitpool = codec_cfg->codecSpecific.sbcData.maxBitpool;
        switch (codec_cfg->bitsPerSample)
        {
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
                 ALOGE("SBC: Unknown bits per sample");
        }

        ALOGD("%s: SBC: copied codec config: min_bitpool:%d max_bitpool:%d",
              __func__, sbc_codec.min_bitpool, sbc_codec.max_bitpool);

        if (codec_cfg->encodedAudioBitrate == 0) {
            ALOGW("%s: SBC: bitrate is zero", __func__);
            sbc_codec.bitrate = 328000;
        } else if (codec_cfg->encodedAudioBitrate >= 0x00000001 &&
                   codec_cfg->encodedAudioBitrate <= 0x00FFFFFF) {
            sbc_codec.bitrate = codec_cfg->encodedAudioBitrate;
        }

        ALOGI("%s: SBC: bitrate:%d i/p bitrate:%d", __func__, sbc_codec.bitrate,
              codec_cfg->encodedAudioBitrate);
        *codec_type = AUDIO_FORMAT_SBC;

        if (sample_freq) *sample_freq = sbc_codec.sampling_rate;

        ALOGI("%s: SBC: done copying full codec config", __func__);
        return ((void *)(&sbc_codec));

    } else if (codec_cfg->codecType == CodecType::AAC) {
        memset(&aac_codec, 0, sizeof(audio_aac_encoder_config_t));

        /*switch (byte & A2D_AAC_IE_OBJ_TYPE_MSK) {
            case A2D_AAC_IE_OBJ_TYPE_MPEG_2_AAC_LC:
                aac_codec.enc_mode = AUDIO_FORMAT_AAC_SUB_LC;
                break;
            case A2D_AAC_IE_OBJ_TYPE_MPEG_4_AAC_LC:
                aac_codec.enc_mode = AUDIO_FORMAT_AAC_SUB_LC;
                break;
            case A2D_AAC_IE_OBJ_TYPE_MPEG_4_AAC_LTP:
                aac_codec.enc_mode = AUDIO_FORMAT_AAC_SUB_LTP;
                break;
            case A2D_AAC_IE_OBJ_TYPE_MPEG_4_AAC_SCA:
                aac_codec.enc_mode = AUDIO_FORMAT_AAC_SUB_SCALABLE;
                break;
            default:
                ALOGE("%s: AAC: unknown encoder mode", __func__);
                break;
        }*/

        //USE 0 (AAC_LC) as hardcoded value till Audio
        //define constants
        aac_codec.enc_mode = 0;

        //USE LOAS(1) or LATM(4) hardcoded values till
        //Audio define proper constants
        aac_codec.format_flag = 4;

        switch (codec_cfg->sampleRate) {
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
                      codec_cfg->sampleRate);
                break;
        }

        switch (codec_cfg->channelMode) {
            case ChannelMode::MONO:
                 aac_codec.channels = 1;
                 break;
            case ChannelMode::STEREO:
                 aac_codec.channels = 2;
                 break;
            default:
                 ALOGE("%s: AAC: unknown channel mode:%d", __func__,
                       codec_cfg->channelMode);
                 break;
        }

        /*switch (byte & A2D_AAC_IE_VBR_MSK) {
            case A2D_AAC_IE_VBR:
                break;
            default:
                ALOGE("%s: AAC: VBR not supported", __func__);
                break;
        }*/

        switch (codec_cfg->bitsPerSample) {
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
                ALOGE("%s: AAC: Unknown bits per sample", __func__);
                break;
        }

        uint32_t bitrate_hal = 0;
        if (codec_cfg->encodedAudioBitrate == 0) {
            ALOGW("%s: AAC: bitrate is zero", __func__);
            // aac_codec.bitrate = ?
        } else if (codec_cfg->encodedAudioBitrate >= 0x00000001 &&
                   codec_cfg->encodedAudioBitrate <= 0x00FFFFFF) {
            bitrate_hal = codec_cfg->encodedAudioBitrate;
        }

        *codec_type = AUDIO_FORMAT_AAC;
        ALOGI("%s: AAC HAL: bitrate:%lu peermtu:%lu", __func__,
              bitrate_hal, codec_cfg->peerMtu);
        uint16_t mtu = codec_cfg->peerMtu;
        if ((mtu == 0) || (mtu > MAX_3MBPS_A2DP_STREAM_MTU))
            mtu = MAX_3MBPS_A2DP_STREAM_MTU;
        aac_codec.bitrate = (mtu - AAC_LATM_HEADER) * (8 * aac_codec.sampling_rate / AAC_SAMPLE_SIZE);
        // Make sure bitrate is within the limit negotiated by stack
        if (aac_codec.bitrate > bitrate_hal)
            aac_codec.bitrate = bitrate_hal;
        // Configure DSP for peak MTU
        aac_codec.frame_ctl.ctl_type = A2D_AAC_FRAME_PEAK_MTU;
        aac_codec.frame_ctl.ctl_value = mtu;
        ALOGI("%s: AAC HW: sampling_rate:%lu bitrate:%lu mtu:%d ctl_type:%d",
              __func__, aac_codec.sampling_rate, aac_codec.bitrate, mtu,
              aac_codec.frame_ctl.ctl_type);
        if (sample_freq) *sample_freq = aac_codec.sampling_rate;
        ALOGI("%s: AAC: done copying full codec config", __func__);
        return ((void *)(&aac_codec));

    } else if (codec_cfg->codecType == CodecType::APTX ||
               codec_cfg->codecType == CodecType::APTX_HD) {
        memset(&aptx_codec, 0, sizeof(audio_aptx_encoder_config_t));

        switch (codec_cfg->sampleRate) {
            case SampleRate::RATE_48000:
                 aptx_codec.sampling_rate = 48000;
                 break;
            case SampleRate::RATE_44100:
                 aptx_codec.sampling_rate = 44100;
                 break;
            default:
                 ALOGE("%s: APTX: unknown sampling rate:%d", __func__,
                       codec_cfg->sampleRate);
                 break;
        }

        switch (codec_cfg->channelMode) {
            case ChannelMode::STEREO:
                 aptx_codec.channels = 2;
                 break;
            case ChannelMode::MONO:
                 aptx_codec.channels = 1;
                 break;
            default:
                 ALOGE("%s: APTX: unknown channel mode:%d", __func__,
                       codec_cfg->channelMode);
                 break;
        }
        switch (codec_cfg->bitsPerSample) {
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
                 ALOGE("APTX:Unknown bits per sample");
        }
        ALOGD("%s: APTX: codec config copied", __func__);

        if (codec_cfg->encodedAudioBitrate == 0) {
            ALOGW("%s: APTX: bitrate is zero", __func__);
            // aptx_codec.bitrate = ?
        } else if (codec_cfg->encodedAudioBitrate >= 0x00000001 &&
                   codec_cfg->encodedAudioBitrate <= 0x00FFFFFF) {
            aptx_codec.bitrate = codec_cfg->encodedAudioBitrate;
        }
        if (codec_cfg->codecType == CodecType::APTX) {
           *codec_type = AUDIO_FORMAT_APTX;
           ALOGI("%s: APTX: done copying full codec config", __func__);
        } else if (codec_cfg->codecType == CodecType::APTX_HD) {
           *codec_type = AUDIO_FORMAT_APTX_HD;
           ALOGI("%s: APTXHD: done copying full codec config", __func__);
        }

        if (sample_freq) *sample_freq = aptx_codec.sampling_rate;
        return ((void *)&aptx_codec);

    } else if (codec_cfg->codecType == CodecType::LDAC) {
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

void a2dp_stream_common_init(struct a2dp_stream_common *common)
{
    pthread_mutexattr_t lock_attr;
    ALOGI("%s", __func__);

    pthread_mutexattr_init(&lock_attr);
    pthread_mutexattr_settype(&lock_attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&common->lock, &lock_attr);
    pthread_mutexattr_destroy(&lock_attr);
    common->state = AUDIO_A2DP_STATE_STOPPED;
    common->sink_latency = A2DP_DEFAULT_SINK_LATENCY;
}

int wait_for_stack_response(uint8_t time_to_wait)
{
    ALOGD("%s", __func__);
    struct timespec now, wait_time;
    uint8_t retry = 0;
    pthread_mutex_lock(&audio_stream.ack_lock);
    if (stack_cb == NULL) {
        ALOGE("%s: stack deinitialized", __func__);
        pthread_mutex_unlock(&audio_stream.ack_lock);
        return retry;
    }
    while (retry < CTRL_CHAN_RETRY_COUNT && ack_recvd == 0) {
        ALOGW("%s: entering conditional wait: retry = %d ack_recvd = %d",
              __func__, retry, ack_recvd);
        clock_gettime(CLOCK_REALTIME, &now);
        now.tv_sec += time_to_wait;
        pthread_cond_timedwait(&ack_cond, &audio_stream.ack_lock, &now);
        retry++;
    }
    pthread_mutex_unlock(&audio_stream.ack_lock);
    if (ack_recvd) {
        ALOGV("%s: ack received", __func__);
    }
    ALOGV("%s: returning retry = %d", __func__, retry);
    return retry;
}

void bt_stack_init(bt_lib_callback_t *lib_cb)
{
    ALOGI("%s", __func__);
    int ret = 0;
    stack_cb = lib_cb;
}

void bt_stack_deinit(tA2DP_CTRL_ACK status)
{
    ALOGI("%s", __func__);
    pthread_mutex_lock(&audio_stream.ack_lock);
    stack_cb = NULL;
    audio_stream.ack_status = status;
    audio_stream.session_ready = -1;
    if (!ack_recvd) {
        ack_recvd = 1;
        pthread_cond_signal(&ack_cond);
    }
    pthread_mutex_unlock(&audio_stream.ack_lock);
}

void bt_stack_on_stream_started(tA2DP_CTRL_ACK status)
{
    ALOGI("%s: status = %d", __func__, status);
    pthread_mutex_lock(&audio_stream.ack_lock);
    audio_stream.ack_status = status;
    resp_received = true;
    if (!ack_recvd) {
        ack_recvd = 1;
        pthread_cond_signal(&ack_cond);
    }
    pthread_mutex_unlock(&audio_stream.ack_lock);
}

void bt_stack_on_stream_suspended(tA2DP_CTRL_ACK status)
{
    ALOGI("%s: status = %d", __func__, status);
    pthread_mutex_lock(&audio_stream.ack_lock);
    audio_stream.ack_status = status;
    resp_received = true;
    if (!ack_recvd) {
        ack_recvd = 1;
        pthread_cond_signal(&ack_cond);
    }
    pthread_mutex_unlock(&audio_stream.ack_lock);
}

void bt_stack_on_stream_stopped(tA2DP_CTRL_ACK status)
{
    ALOGI("%s: status = %d", __func__, status);
    pthread_mutex_lock(&audio_stream.ack_lock);
    audio_stream.ack_status = status;
    resp_received = true;
    if (!ack_recvd) {
        ack_recvd = 1;
        pthread_cond_signal(&ack_cond);
    }
    pthread_mutex_unlock(&audio_stream.ack_lock);
}

void bt_stack_start_session(const CodecConfiguration &cfg)
{
    ALOGI("%s", __func__);
    pthread_mutex_lock(&audio_stream.ack_lock);
    memcpy(&audio_stream.codec_cfg, &cfg, sizeof(CodecConfiguration));
    audio_stream.session_ready = 0;
    if (!ack_recvd) {
        ack_recvd = 1;
        pthread_cond_signal(&ack_cond);
    }
    pthread_mutex_unlock(&audio_stream.ack_lock);
}

void bt_stack_on_get_sink_latency(uint16_t latency)
{
    ALOGI("%s", __func__);
    pthread_mutex_lock(&audio_stream.ack_lock);
    audio_stream.sink_latency = latency;
    resp_received = true;
    if (!ack_recvd) {
        ack_recvd = 1;
        pthread_cond_signal(&ack_cond);
    }
    pthread_mutex_unlock(&audio_stream.ack_lock);
}

// as code is converged from Audio HAL for Pixel and non pixel
// below API would be called.
int audio_start_stream()
{
    return audio_stream_start();
}

int audio_stream_start()
{
    int i, j, ack_ret;
    tA2DP_CTRL_ACK status = A2DP_CTRL_ACK_SUCCESS;
    ALOGD("%s: state = %s", __func__, dump_a2dp_hal_state(audio_stream.state));
    pthread_mutex_lock(&audio_stream.lock);

    if (audio_stream.state == AUDIO_A2DP_STATE_SUSPENDED) {
        ALOGW("%s: stream suspended", __func__);
        pthread_mutex_unlock(&audio_stream.lock);
        return -1;
    }

    for (j = 0; j <STREAM_START_MAX_RETRY_LOOPER; j++) {
        for (i = 0; i < STREAM_START_MAX_RETRY_COUNT; i++) {
            if (stack_cb) {
                audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
                resp_received = false;
                stack_cb->start_req_cb();
                ack_recvd = 0;
                if (!resp_received) {
                    ack_ret = wait_for_stack_response(1);
                    if (ack_ret == CTRL_CHAN_RETRY_COUNT && !ack_recvd) {
                        ALOGE("%s: failed to get ack from stack", __func__);
                        status = (tA2DP_CTRL_ACK)-1;
                        goto end;
                    }
                }
                pthread_mutex_lock(&audio_stream.ack_lock);
                status = audio_stream.ack_status;
                audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
                ALOGD("%s: status = %s", __func__, dump_a2dp_ctrl_ack(status));
                if (status == A2DP_CTRL_ACK_PENDING) {
                    ALOGW("%s: waiting in pending", __func__);
                    ack_recvd = 0;
                    pthread_mutex_unlock(&audio_stream.ack_lock);
                    wait_for_stack_response(5);
                    status = audio_stream.ack_status;
                    ALOGW("%s: done waiting in pending: status = %s", __func__,
                          dump_a2dp_ctrl_ack(status));
                    audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
                }else{
                 pthread_mutex_unlock(&audio_stream.ack_lock);
                }

                if (status == A2DP_CTRL_ACK_SUCCESS) {
                    ALOGW("%s: a2dp stream started successfully", __func__);
                    audio_stream.state = AUDIO_A2DP_STATE_STARTED;
                    goto end;
                } else if (status == A2DP_CTRL_ACK_INCALL_FAILURE ||
                           status == A2DP_CTRL_ACK_UNSUPPORTED ||
                           status == A2DP_CTRL_ACK_DISCONNECT_IN_PROGRESS ||
                           status == A2DP_CTRL_ACK_UNKNOWN) {
                    ALOGW("%s: a2dp stream start failed: status = %s", __func__,
                          dump_a2dp_ctrl_ack(status));
                    audio_stream.state = AUDIO_A2DP_STATE_STOPPED;
                    goto end;
                } else if (status == A2DP_CTRL_ACK_FAILURE) {
                    ALOGW("%s: a2dp stream start failed: generic failure", __func__);
                }
            } else {
                ALOGW("%s: stack shutdown", __func__);
                pthread_mutex_unlock(&audio_stream.lock);
                return A2DP_CTRL_SKT_DISCONNECTED;
            }
            ALOGW("%s: a2dp stream not started: wait 100ms and retry", __func__);
            usleep(100000);
        }
        ALOGW("%s: check if valid connection is still up or not", __func__);

        // For every 1 sec check if a2dp is still up, to avoid
        // blocking the audio thread forever if a2dp connection is closed
        // for some reason
        audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
        resp_received = false;
        if (audio_stream.session_ready != 0) {
            ALOGE("%s: no valid a2dp connection", __func__);
            pthread_mutex_unlock(&audio_stream.lock);
            return -1;
        }
    }
end:
    if (audio_stream.state != AUDIO_A2DP_STATE_STARTED) {
        ALOGE("%s: failed to start a2dp stream", __func__);
        pthread_mutex_unlock(&audio_stream.lock);
        return status;
    }
    pthread_mutex_unlock(&audio_stream.lock);
    return status;
}

int audio_stream_open()
{
    ALOGI("%s", __func__);
    struct timespec now;
    a2dp_stream_common_init(&audio_stream);
    if (stack_cb == NULL) {
        /* If stack is not ready yet, wait for sometime
           for startSession to show up */
        pthread_mutex_lock(&audio_stream.ack_lock);
        ack_recvd = 0;
        clock_gettime(CLOCK_REALTIME, &now);
        now.tv_sec += 2; //2 sec wait
        pthread_cond_timedwait(&ack_cond, &audio_stream.ack_lock, &now);
        pthread_mutex_unlock(&audio_stream.ack_lock);
    }
    if (stack_cb != NULL) {
        ALOGI("%s: success", __func__);
        return 0;
    }
    ALOGE("%s: failed", __func__);
    return -1;
}

int audio_stream_close()
{
    ALOGI("%s", __func__);
    tA2DP_CTRL_ACK status = A2DP_CTRL_ACK_SUCCESS;
    pthread_mutex_lock(&audio_stream.lock);
    if (audio_stream.state == AUDIO_A2DP_STATE_STARTED ||
        audio_stream.state == AUDIO_A2DP_STATE_STOPPING) {
        ALOGW("%s: suspending audio stream", __func__);
        if (stack_cb) {
            int ack_ret = 0;
            audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
            resp_received = false;
            stack_cb->suspend_req_cb();
            ack_recvd = 0;
            if (!resp_received) {
                ack_ret = wait_for_stack_response(1);
                if (ack_ret == 3 &&
                    audio_stream.ack_status == A2DP_CTRL_ACK_UNKNOWN) {
                    ALOGE("%s: failed to get ack from stack", __func__);
                    pthread_mutex_unlock(&audio_stream.lock);
                    return -1;
                }
            }
        }
    }
    pthread_mutex_unlock(&audio_stream.lock);
    return 0;
}

// as code is converged from Audio HAL for Pixel and non pixel
// below API would be called.
int audio_stop_stream()
{
    return audio_stream_stop();
}

int audio_stream_stop()
{
    ALOGI("%s", __func__);
    int ret = -1;
    tA2DP_CTRL_ACK status;
    pthread_mutex_lock(&audio_stream.lock);
    if (stack_cb) {
        if (audio_stream.state != AUDIO_A2DP_STATE_SUSPENDED) {
            int ack_ret = 0;
            ack_recvd = 0;
            resp_received = false;
            audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
            stack_cb->suspend_req_cb();
            if (!resp_received) {
                ack_ret = wait_for_stack_response(1);
                if (ack_ret == CTRL_CHAN_RETRY_COUNT && !ack_recvd) {
                    ALOGE("%s: failed to get ack from stack", __func__);
                    pthread_mutex_unlock(&audio_stream.lock);
                    return -1;
                }
            }
            pthread_mutex_lock(&audio_stream.ack_lock);
            status = audio_stream.ack_status;
            audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
            ALOGI("%s: ack status = %s", __func__, dump_a2dp_ctrl_ack(status));
            if (status == A2DP_CTRL_ACK_PENDING) {
                ack_recvd = 0;
                pthread_mutex_unlock(&audio_stream.ack_lock);
                wait_for_stack_response(5);
                status = audio_stream.ack_status;
                audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
                if (status == A2DP_CTRL_ACK_SUCCESS) ret = 0;
            } else if (status == A2DP_CTRL_ACK_SUCCESS) {
                ALOGD("%s: success", __func__);
                audio_stream.state = AUDIO_A2DP_STATE_STANDBY;
                pthread_mutex_unlock(&audio_stream.lock);
                pthread_mutex_unlock(&audio_stream.ack_lock);
                return 0;
            } else {
                ALOGW("%s: failed", __func__);
                audio_stream.state = AUDIO_A2DP_STATE_STOPPED;
                pthread_mutex_unlock(&audio_stream.lock);
                pthread_mutex_unlock(&audio_stream.ack_lock);
                return -1;
            }
        }
    } else {
       ALOGW("%s: stack is down", __func__);
    }
    audio_stream.state = AUDIO_A2DP_STATE_STOPPED;
    pthread_mutex_unlock(&audio_stream.lock);
    return ret;
}

// as code is converged from Audio HAL for Pixel and non pixel
// below API would be called.
int audio_suspend_stream()
{
    return audio_stream_suspend();
}

int audio_stream_suspend()
{
    ALOGI("%s", __func__);
    tA2DP_CTRL_ACK status;

    pthread_mutex_lock(&audio_stream.lock);
    if (stack_cb) {
        if (audio_stream.state != AUDIO_A2DP_STATE_SUSPENDED) {
            int ack_ret = 0;
            ack_recvd = 0;
            resp_received = false;
            audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
            stack_cb->suspend_req_cb();
            if (!resp_received) {
                ack_ret = wait_for_stack_response(1);
                if (ack_ret == CTRL_CHAN_RETRY_COUNT && !ack_recvd) {
                    ALOGE("%s: failed to get ack from stack", __func__);
                    pthread_mutex_unlock(&audio_stream.lock);
                    return -1;
                }
            }
            pthread_mutex_lock(&audio_stream.ack_lock);
            status = audio_stream.ack_status;
            audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
            ALOGW("%s: ack status = %s", __func__, dump_a2dp_ctrl_ack(status));
            if (status == A2DP_CTRL_ACK_PENDING) {
                //TODO wait for the response;
                ack_recvd = 0;
                pthread_mutex_unlock(&audio_stream.ack_lock);
                wait_for_stack_response(5);
                status = audio_stream.ack_status;
                audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
            } else if (status == A2DP_CTRL_ACK_SUCCESS) {
                ALOGD("%s: success", __func__);
                audio_stream.state = AUDIO_A2DP_STATE_SUSPENDED;
                pthread_mutex_unlock(&audio_stream.lock);
                pthread_mutex_unlock(&audio_stream.ack_lock);
                return 0;
            } else {
                ALOGW("%s: failed", __func__);
                pthread_mutex_unlock(&audio_stream.lock);
                pthread_mutex_unlock(&audio_stream.ack_lock);
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
    ALOGI("%s: state = %s", __func__, dump_a2dp_hal_state(audio_stream.state));
    pthread_mutex_lock(&audio_stream.lock);
    if (audio_stream.state != AUDIO_A2DP_STATE_STOPPED ||
        audio_stream.state != AUDIO_A2DP_STATE_STOPPING) {
        audio_stream.state = AUDIO_A2DP_STATE_STOPPED;
    }
    pthread_mutex_unlock(&audio_stream.lock);
}

void clear_a2dp_suspend_flag()
{
    ALOGI("%s: state = %s", __func__, dump_a2dp_hal_state(audio_stream.state));
    pthread_mutex_lock(&audio_stream.lock);
    if (audio_stream.state == AUDIO_A2DP_STATE_SUSPENDED) {
        audio_stream.state = AUDIO_A2DP_STATE_STOPPED;
    }
    pthread_mutex_unlock(&audio_stream.lock);
}

void * audio_get_codec_config(uint8_t *multicast_status, uint8_t *num_dev,
                              audio_format_t *codec_type)
{
    int i, status;
    ALOGI("%s: state = %s", __func__, dump_a2dp_hal_state(audio_stream.state));
    if (audio_stream.session_ready == 0) {
      return (a2dp_codec_parser(&audio_stream.codec_cfg, codec_type, NULL));
    }
    return NULL;
}

int audio_check_a2dp_ready()
{
    ALOGI("%s: state = %s", __func__, dump_a2dp_hal_state(audio_stream.state));
    tA2DP_CTRL_ACK status;
    pthread_mutex_lock(&audio_stream.lock);
    if (audio_stream.session_ready) {
      status = A2DP_CTRL_ACK_SUCCESS;
    } else {
      status = A2DP_CTRL_ACK_FAILURE;
    }
    pthread_mutex_unlock(&audio_stream.lock);
    return status;
}

uint16_t audio_get_a2dp_sink_latency()
{
    ALOGI("%s: state = %s", __func__, dump_a2dp_hal_state(audio_stream.state));
    pthread_mutex_lock(&audio_stream.lock);
    if (update_initial_sink_latency) {
        if (stack_cb) {
            resp_received = false;
            stack_cb->get_sink_latency_cb();
            ack_recvd = 0;
            if (resp_received == false)
                wait_for_stack_response(1);
        } else {
            audio_stream.sink_latency = A2DP_DEFAULT_SINK_LATENCY;
        }
        update_initial_sink_latency = false;
    }
    pthread_mutex_unlock(&audio_stream.lock);
    return audio_stream.sink_latency;
}

void ldac_codec_parser(CodecConfiguration *codec_cfg)
{
    memset(&ldac_codec, 0, sizeof(audio_ldac_encoder_config_t));

    switch (codec_cfg->sampleRate) {
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
                   codec_cfg->sampleRate);
             break;
    }

    switch (codec_cfg->channelMode) {
        case ChannelMode::MONO:
            // TODO: Fix this once HAL is updated
            // Stack only supports DUAL and STEREO
            ldac_codec.channel_mode = A2D_LDAC_CHANNEL_DUAL;
            break;
        case ChannelMode::STEREO:
            ldac_codec.channel_mode = A2D_LDAC_CHANNEL_STEREO;
            break;
        default:
             ALOGE("%s: LDAC: unknown channel mode:%d", __func__,
                   codec_cfg->channelMode);
             break;
    }

    ALOGD("%s: LDAC: codec config copied", __func__);
    uint16_t mtu = codec_cfg->peerMtu;
    if ((mtu == 0) || (mtu > MAX_2MBPS_A2DP_STREAM_MTU))
        mtu = MAX_2MBPS_A2DP_STREAM_MTU;
    ldac_codec.mtu = mtu;

    if (codec_cfg->encodedAudioBitrate == 0) {
        ALOGW("%s: LDAC: bitrate is zero", __func__);
        // ldac_codec.bitrate = ?
    } else if (codec_cfg->encodedAudioBitrate >= 0x00000001 &&
               codec_cfg->encodedAudioBitrate <= 0x00FFFFFF) {
        ldac_codec.bitrate = codec_cfg->encodedAudioBitrate;
    }

    switch (codec_cfg->bitsPerSample)
    {
        case BitsPerSample::BITS_16: ldac_codec.bits_per_sample = 16;
          break;
        case BitsPerSample::BITS_24: ldac_codec.bits_per_sample = 24;
          break;
        case BitsPerSample::BITS_32: ldac_codec.bits_per_sample = 32;
          break;
        default:
          ALOGE("LDAC: Unknown bits per sample");
    }
    ALOGW("%s: bits_per_sample:%d\n ",__func__,ldac_codec.bits_per_sample);
    ldac_codec.is_abr_enabled = (codec_cfg->codecSpecific.ldacData.bitrateIndex == 0x7F);

    ALOGW("Create Lookup for %d with ABR %d", codec_cfg->sampleRate, ldac_codec.is_abr_enabled);
    ALOGW("Codec index value %x", codec_cfg->codecSpecific.ldacData.bitrateIndex);
    if (codec_cfg->sampleRate == SampleRate::RATE_44100 ||
            codec_cfg->sampleRate == SampleRate::RATE_88200) {
        int num_of_level_entries =
            sizeof(bit_rate_level_44_1k_88_2k_database)/sizeof(bit_rate_level_44_1k_88_2k_table_t);
        ldac_codec.level_to_bitrate_map.num_levels = num_of_level_entries;
        if (ldac_codec.is_abr_enabled) {
         ldac_codec.bitrate = bit_rate_level_44_1k_88_2k_database[0].bit_rate_value;
         ALOGW("Send start highest bit-rate value %d", ldac_codec.bitrate);
        }
        for (int i = 0; i < num_of_level_entries; i++) {
            ldac_codec.level_to_bitrate_map.bit_rate_level_map[i].link_quality_level =
                bit_rate_level_44_1k_88_2k_database[i].level_value;
            ldac_codec.level_to_bitrate_map.bit_rate_level_map[i].bitrate =
                bit_rate_level_44_1k_88_2k_database[i].bit_rate_value;
            ALOGW("Level: %d, bit-rate: %d",
                ldac_codec.level_to_bitrate_map.bit_rate_level_map[i].link_quality_level,
                ldac_codec.level_to_bitrate_map.bit_rate_level_map[i].bitrate);
        }
    } else if (codec_cfg->sampleRate == SampleRate::RATE_48000 ||
            codec_cfg->sampleRate == SampleRate::RATE_96000) {
        int num_of_level_entries =
            sizeof(bit_rate_level_48k_96k_database)/sizeof(bit_rate_level_48k_96k_table_t);
        ldac_codec.level_to_bitrate_map.num_levels = num_of_level_entries;
        if (ldac_codec.is_abr_enabled) {
         ldac_codec.bitrate = bit_rate_level_48k_96k_database[0].bit_rate_value;
         ALOGW("Send start highest bit-rate value %d", ldac_codec.bitrate);
        }
        for (int i = 0; i < num_of_level_entries; i++) {
            ldac_codec.level_to_bitrate_map.bit_rate_level_map[i].link_quality_level =
                bit_rate_level_48k_96k_database[i].level_value;
            ldac_codec.level_to_bitrate_map.bit_rate_level_map[i].bitrate =
                bit_rate_level_48k_96k_database[i].bit_rate_value;
            ALOGW("Level: %d, bit-rate: %d",
                ldac_codec.level_to_bitrate_map.bit_rate_level_map[i].link_quality_level,
                ldac_codec.level_to_bitrate_map.bit_rate_level_map[i].bitrate);
        }
    } else {
        ALOGW("Unsupported Invalid frequency");
    }

    ALOGW("%s: LDAC: bitrate: %lu", __func__, ldac_codec.bitrate);
    ALOGW("LDAC: Done copying full codec config");
}

bool audio_is_scrambling_enabled(void)
{
    return true;
}
