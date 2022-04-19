/*============================================================================
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ============================================================================*/

#include <stdbool.h>
#include <stdint.h>

/* App control structure to set tbhdx app control parameters */
typedef struct {
    int32_t spksize;
    float basslvl;
    float hpratio;
    float extbass;
} qap_dts_tbhdx_app_ctrl_t;

/**
 * soundx module parameters IDs used to set configuration parameters during INIT
 * and run time for both get/set apis
 */
typedef enum {
    ID_0,
    QAP_SOUNDX_PROCESS_INTERLEAVED_DATA,  //uses bool as payload data
/*Soundx parameter IDs*/
    QAP_SOUNDX_EN,                        //uses bool as payload data
    QAP_SOUNDX_DISCARD,                   //uses bool as payload data
    QAP_SOUNDX_HEIGHT_DISCARD,            //uses bool as payload data
    QAP_SOUNDX_IN_MODE,                   //uses int32_t as payload data
    QAP_SOUNDX_OUT_MODE,                  //uses int32_t as payload data
    QAP_SOUNDX_IN_GAIN,                   //uses double as payload data
    QAP_SOUNDX_OUT_GAIN,                  //uses double as payload data
    QAP_SOUNDX_DOWNMIX_COEFF,             //uses double as payload data
    QAP_SOUNDX_VIRTUAL_HEIGHT_EN,         //uses bool as payload data
    QAP_SOUNDX_VIRT_HEIGHT_DECORR_EN,     //uses bool as payload data
    QAP_SOUNDX_VIRT_HEIGHT_ELEVFLT_EN,    //uses bool as payload data
    QAP_SOUNDX_VIRT_HEIGHT_SUR_EN,        //uses bool as payload data
    QAP_SOUNDX_VIRT_HEIGHT_TONE_ADJUST,   //uses int32_t as payload data
    QAP_SOUNDX_HEIGHT_UPMIX_EN,           //uses bool as payload data
    QAP_SOUNDX_HEIGHT_UPMIX_DELAY,        //uses int32_t as payload data
    QAP_SOUNDX_HEIGHT_UPMIX_HPB_EN,       //uses bool as payload data
    QAP_SOUNDX_HEIGHT_UPMIX_HPB_PAN,      //uses int32_t as payload data
    QAP_SOUNDX_HEIGHT_UPMIX_QCLS_CUT_GAIN, //uses double as payload data
    QAP_SOUNDX_HEIGHT_UPMIX_QCLS_BOOST_GAIN,  //uses double as payload data
    QAP_SOUNDX_TSHD2_HORIZ_EN,               //uses bool as payload data
    QAP_SOUNDX_TSHD2_HORIZ_CENTFRNT_MIX,   //uses double as payload data
    QAP_SOUNDX_TSHD2_HORIZ_SUR_LEVEL,      //uses double as payload data
    QAP_SOUNDX_TSHD2_HORIZ_PERCURSEL_EN,     //uses bool as payload data
    QAP_SOUNDX_TSHD2_HORIZ_SUR_EN,           //uses bool as payload data
    QAP_SOUNDX_TSHD2_HORIZ_FRT_TBHDX_EN,     //uses bool as payload data
    QAP_SOUNDX_TSHD2_HORIZ_FRT_TBHDX_PROC_MODE, //uses int32_t as payload data
    QAP_SOUNDX_TSHD2_HORIZ_FRT_TBHDX_SPK_SZ,    //uses int32_t as payload data
    QAP_SOUNDX_TSHD2_HORIZ_FRT_TBHDX_DYNMS,     //uses double as payload data
    QAP_SOUNDX_TSHD2_HORIZ_FRT_TBHDX_HP_EN,     //uses bool as payload data
    QAP_SOUNDX_TSHD2_HORIZ_FRT_TBHDX_HP_ORDER,  //uses int32_t as payload data
    QAP_SOUNDX_TSHD2_HORIZ_FRT_TBHDX_BASS_LVL,  //uses double as payload data
    QAP_SOUNDX_TSHD2_HORIZ_FRT_TBHDX_EXTBASS,   //uses double as payload data
    QAP_SOUNDX_TSHD2_HORIZ_CTR_TBHDX_EN,      //uses bool as payload data
    QAP_SOUNDX_TSHD2_HORIZ_CTR_TBHDX_SPK_SZ,  //uses int32_t as payload data
    QAP_SOUNDX_TSHD2_HORIZ_CTR_TBHDX_HP_ORDER,//uses int32_t as payload data
    QAP_SOUNDX_TSHD2_HORIZ_CTR_TBHDX_DYNMS,   //uses double as payload data
    QAP_SOUNDX_TSHD2_HORIZ_CTR_TBHDX_HP_EN,   //uses bool as payload data
    QAP_SOUNDX_TSHD2_HORIZ_CTR_TBHDX_BASS_LVL, //uses double as payload data
    QAP_SOUNDX_TSHD2_HORIZ_CTR_TBHDX_EXTBASS,  //uses double as payload data
    QAP_SOUNDX_TSHD2_HORIZ_SUR_TBHDX_EN,      //uses bool as payload data
    QAP_SOUNDX_TSHD2_HORIZ_SUR_TBHDX_PROC_MODE, //uses int32_t as payload data
    QAP_SOUNDX_TSHD2_HORIZ_SUR_TBHDX_SPK_SZ,  //uses int32_t as payload data
    QAP_SOUNDX_TSHD2_HORIZ_SUR_TBHDX_DYNMS,   //uses double as payload data
    QAP_SOUNDX_TSHD2_HORIZ_SUR_TBHDX_HP_EN,    //uses bool as payload data
    QAP_SOUNDX_TSHD2_HORIZ_SUR_TBHDX_HP_ORDER, //uses int32_t as payload data
    QAP_SOUNDX_TSHD2_HORIZ_SUR_TBHDX_BASS_LVL, //uses double as payload data
    QAP_SOUNDX_TSHD2_HORIZ_SUR_TBHDX_EXTBASS,  //uses double as payload data
    QAP_SOUNDX_TSHD2_HORIZ_LFE_TBHDX_EN,        //uses bool as payload data
    QAP_SOUNDX_TSHD2_HORIZ_LFE_TBHDX_BASS_LVL,  //uses double as payload data
    QAP_SOUNDX_TSHD2_HORIZ_LFE_TBHDX_SPK_SZ,    //uses int32_t as payload data
    QAP_SOUNDX_TSHD2_HORIZ_LFEGAIN_LEVEL,       //uses double as payload data
    QAP_SOUNDX_TSHD2_HORIZ_FRT_DEF_EN,          //uses bool as payload data
    QAP_SOUNDX_TSHD2_HORIZ_FRT_DEF_LEVEL,       //uses double as payload data
    QAP_SOUNDX_TSHD2_HORIZ_SUR_DEF_EN,          //uses bool as payload data
    QAP_SOUNDX_TSHD2_HORIZ_SUR_DEF_LEVEL,       //uses double as payload data
    QAP_SOUNDX_TSHD2_HORIZ_CTR_DEF_EN,          //uses bool as payload data
    QAP_SOUNDX_TSHD2_HORIZ_CTR_DEF_LEVEL,       //uses double as payload data
    QAP_SOUNDX_TSHD2_HORIZ_DIAGC_EN,            //uses bool as payload data
    QAP_SOUNDX_TSHD2_HORIZ_DIAGC_LEVEL,         //uses double as payload data
    QAP_SOUNDX_TSHD2_HORIZ_FRT_SHPF_EN,          //uses bool as payload data
    QAP_SOUNDX_TSHD2_HORIZ_FRT_SHPF_FREQ,        //uses int32_t as payload data
    QAP_SOUNDX_TSHD2_HORIZ_SUR_SHPF_EN,          //uses bool as payload data
    QAP_SOUNDX_TSHD2_HORIZ_SUR_SHPF_FREQ,        //uses int32_t as payload data
    QAP_SOUNDX_INPUT_SAMPLE_RATE,                //uses int32_t as payload data
    QAP_SOUNDX_BUFFER_LATENCY,                    //uses int32_t as payload data
 /* Filter coefficient IDs */
 /* Coef structure uses qap_dts_tbhdx_app_ctrl_t as payload data */
    QAP_SOUNDX_TSHD2_HORIZ_FRT_TBHDX_COEF_STU,
    QAP_SOUNDX_TSHD2_HORIZ_CTR_TBHDX_COEF_STU,
    QAP_SOUNDX_TSHD2_HORIZ_SUR_TBHDX_COEF_STU,
    QAP_SOUNDX_TSHD2_HORIZ_LFE_TBHDX_COEF_STU,
    QAP_SOUNDX_APP_FRT_SPACEHPF_FREQ,          //uses double as payload data
    QAP_SOUNDX_APP_SUR_SPACEHPF_FREQ,          //uses double as payload data
} qap_soundx_module_cmd_payload_id_t;

/**
 *soundx configuration parameters.Each parameter can be modified by changing
 *soundx value corresponding to above IDs
 */
typedef struct {
    int32_t soundx_process_interleaved_data;
    bool soundx_en;
    int32_t soundx_discard;
    int32_t soundx_height_discard;
    int32_t soundx_in_mode;
    int32_t soundx_out_mode;
    double soundx_in_gain;
    double soundx_out_gain;
    double soundx_downmix_coeff;
    int32_t soundx_virtual_height_en;
    bool soundx_virt_height_decorr_en;
    bool soundx_virt_height_elevflt_en;
    bool soundx_virt_height_sur_en;
    int32_t soundx_virt_height_tone_adjust;
    bool soundx_height_upmix_en;
    int32_t soundx_height_upmix_delay;
    bool soundx_height_upmix_hpb_en;
    int32_t soundx_height_upmix_hpb_pan;
    double soundx_height_upmix_qcls_cut_gain;
    double soundx_height_upmix_qcls_boost_gain;
    bool soundx_tshd2_horiz_en;
    double soundx_tshd2_horiz_centfrnt_mix;
    double soundx_tshd2_horiz_sur_level;
    bool soundx_tshd2_horiz_percursel_en;
    bool soundx_tshd2_horiz_sur_en;
    bool soundx_tshd2_horiz_frt_tbhdx_en;
    int32_t soundx_tshd2_horiz_frt_tbhdx_proc_mode;
    int32_t soundx_tshd2_horiz_frt_tbhdx_spk_sz;
    double soundx_tshd2_horiz_frt_tbhdx_dynms;
    bool soundx_tshd2_horiz_frt_tbhdx_hp_en;
    int32_t soundx_tshd2_horiz_frt_tbhdx_hp_order;
    double soundx_tshd2_horiz_frt_tbhdx_bass_lvl;
    double soundx_tshd2_horiz_frt_tbhdx_extbass;
    bool soundx_tshd2_horiz_ctr_tbhdx_en;
    int32_t soundx_tshd2_horiz_ctr_tbhdx_spk_sz;
    int32_t soundx_tshd2_horiz_ctr_tbhdx_hp_order;
    double soundx_tshd2_horiz_ctr_tbhdx_dynms;
    bool soundx_tshd2_horiz_ctr_tbhdx_hp_en;
    double soundx_tshd2_horiz_ctr_tbhdx_bass_lvl;
    double soundx_tshd2_horiz_ctr_tbhdx_extbass;
    bool soundx_tshd2_horiz_sur_tbhdx_en;
    int32_t soundx_tshd2_horiz_sur_tbhdx_proc_mode;
    int32_t soundx_tshd2_horiz_sur_tbhdx_spk_sz;
    double soundx_tshd2_horiz_sur_tbhdx_dynms;
    bool soundx_tshd2_horiz_sur_tbhdx_hp_en;
    int32_t soundx_tshd2_horiz_sur_tbhdx_hp_order;
    double soundx_tshd2_horiz_sur_tbhdx_bass_lvl;
    double soundx_tshd2_horiz_sur_tbhdx_extbass;
    bool soundx_tshd2_horiz_lfe_tbhdx_en;
    double soundx_tshd2_horiz_lfe_tbhdx_bass_lvl;
    int32_t soundx_tshd2_horiz_lfe_tbhdx_spk_sz;
    double soundx_tshd2_horiz_lfegain_level;
    bool soundx_tshd2_horiz_frt_def_en;
    double soundx_tshd2_horiz_frt_def_level;
    int32_t soundx_tshd2_horiz_sur_def_en;
    double soundx_tshd2_horiz_sur_def_level;
    bool soundx_tshd2_horiz_ctr_def_en;
    double soundx_tshd2_horiz_ctr_def_level;
    bool soundx_tshd2_horiz_diagc_en;
    double soundx_tshd2_horiz_diagc_level;
    bool soundx_tshd2_horiz_frt_shpf_en;
    int32_t soundx_tshd2_horiz_frt_shpf_freq;
    bool soundx_tshd2_horiz_sur_shpf_en;
    int32_t soundx_tshd2_horiz_sur_shpf_freq;
    int32_t soundx_input_sample_rate;
    int32_t soundx_buffer_latency;
    qap_dts_tbhdx_app_ctrl_t soundx_tbhdx_frt_coefs;
    qap_dts_tbhdx_app_ctrl_t soundx_tbhdx_ctr_coefs;
    qap_dts_tbhdx_app_ctrl_t soundx_tbhdx_sur_coefs;
    qap_dts_tbhdx_app_ctrl_t soundx_tbhdx_sub_coefs;
    double soundx_app_frt_spacehpf_freq;
    double soundx_app_sur_spacehpf_freq;
} qap_soundx_config_params_t;

/* Payload for qap_module_cmd get/set param :
 * @data - to be used to send param specific data to soundx module
 * Data to filled in as per the above structure(qap_soundx_config_params_t)
 */
typedef struct {
    qap_soundx_module_cmd_payload_id_t id;
    void *data;
} qap_soundx_session_cmd_payload_t;


