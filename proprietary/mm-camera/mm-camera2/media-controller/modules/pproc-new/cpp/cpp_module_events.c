/*============================================================================

  Copyright (c) 2013-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include "eztune_diagnostics.h"
#include "cpp_module_events.h"
#include "mct_profiler.h"

#define MIN_NATIVE_BUF   4

/** cpp_module_get_frame_valid:
 *
 *  @module: mct module handle
 *  @identity: event identity
 *  @frame_id: frame id for which validity needs to be checked
 *  @stream_type: stream type for which validity needs to be
 *              checked
 *  @reset : remove the StreamID from the queue.
 *
 *  This function passes upstream event to ISP to find out
 *  whether this frame needs to be processed for this stream
 *
 *  Return: is_frame_valid on success and 0 on failure
 **/
uint32_t cpp_module_get_frame_valid(mct_module_t* module,
  uint32_t identity, uint32_t frame_id, uint32_t stream_id,
  cam_stream_type_t stream_type, boolean reset)
{
  int32_t                      rc = 0;
  uint32_t                     i =0;
  uint32_t                     q_idx = frame_id % FRAME_CTRL_SIZE;
  mct_event_t                  new_event;
  mct_event_frame_request_t    frame_request;
  cpp_module_stream_params_t  *stream_params;
  cpp_module_session_params_t *session_params;

  cpp_module_ctrl_t* ctrl = (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_PER_FRAME_ERR("invalid control, failed\n");
    return rc;
  }

  cpp_module_get_params_for_identity(ctrl, identity, &session_params,
    &stream_params);
  if(!session_params) {
    CPP_PER_FRAME_ERR("invalid session params, failed\n");
    return rc;
  }

  /* Return valid for offline stream */
  if (stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    rc = 1;
  } else {
    for (i = 0; i < session_params->valid_stream_ids[q_idx].num_streams; i++) {
      if (session_params->valid_stream_ids[q_idx].stream_request[i].streamID ==
        stream_id) {
        if (reset) {
          session_params->valid_stream_ids[q_idx].stream_request[i].streamID = 0;
          session_params->verify_proc_stream_ids[q_idx].stream_request[i].streamID = 0;
        }
        rc = 1;
        break;
      }
    }
  }
  return rc;
}

/** cpp_module_create_cpp_event:
 *
 * Description:
 *  Create cpp event and fill ack key, and processed divert
 *  information.
 *
 **/
cpp_module_event_t *cpp_module_create_cpp_event(cpp_module_ack_key_t ack_key,
  cpp_hardware_params_t *hw_params, isp_buf_divert_t *isp_buf,
  uint32_t identity, uint32_t div_identity,uint32_t duplicate_identity)
{
  cpp_hardware_params_t *h_params;
  cpp_module_event_t *cpp_event = (cpp_module_event_t*)
    CAM_MALLOC(sizeof(cpp_module_event_t));
  if(!cpp_event) {
    CPP_ERR("CAM_MALLOC() failed\n");
    return NULL;
  }
  memset(cpp_event, 0x00, sizeof(cpp_module_event_t));
  cpp_event->ack_key = ack_key;
  /* by default all events are created valid */
  cpp_event->invalid = FALSE;
  if (hw_params) {
    /* this is hw processing event */
    cpp_event->hw_process_flag = TRUE;
    cpp_event->type = CPP_MODULE_EVENT_PROCESS_BUF;
    cpp_event->u.process_buf_data.proc_identity = identity;

    /* copy isp buf and other data from the mct event */
    // copying input buffer_access is taken care here.
    memcpy(&(cpp_event->u.process_buf_data.isp_buf_divert),
      (isp_buf_divert_t *)(isp_buf), sizeof(isp_buf_divert_t));
    /* copy the stream hw params in event */
    memcpy(&(cpp_event->u.process_buf_data.hw_params), hw_params,
      sizeof(cpp_hardware_params_t));
    h_params = &(cpp_event->u.process_buf_data.hw_params);
    /* Reset interpolate mask for the next frame */
    hw_params->interpolate_mask = 0;
    /* processed divert related info */
    cpp_event->u.process_buf_data.proc_div_identity =
      div_identity;
    /* update the divert identity in buf info */
    if (duplicate_identity != 0) {
      cpp_event->u.process_buf_data.hw_params.duplicate_output = TRUE;
      cpp_event->u.process_buf_data.hw_params.duplicate_identity =
        duplicate_identity;
      cpp_event->u.process_buf_data.hw_params.dup_buffer_info.identity =
        duplicate_identity;
    }
    cpp_event->u.process_buf_data.proc_div_required = FALSE;
    if (div_identity != PPROC_INVALID_IDENTITY) {
      cpp_event->u.process_buf_data.proc_div_required = TRUE;
    }
  CPP_BUF_DBG("stream %d, index %x, dup_index %x frame_id %d",
    h_params->stream_type,
    h_params->output_buffer_info.index,
    h_params->dup_buffer_info.index,
    h_params->frame_id);
  } else {
    /* this is unprocessed divert event */
    if (div_identity == PPROC_INVALID_IDENTITY) {
      CPP_ERR("failed invalid unprocess div identity\n");
      free(cpp_event);
      return NULL;
    }
    cpp_event->hw_process_flag = FALSE;
    cpp_event->type = CPP_MODULE_EVENT_DIVERT_BUF;

    // Forwarding buf_divert, buffer_access flags taken care automatically
    /* copy isp buf and other data from the mct event */
    memcpy(&(cpp_event->u.divert_buf_data.isp_buf_divert),
      (isp_buf_divert_t*)(isp_buf), sizeof(isp_buf_divert_t));
    cpp_event->u.divert_buf_data.div_identity = div_identity;
    cpp_event->u.divert_buf_data.isp_buf_divert.identity = identity;
    cpp_event->u.divert_buf_data.isp_buf_divert.pass_through = 1;
  }
  CPP_PER_FRAME_LOW("proc_identity 0x%x proc_div_identity 0x%x proc_div_required %d\n",
    cpp_event->u.process_buf_data.proc_identity,
    cpp_event->u.process_buf_data.proc_div_identity,
    cpp_event->u.process_buf_data.proc_div_required);
  return cpp_event;
}

boolean cpp_module_check_frame_skip(cpp_module_stream_params_t *stream_params,
  cpp_module_session_params_t *session_params, uint32_t frame_id)
{
  /* decide if skip is required for HFR */
  if (stream_params->hfr_skip_info.skip_required &&
    session_params->hal_version != CAM_HAL_V3) {
    if ((cpp_decide_hfr_skip(frame_id -
      stream_params->hfr_skip_info.frame_offset,
      stream_params->hfr_skip_info.skip_count)) == TRUE) {
      /* Skip this frame */
      CPP_BUF_DBG("skipping frame_id=%d for identity=0x%x",
        frame_id, stream_params->identity);
      CPP_BUF_DBG("skip_count=%d, offset=%d",
        stream_params->hfr_skip_info.skip_count,
        stream_params->hfr_skip_info.frame_offset);
      return TRUE;
    }
  }
  return FALSE;
}


/* cpp_module_dump_hw_proc_list:
 *
 * this function dumps the hardware process list
 **/
static void cpp_module_dump_hw_proc_list(
  cpp_module_process_list_t *hw_process_list)
{
  uint32_t i;
  cpp_module_hw_proc_info_t *hw_proc_info = NULL;

  if (hw_process_list == NULL) {
    CPP_ERR("process list = NULL, return");
    return;
  }

  CPP_BUF_LOW("====dump CPP reprocessing list====");
  CPP_BUF_LOW("num_hw_process = %d", hw_process_list->num_hw_process);
  for (i = 0; i < hw_process_list->num_hw_process; i++) {
    hw_proc_info = &hw_process_list->hw_proc_info[i];
    if (hw_proc_info->process_stream != NULL &&
      hw_proc_info->process_stream->identity != 0) {
      CPP_BUF_LOW(" hw_proc_list[%d]: stream id %x, dup identity %x", i,
        hw_proc_info->process_stream->identity,
        hw_proc_info->dup_identity);
    }
  }

  return;
}

/* cpp_module_decide_hw_proc_info:
 *
 * this function decide hw process info and duplication
 **/
static boolean cpp_module_decide_hw_proc_info(mct_module_t* module,
  cpp_module_session_params_t *session_params,
  cpp_module_stream_params_t **stream_list, uint32_t num_streams,
  cpp_module_process_list_t *hw_process_list, bool perf_bottleneck)
{
  uint32_t i = 0;
  uint32_t j = 0;
  cpp_module_ctrl_t *ctrl = NULL;
  cpp_module_stream_params_t *stream_params1 = NULL;
  cpp_module_stream_params_t *stream_params2 = NULL;
  cpp_module_hw_proc_info_t *hw_proc_info = NULL;
  boolean is_duplicate = FALSE;

  if (!module || !stream_list) {
    CPP_BUF_ERR("null ptr %p %p", module, stream_list);
    return FALSE;
  }

  if (num_streams > CPP_MODULE_MAX_STREAMS) {
    CPP_BUF_ERR("num_stream %d > max supported %d",
      num_streams, CPP_MODULE_MAX_STREAMS);
    return FALSE;
  }

  ctrl = (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_BUF_LOW("invalid control, failed\n");
    return -EFAULT;
  }

  for (i = 0; i < num_streams; i++) {
    /* get stream parameters based on the event identity */
    stream_params1 = stream_list[i];
    if(!stream_params1 || stream_params1->identity == 0) {
      CPP_BUF_LOW("can not find valid stream\n");
      continue;
    }

    hw_proc_info = &hw_process_list->
      hw_proc_info[hw_process_list->num_hw_process];

     /*if identity is not 0, put in new hw process*/
    hw_proc_info->process_stream = stream_params1;
    stream_list[i] = NULL;

    /*full search to find duplicate
      given duplication number doesnt look to increase,
      assume only 1 duplicate*/
    for (j = i + 1; j < num_streams; j++) {
      stream_params2 = stream_list[j];
      if(!stream_params2 || stream_params2->identity == 0) {
          CPP_BUF_LOW("can not find valid stream\n");
          continue;
        }

      is_duplicate = cpp_module_util_check_duplicate(
        stream_params1, stream_params2, ctrl->cpphw->hwinfo.version);
      if (is_duplicate == TRUE && hw_proc_info->dup_identity == 0) {
        hw_proc_info->dup_identity = stream_params2->identity;
        stream_list[j] = NULL;
        break;
      }
    }
    cpp_module_decide_runtime_clock_update(ctrl, session_params,
      perf_bottleneck, is_duplicate, FALSE,
      (stream_params1->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC), 0);
    hw_process_list->num_hw_process++;

  }
  cpp_module_dump_hw_proc_list(hw_process_list);

  return TRUE;
}

/* cpp_module_decide_process_list:
 *
 **/
static int32_t cpp_module_decide_process_list(mct_module_t* module,
  cpp_module_session_params_t *session_params,
  cpp_module_stream_params_t *stream_params, uint32_t frame_id,
  cpp_module_process_list_t *hw_process_list)
{
  cpp_module_ctrl_t           *ctrl = NULL;
  boolean                     is_process_main_stream = FALSE;
  uint32_t                    stream_frame_valid = 0;
  uint32_t                    linked_stream_frame_valid = 0;
  uint32_t                    i = 0;
  uint32_t                    stream_list_size = 0;
  cpp_module_stream_params_t *stream_parm_list[CPP_MODULE_MAX_STREAMS];
  cpp_module_stream_params_t *linked_stream_params = NULL;
  bool perf_bottleneck = false;

  if (stream_params == NULL || hw_process_list == NULL) {
    CPP_PER_FRAME_ERR(" null pointer %p %p", stream_params, hw_process_list);
    return -1;
  }

  ctrl = module ? (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module) : NULL;
  if(!ctrl) {
    CPP_BUF_LOW("invalid control, failed\n");
    return -EFAULT;
  }
  memset(&stream_parm_list[0], 0 ,
    sizeof(cpp_module_stream_params_t *) * CPP_MODULE_MAX_STREAMS);

  /* decide processing requirements based on the stream params */
  if (TRUE == cpp_module_util_decide_proc_frame_per_stream(module,
     session_params, stream_params, frame_id) ) {
     CPP_PER_FRAME_DBG("TRUE for stream %d , frame %d",
       stream_params->stream_type, frame_id);
     stream_parm_list[stream_list_size] = stream_params;
     stream_list_size++;
     cpp_module_util_inc_clock_counter_l(session_params,
       stream_params);
     if (cpp_module_determine_perf_bottlneck(ctrl, session_params, stream_params)) {
       perf_bottleneck = true;
       CPP_CLOCK_LOW("perf_bottleneck %d",perf_bottleneck,
         stream_params->stream_type);
     }
  } else {
     CPP_PER_FRAME_DBG("FALSE for stream %d , frame %d",
       stream_params->stream_type, frame_id);
  }

 /* linked stream case */
  if (stream_params->num_linked_streams > 0) {
    for (i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
      linked_stream_params = stream_params->linked_streams[i];

      if (linked_stream_params == NULL ||
        linked_stream_params->identity == 0)
        continue;

      PTHREAD_MUTEX_LOCK(&(linked_stream_params->mutex));
      /* check linked stream on and frame valid */
      /* stream on, valid frame and no skip, need to process */
      if (TRUE == cpp_module_util_decide_proc_frame_per_stream(module,
        session_params, linked_stream_params, frame_id)) {
        CPP_PER_FRAME_DBG("TRUE for stream %d , frame %d",
        linked_stream_params->stream_type, frame_id);
        //Add the stream if native buffer is not enabled
        if (!(cpp_module_util_is_cpp_proc_need(module, stream_params, linked_stream_params))) {
          stream_parm_list[stream_list_size] = linked_stream_params;
          stream_list_size++;
          cpp_module_util_inc_clock_counter_l(session_params,
            linked_stream_params);
          if (cpp_module_determine_perf_bottlneck(ctrl, session_params, linked_stream_params)) {
            perf_bottleneck = true;
            CPP_CLOCK_LOW("perf_bottleneck %d", perf_bottleneck,
              linked_stream_params->stream_type);
          }
        } else {
          CPP_DBG("not adding the linked stream type %d",linked_stream_params->stream_type);
        }
      }
      PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));

      /*most we can support CPP_MODULE_MAX_STREAMS*/
      if (stream_list_size >= CPP_MODULE_MAX_STREAMS)
        break;

    } /*for loop for all linked streams*/
  }

  /* once return back is FALSE,
     in caller function mark drity and ack */
  if ((stream_list_size == 0) || (stream_list_size > CPP_MODULE_MAX_STREAMS)) {
    CPP_PER_FRAME_DBG("num stream to handle = %d error!!\n", stream_list_size);
    return -1;
  }

  /*from arbitary identity list to decide duplication and hw process*/
  cpp_module_decide_hw_proc_info(module, session_params, &stream_parm_list[0],
    stream_list_size, hw_process_list, perf_bottleneck);

  return 0;
}

/* cpp_module_prepare_hw_parm:
 *
 **/
static int32_t cpp_module_prepare_hw_parm(mct_module_t* module,
  cpp_module_session_params_t *session_params,
  cpp_module_stream_params_t *process_stream_params,
  isp_buf_divert_t *isp_buf)
{

  if (!module || !session_params || !process_stream_params || !isp_buf) {
    CPP_BUF_ERR("failed! null ptr %p %p %p %p",
      module, session_params, process_stream_params, isp_buf);
    return -1;
  }
  /* disable retry if hfr mode is enabled */
  process_stream_params->hw_params.retry_disable =
    (session_params->fps_range.video_max_fps > 30.0f) ? 1 : 0;

  process_stream_params->hw_params.we_disable = FALSE;
  if (process_stream_params->hw_params.tnr_enable) {
    if (process_stream_params->stream_info->streaming_mode ==
      CAM_STREAMING_MODE_BURST) {
      process_stream_params->hw_params.we_disable = TRUE;
      if (process_stream_params->req_frame_output &&
        (process_stream_params->current_burst_cnt <
        process_stream_params->stream_info->num_burst)) {
        process_stream_params->hw_params.we_disable = FALSE;
        process_stream_params->current_burst_cnt++;
      }
    }
    if (process_stream_params->hw_params.uv_upsample_enable !=
      isp_buf->is_uv_subsampled) {
      process_stream_params->hw_params.grey_ref_enable = TRUE;
    }
  }
  process_stream_params->hw_params.uv_upsample_enable =
    isp_buf->is_uv_subsampled;

  return 0;
}

/* cpp_module_update_clock_by_process_list
 *
 *  Set/reset the clock rate object duplication flag of the current stream
 *  and linked stream.
 **/
static int cpp_module_update_clock_by_process_list(mct_module_t *module,
  cpp_module_ctrl_t *ctrl,
  cpp_module_process_list_t hw_process_list,
  cpp_module_session_params_t *session_params,
  cpp_module_stream_params_t *stream_params)
{
  uint32_t i = 0;
  cpp_module_stream_clk_rate_t *clk_rate_obj = NULL;
  cpp_module_stream_clk_rate_t *linked_clk_rate_obj = NULL;
  cpp_module_stream_clk_rate_t *shared_clk_rate_obj = NULL;
  cpp_module_stream_params_t  *process_stream_params = NULL;
  cpp_module_stream_params_t  *linked_stream_params = NULL;
  cpp_module_hw_proc_info_t    *hw_proc_info = NULL;
  cpp_module_session_params_t *lsession_params = NULL;
  cpp_module_stream_params_t *lstream_params = NULL;;

  int rc = 0;

  for (i = 0; i < hw_process_list.num_hw_process; i++) {
    hw_proc_info = &hw_process_list.hw_proc_info[i];
    process_stream_params = hw_proc_info->process_stream;
    if (!process_stream_params) {
      CPP_ERR("invalid stream in process info");
      continue;
    }

    clk_rate_obj = cpp_module_find_clk_rate_by_identity(ctrl,
      process_stream_params->hw_params.identity);
    if (clk_rate_obj == NULL) {
      CPP_ERR(" clk rate obj = NULL, identity = %x",
        process_stream_params->hw_params.identity);
      return FALSE;
    }

    /*check if duplication*/
    if (hw_proc_info->dup_identity != 0) {
      linked_clk_rate_obj = cpp_module_find_clk_rate_by_identity(
        ctrl, hw_proc_info->dup_identity);
      if (linked_clk_rate_obj != NULL) {
        linked_clk_rate_obj->config_flags |= 0x1;
        clk_rate_obj->config_flags |= 0x1;
      }
      cpp_module_get_params_for_identity(ctrl,
        hw_proc_info->dup_identity, &lsession_params, &lstream_params);
      if (lstream_params) {
        cpp_module_update_process_delay_l(ctrl, session_params,
          lstream_params, linked_clk_rate_obj,
          cpp_module_determine_perf_bottlneck(ctrl, session_params, lstream_params));
      }
    }
    /*check if output is shared*/
    if (process_stream_params->share_output > 1) {
       clk_rate_obj->config_flags |= (0x1 << SHARED_FLAG_SHIFT);
         if (stream_params->num_linked_streams > 0) {
            clk_rate_obj->shared_output_num = process_stream_params->share_output;
           for (i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
             linked_stream_params = stream_params->linked_streams[i];

             if (linked_stream_params == NULL ||
               linked_stream_params->identity == 0)
               continue;

             shared_clk_rate_obj = cpp_module_find_clk_rate_by_identity(
               ctrl, linked_stream_params->identity);
             if (shared_clk_rate_obj != NULL) {
               shared_clk_rate_obj->config_flags |= (0x1 << SHARED_FLAG_SHIFT);
               shared_clk_rate_obj->shared_output_num =
                 process_stream_params->share_output;
             }
           }
        }
    }
    cpp_module_update_process_delay_l(ctrl, session_params, process_stream_params, clk_rate_obj,
      cpp_module_determine_perf_bottlneck(ctrl, session_params, process_stream_params));
  }

  rc = cpp_module_update_clock_freq(module, ctrl,
    session_params, stream_params, FALSE);
  if (rc < 0) {
    CPP_ERR(" update clock failed! rc = %d", rc);
    return rc;
  }

  return 0;
}

static void cpp_module_process_sticky_set_param_event(mct_module_t *module,
  cpp_module_session_params_t* session_params,
  uint32_t identity,
  uint32_t cur_frame_id)
{
  int32_t rc;
  mct_event_control_parm_t ctrl_parm;
  boolean                  param_processed = FALSE;
  cpp_module_ctrl_t *ctrl = NULL;

  if (!module || !session_params) {
    CPP_ERR("failed: module %p session_params %p", module, session_params);
    return;
  }

  ctrl = (cpp_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_PER_FRAME_ERR("invalid cpp control, failed");
    return;
  }

  if (session_params->sticky_set_parm.spl_effect.is_valid &&
    session_params->sticky_set_parm.spl_effect.apply_frame_id <= cur_frame_id) {
    ctrl_parm.type = CAM_INTF_PARM_EFFECT;
    ctrl_parm.parm_data = &session_params->sticky_set_parm.spl_effect.value;
    CPP_ASF_DBG("CAM_INTF_PARM_EFFECT apply_frame_id %d cur_frame_id %d iden"
      "0x%x value %d",
        session_params->sticky_set_parm.spl_effect.apply_frame_id,
          cur_frame_id, identity,
            session_params->sticky_set_parm.spl_effect.value);
    rc = cpp_module_process_set_param_event(ctrl, identity,
      ctrl_parm.type, ctrl_parm.parm_data, &param_processed,
        cur_frame_id);
    memset(&(session_params->sticky_set_parm.spl_effect), 0,
      sizeof(session_params->sticky_set_parm.spl_effect));
  }

  if (session_params->sticky_set_parm.edge_mode.is_valid &&
    session_params->sticky_set_parm.edge_mode.apply_frame_id <= cur_frame_id) {
    ctrl_parm.type = CAM_INTF_META_EDGE_MODE;
    ctrl_parm.parm_data = &session_params->sticky_set_parm.edge_mode.value;
    CPP_ASF_DBG("CAM_INTF_PARM_EFFECT apply_frame_id %d cur_frame_id %d iden"
      "0x%x edg_mode %d sharpness %d",
        session_params->sticky_set_parm.edge_mode.apply_frame_id,
          cur_frame_id, identity,
            session_params->sticky_set_parm.edge_mode.value.edge_mode,
              session_params->sticky_set_parm.edge_mode.value.sharpness);
    rc = cpp_module_process_set_param_event(ctrl, identity,
      ctrl_parm.type, ctrl_parm.parm_data, &param_processed,
        cur_frame_id);
    memset(&(session_params->sticky_set_parm.edge_mode), 0,
      sizeof(session_params->sticky_set_parm.edge_mode));
  }

  if (session_params->sticky_set_parm.reduction_mode.is_valid &&
    session_params->sticky_set_parm.reduction_mode.apply_frame_id <=
      cur_frame_id) {
    ctrl_parm.type = CAM_INTF_META_NOISE_REDUCTION_MODE;
    ctrl_parm.parm_data = &session_params->sticky_set_parm.reduction_mode.value;
    CPP_DENOISE_DBG("CAM_INTF_META_NOISE_REDUCTION_MODE apply_frame_id %d"
      "cur_frame_id %d iden 0x%x value %d",
        session_params->sticky_set_parm.reduction_mode.apply_frame_id,
          cur_frame_id, identity,
            session_params->sticky_set_parm.reduction_mode.value);
    rc = cpp_module_process_set_param_event(ctrl, identity,
      ctrl_parm.type, ctrl_parm.parm_data, &param_processed,
        cur_frame_id);
    memset(&(session_params->sticky_set_parm.reduction_mode), 0,
      sizeof(session_params->sticky_set_parm.reduction_mode));
  }

  if (session_params->sticky_set_parm.cds_mode.is_valid &&
    session_params->sticky_set_parm.cds_mode.apply_frame_id <= cur_frame_id) {
    ctrl_parm.type = CAM_INTF_PARM_CDS_MODE;
    ctrl_parm.parm_data = &session_params->sticky_set_parm.cds_mode.value;
    CPP_DENOISE_DBG("CAM_INTF_PARM_CDS_MODE apply_frame_id %d cur_frame_id"
      "%d iden 0x%x value %d",
        session_params->sticky_set_parm.cds_mode.apply_frame_id,
          cur_frame_id, identity,
            session_params->sticky_set_parm.cds_mode.value);
    rc = cpp_module_process_set_param_event(ctrl, identity,
      ctrl_parm.type, ctrl_parm.parm_data, &param_processed,
        cur_frame_id);
    memset(&(session_params->sticky_set_parm.cds_mode), 0,
      sizeof(session_params->sticky_set_parm.cds_mode));
  }

  if (session_params->sticky_set_parm.dyn_feast.is_valid &&
    session_params->sticky_set_parm.dyn_feast.apply_frame_id <= cur_frame_id) {
    ctrl_parm.type = CAM_INTF_META_IMG_DYN_FEAT;
    ctrl_parm.parm_data = &session_params->sticky_set_parm.dyn_feast.value;
    CPP_TNR_DBG("CAM_INTF_META_IMG_DYN_FEAT apply_frame_id %d cur_frame_id"
      "%d iden 0x%x mask %d count %d",
        session_params->sticky_set_parm.dyn_feast.apply_frame_id, cur_frame_id,
          identity,
            session_params->sticky_set_parm.dyn_feast.value.dyn_feature_mask,
              session_params->sticky_set_parm.dyn_feast.value.input_count);
    rc = cpp_module_process_set_param_event(ctrl, identity,
      ctrl_parm.type, ctrl_parm.parm_data, &param_processed,
        cur_frame_id);
    memset(&(session_params->sticky_set_parm.dyn_feast), 0,
      sizeof(session_params->sticky_set_parm.dyn_feast));
  }

}

int32_t cpp_module_validate_pcr_with_streamid(
  cpp_module_session_params_t** session_params, uint32_t q_idx,
  uint32_t identity) {
  uint32_t k;
  for (k = 0; k < session_params[0]-> \
    verify_proc_stream_ids[q_idx].num_streams; k++) {
    if (session_params[0]-> \
      verify_proc_stream_ids[q_idx].stream_request[k].streamID ==
      PPROC_GET_STREAM_ID(identity)) {
      return 0;
    }
  }
  return -EFAULT;
}

int32_t cpp_module_check_pcr_with_frameid(cpp_module_ctrl_t* ctrl,
  uint32_t frame_id, cpp_module_session_params_t** session_params,
  cpp_module_stream_params_t** stream_params, cam_stream_type_t stream_type,
  uint32_t identity)
{
  if(!ctrl) {
    CPP_DBG("failed, ctrl=%p", ctrl);
    return -EINVAL;
  }

  int32_t rc;
  uint32_t i,j;
  uint32_t q_idx = frame_id % FRAME_CTRL_SIZE;
  uint32_t session_id = CPP_GET_SESSION_ID(identity);

  for(i=0; i < CPP_MODULE_MAX_SESSIONS; i++) {
    if (ctrl->session_params[i]) {
      if (ctrl->session_params[i]->session_id == session_id) {
        *session_params = ctrl->session_params[i];
        for (j=0; j < CPP_MODULE_MAX_STREAMS; j++) {
          if (ctrl->session_params[i]->stream_params[j]) {
            *stream_params = ctrl->session_params[i]->stream_params[j];
            if (stream_params[0]->stream_type == stream_type) {
              rc = cpp_module_validate_pcr_with_streamid(session_params,
                     q_idx, identity);
              if (rc == 0)
                return 0;
            }
          }
        }
      }
    }
  }
  return -EFAULT;
}


/* cpp_module_send_buf_divert_event:
 *
 **/
static int32_t cpp_module_send_buf_divert_event(mct_module_t* module,
  uint32_t identity, isp_buf_divert_t *isp_buf)
{
  uint32_t                     identity_list[CPP_MODULE_MAX_STREAMS];
  uint32_t                     identity_list_size = 0;
  uint32_t                     i = 0, j = 0;
  uint32_t                     process_frame_control = 0;
  cpp_module_ctrl_t            *ctrl = NULL;
  uint32_t                     frame_id = 0;
  int32_t                      ret = 0;
  struct timeval               tv1, tv2;
  boolean                      duplicate_output = FALSE;
  uint32_t                     stream_frame_valid = 0;
  uint32_t                     linked_stream_frame_valid;
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_stream_params_t  *process_stream_params = NULL;
  cpp_module_stream_params_t  *temp_stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_process_list_t    hw_process_list;
  mct_module_type_t            mod_type = MCT_MODULE_FLAG_INVALID;
  uint32_t                     div_identity = PPROC_INVALID_IDENTITY;
  uint32_t                     main_stream_priority = 0;
  cpp_thread_msg_t             msg;
  uint32_t                     event_idx = 0, num_events = 0;
  cpp_module_event_t           *cpp_event[CPP_MODULE_MAX_STREAMS];
  boolean                      skip_frame = FALSE;
  cpp_module_hw_proc_info_t    *hw_proc_info = NULL;
  uint32_t                     duplicate_identity = 0;
  cpp_module_ack_key_t         key;
  mct_port_t                   *stream_port = NULL;

  gettimeofday(&tv1, NULL);
  if(!module || !isp_buf) {
    CPP_BUF_ERR("failed, module=%p, isp_buf=%p\n", module, isp_buf);
    return -EINVAL;
  }

  ctrl = (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_BUF_ERR("invalid control, failed\n");
    return -EFAULT;
  }

  frame_id = isp_buf->buffer.sequence;

  memset(&msg, 0 , sizeof(cpp_thread_msg_t));
  memset(&cpp_event[0], 0 , sizeof(cpp_module_event_t *) * CPP_MODULE_MAX_STREAMS);
  memset(&hw_process_list, 0, sizeof(cpp_module_process_list_t));
  memset(identity_list, 0, sizeof(uint32_t) * CPP_MODULE_MAX_STREAMS);
  memset(&key, 0 , sizeof(cpp_module_ack_key_t));

  /* get stream parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl,
    identity, &session_params, &stream_params);
  if(!stream_params) {
    CPP_BUF_ERR("invalid stream params, failed\n");
    return -EFAULT;
  }

  stream_port = (mct_port_t *) stream_params->parent;
  if (stream_port == NULL) {
    CPP_BUF_ERR("invalid stream port, failed\n");
    return -EFAULT;
  }

  /*lock afor stream on/off/buffer divert decision*/
  MCT_OBJECT_LOCK(stream_port);
  process_frame_control = CPP_FRAME_QUEUE_APPLY;
  if (stream_params->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC)
    process_frame_control |= CPP_FRAME_QUEUE_PREPARE;

  /* Apply the perframe control setting and prepare the corresponding
     reporting metadata entries */
  cpp_module_process_frame_control(module, &session_params->per_frame_params,
    identity, frame_id, process_frame_control);
  cpp_module_process_sticky_set_param_event(module, session_params,
    identity, frame_id);

  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  main_stream_priority = stream_params->priority;

  /*update frame id on all stream*/
  cpp_module_util_update_stream_frame_id(stream_params, frame_id);

  /*decide cpp hw process info*/
  ret = cpp_module_decide_process_list(module, session_params, stream_params,
    frame_id, &hw_process_list);
  if (ret < 0) {
    PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
    /* linked stream case */
    //TODO:Remove sat_status variable, should be generic
    if (stream_params->num_linked_streams > 0 && ctrl->sat_status) {
      cpp_module_stream_params_t *linked_stream_params = NULL;
      cpp_module_handle_isp_drop_buffer(module, frame_id,
        session_params->is_slave, stream_params->stream_type, identity);

      for (i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
        linked_stream_params = stream_params->linked_streams[i];
        if (linked_stream_params == NULL ||
          linked_stream_params->identity == 0)
          continue;
        cpp_module_handle_isp_drop_buffer(module, frame_id,
          session_params->is_slave, linked_stream_params->stream_type,
          linked_stream_params->identity);
      }
    } else {
      cpp_module_handle_isp_drop_buffer(module, frame_id,
        session_params->is_slave, stream_params->stream_type, identity);
    }
    CPP_HIGH("Send inplace ACK for frame %d %x", frame_id, identity);
    PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
    isp_buf->ack_flag = TRUE;
    isp_buf->is_buf_dirty = 1;
    PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
    MCT_OBJECT_UNLOCK(stream_port);
    return 0;
  }
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

  /*check if need to update clock rate.
    HAL1 only set the flag on stream on/off
    HAL3 may change per frame or for some cases*/
  if (ctrl->runtime_clk_update == TRUE) {

    PTHREAD_MUTEX_LOCK(&(ctrl->cpp_mutex));
    ctrl->runtime_clk_update = FALSE;
    PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));

    PTHREAD_MUTEX_LOCK(&(ctrl->clk_rate_list.mutex));
    ret = cpp_module_update_clock_by_process_list(
      module, ctrl, hw_process_list, session_params, stream_params);
    PTHREAD_MUTEX_UNLOCK(&(ctrl->clk_rate_list.mutex));

    if (ret < 0) {
      CPP_ERR("update clock failed! ret = %d", ret);
      MCT_OBJECT_UNLOCK(stream_port);
      return ret;
    }
  }

  /* Decide the events to be queued to process this buffer */
  /* based on configuration, at max 3 events are queued for one buffer */

  CPP_BUF_DBG("pp_dbg  num_hw process %d", hw_process_list.num_hw_process);
  /* Step 1. Based on the number of process identities set in divert config,
     generate cpp events accordingly */
  for (j = 0; j < hw_process_list.num_hw_process; j++) {
    hw_proc_info = &hw_process_list.hw_proc_info[j];
    process_stream_params = hw_proc_info->process_stream;
    if (!process_stream_params) {
      CPP_BUF_ERR("invalid stream in process info");
      continue;
    }

    CPP_BUF_DBG("cur stream _param %p process stream param %p",
      stream_params,  process_stream_params);

    /*lock and unlock for each processing stream, inside for loop*/
    PTHREAD_MUTEX_LOCK(&(process_stream_params->mutex));
    duplicate_identity = hw_proc_info->dup_identity;
    ret = cpp_module_prepare_hw_parm(module, session_params,
      process_stream_params, isp_buf);
    if (ret < 0) {
      CPP_BUF_ERR("cpp_module_prepare_hw_parm error!");
      PTHREAD_MUTEX_UNLOCK(&(process_stream_params->mutex));
      MCT_OBJECT_UNLOCK(stream_port);
      return -EINVAL;
    }

    /*decide divert id based on:
      1. mod type: SOURCE and INDEXABLE
      2. if req frame */
    div_identity = cpp_module_util_decide_divert_id(module, process_stream_params);
    if (hw_proc_info->dup_identity != 0) {
      cpp_module_get_params_for_identity(ctrl, hw_proc_info->dup_identity,
        &session_params, &temp_stream_params);
        process_stream_params->hw_params.dup_buffer_info =
          temp_stream_params->hw_params.output_buffer_info;
        if (div_identity == 0)
          div_identity = cpp_module_util_decide_divert_id(module, temp_stream_params);
    }

    /*one event per HW pass*/
    /* create a key for ack with original event identity, this key will be
       put in all corresponding events in queue and used to release the ack */
    key.identity = identity;
    key.buf_identity = isp_buf->identity;
    key.buf_idx = isp_buf->buffer.index;
    key.channel_id = isp_buf->channel_id;
    key.frame_id = isp_buf->buffer.sequence;
    key.meta_datas = isp_buf->meta_data;
    CPP_BUF_LOW("Saved identitys to key event 0x%x , buf 0x%x", key.identity,
      key.buf_identity);

    cpp_event[num_events] = cpp_module_create_cpp_event(key,
      &(process_stream_params->hw_params), isp_buf,
      process_stream_params->identity,
      div_identity, duplicate_identity);
    if(!cpp_event[num_events]) {
      CPP_BUF_ERR("CAM_MALLOC() failed\n");
      PTHREAD_MUTEX_UNLOCK(&(process_stream_params->mutex));
      MCT_OBJECT_UNLOCK(stream_port);
      return -ENOMEM;
    }
    num_events++;
   /*Resetting the grey frame state */
    process_stream_params->hw_params.grey_ref_enable = FALSE;
    PTHREAD_MUTEX_UNLOCK(&(process_stream_params->mutex));
  }

  /* before queuing any events, first put corresponding ACK in the ack_list */
  /* now enqueue all events one by one in priority queue */
  PTHREAD_MUTEX_LOCK(&(ctrl->cpp_mutex));
  if (ctrl->cpp_thread_started) {
    if (num_events != 0) {
      cpp_module_put_new_ack_in_list(ctrl, key, 1, num_events, isp_buf);
      for(i = 0; i < num_events; i++) {
        ret = cpp_module_enq_event(ctrl, cpp_event[i], main_stream_priority);
        if(ret < 0) {
          /* Should'nt we free here */
          free(cpp_event[i]);
          cpp_event[i] = NULL;
          CPP_BUF_ERR("failed, i=%d\n", i);
          PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
          MCT_OBJECT_UNLOCK(stream_port);
          return -EFAULT;
        }
      }

      /* notify the thread about this new events */
      msg.type = CPP_THREAD_MSG_NEW_EVENT_IN_Q;
      cpp_module_post_msg_to_thread(ctrl, msg);
    } else {
      /* if no events needs to be queued, do a piggy-back ACK */
      isp_buf->ack_flag = TRUE;
      isp_buf->is_buf_dirty = 1;
      CPP_HIGH("buffer event %d received with no divert config\n", frame_id);
      PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
      MCT_OBJECT_UNLOCK(stream_port);
      return 0;
    }
  } else {
    isp_buf->ack_flag = TRUE;
    isp_buf->is_buf_dirty = 1;
    CPP_BUF_DBG("Thread not started, piggy back ack\n");
    PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
    MCT_OBJECT_UNLOCK(stream_port);
    return -EINVAL;
  }
  PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));

  gettimeofday(&tv2, NULL);
  CPP_BUF_LOW("downstream event time = %6ld us",
    (tv2.tv_sec - tv1.tv_sec) * 1000000L +
    (tv2.tv_usec - tv1.tv_usec));

  MCT_OBJECT_UNLOCK(stream_port);
  return 0;
}

/** cpp_module_handle_buf_divert_event:
 *
 * Description:
 *  Handle the MCT_EVENT_MODULE_BUF_DIVERT event. First put corresponding
 *  acknowledgement in a list which will be sent later. Depending on the
 *  stream's parameters, divert and processing events are added in
 *  cpp's priority queue. cpp_thread will pick up these events one by one in
 *  order and when all events corresponding to the ACK are processed,
 *  the ACK will be removed from list and will be sent upstream.
 *
 **/
int32_t cpp_module_handle_buf_divert_event(mct_module_t* module,
  mct_event_t* event)
{
  cpp_module_ctrl_t           *ctrl = NULL;
  isp_buf_divert_t            *isp_buf = NULL;
  uint32_t                     frame_id = 0;
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_stream_params_t  *linked_stream_params = NULL;
  cpp_module_frame_hold_t     *frame_hold = NULL;
  cpp_module_dis_hold_t       *dis_hold = NULL;
  int32_t                      ret = 0;

  if(!module || !event) {
    CPP_BUF_ERR("failed, module=%p, event=%p\n", module, event);
    return -EINVAL;
  }

  ctrl = (cpp_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  if (!ctrl) {
    CPP_BUF_ERR("failed ctrl %p\n", ctrl);
    return -EFAULT;
  }

  isp_buf = (isp_buf_divert_t *)(event->u.module_event.module_event_data);
  if (!isp_buf) {
    CPP_BUF_ERR("isp_buf %p\n", isp_buf);
    return -EFAULT;
  }

  if((isp_buf->bayerdata == 1) || ctrl->is_hw_error) {
    /* Send acknowledge to free the buffer. */
    isp_buf->ack_flag = 1;
    isp_buf->is_buf_dirty = 1;
    CPP_BUF_DBG("send inplace ack %d\n", ctrl->is_hw_error);
    return 0;
  }

  if (isp_buf->pass_through == 1) {
    /* This buffer divert event simply needs a bypass through. So directly
       send to downstream module. This event is not queued and ref counted.
       The rule is that downstream module is expected to do a piggyback ack.
       This type of unprocess divert is not very clean though. */
    ret = cpp_module_send_event_downstream(ctrl->p_module, event);
    if (ret < 0) {
      CPP_BUF_ERR("failed %d", ret);
    }
    return ret;
  }

  frame_id = isp_buf->buffer.sequence;

  cpp_module_get_params_for_identity(ctrl, event->identity, &session_params,
     &stream_params);
  if (!session_params || !stream_params) {
    CPP_BUF_ERR("failed params %p %p\n", session_params, stream_params);
    return -EFAULT;
  }

  CPP_HIGH("received buffer divert for %d, identity %x\n", frame_id, event->identity);
  PTHREAD_MUTEX_LOCK(&(session_params->dis_mutex));
  if (IF_IS_HOLD_REQD(session_params, stream_params)) {
    /* Check whether there is already one frame on HOLD */
    frame_hold = &session_params->frame_hold;
    if (frame_hold->is_frame_hold == TRUE) {
      /* DIS crop event is not sent for frame on HOLD yet. But next frame
        is received. Send frame on HOLD for processing */
      CPP_BUF_DBG("dis not received for previous frame -> %d for processing\n",
        frame_hold->isp_buf.buffer.sequence);
      cpp_module_send_buf_divert_event(module, frame_hold->identity,
        &frame_hold->isp_buf);
      /* Set is_frame_hold flag to FALSE */
      frame_hold->is_frame_hold = FALSE;
    }
    dis_hold = &session_params->dis_hold;
    /* Check whether DIS frame id is valid &&
       Check whether DIS crop event for this frame has already arrived */
    if (dis_hold->is_valid == TRUE && frame_id <= dis_hold->dis_frame_id) {
      CPP_BUF_DBG("DIS already arrived for %d, send for processing\n",
        isp_buf->buffer.sequence);
      /* Send current frame for processing */
      cpp_module_send_buf_divert_event(module, event->identity, isp_buf);
    } else if (TRUE == cpp_module_util_check_link_streamon(stream_params)) {
      /* DIS frame id is either invalid or DIS crop event for this frame
         has not arrived yet. HOLD this frame */
      CPP_BUF_DBG("HOLD %d\n", isp_buf->buffer.sequence);
      frame_hold->is_frame_hold = TRUE;
      frame_hold->identity = event->identity;
      // copying input buffer's buffer_access flag into frame_hold
      memcpy(&frame_hold->isp_buf, isp_buf, sizeof(isp_buf_divert_t));
    } else {
      /* Send acknowledge to free the buffer. */
      isp_buf->ack_flag = 1;
      isp_buf->is_buf_dirty = 1;
    }
  } else {
    CPP_BUF_LOW("send %d for processing\n", isp_buf->buffer.sequence);
    cpp_module_send_buf_divert_event(module, event->identity, isp_buf);
  }
  PTHREAD_MUTEX_UNLOCK(&(session_params->dis_mutex));
  return 0;
}


/* cpp_module_handle_isp_out_dim_event:
 *
 * Description:
 *
 **/
int32_t cpp_module_handle_isp_out_dim_event(mct_module_t* module,
  mct_event_t* event)
{
  uint32_t k;
  int32_t rc;
  if(!module || !event) {
    CPP_ERR("failed, module=%p, event=%p\n", module, event);
    return -EINVAL;
  }
  cpp_module_ctrl_t* ctrl = (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_ERR("invalid control, failed\n");
    return -EFAULT;
  }
  mct_stream_info_t *stream_info =
    (mct_stream_info_t *)(event->u.module_event.module_event_data);
  if(!stream_info) {
    CPP_ERR("invalid streaminfo, failed\n");
    return -EFAULT;
  }
  CPP_HIGH("identity=0x%x, dim=%dx%d fmt= %d\n", event->identity,
    stream_info->dim.width, stream_info->dim.height, stream_info->fmt);
  /* get stream parameters based on the event identity */
  cpp_module_stream_params_t *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CPP_ERR("invalid streamparams, failed\n");
    return -EFAULT;
  }

  /* format info */
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  rc = cpp_module_util_update_format(stream_info, &stream_params->hw_params.input_info);
  if (rc < 0 ) {
    PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
    return -EINVAL;
  }

  /* update the dimension of the stream */
  cpp_module_util_update_dim(stream_info,&stream_params->hw_params.input_info);
  CPP_INFO("[STREAM_PARAMS]  stream type %d, width %d, height %d,"
      "stride %d, scanline %d, planes %d", stream_params->hw_params.stream_type,
      stream_params->hw_params.output_info.width,
      stream_params->hw_params.output_info.height,
      stream_params->hw_params.output_info.stride,
      stream_params->hw_params.output_info.scanline,
      stream_info->buf_planes.plane_info.num_planes);

  CPP_LOW("stream_params->native_buff :%d",stream_params->native_buff);
  if (stream_params->native_buff > 0) {
    cpp_module_util_update_dim(stream_info, &stream_params->hw_params.output_info);
  }

  /* init crop info */
  stream_params->hw_params.crop_info.stream_crop.x = 0;
  stream_params->hw_params.crop_info.stream_crop.y = 0;
  stream_params->hw_params.crop_info.stream_crop.dx = stream_info->dim.width;
  stream_params->hw_params.crop_info.stream_crop.dy = stream_info->dim.height;
  stream_params->hw_params.crop_info.is_crop.x = 0;
  stream_params->hw_params.crop_info.is_crop.y = 0;
  stream_params->hw_params.crop_info.is_crop.dx = stream_info->dim.width;
  stream_params->hw_params.crop_info.is_crop.dy = stream_info->dim.height;

  /* whether stream use scratch buffer or not */
  stream_params->hw_params.expect_divert = stream_info->expect_divert;

  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
  rc = cpp_module_send_event_downstream(module, event);
  if(rc < 0) {
    CPP_ERR("failed, module_event_type=%d, identity=0x%x",
      event->u.module_event.type, event->identity);
    return -EFAULT;
  }
  return 0;
}

/* cpp_module_handle_isp_drop_buffer:
 *
 * Description:
 *
 **/
int32_t cpp_module_handle_isp_drop_buffer(mct_module_t* module,
  uint32_t frame_id, bool is_slave, cam_stream_type_t stream_type,
  uint32_t identity)
{
  int32_t                       rc, i = 0;
  cpp_module_stream_params_t   *stream_params = NULL;
  cpp_module_session_params_t  *session_params = NULL;
  cpp_module_ctrl_t            *ctrl = NULL;
  cpp_module_event_t           *isp_buffer_drop_event = NULL;
  uint32_t                  stream_frame_valid = 0;
  uint32_t process_frame_control;

  if(!module) {
    CPP_BUF_ERR("failed, module=%p\n", module);
    return -EINVAL;
  }

  ctrl = (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_BUF_ERR("invalid cpp control, failed\n");
    return -EFAULT;
  }

  uint32_t session_id = CPP_GET_SESSION_ID(identity);
  uint32_t stream_id = CPP_GET_STREAM_ID(identity);

  /* Assuming HAL doesn't allocate output buffer for
     slave session i.e no pcr for slave session */
  if (!ctrl->sat_status && is_slave)
    return 0;


  rc = cpp_module_check_pcr_with_frameid(ctrl, frame_id,
    &session_params, &stream_params, stream_type, identity);
  if (rc < 0) {
    CPP_BUF_DBG("No PCR for identity %x frame id %d",identity, frame_id);
    return 0;
  }
  if (!session_params || !stream_params) {
    CPP_BUF_ERR("failed session_params %p, stream_params %p\n",
      session_params, stream_params);
    return -EFAULT;
  }

  if (stream_params->enable_post_cpp == 1)
    return 0;

  process_frame_control = CPP_FRAME_QUEUE_APPLY;
  if (stream_params->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC)
    process_frame_control |= CPP_FRAME_QUEUE_PREPARE;

  /* Apply the perframe control setting and prepare the corresponding
     reporting metadata entries */
  cpp_module_process_frame_control(module, &session_params->per_frame_params,
    stream_params->identity, frame_id, process_frame_control);

  if (session_params->hal_version != CAM_HAL_V3) {
    CPP_BUF_HIGH("Drop event not valid on HAL1,ignore");
    return 0;
  }

  /* Check whether frame processing is valid. Only then the buffer
     is available in stream queue */
  if (stream_params->is_stream_on == TRUE) {
    stream_frame_valid = cpp_module_get_frame_valid(module,
      stream_params->identity, frame_id,
      PPROC_GET_STREAM_ID(stream_params->identity),
      stream_params->stream_type, false);
  }

  if (stream_frame_valid) {
    CPP_BUF_DBG("Request is valid");
    isp_buffer_drop_event =
      (cpp_module_event_t*)CAM_MALLOC(sizeof(cpp_module_event_t));
    if(!isp_buffer_drop_event) {
      CPP_BUF_ERR("CAM_MALLOC failed\n");
      return -ENOMEM;
    }
    memset(isp_buffer_drop_event, 0x00, sizeof(cpp_module_event_t));
    isp_buffer_drop_event->hw_process_flag = TRUE;
    isp_buffer_drop_event->type = CPP_MODULE_EVENT_ISP_BUFFER_DROP;
    //isp_buffer_drop_event->ack_key = 0; // No ACK expected so key is not needed
    isp_buffer_drop_event->invalid = FALSE;
    isp_buffer_drop_event->u.drop_buffer.frame_id =
      frame_id;
    isp_buffer_drop_event->u.drop_buffer.stream_params = stream_params;

    CPP_HIGH("ISP buf drop for id %x frame %d \n",
      stream_params->identity, frame_id);

    PTHREAD_MUTEX_LOCK(&(ctrl->cpp_mutex));
    if (ctrl->cpp_thread_started) {
      cpp_thread_msg_t msg;
      rc = cpp_module_enq_event(ctrl, isp_buffer_drop_event,
          CPP_PRIORITY_REALTIME);
      if (rc < 0) {
        CPP_BUF_LOW("Enqueue event failed");
        free(isp_buffer_drop_event);
        PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
        return rc;
      }
      memset(&msg, 0, sizeof(cpp_thread_msg_t));
      msg.type = CPP_THREAD_MSG_NEW_EVENT_IN_Q;
      cpp_module_post_msg_to_thread(ctrl, msg);
    } else {
      free(isp_buffer_drop_event);
      CPP_BUF_HIGH("Thread not active, dont queue for identity %x, frame %d",
        stream_params->identity, frame_id);
      PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
      return -EINVAL;
    }
    PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
  } else {
    CPP_BUF_DBG("ISP buf drop for invalid frame\n");
    //Ignore the error if drop is on invalid frame. need not be a fatal error.
    return 0;
  }

  CPP_BUF_DBG("Exit\n");
  return 0;
}


/* cpp_module_handle_aec_manual_update:
 *
 * Description:
 *
 **/
int32_t cpp_module_handle_aec_manual_update(mct_module_t *module,
  mct_event_t *event)
{
  int32_t                       rc, i = 0;
  cpp_params_aec_trigger_info_t aec_trigger;
  aec_manual_update_t          *aec_manual_update;
  cpp_module_ctrl_t            *ctrl = NULL;
  cpp_module_stream_params_t   *stream_params = NULL;
  cpp_module_session_params_t  *session_params = NULL;

  if(!module || !event) {
    CPP_ERR("failed, module=%p, event=%p\n", module, event);
    return -EINVAL;
  }

  CPP_DBG("identity=0x%x", event->identity);

  ctrl = (cpp_module_ctrl_t*)MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_ERR("invalid cpp control, failed\n");
    return -EFAULT;
  }

  /* get stream parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!session_params || !stream_params) {
    CPP_ERR("invalid session %p stream %p failed\n", session_params, stream_params);
    return -EFAULT;
  }

  aec_manual_update =
    (aec_manual_update_t *)event->u.module_event.module_event_data;
  aec_trigger.gain = aec_manual_update->sensor_gain;
  aec_trigger.lux_idx = aec_manual_update->lux_idx;
  aec_trigger.exp_time_ratio = 1.5f;
  aec_trigger.aec_sensitivity_ratio = 1.5f;
  aec_trigger.total_adrc_gain = 1.5f;
  aec_trigger.drc_color_gain = 1.5f;
  if (stream_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    cpp_module_update_aec(module, stream_params->identity, &aec_trigger);
  } else {
    session_params->aec_trigger.gain = aec_manual_update->sensor_gain;
    session_params->aec_trigger.lux_idx = aec_manual_update->lux_idx;
    session_params->aec_trigger.exp_time_ratio = 1.5f;
    session_params->aec_trigger.aec_sensitivity_ratio = 1.5f;
    session_params->aec_trigger.total_adrc_gain = 1.5f;
    session_params->aec_trigger.drc_color_gain = 1.5f;
    for(i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
      if(session_params->stream_params[i] &&
        session_params->stream_params[i]->stream_type !=
        CAM_STREAM_TYPE_OFFLINE_PROC) {
        cpp_module_update_aec(module,
          session_params->stream_params[i]->identity, &aec_trigger);
      }
    }
  }

  rc = cpp_module_send_event_downstream(module, event);
  if(rc < 0) {
    CPP_ERR("failed, module_event_type=%d, identity=0x%x",
      event->u.module_event.type, event->identity);
    return -EFAULT;
  }
  return 0;
}

/* cpp_module_update_aec:
 *
 * Description:
 *
 **/
int32_t cpp_module_update_aec(mct_module_t *module,
  uint32_t identity, cpp_params_aec_trigger_info_t *aec_trigger)
{
  cpp_module_ctrl_t           *ctrl = NULL;
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;

  if(!module) {
    CPP_ERR("failed, module=%p\n", module);
    return -EINVAL;
  }

  ctrl = (cpp_module_ctrl_t*)MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_ERR("invalid control, failed\n");
    return -EFAULT;
  }

  /* get stream parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CPP_ERR("invalid stream params, failed\n");
    return -EFAULT;
  }
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  if (!F_EQUAL(stream_params->hw_params.aec_trigger.lux_idx, aec_trigger->lux_idx) ||
      !F_EQUAL(stream_params->hw_params.aec_trigger.gain, aec_trigger->gain) ||
      !F_EQUAL(stream_params->hw_params.aec_trigger.exp_time_ratio,
        aec_trigger->exp_time_ratio) ||
      !F_EQUAL(stream_params->hw_params.aec_trigger.aec_sensitivity_ratio,
        aec_trigger->aec_sensitivity_ratio)) {
    stream_params->hw_params.aec_trigger.lux_idx = aec_trigger->lux_idx;
    stream_params->hw_params.aec_trigger.gain = aec_trigger->gain;
    CPP_DBG("aec lux %f, gain %f, stream type %d", aec_trigger->lux_idx,
      aec_trigger->gain,  stream_params->hw_params.stream_type);
    stream_params->hw_params.aec_trigger.exp_time_ratio = aec_trigger->exp_time_ratio;
    stream_params->hw_params.aec_trigger.aec_sensitivity_ratio = aec_trigger->aec_sensitivity_ratio;
    stream_params->hw_params.aec_trigger.total_adrc_gain = aec_trigger->total_adrc_gain;
    stream_params->hw_params.aec_trigger.drc_color_gain = aec_trigger->drc_color_gain;
    stream_params->hw_params.interpolate_mask |= CPP_INTERPOLATE_ALL;

    stream_params->hw_params.low_light_capture_enable =
      aec_trigger->low_light_capture_update_flag;
  } else {
    CPP_DBG("skip aec update with same lux idx and gain.");
  }
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

  return 0;
}

/* cpp_module_handle_aec_update_event:
 *
 * Description:
 *
 **/
int32_t cpp_module_handle_aec_update_event(mct_module_t* module,
  mct_event_t* event)
{
  stats_update_t               *stats_update;
  aec_update_t                 *aec_update;
  int32_t                       rc, i = 0;
  cpp_params_aec_trigger_info_t aec_trigger;
  cpp_module_stream_params_t   *stream_params = NULL;
  cpp_module_session_params_t  *session_params = NULL;
  cpp_module_ctrl_t            *ctrl = NULL;

  if(!module || !event) {
    CPP_ERR("failed, module=%p, event=%p\n", module, event);
    return -EINVAL;
  }

  CPP_DBG("identity=0x%x", event->identity);

  ctrl = (cpp_module_ctrl_t*)MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_ERR("invalid cpp control, failed\n");
    return -EFAULT;
  }

  /* get stream parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!session_params || !stream_params) {
    CPP_ERR("invalid stream %p, session %p params failed\n",
      session_params, stream_params);
    return -EFAULT;
  }

  stats_update =
      (stats_update_t *)event->u.module_event.module_event_data;
  aec_update = &stats_update->aec_update;

  if (stats_update->flag & STATS_UPDATE_AEC) {
    aec_trigger.gain = aec_update->real_gain;
    aec_trigger.lux_idx = aec_update->lux_idx;
    aec_trigger.exp_time_ratio = aec_update->hdr_exp_time_ratio;
    aec_trigger.aec_sensitivity_ratio = aec_update->hdr_sensitivity_ratio;
    aec_trigger.total_adrc_gain = aec_update->total_drc_gain;
    aec_trigger.drc_color_gain = aec_update->color_drc_gain;
    aec_trigger.low_light_capture_update_flag = aec_update->low_light_capture_update_flag;

    if (stream_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
      cpp_module_update_aec(module, stream_params->identity, &aec_trigger);
    } else {
      session_params->aec_trigger.gain = aec_update->real_gain;
      session_params->aec_trigger.lux_idx = aec_update->lux_idx;
      session_params->aec_trigger.exp_time_ratio =
        aec_update->hdr_exp_time_ratio;
      session_params->aec_trigger.aec_sensitivity_ratio=
        aec_update->hdr_sensitivity_ratio;
      session_params->hw_params.low_light_capture_enable =
        aec_update->low_light_capture_update_flag;
      session_params->aec_trigger.total_adrc_gain =
        aec_update->total_drc_gain;
      session_params->aec_trigger.drc_color_gain =
        aec_update->color_drc_gain;
      /*OIS Capture  for session*/
      for(i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
        if(session_params->stream_params[i] &&
          session_params->stream_params[i]->stream_type !=
          CAM_STREAM_TYPE_OFFLINE_PROC) {
          cpp_module_update_aec(module,
            session_params->stream_params[i]->identity, &aec_trigger);
          /*OIS Capture */
        }
      }
    }

  }

  rc = cpp_module_send_event_downstream(module, event);
  if(rc < 0) {
    CPP_ERR("failed, module_event_type=%d, identity=0x%x",
      event->u.module_event.type, event->identity);
    return -EFAULT;
  }
  return 0;
}


/* cpp_module_extract_chromatix:
 *
 * Description:
 *
 **/
void cpp_module_extract_chromatix(cam_stream_type_t stream_type,
  modulesChromatix_t *stream_chromatix,
  modulesChromatix_t *input_chromatix)
{
  if (!(input_chromatix->modules_reloaded & CAMERA_CHROMATIX_MODULE_CPP)) {
    CPP_DBG("Donot reload chromatix - no change stream %d", stream_type);
    return;
  }
  stream_chromatix->chromatixCppPtr = input_chromatix->chromatixCppPtr;

  if (stream_type == CAM_STREAM_TYPE_VIDEO) {
    stream_chromatix->chromatixCppPtr = input_chromatix->chromatixVideoCppPtr;
  }
  /* For streamtypes Snapshot/liveshot/callback if snap chromatix
     pointer is available, stream chromatix pointer can use it */
  if ((stream_type == CAM_STREAM_TYPE_SNAPSHOT) ||
    (stream_type == CAM_STREAM_TYPE_CALLBACK) ||
    (stream_type == CAM_STREAM_TYPE_OFFLINE_PROC)){
    if (input_chromatix->chromatixSnapCppPtr) {
      stream_chromatix->chromatixCppPtr = input_chromatix->chromatixSnapCppPtr;
      stream_chromatix->chromatixSnapCppPtr = input_chromatix->chromatixSnapCppPtr;
    }
    if (input_chromatix->chromatixUsCppPtr) {
      stream_chromatix->chromatixUsCppPtr = input_chromatix->chromatixUsCppPtr;
    }
    if (input_chromatix->chromatixDsCppPtr) {
      stream_chromatix->chromatixDsCppPtr = input_chromatix->chromatixDsCppPtr;
    }
  }

  CPP_DBG("[MOD_CHROMATIX]  chromatix ptr: stream_type %d, stream_chromatix %p,"
    "snap %p, downscale %p, upscale %p\n",
    stream_type, stream_chromatix->chromatixCppPtr,
    stream_chromatix->chromatixSnapCppPtr,
    stream_chromatix->chromatixDsCppPtr,
    stream_chromatix->chromatixUsCppPtr);
}

/* cpp_module_handle_chromatix_ptr_event:
 *
 * Description:
 *
 **/
int32_t cpp_module_handle_chromatix_ptr_event(mct_module_t* module,
  mct_event_t* event)
{
  modulesChromatix_t *chromatix_param;
  int32_t rc;

  if(!module || !event) {
    CPP_ERR("failed, module=%p, event=%p\n", module, event);
    return -EINVAL;
  }
  cpp_module_ctrl_t* ctrl = (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_ERR("invalid control, failed\n");
    return -EFAULT;
  }
  CPP_DBG("identity=0x%x\n", event->identity);
  /* get stream parameters based on the event identity */
  cpp_module_stream_params_t *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CPP_ERR("invalid stream params, failed\n");
    return -EFAULT;
  }

  chromatix_param =
    (modulesChromatix_t *)event->u.module_event.module_event_data;

  if (!chromatix_param) {
    CPP_ERR("failed, chromatix_param NULL");
    return -EINVAL;
  }
  CPP_HIGH("Chromatix ptr: [MOD_CHROMATIX] com :%p c:%p prev_cpp:%p snap_cpp:%p "
    "snap upscale %p, snap_down %p, video_cpp:%p postproc:%p ois_snap %p",
    chromatix_param->chromatixComPtr,  chromatix_param->chromatixPtr,
    chromatix_param->chromatixCppPtr, chromatix_param->chromatixSnapCppPtr,
    chromatix_param->chromatixUsCppPtr,
    chromatix_param->chromatixDsCppPtr,
    chromatix_param->chromatixVideoCppPtr,
    chromatix_param->chromatixPostProcPtr,
    chromatix_param->chromatixOisSnapCppPtr);
  /* Update the chromatix ptr in session params.
     This chromatix ptr will be used by all streams in this session */
  if(stream_params->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) {
    uint32_t i;
    session_params->module_chromatix = *chromatix_param;

    for (i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
      if (session_params->stream_params[i] &&
        (session_params->stream_params[i]->stream_type !=
        CAM_STREAM_TYPE_OFFLINE_PROC)) {
        PTHREAD_MUTEX_LOCK(&(session_params->stream_params[i]->mutex));
        session_params->stream_params[i]->hw_params.module_chromatix = *chromatix_param;
        PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));
      }
    }
  } else {
    PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
    stream_params->hw_params.module_chromatix = *chromatix_param;
    PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
  }

  rc = cpp_module_send_event_downstream(module, event);
  if(rc < 0) {
    CPP_ERR("failed, module_event_type=%d, identity=0x%x",
      event->u.module_event.type, event->identity);
    return -EFAULT;
  }

  return 0;
}

/* cpp_module_handle_stream_crop_event:
 *
 * Description:
 *
 **/
int32_t cpp_module_handle_stream_crop_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t rc;
  cpp_hardware_params_t *hw_params = NULL;
  cpp_module_stream_params_t *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;

  if(!module || !event) {
    CPP_CROP_ERR("failed, module=%p, event=%p\n", module, event);
    return -EINVAL;
  }
  cpp_module_ctrl_t* ctrl = (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_CROP_ERR("invalid cpp control, failed\n");
    return -EFAULT;
  }

  mct_bus_msg_stream_crop_t *stream_crop =
    (mct_bus_msg_stream_crop_t *) event->u.module_event.module_event_data;
  if(!stream_crop) {
    CPP_CROP_ERR("invalid stream crop, failed\n");
    return -EFAULT;
  }
  /* if crop is (0, 0, 0, 0) ignore the event */
  if (stream_crop->x == 0 && stream_crop->y == 0 &&
      stream_crop->crop_out_x == 0 && stream_crop->crop_out_y == 0) {
    return 0;
  }
  /* get stream parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CPP_CROP_ERR("invalid stream params, failed\n");
    return -EFAULT;
  }
  hw_params = &stream_params->hw_params;
  if ((!hw_params->input_info.width) ||
    (!hw_params->input_info.height)) {
    CPP_CROP_HIGH("Crop event received before output dimension");
    return 0;
  }

  /* In bokeh snapshot usecase, in case of bad environment
     conditions, CPP needs to do crop operation. But In case of
     good enivironment, HAL will send ingore_crop request
     to PPROC, Jpeg will do crop instead of CPP*/
  if (ctrl->rtb_status &&
    (stream_params->stream_type == CAM_STREAM_TYPE_PREVIEW)) {
    hw_params->interpolate_mask |= CPP_INTERPOLATE_ALL;
    hw_params->isp_width_map = stream_crop->width_map;
    hw_params->isp_height_map = stream_crop->height_map;
    hw_params->isp_scaler_ratio = stream_crop->scale_ratio;
    return 0;
  }

  IS_CROP_PARAM_VALID(stream_crop->x, hw_params->input_info.width, 0);
  IS_CROP_PARAM_VALID(stream_crop->y, hw_params->input_info.height, 0);
  IS_CROP_PARAM_VALID(stream_crop->crop_out_x, hw_params->input_info.width,
    hw_params->input_info.width);
  IS_CROP_PARAM_VALID(stream_crop->crop_out_y, hw_params->input_info.height,
    hw_params->input_info.height);

  if (((stream_crop-> x  + stream_crop->crop_out_x) <=
    hw_params->input_info.width) &&
    ((stream_crop->y + stream_crop->crop_out_y) <=
    hw_params->input_info.height)) {
    PTHREAD_MUTEX_LOCK(&(stream_params->mutex));

    /* if crop is same as before, ignore the event */
    if (stream_crop->x != hw_params->crop_info.stream_crop.x ||
      stream_crop->y != hw_params->crop_info.stream_crop.y ||
      stream_crop->crop_out_x != hw_params->crop_info.stream_crop.dx ||
      stream_crop->crop_out_y != hw_params->crop_info.stream_crop.dy ||
      stream_crop->width_map != hw_params->isp_width_map ||
      stream_crop->height_map != hw_params->isp_height_map) {

      hw_params->interpolate_mask |= CPP_INTERPOLATE_ALL;
      if (stream_params->hw_params.crop_enable) {
        hw_params->crop_info.stream_crop.x = stream_crop->x;
        hw_params->crop_info.stream_crop.y = stream_crop->y;
        hw_params->crop_info.stream_crop.dx = stream_crop->crop_out_x;
        hw_params->crop_info.stream_crop.dy = stream_crop->crop_out_y;
      }

      /* correct dimension when scaling cropped image
         when the cropping starts from (0,0) */
      if (hw_params->crop_info.stream_crop.x == 0 &&
        hw_params->crop_info.stream_crop.y == 0 &&
        hw_params->crop_info.is_crop.x == 0 &&
        hw_params->crop_info.is_crop.y == 0) {

        hw_params->input_info.width = hw_params->crop_info.stream_crop.dx;
        hw_params->input_info.height = hw_params->crop_info.stream_crop.dy;
        hw_params->crop_info.is_crop.dx = hw_params->crop_info.stream_crop.dx;
        hw_params->crop_info.is_crop.dy = hw_params->crop_info.stream_crop.dy;
      }

      hw_params->isp_width_map = stream_crop->width_map;
      hw_params->isp_height_map = stream_crop->height_map;

      hw_params->isp_scaler_ratio = stream_crop->scale_ratio;
      CPP_CROP_DBG("frame id %d, stream_crop.x=%d, stream_crop.y=%d,"
        "stream_crop.dx=%d, stream_crop.dy=%d scale_ratio %f, identity=0x%x",
        stream_crop->frame_id, stream_crop->x, stream_crop->y,
        stream_crop->crop_out_x, stream_crop->crop_out_y, stream_crop->scale_ratio,
        event->identity);
    } else {
      CPP_CROP_DBG("No crop update required");
    }
    PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
  }
  /* This crop info event cannot be sent out as it is. This need to be a
     new crop event based on whether CPP can handle the requested crop.
     Only residual crop needs to be sent out */
  /* Because this event can be applied either directly or after queueing
     the caller should take care of forwarding to downstream appropriately */
#if 0
  rc = cpp_module_send_event_downstream(module, event);
  if(rc < 0) {
    CPP_ERR("failed, module_event_type=%d, identity=0x%x",
      event->u.module_event.type, event->identity);
    return -EFAULT;
  }
#endif
  return 0;
}

/* cpp_module_handle_stream_fovc_crop_factor_event:
 *
 * Update the stream parameter - Field of View Compensation (FOVC) crop factor
 * value based on the event propagated from MCT -> AF Port -> CPP.
 *
 * The value of which is then used in cpp_params_calculate_crop() to inform the
 * CPP hardware for appropriate cropping and/or down scaling to generate output
 * frame/buffer.
 *
 * @ module[in] - event originating mct module
 * @ event[in]  - event data to be handled
 *
 * Return: SUCCESS (0), if no errors encountered
 *         -EINVAL,     otherwise
 *
 **/
int32_t cpp_module_handle_stream_fovc_crop_factor_event(
  mct_module_t *module,
  mct_event_t *event)
{
  int32_t rc = 0;
  int32_t si = 0;
  float *fovc_crop_factor = NULL;
  cpp_hardware_params_t *hw_params = NULL;
  cpp_module_stream_params_t *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;

  /* Validate module, event and stream parameters */
  rc = cpp_module_util_check_event_params(
         module,
         event,
         &session_params,
         &stream_params);
  if (rc) {
    CPP_ERR("failed, module_event_type=%d, identity=0x%x",
      event->u.module_event.type,
      event->identity);
    return -EFAULT;
  }

  fovc_crop_factor = (float *)(event->u.module_event.module_event_data);
  if (!fovc_crop_factor) {
    CPP_CROP_ERR("invalid stream fovc crop factor, failed\n");
    return -EFAULT;
  }

  CPP_CROP_DBG("fovc crop factor=%f, frame id=%d, identity=0x%x",
    *fovc_crop_factor,
    event->u.module_event.current_frame_id,
    event->identity);

  /*
   * Update hardware parameters with fovc crop factor
   */
  hw_params =
    (stream_params->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) ?
      &session_params->hw_params:  /* all streams in a session */
      &stream_params->hw_params;   /* current stream */

  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));

  /* apply to either all streams in a session or current stream */
  hw_params->crop_info.fovc_crop_factor = *fovc_crop_factor;

  /* apply to all streams in a session, if needed */
  if (stream_params->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) {
    for (si = 0; si < CPP_MODULE_MAX_STREAMS; si++) {
      cpp_module_stream_params_t * l_stream_params =
        session_params->stream_params[si];
      if (l_stream_params &&
          (l_stream_params-> stream_type != CAM_STREAM_TYPE_OFFLINE_PROC)) {
        l_stream_params->hw_params.crop_info.fovc_crop_factor =
          *fovc_crop_factor;
      }
    }
  }

  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

  return rc;
}  /* cpp_module_handle_stream_fovc_crop_factor_event() */

/* cpp_module_handle_stream_dualcam_shift_offset_event:
 *
 * @ module - Event originating MCT module
 * @ event  - MCT event to be handled
 *
 * Update the stream parameters - Dual Camera Shift Offset (DCSO) values based
 * on the event propagated from either HAL/MCT -> CPP for the reprocess/offline
 * stream (sent as part of metadata) or DualCam -> CPP for the live/real-time
 * streams (sent as part of MCT event).
 *
 * The value of which is then used in cpp_params_calculate_crop() to inform the
 * CPP hardware for appropriate 2-dimensional shift in generating the output
 * frame/buffer.
 *
 **/
int32_t cpp_module_handle_stream_dualcam_shift_offset_event(
  mct_module_t *module,
  mct_event_t *event)
{
  int32_t rc = 0;
  uint16_t si = 0;
  cam_sac_output_info_t *dualcam_shift_offsets = NULL;
  cpp_module_stream_params_t *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;

  /* Validate module, event and stream parameters */
  if ((rc = cpp_module_util_check_event_params(module,
              event,
              &session_params,
              &stream_params))) {
    CPP_ERR("failed, module_event_type=%d, identity=0x%x",
      event->u.module_event.type,
      event->identity);
    return -EFAULT;
  }

  dualcam_shift_offsets =
    (cam_sac_output_info_t *)(event->u.module_event.module_event_data);
  if (!dualcam_shift_offsets) {
    CPP_ERR("invalid stream dual cam shift offset, failed\n");
    return -EFAULT;
  }

  CPP_DBG("Dual camera shift offsets: \n"
    "is_output_shift_valid=%u, output_shift:{sh=%i,sv=%i}, "
    "reference_res_for_output_shift:{w=%i,h=%i}, "
    "frame id=%d, identity=0x%x",
    dualcam_shift_offsets->is_output_shift_valid,
    dualcam_shift_offsets->output_shift.shift_horz,
    dualcam_shift_offsets->output_shift.shift_vert,
    dualcam_shift_offsets->reference_res_for_output_shift.width,
    dualcam_shift_offsets->reference_res_for_output_shift.height,
    event->u.module_event.current_frame_id,
    event->identity);

  if (stream_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    PTHREAD_MUTEX_LOCK(&(stream_params->mutex));

    stream_params->hw_params.dualcam_shift_offsets = *dualcam_shift_offsets;

    PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
    return rc;
  }

  for (si = 0; si < CPP_MODULE_MAX_STREAMS; si++) {

    stream_params = session_params->stream_params[si];
    if (!stream_params) {
      continue;
    }

    switch (stream_params->stream_type) {
      case CAM_STREAM_TYPE_PREVIEW:
      case CAM_STREAM_TYPE_VIDEO:
      case CAM_STREAM_TYPE_SNAPSHOT:
      case CAM_STREAM_TYPE_CALLBACK: {
        PTHREAD_MUTEX_LOCK(&(stream_params->mutex));

        stream_params->hw_params.dualcam_shift_offsets = *dualcam_shift_offsets;

        PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
        }
        break;
      case CAM_STREAM_TYPE_OFFLINE_PROC: {
        /* Offline case handled as above already, through stream params */
        }
        break;
      default: {
        CPP_LOW("Dual camera shift offsets is not supported for stream %d",
          stream_params->stream_type);
        }
      }
    }

  return rc;
}  /* cpp_module_handle_stream_dualcam_shift_offset_event() */

/* cpp_module_handle_stream_dualcam_shift_margins_event:
 *
 * @ module - Event originating MCT module
 * @ event  - MCT event to be handled
 *
 * Update the stream parameters - Dual Camera Shift Margins (DCSM) values based
 * on the event propagated from either HAL/MCT -> CPP for the reprocess/offline
 * stream (sent as part of metadata) or DualCam -> CPP for the live/real-time
 * streams (sent as part of MCT event).
 *
 * The value of which is then used in cpp_params_calculate_crop() to inform the
 * CPP hardware for appropriate 2-dimensional shift margins in generating the
 * output frame/buffer.
 *
 **/
int32_t cpp_module_handle_stream_dualcam_shift_margins_event(
  mct_module_t *module,
  mct_event_t *event)
{
  int32_t rc = 0;
  uint16_t si = 0;
  cam_crop_data_t *dualcam_crop_data = NULL;
  cpp_module_stream_params_t *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;

  /* Validate module, event and stream parameters */
  if ((rc = cpp_module_util_check_event_params(
              module,
              event,
              &session_params,
              &stream_params))) {
    CPP_ERR("failed, module_event_type=%d, identity=0x%x",
      event->u.module_event.type,
      event->identity);
    return -EFAULT;
  }

  dualcam_crop_data =
    (cam_crop_data_t *)(event->u.module_event.module_event_data);
  if (!dualcam_crop_data) {
    CPP_ERR("invalid stream dual cam shift margins, failed\n");
    return -EFAULT;
  }

  CPP_DBG("Dual camera shift margins: "
    "margins:{wm=%f,hm=%f) "
    "frame id=%d, identity=0x%x",
    dualcam_crop_data->margins.widthMargins,  /* in %*/
    dualcam_crop_data->margins.heightMargins, /* in %*/
    event->u.module_event.current_frame_id,
    event->identity);

  if (stream_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    PTHREAD_MUTEX_LOCK(&(stream_params->mutex));

    stream_params->hw_params.crop_info.dualcam_crop_data = *dualcam_crop_data;

    PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
    return rc;
  }

  for (si = 0; si < CPP_MODULE_MAX_STREAMS; si++) {

    stream_params = session_params->stream_params[si];
    if (!stream_params) {
      continue;
    }

    switch (stream_params->stream_type) {
      case CAM_STREAM_TYPE_PREVIEW:
      case CAM_STREAM_TYPE_VIDEO:
      case CAM_STREAM_TYPE_SNAPSHOT:
      case CAM_STREAM_TYPE_CALLBACK: {
        PTHREAD_MUTEX_LOCK(&(stream_params->mutex));

        stream_params->hw_params.crop_info.dualcam_crop_data = *dualcam_crop_data;

        PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
        }
        break;
      case CAM_STREAM_TYPE_OFFLINE_PROC: {
        /* Offline case handled as above already, through stream params */
        }
        break;
      default: {
        CPP_LOW("Dual camera shift margins is not supported for stream %d",
          stream_params->stream_type);
        }
      }
    }

  return rc;
}  /* cpp_module_handle_stream_dualcam_shift_margins_event() */

/* cpp_module_handle_inform_lpm_event:
 *
 * Description:
 *
 **/
int32_t cpp_module_handle_inform_lpm_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t i;

  if(!module || !event) {
    CPP_DENOISE_ERR("failed, module=%p, event=%p\n", module, event);
    return -EINVAL;
  }
  cpp_module_ctrl_t* ctrl = (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_DENOISE_ERR("invalid control, failed\n");
    return -EFAULT;
  }

  mct_event_inform_lpm_t *lpm_event =
    (mct_event_inform_lpm_t *) event->u.module_event.module_event_data;
  if(!lpm_event) {
    CPP_DENOISE_ERR("invalid lpm event, failed\n");
    return -EFAULT;
  }

  /* get stream parameters based on the event identity */
  cpp_module_stream_params_t *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params || !session_params) {
    CPP_DENOISE_ERR("invalid stream params %p, session params %p failed\n",
    stream_params, session_params);
    return -EFAULT;
  }
  if(stream_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    CPP_DENOISE_HIGH("offline lpm_event->is_lpm_enabled %d \n",
      lpm_event->is_lpm_enabled);
    stream_params->hw_params.lpm_enabled = lpm_event->is_lpm_enabled;
  } else {
    CPP_DENOISE_HIGH("realitime lpm_event->is_lpm_enabled %d \n",
      lpm_event->is_lpm_enabled);
    session_params->hw_params.lpm_enabled = lpm_event->is_lpm_enabled;
    for(i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
      if(session_params->stream_params[i] &&
        session_params->stream_params[i]->stream_type !=
        CAM_STREAM_TYPE_OFFLINE_PROC) {
        session_params->stream_params[i]->hw_params.lpm_enabled = lpm_event->is_lpm_enabled;
      }
    }
  }

  return 0;
}

/* cpp_module_update_curves:
 *
 * Description:
 *
 **/
static int32_t cpp_module_update_curves(
  cpp_module_session_params_t *session_params __unused,
  cpp_module_stream_params_t  *stream_params, mct_hdr_data *hdr_data)
{

  uint32_t i ;
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  stream_params->hw_params.curves.ltm_enable = hdr_data->ltm_table.is_enable;
  stream_params->hw_params.curves.ltm_size = hdr_data->ltm_table.size;
  stream_params->hw_params.curves.gamma_enable =
    hdr_data->gamma_table.is_enable;
  stream_params->hw_params.curves.gamma_size = hdr_data->gamma_table.size;
  stream_params->hw_params.curves.gtm_enable = hdr_data->gtm_table.is_enable;
  stream_params->hw_params.curves.gtm_size = hdr_data->gtm_table.size;
  if ((hdr_data->ltm_table.size > 0) &&
    (hdr_data->ltm_table.size <= LTM_SIZE)) {
    memcpy(stream_params->hw_params.curves.ltm_master_curve,
    hdr_data->ltm_table.hw_master_curve, hdr_data->ltm_table.size);
    memcpy(stream_params->hw_params.curves.ltm_master_scale,
    hdr_data->ltm_table.hw_master_scale, hdr_data->ltm_table.size);
    memcpy(stream_params->hw_params.curves.ltm_orig_master_scale,
    hdr_data->ltm_table.hw_orig_master_scale, hdr_data->ltm_table.size);
  }

  if ((hdr_data->gamma_table.size > 0) &&
    (hdr_data->gamma_table.size <= GAMMA_SIZE)) {
    memcpy(stream_params->hw_params.curves.gamma_G,
    hdr_data->gamma_table.hw_table_g, hdr_data->gamma_table.size);
  }

  if ((hdr_data->gtm_table.size > 0) &&
    (hdr_data->gtm_table.size <= GTM_SIZE)) {
    memcpy(stream_params->hw_params.curves.gtm_yratio_base,
      hdr_data->gtm_table.base, hdr_data->gtm_table.size);
    memcpy(stream_params->hw_params.curves.gtm_yratio_slope,
      hdr_data->gtm_table.slope, hdr_data->gtm_table.size);
  }
  stream_params->hw_params.curves.hdr_msb_mode = hdr_data->msb_mode;
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
  return 0;
}

/* cpp_module_handle_post_hdr_curves:
 *
 * Description:
 *
 **/
int32_t cpp_module_handle_post_hdr_curves(mct_module_t* module,
  mct_event_t* event)
{
  int32_t i;

  if(!module || !event) {
    CPP_DENOISE_ERR("failed, module=%p, event=%p\n", module, event);
    return -EINVAL;
  }

  cpp_module_ctrl_t* ctrl = (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_DENOISE_ERR("invalid control, failed\n");
    return -EFAULT;
  }

  mct_hdr_data *hdr_data =
    (mct_hdr_data *) event->u.module_event.module_event_data;
  if(!hdr_data) {
    CPP_DENOISE_ERR("invalid hdr curves, failed\n");
    return -EFAULT;
  }

  /* get stream parameters based on the event identity */
  cpp_module_stream_params_t *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params || !session_params) {
    CPP_DENOISE_ERR("invalid stream params %p, session params %p failed\n",
    stream_params, session_params);
    return -EFAULT;
  }
  if((stream_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) &&
    (stream_params->hw_params.hdr_mode == CAM_SENSOR_HDR_ZIGZAG)) {
    cpp_module_update_curves(session_params, stream_params, hdr_data);
  } else {
    for(i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
      if (IS_CURVE_UPDATE_NEEDED(session_params->stream_params[i])) {
        cpp_module_update_curves(session_params,
          session_params->stream_params[i], hdr_data);
      }
    }
  }
  return 0;
}

/* cpp_module_handle_dis_update_event:
 *
 * Description:
 *
 **/
int32_t cpp_module_handle_dis_update_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t rc;
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_stream_params_t  *linked_stream_params = NULL;
  cpp_module_frame_hold_t     *frame_hold = FALSE;
  is_update_t                 *is_update = NULL;
  cpp_module_ctrl_t           *ctrl = NULL;
  uint32_t                     i = 0;

  if(!module || !event) {
    CPP_CROP_ERR("failed, module=%p, event=%p\n", module, event);
    return -EINVAL;
  }

  ctrl = (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_CROP_ERR("invalid cpp control, failed\n");
    return -EFAULT;
  }

  is_update =
    (is_update_t *) event->u.module_event.module_event_data;
  if(!is_update) {
    CPP_CROP_ERR("invalid is update, failed\n");
    return -EFAULT;
  }

  /* get stream parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!session_params || !stream_params) {
    CPP_CROP_ERR("failed params %p %p\n", session_params, stream_params);
    return -EFAULT;
  }

  PTHREAD_MUTEX_LOCK(&(session_params->dis_mutex));
  /* Check whether DIS is enabled, else return without storing */
  if (session_params->dis_enable == 0) {
    CPP_CROP_DBG("dis enable %d\n", session_params->dis_enable);
    PTHREAD_MUTEX_UNLOCK(&(session_params->dis_mutex));
    return 0;
  }
  PTHREAD_MUTEX_UNLOCK(&(session_params->dis_mutex));

  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));

  /* Update is_crop in stream_params */
  if ((is_update->width == 0) || (is_update->height == 0)) {
    stream_params->hw_params.crop_info.is_crop.x = 0;
    stream_params->hw_params.crop_info.is_crop.y = 0;
    stream_params->hw_params.crop_info.is_crop.dx =
      stream_params->hw_params.input_info.width;
    stream_params->hw_params.crop_info.is_crop.dy =
      stream_params->hw_params.input_info.height;
    CPP_CROP_DBG("Wrong IS crop data width %d height %d",
      is_update->width, is_update->height);
  } else {
      if (((is_update->x  + is_update->width) <=
        (int32_t)(stream_params->hw_params.input_info.width)) &&
      ((is_update->y + is_update->height) <=
        (int32_t)(stream_params->hw_params.input_info.height))) {
        CPP_CROP_HIGH("frame id %d x %d y %d dx %d dy %d"
          "width %d height %d iden 0x%x\n",
          is_update->frame_id,
          is_update->x, is_update->y, is_update->width, is_update->height,
          stream_params->hw_params.input_info.width, stream_params->hw_params.input_info.height,
          stream_params->identity);
          stream_params->hw_params.crop_info.is_crop.x = is_update->x;
          stream_params->hw_params.crop_info.is_crop.y = is_update->y;
          stream_params->hw_params.crop_info.is_crop.dx = is_update->width;
          stream_params->hw_params.crop_info.is_crop.dy = is_update->height;
      } else {
        CPP_CROP_HIGH("frame id %d x %d y %d dx %d dy %d"
         " width %d height %d iden 0x%x\n", is_update->frame_id,
          is_update->x, is_update->y, is_update->width, is_update->height,
          stream_params->hw_params.input_info.width, stream_params->hw_params.input_info.height,
          stream_params->identity);
        PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
        return 0;
      }
  }
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

  PTHREAD_MUTEX_LOCK(&(session_params->dis_mutex));
   /* Update frame id in session_params */
  session_params->dis_hold.is_valid = TRUE;
  session_params->dis_hold.dis_frame_id = is_update->frame_id;
  PTHREAD_MUTEX_UNLOCK(&(session_params->dis_mutex));

  for (i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
    /* Update is_crop in linked_stream_params */
    linked_stream_params = stream_params->linked_streams[i];
    if (linked_stream_params != NULL &&
        linked_stream_params->identity != 0) {
      PTHREAD_MUTEX_LOCK(&(linked_stream_params->mutex));
      linked_stream_params->hw_params.crop_info.is_crop.x =
        stream_params->hw_params.crop_info.is_crop.x;
      linked_stream_params->hw_params.crop_info.is_crop.y =
        stream_params->hw_params.crop_info.is_crop.y;
      linked_stream_params->hw_params.crop_info.is_crop.dx =
        stream_params->hw_params.crop_info.is_crop.dx;
      linked_stream_params->hw_params.crop_info.is_crop.dy =
        stream_params->hw_params.crop_info.is_crop.dy;
      PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
    }
  }
  PTHREAD_MUTEX_LOCK(&(session_params->dis_mutex));
  frame_hold = &session_params->frame_hold;
  /* Check whether frame is on HOLD &&
     DIS crop event is for frame on HOLD */
  if ((frame_hold->is_frame_hold == TRUE) &&
    (session_params->dis_hold.dis_frame_id >=
    frame_hold->isp_buf.buffer.sequence)) {
    CPP_CROP_DBG("send %d for processing\n", frame_hold->isp_buf.buffer.sequence);
    /* Send this frame for CPP processing */
    cpp_module_send_buf_divert_event(module, frame_hold->identity,
      &frame_hold->isp_buf);
    /* Update frame hold flag to FALSE */
    frame_hold->is_frame_hold = FALSE;
  }

  CPP_CROP_DBG("is_crop.x=%d, is_crop.y=%d, is_crop.dx=%d, is_crop.dy=%d,"
    " identity=0x%x", is_update->x, is_update->y,
    is_update->width, is_update->height, event->identity);

  PTHREAD_MUTEX_UNLOCK(&(session_params->dis_mutex));

  /* TODO: Review where DIS info needs to be sent out. */
  if (stream_params->enable_post_cpp > 0) {
    rc = cpp_module_send_event_downstream(module, event);
    if(rc < 0) {
      CPP_ERR("failed, module_event_type=%d, identity=0x%x",
        event->u.module_event.type, event->identity);
      return -EFAULT;
    }
  }
  return 0;
}

/* cpp_module_handle_stream_cfg_event:
 *
 * Description:
 *
 **/
int32_t cpp_module_handle_stream_cfg_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t rc, i = 0;
  if(!module || !event) {
    CPP_ERR("failed, module=%p, event=%p\n", module, event);
    return -EINVAL;
  }
  cpp_module_ctrl_t* ctrl = (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_ERR("invalid cpp control, failed\n");
    return -EFAULT;
  }
  sensor_out_info_t *sensor_out_info =
    (sensor_out_info_t *)(event->u.module_event.module_event_data);
  if (!sensor_out_info) {
    CPP_ERR("invalid sensor_out_info, failed\n");
    return -EFAULT;
  }
  /* get stream parameters based on the event identity */
  cpp_module_stream_params_t *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CPP_ERR("invalid stream_params, failed\n");
    return -EFAULT;
  }

  if (stream_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
    stream_params->hfr_skip_info.frame_offset =
      sensor_out_info->num_frames_skip + 1;
    stream_params->hfr_skip_info.input_fps = sensor_out_info->max_fps;
    stream_params->hw_params.sensor_dim_info.width =
      sensor_out_info->dim_output.width;
    stream_params->hw_params.sensor_dim_info.height =
      sensor_out_info->dim_output.height;
    stream_params->hw_params.camif_dim.width =
      sensor_out_info->request_crop.last_pixel -
      sensor_out_info->request_crop.first_pixel + 1;
    stream_params->hw_params.camif_dim.height =
      sensor_out_info->request_crop.last_line -
      sensor_out_info->request_crop.first_line + 1;

    CPP_INFO("frame_offset=%d, input_fps=%.2f, identity=0x%x",
      stream_params->hfr_skip_info.frame_offset,
      stream_params->hfr_skip_info.input_fps, event->identity);
    cpp_module_update_hfr_skip(stream_params);
    PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
  } else {
    for (i=0; i<CPP_MODULE_MAX_STREAMS; i++) {
      if (session_params->stream_params[i] &&
        (session_params->stream_params[i]->stream_type !=
        CAM_STREAM_TYPE_OFFLINE_PROC)) {
        PTHREAD_MUTEX_LOCK(&(session_params->stream_params[i]->mutex));
        session_params->stream_params[i]->hfr_skip_info.frame_offset =
          sensor_out_info->num_frames_skip + 1;
        session_params->stream_params[i]->hfr_skip_info.input_fps =
          sensor_out_info->max_fps;
        session_params->stream_params[i]->hw_params.sensor_dim_info.width =
          sensor_out_info->dim_output.width;
        session_params->stream_params[i]->hw_params.sensor_dim_info.height =
          sensor_out_info->dim_output.height;
        session_params->stream_params[i]->hw_params.camif_dim.width =
          sensor_out_info->request_crop.last_pixel -
          sensor_out_info->request_crop.first_pixel + 1;
        session_params->stream_params[i]->hw_params.camif_dim.height =
          sensor_out_info->request_crop.last_line -
          sensor_out_info->request_crop.first_line + 1;
        CPP_INFO("frame_offset=%d, input_fps=%.2f, identity=0x%x",
          session_params->stream_params[i]->hfr_skip_info.frame_offset,
          session_params->stream_params[i]->hfr_skip_info.input_fps,
          session_params->stream_params[i]->identity);
        cpp_module_update_hfr_skip(session_params->stream_params[i]);
        PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));
      }
    }
  }
  session_params->camif_dim.width =
    sensor_out_info->request_crop.last_pixel -
      sensor_out_info->request_crop.first_pixel + 1;
  session_params->camif_dim.height =
    sensor_out_info->request_crop.last_line -
    sensor_out_info->request_crop.first_line + 1;

  rc = cpp_module_send_event_downstream(module, event);
  if(rc < 0) {
    CPP_ERR("failed, module_event_type=%d, identity=0x%x",
      event->u.module_event.type, event->identity);
    return -EFAULT;
  }
  return 0;
}

/* cpp_module_handle_set_output_buff_event:
 *
 * Description:
 *
 **/
int32_t cpp_module_handle_set_output_buff_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t               rc;
  mct_stream_map_buf_t *img_buf;

  if(!module || !event) {
    CPP_ERR("failed, module=%p, event=%p\n", module, event);
    return -EINVAL;
  }
  cpp_module_ctrl_t* ctrl = (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_ERR("invalid cpp control, failed\n");
    return -EFAULT;
  }
  img_buf = (mct_stream_map_buf_t *)(event->u.module_event.module_event_data);
  if (!img_buf) {
    CPP_ERR("invalid img_buf, failed\n");
    return -EFAULT;
  }
  /* get stream parameters based on the event identity */
  cpp_module_stream_params_t *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CPP_ERR("invalid stream params, failed\n");
    return -EFAULT;
  }
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  stream_params->hw_params.output_buffer_info.fd =
    (unsigned long)img_buf->buf_planes[0].fd;
  stream_params->hw_params.output_buffer_info.index = img_buf->buf_index;
  stream_params->hw_params.output_buffer_info.native_buff = TRUE;
  stream_params->hw_params.output_buffer_info.offset = 0;
  stream_params->hw_params.output_buffer_info.processed_divert = 0;
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

  return 0;
}

/* cpp_module_handle_load_chromatix_event:
 *
 **/
int32_t cpp_module_handle_load_chromatix_event(mct_module_t* module,
  mct_event_t* event)
{
    int32_t rc;
    if(!module || !event) {
      CPP_ERR("failed, module=%p, event=%p\n", module, event);
      return -EINVAL;
    }
    cpp_module_ctrl_t* ctrl = (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
    if(!ctrl) {
      CPP_ERR("invalid cpp control, failed\n");
      return -EFAULT;
    }

    uint32_t  i;
    modulesChromatix_t  *chromatix_param =
      (modulesChromatix_t  *)(event->u.module_event.module_event_data);
    /* get stream parameters */
    cpp_module_session_params_t* session_params = NULL;
    cpp_module_stream_params_t*  stream_params = NULL;

    cpp_module_get_params_for_identity(ctrl, event->identity,
      &session_params, &stream_params);
    if(!session_params) {
      CPP_ERR("invalid session params, failed\n");
      return -EFAULT;
    }

    if(stream_params->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) {
      uint32_t i;

      for (i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
        if (session_params->stream_params[i] &&
          (session_params->stream_params[i]->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC)) {
          PTHREAD_MUTEX_LOCK(&(session_params->stream_params[i]->mutex));
          session_params->stream_params[i]->hw_params.module_chromatix =
            *chromatix_param;
          session_params->stream_params[i]->hw_params.interpolate_mask |=
            CPP_INTERPOLATE_ALL;
          PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));
        }
      }
    } else {
      PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
      stream_params->hw_params.module_chromatix = *chromatix_param;
      stream_params->hw_params.interpolate_mask |= CPP_INTERPOLATE_ALL;
      PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
    }
    return 0;
}


/* cpp_module_set_parm_sharpness:
 *
 **/
static int32_t cpp_module_set_parm_sharpness(cpp_module_ctrl_t *ctrl,
  uint32_t identity, int32_t value)
{
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  float                        trigger_input;
  uint32_t                          i = 0;
  cpp_hardware_params_t       *hw_params;

  if(!ctrl) {
    CPP_ASF_ERR("invalid control, failed");
    return -EFAULT;
  }
  /* get parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!session_params) {
    CPP_ASF_ERR("invalid session params, failed\n");
    return -EFAULT;
  }

  if (stream_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    hw_params = &stream_params->hw_params;
  } else {
    hw_params = &session_params->hw_params;
    hw_params->asf_mask = TRUE;
  }

  hw_params->sharpness_level = cpp_get_sharpness_ratio(value);

  CPP_ASF_HIGH("value:%d, sharpness_level:%f\n", value,
    hw_params->sharpness_level);

  cpp_module_util_update_asf_params(hw_params, hw_params->asf_mask);
  CPP_ASF_HIGH("asf_mode = %d, sharpness_level=%f",
    session_params->hw_params.asf_mode,
    session_params->hw_params.sharpness_level);

  if (stream_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    stream_params->hw_params.interpolate_mask |= CPP_INTERPOLATE_ASF;
  } else {
    /* apply this to all streams in session */
    for(i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
      if (session_params->stream_params[i] &&
        (session_params->stream_params[i]->stream_type !=
        CAM_STREAM_TYPE_OFFLINE_PROC)) {
        PTHREAD_MUTEX_LOCK(&(session_params->stream_params[i]->mutex));
        session_params->stream_params[i]->hw_params.sharpness_level =
          cpp_get_sharpness_ratio(value);

        cpp_module_util_update_asf_params(
          &session_params->stream_params[i]->hw_params,
          session_params->stream_params[i]->hw_params.asf_mask);

        session_params->stream_params[i]->hw_params.interpolate_mask |=
          CPP_INTERPOLATE_ASF;

        CPP_ASF_HIGH("[SHARPNESS] stream type %d, asf mode  %d, asf level %f , asf_mask %d",
           session_params->stream_params[i]->stream_type,
           session_params->stream_params[i]->hw_params.asf_mode,
           session_params->stream_params[i]->hw_params.sharpness_level,
           session_params->stream_params[i]->hw_params.asf_mask);
        PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));
      }
    }
  }

  return 0;
}



/* cpp_module_set_parm_sceneMode:
 *
 **/
static int32_t cpp_module_set_parm_sceneMode(cpp_module_ctrl_t *ctrl,
  uint32_t identity, int32_t value)
{
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  int32_t                      i = 0;

  if(!ctrl) {
    CPP_ASF_ERR("invalid control, failed");
    return -EFAULT;
  }
  /* get parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!session_params) {
    CPP_ASF_ERR("invalid control, failed\n");
    return -EFAULT;
  }

  if (stream_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    CPP_ASF_DBG("Update only stream params iden:0x%x\n", identity);
    PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
    if (value != CAM_SCENE_MODE_OFF) {
      stream_params->hw_params.scene_mode_on = 1;
    } else {
      stream_params->hw_params.scene_mode_on = 0;
    }
    PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
  } else {
    if (value != CAM_SCENE_MODE_OFF) {
      session_params->hw_params.scene_mode_on = 1;
    } else {
      session_params->hw_params.scene_mode_on = 0;
    }
    /* TODO: SET_PARAM will be triggered intially before any streamon etc.,
       and also when ever there is UI change */
    /* apply this to all streams in session */
    for (i=0; i<CPP_MODULE_MAX_STREAMS; i++){
      if (session_params->stream_params[i] &&
        (session_params->stream_params[i]->stream_type !=
        CAM_STREAM_TYPE_OFFLINE_PROC)){
        PTHREAD_MUTEX_LOCK(&(session_params->stream_params[i]->mutex));
        session_params->stream_params[i]->hw_params.scene_mode_on =
          session_params->hw_params.scene_mode_on;
        PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));
      }
    }
  }

  return 0;
}

/* cpp_module_set_parm_denoise:
 *
 **/
static int32_t cpp_module_set_parm_denoise(cpp_module_ctrl_t *ctrl,
  uint32_t identity, cam_denoise_param_t parm)
{
  uint32_t                     i;
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  float                        trigger_input;

  if(!ctrl) {
    CPP_DENOISE_ERR("invalid control, failed");
    return -EFAULT;
  }
  /* get parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!session_params || !stream_params) {
    CPP_DENOISE_ERR("failed session_params:%p, stream_params:%p\n",
      session_params, stream_params);
    return -EFAULT;
  }

  CPP_DENOISE_DBG("iden:0x%x denoise_enable:%d\n", identity, parm.denoise_enable);

  if (stream_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    CPP_DENOISE_LOW("Update only stream params iden:0x%x\n", identity);
    PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
    if (stream_params->hw_params.denoise_mask)
      stream_params->hw_params.denoise_enable = parm.denoise_enable;
    CPP_DENOISE_DBG("OFFLINE stream type %d, denoise_mask %d,"
         "denoise_enable %d", stream_params->stream_type,
         stream_params->hw_params.denoise_mask,
         stream_params->hw_params.denoise_enable);
    PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

    stream_params->hw_params.interpolate_mask |= CPP_INTERPOLATE_WNR;
  } else {
    session_params->hw_params.denoise_enable = parm.denoise_enable;
    /* TODO: SET_PARAM will be triggered intially before any streamon etc.,
       and also when ever there is UI change */
    /* apply this to all streams in session */
    for (i = 0; i < CPP_MODULE_MAX_STREAMS; i++){
      if (session_params->stream_params[i] &&
        (session_params->stream_params[i]->stream_type !=
        CAM_STREAM_TYPE_OFFLINE_PROC)){

        PTHREAD_MUTEX_LOCK(&(session_params->stream_params[i]->mutex));
        if (session_params->stream_params[i]->hw_params.denoise_mask == TRUE)
          session_params->stream_params[i]->hw_params.denoise_enable =
            parm.denoise_enable;
        CPP_DENOISE_DBG("stream type %d, denoise_mask %d,"
          "denoise_enable %d",
          session_params->stream_params[i]->stream_type,
          session_params->stream_params[i]->hw_params.denoise_mask,
          session_params->stream_params[i]->hw_params.denoise_enable);
        PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));

        session_params->stream_params[i]->hw_params.interpolate_mask |=
          CPP_INTERPOLATE_WNR;
      }
    }
  }
  return 0;
}

/* cpp_module_set_parm_tnr:
 *
 **/
static int32_t cpp_module_set_parm_tnr(cpp_module_ctrl_t *ctrl,
  uint32_t identity, cam_denoise_param_t parm)
{
  uint32_t                     i;
  int32_t                      rc;

  if(!ctrl) {
    CPP_TNR_ERR("invalid control, failed");
    rc = -EFAULT;
    goto end;
  }

  if (ctrl->tnr_module_func_tbl.set) {
    rc = ctrl->tnr_module_func_tbl.set(ctrl, identity, parm.denoise_enable);
  } else {
    CPP_TNR_HIGH("TNR module is not initialized");
    // There would be targets without TNR support. Do not throw an error back.
    rc = 0;
  }

end:
  return rc;
}

/* cpp_module_set_parm_edge_mode:
 *
 **/
static int32_t cpp_module_set_parm_edge_mode(cpp_module_ctrl_t *ctrl,
  uint32_t identity, cam_edge_application_t value)
{
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  chromatix_parms_type        *chromatix_ptr;
  float                        trigger_input;
  uint32_t                          i;
  int32_t                          rc;

  if(!ctrl) {
    CPP_ASF_ERR("invalid cpp control, failed");
    return -EFAULT;
  }
  /* get parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!session_params) {
    CPP_ASF_ERR("invalid session params, failed\n");
    return -EFAULT;
  }

  CPP_ASF_DBG("iden:0x%x mode:%d\n", identity, value.edge_mode);
  if (stream_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
    stream_params->hw_params.edge_mode = value.edge_mode;
    PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
  } else {
    session_params->hw_params.edge_mode = value.edge_mode;

    for (i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
      if (session_params->stream_params[i] &&
        (session_params->stream_params[i]->stream_type !=
        CAM_STREAM_TYPE_OFFLINE_PROC)) {
        PTHREAD_MUTEX_LOCK(&(session_params->stream_params[i]->mutex));
        session_params->stream_params[i]->hw_params.edge_mode =
          session_params->hw_params.edge_mode;
        PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));
      }
    }
  }
  //set sharpness : based on edge mode, desired sharpness is already set by HAL
  rc = cpp_module_set_parm_sharpness(ctrl, identity, value.sharpness);
  if(rc < 0) {
    CPP_ASF_ERR("set_parm_sharpness failed %d", rc);
    return rc;
  }
  return 0;
}

/* cpp_module_set_parm_hdr:
 *
 **/
static int32_t cpp_module_set_parm_hdr(cpp_module_ctrl_t *ctrl,
  uint32_t identity, cam_sensor_hdr_type_t parm_hdr_mode)
{
  uint32_t                     i;
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;

  if (!ctrl) {
    CPP_DENOISE_ERR("invalid cpp control, failed");
    return -EFAULT;
  }

  /* get parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if (!session_params || !stream_params) {
    CPP_DENOISE_ERR("failed session_params:%p, stream_params:%p\n",
      session_params, stream_params);
    return -EFAULT;
  }

  CPP_DENOISE_LOW("[HDR] iden:0x%x HDR mode:%d\n", identity, parm_hdr_mode);

  if (stream_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    CPP_DENOISE_LOW("Update only stream params iden:0x%x\n", identity);
    PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
    stream_params->hw_params.hdr_mode = parm_hdr_mode;
    CPP_DENOISE_LOW("[HDR] OFFLINE stream type %d, HDR mode %d",
      stream_params->stream_type,
      stream_params->hw_params.hdr_mode);
    PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

    if (parm_hdr_mode == CAM_SENSOR_HDR_ZIGZAG) {
      stream_params->hw_params.interpolate_mask |= CPP_INTERPOLATE_WNR;
      stream_params->hw_params.interpolate_mask |= CPP_INTERPOLATE_ASF;
    }
  } else {
    session_params->hw_params.hdr_mode = parm_hdr_mode;

    /* TODO: SET_PARAM will be triggered intially before any streamon etc.,
       and also when ever there is UI change */
    /* apply this to all streams in session */
    for (i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
      if (session_params->stream_params[i] &&
        (session_params->stream_params[i]->stream_type !=
        CAM_STREAM_TYPE_OFFLINE_PROC)) {

        PTHREAD_MUTEX_LOCK(&(session_params->stream_params[i]->mutex));
        session_params->stream_params[i]->hw_params.hdr_mode = parm_hdr_mode;
        CPP_DENOISE_LOW("[HDR] stream type %d, HDR mode %d",
          session_params->stream_params[i]->stream_type,
          session_params->stream_params[i]->hw_params.hdr_mode);
        PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));

        if (parm_hdr_mode == CAM_SENSOR_HDR_ZIGZAG) {
          session_params->stream_params[i]->hw_params.interpolate_mask |=
            CPP_INTERPOLATE_WNR;
          session_params->stream_params[i]->hw_params.interpolate_mask |=
            CPP_INTERPOLATE_ASF;
        }
      }
    }
  }
  return 0;
}

static int32_t cpp_module_set_parm_noise_red_mode(cpp_module_ctrl_t *ctrl,
  uint32_t identity, int32_t value)
{
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  chromatix_parms_type        *chromatix_ptr;
  cam_denoise_param_t          parm;
  float                        trigger_input;
  uint32_t                          i;
  int32_t                          rc;

  if(!ctrl) {
    CPP_DENOISE_ERR("invalid control, failed");
    return -EFAULT;
  }
  /* get parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!session_params) {
    CPP_DENOISE_ERR("invalid session params, failed\n");
    return -EFAULT;
  }
  //if the noise reduction mode is not off, set the wnr
  CPP_DENOISE_DBG("iden:0x%x CAM_NOISE_REDUCTION_MODE:%d\n",
    identity, value);
  if (value == CAM_NOISE_REDUCTION_MODE_OFF) {
    parm.denoise_enable = 0;
  } else {
    parm.denoise_enable = 1;
  }
  rc = cpp_module_set_parm_denoise(ctrl, identity, parm);
  if(rc < 0) {
      CPP_DENOISE_ERR("set_parm_denoise failed %d", rc);
      return rc;
  }
  return 0;
}

/* cpp_module_set_parm_effect:
 *
 **/
static int32_t cpp_module_set_parm_effect(cpp_module_ctrl_t *ctrl,
  uint32_t identity, int32_t value)
{
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  float                        trigger_input;
  uint32_t                          i;
  cpp_hardware_params_t       *hw_params;

  if(!ctrl) {
    CPP_ASF_ERR("invalid control, failed");
    return -EFAULT;
  }
  /* get parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!session_params) {
    CPP_ASF_ERR("invalid session params, failed\n");
    return -EFAULT;
  }

  if (stream_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    hw_params = &stream_params->hw_params;
  } else {
    hw_params = &session_params->hw_params;
    hw_params->asf_mask = TRUE;
  }

  hw_params->effect_mode = value;
  if (hw_params->asf_mask) {
    switch(value) {
    case CAM_EFFECT_MODE_EMBOSS:
      hw_params->asf_mode = CPP_PARAM_ASF_EMBOSS;
      break;
    case CAM_EFFECT_MODE_SKETCH:
      hw_params->asf_mode = CPP_PARAM_ASF_SKETCH;
      break;
    case CAM_EFFECT_MODE_NEON:
      hw_params->asf_mode = CPP_PARAM_ASF_NEON;
      break;
#ifdef BEAUTY_FACE
    case CAM_EFFECT_MODE_BEAUTY:
      hw_params->asf_mode = CPP_PARAM_ASF_BEAUTYSHOT;
      break;
#endif
    case CAM_EFFECT_MODE_OFF:
    default:
      if (hw_params->sharpness_level == 0.0f) {
        hw_params->asf_mode = CPP_PARAM_ASF_OFF;
      } else {
        hw_params->asf_mode = CPP_PARAM_ASF_DUAL_FILTER;
      }
      break;
     }
  } else {
    hw_params->asf_mode = CPP_PARAM_ASF_OFF;
    hw_params->sharpness_level = 0.0f;
  }
  CPP_ASF_HIGH("iden:0x%x effect:%d\n",
    identity, hw_params->asf_mode);

  if (stream_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    stream_params->hw_params.interpolate_mask |= CPP_INTERPOLATE_ASF;
  } else {
    /* TODO: SET_PARAM will be triggered intially before any streamon etc.,
       and also when ever there is UI change */
    /* apply this to all streams in session */
    for (i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
      if (session_params->stream_params[i] &&
        (session_params->stream_params[i]->stream_type !=
        CAM_STREAM_TYPE_OFFLINE_PROC)) {
        PTHREAD_MUTEX_LOCK(&(session_params->stream_params[i]->mutex));
        if (session_params->stream_params[i]->hw_params.asf_mask) {
          session_params->stream_params[i]->hw_params.asf_mode =
            session_params->hw_params.asf_mode;
        }
        PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));
        session_params->stream_params[i]->hw_params.interpolate_mask |=
          CPP_INTERPOLATE_ASF;
      }
    }
  }
  return 0;
}

/* cpp_module_set_parm_fps_range:
 *
 **/
static int32_t cpp_module_set_parm_fps_range(cpp_module_ctrl_t *ctrl,
  uint32_t identity, cam_fps_range_t *fps_range)
{
  if((!ctrl) || (!fps_range)){
    CPP_ERR("failed, ctrl %p, fps_range %p", ctrl, fps_range);
    return -EFAULT;
  }
  /* get parameters based on the event identity */
  cpp_module_stream_params_t *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if (!session_params) {
    CPP_ERR("invalid session params, failed\n");
    return -EFAULT;
  }

  CPP_DBG("iden:0x%x fps min:%f, max:%f video min %f max %f\n",
    identity, fps_range->min_fps, fps_range->max_fps,
    fps_range->video_min_fps, fps_range->video_max_fps);
  /* apply this to all streams where hfr skip is required */
  if (stream_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
    if(stream_params->hfr_skip_info.skip_required) {
      stream_params->hfr_skip_info.output_fps = fps_range->max_fps;
      cpp_module_update_hfr_skip(stream_params);
    }
    PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
  } else {
    uint32_t i;
    session_params->fps_range.max_fps = fps_range->max_fps;
    session_params->fps_range.min_fps = fps_range->min_fps;
    session_params->fps_range.video_max_fps = fps_range->video_max_fps;
    session_params->fps_range.video_min_fps = fps_range->video_min_fps;
    for (i=0; i<CPP_MODULE_MAX_STREAMS; i++) {
      if (session_params->stream_params[i] &&
        (session_params->stream_params[i]->stream_type !=
        CAM_STREAM_TYPE_OFFLINE_PROC)) {
        PTHREAD_MUTEX_LOCK(&(session_params->stream_params[i]->mutex));
        if(session_params->stream_params[i]->hfr_skip_info.skip_required) {
          if (session_params->stream_params[i]->stream_type ==
            CAM_STREAM_TYPE_VIDEO) {
            session_params->stream_params[i]->hfr_skip_info.output_fps =
              fps_range->video_max_fps;
            session_params->stream_params[i]->hfr_skip_info.input_fps =
              fps_range->video_max_fps;
          } else {
            session_params->stream_params[i]->hfr_skip_info.output_fps =
              fps_range->max_fps;
            session_params->stream_params[i]->hfr_skip_info.input_fps =
              fps_range->max_fps;
          }
          cpp_module_update_hfr_skip(session_params->stream_params[i]);
        }
        PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));
      }
    }
  }
  return 0;
}

/* cpp_module_set_parm_rotation:
 *
 **/
static int32_t cpp_module_set_parm_rotation(cpp_module_ctrl_t *ctrl,
  uint32_t identity, cam_rotation_info_t rotation_info)
{
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  uint32_t                     rot_identity;
  cam_pp_feature_config_t     *pp_config;

  if(!ctrl) {
    CPP_ERR("invalid cpp control, failed");
    return -EFAULT;
  }

  rot_identity = ((identity & 0xFFFF0000) | rotation_info.streamId);
  /* get parameters based on the rotation identity */
  cpp_module_get_params_for_identity(ctrl, rot_identity,
    &session_params, &stream_params);
  if(!session_params || !stream_params) {
    CPP_HIGH("session_params:%p, stream_params:%p\n",
      session_params, stream_params);
    return 0;
  }
  CPP_DBG("iden:0x%x SET UP STREAM ROTATION:%d\n",
    rot_identity, rotation_info.rotation);
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  if (stream_params->stream_info->stream_type ==
    CAM_STREAM_TYPE_OFFLINE_PROC) {
    pp_config =
      &stream_params->stream_info->reprocess_config.pp_feature_config;
  } else {
    pp_config = &stream_params->stream_info->pp_config;
  }

  if (pp_config->feature_mask & CAM_QCOM_FEATURE_ROTATION) {
    if (rotation_info.rotation == ROTATE_0) {
      stream_params->hw_params.rotation = 0;
    } else if (rotation_info.rotation == ROTATE_90) {
      stream_params->hw_params.rotation = 1;
    } else if (rotation_info.rotation == ROTATE_180) {
      stream_params->hw_params.rotation = 2;
    } else if (rotation_info.rotation == ROTATE_270) {
      stream_params->hw_params.rotation = 3;
    }
  }
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

  return 0;
}

/** cpp_module_store_frame_control: Store frame control requests
 *
 *  @per_frame_params: handle to per frame params
 *  @future_frame_id:  future frame id
 *  @type: Type of frame control parm
 *  @data: Data associated with frame control parm
 *
 *  This function is called to queue the frame control parms
 *  request to be processed later
 *
 *  Return: 0 for success and negative for failure
 **/
static int32_t cpp_module_store_frame_control(cpp_module_ctrl_t *ctrl,
  cpp_per_frame_params_t *per_frame_params, uint32_t future_frame_id,
  mct_event_type mct_type, void *payload, uint32_t identity)
{
  cpp_frame_ctrl_data_t    *frame_ctrl_data = NULL;
  int32_t                   q_idx = (future_frame_id % FRAME_CTRL_SIZE);
  void                     *parm_data = NULL;
  int32_t                   rc = -EFAULT;
  mct_event_control_parm_t *control_param = NULL;
  mct_event_module_t       *module_event_ptr = NULL;

  if (!per_frame_params || !payload) {
    CPP_PER_FRAME_ERR("failed per_frame_params:%p, payload:%p\n",
      per_frame_params, payload);
    return rc;
  }

  frame_ctrl_data = CAM_CALLOC(1, sizeof(cpp_frame_ctrl_data_t));
  if (!frame_ctrl_data) {
    CPP_PER_FRAME_ERR("frame_ctrl_data null, failed\n");
    return rc;
  }

  frame_ctrl_data->frame_id = future_frame_id;
  frame_ctrl_data->q_entry_type = CPP_FRM_CTRL_Q_APPLY;

  PTHREAD_MUTEX_LOCK(&per_frame_params->frame_ctrl_mutex[q_idx]);

  if (MCT_EVENT_MODULE_EVENT == mct_type) {
    module_event_ptr = (mct_event_module_t *)payload;
    frame_ctrl_data->mct_type = mct_type;
    frame_ctrl_data->identity = identity;
    frame_ctrl_data->u.module_event.type = module_event_ptr->type;
    frame_ctrl_data->u.module_event.current_frame_id =
      future_frame_id;
    switch (module_event_ptr->type) {
    case MCT_EVENT_MODULE_STREAM_CROP:
      CPP_CROP_LOW("MCT_EVENT_MODULE_STREAM_CROP\n");
      parm_data = CAM_CALLOC(1, sizeof(mct_bus_msg_stream_crop_t));
      if (parm_data) {
        *(mct_bus_msg_stream_crop_t *)parm_data =
          *(mct_bus_msg_stream_crop_t *)module_event_ptr->module_event_data;
      }
      break;
    case MCT_EVENT_MODULE_STATS_FOVC_MAGNIFICATION_FACTOR:
      CPP_LOW("MCT_EVENT_MODULE_STATS_FOVC_MAGNIFICATION_FACTOR\n");
      parm_data = CAM_CALLOC(1, sizeof(float));
      if (parm_data) {
        *(float *)parm_data =
          *(float *)module_event_ptr->module_event_data;
      }
      break;
    case MCT_EVENT_MODULE_DUALCAM_SHIFT_OFFSET:
      CPP_LOW("MCT_EVENT_MODULE_DUALCAM_SHIFT_OFFSET\n");
      parm_data = CAM_CALLOC(1, sizeof(cam_sac_output_info_t));
      if (parm_data) {
        *(cam_sac_output_info_t *)parm_data =
          *(cam_sac_output_info_t *)module_event_ptr->module_event_data;
      }
      break;
    case MCT_EVENT_MODULE_ISP_INFORM_LPM:
      CPP_DENOISE_LOW("MCT_EVENT_MODULE_ISP_INFORM_LPM\n");
      parm_data = CAM_CALLOC(1, sizeof(mct_event_inform_lpm_t));
      if (parm_data) {
        *(mct_event_inform_lpm_t *)parm_data =
          *(mct_event_inform_lpm_t *)module_event_ptr->module_event_data;
      }
      break;
    case MCT_EVENT_MODULE_STATS_AEC_UPDATE:
      CPP_PER_FRAME_LOW("MCT_EVENT_MODULE_STATS_AEC_UPDATE\n");
      parm_data = CAM_CALLOC(1, sizeof(stats_update_t));
      if (parm_data) {
        *(stats_update_t *)parm_data =
          *(stats_update_t *)module_event_ptr->module_event_data;
      }
      break;
    case MCT_EVENT_MODULE_POST_HDR_CURVES:
      CPP_DENOISE_LOW("MCT_EVENT_MODULE_POST_HDR_CURVES");
      parm_data = CAM_CALLOC(1, sizeof(mct_hdr_data));
      if (parm_data) {
        *(mct_hdr_data*)parm_data =
          *(mct_hdr_data *)module_event_ptr->module_event_data;
     }
     break;
    case MCT_EVENT_MODULE_ISP_OUTPUT_DIM:
       CPP_PER_FRAME_LOW("MCT_EVENT_MODULE_ISP_OUTPUT_DIM\n");
       parm_data = CAM_CALLOC(1, sizeof(mct_stream_info_t));
       if (parm_data) {
        *(mct_stream_info_t *)parm_data =
          *(mct_stream_info_t *)module_event_ptr->module_event_data;
      }
      break;
    default:
      break;
    }

    if (parm_data) {
      frame_ctrl_data->u.module_event.module_event_data = parm_data;
      mct_queue_push_tail(per_frame_params->frame_ctrl_q[q_idx],
        (void *)frame_ctrl_data);
    } else {
      CPP_PER_FRAME_DBG("module event not for CPP\n");
      free(frame_ctrl_data);
    }
  } else if (MCT_EVENT_CONTROL_CMD == mct_type) {
    CPP_PER_FRAME_DBG("CPP_FRM_CTRL_Q_APPLY enqueue apply_frmid %d"
      "q_idx:%d\n", frame_ctrl_data->frame_id,
      q_idx);

    control_param = (mct_event_control_parm_t *)payload;
    frame_ctrl_data->mct_type = mct_type;
    frame_ctrl_data->u.ctrl_param.type = control_param->type;
    frame_ctrl_data->identity = identity;
    switch (control_param->type) {
    case CAM_INTF_PARM_SHARPNESS:
      CPP_ASF_LOW("CAM_INTF_PARM_SHARPNESS\n");
      parm_data = CAM_CALLOC(1, sizeof(int32_t));
      if (parm_data) {
        *(int32_t *)parm_data = *(int32_t *)control_param->parm_data;
      }
      break;
    case CAM_INTF_META_NOISE_REDUCTION_MODE:
      CPP_DENOISE_LOW("CAM_INTF_META_NOISE_REDUCTION_MODE\n");
      parm_data = CAM_CALLOC(1, sizeof(int32_t));
      if (parm_data) {
        *(int32_t *)parm_data = *(int32_t *)control_param->parm_data;
      }
      break;
    case CAM_INTF_PARM_EFFECT:
      CPP_ASF_LOW("CAM_INTF_PARM_EFFECT\n");
      parm_data = CAM_CALLOC(1, sizeof(int32_t));
      if (parm_data) {
        *(int32_t *)parm_data = *(int32_t *)control_param->parm_data;
      }
      break;
    case CAM_INTF_META_EDGE_MODE:
      CPP_ASF_LOW("CAM_INTF_META_EDGE_MODE\n");
      parm_data = CAM_CALLOC(1, sizeof(cam_edge_application_t));
      if (parm_data) {
        *(cam_edge_application_t *)parm_data =
          *(cam_edge_application_t *)control_param->parm_data;
      }
      break;
    case CAM_INTF_PARM_WAVELET_DENOISE:
      CPP_DENOISE_LOW("CAM_INTF_PARM_WAVELET_DENOISE\n");
      parm_data = CAM_CALLOC(1, sizeof(cam_denoise_param_t));
      if (parm_data) {
        *(cam_denoise_param_t *)parm_data =
          *(cam_denoise_param_t *)control_param->parm_data;
      }
      break;
    case CAM_INTF_PARM_TEMPORAL_DENOISE:
      CPP_TNR_LOW("CAM_INTF_PARM_TEMPORAL_DENOISE\n");
      parm_data = CAM_CALLOC(1, sizeof(cam_denoise_param_t));
      if (parm_data) {
        *(cam_denoise_param_t *)parm_data =
          *(cam_denoise_param_t *)control_param->parm_data;
      }
      break;
    case CAM_INTF_PARM_FPS_RANGE:
      CPP_PER_FRAME_LOW("CAM_INTF_PARM_FPS_RANGE\n");
      parm_data = CAM_CALLOC(1, sizeof(cam_fps_range_t));
      if (parm_data) {
        *(cam_fps_range_t *)parm_data =
          *(cam_fps_range_t *)control_param->parm_data;
      }
      break;
    case CAM_INTF_PARM_ROTATION: {
      cpp_module_stream_params_t  *stream_params = NULL;
      cpp_module_session_params_t *session_params = NULL;
      uint32_t                     rot_identity;
      cam_rotation_info_t         *incoming_param = control_param->parm_data;

      CPP_PER_FRAME_LOW("CAM_INTF_PARM_ROTATION\n");
      rot_identity = ((identity & 0xFFFF0000) | incoming_param->streamId);
      /* Find the stream id is supported in current available streams */
      cpp_module_get_params_for_identity(ctrl, rot_identity,
        &session_params, &stream_params);
      if(session_params && stream_params) {
        parm_data = CAM_CALLOC(1, sizeof(cam_rotation_info_t));
        if (parm_data) {
          *(cam_rotation_info_t *)parm_data =
            *(cam_rotation_info_t *)incoming_param;
        }
      }
    }
      break;
    case CAM_INTF_META_STREAM_ID:
      CPP_PER_FRAME_DBG("CAM_INTF_META_STREAM_ID\n");
      cpp_module_session_params_t *session_params = NULL;
      cpp_module_stream_params_t  *stream_params = NULL;

      parm_data = CAM_CALLOC(1, sizeof(cam_stream_ID_t));
      if (parm_data) {
        uint32_t j;
        *(cam_stream_ID_t *)parm_data =
          *(cam_stream_ID_t *)control_param->parm_data;
        for (j = 0; j <
          ((cam_stream_ID_t *)control_param->parm_data)->num_streams; j++) {
          uint32_t request_identity = (identity & 0xFFFF0000) |
            ((cam_stream_ID_t *)control_param->parm_data)-> \
               stream_request[j].streamID;

          cpp_module_get_params_for_identity(ctrl, request_identity,
            &session_params, &stream_params);

          if(session_params != NULL) {
            session_params->verify_proc_stream_ids[q_idx] =
              *(cam_stream_ID_t *)(parm_data);
          }
          CPP_HIGH("META_STREAM_ID identity 0x%x q_idx %d frame id %d streamID:%d, \
            request_identity 0x%x",
            identity, q_idx, future_frame_id,
            ((cam_stream_ID_t *)control_param->parm_data)->stream_request[j].streamID,
            request_identity);
        }
      }
      break;
    case CAM_INTF_PARM_CDS_MODE:
      CPP_PER_FRAME_LOW("CAM_INTF_PARM_CDS_MODE\n");
      parm_data = CAM_CALLOC(1, sizeof(int32_t));
      if (parm_data) {
        *(int32_t *)parm_data =
          *(int32_t *)control_param->parm_data;
      }
      break;
    case CAM_INTF_META_IMG_DYN_FEAT:
      CPP_PER_FRAME_LOW("CAM_INTF_META_IMG_DYN_FEAT\n");
      parm_data = CAM_CALLOC(1, sizeof(cam_dyn_img_data_t));
      if (parm_data) {
        *(cam_dyn_img_data_t *)parm_data =
          *(cam_dyn_img_data_t *)control_param->parm_data;
      }
      break;
    case CAM_INTF_PARM_SENSOR_HDR:
      CPP_PER_FRAME_LOW("CAM_INTF_PARM_SENSOR_HDR\n");
      parm_data = CAM_CALLOC(1, sizeof(cam_sensor_hdr_type_t));
      if (parm_data) {
        *(cam_sensor_hdr_type_t *)parm_data =
          *(cam_sensor_hdr_type_t *)control_param->parm_data;
      }
      break;
    default:
      cpp_module_process_set_param_event(ctrl, identity, control_param->type,
        control_param->parm_data, NULL, future_frame_id);
      break;
    }

    if (parm_data) {
      frame_ctrl_data->u.ctrl_param.parm_data = parm_data;
      mct_queue_push_tail(per_frame_params->frame_ctrl_q[q_idx],
        (void *)frame_ctrl_data);
    } else {
      CPP_PER_FRAME_DBG("set param not for CPP\n");
      free(frame_ctrl_data);
    }
  } else {
      CPP_PER_FRAME_DBG("not valid mct command\n");
      free(frame_ctrl_data);
  }

  PTHREAD_MUTEX_UNLOCK(&per_frame_params->frame_ctrl_mutex[q_idx]);

  return 0;
}

/** cpp_module_process_frame_control: Handle frame control
 *  requests
 *
 *  @module:  mct module handle
 *  @per_frame_params: per frame params
 *  @frame_duration: frame duration to vary FPS
 *
 *  This function is called for every sof when frame control is enabled.
 *  It handles the frame control requests scheduled at this SOF
 *
 *  Return: 0 for success and negative for failure
 **/
int32_t cpp_module_process_frame_control(mct_module_t *module,
  cpp_per_frame_params_t *per_frame_params, uint32_t identity,
  uint32_t cur_frame_id, uint32_t process_type)
{
  int32_t                      rc = 0;
  cpp_frame_ctrl_data_t       *frame_ctrl_data = NULL;
  uint32_t                     q_idx = (cur_frame_id % FRAME_CTRL_SIZE);
  uint32_t                     report_q_idx = q_idx;
  uint32_t                     report_frame_id;
  mct_bus_msg_t                bus_msg;
  cpp_module_ctrl_t           *ctrl = NULL;
  boolean                      param_processed = FALSE, queue_compatible;
  uint32_t                     i, queue_len = 0;
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;

  if (!module || !per_frame_params) {
    CPP_PER_FRAME_ERR("failed %p %p\n", module, per_frame_params);
    return -EFAULT;
  }

  ctrl = (cpp_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_PER_FRAME_ERR("invalid cpp control, failed");
    return -EFAULT;
  }

  cpp_module_get_params_for_identity(ctrl, identity, &session_params,
     &stream_params);
  if(!stream_params) {
    CPP_PER_FRAME_ERR("invalid stream_params, failed\n");
    return -EFAULT;
  }

  CPP_PER_FRAME_DBG("process frmid:%d q_idx:%d \n", cur_frame_id, q_idx);
  pthread_mutex_lock(&per_frame_params->frame_ctrl_mutex[q_idx]);
  queue_len = per_frame_params->frame_ctrl_q[q_idx]->length;
  for (i = 0; i < queue_len; i++) {
    queue_compatible = cpp_module_pop_per_frame_entry(ctrl, per_frame_params,
      q_idx, cur_frame_id, &frame_ctrl_data, stream_params);
    if (queue_compatible == FALSE)
      break;

    if (!frame_ctrl_data)
      continue;

    if (frame_ctrl_data->q_entry_type == CPP_FRM_CTRL_Q_APPLY) {
      if ((process_type & CPP_FRAME_QUEUE_APPLY)) {
        if (MCT_EVENT_MODULE_EVENT == frame_ctrl_data->mct_type) {
          mct_event_t module_event;
          memset(&module_event, 0, sizeof(mct_event_t));
          module_event.identity = frame_ctrl_data->identity;
          module_event.direction = MCT_EVENT_DOWNSTREAM;
          module_event.type = MCT_EVENT_MODULE_EVENT;
          module_event.u.module_event = frame_ctrl_data->u.module_event;
          rc = cpp_module_process_module_event(module, &module_event);
          if (rc < 0) {
            CPP_PER_FRAME_ERR("failed: type:%d rc %d",
              frame_ctrl_data->u.module_event.type, rc);
          }
          free(frame_ctrl_data->u.module_event.module_event_data);
          free(frame_ctrl_data);
        } else if (MCT_EVENT_CONTROL_CMD == frame_ctrl_data->mct_type) {
          CPP_PER_FRAME_DBG("CPP_FRM_CTRL_Q_APPLY dequeue type = %d"
            "apply_frmid %d\n",
            frame_ctrl_data->u.ctrl_param.type, cur_frame_id);
          param_processed = FALSE;
          rc = cpp_module_process_set_param_event(ctrl, identity,
            frame_ctrl_data->u.ctrl_param.type,
            frame_ctrl_data->u.ctrl_param.parm_data, &param_processed,
            cur_frame_id);
          if ((rc == 0) && (param_processed == TRUE) &&
            (process_type & CPP_FRAME_QUEUE_PREPARE)) {
            /* The parameter that got just applied needs to be reported depending
            on the max_pipeline delay and its own apply delay. Reuse the same
            event for reporting */
            report_frame_id = cur_frame_id + per_frame_params->max_report_delay;
            report_q_idx = report_frame_id % FRAME_CTRL_SIZE;
            if (report_frame_id > per_frame_params->cur_frame_id){
              pthread_mutex_unlock(&per_frame_params->frame_ctrl_mutex[q_idx]);

              pthread_mutex_lock(&per_frame_params->frame_ctrl_mutex[report_q_idx]);

              cpp_module_add_report_entry(per_frame_params,
                frame_ctrl_data, report_frame_id, identity);

              pthread_mutex_unlock(&per_frame_params->frame_ctrl_mutex[report_q_idx]);

              pthread_mutex_lock(&per_frame_params->frame_ctrl_mutex[q_idx]);
            } else if (report_frame_id == per_frame_params->cur_frame_id) {
              rc = cpp_module_util_post_metadata_to_bus(module,
                frame_ctrl_data->u.ctrl_param.type,
                frame_ctrl_data->u.ctrl_param.parm_data, identity);

              /* Delete it after reporting */
              free(frame_ctrl_data->u.ctrl_param.parm_data);
              free(frame_ctrl_data);
            } else {
              /* Delete it if not reporting */
              free(frame_ctrl_data->u.ctrl_param.parm_data);
              free(frame_ctrl_data);
            }
          } else {
            /* Delete it if not reporting */
            free(frame_ctrl_data->u.ctrl_param.parm_data);
            free(frame_ctrl_data);
          }
        }
      } else {
        /* Enqueue the entry back to the queue */
        mct_queue_push_tail(per_frame_params->frame_ctrl_q[q_idx],
          (void *)frame_ctrl_data);
      }
    } else if (frame_ctrl_data->q_entry_type == CPP_FRM_CTRL_Q_REPORT) {
      if ((process_type & CPP_FRAME_QUEUE_REPORT)) {
        /* Report the entry first */
        CPP_META_DBG("CPP_FRM_CTRL_Q_REPORT dequeue type = %d reprt_frmid %d"
          " rpt_q_idx:%d\n",
          frame_ctrl_data->u.ctrl_param.type, frame_ctrl_data->frame_id, q_idx);
        rc = cpp_module_util_post_metadata_to_bus(module,
          frame_ctrl_data->u.ctrl_param.type,
          frame_ctrl_data->u.ctrl_param.parm_data, identity);

        /* Delete it after reporting */
        free(frame_ctrl_data->u.ctrl_param.parm_data);
        free(frame_ctrl_data);
      } else {
        /* Enqueue the entry back to the queue */
        mct_queue_push_tail(per_frame_params->frame_ctrl_q[q_idx],
          (void *)frame_ctrl_data);
      }
    } else {
      CPP_PER_FRAME_ERR("failed: type:%d rc %d", frame_ctrl_data->u.ctrl_param.type, rc);
      /* Entry is invalid */
      if (MCT_EVENT_MODULE_EVENT == frame_ctrl_data->mct_type)
        free(frame_ctrl_data->u.module_event.module_event_data);
      else
        free(frame_ctrl_data->u.ctrl_param.parm_data);
      free(frame_ctrl_data);
      continue;
    }
  }
  pthread_mutex_unlock(&per_frame_params->frame_ctrl_mutex[q_idx]);

  return TRUE;
}

int32_t cpp_module_handle_sof_set_parm_event(mct_module_t* module,
  mct_event_t* event)
{
  CPP_PER_FRAME_LOW("E");
  mct_event_super_control_parm_t *param = NULL;
  uint32_t index = 0;
  int32_t rc = 0;
  mct_event_t sub_event;
  cpp_module_ctrl_t *ctrl = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_per_frame_params_t *per_frame_params = NULL;

  MCT_PROF_LOG_BEG(PROF_CPP_SP);
  if (!module || !event) {
   CPP_PER_FRAME_ERR("failed, module=%p, event=%p", module, event);
   return -EINVAL;
 }

  param =
    (mct_event_super_control_parm_t *)event->u.ctrl_event.control_event_data;
  if (!param) {
    CPP_PER_FRAME_ERR("failed: param %p", param);
    return FALSE;
  }

  ctrl = (cpp_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_PER_FRAME_ERR("invalid control, failed");
    return -EFAULT;
  }

  /* get parameters based on the session id */
  cpp_module_get_params_for_session_id(ctrl,
    CPP_GET_SESSION_ID(event->identity), &session_params);
  if(!session_params) {
    CPP_PER_FRAME_ERR("invalid session_params, failed\n");
    return -EFAULT;
  }

  /* Update current frame id */
  per_frame_params = &session_params->per_frame_params;
  PTHREAD_MUTEX_LOCK(&per_frame_params->mutex);
  per_frame_params->cur_frame_id = event->u.ctrl_event.current_frame_id;
  PTHREAD_MUTEX_UNLOCK(&per_frame_params->mutex);

  CPP_PER_FRAME_HIGH("frmid=%d, identity=0x%x",
    per_frame_params->cur_frame_id, event->identity);

  sub_event.direction = event->direction;
  sub_event.identity = event->identity;
  sub_event.timestamp = event->timestamp;
  sub_event.type = MCT_EVENT_CONTROL_CMD;
  sub_event.u.ctrl_event.type = MCT_EVENT_CONTROL_SET_PARM;
  sub_event.u.ctrl_event.current_frame_id =
    event->u.ctrl_event.current_frame_id;
  /* Handle all set params */
  for (index = 0; index < param->num_of_parm_events; index++) {
    sub_event.u.ctrl_event.control_event_data = &param->parm_events[index];
    rc = cpp_module_handle_set_parm_event(module, &sub_event);
  }

  MCT_PROF_LOG_END();
  rc = cpp_module_send_event_downstream(module, event);
  if(rc < 0) {
    CPP_ERR("failed, ctrl_event_type=%d, identity=0x%x",
      event->u.ctrl_event.type, event->identity);
    return -EFAULT;
  }

  CPP_PER_FRAME_LOW("X");
  return 0;

}

/* cpp_module_set_parm_dis:
 *
 **/
static int32_t cpp_module_set_parm_dis(cpp_module_ctrl_t *ctrl,
  uint32_t identity, int32_t dis_enable)
{
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  if(!ctrl) {
    CPP_CROP_ERR("invalid control, failed");
    return -EFAULT;
  }
  /* get parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, identity, &session_params,
     &stream_params);
  if(!session_params) {
    CPP_CROP_ERR("invalid session params, failed\n");
    return -EFAULT;
  }

  PTHREAD_MUTEX_LOCK(&(session_params->dis_mutex));
  /* Update dis_enable flag in session_params */
  session_params->dis_enable = dis_enable;
  CPP_CROP_DBG("dis_enable %d\n", dis_enable);
  if (dis_enable == 0) {
    /* Invalidate DIS hold flag */
    session_params->dis_hold.is_valid = FALSE;
  }
  PTHREAD_MUTEX_UNLOCK(&(session_params->dis_mutex));
  return 0;
}

/* cpp_module_set_parm_flip:
 *    @ctrl: cpp module control struct
 *    @indentity: current indentity
 *    @flip_mask: new flip mode
 *
 *    Set the flip mode sent form application
 *
 *    Returns 0 on succes or EFAULT if some of the parameters
 *      is missing.
 **/
static int32_t cpp_module_set_parm_flip(cpp_module_ctrl_t *ctrl,
  uint32_t identity, int32_t flip_mask)
{
  if(!ctrl) {
    CPP_ERR("invalid cpp control, failed");
    return -EFAULT;
  }
  /* get parameters based on the event identity */
  cpp_module_stream_params_t *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CPP_ERR("invalid stream params, failed\n");
    return -EFAULT;
  }

  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  stream_params->hw_params.mirror = flip_mask;
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

  return 0;
}

/* cpp_module_process_set_parm_event:
 *
 * Description:
 *   Handle the set_parm event.
 **/
int32_t cpp_module_process_set_param_event(cpp_module_ctrl_t *ctrl,
  uint32_t identity, cam_intf_parm_type_t type, void *parm_data,
  boolean *param_processed, uint32_t current_frame_id)
{
  int32_t  rc = 0;
  uint32_t q_idx = current_frame_id % FRAME_CTRL_SIZE;

  if (!ctrl || !parm_data) {
    CPP_ERR("failed: ctrl %p parm_data %p", ctrl, parm_data);
    return -EINVAL;
  }

  if (param_processed) {
    *param_processed = FALSE;
  }

  switch (type) {
  case CAM_INTF_PARM_SHARPNESS: {
    int32_t value = *(int32_t*)(parm_data);
    CPP_ASF_DBG("CAM_INTF_PARM_SHARPNESS, value=%d, identity=0x%x",
      value, identity);
    rc = cpp_module_set_parm_sharpness(ctrl, identity, value);
    if(rc < 0) {
      CPP_ASF_ERR("set_parm_sharpness %d", rc);
      return rc;
    }
    if (param_processed) {
      *param_processed = TRUE;
    }
    break;
  }
  case CAM_INTF_PARM_BESTSHOT_MODE: {
    int32_t value = *(int32_t*)(parm_data);
    CPP_ASF_DBG("CAM_INTF_PARM_BESTSHOT_MODE, value=%d, identity=0x%x",
      value, identity);
    rc = cpp_module_set_parm_sceneMode(ctrl, identity, value);
    if(rc < 0) {
      CPP_ASF_ERR("set_parm_sceneMode failed %d", rc);
      return rc;
    }
    break;
  }
  case CAM_INTF_PARM_EFFECT: {
    int32_t value = *(int32_t*)(parm_data);
    CPP_ASF_DBG("CAM_INTF_PARM_EFFECT, value=%d, identity=0x%x",
      value, identity);
    rc = cpp_module_set_parm_effect(ctrl, identity, value);
    if(rc < 0) {
      CPP_ASF_ERR("set_parm_effect failed %d", rc);
      return rc;
    }
    break;
  }
  case CAM_INTF_META_EDGE_MODE: {
    cam_edge_application_t value = *(cam_edge_application_t*)(parm_data);
    CPP_ASF_DBG("CAM_INTF_META_EDGE_MODE, edge_mode=%d, sharpness:%d, identity=0x%x",
      value.edge_mode, value.sharpness, identity);
    rc = cpp_module_set_parm_edge_mode(ctrl, identity, value);
    if(rc < 0) {
      CPP_DENOISE_ERR("set_parm_edge_mode failed %d", rc);
      return rc;
    }
    break;
  }
  case CAM_INTF_PARM_HAL_VERSION: {
    int32_t value = *(int32_t*)(parm_data);
    cpp_module_session_params_t* session_params = NULL;
    cpp_module_stream_params_t* stream_params = NULL;
    CPP_LOW("CAM_INTF_PARM_HAL_VERSION, value=%d, identity=0x%x",
      value, identity);
    /* get parameters based on the event identity */
    rc = cpp_module_get_params_for_identity(ctrl, identity,
      &session_params, &stream_params);
    if(!session_params) {
      CPP_ERR("invalid session params, failed\n");
      return -EFAULT;
    }
    session_params->hal_version = value;
    break;
  }
  case CAM_INTF_META_NOISE_REDUCTION_MODE: {
    int32_t value = *(int32_t*)(parm_data);
    CPP_DENOISE_DBG("CAM_INTF_META_NOISE_REDUCTION_MODE, value=%d,"
      "identity=0x%x", value, identity);
    rc = cpp_module_set_parm_noise_red_mode(ctrl, identity, value);
    if(rc < 0) {
      CPP_DENOISE_ERR("set_parm_noise_red_mode failed %d", rc);
      return rc;
    }
    break;
  }
  case CAM_INTF_PARM_WAVELET_DENOISE: {
    cam_denoise_param_t parm = *(cam_denoise_param_t *)(parm_data);
    CPP_DENOISE_DBG("CAM_INTF_PARM_WAVELET_DENOISE, enable=%d, identity=0x%x",
      parm.denoise_enable, identity);
    rc = cpp_module_set_parm_denoise(ctrl, identity, parm);
    if(rc < 0) {
      CPP_DENOISE_ERR("set_parm_denoise failed %d", rc);
      return rc;
    }
    if (param_processed) {
      *param_processed = TRUE;
    }
    break;
  }
  case CAM_INTF_PARM_TEMPORAL_DENOISE: {
    cam_denoise_param_t parm = *(cam_denoise_param_t *)(parm_data);
    CPP_TNR_DBG("CAM_INTF_PARM_TEMPORAL_DENOISE, enable=%d, identity=0x%x",
      parm.denoise_enable, identity);
    rc = cpp_module_set_parm_tnr(ctrl, identity, parm);
    if(rc < 0) {
      CPP_TNR_ERR("set_parm_tnr failed %d", rc);
      return rc;
    }
    if (param_processed) {
      *param_processed = TRUE;
    }
    break;
  }
  case CAM_INTF_PARM_FPS_RANGE: {
    cam_fps_range_t *fps_range = (cam_fps_range_t *)(parm_data);
    CPP_DBG("CAM_INTF_PARM_FPS_RANGE,, max_fps=%.2f, identity=0x%x",
      fps_range->max_fps, identity);
    rc = cpp_module_set_parm_fps_range(ctrl, identity, fps_range);
    if(rc < 0) {
      CPP_ERR("failed rc %d", rc);
      return rc;
    }
    if (param_processed) {
      *param_processed = TRUE;
    }
    break;
  }
  case CAM_INTF_PARM_ROTATION: {
    cam_rotation_info_t rotation = *(cam_rotation_info_t *)(parm_data);
    CPP_PER_FRAME_HIGH("CAM_INTF_PARM_ROTATION,rotation %d", rotation.rotation);
    rc = cpp_module_set_parm_rotation(ctrl, identity, rotation);
    if(rc < 0) {
      CPP_PER_FRAME_ERR("set_parm_rotation failed %d", rc);
      return rc;
    }
    if (param_processed) {
      *param_processed = TRUE;
    }
    break;
  }
  case CAM_INTF_META_STREAM_ID: {
    cpp_module_session_params_t *session_params = NULL;
    cpp_module_stream_params_t  *stream_params = NULL;
    CPP_PER_FRAME_DBG("CAM_INTF_META_STREAM_ID\n");
    /* get parameters based on the event identity */
    rc = cpp_module_get_params_for_identity(ctrl, identity, &session_params,
      &stream_params);
    if(!session_params) {
      CPP_PER_FRAME_ERR("null session params, failed\n");
      return -EFAULT;
    }
    session_params->valid_stream_ids[q_idx] = *(cam_stream_ID_t *)(parm_data);
    if(rc < 0) {
      CPP_ERR("failed %d", rc);
      return rc;
    }
    break;
  }
  case CAM_INTF_META_STREAM_INFO : {
    /* This event is sent by HAL to indicate a configuration change. There
       are chances that the session stream will not be removed while all
       other streams might be removed and newly created. So this event can
       be used to indicate a reset of session params including the per
       frame control queue */
    cpp_module_session_params_t *session_params = NULL;
    cpp_module_stream_params_t  *stream_params = NULL;
    uint32_t i;
    CPP_PER_FRAME_DBG("CAM_INTF_META_STREAM_INFO\n");
    /* get parameters based on the event identity */
    rc = cpp_module_get_params_for_identity(ctrl, identity, &session_params,
      &stream_params);
    if(!session_params) {
      CPP_ERR("invalid session params, failed\n");
      return -EFAULT;
    }
    memset(&session_params->valid_stream_ids[0], 0,
      sizeof(session_params->valid_stream_ids));
    memset(&session_params->verify_proc_stream_ids[0], 0,
     sizeof(session_params->verify_proc_stream_ids));
    cam_stream_size_info_t *meta_stream_info =
      (cam_stream_size_info_t *)(parm_data);
    ctrl->rtb_status = 0;
    ctrl->sat_status = 0;
    for (i = 0; i < meta_stream_info->num_streams; i++) {
       if (meta_stream_info->postprocess_mask[i])
       {
         meta_stream_info->topology |= 1 << 16; // set securemode bit for CPP
         CPP_HIGH("Set CPP in secure mode %x\n", meta_stream_info->topology);
       }
       CPP_HIGH("META_STREAM_INFO type = %d w x h : %d x %d pp_mask = 0x%x",
        meta_stream_info->type[i],
        meta_stream_info->stream_sizes[i].width,
        meta_stream_info->stream_sizes[i].height,
        meta_stream_info->postprocess_mask[i]);
        if (meta_stream_info->postprocess_mask[i] & CAM_QTI_FEATURE_RTB) {
          ctrl->rtb_status =
            ((meta_stream_info->postprocess_mask[i] & CAM_QTI_FEATURE_RTB) ? 1 : 0);
        }
        if (meta_stream_info->postprocess_mask[i] & CAM_QTI_FEATURE_SAT) {
          ctrl->sat_status =
            ((meta_stream_info->postprocess_mask[i] & CAM_QTI_FEATURE_SAT) ? 1 : 0);
        }
        CPP_HIGH("rtb_status %d sat status %d",ctrl->rtb_status, ctrl->sat_status);
    }
    break;
  }
  case CAM_INTF_PARM_DIS_ENABLE: {
    int32_t dis_enable = *(int32_t *)(parm_data);
    CPP_PER_FRAME_HIGH("CAM_INTF_PARM_DIS_ENABLE, enable=%d, identity=0x%x",
      dis_enable, identity);
    rc = cpp_module_set_parm_dis(ctrl, identity, dis_enable);
    if(rc < 0) {
      CPP_ERR("failed %d", rc);
      return rc;
    }
    break;
  }
  case CAM_INTF_PARM_SET_PP_COMMAND: {
    uint32_t  i;
    tune_cmd_t *ez_tune_cmd = (tune_cmd_t *)parm_data;

    /* get stream parameters */
    cpp_module_session_params_t* session_params = NULL;
    cpp_module_stream_params_t*  stream_params = NULL;

    cpp_module_get_params_for_identity(ctrl, identity,
      &session_params, &stream_params);
    if(!session_params) {
      CPP_PER_FRAME_ERR("null session params, failed\n");
      return -EFAULT;
    }
    CPP_PER_FRAME_LOW("Received CAM_INTF_PARM_SET_PP_COMMAND event\n"
      "module type %d command %d value %d",
      ez_tune_cmd->module, ez_tune_cmd->type,
      ez_tune_cmd->value);
    switch (ez_tune_cmd->module) {
    case PP_MODULE_WNR: {
      if(ez_tune_cmd->type == SET_ENABLE) {
        session_params->hw_params.ez_tune_wnr_enable = ez_tune_cmd->value;
        session_params->diag_params.control_wnr.enable = ez_tune_cmd->value;
        /* apply this to all streams in session */
        for(i=0; i<CPP_MODULE_MAX_STREAMS; i++) {
          if(session_params->stream_params[i]) {
            session_params->stream_params[i]->hw_params.ez_tune_wnr_enable =
              ez_tune_cmd->value;
          }
        }
      }
      else if(ez_tune_cmd->type == SET_CONTROLENABLE) {
        session_params->diag_params.control_wnr.cntrlenable =
          ez_tune_cmd->value;
        /* apply this to all streams in session */
        for(i=0; i<CPP_MODULE_MAX_STREAMS; i++) {
          if(session_params->stream_params[i]) {
            session_params->stream_params[i]->hw_params.denoise_lock =
              ez_tune_cmd->value;
          }
        }
      }
      break;
    }
    case PP_MODULE_ASF: {
      if(ez_tune_cmd->type == SET_ENABLE) {
        session_params->hw_params.ez_tune_asf_enable = ez_tune_cmd->value;
        session_params->diag_params.control_asf7x7.enable =
          ez_tune_cmd->value;
        /* apply this to all streams in session */
        for(i=0; i<CPP_MODULE_MAX_STREAMS; i++) {
          if(session_params->stream_params[i]) {
            session_params->stream_params[i]->hw_params.ez_tune_asf_enable =
              ez_tune_cmd->value;
          }
        }
      }
      else if(ez_tune_cmd->type == SET_CONTROLENABLE) {
        session_params->diag_params.control_asf7x7.cntrlenable =
          ez_tune_cmd->value;
        /* apply this to all streams in session */
        for(i=0; i<CPP_MODULE_MAX_STREAMS; i++) {
          if(session_params->stream_params[i]) {
            session_params->stream_params[i]->hw_params.asf_lock =
              ez_tune_cmd->value;
          }
        }
      }
      break;
    }
    case PP_MODULE_ALL: {
      if(ez_tune_cmd->type == SET_STATUS) {
        session_params->hw_params.diagnostic_enable =
          ez_tune_cmd->value;
        /* apply this to all streams in session */
        for(i=0; i<CPP_MODULE_MAX_STREAMS; i++) {
          if(session_params->stream_params[i]) {
            session_params->stream_params[i]->hw_params.diagnostic_enable =
              ez_tune_cmd->value;
          }
        }
      }
      break;
    }
    default:
      break;
    }
    break;
  }
  case CAM_INTF_PARM_CDS_MODE: {
    rc = cpp_module_set_parm_dsdn(ctrl, identity, *(int32_t*)(parm_data));
    if(rc < 0) {
      CPP_PER_FRAME_ERR("CAM_INTF_PARM_CDS_MODE failed");
      return rc;
    }
    break;
  }
  case CAM_INTF_META_IMG_DYN_FEAT: {
    cam_dyn_img_data_t *dyn_img_data = (cam_dyn_img_data_t *)parm_data;
    rc = cpp_module_set_parm_img_dyn_feat(ctrl, identity,
      ((cam_dyn_img_data_t *)parm_data)->dyn_feature_mask);
    if(rc < 0) {
      CPP_PER_FRAME_ERR("CAM_INTF_META_IMG_DYN_FEAT failed");
      return rc;
    }
    break;
  }
  case CAM_INTF_PARM_SENSOR_HDR: {
    cam_sensor_hdr_type_t hdr_type = *(cam_sensor_hdr_type_t *)(parm_data);
    CPP_PER_FRAME_LOW("CAM_INTF_PARM_SENSOR_HDR %d", hdr_type);
    rc = cpp_module_set_parm_hdr(ctrl, identity, hdr_type);
    if (rc < 0) {
      CPP_PER_FRAME_LOW("CAM_INTF_PARM_SENSOR_HDR failed");
      return rc;
    }
    break;
  }
  default:
    break;
  }
  return rc;
}

/* cpp_module_handle_module_event:
 *
 * Description:
 *   Handle the module event.
 **/
int32_t cpp_module_handle_module_event(mct_module_t* module,
  mct_event_t *event)
{
  int32_t                      rc = 0;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_per_frame_params_t      *per_frame_params = NULL;
  uint32_t                     cur_frame_id = 0;
  uint32_t                     apply_frame_id = 0;
  boolean                      param_processed = FALSE;

  if(!module || !event) {
    CPP_PER_FRAME_ERR("failed, module=%p, event=%p", module, event);
    return -EINVAL;
  }
  cpp_module_ctrl_t *ctrl = (cpp_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_PER_FRAME_ERR("null control, failed");
    return -EFAULT;
  }

  mct_event_module_t *module_event =
    (mct_event_module_t *)&event->u.module_event;
  if(!event->u.module_event.module_event_data) {
    CPP_PER_FRAME_ERR("invalid event data, failed");
    return -EFAULT;
  }
  /* get parameters based on the identity */
  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!session_params || !stream_params) {
    CPP_PER_FRAME_ERR("failed %p %p\n", session_params, stream_params);
    return -EFAULT;
  }

  per_frame_params = &session_params->per_frame_params;

  if (stream_params->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) {
    cur_frame_id = event->u.module_event.current_frame_id;
    apply_frame_id = cur_frame_id + per_frame_params->max_apply_delay -
      per_frame_params->cpp_delay;
  } else {
    apply_frame_id = cur_frame_id = event->u.module_event.current_frame_id;
  }

  if (session_params->is_stream_on == FALSE) {
    /* Apply set param immediately if  session is not streaming yet */
    CPP_PER_FRAME_DBG("Apply only apply_frame_id=%d, type=%d",
      apply_frame_id, module_event->type);
    rc = cpp_module_process_module_event(module, event);
  } else {
    if((cur_frame_id <=
      (per_frame_params->max_apply_delay - per_frame_params->cpp_delay)) ||
      (stream_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC)) {
      /* Apply set param immediately if offline/special case for frameid less than max delay */
      CPP_PER_FRAME_DBG("Apply apply_frame_id=%d, type=%d",
        apply_frame_id, module_event->type);
      rc = cpp_module_process_module_event(module, event);
    }

    /*
     * Store or apply set param based on per frame control logic. They will be
     * applied subsequently by the if {} else {} blocks (immediate above), i.e.,
     * handled through cpp_module_process_module_event().
     */
    if((cur_frame_id <= apply_frame_id) &&
      (stream_params->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC)){
      CPP_PER_FRAME_DBG("Store. current frame id %d,"
        " apply_frame_id=%d, delay = %d type %d ",
        cur_frame_id, apply_frame_id,
        (per_frame_params->max_apply_delay - per_frame_params->cpp_delay), module_event->type);
      rc = cpp_module_store_frame_control(ctrl, per_frame_params, apply_frame_id,
        MCT_EVENT_MODULE_EVENT, module_event, event->identity);
    } else if (cur_frame_id > apply_frame_id){
      CPP_PER_FRAME_ERR("frame to apply is missed cur %d apply %d\n",
        cur_frame_id, apply_frame_id);
      return -EFAULT;
    }
  }


  if (event->direction == MCT_EVENT_DOWNSTREAM) {
    rc = cpp_module_send_event_downstream(module, event);
  } else {
    rc = cpp_module_send_event_upstream(module, event);
  }
  if (rc < 0) {
    CPP_PER_FRAME_ERR("failed rc %d", rc);
  }
  return rc;
}

static void cpp_module_handle_sticky_set_parm_event(
  cpp_module_session_params_t *session_params,
  mct_event_control_parm_t *ctrl_parm,
  uint32_t apply_frame_id,
  mct_event_t* event) {
  if (ctrl_parm->parm_data) {
    switch (ctrl_parm->type) {
      case CAM_INTF_PARM_EFFECT:
        session_params->sticky_set_parm.spl_effect.value =
          *(int32_t*)(ctrl_parm->parm_data);
        session_params->sticky_set_parm.spl_effect.is_valid = true;
        session_params->sticky_set_parm.spl_effect.apply_frame_id =
          apply_frame_id;
        session_params->sticky_set_parm.spl_effect.identity = event->identity;
        CPP_ASF_DBG("CAM_INTF_PARM_EFFECT frameid %d iden 0x%x value %d",
          apply_frame_id, event->identity,
            session_params->sticky_set_parm.spl_effect.value);
      break;
      case CAM_INTF_META_EDGE_MODE:
        session_params->sticky_set_parm.edge_mode.value =
          *(cam_edge_application_t*)(ctrl_parm->parm_data);
        session_params->sticky_set_parm.edge_mode.is_valid = true;
        session_params->sticky_set_parm.edge_mode.apply_frame_id =
          apply_frame_id;
        session_params->sticky_set_parm.edge_mode.identity =
          event->identity;
      break;
      case CAM_INTF_META_NOISE_REDUCTION_MODE:
        session_params->sticky_set_parm.reduction_mode.value =
          *(int32_t*)(ctrl_parm->parm_data);
        session_params->sticky_set_parm.reduction_mode.is_valid = true;
        session_params->sticky_set_parm.reduction_mode.apply_frame_id =
          apply_frame_id;
        session_params->sticky_set_parm.reduction_mode.identity =
          event->identity;
      break;
      case CAM_INTF_PARM_CDS_MODE:
        session_params->sticky_set_parm.cds_mode.value =
          *(int32_t*)(ctrl_parm->parm_data);
        session_params->sticky_set_parm.cds_mode.is_valid = true;
        session_params->sticky_set_parm.cds_mode.apply_frame_id =
          apply_frame_id;
        session_params->sticky_set_parm.cds_mode.identity =
          event->identity;
      break;
      case CAM_INTF_META_IMG_DYN_FEAT:
        session_params->sticky_set_parm.dyn_feast.value =
          *(cam_dyn_img_data_t*)(ctrl_parm->parm_data);
        session_params->sticky_set_parm.dyn_feast.is_valid = true;
        session_params->sticky_set_parm.dyn_feast.apply_frame_id =
          apply_frame_id;
        session_params->sticky_set_parm.dyn_feast.identity =
          event->identity;
      break;
      default:
        break;
    }
  }
  return;
}

/* cpp_module_handle_set_parm_event:
 *
 * Description:
 *   Handle the set_parm event.
 **/
int32_t cpp_module_handle_set_parm_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t                      rc = 0;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_per_frame_params_t      *per_frame_params = NULL;
  uint32_t                     cur_frame_id = 0;
  uint32_t                     apply_frame_id = 0;
  boolean                      param_processed = FALSE;

  if(!module || !event) {
    CPP_PER_FRAME_ERR("failed, module=%p, event=%p", module, event);
    return -EINVAL;
  }
  mct_event_control_parm_t *ctrl_parm =
    (mct_event_control_parm_t *) event->u.ctrl_event.control_event_data;
  if(!ctrl_parm) {
    CPP_PER_FRAME_ERR("invalid ctrl_parm, failed");
    return -EFAULT;
  }
  cpp_module_ctrl_t *ctrl = (cpp_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_PER_FRAME_ERR("invalid cpp control, failed");
    return -EFAULT;
  }

  /* get parameters based on the identity */
  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!session_params || !stream_params) {
    CPP_PER_FRAME_ERR("failed %p %p\n", session_params, stream_params);
    return -EFAULT;
  }

  per_frame_params = &session_params->per_frame_params;
  cur_frame_id = event->u.ctrl_event.current_frame_id;
  apply_frame_id = cur_frame_id;

  if (stream_params->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) {
    apply_frame_id = cur_frame_id + per_frame_params->max_apply_delay -
      per_frame_params->cpp_delay;
  }

  if (session_params->is_stream_on == FALSE) {
    /* Apply set param immediately if current stream is offline stream
       or session is not streaming yet */
    CPP_PER_FRAME_DBG("Apply only apply_frame_id=%d, type=%d",
      apply_frame_id, ctrl_parm->type);
    rc = cpp_module_process_set_param_event(ctrl, event->identity,
      ctrl_parm->type, ctrl_parm->parm_data, &param_processed,
      cur_frame_id);
  } else if ((cur_frame_id == apply_frame_id) &&
    (stream_params->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC)) {
    CPP_PER_FRAME_DBG("Apply & Report apply_frame_id=%d, type=%d",
      apply_frame_id, ctrl_parm->type);
    /* Apply set param immediately. The additional logic of apply and store
       is needed only for module_events. This is because set_param events
       like META_STREAM_ID cannot be applied and stored. They are events
       directed to specific frame_id. This special handling is needed for
       module events like crop that handle Zoom. */
    rc = cpp_module_process_set_param_event(ctrl, event->identity,
      ctrl_parm->type, ctrl_parm->parm_data, &param_processed,
      cur_frame_id);
    if ((rc == 0) && (param_processed == TRUE)) {
      /* Need to report parameter */
      rc = cpp_module_util_post_metadata_to_bus(module, ctrl_parm->type,
        ctrl_parm->parm_data, event->identity);
    }
  } else {
    if (cur_frame_id > apply_frame_id) {
      CPP_PER_FRAME_ERR("frame to apply is missed cur %d apply %d\n",
        cur_frame_id, apply_frame_id);
      return -EFAULT;
    }

    CPP_PER_FRAME_DBG("sticky_set_parm apply_frame_id=%d, type=%d",
      apply_frame_id, ctrl_parm->type);
    cpp_module_handle_sticky_set_parm_event(session_params,
      ctrl_parm, apply_frame_id, event);

    /* Store or apply set param based on per frame control logic */
    rc = cpp_module_store_frame_control(ctrl, per_frame_params, apply_frame_id,
      MCT_EVENT_CONTROL_CMD, ctrl_parm, event->identity);
  }

  /* Since handle set param event is being reused from sof_set_param_event
     this function does not forward the event to downstream module.*/
#if 0
  rc = cpp_module_send_event_downstream(module, event);
  if(rc < 0) {
    CPP_ERR("failed, module_event_type=%d, identity=0x%x",
      event->u.module_event.type, event->identity);
    return -EFAULT;
  }
#endif

  if (rc < 0) {
    CPP_ERR("failed: cpp_module_process_set_param_event_per_frame rc %d",
      rc);
  }
  return rc;
}


/* cpp_module_handle_set_stream_parm_event:
 *
 *  @module: mct module structure for cpp module
 *  @event: incoming event
 *
 *    Handles stream_param events. Such as SET_FLIP_TYPE.
 *
 *    Returns 0 on success.
 **/
int32_t cpp_module_handle_set_stream_parm_event(mct_module_t* module,
  mct_event_t* event)
{
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_stream_params_t  *stream_params = NULL;

  if(!module || !event) {
    CPP_ERR("failed, module=%p, event=%p", module, event);
    return -EINVAL;
  }

  cam_stream_parm_buffer_t *param =
    (cam_stream_parm_buffer_t *)event->u.ctrl_event.control_event_data;
  int32_t rc;
  cpp_module_ctrl_t *ctrl = (cpp_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  switch(param->type) {
    case  CAM_INTF_PARM_REQUEST_OPS_MODE: {
    cpp_module_stream_clk_rate_t *clk_rate_obj = NULL;

    CPP_DBG("CAM_INTF_PARM_REQUEST_OPS_MODE, value=%d, identity=0x%x",
      param->perf_mode, event->identity);

    cpp_module_get_params_for_identity(ctrl, event->identity,
      &session_params, &stream_params);
    if(!session_params || !stream_params) {
      CPP_ERR("failed %p %p\n", session_params, stream_params);
      return 0;
    }

    PTHREAD_MUTEX_LOCK(&(ctrl->clk_rate_list.mutex));
    clk_rate_obj = cpp_module_find_clk_rate_by_identity(ctrl,
      event->identity);
    if (clk_rate_obj == NULL) {
      CPP_ERR(" clk rate obj = NULL, identity = %x",
        event->identity);
      PTHREAD_MUTEX_UNLOCK(&(ctrl->clk_rate_list.mutex));
      return 0;
    }

    if (stream_params->stream_info->perf_mode != param->perf_mode) {
      clk_rate_obj->perf_mode = param->perf_mode;
    } else {
      clk_rate_obj->perf_mode = stream_params->stream_info->perf_mode;
    }
    PTHREAD_MUTEX_UNLOCK(&(ctrl->clk_rate_list.mutex));

    PTHREAD_MUTEX_LOCK(&(ctrl->cpp_mutex));
    ctrl->runtime_clk_update = TRUE;
    PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
    break;
  }
  case CAM_STREAM_PARAM_TYPE_SET_FLIP: {
    int32_t flip_mask = param->flipInfo.flip_mask;

    rc = cpp_module_set_parm_flip(ctrl, event->identity, flip_mask);
    if(rc) {
      CPP_ERR("set_parm_flip failed %d", rc);
      return rc;
    }
    rc = cpp_module_send_event_downstream(module, event);
    if(rc < 0) {
      CPP_ERR("failed, control_event_type=%d, identity=0x%x",
        event->u.ctrl_event.type, event->identity);
      return -EFAULT;
    }
    break;
  }
#ifdef REQUEST_FRAMES
  case CAM_INTF_PARM_REQUEST_FRAMES: {
    CPP_ERR("CAM_INTF_PARM_REQUEST_FRAMES, enable=%d, identity=0x%x",
      param->frameRequest.enableStream, event->identity);
    /* get parameters based on the identity */
    cpp_module_get_params_for_identity(ctrl, event->identity,
      &session_params, &stream_params);
    if(!session_params || !stream_params) {
      CPP_ERR("failed %p %p\n", session_params, stream_params);
      return -EFAULT;
    }
    stream_params->req_frame_output = param->frameRequest.enableStream;
    if (!stream_params->req_frame_output)
      stream_params->current_burst_cnt = 0;
    break;
  }
#endif
  default:
    rc = cpp_module_send_event_downstream(module, event);
    if(rc < 0) {
      CPP_ERR("failed, control_event_type=%d, identity=0x%x",
        event->u.ctrl_event.type, event->identity);
      return -EFAULT;
    }
    break;
  }

  return 0;
}

/* cpp_module_handle_update_buf_info
 *
 *  @ module - structure that holds current module information.
 *  @ event - structure that holds event data.
 *
 *  Event handler that creates buff info list send it to hardware layer for
 *  process. Send downstream event also.
 *
 *  Returns 0 on success.
 *
**/
int32_t cpp_module_handle_update_buf_info(mct_module_t* module,
  mct_event_t* event, boolean delete_buf)
{
  uint32_t                        i;
  cpp_module_stream_buff_info_t   stream_buff_info;
  cpp_hardware_stream_buff_info_t hw_strm_buff_info;
  cpp_hardware_buff_update_t      buff_update;
  mct_stream_map_buf_t *buf_holder =
    (mct_stream_map_buf_t *)event->u.ctrl_event.control_event_data;
  cpp_module_ctrl_t              *ctrl =
    (cpp_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  cpp_hardware_cmd_t              cmd;
  boolean                         rc = 0;
  cpp_module_session_params_t    *session_params = NULL;
  cpp_module_stream_params_t     *stream_params = NULL;
  mct_port_t                   *stream_port;
  mct_module_type_t            mod_type = MCT_MODULE_FLAG_INVALID;


  memset(&stream_buff_info, 0, sizeof(cpp_module_stream_buff_info_t));
  memset(&hw_strm_buff_info, 0, sizeof(cpp_hardware_stream_buff_info_t));

    /* get parameters based on the identity */
  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!session_params || !stream_params) {
    CPP_BUF_ERR("failed %p %p\n", session_params, stream_params);
    rc = -EFAULT;
    goto end;
  }

  stream_port = (mct_port_t *) stream_params->parent;
  if (stream_port == NULL) {
    CPP_BUF_ERR("stream_port null, failed\n");
    rc = -EFAULT;
    goto end;
  }

  mod_type = mct_module_find_type(module,stream_params->identity);
  if (stream_params->is_stream_on == FALSE) {
    rc = cpp_module_send_event_downstream(module,event);
    if(rc < 0) {
      CPP_BUF_ERR("failed %d\n", rc);
    }
    goto end;
  }

 /* synchronize with stream on/off/buffer divert decision */
  MCT_OBJECT_LOCK(stream_port);

  /* attach the identity */
  stream_buff_info.identity = event->identity;
  stream_buff_info.delete_buf = delete_buf;
  /* Apend the new buffer to cpp module's  own list of buffer info */
  if (cpp_module_util_map_buffer_info(buf_holder, &stream_buff_info) == FALSE) {
    CPP_BUF_ERR("error creating stream buff list\n");
    goto CPP_MODULE_BUF_UPDATE_ERROR1;
  }

  if (!stream_buff_info.num_buffs) {
    rc = cpp_module_send_event_downstream(module,event);
    goto CPP_MODULE_BUF_UPDATE_ERROR1;
  }

  /* create and translate to hardware buffer array */
  hw_strm_buff_info.buffer_info = (cpp_hardware_buffer_info_t *)CAM_MALLOC(
    sizeof(cpp_hardware_buffer_info_t) * stream_buff_info.num_buffs);
  if(NULL == hw_strm_buff_info.buffer_info) {
    CPP_BUF_ERR("error creating hw buff list\n");
    goto CPP_MODULE_BUF_UPDATE_ERROR1;
  }

  hw_strm_buff_info.identity = stream_buff_info.identity;
  if (mct_list_traverse(stream_buff_info.buff_list,
    cpp_module_util_create_hw_stream_buff, &hw_strm_buff_info) == FALSE) {
    CPP_BUF_ERR("error creating stream buff list\n");
    goto CPP_MODULE_BUF_UPDATE_ERROR2;
  }

  if(hw_strm_buff_info.num_buffs != stream_buff_info.num_buffs) {
    CPP_BUF_ERR("error creating stream buff list\n");
    goto CPP_MODULE_BUF_UPDATE_ERROR2;
  }

  buff_update.delete_buf = delete_buf;
  buff_update.is_secure = stream_params->stream_info->is_secure;
  buff_update.stream_buff_list = &hw_strm_buff_info;

  cmd.u.buff_update = &buff_update;
  cmd.type = CPP_HW_CMD_BUF_UPDATE;
  rc = cpp_hardware_process_command(ctrl->cpphw, cmd);
  if(rc < 0) {
    CPP_BUF_ERR("CPP_HW_CMD_BUF_UPDATE failed\n");
    goto CPP_MODULE_BUF_UPDATE_ERROR2;
  }
  rc = cpp_module_send_event_downstream(module,event);
  if(rc < 0) {
    CPP_BUF_ERR("send down stream failed\n");
    goto CPP_MODULE_BUF_UPDATE_ERROR2;
  }

CPP_MODULE_BUF_UPDATE_ERROR2:
  free(hw_strm_buff_info.buffer_info);
CPP_MODULE_BUF_UPDATE_ERROR1:
  mct_list_traverse(stream_buff_info.buff_list,
    cpp_module_util_free_buffer_info, &stream_buff_info);
  mct_list_free_list(stream_buff_info.buff_list);

  MCT_OBJECT_UNLOCK(stream_port);
end:
  return rc;
}


/* cpp_module_handle_streamon_event:
 *
 **/
int32_t cpp_module_handle_streamon_event(mct_module_t* module,
  mct_event_t* event)
{
  cpp_module_stream_buff_info_t   stream_buff_info;
  cpp_hardware_stream_buff_info_t hw_strm_buff_info;
  cpp_hardware_buff_update_t      buff_update;
  mct_stream_info_t              *streaminfo =
    (mct_stream_info_t *)event->u.ctrl_event.control_event_data;
  cpp_module_ctrl_t              *ctrl =
    (cpp_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  cpp_hardware_cmd_t              cmd;
  boolean                         rc = -EINVAL;
  mct_event_t                     new_event;
  stats_get_data_t                stats_get;
  mct_port_t                   *stream_port = NULL;
  uint32_t                        i = 0;
  cpp_module_stream_params_t      *linked_stream_params = NULL;
  boolean alloc_buf = false;
  /* get stream parameters */
  cpp_module_session_params_t* session_params = NULL;
  cpp_module_stream_params_t*  stream_params = NULL;
  cpp_module_stream_params_t      *top_stream_param = NULL;
  uint32_t                        buff_count = 0;
  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CPP_ERR("invalid stream params, failed\n");
    return -EFAULT;
  }
  stream_port = (mct_port_t *) stream_params->parent;
  if (stream_port == NULL) {
    CPP_ERR("stream port null, failed\n");
    return -EFAULT;
  }

  /*lock afor stream on/off/buffer divert decision*/
  MCT_OBJECT_LOCK(stream_port);

  memset(&stream_buff_info, 0, sizeof(cpp_module_stream_buff_info_t));
  memset(&hw_strm_buff_info, 0, sizeof(cpp_hardware_stream_buff_info_t));

  /* attach the identity */
  stream_params->cur_frame_id = 0;
  stream_buff_info.identity = event->identity;
  /* traverse through the mct stream buff list and create cpp module's
     own list of buffer info */
  if (mct_list_traverse(streaminfo->img_buffer_list,
    cpp_module_util_map_buffer_info, &stream_buff_info) == FALSE) {
    CPP_ERR("error creating stream buff list\n");
    goto CPP_MODULE_STREAMON_ERROR1;
  }

  /* create and translate to hardware buffer array */
  hw_strm_buff_info.buffer_info = (cpp_hardware_buffer_info_t *)CAM_MALLOC(
    sizeof(cpp_hardware_buffer_info_t) * stream_buff_info.num_buffs);
  if(NULL == hw_strm_buff_info.buffer_info) {
    CPP_ERR("error creating hw buff list\n");
    goto CPP_MODULE_STREAMON_ERROR1;
  }

  hw_strm_buff_info.identity = stream_buff_info.identity;
  if (mct_list_traverse(stream_buff_info.buff_list,
    cpp_module_util_create_hw_stream_buff, &hw_strm_buff_info) == FALSE) {
    CPP_ERR("error creating stream buff list\n");
    goto CPP_MODULE_STREAMON_ERROR2;
  }

  if(hw_strm_buff_info.num_buffs != stream_buff_info.num_buffs) {
    CPP_ERR("error creating stream buff list\n");
    goto CPP_MODULE_STREAMON_ERROR2;
  }

  //configure number of preview frames to process in turbo
  if ((stream_params->stream_type == CAM_STREAM_TYPE_PREVIEW) &&
    (!session_params->is_slave)) {
    uint32_t turbo_cnt = session_params->turbo_frame_count;
    stream_params->track_frame_done = TRUE;
    stream_params->turbo_frameproc_count = turbo_cnt ?
      turbo_cnt: CPP_FRAMES_TURBO_THRESHOLD;
    CPP_LOW("track_frame_done %d turbo_frameproc_count %d\n",
      stream_params->track_frame_done,
      stream_params->turbo_frameproc_count);
  }

  cpp_module_set_clock_freq(ctrl, module, stream_params,
    1,session_params);

  buff_update.delete_buf = FALSE;
  buff_update.is_secure = stream_params->stream_info->is_secure;
  buff_update.secure_mode = stream_params->stream_info->secure_mode;
  buff_update.stream_buff_list = &hw_strm_buff_info;

  cmd.u.buff_update = &buff_update;
  cmd.type = CPP_HW_CMD_STREAMON;
  rc = cpp_hardware_process_command(ctrl->cpphw, cmd);
  if(rc < 0) {
    CPP_ERR("CPP_HW_CMD_STREAMON failed\n");
    goto CPP_MODULE_STREAMON_ERROR2;
  }
  rc = cpp_module_send_event_downstream(module,event);
  if(rc < 0) {
    CPP_ERR("failed %d\n", rc);
    goto CPP_MODULE_STREAMON_ERROR2;
  }

  CPP_DBG("check native_buff %d\n",stream_params->native_buff);
  if (stream_params->native_buff > 0) {
    cam_frame_len_offset_t plane_info = {};
    rc = cpp_module_util_update_plane_info(&stream_params->hw_params,
      &stream_params->hw_params.input_info, &plane_info);
    if (rc < 0) {
      CPP_ERR("update info failed\n");
    }

    if (stream_params->num_linked_streams == 0) {
      CPP_DBG("non linked for 0x%x", stream_params->identity);
      alloc_buf = true;
      buff_count = stream_params->native_buff;
      stream_params->hw_params.output_buffer_info.identity =
        stream_params->identity;
      stream_params->share_output = 1;
    } else {
      CPP_DBG("linked for 0x%x linked %d",
              stream_params->identity, stream_params->num_linked_streams);
      top_stream_param = NULL;
      buff_count = 0;

      PTHREAD_MUTEX_LOCK(&(ctrl->cpp_mutex));
      ctrl->runtime_clk_update = TRUE;
      PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));

      /*It's assumed that the first stream on linked stream has the highest resolution.
        so it's not required to loop for greatest resolution to allocate buffer. => after 1st
        stream-on, next stream-on for linked stream would have lesser resolution*/

      for (i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
        linked_stream_params = stream_params->linked_streams[i];
        if (linked_stream_params == NULL || linked_stream_params->identity == 0) {
          continue;
        }

        CPP_DBG("linked_stream_params 0x%x linked %d",
              linked_stream_params->identity, linked_stream_params->share_output);
        //Keep a reference for 1st stream in the list
        if (top_stream_param == NULL) {
          top_stream_param = linked_stream_params;
        }

        //Get the max buffer required for the group
        if (linked_stream_params->native_buff > buff_count) {
          buff_count = linked_stream_params->native_buff;
        }
      }

      //linked_stream_params = stream_params->linked_streams[0];
      PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
      if (stream_params->share_output == 0) {
        alloc_buf = true;
        stream_params->hw_params.output_buffer_info.identity =
          stream_params->identity;
      }

      stream_params->share_output += 1;
      if(top_stream_param) {
        top_stream_param->share_output += 1;
        /*All linked stream shares same output_buffer_info identity*/
        top_stream_param->hw_params.output_buffer_info.identity =
          stream_params->hw_params.output_buffer_info.identity;
      }
      PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
    }

    if (alloc_buf) {
      buff_count = buff_count + MIN_NATIVE_BUF;
      CPP_DBG("allocating %d buffer for 0x%x", buff_count,stream_params->identity);
      rc = pp_native_buf_mgr_allocate_buff(&ctrl->pp_buf_mgr, buff_count,
        CPP_GET_SESSION_ID(stream_params->identity),
        CPP_GET_STREAM_ID(stream_params->identity), &plane_info, 1);

      if (rc < 0) {
        CPP_ERR("native buf allocation failed");
        goto CPP_MODULE_STREAMON_ERROR2;
      }
    }
  }

  if ((ctrl->tnr_module_func_tbl.prepare) &&
    (stream_params->hw_params.tnr_mask)) {
      rc = ctrl->tnr_module_func_tbl.prepare(ctrl, stream_params->identity);
      if (rc < 0) {
        CPP_TNR_ERR("TNR prepare failed %d\n", rc);
        goto CPP_MODULE_STREAMON_ERROR2;
      }
  }

  /* change state to stream ON */
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  stream_params->is_stream_on = TRUE;
  if (!session_params->stream_on_count) {
    session_params->is_stream_on = TRUE;
  }
  if (stream_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    session_params->per_frame_params.offline_stream_cnt++;
  } else if (stream_params->stream_type != CAM_STREAM_TYPE_PARM) {
    session_params->per_frame_params.real_time_stream_cnt++;
  }
  session_params->stream_on_count++;
  stream_params->hw_params.diagnostic_enable =
    session_params->hw_params.diagnostic_enable;
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
  CPP_INFO("identity=0x%x, stream-on done", event->identity);

  if (stream_params->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) {
    /* TODO: Fetch AEC info by generating an event and store triggers in
       session params */
    new_event.type = MCT_EVENT_MODULE_EVENT;
    new_event.identity = streaminfo->identity;
    new_event.direction = MCT_EVENT_UPSTREAM;
    new_event.u.module_event.type = MCT_EVENT_MODULE_PPROC_GET_AEC_UPDATE;
    new_event.u.module_event.module_event_data = (void *)&stats_get;
    memset(&stats_get, 0, sizeof(stats_get_data_t));
    rc = cpp_module_send_event_upstream(module, &new_event);
    if(rc < 0) {
      CPP_ERR("failed %d", rc);
      goto CPP_MODULE_STREAMON_ERROR2;
    }

    session_params->aec_trigger.gain = stats_get.aec_get.real_gain[0];
    session_params->aec_trigger.lux_idx = stats_get.aec_get.lux_idx;

    stream_params->hw_params.scene_mode_on =
      session_params->hw_params.scene_mode_on;
    stream_params->hw_params.aec_trigger.lux_idx =
      session_params->aec_trigger.lux_idx;
    stream_params->hw_params.aec_trigger.gain =
      session_params->aec_trigger.gain;
    CPP_DENOISE_LOW("denoise_enable:%d", stream_params->hw_params.denoise_enable);
    CPP_ASF_LOW("sharpness_level:%f", stream_params->hw_params.sharpness_level);
    CPP_ASF_LOW("asf_mode:%d", stream_params->hw_params.asf_mode);

    stream_params->hw_params.interpolate_mask |= CPP_INTERPOLATE_ALL;
  }

  stream_params->current_burst_cnt = 0;
  rc = 0;

CPP_MODULE_STREAMON_ERROR2:
  free(hw_strm_buff_info.buffer_info);

CPP_MODULE_STREAMON_ERROR1:
  mct_list_traverse(stream_buff_info.buff_list,
    cpp_module_util_free_buffer_info, &stream_buff_info);
  mct_list_free_list(stream_buff_info.buff_list);

  MCT_OBJECT_UNLOCK(stream_port);
  return rc;
}

/* cpp_module_handle_streamoff_event:
 *
 **/
int32_t cpp_module_handle_streamoff_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t                      rc;
  int32_t                      handle_downstream_error;
  cpp_module_frame_hold_t     *frame_hold = NULL;
  cpp_per_frame_params_t      *per_frame_params;
  uint32_t                     queue_len = 0;
  uint32_t                     j, i;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_stream_params_t  *linked_stream_params = NULL;
  mct_port_t                  *stream_port = NULL;
  uint32_t                     num_linked_streams = 0;
  cpp_hardware_cmd_t           cmd;

  if(!module || !event) {
    CPP_ERR("failed, module=%p, event=%p\n",
      module, event);
    return -EINVAL;
  }
  mct_stream_info_t *streaminfo =
    (mct_stream_info_t *)event->u.ctrl_event.control_event_data;
  uint32_t identity = event->identity;
  CPP_INFO("info: doing stream-off for identity 0x%x", identity);

  cpp_module_ctrl_t* ctrl = (cpp_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_ERR("invalid ctrl failed\n");
    return -EINVAL;
  }
  cpp_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CPP_ERR("invalid stream parama, failed\n");
    return -EFAULT;
  }

  stream_port = (mct_port_t *) stream_params->parent;
  if (stream_port == NULL) {
    CPP_ERR("invalid stream port failed\n");
    return -EFAULT;
  }

  memset(&cmd, 0, sizeof(cpp_hardware_cmd_t));

  PTHREAD_MUTEX_LOCK(&(session_params->dis_mutex));

  /* Check whether there is a frame on HOLD */
  frame_hold = &session_params->frame_hold;
  if (frame_hold->is_frame_hold == TRUE) {
    for (i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
      linked_stream_params = stream_params->linked_streams[i];
      if (linked_stream_params == NULL ||
        linked_stream_params->identity == 0) {
        continue;
      }

      /* Check whether identity of frame on HOLD matches with identity of
         stream_params / linked_stream_params */
      if ((stream_params->identity == frame_hold->identity) ||
        (linked_stream_params->identity == frame_hold->identity)) {
        /* Send frame on HOLD for processing */
        cpp_module_send_buf_divert_event(module, frame_hold->identity,
          &frame_hold->isp_buf);

        /* Set is_frame_hold flag to FALSE */
        frame_hold->is_frame_hold = FALSE;

        /*frame hold only support for one stream and one frame.
          may extend to multiple streams in the future*/
        break;
      }
    }
  }
  /*
   * Check whether DIS is enabled along with is type AND current stream off
   * belongs to preview or video
   */
  if (IF_DIS_ENABLE(session_params, stream_params)) {
    /* DIS crop event is not sent for camcorder preview. It is sent only after
       camcorder recording is started. while returning from camcorder recording
       to camcorder preview session, DIS crop should not be applied for future
       preview frames. Invalidate DIS crop valid flag so that preview stream
       does not use it */
    session_params->dis_hold.is_valid = FALSE;
    /* Reset DIS crop params in stream_params and linked_stream_params */
    PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
    stream_params->hw_params.crop_info.is_crop.x = 0;
    stream_params->hw_params.crop_info.is_crop.y = 0;
    stream_params->hw_params.crop_info.is_crop.dx =
      stream_params->hw_params.input_info.width;
    stream_params->hw_params.crop_info.is_crop.dy =
      stream_params->hw_params.input_info.height;
    PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

    for (i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
      linked_stream_params = stream_params->linked_streams[i];
      if (linked_stream_params &&
          linked_stream_params->identity != 0) {
        PTHREAD_MUTEX_LOCK(&(linked_stream_params->mutex));
        linked_stream_params->hw_params.crop_info.is_crop.x = 0;
        linked_stream_params->hw_params.crop_info.is_crop.y = 0;
        linked_stream_params->hw_params.crop_info.is_crop.dx =
          linked_stream_params->hw_params.input_info.width;
        linked_stream_params->hw_params.crop_info.is_crop.dy =
          linked_stream_params->hw_params.input_info.height;
        PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
      }
    }
  }
  PTHREAD_MUTEX_UNLOCK(&(session_params->dis_mutex));

  /*lock after stream on/off/buffer divert decision*/
  MCT_OBJECT_LOCK(stream_port);
  /* invalidate any remaining entries in queue corresponding to
     this identity. This will also send/update corresponding ACKs */
  CPP_DBG("info: invalidating queue.");
  rc = cpp_module_invalidate_queue(ctrl, identity);
  if (rc < 0) {
    CPP_ERR("invalidating queue-failed, continue stream-off\n");
    rc = -EFAULT;
  }

  /* change the state of this stream to OFF, this will prevent
     any incoming buffers to be added to the processing queue  */
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  stream_params->is_stream_on = FALSE;
  stream_params->cur_frame_id = 0;
  session_params->stream_on_count--;
  if (!session_params->stream_on_count) {
    session_params->is_stream_on = FALSE;
  }
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

  MCT_OBJECT_UNLOCK(stream_port);


  /* send stream_off to downstream. This blocking call ensures
     downstream modules are streamed off and no acks pending from them */
  handle_downstream_error = cpp_module_send_event_downstream(module, event);
  if (handle_downstream_error < 0) {
    CPP_ERR("downstream stream-off failed\n");
  }
  CPP_HIGH("downstream stream-off done.");

  MCT_OBJECT_LOCK(stream_port);

  cpp_module_set_clock_freq(ctrl, module, stream_params,
    0, session_params);
  /* process hardware command for stream off, this ensures
     hardware is done with this identity */
  cmd.type = CPP_HW_CMD_STREAMOFF;
  cmd.u.streamoff_data.streamoff_identity = streaminfo->identity;
  cmd.u.streamoff_data.is_hw_err = ctrl->is_hw_error;
  cmd.u.streamoff_data.is_secure = streaminfo->is_secure;
  memset(cmd.u.streamoff_data.linked_identity, 0,
    sizeof(uint32_t ) * CPP_HARDWARE_MAX_STREAMS);

  for (i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
    linked_stream_params = stream_params->linked_streams[i];
    if (linked_stream_params != NULL && linked_stream_params->identity != 0) {
      cmd.u.streamoff_data.linked_identity[num_linked_streams] = linked_stream_params->identity;
      num_linked_streams++;
    }
  }

  cmd.u.streamoff_data.num_linked_streams =  num_linked_streams;
  rc = cpp_hardware_process_command(ctrl->cpphw, cmd);
  if(rc < 0) {
    CPP_ERR("hw streamoff failed\n");
    goto end;
  }
  if(handle_downstream_error < 0) {
    CPP_ERR("error from down stream modules = %d \n", handle_downstream_error);
    rc = -EFAULT;
    goto end;
  }

  if (stream_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    per_frame_params = &session_params->per_frame_params;
    if (per_frame_params->offline_stream_cnt) {
      per_frame_params->offline_stream_cnt--;
      if (!per_frame_params->offline_stream_cnt) {
        cpp_module_free_stream_based_entry(ctrl,
          stream_params->stream_type, per_frame_params);
      }
    }
  } else if (stream_params->stream_type != CAM_STREAM_TYPE_PARM) {
    per_frame_params = &session_params->per_frame_params;
    if (per_frame_params->real_time_stream_cnt) {
      per_frame_params->real_time_stream_cnt--;
      if (!per_frame_params->real_time_stream_cnt) {
        cpp_module_free_stream_based_entry(ctrl,
          stream_params->stream_type, per_frame_params);
        memset(&session_params->valid_stream_ids[0], 0,
          sizeof(session_params->valid_stream_ids));
        memset(&session_params->verify_proc_stream_ids[0], 0,
         sizeof(session_params->verify_proc_stream_ids));
      }
    }
  }

end:

  if ( stream_params->native_buff > 0 ) {
      /*cpp uses native buffers,so clean it*/
    if (pp_native_buf_mgr_queue_exists(&ctrl->pp_buf_mgr,
        CPP_GET_SESSION_ID(stream_params->identity),
        CPP_GET_STREAM_ID(stream_params->identity))) {
      pp_native_buf_mgr_free_queue(&ctrl->pp_buf_mgr,
        CPP_GET_SESSION_ID(stream_params->identity),
        CPP_GET_STREAM_ID(stream_params->identity));
      }
  }
  if ((ctrl->tnr_module_func_tbl.unprepare) &&
    (stream_params->hw_params.tnr_mask)) {
      rc = ctrl->tnr_module_func_tbl.unprepare(ctrl, stream_params->identity);
      if (rc < 0) {
        CPP_TNR_ERR("TNR unprepare failed %d\n", rc);
      }
  }
  CPP_INFO("info: stream-off done for identity 0x%x rc:%d", identity,rc);
  MCT_OBJECT_UNLOCK(stream_port);
  return rc;
}

int32_t cpp_module_handle_sleep_event(mct_module_t* module,mct_event_t* event){
   cpp_module_ctrl_t* ctrl = NULL;
   cam_dual_camera_perf_control_t *perf_ctrl = NULL;
   cpp_module_stream_params_t *stream_params = NULL;
   cpp_module_session_params_t *session_params = NULL;
   int32_t rc = TRUE;

   if (!module || !event) {
      CPP_ERR("failed, module=%p, event=%p\n",module,event);
      return -EFAULT;
   }

   ctrl = (cpp_module_ctrl_t*)MCT_OBJECT_PRIVATE(module);
   if (!ctrl) {
      CPP_ERR("Invalid cpp ctrl, failed\n");
      return -EFAULT;
   }

   perf_ctrl = (cam_dual_camera_perf_control_t*)
                 event->u.ctrl_event.control_event_data;
   if (!perf_ctrl){
      CPP_ERR("Invalid perf_ctrl, failed\n");
      return -EFAULT;
   }

   cpp_module_get_params_for_identity(ctrl, event->identity,
     &session_params, &stream_params);
   if(!stream_params || !session_params) {
    CPP_ERR("Invalid stream param %p session param %p, failed\n",
    stream_params, session_params);
    return -EFAULT;
   }

   if (perf_ctrl->enable == TRUE) {
     CPP_HIGH("Session for id %x will sleep\n",event->identity);
     session_params->dualcam_perf.enable = TRUE;
     session_params->dualcam_perf.perf_mode = perf_ctrl->perf_mode;
   } else {
     session_params->dualcam_perf.enable = FALSE;
     session_params->dualcam_perf.perf_mode = -1;
   }

   return rc;
}

/** cpp_module_handle_sof_notify:
 *
 *  @module: module handle
 *
 *  @event: event
 *
 *  Handle sof notify event
 *
 *  Return 0 on success and negative on failure
 **/
int32_t cpp_module_handle_sof_notify(mct_module_t *module, mct_event_t *event)
{
  int32_t                      rc = 0;
  cpp_module_ctrl_t           *ctrl = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_per_frame_params_t      *per_frame_params = NULL;
  uint32_t                     real_time_stream_cnt;
  mct_bus_msg_t                bus_msg;

  MCT_PROF_LOG_BEG(PROF_CPP_SOF);
  if (!module || !event) {
    CPP_ERR("failed module %p event %p\n", module, event);
    return -EINVAL;
  }

  ctrl = (cpp_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_ERR("invalid cpp control\n");
    return -EFAULT;
  }

  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CPP_ERR("invalid stream params, failed\n");
    return -EFAULT;
  }

  /* Update current frame id */
  per_frame_params = &session_params->per_frame_params;
  PTHREAD_MUTEX_LOCK(&per_frame_params->mutex);
  per_frame_params->cur_frame_id = event->u.module_event.current_frame_id;
  real_time_stream_cnt = per_frame_params->real_time_stream_cnt;
  PTHREAD_MUTEX_UNLOCK(&per_frame_params->mutex);

  cpp_module_handle_request_based_fallback(ctrl, session_params);

  CPP_HIGH("frmid=%d, identity=0x%x",
    per_frame_params->cur_frame_id, event->identity);

  if (real_time_stream_cnt) {
    rc = cpp_module_post_sticky_meta_entry(ctrl, event->identity,
      per_frame_params, per_frame_params->cur_frame_id,
      CAM_INTF_PARM_EFFECT);
    if(rc < 0) {
      CPP_META_ERR("failed %d\n", rc);
      return -EFAULT;
    }

    rc = cpp_module_post_sticky_meta_entry(ctrl, event->identity,
     per_frame_params, per_frame_params->cur_frame_id,
       CAM_INTF_META_EDGE_MODE);
    if(rc < 0) {
      CPP_META_ERR("failed %d\n", rc);
      return -EFAULT;
   }

    rc = cpp_module_post_sticky_meta_entry(ctrl, event->identity,
     per_frame_params, per_frame_params->cur_frame_id,
       CAM_INTF_META_NOISE_REDUCTION_MODE);
    if(rc < 0) {
      CPP_META_ERR("failed %d\n", rc);
      return -EFAULT;
    }

    rc = cpp_module_post_sticky_meta_entry(ctrl, event->identity,
     per_frame_params, per_frame_params->cur_frame_id,
       CAM_INTF_PARM_CDS_MODE);
    if(rc < 0) {
      CPP_META_ERR("failed %d", rc);
      return -EFAULT;
    }

    rc = cpp_module_post_sticky_meta_entry(ctrl, event->identity,
      per_frame_params, per_frame_params->cur_frame_id,
      CAM_INTF_META_IMG_DYN_FEAT);
    if (rc < 0) {
      CPP_META_ERR("post dyn_feat meta failed %d", rc);
      return -EFAULT;
    }

    rc = cpp_module_process_frame_control(module, per_frame_params,
      event->identity, per_frame_params->cur_frame_id, CPP_FRAME_QUEUE_REPORT);
    if (rc < 0) {
      CPP_META_ERR("failed: rc %d\n", rc);
    }

#ifdef ASF_OSD
    {
      cam_asf_trigger_regions_t    regions;
      /* To send message to MCT bus, we need the pointer to parent MCT module.
         Send event upstream to get it, then use it for bus msg */
      mct_event_t event1;
      mct_module_t *parent_module = NULL;

      event1.type = MCT_EVENT_MODULE_EVENT;
      event1.direction = MCT_EVENT_UPSTREAM;
      event1.identity = event->identity;
      event1.u.module_event.type = MCT_EVENT_MODULE_GET_PARENT_MODULE;
      event1.u.module_event.module_event_data = &parent_module;
      rc = cpp_module_send_event_upstream(module, &event1);
      if(rc < 0) {
        CPP_META_ERR("failed, module_event_type=%d, identity=0x%x",
          event1.u.module_event.type, event1.identity);
        return -EFAULT;
      }
      if(parent_module == NULL) {
        CPP_META_ERR("failed, parent module NULL");
        return -EFAULT;
      }

      memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
      memset(&regions, 0, sizeof(cam_asf_trigger_regions_t));

      bus_msg.type = MCT_BUS_MSG_TRIGGER_REGION;
      bus_msg.msg = (void *)&regions;
      bus_msg.size = sizeof(cam_asf_trigger_regions_t);
      bus_msg.sessionid = CPP_GET_SESSION_ID(event->identity);
      /*Picking up value from session params since SOF notify will be in session stream*/
      regions.region1 = session_params->hw_params.asf_info.asf_reg1_idx;
      regions.region2 = session_params->hw_params.asf_info.asf_reg2_idx;

      CPP_META_LOW("sending bus msg, session_id=%d, R1=%d, R2=%d",
        bus_msg.sessionid, regions.region1, regions.region2);

      if (mct_module_post_bus_msg(parent_module, &bus_msg) != TRUE) {
        CPP_META_ERR("mct_module_post_bus_msg() failed. session_id=%d",
          bus_msg.sessionid);
      }
   }
#endif
  }

  MCT_PROF_LOG_END();
  rc = cpp_module_send_event_downstream(module, event);
  if(rc < 0) {
    CPP_ERR("failed\n");
    return -EFAULT;
  }

  return rc;
}

int32_t cpp_module_handle_frame_skip_event(mct_module_t *module,
  mct_event_t *event)
{
  cpp_frame_ctrl_data_t       *frame_ctrl_data = NULL;
  uint32_t                     skip_frame_id = 0;
  uint32_t                     skip_frame_cnt = 1;
  uint32_t                     q_idx = 0;
  uint32_t                     new_q_idx = 0;
  cpp_module_ctrl_t           *ctrl = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_per_frame_params_t      *per_frame_params = NULL;
  boolean                      queue_compatible = TRUE;
  uint32_t                     i, queue_len = 0;

  if (!module || !event) {
    CPP_ERR("failed %p %p\n", module, event);
    return -EFAULT;
  }

  ctrl = (cpp_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_ERR("invalid cpp ctrl, failed");
    return -EFAULT;
  }

  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CPP_ERR("invalid stream params, failed\n");
    return -EFAULT;
  }

  /* Update current frame id */
  per_frame_params = &session_params->per_frame_params;
  skip_frame_id = *(uint32_t *)event->u.module_event.module_event_data;
  q_idx = (skip_frame_id % FRAME_CTRL_SIZE);
  new_q_idx = (q_idx + skip_frame_cnt) % FRAME_CTRL_SIZE;

  CPP_PER_FRAME_DBG("process frmid:%d q_idx:%d \n", skip_frame_id, q_idx);
  pthread_mutex_lock(&per_frame_params->frame_ctrl_mutex[q_idx]);
  queue_len = per_frame_params->frame_ctrl_q[q_idx]->length;
  for (i = 0; i < queue_len; i++) {
    queue_compatible = cpp_module_pop_per_frame_entry(ctrl, per_frame_params,
      q_idx, skip_frame_id, &frame_ctrl_data, stream_params);
    if (queue_compatible == FALSE)
      break;

    if (!frame_ctrl_data)
      continue;

    frame_ctrl_data->frame_id = skip_frame_id + skip_frame_cnt;
    new_q_idx = (frame_ctrl_data->frame_id % FRAME_CTRL_SIZE);
    mct_queue_push_tail(per_frame_params->frame_ctrl_q[new_q_idx],
      (void *)frame_ctrl_data);
  }
  pthread_mutex_unlock(&per_frame_params->frame_ctrl_mutex[q_idx]);

  return TRUE;
}

/** cpp_module_post_sticky_meta_entry: Store frame control
 *  requests
 *
 *  @identity: unique stream & session identity
 *  @per_frame_params: handle to per frame params
 *  @future_frame_id:  future frame id
 *  @type: Type of frame control parm
 *
 *  This function is called to queue the frame control parms
 *  request to be processed later
 *
 *  Return: 0 for success and negative for failure
 **/
int32_t cpp_module_post_sticky_meta_entry(cpp_module_ctrl_t *ctrl,
  uint32_t identity, cpp_per_frame_params_t *per_frame_params,
  uint32_t cur_frame_id, cam_intf_parm_type_t type)
{
  cpp_module_stream_params_t  *stream_params;
  cpp_module_session_params_t *session_params;
  cpp_frame_ctrl_data_t       *frame_ctrl_data = NULL;
  int32_t                      q_idx = (cur_frame_id % FRAME_CTRL_SIZE);
  void                        *parm_data = NULL;
  int32_t                      rc = -EFAULT;

  if (!ctrl || !per_frame_params) {
    CPP_META_ERR("failed ctrl:%p, per_frame_params:%p\n",
      ctrl, per_frame_params);
    return rc;
  }

  /* get parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!session_params) {
    CPP_META_ERR("invalid session params, failed\n");
    return -EFAULT;
  }

  PTHREAD_MUTEX_LOCK(&per_frame_params->frame_ctrl_mutex[q_idx]);
  switch (type) {
  case CAM_INTF_PARM_EFFECT:
    CPP_ASF_LOW("CAM_INTF_PARM_EFFECT\n");
    parm_data = CAM_CALLOC(1, sizeof(int32_t));
    if (parm_data) {
      *((int32_t *)parm_data) = session_params->hw_params.effect_mode;
    }
    break;
  case CAM_INTF_META_EDGE_MODE:
    CPP_ASF_LOW("CAM_INTF_META_EDGE_MODE\n");
    parm_data = CAM_CALLOC(1, sizeof(cam_edge_application_t));
    if (parm_data) {
      ((cam_edge_application_t *)parm_data)->edge_mode =
        session_params->hw_params.edge_mode;
      ((cam_edge_application_t *)parm_data)->sharpness =
        session_params->hw_params.sharpness_level *
        CPP_DEFAULT_SHARPNESS;
    }
    break;
  case CAM_INTF_META_NOISE_REDUCTION_MODE:
    CPP_DENOISE_LOW("CAM_INTF_META_NOISE_REDUCTION_MODE\n");
    parm_data = CAM_CALLOC(1, sizeof(int32_t));
    if (parm_data) {
      *(int32_t *)parm_data = session_params->hw_params.denoise_enable;
    }
    break;
  case CAM_INTF_PARM_CDS_MODE:
    CPP_DENOISE_LOW("CAM_INTF_PARM_CDS_MODE\n");
    parm_data = CAM_CALLOC(1, sizeof(int32_t));
    if (parm_data) {
      *(int32_t *)parm_data = session_params->hw_params.dsdn_enable;
    }
    break;
  case CAM_INTF_META_IMG_DYN_FEAT:
    parm_data =  CAM_CALLOC(1, sizeof(cam_dyn_img_data_t));
    if (parm_data) {
      ((cam_dyn_img_data_t *)parm_data)->dyn_feature_mask |=
        (session_params->hw_params.hyst_dsdn_status << DYN_IMG_CDS_HYS_BIT) |
        (session_params->hw_params.hyst_tnr_status << DYN_IMG_TNR_HYS_BIT) |
        (session_params->hw_params.hyst_pbf_status << DYN_IMG_PBF_HYS_BIT) |
        (session_params->hw_params.hyst_asf_rnr_status << DYN_IMG_ASF_RNR_HYS_BIT);
      CPP_DENOISE_LOW("CAM_INTF_META_IMG_DYN_FEAT_i%x_f%d mask %llx\n",
        identity,
        cur_frame_id,
        ((cam_dyn_img_data_t *)parm_data)->dyn_feature_mask);
    }
    break;
  default:
    break;
  }

  if (parm_data) {
    frame_ctrl_data = CAM_CALLOC(1, sizeof(cpp_frame_ctrl_data_t));
    if (!frame_ctrl_data) {
      CPP_META_LOW("failed\n");
      free(parm_data);
      PTHREAD_MUTEX_UNLOCK(&per_frame_params->frame_ctrl_mutex[q_idx]);
      return rc;
    }

    frame_ctrl_data->u.ctrl_param.type = type;
    frame_ctrl_data->u.ctrl_param.parm_data = parm_data;
    cpp_module_add_report_entry(per_frame_params, frame_ctrl_data,
      cur_frame_id, identity);
  }

  PTHREAD_MUTEX_UNLOCK(&per_frame_params->frame_ctrl_mutex[q_idx]);

  return 0;
}

/** cpp_module_add_report_entry: Add entries to report queue
 *
 *  @per_frame_params: per frame params
 *  @frame_ctrl_data: event data entry
 *  @cur_frame_id: current frame id
 *
 *  This function is called for every sof when frame control is enabled.
 *  It handles the posting of metadata entries to reporting
 *  queue.
 *
 *  Return: 0 for success and negative for failure
 **/
int32_t cpp_module_add_report_entry(
  cpp_per_frame_params_t *per_frame_params,
  cpp_frame_ctrl_data_t *frame_ctrl_data, uint32_t cur_frame_id,
  uint32_t identity)
{
  uint32_t               report_frm_id = 0;
  uint32_t               report_q_idx = 0;
  int32_t                report_offset = 0;

  if (!per_frame_params || !frame_ctrl_data) {
    CPP_META_ERR("failed %p %p\n", per_frame_params, frame_ctrl_data);
    return -EFAULT;
  }

  report_offset = per_frame_params->max_report_delay - 1;
  if (report_offset < 0) {
    report_offset = 0;
  }
  report_frm_id = cur_frame_id + report_offset;
  report_q_idx = report_frm_id % FRAME_CTRL_SIZE;
  frame_ctrl_data->frame_id = report_frm_id;
  frame_ctrl_data->q_entry_type = CPP_FRM_CTRL_Q_REPORT;
  frame_ctrl_data->identity = identity;
  CPP_META_LOW("CPP_FRM_CTRL_Q_REPORT enqueue type = %d reprt_frmid %d"
    " rpt_q_idx:%d\n",
    frame_ctrl_data->u.ctrl_param.type, frame_ctrl_data->frame_id,
    report_q_idx);
  mct_queue_push_tail(per_frame_params->frame_ctrl_q[report_q_idx],
    (void *)frame_ctrl_data);

  return TRUE;
}

/* cpp_module_handle_request_divert:
 *
 *  Handles the buf_divert request event coming from downstream module.
 *
 */
int32_t cpp_module_handle_request_divert(mct_module_t* module,
  mct_event_t* event)
{
  cpp_module_stream_params_t  *stream_params;
  cpp_module_session_params_t *session_params;

  if(!module || !event) {
    CPP_ERR("failed, module=%p, event=%p\n", module, event);
    return -EINVAL;
  }
  cpp_module_ctrl_t* ctrl = (cpp_module_ctrl_t*)MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_ERR("invalid cpp control, failed\n");
    return -EINVAL;
  }

  /* get parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!session_params) {
    CPP_ERR("failed\n");
    return -EFAULT;
  }

  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  stream_params->req_frame_divert =
    *(uint32_t *)event->u.module_event.module_event_data;
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
  return 0;
}

int32_t cpp_module_handle_face_event(mct_module_t* module,
  mct_event_t* event)
{
  cpp_module_stream_params_t  *stream_params;
  cpp_module_session_params_t *session_params;
  mct_face_info_t             *face_info;
  int i = 0;

  if(!module || !event) {
    CPP_ERR("failed, module=%p, event=%p\n", module, event);
    return -EINVAL;
  }

  cpp_module_ctrl_t* ctrl = (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_ERR("invalid cpp control, failed\n");
    return -EFAULT;
  }

  CPP_DBG("identity=0x%x", event->identity);

  /* get stream parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!session_params || !stream_params) {
    CPP_ERR("failed session_params:%p, stream_params:%p\n",
      session_params, stream_params);
    return -EFAULT;
  }

  face_info = event->u.module_event.module_event_data;
  if (stream_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    /* Update the zsl shot chromatix ptr in session params. */
    stream_params->hw_params.face_count = face_info->face_count;
  } else {
    /* Update the chromatix ptr in session params. */
    session_params->hw_params.face_count = face_info->face_count;
    for(i=0; i<CPP_MODULE_MAX_STREAMS; i++) {
      if(session_params->stream_params[i] &&
         (session_params->stream_params[i]->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC)) {
        session_params->stream_params[i]->hw_params.face_count = face_info->face_count;
      }
    }
  }
  return 0;
}

/** cpp_module_set_parm_dsdn:
 *
 *  @ctrl - This parameter holds the cpp control data structure.
 *  @identity - This parameter holds identity on which the cds event
 *   is recieved.
 *  @value - This parameter holds the cds value.
 *
 *  This function is a set parameter function used to store the cds value
 *  set by HAL on a particular identity.
 *
 *  Return: Returns 0 at success and error (-ve value) on failure.
 **/
int32_t cpp_module_set_parm_dsdn(cpp_module_ctrl_t *ctrl, uint32_t identity,
  int32_t value)
{
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  uint32_t                     dsdn_value;
  int32_t                      rc = 0, i;

  if (!ctrl) {
    CPP_DENOISE_ERR("failed ctrl %p", ctrl);
    rc = -EFAULT;
    goto end;
  }
  if (!ctrl->cpphw) {
    CPP_DENOISE_ERR("failed cpphw %p", ctrl->cpphw);
    rc = -EFAULT;
    goto end;
  }

  if (!IS_HW_VERSION_SUPPORTS_DSDN(ctrl->cpphw->hwinfo.version)) {
    CPP_DENOISE_DBG("CPP CDS not supported for version 0x%x",
      ctrl->cpphw->hwinfo.version);
    rc = 0;
    goto end;
  }
  /* get parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!session_params || !stream_params) {
    CPP_DENOISE_ERR("failed, iden 0x%x, session_param %p, stream param %p\n",
      identity, session_params, stream_params);
    rc = -EFAULT;
    goto end;
  }

  dsdn_value = ((value == CAM_CDS_MODE_ON) || (value == CAM_CDS_MODE_AUTO)) ?
    1 : 0;

  if (stream_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
    stream_params->hw_params.dsdn_enable =
      stream_params->hw_params.dsdn_mask ? dsdn_value : 0;
    PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
  } else {
    session_params->hw_params.dsdn_enable = dsdn_value;
    for (i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
      if (session_params->stream_params[i]) {
        switch (session_params->stream_params[i]->stream_type) {
        case CAM_STREAM_TYPE_PREVIEW:
        case CAM_STREAM_TYPE_VIDEO:
        case CAM_STREAM_TYPE_SNAPSHOT:
        case CAM_STREAM_TYPE_CALLBACK:
        PTHREAD_MUTEX_LOCK(&(session_params->stream_params[i]->mutex));
          session_params->stream_params[i]->hw_params.dsdn_enable =
            session_params->stream_params[i]->hw_params.dsdn_mask ? dsdn_value : 0;
        PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));
          break;
        case CAM_STREAM_TYPE_OFFLINE_PROC:
          // Offline handled through stream params, not an error case
          break;
        default:
          CPP_DENOISE_DBG("CPP CDS not supported for stream %d",
            session_params->stream_params[i]->stream_type);
          break;
        }
      }
    }
  }

end:
  return rc;
}

/** cpp_module_set_parm_img_dyn_feat:
 *
 *  @ctrl - This parameter holds the cpp control data structure.
 *  @identity - This parameter holds identity on which the cds event
 *   is recieved.
 *  @value - This parameter holds the cds value.
 *
 *  This function is a set parameter function used to store the dynamic feature
 *  mask value (hystersis value )updated by pproc or internaly calculated
 *  from aec event.
 *
 *  Return: Returns 0 at success and error (-ve value) on failure.
 **/
int32_t cpp_module_set_parm_img_dyn_feat(cpp_module_ctrl_t *ctrl,
  uint32_t identity, uint64_t value)
{
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  uint32_t                     dyn_feat;
  int32_t                      rc = 0, i;
  bool                         previous_tnr_hyst_state = FALSE;

  if (!ctrl) {
    CPP_DENOISE_ERR("failed ctrl %p", ctrl);
    rc = -EFAULT;
    goto end;
  }

  if (!ctrl->cpphw) {
    CPP_DENOISE_ERR("failed cpphw %p", ctrl->cpphw);
    rc = -EFAULT;
    goto end;
  }

  if (!IS_HW_VERSION_SUPPORTS_DYN_FEAT(ctrl->cpphw->hwinfo.version)) {
    CPP_DBG("img_dyn_feat not supported for version 0x%x",
      ctrl->cpphw->hwinfo.version);
    rc = 0;
    goto end;
  }

  /* get parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!session_params || !stream_params) {
    CPP_TNR_ERR("failed, iden 0x%x, session_param %p, stream param %p\n",
      identity, session_params, stream_params);
    rc = -EFAULT;
    goto end;
  }

  if (stream_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
    stream_params->hw_params.hyst_dsdn_status = ((value & DYN_IMG_CDS_HYS) >>
      DYN_IMG_CDS_HYS_BIT);
    previous_tnr_hyst_state = stream_params->hw_params.hyst_tnr_status;
    stream_params->hw_params.hyst_tnr_status = ((value & DYN_IMG_TNR_HYS) >>
      DYN_IMG_TNR_HYS_BIT);
    stream_params->hw_params.hyst_pbf_status = ((value & DYN_IMG_PBF_HYS) >>
      DYN_IMG_PBF_HYS_BIT);
    stream_params->hw_params.hyst_asf_rnr_status =
      ((value & DYN_IMG_ASF_RNR_HYS) >> DYN_IMG_ASF_RNR_HYS_BIT);

    CPP_TNR_HIGH("offline: value %llu, dsdn_status %d, tnr status %d "
      "pbf status %d asf_rnr status %d",
      value, stream_params->hw_params.hyst_dsdn_status,
      stream_params->hw_params.hyst_tnr_status,
      stream_params->hw_params.hyst_pbf_status,
      stream_params->hw_params.hyst_asf_rnr_status);

    if(!previous_tnr_hyst_state && stream_params->hw_params.hyst_tnr_status){
       CPP_TNR_HIGH("TNR switching on. Insert grey frame!");
       stream_params->hw_params.grey_ref_enable = TRUE;
    }
    PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
  } else {
    /* Do not store the value for session stream.It is updated from AEC event */
    for (i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
      if (session_params->stream_params[i]) {
        switch (session_params->stream_params[i]->stream_type) {
        case CAM_STREAM_TYPE_PREVIEW:
        case CAM_STREAM_TYPE_VIDEO:
        case CAM_STREAM_TYPE_SNAPSHOT:
        case CAM_STREAM_TYPE_CALLBACK:
          PTHREAD_MUTEX_LOCK(&(session_params->stream_params[i]->mutex));
          session_params->stream_params[i]->hw_params.hyst_dsdn_status =
            ((value & DYN_IMG_CDS_HYS) >> DYN_IMG_CDS_HYS_BIT);
          previous_tnr_hyst_state =
            session_params->stream_params[i]->hw_params.hyst_tnr_status;
          session_params->stream_params[i]->hw_params.hyst_tnr_status =
            ((value & DYN_IMG_TNR_HYS) >> DYN_IMG_TNR_HYS_BIT);
          session_params->stream_params[i]->hw_params.hyst_pbf_status =
            ((value & DYN_IMG_PBF_HYS) >> DYN_IMG_PBF_HYS_BIT);
          session_params->stream_params[i]->hw_params.hyst_asf_rnr_status =
            ((value & DYN_IMG_ASF_RNR_HYS) >> DYN_IMG_ASF_RNR_HYS_BIT);

          if(!previous_tnr_hyst_state &&
            session_params->stream_params[i]->hw_params.hyst_tnr_status){
            CPP_TNR_DBG("TNR switching on. Insert grey frame!");
            session_params->stream_params[i]->hw_params.grey_ref_enable = TRUE;
          }

          CPP_TNR_DBG("stream type %d value %llu, dsdn_status %d, tnr status %d "
            "pbf status %d",
            session_params->stream_params[i]->stream_type,
            value, session_params->stream_params[i]->hw_params.hyst_dsdn_status,
            session_params->stream_params[i]->hw_params.hyst_tnr_status,
            session_params->stream_params[i]->hw_params.hyst_pbf_status);

          PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));
          break;
        case CAM_STREAM_TYPE_OFFLINE_PROC:
          // Offline handled through stream params, not an error case
          break;
        default:
          CPP_TNR_LOW("HYSTERSIS INFO not supported for stream %d",
            session_params->stream_params[i]->stream_type);
          break;
        }
      }
    }
  }
end:
  return rc;
}
int32_t cpp_module_request_pproc_divert_info(mct_module_t* module,
  mct_event_t *event)
{
  pp_buf_divert_request_t *divert_request = NULL;
  pp_buf_divert_request_t cpp_divert_request;
  cpp_module_ctrl_t           *ctrl = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_stream_params_t  *stream_params = NULL;
  int rc = 0;

  if (!module || !event) {
    CPP_ERR("failed %p %p\n", module, event);
    rc = -EFAULT;
    goto end;
  }

  ctrl = (cpp_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_ERR("invalid cpp control, failed");
    rc = -EFAULT;
    goto end;
  }

  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params || !session_params) {
    CPP_ERR("failed , stream_params %p , session_params %p\n",
      stream_params, session_params);
    rc = -EFAULT;
    goto end;
  }
  memset(&cpp_divert_request, 0, sizeof(cpp_divert_request));

  divert_request =
    (pp_buf_divert_request_t *)(event->u.module_event.module_event_data);
  divert_request->need_divert = TRUE;
  if (stream_params->hw_params.tnr_enable &&
    stream_params->stream_info->streaming_mode ==
    CAM_STREAMING_MODE_BURST) {
    divert_request->force_streaming_mode = CAM_STREAMING_MODE_CONTINUOUS;
  }

  switch (stream_params->stream_type) {
    case CAM_STREAM_TYPE_OFFLINE_PROC:
    case CAM_STREAM_TYPE_SNAPSHOT:
      //TODO: Do we need to handle callback stream
      if (session_params->hal_version != CAM_HAL_V3) {
        divert_request->min_addr_offset_h = mct_util_calculate_lcm(
          divert_request->min_addr_offset_h, ctrl->cpphw->max_supported_padding);
        divert_request->min_addr_offset_v = mct_util_calculate_lcm(
          divert_request->min_addr_offset_v, ctrl->cpphw->max_supported_padding);
      } else {
        divert_request->min_addr_offset_h = 0;
        divert_request->min_addr_offset_v = 0;
      }
      CPP_HIGH("stream_type %d, offset h %d, offset v %d",
        stream_params->stream_type,
        divert_request->min_addr_offset_h,
        divert_request->min_addr_offset_v);
      break;
    default:
      break;
  }
  if (stream_params->enable_post_cpp) {
    CPP_LOW("Sending cpp_module_request_pproc_divert_info  id %x",
           event->identity);
    //Use cpp local copy to send event to downstream
    event->u.module_event.module_event_data = &cpp_divert_request;
    rc = cpp_module_send_event_downstream(module, event);
    if (rc < 0) {
      CPP_ERR("failed, module_event_type=%d, identity=0x%x",
        event->u.module_event.type, event->identity);
      return -EFAULT;
   }
   if (cpp_divert_request.num_additional_buffers > 0) {
     stream_params->native_buff =
       cpp_divert_request.num_additional_buffers;
   }
  }

  event->u.module_event.module_event_data = divert_request;

end:
  return rc;
}

/* cpp_module_handle_sensor_query_output_size:
 *
 * Update the stream parameter - output margin size value(s) based on the event
 * propagated from MCT -> VFE/ISP -> CPP.
 *
 * The VFE/ISP will make use of these value(s) in order to select appropriate
 * maximum size(s) to configure the sensor(s), to cater to the output frame size
 * requirements by all the subsequent modules for the given stream(s).
 *
 **/
int32_t cpp_module_handle_sensor_query_output_size(
  mct_module_t* module,
  mct_event_t *event)
{
  int32_t  rc = 0;
  uint32_t si = 0;
  sensor_isp_stream_sizes_t *sensor_isp_info = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cam_frame_margins_t sac_crop_margins = {0.0f, 0.0f};

  /* Validate module, event and stream parameters */
  if ((rc = cpp_module_util_check_event_params(
              module,
              event,
              &session_params,
              NULL))) {
    CPP_ERR("failed, module_event_type=%d, identity=0x%x",
      event->u.module_event.type,
      event->identity);
    return -EFAULT;
  }
  if (!session_params) {
    assert(session_params != NULL);
    CPP_ERR("Invalid Session:%p", session_params);
    return -EFAULT;
  }

  sensor_isp_info =
    (sensor_isp_stream_sizes_t *)(event->u.module_event.module_event_data);
  if (!sensor_isp_info) {
    assert(sensor_isp_info != NULL);
    CPP_ERR("invalid sensor stream info. %p, failed\n", sensor_isp_info);
    return -EFAULT;
  }

  rc = cpp_module_send_event_downstream(module, event);
  if (rc < 0) {
    CPP_ERR("failed, module_event_type=%d, identity=0x%x",
      event->u.module_event.type, event->identity);
    return -EFAULT;
  }

  /*
   * Cache the SAC crop margins into Session Parameters,
   * and apply them to realtime streams associated with.
   */
  cpp_module_util_get_sac_margins(&sac_crop_margins, session_params);

  /* Updated margins for all streams ouput buffers and event information. */
  for (si = 0; si < sensor_isp_info->num_streams; si++) {

    CPP_DBG("b4: sensor_isp_info->type[%d]=%d, "
      "sensor_isp_info->stream_sizes[%d]={w=%d,h=%d} "
      "sensor_isp_info->margins[%d]={w=%.2f%,h=%.2f%} "
      "sensor_isp_info->stream_sz_plus_margin[%d]={w=%d,h=%d} ",
      si,
      sensor_isp_info->type[si],
      si,
      sensor_isp_info->stream_sizes[si].width,
      sensor_isp_info->stream_sizes[si].height,
      si,
      sensor_isp_info->margins[si].widthMargins,
      sensor_isp_info->margins[si].heightMargins,
      si,
      sensor_isp_info->stream_sz_plus_margin[si].width,
      sensor_isp_info->stream_sz_plus_margin[si].height);

    cpp_module_util_apply_fixed_fov_margin(sensor_isp_info, si);
    cpp_module_util_apply_sac_margin(sensor_isp_info, si, &sac_crop_margins);
    cpp_module_util_apply_cpp_downscaler_margin(sensor_isp_info, si);

    CPP_DBG("l8r: sensor_isp_info->type[%d]=%d, "
      "sensor_isp_info->stream_sizes[%d]={w=%d,h=%d} "
      "sensor_isp_info->margins[%d]={w=%.2f%,h=%.2f%} "
      "sensor_isp_info->stream_sz_plus_margin[%d]={w=%d,h=%d} ",
      si,
      sensor_isp_info->type[si],
      si,
      sensor_isp_info->stream_sizes[si].width,
      sensor_isp_info->stream_sizes[si].height,
      si,
      sensor_isp_info->margins[si].widthMargins,
      sensor_isp_info->margins[si].heightMargins,
      si,
      sensor_isp_info->stream_sz_plus_margin[si].width,
      sensor_isp_info->stream_sz_plus_margin[si].height);

  }

  /*
   * TODO: When CPP native buffer support is added, update the buffer dimensions
   * based on downstream module's request.
   */

  end:
  return rc;
}

int32_t cpp_module_update_clk_obj_slave_session(cpp_module_ctrl_t* ctrl,
  cpp_module_session_params_t *session_params,
  cpp_module_stream_params_t *stream_params)
{
  int32_t i = 0;
  cpp_module_stream_clk_rate_t *clk_rate_obj = NULL;

  /*
   * Avoid updating slave in clock object for bayer-mono or
   * when both sessions need to be processed for tuning cases
   */
  if (cpp_module_util_is_slave_proc_required(session_params, stream_params))
    return 0;

  /* get clock object for all streams in session and
     update slave session in clock object */
  PTHREAD_MUTEX_LOCK(&(ctrl->clk_rate_list.mutex));
  for(i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
    if (session_params->stream_params[i]) {
      clk_rate_obj = cpp_module_find_clk_rate_by_identity(ctrl,
        session_params->stream_params[i]->identity);
      if (clk_rate_obj) {
        clk_rate_obj->slave_session = session_params->is_slave;
      }
    }
  }
  PTHREAD_MUTEX_UNLOCK(&(ctrl->clk_rate_list.mutex));
  return 0;
}

int32_t cpp_module_handle_master_info(mct_module_t* module,
  mct_event_t* event)
{
  cpp_module_ctrl_t* ctrl = NULL;
  cam_dual_camera_master_info_t  *master_info = NULL;
  cpp_module_stream_params_t *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_session_params_t *link_session_params = NULL;

  if (!module || !event) {
    CPP_ERR("failed, module=%p, event=%p\n",module,event);
    return -EFAULT;
  }

  ctrl = (cpp_module_ctrl_t*)MCT_OBJECT_PRIVATE(module);
  if (!ctrl) {
    CPP_ERR("Invalid cpp ctrl, failed\n");
   return -EFAULT;
  }

  master_info = (cam_dual_camera_master_info_t*)
    event->u.ctrl_event.control_event_data;
  if (!master_info) {
    CPP_ERR("Invalid master_info, failed\n");
    return -EFAULT;
  }

  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if (!session_params) {
    CPP_ERR("Invalid session param %p, failed\n",session_params);
    return -EINVAL;
  }
  cpp_module_get_params_for_identity(ctrl, session_params->link_session_id,
    &link_session_params, &stream_params);
  if (!link_session_params || !stream_params)  {
    CPP_ERR("Invalid link_session_params %p, or stream_params %p,  failed\n",
      link_session_params, stream_params);
    return -EINVAL;
  }

  /* Cache the mode into both params for both sessions. */
  PTHREAD_MUTEX_LOCK(&(ctrl->cpp_mutex));

  session_params->is_slave = master_info->mode;
  link_session_params->is_slave = !session_params->is_slave;

  CPP_HIGH("current session, slave : [%x, %d], linked session, slave [%x, %d]",
    event->identity, session_params->is_slave,
    session_params->link_session_id, link_session_params->is_slave);

  PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));

  cpp_module_update_clk_obj_slave_session(ctrl, session_params,
    stream_params);
  cpp_module_update_clk_obj_slave_session(ctrl, link_session_params,
    stream_params);

  /* trigger runtime update */
  PTHREAD_MUTEX_LOCK(&(ctrl->cpp_mutex));
  ctrl->runtime_clk_update = TRUE;
  PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));

  return 0;
}

int32_t cpp_module_handle_intra_link_session(mct_module_t* module,
  mct_event_t* event)
{
  cpp_module_ctrl_t* ctrl = NULL;
  cpp_module_stream_params_t *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;

  if (!module || !event) {
    CPP_ERR("failed, module=%p, event=%p\n", module, event);
    return -EFAULT;
  }

  ctrl = (cpp_module_ctrl_t*)MCT_OBJECT_PRIVATE(module);
  if (!ctrl) {
    CPP_ERR("Invalid cpp ctrl, failed\n");
   return -EFAULT;
  }

  cam_sync_related_sensors_event_info_t* linking_params =
    (cam_sync_related_sensors_event_info_t*)
    event->u.ctrl_event.control_event_data;

  int32_t link_identity = linking_params->related_sensor_session_id;

  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if (!session_params) {
    CPP_ERR("Invalid session param %p, failed", session_params);
    return -EINVAL;
  }

  PTHREAD_MUTEX_LOCK(&(ctrl->cpp_mutex));

  session_params->cam_type = linking_params->type;
  session_params->is_slave = linking_params->mode;
  session_params->link_session_id = link_identity;
  session_params->cam_role = linking_params->cam_role;

  CPP_HIGH("current session %x, linked session %x, current session mode %d",
    event->identity, link_identity, linking_params->mode);

  PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));

  return 0;
}
