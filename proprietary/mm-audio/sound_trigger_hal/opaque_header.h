/* opaque_header.h
 *
 * This file contains the structures needed for the updated
 * recognition config and recognition event opaque data. These
 * structures will also be used by the client applications for
 * SVA.
 *
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
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
