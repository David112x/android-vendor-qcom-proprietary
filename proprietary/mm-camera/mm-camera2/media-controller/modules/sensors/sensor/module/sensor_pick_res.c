/* sensor_pick_res.c
 *
 * Copyright (c) 2014-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "math.h"
#include "sensor_util.h"
#include "sensor_pick_res.h"

#define QFACTOR 100
#define MAX_FPS_MAX_VARIANCE 29.0f
#define DEFAULT_DEPTH_SENSOR_MODE 0

const uint32_t pick_v1[SEN_COND_MAX][SEN_USECASE_MAX] = {
  /*
   *  For sensor pick resolution we can use the below table.
   *  This table determines the sensor mode, pick scenario based on
   *  camera usecase. Per usecase we have a different scenarios.
   *  For example if we have to skip the frame rate condition for
   *  Snapshot usecase we need to put value 0 for the FPS
   *  pick_v1[0][4] = 0.
   */
  /*FAST AEC  QUAD  HFR   IHDR   RHDR VHDR Snapshot Video/P *//*check for 1*/
  {    1,      0,    1,    0,    0,   1,     0,      1    }, /* FPS*/
  {    0,      0,    1,    0,    0,   0,     1,      1    }, /* Bounded FPS*/
  {    0,      1,    0,    1,    1,   0,     1,      1    }, /* Aspect ratio*/
  {    0,      1,    0,    1,    1,   1,     1,      1    }, /* Resolution w*/
  {    0,      1,    0,    1,    1,   0,     1,      1    }, /* Resolution h*/
  {    1,      0,    1,    1,    1,   1,     1,      1    }, /* Mode Pix Clk */
  {    0,      1,    0,    0,    0,   0,     0,      0    }, /* mod QUADRA */
  {    0,      0,    1,    0,    0,   0,     0,      0    }, /* mod HFR*/
  {    0,      0,    0,    0,    0,   0,     1,      1    }, /* mod DEF*/
  {    0,      0,    0,    1,    0,   1,     0,      0    }, /* mod IHDR*/
  {    0,      0,    0,    0,    1,   0,     0,      0    }, /* mod RHDR*/
  {    1,      0,    1,    0,    0,   1,     1,      0    }, /* mod Max Res*/
  {    0,      0,    0,    1,    1,   0,     0,      1    }, /* mod Best Res*/
};

const uint32_t pick_v3[SEN_COND_MAX][SEN_USECASE_MAX] = {
  /*FAST AEC  QUAD HFR   IHDR  RHDR VHDR Snapshot   Video/P  *//*check for 1*/
  [SEN_COND_FPS] =
  {    1,      0,  1,    0,   0,    1,    0,        1  }, /* FPS*/
  [SEN_COND_BOUNDED_FPS] =
  {    0,      0,  1,    0,   0,    0,    1,        1  }, /* Bounded FPS*/
  [SEN_COND_ASPR] =
  {    0,      1,  0,    1,   1,    0,    1,        1  }, /* Aspect ratio */
  [SEN_COND_W] =
  {    0,      1,  0,    1,   1,    1,    1,        1  }, /* Resolution w*/
  [SEN_COND_H] =
  {    0,      1,  0,    1,   1,    0,    1,        1  }, /* Resolution h*/
  [SEN_COND_MODE_HFR] =
  {    0,      0,  1,    0,   0,    0,    0,        0  }, /* mod HFR */
  [SEN_COND_MODE_DEF] =
  {    0,      0,  0,    0,   0,    0,    1,        1  }, /* mod DEF*/
  [SEN_COND_MODE_IHDR] =
  {    0,      0,  0,    1,   0,    1,    0,        0  }, /* mod IHDR*/
  [SEN_COND_MODE_RHDR] =
  {    0,      0,  0,    0,   1,    0,    0,        0  }, /* mod RHDR*/
  [SEN_COND_MODE_MPIX] =
  {    1,      0,  1,    0,   0,    1,    1,        0  }, /* mod Max Res*/
  [SEN_COND_MODE_BEST_RES] =
  {    0,      0,  0,    1,   1,    0,    0,        1  }, /* mod Best Res*/
  [SEN_COND_MODE_QUADRA] =
  {    0,      1,  0,    0,   0,    0,    0,        0  }, /* mod QUADRA */
};

const is_usecase_func_t func_v1 = {
    [SEN_USECASE_FASTAEC] = sensor_pick_res_is_fast_aec,
    [SEN_USECASE_QUADRA] = sensor_pick_res_is_quadra,
    [SEN_USECASE_HFR] = sensor_pick_res_is_hfr,
    [SEN_USECASE_IHDR] = sensor_pick_res_is_ihdr,
    [SEN_USECASE_RHDR] = sensor_pick_res_is_rhdr,
    [SEN_USECASE_VHDR] = sensor_pick_res_is_vhdr,
    [SEN_USECASE_VP] = sensor_pick_res_is_video_preview,
    [SEN_USECASE_SNAP] = sensor_pick_res_is_snapshot
};

const check_func_t check_v1 = {
    [SEN_COND_FPS] = sensor_pick_check_fps,
    [SEN_COND_BOUNDED_FPS] = sensor_pick_check_bounded_fps,
    [SEN_COND_ASPR] = sensor_pick_check_aspect_ratio,
    [SEN_COND_W] = sensor_pick_check_width,
    [SEN_COND_H] = sensor_pick_check_height,
    [SEN_COND_CLK] = sensor_pick_check_pixclk,
    [SEN_COND_MODE_QUADRA] = sensor_pick_check_mode,
    [SEN_COND_MODE_HFR] = sensor_pick_check_mode,
    [SEN_COND_MODE_DEF] = sensor_pick_check_mode,
    [SEN_COND_MODE_IHDR] = sensor_pick_check_mode,
    [SEN_COND_MODE_RHDR] = sensor_pick_check_mode,
    [SEN_COND_MODE_MPIX] = sensor_pick_check_mpix,
    [SEN_COND_MODE_BEST_RES] = sensor_pick_check_best_res
};

/** sensor_pick_check_mode: checking input mode
 *
 *  @pick_data: handle to sensor pick structure
 *
 *  This function check input mode based on requested mode
 *
 *  Return: TRUE for success
 *          FALSE error for failure
 **/
boolean sensor_pick_check_mode(sensor_pick_data_t *pick_data)
{
  SLOW("check mode: in %d, exp %d", pick_data->in_mode, pick_data->ex_mode);
  return (pick_data->in_mode & pick_data->ex_mode);
}

/** sensor_pick_check_pixclk: checking input sensor pixel clk
 *
 *  @pick_data: handle to sensor pick structure
 *
 *  This function check input pixel clock based on requested clock
 *
 *  Return: TRUE for success
 *          FALSE error for failure
 **/
boolean sensor_pick_check_pixclk(sensor_pick_data_t *pick_data)
{
  SLOW("check pixclk: %d", 1);
  return (pick_data->ex_clk >= pick_data->in_clk) || !(pick_data->ex_clk);
}

/** sensor_pick_check_height: checking input sensor height dimention
 *
 *  @pick_data: handle to sensor pick structure
 *
 *  This function check input height based on requested height
 *
 *  Return: TRUE for success
 *          FALSE error for failure
 **/
boolean sensor_pick_check_height(sensor_pick_data_t *pick_data)
{
  SLOW("check h: %d", (pick_data->in_h >= pick_data->ex_h));
  return (pick_data->in_h >= pick_data->ex_h);
}

/** sensor_pick_check_width: checking input sensor width dimention
 *
 *  @pick_data: handle to sensor pick structure
 *
 *  This function check input width based on requested width
 *
 *  Return: TRUE for success
 *          FALSE error for failure
 **/
boolean sensor_pick_check_width(sensor_pick_data_t *pick_data)
{
  SLOW("check w: %d", (pick_data->in_w >= pick_data->ex_w));
  return (pick_data->in_w >= pick_data->ex_w);
}

/** sensor_pick_check_width: checking input sensor resolution in mega pixels
 *
 *  @pick_data: handle to sensor pick structure
 *
 *  This function check input mega pixels based on requested mega pixels
 *
 *  Return: TRUE for success
 *          FALSE error for failure
 **/
boolean sensor_pick_check_mpix(sensor_pick_data_t *pick_data)
{
  uint32_t mpix = pick_data->in_w * pick_data->in_h;

  pick_data->tmp_mpix = mpix;

  SLOW("check mpix: %d", (mpix >= pick_data->last_mpix));
  return (mpix >= pick_data->last_mpix);
}

/** sensor_pick_check_best_res: checking input sensor resolution for best res
 *
 *  @pick_data: handle to sensor pick structure
 *
 *  This function check input resolution based on requested mega pixels
 *
 *  Return: TRUE for success
 *          FALSE error for failure
 **/
boolean sensor_pick_check_best_res(sensor_pick_data_t *pick_data)
{
  uint32_t mpix = pick_data->in_w * pick_data->in_h;
  uint32_t ex_res = pick_data->ex_w * pick_data->ex_h;

  pick_data->tmp_mpix = mpix;

  if (!pick_data->last_mpix){
    pick_data->last_mpix = mpix;
  }

  SLOW("check mpix: %d: %d: mpix:%d, Last:%d, ex_res:%d",
    (mpix <= pick_data->last_mpix),
    (mpix >= ex_res), mpix, pick_data->last_mpix, ex_res);

  return ((mpix <= pick_data->last_mpix) && (mpix >= ex_res));
}

/** sensor_pick_check_aspect_ratio: checking input sensor aspect ratio
 *
 *  @pick_data: handle to sensor pick structure
 *
 *  This function check input aspect ratio based on requested aspect ratio
 *
 *  Return: TRUE for success
 *          FALSE error for failure
 **/
boolean sensor_pick_check_aspect_ratio(sensor_pick_data_t *pick_data)
{
  uint32_t asp_ratio = 0;
  uint32_t last_ar = pick_data->last_ar;

  asp_ratio = abs(pick_data->ex_ar - pick_data->in_ar);

  pick_data->tmp_ar = asp_ratio;

  SLOW("check asp: %d, asp_ratio: %d", (asp_ratio < last_ar), asp_ratio);

  /* retriger resolution check */
  if (asp_ratio < last_ar)
    pick_data->last_mpix = 0;

  return (asp_ratio < last_ar);
}

/** sensor_pick_check_fps: checking input sensor frame rate
 *
 *  @pick_data: handle to sensor pick structure
 *
 *  This function check input frame rate based on requested frame rate
 *
 *  Return: TRUE for success
 *          FALSE error for failure
 **/
boolean sensor_pick_check_fps(sensor_pick_data_t *pick_data)
{
  SLOW("check fps: %d",
    (pick_data->in_fps >= pick_data->ex_fps));

  if (pick_data->ex_fps) {
    return (pick_data->in_fps >= pick_data->ex_fps);
  } else {
    return TRUE;
  }
}

/** sensor_pick_check_bounded_fps: checking input sensor frame rate
 *
 *  @pick_data: handle to sensor pick structure
 *
 *  This function check input frame rate based on requested frame rate
 *
 *  Return: TRUE for success
 *          FALSE error for failure
 **/
boolean sensor_pick_check_bounded_fps(sensor_pick_data_t *pick_data)
{
  SLOW("check fps: %d",
    (pick_data->in_fps >= pick_data->ex_fps) &&
   pick_data->in_fps < (pick_data->ex_fps+MAX_FPS_MAX_VARIANCE));

  if (pick_data->ex_fps) {
    return (pick_data->in_fps >= pick_data->ex_fps) &&
     (pick_data->in_fps < (pick_data->ex_fps+MAX_FPS_MAX_VARIANCE));
  } else {
    return TRUE;
  }
}

/** sensor_pick_res_is_fast_aec: checking for FAST AEC usecase
 *
 *  @pick_usecase: handle to sensor pick structure
 *
 *  This function return true if FAST AEC usecase is enabled
 *
 *  Return: TRUE for success
 *          FALSE error for failure
 **/
boolean sensor_pick_res_is_fast_aec(
  __attribute__((unused)) sensor_pick_usecase_t *pick_usecase)
{
  SLOW("Enter");
  return (pick_usecase->is_fast_aec_mode_on == TRUE);
}

/** sensor_pick_res_is_hfr: checking for HFR usecase
 *
 *  @pick_usecase:pointer to sensor pick usecase structure
 *
 *  This function return true if HFR usecase is enabled
 *
 *  Return: TRUE for success
 *          FALSE error for failure
 **/
boolean sensor_pick_res_is_hfr(sensor_pick_usecase_t *pick_usecase)
{
    return ((pick_usecase->hfr_mode != CAM_HFR_MODE_OFF) &&
      (pick_usecase->stream_mask & (1 << CAM_STREAM_TYPE_VIDEO)));
}

/** sensor_pick_res_is_ihdr: checking for in-sensor HDR usecase
 *
 *  @pick_usecase:pointer to sensor pick usecase structure
 *
 *  This function return true if SHDR usecase is enabled
 *
 *  Return: TRUE for success
 *          FALSE error for failure
 **/
boolean sensor_pick_res_is_ihdr(sensor_pick_usecase_t *pick_usecase)
{
    return (pick_usecase->in_sensor_hdr_enable) &&
      (pick_usecase->stream_mask & (1 << CAM_STREAM_TYPE_PREVIEW) ||
      pick_usecase->stream_mask & (1 << CAM_STREAM_TYPE_SNAPSHOT)) &&
      !(pick_usecase->stream_mask & (1 << CAM_STREAM_TYPE_VIDEO));
}

/** sensor_pick_res_is_vhdr: checking for in-sensor video HDR usecase
 *
 *  @pick_usecase:pointer to sensor pick usecase structure
 *
 *  This function return true if VHDR usecase is enabled
 *
 *  Return: TRUE for success
 *          FALSE error for failure
 **/
boolean sensor_pick_res_is_vhdr(sensor_pick_usecase_t *pick_usecase)
{
    return (pick_usecase->in_sensor_hdr_enable) &&
      pick_usecase->stream_mask & (1 << CAM_STREAM_TYPE_VIDEO);
}

/** sensor_pick_res_is_rhdr: checking for RAW HDR usecase
 *
 *  @pick_usecase:pointer to sensor pick usecase structure
 *
 *  This function return true if RAW HDR usecase is enabled
 *
 *  Return: TRUE for success
 *          FALSE error for failure
 **/
boolean sensor_pick_res_is_rhdr(sensor_pick_usecase_t *pick_usecase)
{
    return (pick_usecase->raw_hdr_enable) &&
      (pick_usecase->stream_mask & (1 << CAM_STREAM_TYPE_PREVIEW) ||
      pick_usecase->stream_mask & (1 << CAM_STREAM_TYPE_SNAPSHOT) ||
      pick_usecase->stream_mask & (1 << CAM_STREAM_TYPE_VIDEO));
}


/** sensor_pick_res_is_video_preview: checking for video preview usecase
 *
 *  @pick_usecase:pointer to sensor pick usecase structure
 *
 *  This function return true if video preview usecase is enabled
 *
 *  Return: TRUE for success
 *          FALSE error for failure
 **/
boolean sensor_pick_res_is_video_preview(
  __attribute__((unused)) sensor_pick_usecase_t *pick_usecase)
{
  /*default use case*/
  return TRUE;
}

/** sensor_pick_res_is_snapshot: checking for snapshot usecase
 *
 *  @pick_usecase:pointer to sensor pick usecase structure
 *
 *  This function return true if snapshot usecase is enabled
 *
 *  Return: TRUE for success
 *          FALSE error for failure
 **/
boolean sensor_pick_res_is_snapshot(sensor_pick_usecase_t *pick_usecase)
{
  return (pick_usecase->stream_mask & (1 << CAM_STREAM_TYPE_SNAPSHOT) &&
    !(pick_usecase->stream_mask & (1 << CAM_STREAM_TYPE_VIDEO)) &&
    !(pick_usecase->stream_mask & (1 << CAM_STREAM_TYPE_PREVIEW)));
}

/** sensor_pick_res_is_quadra: checking for Quadra CFA quadra mode usecase
 *
 *  @pick_usecase:pointer to sensor pick usecase structure
 *
 *  This function return true if quadra usecase is enabled
 *
 *  Return: TRUE for success
 *          FALSE error for failure
 **/
boolean sensor_pick_res_is_quadra(sensor_pick_usecase_t *pick_usecase)
{
    return (pick_usecase->is_quadra_mode) &&
      pick_usecase->stream_mask & (1 << CAM_STREAM_TYPE_SNAPSHOT);
}

/** sensor_pick_get_mode: checking for sensor mode
 *
 *  @usecase: sensor pick usecase
 *
 *  This function return snesor mode based on pick usecase
 *
 *  Return: snesor mode
 **/
uint16_t sensor_pick_get_mode(sensor_usecase_t usecase)
{
  if (usecase == SEN_USECASE_FASTAEC)
    return SENSOR_DEFAULT_MODE;
  else if (usecase == SEN_USECASE_HFR)
    return SENSOR_HFR_MODE;
  else if(usecase == SEN_USECASE_IHDR)
    return SENSOR_HDR_MODE;
  else if(usecase == SEN_USECASE_RHDR)
    return SENSOR_RAW_HDR_MODE;
  else if(usecase == SEN_USECASE_VHDR)
    return SENSOR_HDR_MODE;
  else if(usecase == SEN_USECASE_SNAP)
    return SENSOR_DEFAULT_MODE;
  else if(usecase == SEN_USECASE_VP)
    return SENSOR_DEFAULT_MODE;
  else if(usecase == SEN_USECASE_QUADRA)
    return SENSOR_QUADRA_MODE;
  else
    return SENSOR_DEFAULT_MODE;
}

static void sensor_pick_fill_input_data(sensor_pick_data_t *pick_data,
  struct sensor_lib_out_info_t *in_info)
{
  pick_data->in_w = in_info->x_output;
  pick_data->in_h = in_info->y_output;
  pick_data->in_fps = in_info->max_fps;
  pick_data->in_clk = in_info->op_pixel_clk;
  pick_data->in_mode = in_info->mode;
  pick_data->in_ar = (pick_data->in_w*QFACTOR)/pick_data->in_h;

  SLOW("in_w:%d, in_h:%d,in_fps:%f, in_clk:%d, in_mode:%d, in_ar: %d",
    pick_data->in_w,
    pick_data->in_h,
    pick_data->in_fps,
    pick_data->in_clk,
    pick_data->in_mode,
    pick_data->in_ar);
}

static void sensor_pick_fill_expected_data(void *sctrl)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_pick_dev_t *sensor_pick = &ctrl->s_data->sensor_pick;
  sensor_pick_data_t *pick_data = &sensor_pick->pick_data;
  sensor_pick_usecase_t *pick_usecase = &sensor_pick->pick_usecase;
  sensor_set_res_cfg_t *res_cfg = &sensor_pick->res_cfg;

  pick_data->ex_w = res_cfg->width;
  pick_data->ex_h = res_cfg->height;
  pick_data->ex_mode = sensor_pick_get_mode(sensor_pick->usecase);
  pick_data->ex_fps = ctrl->s_data->cur_fps;
  pick_data->ex_clk = ctrl->s_data->isp_pixel_clk_max;
  pick_data->ex_ar = res_cfg->aspect_r;
  pick_data->tmp_ar = 0;
  pick_data->tmp_mpix = 0;
  pick_data->last_ar = 1000;
  pick_data->last_mpix = 0;

  if (pick_usecase->is_fast_aec_mode_on)
    pick_data->ex_fps = sensor_get_hfr_mode_fps(res_cfg->fast_aec_sensor_mode);

  SLOW("ex_w:%d, ex_h:%d,ex_mode:%d, ex_fps:%f, ex_clk:%d, ex_ar: %d",
    pick_data->ex_w,
    pick_data->ex_h,
    pick_data->ex_mode,
    pick_data->ex_fps,
    pick_data->ex_clk,
    pick_data->ex_ar);
}

static void sensor_pick_fill_usecase_data(void *sctrl)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_pick_dev_t *sensor_pick = &ctrl->s_data->sensor_pick;
  sensor_pick_usecase_t *pick_usecase = &sensor_pick->pick_usecase;
  sensor_set_res_cfg_t *res_cfg = &sensor_pick->res_cfg;

  pick_usecase->stream_mask = res_cfg->stream_mask;
  pick_usecase->hfr_mode = ctrl->s_data->hfr_mode;
  pick_usecase->is_fast_aec_mode_on = res_cfg->is_fast_aec_mode_on;
  pick_usecase->is_quadra_mode = ctrl->s_data->is_quadra_mode;

  switch(ctrl->s_data->hdr_mode) {
    case CAM_SENSOR_HDR_IN_SENSOR:
    case CAM_SENSOR_3EXP_HDR_IN_SENSOR:
      SHIGH("In-sensor HDR enabled");
      pick_usecase->in_sensor_hdr_enable = 1;
      pick_usecase->raw_hdr_enable = 0;
      break;
    case CAM_SENSOR_HDR_ZIGZAG:
    case CAM_SENSOR_HDR_STAGGERED:
      SHIGH("RAW HDR enabled");
      pick_usecase->in_sensor_hdr_enable = 0;
      pick_usecase->raw_hdr_enable = 1;
      break;
    default:
      pick_usecase->in_sensor_hdr_enable = 0;
      pick_usecase->raw_hdr_enable = 0;
      break;
  }
}

static sensor_usecase_t sensor_pick_get_usecase(sensor_pick_dev_t *sensor_pick)
{
  int32_t i;

  for (i = 0; i < SEN_USECASE_MAX; i++) {
    if (TRUE == (*sensor_pick->func)[i](&sensor_pick->pick_usecase))
      return (sensor_usecase_t)i;
  }
  return SEN_USECASE_DEFAULT;
}

static void sensor_pick_get_method_by_hal(void *sctrl,
  sensor_pick_dev_t *sensor_pick)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;

  switch(ctrl->s_data->hal_version){
  case CAM_HAL_V1:
    sensor_pick->pick = (void*)&pick_v1[0][0];
    break;
  case CAM_HAL_V3:
    sensor_pick->pick = (void*)&pick_v3[0][0];
    break;
  default:
    sensor_pick->pick = (void*)&pick_v1[0][0];
    SERR("Invalid HAL version (can use default settings) : %d",
      ctrl->s_data->hal_version);
    break;
  }
  return;
}

static int32_t sensor_pick_fill_data(void *sctrl, sensor_set_res_cfg_t *res_cfg)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_pick_dev_t *sensor_pick = &ctrl->s_data->sensor_pick;
  struct sensor_lib_out_info_array *out_info_array =
    &ctrl->lib_params->sensor_lib_ptr->out_info_array;

  sensor_pick->res_cfg = *res_cfg;
  sensor_pick->func = (is_usecase_func_t *)func_v1;
  sensor_pick->check = (check_func_t *)check_v1;
  sensor_pick_get_method_by_hal(sctrl, sensor_pick);
  sensor_pick->mode_array_size = out_info_array->size;
  SLOW("mode_array_size : %d", sensor_pick->mode_array_size);

  sensor_pick_fill_usecase_data(sctrl);
  sensor_pick->usecase = sensor_pick_get_usecase(sensor_pick);
  SLOW("usecase : %d", sensor_pick->usecase);

  sensor_pick_fill_expected_data(sctrl);

  return 0;
}


static int32_t sensor_pick_res_idx(sensor_pick_dev_t *sensor_pick)
{
  int32_t i;
  boolean ret = 0;

  for(i = 0; i<SEN_COND_MAX; i++){
    if ((*sensor_pick->pick)[i][sensor_pick->usecase]) {
      ret = (*sensor_pick->check)[i](&sensor_pick->pick_data);
      if (!ret) {
        SLOW("Can't match in_w:%d, in_h:%d,in_fps:%f, in_clk:%d, in_mode:%d",
          sensor_pick->pick_data.in_w,
          sensor_pick->pick_data.in_h,
          sensor_pick->pick_data.in_fps,
          sensor_pick->pick_data.in_clk,
          sensor_pick->pick_data.in_mode);
        break;
      }
    }
  }
  return ret;
}

static inline void sensor_pick_restore_tmp(sensor_pick_data_t *pick_data)
{
  pick_data->last_mpix = pick_data->tmp_mpix;
  pick_data->last_ar = pick_data->tmp_ar;
}

static int32_t sensor_pick_validate_data(sensor_pick_dev_t *sensor_pick)
{
  /* Validate usecase */
  if (sensor_pick->usecase >= SEN_USECASE_MAX) {
    SERR("failed pick usecase : %d", sensor_pick->usecase);
    return -EINVAL;
  }

  /* Validate input mode array sensor modes */
  if (!sensor_pick->mode_array_size) {
    SERR("failed sensor mode array : %d", sensor_pick->mode_array_size);
    return -EINVAL;
  }

  if (((*sensor_pick->pick)[SEN_COND_MODE_MPIX][sensor_pick->usecase]) &&
    ((*sensor_pick->pick)[SEN_COND_MODE_BEST_RES][sensor_pick->usecase])) {
    SERR("failed pick resolution mode");
    return -EINVAL;
  }
  return 0;
}

/** sensor_pick_resolution: get output dimension for sensor
 *
 *  @sctrl: handle to sensor control structure
 *  @res_cfg: handle to sensor_set_res_cfg_t
 *  @pick_res: handle to proper resolution
 *
 *  This function stores picks proper resolution based on mode
 *  and requested dimension
 *
 *  Return: SENSOR_SUCCESS for success
 *          Negative error for failure
 **/
int32_t sensor_pick_resolution(void *sctrl,
  sensor_set_res_cfg_t *res_cfg, int32_t *pick_res)
{
  int32_t i, res_idx = 0;
  boolean ret = FALSE;
  sensor_ctrl_t *ctrl;
  sensor_pick_dev_t *sensor_pick;
  struct sensor_lib_out_info_array *out_info_array;
  struct sensor_lib_out_info_t *out_info;
  sensor_pick_data_t *pick_data;

  RETURN_ERR_ON_NULL(sctrl, -EINVAL);
  RETURN_ERR_ON_NULL(res_cfg, -EINVAL);
  RETURN_ERR_ON_NULL(pick_res, -EINVAL);

  ctrl = (sensor_ctrl_t *)sctrl;
  sensor_pick = &ctrl->s_data->sensor_pick;

  RETURN_ERR_ON_NULL(ctrl->lib_params, -EINVAL);
  RETURN_ERR_ON_NULL(ctrl->lib_params->sensor_lib_ptr, -EINVAL);

  out_info_array = &ctrl->lib_params->sensor_lib_ptr->out_info_array;

  /*We have only one mode for depth sensors as of now.
    Will refine the logic when multiple modes are introduced.
    Even the selection criteria has to be modified then.*/
  if (res_cfg->stream_mask & (1 << CAM_STREAM_TYPE_DEPTH)) {
    *pick_res = DEFAULT_DEPTH_SENSOR_MODE;
    goto pick_res_done;
  }

  pick_data = &sensor_pick->pick_data;

  ret = sensor_pick_fill_data(sctrl, res_cfg);
  if (ret < 0) {
    SERR("failed");
    return -EINVAL;
  }

  ret = sensor_pick_validate_data(sensor_pick);
  if (ret < 0) {
    SERR("failed");
    return -EINVAL;
  }

  for (i = sensor_pick->mode_array_size - 1; i >= 0; i--) {
    out_info = &(out_info_array->out_info[i]);
    /* Update dimention for current sensor mode */
    sensor_pick_fill_input_data(&sensor_pick->pick_data, out_info);
    /* Try to match user input parameters with sensor mode  */
    ret = sensor_pick_res_idx(sensor_pick);
    if (ret) {
      sensor_pick_restore_tmp(pick_data);
      res_idx = i;
      SLOW("tmp res_idx: %d", res_idx);
      SLOW("Matched in_w:%d, in_h:%d,in_fps:%f, in_clk:%d, in_mode:%d",
        sensor_pick->pick_data.in_w,
        sensor_pick->pick_data.in_h,
        sensor_pick->pick_data.in_fps,
        sensor_pick->pick_data.in_clk,
        sensor_pick->pick_data.in_mode);
    }
    SLOW("===========================================================");
  }

  *pick_res = res_idx;

pick_res_done:
  SHIGH("res_idx: %d", res_idx);
  SLOW("Matched pick_w:%d, pick_h:%d, pick_fps:%f, pick_clk:%d, pick_mode:%d",
    out_info_array->out_info[res_idx].x_output,
    out_info_array->out_info[res_idx].y_output,
    out_info_array->out_info[res_idx].max_fps,
    out_info_array->out_info[res_idx].op_pixel_clk,
    out_info_array->out_info[res_idx].mode);

  return 0;
}
