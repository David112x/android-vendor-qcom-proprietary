/*============================================================================
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ============================================================================*/

#ifndef _OAR_MODULE_H
#define _OAR_MODULE_H

#include <stdint.h>
#include "qap_defs.h"

#if __cplusplus
extern "C"
{
#endif

/* DLB Data type */
/* GST_AUDIO_FORMAT_S16LE */
#define QAP_OAR_DLB_BUFFER_SHORT_16     4
/* GST_AUDIO_FORMAT_S32LE */
#define QAP_OAR_DLB_BUFFER_LONG_32      6
/* GST_AUDIO_FORMAT_F32LE */
#define QAP_OAR_DLB_BUFFER_FLOAT        7
/* default value for oamd pass */
#define QAP_OAR_PASS_OAMD_DEFAULT       (0)
/* max oar channels */
#define QAP_OAR_MAX_CHANNELS            (35)

/* Control parameter structure */
typedef enum {
    /* If plugin is run for Dolby CIDK certification,
     * channel reordering of buffer is not done at QAP & GST.
     */
    QAP_OAR_USECASE_CIDK = 0,
    /* If plugin is run for Dolby IIDK certification,
     * channel reordering done at QAP only
     */
    QAP_OAR_USECASE_IIDK = 1,
    /* If plugin is run in pipeline (gst-launch),
     * channel reordering done at QAP and GST
     */
    QAP_OAR_USECASE_GST = 2
} qap_oar_usecase_t;

/**
 * oar configuration parameters
 */
typedef struct oar_config_params {
    /* run usecase for IIDK, CIDK or Gstreamer */
    qap_oar_usecase_t usecase;
    /* speakers required/present*/
    unsigned long   speaker_bitfield;
    /* maximum processing block size allowed */
    unsigned        max_block_size;
    /* Maximum number of objects */
    unsigned        max_objects;
    /* maximum number of OAMD payloads per block of audio passed to OAR */
    unsigned        max_num_oamd_payloads;
    /* Minimum period elapse between the application of two metadata updates */
    unsigned        min_period_md_update;
    /* sample rate of incoming audio */
    unsigned long   sample_rate;
    /* cutoff frequency in Hz for bass extraction */
    int             bass_extraction_mode;
    /* Disable limiting with 0 enable limiting with 1 */
    int             b_limiter_enable;
    /* number of objects that will be processed with size */
    unsigned        max_objects_with_size;
    /* The internal processing block size of OAR */
    unsigned        processing_block_size;
    /* add bed oamds for channel base audio and render the objects */
    bool oamd_encode_enable;
    /* upmix enable flag */
    bool upmix_enable;
    /* output dolby buffer type */
    uint32_t  outbuffer_type;
    /* number of elements to process*/
    uint32_t  nb_elements;
} qap_oar_config_t;

/**
 * OAR commands
 */
typedef enum{
    /* get the process type */
	QAP_OAR_SESSION_PARAM_GET_PROCESSING_TYPE,
	QAP_OAR_MODULE_PARAM_SET_BASS_EXTRACTION_MODE,
	QAP_OAR_MODULE_PARAM_SET_PROCESSING_BLOCK_SIZE,
	QAP_OAR_MODULE_PARAM_GET_OUT_CHANNEL_INFO
} qap_oar_param_t;

/* Payload for qap_module_cmd get/set param :
 * @data - to be used to send param specific data to oar module
 */
typedef struct {
    qap_oar_param_t id;
    void *data;
} qap_oar_cmd_payload_t;

/**
 * oar pcm audio details
 */
typedef struct oar_audio_info {
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
    /* no of samples */
    uint32_t samples;
    /* width of pcm */
    uint32_t width;
    /* depth of pcm */
    uint32_t depth;
    /* interleaved or non interleaved */
    uint32_t layout;
    /* sample rate */
    uint64_t sample_rate;
    /* channel mask */
    uint64_t channel_mask;
} oar_audio_info_t;

/* Data structure to send OAMD from QAP OAR */
typedef struct dolby_oar_meta {
    /* TRUE if object audio/ FALSE for channel based */
    bool object_audio;
    /* Stream type */
    int stream_type;
    /* metadata */
    void *meta;
} dolby_oar_meta_t;

typedef struct {
    /* speakers required/present*/
    unsigned long speaker_bitfield;
    /* number of channels */
    uint32_t channels;
    /* output channel map */
    uint8_t ch_map[QAP_AUDIO_MAX_CHANNELS];
} qap_oar_out_chn_info_t;

#if __cplusplus
} /* extern "C" */
#endif

#endif /* _OAR_MODULE_H */
