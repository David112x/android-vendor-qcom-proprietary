
/* hvx_zzHDR.c
*
* Copyright (c) 2015 - 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*
*/

/* std headers */
#include <stdio.h>
#include <stdlib.h>
#include <utils/Log.h>
#include <string.h>
#include <math.h>
#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#include <linux/videodev2.h>
/* hvx headers */
#include "hvx_lib.h"
#include "zzHDR_reg.h"


#define ILOG(fmt, args...) \
  ALOGE("%s:%d hvx_debug "fmt"\n", __func__, __LINE__, ##args)

#define INLOG(fmt, args...) \
  ALOGE("hvx_debug "fmt"\n", ##args)

#ifndef sign
#define sign(x) ((x < 0) ?(-1) : (1))
#endif

#ifndef Round
#define Round(x) (int)(x + sign(x)*0.5)
#endif

#define DEFAULT_BLACK_LEVEL 1024
/* Return v1 * ratio + v2 * ( 1.0 - ratio ) */
#define LINEAR_INTERPOLATION(v1, v2, ratio) \
  ((v2) + ((ratio) * ((v1) - (v2))))
#define F_EQUAL(a, b) \
  ( fabs(a-b) < 1e-4 )

#define CLAMP_LIMIT(x, t1, t2) (((x) <= (t1))? (t1): ((x) > (t2))? (t2): (x))

#define FLOAT_TO_Q(exp, f) \
  ((int32_t)((f*(1<<(exp))) + ((f<0) ? -0.5 : 0.5)))

#define ISP_HDR_WB_RATIO_MAX 16383
#define ISP_HDR_WB_RATIO_MIN 1024

#define ISP_HDR_EXP_RATIO_MAX 16384
#define ISP_HDR_EXP_RATIO_MIN 1024

#define ISP_HDR_EXP_RECIP_RATIO_MAX 256
#define ISP_HDR_EXP_RECIP_RATIO_MIN 16

#define ISP_HDR_MAC_SQRT_ANALOG_GAIN_MAX 90
#define ISP_HDR_MAC_SQRT_ANALOG_GAIN_MIN 16



static hvx_ret_type_t zzHDR_print_cfg(zzHDR_cfg_cmd  *zzHDR_cfg)
{
  if (!zzHDR_cfg){
      ALOGE("zzHDR_cfg is NULL\n");
      return HVX_FALSE;
  }
  ALOGE("%s zzhdr_linear_mode %d", __func__, zzHDR_cfg->zzhdr_linear_mode);
  ALOGE("%s hdr_exp_ratio %d",  __func__, zzHDR_cfg->hdr_exp_ratio);
  ALOGE("%s hdr_exp_ratio_recip %d", __func__, zzHDR_cfg->hdr_exp_ratio_recip);
  ALOGE("%s hdr_black_in %d",  __func__, zzHDR_cfg->hdr_black_in);
  ALOGE("%s hdr_rg_wb_gain_ratio %d",  __func__, zzHDR_cfg->hdr_rg_wb_gain_ratio);
  ALOGE("%s hdr_bg_wb_gain_ratio %d ", __func__, zzHDR_cfg->hdr_bg_wb_gain_ratio );
  ALOGE("%s hdr_gr_wb_gain_ratio %d" ,__func__,zzHDR_cfg->hdr_gr_wb_gain_ratio);
  ALOGE("%s hdr_gb_wb_gain_ratio %d",__func__, zzHDR_cfg->hdr_gb_wb_gain_ratio);
  ALOGE("%s hdr_rec_h_edge_th1 %d", __func__, zzHDR_cfg->hdr_rec_h_edge_th1);
  ALOGE("%s hdr_rec_h_edge_dth_log2 %d", __func__, zzHDR_cfg->hdr_rec_h_edge_dth_log2);
  ALOGE("%s hdr_rec_motion_th1 %d", __func__, zzHDR_cfg->hdr_rec_motion_th1);
  ALOGE("%s hdr_rec_motion_dth_log2 %d", __func__, zzHDR_cfg->hdr_rec_motion_dth_log2);
  ALOGE("%s hdr_rec_dark_th1 %d", __func__, zzHDR_cfg->hdr_rec_dark_th1);
  ALOGE("%s hdr_rec_dark_dth_log2 %d", __func__, zzHDR_cfg->hdr_rec_dark_dth_log2);
  ALOGE("%s hdr_mac_lowlight_strength %d",__func__, zzHDR_cfg->hdr_mac_lowlight_strength);
  ALOGE("%s hdr_mac_lowlight_th1 %d", __func__, zzHDR_cfg->hdr_mac_lowlight_th1);
  ALOGE("%s hdr_mac_lowlight_dth_log2 %d", __func__, zzHDR_cfg->hdr_mac_lowlight_dth_log2);
  ALOGE("%s hdr_mac_hilight_th1 %d", __func__, zzHDR_cfg->hdr_mac_hilight_th1);
  ALOGE("%s hdr_mac_hilight_dth_log2 %d", __func__, zzHDR_cfg->hdr_mac_hilight_dth_log2);
  ALOGE("%s hdr_zrec_pattern %d", __func__,zzHDR_cfg->hdr_zrec_pattern);
  ALOGE("%s hdr_zrec_first_rb_exp %d", __func__, zzHDR_cfg->hdr_zrec_first_rb_exp);
  ALOGE("%s hdr_zrec_prefilt_tap0 %d",  __func__, zzHDR_cfg->hdr_zrec_prefilt_tap0);
  ALOGE("%s hdr_zrec_g_grad_th1 %d", __func__, zzHDR_cfg->hdr_zrec_g_grad_th1);
  ALOGE("%s hdr_zrec_g_grad_dth_log2 %d",  __func__, zzHDR_cfg->hdr_zrec_g_grad_dth_log2);
  ALOGE("%s hdr_zrec_rb_grad_th1 %d", __func__, zzHDR_cfg->hdr_zrec_rb_grad_th1 );
  ALOGE("%s hdr_zrec_rb_grad_dth_log2 %d",  __func__, zzHDR_cfg->hdr_zrec_rb_grad_dth_log2);

  return HVX_TRUE;

}


static hvx_ret_type_t zzHDR_init_config(zzHDR_cfg_cmd  *zzHDR_cfg)
{
  if (!zzHDR_cfg){
      ALOGE("zzHDR_cfg is NULL\n");
      return HVX_FALSE;
  }
  zzHDR_cfg->zzhdr_linear_mode = 1;
  zzHDR_cfg->hdr_exp_ratio = 1024;
  zzHDR_cfg->hdr_exp_ratio_recip = 256;
  zzHDR_cfg->hdr_black_in = DEFAULT_BLACK_LEVEL;
  zzHDR_cfg->hdr_rg_wb_gain_ratio = 4096;
  zzHDR_cfg->hdr_bg_wb_gain_ratio = 4096;
  zzHDR_cfg->hdr_gr_wb_gain_ratio = 4096;
  zzHDR_cfg->hdr_gb_wb_gain_ratio = 4096;
  zzHDR_cfg->hdr_rec_h_edge_th1 = 16;
  zzHDR_cfg->hdr_rec_h_edge_dth_log2 = 4;
  zzHDR_cfg->hdr_rec_motion_th1 = 184;
  zzHDR_cfg->hdr_rec_motion_dth_log2 = 4;
  zzHDR_cfg->hdr_rec_dark_th1 = 0;
  zzHDR_cfg->hdr_rec_dark_dth_log2 = 4;
  zzHDR_cfg->hdr_mac_lowlight_strength = 8;
  zzHDR_cfg->hdr_mac_lowlight_th1 = 0;
  zzHDR_cfg->hdr_mac_lowlight_dth_log2 = 6;
  zzHDR_cfg->hdr_mac_hilight_th1 = 232;
  zzHDR_cfg->hdr_mac_hilight_dth_log2 = 10;
  zzHDR_cfg->hdr_zrec_pattern = 0;

  zzHDR_cfg->hdr_zrec_first_rb_exp = 0;
  zzHDR_cfg->hdr_zrec_prefilt_tap0 = 16;
  zzHDR_cfg->hdr_zrec_g_grad_th1 = 32;
  zzHDR_cfg->hdr_zrec_g_grad_dth_log2 = 5;
  zzHDR_cfg->hdr_zrec_rb_grad_th1 = 32;
  zzHDR_cfg->hdr_zrec_rb_grad_dth_log2 = 5;

  return HVX_TRUE;

}

static char * hvx_util_sensor_pattern(uint32_t pix_fmt_fourcc)
{
  char *sensor_pattern = NULL;
  ALOGI("%s %d ", __func__, __LINE__);
  sensor_pattern = (char *) malloc(sizeof(char) * 32);
  if (!sensor_pattern) {
    return NULL;
  }
  switch (pix_fmt_fourcc)
  {
    case V4L2_PIX_FMT_SBGGR8:
    case V4L2_PIX_FMT_SBGGR10:
    case V4L2_PIX_FMT_SBGGR12:
      strlcpy(sensor_pattern, "BGGR",32);
    break;

    case V4L2_PIX_FMT_SGBRG8:
    case V4L2_PIX_FMT_SGBRG10:
    case V4L2_PIX_FMT_SGBRG12:
      strlcpy(sensor_pattern, "GBGR",32);
    break;

    case V4L2_PIX_FMT_SGRBG8:
    case V4L2_PIX_FMT_SGRBG10:
    case V4L2_PIX_FMT_SGRBG12:
      strlcpy(sensor_pattern, "GRBG",32);
    break;

    case V4L2_PIX_FMT_SRGGB8:
    case V4L2_PIX_FMT_SRGGB10:
    case V4L2_PIX_FMT_SRGGB12:
      strlcpy(sensor_pattern, "RGGB",32);
    break;

    default:
      strlcpy(sensor_pattern, "BGGR",32);
      break;

  }
  ALOGD("sensor_pattern %s ", sensor_pattern);
  return sensor_pattern;

}


static hvx_ret_type_t hvx_lib_open(void **oem_data)
{
  zzHDR_mod_t *zzHDR_mod = (zzHDR_mod_t *)calloc(1, sizeof(zzHDR_mod_t));
  if (!zzHDR_mod) {
    /* log error */
    ALOGE("%s:%d failed: zzHDR_mod %p\n", __func__, __LINE__, zzHDR_mod);
    return HVX_RET_FAILURE;
  }
  *oem_data = (void *)zzHDR_mod;
  return HVX_RET_SUCCESS;
}

static hvx_ret_type_t hvx_lib_get_hvx_info(void *oem_data,
    hvx_lib_get_hvx_info_t *hvx_info)
{
  char *sensor_pattern = NULL;
  if (!oem_data || !hvx_info) {
   ALOGE("%s:%d failed: oem_data %p hvx_info %p\n", __func__, __LINE__,
     oem_data, hvx_info);
   return HVX_RET_FAILURE;
  }

  hvx_info->hvx_enable = HVX_TRUE;
  sensor_pattern = hvx_util_sensor_pattern(hvx_info->output_format);
  strlcpy(hvx_info->algo_name, "zzhdr_",
    sizeof(hvx_info->algo_name));
  if (!sensor_pattern) {
    strlcpy(hvx_info->algo_name, "zzhdr_BGGR",
      sizeof(hvx_info->algo_name));
  } else {
    strlcat(hvx_info->algo_name, sensor_pattern,
      strlen(sensor_pattern) + strlen(hvx_info->algo_name) +2);
  }
  ALOGI("%s algo name %s sensor_pattern %s \n",__func__,  hvx_info->algo_name,
       sensor_pattern );
  hvx_info->hvx_out_width = hvx_info->sensor_width;
  hvx_info->hvx_out_height = hvx_info->sensor_height;
  if (hvx_info->available_hvx_units >= 1) {
    hvx_info->request_hvx_units = 1;
  }
  if (hvx_info->available_hvx_vector_mode >= HVX_LIB_VECTOR_64) {
    hvx_info->request_hvx_vector_mode = HVX_LIB_VECTOR_128;
  }
  hvx_info->is_pix_intf_needed = HVX_TRUE;
  hvx_info->is_stats_needed = HVX_FALSE;
  hvx_info->is_dump_frame_needed = HVX_TRUE;
  /* Disbaling black level module */
  hvx_info->disable_module_mask |= ((uint64_t)1 << ISP_MOD_LINEARIZATION);
  hvx_info->disable_module_mask |= ((uint64_t)1 << ISP_MOD_BLS);
  hvx_info->disable_module_mask |= ((uint64_t)1 << ISP_MOD_PEDESTAL);
  if (sensor_pattern) {
    free(sensor_pattern);
    sensor_pattern = NULL;
  }
  return HVX_RET_SUCCESS;
}

static hvx_ret_type_t hvx_lib_consume_stats(void *oem_data,
  unsigned int frame_id, hvx_lib_stats_t *stats_data,
  const hvx_lib_adsp_config_t *adsp_config, void *caller_data)
{
  struct hvx_lib_private_data_t *private_data = NULL;
  hvx_ret_type_t                 ret = HVX_RET_SUCCESS;
  uint32_t                      *data = NULL;
  char                           value[PROPERTY_VALUE_MAX];
  char                           buf[100];
  FILE                          *fptr = NULL;
  uint32_t                       i = 0;
  int                            write_length;

  ALOGI("%s:%d: E", __func__, __LINE__);
  if (!oem_data || !adsp_config || !adsp_config->adsp_config_call ||
    !stats_data || !caller_data) {
    /* log error */
    ALOGE("%s:%d failed: %p %p %p %p\n", __func__, __LINE__, oem_data,
      adsp_config, stats_data, caller_data);
    return HVX_RET_FAILURE;
  }

  ALOGD("%s:%d: buffer info: left addr %p, left size %d, right addr %p, right size %d",
        __func__, __LINE__, stats_data->stats_left, stats_data->stats_left_size,
        stats_data->stats_right, stats_data->stats_right_size);

  /* Dump frame buffer */
  if (stats_data->stats_left && stats_data->stats_left_size) {
    data = (uint32_t *)stats_data->stats_left;
    snprintf(buf, sizeof(buf), QCAMERA_DUMP_FRM_LOCATION"hvx_stats_left_frame%d.bin", frame_id);
    fptr = fopen(buf, "w+");
    if (!fptr) {
      ALOGE("%s:%d failed fptr %p buf %s\n", __func__, __LINE__, fptr, buf);
    } else {
      write_length = fwrite(data, 1, stats_data->stats_left_size, fptr);
      ALOGE("%s: %d: write length for frame id %d is %d", __func__, __LINE__,
            frame_id, write_length);
      fclose(fptr);
    }
  }
  if (stats_data->stats_right && stats_data->stats_right_size) {
    data = (uint32_t *)stats_data->stats_right;
    snprintf(buf, sizeof(buf), QCAMERA_DUMP_FRM_LOCATION"hvx_stas_right_frame%d.bin", frame_id);
    fptr = fopen(buf, "w+");
    if (!fptr) {
      ALOGE("%s:%d failed fptr %p buf %s\n", __func__, __LINE__, fptr, buf);
    } else {
      write_length = fwrite(data, 1, stats_data->stats_right_size, fptr);
      ALOGE("%s: %d: write length for frame id %d is %d", __func__, __LINE__,
            frame_id, write_length);
      fclose(fptr);
    }
  }

  ALOGI("%s:%d: X", __func__, __LINE__);
  return HVX_RET_SUCCESS;
}

static hvx_ret_type_t hvx_lib_consume_dump(void *oem_data,
  unsigned int frame_id, hvx_lib_stats_t *stats_data,
  const hvx_lib_adsp_config_t *adsp_config, void *caller_data)
{
  struct hvx_lib_private_data_t *private_data = NULL;
  hvx_ret_type_t                 ret = HVX_RET_SUCCESS;
  uint32_t                      *data = NULL;
  char                           value[PROPERTY_VALUE_MAX];
  char                           buf[100];
  FILE                          *fptr = NULL;
  uint32_t                       i = 0;
  int                            write_length;

  ALOGI("%s:%d: E", __func__, __LINE__);
  if (!oem_data || !adsp_config || !adsp_config->adsp_config_call ||
    !stats_data || !caller_data) {
    /* log error */
    ALOGE("%s:%d failed: %p %p %p %p\n", __func__, __LINE__, oem_data,
      adsp_config, stats_data, caller_data);
    return HVX_RET_FAILURE;
  }

  ALOGD("%s:%d: buffer info: left addr %p, left size %d, right addr %p, right size %d",
        __func__, __LINE__, stats_data->stats_left, stats_data->stats_left_size,
        stats_data->stats_right, stats_data->stats_right_size);

  /* Dump frame buffer */
  if (stats_data->stats_left && stats_data->stats_left_size) {
    data = (uint32_t *)stats_data->stats_left;
    snprintf(buf, sizeof(buf), QCAMERA_DUMP_FRM_LOCATION"hvx_dump_left_frame%d.bin", frame_id);
    fptr = fopen(buf, "w+");
    if (!fptr) {
      ALOGE("%s:%d failed fptr %p buf %s\n", __func__, __LINE__, fptr, buf);
    } else {
      write_length = fwrite(data, 1, stats_data->stats_left_size, fptr);
     //write_length = fwrite(data, 1, 128, fptr);
      ALOGE("%s: %d: write length for frame id %d is %d", __func__, __LINE__,
            frame_id, write_length);
      fclose(fptr);
    }
  }
  if (stats_data->stats_right && stats_data->stats_right_size) {
    data = (uint32_t *)stats_data->stats_right;
    snprintf(buf, sizeof(buf), QCAMERA_DUMP_FRM_LOCATION"hvx_dump_right_frame%d.bin", frame_id);
    fptr = fopen(buf, "w+");
    if (!fptr) {
      ALOGE("%s:%d failed fptr %p buf %s\n", __func__, __LINE__, fptr, buf);
    } else {
      write_length = fwrite(data, 1, stats_data->stats_right_size, fptr);
      //write_length = fwrite(data, 1, 128, fptr);
      ALOGE("%s: %d: write length for frame id %d is %d", __func__, __LINE__,
            frame_id, write_length);
      fclose(fptr);
    }
  }

  ALOGD("%s:%d: X", __func__, __LINE__);
  return HVX_RET_SUCCESS;
}

/** zzHDR_util_calc_interpolation_weight
 * @value: input value
 * @start: start value (min)
 * @end: end value (max)
 *
 * This function finds how far input avlue is as compared to start and end
 * points. If value is below start point it returns zero, if its beyond end
 * outout is one. Otherwise it calulates % of inputvalue against start-end range
 **/
float zzHDR_util_calc_interpolation_weight(float value,
  float start, float end)
{
  /* return value is a ratio to the start point,
    "start" point is always the smaller gain/lux one.
    thus,
    "start" could be lowlight trigger start, and bright light trigger end*/
  if (start != end) {
    if (value  <= start)
      return 0.0;
    else if (value  >= end)
      return 1.0;
    else
      return(value  - start) / (end - start);
  } else {
    ALOGD("Trigger Warning: same value %f", start);
    return 0.0;
  }
}


/** zzhdr_util_get_aec_ratio_lowlight
 *
 *    @tunning_type:
 *    @trigger_ptr:
 *    @aec_out:
 *
 *   Get trigger ratio based on lowlight trigger.
 *   Please note that ratio is the weight of the normal light.
 *    LOW           Mix            Normal
 *    ----------|----------------|----------
 *        low_end(ex: 400)    low_start(ex: 300)
 *
 **/
float zzHDR_util_get_aec_ratio_lowlight(tuning_control_type tunning_type,
  void *trigger_ptr, const hvx_lib_sof_params_t *sof_params)
{
  float normal_light_ratio = 0.0;
  float ratio_to_low_start = 0.0;
  tuning_control_type tunning = (tuning_control_type)tunning_type;

  trigger_point_type *trigger = (trigger_point_type *)trigger_ptr;

  switch (tunning) {
  /* 0 is Lux Index based */
  case 0: {
    ALOGD("%s lux_idx = %f lux_start %ld lux_end %ld ", __func__,
          sof_params->lux_idx, trigger->lux_index_start, trigger->lux_index_end);
    ratio_to_low_start = zzHDR_util_calc_interpolation_weight(
      sof_params->lux_idx, trigger->lux_index_start, trigger->lux_index_end);
    }
      break;
  /* 1 is Gain Based */
  case 1: {
    ALOGD("%s real_gain = %f gain_start %f gain_end %f ", __func__,
          sof_params->real_gain, trigger->gain_start, trigger->gain_end);
    ratio_to_low_start = zzHDR_util_calc_interpolation_weight(
      sof_params->real_gain, trigger->gain_start, trigger->gain_end);
  }
    break;

  default: {
    ALOGD("%s %d get_trigger_ratio: tunning type %d is not supported",
          __func__, __LINE__, tunning);
  }
    break;
  }

  /*ratio_to_low_start is the sitance to low start,
    the smaller distance to low start,
    the higher ratio applied normal light*/
  normal_light_ratio = 1 - ratio_to_low_start;
  ALOGD("%s ratio_to_low_start %f ", __func__, ratio_to_low_start);

  if (normal_light_ratio < 0) {
    normal_light_ratio = 0;
  } else if (normal_light_ratio > 1.0) {
    normal_light_ratio = 1.0;
  }

  return normal_light_ratio;
}

void hvx_zzHDR_copy_black_level(zzHDR_L_black_level_t *bl_input,
  CHROMATIX_LINEARIZATION_TYPE *linearisation_type)
{
 bl_input->r_lut_p = (float)linearisation_type->r_lut_p[0] * (float)(1 << 2);
 bl_input->b_lut_p = (float)linearisation_type->b_lut_p[0] * (float)(1 << 2);
 bl_input->gb_lut_p = (float)linearisation_type->gb_lut_p[0] * (float)(1 << 2);
 bl_input->gr_lut_p = (float)linearisation_type->gr_lut_p[0] * (float)(1 << 2);
}

void hvx_zzHDR_interpolate(zzHDR_L_black_level_t *bl_input1,
  zzHDR_L_black_level_t *bl_input2, zzHDR_L_black_level_t *bl_output, float aec_ratio)
{
  bl_output->r_lut_p = LINEAR_INTERPOLATION(bl_input1->r_lut_p, bl_input2->r_lut_p, aec_ratio);
  bl_output->b_lut_p = LINEAR_INTERPOLATION(bl_input1->b_lut_p, bl_input2->b_lut_p, aec_ratio);
  bl_output->gr_lut_p = LINEAR_INTERPOLATION(bl_input1->gr_lut_p, bl_input2->gr_lut_p, aec_ratio);
  bl_output->gb_lut_p = LINEAR_INTERPOLATION(bl_input1->gb_lut_p, bl_input2->gb_lut_p, aec_ratio);
}

uint32_t hvx_zzHDR_calculate_black_level(zzHDR_mod_t  *zzHDR_mod,
                     const hvx_lib_sof_params_t *sof_params)
{
  CHROMATIX_L_TYPE *chromatix_L;
  float aec_ratio = 0.0f;
  uint8_t need_update = HVX_FALSE;
  zzHDR_L_black_level_t lowlight_input;
  zzHDR_L_black_level_t normal_input;
  zzHDR_L_black_level_t output;
  uint32_t  avg_blk_level;
  uint32_t blss_blk_lvl;
  Chromatix_blk_subtract_scale_type *blss_type;

  if (!zzHDR_mod || !sof_params){
      ALOGE("%s %d failed zzHDR_mod %p sof_params %p ",
            __func__, __LINE__, zzHDR_mod, sof_params);
      return DEFAULT_BLACK_LEVEL;
  }
  avg_blk_level = zzHDR_mod->zzHDR_cfg.hdr_black_in;

  chromatix_L = &zzHDR_mod->chromatix_comm_ptr->chromatix_L;
  blss_type   = &zzHDR_mod->chromatix_comm_ptr->Chromatix_BLSS_data;

  aec_ratio = zzHDR_util_get_aec_ratio_lowlight(
    chromatix_L->control_linearization,
    (void *)&(chromatix_L->linearization_lowlight_trigger),
    sof_params);

  ALOGD("%s aec_ratio %f zzHDR_mod->aec_ratio %f \n",__func__,  zzHDR_mod->aec_ratio, aec_ratio);
  if (!F_EQUAL(zzHDR_mod->aec_ratio, aec_ratio)) {
    zzHDR_mod->aec_ratio = aec_ratio;
    need_update = HVX_TRUE;
  }
  if (need_update){
      hvx_zzHDR_copy_black_level(&lowlight_input, &chromatix_L->linear_table_lowlight);
      hvx_zzHDR_copy_black_level(&normal_input, &chromatix_L->linear_table_normal);
      hvx_zzHDR_interpolate(&normal_input,&lowlight_input,&output, zzHDR_mod->aec_ratio);
      avg_blk_level = Round((output.r_lut_p + output.b_lut_p + output.gr_lut_p + output.gb_lut_p)/4.0f);
      ALOGD("%s avg_blk_level %d \n",__func__, avg_blk_level);
      blss_blk_lvl = Round(
        LINEAR_INTERPOLATION(blss_type->black_level_normal.black_level_offset,
        blss_type->black_level_lowlight.black_level_offset , aec_ratio) * 4.0f);
      avg_blk_level += blss_blk_lvl;
      ALOGD("%s aec_ratio %f blss_blk_lvl %d avg_blk_level %d \n",__func__, aec_ratio, blss_blk_lvl, avg_blk_level);
  }

  return avg_blk_level;

}

static hvx_ret_type_t hvx_zzHDR_prepare_hw_config(zzHDR_mod_t  *zzHDR_mod,
                     const hvx_lib_sof_params_t *sof_params)
{
  hvx_ret_type_t           ret = HVX_RET_SUCCESS;
  CHROMATIX_PARAMS_TYPE *chromatix_ptr = NULL;

  CHROMATIX_VHDR_TYPE    *chromatix_VHDR = NULL;
  CHROMATIX_VHDR_CORE    *chromatix_HDR_core = NULL;
  CHROMATIX_VHDR_RESERVE *chromatix_HDR_reserve = NULL;
  zzHDR_cfg_cmd          *zzHDR_cfg;
  uint32_t               i = 0;
  trigger_point2_type    trigger_points[MAX_SETS_FOR_TONE_NOISE_ADJ];
  float                  trigger_ratio = 0.0f;
  float                  linear_ratio = 0.0f;

  if(!zzHDR_mod || !sof_params){
     ALOGE("%s:%d failed: zzHDR_mod %p sof_params %p\n", __func__, __LINE__,
     zzHDR_mod, sof_params);
   return HVX_RET_FAILURE;
  }

  chromatix_ptr = (CHROMATIX_PARAMS_TYPE *)
    sof_params->chromatix_ptr;
  if (!chromatix_ptr) {
    ALOGE("failed: chromatix_ptr %p", chromatix_ptr);
    return HVX_RET_FAILURE;
  }
  chromatix_VHDR =
    &chromatix_ptr->chromatix_post_processing.chromatix_video_HDR;

  chromatix_HDR_core = &chromatix_VHDR->videoHDR_core_data[0];
  chromatix_HDR_reserve = &chromatix_VHDR->videoHDR_reserve_data;

  zzHDR_cfg = &zzHDR_mod->zzHDR_cfg;
  /*config HDR reg*/
  zzHDR_cfg->hdr_rg_wb_gain_ratio = CLAMP_LIMIT(
    FLOAT_TO_Q(12, zzHDR_mod->wb_rg_ratio), ISP_HDR_WB_RATIO_MIN,
    ISP_HDR_WB_RATIO_MAX);
  zzHDR_cfg->hdr_bg_wb_gain_ratio = CLAMP_LIMIT(
    FLOAT_TO_Q(12, zzHDR_mod->wb_bg_ratio), ISP_HDR_WB_RATIO_MIN,
    ISP_HDR_WB_RATIO_MAX);
  zzHDR_cfg->hdr_gb_wb_gain_ratio = CLAMP_LIMIT(
    FLOAT_TO_Q(12, zzHDR_mod->wb_gb_ratio), ISP_HDR_WB_RATIO_MIN,
    ISP_HDR_WB_RATIO_MAX);
  zzHDR_cfg->hdr_gr_wb_gain_ratio = CLAMP_LIMIT(
    FLOAT_TO_Q(12, zzHDR_mod->wb_gr_ratio), ISP_HDR_WB_RATIO_MIN,
    ISP_HDR_WB_RATIO_MAX);

  zzHDR_cfg->hdr_exp_ratio = CLAMP_LIMIT(
    FLOAT_TO_Q(10, zzHDR_mod->hdr_sensitivity_ratio), ISP_HDR_EXP_RATIO_MIN,
    ISP_HDR_EXP_RATIO_MAX);
  zzHDR_cfg->hdr_exp_ratio_recip = CLAMP_LIMIT(
    FLOAT_TO_Q(8, (1.0 / zzHDR_mod->hdr_sensitivity_ratio)), ISP_HDR_EXP_RECIP_RATIO_MIN,
    ISP_HDR_EXP_RECIP_RATIO_MAX);

  if ( chromatix_VHDR->control_hdr == CONTROL_AEC_EXP_SENSITIVITY_RATIO ){
    trigger_ratio = zzHDR_mod->hdr_sensitivity_ratio;
    for(i = 0 ; i < MAX_SETS_FOR_TONE_NOISE_ADJ; i++){
        trigger_points[i] = chromatix_HDR_core[i].aec_sensitivity_ratio;
    }
  } else if (chromatix_VHDR->control_hdr == CONTROL_EXP_TIME_RATIO){
      trigger_ratio = zzHDR_mod->hdr_exp_time_ratio;
      for(i = 0 ; i < MAX_SETS_FOR_TONE_NOISE_ADJ; i++){
        trigger_points[i] = chromatix_HDR_core[i].exp_time_trigger;
    }
  }

   for (i = 0; i < MAX_SETS_FOR_TONE_NOISE_ADJ; i++) {
     if (trigger_ratio < trigger_points[i].start ||
       ((i + 1) == MAX_SETS_FOR_TONE_NOISE_ADJ)) {
      /*config HDR recon reg,
    Recon decide how to reconstruct from sensor frame to T1 and T2 frame*/

       zzHDR_cfg->hdr_rec_h_edge_th1 =
        chromatix_HDR_core[i].recon_h_edge_th1;
       zzHDR_cfg->hdr_rec_motion_th1 =
         chromatix_HDR_core[i].recon_motion_th1;
       zzHDR_cfg->hdr_rec_dark_th1 =
         chromatix_HDR_core[i].recon_dark_th1;

     /*config HDR MAC reg MAC decide how to combine T1 and T2 two
       differnt Exp image*/
       zzHDR_cfg->hdr_mac_lowlight_th1 = chromatix_HDR_core[i].mac_low_light_th1;

       zzHDR_cfg->hdr_mac_lowlight_strength =
         chromatix_HDR_core[i].mac_low_light_strength;
       zzHDR_cfg->hdr_mac_hilight_dth_log2 = chromatix_HDR_core[i].mac_high_light_dth_log2;
       zzHDR_cfg->hdr_mac_hilight_th1 = chromatix_HDR_core[i].mac_high_light_th1;

        zzHDR_cfg->hdr_zrec_prefilt_tap0 = chromatix_HDR_core[i].hdr_zrec_prefilt_tap0;
        zzHDR_cfg->hdr_zrec_g_grad_th1 = chromatix_HDR_core[i].hdr_zrec_g_grad_th1;

        zzHDR_cfg->hdr_zrec_rb_grad_th1 = chromatix_HDR_core[i].hdr_zrec_rb_grad_th1;
        break;
       } else if (trigger_ratio >= trigger_points[i].start &&
                  trigger_ratio <= trigger_points[i].end) {
         if ((trigger_points[i].end - trigger_points[i].start) > 0) {
           linear_ratio =
             (trigger_points[i].end - trigger_ratio) /
             (trigger_points[i].end - trigger_points[i].start);
         } else {
           linear_ratio = 1.0f;
         }
       zzHDR_cfg->hdr_rec_h_edge_th1 =
        LINEAR_INTERPOLATION(chromatix_HDR_core[i].recon_h_edge_th1, chromatix_HDR_core[i+1].recon_h_edge_th1,
                                   linear_ratio);
       zzHDR_cfg->hdr_rec_motion_th1 = LINEAR_INTERPOLATION(chromatix_HDR_core[i].recon_motion_th1,
         chromatix_HDR_core[i+1].recon_motion_th1, linear_ratio);
       zzHDR_cfg->hdr_rec_dark_th1 = LINEAR_INTERPOLATION(chromatix_HDR_core[i].recon_dark_th1,
         chromatix_HDR_core[i+1].recon_dark_th1, linear_ratio);

     /*config HDR MAC reg MAC decide how to combine T1 and T2 two
       differnt Exp image*/
       zzHDR_cfg->hdr_mac_lowlight_th1 = LINEAR_INTERPOLATION(chromatix_HDR_core[i].mac_low_light_th1,
                                         chromatix_HDR_core[i+1].mac_low_light_th1, linear_ratio);
       zzHDR_cfg->hdr_mac_lowlight_strength =LINEAR_INTERPOLATION (chromatix_HDR_core[i].mac_low_light_strength,
         chromatix_HDR_core[i+1].mac_low_light_strength, linear_ratio);
       zzHDR_cfg->hdr_mac_hilight_dth_log2 = LINEAR_INTERPOLATION(chromatix_HDR_core[i].mac_high_light_dth_log2,
                        chromatix_HDR_core[i+1].mac_high_light_dth_log2 ,linear_ratio);
       zzHDR_cfg->hdr_mac_hilight_th1 = LINEAR_INTERPOLATION(chromatix_HDR_core[i].mac_high_light_th1,
                                                             chromatix_HDR_core[i+1].mac_high_light_th1, linear_ratio);

        zzHDR_cfg->hdr_zrec_prefilt_tap0 = LINEAR_INTERPOLATION(chromatix_HDR_core[i].hdr_zrec_prefilt_tap0,
                                                                chromatix_HDR_core[i+1].hdr_zrec_prefilt_tap0, linear_ratio);
        zzHDR_cfg->hdr_zrec_g_grad_th1 = LINEAR_INTERPOLATION(chromatix_HDR_core[i].hdr_zrec_g_grad_th1,
                                                               chromatix_HDR_core[i+1].hdr_zrec_g_grad_th1, linear_ratio);
        zzHDR_cfg->hdr_zrec_rb_grad_th1 = LINEAR_INTERPOLATION(chromatix_HDR_core[i].hdr_zrec_rb_grad_th1,
                                                               chromatix_HDR_core[i].hdr_zrec_rb_grad_th1, linear_ratio);
        break;
       }
   }

   if (linear_ratio == 1.0f){
      zzHDR_cfg->zzhdr_linear_mode = 1;
    } else {
      zzHDR_cfg->zzhdr_linear_mode = 0;
    }

    zzHDR_cfg->hdr_black_in   = hvx_zzHDR_calculate_black_level(zzHDR_mod,sof_params);

    zzHDR_cfg->hdr_rec_h_edge_dth_log2=
      chromatix_HDR_reserve->recon_h_edge_dth_log2;
    zzHDR_cfg->hdr_rec_motion_dth_log2 =
      chromatix_HDR_reserve->recon_motion_dth_log2;
    zzHDR_cfg->hdr_rec_dark_dth_log2 =
      chromatix_HDR_reserve->recon_dark_dth_log2;


    zzHDR_cfg->hdr_mac_lowlight_dth_log2 = chromatix_HDR_reserve->mac_low_light_dth_log2;


    zzHDR_cfg->hdr_zrec_pattern = chromatix_HDR_reserve->hdr_zrec_pattern;
    zzHDR_cfg->hdr_zrec_first_rb_exp = chromatix_HDR_reserve->hdr_zrec_first_rb_exp;
    zzHDR_cfg->hdr_zrec_g_grad_dth_log2 = chromatix_HDR_reserve->hdr_zrec_g_grad_dth_log2;
    zzHDR_cfg->hdr_zrec_rb_grad_dth_log2 = chromatix_HDR_reserve->hdr_zrec_rb_grad_dth_log2;
  return ret;
}

static hvx_ret_type_t hvx_lib_get_dump_buffer_size(int output_width,
  int output_height, int right_stripe_offset, int overlap, int * dump_frame_size)
{
   int vfe0_width, vfe1_width;
   hvx_ret_type_t               ret = HVX_RET_SUCCESS;
   vfe0_width = right_stripe_offset + overlap;
   vfe1_width = output_width - right_stripe_offset;

   if (vfe0_width > vfe1_width) {
     *dump_frame_size = (((vfe0_width*2 + 16*2 + 127) >> 7) << 7) * (output_height);
   } else {
     *dump_frame_size = (((vfe1_width*2 + 16*2 + 127) >> 7) << 7) * (output_height);
   }
   return ret;
}

static hvx_ret_type_t hvx_lib_set_config(void *oem_data,
  const hvx_lib_config_t *lib_config,
  const hvx_lib_adsp_config_t *adsp_config,
  void *caller_data)
{
  hvx_ret_type_t               ret = HVX_RET_SUCCESS;
  zzHDR_mod_t                 *zzHDR_mod = NULL;
  zzHDR_cfg_cmd               *zzHDR_cfg;

  if (!oem_data || !lib_config || !adsp_config ||
    !adsp_config->adsp_config_call) {
    ALOGE("%s:%d failed: %p %p %p\n", __func__, __LINE__, oem_data,
      lib_config, adsp_config);
    return HVX_RET_FAILURE;
  }

  zzHDR_mod = (zzHDR_mod_t *)oem_data;
  zzHDR_cfg = &zzHDR_mod->zzHDR_cfg;

  zzHDR_mod->wb_rg_ratio = 1.0;
  zzHDR_mod->wb_bg_ratio = 1.0;
  zzHDR_mod->wb_gr_ratio = 1.0;
  zzHDR_mod->wb_gb_ratio = 1.0;
  zzHDR_mod->hdr_sensitivity_ratio = 1.0;
  zzHDR_mod->hdr_exp_time_ratio  = 1.0;

  memset(zzHDR_cfg, 0, sizeof(zzHDR_cfg_cmd));

  zzHDR_init_config(zzHDR_cfg);
  /* Call adsp_config to pass call to ADSP */
  ret = adsp_config->adsp_config_call(zzHDR_cfg, sizeof(zzHDR_cfg_cmd),
    caller_data);
  if (ret != HVX_RET_SUCCESS) {
    /* log error */
    ALOGE("%s:%d failed: adsp_config_call ret %d\n", __func__, __LINE__, ret);
  }

  return ret;
}

static hvx_ret_type_t hvx_lib_sof(void *oem_data,
  const hvx_lib_sof_params_t *sof_params,
  const hvx_lib_adsp_config_t *adsp_config,
  void *caller_data)
{
  hvx_ret_type_t       ret = HVX_RET_SUCCESS;
  zzHDR_mod_t         *zzHDR_mod = NULL;
  zzHDR_cfg_cmd       *zzHDR_cfg;
  char value[PROPERTY_VALUE_MAX];
  int32_t dbg_enabled = 0;

  if (!oem_data || !adsp_config || !adsp_config->adsp_config_call ||
    !sof_params ) {
    /* log error */
    ALOGE("%s:%d failed: oem_data %p adsp_config %p sof_params %p\n",
      __func__, __LINE__, oem_data, adsp_config, sof_params);
    return HVX_RET_FAILURE;
  }
  property_get("persist.vendor.camera.zzhdr.debug", value, "0");
  dbg_enabled = atoi(value);
  if (dbg_enabled) {
    ALOGE("%s sof_params exp_time ratio %f sensitivity ratio %f ",
      __func__, sof_params->hdr_exp_time_ratio, sof_params->hdr_sensitivity_ratio);
  }

  zzHDR_mod = (zzHDR_mod_t *)oem_data;

  if (!F_EQUAL(sof_params->g_gain , 0.0) &&
         !F_EQUAL(sof_params->r_gain , 0.0) &&
         !F_EQUAL(sof_params->b_gain , 0.0)) {
       zzHDR_mod->wb_rg_ratio =
         sof_params->r_gain / sof_params->g_gain;
       zzHDR_mod->wb_bg_ratio =
         sof_params->b_gain / sof_params->g_gain;
       zzHDR_mod->wb_gr_ratio =
         sof_params->g_gain / sof_params->r_gain;
       zzHDR_mod->wb_gb_ratio =
         sof_params->g_gain / sof_params->b_gain;
     } else {
       zzHDR_mod->wb_rg_ratio = 1.0;
       zzHDR_mod->wb_bg_ratio = 1.0;
       zzHDR_mod->wb_gr_ratio = 1.0;
       zzHDR_mod->wb_gb_ratio = 1.0;
     }

  if (!F_EQUAL(sof_params->hdr_sensitivity_ratio, 0.0)) {
     zzHDR_mod->hdr_sensitivity_ratio = sof_params->hdr_sensitivity_ratio;
  } else {
    zzHDR_mod->hdr_sensitivity_ratio = 1.0;
  }
  if (!F_EQUAL(sof_params->hdr_exp_time_ratio, 0.0)) {
     zzHDR_mod->hdr_exp_time_ratio = sof_params->hdr_exp_time_ratio;
  } else {
    zzHDR_mod->hdr_exp_time_ratio = 1.0;
  }
  zzHDR_mod->analog_gain = sof_params->real_gain;
  zzHDR_mod->chromatix_ptr = sof_params->chromatix_ptr;
  zzHDR_mod->chromatix_comm_ptr = sof_params->chromatix_comm_ptr;
  zzHDR_cfg = &(zzHDR_mod->zzHDR_cfg);
  hvx_zzHDR_prepare_hw_config(zzHDR_mod,sof_params);

  /* Since HVX  gets SOF ahead of CTRL SOF*/
  zzHDR_cfg->frame_id = sof_params->frame_id+1;

  if(dbg_enabled)
    zzHDR_print_cfg(zzHDR_cfg);

  /* Call adsp_config to pass call to ADSP */
  ret = adsp_config->adsp_config_call(zzHDR_cfg, sizeof(zzHDR_cfg_cmd),
    caller_data);
  if (ret != HVX_RET_SUCCESS) {
    /* log error */
    ALOGE("%s:%d failed: adsp_config_call ret %d\n", __func__, __LINE__, ret);
  }

  return HVX_RET_SUCCESS;
}

static hvx_ret_type_t hvx_lib_close(void *oem_data)
{
  if (!oem_data) {
     /* log error */
     ALOGE("%s:%d failed: hvx_lib_close oem_data %p\n", __func__, __LINE__,
       oem_data);
     return HVX_RET_FAILURE;
  }

  free(oem_data);

  return HVX_RET_SUCCESS;
}

hvx_ret_type_t hvx_lib_fill_function_table(
  hvx_lib_function_table_t *func_table)
{
  ILOG("");
  if (!func_table) {
    ALOGE("%s:%d failed func_table %p\n", __func__, __LINE__, func_table);
    return HVX_RET_FAILURE;
  }

  func_table->hvx_lib_open = hvx_lib_open;
  func_table->hvx_lib_get_hvx_info = hvx_lib_get_hvx_info;
  func_table->hvx_lib_set_config = hvx_lib_set_config;
  func_table->hvx_lib_consume_stats = hvx_lib_consume_stats;
  func_table->hvx_lib_consume_dump = hvx_lib_consume_dump;
  func_table->hvx_lib_sof = hvx_lib_sof;
  func_table->hvx_lib_close = hvx_lib_close;
  func_table->hvx_lib_get_dump_buffer_size =  hvx_lib_get_dump_buffer_size;

  return HVX_RET_SUCCESS;
}
