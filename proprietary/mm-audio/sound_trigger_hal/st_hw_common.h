/* st_hw_common.h
 *
 * Contains common functionality between
 * sound trigger hw and sound trigger extension interface.

 * Copyright (c) 2016, 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "sound_trigger_hw.h"
#include "st_session.h"

bool st_hw_check_ses_ss_usecase_allowed(st_session_t *st_ses);
bool st_hw_check_lpi_support(struct sound_trigger_device *stdev, st_session_t *st_ses);
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
int generate_sound_trigger_recognition_config_payload(
    const void *sm_levels_generic, unsigned char **out_payload,
    unsigned int *out_payload_size, uint32_t version);
int generate_sound_trigger_recognition_config_payload_v2(
    const void *sm_levels_generic, unsigned char **out_payload,
    unsigned int *out_payload_size, uint32_t version);

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
