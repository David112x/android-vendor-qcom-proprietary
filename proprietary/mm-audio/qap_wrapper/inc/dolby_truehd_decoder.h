/*============================================================================
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ============================================================================*/

#ifndef DOLBY_TH_DECODER_H
#define DOLBY_TH_DECODER_H

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
#define QAP_TH_PCMOUT_COUNT        (4)
/* used for setting default frame region */
#define QAP_TH_END_OF_STREAM       (-1)
/* default output mode */
#define QAP_TH_DEFAULTOUTMODE      (7)
/* default scale factor */
#define QAP_TH_DEFAULTSCALEFACTOR  (100)
#define QAP_TH_MINSCALEFACTOR      (0)
/* minimum scale factor */
#define QAP_TH_MAXSCALEFACTOR      (100)
/* Used for routing array initialization */
#define QAP_TH_CHAN_NOT_SET        (-1)
#define QAP_TH_OUT_MODE_RAW        (0)
/* Data types - equivalent to dolby datatypes (subjected to change with change in dolby enum values) */
#define QAP_TH_MAX_OUT_MODE        (28)
#define QAP_TH_MAX_CHAN_CFG        (QAP_TH_MAX_OUT_MODE + 1)
#define QAP_TH_DLB_BUFFER_FLOAT    (7)
#define QAP_TH_DLB_BUFFER_INT_LEFT (5)
#define UNUSED(x) do{ (void)x; } while(0)

/* Enums used for decoder configuration */

/* Dolby TrueHD decoder DRC enable mode enumeration     */
typedef enum {
    /* DRC disabled */
    QAP_DTHDI_DEC_DRC_ENABLE_MODE_OFF = 0,
    /* Normal DRC mode follows bitstream control flag   */
    QAP_DTHDI_DEC_DRC_ENABLE_MODE_FOLLOW,
    /* Normal DRC mode enabled (-31dBFS ref level)      */
    QAP_DTHDI_DEC_DRC_ENABLE_MODE_NORMAL,
    /* Heavy DRC mode enabled (-20dBFS ref level)       */
    QAP_DTHDI_DEC_DRC_ENABLE_MODE_HEAVY,
    /* Portable DRC mode enabled (-8dBFS ref level)     */
    QAP_DTHDI_DEC_DRC_ENABLE_MODE_PORTABLE_8,
    /* Portable DRC mode enabled (-11dBFS ref level)    */
    QAP_DTHDI_DEC_DRC_ENABLE_MODE_PORTABLE_11,
    /* Portable DRC mode enabled (-14dBFS ref level)    */
    QAP_DTHDI_DEC_DRC_ENABLE_MODE_PORTABLE_14,
    QAP_DTHDI_DEC_DRC_ENABLE_MODE_COUNT,
} QAP_DTHDI_DEC_DRC_ENABLE_MODE;

/* Dolby TrueHD decoder presentation selection enumeration */
typedef enum
{
    /* Automatically select the correct presentation    */
    QAP_DTHDI_DEC_PRESENTATION_AUTO = -1,
    /* Select the 2 channel presentation from bitstream */
    QAP_DTHDI_DEC_PRESENTATION_2CH  =  0,
    /* Select the 6 channel presentation from bitstream */
    QAP_DTHDI_DEC_PRESENTATION_6CH,
    /* Select the 8 channel presentation from bitstream */
    QAP_DTHDI_DEC_PRESENTATION_8CH,
    /* Select the 16 channel presentation from bitstream*/
    QAP_DTHDI_DEC_PRESENTATION_16CH,
    QAP_DTHDI_DEC_PRESENTATION_COUNT,
} QAP_DTHDI_DEC_PRESENTATION;

typedef enum {
    /* LFE channel is not included in the output PCM data */
    QAP_DTHDI_LFEOUT_OFF,
    /* LFE channel is included in the output PCM data if it exists in the bitstream */
    QAP_DTHDI_LFEOUT_ON
} QAP_DTHDI_DEC_LFEOUT_MODES;

/* Following enums supported for run time set-get params in QAP module */
/* Module get params for TH */
typedef enum {
    /* Module output buffer size */
    QAP_TH_MODULE_PARAM_GET_OUT_BUF_SIZE,
    /* DRC mode - refer TH_UDC_COMP */
    QAP_TH_MODULE_PARAM_GET_DRC_MODE,
    /* Output channel configuration */
    QAP_TH_MODULE_PARAM_GET_OUT_CH_CFG,
    /* LFE channel to enable */
    QAP_TH_MODULE_PARAM_GET_LFE_ON,
    QAP_TH_MODULE_PARAM_GET_MAX
} qap_th_module_get_param_t;

/* Module set params for TH */
typedef enum {
    /* DRC mode */
    QAP_TH_MODULE_PARAM_SET_DRC_MODE,
    /* Output channel configuration */
    QAP_TH_MODULE_PARAM_SET_OUT_CH_CFG,
    /* LFE channel to enable */
    QAP_TH_MODULE_PARAM_SET_LFE_ON,
    QAP_TH_MODULE_PARAM_SET_MAX
} qap_th_module_set_param_t;

/* Session get params for TH */
typedef enum {
    /* Synchronous / asynchronous processing */
    QAP_TH_SESSION_PARAM_GET_PROCESSING_TYPE,
    QAP_TH_SESSION_GET_PARAM_MAX
} qap_th_session_get_param_t;

/* Session set params for TH */
typedef enum {
    QAP_TH_SESSION_SET_PARAM_MAX
} qap_th_session_set_param_t;

/* Payload for qap_module_cmd get/set param : @data - to be used to send param specific data to th module */
typedef struct {
    union {
    qap_th_module_get_param_t get_id;
    qap_th_module_set_param_t set_id;
    };
    void *data;
} qap_th_moudle_cmd_payload_t;

/* Payload for qap_session_cmd get/set param : @data - to be used to send param specific data to th module */
typedef struct {
    union {
        qap_th_session_get_param_t get_id;
        qap_th_session_set_param_t set_id;
    };
    void *data;
} qap_th_session_cmd_payload_t;

/* Data stucture to send OAMD from QAP TH */
typedef struct {
    /* TRUE if object audio/ FALSE for channel based */
    bool object_audio;
    /* metadata */
    void *meta;
} qap_th_meta_t;

/* Control parameter structure */
typedef enum {
    /* If plugin is run for Dolby CIDK certification,
     * channel reordering of buffer is not done at QAP & GST.
     */
    QAP_TH_USECASE_CIDK = 0,
    /* If plugin is run for Dolby IIDK certification,
     * channel reordering done at QAP only
     */
    QAP_TH_USECASE_IIDK = 1,
    /* If plugin is run in pipeline (gst-launch),
     * channel reordering done at QAP and GST
     */
    QAP_TH_USECASE_GST = 2
} qap_th_usecase_t;

/* Supported channel output modes  */
typedef enum {
    QAP_TH_OUTPUT_CHAN_ID_RAW = 0,
    QAP_TH_OUTPUT_CHAN_ID_C = 1,
    QAP_TH_OUTPUT_CHAN_ID_L_R = 2,
    QAP_TH_OUTPUT_CHAN_ID_L_R_C_l = 3,
    QAP_TH_OUTPUT_CHAN_ID_L_R_l_Ls_Rs = 6,
    QAP_TH_OUTPUT_CHAN_ID_L_R_C_l_Ls_Rs = 7,
    QAP_TH_OUTPUT_CHAN_ID_L_R_C_l_Cvh = 8,
    QAP_TH_OUTPUT_CHAN_ID_L_R_l_Ls_Rs_Ts = 9,
    QAP_TH_OUTPUT_CHAN_ID_L_R_C_l_Ls_Rs_Ts = 10,
    QAP_TH_OUTPUT_CHAN_ID_L_R_C_l_Ls_Rs_Cvh = 11,
    QAP_TH_OUTPUT_CHAN_ID_L_R_C_l_Lc_Rc = 12,
    QAP_TH_OUTPUT_CHAN_ID_L_R_l_Ls_Rs_Lw_Rw = 13,
    QAP_TH_OUTPUT_CHAN_ID_L_R_l_Ls_Rs_Lvh_Rvh = 14,
    QAP_TH_OUTPUT_CHAN_ID_L_R_l_Ls_Rs_Lsd_Rsd = 15,
    QAP_TH_OUTPUT_CHAN_ID_L_R_l_Ls_Rs_Lrs_Rrs = 16,
    QAP_TH_OUTPUT_CHAN_ID_L_R_C_l_Ls_Rs_Lc_Rc = 17,
    QAP_TH_OUTPUT_CHAN_ID_L_R_C_l_Ls_Rs_Lw_Rw = 18,
    QAP_TH_OUTPUT_CHAN_ID_L_R_C_l_Ls_Rs_Lvh_Rvh = 19,
    QAP_TH_OUTPUT_CHAN_ID_L_R_C_l_Ls_Rs_Lsd_Rsd = 20,
    QAP_TH_OUTPUT_CHAN_ID_L_R_C_l_Ls_Rs_Lrs_Rrs = 21,
    QAP_TH_OUTPUT_CHAN_ID_L_R_C_l_Ls_Rs_Ts_Cvh = 22,
    QAP_TH_OUTPUT_CHAN_ID_L_R_C_l_Ls_Rs_Lts_Rts = 28,
} qap_th_output_cfg_t;

typedef struct {
    /* True HD Presentation mode */
    QAP_DTHDI_DEC_PRESENTATION presentation;
    /* DRC mode for True HD Decoder */
    QAP_DTHDI_DEC_DRC_ENABLE_MODE drc_mode;
    /* DRC cut value from 0-100 */
    int32_t drc_cut;
    /* DRC boost value from 0-100 */
    int32_t drc_boost;
    /* True HD loudness management */
    bool loudness_mgmt_enable;
} th_output_params_t;

/**
 * th_config_params_t is decoder context configured at init time by the application. This context will be conifgured in the Gstreamer
 * plugin through set_property as void type param from the application. Gstreamer will pass this context to QAP during qap_module_init
 * call for creation and configuration of TH decoder instance. Later qap_th_module_get_param_t will be used in qap_module_cmd to control the
 * run-time properties of True HD plugin.
 */

/* Decoder context */
typedef struct {
    /* Select test case to run */
    qap_th_usecase_t usecase;
    /* Direct decode selection */
    bool direct_decode;
    /* Quit on err */
    bool quit_on_err;
    /* Atmos enable */
    bool atmos_enable;
    /* Requested QAP_DTHDI_DEC_CH_CFG_* value */
    qap_th_output_cfg_t out_ch_cfg;
    /* Requested Presantation Selection */
    QAP_DTHDI_DEC_PRESENTATION pres_selection;
    /* Number of PCM outputs */
    int32_t num_of_pcmout;
    /* Output LFE channel present */
    int32_t out_lfe;
    /* The data type of the dlb_buffer for the PCM output */
    int32_t data_type;
    /* user-specified subroutine parameters */
    th_output_params_t output_params[QAP_TH_PCMOUT_COUNT];
} th_config_params_t;

#if __cplusplus
} // extern "C"
#endif

#endif //DOLBY_TH_DECODER_H
