/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __COLOR_XFORM46_H__
#define __COLOR_XFORM46_H__

#include "color_xform_reg.h"
#include "isp_sub_module_common.h"

/* TODO pass from Android.mk */
#define COLOR_XFORM46_VERSION "46"

#define COLOR_XFORM46_VIEWFINDER_NAME(n) \
  "color_xform_viewfinder"n

#define COLOR_XFORM46_ENCODER_NAME(n) \
  "color_xform_encoder"n

#define COLOR_XFORM46_VIDEO_NAME(n) \
  "color_xform_video"n

#define COLOR_XFORM_SVALUE_THRESHOLD 1.25f

typedef enum color_xform_type {
  XFORM_601_601,      /*None. ITU601(0-255)*/
  XFORM_601_601_SDTV, /*ITU601 (16-235)*/
  XFORM_601_709,      /*ITU709 */
  XFORM_MAX,          /*No color transform*/
} color_xform_type_t;

typedef enum {
  ISP_COLOR_XFORM_ENCODER = 0,
  ISP_COLOR_XFORM_VIEWFINDER = 1,
  ISP_COLOR_XFORM_VIDEO = 2,
  ISP_COLOR_XFORM_MAX_NUM = 3
} isp_color_xform_type;

typedef struct {
  uint32_t                  streaming_mode_mask;
  ISP_ColorXformCfgCmdType  reg_cmd;
  uint32_t                  reg_offset;
  uint32_t                  reg_len;
  isp_color_xform_type      color_xform_path;
  boolean                   streaming_mode_burst;
  float                     scaling_factor;
} color_xform46_t;

boolean color_xform46_init(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);

void color_xform46_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);

void color_xform46_update_streaming_mode_mask(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, uint32_t streaming_mode_mask);

boolean color_xform46_scaler_output_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean color_xform46_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
#endif// __COLOR_XFORM46_H__
