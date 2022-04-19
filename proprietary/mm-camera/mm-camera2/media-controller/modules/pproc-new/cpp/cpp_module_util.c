/*============================================================================

  Copyright (c) 2013, 2016, 2017, 2018-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include "eztune_diagnostics.h"
#include "cpp_port.h"
#include "cpp_module.h"
#include "camera_dbg.h"
#include "cpp_log.h"

#define CPP_UPSCALE_THRESHOLD(ptr) \
   ((chromatix_cpp_type *)ptr)->up_scale_threshold
#define CPP_DOWNSCALE_THRESHOLD(ptr) \
   ((chromatix_cpp_type *)ptr)->down_scale_threshold

/* cpp_module_util_decide_divert_by_identity:
 *   @stream_params - pointer to the stream.
 *
 * this function decide if need divert
 * return divert idenetiy, if no need to divert, return 0
 **/
uint32_t cpp_module_util_decide_divert_id(mct_module_t *module,
  cpp_module_stream_params_t *stream_params)
{
  mct_module_type_t     mod_type = MCT_MODULE_FLAG_INVALID;
  uint32_t              div_identity = 0;

  mod_type = mct_module_find_type(module, stream_params->identity);

  if ( stream_params->native_buff > 0 ) {
    return stream_params->identity;
  }

  if (((mod_type == MCT_MODULE_FLAG_SOURCE) ||
      (mod_type == MCT_MODULE_FLAG_INDEXABLE)) &&
      (stream_params->req_frame_divert ||
      (stream_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC))) {
    return stream_params->identity;
  }

  return 0;
}
/* cpp_module_util_check_duplicate:
 *   @stream_params - pointer to the stream.
 *
 * this function decide if two stream are duplicate.
 * Disables duplication when batch mode is set.
 **/
boolean cpp_module_util_check_duplicate(
  cpp_module_stream_params_t *stream_params1,
  cpp_module_stream_params_t *stream_params2,
  uint32_t version)
{

  cpp_hardware_params_t        *hw_params1;
  cpp_hardware_params_t        *hw_params2;
  cam_streaming_mode_t  stream1_streaming_mode,stream2_streaming_mode;

  if (stream_params1 == NULL || stream_params2 == NULL) {
    CPP_ERR("null pointer ! stream_params1 = %p, stream_params2 = %p\n",
      stream_params1, stream_params2);
    return FALSE;
  }

  stream1_streaming_mode = stream_params1->stream_info->streaming_mode;
  stream2_streaming_mode = stream_params2->stream_info->streaming_mode;
  if (((stream1_streaming_mode == CAM_STREAMING_MODE_BATCH) ||
    (stream2_streaming_mode == CAM_STREAMING_MODE_BATCH)) &&
    (version < CPP_HW_VERSION_6_0_0)) {
    CPP_HIGH("Batch mode ON, duplication disabled for hw version %d", version);
    return FALSE;
  }

  if (cpp_module_util_is_cpp_proc_need(NULL, stream_params1, stream_params2)
       == TRUE) {
     return FALSE;
  }

  hw_params1 = &stream_params1->hw_params;
  hw_params2 = &stream_params2->hw_params;
  if(hw_params1->output_info.width == hw_params2->output_info.width &&
     hw_params1->output_info.height == hw_params2->output_info.height &&
     hw_params1->output_info.stride == hw_params2->output_info.stride &&
     hw_params1->output_info.scanline == hw_params2->output_info.scanline &&
     hw_params1->rotation == hw_params2->rotation &&
     hw_params1->mirror== hw_params2->mirror &&
     hw_params1->output_info.plane_fmt == hw_params2->output_info.plane_fmt) {
    /* make the linked streams duplicates of each other */
    CPP_HIGH("linked streams formats match: output duplication enabled\n");
    return TRUE;
  }
  return FALSE;
}

/* cpp_module_util_decide_proc_frame_per_stream:
 *   @stream_params - pointer to the stream.
 *
 *   This function is called in list traverse. It make a sum of the loads of
 *   all streams that are on.
 **/
boolean cpp_module_util_decide_proc_frame_per_stream(mct_module_t *module,
  cpp_module_session_params_t *session_params,
  cpp_module_stream_params_t *stream_params, uint32_t frame_id)
{
  boolean  is_process_stream = FALSE;
  boolean  is_sw_skip = FALSE;
  uint32_t is_frame_ready = 0;
  cpp_module_ctrl_t* ctrl;

  if (stream_params == NULL || session_params == NULL) {
    CPP_ERR(" stream param = %p session_param = %p", stream_params, session_params);
    return FALSE;
  }

  if (stream_params->is_stream_on == TRUE)  {
    cpp_module_get_and_update_buf_index(session_params,
      stream_params, frame_id);
    is_sw_skip =
      cpp_module_check_frame_skip(stream_params, session_params, frame_id);
    if (is_sw_skip == TRUE) {
      /*if sw skip is on for this frame, skip process*/
      return FALSE;
    }

   if ((
     /* if not offline stream */
     stream_params->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) &&
     /* if not EIS 3.0 on video */
    !((stream_params->stream_type == CAM_STREAM_TYPE_VIDEO) &&
    (stream_params->stream_info->is_type == IS_TYPE_EIS_3_0)) &&
    /* slave session & not  bayer- mono and force slave processing is not requested*/
    ((session_params->is_slave == TRUE) &&
    !cpp_module_util_is_slave_proc_required(session_params, stream_params))) {
    /*if HAL3, need to check if HAL queue the buffer or not*/
    if (session_params->hal_version == CAM_HAL_V3 &&
        cpp_module_get_frame_valid(module, stream_params->identity,
        frame_id, PPROC_GET_STREAM_ID(stream_params->identity),
        stream_params->stream_type, false) == 1) {
        CPP_HIGH("Send piggy ack is not required, need to process this frame\n");
    } else {
      CPP_HIGH("Send piggy ack %d %x \n", frame_id,stream_params->identity);
      return FALSE;
    }
  }

    is_process_stream = TRUE;

    /*if HAL3, need to check if HAL queue the buffer or not*/
    if (session_params->hal_version == CAM_HAL_V3 &&
        cpp_module_get_frame_valid(module, stream_params->identity,
        frame_id, PPROC_GET_STREAM_ID(stream_params->identity),
        stream_params->stream_type, true) == 0) {
        is_process_stream = FALSE;
    }
  } else {
    /*if stream is not on then no process stream*/
    is_process_stream = FALSE;
  }

  ctrl = (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_ERR("invalid cpp control, failed\n");
    return FALSE;
  }

  if (session_params->hal_version == CAM_HAL_V3 && ctrl->rtb_status) {
    is_process_stream = TRUE;
  }

  return is_process_stream;
}

/* cpp_module_util_update_stream_frame_id:
 *   @stream_params - pointer to the stream.
 *
 *   This function is called in list traverse. It make a sum of the loads of
 *   all streams that are on.
 **/
void cpp_module_util_update_stream_frame_id(
  cpp_module_stream_params_t *stream_params, uint32_t frame_id)
{
  uint32_t i = 0;
  cpp_module_stream_params_t *linked_stream_params = NULL;
  if (stream_params == NULL) {
    CPP_ERR("stream param = %p", stream_params);
    return;
  }

  if (stream_params->is_stream_on) {
    stream_params->cur_frame_id = frame_id;
  }
  for (i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
    linked_stream_params = stream_params->linked_streams[i];
    if ((linked_stream_params != NULL) && (linked_stream_params->identity) &&
      (linked_stream_params->is_stream_on)) {
       linked_stream_params->cur_frame_id = frame_id;
    }
  }

  return;
}

/* cpp_module_util_check_link_streamon:
 *   @stream_params - pointer to the stream.
 *
 *   This function is called in list traverse. It make a sum of the loads of
 *   all streams that are on.
 **/
boolean cpp_module_util_check_link_streamon(cpp_module_stream_params_t *stream_params)
{

  uint32_t i = 0;
  cpp_module_stream_params_t *linked_stream_params = NULL;

  if (stream_params == NULL) {
    CPP_ERR(" stream param = %p", stream_params);
    return FALSE;
  }

  if (stream_params->is_stream_on == TRUE) {
    return TRUE;
  }

  for (i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
    linked_stream_params = stream_params->linked_streams[i];
    if (linked_stream_params != NULL && linked_stream_params->identity) {
      if (linked_stream_params->is_stream_on == TRUE) {
        return TRUE;
      }
    }
  }

  return FALSE;
}

/*cpp_module_dump_stream_param:
 *   @stream_params - pointer to the stream.
 *
 *   This function is called in list traverse. It make a sum of the loads of
 *   all streams that are on.
 **/
void cpp_module_dump_stream_param(cpp_module_stream_params_t *stream_params)
{
  if (stream_params == NULL) {
    CPP_ERR(" stream param = %p", stream_params);
    return;
  }

  CPP_HIGH("linked stream w=%d, h=%d, st=%d, sc=%d, fmt=%d, identity=0x%x",
    stream_params->hw_params.output_info.width,
    stream_params->hw_params.output_info.height,
    stream_params->hw_params.output_info.stride,
    stream_params->hw_params.output_info.scanline,
    stream_params->hw_params.output_info.plane_fmt,
    stream_params->identity);
}

/* cpp_module_dump_all_linked_stream_info:
 *   @stream_params - pointer to the stream.
 *
 *   This function is called in list traverse. It make a sum of the loads of
 *   all streams that are on.
 **/
void cpp_module_dump_all_linked_stream_info(
  cpp_module_stream_params_t *stream_params)
{
  int i = 0;
  cpp_module_stream_params_t *linked_stream_params = NULL;

  if (stream_params == NULL) {
    CPP_ERR(" stream param = %p", stream_params);
    return;
  }

  for (i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
    linked_stream_params = stream_params->linked_streams[i];
    if (linked_stream_params != NULL && linked_stream_params->identity) {
      cpp_module_dump_stream_param(linked_stream_params);
    }
  }
}

static boolean find_port_with_identity_find_func(void *data, void *user_data)
{
  if(!data || !user_data) {
    CPP_ERR("failed, data=%p, user_data=%p\n", data, user_data);
    return FALSE;
  }
  mct_port_t *port = (mct_port_t*) data;
  uint32_t identity = *(uint32_t*) user_data;

  cpp_port_data_t *port_data = (cpp_port_data_t *) MCT_OBJECT_PRIVATE(port);
  uint32_t i;
  for(i=0; i<CPP_MAX_STREAMS_PER_PORT; i++) {
    if(port_data->stream_data[i].port_state != CPP_PORT_STATE_UNRESERVED &&
        port_data->stream_data[i].identity == identity) {
      return TRUE;
    }
  }
  return FALSE;
}

mct_port_t* cpp_module_find_port_with_identity(mct_module_t *module,
  mct_port_direction_t dir, uint32_t identity)
{
  mct_port_t *port = NULL;
  mct_list_t *templist;
  switch(dir) {
  case MCT_PORT_SRC:
    templist = mct_list_find_custom(
       MCT_MODULE_SRCPORTS(module), &identity,
        find_port_with_identity_find_func);
    if(templist) {
        port = (mct_port_t*)(templist->data);
    }
    break;
  case MCT_PORT_SINK:
    templist = mct_list_find_custom(
       MCT_MODULE_SINKPORTS(module), &identity,
        find_port_with_identity_find_func);
    if(templist) {
      port = (mct_port_t*)(templist->data);
    }
    break;
  default:
    CPP_ERR("failed, bad port_direction=%d", dir);
    return NULL;
  }
  return port;
}

boolean ack_find_func(void* data, void* userdata)
{
  if(!data || !userdata) {
    CPP_ERR("failed, data=%p, userdata=%p\n", data, userdata);
    return FALSE;
  }
  cpp_module_ack_t* cpp_ack = (cpp_module_ack_t*) data;
  cpp_module_ack_key_t* key = (cpp_module_ack_key_t*) userdata;
  if(cpp_ack->isp_buf_divert_ack.identity == key->identity &&
     cpp_ack->isp_buf_divert_ack.buf_idx == key->buf_idx) {
    return TRUE;
  }
  return FALSE;
}

cpp_module_ack_t* cpp_module_find_ack_from_list(cpp_module_ctrl_t *ctrl,
  cpp_module_ack_key_t key)
{
  mct_list_t *templist;
  templist = mct_list_find_custom(ctrl->ack_list.list, &key, ack_find_func);
  if(templist) {
    return (cpp_module_ack_t*)(templist->data);
  }
  return NULL;
}

static
boolean clk_rate_find_by_identity_func(void* data, void* userdata)
{
  if(!data || !userdata) {
    CPP_ERR("failed, data=%p, userdata=%p\n", data, userdata);
    return FALSE;
  }

  cpp_module_stream_clk_rate_t *clk_rate_obj =
    (cpp_module_stream_clk_rate_t*) data;
  uint32_t identity = *(uint32_t*)userdata;

  if(clk_rate_obj->identity == identity) {
    return TRUE;
  }
  return FALSE;
}

cpp_module_stream_clk_rate_t *
cpp_module_find_clk_rate_by_identity(cpp_module_ctrl_t *ctrl,
  uint32_t identity)
{
  mct_list_t *templist;

  templist = mct_list_find_custom(ctrl->clk_rate_list.list, &identity,
    clk_rate_find_by_identity_func);
  if(templist) {
    return (cpp_module_stream_clk_rate_t *)(templist->data);
  }
  return NULL;
}

static
boolean clk_rate_find_by_value_func(void* data, void* userdata)
{
  if(!data) {
    CPP_ERR("failed, data=%p\n", data);
    return FALSE;
  }
  cpp_module_stream_clk_rate_t *curent_clk_obj =
    (cpp_module_stream_clk_rate_t *) data;
  cpp_module_stream_clk_rate_t **clk_obj =
    (cpp_module_stream_clk_rate_t **)userdata;

  if (NULL == *clk_obj) {
    *clk_obj = curent_clk_obj;
    return TRUE;
  }

  if (((cpp_module_stream_clk_rate_t *)(*clk_obj))->total_load <
    curent_clk_obj->total_load) {
    *clk_obj = curent_clk_obj;
  }
  return TRUE;
}

cpp_module_stream_clk_rate_t *
cpp_module_find_clk_rate_by_value(cpp_module_ctrl_t *ctrl)
{
  cpp_module_stream_clk_rate_t *clk_obj = NULL;
  int32_t rc;

  rc = mct_list_traverse(ctrl->clk_rate_list.list,
    clk_rate_find_by_value_func, &clk_obj);

  if (!rc) {
    return NULL;
  }
  if (clk_obj) {
    return (clk_obj);
  }
  return NULL;
}

/* cpp_module_clk_rate_find_by_value:
 *   @data - the member of the list
 *   @userdata - pointer to the variable that collects the result.
 *
 *   This function is called in list traverse. It make a sum of the loads of
 *   all streams that are on.
 **/
static
boolean cpp_module_clk_rate_find_by_value(void* data, void* userdata)
{
  uint64_t current_clk_obj_load = 0, duplication_load = 0;
  cpp_module_stream_clk_rate_t *curent_clk_obj = NULL;
  cpp_module_total_load_t *current_load = NULL;
  uint64_t input_bw = 0;
  uint64_t output_bw = 0;
  uint64_t output_count = 0;


  if(!data || !userdata) {
    CPP_ERR("failed, data=%p, userdata %p\n", data, userdata);
    return FALSE;
  }

  curent_clk_obj = (cpp_module_stream_clk_rate_t *) data;
  current_load = (cpp_module_total_load_t *)userdata;

 CPP_CLOCK_LOW("identity  %x, delay %d, force %d, excess %d",
    curent_clk_obj->identity, curent_clk_obj->process_delay,
    current_load->force, curent_clk_obj->excess_load_per_frame);

  /*
   * Check if need to skip a streams load.
   * skip if force is not set and process delay is zero
   */
  if (IS_SKIP_STREAM_LOAD(curent_clk_obj, current_load)) {
    CPP_CLOCK_HIGH("skip for %x, delay %d, force %d, slave %d",
      curent_clk_obj->identity, curent_clk_obj->process_delay,
      current_load->force, curent_clk_obj->slave_session);
    goto end;
  }

  /* The flag is set if the duplicate output is set and
     if the linked stream is also on. */
  if (curent_clk_obj->config_flags & 0x1) {
    current_clk_obj_load = (float)(curent_clk_obj->total_load / 2);
    duplication_load += (float) ((current_clk_obj_load +
      (curent_clk_obj->excess_load_per_frame / 2)) *
      curent_clk_obj->system_overhead);
    input_bw = current_clk_obj_load +
      (curent_clk_obj->excess_load_per_frame / 2);
    output_bw = (float) ((current_clk_obj_load +
      (curent_clk_obj->excess_load_per_frame / 2)) *
      curent_clk_obj->system_overhead);
  }else if(((curent_clk_obj->config_flags & SHARE_FLAG) >> SHARED_FLAG_SHIFT)){
    if (curent_clk_obj->shared_output_num > 1) {
      output_count = curent_clk_obj->shared_output_num;
      CPP_CLOCK_LOW("calculating load for %p %d",curent_clk_obj,output_count);
      current_clk_obj_load = (float)(curent_clk_obj->total_load / output_count);
      duplication_load += (float) ((current_clk_obj_load +
        (curent_clk_obj->excess_load_per_frame / output_count )) *
        curent_clk_obj->system_overhead);
    }
  } else {
    current_clk_obj_load = curent_clk_obj->total_load;
    input_bw = curent_clk_obj->input_load +
      curent_clk_obj->excess_load_per_frame;
    output_bw = (float) ((curent_clk_obj->output_load +
      curent_clk_obj->excess_load_per_frame) *
        curent_clk_obj->system_overhead);
  }

  CPP_CLOCK_LOW("iden %x dup %d, tot %lld, load %lld, excess %lld",
    curent_clk_obj->identity, (curent_clk_obj->config_flags & 0x1),
    curent_clk_obj->total_load, current_clk_obj_load,
    curent_clk_obj->excess_load_per_frame);

  if (curent_clk_obj->config_flags & 0x2) {
    current_load->input_ref_load += input_bw;
    current_load->output_ref_load += output_bw;
  }

  /*
   * If CPP cds (dsdn) is on only chroma is fetched.
   * Account for input load.for reference load.
   */
  if (curent_clk_obj->config_flags & 0x4) {
    current_load->input_ref_load +=  input_bw;
  }

  if (curent_clk_obj->config_flags & 0x8) {
    output_bw /= COMP_RATIO_NRT;
    duplication_load /= COMP_RATIO_NRT;
  }

  current_load->duplication_load += duplication_load;
  current_load->input_bw += input_bw;
  current_load->output_bw += output_bw;
  current_load->clk += ((float)current_clk_obj_load *
    curent_clk_obj->system_overhead);
  current_load->perf_mode |= curent_clk_obj->perf_mode;

  CPP_CLOCK_LOW("dup %lld, ip %lld, op %lld cl %lld, process_delay %d",
    current_load->duplication_load, current_load->input_bw,
    current_load->output_bw, current_load->clk,
    curent_clk_obj->process_delay);

  if (current_load->process_delay <= curent_clk_obj->process_delay)
    current_load->process_delay = curent_clk_obj->process_delay;
end:
  return TRUE;
}

/* cpp_module_get_total_load_by_value:
 *   @ctrl - odule's control data
 *   @current_load - pointer to the variable that keeps the sum of all loads
 *   of the streams.
 *
 *   This function traverses through the list of streams loads.
 **/
int32_t cpp_module_get_total_load_by_value(cpp_module_ctrl_t *ctrl,
  cpp_module_total_load_t *current_load)
{
  int32_t rc = 0;

  rc = mct_list_traverse(ctrl->clk_rate_list.list,
    cpp_module_clk_rate_find_by_value, current_load);

  return rc;
}

cam_streaming_mode_t cpp_module_get_streaming_mode(mct_module_t *module,
  uint32_t identity)
{
  if (!module) {
    CPP_ERR("invalid module, failed\n");
    return -EINVAL;
  }
  mct_port_t* port = cpp_module_find_port_with_identity(module, MCT_PORT_SINK,
                       identity);
  if (!port) {
    CPP_ERR("port not found, identity=0x%x\n", identity);
    return -EINVAL;
  }
  cpp_port_data_t* port_data = (cpp_port_data_t*) MCT_OBJECT_PRIVATE(port);
  uint32_t i;
  for (i=0; i<CPP_MAX_STREAMS_PER_PORT; i++) {
    if (port_data->stream_data[i].identity == identity) {
      CPP_DBG("identity 0x%x, streaming_mode %d", identity,
        port_data->stream_data[i].streaming_mode);
      return port_data->stream_data[i].streaming_mode;
    }
  }
  return CAM_STREAMING_MODE_MAX;
}

int32_t cpp_module_get_params_for_identity(cpp_module_ctrl_t* ctrl,
  uint32_t identity, cpp_module_session_params_t** session_params,
  cpp_module_stream_params_t** stream_params)
{
  if(!ctrl || !session_params || !stream_params) {
    CPP_DBG("failed, ctrl=%p, session_params=%p, stream_params=%p",
      ctrl, session_params, stream_params);
    return -EINVAL;
  }
  uint32_t session_id;
  uint32_t i,j;
  boolean success = FALSE;
  session_id = CPP_GET_SESSION_ID(identity);
  for(i=0; i < CPP_MODULE_MAX_SESSIONS; i++) {
    if(ctrl->session_params[i]) {
      if(ctrl->session_params[i]->session_id == session_id) {
        *session_params = ctrl->session_params[i];
        for(j=0; j < CPP_MODULE_MAX_STREAMS; j++) {
          if(ctrl->session_params[i]->stream_params[j]) {
            if(ctrl->session_params[i]->stream_params[j]->identity ==
                identity) {
              *stream_params = ctrl->session_params[i]->stream_params[j];
              success = TRUE;
              break;
            }
          }
        }
      }
    }
    if(success == TRUE) {
      break;
    }
  }
  if(success == FALSE) {
    CPP_HIGH("failed, identity=0x%x", identity);
    return -EFAULT;
  }
  return 0;
}

int32_t cpp_module_get_params_for_streamtype(
  cam_stream_type_t stream_type,
  cpp_module_stream_params_t** stream_params)
{
  cpp_module_stream_params_t *tstream_params = NULL;
  cpp_module_stream_params_t *lstream_params = NULL;
  boolean success = FALSE;
  uint32_t i = 0;
  int32_t ret;

  tstream_params = *stream_params;
  if (tstream_params->stream_type != stream_type) {
    for (i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
      lstream_params = tstream_params->linked_streams[i];
      if (lstream_params && lstream_params->stream_type == stream_type)
        *stream_params = lstream_params;
        success = TRUE;
        break;
      }
    if(success == FALSE) {
      CPP_HIGH("stream params for type %d not found", stream_type);
      return -EINVAL;
    }
  }
  return 0;
}

int32_t cpp_module_util_update_clock_rate(cpp_module_ctrl_t* ctrl,
  cpp_module_session_params_t  *session_params __unused,
  cam_stream_type_t stream_type,
  cpp_module_stream_params_t **s_params)
{
  cpp_module_stream_clk_rate_t *clk_rate_obj = NULL;

  cpp_module_stream_params_t *stream_params = NULL;
  /* get stream parameters based on stream type */
  cpp_module_get_params_for_streamtype(
    stream_type, s_params);

  stream_params = *s_params;
  CPP_CLOCK_DBG("track_frame_done %d, turbo_frameproc_count %d",
    stream_params->track_frame_done, stream_params->turbo_frameproc_count);
  //update clock after 'n'(or first) frame done
  if (stream_params->track_frame_done &&
    stream_params->turbo_frameproc_count && stream_params->is_stream_on) {
    /* get stream clk rate based on the stream identity */
    clk_rate_obj = cpp_module_find_clk_rate_by_identity(ctrl,
      stream_params->identity);
    if (clk_rate_obj == NULL) {
      CPP_ERR(" clk rate obj = NULL, identity = %x",
        stream_params->identity);
      return -EINVAL;
    }
    stream_params->turbo_frameproc_count--;
    if (!stream_params->turbo_frameproc_count) {
      PTHREAD_MUTEX_LOCK(&(ctrl->clk_rate_list.mutex));
      stream_params->track_frame_done = FALSE;
      clk_rate_obj->perf_mode = CAM_PERF_NORMAL;
      CPP_CLOCK_HIGH("type %d track_frame_done %d clk_update %d perf_mode %d",
        stream_params->stream_type, stream_params->track_frame_done,
        ctrl->runtime_clk_update, clk_rate_obj->perf_mode);
      PTHREAD_MUTEX_UNLOCK(&(ctrl->clk_rate_list.mutex));

      PTHREAD_MUTEX_LOCK(&(ctrl->cpp_mutex));
      ctrl->runtime_clk_update = TRUE;
      cpp_module_update_clock_status(ctrl, CPP_CLOCK_BUMP_DOWN);
      PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
    }
  }
  return 0;
}


int32_t cpp_module_get_params_for_session_id(cpp_module_ctrl_t* ctrl,
  uint32_t session_id, cpp_module_session_params_t** session_params)
{
  uint32_t i = 0;
  if (!ctrl || !session_params) {
    CPP_DBG("failed, ctrl=%p, session_params=%p\n",
      ctrl, session_params);
    return -EINVAL;
  }
  *session_params = NULL;
  for (i = 0; i < CPP_MODULE_MAX_SESSIONS; i++) {
    if (ctrl->session_params[i]) {
      if (ctrl->session_params[i]->session_id == session_id) {
        *session_params = ctrl->session_params[i];
        break;
      }
    }
  }
  if(!(*session_params)) {
    CPP_ERR("failed, session_id=0x%x", session_id);
    return -EFAULT;
  }
  return 0;
}

void cpp_module_dump_stream_params(cpp_module_stream_params_t *stream_params __unused,
  const char* func __unused, uint32_t line __unused)
{
  CPP_LOW("%s:%d:---------- Dumping stream params %p ------------", func, line, stream_params);
  if(!stream_params) {
    CPP_DBG("%s:%d: failed", func, line);
    return;
  }
  CPP_LOW("%s:%d: stream_params.identity=0x%x", func, line, stream_params->identity);
  CPP_LOW("%s:%d: ---------------------------------------------------------", func, line);
}

boolean cpp_module_util_map_buffer_info(void *d1, void *d2)
{
  mct_stream_map_buf_t          *img_buf = (mct_stream_map_buf_t *)d1;
  cpp_module_stream_buff_info_t *stream_buff_info =
    (cpp_module_stream_buff_info_t *)d2;
  cpp_module_buffer_info_t      *buffer_info;
  mct_list_t                    *list_entry = NULL;

  if (!img_buf || !stream_buff_info) {
    CPP_ERR("failed img_buf=%p stream_buff_info=%p", img_buf, stream_buff_info);
    return FALSE;
  }

  if ((img_buf->buf_type == CAM_MAPPING_BUF_TYPE_OFFLINE_META_BUF) ||
      (img_buf->buf_type == CAM_MAPPING_BUF_TYPE_MISC_BUF)) {
    return TRUE;
  }

  if (!stream_buff_info->delete_buf &&
    (img_buf->buf_type == CAM_MAPPING_BUF_TYPE_OFFLINE_INPUT_BUF)) {
    return TRUE;
  }

  buffer_info = CAM_MALLOC(sizeof(cpp_module_buffer_info_t));
  if (NULL == buffer_info) {
    CPP_ERR("CAM_MALLOC() failed\n");
    return FALSE;
  }

  memset((void *)buffer_info, 0, sizeof(cpp_module_buffer_info_t));

  if ((img_buf->common_fd == TRUE)||(img_buf->num_planes== 1)) {
    buffer_info->fd = img_buf->buf_planes[0].fd;
    buffer_info->index = img_buf->buf_index;
    /* Need to get this information from stream info stored in module.
       But because the structure is reused for all buffer operation viz.
       (Enqueue stream buffer list / process frame) the below fields can be
       set to default */
    buffer_info->offset = 0;;
    buffer_info->native_buff =
      (img_buf->buf_type == CAM_MAPPING_BUF_TYPE_OFFLINE_INPUT_BUF);
    buffer_info->processed_divert = FALSE;
  } else {
    CPP_ERR("error in supporting multiple planar FD\n");
    free(buffer_info);
    return FALSE;
  }

  list_entry = mct_list_append(stream_buff_info->buff_list,
    buffer_info, NULL, NULL);
  if (NULL == list_entry) {
    CPP_ERR("Error appending node\n");
    free(buffer_info);
    return FALSE;
  }

  stream_buff_info->buff_list = list_entry;
  stream_buff_info->num_buffs++;
  return TRUE;
}

boolean cpp_module_util_free_buffer_info(void *d1, void *d2)
{
  cpp_module_buffer_info_t      *buffer_info =
    (cpp_module_buffer_info_t *)d1;
  cpp_module_stream_buff_info_t *stream_buff_info =
    (cpp_module_stream_buff_info_t *)d2;

  if (!buffer_info || !stream_buff_info) {
    CPP_ERR("error buffer_info:%p stream_buff_info:%p\n",
      buffer_info, stream_buff_info);
    return FALSE;
  }

  if (stream_buff_info->num_buffs == 0) {
    CPP_ERR("error in num of buffs\n");
    return FALSE;
  }

  free(buffer_info);
  stream_buff_info->num_buffs--;
  return TRUE;
}

boolean cpp_module_util_create_hw_stream_buff(void *d1, void *d2)
{
  cpp_module_buffer_info_t        *buffer_info =
    (cpp_module_buffer_info_t *)d1;
  cpp_hardware_stream_buff_info_t *hw_strm_buff_info =
    (cpp_hardware_stream_buff_info_t *)d2;
  uint32_t num_buffs;

  if (!buffer_info || !hw_strm_buff_info) {
    CPP_ERR("error buffer_info:%p hw_strm_buff_info:%p\n",
      buffer_info, hw_strm_buff_info);
    return FALSE;
  }

  /* We make an assumption that a linera array will be provided */
  num_buffs = hw_strm_buff_info->num_buffs;
  hw_strm_buff_info->buffer_info[num_buffs].fd =
    (unsigned long)buffer_info->fd;
  hw_strm_buff_info->buffer_info[num_buffs].index = buffer_info->index;
  hw_strm_buff_info->buffer_info[num_buffs].offset = buffer_info->offset;
  hw_strm_buff_info->buffer_info[num_buffs].native_buff =
    buffer_info->native_buff;
  hw_strm_buff_info->buffer_info[num_buffs].processed_divert =
    buffer_info->processed_divert;
  hw_strm_buff_info->buffer_info[num_buffs].identity =
    hw_strm_buff_info->identity;

  hw_strm_buff_info->num_buffs++;
  return TRUE;
}

/* cpp_module_invalidate_q_traverse_func:
 *
 * Invalidates queue entry and adds ack_key in key_list base on identity.
 *
 **/
boolean cpp_module_invalidate_q_traverse_func(void* qdata, void* userdata)
{
  if (!qdata || !userdata) {
    CPP_ERR("failed, qdata=%p input=%p\n", qdata, userdata);
    return FALSE;
  }
  void** input = (void**)userdata;
  cpp_module_event_t* cpp_event = (cpp_module_event_t *) qdata;
  cpp_module_ctrl_t*  ctrl = (cpp_module_ctrl_t *) input[0];
  uint32_t identity = *(uint32_t*) input[1];
  mct_list_t **key_list = (mct_list_t **) input[2];
  uint8_t do_ack = FALSE;

  if(!ctrl) {
    CPP_ERR("failed, ivalid ctrl\n");
    return FALSE;
  }
  /* invalidate the event and add key in key list */
  if(cpp_event->type != CPP_MODULE_EVENT_CLOCK) {
    if ((cpp_event->type != CPP_MODULE_EVENT_PARTIAL_FRAME) &&
      (cpp_event->ack_key.identity == identity)) {
      cpp_event->invalid = TRUE;
      do_ack = TRUE;
    } else if ((cpp_event->type == CPP_MODULE_EVENT_PARTIAL_FRAME) &&
      (cpp_event->u.partial_frame.frame->identity == identity)) {
      cpp_event->invalid = TRUE;
    }
    if (cpp_event->invalid == TRUE && do_ack) {
      cpp_module_ack_key_t *key =
        (cpp_module_ack_key_t *) CAM_MALLOC (sizeof(cpp_module_ack_key_t));
      if(!key) {
        CPP_ERR("failed, invalid key\n");
        return FALSE;
      }
      memcpy(key, &(cpp_event->ack_key), sizeof(cpp_module_ack_key_t));
      *key_list = mct_list_append(*key_list, key, NULL, NULL);
    }
  }
  return TRUE;
}


/* cpp_module_release_ack_traverse_func:
 *
 * traverses through the list of keys and updates ACK corresponding to the
 * key.
 *
 **/
boolean cpp_module_release_ack_traverse_func(void* data, void* userdata)
{
  int32_t rc;
  if (!data || !userdata) {
    CPP_ERR("failed, data=%p userdata=%p\n", data, userdata);
    return FALSE;
  }
  cpp_module_ack_key_t* key = (cpp_module_ack_key_t *) data;
  cpp_module_ctrl_t*  ctrl = (cpp_module_ctrl_t *) userdata;
  rc = cpp_module_do_ack(ctrl, *key, 0);
  if(rc < 0) {
    CPP_ERR("failed, identity=0x%x\n", key->identity);
      return FALSE;
  }
  return TRUE;
}

/* cpp_module_key_list_free_traverse_func:
 *
 * traverses through the list of keys and frees the data.
 *
 **/
boolean cpp_module_key_list_free_traverse_func(void* data, void* userdata __unused)
{
  cpp_module_ack_key_t* key = (cpp_module_ack_key_t *) data;
  free(key);
  return TRUE;
}

/** cpp_module_update_hfr_skip:
 *
 *  Description:
 *    Based on input and output fps, calculte the skip count
 *    according to this formula,
 *      count = floor(input/output) - 1, if input > output
 *            = 0, otherwise
 *
 **/
int32_t cpp_module_update_hfr_skip(cpp_module_stream_params_t *stream_params)
{
  uint8_t cal_batchsize, linked_stream_batchsize = 0;
  cpp_module_stream_params_t *linked_stream_params = NULL;
  uint32_t i = 0;

  if(!stream_params) {
    CPP_ERR("invalid stream params, failed");
    return -EINVAL;
  }

  stream_params->hfr_skip_info.skip_count =
    floor(stream_params->hfr_skip_info.input_fps /
          stream_params->hfr_skip_info.output_fps) - 1;

  /* For preview, If streams are bundled and the linked stream is a batch ,
   * then calculate skip count
   */

  /* support only one batch pattern in one port,
    loop for linked streams and find BATCH stream,
    use it to recalculate the skip pattern */
  if(stream_params->stream_type == CAM_STREAM_TYPE_PREVIEW) {
    /*find a batch mode stream*/
    for(i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
      linked_stream_params = stream_params->linked_streams[i];
      if (linked_stream_params && linked_stream_params->identity != 0) {
        if (linked_stream_params->stream_info->streaming_mode  == CAM_STREAMING_MODE_BATCH) {
          /*only one batch mode per port/per linked stream group*/
          break;
        }
      }
    }

    if(linked_stream_params && linked_stream_params->stream_info &&
      linked_stream_params->stream_info->streaming_mode ==
      CAM_STREAMING_MODE_BATCH) {
      CPP_HIGH("Batchmode enabled ipfps = %f opfps =%f batchsize = %d",
        stream_params->hfr_skip_info.input_fps,
        stream_params->hfr_skip_info.output_fps,
        linked_stream_params->stream_info-> user_buf_info.frame_buf_cnt);

      cal_batchsize = floor(stream_params->hfr_skip_info.input_fps /
        stream_params->hfr_skip_info.output_fps);

      linked_stream_batchsize = linked_stream_params->
        stream_info->user_buf_info.frame_buf_cnt;

      linked_stream_params->hfr_skip_info.skip_count = 0;
      if(cal_batchsize > linked_stream_batchsize) {
        stream_params->hfr_skip_info.skip_count =
          (cal_batchsize / linked_stream_batchsize) - 1;
      } else {
        /* In this case don't skip any */
        stream_params->hfr_skip_info.skip_count = 0;
      }
    }
  }

  if(stream_params->hfr_skip_info.skip_count < 0) {
    stream_params->hfr_skip_info.skip_count = 0;
  }

  CPP_BUF_DBG("Skip count = %d\n", stream_params->hfr_skip_info.skip_count);
  return 0;
}

/** cpp_module_set_output_duplication_flag:
 *
 *  Description:
 *    Based on stream's dimension info and existance of a linked
 *    stream, decide if output-duplication feature of cpp
 *    hardware can be utilized.
 *
 **/
int32_t cpp_module_set_output_duplication_flag(
  cpp_module_stream_params_t *stream_params)
{
  if(!stream_params) {
    CPP_ERR("invalid streamparams, failed");
    return -EINVAL;
  }
  stream_params->hw_params.duplicate_output = FALSE;
  stream_params->hw_params.duplicate_identity = 0x00;
  stream_params->hw_params.dup_buffer_info.identity = 0x00;

  CPP_HIGH(
    "current stream w=%d, h=%d, st=%d, sc=%d, fmt=%d, identity=0x%x",
    stream_params->hw_params.output_info.width,
    stream_params->hw_params.output_info.height,
    stream_params->hw_params.output_info.stride,
    stream_params->hw_params.output_info.scanline,
    stream_params->hw_params.output_info.plane_fmt,
    stream_params->identity);

  /* if there is no linked stream, no need for duplication */
  if(stream_params->num_linked_streams == 0) {
    CPP_DBG("info: no linked stream");
    return 0;
  }

  cpp_module_dump_all_linked_stream_info(stream_params);

  return 0;
}

/** cpp_module_get_divert_info:
 *
 *  Description:
 *    Based on streamon state of "this" stream and "linked"
 *      stream fetch the divert configuration sent by pproc
 *      module or otherwise return NULL
 *
 **/
pproc_divert_info_t *cpp_module_get_divert_info(uint32_t *identity_list,
  uint32_t identity_list_size, cpp_divert_info_t *cpp_divert_info)
{
  uint32_t i = 0, j = 0;
  uint8_t identity_mapped_idx = 0;
  uint8_t divert_info_config_table_idx = 0;
  pproc_divert_info_t *divert_info = NULL;

  /* Loop through the identity list to determine the corresponding index
     in the cpp_divert_info */
  for (i = 0; i < identity_list_size; i++) {
    if (identity_list[i] != PPROC_INVALID_IDENTITY) {
      /* Search the requested identity from the cpp_divert_info table */
      identity_mapped_idx = 0;
      for (j = 0; j < CPP_MAX_STREAMS_PER_PORT; j++) {
        if (cpp_divert_info->identity[j] == identity_list[i]) {
          identity_mapped_idx = j;
          break;
        }
      }
      if (j < CPP_MAX_STREAMS_PER_PORT) {
        divert_info_config_table_idx |= (1 << identity_mapped_idx);
      }
    }
  }

  if(divert_info_config_table_idx) {
    divert_info = &cpp_divert_info->config[divert_info_config_table_idx];
  }
  return divert_info;
}

int32_t cpp_module_util_post_diag_to_bus(mct_module_t *module,
  ez_pp_params_t *cpp_params, uint32_t identity)
{
  mct_bus_msg_t bus_msg_cpp_diag;
  mct_event_t event;

  bus_msg_cpp_diag.type = MCT_BUS_MSG_PP_CHROMATIX_LITE;
  bus_msg_cpp_diag.size = sizeof(ez_pp_params_t);
  bus_msg_cpp_diag.msg = (void *)cpp_params;
  bus_msg_cpp_diag.sessionid = (identity & 0xFFFF0000) >> 16;

  /* CPP being a sub-module inside pproc it cannot directly access mct */
  /* Create an event so that PPROC can post it to MCT */
  event.identity = identity;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_UPSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_PP_SUBMOD_POST_TO_BUS;
  event.u.module_event.module_event_data = (void *)&bus_msg_cpp_diag;

  if (cpp_module_send_event_upstream(module, &event) != 0) {
    CPP_META_ERR("error posting diag to bus\n");
  }
  return 0;
}

int32_t cpp_module_util_update_session_diag_params(mct_module_t *module,
  cpp_hardware_params_t* hw_params)
{
  cpp_module_stream_params_t *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_ctrl_t *ctrl;

  /* Check whether the current stream type needs update diag params */
   if ((hw_params->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) &&
     (hw_params->stream_type != CAM_STREAM_TYPE_SNAPSHOT) &&
     (hw_params->stream_type != CAM_STREAM_TYPE_PREVIEW)){
     return 0;
   }

  ctrl = (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_ERR("invalid control, failed\n");
    return -EFAULT;
  }

  /* Pick up session params and update diag params */
  cpp_module_get_params_for_identity(ctrl, hw_params->identity,
    &session_params, &stream_params);
  if(!session_params) {
    CPP_ERR("failed\n");
    return -EFAULT;
  }

  if ((hw_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) ||
    (hw_params->stream_type == CAM_STREAM_TYPE_SNAPSHOT)){
    memcpy(&session_params->diag_params.snap_asf7x7, &hw_params->asf_diag,
      sizeof(asfsharpness7x7_t));
    memcpy(&session_params->diag_params.snap_asf9x9, &hw_params->asf9x9_diag,
      sizeof(asfsharpness9x9_t));
    memcpy(&session_params->diag_params.snap_wnr, &hw_params->wnr_diag,
      sizeof(wavelet_t));
  } else if (hw_params->stream_type == CAM_STREAM_TYPE_PREVIEW) {
    memcpy(&session_params->diag_params.prev_asf7x7, &hw_params->asf_diag,
      sizeof(asfsharpness7x7_t));
    memcpy(&session_params->diag_params.prev_asf9x9, &hw_params->asf9x9_diag,
      sizeof(asfsharpness9x9_t));
    memcpy(&session_params->diag_params.prev_wnr, &hw_params->wnr_diag,
      sizeof(wavelet_t));
  }

  if (hw_params->diagnostic_enable) {
    if (hw_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
      cpp_module_hw_cookie_t *cookie;
      meta_data_container    *meta_datas;
      cookie = (cpp_module_hw_cookie_t *)hw_params->cookie;
      if (cookie->meta_datas) {
        meta_datas = cookie->meta_datas;
        if (meta_datas->mct_meta_data) {
          add_metadata_entry(CAM_INTF_META_CHROMATIX_LITE_PP,
            sizeof(ez_pp_params_t), &session_params->diag_params,
            meta_datas->mct_meta_data);
        } else {
          CPP_META_ERR("mct meta data is NULL\n");
        }
      } else {
        CPP_META_DBG("meta_data container is NULL\n");
      }
    } else {
      /* Post the updated diag params to bus if diagnostics is enabled */
      cpp_module_util_post_diag_to_bus(module, &session_params->diag_params,
        hw_params->identity);
    }
  }
  return 0;
}

int32_t cpp_module_util_post_crop_info(
  mct_module_t *module,
  cpp_hardware_params_t* hw_params,
  cpp_module_stream_params_t *stream_params)
{
  cam_stream_crop_info_t   crop_info;

  /* Output CPP crop to metadata for snapshots.
   * Required by dual-camera to calculate FOV changes.
   */
  if(stream_params->stream_type != CAM_STREAM_TYPE_SNAPSHOT &&
     stream_params->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) {
    return 0;
  }

  memset(&crop_info, 0, sizeof(crop_info));
  crop_info.stream_id       = PPROC_GET_STREAM_ID(stream_params->identity);
  crop_info.roi_map.left    = 0;
  crop_info.roi_map.top     = 0;
  crop_info.roi_map.width   = hw_params->input_info.width;
  crop_info.roi_map.height  = hw_params->input_info.height;
  crop_info.crop.left       = hw_params->crop_info.process_window_first_pixel;
  crop_info.crop.top        = hw_params->crop_info.process_window_first_line;
  crop_info.crop.width      = hw_params->crop_info.process_window_width;
  crop_info.crop.height     = hw_params->crop_info.process_window_height;

  CPP_CROP_LOW("CROP_INFO_CPP: str_id %x crop (%d, %d, %d,%d) ==> (%d,%d,%d,%d)",
    crop_info.stream_id,
    crop_info.roi_map.left, crop_info.roi_map.top,
    crop_info.roi_map.width, crop_info.roi_map.height,
    crop_info.crop.left, crop_info.crop.top,
    crop_info.crop.width, crop_info.crop.height);

  /* Put crop info into the metadata.
   * For reprocess update the metadata directly,
   * for snapshot send the data to MCT for update
   */
  if (hw_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    cpp_module_hw_cookie_t  *cookie = NULL;
    meta_data_container     *meta_datas = NULL;
    metadata_buffer_t       *meta_data_mct = NULL;

    cookie = hw_params->cookie;
    if(cookie != NULL) {
      meta_datas = (meta_data_container *)cookie->meta_datas;
    }
    if(meta_datas != NULL) {
      meta_data_mct = (metadata_buffer_t *)meta_datas->mct_meta_data;
    }

    if (meta_data_mct != NULL) {
      add_metadata_entry(CAM_INTF_META_SNAP_CROP_INFO_CPP,
        sizeof(crop_info), &crop_info, meta_data_mct);
    } else {
      CPP_META_ERR("meta_data container is NULL\n");
    }
  } else { /* snapshot stream - post to MCT */
    mct_bus_msg_t bus_msg;

    memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
    bus_msg.sessionid = PPROC_GET_SESSION_ID(stream_params->identity);
    bus_msg.type = MCT_BUS_MSG_SNAP_CROP_INFO_PP;
    bus_msg.size = sizeof(crop_info);
    bus_msg.msg = &crop_info;

    if (cpp_module_util_post_to_bus(module, &bus_msg, stream_params->identity) != 0) {
      CPP_META_ERR("error posting to bus\n");
    }
  }

  return 0;
}

int32_t cpp_module_util_handle_frame_drop(mct_module_t *module,
  cpp_module_stream_params_t* stream_params, uint32_t frame_id,
  cam_hal_version_t hal_version)
{
  int32_t                   rc = 0;
  uint32_t                  stream_frame_valid = 0;
  mct_event_t               mct_event;
  mct_event_frame_request_t frame_req;
  cpp_hardware_cmd_t        cmd;
  cpp_hardware_event_data_t hw_event_data;

  cpp_module_ctrl_t* ctrl = (cpp_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_BUF_ERR("invalid cpp control, failed\n");
    return -EINVAL;
  }


  if (!stream_params->hw_params.expect_divert ||
    (stream_params->is_stream_on != TRUE)) {
    return 0;
  }

  /* Check whether frame processing is valid. Only then the buffer
     is available in stream queue */
  if ((hal_version == CAM_HAL_V3) &&
    (stream_params->is_stream_on == TRUE)) {
    stream_frame_valid = cpp_module_get_frame_valid(module,
      stream_params->identity, frame_id,
      PPROC_GET_STREAM_ID(stream_params->identity),
      stream_params->stream_type, true);
  }

  if (stream_frame_valid) {
    /* Send upstream event to indicate frame drop */
    memset(&mct_event, 0, sizeof(mct_event));
    mct_event.u.module_event.type = MCT_EVENT_MODULE_FRAME_DROP_NOTIFY;
    memset(&frame_req, 0, sizeof(frame_req));
    frame_req.frame_index = frame_id;
    frame_req.stream_ids.num_streams = 1;
    frame_req.stream_ids.stream_request[0].streamID = (stream_params->identity & 0x0000FFFF);
    mct_event.u.module_event.module_event_data = (void *)&frame_req;
    mct_event.type = MCT_EVENT_MODULE_EVENT;
    mct_event.identity = stream_params->identity;
    mct_event.direction = MCT_EVENT_UPSTREAM;
    rc = cpp_module_send_event_upstream(module, &mct_event);
    if (rc < 0) {
      CPP_BUF_ERR("failed %d", rc);
      return -EFAULT;
    }

    /* TODO: Flush the queue parameters if any */

    /* Send the IOCTL to kernel to pop the buffer */
    rc = cpp_module_util_pop_buffer(ctrl, stream_params, frame_id);
  }

  return 0;
}

int32_t cpp_module_util_pop_buffer(cpp_module_ctrl_t* ctrl,
  cpp_module_stream_params_t* stream_params, uint32_t frame_id)
{
  int32_t                   rc = 0;
  cpp_hardware_cmd_t        cmd;
  cpp_hardware_event_data_t hw_event_data;
  cpp_module_stream_params_t  *stream_params1 = NULL;
  cpp_module_session_params_t *session_params = NULL;
  uint32_t buf_idx = 0xFFFFFFFF;
  uint32_t stream_id = PPROC_GET_STREAM_ID(stream_params->identity);
  cam_stream_type_t stream_type = stream_params->stream_type;
  uint32_t i;
  bool valid_request = false;

  if(!ctrl) {
    CPP_BUF_ERR("invalid cpp control, failed\n");
    return -EINVAL;
  }
  /* get stream parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl,
    stream_params->identity, &session_params, &stream_params1);
  if(!session_params || !stream_params1) {
    CPP_BUF_ERR("failed session_params %p, stream_params %p\n",
      session_params, stream_params);
    return -EFAULT;
  }
  /* get the buffer index from per frame queue */
  uint32_t q_idx = frame_id % FRAME_CTRL_SIZE;
  if (stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) {
    for (i = 0; i < session_params->valid_stream_ids[q_idx].num_streams; i++) {
      if (session_params->valid_stream_ids[q_idx].stream_request[i].streamID ==
        stream_id) {
        buf_idx = session_params->valid_stream_ids[q_idx]. \
          stream_request[i].buf_index;
        session_params->valid_stream_ids[q_idx].stream_request[i].streamID = 0;
        session_params->verify_proc_stream_ids[q_idx]. \
          stream_request[i].streamID = 0;
        valid_request = true;
        break;
      }
    }
  }

  if (session_params->hal_version != CAM_HAL_V3)
    buf_idx = 0xFFFFFFFF;

  if (session_params->hal_version == CAM_HAL_V3 && (!valid_request)) {
    CPP_BUF_LOW("Invalid buffer index");
    return -EINVAL;
  }
  CPP_BUF_ERR("Sending IOCTL to pop frame id %d buf_idx %d\n",
    frame_id, buf_idx);
  /* Send the IOCTL to kernel to pop the buffer */
  memset(&hw_event_data, 0, sizeof(hw_event_data));
  cmd.type = CPP_HW_CMD_POP_STREAM_BUFFER;
  cmd.u.event_data = &hw_event_data;
  hw_event_data.frame_id = frame_id;
  hw_event_data.identity = stream_params->identity;
  hw_event_data.buf_idx = buf_idx;
  rc = cpp_hardware_process_command(ctrl->cpphw, cmd);
  if (rc < 0) {
    CPP_BUF_ERR("failed stream buffer pop, iden:0x%x frmid:%d\n",
     stream_params->identity, frame_id);
    return rc;
  }

  return 0;
}



/** cpp_module_utill_free_queue_data:
 *
 *  @data: list data
 *
 *  @user_data : user data (NULL)
 *
 *  Free list data
 **/
boolean cpp_module_utill_free_queue_data(void *data, void *user_data __unused)
{
  cpp_frame_ctrl_data_t *frame_ctrl_data = data;
  if (frame_ctrl_data) {
    if (MCT_EVENT_MODULE_EVENT == frame_ctrl_data->mct_type)
      free(frame_ctrl_data->u.module_event.module_event_data);
    else
      free(frame_ctrl_data->u.ctrl_param.parm_data);
    free(frame_ctrl_data);
  }
  return TRUE;
}

int32_t cpp_module_util_post_to_bus(mct_module_t *module,
  mct_bus_msg_t *bus_msg, uint32_t identity)
{
  int32_t rc = 0;
  mct_event_t event;

  /* CPP being a sub-module inside pproc it cannot directly access mct */
  /* Create an event so that PPROC can post it to MCT */
  event.identity = identity;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_UPSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_PP_SUBMOD_POST_TO_BUS;
  event.u.module_event.module_event_data = (void *)bus_msg;

  CPP_META_DBG("type:%d size:%d\n", bus_msg->type, bus_msg->size);
  rc = cpp_module_send_event_upstream(module, &event);
  if (rc < 0) {
    CPP_META_ERR("error posting msg to bus\n");
  }
  return rc;
}

int32_t cpp_module_util_post_metadata_to_bus(mct_module_t *module,
  cam_intf_parm_type_t type, void *parm_data, uint32_t identity)
{
  int32_t rc = 0;
  mct_bus_msg_t bus_msg;

  if (!module || !parm_data) {
    CPP_ERR("failed: module %p parm_data %p", module, parm_data);
    return -EINVAL;
  }

  memset(&bus_msg, 0x0, sizeof(bus_msg));
  bus_msg.sessionid = (identity >> 16) & 0xFFFF;
  bus_msg.msg = (void *)parm_data;
  bus_msg.size = 0;

  switch (type) {
  case CAM_INTF_PARM_SHARPNESS: {
    bus_msg.type = MCT_BUS_MSG_SET_SHARPNESS;
    bus_msg.size = sizeof(int32_t);
    CPP_ASF_DBG("CAM_INTF_PARM_SHARPNESS size:%d value:%d\n",
      bus_msg.size, *(int32_t *)parm_data);
  }
    break;
  case CAM_INTF_PARM_EFFECT: {
    bus_msg.type = MCT_BUS_MSG_SET_EFFECT;
    bus_msg.size = sizeof(int32_t);
    CPP_ASF_DBG("CAM_INTF_PARM_EFFECT size:%d value:%d\n",
      bus_msg.size, *(int32_t *)parm_data);
  }
    break;
  case CAM_INTF_META_EDGE_MODE: {
    bus_msg.type = MCT_BUS_MSG_SET_EDGE_MODE;
    bus_msg.size = sizeof(cam_edge_application_t);
    CPP_ASF_DBG("CAM_INTF_META_EDGE_MODE size:%d mode:%d strength:%d\n",
      bus_msg.size, ((cam_edge_application_t *)parm_data)->edge_mode,
      ((cam_edge_application_t *)parm_data)->sharpness);
  }
    break;
  case CAM_INTF_META_NOISE_REDUCTION_MODE: {
    bus_msg.type = MCT_BUS_MSG_SET_NOISE_REDUCTION_MODE;
    bus_msg.size = sizeof(int32_t);
    CPP_DENOISE_DBG("CAM_INTF_META_NOISE_REDUCTION_MODE size:%d value:%d\n",
      bus_msg.size, *(int32_t *)parm_data);
  }
    break;
  case CAM_INTF_PARM_WAVELET_DENOISE: {
    bus_msg.type = MCT_BUS_MSG_SET_WAVELET_DENOISE;
    bus_msg.size = sizeof(cam_denoise_param_t);
    CPP_DENOISE_DBG("CAM_INTF_PARM_WAVELET_DENOISE size:%d enable:%d"
      "plates:%d\n", bus_msg.size,
      ((cam_denoise_param_t *)parm_data)->denoise_enable,
      ((cam_denoise_param_t *)parm_data)->process_plates);
  }
    break;
  case CAM_INTF_PARM_TEMPORAL_DENOISE: {
    bus_msg.type = MCT_BUS_MSG_SET_TEMPORAL_DENOISE;
    bus_msg.size = sizeof(cam_denoise_param_t);
    CPP_TNR_DBG("CAM_INTF_PARM_TEMPORAL_DENOISE size:%d enable:%d"
      "plates:%d\n", bus_msg.size,
      ((cam_denoise_param_t *)parm_data)->denoise_enable,
      ((cam_denoise_param_t *)parm_data)->process_plates);
  }
    break;
  case CAM_INTF_PARM_ROTATION: {
    bus_msg.type = MCT_BUS_MSG_SET_ROTATION;
    bus_msg.size = sizeof(cam_rotation_info_t);
    CPP_META_DBG("CAM_INTF_PARM_ROTATION size:%d rotation:%x, dev_rotation:%x\n",
      bus_msg.size,
     ((cam_rotation_info_t *)parm_data)->rotation,
     ((cam_rotation_info_t *)parm_data)->device_rotation);
  }
    break;
  case CAM_INTF_PARM_CDS_MODE: {
    bus_msg.type = MCT_BUS_MSG_SET_CDS;
    bus_msg.size = sizeof(int32_t);
    CPP_DENOISE_DBG("CAM_INTF_PARM_CDS_MODE size:%d value:%d\n",
      bus_msg.size, *(int32_t *)parm_data);
    break;
  }
  case CAM_INTF_META_IMG_DYN_FEAT: {
    bus_msg.type = MCT_BUS_MSG_SET_IMG_DYN_FEAT;
    bus_msg.size = sizeof(cam_dyn_img_data_t);
    CPP_DENOISE_DBG("CAM_INTF_META_IMG_DYN_FEAT size:%d value:%llu\n",
      bus_msg.size,
      ((cam_dyn_img_data_t *)parm_data)->dyn_feature_mask);
    break;
  }
  default:
    bus_msg.type = MCT_BUS_MSG_MAX;
    bus_msg.size = 0;
    break;
  }

  if (bus_msg.size) {
    rc = cpp_module_util_post_to_bus(module, &bus_msg, identity);
    if (rc < 0) {
      CPP_META_ERR("failed to post meta to bus: type=%d\n", bus_msg.type);
    }
  }
  return rc;
}

/** cpp_module_utils_fetch_native_bufs:
 *
 *  @data - This parameter holds the current member of the list.
 *  @user_data - This parameter holds the user data to be set.
 *
 *  This function allocates and populates buf_holder structure with data for
 *  the current native buffer. It appends the allocated structure to buffer
 *  array.
 *
 *  Return: Returns 0 at success.
 **/
boolean cpp_module_utils_fetch_native_bufs(void *data, void *user_data) {

  pp_frame_buffer_t *img_buf = (pp_frame_buffer_t *)data;
  cpp_hardware_native_buff_array *buffer_array =
    (cpp_hardware_native_buff_array *)user_data;
  uint32_t i;

  if (!img_buf || !buffer_array) {
    CPP_BUF_ERR("Fail to fetch native buffers");
    return FALSE;
  }

  memset(img_buf->vaddr, 0x80, img_buf->ion_alloc[0].len);

  for (i = 0; i < CPP_TNR_SCRATCH_BUFF_COUNT; i++) {
    if (buffer_array->buff_array[i].fd == 0) {
      buffer_array->buff_array[i].fd = img_buf->fd;
      buffer_array->buff_array[i].index = img_buf->buffer.index;
      buffer_array->buff_array[i].native_buff = 1;
      buffer_array->buff_array[i].offset = 0;
      buffer_array->buff_array[i].processed_divert = 0;
      buffer_array->buff_array[i].vaddr = img_buf->vaddr;
      buffer_array->buff_array[i].alloc_len = img_buf->ion_alloc[0].len;
      break;
    }
  }

  if (i >= CPP_TNR_SCRATCH_BUFF_COUNT) {
    CPP_BUF_ERR("Array is full\n");
    return FALSE;
  }

  return TRUE;
}

int32_t cpp_module_port_mapping(mct_module_t *module, mct_port_direction_t dir,
  mct_port_t *port, uint32_t identity)
{
  uint16_t stream_id;
  uint16_t session_id;
  cpp_module_ctrl_t *ctrl;
  uint16_t dir_idx = 0;

  if (!module) {
    CPP_ERR("Invalid pointers module:%p", module);
    return -EINVAL;
  }

  stream_id = CPP_GET_STREAM_ID(identity);
  session_id = CPP_GET_SESSION_ID(identity);
  ctrl = (cpp_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);

  if (!ctrl) {
    CPP_ERR("Invalid pointers ctrl:%p", ctrl);
    return -EINVAL;
  }

  if ((session_id >= CPP_MODULE_MAX_SESSIONS) ||
    (stream_id >= CPP_MODULE_MAX_STREAMS)) {
    CPP_ERR("Exceeding the port map entries, session:%d, stream:%d",
      session_id, stream_id);
    return FALSE;
  }

  if (dir == MCT_PORT_SINK) {
    dir_idx = 1;
  }

  if ((port != NULL) && (ctrl->port_map[session_id][stream_id][dir_idx] !=
    NULL)) {
    CPP_ERR("Port from old stream, port:%p",
      ctrl->port_map[session_id][stream_id][dir_idx]);
    return -EINVAL;
  }

  ctrl->port_map[session_id][stream_id][dir_idx] = port;

  return 0;
}

boolean cpp_module_check_queue_compatibility(cpp_module_ctrl_t *ctrl,
  cam_stream_type_t process_stream_type, uint32_t process_identity,
  uint32_t queue_identity)
{
  boolean                      ret = TRUE;
  cam_stream_type_t            queue_stream_type;
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;

  if (process_identity == queue_identity)
    return ret;

  cpp_module_get_params_for_identity(ctrl, queue_identity, &session_params,
     &stream_params);
  if(!stream_params) {
    CPP_ERR("invalid stream params, failed\n");
    return ret;
  }

  if ((stream_params->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) &&
    (process_stream_type != CAM_STREAM_TYPE_OFFLINE_PROC))
    return ret;

  return FALSE;
}

boolean cpp_module_pop_per_frame_entry(cpp_module_ctrl_t *ctrl,
  cpp_per_frame_params_t *per_frame_params, uint32_t q_idx,
  uint32_t cur_frame_id, cpp_frame_ctrl_data_t **frame_ctrl_data,
  cpp_module_stream_params_t *stream_params)
{
  boolean                queue_compatible = TRUE;
  cpp_frame_ctrl_data_t *local_frame_ctrl_data;

  if (!frame_ctrl_data) {
    CPP_PER_FRAME_ERR("frame_ctrl_data:%p\n", frame_ctrl_data);
    return FALSE;
  }

  *frame_ctrl_data = NULL;
  if (!ctrl || !per_frame_params) {
    CPP_PER_FRAME_ERR("ctrl:%p, per_frame_params:%p\n", ctrl,
      per_frame_params);
    return FALSE;
  }

  local_frame_ctrl_data =
    mct_queue_pop_head(per_frame_params->frame_ctrl_q[q_idx]);
  if (!local_frame_ctrl_data)
    return FALSE;

  /* Since we are currently reusing the queue for realtime & offline
     processing the frameid from the queue entry needs to be checked
     considering the fact that some offline entries with different
     frameid may be available. Also for different concurrent offline
     streams the entries of current offline stream should not be
     compared with other offline streams */
  queue_compatible = cpp_module_check_queue_compatibility(ctrl,
    stream_params->stream_type, stream_params->identity,
    local_frame_ctrl_data->identity);
  if (queue_compatible == FALSE) {
    /* Enqueue the entry back to the queue */
    mct_queue_push_tail(per_frame_params->frame_ctrl_q[q_idx],
      (void *)local_frame_ctrl_data);
    return TRUE;
  }

  if (local_frame_ctrl_data->frame_id != cur_frame_id) {
    CPP_PER_FRAME_LOW("failed: wrong queue for mct_type = %d frame %d exp %d",
      local_frame_ctrl_data->mct_type,
      local_frame_ctrl_data->frame_id,
      cur_frame_id);
    if (MCT_EVENT_MODULE_EVENT == local_frame_ctrl_data->mct_type)
      free(local_frame_ctrl_data->u.module_event.module_event_data);
    else
      free(local_frame_ctrl_data->u.ctrl_param.parm_data);
    free(local_frame_ctrl_data);
    return TRUE;
  }

  *frame_ctrl_data = local_frame_ctrl_data;
  return  TRUE;
}

void cpp_module_free_stream_based_entry(cpp_module_ctrl_t *ctrl,
  cam_stream_type_t stream_type, cpp_per_frame_params_t *per_frame_params)
{
  uint32_t                     j, i, queue_len;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_frame_ctrl_data_t       *frame_ctrl_data;

  /* Dont wait for stop session, clear the offline stream per
     frame queue entries after last offline stream is off. */
  for (j = 0; j < FRAME_CTRL_SIZE; j++) {
    if (!per_frame_params->frame_ctrl_q[j]) {
      continue;
    }

    pthread_mutex_lock(&per_frame_params->frame_ctrl_mutex[j]);
    queue_len = per_frame_params->frame_ctrl_q[j]->length;
    for (i = 0; i < queue_len; i++) {
      frame_ctrl_data =
        mct_queue_pop_head(per_frame_params->frame_ctrl_q[j]);
      if (!frame_ctrl_data) {
        CPP_PER_FRAME_ERR("frame_ctrl_data:%p\n", frame_ctrl_data);
        continue;
      }

      cpp_module_get_params_for_identity(ctrl,
        frame_ctrl_data->identity, &session_params, &stream_params);
      if(!stream_params) {
        CPP_PER_FRAME_ERR("queue_stream_params:%p\n", stream_params);
        continue;
      }

      if (((stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) &&
        (stream_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC)) ||
        ((stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) &&
        (stream_params->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC))) {
        cpp_module_utill_free_queue_data(frame_ctrl_data, NULL);
        continue;
      }

      mct_queue_push_tail(per_frame_params->frame_ctrl_q[j],
        (void *)frame_ctrl_data);
    }
    pthread_mutex_unlock(&per_frame_params->frame_ctrl_mutex[j]);
  }
}

void cpp_module_util_update_asf_params(cpp_hardware_params_t *hw_params, bool asf_mask)
{

  if (asf_mask == TRUE) {
    if (hw_params->sharpness_level == 0.0f) {
      if (hw_params->asf_mode == CPP_PARAM_ASF_DUAL_FILTER) {
        CPP_ASF_LOW("CPP_PARAM_ASF_OFF");
        hw_params->asf_mode = CPP_PARAM_ASF_OFF;
      }
    } else {
      if (hw_params->asf_mode == CPP_PARAM_ASF_OFF) {
        CPP_ASF_LOW("CPP_PARAM_ASF_DUAL_FILTER");
        hw_params->asf_mode = CPP_PARAM_ASF_DUAL_FILTER;
      }
    }
  } else {
    hw_params->asf_mode = CPP_PARAM_ASF_OFF;
    hw_params->sharpness_level = 0.0f;
  }
  CPP_ASF_LOW("stream_type %d, asf_mask %d, asf_mode %d,asf_level %f",
    hw_params->stream_type, asf_mask,
    hw_params->asf_mode, hw_params->sharpness_level);
}

void cpp_module_util_update_asf_region(cpp_module_session_params_t
  *session_params, cpp_module_stream_params_t *stream_params,
  cpp_hardware_params_t *hw_params)
{
  if(!session_params || !stream_params || !hw_params){
    CPP_ERR("Invalid parameters\n");
    return;
  }

  if(stream_params->stream_type == CAM_STREAM_TYPE_PREVIEW) {
    /*Update session param value since it will be consumed in sof_notify which will be in session
       stream  for OSD feature*/
    CPP_META_LOW("Preview stream .Updating session params\n");
    session_params->hw_params.asf_info.region = hw_params->asf_info.region;
    session_params->hw_params.asf_info.asf_reg1_idx =
      hw_params->asf_info.asf_reg1_idx;
    session_params->hw_params.asf_info.asf_reg2_idx =
      hw_params->asf_info.asf_reg2_idx;
  } else {
    CPP_META_LOW("Not preview stream. Not updating session params\n");
  }
}

void cpp_module_util_calculate_scale_ratio(cpp_hardware_params_t *hw_params,
  float *isp_scale_ratio, float *cpp_scale_ratio)
{
  if (!hw_params->isp_width_map || !hw_params->isp_height_map ||
    !hw_params->input_info.width || !hw_params->input_info.height) {
    *isp_scale_ratio = 1.0f;
  } else {
    CPP_CROP_LOW("### isp w, h [%d, %d], i/p w,h[%d: %d]",
        hw_params->isp_width_map, hw_params->isp_height_map,
        hw_params->input_info.width, hw_params->input_info.height);
    float width_ratio, height_ratio;
    width_ratio = (float)hw_params->isp_width_map /
    hw_params->input_info.width;
    height_ratio = (float)hw_params->isp_height_map /
    hw_params->input_info.height;
    if (width_ratio <= height_ratio) {
      *isp_scale_ratio = width_ratio;
    } else {
      *isp_scale_ratio = height_ratio;
    }
  }

  if (!hw_params->crop_info.process_window_width ||
    !hw_params->crop_info.process_window_height ||
    !hw_params->input_info.width || !hw_params->input_info.height) {
    *cpp_scale_ratio = 1.0f;
  } else {
    CPP_CROP_LOW("### crop w, h [%d, %d], o/p w,h[%d: %d]",
      hw_params->crop_info.process_window_width,
      hw_params->crop_info.process_window_height,
      hw_params->output_info.width, hw_params->output_info.height);
    float width_ratio, height_ratio;
    width_ratio = (float)hw_params->crop_info.process_window_width /
    hw_params->output_info.width;
    height_ratio = (float)hw_params->crop_info.process_window_height /
    hw_params->output_info.height;
    if (width_ratio <= height_ratio) {
      *cpp_scale_ratio = width_ratio;
    } else {
      *cpp_scale_ratio = height_ratio;
    }
  }
  CPP_CROP_DBG("### Scale ratio [isp: %f, cpp: %f]",
    *isp_scale_ratio, *cpp_scale_ratio);
  return;
}

#ifdef OEM_CHROMATIX
bool cpp_module_util_is_two_pass_reprocess(cpp_module_stream_params_t *stream_params)
{
  cam_pp_feature_config_t *pp_config = NULL;
  mct_stream_info_t* stream_info = stream_params->stream_info;

  if (!stream_params || !stream_params->stream_info) {
    CPP_LOW("Invalid stream info for stream %d", stream_params->stream_type);
  }
  switch (stream_params->stream_type) {
    case CAM_STREAM_TYPE_OFFLINE_PROC:
      pp_config = &stream_info->reprocess_config.pp_feature_config;
    break;
    case CAM_STREAM_TYPE_CALLBACK:
    case CAM_STREAM_TYPE_SNAPSHOT:
    case CAM_STREAM_TYPE_VIDEO:
    default:
      pp_config = &stream_info->pp_config;
    break;
  }

  if (!pp_config) {
    CPP_ERR("pp config null, no two pass");
    goto end;
  }

  CPP_DBG("[REPROCESS] reprocess count %d, total reprocess_count %d",
    pp_config->cur_reproc_count, pp_config->total_reproc_count);

  // Two reprocess counts return true
  if (pp_config->total_reproc_count == 2) {
    return true;
  }
end:
  return false;
}
#endif

int32_t cpp_module_util_update_chromatix_pointer(cpp_module_stream_params_t  *stream_params,
  cpp_module_stream_params_t  *dup_stream_params,
  modulesChromatix_t *chromatix_ptr, float scale_ratio)
{

  chromatix_cpp_type *chromatix_snap_ptr = NULL;
  int32_t ret = 0;
  if (!stream_params) {
     ret = -EFAULT;
     CPP_ERR("Invalid stream params");
     goto end;
  }

  if (!chromatix_ptr) {
     ret = -EFAULT;
     CPP_ERR("Invalid chromatix pointer");
     goto end;
  }
  CPP_DBG("[CHROMATIX] chromatix ptr: stream_type %d, stream_chromatix %p,"
   "cpp_scale_ratio %f\n",
   stream_params->stream_type, chromatix_ptr->chromatixCppPtr,
   scale_ratio);

  switch (stream_params->stream_type) {
    case CAM_STREAM_TYPE_CALLBACK:
    case CAM_STREAM_TYPE_SNAPSHOT:
    case CAM_STREAM_TYPE_OFFLINE_PROC:
      chromatix_snap_ptr =
        (chromatix_cpp_type *)chromatix_ptr->chromatixSnapCppPtr;
      if (!chromatix_snap_ptr) {
        CPP_ERR("Invalid chromatix snapshot pointer, using preview");
        ret = 0;
        goto end;
      }
      #ifdef OEM_CHROMATIX
      if (!cpp_module_util_is_two_pass_reprocess(stream_params)) {
        if(stream_params->hw_params.low_light_capture_enable) {
          chromatix_ptr->chromatixCppPtr =
            (chromatix_ptr->chromatixOisSnapCppPtr != NULL) ?
            chromatix_ptr->chromatixOisSnapCppPtr : chromatix_snap_ptr;
        } else {
          chromatix_ptr->chromatixCppPtr = chromatix_snap_ptr;
        }
        CPP_DBG("### [CHROMATIX] chromatix ptr: Not 2 step reprocess"
          " for stream_type %d stream chromatix %p",
          stream_params->stream_type, chromatix_ptr->chromatixCppPtr);
        ret = 0;
        goto end;
      }
      #endif

      if(scale_ratio < CPP_UPSCALE_THRESHOLD(chromatix_snap_ptr)) {
        /* Low light Capture */
        if(stream_params->hw_params.low_light_capture_enable) {
          chromatix_ptr->chromatixCppPtr =
            (chromatix_ptr->chromatixOisUsCppPtr != NULL) ?
            chromatix_ptr->chromatixOisUsCppPtr : chromatix_snap_ptr;
        } else {
        chromatix_ptr->chromatixCppPtr =
          (chromatix_ptr->chromatixUsCppPtr != NULL) ?
          chromatix_ptr->chromatixUsCppPtr : chromatix_snap_ptr;
        }

      } else if (scale_ratio > CPP_DOWNSCALE_THRESHOLD(chromatix_snap_ptr)) {
        if(stream_params->hw_params.low_light_capture_enable) {
        chromatix_ptr->chromatixCppPtr =
          (chromatix_ptr->chromatixOisDsCppPtr != NULL) ?
          chromatix_ptr->chromatixOisDsCppPtr : chromatix_snap_ptr;
        } else {
          chromatix_ptr->chromatixCppPtr =
            (chromatix_ptr->chromatixDsCppPtr != NULL) ?
            chromatix_ptr->chromatixDsCppPtr : chromatix_snap_ptr;
        }
      } else {
        if(stream_params->hw_params.low_light_capture_enable) {
          chromatix_ptr->chromatixCppPtr = chromatix_ptr->chromatixOisSnapCppPtr;
          CPP_DBG("OIS capture enabled stream_type %d stream chromatix %p",
            stream_params->stream_type, chromatix_ptr->chromatixCppPtr);
        } else {
          chromatix_ptr->chromatixCppPtr = chromatix_snap_ptr;
        }
      }

      break;
    case CAM_STREAM_TYPE_VIDEO:
      if(chromatix_ptr->chromatixVideoCppPtr) {
        chromatix_ptr->chromatixCppPtr =
          (chromatix_cpp_type *)chromatix_ptr->chromatixVideoCppPtr;
      }
      break;
    case CAM_STREAM_TYPE_PREVIEW:
      /* Use video chromatix if buffer is on preview in video mode during duplication */
      if ((dup_stream_params) &&
        (dup_stream_params->stream_type == CAM_STREAM_TYPE_VIDEO) &&
        (chromatix_ptr->chromatixVideoCppPtr)) {
        chromatix_ptr->chromatixCppPtr =
          (chromatix_cpp_type *)chromatix_ptr->chromatixVideoCppPtr;
      }
     break;
    default:
      CPP_DBG("###default ptr update for stream_type %d",
        stream_params->stream_type);
    break;
  }

  CPP_DBG("### [CHROMATIX]  chromatix ptr after: stream_type %d, stream_chromatix %p\n",
    stream_params->stream_type, chromatix_ptr->chromatixCppPtr);
end:
  return ret;
}

/** cpp_module_util_update_plane_info:
 *
 *  @hw_params - This parameter holds the cpp hardware params data structure.
 *  @dim_info - This parameter holds cpp dimension info data structure.
 *  @plane_info - This paramter is input data structure referring to the
 *  HAL plane info data structure having dimension/fmt/plane parameters.
 *  This data structure is updated in the function.
 *
 *  This function is a utility function used to convert/copy the buffer/plane
 *  dimension related parameters to HAL data structure so clients like
 *  buffer manager that accepts HAL data structure recieves correct params.
 *
 *  Return: Returns 0 at success and error (-ve value) on failure.
 **/
int32_t cpp_module_util_update_plane_info(cpp_hardware_params_t *hw_params,
  cpp_params_dim_info_t *dim_info, cam_frame_len_offset_t *plane_info)
{
    uint8_t i = 0;
    if (!hw_params || !dim_info || !plane_info) {
      CPP_ERR("invalid input parameters, cannot update plane info");
      return -EINVAL;
    }
    memset(plane_info, 0x00, sizeof(cam_frame_len_offset_t));
    if(dim_info->plane_fmt == CPP_PARAM_PLANE_Y) {
      plane_info->num_planes = 1;
    } else if (dim_info->plane_fmt == CPP_PARAM_PLANE_CRCB420){
      plane_info->num_planes = 3;
    } else {
      plane_info->num_planes = 2;
    }
    for (i = 0; i < plane_info->num_planes; i++) {
      plane_info->mp[i].width = dim_info->width;
      plane_info->mp[i].height = dim_info->height;
      plane_info->mp[i].stride = dim_info->stride;
      plane_info->mp[i].scanline = dim_info->scanline;
      plane_info->mp[i].len = dim_info->plane_info[i].plane_len;
      CPP_BUF_DBG("plane:%d,width:%d,height:%d,st:%d,sc:%d,len:%d",
        i, plane_info->mp[i].width, plane_info->mp[i].height,
        plane_info->mp[i].stride, plane_info->mp[i].scanline, plane_info->mp[i].len);
    }
    plane_info->frame_len = dim_info->frame_len;
    CPP_BUF_DBG("frame_len %d", plane_info->frame_len);
    return 0;
}

/** cpp_module_util_configure_clock_rate:
 *
 *  @ctrl - This parameter holds the cpp control pointer
 *  @perf_mode - This parameter holds info if performance mode is set from HAL.
 *  @index - This parameter holds the referenceindex to the clock freq table.
 *  @clk_rate - This parameter points to clock rate calculated from the load.
 *
 *  This function is a utility function to set the clock value calculated
 *  with default load or override with user input based on property or
 *  turbo value based on performance falg set from HAL.
 *
 *  Return: Returns 0 at success and error (-ve value) on failure.
 **/
int32_t cpp_module_util_configure_clock_rate(cpp_module_ctrl_t *ctrl,
  uint32_t perf_mode, uint32_t *index, long unsigned int *clk_rate)
{
  char value[PROPERTY_VALUE_MAX] = "";

  do {
    property_get("vendor.camera.cpp.set.clock", value, "0");
    if (atoi(value)) {
      *index = 0;
      *clk_rate = (atoi(value)) * 1000000;
      CPP_DBG("Set clock rate with property clk rate %lu", *clk_rate);
      break;
    } else if ((perf_mode & CAM_PERF_HIGH_PERFORMANCE) >>
      CAM_PERF_HIGH_PERFORMANCE_BIT) {
      *index = ctrl->turbo_caps ? (ctrl->cpphw->hwinfo.freq_tbl_count - 1) :
        (ctrl->cpphw->hwinfo.freq_tbl_count - 2);
      break;
    } else if (perf_mode & CAM_PERF_HIGH) {
      *index = ctrl->cpphw->hwinfo.freq_tbl_count - 2;
      break;
    } else {
      *index = 0;
      break;
    }
  } while(0);

  return 0;
}

int32_t cpp_module_util_post_error_to_bus(mct_module_t *module, uint32_t identity)
{
  mct_bus_msg_t bus_msg;
  mct_event_t event;

  CPP_ERR(": post error MCT_BUS_MSG_SEND_HW_ERROR");
  mct_bus_msg_error_message_t err_msg;
  memset(&bus_msg, 0, sizeof(bus_msg));
  bus_msg.sessionid =  CPP_GET_SESSION_ID(identity);
  bus_msg.type = MCT_BUS_MSG_SEND_HW_ERROR;

  /* CPP being a sub-module inside pproc it cannot directly access mct */
  /* Create an event so that PPROC can post it to MCT */
  event.identity = identity;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_UPSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_PP_SUBMOD_POST_TO_BUS;
  event.u.module_event.module_event_data = (void *)&bus_msg;

  if (cpp_module_send_event_upstream(module, &event) != 0) {
    CPP_META_ERR("error posting diag to bus\n");
  }
  return 0;
}

/** cpp_module_util_get_cds_hystersis_info
 *
 *  @chromatix_cpp - This parameter holds the preview chromatix pointer.
 *  @hw_params - This parameter holds the sessions hw params.
 *  @aec_trigger - This parameter holds the aec trigger value.
 *
 *  This function is used to extracts the trigger and calculate and update
 *  the hystersis value.
 *
 *  Return: Returns 0 on success error otherwise
 **/
int32_t cpp_module_util_get_cds_hystersis_info(chromatix_cpp_type *chromatix_cpp,
  cpp_hardware_params_t *hw_params, cpp_params_aec_trigger_info_t *aec_trigger)
{
  float trigger_input;
  float bf_dec_hyst_trigger_strt;
  float bf_dec_hyst_trigger_end;

#if ((defined (CHROMATIX_308) || defined (CHROMATIX_308E) || \
  defined (CHROMATIX_309) || defined (CHROMATIX_310) || defined (CHROMATIX_310E)) && \
  defined(CAMERA_USE_CHROMATIX_HW_WNR_TYPE))
  Chromatix_hardware_wavelet_type   *wavelet_denoise;
#else
  wavelet_denoise_type *wavelet_denoise;
  goto end;
#endif

  GET_WAVELET_DENOISE_POINTER(chromatix_cpp, hw_params, wavelet_denoise);
  if (wavelet_denoise->control_denoise == 0) {
    CPP_DENOISE_LOW("lux triggered");
    /* Lux index based */
    trigger_input = aec_trigger->lux_idx;
    bf_dec_hyst_trigger_strt = GET_WNR_DEC_LUXIDX_TRIGGER_START(wavelet_denoise);
    bf_dec_hyst_trigger_end = GET_WNR_DEC_LUXIDX_TRIGGER_END(wavelet_denoise);
  } else {
    CPP_DENOISE_LOW("gain triggered");
    /* Gain based */
    trigger_input = aec_trigger->gain;
    bf_dec_hyst_trigger_strt = GET_WNR_DEC_GAIN_TRIGGER_START(wavelet_denoise);
    bf_dec_hyst_trigger_end = GET_WNR_DEC_GAIN_TRIGGER_END(wavelet_denoise);
  }

  CPP_DENOISE_DBG("Previous CDS hystersis state %d", hw_params->hyst_dsdn_status);
  hw_params->hyst_dsdn_status =
    cpp_module_utils_get_hysteresis_trigger(trigger_input,
    bf_dec_hyst_trigger_strt, bf_dec_hyst_trigger_end,
    hw_params->hyst_dsdn_status);

  CPP_DENOISE_DBG("### CDS HYSTERSIS trigger_input %f, trigger start %f,"
  "trigger end %f, prev dsdn hyst_status %d",
   trigger_input, bf_dec_hyst_trigger_strt, bf_dec_hyst_trigger_end,
   hw_params->hyst_dsdn_status);

end:
  return 0;
}

/** cpp_module_util_get_tnr_hystersis_info
 *
 *  @chromatix_cpp - This parameter holds the preview chromatix pointer.
 *  @hw_params - This parameter holds the sessions hw params.
 *  @aec_trigger - This parameter holds the aec trigger value.
 *
 *  This function is used to extracts the trigger and calculate and update
 *  the hystersis value.
 *
 *  Return: Returns 0 on success error otherwise
 **/
 int32_t cpp_module_util_get_tnr_hystersis_info(
  chromatix_cpp_type *cpp_chromatix_ptr, cpp_hardware_params_t *hw_params,
  cpp_params_aec_trigger_info_t *aec_trigger) {

  float                       hyst_trigger_strt;
  float                       hyst_trigger_end;
  float                       trigger_input;
  tuning_control_type       tnr_hysteresis_trigger_type;
  chromatixWNRDenoise_type *tnr_chrmtx_ptr = NULL;

  tnr_chrmtx_ptr = &cpp_chromatix_ptr->chromatix_temporal_denoise;

  tnr_hysteresis_trigger_type = GET_TNR_HYSTERESIS_TRIGGER_TYPE(tnr_chrmtx_ptr);
  if(tnr_hysteresis_trigger_type == 0) {
    /* Lux index based */
    trigger_input = aec_trigger->lux_idx;
    hyst_trigger_strt = GET_TNR_LUXIDX_TRIGGER_START(tnr_chrmtx_ptr);
    hyst_trigger_end = GET_TNR_LUXIDX_TRIGGER_END(tnr_chrmtx_ptr);
  } else {
    /* Gain based */
    trigger_input = aec_trigger->gain;
    hyst_trigger_strt = GET_TNR_GAIN_TRIGGER_START(tnr_chrmtx_ptr);
    hyst_trigger_end = GET_TNR_GAIN_TRIGGER_END(tnr_chrmtx_ptr);
  }

  /* Update the tnr hysteresis status*/
  CPP_TNR_LOW("Previous TNR hystersis state %d", hw_params->hyst_tnr_status);
  hw_params->hyst_tnr_status = cpp_module_utils_get_hysteresis_trigger(
    trigger_input, hyst_trigger_strt,
    hyst_trigger_end, hw_params->hyst_tnr_status);

  CPP_TNR_LOW("type %d input %f start %f end %f prev %d",
    tnr_hysteresis_trigger_type, trigger_input,
    hyst_trigger_strt, hyst_trigger_end, hw_params->hyst_tnr_status);

  return 0;
}

/** cpp_module_util_get_pbf_hystersis_info
 *
 *  @chromatix_cpp - This parameter holds the preview chromatix pointer.
 *  @hw_params - This parameter holds the sessions hw params.
 *  @aec_trigger - This parameter holds the aec trigger value.
 *
 *  This function is used to extracts the trigger and calculate and update
 *  the hystersis value.
 *
 *  Return: Returns 0 on success error otherwise
 **/
int32_t cpp_module_util_get_pbf_hystersis_info(
  chromatix_cpp_type *cpp_chromatix_ptr, cpp_hardware_params_t *hw_params,
  cpp_params_aec_trigger_info_t *aec_trigger) {

  float                       hyst_trigger_strt;
  float                       hyst_trigger_end;
  float                       trigger_input;
  tuning_control_type       pbf_hysteresis_trigger_type;
  Chromatix_prescaler_bf_type *pbf_chrmtx_ptr = NULL;

  pbf_chrmtx_ptr = &cpp_chromatix_ptr->chromatix_pbf_data;

  pbf_hysteresis_trigger_type = GET_PBF_HYSTERESIS_TRIGGER_TYPE(pbf_chrmtx_ptr);
  if(pbf_hysteresis_trigger_type == 0) {
    /* Lux index based */
    trigger_input = aec_trigger->lux_idx;
    hyst_trigger_strt = GET_PBF_HYSTERESIS_LUXIDX_TRIGER_START(pbf_chrmtx_ptr);
    hyst_trigger_end = GET_PBF_HYSTERESIS_LUXIDX_TRIGER_END(pbf_chrmtx_ptr);
  } else {
    /* Gain based */
    trigger_input = aec_trigger->gain;
    hyst_trigger_strt = GET_PBF_HYSTERESIS_GAIN_TRIGER_START(pbf_chrmtx_ptr);
    hyst_trigger_end = GET_PBF_HYSTERESIS_GAIN_TRIGER_END(pbf_chrmtx_ptr);
  }

  /* Update the pbf hysteresis status*/
  CPP_TNR_LOW("Previous PBF hystersis state %d", hw_params->hyst_pbf_status);
  hw_params->hyst_pbf_status = cpp_module_utils_get_hysteresis_trigger(
    trigger_input, hyst_trigger_strt,
    hyst_trigger_end, hw_params->hyst_pbf_status);

  CPP_TNR_LOW("type %d input %f start %f end %f prev %d",
    pbf_hysteresis_trigger_type, trigger_input,
    hyst_trigger_strt, hyst_trigger_end, hw_params->hyst_pbf_status);

  return 0;
}

/** cpp_module_util_get_wnr_rnr_hystersis_info
 *
 *  @chromatix_cpp - This parameter holds the preview chromatix pointer.
 *  @hw_params - This parameter holds the sessions hw params.
 *  @aec_trigger - This parameter holds the aec trigger value.
 *
 *  This function is used to extracts the trigger and calculate and update
 *  the hystersis value.
 *
 *  Return: Returns 0 on success error otherwise
 **/
int32_t cpp_module_util_get_wnr_rnr_hystersis_info(chromatix_cpp_type *chromatix_cpp,
  cpp_hardware_params_t *hw_params, cpp_params_aec_trigger_info_t *aec_trigger)
{
  float trigger_input;
  float wnr_rnr_hyst_trigger_strt;
  float wnr_rnr_hyst_trigger_end;

#if ((defined (CHROMATIX_308) || defined (CHROMATIX_308E) || \
  defined (CHROMATIX_309) || defined (CHROMATIX_310) || defined (CHROMATIX_310E)) && \
  defined(CAMERA_USE_CHROMATIX_HW_WNR_TYPE))
  Chromatix_hardware_wavelet_type   *wavelet_denoise;
#else
  wavelet_denoise_type *wavelet_denoise;
  goto end;
#endif

  GET_WAVELET_DENOISE_POINTER(chromatix_cpp, hw_params, wavelet_denoise);
  if (wavelet_denoise->control_denoise == 0) {
    CPP_DENOISE_LOW("lux triggered");
    /* Lux index based */
    trigger_input = aec_trigger->lux_idx;
    wnr_rnr_hyst_trigger_strt = GET_WNR_RNR_LUXIDX_TRIGGER_START(wavelet_denoise);
    wnr_rnr_hyst_trigger_end = GET_WNR_RNR_LUXIDX_TRIGGER_END(wavelet_denoise);
  } else {
    CPP_DENOISE_LOW("gain triggered");
    /* Gain based */
    trigger_input = aec_trigger->gain;
    wnr_rnr_hyst_trigger_strt = GET_WNR_RNR_GAIN_TRIGGER_START(wavelet_denoise);
    wnr_rnr_hyst_trigger_end = GET_WNR_RNR_GAIN_TRIGGER_END(wavelet_denoise);
  }

  CPP_DENOISE_DBG("Previous WNR RNR hystersis state %d", hw_params->hyst_dsdn_status);
  hw_params->hyst_wnr_rnr_status =
    cpp_module_utils_get_hysteresis_trigger(trigger_input,
    wnr_rnr_hyst_trigger_strt, wnr_rnr_hyst_trigger_end,
    hw_params->hyst_wnr_rnr_status);

  CPP_DENOISE_DBG("### WNR RNR HYSTERSIS trigger_input %f, trigger start %f,"
  "trigger end %f, prev dsdn hyst_status %d",
   trigger_input, wnr_rnr_hyst_trigger_strt, wnr_rnr_hyst_trigger_end,
   hw_params->hyst_wnr_rnr_status);

end:
  return 0;
}

/** cpp_module_util_get_asf_rnr_hystersis_info
 *
 *  @chromatix_cpp - This parameter holds the preview chromatix pointer.
 *  @hw_params - This parameter holds the sessions hw params.
 *  @aec_trigger - This parameter holds the aec trigger value.
 *
 *  This function is used to extracts the trigger and calculate and update
 *  the hystersis value.
 *
 *  Return: Returns 0 on success error otherwise
 **/
int32_t cpp_module_util_get_asf_rnr_hystersis_info(chromatix_cpp_type *chromatix_cpp,
  cpp_hardware_params_t *hw_params, cpp_params_aec_trigger_info_t *aec_trigger)
{

  float                       hyst_trigger_strt;
  float                       hyst_trigger_end;
  float                       trigger_input;
  tuning_control_type         asf_rnr_hysteresis_trigger_type;
  chromatix_ASF_9x9_type     *asf_chrmtx_ptr = NULL;

  asf_chrmtx_ptr = &chromatix_cpp->chromatix_ASF_9x9;

  asf_rnr_hysteresis_trigger_type = GET_ASF_RNR_HYSTERESIS_TRIGGER_TYPE(asf_chrmtx_ptr);
  if(asf_rnr_hysteresis_trigger_type == 0) {
    /* Lux index based */
    trigger_input = aec_trigger->lux_idx;
    hyst_trigger_strt = GET_ASF_RNR_HYSTERESIS_LUXIDX_TRIGER_START(asf_chrmtx_ptr);
    hyst_trigger_end = GET_ASF_RNR_HYSTERESIS_LUXIDX_TRIGER_END(asf_chrmtx_ptr);
  } else {
    /* Gain based */
    trigger_input = aec_trigger->gain;
    hyst_trigger_strt = GET_ASF_RNR_HYSTERESIS_GAIN_TRIGER_START(asf_chrmtx_ptr);
    hyst_trigger_end = GET_ASF_RNR_HYSTERESIS_GAIN_TRIGER_END(asf_chrmtx_ptr);
  }

  /* Update the pbf hysteresis status*/
  CPP_ASF_DBG("Previous ASF RNR hystersis state %d", hw_params->hyst_pbf_status);
  hw_params->hyst_asf_rnr_status = cpp_module_utils_get_hysteresis_trigger(
    trigger_input, hyst_trigger_strt,
    hyst_trigger_end, hw_params->hyst_asf_rnr_status);

  CPP_ASF_DBG("type %d input %f start %f end %f prev %d",
    asf_rnr_hysteresis_trigger_type, trigger_input,
    hyst_trigger_strt, hyst_trigger_end, hw_params->hyst_asf_rnr_status);

  return 0;
}

/** cpp_module_util_get_hystersis_info
 *
 *  @module - This parameter holds the mct module type.
 *  @event - This parameter holds the mct event.
 *
 *  This function is used to calculate and update the hystersis
 *  status of module based on hysteresis trigger
 *
 *  Return: Returns 0 on success error otherwise
 **/


/** cpp_module_util_get_hystersis_info
 *
 *  @module - This parameter holds the mct module type.
 *  @event - This parameter holds the mct event.
 *
 *  This function is used to calculate and update the hystersis
 *  status of module based on hysteresis trigger
 *
 *  Return: Returns 0 on success error otherwise
 **/
int32_t cpp_module_util_get_hystersis_info(mct_module_t* module, mct_event_t *event)
{
  modulesChromatix_t *module_chromatix;
  chromatix_cpp_type *chromatix_cpp;
  cpp_module_ctrl_t           *ctrl;
  cpp_module_session_params_t *session_params;
  cpp_module_stream_params_t *stream_params;
  stats_update_t               *stats_update;
  aec_update_t                 *aec_update;
  int32_t                       rc = 0;
  cpp_params_aec_trigger_info_t aec_trigger;
  cpp_per_frame_params_t      *per_frame_params;
  cam_dyn_img_data_t dyn_img_data;
  mct_event_control_parm_t event_parm;
  mct_event_t l_event;


  ctrl = (cpp_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CPP_DENOISE_ERR("invalid cpp ctrl, failed\n");
    rc = -EFAULT;
    goto end;
  }

  if (ctrl->cpphw->hwinfo.version != CPP_HW_VERSION_6_0_0 &&
    ctrl->cpphw->hwinfo.version != CPP_HW_VERSION_6_1_0 &&
    ctrl->cpphw->hwinfo.version != CPP_HW_VERSION_6_1_2 &&
    ctrl->cpphw->hwinfo.version != CPP_HW_VERSION_6_1_4) {
    goto end;
  }

  cpp_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if (!session_params || !stream_params) {
    CPP_DENOISE_ERR("error: Invalid session %p or stream %p params\n",
      session_params, stream_params);
    rc = -EINVAL;
    goto end;
  }

  /* Hystersis updated for offline is already in metadata. Do not re-compute */
  if (stream_params->hw_params.stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    goto end;
  }

  stats_update =
      (stats_update_t *)event->u.module_event.module_event_data;
  aec_update = &stats_update->aec_update;

  if (stats_update->flag & STATS_UPDATE_AEC) {
    aec_trigger.gain = aec_update->real_gain;
    aec_trigger.lux_idx = aec_update->lux_idx;
  } else {
    CPP_DENOISE_LOW("No change in AEC - hyst status dsdn %d tnr %d pbf %d ",
      session_params->hw_params.hyst_dsdn_status,
      session_params->hw_params.hyst_tnr_status,
      session_params->hw_params.hyst_pbf_status);
     goto end;
  }

  module_chromatix = &session_params->module_chromatix;
  if (!module_chromatix) {
    CPP_DENOISE_ERR("error: Invalid module_chromatix\n");
    rc = -EINVAL;
    goto end;
  }

  chromatix_cpp =  (chromatix_cpp_type *)module_chromatix->chromatixCppPtr;
  if (!chromatix_cpp) {
    CPP_DENOISE_ERR("error: Invalid cpp chromatix\n");
    rc = -EINVAL;
    goto end;
  }

  CPP_DENOISE_DBG("Hystersis for frame %d",
    event->u.module_event.current_frame_id);
  cpp_module_util_get_cds_hystersis_info(chromatix_cpp,
    &session_params->hw_params, &aec_trigger);
  cpp_module_util_get_asf_rnr_hystersis_info(chromatix_cpp,
    &session_params->hw_params, &aec_trigger);

  cpp_module_util_get_tnr_hystersis_info(chromatix_cpp,
  &session_params->hw_params, &aec_trigger);
  cpp_module_util_get_pbf_hystersis_info(chromatix_cpp,
    &session_params->hw_params, &aec_trigger);

   /* generate a set param event and push it to perframe queue */
   memset(&dyn_img_data, 0x0, sizeof(cam_dyn_img_data_t));
   dyn_img_data.dyn_feature_mask =
     (session_params->hw_params.hyst_dsdn_status << DYN_IMG_CDS_HYS_BIT) |
     (session_params->hw_params.hyst_tnr_status << DYN_IMG_TNR_HYS_BIT) |
     (session_params->hw_params.hyst_pbf_status << DYN_IMG_PBF_HYS_BIT) |
     (session_params->hw_params.hyst_pbf_status << DYN_IMG_ASF_RNR_HYS_BIT);
   /* send Dynamic feature data event */
   l_event.identity =  event->identity;
   l_event.direction = MCT_EVENT_DOWNSTREAM;
   l_event.type = MCT_EVENT_CONTROL_CMD;
   l_event.timestamp = 0;
   l_event.u.ctrl_event.type = MCT_EVENT_CONTROL_SET_PARM;
   l_event.u.ctrl_event.control_event_data = &event_parm;
   l_event.u.ctrl_event.current_frame_id = event->u.module_event.current_frame_id;
   event_parm.type = CAM_INTF_META_IMG_DYN_FEAT;
   event_parm.parm_data =  (void *)&dyn_img_data;
   cpp_module_handle_set_parm_event(module, &l_event);

end:
  return rc;
}

/** cpp_module_utils_get_hysteresis_trigger
 *
 *  @trigger_input - This parameter holds the input trigger for interpolation
 *  @trigger_start - This parameter holds the start threshold to keep the module
 *   disabled.
 *  @trigger_end - This parameter holds the end threshold to keep the module
 *   enabled
 *  @prev_status -  This parameter holds previous status of the module.
 *
 *  This function is used to get the state of module based on hysteresis trigger
 *
 *  Return: Returns state of the module based on hysteresis
 *  (true enable / false disable).
 **/
bool cpp_module_utils_get_hysteresis_trigger(float trigger_input,
  float trigger_start, float trigger_end, bool prev_state)
{

  /* No trigger, assume hysteresis trigg returns true (do not disable module) */
  if ((F_EQUAL(trigger_start, 0.0f)) || (F_EQUAL(trigger_end, 0.0f)))
    return true;

  /* trigger i/p < trigger start - disable module based on hystersis trigger */
  if (trigger_input < trigger_start)
    return false;
  /* trigger i/p >= trigger end - enable module based on hystersis trigger */
  else if ((F_EQUAL(trigger_input, trigger_end)) || (trigger_input > trigger_end))
    return true;
  /* trigger between start and end threshold - return previous status */
  else
      return prev_state;
}

int32_t cpp_module_util_check_per_frame_limits(cpp_module_ctrl_t *ctrl,
  uint32_t identity, uint32_t cur_frame_id, cam_stream_ID_t *valid_stream_ids)
{
  uint32_t                      i = 0, j = 0;
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;

  /* Scan through the real time streams and handle empty buffer done
     for those streams for which buffer divert has raced compared to
     Super-param (CAM_INTF_META_STREAM_ID) */
  cpp_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if (!session_params || !stream_params || !valid_stream_ids) {
    CPP_BUF_ERR("session_params: %p, stream_params: %p valid_stream_ids: %p\n",
      session_params, stream_params, valid_stream_ids);
    return 0;
  }

  for (j = 0; j < valid_stream_ids->num_streams; j++) {
    for (i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
      if (session_params->stream_params[i] &&
        (valid_stream_ids->stream_request[j].streamID ==
        CPP_GET_STREAM_ID(session_params->stream_params[i]->identity)) &&
        (session_params->stream_params[i]->stream_type !=
        CAM_STREAM_TYPE_OFFLINE_PROC)) {
        PTHREAD_MUTEX_LOCK(&(session_params->stream_params[i]->mutex));
        if (session_params->stream_params[i]->cur_frame_id >=
          cur_frame_id) {
          PTHREAD_MUTEX_LOCK(&(ctrl->cpp_mutex));
          if (ctrl->cpp_thread_started) {
            cpp_module_event_t           *isp_buffer_drop_event = NULL;
            cpp_thread_msg_t             msg;
            isp_buffer_drop_event =
              (cpp_module_event_t*)CAM_MALLOC(sizeof(cpp_module_event_t));
            if(!isp_buffer_drop_event) {
              CPP_BUF_ERR("CAM_MALLOC failed\n");
              PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
              return -ENOMEM;
            }
            memset(isp_buffer_drop_event, 0x00, sizeof(cpp_module_event_t));
            isp_buffer_drop_event->hw_process_flag = TRUE;
            isp_buffer_drop_event->type = CPP_MODULE_EVENT_ISP_BUFFER_DROP;
            isp_buffer_drop_event->invalid = FALSE;
            isp_buffer_drop_event->u.drop_buffer.frame_id = cur_frame_id;
            isp_buffer_drop_event->u.drop_buffer.stream_params =
              session_params->stream_params[i];

            CPP_BUF_ERR("SOF drop for id %x frame %d \n",
              session_params->stream_params[i]->identity,
              isp_buffer_drop_event->u.drop_buffer.frame_id);

            cpp_module_enq_event(ctrl, isp_buffer_drop_event,
              CPP_PRIORITY_REALTIME);

            memset(&msg, 0, sizeof(cpp_thread_msg_t));
            msg.type = CPP_THREAD_MSG_NEW_EVENT_IN_Q;
            cpp_module_post_msg_to_thread(ctrl, msg);
          } else {
            CPP_BUF_ERR("Thread not active");
          }
          PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
          valid_stream_ids->stream_request[j].streamID = 0;
        }
        PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));
        break;
      }
    }
  }

  return 0;
}

/** cpp_module_util_check_event_params
 *
 * @ module - event originating mct module
 * @ event  - event data to be handled
 *
 * This function validates the event parameters and returns session, stream
 * specific parameters, as needed.
 *
 **/
int32_t cpp_module_util_check_event_params(mct_module_t *module,
  mct_event_t *event,
  cpp_module_session_params_t **session_params,
  cpp_module_stream_params_t **stream_params)
{
  cpp_module_ctrl_t *ctrl = NULL;
  cpp_module_stream_params_t *l_stream_params = NULL;
  cpp_module_session_params_t *l_session_params = NULL;

  /* Validate input parameters */
  if (!(module && event)) {
    CPP_ERR("failed, module=%p, event=%p\n", module, event);
    return -EINVAL;
  }

  ctrl = (cpp_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  if (!ctrl) {
    CPP_ERR("invalid cpp control, failed\n");
    return -EFAULT;
  }

  /* Get stream parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl,
    event->identity,
    &l_session_params,
    &l_stream_params);
  if (!l_stream_params) {
    CPP_ERR("invalid stream params, failed\n");
    return -EFAULT;
  }

  if (session_params) {
    *session_params = l_session_params;
  }
  if (stream_params) {
    *stream_params = l_stream_params;
  }

  return 0;
}

/** cpp_module_util_map_rot_to_string
 *
 *
 * @string - string param that holds the string literal for rotation
 * @rot - rotation value
 *
 * This function provides a mapping from the rotation value to string
 *
 **/

void cpp_module_util_map_rot_to_string(uint32_t rot_cfg, char *rot)
{

  switch (rot_cfg) {
    case ROT_0:
      memcpy(rot, "rot_0", sizeof("rot_0"));
      break;
    case ROT_0_H_FLIP:
      memcpy(rot, "rot_0_h_flip", sizeof("rot_0_h_flip"));
      break;
    case ROT_0_V_FLIP:
      memcpy(rot, "rot_0_v_flip", sizeof("rot_0_v_flip"));
      break;
    case ROT_0_HV_FLIP:
      memcpy(rot, "rot_0_hv_flip", sizeof("rot_0_hv_flip"));
      break;
    case ROT_90:
      memcpy(rot, "rot_90", sizeof("rot_90"));
      break;
    case ROT_90_H_FLIP:
      memcpy(rot, "rot_90_h_flip", sizeof("rot_90_h_flip"));
      break;
    case ROT_90_V_FLIP:
      memcpy(rot, "rot_90_v_flip", sizeof("rot_90_v_flip"));
      break;
    case ROT_90_HV_FLIP:
      memcpy(rot, "rot_90_hv_flip", sizeof("rot_90_hv_flip"));
      break;
    default:
      break;
  }
}

/** cpp_module_util_map_stream_to_string
 *
 *
 * @string - string param that holds the string literal for stream type
 * @stream type - stream type
 *
 * This function provides a mapping from the stream type to string
 *
 **/

void cpp_module_util_map_stream_to_string(uint32_t stream_type, char *stream)
{
  switch (stream_type) {
    case CAM_STREAM_TYPE_PREVIEW:
      memcpy(stream, "preview", sizeof("preview"));
      break;
    case CAM_STREAM_TYPE_POSTVIEW:
      memcpy(stream, "postview", sizeof("postview"));
      break;
    case CAM_STREAM_TYPE_SNAPSHOT:
      memcpy(stream, "snapshot", sizeof("snapshot"));
      break;
    case CAM_STREAM_TYPE_VIDEO:
      memcpy(stream, "video", sizeof("video"));
      break;
    case CAM_STREAM_TYPE_CALLBACK:
      memcpy(stream, "callback", sizeof("callback"));
      break;
    case CAM_STREAM_TYPE_IMPL_DEFINED:
      memcpy(stream, "impl_defined", sizeof("impl_defined"));
      break;
    case CAM_STREAM_TYPE_METADATA:
      memcpy(stream, "metadata", sizeof("metadata"));
      break;
    case CAM_STREAM_TYPE_RAW:
      memcpy(stream, "raw", sizeof("raw"));
      break;
    case CAM_STREAM_TYPE_OFFLINE_PROC:
      memcpy(stream, "offline", sizeof("offline"));
      break;
    case CAM_STREAM_TYPE_PARM:
      memcpy(stream, "session", sizeof("session"));
      break;
    case CAM_STREAM_TYPE_ANALYSIS:
      memcpy(stream, "analysis", sizeof("analysis"));
      break;
    default:
      memcpy(stream, "default", sizeof("default"));
      break;
  }
}

/** cpp_module_util_dumps_feature_info
 *
 *  @cpphw - This parameter holds the cpp hardware data structure.
 *  @hw_params - This parameter holds the cpp hardware params data structure.
 *  @frame_info - This parameter holds the cpp frame info data structure.
 *  @msm_cpp_frame_info -  This parameter holds the kernel cpp frame info data
 *  structure.
 *
 *  This function is used to extract the cpp feature enablement info from
 *  cpp hardware params, cpp frame info, and the frame message to dump the
 *  debug information
 *
 *  Return: void return type.
 *
 **/

void cpp_module_util_dumps_feature_info(cpp_hardware_t *cpphw,
  cpp_hardware_params_t *hw_params,
  struct cpp_frame_info_t* frame_info,
  struct msm_cpp_frame_info_t *msm_cpp_frame_info)
{

  uint32_t i = 0;
  /* check the bit maks if dump needed */
  if (IS_VALID_MASK(g_cpp_log_featureMask, CPP_FEATURE_BIT)) {

    uint32_t *frame_msg =  msm_cpp_frame_info->cpp_cmd_msg;
    static uint32_t plane_base_index = 0;
    static uint32_t plane_base_size = 0;
    static uint32_t rnr_wnr_idx = 0, rnr_asf_idx = 0, rnr_wnr_offset = 0;
    if (!frame_msg) {
      CPP_ERR("invalid frame msg");
      goto end;
    }
    if (!plane_base_index || !plane_base_size) {
      switch (cpphw->hwinfo.version) {
        case CPP_HW_VERSION_6_0_0:
          plane_base_index = 481;
          plane_base_size =  24;
        break;
        case CPP_HW_VERSION_5_1_0:
        case CPP_HW_VERSION_5_0_0:
          plane_base_index = 478;
          plane_base_size = 5;
        break;
        case CPP_HW_VERSION_4_3_4:
        case CPP_HW_VERSION_4_3_3:
        case CPP_HW_VERSION_4_3_2:
        case CPP_HW_VERSION_4_3_0:
          plane_base_index = 141;
          plane_base_size = 5;
        break;
        case CPP_HW_VERSION_6_1_0:
        case CPP_HW_VERSION_6_1_2:
        case CPP_HW_VERSION_6_1_4:
          plane_base_index = 715;
          plane_base_size = 25;
          rnr_asf_idx = 635;
          rnr_wnr_idx = 406;
          rnr_wnr_offset = 80;
        break;
        default:
          goto end;
        break;
      }
    }

    char stream[256];
    cpp_module_util_map_stream_to_string(hw_params->stream_type, stream);

    char rot[256] = "rot_0";
    cpp_module_util_map_rot_to_string(
      frame_info->plane_info[0].stripe_info1[0].rot_info.rot_cfg, rot);

    uint32_t indx = 0, rnr_idx = rnr_wnr_idx;
    CPP_FEATURE_INFO("stream %s, Frame id %d, identity %x, session %d," \
      "duplication %s, batch mode %s, batch size %d, ubwc %s," \
      "power_collapse %s, rotation %s, scale %s", \
      stream, frame_info->frame_id,
      frame_info->identity, CPP_GET_SESSION_ID(frame_info->identity),
      frame_info->dup_output ? "on" : "off",
      frame_info->batch_info.batch_mode ? "on" : "off",
      frame_info->batch_info.batch_size,
      (UBWC_ENABLE(hw_params->output_info.plane_fmt)) ? "on" : "off",
      !(frame_info->power_collapse) ? "on" : "off",
      rot, ((F_EQUAL(hw_params->cpp_scale_ratio, 1.0f))  ? "no scale" :
      (hw_params->cpp_scale_ratio < 1.0f) ? "upscale" : "downscale"));
    for (i = 0; i < frame_info->num_planes; i++) {
      indx = plane_base_index + (i * plane_base_size);
      CPP_FEATURE_INFO(" stream %s %s %d: wnr  %s, asf %s, dsdn %s, tnr %s, pbf %s",
        stream, ((!i) ? "LUMA" : "CHROMA"), i,
        ((frame_msg[indx] & 0x1) ? "on" : "off"),
        (((frame_msg[indx] & 0x8) >> 3) ? "on" : "off"),
        (((frame_msg[indx] & 0x2000000) >> 25) ? "on" : "off"),
        (((frame_msg[indx] & 0x40) >> 6) ? "on" : "off"),
        (((frame_msg[indx] & 0x100) >> 8) ? "on" : "off"));
      if (rnr_wnr_idx && rnr_wnr_offset) {
        CPP_FEATURE_INFO("stream %s, plane %s: "
          "WNR-RNR %s, Trilateral filter %s, padding mode %s",
          stream, ((!i) ? "LUMA" : "CHROMA"),
          ((((frame_msg[rnr_idx] >> 28) & 0x1) &&
          ((frame_msg[rnr_idx + 1] >> 28) & 0x1) &&
          ((frame_msg[rnr_idx + 2] >> 28) & 0x1) &&
          ((frame_msg[rnr_idx + 3] >> 28) & 0x1)) ? "on" : "off"),
          ((((frame_msg[rnr_idx] >> 29) & 0x1) &&
          ((frame_msg[rnr_idx + 1] >> 29) & 0x1) &&
          ((frame_msg[rnr_idx + 2] >> 29) & 0x1) &&
          ((frame_msg[rnr_idx + 3] >> 29) & 0x1)) ? "on" : "off"),
          ((((frame_msg[rnr_idx] >> 24) & 0x1) &&
          ((frame_msg[rnr_idx + 1] >> 24) & 0x1) &&
          ((frame_msg[rnr_idx + 2] >> 24) & 0x1) &&
          ((frame_msg[rnr_idx + 3] >> 24) & 0x1)) ? "on" : "off")
        );
        rnr_idx += rnr_wnr_offset;
      }
    }
    if (rnr_asf_idx) {
      CPP_FEATURE_INFO(" stream %s ASF-RNR %s, ASF-Symmetric filter %s",
        stream, (((frame_msg[rnr_asf_idx] >> 24) & 0x1) ? "on" : "off"),
        (((frame_msg[rnr_asf_idx] >> 18 ) & 0x1) ? "on" : "off"));
    }
  }
end:
  return;
}

boolean cpp_module_util_is_cpp_downscale_enabled(uint32_t width,
  uint32_t height, cam_stream_type_t stream_type,
  cam_feature_mask_t pp_mask)
{

  float aspect_ratio = 0.0f;
  uint32_t ref_width, ref_height = 0;
  char value[PROPERTY_VALUE_MAX];
  uint32_t override_cpp_ds_feature_mask = 0;
  aspect_ratio = (float)width/height;

  if(stream_type != CAM_STREAM_TYPE_PREVIEW &&
     stream_type != CAM_STREAM_TYPE_VIDEO) {
     CPP_LOW("Unsupported streamtype for CPP DS!");
     return FALSE;
  }
  property_get("persist.vendor.camera.cpp.dsoverride", value, "0");
  override_cpp_ds_feature_mask = atoi(value);

  if((pp_mask & CAM_QTI_FEATURE_CPP_DOWNSCALE) == 0) {
    CPP_LOW("pp mask for CPP DS not set!");
    if(override_cpp_ds_feature_mask) {
      CPP_LOW("CAM_QTI_FEATURE_CPP_DOWNSCALE not set but overriding!");
    } else {
      return FALSE;
    }
  }

  if(aspect_ratio <= 1.0f) {
    CPP_LOW("Aspect Ratio 1:1!");
    ref_width = 1440;
    ref_height = 1440;
  } else if (aspect_ratio > 1.0f && aspect_ratio <= (float)4/3) {
     CPP_LOW("Aspect Ratio 4/3! calc %f ", (float)4/3);
    ref_width = 1920;
    ref_height = 1440;
  } else if (aspect_ratio > (float)4/3 && aspect_ratio <= (float)16/9) {
    CPP_LOW("Aspect Ratio 16/9! calc %f", (float)16/9);
    ref_width = 1920;
    ref_height = 1080;
  } else {   //(aspect_ratio > (float)16/9)
    CPP_LOW("Aspect Ratio greater than 16/9!");
    ref_width = 1920;
    ref_height = 1440;
  }

  if(width >= ref_width && height >= ref_height) {
    CPP_ERR("Streams dim %dx%d bigger than ref dims %dx%d",
      width, height, ref_width, ref_height);
    return FALSE;
  }

  CPP_ERR("Stream type %d Original dim %dX%d reference dim %dX%d", stream_type, width, height,
    ref_width, ref_height);
  return TRUE;
}

void cpp_module_util_get_max_downscale_dim(uint32_t width,
  uint32_t height, uint32_t *max_width, uint32_t *max_height,
  cam_feature_mask_t pp_mask, cam_format_t format)
{
  float max_downscale_limit = CPP_DOWNSCALE_LIMIT;
  float height_ratio, width_ratio = 0.0f;
  float aspect_ratio = 0.0f;

  aspect_ratio = (float)width/height;

  if(aspect_ratio <= 1.0f) {
    CPP_LOW("Aspect Ratio 1:1!");
    *max_width = 1440;
    *max_height = 1440;
  } else if (aspect_ratio > 1.0f && aspect_ratio <= (float)4/3) {
     CPP_LOW("Aspect Ratio 4/3! calc %f ", (float)4/3);
    *max_width = 1920;
    *max_height = 1440;
  } else if (aspect_ratio > (float)4/3 && aspect_ratio <= (float)16/9) {
    CPP_LOW("Aspect Ratio 16/9! calc %f", (float)16/9);
    *max_width = 1920;
    *max_height = 1080;
  } else {   //(aspect_ratio > (float)16/9)
    CPP_LOW("Aspect Ratio greater than 16/9!");
    *max_width = 1920;
    *max_height = 1440;
  }

  if((pp_mask & CAM_QCOM_FEATURE_CPP_TNR) &&
    (format == CAM_FORMAT_YUV_420_NV12_UBWC)) {
    max_downscale_limit = CPP_DOWNSCALE_LIMIT_TNR_UBWC;
  } else if(pp_mask & CAM_QCOM_FEATURE_CPP_TNR) {
    max_downscale_limit = CPP_DOWNSCALE_LIMIT_TNR;
  }else if (format == CAM_FORMAT_YUV_420_NV12_UBWC) {
    max_downscale_limit = CPP_DOWNSCALE_LIMIT_UBWC;
  }

  width_ratio = (float)*max_width /width;
  height_ratio = (float)*max_height/height;

  CPP_LOW(" wr %f hr %f max_ds_limit %f", width_ratio,height_ratio,  max_downscale_limit);
  if(width_ratio > max_downscale_limit || height_ratio > max_downscale_limit) {
    CPP_ERR("Updating initial dims to req dims");
    *max_width = width * (int)max_downscale_limit;
    *max_height = height * (int)max_downscale_limit;
  }

  CPP_ERR("Original dim %dX%d req dim %dX%d", width, height,
    *max_width, *max_height);

}

int32_t cpp_module_util_inc_clock_counter_l(
  cpp_module_session_params_t *session_params __unused,
  cpp_module_stream_params_t *stream_params)
{
  stream_params->process_delay++;
  CPP_CLOCK_DBG("CLK: inc stream %d, process_delay %d",
    stream_params->stream_type, stream_params->process_delay);
  return 0;
}

/* call from buffer divert */
int32_t cpp_module_util_inc_clock_counter(
  cpp_module_session_params_t *session_params,
  cpp_module_stream_params_t *stream_params)
{
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  cpp_module_util_inc_clock_counter_l(session_params, stream_params);
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
  return 0;
}

int32_t cpp_module_util_dec_clock_counter_l(
  cpp_module_session_params_t *session_params __unused,
  cpp_module_stream_params_t *stream_params)
{
  if (stream_params->process_delay)
    stream_params->process_delay--;
  CPP_CLOCK_DBG("CLK: dec stream %d, process_delay %d",
    stream_params->stream_type, stream_params->process_delay);
  return 0;
}

int32_t cpp_module_util_dec_clock_counter(
  cpp_module_session_params_t *session_params,
  cpp_module_stream_params_t *stream_params)
{
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  cpp_module_util_dec_clock_counter_l(session_params, stream_params);
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
  return 0;
}

int32_t cpp_module_determine_perf_bottlneck(
  cpp_module_ctrl_t *ctrl,
  cpp_module_session_params_t *session_params __unused,
  cpp_module_stream_params_t *stream_params)
{
  int32_t process_delay = 0;
  process_delay = (stream_params->process_delay && ctrl->clock_dcvs) ?
    (stream_params->process_delay - 1) : 0;
  return process_delay;
}

int32_t cpp_module_decide_runtime_clock_update(
  cpp_module_ctrl_t *ctrl,
  cpp_module_session_params_t *session_params,
  int32_t perf_bottleneck, int32_t dup_status, int32_t bump_down,
  int32_t offline, int32_t crop_based)
{
  PTHREAD_MUTEX_LOCK(&(ctrl->cpp_mutex));

  /* update state to bump up if performance bottle neck detected */
  if (perf_bottleneck) {
    ctrl->runtime_clk_update = TRUE;
    cpp_module_update_clock_status(ctrl, CPP_CLOCK_BUMP_UP);
    goto end;
  }

  /* if duplication is enabled bump down the clock */
  if (dup_status) {
    cpp_module_update_clock_status(ctrl, CPP_CLOCK_BUMP_DOWN);
    ctrl->runtime_clk_update = TRUE;
    goto end;
  }

  /* explicit bump down triggered from SOF */
  if (bump_down) {
    cpp_module_update_clock_status(ctrl, CPP_CLOCK_BUMP_DOWN);
    ctrl->runtime_clk_update = TRUE;
    goto end;

  }

  if (offline) {
    ctrl->runtime_clk_update = TRUE;
    goto end;
  }

  if (crop_based) {
    ctrl->runtime_clk_update = TRUE;
    goto end;
  }

  /* for HAL3 trigger update to handle request changes */
  if (session_params->hal_version == CAM_HAL_V3) {
    ctrl->runtime_clk_update = TRUE;
    goto end;
  }

end:
  CPP_CLOCK_DBG("CLK : runtime update %d, perf %d, dup %d, bump %d",
    ctrl->runtime_clk_update, perf_bottleneck, dup_status, bump_down);
  PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
  return 0;
}

int32_t cpp_module_init_clock_parameters_l(
  cpp_module_stream_params_t *stream_params, int64_t dim)
{
  stream_params->load_per_frame = dim;
  return 0;
}

int32_t cpp_module_init_clock_parameters(
  cpp_module_stream_params_t *stream_params, int64_t dim)
{
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  cpp_module_init_clock_parameters_l(
    stream_params, dim);
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
  return 0;
}

cpp_module_stream_clk_rate_t * cpp_module_get_clk_obj (
  cpp_module_ctrl_t *ctrl,
  cpp_module_stream_params_t *stream_params)
{
  cpp_module_stream_clk_rate_t *clk_rate_obj = NULL;
  PTHREAD_MUTEX_LOCK(&(ctrl->clk_rate_list.mutex));
  clk_rate_obj = cpp_module_find_clk_rate_by_identity(ctrl,
    stream_params->hw_params.identity);
  if (clk_rate_obj == NULL) {
    CPP_CLOCK_ERR("clk rate obj = NULL, identity = %x",
     stream_params->hw_params.identity);
  }
  PTHREAD_MUTEX_UNLOCK(&(ctrl->clk_rate_list.mutex));
  return clk_rate_obj;
}

int32_t cpp_module_update_process_delay_l(
  cpp_module_ctrl_t *ctrl,
  cpp_module_session_params_t *session_params,
  cpp_module_stream_params_t *stream_params,
  cpp_module_stream_clk_rate_t *clk_rate_obj,
  bool perf_bottleneck)
{
  if (clk_rate_obj) {
    /* Update the process delay and excess load for the stream */
    clk_rate_obj->process_delay = stream_params->process_delay;
    clk_rate_obj->excess_load_per_frame = (stream_params->process_delay &&
      ctrl->clock_dcvs) ? (stream_params->process_delay - 1) *
      stream_params->load_per_frame : 0;
    UPDATE_INPUT_LOAD(ctrl, session_params, stream_params, clk_rate_obj);
    if (perf_bottleneck) {
      /*
       * for offline stream if delay happens, update the excess load at
       * MINIMUM_PROCESS_TIME rate
       */
      if ((stream_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) &&
         (!clk_rate_obj->excess_load_per_frame))
      clk_rate_obj->excess_load_per_frame =  MINIMUM_PROCESS_TIME *
        stream_params->load_per_frame;
      CPP_CLOCK_DBG("CLK: stream %d, excess_load_per_frame %lld, delay %d",
        stream_params->stream_type, clk_rate_obj->excess_load_per_frame,
        clk_rate_obj->process_delay);
    }
  }
  return 0;
}

int32_t cpp_module_update_process_delay(
  cpp_module_ctrl_t *ctrl,
  cpp_module_session_params_t *session_params,
  cpp_module_stream_params_t *stream_params,
  cpp_module_stream_clk_rate_t *clk_rate_obj,
  bool perf_bottleneck)
{
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  cpp_module_update_process_delay_l(ctrl, session_params, stream_params,
    clk_rate_obj, perf_bottleneck);
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
  return 0;
}

int32_t cpp_module_update_threshold_idx(
  cpp_module_session_params_t *session_params,
  cpp_module_stream_params_t *stream_params __unused)
{
  session_params->clk_ref_threshold_idx =
    session_params->per_frame_params.cur_frame_id;
  CPP_CLOCK_HIGH("CLK: ref indx %d", session_params->clk_ref_threshold_idx);
  return 0;
}

int32_t cpp_module_handle_request_based_fallback(
  cpp_module_ctrl_t *ctrl,
  cpp_module_session_params_t *session_params)
{
  int32_t bump_down = 0;
  int32_t i = 0;

  /*
   * If not in bumped up state do not check if bump down condition
   * are met. transition to bump down only from bumped state.
   */
  PTHREAD_MUTEX_LOCK(&(ctrl->cpp_mutex));
  if (ctrl->clk_state != CPP_CLOCK_BUMPED_UP) {
    CPP_CLOCK_DBG("Not bumped up %d", ctrl->clk_state);
    PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
    goto end;
  }
  PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));

  /*
   * Check for all the sessions for the thresholds and make sure
   * all sessions satify the sof frame id greater than threshold
   * to bump down the clock
   */
  for (i = 0; i < CPP_MODULE_MAX_SESSIONS; i++) {
    if (ctrl->session_params[i]) {
      if ((ctrl->session_params[i]->clk_ref_threshold_idx +
        CLOCK_REFERENCE_THRESHOLD) >
        (int)session_params->per_frame_params.cur_frame_id) {
        bump_down = 0;
        break;
      } else {
        bump_down = 1;
      }
      CPP_CLOCK_HIGH("CLK: session %d, bump_down %d, thresh %d, frame %d",
        i, bump_down, ctrl->session_params[i]->clk_ref_threshold_idx,
        session_params->per_frame_params.cur_frame_id);
    }
  }
  if (bump_down) {
    CPP_CLOCK_DBG("CLK bump_down %d, do a runtime update", bump_down);
    cpp_module_decide_runtime_clock_update(ctrl, session_params,
      0, 0, bump_down, 0, 0);
  }
end:
  return 0;
}

int32_t cpp_module_is_clock_update_needed(
  cpp_module_ctrl_t *ctrl,
  cpp_module_session_params_t *session_params,
  cpp_module_stream_params_t *stream_params,
  int64_t clk, int64_t bw, int32_t force)
{
  bool update = true;
  PTHREAD_MUTEX_LOCK(&(ctrl->cpp_mutex));

  CPP_CLOCK_LOW("clk, bw :  (%lld , %lld), bw (%lld,  %lld)",
    clk, ctrl->clk, bw, ctrl->bw);

  /* force update clock if flag set */
  if (force) {
    CPP_CLOCK_DBG("CLOCK force update");
    goto end;
  }

  /* no clock update required if  clock and bw same as before */
  if (IS_CLOCK_BW_EQUAL(ctrl, clk, bw)) {
    CPP_CLOCK_DBG(" clk , bw :  (%lld , %lld), bw (%lld,  %lld)",
      clk, ctrl->clk, bw, ctrl->bw);
    cpp_module_update_clock_status(ctrl, CPP_CLOCK_STEADY);
    update = false;
    goto end;
  }

  /* Clock needs to be bumped down, update clock */
  if (ctrl->clk_state == CPP_CLOCK_BUMP_DOWN) {
    CPP_CLOCK_HIGH("CLOCK force bump down");
    goto end;
  }

  /*
   * calculated clock less than current,
   * trigger bump down sequence, update threshold
   */
  if(IS_CLOCK_BW_LESS(ctrl, clk, bw)) {
    CPP_CLOCK_DBG("LESS : clk , bw :  (%lld , %lld), bw (%lld,  %lld)",
      clk, ctrl->clk, bw, ctrl->bw);
    if (ctrl->clk_state != CPP_CLOCK_BUMPED_UP) {
      cpp_module_update_threshold_idx(session_params, stream_params);
      cpp_module_update_clock_status(ctrl, CPP_CLOCK_BUMPED_UP);
    }
    update = false;
    goto end;
  }

  /* Clock needs to be bumped up , update threshold */
  if (ctrl->clk_state == CPP_CLOCK_BUMP_UP) {
    CPP_CLOCK_DBG("CLOCK bump update");
    cpp_module_update_threshold_idx(session_params, stream_params);
    goto end;
  }

end:
  PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
  return update;
}

int32_t cpp_module_update_clock_status(cpp_module_ctrl_t *ctrl,
  cpp_clock_state state)
{
  CPP_CLOCK_DBG("current state %d", ctrl->clk_state);
  switch (state) {
    /* Update clock to steady state only if not in Bump up or bumped state. */
    case CPP_CLOCK_STEADY:
      if ((ctrl->clk_state != CPP_CLOCK_BUMP_UP) &&
        (ctrl->clk_state != CPP_CLOCK_BUMPED_UP))
        ctrl->clk_state = CPP_CLOCK_STEADY;
    break;
    case CPP_CLOCK_BUMP_UP:
      ctrl->clk_state = CPP_CLOCK_BUMP_UP;
    break;
    case CPP_CLOCK_BUMPED_UP:
      ctrl->clk_state = CPP_CLOCK_BUMPED_UP;
    break;
    case CPP_CLOCK_BUMP_DOWN:
      /* Update state to bump down only if not in bump up state. */
      if (ctrl->clk_state != CPP_CLOCK_BUMP_UP)
        ctrl->clk_state = CPP_CLOCK_BUMP_DOWN;
    break;
  }
  CPP_CLOCK_HIGH("updated state %d", ctrl->clk_state);
  return 0;
}


int32_t cpp_module_get_round_clock_rate(cpp_module_ctrl_t *ctrl,
  unsigned long *clk_rate, unsigned long *round_clk_rate,
  uint32_t *clock_index)
{
  uint32_t i = *clock_index;
  for (; i < ctrl->cpphw->hwinfo.freq_tbl_count; i++) {
    if (*clk_rate <= ctrl->cpphw->hwinfo.freq_tbl[i]) {
      *round_clk_rate = ctrl->cpphw->hwinfo.freq_tbl[i];
      *clock_index = i;
      break;
    }
  }
  if (i >= ctrl->cpphw->hwinfo.freq_tbl_count) {
    *round_clk_rate =
      ctrl->cpphw->hwinfo.freq_tbl[ctrl->cpphw->hwinfo.freq_tbl_count - 1];
    *clock_index = ctrl->cpphw->hwinfo.freq_tbl_count - 1;
  }
  CPP_CLOCK_DBG("round clock rate %d", *clock_index);
  return 0;
}


/* Update the round clock rate variable with update clock if bump up is done */
int32_t cpp_module_clock_bump(cpp_module_ctrl_t *ctrl,
   cpp_module_total_load_t *load, unsigned long *clk_rate,
   uint32_t clock_indx)
{
  /*
   * perf bottle neck present need bump in clock. band width already updated
   * with process delay.
   */
  if ((load->process_delay > 1) && (ctrl->clock_dcvs)) {
    CPP_INFO("CLOCK bumped-up delay %d",(load->process_delay - 1));
    clock_indx += (load->process_delay - 1);
    *clk_rate =
      (clock_indx >= ctrl->cpphw->hwinfo.freq_tbl_count) ?
      ctrl->cpphw->hwinfo.freq_tbl[ctrl->cpphw->hwinfo.freq_tbl_count - 1] :
      ctrl->cpphw->hwinfo.freq_tbl[clock_indx];
      cpp_module_update_clock_status(ctrl, CPP_CLOCK_BUMPED_UP);
  } else {
    CPP_CLOCK_DBG("set steady state");
    cpp_module_update_clock_status(ctrl, CPP_CLOCK_STEADY);
  }
  return 0;
}

boolean cpp_module_invalidate_clock_q_traverse(void* qdata,
  void* userdata __unused)
{
  if (!qdata) {
    CPP_CLOCK_ERR("failed, qdata=%p\n", qdata);
    return FALSE;
  }

  /* Invalidate the clock event pending in queue, if a new event is pending */
  cpp_module_event_t* cpp_event = (cpp_module_event_t *) qdata;
  if (cpp_event && (cpp_event->type == CPP_MODULE_EVENT_CLOCK)) {
    CPP_CLOCK_DBG("INVALIDATE CLOCK event clk %llu, bw %llu",
      cpp_event->u.clock_data.clk_rate, cpp_event->u.clock_data.bandwidth_avg);
    cpp_event->invalid = TRUE;
  }
  return TRUE;
}

int32_t cpp_invalidate_clock_event(cpp_module_ctrl_t *ctrl __unused,
  mct_queue_t *queue)
{
  mct_queue_traverse(queue, cpp_module_invalidate_clock_q_traverse, NULL);
  return 0;
}

int32_t cpp_module_set_clock_dcvs_params(cpp_module_ctrl_t *ctrl)
{

  char value[PROPERTY_VALUE_MAX] = "";
  property_get("persist.vendor.camera.cpp.clock.dcvs", value, "1");
  ctrl->clock_dcvs = atoi(value);

  property_get("persist.vendor.camera.cpp.turbo.caps", value, "0");
  ctrl->turbo_caps = atoi(value);

  property_get("persist.vendor.camera.cpp.clk.thresh", value, "0");
  /* trigger indicating  number of frames after which clock needs bump down */
  ctrl->clock_threshold = atoi(value) ? atoi(value) :
    CLOCK_REFERENCE_THRESHOLD;

  CPP_CLOCK_HIGH("clock_threshold %d, DCVS %d",
    ctrl->clock_threshold, ctrl->clock_dcvs);
  return 0;
}

float cpp_module_get_bytes_per_pixel(cpp_params_plane_fmt_t format)
{

    float bytes_per_pixel = 1.5f;

    switch (format) {
    case CPP_PARAM_PLANE_CRCB444:
    case CPP_PARAM_PLANE_CBCR444: {
      bytes_per_pixel = 3;
      break;
    }
    case CPP_PARAM_PLANE_CRCB422:
    case CPP_PARAM_PLANE_CBCR422: {
      bytes_per_pixel = 2;
      break;
    }
    default: {
      bytes_per_pixel = 1.5f;
      break;
    }
    }
    return bytes_per_pixel;
}

void cpp_module_util_apply_fixed_fov_margin(
  sensor_isp_stream_sizes_t *sensor_isp_info, uint32_t si)
{
  boolean apply_fovc_crop_margin = FALSE;

#define FOVC_MARGIN_ALIGN(value_of, align_to) \
  (((value_of) + (align_to - 1)) & ~(align_to - 1));

  /*
   * Use the CAM_QTI_FEATURE_FIXED_FOVC feature mask as this sequence
   * is initiated before the actual streams are created/started.
   */
  apply_fovc_crop_margin =
    (sensor_isp_info->postprocess_mask[si] & CAM_QTI_FEATURE_FIXED_FOVC) &&
    ((sensor_isp_info->type[si] == CAM_STREAM_TYPE_PREVIEW) ||
     (sensor_isp_info->type[si] == CAM_STREAM_TYPE_VIDEO)   ||
     (sensor_isp_info->type[si] == CAM_STREAM_TYPE_CALLBACK));

  if (apply_fovc_crop_margin == TRUE) {
    sensor_isp_info->margins[si].widthMargins +=
      (MCT_PIPELINE_FIXED_FOVC_FACTOR);
    sensor_isp_info->margins[si].heightMargins +=
      (MCT_PIPELINE_FIXED_FOVC_FACTOR);

    sensor_isp_info->stream_sz_plus_margin[si].width *=
      (1 + MCT_PIPELINE_FIXED_FOVC_FACTOR);
    sensor_isp_info->stream_sz_plus_margin[si].height *=
      (1 + MCT_PIPELINE_FIXED_FOVC_FACTOR);

    sensor_isp_info->stream_sz_plus_margin[si].width =
      FOVC_MARGIN_ALIGN(sensor_isp_info->stream_sz_plus_margin[si].width,
        2);
    sensor_isp_info->stream_sz_plus_margin[si].height =
      FOVC_MARGIN_ALIGN(sensor_isp_info->stream_sz_plus_margin[si].height,
        2);
  }
}

void cpp_module_util_apply_cpp_downscaler_margin(
  sensor_isp_stream_sizes_t *sensor_isp_info, uint32_t si)
{
  uint32_t max_width, max_height = 0;

  /* Update only for preview/video streams. (No Callback?) */
  if (cpp_module_util_is_cpp_downscale_enabled(
        sensor_isp_info->stream_sz_plus_margin[si].width,
        sensor_isp_info->stream_sz_plus_margin[si].height,
        sensor_isp_info->type[si],
        sensor_isp_info->postprocess_mask[si])) {
    CPP_LOW("CPP D enabled!\n");

    cpp_module_util_get_max_downscale_dim(
      sensor_isp_info->stream_sz_plus_margin[si].width,
      sensor_isp_info->stream_sz_plus_margin[si].height,
      &max_width,
      &max_height,
      sensor_isp_info->postprocess_mask[si],
      sensor_isp_info->stream_format[si]);

    sensor_isp_info->stream_sz_plus_margin[si].width = max_width;
    sensor_isp_info->margins[si].widthMargins =
      ((float)(sensor_isp_info->stream_sz_plus_margin[si].width -
        sensor_isp_info->stream_sizes[si].width) /
        sensor_isp_info->stream_sizes[si].width) * 100;

    sensor_isp_info->stream_sz_plus_margin[si].height = max_height;
    sensor_isp_info->margins[si].heightMargins =
      ((float)(sensor_isp_info->stream_sz_plus_margin[si].height -
        sensor_isp_info->stream_sizes[si].height) /
        sensor_isp_info->stream_sizes[si].height) * 100;

    CPP_LOW("Type %d stream %dX%d with margins dim %dX%d margins %fX%f! \n",
      sensor_isp_info->type[si],
      sensor_isp_info->stream_sizes[si].width,
      sensor_isp_info->stream_sizes[si].height,
      sensor_isp_info->stream_sz_plus_margin[si].width,
      sensor_isp_info->stream_sz_plus_margin[si].height,
      sensor_isp_info->margins[si].widthMargins,
      sensor_isp_info->margins[si].heightMargins);
  }
}

int
cpp_module_util_get_sac_margins(
  cam_frame_margins_t *margins,
  cpp_module_session_params_t *session_params)
{
  assert(margins != NULL);
  assert(session_params != NULL);

  if (session_params->cam_type == CAM_TYPE_AUX) {
    margins->widthMargins  = 0.10f;
    margins->heightMargins = 0.10f;
  } else {
    margins->widthMargins  = 0.00f;
    margins->heightMargins = 0.00f;
  }

  /* Also, cache the margins into Session Parameters */
  session_params->hw_params.crop_info.sac_crop_margins = *margins;

  CPP_DBG("SAC margins:{wm=%d,hm=%d} ",
    margins->widthMargins, margins->heightMargins);

  return 0;
}

void
cpp_module_util_apply_sac_margin(
  sensor_isp_stream_sizes_t *sensor_isp_info,
  uint32_t si,
  const cam_frame_margins_t *sac_crop_margins)
{
  assert(sensor_isp_info  != NULL);
  assert(sac_crop_margins != NULL);

  boolean need_sac_crop_margins =
    (sensor_isp_info->postprocess_mask[si] & CAM_QTI_FEATURE_SAC) &&
    ((sensor_isp_info->type[si] == CAM_STREAM_TYPE_PREVIEW) ||
     (sensor_isp_info->type[si] == CAM_STREAM_TYPE_VIDEO)   ||
     (sensor_isp_info->type[si] == CAM_STREAM_TYPE_CALLBACK));

  CPP_DBG("sensor_isp_info->postprocess_mask[%u]:0x%x "
    "CAM_QTI_FEATURE_SAC:%llu (0x%llx) "
    "(sensor_isp_info->postprocess_mask[%u] & CAM_QTI_FEATURE_SAC):0x%x "
    "(sensor_isp_info->type[%u] == CAM_STREAM_TYPE_PREVIEW):%u "
    "(sensor_isp_info->type[%u] == CAM_STREAM_TYPE_VIDEO):%u "
    "(sensor_isp_info->type[%u] == CAM_STREAM_TYPE_CALLBACK):%u",
    si, sensor_isp_info->postprocess_mask[si],
    CAM_QTI_FEATURE_SAC, CAM_QTI_FEATURE_SAC,
    si, (sensor_isp_info->postprocess_mask[si] & CAM_QTI_FEATURE_SAC),
    si, (sensor_isp_info->type[si] == CAM_STREAM_TYPE_PREVIEW),
    si, (sensor_isp_info->type[si] == CAM_STREAM_TYPE_VIDEO),
    si, (sensor_isp_info->type[si] == CAM_STREAM_TYPE_CALLBACK));

  /*
   * Use the CAM_QTI_FEATURE_SAC feature mask as this sequence
   * is initiated before the actual streams are created/started.
   */
  CPP_DBG("Need SAC crop_margins to Session:%s ",
    (need_sac_crop_margins) ? "Yes" : "No");

  if (need_sac_crop_margins) {
    /*
     * Apply the same crop margins, from the cached margins from the
     * Session Parameters.
     */
    sensor_isp_info->margins[si].widthMargins  +=
      sac_crop_margins->widthMargins;
    sensor_isp_info->margins[si].heightMargins +=
      sac_crop_margins->heightMargins;

    sensor_isp_info->stream_sz_plus_margin[si].width  *=
      (1 + sac_crop_margins->widthMargins);
    sensor_isp_info->stream_sz_plus_margin[si].height *=
      (1 + sac_crop_margins->heightMargins);

    sensor_isp_info->stream_sz_plus_margin[si].width  =
      FOVC_MARGIN_ALIGN(sensor_isp_info->stream_sz_plus_margin[si].width,
        2);
    sensor_isp_info->stream_sz_plus_margin[si].height =
      FOVC_MARGIN_ALIGN(sensor_isp_info->stream_sz_plus_margin[si].height,
        2);
  }
  return;
} /* cpp_module_util_apply_sac_margin() */

inline void
cpp_module_util_cache_sac_margin_to_stream(
  cpp_module_stream_params_t *stream_params,
  const cpp_module_session_params_t *session_params)
{
  assert(stream_params  != NULL);
  assert(session_params != NULL);

  /*
   * Copy the SAC crop margins from the cached margins available in the
   * Session Parameters. These values are used later to crop the images
   * for SAC in the cpp_params_apply_dualcam_shift_offset_and_margins().
   */
  stream_params->hw_params.crop_info.sac_crop_margins =
    session_params->hw_params.crop_info.sac_crop_margins;

  return;
} /* cpp_module_util_cache_sac_margin_to_stream() */


boolean cpp_module_util_is_cpp_proc_need(
   mct_module_t *module __unused,
   cpp_module_stream_params_t *stream_params,
   cpp_module_stream_params_t *linked_stream_params)
{
  mct_port_t *port = (mct_port_t*)stream_params->parent;
  mct_port_t *port_linked = (mct_port_t*)linked_stream_params->parent;

  if (port == port_linked && (stream_params->native_buff > 0) ) {
    return TRUE;
  }
 return FALSE;
}

int cpp_module_util_update_format(mct_stream_info_t *stream_info,
                                   cpp_params_dim_info_t *dim_info)
{
  /* format info */
  if (stream_info->fmt == CAM_FORMAT_YUV_420_NV12 ||
    stream_info->fmt == CAM_FORMAT_YUV_420_NV12_VENUS) {
    dim_info->plane_fmt = CPP_PARAM_PLANE_CBCR;
  } else if (stream_info->fmt == CAM_FORMAT_YUV_420_NV21 ||
    stream_info->fmt == CAM_FORMAT_YUV_420_NV21_VENUS) {
    dim_info->plane_fmt = CPP_PARAM_PLANE_CRCB;
  } else if (stream_info->fmt == CAM_FORMAT_YUV_422_NV16) {
    dim_info->plane_fmt = CPP_PARAM_PLANE_CBCR422;
  } else if (stream_info->fmt == CAM_FORMAT_YUV_422_NV61) {
    dim_info->plane_fmt = CPP_PARAM_PLANE_CRCB422;
  } else if (stream_info->fmt == CAM_FORMAT_YUV_420_YV12) {
    dim_info->plane_fmt = CPP_PARAM_PLANE_CRCB420;
  } else if (stream_info->fmt == CAM_FORMAT_YUV_444_NV24) {
    dim_info->plane_fmt = CPP_PARAM_PLANE_CBCR444;
  } else if (stream_info->fmt == CAM_FORMAT_YUV_444_NV42) {
    dim_info->plane_fmt = CPP_PARAM_PLANE_CRCB444;
  } else if (stream_info->fmt == CAM_FORMAT_Y_ONLY) {
    dim_info->plane_fmt = CPP_PARAM_PLANE_Y;
  } else if (stream_info->fmt == CAM_FORMAT_Y_ONLY_10_BPP) {
    dim_info->plane_fmt = CPP_PARAM_PLANE_Y;
  } else if (stream_info->fmt == CAM_FORMAT_Y_ONLY_12_BPP) {
    dim_info->plane_fmt = CPP_PARAM_PLANE_Y;
  } else if (stream_info->fmt == CAM_FORMAT_YUV_420_NV12_UBWC) {
    CPP_HIGH("CPP_PARAM_PLANE_CBCR_UBWC");
    dim_info->plane_fmt = CPP_PARAM_PLANE_CBCR_UBWC;
  }
  else {
    CPP_ERR("Format not supported %d\n",stream_info->fmt);
    return -1;
  }
  return 0;
}

void cpp_module_util_update_dim(mct_stream_info_t *stream_info,
  cpp_params_dim_info_t *dim_info)
{
  uint32_t k;

  dim_info->width = stream_info->dim.width;
  dim_info->height = stream_info->dim.height;
  dim_info->scanline =
    stream_info->buf_planes.plane_info.mp[0].scanline;
  dim_info->stride =
    stream_info->buf_planes.plane_info.mp[0].stride;
  dim_info->frame_len =
      stream_info->buf_planes.plane_info.frame_len;

  for (k = 0; k < stream_info->buf_planes.plane_info.num_planes; k++) {
    dim_info->plane_info[k].plane_offset_x =
      stream_info->buf_planes.plane_info.mp[k].offset_x;
    dim_info->plane_info[k].plane_offset_y =
      stream_info->buf_planes.plane_info.mp[k].offset_y;
    dim_info->plane_info[k].meta_stride =
      stream_info->buf_planes.plane_info.mp[k].meta_stride;
    dim_info->plane_info[k].meta_scanline =
      stream_info->buf_planes.plane_info.mp[k].meta_scanline;
    dim_info->plane_info[k].meta_len =
      stream_info->buf_planes.plane_info.mp[k].meta_len;
    dim_info->plane_info[k].plane_offsets =
      stream_info->buf_planes.plane_info.mp[k].offset;
    dim_info->plane_info[k].plane_len =
      stream_info->buf_planes.plane_info.mp[k].len;
    CPP_INFO("offset_x %d, offset_y %d, offset %d, meta_len %d,"
      "meta_scanline %d, meta_stride %d, plane_len %d",
      stream_info->buf_planes.plane_info.mp[k].offset_x,
      stream_info->buf_planes.plane_info.mp[k].offset_y,
      stream_info->buf_planes.plane_info.mp[k].offset,
      stream_info->buf_planes.plane_info.mp[k].meta_len,
      stream_info->buf_planes.plane_info.mp[k].meta_scanline,
      stream_info->buf_planes.plane_info.mp[k].meta_stride,
      stream_info->buf_planes.plane_info.mp[k].len);
  }
}

void cpp_module_util_update_feature_mask(cam_pp_feature_config_t *pp_config,
                                           cpp_hardware_params_t *hw_params){

   char cpp_prop[PROPERTY_VALUE_MAX];
   memset(cpp_prop, 0, sizeof(cpp_prop));

   if (pp_config->feature_mask & CAM_QCOM_FEATURE_SCALE) {
     hw_params->scale_enable = 1;
   } else {
     hw_params->scale_enable = 0;
   }
   if (pp_config->feature_mask & CAM_QCOM_FEATURE_CROP) {
     hw_params->crop_enable = 1;
   } else {
     hw_params->crop_enable = 0;
   }
   if (pp_config->feature_mask & CAM_QTI_FEATURE_FIXED_FOVC) {
     hw_params->fovc_enable = 1;
   } else {
     hw_params->fovc_enable = 0;
   }
   if (pp_config->feature_mask & CAM_QCOM_FEATURE_CDS) {
     hw_params->downsample_mask = 1;
   } else {
     hw_params->downsample_mask = 0;
   }
   if (pp_config->feature_mask & CAM_QTI_FEATURE_SAC) {
     hw_params->sac_enable = 1;
   } else {
     hw_params->sac_enable = 0;
   }
   property_get("persist.vendor.camera.cpp.luma.dsdn", cpp_prop, "0");
   hw_params->luma_dsdn = atoi(cpp_prop);
}

uint32_t cpp_module_util_is_slave_proc_required(
  cpp_module_session_params_t *session_params,
  cpp_module_stream_params_t *stream_params) {

  assert(stream_params != NULL);
  assert(session_params != NULL);
  return (session_params->cam_role == CAM_ROLE_BAYER) ||
         (session_params->cam_role == CAM_ROLE_MONO) ||
         (stream_params->stream_info->pp_config.feature_mask & CAM_QTI_FEATURE_RTB) ||
         (session_params->force_slave_process);
}
