/*============================================================================
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ============================================================================*/

#ifndef DOLBY_MAT_DECODER_H
#define DOLBY_MAT_DECODER_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "qap_api.h"
#include "qap_defs.h"
#include "qap_module.h"

#if __cplusplus
extern "C"
{
#endif

#define QAP_MAT_DLB_BUFFER_SHORT_16 (4)
#define QAP_MAT_DLB_BUFFER_LONG_32 (6)
#define QAP_MAT_DLB_BUFFER_FLOAT (7)
#define QAP_MAT_DLB_BUFFER_INT_LEFT (5)
#define QAP_MAT_DLB_BUFFER_OCTET_UNPACKED (1)
#define QAP_MAT_MAX_FILENAME_LENGTH (2048)
#define QAP_MAT_MAX_PCM_CHANNEL_COUNT (32)
#define QAP_MATDEC_MAX_PCM_SAMPLES (1920)

/* This enumerated list defines the types of content available for MAT. */
typedef enum {
    /* Dolby TrueHD */
    QAP_MAT_CONTENT_TYPE_DTHD = 0,
    /* PCM + Metadata */
    QAP_MAT_CONTENT_TYPE_PCM = 1,
} qap_mat_content_type_enum_t;

/* Dolby MAT decoder DRC enable mode enumeration values and their meanings. */
typedef enum {
    /* DRC disabled */
    QAP_MAT_DEC_DRC_ENABLE_MODE_OFF = 0,
    /* Normal DRC mode enabled (-31dBFS ref level) */
    QAP_MAT_DEC_DRC_ENABLE_MODE_NORMAL,
    /* Heavy DRC mode enabled (-20dBFS ref level) */
    QAP_MAT_DEC_DRC_ENABLE_MODE_HEAVY,
    QAP_MAT_DEC_DRC_ENABLE_MODE_COUNT,
} qap_mat_dec_drc_enable_mode_enum_t;

/* Dolby MAT decoder DRC enable mode enumeration values and their meanings. */
typedef enum {
    /* DRC/Dialnorm disabled */
    QAP_MAT_DEC_LOUDNESS_MGMT_OFF = 0,
    /* DRC/Dialnorm enabled */
    QAP_MAT_DEC_LOUDNESS_MGMT_ON = 1,
    QAP_MAT_DEC_LOUDNESS_MGMT_COUNT,
} qap_mat_dec_loudness_mgmt_enum_t;

typedef enum {
    /* If plugin is run for Dolby CIDK certification,
     * channel reordering of buffer is not done at QAP & GST.
     */
    QAP_MAT_USECASE_CIDK = 0,
    /* If plugin is run for Dolby IIDK certification,
     * channel reordering done at QAP & GST
     */
    QAP_MAT_USECASE_IIDK = 1,
} QAP_DDP_USAGE;

/* Following enums supported for run time set-get params in QAP module */

/* Module get params for MAT */
typedef enum {
    /* Module output buffer size */
    QAP_MAT_MODULE_PARAM_GET_OUT_BUF_SIZE,
    /* DRC mode */
    QAP_MAT_MODULE_PARAM_GET_DRC_MODE,
    QAP_MAT_MODULE_PARAM_GET_DRC_CUT,
    QAP_MAT_MODULE_PARAM_GET_DRC_BOOST,
    QAP_MAT_MODULE_PARAM_GET_LOUD_MNGT_ENABLE,
    QAP_MAT_MODULE_PARAM_GET_MAX
} qap_mat_module_get_param_t;

/* Module set params for MAT */
typedef enum {
    /* DRC mode */
    QAP_MAT_MODULE_PARAM_SET_DRC_MODE,
    QAP_MAT_MODULE_PARAM_SET_DRC_CUT,
    QAP_MAT_MODULE_PARAM_SET_DRC_BOOST,
    QAP_MAT_MODULE_PARAM_SET_LOUD_MNGT_ENABLE,
    QAP__MODULE_PARAM_SET_MAX
} qap_mat_module_set_param_t;

/* Session get params for MAT */
typedef enum {
    /* Synchronous / asynchronous processing */
    QAP_MAT_SESSION_PARAM_GET_PROCESSING_TYPE,
    QAP_MAT_SESSION_GET_PARAM_MAX
} qap_mat_session_get_param_t;

/* Session set params for MAT */
typedef enum {
    QAP_MAT_SESSION_SET_PARAM_MAX
} qap_mat_session_set_param_t;

/* Payload for qap_module_cmd get/set param : @data - to be used to send param specific data to mat module */
typedef struct {
    union {
    qap_mat_module_get_param_t get_id;
    qap_mat_module_set_param_t set_id;
    };
    void *data;
} qap_mat_module_cmd_payload_t;

/* Payload for qap_session_cmd get/set param : @data - to be used to send param specific data to mat module */
typedef struct {
    union {
        qap_mat_session_get_param_t get_id;
        qap_mat_session_set_param_t set_id;
    };
    void *data;
} qap_mat_session_cmd_payload_t;

/* Data stucture to send OAMD from QAP MAT */
typedef struct {
    /* TRUE if object audio/ FALSE for channel based */
    bool object_audio;
    /* metadata */
    void *meta;
} qap_mat_meta_t;


typedef struct {
    /* Configured operating mode. */
    int operating_mode;
    /* Verbose mode. */
    int verbose_enable;
    /* DRC mode */
    qap_mat_dec_drc_enable_mode_enum_t drc_mode;
    /* DRC cut scale factor */
    int drc_cut;
    /* DRC boost scale factor */
    int drc_boost;
    /* Loudness management */
    qap_mat_dec_loudness_mgmt_enum_t loudness_mgmt_enable;
    /* Requested decoder output data type. */
    int data_type;
    /* Meta file name of channel meta dump is enabled */
    char meta_file_base_name[QAP_MAT_MAX_FILENAME_LENGTH];
    /* Enum to indicate IIDK / CIDK / launch */
    int32_t usage;
} qap_mat_dec_config_t;

#if __cplusplus
} // extern "C"
#endif

#endif //DOLBY_MAT_DECODER_H
