/******************************************************************************
 * Copyright (C) 2017, The Linux Foundation. All rights reserved.
 * Not a Contribution.
 ******************************************************************************/
/******************************************************************************
 *
 *  Copyright (C) 2016 The Android Open Source Project
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

#define LOG_TAG "bt_btif_a2dp_control"

#include <base/logging.h>
#include <stdbool.h>
#include <stdint.h>

#include "audio_a2dp_hw/include/audio_a2dp_hw.h"
#include "bt_common.h"
#include "btif_a2dp.h"
#include "btif_a2dp_control.h"
#include "btif_a2dp_sink.h"
#include "btif_a2dp_source.h"
#include "btif_av.h"
#include "btif_av_co.h"
#include "btif_hf.h"
#include "osi/include/osi.h"
#include "uipc.h"
#include "btif_a2dp_audio_interface.h"

#define A2DP_DATA_READ_POLL_MS 10
#define A2DP_NUM_STRS 5

struct {
  uint64_t total_bytes_read = 0;
  uint16_t audio_delay[A2DP_NUM_STRS] = {0, 0, 0, 0, 0};
  struct timespec timestamp = {};
} delay_report_stats;

extern int btif_av_get_latest_device_idx_to_start();
extern int btif_get_is_remote_started_idx();
extern tBTA_AV_HNDL btif_av_get_av_hdl_from_idx(int idx);
extern bool btif_av_is_playing_on_other_idx(int current_index);
extern bool btif_av_is_handoff_set();
extern int btif_max_av_clients;
extern bool btif_av_is_state_opened(int i);
static void btif_a2dp_data_cb(tUIPC_CH_ID ch_id, tUIPC_EVENT event);
static void btif_a2dp_ctrl_cb(tUIPC_CH_ID ch_id, tUIPC_EVENT event);
static void btif_a2dp_snd_ctrl_cmd(tA2DP_CTRL_CMD cmd);

/* We can have max one command pending */
static tA2DP_CTRL_CMD a2dp_cmd_pending = A2DP_CTRL_CMD_NONE;
static tA2DP_CTRL_CMD a2dp_cmd_queued = A2DP_CTRL_CMD_NONE;
static char a2dp_hal_imp[PROPERTY_VALUE_MAX] = "false";

bool is_block_hal_start = false;

void btif_a2dp_control_init(void) {
  a2dp_cmd_pending = A2DP_CTRL_CMD_NONE;
  a2dp_cmd_queued = A2DP_CTRL_CMD_NONE;
  UIPC_Init(NULL);
  UIPC_Open(UIPC_CH_ID_AV_CTRL, btif_a2dp_ctrl_cb);
}

void btif_a2dp_control_cleanup(void) {
  /* This calls blocks until UIPC is fully closed */
  UIPC_Close(UIPC_CH_ID_ALL);
}

static void btif_a2dp_recv_ctrl_data(void) {
  tA2DP_CTRL_CMD cmd = A2DP_CTRL_CMD_NONE;
  int n;
  int rs_idx, cur_idx;

  uint8_t read_cmd = 0; /* The read command size is one octet */
  n = UIPC_Read(UIPC_CH_ID_AV_CTRL, NULL, &read_cmd, 1);
  cmd = static_cast<tA2DP_CTRL_CMD>(read_cmd);

  /* detach on ctrl channel means audioflinger process was terminated */
  if (n == 0) {
    APPL_TRACE_WARNING("%s: CTRL CH DETACHED", __func__);
    UIPC_Close(UIPC_CH_ID_AV_CTRL);
    return;
  }

  APPL_TRACE_DEBUG("btif_a2dp_recv_ctrl_data: %s", audio_a2dp_hw_dump_ctrl_event(cmd));

  if (property_get("persist.vendor.bt.a2dp.hal.implementation", a2dp_hal_imp, "false") &&
          !strcmp(a2dp_hal_imp, "true")) {
    switch (cmd) {
      uint8_t local_ack;
      case A2DP_CTRL_CMD_CHECK_READY:
        if (btif_a2dp_source_media_task_is_shutting_down()) {
          APPL_TRACE_WARNING("%s: A2DP command %s while media task shutting down",
                  __func__, audio_a2dp_hw_dump_ctrl_event(cmd));
          local_ack = A2DP_CTRL_ACK_FAILURE;
        } else if (btif_av_is_connected()) {
          /* check for valid AV connection */
          BTIF_TRACE_DEBUG("%s:Got valid connection",__func__);
          local_ack = A2DP_CTRL_ACK_SUCCESS;
        } else {
          BTIF_TRACE_DEBUG("%s:A2dp command %s while valid AV connection",
                  __func__, audio_a2dp_hw_dump_ctrl_event(cmd));
          local_ack = A2DP_CTRL_ACK_FAILURE;
        }
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0, &local_ack, sizeof(local_ack));
        break;

      case A2DP_CTRL_CMD_CHECK_STREAM_STARTED:
        if ((btif_av_stream_started_ready() == TRUE))
          local_ack = A2DP_CTRL_ACK_SUCCESS;
        else
          local_ack = A2DP_CTRL_ACK_FAILURE;
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0, &local_ack, sizeof(local_ack));
        break;

      case A2DP_CTRL_GET_INPUT_AUDIO_CONFIG: {
        tA2DP_SAMPLE_RATE sample_rate = btif_a2dp_sink_get_sample_rate();
        tA2DP_CHANNEL_COUNT channel_count = btif_a2dp_sink_get_channel_count();

        local_ack = A2DP_CTRL_ACK_SUCCESS;
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0, &local_ack, sizeof(local_ack));

        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0, reinterpret_cast<uint8_t*>(&sample_rate),
                sizeof(tA2DP_SAMPLE_RATE));
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0, &channel_count,
                sizeof(tA2DP_CHANNEL_COUNT));
        break;
      }

      case A2DP_CTRL_GET_OUTPUT_AUDIO_CONFIG: {
        btav_a2dp_codec_config_t codec_config;
        btav_a2dp_codec_config_t codec_capability;
        codec_config.sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_NONE;
        codec_config.bits_per_sample = BTAV_A2DP_CODEC_BITS_PER_SAMPLE_NONE;
        codec_config.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_NONE;
        codec_capability.sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_NONE;
        codec_capability.bits_per_sample = BTAV_A2DP_CODEC_BITS_PER_SAMPLE_NONE;
        codec_capability.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_NONE;

        A2dpCodecConfig* current_codec = bta_av_get_a2dp_current_codec();
        if (current_codec != nullptr) {
          codec_config = current_codec->getCodecConfig();
          codec_capability = current_codec->getCodecCapability();
        }

        local_ack = A2DP_CTRL_ACK_SUCCESS;
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0, &local_ack, sizeof(local_ack));

        // Send the current codec config
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0,
                reinterpret_cast<const uint8_t*>(&codec_config.sample_rate),
                sizeof(btav_a2dp_codec_sample_rate_t));
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0,
                reinterpret_cast<const uint8_t*>(&codec_config.bits_per_sample),
                sizeof(btav_a2dp_codec_bits_per_sample_t));
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0,
                reinterpret_cast<const uint8_t*>(&codec_config.channel_mode),
                sizeof(btav_a2dp_codec_channel_mode_t));
        // Send the current codec capability
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0,
                reinterpret_cast<const uint8_t*>(&codec_capability.sample_rate),
                sizeof(btav_a2dp_codec_sample_rate_t));
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0, reinterpret_cast<const uint8_t*>(
                                           &codec_capability.bits_per_sample),
                sizeof(btav_a2dp_codec_bits_per_sample_t));
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0, reinterpret_cast<const uint8_t*>(
                                           &codec_capability.channel_mode),
                sizeof(btav_a2dp_codec_channel_mode_t));
        break;
      }

      case A2DP_CTRL_SET_OUTPUT_AUDIO_CONFIG: {
        btav_a2dp_codec_config_t codec_config;
        codec_config.codec_type = BTAV_A2DP_CODEC_INDEX_SOURCE_MIN;
        codec_config.codec_priority = BTAV_A2DP_CODEC_PRIORITY_DEFAULT;
        codec_config.codec_specific_1 = 0;
        codec_config.codec_specific_2 = 0;
        codec_config.codec_specific_3 = 0;
        codec_config.codec_specific_4 = 0;
        codec_config.sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_NONE;
        codec_config.bits_per_sample = BTAV_A2DP_CODEC_BITS_PER_SAMPLE_NONE;
        codec_config.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_NONE;

        local_ack = A2DP_CTRL_ACK_SUCCESS;
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0, &local_ack, sizeof(local_ack));
        // Send the current codec config
        if (UIPC_Read(UIPC_CH_ID_AV_CTRL, 0,
                reinterpret_cast<uint8_t*>(&codec_config.sample_rate),
                sizeof(btav_a2dp_codec_sample_rate_t)) !=
                sizeof(btav_a2dp_codec_sample_rate_t)) {
          APPL_TRACE_ERROR("%s: Error reading sample rate from audio HAL",
                  __func__);
          break;
        }
        if (UIPC_Read(UIPC_CH_ID_AV_CTRL, 0,
                reinterpret_cast<uint8_t*>(&codec_config.bits_per_sample),
                sizeof(btav_a2dp_codec_bits_per_sample_t)) !=
                sizeof(btav_a2dp_codec_bits_per_sample_t)) {
          APPL_TRACE_ERROR("%s: Error reading bits per sample from audio HAL",
                  __func__);
          break;
        }
        if (UIPC_Read(UIPC_CH_ID_AV_CTRL, 0,
                reinterpret_cast<uint8_t*>(&codec_config.channel_mode),
                sizeof(btav_a2dp_codec_channel_mode_t)) !=
                sizeof(btav_a2dp_codec_channel_mode_t)) {
          APPL_TRACE_ERROR("%s: Error reading channel mode from audio HAL",
                  __func__);
          break;
        }
        APPL_TRACE_DEBUG(
                "%s: A2DP_CTRL_SET_OUTPUT_AUDIO_CONFIG: "
                "sample_rate=0x%x bits_per_sample=0x%x "
                "channel_mode=0x%x",
                __func__, codec_config.sample_rate, codec_config.bits_per_sample,
                codec_config.channel_mode);
        btif_a2dp_source_feeding_update_req(codec_config);
        break;
      }

      case A2DP_CTRL_CMD_OFFLOAD_START: {
        uint8_t hdl = 0;
        int idx = btif_get_is_remote_started_idx();
        if (idx < btif_max_av_clients) {
          hdl = btif_av_get_av_hdl_from_idx(idx);
          APPL_TRACE_DEBUG("%s: hdl = %d",__func__, hdl);
        } else {
          APPL_TRACE_ERROR("%s: Invalid index",__func__);
          break;
        }
        btif_dispatch_sm_event(BTIF_AV_OFFLOAD_START_REQ_EVT, (char *)&hdl, 1);
        break;
      }
      case A2DP_CTRL_GET_SINK_LATENCY: {
        tA2DP_LATENCY sink_latency;

        sink_latency = btif_av_get_sink_latency();
        local_ack = A2DP_CTRL_ACK_SUCCESS;
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0, &local_ack, sizeof(local_ack));
        /* Send sink latency */
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0,
                reinterpret_cast<const uint8_t*>(&sink_latency),
                sizeof(tA2DP_LATENCY));
        break;
      }

      case A2DP_CTRL_CMD_STREAM_OPEN:
        APPL_TRACE_DEBUG("Accept Audio Start after Stream open");
        is_block_hal_start = false;
        btif_a2dp_source_cancel_unblock_audio_start();
        local_ack = A2DP_CTRL_ACK_SUCCESS;
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0, &local_ack, sizeof(local_ack));
        break;

      case A2DP_CTRL_GET_PRESENTATION_POSITION: {
        local_ack = A2DP_CTRL_ACK_SUCCESS;
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0, &local_ack, sizeof(local_ack));

        int idx = btif_av_get_current_playing_dev_idx();
        uint16_t audio_delay = (idx < btif_max_av_clients) ? delay_report_stats.audio_delay[idx]:0;
        APPL_TRACE_DEBUG("Delay Rpt: total bytes read = %d", delay_report_stats.total_bytes_read);
        APPL_TRACE_DEBUG("Delay Rpt: delay = %d, index: %d", audio_delay, idx);
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0,
                  (uint8_t*)&(delay_report_stats.total_bytes_read),
                  sizeof(uint64_t));
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0,
                  (uint8_t*)&(audio_delay), sizeof(uint16_t));

        uint32_t seconds = delay_report_stats.timestamp.tv_sec;
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0, (uint8_t*)&seconds, sizeof(seconds));

        uint32_t nsec = delay_report_stats.timestamp.tv_nsec;
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0, (uint8_t*)&nsec, sizeof(nsec));
        APPL_TRACE_DEBUG("Delay Rpt: seconds = %d, nsec = %d" ,seconds, nsec);
        break;
      }

      default:
        if (a2dp_cmd_pending != A2DP_CTRL_CMD_NONE)
        {
          uint8_t err_ack = A2DP_CTRL_ACK_PREVIOUS_COMMAND_PENDING;
          APPL_TRACE_ERROR("btif_recv_ctrl_data: warning : previous command pending, queueing command");
          if (a2dp_cmd_pending == cmd)
          {
            APPL_TRACE_ERROR("btif_recv_ctrl_data: Not queuing same command");
          }
          else
          {
            a2dp_cmd_queued = cmd;
          }
          UIPC_Send(UIPC_CH_ID_AV_CTRL, 0, &err_ack, sizeof(err_ack));
        }
        else
        {
          a2dp_cmd_pending = cmd;
          btif_a2dp_snd_ctrl_cmd(cmd);
        }
    }
  } else {
    a2dp_cmd_pending = cmd;

    switch (cmd) {
      case A2DP_CTRL_CMD_CHECK_READY:
        if (btif_a2dp_source_media_task_is_shutting_down()) {
          APPL_TRACE_WARNING("%s: A2DP command %s while media task shutting down",
                  __func__, audio_a2dp_hw_dump_ctrl_event(cmd));
          btif_a2dp_command_ack(A2DP_CTRL_ACK_FAILURE);
          return;
        }
        /* check for valid AV connection */
        if (btif_av_is_connected()) {
          BTIF_TRACE_DEBUG("%s:Got valid connection",__func__);
          btif_a2dp_command_ack(A2DP_CTRL_ACK_SUCCESS);
        } else {
          BTIF_TRACE_DEBUG("%s:A2dp command %s while valid AV connection",
                  __func__, audio_a2dp_hw_dump_ctrl_event(cmd));
          btif_a2dp_command_ack(A2DP_CTRL_ACK_FAILURE);
        }
        break;

      case A2DP_CTRL_CMD_CHECK_STREAM_STARTED:
        if ((btif_av_stream_started_ready() == TRUE))
          btif_a2dp_command_ack(A2DP_CTRL_ACK_SUCCESS);
        else
          btif_a2dp_command_ack(A2DP_CTRL_ACK_FAILURE);
        break;

      case A2DP_CTRL_CMD_START:
        /*
         * Don't send START request to stack while we are in a call.
         * Some headsets such as "Sony MW600", don't allow AVDTP START
         * while in a call, and respond with BAD_STATE.
         */
        if (!bluetooth::headset::btif_hf_is_call_vr_idle()) {
          btif_a2dp_command_ack(A2DP_CTRL_ACK_INCALL_FAILURE);
          break;
        }

        if (btif_av_is_handoff_set() && is_block_hal_start) {
          APPL_TRACE_WARNING("%s: A2DP command %s under handoff and HAL Start block",
                  __func__, audio_a2dp_hw_dump_ctrl_event(cmd));
          btif_a2dp_command_ack(A2DP_CTRL_ACK_SUCCESS);
          break;
        }

        if (btif_a2dp_source_is_streaming()) {
          APPL_TRACE_WARNING("%s: A2DP command %s while source is streaming, return",
                  __func__, audio_a2dp_hw_dump_ctrl_event(cmd));
          btif_a2dp_command_ack(A2DP_CTRL_ACK_SUCCESS);
          break;
        }

        rs_idx = btif_get_is_remote_started_idx();
        cur_idx = btif_av_get_latest_device_idx_to_start();

        APPL_TRACE_IMP("%s: RS Idx %d, Cur Idx %d, A2DP command %s",
                __func__, rs_idx, cur_idx, audio_a2dp_hw_dump_ctrl_event(cmd));

        if (btif_a2dp_source_is_remote_start()) {
          if (((rs_idx != btif_max_av_clients) && (rs_idx == cur_idx))
                  || (rs_idx == btif_max_av_clients)){
            btif_a2dp_source_cancel_remote_start();
            if (rs_idx != btif_max_av_clients)
              btif_dispatch_sm_event(
                      BTIF_AV_RESET_REMOTE_STARTED_FLAG_UPDATE_AUDIO_STATE_EVT, &rs_idx, sizeof(rs_idx));
                      APPL_TRACE_WARNING("%s: Cancel RS timer for the current index",
                      __func__);
            } else {
              APPL_TRACE_WARNING("%s: RS timer running on other index, ignore",
                      __func__);
            }
        }

        /* In Dual A2dp, first check for started state of stream
         * as we dont want to START again as while doing Handoff
         * the stack state will be started, so it is not needed
         * to send START again, just open the media socket
         * and ACK the audio HAL.
         */
        if (btif_av_stream_started_ready()) {
          if ((rs_idx != btif_max_av_clients) && (rs_idx == cur_idx)) {
            uint8_t hdl = btif_av_get_av_hdl_from_idx(rs_idx);
            APPL_TRACE_DEBUG("%s: setup codec idx %d hdl = %d",
                    __func__, rs_idx, hdl);
            if (hdl >= 0)
              btif_a2dp_source_setup_codec(hdl);
          }
          UIPC_Open(UIPC_CH_ID_AV_AUDIO, btif_a2dp_data_cb);
          btif_a2dp_command_ack(A2DP_CTRL_ACK_SUCCESS);
          APPL_TRACE_WARNING("%s: A2DP command %s while AV stream is alreday started",
                  __func__, audio_a2dp_hw_dump_ctrl_event(cmd));
          break;
        } else if (btif_av_stream_ready()) {
          /* Setup audio data channel listener */
          UIPC_Open(UIPC_CH_ID_AV_AUDIO, btif_a2dp_data_cb);
          /*
           * Post start event and wait for audio path to open.
           * If we are the source, the ACK will be sent after the start
           * procedure is completed, othewise send it now.
           */
          if (cur_idx <  btif_max_av_clients &&
              btif_av_is_state_opened(cur_idx)) {
            btif_dispatch_sm_event(BTIF_AV_START_STREAM_REQ_EVT, NULL, 0);
            if (btif_av_get_peer_sep() == AVDT_TSEP_SRC)
              btif_a2dp_command_ack(A2DP_CTRL_ACK_SUCCESS);
          break;
          }
        } else if (btif_av_is_handoff_set() && !(is_block_hal_start)) {
          APPL_TRACE_DEBUG("%s: Entertain Audio Start after stream open", __func__);
          UIPC_Open(UIPC_CH_ID_AV_AUDIO, btif_a2dp_data_cb);
          btif_dispatch_sm_event(BTIF_AV_START_STREAM_REQ_EVT, NULL, 0);
          if (btif_av_get_peer_sep() == AVDT_TSEP_SRC)
            btif_a2dp_command_ack(A2DP_CTRL_ACK_SUCCESS);
          break;
        }

        APPL_TRACE_WARNING("%s: A2DP command %s while AV stream is not ready",
                __func__, audio_a2dp_hw_dump_ctrl_event(cmd));
        btif_a2dp_command_ack(A2DP_CTRL_ACK_FAILURE);
        break;

      case A2DP_CTRL_CMD_STOP: {
        if (btif_av_get_peer_sep() == AVDT_TSEP_SNK &&
                !btif_a2dp_source_is_streaming()) {
          /* We are already stopped, just ack back */
          btif_a2dp_command_ack(A2DP_CTRL_ACK_SUCCESS);
          break;
        }

        btif_dispatch_sm_event(BTIF_AV_STOP_STREAM_REQ_EVT, NULL, 0);
        btif_a2dp_command_ack(A2DP_CTRL_ACK_SUCCESS);
        break;
      }

      case A2DP_CTRL_CMD_SUSPEND:
        /* Local suspend */
        rs_idx = btif_get_is_remote_started_idx();
        cur_idx = btif_av_get_latest_device_idx_to_start();

        APPL_TRACE_IMP("%s: RS Idx %d, Cur Idx %d, A2DP command %s",
                __func__, rs_idx, cur_idx, audio_a2dp_hw_dump_ctrl_event(cmd));

        if (btif_av_stream_started_ready()) {
          if (rs_idx != btif_max_av_clients) {
            bool is_playing_othr_idx = btif_av_is_playing_on_other_idx(rs_idx);
            if (is_playing_othr_idx) {
              APPL_TRACE_DEBUG("%s: Other Idx than RS Idx %d is started",
                      __func__, rs_idx);
              btif_dispatch_sm_event(BTIF_AV_SUSPEND_STREAM_REQ_EVT, NULL, 0);
              break;
            } else {
              APPL_TRACE_DEBUG("%s: Idx %d is remote started, ACK back",
                                                                __func__, rs_idx);
            }
          } else {
            APPL_TRACE_DEBUG("%s: No idx in remote started, trigger suspend",
                    __func__);
            btif_dispatch_sm_event(BTIF_AV_SUSPEND_STREAM_REQ_EVT, NULL, 0);
            break;
          }
        }
        /* If we are not in started state, just ack back ok and let
         * audioflinger close the channel. This can happen if we are
         * remotely suspended, clear REMOTE SUSPEND flag.
         */
        btif_av_clear_remote_suspend_flag();
        btif_a2dp_command_ack(A2DP_CTRL_ACK_SUCCESS);
        break;

      case A2DP_CTRL_GET_INPUT_AUDIO_CONFIG: {
        tA2DP_SAMPLE_RATE sample_rate = btif_a2dp_sink_get_sample_rate();
        tA2DP_CHANNEL_COUNT channel_count = btif_a2dp_sink_get_channel_count();

        btif_a2dp_command_ack(A2DP_CTRL_ACK_SUCCESS);
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0, reinterpret_cast<uint8_t*>(&sample_rate),
                sizeof(tA2DP_SAMPLE_RATE));
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0, &channel_count,
                sizeof(tA2DP_CHANNEL_COUNT));
        break;
      }

      case A2DP_CTRL_GET_OUTPUT_AUDIO_CONFIG: {
        btav_a2dp_codec_config_t codec_config;
        btav_a2dp_codec_config_t codec_capability;
        codec_config.sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_NONE;
        codec_config.bits_per_sample = BTAV_A2DP_CODEC_BITS_PER_SAMPLE_NONE;
        codec_config.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_NONE;
        codec_capability.sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_NONE;
        codec_capability.bits_per_sample = BTAV_A2DP_CODEC_BITS_PER_SAMPLE_NONE;
        codec_capability.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_NONE;

        A2dpCodecConfig* current_codec = bta_av_get_a2dp_current_codec();
        if (current_codec != nullptr) {
          codec_config = current_codec->getCodecConfig();
          codec_capability = current_codec->getCodecCapability();
        }

        btif_a2dp_command_ack(A2DP_CTRL_ACK_SUCCESS);
        // Send the current codec config
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0,
                reinterpret_cast<const uint8_t*>(&codec_config.sample_rate),
                sizeof(btav_a2dp_codec_sample_rate_t));
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0,
                reinterpret_cast<const uint8_t*>(&codec_config.bits_per_sample),
                sizeof(btav_a2dp_codec_bits_per_sample_t));
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0,
                reinterpret_cast<const uint8_t*>(&codec_config.channel_mode),
                sizeof(btav_a2dp_codec_channel_mode_t));
        // Send the current codec capability
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0,
                reinterpret_cast<const uint8_t*>(&codec_capability.sample_rate),
                sizeof(btav_a2dp_codec_sample_rate_t));
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0, reinterpret_cast<const uint8_t*>(
                                           &codec_capability.bits_per_sample),
                sizeof(btav_a2dp_codec_bits_per_sample_t));
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0, reinterpret_cast<const uint8_t*>(
                                           &codec_capability.channel_mode),
                sizeof(btav_a2dp_codec_channel_mode_t));
        break;
      }

      case A2DP_CTRL_SET_OUTPUT_AUDIO_CONFIG: {
        btav_a2dp_codec_config_t codec_config;
        codec_config.codec_type = BTAV_A2DP_CODEC_INDEX_SOURCE_MIN;
        codec_config.codec_priority = BTAV_A2DP_CODEC_PRIORITY_DEFAULT;
        codec_config.codec_specific_1 = 0;
        codec_config.codec_specific_2 = 0;
        codec_config.codec_specific_3 = 0;
        codec_config.codec_specific_4 = 0;
        codec_config.sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_NONE;
        codec_config.bits_per_sample = BTAV_A2DP_CODEC_BITS_PER_SAMPLE_NONE;
        codec_config.channel_mode = BTAV_A2DP_CODEC_CHANNEL_MODE_NONE;

        btif_a2dp_command_ack(A2DP_CTRL_ACK_SUCCESS);
        // Send the current codec config
        if (UIPC_Read(UIPC_CH_ID_AV_CTRL, 0,
                reinterpret_cast<uint8_t*>(&codec_config.sample_rate),
                sizeof(btav_a2dp_codec_sample_rate_t)) !=
                sizeof(btav_a2dp_codec_sample_rate_t)) {
          APPL_TRACE_ERROR("%s: Error reading sample rate from audio HAL",
                  __func__);
          break;
        }
        if (UIPC_Read(UIPC_CH_ID_AV_CTRL, 0,
                reinterpret_cast<uint8_t*>(&codec_config.bits_per_sample),
                sizeof(btav_a2dp_codec_bits_per_sample_t)) !=
                sizeof(btav_a2dp_codec_bits_per_sample_t)) {
          APPL_TRACE_ERROR("%s: Error reading bits per sample from audio HAL",
                  __func__);
          break;
        }
        if (UIPC_Read(UIPC_CH_ID_AV_CTRL, 0,
                reinterpret_cast<uint8_t*>(&codec_config.channel_mode),
                sizeof(btav_a2dp_codec_channel_mode_t)) !=
                sizeof(btav_a2dp_codec_channel_mode_t)) {
          APPL_TRACE_ERROR("%s: Error reading channel mode from audio HAL",
                  __func__);
          break;
        }
        APPL_TRACE_DEBUG(
                "%s: A2DP_CTRL_SET_OUTPUT_AUDIO_CONFIG: "
                "sample_rate=0x%x bits_per_sample=0x%x "
                "channel_mode=0x%x",
                __func__, codec_config.sample_rate, codec_config.bits_per_sample,
                codec_config.channel_mode);
        btif_a2dp_source_feeding_update_req(codec_config);
        break;
      }

      case A2DP_CTRL_CMD_OFFLOAD_START: {
        uint8_t hdl = 0;
        int idx = btif_get_is_remote_started_idx();
        if (idx < btif_max_av_clients) {
          hdl = btif_av_get_av_hdl_from_idx(idx);
          APPL_TRACE_DEBUG("%s: hdl = %d",__func__, hdl);
        } else {
          APPL_TRACE_ERROR("%s: Invalid index",__func__);
          break;
        }
        btif_dispatch_sm_event(BTIF_AV_OFFLOAD_START_REQ_EVT, (char *)&hdl, 1);
        break;
      }
      case A2DP_CTRL_GET_SINK_LATENCY: {
        tA2DP_LATENCY sink_latency;

        sink_latency = btif_av_get_sink_latency();
        btif_a2dp_command_ack(A2DP_CTRL_ACK_SUCCESS);
        /* Send sink latency */
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0,
                reinterpret_cast<const uint8_t*>(&sink_latency),
                sizeof(tA2DP_LATENCY));
        break;
      }

      case A2DP_CTRL_CMD_STREAM_OPEN:
        APPL_TRACE_DEBUG("Accept Audio Start after Stream open");
        is_block_hal_start = false;
        btif_a2dp_source_cancel_unblock_audio_start();
        btif_a2dp_command_ack(A2DP_CTRL_ACK_SUCCESS);
        break;

      case A2DP_CTRL_GET_PRESENTATION_POSITION: {
        btif_a2dp_command_ack(A2DP_CTRL_ACK_SUCCESS);
        int idx = btif_av_get_current_playing_dev_idx();
        uint16_t audio_delay = (idx < btif_max_av_clients) ? delay_report_stats.audio_delay[idx]:0;
        APPL_TRACE_DEBUG("Delay Rpt: total bytes read = %d", delay_report_stats.total_bytes_read);
        APPL_TRACE_DEBUG("Delay Rpt: delay = %d, index: %d", audio_delay, idx);
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0,
                  (uint8_t*)&(delay_report_stats.total_bytes_read),
                  sizeof(uint64_t));
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0,
                  (uint8_t*)&(audio_delay), sizeof(uint16_t));

        uint32_t seconds = delay_report_stats.timestamp.tv_sec;
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0, (uint8_t*)&seconds, sizeof(seconds));

        uint32_t nsec = delay_report_stats.timestamp.tv_nsec;
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0, (uint8_t*)&nsec, sizeof(nsec));
        APPL_TRACE_DEBUG("Delay Rpt: seconds = %d, nsec = %d" ,seconds, nsec);
        break;
      }

      default:
        APPL_TRACE_ERROR("%s: UNSUPPORTED CMD (%d)", __func__, cmd);
        btif_a2dp_command_ack(A2DP_CTRL_ACK_FAILURE);
        break;
    }
  }
}

void btif_a2dp_snd_ctrl_cmd(tA2DP_CTRL_CMD cmd) {
  int rs_idx, cur_idx;

  switch (cmd) {
    case A2DP_CTRL_CMD_START:
      /*
       * Don't send START request to stack while we are in a call.
       * Some headsets such as "Sony MW600", don't allow AVDTP START
       * while in a call, and respond with BAD_STATE.
       */
      if (!bluetooth::headset::btif_hf_is_call_vr_idle()) {
        btif_a2dp_command_ack(A2DP_CTRL_ACK_INCALL_FAILURE);
        break;
      }

      if (btif_av_is_handoff_set() && is_block_hal_start) {
        APPL_TRACE_WARNING("%s: A2DP command %s under handoff and HAL Start block",
                __func__, audio_a2dp_hw_dump_ctrl_event(cmd));
        btif_a2dp_command_ack(A2DP_CTRL_ACK_SUCCESS);
        break;
      }

      if (btif_a2dp_source_is_streaming()) {
        APPL_TRACE_WARNING("%s: A2DP command %s while source is streaming, return",
                __func__, audio_a2dp_hw_dump_ctrl_event(cmd));
        btif_a2dp_command_ack(A2DP_CTRL_ACK_SUCCESS);
        break;
      }

      rs_idx = btif_get_is_remote_started_idx();
      cur_idx = btif_av_get_latest_device_idx_to_start();

      APPL_TRACE_IMP("%s: RS Idx %d, Cur Idx %d, A2DP command %s",
        __func__, rs_idx, cur_idx, audio_a2dp_hw_dump_ctrl_event(cmd));

      if (btif_a2dp_source_is_remote_start()) {
        if (((rs_idx != btif_max_av_clients) && (rs_idx == cur_idx))
                || (rs_idx == btif_max_av_clients)){
          btif_a2dp_source_cancel_remote_start();
          if (rs_idx != btif_max_av_clients)
            btif_dispatch_sm_event(BTIF_AV_RESET_REMOTE_STARTED_FLAG_UPDATE_AUDIO_STATE_EVT,
                    &rs_idx, sizeof(rs_idx));
          APPL_TRACE_WARNING("%s: Cancel RS timer for the current index", __func__);
        } else {
          APPL_TRACE_WARNING("%s: RS timer running on other index, ignore",
                  __func__);
        }
      }

      /* In Dual A2dp, first check for started state of stream
       * as we dont want to START again as while doing Handoff
       * the stack state will be started, so it is not needed
       * to send START again, just open the media socket
       * and ACK the audio HAL.
       */
      if (btif_av_stream_started_ready()) {
        if ((rs_idx != btif_max_av_clients) && (rs_idx == cur_idx)) {
          uint8_t hdl = btif_av_get_av_hdl_from_idx(rs_idx);
          APPL_TRACE_DEBUG("%s: setup codec idx %d hdl = %d",
                  __func__, rs_idx, hdl);
          if (hdl >= 0)
            btif_a2dp_source_setup_codec(hdl);
        }
        UIPC_Open(UIPC_CH_ID_AV_AUDIO, btif_a2dp_data_cb);
        btif_a2dp_command_ack(A2DP_CTRL_ACK_SUCCESS);
        APPL_TRACE_WARNING("%s: A2DP command %s while AV stream is alreday started",
                __func__, audio_a2dp_hw_dump_ctrl_event(cmd));
        break;
      } else if (btif_av_stream_ready()) {
        /* Setup audio data channel listener */
        UIPC_Open(UIPC_CH_ID_AV_AUDIO, btif_a2dp_data_cb);
        /*
         * Post start event and wait for audio path to open.
         * If we are the source, the ACK will be sent after the start
         * procedure is completed, othewise send it now.
         */
        if (cur_idx <  btif_max_av_clients &&
                btif_av_is_state_opened(cur_idx)) {
          btif_dispatch_sm_event(BTIF_AV_START_STREAM_REQ_EVT, NULL, 0);
          if (btif_av_get_peer_sep() == AVDT_TSEP_SRC)
            btif_a2dp_command_ack(A2DP_CTRL_ACK_SUCCESS);
          break;
        }
      } else if (btif_av_is_handoff_set() && !(is_block_hal_start)) {
        APPL_TRACE_DEBUG("%s: Entertain Audio Start after stream open", __func__);
        UIPC_Open(UIPC_CH_ID_AV_AUDIO, btif_a2dp_data_cb);
        btif_dispatch_sm_event(BTIF_AV_START_STREAM_REQ_EVT, NULL, 0);
        if (btif_av_get_peer_sep() == AVDT_TSEP_SRC)
          btif_a2dp_command_ack(A2DP_CTRL_ACK_SUCCESS);
        break;
      }

      APPL_TRACE_WARNING("%s: A2DP command %s while AV stream is not ready",
              __func__, audio_a2dp_hw_dump_ctrl_event(cmd));
      btif_a2dp_command_ack(A2DP_CTRL_ACK_FAILURE);
      break;

    case A2DP_CTRL_CMD_STOP: {
      if (btif_av_get_peer_sep() == AVDT_TSEP_SNK &&
              !btif_a2dp_source_is_streaming()) {
        /* We are already stopped, just ack back */
        btif_a2dp_command_ack(A2DP_CTRL_ACK_SUCCESS);
        break;
      }

      btif_dispatch_sm_event(BTIF_AV_STOP_STREAM_REQ_EVT, NULL, 0);
      btif_a2dp_command_ack(A2DP_CTRL_ACK_SUCCESS);
      break;
    }

    case A2DP_CTRL_CMD_SUSPEND:
      /* Local suspend */
      rs_idx = btif_get_is_remote_started_idx();
      cur_idx = btif_av_get_latest_device_idx_to_start();

      APPL_TRACE_IMP("%s: RS Idx %d, Cur Idx %d, A2DP command %s",
              __func__, rs_idx, cur_idx, audio_a2dp_hw_dump_ctrl_event(cmd));

      if (btif_av_stream_started_ready()) {
        if (rs_idx != btif_max_av_clients) {
          bool is_playing_othr_idx = btif_av_is_playing_on_other_idx(rs_idx);
          if (is_playing_othr_idx) {
            APPL_TRACE_DEBUG("%s: Other Idx than RS Idx %d is started",
                                                            __func__, rs_idx);
            btif_dispatch_sm_event(BTIF_AV_SUSPEND_STREAM_REQ_EVT, NULL, 0);
            break;
          } else {
            APPL_TRACE_DEBUG("%s: Idx %d is remote started, ACK back",
                    __func__, rs_idx);
          }
        } else {
          APPL_TRACE_DEBUG("%s: No idx in remote started, trigger suspend",
                  __func__);
          btif_dispatch_sm_event(BTIF_AV_SUSPEND_STREAM_REQ_EVT, NULL, 0);
          break;
        }
      }
      /* If we are not in started state, just ack back ok and let
       * audioflinger close the channel. This can happen if we are
       * remotely suspended, clear REMOTE SUSPEND flag.
       */
      btif_av_clear_remote_suspend_flag();
      btif_a2dp_command_ack(A2DP_CTRL_ACK_SUCCESS);
      break;

    default:
      APPL_TRACE_ERROR("UNSUPPORTED CMD (%d)", cmd);
      btif_a2dp_command_ack(A2DP_CTRL_ACK_FAILURE);
      break;
    }

    // Don't log A2DP_CTRL_GET_PRESENTATION_POSITION by default, because it
    // could be very chatty when audio is streaming.
    if (cmd == A2DP_CTRL_GET_PRESENTATION_POSITION) {
      APPL_TRACE_DEBUG("btif_a2dp_snd_ctrl_cmd: %s DONE", audio_a2dp_hw_dump_ctrl_event(cmd));
    } else {
      APPL_TRACE_IMP("btif_a2dp_snd_ctrl_cmd: %s DONE", audio_a2dp_hw_dump_ctrl_event(cmd));
    }
}

static void btif_a2dp_ctrl_cb(UNUSED_ATTR tUIPC_CH_ID ch_id,
                              tUIPC_EVENT event) {
  // Don't log UIPC_RX_DATA_READY_EVT by default, because it
  // could be very chatty when audio is streaming.
  if (event == UIPC_RX_DATA_READY_EVT) {
    APPL_TRACE_DEBUG("%s: A2DP-CTRL-CHANNEL EVENT %s", __func__,
                     dump_uipc_event(event));
  } else {
    APPL_TRACE_WARNING("%s: A2DP-CTRL-CHANNEL EVENT %s", __func__,
                       dump_uipc_event(event));
  }

  switch (event) {
    case UIPC_OPEN_EVT:
      break;

    case UIPC_CLOSE_EVT:
      /* restart ctrl server unless we are shutting down */
      if (btif_a2dp_source_media_task_is_running())
        UIPC_Open(UIPC_CH_ID_AV_CTRL, btif_a2dp_ctrl_cb);
      break;

    case UIPC_RX_DATA_READY_EVT:
      btif_a2dp_recv_ctrl_data();
      break;

    default:
      APPL_TRACE_ERROR("%s: ### A2DP-CTRL-CHANNEL EVENT %d NOT HANDLED ###",
                       __func__, event);
      break;
  }
}

static void btif_a2dp_data_cb(UNUSED_ATTR tUIPC_CH_ID ch_id,
                              tUIPC_EVENT event) {
  APPL_TRACE_WARNING("%s: BTIF MEDIA (A2DP-DATA) EVENT %s", __func__,
                     dump_uipc_event(event));

  switch (event) {
    case UIPC_OPEN_EVT: {
      /*
       * Read directly from media task from here on (keep callback for
       * connection events.
       */
      UIPC_Ioctl(UIPC_CH_ID_AV_AUDIO, UIPC_REG_REMOVE_ACTIVE_READSET, NULL);
      UIPC_Ioctl(UIPC_CH_ID_AV_AUDIO, UIPC_SET_READ_POLL_TMO,
                 reinterpret_cast<void*>(A2DP_DATA_READ_POLL_MS));

      if (btif_av_get_peer_sep() == AVDT_TSEP_SNK) {
        /* Start the media task to encode the audio */
        btif_a2dp_source_start_audio_req();
      }

      /* ACK back when media task is fully started */
      break;
    }

    case UIPC_CLOSE_EVT:
      APPL_TRACE_EVENT("%s: ## AUDIO PATH DETACHED ##", __func__);

      if (property_get("persist.vendor.bt.a2dp.hal.implementation", a2dp_hal_imp, "false") &&
            !strcmp(a2dp_hal_imp, "true")) {
        APPL_TRACE_IMP("command %s pending", audio_a2dp_hw_dump_ctrl_event(a2dp_cmd_pending));
        APPL_TRACE_IMP("command %s queued", audio_a2dp_hw_dump_ctrl_event(a2dp_cmd_queued));
        /*
         * Send stop request only if we are actively streaming and haven't
         * received a stop/suspend request. Potentially, the audioflinger detached
         * abnormally or remote delayed the ACK.
         */
        if (btif_a2dp_source_is_streaming() &&
                (!((a2dp_cmd_pending == A2DP_CTRL_CMD_STOP) ||
                (a2dp_cmd_pending == A2DP_CTRL_CMD_SUSPEND))) &&
                (!((a2dp_cmd_queued == A2DP_CTRL_CMD_STOP) ||
                (a2dp_cmd_queued == A2DP_CTRL_CMD_SUSPEND)))) {
          APPL_TRACE_IMP("Trigger Stop stream request as data channel got detached");
          /* Post stop event and wait for audio path to stop */
          btif_dispatch_sm_event(BTIF_AV_STOP_STREAM_REQ_EVT, NULL, 0);
        }
      } else {
        btif_a2dp_command_ack(A2DP_CTRL_ACK_SUCCESS);
        /*
         * Send stop request only if we are actively streaming and haven't
         * received a stop request. Potentially, the audioflinger detached
         * abnormally.
         */
        if (btif_a2dp_source_is_streaming()) {
          /* Post stop event and wait for audio path to stop */
          btif_dispatch_sm_event(BTIF_AV_STOP_STREAM_REQ_EVT, NULL, 0);
        }
      }
      break;

    default:
      APPL_TRACE_ERROR("%s: ### A2DP-DATA EVENT %d NOT HANDLED ###", __func__,
                       event);
      break;
  }
}

void btif_a2dp_command_ack(tA2DP_CTRL_ACK status) {
  uint8_t ack = status;

  // Don't log A2DP_CTRL_GET_PRESENTATION_POSITION by default, because it
  // could be very chatty when audio is streaming.
  if (a2dp_cmd_pending == A2DP_CTRL_GET_PRESENTATION_POSITION) {
    APPL_TRACE_DEBUG("%s: ## a2dp ack : %s, queued : %s,  status %d ##", __func__,
            audio_a2dp_hw_dump_ctrl_event(a2dp_cmd_pending),
            audio_a2dp_hw_dump_ctrl_event(a2dp_cmd_queued), status);
  } else {
    APPL_TRACE_WARNING("%s: ## a2dp ack : %s, queued : %s,  status %d ##", __func__,
            audio_a2dp_hw_dump_ctrl_event(a2dp_cmd_pending),
            audio_a2dp_hw_dump_ctrl_event(a2dp_cmd_queued), status);
  }

  /* Sanity check */
  if (property_get("persist.vendor.bt.a2dp.hal.implementation", a2dp_hal_imp, "false") &&
          !strcmp(a2dp_hal_imp, "true")) {
    if (a2dp_cmd_pending == A2DP_CTRL_CMD_NONE) {
      APPL_TRACE_ERROR("btif_a2dp_command_ack: warning : no command pending, ignore ack");
      if (a2dp_cmd_queued != A2DP_CTRL_CMD_NONE)
      {
        APPL_TRACE_ERROR("btif_a2dp_command_ack: command %s queued when no command pending, reset",
                audio_a2dp_hw_dump_ctrl_event(a2dp_cmd_queued));
        a2dp_cmd_queued = A2DP_CTRL_CMD_NONE;
      }
      return;
    } else if ((ack == A2DP_CTRL_ACK_SUCCESS) &&
            (a2dp_cmd_queued != A2DP_CTRL_CMD_NONE) &&
            (a2dp_cmd_pending != a2dp_cmd_queued)) {
      // No need to send ACK to HAL as it is already timed out for current command
      // Not clearing queued command to identify later whether on completion of the same HAL to be ACKed back
      APPL_TRACE_IMP("btif_a2dp_command_ack: queued command %s to be sent, on completion of %s",
              audio_a2dp_hw_dump_ctrl_event(a2dp_cmd_queued),
              audio_a2dp_hw_dump_ctrl_event(a2dp_cmd_pending));
      a2dp_cmd_pending = a2dp_cmd_queued;
      btif_a2dp_snd_ctrl_cmd(a2dp_cmd_pending);
      return;
    } else {
      // On current command ack failure, we do not process queued command, but flush it
      /* clear pending and queued*/
      a2dp_cmd_pending = A2DP_CTRL_CMD_NONE;
      if (a2dp_cmd_queued != A2DP_CTRL_CMD_NONE) {
        APPL_TRACE_IMP("btif_a2dp_command_ack: Not acking as ack is waited for queued command");
        /* no need to ack as we alreday unblocked HAL with error
            A2DP_CTRL_ACK_PREVIOUS_COMMAND_PENDING in case of queued command*/
        a2dp_cmd_queued = A2DP_CTRL_CMD_NONE;
      } else {
        /* acknowledge start request */
        UIPC_Send(UIPC_CH_ID_AV_CTRL, 0, &ack, sizeof(ack));
      }
    }
  } else {
    /* Sanity check */
    if (a2dp_cmd_pending == A2DP_CTRL_CMD_NONE) {
      APPL_TRACE_ERROR("%s: warning : no command pending, ignore ack", __func__);
      return;
    }

    /* Clear pending */
    a2dp_cmd_pending = A2DP_CTRL_CMD_NONE;

    /* Acknowledge start request */
    UIPC_Send(UIPC_CH_ID_AV_CTRL, 0, &ack, sizeof(ack));
  }
}
tA2DP_CTRL_CMD btif_a2dp_control_get_pending_command() {
  return a2dp_cmd_pending;
}

void btif_a2dp_control_log_bytes_read(uint32_t bytes_read) {
  delay_report_stats.total_bytes_read += bytes_read;
  clock_gettime(CLOCK_MONOTONIC, &delay_report_stats.timestamp);
}

void btif_a2dp_control_set_audio_delay(uint16_t delay, int index) {
  APPL_TRACE_DEBUG("%s: DELAY: %.1f ms, index: %d", __func__, (float)delay / 10, index);
  delay_report_stats.audio_delay[index] = delay;
}

void btif_a2dp_control_reset_audio_delay(int index) {
  APPL_TRACE_DEBUG("%s", __func__);
  delay_report_stats.audio_delay[index] = 0;
  delay_report_stats.total_bytes_read = 0;
  delay_report_stats.timestamp = {};
}

// For Split-A2DP
uint16_t btif_a2dp_control_get_audio_delay(int index) {
  // delay report is the audio delay from the remote headset receiving
  // data to the headset playing sound in units of 1/10ms
  uint16_t delay = delay_report_stats.audio_delay[index] / 10;
  APPL_TRACE_DEBUG("%s: DELAY: %d ms, index: %d", __func__, delay, index);
  return (delay > 0) ? delay : 0;
}

void btif_a2dp_pending_cmds_reset() {
  a2dp_cmd_pending = A2DP_CTRL_CMD_NONE;
  a2dp_cmd_queued = A2DP_CTRL_CMD_NONE;
}
