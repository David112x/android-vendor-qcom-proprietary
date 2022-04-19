/****************************************************************************
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
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
 *  Filename:      audio_a2dp_hw.h
 *
 *  Description:
 *
 *****************************************************************************/
#ifndef BT_HOST_IPC_H
#define BT_HOST_IPC_H
#include <system/audio.h>
#include <android/hardware/bluetooth/a2dp/1.0/types.h>

/*****************************************************************************
**  Constants & Macros
******************************************************************************/
#define BT_AUDIO_HARDWARE_INTERFACE "libbthost"

using android::hardware::bluetooth::a2dp::V1_0::Status;
#define MAX_LEVELS 5
using android::hardware::bluetooth::a2dp::V1_0::CodecConfiguration;
using android::hardware::bluetooth::a2dp::V1_0::CodecType;
using android::hardware::bluetooth::a2dp::V1_0::SampleRate;
using android::hardware::bluetooth::a2dp::V1_0::BitsPerSample;
using android::hardware::bluetooth::a2dp::V1_0::ChannelMode;

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
    A2DP_CTRL_ACK_FAILURE,
    A2DP_CTRL_ACK_INCALL_FAILURE, /* Failure when in Call*/
    A2DP_CTRL_ACK_PENDING,
    A2DP_CTRL_ACK_DISCONNECT_IN_PROGRESS,
    A2DP_CTRL_SKT_DISCONNECTED,
    A2DP_CTRL_ACK_UNSUPPORTED,
    A2DP_CTRL_ACK_UNKNOWN,
} tA2DP_CTRL_ACK;

typedef enum {
    AUDIO_A2DP_STATE_STARTING,
    AUDIO_A2DP_STATE_STARTED,
    AUDIO_A2DP_STATE_STOPPING,
    AUDIO_A2DP_STATE_STOPPED,
    AUDIO_A2DP_STATE_SUSPENDED, /* need explicit set param call to resume (suspend=false) */
    AUDIO_A2DP_STATE_STANDBY    /* allows write to autoresume */
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
    CodecConfiguration      codec_cfg;
    uint8_t                 session_ready;
    uint16_t                sink_latency;
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

#define A2D_AAC_FRAME_PEAK_MTU       0  /* Configure peak MTU */
#define A2D_AAC_FRAME_PEAK_BITRATE   1  /* Configure peak bitrate */

#define A2DP_DEFAULT_SINK_LATENCY 0

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

struct bit_rate_level_map_t {
    uint32_t link_quality_level;
    uint32_t bitrate;
};

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

/* HIDL callbacks to invoke callback to BT stack */
typedef void (*bt_ipc_start_stream_req_cb)(void);
typedef void (*bt_ipc_suspend_stream_req_cb)(void);
typedef void (*bt_ipc_stop_stream_req_cb)(void);
typedef void (*bt_ipc_a2dp_check_ready_cb)(void);
typedef void (*bt_ipc_get_codec_config_cb)(void);
typedef void (*bt_ipc_get_sink_latency_cb)(void);

typedef struct {
    bt_ipc_start_stream_req_cb start_req_cb;
    bt_ipc_suspend_stream_req_cb suspend_req_cb;
    bt_ipc_stop_stream_req_cb stop_req_cb;
    bt_ipc_a2dp_check_ready_cb a2dp_check_ready_cb;
    bt_ipc_get_codec_config_cb get_codec_cfg_cb;
    bt_ipc_get_sink_latency_cb get_sink_latency_cb;
} bt_lib_callback_t;

extern "C" void bt_stack_init(bt_lib_callback_t *lib_cb);
extern "C" void bt_stack_deinit(tA2DP_CTRL_ACK status);
extern "C" void bt_stack_on_stream_started(tA2DP_CTRL_ACK status);
extern "C" void bt_stack_on_stream_suspended(tA2DP_CTRL_ACK status);
extern "C" void bt_stack_on_stream_stopped(tA2DP_CTRL_ACK status);
extern "C" void bt_stack_start_session(const CodecConfiguration &config);
extern "C" void bt_stack_on_check_a2dp_ready(tA2DP_CTRL_ACK status);
extern "C" void bt_stack_on_get_sink_latency(uint16_t latency);
extern "C" int audio_stream_open(void);
extern "C" int audio_stream_close(void);
extern "C" int audio_stream_start(void);
extern "C" int audio_start_stream(void);
extern "C" int audio_stream_stop(void);
extern "C" int audio_stop_stream(void);
extern "C" int audio_stream_suspend(void);
extern "C" int audio_suspend_stream(void);
extern "C" void* audio_get_codec_config(uint8_t *mcast, uint8_t *num_dev, audio_format_t *codec_type);
extern "C" void audio_handoff_triggered(void);
extern "C" void clear_a2dp_suspend_flag(void);
extern "C" void* audio_get_next_codec_config(uint8_t idx, audio_format_t *codec_type);
extern "C" int audio_check_a2dp_ready(void);
extern "C" uint16_t audio_get_a2dp_sink_latency();
extern "C" int wait_for_stack_response(uint8_t duration);
extern "C" bool audio_is_scrambling_enabled(void);

#endif
