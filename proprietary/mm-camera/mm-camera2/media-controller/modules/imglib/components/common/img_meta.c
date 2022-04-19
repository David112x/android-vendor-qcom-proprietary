/**********************************************************************
*  Copyright (c) 2014-2017, 2019 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "img_meta.h"

#include <cutils/properties.h>
#include <stdlib.h>

/** IMG_DUMP:
 *    @...: dump contents
 *
 * Dumps metadata
 **/
#define IMG_DUMP(...) if (fprintf(__VA_ARGS__) < 0) {\
  IDBG_ERROR("%s:%d failed: Cannot dump to file\n", __func__, __LINE__);\
  return IMG_ERR_GENERAL;}

/** IMG_DUMP_DEC:
 *    @f: dump file
 *    @val: value to be dumped
 *
 * Dumps decimal
 **/
#define IMG_DUMP_DEC(f, val) IMG_DUMP(f, "%s value is %d\n", #val, val)

/** IMG_DUMP_DEC_AT_INDEX:
 *    @f: dump file
 *    @val: value to be dumped
 *    @i: array index
 *
 * Dumps decimal in array
 **/
#define IMG_DUMP_DEC_AT_INDEX(f, val, i) \
  IMG_DUMP(f,"%s[%d] value is %d\n", #val, i, val[i])

/** IMG_DUMP_LONG:
 *    @f: dump file
 *    @val: value to be dumped
 *
 * Dumps long
 **/
#define IMG_DUMP_LONG(f, val) IMG_DUMP(f, "%s value is %ld\n", #val, val)

/** IMG_DUMP_LONGLONG:
 *    @f: dump file
 *    @val: value to be dumped
 *
 * Dumps long
 **/
#define IMG_DUMP_LONGLONG(f, val) IMG_DUMP(f, "%s value is %lld\n", #val, val)

/** IMG_DUMP_DEC_AT_INDEX:
 *    @f: dump file
 *    @val: value to be dumped
 *    @i: array index
 *
 * Dumps long in array
 **/
#define IMG_DUMP_LONG_AT_INDEX(f, val, i) \
  IMG_DUMP(f,"%s[%d] value is %ld\n", #val, i, val[i])

/** IMG_DUMP_HEX:
 *    @f: dump file
 *    @val: value to be dumped
 *
 * Dumps hex
 **/
#define IMG_DUMP_HEX(f, val) IMG_DUMP(f, "%s value is 0x%x\n", #val, val)

/** IMG_DUMP_HEX_AT_INDEX:
 *    @f: dump file
 *    @val: value to be dumped
 *    @i: array index
 *
 * Dumps hex in array
 **/
#define IMG_DUMP_HEX_AT_INDEX(f, val, i) \
  IMG_DUMP(f,"%s[%d] value is 0x%x\n", #val, i, val[i])

/** IMG_DUMP_STRING:
 *    @f: dump file
 *    @val: value to be dumped
 *
 * Dumps string
 **/
#define IMG_DUMP_STRING(f, val) IMG_DUMP(f, "%s value is %s\n", #val, val)

/** IMG_DUMP_STRING_AT_INDEX:
 *    @f: dump file
 *    @val: value to be dumped
 *    @i: array index
 *
 * Dumps string in array
 **/
#define IMG_DUMP_STRING_AT_INDEX(f, val, i) \
  IMG_DUMP(f,"%s[%d] value is %s\n", #val, i, val[i])

/** IMG_DUMP_PTR:
 *    @f: dump file
 *    @val: value to be dumped
 *
 * Dumps pointer
 **/
#define IMG_DUMP_PTR(f, val) IMG_DUMP(f, "%s value is %p\n", #val, val)

/** IMG_DUMP_PTR_AT_INDEX:
 *    @f: dump file
 *    @val: value to be dumped
 *    @i: array index
 *
 * Dumps ptr in array
 **/
#define IMG_DUMP_PTR_AT_INDEX(f, val, i) \
  IMG_DUMP(f,"%s[%d] value is %p\n", #val, i, val[i])


/** IMG_DUMP_FLOAT:
 *    @f: dump file
 *    @val: value to be dumped
 *
 * Dumps float
 **/
#define IMG_DUMP_FLOAT(f, val) IMG_DUMP(f, "%s value is %f\n", #val, val)

/** IMG_DUMP_FLOAT_AT_INDEX:
 *    @f: dump file
 *    @val: value to be dumped
 *    @i: array index
 *
 * Dumps ptr in array
 **/
#define IMG_DUMP_FLOAT_AT_INDEX(f, val, i) \
  IMG_DUMP(f,"%s[%d] value is %f\n", #val, i, val[i])

static int32_t img_dump_meta_gamma(img_meta_t *p_meta, FILE* fd);
static int32_t img_dump_meta_aec_info(img_meta_t *p_meta, FILE* fd);
static int32_t img_dump_meta_awb_info(img_meta_t *p_meta, FILE* fd);
static int32_t img_dump_meta_af_info(img_meta_t *p_meta, FILE* fd);
static int32_t img_dump_meta_output_roi(img_meta_t *p_meta, FILE* fd);
static int32_t img_dump_meta_zoom_factor(img_meta_t *p_meta, FILE* fd);
static int32_t img_dump_meta_no_rot_fd_info(img_meta_t *p_meta, FILE* fd);
static int32_t img_dump_meta_fd_info(img_meta_t *p_meta, FILE* fd);
static int32_t img_dump_meta_rotation(img_meta_t *p_meta, FILE* fd);
static int32_t img_dump_meta_flip(img_meta_t *p_meta, FILE* fd);
static int32_t img_dump_meta_misc_data(img_meta_t *p_meta, FILE* fd);
static int32_t img_dump_meta_chromaflash_ctrl(img_meta_t *p_meta, FILE* fd);
static int32_t img_dump_meta_num_input(img_meta_t *p_meta, FILE* fd);
static int32_t img_dump_meta_tp_config(img_meta_t *p_meta, FILE* fd);
static int32_t img_dump_meta_dcrf_runtime_param(img_meta_t *p_meta, FILE* fd);
static int32_t img_dump_meta_dcrf_result(img_meta_t *p_meta, FILE* fd);
static int32_t img_dump_meta_frame_operations(img_meta_t *p_meta, FILE* fd);
static int32_t img_dump_meta_seemore_cfg(img_meta_t *p_meta, FILE* fd);
static int32_t img_dump_meta_stillmore_cfg(img_meta_t *p_meta, FILE* fd);
static int32_t img_dump_meta_paaf_param(img_meta_t *p_meta, FILE* fd);
static int32_t img_dump_meta_sat_param(img_meta_t *p_meta, FILE* fd);
static int32_t img_dump_meta_sac_param(img_meta_t *p_meta, FILE* fd);
static int32_t img_dump_meta_rtb_param(img_meta_t *p_meta, FILE* fd);
static int32_t img_dump_meta_rtbdm_param(img_meta_t *p_meta, FILE* fd);

/** img_dump_meta_func
 *    @p_meta: meta handler
 *    @fd: dump file fd
 *
 * Dumps data from metadata
 *
 * Returns standard image lib codes
 **/
typedef int32_t (*img_dump_meta_func)(img_meta_t *p_meta, FILE* fd);

static img_dump_meta_func dump_handlers[] = {
  img_dump_meta_gamma,
  img_dump_meta_aec_info,
  img_dump_meta_awb_info,
  img_dump_meta_af_info,
  img_dump_meta_output_roi,
  img_dump_meta_zoom_factor,
  img_dump_meta_no_rot_fd_info,
  img_dump_meta_fd_info,
  img_dump_meta_rotation,
  img_dump_meta_flip,
  img_dump_meta_misc_data,
  img_dump_meta_chromaflash_ctrl,
  img_dump_meta_num_input,
  img_dump_meta_tp_config,
  img_dump_meta_dcrf_runtime_param,
  img_dump_meta_dcrf_result,
  img_dump_meta_frame_operations,
  img_dump_meta_seemore_cfg,
  img_dump_meta_stillmore_cfg,
  img_dump_meta_paaf_param,
  img_dump_meta_sat_param,
  img_dump_meta_sac_param,
  img_dump_meta_rtb_param,
  img_dump_meta_rtbdm_param,
};

/**
 * Function: img_get_meta
 *
 * Description: This macro is to get the meta value from
 *               metadata if present
 *
 * Arguments:
 *   @p_meta_data : meta buffer
 *   @meta_type: meta type
 *
 * Return values:
 *     meta buffer value pointer
 *
 * Notes: none
 **/
void *img_get_meta(void *p_meta_data, uint32_t meta_type)
{
  img_meta_t *p_meta = (img_meta_t *)p_meta_data;
  img_meta_type_t type = (img_meta_type_t) meta_type;
  void *val = NULL;

  if (!p_meta) {
    IDBG_ERROR("%s:%d] Null pointer detected", __func__, __LINE__);
    return val;
  }

  if ((type < IMG_META_MAX) && p_meta->valid[type]) {
    switch (type) {
    case IMG_META_R_GAMMA:
      val = (void *)&p_meta->gamma_R;
      break;
    case IMG_META_G_GAMMA:
      val = (void *)&p_meta->gamma_G;
      break;
    case IMG_META_B_GAMMA:
      val = (void *)&p_meta->gamma_B;
      break;
    case IMG_META_AEC_INFO:
      val = (void *)&p_meta->aec_info;
      break;
    case IMG_META_AWB_INFO:
      val = (void *)&p_meta->awb_info;
      break;
    case IMG_META_AF_INFO:
      val = (void *)&p_meta->af_info;
      break;
    case IMG_META_OUTPUT_ROI:
      val = (void *)&p_meta->output_crop;
      break;
    case IMG_META_ZOOM_FACTOR:
      val = (void *)&p_meta->zoom_factor;
      break;
    case IMG_META_NO_ROT_FD_INFO:
      val = (void *)&p_meta->no_rot_fd_info;
      break;
    case IMG_META_FD_INFO:
      val = (void *)&p_meta->fd_info;
      break;
    case IMG_META_ROTATION:
      val = (void *)&p_meta->rotation;
      break;
    case IMG_META_FLIP:
      val = (void *)&p_meta->snapshot_flip;
      break;
    case IMG_META_MISC_DATA:
      val = (void *)&p_meta->misc_data;
      break;
    case IMG_META_CHROMAFLASH_CTRL:
      val = (void *)&p_meta->cf_ctrl;
      break;
    case IMG_META_NUM_INPUT:
      val = (void *)&p_meta->num_input;
      break;
    case IMG_META_TP_CONFIG:
      val = (void *)&p_meta->tp_config;
      break;
    case IMG_META_DCRF_RUNTIME_PARAM:
      val = (void *)&p_meta->dcrf_runtime_params;
      break;
    case IMG_META_DCRF_RESULT:
      val = (void *)&p_meta->dcrf_result;
      break;
    case IMG_META_SW2D_OPS:
      val = (void *)&p_meta->sw2d_operation;
      break;
    case IMG_META_OPAQUE_DATA:
      val = (void *)&p_meta->opaque_data;
      break;
    case IMG_META_SEEMORE_CFG:
      val = (void *)&p_meta->seemore_cfg;
      break;
    case IMG_META_STILLMORE_CFG:
      val = (void *)&p_meta->stillmore_cfg;
      break;
    case IMG_META_PAAF_CFG:
      val = (void *)&p_meta->paaf_cfg;
      break;
    case IMG_META_PAAF_RESULT:
      val = (void *)&p_meta->paaf_result;
      break;
    case IMG_META_SPATIAL_TRANSFORM_CFG:
      val = (void *)&p_meta->qdcc_cfg.sat_cfg;
      break;
    case IMG_META_SPATIAL_TRANSFORM_RESULT:
      val = (void *)&p_meta->qdcc_result.sat_result;
      break;
    case IMG_META_SPATIAL_ALIGN_COMPUTE_CFG:
      val = (void *)&p_meta->qdcc_cfg.sac_cfg;
      break;
    case IMG_META_SPATIAL_ALIGN_COMPUTE_RESULT:
      val = (void *)&p_meta->qdcc_result.sac_result;
      break;
    case IMG_META_BOKEH_CFG:
      val = (void *)&p_meta->qdcc_cfg.rtb_cfg;
      break;
    case IMG_META_BOKEH_RESULT:
      val = (void *)&p_meta->qdcc_result.rtb_result;
      break;
    case IMG_META_BOKEH_DEPTH_MAP_CFG:
      val = (void *)&p_meta->qdcc_cfg.rtbdm_cfg;
      break;
    case IMG_META_BOKEH_DEPTH_MAP_RESULT:
      val = (void *)&p_meta->qdcc_result.rtbdm_result;
      break;
    case IMG_META_FOV_CONFIG:
      val = (void *)&p_meta->fov_cfg;
      break;
    case IMG_META_QDC_CROP:
      val = (void *)&p_meta->bokeh_crop;
      break;
    default:
      IDBG_ERROR("Not supported meta type %d", type);
    }
  }
  return val;
}

/**
 * Function: img_set_meta
 *
 * Description: This macro is to set the meta value in metadata
 *
 * Arguments:
 *   @p_meta_data : meta buffer
 *   @meta_type: meta type
 *   @val: pointer to new value
 *
 * Returns standard image lib codes
 *
 * Notes: none
 **/
int32_t img_set_meta(void *p_meta_data, uint32_t meta_type, void* val)
{
  img_meta_t *p_meta = (img_meta_t *)p_meta_data;
  img_meta_type_t type = (img_meta_type_t) meta_type;

  if (!p_meta || !val) {
    IDBG_ERROR("%s:%d] Null pointer detected", __func__, __LINE__);
    IDBG_ERROR("%s:%d] p_meta %p val %p", __func__, __LINE__, p_meta, val);
    return IMG_ERR_GENERAL;
  }

  switch (type) {
  case IMG_META_R_GAMMA:
    memcpy(&p_meta->gamma_R, val, sizeof(p_meta->gamma_R));
    break;
  case IMG_META_G_GAMMA:
    memcpy(&p_meta->gamma_G, val, sizeof(p_meta->gamma_G));
    break;
  case IMG_META_B_GAMMA:
    memcpy(&p_meta->gamma_B, val, sizeof(p_meta->gamma_B));
    break;
  case IMG_META_AEC_INFO:
    memcpy(&p_meta->aec_info, val, sizeof(p_meta->aec_info));
    break;
  case IMG_META_AWB_INFO:
    memcpy(&p_meta->awb_info, val, sizeof(p_meta->awb_info));
    break;
  case IMG_META_AF_INFO:
    memcpy(&p_meta->af_info, val, sizeof(p_meta->af_info));
    break;
  case IMG_META_OUTPUT_ROI:
    memcpy(&p_meta->output_crop, val, sizeof(p_meta->output_crop));
    break;
  case IMG_META_QDC_CROP:
    memcpy(&p_meta->bokeh_crop, val, sizeof(p_meta->bokeh_crop));
    break;
  case IMG_META_BOKEH_CROP:
    memcpy(&p_meta->l_stream_crop, val, sizeof(p_meta->l_stream_crop));
    break;
  case IMG_META_ZOOM_FACTOR:
    memcpy(&p_meta->zoom_factor, val, sizeof(p_meta->zoom_factor));
    break;
  case IMG_META_USER_ZOOM_FACTOR:
    memcpy(&p_meta->user_zoom_factor, val, sizeof(p_meta->user_zoom_factor));
    break;
  case IMG_META_NO_ROT_FD_INFO:
    memcpy(&p_meta->no_rot_fd_info, val, sizeof(p_meta->no_rot_fd_info));
    break;
  case IMG_META_FD_INFO:
    memcpy(&p_meta->fd_info, val, sizeof(p_meta->fd_info));
    break;
  case IMG_META_ROTATION:
    memcpy(&p_meta->rotation, val, sizeof(p_meta->rotation));
    break;
  case IMG_META_FLIP:
    memcpy(&p_meta->snapshot_flip, val, sizeof(p_meta->snapshot_flip));
    break;
  case IMG_META_MISC_DATA:
    memcpy(&p_meta->misc_data, val, sizeof(p_meta->misc_data));
    break;
  case IMG_META_CHROMAFLASH_CTRL:
    memcpy(&p_meta->cf_ctrl, val, sizeof(p_meta->cf_ctrl));
    break;
  case IMG_META_NUM_INPUT:
    memcpy(&p_meta->num_input, val, sizeof(p_meta->num_input));
    break;
  case IMG_META_TP_CONFIG:
    memcpy(&p_meta->tp_config, val, sizeof(p_meta->tp_config));
    break;
  case IMG_META_DCRF_RUNTIME_PARAM:
    memcpy(&p_meta->dcrf_runtime_params, val,
      sizeof(p_meta->dcrf_runtime_params));
    break;
  case IMG_META_DCRF_RESULT:
    memcpy(&p_meta->dcrf_result, val, sizeof(p_meta->dcrf_result));
    break;
  case IMG_META_SW2D_OPS:
    memcpy(&p_meta->sw2d_operation, val, sizeof(p_meta->sw2d_operation));
    break;
  case IMG_META_OPAQUE_DATA: {
    img_opaque_data_set_t *p_opaque_data_set = (img_opaque_data_set_t *)val;
    if (p_opaque_data_set->data_size > sizeof(p_meta->opaque_data)) {
      IDBG_ERROR("Size out of bounds %d %d", p_opaque_data_set->data_size,
        sizeof(p_meta->opaque_data));
      return IMG_ERR_INVALID_INPUT;
    }
    memcpy(&p_meta->opaque_data, p_opaque_data_set->p_data,
      p_opaque_data_set->data_size);
    break;
  }
  case IMG_META_SEEMORE_CFG:
    memcpy(&p_meta->seemore_cfg, val, sizeof(p_meta->seemore_cfg));
    break;
  case IMG_META_STILLMORE_CFG:
    memcpy(&p_meta->stillmore_cfg, val, sizeof(p_meta->stillmore_cfg));
    break;
  case IMG_META_PAAF_CFG:
    memcpy(&p_meta->paaf_cfg, val, sizeof(p_meta->paaf_cfg));
    break;
  case IMG_META_PAAF_RESULT:
    memcpy(&p_meta->paaf_result, val, sizeof(p_meta->paaf_result));
    break;
  case IMG_META_SPATIAL_TRANSFORM_CFG:
    memcpy(&p_meta->qdcc_cfg.sat_cfg, val,
    sizeof(p_meta->qdcc_cfg.sat_cfg));
    break;
  case IMG_META_SPATIAL_TRANSFORM_RESULT:
    memcpy(&p_meta->qdcc_result.sat_result, val,
    sizeof(p_meta->qdcc_result.sat_result));
    break;
  case IMG_META_SPATIAL_ALIGN_COMPUTE_CFG:
    memcpy(&p_meta->qdcc_cfg.sac_cfg, val,
    sizeof(p_meta->qdcc_cfg.sac_cfg));
    break;
  case IMG_META_SPATIAL_ALIGN_COMPUTE_RESULT:
    memcpy(&p_meta->qdcc_result.sac_result, val,
    sizeof(p_meta->qdcc_result.sac_result));
    break;
  case IMG_META_BOKEH_CFG:
    memcpy(&p_meta->qdcc_cfg.rtb_cfg, val,
    sizeof(p_meta->qdcc_cfg.rtb_cfg));
    break;
  case IMG_META_BOKEH_RESULT:
    memcpy(&p_meta->qdcc_result.rtb_result, val,
    sizeof(p_meta->qdcc_result.rtb_result));
    break;
  case IMG_META_BOKEH_DEPTH_MAP_CFG:
    memcpy(&p_meta->qdcc_cfg.rtbdm_cfg, val,
    sizeof(p_meta->qdcc_cfg.rtbdm_cfg));
    break;
  case IMG_META_BOKEH_DEPTH_MAP_RESULT:
    memcpy(&p_meta->qdcc_result.rtbdm_result, val,
    sizeof(p_meta->qdcc_result.rtbdm_result));
    break;
  case IMG_META_FOV_CONFIG:
    memcpy(&p_meta->fov_cfg, val, sizeof(p_meta->fov_cfg));
    break;

  case IMG_META_GYRO:
    memcpy(&p_meta->thirdparty_eis_cfg.is_gyro_stats, val,
    sizeof(img_gyro_stats_t));
    break;

  case IMG_META_ACCEL:
    memcpy(&p_meta->thirdparty_eis_cfg.is_accel_stats, val,
    sizeof(img_accel_stats_t));
    break;

  default:
    IDBG_ERROR("Not supported meta type %d", type);
    return IMG_ERR_GENERAL;
  }
  p_meta->valid[type] = TRUE;

  return IMG_SUCCESS;
}

/** img_dump_meta_gamma
 *    @p_meta: meta handler
 *    @fd: dump file fd
 *
 * Dumps gamma tables from metadata
 *
 * Returns standard image lib codes
 **/
static int32_t img_dump_meta_gamma(img_meta_t *p_meta, FILE* fd)
{
  uint32_t i;
  uint16_t* gamma_R = img_get_meta(p_meta, IMG_META_R_GAMMA);
  uint16_t* gamma_G = img_get_meta(p_meta, IMG_META_G_GAMMA);
  uint16_t* gamma_B = img_get_meta(p_meta, IMG_META_B_GAMMA);

  if (gamma_R) {
    IMG_DUMP(fd, "\nRed Gamma:\n");
    for (i=0; i<GAMMA_TABLE_ENTRIES; i++) {
      IMG_DUMP_HEX_AT_INDEX(fd, gamma_R, i);
    }
  }

  if (gamma_G) {
    IMG_DUMP(fd, "\nGreen Gamma:\n");
    for (i=0; i<GAMMA_TABLE_ENTRIES; i++) {
      IMG_DUMP_HEX_AT_INDEX(fd, gamma_G, i);
    }
  }

  if (gamma_B) {
    IMG_DUMP(fd, "\nBlue Gamma:\n");
    for (i=0; i<GAMMA_TABLE_ENTRIES; i++) {
      IMG_DUMP_HEX_AT_INDEX(fd, gamma_B, i);
    }
  }

  return IMG_SUCCESS;
}

/** img_dump_meta_aec_info
 *    @p_meta: meta handler
 *    @fd: dump file fd
 *
 * Dumps aec info from metadata
 *
 * Returns standard image lib codes
 **/
static int32_t img_dump_meta_aec_info(img_meta_t *p_meta, FILE* fd)
{
  img_aec_info_t *aec_info = img_get_meta(p_meta, IMG_META_AEC_INFO);
  float real_gain;
  uint32_t linecount;
  float exp_time;
  float lux_idx;
  uint32_t iso;
  img_stats_status_t aec_status;

  if (aec_info) {
    IMG_DUMP(fd, "\nAEC Info:\n");

    real_gain = aec_info->real_gain;
    linecount = aec_info->linecount;
    exp_time = aec_info->exp_time;
    lux_idx = aec_info->lux_idx;
    iso = aec_info->iso;
    aec_status = aec_info->aec_status;

    IMG_DUMP_FLOAT(fd, real_gain);
    IMG_DUMP_DEC(fd, linecount);
    IMG_DUMP_FLOAT(fd, exp_time);
    IMG_DUMP_FLOAT(fd, lux_idx);
    IMG_DUMP_DEC(fd, iso);
    IMG_DUMP_DEC(fd, aec_status);
  }

  return IMG_SUCCESS;
}

/** img_dump_meta_awb_info
 *    @p_meta: meta handler
 *    @fd: dump file fd
 *
 * Dumps awb info from metadata
 *
 * Returns standard image lib codes
 **/
static int32_t img_dump_meta_awb_info(img_meta_t *p_meta, FILE* fd)
{
  img_awb_info_t *awb_info = img_get_meta(p_meta, IMG_META_AWB_INFO);
  float r_gain;
  float g_gain;
  float b_gain;
  uint32_t color_temp;
  float *ccm;
  int i;
  img_stats_status_t awb_status;

  if (awb_info) {
    IMG_DUMP(fd, "\nAWB Info:\n");

    r_gain = awb_info->r_gain;
    g_gain = awb_info->g_gain;
    b_gain = awb_info->b_gain;
    color_temp = awb_info->color_temp;
    awb_status = awb_info->awb_status;

    IMG_DUMP_FLOAT(fd, r_gain);
    IMG_DUMP_FLOAT(fd, g_gain);
    IMG_DUMP_FLOAT(fd, b_gain);
    IMG_DUMP_DEC(fd, color_temp);
    ccm = &awb_info->ccm[0][0];
    for (i = 0; i < awb_info->ccm_size; i++) {
      IMG_DUMP_FLOAT_AT_INDEX(fd, ccm, i);
    }
    IMG_DUMP_DEC(fd, awb_status);
  }

  return IMG_SUCCESS;
}

/** img_dump_meta_af_info
 *    @p_meta: meta handler
 *    @fd: dump file fd
 *
 * Dumps af info from metadata
 *
 * Returns standard image lib codes
 **/
static int32_t img_dump_meta_af_info(img_meta_t *p_meta, FILE* fd)
{
  img_af_info_t *af_info = img_get_meta(p_meta, IMG_META_AF_INFO);
  img_rect_t *af_roi;

  if (af_info) {
    IMG_DUMP(fd, "\nAF Info:\n");
    IMG_DUMP_DEC(fd, af_info->af_status);
    IMG_DUMP_FLOAT(fd, af_info->object_distance);
    IMG_DUMP_FLOAT(fd, af_info->far_field);
    IMG_DUMP_FLOAT(fd, af_info->near_field);
    IMG_DUMP_FLOAT(fd, af_info->lens_shift);
    IMG_DUMP_FLOAT(fd, af_info->focal_length_ratio);

    af_roi = &af_info->focused_roi;
    IMG_DUMP(fd, "\nAF Info:\n");
    IMG_DUMP_DEC(fd, af_roi->pos.x);
    IMG_DUMP_DEC(fd, af_roi->pos.y);
    IMG_DUMP_DEC(fd, af_roi->size.width);
    IMG_DUMP_DEC(fd, af_roi->size.height);
  } else {
    IDBG_ERROR("AF Info is NULL");
  }
  return IMG_SUCCESS;
}

/** img_dump_meta_output_roi
 *    @p_meta: meta handler
 *    @fd: dump file fd
 *
 * Dumps output crop info from metadata
 *
 * Returns standard image lib codes
 **/
static int32_t img_dump_meta_output_roi(img_meta_t *p_meta, FILE* fd)
{
  img_rect_t *output_crop = img_get_meta(p_meta, IMG_META_OUTPUT_ROI);
  int32_t x, y, width, height;

  if (output_crop) {
    IMG_DUMP(fd, "\nOutput crop:\n");

    x = output_crop->pos.x;
    y = output_crop->pos.y;
    width = output_crop->size.width;
    height = output_crop->size.height;

    IMG_DUMP_DEC(fd, x);
    IMG_DUMP_DEC(fd, y);
    IMG_DUMP_DEC(fd, width);
    IMG_DUMP_DEC(fd, height);
  }

  return IMG_SUCCESS;
}

/** img_dump_meta_zoom_factor
 *    @p_meta: meta handler
 *    @fd: dump file fd
 *
 * Dumps zoom factor info from metadata
 *
 * Returns standard image lib codes
 **/
static int32_t img_dump_meta_zoom_factor(img_meta_t *p_meta, FILE* fd)
{
  float* p_zoom_factor = img_get_meta(p_meta, IMG_META_ZOOM_FACTOR);
  float zoom_factor;

  if (p_zoom_factor) {
    IMG_DUMP(fd, "\nZoom Factor:\n");
    zoom_factor = *p_zoom_factor;
    IMG_DUMP_FLOAT(fd, zoom_factor);
  }

  return IMG_SUCCESS;
}

/** img_dump_meta_no_rot_fd_info
 *    @p_meta: meta handler
 *    @fd: dump file fd
 *
 * Dumps faces detect info before rotation from metadata
 *
 * Returns standard image lib codes
 **/
static int32_t img_dump_meta_no_rot_fd_info(img_meta_t *p_meta, FILE* fd)
{
  img_fd_info_t *fd_info = img_get_meta(p_meta, IMG_META_NO_ROT_FD_INFO);
  uint32_t valid_faces_detected;
  uint32_t fd_frame_width;
  uint32_t fd_frame_height;
  int32_t *faceRollDir;
  uint32_t *faceROIx;
  uint32_t *faceROIy;
  uint32_t *faceROIWidth;
  uint32_t *faceROIHeight;
  uint32_t i;

  if (fd_info) {
    IMG_DUMP(fd, "\nBefore Rotation Face Detect Info:\n");

    valid_faces_detected = fd_info->valid_faces_detected;
    fd_frame_width = fd_info->fd_frame_width;
    fd_frame_height = fd_info->fd_frame_height;
    faceRollDir = fd_info->faceRollDir;
    faceROIx = fd_info->faceROIx;
    faceROIy = fd_info->faceROIy;
    faceROIWidth = fd_info->faceROIWidth;
    faceROIHeight = fd_info->faceROIHeight;

    IMG_DUMP_DEC(fd, valid_faces_detected);
    IMG_DUMP_DEC(fd, fd_frame_width);
    IMG_DUMP_DEC(fd, fd_frame_height);

    if (valid_faces_detected > IMGLIB_ARRAY_SIZE(fd_info->faceRollDir)) {
      valid_faces_detected = IMGLIB_ARRAY_SIZE(fd_info->faceRollDir);
    }

    for (i=0; i<valid_faces_detected; i++) {
      IMG_DUMP(fd, "Face %d:\n", i);

      IMG_DUMP_DEC_AT_INDEX(fd, faceRollDir, i);
      IMG_DUMP_DEC_AT_INDEX(fd, faceROIx, i);
      IMG_DUMP_DEC_AT_INDEX(fd, faceROIy, i);
      IMG_DUMP_DEC_AT_INDEX(fd, faceROIWidth, i);
      IMG_DUMP_DEC_AT_INDEX(fd, faceROIHeight, i);
    }
  }

  return IMG_SUCCESS;
}

/** img_dump_meta_fd_info
 *    @p_meta: meta handler
 *    @fd: dump file fd
 *
 * Dumps faces detect info from metadata
 *
 * Returns standard image lib codes
 **/
static int32_t img_dump_meta_fd_info(img_meta_t *p_meta, FILE* fd)
{
  img_fd_info_t *fd_info = img_get_meta(p_meta, IMG_META_FD_INFO);
  uint32_t valid_faces_detected;
  uint32_t fd_frame_width;
  uint32_t fd_frame_height;
  int32_t *faceRollDir;
  uint32_t *faceROIx;
  uint32_t *faceROIy;
  uint32_t *faceROIWidth;
  uint32_t *faceROIHeight;
  uint32_t i;

  if (fd_info) {
    IMG_DUMP(fd, "\nFace Detect Info:\n");

    valid_faces_detected = fd_info->valid_faces_detected;
    fd_frame_width = fd_info->fd_frame_width;
    fd_frame_height = fd_info->fd_frame_height;
    faceRollDir = fd_info->faceRollDir;
    faceROIx = fd_info->faceROIx;
    faceROIy = fd_info->faceROIy;
    faceROIWidth = fd_info->faceROIWidth;
    faceROIHeight = fd_info->faceROIHeight;

    IMG_DUMP_DEC(fd, valid_faces_detected);
    IMG_DUMP_DEC(fd, fd_frame_width);
    IMG_DUMP_DEC(fd, fd_frame_height);

    if (valid_faces_detected > IMGLIB_ARRAY_SIZE(fd_info->faceRollDir)) {
      valid_faces_detected = IMGLIB_ARRAY_SIZE(fd_info->faceRollDir);
    }

    for (i=0; i<valid_faces_detected; i++) {
      IMG_DUMP(fd, "Face %d:\n", i);

      IMG_DUMP_DEC_AT_INDEX(fd, faceRollDir, i);
      IMG_DUMP_DEC_AT_INDEX(fd, faceROIx, i);
      IMG_DUMP_DEC_AT_INDEX(fd, faceROIy, i);
      IMG_DUMP_DEC_AT_INDEX(fd, faceROIWidth, i);
      IMG_DUMP_DEC_AT_INDEX(fd, faceROIHeight, i);
    }
  }

  return IMG_SUCCESS;
}

/** img_dump_meta_rotation
 *    @p_meta: meta handler
 *    @fd: dump file fd
 *
 * Dumps rotation from metadata
 *
 * Returns standard image lib codes
 **/
static int32_t img_dump_meta_rotation(img_meta_t *p_meta, FILE* fd)
{
  img_rotation_t *p_rotation = img_get_meta(p_meta, IMG_META_ROTATION);
  int32_t frame_rotation;
  int32_t device_rotation;

  if (p_rotation) {
    IMG_DUMP(fd, "\nRotation:\n");

    frame_rotation = p_rotation->frame_rotation;
    device_rotation = p_rotation->device_rotation;

    IMG_DUMP_DEC(fd, frame_rotation);
    IMG_DUMP_DEC(fd, device_rotation);
  }

  return IMG_SUCCESS;
}

/** img_dump_meta_flip
 *    @p_meta: meta handler
 *    @fd: dump file fd
 *
 * Dumps snapshot flip mode from metadata
 *
 * Returns standard image lib codes
 **/
static int32_t img_dump_meta_flip(img_meta_t *p_meta, FILE* fd)
{
  int32_t *p_flip = img_get_meta(p_meta, IMG_META_FLIP);
  int32_t snapshot_flip;

  if (p_flip) {
    IMG_DUMP(fd, "\nSnapshotFlip:\n");
    snapshot_flip = *p_flip;
    IMG_DUMP_DEC(fd, snapshot_flip);
  }

  return IMG_SUCCESS;
}

/** img_dump_meta_misc_data
 *    @p_meta: meta handler
 *    @fd: dump file fd
 *
 * Dumps misc data from metadata
 *
 * Returns standard image lib codes
 **/
static int32_t img_dump_meta_misc_data(img_meta_t *p_meta, FILE* fd)
{
  img_misc_t *misc_data = img_get_meta(p_meta, IMG_META_MISC_DATA);
  uint32_t i;
  uint32_t result, header_size, width, height;
  uint32_t size;
  uint8_t* data;

  if (misc_data) {
    IMG_DUMP(fd, "\nMisc Data:\n");

    data = p_meta->misc_data.data;
    result = misc_data->result;
    header_size = misc_data->header_size;
    width = misc_data->width;
    height = misc_data->height;
    size = misc_data->header_size + misc_data->width * misc_data->height;

    IMG_DUMP_DEC(fd, result);
    IMG_DUMP_DEC(fd, header_size);
    IMG_DUMP_DEC(fd, width);
    IMG_DUMP_DEC(fd, height);

    if (data) {
      for (i=0; i<size; i++) {
        IMG_DUMP_HEX_AT_INDEX(fd, data, i);
      }
    }
  }

  return IMG_SUCCESS;
}

/** img_dump_meta_chromaflash_ctrl
 *    @p_meta: meta handler
 *    @fd: dump file fd
 *
 * Dumps chroma flash control from metadata
 *
 * Returns standard image lib codes
 **/
static int32_t img_dump_meta_chromaflash_ctrl(img_meta_t *p_meta, FILE* fd)
{
  img_chromaflash_ctrl_t* p_cf_ctrl = img_get_meta(p_meta,
    IMG_META_CHROMAFLASH_CTRL);
  uint8_t deghost_enable;
  float flash_weight;
  float br_intensity;
  float contrast_enhancement;
  float sharpen_beta;
  float br_color;

  if (p_cf_ctrl) {
    IMG_DUMP(fd, "\nChroma Flash control:\n");

    deghost_enable = p_cf_ctrl->deghost_enable;
    flash_weight = p_cf_ctrl->flash_weight;
    br_intensity = p_cf_ctrl->br_intensity;
    contrast_enhancement = p_cf_ctrl->contrast_enhancement;
    sharpen_beta = p_cf_ctrl->sharpen_beta;
    br_color = p_cf_ctrl->br_color;

    IMG_DUMP_DEC(fd, deghost_enable);
    IMG_DUMP_FLOAT(fd, flash_weight);
    IMG_DUMP_FLOAT(fd, br_intensity);
    IMG_DUMP_FLOAT(fd, contrast_enhancement);
    IMG_DUMP_FLOAT(fd, sharpen_beta);
    IMG_DUMP_FLOAT(fd, br_color);
  }

  return IMG_SUCCESS;
}

/** img_dump_meta_num_input
 *    @p_meta: meta handler
 *    @fd: dump file fd
 *
 * Dumps number of input bufs from metadata
 *
 * Returns standard image lib codes
 **/
static int32_t img_dump_meta_num_input(img_meta_t *p_meta, FILE* fd)
{
  uint8_t *p_num_input = img_get_meta(p_meta, IMG_META_NUM_INPUT);
  uint8_t num_input;

  if (p_num_input) {
    IMG_DUMP(fd, "\nNum Input:\n");

    num_input = *p_num_input;

    IMG_DUMP_DEC(fd, num_input);
  }

  return IMG_SUCCESS;
}

/** img_dump_meta_tp_config
 *    @p_meta: meta handler
 *    @fd: dump file fd
 *
 * Dumps the trueportrait config from metadata
 *
 * Returns standard image lib codes
 **/
static int32_t img_dump_meta_tp_config(img_meta_t *p_meta, FILE* fd)
{
  img_tp_config_t *p_tp_config = img_get_meta(p_meta, IMG_META_TP_CONFIG);
  uint32_t bodyMaskEnabled = -1;
  uint32_t effectsEnabled = -1;
  uint32_t effect = 0;
  uint32_t intensity = 0;

  if (p_tp_config) {
    IMG_DUMP(fd, "\nTruePortrait Config:\n");

    bodyMaskEnabled = p_tp_config->enable_bodymask;
    effectsEnabled = p_tp_config->enable_effects;
    effect = p_tp_config->effect;
    intensity = p_tp_config->intensity;

    IMG_DUMP_DEC(fd, bodyMaskEnabled);
    IMG_DUMP_DEC(fd, effectsEnabled);
    IMG_DUMP_DEC(fd, effect);
    IMG_DUMP_DEC(fd, intensity);
  }

  return IMG_SUCCESS;
}

/** img_dump_meta_dcrf_runtime_param
 *    @p_meta: meta handler
 *    @fd: dump file fd
 *
 * Dumps dcrf runtime param from metadata
 *
 * Returns standard image lib codes
 **/
static int32_t img_dump_meta_dcrf_runtime_param(img_meta_t *p_meta, FILE* fd)
{
  int i = 0;
  img_dcrf_input_runtime_param_t *runtime_param =
    img_get_meta(p_meta, IMG_META_DCRF_RUNTIME_PARAM);

  if (runtime_param) {
    float lens_zoom_ratio = runtime_param->lens_zoom_ratio;
    img_rect_t roi_of_main = runtime_param->roi_of_main;
    uint32_t af_fps = runtime_param->af_fps;
    img_fov_t *fov_params_main = runtime_param->fov_params_main;
    img_fov_t *fov_params_aux = runtime_param->fov_params_aux;

    IMG_DUMP(fd, "\nDCRF Runtime param:\n");

    IMG_DUMP_FLOAT(fd, lens_zoom_ratio);
    IMG_DUMP_DEC(fd, roi_of_main.pos.x);
    IMG_DUMP_DEC(fd, roi_of_main.pos.y);
    IMG_DUMP_DEC(fd, roi_of_main.size.width);
    IMG_DUMP_DEC(fd, roi_of_main.size.height);
    IMG_DUMP_DEC(fd, af_fps);

    IMG_DUMP(fd, "FOV main :\n");
    for (i=0; i<IMG_MAX_FOV; i++) {
      IMG_DUMP_DEC(fd, fov_params_main->module);
      IMG_DUMP_DEC(fd, fov_params_main->input_width);
      IMG_DUMP_DEC(fd, fov_params_main->input_height);
      IMG_DUMP_DEC(fd, fov_params_main->offset_x);
      IMG_DUMP_DEC(fd, fov_params_main->offset_y);
      IMG_DUMP_DEC(fd, fov_params_main->fetch_window_width);
      IMG_DUMP_DEC(fd, fov_params_main->fetch_window_height);
      IMG_DUMP_DEC(fd, fov_params_main->output_window_width);
      IMG_DUMP_DEC(fd, fov_params_main->output_window_height);
    }
    IMG_DUMP(fd, "FOV aux :\n");
    for (i=0; i<IMG_MAX_FOV; i++) {
      IMG_DUMP_DEC(fd, fov_params_aux->module);
      IMG_DUMP_DEC(fd, fov_params_aux->input_width);
      IMG_DUMP_DEC(fd, fov_params_aux->input_height);
      IMG_DUMP_DEC(fd, fov_params_aux->offset_x);
      IMG_DUMP_DEC(fd, fov_params_aux->offset_y);
      IMG_DUMP_DEC(fd, fov_params_aux->fetch_window_width);
      IMG_DUMP_DEC(fd, fov_params_aux->fetch_window_height);
      IMG_DUMP_DEC(fd, fov_params_aux->output_window_width);
      IMG_DUMP_DEC(fd, fov_params_aux->output_window_height);
    }
  }

  return IMG_SUCCESS;
}

/** img_dump_meta_dcrf_result
 *    @p_meta: meta handler
 *    @fd: dump file fd
 *
 * Dumps dcrf result from metadata
 *
 * Returns standard image lib codes
 **/
static int32_t img_dump_meta_dcrf_result(img_meta_t *p_meta, FILE* fd)
{
  img_dcrf_output_result_t *result =
    img_get_meta(p_meta, IMG_META_DCRF_RESULT);

  if (result) {
    uint32_t frame_id = result->frame_id;
    uint64_t timestamp = result->timestamp;
    uint32_t distance_in_mm = result->distance_in_mm;
    uint32_t confidence = result->confidence;
    uint32_t status = result->status;
    img_rect_t focused_roi = result->focused_roi;
    uint32_t focused_x = result->focused_x;

    IMG_DUMP(fd, "\nDCRF Result:\n");

    IMG_DUMP_DEC(fd, frame_id);
    IMG_DUMP_LONGLONG(fd, timestamp);
    IMG_DUMP_DEC(fd, distance_in_mm);
    IMG_DUMP_DEC(fd, confidence);
    IMG_DUMP_DEC(fd, status);
    IMG_DUMP_DEC(fd, focused_x);
    IMG_DUMP_DEC(fd, focused_roi.pos.x);
    IMG_DUMP_DEC(fd, focused_roi.pos.y);
    IMG_DUMP_DEC(fd, focused_roi.size.width);
    IMG_DUMP_DEC(fd, focused_roi.size.height);
  }
  return IMG_SUCCESS;
}

/** img_dump_meta_frame_operations
 *    @p_meta: meta handler
 *    @fd: dump file fd
 *
 * Dumps frame operations enabled from metadata
 *
 * Returns standard image lib codes
 **/
static int32_t img_dump_meta_frame_operations(img_meta_t *p_meta, FILE* fd)
{
  uint32_t *p_sw2d_frameops = img_get_meta(p_meta, IMG_META_SW2D_OPS);

  if (p_sw2d_frameops) {
    IMG_DUMP(fd, "\nSW2D Frame Operation:\n");
    IMG_DUMP_DEC(fd, *p_sw2d_frameops);
  }

  return IMG_SUCCESS;
}

/** img_dump_meta_seemore_cfg
 *    @p_meta: meta handler
 *    @fd: dump file fd
 *
 * Dumps seemore config from metadata
 *
 * Returns standard image lib codes
 **/
static int32_t img_dump_meta_seemore_cfg(img_meta_t *p_meta, FILE* fd)
{
  img_seemore_cfg_t *p_cfg =
    img_get_meta(p_meta, IMG_META_SEEMORE_CFG);

  if (p_cfg) {

    float br_intensity = p_cfg->br_intensity;
    float br_color = p_cfg->br_color;
    bool enable_LTM = p_cfg->enable_LTM;
    bool enable_TNR = p_cfg->enable_TNR;
    float tnr_intensity = p_cfg->tnr_intensity;
    float mot_det_sensitivity = p_cfg->mot_det_sensitivity;
    IMG_DUMP(fd, "\nSeemore CFG:\n");

    IMG_DUMP_FLOAT(fd, br_intensity);
    IMG_DUMP_FLOAT(fd, br_color);
    IMG_DUMP_DEC(fd, enable_LTM);
    IMG_DUMP_DEC(fd, enable_TNR);
    IMG_DUMP_FLOAT(fd, tnr_intensity);
    IMG_DUMP_FLOAT(fd, mot_det_sensitivity);
  }
  return IMG_SUCCESS;
}

/** img_dump_meta_stillmore_cfg
 *
 *    @p_meta: meta handler
 *    @fd: dump file fd
 *
 * Dumps stillmore config from metadata
 *
 * Returns standard image lib codes
 **/
static int32_t img_dump_meta_stillmore_cfg(img_meta_t *p_meta, FILE* fd)
{
  img_stillmore_cfg_t *p_cfg =
    img_get_meta(p_meta, IMG_META_STILLMORE_CFG);

  if (p_cfg) {

    float br_intensity = p_cfg->br_intensity;
    IMG_DUMP(fd, "\nStillmore CFG:\n");

    IMG_DUMP_FLOAT(fd, br_intensity);
  }
  return IMG_SUCCESS;
}

/** img_dump_meta_paaf_param
 *
 *    @p_meta: meta handler
 *    @fd: dump file fd
 *
 * Dumps stillmore config from metadata
 *
 * Returns standard image lib codes
 **/
static int32_t img_dump_meta_paaf_param(img_meta_t *p_meta, FILE* fd)
{
  img_paaf_cfg_t *p_paaf_cfg =
    img_get_meta(p_meta, IMG_META_PAAF_CFG);

  if (p_paaf_cfg) {
    IMG_DUMP(fd, "\nPAAF CFG:\n");
    IMG_DUMP_DEC(fd, p_paaf_cfg->enable);
    IMG_DUMP_DEC(fd, p_paaf_cfg->frame_id);
    if (p_paaf_cfg->enable) {
      IMG_DUMP_DEC(fd, p_paaf_cfg->filter_type);
      IMG_DUMP_DEC(fd, p_paaf_cfg->coeff_len);
      if (p_paaf_cfg->filter_type == PAAF_ON_IIR) {
        IMG_DUMP_FLOAT_AT_INDEX(fd, p_paaf_cfg->coeffa, 0);
        IMG_DUMP_FLOAT_AT_INDEX(fd, p_paaf_cfg->coeffa, 1);
        IMG_DUMP_FLOAT_AT_INDEX(fd, p_paaf_cfg->coeffa, 2);
        IMG_DUMP_FLOAT_AT_INDEX(fd, p_paaf_cfg->coeffa, 3);
        IMG_DUMP_FLOAT_AT_INDEX(fd, p_paaf_cfg->coeffa, 4);
        IMG_DUMP_FLOAT_AT_INDEX(fd, p_paaf_cfg->coeffa, 5);
        IMG_DUMP_FLOAT_AT_INDEX(fd, p_paaf_cfg->coeffb, 0);
        IMG_DUMP_FLOAT_AT_INDEX(fd, p_paaf_cfg->coeffb, 1);
        IMG_DUMP_FLOAT_AT_INDEX(fd, p_paaf_cfg->coeffb, 2);
        IMG_DUMP_FLOAT_AT_INDEX(fd, p_paaf_cfg->coeffb, 3);
        IMG_DUMP_FLOAT_AT_INDEX(fd, p_paaf_cfg->coeffb, 4);
        IMG_DUMP_FLOAT_AT_INDEX(fd, p_paaf_cfg->coeffb, 5);
      } else if (p_paaf_cfg->filter_type == PAAF_ON_FIR) {
        IMG_DUMP_DEC_AT_INDEX(fd, p_paaf_cfg->coeff_fir, 0);
        IMG_DUMP_DEC_AT_INDEX(fd, p_paaf_cfg->coeff_fir, 1);
        IMG_DUMP_DEC_AT_INDEX(fd, p_paaf_cfg->coeff_fir, 2);
        IMG_DUMP_DEC_AT_INDEX(fd, p_paaf_cfg->coeff_fir, 3);
        IMG_DUMP_DEC_AT_INDEX(fd, p_paaf_cfg->coeff_fir, 4);
        IMG_DUMP_DEC_AT_INDEX(fd, p_paaf_cfg->coeff_fir, 5);
        IMG_DUMP_DEC_AT_INDEX(fd, p_paaf_cfg->coeff_fir, 6);
        IMG_DUMP_DEC_AT_INDEX(fd, p_paaf_cfg->coeff_fir, 7);
        IMG_DUMP_DEC_AT_INDEX(fd, p_paaf_cfg->coeff_fir, 8);
        IMG_DUMP_DEC_AT_INDEX(fd, p_paaf_cfg->coeff_fir, 9);
        IMG_DUMP_DEC_AT_INDEX(fd, p_paaf_cfg->coeff_fir, 10);
      }
      IMG_DUMP_FLOAT(fd, p_paaf_cfg->FV_min);
      IMG_DUMP_DEC(fd, p_paaf_cfg->pixel_skip_cnt);
      IMG_DUMP_DEC(fd, p_paaf_cfg->roi.pos.x);
      IMG_DUMP_DEC(fd, p_paaf_cfg->roi.pos.y);
      IMG_DUMP_DEC(fd, p_paaf_cfg->roi.size.width);
      IMG_DUMP_DEC(fd, p_paaf_cfg->roi.size.height);
      IMG_DUMP_DEC(fd, p_paaf_cfg->cropped_roi.pos.x);
      IMG_DUMP_DEC(fd, p_paaf_cfg->cropped_roi.pos.y);
      IMG_DUMP_DEC(fd, p_paaf_cfg->cropped_roi.size.width);
      IMG_DUMP_DEC(fd, p_paaf_cfg->cropped_roi.size.height);
    }
  }
  return IMG_SUCCESS;
}

/** img_dump_meta_sat_param
 *
 *    @p_meta: meta handler
 *    @fd: dump file fd
 *
 * Dumps SAT config from metadata
 *
 * Returns standard image lib codes
 **/
static int32_t img_dump_meta_sat_param(img_meta_t *p_meta, FILE* fd)
{
  int i;
  img_fov_t *fov = NULL;
  float frame_rate;
  img_camera_role_t camera_role;
  bool low_power_mode;
  img_camera_fallback_mode_t fallback_mode;
  img_spatial_transform_cfg_t *p_sat_cfg =
    img_get_meta(p_meta, IMG_META_SPATIAL_TRANSFORM_CFG);

  IMG_DUMP(fd, "\nSAT CFG:\n");
  if (p_sat_cfg) {
    frame_rate = p_sat_cfg->frameRate;
    low_power_mode = p_sat_cfg->low_power_mode;
    camera_role = p_sat_cfg->camera_role;
    fallback_mode = p_sat_cfg->fallback_mode;
    fov = &p_sat_cfg->fov_params[0];

    IMG_DUMP_DEC(fd, camera_role);
    IMG_DUMP_DEC(fd, low_power_mode);
    IMG_DUMP_DEC(fd, fallback_mode);
    IMG_DUMP_FLOAT(fd, p_sat_cfg->frameRate);

    for (i = 0; i < IMG_MAX_FOV; i++) {
      IMG_DUMP_DEC(fd, fov[i].module);
      IMG_DUMP_DEC(fd, fov[i].input_width);
      IMG_DUMP_DEC(fd, fov[i].input_height);
      IMG_DUMP_DEC(fd, fov[i].fetch_window_width);
      IMG_DUMP_DEC(fd, fov[i].fetch_window_height);
      IMG_DUMP_DEC(fd, fov[i].offset_x);
      IMG_DUMP_DEC(fd, fov[i].offset_y);
    }
  } else {
    IDBG_ERROR("SAT config is NULL");
  }
  img_dump_meta_zoom_factor(p_meta, fd);
  img_dump_meta_aec_info(p_meta, fd);
  img_dump_meta_af_info(p_meta, fd);
  img_dump_meta_awb_info(p_meta, fd);

  return IMG_SUCCESS;
}

/** img_dump_meta_sac_param
 *
 *    @p_meta: meta handler
 *    @fd: dump file fd
 *
 * Dumps SAC config from metadata
 *
 * Returns standard image lib codes
 **/
static int32_t img_dump_meta_sac_param(img_meta_t *p_meta, FILE* fd)
{
  int i;
  img_fov_t *fov = NULL;
  img_camera_role_t camera_role;
  img_spatial_align_compute_cfg_t *p_sac_cfg =
    img_get_meta(p_meta, IMG_META_SPATIAL_ALIGN_COMPUTE_CFG);

  IMG_DUMP(fd, "\nSAC CFG:\n");
  if (p_sac_cfg) {
    camera_role = p_sac_cfg->camera_role;
    fov = &p_sac_cfg->fov_params[0];

    IMG_DUMP_DEC(fd, camera_role);

    for (i = 0; i < IMG_MAX_FOV; i++) {
      IMG_DUMP_DEC(fd, fov[i].module);
      IMG_DUMP_DEC(fd, fov[i].input_width);
      IMG_DUMP_DEC(fd, fov[i].input_height);
      IMG_DUMP_DEC(fd, fov[i].fetch_window_width);
      IMG_DUMP_DEC(fd, fov[i].fetch_window_height);
      IMG_DUMP_DEC(fd, fov[i].offset_x);
      IMG_DUMP_DEC(fd, fov[i].offset_y);
    }
  } else {
    IDBG_ERROR("SAC config is NULL");
  }
  img_dump_meta_zoom_factor(p_meta, fd);
  img_dump_meta_aec_info(p_meta, fd);
  img_dump_meta_af_info(p_meta, fd);
  img_dump_meta_awb_info(p_meta, fd);

  return IMG_SUCCESS;
}

/** img_dump_meta_rtb_param
 *
 *    @p_meta: meta handler
 *    @fd: dump file fd
 *
 * Dumps RTB config from metadata
 *
 * Returns standard image lib codes
 **/
static int32_t img_dump_meta_rtb_param(img_meta_t *p_meta, FILE* fd)
{
  int i;
  img_camera_role_t camera_role;
  img_fov_t *fov = NULL;
  uint32_t blur_level;
  uint32_t blur_min_value;
  uint32_t blur_max_value;
  uint8_t capture_requested;
  img_realtime_bokeh_cfg_t *p_rtb_cfg =
    img_get_meta(p_meta, IMG_META_BOKEH_CFG);

  IMG_DUMP(fd, "\nRTB CFG:\n");
  if (p_rtb_cfg) {
    camera_role = p_rtb_cfg->camera_role;
    fov = &p_rtb_cfg->fov_params[0];
    blur_level = p_rtb_cfg->blur_level;
    blur_min_value = p_rtb_cfg->blur_min_value;
    blur_max_value = p_rtb_cfg->blur_max_value;
    capture_requested = p_rtb_cfg->capture_requested;
    IMG_DUMP_DEC(fd, camera_role);

    for (i = 0; i < IMG_MAX_FOV; i++) {
      IMG_DUMP_DEC(fd, fov[i].module);
      IMG_DUMP_DEC(fd, fov[i].input_width);
      IMG_DUMP_DEC(fd, fov[i].input_height);
      IMG_DUMP_DEC(fd, fov[i].fetch_window_width);
      IMG_DUMP_DEC(fd, fov[i].fetch_window_height);
      IMG_DUMP_DEC(fd, fov[i].offset_x);
      IMG_DUMP_DEC(fd, fov[i].offset_y);
    }

    IMG_DUMP_DEC(fd, blur_level);
    IMG_DUMP_DEC(fd, blur_min_value);
    IMG_DUMP_DEC(fd, blur_max_value);
    IMG_DUMP_DEC(fd, capture_requested);

  } else {
    IDBG_ERROR("SAC config is NULL");
  }
  img_dump_meta_zoom_factor(p_meta, fd);
  img_dump_meta_aec_info(p_meta, fd);
  img_dump_meta_af_info(p_meta, fd);
  img_dump_meta_awb_info(p_meta, fd);

  return IMG_SUCCESS;
}

/** img_dump_meta_rtbdm_param
 *
 *    @p_meta: meta handler
 *    @fd: dump file fd
 *
 * Dumps RTBDM config from metadata
 *
 * Returns standard image lib codes
 **/
static int32_t img_dump_meta_rtbdm_param(img_meta_t *p_meta, FILE* fd)
{
  int i;
  img_camera_role_t camera_role;
  img_fov_t *fov = NULL;
  uint32_t blur_level;
  uint32_t blur_min_value;
  uint32_t blur_max_value;
  uint8_t capture_requested;
  img_realtime_bokeh_depth_map_cfg_t *p_rtbdm_cfg =
    img_get_meta(p_meta, IMG_META_BOKEH_DEPTH_MAP_CFG);

  IMG_DUMP(fd, "\nRTBDM CFG:\n");
  if (p_rtbdm_cfg) {
    camera_role = p_rtbdm_cfg->camera_role;
    fov = &p_rtbdm_cfg->fov_params[0];
    blur_level = p_rtbdm_cfg->blur_level;
    blur_min_value = p_rtbdm_cfg->blur_min_value;
    blur_max_value = p_rtbdm_cfg->blur_max_value;
    capture_requested = p_rtbdm_cfg->capture_requested;
    IMG_DUMP_DEC(fd, camera_role);

    for (i = 0; i < IMG_MAX_FOV; i++) {
      IMG_DUMP_DEC(fd, fov[i].module);
      IMG_DUMP_DEC(fd, fov[i].input_width);
      IMG_DUMP_DEC(fd, fov[i].input_height);
      IMG_DUMP_DEC(fd, fov[i].fetch_window_width);
      IMG_DUMP_DEC(fd, fov[i].fetch_window_height);
      IMG_DUMP_DEC(fd, fov[i].offset_x);
      IMG_DUMP_DEC(fd, fov[i].offset_y);
    }

    IMG_DUMP_DEC(fd, blur_level);
    IMG_DUMP_DEC(fd, blur_min_value);
    IMG_DUMP_DEC(fd, blur_max_value);
    IMG_DUMP_DEC(fd, capture_requested);

  } else {
    IDBG_ERROR("SAC config is NULL");
  }
  img_dump_meta_zoom_factor(p_meta, fd);
  img_dump_meta_aec_info(p_meta, fd);
  img_dump_meta_af_info(p_meta, fd);
  img_dump_meta_awb_info(p_meta, fd);

  return IMG_SUCCESS;
}

/** img_dump_meta
 *
 *    @p_meta_data: metadata handler
 *    @file_name: file name prefix
 *
 * Saves specified meta data
 *
 * Returns nothing
 **/
void img_dump_meta(void *p_meta_data, const char* file_name)
{
  uint32_t i;
  img_meta_t *p_meta = (img_meta_t *)p_meta_data;
  FILE* p_out_file_handler;

  if (!p_meta || !file_name) {
    IDBG_ERROR("%s:%d failed: Null pointer detected\n", __func__, __LINE__);
    return;
  }

  p_out_file_handler = fopen(file_name, "w+");

  if (!p_out_file_handler) {
    IDBG_ERROR("%s:%d failed: Cannot open file %s\n", __func__, __LINE__,
      file_name);
    return;
  }

  for (i=0; i<IMGLIB_ARRAY_SIZE(dump_handlers); i++) {
    if (IMG_SUCCESS != dump_handlers[i](p_meta, p_out_file_handler)) {
      break;
    }
  }

  fclose(p_out_file_handler);

  return;
}
