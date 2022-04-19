/* isp_common.h
 *
 * Copyright (c) 2012-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __ISP_COMMON_H__
#define __ISP_COMMON_H__

/* std headers */
#include <math.h>

/* kernel headers */
#include <media/msmb_isp.h>

/* mctl headers */
#include "media_controller.h"
#include "modules.h"
#include "mct_event_stats.h"
#include "mct_pipeline.h"
#include "chromatix.h"
#include "chromatix_common.h"
#include "chromatix_iot.h"

#include "eztune_vfe_diagnostics.h"

/* isp headers */
#include "isp_defs.h"

/* Gamma is largest dmi table 512 entries * 3 tables * 4 byte each entry */
#define MAX_GAMMA_NUM_ENTRIES    512
#define MAX_GAMMA_DMI_TBL_SIZE  (MAX_GAMMA_NUM_ENTRIES * 3 * 4)
#define MAX_DMI_TBL_SIZE         MAX_GAMMA_DMI_TBL_SIZE

/* Limits on viewfinder dimensions */
#define ISP44_VIEWFINDER_MAX_WIDTH  2560
#define ISP44_VIEWFINDER_MAX_HEIGHT 2048

/*Temporal Coefficients */
#define TEMPORAL_WEIGHTS  0.25f
#define TEMPORAL_STRENGTH 0.25f

/*This macro will return FALSE to ret if evaluated expression returned NULL*/
#define RETURN_IF_NULL(expr)         \
    if ((expr) == NULL) {            \
        ISP_ERR("failed: NULL pointer detected "); \
        return FALSE;                \
     }

/* Macro to jump to error label when expression evalueted is false*/
#define GOTO_ERROR_IF_NULL(expr)                         \
    if ((expr) == NULL) {                            \
        ISP_ERR("failed"); \
        goto error;                                 \
     }

/* Macro to return FALSE when expression evalueted is false*/
#define RETURN_IF_FALSE(expr) \
    if ((expr) == FALSE) { \
        ISP_ERR("failed"); \
        return FALSE; \
     }

/* Macro to jump to error label when expression evalueted is false*/
#define GOTO_ERROR_IF_FALSE(expr) \
    if ((expr) == FALSE) { \
        ISP_ERR("failed"); \
        goto error; \
     }

/* Macro to jump to error label when expression evalueted is false*/
#define GOTO_ERROR_IF_NULL(expr) \
    if ((expr) == NULL) { \
        ISP_ERR("failed"); \
        goto error; \
     }

/* Macro to return FALSE when expression evalueted is false*/
#define RETURN_IF_FALSE(expr)                         \
    if ((expr) == FALSE) {                            \
        ISP_ERR("failed"); \
        return FALSE;                                 \
     }

/* Macro to jump to error label when expression evalueted is false*/
#define GOTO_ERROR_IF_FALSE(expr)                         \
    if ((expr) == FALSE) {                            \
        ISP_ERR("failed"); \
        goto error;                                 \
     }

#define CLAMP_LIMIT(x, t1, t2) (((x) <= (t1))? (t1): ((x) > (t2))? (t2): (x))

#define LTM_MC_TBL_SIZE 256
#define LTM_MS_TBL_SIZE 256
#define LTM_SC_TBL_SIZE 256
#define LTM_SS_TBL_SIZE 256
#define LTM_MASK_TBL_SIZE 256
#define LTM_W_TBL_SIZE  54

#define GTM_LUT_NUM_BIN  65
#define CHROMATIX_BITWIDTH 12
#define ISP_CHROMATIX_BIT_DIFF (ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH)
#define NON_HDR_MUL_FACTOR (1.0f * (float)(1 << ISP_CHROMATIX_BIT_DIFF))
#define HDR_OUTPUT_BIT_WIDTH 10

typedef enum {
  ISP_MOD_LINEARIZATION              = 0,
  ISP_MOD_ROLLOFF                    = 1,
  ISP_MOD_DEMUX                      = 2,
  ISP_MOD_BPC                        = 3,
  ISP_MOD_BCC                        = 4,
  ISP_MOD_ABF                        = 5,
  ISP_MOD_BE_STATS                   = 6,
  ISP_MOD_BG_STATS                   = 7,
  ISP_MOD_BF_STATS                   = 8,
  ISP_MOD_AWB_STATS                  = 9,
  ISP_MOD_RS_STATS                   = 10,
  ISP_MOD_CS_STATS                   = 11,
  ISP_MOD_IHIST_STATS                = 12,
  ISP_MOD_SKIN_BHIST_STATS           = 13,
  ISP_MOD_BF_SCALE_STATS             = 14,
  ISP_MOD_DEMOSAIC                   = 15,
  ISP_MOD_ASF                        = 16,
  ISP_MOD_GAMMA                      = 17,
  ISP_MOD_WB                         = 18,
  ISP_MOD_LTM                        = 19,
  ISP_MOD_LA                         = 20,
  ISP_MOD_CHROMA_ENHANCE             = 21,
  ISP_MOD_COLOR_CORRECT              = 22,
  ISP_MOD_CLF                        = 23,
  ISP_MOD_CHROMA_SUPPRESS            = 24,
  ISP_MOD_MCE                        = 25,
  ISP_MOD_SCE                        = 26,
  ISP_MOD_COLOR_XFORM_VIEWFINDER     = 27,
  ISP_MOD_SCALER_VIEWFINDER          = 28,
  ISP_MOD_FOV_VIEWFINDER             = 29,
  ISP_MOD_COLOR_XFORM_ENCODER        = 30,
  ISP_MOD_SCALER_ENCODER             = 31,
  ISP_MOD_FOV_ENCODER                = 32,
  ISP_MOD_HDR_BE_STATS               = 33,
  ISP_MOD_HDR_BHIST_STATS            = 34,
  ISP_MOD_GTM                        = 35,
  ISP_MOD_GIC                        = 36,
  ISP_MOD_HDR                        = 37,
  ISP_MOD_PEDESTAL                   = 38,
  ISP_MOD_COLOR_XFORM_VIDEO          = 39,
  ISP_MOD_SCALER_VIDEO               = 40,
  ISP_MOD_FOV_VIDEO                  = 41,
  ISP_MOD_CLAMP_VIEWFINDER           = 42,
  ISP_MOD_CLAMP_ENCODER              = 43,
  ISP_MOD_CLAMP_VIDEO                = 44,
  ISP_MOD_BLS                        = 45,
  ISP_MOD_CAC                        = 46,
  ISP_MOD_SNR                        = 47,
  ISP_MOD_ABCC                       = 48,
  ISP_MOD_AEC_BG_STATS               = 49,
  ISP_MOD_RCCB                       = 50,
  ISP_MOD_PDAF                       = 51,
  ISP_MOD_ALL                        = 52,
  ISP_MOD_MAX_NUM                    = 53,
} isp_hw_module_id_t;

typedef enum {
  ISP_LOG_LINEARIZATION              = 0,
  ISP_LOG_ROLLOFF                    = 1,
  ISP_LOG_DEMUX                      = 2,
  ISP_LOG_BPC                        = 3,
  ISP_LOG_BCC                        = 4,
  ISP_LOG_ABF                        = 5,
  ISP_LOG_BE_STATS                   = 6,
  ISP_LOG_BG_STATS                   = 7,
  ISP_LOG_BF_STATS                   = 8,
  ISP_LOG_AWB_STATS                  = 9,
  ISP_LOG_RS_STATS                   = 10,
  ISP_LOG_CS_STATS                   = 11,
  ISP_LOG_IHIST_STATS                = 12,
  ISP_LOG_BHIST_STATS                = 13,
  ISP_LOG_BF_SCALE_STATS             = 14,
  ISP_LOG_DEMOSAIC                   = 15,
  ISP_LOG_ASF                        = 16,
  ISP_LOG_GAMMA                      = 17,
  ISP_LOG_WB                         = 18,
  ISP_LOG_LTM                        = 19,
  ISP_LOG_LA                         = 20,
  ISP_LOG_CHROMA_ENHANCE             = 21,
  ISP_LOG_COLOR_CORRECT              = 22,
  ISP_LOG_CLF                        = 23,
  ISP_LOG_CHROMA_SUPPRESS            = 24,
  ISP_LOG_MCE                        = 25,
  ISP_LOG_SCE                        = 26,
  ISP_LOG_COLOR_XFORM                = 27,
  ISP_LOG_SCALER                     = 28,
  ISP_LOG_FOVCROP                    = 29,
  ISP_LOG_COLOR_XFORM_ENCODER        = 30,
  ISP_LOG_SCALER_ENCODER             = 31,
  ISP_LOG_FOV_ENCODER                = 32,
  ISP_LOG_HDR_BE_STATS               = 33,
  ISP_LOG_HDR_BHIST_STATS            = 34,
  ISP_LOG_GTM                        = 35,
  ISP_LOG_GIC                        = 36,
  ISP_LOG_HDR                        = 37,
  ISP_LOG_PEDESTAL                   = 38,
  ISP_LOG_COLOR_XFORM_VIDEO          = 39,
  ISP_LOG_SCALER_VIDEO               = 40,
  ISP_LOG_FOV_VIDEO                  = 41,
  ISP_LOG_CLAMP                      = 42,
  ISP_LOG_CLAMP_ENCODER              = 43,
  ISP_LOG_CLAMP_VIDEO                = 44,
  ISP_LOG_BLSS                       = 45,
  ISP_LOG_CAC                        = 46,
  ISP_LOG_SNR                        = 47,
  ISP_LOG_ABCC                       = 48,
  ISP_LOG_AEC_BG                     = 49,
  ISP_LOG_RCCB                       = 50,
  ISP_LOG_PDAF                       = 51,
  ISP_LOG_COMMON                     = 52,
  ISP_LOG_MAX                        = 53,
} isp_log_sub_modules_t;

typedef enum {
  ISP_STATE_IDLE,
  ISP_STATE_CONFIGURING,
  ISP_STATE_STREAMING,
  ISP_STATE_MAX
} isp_state_t;

typedef enum {
  ISP_PRIVATE_GET_VFE_DIAG_INFO_USER      = 0,
  ISP_PRIVATE_SET_MOD_ENABLE              = 1,
  ISP_PRIVATE_SET_TRIGGER_ENABLE          = 2,
  ISP_PRIVATE_ACTION_RESET                = 3,
  ISP_PRIVATE_SET_TRIGGER_UPDATE          = 4,
  ISP_PRIVATE_FETCH_SCALER_OUTPUT         = 5,
  ISP_PRIVATE_FETCH_SCALER_CROP_REQUEST   = 6,
  ISP_PRIVATE_FETCH_SCALER_HW_STREAM_INFO = 7,
  /* isp_module_enable_bit_info_t */
  ISP_PRIVATE_FETCH_CROP_FACTOR           = 8,
  ISP_PRIVATE_REQUEST_STRIPE_LIMITATION   = 9,
  ISP_PRIVATE_REQUEST_STRIPE_OFFSET       = 10,
  ISP_PRIVATE_SET_STRIPE_INFO             = 11,
  ISP_PRIVATE_REQUEST_STREAM_SPLIT_INFO   = 12,
  ISP_PRIVATE_SET_STREAM_SPLIT_INFO       = 13,
  ISP_PRIVATE_FETCH_RS_CS_STATS_INFO      = 14,
  ISP_PRIVATE_SCALER_OUTPUT_UPDATE        = 15,
  ISP_PRIVATE_REQUEST_FRAME_SKIP          = 16,
  ISP_PRIVATE_CURRENT_SUBMOD_ENABLE       = 17,
  ISP_PRIVATE_FETCH_DEMUX_GAIN            = 18,
  ISP_PRIVATE_FETCH_ROLLOFF_INPUT_TABLE   = 19,
  ISP_PRIVATE_SET_SENSOR_DIM              = 20,
  ISP_PRIVATE_HW_STREAM_LINKING           = 21,
  ISP_PRIVATE_HW_GET_STATS_CAPABILITES    = 22,
  ISP_PRIVATE_REQUEST_ZOOM_PARAMS         = 23,
  ISP_PRIVATE_REQUEST_CDS_CAP             = 24,
  ISP_PRIVATE_HW_LIMITATIONS              = 25,
  ISP_PRIVATE_FETCH_BLKLVL_OFFSET         = 26,
  ISP_PRIVATE_EVENT_GET_MOD_CFG_MASK      = 27,
  ISP_PRIVATE_PASS_SCALER_SCALE_FACTOR    = 28,
  ISP_PRIVATE_GET_GAMMA_INTERPOLATED_TABLE  = 29,
  ISP_PRIVATE_FETCH_GTM_DATA                = 30,
  ISP_PRIVATE_MAX                         = 31
} isp_private_event_type_t;

typedef enum {
  ISP_MODULE_EVENT_INVALID                         = 0,
  ISP_MODULE_EVENT_SET_STREAM_CONFIG               = 1,
  ISP_MODULE_EVENT_SET_STREAM_CONFIG_FOR_FLASH     = 2,
  ISP_MODULE_EVENT_SET_CHROMATIX_PTR               = 3,
  ISP_MODULE_EVENT_SET_FLASH_MODE                  = 4,
  ISP_MODULE_EVENT_AF_EXP_COMPENSATE               = 5,
  ISP_MODULE_EVENT_STATS_AEC_UPDATE                = 6,
  ISP_MODULE_EVENT_STATS_AWB_UPDATE                = 7,
  ISP_MODULE_EVENT_STATS_ASD_UPDATE                = 8,
  ISP_MODULE_EVENT_STATS_UPDATE                    = 9,
  ISP_MODULE_EVENT_STATS_DIS_UPDATE                = 10,
  ISP_MODULE_EVENT_STATS_AEC_CONFIG_UPDATE         = 11,
  ISP_MODULE_EVENT_STATS_AWB_CONFIG_UPDATE         = 12,
  ISP_MODULE_EVENT_STATS_AF_CONFIG_UPDATE          = 13,
  ISP_MODULE_EVENT_STATS_RS_CONFIG_UPDATE          = 14,
  ISP_MODULE_EVENT_SET_DIGITAL_GAIN                = 15,
  ISP_MODULE_EVENT_GET_ISP_TABLES                  = 16,
  ISP_MODULE_EVENT_SET_FAST_AEC_CONVERGE_MODE      = 17,
  ISP_MODULE_EVENT_ISP_PRIVATE_EVENT               = 18,
  ISP_MODULE_EVENT_SET_AF_ROLLOFF_PARAMS           = 19,
  ISP_MODULE_EVENT_SENSOR_LENS_POSITION_UPDATE     = 20,
  ISP_MODULE_EVENT_LA_ALGO_UPDATE                  = 21,
  ISP_MODULE_EVENT_LTM_ALGO_UPDATE                 = 22,
  ISP_MODULE_EVENT_CDS_REQUEST                     = 23,
  ISP_MODULE_EVENT_GTM_ALGO_UPDATE                 = 24,
  ISP_MODULE_EVENT_MANUAL_AWB_UPDATE               = 25,
  ISP_MODULE_EVENT_TINTLESS_ALGO_UPDATE            = 26,
  ISP_MODULE_EVENT_OFFLINE_CONFIG_OVERWRITE        = 27,
  ISP_MODULE_EVENT_STATS_AF_UPDATE                 = 28,
  ISP_MODULE_EVENT_SET_DEFECTIVE_PIXELS            = 29,
  ISP_MODULE_EVENT_REQUEST_STATS_TYPE              = 30,
  ISP_MODULE_EVENT_SET_SENSOR_HDR_MODE             = 31,
  ISP_MODULE_EVENT_SET_PDAF_PATTERN                = 32,
  ISP_MODULE_EVENT_ISP_DISABLE_MODULE              = 33,
  ISP_MODULE_EVENT_STATS_FOVC_MAGNIFICATION_FACTOR = 34,
  ISP_MODULE_EVENT_STATS_AEC_MANUAL_UPDATE         = 35,
  ISP_MODULE_EVENT_MAX                             = 36,
} isp_module_event_type_t;

typedef enum {
  ISP_CONTROL_EVENT_STREAMON                       = 0,
  ISP_CONTROL_EVENT_STREAMOFF                      = 1,
  ISP_CONTROL_EVENT_SET_PARM                       = 2,
  ISP_CONTROL_EVENT_SET_SUPER_PARM                 = 3,
  ISP_CONTROL_EVENT_MAX                            = 4
} isp_control_event_type_t;

typedef enum awb_cct_type {
  AWB_CCT_TYPE_D65,
  AWB_CCT_TYPE_D65_TL84,
  AWB_CCT_TYPE_TL84,
  AWB_CCT_TYPE_TL84_A,
  AWB_CCT_TYPE_A,
  AWB_CCT_TYPE_D50,
  AWB_CCT_TYPE_D50_TL84,
  AWB_CCT_TYPE_D50_D65,
  AWB_CCT_TYPE_A_H,
  AWB_CCT_TYPE_H,
  AWB_CCT_TYPE_MAX,
}awb_cct_type;

/** cct_trigger_type
 *
 *  @mired_start: mired start point
 *  @mired_end: mired end point
 **/
typedef struct{
  float mired_start;
  float mired_end;
}cct_trigger_type;

/** cct_trigger_type
 *
 *  @trigger_A: mired trigger info A
 *  @trigger_d65: mired trigger info D65
 *  @mired_color_temp: mired color tempurature
 **/
typedef struct {
  cct_trigger_type trigger_A;
  cct_trigger_type trigger_d50;
  cct_trigger_type trigger_d65;
  cct_trigger_type trigger_H;
  float mired_color_temp;
}cct_trigger_info;

typedef enum {
  ISP_SET_PARM_CONTRAST                           = 0,
  ISP_SET_PARM_SATURATION                         = 1,
  ISP_SET_PARM_ZOOM                               = 2,
  ISP_SET_META_SCALER_CROP_REGION                 = 3,
  ISP_SET_PARM_EFFECT                             = 4,
  ISP_SET_META_MODE                               = 5,
  ISP_SET_PARM_WHITE_BALANCE                      = 6,
  ISP_SET_META_NOISE_REDUCTION_MODE               = 7,
  ISP_SET_META_LENS_SHADING_MODE                  = 8,
  ISP_SET_META_LENS_SHADING_MAP_MODE              = 9,
  ISP_SET_META_TONEMAP_MODE                       = 10,
  ISP_SET_META_TONEMAP_CURVES                     = 11,
  ISP_SET_META_COLOR_CORRECT_MODE                 = 12,
  ISP_SET_META_COLOR_CORRECT_GAINS                = 13,
  ISP_SET_META_COLOR_CORRECT_TRANSFORM            = 14,
  ISP_SET_META_BLACK_LEVEL_LOCK                   = 15,
  ISP_SET_PARM_BESTSHOT_MODE                      = 16,
  ISP_SET_PARM_SCE_FACTOR                         = 17,
  ISP_SET_PARM_HFR                                = 18,
  ISP_SET_PARM_DIS_ENABLE                         = 19,
  ISP_SET_PARM_FRAMESKIP                          = 20,
  ISP_SET_PARM_RECORDING_HINT                     = 21,
  ISP_SET_PARM_TINTLESS                           = 22,
  ISP_SET_PARM_SET_VFE_COMMAND                    = 23,
  ISP_SET_PARM_VT                                 = 24,
  ISP_SET_PARM_HDR                                = 25,
  ISP_SET_INTF_PARM_CAC                           = 26,
  ISP_SET_PARM_UPDATE_DEBUG_LEVEL                 = 27,
  ISP_SET_INTF_PARM_LTM_MODE                      = 28,
  ISP_SET_INTF_HOTPIXEL_MODE                      = 29,
  ISP_SET_DUAL_LED_CALIBRATION                    = 30,
  ISP_SET_PARM_SENSOR_HDR_MODE                    = 31,
  ISP_SET_PARM_LONGSHOT_ENABLE                    = 32,
  ISP_SET_AEC_MODE                                = 33,
  ISP_SET_MANUAL_GAIN                             = 34,
  ISP_SET_PARM_BHIST_META_ENABLE                  = 35,
  ISP_SET_PARM_ZSL_MODE                           = 36,
  ISP_SET_PARM_BOKEH_MODE                         = 37,
  ISP_SET_MAX                                     = 38
} isp_set_param_type_t;

typedef enum {
  TRIGGER_LOWLIGHT,
  TRIGGER_NORMAL,
  TRIGGER_OUTDOOR,
} trigger_lighting_t;

/** isp_module_enable_info_t:
 *
 *  @reconfig_needed: boolean flag to determine whether
 *                  reconfig needed
 *  @submod_mask: enable if reconfigure needed per module
 *  @submod_enable: enable / disable submodule bit
 **/
typedef struct {
  boolean reconfig_needed;
  uint8_t submod_mask[ISP_MOD_MAX_NUM];
  uint8_t submod_enable[ISP_MOD_MAX_NUM];
} isp_module_enable_info_t;

/** isp_effects_params_t
 *
 *  @contrast: contrast
 *  @hue: hue
 *  @saturation: saturation
 *  @spl_effect: spl effect
 *  @effect_type_mask: bit of isp_effect_type_t
 **/
typedef struct {
  int32_t               contrast;
  float                 hue;
  float                 saturation;
  cam_effect_mode_type  spl_effect;
  uint32_t              effect_type_mask;
}isp_effects_params_t;

typedef struct {
  float ratio;
  float ratio_adj;
  trigger_lighting_t lighting;
} trigger_ratio_t;

/** isp_mod_set_enable_t:
 *
 *  @enable_mask: enable module based on bit mask - refer
 *              enum in isp_hw_module_id_t
 *  @fast_aec_mode: fast aec mode enable / disable
 **/
typedef struct {
  uint64_t enable_mask;
  uint8_t fast_aec_mode;
} isp_mod_set_enable_t;

/** isp_dim_t
 *
 *  @width: width
 *  @height: height
 **/
typedef struct {
  int32_t width;
  int32_t height;
} isp_dim_t;

/** isp_crop_window_info_t:
 *
 *  @x: left
 *  @y: top
 *  @crop_out_x: width
 *  @crop_out_y: height
 **/
typedef struct {
  uint32_t     x;
  uint32_t     y;
  uint32_t     crop_out_x;
  uint32_t     crop_out_y;
} isp_crop_window_info_t;

/**isp_hw_stream_info_t
 *  @fmt: format
 *  @width: width
 *  @height: height
 **/
typedef struct {
  uint32_t             identity;
  boolean              need_uv_subsample;
  cam_format_t         fmt;
  int32_t              width;
  int32_t              height;
  boolean              dis_enabled;
  int32_t              width_before_dis;
  int32_t              height_before_dis;
  cam_streaming_mode_t streaming_mode;
  float                aspect_ratio;
} isp_hw_stream_info_t;

/**isp_stripe_limit_info_t
 *  @min_overlap: overlap limit
 *  @max_stripe_offset: offset limit
 **/
typedef struct {
  uint32_t min_overlap_left;
  uint32_t min_overlap_right;
  uint32_t max_stripe_offset;
  uint32_t min_left_split;         /* max split point */
  uint32_t smallest_split_point;   /* min split point */
  uint32_t rolloff_min_dist_from_grid;
  uint32_t rolloff_grid_width;
  uint32_t rolloff_subgrid_width;
  uint32_t rolloff_min_dist_from_subgrid;
  uint32_t max_left_split;
  uint32_t max_right_split;
  uint32_t pdaf_enable;
  uint32_t pdaf_hoffset;
  uint32_t pdaf_blockwidth;
} isp_stripe_limit_info_t;

typedef struct {
  boolean                is_chromatix_overwrite;
  modulesChromatix_t     offline_chromatix;
  boolean                is_hw_update_list_overwrite;
  void                   *offline_hw_update_list;
  sensor_out_info_t      sensor_out_info;
  aec_update_t           aec_update;
  awb_update_t           awb_update;
} isp_fetch_eng_cfg_data_t;


/**isp_stripe_request_t
 *  @stripe_limit: stripe limit
 *  @ispif_out_info: ispif_out_info
 **/
typedef struct {
  isp_stripe_limit_info_t stripe_limit;
  ispif_out_info_t        ispif_out_info;
  boolean                 offline_mode;
} isp_stripe_request_t;

/**isp_hw_rs_cs_stats_info_t
 *  @num_rows: rows
 *  @num_cols: columns
 **/
typedef struct {
  int32_t num_rows;
  int32_t num_cols;
} isp_hw_rs_cs_stats_info_t;

/** isp_pixel_window_info_t:
 *
 *  @width: left
 *  @height: top
 *  @scaling_factor: width
 **/
typedef struct {
  uint32_t           width;
  uint32_t           height;
  float              scaling_factor;
  uint32_t           right_split_y_width;
  uint32_t           right_split_cbcr_width;
  cam_crop_region_t  modified_crop_window;
} isp_pixel_window_info_t;

/** isp_private_event_t:
 *
 *  @type: private event type
 *  @data: private event data
 *  @data_size: private event data size
 **/
typedef struct {
  isp_private_event_type_t     type;
  void                        *data;
  uint32_t                     data_size;
} isp_private_event_t;

/** isp_stats_config_t:
 *
 *  @stats_mask: stats mask
 *  @aec_config: aec config
 *  @awb_config: awb config
 *  @af_config: af config
 *  @bhist_config: bhist config
 **/
typedef struct {
  uint32_t           stats_mask;
  aec_config_t       aec_config;
  awb_config_t       awb_config;
  af_config_t        af_config;
  aec_bhist_config_t bhist_config;
} isp_stats_config_t;

/** rgns_stats_param_t:
 *
 *  @is_valid: is params valid
 *  @h_rgns_start: h_rgns_start
 *  @h_rgns_end: h_rgns_end
 *  @h_rgns_total: h_rgns_total
 *  @v_rgns_total: v_rgns_total
 **/
typedef struct {
  boolean  is_valid;
  uint32_t h_rgns_start;
  uint32_t h_rgns_end;
  uint32_t h_rgns_total;
  uint32_t v_rgns_total;
} isp_rgns_stats_param_t;

/** isp_stats_roi_params_t:
 *
 *  @rgnWidth: region width
 *  @rgnHeight: region height
 *  @rgnHOffset: region horizontal offset
 *  @rgnVOffset: region vertical offset
 *  @rgnHNum: H num
 *  @rgnVNum: V num
 *  @rMax: max R
 *  @grMax: max Gr
 *  @bMax: max R
 *  @gbMax: max Gb
 **/
typedef struct {
  uint32_t rgnWidth;
  uint32_t rgnHeight;
  uint32_t rgnHOffset;
  uint32_t rgnVOffset;
  uint32_t rgnHNum;
  uint32_t rgnVNum;
  uint16_t rMax;
  uint16_t grMax;
  uint16_t bMax;
  uint16_t gbMax;
} isp_stats_roi_params_t;

/** isp_zoom_params_t:
 *
 *  @identity: identity
 *  @isp_output: ISP output
 *  @crop_window: crop window
 *  @camif_output: camif output
 *  @scaler_output: scaler output
 *  @scaling_ratio: scaling ratio
 *  @fov_output: fov output
 **/
typedef struct {
  uint32_t               identity;
  isp_dim_t              isp_output;
  isp_crop_window_info_t crop_window;
  isp_crop_window_info_t crop_window_calculated;
  isp_dim_t              camif_output;
  isp_dim_t              scaler_output;
  float                  scaling_ratio;
  isp_crop_window_info_t fov_output;
} isp_zoom_params_t;

/** isp_la_8k_type_t:
 *  @offset: default=3.3, range =0.0 to 8.0
 *  @low_beam: default=0.9, range =0.0 to 1.0
 *  @high_beam:default=0.1 , range =0.0 to 1.0
 *  @histogram_cap: default=5.0, range 2.0 to 8.0
 *  @cap_high: default=2.0, range=1.0 to 4.0
 *  @cap_low: default=0.75, range=0.0 to 2.0
 *  @cap_adjust: LA 8k cap adjust
 *  @CDF_50_thr: CDF pix value 50 threshold
 *  LA algo parm paasing from chromatix
 **/
typedef struct {
  float offset;
  float low_beam;
  float high_beam;
  float histogram_cap;
  float cap_high;
  float cap_low;
  float cap_adjust;
  uint16_t CDF_50_thr;
} isp_la_8k_type;

/** isp_gtm_algo_params_t:
 *  @is_valid: valid flag\
 *  @is_prev_key_valid: flag to indicate whether prev_key
 *                      (and prev_max_v_hist) is valid
 *  @prev_key: midtone_key used in previous frame
 *  @prev_max_v_hist: max_v_hist used in previous frame
 *  @params: chromatix parameters for algorithm
 *
 *  GTM specific algo parameters
 **/
typedef struct {
  uint8_t is_valid;
  uint8_t is_prev_key_valid;
  boolean temporal_filter_enable;
  float prev_key;
  float prev_max_v_hist;
  float prev_min_v_hist;
  chromatix_GTM params;
} isp_gtm_algo_params_t;

typedef enum {
  SUBGRID1_1  = 1,
  SUBGRID2_2  = 2,
  SUBGRID4_4  = 4,
  SUBGRID8_8  = 8
} tintless_subgrid_t;

/** isp_tintless_mesh_config_t:
 *
 *  @is_valid: is params valid
 *  @num_mesh_elem_rows: num of rows
 *  @num_mesh_elem_cols: num of columns
 *  @offset_horizontal: horizontal offset
 *  @offset_vertical: vertical offset
 *  @subgrid_width: subgrid width
 *  @subgrid_height: subgrid height
 *  @subgrids: subgrids type
 *  @is_tintless_array_valid: flag for tintless array validity
 *  @tintless_array: tintless array params
 **/
typedef struct {
  boolean                 is_valid;
  uint32_t                num_mesh_elem_rows;
  uint32_t                num_mesh_elem_cols;
  uint32_t                offset_horizontal;
  uint32_t                offset_vertical;
  uint32_t                subgrid_width;
  uint32_t                subgrid_height;
  tintless_subgrid_t      subgrids;
  boolean                 is_tintless_array_valid;
  mesh_rolloff_array_type input_tintless_array;
  boolean                 update_algo;
} isp_tintless_mesh_config_t;

/** isp_tintless_strength_params_t:
 *
 *  @is_tintless_strength_valid: is params valid
 *  @tintless_strength: tintless strength from chromatix
 **/
typedef struct {
  boolean is_tintless_strength_valid;
  chromatix_color_tint_correction_type tintless_strength;
} isp_tintless_strength_params_t;

/** isp_tintless_stats_config_t:
 *
 *  @is_valid: is data valid
 *  @camif_win_w: width of the camif window
 *  @camif_win_h: heigth of the camif window
 *  @stat_elem_w: width of one stat element
 *  @stat_elem_h: heigth of one stat element
 *  @num_stat_elem_rows: number of stat element rows
 *  @num_stat_elem_cols: number of stat element columns
 **/
typedef struct {
  boolean   is_valid;
  uint32_t  camif_win_w;
  uint32_t  camif_win_h;
  uint32_t  stat_elem_w;
  uint32_t  stat_elem_h;
  uint32_t  num_stat_elem_rows;
  uint32_t  num_stat_elem_cols;
  uint32_t  saturation_limit;
  enum msm_isp_stats_type   stats_type;
} isp_tintless_stats_config_t;

/** isp_stretching_gain_t
 *  @r_gain: R channel stretching gain
 *  @gr_gain: GR channel stretching gain
 *  @gb_gain: GB channel stretching gain
 *  @b_gain: B channel stretching gain
 *
 *  Stretching gain params needed by Demux
 **/
typedef struct {
  float r_gain;
  float gr_gain;
  float gb_gain;
  float b_gain;
} isp_stretching_gain_t;

/** isp_algo_params_t
 *  @isp_version: hw version
 *  @is_la_algo_parm_valid: parm valid flag
 *  @la_8k_algo_parm: LA algo input parm
 *  @is_ltm_algo_parm_valid: LTM parm valid flag
 *  @ihist_shift_bits: shift bits from ihist stats
 *  @backlight_scene_severity: from LA module
 *  @ltm_lut_size: LTM curve size from LTM module
 *  @normal_ltm_data: chromatix LTM normal
 *  @backlit_ltm_data: chromatix LTM backlit
 *  @is_algo_gamma_valid: gamma table parm valid flag
 *  @gamma_r: gamma r channel
 *  @gamma_g: gamma g channel
 *  @gamma_b: gamma b channel
 *  @tintless_mesh_config: tintless mesh config
 *  @tintless_stats_config: tintless stats config
 *  @tint_strength: chromatix tint strength config
 *
 *  algo params for isp internel algo
 **/
typedef struct {
  uint32_t               isp_version;
  /*LA CURVE*/
  boolean                is_la_algo_parm_valid;
  isp_la_8k_type         la_8k_algo_parm;
  /*LTM algo*/
  boolean                is_ltm_algo_parm_valid;
  uint32_t               ihist_shift_bits;
  uint32_t               backlight_scene_severity;
  uint32_t               ltm_lut_size;
  chromatix_LTM_type     normal_ltm_data;
  chromatix_LTM_type     backlit_ltm_data;
  /*GAMMA tbl for LTM*/
  boolean  is_algo_gamma_valid;
  uint32_t gamma_r[MAX_GAMMA_NUM_ENTRIES];
  uint32_t gamma_g[MAX_GAMMA_NUM_ENTRIES];
  uint32_t gamma_b[MAX_GAMMA_NUM_ENTRIES];
  /* GTM algo params */
  isp_gtm_algo_params_t  gtm;
  /* Tintless config */
  isp_tintless_mesh_config_t           tintless_mesh_config;
  isp_tintless_stats_config_t          tintless_stats_config[MSM_ISP_STATS_MAX];
  isp_tintless_strength_params_t       tintless_strength_params;
  boolean  ae_bracketing_enable;
  boolean  manual_control_enable;
  float    clamping;
  isp_stretching_gain_t st_gain;
  float    linearization_max_val;
  shdr_ltm_tuning_type autoltm_params;
} isp_algo_params_t;

/** isp_saved_ltm_params_t
 *  @high_clutter: calculated from ihist
 *  @low_clutter: calculated from ihist
 *  @busyness: calculated from bg stats
 *  @prevManuCapBias: calculated from bg stats
 *  @mask_curve: LTM mask curve
 *  @master_curve: LTM master curve
 *  @master_scale: LTM master scale curve
 *  @shift_curve: LTM shift curve
 *  @shift_scale: LTM shift scale curve
 *  @mask_curve_size: mask curve size
 *  @master_scale_size: master curve size
 *  @shift_curve_size: shift curve size
 *  @shift_scale_size: shift scale curve size
 *  @autoltm_obj: pointer of context of autoltm
 *
 *  LTM algo output params
 **/
typedef struct {
  float            high_clutter;
  float            low_clutter;
  float            busyness;
  uint32_t         prevManuCapBias;
  uint32_t         mask_curve[LTM_MASK_TBL_SIZE];
  double           master_curve[LTM_MC_TBL_SIZE];
  double           master_scale[LTM_MS_TBL_SIZE];
  double           shift_curve[LTM_SC_TBL_SIZE];
  double           shift_scale[LTM_SS_TBL_SIZE];
  uint32_t         mask_curve_size;
  uint32_t         master_curve_size;
  uint32_t         master_scale_size;
  uint32_t         shift_curve_size;
  uint32_t         shift_scale_size;
  void*            autoltm_obj;
} isp_saved_ltm_params_t;

/** isp_saved_la_params_t
 *  @is_la_curve_valid: curve valide flag
 *  @la_curve: la algo output
 *  @curve_size: size of la curve
 *
 *  LA algo output params
 **/
typedef struct {
  boolean   is_la_curve_valid;
  uint8_t   la_curve[256];
  uint32_t  curve_size;
} isp_saved_la_params_t;

/** isp_saved_gtm_params_t
 *  @gtm_lut: look up table of Yratio_base (Q12)
 *  @key: internal algorithm state that needs to be
 *        kept for next iteration
 *  @max_v_hist: internal algorithm state that needs
 *               to be kept for next iteration
 *  @is_key_valid: flag indicating whether the key
 *                 (and max_v_hist) is valid
 *  GTM algo output params
 */
typedef struct {
  double     gtm_yout[GTM_LUT_NUM_BIN];
  float     key;
  float     max_v_hist;
  float     min_v_hist;
  uint8_t   is_key_valid;
} isp_saved_gtm_params_t;

/** isp_saved_tintless_params_t:
 *
 *  @tintless_array_param: tintless array param handle
 *  @tintless_params: tintless_params handle
 *  @tintless_frame_skip: frame skip to execute algo
 **/
typedef struct {
  mesh_rolloff_array_type   tintless_array_param;
  void                     *tintless_params;
  uint32_t                  tintless_algo_skip;
  uint32_t                  algo_skip_cnt;
  boolean                   is_flash_mode;
} isp_saved_tintless_params_t;

/** isp_saved_algo_params_t
 *  @la_saved_algo_parm: LA algo output
 *  @ltm_saved_algo_parm: LTM algo output
 *  @gtm_saved_algo_parm: GTM algo output
 *  isp internel algo output params
 **/
typedef struct {
  isp_saved_la_params_t       la_saved_algo_parm;
  isp_saved_ltm_params_t      ltm_saved_algo_parm;
  isp_saved_gtm_params_t      gtm_saved_algo_parm;
  isp_saved_tintless_params_t tintless_saved_algo_parm;
} isp_saved_algo_params_t;

/** bf_fw_stats_cfg_data_t
 *  @bf_fw_roi_cfg_end_pix_order: bf fw stats roi in end pix
 *                              order
 *  @bf_fw_roi_cfg_3a_order: bf fw stats roi given by 3a that
 *  was applied.
 *  roi information needed while parsing
 **/
typedef struct {
  bf_fw_roi_cfg_t             bf_fw_roi_cfg_end_pix_order;
  bf_fw_roi_cfg_t             bf_fw_roi_cfg_3a_order;
} bf_fw_stats_cfg_data_t;

/** isp_saved_stats_params_t:
 *
 *  @stats_params: stats params
 *  @stats_roi_params: stats ROI params
 *  @rs_shift_bits: rs shift width
 *  @cs_shift_bits: cs shift width
 *  @stats_config: stats config that was used by sub modules.
 *               this stats_config shall be passed to 3A along
 *               with parsed stats buffer
 *  @zoom_params: zoom params
 *  @module_enable_info: module enable bit info
 *  @bhist_meta_enable: used to determine if bhist stats need
 *                to be provided in meta data or not
 **/
typedef struct {
  isp_rgns_stats_param_t      rgns_stats[MSM_ISP_STATS_MAX];
  isp_stats_roi_params_t      stats_roi_params[MSM_ISP_STATS_MAX];
  uint32_t                    rs_shift_bits;
  uint32_t                    cs_shift_bits;
  isp_stats_config_t          stats_config;
  cam_zoom_info_t             zoom;
  isp_zoom_params_t           zoom_params[ISP_MAX_STREAMS];
  isp_module_enable_info_t    module_enable_info;
  q3a_be_info_t               be_info;
  bf_fw_stats_cfg_data_t      bf_fw_stats_cfg_info;
  boolean                     is_valid;
  int32_t                     bhist_meta_enable;
} isp_saved_stats_params_t;

typedef enum {
  ISP_METADUMP_LINEARIZATION   = 0,
  ISP_METADUMP_ROLLOFF         = 1,
  ISP_METADUMP_GAMMA           = 2,
  ISP_METADUMP_LA              = 3,
  ISP_METADUMP_LTM             = 4,
  ISP_METADUMP_ABF             = 5,
  ISP_METADUMP_GIC             = 6,
  ISP_METADUMP_PEDESTAL        = 7,
  ISP_METADUMP_GTM             = 8,
  ISP_METADUMP_MAX_NUM         = 9
} isp_meta_dump_module_id_t;

/** mct_bus_msg_isp_gamma_t:
 *    @entry_size: size of each gamma entry. 2 byte for vfe40, 4: vfe44
 *    @num_entries num of entries in gamma table
 *    @num_tables: num of tables gamma posted (R/G/B/all)
 *    @pack_bits: num of bits for value and delta in hw packing
 *    @read_lengh: total length of RGB tables
 *    @hw_dmi_tbl: hw table static memory MAX of GAMMA, LINEARIZATION, ROLLOFF
 */
typedef struct {
  uint16_t entry_size;
  uint16_t num_entries;
  uint32_t pack_bits;
  uint32_t read_length;
  uint8_t  hw_dmi_tbl[MAX_DMI_TBL_SIZE];
} isp_hw_read_info_t;

/** isp_temporal_luxfilter_params_t:
 *
 *  @init_flag: flag to indicate whether
 *                       yslter is need
 *  @speed_coeff:
 *  @lpf_coeff:
 *  @infinite_lux:
 *  @output_lux:
 **/

typedef struct {
  boolean                 init_flag;
  float                   temporal_lux;
  float                   prev_lux;
} isp_temporal_luxfilter_params_t;


/** isp_dmi_tbl_info_t:
 *
 *  @dmi_info: pointer to meta data/dmi tables from gamma/LA
 **/
typedef struct {
  isp_hw_read_info_t    dmi_info[ISP_METADUMP_MAX_NUM];
} isp_dmi_tbl_meta_t;

/** isp_per_frame_meta_t: Generic structure used for posting
 * per frame metadata to mct.
 *
 *  @bestshot_mode: bestshot/scene mode
 *  @vfe_diag_enable: vfe diagnostics feature enable flag
 *  @vfe_diag: vfe diagnostics structure
 *  @awb_update:  awb updae updated from white balance module
 *  @lens_shading_map_mode:    LSM map mode, to send map or not
 *  @lens_shading_mode:        Enable/Disable LSM module
 *  @abf_mode:                 Enable/Disable ABF module
 *  @rolloff_tbl:              Application rolloff table
 *  @tonemap_mode:             Tone map mode fast/manual
 *  @tone_map:                 manual tone map
 *  @cc_mode:                  cc mode fast/manual
 *  @color_correction_matrix:  manual CC matrix
 *  @cc_gain:                  manual WB gains
 *  @app_ctrl_mode:            manual WB mode
 *  @crop_window:              refrence Crop window applied
 *  @black_level_lock:   Enable/Disable trigger for black level
 *  @capture_intent            Snapshor request present
 *  @tone_curve:               manual tone map curve
 **/
typedef struct {
  cam_scene_mode_type          bestshot_mode;
  boolean                      vfe_diag_enable;
  vfe_diagnostics_t            vfe_diag;
  awb_update_t                 awb_update;
  cam_lens_shading_map_mode_t  lens_shading_map_mode;
  cam_lens_shading_mode_t      lens_shading_mode;
  cam_noise_reduction_mode_t   abf_mode;
  cam_aberration_mode_t        cac_mode;
  cam_hotpixel_mode_t          hot_pix_mode;
  cam_lens_shading_map_t       rolloff_tbl;
  cam_tonemap_mode_t           tonemap_mode;
  cam_rgb_tonemap_curves       tone_map;
  cam_color_correct_mode_t     cc_mode;
  cam_color_correct_matrix_t   color_correction_matrix;
  cam_color_correct_gains_t    cc_gain;
  cam_control_mode_t           app_ctrl_mode;
  cam_ae_mode_type             aec_ctrl_mode;
  cam_crop_region_t            crop_window;
  cam_black_level_lock_t       black_level_lock;
  uint8_t                      capture_intent;
  cam_zoom_info_t              zoom_value;
  float                        fovc_zoom;
  cam_effect_mode_type         special_effect;
  int32_t                      contrast;
  int32_t                      saturation;
  cam_profile_tone_curve       tone_curve;
  cam_neutral_col_point_t      neutral_color_point;
  cam_sensor_hdr_type_t        sensor_hdr;
  int32_t                      isp_gain;
  int32_t                      tintless_enable;
} isp_per_frame_meta_t;

/** isp_saved_events_t:
 *
 *  @set_params: Array to store all set params
 *  @set_params_valid: flag to indicate whether set_param is
 *                   valid
 *  @module_events: Array to store all module events
 *  @module_events_valid: flag to indicate whether module event
 *                      is valid
 *  @frame_id: frame id which used to process saved trigger events
 **/
typedef struct {
  mct_event_t *set_params[ISP_SET_MAX];
  boolean      set_params_valid[ISP_SET_MAX];
  mct_event_t *module_events[ISP_MODULE_EVENT_MAX];
  boolean      module_events_valid[ISP_MODULE_EVENT_MAX];
  uint32_t     frame_id;
} isp_saved_events_t;

/** isp_sub_module_output_t:
 *
 *  @hw_update_list: hw update list from every module
 *  @stats_params: saved stats params handle per ISP
 *  @also_params: input to internel algo
 *  @frame_id: frame id
 **/
typedef struct {
  struct msm_vfe_cfg_cmd_list *hw_update_list;
  isp_saved_stats_params_t    *stats_params;
  isp_meta_t                  *meta_dump_params;
  isp_dmi_tbl_meta_t          *dmi_tbl;
  isp_algo_params_t           *algo_params;
  boolean                     *is_cds_update;
  isp_per_frame_meta_t        *frame_meta;
  uint32_t                     frame_id;
  uint32_t                     metadata_dump_enable;
  boolean                      enableLPM;
  isp_saved_events_t          *saved_events;
  mct_hdr_data                *hdr_data;
} isp_sub_module_output_t;

/** Hist_DMI_CfgCmdType:
 *
 *  @set_channel: dmi channel
 *  @set_start_addr: start addr
 *  @table: dmi table to be updated
 *  @reset_channel: reset channel
 *  @reset_start_addr: reset addr
 **/
typedef struct Hist_DMI_CfgCmdType {
  uint32_t set_channel;
  uint32_t set_start_addr;
  uint64_t table[256];
  uint32_t reset_channel;
  uint32_t reset_start_addr;
}Hist_DMI_CfgCmdType;

typedef enum {
  ISP_HW_STREAM_VIEWFINDER,
  ISP_HW_STREAM_ENCODER,
  ISP_HW_STREAM_VIDEO,
  ISP_HW_STREAM_IDEAL_RAW,
  ISP_HW_STREAM_OFFLINE,
  ISP_HW_STREAM_MAX,
} isp_hw_streamid_t;

typedef enum {
  CAMERA_FLASH_NONE,
  CAMERA_FLASH_LED,
  CAMERA_FLASH_STROBE,
} camera_flash_type;

/** isp_submod_hw_limit_desc_t
 *
 *  @hw_stream_id: HW streams id
 *  @max_width: max supported width
 *  @max_height: max supported height
 *  @max_scale_ratio: max supported downscale ration
 **/
typedef struct {
  isp_hw_streamid_t hw_stream_id;
  uint32_t          max_width;
  uint32_t          max_height;
  uint32_t          max_scale_ratio;
} isp_scaler_hw_limit_desc_t;

/** isp_submod_hw_limit_t
 *
 *  @num_hw_streams: stream number
 *  @hw_limits: HW limitation description
 **/
typedef struct {
  uint32_t          num_hw_streams;
  isp_scaler_hw_limit_desc_t hw_limits[ISP_HW_STREAM_MAX];
} isp_scaler_hw_limit_t;

/** isp_module_enable_LPM_info_t:
 *
 *  @informLPM: boolean to decide whether LPM mode should be informed
 *  @cfg_info: struct if type isp_module_enable_info_t
 **/
typedef struct {
  boolean enableLPM;
  isp_module_enable_info_t cfg_info;
} isp_module_enable_LPM_info_t;

typedef enum {
  ISP_CAM_BPP_8   = 1,
  ISP_CAM_BPP_10  = 2,
  ISP_CAM_BPP_12  = 3,
  ISP_CAM_BPP_14  = 4,
  ISP_CAM_BPP_MAX = 5,
} isp_cam_bpp_format;

typedef enum {
  ISP_CAM_BAYER_GR     = 1,
  ISP_CAM_BAYER_BG     = 2,
  ISP_CAM_BAYER_GB     = 3,
  ISP_CAM_BAYER_RG     = 4,
  ISP_CAM_BAYER_YCBYCR = 5,
  ISP_CAM_BAYER_YCRYCB = 6,
  ISP_CAM_BAYER_CBYCRY = 7,
  ISP_CAM_BAYER_CRYCBY = 8,
  ISP_CAM_BAYER_MAX    = 9,
} isp_cam_bayer_format;

typedef enum {
  ISP_CAM_PACK_QCOM    = 1,
  ISP_CAM_PACK_MIPI    = 2,
  ISP_CAM_PACK_DPCM6   = 3,
  ISP_CAM_PACK_DPCM8   = 4,
  ISP_CAM_PACK_PLAIN8  = 5,
  ISP_CAM_PACK_PLAIN16 = 6,
  ISP_CAM_PACK_DPCM10  = 7,
} isp_cam_pack_format;

/** isp_crop_factors_t:
 *
 *  @crop_applied: crop window to applied
 *  @crop_to_calculate: crop to calculate final window
 **/
typedef struct {
  uint32_t crop_applied;
  uint32_t crop_to_calculate;
} isp_crop_factors_t;

#endif
