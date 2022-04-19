/* isp_pipeline48.c
 *
 * Copyright (c) 2014-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* std headers */
#include <stdio.h>
/* isp headers */
#include "isp_pipeline.h"
#include "isp_pipeline_reg.h"
#include "isp_pipeline48_parser.h"
#include "isp_defs.h"
#include "isp_log.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_COMMON, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_COMMON, fmt, ##args)

#define ISP_MAX_APPLYING_DELAY  2
#define SOC_ID_SDM455 385

static boolean isp_pipeline48_calculate_roi_map(
  isp_zoom_params_t* zoom_params_arr,
  mct_bus_msg_stream_crop_t *stream_crop, uint32_t identity);
static boolean isp_pipeline48_update_module_cfg(int32_t fd,
  uint8_t *curr_submod_enable, isp_module_enable_info_t *enable_bit_info,
  cam_format_t sensor_fmt);
static boolean isp_pipeline48_map_stats_mask_to_hw(uint32_t *stats_mask);
static boolean isp_pipeline48_pipeline_update_stats_mask_for_mono_sensor(
  uint32_t *stats_mask);

static uint32_t adrc_hw_module_mask (void);

#define ISP_PIX48_CLK_USER_MAX ISP_PIX_CLK_600MHZ

static isp_hw_info_t common_hw[] = {
  {{"pedestal_correct46"}, ISP_MOD_PEDESTAL, MSM_ISP_STATS_MAX},
  {{"black_level48"}, ISP_MOD_BLS, MSM_ISP_STATS_MAX},
  {{"linearization40"}, ISP_MOD_LINEARIZATION, MSM_ISP_STATS_MAX},
  {{"pdaf48"}, ISP_MOD_PDAF, MSM_ISP_STATS_MAX},
  {{"demux48"}, ISP_MOD_DEMUX, MSM_ISP_STATS_MAX},
  {{"hdr48"}, ISP_MOD_HDR, MSM_ISP_STATS_MAX},
  {{"bpc48"}, ISP_MOD_BPC, MSM_ISP_STATS_MAX},
  {{"abf48"}, ISP_MOD_ABF, MSM_ISP_STATS_MAX},
  {{"mesh_rolloff44"}, ISP_MOD_ROLLOFF, MSM_ISP_STATS_MAX},
  {{"gic48"}, ISP_MOD_GIC, MSM_ISP_STATS_MAX},
  {{"demosaic48"}, ISP_MOD_DEMOSAIC, MSM_ISP_STATS_MAX},
  {{"bf_stats47"}, ISP_MOD_BF_STATS, MSM_ISP_STATS_BF},
  {{"bg_stats46"}, ISP_MOD_BG_STATS, MSM_ISP_STATS_BG},
  {{"aec_bg_stats47"}, ISP_MOD_AEC_BG_STATS, MSM_ISP_STATS_HDR_BE}, /* HDR_BE is for AEC */
  {{"hdr_be_stats46"}, ISP_MOD_HDR_BE_STATS, MSM_ISP_STATS_AEC_BG}, /* AEC BG tapped before rolloff for tintless */
  {{"hdr_bhist_stats44"}, ISP_MOD_HDR_BHIST_STATS, MSM_ISP_STATS_HDR_BHIST},
  {{"rs_stats46"}, ISP_MOD_RS_STATS, MSM_ISP_STATS_RS},
  {{"cs_stats46"}, ISP_MOD_CS_STATS, MSM_ISP_STATS_CS},
  {{"ihist_stats46"}, ISP_MOD_IHIST_STATS, MSM_ISP_STATS_IHIST},
  {{"bhist_stats44"}, ISP_MOD_SKIN_BHIST_STATS, MSM_ISP_STATS_BHIST},
  {{"color_correct46"}, ISP_MOD_COLOR_CORRECT, MSM_ISP_STATS_MAX},
  {{"gtm46"}, ISP_MOD_GTM, MSM_ISP_STATS_MAX},
  {{"gamma44"}, ISP_MOD_GAMMA, MSM_ISP_STATS_MAX},
  {{"ltm47"}, ISP_MOD_LTM, MSM_ISP_STATS_MAX},
  {{"chroma_enhan40"}, ISP_MOD_CHROMA_ENHANCE, MSM_ISP_STATS_MAX},
  {{"cac47"}, ISP_MOD_CAC, MSM_ISP_STATS_MAX},
  {{"snr47"}, ISP_MOD_SNR, MSM_ISP_STATS_MAX},
  {{"chroma_suppress40"}, ISP_MOD_CHROMA_SUPPRESS, MSM_ISP_STATS_MAX},
  {{"mce40"}, ISP_MOD_MCE, MSM_ISP_STATS_MAX},
  {{"sce40"}, ISP_MOD_SCE, MSM_ISP_STATS_MAX},
};

/* In v3 No Pedestal, CAC and SNR when compared with v1 */
static isp_hw_info_t common_hw_v3[] = {
  {{"black_level48"}, ISP_MOD_BLS, MSM_ISP_STATS_MAX},
  {{"linearization40"}, ISP_MOD_LINEARIZATION, MSM_ISP_STATS_MAX},
  {{"pdaf48"}, ISP_MOD_PDAF, MSM_ISP_STATS_MAX},
  {{"demux48"}, ISP_MOD_DEMUX, MSM_ISP_STATS_MAX},
  {{"hdr48"}, ISP_MOD_HDR, MSM_ISP_STATS_MAX},
  {{"bpc48"}, ISP_MOD_BPC, MSM_ISP_STATS_MAX},
  {{"abf48"}, ISP_MOD_ABF, MSM_ISP_STATS_MAX},
  {{"mesh_rolloff44"}, ISP_MOD_ROLLOFF, MSM_ISP_STATS_MAX},
  {{"gic48"}, ISP_MOD_GIC, MSM_ISP_STATS_MAX},
  {{"demosaic48"}, ISP_MOD_DEMOSAIC, MSM_ISP_STATS_MAX},
  {{"bf_stats47"}, ISP_MOD_BF_STATS, MSM_ISP_STATS_BF},
  {{"bg_stats46"}, ISP_MOD_BG_STATS, MSM_ISP_STATS_BG},
  {{"aec_bg_stats47"}, ISP_MOD_AEC_BG_STATS, MSM_ISP_STATS_HDR_BE}, /* HDR_BE is for AEC */
  {{"hdr_be_stats46"}, ISP_MOD_HDR_BE_STATS, MSM_ISP_STATS_AEC_BG}, /* AEC BG tapped before rolloff for tintless */
  {{"hdr_bhist_stats44"}, ISP_MOD_HDR_BHIST_STATS, MSM_ISP_STATS_HDR_BHIST},
  {{"rs_stats46"}, ISP_MOD_RS_STATS, MSM_ISP_STATS_RS},
  {{"cs_stats46"}, ISP_MOD_CS_STATS, MSM_ISP_STATS_CS},
  {{"ihist_stats46"}, ISP_MOD_IHIST_STATS, MSM_ISP_STATS_IHIST},
  {{"bhist_stats44"}, ISP_MOD_SKIN_BHIST_STATS, MSM_ISP_STATS_BHIST},
  {{"color_correct46"}, ISP_MOD_COLOR_CORRECT, MSM_ISP_STATS_MAX},
  {{"gtm46"}, ISP_MOD_GTM, MSM_ISP_STATS_MAX},
  {{"gamma44"}, ISP_MOD_GAMMA, MSM_ISP_STATS_MAX},
  {{"ltm47"}, ISP_MOD_LTM, MSM_ISP_STATS_MAX},
  {{"chroma_enhan40"}, ISP_MOD_CHROMA_ENHANCE, MSM_ISP_STATS_MAX},
  {{"chroma_suppress40"}, ISP_MOD_CHROMA_SUPPRESS, MSM_ISP_STATS_MAX},
  {{"mce40"}, ISP_MOD_MCE, MSM_ISP_STATS_MAX},
  {{"sce40"}, ISP_MOD_SCE, MSM_ISP_STATS_MAX},
};

/* In v4 No Pedestal, CAC, SNR, HDR when compared with v1 */
static isp_hw_info_t common_hw_v4[] = {
  {{"black_level48"}, ISP_MOD_BLS, MSM_ISP_STATS_MAX},
  {{"linearization40"}, ISP_MOD_LINEARIZATION, MSM_ISP_STATS_MAX},
  {{"pdaf48"}, ISP_MOD_PDAF, MSM_ISP_STATS_MAX},
  {{"demux48"}, ISP_MOD_DEMUX, MSM_ISP_STATS_MAX},
  {{"bpc48"}, ISP_MOD_BPC, MSM_ISP_STATS_MAX},
  {{"abf48"}, ISP_MOD_ABF, MSM_ISP_STATS_MAX},
  {{"mesh_rolloff44"}, ISP_MOD_ROLLOFF, MSM_ISP_STATS_MAX},
  {{"gic48"}, ISP_MOD_GIC, MSM_ISP_STATS_MAX},
  {{"demosaic48"}, ISP_MOD_DEMOSAIC, MSM_ISP_STATS_MAX},
  {{"bf_stats47"}, ISP_MOD_BF_STATS, MSM_ISP_STATS_BF},
  {{"bg_stats46"}, ISP_MOD_BG_STATS, MSM_ISP_STATS_BG},
  {{"aec_bg_stats47"}, ISP_MOD_AEC_BG_STATS, MSM_ISP_STATS_HDR_BE}, /* HDR_BE is for AEC */
  {{"hdr_be_stats46"}, ISP_MOD_HDR_BE_STATS, MSM_ISP_STATS_AEC_BG}, /* AEC BG tapped before rolloff for tintless */
  {{"hdr_bhist_stats44"}, ISP_MOD_HDR_BHIST_STATS, MSM_ISP_STATS_HDR_BHIST},
  {{"rs_stats46"}, ISP_MOD_RS_STATS, MSM_ISP_STATS_RS},
  {{"cs_stats46"}, ISP_MOD_CS_STATS, MSM_ISP_STATS_CS},
  {{"ihist_stats46"}, ISP_MOD_IHIST_STATS, MSM_ISP_STATS_IHIST},
  {{"bhist_stats44"}, ISP_MOD_SKIN_BHIST_STATS, MSM_ISP_STATS_BHIST},
  {{"color_correct46"}, ISP_MOD_COLOR_CORRECT, MSM_ISP_STATS_MAX},
  {{"gtm46"}, ISP_MOD_GTM, MSM_ISP_STATS_MAX},
  {{"gamma44"}, ISP_MOD_GAMMA, MSM_ISP_STATS_MAX},
  {{"ltm47"}, ISP_MOD_LTM, MSM_ISP_STATS_MAX},
  {{"chroma_enhan40"}, ISP_MOD_CHROMA_ENHANCE, MSM_ISP_STATS_MAX},
  {{"chroma_suppress40"}, ISP_MOD_CHROMA_SUPPRESS, MSM_ISP_STATS_MAX},
  {{"mce40"}, ISP_MOD_MCE, MSM_ISP_STATS_MAX},
  {{"sce40"}, ISP_MOD_SCE, MSM_ISP_STATS_MAX},
};

static isp_hw_params_t common_hw_params = {
  .hw_info_table = common_hw,
  .num_hw = ARRAY_SIZE(common_hw),
};

static isp_hw_params_t common_hw_params_v3 = {
  .hw_info_table = common_hw_v3,
  .num_hw = ARRAY_SIZE(common_hw_v3),
};

static isp_hw_params_t common_hw_params_v4 = {
  .hw_info_table = common_hw_v4,
  .num_hw = ARRAY_SIZE(common_hw_v4),
};

static isp_hw_info_t viewfinder_stream_hw[] = {
  {{"color_xform_viewfinder46"}, ISP_MOD_COLOR_XFORM_VIEWFINDER, MSM_ISP_STATS_MAX},
  {{"scaler_viewfinder46"}, ISP_MOD_SCALER_VIEWFINDER, MSM_ISP_STATS_MAX},
  {{"fovcrop_viewfinder46"}, ISP_MOD_FOV_VIEWFINDER, MSM_ISP_STATS_MAX},
  {{"clamp_viewfinder40"}, ISP_MOD_CLAMP_VIEWFINDER, MSM_ISP_STATS_MAX},
};

static isp_hw_info_t encoder_stream_hw[] = {
  {{"color_xform_encoder46"}, ISP_MOD_COLOR_XFORM_ENCODER, MSM_ISP_STATS_MAX},
  {{"scaler_encoder46"}, ISP_MOD_SCALER_ENCODER, MSM_ISP_STATS_MAX},
  {{"fovcrop_encoder46"}, ISP_MOD_FOV_ENCODER, MSM_ISP_STATS_MAX},
  {{"clamp_encoder40"}, ISP_MOD_CLAMP_ENCODER, MSM_ISP_STATS_MAX},
};

static isp_hw_info_t video_stream_hw[] = {
  {{"color_xform_video46"}, ISP_MOD_COLOR_XFORM_VIDEO, MSM_ISP_STATS_MAX},
  {{"scaler_video46"}, ISP_MOD_SCALER_VIDEO, MSM_ISP_STATS_MAX},
  {{"fovcrop_video46"}, ISP_MOD_FOV_VIDEO, MSM_ISP_STATS_MAX},
  {{"clamp_video40"}, ISP_MOD_CLAMP_VIDEO, MSM_ISP_STATS_MAX},
};

static isp_hw_params_t stream_hw_params[] = {
  {
    .hw_info_table = viewfinder_stream_hw,
    .num_hw = ARRAY_SIZE(viewfinder_stream_hw),
  },
  {
    .hw_info_table = encoder_stream_hw,
    .num_hw = ARRAY_SIZE(encoder_stream_hw),
  },
  {
    .hw_info_table = video_stream_hw,
    .num_hw = ARRAY_SIZE(video_stream_hw),
  },
};

static uint32_t isp_pipeline_stats_tap_location[MSM_ISP_STATS_MAX] = {
  /* AEC BG is HDR BE hardware block. HDR BE and HDR BHIST
   * Taps from same point. Select Tap location from HDR BE stats driver */
  [MSM_ISP_STATS_AEC_BG] =
    (ISP_STATS_TAP_BEFORE_LENS_ROLLOFF | ISP_STATS_TAP_AFTER_LENS_ROLLOFF),
};

static isp_pipeline_stats_func_table_t func_table = {
  .pipeline_fill_stats_info = isp_pipeline48_parser_fill_stats_info,
  .pipeline_stats_parse[MSM_ISP_STATS_BF] = isp_pipeline48_parser_bf_stats,
  .pipeline_stats_parse[MSM_ISP_STATS_BG] = isp_pipeline48_parser_bg_stats,
  .pipeline_stats_parse[MSM_ISP_STATS_AEC_BG] = isp_pipeline48_parser_aec_bg_stats,
  .pipeline_stats_parse[MSM_ISP_STATS_RS] = isp_pipeline48_parser_rs_stats,
  .pipeline_stats_parse[MSM_ISP_STATS_CS] = isp_pipeline48_parser_cs_stats,
  .pipeline_stats_parse[MSM_ISP_STATS_IHIST] =
    isp_pipeline48_parser_ihist_stats,
  .pipeline_stats_parse[MSM_ISP_STATS_BHIST] =
    isp_pipeline48_parser_bhist_stats,
  .pipeline_stats_parse[MSM_ISP_STATS_HDR_BE] = isp_pipeline48_parser_hdr_be_stats,
  .pipeline_stats_parse[MSM_ISP_STATS_HDR_BHIST] =
    isp_pipeline48_parser_hdr_bhist_stats,
  .pipeline_calculate_roi_map = isp_pipeline48_calculate_roi_map,
  .pipeline_update_module_cfg = isp_pipeline48_update_module_cfg,
  .adrc_hw_module_id_mask = adrc_hw_module_mask,
  .pipeline_map_stats_mask_to_hw = isp_pipeline48_map_stats_mask_to_hw,
  .pipeline_update_stats_mask_for_mono_sensor =
    isp_pipeline48_pipeline_update_stats_mask_for_mono_sensor,
};

static isp_pipeline_t isp_pipeline = {
  .common_hw_params = &common_hw_params,
  .stream_hw_params[ISP_HW_STREAM_VIEWFINDER] =
     &stream_hw_params[ISP_HW_STREAM_VIEWFINDER],
  .stream_hw_params[ISP_HW_STREAM_ENCODER] =
     &stream_hw_params[ISP_HW_STREAM_ENCODER],
  .stream_hw_params[ISP_HW_STREAM_VIDEO] =
     &stream_hw_params[ISP_HW_STREAM_VIDEO],
  .func_table = &func_table,
  .supported_stats_tap_location = isp_pipeline_stats_tap_location,
  .is_ahb_clk_cfg_supported = FALSE,
  .single_vfe_cx_level = ISP_CX_LEVEL_NOMINAL,
};

static isp_pipeline_t isp_pipeline_v3 = {
  .common_hw_params = &common_hw_params_v3,
  .stream_hw_params[ISP_HW_STREAM_VIEWFINDER] =
     &stream_hw_params[ISP_HW_STREAM_VIEWFINDER],
  .stream_hw_params[ISP_HW_STREAM_ENCODER] =
     &stream_hw_params[ISP_HW_STREAM_ENCODER],
  .stream_hw_params[ISP_HW_STREAM_VIDEO] =
     &stream_hw_params[ISP_HW_STREAM_VIDEO],
  .func_table = &func_table,
  .supported_stats_tap_location = isp_pipeline_stats_tap_location,
  .is_ahb_clk_cfg_supported = FALSE,
  .single_vfe_cx_level = ISP_CX_LEVEL_NOMINAL,
};

static isp_pipeline_t isp_pipeline_v4 = {
  .common_hw_params = &common_hw_params_v4,
  .stream_hw_params[ISP_HW_STREAM_VIEWFINDER] =
     &stream_hw_params[ISP_HW_STREAM_VIEWFINDER],
  .stream_hw_params[ISP_HW_STREAM_ENCODER] =
     &stream_hw_params[ISP_HW_STREAM_ENCODER],
  .stream_hw_params[ISP_HW_STREAM_VIDEO] =
     &stream_hw_params[ISP_HW_STREAM_VIDEO],
  .func_table = &func_table,
  .supported_stats_tap_location = isp_pipeline_stats_tap_location,
  .is_ahb_clk_cfg_supported = FALSE,
  .single_vfe_cx_level = ISP_CX_LEVEL_NOMINAL,
};

/** adrc_hw_module_mask:
 *
 *  Return adrc module mask
 **/
uint32_t adrc_hw_module_mask()
{
  return (1 << ISP_ADRC_MOD_LTM) | (1 << ISP_ADRC_MOD_GTM);
}

/** isp_pipeline48_calculate_roi_map:
 *
 *  @saved_params: ISP saved params
 *  @stream_crop: stream crop params
 *  @identity: event identity
 *
 *  Calculate ROI map based on camif output, fovcrop and
 *  scaler output
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_pipeline48_calculate_roi_map(
  isp_zoom_params_t* zoom_params_arr,
  mct_bus_msg_stream_crop_t *stream_crop, uint32_t identity)
{
  uint32_t           i = 0,
                     fov_cropped_width = 0,
                     fov_cropped_height = 0;
  isp_zoom_params_t *zoom_params = NULL;

  if (!zoom_params_arr || !stream_crop) {
    ISP_ERR("failed: zoom_params_arr %p stream_crop %p", zoom_params_arr,
      stream_crop);
    return FALSE;
  }

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    zoom_params = &zoom_params_arr[i];
    if (zoom_params->identity == identity) {
      break;
    }
    zoom_params = NULL;
  }

  if ((i >= ISP_MAX_STREAMS) || (!zoom_params)) {
    ISP_ERR("invalid ide %x", identity);
    return FALSE;
  }

  /* Since scaler is present before fov, scale fov region */
  stream_crop->x_map = (float)zoom_params->fov_output.x *
    zoom_params->scaling_ratio;
   stream_crop->y_map= (float)zoom_params->fov_output.y *
    zoom_params->scaling_ratio;
  stream_crop->width_map = (float)zoom_params->fov_output.crop_out_x *
    zoom_params->scaling_ratio;
  stream_crop->height_map = (float)zoom_params->fov_output.crop_out_y *
    zoom_params->scaling_ratio;

  /* Validate whether ROI map is within camif output */
  if ((stream_crop->x_map + stream_crop->width_map) >
    (uint32_t)zoom_params->camif_output.width) {
    ISP_ERR("failed: invalid roi map x %d dx %d camif w %d",
      stream_crop->x_map, stream_crop->width_map,
      zoom_params->camif_output.width);
  }

  if ((stream_crop->y_map + stream_crop->height_map) >
    (uint32_t)zoom_params->camif_output.height) {
    ISP_ERR("failed: invalid roi map y %d dy %d camif y %d",
      stream_crop->y_map, stream_crop->height_map,
      zoom_params->camif_output.height);
  }

  ISP_DBG("ide %x map %d %d %d %d", identity, stream_crop->x_map,
    stream_crop->y_map, stream_crop->width_map, stream_crop->height_map);
  return TRUE;
}

typedef struct {
  ISP_ModuleCfg  val;
  ISP_ModuleCfg  mask;
} module_cfg_t;

/** isp_pipeline48_create_module_cfg_mask:
 *
 *  @curr_submod_enable: submodules currently enabled
 *  @input_enable_bit_info: submod mask and submod enable
 *  @cfg: module cfg output
 *
 *  Convert submod mask and enable from enum order to
 *  register order
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_pipeline48_create_module_cfg_mask(
  uint8_t *curr_submod_enable,
  isp_module_enable_info_t *input_enable_bit_info,
  module_cfg_t *cfg, cam_format_t sensor_fmt)
{
  isp_hw_module_id_t module_id;

  if (!input_enable_bit_info || !cfg) {
    ISP_ERR("failed: %p %p", input_enable_bit_info, cfg);
    return FALSE;
  }

#define SET_FIELD(field, value) \
  { \
    cfg->val.field = value; \
    cfg->mask.field |= ~0; \
  }

  for (module_id = 0; module_id < ISP_MOD_MAX_NUM; module_id++) {
    if (input_enable_bit_info->submod_mask[module_id]) {
      uint8_t enable = input_enable_bit_info->submod_enable[module_id];
      switch (module_id) {
      case ISP_MOD_LINEARIZATION:
        SET_FIELD(mod_en.fields.black, enable);
        break;

      case ISP_MOD_HDR:
        /*HDR recon and mac need to enable together
          to work properly*/
        SET_FIELD(mod_en.fields.hdr_recon, enable);
        SET_FIELD(mod_en.fields.hdr_mac, enable);
        break;

      case ISP_MOD_ROLLOFF:
        SET_FIELD(mod_en.fields.rolloff, enable);
        break;

      case ISP_MOD_DEMUX:
        SET_FIELD(mod_en.fields.demux, enable);
        break;

      case ISP_MOD_BPC:
        SET_FIELD(mod_en.fields.bpc, enable);
        break;

      case ISP_MOD_ABF:
        SET_FIELD(mod_en.fields.abf, enable);
        break;

      case ISP_MOD_DEMOSAIC:
        SET_FIELD(mod_en.fields.demosaic, enable);
        break;

      case ISP_MOD_GAMMA:
        SET_FIELD(mod_en.fields.gamma, enable);
        break;

      case ISP_MOD_LTM:
        SET_FIELD(mod_en.fields.ltm, enable);
        break;

      case ISP_MOD_GTM:
        SET_FIELD(mod_en.fields.gtm, enable);
        break;

      case ISP_MOD_GIC:
        SET_FIELD(mod_en.fields.gic, enable);
        break;

      case ISP_MOD_PEDESTAL:
        SET_FIELD(mod_en.fields.pedestal, enable);
        break;

      case ISP_MOD_PDAF:
        SET_FIELD(mod_en.fields.pdaf, enable);
        break;

      case ISP_MOD_CHROMA_ENHANCE:
        SET_FIELD(mod_en.fields.chroma_enhan, enable);
        switch (sensor_fmt) {
          case CAM_FORMAT_Y_ONLY:
          case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GREY:
          case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GREY:
          case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GREY:
          case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GREY:
            SET_FIELD(stats_cfg.fields.color_conv, 0);
            break;
          default:
            SET_FIELD(stats_cfg.fields.color_conv, enable);
            break;
          }
        break;

      case ISP_MOD_COLOR_CORRECT:
        SET_FIELD(mod_en.fields.color_correct, enable);
        break;

      case ISP_MOD_CHROMA_SUPPRESS:
      case ISP_MOD_MCE:
        SET_FIELD(mod_en.fields.chroma_suppress,
          curr_submod_enable[ISP_MOD_CHROMA_SUPPRESS] ||
          curr_submod_enable[ISP_MOD_MCE]);
        SET_FIELD(cs_mce_cfg.fields.cs_en,
          curr_submod_enable[ISP_MOD_CHROMA_SUPPRESS]);
        SET_FIELD(cs_mce_cfg.fields.mce_en,
          curr_submod_enable[ISP_MOD_MCE]);
        break;

      case ISP_MOD_SCE:
        SET_FIELD(mod_en.fields.skin_enhan, enable);
        break;

      case ISP_MOD_COLOR_XFORM_VIEWFINDER:
        SET_FIELD(mod_en.fields.color_xform_view, enable);
        break;

      case ISP_MOD_SCALER_VIEWFINDER:
        SET_FIELD(mod_en.fields.scaler_view, enable);
        break;

      case ISP_MOD_FOV_VIEWFINDER:
        SET_FIELD(mod_en.fields.crop_view, enable);
        break;

      case ISP_MOD_COLOR_XFORM_ENCODER:
        SET_FIELD(mod_en.fields.color_xform_enc, enable);
        break;

      case ISP_MOD_SCALER_ENCODER:
        SET_FIELD(mod_en.fields.scaler_enc, enable);
        break;

      case ISP_MOD_FOV_ENCODER:
        SET_FIELD(mod_en.fields.crop_enc, enable);
        break;

      case ISP_MOD_COLOR_XFORM_VIDEO:
        SET_FIELD(mod_en.fields.color_xform_vid, enable);
        break;

      case ISP_MOD_SCALER_VIDEO:
        SET_FIELD(mod_en.fields.scaler_vid, enable);
        break;

      case ISP_MOD_FOV_VIDEO:
        SET_FIELD(mod_en.fields.crop_vid, enable);
        break;

      case ISP_MOD_BE_STATS:
      case ISP_MOD_BG_STATS:
      case ISP_MOD_BF_STATS:
      case ISP_MOD_RS_STATS:
      case ISP_MOD_CS_STATS:
      case ISP_MOD_IHIST_STATS:
      case ISP_MOD_HDR_BE_STATS:
      case ISP_MOD_HDR_BHIST_STATS:
      case ISP_MOD_AEC_BG_STATS:
      case ISP_MOD_SKIN_BHIST_STATS:
        /* do nothing for stats, they are
           enabled/disabled in kernel */
        break;

      case ISP_MOD_CLAMP_VIEWFINDER:
      case ISP_MOD_CLAMP_ENCODER:
      case ISP_MOD_CLAMP_VIDEO:
        /* Enabled by default, no specific bit */
        break;

      case ISP_MOD_BLS:
        SET_FIELD(mod_en.fields.bls, enable);
        break;

      case ISP_MOD_SNR:
      case ISP_MOD_CAC:
        SET_FIELD(mod_en.fields.cac_snr,
          curr_submod_enable[ISP_MOD_CAC] || curr_submod_enable[ISP_MOD_SNR]);
        SET_FIELD(cac_cfg.fields.cac_enable,
          curr_submod_enable[ISP_MOD_CAC]);
        SET_FIELD(cac_cfg.fields.snr_enable,
          curr_submod_enable[ISP_MOD_SNR]);
        break;

      default:
        ISP_ERR("invalid module id %d", module_id);
        break;
      }
    }
  }

  return TRUE;
}

/** isp_pipeline_set_reg_cfg_cmd:
 *
 *  @reg_cfg_cmd: vfe config command
 *  @reg_offset: register offset
 *  @mask: mask to indicate what bits to program
 *  @val: value to be programmed
 *
 *  prepares the reg_cfg_cmd for programming to the register
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_pipeline48_set_reg_cfg_cmd(struct msm_vfe_reg_cfg_cmd *reg_cfg_cmd,
  uint32_t reg_offset, uint32_t mask, uint32_t val)
{
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    return FALSE;
  }
  reg_cfg_cmd->cmd_type = VFE_CFG_MASK;
  reg_cfg_cmd->u.mask_info.reg_offset = reg_offset;
  reg_cfg_cmd->u.mask_info.mask = mask;
  reg_cfg_cmd->u.mask_info.val = val;

  return TRUE;
}

/** isp_pipeline48_update_module_cfg:
 *
 *  @fd: file descriptor
 *  @enable_bit_info: input enable bit info
 *
 *  Configure module_cfg register to enable submodules
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_pipeline48_update_module_cfg(int32_t fd,
  uint8_t *curr_submod_enable,
  isp_module_enable_info_t *enable_bit_info, cam_format_t sensor_fmt)
{
  boolean                     ret = TRUE;
  int32_t                     rc = 0;
  struct msm_vfe_reg_cfg_cmd  reg_cfg_cmd[7];
  module_cfg_t                module_cfg;
  struct msm_vfe_cfg_cmd2     cfg_cmd;

  if ((fd <= 0) || !enable_bit_info) {
    ISP_ERR("failed: fd %d enable_bit_info %p sensor_fmt %d", fd,
      enable_bit_info, sensor_fmt);
    return FALSE;
  }

  memset(&reg_cfg_cmd, 0, (sizeof(reg_cfg_cmd)));
  memset(&module_cfg, 0, sizeof(module_cfg));

  ret = isp_pipeline48_create_module_cfg_mask(curr_submod_enable,
          enable_bit_info, &module_cfg, sensor_fmt);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_pipeline48_create_module_cfg_mask");
    return FALSE;
  }

  cfg_cmd.cfg_data = (void *)&module_cfg;
  cfg_cmd.cmd_len = sizeof(module_cfg);
  cfg_cmd.cfg_cmd = (void *)&reg_cfg_cmd;
  cfg_cmd.num_cfg = 7;

  isp_pipeline48_set_reg_cfg_cmd(&reg_cfg_cmd[0], ISP_MODULE_LENS_EN_OFFSET,
    module_cfg.mask.mod_en.values.lens_en,
    module_cfg.val.mod_en.values.lens_en);
  isp_pipeline48_set_reg_cfg_cmd(&reg_cfg_cmd[1], ISP_MODULE_COLOR_EN_OFFSET,
    module_cfg.mask.mod_en.values.color_en,
    module_cfg.val.mod_en.values.color_en);
  isp_pipeline48_set_reg_cfg_cmd(&reg_cfg_cmd[2], ISP_MODULE_ZOOM_EN_OFFSET,
    module_cfg.mask.mod_en.values.zoom_en,
    module_cfg.val.mod_en.values.zoom_en);
  isp_pipeline48_set_reg_cfg_cmd(&reg_cfg_cmd[3], ISP_CS_MCE_CFG_OFFSET,
    module_cfg.mask.cs_mce_cfg.value,
    module_cfg.val.cs_mce_cfg.value);
  isp_pipeline48_set_reg_cfg_cmd(&reg_cfg_cmd[4], ISP_STATS_CFG_OFFSET,
    module_cfg.mask.stats_cfg.value,
    module_cfg.val.stats_cfg.value);
  isp_pipeline48_set_reg_cfg_cmd(&reg_cfg_cmd[6], ISP_CAC_SNR_OFFSET,
    module_cfg.mask.cac_cfg.value,
    module_cfg.val.cac_cfg.value);

  /* Perform ioctl */
  rc = ioctl(fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
  if (rc < 0) {
    ISP_ERR("failed: rc %d", rc);
    ret = FALSE;
  }

  return ret;
}

/** isp_pipeline48_map_stats_mask_to_hw
 *
 *  @stats_mask: mask of available stats modules
 *
 *  Map stats mask based on hw supported stats
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_pipeline48_map_stats_mask_to_hw(uint32_t *stats_mask)
{
  uint8_t aec_bg = 0, hdr_be = 0;

  if (!stats_mask)
    return FALSE;

  /* Extract HDR BE and AEC BG bit */
  hdr_be = (*stats_mask & (1 << MSM_ISP_STATS_HDR_BE)) ? 1: 0;
  aec_bg = (*stats_mask & (1 << MSM_ISP_STATS_AEC_BG)) ? 1: 0;

  /* Remove HDR BE and AEC BG bit */
  *stats_mask &= ~((1 << MSM_ISP_STATS_AEC_BG) | (1 << MSM_ISP_STATS_HDR_BE));

  /* Swap HDR BE and AEC BG and set in stats_mask */
  if (hdr_be)
    *stats_mask |= (1 << MSM_ISP_STATS_AEC_BG);

  if (aec_bg)
    *stats_mask |= (1 << MSM_ISP_STATS_HDR_BE);

  return TRUE;
}

/** isp_pipeline48_open:
 *
 *  @isp_version: ISP version
 *  @isp_id: ISP hw id
 *
 *  Initialize ISP pipeline resources based on isp_version
 *  and isp_id
 *
 *  Return TRUE on success and FALSE on failure
 **/

isp_pipeline_t *isp_pipeline48_open(uint32_t isp_version, uint32_t isp_id)
{
  isp_pipeline_t *curr_pipeline;
  int fd = 0, soc_id = 0;
  char buf[10] = {0};

  if (isp_id >= ISP_HW_MAX) {
    ISP_ERR("failed: invalid isp id %d", isp_id);
    return NULL;
  }

  fd = open("/sys/devices/soc0/soc_id", O_RDONLY);
  if (fd >= 0) {
    if (read(fd, buf, sizeof(buf) - 1) == -1) {
      ISP_ERR("Unable to read soc_id");
    } else {
      soc_id = atoi(buf);
    }
  }

  curr_pipeline = (isp_pipeline_t *)CAM_MALLOC(sizeof(*curr_pipeline));
  if (!curr_pipeline) {
    ISP_ERR("failed");
    return NULL;
  }

  if (isp_version == ISP_SDM660) {
      if (soc_id == SOC_ID_SDM455) {
         memcpy(curr_pipeline, &isp_pipeline_v4, sizeof(*curr_pipeline));
      } else {
         memcpy(curr_pipeline, &isp_pipeline_v3, sizeof(*curr_pipeline));
      }
      curr_pipeline->hw_version = isp_version;
      curr_pipeline->isp_version = SET_ISP_VERSION(ISP_VERSION_48, ISP_REVISION_V3);
      curr_pipeline->max_pix_clk_rate[ISP_CX_LEVEL_NOMINAL] = ISP_PIX_CLK_480MHZ;
      curr_pipeline->max_pix_clk_rate[ISP_CX_LEVEL_TURBO] = ISP_PIX_CLK_576MHZ;

      curr_pipeline->num_pix = 1;
      curr_pipeline->num_rdi = 3;
      curr_pipeline->num_wms = 7;
      curr_pipeline->num_register = ISP48_NUM_REG_DUMP;
      curr_pipeline->isp_id = isp_id;
      curr_pipeline->is_camif_crop_supported = FALSE;
      curr_pipeline->is_camif_raw_op_fmt_supported = TRUE;
      curr_pipeline->is_mono_stats_supported = 1;

      if (isp_id == ISP_HW_0) {
        curr_pipeline->max_width = 5504;
        curr_pipeline->max_height = 16383;
        curr_pipeline->hw_stream_info[ISP_HW_STREAM_ENCODER].max_width = 5504;
        curr_pipeline->hw_stream_info[ISP_HW_STREAM_ENCODER].max_height = 16383;
        curr_pipeline->hw_stream_info[ISP_HW_STREAM_VIEWFINDER].max_width = 4608;
        curr_pipeline->hw_stream_info[ISP_HW_STREAM_VIEWFINDER].max_height = 16383;
        curr_pipeline->hw_stream_info[ISP_HW_STREAM_VIDEO].max_width = 4608;
        curr_pipeline->hw_stream_info[ISP_HW_STREAM_VIDEO].max_height = 16383;
      } else if (isp_id == ISP_HW_1) {
        curr_pipeline->max_width = 5504;
        curr_pipeline->max_height = 16383;
        curr_pipeline->hw_stream_info[ISP_HW_STREAM_ENCODER].max_width = 5504;
        curr_pipeline->hw_stream_info[ISP_HW_STREAM_ENCODER].max_height = 16383;
        curr_pipeline->hw_stream_info[ISP_HW_STREAM_VIEWFINDER].max_width = 4608;
        curr_pipeline->hw_stream_info[ISP_HW_STREAM_VIEWFINDER].max_height = 16383;
        curr_pipeline->hw_stream_info[ISP_HW_STREAM_VIDEO].max_width = 4608;
        curr_pipeline->hw_stream_info[ISP_HW_STREAM_VIDEO].max_height = 16383;
      }

      curr_pipeline->hw_stream_info[ISP_HW_STREAM_IDEAL_RAW].max_width =
        curr_pipeline->hw_stream_info[ISP_HW_STREAM_ENCODER].max_width;
      curr_pipeline->hw_stream_info[ISP_HW_STREAM_IDEAL_RAW].max_height =
        curr_pipeline->hw_stream_info[ISP_HW_STREAM_ENCODER].max_height;

  } else if(isp_version >= ISP_MSM8998) {
    memcpy(curr_pipeline, &isp_pipeline, sizeof(*curr_pipeline));
    curr_pipeline->hw_version = isp_version;
    curr_pipeline->isp_version = SET_ISP_VERSION(ISP_VERSION_48, ISP_REVISION_V1);
    curr_pipeline->max_pix_clk_rate[ISP_CX_LEVEL_NOMINAL] = ISP_PIX48_CLK_USER_MAX;
    curr_pipeline->max_pix_clk_rate[ISP_CX_LEVEL_TURBO] = ISP_PIX_CLK_600MHZ;

    curr_pipeline->num_pix = 1;
    curr_pipeline->num_rdi = 3;
    curr_pipeline->num_wms = 7;
    curr_pipeline->num_register = ISP48_NUM_REG_DUMP;
    curr_pipeline->isp_id = isp_id;
    curr_pipeline->is_camif_crop_supported = FALSE;
    curr_pipeline->is_camif_raw_op_fmt_supported = TRUE;
    curr_pipeline->is_mono_stats_supported = 1;

    if (isp_id == ISP_HW_0) {
      curr_pipeline->max_width = 6272;
      curr_pipeline->max_height = 16383;
      curr_pipeline->hw_stream_info[ISP_HW_STREAM_ENCODER].max_width = 6272;
      curr_pipeline->hw_stream_info[ISP_HW_STREAM_ENCODER].max_height = 16383;
      curr_pipeline->hw_stream_info[ISP_HW_STREAM_VIEWFINDER].max_width = 4608;
      curr_pipeline->hw_stream_info[ISP_HW_STREAM_VIEWFINDER].max_height = 16383;
      curr_pipeline->hw_stream_info[ISP_HW_STREAM_VIDEO].max_width = 4608;
      curr_pipeline->hw_stream_info[ISP_HW_STREAM_VIDEO].max_height = 16383;
    } else if (isp_id == ISP_HW_1) {
      curr_pipeline->max_width = 5504;
      curr_pipeline->max_height = 16383;
      curr_pipeline->hw_stream_info[ISP_HW_STREAM_ENCODER].max_width = 5504;
      curr_pipeline->hw_stream_info[ISP_HW_STREAM_ENCODER].max_height = 16383;
      curr_pipeline->hw_stream_info[ISP_HW_STREAM_VIEWFINDER].max_width = 4608;
      curr_pipeline->hw_stream_info[ISP_HW_STREAM_VIEWFINDER].max_height = 16383;
      curr_pipeline->hw_stream_info[ISP_HW_STREAM_VIDEO].max_width = 4608;
      curr_pipeline->hw_stream_info[ISP_HW_STREAM_VIDEO].max_height = 16383;
    }

    curr_pipeline->hw_stream_info[ISP_HW_STREAM_IDEAL_RAW].max_width =
      curr_pipeline->hw_stream_info[ISP_HW_STREAM_ENCODER].max_width;
    curr_pipeline->hw_stream_info[ISP_HW_STREAM_IDEAL_RAW].max_height =
      curr_pipeline->hw_stream_info[ISP_HW_STREAM_ENCODER].max_height;

  } else {
    ISP_ERR("failed: invalid isp version %x", isp_version);
    free(curr_pipeline);
    return NULL;
  }

  curr_pipeline->num_stats_comp_grp = 2;
  curr_pipeline->max_scale_ratio = 69;
  curr_pipeline->num_hw_streams = ARRAY_SIZE(stream_hw_params);
  curr_pipeline->hw_stream_ids[ISP_HW_STREAM_ENCODER] =
    ISP_HW_STREAM_ENCODER;
  curr_pipeline->hw_stream_ids[ISP_HW_STREAM_VIEWFINDER] =
    ISP_HW_STREAM_VIEWFINDER;
  curr_pipeline->hw_stream_ids[ISP_HW_STREAM_VIDEO] =
    ISP_HW_STREAM_VIDEO;
  curr_pipeline->pipeline_applying_delay = ISP_MAX_APPLYING_DELAY;

  curr_pipeline->is_hvx_present  = 1;

  return curr_pipeline;
}

/** isp_pipeline48_pipeline_update_stats_mask_for_mono_sensor:
 *
 *  @stats_mask: mask of available stats modules
 *
 *  Update stats_mask for Mono sensor to correctly denote
 *  stats available from HW
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_pipeline48_pipeline_update_stats_mask_for_mono_sensor(
  uint32_t *stats_mask)
{
  if (!stats_mask)
    return FALSE;

  return TRUE;
}
