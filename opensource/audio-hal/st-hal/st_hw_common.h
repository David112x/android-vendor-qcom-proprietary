/* st_hw_common.h
 *
 * Contains common functionality between
 * sound trigger hw and sound trigger extension interface.
 *
 * Copyright (c) 2016, 2018-2019, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
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

#include "sound_trigger_hw.h"
#include "st_session.h"

bool st_hw_check_ses_ss_usecase_allowed(st_session_t *st_ses);
void st_hw_check_and_update_lpi(struct sound_trigger_device *stdev, st_session_t *st_ses);
bool st_hw_check_vad_support(struct sound_trigger_device *stdev, st_session_t *st_ses, bool lpi_enable);
void st_hw_check_and_set_lpi_mode(st_session_t *st_ses);
bool st_hw_check_multi_stage_lsm_support();
int st_hw_ses_update_config(st_session_t *st_ses, st_hw_session_t *st_hw_ses);

int stop_other_sessions(struct sound_trigger_device *stdev,
                        st_session_t *cur_ses);
int start_other_sessions(struct sound_trigger_device *stdev,
                        st_session_t *cur_ses);
st_session_t* get_sound_trigger_session(struct sound_trigger_device *stdev,
                                   sound_model_handle_t sound_model_handle);

/* callback to hw for session events */
typedef void (*hw_session_notify_callback_t)(sound_model_handle_t handle,
                                     st_session_event_id_t event);
int hw_session_notifier_init(hw_session_notify_callback_t cb);
void hw_session_notifier_deinit();
int hw_session_notifier_enqueue(sound_model_handle_t handle,
                                st_session_event_id_t event, uint64_t delay_ms);
int hw_session_notifier_cancel(sound_model_handle_t handle,
                               st_session_event_id_t event);

/* 2nd stage detection */
int st_hw_ses_get_hist_buff_payload(st_hw_session_t *p_ses,
    uint8_t *payload_buff, size_t buff_size);

static inline uint64_t get_current_time_ns()
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * NSECS_PER_SEC) + ts.tv_nsec;
}

static inline unsigned int convert_ms_to_bytes
(
    unsigned int input_ms,
    struct pcm_config *config
)
{
    return ((input_ms * config->rate * config->channels *
        (pcm_format_to_bits(config->format) >> 3)) / 1000);
}

static inline unsigned int convert_bytes_to_ms
(
    unsigned int input_bytes,
    struct pcm_config *config
)
{
    return ((input_bytes * 1000) / (config->rate * config->channels *
        (pcm_format_to_bits(config->format) >> 3)));
}
