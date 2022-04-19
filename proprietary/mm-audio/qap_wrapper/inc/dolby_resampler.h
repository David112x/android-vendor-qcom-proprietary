/*============================================================================
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ============================================================================*/

#ifndef DAT_RESAMPLE_H
#define DAT_RESAMPLE_H

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
/**
 * Resample module commands
 */
typedef enum qap_resample_moudle_cmd_payload {
    QAP_RESAMPLE_SESSION_PARAM_GET_PROCESSING_TYPE,
} qap_resample_moudle_cmd_payload_t;

/* Payload for qap_module_cmd get/set param :
 * @data - to be used to send param specific data to resample module
 */
typedef struct {
    qap_resample_moudle_cmd_payload_t id;
    void *data;
} qap_resample_session_cmd_payload_t;

typedef enum {
    QAP_DAT_RESAMPLE_RET_INVALID = -1,
    QAP_DAT_RESAMPLE_RET_FAIL = 0,
    QAP_DAT_RESAMPLE_RET_SUCCESS = 1,
} qap_resampler_status_t;

typedef enum {
    QAP_DAT_RESAMPLE_DATA_S16LE = 0,
    QAP_DAT_RESAMPLE_DATA_S32LE,
    QAP_DAT_RESAMPLE_DATA_F32LE,
} qap_resampler_format_t;

typedef enum {
    QAP_DAT_RESAMPLE_UNDEFINED = 0,
    QAP_DAT_RESAMPLE_1X = 1,
    QAP_DAT_RESAMPLE_2X = 2,
    QAP_DAT_RESAMPLE_4X = 4,
} qap_resampler_scale_factor_t;

typedef enum {
    QAP_DAT_RESAMPLER_MODE_UPSAMPLER,
    QAP_DAT_RESAMPLER_MODE_DOWNSAMPLER,
    QAP_DAT_RESAMPLER_MODE_UNDEFINED,
} qap_resampler_mode_t;

typedef struct {
    unsigned int in_sample_rate;
    unsigned int out_sample_rate;
    unsigned int chn_num;
    qap_audio_format_t format;bool is_interleaved;
    qap_resampler_mode_t mode;
    qap_resampler_scale_factor_t scalef;
} resample_config_params_t;
/**
 * resampler pcm audio details
 */
typedef struct resample_audio_info {
    /* bit format for the pcm, 16, 32 etc */
    uint32_t format;
    /* number of channels */
    uint32_t channels;
    /* bits fer frame */
    uint32_t bits_per_frame;
    /* bytes per frame */
    uint32_t bytes_per_sample;
    /* little endian or big endian */
    uint32_t endianness;
    /* width of pcm */
    uint32_t width;
    /* depth of pcm */
    uint32_t depth;
    /* interleaved or non interleaved */
    uint32_t layout;
    /* sample rate */
    uint64_t sample_rate;
} resample_audio_info_t;

#endif

