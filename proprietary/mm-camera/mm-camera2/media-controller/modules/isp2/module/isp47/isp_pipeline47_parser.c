/* isp_pipeline47_parser.c
 *
 * Copyright (c) 2014-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* mctl headers */
#include "isp_module.h"
#include "q3a_stats_hw.h"

/* isp headers */
#include "isp_pipeline47_parser.h"
#include "isp_log.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_COMMON, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_COMMON, fmt, ##args)

#define ALIGN32(x) ((x + 31) / 32 ) * 32

#define STATS_BG_BUF_NUM  4
#define STATS_BG_MAX_RGN_HNUM 160
#define STATS_BG_MAX_RGN_VNUM 90
#define STATS_BG_NUM_ENTRIES 12
#define STATS_BG_SIZE_PER_ENTRY (sizeof(uint32_t))
#define STATS_BG_BUF_SIZE (STATS_BG_MAX_RGN_HNUM * STATS_BG_MAX_RGN_VNUM * STATS_BG_NUM_ENTRIES * STATS_BG_SIZE_PER_ENTRY)

#define STATS_AEC_BG_BUF_NUM  4
#define STATS_AEC_BG_MAX_RGN_HNUM 160
#define STATS_AEC_BG_MAX_RGN_VNUM 90
#define STATS_AEC_BG_NUM_ENTRIES 12
#define STATS_AEC_BG_SIZE_PER_ENTRY (sizeof(uint32_t))
#define STATS_AEC_BG_BUF_SIZE (STATS_AEC_BG_MAX_RGN_HNUM * STATS_AEC_BG_MAX_RGN_VNUM * STATS_AEC_BG_NUM_ENTRIES * STATS_AEC_BG_SIZE_PER_ENTRY)

#define STATS_BE_BUF_NUM  4
#define STATS_BE_MAX_RGN_HNUM 32
#define STATS_BE_MAX_RGN_VNUM 24
#define STATS_BE_NUM_ENTRIES 6
#define STATS_BE_SIZE_PER_ENTRY (sizeof(uint32_t))
#define STATS_BE_BUF_SIZE (STATS_BE_MAX_RGN_HNUM * STATS_BE_MAX_RGN_VNUM * STATS_BE_NUM_ENTRIES * STATS_BE_SIZE_PER_ENTRY)

/* 4 buffer for early intr and 4 for normal intr */
#define STATS_BF_BUF_NUM  8
#define STATS_BF_MAX_RGN 281
#define STATS_BF_NUM_ENTRIES 6
#define STATS_BF_SIZE_PER_ENTRY (sizeof(uint64_t))
#define STATS_BF_BUF_SIZE (STATS_BF_MAX_RGN * STATS_BF_NUM_ENTRIES * STATS_BF_SIZE_PER_ENTRY)

#define STATS_RS_BUF_NUM  4
#define STATS_RS_MAX_VNUM 1024
#define STATS_RS_MAX_HNUM 16
#define STATS_RS_SIZE_PER_ENTRY (sizeof(uint16_t))
#define STATS_RS_BUF_SIZE (STATS_RS_MAX_HNUM *  STATS_RS_MAX_VNUM * STATS_RS_SIZE_PER_ENTRY)

#define STATS_CS_BUF_NUM  4
#define STATS_CS_MAX_VNUM 4
#define STATS_CS_MAX_HNUM 1560
#define STATS_CS_SIZE_PER_ENTRY (sizeof(uint16_t))
#define STATS_CS_BUF_SIZE (STATS_CS_MAX_HNUM *  STATS_CS_MAX_VNUM * STATS_CS_SIZE_PER_ENTRY)

#define STATS_IHIST_BUF_NUM    4
#define STATS_IHIST_MAX_NUM 256
#define STATS_IHIST_SIZE_PER_ENTRY (sizeof(uint16_t))
#define STATS_IHIST_BUF_SIZE (STATS_IHIST_MAX_NUM * STATS_IHIST_SIZE_PER_ENTRY)

#define STATS_BHIST_STATS_BUF_NUM  4
#define STATS_BHIST_BIN_SIZE 4096
#define STATS_BHIST_BIN_NUM 4
#define STATS_BHIST_SIZE_PER_ENTRY (sizeof(uint32_t))
#define STATS_BHIST_BUF_SIZE (STATS_BHIST_BIN_SIZE * STATS_BHIST_BIN_NUM * STATS_BHIST_SIZE_PER_ENTRY)

#define STATS_HDR_BE_BUF_NUM  4
#define STATS_HDR_BE_MAX_RGN_HNUM 160
#define STATS_HDR_BE_MAX_RGN_VNUM 90
#define STATS_HDR_BE_NUM_ENTRIES 12
#define STATS_HDR_BE_SIZE_PER_ENTRY (sizeof(uint32_t))
#define STATS_HDR_BE_BUF_SIZE (STATS_HDR_BE_MAX_RGN_HNUM * STATS_HDR_BE_MAX_RGN_VNUM * STATS_HDR_BE_NUM_ENTRIES * STATS_HDR_BE_SIZE_PER_ENTRY)

#define STATS_HDR_BHIST_BUF_NUM  4
#define STATS_HDR_BHIST_BIN_SIZE 1024
#define STATS_HDR_BHIST_BIN_NUM 4
#define STATS_HDR_BHIST_SIZE_PER_ENTRY (sizeof(uint32_t))
#define STATS_HDR_BHIST_BUF_SIZE (STATS_BHIST_BIN_SIZE * STATS_BHIST_BIN_NUM * STATS_BHIST_SIZE_PER_ENTRY)

#define ALL_ONES_32(x) (x ? (((uint32_t)~0) >> (32-x)) : 0)

#define ALL_ONES_32_LEFTSHIFT(x,y) (ALL_ONES_32(x) << y)

/* #define PIPELINE47_DEBUG */
#ifdef PIPELINE47_DEBUG
#undef ISP_DBG
#define ISP_DBG ISP_ERR
#undef ISP_HIGH
#define ISP_HIGH ISP_ERR
#endif

/** isp_pipeline47_fill_stats_info:
 *
 *  @session_id: session id
 *  @iface_resource_request: handle to iface_resource_request_t
 *
 *  Fill stats info to request for specific stats
 **/
boolean isp_pipeline47_parser_fill_stats_info(uint32_t session_id,
  iface_resource_request_t *iface_resource_request,
  isp_stats_mask_ctrl_t *stats_ctrl, boolean fast_aec_mode, uint32_t num_isp,
  isp_parser_session_params_t *parser_session_params __unused)
{
  isp_stats_info_t *stats_info = NULL;
  uint32_t          stats_mask = 0;
  uint32_t          parse_mask = 0;
  uint32_t          i = 0;

  if (!iface_resource_request || !stats_ctrl) {
    ISP_ERR("failed: iface_resource_request %p stats_control: %p",
      iface_resource_request, stats_ctrl);
    return FALSE;
  }

  stats_mask = stats_ctrl->stats_mask;
  parse_mask = stats_ctrl->parse_mask;

  iface_resource_request->num_stats_stream = 0;
  if (num_isp < 1 || num_isp > 2) {
    ISP_ERR("failed : num_isp is wrong %d", num_isp);
    return FALSE;
  }

  /* BF STATS */
  if ((stats_mask & (1 << MSM_ISP_STATS_BF)) && (fast_aec_mode == FALSE)) {
    stats_info = &iface_resource_request->isp_stats_info
      [iface_resource_request->num_stats_stream++];
    stats_info->buf_len = ALIGN32(STATS_BF_BUF_SIZE) * num_isp;
    if (iface_resource_request->ispif_split_output_info.is_split == TRUE) {
      stats_info->buf_type = ISP_SHARE_BUF;
    } else {
      stats_info->buf_type = ISP_PRIVATE_BUF;
    }
    for (i = 0; i < stats_ctrl->num_stats_comp_grp; i++) {
      if (stats_ctrl->stats_comp_grp_mask[i] & (1 << MSM_ISP_STATS_BF)) {
        stats_info->comp_flag = i + 1;
       /*When STATS Composite group is decided break out from loop*/
        break;
      }
    }
    stats_info->parse_flag =
      (parse_mask & (1 << MSM_ISP_STATS_BF))? 1: 0;
    /*3A reports wrong STATS Group then override to default one*/
    if (stats_info->comp_flag == STATS_COMPOSITE_GRP_NONE)
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
    stats_info->buf_len = ALIGN32(STATS_BG_BUF_SIZE) * num_isp;
    if (iface_resource_request->ispif_split_output_info.is_split == TRUE) {
      stats_info->buf_type = ISP_SHARE_BUF;
    } else {
      stats_info->buf_type = ISP_PRIVATE_BUF;
    }
    for (i = 0; i < stats_ctrl->num_stats_comp_grp; i++) {
      if (stats_ctrl->stats_comp_grp_mask[i] & (1 << MSM_ISP_STATS_BG)) {
        stats_info->comp_flag = i + 1;
        /*When STATS Composite group is decided break out from loop*/
        break;
      }
    }
    stats_info->parse_flag =
      (parse_mask & (1 << MSM_ISP_STATS_BG))? 1: 0;
    /*3A reports wrong STATS Group then override to default one*/
    if (stats_info->comp_flag == STATS_COMPOSITE_GRP_NONE)
      stats_info->comp_flag = STATS_COMPOSITE_GRP_1;

    stats_info->num_bufs = STATS_BG_BUF_NUM;
    stats_info->session_id = session_id;
    stats_info->stats_type = MSM_ISP_STATS_BG;
    ISP_DBG("bg stats info %d %d %d %d %d %d num %d", stats_info->buf_len,
      stats_info->buf_type, stats_info->comp_flag, stats_info->num_bufs,
      stats_info->session_id, stats_info->stats_type,
      iface_resource_request->num_stats_stream);
  }

  /* AEC BG STATS */
  if (stats_mask & (1 << MSM_ISP_STATS_AEC_BG)) {
    stats_info = &iface_resource_request->isp_stats_info
      [iface_resource_request->num_stats_stream++];
    stats_info->buf_len = ALIGN32(STATS_AEC_BG_BUF_SIZE) * num_isp;
    if (iface_resource_request->ispif_split_output_info.is_split == TRUE) {
      stats_info->buf_type = ISP_SHARE_BUF;
    } else {
      stats_info->buf_type = ISP_PRIVATE_BUF;
    }
    for (i = 0; i < stats_ctrl->num_stats_comp_grp; i++) {
      if (stats_ctrl->stats_comp_grp_mask[i] & (1 << MSM_ISP_STATS_AEC_BG)) {
        stats_info->comp_flag = i + 1;
        /*When STATS Composite group is decided break out from loop*/
        break;
      }
    }
    stats_info->parse_flag =
      (parse_mask & (1 << MSM_ISP_STATS_AEC_BG))? 1: 0;
    /*3A reports wrong STATS Group then override to default one*/
    if (stats_info->comp_flag == STATS_COMPOSITE_GRP_NONE)
      stats_info->comp_flag = STATS_COMPOSITE_GRP_1;

    stats_info->num_bufs = STATS_AEC_BG_BUF_NUM;
    stats_info->session_id = session_id;
    stats_info->stats_type = MSM_ISP_STATS_AEC_BG;
    ISP_DBG("aec bg stats info %d %d %d %d %d %d num %d", stats_info->buf_len,
      stats_info->buf_type, stats_info->comp_flag, stats_info->num_bufs,
      stats_info->session_id, stats_info->stats_type,
      iface_resource_request->num_stats_stream);
  }

  /* RS STATS */
  if (stats_mask & (1 << MSM_ISP_STATS_RS)) {
    stats_info = &iface_resource_request->isp_stats_info
      [iface_resource_request->num_stats_stream++];
    stats_info->buf_len = ALIGN32(STATS_RS_BUF_SIZE) * num_isp;
    if (iface_resource_request->ispif_split_output_info.is_split == TRUE) {
      stats_info->buf_type = ISP_SHARE_BUF;
    } else {
      stats_info->buf_type = ISP_PRIVATE_BUF;
    }
    for (i = 0; i < stats_ctrl->num_stats_comp_grp; i++) {
      if (stats_ctrl->stats_comp_grp_mask[i] & (1 << MSM_ISP_STATS_RS)) {
        stats_info->comp_flag = i + 1;
        /*When STATS Composite group is decided break out from loop*/
        break;
      }
    }
    stats_info->parse_flag =
      (parse_mask & (1 << MSM_ISP_STATS_RS))? 1: 0;
    /*3A reports wrong STATS Group then override to default one*/
    if (stats_info->comp_flag == STATS_COMPOSITE_GRP_NONE)
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
    stats_info->buf_len = ALIGN32(STATS_CS_BUF_SIZE) * num_isp;
    if (iface_resource_request->ispif_split_output_info.is_split == TRUE) {
      stats_info->buf_type = ISP_SHARE_BUF;
    } else {
      stats_info->buf_type = ISP_PRIVATE_BUF;
    }
    for (i = 0; i < stats_ctrl->num_stats_comp_grp; i++) {
      if (stats_ctrl->stats_comp_grp_mask[i] & (1 << MSM_ISP_STATS_CS)) {
        stats_info->comp_flag = i + 1;
        /*When STATS Composite group is decided break out from loop*/
        break;
      }
    }
    stats_info->parse_flag =
      (parse_mask & (1 << MSM_ISP_STATS_CS))? 1: 0;
    /*3A reports wrong STATS Group then override to default one*/
    if (stats_info->comp_flag == STATS_COMPOSITE_GRP_NONE)
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
    stats_info->buf_len = ALIGN32(STATS_BHIST_BUF_SIZE) * num_isp;
    if (iface_resource_request->ispif_split_output_info.is_split == TRUE) {
      stats_info->buf_type = ISP_SHARE_BUF;
    } else {
      stats_info->buf_type = ISP_PRIVATE_BUF;
    }
    for (i = 0; i < stats_ctrl->num_stats_comp_grp; i++) {
      if (stats_ctrl->stats_comp_grp_mask[i] & (1 << MSM_ISP_STATS_BHIST)) {
        stats_info->comp_flag = i + 1;
        /*When STATS Composite group is decided break out from loop*/
        break;
      }
    }
    stats_info->parse_flag =
      (parse_mask & (1 << MSM_ISP_STATS_BHIST))? 1: 0;
    /*3A reports wrong STATS Group then override to default one*/
    if (stats_info->comp_flag == STATS_COMPOSITE_GRP_NONE)
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
    stats_info->buf_len = ALIGN32(STATS_IHIST_BUF_SIZE) * num_isp;
    if (iface_resource_request->ispif_split_output_info.is_split == TRUE) {
      stats_info->buf_type = ISP_SHARE_BUF;
    } else {
      stats_info->buf_type = ISP_PRIVATE_BUF;
    }
    for (i = 0; i < stats_ctrl->num_stats_comp_grp; i++) {
      if (stats_ctrl->stats_comp_grp_mask[i] & (1 << MSM_ISP_STATS_IHIST)) {
        stats_info->comp_flag = i + 1;
        /*When STATS Composite group is decided break out from loop*/
        break;
      }
    }
    stats_info->parse_flag =
      (parse_mask & (1 << MSM_ISP_STATS_IHIST))? 1: 0;
    /*3A reports wrong STATS Group then override to default one*/
    if (stats_info->comp_flag == STATS_COMPOSITE_GRP_NONE)
      stats_info->comp_flag = STATS_COMPOSITE_GRP_1;

    stats_info->num_bufs = STATS_IHIST_BUF_NUM;
    stats_info->session_id = session_id;
    stats_info->stats_type = MSM_ISP_STATS_IHIST;
    ISP_DBG("ihist stats info %d %d %d %d %d %d", stats_info->buf_len,
      stats_info->buf_type, stats_info->comp_flag, stats_info->num_bufs,
      stats_info->session_id, stats_info->stats_type);
  }

   /* HDR_BE STATS */
  if (stats_mask & (1 << MSM_ISP_STATS_HDR_BE)) {
    stats_info = &iface_resource_request->isp_stats_info
      [iface_resource_request->num_stats_stream++];
    stats_info->buf_len = ALIGN32(STATS_HDR_BE_BUF_SIZE) * num_isp;
    if (iface_resource_request->ispif_split_output_info.is_split == TRUE) {
      stats_info->buf_type = ISP_SHARE_BUF;
    } else {
      stats_info->buf_type = ISP_PRIVATE_BUF;
    }
    for (i = 0; i < stats_ctrl->num_stats_comp_grp; i++) {
      if (stats_ctrl->stats_comp_grp_mask[i] & (1 << MSM_ISP_STATS_HDR_BE)) {
        stats_info->comp_flag = i + 1;
        /*When STATS Composite group is decided break out from loop*/
        break;
      }
    }
    stats_info->parse_flag =
      (parse_mask & (1 << MSM_ISP_STATS_HDR_BE))? 1: 0;
    /*3A reports wrong STATS Group then override to default one*/
    if (stats_info->comp_flag == STATS_COMPOSITE_GRP_NONE)
      stats_info->comp_flag = STATS_COMPOSITE_GRP_1;

    stats_info->num_bufs = STATS_HDR_BE_BUF_NUM;
    stats_info->session_id = session_id;
    stats_info->stats_type = MSM_ISP_STATS_HDR_BE;
    ISP_DBG("bf scale stats info %d %d %d %d %d %d num %d", stats_info->buf_len,
      stats_info->buf_type, stats_info->comp_flag, stats_info->num_bufs,
      stats_info->session_id, stats_info->stats_type,
      iface_resource_request->num_stats_stream);
  }

   /* HDR_BHIST STATS */
  if (stats_mask & (1 << MSM_ISP_STATS_HDR_BHIST)) {
    stats_info = &iface_resource_request->isp_stats_info
      [iface_resource_request->num_stats_stream++];
    stats_info->buf_len = ALIGN32(STATS_HDR_BHIST_BUF_SIZE) * num_isp;
    if (iface_resource_request->ispif_split_output_info.is_split == TRUE) {
      stats_info->buf_type = ISP_SHARE_BUF;
    } else {
      stats_info->buf_type = ISP_PRIVATE_BUF;
    }
    for (i = 0; i < stats_ctrl->num_stats_comp_grp; i++) {
      if (stats_ctrl->stats_comp_grp_mask[i] & (1 << MSM_ISP_STATS_HDR_BHIST)) {
        stats_info->comp_flag = i + 1;
        /*When STATS Composite group is decided break out from loop*/
        break;
      }
    }
    stats_info->parse_flag =
      (parse_mask & (1 << MSM_ISP_STATS_HDR_BHIST))? 1: 0;
    /*3A reports wrong STATS Group then override to default one*/
    if (stats_info->comp_flag == STATS_COMPOSITE_GRP_NONE)
      stats_info->comp_flag = STATS_COMPOSITE_GRP_1;

    stats_info->num_bufs = STATS_HDR_BHIST_BUF_NUM;
    stats_info->session_id = session_id;
    stats_info->stats_type = MSM_ISP_STATS_HDR_BHIST;
    ISP_DBG("bf scale stats info %d %d %d %d %d %d num %d", stats_info->buf_len,
      stats_info->buf_type, stats_info->comp_flag, stats_info->num_bufs,
      stats_info->session_id, stats_info->stats_type,
      iface_resource_request->num_stats_stream);
  }

  return TRUE;
}

/** isp_pipeline41_parser_bf_stats:
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
boolean isp_pipeline47_parser_bf_stats(void *input_buf,
  mct_event_stats_isp_t *output,
  isp_saved_stats_params_t *saved_stats_params,
  isp_parser_session_params_t *parser_session_params)
{
  int                         window = 0;
  uint32_t                    i = 0;
  uint64_t                   *H1_Sh = NULL,
                             *H2_Sh = NULL,
                             *V_Sh = NULL,
                             *H1_Sum = NULL,
                             *H2_Sum = NULL,
                             *V_Sum = NULL;
  uint32_t                   *H1_Cnt = NULL,
                             *H2_Cnt = NULL,
                             *V_Cnt = NULL,
                              num_roi = 0;
  boolean                     Sel = FALSE;
  uint32_t                    region_num = 0;
  uint64_t                   *current_region = NULL;
  q3a_bf_stats_t             *bf_stats = NULL;
  mct_event_stats_isp_data_t *stats_data = NULL;
  bf_fw_stats_t              *fw_stats = NULL;
  bf_fw_roi_cfg_t            *bf_fw_roi_cfg = NULL;

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
  ISP_DBG("bf stats_mask %x type %d buf %p size %d",
    output->stats_mask, stats_data->stats_type, stats_data->stats_buf,
    stats_data->buf_size);

  bf_stats->mask = MCT_EVENT_STATS_BF_FW;
  bf_stats->config_id =
    saved_stats_params->stats_config.af_config.config_id;
  fw_stats = &bf_stats->u.fw_stats;
  num_roi = bf_fw_roi_cfg->num_bf_fw_roi_dim;
  if (num_roi > BF_FW_MAX_ROI_REGIONS) {
    ISP_ERR("num_roi %d is greater than max roi regions", num_roi);
    return FALSE;
  }

  H1_Cnt     = fw_stats->bf_h1_num;
  H1_Sum     = fw_stats->bf_h1_sum;
  H1_Sh      = fw_stats->bf_h1_sharp;
  H2_Cnt     = fw_stats->bf_h2_num;
  H2_Sum     = fw_stats->bf_h2_sum;
  H2_Sh      = fw_stats->bf_h2_sharp;
  V_Cnt      = fw_stats->bf_v_num;
  V_Sum      = fw_stats->bf_v_sum;
  V_Sh       = fw_stats->bf_v_sharp;

  current_region = (uint64_t *)input_buf;

  for (i = 0; i < bf_fw_roi_cfg->num_bf_fw_roi_dim &&
        bf_fw_roi_cfg->bf_fw_roi_dim[i].is_valid == TRUE; i++) {

    region_num = bf_fw_roi_cfg->bf_fw_roi_dim[i].region_num;
    if (region_num >= BF_FW_MAX_ROI_REGIONS) {
      ISP_ERR("region number > BF_FW_MAX_ROI_REGIONS");
      return FALSE;
    }
    Sel    = ((*(current_region)) & 0x8000000000000000) >> 63;
    H1_Cnt[region_num] += ((*(current_region)) & 0x3FFFFF8000000000) >> 39;
    H1_Sum[region_num] += ((*(current_region)) & 0x1FFFFFFFFF);
    current_region++;
    H1_Sh[region_num]  += ((*(current_region)) & 0xFFFFFFFFFF);
    current_region++;
    H2_Cnt[region_num] += ((*(current_region)) & 0x3FFFFF8000000000) >> 39;
    H2_Sum[region_num] += ((*(current_region)) & 0x1FFFFFFFFF);
    current_region++;
    H2_Sh[region_num]  += ((*(current_region)) & 0xFFFFFFFFFF);
    current_region++;
    V_Cnt[region_num] += ((*(current_region)) & 0x3FFFFF8000000000) >> 39;
    V_Sum[region_num] += ((*(current_region)) & 0x1FFFFFFFFF);
    current_region++;
    V_Sh[region_num]  += ((*(current_region)) & 0xFFFFFFFFFF);
    current_region++;
    ISP_DBG("region %d H1_Cnt %u H2_Cnt %u region_num %d", Sel, H1_Cnt[region_num],
      H2_Cnt[region_num], region_num);
  }

  fw_stats->bf_fw_roi_cfg =
    saved_stats_params->bf_fw_stats_cfg_info.bf_fw_roi_cfg_3a_order;

  return TRUE;
}

/** isp_pipeline47_parser_bg_stats:
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
boolean isp_pipeline47_parser_bg_stats(void *input_buf,
  mct_event_stats_isp_t *output,
  isp_saved_stats_params_t *saved_stats_params,
  isp_parser_session_params_t *parser_session_params)
{
  uint32_t                   *SY = NULL,*Sr = NULL, *Sb = NULL, *Sgr = NULL,
                             *Sgb = NULL;
  uint32_t                   *r_num = NULL, *b_num = NULL, *gr_num = NULL,
                             *gb_num = NULL;
  uint32_t                   *Sr_sat = NULL, *Sb_sat = NULL, *Sgr_sat = NULL,
                             *Sgb_sat = NULL;
  uint32_t                   *r_sat_num = NULL, *b_sat_num = NULL, *gr_sat_num = NULL,
                             *gb_sat_num = NULL;
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

  if (parser_session_params->saturated_ext == FALSE) {
    bg_stats->ex_fields = BG_EX_FIELDS_NONE;
  } else {
    bg_stats->ex_fields = BG_EX_FIELDS_SATURATED;
  }

  Sr     = bg_stats->bg_r_sum;
  Sb     = bg_stats->bg_b_sum;
  Sgr    = bg_stats->bg_gr_sum;
  Sgb    = bg_stats->bg_gb_sum;
  r_num  = bg_stats->bg_r_num;
  b_num  = bg_stats->bg_b_num;
  gr_num = bg_stats->bg_gr_num;
  gb_num = bg_stats->bg_gb_num;
  if (parser_session_params->saturated_ext == TRUE) {
    Sr_sat     = bg_stats->bg_r_sat_sum;
    Sb_sat     = bg_stats->bg_b_sat_sum;
    Sgr_sat    = bg_stats->bg_gr_sat_sum;
    Sgb_sat    = bg_stats->bg_gb_sat_sum;
    r_sat_num  = bg_stats->bg_r_sat_num;
    b_sat_num  = bg_stats->bg_b_sat_num;
    gr_sat_num = bg_stats->bg_gr_sat_num;
    gb_sat_num = bg_stats->bg_gb_sat_num;
  }
  current_region = (uint32_t*)input_buf;
  bg_stats->bg_region_h_num += (h_rgns_end - h_rgns_start + 1);
  bg_stats->bg_region_v_num = v_rgns_total;
  /*
   * BG Stats expect:
   * 1 - 30bit out of 32bit r_sum
   * 2 - 30bit out of 32bit b_sum
   * 3 - 30bit out of 32bit gr_sum
   * 4 - 30bit out of 32bit gb_sum
   * 5 - 16bit out of 32bit bcnt, 16bit out of 32bit rcnt
   * 6 - 16bit out of 32bit gbcnt, 16bit out of 32bit  grcnt
   * 7 - 30bit out of 32bit USL rnum
   * 8 - 30bit out of 32bit LSL bnum
   * 9 - 30bit out of 32bit USL grbnum
   * 10- 30bit out of 32bit LSL gbnum
   * 11- 16bit out of 32bit sat_bcnt, 16bit out of 32bit sat_rcnt
   * 12- 16bit out of 32bit sat_gbcnt, 16bit out of 32bit  sat_grcnt
   */
  for (j = 0; j < v_rgns_total; j++) {
    for (i = 0; i < h_rgns_total; i++) {
      if (i >= h_rgns_start && i <= h_rgns_end) {
        /* 30 bits sum of r, b, gr, gb. */
        *Sr = ((*(current_region)) & ALL_ONES_32(30));
        current_region ++;
        *Sb = ((*(current_region)) & ALL_ONES_32(30));
        current_region ++;
        *Sgr = ((*(current_region)) & ALL_ONES_32(30));
        current_region ++;
        *Sgb = ((*(current_region)) & ALL_ONES_32(30));
        current_region ++;
        /*16 bit pixel count used for r_sum, b_sum, gr_sum and gb_sum*/
        *r_num = ((*(current_region)) & ALL_ONES_32(16));
        *b_num = ((*(current_region)) & ALL_ONES_32_LEFTSHIFT(16,16)) >> 16;
        current_region++;
        *gr_num = ((*(current_region)) & ALL_ONES_32(16));
        *gb_num = ((*(current_region)) & ALL_ONES_32_LEFTSHIFT(16,16)) >> 16;
        current_region ++;
        if (parser_session_params->saturated_ext == TRUE) {
          /* 30 bits sum of r, b, gr, gb. Sat*/
          *Sr_sat = ((*(current_region)) & ALL_ONES_32(30));
        }
        current_region ++;
        if (parser_session_params->saturated_ext == TRUE) {
          *Sb_sat = ((*(current_region)) & ALL_ONES_32(30));
        }
        current_region ++;
        if (parser_session_params->saturated_ext == TRUE) {
          *Sgr_sat = ((*(current_region)) & ALL_ONES_32(30));
        }
        current_region ++;
        if (parser_session_params->saturated_ext == TRUE) {
          *Sgb_sat = ((*(current_region)) & ALL_ONES_32(30));
        }
        current_region ++;
        if (parser_session_params->saturated_ext == TRUE) {
          /*16 bit pixel count used for r_sum, b_sum, gr_sum and gb_sum Sat*/
          *r_sat_num = ((*(current_region)) & ALL_ONES_32(16));
          *b_sat_num = ((*(current_region)) & ALL_ONES_32_LEFTSHIFT(16,16)) >>
            16;
        }
        current_region++;
        if (parser_session_params->saturated_ext == TRUE) {
          *gr_sat_num = ((*(current_region)) & ALL_ONES_32(16));
          *gb_sat_num = ((*(current_region)) & ALL_ONES_32_LEFTSHIFT(16,16)) >>
            16;
        }
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
      if (parser_session_params->saturated_ext == TRUE) {
        Sr_sat++;
        Sb_sat++;
        Sgr_sat++;
        Sgb_sat++;
        r_sat_num++;
        b_sat_num++;
        gr_sat_num++;
        gb_sat_num++;
      }
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

/** isp_pipeline47_parser_aec_bg_stats:
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
boolean isp_pipeline47_parser_aec_bg_stats(void *input_buf,
  mct_event_stats_isp_t *output,
  isp_saved_stats_params_t *saved_stats_params,
  isp_parser_session_params_t *parser_session_params)
{
  uint32_t                   *SY = NULL,*Sr = NULL, *Sb = NULL, *Sgr = NULL,
                             *Sgb = NULL;
  uint32_t                   *r_num = NULL, *b_num = NULL, *gr_num = NULL,
                             *gb_num = NULL;
  uint32_t                   *Sr_sat = NULL, *Sb_sat = NULL, *Sgr_sat = NULL,
                             *Sgb_sat = NULL;
  uint32_t                   *r_sat_num = NULL, *b_sat_num = NULL, *gr_sat_num = NULL,
                             *gb_sat_num = NULL;
  uint32_t                   *current_region = NULL;
  uint32_t                    i = 0, j = 0, x = 0, y = 0;
  uint32_t                    h_rgns_start = 0;
  uint32_t                    h_rgns_end = 0;
  uint32_t                    h_rgns_total = 0;
  uint32_t                    v_rgns_total = 0;
  q3a_bg_stats_t             *aec_bg_stats = NULL;
  isp_rgns_stats_param_t     *rgns_stats = NULL;
  mct_event_stats_isp_data_t *stats_data = NULL;
  isp_stats_roi_params_t     *roi_params = NULL;

  if (!input_buf || !output || !saved_stats_params ||
    !parser_session_params) {
    ISP_ERR("failed: %p %p %p %p", input_buf, output,
      saved_stats_params, parser_session_params);
    return FALSE;
  }

  rgns_stats = &saved_stats_params->rgns_stats[MSM_ISP_STATS_AEC_BG];
  ISP_DBG("bg rgns_stats %d %d %d %d %d", rgns_stats->is_valid,
    rgns_stats->h_rgns_start, rgns_stats->h_rgns_end,
    rgns_stats->h_rgns_total, rgns_stats->v_rgns_total);

  if (rgns_stats->is_valid == FALSE) {
    ISP_ERR("failed: is_valid FALSE");
    return FALSE;
  }

  roi_params = &saved_stats_params->stats_roi_params[MSM_ISP_STATS_AEC_BG];

  output->stats_mask |= (1 << MSM_ISP_STATS_AEC_BG);
  stats_data = &output->stats_data[MSM_ISP_STATS_AEC_BG];
  if (stats_data->buf_size != sizeof(q3a_bg_stats_t)) {
    ISP_ERR("failed: invalid buf_size %d expected %d", stats_data->buf_size,
      sizeof(q3a_bg_stats_t));
    return FALSE;
  }

  aec_bg_stats = (q3a_bg_stats_t *)stats_data->stats_buf;
  if (!aec_bg_stats) {
    ISP_ERR("failed: aec_bg_stats NULL");
    return FALSE;
  }

  stats_data->stats_type = MSM_ISP_STATS_AEC_BG;
  stats_data->used_size = sizeof(*aec_bg_stats);
  ISP_DBG("bg stats_mask %x type %d buf %p size %d",
    output->stats_mask, stats_data->stats_type, stats_data->stats_buf,
    stats_data->buf_size);

  h_rgns_start = rgns_stats->h_rgns_start;
  h_rgns_end = rgns_stats->h_rgns_end;
  h_rgns_total = rgns_stats->h_rgns_total;
  v_rgns_total = rgns_stats->v_rgns_total;

  if (parser_session_params->saturated_ext == FALSE) {
    aec_bg_stats->ex_fields = BG_EX_FIELDS_NONE;
  } else {
    aec_bg_stats->ex_fields = BG_EX_FIELDS_SATURATED;
  }

  Sr     = aec_bg_stats->bg_r_sum;
  Sb     = aec_bg_stats->bg_b_sum;
  Sgr    = aec_bg_stats->bg_gr_sum;
  Sgb    = aec_bg_stats->bg_gb_sum;
  r_num  = aec_bg_stats->bg_r_num;
  b_num  = aec_bg_stats->bg_b_num;
  gr_num = aec_bg_stats->bg_gr_num;
  gb_num = aec_bg_stats->bg_gb_num;
  if (parser_session_params->saturated_ext == TRUE) {
    Sr_sat     = aec_bg_stats->bg_r_sat_sum;
    Sb_sat     = aec_bg_stats->bg_b_sat_sum;
    Sgr_sat    = aec_bg_stats->bg_gr_sat_sum;
    Sgb_sat    = aec_bg_stats->bg_gb_sat_sum;
    r_sat_num  = aec_bg_stats->bg_r_sat_num;
    b_sat_num  = aec_bg_stats->bg_b_sat_num;
    gr_sat_num = aec_bg_stats->bg_gr_sat_num;
    gb_sat_num = aec_bg_stats->bg_gb_sat_num;
  }
  current_region = (uint32_t*)input_buf;
  aec_bg_stats->bg_region_h_num += (h_rgns_end - h_rgns_start + 1);
  aec_bg_stats->bg_region_v_num = v_rgns_total;
  /*
   * BG Stats expect:
   * 1 - 30bit out of 32bit r_sum
   * 2 - 30bit out of 32bit b_sum
   * 3 - 30bit out of 32bit gr_sum
   * 4 - 30bit out of 32bit gb_sum
   * 5 - 16bit out of 32bit bcnt, 16bit out of 32bit rcnt
   * 6 - 16bit out of 32bit gbcnt, 16bit out of 32bit  grcnt
   * 7 - 30bit out of 32bit USL rnum
   * 8 - 30bit out of 32bit LSL bnum
   * 9 - 30bit out of 32bit USL grbnum
   * 10- 30bit out of 32bit LSL gbnum
   * 11- 16bit out of 32bit sat_bcnt, 16bit out of 32bit sat_rcnt
   * 12- 16bit out of 32bit sat_gbcnt, 16bit out of 32bit  sat_grcnt
   */
  for (j = 0; j < v_rgns_total; j++) {
    for (i = 0; i < h_rgns_total; i++) {
      if (i >= h_rgns_start && i <= h_rgns_end) {
        /* 30 bits sum of r, b, gr, gb. */
        *Sr = ((*(current_region)) & ALL_ONES_32(30));
        current_region ++;
        *Sb = ((*(current_region)) & ALL_ONES_32(30));
        current_region ++;
        *Sgr = ((*(current_region)) & ALL_ONES_32(30));
        current_region ++;
        *Sgb = ((*(current_region)) & ALL_ONES_32(30));
        current_region ++;
        /*16 bit pixel count used for r_sum, b_sum, gr_sum and gb_sum*/
        *r_num = ((*(current_region)) & ALL_ONES_32(16));
        *b_num = ((*(current_region)) & ALL_ONES_32_LEFTSHIFT(16,16)) >> 16;
        current_region++;
        *gr_num = ((*(current_region)) & ALL_ONES_32(16));
        *gb_num = ((*(current_region)) & ALL_ONES_32_LEFTSHIFT(16,16)) >> 16;
        current_region ++;
        if (parser_session_params->saturated_ext == TRUE) {
          /* 30 bits sum of r, b, gr, gb. Sat*/
          *Sr_sat = ((*(current_region)) & ALL_ONES_32(30));
        }
        current_region ++;
        if (parser_session_params->saturated_ext == TRUE) {
          *Sb_sat = ((*(current_region)) & ALL_ONES_32(30));
        }
        current_region ++;
        if (parser_session_params->saturated_ext == TRUE) {
          *Sgr_sat = ((*(current_region)) & ALL_ONES_32(30));
        }
        current_region ++;
        if (parser_session_params->saturated_ext == TRUE) {
          *Sgb_sat = ((*(current_region)) & ALL_ONES_32(30));
        }
        current_region ++;
        if (parser_session_params->saturated_ext == TRUE) {
          /*16 bit pixel count used for r_sum, b_sum, gr_sum and gb_sum Sat*/
          *r_sat_num = ((*(current_region)) & ALL_ONES_32(16));
          *b_sat_num = ((*(current_region)) & ALL_ONES_32_LEFTSHIFT(16,16)) >>
            16;
        }
        current_region++;
        if (parser_session_params->saturated_ext == TRUE) {
          *gr_sat_num = ((*(current_region)) & ALL_ONES_32(16));
          *gb_sat_num = ((*(current_region)) & ALL_ONES_32_LEFTSHIFT(16,16)) >>
            16;
        }
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
      if (parser_session_params->saturated_ext == TRUE) {
        Sr_sat++;
        Sb_sat++;
        Sgr_sat++;
        Sgb_sat++;
        r_sat_num++;
        b_sat_num++;
        gr_sat_num++;
        gb_sat_num++;
      }
    }
  }
  aec_bg_stats->bg_region_height = roi_params->rgnHeight + 1;
  aec_bg_stats->bg_region_width = roi_params->rgnWidth + 1;
  aec_bg_stats->region_pixel_cnt =
    (roi_params->rgnWidth + 1) * (roi_params->rgnHeight + 1);
  aec_bg_stats->rMax = roi_params->rMax;
  aec_bg_stats->bMax = roi_params->bMax;
  aec_bg_stats->grMax = roi_params->grMax;
  aec_bg_stats->gbMax = roi_params->gbMax;

  /* convert bayer r,g,b stat into Ysum to make
   * it work on current 3a version
   * that uses 16x16 Ysum is done in 3A code
   **/
  return TRUE;
}

/** isp_pipeline47_parser_rs_stats:
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
boolean isp_pipeline47_parser_rs_stats(void *input_buf,
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
  q3a_rs_stats_t             *rs_stats = NULL;
  isp_rgns_stats_param_t     *rgns_stats = NULL;
  mct_event_stats_isp_data_t *stats_data = NULL;
  uint32_t                    num_rs_h_rgn = 0;

  if (!input_buf || !output || !saved_stats_params ||
    !parser_session_params) {
    ISP_ERR("failed: %p %p %p %p", input_buf, output,
      saved_stats_params, parser_session_params);
    return FALSE;
  }

  rgns_stats = &saved_stats_params->rgns_stats[MSM_ISP_STATS_RS];
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
    rs_stats->num_row_sum =  num_rs_h_rgn * v_rgns_total;
    rs_stats->num_h_regions = num_rs_h_rgn;
  } else {
    rs_stats->num_row_sum +=  num_rs_h_rgn * v_rgns_total;
    rs_stats->num_h_regions += num_rs_h_rgn;
  }

  if (h_rgns_end >= MAX_RS_H_REGIONS || h_rgns_end >= MAX_RS_H_REGIONS) {
    ISP_ERR("error: h_rgns_end  >= MAX_RS_H_REGIONS"
      "or h_rgns_end >= MAX_RS_H_REGIONS");
    return FALSE;
  }
  rs_stats->num_v_regions = v_rgns_total;
  for (j = 0; j < v_rgns_total; j++) {
    for (i = h_rgns_start; i <= h_rgns_end; i++) {
      rs_stats->row_sum[i][j] += (*current_region++);
    }
  }
  return TRUE;
}

/** isp_pipeline47_parser_cs_stats:
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
boolean isp_pipeline47_parser_cs_stats(void *input_buf,
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

/** isp_pipeline47_parser_ihist_stats:
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
boolean isp_pipeline47_parser_ihist_stats(void *input_buf,
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

  ihist_stats->valid_fields = IHIST_VALID_B | IHIST_VALID_G |
                              IHIST_VALID_R | IHIST_VALID_YCC ;
  ihist_stats->num_bins = STATS_IHIST_MAX_NUM;
  ISP_DBG("stats_mask %x type %d buf %p size %d",
     output->stats_mask, stats_data->stats_type, stats_data->stats_buf,
     stats_data->buf_size);

  hist_statsBuffer = (uint16_t *)input_buf;
  for (i= 0; i< ihist_stats->num_bins; i++) {
    ihist_stats->ycc[i] += *hist_statsBuffer;
    hist_statsBuffer++;
    ISP_DBG("%d : ycc %d", i , ihist_stats->ycc[i]);
  }
  for (i= 0; i< ihist_stats->num_bins; i++) {
    ihist_stats->g[i] += *hist_statsBuffer;
    hist_statsBuffer++;
    ISP_DBG("%d : g %d", i , ihist_stats->g[i]);
  }
  for (i= 0; i< ihist_stats->num_bins; i++) {
    ihist_stats->b[i] += *hist_statsBuffer;
    hist_statsBuffer++;
    ISP_DBG("%d : b %d", i , ihist_stats->b[i]);
  }
  for (i= 0; i< ihist_stats->num_bins; i++) {
    ihist_stats->r[i] += *hist_statsBuffer;
    hist_statsBuffer++;
    ISP_DBG("%d : r %d", i , ihist_stats->r[i]);
  }
  return TRUE;
}

/** isp_pipeline47_parser_bhist_stats:
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
boolean isp_pipeline47_parser_bhist_stats(void *input_buf,
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

  stats_data->stats_type = MSM_ISP_STATS_BHIST;
  stats_data->used_size = sizeof(*bhist_stats);
  ISP_DBG("bhist stats_mask %x type %d buf %p size %d",
    output->stats_mask, stats_data->stats_type, stats_data->stats_buf,
    stats_data->buf_size);

  bhist_stats->num_bins = STATS_BHIST_BIN_SIZE;
  Srh = bhist_stats->bayer_r_hist;
  Sbh = bhist_stats->bayer_b_hist;
  Sgrh = bhist_stats->bayer_gr_hist;
  Sgbh = bhist_stats->bayer_gb_hist;
  current_region = input_buf;

  for (i = 0; i < bhist_stats->num_bins; i++) {
    *Srh += ((*(current_region)) & ALL_ONES_32(25));
    Srh++; current_region++;
    *Sbh += ((*(current_region)) & ALL_ONES_32(25));
    Sbh++; current_region++;
    *Sgrh += ((*(current_region)) & ALL_ONES_32(25));
    Sgrh++; current_region++;
    *Sgbh += ((*(current_region)) & ALL_ONES_32(25));
    Sgbh++; current_region++;
  }

  bhist_stats->bayer_b_hist_valid = bhist_stats->bayer_gb_hist_valid =
    bhist_stats->bayer_gr_hist_valid = bhist_stats->bayer_r_hist_valid = true;

  return TRUE;
}

/** isp_pipeline47_parser_hdr_be_stats:
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
boolean isp_pipeline47_parser_hdr_be_stats(void *input_buf,
  mct_event_stats_isp_t *output,
  isp_saved_stats_params_t *saved_stats_params,
  isp_parser_session_params_t *parser_session_params)
{
  int                         window = 0;
  uint32_t                    i = 0, j = 0;
  uint32_t                   *Sr = NULL,*Sb = NULL, *Sgr = NULL, *Sgb = NULL;

  uint32_t                   *r_num = NULL, *b_num = NULL, *gr_num = NULL,
                             *gb_num = NULL;
  uint32_t                   *Sr_sat = NULL, *Sb_sat = NULL, *Sgr_sat = NULL,
                             *Sgb_sat = NULL;
  uint32_t                   *r_sat_num = NULL, *b_sat_num = NULL, *gr_sat_num = NULL,
                             *gb_sat_num = NULL;

  uint32_t                   *current_region = NULL;
  uint32_t                    h_rgns_start = 0;
  uint32_t                    h_rgns_end = 0;
  uint32_t                    h_rgns_total = 0;
  uint32_t                    v_rgns_total = 0;
  q3a_hdr_be_stats_t          *hdr_be_stats = NULL;
  isp_rgns_stats_param_t      *rgns_stats = NULL;
  mct_event_stats_isp_data_t  *stats_data = NULL;
  isp_stats_roi_params_t     *roi_params = NULL;

  if (!input_buf || !output || !saved_stats_params ||
    !parser_session_params) {
    ISP_ERR("failed: %p %p %p %p", input_buf, output,
      saved_stats_params, parser_session_params);
    return FALSE;
  }

  rgns_stats = &saved_stats_params->rgns_stats[MSM_ISP_STATS_HDR_BE];
  ISP_DBG("bf scale rgns_stats %d %d %d %d %d", rgns_stats->is_valid,
    rgns_stats->h_rgns_start, rgns_stats->h_rgns_end,
    rgns_stats->h_rgns_total, rgns_stats->v_rgns_total);

  if (rgns_stats->is_valid == FALSE) {
    ISP_ERR("failed: is_valid FALSE");
    return FALSE;
  }

  roi_params = &saved_stats_params->stats_roi_params[MSM_ISP_STATS_HDR_BE];

  output->stats_mask |= (1 << MSM_ISP_STATS_HDR_BE);
  stats_data = &output->stats_data[MSM_ISP_STATS_HDR_BE];
  if (stats_data->buf_size != sizeof(q3a_hdr_be_stats_t)) {
    ISP_ERR("failed: invalid buf_size %d expected %d", stats_data->buf_size,
      sizeof(q3a_hdr_be_stats_t));
    return FALSE;
  }

  hdr_be_stats = (q3a_hdr_be_stats_t *)stats_data->stats_buf;
  if (!hdr_be_stats) {
    ISP_ERR("failed: hdr_be_stats NULL");
    return FALSE;
  }

  stats_data->stats_type = MSM_ISP_STATS_HDR_BE;
  stats_data->used_size = sizeof(*hdr_be_stats);
  ISP_DBG("bf scale stats_mask %x type %d buf %p size %d",
    output->stats_mask, stats_data->stats_type, stats_data->stats_buf,
    stats_data->buf_size);
  hdr_be_stats->hdr_mode = EXPOSURE_ALL;

  if (parser_session_params->saturated_ext == FALSE) {
    hdr_be_stats->ex_fields = BG_EX_FIELDS_NONE;
  } else {
    hdr_be_stats->ex_fields = BG_EX_FIELDS_SATURATED;
  }

  h_rgns_start = rgns_stats->h_rgns_start;
  h_rgns_end = rgns_stats->h_rgns_end;
  h_rgns_total = rgns_stats->h_rgns_total;
  v_rgns_total = rgns_stats->v_rgns_total;

  Sr         = hdr_be_stats->be_r_sum;
  Sb         = hdr_be_stats->be_b_sum;
  Sgr        = hdr_be_stats->be_gr_sum;
  Sgb        = hdr_be_stats->be_gb_sum;
  r_num      = hdr_be_stats->be_r_num;
  b_num      = hdr_be_stats->be_b_num;
  gr_num     = hdr_be_stats->be_gr_num;
  gb_num     = hdr_be_stats->be_gb_num;

  if (parser_session_params->saturated_ext == TRUE) {
    Sr_sat     = hdr_be_stats->be_r_sat_sum;
    Sb_sat     = hdr_be_stats->be_b_sat_sum;
    Sgr_sat    = hdr_be_stats->be_gr_sat_sum;
    Sgb_sat    = hdr_be_stats->be_gb_sat_sum;

    r_sat_num  = hdr_be_stats->be_r_sat_num;
    b_sat_num  = hdr_be_stats->be_b_sat_num;
    gr_sat_num = hdr_be_stats->be_gr_sat_num;
    gb_sat_num = hdr_be_stats->be_gb_sat_num;
  }

  current_region = (uint32_t *)input_buf;
  hdr_be_stats->be_region_h_num += (h_rgns_end - h_rgns_start + 1);
  hdr_be_stats->be_region_v_num = v_rgns_total;

  for (j = 0; j < v_rgns_total; j++) {
    for (i = 0; i < h_rgns_total; i++) {
      if (i >= h_rgns_start && i <= h_rgns_end) {
        /*parse HDR_BE stats */
        *Sr = (*(current_region)) & ALL_ONES_32(30);
        current_region++;
        *Sb = (*(current_region)) & ALL_ONES_32(30);
        current_region++;
        *Sgr = (*(current_region)) & ALL_ONES_32(30);
        current_region++;
        *Sgb = (*(current_region)) & ALL_ONES_32(30);
        current_region++;
        *r_num = ((*(current_region)) & ALL_ONES_32(16));
        *b_num = (((*(current_region)) & ALL_ONES_32_LEFTSHIFT(16,16))) >> 16;
        current_region++;

        *gr_num = ((*(current_region)) & ALL_ONES_32(16));
        *gb_num = (((*(current_region)) & ALL_ONES_32_LEFTSHIFT(16,16))) >> 16;
        current_region++;

        if (parser_session_params->saturated_ext == TRUE) {
          /*parse HDR_BE stats */
          *Sr_sat = (*(current_region)) & ALL_ONES_32(30);
        }
        current_region++;
        if (parser_session_params->saturated_ext == TRUE) {
          *Sb_sat = (*(current_region)) & ALL_ONES_32(30);
        }
        current_region++;
        if (parser_session_params->saturated_ext == TRUE) {
          *Sgr_sat = (*(current_region)) & ALL_ONES_32(30);
        }
        current_region++;
        if (parser_session_params->saturated_ext == TRUE) {
          *Sgb_sat = (*(current_region)) & ALL_ONES_32(30);
        }
        current_region++;
        if (parser_session_params->saturated_ext == TRUE) {
          *r_sat_num = ((*(current_region)) & ALL_ONES_32(16));
          *b_sat_num = (((*(current_region)) & ALL_ONES_32_LEFTSHIFT(16,16))) >> 16;
        }
        current_region++;

        if (parser_session_params->saturated_ext == TRUE) {
          *gr_sat_num = ((*(current_region)) & ALL_ONES_32(16));
          *gb_sat_num = (((*(current_region)) & ALL_ONES_32_LEFTSHIFT(16,16))) >> 16;
        }
        current_region++;
      }
      Sr++;
      Sb++;
      Sgr++;
      Sgb++;
      r_num++;
      b_num++;
      gr_num++;
      gb_num++;
      if (parser_session_params->saturated_ext == TRUE) {
        Sr_sat++;
        Sb_sat++;
        Sgr_sat++;
        Sgb_sat++;
        r_sat_num++;
        b_sat_num++;
        gr_sat_num++;
        gb_sat_num++;
      }
    }
  }

  hdr_be_stats->rgnWidth = roi_params->rgnWidth + 1;
  hdr_be_stats->rgnHeight = roi_params->rgnHeight + 1;
  hdr_be_stats->rgnHOffset = roi_params->rgnHOffset;
  hdr_be_stats->rgnVOffset = roi_params->rgnVOffset;

  hdr_be_stats->region_pixel_cnt =
    (roi_params->rgnWidth + 1) * (roi_params->rgnHeight + 1);
  hdr_be_stats->rMax = roi_params->rMax;
  hdr_be_stats->bMax = roi_params->bMax;
  hdr_be_stats->grMax = roi_params->grMax;
  hdr_be_stats->gbMax = roi_params->gbMax;

  return TRUE;
}

/** isp_pipeline47_parser_hdr_bhist_stats:
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
boolean isp_pipeline47_parser_hdr_bhist_stats(void *input_buf,
  mct_event_stats_isp_t *output,
  isp_saved_stats_params_t *saved_stats_params_isp0,
  isp_saved_stats_params_t *saved_stats_params_isp1,
  isp_parser_session_params_t *parser_session_params)
{
  uint16_t                   *current_region = NULL;
  uint32_t                    i = 0;
  q3a_bhist_stats_t          *hdr_bhist_stats = NULL;
  mct_event_stats_isp_data_t *stats_data = NULL;
  uint32_t                   *Srh = NULL, *Sbh = NULL, *Sgrh = NULL,
                             *Sgbh = NULL;

  if (!input_buf || !output || !saved_stats_params_isp0 ||
    !saved_stats_params_isp1 || !parser_session_params) {
    ISP_ERR("failed: %p %p %p %p %p", input_buf, output,
      saved_stats_params_isp0, saved_stats_params_isp1, parser_session_params);
    return FALSE;
  }

  output->stats_mask |= (1 << MSM_ISP_STATS_HDR_BHIST);
  stats_data = &output->stats_data[MSM_ISP_STATS_HDR_BHIST];
  if (stats_data->buf_size != sizeof(q3a_bhist_stats_t)) {
    ISP_ERR("failed: invalid buf_size %d expected %d", stats_data->buf_size,
      sizeof(q3a_bhist_stats_t));
    return FALSE;
  }

  hdr_bhist_stats = (q3a_bhist_stats_t *)stats_data->stats_buf;
  if (!hdr_bhist_stats) {
    ISP_ERR("failed: hdr_bhist_stats NULL");
    return FALSE;
  }

  hdr_bhist_stats->num_bins = STATS_HDR_BHIST_BIN_SIZE;
  stats_data->stats_type = MSM_ISP_STATS_HDR_BHIST;
  stats_data->used_size = sizeof(*hdr_bhist_stats);
  ISP_DBG("bhist stats_mask %x type %d buf %p size %d",
    output->stats_mask, stats_data->stats_type, stats_data->stats_buf,
    stats_data->buf_size);

  Srh = hdr_bhist_stats->bayer_r_hist;
  Sbh = hdr_bhist_stats->bayer_b_hist;
  Sgrh = hdr_bhist_stats->bayer_gr_hist;
  Sgbh = hdr_bhist_stats->bayer_gb_hist;
  current_region = input_buf;

  for (i = 0; i < hdr_bhist_stats->num_bins ; i++) {
    *Srh += ((*(current_region)) & ALL_ONES_32(25));
    Srh++; current_region++;
    *Sbh += ((*(current_region)) & ALL_ONES_32(25));
    Sbh++; current_region++;
    *Sgrh += ((*(current_region)) & ALL_ONES_32(25));
    Sgrh++; current_region++;
    *Sgbh += ((*(current_region)) & ALL_ONES_32(25));
    Sgbh++; current_region++;
  }

  hdr_bhist_stats->bayer_b_hist_valid = hdr_bhist_stats->bayer_gb_hist_valid =
    hdr_bhist_stats->bayer_gr_hist_valid = hdr_bhist_stats->bayer_r_hist_valid =
    true;

  return TRUE;
}
