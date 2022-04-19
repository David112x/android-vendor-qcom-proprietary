/* isp_pipeline40.c
 *
 * Copyright (c) 2012-2014, 2017 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <stdio.h>

/* isp headers */
#include "isp_pipeline.h"
#include "isp_pipeline40_parser.h"
#include "isp_defs.h"
#include "isp_log.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_COMMON, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_COMMON, fmt, ##args)

#define ISP_MAX_APPLYING_DELAY               2

#define ISP_PIX40_CLK_USER_MAX ISP_PIX_CLK_320MHZ

#define MODULE_CFG_BLACK_EN                  0
#define MODULE_CFG_LENSROLLOFF_EN            1
#define MODULE_CFG_DEMUX_EN                  2
#define MODULE_CFG_CHROMA_UPSAMPLE_EN        3
#define MODULE_CFG_DEMOSAIC_EN               4
#define MODULE_CFG_WB_EN                     11
#define MODULE_CFG_CLF_EN                    12
#define MODULE_CFG_COLOR_CORRECT_EN          13
#define MODULE_CFG_RGB_LUT_EN                14
#define MODULE_CFG_STATS_IHIST_EN            15
#define MODULE_CFG_LA_EN                     16
#define MODULE_CFG_CHROMA_ENHAN_EN           17
#define MODULE_CFG_STATS_SKIN_BHIST_EN       18
#define MODULE_CFG_CHROMA_SUPPRESS_MCE_EN    19
#define MODULE_CFG_SKIN_ENHAN_EN             20
#define MODULE_CFG_COLOR_XFORM_ENC_EN        21
#define MODULE_CFG_COLOR_XFORM_VIEW_EN       22
#define MODULE_CFG_SCALE_ENC_EN              23
#define MODULE_CFG_SCALE_VIEW_EN             24
#define MODULE_CFG_CROP_ENC_EN               27
#define MODULE_CFG_CROP_VIEW_EN              28

#define MODULE_CFG_MCE_EN                    28
#define MODULE_CFG_CS_EN                     24

#define MODULE_CFG_COLOR_CONV_EN             0
#define MODULE_CFG_BAYER_HIST_SEL            1

#define MODULE_CFG_DBPC_EN                   0
#define MODULE_CFG_DBCC_EN                   1
#define MODULE_CFG_ABF_EN                    3

#define MODULE_CFG_REGISTER_ADDR             0x18
#define CHROMA_SUPPRESS_CFG_1_ADDR           0x668
#define STATS_CFG_ADDR                       0x888
#define DEMOSAIC_CFG_ADDR                    0x440

static boolean isp_pipeline40_update_module_cfg(int32_t fd,
  uint8_t *curr_submod_enable,
  isp_module_enable_info_t *enable_bit_info, cam_format_t sensor_fmt);

typedef enum {
  ISP_PIPELINE40_MODULE_CFG,
  ISP_PIPELINE40_MCE_CS,
  ISP_PIPELINE40_STATS_CFG,
  ISP_PIPELINE40_BPC_BCC_ABF,
  ISP_PIPELINE40_MOD_EN_MAX,
} isp_pipeline40_mod_en_type_t;

static isp_hw_info_t common_hw[] = {
  {{"linearization40"},   ISP_MOD_LINEARIZATION,    MSM_ISP_STATS_MAX},
  {{"mesh_rolloff40"},    ISP_MOD_ROLLOFF,          MSM_ISP_STATS_MAX},
  {{"demux40"},           ISP_MOD_DEMUX,            MSM_ISP_STATS_MAX},
  {{"demosaic40"},        ISP_MOD_DEMOSAIC,         MSM_ISP_STATS_MAX},
  {{"bf_stats40"},        ISP_MOD_BF_STATS,         MSM_ISP_STATS_BF},
  {{"be_stats44"},        ISP_MOD_BE_STATS,         MSM_ISP_STATS_BE},
  {{"bg_stats44"},        ISP_MOD_BG_STATS,         MSM_ISP_STATS_BG},
  {{"rs_stats44"},        ISP_MOD_RS_STATS,         MSM_ISP_STATS_RS},
  {{"cs_stats44"},        ISP_MOD_CS_STATS,         MSM_ISP_STATS_CS},
  {{"ihist_stats44"},     ISP_MOD_IHIST_STATS,      MSM_ISP_STATS_IHIST},
  {{"bhist_stats44"},     ISP_MOD_SKIN_BHIST_STATS, MSM_ISP_STATS_BHIST},
  {{"chroma_enhan40"},    ISP_MOD_CHROMA_ENHANCE,   MSM_ISP_STATS_MAX},
  {{"bpc40"},             ISP_MOD_BPC,              MSM_ISP_STATS_MAX},
  {{"bcc40"},             ISP_MOD_BPC,              MSM_ISP_STATS_MAX},
  {{"abf40"},             ISP_MOD_ABF,              MSM_ISP_STATS_MAX},
  {{"color_correct40"},   ISP_MOD_COLOR_CORRECT,    MSM_ISP_STATS_MAX},
  {{"chroma_suppress40"}, ISP_MOD_CHROMA_SUPPRESS,  MSM_ISP_STATS_MAX},
  {{"luma_adaptation40"}, ISP_MOD_LA,               MSM_ISP_STATS_MAX},
  {{"wb40"},              ISP_MOD_WB,               MSM_ISP_STATS_MAX},
  {{"gamma40"},           ISP_MOD_GAMMA,            MSM_ISP_STATS_MAX},
  {{"mce40"},             ISP_MOD_MCE,              MSM_ISP_STATS_MAX},
  {{"sce40"},             ISP_MOD_SCE,              MSM_ISP_STATS_MAX},
};

static isp_hw_params_t common_hw_params = {
  .hw_info_table = common_hw,
  .num_hw = ARRAY_SIZE(common_hw),
};

static isp_hw_info_t viewfinder_stream_hw[] = {
  {{"color_xform_viewfinder40"}, ISP_MOD_COLOR_XFORM_VIEWFINDER,
  MSM_ISP_STATS_MAX},
  {{"scaler_viewfinder44"},     ISP_MOD_SCALER_VIEWFINDER, MSM_ISP_STATS_MAX},
  {{"fovcrop_viewfinder40"},    ISP_MOD_FOV_VIEWFINDER,    MSM_ISP_STATS_MAX},
  {{"clamp_viewfinder40"},      ISP_MOD_CLAMP_VIEWFINDER,  MSM_ISP_STATS_MAX},
};

static isp_hw_info_t encoder_stream_hw[] = {
  {{"color_xform_encoder40"}, ISP_MOD_COLOR_XFORM_ENCODER, MSM_ISP_STATS_MAX},
  {{"scaler_encoder44"},      ISP_MOD_SCALER_ENCODER,      MSM_ISP_STATS_MAX},
  {{"fovcrop_encoder40"},     ISP_MOD_FOV_ENCODER,         MSM_ISP_STATS_MAX},
  {{"clamp_encoder40"},       ISP_MOD_CLAMP_ENCODER,       MSM_ISP_STATS_MAX},
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
};

/** isp_pipeline40_calculate_roi_map:
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
static boolean isp_pipeline40_calculate_roi_map(
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

static isp_pipeline_stats_func_table_t func_table = {
  .pipeline_fill_stats_info = isp_pipeline40_parser_fill_stats_info,
  .pipeline_calculate_roi_map = isp_pipeline40_calculate_roi_map,
  .pipeline_update_module_cfg = isp_pipeline40_update_module_cfg,
  .pipeline_stats_parse[MSM_ISP_STATS_BG] = isp_pipeline40_parser_bg_stats,
  .pipeline_stats_parse[MSM_ISP_STATS_BF] = isp_pipeline40_parser_bf_stats,
  .pipeline_stats_parse[MSM_ISP_STATS_BE] = isp_pipeline40_parser_be_stats,
  .pipeline_stats_parse[MSM_ISP_STATS_CS] = isp_pipeline40_parser_cs_stats,
  .pipeline_stats_parse[MSM_ISP_STATS_RS] = isp_pipeline40_parser_rs_stats,
  .pipeline_stats_parse[MSM_ISP_STATS_BHIST] = isp_pipeline40_parser_bhist_stats,
  .pipeline_stats_parse[MSM_ISP_STATS_IHIST] = isp_pipeline40_parser_ihist_stats,
};

static isp_pipeline_t isp_pipeline = {
  .common_hw_params = &common_hw_params,
  .stream_hw_params[ISP_HW_STREAM_VIEWFINDER] =
     &stream_hw_params[ISP_HW_STREAM_VIEWFINDER],
  .stream_hw_params[ISP_HW_STREAM_ENCODER] =
     &stream_hw_params[ISP_HW_STREAM_ENCODER],
  .func_table = &func_table,
  .is_ahb_clk_cfg_supported = FALSE,
  .single_vfe_cx_level = ISP_CX_LEVEL_NOMINAL,
};
/** isp_pipeline40_set_register_mask_val_default:
 *
 *  @register_mask: mask to store bits that need change
 *  @register_val: for masked bit, enable / disable bit in
 *               actual register
 *  @mod_en_type: module enable type
 *  @register_bit: register bit to enable / disable
 *  @enable: enable or disable
 *
 *  Set register_mask / register_val by default
 *
 *  Return void
 **/
static inline void isp_pipeline40_set_register_mask_val_default(
  uint32_t *register_mask, uint32_t *register_val,
  isp_pipeline40_mod_en_type_t mod_en_type,
  uint32_t register_bit, uint8_t enable)
{
  if (!register_mask || !register_val ||
    (mod_en_type >= ISP_PIPELINE40_MOD_EN_MAX) || (register_bit > 31)) {
    ISP_ERR("failed: %p %p %d %d", register_mask, register_val, mod_en_type,
      register_bit);
  } else {
    register_mask[mod_en_type] |= (1 << register_bit);
    register_val[mod_en_type] |= enable << register_bit;
  }
}

/** isp_pipeline40_set_register_mask_val:
 *
 *  @register_mask: mask to store bits that need change
 *  @register_val: for masked bit, enable / disable bit in
 *               actual register
 *  @mod_en_type: module enable type
 *  @bit_info: handle to input enable bit info
 *  @module_id: module id
 *  @register_bit: register bit to enable / disable
 *
 *  Set register_mask / register_val based on input enable bit
 *  info
 *
 *  Return void
 **/
static inline void isp_pipeline40_set_register_mask_val(
  uint32_t *register_mask, uint32_t *register_val,
  isp_pipeline40_mod_en_type_t mod_en_type, isp_module_enable_info_t *bit_info,
  isp_hw_module_id_t module_id, uint32_t register_bit)
{
  if (!register_mask || !register_val || !bit_info ||
    (mod_en_type >= ISP_PIPELINE40_MOD_EN_MAX) ||
    (module_id >= ISP_MOD_MAX_NUM) || (register_bit > 31)) {
    ISP_ERR("failed: %p %p %p %d %d %d", register_mask, register_val, bit_info,
      mod_en_type, module_id, register_bit);
  } else {
    register_mask[mod_en_type] |= (1 << register_bit);
    if (bit_info->submod_enable[module_id]) {
      register_val[mod_en_type] |= 1 << register_bit;
    } else {
      register_val[mod_en_type] &= ~(1 << register_bit);
    }
  }
}

/** isp_pipeline40_create_module_cfg_mask:
 *
 *  @input_enable_bit_info: submod mask and submod enable
 *  @curr_submod_enable: the submodules that are currently enabled
 *  @register_mask: output register mask
 *  @register_val: output register val
 *
 *  Convert submod mask and enable from enum order to
 *  register order
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_pipeline40_create_module_cfg_mask(
  uint8_t *curr_submod_enable,
  isp_module_enable_info_t *input_enable_bit_info,
  uint32_t *register_mask, uint32_t *register_val)
{
  isp_hw_module_id_t           module_id = 0;

  if (!input_enable_bit_info || !register_mask || !register_val) {
    ISP_ERR("failed: %p %p %p", input_enable_bit_info, register_mask,
      register_val);
    return FALSE;
  }

  for (module_id = 0; module_id < ISP_MOD_MAX_NUM; module_id++) {
    if (input_enable_bit_info->submod_mask[module_id]) {
      switch (module_id) {
      case ISP_MOD_LINEARIZATION:
        isp_pipeline40_set_register_mask_val(register_mask, register_val,
          ISP_PIPELINE40_MODULE_CFG, input_enable_bit_info, module_id,
          MODULE_CFG_BLACK_EN);
        break;

      case ISP_MOD_ROLLOFF:
        isp_pipeline40_set_register_mask_val(register_mask, register_val,
          ISP_PIPELINE40_MODULE_CFG, input_enable_bit_info, module_id,
          MODULE_CFG_LENSROLLOFF_EN);
        break;

      case ISP_MOD_DEMUX:
        isp_pipeline40_set_register_mask_val(register_mask, register_val,
          ISP_PIPELINE40_MODULE_CFG, input_enable_bit_info, module_id,
          MODULE_CFG_DEMUX_EN);
        break;

      case ISP_MOD_BPC:
        isp_pipeline40_set_register_mask_val(register_mask,
          register_val, ISP_PIPELINE40_BPC_BCC_ABF, input_enable_bit_info,
          module_id, MODULE_CFG_DBPC_EN);
        break;

      case ISP_MOD_BCC:
        isp_pipeline40_set_register_mask_val(register_mask,
          register_val, ISP_PIPELINE40_BPC_BCC_ABF, input_enable_bit_info,
          module_id, MODULE_CFG_DBCC_EN);
        break;

      case ISP_MOD_ABF:
        isp_pipeline40_set_register_mask_val(register_mask, register_val,
          ISP_PIPELINE40_BPC_BCC_ABF, input_enable_bit_info, module_id,
          MODULE_CFG_ABF_EN);
        break;

      case ISP_MOD_SKIN_BHIST_STATS:
        /* Set bhist_sel bit in STATS_CFG register */
        isp_pipeline40_set_register_mask_val(register_mask, register_val,
          ISP_PIPELINE40_STATS_CFG, input_enable_bit_info, module_id,
          MODULE_CFG_BAYER_HIST_SEL);
        break;

      case ISP_MOD_DEMOSAIC:
        isp_pipeline40_set_register_mask_val(register_mask, register_val,
          ISP_PIPELINE40_MODULE_CFG, input_enable_bit_info, module_id,
          MODULE_CFG_DEMOSAIC_EN);
        break;

      case ISP_MOD_GAMMA:
        isp_pipeline40_set_register_mask_val(register_mask, register_val,
          ISP_PIPELINE40_MODULE_CFG, input_enable_bit_info, module_id,
          MODULE_CFG_RGB_LUT_EN);
        break;

      case ISP_MOD_WB:
        isp_pipeline40_set_register_mask_val(register_mask, register_val,
          ISP_PIPELINE40_MODULE_CFG, input_enable_bit_info, module_id,
          MODULE_CFG_WB_EN);
        break;

      case ISP_MOD_LA:
        isp_pipeline40_set_register_mask_val(register_mask, register_val,
          ISP_PIPELINE40_MODULE_CFG, input_enable_bit_info, module_id,
          MODULE_CFG_LA_EN);
        break;

      case ISP_MOD_CHROMA_ENHANCE:
        /* Set chroma_enhan_en bit in module_cfg register */
        isp_pipeline40_set_register_mask_val(register_mask, register_val,
          ISP_PIPELINE40_MODULE_CFG, input_enable_bit_info, module_id,
          MODULE_CFG_CHROMA_ENHAN_EN);
        /* Set color conv bit in STATS_CFG register */
        isp_pipeline40_set_register_mask_val(register_mask, register_val,
          ISP_PIPELINE40_STATS_CFG, input_enable_bit_info, module_id,
          MODULE_CFG_COLOR_CONV_EN);
        break;

      case ISP_MOD_COLOR_CORRECT:
        isp_pipeline40_set_register_mask_val(register_mask, register_val,
          ISP_PIPELINE40_MODULE_CFG, input_enable_bit_info, module_id,
          MODULE_CFG_COLOR_CORRECT_EN);
        break;

      case ISP_MOD_CLF:
        isp_pipeline40_set_register_mask_val(register_mask, register_val,
          ISP_PIPELINE40_MODULE_CFG, input_enable_bit_info, module_id,
          MODULE_CFG_CLF_EN);
        break;

      case ISP_MOD_CHROMA_SUPPRESS:
        /* enable mce_en if either CHROMA_SUPPRESS or MCE is enabled */
        if (curr_submod_enable[ISP_MOD_CHROMA_SUPPRESS] ||
            curr_submod_enable[ISP_MOD_MCE])
          isp_pipeline40_set_register_mask_val_default(register_mask,
            register_val, ISP_PIPELINE40_MODULE_CFG,
            MODULE_CFG_CHROMA_SUPPRESS_MCE_EN, 1);
        else
          isp_pipeline40_set_register_mask_val_default(register_mask,
            register_val, ISP_PIPELINE40_MODULE_CFG,
            MODULE_CFG_CHROMA_SUPPRESS_MCE_EN, 0);
        /* Enable / Disable mce bit in CHROMA_SUPPRESS_CFG_1 */
        isp_pipeline40_set_register_mask_val(register_mask, register_val,
          ISP_PIPELINE40_MCE_CS, input_enable_bit_info, module_id,
          MODULE_CFG_CS_EN);
        break;

      case ISP_MOD_MCE:
        /* enable mce_en if either CHROMA_SUPPRESS or MCE is enabled */
        if (curr_submod_enable[ISP_MOD_CHROMA_SUPPRESS] ||
            curr_submod_enable[ISP_MOD_MCE])
          isp_pipeline40_set_register_mask_val_default(register_mask,
            register_val, ISP_PIPELINE40_MODULE_CFG,
            MODULE_CFG_CHROMA_SUPPRESS_MCE_EN, 1);
        else
          isp_pipeline40_set_register_mask_val_default(register_mask,
            register_val, ISP_PIPELINE40_MODULE_CFG,
            MODULE_CFG_CHROMA_SUPPRESS_MCE_EN, 0);
        /* Enable / Disable mce bit in CHROMA_SUPPRESS_CFG_1 */
        isp_pipeline40_set_register_mask_val(register_mask, register_val,
          ISP_PIPELINE40_MCE_CS, input_enable_bit_info, module_id,
          MODULE_CFG_MCE_EN);
        break;

      case ISP_MOD_SCE:
        isp_pipeline40_set_register_mask_val(register_mask, register_val,
          ISP_PIPELINE40_MODULE_CFG, input_enable_bit_info, module_id,
          MODULE_CFG_SKIN_ENHAN_EN);
        break;

      case ISP_MOD_COLOR_XFORM_VIEWFINDER:
        isp_pipeline40_set_register_mask_val(register_mask, register_val,
          ISP_PIPELINE40_MODULE_CFG, input_enable_bit_info, module_id,
          MODULE_CFG_COLOR_XFORM_VIEW_EN);
        break;

      case ISP_MOD_SCALER_VIEWFINDER:
        isp_pipeline40_set_register_mask_val(register_mask, register_val,
          ISP_PIPELINE40_MODULE_CFG, input_enable_bit_info, module_id,
          MODULE_CFG_SCALE_VIEW_EN);
        break;

      case ISP_MOD_FOV_VIEWFINDER:
        isp_pipeline40_set_register_mask_val(register_mask, register_val,
          ISP_PIPELINE40_MODULE_CFG, input_enable_bit_info, module_id,
          MODULE_CFG_CROP_VIEW_EN);
        break;

      case ISP_MOD_COLOR_XFORM_ENCODER:
        isp_pipeline40_set_register_mask_val(register_mask, register_val,
          ISP_PIPELINE40_MODULE_CFG, input_enable_bit_info, module_id,
          MODULE_CFG_COLOR_XFORM_ENC_EN);
        break;

      case ISP_MOD_SCALER_ENCODER:
        isp_pipeline40_set_register_mask_val(register_mask, register_val,
          ISP_PIPELINE40_MODULE_CFG, input_enable_bit_info, module_id,
          MODULE_CFG_SCALE_ENC_EN);
        break;

      case ISP_MOD_FOV_ENCODER:
        isp_pipeline40_set_register_mask_val(register_mask, register_val,
          ISP_PIPELINE40_MODULE_CFG, input_enable_bit_info, module_id,
          MODULE_CFG_CROP_ENC_EN);
        break;

      case ISP_MOD_BE_STATS:
      case ISP_MOD_BG_STATS:
      case ISP_MOD_BF_STATS:
      case ISP_MOD_AWB_STATS:
      case ISP_MOD_RS_STATS:
      case ISP_MOD_CS_STATS:
      case ISP_MOD_IHIST_STATS:
        /* Enabled by iface2 */
        break;


      default:
        ISP_ERR("invalid module id %d", module_id);
        break;
      }
    }
  }

  return TRUE;
}

/** isp_pipeline40_update_module_cfg:
 *
 *  @fd: file descriptor
 *  @curr_submod_enable: submodules that are currently enabled
 *  @enable_bit_info: input enable bit info
 *
 *  Configure module_cfg register to enable submodules
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_pipeline40_update_module_cfg(int32_t fd,
  uint8_t *curr_submod_enable, isp_module_enable_info_t *enable_bit_info,
  cam_format_t sensor_fmt)
{
  boolean                      ret = TRUE;
  int32_t                      rc = 0;
  struct msm_vfe_cfg_cmd2      cfg_cmd;
  struct msm_vfe_reg_cfg_cmd   reg_cfg_cmd[ISP_PIPELINE40_MOD_EN_MAX];
  uint32_t                     register_mask[ISP_PIPELINE40_MOD_EN_MAX] = {0},
                               register_val[ISP_PIPELINE40_MOD_EN_MAX] = {0};
  uint32_t                     num_cfg = 0;
  isp_pipeline40_mod_en_type_t mod_en_type = 0;

  if ((fd <= 0) || !enable_bit_info) {
    ISP_ERR("failed: fd %d enable_bit_info %p sensor_fmt %d", fd,
      enable_bit_info, sensor_fmt);
    return FALSE;
  }

  ret = isp_pipeline40_create_module_cfg_mask(curr_submod_enable,
    enable_bit_info, register_mask, register_val);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_pipeline40_create_module_cfg_mask");
    return FALSE;
  }

  num_cfg = 0;
  for (mod_en_type = 0; mod_en_type < ISP_PIPELINE40_MOD_EN_MAX;
    mod_en_type++) {
    if (register_mask[mod_en_type]) {
      num_cfg++;
    }
  }

  if (!num_cfg) {
    /* No update to hw */
    return TRUE;
  }

  memset(reg_cfg_cmd, 0, (sizeof(struct msm_vfe_reg_cfg_cmd) *
    ISP_PIPELINE40_MOD_EN_MAX));

  memset(&cfg_cmd, 0, sizeof(struct msm_vfe_cfg_cmd2));

  cfg_cmd.cfg_cmd = (void *)&reg_cfg_cmd;
  cfg_cmd.num_cfg = num_cfg;

  num_cfg = 0;
  for (mod_en_type = 0; mod_en_type < ISP_PIPELINE40_MOD_EN_MAX;
    mod_en_type++) {
    if (register_mask[mod_en_type]) {
      ISP_DBG("MODULECFG type %d register_mask %x register_val %x", mod_en_type,
        register_mask[mod_en_type], register_val[mod_en_type]);
      switch (mod_en_type) {
      case ISP_PIPELINE40_MODULE_CFG:
        reg_cfg_cmd[num_cfg].u.mask_info.reg_offset = MODULE_CFG_REGISTER_ADDR;
        break;

      case ISP_PIPELINE40_MCE_CS:
        reg_cfg_cmd[num_cfg].u.mask_info.reg_offset =
          CHROMA_SUPPRESS_CFG_1_ADDR;
        break;

      case ISP_PIPELINE40_STATS_CFG:
        reg_cfg_cmd[num_cfg].u.mask_info.reg_offset = STATS_CFG_ADDR;
        break;

      case ISP_PIPELINE40_BPC_BCC_ABF:
        reg_cfg_cmd[num_cfg].u.mask_info.reg_offset = DEMOSAIC_CFG_ADDR;
        break;

      default:
        ISP_ERR("failed: invalid mod_en_type %d", mod_en_type);
        break;
      }
      reg_cfg_cmd[num_cfg].cmd_type = VFE_CFG_MASK;
      reg_cfg_cmd[num_cfg].u.mask_info.mask = register_mask[mod_en_type];
      reg_cfg_cmd[num_cfg].u.mask_info.val = register_val[mod_en_type];
      num_cfg++;
    }
  }

  /* Perform ioctl */
  rc = ioctl(fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
  if (rc < 0) {
    ISP_ERR("failed: rc %d", rc);
    ret = FALSE;
  }

  return ret;
}
/** isp_pipeline40_open:
 *
 *  @isp_version: ISP version
 *  @isp_id: ISP hw id
 *
 *  Initialize ISP pipeline resources based on isp_version
 *  and isp_id
 *
 *  Return TRUE on success and FALSE on failure
 **/
isp_pipeline_t *isp_pipeline40_open(uint32_t isp_version, uint32_t isp_id)
{
  isp_pipeline_t *curr_pipeline;

  if (isp_id >= ISP_HW_MAX) {
    ISP_ERR("failed: invalid isp id %d", isp_id);
    return NULL;
  }

  curr_pipeline = (isp_pipeline_t *)CAM_MALLOC(sizeof(*curr_pipeline));
  if (!curr_pipeline) {
    ISP_ERR("failed");
    return NULL;
  }
  memcpy(curr_pipeline, &isp_pipeline, sizeof(*curr_pipeline));

  if (isp_version >= ISP_MSM8226_V1) {
    /* 8x26 series */
    curr_pipeline->isp_version = SET_ISP_VERSION(ISP_VERSION_40,
      ISP_REVISION_V2);
    curr_pipeline->num_pix = 1;
    curr_pipeline->num_rdi = 3;
    curr_pipeline->num_wms = 7;
    curr_pipeline->num_register = ISP40_NUM_REG_DUMP;
    curr_pipeline->isp_id = isp_id;
    curr_pipeline->is_camif_crop_supported = FALSE;
    curr_pipeline->is_camif_raw_op_fmt_supported = FALSE;
    curr_pipeline->is_mono_stats_supported = 0;
    curr_pipeline->max_width = 4288;
    curr_pipeline->max_height = 3216;
    /* TODO check this */
    curr_pipeline->hw_stream_info[ISP_HW_STREAM_ENCODER].max_width = 4288;
    curr_pipeline->hw_stream_info[ISP_HW_STREAM_ENCODER].max_height = 3216;
    curr_pipeline->hw_stream_info[ISP_HW_STREAM_VIEWFINDER].max_width = 2048;
    curr_pipeline->hw_stream_info[ISP_HW_STREAM_VIEWFINDER].max_height = 1600;

    curr_pipeline->hw_stream_info[ISP_HW_STREAM_IDEAL_RAW].max_width =
      curr_pipeline->hw_stream_info[ISP_HW_STREAM_ENCODER].max_width;
    curr_pipeline->hw_stream_info[ISP_HW_STREAM_IDEAL_RAW].max_height =
      curr_pipeline->hw_stream_info[ISP_HW_STREAM_ENCODER].max_height;

    curr_pipeline->max_pix_clk_rate[ISP_CX_LEVEL_NOMINAL] = ISP_PIX40_CLK_USER_MAX;
    curr_pipeline->max_pix_clk_rate[ISP_CX_LEVEL_TURBO] = ISP_PIX_CLK_320MHZ;

  } else if (isp_version >= ISP_MSM8974_V1) {
    /* 8974 series */
    if (isp_version == ISP_MSM8974_V1) {
      curr_pipeline->isp_version = SET_ISP_VERSION(ISP_VERSION_40,
        ISP_REVISION_V1);
    } else if (isp_version == ISP_MSM8974_V2) {
      curr_pipeline->isp_version = SET_ISP_VERSION(ISP_VERSION_40,
        ISP_REVISION_V2);
    } else if (isp_version == ISP_MSM8974_V3) {
      curr_pipeline->isp_version = SET_ISP_VERSION(ISP_VERSION_40,
        ISP_REVISION_V3);
    } else if (isp_version == ISP_MSM8916_V1) {
      curr_pipeline->isp_version = SET_ISP_VERSION(ISP_VERSION_40,
        ISP_REVISION_V1);
    } else if (isp_version == ISP_MSM8939){
      curr_pipeline->isp_version = SET_ISP_VERSION(ISP_VERSION_40,
        ISP_REVISION_V1);
    } else {
      ISP_ERR("failed: invalid isp version %x", isp_version);
    }

    curr_pipeline->num_pix = 1;
    curr_pipeline->num_rdi = 3;
    curr_pipeline->num_wms = 7;
    curr_pipeline->num_register = ISP40_NUM_REG_DUMP;
    curr_pipeline->isp_id = isp_id;
    curr_pipeline->is_camif_crop_supported = FALSE;
    curr_pipeline->is_camif_raw_op_fmt_supported = FALSE;
    curr_pipeline->is_mono_stats_supported = 0;

    if (isp_id == ISP_HW_0) {
      curr_pipeline->max_width = 5376;
      curr_pipeline->max_height = 4032;
      curr_pipeline->hw_stream_info[ISP_HW_STREAM_ENCODER].max_width = 5376;
      curr_pipeline->hw_stream_info[ISP_HW_STREAM_ENCODER].max_height = 4032;
      curr_pipeline->hw_stream_info[ISP_HW_STREAM_VIEWFINDER].max_width = 2560;
      curr_pipeline->hw_stream_info[ISP_HW_STREAM_VIEWFINDER].max_height = 2048;
    } else if (isp_id == ISP_HW_1) {
      curr_pipeline->max_width = 4288;
      curr_pipeline->max_height = 3216;
      curr_pipeline->hw_stream_info[ISP_HW_STREAM_ENCODER].max_width = 4288;
      curr_pipeline->hw_stream_info[ISP_HW_STREAM_ENCODER].max_height = 3216;
      curr_pipeline->hw_stream_info[ISP_HW_STREAM_VIEWFINDER].max_width = 2048;
      curr_pipeline->hw_stream_info[ISP_HW_STREAM_VIEWFINDER].max_height = 1080;
    }

    curr_pipeline->max_pix_clk_rate[ISP_CX_LEVEL_NOMINAL] = ISP_PIX40_CLK_USER_MAX;
    curr_pipeline->max_pix_clk_rate[ISP_CX_LEVEL_TURBO] = ISP_PIX_CLK_320MHZ;
  } else {
    ISP_ERR("failed: invalid isp version %x", isp_version);
    free(curr_pipeline);
    return NULL;
  }

  curr_pipeline->num_stats_comp_grp = 2;
  curr_pipeline->max_scale_ratio = 16;
  curr_pipeline->num_hw_streams = ARRAY_SIZE(stream_hw_params);
  curr_pipeline->hw_stream_ids[ISP_HW_STREAM_ENCODER] =
    ISP_HW_STREAM_ENCODER;
  curr_pipeline->hw_stream_ids[ISP_HW_STREAM_VIEWFINDER] =
    ISP_HW_STREAM_VIEWFINDER;
  curr_pipeline->pipeline_applying_delay = ISP_MAX_APPLYING_DELAY;

  return curr_pipeline;
}
