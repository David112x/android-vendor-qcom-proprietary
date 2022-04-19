/*============================================================================
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ============================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "qap_api.h"
#include "qap_defs.h"
#include "qap_module.h"

#define QAP_DMX_MAX_CHANNELS    256
#define QAP_DMX_CHN_CFG_MAX     32

typedef enum {
    QAP_DMX_CHN_CFG_20,
    QAP_DMX_CHN_CFG_21,
    QAP_DMX_CHN_CFG_212,
    QAP_DMX_CHN_CFG_202,
    QAP_DMX_CHN_CFG_30,
    QAP_DMX_CHN_CFG_31,
    QAP_DMX_CHN_CFG_312,
    QAP_DMX_CHN_CFG_302,
    QAP_DMX_CHN_CFG_40,
    QAP_DMX_CHN_CFG_41,
    QAP_DMX_CHN_CFG_412,
    QAP_DMX_CHN_CFG_402,
    QAP_DMX_CHN_CFG_414,
    QAP_DMX_CHN_CFG_404,
    QAP_DMX_CHN_CFG_50,
    QAP_DMX_CHN_CFG_51,
    QAP_DMX_CHN_CFG_512,
    QAP_DMX_CHN_CFG_502,
    QAP_DMX_CHN_CFG_514,
    QAP_DMX_CHN_CFG_504,
    QAP_DMX_CHN_CFG_60,
    QAP_DMX_CHN_CFG_61,
    QAP_DMX_CHN_CFG_612,
    QAP_DMX_CHN_CFG_602,
    QAP_DMX_CHN_CFG_614,
    QAP_DMX_CHN_CFG_604,
    QAP_DMX_CHN_CFG_70,
    QAP_DMX_CHN_CFG_71,
    QAP_DMX_CHN_CFG_712,
    QAP_DMX_CHN_CFG_702,
    QAP_DMX_CHN_CFG_714,
    QAP_DMX_CHN_CFG_704
} qap_dmx_chn_cfg_t;

typedef struct {
    bool mat_pcm_mode;
    qap_dmx_chn_cfg_t speaker_opt;
} qap_dmx_config_t;

typedef struct {
    unsigned int in_chn_num;
    unsigned int out_chn_num;
    unsigned long in_sample_rate;
    bool is_interleaved;
    unsigned int speakers;
    qap_dmx_config_t dmx_config;
    qap_audio_format_t format;
    qap_pcm_chmap in_chn_map[QAP_DMX_MAX_CHANNELS];
    qap_pcm_chmap out_chn_map[QAP_DMX_MAX_CHANNELS];
} qap_dmx_audio_info_t;

/* Module params for DAP */
typedef enum {
    QAP_DMX_PARAM_CHAN_INFO_GET,
    /* Get process type */
    QAP_DMX_SESSION_PARAM_GET_PROCESSING_TYPE
} qap_dmx_param_t;

/* Structure to send parameter values from gstreamer to qap */
typedef struct  {
    /* Downmixer param emums */
    qap_dmx_param_t id;
    void *data;
} qap_dmx_cmd_payload_t;
