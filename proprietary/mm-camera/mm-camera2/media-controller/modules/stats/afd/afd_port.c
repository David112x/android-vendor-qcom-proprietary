/* afd_port.c
 *
 * Copyright (c) 2013-2014, 2017-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "afd_port.h"
#include "afd_thread.h"
#include "stats_port.h"
#include "stats_module.h"
#include "modules.h"
#include "stats_util.h"
#include "sensor_lib.h"
#define AFD_SEND_IMMEDIATE 0

/* Tuning parameters */
/* Due the lack of reserve fields in AFD Chromatix add new parameters here,
 * don't move to afd.h that will make this definitions accessible to algo */
/* static level 3 slope check threshold */
#define AFD_STATIC_SLOPE_STEEP_THR 200000
#define AFD_STATIC_SLOPE_NOT_STEEP_THR 10000
/* static level 2 confidence calculation */
#define AFD_STATIC_CONFIDENCE_LEVEL_H 5
#define AFD_STATIC_CONFIDENCE_LEVEL_L 3
#define AFD_STATIC_CONFIDENCE_LEVEL_H_RATIO (1.2f)
#define AFD_STATIC_CONFIDENCE_LEVEL_L_RATIO (1.1f)
#define AFD_STATIC_CONFIDENCE_LEVEL_H_LL_RATIO (1.1f)
#define AFD_STATIC_CONFIDENCE_LEVEL_L_LL_RATIO (1.05f)
/* low light static tuning parameters */
#define AFD_STATIC_LUX_IDX_LOWLIGHT_THRESHOLD (350.0f)
#define AFD_STATIC_THRESHOLD_LEVEL_COMPENSATION (0.98f)
#define AFD_STATIC_ROW_SUM_LOWLIGHT_THR 5000
#define AFD_STATIC_ROW_SUM_LOWLIGHT_COMP 8
/* end of tuning parameters */

/*Forward Declarations*/
static boolean afd_port_link_to_peer(mct_port_t *port,
                                        mct_event_t *event);
static boolean afd_port_unlink_from_peer(mct_port_t *port,
                                              mct_event_t *event);
static boolean afd_port_local_unlink(mct_port_t *port,
                                        mct_event_t *event);
static boolean afd_port_remote_unlink(mct_port_t *port);
static boolean afd_port_handle_role_switch(mct_port_t *port,mct_event_t *event);
static boolean afd_port_set_role_switch(afd_port_private_t  *private);
static boolean afd_port_reset_dual_cam_info(afd_port_private_t  *port);
static boolean afd_port_if_stats_can_be_configured(afd_port_private_t  *port);
static boolean afd_port_update_aux_sync_and_update(afd_port_private_t  *port);
static boolean afd_port_update_LPM(mct_port_t *port, cam_dual_camera_perf_control_t* perf_ctrl);
static boolean afd_port_is_algo_active(mct_port_t *port);
static boolean afd_port_forward_bus_message_if_linked(mct_port_t *mct_port);
static boolean afd_port_forward_update_event_if_linked(mct_port_t* mct_port,
                                                              stats_update_t* stats_update);
static boolean afd_port_store_peer_bus_message(mct_port_t *mct_port,mct_bus_msg_afd_t *afd_msg);
static boolean afd_port_store_peer_update_event(mct_port_t* mct_port, stats_update_t* stats_update);
static boolean afd_port_intra_event(mct_port_t *port, mct_event_t *event);

static void afd_port_configure_rs_stats(afd_output_data_t *output,
  mct_port_t *port)
{
  afd_port_private_t *private = NULL;
  mct_event_t        event;
  rs_config_t  rs_config;

  private = (afd_port_private_t *)(port->port_private);
  rs_config.max_algo_support_h_rgn = output->max_algo_hnum;
  rs_config.max_algo_support_v_rgn = output->max_algo_vnum;

  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = private->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_RS_CONFIG_UPDATE;
  event.u.module_event.current_frame_id = output->sof_id;
  event.u.module_event.module_event_data = (void *)(&rs_config);

  MCT_PORT_EVENT_FUNC(port)(port, &event);
}

static void afd_send_bus_message(mct_port_t *port,
  mct_bus_msg_type_t bus_msg_type,
  void* payload,
  int size,
  int sof_id)
{
  afd_port_private_t *afd_port = (afd_port_private_t *)(port->port_private);
  mct_event_t event;
  mct_bus_msg_t bus_msg;
  STATS_MEMSET(&bus_msg, 0, sizeof(mct_bus_msg_t));

  bus_msg.sessionid = (afd_port->reserved_id >> 16);
  bus_msg.type = bus_msg_type;
  bus_msg.msg = payload;
  bus_msg.size = size;

  /* pack into an mct_event object*/
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = afd_port->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_POST_TO_BUS;
  event.u.module_event.current_frame_id = sof_id;
  event.u.module_event.module_event_data = (void *)(&bus_msg);

  MCT_PORT_EVENT_FUNC(port)(port, &event);
  return;
}

/** afd_send_batch_bus_message:
 *    @port:   pointer to afd port
 *    @regular_sof_id: sof_id for data been send
 *
 * Batch AFD metadata into a single message and send it.
 *
 * Return nothing
 **/
static void afd_send_batch_bus_message(mct_port_t *port, uint32_t regular_sof_id)
{
  afd_port_private_t *private = (afd_port_private_t *)(port->port_private);
  mct_bus_msg_afd_t afd_regular;
  afd_regular.scene_flicker = private->meta_scene_flicker;
  afd_regular.antibanding_mode = private->antibanding_mode;

  afd_send_bus_message(port, MCT_BUS_MSG_AFD, (void*)&afd_regular,
    sizeof(mct_bus_msg_afd_t), regular_sof_id);
}

static void afd_port_stats_done_callback(void *p, void* stats)
{
  mct_port_t         *port    = (mct_port_t *)p;
  afd_port_private_t *private = NULL;
  stats_t            *afd_stats = (stats_t *)stats;

  if (!port) {
    return;
  }
  private = (afd_port_private_t *)(port->port_private);
  if (!private) {
    return;
  }

  if (afd_stats) {
    AFD_HIGH("DONE AFD stats ACK back");
    circular_stats_data_done(afd_stats->ack_data, port,
                             private->reserved_id, 0);
  }
}
/** afd_port_callback
 *
 **/
static void afd_port_callback(afd_output_data_t *output, void *p)
{
  mct_port_t         *port    = (mct_port_t *)p;
  afd_port_private_t *private = NULL;
  mct_event_t        event;
  stats_update_t     stats_update;

  if (!output || !port) {
    return;
  }
  private = (afd_port_private_t *)(port->port_private);
  if (!private) {
    return;
  }
  if (output->type & AFD_CB_OUTPUT) {
    uint8_t meta_scene_flicker;
    switch (output->afd_atb) {
    case AFD_TYPE_60HZ: {
      stats_update.afd_update.afd_atb = AFD_TBL_60HZ;
      meta_scene_flicker = CAM_FLICKER_60_HZ;
    }
      break;
    case AFD_TYPE_50HZ: {
      stats_update.afd_update.afd_atb = AFD_TBL_50HZ;
      meta_scene_flicker = CAM_FLICKER_50_HZ;
    }
      break;
    default: {
      meta_scene_flicker = CAM_FLICKER_NONE;
      stats_update.afd_update.afd_atb = AFD_TBL_OFF;
    }
      break;
    }
    stats_update.afd_update.afd_enable    = output->afd_enable;
    stats_update.afd_update.afd_exec_once = output->afd_exec_once;
    stats_update.afd_update.afd_monitor   = output->afd_monitor;
    stats_update.flag                     = STATS_UPDATE_AFD;
    /* pack into an mct_event object*/
    event.direction = MCT_EVENT_UPSTREAM;
    event.identity = private->reserved_id;
    event.type     = MCT_EVENT_MODULE_EVENT;

    event.u.module_event.type              = MCT_EVENT_MODULE_STATS_AFD_UPDATE;
    event.u.module_event.module_event_data = (void *)(&stats_update);
    event.u.module_event.current_frame_id  = output->sof_id;
    afd_port_forward_update_event_if_linked(port,&stats_update);
    MCT_PORT_EVENT_FUNC(port)(port, &event);
    private->meta_scene_flicker = meta_scene_flicker;
    /* Configure RS num*/
  }

  if((output->type & AFD_CB_STATS_CONFIG) &&
    afd_port_if_stats_can_be_configured(private)) {
    afd_port_configure_rs_stats(output, port);
  }

  return;
}

/** afd_port_start_threads
 *    @port: pointer to afd port
 *  Launch afd thread
 **/
static boolean afd_port_init_threads(mct_port_t *port)
{
  boolean            rc = TRUE;
  afd_port_private_t *private = port->port_private;

  private->thread_data = afd_thread_init();
  AFD_LOW("private->thread_data: %p", private->thread_data);
  if (private->thread_data == NULL) {
    AFD_ERR("private->thread_data is NULL");
    rc = FALSE;
  }
  return rc;
}

/** afd_port_start_threads
 *    @port: pointer to afd port
 *  Launch afd thread
 **/
static boolean afd_port_start_threads(mct_port_t *port)
{
  boolean     rc = FALSE;
  afd_port_private_t *private = port->port_private;

  if (private->thread_data != NULL) {
    rc = afd_thread_start(port);
    if (rc == FALSE) {
      afd_thread_deinit(port);
    }
  }
  AFD_LOW("Start afd thread");
  return rc;
}

/** afd_port_check_session_id
 *    @d1: session+stream identity
 *    @d2: session+stream identity
 *
 *  To find out if both identities are matching;
 *  Return TRUE if matches.
 **/
static boolean afd_port_check_session_id(void *d1, void *d2)
{
  unsigned int v1, v2;
  v1 = *((unsigned int *)d1);
  v2 = *((unsigned int *)d2);

  return (((v1 & 0xFFFF0000) ==
    (v2 & 0xFFFF0000)) ? TRUE : FALSE);
}

/** afd_port_fill_extended_params:
 *    @params: structure to be fill with tuning data
 *
 *  Fill extended parameters used for tuning AFD algorithm
 **/
static void afd_port_fill_extended_params(afd_extended_parameters_t *params)
{
  /* static level 3 slope check threshold */
  params->static_slope_steep_thr = AFD_STATIC_SLOPE_STEEP_THR;
  params->static_slope_not_steep_thr = AFD_STATIC_SLOPE_NOT_STEEP_THR;
  /* static level 2 confidence calculation */
  params->static_confidence_level_h = AFD_STATIC_CONFIDENCE_LEVEL_H;
  params->static_confidence_level_l = AFD_STATIC_CONFIDENCE_LEVEL_L;
  params->static_confidence_level_h_ratio = AFD_STATIC_CONFIDENCE_LEVEL_H_RATIO;
  params->static_confidence_level_l_ratio = AFD_STATIC_CONFIDENCE_LEVEL_L_RATIO;
  params->static_confidence_level_h_ll_ratio =
    AFD_STATIC_CONFIDENCE_LEVEL_H_LL_RATIO;
  params->static_confidence_level_l_ll_ratio =
    AFD_STATIC_CONFIDENCE_LEVEL_L_LL_RATIO;
  params->static_confidence_level_slope_ratio =
    AFD_STATIC_CONFIDENCE_LEVEL_H_RATIO;
  /* low light static tuning parameters */
  params->static_lux_idx_lowlight_threshold =
    AFD_STATIC_LUX_IDX_LOWLIGHT_THRESHOLD;
  params->static_threshold_level_compensation =
    AFD_STATIC_THRESHOLD_LEVEL_COMPENSATION;
  params->static_row_sum_lowlight_thr = AFD_STATIC_ROW_SUM_LOWLIGHT_THR;
  params->static_row_sum_lowlight_comp = AFD_STATIC_ROW_SUM_LOWLIGHT_COMP;
}


static int32_t afd_port_parse_RDI_stats(afd_port_private_t *private,
  uint32_t *destLumaBuff, void *rawBuff)
{
  int32_t rc = -1;

  if (private->parse_RDI_stats.parse_flicker_stats_callback != NULL) {
    rc = private->parse_RDI_stats.parse_flicker_stats_callback(destLumaBuff,
        rawBuff);
  } else {
    AFD_LOW(" parse_FLICKER_stats_callback not supported");
    rc = -1;
  }

  return rc;
}

static void afd_hdr_stats_send_event(afd_port_private_t *private)
{
  /* TODO: add lock for the send event arbitration*/
  boolean send_immediately = FALSE;

  /* stats sync*/
  if((private->hdr_stats_sync.flicker_stats_frame_id == private->hdr_stats_sync.bg_stats_frame_id) &&
      (private->hdr_stats_sync.stats_type_mask == (STATS_BG | STATS_RS))) {//debug
    private->hdr_stats_sync.stats_type_mask &= ~(STATS_BG | STATS_RS);
    send_immediately = TRUE;
  }else if(private->hdr_stats_sync.flicker_stats_frame_id > private->hdr_stats_sync.bg_stats_frame_id) {
    /* clear bg stats */
    private->hdr_stats_sync.stats_type_mask &= ~STATS_BG;//debug
    send_immediately = FALSE;
  }else if(private->hdr_stats_sync.flicker_stats_frame_id < private->hdr_stats_sync.bg_stats_frame_id) {
    /* clear bg stats */
    private->hdr_stats_sync.stats_type_mask &= ~STATS_RS;
    send_immediately = FALSE;
  }else {
    send_immediately = FALSE;
  }
  AFD_HIGH("flicker_stats_frame_id=%d, bg_stats_frame_id=%d, ae_hist_stats_type_mask=0x%x, send_immediately=%d",
      private->hdr_stats_sync.flicker_stats_frame_id,
      private->hdr_stats_sync.bg_stats_frame_id,
      private->hdr_stats_sync.stats_type_mask,
      send_immediately);

  /* send stats to core*/
  if(send_immediately == TRUE) {
    afd_thread_msg_t *afd_msg   =
      (afd_thread_msg_t *)malloc(sizeof(afd_thread_msg_t));
    if (afd_msg == NULL) {
      return;
    }
    STATS_MEMSET(afd_msg, 0 , sizeof(afd_thread_msg_t));
    stats_t * afd_stats = (stats_t *)calloc(1, sizeof(stats_t));
    if(afd_stats == NULL) {
      free(afd_msg);
      afd_msg = NULL;
      return;
    }

    afd_msg->type = MSG_AFD_STATS;
    afd_stats->stats_type_mask = STATS_BG | STATS_RS;
    afd_stats->frame_id = private->hdr_stats_sync.bg_stats_frame_id;
    afd_stats->bayer_stats.p_q3a_bg_stats = private->hdr_stats_sync.p_q3a_bg_stats; //debug
    afd_stats->yuv_stats.p_q3a_rs_stats = private->hdr_stats_sync.p_q3a_flicker_stats;//debug
    afd_msg->u.stats = afd_stats;
    afd_thread_en_q_msg(private->thread_data, afd_msg);
  }
}

/** aec_port_handle_vhdr_buf:
 *    @port:  TODO
 *    @event: TODO
 *
 * TODO description
 *
 * Return nothing
 **/
static boolean afd_port_handle_flicker_buf(afd_port_private_t *private, isp_buf_divert_t *flicker_stats_buff )
{
#if 0
  boolean rc = TRUE;
  uint32_t *afd_stats = (uint32_t *)calloc(1, sizeof(uint32_t) * 96 * 4);
  afd_port_parse_RDI_stats(private, (uint32_t *)afd_stats, flicker_stats_buff->vaddr);
  return rc;
#else
  boolean rc = TRUE;
  afd_thread_msg_t *afd_msg   =
    (afd_thread_msg_t *)malloc(sizeof(afd_thread_msg_t));
  if (afd_msg == NULL) {
    return FALSE;
  }
  STATS_MEMSET(afd_msg, 0 , sizeof(afd_thread_msg_t));
  stats_t * afd_stats = (stats_t *)calloc(1, sizeof(stats_t));
  if(afd_stats == NULL) {
    free(afd_msg);
    afd_msg = NULL;
    return FALSE;
  }

  afd_stats->yuv_stats.p_q3a_rs_stats = (q3a_rs_stats_t *)calloc(1, sizeof(q3a_rs_stats_t));
  if(afd_stats->yuv_stats.p_q3a_rs_stats == NULL) {
    free(afd_stats);
    free(afd_msg);
    afd_msg = NULL;
    return FALSE;
  }

  afd_stats->frame_id = flicker_stats_buff->buffer.sequence;
  afd_stats->stats_type_mask |= STATS_RS;
  afd_stats->yuv_stats.p_q3a_rs_stats->num_row_sum = 96 * 4;
  afd_stats->yuv_stats.p_q3a_rs_stats->num_h_regions = 4;
  afd_port_parse_RDI_stats(private, (uint32_t *)afd_stats->yuv_stats.p_q3a_rs_stats->row_sum,
    flicker_stats_buff->vaddr);
#endif

  private->hdr_stats_sync.flicker_stats_frame_id = afd_stats->frame_id;
  private->hdr_stats_sync.stats_type_mask |= STATS_RS;
  STATS_MEMCPY(private->hdr_stats_sync.p_q3a_flicker_stats, sizeof(q3a_rs_stats_t),
     afd_stats->yuv_stats.p_q3a_rs_stats, sizeof(q3a_rs_stats_t));
  afd_hdr_stats_send_event(private);
  return rc;
}

/* determin if AEC is currently using the divert stats */
boolean afd_port_using_HDR_divert_stats(afd_port_private_t *private)
{
  if (!private) {
    return FALSE;
  }

  return (private->is_in_sensor_HDR);
}


/** afd_port_proc_downstream_event:
 *    @port : afd port pointer.
 *    @event: mct event for afd module
 *
 *  Send downstream event to afd algorithm
 **/
static boolean afd_port_proc_downstream_event(mct_port_t *port,
  mct_event_t *event)
{
  boolean            rc = TRUE;
  afd_port_private_t *private = (afd_port_private_t *)(port->port_private);
  mct_event_module_t *mod_evt = &(event->u.module_event);

  switch (mod_evt->type) {
  case MCT_EVENT_MODULE_SET_CHROMATIX_WRP: {
    chromatix_3a_parms_wrapper *chromatix_wrapper = (chromatix_3a_parms_wrapper *)mod_evt->module_event_data;
    afd_thread_msg_t *afd_msg =
      (afd_thread_msg_t *)malloc(sizeof(afd_thread_msg_t));
    if (afd_msg == NULL) {
      return FALSE;
    }
    STATS_MEMSET(afd_msg, 0, sizeof(afd_thread_msg_t));

    afd_msg->type = MSG_AFD_SET;
    afd_msg->u.afd_set_parm.type = AFD_SET_PARAM_INIT_CHROMATIX;
    afd_msg->u.afd_set_parm.u.init_param.chromatix =  chromatix_wrapper;
    afd_port_fill_extended_params(
      &afd_msg->u.afd_set_parm.u.init_param.extended_params);
    rc = afd_thread_en_q_msg(private->thread_data, afd_msg);
  } /* case MCT_EVENT_MODULE_SET_CHROMATIX_PTR */
    break;
  case MCT_EVENT_MODULE_START_STOP_STATS_THREADS: {
    uint8_t *start_flag = (uint8_t*)(mod_evt->module_event_data);
    AFD_LOW("MCT_EVENT_MODULE_START_STOP_STATS_THREADS start_flag: %d",
      *start_flag);

    if (*start_flag) {
      if (afd_port_start_threads(port) == FALSE) {
        AFD_LOW("afd thread start failed");
        rc = FALSE;
      }
    } else {
      if (private->thread_data) {
        afd_thread_stop(private->thread_data);
      }
    }
  }
    break;
  case MCT_EVENT_MODULE_SET_STREAM_CONFIG: {
    afd_thread_msg_t *afd_msg =
      (afd_thread_msg_t *)calloc(1, sizeof(afd_thread_msg_t));
    if (afd_msg == NULL) {
      return FALSE;
    }

    sensor_out_info_t *sensor_info =
      (sensor_out_info_t *)(mod_evt->module_event_data);

    private->parse_RDI_stats =
        (sensor_RDI_parser_func_t)sensor_info->parse_RDI_statistics;

    afd_msg->type = MSG_AFD_SET;
    afd_msg->u.afd_set_parm.type = AFD_SET_SENSOR_PARAM;
    afd_msg->u.afd_set_parm.u.aec_af_data.max_sensor_preview_fps =
      sensor_info->vt_pixel_clk /
      (sensor_info->fl_lines * sensor_info->ll_pck)
      * 0x00000100;
    afd_msg->u.afd_set_parm.u.aec_af_data.max_preview_fps =
      sensor_info->max_fps * 0x00000100;
    afd_msg->u.afd_set_parm.u.aec_af_data.preview_fps =
      sensor_info->max_fps * 0x00000100;
    afd_msg->u.afd_set_parm.u.aec_af_data.preview_linesPerFrame =
      sensor_info->fl_lines;
    afd_msg->u.afd_set_parm.u.aec_af_data.sen_dim_height =
      sensor_info->dim_output.height;
    rc = afd_thread_en_q_msg(private->thread_data, afd_msg);
  }/* MCT_EVENT_MODULE_SET_STREAM_CONFIG*/
    break;

  case MCT_EVENT_MODULE_STATS_AEC_UPDATE: {
    if(!afd_port_is_algo_active(port)) {
      return rc;
    }
    afd_thread_msg_t *afd_msg   =
        (afd_thread_msg_t *)malloc(sizeof(afd_thread_msg_t));
    if (afd_msg == NULL) {
      return FALSE;
    }
    STATS_MEMSET(afd_msg, 0, sizeof(afd_thread_msg_t));

    stats_update_t *stats_update =
      (stats_update_t *)mod_evt->module_event_data;

    if (!stats_update || stats_update->flag != STATS_UPDATE_AEC) {
      rc = FALSE;
      free(afd_msg);
      afd_msg = NULL;
      break;
    }

    afd_msg->type = MSG_AFD_SET;
    afd_msg->u.afd_set_parm.type = AFD_SET_AEC_PARAM;
    afd_msg->u.afd_set_parm.u.aec_af_data.aec_settled =
      stats_update->aec_update.settled;
    afd_msg->u.afd_set_parm.u.aec_af_data.band_50hz_gap =
      stats_update->aec_update.band_50hz_gap;
    afd_msg->u.afd_set_parm.u.aec_af_data.cur_line_cnt =
      stats_update->aec_update.linecount;
    afd_msg->u.afd_set_parm.u.aec_af_data.max_line_cnt =
      stats_update->aec_update.max_line_cnt;
    afd_msg->u.afd_set_parm.u.aec_af_data.exp_time =
      stats_update->aec_update.exp_time;
    afd_msg->u.afd_set_parm.u.aec_af_data.real_gain =
      stats_update->aec_update.real_gain;
    afd_msg->u.afd_set_parm.u.aec_af_data.lux_idx =
      stats_update->aec_update.lux_idx;



    switch (stats_update->aec_update.cur_atb) {
      case STATS_PROC_ANTIBANDING_OFF: {
        afd_msg->u.afd_set_parm.u.aec_af_data.aec_atb =
          AFD_TYPE_OFF;
      }
        break;
      case STATS_PROC_ANTIBANDING_60HZ: {
        afd_msg->u.afd_set_parm.u.aec_af_data.aec_atb =
          AFD_TYPE_60HZ;
      }
        break;
      case STATS_PROC_ANTIBANDING_50HZ: {
        afd_msg->u.afd_set_parm.u.aec_af_data.aec_atb =
          AFD_TYPE_50HZ;
      }
        break;
      default: {
        afd_msg->u.afd_set_parm.u.aec_af_data.aec_atb =
          AFD_TYPE_OFF;
      }
    }
    rc = afd_thread_en_q_msg(private->thread_data, afd_msg);
  }
    break;

  case MCT_EVENT_MODULE_STATS_EXT_DATA: {
    boolean rc = FALSE;
    mct_event_stats_ext_t *stats_ext_event;
    mct_event_stats_isp_t *stats_event ;
    stats_ext_event = (mct_event_stats_ext_t *)(mod_evt->module_event_data);
    stats_event = stats_ext_event->stats_data;

    if(!afd_port_is_algo_active(port)) {
      AFD_LOW("AFD_Stats-[mode -%d role-%d] Not being handled",
        private->dual_cam_info.mode,
        private->dual_cam_info.cam_role);
      return TRUE;
    }

    /* copy bg stats only*/
    if((stats_event != NULL) && stats_event->stats_mask & (1 << MSM_ISP_STATS_BG) &&
        afd_port_using_HDR_divert_stats(private)) {
      private->hdr_stats_sync.bg_stats_frame_id = stats_event->frame_id;
      private->hdr_stats_sync.stats_type_mask |= STATS_BG;
      STATS_MEMCPY(private->hdr_stats_sync.p_q3a_bg_stats, sizeof(q3a_bg_stats_t),
        stats_event->stats_data[MSM_ISP_STATS_BG].stats_buf, sizeof(q3a_bg_stats_t));
      afd_hdr_stats_send_event(private);
    }

    if(afd_port_using_HDR_divert_stats(private)) {
      return TRUE;
    }
    if (stats_event) {
      /* Need both */
      if (!((stats_event->stats_mask & (1 << MSM_ISP_STATS_RS)) &&
        (stats_event->stats_mask & (1 << MSM_ISP_STATS_BG)))) {
        return TRUE;
      }
      afd_thread_msg_t *afd_msg   =
        (afd_thread_msg_t *)malloc(sizeof(afd_thread_msg_t));
      if (afd_msg == NULL) {
        return FALSE;
      }
      STATS_MEMSET(afd_msg, 0 , sizeof(afd_thread_msg_t));
      stats_t * afd_stats = (stats_t *)calloc(1, sizeof(stats_t));
      if(afd_stats == NULL) {
        free(afd_msg);
        afd_msg = NULL;
        break;
      }

      afd_stats->frame_id = stats_event->frame_id;
      if (stats_event->stats_mask & (1 << MSM_ISP_STATS_RS)) {
        afd_msg->u.stats = afd_stats;
        afd_msg->type = MSG_AFD_STATS;
        afd_stats->stats_type_mask |= STATS_RS;
        afd_stats->yuv_stats.p_q3a_rs_stats =
          stats_event->stats_data[MSM_ISP_STATS_RS].stats_buf;
        rc = TRUE;
      }

      if (rc && stats_event->stats_mask & (1 << MSM_ISP_STATS_BG)) {
        afd_stats->stats_type_mask |= STATS_BG;
        afd_stats->bayer_stats.p_q3a_bg_stats =
          stats_event->stats_data[MSM_ISP_STATS_BG].stats_buf;
      }
      afd_stats->ack_data = stats_ext_event;
      circular_stats_data_use(stats_ext_event);

      rc = afd_thread_en_q_msg(private->thread_data, afd_msg);
      if (rc == FALSE) {
        circular_stats_data_done(stats_ext_event, 0, 0, 0);
        /* stats msg and payload will be free'd from inside enq_msg call */
      }
    } /* if(stats_event)*/
  } /* MCT_EVENT_MODULE_STATS_DATA*/
   break;

  case MCT_EVENT_MODULE_ISP_DIVERT_TO_3A:
  case MCT_EVENT_MODULE_BUF_DIVERT: {
    isp_buf_divert_t        *stats_buff =
      (isp_buf_divert_t *)mod_evt->module_event_data;
    if(stats_buff->stats_type != FLICKER_STATS){
      /* Only AFD module only handles FLICKER buffer */
      break;
    }

    afd_port_handle_flicker_buf(private, stats_buff);
    /*Since this buffer is consumed in same thread context,
        * piggy back  buffer by setting ack_flag to TRUE*/
    stats_buff->ack_flag = TRUE;
    stats_buff->is_buf_dirty = TRUE;
  }
    break;

   case MCT_EVENT_MODULE_REQUEST_STATS_TYPE: {
     mct_event_request_stats_type *stats_info =
      (mct_event_request_stats_type *)mod_evt->module_event_data;

     if (ISP_STREAMING_OFFLINE == stats_info->isp_streaming_type) {
       AFD_HIGH("AFD doesn't support offline processing yet. Returning.");
       break;
     }

    if (stats_info->supported_stats_mask & (1 << MSM_ISP_STATS_RS)) {
       stats_info->enable_stats_mask |= (1 << MSM_ISP_STATS_RS);
       stats_info->enable_stats_parse_mask |= (1 << MSM_ISP_STATS_RS);
       stats_info->stats_comp_grp_mask[STATS_GROUP_0] |= (1 << MSM_ISP_STATS_RS);
     }

    if (stats_info->supported_stats_mask & (1 << MSM_ISP_STATS_BG)) {
      stats_info->enable_stats_mask |= (1 << MSM_ISP_STATS_BG);
      stats_info->enable_stats_parse_mask |= (1 << MSM_ISP_STATS_BG);
      stats_info->stats_comp_grp_mask[STATS_GROUP_0] |= (1 << MSM_ISP_STATS_BG);
    }
  }
    break;

  case MCT_EVENT_MODULE_ISP_STATS_INFO: {
    rc = FALSE;
    mct_stats_info_t *stats_info =
      (mct_stats_info_t *)mod_evt->module_event_data;
    if (NULL == stats_info) {
      AFD_ERR("error: NULL event_data");
      break;
    }
    afd_thread_msg_t *afd_msg =
      (afd_thread_msg_t *)calloc(1, sizeof(afd_thread_msg_t));
    if (NULL == afd_msg) {
      AFD_ERR("malloc failed for stats_msg");
      break;
    }

    afd_msg->type = MSG_AFD_SET;
    afd_msg->u.afd_set_parm.type = AFD_SET_PARAM_STATS_DEPTH;
    afd_msg->u.afd_set_parm.u.stats_depth = stats_info->stats_depth;
    rc = afd_thread_en_q_msg(private->thread_data, afd_msg);
  }
    break;

  case MCT_EVENT_MODULE_STATS_AF_UPDATE: {
    stats_update_t *stats_update =
      (stats_update_t *)mod_evt->module_event_data;

    if(!afd_port_is_algo_active(port)) {
      return rc;
    }

    rc = FALSE;
    if (!stats_update || stats_update->flag != STATS_UPDATE_AF) {
      break;
    }

    afd_thread_msg_t *afd_msg =
      (afd_thread_msg_t *)calloc(1, sizeof(afd_thread_msg_t));
    if (NULL == afd_msg) {
      break;
    }

    afd_msg->type = MSG_AFD_SET;
    afd_msg->u.afd_set_parm.type = AFD_SET_AF_PARAM;
    afd_msg->u.afd_set_parm.u.aec_af_data.af_active =
      stats_update->af_update.af_active;
    afd_msg->u.afd_set_parm.u.aec_af_data.cont_af_enabled =
      stats_update->af_update.cont_af_enabled;
    rc = afd_thread_en_q_msg(private->thread_data, afd_msg);
   }
    break;

  default: {
  }
    break;
  } /* switch (mod_evt->type) */

  return rc;
}
/** afd_port_proc_fill_antibanding_parm
 *
 **/
static boolean afd_port_proc_fill_antibanding_parm(afd_thread_msg_t *afd_msg,
  cam_antibanding_mode_type mod)
{
  boolean rc = TRUE;

  switch (mod) {
    case  CAM_ANTIBANDING_MODE_OFF: {
      afd_msg->u.afd_set_parm.u.set_enable.afd_enable = FALSE;
    }
      break;
    case CAM_ANTIBANDING_MODE_60HZ: {
      afd_msg->u.afd_set_parm.u.set_enable.afd_enable = FALSE;
      afd_msg->u.afd_set_parm.u.set_enable.afd_mode = AFD_TYPE_60HZ;
    }
      break;
    case CAM_ANTIBANDING_MODE_50HZ: {
      afd_msg->u.afd_set_parm.u.set_enable.afd_enable = FALSE;
      afd_msg->u.afd_set_parm.u.set_enable.afd_mode = AFD_TYPE_50HZ;
    }
      break;
    case CAM_ANTIBANDING_MODE_AUTO: {
      afd_msg->u.afd_set_parm.u.set_enable.afd_enable = TRUE;
      afd_msg->u.afd_set_parm.u.set_enable.afd_mode = AFD_TYPE_AUTO;
    }
      break;
    case CAM_ANTIBANDING_MODE_AUTO_50HZ: {
      afd_msg->u.afd_set_parm.u.set_enable.afd_enable = TRUE;
      afd_msg->u.afd_set_parm.u.set_enable.afd_mode = AFD_TYPE_AUTO_50HZ;
    }
      break;
    case CAM_ANTIBANDING_MODE_AUTO_60HZ: {
      afd_msg->u.afd_set_parm.u.set_enable.afd_enable = TRUE;
      afd_msg->u.afd_set_parm.u.set_enable.afd_mode = AFD_TYPE_AUTO_60HZ;
    }
      break;
    default: {
      rc = FALSE;
    }
      break;
    } /* switch(mod)*/

  return rc;
}
/** afd_port_proc_downstream_ctrl
 *
 **/
static boolean afd_port_proc_downstream_ctrl(mct_port_t *port,
  mct_event_t *event)
{
  boolean             rc = TRUE;
  afd_port_private_t  *private  = (afd_port_private_t *)(port->port_private);
  mct_event_control_t *mod_ctrl = &(event->u.ctrl_event);

  switch (mod_ctrl->type) {
  case MCT_EVENT_CONTROL_SET_PARM: {
     stats_set_params_type *stat_parm =
       (stats_set_params_type *)mod_ctrl->control_event_data;
     afd_thread_msg_t *afd_msg =
       (afd_thread_msg_t *)malloc(sizeof(afd_thread_msg_t));
     if (afd_msg == NULL) {
       return FALSE;
     }
     STATS_MEMSET(afd_msg, 0, sizeof(afd_thread_msg_t));

     switch (stat_parm->param_type) {
     case STATS_SET_AFD_PARAM: {
       afd_msg->type                =  MSG_AFD_SET;
       afd_msg->u.afd_set_parm.type =  AFD_SET_ENABLE;
       rc = afd_port_proc_fill_antibanding_parm(afd_msg,
         stat_parm->u.afd_param);
       private->antibanding_mode = stat_parm->u.afd_param;
     }
       break;
     case STATS_SET_COMMON_PARAM: {
       stats_common_set_parameter_t *common_param =
         &(stat_parm->u.common_param);
       if (common_param->type == COMMON_SET_PARAM_STATS_DEBUG_MASK) {
         afd_msg->type = MSG_AFD_SET;
         afd_msg->u.afd_set_parm.type = AFD_SET_STATS_DEBUG_MASK;
         rc = TRUE;
       } else if (common_param->type == COMMON_SET_PARAM_STREAM_ON_OFF) {
        AFD_LOW("COMMON_SET_PARAM_STREAM_ON_OFF %d", common_param->u.stream_on);
        private->thread_data->no_stats_mode = !common_param->u.stream_on;

        // stream off, need to flush existing stats
        // send a sync msg here to flush the stats & other msg
        if (!common_param->u.stream_on) {
          afd_port_unlink_from_peer(port, event);
          afd_thread_msg_t afd_stats_msg;
          STATS_MEMSET(&afd_stats_msg, 0, sizeof(afd_thread_msg_t));
          afd_stats_msg.type = MSG_AFD_STATS_MODE;
          afd_stats_msg.sync_flag = TRUE;
          afd_thread_en_q_msg(private->thread_data, &afd_stats_msg);
          AFD_LOW("COMMON_SET_PARAM_STREAM_ON_OFF end");
        }
        // set rc to FALSE, no need to send again in the folling code
        rc = FALSE;
      }else if(common_param->type == COMMON_SET_PARAM_VIDEO_HDR){
        stats_common_set_parameter_t *common_param =
            &(stat_parm->u.common_param);
        if (common_param->u.video_hdr == CAM_SENSOR_3EXP_HDR_IN_SENSOR) {
          private->is_in_sensor_HDR = TRUE;
        }else {
          private->is_in_sensor_HDR = FALSE;
        }
        AFD_LOW("is_in_sensor_HDR = %d", private->is_in_sensor_HDR);
        afd_msg->type = MSG_AFD_SET;
        afd_msg->u.afd_set_parm.type = AFD_SET_PARAM_3HDR_ENABLE;
        afd_msg->u.afd_set_parm.u.HDR_enable = private->is_in_sensor_HDR;
        rc = TRUE;
      }else if(common_param->type == COMMON_SET_PARAM_SNAPSHOT_HDR) {
        stats_common_set_parameter_t *common_param =
            &(stat_parm->u.common_param);
        if (common_param->u.snapshot_hdr == CAM_SENSOR_3EXP_HDR_IN_SENSOR) {
          private->is_in_sensor_HDR = TRUE;
        }else {
          private->is_in_sensor_HDR = FALSE;
        }
        AFD_LOW("is_in_sensor_HDR = %d", private->is_in_sensor_HDR);
        afd_msg->type = MSG_AFD_SET;
        afd_msg->u.afd_set_parm.type = AFD_SET_PARAM_3HDR_ENABLE;
        afd_msg->u.afd_set_parm.u.HDR_enable = private->is_in_sensor_HDR;
        rc = TRUE;
      }else {
        rc = FALSE;
      }
     }
       break;

     default: {
     }
       break;
     }

     if (rc == TRUE) {
       rc = afd_thread_en_q_msg(private->thread_data, afd_msg);
     } else {
       free(afd_msg);
       afd_msg = NULL;
     }
     break;
  } /* case MCT_EVENT_CONTROL_SET_PARM*/

  case MCT_EVENT_CONTROL_SOF: {
    mct_bus_msg_isp_sof_t *sof_event;
    sof_event =(mct_bus_msg_isp_sof_t *)(mod_ctrl->control_event_data);
    afd_thread_msg_t *afd_msg =
    (afd_thread_msg_t *)malloc(sizeof(afd_thread_msg_t));
    if (afd_msg == NULL)
      return FALSE;
    STATS_MEMSET(afd_msg, 0, sizeof(afd_thread_msg_t));
    afd_msg->type                =  MSG_AFD_SET;
    afd_msg->u.afd_set_parm.type =  AFD_SET_SOF;
    afd_msg->u.afd_set_parm.u.set_sof_id = sof_event->frame_id;
    rc = afd_thread_en_q_msg(private->thread_data, afd_msg);
    afd_send_batch_bus_message(port, sof_event->frame_id);
    afd_port_forward_bus_message_if_linked(port);
  }
    break;
  case MCT_EVENT_CONTROL_STREAMON: {
    afd_thread_msg_t *afd_msg =
      (afd_thread_msg_t *)malloc(sizeof(afd_thread_msg_t));
    if (afd_msg == NULL)
      return FALSE;
    STATS_MEMSET(afd_msg, 0, sizeof(afd_thread_msg_t));
    /* Asynchronous Get*/
    afd_msg->type                =  MSG_AFD_GET;
    afd_msg->u.afd_get_parm.type =  AFD_GET_STATS_CONFIG;
    afd_msg->sync_flag = TRUE;
    rc = afd_thread_en_q_msg(private->thread_data, afd_msg);
    free(afd_msg);

    private->hdr_stats_sync.flicker_stats_frame_id = 0;
    private->hdr_stats_sync.bg_stats_frame_id = 0;
  }
    break;

  case MCT_EVENT_CONTROL_MASTER_INFO: {
     afd_port_handle_role_switch(port,event);
   }
    break;

  case MCT_EVENT_CONTROL_LINK_INTRA_SESSION: {
    afd_port_link_to_peer(port,event);
  }
    break;
  case MCT_EVENT_CONTROL_UNLINK_INTRA_SESSION: {
    afd_port_unlink_from_peer(port,event);
  }
    break;
  case MCT_EVENT_CONTROL_HW_WAKEUP:
  case MCT_EVENT_CONTROL_HW_SLEEP: {
    afd_port_update_LPM(port,
      (cam_dual_camera_perf_control_t*)event->u.ctrl_event.control_event_data);
  }
    break;
  case MCT_EVENT_CONTROL_STREAMOFF: {
  }
    break;
  default: {
  }
    break;
  }
  return rc;
}

/** afd_port_event
 *    @port:  this port from where the event should go
 *    @event: event object to send upstream or downstream
 *
 *  Because AFD module works no more than a sink module,
 *  hence its upstream event will need a little bit processing.
 *
 *  Return TRUE for successful event processing.
 **/
static boolean afd_port_event(mct_port_t *port, mct_event_t *event)
{
  boolean rc = FALSE;
  afd_port_private_t *private;
  /* sanity check */
  if (!port || !event) {
    return FALSE;
  }

  private = (afd_port_private_t *)(port->port_private);
  if (!private) {
    return FALSE;
  }

  /* sanity check: ensure event is meant for port with same identity*/
  if ((private->reserved_id & 0xFFFF0000) != (event->identity & 0xFFFF0000)) {
    return FALSE;
  }

  switch (MCT_EVENT_DIRECTION(event)) {
  case MCT_EVENT_DOWNSTREAM: {

    switch (event->type) {
    case MCT_EVENT_MODULE_EVENT: {
      rc= afd_port_proc_downstream_event(port, event);
    } /* case MCT_EVENT_MODULE_EVENT */
      break;

    case MCT_EVENT_CONTROL_CMD: {
      rc = afd_port_proc_downstream_ctrl(port,event);
    }
      break;

    default: {
    }
      break;
    }
  } /* case MCT_EVENT_TYPE_DOWNSTREAM */
    break;

  case MCT_EVENT_UPSTREAM: {
    mct_port_t *peer = MCT_PORT_PEER(port);
    MCT_PORT_EVENT_FUNC(peer)(peer, event);
  }
    break;

  default: {
    rc = FALSE;
  }
    break;
  }

  return rc;
}

/** afd_port_set_caps
 *    @port: port object which the caps to be set;
 *    @caps: this port's capability.
 *
 *  Return TRUE if it is valid soruce port.
 *
 *  Function overwrites a ports capability.
 **/
static boolean afd_port_set_caps(mct_port_t *port, mct_port_caps_t *caps)
{
  if (strcmp(MCT_PORT_NAME(port), "afd_sink")) {
    AFD_ERR("Error");
    return FALSE;
  }

  port->caps = *caps;
  return TRUE;
}

/** afd_port_check_caps_reserve
 *    @port: this interface module's port;
 *    @peer_caps: the capability of peer port which wants to match
 *                interterface port;
 *    @stream_info:
 *
 *  Stats modules are pure s/w software modules, and every port can
 *  support one identity. If the identity is different, support
 *  can be provided via create a new port. Regardless source or
 *  sink port, once capabilities are matched,
 *  - If this port has not been used, it can be supported;
 *  - If the requested stream is in existing identity, return
 *    failure
 *  - If the requested stream belongs to a different session, the port
 *  can not be used.
 **/
static boolean afd_port_check_caps_reserve(mct_port_t *port, void *caps,
  void *s_info)
{
  boolean            rc = FALSE;
  mct_port_caps_t    *port_caps;
  afd_port_private_t *private;
  mct_stream_info_t *stream_info = (mct_stream_info_t *)s_info;

  if (!port || !caps || !stream_info ||
      strcmp(MCT_OBJECT_NAME(port), "afd_sink")) {
    return rc;
  }

  MCT_OBJECT_LOCK(port);

  port_caps = (mct_port_caps_t *)caps;
  if (port_caps->port_caps_type != MCT_PORT_CAPS_STATS) {
    rc = FALSE;
    goto reserve_done;
  }

  private = (afd_port_private_t *)port->port_private;
  switch (private->state) {
  case AFD_PORT_STATE_LINKED: {
    if ((private->reserved_id & 0xFFFF0000) ==
      (stream_info->identity & 0xFFFF0000)) {
      rc = TRUE;
    }
  }
    break;

  case AFD_PORT_STATE_CREATED:
  case AFD_PORT_STATE_UNRESERVED: {
    private->reserved_id = stream_info->identity;
    private->stream_type = stream_info->stream_type;
    private->state       = AFD_PORT_STATE_RESERVED;
    rc = TRUE;
  }
    break;

  case AFD_PORT_STATE_RESERVED: {
    if ((private->reserved_id & 0xFFFF0000) ==
      (stream_info->identity & 0xFFFF0000)) {
      rc = TRUE;
    }
  }
    break;

  default: {
    rc = FALSE;
  }
    break;
  }

reserve_done:
  MCT_OBJECT_UNLOCK(port);

  return rc;
}

/** afd_port_check_caps_unreserve
 *    @port: this port object to remove the session/stream;
 *    @identity: session+stream identity.
 *
 *    Return FALSE if the identity is not existing.
 *
 *  This function frees the identity from port's children list.
 **/
static boolean afd_port_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  afd_port_private_t *private;
  boolean            rc = FALSE;

  if (!port || strcmp(MCT_OBJECT_NAME(port), "afd_sink")) {
    return FALSE;
  }

  private = (afd_port_private_t *)port->port_private;
  if (!private) {
    return FALSE;
  }

  MCT_OBJECT_LOCK(port);
  if ((private->state == AFD_PORT_STATE_UNLINKED ||
    private->state == AFD_PORT_STATE_RESERVED ||
    private->state == AFD_PORT_STATE_LINKED) &&
    ((private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000))) {

    if (!MCT_OBJECT_REFCOUNT(port)) {
      private->state       = AFD_PORT_STATE_UNRESERVED;
      private->reserved_id = (private->reserved_id & 0xFFFF0000);
    }
    rc = TRUE;
  } else {
    rc = FALSE;
  }
  MCT_OBJECT_UNLOCK(port);
  return rc;
}

/** afd_port_ext_link
 *    @identity:  Identity of session/stream
 *    @port: SINK of AFD ports
 *    @peer: For AFD sink- peer is STATS sink port
 *
 *  Set AFD port's external peer port, which is STATS module's
 *  sink port.
 *
 *  Return TRUE on success.
 **/
static boolean afd_port_ext_link(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  boolean rc = FALSE, thread_init = FALSE;
  afd_port_private_t  *private;
  mct_event_t         event;

  if (strcmp(MCT_OBJECT_NAME(port), "afd_sink")) {
    return FALSE;
  }

  private = (afd_port_private_t *)port->port_private;
  if (!private) {
    return FALSE;
  }

  MCT_OBJECT_LOCK(port);
  switch (private->state) {
  case AFD_PORT_STATE_RESERVED:
  case AFD_PORT_STATE_UNLINKED:
    if ((private->reserved_id & 0xFFFF0000) != (identity & 0xFFFF0000)) {
      break;
    }
  /* No break. Fall through */
  case AFD_PORT_STATE_CREATED: {
    thread_init = TRUE;
    rc = TRUE;
  }
    break;

  case AFD_PORT_STATE_LINKED: {
    if ((private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000)) {
      rc = TRUE;
      thread_init = FALSE;
    }
  }
    break;

  default: {
  }
    break;
  }

  if (rc == TRUE) {

    if (thread_init == TRUE) {
      if (private->thread_data == NULL) {
        rc = FALSE;
        goto afd_ext_link_done;
      }
    }

    private->state = AFD_PORT_STATE_LINKED;
    MCT_PORT_PEER(port) = peer;
    MCT_OBJECT_REFCOUNT(port) += 1;
  }

afd_ext_link_done:
  MCT_OBJECT_UNLOCK(port);
  mct_port_add_child(identity, port);
  return rc;
}

/** afd_port_ext_unlink
 *
 *  @identity: Identity of session/stream
 *  @port: afd module's sink port
 *  @peer: peer of stats sink port
 *
 * This funtion unlink the peer ports of stats sink, src ports
 * and its peer submodule's port
 *
 **/
static void afd_port_ext_unlink(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  afd_port_private_t *private;

  if (!port || !peer || MCT_PORT_PEER(port) != peer)
    return;

  private = (afd_port_private_t *)port->port_private;
  if (!private) {
    return;
  }

  MCT_OBJECT_LOCK(port);
  if (private->state == AFD_PORT_STATE_LINKED &&
    ((private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000))) {
    MCT_OBJECT_REFCOUNT(port) -= 1;
    if (!MCT_OBJECT_REFCOUNT(port)) {
      AFD_LOW("afd_data=%p", private->thread_data);
      private->state = AFD_PORT_STATE_UNLINKED;
    }
  }
  MCT_OBJECT_UNLOCK(port);
  mct_port_remove_child(identity, port);

  return;
}

/** afd_port_find_identity
 *
 **/
boolean afd_port_find_identity(mct_port_t *port, unsigned int identity)
{
  afd_port_private_t *private;

  if (!port) {
    return FALSE;
  }

  if (strcmp(MCT_OBJECT_NAME(port), "afd_sink")) {
    return FALSE;
  }

  private = port->port_private;

  if (private) {
    return ((private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000) ?
      TRUE : FALSE);
  }

  return FALSE;
}

/** afd_port_deinit
 *    @port: afd sink port
 *
 *  de-initialize one AFD sink port
 *
 *  Return nothing
 **/
void afd_port_deinit(mct_port_t *port)
{
  afd_port_private_t *private;

  if (!port) {
    return;
  }

  if(strcmp(MCT_OBJECT_NAME(port), "afd_sink")) {
    return;
  }

  private = port->port_private;
  if (private == NULL) {
    return;
  }

  if(private->hdr_stats_sync.p_q3a_flicker_stats != NULL) {
    free(private->hdr_stats_sync.p_q3a_flicker_stats);
  }
  if(private->hdr_stats_sync.p_q3a_bg_stats != NULL) {
    free(private->hdr_stats_sync.p_q3a_bg_stats);
  }

  if (private) {
    afd_thread_deinit(port);
    afd_destroy(private->afd_object.afd);
    free(port->port_private);
    port->port_private = NULL;
  }
}

/** afd_port_init
 *    @port: port object to be initialized
 *
 *  Port initialization, use this function to overwrite
 *  default port methods and install capabilities. Stats
 *  module should have ONLY sink port.
 *
 *  Return TRUE on success.
 **/
boolean afd_port_init(mct_port_t *port, unsigned int identity)
{
  mct_port_caps_t    caps;
  afd_port_private_t *private;
  mct_list_t         *list;

  private = malloc(sizeof(afd_port_private_t));
  if (private == NULL) {
    return FALSE;
  }
  STATS_MEMSET(private, 0, sizeof(afd_port_private_t));

  /* initialize AFD object */
  AFD_INITIALIZE_LOCK(&private->afd_object);
  private->afd_object.set_parameters = afd_set_parameters;
  private->afd_object.get_parameters = afd_get_parameters;
  private->afd_object.process = afd_process;
  private->afd_object.afd_cb = afd_port_callback;
  private->afd_object.afd_stats_cb = afd_port_stats_done_callback;
  private->afd_object.afd = afd_init();
  if (private->afd_object.afd == NULL) {
    free(private);
    private = NULL;
    return FALSE;
  }

  private->reserved_id = identity;
  private->state       = AFD_PORT_STATE_CREATED;

  afd_port_reset_dual_cam_info(private);

  private->hdr_stats_sync.p_q3a_flicker_stats =
    (q3a_rs_stats_t *)malloc(sizeof(q3a_rs_stats_t));
  private->hdr_stats_sync.p_q3a_bg_stats =
    (q3a_bg_stats_t *)malloc(sizeof(q3a_bg_stats_t));

  port->port_private  = private;
  port->direction     = MCT_PORT_SINK;
  caps.port_caps_type = MCT_PORT_CAPS_STATS;
  caps.u.stats.flag   = MCT_PORT_CAP_STATS_CS_RS;

  afd_port_init_threads(port);
  mct_port_set_event_func(port, afd_port_event);
  mct_port_set_intra_event_func(port, afd_port_intra_event);
  mct_port_set_set_caps_func(port, afd_port_set_caps);
  mct_port_set_ext_link_func(port, afd_port_ext_link);
  mct_port_set_unlink_func(port, afd_port_ext_unlink);
  mct_port_set_check_caps_reserve_func(port, afd_port_check_caps_reserve);
  mct_port_set_check_caps_unreserve_func(port, afd_port_check_caps_unreserve);

  if (port->set_caps) {
    port->set_caps(port, &caps);
  }
  return TRUE;
}

/*Start of AFD Dual Camera Port handling*/

/* Summary of code changes:
  AFD can be in stand alone mode or in follow mode
  While in Stand alone, it may have obligation to send information
  to peer if its linked  and running Qualcomm code and sync is enabled
  While in follow mode, Algorithm wont be running, however
  AFD update and  status from peer needs to be stored and later
  used during Role Switch
  Low Power Modes may turn off sync
  Roles can be changed
*/

/** afd_port_link_to_peer:
 *    @port: MCT port data
 *    @event: module event received
 *
 * Links to peer AFD Port (Master/Slave)
 *
 * Return boolean
 **/
boolean afd_port_link_to_peer(mct_port_t *port,
                                 mct_event_t *event)
{
  boolean                                rc = FALSE;
  mct_port_t                            *this_port = port;
  mct_port_t                            *peer_port = NULL;
  cam_sync_related_sensors_event_info_t *link_param = NULL;
  uint32_t                               peer_identity = 0;
  afd_port_private_t  *private = (afd_port_private_t *)(port->port_private);

  link_param = (cam_sync_related_sensors_event_info_t *)
    (event->u.ctrl_event.control_event_data);
  peer_identity = link_param->related_sensor_session_id;

  rc = stats_util_get_peer_port(event, peer_identity,this_port,
    &peer_port);

  if (rc == FALSE) {
    AFD_ERR("AFDDualCam FAIL to Get Peer Port");
    return rc;
  }
  private->dual_cam_info.cam_role = link_param->cam_role;
  private->dual_cam_info.mode = link_param->mode;
  private->dual_cam_info.sync_3a_mode = link_param->sync_3a_config.sync_mode_stats;
  private->dual_cam_info.is_LPM_on = FALSE;
  private->dual_cam_info.intra_peer_id = peer_identity;
  afd_port_handle_role_switch(port,NULL);
  MCT_PORT_INTRALINKFUNC(this_port)(peer_identity, this_port, peer_port);
  AFD_HIGH("AFDDualCam-[role-%d mode-%d 3aSync-%d AuxSync-%d AuxUpdate-%d] Linking to session %x Success",
    private->dual_cam_info.cam_role,
    private->dual_cam_info.mode,
    private->dual_cam_info.sync_3a_mode,
    private->dual_cam_info.is_aux_sync_enabled,
    private->dual_cam_info.is_aux_update_enabled,
    peer_identity);
  return rc;
}

/** afd_port_unlink_from_peer:
 *    @port: MCT port data
 *    @event: module event received
 *
 * UnLinks from peer AFD Port (Master/Slave)
 *
 * Return boolean
 **/
boolean afd_port_unlink_from_peer(mct_port_t *port,
                                               mct_event_t *event)
{
  afd_port_private_t  *private = (afd_port_private_t *)(port->port_private);
  if(private->dual_cam_info.intra_peer_id){
    AFD_HIGH("AFDDualCam-[mode -%d role-%d] Unlink Started",
      private->dual_cam_info.mode,
      private->dual_cam_info.cam_role);
    afd_port_local_unlink(port,event);
    afd_port_remote_unlink(port);
    AFD_HIGH("AFDDualCam-[mode -%d role-%d] Unlink complete",
      private->dual_cam_info.mode,
      private->dual_cam_info.cam_role);
    afd_port_reset_dual_cam_info(private);
  }
  return TRUE;
}

/** afd_port_local_unlink:
 *    @port: MCT port data
 *    @event: module event received
 *
 * UnLinks from peer AFD Port locally (Master/Slave)
 *
 * Return boolean
 **/
boolean afd_port_local_unlink(mct_port_t *port,
                                        mct_event_t *event)
{
  boolean                                rc = FALSE;
  mct_port_t                            *this_port = port;
  mct_port_t                            *peer_port = NULL;
  cam_sync_related_sensors_event_info_t *link_param = NULL;
  afd_port_private_t                    *private = (afd_port_private_t *)(port->port_private);
  uint32_t                               peer_identity = private->dual_cam_info.intra_peer_id;

  if (peer_identity) {
    rc = stats_util_get_peer_port(event, peer_identity, this_port, &peer_port);
    if (rc == TRUE) {
      MCT_PORT_INTRAUNLINKFUNC(peer_port);
    }
    AFD_HIGH("AFDDualCam-[mode -%d role-%d] Unlinking from peer session",
      private->dual_cam_info.mode,
      private->dual_cam_info.cam_role);
  } else {
    rc = TRUE;
  }

  return rc;
}

/** afd_port_remote_unlink:
 *    @port: MCT port data
 *    @event: module event received
 *
 * Asks Peer to do unlink (Master/Slave)
 *
 * Return boolean
 **/
boolean afd_port_remote_unlink(mct_port_t *port)
{
  boolean                                rc = FALSE;
  mct_port_t                            *this_port = port;
  mct_port_t                            *peer_port = NULL;
  cam_sync_related_sensors_event_info_t *link_param = NULL;
  uint32_t                               peer_identity = 0;
  afd_port_private_t  *private = (afd_port_private_t *)(port->port_private);
  /* Forward the AFD update info to the slave session */
  AFD_HIGH("AFDDualCam-[mode -%d role-%d] Remote Unlink issued",
    private->dual_cam_info.mode,
    private->dual_cam_info.cam_role);
  rc = stats_util_post_intramode_event(port,
                                       private->dual_cam_info.intra_peer_id,
                                       MCT_EVENT_MODULE_STATS_PEER_UNLINK,
                                       NULL);
  return rc;
}

/** afd_port_remote_LPM:
 *    @port: MCT port data
 *    @event: module event received
 *
 * Informs peer that we have gone to LPM mode
 *
 * Return boolean
 **/
boolean afd_port_remote_LPM(mct_port_t *port,cam_dual_camera_perf_control_t* perf_ctrl)
{
  boolean                                rc = FALSE;
  mct_port_t                            *this_port = port;
  mct_port_t                            *peer_port = NULL;
  cam_sync_related_sensors_event_info_t *link_param = NULL;
  uint32_t                               peer_identity = 0;
  afd_port_private_t  *private = (afd_port_private_t *)(port->port_private);
  /* Forward the LPM to the Master session */
  AFD_HIGH("AFDDualCam-[mode -%d role-%d] Remote LPM issued",
    private->dual_cam_info.mode,
    private->dual_cam_info.cam_role);
  rc = stats_util_post_intramode_event(port,
                                       private->dual_cam_info.intra_peer_id,
                                       MCT_EVENT_MODULE_STATS_PEER_LPM_EVENT,
                                       perf_ctrl);
  return rc;
}

/** afd_port_LPM_from_peer:
 *    @port: MCT port data
 *    @event: module event received
 *
 * LPM from peer AFD Port (Master/Slave)
 *
 * Return boolean
 **/
boolean afd_port_LPM_from_peer(mct_port_t *port,
                                     cam_dual_camera_perf_control_t* perf_ctrl)
{
  afd_port_private_t  *private = (afd_port_private_t *)(port->port_private);
  if(private->dual_cam_info.is_LPM_on != perf_ctrl->enable) {
    private->dual_cam_info.is_LPM_on = perf_ctrl->enable;
    AFD_HIGH("AFDDualCam-[mode -%d role-%d] LPM Updated=%d",
      private->dual_cam_info.mode,
      private->dual_cam_info.cam_role,
      perf_ctrl->enable);
    afd_port_update_aux_sync_and_update(private);
  }
  return TRUE;
}

/** afd_port_handle_role_switch:
 *    @port: MCT port data
 *
 * Handles Role switch from Master to Slave and Vice versa (Master/Slave)
 *
 * Return boolean
 **/
boolean afd_port_handle_role_switch(mct_port_t *port,mct_event_t *event)
{
  mct_port_t                            *this_port = port;
  afd_port_private_t  *private = (afd_port_private_t *)(port->port_private);
  if (event) {
    cam_dual_camera_master_info_t *dual_master_info = (cam_dual_camera_master_info_t *)
      (event->u.ctrl_event.control_event_data);
    if (dual_master_info){
      private->dual_cam_info.mode = dual_master_info->mode;
    }
  }
  afd_port_update_aux_sync_and_update(private);
  afd_port_set_role_switch(private);
  /*Need to inform Algorithm  here: TO DO*/
  AFD_HIGH("AFDDualCam-[mode -%d role-%d] Role Switch",
    private->dual_cam_info.mode,
    private->dual_cam_info.cam_role);
  return TRUE;
}

/** afd_port_set_role_switch
 *    @private:   Private data of the port
 *
 * Return: TRUE if no error
 **/
static boolean afd_port_set_role_switch(afd_port_private_t  *private)
{
  afd_thread_msg_t *afd_msg =
    (afd_thread_msg_t *)malloc(sizeof(afd_thread_msg_t));
  if (afd_msg == NULL) {
    return FALSE;
  }
  memset(afd_msg, 0, sizeof(afd_thread_msg_t));
  memset(&afd_msg->u.afd_set_parm.u.role_switch, 0, sizeof(afd_role_switch_params_t));
  if (private->dual_cam_info.mode == CAM_MODE_PRIMARY &&
    private->dual_cam_info.peer_stats_update.validUpdate ) {
    afd_msg->u.afd_set_parm.u.role_switch.master = TRUE;
    switch (private->dual_cam_info.peer_stats_update.stats_update.afd_update.afd_atb) {
      case AFD_TBL_60HZ:{
        afd_msg->u.afd_set_parm.u.role_switch.afd_type = AFD_TYPE_60HZ;
      }
        break;
      case AFD_TBL_50HZ: {
        afd_msg->u.afd_set_parm.u.role_switch.afd_type = AFD_TYPE_50HZ;
      }
        break;
      default: {
        afd_msg->u.afd_set_parm.u.role_switch.afd_type = AFD_TYPE_OFF;
      }
        break;
    }
    afd_msg->u.afd_set_parm.u.role_switch.afd_enable =
      private->dual_cam_info.peer_stats_update.stats_update.afd_update.afd_enable;
    afd_msg->u.afd_set_parm.u.role_switch.afd_exec_once =
      private->dual_cam_info.peer_stats_update.stats_update.afd_update.afd_exec_once;
    afd_msg->u.afd_set_parm.u.role_switch.afd_monitor =
      private->dual_cam_info.peer_stats_update.stats_update.afd_update.afd_monitor;
  } else {
    afd_msg->u.afd_set_parm.u.role_switch.afd_type = AFD_TYPE_MAX;
  }
  AFD_HIGH("AFDDualCam-[mode -%d role-%d] Role Switch Master=%d Informed AFD Type=%d Enable=%d ExecOnce=%d MOnitor=%d",
    private->dual_cam_info.mode,
    private->dual_cam_info.cam_role,
    afd_msg->u.afd_set_parm.u.role_switch.master,
    afd_msg->u.afd_set_parm.u.role_switch.afd_type,
    afd_msg->u.afd_set_parm.u.role_switch.afd_enable,
    afd_msg->u.afd_set_parm.u.role_switch.afd_exec_once,
    afd_msg->u.afd_set_parm.u.role_switch.afd_monitor);

  afd_msg->type = MSG_AFD_SET;
  afd_msg->u.afd_set_parm.type = AFD_SET_PARAM_ROLE_SWITCH;
  return afd_thread_en_q_msg(private->thread_data, afd_msg);

}


/** afd_port_reset_dual_cam_info:
 *    @port: AFD Private  port data
 *
 * Resets Dual Cam Info (Master/Slave)
 *
 * Return boolean
 **/
boolean afd_port_reset_dual_cam_info(afd_port_private_t  *port)
{
  AFD_HIGH("AFDDualCam-[mode -%d role-%d] Reset",
      port->dual_cam_info.mode,
      port->dual_cam_info.cam_role);
  port->dual_cam_info.mode = CAM_MODE_PRIMARY;
  port->dual_cam_info.sync_3a_mode = CAM_3A_SYNC_NONE;
  port->dual_cam_info.is_LPM_on = FALSE;
  port->dual_cam_info.is_aux_sync_enabled = FALSE;
  port->dual_cam_info.is_aux_update_enabled = FALSE;
  port->dual_cam_info.is_algo_active = TRUE;
  port->dual_cam_info.intra_peer_id = 0;
  port->dual_cam_info.peer_stats_update.validUpdate = FALSE;
  return TRUE;
}

/** afd_port_if_stats_can_be_configured:
 *    @port: AFD Private  port data
 *
 * Checks if Stats can be configured (Master/Slave)
 *
 * Return boolean
 **/
boolean afd_port_if_stats_can_be_configured(afd_port_private_t  *port)
{
  boolean config = TRUE;

  /*We dont want to configure stats if we are in Follow and Mono mode*/
  if ((port->dual_cam_info.sync_3a_mode == CAM_3A_SYNC_FOLLOW) &&
      (port->dual_cam_info.mode == CAM_MODE_SECONDARY) &&
      (port->dual_cam_info.cam_role == CAM_ROLE_MONO)) {
      AFD_HIGH("AFDDualCam-[mode -%d role-%d] Stats cannot be configured",
        port->dual_cam_info.mode,
        port->dual_cam_info.cam_role);
      config = FALSE;
  }
  return config;
}

/** afd_port_update_aux_sync_and_update:
 *    @port: AFD Private  port data
 *
 * Updates Aux sync and update conditions (Master/Slave)
 *
 * Return boolean
 **/
boolean afd_port_update_aux_sync_and_update(afd_port_private_t  *port)
{
  boolean is_aux_sync_enabled = FALSE;
  boolean is_aux_update_enabled = FALSE;
  boolean is_algo_active = TRUE;

  if(port->dual_cam_info.is_LPM_on) {
    port->dual_cam_info.is_aux_sync_enabled = FALSE;
    port->dual_cam_info.is_aux_update_enabled = FALSE;
    AFD_HIGH("AFDDualCam-[mode -%d role-%d] Aux Sync Enabled=%d Aux Update Enabled=%d",
      port->dual_cam_info.mode,
      port->dual_cam_info.cam_role,
      is_aux_sync_enabled,
      is_aux_update_enabled);
    return TRUE;
  }

  if ((port->dual_cam_info.is_LPM_on == FALSE)
    && (port->dual_cam_info.sync_3a_mode == CAM_3A_SYNC_FOLLOW)
    && (port->dual_cam_info.mode == CAM_MODE_PRIMARY)
    && (port->dual_cam_info.intra_peer_id != 0)) {
      is_aux_sync_enabled = TRUE;
  }

  if ((port->dual_cam_info.is_LPM_on == FALSE)
      &&(port->dual_cam_info.sync_3a_mode == CAM_3A_SYNC_FOLLOW)
      && (port->dual_cam_info.mode == CAM_MODE_SECONDARY)
      && (port->dual_cam_info.intra_peer_id != 0)) {
     is_aux_update_enabled = TRUE;
  }

  if((port->dual_cam_info.sync_3a_mode == CAM_3A_SYNC_FOLLOW)
      && (port->dual_cam_info.mode == CAM_MODE_SECONDARY)) {
      is_algo_active = FALSE;
  }

  if(is_algo_active != port->dual_cam_info.is_algo_active) {
    AFD_HIGH("AFDDualCam-[mode -%d role-%d] is_algo_active=%d",
      port->dual_cam_info.mode,
      port->dual_cam_info.cam_role,
      is_algo_active);
    port->dual_cam_info.is_algo_active = is_algo_active;
  }

  if(is_aux_sync_enabled != port->dual_cam_info.is_aux_sync_enabled) {
    AFD_HIGH("AFDDualCam-[mode -%d role-%d] Aux Sync Enabled=%d",
      port->dual_cam_info.mode,
      port->dual_cam_info.cam_role,
      is_aux_sync_enabled);
    port->dual_cam_info.is_aux_sync_enabled = is_aux_sync_enabled;
  }

  if(is_aux_update_enabled != port->dual_cam_info.is_aux_update_enabled) {
    AFD_HIGH("AFDDualCam-[mode -%d role-%d] Aux Update Enabled=%d",
      port->dual_cam_info.mode,
      port->dual_cam_info.cam_role,
      is_aux_update_enabled);
    port->dual_cam_info.is_aux_update_enabled = is_aux_update_enabled;
  }
  return TRUE;
}

/** afd_port_update_LPM:
 *    @port:  Private  port data
 *    @LPM: Low Power Mode
 *
 * Update LPM mode  (Master/Slave)
 *
 * Return boolean
 **/
static boolean afd_port_update_LPM(mct_port_t *port, cam_dual_camera_perf_control_t* perf_ctrl)
{
  boolean                                rc = FALSE;
  afd_port_private_t  *private = (afd_port_private_t *)(port->port_private);
  if(private->dual_cam_info.is_LPM_on != perf_ctrl->enable) {
    private->dual_cam_info.is_LPM_on = perf_ctrl->enable;
    AFD_HIGH("AFDDualCam-[mode -%d role-%d] LPM Updated=%d",
      private->dual_cam_info.mode,
      private->dual_cam_info.cam_role,
      perf_ctrl->enable);
    afd_port_update_aux_sync_and_update(private);
  }
  afd_port_remote_LPM(port,perf_ctrl);
  return TRUE;
}

/** afd_port_is_algo_active:
 *    @port:  Private  port data
 *
 * Is Algorithm Active  (Master/Slave)
 *
 * Return boolean
 **/
static boolean afd_port_is_algo_active(mct_port_t *port)
{
  afd_port_private_t  *private = (afd_port_private_t *)(port->port_private);
  return private->dual_cam_info.is_algo_active;
}


/** afd_port_forward_bus_message_if_linked:
 *    @port: MCT  port data
 *
 * Forward the Bus Message to the slave session if: dual camera is in use, the
 * cameras are linked, and this method is called from the master session (Master)
 *
 * Return boolean
 **/
boolean afd_port_forward_bus_message_if_linked(mct_port_t *mct_port)
{
  afd_port_private_t *private = (afd_port_private_t *)(mct_port->port_private);
  boolean result = true;

  /* Only forward the AFD BUS Message if Aux Sync is Enabled*/
  if (private->dual_cam_info.is_aux_sync_enabled)
  {
    mct_bus_msg_afd_t afd_regular;
    afd_regular.scene_flicker = private->meta_scene_flicker;
    afd_regular.antibanding_mode = private->antibanding_mode;
    AFD_LOW("AFDDualCam-[mode -%d role-%d] Bus message fwded",
      private->dual_cam_info.mode,
      private->dual_cam_info.cam_role);
    /* Forward the AFD BUS Message update info to the slave session */
    result = stats_util_post_intramode_event(mct_port,
                                             private->dual_cam_info.intra_peer_id,
                                             MCT_EVENT_MODULE_STATS_PEER_STATUS_EVENT,
                                             (void*)&afd_regular);
    if (!result)
    {
      AFD_MSG_ERROR("AFDDualCam-[mode -%d role-%d] Error! failed to forward the AFD BUS Message event to the slave (id=%d)",
        private->dual_cam_info.mode,
        private->dual_cam_info.cam_role,
        private->dual_cam_info.intra_peer_id );
    }
  }
  return result;
}

/** afd_port_forward_update_event_if_linked:
  *    @mct_port:   MCT port object
  *    @stats_update: AFD Status update
  *
  * Forward the AFD update event to the slave session if: dual camera is in use, the
  * cameras are linked, and this method is called from the master session (Master)
  *
  * Return: boolean value indicating success or failure
  */
boolean afd_port_forward_update_event_if_linked( mct_port_t* mct_port,stats_update_t* stats_update)
{
  afd_port_private_t* afd_port = (afd_port_private_t *)(mct_port->port_private);
  boolean result = true;

  /* Only forward the AFD update if Aux Sync is Enabled*/
  if (afd_port->dual_cam_info.is_aux_sync_enabled)
  {
    AFD_LOW("AFDDualCam-[mode -%d role-%d] Update message fwded",
      afd_port->dual_cam_info.mode,
      afd_port->dual_cam_info.cam_role);
    /* Forward the AFD update info to the slave session */
    result = stats_util_post_intramode_event(mct_port,
                                             afd_port->dual_cam_info.intra_peer_id,
                                             MCT_EVENT_MODULE_STATS_PEER_UPDATE_EVENT,
                                             (void*)stats_update);
    if (!result)
    {
      AFD_MSG_ERROR("AFDDualCam-[mode -%d role-%d] Error! failed to forward the AFD UPdate Message event to the slave (id=%d)",
        afd_port->dual_cam_info.cam_role,
        afd_port->dual_cam_info.intra_peer_id);
    }
  }
  return result;
}

/** afd_port_store_peer_bus_message:
 *    @port:       MCT port
 *    @afd_msg: Master Status message to be saved
 *
 * Return TRUE if the event is processed successfully.
 **/
boolean afd_port_store_peer_bus_message(mct_port_t *mct_port,mct_bus_msg_afd_t *afd_msg)
{
  afd_port_private_t *private = (afd_port_private_t *)(mct_port->port_private);
  Q3A_MEMCPY(&private->dual_cam_info.peer_stats_update.afd_msg,afd_msg,sizeof(mct_bus_msg_afd_t));
  AFD_LOW("AFDDualCam-[mode -%d role-%d] Storing Peer Bus Message",
    private->dual_cam_info.mode,
    private->dual_cam_info.cam_role);
  return TRUE;
}


/** afd_port_store_peer_update_event:
 *    @port:  MCT port
 *    @stats_update: Stats result from Master
 *
 * Return TRUE if the event is processed successfully.
 **/
boolean afd_port_store_peer_update_event(mct_port_t* mct_port, stats_update_t* stats_update)
{
  afd_port_private_t *private = (afd_port_private_t *)(mct_port->port_private);
  Q3A_MEMCPY(&private->dual_cam_info.peer_stats_update.stats_update,stats_update,sizeof(stats_update_t));
  private->dual_cam_info.peer_stats_update.validUpdate = TRUE;
  AFD_LOW("AFDDualCam-[mode -%d role-%d] Storing Peer Update Message ATB=%d",
    private->dual_cam_info.mode,
    private->dual_cam_info.cam_role,
    private->dual_cam_info.peer_stats_update.stats_update.afd_update.afd_atb);
  return TRUE;
}

/** afd_port_intra_event:
 *    @port:  MCT port
 *    @event: MCT module
 *
 * Handles the intra-module events sent between AFD master and slave sessions
 *
 * Return TRUE if the event is processed successfully.
 **/
boolean afd_port_intra_event(mct_port_t *port, mct_event_t *event)
{
  afd_port_private_t *private;

  /* sanity check */
  if (!port || !event) {
    return FALSE;
  }

  private = (afd_port_private_t *)(port->port_private);
  AFD_LOW("AFDDualCam-[mode -%d role-%d] Received AFD intra-module peer event",
    private->dual_cam_info.mode,
    private->dual_cam_info.cam_role);

  if (!private) {
    return FALSE;
  }

  /* sanity check: ensure event is meant for port with same identity*/
  if ((private->reserved_id & 0xFFFF0000) != (event->identity & 0xFFFF0000)) {
    return FALSE;
  }

  switch(event->u.module_event.type) {
    case MCT_EVENT_MODULE_STATS_PEER_UPDATE_EVENT:
      AFD_LOW("AFDDualCam-[mode -%d role-%d] Received MCT_EVENT_MODULE_STATS_PEER_UPDATE_EVENT",
        private->dual_cam_info.mode,
        private->dual_cam_info.cam_role);
      afd_port_store_peer_update_event(port,(stats_update_t*)event->u.module_event.module_event_data);
      break;
    case MCT_EVENT_MODULE_STATS_PEER_STATUS_EVENT:
      AFD_LOW("AFDDualCam-[mode -%d role-%d] Received MCT_EVENT_MODULE_STATS_PEER_STATUS_EVENT",
        private->dual_cam_info.mode,
        private->dual_cam_info.cam_role);
      afd_port_store_peer_bus_message(port,(mct_bus_msg_afd_t*)event->u.module_event.module_event_data);
      break;
    case MCT_EVENT_MODULE_STATS_PEER_UNLINK:
      AFD_LOW("AFDDualCam-[mode -%d role-%d] Received MCT_EVENT_MODULE_STATS_PEER_UNLINK",
        private->dual_cam_info.mode,
        private->dual_cam_info.cam_role);
      afd_port_local_unlink(port,event);
      afd_port_reset_dual_cam_info(private);
      break;
    case MCT_EVENT_MODULE_STATS_PEER_LPM_EVENT:
      AFD_LOW("AFDDualCam-[mode -%d role-%d] Received MCT_EVENT_MODULE_STATS_PEER_LPM_EVENT",
        private->dual_cam_info.mode,
        private->dual_cam_info.cam_role);
      afd_port_LPM_from_peer(port,
        (cam_dual_camera_perf_control_t*)event->u.ctrl_event.control_event_data);
      break;
    default:
      AFD_ERR("AFDDualCam-[mode -%d role-%d] Error! Received unknown intra-module event type: %d",
        private->dual_cam_info.mode,
        private->dual_cam_info.cam_role,
        event->u.module_event.type);
      break;
  }

  return TRUE;
}
