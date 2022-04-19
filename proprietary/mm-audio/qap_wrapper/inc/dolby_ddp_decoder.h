/*============================================================================
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ============================================================================*/

#ifndef DOLBY_DDP_DECODER_H
#define DOLBY_DDP_DECODER_H

#include <stdint.h>
#include <stdbool.h>

#include "qap_api.h"
#include "qap_defs.h"
#include "qap_module.h"

#if __cplusplus
extern "C"
{
#endif

/* Macros */
#define QAP_DDP_UDC_PCMOUT_COUNT (8)
#define QAP_DDP_JOCD_MAX_CHANNELS (16)

/* Maxmimum number of objects output by the JOC      */
#define QAP_DDP_MAX_OBJECTS (16)
#define QAP_DDP_MAX_SAMPLES (2048)
/* used for setting default frame region */
#define QAP_DDP_END_OF_STREAM       (-1)
/* default output mode */
#define QAP_DDP_DEFAULTOUTMODE      (21)
/* default scale factor */
#define QAP_DDP_DEFAULTSCALEFACTOR  (100)
#define QAP_DDP_MINSCALEFACTOR      (0)
/* minimum scale factor */
#define QAP_DDP_MAXSCALEFACTOR      (100)
/* maximum scale factor */
#define QAP_DDP_DEFAULTSUBSTREAMID  (0)
/* Used for routing array initialization */
#define QAP_DDP_CHAN_NOT_SET        (-1)
#define QAP_DDP_OUT_MODE_RAW        (-1)
/* Data types - equivalent to dolby datatypes (subjected to change with change in dolby enum values) */
#define QAP_DDP_MAX_OUT_MODE        (28)
#define QAP_DDP_MAX_CHAN_CFG        (QAP_DDP_MAX_OUT_MODE + 1)
#define QAP_DDP_DLB_BUFFER_SHORT_16 (4)
#define QAP_DDP_DLB_BUFFER_LONG_32  (6)
#define QAP_DDP_DLB_BUFFER_FLOAT    (7)
#define QAP_DDP_DLB_BUFFER_INT_LEFT (5)
#define UNUSED(x) do{ (void)x; } while(0)

/* Enums used for decoder configuration */
typedef enum {
    /* Decoding only mode */
    QAP_DDPI_UDC_DECODE_ONLY = 0,
    /* Converting only mode */
    QAP_DDPI_UDC_CONVERT_ONLY = 2,
    /* JOC decoding mode, PCM output */
    QAP_DDPI_UDC_JOC_DECODE_PCM_OUT = 3,
} QAP_DDPI_UDC_RUNNING_MODE;

typedef enum {
    /* Custom mode (no digital dialogue normalization)*/
    QAP_DDPI_UDC_COMP_CUSTOM_0,
    /* 2/0 Dolby Surround mode */
    QAP_DDPI_UDC_COMP_CUSTOM_1,
    /* Line out mode */
    QAP_DDPI_UDC_COMP_LINE,
    /* RF mode */
    QAP_DDPI_UDC_COMP_RF,
    /* Portable mode -8dB */
    QAP_DDPI_UDC_COMP_PORTABLE_L8,
    /* Portable mode -11dB */
    QAP_DDPI_UDC_COMP_PORTABLE_L11,
    /* Portable mode -14dB */
    QAP_DDPI_UDC_COMP_PORTABLE_L14,
} QAP_DDPI_UDC_COMP;

typedef enum {
    /* LFE channel is not included in the output PCM data */
    QAP_DDPI_UDC_LFEOUTOFF,
    /* LFE channel is included in the output PCM data if it exists in the bitstream */
    QAP_DDPI_UDC_LFEOUTSINGLE
} QAP_DDPI_UDC_LFEOUTMODES;

typedef enum {
    /* Automatically detects stereo mode based on setting in the bit stream. */
    QAP_DDPI_UDC_STEREOMODE_AUTO,
    /* 2/0 Dolby Surround compatible (Lt/Rt) */
    QAP_DDPI_UDC_STEREOMODE_SRND,
    /* 2/0 Stereo (Lo/Ro) */
    QAP_DDPI_UDC_STEREOMODE_STEREO
} QAP_DDPI_UDC_STEREOMODE;

typedef enum {
    /* Stereo */
    QAP_DDPI_UDC_DUAL_STEREO,
    /* Left mono */
    QAP_DDPI_UDC_DUAL_LEFTMONO,
    /* Right mono */
    QAP_DDPI_UDC_DUAL_RGHTMONO,
    /* Mixed mono */
    QAP_DDPI_UDC_DUAL_MIXMONO
} QAP_DDPI_UDC_DUALMONO_DOWNMIX;

typedef enum {
    /* Single input mode */
    QAP_DDPI_UDC_INPUTMODE_SINGLEINPUT,
    /* Dual input mode (Only valid in dual decoding mode) */
    QAP_DDPI_UDC_INPUTMODE_DUALINPUT,
    QAP_DDPI_UDC_INPUTMODE_COUNT
} QAP_DDPI_UDC_INPUTMODE;

typedef enum {
    /* If plugin is run for Dolby CIDK certification,
     * channel reordering of buffer is not done at QAP & GST.
     */
    QAP_DDP_USECASE_CIDK = 0,
    /* If plugin is run for Dolby IIDK certification,
     * channel reordering done at QAP & GST
     */
    QAP_DDP_USECASE_IIDK = 1,
} QAP_DDP_USECASE;

typedef enum {
    /* No decorrelation */
    QAP_DDPI_UDC_DECORR_OFF,
    /* Unguided decorrelation */
    QAP_DDPI_UDC_DECORR_UNGUIDED,
    /* Adaptive decorrelation (Only valid in decoding only mode or decoding and converting mode;
     deactivate the decorrelation for JOC content if the output mode matches the input audio coding mode(acmod),
     otherwise, the decoder will activate unguided decorrelation automatically.)*/
    QAP_DDPI_UDC_DECORR_ADAPTIVE
} QAP_DDPI_UDC_DECORR_MODE;

typedef enum {
    /* Disable Hash protection in outbound Evolution data */
    QAP_DDPI_UDC_EVO_DISABLE_PCMHASH = 0,
    /* Enable Hash protection in outbound Evolution data (default) */
    QAP_DDPI_UDC_EVO_ENABLE_PCMHASH = 1
} QAP_DDPI_UDC_EVOLUTION_MODE;

/* Select dep / indep substream to evo framework quick access */
typedef enum {
    QAP_DDPI_UDC_SUBSTREAM_TYPE_UNDEFINE = -1,
    QAP_DDPI_UDC_SUBSTREAM_TYPE_INDEP = 0,
    QAP_DDPI_UDC_SUBSTREAM_TYPE_DEP = 1
} QAP_DDPI_UDC_SUBSTREAM_TYPE;

typedef enum {
    /* Disable associate decoding. */
    QAP_DDPI_UDC_SUBSTREAMID_UNDEFINED = -1,
    /* Select substream 0 for associate decoding
     * (default value for dual input mode, equal to DDPI_UDC_SUBSTREAMID_UNDEFINED in single input mode). */
    QAP_DDPI_UDC_SUBSTREAMID_0 = 0,
    /* Select substream 1 for associate decoding
     * (default value for single input mode, equal to DDPI_UDC_SUBSTREAMID_0 in dual input mode). */
    QAP_DDPI_UDC_SUBSTREAMID_1 = 1,
    /* Select substream 2 for associate decoding(equal to DDPI_UDC_SUBSTREAMID_0 in dual input mode). */
    QAP_DDPI_UDC_SUBSTREAMID_2 = 2,
    /* Select substream 3 for associate decoding(equal to DDPI_UDC_SUBSTREAMID_0 in dual input mode). */
    QAP_DDPI_UDC_SUBSTREAMID_3 = 3
} QAP_DDPI_UDC_SUBSTREAMID;

typedef enum {
    QAP_DDPI_UDC_PCMCONCEAL_ALWAYSRPT = -1,
    QAP_DDPI_UDC_PCMCONCEAL_ALWAYSMUTE
} QAP_DDPI_UDC_PCMCONCEAL;

/* Following enums supported for run time set-get params in QAP module */

/* Module get params for DDP */
typedef enum {
    /* Module output buffer size */
    QAP_DDP_MODULE_PARAM_GET_OUT_BUF_SIZE,
    /* DRC mode - refer DDP_UDC_COMP */
    QAP_DDP_MODULE_PARAM_GET_DRC_MODE,
    /* Output channel configuration */
    QAP_DDP_MODULE_PARAM_GET_OUT_CH_CFG,
    /* LFE channel to enable */
    QAP_DDP_MODULE_PARAM_GET_LFE_ON,
    /* Select substream for associate decoding */
    QAP_DDP_MODULE_PARAM_GET_ASSO_SUBSTREAM_SEL,
    /* Enable mixer */
    QAP_DDP_MODULE_PARAM_GET_ENABLE_MIXER,
    /* Force downmix output in JOC mode */
    QAP_DDP_MODULE_PARAM_GET_JOC_FORCE_DWNMIX,
    QAP_DDP_MODULE_PARAM_GET_MAX
} qap_ddp_module_get_param_t;

/* Module set params for DDP */
typedef enum {
    /* DRC mode - refer DDP_UDC_COMP */
    QAP_DDP_MODULE_PARAM_SET_DRC_MODE,
    /* Output channel configuration */
    QAP_DDP_MODULE_PARAM_SET_OUT_CH_CFG,
    /* LFE channel to enable */
    QAP_DDP_MODULE_PARAM_SET_LFE_ON,
    /* Select substream for associate decoding */
    QAP_DDP_MODULE_PARAM_SET_ASSO_SUBSTREAM_SEL,
    /* Enable mixer */
    QAP_DDP_MODULE_PARAM_SET_ENABLE_MIXER,
    /* Force downmix output in JOC mode */
    QAP_DDP_MODULE_PARAM_SET_JOC_FORCE_DWNMIX,
    QAP_DDP_MODULE_PARAM_SET_MAX
} qap_ddp_module_set_param_t;

/* Session get params for DDP */
typedef enum {
    /* Synchronous / asynchronous processing */
    QAP_DDP_SESSION_PARAM_GET_PROCESSING_TYPE,
    QAP_DDP_SESSION_GET_PARAM_MAX
} qap_ddp_session_get_param_t;

/* Session set params for DDP */
typedef enum {
    QAP_DDP_SESSION_SET_PARAM_MAX
} qap_ddp_session_set_param_t;

/* Payload for qap_module_cmd get/set param : @data - to be used to send param specific data to ddp module */
typedef struct {
    union {
    qap_ddp_module_get_param_t get_id;
    qap_ddp_module_set_param_t set_id;
    };
    void *data;
} qap_ddp_moudle_cmd_payload_t;

/* Payload for qap_session_cmd get/set param : @data - to be used to send param specific data to ddp module */
typedef struct {
    union {
        qap_ddp_session_get_param_t get_id;
        qap_ddp_session_set_param_t set_id;
    };
    void *data;
} qap_ddp_session_cmd_payload_t;

/* Data stucture to send OAMD from QAP DDP */
typedef struct {
    /* TRUE if object audio/ FALSE for channel based */
    bool object_audio;
    /* metadata */
    void *meta;
} qap_ddp_meta_t;

/* Control parameter structure */
typedef struct {
    /* running mode */
    int32_t running_mode;
    /* dynamic range compression mode */
    int32_t comp_mode;
    /* output LFE channel present */
    int32_t out_lfe;
    /* output channel configuration */
    int32_t out_chan_config;
    /* PCM scale factor */
    int32_t pcm_scale;
    /* dynamic range compression boost scale factor */
    int32_t dyn_scale_high;
    /* dynamic range compression cut scale factor */
    int32_t dyn_scale_low;
    /* stereo output mode */
    int32_t stereo_mode;
    /* dual mono reproduction mode */
    int32_t dual_mode;
    /* single or dual input */
    int32_t input_mode;
    /* substream ID selector */
    int32_t assoc_id_select;
    /* MDCT bandlimiting mode */
    int32_t mdct_band_limit;
    /* Subroutine PCM output error concealment flag */
    int32_t dec_error_conceal_flag;
    /* Subroutine PCM output error concealment method */
    int32_t dec_error_conceal_type;
    /* Subroutine DD output error concealment flag */
    int32_t cnv_error_conceal_flag;
    int32_t drc_suppress_mode;
    /* converting time slice region start */
    int32_t convert_timeslice_start;
    /* converting time slice region end */
    int32_t convert_timeslice_end;
    /* decoding time slice region start */
    int32_t decode_timeslice_start;
    /* decoding time slice region end */
    int32_t decode_timeslice_end;
    /* Frame debug flags */
    uint32_t frm_debug_flags;
    /* Decode debug flags */
    uint32_t dec_debug_flags;
    /* Decorrelator operating mode */
    int32_t decorr_mode;
    /* Evolution decoder operating mode */
    int32_t evo_hash_flag;
    /*  evolution quick access flag */
    int32_t is_evolution_quickaccess;
    /*  quick access evolution substream ID */
    int32_t evo_quickaccess_substream_id;
    /*  quick access evolution substream type */
    int32_t evo_quickaccess_strm_type;
    /*  User mixing balance between main and associated programs. Range is -32 to 32. */
    int32_t mixer_user_pref;
    /*  mixer mode flag */
    int32_t mixer_mode;
    /*  force joc output downmix content */
    int32_t joc_force_downmix;
} ddp_output_params_t;

/**
 * ddp_config_params_t is decoder context configured at init time by the application. This context will be conifgured in the Gstreamer
 * plugin through set_property as void type param from the application. Gstreamer will pass this context to QAP during qap_module_init
 * call for creation and configuration of DDP decoder instance. Later qap_ddp_module_get_param_t will be used in qap_module_cmd to control the
 * run-time properties of DDP plugin.
 */

/* Decoder context */
typedef struct {
    /* number of DD+ inputs (1 or 2) */
    int32_t num_of_ddpin;
    /* number of DD outputs (1 - DDP to DD conversion or 0 - No conversion(default))  */
    int32_t num_of_ddout;
    /* number of PCM outputs */
    int32_t num_of_pcmout;
    /* num. of main program PCM outputs */
    int32_t num_of_main_pcmout;
    /* number of output channels  2/6/8 */
    int32_t out_chans[QAP_DDP_UDC_PCMOUT_COUNT];
    /* Output data interleaved or non-interleaved flag */
    int32_t is_interleaved_flag;
    /* The data type of the dlb_buffer for the PCM output */
    int32_t data_type;
    /* Disable the decorrelator for associate audio decoding */
    int32_t disable_associated_decorr;
    /* Disable the evolution for associate audio decoding */
    int32_t disable_associated_evolution;
    /* user-specified subroutine parameters */
    ddp_output_params_t output_params[QAP_DDP_UDC_PCMOUT_COUNT];
    /* metadata output flags - Future use */
    uint32_t mdat_flags;
    /* evolution metadata output flags  - Future use */
    uint8_t evo_out_to_file_flag;
    /* evolution metadata output flags  - Future use */
    uint8_t evo_quickaccess_out_to_file_flag;
    /* quit on err */
    uint8_t quit_on_err;
    /* Used for JOC mode, the number of channels for one single file - Future use */
    int32_t joc_num_out_channels[QAP_DDP_JOCD_MAX_CHANNELS];
    /* Flag to indicate JOC is enabled */
    bool joc_enable;
    /* Flag to indicate if direct decode was requested */
    bool direct_decode_enable;
    /* Enum to indicate IIDK / CIDK / launch */
    int32_t usage;
} ddp_config_params_t;

#if __cplusplus
} // extern "C"
#endif

#endif //DOLBY_DDP_DECODER_H
