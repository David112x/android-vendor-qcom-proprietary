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
namespace vendor {
namespace qti {
namespace btoffload {

extern int audio_stream_open_qti(void);
extern int audio_stream_close_qti(void);
extern int audio_stream_start_qti(void);
extern int audio_stream_stop_qti(void);
extern int audio_stream_suspend_qti(void);
extern void* audio_get_codec_config_qti(uint8_t *mcast, uint8_t *num_dev, audio_format_t *codec_type);
extern void audio_handoff_triggered_qti(void);
extern void clear_a2dp_suspend_flag_qti(void);
extern void* audio_get_next_codec_config(uint8_t idx, audio_format_t *codec_type);
extern int audio_check_a2dp_ready_qti(void);
extern uint16_t audio_get_a2dp_sink_latency_qti();
extern void bt_audio_pre_init_qti(void);
extern bool audio_is_scrambling_enabled_qti(void);

}  // namespace btoffload
}  // qti
}  // vendor
