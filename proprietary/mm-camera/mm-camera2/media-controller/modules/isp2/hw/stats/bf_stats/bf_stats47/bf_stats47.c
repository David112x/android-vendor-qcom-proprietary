/* bf_stats47.c
 *
 * Copyright (c) 2014-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* isp headers */
#include "bf_stats47.h"
#include "isp_sub_module_util.h"
#include "isp_log.h"
#include "isp_defs.h"
#include "isp_pipeline_reg.h"
#include "isp_sub_module_log.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_BF_STATS, fmt, ##args)

#define IS_XY_EQUAL(a, b) ((a->x == b->x) && (a->y == b->y))
/** bf_stats47_update_min_stripe_overlap
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  Update min stripe overlap limit
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bf_stats47_update_min_stripe_overlap(isp_sub_module_t *isp_sub_module,
  void *data)
{
  bf_stats47_t                  *bf_stats = NULL;
  isp_stripe_limit_info_t       *stripe_limit = NULL;
  isp_private_event_t           *private_event = NULL;
  uint32_t                       bf_stats_min_overlap = 0;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  if (!isp_sub_module->submod_enable) {
    ISP_DBG("skip, bf stats enabled %d", isp_sub_module->submod_enable);
    return TRUE;
  }

  private_event = (isp_private_event_t *)data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  stripe_limit = (isp_stripe_limit_info_t *)private_event->data;
  if (!stripe_limit) {
    ISP_ERR("failed: stripe_limit %p", stripe_limit);
    return FALSE;
  }
  bf_stats_min_overlap = MIN_X_OFFSET + H_FIR_MARGIN;
  if (stripe_limit->min_overlap_left * 2 < bf_stats_min_overlap)
    stripe_limit->min_overlap_left = (MIN_X_OFFSET + H_FIR_MARGIN) / 2;
  if (stripe_limit->min_overlap_right * 2 < bf_stats_min_overlap)
    stripe_limit->min_overlap_right = (MIN_X_OFFSET + H_FIR_MARGIN) / 2;

 if (stripe_limit->min_overlap_left < MIN_BF_OVERLAP)
    stripe_limit->min_overlap_left = MIN_BF_OVERLAP;
  if (stripe_limit->min_overlap_right < MIN_BF_OVERLAP)
    stripe_limit->min_overlap_right = MIN_BF_OVERLAP;

  return TRUE;
}

/** bf_stats47_set_stripe_info:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy sensor dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bf_stats47_set_stripe_info(isp_sub_module_t *isp_sub_module,
  void *data)
{
  bf_stats47_t            *bf_stats = NULL;
  ispif_out_info_t        *ispif_stripe_info = NULL;
  isp_private_event_t     *private_event = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  ispif_stripe_info = (ispif_out_info_t *)private_event->data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bf_stats = (bf_stats47_t *)isp_sub_module->private_data;
  if (!bf_stats) {
    ISP_ERR("failed: bf_stats %p", bf_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  bf_stats->ispif_out_info = *ispif_stripe_info;
  ISP_DBG("right stripe offset %d", ispif_stripe_info->right_stripe_offset);

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** bf_stats47_set_split_info:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy sensor dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bf_stats47_set_split_info(isp_sub_module_t *isp_sub_module,
  void *data)
{
  bf_stats47_t            *bf_stats = NULL;
  isp_out_info_t          *isp_split_out_info = NULL;
  isp_private_event_t     *private_event = NULL;

  ISP_DBG("bf_stats47_set_split_info");
  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  isp_split_out_info = (isp_out_info_t *)private_event->data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bf_stats = (bf_stats47_t *)isp_sub_module->private_data;
  if (!bf_stats) {
    ISP_ERR("failed: bf_stats %p", bf_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  bf_stats->isp_out_info = *isp_split_out_info;
  ISP_DBG("left output width %d right output width %d",
    bf_stats->isp_out_info.left_output_width,
    bf_stats->isp_out_info.right_output_width);

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/**bf_stats47_split_cfg
*  @bf_stats: bf stats handle
*  @roi_cfg: bf stats roi cfg            H_FIR_MARGIN
*  Split roi for Dual vfe         -->    <-----------
*  |----|-----------------|----------|--|-----------------------|
*  |    |                 |          |  |
*  |    |                 |          |  |
*  |    |                 |          |  |
*  |    |                 |          |  |
*  |    |                 |          |  |
*  |    |                 |          |  |
*  |    |                 |          |  |
*  |    |                 |          |  |
*  |    |                 |          |  |
*  |----|-----------------|----------|--|-----------------------|
*  <---->                 <------------->
* MIN_X_OFFSET                 overlap
*  <------------------------------------>
*        left_stripe_stats_bound
*  <--------------------------------->
*        left_stripe_roi_limit
*  <---------------------->
*     right stripe offset
*  <------------------------------------------------------------->
*                     sensor output width
*
*  If the ROI is fitting in left vfe between MIN_X_OFFSET and
*  left_stripe_roi_limit then the roi is configured in left vfe.
*  if roi is in between left_stripe_roi_limit and sensor output
*  width then the roi is placed in right vfe. if the roi is
*  extending to both vfe then it is cut into two pieces at
*  left_stripe_roi_limit and combined in parsing.
  */
static boolean bf_stats47_split_cfg(bf_stats47_t *bf_stats,
  bf_fw_roi_cfg_t *roi_cfg)
{
  boolean              ret = TRUE;
  uint32_t             i = 0, j = 0,
                       num_entries = 0,
                       overlap = 0,
                       num_left_rgns = 0,
                       num_right_rgns = 0,
                       left_stripe_stats_bound = 0,
                       right_stripe_offset = 0,
                       left_stripe_roi_limit = 0,
                       tempx, left_width = 0,
                       right_width = 0;
  bf_fw_roi_dim_t     *roi_dim = NULL,
                      *roi_i_dim = NULL,
                      *roi_j_dim = NULL;
  isp_out_info_t      *isp_out = NULL;
  bf_fw_config_t      *bf_fw_local = NULL;
  bf_fw_roi_cfg_t     *bf_hw_roi = NULL,
                       bf_hw_roi_left_vfe,
                       bf_hw_roi_right_vfe;

  if (!bf_stats || !roi_cfg) {
    ISP_ERR("failed: %p %p", bf_stats, roi_cfg);
    return FALSE;
  }
  isp_out = &bf_stats->isp_out_info;
  overlap = bf_stats->ispif_out_info.overlap;
  left_stripe_stats_bound = isp_out->right_stripe_offset + overlap;
  left_stripe_roi_limit = left_stripe_stats_bound - H_FIR_MARGIN;
  bf_hw_roi = &bf_stats->bf_hw_roi_cfg;
  memset(&bf_hw_roi_left_vfe, 0, sizeof(bf_fw_roi_cfg_t));
  memset(&bf_hw_roi_right_vfe, 0, sizeof(bf_fw_roi_cfg_t));

  ISP_DBG("overlap %d, left_stripe_stats_bound %d", overlap,
    left_stripe_stats_bound);

  if (roi_cfg->is_valid == FALSE) {
    return TRUE;
  }

  bf_stats->x_min = bf_stats->y_min = ~0;
  bf_stats->x_max = bf_stats->y_max = 0;

  bf_fw_local = &bf_stats->af_config.bf_fw;
  right_stripe_offset = isp_out->right_stripe_offset;
  if (isp_out->is_split == TRUE) {
    for (i = 0; i < roi_cfg->num_bf_fw_roi_dim &&
          roi_cfg->bf_fw_roi_dim[i].is_valid == TRUE; i++) {
      roi_dim = &roi_cfg->bf_fw_roi_dim[i];
      ISP_DBG("x %d rso %d w %d lssb %d", roi_dim->x, right_stripe_offset,
        roi_dim->w, left_stripe_stats_bound);
      if ((roi_dim->x + roi_dim->w) <= left_stripe_roi_limit) {
        /* ROI can be put on left VFE*/
        bf_hw_roi_left_vfe.bf_fw_roi_dim[num_left_rgns].x = roi_dim->x;
        bf_hw_roi_left_vfe.bf_fw_roi_dim[num_left_rgns].y = roi_dim->y;
        bf_hw_roi_left_vfe.bf_fw_roi_dim[num_left_rgns].w = roi_dim->w;
        bf_hw_roi_left_vfe.bf_fw_roi_dim[num_left_rgns].h = roi_dim->h;
        bf_hw_roi_left_vfe.bf_fw_roi_dim[num_left_rgns].region_num =
          roi_dim->region_num;
        bf_hw_roi_left_vfe.bf_fw_roi_dim[num_left_rgns].is_valid = TRUE;

        if (isp_out->stripe_id == ISP_STRIPE_LEFT) {
          /* x_min indicates the active boundary for IIR operation. IIR needs
           * atleast 64 pixels to stabilize before any stats can be collected.
           * Hence subtract 64 from x_min to inidcate IIR active boundary
           */
          bf_stats->x_min = MIN(bf_stats->x_min, (roi_dim->x - H_IIR_MARGIN));
          bf_stats->y_min = MIN(bf_stats->y_min, (roi_dim->y - V_IIR_MARGIN));
          /* x_max indicates end of active boundary which is right edge of
           * ROI window
           */
          bf_stats->x_max = MAX(bf_stats->x_max, (roi_dim->x + roi_dim->w));
          bf_stats->y_max = MAX(bf_stats->y_max, (roi_dim->y + roi_dim->h));
        }

        num_left_rgns ++;
      } else if ((roi_dim->x < (right_stripe_offset + MIN_X_OFFSET)) &&
        ((roi_dim->x + roi_dim->w) > left_stripe_roi_limit)) {
        /* Try to cut the ROI at left_stripe_roi_limit. If the right width is
           less than MIN_WIDTH then adjust left width and right width to
           satisfy the MIN_WIDTH requirement. */
        left_width = left_stripe_stats_bound - roi_dim->x - H_FIR_MARGIN - 1;
        left_width = ODD_FLOOR(left_width);
        tempx = left_stripe_stats_bound - H_FIR_MARGIN;
        right_width = ODD_FLOOR(roi_dim->x + roi_dim->w - tempx);
        if (right_width < MIN_WIDTH) {
          left_width -= MIN_WIDTH;
          bf_hw_roi_right_vfe.bf_fw_roi_dim[num_right_rgns].x =
            EVEN_FLOOR(overlap - H_FIR_MARGIN - MIN_WIDTH);
          right_width += MIN_WIDTH;
        } else {
          bf_hw_roi_right_vfe.bf_fw_roi_dim[num_right_rgns].x =
            EVEN_FLOOR(overlap - H_FIR_MARGIN);
        }
        if (left_width >= (MIN_WIDTH - 1)) {
          bf_hw_roi_left_vfe.bf_fw_roi_dim[num_left_rgns].x = roi_dim->x;
          bf_hw_roi_left_vfe.bf_fw_roi_dim[num_left_rgns].y = roi_dim->y;
          bf_hw_roi_left_vfe.bf_fw_roi_dim[num_left_rgns].w = left_width;
          bf_hw_roi_left_vfe.bf_fw_roi_dim[num_left_rgns].h = roi_dim->h;
          bf_hw_roi_left_vfe.bf_fw_roi_dim[num_left_rgns].region_num =
            roi_dim->region_num;
          bf_hw_roi_left_vfe.bf_fw_roi_dim[num_left_rgns].is_valid = TRUE;

          if (isp_out->stripe_id == ISP_STRIPE_LEFT) {
            /* x_min indicates the active boundary for IIR operation. IIR needs
             * atleast 64 pixels to stabilize before any stats can be collected.
             * Hence subtract 64 from x_min to inidcate IIR active boundary
             */
            bf_stats->x_min = MIN(bf_stats->x_min, (roi_dim->x - H_IIR_MARGIN));
            bf_stats->y_min = MIN(bf_stats->y_min, (roi_dim->y - V_IIR_MARGIN));
            /* x_max indicates end of active boundary which is right edge of
             * ROI window
             */
            bf_stats->x_max = MAX(bf_stats->x_max, (roi_dim->x + left_width));
            bf_stats->y_max = MAX(bf_stats->y_max, (roi_dim->y + roi_dim->h));
          }

          num_left_rgns ++;
        }

        bf_hw_roi_right_vfe.bf_fw_roi_dim[num_right_rgns].y = roi_dim->y;
        bf_hw_roi_right_vfe.bf_fw_roi_dim[num_right_rgns].w = right_width;
        bf_hw_roi_right_vfe.bf_fw_roi_dim[num_right_rgns].h = roi_dim->h;
        bf_hw_roi_right_vfe.bf_fw_roi_dim[num_right_rgns].region_num =
          roi_dim->region_num;
        bf_hw_roi_right_vfe.bf_fw_roi_dim[num_right_rgns].is_valid = TRUE;

        if (isp_out->stripe_id == ISP_STRIPE_RIGHT) {
          /* x_min indicates the active boundary for IIR operation. IIR needs
           * atleast 64 pixels to stabilize before any stats can be collected.
           * Hence subtract 64 from x_min to inidcate IIR active boundary
           */
          bf_stats->x_min = MIN(bf_stats->x_min, (roi_dim->x - H_IIR_MARGIN));
          bf_stats->y_min = MIN(bf_stats->y_min, (roi_dim->y - V_IIR_MARGIN));
          /* x_max indicates end of active boundary which is right edge of
           * ROI window
           */
          bf_stats->x_max = MAX(bf_stats->x_max, (roi_dim->x + right_width));
          bf_stats->y_max = MAX(bf_stats->y_max, (roi_dim->y + roi_dim->h));
        }

        num_right_rgns++;
      } else if ((roi_dim->x + roi_dim->w) > left_stripe_roi_limit &&
        (roi_dim->x >= (right_stripe_offset + MIN_X_OFFSET))) {
        /* ROI can be put on right VFE */
        bf_hw_roi_right_vfe.bf_fw_roi_dim[num_right_rgns].x = roi_dim->x -
          right_stripe_offset;
        bf_hw_roi_right_vfe.bf_fw_roi_dim[num_right_rgns].y = roi_dim->y;
        bf_hw_roi_right_vfe.bf_fw_roi_dim[num_right_rgns].w = roi_dim->w;
        bf_hw_roi_right_vfe.bf_fw_roi_dim[num_right_rgns].h = roi_dim->h;
        bf_hw_roi_right_vfe.bf_fw_roi_dim[num_right_rgns].region_num =
          roi_dim->region_num;
        bf_hw_roi_right_vfe.bf_fw_roi_dim[num_right_rgns].is_valid = TRUE;

        if (isp_out->stripe_id == ISP_STRIPE_RIGHT) {
          /* x_min indicates the active boundary for IIR operation. IIR needs
           * atleast 64 pixels to stabilize before any stats can be collected.
           * Hence subtract 64 from x_min to inidcate IIR active boundary
           */
          bf_stats->x_min = MIN(bf_stats->x_min, (roi_dim->x -
            right_stripe_offset - H_IIR_MARGIN));
          bf_stats->y_min = MIN(bf_stats->y_min, (roi_dim->y - V_IIR_MARGIN));
          /* x_max indicates end of active boundary which is right edge of
           * ROI window
           */
          bf_stats->x_max = MAX(bf_stats->x_max, (roi_dim->x -
            right_stripe_offset + roi_dim->w));
          bf_stats->y_max = MAX(bf_stats->y_max, (roi_dim->y + roi_dim->h));
        }

        num_right_rgns++;
      } else {
        ISP_ERR("Error: ROI in limbo and not consumed x %d y %d w %d h %d",
           roi_dim->x, roi_dim->y, roi_dim->w, roi_dim->h);
      }
    }

    if (isp_out->stripe_id == ISP_STRIPE_LEFT) {
      if (num_left_rgns > 0) {
        memcpy(bf_hw_roi, &bf_hw_roi_left_vfe, sizeof(bf_fw_roi_cfg_t));
        bf_hw_roi->is_valid = TRUE;
        bf_hw_roi->num_bf_fw_roi_dim = num_left_rgns;
      } else {
        ISP_DBG("%s Create a dummy left region", __func__);
        bf_hw_roi->bf_fw_roi_dim[0].x = MIN_X_OFFSET;
        bf_hw_roi->bf_fw_roi_dim[0].y = MIN_Y_OFFSET;
        bf_hw_roi->bf_fw_roi_dim[0].w = MIN_WIDTH - 1;
        bf_hw_roi->bf_fw_roi_dim[0].h = MIN_HEIGHT - 1;
        bf_hw_roi->bf_fw_roi_dim[0].is_valid = FALSE;
        bf_hw_roi->bf_fw_roi_dim[0].region_num = 0;
        bf_hw_roi->num_bf_fw_roi_dim = 1;
        bf_hw_roi->is_valid = TRUE;
        bf_stats->x_min = MIN_X_OFFSET;
        bf_stats->y_min = MIN_Y_OFFSET;
        bf_stats->x_max = MIN_X_OFFSET + MIN_WIDTH - 1;
        bf_stats->y_max = MIN_Y_OFFSET + MIN_HEIGHT - 1;
      }
    } else {
      if (num_right_rgns > 0) {
        memcpy(bf_hw_roi, &bf_hw_roi_right_vfe, sizeof(bf_fw_roi_cfg_t));
        bf_hw_roi->is_valid = TRUE;
        bf_hw_roi->num_bf_fw_roi_dim = num_right_rgns;
      } else {
        ISP_DBG("%s Create a dummy right region", __func__);
        bf_hw_roi->bf_fw_roi_dim[0].x = MIN_X_OFFSET;
        bf_hw_roi->bf_fw_roi_dim[0].y = MIN_Y_OFFSET;
        bf_hw_roi->bf_fw_roi_dim[0].w = MIN_WIDTH - 1;
        bf_hw_roi->bf_fw_roi_dim[0].h = MIN_HEIGHT - 1;
        bf_hw_roi->bf_fw_roi_dim[0].is_valid = FALSE;
        bf_hw_roi->bf_fw_roi_dim[0].region_num = 0;
        bf_hw_roi->num_bf_fw_roi_dim = 1;
        bf_hw_roi->is_valid = TRUE;
        bf_stats->x_min = MIN_X_OFFSET;
        bf_stats->y_min = MIN_Y_OFFSET;
        bf_stats->x_max = MIN_X_OFFSET + MIN_WIDTH - 1;
        bf_stats->y_max = MIN_Y_OFFSET + MIN_HEIGHT - 1;
      }
    }
  } else {
    /*Single VFE case copy only valid ROIs*/
    for (i = 0; i < roi_cfg->num_bf_fw_roi_dim &&
          roi_cfg->bf_fw_roi_dim[i].is_valid == TRUE; i++) {
      roi_dim = &roi_cfg->bf_fw_roi_dim[i];
      bf_hw_roi_left_vfe.bf_fw_roi_dim[num_left_rgns].x = roi_dim->x;
      bf_hw_roi_left_vfe.bf_fw_roi_dim[num_left_rgns].y = roi_dim->y;
      bf_hw_roi_left_vfe.bf_fw_roi_dim[num_left_rgns].w = roi_dim->w;
      bf_hw_roi_left_vfe.bf_fw_roi_dim[num_left_rgns].h = roi_dim->h;
      bf_hw_roi_left_vfe.bf_fw_roi_dim[num_left_rgns].region_num =
        roi_dim->region_num;
      bf_hw_roi_left_vfe.bf_fw_roi_dim[num_left_rgns].is_valid = TRUE;

      /* x_min indicates the active boundary for IIR operation. IIR needs
       * atleast 64 pixels to stabilize before any stats can be collected.
       * Hence subtract 64 from x_min to inidcate IIR active boundary
       */
      bf_stats->x_min = MIN(bf_stats->x_min, (roi_dim->x - H_IIR_MARGIN));
      bf_stats->y_min = MIN(bf_stats->y_min, (roi_dim->y - V_IIR_MARGIN));
      /* x_max indicates end of active boundary which is right edge of
       * ROI window
       */
      bf_stats->x_max = MAX(bf_stats->x_max, (roi_dim->x + roi_dim->w));
      bf_stats->y_max = MAX(bf_stats->y_max, (roi_dim->y + roi_dim->h));

      num_left_rgns++;
    }
    if (num_left_rgns > 0) {
      memcpy(bf_hw_roi, &bf_hw_roi_left_vfe, sizeof(bf_fw_roi_cfg_t));
      bf_hw_roi->is_valid = TRUE;
      bf_hw_roi->num_bf_fw_roi_dim = num_left_rgns;
    } else {
      ISP_ERR("error: No valid ROIs found, restoring old rois");
    }
  }

  bf_stats->num_left_rgns = num_left_rgns;
  bf_stats->num_right_rgns = num_right_rgns;
  ISP_DBG("num_left_rgns %d, num_right_rgns %d", bf_stats->num_left_rgns, bf_stats->num_right_rgns);

  return TRUE;

}/*bf_stats47_split_cfg*/

/** bf_stats47_fill_stats_parser_params:
 *
 *  @bf_stats: bf stats params
 *  @stats_params: stats parser params
 *
 *  Fill stats parser params based on single VFE or dual VFE
 *  case
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bf_stats47_fill_stats_parser_params(bf_stats47_t *bf_stats,
  isp_saved_stats_params_t *stats_params)
{
  isp_rgns_stats_param_t    *rgns_stats = NULL;
  isp_stats_config_t        *stats_config = NULL;

  if (!bf_stats || !stats_params) {
    ISP_ERR("failed: %p %p", bf_stats, stats_params);
    return FALSE;
  }

  rgns_stats = &stats_params->rgns_stats[MSM_ISP_STATS_BF];

  rgns_stats->is_valid = TRUE;
  stats_config = &stats_params->stats_config;
  stats_config->stats_mask |= (1 << MSM_ISP_STATS_BF);
  stats_config->af_config = bf_stats->af_config;

  /* Fill stats_params */
  stats_params->bf_fw_stats_cfg_info.bf_fw_roi_cfg_end_pix_order =
    bf_stats->bf_roi_end_pix_order ;
  stats_params->bf_fw_stats_cfg_info.bf_fw_roi_cfg_3a_order =
    bf_stats->af_config.bf_fw.bf_fw_roi_cfg;

  return TRUE;
}

/** bf_stats47_adjust_signbit:
 *
 *  @val: value to adjust
 *  @num_bits: number of destination bits
 *
 *  Adjust sign bit
 **/
int32_t bf_stats47_adjust_signbit(int32_t val, uint8_t num_bits)
{
  if (val < 0) {
    val = -(val);
    val &= ((1 << num_bits) - 1);
    val = (1 << num_bits) - val;
  } else {
    val &= ((1 << num_bits) - 1);
  }
  return val;
}

/** bf_stats47_fill_h_fir_params:
 *
 *  @bf_stats_h_fir_cfg: h fir cfg in bf_stats struct
 *  @af_fir_filter_cfg: h fir cfg from 3A
 *
 *  Fill h fir cfg from 3A info bf_stats packed struct
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bf_stats47_fill_h_fir_params(
  bf_stats_h_fir_cfg_t *bf_stats_h_fir_cfg,
  bf_fir_filter_cfg_t  *af_fir_filter_cfg)
{
  if (!bf_stats_h_fir_cfg || !af_fir_filter_cfg) {
    ISP_ERR("failed: bf_stats_h_fir_cfg %p af_fir_filter_cfg %p",
      bf_stats_h_fir_cfg, af_fir_filter_cfg);
    return FALSE;
  }

  bf_stats_h_fir_cfg->a00 = bf_stats47_adjust_signbit(af_fir_filter_cfg->a[0],
    FIR_BITS);
  bf_stats_h_fir_cfg->a01 = bf_stats47_adjust_signbit(af_fir_filter_cfg->a[1],
    FIR_BITS);
  bf_stats_h_fir_cfg->a02 = bf_stats47_adjust_signbit(af_fir_filter_cfg->a[2],
    FIR_BITS);
  bf_stats_h_fir_cfg->a03 = bf_stats47_adjust_signbit(af_fir_filter_cfg->a[3],
    FIR_BITS);
  bf_stats_h_fir_cfg->a04 = bf_stats47_adjust_signbit(af_fir_filter_cfg->a[4],
    FIR_BITS);
  bf_stats_h_fir_cfg->a05 = bf_stats47_adjust_signbit(af_fir_filter_cfg->a[5],
    FIR_BITS);
  bf_stats_h_fir_cfg->a06 = bf_stats47_adjust_signbit(af_fir_filter_cfg->a[6],
    FIR_BITS);
  bf_stats_h_fir_cfg->a07 = bf_stats47_adjust_signbit(af_fir_filter_cfg->a[7],
    FIR_BITS);
  bf_stats_h_fir_cfg->a08 = bf_stats47_adjust_signbit(af_fir_filter_cfg->a[8],
    FIR_BITS);
  bf_stats_h_fir_cfg->a09 = bf_stats47_adjust_signbit(af_fir_filter_cfg->a[9],
    FIR_BITS);
  bf_stats_h_fir_cfg->a10 = bf_stats47_adjust_signbit(af_fir_filter_cfg->a[10],
    FIR_BITS);
  bf_stats_h_fir_cfg->a11 = bf_stats47_adjust_signbit(af_fir_filter_cfg->a[11],
    FIR_BITS);
  bf_stats_h_fir_cfg->a12 = bf_stats47_adjust_signbit(af_fir_filter_cfg->a[12],
    FIR_BITS);

  return TRUE;
}

/** bf_stats47_fill_h_iir_params:
 *
 *  @bf_stats_h_iir_cfg: h iir cfg in bf_stats struct
 *  @af_iir_filter_cfg: h iir cfg from 3A
 *
 *  Fill h iir cfg from 3A info bf_stats packed struct
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bf_stats47_fill_h_iir_params(
  bf_stats_h_iir_cfg_t *bf_stats_h_iir_cfg,
  bf_iir_filter_cfg_t *af_iir_filter_cfg)
{
  if (!bf_stats_h_iir_cfg || !af_iir_filter_cfg) {
    ISP_ERR("failed: bf_stats_h_iir_cfg %p af_iir_filter_cfg %p",
      bf_stats_h_iir_cfg, af_iir_filter_cfg);
    return FALSE;
  }

  bf_stats_h_iir_cfg->b10 = bf_stats47_adjust_signbit(FLOAT_TO_Q(14,
    af_iir_filter_cfg->b10), IIR_BITS);
  bf_stats_h_iir_cfg->b11 = bf_stats47_adjust_signbit(FLOAT_TO_Q(14,
    af_iir_filter_cfg->b11), IIR_BITS);
  bf_stats_h_iir_cfg->b12 = bf_stats47_adjust_signbit(FLOAT_TO_Q(14,
    af_iir_filter_cfg->b12), IIR_BITS);
  bf_stats_h_iir_cfg->b22 = bf_stats47_adjust_signbit(FLOAT_TO_Q(14,
    af_iir_filter_cfg->b22), IIR_BITS);
  bf_stats_h_iir_cfg->a11 = bf_stats47_adjust_signbit(FLOAT_TO_Q(14,
    af_iir_filter_cfg->a11), IIR_BITS);
  bf_stats_h_iir_cfg->a12 = bf_stats47_adjust_signbit(FLOAT_TO_Q(14,
    af_iir_filter_cfg->a12), IIR_BITS);
  bf_stats_h_iir_cfg->b20 = bf_stats47_adjust_signbit(FLOAT_TO_Q(14,
    af_iir_filter_cfg->b20), IIR_BITS);
  bf_stats_h_iir_cfg->b21 = bf_stats47_adjust_signbit(FLOAT_TO_Q(14,
    af_iir_filter_cfg->b21), IIR_BITS);
  bf_stats_h_iir_cfg->a21 = bf_stats47_adjust_signbit(FLOAT_TO_Q(14,
    af_iir_filter_cfg->a21), IIR_BITS);
  bf_stats_h_iir_cfg->a22 = bf_stats47_adjust_signbit(FLOAT_TO_Q(14,
    af_iir_filter_cfg->a22), IIR_BITS);
  return TRUE;
}

/** bf_stats47_fill_v_fir_params:
 *
 *  @bf_stats_v_fir_cfg: v fir cfg in bf_stats struct
 *  @af_fir_filter_cfg: v fir cfg from 3A
 *
 *  Fill v fir cfg from 3A info bf_stats packed struct
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bf_stats47_fill_v_fir_params(
  bf_stats_v_fir_cfg_t *bf_stats_v_fir_cfg,
  bf_fir_filter_cfg_t  *af_fir_filter_cfg)
{
  if (!bf_stats_v_fir_cfg || !af_fir_filter_cfg) {
    ISP_ERR("failed: bf_stats_v_fir_cfg %p af_fir_filter_cfg %p",
      bf_stats_v_fir_cfg, af_fir_filter_cfg);
    return FALSE;
  }

  bf_stats_v_fir_cfg->a00 = bf_stats47_adjust_signbit(af_fir_filter_cfg->a[0],
    FIR_BITS);
  bf_stats_v_fir_cfg->a01 = bf_stats47_adjust_signbit(af_fir_filter_cfg->a[1],
    FIR_BITS);
  bf_stats_v_fir_cfg->a02 = bf_stats47_adjust_signbit(af_fir_filter_cfg->a[2],
    FIR_BITS);
  bf_stats_v_fir_cfg->a03 = bf_stats47_adjust_signbit(af_fir_filter_cfg->a[3],
    FIR_BITS);
  bf_stats_v_fir_cfg->a04 = bf_stats47_adjust_signbit(af_fir_filter_cfg->a[4],
    FIR_BITS);

  return TRUE;
}

/** bf_stats47_fill_v_iir_params:
 *
 * @bf_stats_v_iir_cfg: v iir cfg in bf_stats struct
 * @af_iir_filter_cfg: v iir cfg from 3A
 *
 * Fill v iir cfg from 3A info bf_stats packed struct
 *
 * Return TRUE on success and FALSE on failure
 **/
#ifdef __BF_STATS41_REG_H__
static boolean bf_stats47_fill_v_iir1_params(
  bf_stats_v_iir_cfg1_t *bf_stats_v_iir_cfg1,
  bf_iir_filter_cfg_t *af_iir_filter_cfg)
{
  if (!bf_stats_v_iir_cfg1 || !af_iir_filter_cfg) {
    ISP_ERR("failed: bf_stats_v_iir_cfg1 %p af_iir_filter_cfg %p",
      bf_stats_v_iir_cfg1, af_iir_filter_cfg);
    return FALSE;
  }
  bf_stats_v_iir_cfg1->b20 = bf_stats47_adjust_signbit(FLOAT_TO_Q(14,
    af_iir_filter_cfg->b20), IIR_BITS);
  bf_stats_v_iir_cfg1->b21 = bf_stats47_adjust_signbit(FLOAT_TO_Q(14,
    af_iir_filter_cfg->b21), IIR_BITS);
  bf_stats_v_iir_cfg1->b22 = bf_stats47_adjust_signbit(FLOAT_TO_Q(14,
    af_iir_filter_cfg->b22), IIR_BITS);
  bf_stats_v_iir_cfg1->a21 = bf_stats47_adjust_signbit(FLOAT_TO_Q(14,
    af_iir_filter_cfg->a21), IIR_BITS);
  bf_stats_v_iir_cfg1->a22 = bf_stats47_adjust_signbit(FLOAT_TO_Q(14,
    af_iir_filter_cfg->a22), IIR_BITS);
  return TRUE;
}
#endif


/** bf_stats47_fill_v_iir_params:
 *
 *  @bf_stats_v_iir_cfg: v iir cfg in bf_stats struct
 *  @af_iir_filter_cfg: v iir cfg from 3A
 *
 *  Fill v iir cfg from 3A info bf_stats packed struct
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bf_stats47_fill_v_iir_params(
  bf_stats_v_iir_cfg_t *bf_stats_v_iir_cfg,
  bf_iir_filter_cfg_t *af_iir_filter_cfg)
{
  if (!bf_stats_v_iir_cfg || !af_iir_filter_cfg) {
    ISP_ERR("failed: bf_stats_v_iir_cfg %p af_iir_filter_cfg %p",
      bf_stats_v_iir_cfg, af_iir_filter_cfg);
    return FALSE;
  }

  bf_stats_v_iir_cfg->b10 = bf_stats47_adjust_signbit(FLOAT_TO_Q(14,
    af_iir_filter_cfg->b10), IIR_BITS);
  bf_stats_v_iir_cfg->b11 = bf_stats47_adjust_signbit(FLOAT_TO_Q(14,
    af_iir_filter_cfg->b11), IIR_BITS);
  bf_stats_v_iir_cfg->b12 = bf_stats47_adjust_signbit(FLOAT_TO_Q(14,
    af_iir_filter_cfg->b12), IIR_BITS);
  bf_stats_v_iir_cfg->a11 = bf_stats47_adjust_signbit(FLOAT_TO_Q(14,
    af_iir_filter_cfg->a11), IIR_BITS);
  bf_stats_v_iir_cfg->a12 = bf_stats47_adjust_signbit(FLOAT_TO_Q(14,
    af_iir_filter_cfg->a12), IIR_BITS);

  return TRUE;
}

static uint32_t bf_stats47_calc_coring(
  uint32_t idx, uint32_t val)
{
  float tmp = (float)((float)(idx*idx)/(val*val));
  return (uint32_t)(tmp*16.0f);
}

/** bf_stats47_fill_coring_params:
 *
 *  @bf_stats_coring_cfg: coring cfg in bf_stats struct
 *  @af_filter_coring_cfg: coring cfg from 3A
 *
 *  Fill coring cfg from 3A info bf_stats packed struct
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bf_stats47_fill_coring_params(
  bf_stats_coring_cfg_t *bf_stats_coring_cfg,
  bf_filter_coring_cfg_t *af_filter_coring_cfg)
{
  bf_filter_coring_cfg_t temp_af_filter_coring;
  uint32_t temp_threshold = 0, i = 0;
  if (!bf_stats_coring_cfg || !af_filter_coring_cfg) {
    ISP_ERR("failed: bf_stats_coring_cfg %p af_filter_coring_cfg %p",
      bf_stats_coring_cfg, af_filter_coring_cfg);
    return FALSE;
  }

  bf_stats_coring_cfg->threshold = (af_filter_coring_cfg->threshold &
    THRESHOLD_MASK);
  temp_threshold = bf_stats_coring_cfg->threshold;
  bf_stats_coring_cfg->ind_0 = af_filter_coring_cfg->ind[0];
  bf_stats_coring_cfg->ind_1 = af_filter_coring_cfg->ind[1];
  bf_stats_coring_cfg->ind_2 = af_filter_coring_cfg->ind[2];
  bf_stats_coring_cfg->ind_3 = af_filter_coring_cfg->ind[3];
  bf_stats_coring_cfg->ind_4 = af_filter_coring_cfg->ind[4];
  bf_stats_coring_cfg->ind_5 = af_filter_coring_cfg->ind[5];
  bf_stats_coring_cfg->ind_6 = af_filter_coring_cfg->ind[6];
  bf_stats_coring_cfg->ind_7 = af_filter_coring_cfg->ind[7];
  bf_stats_coring_cfg->ind_8 = af_filter_coring_cfg->ind[8];
  bf_stats_coring_cfg->ind_9 = af_filter_coring_cfg->ind[9];
  bf_stats_coring_cfg->ind_10 = af_filter_coring_cfg->ind[10];
  bf_stats_coring_cfg->ind_11 = af_filter_coring_cfg->ind[11];
  bf_stats_coring_cfg->ind_12 = af_filter_coring_cfg->ind[12];
  bf_stats_coring_cfg->ind_13 = af_filter_coring_cfg->ind[13];
  bf_stats_coring_cfg->ind_14 = af_filter_coring_cfg->ind[14];
  bf_stats_coring_cfg->ind_15 = af_filter_coring_cfg->ind[15];
  bf_stats_coring_cfg->ind_16 = af_filter_coring_cfg->ind[16];

  #ifdef __BF_STATS41_REG_H__
  if (bf_stats_coring_cfg->threshold < 16) {
    bf_stats_coring_cfg->threshold = 16;
    temp_af_filter_coring.ind[0] = 0;
    for (i = 1; i< temp_threshold; i++) {
      temp_af_filter_coring.ind[i] = bf_stats47_calc_coring(i, temp_threshold);
    }
    for (i; i<=16; i++) {
      temp_af_filter_coring.ind[i] = bf_stats_coring_cfg->threshold;
    }
    bf_stats_coring_cfg->ind_0  =  temp_af_filter_coring.ind[0];
    bf_stats_coring_cfg->ind_1  =  temp_af_filter_coring.ind[1];
    bf_stats_coring_cfg->ind_2  =  temp_af_filter_coring.ind[2];
    bf_stats_coring_cfg->ind_3  =  temp_af_filter_coring.ind[3];
    bf_stats_coring_cfg->ind_4  =  temp_af_filter_coring.ind[4];
    bf_stats_coring_cfg->ind_5  =  temp_af_filter_coring.ind[5];
    bf_stats_coring_cfg->ind_6  =  temp_af_filter_coring.ind[6];
    bf_stats_coring_cfg->ind_7  =  temp_af_filter_coring.ind[7];
    bf_stats_coring_cfg->ind_8  =  temp_af_filter_coring.ind[8];
    bf_stats_coring_cfg->ind_9  =  temp_af_filter_coring.ind[9];
    bf_stats_coring_cfg->ind_10 =  temp_af_filter_coring.ind[10];
    bf_stats_coring_cfg->ind_11 =  temp_af_filter_coring.ind[11];
    bf_stats_coring_cfg->ind_12 =  temp_af_filter_coring.ind[12];
    bf_stats_coring_cfg->ind_13 =  temp_af_filter_coring.ind[13];
    bf_stats_coring_cfg->ind_14 =  temp_af_filter_coring.ind[14];
    bf_stats_coring_cfg->ind_15 =  temp_af_filter_coring.ind[15];
    bf_stats_coring_cfg->ind_16 =  temp_af_filter_coring.ind[16];
  }
  #endif

  return TRUE;
}

/** bf_stats47_store_filter_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @bf_stats: bf stats handle
 *
 *  Update filter config from 3a event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bf_stats47_store_filter_config(isp_sub_module_t *isp_sub_module,
  bf_stats47_t *bf_stats)
{
  boolean                     ret = TRUE,
                              func_ret = TRUE;
  bf_fw_config_t         *bf_fw = NULL;
  bf_stats_filter_cfg_t      *bf_stats_filter_cfg = NULL;
  bf_stats_shift_bits_cfg_t  *bf_stats_shift_bits_cfg = NULL;
  bf_stats_coring_gain_cfg_t *bf_stats_coring_gain_cfg = NULL;
  bf_stats_enable_t          *bf_stats_enable_mask = NULL;
  bf_stats_enable_t          *bf_stats_enable_val = NULL;
  bf_fir_filter_cfg_t        *bf_fir_filter_cfg = NULL;
  bf_iir_filter_cfg_t        *bf_iir_filter_cfg = NULL;

  if (!isp_sub_module || !bf_stats) {
    ISP_ERR("failed: %p %p", isp_sub_module, bf_stats);
    return FALSE;
  }

  bf_fw = &bf_stats->af_config.bf_fw;
  bf_stats_filter_cfg = &bf_stats->pcmd.bf_stats_filter_cfg;
  bf_stats_enable_mask = &bf_stats->pcmd.bf_stats_enable_mask;
  bf_stats_enable_val = &bf_stats->pcmd.bf_stats_enable_val;

  /* Update H_1 filter Start */
  bf_fir_filter_cfg =
    &bf_fw->bf_filter_cfg[BF_FILTER_TYPE_H_1].bf_fir_filter_cfg;
  bf_iir_filter_cfg =
    &bf_fw->bf_filter_cfg[BF_FILTER_TYPE_H_1].bf_iir_filter_cfg;

  /* Update whether H1 will receive scaled / unscaled input */
  /* Check whether downscaler is supported and enabled */
  /* Check whether H1 needs downscaled input */
  if ((BF_DOWN_SCALER_SUPPORTED == TRUE) &&
    (bf_stats->bf_down_scaler.enable_val.scale_en) &&
    (bf_fw->bf_filter_cfg[BF_FILTER_TYPE_H_1].h_scale_en == TRUE)) {
    /* Enable downscaled input for H1 */
    bf_stats_enable_mask->fields.h_1_scale_sel = 0x1;
    bf_stats_enable_val->fields.h_1_scale_sel = 0x1;
  }

  /* Update FIR filter */
  if (bf_fir_filter_cfg->enable == TRUE) {
    bf_stats_enable_mask->fields.h_1_fir_en = 0x1;
    bf_stats_enable_val->fields.h_1_fir_en = 0x1;
    ret = bf_stats47_fill_h_fir_params(
      &bf_stats_filter_cfg->bf_stats_h_1_fir_cfg,
      bf_fir_filter_cfg);
    if (ret == FALSE) {
      ISP_ERR("failed: bf_stats47_fill_h_fir_params for h1");
      func_ret = FALSE;
    }
  } else {
    bf_stats_enable_mask->fields.h_1_fir_en = 0x1;
    bf_stats_enable_val->fields.h_1_fir_en = 0x0;
  }

  /* Update IIR filter */
  if (bf_iir_filter_cfg->enable == TRUE) {
    bf_stats_enable_mask->fields.h_1_iir_en = 0x1;
    bf_stats_enable_val->fields.h_1_iir_en = 0x1;
    ret = bf_stats47_fill_h_iir_params(
      &bf_stats_filter_cfg->bf_stats_h_1_iir_cfg,
      bf_iir_filter_cfg);
    if (ret == FALSE) {
      ISP_ERR("failed: bf_stats47_fill_h_iir_params for h1");
      func_ret = FALSE;
    }
  } else {
    bf_stats_enable_mask->fields.h_1_iir_en = 0x1;
    bf_stats_enable_val->fields.h_1_iir_en = 0x0;
  }
  /* Update H_1 filter End */

  /* Update H_2 filter Start */
  bf_fir_filter_cfg =
    &bf_fw->bf_filter_cfg[BF_FILTER_TYPE_H_2].bf_fir_filter_cfg;
  bf_iir_filter_cfg =
    &bf_fw->bf_filter_cfg[BF_FILTER_TYPE_H_2].bf_iir_filter_cfg;

  /* Update whether H2 will receive scaled / unscaled input */
  /* Check whether downscaler is supported and enabled */
  /* Check whether H2 needs downscaled input */
  if ((BF_DOWN_SCALER_SUPPORTED == TRUE) &&
    (bf_stats->bf_down_scaler.enable_val.scale_en) &&
    (bf_fw->bf_filter_cfg[BF_FILTER_TYPE_H_2].h_scale_en == TRUE)) {
    /* Enable downscaled input for H1 */
    bf_stats_enable_mask->fields.h_2_scale_sel = 0x1;
    bf_stats_enable_val->fields.h_2_scale_sel = 0x1;
  }

  /* Update FIR filter */
  if (bf_fir_filter_cfg->enable == TRUE) {
    bf_stats_enable_mask->fields.h_2_fir_en = 0x1;
    bf_stats_enable_val->fields.h_2_fir_en = 0x1;
    ret = bf_stats47_fill_h_fir_params(
      &bf_stats_filter_cfg->bf_stats_h_2_fir_cfg,
      bf_fir_filter_cfg);
    if (ret == FALSE) {
      ISP_ERR("failed: bf_stats47_fill_h_fir_params for h1");
      func_ret = FALSE;
    }
  } else {
    bf_stats_enable_mask->fields.h_2_fir_en = 0x1;
    bf_stats_enable_val->fields.h_2_fir_en = 0x0;
  }

  /* Update IIR filter */
  if (bf_iir_filter_cfg->enable == TRUE) {
    bf_stats_enable_mask->fields.h_2_iir_en = 0x1;
    bf_stats_enable_val->fields.h_2_iir_en = 0x1;
    ret = bf_stats47_fill_h_iir_params(
      &bf_stats_filter_cfg->bf_stats_h_2_iir_cfg,
      bf_iir_filter_cfg);
    if (ret == FALSE) {
      ISP_ERR("failed: bf_stats47_fill_h_iir_params for h1");
      func_ret = FALSE;
    }
  } else {
    bf_stats_enable_mask->fields.h_2_iir_en = 0x1;
    bf_stats_enable_val->fields.h_2_iir_en = 0x0;
  }
  /* Update H_2 filter End */

  /* Update V filter Start */
  bf_fir_filter_cfg =
    &bf_fw->bf_filter_cfg[BF_FILTER_TYPE_V].bf_fir_filter_cfg;
  bf_iir_filter_cfg =
    &bf_fw->bf_filter_cfg[BF_FILTER_TYPE_V].bf_iir_filter_cfg;

  /* Update FIR filter */
  if (bf_fir_filter_cfg->enable == TRUE) {
    bf_stats_enable_mask->fields.v_fir_en = 0x1;
    bf_stats_enable_val->fields.v_fir_en = 0x1;
    ret = bf_stats47_fill_v_fir_params(
      &bf_stats_filter_cfg->bf_stats_v_fir_cfg,
      bf_fir_filter_cfg);
    if (ret == FALSE) {
      ISP_ERR("failed: bf_stats47_fill_v_fir_params for v");
      func_ret = FALSE;
    }
  } else {
    bf_stats_enable_mask->fields.v_fir_en = 0x1;
    bf_stats_enable_val->fields.v_fir_en = 0x0;
  }

  /* Update IIR filter */
  if (bf_iir_filter_cfg->enable == TRUE) {
    bf_stats_enable_mask->fields.v_iir_en = 0x1;
    bf_stats_enable_val->fields.v_iir_en = 0x1;
    ret = bf_stats47_fill_v_iir_params(
      &bf_stats_filter_cfg->bf_stats_v_iir_cfg,
      bf_iir_filter_cfg);
    if (ret == FALSE) {
      ISP_ERR("failed: bf_stats47_fill_v_iir_params for V");
      func_ret = FALSE;
    }
    #ifdef __BF_STATS41_REG_H__
      ret = bf_stats47_fill_v_iir1_params(
        &bf_stats_filter_cfg->bf_stats_v_iir_cfg1,
        bf_iir_filter_cfg);
        if (ret == FALSE) {
          ISP_ERR("failed: bf_stats47_fill_v_iir_params for V1");
          func_ret = FALSE;
        }
    #endif
  } else {
    bf_stats_enable_mask->fields.v_iir_en = 0x1;
    bf_stats_enable_val->fields.v_iir_en = 0x0;
  }
  /* Update V filter End */

  /* Update shift bits Start */
  bf_stats_shift_bits_cfg = &bf_stats_filter_cfg->bf_stats_shift_bits_cfg;

  bf_stats_shift_bits_cfg->h_1 = bf_stats47_adjust_signbit(
    bf_fw->bf_filter_cfg[BF_FILTER_TYPE_H_1].shift_bits, SHIFTER_BITS);
  bf_stats_shift_bits_cfg->h_2 = bf_stats47_adjust_signbit(
    bf_fw->bf_filter_cfg[BF_FILTER_TYPE_H_2].shift_bits, SHIFTER_BITS);
  bf_stats_shift_bits_cfg->v = bf_stats47_adjust_signbit(
    bf_fw->bf_filter_cfg[BF_FILTER_TYPE_V].shift_bits, SHIFTER_BITS);

  /* Update shift bits End */

  /* Update H_1 coring config Start */
  ret = bf_stats47_fill_coring_params(
    &bf_stats_filter_cfg->bf_stats_h_1_coring_cfg,
    &bf_fw->bf_filter_cfg[BF_FILTER_TYPE_H_1].bf_filter_coring_cfg);
  if (ret == FALSE) {
    ISP_ERR("failed: bf_stats47_fill_coring_params for h1");
    func_ret = FALSE;
  }
  /* Update H_1 coring config End */

  /* Update H_2 coring config Start */
  ret = bf_stats47_fill_coring_params(
    &bf_stats_filter_cfg->bf_stats_h_2_coring_cfg,
    &bf_fw->bf_filter_cfg[BF_FILTER_TYPE_H_2].bf_filter_coring_cfg);
  if (ret == FALSE) {
    ISP_ERR("failed: bf_stats47_fill_coring_params for h2");
    func_ret = FALSE;
  }
  /* Update H_2 coring config End */

  /* Update V coring config Start */
  ret = bf_stats47_fill_coring_params(
    &bf_stats_filter_cfg->bf_stats_v_coring_cfg,
    &bf_fw->bf_filter_cfg[BF_FILTER_TYPE_V].bf_filter_coring_cfg);
  if (ret == FALSE) {
    ISP_ERR("failed: bf_stats47_fill_coring_params for v");
    func_ret = FALSE;
  }
  /* Update V coring config End */

  /* Update Coring gain cfg Start */
  bf_stats_coring_gain_cfg = &bf_stats_filter_cfg->bf_stats_coring_gain_cfg;

  bf_stats_coring_gain_cfg->h_1_gain =
    (bf_fw->bf_filter_cfg[BF_FILTER_TYPE_H_1].bf_filter_coring_cfg.gain &
    CORING_BITS);
  bf_stats_coring_gain_cfg->h_2_gain =
    (bf_fw->bf_filter_cfg[BF_FILTER_TYPE_H_2].bf_filter_coring_cfg.gain &
    CORING_BITS);
  bf_stats_coring_gain_cfg->v_gain =
    (bf_fw->bf_filter_cfg[BF_FILTER_TYPE_V].bf_filter_coring_cfg.gain &
     CORING_BITS);

  /* Update Coring gain cfg End */

  return func_ret;
}

/** bf_stats47_debug_filter_config:
 *
 *  @bf_stats_filter_cfg: Pointer to the reg_cmd struct that
 *        needs to be dumped
 *
 *  Print the value of the parameters in reg_cmd
 *
 * Return void
 **/
static void bf_stats47_debug_filter_config(
  bf_stats_filter_cfg_t *bf_stats_filter_cfg)
{
  bf_stats_h_fir_cfg_t       *bf_stats_h_fir_cfg = NULL;
  bf_stats_h_iir_cfg_t       *bf_stats_h_iir_cfg = NULL;
  bf_stats_v_fir_cfg_t       *bf_stats_v_fir_cfg = NULL;
  bf_stats_v_iir_cfg_t       *bf_stats_v_iir_cfg = NULL;

  #ifdef __BF_STATS41_REG_H__
    bf_stats_v_iir_cfg1_t      *bf_stats_v_iir_cfg1 = NULL;
  #endif
  if (!bf_stats_filter_cfg) {
    ISP_ERR("failed bf_stats_filter_cfg %p", bf_stats_filter_cfg);
    return;
  }
  /* Print debug cmd */
  /* H1 cfg */
  /* H1 FIR cfg */
  bf_stats_h_fir_cfg = &bf_stats_filter_cfg->bf_stats_h_1_fir_cfg;
  ISP_DBG("h1 fir a = 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x "
"0x%x 0x%x",
    bf_stats_h_fir_cfg->a00, bf_stats_h_fir_cfg->a01, bf_stats_h_fir_cfg->a02,
    bf_stats_h_fir_cfg->a03, bf_stats_h_fir_cfg->a04, bf_stats_h_fir_cfg->a05,
    bf_stats_h_fir_cfg->a06, bf_stats_h_fir_cfg->a07, bf_stats_h_fir_cfg->a08,
    bf_stats_h_fir_cfg->a09, bf_stats_h_fir_cfg->a10, bf_stats_h_fir_cfg->a11,
    bf_stats_h_fir_cfg->a12);
  /* H1 IIR cfg */
  bf_stats_h_iir_cfg = &bf_stats_filter_cfg->bf_stats_h_1_iir_cfg;
  ISP_DBG("h1 iir b10 = 0x%x b11 = 0x%x b12 = 0x%x a11 = 0x%x a12 = 0x%x "
"b20 = 0x%x b21 = 0x%x b22 = 0x%x a21 = 0x%x a22 = 0x%x",
    bf_stats_h_iir_cfg->b10, bf_stats_h_iir_cfg->b11, bf_stats_h_iir_cfg->b12,
    bf_stats_h_iir_cfg->a11, bf_stats_h_iir_cfg->a12, bf_stats_h_iir_cfg->b20,
    bf_stats_h_iir_cfg->b21, bf_stats_h_iir_cfg->b22, bf_stats_h_iir_cfg->a21,
    bf_stats_h_iir_cfg->a22);
  /* H2 cfg */
  /* H2 FIR cfg */
  bf_stats_h_fir_cfg = &bf_stats_filter_cfg->bf_stats_h_2_fir_cfg;
  ISP_DBG("h2 fir a = 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x "
"0x%x 0x%x",
    bf_stats_h_fir_cfg->a00, bf_stats_h_fir_cfg->a01, bf_stats_h_fir_cfg->a02,
    bf_stats_h_fir_cfg->a03, bf_stats_h_fir_cfg->a04, bf_stats_h_fir_cfg->a05,
    bf_stats_h_fir_cfg->a06, bf_stats_h_fir_cfg->a07, bf_stats_h_fir_cfg->a08,
    bf_stats_h_fir_cfg->a09, bf_stats_h_fir_cfg->a10, bf_stats_h_fir_cfg->a11,
    bf_stats_h_fir_cfg->a12);
  /* H2 IIR cfg */
  bf_stats_h_iir_cfg = &bf_stats_filter_cfg->bf_stats_h_2_iir_cfg;
  ISP_DBG("h2 iir b10 = 0x%x b11 = 0x%x b12 = %x a11 = %x a12 = %x b20 = %x "
"b21 = %x b22 = %x a21 = %x a22 = %x",
    bf_stats_h_iir_cfg->b10, bf_stats_h_iir_cfg->b11, bf_stats_h_iir_cfg->b12,
    bf_stats_h_iir_cfg->a11, bf_stats_h_iir_cfg->a12, bf_stats_h_iir_cfg->b20,
    bf_stats_h_iir_cfg->b21, bf_stats_h_iir_cfg->b22, bf_stats_h_iir_cfg->a21,
    bf_stats_h_iir_cfg->a22);
  /* V cfg */
  /* V FIR cfg */
  bf_stats_v_fir_cfg = &bf_stats_filter_cfg->bf_stats_v_fir_cfg;
  ISP_DBG("v fir a = 0x%x 0x%x 0x%x 0x%x 0x%x",
    bf_stats_v_fir_cfg->a00, bf_stats_v_fir_cfg->a01, bf_stats_v_fir_cfg->a02,
    bf_stats_v_fir_cfg->a03, bf_stats_v_fir_cfg->a04);
  /* V IIR cfg */
  bf_stats_v_iir_cfg = &bf_stats_filter_cfg->bf_stats_v_iir_cfg;
  ISP_DBG("v iir b10 = 0x%x b11 = 0x%x b12 = 0x%x a11 = 0x%x a12 = 0x%x",
    bf_stats_v_iir_cfg->b10, bf_stats_v_iir_cfg->b11, bf_stats_v_iir_cfg->b12,
    bf_stats_v_iir_cfg->a11, bf_stats_v_iir_cfg->a12);
  #ifdef __BF_STATS41_REG_H__
    /* V IIR cfg 1*/
    bf_stats_v_iir_cfg1 = &bf_stats_filter_cfg->bf_stats_v_iir_cfg1;
      ISP_DBG("v iir b20 = 0x%x b21 = 0x%x b22 = 0x%x a21 = 0x%x a22 = 0x%x",
      bf_stats_v_iir_cfg1->b20, bf_stats_v_iir_cfg1->b21, bf_stats_v_iir_cfg1->b22,
      bf_stats_v_iir_cfg1->a21, bf_stats_v_iir_cfg1->a22);
  #endif
}

/** bf_stats47_write_filter_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @bf_stats: bf stats handle
 *
 *  Prepare hw update list for filter config
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bf_stats47_write_filter_config(isp_sub_module_t *isp_sub_module,
  bf_stats47_t *bf_stats)
{
  boolean                      ret = TRUE;
  bf_stats_filter_cfg_t       *bf_stats_filter_cfg = NULL;
  bf_stats_filter_cfg_t       *copy_bf_stats_filter_cfg = NULL;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;

  if (!isp_sub_module || !bf_stats) {
    ISP_ERR("failed: %p %p", isp_sub_module, bf_stats);
    return FALSE;
  }

  bf_stats_filter_cfg = &bf_stats->pcmd.bf_stats_filter_cfg;
  bf_stats47_debug_filter_config(bf_stats_filter_cfg);

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_CALLOC(sizeof(*hw_update), 1);
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)CAM_CALLOC(sizeof(*reg_cfg_cmd), 1);
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_CMD;
  }

  copy_bf_stats_filter_cfg =
    (bf_stats_filter_cfg_t *)CAM_CALLOC(sizeof(*copy_bf_stats_filter_cfg), 1);
  if (!copy_bf_stats_filter_cfg) {
    ISP_ERR("failed: copy_bf_stats_filter_cfg %p", copy_bf_stats_filter_cfg);
    goto ERROR_COPY_CMD;
  }

  *copy_bf_stats_filter_cfg = *bf_stats_filter_cfg;
  cfg_cmd = &hw_update->cfg_cmd;
  cfg_cmd->cfg_data = (void *)copy_bf_stats_filter_cfg;
  cfg_cmd->cmd_len = sizeof(*copy_bf_stats_filter_cfg);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = 1;

  reg_cfg_cmd->cmd_type = VFE_WRITE;
  reg_cfg_cmd->u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd->u.rw_info.reg_offset = BF_STATS_OFF_0;
  reg_cfg_cmd->u.rw_info.len = sizeof(*copy_bf_stats_filter_cfg);

  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update");
    goto ERROR_APPEND;
  }

  return TRUE;

ERROR_APPEND:
  free(copy_bf_stats_filter_cfg);
ERROR_COPY_CMD:
  free(reg_cfg_cmd);
ERROR_REG_CFG_CMD:
  free(hw_update);
  return FALSE;
}

/** bf_stats47_store_dmi_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @bf_stats: bf stats handle
 *
 *  Store DMI roi config from 3a event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bf_stats47_store_dmi_config(isp_sub_module_t *isp_sub_module,
  bf_stats47_t *bf_stats)
{
  boolean                     ret = TRUE;
  uint64_t                   *packed_roi_dmi_cfg = NULL;
  uint32_t                    i = 0, j = 0, k = 0;
  bf_fw_roi_cfg_t            *bf_fw_roi_cfg = NULL;

  if (!isp_sub_module || !bf_stats) {
    ISP_ERR("failed: %p %p", isp_sub_module, bf_stats);
    return FALSE;
  }

  bf_fw_roi_cfg = &bf_stats->bf_hw_roi_cfg;

  /* Validate bf_fw_roi_cfg->num_bf_fw_roi_dim */
  if (bf_fw_roi_cfg->num_bf_fw_roi_dim >= BF_FW_MAX_ROI_REGIONS) {
    ISP_ERR("failed: invalid num_bf_fw_roi_dim %d",
      bf_fw_roi_cfg->num_bf_fw_roi_dim);
    return FALSE;
  }

  /* Update no. of roi's entries */
  bf_stats->num_roi = bf_fw_roi_cfg->num_bf_fw_roi_dim;

  /* pack DMI */
  packed_roi_dmi_cfg = bf_stats->packed_roi_dmi_cfg;

  if (BF_STATS_VERSION == 48) {
    /* In pipeline 48 version of BF stats, frame tag is written as first
     * of DMI
     */
    packed_roi_dmi_cfg[i++] = bf_stats->af_config.config_id;
    ++bf_stats->num_roi;

    for (j = 0; j < bf_fw_roi_cfg->num_bf_fw_roi_dim; i++, j++) {

      packed_roi_dmi_cfg[i] =
        (((uint64_t)bf_fw_roi_cfg->bf_fw_roi_dim[j].region) <<
        DMI_I_SHIFT) |
        ((((uint64_t)bf_fw_roi_cfg->bf_fw_roi_dim[j].region_num) & DMI_ID_BITS)
        << DMI_ID_SHIFT) |
        ((((uint64_t)bf_fw_roi_cfg->bf_fw_roi_dim[j].x) & DMI_X_BITS) <<
        DMI_X_SHIFT) |
        ((((uint64_t)bf_fw_roi_cfg->bf_fw_roi_dim[j].y) & DMI_Y_BITS) <<
        DMI_Y_SHIFT) |
        ((((uint64_t)bf_fw_roi_cfg->bf_fw_roi_dim[j].w) & DMI_W_BITS) <<
        DMI_W_SHIFT) |
        ((((uint64_t)bf_fw_roi_cfg->bf_fw_roi_dim[j].h) & DMI_H_BITS)) <<
        DMI_H_SHIFT;
    }

    /* Write 0, 0, 0, 0, 0 to indicate last region */
    packed_roi_dmi_cfg[i] = 0;
  } else {

    for (i = 0, j = 0; j < bf_fw_roi_cfg->num_bf_fw_roi_dim; i++, j++) {

      packed_roi_dmi_cfg[i] =
        (((uint64_t)bf_fw_roi_cfg->bf_fw_roi_dim[j].region) <<
        DMI_I_SHIFT) |
        ((((uint64_t)bf_fw_roi_cfg->bf_fw_roi_dim[j].x) & DMI_X_BITS) <<
        DMI_X_SHIFT) |
        ((((uint64_t)bf_fw_roi_cfg->bf_fw_roi_dim[j].y) & DMI_Y_BITS) <<
        DMI_Y_SHIFT) |
        ((((uint64_t)bf_fw_roi_cfg->bf_fw_roi_dim[j].w) & DMI_W_BITS) <<
        DMI_W_SHIFT) |
        ((((uint64_t)bf_fw_roi_cfg->bf_fw_roi_dim[j].h) & DMI_H_BITS)) <<
        DMI_H_SHIFT;
    }

    /* Write last DMI config twice indicate to hardware that this is the last
     * region
     */
    packed_roi_dmi_cfg[i] =
      (((uint64_t)bf_fw_roi_cfg->bf_fw_roi_dim[j-1].region) <<
      DMI_I_SHIFT) |
      ((((uint64_t)bf_fw_roi_cfg->bf_fw_roi_dim[j-1].x) & DMI_X_BITS) <<
      DMI_X_SHIFT) |
      ((((uint64_t)bf_fw_roi_cfg->bf_fw_roi_dim[j-1].y) & DMI_Y_BITS) <<
      DMI_Y_SHIFT) |
      ((((uint64_t)bf_fw_roi_cfg->bf_fw_roi_dim[j-1].w) & DMI_W_BITS) <<
      DMI_W_SHIFT) |
      ((((uint64_t)bf_fw_roi_cfg->bf_fw_roi_dim[j-1].h) & DMI_H_BITS)) <<
      DMI_H_SHIFT;
  }

  /* Increment num_roi by 1 to account for last dmi config */
  bf_stats->num_roi++;

  return TRUE;
}

/** bf_stats47_debug_dmi_config:
 *
 *  @bf_stats: bf stats handle
 *
 *  Print DMI config
 *
 *  Return void
 **/
static void bf_stats47_debug_dmi_config(bf_stats47_t *bf_stats)
{
  uint64_t            *packed_roi_dmi_cfg = NULL;
  bf_fw_roi_cfg_t *bf_fw_roi_cfg = NULL;
  uint32_t             i = 0;

  if (!bf_stats) {
    ISP_ERR("failed: bf_stats %p", bf_stats);
    return;
  }

  bf_fw_roi_cfg = &bf_stats->af_config.bf_fw.bf_fw_roi_cfg;
  ISP_DBG("3A ROI config start");
  for (i = 0; i < bf_fw_roi_cfg->num_bf_fw_roi_dim; i++) {
    ISP_DBG("i %d p/s %d x %d y %d w %d h %d", i,
      bf_fw_roi_cfg->bf_fw_roi_dim[i].region,
      bf_fw_roi_cfg->bf_fw_roi_dim[i].x,
      bf_fw_roi_cfg->bf_fw_roi_dim[i].y,
      bf_fw_roi_cfg->bf_fw_roi_dim[i].w,
      bf_fw_roi_cfg->bf_fw_roi_dim[i].h);
  }
  ISP_DBG("3A ROI config start");

  ISP_DBG("DMI ROI config start");
  for (i = 0; i < bf_stats->num_roi; i++) {
    packed_roi_dmi_cfg = &bf_stats->packed_roi_dmi_cfg[i];
    ISP_DBG("i %d p/s %d x %d y %d w %d h %d", i,
      (uint32_t)(*packed_roi_dmi_cfg >> DMI_I_SHIFT),
      (uint32_t)((*packed_roi_dmi_cfg >> DMI_X_SHIFT) & DMI_X_BITS),
      (uint32_t)((*packed_roi_dmi_cfg >> DMI_Y_SHIFT) & DMI_Y_BITS),
      (uint32_t)((*packed_roi_dmi_cfg >> DMI_W_SHIFT) & DMI_W_BITS),
      (uint32_t)((*packed_roi_dmi_cfg >> DMI_H_SHIFT) & DMI_H_BITS));
  }
  ISP_DBG("DMI ROI config start");
}
/** bf_stats47_debug_dmi_config:
 *
 *  @bf_stats: bf stats handle
 *
 *  Print DMI config
 *
 *  Return void
 **/
static void bf_stats47_debug_roi_config(bf_fw_roi_cfg_t *bf_fw_roi_cfg)
{
  uint32_t             i = 0;

  for (i = 0; i < bf_fw_roi_cfg->num_bf_fw_roi_dim; i++) {
    ISP_DBG("i %d p/s %d x %d y %d w %d h %d", i,
      bf_fw_roi_cfg->bf_fw_roi_dim[i].region,
      bf_fw_roi_cfg->bf_fw_roi_dim[i].x,
      bf_fw_roi_cfg->bf_fw_roi_dim[i].y,
      bf_fw_roi_cfg->bf_fw_roi_dim[i].w,
      bf_fw_roi_cfg->bf_fw_roi_dim[i].h);
  }
}

/** bf_stats47_write_dmi_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @bf_stats: bf stats handle
 *  @sub_module_output: sub module output handle to store hw
 *                    update list
 *
 *  Prepare hw update list for DMI ROI config
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bf_stats47_write_dmi_config(isp_sub_module_t *isp_sub_module,
  bf_stats47_t *bf_stats, isp_sub_module_output_t *sub_module_output)
{
  boolean ret = TRUE;

  if (!isp_sub_module || !bf_stats || !sub_module_output) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, bf_stats, sub_module_output);
    return FALSE;
  }

  bf_stats47_debug_dmi_config(bf_stats);

  ret = isp_sub_module_util_write_dmi((void*)bf_stats->packed_roi_dmi_cfg,
    sizeof(uint64_t) * bf_stats->num_roi,
    STATS_BAF_RGN_IND_LUT_BANK0 +
    bf_stats->pcmd.bf_stats_enable_val.fields.roi_ind_lut_bank_sel,
    VFE_WRITE_DMI_64BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_write_dmi");
  }

  return ret;
}

/** bf_stats47_write_y_conv_config:
 *
 *  @isp_sub_module: handle to isp_sub_module
 *  @bf_stats: handle to bf_stats
 *  @sub_module_output: handle to sub_module_output
 *
 *  Prepare hw update list for input config
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bf_stats47_write_y_conv_config(isp_sub_module_t *isp_sub_module,
  bf_stats47_t *bf_stats, isp_sub_module_output_t *sub_module_output)
{
  boolean                      ret = TRUE;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  bf_stats_y_conv_t           *bf_stats_y_conv = NULL;

  if (!isp_sub_module || !bf_stats || !sub_module_output) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, bf_stats, sub_module_output);
    return FALSE;
  }

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_CALLOC(sizeof(*hw_update), 1);
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }

  cfg_cmd = &hw_update->cfg_cmd;

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)CAM_CALLOC(sizeof(*reg_cfg_cmd), 1);
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    ret = FALSE;
  }

  bf_stats_y_conv = (bf_stats_y_conv_t *)CAM_CALLOC(sizeof(*bf_stats_y_conv), 1);
  if (!bf_stats_y_conv) {
    ISP_ERR("failed: bf_stats_y_conv %p", bf_stats_y_conv);
    free(reg_cfg_cmd);
    free(hw_update);
    return FALSE;
  }
  memcpy(bf_stats_y_conv, &bf_stats->pcmd.bf_stats_y_conv,
    sizeof(*bf_stats_y_conv));

  if (ret == TRUE) {
    cfg_cmd->cfg_data = (void *)bf_stats_y_conv;
    cfg_cmd->cmd_len = sizeof(*bf_stats_y_conv);
    cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
    cfg_cmd->num_cfg = 1;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = STATS_BAF_Y_CONV_ADDR;
    reg_cfg_cmd[0].u.rw_info.len = sizeof(*bf_stats_y_conv);
    ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
    ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_sub_module_util_store_hw_update");
    }
  }

  if (ret == FALSE) {
    ISP_ERR("failed: STATS_BAF_Y_CONV_ADDR");
    free(bf_stats_y_conv);
    free(reg_cfg_cmd);
    free(hw_update);
  }
  return ret;
}
/** bf_stats47_debug_baf_stats_cfg_mask:
 *
 *  @pcmd: pcmd handle
 *
 *  Print baf stats cfg mask and val
 *
 *  Return void
 **/
static void bf_stats47_debug_baf_stats_cfg_mask(bf_stats_reg_cfg_t *pcmd)
{
  if (!pcmd) {
    ISP_ERR("failed: pcmd %p", pcmd);
    return;
  }
  ISP_DBG("mask:val g_sel=%d:%d gamma_lut_en %d:%d gamma_lut_bank_sel %d:%d",
    pcmd->bf_stats_enable_mask.fields.g_sel,
    pcmd->bf_stats_enable_val.fields.g_sel,
    pcmd->bf_stats_enable_mask.fields.gamma_lut_en,
    pcmd->bf_stats_enable_val.fields.gamma_lut_en,
    pcmd->bf_stats_enable_mask.fields.gamma_lut_bank_sel,
    pcmd->bf_stats_enable_val.fields.gamma_lut_bank_sel);
  ISP_DBG("mask:val ch_sel=%d:%d scale_en %d:%d h_1_scale_sel %d:%d",
    pcmd->bf_stats_enable_mask.fields.ch_sel,
    pcmd->bf_stats_enable_val.fields.ch_sel,
    pcmd->bf_stats_enable_mask.fields.scale_en,
    pcmd->bf_stats_enable_val.fields.scale_en,
    pcmd->bf_stats_enable_mask.fields.h_1_scale_sel,
    pcmd->bf_stats_enable_val.fields.h_1_scale_sel);
  ISP_DBG("mask:val h_2_scale_sel=%d:%d h_1_fir_en %d:%d h_1_iir_en %d:%d",
    pcmd->bf_stats_enable_mask.fields.h_2_scale_sel,
    pcmd->bf_stats_enable_val.fields.h_2_scale_sel,
    pcmd->bf_stats_enable_mask.fields.h_1_fir_en,
    pcmd->bf_stats_enable_val.fields.h_1_fir_en,
    pcmd->bf_stats_enable_mask.fields.h_1_iir_en,
    pcmd->bf_stats_enable_val.fields.h_1_iir_en);
  ISP_DBG("mask:val h_2_fir_en=%d:%d h_2_iir_en %d:%d v_fir_en %d:%d",
    pcmd->bf_stats_enable_mask.fields.h_2_fir_en,
    pcmd->bf_stats_enable_val.fields.h_2_fir_en,
    pcmd->bf_stats_enable_mask.fields.h_2_iir_en,
    pcmd->bf_stats_enable_val.fields.h_2_iir_en,
    pcmd->bf_stats_enable_mask.fields.v_fir_en,
    pcmd->bf_stats_enable_val.fields.v_fir_en);
  ISP_DBG("mask:val v_iir_en=%d:%d roi_ind_lut_bank_sel %d:%d",
    pcmd->bf_stats_enable_mask.fields.v_iir_en,
    pcmd->bf_stats_enable_val.fields.v_iir_en,
    pcmd->bf_stats_enable_mask.fields.roi_ind_lut_bank_sel,
    pcmd->bf_stats_enable_val.fields.roi_ind_lut_bank_sel);
}

/** bf_stats47_write_baf_stats_cfg_mask:
 *
 *  @isp_sub_module: handle to isp_sub_module
 *  @bf_stats: handle to bf_stats
 *  @sub_module_output: handle to sub_module_output
 *
 *  Write STATS_CFG register as VFE_MASK
 *
 *  Return TRUE on succes and FALSE on failure
 **/
static boolean bf_stats47_write_baf_stats_cfg_mask(
  isp_sub_module_t *isp_sub_module, bf_stats47_t *bf_stats,
  isp_sub_module_output_t *sub_module_output)
{
  boolean                      ret = TRUE;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  uint32_t                    *val = NULL;

  if (!isp_sub_module || !bf_stats || !sub_module_output) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, bf_stats, sub_module_output);
    return FALSE;
  }

  bf_stats47_debug_baf_stats_cfg_mask(&bf_stats->pcmd);

  /* Create VFE mask */
  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_CALLOC(sizeof(*hw_update), 1);
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }

  cfg_cmd = &hw_update->cfg_cmd;

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)CAM_CALLOC(sizeof(*reg_cfg_cmd), 1);
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    ret = FALSE;
  }

  if (ret == TRUE) {
    cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
    cfg_cmd->num_cfg = 1;

    reg_cfg_cmd[0].cmd_type = VFE_CFG_MASK;
    reg_cfg_cmd[0].u.mask_info.reg_offset = STATS_BAF_CFG_ADDR;
    val = (uint32_t *)&bf_stats->pcmd.bf_stats_enable_mask.value;
    reg_cfg_cmd[0].u.mask_info.mask = *val;
    val = (uint32_t *)&bf_stats->pcmd.bf_stats_enable_val.value;
    reg_cfg_cmd[0].u.mask_info.val = *val;

    ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
    ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_sub_module_util_store_hw_update");
    }
  }

  if (ret == FALSE) {
    free(reg_cfg_cmd);
    free(hw_update);
  }

  return ret;
}

/** bf_stats47_chk_adj_strt_pix_roi_lim:
 *
 *  @bf_stats: bf stats handle
 *  @roi_cfg: bf stats roi cfg
 *
 *  Check whether both ROI starts on same pixel or starting line
 *  apart by min.
 *
 *  Return TRUE on success and FALSE on failure
 **/

static boolean bf_stats47_chk_sum_of_y_coeff(bf_input_cfg_t *bf_input_cfg)
{
  float a0, a1, a2;
  boolean ret = FALSE;
  a0 = bf_input_cfg->y_a_cfg[0];
  a1 = bf_input_cfg->y_a_cfg[1];
  a2 = bf_input_cfg->y_a_cfg[2];
  if ((a0 + a1 + a2) <=1.0f )
    ret = TRUE;

  return ret;
}


/** bf_stats47_input_config:
 *
 *  @isp_sub_module: handle to isp_sub_module
 *  @bf_stats: handle to bf_stats
 *  @sub_module_output: handle to sub_module_output
 *
 *  Configure input sel
 *
 *  Return TRUE on succes and FALSE on failure
 **/
static boolean bf_stats47_input_config(isp_sub_module_t *isp_sub_module,
  bf_stats47_t *bf_stats, isp_sub_module_output_t *sub_module_output)
{
  boolean                 ret = TRUE,
                          func_ret = TRUE;
  bf_stats_y_conv_t      *bf_stats_y_conv = NULL;
  bf_stats_enable_t      *bf_stats_enable_mask = NULL;
  bf_stats_enable_t      *bf_stats_enable_val = NULL;
  bf_input_cfg_t         *bf_input_cfg = NULL;
  bf_gamma_lut_cfg_t     *bf_gamma_lut_cfg = NULL;
  bf_gamma_cfg_t         *bf_gamma_cfg = NULL;
  boolean                force_y_channel = FALSE, override_y_coeff = FALSE;
  uint32_t               temp_coeff = 0;
  boolean                lsb_allignment = FALSE;

  if (!isp_sub_module || !bf_stats || !sub_module_output) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, bf_stats, sub_module_output);
    return FALSE;
  }

  bf_stats_enable_mask = &bf_stats->pcmd.bf_stats_enable_mask;
  bf_stats_enable_val = &bf_stats->pcmd.bf_stats_enable_val;
  bf_stats_y_conv = &bf_stats->pcmd.bf_stats_y_conv;
  bf_input_cfg = &bf_stats->af_config.bf_fw.bf_input_cfg;
  bf_gamma_cfg = &bf_stats->bf_gamma_cfg;
  bf_gamma_lut_cfg = &bf_gamma_cfg->bf_gamma_lut_cfg;
  bf_stats->gamma_downscale_factor = 1;

  if (isp_sub_module->hdr_mode == CAM_SENSOR_HDR_IN_SENSOR ||
      isp_sub_module->hdr_mode == CAM_SENSOR_HDR_ZIGZAG ){
    lsb_allignment = TRUE;
  }

  if (BF_GAMMA_USE_Y_CHANNEL == TRUE ||
    lsb_allignment) {
    if ((bf_gamma_lut_cfg->is_valid) &&
         bf_input_cfg->bf_channel_sel == BF_CHANNEL_SELECT_G) {
      force_y_channel = TRUE;
      if (lsb_allignment) {
        /* In HDR mode only LSB's of 10 bit is valid, Gamma lookup is
           indexed by MSB's of 5 bits, SO in zzHDR mode only 1 bit is
           valid. To convert 5 bit index to 2 bit index deviding by 8
        */
        bf_stats->gamma_downscale_factor = 8;
      } else {
        bf_stats->gamma_downscale_factor = 2;
      }
      ISP_DBG("Adjusting Y COEFF force_y_channel %d",force_y_channel);
    }
    if (bf_input_cfg->bf_channel_sel == BF_CHANNEL_SELECT_Y) {
      if (bf_stats47_chk_sum_of_y_coeff(bf_input_cfg) &&
          bf_gamma_lut_cfg->is_valid) {
        override_y_coeff = TRUE;
        if (lsb_allignment) {
          bf_stats->gamma_downscale_factor = 8;
        } else {
          bf_stats->gamma_downscale_factor = 2;
        }
        ISP_DBG(" Overwriting Y Coeff %d",override_y_coeff);
      } else if (!bf_stats47_chk_sum_of_y_coeff(bf_input_cfg) &&
                  bf_gamma_lut_cfg->is_valid) {
        override_y_coeff = FALSE;
        if (lsb_allignment) {
          /* In zzHDR mode only LSB's of 10 bit is valid, Gamma lookup is
           indexed by MSB's of 5 bits, SO in zzHDR mode only 1 bit is
           valid. To convert 5 bit index to 1 bit index deviding by 16
         */
          bf_stats->gamma_downscale_factor = 16;
        } else {
          bf_stats->gamma_downscale_factor = 4;
        }
      }
    }
  }

  if (bf_input_cfg->bf_channel_sel == BF_CHANNEL_SELECT_Y) {
    bf_stats_enable_mask->fields.ch_sel = 0x1;
    bf_stats_enable_val->fields.ch_sel = STATS_BAF_CFG_CH_SEL_Y;

    bf_stats_y_conv->a00 = (override_y_coeff ?
           (FLOAT_TO_Q(STATS_BAF_Y_CONV_CFG_SHIFT,
           (float) (bf_input_cfg->y_a_cfg[0] * 1.999f))) :
           (FLOAT_TO_Q(STATS_BAF_Y_CONV_CFG_SHIFT,(bf_input_cfg->y_a_cfg[0]))));
    bf_stats_y_conv->a01 = (override_y_coeff ?
           (FLOAT_TO_Q(STATS_BAF_Y_CONV_CFG_SHIFT,
           (float) (bf_input_cfg->y_a_cfg[1] * 1.999f))):
           (FLOAT_TO_Q(STATS_BAF_Y_CONV_CFG_SHIFT,(bf_input_cfg->y_a_cfg[1]))));
    bf_stats_y_conv->a02 = (override_y_coeff ?
           (FLOAT_TO_Q(STATS_BAF_Y_CONV_CFG_SHIFT,
           (float) (bf_input_cfg->y_a_cfg[2] * 1.999f))) :
           (FLOAT_TO_Q(STATS_BAF_Y_CONV_CFG_SHIFT,(bf_input_cfg->y_a_cfg[2]))));

    if (bf_input_cfg->bf_input_g_sel == BF_INPUT_SELECT_GR) {
      bf_stats_enable_mask->fields.g_sel = 0x1;
      bf_stats_enable_val->fields.g_sel = STATS_BAF_CFG_G_SEL_GR;
    } else if (bf_input_cfg->bf_input_g_sel == BF_INPUT_SELECT_GB) {
      bf_stats_enable_mask->fields.g_sel = 0x1;
      bf_stats_enable_val->fields.g_sel = STATS_BAF_CFG_G_SEL_GB;
    } else {
      ISP_ERR("error: invalid bf_input_g_sel %d", bf_input_cfg->bf_input_g_sel);
    }
    ISP_DBG("A0 %d A1 %d A2 %d",bf_stats_y_conv->a00,
             bf_stats_y_conv->a01, bf_stats_y_conv->a02);
    ISP_DBG("ipA0 %f ipA1 %f ipA2 %f",bf_input_cfg->y_a_cfg[0],
             bf_input_cfg->y_a_cfg[1], bf_input_cfg->y_a_cfg[2]);

  } else if (bf_input_cfg->bf_channel_sel == BF_CHANNEL_SELECT_G) {
    if (force_y_channel){
      bf_stats_y_conv->a00 = FLOAT_TO_Q(STATS_BAF_Y_CONV_CFG_SHIFT, 0);
      bf_stats_y_conv->a01 = FLOAT_TO_Q(STATS_BAF_Y_CONV_CFG_SHIFT, 1.99);
      bf_stats_y_conv->a02 = FLOAT_TO_Q(STATS_BAF_Y_CONV_CFG_SHIFT, 0);
    }
    if (bf_input_cfg->bf_input_g_sel == BF_INPUT_SELECT_GR) {
      bf_stats_enable_mask->fields.ch_sel = 0x1;
      bf_stats_enable_val->fields.ch_sel = (force_y_channel ?
           STATS_BAF_CFG_CH_SEL_Y : STATS_BAF_CFG_CH_SEL_G);
      bf_stats_enable_mask->fields.g_sel = 0x1;
      bf_stats_enable_val->fields.g_sel = STATS_BAF_CFG_G_SEL_GR;
    } else if (bf_input_cfg->bf_input_g_sel == BF_INPUT_SELECT_GB) {
      bf_stats_enable_mask->fields.ch_sel = 0x1;
      bf_stats_enable_val->fields.ch_sel = (force_y_channel ?
           STATS_BAF_CFG_CH_SEL_Y : STATS_BAF_CFG_CH_SEL_G);
      bf_stats_enable_mask->fields.g_sel = 0x1;
      bf_stats_enable_val->fields.g_sel = STATS_BAF_CFG_G_SEL_GB;
    } else {
    ISP_ERR("error: invalid bf_input_g_sel %d", bf_input_cfg->bf_input_g_sel);
    }
  }
  ret = bf_stats47_write_y_conv_config(isp_sub_module, bf_stats,
      sub_module_output);
  if (ret == FALSE) {
      ISP_ERR("failed: bf_stats47_write_y_conv_config");
      func_ret = FALSE;
  }
  return func_ret;
}

/** bf_stats_are_regions_overlapping:
 *
 *  @roi_i_dim: ROI 1
 *  @roi_j_dim: ROI 2
 *
 *  Return true if ROI 1 and ROI 2 are overlapping. FALSE
 *  otherwise
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bf_stats_are_regions_overlapping(bf_fw_roi_dim_t *roi_i_dim,
  bf_fw_roi_dim_t *roi_j_dim)
{
  boolean                  ret = FALSE;
  uint32_t                 i_x = 0,
                           i_y = 0,
                           i_w = 0,
                           i_h = 0,
                           j_x = 0,
                           j_y = 0,
                           j_w = 0,
                           j_h = 0;
  i_x = roi_i_dim->x;
  i_y = roi_i_dim->y;
  i_w = roi_i_dim->w;
  i_h = roi_i_dim->h;

  j_x = roi_j_dim->x;
  j_y = roi_j_dim->y;
  j_w = roi_j_dim->w;
  j_h = roi_j_dim->h;

  /* Check whether i and j are overlapping */
  if ((j_x < (i_x + i_w)) && (j_y < (i_y + i_h))) {
    if ((j_x >= i_x) && (j_x < (i_x + i_w))) {
      ret = TRUE;
    } else if ((j_x + j_w) > i_x) {
      ret = TRUE;
    } else {
      ret = FALSE;
    }
  }
  return ret;
}
/** bf_stats47_chk_adj_strt_pix_roi_lim:
 *
 *  @bf_stats: bf stats handle
 *  @roi_cfg: bf stats roi cfg
 *
 *  Check whether both ROI starts on same pixel or starting line
 *  apart by min.
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bf_stats47_chk_adj_strt_pix_roi_lim(bf_fw_roi_cfg_t *roi_cfg,
  bf_stats47_t *bf_stats)
{
  boolean             ret = TRUE;
  uint32_t            i = 0,
                      j = 0;
  bf_fw_roi_dim_t    *roi_i_dim = NULL,
                     *roi_j_dim = NULL;

  if (!roi_cfg || !bf_stats) {
    ISP_ERR("failed: Null pointer roi_cfg %p bf_stats %p", roi_cfg, bf_stats);
    return FALSE;
  }
  for (i = 0; (i < roi_cfg->num_bf_fw_roi_dim) &&
    (roi_cfg->bf_fw_roi_dim[i].is_valid == TRUE); i++) {

    /* Checking whether both ROI starts on same pixel or starting line apart
     * by min
     */

    roi_i_dim = &roi_cfg->bf_fw_roi_dim[i];
    for (j = i+1; (j < roi_cfg->num_bf_fw_roi_dim) &&
      (roi_cfg->bf_fw_roi_dim[j].is_valid == TRUE); j++) {

      roi_j_dim = &roi_cfg->bf_fw_roi_dim[j];

      if (IS_XY_EQUAL(roi_i_dim, roi_j_dim) ||
        (bf_stats_are_regions_overlapping(roi_i_dim, roi_j_dim) &&
         abs((int)(roi_i_dim->x - roi_j_dim->x)) < MIN_START_PIX_OVERLAP)) {
        if ((roi_j_dim->w - MIN_START_PIX_OVERLAP) >= MIN_WIDTH) {
          /* Move ROI to right with reduced width */
          roi_j_dim->x += MIN_START_PIX_OVERLAP;
        } else if ((roi_j_dim->x + roi_j_dim->w + MIN_START_PIX_OVERLAP) <=
          bf_stats->sensor_width - H_FIR_MARGIN) {
          /* Move ROI to right */
          roi_j_dim->x += MIN_START_PIX_OVERLAP;
        } else {
          /* Mark ROI as invalid */
          roi_j_dim->is_valid = FALSE;
          ISP_ERR("roi invalid x %d y %d w %d h %d", roi_j_dim->x, roi_j_dim->y,
            roi_j_dim->w, roi_j_dim->h);
          ret = FALSE;
          continue;
        }
      }
      break;
    }
  }
  return ret;
}

/** bf_stats47_bubble_sort_start_pix_order:
 *
 *  @bf_stats: bf stats handle
 *  @roi_cfg: bf stats roi cfg
 *
 *  Bubble sort the roi in starting pix order in raster scan.
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bf_stats47_bubble_sort_start_pix_order(bf_fw_roi_cfg_t *roi_cfg,
  bf_stats47_t *bf_stats)
{
  boolean                 swapped = TRUE;
  uint32_t                i = 0, j = 0,
                          i_start, j_start,
                          i_end, j_end;
  bf_fw_roi_dim_t        *roi_i_dim = NULL,
                         *roi_j_dim = NULL;
  bf_fw_roi_dim_t         roi_swap_dim;

  while (swapped) {
    swapped = FALSE;
    for (i = 0; (i < roi_cfg->num_bf_fw_roi_dim) &&
      (roi_cfg->bf_fw_roi_dim[i].is_valid == TRUE); i++) {

      roi_i_dim = &roi_cfg->bf_fw_roi_dim[i];

      /* Calcuate starting pixel in single dimension based on raster scan
       * order
       */
      i_start = (roi_i_dim->y * bf_stats->sensor_width) + roi_i_dim->x;

      /* Calcuate ending pixel in single dimension based on raster scan
       * order
       */
      i_end = (roi_i_dim->y * bf_stats->sensor_width) + roi_i_dim->x +
        roi_i_dim->w;

      for (j = i+1; (j < roi_cfg->num_bf_fw_roi_dim) &&
        (roi_cfg->bf_fw_roi_dim[j].is_valid == TRUE); j++) {

        roi_j_dim = &roi_cfg->bf_fw_roi_dim[j];

        j_start = (roi_j_dim->y * bf_stats->sensor_width) + roi_j_dim->x;

        j_end = (roi_j_dim->y * bf_stats->sensor_width) + roi_j_dim->x +
          roi_j_dim->w;
        /* Bubble sort */
        if (i_start > j_start) {
          roi_swap_dim = *roi_i_dim;
          *roi_i_dim = *roi_j_dim;
          *roi_j_dim = roi_swap_dim;
          swapped = TRUE;
        }
        break;
      }
    }
  }
  return TRUE;
}

/** bf_stats47_validate_roi_boundary:
 *
 *  @bf_stats: bf stats handle
 *  @roi_cfg: bf stats roi cfg
 *  @scale_ratio: if bf scaling is enabled then need to check
 *              the limitations on the scaled image.
 *
 *  Validate if the number of ROIs in in any line
 *  does not exceed the max limitation of hardware
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bf_stats47_validate_roi_boundary(bf_fw_roi_cfg_t *roi_cfg, bf_stats47_t *bf_stats,
  float scale_ratio)
{
  uint32_t             i = 0;
  bf_fw_roi_dim_t     *roi_dim = NULL;
  if (!roi_cfg || !bf_stats) {
    ISP_ERR("failed: Null pointer roi_cfg %p bf_stats %p", roi_cfg, bf_stats);
    return FALSE;
  }

  if (scale_ratio > 1.0f) {
    ISP_ERR("failed: scale ratio > 1.0f");
    return FALSE;
  }

  /* Validate ROI config */
  for (i = 0; i < roi_cfg->num_bf_fw_roi_dim; i++) {

    roi_dim = &roi_cfg->bf_fw_roi_dim[i];

    /* Mark this ROI as valid */
    roi_dim->is_valid = TRUE;

    /* Validate region */
    if (roi_dim->region >= BF_FW_MAX_REGION) {
      ISP_DBG("error: invalid region %d", roi_dim->region);
      /* Hardcode to secondary region */
      roi_dim->region = BF_FW_SECONDARY_REGION;
    }

    if (!roi_dim->w || !roi_dim->h) {
      ISP_DBG("error: region width %d region height %d", roi_dim->w,
        roi_dim->h);
      roi_dim->is_valid = FALSE;
      continue;
    }

    /* Validate mininum margin on left side */
    if ((roi_dim->x * scale_ratio) < MIN_X_OFFSET) {
      ISP_DBG("error: invalid x %d for y %d w %d h %d Changed x %d",
        roi_dim->x, roi_dim->y, roi_dim->w, roi_dim->h,
        EVEN_CEIL((int)((MIN_X_OFFSET + 1) / scale_ratio - 1)));
      roi_dim->x = (MIN_X_OFFSET + 1) / scale_ratio - 1;
      roi_dim->x = EVEN_CEIL(roi_dim->x);
    }

    /* Validate whether x is even */
    if (IF_ODD(roi_dim->x)) {
      ISP_DBG("error: invalid x %d for y %d w %d h %d Changed x %d",
        roi_dim->x, roi_dim->y, roi_dim->w, roi_dim->h, EVEN_FLOOR(roi_dim->x));
      roi_dim->x = EVEN_FLOOR(roi_dim->x);
    }

    /* Validate min y */
    if (roi_dim->y < MIN_Y_OFFSET) {
      ISP_DBG("error: invalid y %d for x %d w %d h %d Changed y %d",
        roi_dim->y, roi_dim->x, roi_dim->w, roi_dim->h, MIN_Y_OFFSET);
      roi_dim->y = MIN_Y_OFFSET;
    }

    /* Validate whether y is even */
    if (IF_ODD(roi_dim->y)) {
      ISP_DBG("error: invalid y %d for x %d w %d h %d Changed y %d",
        roi_dim->y, roi_dim->x,roi_dim->w, roi_dim->h, EVEN_FLOOR(roi_dim->y));
      roi_dim->y = EVEN_FLOOR(roi_dim->y);
    }

    /* Validate whether w is odd */
    if (IF_EVEN(roi_dim->w)) {
      ISP_DBG("error: invalid w %d for x %d y %d h %d Changed w %d",
        roi_dim->w, roi_dim->x, roi_dim->y, roi_dim->h, ODD_FLOOR(roi_dim->w));
      roi_dim->w = ODD_FLOOR(roi_dim->w);
    }

    /* Validate whether h is odd */
    if (IF_EVEN(roi_dim->h)) {
      ISP_DBG("error: invalid h %d for x %d y %d w %d Changed h %d",
        roi_dim->h, roi_dim->x, roi_dim->y, roi_dim->w, ODD_FLOOR(roi_dim->h));
      roi_dim->h = ODD_FLOOR(roi_dim->h);
    }

    /* Validate min w */
    if ((roi_dim->w * scale_ratio) < MIN_WIDTH) {
      ISP_DBG("error: invalid w %d for x %d y %d h %d, Changed w %d",
        roi_dim->w, roi_dim->x, roi_dim->y, roi_dim->h,
        ODD_CEIL((int)roundf((float)MIN_WIDTH / scale_ratio)));
      roi_dim->w = roundf((float)MIN_WIDTH / scale_ratio);
      roi_dim->w = ODD_CEIL(roi_dim->w);
    }

    /* Validate max w */
    if ((roi_dim->w) >= (bf_stats->sensor_width / 2)) {
      ISP_DBG("error: invalid w %d for x %d y %d h %d, Changed w %d",
        roi_dim->w, roi_dim->x, roi_dim->y, roi_dim->h,
        ODD_FLOOR(bf_stats->sensor_width/2));
       roi_dim->w = bf_stats->sensor_width / 2;
       roi_dim->w = ODD_FLOOR(roi_dim->w);
    }

    /*Validate minimum margin on right side*/
    if (((roi_dim->x + roi_dim->w) * scale_ratio) > (bf_stats->sensor_width *
      scale_ratio - H_FIR_MARGIN)) {
      ISP_DBG("error: invalid x %d for y %d w %d h %d Discard ROI",
        roi_dim->x, roi_dim->y, roi_dim->w, roi_dim->h);
      roi_dim->is_valid = FALSE;
      continue;
    }

    /* Validate min h */
    if (roi_dim->h < MIN_HEIGHT) {
      ISP_DBG("error: invalid h %d for x %d y %d w %d Changed h %d",
        roi_dim->h, roi_dim->x, roi_dim->y, roi_dim->w, MIN_HEIGHT - 1);
      roi_dim->h = MIN_HEIGHT - 1;
    }

    /* Validate minimum margin at bottom */
    if ((roi_dim->y + roi_dim->h) > (bf_stats->sensor_height - V_FIR_MARGIN)) {
      ISP_DBG("error: invalid y %d for x %d w %d h %d Discard ROI",
        roi_dim->y, roi_dim->x, roi_dim->w, roi_dim->h);
      roi_dim->is_valid = FALSE;
      continue;
    }

    /* Validate max h */
    if (roi_dim->h >= (bf_stats->sensor_height / 2)) {
      ISP_DBG("error: invalid h %d for x %d y %d w %d Changed h %d",
        roi_dim->h, roi_dim->x, roi_dim->y, roi_dim->w,
        ODD_FLOOR(bf_stats->sensor_height / 2));
       roi_dim->h = bf_stats->sensor_height / 2;
       roi_dim->h = ODD_FLOOR(roi_dim->h);
    }
  }

  return TRUE;
}

/** bf_stats47_chk_max_horiz_lim_per_row :
 *
 *  @bf_stats: bf stats handle
 *  @roi_cfg: bf stats roi cfg
 *
 *  Validate if the number of ROIs in in any line
 *  does not exceed the max limitation of hardware
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bf_stats47_chk_max_horiz_lim_per_row (bf_stats47_t *bf_stats,
  bf_fw_roi_cfg_t *roi_cfg)
{
  boolean              ret = TRUE;
  uint32_t             i = 0;
  bf_fw_roi_dim_t     *roi_dim = NULL;

  if (!bf_stats || ! roi_cfg) {
    ISP_ERR("Null pointer bf_stats %p roi_cfg %p", bf_stats, roi_cfg);
    return FALSE;
  }
  /* Count # of horizontal regions in each row */
  memset(bf_stats->validate_horiz_limit, 0, sizeof(uint32_t) *
    bf_stats->sensor_height);
  for (i = 0; (i < roi_cfg->num_bf_fw_roi_dim) &&
    (roi_cfg->bf_fw_roi_dim[i].is_valid == TRUE); i++) {

    roi_dim = &roi_cfg->bf_fw_roi_dim[i];

    if (roi_dim->y < bf_stats->sensor_height) {
      bf_stats->validate_horiz_limit[roi_dim->y]++;
    } else {
      ISP_ERR("error: invalid y %d sensor_height %d", roi_dim->y,
        bf_stats->sensor_height);
      roi_dim->is_valid = FALSE;
    }
  }

  /* Validate max horizontal regions in each row */
  for (i = 0; i < bf_stats->sensor_height; i++) {
    if (bf_stats->validate_horiz_limit[i] > MAX_HORIZONTAL_REGIONS) {
      ISP_ERR("failed: # of horizontal regions exceeded limit %d for row %d",
        MAX_HORIZONTAL_REGIONS, i);
      /* Cannot adjust ROI's, return error and do not consume enter ROI
       * config
       */
      return FALSE;
    }
  }
  return TRUE;
}
/** bf_stats47_chk_max_region_overlap:
 *
 *  @bf_stats: bf stats handle
 *  @roi_cfg: bf stats roi cfg
 *
 *  Validate max overlapping region specified by hardware
 *  limitation
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bf_stats47_chk_max_region_overlap(bf_fw_roi_cfg_t *roi_cfg)
{
  boolean                  ret = FALSE;
  uint32_t                 i = 0, j = 0,
                           overlap_count = 0;
  bf_fw_roi_dim_t         *roi_dim = NULL,
                          *roi_i_dim = NULL,
                          *roi_j_dim = NULL;

  if (!roi_cfg) {
    ISP_ERR("Null Pointer roi_cfg %p", roi_cfg);
    return FALSE;
  }
  /* Validate MAX_OVERLAPPING_REGIONS for all ROI's */
  for (i = 0; (i < roi_cfg->num_bf_fw_roi_dim) &&
    (roi_cfg->bf_fw_roi_dim[i].is_valid == TRUE); i++) {

    roi_i_dim = &roi_cfg->bf_fw_roi_dim[i];

    overlap_count = 0;

    for (j = i+1; (j < (i + MAX_HORIZONTAL_REGIONS)) &&
      (j < roi_cfg->num_bf_fw_roi_dim); j++) {

      roi_j_dim = &roi_cfg->bf_fw_roi_dim[j];

      if (roi_j_dim->is_valid == FALSE) {
        continue;
      }

      if (bf_stats_are_regions_overlapping(roi_i_dim, roi_j_dim)) {
        #ifdef OVERLAPPING_REG_SUPPORT
          overlap_count++;
        #else
          ISP_ERR("error: Regions are overlapping");
          return FALSE;
        #endif
      }

      /* Check whether overlap_count is within limits */
      if (overlap_count > MAX_OVERLAPPING_REGIONS) {
        ISP_ERR("failed: overlap_count %d exceeded max %d (%d,%d,%d,%d)",
          overlap_count, MAX_OVERLAPPING_REGIONS, roi_i_dim->x, roi_i_dim->y,
          roi_i_dim->w, roi_i_dim->y);
        /* Cannot adjust ROI's, return error and do not consume enter ROI
         * config
         */
        return FALSE;
      }
    }
  }
  return TRUE;
}
/** bf_stats47_chk_max_vert_spacing_and_disjoint:
 *
 *  @bf_stats: bf stats handle
 *  @roi_cfg: bf stats roi cfg
 *
 *  Validate max vertical spacing between i'th and i + 21st
 *  region and validate disjoint ROI's
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bf_stats47_chk_max_vert_spacing_and_disjoint(bf_stats47_t *bf_stats,
  bf_fw_roi_cfg_t *roi_cfg)
{
  boolean                ret = TRUE;
  uint32_t               i = 0;
  bf_fw_roi_dim_t       *roi_i_dim = NULL,
                        *roi_j_dim = NULL;
  uint32_t               j_start = 0,
                         i_end = 0;

  for (i = 0; i < (roi_cfg->num_bf_fw_roi_dim) &&
    (roi_cfg->bf_fw_roi_dim[i].is_valid == TRUE); i++) {

    if ((i + MAX_HORIZONTAL_REGIONS) >= roi_cfg->num_bf_fw_roi_dim) {
      break;
    }

    if (roi_cfg->bf_fw_roi_dim[i + MAX_HORIZONTAL_REGIONS].is_valid ==
      FALSE) {
      continue;
    }

    roi_i_dim = &roi_cfg->bf_fw_roi_dim[i];
    roi_j_dim = &roi_cfg->bf_fw_roi_dim[i + MAX_HORIZONTAL_REGIONS];

    /* Check whether i and (i + MAX_HORIZONTAL_REGIONS) are spaced apart
     * by MAX_VERTICAL_SPACING lines
     */
    if (((roi_j_dim->y + roi_j_dim->h) - (roi_i_dim->y + roi_i_dim->h)) <
      MAX_VERTICAL_SPACING) {
      ISP_ERR("failed: (j-> y %d h %d) - (i-> y %d h %d) < max spacing %d",
        roi_j_dim->y, roi_j_dim->h, roi_i_dim->y, roi_i_dim->h,
      MAX_VERTICAL_SPACING);
      /* Cannot adjust ROI's, return error and do not consume enter ROI
       * config
       */
      return FALSE;
    }

    /* Check whether i and (i + MAX_HORIZONTAL_REGIONS) are disjoint */
    i_end = ((roi_i_dim->y + roi_i_dim->h) * bf_stats->sensor_width) +
      roi_i_dim->x + roi_i_dim->w;

    j_start = (roi_j_dim->y * bf_stats->sensor_width) + roi_j_dim->x;

    if (i_end >= j_start) {
      ISP_ERR("failed: i (%d,%d,%d,%d) and (i+%d) (%d %d %d %d) not disjoint",
        roi_i_dim->x, roi_i_dim->y, roi_i_dim->w, roi_i_dim->h,
        MAX_HORIZONTAL_REGIONS, roi_j_dim->x, roi_j_dim->y, roi_j_dim->w,
        roi_j_dim->h);
      /* Cannot adjust ROI's, return error and do not consume enter ROI
       * config
       */
      return FALSE;
    }
  }
  return TRUE;
}

/** bf_stats47_validate_roi_cfg:
 *
 *  @bf_stats: bf stats handle
 *  @roi_cfg: bf stats roi cfg
 *  @bf_scale_cfg: bf scale cfg
 *
 *  Validate bf stats roi cfg
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bf_stats47_validate_roi_cfg(bf_stats47_t *bf_stats,
  bf_fw_roi_cfg_t *roi_cfg, bf_scale_cfg_t *bf_scale_cfg)
{
  boolean              ret = TRUE,
                       swapped = TRUE;
  uint32_t             i = 0,
                       j = 0,
                       i_start = 0,
                       j_start = 0,
                       i_end = 0,
                       j_end = 0,
                       overlap_count = 0,
                       i_x = 0,
                       i_y = 0,
                       i_w = 0,
                       i_h = 0,
                       j_x = 0,
                       j_y = 0,
                       j_w = 0,
                       j_h = 0,
                       temp_width,
                       temp_height;
  bf_fw_roi_dim_t     *roi_dim = NULL,
                      *roi_i_dim = NULL,
                      *roi_j_dim = NULL;
  float                scale_ratio = 1.0f;

  if (!bf_stats || !roi_cfg || !bf_scale_cfg) {
    ISP_ERR("failed: %p %p %p", bf_stats, roi_cfg, bf_scale_cfg);
    return FALSE;
  }

  if (roi_cfg->is_valid == FALSE) {
    return TRUE;
  }

  /* Calculate scale ratio based on downscaler params */
  if ((bf_scale_cfg->is_valid == TRUE) && (bf_scale_cfg->bf_scale_en == TRUE)) {
    if (bf_scale_cfg->scale_n) {
      scale_ratio = bf_scale_cfg->scale_m / bf_scale_cfg->scale_n;
    }
  }

  if ((roi_cfg->is_valid == TRUE) && (roi_cfg->num_bf_fw_roi_dim == 0)) {
    ISP_ERR("ROI dim from 3a is 0");
    return FALSE;
  }

  ISP_DBG("scale ratio %f", scale_ratio);
  /* Check for max num roi regions */
  if (roi_cfg->num_bf_fw_roi_dim > MAX_NUM_ROI) {
    ISP_ERR("error: 3a num roi %d > max_num_roi %d",
      roi_cfg->num_bf_fw_roi_dim, MAX_NUM_ROI);
    return FALSE;
  }
  ret = bf_stats47_validate_roi_boundary(roi_cfg, bf_stats, scale_ratio);
  if (ret == FALSE) {
    ISP_ERR("failed: bf_stats47_validate_roi_boundary");
  }
  /* Bubble Sort based on starting pixel */
  ret = bf_stats47_bubble_sort_start_pix_order(roi_cfg, bf_stats);
  if (ret == FALSE) {
    ISP_ERR("failed: bf_stats47_bubble_sort_start_pix_order");
    return FALSE;
  }
  ret = bf_stats47_chk_adj_strt_pix_roi_lim(roi_cfg, bf_stats);
  if (ret == FALSE) {
    ISP_ERR("failed: bf_stats47_chk_adj_strt_pix_roi_lim");
  }
  ISP_DBG("After Bubble sort");
  bf_stats47_debug_roi_config(roi_cfg);

  ret = bf_stats47_chk_max_horiz_lim_per_row (bf_stats, roi_cfg);
  if (ret == FALSE) {
    ISP_ERR("failed: bf_stats47_chk_max_horiz_lim_per_row ");
    return FALSE;
  }

  ret = bf_stats47_chk_max_vert_spacing_and_disjoint(bf_stats, roi_cfg);
  if (ret == FALSE) {
    ISP_ERR("failed: bf_stats47_chk_max_vert_spacing_and_disjoint");
    return FALSE;
  }

  ret = bf_stats47_chk_max_region_overlap(roi_cfg);
  if (ret == FALSE) {
    ISP_ERR("failed: bf_stats47_chk_max_region_overlap");
    return FALSE;
  }

  return TRUE;
}

/** bf_stats47_sort_ending_pixel_order:
 *
 *  @bf_stats: bf stats handle
 *
 *  Bubble sort in ending pixel order
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bf_stats47_sort_ending_pixel_order(bf_stats47_t *bf_stats)
{
  boolean             ret = TRUE;
  uint8_t             i = 0, j = 0, count = 0;
  uint32_t            overlap = 0,
                      left_stripe_stats_bound = 0,
                      camif_width = 0,
                      camif_height = 0;
  uint64_t            i_start, j_start;
  bf_fw_roi_cfg_t     *end_pix_order_roi = NULL;
  bf_fw_roi_dim_t     *roi_i_dim,
                      *roi_j_dim;
  bf_fw_roi_dim_t     roi_swap_dim;
  isp_out_info_t      *isp_out;
  boolean              swapped = TRUE;

  isp_out = &bf_stats->isp_out_info;
  overlap = bf_stats->ispif_out_info.overlap;
  left_stripe_stats_bound = isp_out->right_stripe_offset + overlap;

  if (isp_out->is_split == TRUE) {
    if (isp_out->stripe_id == ISP_STRIPE_LEFT)
      camif_width = left_stripe_stats_bound;
    else
      camif_width = bf_stats->sensor_width - isp_out->right_stripe_offset;
  } else {
    camif_width = bf_stats->sensor_width;
  }
  end_pix_order_roi = &bf_stats->bf_roi_end_pix_order;
  *end_pix_order_roi = bf_stats->bf_hw_roi_cfg;
  /* Bubble Sort based on ending pixel */

  while (swapped) {
    swapped = FALSE;
    for (i = 0; i < end_pix_order_roi->num_bf_fw_roi_dim; i++) {

      if (end_pix_order_roi->bf_fw_roi_dim[i].is_valid == FALSE)
        continue;

      roi_i_dim = &end_pix_order_roi->bf_fw_roi_dim[i];

      /* Calcuate starting pixel in single dimension based on raster scan
       * order
       */
      i_start = ((roi_i_dim->y + roi_i_dim->h) * camif_width) +
        roi_i_dim->x + roi_i_dim->w;
      if (i < 2) {
        ISP_DBG("i_start = %llu", i_start);
      }

      for (j = i+1; (j < end_pix_order_roi->num_bf_fw_roi_dim) &&
        (end_pix_order_roi->bf_fw_roi_dim[j].is_valid == TRUE); j++) {

        roi_j_dim = &end_pix_order_roi->bf_fw_roi_dim[j];

        j_start = ((roi_j_dim->y + roi_j_dim->h) * camif_width) +
          roi_j_dim->x + roi_i_dim->w;

        /* Bubble sort */
        if (i_start > j_start) {
          ISP_DBG("swapped");
          roi_swap_dim = *roi_i_dim;
          *roi_i_dim = *roi_j_dim;
          *roi_j_dim = roi_swap_dim;
          swapped = TRUE;
        }
        break;
      }
    }
  }
  return ret;
}

/** chk_adjacent_end_pix_roi_limitation:
 *
 *  @bf_stats: bf stats handle
 *
 *  Check whether both ROI ends on same pixel or ending line
 *  apart by min
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean chk_adjacent_end_pix_roi_limitation(bf_stats47_t *bf_stats)
{
  uint32_t            i = 0,
                      j = 0,
                      i_start,
                      j_start,
                      i_end = 0,
                      j_end = 0,
                      camif_width,
                      overlap,
                      left_stripe_stats_bound;
  bf_fw_roi_dim_t    *roi_i_dim = NULL,
                     *roi_j_dim = NULL;
  bf_fw_roi_cfg_t    *roi_cfg = NULL;
  isp_out_info_t      *isp_out;

  if (!bf_stats) {
    ISP_ERR("failed: Null pointer %p", bf_stats);
    return FALSE;
  }

  isp_out = &bf_stats->isp_out_info;
  overlap = bf_stats->ispif_out_info.overlap;
  left_stripe_stats_bound = isp_out->right_stripe_offset + overlap;
  roi_cfg = &bf_stats->bf_roi_end_pix_order ;

  if (isp_out->is_split == TRUE) {
    if (isp_out->stripe_id == ISP_STRIPE_LEFT)
      camif_width = left_stripe_stats_bound;
    else
      camif_width = bf_stats->sensor_width - isp_out->right_stripe_offset;
  } else {
    camif_width = bf_stats->sensor_width;
  }

  for (i = 0; (i < roi_cfg->num_bf_fw_roi_dim) &&
    (roi_cfg->bf_fw_roi_dim[i].is_valid == TRUE); i++) {

    roi_i_dim = &roi_cfg->bf_fw_roi_dim[i];
    i_start = (roi_i_dim->y * camif_width) + roi_i_dim->x;
    i_end = (roi_i_dim->y * camif_width) + roi_i_dim->x +
      roi_i_dim->w;
    for (j = i+1; (j < roi_cfg->num_bf_fw_roi_dim) &&
      (roi_cfg->bf_fw_roi_dim[j].is_valid == TRUE); j++) {

      roi_j_dim = &roi_cfg->bf_fw_roi_dim[j];
      j_start = (roi_j_dim->y * camif_width) + roi_j_dim->x;
      j_end = (roi_j_dim->y * camif_width) + roi_j_dim->x +
        roi_j_dim->w;

      /* Checking whether both ROI ends on same pixel or ending line apart
       * by min
       */
      if ((i_end == j_end) ||
        (abs((int)((roi_i_dim->x + roi_i_dim->w) - (roi_j_dim->x + roi_j_dim->w))) <
        MIN_START_PIX_OVERLAP)) {
        ISP_ERR("error: two roi ending on same pixel x %d y %d w %d h %d",
          roi_i_dim->x, roi_i_dim->y, roi_i_dim->w, roi_i_dim->h);
        /* Change roi_j_dim */
        if ((roi_j_dim->x + roi_j_dim->w + MIN_START_PIX_OVERLAP) <=
          camif_width - H_FIR_MARGIN) {
          /* Move ROI to right */
          roi_j_dim->x += MIN_START_PIX_OVERLAP;
        } else {
          /* Mark ROI as invalid */
          roi_j_dim->is_valid = FALSE;
          continue;
        }
      }
      break;
    }
  }
  return TRUE;
}

/** bf_stats47_set_default_roi:
 *
 *  @bf_stats: bodule handle
 *
 *  Configure 16 % x 16 % ROI in 5 x 5 grids as default ROI
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bf_stats47_set_default_roi(bf_stats47_t *bf_stats)
{
  bf_fw_roi_cfg_t        *af_fw_roi_cfg = NULL;
  uint32_t                h_offset = 0,
                          v_offset = 0,
                          roi_w = 0,
                          roi_h = 0,
                          h_num = 0,
                          v_num = 0,
                          i = 0,
                          j = 0;
  bf_fw_roi_dim_t        *af_fw_roi_dim = NULL;
  uint32_t                region_num = 0;

  if (!bf_stats) {
    ISP_ERR("failed: Null pointer bf_stats %p", bf_stats);
    return FALSE;
  }
  af_fw_roi_cfg = &bf_stats->af_config.bf_fw.bf_fw_roi_cfg;
  af_fw_roi_cfg->is_valid = TRUE;
  af_fw_roi_cfg->num_bf_fw_roi_dim = 0;
  /* Configure 16 % x 16 % ROI in 5 x 5 grids */
  h_num = v_num = 5;
  roi_w = FLOOR2((25 * bf_stats->sensor_width) / h_num / 100);
  roi_h = FLOOR2((25 * bf_stats->sensor_height) / v_num / 100);
  h_offset = (bf_stats->sensor_width - (h_num * roi_w)) / 2;
  v_offset = (bf_stats->sensor_height - (v_num * roi_h)) / 2;

  for (i = 0; i < v_num; i ++) {
    for (j = 0; j < h_num; j++) {
      af_fw_roi_dim = &af_fw_roi_cfg->bf_fw_roi_dim[
        af_fw_roi_cfg->num_bf_fw_roi_dim++];
      af_fw_roi_dim->region = BF_FW_PRIMARY_REGION;
      af_fw_roi_dim->x = FLOOR2(h_offset + (j * roi_w));
      af_fw_roi_dim->y = FLOOR2(v_offset + (i * roi_h));
      af_fw_roi_dim->w = roi_w - 1;
      af_fw_roi_dim->h = roi_h - 1;
      af_fw_roi_dim->region_num = region_num++;
      af_fw_roi_dim->is_valid = TRUE;
    }
  }

  return TRUE;
}
/** bf_stats47_trigger_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to isp_private_event_t
 *
 *  Configure the entry and reg_cmd for bf_stats using values passed in pix
 *  settings
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bf_stats47_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                      ret = TRUE,
                               func_ret = TRUE;
  int32_t                      i = 0;
  bf_stats47_t                *bf_stats = NULL;
  isp_private_event_t         *private_event = NULL;
  isp_sub_module_output_t     *sub_module_output = NULL;
  bf_fw_roi_cfg_t             *af_fw_roi_cfg = NULL;
  bf_stats47_ext_func_table_t *ext_func_table = NULL;
  isp_meta_entry_t            *baf_dmi_info = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  sub_module_output = (isp_sub_module_output_t *)private_event->data;
  if (!sub_module_output) {
    ISP_ERR("failed: output %p", sub_module_output);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (!isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  bf_stats = (bf_stats47_t *)isp_sub_module->private_data;
  if (!bf_stats) {
    ISP_ERR("failed: bf_stats %p", bf_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  ext_func_table = (bf_stats47_ext_func_table_t*)bf_stats->ext_func_table;

  if (isp_sub_module->submod_enable == FALSE) {
    ISP_DBG("BF not enabled / trigger update pending false");
    /* Fill af_config */
    if (sub_module_output->stats_params) {
      ret = bf_stats47_fill_stats_parser_params(bf_stats,
        sub_module_output->stats_params);
      if (ret == FALSE) {
        ISP_ERR("failed: bf_stats47_fill_stats_parser_params");
        func_ret = FALSE;
      }
    }

    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return func_ret;
  }

  /* 1. Update Y conversion cfg */
  if (bf_stats->in_cfg_trigger_update_pending == TRUE) {
    ret = bf_stats47_input_config(isp_sub_module, bf_stats, sub_module_output);
    if (ret == FALSE) {
      ISP_ERR("failed: bf_stats47_input_config");
      func_ret = FALSE;
    }
    bf_stats->in_cfg_trigger_update_pending = FALSE;
  }

  if (BF_DOWN_SCALER_SUPPORTED == TRUE) {
    /* 3. Update downscaler cfg */
    ret = bf_down_scaler_trigger_update(isp_sub_module,
      &bf_stats->bf_down_scaler, sub_module_output);
    if (ret == FALSE) {
      ISP_ERR("failed: bf_down_scaler_trigger_update");
      func_ret = FALSE;
    }
  }

  if (bf_stats->filter_trigger_update_pending == TRUE) {
    /* 4. Store filter cfg */
    ret = bf_stats47_store_filter_config(isp_sub_module, bf_stats);
    if (ext_func_table && ext_func_table->bf_stats47_ext_fill_filter_params) {
      ret |= ext_func_table->bf_stats47_ext_fill_filter_params(
        bf_stats);
      if (ret == FALSE) {
        ISP_ERR("failed: bf_stats47_ext_fill_filter_params");
        func_ret = FALSE;
      }
    }
    if (ret == FALSE) {
      ISP_ERR("failed: bf_stats47_store_filter_config");
      func_ret = FALSE;
    } else {
      /* 5. Write filter cfg */
      ret = bf_stats47_write_filter_config(isp_sub_module, bf_stats);
      if (ret == FALSE) {
        ISP_ERR("failed: bf_stats47_store_filter_config");
        func_ret = FALSE;
      }
    }
  }

  if (bf_stats->roi_trigger_update_pending == TRUE) {
    af_fw_roi_cfg = &bf_stats->af_config.bf_fw.bf_fw_roi_cfg;

    if (af_fw_roi_cfg->is_valid == FALSE) {
      ISP_ERR("3a given config is invalided by isp. use default config");
      bf_stats47_set_default_roi(bf_stats);
    }

    bf_stats47_split_cfg(bf_stats, af_fw_roi_cfg);
    ISP_DBG("before end pixel sorting");
    bf_stats47_debug_roi_config(&bf_stats->bf_hw_roi_cfg);
    bf_stats47_sort_ending_pixel_order(bf_stats);
    ISP_DBG("after end pixel sorting");
    bf_stats47_debug_roi_config(&bf_stats->bf_roi_end_pix_order );
    chk_adjacent_end_pix_roi_limitation(bf_stats);

    /* 6. Validate DMI cfg */
    ret = bf_stats47_store_dmi_config(isp_sub_module, bf_stats);
    if (ret == FALSE) {
      ISP_ERR("failed: bf_stats47_store_filter_config");
      func_ret = FALSE;
    }

    if (ext_func_table && ext_func_table->bf_stats47_ext_fill_roi_params) {
      ret = ext_func_table->bf_stats47_ext_fill_roi_params(
        bf_stats);
      if (ret == FALSE) {
        ISP_ERR("failed: bf_stats47_ext_fill_roi_params");
        func_ret = FALSE;
      }
    }

    /* Switch DMI */
    bf_stats->pcmd.bf_stats_enable_mask.fields.roi_ind_lut_bank_sel = 1;
    bf_stats->pcmd.bf_stats_enable_val.fields.roi_ind_lut_bank_sel ^= 1;
    if (BF_CGC_OVERRIDE == TRUE) {
      ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
        BF_CGC_OVERRIDE_REGISTER, BF_CGC_OVERRIDE_BIT, TRUE);
      if (ret == FALSE) {
        ISP_ERR("failed: enable cgc");
      }
    }

    /* 7. Write DMI cfg */
    ret = bf_stats47_write_dmi_config(isp_sub_module, bf_stats,
      sub_module_output);
    if (ret == FALSE) {
      ISP_ERR("failed: bf_stats47_write_dmi_config");
      func_ret = FALSE;
    }
    if (BF_CGC_OVERRIDE == TRUE) {
      ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
        BF_CGC_OVERRIDE_REGISTER, BF_CGC_OVERRIDE_BIT, FALSE);
      if (ret == FALSE) {
        ISP_ERR("failed: disable cgc");
      }
    }
  }

  if ((bf_stats->filter_trigger_update_pending == TRUE) ||
    (bf_stats->roi_trigger_update_pending == TRUE)) {
    if (ext_func_table && ext_func_table->bf_stats47_ext_write_ext_params) {
      ret = ext_func_table->bf_stats47_ext_write_ext_params(
        isp_sub_module, bf_stats);
      if (ret == FALSE) {
        ISP_ERR("failed: bf_stats47_ext_write_ext_params");
        func_ret = FALSE;
      }
    }
    bf_stats->filter_trigger_update_pending = FALSE;
    bf_stats->roi_trigger_update_pending = FALSE;
  }

  if (bf_stats->gamma_trigger_update_pending == TRUE) {
    if (BF_GAMMA_SUPPORTED == TRUE) {
      /* 2. Update gamma cfg */
      ret = bf_gamma_trigger_update(isp_sub_module, &bf_stats->bf_gamma_cfg,
        sub_module_output, bf_stats->gamma_downscale_factor);
      if (ret == FALSE) {
        ISP_ERR("failed: bf_gamma_trigger_update");
        func_ret = FALSE;
      }
      bf_stats->gamma_trigger_update_pending = FALSE;
    }
  }

  if (bf_stats->pcmd.bf_stats_enable_mask.value) {
    ret = bf_stats47_write_baf_stats_cfg_mask(isp_sub_module, bf_stats,
      sub_module_output);
    if (ret == FALSE) {
      ISP_ERR("failed: bf_stats47_write_baf_stats_cfg_mask");
      func_ret = FALSE;
    }
    /* Reset Enable Mask */
    bf_stats->pcmd.bf_stats_enable_mask.value = 0;
  }

  /* Set config_pending to FALSE */
  isp_sub_module->config_pending = FALSE;

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module,
    sub_module_output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  /* Fill af_config */
  if (sub_module_output->stats_params) {
    ret = bf_stats47_fill_stats_parser_params(bf_stats,
      sub_module_output->stats_params);
    if (ret == FALSE) {
      ISP_ERR("failed: bf_stats47_fill_stats_parser_params");
      func_ret = FALSE;
    }
  }

  if (sub_module_output->meta_dump_params &&
    sub_module_output->metadata_dump_enable == 1) {
    /*fill in DMI info*/
    baf_dmi_info = &sub_module_output->
      meta_dump_params->meta_entry[ISP_META_STATS_BF_RGN_IND_LUT];
    /*dmi table length*/
    baf_dmi_info->len =
      sizeof(uint64_t) * bf_stats->num_roi;
    /*dmi type */
    baf_dmi_info->dump_type = ISP_META_STATS_BF_RGN_IND_LUT;
    baf_dmi_info->start_addr = 0;
    sub_module_output->meta_dump_params->frame_meta.num_entry++;

    memcpy(baf_dmi_info->isp_meta_dump,
      &bf_stats->packed_roi_dmi_cfg, baf_dmi_info->len);

  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return func_ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  ISP_ERR("failed");
  return FALSE;
} /* bf_stats47_trigger_update */



/** bf_stats47_validate_input_cfg:
 *
 *  @bf_stats: handle to bf_stats
 *  @bf_input_cfg: handle to bf_input_cfg
 *
 *  Validate input config
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bf_stats47_validate_input_cfg(bf_stats47_t *bf_stats,
  bf_input_cfg_t *bf_input_cfg)
{
  boolean  ret = TRUE;
  uint32_t i = 0;
  float    a_sum = 0.0f;

  if (!bf_stats || !bf_input_cfg) {
    ISP_ERR("failed: %p %p", bf_stats, bf_input_cfg);
    return FALSE;
  }

  if (bf_input_cfg->bf_channel_sel >= BF_CHANNEL_SELECT_MAX) {
    ISP_ERR("error: invalid input sel %d max %d", bf_input_cfg->bf_channel_sel,
      BF_CHANNEL_SELECT_MAX);
    return FALSE;
  }

  if (bf_input_cfg->bf_input_g_sel >= BF_INPUT_SELECT_MAX) {
    ISP_ERR("error: invalid input sel %d max %d", bf_input_cfg->bf_input_g_sel,
      BF_INPUT_SELECT_MAX);
    return FALSE;
  }

  if (bf_input_cfg->bf_channel_sel == BF_CHANNEL_SELECT_Y) {
    /* Validate y coefficients */
    for (i = 0; i < MAX_Y_CONFIG; i++) {
      if ((bf_input_cfg->y_a_cfg[i] >= 0.0f) &&
        (bf_input_cfg->y_a_cfg[i] <= 1.0f)) {
        a_sum += bf_input_cfg->y_a_cfg[i];
      } else {
        ISP_INFO("warning: invalid y a cfg %f i %d", bf_input_cfg->y_a_cfg[i], i);
        return FALSE;
      }
    }
    if (a_sum > 1.0f) {
      ISP_ERR("error: invalid a_sum %f i %d", a_sum, i);
      return FALSE;
    }
  }

  return ret;
}

/** bf_stats47_aec_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  Handle stats config update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bf_stats47_aec_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                 ret = TRUE;
  stats_update_t         *stats_update = NULL;
  bf_stats47_t           *bf_stats = NULL;
  bf_gamma_cfg_t         *bf_gamma_cfg = NULL;
  bf_fw_config_t         *bf_fw_local = NULL;
  aec_update_t           *aec_update = NULL;
  float                   scale_ratio = 1.0f;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  if (BF_GAMMA_SUPPORTED == TRUE) {
    PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
    bf_stats = (bf_stats47_t *)isp_sub_module->private_data;
    if (!bf_stats) {
      ISP_ERR("failed: bf_stats %p", bf_stats);
      PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
      return FALSE;
    }
    stats_update = (stats_update_t *)data;
    bf_gamma_cfg = &bf_stats->bf_gamma_cfg;
    bf_fw_local = &bf_stats->af_config.bf_fw;
    if (bf_gamma_cfg->bf_gamma_lut_cfg.is_valid) {
      aec_update = &stats_update->aec_update;
      if (bf_stats->ref_exp_index == 0) {
        scale_ratio = 1.0f;
      } else {
        scale_ratio = pow(1.03f, bf_stats->ref_exp_index - aec_update->exp_index);
      }
      ISP_DBG("curr scale_ratio: %f, new scale_ratio: %f",
        bf_stats->scale_ratio, scale_ratio);
      if (!F_EQUAL(scale_ratio, bf_stats->scale_ratio)) {
        bf_gamma_scale_config(bf_gamma_cfg, bf_fw_local, scale_ratio);
        bf_stats->scale_ratio = scale_ratio;
        bf_stats->gamma_trigger_update_pending = TRUE;
      }
      bf_stats->aec_update = *aec_update;
    }
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  }
  return ret;
}

/** bf_stats47_af_exp_compensate:
 *
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  Handle af exposure compensate event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bf_stats47_af_exp_compensate(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                 ret = TRUE;
  bf_stats47_t           *bf_stats = NULL;
  boolean                 af_exp_comp = FALSE;
  aec_update_t           *aec_update;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  if (BF_GAMMA_SUPPORTED == TRUE) {
    PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
    bf_stats = (bf_stats47_t *)isp_sub_module->private_data;
    if (!bf_stats) {
      ISP_ERR("failed: bf_stats %p", bf_stats);
      PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
      return FALSE;
    }
    aec_update = &bf_stats->aec_update;
    af_exp_comp = *(boolean *)data;
    if (af_exp_comp) {
      bf_stats->ref_exp_index = aec_update->exp_index;
      ISP_DBG("Enable af exp compensate, set ref exp_index: %d",
        bf_stats->ref_exp_index);
    } else {
      bf_stats->ref_exp_index = 0;
      ISP_DBG("Disable af exp compensate, set ref exp_index: %d",
        bf_stats->ref_exp_index);
    }
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  }
  return ret;
}

/** bf_stats47_stats_config_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  Handle stats config update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bf_stats47_stats_config_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean             ret = TRUE,
                      func_ret = TRUE;
  bf_stats47_t       *bf_stats = NULL;
  bf_fw_config_t     *bf_fw_3a = NULL;
  bf_fw_config_t     *bf_fw_local = NULL;
  uint32_t            i = 0;
  af_config_t        *af_config = NULL;
  bf_scale_cfg_t      bf_scale_cfg;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  af_config = (af_config_t *)data;
  if (!(af_config->mask & MCT_EVENT_STATS_BF_FW)) {
    return TRUE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bf_stats = (bf_stats47_t *)isp_sub_module->private_data;
  if (!bf_stats) {
    ISP_ERR("failed: bf_stats %p", bf_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  /* Take local copy of config_id which will passed in parsed buffer
   * to 3A
   */
  bf_stats->af_config.config_id = af_config->config_id;

  if (!bf_stats->sensor_width || !bf_stats->sensor_height ||
    !bf_stats->validate_horiz_limit) {
    ISP_ERR("error: invalid sensor dim w %d h %d validate_horiz_limit %p",
      bf_stats->sensor_width, bf_stats->sensor_height,
      bf_stats->validate_horiz_limit);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  bf_fw_3a = &af_config->bf_fw;
  bf_fw_local = &bf_stats->af_config.bf_fw;

  /* Validate Input cfg */
  if (bf_fw_3a->bf_input_cfg.is_valid == TRUE) {
    ret = bf_stats47_validate_input_cfg(bf_stats, &bf_fw_3a->bf_input_cfg);
    if (ret == TRUE) {
      memcpy(&bf_fw_local->bf_input_cfg, &bf_fw_3a->bf_input_cfg,
        sizeof(bf_fw_local->bf_input_cfg));
      bf_stats->in_cfg_trigger_update_pending = TRUE;
    } else {
      ISP_INFO("warning: bf_stats47_validate_roi_cfg");
      func_ret = FALSE;
    }
  }

  /* Initialize bf_scale_cfg and disable downscaling by default */
  bf_scale_cfg.is_valid = TRUE;
  bf_scale_cfg.bf_scale_en = FALSE;

  if (BF_DOWN_SCALER_SUPPORTED == TRUE) {
    /* Downscaler config */
    ret = bf_down_scaler_stats_config_update(isp_sub_module,
      &bf_stats->bf_down_scaler, bf_fw_3a);
    if (ret == FALSE) {
      ISP_ERR("failed: bf_down_scaler_stats_config_update");
      func_ret = FALSE;
    }

    ret = bf_down_scaler_get_downscaler_params(isp_sub_module,
      &bf_stats->bf_down_scaler, &bf_scale_cfg);
    if (ret == FALSE) {
      bf_scale_cfg.is_valid = TRUE;
      bf_scale_cfg.bf_scale_en = FALSE;
      ISP_ERR("failed: bf_down_scaler_get_downscaler_params");
    }
  }

  /* Validate DMI */
  if (bf_fw_3a->bf_fw_roi_cfg.is_valid == TRUE) {
     bf_stats47_debug_roi_config(&bf_fw_3a->bf_fw_roi_cfg);
    ret = bf_stats47_validate_roi_cfg(bf_stats,
      &bf_fw_3a->bf_fw_roi_cfg, &bf_scale_cfg);
    if (ret == TRUE) {
      memcpy(&bf_stats->af_config.bf_fw.bf_fw_roi_cfg,
        &af_config->bf_fw.bf_fw_roi_cfg,
        sizeof(bf_stats->af_config.bf_fw.bf_fw_roi_cfg));
      bf_stats->roi_trigger_update_pending = TRUE;
    } else {
      ISP_ERR("failed: bf_stats47_validate_roi_cfg");
      bf_stats->af_config.bf_fw.bf_fw_roi_cfg.is_valid = FALSE;
      func_ret = FALSE;
    }
  }

  if (BF_GAMMA_SUPPORTED == TRUE) {
    /* Gamma config */
    ret = bf_gamma_stats_config_update(isp_sub_module, &bf_stats->bf_gamma_cfg,
      bf_fw_3a);
    if (ret == FALSE) {
      ISP_ERR("failed: bf_gamma_stats_config_update");
      func_ret = FALSE;
    }
    if (bf_fw_3a->bf_gamma_lut_cfg.is_valid) {
      memcpy(&bf_fw_local->bf_gamma_lut_cfg, &bf_fw_3a->bf_gamma_lut_cfg,
        sizeof(bf_fw_local->bf_gamma_lut_cfg));
    }
    bf_stats->gamma_trigger_update_pending = TRUE;
  }

  /* filter config */
  for (i = 0; i < BF_FILTER_TYPE_MAX; i++) {
    if (bf_fw_3a->bf_filter_cfg[i].is_valid) {
      switch (i) {
      case BF_FILTER_TYPE_H_1:
        memcpy(&bf_fw_local->bf_filter_cfg[i],
          &bf_fw_3a->bf_filter_cfg[i],
          sizeof(bf_fw_local->bf_filter_cfg[i]));
        break;
      case BF_FILTER_TYPE_H_2:
        memcpy(&bf_fw_local->bf_filter_cfg[i],
          &bf_fw_3a->bf_filter_cfg[i],
          sizeof(bf_fw_local->bf_filter_cfg[i]));
        break;
      case BF_FILTER_TYPE_V:
        memcpy(&bf_fw_local->bf_filter_cfg[i],
          &bf_fw_3a->bf_filter_cfg[i],
          sizeof(bf_fw_local->bf_filter_cfg[i]));
        break;
      default:
        ISP_ERR("failed: invalid i %d", i);
        break;
      }
    }
  }
  bf_stats->filter_trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
}

/** set_default_filter_cfg:
 *
 *  @bf_stats: handle to bf_stats
 *  @af_fw: af floating window configuration from 3a
 *
 *  Set default filter configuration
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean set_default_filter_cfg(bf_fw_config_t *af_fw,
  bf_stats47_t *bf_stats)
{
  bf_filter_cfg_t        *af_filter_cfg = NULL;
  bf_fir_filter_cfg_t    *af_fir_filter_cfg = NULL;
  bf_iir_filter_cfg_t    *af_iir_filter_cfg = NULL;
  bf_filter_coring_cfg_t *af_stats_coring_cfg = NULL;

  if (!af_fw || !bf_stats) {
    ISP_ERR("failed: NULL pointer af_fw %p bf_stats %p", af_fw, bf_stats);
    return FALSE;
  }
  /* H1 cfg */
  af_filter_cfg = &af_fw->bf_filter_cfg[BF_FILTER_TYPE_H_1];
  af_filter_cfg->is_valid = TRUE;
  af_filter_cfg->h_scale_en = FALSE;
  af_filter_cfg->shift_bits = -3;

  /* H1 FIR cfg */
  af_fir_filter_cfg =
    &af_fw->bf_filter_cfg[BF_FILTER_TYPE_H_1].bf_fir_filter_cfg;
  af_fir_filter_cfg->enable = TRUE;
  af_fir_filter_cfg->a[0] = -1;
  af_fir_filter_cfg->a[1] = -2;
  af_fir_filter_cfg->a[2] = -1;
  af_fir_filter_cfg->a[3] = 1;
  af_fir_filter_cfg->a[4] = 5;
  af_fir_filter_cfg->a[5] = 8;
  af_fir_filter_cfg->a[6] = 10;
  af_fir_filter_cfg->a[7] = 8;
  af_fir_filter_cfg->a[8] = 5;
  af_fir_filter_cfg->a[9] = 1;
  af_fir_filter_cfg->a[10] = -1;
  af_fir_filter_cfg->a[11] = -2;
  af_fir_filter_cfg->a[12] = -1;
  af_fir_filter_cfg->num_a = 13;

  /* H1 IIR cfg */
  af_iir_filter_cfg =
    &af_fw->bf_filter_cfg[BF_FILTER_TYPE_H_1].bf_iir_filter_cfg;
  af_iir_filter_cfg->enable = TRUE;
  af_iir_filter_cfg->b10 = 0.092346;
  af_iir_filter_cfg->b11 = 0.000000;
  af_iir_filter_cfg->b12 = -0.092346;
  af_iir_filter_cfg->a11 = 1.712158;
  af_iir_filter_cfg->a12 = -0.815308;
  af_iir_filter_cfg->b20 = 0.112976;
  af_iir_filter_cfg->b21 = 0.000000;
  af_iir_filter_cfg->b22 = -0.112976;
  af_iir_filter_cfg->a21 = 1.869690;
  af_iir_filter_cfg->a22 = -0.898743;

  /* H2 cfg */
  af_filter_cfg = &af_fw->bf_filter_cfg[BF_FILTER_TYPE_H_2];
  af_filter_cfg->is_valid = TRUE;
  af_filter_cfg->h_scale_en = TRUE;
  af_filter_cfg->shift_bits = 3;

  /* H2 FIR cfg */
  af_fir_filter_cfg =
    &af_fw->bf_filter_cfg[BF_FILTER_TYPE_H_2].bf_fir_filter_cfg;
  af_fir_filter_cfg->enable = FALSE;

  /* H2 IIR cfg */
  af_iir_filter_cfg =
    &af_fw->bf_filter_cfg[BF_FILTER_TYPE_H_2].bf_iir_filter_cfg;
  af_iir_filter_cfg->enable = TRUE;
  af_iir_filter_cfg->b10 = 0.078064;
  af_iir_filter_cfg->b11 = 0.000000;
  af_iir_filter_cfg->b12 = -0.078064;
  af_iir_filter_cfg->a11 = 1.735413;
  af_iir_filter_cfg->a12 = -0.843811;
  af_iir_filter_cfg->b20 = 0.257202;
  af_iir_filter_cfg->b21 = 0.000000;
  af_iir_filter_cfg->b22 = -0.257202;
  af_iir_filter_cfg->a21 = 1.477051;
  af_iir_filter_cfg->a22 = -0.760071;

  /* V cfg */
  af_filter_cfg = &af_fw->bf_filter_cfg[BF_FILTER_TYPE_V];
  af_filter_cfg->is_valid = TRUE;
  af_filter_cfg->shift_bits = 0;

  /* V FIR cfg */
  af_fir_filter_cfg =
    &af_fw->bf_filter_cfg[BF_FILTER_TYPE_V].bf_fir_filter_cfg;
  af_fir_filter_cfg->enable = TRUE;
  af_fir_filter_cfg->a[0] = 1;
  af_fir_filter_cfg->a[1] = 1;
  af_fir_filter_cfg->a[2] = 1;
  af_fir_filter_cfg->a[3] = 1;
  af_fir_filter_cfg->a[4] = 1;
  af_fir_filter_cfg->num_a = 5;

  /* V IIR cfg */
  af_iir_filter_cfg =
    &af_fw->bf_filter_cfg[BF_FILTER_TYPE_V].bf_iir_filter_cfg;
  af_iir_filter_cfg->enable = TRUE;
  #ifdef __BF_STATS47_REG_H__
    af_iir_filter_cfg->b10 = 0.894897;
    af_iir_filter_cfg->b11 = -1.789673;
    af_iir_filter_cfg->b12 = 0.894897;
    af_iir_filter_cfg->a11 = 1.778625;
    af_iir_filter_cfg->a12 = -0.800781;
  #endif

  #if defined(__BF_STATS41_REG_H__) || defined(__BF_STATS48_REG_H__)
    af_iir_filter_cfg->b10 = 0.092346;
    af_iir_filter_cfg->b11 = 0.000000;
    af_iir_filter_cfg->b12 = -0.092346;
    af_iir_filter_cfg->a11 = 1.712158;
    af_iir_filter_cfg->a12 = -0.815308;
    af_iir_filter_cfg->b20 = 0.112976;
    af_iir_filter_cfg->b21 = 0.000000;
    af_iir_filter_cfg->b22 = -0.112976;
    af_iir_filter_cfg->a21 = 1.869690;
    af_iir_filter_cfg->a22 = -0.898743;
  #endif

  /* Coring cfg */
  /* H1 coring cfg */
  af_stats_coring_cfg =
    &af_fw->bf_filter_cfg[BF_FILTER_TYPE_H_1].bf_filter_coring_cfg;
  af_stats_coring_cfg->ind[0] = 0;
  af_stats_coring_cfg->ind[1] = 16;
  af_stats_coring_cfg->ind[2] = 16;
  af_stats_coring_cfg->ind[3] = 16;
  af_stats_coring_cfg->ind[4] = 16;
  af_stats_coring_cfg->ind[5] = 16;
  af_stats_coring_cfg->ind[6] = 16;
  af_stats_coring_cfg->ind[7] = 16;
  af_stats_coring_cfg->ind[8] = 16;
  af_stats_coring_cfg->ind[9] = 16;
  af_stats_coring_cfg->ind[10] = 16;
  af_stats_coring_cfg->ind[11] = 16;
  af_stats_coring_cfg->ind[12] = 16;
  af_stats_coring_cfg->ind[13] = 16;
  af_stats_coring_cfg->ind[14] = 16;
  af_stats_coring_cfg->ind[15] = 16;
  af_stats_coring_cfg->ind[16] = 16;
  af_stats_coring_cfg->threshold = (1 << 16);
  af_stats_coring_cfg->gain = 16;

  /* H2 coring cfg */
  af_stats_coring_cfg =
    &af_fw->bf_filter_cfg[BF_FILTER_TYPE_H_2].bf_filter_coring_cfg;
  af_stats_coring_cfg->ind[0] = 0;
  af_stats_coring_cfg->ind[1] = 16;
  af_stats_coring_cfg->ind[2] = 16;
  af_stats_coring_cfg->ind[3] = 16;
  af_stats_coring_cfg->ind[4] = 16;
  af_stats_coring_cfg->ind[5] = 16;
  af_stats_coring_cfg->ind[6] = 16;
  af_stats_coring_cfg->ind[7] = 16;
  af_stats_coring_cfg->ind[8] = 16;
  af_stats_coring_cfg->ind[9] = 16;
  af_stats_coring_cfg->ind[10] = 16;
  af_stats_coring_cfg->ind[11] = 16;
  af_stats_coring_cfg->ind[12] = 16;
  af_stats_coring_cfg->ind[13] = 16;
  af_stats_coring_cfg->ind[14] = 16;
  af_stats_coring_cfg->ind[15] = 16;
  af_stats_coring_cfg->ind[16] = 16;
  af_stats_coring_cfg->threshold = (1 << 16);
  af_stats_coring_cfg->gain = 16;

  /* V coring cfg */
  af_stats_coring_cfg =
    &af_fw->bf_filter_cfg[BF_FILTER_TYPE_V].bf_filter_coring_cfg;
  af_stats_coring_cfg->ind[0] = 0;
  af_stats_coring_cfg->ind[1] = 16;
  af_stats_coring_cfg->ind[2] = 16;
  af_stats_coring_cfg->ind[3] = 16;
  af_stats_coring_cfg->ind[4] = 16;
  af_stats_coring_cfg->ind[5] = 16;
  af_stats_coring_cfg->ind[6] = 16;
  af_stats_coring_cfg->ind[7] = 16;
  af_stats_coring_cfg->ind[8] = 16;
  af_stats_coring_cfg->ind[9] = 16;
  af_stats_coring_cfg->ind[10] = 16;
  af_stats_coring_cfg->ind[11] = 16;
  af_stats_coring_cfg->ind[12] = 16;
  af_stats_coring_cfg->ind[13] = 16;
  af_stats_coring_cfg->ind[14] = 16;
  af_stats_coring_cfg->ind[15] = 16;
  af_stats_coring_cfg->ind[16] = 16;
  af_stats_coring_cfg->threshold = (1 << 16);
  af_stats_coring_cfg->gain = 16;

   /* set trigger update pending to true*/
  bf_stats->filter_trigger_update_pending = TRUE;

  return TRUE;
}
/** bf_stats47_set_stream_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy sensor dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bf_stats47_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                 ret = TRUE,
                          func_ret = TRUE;
  bf_stats47_t           *bf_stats = NULL;
  sensor_out_info_t      *sensor_out_info = NULL;
  bf_fw_config_t         *af_fw = NULL;

  /* Validate bf_fw_roi_cfg->num_bf_fw_roi_dim */
  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  sensor_out_info = (sensor_out_info_t *)data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bf_stats = (bf_stats47_t *)isp_sub_module->private_data;
  if (!bf_stats) {
    ISP_ERR("failed: bf_stats %p", bf_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  bf_stats->sensor_out_info = *sensor_out_info;
  bf_stats->sensor_width = (sensor_out_info->request_crop.last_pixel -
    sensor_out_info->request_crop.first_pixel + 1);
  bf_stats->sensor_height = (sensor_out_info->request_crop.last_line -
    sensor_out_info->request_crop.first_line + 1);

  af_fw = &bf_stats->af_config.bf_fw;
  af_fw->bf_input_cfg.y_a_cfg[0] = 0;
  af_fw->bf_input_cfg.y_a_cfg[1] = 0;
  af_fw->bf_input_cfg.y_a_cfg[2] = 0;
  bf_stats->in_cfg_trigger_update_pending = TRUE;

  /* Initialize default values */
  /* 1. Select Gr as input cfg */
  af_fw->bf_input_cfg.bf_input_g_sel = BF_INPUT_SELECT_GR;
  af_fw->bf_input_cfg.bf_channel_sel = BF_CHANNEL_SELECT_G;

  /* 2. Gamma cfg */
  if (BF_GAMMA_SUPPORTED == TRUE) {
    ret = bf_gamma_set_stream_config(isp_sub_module, &bf_stats->bf_gamma_cfg,
      sensor_out_info);
    if (ret == FALSE) {
      ISP_ERR("failed: bf_gamma_trigger_update");
      func_ret = FALSE;
    }
  }

  /* 3. Scaler cfg */
  if (BF_DOWN_SCALER_SUPPORTED == TRUE) {
    ret = bf_down_scaler_set_stream_config(isp_sub_module,
      &bf_stats->bf_down_scaler, sensor_out_info);
    if (ret == FALSE) {
      ISP_ERR("failed: bf_down_scaler_set_stream_config");
      func_ret = FALSE;
    }
  }

  /* 4. Filter cfg */
  set_default_filter_cfg(af_fw, bf_stats);
  ISP_DBG("sensor width %d sensor height %d", bf_stats->sensor_width,
    bf_stats->sensor_height);
  /* 5. Default ROI */
  bf_stats47_set_default_roi(bf_stats);

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return func_ret;
} /* bf_stats47_set_stream_config */

/** bf_stats47_get_stats_capabilities:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  @data: handle to mct_stats_info_t
 *
 *  Get stats caps
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean bf_stats47_get_stats_capabilities(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                     ret = TRUE;
  mct_stats_info_t           *stats_info = NULL;
  mct_stats_bf_roi_caps_t    *bf_roi_caps = NULL;
  sensor_out_info_t          *sensor_out_info = NULL;
  bf_stats47_t               *bf_stats = NULL;
  uint32_t                    sensor_width = 0, sensor_height = 0;
  isp_private_event_t        *private_event = NULL;
  mct_stats_bf_fw_caps_t     *bf_fw_caps = NULL;
  mct_stats_filter_caps_t    *filter_caps = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: isp_sub_module %p data %p", isp_sub_module, data);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bf_stats = (bf_stats47_t *)isp_sub_module->private_data;
  if (!bf_stats) {
    ISP_ERR("failed: bf_stats %p", bf_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  sensor_out_info = &bf_stats->sensor_out_info;
  sensor_width = (sensor_out_info->request_crop.last_pixel -
    sensor_out_info->request_crop.first_pixel + 1);
  sensor_height = (sensor_out_info->request_crop.last_line -
    sensor_out_info->request_crop.first_line + 1);

  private_event = (isp_private_event_t *)data;
  stats_info = (mct_stats_info_t *)private_event->data;
  if (!stats_info) {
    ISP_ERR("failed: stats_info %p", stats_info);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  stats_info->bf_caps.bf_mask |= MCT_EVENT_STATS_BF_FW;

  bf_roi_caps = &stats_info->bf_caps.bf_roi_caps;
  bf_roi_caps->is_valid = TRUE;
  bf_roi_caps->min_grid_width = MIN_WIDTH;
  if (MAX_GRID_WIDTH_DIVIDER) {
    bf_roi_caps->max_grid_width = sensor_width / MAX_GRID_WIDTH_DIVIDER;
  } else {
    ISP_ERR("failed: MAX_GRID_WIDTH_DIVIDER %d", MAX_GRID_WIDTH_DIVIDER);
  }
  bf_roi_caps->min_grid_height = MIN_HEIGHT;
  if (MAX_GRID_HEIGHT_DIVIDER) {
    bf_roi_caps->max_grid_height = sensor_height / MAX_GRID_HEIGHT_DIVIDER;
  } else {
    ISP_ERR("failed: MAX_GRID_HEIGHT_DIVIDER %d", MAX_GRID_HEIGHT_DIVIDER);
  }
  bf_roi_caps->min_hor_offset = MIN_X_OFFSET;
  bf_roi_caps->max_hor_offset = sensor_width - MIN_WIDTH - MIN_X_OFFSET;
  bf_roi_caps->min_ver_offset = MIN_Y_OFFSET;
  bf_roi_caps->max_ver_offset = sensor_height - MIN_HEIGHT - MIN_Y_OFFSET;
  bf_roi_caps->min_hor_grids = MIN_H_NUM;
  bf_roi_caps->max_hor_grids = MAX_H_NUM;
  bf_roi_caps->min_ver_grids = MIN_V_NUM;
  bf_roi_caps->max_ver_grids = MAX_V_NUM;
  bf_roi_caps->max_total_grids = MAX_NUM_ROI;

  ISP_DBG("%x %d %d %d %d %d %d %d %d %d %d %d %d", stats_info->bf_caps.bf_mask,
    bf_roi_caps->min_grid_width, bf_roi_caps->min_grid_height,
    bf_roi_caps->max_grid_width, bf_roi_caps->max_grid_height,
    bf_roi_caps->min_hor_offset, bf_roi_caps->max_hor_offset,
    bf_roi_caps->min_ver_offset, bf_roi_caps->max_ver_offset,
    bf_roi_caps->min_hor_grids, bf_roi_caps->max_hor_grids,
    bf_roi_caps->min_ver_grids, bf_roi_caps->max_ver_grids);

  /* Get gamma capabilities */
  if (BF_GAMMA_SUPPORTED == TRUE) {
    ret = bf_gamma_get_stats_capabilities(isp_sub_module, stats_info);
    if (ret == FALSE) {
      ISP_ERR("failed: bf_gamma_get_stats_capabilities");
    }
  }

  bf_fw_caps = &stats_info->bf_caps.bf_fw_caps;
  bf_fw_caps->is_valid = TRUE;
  bf_fw_caps->is_ch_sel_supported = BF_CH_SEL_SUPPORTED;
  bf_fw_caps->min_independent_windows = MAX_NUM_ROI;
  bf_fw_caps->max_independent_windows = MAX_NUM_ROI;
  bf_fw_caps->min_hor_start_pixel_spacing = MIN_START_PIX_OVERLAP;
  bf_fw_caps->min_hor_end_pixel_spacing = MIN_END_PIX_OVERLAP;
  bf_fw_caps->max_overlapping_windows = MAX_OVERLAPPING_REGIONS;
  bf_fw_caps->max_hor_grids_each_line = MAX_HORIZONTAL_REGIONS;

  /* Fill filter capabilities */
  bf_fw_caps->num_filter_caps = 0;

  filter_caps = &bf_fw_caps->filter_caps[0];
  filter_caps->filter_type = BF_FILTER_TYPE_H_1;
  filter_caps->num_fir_a_kernel = NUM_H1_FIR_A_KERNEL;
  filter_caps->num_iir_a_kernel = NUM_H1_IIR_A_KERNEL;
  filter_caps->num_iir_b_kernel = NUM_H1_IIR_B_KERNEL;
  filter_caps->fir_disable_supported = H1_FIR_DISABLE_SUPPORTED;
  filter_caps->iir_disable_supported = H1_IIR_DISABLE_SUPPORTED;
  filter_caps->downscale_supported = H1_DOWNSCALE_SUPPORTED;
  filter_caps->is_coring_variable = H1_IS_CORING_VARIABLE;
  bf_fw_caps->num_filter_caps++;

  filter_caps = &bf_fw_caps->filter_caps[1];
  filter_caps->filter_type = BF_FILTER_TYPE_H_2;
  filter_caps->num_fir_a_kernel = NUM_H2_FIR_A_KERNEL;
  filter_caps->num_iir_a_kernel = NUM_H2_IIR_A_KERNEL;
  filter_caps->num_iir_b_kernel = NUM_H2_IIR_B_KERNEL;
  filter_caps->fir_disable_supported = H2_FIR_DISABLE_SUPPORTED;
  filter_caps->iir_disable_supported = H2_IIR_DISABLE_SUPPORTED;
  filter_caps->downscale_supported = H2_DOWNSCALE_SUPPORTED;
  filter_caps->is_coring_variable = H2_IS_CORING_VARIABLE;
  bf_fw_caps->num_filter_caps++;

  filter_caps = &bf_fw_caps->filter_caps[2];
  filter_caps->filter_type = BF_FILTER_TYPE_V;
  filter_caps->num_fir_a_kernel = NUM_V_FIR_A_KERNEL;
  filter_caps->num_iir_a_kernel = NUM_V_IIR_A_KERNEL;
  filter_caps->num_iir_b_kernel = NUM_V_IIR_B_KERNEL;
  filter_caps->fir_disable_supported = V_FIR_DISABLE_SUPPORTED;
  filter_caps->iir_disable_supported = V_IIR_DISABLE_SUPPORTED;
  filter_caps->downscale_supported = V_DOWNSCALE_SUPPORTED;
  filter_caps->is_coring_variable = V_IS_CORING_VARIABLE;
  bf_fw_caps->num_filter_caps++;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** bf_stats47_streamon:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: stream on payload
 *
 *  This function adds ref count for stream on flag and sets
 *  trigger_update_pending flag to TRUE for first STREAM ON
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bf_stats47_streamon(isp_sub_module_t *isp_sub_module, void *data)
{
  boolean       ret = TRUE;
  bf_stats47_t *bf_stats = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (isp_sub_module->stream_on_count++) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  bf_stats = (bf_stats47_t *)isp_sub_module->private_data;
  if (!bf_stats) {
    ISP_ERR("failed: bf_stats %p", bf_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  bf_stats->validate_horiz_limit = (uint32_t *)CAM_CALLOC(sizeof(uint32_t) *
    bf_stats->sensor_height, 1);
  if (!bf_stats->validate_horiz_limit) {
    ISP_ERR("failed: no mem");
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  bf_stats->in_cfg_trigger_update_pending = TRUE;
  bf_stats->roi_trigger_update_pending    = TRUE;
  bf_stats->filter_trigger_update_pending = TRUE;
  bf_stats->gamma_trigger_update_pending  = TRUE;
  isp_sub_module->trigger_update_pending  = TRUE;
  isp_sub_module->config_pending          = TRUE;

  bf_stats_ext47_init(bf_stats);

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;

} /* bf_stats47_streamon */

/** bf_stats47_streamoff:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: control event data
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bf_stats47_streamoff(isp_sub_module_t *isp_sub_module, void *data)
{
  bf_stats47_t *bf_stats = NULL;
  bf_stats47_ext_func_table_t *ext_func_table = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bf_stats = (bf_stats47_t *)isp_sub_module->private_data;
  if (!bf_stats) {
    ISP_ERR("failed: bf_stats %p", bf_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  ext_func_table = (bf_stats47_ext_func_table_t*)bf_stats->ext_func_table;
  if (ext_func_table && ext_func_table->bf_stats47_ext_destroy) {
    ext_func_table->bf_stats47_ext_destroy(bf_stats);
  }

  free(bf_stats->validate_horiz_limit);
  memset(bf_stats, 0, sizeof(*bf_stats));

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* bf_stats47_streamoff */

/** bf_stats47_init:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the bf_stats module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean bf_stats47_init(isp_sub_module_t *isp_sub_module)
{
  boolean       ret = TRUE,
                func_ret = TRUE;
  bf_stats47_t *bf_stats = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bf_stats = (bf_stats47_t *)CAM_CALLOC(sizeof(bf_stats47_t), 1);
  if (!bf_stats) {
    ISP_ERR("failed: bf_stats %p", bf_stats);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (func_ret == TRUE) {
    isp_sub_module->private_data = (void *)bf_stats;
  } else {
    free(bf_stats);
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return func_ret;
}/* bf_stats47_init */

/** bf_stats47_destroy:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamically allocated resources
 *
 *  Return none
 **/
void bf_stats47_destroy(isp_sub_module_t *isp_sub_module)
{
  bf_stats47_t *bf_stats = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return;
  }

  bf_stats = (bf_stats47_t *)isp_sub_module->private_data;

  if (!bf_stats) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  free(isp_sub_module->private_data);

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return;
} /* bf_stats47_destroy */
