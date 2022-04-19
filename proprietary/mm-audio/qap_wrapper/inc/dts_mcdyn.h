/*============================================================================
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ============================================================================*/

#ifndef _DTS_MCDYN_H_
#define _DTS_MCDYN_H_

#include <stdbool.h>
#include <stdint.h>

/* Scaling factors for MCDYN parameter values. Needed by the client in order
 * to convert the float values received from command line or application to
 * integer values.
 */
#define QAP_SCALE_MCDYN_MBHL_OUTPUT_GAIN       32767
#define QAP_SCALE_MCDYN_MBHL_ANTICLIP_LVL      32767
#define QAP_SCALE_MCDYN_MBHL_COMP_RATIO        67108864
#define QAP_SCALE_MCDYN_MBHL_BOOST             524288
#define QAP_SCALE_MCDYN_MBHL_LOWCOMPTHRESH     2147483647
#define QAP_SCALE_MCDYN_MBHL_MIDCOMPTHRESH     2147483647
#define QAP_SCALE_MCDYN_MBHL_HIGHCOMPTHRESH    2147483647
#define QAP_SCALE_MCDYN_MBHL_COMPMAKEUPGAIN    8388608

/* Buffer details */
#define QAP_MCDYN_MAX_CHANNELS 8
#define QAP_MCDYN_FRAME_SIZE   256

/* To set mcdyn coefficient structure */
typedef struct {
    /* Filter coefficients */
    float  low_cross_freq;
    float  mid_cross_freq;
} qap_dts_mcdyn_filter_coeffs_t;

/* DTS MultiChannel Dynamics MBHL configuration structure.
 * Nothing but payload id's data to be filled for each enum value declared
 * below. All data types declared as per DTS code compatibility.
 */
typedef struct {
    int32_t    process_enable;
    int32_t    mcd_io_mode;
    /* MVol controls */
    int32_t    mv_enable;
    int32_t    mv_vol_step;
    int32_t    mv_mute;
    /* MCD controls */
    int32_t    hl_enable;
    int32_t    output_gain;
    int32_t    ac_level;
    int32_t    attack;
    int32_t    low_decay;
    int32_t    mid_decay;
    int32_t    high_decay;
    int32_t    ratio;
    int32_t    hl_mode;
    int32_t    boost;
    int32_t    low_cross;
    int32_t    mid_cross;
    int32_t    low_comp_thresh;
    int32_t    mid_comp_thresh;
    int32_t    high_comp_thresh;
    int32_t    comp_make_up_gain;
    /* Common controls */
    int32_t    sample_rate;
    /* Filter coefficients */
    qap_dts_mcdyn_filter_coeffs_t filters;
} qap_dts_mcdyn_api_ctrl_t;

/**
 * MCDYN module commands
 */
typedef enum {
    QAP_MCDYN_SESSION_PARAM_GET_PROCESSING_TYPE,
    /* MCD MBHL Parameters */
    /* Below parameters uses int32_t as payload data */
    QAP_MCD_PROCESS_EN,
    QAP_MCD_MBHL_IO_MODE,
    QAP_MCD_MBHL_MVOL_EN,
    QAP_MCD_MBHL_MVOL_STEP,
    QAP_MCD_MBHL_MVOL_MUTE,
    QAP_MCD_MBHL_EN,
    QAP_MCD_MBHL_OUTPUT_GAIN,
    QAP_MCD_MBHL_ANTICLIP_LVL,
    QAP_MCD_MBHL_COMP_ATTACKS,
    QAP_MCD_MBHL_LOWCOMP_DECAY,
    QAP_MCD_MBHL_MIDCOMP_DECAY,
    QAP_MCD_MBHL_HIGHCOMP_DECAY,
    QAP_MCD_MBHL_COMP_RATIO,
    QAP_MCD_MBHL_MODE,
    QAP_MCD_MBHL_BOOST,
    QAP_MCD_MBHL_LOWCROSS,
    QAP_MCD_MBHL_MIDCROSS,
    QAP_MCD_MBHL_LOWCOMPTHRESH,
    QAP_MCD_MBHL_MIDCOMPTHRESH,
    QAP_MCD_MBHL_HIGHCOMPTHRESH,
    QAP_MCD_MBHL_COMPMAKEUPGAIN,
    /* Filter coefficient values */
    /* Coef structure uses qap_dts_mcdyn_filter_coeffs_t as payload data */
    QAP_MCD_MBHL_COEF_STU,
    /* Not applicable for now. TO-DO. */
    QAP_MCD_MBHL_CONTROLS_STU,
    /* MCD MBHL Common Parameters */
    /* Uses int32_t as payload data */
    QAP_MCD_INPUT_SAMPLERATE_I32
} qap_mcdyn_module_cmd_payload_id_t;

/* Payload for qap_module_cmd get/set param :
 * @data - to be used to send param specific data to mcdyn module
 */
typedef struct {
    qap_mcdyn_module_cmd_payload_id_t id;
    /* data to be filled based on qap_dts_mcdyn_api_ctrl_t and
     * qap_mcdyn_module_cmd_payload_id_t.
     */
    void *data;
} qap_mcdyn_session_cmd_payload_t;

/**
 * mcdyn pcm audio details
 */
typedef struct {
    /* bit format for the pcm, 16, 24, 32 etc */
    uint32_t format;
    /* number of channels */
    uint32_t channels;
    /* bit depth */
    uint32_t bit_depth;
    /* sample rate */
    uint32_t sample_rate;
    /* Input/Output mode */
    uint32_t io_mode;
} qap_dts_mcdyn_audio_info_t;

/* MCD MBHL Audio PCM format */
typedef enum {
    QAP_MCDYN_PCM_FMT_32B_F32_NE,
    QAP_MCDYN_PCM_FMT_32B_I32_NE,
    QAP_MCDYN_PCM_FMT_24B_I32_NE,
    QAP_MCDYN_PCM_FMT_16B_I32_NE,
    QAP_MCDYN_PCM_FMT_16B_I16_NE
} qap_dts_mcdyn_PCM_format_t;

/* MCD MBHL Input Output mode */
typedef enum {
    QAP_MCDYN_IN_OUT_STEREO,
    QAP_MCDYN_IN_OUT_2_1,
    QAP_MCDYN_IN_OUT_3_0,
    QAP_MCDYN_IN_OUT_3_1,
    QAP_MCDYN_IN_OUT_5_1,
    QAP_MCDYN_IN_OUT_7_1
} qap_dts_mcdyn_IO_mode_t;

#endif /* _DTS_MCDYN_H_ */

