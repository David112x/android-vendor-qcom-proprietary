/******************************************************************************
 *  Copyright (C) 2017, The Linux Foundation. All rights reserved.
 *  Not a Contribution.
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted (subject to the limitations in the
 *  disclaimer below) provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.

    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

 *  NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 *  GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 *  HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 *  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 *  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 *  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 *  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

#ifndef BTIF_A2DP_SOURCE_H
#define BTIF_A2DP_SOURCE_H

#include <stdbool.h>

#include "bta_av_api.h"

typedef struct {
  bool vs_configs_exchanged;
  bool tx_started;
  bool tx_stop_initiated;
  bool tx_start_initiated;
  bool multi_vsc_support;
  bool start_reset;
} tBTIF_A2DP_SOURCE_VSC;

typedef struct {
  // Counter for total updates
  size_t total_updates;

  // Last update timestamp (in us)
  uint64_t last_update_us;

  // Counter for overdue scheduling
  size_t overdue_scheduling_count;

  // Accumulated overdue scheduling deviations (in us)
  uint64_t total_overdue_scheduling_delta_us;

  // Max. overdue scheduling delta time (in us)
  uint64_t max_overdue_scheduling_delta_us;

  // Counter for premature scheduling
  size_t premature_scheduling_count;

  // Accumulated premature scheduling deviations (in us)
  uint64_t total_premature_scheduling_delta_us;

  // Max. premature scheduling delta time (in us)
  uint64_t max_premature_scheduling_delta_us;

  // Counter for exact scheduling
  size_t exact_scheduling_count;

  // Accumulated and counted scheduling time (in us)
  uint64_t total_scheduling_time_us;
} scheduling_stats_t;

typedef struct {
  uint64_t session_start_us;
  uint64_t session_end_us;

  scheduling_stats_t tx_queue_enqueue_stats;
  scheduling_stats_t tx_queue_dequeue_stats;

  size_t tx_queue_total_frames;
  size_t tx_queue_max_frames_per_packet;

  uint64_t tx_queue_total_queueing_time_us;
  uint64_t tx_queue_max_queueing_time_us;

  size_t tx_queue_total_readbuf_calls;
  uint64_t tx_queue_last_readbuf_us;

  size_t tx_queue_total_flushed_messages;
  uint64_t tx_queue_last_flushed_us;
  size_t tx_queue_total_dropped_messages;
  size_t tx_queue_max_dropped_messages;
  size_t tx_queue_dropouts;
  uint64_t tx_queue_last_dropouts_us;

  size_t media_read_total_underflow_bytes;
  size_t media_read_total_underflow_count;
  uint64_t media_read_last_underflow_us;
} btif_media_stats_t;

typedef struct {
  thread_t* worker_thread;
  fixed_queue_t* cmd_msg_queue;
  fixed_queue_t* tx_audio_queue;
  bool tx_flush; /* Discards any outgoing data when true */
  alarm_t* unblock_audio_start_alarm;
  alarm_t* media_alarm;
  alarm_t *remote_start_alarm;
  const tA2DP_ENCODER_INTERFACE* encoder_interface;
  period_ms_t encoder_interval_ms; /* Local copy of the encoder interval */
  btif_media_stats_t stats;
  btif_media_stats_t accumulated_stats;
  int last_remote_started_index;
  int *last_started_index_pointer;
} tBTIF_A2DP_SOURCE_CB;

// Initialize and startup the A2DP Source module.
// This function should be called by the BTIF state machine prior to using the
// module.
bool btif_a2dp_source_startup(void);

// Shutdown and cleanup the A2DP Source module.
// This function should be called by the BTIF state machine during
// graceful shutdown and cleanup.
void btif_a2dp_source_shutdown(void);

// Check whether the A2DP Source media task is running.
// Returns true if the A2DP Source media task is running, otherwise false.
bool btif_a2dp_source_media_task_is_running(void);

// Check whether the A2DP Source media task is shutting down.
// Returns true if the A2DP Source media task is shutting down.
bool btif_a2dp_source_media_task_is_shutting_down(void);

// Return true if the A2DP Source module is streaming.
bool btif_a2dp_source_is_streaming(void);

// Return true if the A2DP remote is trying to start the session
bool btif_a2dp_source_is_remote_start(void);

// Cancel remote start alarm
void btif_a2dp_source_cancel_remote_start();

//return last remote started index
int btif_a2dp_source_last_remote_start_index();

void btif_a2dp_source_cancel_unblock_audio_start(void);

// Setup the A2DP Source codec, and prepare the encoder.
// This function should be called prior to starting A2DP streaming.
bt_status_t btif_a2dp_source_setup_codec(tBTA_AV_HNDL handle);

// Process a request to start the A2DP audio encoding task.
void btif_a2dp_source_start_audio_req(void);

// Process a request to stop the A2DP audio encoding task.
void btif_a2dp_source_stop_audio_req(void);

// Process a request to update the A2DP audio encoder with user preferred
// codec configuration.
// |codec_user_config| contains the preferred codec user configuration.
void btif_a2dp_source_encoder_user_config_update_req(
    const btav_a2dp_codec_config_t& codec_user_config, const RawAddress& bd_addr);

// Process a request to update the A2DP audio encoding with new audio
// configuration feeding parameters stored in |codec_audio_config|.
// The fields that are used are: |codec_audio_config.sample_rate|,
// |codec_audio_config.bits_per_sample| and |codec_audio_config.channel_mode|.
void btif_a2dp_source_feeding_update_req(
    const btav_a2dp_codec_config_t& codec_audio_config);

// Process 'idle' request from the BTIF state machine during initialization.
void btif_a2dp_source_on_idle(void);

// Process 'stop' request from the BTIF state machine to stop A2DP streaming.
// |p_av_suspend| is the data associated with the request - see
// |tBTA_AV_SUSPEND|.
void btif_a2dp_source_on_stopped(tBTA_AV_SUSPEND* p_av_suspend);

// Process 'suspend' request from the BTIF state machine to suspend A2DP
// streaming.
// |p_av_suspend| is the data associated with the request - see
// |tBTA_AV_SUSPEND|.
void btif_a2dp_source_on_suspended(tBTA_AV_SUSPEND* p_av_suspend);

// Enable/disable discarding of transmitted frames.
// If |enable| is true, the discarding is enabled, otherwise is disabled.
void btif_a2dp_source_set_tx_flush(bool enable);

// Get the next A2DP buffer to send.
// Returns the next A2DP buffer to send if available, otherwise NULL.
BT_HDR* btif_a2dp_source_audio_readbuf(void);

// Dump debug-related information for the A2DP Source module.
// |fd| is the file descriptor to use for writing the ASCII formatted
// information.
void btif_a2dp_source_debug_dump(int fd);

// Update the A2DP Source related metrics.
// This function should be called before collecting the metrics.
void btif_a2dp_source_update_metrics(void);

// Honor remote avdtp start
// This function will start a 3 second timer. If the a2dp streaming is
// started within this time, then the timer will be cancelled. Else-If
// timer expires, avdpt suspend will be issued to the remote
void btif_a2dp_source_on_remote_start(struct alarm_t **remote_start_alarm, int index);

void btif_trigger_unblock_audio_start_recovery_timer(void);

// Return true if the HAL v2 is enabled
bool btif_a2dp_source_is_hal_v2_enabled(void);

// Return true if the HAL v2 is supported
bool btif_a2dp_source_is_hal_v2_supported(void);

// returns true incase of restart session needed
bool btif_a2dp_source_is_restart_session_needed();

// Start the A2DP Source session.
// This function should be called by the BTIF state machine after
// btif_a2dp_source_startup() to start the streaming session for |peer_address|.
bool btif_a2dp_source_start_session(const RawAddress& peer_address);

// Restart the A2DP Source session.
// This function should be called by the BTIF state machine after
// btif_a2dp_source_startup() to restart the streaming session.
// |old_peer_address| is the peer address of the old session. This address
// can be empty.
// |new_peer_address| is the peer address of the new session. This address
// cannot be empty.
bool btif_a2dp_source_restart_session(const RawAddress& old_peer_address,
                                      const RawAddress& new_peer_address);

// End the A2DP Source session.
// This function should be called by the BTIF state machine to end the
// streaming session for |peer_address|.
bool btif_a2dp_source_end_session(const RawAddress& peer_address);

// Initialise the a2dp source interface for split
// This function should be called by the BTIF state machine to init the
// split source interface.
void btif_a2dp_source_if_init(void);

// DeInitialise the a2dp source interface for split
// This function should be called by the BTIF state machine to deinit the
// split source interface.
void btif_a2dp_source_if_deinit(void);
// Function to update latency/delay value to Audio
void btif_a2dp_update_sink_latency_change();

// Function to process requests from HIDL requests
void btif_a2dp_source_process_request(tA2DP_CTRL_CMD cmd);

// Generic function to send ack to HAL interface of stack
void btif_a2dp_source_command_ack(tA2DP_CTRL_CMD cmd, tA2DP_CTRL_ACK status);
#endif /* BTIF_A2DP_SOURCE_H */
