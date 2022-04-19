/* pa_qti_soundtrigger.h
 *
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef PA_QTI_SOUNDTRIGGER_H
#define PA_QTI_SOUNDTRIGGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <system/sound_trigger.h>

/*
 * Below are the definitions that are used by PA QTI ST client and
 * client wrapper.
 */
#define PA_QST_MODULE_ID_PRIMARY "soundtrigger.primary"
/* set/get parameter keys */
#define PA_QST_PARAMETER_CUSTOM_CHANNEL_MIXING "st_custom_channel_mixing"
#define PA_QST_PARAMETER_SESSION_PAUSE "st_session_pause"
#define PA_QST_PARAMETER_BAD_MIC_CHANNEL_INDEX "st_bad_mic_channel_index"
#define PA_QST_PARAMETER_DIRECTION_OF_ARRIVAL "st_direction_of_arrival"
#define PA_QST_PARAMETER_CHANNEL_INDEX "st_channel_index"
#define PA_QST_PARAMETER_EC_REF_DEVICE "st_ec_ref_device"

typedef void pa_qst_handle_t;
typedef int pa_qst_ses_handle_t;

struct pa_qst_phrase_recognition_event {
    struct sound_trigger_phrase_recognition_event phrase_event; /* key phrase recognition event */
    uint64_t timestamp; /* event time stamp */
};

struct pa_qst_source_tracking_param {
    int target_angle_L16[2];
    int interf_angle_L16[2];
    char polarActivityGUI[360];
};

struct pa_qst_target_channel_index_param {
    int target_chan_idx;
};

typedef union {
    struct pa_qst_source_tracking_param st_params;
    struct pa_qst_target_channel_index_param ch_index_params;
} pa_qst_get_param_payload_t;

typedef void (*pa_qst_recognition_callback_t)(struct sound_trigger_recognition_event *event,
                                              void *cookie);
typedef void (*pa_qst_sound_model_callback_t)(struct sound_trigger_model_event *event,
                                              void *cookie);

/*
 * Below are the API definitions that are used by PA QTI ST client
 */
int pa_qst_load_sound_model(const pa_qst_handle_t *mod_handle,
                            struct sound_trigger_sound_model *sound_model,
                            pa_qst_sound_model_callback_t callback,
                            void *cookie,
                            pa_qst_ses_handle_t *handle);

int pa_qst_unload_sound_model(const pa_qst_handle_t *mod_handle,
                              pa_qst_ses_handle_t handle);

int pa_qst_start_recognition(const pa_qst_handle_t *mod_handle,
                             pa_qst_ses_handle_t sound_model_handle,
                             const struct sound_trigger_recognition_config *config,
                             pa_qst_recognition_callback_t callback,
                             void *cookie);

int pa_qst_stop_recognition(const pa_qst_handle_t *mod_handle,
                            pa_qst_ses_handle_t sound_model_handle);

int pa_qst_set_parameters(const pa_qst_handle_t *mod_handle,
                          pa_qst_ses_handle_t sound_model_handle,
                          const char *kv_pairs);

int pa_qst_get_param_data(const pa_qst_handle_t *mod_handle,
                          pa_qst_ses_handle_t sound_model_handle,
                          const char *param,
                          void *payload,
                          size_t payload_size,
                          size_t *param_data_size);

size_t pa_qst_get_buffer_size(const pa_qst_handle_t *mod_handle,
                              pa_qst_ses_handle_t sound_model_handle);

int pa_qst_read_buffer(const pa_qst_handle_t *mod_handle,
                       pa_qst_ses_handle_t sound_model_handle,
                       unsigned char *buf,
                       size_t bytes);

int pa_qst_stop_buffering(const pa_qst_handle_t *mod_handle,
                          pa_qst_ses_handle_t sound_model_handle);

int pa_qst_get_properties(const pa_qst_handle_t *mod_handle,
                          struct sound_trigger_properties *properties);

int pa_qst_get_version(const pa_qst_handle_t *mod_handle);

pa_qst_handle_t *pa_qst_init(const char *module_name);

int pa_qst_deinit(const pa_qst_handle_t *mod_handle);

#ifdef __cplusplus
}
#endif

#endif  //PA_QTI_SOUNDTRIGGER_H
