/* st_hw_defs.h
 *
 * Copyright (c) 2016-2019, The Linux Foundation. All rights reserved.
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

/* This file should be a copy of qsthw_defs.h under qsthw_api */

#include <system/sound_trigger.h>

#ifndef SOUND_TRIGGER_HW_DEFS_H
#define SOUND_TRIGGER_HW_DEFS_H

/*
 * Below are the parameter definitions that are used by ST HAL and
 * qti ST HAL wrapper client.
 */

/* sound trigger recognition event type */
typedef enum {
    QSTHW_RC_EVENT_TYPE_NON_TIMESTAMP = 0,
    QSTHW_RC_EVENT_TYPE_TIMESTAMP = 1
} qsthw_recognition_event_type_t;


/*
 * QTI enhanced recognition event with detection timestamp
 */
struct qsthw_phrase_recognition_event {
    struct sound_trigger_phrase_recognition_event phrase_event; /* key phrase recognition event */
    uint64_t                                      timestamp;       /* event time stamp */
};

struct qsthw_source_tracking_param
{
    int target_angle_L16[2];
    int interf_angle_L16[2];
    char polarActivityGUI[360];
};

struct qsthw_target_channel_index_param
{
    int target_chan_idx;
};

struct qsthw_esp_energy_levels_param {
    int voiceEnergy;
    int ambientNoiseEnergy;
};

typedef union {
    struct qsthw_source_tracking_param st_params;
    struct qsthw_target_channel_index_param ch_index_params;
    struct qsthw_esp_energy_levels_param esp_energy_levels_params;
} qsthw_get_param_payload_t;

/* sound trigger set parameter keys */
#define QSTHW_PARAMETER_CUSTOM_CHANNEL_MIXING "st_custom_channel_mixing"
#define QSTHW_PARAMETER_SESSION_PAUSE "st_session_pause"
#define QSTHW_PARAMETER_BAD_MIC_CHANNEL_INDEX "st_bad_mic_channel_index"
#define QSTHW_PARAMETER_DIRECTION_OF_ARRIVAL "st_direction_of_arrival"
#define QSTHW_PARAMETER_CHANNEL_INDEX "st_channel_index"
#define QSTHW_PARAMETER_EC_REF_DEVICE "st_ec_ref_device"
#define QSTHW_PARAMETER_ESP_ENERGY_LEVELS "st_esp_energy_levels"

#endif  //SOUND_TRIGGER_HW_DEFS_H
