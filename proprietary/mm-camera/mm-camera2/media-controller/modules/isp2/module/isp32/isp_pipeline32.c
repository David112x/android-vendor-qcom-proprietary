
/* isp_pipeline32.c
 *
 * Copyright (c) 2012-2013, 2017 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <stdio.h>

/* isp headers */
#include "isp_pipeline.h"
#include "isp_defs.h"
#include "isp_log.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_COMMON, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_COMMON, fmt, ##args)

#define ISP_MAX_APPLYING_DELAY               2

static isp_hw_info_t common_hw[] = {
  {{"linearization"},    ISP_MOD_LINEARIZATION,   MSM_ISP_STATS_MAX},
  {{"rolloff"},          ISP_MOD_ROLLOFF,         MSM_ISP_STATS_MAX},
  {{"demux"},            ISP_MOD_DEMUX,           MSM_ISP_STATS_MAX},
  {{"demosaic"},         ISP_MOD_DEMOSAIC,        MSM_ISP_STATS_MAX},
  {{"bpc"},              ISP_MOD_BPC,             MSM_ISP_STATS_MAX},
  {{"abf"},              ISP_MOD_ABF,             MSM_ISP_STATS_MAX},
  {{"asf"},              ISP_MOD_ASF,             MSM_ISP_STATS_MAX},
  {{"colorcorrect"},     ISP_MOD_COLOR_CORRECT,   MSM_ISP_STATS_MAX},
  {{"chroma_subsample"}, ISP_MOD_CHROMA_ENHANCE,  MSM_ISP_STATS_MAX},
  {{"chroma_suppress"},  ISP_MOD_CHROMA_SUPPRESS, MSM_ISP_STATS_MAX},
  {{"luma_adaptation"},  ISP_MOD_LA,              MSM_ISP_STATS_MAX},
  {{"mce"},              ISP_MOD_MCE,             MSM_ISP_STATS_MAX},
  {{"sce"},              ISP_MOD_SCE,             MSM_ISP_STATS_MAX},
  {{"clf"},              ISP_MOD_CLF,             MSM_ISP_STATS_MAX},
  {{"wb"},               ISP_MOD_WB,              MSM_ISP_STATS_MAX},
  {{"gamma"},            ISP_MOD_GAMMA,           MSM_ISP_STATS_MAX},
  {{"bcc"},              ISP_MOD_BCC,             MSM_ISP_STATS_MAX},
  {{"stats"},            ISP_MOD_BE_STATS,        MSM_ISP_STATS_MAX},
};

static isp_hw_params_t common_hw_params = {
  .hw_info_table = common_hw,
  .num_hw = ARRAY_SIZE(common_hw),
};

static isp_hw_info_t viewfinder_stream_hw[] = {
//    {"colorxform_viewfinder"},
    {{"scaler_viewfinder"}, ISP_MOD_SCALER_VIEWFINDER, MSM_ISP_STATS_MAX},
//    {"fovcrop_viewfinder"},
//    {"clamp_viewfinder"},
};

static isp_hw_info_t encoder_stream_hw[] = {
//    {"colorxform_encoder"},
//    {"scaler_viewfinder"},
    {{"scaler_encoder"}, ISP_MOD_SCALER_ENCODER, MSM_ISP_STATS_MAX},
//    {"fovcrop_encoder"},
//    {"clamp_encoder"},
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

static uint32_t max_supported_stats =
(
  //(1 << MSM_ISP_STATS_AWB)   |
  (1 << MSM_ISP_STATS_RS)    |  /* share with BG */
  (1 << MSM_ISP_STATS_CS)    |  /* shared with BF */
  (1 << MSM_ISP_STATS_IHIST) |  /* for both CS and RS stats modules */
  (1 << MSM_ISP_STATS_BE)    |
  (1 << MSM_ISP_STATS_BF)    |
  (1 << MSM_ISP_STATS_BG)    |
  (1 << MSM_ISP_STATS_BHIST)
);

static isp_pipeline_t isp_pipeline = {
  .common_hw_params = &common_hw_params,
  .stream_hw_params[ISP_HW_STREAM_VIEWFINDER] =
     &stream_hw_params[ISP_HW_STREAM_VIEWFINDER],
  .stream_hw_params[ISP_HW_STREAM_ENCODER] =
     &stream_hw_params[ISP_HW_STREAM_ENCODER],
  .single_vfe_cx_level = ISP_CX_LEVEL_NOMINAL,
  //.max_supported_stats = max_supported_stats,
};

/** isp_pipeline32_open:
 *
 *  @isp_version: ISP version
 *  @isp_id: ISP hw id
 *
 *  Initialize ISP pipeline resources based on isp_version
 *  and isp_id
 *
 *  Return TRUE on success and FALSE on failure
 **/
isp_pipeline_t *isp_pipeline32_open(uint32_t isp_version, uint32_t isp_id)
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

  if (isp_version >= ISP_MSM8960V1) {
    /* 8960 / 8930 / 8x10  series */
    if ((isp_version == ISP_MSM8930) || (isp_version == ISP_MSM8610)) {
      curr_pipeline->isp_version = SET_ISP_VERSION(ISP_VERSION_32,
        ISP_REVISION_V2);
    } else if (isp_version == ISP_MSM8960V1) {
      curr_pipeline->isp_version = SET_ISP_VERSION(ISP_VERSION_32,
        ISP_REVISION_V1);
    }
    else {
      curr_pipeline->isp_version = SET_ISP_VERSION(ISP_VERSION_32,
        ISP_REVISION_V2);
    }

    curr_pipeline->num_pix = 1;
    curr_pipeline->num_rdi = 3;
    curr_pipeline->num_wms = 7;
    curr_pipeline->num_register = ISP32_NUM_REG_DUMP;
    curr_pipeline->isp_id = isp_id;
    curr_pipeline->is_camif_crop_supported = FALSE;
    curr_pipeline->is_camif_raw_op_fmt_supported = FALSE;
    curr_pipeline->is_mono_stats_supported = 0;
    curr_pipeline->max_width = 4736;
    curr_pipeline->max_height = 3552;
      /* TODO check this */
    curr_pipeline->hw_stream_info[ISP_HW_STREAM_ENCODER].max_width = 4736;
    curr_pipeline->hw_stream_info[ISP_HW_STREAM_ENCODER].max_height = 3552;
    curr_pipeline->hw_stream_info[ISP_HW_STREAM_VIEWFINDER].max_width = 2048;
    curr_pipeline->hw_stream_info[ISP_HW_STREAM_VIEWFINDER].max_height = 1600;

    curr_pipeline->hw_stream_info[ISP_HW_STREAM_IDEAL_RAW].max_width =
      curr_pipeline->hw_stream_info[ISP_HW_STREAM_ENCODER].max_width;
    curr_pipeline->hw_stream_info[ISP_HW_STREAM_IDEAL_RAW].max_height =
      curr_pipeline->hw_stream_info[ISP_HW_STREAM_ENCODER].max_height;

    curr_pipeline->max_pix_clk_rate[ISP_CX_LEVEL_NOMINAL] = ISP_PIX_CLK_320MHZ;

  } else {
    ISP_ERR("failed: invalid isp version %x", isp_version);
    free(curr_pipeline);
    return NULL;
  }

  curr_pipeline->max_scale_ratio = 16;
  curr_pipeline->num_hw_streams = ARRAY_SIZE(stream_hw_params);
  curr_pipeline->hw_stream_ids[0] = ISP_HW_STREAM_ENCODER;
  curr_pipeline->hw_stream_ids[1] = ISP_HW_STREAM_VIEWFINDER;
  curr_pipeline->pipeline_applying_delay = ISP_MAX_APPLYING_DELAY;

  return curr_pipeline;
}
