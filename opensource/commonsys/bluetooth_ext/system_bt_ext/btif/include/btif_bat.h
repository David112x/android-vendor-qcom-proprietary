/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*******************************************************************************
 *
 *  Filename:      btif_bat.h
 *
 *  Description:   Header file for all interface apis call in broadcast audio
 *                 transmtter from internal stack.
 *
 ******************************************************************************/

#ifndef BTIF_BA_H
#define BTIF_BA_H

#include "btif_common.h"
#include "btif_sm.h"
#include "bta_sys.h"

/*******************************************************************************
 *  BTIF BA API
 ******************************************************************************/

typedef enum {
    BA_CTRL_ACK_SUCCESS,
    BA_CTRL_ACK_FAILURE,
    BA_CTRL_ACK_INCALL_FAILURE, /* Failure when in Call*/
    BA_CTRL_ACK_UNSUPPORTED,
    BA_CTRL_ACK_PENDING,
    BA_CTRL_ACK_DISCONNECT_IN_PROGRESS,
    BA_CTRL_SKT_DISCONNECTED,
    BA_CTRL_ACK_UNKNOWN,
} tBA_CTRL_ACK;

typedef enum {
    BTIF_BA_STATE_IDLE_AUDIO_PENDING = 0,
    BTIF_BA_STATE_IDLE_AUDIO_STREAMING,
    BTIF_BA_STATE_IDLE_AUDIO_NS,
    BTIF_BA_STATE_PENDING_AUDIO_NS,
    BTIF_BA_STATE_PAUSED_AUDIO_NS,
    BTIF_BA_STATE_STREAMING_AUDIO_NS,
}btif_ba_state_t;

typedef enum {
    BTIF_BA_API_INIT_REQ_EVT = 1,
    BTIF_BA_API_SET_STATE_START_REQ_EVT,
    BTIF_BA_API_SET_STATE_STOP_REQ_EVT,
    BTIF_BA_API_REFRESH_ENC_KEY_REQ_EVT,
    BTIF_BA_API_SET_VOL_LEVEL,
    BTIF_BA_BT_A2DP_DISC_EVT,
    BTIF_BA_BT_A2DP_PAUSED_EVT,
    BTIF_BA_BT_A2DP_STARTED_EVT,
    BTIF_BA_BT_A2DP_STARTING_EVT,// when A2DP send AVDTP_START to remote.
    BTIF_BA_CMD_PAUSE_REQ_EVT,// CMD and RSP evt should be handled only in B_Pe
    BTIF_BA_CMD_STREAM_REQ_EVT,
    BTIF_BA_CMD_STOP_REQ_EVT,
    BTIF_BA_CMD_UPDATE_ENC_KEY,
    BTIF_BA_CMD_SEND_VOL_UPDATE,
    BTIF_BA_RSP_STOP_DONE_EVT,
    BTIF_BA_RSP_PAUSE_DONE_EVT,
    BTIF_BA_RSP_STREAM_DONE_EVT,
    BTIF_BA_RSP_VOL_UPDATE_DONE_EVT,
    BTIF_BA_RSP_ENC_KEY_UPDATE_DONE_EVT,
    BTIF_BA_CSB_TIMEOUT_EVT,
    BTIF_BA_AUDIO_START_REQ_EVT,
    BTIF_BA_AUDIO_PAUSE_REQ_EVT,
    BTIF_BA_AUDIO_STOP_REQ_EVT,
    BTIF_BA_API_DEINIT_REQ_EVT,
} btif_ba_sm_event_t;

#define ENCRYPTION_KEY_LEN  16
#define DIV_KEY_LEN         2

#define CODEC_TYPE_CELT 0xEF   /* CELT media codec type */
// CELT Codec config in order.
// 7-4 bits of first byte of codec_info element
#define A2D_CELT_SAMP_FREQ_MASK    0xF0
#define A2D_CELT_SAMP_FREQ_48      0x10
#define A2D_CELT_SAMP_FREQ_44      0x20
#define A2D_CELT_SAMP_FREQ_32      0x40
// 0-3 bits of first byte of codec_info element
#define A2D_CELT_CHANNEL_MASK      0x0F
#define A2D_CELT_CH_MONO           0x01
#define A2D_CELT_CH_STEREO         0x02
// 7-4 bits of second byte: frame size
#define A2D_CELT_FRAME_SIZE_MASK   0xF0
#define A2D_CELT_FRAME_SIZE_64     0x10
#define A2D_CELT_FRAME_SIZE_128    0x20
#define A2D_CELT_FRAME_SIZE_256    0x40
#define A2D_CELT_FRAME_SIZE_512    0x80
//0-3 bits of second byte: actual value of complexity
#define A2D_CELT_COMPLEXITY_MASK   0x0F
#define A2D_CELT_COMPLEXITY_BA     0x01
// 7-4 bits of third byte: prediction mode
#define A2D_CELT_PREDICTION_MODE_MASK   0xF0
// 0th bit of third byte: vbr flag
#define A2D_CELT_VBR_MASK         0x01
//4-7 bytes: actual value of bit rate.
#define A2D_CELT_BIT_RATE_48_2_5    0x0220EC // 139500

#define STREAM_ID_48 1
#define STREAM_ID_PAUSED 0

// public ba apis called by other modules.
btif_ba_state_t btif_ba_get_state();
void getBACodecConfig(uint8_t* p_codec_config);
void btif_ba_bta_callback(uint16_t event, uint8_t result);
void ba_send_message(uint8_t event, uint8_t size, char* ptr, bool is_btif_thread);
uint16_t btif_get_ba_latency();
bool btif_ba_is_active();
uint8_t btif_ba_get_sample_rate();
uint8_t btif_ba_get_channel_mode();
uint8_t btif_ba_get_frame_size();
uint8_t btif_ba_get_complexity();
uint8_t btif_ba_get_prediction_mode();
uint8_t btif_ba_get_vbr_flag();
uint32_t btif_ba_get_bitrate();

/*
uint8_t get_curr_vol_level();
uint8_t get_max_vol_level();
void get_enc_key(uint8_t *ptr);
void get_div(uint8_t *ptr);*/

#endif /* BTIF_BA_H */
