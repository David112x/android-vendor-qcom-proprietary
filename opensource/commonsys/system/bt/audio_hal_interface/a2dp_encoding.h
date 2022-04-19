/*
 * Copyright 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "audio_a2dp_hw/include/audio_a2dp_hw.h"
#include <vendor/qti/hardware/bluetooth_audio/2.0/types.h>
#include "osi/include/thread.h"
#include "bta_av_api.h"

using vendor::qti::hardware::bluetooth_audio::V2_0::SessionType;
using vendor::qti::hardware::bluetooth_audio::V2_0::SessionParamType;

namespace bluetooth {
namespace audio {
namespace a2dp {

bool is_hal_2_0_supported();

// Check if new bluetooth_audio is enabled
bool is_hal_2_0_enabled();

// Initialize BluetoothAudio HAL: openProvider
bool init( thread_t* message_loop);

// Clean up BluetoothAudio HAL
void cleanup();

// Set up the codec into BluetoothAudio HAL
bool setup_codec();

// Send command to the BluetoothAudio HAL: StartSession, EndSession,
// StreamStarted, StreamSuspended
void start_session();
void end_session();
tA2DP_CTRL_CMD get_pending_command();
bool is_restart_session_needed();
void reset_pending_command();
void update_pending_command(tA2DP_CTRL_CMD cmd);
void ack_stream_started(const tA2DP_CTRL_ACK& status);
void ack_stream_suspended(const tA2DP_CTRL_ACK& status);

// Read from the FMQ of BluetoothAudio HAL
size_t read(uint8_t* p_buf, uint32_t len);

// Update A2DP delay report to BluetoothAudio HAL
void set_remote_delay(uint16_t delay_report);
bool is_streaming();
SessionType get_session_type();
void update_session_params(SessionParamType param_type);

}  // namespace a2dp
}  // namespace audio
}  // namespace bluetooth
