/* opaque_header.h
 *
 * This file contains the structures needed for the updated
 * recognition config and recognition event opaque data. These
 * structures will also be used by the client applications for
 * SVA.
 *
 * Copyright (c) 2018-2019, The Linux Foundation. All rights reserved.
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

#ifndef ST_OPAQUE_H
#define ST_OPAQUE_H

#include "st_second_stage.h"

#define ST_MAX_SOUND_MODELS 10
#define ST_MAX_KEYWORDS 10
#define ST_MAX_USERS 10

typedef enum st_param_key st_param_key_t;

enum st_param_key {
    ST_PARAM_KEY_CONFIDENCE_LEVELS,
    ST_PARAM_KEY_HISTORY_BUFFER_CONFIG,
    ST_PARAM_KEY_KEYWORD_INDICES,
    ST_PARAM_KEY_TIMESTAMP,
    ST_PARAM_KEY_DETECTION_PERF_MODE,
};

struct st_param_header
{
    st_param_key_t key_id;
    uint32_t payload_size;
}__packed;

struct st_user_levels
{
    uint32_t user_id;
    uint8_t level;
}__packed;

struct st_keyword_levels
{
    uint8_t kw_level;
    uint32_t num_user_levels;
    struct st_user_levels user_levels[ST_MAX_USERS];
}__packed;

struct st_sound_model_conf_levels
{
    listen_model_indicator_enum sm_id;
    uint32_t num_kw_levels;
    struct st_keyword_levels kw_levels[ST_MAX_KEYWORDS];
}__packed;

struct st_confidence_levels_info
{
    uint32_t version; /* value: 0x1 */
    uint32_t num_sound_models;
    struct st_sound_model_conf_levels conf_levels[ST_MAX_SOUND_MODELS];
}__packed;

struct st_user_levels_v2
{
    uint32_t user_id;
    int32_t level;
}__packed;

struct st_keyword_levels_v2
{
    int32_t kw_level;
    uint32_t num_user_levels;
    struct st_user_levels_v2 user_levels[ST_MAX_USERS];
}__packed;

struct st_sound_model_conf_levels_v2
{
    listen_model_indicator_enum sm_id;
    uint32_t num_kw_levels;
    struct st_keyword_levels_v2 kw_levels[ST_MAX_KEYWORDS];
}__packed;

struct st_confidence_levels_info_v2
{
    uint32_t version; /* value: 0x02 */
    uint32_t num_sound_models;
    struct st_sound_model_conf_levels_v2 conf_levels[ST_MAX_SOUND_MODELS];
}__packed;

struct st_hist_buffer_info
{
    uint32_t version; /* value: 0x02 */
    uint32_t hist_buffer_duration_msec;
    uint32_t pre_roll_duration_msec;
}__packed;

struct st_keyword_indices_info
{
    uint32_t version; /* value: 0x01 */
    uint32_t start_index; /* in bytes */
    uint32_t end_index;   /* in bytes */
}__packed;

struct st_timestamp_info
{
    uint32_t version; /* value: 0x01 */
    uint64_t first_stage_det_event_time;  /* in nanoseconds */
    uint64_t second_stage_det_event_time; /* in nanoseconds */
}__packed;

struct st_det_perf_mode_info
{
    uint32_t version; /* value: 0x01 */
    uint8_t mode; /* 0 -Low Power, 1 -High performance */
}__packed;
#endif /* ST_OPAQUE_H */
