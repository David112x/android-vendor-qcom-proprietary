/* isp_pipeline44_parser.h
 *
 * Copyright (c) 2012-2014, 2017 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* mctl headers */
#include "q3a_stats_hw.h"
#include "isp_module.h"

/* isp headers */
#include "isp_pipeline44_parser.h"
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

#define STATS_BF_BUF_NUM  4
#define STATS_BF_BUF_SIZE  ((18 * 14 * 14) * 4)

#define STATS_RS_BUF_NUM  4
#define STATS_RS_BUF_SIZE  ((1024 * 4) * 2)

#define STATS_CS_BUF_NUM  4
#define STATS_CS_BUF_SIZE  (5376 * 4)

#define STATS_IHIST_BUF_NUM    4
#define STATS_IHIST_BUF_SIZE  (128 * 4)

#define STATS_BHIST_STATS_BUF_NUM  4
#define STATS_BHIST_STATS_BUF_SIZE  (256 * 4) * 4

#define STATS_BF_SCALE_BUF_NUM  4
#define STATS_BF_SCALE_BUF_SIZE  ((18 * 14 * 14) * 4)

/* #define PIPELINE44_DEBUG */
#ifdef PIPELINE44_DEBUG
#undef ISP_DBG
#define ISP_DBG ISP_ERR
#undef ISP_HIGH
#define ISP_HIGH ISP_ERR
#endif

/** isp_pipeline44_fill_stats_info:
 *
 *  @session_id: session id
 *  @iface_resource_request: handle to iface_resource_request_t
 *
 *  Fill stats info to request for specific stats
 **/
boolean isp_pipeline44_parser_fill_stats_info(uint32_t session_id,
  iface_resource_request_t *iface_resource_request,
  isp_stats_mask_ctrl_t *stats_ctrl, boolean fast_aec_mode, uint32_t num_isp,
  isp_parser_session_params_t *parser_session_params __unused)
{
  isp_stats_info_t *stats_info = NULL;
  uint32_t          stats_mask = 0;
  uint32_t i;

  if (!iface_resource_request || !stats_ctrl) {
    ISP_ERR("failed: iface_resource_request %p stats_ctrl %p",
      iface_resource_request, stats_ctrl);
    return FALSE;
  }

  stats_mask = stats_ctrl->stats_mask;

  iface_resource_request->num_stats_stream = 0;
  if (num_isp < 1 || num_isp > 2) {
    ISP_ERR("failed : num_isp is wrong %d", num_isp);
    return FALSE;
  }

  /* BF STATS */
  if ((stats_mask & (1 << MSM_ISP_STATS_BF)) && (fast_aec_mode == FALSE)) {
     stats_info = &iface_resource_request->isp_stats_info
       [iface_resource_request->num_stats_stream++];
     stats_info->buf_len = STATS_BF_BUF_SIZE * num_isp;
     if (iface_resource_request->ispif_split_output_info.is_split == TRUE) {
        stats_info->buf_type = ISP_SHARE_BUF;
     } else {
       stats_info->buf_type = ISP_PRIVATE_BUF;
     }
     stats_info->comp_flag = STATS_COMPOSITE_GRP_2;
     stats_info->num_bufs = STATS_BF_BUF_NUM;
     stats_info->session_id = session_id;
     stats_info->stats_type = MSM_ISP_STATS_BF;
     ISP_DBG("bf stats info %d %d %d %d %d %d num %d", stats_info->buf_len,
       stats_info->buf_type, stats_info->comp_flag, stats_info->num_bufs,
       stats_info->session_id, stats_info->stats_type,
       iface_resource_request->num_stats_stream);
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
     stats_info->comp_flag = STATS_COMPOSITE_GRP_1;
     stats_info->num_bufs = STATS_BG_BUF_NUM;
     stats_info->session_id = session_id;
     stats_info->stats_type = MSM_ISP_STATS_BG;
     ISP_DBG("bg stats info %d %d %d %d %d %d num %d", stats_info->buf_len,
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
     stats_info->comp_flag = STATS_COMPOSITE_GRP_1;
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
     stats_info->comp_flag = STATS_COMPOSITE_GRP_1;
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
     stats_info->comp_flag = STATS_COMPOSITE_GRP_1;
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
     stats_info->comp_flag = STATS_COMPOSITE_GRP_1;
     stats_info->num_bufs = STATS_IHIST_BUF_NUM;
     stats_info->session_id = session_id;
     stats_info->stats_type = MSM_ISP_STATS_IHIST;
     ISP_HIGH("ihist stats info %d %d %d %d %d %d", stats_info->buf_len,
       stats_info->buf_type, stats_info->comp_flag, stats_info->num_bufs,
       stats_info->session_id, stats_info->stats_type);
  }

  /* BF SCALE STATS */
  if ((stats_mask & (1 << MSM_ISP_STATS_BF_SCALE)) &&
    (fast_aec_mode == FALSE)) {
     stats_info = &iface_resource_request->isp_stats_info
       [iface_resource_request->num_stats_stream++];
     stats_info->buf_len = STATS_BF_SCALE_BUF_SIZE * num_isp;
     if (iface_resource_request->ispif_split_output_info.is_split == TRUE) {
        stats_info->buf_type = ISP_SHARE_BUF;
     } else {
       stats_info->buf_type = ISP_PRIVATE_BUF;
     }
     stats_info->comp_flag = STATS_COMPOSITE_GRP_2;
     stats_info->num_bufs = STATS_BF_SCALE_BUF_NUM;
     stats_info->session_id = session_id;
     stats_info->stats_type = MSM_ISP_STATS_BF_SCALE;
     ISP_DBG("bf scale stats info %d %d %d %d %d %d num %d",
       stats_info->buf_len, stats_info->buf_type,
       stats_info->comp_flag, stats_info->num_bufs,
       stats_info->session_id, stats_info->stats_type,
       iface_resource_request->num_stats_stream);
  }

  for (i = 0; i < iface_resource_request->num_stats_stream; i++) {
    stats_info = &iface_resource_request->isp_stats_info[i];
    stats_info->parse_flag = 1;
  }

  return TRUE;
}

/** isp_pipeline44_parser_bf_stats_dump_parsed_stats:
 *
 *  @bf_stats: bf_stats struct that is to be printed
 *
 * Print value of current parsed bf stats
 *
 * Return void
 **/
static void isp_pipeline44_parser_bf_stats_dump_parsed_stats(
  bf_grid_stats_t *bf_stats)
{
  uint32_t i, j, idx;
  uint32_t bf_r_sum_zero_cnt = 0;
  uint32_t bf_b_sum_zero_cnt = 0;
  uint32_t bf_gr_sum_zero_cnt = 0;
  uint32_t bf_gb_sum_zero_cnt = 0;
  uint32_t bf_r_sharp_zero_cnt = 0;
  uint32_t bf_b_sharp_zero_cnt = 0;
  uint32_t bf_gr_sharp_zero_cnt = 0;
  uint32_t bf_gb_sharp_zero_cnt = 0;
  uint32_t bf_r_num_zero_cnt = 0;
  uint32_t bf_b_num_zero_cnt = 0;
  uint32_t bf_gr_num_zero_cnt = 0;
  uint32_t bf_gb_num_zero_cnt = 0;
  uint32_t bf_r_max_fv_zero_cnt = 0;
  uint32_t bf_b_max_fv_zero_cnt = 0;
  uint32_t bf_gr_max_fv_zero_cnt = 0;
  uint32_t bf_gb_max_fv_zero_cnt = 0;

  if (!bf_stats) {
    ISP_ERR("failed: bf_stats %p", bf_stats);
    return;
  }

  ISP_DBG("E - buf_ptr = %p, rgn_h_num = %d\n"
    "rgn_v_num = %d, use_max_fv = %d", bf_stats,
    bf_stats->bf_region_h_num, bf_stats->bf_region_v_num, bf_stats->use_max_fv);

  for (i = 0; i < bf_stats->bf_region_v_num; i++) {
    for (j = 0; j < bf_stats->bf_region_h_num; j++) {
      idx = i *  bf_stats->bf_region_h_num + j;
      if (idx >= MAX_BF_STATS_NUM)
        goto end;
      if (0 ==  bf_stats->bf_r_sum[idx])
        bf_r_sum_zero_cnt++;
      if (0 ==  bf_stats->bf_gr_sum[idx])
        bf_gr_sum_zero_cnt++;
      if (0 ==  bf_stats->bf_gb_sum[idx])
        bf_gb_sum_zero_cnt++;
      if (0 ==  bf_stats->bf_r_sharp[idx])
        bf_r_sharp_zero_cnt++;
      if (0 ==  bf_stats->bf_b_sharp[idx])
        bf_b_sharp_zero_cnt++;
      if (0 ==  bf_stats->bf_gr_sharp[idx])
        bf_gr_sharp_zero_cnt++;
      if (0 ==  bf_stats->bf_gb_sharp[idx])
        bf_gb_sharp_zero_cnt++;
      if (0 ==  bf_stats->bf_b_num[idx])
        bf_b_num_zero_cnt++;
      if (0 ==  bf_stats->bf_gr_num[idx])
        bf_gr_num_zero_cnt++;
      if (0 ==  bf_stats->bf_gb_num[idx])
        bf_gb_num_zero_cnt++;
      if (0 ==  bf_stats->bf_r_max_fv[idx])
        bf_r_max_fv_zero_cnt++;
      if (0 ==  bf_stats->bf_b_max_fv[idx])
        bf_b_max_fv_zero_cnt++;
      if (0 ==  bf_stats->bf_gr_max_fv[idx])
        bf_gr_max_fv_zero_cnt++;
      if (0 ==  bf_stats->bf_gb_max_fv[idx])
        bf_gb_max_fv_zero_cnt++;
    }
  }

end:
  ISP_DBG("X bf_stats_dump: rgn_h_num = %d rgn_v_num = %d\n"
    "use_max_fv = %d\n"
    "r_sum_zero_cnt     = %d\n"
    "gr_sum_zero_cnt    = %d\n"
    "gb_sum_zero_cnt   = %d\n"
    "r_sharp_zero_cnt   = %d\n"
    "b_sharp_zero_cnt   = %d\n"
    "gr_sharp_zero_cntt = %d\n"
    "gb_sharp_zero_cnt = %d\n"
    "b_num_zero_cnt    = %d\n"
    "gr_num_zero_cnt    = %d\n"
    "gb_num_zero_cnt    = %d\n"
    "r_max_fv_zero_cnt = %d\n"
    "b_max_fv_zero_cnt = %d\n"
    "gr_max_fv_zero_cnt = %d\n"
    "gb_max_fv_zero_cnt = %d\n",
    bf_stats->bf_region_h_num,
    bf_stats->bf_region_v_num,
    bf_stats->use_max_fv,
    bf_r_sum_zero_cnt,
    bf_gr_sum_zero_cnt,
    bf_gb_sum_zero_cnt,
    bf_r_sharp_zero_cnt,
    bf_b_sharp_zero_cnt,
    bf_gr_sharp_zero_cnt,
    bf_gb_sharp_zero_cnt,
    bf_b_num_zero_cnt,
    bf_gr_num_zero_cnt,
    bf_gb_num_zero_cnt,
    bf_r_max_fv_zero_cnt,
    bf_b_max_fv_zero_cnt,
    bf_gr_max_fv_zero_cnt,
    bf_gb_max_fv_zero_cnt);
  return;
}

/** isp_pipeline44_parser_bf_stats:
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
boolean isp_pipeline44_parser_bf_stats(void *input_buf,
  mct_event_stats_isp_t *output,
  isp_saved_stats_params_t *saved_stats_params,
  isp_parser_session_params_t *parser_session_params)
{
  int                         window = 0;
  uint32_t                    i = 0, j = 0;
  uint32_t                   *Sr = NULL,*Sb = NULL, *Sgr = NULL, *Sgb = NULL;
  uint64_t                   *r_sh = NULL, *b_sh = NULL, *gr_sh = NULL,
                             *gb_sh = NULL;
  uint32_t                   *r_num = NULL, *b_num = NULL, *gr_num = NULL,
                             *gb_num = NULL;
  uint64_t                   *r_fv_max = NULL, *b_fv_max = NULL,
                             *gr_fv_max = NULL, *gb_fv_max = NULL;
  uint32_t                   *current_region = NULL;
  uint32_t                    h_rgns_start = 0;
  uint32_t                    h_rgns_end = 0;
  uint32_t                    h_rgns_total = 0;
  uint32_t                    v_rgns_total = 0;
  q3a_bf_stats_t             *bf_stats = NULL;
  isp_rgns_stats_param_t     *rgns_stats = NULL;
  mct_event_stats_isp_data_t *stats_data = NULL;
  bf_grid_stats_t            *grid_stats = NULL;

  if (!input_buf || !output || !saved_stats_params ||
    !parser_session_params) {
    ISP_ERR("failed: %p %p %p %p", input_buf, output,
      saved_stats_params, parser_session_params);
    return FALSE;
  }

  rgns_stats = &saved_stats_params->rgns_stats[MSM_ISP_STATS_BF];
  ISP_DBG("bf rgns_stats %d %d %d %d %d", rgns_stats->is_valid,
    rgns_stats->h_rgns_start, rgns_stats->h_rgns_end,
    rgns_stats->h_rgns_total, rgns_stats->v_rgns_total);

  if (rgns_stats->is_valid == FALSE) {
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
  ISP_DBG("bf stats_mask %x type %d buf %p size %d",
    output->stats_mask, stats_data->stats_type, stats_data->stats_buf,
    stats_data->buf_size);

  h_rgns_start = rgns_stats->h_rgns_start;
  h_rgns_end = rgns_stats->h_rgns_end;
  h_rgns_total = rgns_stats->h_rgns_total;
  v_rgns_total = rgns_stats->v_rgns_total;

  bf_stats->mask = MCT_EVENT_STATS_BF_SINGLE;
  grid_stats = &bf_stats->u.grid_stats;
  Sr     = grid_stats->bf_r_sum;
  Sb     = grid_stats->bf_b_sum;
  Sgr    = grid_stats->bf_gr_sum;
  Sgb    = grid_stats->bf_gb_sum;
  r_sh   = grid_stats->bf_r_sharp;
  b_sh   = grid_stats->bf_b_sharp;
  gr_sh  = grid_stats->bf_gr_sharp;
  gb_sh  = grid_stats->bf_gb_sharp;
  r_num  = grid_stats->bf_r_num;
  b_num  = grid_stats->bf_b_num;
  gr_num = grid_stats->bf_gr_num;
  gb_num = grid_stats->bf_gb_num;
  r_fv_max = grid_stats->bf_r_max_fv;
  b_fv_max = grid_stats->bf_b_max_fv;
  gr_fv_max = grid_stats->bf_gr_max_fv;
  gb_fv_max = grid_stats->bf_gb_max_fv;

  current_region = (uint32_t *)input_buf;
  bf_stats->config_id =
    saved_stats_params->stats_config.af_config.config_id;
  grid_stats->use_max_fv = 1;
  grid_stats->bf_region_h_num += (h_rgns_end - h_rgns_start + 1);
  grid_stats->bf_region_v_num = v_rgns_total;

  for (j = 0; j < v_rgns_total; j++) {
    for (i = 0; i < h_rgns_total; i++) {
      if (i >= h_rgns_start && i <= h_rgns_end) {
        /*parse AF stats */
        *Sr = ((*(current_region)) & 0x00FFFFFF);
        current_region++;
        *Sb = ((*(current_region)) & 0x00FFFFFF);
        current_region++;
        *Sgr = ((*(current_region)) & 0x00FFFFFF);
        current_region++;
        *Sgb = ((*(current_region)) & 0x00FFFFFF);
        current_region++;
        *r_sh = *current_region;
        current_region++;
        *b_sh = *current_region;
        current_region++;
        *gr_sh = *current_region;
        current_region++;
        *gb_sh = *current_region;
        current_region++;
        *r_num = ((*(current_region)) & 0x0000FFFF);
        *b_num = ((*(current_region)) & 0xFFFF0000) >> 16;
        current_region++;
        *gr_num = ((*(current_region)) & 0x0000FFFF);
        *gb_num = ((*(current_region)) & 0xFFFF0000) >> 16;
        current_region++;
        *r_fv_max = ((*current_region) & 0x00FFFFFF);
        current_region++;
        *b_fv_max = ((*current_region) & 0x00FFFFFF);
        current_region++;
        *gr_fv_max = ((*current_region) & 0x00FFFFFF);
        current_region++;
        *gb_fv_max = ((*current_region) & 0x00FFFFFF);
        current_region++;
      }
      Sr++;
      Sb++;
      Sgr++;
      Sgb++;
      r_sh++;
      b_sh++;
      gr_sh++;
      gb_sh++;
      r_num++;
      b_num++;
      gr_num++;
      gb_num++;
      r_fv_max++;
      b_fv_max++;
      gr_fv_max++;
      gb_fv_max++;
    }
  }

  isp_pipeline44_parser_bf_stats_dump_parsed_stats(&bf_stats->u.grid_stats);
  return TRUE;
}

/** isp_pipeline44_parser_bg_stats:
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
boolean isp_pipeline44_parser_bg_stats(void *input_buf,
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

  bg_stats->ex_fields = BG_EX_FIELDS_NONE;
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

/** isp_pipeline44_parser_be_stats:
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
boolean isp_pipeline44_parser_be_stats(void *input_buf,
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


/** isp_pipeline44_parser_rs_stats:
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
boolean isp_pipeline44_parser_rs_stats(void *input_buf,
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

/** isp_pipeline44_parser_cs_stats:
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
boolean isp_pipeline44_parser_cs_stats(void *input_buf,
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

/** isp_pipeline44_parser_ihist_stats:
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
boolean isp_pipeline44_parser_ihist_stats(void *input_buf,
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

/** isp_pipeline44_parser_bhist_stats:
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
boolean isp_pipeline44_parser_bhist_stats(void *input_buf,
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

/** isp_pipeline44_parser_bf_scale_stats:
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
boolean isp_pipeline44_parser_bf_scale_stats(void *input_buf,
  mct_event_stats_isp_t *output,
  isp_saved_stats_params_t *saved_stats_params,
  isp_parser_session_params_t *parser_session_params)
{
  int                         window = 0;
  uint32_t                    i = 0, j = 0;
  uint32_t                   *Sr = NULL,*Sb = NULL, *Sgr = NULL, *Sgb = NULL;
  uint64_t                   *r_sh = NULL, *b_sh = NULL, *gr_sh = NULL,
                             *gb_sh = NULL;
  uint32_t                   *r_num = NULL, *b_num = NULL, *gr_num = NULL,
                             *gb_num = NULL;
  uint64_t                   *r_fv_max = NULL, *b_fv_max = NULL,
                             *gr_fv_max = NULL, *gb_fv_max = NULL;
  uint32_t                   *current_region = NULL;
  uint32_t                    h_rgns_start = 0;
  uint32_t                    h_rgns_end = 0;
  uint32_t                    h_rgns_total = 0;
  uint32_t                    v_rgns_total = 0;
  q3a_bf_stats_t             *bf_scale_stats = NULL;
  isp_rgns_stats_param_t     *rgns_stats = NULL;
  mct_event_stats_isp_data_t *stats_data = NULL;
  bf_grid_stats_t            *grid_stats = NULL;

  if (!input_buf || !output || !saved_stats_params ||
    !parser_session_params) {
    ISP_ERR("failed: %p %p %p %p", input_buf, output,
      saved_stats_params, parser_session_params);
    return FALSE;
  }

  rgns_stats = &saved_stats_params->rgns_stats[MSM_ISP_STATS_BF_SCALE];
  ISP_DBG("bf scale rgns_stats %d %d %d %d %d", rgns_stats->is_valid,
    rgns_stats->h_rgns_start, rgns_stats->h_rgns_end,
    rgns_stats->h_rgns_total, rgns_stats->v_rgns_total);

  if (rgns_stats->is_valid == FALSE) {
    ISP_ERR("failed: is_valid FALSE");
    return FALSE;
  }

  output->stats_mask |= (1 << MSM_ISP_STATS_BF_SCALE);
  stats_data = &output->stats_data[MSM_ISP_STATS_BF_SCALE];
  if (stats_data->buf_size != sizeof(q3a_bf_stats_t)) {
    ISP_ERR("failed: invalid buf_size %d expected %d", stats_data->buf_size,
      sizeof(q3a_bf_stats_t));
    return FALSE;
  }

  bf_scale_stats = (q3a_bf_stats_t *)stats_data->stats_buf;
  if (!bf_scale_stats) {
    ISP_ERR("failed: bf_scale_stats NULL");
    return FALSE;
  }

  stats_data->stats_type = MSM_ISP_STATS_BF_SCALE;
  stats_data->used_size = sizeof(*bf_scale_stats);
  ISP_DBG("bf scale stats_mask %x type %d buf %p size %d",
    output->stats_mask, stats_data->stats_type, stats_data->stats_buf,
    stats_data->buf_size);

  h_rgns_start = rgns_stats->h_rgns_start;
  h_rgns_end = rgns_stats->h_rgns_end;
  h_rgns_total = rgns_stats->h_rgns_total;
  v_rgns_total = rgns_stats->v_rgns_total;

  bf_scale_stats->mask = MCT_EVENT_STATS_BF_SCALE;
  grid_stats = &bf_scale_stats->u.grid_stats;
  Sr     = grid_stats->bf_r_sum;
  Sb     = grid_stats->bf_b_sum;
  Sgr    = grid_stats->bf_gr_sum;
  Sgb    = grid_stats->bf_gb_sum;
  r_sh   = grid_stats->bf_r_sharp;
  b_sh   = grid_stats->bf_b_sharp;
  gr_sh  = grid_stats->bf_gr_sharp;
  gb_sh  = grid_stats->bf_gb_sharp;
  r_num  = grid_stats->bf_r_num;
  b_num  = grid_stats->bf_b_num;
  gr_num = grid_stats->bf_gr_num;
  gb_num = grid_stats->bf_gb_num;
  r_fv_max = grid_stats->bf_r_max_fv;
  b_fv_max = grid_stats->bf_b_max_fv;
  gr_fv_max = grid_stats->bf_gr_max_fv;
  gb_fv_max = grid_stats->bf_gb_max_fv;

  current_region = (uint32_t *)input_buf;
  grid_stats->use_max_fv = 1;
  grid_stats->bf_region_h_num += (h_rgns_end - h_rgns_start + 1);
  grid_stats->bf_region_v_num = v_rgns_total;

  for (j = 0; j < v_rgns_total; j++) {
    for (i = 0; i < h_rgns_total; i++) {
      if (i >= h_rgns_start && i <= h_rgns_end) {
        /*parse AF stats */
        *Sr = ((*(current_region)) & 0x00FFFFFF);
        current_region++;
        *Sb = ((*(current_region)) & 0x00FFFFFF);
        current_region++;
        *Sgr = ((*(current_region)) & 0x00FFFFFF);
        current_region++;
        *Sgb = ((*(current_region)) & 0x00FFFFFF);
        current_region++;
        *r_sh = *current_region;
        current_region++;
        *b_sh = *current_region;
        current_region++;
        *gr_sh = *current_region;
        current_region++;
        *gb_sh = *current_region;
        current_region++;
        *r_num = ((*(current_region)) & 0x0000FFFF);
        *b_num = ((*(current_region)) & 0xFFFF0000) >> 16;
        current_region++;
        *gr_num = ((*(current_region)) & 0x0000FFFF);
        *gb_num = ((*(current_region)) & 0xFFFF0000) >> 16;
        current_region++;
        *r_fv_max = ((*current_region) & 0x00FFFFFF);
        current_region++;
        *b_fv_max = ((*current_region) & 0x00FFFFFF);
        current_region++;
        *gr_fv_max = ((*current_region) & 0x00FFFFFF);
        current_region++;
        *gb_fv_max = ((*current_region) & 0x00FFFFFF);
        current_region++;
      }
      Sr++;
      Sb++;
      Sgr++;
      Sgb++;
      r_sh++;
      b_sh++;
      gr_sh++;
      gb_sh++;
      r_num++;
      b_num++;
      gr_num++;
      gb_num++;
      r_fv_max++;
      b_fv_max++;
      gr_fv_max++;
      gb_fv_max++;
    }
  }

  isp_pipeline44_parser_bf_stats_dump_parsed_stats(
    &bf_scale_stats->u.grid_stats);
  return TRUE;
}
