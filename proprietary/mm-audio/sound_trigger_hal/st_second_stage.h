/*
 * This file contains a sound trigger second stage session abstraction. This
 * abstraction represents a single st second stage session from the st session
 * perspective.
 *
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#ifndef ST_SECOND_STAGE_H
#define ST_SECOND_STAGE_H

#include <stdint.h>
#include <pthread.h>
#include <cutils/list.h>
#ifdef ST_MULTI_STAGE_ENABLED
#include <capi_v2.h>
#endif
#include "st_common_defs.h"
#include "sml_model_parser.h"
#include "st_buffering.h"

#define CNN_FRAME_SIZE (320)

typedef enum st_sound_model_type {
    ST_SM_TYPE_NONE,
    ST_SM_TYPE_KEYWORD_DETECTION,
    ST_SM_TYPE_USER_VERIFICATION,
    ST_SM_TYPE_CUSTOM_DETECTION,
    ST_SM_TYPE_MAX
}st_sound_model_type_t;

typedef enum st_ss_detection_status {
    KEYWORD_DETECTION_PENDING = 0x1,
    KEYWORD_DETECTION_SUCCESS = 0x2,
    KEYWORD_DETECTION_REJECT = 0x4,
    USER_VERIFICATION_PENDING = 0x8,
    USER_VERIFICATION_SUCCESS = 0x10,
    USER_VERIFICATION_REJECT = 0x20,
}st_ss_detection_status_t;

typedef struct st_second_stage_info {
    enum st_sound_model_type sm_detection_type;
    listen_model_indicator_enum sm_id;
    char lib_name[MAX_LIB_HANDLE_SIZE];
    unsigned int sample_rate;
    unsigned int bit_width;
    unsigned int channel_count;
    unsigned int data_after_kw_end;
}st_second_stage_info_t;

typedef struct st_lsm_second_stage_config {
    struct st_second_stage_info *ss_info;
    struct listnode list_node;
    struct st_lsm_ss_params *params;
    /* sound model data */
    uint8_t *sm_data;
    unsigned int sm_size;
    /* confidence levels */
    int32_t confidence_threshold;
}st_lsm_ss_config_t;

typedef struct st_arm_second_stage {
    struct st_second_stage_info *ss_info;
    struct listnode list_node;
    struct st_arm_ss_session *ss_session;
    FILE *dump_fp;
}st_arm_second_stage_t;

typedef struct st_arm_ss_session {
    /* For controlling the second stage session */
    pthread_t thread;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    bool exit_thread;
    struct st_session *st_ses;

    /* For CNN to overwrite 1st stage indices */
    uint32_t kw_start_idx;
    uint32_t kw_end_idx;

    /* For passing the buffered pcm data to modules */
    uint8_t *hw_rd_ptr;
    unsigned int unread_bytes;
    bool exit_buffering;
    unsigned int buff_sz;
    unsigned int bytes_processed;
    unsigned int buf_start;
    unsigned int buf_end;

    /* For handling SUCCESS/REJECT results from modules */
    enum st_ss_detection_status det_status;

    /* For communicating with Capi modules */
#ifdef ST_MULTI_STAGE_ENABLED
    capi_v2_init_f  capi_init;
    void *capi_lib_handle;
    capi_v2_t *capi_handle;
#endif

    /* For registering the sound model with modules */
    uint8_t *sound_model;
    unsigned int sm_size;

    /* For getting and setting confidence levels */
    int32_t confidence_threshold;
    int32_t confidence_score;

    /* For controlling the second stage process threads */
    bool start_processing;
}st_arm_ss_session_t;

#ifdef ST_MULTI_STAGE_ENABLED
int st_second_stage_module_init(st_arm_second_stage_t *st_sec_stage,
    void *lib_name);
int st_second_stage_module_deinit(st_arm_second_stage_t *st_sec_stage);
int st_second_stage_prepare_session(st_arm_second_stage_t *st_sec_stage);
int st_second_stage_start_session(st_arm_second_stage_t *st_sec_stage);
int st_second_stage_stop_session(st_arm_second_stage_t *st_sec_stage);
#else
#define st_second_stage_module_init(a, b) (0)
#define st_second_stage_module_deinit(a) (0)
#define st_second_stage_prepare_session(a) (0)
#define st_second_stage_start_session(a) (0)
#define st_second_stage_stop_session(a) (0)
#endif

#endif /* ST_SECOND_STAGE_H */
