/****************************************************************************
 * Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 *****************************************************************************/
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

/*****************************************************************************
 *
 *  Filename:      btaudio_offload.h
 *
 *  Description:
 *
 *****************************************************************************/
#ifndef BT_HOST_IPC_H
#define BT_HOST_IPC_H
#include <system/audio.h>
#include "btaudio_offload_qti.h"
/*****************************************************************************
**  Constants & Macros
******************************************************************************/
#define BT_AUDIO_HARDWARE_INTERFACE "libbthost"

#include <time.h>

#include <vendor/qti/hardware/bluetooth_audio/2.0/types.h>
#include <vendor/qti/hardware/bluetooth_audio/2.0/IBluetoothAudioPort.h>
#include <vendor/qti/hardware/bluetooth_audio/2.0/IBluetoothAudioProvider.h>

namespace vendor {
namespace qti {
namespace btoffload {

using ::vendor::qti::hardware::bluetooth_audio::V2_0::SessionType;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::IBluetoothAudioPort;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::IBluetoothAudioProvider;
using ::android::hardware::kSynchronizedReadWrite;
using ::android::hardware::hidl_vec;
using ::android::sp;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::AudioConfiguration;

using ::vendor::qti::hardware::bluetooth_audio::V2_0::SessionParams;
typedef ::vendor::qti::hardware::bluetooth_audio::V2_0::Status BluetoothAudioStatus;

typedef enum {
    HIDL_REQUEST_STATUS_SUCCESS = 0,
    HIDL_REQUEST_STATUS_TIMEOUT,
    HIDL_REQUEST_STATUS_FAILED,
    HIDL_REQUEST_STATUS_CLIENT_NOT_READY,
    HIDL_REQUEST_STATUS_CLIENT_DEAD,
    HIDL_REQUEST_STATUS_NONE
} tHIDL_REQUEST_STATUS;

typedef enum {
    A2DP_CTRL_CMD_NONE,
    A2DP_CTRL_CMD_CHECK_READY,
    A2DP_CTRL_CMD_CHECK_STREAM_STARTED,
    A2DP_CTRL_CMD_START,
    A2DP_CTRL_CMD_STOP,
    A2DP_CTRL_CMD_SUSPEND,
    A2DP_CTRL_GET_AUDIO_CONFIG,
    A2DP_CTRL_CMD_OFFLOAD_START,
    A2DP_CTRL_CMD_OFFLOAD_SUPPORTED,
    A2DP_CTRL_CMD_OFFLOAD_NOT_SUPPORTED,
} tA2DP_CTRL_CMD;

typedef enum {
    A2DP_CTRL_ACK_SUCCESS,
    A2DP_CTRL_ACK_UNSUPPORTED,
    A2DP_CTRL_ACK_FAILURE,
    A2DP_CTRL_ACK_PENDING,
    A2DP_CTRL_ACK_INCALL_FAILURE,
    A2DP_CTRL_ACK_SW_ERROR,
    A2DP_CTRL_ACK_LW_ERROR,
    A2DP_CTRL_ACK_FAILURE_DISC_IN_PROGRESS,
    A2DP_CTRL_ACK_FAILURE_NO_WAIT,
    A2DP_CTRL_ACK_UNKNOWN,
} tA2DP_CTRL_ACK;

typedef enum {
    AUDIO_A2DP_STATE_STANDBY,    /* allows write to autoresume */
    AUDIO_A2DP_STATE_STARTING,
    AUDIO_A2DP_STATE_STARTED,
    AUDIO_A2DP_STATE_STOPPING,
    AUDIO_A2DP_STATE_STOPPED,
    AUDIO_A2DP_STATE_SUSPENDED /* need explicit set param call to resume (suspend=false) */
} tA2DP_STATE;

#define  MAX_CODEC_CFG_SIZE  30

struct a2dp_config {
    uint32_t                rate;
    uint32_t                channel_flags;
    int                     format;
};

struct a2dp_stream_common {
    pthread_mutex_t         lock;
    pthread_mutex_t         ack_lock;
    tA2DP_STATE             state;
    tA2DP_CTRL_ACK          ack_status;
    uint8_t                 multicast;
    uint8_t                 num_conn_dev;
    AudioConfiguration      codec_cfg;
    uint8_t                 session_ready;
    uint16_t                sink_latency;
    uint16_t                ctrl_key;
    SessionType             sessionType;
    std::function<void(const uint16_t&, const bool&, const BluetoothAudioStatus&)> control_result_cb;
    std::function<void(const uint16_t&)> session_changed_cb;
    std::function<void(const uint16_t &, const SessionParams &session_params)>
       session_params_cb;
};

/* codec specific definitions */
#define A2D_SBC_BLK_MASK         0xF0
#define A2D_SBC_SUBBAND_MASK     0x0C
#define A2D_SBC_ALLOC_MASK       0x03
#define A2D_SBC_BLOCKS_4         0x80    /* 4 blocks */
#define A2D_SBC_BLOCKS_8         0x40    /* 8 blocks */
#define A2D_SBC_BLOCKS_12        0x20    /* 12blocks */
#define A2D_SBC_BLOCKS_16        0x10    /* 16blocks */
#define A2D_SBC_SUBBAND_4        0x08    /* b3: 4 */
#define A2D_SBC_SUBBAND_8        0x04    /* b2: 8 */
#define A2D_SBC_ALLOC_MD_S       0x02    /* b1: SNR */
#define A2D_SBC_ALLOC_MD_L       0x01    /* b0: loundess */
#define A2D_SBC_CHANNEL_MONO     0x00
#define A2D_SBC_CHANNEL_DUAL_MONO    0x01
#define A2D_SBC_CHANNEL_STEREO       0x02
#define A2D_SBC_CHANNEL_JOINT_STEREO 0x03

#define A2D_LDAC_CHANNEL_STEREO       0x01
#define A2D_LDAC_CHANNEL_DUAL         0x02
#define A2D_LDAC_CHANNEL_MONO         0x04

#define A2DP_APTX_ADAPTIVE_CHANNELS_MONO          (0x01)
#define A2DP_APTX_ADAPTIVE_CHANNELS_STEREO        (0x02)
#define A2DP_APTX_ADAPTIVE_CHANNELS_TWS_STEREO    (0x04)
#define A2DP_APTX_ADAPTIVE_CHANNELS_JOINT_STEREO  (0x08)
#define A2DP_APTX_ADAPTIVE_CHANNELS_TWS_MONO      (0x10)
#define A2DP_APTX_ADAPTIVE_CHANNELS_EARBUD        (0x20)

#define AptxAdaptiveChannelMode_STEREO        (0)

#define A2D_AAC_FRAME_PEAK_MTU       0  /* Configure peak MTU */
#define A2D_AAC_FRAME_PEAK_BITRATE   1  /* Configure peak bitrate */

#define A2DP_DEFAULT_SINK_LATENCY 0

#define ENC_CODEC_TYPE_APTX_ADAPTIVE 620756992u // 0x25000000UL
#define AUDIO_CODEC_TYPE_CELT         603979776u // 0x24000000UL
#define ENC_CODEC_TYPE_APTX_DUAL_MONO 570425344u // 0x22000000UL

typedef struct {
    uint32_t subband;    /* 4, 8 */
    uint32_t blk_len;    /* 4, 8, 12, 16 */
    uint16_t sampling_rate; /*44.1khz,48khz*/
    uint8_t  channels;      /*0(Mono),1(Dual_mono),2(Stereo),3(JS)*/
    uint8_t  alloc;         /*0(Loudness),1(SNR)*/
    uint8_t  min_bitpool;   /* 2 */
    uint8_t  max_bitpool;   /*53(44.1khz),51 (48khz) */
    uint32_t bitrate;      /* 320kbps to 512kbps */
    uint32_t bits_per_sample;  /* 16 bit */
} audio_sbc_encoder_config_t;

/* Information about BT APTX encoder configuration
 * This data is used between audio HAL module and
 * BT IPC library to configure DSP encoder
 */
typedef struct {
    uint16_t sampling_rate;
    uint8_t  channels;
    uint32_t bitrate;
    uint32_t bits_per_sample;
} audio_aptx_encoder_config_t;

typedef struct {
    uint32_t  sampling_rate;
    uint32_t  mtu;
    int32_t  channel_mode;
    uint32_t  min_sink_buffering_LL;
    uint32_t  max_sink_buffering_LL;
    uint32_t  min_sink_buffering_HQ;
    uint32_t  max_sink_buffering_HQ;
    uint32_t  min_sink_buffering_TWS;
    uint32_t  max_sink_buffering_TWS;
    uint32_t  aptx_mode;
    uint8_t  TTP_LL_low;
    uint8_t  TTP_LL_high;
    uint8_t  TTP_HQ_low;
    uint8_t  TTP_HQ_high;
    uint8_t  TTP_TWS_low;
    uint8_t  TTP_TWS_high;
    uint32_t bits_per_sample;
    uint32_t  input_mode;
    uint32_t  input_fade_duration;
    uint8_t  sink_capabilities[11];
} audio_aptx_adaptive_encoder_config_t;

typedef struct {
    uint16_t sampling_rate;
    uint8_t  channels;
    uint32_t bitrate;
    uint8_t sync_mode;
} audio_aptx_tws_encoder_config_t;

typedef struct {
    uint32_t sampling_rate; /* 32000 - 48000, 48000 */
    uint16_t channels; /* 1-Mono, 2-Stereo, 2*/
    uint16_t frame_size; /* 64-128-256-512, 512 */
    uint16_t complexity; /* 0-10, 1 */
    uint16_t prediction_mode; /* 0-1-2, 0 */
    uint16_t vbr_flag; /* 0-1, 0*/
    uint32_t bitrate; /*32000 - 1536000, 139500*/
} audio_ba_celt_encoder_config_t;

struct bit_rate_level_map_t {
    uint32_t link_quality_level;
    uint32_t bitrate;
};

#define MAX_LEVELS 5

struct quality_level_to_bitrate_info {
    uint32_t num_levels;
    struct bit_rate_level_map_t bit_rate_level_map[MAX_LEVELS];
};

/* Information about BT LDAC encoder configuration
 * This data is used between audio HAL module and
 * BT IPC library to configure DSP encoder
 */
typedef struct {
    uint32_t sampling_rate;
    uint32_t bitrate;
    uint16_t channel_mode;
    uint16_t mtu;
    uint32_t bits_per_sample;
    bool is_abr_enabled;
    struct quality_level_to_bitrate_info level_to_bitrate_map;
} audio_ldac_encoder_config_t;

/* Structure to control frame size of AAC encoded frames. */
struct aac_frame_size_control_t {
    /* Type of frame size control: MTU_SIZE / PEAK_BIT_RATE */
    uint32_t ctl_type;
    /* Control value
     * MTU_SIZE: MTU size in bytes
     * PEAK_BIT_RATE: Peak bitrate in bits per second.
     */
    uint32_t ctl_value;
};


/* Information about BT AAC encoder configuration
 * This data is used between audio HAL module and
 * BT IPC library to configure DSP encoder
 */
typedef struct {
    uint32_t enc_mode; /* LC, SBR, PS */
    uint16_t format_flag; /* RAW, ADTS */
    uint16_t channels; /* 1-Mono, 2-Stereo */
    uint32_t sampling_rate;
    uint32_t bitrate;
    uint32_t bits_per_sample;
    struct aac_frame_size_control_t frame_ctl;
} audio_aac_encoder_config_t;

static void btapoffload_port_init();

extern int wait_for_stack_response(uint8_t duration);

}  // namespace btoffload
}  // qti
}  // vendor

#endif
