/* isp_parser.c
 *
 * Copyright (c) 2012-2014, 2018 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <poll.h>

/* kernel headers */
#include "media/msmb_isp.h"

/* mctl headers */
#include "mct_event_stats.h"
#include "media_controller.h"
#include "mct_list.h"

/* isp headers */
#include "isp_module.h"
#include "isp_parser_thread.h"
#include "isp_trigger_thread.h"
#include "isp_log.h"
#include "isp_util.h"
#include "isp_algo.h"
#include "isp_stats_buf_mgr.h"
#include "isp_pipeline_reg.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_COMMON, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_COMMON, fmt, ##args)

static boolean isp_dump_stats_data_to_file(isp_session_param_t *session_param,
                                           mct_event_stats_isp_t *stats_data);

/** isp_parser_thread_enqueue_event:
 *
 *  @trigger_update_params: trigger update params handle
 *  @data: data to be returned
 *
 *  Push new event to in stats queue
 *  Mutex Lock protection is outside this function.
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_parser_thread_enqueue_event(
  isp_parser_params_t *parser_params, void *data)
{
  boolean ret = TRUE;

  if (!parser_params || !parser_params->in_stats_queue) {
    ISP_ERR("failed: %p", parser_params);
    return FALSE;
  }

  mct_queue_push_tail(parser_params->in_stats_queue, (void *)data);
  return ret;
}


/** isp_parser_thread_dequeue_event:
 *
 *  @trigger_update_params: trigger update params handle
 *  @data: data to be returned
 *
 *  If queue is not empty, pop message from queue
 *  Mutex Lock protection is inside this function.
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_parser_thread_dequeue_event(
  isp_parser_params_t *parser_params, void **data)
{
  boolean ret = TRUE;

  if (!parser_params || !parser_params->in_stats_queue) {
    ISP_ERR("failed: %p", parser_params);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&parser_params->mutex);
  if (MCT_QUEUE_IS_EMPTY(parser_params->in_stats_queue) == FALSE) {
    *data = mct_queue_pop_head(parser_params->in_stats_queue);
    ret = TRUE;
  } else {
    ret = FALSE;
  }
  PTHREAD_MUTEX_UNLOCK(&parser_params->mutex);
  return ret;
} /* isp_trigger_thread_get_event_from_queue */

/** isp_parser_thread_send_divert_ack:
 *
 *  @module: mct module handle
 *  @stats_notify_event: stats notify event
 *
 *  Send upstream event for notify ack
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_parser_thread_send_divert_ack(mct_module_t *module,
  mct_event_t *stats_notify_event)
{
  boolean ret = TRUE;

  if (!stats_notify_event) {
    return FALSE;
  }

  stats_notify_event->direction = MCT_EVENT_UPSTREAM;
  stats_notify_event->u.module_event.type =
    MCT_EVENT_MODULE_RAW_STATS_DIVERT_ACK;
  ret = isp_util_forward_event_from_module(module, stats_notify_event);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_forward_event_from_module");
  }
  return ret;
}

/* Put the bundled stats buffer back to buf manager and free mct event */
static boolean isp_parser_thread_release_resource(mct_event_t *stats_notify_event,
  isp_bundled_stats_buf_mgr_t *buf_mgr)
{
  boolean ret = TRUE;

  if (!stats_notify_event) {
    ISP_ERR("failed: %p", stats_notify_event);
    return FALSE;
  }

  if (stats_notify_event->u.module_event.module_event_data) {
    ret = isp_bundled_stats_buf_mgr_put_buf(buf_mgr,
      stats_notify_event->u.module_event.module_event_data);
    stats_notify_event->u.module_event.module_event_data = NULL;
  }
  if (stats_notify_event) {
    free(stats_notify_event);
    stats_notify_event = NULL;
  }
  return ret;
}

static void isp_parser_thread_send_bhist_stats_to_hal(
  isp_session_param_t *session_param,
  mct_module_t *module,
  iface_raw_stats_buf_info_t *raw_stats_info,
  mct_event_stats_isp_t      *stats_data,
  isp_saved_stats_params_t    *stats_params)
{
  int i;
  mct_bus_msg_isp_bhist_stats_t bhist_msg;
  cam_hist_stats_t *hist = &bhist_msg.bhist_info;
  mct_bus_msg_t bus_msg;
  cam_histogram_data_t       *hist_data = NULL;
  uint32_t                   *stats_ptr = NULL;
  q3a_bhist_stats_t          *bhist_stats;
  isp_stats_data_t           *isp_stats = NULL;
  uint8_t                    *frame_index = NULL;

   /* init isp stats pointer when CSIDTG is used */
  if (session_param->isp_props.csidtg_enable == TRUE)
  {
    isp_stats = session_param->csidtg_data.isp_stats;
    frame_index = &isp_stats->frame_index;
  }

  if (isp_stats != NULL && frame_index != NULL)
  {
    mct_event_stats_isp_t *saved_stats =
      &isp_stats->saved_stats[*frame_index];
    bhist_stats = saved_stats->stats_data[MSM_ISP_STATS_BHIST].stats_buf;
  }
  else
  {
    bhist_stats =
      stats_data->stats_data[MSM_ISP_STATS_BHIST].stats_buf;
  }

  if (bhist_stats) {
    memset(&bhist_msg, 0, sizeof(mct_bus_msg_isp_bhist_stats_t));
    bhist_msg.bhist_meta_enable = stats_params->bhist_meta_enable;

    memset(&bus_msg, 0, sizeof(bus_msg));
    bus_msg.type = MCT_BUS_MSG_HIST_STATS_INFO;
    bus_msg.msg = (void *)&bhist_msg;
    bus_msg.size = sizeof(mct_bus_msg_isp_bhist_stats_t);
    bus_msg.sessionid = session_param->session_id;

    hist->type = CAM_HISTOGRAM_TYPE_BAYER;

    if ((TRUE == bhist_stats->bayer_r_hist_valid) &&
      (FALSE == bhist_stats->bayer_gr_hist_valid) &&
      (FALSE == bhist_stats->bayer_gb_hist_valid) &&
      (FALSE == bhist_stats->bayer_b_hist_valid))
    {
       hist_data = &hist->bayer_stats.r_stats;
       stats_ptr = bhist_stats->bayer_r_hist;
       hist->bayer_stats.data_type = CAM_STATS_CHANNEL_Y;

      if (CAM_HISTOGRAM_STATS_SIZE == bhist_stats->num_bins) {
        memcpy(hist_data->hist_buf, stats_ptr,
        sizeof(hist->bayer_stats.r_stats.hist_buf));
      } else if (bhist_stats->num_bins > CAM_HISTOGRAM_STATS_SIZE &&
                bhist_stats->num_bins % CAM_HISTOGRAM_STATS_SIZE == 0) {
        /* Stats from HW have more bins and is a multiple of output bin size */
        /* scale bins by collapsing */
        uint32_t factor = bhist_stats->num_bins / CAM_HISTOGRAM_STATS_SIZE;
        uint32_t i, j;
        for (i = 0; i < CAM_HISTOGRAM_STATS_SIZE; i++) {
          for (j = 0; j < factor; j++) {
           hist_data->hist_buf[i] +=
             stats_ptr[i * factor + j];
           }
        }
      } else {
        ISP_ERR("CHANNEL_Y Size mismatch error bhist_stats->num_bins %d",
        bhist_stats->num_bins);
        return;
       }
    }

    if ((TRUE == bhist_stats->bayer_r_hist_valid) &&
      (TRUE == bhist_stats->bayer_gr_hist_valid) &&
      (TRUE == bhist_stats->bayer_gb_hist_valid) &&
      (TRUE == bhist_stats->bayer_b_hist_valid))
    {
      hist->bayer_stats.data_type = CAM_STATS_CHANNEL_ALL;
      if (CAM_HISTOGRAM_STATS_SIZE == bhist_stats->num_bins) {
        memcpy(hist->bayer_stats.r_stats.hist_buf, &bhist_stats->bayer_r_hist,
          sizeof(hist->bayer_stats.r_stats.hist_buf));
        memcpy(hist->bayer_stats.b_stats.hist_buf, &bhist_stats->bayer_b_hist,
          sizeof(hist->bayer_stats.b_stats.hist_buf));
        memcpy(hist->bayer_stats.gr_stats.hist_buf, &bhist_stats->bayer_gr_hist,
          sizeof(hist->bayer_stats.gr_stats.hist_buf));
        memcpy(hist->bayer_stats.gb_stats.hist_buf, &bhist_stats->bayer_gb_hist,
          sizeof(hist->bayer_stats.gb_stats.hist_buf));
      } else if (bhist_stats->num_bins > CAM_HISTOGRAM_STATS_SIZE &&
                bhist_stats->num_bins % CAM_HISTOGRAM_STATS_SIZE == 0) {
        /* Stats from HW have more bins and is a multiple of output bin size */
        /* scale bins by collapsing */
        uint32_t factor = bhist_stats->num_bins / CAM_HISTOGRAM_STATS_SIZE;
        uint32_t i, j;
        for (i = 0; i < CAM_HISTOGRAM_STATS_SIZE; i++) {
          for (j = 0; j < factor; j++) {
            hist->bayer_stats.r_stats.hist_buf[i]  +=
              bhist_stats->bayer_r_hist[i * factor + j];
            hist->bayer_stats.b_stats.hist_buf[i]  +=
              bhist_stats->bayer_b_hist[i * factor + j];
            hist->bayer_stats.gr_stats.hist_buf[i] +=
              bhist_stats->bayer_gr_hist[i * factor + j];
            hist->bayer_stats.gb_stats.hist_buf[i] +=
              bhist_stats->bayer_gb_hist[i * factor + j];
          }
        }
      } else {
        ISP_ERR("CHANNEL_ALL Size mismatch error bhist_stats->num_bins %d",
        bhist_stats->num_bins);
        return;
      }
    }

    if ((FALSE == bhist_stats->bayer_r_hist_valid) &&
      (TRUE == bhist_stats->bayer_gr_hist_valid) &&
      (FALSE == bhist_stats->bayer_gb_hist_valid) &&
      (FALSE == bhist_stats->bayer_b_hist_valid))
    {
      hist_data = &hist->bayer_stats.gr_stats;
      stats_ptr = bhist_stats->bayer_gr_hist;
      hist->bayer_stats.data_type = CAM_STATS_CHANNEL_GR;

      if (CAM_HISTOGRAM_STATS_SIZE == bhist_stats->num_bins) {
        memcpy(hist_data->hist_buf, stats_ptr,
        sizeof(hist->bayer_stats.gr_stats.hist_buf));
      } else if (bhist_stats->num_bins > CAM_HISTOGRAM_STATS_SIZE &&
                bhist_stats->num_bins % CAM_HISTOGRAM_STATS_SIZE == 0) {
        /* Stats from HW have more bins and is a multiple of output bin size */
        /* scale bins by collapsing */
        uint32_t factor = bhist_stats->num_bins / CAM_HISTOGRAM_STATS_SIZE;
        uint32_t i, j;
        for (i = 0; i < CAM_HISTOGRAM_STATS_SIZE; i++) {
          for (j = 0; j < factor; j++) {
           hist_data->hist_buf[i] +=
             stats_ptr[i * factor + j];
          }
        }
      } else {
        ISP_ERR("CHANNEL_GR Size mismatch error bhist_stats->num_bins %d",
          bhist_stats->num_bins);
        return;
      }
    }

    if (TRUE != isp_util_send_metadata_entry(module, &bus_msg,
      raw_stats_info->frame_id)) {
      ISP_ERR("session_id = %d error", session_param->session_id);
    }
  }
}

/** isp_parser_thread_process:
 *
 *  @module: mct module handle
 *  @isp_resource: isp resource handle
 *  @session_param: session param
 *
 *  Handle stats_isp_t update
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_parser_thread_process(mct_module_t *module,
  isp_resource_t *isp_resource, isp_session_param_t *session_param)
{
  boolean                     ret = TRUE;
  isp_hw_id_t                 hw_id0 = 0, hw_id1 = 0;
  mct_event_t                *stats_notify_event;
  isp_parser_params_t        *parser_params = NULL;
  mct_event_t                 stats_data_event;
  mct_event_t                 algo_output_event;
  iface_raw_stats_buf_info_t *raw_stats_info = NULL;
  isp_bundled_stats_buf_info_params_t  *bundled_stats_info = NULL;
  uint32_t                    i = 0, stats_type, stats_parse_flag = 1;
  mct_event_stats_isp_t      *stats_data = NULL;
  isp_saved_stats_params_t   *stats_params[ISP_HW_MAX];
  isp_algo_params_t          *algo_parm[ISP_HW_MAX];
  isp_hw_id_t                 hw_index = 0;
  boolean                     is_ack_done = FALSE;
  iface_resource_request_t    *resource_request = NULL;
  isp_stats_info_t           *stats_info = NULL;
  uint32_t                    ext_stats_mask = 0;
  isp_stats_data_t           *isp_stats = NULL;
  uint8_t                    *frame_index = NULL;
  bf_fw_roi_cfg_t            *roi_cfg;
  mct_event_stats_isp_data_t *stats_data_bf = NULL;
  q3a_bf_stats_t             *bf_stats = NULL;
  isp_bundled_stats_buf_mgr_t *bundled_stats_buf_mgr;

  if (!module || !isp_resource || !session_param) {
    ISP_ERR("failed: %p %p %p", module, isp_resource, session_param);
    return FALSE;
  }

  parser_params = &session_param->parser_params;
  bundled_stats_buf_mgr = &parser_params->bundled_stats_buf_mgr;

  memset(&stats_data_event, 0, sizeof(stats_data_event));
  memset(&algo_output_event, 0, sizeof(algo_output_event));

  PTHREAD_MUTEX_LOCK(&parser_params->mutex);

  stats_notify_event = parser_params->stats_notify_event;
  parser_params->stats_notify_event = NULL;

  PTHREAD_MUTEX_UNLOCK(&parser_params->mutex);

  if (stats_notify_event->u.module_event.type !=
    MCT_EVENT_MODULE_RAW_STATS_DIVERT) {
    ISP_ERR("failed: invalid event type %d expected %d",
      stats_notify_event->u.module_event.type,
      MCT_EVENT_MODULE_RAW_STATS_DIVERT);
    goto ERROR;
  }

  bundled_stats_info = (isp_bundled_stats_buf_info_params_t *)
    stats_notify_event->u.module_event.module_event_data;
  if (!bundled_stats_info) {
    ISP_ERR("failed: raw_stats_info %p", bundled_stats_info);
    goto ERROR;
  }

  raw_stats_info = &bundled_stats_info->raw_stats_info;
  if (!raw_stats_info) {
    ISP_ERR("failed: raw_stats_info %p", raw_stats_info);
    goto ERROR;
  }

  ISP_DBG("raw_stats_info->stats_mask %x frame_id %d",
    raw_stats_info->stats_mask,
    raw_stats_info->frame_id);

  for (hw_index = 0; hw_index < ISP_HW_MAX; hw_index++) {
    stats_params[hw_index] = &bundled_stats_info->stats_params[hw_index];
    algo_parm[hw_index]    = &bundled_stats_info->algo_parm[hw_index];
  }

  if (!raw_stats_info->stats_mask) {
    ISP_ERR("failed: invalid stats_mask %x", raw_stats_info->stats_mask);
    goto ERROR;
  }

  if (session_param->state == ISP_STATE_IDLE) {
    goto ERROR;
  }

  ext_stats_mask = raw_stats_info->stats_mask;
  /* Pass enable stats mask to ISP pipeline to map to hw mask */
  if (isp_resource->isp_resource_info[0].
       isp_pipeline->func_table->pipeline_map_stats_mask_to_hw) {
    ret = isp_resource->isp_resource_info[0].isp_pipeline->
      func_table->pipeline_map_stats_mask_to_hw(&ext_stats_mask);
    if (ret == FALSE) {
      ISP_ERR("failed: pipeline_map_stats_mask_to_hw");
    }
  }

  /* Get buffers from buffer maanager */
  if (session_param->offline_num_isp > 0 &&
    raw_stats_info->hw_id == session_param->offline_hw_id[0]) {
    stats_data = isp_stats_buf_mgr_get_buf(
       &parser_params->buf_mgr[ISP_STREAMING_OFFLINE],
      ext_stats_mask);
    if (!stats_data) {
      ISP_ERR("failed: get buf failed");
      goto ERROR;
    }
    resource_request = &parser_params->resource_request[ISP_STREAMING_OFFLINE];
    stats_data->isp_streaming_type = ISP_STREAMING_OFFLINE;
  } else {
    stats_data = isp_stats_buf_mgr_get_buf(
       &parser_params->buf_mgr[ISP_STREAMING_ONLINE],
      ext_stats_mask);
    if (!stats_data) {
      ISP_ERR("failed: get buf failed");
      goto ERROR;
    }
    resource_request = &parser_params->resource_request[ISP_STREAMING_ONLINE];
    stats_data->isp_streaming_type = ISP_STREAMING_ONLINE;
  }

  for (i = 0; i < resource_request->num_stats_stream; i++) {
    stats_info = &resource_request->isp_stats_info[i];
    if (raw_stats_info->stats_mask & (1 << stats_info->stats_type)) {
      stats_type = stats_info->stats_type;

      if (ISP_SWAP_AEC_BG_HDR_BE == TRUE) {
        if (stats_info->stats_type == MSM_ISP_STATS_AEC_BG)
          stats_type = MSM_ISP_STATS_HDR_BE;
        else if (stats_info->stats_type == MSM_ISP_STATS_HDR_BE)
          stats_type = MSM_ISP_STATS_AEC_BG;
      }

      if (stats_type >= MSM_ISP_STATS_MAX ||
        !stats_data->stats_data[stats_type].stats_buf) {
        ISP_ERR("failed: stats_buf NULL or invalid stats_type %d", stats_type);
        goto ERROR;
      }
      stats_parse_flag = stats_info->parse_flag;
      if (session_param->num_isp == 1) {
        hw_id0 = session_param->hw_id[0];
        if (stats_parse_flag == 1) {
          ret = isp_resource_pipeline_parse(isp_resource, hw_id0,
            stats_info->stats_type,
            raw_stats_info->raw_stats_buf_len[stats_info->stats_type],
            raw_stats_info->raw_stats_buffer[stats_info->stats_type],
            stats_data, stats_params[hw_id0], NULL,
            &parser_params->parser_session_params);
        }
      } else {
        hw_id0 = session_param->hw_id[0];
        hw_id1 = session_param->hw_id[1];
        if (stats_parse_flag == 1) {
          ret = isp_resource_pipeline_parse(isp_resource, hw_id0,
            stats_info->stats_type,
            raw_stats_info->raw_stats_buf_len[stats_info->stats_type],
            raw_stats_info->raw_stats_buffer[stats_info->stats_type],
            stats_data, stats_params[hw_id0],stats_params[hw_id1],
            &parser_params->parser_session_params);
        }
      }
      if (ret == FALSE) {
        ISP_ERR("failed: isp_resource_pipepine_parse stats type %d", stats_type);
        goto ERROR;
      }
    }
  }

  if( raw_stats_info->stats_mask & (1 << MSM_ISP_STATS_BF)) {
    hw_id0 = session_param->hw_id[0];
    roi_cfg = &stats_params[hw_id0]->stats_config.af_config.bf_fw.bf_fw_roi_cfg;
    if (roi_cfg->max_primary_abs_height > 10) {
      raw_stats_info->pd_data.pd_stats_read_data_length = roi_cfg->max_primary_abs_height - 10;
    } else {
      raw_stats_info->pd_data.pd_stats_read_data_length = 0;
    }
    stats_data_bf = &stats_data->stats_data[MSM_ISP_STATS_BF];
    if (stats_data_bf) {
      bf_stats = (q3a_bf_stats_t *)(stats_data_bf->stats_buf);
      memcpy(&bf_stats->u.fw_stats.pd_data, &raw_stats_info->pd_data,
                                        sizeof(mct_bf_pd_stats_data_t));
    }
    ISP_DBG("frame_id : %d buf_index %d padf vaddr : %p length : %d\n",
    raw_stats_info->frame_id, raw_stats_info->pd_data.buf_idx,
    raw_stats_info->pd_data.pd_buffer_data,
    raw_stats_info->pd_data.pd_stats_read_data_length);
  }

  /* init isp stats pointer when CSIDTG is used */
  if (session_param->isp_props.csidtg_enable == TRUE)
  {
    isp_stats = session_param->csidtg_data.isp_stats;
    frame_index = &isp_stats->frame_index;
  }

  /* Post BHist stats to mct bus */
  if ((raw_stats_info->stats_mask & (1 << MSM_ISP_STATS_BHIST)) &&
      stats_params[hw_id0]->bhist_meta_enable) {
    isp_parser_thread_send_bhist_stats_to_hal(session_param, module,
      raw_stats_info, stats_data, stats_params[hw_id0]);
  }

  /* Call stats notify event to 3A */
  stats_data->frame_id = raw_stats_info->frame_id;
  stats_data->timestamp = raw_stats_info->timestamp;
  stats_data->sof_timestamp = raw_stats_info->sof_timestamp;

  /* dump ISP stats into a file for CSIDTG */
  if (session_param->isp_props.dump_stats == TRUE &&
      session_param->isp_props.csidtg_enable == FALSE)
  {
    isp_dump_stats_data_to_file(session_param, stats_data);
  }

  /*send event to 3A*/
  memset(&stats_data_event, 0, sizeof(stats_data_event));

  stats_data_event.direction = MCT_EVENT_DOWNSTREAM;
  stats_data_event.type = MCT_EVENT_MODULE_EVENT;
  stats_data_event.identity = stats_notify_event->identity;
  stats_data_event.u.module_event.type = MCT_EVENT_MODULE_STATS_DATA;

  if (session_param->isp_props.csidtg_enable == TRUE && isp_stats != NULL)
  {
    isp_stats->saved_stats[*frame_index].frame_id = stats_data->frame_id;
    isp_stats->saved_stats[*frame_index].timestamp = stats_data->timestamp;
    isp_stats->saved_stats[*frame_index].sof_timestamp =
                                         stats_data->sof_timestamp;
    stats_data_event.u.module_event.module_event_data =
      (void *)&isp_stats->saved_stats[*frame_index];
    isp_stats->saved_stats[*frame_index].ack_flag = TRUE;
  }
  else
  {
    stats_data_event.u.module_event.module_event_data = (void *)stats_data;
  }

  /* Set ack_flag to TRUE, if 3A need to consume this buffer, it will set
   * to FALSE
   */
  stats_data->ack_flag = TRUE;

  ISP_DBG("stats_divert_dbg forward stats to 3a mask %x", stats_data->stats_mask);
  if (session_param->isp_props.csidtg_enable == TRUE)
  {
    if (stats_data->ack_flag == TRUE) {
      /* Return buffers to buffer maanager */
      ret = isp_stats_buf_mgr_put_buf(
         &parser_params->buf_mgr[stats_data->isp_streaming_type], stats_data);
      if (ret == FALSE) {
        ISP_ERR("failed: pet buf failed");
      }
    }
    stats_data->ack_flag = FALSE;
  }
  ret =  isp_util_forward_event_downstream_to_type(module, &stats_data_event,
    MCT_PORT_CAPS_STATS);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_forward_event_downstream_to_type");
  }

  /* Call stats notify ack to iface to enqueue stats buffer back to kernel*/
  ret = isp_parser_thread_send_divert_ack(module, stats_notify_event);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_parser_thread_send_divert_ack");
  }

  if (session_param->isp_props.csidtg_enable == TRUE && isp_stats != NULL)
  {
    ret = isp_algo_execute_internal_algo(module,
      session_param, &isp_stats->saved_stats[*frame_index],
      algo_parm[session_param->hw_id[0]]);
    if (ret == FALSE) {
      ISP_DBG("failed: isp_parser_thread_execute_internal_algo");
    }
    (*frame_index)++;
    (*frame_index) %= MAX_ISP_STATS;
  }
  else
  {
    /* Run ISP internel algo, ex: LA, LTM, tintless, etc*/
    ret = isp_algo_execute_internal_algo(module,
      session_param, stats_data, algo_parm[session_param->hw_id[0]]);
    if (ret == FALSE) {
      ISP_DBG("failed: isp_parser_thread_execute_internal_algo");
    }
  }


  ret = isp_parser_thread_release_resource(stats_notify_event,
    bundled_stats_buf_mgr);
  if (ret == FALSE)
    ISP_ERR("failed: put back bundled stats info buffer failure");

  if (stats_data->ack_flag == TRUE) {
    /* Return buffers to buffer maanager */
    ret = isp_stats_buf_mgr_put_buf(
       &parser_params->buf_mgr[stats_data->isp_streaming_type], stats_data);
    if (ret == FALSE) {
      ISP_ERR("failed: pet buf failed");
    }
  }

  return ret;

ERROR:
  if (stats_data) {
    /* Return buffers to buffer maanager */
    ret = isp_stats_buf_mgr_put_buf(
       &parser_params->buf_mgr[stats_data->isp_streaming_type], stats_data);
    if (ret == FALSE) {
      ISP_ERR("failed: pet buf failed");
    }
  }

  if (is_ack_done == FALSE) {
    /* Call stats notify ack to iface to enqueue stats buffer back to kernel*/
    ret = isp_parser_thread_send_divert_ack(module, stats_notify_event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_parser_thread_send_divert_ack");
    }
    ret = isp_parser_thread_release_resource(stats_notify_event,
      bundled_stats_buf_mgr);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_parser_thread_release_resource");
    }
  }
  return FALSE;
} /* isp_parser_thread_process */

/** isp_parser_thread_free_params:
 *
 *  @module: module handle
 *  @parser_params: parser param
 *
 *  Send ack for stats notify event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_parser_thread_free_params(mct_module_t *module,
  isp_parser_params_t *parser_params)
{
  isp_hw_id_t hw_index = 0;
  isp_bundled_stats_buf_mgr_t *buf_mgr = NULL;

  if (!module || !parser_params) {
    ISP_ERR("failed: module %p parser_params %p", module, parser_params);
    return FALSE;
  }
  buf_mgr = &parser_params->bundled_stats_buf_mgr;

  PTHREAD_MUTEX_LOCK(&parser_params->mutex);
  isp_parser_thread_send_divert_ack(module, parser_params->stats_notify_event);
  isp_parser_thread_release_resource(parser_params->stats_notify_event,
    buf_mgr);
  parser_params->stats_notify_event = NULL;

  /* Clear stats params */
  for (hw_index = 0; hw_index < ISP_HW_MAX; hw_index++) {
    memset(&parser_params->stats_params[hw_index], 0,
      sizeof(isp_saved_stats_params_t));
  }
  PTHREAD_MUTEX_UNLOCK(&parser_params->mutex);

  return TRUE;
}

/** isp_parser_thread_func:
 *
 *  @data: handle to session_param
 *
 *  ISP main thread handler
 *
 *  Returns NULL
 **/
static void *isp_parser_thread_func(void *data)
{
  boolean                      ret = TRUE;
  isp_session_param_t         *session_param;
  struct pollfd                pollfds;
  int32_t                      num_fds = 1, ready = 0, i = 0, read_bytes = 0;
  isp_parser_thread_event_t    event;
  isp_parser_params_t         *parser_params = NULL;
  boolean                      exit_thread = FALSE;
  isp_parser_thread_priv_t    *thread_priv = NULL;
  mct_module_t                *module = NULL;
  isp_resource_t              *isp_resource = NULL;
  void                        *pending_stats_event = NULL;

  if (!data) {
    ISP_ERR("failed: data %p", data);
    return NULL;
  }

  ISP_HIGH("isp_new_thread parser thread start");
  thread_priv = (isp_parser_thread_priv_t *)data;
  module = thread_priv->module;
  isp_resource = thread_priv->isp_resource;
  session_param = thread_priv->session_param;
  if (!module || !isp_resource || !session_param) {
    ISP_ERR("failed: %p %p %p", module, isp_resource, session_param);
    return NULL;
  }

  parser_params = &session_param->parser_params;
  PTHREAD_MUTEX_LOCK(&parser_params->mutex);
  parser_params->is_thread_alive = TRUE;
  pthread_cond_signal(&parser_params->cond);
  PTHREAD_MUTEX_UNLOCK(&parser_params->mutex);

  while (exit_thread == FALSE) {
    pollfds.fd = parser_params->pipe_fd[READ_FD];
    pollfds.events = POLLIN|POLLPRI;
    ready = poll(&pollfds, (nfds_t)num_fds, -1);
    if (ready > 0) {
      if (pollfds.revents & (POLLIN|POLLPRI)) {
        read_bytes = read(pollfds.fd, &event,
          sizeof(isp_parser_thread_event_t));
        if ((read_bytes < 0) ||
            (read_bytes != sizeof(isp_parser_thread_event_t))) {
          ISP_ERR("failed: read_bytes %d", read_bytes);
          continue;
        }
        switch (event.type) {
        case ISP_PARSER_EVENT_PROCESS:
          while (1) {
            pending_stats_event = NULL;
            ret = isp_parser_thread_dequeue_event(parser_params,
              &pending_stats_event);
            if (ret == FALSE) {
              ISP_DBG("Queue is empty");
              break;
            }
            if (pending_stats_event != NULL) {
              PTHREAD_MUTEX_LOCK(&parser_params->mutex);
              parser_params->stats_notify_event =
                (mct_event_t *)pending_stats_event;
              PTHREAD_MUTEX_UNLOCK(&parser_params->mutex);
              ret = isp_parser_thread_process(module,
                isp_resource, session_param);
              if (ret == FALSE) {
                ISP_ERR("failed: isp_parser_process");
                break;
              }
            }
          }
          break;
        case ISP_PARSER_THREAD_EVENT_FREE_QUEUE:
          ISP_HIGH("Free stats parsing input queue");
          while (1) {
            pending_stats_event = NULL;
            ret = isp_parser_thread_dequeue_event(parser_params,
              &pending_stats_event);
            if (ret == FALSE) {
              ISP_DBG("Queue is empty");
              break;
            }
            if (pending_stats_event != NULL) {
              PTHREAD_MUTEX_LOCK(&parser_params->mutex);
              parser_params->stats_notify_event =
                (mct_event_t *)pending_stats_event;
              PTHREAD_MUTEX_UNLOCK(&parser_params->mutex);
              ret = isp_parser_thread_free_params(module,
                &session_param->parser_params);
              if (ret == FALSE) {
                ISP_ERR("failed: isp_parser_thread_free_params");
                break;
              }
            }
          }
          /* Unblock caller as parser thread queue is cleared */
          PTHREAD_MUTEX_LOCK(&parser_params->mutex);
          pthread_cond_signal(&parser_params->cond);
          PTHREAD_MUTEX_UNLOCK(&parser_params->mutex);
          ISP_HIGH("Free stats parsing input queue done");
          break;
        case ISP_PARSER_EVENT_ABORT_THREAD:
          exit_thread = TRUE;
          break;
        default:
          ISP_ERR("invalid event type %d", event.type);
          break;
        }
      }
    } else if (ready <= 0) {
      if (errno != EINTR) {
        ISP_ERR("failed: exit thread");
        break;
      }
    }
  }

  return NULL;
}

/** isp_parser_thread_post_message:
 *
 *  @parser_params: parser params
 *  @type: message type to be posted
 *
 *  Post event to session thread
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_parser_thread_post_message(
  isp_parser_params_t *parser_params, isp_parser_event_type_t type)
{
  int32_t                      rc = 0;
  isp_parser_thread_event_t    message;

  if (!parser_params || (type >= ISP_PARSER_EVENT_MAX)) {
    ISP_ERR("failed: %p type %d", parser_params, type);
    return FALSE;
  }

  memset(&message, 0, sizeof(message));
  message.type = type;
  rc = write(parser_params->pipe_fd[WRITE_FD], &message, sizeof(message));
  if(rc < 0) {
    ISP_ERR("failed: rc %d", rc);
    return FALSE;
  }

  return TRUE;
} /* isp_parser_thread_post_message */

/** isp_parser_thread_save_stats_nofity_event:
 *
 *  @module: mct module handle
 *  @session_param: session param handle
 *  @event: event to be stored
 *
 *  If there is pending event to be handled, return it and
 *  store current in parser params
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_parser_thread_save_stats_nofity_event(mct_module_t *module,
  isp_session_param_t *session_param, mct_event_t *event)
{
  boolean                     ret = TRUE;
  isp_parser_params_t        *parser_params = NULL;
  mct_event_t                *stats_notify_event;
  iface_raw_stats_buf_info_t *raw_stats_info = NULL;
  mct_event_t                *copy_event = NULL;
  isp_bundled_stats_buf_info_params_t  *bundled_stats_info = NULL;
  uint32_t                    hw_id = 0;


  RETURN_IF_NULL(module);
  RETURN_IF_NULL(session_param);
  RETURN_IF_NULL(event);

  parser_params = &session_param->parser_params;
  PTHREAD_MUTEX_LOCK(&parser_params->mutex);
  raw_stats_info = event->u.module_event.module_event_data;
  GOTO_ERROR_IF_NULL(raw_stats_info);

  bundled_stats_info = isp_bundled_stats_buf_mgr_get_buf(&parser_params->bundled_stats_buf_mgr);
  GOTO_ERROR_IF_NULL(bundled_stats_info);

  bundled_stats_info->raw_stats_info = *raw_stats_info;

  copy_event = (mct_event_t *)CAM_MALLOC(sizeof(*copy_event));
  GOTO_ERROR_IF_NULL(copy_event);
  memset(copy_event, 0, sizeof(*copy_event));


  for (hw_id = 0; hw_id < ISP_HW_MAX; hw_id++) {
    bundled_stats_info->stats_params[hw_id] = parser_params->stats_params[hw_id];
    bundled_stats_info->algo_parm[hw_id] = parser_params->algo_parm[hw_id];
  }

  /* Deep copy of incoming stats event */
  *copy_event = *event;
  copy_event->u.module_event.module_event_data =
    bundled_stats_info;


  ret = isp_parser_thread_enqueue_event(parser_params, (void *)copy_event);
  GOTO_ERROR_IF_FALSE(ret);

  ret = isp_parser_thread_post_message(parser_params, ISP_PARSER_EVENT_PROCESS);
  GOTO_ERROR_IF_FALSE(ret);

  PTHREAD_MUTEX_UNLOCK(&parser_params->mutex);
  return ret;

error:
  if (bundled_stats_info)
    isp_bundled_stats_buf_mgr_put_buf(&parser_params->bundled_stats_buf_mgr,
      bundled_stats_info);
  if (copy_event)
    free(copy_event);
  PTHREAD_MUTEX_UNLOCK(&parser_params->mutex);
  return FALSE;
} /* isp_parser_thread_save_stats_nofity_event */

/** isp_initialize_property:
 *
 *  @session_param: isp session param handle
 *
 *  Initialize ISP properties
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean isp_initialize_property(isp_session_param_t *session_param)
{
  char prop[PROPERTY_VALUE_MAX];

  property_get("persist.vendor.camera.isp.stats_dump", prop, "0");
  session_param->isp_props.dump_stats = atoi(prop);

  property_get("persist.vendor.camera.csidtg.enable", prop, "0");
  session_param->isp_props.csidtg_enable = atoi(prop);

  return TRUE;
}

/** isp_open_stats_file:
 *
 *  @session_param: isp session param handle
 *
 *  Open ISP stats file
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean isp_open_stats_file(isp_session_param_t *session_param)
{
  boolean ret = TRUE;
  const char stats_header[20] = "QCOM ISP STATS";
  char header[20];

  if(session_param->csidtg_data.stats_fd != NULL)
  {
    ISP_ERR("Stats File pointer already set");
    return ret;
  }

  if (session_param->isp_props.dump_stats == TRUE &&
      session_param->isp_props.csidtg_enable == FALSE)
  {
    session_param->csidtg_data.stats_fd = fopen(ISP_STATS_FILE, "wb");
    if(session_param->csidtg_data.stats_fd == NULL)
    {
      ret = FALSE;
      ISP_ERR("Failed to open Stats File");
    }
    else
    {
      session_param->csidtg_data.frame_wr_cnt = 0;
      fwrite(stats_header, sizeof(char),
               sizeof(stats_header),session_param->csidtg_data.stats_fd);
    }
  }
  else if (session_param->isp_props.csidtg_enable == TRUE &&
           session_param->isp_props.dump_stats == FALSE)
  {
    session_param->csidtg_data.stats_fd = fopen(ISP_STATS_FILE, "rb");
    if(session_param->csidtg_data.stats_fd == NULL)
    {
      ret = FALSE;
      ISP_ERR("Failed to open Stats File");
    }
    else
    {
      fread(header, sizeof(char),
          sizeof(header), session_param->csidtg_data.stats_fd);
      if(strcmp(header,stats_header) != 0)
      {
        ISP_ERR("Failed: Not a valid ISP Stats File");
        ret = FALSE;
      }
    }
  }

  return ret;
}

/** isp_load_csidtg_stats_data:
 *
 *  @session_param: isp session param handle
 *
 *  Load ISP stats data from file
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean isp_load_csidtg_stats_data(isp_session_param_t *session_param)
{
  uint8_t index;
  uint8_t cnt;
  boolean ret = TRUE;
  mct_event_stats_isp_t *isp_saved_stats = NULL;
  isp_stats_data_t *isp_stats_csidtg = NULL;
  FILE *stats_fd = NULL;

  stats_fd = session_param->csidtg_data.stats_fd;
  if(stats_fd == NULL)
  {
    ISP_ERR("failed: NULL File pointer for stats");
    ret = FALSE;
    goto NULL_ERROR;
  }

  if(session_param->csidtg_data.isp_stats != NULL)
  {
    isp_saved_stats = session_param->csidtg_data.isp_stats->saved_stats;
    // NULL check
    if(isp_saved_stats == NULL)
    {
      ISP_ERR("failed: NULL pointer for saved stats");
      ret = FALSE;
      goto NULL_ERROR;
    }


    // free up buffer if it is already allocated
    for(index = 0; index < MAX_ISP_STATS; index++)
    {
      for(cnt = 0; cnt < MSM_ISP_STATS_MAX; cnt++)
      {
        if(isp_saved_stats[index].stats_data[cnt].stats_buf != NULL)
        {
          free(isp_saved_stats[index].stats_data[cnt].stats_buf);
          isp_saved_stats[index].stats_data[cnt].stats_buf = NULL;
        }
      }
    }
    free(session_param->csidtg_data.isp_stats);
    session_param->csidtg_data.isp_stats = NULL;
  }

  if (session_param->csidtg_data.isp_stats == NULL)
  {
    isp_stats_csidtg = (isp_stats_data_t *)CAM_MALLOC(sizeof(isp_stats_data_t));
    if(isp_stats_csidtg == NULL)
    {
      ISP_ERR("failed: To allocate memory for isp stats for csid testgen");
      ret = FALSE;
      goto NULL_ERROR;
    }
    else
    {
      session_param->csidtg_data.isp_stats = isp_stats_csidtg;
      isp_saved_stats = session_param->csidtg_data.isp_stats->saved_stats;
      // NULL check
      if(isp_saved_stats == NULL)
      {
        ISP_ERR("failed: NULL pointer for saved stats");
        ret = FALSE;
        goto NULL_ERROR_SAVED_STATS;
      }
    }
  }

  memset(session_param->csidtg_data.isp_stats, 0, sizeof(isp_stats_data_t));

  // Load the stats data from a file to data structure
  for(index = 0; index < MAX_ISP_STATS; index++)
  {
    fread(&isp_saved_stats[index].timestamp, sizeof(char),
          sizeof(struct timeval), stats_fd);
    fread(&isp_saved_stats[index].frame_id, sizeof(char),
          sizeof(uint32_t), stats_fd);
    fread(&isp_saved_stats[index].stats_mask, sizeof(char),
          sizeof(uint32_t), stats_fd);

    for(cnt = 0; cnt < MSM_ISP_STATS_MAX; cnt++)
    {
      fread(&isp_saved_stats[index].stats_data[cnt].stats_type, sizeof(char),
            sizeof(enum msm_isp_stats_type),stats_fd);
      fread(&isp_saved_stats[index].stats_data[cnt].buf_size, sizeof(char),
            sizeof(uint32_t),stats_fd);
      if(isp_saved_stats[index].stats_data[cnt].buf_size != 0)
      {
        isp_saved_stats[index].stats_data[cnt].stats_buf =
          CAM_MALLOC(isp_saved_stats[index].stats_data[cnt].buf_size);
        if(isp_saved_stats[index].stats_data[cnt].stats_buf == NULL)
        {
          ISP_ERR("failed: To allocate memory for stats buf");
          ret = FALSE;
          goto STATS_ERROR;
        }
        fread(isp_saved_stats[index].stats_data[cnt].stats_buf, sizeof(char),
              isp_saved_stats[index].stats_data[cnt].buf_size,stats_fd);
      }
      fread(&isp_saved_stats[index].stats_data[cnt].used_size, sizeof(char),
             sizeof(uint32_t),stats_fd);
      fread(&isp_saved_stats[index].stats_data[cnt].buf_idx, sizeof(char),
            sizeof(uint32_t),stats_fd);
    }

    fread(&isp_saved_stats[index].isp_streaming_type, sizeof(char),
          sizeof(isp_streaming_type_t),stats_fd);
    fread(&isp_saved_stats[index].ack_flag, sizeof(char), sizeof(boolean),
          stats_fd);
  }
  fclose(stats_fd);
  session_param->csidtg_data.stats_fd = NULL;
  stats_fd = NULL;
  return ret;

STATS_ERROR:
  isp_saved_stats = session_param->csidtg_data.isp_stats->saved_stats;

  // free up buffer if it is already allocated
  for(index = 0; index < MAX_ISP_STATS; index++)
  {
    for(cnt = 0; cnt < MSM_ISP_STATS_MAX; cnt++)
    {
      if(isp_saved_stats[index].stats_data[cnt].stats_buf != NULL)
      {
        free(isp_saved_stats[index].stats_data[cnt].stats_buf);
        isp_saved_stats[index].stats_data[cnt].stats_buf = NULL;
      }
    }
  }
NULL_ERROR_SAVED_STATS:
  free(session_param->csidtg_data.isp_stats);
  session_param->csidtg_data.isp_stats = NULL;
NULL_ERROR:
  return ret;
}

/** isp_dump_stats_data_to_file:
 *
 *  @session_param: isp session param handle
 *  @stats_data: isp stats data
 *
 *  Dump and save ISP Stats to a file
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean isp_dump_stats_data_to_file(isp_session_param_t *session_param,
                                           mct_event_stats_isp_t *stats_data)
{
  FILE *stats_fd = NULL;
  uint8_t *frame_wr_cnt = NULL;

  frame_wr_cnt = &session_param->csidtg_data.frame_wr_cnt;

  // copy isp stats data into a file. required only for csid test-gen
  if (stats_data->frame_id >= ISP_START_FRAME_ID &&
      *frame_wr_cnt < MAX_ISP_STATS)
  {
    uint8_t cnt;

    stats_fd = session_param->csidtg_data.stats_fd;
    if(stats_fd == NULL)
    {
      ISP_ERR("failed: NULL File pointer for stats");
      return FALSE;
    }

    ISP_DBG("Stats WRITE frame_id=%d",stats_data->frame_id);
    fwrite(&stats_data->timestamp, sizeof(char), sizeof(struct timeval),
           stats_fd);
    fwrite(&stats_data->frame_id, sizeof(char), sizeof(uint32_t),stats_fd);
    fwrite(&stats_data->stats_mask, sizeof(char), sizeof(uint32_t),stats_fd);

    for(cnt = 0; cnt < MSM_ISP_STATS_MAX; cnt++)
    {
      fwrite(&stats_data->stats_data[cnt].stats_type, sizeof(char),
             sizeof(enum msm_isp_stats_type),stats_fd);
      fwrite(&stats_data->stats_data[cnt].buf_size, sizeof(char),
             sizeof(uint32_t),stats_fd);
      if(stats_data->stats_data[cnt].buf_size != 0)
      {
        fwrite(stats_data->stats_data[cnt].stats_buf, sizeof(char),
               stats_data->stats_data[cnt].buf_size,stats_fd);
      }

      fwrite(&stats_data->stats_data[cnt].used_size, sizeof(char),
             sizeof(uint32_t),stats_fd);
      fwrite(&stats_data->stats_data[cnt].buf_idx, sizeof(char),
             sizeof(uint32_t),stats_fd);
    }

    fwrite(&stats_data->isp_streaming_type, sizeof(char),
           sizeof(isp_streaming_type_t),stats_fd);
    fwrite(&stats_data->ack_flag, sizeof(char), sizeof(boolean),stats_fd);
    (*frame_wr_cnt)++;
  }

  if(*frame_wr_cnt == MAX_ISP_STATS &&
      session_param->csidtg_data.stats_fd != NULL)
  {
    fclose(session_param->csidtg_data.stats_fd);
    stats_fd = NULL;
    session_param->csidtg_data.stats_fd = NULL;
  }

  return TRUE;
}

/** isp_parser_thread_create:
 *
 *  @session_param: isp session param handle
 *
 *  Create new ISP thread
 *
 *  Returns TRUE on success and FALSE on failure
 **/
boolean isp_parser_thread_create(mct_module_t *module,
  isp_resource_t *isp_resource, isp_session_param_t *session_param)
{
  int32_t                      rc = 0;
  boolean                      ret = TRUE;
  isp_parser_params_t         *parser_params = NULL;
  isp_parser_thread_priv_t     thread_priv;

  if (!module || !isp_resource || !session_param) {
    ISP_ERR("failed: %p %p %p", module, isp_resource, session_param);
    return FALSE;
  }

  parser_params = &session_param->parser_params;
  /* Create PIPE to communicate with isp thread */
  rc = pipe(parser_params->pipe_fd);
  if(rc < 0) {
    ISP_ERR("pipe() failed");
    return FALSE;
  }

  /* Create input queue for raw stats*/
  parser_params->in_stats_queue =
    (mct_queue_t *)CAM_MALLOC(sizeof(*parser_params->in_stats_queue));
  if (!parser_params->in_stats_queue) {
    ISP_ERR("failed: no free memory for stats queue");
    ret = FALSE;
    goto ERROR_MALLOC;
  }
  memset(parser_params->in_stats_queue, 0,
    sizeof(*parser_params->in_stats_queue));
  mct_queue_init(parser_params->in_stats_queue);

  thread_priv.module = module;
  thread_priv.isp_resource = isp_resource;
  thread_priv.session_param = session_param;

  PTHREAD_MUTEX_LOCK(&parser_params->mutex);
  parser_params->is_thread_alive = FALSE;
  rc = pthread_create(&parser_params->parser_thread, NULL,
    isp_parser_thread_func, &thread_priv);
  pthread_setname_np(parser_params->parser_thread, "CAM_isp_parser");
  if(rc < 0) {
    ISP_ERR("pthread_create() failed rc= %d", rc);
    ret = FALSE;
    goto ERROR_THREAD;
  }

  while(parser_params->is_thread_alive == FALSE) {
    pthread_cond_wait(&parser_params->cond, &parser_params->mutex);
  }
  PTHREAD_MUTEX_UNLOCK(&parser_params->mutex);

  isp_initialize_property(session_param);

  if ( session_param->isp_props.csidtg_enable == TRUE ||
       session_param->isp_props.dump_stats == TRUE)
  {
    ret = isp_open_stats_file(session_param);
    if (ret == TRUE && session_param->isp_props.csidtg_enable == TRUE)
    {
      isp_load_csidtg_stats_data(session_param);
    }
    // Setting ret to TRUE to unblock when ISP stats file is not present
    ret = TRUE;
  }
  return ret;
ERROR_THREAD:
  mct_queue_free(parser_params->in_stats_queue);
  parser_params->in_stats_queue = NULL;
ERROR_MALLOC:
  close(parser_params->pipe_fd[READ_FD]);
  close(parser_params->pipe_fd[WRITE_FD]);
  PTHREAD_MUTEX_UNLOCK(&parser_params->mutex);
  return ret;
}

/** isp_parser_thread_join:
 *
 *  @session_param: ISP session param
 *
 *  Join ISP thread
 *
 *  Returns: void
 **/
void isp_parser_thread_join(isp_session_param_t *session_param)
{
  boolean                 ret = TRUE;
  isp_parser_params_t *parser_params = NULL;

  if (!session_param) {
    ISP_ERR("failed: session_param %p", session_param);
    return;
  }

  parser_params = &session_param->parser_params;

  PTHREAD_MUTEX_LOCK(&parser_params->mutex);
  parser_params->is_thread_alive = FALSE;
  PTHREAD_MUTEX_UNLOCK(&parser_params->mutex);

  isp_parser_thread_post_message(parser_params,
    ISP_PARSER_EVENT_ABORT_THREAD);

  /* Join session thread */
  pthread_join(parser_params->parser_thread, NULL);

  PTHREAD_MUTEX_LOCK(&parser_params->mutex);
  if (parser_params->in_stats_queue) {
    mct_queue_free(parser_params->in_stats_queue);
    parser_params->in_stats_queue = NULL;
  }
  ISP_HIGH("isp stats input queue deleted");
  PTHREAD_MUTEX_UNLOCK(&parser_params->mutex);

  close(parser_params->pipe_fd[READ_FD]);
  close(parser_params->pipe_fd[WRITE_FD]);
}
