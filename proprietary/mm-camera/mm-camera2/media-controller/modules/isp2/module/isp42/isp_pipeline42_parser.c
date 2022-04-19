/* isp_pipeline42_parser.c
 *
 * Copyright (c) 2015, 2017-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

/* mctl headers */
#include "q3a_stats_hw.h"
#include "isp_module.h"

/* isp headers */
#include "isp_pipeline42_parser.h"
#include "isp_log.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_COMMON, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_COMMON, fmt, ##args)

#define STATS_BG_BUF_NUM  4
#define STATS_BG_BUF_SIZE   ((64 * 48 * 6) * 4)

#define STATS_BE_BUF_NUM  4
#define STATS_BE_BUF_SIZE   ((32 * 24 * 6) * 4)

#define STATS_BF_BUF_NUM  8
#define STATS_BF_MAX_RGN 281
#define STATS_BF_NUM_ENTRIES 4
#define STATS_BF_SIZE_PER_ENTRY (sizeof(uint64_t))
#define STATS_BF_BUF_SIZE (STATS_BF_MAX_RGN * STATS_BF_NUM_ENTRIES * STATS_BF_SIZE_PER_ENTRY)

#define STATS_RS_BUF_NUM  4
#define STATS_RS_BUF_SIZE  ((1024 * 4) * 2)

#define STATS_CS_BUF_NUM  4
#define STATS_CS_BUF_SIZE  (5376 * 4)

#define STATS_IHIST_BUF_NUM    4
#define STATS_IHIST_BUF_SIZE  (128 * 4)

#define STATS_BHIST_STATS_BUF_NUM  4
#define STATS_BHIST_STATS_BUF_SIZE  (256 * 4) * 4

/** isp_pipeline42_fill_stats_info:
 *
 *  @session_id: session id
 *  @iface_resource_request: handle to iface_resource_request_t
 *
 *  Fill stats info to request for specific stats
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_pipeline42_parser_fill_stats_info(uint32_t session_id,
  iface_resource_request_t *iface_resource_request,
  isp_stats_mask_ctrl_t *stats_ctrl, boolean fast_aec_mode, uint32_t num_isp,
  isp_parser_session_params_t *parser_session_params __unused)
{
  isp_stats_info_t *stats_info = NULL;
  uint32_t          stats_mask = 0;
  uint32_t i;

  if (!iface_resource_request || !stats_ctrl) {
    ISP_ERR("failed: %p %p", iface_resource_request, stats_ctrl);
    return FALSE;
  }

  stats_mask = stats_ctrl->stats_mask;

  iface_resource_request->num_stats_stream = 0;
  if (num_isp < 1 || num_isp > 2) {
    ISP_ERR("failed : num_isp is wrong %d", num_isp);
    return FALSE;
  }

  /* BG STATS */
  if (stats_mask & (1 << MSM_ISP_STATS_BG)) {
    stats_info = &iface_resource_request->isp_stats_info
      [iface_resource_request->num_stats_stream++];
    stats_info->buf_len = STATS_BG_BUF_SIZE * num_isp;
    if (iface_resource_request->ispif_split_output_info.is_split == TRUE) {
      stats_info->buf_type = ISP_SHARE_BUF;
    } else {
      stats_info->buf_type = ISP_PRIVATE_BUF;
    }
    stats_info->comp_flag = 1;
    stats_info->num_bufs = STATS_BG_BUF_NUM;
    stats_info->session_id = session_id;
    stats_info->stats_type = MSM_ISP_STATS_BG;
    ISP_DBG("bg stats info %d %d %d %d %d %d num %d", stats_info->buf_len,
      stats_info->buf_type, stats_info->comp_flag, stats_info->num_bufs,
      stats_info->session_id, stats_info->stats_type,
      iface_resource_request->num_stats_stream);
  }

  /* BF STATS */
  if ((stats_mask & (1 << MSM_ISP_STATS_BF)) &&
    (fast_aec_mode == FALSE)) {
    stats_info = &iface_resource_request->isp_stats_info
      [iface_resource_request->num_stats_stream++];
    stats_info->buf_len = STATS_BF_BUF_SIZE * num_isp;
    if (iface_resource_request->ispif_split_output_info.is_split == TRUE) {
      stats_info->buf_type = ISP_SHARE_BUF;
    } else {
      stats_info->buf_type = ISP_PRIVATE_BUF;
    }
    stats_info->comp_flag = 0;
    stats_info->num_bufs = STATS_BF_BUF_NUM;
    stats_info->session_id = session_id;
    stats_info->stats_type = MSM_ISP_STATS_BF;
    ISP_DBG("bf stats info %d %d %d %d %d %d num %d", stats_info->buf_len,
      stats_info->buf_type, stats_info->comp_flag, stats_info->num_bufs,
      stats_info->session_id, stats_info->stats_type,
      iface_resource_request->num_stats_stream);
  }

  /* BE STATS */
  if (stats_mask & (1 << MSM_ISP_STATS_BE)) {
    stats_info = &iface_resource_request->isp_stats_info
      [iface_resource_request->num_stats_stream++];
    stats_info->buf_len = STATS_BE_BUF_SIZE * num_isp;
    if (iface_resource_request->ispif_split_output_info.is_split == TRUE) {
       stats_info->buf_type = ISP_SHARE_BUF;
    } else {
      stats_info->buf_type = ISP_PRIVATE_BUF;
    }
    stats_info->comp_flag = 1;
    stats_info->num_bufs = STATS_BE_BUF_NUM;
    stats_info->session_id = session_id;
    stats_info->stats_type = MSM_ISP_STATS_BE;
    ISP_DBG("be stats info %d %d %d %d %d %d num %d", stats_info->buf_len,
      stats_info->buf_type, stats_info->comp_flag, stats_info->num_bufs,
      stats_info->session_id, stats_info->stats_type,
      iface_resource_request->num_stats_stream);
  }

  /* RS STATS */
  if (stats_mask & (1 << MSM_ISP_STATS_RS)) {
     stats_info = &iface_resource_request->isp_stats_info
       [iface_resource_request->num_stats_stream++];
     stats_info->buf_len = STATS_RS_BUF_SIZE * num_isp;
     if (iface_resource_request->ispif_split_output_info.is_split == TRUE) {
        stats_info->buf_type = ISP_SHARE_BUF;
     } else {
       stats_info->buf_type = ISP_PRIVATE_BUF;
     }
     stats_info->comp_flag = 1;
     stats_info->num_bufs = STATS_RS_BUF_NUM;
     stats_info->session_id = session_id;
     stats_info->stats_type = MSM_ISP_STATS_RS;
     ISP_DBG("rs stats info %d %d %d %d %d %d num %d", stats_info->buf_len,
       stats_info->buf_type, stats_info->comp_flag, stats_info->num_bufs,
       stats_info->session_id, stats_info->stats_type,
       iface_resource_request->num_stats_stream);
  }

  /* CS STATS */
  if (stats_mask & (1 << MSM_ISP_STATS_CS)) {
     stats_info = &iface_resource_request->isp_stats_info
       [iface_resource_request->num_stats_stream++];
     stats_info->buf_len = STATS_CS_BUF_SIZE * num_isp;
     if (iface_resource_request->ispif_split_output_info.is_split == TRUE) {
       stats_info->buf_type = ISP_SHARE_BUF;
     } else {
       stats_info->buf_type = ISP_PRIVATE_BUF;
     }
     stats_info->comp_flag = 1;
     stats_info->num_bufs = STATS_CS_BUF_NUM;
     stats_info->session_id = session_id;
     stats_info->stats_type = MSM_ISP_STATS_CS;
     ISP_DBG("cs stats info %d %d %d %d %d %d num %d", stats_info->buf_len,
       stats_info->buf_type, stats_info->comp_flag, stats_info->num_bufs,
       stats_info->session_id, stats_info->stats_type,
       iface_resource_request->num_stats_stream);
  }

  /* BHIST STATS */
  if (stats_mask & (1 << MSM_ISP_STATS_BHIST)) {
     stats_info = &iface_resource_request->isp_stats_info
       [iface_resource_request->num_stats_stream++];
     stats_info->buf_len = STATS_BHIST_STATS_BUF_SIZE * num_isp;
     if (iface_resource_request->ispif_split_output_info.is_split == TRUE) {
       stats_info->buf_type = ISP_SHARE_BUF;
     } else {
       stats_info->buf_type = ISP_PRIVATE_BUF;
     }
     stats_info->comp_flag = 1;
     stats_info->num_bufs = STATS_BHIST_STATS_BUF_NUM;
     stats_info->session_id = session_id;
     stats_info->stats_type = MSM_ISP_STATS_BHIST;
     ISP_DBG("bhist stats info %d %d %d %d %d %d num %d", stats_info->buf_len,
       stats_info->buf_type, stats_info->comp_flag, stats_info->num_bufs,
       stats_info->session_id, stats_info->stats_type,
       iface_resource_request->num_stats_stream);
  }

  /*IHIST STATS*/
  if (stats_mask & (1 << MSM_ISP_STATS_IHIST)) {
     stats_info = &iface_resource_request->isp_stats_info
       [iface_resource_request->num_stats_stream++];
     stats_info->buf_len = STATS_IHIST_BUF_SIZE* num_isp;
     if (iface_resource_request->ispif_split_output_info.is_split == TRUE) {
       stats_info->buf_type = ISP_SHARE_BUF;
     } else {
       stats_info->buf_type = ISP_PRIVATE_BUF;
     }
     stats_info->comp_flag = 1;
     stats_info->num_bufs = STATS_IHIST_BUF_NUM;
     stats_info->session_id = session_id;
     stats_info->stats_type = MSM_ISP_STATS_IHIST;
     ISP_HIGH("ihist stats info %d %d %d %d %d %d", stats_info->buf_len,
       stats_info->buf_type, stats_info->comp_flag, stats_info->num_bufs,
       stats_info->session_id, stats_info->stats_type);
  }

  for (i = 0; i < iface_resource_request->num_stats_stream; i++) {
    stats_info = &iface_resource_request->isp_stats_info[i];
    stats_info->parse_flag = 1;
  }

  return TRUE;
}

/** isp_pipeline42_parser_bf_stats:
 *
 *  @input_buf: input buffer
 *  @output: output params to be filled
 *  @stats_3a_data: stats 3a data handle
 *  @saved_stats_params_isp0: stats params for ISP 0
 *  @saved_stats_params_isp1: stats params for ISP 1
 *
 *  Parse the stats parameters received from raw_buf to their
 *  corresponding struct
 *
 *  Return TRUE on success and FALSE on failure
 **/

boolean isp_pipeline42_parser_bf_stats(void *input_buf,
  mct_event_stats_isp_t *output,
  isp_saved_stats_params_t *saved_stats_params,
  isp_parser_session_params_t *parser_session_params)
{
  int                         window = 0;
  uint32_t                    i = 0;
  uint64_t                   *H1_Sh = NULL,
                             *V_Sh = NULL,
                             *H1_Sum = NULL,
                             *V_Sum = NULL;
  uint32_t                   *H1_Cnt = NULL,
                             *V_Cnt = NULL,
                              num_roi = 0;
  uint32_t                    region_num;
  uint64_t                   *current_region = NULL;
  q3a_bf_stats_t             *bf_stats = NULL;
  mct_event_stats_isp_data_t *stats_data = NULL;
  bf_fw_stats_t              *fw_stats = NULL;
  bf_fw_roi_cfg_t            *bf_fw_roi_cfg = NULL;
  static uint32_t            abc = 0;
  char file_name[FILENAME_MAX];
  int file_fd;

  if (!input_buf || !output || !saved_stats_params ||
    !parser_session_params) {
    ISP_ERR("failed: %p %p %p %p", input_buf, output,
      saved_stats_params, parser_session_params);
    return FALSE;
  }

  bf_fw_roi_cfg =
    &saved_stats_params->bf_fw_stats_cfg_info.bf_fw_roi_cfg_end_pix_order;
  if (bf_fw_roi_cfg->is_valid == FALSE) {
    ISP_ERR("failed: is_valid FALSE");
    return FALSE;
  }

  output->stats_mask |= (1 << MSM_ISP_STATS_BF);
  stats_data = &output->stats_data[MSM_ISP_STATS_BF];
  if (stats_data->buf_size != sizeof(q3a_bf_stats_t)) {
    ISP_ERR("failed: invalid buf_size %d expected %d", stats_data->buf_size,
      sizeof(q3a_bf_stats_t));
    return FALSE;
  }

  bf_stats = (q3a_bf_stats_t *)stats_data->stats_buf;
  if (!bf_stats) {
    ISP_ERR("failed: bf_stats NULL");
    return FALSE;
  }

  stats_data->stats_type = MSM_ISP_STATS_BF;
  stats_data->used_size = sizeof(*bf_stats);
  ISP_DBG(" bf stats_mask %x type %d buf %p size %d",
    output->stats_mask, stats_data->stats_type, stats_data->stats_buf,
    stats_data->buf_size);

  bf_stats->mask = MCT_EVENT_STATS_BF_FW;
  fw_stats = &bf_stats->u.fw_stats;
  num_roi = bf_fw_roi_cfg->num_bf_fw_roi_dim;
  if (num_roi > BF_FW_MAX_ROI_REGIONS) {
    ISP_ERR("num_roi %d is greater than max roi regions", num_roi);
    return FALSE;
  }

  H1_Cnt     = fw_stats->bf_h1_num;
  H1_Sum     = fw_stats->bf_h1_sum;
  H1_Sh      = fw_stats->bf_h1_sharp;
  V_Cnt      = fw_stats->bf_v_num;
  V_Sum      = fw_stats->bf_v_sum;
  V_Sh       = fw_stats->bf_v_sharp;

  current_region = (uint64_t *)input_buf;

  ISP_DBG("input buf size = %d",sizeof((uint64_t *)input_buf));
  #if 0
    snprintf(file_name, sizeof(file_name),
      QCAMERA_DUMP_FRM_LOCATION"%s_%04d.%s", "bf_stats", abc++, "txt");
    file_fd = open(file_name, O_RDWR | O_CREAT, 0777);
    if (file_fd < 0) {
      ISP_ERR("%s: cannot open file %s \n", __func__, file_name);
    } else {
      write(file_fd, (uint8_t *)input_buf, stats_data->buf_size);
      close(file_fd);
      ISP_ERR("Dumped input_buf to %s", file_name);
    }
  #endif
  for (i = 0; i < bf_fw_roi_cfg->num_bf_fw_roi_dim  &&
        bf_fw_roi_cfg->bf_fw_roi_dim[i].is_valid == TRUE; i++) {

    region_num = bf_fw_roi_cfg->bf_fw_roi_dim[i].region_num;
    if (region_num >= BF_FW_MAX_ROI_REGIONS) {
      ISP_ERR("region number > BF_FW_MAX_ROI_REGIONS");
      return FALSE;
    }

    /*parse AF stats */
    H1_Cnt[region_num] += ((*(current_region)) & 0x3FFFFF8000000000) >> 39;
    H1_Sum[region_num] += ((*(current_region)) & 0x1FFFFFFFFF);
    current_region++;
    H1_Sh[region_num] += ((*(current_region)) & 0xFFFFFFFFFF);
    current_region++;
    V_Cnt[region_num] += ((*(current_region)) & 0x3FFFFF8000000000) >> 39;
    V_Sum[region_num] += ((*(current_region)) & 0x1FFFFFFFFF);
    current_region++;
    V_Sh[region_num] += ((*(current_region)) & 0xFFFFFFFFFF);
    current_region++;

    ISP_DBG("h1_cnt = %d h1_sum = %"PRId64" h1_sh = %"PRId64", v_cnt = %d, \
      V_sum = %"PRId64", V_sh = %"PRId64" \n", H1_Cnt[region_num],
      H1_Sum[region_num], H1_Sh[region_num], V_Cnt[region_num],
      V_Sum[region_num], V_Sh[region_num]);
  }

  fw_stats->bf_fw_roi_cfg =
    saved_stats_params->bf_fw_stats_cfg_info.bf_fw_roi_cfg_3a_order;

  return TRUE;
}

/** isp_pipeline42_parser_bg_stats:
 *
 *  @input_buf: input buffer
 *  @output: output params to be filled
 *  @saved_stats_params_isp0: stats params for ISP 0
 *  @saved_stats_params_isp1: stats params for ISP 1
 *
 *  Parse the different stats parameters received from raw_buf to their
 *  corresponding struct
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_pipeline42_parser_bg_stats(void *input_buf,
  mct_event_stats_isp_t *output,
  isp_saved_stats_params_t *saved_stats_params,
  isp_parser_session_params_t *parser_session_params)
{
  uint32_t                   *SY = NULL,*Sr = NULL, *Sb = NULL, *Sgr = NULL,
                             *Sgb = NULL;
  uint32_t                   *r_num = NULL, *b_num = NULL, *gr_num = NULL,
                             *gb_num = NULL;
  uint32_t                   *current_region = NULL;
  uint32_t                    i = 0, j = 0, x = 0, y = 0;
  uint32_t                    h_rgns_start = 0;
  uint32_t                    h_rgns_end = 0;
  uint32_t                    h_rgns_total = 0;
  uint32_t                    v_rgns_total = 0;
  q3a_bg_stats_t             *bg_stats = NULL;
  isp_rgns_stats_param_t     *rgns_stats = NULL;
  mct_event_stats_isp_data_t *stats_data = NULL;
  isp_stats_roi_params_t     *roi_params = NULL;

  if (!input_buf || !output || !saved_stats_params ||
    !parser_session_params) {
    ISP_ERR("failed: %p %p %p %p", input_buf, output,
      saved_stats_params, parser_session_params);
    return FALSE;
  }

  rgns_stats = &saved_stats_params->rgns_stats[MSM_ISP_STATS_BG];
  ISP_DBG("bg rgns_stats %d %d %d %d %d", rgns_stats->is_valid,
    rgns_stats->h_rgns_start, rgns_stats->h_rgns_end,
    rgns_stats->h_rgns_total, rgns_stats->v_rgns_total);

  if (rgns_stats->is_valid == FALSE) {
    ISP_ERR("failed: is_valid FALSE");
    return FALSE;
  }

  roi_params = &saved_stats_params->stats_roi_params[MSM_ISP_STATS_BG];

  output->stats_mask |= (1 << MSM_ISP_STATS_BG);
  stats_data = &output->stats_data[MSM_ISP_STATS_BG];
  if (stats_data->buf_size != sizeof(q3a_bg_stats_t)) {
    ISP_ERR("failed: invalid buf_size %d expected %d", stats_data->buf_size,
      sizeof(q3a_bg_stats_t));
    return FALSE;
  }

  bg_stats = (q3a_bg_stats_t *)stats_data->stats_buf;
  if (!bg_stats) {
    ISP_ERR("failed: bg_stats NULL");
    return FALSE;
  }

  stats_data->stats_type = MSM_ISP_STATS_BG;
  stats_data->used_size = sizeof(*bg_stats);
  ISP_DBG("bg stats_mask %x type %d buf %p size %d",
    output->stats_mask, stats_data->stats_type, stats_data->stats_buf,
    stats_data->buf_size);

  h_rgns_start = rgns_stats->h_rgns_start;
  h_rgns_end = rgns_stats->h_rgns_end;
  h_rgns_total = rgns_stats->h_rgns_total;
  v_rgns_total = rgns_stats->v_rgns_total;

  Sr     = bg_stats->bg_r_sum;
  Sb     = bg_stats->bg_b_sum;
  Sgr    = bg_stats->bg_gr_sum;
  Sgb    = bg_stats->bg_gb_sum;
  r_num  = bg_stats->bg_r_num;
  b_num  = bg_stats->bg_b_num;
  gr_num = bg_stats->bg_gr_num;
  gb_num = bg_stats->bg_gb_num;

  current_region = (uint32_t*)input_buf;
  bg_stats->bg_region_h_num += (h_rgns_end - h_rgns_start + 1);
  bg_stats->bg_region_v_num = v_rgns_total;
  /*
   * BG Stats expect:
   * 1 - 23bit out of 32bit r_sum
   * 2 - 23bit out of 32bit b_sum
   * 3 - 23bit out of 32bit gr_sum
   * 4 - 23bit out of 32bit gb_sum
   * 5 - 15bit out of 32bit USL bnum, 15bit out of 32bit LSL rnum
   * 6 - 15bit out of 32bit USL gbnum, 15bit out of 32bit LSL grnum
   * Expect buf_size = 72*54 * 6 = 23328  (uint32)  93312
   */
  for (j = 0; j < v_rgns_total; j++) {
    for (i = 0; i < h_rgns_total; i++) {
      if (i >= h_rgns_start && i <= h_rgns_end) {
        /* 23 bits sum of r, b, gr, gb. */
        *Sr = ((*(current_region)) & 0x007FFFFF);
        current_region ++;
        *Sb = ((*(current_region)) & 0x007FFFFF);
        current_region ++;
        *Sgr = ((*(current_region)) & 0x007FFFFF);
        current_region ++;
        *Sgb = ((*(current_region)) & 0x007FFFFF);
        current_region ++;
        /*15 bit pixel count used for r_sum, b_sum, gr_sum and gb_sum*/
        *r_num = ((*(current_region)) & 0x00007FFF);
        *b_num = ((*(current_region)) & 0x7FFF0000) >> 16;
        current_region++;
        *gr_num = ((*(current_region)) & 0x00007FFF);
        *gb_num = ((*(current_region)) & 0x7FFF0000) >> 16;
        current_region ++;
      }
      Sr++;
      Sb++;
      Sgr++;
      Sgb++;
      r_num++;
      b_num++;
      gr_num++;
      gb_num++;
    }
  }
  bg_stats->bg_region_height = roi_params->rgnHeight + 1;
  bg_stats->bg_region_width = roi_params->rgnWidth + 1;
  bg_stats->region_pixel_cnt =
    (roi_params->rgnWidth + 1) * (roi_params->rgnHeight + 1);
  bg_stats->rMax = roi_params->rMax;
  bg_stats->bMax = roi_params->bMax;
  bg_stats->grMax = roi_params->grMax;
  bg_stats->gbMax = roi_params->gbMax;

  /* convert bayer r,g,b stat into Ysum to make
   * it work on current 3a version
   * that uses 16x16 Ysum is done in 3A code
   **/
  return TRUE;
}

/** isp_pipeline42_parser_be_stats:
 *
 *  @input_buf: input buffer
 *  @output: output params to be filled
 *  @stats_3a_data: stats 3a data handle
 *  @saved_stats_params: stats params
 *
 *  Parse the different stats parameters received from raw_buf to their
 *  corresponding struct
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_pipeline42_parser_be_stats(void *input_buf,
  mct_event_stats_isp_t *output,
  isp_saved_stats_params_t *saved_stats_params,
  isp_parser_session_params_t *parser_session_params)
{
  int32_t                   *SY = NULL,*Sr = NULL, *Sb = NULL, *Sgr = NULL,
                             *Sgb = NULL;
  int32_t                   *r_num = NULL, *b_num = NULL, *gr_num = NULL,
                             *gb_num = NULL;
  uint32_t                   *current_region = NULL;
  uint32_t                    i = 0, j = 0, x = 0, y = 0;
  uint32_t                    h_rgns_start = 0;
  uint32_t                    h_rgns_end = 0;
  uint32_t                    h_rgns_total = 0;
  uint32_t                    v_rgns_total = 0;
  q3a_be_stats_t             *be_stats = NULL;
  isp_rgns_stats_param_t     *rgns_stats = NULL;
  mct_event_stats_isp_data_t *stats_data = NULL;
  isp_stats_roi_params_t     *roi_params = NULL;

  if (!input_buf || !output || !saved_stats_params ||
    !parser_session_params) {
    ISP_ERR("failed: %p %p %p %p", input_buf, output,
      saved_stats_params, parser_session_params);
    return FALSE;
  }

  rgns_stats = &saved_stats_params->rgns_stats[MSM_ISP_STATS_BE];
  ISP_DBG("be rgns_stats %d %d %d %d %d", rgns_stats->is_valid,
    rgns_stats->h_rgns_start, rgns_stats->h_rgns_end,
    rgns_stats->h_rgns_total, rgns_stats->v_rgns_total);

  if (rgns_stats->is_valid == FALSE) {
    ISP_ERR("failed: is_valid FALSE");
    return FALSE;
  }

  roi_params = &saved_stats_params->stats_roi_params[MSM_ISP_STATS_BE];

  output->stats_mask |= (1 << MSM_ISP_STATS_BE);
  stats_data = &output->stats_data[MSM_ISP_STATS_BE];
  if (stats_data->buf_size != sizeof(q3a_be_stats_t)) {
    ISP_ERR("failed: invalid buf_size %d expected %d", stats_data->buf_size,
      sizeof(q3a_be_stats_t));
    return FALSE;
  }
  be_stats = (q3a_be_stats_t *)stats_data->stats_buf;
  if (!be_stats) {
    ISP_ERR("failed: be_stats NULL");
    return FALSE;
  }

  stats_data->stats_type = MSM_ISP_STATS_BE;
  stats_data->used_size = sizeof(*be_stats);
  ISP_DBG("be stats_mask %x type %d buf %p size %d",
    output->stats_mask, stats_data->stats_type, stats_data->stats_buf,
    stats_data->buf_size);

  h_rgns_start = rgns_stats->h_rgns_start;
  h_rgns_end = rgns_stats->h_rgns_end;
  h_rgns_total = rgns_stats->h_rgns_total;
  v_rgns_total = rgns_stats->v_rgns_total;

  Sr     = be_stats->r_sum;
  Sb     = be_stats->b_sum;
  Sgr    = be_stats->gr_sum;
  Sgb    = be_stats->gb_sum;
  r_num  = be_stats->r_count;
  b_num  = be_stats->b_count;
  gr_num = be_stats->gr_count;
  gb_num = be_stats->gb_count;

  current_region = (uint32_t*)input_buf;

  be_stats->nx = rgns_stats->h_rgns_total;
  be_stats->ny = rgns_stats->v_rgns_total;
  be_stats->dx = roi_params->rgnHOffset;
  be_stats->dy = roi_params->rgnHOffset;
  be_stats->sx = roi_params->rgnWidth + 1;//* h_rgns_end;
  be_stats->sy = roi_params->rgnHeight + 1;//* (v_rgns_total-1);
  /*
   * BE Stats expect:
   * 1 - 24bit out of 32bit r_sum
   * 2 - 24bit out of 32bit b_sum
   * 3 - 24bit out of 32bit gr_sum
   * 4 - 24bit out of 32bit gb_sum
   * 5 - 15bit out of 32bit USL bnum, 15bit out of 32bit LSL rnum
   * 6 - 15bit out of 32bit USL gbnum, 15bit out of 32bit LSL grnum
   * Expect buf_size = 72*54 * 6 = 23328  (uint32)  93312
   */
  for (j = 0; j < v_rgns_total; j++) {
    for (i = 0; i < h_rgns_total; i++) {
      if (i >= h_rgns_start && i <= h_rgns_end) {
        /* 24 bits sum of r, b, gr, gb. */
        *Sr = ((*(current_region)) & 0x00FFFFFF);
        current_region ++;
        *Sb = ((*(current_region)) & 0x00FFFFFF);
        current_region ++;
        *Sgr = ((*(current_region)) & 0x00FFFFFF);
        current_region ++;
        *Sgb = ((*(current_region)) & 0x00FFFFFF);
        current_region ++;
        /*16 bit pixel count used for r_sum, b_sum, gr_sum and gb_sum*/
        *r_num = ((*(current_region)) & 0x0000FFFF);
        *b_num = ((*(current_region)) & 0xFFFF0000) >> 16;
        current_region++;
        *gr_num = ((*(current_region)) & 0x0000FFFF);
        *gb_num = ((*(current_region)) & 0xFFFF0000) >> 16;
        current_region ++;
      }
      Sr++;
      Sb++;
      Sgr++;
      Sgb++;
      r_num++;
      b_num++;
      gr_num++;
      gb_num++;
    }
  }
  be_stats->rMax = roi_params->rMax;
  be_stats->bMax = roi_params->bMax;
  be_stats->grMax = roi_params->grMax;
  be_stats->gbMax = roi_params->gbMax;
  /* convert bayer r,g,b stat into Ysum to make
   * it work on current 3a version
   * that uses 16x16 Ysum is done in 3A code
   **/
  return TRUE;
}

/** isp_pipeline42_parser_rs_stats:
 *
 *  @input_buf: input buffer
 *  @output: output params to be filled
 *  @stats_3a_data: stats 3a data handle
 *  @saved_stats_params_isp0: stats params for ISP 0
 *  @saved_stats_params_isp1: stats params for ISP 1
 *
 *  Parse the different stats parameters received from raw_buf to their
 *  corresponding struct
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_pipeline42_parser_rs_stats(void *input_buf,
  mct_event_stats_isp_t *output,
  isp_saved_stats_params_t *saved_stats_params_isp,
  isp_parser_session_params_t *parser_session_params)
{
  uint16_t                   *current_region = NULL;
  uint32_t                    i = 0, j = 0, x = 0, y = 0;
  uint32_t                    h_rgns_start = 0;
  uint32_t                    h_rgns_end = 0;
  uint32_t                    h_rgns_total = 0;
  uint32_t                    v_rgns_total = 0;
  q3a_rs_stats_t             *rs_stats = NULL;
  isp_rgns_stats_param_t     *rgns_stats = NULL;
  mct_event_stats_isp_data_t *stats_data = NULL;
  uint32_t                    num_rs_h_rgn = 0;

  if (!input_buf || !output || !saved_stats_params_isp ||
    !parser_session_params) {
    ISP_ERR("failed: %p %p %p %p", input_buf, output,
      saved_stats_params_isp, parser_session_params);
    return FALSE;
  }

  rgns_stats = &saved_stats_params_isp->rgns_stats[MSM_ISP_STATS_RS];
  ISP_DBG("rs rgns_stats %d %d %d %d %d", rgns_stats->is_valid,
    rgns_stats->h_rgns_start, rgns_stats->h_rgns_end,
    rgns_stats->h_rgns_total, rgns_stats->v_rgns_total);

  if (rgns_stats->is_valid == FALSE) {
    ISP_ERR("failed: is_valid FALSE");
    return FALSE;
  }

  output->stats_mask |= (1 << MSM_ISP_STATS_RS);
  stats_data = &output->stats_data[MSM_ISP_STATS_RS];
  if (stats_data->buf_size != sizeof(q3a_rs_stats_t)) {
    ISP_ERR("failed: invalid buf_size %d expected %d", stats_data->buf_size,
      sizeof(q3a_rs_stats_t));
    return FALSE;
  }
  rs_stats = (q3a_rs_stats_t *)stats_data->stats_buf;
  if (!rs_stats) {
    ISP_ERR("failed: rs_stats NULL");
    return FALSE;
  }

  stats_data->stats_type = MSM_ISP_STATS_RS;
  stats_data->used_size = sizeof(*rs_stats);
  ISP_DBG("rs stats_mask %x type %d buf %p size %d",
    output->stats_mask, stats_data->stats_type, stats_data->stats_buf,
    stats_data->buf_size);

  h_rgns_start = rgns_stats->h_rgns_start;
  h_rgns_end = rgns_stats->h_rgns_end;
  h_rgns_total = rgns_stats->h_rgns_total;
  v_rgns_total = rgns_stats->v_rgns_total;

  current_region = (uint16_t *)input_buf;
  num_rs_h_rgn = h_rgns_end - h_rgns_start + 1;

  /* consider two VFE are splitting the only H rgn,
     the row num is still 1 * v_rgns_total
     or left VFE rgn and right rgn will add up togehter*/
  if (num_rs_h_rgn == 1 && h_rgns_total == 1) {
    rs_stats->num_row_sum = num_rs_h_rgn * v_rgns_total;
    rs_stats->num_h_regions = num_rs_h_rgn;
  } else {
    rs_stats->num_row_sum += num_rs_h_rgn * v_rgns_total;
    rs_stats->num_h_regions += num_rs_h_rgn;
  }
  rs_stats->num_v_regions = v_rgns_total;
  for (j = 0; j < v_rgns_total; j++) {
    for (i = h_rgns_start; i <= h_rgns_end; i++) {
      rs_stats->row_sum[i][j] += (*current_region++);
    }
  }
  return TRUE;
}

/** isp_pipeline42_parser_cs_stats:
 *
 *  @input_buf: input buffer
 *  @output: output params to be filled
 *  @stats_3a_data: stats 3a data handle
 *  @saved_stats_params_isp0: stats params for ISP 0
 *  @saved_stats_params_isp1: stats params for ISP 1
 *
 *  Parse the different stats parameters received from raw_buf to their
 *  corresponding struct
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_pipeline42_parser_cs_stats(void *input_buf,
  mct_event_stats_isp_t *output,
  isp_saved_stats_params_t *saved_stats_params,
  isp_parser_session_params_t *parser_session_params)
{
  uint16_t                   *current_region = NULL;
  uint32_t                    i = 0, j = 0, x = 0, y = 0;
  uint32_t                    h_rgns_start = 0;
  uint32_t                    h_rgns_end = 0;
  uint32_t                    h_rgns_total = 0;
  uint32_t                    v_rgns_total = 0;
  q3a_cs_stats_t             *cs_stats = NULL;
  isp_rgns_stats_param_t     *rgns_stats = NULL;
  mct_event_stats_isp_data_t *stats_data = NULL;
  uint32_t                   *CSum = NULL;
  uint32_t                    shiftBits = 0;

  if (!input_buf || !output || !saved_stats_params ||
    !parser_session_params) {
    ISP_ERR("failed: %p %p %p %p", input_buf, output,
      saved_stats_params, parser_session_params);
    return FALSE;
  }

  rgns_stats = &saved_stats_params->rgns_stats[MSM_ISP_STATS_CS];
  shiftBits = saved_stats_params->cs_shift_bits;
  ISP_DBG("rs rgns_stats %d %d %d %d %d", rgns_stats->is_valid,
    rgns_stats->h_rgns_start, rgns_stats->h_rgns_end,
    rgns_stats->h_rgns_total, rgns_stats->v_rgns_total);

  if (rgns_stats->is_valid == FALSE) {
    ISP_ERR("failed: is_valid FALSE");
    return FALSE;
  }

  output->stats_mask |= (1 << MSM_ISP_STATS_CS);
  stats_data = &output->stats_data[MSM_ISP_STATS_CS];
  if (stats_data->buf_size != sizeof(q3a_cs_stats_t)) {
    ISP_ERR("failed: invalid buf_size %d expected %d", stats_data->buf_size,
      sizeof(q3a_cs_stats_t));
    return FALSE;
  }
  cs_stats = (q3a_cs_stats_t *)stats_data->stats_buf;
  if (!cs_stats) {
    ISP_ERR("failed: cs_stats NULL");
    return FALSE;
  }

  stats_data->stats_type = MSM_ISP_STATS_CS;
  stats_data->used_size = sizeof(*cs_stats);
  ISP_DBG("cs stats_mask %x type %d buf %p size %d",
    output->stats_mask, stats_data->stats_type, stats_data->stats_buf,
    stats_data->buf_size);

  h_rgns_start = rgns_stats->h_rgns_start;
  h_rgns_end = rgns_stats->h_rgns_end;
  h_rgns_total = rgns_stats->h_rgns_total;
  v_rgns_total = rgns_stats->v_rgns_total;

  current_region = (uint16_t *)input_buf;
  CSum = cs_stats->col_sum;
  cs_stats->num_col_sum += (h_rgns_end - h_rgns_start + 1) * v_rgns_total;
  ISP_DBG("num = %d, shiftBits = %d\n", cs_stats->num_col_sum, shiftBits);

  for (j = 0; j < v_rgns_total; j++) {
    CSum = CSum + h_rgns_start;
    for (i = h_rgns_start; i <= h_rgns_end; i++)
      *CSum++ = (*current_region++) << shiftBits;
    CSum = CSum + h_rgns_total - h_rgns_end - 1;
  }

  return TRUE;
}

/** isp_pipeline42_parser_ihist_stats:
 *
 *  @input_buf: input buffer
 *  @output: output params to be filled
 *  @stats_3a_data: stats 3a data handle
 *  @saved_stats_params_isp0: stats params for ISP 0
 *  @saved_stats_params_isp1: stats params for ISP 1
 *
 *  Parse the different stats parameters received from raw_buf to their
 *  corresponding struct
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_pipeline42_parser_ihist_stats(void *input_buf,
  mct_event_stats_isp_t *output,
  isp_saved_stats_params_t *saved_stats_params,
  isp_parser_session_params_t *parser_session_params)
{
  uint32_t                    i = 0;
  uint16_t                   *hist_statsBuffer = NULL;
  q3a_ihist_stats_t          *ihist_stats = NULL;
  mct_event_stats_isp_data_t *stats_data = NULL;

  if (!input_buf || !output || !saved_stats_params ||
    !parser_session_params) {
    ISP_ERR("failed: %p %p %p %p", input_buf, output,
      saved_stats_params, parser_session_params);
    return FALSE;
  }

  output->stats_mask |= (1 << MSM_ISP_STATS_IHIST);
  stats_data = &output->stats_data[MSM_ISP_STATS_IHIST];
  if (stats_data->buf_size != sizeof(q3a_ihist_stats_t)) {
    ISP_ERR("failed: invalid buf_size %d expected %d", stats_data->buf_size,
      sizeof(q3a_ihist_stats_t));
    return FALSE;
  }
  ihist_stats = (q3a_ihist_stats_t *)stats_data->stats_buf;
  if (!ihist_stats) {
    ISP_ERR("failed: ihist_stats NULL");
    return FALSE;
  }

  stats_data->stats_type = MSM_ISP_STATS_IHIST;
  stats_data->used_size = sizeof(*ihist_stats);

  ISP_DBG("stats_mask %x type %d buf %p size %d",
     output->stats_mask, stats_data->stats_type, stats_data->stats_buf,
     stats_data->buf_size);

  hist_statsBuffer = (uint16_t *)input_buf;
  ihist_stats->valid_fields = IHIST_VALID_HISTOGRAM;
  ihist_stats->num_bins = 256;
  for (i= 0; i< ihist_stats->num_bins; i++) {
    ihist_stats->histogram[i] += *hist_statsBuffer;
    hist_statsBuffer++;

  }

  return TRUE;
}

/** isp_pipeline42_parser_bhist_stats:
 *
 *  @input_buf: input buffer
 *  @output: output params to be filled
 *  @stats_3a_data: stats 3a data handle
 *  @saved_stats_params_isp0: stats params for ISP 0
 *  @saved_stats_params_isp1: stats params for ISP 1
 *
 *  Parse the different stats parameters received from raw_buf to their
 *  corresponding struct
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_pipeline42_parser_bhist_stats(void *input_buf,
  mct_event_stats_isp_t *output,
  isp_saved_stats_params_t *saved_stats_params,
  isp_parser_session_params_t *parser_session_params)
{
  uint32_t                   *current_region = NULL;
  uint32_t                    i = 0;
  q3a_bhist_stats_t          *bhist_stats = NULL;
  mct_event_stats_isp_data_t *stats_data = NULL;
  uint32_t                   *Srh = NULL, *Sbh = NULL, *Sgrh = NULL,
                             *Sgbh = NULL;

  if (!input_buf || !output || !saved_stats_params ||
    !parser_session_params) {
    ISP_ERR("failed: %p %p %p %p", input_buf, output,
      saved_stats_params, parser_session_params);
    return FALSE;
  }

  output->stats_mask |= (1 << MSM_ISP_STATS_BHIST);
  stats_data = &output->stats_data[MSM_ISP_STATS_BHIST];
  if (stats_data->buf_size != sizeof(q3a_bhist_stats_t)) {
    ISP_ERR("failed: invalid buf_size %d expected %d", stats_data->buf_size,
      sizeof(q3a_bhist_stats_t));
    return FALSE;
  }
  bhist_stats = (q3a_bhist_stats_t *)stats_data->stats_buf;
  if (!bhist_stats) {
    ISP_ERR("failed: bhist_stats NULL");
    return FALSE;
  }

  bhist_stats->num_bins = MAX_HIST_STATS_NUM;
  stats_data->stats_type = MSM_ISP_STATS_BHIST;
  stats_data->used_size = sizeof(*bhist_stats);
  ISP_DBG("bhist stats_mask %x type %d buf %p size %d",
    output->stats_mask, stats_data->stats_type, stats_data->stats_buf,
    stats_data->buf_size);

  Srh = bhist_stats->bayer_r_hist;
  Sbh = bhist_stats->bayer_b_hist;
  Sgrh = bhist_stats->bayer_gr_hist;
  Sgbh = bhist_stats->bayer_gb_hist;
  current_region = input_buf;

  /* 0 to 255, total 256 bins */
  for (i = 0; i < MAX_HIST_STATS_NUM; i++) {
    *Srh += ((*(current_region)) & 0x007FFFFF);
    Srh++; current_region++;
    *Sbh += ((*(current_region)) & 0x007FFFFF);
    Sbh++; current_region++;
    *Sgrh += ((*(current_region)) & 0x007FFFFF);
    Sgrh++; current_region++;
    *Sgbh += ((*(current_region)) & 0x007FFFFF);
    Sgbh++; current_region++;
  }

  bhist_stats->bayer_b_hist_valid = bhist_stats->bayer_gb_hist_valid =
    bhist_stats->bayer_gr_hist_valid = bhist_stats->bayer_r_hist_valid = true;

  return TRUE;
}

