 /* isp_handler.c
 *
 * Copyright (c) 2012-2017,2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
/* std headers */
#include <stdio.h>

/* kernel headers */
#include <linux/videodev2.h>

/* mctl headers */
#include "media_controller.h"
#include "mct_object.h"
#include "mct_profiler.h"

/* hal headers */
#include "cam_types.h"

/* isp headers */
#include "isp_module.h"
#include "isp_log.h"
#include "isp_defs.h"
#include "isp_util.h"
#include "isp_handler.h"
#include "isp_trigger_thread.h"
#include "isp_hw_update_thread.h"
#include "isp_hw_update_util.h"
#include "isp_parser_thread.h"
#include "q3a_stats_hw.h"
#include "isp_stats_buf_mgr.h"
#include "isp_pipeline_reg.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_COMMON, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_COMMON, fmt, ##args)

/** isp_handler_clear_events_queues:
 *
 *  @module: mct module handle
 *  @session_param: session in use
 *
 *  Free hw update params, free parser params, invalidate saved events,
 *  Clear stats_config_update event in saved_events etc.
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_handler_clear_events_queues(mct_module_t *module,
  isp_session_param_t *session_param)
{
  boolean ret = TRUE;
  RETURN_IF_NULL(module);
  RETURN_IF_NULL(session_param);

  /* Post message to parser thread to free parser params */
  PTHREAD_MUTEX_LOCK(&session_param->parser_params.mutex);
  ret = isp_parser_thread_post_message(&session_param->parser_params,
    ISP_PARSER_THREAD_EVENT_FREE_QUEUE);
  if (ret == FALSE) {
    ISP_ERR("failed: ISP_PARSER_THREAD_EVENT_FREE_QUEUE");
  }
  /* Block caller to ensure parser thread queue is cleared */
  pthread_cond_wait(&session_param->parser_params.cond,
    &session_param->parser_params.mutex);
  PTHREAD_MUTEX_UNLOCK(&session_param->parser_params.mutex);

  /* Post message to trigger thread to invalidate saved events */
  PTHREAD_MUTEX_LOCK(&session_param->trigger_update_params.mutex);
  ret = isp_trigger_thread_post_message(&session_param->trigger_update_params,
    ISP_TRIGGER_THREAD_EVENT_CLEAR_EVENTS, 0, session_param);
  if (ret == FALSE) {
    ISP_ERR("failed: ISP_TRIGGER_THREAD_EVENT_CLEAR_EVENTS");
  }
  /* Block caller to ensure parser thread queue is cleared */
  pthread_cond_wait(&session_param->trigger_update_params.cond,
    &session_param->trigger_update_params.mutex);
  PTHREAD_MUTEX_UNLOCK(&session_param->trigger_update_params.mutex);

  /* Post message to hw update thread to free hw update params */
  PTHREAD_MUTEX_LOCK(&session_param->hw_update_params.mutex);
  ret = isp_hw_update_thread_post_message(&session_param->hw_update_params,
    ISP_HW_UPDATE_EVENT_FREE_HW_UPDATE_LIST, session_param);
  if (ret == FALSE) {
    ISP_ERR("failed: ISP_HW_UPDATE_EVENT_FREE_HW_UPDATE_LIST");
  }
  /* Block caller to ensure hw update thread queue is cleared */
  pthread_cond_wait(&session_param->hw_update_params.cond,
    &session_param->hw_update_params.mutex);
  PTHREAD_MUTEX_UNLOCK(&session_param->hw_update_params.mutex);

  isp_util_free_offline_shared_queue(session_param->offline_trigger_param_q);

  if (session_param->hw_update_params.metadata_dump_enable) {
     ret = isp_util_free_all_meta_dump_entry(module, session_param);
     if (ret == FALSE) {
       ISP_ERR("failed, isp_util_free_all_meta_dump_entry");
     }
  }
  ret = isp_util_free_dmi_meta_tbl(module, session_param);
  if (ret == FALSE) {
    ISP_ERR("failed, isp_util_free_dmi_meta_tbl");
  }

  ret = isp_util_free_per_frame_meta(module, session_param);
  if (ret == FALSE) {
    ISP_ERR("failed, isp_util_free_per_frame_meta");
  }

  ret = isp_util_clear_all_stored_metadata(session_param);
  if (ret == FALSE) {
    ISP_ERR("failed, isp_util_free_per_frame_meta");
  }

  session_param->curr_frame_id = 0;

  return ret;
}

/** isp_handler_raw_stats_divert:
 *
 *  @module: mct module handle
 *  @event: event to be handled
 *
 *  Handle raw stats notify event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_handler_raw_stats_divert(mct_module_t *module, mct_event_t *event)
{
  boolean              ret = TRUE;
  isp_session_param_t *session_param = NULL;

  if (!module || !event) {
    ISP_ERR("failed: module %p event %p", module, event);
    return FALSE;
  }

  ret = isp_util_get_session_params(module, ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_stream_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&session_param->state_mutex);
  if (session_param->state == ISP_STATE_IDLE) {
    PTHREAD_MUTEX_UNLOCK(&session_param->state_mutex);
    ISP_HIGH("isp is streamed off, piggy back stats buffer");
    iface_raw_stats_buf_info_t *raw_stats_info = NULL;
    raw_stats_info = (iface_raw_stats_buf_info_t *)
      event->u.module_event.module_event_data;
    if (!raw_stats_info) {
      ISP_ERR("failed: %p", raw_stats_info);
      return FALSE;
    }
    raw_stats_info->ack_flag = TRUE;
    return TRUE;
  } else {
    PTHREAD_MUTEX_UNLOCK(&session_param->state_mutex);
  }

  ret = isp_parser_thread_save_stats_nofity_event(module, session_param, event);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_parser_thread_copy_event");
  }

  return ret;
} /* isp_handler_raw_stats_divert */


/** isp_handler_sof:
 *
 *  @module: mct module handle
 *  @port: port on which this event arrived
 *  @event: SOF event to be handled
 *
 *  1. Save the settings applied in previous frame
 *  2. Signal trigger_udpate thread
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_handler_sof(mct_module_t *module, mct_port_t *port,
  mct_event_t *event)
{
  boolean                      ret = TRUE;
  isp_session_param_t         *session_param = NULL;
  isp_trigger_update_params_t *trigger_update_params = NULL;
  isp_hw_update_params_t      *hw_update_params = NULL;
  mct_bus_msg_isp_sof_t       *sof_event = NULL;
  boolean                      is_trigger_thread_busy,
                               is_hw_thread_busy = FALSE;
  isp_t                       *isp = NULL;
  isp_reg_update_state_t       reg_update_state;

  MCT_PROF_LOG_BEG(__func__);

  if (!module || !port || !event) {
    ISP_ERR("failed: %p %p %p", module, port, event);
    return FALSE;
  }

  ret = isp_util_get_session_params(module, ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_stream_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&session_param->state_mutex);
  reg_update_state = session_param->reg_update_info.reg_update_state;

  /* Ignore SOF after stream off */
  if (session_param->state == ISP_STATE_IDLE) {
    PTHREAD_MUTEX_UNLOCK(&session_param->state_mutex);
    isp_hw_update_util_request_reg_update(session_param);
    return TRUE;
  }
  PTHREAD_MUTEX_UNLOCK(&session_param->state_mutex);


  PTHREAD_MUTEX_LOCK(&session_param->thread_busy_mutex);
  is_hw_thread_busy = session_param->hw_update_params.is_thread_busy;
  is_trigger_thread_busy = session_param->trigger_update_params.is_thread_busy;
  PTHREAD_MUTEX_UNLOCK(&session_param->thread_busy_mutex);

  /* Signal trigger update thread */
  sof_event = (mct_bus_msg_isp_sof_t *)event->u.ctrl_event.control_event_data;
  if (!sof_event) {
    ISP_ERR("failed: sof_event %p", sof_event);
    return FALSE;
  }
  ISP_DBG(" E frame_id %u, svhdr_enable = %d", sof_event->frame_id, session_param->svhdr_enb);
  if (session_param->svhdr_enb) {
      ret = isp_trigger_thread_store_new_params_for_offline(session_param,
        sof_event->frame_id);
      if (ret == FALSE) {
        ISP_INFO("Warning: failed to isp_trigger_thread_store_new_params_for_offline");
      }
      return ret;
    }

  if (is_trigger_thread_busy == FALSE && is_hw_thread_busy == FALSE &&
    (reg_update_state == ISP_REG_UPDATE_STATE_NOT_REQUESTED ||
     reg_update_state == ISP_REG_UPDATE_STATE_RECEIVED)) {

    if (session_param->hw_update_params.skip_hw_update == FALSE) {
      ret = isp_util_update_hw_param(module, session_param);
      if (ret == FALSE) {
        ISP_ERR("failed: isp_util_update_hw_param");
      }
    } else {
      isp_hw_update_util_request_reg_update(session_param);
    }

    ret = isp_trigger_thread_handle_sof(&session_param->trigger_update_params,
             sof_event->frame_id, session_param);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_trigger_thread_handle_sof");
    }
  }

  MCT_PROF_LOG_END();

  return ret;
}
/** isp_handler_fe_sof:
 *
 *  @module: mct module handle
 *  @port: port on which this event arrived
 *  @event: SOF event to be handled
 *
 *  1. Save the settings applied in previous frame
 *  2. Signal trigger_udpate thread
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_handler_module_fe_sof(mct_module_t *module, mct_port_t *port,
  mct_event_t *event)
{
  boolean                      ret = TRUE;
  isp_session_param_t         *session_param = NULL;
  isp_hw_update_params_t      *hw_update_params = NULL;
  mct_bus_msg_isp_sof_t       *sof_event = NULL;
  boolean                      is_trigger_thread_busy,
                               is_hw_thread_busy = FALSE;
  isp_t                       *isp = NULL;

  MCT_PROF_LOG_BEG(__func__);

  if (!module || !port || !event) {
    ISP_ERR("failed: %p %p %p", module, port, event);
    return FALSE;
  }

  ret = isp_util_get_session_params(module, ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_stream_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&session_param->state_mutex);

  /* Ignore SOF after stream off */
  if (session_param->state == ISP_STATE_IDLE) {
    PTHREAD_MUTEX_UNLOCK(&session_param->state_mutex);
    isp_hw_update_util_request_reg_update(session_param);
    return TRUE;
  }
  PTHREAD_MUTEX_UNLOCK(&session_param->state_mutex);

  PTHREAD_MUTEX_LOCK(&session_param->thread_busy_mutex);
  is_hw_thread_busy = session_param->hw_update_params.is_thread_busy;
  is_trigger_thread_busy = session_param->trigger_update_params.is_thread_busy;
  PTHREAD_MUTEX_UNLOCK(&session_param->thread_busy_mutex);
  if (!session_param->svhdr_enb &&
      !session_param->binncorr_mode) {
      ISP_ERR("svhdr & binning not enabled");
      return FALSE;
    }
  /* Signal trigger update thread */

  if (is_trigger_thread_busy == FALSE && is_hw_thread_busy == FALSE) {

    if (session_param->hw_update_params.skip_hw_update == FALSE) {
      ret = isp_util_update_hw_param(module, session_param);
      if (ret == FALSE) {
        ISP_ERR("failed: isp_util_update_hw_param");
      }
    } else {
      isp_hw_update_util_request_reg_update(session_param);
    }

    sof_event = (mct_bus_msg_isp_sof_t *)event->u.ctrl_event.control_event_data;
    if (!sof_event) {
      ISP_ERR("failed: sof_event %p", sof_event);
      return FALSE;
    }
    ISP_DBG(" E frame_id %u", sof_event->frame_id);
    /* Update trigger update params and schedule trigger update thread
       if skip hw update, no trigger another new hw update list*/
    ret = isp_trigger_thread_apply_shared_trigger_params(
       sof_event->frame_id, session_param);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_trigger_thread_apply_shared_trigger_params");
    }
    /*apply cur_event which are copied from queue*/
    PTHREAD_MUTEX_LOCK(&session_param->trigger_update_params.mutex);
    ret = isp_trigger_thread_post_message(&session_param->trigger_update_params,
      ISP_TRIGGER_THREAD_EVENT_PROCESS_QUEUE, sof_event->frame_id, session_param);
    if (ret == FALSE) {
      ISP_ERR("failed: ISP_TRIGGER_THREAD_EVENT_PROCESS_QUEUE");
    }
    PTHREAD_MUTEX_UNLOCK(&session_param->trigger_update_params.mutex);
  }

  MCT_PROF_LOG_END();

  return ret;
}
/** isp_handler_control_sof:
 *
 *  @module: mct module handle
 *  @port: port on which this event arrived
 *  @event: SOF event to be handled
 *
 *  1) Forward event
 *  2) Meta data processing
 *     - Dump VFE registers if dump enabled
 *     - Prepare and send ISP meta dump for BET if enabled
 *     - Save frame metadata based on reporting queue
 *     - Report metadata
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_handler_control_sof(mct_module_t *module, mct_port_t *port,
  mct_event_t *event)
{
  boolean                      ret = TRUE;
  isp_session_param_t         *session_param = NULL;
  isp_trigger_update_params_t *trigger_update_params = NULL;
  isp_hw_update_params_t      *hw_update_params = NULL;
  isp_parser_params_t         *parser_params = NULL;
  isp_hw_id_t                  hw_id = 0;
  isp_hw_update_list_params_t *hw_update_list_params = NULL;
  mct_bus_msg_isp_sof_t       *sof_event = NULL;
  isp_t                       *isp = NULL;
  isp_zoom_params_t           *zoom_params_arr = NULL;
  isp_pipeline_t              *isp_pipeline = NULL;
  cam_zoom_info_t             *zoom = NULL;

  MCT_PROF_LOG_BEG(__func__);

  if (!module || !port || !event) {
    ISP_ERR("failed: %p %p %p", module, port, event);
    goto ERROR;
  }

  ret = isp_util_get_session_params(module, ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_stream_params ret %d %p", ret,
      session_param);
    goto ERROR;
  }

  isp = MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    goto ERROR;
  }

  sof_event = (mct_bus_msg_isp_sof_t *)event->u.ctrl_event.control_event_data;
  if (!sof_event) {
    ISP_ERR("failed: sof_event %p", sof_event);
    return FALSE;
  }
  ISP_DBG(" E %u", sof_event->frame_id);

  session_param->curr_frame_id = sof_event->frame_id;

  /* Forward event to downstream */
  ret = isp_util_forward_event(port, event);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_forward_event");
    return FALSE;
  }

  if (session_param->is_depth == TRUE) {
    ISP_DBG("<non-fatal> skip control_sof for Depth Camera");
    return TRUE;
  }

  if (session_param->rdi_only) {
    isp_util_send_rdi_only_metadata(module, session_param);
    ret = isp_util_report_stored_metadata(module, session_param,
      session_param->curr_frame_id);
    return TRUE;
  }

  isp_pipeline = isp->isp_resource.isp_resource_info[session_param->hw_id[0]].
    isp_pipeline;
  if (!isp_pipeline){
    ISP_ERR("failed: isp_pipeline is %p ", isp_pipeline);
    return FALSE;
  }

  /*only when set property enable then dump*/
  ret = isp_util_dump_register(module, session_param, sof_event->frame_id,
    isp_pipeline->num_register);
  if (ret == FALSE) {
    ISP_ERR("failed: dump register");
    return FALSE;
  }

  /* Report meta data dump, if */
  if (session_param->hw_update_params.metadata_dump_enable == 1) {
    ret = isp_util_report_meta_dump(module,session_param);
    if (ret == FALSE) {
      ISP_ERR("failed: meta data dump reporting");
    }
  }

  isp_util_send_metadata(module, session_param, TRUE);
  isp_util_post_hdr_data(port, session_param, sof_event->frame_id);

  ret = isp_util_report_stored_metadata(module, session_param,
    sof_event->frame_id);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_report_stored_metadata");
  }

  parser_params = &session_param->parser_params;

/* TODO: Remove when 3A supports sending event to request this info */
#if POST_ROLLOFF_INFO
  ret = isp_util_send_BE_info(module, session_param, event->identity,
    sof_event, parser_params->stats_params);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_broadcast_crop_info");
  }
#endif
  zoom_params_arr =
    parser_params->stats_params[session_param->hw_id[0]].zoom_params;

  zoom = &parser_params->stats_params[session_param->hw_id[0]].zoom;
  ret = isp_util_broadcast_crop_info(module, session_param,
    sof_event->frame_id, &sof_event->timestamp, zoom_params_arr, zoom, TRUE);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_broadcast_crop_info");
  }

  MCT_PROF_LOG_END();
  return ret;
ERROR:
  /* Forward event to downstream */
  ret = isp_util_forward_event(port, event);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_forward_event");
  }
  MCT_PROF_LOG_END();
  return ret;
}

static boolean isp_handler_send_output_dims(mct_module_t *module,
  const isp_hw_stream_info_t *hw_stream_info,
  const isp_stream_param_t *stream_param)
{
  boolean                 ret;
  mct_event_t             pp_event;
  mct_stream_info_t       mct_stream_info;

  memset(&pp_event, 0, sizeof(pp_event));
  memset(&mct_stream_info, 0, sizeof(mct_stream_info));
  mct_stream_info = stream_param->stream_info;
  mct_stream_info.dim.width = hw_stream_info->width;
  mct_stream_info.dim.height = hw_stream_info->height;
  mct_stream_info.fmt = hw_stream_info->fmt;
  ISP_HIGH("w %d h %d fmt %d", mct_stream_info.dim.width,
    mct_stream_info.dim.height, mct_stream_info.fmt);

  pp_event.type = MCT_EVENT_MODULE_EVENT;
  pp_event.identity = hw_stream_info->identity;
  pp_event.direction = MCT_EVENT_DOWNSTREAM;
  pp_event.u.module_event.type = MCT_EVENT_MODULE_ISP_OUTPUT_DIM;
  pp_event.u.module_event.module_event_data = &mct_stream_info;

  /* Forward event to pp */
  ret = isp_util_forward_event_downstream_to_all_types(module, &pp_event);
  if (ret == FALSE) {
    ISP_ERR("failed: event type %d", MCT_EVENT_MODULE_ISP_OUTPUT_DIM);
  }

  return ret;

}

static boolean isp_handler_send_dis_config(mct_module_t *module,
  const isp_hw_stream_info_t *hw_stream_info,
  isp_session_param_t *session_param, const isp_stream_param_t *stream_param)
{
  boolean                   ret;
  isp_dis_config_info_t     dis_cfg;
  mct_event_t               event;
  isp_private_event_t       private_event;
  isp_hw_rs_cs_stats_info_t cs_rs_stats_info;

  if(!hw_stream_info->dis_enabled)
    /* DIS is disabled */
    return TRUE;

  /* fetch RS and CS data */
  memset(&event, 0, sizeof(event));
  memset(&private_event, 0, sizeof(private_event));
  memset(&cs_rs_stats_info, 0, sizeof(cs_rs_stats_info));

  private_event.type = ISP_PRIVATE_FETCH_RS_CS_STATS_INFO;
  private_event.data = &cs_rs_stats_info;

  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = hw_stream_info->identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
  event.u.module_event.module_event_data = &private_event;

  ret = isp_util_forward_event_to_all_internal_pipelines(session_param,
    &event);
  if (ret == FALSE) {
    ISP_ERR("failed: event type %d", ISP_PRIVATE_FETCH_RS_CS_STATS_INFO);
    return ret;
  }

  dis_cfg.session_id = session_param->session_id;
  dis_cfg.stream_id = stream_param->stream_info.stream->streamid;
  dis_cfg.streaming_mode = stream_param->stream_info.streaming_mode;
  dis_cfg.height = hw_stream_info->height_before_dis;
  dis_cfg.width = hw_stream_info->width_before_dis;
  dis_cfg.col_num = cs_rs_stats_info.num_cols;
  dis_cfg.row_num = cs_rs_stats_info.num_rows;
 /* send dimensions to DIS */
  memset(&event, 0, sizeof(event));
  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = pack_identity(session_param->session_id,
    stream_param->stream_info.stream->streamid);
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_DIS_CONFIG;
  event.u.module_event.module_event_data = (void *)&dis_cfg;
  ISP_DBG("ISP_DIS_CONFIG iden 0x%x dim %dx%d",
    hw_stream_info->identity,
    dis_cfg.width, dis_cfg.height);
  /* Forward event to pp */
  ret = isp_util_forward_event_from_module(module, &event);
  if (ret == FALSE) {
    ISP_ERR("failed: event type %d", MCT_EVENT_MODULE_ISP_DIS_CONFIG);
  }

  return ret;
}


static boolean isp_handler_send_frameskip_upstream(mct_module_t *module,
  const isp_session_param_t *session_param,
  const isp_stream_param_t *stream_param)
{
  boolean     ret;
  mct_event_t event;
  uint32_t    frameskip = NO_SKIP;

  memset(&event, 0, sizeof(event));

  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = ISP_PACK_IDENTITY(session_param->session_id,
    stream_param->stream_info.stream->streamid);
  event.direction = MCT_EVENT_UPSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_FRAMESKIP;
  event.u.module_event.module_event_data = (void *)&frameskip;

  ret = isp_util_forward_event_from_module(module, &event);
  if (ret == FALSE) {
    ISP_ERR("failed: event type %d", MCT_EVENT_MODULE_ISP_FRAMESKIP);
  }

  return ret;
}

static boolean isp_handler_fetch_scaler_info(
  isp_session_param_t *session_param, isp_hw_stream_info_t *hw_stream_info,
  unsigned int identity)
{
  mct_event_t             super_dim_event;
  isp_private_event_t     private_event;

  /* Get super dim from scaler module */
  memset(&super_dim_event, 0, sizeof(super_dim_event));
  memset(&private_event, 0, sizeof(private_event));
  memset(hw_stream_info, 0, sizeof(*hw_stream_info));

  private_event.type = ISP_PRIVATE_FETCH_SCALER_HW_STREAM_INFO;
  private_event.data = hw_stream_info;

  super_dim_event.type = MCT_EVENT_MODULE_EVENT;
  super_dim_event.identity = identity;
  super_dim_event.direction = MCT_EVENT_DOWNSTREAM;
  super_dim_event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
  super_dim_event.u.module_event.module_event_data = &private_event;

  return isp_util_forward_event_to_all_internal_pipelines(session_param,
    &super_dim_event);
}

static boolean isp_send_stats_types(mct_module_t *module, mct_event_t *event,
  isp_session_param_t *session_param)
{
  mct_event_t             stats_event;
  mct_stats_info_t        mct_stats_info;
  boolean                 ret = TRUE;
  uint32_t                isp_version;
  mct_event_t             internal_event;
  isp_private_event_t     private_event;
  mct_stats_bf_caps_t    *bf_caps = NULL;

  if (!module || !event || !session_param) {
    ISP_ERR("failed: %p %p %p", module, event, session_param);
    return FALSE;
  }

  memset(&stats_event, 0, sizeof(stats_event));
  memset(&mct_stats_info, 0, sizeof(mct_stats_info));

  private_event.type = ISP_PRIVATE_HW_GET_STATS_CAPABILITES;
  private_event.data = &mct_stats_info;

  internal_event.type = MCT_EVENT_MODULE_EVENT;
  internal_event.identity = event->identity;
  internal_event.direction = MCT_EVENT_DOWNSTREAM;
  internal_event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
  internal_event.u.module_event.module_event_data = &private_event;

  ret = isp_util_forward_event_to_all_internal_pipelines(session_param,
    &internal_event);
  if (ret == FALSE) {
    ISP_ERR("failed: ISP_PRIVATE_HW_GET_STATS_CAPABILITES");
  }
  bf_caps = &mct_stats_info.bf_caps;
  ISP_DBG("%x %d %d %d %d %d %d %d %d %d %d %d %d", bf_caps->bf_mask,
  bf_caps->bf_roi_caps.min_grid_width, bf_caps->bf_roi_caps.min_grid_height,
  bf_caps->bf_roi_caps.max_grid_width, bf_caps->bf_roi_caps.max_grid_height,
  bf_caps->bf_roi_caps.min_hor_offset, bf_caps->bf_roi_caps.max_hor_offset,
  bf_caps->bf_roi_caps.min_ver_offset, bf_caps->bf_roi_caps.max_ver_offset,
  bf_caps->bf_roi_caps.min_hor_grids, bf_caps->bf_roi_caps.max_hor_grids,
  bf_caps->bf_roi_caps.min_ver_grids, bf_caps->bf_roi_caps.max_ver_grids);
  mct_stats_info.stats_mask = session_param->parser_params.stats_ctrl.stats_mask;

  ret = isp_util_get_isp_version(module, event->identity,
    &isp_version);
  if (ret == FALSE) {
    ISP_ERR("failed to get isp version");
    return FALSE;
  }
  mct_stats_info.stats_depth = ISP_STATS_BIT_DEPTH;
  mct_stats_info.pipeline_bit_width = ISP_PIPELINE_WIDTH;

  if (GET_ISP_MAIN_VERSION(isp_version) >= ISP_VERSION_44)
    mct_stats_info.kernel_size = MCT_EVENT_STATS_HPF_2X11;
  else if (GET_ISP_MAIN_VERSION(isp_version) == ISP_VERSION_40)
    mct_stats_info.kernel_size = MCT_EVENT_STATS_HPF_2X5;
  else if (GET_ISP_MAIN_VERSION(isp_version) == ISP_VERSION_32)
    mct_stats_info.kernel_size = MCT_EVENT_STATS_HPF_2X5;
  else
    ISP_ERR("unsupported isp version");
  ISP_DBG("isp version %d af_kernel_size %d",
    GET_ISP_MAIN_VERSION(isp_version), mct_stats_info.kernel_size);
  stats_event.type = MCT_EVENT_MODULE_EVENT;
  stats_event.identity = event->identity;
  stats_event.direction = MCT_EVENT_DOWNSTREAM;
  stats_event.u.module_event.type = MCT_EVENT_MODULE_ISP_STATS_INFO;
  stats_event.u.module_event.module_event_data = &mct_stats_info;

  /* Forward event to pp */
  ret = isp_util_forward_event_downstream_to_all_types(module, &stats_event);
  if (ret == FALSE) {
    ISP_ERR("failed: event type %d", MCT_EVENT_MODULE_ISP_STATS_INFO);
  }
  return ret;
}

/** isp_handler_control_streamon:
 *
 *  @module: mct module handle
 *  @port: port on which this event arrived
 *  @event: event to be handled
 *
 *  1) Check whether this session is already streaming, if so,
 *  return
 *
 *  2) If this stream is part of bundle, check whether all
 *  stream in this bundle is streaming, If so, handle stream
 *  ON sequence, else wait for last stream in the bundle
 *
 *  3) If this stream is not part of bundle, handle stream ON
 *  sequence
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_handler_control_streamon(mct_module_t *module, mct_port_t *port,
  mct_event_t *event)
{
  boolean                      ret = TRUE;
  isp_session_param_t          *session_param = NULL;
  isp_stream_param_t           *stream_param = NULL;
  boolean                      is_stream_on = FALSE;
  isp_bundle_params_t          *bundle_params = NULL;
  cam_bundle_config_t          *bundle_info = NULL;
  isp_hw_stream_info_t         hw_stream_info;
  isp_t                        *isp = NULL;
  void                         *save_ptr = NULL;
  isp_stream_port_map_info_t    *streams = NULL;
  stats_get_data_t               stats_get;
  stats_update_t                 stats_update;
  uint32_t                       i = 0;
  boolean                        is_liveshot = FALSE;
  mct_event_t                    mct_event;

  if (!module || !port || !event) {
    ISP_ERR("failed: module %p port %p event %p", module, port, event);
    return FALSE;
  }

  MCT_PROF_LOG_BEG(PROF_ISP_STREAM_ON);

  ISP_HIGH("ide %x", event->identity);
  ret = isp_util_get_session_stream_params(module, event->identity,
    &session_param, &stream_param);
  if (ret == FALSE || !session_param || !stream_param) {
    ISP_ERR("failed: isp_util_get_session_stream_params ret %d %p %p",
      ret, session_param, stream_param);
    MCT_PROF_LOG_END();
    return FALSE;
  }

  if (stream_param->stream_info.stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    ISP_ERR("Offline stream identity %x streamon NOP E",
      event->identity);
    ret = isp_util_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_forward_event");
    }
    ISP_ERR("Offline stream identity %x streamon NOP X",
      event->identity);
    PTHREAD_MUTEX_LOCK(&session_param->mutex);
    /* Allocate stats buffers for offline isp in buffer manager */
    ret = isp_stats_buf_mgr_init(
       &session_param->parser_params.buf_mgr[ISP_STREAMING_OFFLINE],
      &session_param->parser_params.resource_request[ISP_STREAMING_OFFLINE],
      ISP_SWAP_AEC_BG_HDR_BE);

    if (ret == FALSE) {
      ISP_ERR("failed: isp_stats_buf_mgr_init");
      goto ERROR;
    }
    PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
    stream_param->stream_state = ISP_STREAM_STATE_ACTIVE;
    return TRUE;
  }

  isp = MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    MCT_PROF_LOG_END();
    return FALSE;
  }

  if (session_param->num_isp > ISP_HW_MAX) {
    ISP_ERR("failed: invalid num_isp %d", session_param->num_isp);
    return FALSE;
  }

  if (!session_param->num_isp) {
    ISP_ERR("num_isp %d", session_param->num_isp);
    /* Forward event to downstream */
    ret = isp_util_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_forward_event");
    }
    /* For RAW only usecase, num_isp will be 0, bypass stream ON in ISP */
    return ret;
  }

  if (stream_param->stream_info.stream_type != CAM_STREAM_TYPE_RAW) {
    ret = isp_send_stats_types(module, event, session_param);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_send_stats_types");
    }
  }

  PTHREAD_MUTEX_LOCK(&session_param->mutex);

  if (!session_param->stream_on_count) {
    /* Allocate stats buffers in buffer manager */
    ret = isp_stats_buf_mgr_init(
       &session_param->parser_params.buf_mgr[ISP_STREAMING_ONLINE],
      &session_param->parser_params.resource_request[ISP_STREAMING_ONLINE],
      ISP_SWAP_AEC_BG_HDR_BE);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_stats_buf_mgr_init");
      goto ERROR;
    }
    ret = isp_bundled_stats_buf_init(&session_param->
      parser_params.bundled_stats_buf_mgr);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_bundled_stats_buf_mgr_init");
      goto ERROR;
    }
    /* Send 3A about the ADRC Supported ISP HW module*/
    ret = isp_util_send_adrc_hw_module_mask(module, session_param);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_handler_send_adrc_hw_module_mask");
    }
  }

  /* Check whether this stream is part of bundle */
  ISP_HIGH("fastaec %d", session_param->fast_aec_mode);
  bundle_params = stream_param->bundle_params[0];
  if (!bundle_params || session_param->fast_aec_mode == TRUE) {
    /*
     * Check whether this is the first non bundle stream that is receiving
     * stream ON
     */
    if (!(session_param->stream_on_count++)) {
      /* Stream ON */
      is_stream_on = TRUE;
    }
  } else {
    /*
     * Check whether this stream is the last stream that is receiving
     * stream ON
     */
     if (!bundle_params) {
       ISP_ERR("failed: bundle_params %p", bundle_params);
       return FALSE;
     }
     bundle_info = &bundle_params->bundle_info;
     if (++bundle_params->streamon_count == bundle_info->num_of_streams) {
       /* Stream ON */
       is_stream_on = TRUE;
     }
     session_param->stream_on_count++;
  }

  ret = isp_util_send_submod_enable_to_pipeline(&isp->isp_resource,
        session_param);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_send_submod_enable_to_pipeline");
    goto ERROR;
  }

  ret = isp_util_forward_event_to_all_internal_pipelines(session_param,
    event);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_forward_event_to_pipelines");
    goto ERROR;
  }

  ret = isp_util_forward_event_to_int_pipeline_to_fill_cfg(session_param,
     &isp->isp_resource, event->identity,
     stream_param->stream_info.stream_type);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_forward_event_to_pipelines"
    "MCT_EVENT_CONTROL_DYNAMIC_MODULE_ENABLE");
    goto ERROR;
  }

  ret = isp_handler_fetch_scaler_info(session_param, &hw_stream_info,
    event->identity);
  if (ret == TRUE) {
    isp_handler_send_dis_config(module, &hw_stream_info, session_param,
      stream_param);
#ifdef SEND_OUTPUT_DIM_EVENT
    isp_handler_send_output_dims(module, &hw_stream_info, stream_param);
#endif
  }

  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
  if (session_param->fast_aec_mode == TRUE) {
    ret = isp_util_forward_event_downstream_to_type(module, event,
      MCT_PORT_CAPS_STATS);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_forward_event_downstream_to_type");
    }
  } else {
    /* Forward event to downstream */
    ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_ISP_STREAMON_FWD);
    ret = isp_util_forward_event(port, event);
    ATRACE_CAMSCOPE_END(CAMSCOPE_ISP_STREAMON_FWD);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_forward_event");
    }
  }
  PTHREAD_MUTEX_LOCK(&session_param->mutex);

  ret = isp_handler_send_frameskip_upstream(module, session_param,
    stream_param);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_handler_send_frameskip_upstream");
    goto ERROR;
  }

  /* for non zsl snapshot usecase, ISP needs to consume the gain from stats.
     without this changes, ISP applies the previousely saved AEC gain.*/
  streams = session_param->stream_port_map.streams;
  if (stream_param->stream_info.stream_type == CAM_STREAM_TYPE_SNAPSHOT &&
    stream_param->stream_info.streaming_mode != CAM_STREAMING_MODE_CONTINUOUS) {
    for (i = 0; i < session_param->stream_port_map.num_streams; i++) {
      if (streams[i].stream_type == CAM_STREAM_TYPE_VIDEO)
        is_liveshot = TRUE;
    }
    if (!is_liveshot) {
      memset(&mct_event, 0, sizeof(mct_event));
      mct_event.type = MCT_EVENT_MODULE_EVENT;
      mct_event.identity = event->identity;
      mct_event.direction = MCT_EVENT_DOWNSTREAM;
      mct_event.u.module_event.type = MCT_EVENT_MODULE_STATS_GET_DATA;
      mct_event.u.module_event.module_event_data = (void *)&stats_get;

      /* Change event direction to downstream and forward to stats port */
      ret = isp_util_forward_event_downstream_to_type(module, &mct_event,
                                                      MCT_PORT_CAPS_STATS);
      if (ret == FALSE) {
        ISP_ERR("failed: isp_util_forward_event_downstream_to_type type %d",
                mct_event.type);
      }
      memset(&stats_update, 0, sizeof(stats_update));
      mct_event.u.module_event.type = MCT_EVENT_MODULE_STATS_AEC_UPDATE;
      stats_update.aec_update.real_gain = stats_get.aec_get.real_gain[0];
      stats_update.aec_update.sensor_gain = stats_get.aec_get.sensor_gain[0];
      stats_update.aec_update.total_drc_gain = stats_get.aec_get.total_drc_gain;
      stats_update.aec_update.color_drc_gain = stats_get.aec_get.color_drc_gain;
      stats_update.aec_update.gtm_ratio = stats_get.aec_get.gtm_ratio;
      stats_update.aec_update.ltm_ratio = stats_get.aec_get.ltm_ratio;
      stats_update.aec_update.la_ratio = stats_get.aec_get.la_ratio;
      stats_update.aec_update.gamma_ratio = stats_get.aec_get.gamma_ratio;
      stats_update.aec_update.hdr_sensitivity_ratio = stats_get.aec_get.hdr_sensitivity_ratio;
      stats_update.aec_update.hdr_exp_time_ratio = stats_get.aec_get.hdr_exp_time_ratio;
      stats_update.aec_update.lux_idx = stats_get.aec_get.lux_idx;
      stats_update.aec_update.flash_sensitivity = stats_get.aec_get.flash_sensitivity;
      stats_update.aec_update.settled = TRUE;
      stats_update.flag = STATS_UPDATE_AEC;
      mct_event.u.module_event.module_event_data = (void *)&stats_update;
      ret = isp_util_forward_event_to_all_internal_pipelines(session_param,
                                                             &mct_event);
      if (ret == FALSE) {
        ISP_ERR("failed: isp_util_forward_event_to_all_internal_pipelines");
      }
    }
  }


/* ========================================================================== */
  if (is_stream_on == TRUE && session_param->state != ISP_STATE_STREAMING) {
    PTHREAD_MUTEX_LOCK(&session_param->state_mutex);
    session_param->state = ISP_STATE_STREAMING;
    PTHREAD_MUTEX_UNLOCK(&session_param->state_mutex);

    ISP_DBG("call internal hw update event");
    /* Call internal hw update event */
    ret = isp_util_trigger_internal_hw_update_event(session_param,
      session_param->session_based_ide, 0, session_param->hw_id,
      session_param->num_isp, NULL);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_trigger_internal_hw_update_event");
      goto ERROR;
    }


    /* Do iotcl to configure hw */
    ret = isp_hw_update_util_do_ioctl_in_hw_update_params(&isp->isp_resource,
      session_param);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_hw_update_do_ioctl");
      goto ERROR;
    }

    ISP_DBG("update cur_hw_update_list");
    ret = isp_util_update_hw_param(module, session_param);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_update_hw_param");
      goto ERROR;
    }

  }
  stream_param->stream_state = ISP_STREAM_STATE_ACTIVE;
  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
  MCT_PROF_LOG_END();
  return ret;

ERROR:
  ISP_ERR("failed");
  MCT_PROF_LOG_END();
  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
  return ret;
}

/** isp_handler_control_streamoff:
 *
 *  @module: mct module handle
 *  @port: port on which this event arrived
 *  @event: event to be handled
 *
 *  1) Check whether this session is already idle, if so,
 *  return
 *
 *  2) If this stream is part of bundle, check whether all
 *  stream in this bundle is idle, If so, handle stream OFF
 *  sequence, else wait for last stream in the bundle
 *
 *  3) If this stream is not part of bundle, handle stream OFF
 *  sequence
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_handler_control_streamoff(mct_module_t *module, mct_port_t *port,
  mct_event_t *event)
{
  boolean                      ret = TRUE;
  isp_session_param_t         *session_param = NULL;
  isp_stream_param_t          *stream_param = NULL;
  boolean                      is_stream_off = FALSE;
  isp_bundle_params_t         *bundle_params = NULL;
  int                         i;
  isp_t                        *isp = NULL;

  if (!module || !port || !event) {
    ISP_ERR("failed: module %p port %p event %p", module, port, event);
    return FALSE;
  }

  isp = MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    MCT_PROF_LOG_END();
    return FALSE;
  }

  ISP_HIGH("ide %x", event->identity);
  ret = isp_util_get_session_stream_params(module, event->identity,
    &session_param, &stream_param);
  if (ret == FALSE || !session_param || !stream_param) {
    ISP_ERR("failed: isp_util_get_session_stream_params ret %d %p %p",
      ret, session_param, stream_param);
    return FALSE;
  }
  if (stream_param->stream_info.stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    ISP_ERR("Offline stream identity %x streamoff NOP E",
      event->identity);
    ret = isp_util_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_forward_event");
    }
    ISP_ERR("Offline stream identity %x streamoff NOP X",
      event->identity);
    ret = isp_stats_buf_mgr_deinit(
       &session_param->parser_params.buf_mgr[ISP_STREAMING_OFFLINE]);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_stats_buf_mgr_deinit");
    }
    /* memset resource_request struct */
    memset(&session_param->parser_params.resource_request[ISP_STREAMING_OFFLINE], 0,
      sizeof(session_param->parser_params.resource_request[ISP_STREAMING_OFFLINE]));
    stream_param->stream_state = ISP_STREAM_STATE_INACTIVE;
    return TRUE;
  }

  /* Forward event to downstream */
  if (session_param->fast_aec_mode == TRUE) {
    ret = isp_util_forward_event_downstream_to_type(module, event,
      MCT_PORT_CAPS_STATS);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_forward_event_downstream_to_type");
    }
  } else {
    /* Forward event to downstream */
    ret = isp_util_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_forward_event");
    }
  }

  PTHREAD_MUTEX_LOCK(&session_param->mutex);
  bundle_params = stream_param->bundle_params[0];

  ISP_HIGH("fastaec %d", session_param->fast_aec_mode);
  /* Check whether this stream is part of bundle */
  if (!bundle_params || session_param->fast_aec_mode == TRUE) {

    /* Forward stream OFF event to pipelines */
    ISP_HIGH("stream_off %d %d ide %x", event->u.ctrl_event.type,
      MCT_EVENT_CONTROL_STREAMOFF, event->identity);
    ret = isp_util_forward_event_to_all_internal_pipelines(session_param,
      event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_forward_event_to_pipelines");
      ret = FALSE;
      goto ERROR;
    }

    ret = isp_util_forward_event_to_int_pipeline_to_fill_cfg(session_param,
       &isp->isp_resource, event->identity,
       stream_param->stream_info.stream_type);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_forward_event_to_int_pipeline_to_fill_cfg");
      goto ERROR;
    }

    /*
     * Check whether this is the last non bundle stream that is receiving
     * stream OFF. If stream_on_count is already zero, do not decrement it
     */
    if (!session_param->stream_on_count) {
      ISP_ERR("invalid stream_on_count %d", session_param->stream_on_count);
      session_param->state = ISP_STATE_IDLE;
      goto ERROR;
    }
    session_param->stream_on_count--;
  } else {
    /*
     * Check whether this stream is the first stream that is receiving
     * stream OFF
     */
    if(bundle_params->streamon_count ==
       bundle_params->bundle_info.num_of_streams) {

      /* Forward stream OFF event to pipelines */
      for (i = 0; i < bundle_params->bundle_info.num_of_streams; i++) {
        mct_event_t internal_event;
        internal_event = *event;
        internal_event.identity &= 0xFFFF0000;
        internal_event.identity |= bundle_params->bundle_info.stream_ids[i];
    ISP_DBG("stream_off %d %d ide %x", event->u.ctrl_event.type,
      MCT_EVENT_CONTROL_STREAMOFF, event->identity);
        ret = isp_util_forward_event_to_all_internal_pipelines(session_param,
          &internal_event);
        if (ret == FALSE) {
          ISP_ERR("failed: isp_util_forward_event_to_pipelines");
          ret = FALSE;
          goto ERROR;
        }
      }
    }

    if (bundle_params->streamon_count) {
      bundle_params->streamon_count--;
    } else {
      ISP_ERR("Bundle streamon count is less than zero\n");
    }

    if (!session_param->stream_on_count) {
      ISP_ERR("invalid stream_on_count %d", session_param->stream_on_count);
      goto ERROR;
    }
    session_param->stream_on_count--;
  }

  if (!session_param->stream_on_count) {
    /* Stream OFF */
    is_stream_off = TRUE;
  }

  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);

  if (is_stream_off == TRUE) {
    /* Stream OFF */
    PTHREAD_MUTEX_LOCK(&session_param->state_mutex);
    session_param->state = ISP_STATE_IDLE;
    PTHREAD_MUTEX_UNLOCK(&session_param->state_mutex);
    ret = isp_handler_clear_events_queues(module, session_param);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_handler_clear_events_queues");
      goto SKIP_UNLOCK;
    }
  }

  if (!session_param->stream_on_count) {
    /* Deallocate stats buffers in buffer manager */
    ret = isp_stats_buf_mgr_deinit(
       &session_param->parser_params.buf_mgr[ISP_STREAMING_ONLINE]);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_stats_buf_mgr_init");
      goto SKIP_UNLOCK;
    }
    ret = isp_bundled_stats_buf_deinit(
      &session_param->parser_params.bundled_stats_buf_mgr);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_bundled_stats_buf_deinit");
      goto SKIP_UNLOCK;
    }
    /* memset resource_request struct */
    memset(&session_param->parser_params.resource_request, 0,
      sizeof(session_param->parser_params.resource_request));
    memset(&session_param->reg_update_info, 0,
      sizeof(session_param->reg_update_info));
    memset(&session_param->lpm_info, 0, sizeof(session_param->lpm_info));
    memset(&session_param->parser_params.resource_request[ISP_STREAMING_ONLINE], 0,
      sizeof(session_param->parser_params.resource_request[ISP_STREAMING_ONLINE]));
    session_param->parser_params.algo_saved_parm.tintless_saved_algo_parm.is_flash_mode = FALSE;
  }
  stream_param->stream_state = ISP_STREAM_STATE_INACTIVE;

  return ret;
ERROR:
  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
SKIP_UNLOCK:
  stream_param->stream_state = ISP_STREAM_STATE_INACTIVE;
  return ret;
}

/** isp_handler_request_pp_divert:
 *
 *  @module: mct module handle
 *  @port: port on which this event arrived
 *  @event: event to be handled
 *
 * Check whether we will divert the stream to pproc and stores it
 * in stream param. This is needed to determine if we need to
 * send crop message to bus.
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_handler_request_pp_divert(mct_module_t *module,
  mct_port_t *port, mct_event_t *event)
{
  boolean              ret = TRUE;
  isp_t               *isp = NULL;
  isp_session_param_t *session_param = NULL;
  isp_stream_param_t  *stream_param = NULL;

  if (!module || !port || !event) {
    ISP_ERR("failed: %p %p %p", module, port, event);
    return FALSE;
  }

  isp = (isp_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }

  ret = isp_util_get_session_stream_params(module, event->identity,
    &session_param, &stream_param);
  if (ret == FALSE || !session_param || !stream_param) {
    ISP_ERR("failed: isp_util_get_session_stream_params ret %d %p %p",
      ret, session_param, stream_param);
    return FALSE;
  }

  /* Forward event to downstream */
  ret = isp_util_forward_event(port, event);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_forward_event");
  }

  stream_param->need_divert =
        *(boolean *)(event->u.module_event.module_event_data);

  return ret;
}

/** isp_handler_module_set_chromatix_ptr:
 *
 *  @module: mct module handle
 *  @port: port on which this event arrived
 *  @event: event to be handled
 *
 *  Pass event to hw pipeline
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_handler_module_set_chromatix_ptr(mct_module_t *module,
  mct_port_t *port, mct_event_t *event)
{
  boolean              ret = TRUE;
  isp_t               *isp = NULL;
  isp_session_param_t *session_param = NULL;

  if (!module || !port || !event) {
    ISP_ERR("failed: %p %p %p", module, port, event);
    return FALSE;
  }

  isp = (isp_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }

  ret = isp_util_get_session_params(module, ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  /* Forward stream ON event to pipelines */
  ret = isp_util_forward_event_to_all_internal_pipelines(session_param, event);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_forward_event_to_pipelines");
  }

  /* Forward event to downstream */
  ret = isp_util_forward_event(port, event);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_forward_event");
  }
  return ret;
}

/** isp_handler_module_handle_reg_update:
 *  Handle reg update IRQ
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_handler_module_handle_reg_update(mct_module_t *module,
  mct_port_t *port, mct_event_t *event)
{
  boolean                 ret = TRUE;
  isp_t                   *isp = NULL;
  isp_session_param_t     *session_param = NULL;
  uint32_t                *frame_id = NULL;
  mct_event_t             mct_event;
  mct_event_inform_lpm_t  data;

  if (!module || !port || !event) {
    ISP_ERR("failed: %p %p %p", module, port, event);
    return FALSE;
  }

  ret = isp_util_get_session_params(module, ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  frame_id = event->u.module_event.module_event_data;
  if (!frame_id) {
    ISP_ERR(" Error! NULL param");
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&session_param->state_mutex);
  session_param->reg_update_info.last_reg_update_frame_id = *frame_id;
  if (session_param->reg_update_info.reg_update_state ==
    ISP_REG_UPDATE_STATE_REQUESTED) {
    session_param->reg_update_info.reg_update_state =
      ISP_REG_UPDATE_STATE_RECEIVED;
  } else {
    ISP_INFO(" Warning! Invalid reg_update state %d",
      session_param->reg_update_info.reg_update_state);
  }

  if ((session_param->lpm_info.storedenableLPM !=
      session_param->lpm_info.enableLPM) ||
      session_param->lpm_info.isupdated == FALSE) {
    session_param->lpm_info.isupdated = TRUE;
    data.is_lpm_enabled =  session_param->lpm_info.enableLPM;
    /*CPP will add frame ID + 2, hence we need to send frame id with -
     * max apply delay is - 1 here  */
    memset(&mct_event, 0, sizeof(mct_event));
    mct_event.u.module_event.type = MCT_EVENT_MODULE_ISP_INFORM_LPM;
    mct_event.u.module_event.module_event_data = (void *)&data;
    mct_event.type = MCT_EVENT_MODULE_EVENT;
    mct_event.identity = session_param->session_based_ide;
    mct_event.direction = MCT_EVENT_DOWNSTREAM;
    mct_event.u.module_event.current_frame_id =
      *frame_id - (session_param->max_apply_delay - 1);
   if((int)mct_event.u.module_event.current_frame_id < 0)
     mct_event.u.module_event.current_frame_id = 0;

    ret = isp_util_forward_event(port, &mct_event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_forward_event");
      PTHREAD_MUTEX_UNLOCK(&session_param->state_mutex);
      return FALSE;
    }
    session_param->lpm_info.storedenableLPM = session_param->lpm_info.enableLPM;
    session_param->lpm_info.meta_report_pending =  TRUE;
    ISP_DBG("Sent LPM info to PPROC %d frame %d"
      ,session_param->lpm_info.storedenableLPM,
      mct_event.u.module_event.current_frame_id);
  }
  PTHREAD_MUTEX_UNLOCK(&session_param->state_mutex);

  return ret;
}

/** isp_handler_process_offline_metadata:
 *
 *  @module: mct module handle
 *  @port: port on which this event arrived
 *  @event: event to be handled
 *
 *  Extract meta data and send events
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_handler_process_offline_metadata(mct_module_t *module,
  mct_port_t *port, mct_event_t *event)
{
  boolean                           ret = TRUE;
  isp_t                            *isp = NULL;
  iface_fetch_engine_cfg_t         *fetch_engine_cfg_request = NULL;
  isp_session_param_t              *session_param = NULL;
  isp_hw_id_t                       hw_index = 0;
  cam_stream_parm_buffer_t         *parm_buf = NULL;
  metadata_buffer_t                *metadata = NULL;
  mct_list_t                       *l_stream_params = NULL;
  isp_stream_param_t               *stream_param = NULL;
  isp_fetch_eng_cfg_data_t         *fetch_eng_cfg_data = NULL;

  if (!module || !port || !event) {
    ISP_ERR("failed: %p %p %p", module, port, event);
    return FALSE;
  }

  isp = (isp_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }

  ret = isp_util_get_session_params(module,
    ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  /* find stream_params for particular identity */
  l_stream_params = mct_list_find_custom(session_param->l_stream_params,
    &event->identity, isp_util_compare_identity_from_stream_param);
  if (l_stream_params == NULL) {
    ISP_ERR("failed: can not find stream param for this identity %x",
      event->identity);
    return FALSE;
  }

  stream_param = (isp_stream_param_t *)l_stream_params->data;

  fetch_engine_cfg_request =
    (iface_fetch_engine_cfg_t *)event->u.module_event.module_event_data;
  if (fetch_engine_cfg_request == NULL) {
    ISP_ERR("failed, NULL pointer %p", fetch_engine_cfg_request);
    return FALSE;
  }
  parm_buf = fetch_engine_cfg_request->stream_param_buf;

  ISP_DBG("dump_offline meta: input stream info ,id %x, W %d H %d",
    fetch_engine_cfg_request->input_stream_info.identity,
    fetch_engine_cfg_request->input_stream_info.dim.width,
    fetch_engine_cfg_request->input_stream_info.dim.height);

  ISP_DBG("dump_offline parm buf: meta present %d, stream %x, meta idx %d",
    parm_buf->reprocess.meta_present, parm_buf->reprocess.meta_stream_handle,
    parm_buf->reprocess.meta_buf_index);

  /* get meta buffer */
  metadata = isp_util_extract_metadata_buffer(module,
    session_param, stream_param, parm_buf);
  if (metadata == NULL) {
    ISP_ERR("failed: fe_dbg isp_util_extract_metadata_buffer, metadata %p",
      metadata);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&session_param->mutex);
  fetch_eng_cfg_data = &session_param->fetch_eng_cfg_data;
  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);

  memset(&fetch_eng_cfg_data->sensor_out_info,
    0, sizeof(sensor_out_info_t));
  memset(&fetch_eng_cfg_data->aec_update,
    0 ,sizeof(aec_update_t));
  memset(&fetch_eng_cfg_data->awb_update,
    0 ,sizeof(awb_update_t));
  memset(&fetch_eng_cfg_data->offline_chromatix,
    0, sizeof(modulesChromatix_t));
  /* prepare offline set up event data*/
  ret = isp_util_prepare_offline_cfg_data(
    session_param, metadata, fetch_eng_cfg_data, fetch_engine_cfg_request);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_prepare_offline_cfg_data");
  }

  /* send offline cfg event to sub module*/
  for (hw_index = 0; hw_index < ISP_HW_MAX; hw_index++) {
    isp_hw_id_t session_hw_id = session_param->offline_hw_id[hw_index];
    if (session_hw_id == ISP_HW_MAX) {
       continue;
    }
    if (fetch_engine_cfg_request->isp_id_mask & (1 << session_hw_id) &&
        session_hw_id == session_param->offline_hw_id[0]) {
      ret = isp_util_set_offline_cfg_event(isp, session_param,
        fetch_eng_cfg_data, session_hw_id, event->identity);
      if (ret == FALSE) {
        ISP_ERR("failed: isp_util_set_offline_cfg_data");
        return FALSE;
      }
      ret = isp_util_forward_offline_event_modules(stream_param, metadata,
        session_param, event, session_hw_id);
      if (ret == FALSE) {
        ISP_ERR("failed: ret %d", ret);
        return FALSE;
      }
    }
  }

  return ret;
}

/** isp_handler_process_offline_metadata:
 *
 *  @module: mct module handle
 *  @port: port on which this event arrived
 *  @event: event to be handled
 *
 *  Extract meta data and send events
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_handler_process_offline_overwrite(mct_module_t *module,
  mct_port_t *port, mct_event_t *event)
{
  boolean                           ret = TRUE;
  isp_t                            *isp = NULL;
  iface_fetch_engine_cfg_t         *fetch_engine_cfg_request = NULL;
  isp_session_param_t              *session_param = NULL;
  isp_hw_id_t                       hw_index = 0;
  cam_stream_parm_buffer_t         *parm_buf = NULL;
  metadata_buffer_t                *metadata = NULL;
  mct_list_t                       *l_stream_params = NULL;
  isp_stream_param_t               *stream_param = NULL;
  isp_fetch_eng_cfg_data_t         fetch_eng_cfg_data;

  if (!module || !port || !event) {
    ISP_ERR("failed: %p %p %p", module, port, event);
    return FALSE;
  }

  isp = (isp_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }

  ret = isp_util_get_session_params(module,
    ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  /* find stream_params for particular identity */
  l_stream_params = mct_list_find_custom(session_param->l_stream_params,
    &event->identity, isp_util_compare_identity_from_stream_param);
  if (l_stream_params == NULL) {
    ISP_ERR("failed: can not find stream param for this identity %x",
      event->identity);
    return FALSE;
  }

  stream_param = (isp_stream_param_t *)l_stream_params->data;

  fetch_engine_cfg_request =
    (iface_fetch_engine_cfg_t *)event->u.module_event.module_event_data;
  if (fetch_engine_cfg_request == NULL) {
    ISP_ERR("failed, NULL pointer %p", fetch_engine_cfg_request);
    return FALSE;
  }
  parm_buf = fetch_engine_cfg_request->stream_param_buf;

  ISP_DBG("dump_offline meta: input stream info ,id %x, W %d H %d",
    fetch_engine_cfg_request->input_stream_info.identity,
    fetch_engine_cfg_request->input_stream_info.dim.width,
    fetch_engine_cfg_request->input_stream_info.dim.height);

  ISP_DBG("dump_offline parm buf: meta present %d, stream %x, meta idx %d",
    parm_buf->reprocess.meta_present, parm_buf->reprocess.meta_stream_handle,
    parm_buf->reprocess.meta_buf_index);

  /* get meta buffer */
  metadata = isp_util_extract_metadata_buffer(module,
    session_param, stream_param, parm_buf);
  if (metadata == NULL) {
    ISP_ERR("failed: fe_dbg isp_util_extract_metadata_buffer, metadata %p",
      metadata);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&session_param->mutex);
  fetch_eng_cfg_data = session_param->fetch_eng_cfg_data;
  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);

  memset(&fetch_eng_cfg_data.sensor_out_info,
    0, sizeof(sensor_out_info_t));
  memset(&fetch_eng_cfg_data.aec_update,
    0 ,sizeof(aec_update_t));
  memset(&fetch_eng_cfg_data.awb_update,
    0 ,sizeof(awb_update_t));
  memset(&fetch_eng_cfg_data.offline_chromatix,
    0, sizeof(modulesChromatix_t));
  /* prepare offline set up event data*/
  ret = isp_util_prepare_offline_cfg_data(
    session_param, metadata, &fetch_eng_cfg_data, fetch_engine_cfg_request);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_prepare_offline_cfg_data");
  }

  /* send offline cfg event to sub module*/
  for (hw_index = 0; hw_index < ISP_HW_MAX; hw_index++) {
    isp_hw_id_t session_hw_id = session_param->offline_hw_id[hw_index];
    if (session_hw_id == ISP_HW_MAX) {
       continue;
    }
    if (fetch_engine_cfg_request->isp_id_mask & (1 << session_hw_id) &&
        session_hw_id == session_param->offline_hw_id[0]) {
      ret = isp_util_set_offline_cfg_overwrite(isp, session_param,
        &fetch_eng_cfg_data, session_hw_id, event->identity);
      if (ret == FALSE) {
        ISP_ERR("failed: isp_util_set_offline_cfg_data");
        return FALSE;
      }
    }
  }
  return ret;
}

/** isp_handler_streamon_offline_pipeline:
 *
 *  @module: mct module handle
 *  @port: port on which this event arrived
 *  @event: event to be handled
 *
 *  Send streamon to pipeline modules
 *
 *  Return TRUE on success and FALSE on failure
 **/

boolean isp_handler_streamon_offline_pipeline(mct_module_t *module,
  mct_port_t *port, mct_event_t *event)
{

  boolean                           ret = TRUE;
  iface_fetch_engine_cfg_t         *fetch_engine_cfg_request = NULL;
  isp_session_param_t              *session_param = NULL;
  isp_hw_id_t                       hw_index = 0;
  mct_event_t                       internel_event;
  mct_list_t                       *l_stream_params = NULL;
  isp_stream_param_t               *stream_param = NULL;
  mct_stream_info_t                *offline_stream_info = NULL;

  if (!module || !port || !event) {
    ISP_ERR("failed: %p %p %p", module, port, event);
    return FALSE;
  }

  ret = isp_util_get_session_params(module,
    ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  ret = isp_stats_buf_mgr_init(
     &session_param->parser_params.buf_mgr[ISP_STREAMING_OFFLINE],
    &session_param->parser_params.resource_request[ISP_STREAMING_OFFLINE],
    ISP_SWAP_AEC_BG_HDR_BE);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_stats_buf_mgr_init");
    goto ERROR;
  }
  ret = isp_bundled_stats_buf_init(&session_param->
    parser_params.bundled_stats_buf_mgr);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_bundled_stats_buf_mgr_init");
    goto ERROR;
  }

  /* find stream_params for particular identity */
  l_stream_params = mct_list_find_custom(session_param->l_stream_params,
    &event->identity, isp_util_compare_identity_from_stream_param);
  if (l_stream_params == NULL) {
    ISP_ERR("failed: can not find stream param for this identity %x",
      event->identity);
    return FALSE;
  }

  stream_param = (isp_stream_param_t *)l_stream_params->data;

  fetch_engine_cfg_request =
    (iface_fetch_engine_cfg_t *)event->u.module_event.module_event_data;
  if (fetch_engine_cfg_request == NULL) {
    ISP_ERR("failed, NULL pointer %p", fetch_engine_cfg_request);
    return FALSE;
  }

  /* send offline cfg event to sub module*/
  for (hw_index = 0; hw_index < ISP_HW_MAX; hw_index++) {
    isp_hw_id_t session_hw_id = session_param->offline_hw_id[hw_index];
    if (session_hw_id == ISP_HW_MAX) {
       continue;
    }
    if (fetch_engine_cfg_request->isp_id_mask & (1 << session_hw_id) &&
        session_hw_id == session_param->offline_hw_id[0]) {
      /* forward streamon */
      offline_stream_info = &stream_param->stream_info;
      internel_event.type = MCT_EVENT_CONTROL_CMD;
      isp_util_fill_hal_params(MCT_EVENT_CONTROL_STREAMON, &internel_event,
        offline_stream_info, event->identity);
      ret = isp_util_forward_event_to_internal_pipeline(session_param,
        &internel_event, session_hw_id);
      if (ret == FALSE) {
        ISP_ERR("failed: ret %d", ret);
      }
    }
  }

ERROR:
  return ret;
}

/** isp_handler_module_offline_pipeline_config:
 *
 *  @module: mct module handle
 *  @port: port on which this event arrived
 *  @event: event to be handled
 *
 *  Handle set stream config
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_handler_module_offline_pipeline_config(mct_module_t *module,
  mct_port_t *port, mct_event_t *event)
{
  boolean                           ret = TRUE;
  iface_fetch_engine_cfg_t         *isp_cfg_request = NULL;
  uint32_t                          hw_index = 0, i = 0;
  isp_session_param_t              *session_param = NULL;
  isp_hw_update_list_params_t      *hw_update_list_params = NULL;
  isp_t                            *isp = NULL;
  mct_list_t                       *l_stream_params = NULL;
  mct_event_t                       internel_event;
  isp_stream_param_t               *stream_param = NULL;
  isp_zoom_params_t                *zoom_params_arr = NULL;
  cam_zoom_info_t                  *zoom;

  if (!module || !port || !event) {
    ISP_ERR("failed: %p %p %p", module, port, event);
    ret = FALSE;
    goto ERROR;
  }
  ISP_DBG("E ");
  /* Extract module private */
  isp = (isp_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    ret = FALSE;
    goto ERROR;
  }

  isp_cfg_request =
    (iface_fetch_engine_cfg_t *)event->u.module_event.module_event_data;
  if (isp_cfg_request == NULL) {
    ISP_ERR("failed, NULL pointer %p", isp_cfg_request);
    ret = FALSE;
    goto ERROR;
  }
  ret = isp_util_get_session_params(module,
    ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_params ret %d %p", ret,
      session_param);
    goto ERROR;
  }

  session_param->offline_frame_id = event->u.module_event.current_frame_id;

  /*if start then reconfig, if stop then only stop pipeline and break linking*/
  for (hw_index = 0; hw_index < ISP_HW_MAX; hw_index++) {
    isp_hw_id_t session_hw_id = session_param->offline_hw_id[hw_index];
    if (session_hw_id == ISP_HW_MAX) {
       continue;
    }
    if (isp_cfg_request->isp_id_mask & (1 << session_hw_id) &&
      session_hw_id == session_param->offline_hw_id[0]) {
      ISP_ERR("fetch_Dbg fetch config VFE%d START/REPROCESS/STOP %d (0/1/2)",
          session_hw_id, isp_cfg_request->fetch_cfg_cmd);
      if (isp_cfg_request->fetch_cfg_cmd == ISP_FETCH_PIPELINE_START) {
        /* connect isp sub modules with offline stream id */
        ret = isp_handler_create_internal_link_for_stream(module,
            session_param, event->identity, session_hw_id);
        if (ret == FALSE) {
          ISP_ERR("failed: create_internal_link_for_stream ret %d", ret);
          goto ERROR;
        }
        ret = isp_util_update_internal_pipeline(session_param, event->identity,
          session_param->offline_hw_id, session_param->offline_num_isp);
        if (ret == FALSE) {
          ISP_ERR("failed: isp_util_update_internal_pipeline");
          goto ERROR;
        }


        /*Extract meta data and send events to update the pipeline*/
        ret = isp_handler_process_offline_metadata(module, port, event);
        if (ret == FALSE) {
          ISP_ERR("failed: isp_handler_process_offline_metadata");
          goto ERROR;
        }

        if (session_param->multi_pass) {
          memset(&isp_cfg_request->ispif_split_output_info, 0,
            sizeof(ispif_out_info_t));
          isp_resource_request_offline_stripe_info(&isp->isp_resource,
            &isp_cfg_request->ispif_split_output_info,
            (void *)session_param, session_hw_id, event->identity);
          memset(&isp_cfg_request->isp_split_output_info, 0,
            sizeof(isp_out_info_t));
          isp_util_set_offline_split_output_info_per_stream(session_param,
            &isp_cfg_request->isp_split_output_info,
            session_hw_id, event->identity, TRUE);
        }

        /*Send streamon event to the pipeline */
        ret = isp_handler_streamon_offline_pipeline(module, port, event);
        if (ret == FALSE) {
          ISP_ERR("failed: isp_handler_streamon_offline_pipeline");
          goto ERROR;
        }

        ret = isp_util_forward_stats_update_to_pipeline(session_param,
          session_hw_id, event->identity);
        if (ret == FALSE) {
         ISP_ERR("failed: isp_util_forward_stats_update_to_pipeline");
         goto ERROR;
        }
        /*Do trigger update and hardware update for offline isp */
        ret = isp_handler_module_start_offline_pipeline(module, port, event);
        if (ret == FALSE) {
          ISP_ERR("failed: isp_handler_module_config_offline_pipeline");
          goto ERROR;
        }
        zoom_params_arr = &session_param->hw_update_params.
          hw_update_list_params[session_hw_id].stats_params->zoom_params[0];
        zoom = &session_param->hw_update_params.
          hw_update_list_params[session_hw_id].stats_params->zoom;
        ret = isp_util_send_zoom_crop(module, session_param, zoom_params_arr,
          zoom);
        if (ret == FALSE)
          ISP_ERR("falied: isp_util_send_zoom_crop");
      } else if (isp_cfg_request->fetch_cfg_cmd ==
        ISP_FETCH_PIPELINE_REPROCESS) {

        /*Extract meta data and send events to update the pipeline*/
        if (session_param->multi_pass) {
          isp_util_set_offline_split_output_info_per_stream(session_param,
            &isp_cfg_request->isp_split_output_info,
            session_hw_id, event->identity, FALSE);
          isp_handler_process_offline_overwrite(module, port, event);
          if (ret == FALSE) {
            ISP_ERR("failed: isp_handler_process_offline_metadata");
            goto ERROR;
          }
        } else {
          ret = isp_handler_process_offline_metadata(module, port, event);
          if (ret == FALSE) {
            ISP_ERR("failed: isp_handler_process_offline_metadata");
            goto ERROR;
          }
          ret = isp_util_forward_stats_update_to_pipeline(session_param,
            session_hw_id, event->identity);
          if (ret == FALSE) {
            ISP_ERR("failed: isp_handler_process_offline_metadata");
            goto ERROR;
          }
        }

        /*Do trigger update and hardware update for offline isp */
        ret = isp_handler_module_start_offline_pipeline(module, port, event);
        if (ret == FALSE) {
          ISP_ERR("failed: isp_handler_module_config_offline_pipeline");
          goto ERROR;
        }
        zoom_params_arr = &session_param->hw_update_params.
          hw_update_list_params[session_hw_id].stats_params->zoom_params[0];
        zoom = &session_param->hw_update_params.
          hw_update_list_params[session_hw_id].stats_params->zoom;
        ret = isp_util_send_zoom_crop(module, session_param, zoom_params_arr,
          zoom);
        if (ret == FALSE)
          ISP_ERR("falied: isp_util_send_zoom_crop");
      } else if (isp_cfg_request->fetch_cfg_cmd == ISP_FETCH_PIPELINE_STOP) {
        /* forward streamoff */
        l_stream_params = mct_list_find_custom(session_param->l_stream_params,
          &event->identity, isp_util_compare_identity_from_stream_param);
        if (l_stream_params == NULL) {
          ISP_ERR("failed: can not find stream param for this identity %x",
            event->identity);
          return FALSE;
        }
        stream_param = (isp_stream_param_t *)l_stream_params->data;

        memset(&internel_event, 0, sizeof(internel_event));
        internel_event.type = MCT_EVENT_MODULE_EVENT;
        internel_event.identity = event->identity;
        internel_event.direction = MCT_EVENT_DOWNSTREAM;
        internel_event.type = MCT_EVENT_CONTROL_CMD;
        mct_stream_info_t *offline_stream_info = &stream_param->stream_info;
        internel_event.u.ctrl_event.type = MCT_EVENT_CONTROL_STREAMOFF;
        internel_event.u.ctrl_event.control_event_data =
          (void *)offline_stream_info;
        ret = isp_util_forward_event_to_internal_pipeline(session_param,
          &internel_event, session_hw_id);
        if (ret == FALSE) {
          ISP_ERR("failed: ret %d", ret);
        }
         /* Destroys link and closes fd */
        isp_resource_destroy_internal_link_hw_id(port,
           (isp_port_data_t *)MCT_OBJECT_PRIVATE(port), event->identity, session_hw_id);
        session_param->hw_update_params.hw_update_list_params[session_hw_id].fd = 0;
        memset(
          session_param->hw_update_params.
          hw_update_list_params[session_hw_id].stats_params,
          0, sizeof(isp_saved_stats_params_t));
        memset(
          session_param->hw_update_params.
          hw_update_list_params[session_hw_id].applied_stats_params,
          0, sizeof(isp_saved_stats_params_t));
        session_param->hw_update_params.
          hw_update_list_params[session_hw_id].cur_cds_update = FALSE;
        session_param->hw_update_params.
          hw_update_list_params[session_hw_id].new_cds_update = FALSE;
        session_param->multi_pass = 0;
      } else {
        ISP_ERR("Incorrect isp config command received");
      }
    }
  }


ERROR:
  return ret;
}

/** isp_handler_module_offline_metadata:
 *
 *  @module: mct module handle
 *  @port: port on which this event arrived
 *  @event: event to be handled
 *
 *  Handle set stream config
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_handler_module_offline_metadata(
  mct_module_t *module,
  mct_port_t   *port __unused,
  mct_event_t  *event)
{
    boolean  ret = TRUE;
    isp_session_param_t  *session_param = NULL;
    ret = isp_util_get_session_params(module, ISP_GET_SESSION_ID(event->identity),
      &session_param);
    if (ret == FALSE || !session_param) {
      ISP_ERR("failed: isp_util_get_session_stream_params ret %d %p", ret,
      session_param);
      return FALSE;
    }
    isp_util_send_metadata(module, session_param, FALSE);

    return ret;
}

/** isp_handler_module_start_offline_pipeline:
 *
 *  @module: mct module handle
 *  @port: port on which this event arrived
 *  @event: event to be handled
 *
 *  Send trigger update and hardware update events
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_handler_module_start_offline_pipeline(mct_module_t *module,
  mct_port_t *port, mct_event_t *event)
{
  boolean                           ret = TRUE;
  isp_t                            *isp = NULL;
  iface_fetch_engine_cfg_t         *fetch_engine_cfg_request = NULL;
  isp_session_param_t              *session_param = NULL;
  isp_hw_id_t                       hw_index = 0;
  isp_hw_update_params_t           *hw_update_params = NULL;
  isp_hw_update_list_params_t      *hw_update_list_params = NULL;
  isp_fetch_eng_cfg_data_t         fetch_eng_cfg_data;
  isp_zoom_params_t                *zoom_params_arr = NULL;
  uint32_t                         frame_id = 0;
  struct timeval                    timestamp;

  if (!module || !port || !event) {
    ISP_ERR("failed: %p %p %p", module, port, event);
    return FALSE;
  }

  isp = (isp_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }

  ret = isp_util_get_session_params(module,
    ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  fetch_engine_cfg_request =
    (iface_fetch_engine_cfg_t *)event->u.module_event.module_event_data;
  if (fetch_engine_cfg_request == NULL) {
    ISP_ERR("failed, NULL pointer %p", fetch_engine_cfg_request);
    return FALSE;
  }

  ISP_DBG("dump_offline meta: input stream info ,id %x, W %d H %d",
    fetch_engine_cfg_request->input_stream_info.identity,
    fetch_engine_cfg_request->input_stream_info.dim.width,
    fetch_engine_cfg_request->input_stream_info.dim.height);

  PTHREAD_MUTEX_LOCK(&session_param->mutex);
  fetch_eng_cfg_data = session_param->fetch_eng_cfg_data;
  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);

  ISP_HIGH("dump_offline:  trigger update... event id %x", event->identity);
  /* Call internal hw update event */
  ret = isp_util_trigger_internal_hw_update_event(session_param,
    event->identity, 1, session_param->offline_hw_id,
    session_param->offline_num_isp, NULL);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_trigger_internal_hw_update_event");
    goto ERROR;
  }

  ret = isp_util_post_cds_update(module, session_param);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_update_hw_param");
  }

  hw_update_params = &session_param->hw_update_params;
  for (hw_index = 0; hw_index < ISP_HW_MAX; hw_index++) {
    isp_hw_id_t session_hw_id = session_param->offline_hw_id[hw_index];
    if (session_hw_id == ISP_HW_MAX) {
       continue;
    }
    if (fetch_engine_cfg_request->isp_id_mask & (1 << session_hw_id)  &&
        session_hw_id == session_param->offline_hw_id[0]) {
      /* if hw update overwrite, then append the list on to new hw update*/
      if (fetch_eng_cfg_data.is_hw_update_list_overwrite == TRUE) {
        if (fetch_eng_cfg_data.offline_hw_update_list == NULL)
          continue;

        ret = isp_hw_update_util_enqueue(hw_update_params,
          fetch_eng_cfg_data.offline_hw_update_list, session_hw_id);
        if (ret == FALSE) {
          ISP_ERR("failed: failed overwrite offline hw list hw_id %d", session_hw_id);
        }

        fetch_eng_cfg_data.offline_hw_update_list = NULL;
      }
      /* Update hw_update_list */
      hw_update_list_params = &hw_update_params->hw_update_list_params[hw_index];
      if (!hw_update_list_params->cur_hw_update_list) {
        ISP_ERR("cur_hw_update_list is NULL !");
        continue;
      }
    }
  }

  ISP_HIGH("dump_offline: do ioctl ... ");
  /* Do iotcl to configure hw */
  ret = isp_hw_update_util_do_ioctl_in_hw_id_update_params(&isp->isp_resource,
    session_param, session_param->offline_hw_id[0]);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_hw_update_do_ioctl");
    goto ERROR;
  }

  ret = isp_util_update_hw_param_offline(module, session_param,
    session_param->offline_hw_id[0]);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_update_hw_param");
    goto ERROR;
  }

ERROR:
  return ret;
}

/** isp_handler_module_print_zoom_param:
 *
 *  @module: mct module handle
 *  @port: port on which this event arrived
 *  @event: event to be handled
 *
 *  Handle set stream config
 *
 *  Return TRUE on success and FALSE on failure
 **/
void isp_handler_module_print_zoom_param(
    isp_zoom_params_t *zoom_params, // Remove __unused to enable print.
    int                num_params)
{
  int i;

  if (!zoom_params) {
      ISP_ERR("failed: zoom_params %p", zoom_params);
      return;
  }

  for (i = 0; i < num_params; i++) {
    ISP_DBG("%d ==================================================",
        i);
    ISP_DBG("identity %d", zoom_params[i].identity);
    ISP_DBG("isp_output->width %d, isp_output->height %d",
        zoom_params[i].isp_output.width, zoom_params[i].isp_output.height);
    ISP_DBG("crop_window->x %d, crop_window->y %d, crop_window->crop_out_x %d, crop_window->crop_out_y %d",
        zoom_params[i].crop_window.x, zoom_params[i].crop_window.y,
        zoom_params[i].crop_window.crop_out_x,
        zoom_params[i].crop_window.crop_out_y);
    ISP_DBG("camif_output->width %d, camif_output->height %d",
        zoom_params[i].camif_output.width, zoom_params[i].camif_output.height);
    ISP_DBG("scaler_output->width %d, scaler_output->height %d",
        zoom_params[i].scaler_output.width,
        zoom_params[i].scaler_output.height);
    ISP_DBG("scaling_ratio %f", zoom_params[i].scaling_ratio);
    ISP_DBG("fov_output->x %d, fov_output->y %d, fov_output->crop_out_x %d, fov_output->crop_out_y %d",
        zoom_params[i].fov_output.x,
        zoom_params[i].fov_output.y, zoom_params[i].fov_output.crop_out_x,
        zoom_params[i].fov_output.crop_out_y);
  }
}

/** isp_handler_module_send_initial_crop_info:
 *
 *  @module: mct module handle
 *  @port: port on which this event arrived
 *  @event: event to be handled
 *
 *  Handle set stream config
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_handler_module_send_initial_crop_info(
    mct_module_t *module,
    mct_port_t   *port __unused,
    mct_event_t  *event)
{
  boolean ret;
  mct_event_t         event_for_sent;
  isp_private_event_t private_event;
  isp_zoom_params_t   upd_zoom_params[ISP_MAX_STREAMS];
  isp_session_param_t *session_param = NULL;
  cam_zoom_info_t     zoom;
  struct timeval      timestamp;

  ret = isp_util_get_session_params(module,
    ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_params ret %d %p", ret,
      session_param);
    return FALSE;
  }
  if (session_param->num_isp == 0) {
    ISP_ERR("No VFE Resource, Do early exit");
    return TRUE;
  }
  memset(&event_for_sent, 0, sizeof(event_for_sent));
  memset(&private_event, 0, sizeof(private_event));
  memset(&upd_zoom_params, 0, sizeof(upd_zoom_params));
  memset(&zoom, 0, sizeof(cam_zoom_info_t));

  private_event.type = ISP_PRIVATE_REQUEST_ZOOM_PARAMS;
  private_event.data = &upd_zoom_params;

  event_for_sent.type = MCT_EVENT_MODULE_EVENT;
  event_for_sent.identity = event->identity;
  event_for_sent.direction = MCT_EVENT_DOWNSTREAM;
  event_for_sent.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
  event_for_sent.u.module_event.module_event_data = &private_event;

  ret = isp_util_forward_event_to_all_internal_pipelines(session_param,
    &event_for_sent);
  if (ret == FALSE) {
    ISP_ERR("failed: event type ISP_PRIVATE_REQUEST_ZOOM_PARAMS");
  }

  isp_handler_module_print_zoom_param(upd_zoom_params, ISP_MAX_STREAMS);

  gettimeofday(&timestamp, NULL);
  ret = isp_util_broadcast_crop_info(module, session_param,
    0, &timestamp, upd_zoom_params, &zoom, TRUE);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_broadcast_crop_info");
    return ret;
  }

  ret = isp_send_stats_types(module, event, session_param);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_broadcast_crop_info");
    return ret;
  }

  return ret;
}

/** isp_handler_module_set_stream_config:
 *
 *  @module: mct module handle
 *  @port: port on which this event arrived
 *  @event: event to be handled
 *
 *  Handle set stream config
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_handler_module_set_stream_config(mct_module_t *module,
  mct_port_t *port, mct_event_t *event)
{
  boolean              ret = TRUE;
  isp_t               *isp = NULL;
  isp_session_param_t *session_param = NULL;
  sensor_out_info_t   *sensor_out_info = NULL;

  if (!module || !port || !event) {
    ISP_ERR("failed: %p %p %p", module, port, event);
    return FALSE;
  }

  isp = (isp_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }

  ret = isp_util_get_session_params(module,
    ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&session_param->mutex);

  sensor_out_info =
    (sensor_out_info_t *)event->u.module_event.module_event_data;
  session_param->sensor_fmt = sensor_out_info->fmt;

  if (session_param->state == ISP_STATE_IDLE) {
    /* Sensor settings changed, clear stats config residue */
    /* Clear stats_config_update event in saved_events */
    ret = isp_util_clear_invalid_saved_event(&session_param->saved_events);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_clear_stats_config_update");
    }

    /* reset valid flag on first stream config */
    ret = isp_util_saved_event_reset_valid_flag(&session_param->saved_events);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_saved_event_reset_valid_flag");
    }
  }

  /* Forward stream ON event to pipelines */
  if (session_param->session_based_ide) {
    ret = isp_util_forward_event_to_session_based_stream_all_int_pipelines(
      session_param, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_forward_event_to_pipelines");
    }
  } else {
    ret = isp_util_forward_event_to_all_streams_all_internal_pipelines(
      session_param, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_forward_event_to_pipelines");
    }
  }

  /* Forward event to downstream */
  ret = isp_util_forward_event(port, event);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_forward_event");
  }

ERROR:
  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
  return ret;
}

/** isp_handler_module_iface_request_output_resource:
 *
 *  @module: mct module handle
 *  @port: port on which this event arrived
 *  @event: event to be handled
 *
 *  Return IFACE output resource
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_handler_module_iface_request_output_resource(mct_module_t *module,
  mct_port_t *port, mct_event_t *event)
{
  boolean              ret = TRUE;
  isp_t               *isp = NULL;
  isp_session_param_t *session_param = NULL;

  if (!module || !port || !event) {
    ISP_ERR("failed: %p %p %p", module, port, event);
    return FALSE;
  }

  isp = MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }

  ret = isp_util_get_session_params(module, ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_stream_params ret %d %p",
      ret, session_param);
    return FALSE;
  }

  ret = isp_util_forward_saved_events(session_param,
    &session_param->saved_events, FALSE);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_forward_saved_events");
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&session_param->state_mutex);
  session_param->state = ISP_STATE_CONFIGURING;
  PTHREAD_MUTEX_UNLOCK(&session_param->state_mutex);

  ret = isp_resource_iface_request_output_resource(module, port, event);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_resource_iface_request_output_resource");
    return FALSE;
  }

  return ret;
}

/** isp_handler_module_iface_request_offline_output_resource:
 *
 *  @module: mct module handle
 *  @port: port on which this event arrived
 *  @event: event to be handled
 *
 *  Return IFACE output resource
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_handler_module_iface_request_offline_output_resource(
  mct_module_t *module, mct_port_t *port, mct_event_t *event)
{
  boolean ret = TRUE;

  if (!module || !port || !event) {
    ISP_ERR("failed: %p %p %p", module, port, event);
    return FALSE;
  }

  ret = isp_resource_iface_request_offline_isp(module, port, event);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_resource_iface_request_offline_output_resource");
    return FALSE;
  }

  ret = isp_resource_iface_request_offline_output_resource(module, port, event);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_resource_iface_request_offline_output_resource");
    return FALSE;
  }

  return ret;
}

enum msm_vfe_axi_stream_src isp_handler_module_iface_convert_hw_stream
        (isp_hw_streamid_t hw_stream)
{
  enum msm_vfe_axi_stream_src ret_axi_stc = VFE_AXI_SRC_MAX;

  switch (hw_stream)
  {
    case ISP_HW_STREAM_VIEWFINDER:
      ret_axi_stc = PIX_VIEWFINDER;
      break;
    case ISP_HW_STREAM_ENCODER:
      ret_axi_stc = PIX_ENCODER;
      break;
    case ISP_HW_STREAM_VIDEO:
      ret_axi_stc = PIX_VIDEO;
      break;
    case ISP_HW_STREAM_IDEAL_RAW:
      ret_axi_stc = IDEAL_RAW;
      break;
    default:
      break;
  }

  return ret_axi_stc;
}

static void isp_handler_update_super_dim(cam_dimension_t *super_dim_out,
        cam_dimension_t *stream_info_dim_in, sensor_set_dim_t *sensor_output_info)
{
  boolean  same_aspect_ratio;
  float sensor_aspect_ratio, curr_aspect_ratio;

  ISP_HIGH("%s: Enter SuperDim=[%d * %d] InputDim=[%d * %d]\n",
      __func__, super_dim_out->width,super_dim_out->height,
                stream_info_dim_in->width, stream_info_dim_in->height);

  if (super_dim_out->width == 0) {
    super_dim_out->width = stream_info_dim_in->width;
    super_dim_out->height = stream_info_dim_in->height;
    ISP_HIGH("%s:  First time width is 0\n",__func__);
  } else {
    if (super_dim_out->width  * stream_info_dim_in->height ==
        super_dim_out->height * stream_info_dim_in->width) {
      same_aspect_ratio = TRUE;
      ISP_HIGH("%s: Same aspect ratio\n",__func__);
    } else {
      same_aspect_ratio = FALSE;
      ISP_HIGH("%s: not same aspect ratio\n",__func__);
    }

    if (stream_info_dim_in->width > super_dim_out->width) {
      super_dim_out->width = stream_info_dim_in->width;
    }
    if (stream_info_dim_in->height > super_dim_out->height) {
      super_dim_out->height = stream_info_dim_in->height;
    }

    /* If the aspect ratio of the streams sharing the HW stream
           don't match then we need to deriver a bigger dimension
           buffer to overlap the FOV for all the streams */
    if (!same_aspect_ratio) {
      sensor_aspect_ratio = (float)sensor_output_info->dim_output.width /
                                   sensor_output_info->dim_output.height;
      curr_aspect_ratio = (float)super_dim_out->width /
                                 super_dim_out->height;

      if (curr_aspect_ratio > sensor_aspect_ratio) {
        super_dim_out->height = super_dim_out->width / sensor_aspect_ratio;
        super_dim_out->height = FLOOR2(super_dim_out->height);
      } else {
        super_dim_out->width = super_dim_out->height * sensor_aspect_ratio;
        super_dim_out->width = FLOOR2(super_dim_out->width);
      }
    }
  }
  ISP_HIGH("%s: Dimension After aspect ratio match [%d * %d]\n",
      __func__, super_dim_out->width,super_dim_out->height);
}

/** isp_handler_module_iface_request_stream_mapping_info:
 *
 *  @module: mct module handle
 *  @port: port on which this event arrived
 *  @event: event to be handled
 *
 *  Return IFACE output resource
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_handler_module_iface_request_stream_mapping_info(
  mct_module_t *module, mct_port_t *port, mct_event_t *event)
{
  boolean              ret = TRUE;
  isp_t               *isp = NULL;
  isp_session_param_t *session_param = NULL;
  uint32_t             i;
  iface_buf_alloc_t   *buf_alloc_info = NULL;
  enum msm_vfe_axi_stream_src tmp_axi_src = VFE_AXI_SRC_MAX;

  if (!module || !port || !event) {
    ISP_ERR("failed: %p %p %p", module, port, event);
    return FALSE;
  }

  isp = MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }

  ret = isp_util_get_session_params(module, ISP_GET_SESSION_ID(event->identity),
                        &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_stream_params ret %d %p",
      ret, session_param);
    return FALSE;
  }

  buf_alloc_info =
    (iface_buf_alloc_t *)event->u.module_event.module_event_data;
  if (!buf_alloc_info) {
    ISP_ERR("failed: resource_request %p", buf_alloc_info);
    return FALSE;
  }

  buf_alloc_info->num_pix_stream = session_param->stream_port_map.num_streams;

  for (i = 0; i < session_param->stream_port_map.num_streams; i++) {
    ISP_HIGH("INFO: type %d resolution %dx%d hw_stream %d need_native_buff %d "
      "controllable_output %d shared_output %d is_changed %d changed resolution %dx%d\n",
      session_param->stream_port_map.streams[i].stream_type,
      session_param->stream_port_map.streams[i].stream_sizes.width,
      session_param->stream_port_map.streams[i].stream_sizes.height,
      session_param->stream_port_map.streams[i].hw_stream,
      session_param->stream_port_map.streams[i].native_buffer,
      session_param->stream_port_map.streams[i].controlable_output,
      session_param->stream_port_map.streams[i].shared_output,
      session_param->stream_port_map.streams[i].is_changed,
      session_param->stream_port_map.streams[i].changed_stream_sizes.width,
      session_param->stream_port_map.streams[i].changed_stream_sizes.height);

     if (session_param->stream_port_map.streams[i].is_changed) {
       buf_alloc_info->stream_info[i].dim =
         session_param->stream_port_map.streams[i].changed_stream_sizes;
     } else {
       buf_alloc_info->stream_info[i].dim =
         session_param->stream_port_map.streams[i].stream_sizes;
     }
    buf_alloc_info->stream_info[i].use_native_buffer =
        session_param->stream_port_map.streams[i].native_buffer;
    buf_alloc_info->stream_info[i].stream_type =
        session_param->stream_port_map.streams[i].stream_type;
    buf_alloc_info->stream_info[i].axi_stream_src =
        isp_handler_module_iface_convert_hw_stream
        (session_param->stream_port_map.streams[i].hw_stream);

    //Update the super dim info per axi stream
    if (buf_alloc_info->stream_info[i].use_native_buffer) {
      tmp_axi_src = buf_alloc_info->stream_info[i].axi_stream_src;
      cam_dimension_t *super_dim_out =
              &buf_alloc_info->hw_stream_super_dim[tmp_axi_src];
      cam_dimension_t *stream_info_dim_in =
              &buf_alloc_info->stream_info[i].dim;

      isp_handler_update_super_dim(super_dim_out, stream_info_dim_in,
              &session_param->sensor_output_info);
    }

  }

  return ret;
}

/** isp_handler_sensor_output_dim:
 *
 *  @module: mct module handle
 *  @event: event to be handled
 *
 *  Handle sensor output dimension
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_handler_sensor_output_dim(mct_module_t *module, mct_event_t *event)
{
  boolean              ret = TRUE;
  isp_session_param_t *session_param = NULL;
  sensor_set_dim_t    *sen_output = NULL;

  if (!module || !event) {
    ISP_ERR("failed: module %p event %p", module, event);
    return FALSE;
  }

  ret = isp_util_get_session_params(module, ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_stream_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  sen_output = (sensor_set_dim_t *)event->u.module_event.module_event_data;
  if (!sen_output) {
    ISP_ERR("failed: sensor output info");
    return FALSE;
  }
  session_param->sensor_output_info = *sen_output;

  return TRUE;
} /* isp_handler_sensor_output_dim */

/** isp_handler_event:
 *
 *  @module: isp module handle
 *  @port: port on which this event arrived
 *  @event: event to be handled
 *
 *  If session is not streaming, handle event in caller
 *  thread context, if session is already streaming, enqueue
 *  event to be handled by trigger update thread
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_handler_event(mct_module_t *module, mct_port_t *port,
  mct_event_t *event)
{
  boolean              ret = TRUE;
  isp_session_param_t *session_param = NULL;
  mct_event_t         *copy_event_data = NULL;
  uint32_t             size;
  mct_port_caps_type_t forward_event_to_type = 0;
  boolean              upstream_event = FALSE;
  mct_module_t        *port_module = NULL;

  if (!module || !port || !event) {
    ISP_ERR("failed: %p %p %p", module, port, event);
    return FALSE;
  }

  ret = isp_util_get_session_params(module, ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  if (event->direction == MCT_EVENT_UPSTREAM) {
    upstream_event = TRUE;
    if (port->caps.port_caps_type == MCT_PORT_CAPS_STATS) {
      forward_event_to_type = MCT_PORT_CAPS_FRAME;
    } else {
      forward_event_to_type = MCT_PORT_CAPS_STATS;
    }
    /* Forward event to upstream */
    ret = isp_util_forward_event(port, event);
    if (ret == FALSE) {
      ISP_DBG("failed: isp_util_forward_event");
    }
    if (event->u.module_event.type == MCT_EVENT_MODULE_OUTPUT_BUFF_LIST ||
       event->u.module_event.type == MCT_EVENT_MODULE_BAYERPROC_OUTPUT_DIM ||
       event->u.module_event.type == MCT_EVENT_MODULE_BUF_DIVERT ||
       event->u.module_event.type == MCT_EVENT_MODULE_BUF_DIVERT_ACK)
        return ret;
  }
  if (event->type == MCT_EVENT_MODULE_EVENT &&
      (event->u.module_event.type == MCT_EVENT_MODULE_STATS_PDAF_UPDATE ||
       event->u.module_event.type == MCT_EVENT_MODULE_EZTUNE_GET_AFTUNE)) {
    return ret;
  }

  /* Change event direction to downstream */
  event->direction = MCT_EVENT_DOWNSTREAM;

  PTHREAD_MUTEX_LOCK(&session_param->mutex);
  /* Copy event to saved events */
  ret = isp_trigger_thread_enqueue_event(event, &session_param->saved_events,
    session_param->session_based_ide);
  if (ret == FALSE) {
    if (event->type == MCT_EVENT_CONTROL_CMD) {
      ISP_ERR("failed: isp_trigger_thread_enqueue_event type %d",
        event->u.ctrl_event.type);
    } else if (event->type == MCT_EVENT_MODULE_EVENT) {
      ISP_ERR("failed: isp_trigger_thread_enqueue_event type %d",
        event->u.module_event.type);
    }
  }

  PTHREAD_MUTEX_LOCK(&session_param->state_mutex);
  if (session_param->state == ISP_STATE_CONFIGURING) {
    PTHREAD_MUTEX_UNLOCK(&session_param->state_mutex);
    /* Forward event downstream in caller thread context */
    if (session_param->session_based_ide) {
      ret = isp_util_forward_event_to_session_based_stream_all_int_pipelines(
        session_param, event);
      if (ret == FALSE) {
        ISP_ERR("failed: isp_util_forward_event_to_pipelines");
      }
    } else {
      ret = isp_util_forward_event_to_all_streams_all_internal_pipelines(
        session_param, event);
      if (ret == FALSE) {
        ISP_ERR("failed: isp_util_forward_event_to_pipelines");
      }
    }
  } else if (session_param->state == ISP_STATE_STREAMING) {
    PTHREAD_MUTEX_UNLOCK(&session_param->state_mutex);
    /* Copy event to trigger update queue */
    PTHREAD_MUTEX_LOCK(&session_param->trigger_update_params.mutex);
    ret = isp_trigger_thread_enqueue_event(event,
      &session_param->trigger_update_params.new_events,
      session_param->session_based_ide);
    if (ret == FALSE) {
      if (event->type == MCT_EVENT_CONTROL_CMD) {
        ISP_ERR("failed: isp_trigger_thread_enqueue_event type %d",
          event->u.ctrl_event.type);
      } else if (event->type == MCT_EVENT_MODULE_EVENT) {
        ISP_ERR("failed: isp_trigger_thread_enqueue_event type %d",
          event->u.module_event.type);
      }
    }
    PTHREAD_MUTEX_UNLOCK(&session_param->trigger_update_params.mutex);
  } else {
    PTHREAD_MUTEX_UNLOCK(&session_param->state_mutex);
   }


  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);

  if (upstream_event == TRUE) {
    /* forward to only one stats / frame port */
    ret = isp_util_forward_event_downstream_to_type(module, event,
      forward_event_to_type);
    if (ret == FALSE) {
      ISP_DBG("failed: isp_util_divert_event_downstream_to_type type %d",
        event->u.module_event.type);
    }
  } else {
    /* Forward event downstream */
    ret = isp_util_forward_event(port, event);
    if (ret == FALSE) {
      ISP_DBG("failed: isp_util_forward_event");
    }
  }

  return ret;
} /* isp_handler_event */

/** isp_handler_compare_bundle_info_in_session_param
 *
 *  @data1: handle to cam_bundle_config_t
 *  @data2: handle to cam_bundle_config_t
 *
 *  Return TRUE if data1 and data2 match, FALSE otherwise
 **/
static boolean isp_handler_compare_bundle_info_in_session_param(void *data1,
  void *data2)
{
  isp_bundle_params_t *bundle_params = (isp_bundle_params_t *)data1;
  cam_bundle_config_t *bundle_info = (cam_bundle_config_t *)data2;

  if (!data1 || !data2) {
    ISP_ERR("failed: data1 %p data2 %p", data1, data2);
    return FALSE;
  }

  if (!memcmp(&bundle_params->bundle_info, bundle_info,
    sizeof(cam_bundle_config_t))) {
    return TRUE;
  }
  return FALSE;
}

/** isp_handler_update_bundle_in_stream_param:
 *
 *  @data: handle of type isp_stream_param_t
 *  @user_data: handle of type isp_bundle_param_t
 *
 *  Update bundle_param in stream param
 **/
static boolean isp_handler_update_bundle_in_stream_param(void *data,
  void *user_data)
{
  uint32_t             i = 0;
  isp_stream_param_t  *stream_param = (isp_stream_param_t *)data;
  isp_bundle_params_t *bundle_params = (isp_bundle_params_t *)user_data;
  cam_bundle_config_t *bundle_info = NULL;

  if (!data || !user_data) {
    ISP_ERR("failed: data %p user_data %p", data, user_data);
    return TRUE;
  }

  bundle_info = &bundle_params->bundle_info;
  for (i = 0; i < bundle_info->num_of_streams; i++) {
    if (ISP_GET_STREAM_ID(stream_param->stream_info.identity) ==
        bundle_info->stream_ids[i]) {
      stream_param->bundle_params[stream_param->bundle_params_cnt++] = bundle_params;
      break;
    }
  }

  return TRUE;
}

/** isp_handler_set_bundle_info:
 *
 *  @module: module handle
 *  @event: event to be handled
 *
 *  Find whether this bundle is already passed by HAL. If not,
 *  store it in session param
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_handler_set_bundle_info(mct_module_t *module,
  mct_event_t *event)
{
  boolean                   ret = TRUE;
  isp_session_param_t      *session_param = NULL;
  isp_bundle_params_t      *bundle_params = NULL;
  mct_list_t               *l_bundle_params = NULL;
  cam_stream_parm_buffer_t *stream_parm = NULL;
  cam_bundle_config_t      *bundle_info = NULL;
  boolean                   allocate_bundle = FALSE;

  if (!module || !event) {
    ISP_ERR("failed: module %p event %p", module, event);
    return FALSE;
  }

  stream_parm =
    (cam_stream_parm_buffer_t *)event->u.ctrl_event.control_event_data;
  if (!stream_parm) {
    ISP_ERR("failed: stream_parm %p", stream_parm);
    return FALSE;
  }

  bundle_info = &stream_parm->bundleInfo;

  //PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(module));

  ret = isp_util_get_session_params(module, ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  if (!session_param->l_bundle_params) {
    allocate_bundle = TRUE;
  } else {
    /* Check whether this bundle info is already present in the list */
    l_bundle_params = mct_list_find_custom(session_param->l_bundle_params,
      bundle_info, isp_handler_compare_bundle_info_in_session_param);
    if (!l_bundle_params) {
      allocate_bundle = TRUE;
    }
  }

  if (allocate_bundle == TRUE) {
    bundle_params = (isp_bundle_params_t *)CAM_MALLOC(sizeof(*bundle_params));
    if (!bundle_params) {
      ISP_ERR("failed: to alloc mem");
      ret = FALSE;
      goto ERROR;
    }
    memset(bundle_params, 0, sizeof(*bundle_params));
    memcpy(&bundle_params->bundle_info, bundle_info,
      sizeof(bundle_params->bundle_info));

    /* Update bundle params in stream param */
    mct_list_traverse(session_param->l_stream_params,
      isp_handler_update_bundle_in_stream_param, bundle_params);

    l_bundle_params = mct_list_append(session_param->l_bundle_params,
      bundle_params, NULL, NULL);
    if (!l_bundle_params) {
      ISP_ERR("failed: to append bundle params");
      ret = FALSE;
      goto ERROR;
    }
    session_param->l_bundle_params = l_bundle_params;
  }

ERROR:
  //PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
  return ret;
}

/** isp_handler_control_parm_stream_buf:
 *
 *  @module: module handle
 *  @port: port handle
 *  @event: event to be handled
 *
 *  Handle control parm stream buf event sent by HAL
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_handler_control_parm_stream_buf(mct_module_t *module,
  mct_port_t *port, mct_event_t *event)
{
  boolean                   ret = TRUE;
  cam_stream_parm_buffer_t *stream_parm = NULL;

  if (!module || !port || !event) {
    ISP_ERR("failed: %p %p %p", module, port, event);
    return FALSE;
  }
  stream_parm =
    (cam_stream_parm_buffer_t *)event->u.ctrl_event.control_event_data;
  if (!stream_parm) {
    ISP_ERR("failed: stream_parm %p", stream_parm);
    return FALSE;
  }
  switch (stream_parm->type) {
  case CAM_STREAM_PARAM_TYPE_SET_BUNDLE_INFO: {
    ret = isp_handler_set_bundle_info(module, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_handler_set_bundle_info");
    }
    /* Forward event downstream */
    ret = isp_util_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_forward_event");
    }
  }
    break;
  default:
    /* Forward event downstream */
    ret = isp_util_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_forward_event");
    }
    break;
  }
  return ret;
}

/** isp_handler_control_set_parm:
 *
 *  @module: module handle
 *  @port: port handle
 *  @event: event to be handled
 *
 *  Handle set parm event sent by HAL
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_handler_control_set_parm(mct_module_t *module, mct_port_t *port,
  mct_event_t *event)
{
  boolean                    ret = TRUE;
  mct_event_control_parm_t  *parm = NULL;
  isp_session_param_t *session_param = NULL;

  if (!module || !port || !event) {
    ISP_ERR("failed: %p %p %p", module, port, event);
    return FALSE;
  }

  parm =(mct_event_control_parm_t *)(event->u.ctrl_event.control_event_data);
  if (!parm) {
    ISP_ERR("failed: parm %p", parm);
    return FALSE;
  }

  ret = isp_util_get_session_params(module, ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  switch (parm->type) {
  default:
    /* Forward stream ON event to pipelines */
    ret = isp_util_forward_event_to_all_internal_pipelines(session_param,
      event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_forward_event_to_pipelines");
    }
    /* Forward event downstream */
    ret = isp_util_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_forward_event");
    }
    break;
  }
  return ret;
}

/** isp_handler_create_internal_link:
 *
 *  @port: port handle
 *  @stream_info: stream info handle
 *
 *  Create internal pipeline link with all sub moudules
 *
 *  Return TRUE on sucess and FALSE on failure
 **/
boolean isp_handler_create_internal_link(mct_port_t *port,
  mct_stream_info_t *stream_info)
{
  isp_session_param_t *session_param;
  mct_module_t        *module = NULL;
  isp_t               *isp = NULL;
  uint32_t             session_id;
  boolean              ret = TRUE;

  if (!port || !stream_info) {
    ISP_ERR("failed: port %p stream_info %p", port, stream_info);
    return FALSE;
  }

  /* Extract module handle */
  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return FALSE;
  }

  session_id = ISP_GET_SESSION_ID(stream_info->identity);
  ret = isp_util_get_session_params(module, session_id, &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  /* Check if ISP resource is acquired */
  if (!session_param->num_isp) {
    if (stream_info->stream_type == CAM_STREAM_TYPE_PARM) {
      /* Session stream will be link internally after stream mapping */
      return TRUE;
    } else {
      ISP_ERR("Only session stream can be linked before ISP res allocation");
      return FALSE;
    }
  }

  /* Extract module private */
  isp = (isp_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }

  ret = isp_resource_create_link(module, port, &isp->isp_resource, stream_info);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_pipeline_create_link");
  }

  return ret;
}

/** isp_handler_create_internal_link_for_stream:
 *
 *  @module: mct module handle
 *  @session_param: handle to store session param
 *
 *  Create internal pipeline link with all sub moudules
 *
 *  Return TRUE on sucess and FALSE on failure
 **/
boolean isp_handler_create_internal_link_for_stream(
  mct_module_t *module, isp_session_param_t *session_param,
  unsigned int identity, isp_hw_id_t hw_id)
{
  boolean               ret = TRUE;
  mct_port_t           *port = NULL;
  isp_t                *isp = NULL;
  mct_list_t           *l_port = NULL;
  mct_list_t           *l_stream_params = NULL;
  isp_stream_param_t   *stream_param = NULL;

  if (!module || !session_param) {
    ISP_ERR("failed: module %p session_param %p", module, session_param);
    return FALSE;
  }

  /* Extract module private */
  isp = (isp_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }

  /* find stream_params for particular identity */
  l_stream_params = mct_list_find_custom(session_param->l_stream_params,
    &identity, isp_util_compare_identity_from_stream_param);
  if (l_stream_params) {
    stream_param = (isp_stream_param_t *)l_stream_params->data;
  } else {
    ISP_ERR("failed: fetch_dbg can not find stream param for this identity %x",
      identity);
    return FALSE;
  }

  /* find sink port */
  l_port = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
    &stream_param->stream_info.identity, isp_util_find_port_based_on_identity);
  if (!l_port || !l_port->data) {
    ISP_ERR("failed: sinkport");
    return FALSE;
  }
  port = (mct_port_t *)l_port->data;

  ret = isp_resource_create_link_without_alloc(module, port,
    &isp->isp_resource, &stream_param->stream_info, hw_id);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_pipeline_create_link");
  }

  return ret;
}

/** isp_handler_destroy_stream_param:
 *
 *  @port: port on which stream param need to be removed
 *  @identity: stream identity
 *
 *  Remove stream param from stream param list
 *
 *  Return void
 **/
void isp_handler_destroy_stream_param(mct_port_t *port, unsigned int identity)
{
  boolean                      ret = TRUE;
  isp_session_param_t         *session_param = NULL;
  isp_stream_param_t          *stream_param = NULL;
  mct_module_t                *module = NULL;
  isp_bundle_params_t         *bundle_params = NULL;
  mct_list_t                  *l_bundle_params = NULL;
  void                        *temp_params = NULL;
  mct_list_t                  *l_stream_params = NULL;

  if (!port) {
    ISP_ERR("failed %p", port);
    return;
  }

  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed: %p", module);
    return;
  }

  ret = isp_util_get_session_stream_params(module, identity, &session_param,
    &stream_param);
  if (ret == FALSE || !session_param || !stream_param) {
    ISP_ERR("failed: isp_util_get_session_stream_params ret %d %p %p",
      ret, session_param, stream_param);
    return;
  }

  PTHREAD_MUTEX_LOCK(&session_param->mutex);
  uint32_t cnt = stream_param->bundle_params_cnt;
  for(uint32_t i=0; i < cnt; ++i){
  bundle_params = stream_param->bundle_params[i];
  stream_param->bundle_params[i] = NULL;
  if (bundle_params) {
    if ((++bundle_params->caps_unreserve_count) ==
        bundle_params->bundle_info.num_of_streams) {
      /* Remove bundle params from bundle list */
      temp_params = (void *)bundle_params;
      session_param->l_bundle_params = mct_list_remove(
        session_param->l_bundle_params, bundle_params);
      free(temp_params);
      (stream_param->bundle_params_cnt)--;
    }
  }
}
  if (!(--stream_param->ref_count)) {
    temp_params = (void *)stream_param;
    session_param->l_stream_params = mct_list_remove(
      session_param->l_stream_params, (void *)stream_param);
    free(temp_params);
  }

  /* Check whether some other stream is present */
  l_stream_params = mct_list_find_custom(session_param->l_stream_params, NULL,
    isp_util_compare_hal_stream_type);
  if (!l_stream_params) {
    session_param->stream_port_map.num_streams = 0;
    /* Reset stats mask */
    session_param->parser_params.stats_ctrl.stats_mask = 0;
    session_param->parser_params.stats_ctrl.parse_mask = 0;
    memset(&session_param->parser_params.stats_ctrl.stats_comp_grp_mask, 0,
      sizeof(uint32_t) *
      session_param->parser_params.stats_ctrl.num_stats_comp_grp);
    session_param->parser_params.stats_ctrl.num_stats_comp_grp = 0;

    ISP_DBG("num_streams %d", session_param->stream_port_map.num_streams);
  }

  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
}

/** isp_handler_create_stream_param:
 *
 *  @port: port handle
 *  @peer_caps: peer capabilities
 *  @stream_info: mct stream info
 *
 *  Create new stream info and associate with session
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_handler_create_stream_param(
    mct_port_t        *port,
    void              *peer_caps __unused,
    mct_stream_info_t *stream_info)
{
  boolean              ret = TRUE;
  mct_module_t        *module = NULL;
  isp_session_param_t *session_param = NULL;
  uint32_t             session_id = 0;
  mct_list_t          *l_stream_params = NULL;
  isp_stream_param_t  *stream_param = NULL;
  enum msm_vfe_axi_stream_src axi_src = VFE_AXI_SRC_MAX;

  if (!port || !stream_info) {
    ISP_ERR("failed: port %p stream_info %p", port, stream_info);
    return FALSE;
  }

  session_id = ISP_GET_SESSION_ID(stream_info->identity);

  /* Extract module handle */
  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return FALSE;
  }

  ret = isp_util_get_session_params(module, session_id, &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  //PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(module));

  /* Check whether stream param already exist for this identity */
  l_stream_params = mct_list_find_custom(session_param->l_stream_params,
    &stream_info->identity, isp_util_compare_identity_from_stream_param);
  if (l_stream_params) {
    stream_param = (isp_stream_param_t *)l_stream_params->data;
    if (stream_param) {
      stream_param->ref_count++;
    }
    //PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
    return TRUE;
  }

  /* Create ctrl struct for this stream */
  stream_param = (isp_stream_param_t *)CAM_MALLOC(sizeof(*stream_param));
  if (!stream_param) {
    ISP_ERR("failed: isp_stream_param_t %p", stream_param);
    return FALSE;
  }

  memset(stream_param, 0, sizeof(*stream_param));
  pthread_mutex_init(&stream_param->mutex, NULL);
  stream_param->stream_info = *stream_info;
  isp_util_update_stream_info_dims_for_rotation(&stream_param->stream_info);
  stream_param->ref_count++;
  stream_param->stream_state = ISP_STREAM_STATE_INACTIVE;
  stream_param->bundle_params_cnt = 0;
  if (stream_info->stream_type == CAM_STREAM_TYPE_PARM) {
    /* Assign session based identity to module private */
    session_param->session_based_ide = stream_info->identity;
  }

  axi_src =
    isp_util_get_axi_src_type(stream_param->stream_info.fmt, 0);
  if (axi_src == CAMIF_RAW || axi_src > IDEAL_RAW) {
    ISP_WARN("axi src = %d (2 - CAMIF, 4/5/6 - RDI raw), "
      "ISP pipeline not support, Ext link without internel link!", axi_src);
    stream_param->is_pipeline_supported = FALSE;
  } else {
    stream_param->is_pipeline_supported = TRUE;
  }

  /* Add stream param to list of streams */
  l_stream_params = mct_list_append(session_param->l_stream_params,
    stream_param, NULL, NULL);
  if (!l_stream_params) {
    ISP_ERR("failed: l_stream_params %p", l_stream_params);
    //PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
    return FALSE;
  }
  session_param->l_stream_params = l_stream_params;

  //PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));

  return ret;
}

/** isp_handler_get_hw_params:
 *
 *  @port: port handle
 *  @mct_stream_info: mct stream info handle
 *  @hw_stream: hw stream to be returned
 *
 *  Return hw stream used by the stream
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_handler_get_hw_params(mct_port_t *port,
  mct_stream_info_t *mct_stream_info, isp_hw_streamid_t *hw_stream,
  isp_hw_id_t *hw_id)
{
  boolean              ret = TRUE;
  mct_module_t        *module = NULL;
  isp_t               *isp = NULL;
  isp_session_param_t *session_param = NULL;
  isp_stream_param_t  *stream_param = NULL;

  if (!port || !mct_stream_info || !hw_stream) {
    ISP_ERR("failed: %p %p %p", port, mct_stream_info, hw_stream);
    return FALSE;
  }

  /* Extract module handle */
  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return FALSE;
  }

  /* Extract module private */
  isp = MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }

  ret = isp_util_get_session_stream_params(module, mct_stream_info->identity,
    &session_param, &stream_param);
  if ((ret == FALSE) || !session_param || !stream_param) {
    ISP_ERR("failed: isp_util_get_session_stream_params %p %p", session_param,
      stream_param);
    return FALSE;
  }

  if (stream_param->num_hw_stream == 1) {
    *hw_stream = stream_param->hw_stream[0];
    *hw_id = session_param->hw_id[0];
  } else {
    if (mct_stream_info->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
      *hw_stream = ISP_HW_STREAM_OFFLINE;
      *hw_id = ISP_HW_MAX;
    } else if (mct_stream_info->stream_type == CAM_STREAM_TYPE_PARM) {
      *hw_stream = ISP_HW_STREAM_MAX;
      *hw_id = ISP_HW_MAX;
    } else {
      ISP_ERR("failed: num_hw_stream %d", stream_param->num_hw_stream);
    }
  }

  return TRUE;
}

/** isp_handler_handle_hal_param:
 *
 *  @module: mct module handle
 *  @port: port on which this event arrived
 *  @event: SOF event to be handled
 *
 *  Handle HAL parameters witch are needed by ISP module
 *
 *  Return TRUE if HAL control param is OK and FALSE on failure
 **/
boolean isp_handler_handle_hal_param(mct_module_t *module, mct_port_t *port,
  mct_event_t *event)
{
  boolean                      ret = TRUE;
  mct_event_control_parm_t    *param;
  isp_session_param_t         *session_param;

  if (!module || !port || !event) {
    ISP_ERR("failed: %p %p %p", module, port, event);
    return FALSE;
  }

  param = (mct_event_control_parm_t *)event->u.ctrl_event.control_event_data;
  if (!param) {
    ISP_ERR("failed: HAL control_parm is NULL");
    return FALSE;
  }

  ret = isp_util_get_session_params(module, ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_stream_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  switch (param->type) {
  case CAM_INTF_META_STREAM_INFO:
    ret = isp_util_handle_stream_info(module, session_param,
      (cam_stream_size_info_t *)param->parm_data);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_handle_stream_info");
      break;
    }
    break;

  case CAM_INTF_PARM_HAL_VERSION:
    ret = isp_util_set_hal_version(session_param,
      (cam_hal_version_t *)param->parm_data);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_set_hal_version");
    }
    break;

#ifdef OFFLINE_HW_UPDATE_OVERWRITE
  case CAM_INTF_PARM_HW_DATA_OVERWRITE:
    ret = isp_util_set_meta_hw_data_overwrite(session_param,
      (void *)param->parm_data, event->identity);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_set_meta_hw_update_list");
    }
    break;
#endif

  case CAM_INTF_PARM_VFE1_RESERVED_RDI:
  {
    isp_t * isp = NULL;
    isp = (isp_t *)MCT_OBJECT_PRIVATE(module);
    isp->isp_resource.isp_hw_limt.vfe1_reserved_rdi =
      *((int *)param->parm_data);
    ret = TRUE;
  }
    break;

  default:
    break;
  }

  return ret;
}

boolean isp_handler_handle_super_param(mct_module_t *module, mct_port_t *port,
  mct_event_t *event)
{
  mct_event_super_control_parm_t *param;
  isp_session_param_t            *session_param;
  uint32_t                        i = 0;
  boolean                         ret = TRUE;

  if (!module || !port || !event) {
    ISP_ERR("failed: %p %p %p", module, port, event);
    return FALSE;
  }

  param =
    (mct_event_super_control_parm_t *)event->u.ctrl_event.control_event_data;
  if (!param) {
    ISP_ERR("failed: HAL control_parm is NULL");
    return FALSE;
  }

  ret = isp_util_get_session_params(module, ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_stream_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  for (i = 0; i < param->num_of_parm_events; i++) {
    switch (param->parm_events[i].type) {

      case CAM_INTF_PARM_SENSOR_HDR:
        ret = isp_util_handle_vhdr(module, session_param,
         (cam_sensor_hdr_type_t *)param->parm_events[i].parm_data);
         if (ret == FALSE) {
           ISP_ERR("failed: isp_util_handle_vhdr");
         }
         break;

      case CAM_INTF_META_BINNING_CORRECTION_MODE:
        session_param->binncorr_mode=
           *(cam_binning_correction_mode_t *)param->parm_events[i].parm_data;
        ISP_DBG("%s: binn_corr_mode %d", __func__,
           session_param->binncorr_mode);
        break;

      default:
        break;
    }
  }

  return TRUE;
}

boolean isp_handler_link_intra_session(mct_module_t *module, mct_port_t *port,
  mct_event_t *event)
{
  mct_event_super_control_parm_t *param;
  isp_session_param_t            *session_param;
  boolean                         ret = TRUE;

  if (!module || !port || !event) {
    ISP_ERR("failed: %p %p %p", module, port, event);
    return FALSE;
  }

  ret = isp_util_get_session_params(module, ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_stream_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  /* set a flag to indicate dual camera use case */
  session_param->is_dualCam = TRUE;
  CDBG("Enable flag to indicate DualCam use case");

  return TRUE;
}

/** isp_handler_stats_data_ack:
 *
 *  @module: module handle
 *  @event: stats_data_ack event
 *
 *  Return buffers to buf manager
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_handler_stats_data_ack(mct_module_t *module, mct_event_t *event)
{
  boolean                ret = TRUE;
  isp_session_param_t   *session_param = NULL;
  mct_event_stats_isp_t *stats_event_data = NULL;

  if (!module || !event) {
    ISP_ERR("failed: module %p event %p", module, event);
    return FALSE;
  }

  stats_event_data =
    (mct_event_stats_isp_t *)event->u.module_event.module_event_data;
  if (!stats_event_data) {
    ISP_ERR("failed: stats_event_data %p", stats_event_data);
    return FALSE;
  }

  ret = isp_util_get_session_params(module,
    ISP_GET_SESSION_ID(event->identity), &session_param);
  if ((ret == FALSE) || !session_param) {
    ISP_ERR("failed: isp_util_get_session_params %p", session_param);
    return FALSE;
  }
  ISP_DBG("stats_divert_dbg stats ack from 3a mask %x",
    stats_event_data->stats_mask);

  if (session_param->isp_props.csidtg_enable == FALSE)
  {
    ret = isp_stats_buf_mgr_put_buf(
      &session_param->parser_params.buf_mgr[stats_event_data->isp_streaming_type],
      stats_event_data);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_stats_buf_mgr_put_buf");
    }
  }

  return ret;
}

boolean isp_handler_prepare_snapshot(mct_module_t *module, mct_port_t *port,
  mct_event_t *event)
{
  boolean              ret = TRUE;
  isp_session_param_t *session_param = NULL;

  if (!module || !port || !event) {
    ISP_ERR("failed: %p %p %p", module, port, event);
    return FALSE;
  }

  ret = isp_util_get_session_params(module, ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_stream_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  /* Set is_flash_mode to TRUE */
  session_param->parser_params.algo_saved_parm.tintless_saved_algo_parm.is_flash_mode = TRUE;


  return TRUE;
}

boolean isp_handler_hw_wake(mct_module_t *module, mct_port_t *port,
  mct_event_t *event)
{
  boolean              ret = TRUE;
  isp_session_param_t *session_param = NULL;

  if (!module || !port || !event) {
    ISP_ERR("failed: %p %p %p", module, port, event);
    return FALSE;
  }
  ret = isp_util_get_session_params(module, ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_stream_params ret %d %p", ret,
      session_param);
    return FALSE;
  }
  PTHREAD_MUTEX_LOCK(&session_param->metadata_mutex);
  memset(&session_param->metadata, 0, sizeof(session_param->metadata));
  PTHREAD_MUTEX_UNLOCK(&session_param->metadata_mutex);

  return TRUE;
}

boolean isp_handler_stop_zsl_snapshot(mct_module_t *module, mct_port_t *port,
  mct_event_t *event)
{
  boolean              ret = TRUE;
  isp_session_param_t *session_param = NULL;

  if (!module || !port || !event) {
    ISP_ERR("failed: %p %p %p", module, port, event);
    return FALSE;
  }

  ret = isp_util_get_session_params(module, ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_stream_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  /* Set is_flash_mode to TRUE */
  session_param->parser_params.algo_saved_parm.tintless_saved_algo_parm.is_flash_mode = FALSE;


  return TRUE;
}
