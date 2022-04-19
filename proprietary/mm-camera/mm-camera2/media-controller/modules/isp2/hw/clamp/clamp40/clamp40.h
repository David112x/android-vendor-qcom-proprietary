/*============================================================================

  Copyright (c) 2013-2016 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __CLAMP40_H__
#define __CLAMP40_H__

#include "clamp_reg.h"
#include "isp_sub_module_common.h"

/* TODO pass from Android.mk */
#define CLAMP40_VERSION "40"

#define CLAMP40_VIEWFINDER_NAME(n) \
  "clamp_viewfinder"n

#define CLAMP40_ENCODER_NAME(n) \
  "clamp_encoder"n

#define CLAMP40_VIDEO_NAME(n) \
  "clamp_video"n

typedef enum {
  ISP_CLAMP_ENCODER = 0,
  ISP_CLAMP_VIEWFINDER = 1,
  ISP_CLAMP_VIDEO = 2,
  ISP_CLAMP_MAX_NUM = 3
} isp_clamp_type;

typedef struct {
  ISP_OutputClampConfigCmdType reg_cmd;
  uint8_t hw_update_pending;
}isp_clamp_entry_t;

typedef struct {
  isp_clamp_type    clamp_type;
  isp_clamp_entry_t clamp;
  cam_format_t      fmt;
  uint32_t          reg_offset;
  uint32_t          reg_len;
} clamp40_t;

boolean clamp40_init(mct_module_t *module, isp_sub_module_t *isp_sub_module);

void clamp40_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module);

boolean clamp40_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean clamp40_set_stream_config(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

#endif// __CLAMP40_H__
