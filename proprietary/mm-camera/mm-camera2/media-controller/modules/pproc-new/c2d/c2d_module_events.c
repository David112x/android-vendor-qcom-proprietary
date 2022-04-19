/*============================================================================

  Copyright (c) 2013-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#include "c2d_module_events.h"
#include "mct_profiler.h"

/** c2d_module_calculate_stream_crop_info:
 *
 *  @stream_params: stream params
 *  @stream_crop: stream crop
 *
 *  Calculate the effective stream crop.
 *
 *  Return: None
 **/
void c2d_module_calculate_stream_crop_info(
  c2d_module_stream_params_t *stream_params,
  mct_bus_msg_stream_crop_t *stream_crop)
{
  mct_bus_msg_stream_crop_t incoming_stream_crop = *stream_crop;
  struct _c2d_params_is_crop_window_t eis_config;
  cam_dimension_t input_dim;
  cam_dimension_t output_dim;
  boolean eis_valid = FALSE;

  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  input_dim.width = stream_params->hw_params.input_info.width;
  input_dim.height = stream_params->hw_params.input_info.height;
  output_dim.width = stream_params->hw_params.output_info.width;
  output_dim.height = stream_params->hw_params.output_info.height;
  eis_valid = stream_params->dis_hold.is_valid;
  if (eis_valid) {
    memcpy(&eis_config, &stream_params->hw_params.crop_info.is_crop,
      sizeof(struct _c2d_params_is_crop_window_t));
  }
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

  // CROP RECT
  stream_crop->x =
    (stream_crop->x * output_dim.width) / input_dim.width;
  stream_crop->y =
    (stream_crop->y * output_dim.height) / input_dim.height;
  stream_crop->crop_out_x =
    (stream_crop->crop_out_x * output_dim.width) / input_dim.width;
  stream_crop->crop_out_y =
    (stream_crop->crop_out_y * output_dim.height) / input_dim.height;

  // As C2D uses extra margin, For FD
  // to calculate mapping properly w.r.t
  // change in FOV due to margin.
  stream_crop->width_margin =  CALC_MARGIN(input_dim.width, output_dim.width);
  stream_crop->height_margin =  CALC_MARGIN(input_dim.height, output_dim.height);
  C2D_LOW("InputDim:(%d, %d) OutputDim=(%d, %d) Margin =(%f, %f)",
    input_dim.width, input_dim.height,
    output_dim.width, output_dim.height,
    stream_crop->width_margin, stream_crop->height_margin);

  // CAMIF mapping
  if (!eis_valid) {
    // Assuming EIS on preview is identity matrix i.e just the center crop
    stream_crop->width_map =
      (stream_crop->width_map * output_dim.width) / input_dim.width;
    stream_crop->height_map =
      (stream_crop->height_map * output_dim.height) / input_dim.height;
    stream_crop->x_map = stream_crop->x_map +
      ((incoming_stream_crop.width_map - stream_crop->width_map) / 2);
    stream_crop->y_map = stream_crop->y_map +
      ((incoming_stream_crop.height_map - stream_crop->height_map) / 2);
  } else {
    c2d_fpoint in_top_left, in_top_right, in_bottom_left, in_bottom_right;
    c2d_fpoint out_top_left, out_top_right, out_bottom_left, out_bottom_right;

    // Find the Rect that will be used as Source Rect
    // in C2D for applying transformations
    // topleft point
    in_top_left.x = (input_dim.width - output_dim.width) / 2;
    in_top_left.y = (input_dim.height - output_dim.height) / 2;
    // bottom right
    in_bottom_right.x = in_top_left.x + output_dim.width;
    in_bottom_right.y = in_top_left.y + output_dim.height;
    // top right
    in_top_right.x = in_bottom_right.x;
    in_top_right.y = in_top_left.y;
    // bottom left
    in_bottom_left.x = in_top_left.x;
    in_bottom_left.y = in_bottom_right.y;

    C2D_DBG("InputDim:(%d, %d) OutputDim=(%d, %d)",
      input_dim.width, input_dim.height,
      output_dim.width, output_dim.height);

    C2D_DBG("Center crop : "
      "top_left=(%.2f, %.2f) top_right=(%.2f, %.2f), "
      "bottom_left=(%.2f, %.2f), bottom_right=(%.2f, %.2f)",
      in_top_left.x, in_top_left.y,
      in_top_right.x, in_top_right.y,
      in_bottom_left.x, in_bottom_left.y,
      in_bottom_right.x, in_bottom_right.y);

    if (eis_config.transform_type & ((1 << 2))) {
      // transformation matrix has to be applied with origin at center.
      // translate the points
      MOVE_TO_ORIGIN_AT_CENTER(&in_top_left, &input_dim);
      MOVE_TO_ORIGIN_AT_CENTER(&in_top_right, &input_dim);
      MOVE_TO_ORIGIN_AT_CENTER(&in_bottom_left, &input_dim);
      MOVE_TO_ORIGIN_AT_CENTER(&in_bottom_right, &input_dim);
    }

    // Apply transformation matrix and calculate the region that will be
    // fetched from input buffer.
    const float epsilon = 0.000001f;
    MULTIPLY_MATRIX(&in_top_left, eis_config.transform_matrix,
      epsilon, &out_top_left);
    MULTIPLY_MATRIX(&in_top_right, eis_config.transform_matrix,
      epsilon, &out_top_right);
    MULTIPLY_MATRIX(&in_bottom_right, eis_config.transform_matrix,
      epsilon, &out_bottom_right);
    MULTIPLY_MATRIX(&in_bottom_left, eis_config.transform_matrix,
      epsilon, &out_bottom_left);

    if (eis_config.transform_type & ((1 << 2))) {
      // transformation matrix was applied with origin at center.
      // translate the points back to original

      MOVE_TO_ORIGIN_AT_TOPLEFT(&out_top_left, &input_dim);
      MOVE_TO_ORIGIN_AT_TOPLEFT(&out_top_right, &input_dim);
      MOVE_TO_ORIGIN_AT_TOPLEFT(&out_bottom_right, &input_dim);
      MOVE_TO_ORIGIN_AT_TOPLEFT(&out_bottom_left, &input_dim);
    }

    C2D_DBG("Actual EIS CROP: "
      "top_left=(%.2f, %.2f) top_right=(%.2f, %.2f), "
      "bottom_left=(%.2f, %.2f), bottom_right=(%.2f, %.2f)",
      out_top_left.x, out_top_left.y,
      out_top_right.x, out_top_right.y,
      out_bottom_left.x, out_bottom_left.y,
      out_bottom_right.x, out_bottom_right.y);

    // Note that out_xx points after applying transformations may not
    // result in a Rect. Calculate the effective rect region :
    // Outer Bounding Box
    float outer_left = (out_top_left.x < out_bottom_left.x) ?
      out_top_left.x : out_bottom_left.x;
    float outer_right = (out_top_right.x > out_bottom_right.x) ?
      out_top_right.x : out_bottom_right.x;
    float outer_top = (out_top_left.y < out_top_right.y) ?
      out_top_left.y : out_top_right.y;
    float outer_bottom = (out_bottom_left.y > out_bottom_right.y) ?
      out_bottom_left.y : out_bottom_right.y;

    // Inner Bounding Box
    float inner_left = (out_top_left.x < out_bottom_left.x) ?
      out_bottom_left.x : out_top_left.x;
    float inner_right = (out_top_right.x > out_bottom_right.x) ?
      out_bottom_right.x : out_top_right.x;
    float inner_top = (out_top_left.y < out_top_right.y) ?
      out_top_right.y : out_top_left.y;
    float inner_bottom = (out_bottom_left.y > out_bottom_right.y) ?
      out_bottom_right.y : out_bottom_left.y;

    C2D_LOW("OUTER BOUNDING BOX "
      "topleft=(%.2f,%.2f), bottomright=(%.2f,%.2f)",
      outer_left, outer_top, outer_right, outer_bottom);
    C2D_LOW("INNER BOUNDING BOX "
      "topleft=(%.2f,%.2f), bottomright=(%.2f,%.2f)",
      inner_left, inner_top, inner_right, inner_bottom);

    // taking outer bounding box as the final rect
    bool use_inner_bounding_box = FALSE;
    cam_rect_t crop_rect;
    if (use_inner_bounding_box == TRUE) {
      crop_rect.left = (int)inner_left;
      crop_rect.top = (int)inner_top;
      crop_rect.width = (int)(inner_right - inner_left);
      crop_rect.height = (int)(inner_bottom - inner_top);
    } else {
      crop_rect.left = (int)outer_left;
      crop_rect.top = (int)outer_top;
      crop_rect.width = (int)(outer_right - outer_left);
      crop_rect.height = (int)(outer_bottom - outer_top);
    }

    C2D_DBG("Effective EIS CROP : (%d, %d) w=%d, h=%d",
      crop_rect.left, crop_rect.top, crop_rect.width, crop_rect.height);

    // Update the stream_crop information that will be sent downstream
    stream_crop->width_map =
      (stream_crop->width_map * crop_rect.width) / input_dim.width;
    stream_crop->height_map =
      (stream_crop->height_map * crop_rect.height) / input_dim.height;
    stream_crop->x_map = stream_crop->x_map +
      (crop_rect.left * incoming_stream_crop.width_map / input_dim.width);
    stream_crop->y_map = stream_crop->y_map +
      (crop_rect.top * incoming_stream_crop.height_map / input_dim.height);
  }
}


/** c2d_module_get_frame_valid:
 *
 *  @module: mct module handle
 *  @identity: event identity
 *  @frame_id: frame id for which validity needs to be checked
 *  @stream_type: stream type for which validity needs to be
 *              checked
 *  @mctl_frame_id: frame id passed by mctl
 *
 *  This function passes upstream event to ISP to find out
 *  whether this frame needs to be processed for this stream
 *
 *  Return: is_frame_valid on success and 0 on failure
 **/
uint32_t c2d_module_get_frame_valid(mct_module_t* module,
  uint32_t identity, uint32_t frame_id, unsigned int stream_id,
  cam_stream_type_t stream_type)
{
  uint32_t                    stream_valid = 0;
  uint32_t                    i = 0;
  uint32_t                    q_idx = frame_id % PER_FRAME_QUEUE_SIZE;
  mct_event_t                 new_event;
  mct_event_frame_request_t   frame_request;
  c2d_module_stream_params_t  *stream_params;
  c2d_module_session_params_t *session_params;

  c2d_module_ctrl_t* ctrl = (c2d_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if (!ctrl) {
    C2D_ERR("invalid control, failed\n");
    return stream_valid;
  }

  c2d_module_get_params_for_identity(ctrl, identity, &session_params,
    &stream_params);
  if (!session_params) {
    C2D_ERR("invalid session params, failed\n");
     return stream_valid;
  }

  if (session_params->hal_version == CAM_HAL_V3) {
    if (stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
      /* offline stream not request based */
      stream_valid = 1;
    } else {
      PTHREAD_MUTEX_LOCK(&(session_params->c2d_per_frame_mutex));
      /* Check if there is a valid request to set stream valid to true */
      for (i = 0; i < session_params->valid_stream_ids[q_idx].num_streams; i++) {
        if (session_params->valid_stream_ids[q_idx].stream_request[i].streamID == stream_id) {
          session_params->valid_stream_ids[q_idx].stream_request[i].streamID = 0;
          stream_valid = 1;
          break;
        }
      }
      PTHREAD_MUTEX_UNLOCK(&(session_params->c2d_per_frame_mutex));
    }
  } else {
    /* HAL 1 set stream valid to true, process every frame */
    stream_valid = 1;
  }
  return stream_valid;
}

/** c2d_module_create_c2d_event:
 *
 * Description:
 *  Create c2d event and fill ack key, and processed divert
 *  information.
 *
 **/
c2d_module_event_t *c2d_module_create_c2d_event(c2d_module_ack_key_t ack_key,
  c2d_hardware_params_t *hw_params, isp_buf_divert_t *isp_buf,
  uint32_t identity, uint32_t div_identity, mct_stream_info_t **stream_info)
{
  mct_stream_info_t *output_stream_info = NULL, *input_stream_info = NULL;
  c2d_module_event_t *c2d_event = (c2d_module_event_t*)
    CAM_MALLOC(sizeof(c2d_module_event_t));
  if(!c2d_event) {
    C2D_ERR("CAM_MALLOC() failed\n");
    return NULL;
  }
  //mct_stream_info_t  *streaminfo = (mct_stream_info_t *)event->u.ctrl_event.control_event_data;
  memset(c2d_event, 0x00, sizeof(c2d_module_event_t));
  c2d_event->ack_key = ack_key;
  /* by default all events are created valid */
  c2d_event->invalid = FALSE;

  if (hw_params) {
    /* this is hw processing event */
    c2d_event->hw_process_flag = TRUE;
    c2d_event->type = C2D_MODULE_EVENT_PROCESS_BUF;
    c2d_event->u.process_buf_data.proc_identity = identity;

    /* copy isp buf and other data from the mct event */
    memcpy(&(c2d_event->u.process_buf_data.isp_buf_divert),
      (isp_buf_divert_t *)(isp_buf), sizeof(isp_buf_divert_t));
    input_stream_info = stream_info[0];
    output_stream_info = stream_info[0];
    if (stream_info[0]) {
      if (stream_info[0]->identity == isp_buf->identity)
        input_stream_info = stream_info[0];
      if (stream_info[0]->identity == identity)
        output_stream_info = stream_info[0];
    }
    if (stream_info[1]) {
      if (stream_info[1]->identity == isp_buf->identity)
        input_stream_info = stream_info[1];
      if (stream_info[1]->identity == identity)
        output_stream_info = stream_info[1];
    }
    /* Store input_stream_info pointer in c2d event */
    c2d_event->u.process_buf_data.input_stream_info = input_stream_info;
    /* Store stream_info pointer in c2d event */
    c2d_event->u.process_buf_data.stream_info = output_stream_info;
    /* processed divert related info */
    c2d_event->u.process_buf_data.proc_div_identity =
      div_identity;
    c2d_event->u.process_buf_data.proc_div_required = FALSE;
    if (div_identity != PPROC_INVALID_IDENTITY) {
      c2d_event->u.process_buf_data.proc_div_required = TRUE;
    }
    /* copy the stream hw params in event */
    memcpy(&(c2d_event->u.process_buf_data.hw_params), hw_params,
      sizeof(c2d_hardware_params_t));
  } else {
    /* this is unprocessed divert event */
    if (div_identity == PPROC_INVALID_IDENTITY) {
      C2D_ERR("failed invalid unprocess div identity\n");
      free(c2d_event);
      return NULL;
    }
    c2d_event->hw_process_flag = FALSE;
    c2d_event->type = C2D_MODULE_EVENT_DIVERT_BUF;

    /* copy isp buf and other data from the mct event */
    memcpy(&(c2d_event->u.divert_buf_data.isp_buf_divert),
      (isp_buf_divert_t*)(isp_buf), sizeof(isp_buf_divert_t));
    c2d_event->u.divert_buf_data.div_identity = div_identity;
    c2d_event->u.divert_buf_data.isp_buf_divert.identity = identity;
    c2d_event->u.divert_buf_data.isp_buf_divert.pass_through = 1;
  }
  return c2d_event;
}

/** c2d_module_send_buf_divert_event
 *
 *      @module: c2d module structure
 *      @indentity: current stream identity
 *      @isp_buf: buf_divert event structure.
 *
 *      This function creates event and sends events to c2d_thread. The events
 *      are created according to the divert_config structure.
 *
 *      Returns 0 at success
 *
 **/
int32_t c2d_module_send_buf_divert_event(mct_module_t* module,
  uint32_t identity, isp_buf_divert_t *isp_buf)
{
  uint32_t identity_list[2];
  uint32_t identity_list_size = 0;
  if (!module || !isp_buf) {
    C2D_ERR("failed, module=%p, isp_buf=%p\n", module, isp_buf);
    return -EINVAL;
  }
  c2d_module_ctrl_t* ctrl = (c2d_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if (!ctrl) {
    C2D_ERR("failed\n");
    return -EFAULT;
  }
  struct timeval tv1, tv2;
  gettimeofday(&tv1, NULL);
  uint32_t frame_id = isp_buf->buffer.sequence;
  int32_t ret = 0;

  /* get stream parameters based on the event identity */
  c2d_module_stream_params_t *stream_params = NULL;
  c2d_module_stream_params_t *linked_stream_params = NULL;
  c2d_module_session_params_t *session_params = NULL;
  c2d_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if (!stream_params) {
    C2D_ERR("failed\n");
    return -EFAULT;
  }
  linked_stream_params = stream_params->linked_stream_params;

  c2d_module_stream_params_t *linked_stream_list[2];
  uint32_t unproc_div_identity=0x00;
  boolean unproc_div_required=FALSE;
  uint32_t stream_frame_valid = 1, linked_stream_frame_valid = 1;

  identity_list[0] = PPROC_INVALID_IDENTITY;
  identity_list[1] = PPROC_INVALID_IDENTITY;
  linked_stream_list[0] = NULL;
  linked_stream_list[1] = NULL;
  /* decide processing requirements based on the stream params */
  if (linked_stream_params) { /* linked stream case */
    PTHREAD_MUTEX_LOCK(&(linked_stream_params->mutex));

    /* if both streams in the pair are off, drop frame */
    if ((stream_params->is_stream_on == FALSE || !stream_frame_valid)&&
      (linked_stream_params->is_stream_on == FALSE ||
      !linked_stream_frame_valid)) {
      C2D_HIGH("stream is off, drop frame and piggy-back ACK\n");
      isp_buf->ack_flag = TRUE;
      isp_buf->is_buf_dirty = 1;
      PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
      return 0;
    }
    /* if current stream is on and linked stream is off */
    else if (stream_params->is_stream_on == TRUE && stream_frame_valid &&
      (linked_stream_params->is_stream_on == FALSE ||
      !linked_stream_frame_valid)) {
      /* only one pass required on current identity */
      linked_stream_list[0] = stream_params;
      identity_list[0] = stream_params->identity;
      identity_list_size = 1;
    }
    /* if current stream is off and linked stream is on */
    else if((stream_params->is_stream_on == FALSE || !stream_frame_valid) &&
      linked_stream_params->is_stream_on == TRUE &&
      linked_stream_frame_valid) {
      /* only one pass required on linked identity */
      linked_stream_list[0] = linked_stream_params;
      identity_list[0] = linked_stream_params->identity;
      identity_list_size = 1;
    }
    /* if both streams are on */
    else if(stream_params->is_stream_on == TRUE &&
      linked_stream_params->is_stream_on == TRUE &&
      stream_frame_valid && linked_stream_frame_valid) {
      /* first pass on current identity and second pass on linked identity */
      linked_stream_list[0] = stream_params;
      identity_list[0] = stream_params->identity;
      identity_list_size = 1;
      if ((stream_params->stream_info->is_type != IS_TYPE_EIS_2_0) &&
        (!stream_params->interleaved)) {
        linked_stream_list[1] = linked_stream_params;
        identity_list[1] = linked_stream_params->identity;
        identity_list_size++;
      }
    }
  } else { /* non-linked stream case */
    /* if stream is off, drop frame */
    if (stream_params->is_stream_on == FALSE || !stream_frame_valid) {
      C2D_DBG("stream is off, drop frame and piggy-back ACK\n");
      isp_buf->ack_flag = TRUE;
      isp_buf->is_buf_dirty = 1;
      return 0;
    }
    linked_stream_list[0] = stream_params;
    identity_list[0] = stream_params->identity;
    identity_list_size = 1;
  }

  /* create a key for ack with original event identity, this key will be
     put in all corresponding events in queue and used to release the ack */
  c2d_module_ack_key_t key;
  key.identity = identity;
  key.buf_identity = isp_buf->identity;
  key.buf_idx = isp_buf->buffer.index;
  key.channel_id = isp_buf->channel_id;
  C2D_LOW("Saved identitys to key event 0x%x , buf 0x%x",
    key.identity, key.buf_identity);

  /* Decide the events to be queued to process this buffer */
  uint32_t event_idx = 0, num_events = 0;
  /* based on configuration, at max 3 events are queued for one buffer */
  c2d_module_event_t* c2d_event[3];

  /* Step 1. if unprocessed divert is needed, add an event for that */
  if (unproc_div_required == TRUE) {
    c2d_event[event_idx] = c2d_module_create_c2d_event(key, NULL, isp_buf,
      identity, unproc_div_identity, NULL);
    if (!c2d_event[event_idx]) {
      C2D_ERR("CAM_MALLOC() failed\n");
      if (linked_stream_params)
        PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
      return -ENOMEM;
    }
    event_idx++;
  }

  uint32_t i = 0, j = 0;
  uint32_t num_passes = 0;
  boolean skip_frame = FALSE;
  mct_stream_info_t *stream_info[2];
  stream_info[0] = NULL;
  if (stream_params) {
    stream_info[0] = stream_params->stream_info;
  }
  stream_info[1] = NULL;
  if (linked_stream_params) {
    stream_info[1] = linked_stream_params->stream_info;
  }
  /* Step 2. Based on the number of process identities set in divert config,
     generate c2d events accordingly */
  for (j = 0; j < identity_list_size; j++) {
    if (!linked_stream_list[j]) {
      continue;
    }
    if (!linked_stream_list[j]->out_dim_initialized)
     continue;
    skip_frame = FALSE;
    /* decide if skip is required for HFR */
    if (linked_stream_list[j]->hfr_skip_info.skip_required) {
      if ((c2d_decide_hfr_skip(frame_id -
        linked_stream_list[j]->hfr_skip_info.frame_offset,
        linked_stream_list[j]->hfr_skip_info.skip_count)) == TRUE) {
        /* Skip this frame */
        C2D_DBG("skipping frame_id=%d for identity=0x%x",
          frame_id, linked_stream_list[j]->identity);
        C2D_DBG("skip_count=%d, offset=%d",
          linked_stream_list[j]->hfr_skip_info.skip_count,
          linked_stream_list[j]->hfr_skip_info.frame_offset);
        skip_frame = TRUE;
      }
    }

    if (skip_frame == FALSE) {
      c2d_event[event_idx] = c2d_module_create_c2d_event(key,
        &(linked_stream_list[j]->hw_params), isp_buf,
        linked_stream_list[j]->identity,
        0x00000000,
        &stream_info[0]);
      if(!c2d_event[event_idx]) {
        C2D_DBG("CAM_MALLOC() failed\n");
        if(linked_stream_params)
          PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
        return -ENOMEM;
      }
      event_idx++;
    }
    num_passes++;
  }

  num_events = event_idx;
  /* if no events needs to be queued, do a piggy-back ACK */
  if (num_events == 0) {
    isp_buf->ack_flag = TRUE;
    isp_buf->is_buf_dirty = 1;
    if(linked_stream_params)
      PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
    C2D_ERR("buffer event received with no divert config\n");
    return 0;
  }
  /* before queuing any events, first put corresponding ACK in the ack_list */
  c2d_module_put_new_ack_in_list(ctrl, key, 1, num_events, isp_buf);

  /* now enqueue all events one by one in priority queue */
  int32_t rc;
  for (i=0; i<num_events; i++) {
    rc = c2d_module_enq_event(module, c2d_event[i], stream_params->priority);
    if(rc < 0) {
      C2D_ERR("failed, i=%d\n", i);
      if(linked_stream_params)
        PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
      return -EFAULT;
    }
  }
  if (linked_stream_params)
    PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
  /* notify the thread about this new events */

  if (num_events) {
    c2d_thread_msg_t msg;
    memset(&msg, 0, sizeof(c2d_thread_msg_t));
    msg.type = C2D_THREAD_MSG_NEW_EVENT_IN_Q;
    c2d_module_post_msg_to_thread(module, msg);
  }

  gettimeofday(&tv2, NULL);
  C2D_LOW("downstream event time = %6ld us, ",
    (tv2.tv_sec - tv1.tv_sec)*1000000L +
    (tv2.tv_usec - tv1.tv_usec));
  return 0;
}

int32_t c2d_module_send_isp_output_dimension_event(mct_module_t* module,
  c2d_module_stream_params_t *stream_params)
{
  int32_t rc;
  mct_stream_info_t           c2d_out_stream_info;
  mct_event_t                 c2d_dim_event;
  uint32_t i = 0;

  memset(&c2d_out_stream_info, 0x0, sizeof(mct_stream_info_t));
  memset(&c2d_dim_event, 0x0, sizeof(mct_event_t));
  c2d_out_stream_info.dim.width =
    stream_params->hw_params.output_info.width;
  c2d_out_stream_info.dim.height =
    stream_params->hw_params.output_info.height;
  c2d_out_stream_info.buf_planes.plane_info.num_planes =
    stream_params->hw_params.output_info.plane_info.num_planes;
  c2d_out_stream_info.buf_planes.plane_info.frame_len =
    stream_params->hw_params.output_info.plane_info.frame_len;
  c2d_out_stream_info.fmt = stream_params->hw_params.input_info.cam_fmt;
  c2d_out_stream_info.stream_type = stream_params->stream_info->stream_type;
  if ( stream_params->interleaved) {
    c2d_out_stream_info.fmt = stream_params->hw_params.output_info.cam_fmt;
  }
  C2D_INFO("identity=0x%x, dim=%dx%d fmt= %d, frame_len %d\n",
    stream_params->identity,
    c2d_out_stream_info.dim.width, c2d_out_stream_info.dim.height,
    stream_params->stream_info->fmt,
    stream_params->hw_params.output_info.plane_info.frame_len);

  for (;i < c2d_out_stream_info.buf_planes.plane_info.num_planes; i++) {
    c2d_out_stream_info.buf_planes.plane_info.mp[i].stride =
      stream_params->hw_params.output_info.plane_info.mp[i].stride;
    c2d_out_stream_info.buf_planes.plane_info.mp[i].scanline =
      stream_params->hw_params.output_info.plane_info.mp[i].scanline;
    c2d_out_stream_info.buf_planes.plane_info.mp[i].offset =
      stream_params->hw_params.output_info.plane_info.mp[i].offset;
    c2d_out_stream_info.buf_planes.plane_info.mp[i].offset_x =
      stream_params->hw_params.output_info.plane_info.mp[i].offset_x;
    c2d_out_stream_info.buf_planes.plane_info.mp[i].offset_y =
      stream_params->hw_params.output_info.plane_info.mp[i].offset_y;
    c2d_out_stream_info.buf_planes.plane_info.mp[i].len =
      stream_params->hw_params.output_info.plane_info.mp[i].len;
    C2D_INFO("identity 0x%x, plane %d, stride %d, scanline %d, offset %d, offset_x %d,"
     "offset_y %d, len %d", stream_params->identity, i,
     c2d_out_stream_info.buf_planes.plane_info.mp[i].stride,
     c2d_out_stream_info.buf_planes.plane_info.mp[i].scanline,
     c2d_out_stream_info.buf_planes.plane_info.mp[i].offset,
     c2d_out_stream_info.buf_planes.plane_info.mp[i].offset_x,
     c2d_out_stream_info.buf_planes.plane_info.mp[i].offset_y,
     c2d_out_stream_info.buf_planes.plane_info.mp[i].len);
  }

  c2d_dim_event.u.module_event.type = MCT_EVENT_MODULE_ISP_OUTPUT_DIM;
  c2d_dim_event.u.module_event.module_event_data =
    (void *)&c2d_out_stream_info;
  c2d_dim_event.type = MCT_EVENT_MODULE_EVENT;
  c2d_dim_event.identity = stream_params->identity;
  c2d_dim_event.direction = MCT_EVENT_DOWNSTREAM;

  rc = c2d_module_send_event_downstream(module, &c2d_dim_event);
  if (rc < 0) {
    C2D_ERR("failed, module_event_type=%d, identity=0x%x",
      c2d_dim_event.u.module_event.type,
      c2d_dim_event.identity);
    return -EFAULT;
  }

  return 0;

}


int32_t c2d_module_update_stream_dimensions(
  c2d_module_ctrl_t* ctrl,
  c2d_module_stream_params_t *stream_params, isp_dis_config_info_t *dis_config)

{
  uint32_t i = 0, height = 0;
  if(!stream_params || !dis_config) {
    C2D_ERR("failed stream_params %p, dis_config %p",
       stream_params, dis_config);
    return -EFAULT;
  }

  C2D_LOW("B id %x stream op %dX%d plane[0] %dX%d len %d"
    "plane[1] %dX%d len %d num_planes %d\n",
    stream_params->identity,
    stream_params->hw_params.output_info.width,
    stream_params->hw_params.output_info.height,
    stream_params->hw_params.output_info.plane_info.mp[0].stride,
    stream_params->hw_params.output_info.plane_info.mp[0].scanline,
    stream_params->hw_params.output_info.plane_info.mp[0].len,
    stream_params->hw_params.output_info.plane_info.mp[1].stride,
    stream_params->hw_params.output_info.plane_info.mp[1].scanline,
    stream_params->hw_params.output_info.plane_info.mp[1].len,
    stream_params->hw_params.output_info.plane_info.num_planes);

  c2d_module_update_output_dimension(ctrl, &stream_params->hw_params,
   stream_params, dis_config->width, dis_config->height, true);

  C2D_LOW(",A id %x stream op %dX%d plane[0] %dX%d len %d"
    "plane[1] %dX%d len %d frame_len %d\n",
    stream_params->identity,
    stream_params->hw_params.output_info.width,
    stream_params->hw_params.output_info.height,
    stream_params->hw_params.output_info.plane_info.mp[0].stride,
    stream_params->hw_params.output_info.plane_info.mp[0].scanline,
    stream_params->hw_params.output_info.plane_info.mp[0].len,
    stream_params->hw_params.output_info.plane_info.mp[1].stride,
    stream_params->hw_params.output_info.plane_info.mp[1].scanline,
    stream_params->hw_params.output_info.plane_info.mp[1].len,
    stream_params->hw_params.output_info.plane_info.frame_len);
  return 0;
}

/** c2d_module_add_frame_to_holder:
 *
 *  @p_session: pointer to c2d session parameters
 *  @identity: event identity
 *  @p_buf_div: pointer to buf divert structure
 *
 *  This function adds the frame to the session's frame holder
 *  queue
 *
 *  Return: 0 success, -ve values failure
 **/
int32_t c2d_module_add_frame_to_holder(c2d_module_stream_params_t *p_stream,
  uint32_t identity,
  isp_buf_divert_t *p_buf_div)
{
  int32_t i;
  c2d_module_frame_hold_t *p_holder;
  int32_t rear = p_stream->frame_hold_r_idx;
  if (p_stream->frame_hold_cnt >= C2D_NUM_FRAME_HOLD) {
    C2D_ERR("Cannot hold frame %d",
      p_stream->frame_hold_cnt);
    return -ENOMEM;
  }

  for (i = rear; i < (C2D_NUM_FRAME_HOLD + rear); i++) {
    p_holder = &p_stream->frame_hold[i % C2D_NUM_FRAME_HOLD];
    if (!p_holder->is_frame_hold) {
      p_holder->identity = identity;
      p_holder->isp_buf = *p_buf_div;
      p_holder->is_frame_hold = TRUE;
      break;
    }
  }

  if ((C2D_NUM_FRAME_HOLD + rear) == i) {
    /*cannot add the frame*/
    C2D_ERR("Cannot hold frame - incorrect count %d for stream %d",
      p_stream->frame_hold_cnt, p_stream->stream_type);
    return -EINVAL;
  }

  C2D_DBG("HOLD frame_idx %d, for stream %d ",
    p_buf_div->buffer.sequence, p_stream->stream_type);
  p_stream->frame_hold_r_idx = (p_stream->frame_hold_r_idx + 1) %
    C2D_NUM_FRAME_HOLD;
  p_stream->frame_hold_cnt++;
  return 0;
}

/** c2d_module_get_oldest_frame_from_holder:
 *
 *  @p_session: pointer to c2d session parameters
 *
 *  This function retrieves the oldest frame from the session's
 *  frame holder queue
 *
 *  Return: valid pointer success, NULL for failure
 **/
c2d_module_frame_hold_t *c2d_module_get_oldest_frame_from_holder(
  c2d_module_stream_params_t *p_stream)
{
  c2d_module_frame_hold_t *p_holder;
  if (p_stream->frame_hold_cnt < C2D_NUM_FRAME_HOLD) {
    /* need not release the frame */
    return NULL;
  }
  /* pick the oldest frame */
  p_holder = &p_stream->frame_hold[p_stream->frame_hold_f_idx];

  C2D_DBG("Get frame_idx %d, for stream %d",
    p_holder->isp_buf.buffer.sequence,  p_stream->stream_type);
  return p_holder;
}

/** c2d_module_release_holder:
 *
 *  @p_session: pointer to c2d session parameters
 *  @p_holder: pointer to the frame holder
 *
 *  This function releases oldest frame from the holder
 *
 *  Return: none
 **/
void c2d_module_release_holder(
  c2d_module_stream_params_t *p_stream,
  c2d_module_frame_hold_t *p_holder)
{
  p_holder->is_frame_hold = FALSE;
  p_stream->frame_hold_cnt--;
  p_stream->frame_hold_f_idx = (p_stream->frame_hold_f_idx + 1) %
    C2D_NUM_FRAME_HOLD;
  C2D_DBG("Release frame_idx %d for stream %d",
    p_holder->isp_buf.buffer.sequence, p_stream->stream_type);
}

/* c2d_module_handle_buf_divert_event:
 *
 * Description:
 *  Send the MCT_HANDLE_MODULE_BUF_DIVERT event. First put corresponding
 *  acknowledgement in a list which will be sent later. Depending on the
 *  stream's parameters, divert and processing events are added in
 *  c2d's priority queue. c2d_thread will pick up these events one by one in
 *  order and when all events corresponding to the ACK are processed,
 *  the ACK will be removed from list and will be sent upstream.
 *
 **/
int32_t c2d_module_handle_buf_divert_event(mct_module_t* module,
  mct_event_t* event)
{
  c2d_module_ctrl_t           *ctrl = NULL;
  isp_buf_divert_t            *isp_buf = NULL;
  uint32_t                     frame_id = 0;
  c2d_module_stream_params_t  *stream_params = NULL;
  c2d_module_session_params_t *session_params = NULL;
  c2d_module_stream_params_t  *linked_stream_params = NULL;
  c2d_module_frame_hold_t     *frame_hold = NULL;
  c2d_module_dis_hold_t       *dis_hold = NULL;
  int32_t                     rc = -1;
  mct_port_t                  *stream_port = NULL;
  uint32_t stream_frame_valid = 1, linked_stream_frame_valid = 1;

  if (!module || !event) {
    C2D_ERR("failed, module=%p, event=%p\n", module, event);
    return -EINVAL;
  }

  ctrl = (c2d_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  if (!ctrl) {
    C2D_ERR("failed ctrl %p\n", ctrl);
    return -EFAULT;
  }

  isp_buf = (isp_buf_divert_t *)(event->u.module_event.module_event_data);
  if (!isp_buf) {
    C2D_ERR("isp_buf %p\n", isp_buf);
    return -EFAULT;
  }

  if (isp_buf->pass_through == 1) {
    int32_t ret;
    /* This buffer divert event simply needs a bypass through. So directly
       send to downstream module. This event is not queued and ref counted.
       The rule is that downstream module is expected to do a piggyback ack.
       This type of unprocess divert is not very clean though. */
    ret = c2d_module_send_event_downstream(ctrl->p_module, event);
    if (ret < 0) {
      C2D_ERR("failed");
    }
    return ret;
  }
  frame_id = isp_buf->buffer.sequence;

  C2D_LOW("buffer divert on frame id %d, identity %x", frame_id,
    event->identity);
  c2d_module_get_params_for_identity(ctrl, event->identity, &session_params,
     &stream_params);
  if (!session_params || !stream_params) {
    C2D_ERR("failed params %p %p\n", session_params, stream_params);
    return -EFAULT;
  }

  stream_port = (mct_port_t *) stream_params->parent;
  if (stream_port == NULL) {
    C2D_ERR("invalid stream port, failed\n");
    return -EFAULT;
  }

  MCT_OBJECT_LOCK(stream_port);
  linked_stream_params = stream_params->linked_stream_params;

  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));

  stream_frame_valid = c2d_module_get_frame_valid(module, event->identity,
    isp_buf->buffer.sequence, C2D_GET_STREAM_ID(event->identity),
    stream_params->stream_type);

  if (linked_stream_params) {
    PTHREAD_MUTEX_LOCK(&(linked_stream_params->mutex));
    linked_stream_frame_valid = c2d_module_get_frame_valid(module,
      linked_stream_params->identity, isp_buf->buffer.sequence,
      C2D_GET_STREAM_ID(linked_stream_params->identity),
      linked_stream_params->stream_type);
    PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
  } else {
    linked_stream_frame_valid = 0;
  }

  /* piggy back ack immediately if no valid request */
  if (!stream_frame_valid && !linked_stream_frame_valid) {
      goto piggyback;
  }

  if (IF_EIS_ENABLE(session_params, stream_params)) {
    frame_hold = c2d_module_get_oldest_frame_from_holder(stream_params);
    if (NULL != frame_hold) {
      /* DIS crop event is not sent for frame on HOLD yet. But next frame
         is received. Send frame on HOLD for processing */
      C2D_DBG("dis update not received for previous frame -> %d"
        "for processing, stream %d\n",
        frame_hold->isp_buf.buffer.sequence, stream_params->stream_type);
      c2d_module_send_buf_divert_event(module, frame_hold->identity,
        &frame_hold->isp_buf);
      /* release frame */
      c2d_module_release_holder(stream_params, frame_hold);
    }
    dis_hold = &stream_params->dis_hold;
    /* Check whether DIS frame id is valid &&
       Check whether DIS crop event for this frame has already arrived */
    if (dis_hold->is_valid == TRUE && frame_id <= dis_hold->dis_frame_id) {
      C2D_DBG("DIS already arrived for %d, send for processing"
        " for stream %d\n",
        isp_buf->buffer.sequence, stream_params->stream_type);
      /* Send current frame for processing */
      c2d_module_send_buf_divert_event(module, event->identity, isp_buf);
    } else if (stream_params->is_stream_on || (linked_stream_params &&
      linked_stream_params->is_stream_on)) {
      /* DIS frame id is either invalid or DIS crop event for this frame
         has not arrived yet. HOLD this frame */
      rc = c2d_module_add_frame_to_holder(stream_params, event->identity,
        isp_buf);
      if (rc < 0) {
        C2D_ERR("Error cannot hold frame, release %d, for stream %d",
          isp_buf->buffer.sequence, stream_params->stream_type);
        goto piggyback;
      }
    } else {
      /* Send acknowledge to free  the buffer. */
      goto piggyback;
    }
  } else {
    C2D_DBG("send %d for processing on stream %d\n",
      isp_buf->buffer.sequence, stream_params->stream_type);
    c2d_module_send_buf_divert_event(module, event->identity, isp_buf);
  }


  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
  MCT_OBJECT_UNLOCK(stream_port);
  return 0;

piggyback:
  C2D_DBG("Piggy back frame %d, stream %d, linked stream %d, stream valid %d,"
    "linked stream valid %d", isp_buf->buffer.sequence,
    stream_params->stream_type,
    (linked_stream_params ? linked_stream_params->stream_type : 0),
    stream_frame_valid, linked_stream_frame_valid);
  isp_buf->ack_flag = 1;
  isp_buf->is_buf_dirty = 1;
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
  MCT_OBJECT_UNLOCK(stream_port);

  return 0;

}

/* c2d_module_handle_isp_out_dim_event:
 *
 * Description:
 *
 **/
int32_t c2d_module_handle_isp_out_dim_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t rc;
  uint32_t k;
  if(!module || !event) {
    C2D_ERR("failed, module=%p, event=%p\n", module, event);
    return -EINVAL;
  }
  c2d_module_ctrl_t* ctrl = (c2d_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    C2D_ERR("failed\n");
    return -EFAULT;
  }
  mct_stream_info_t *stream_info =
    (mct_stream_info_t *)(event->u.module_event.module_event_data);
  if(!stream_info) {
    C2D_ERR("failed\n");
    return -EFAULT;
  }
  C2D_HIGH("identity=0x%x, dim=%dx%d\n",
    event->identity, stream_info->dim.width, stream_info->dim.height);
  /* get stream parameters based on the event identity */
  c2d_module_stream_params_t *stream_params = NULL;
  c2d_module_session_params_t *session_params = NULL;
  c2d_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    C2D_ERR("failed\n");
    return -EFAULT;
  }
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  /* format info */
  if ((stream_info->fmt == CAM_FORMAT_YUV_420_NV12) ||
    (stream_info->fmt == CAM_FORMAT_YUV_420_NV12_VENUS)) {
    stream_params->hw_params.input_info.c2d_plane_fmt = C2D_PARAM_PLANE_CBCR;
  } else if ((stream_info->fmt == CAM_FORMAT_YUV_420_NV21) ||
     (stream_info->fmt == CAM_FORMAT_YUV_420_NV21_VENUS)) {
    stream_params->hw_params.input_info.c2d_plane_fmt = C2D_PARAM_PLANE_CRCB;
  } else if (stream_info->fmt == CAM_FORMAT_YUV_422_NV16) {
    stream_params->hw_params.input_info.c2d_plane_fmt =
      C2D_PARAM_PLANE_CBCR422;
  } else if (stream_info->fmt == CAM_FORMAT_YUV_422_NV61) {
    stream_params->hw_params.input_info.c2d_plane_fmt =
      C2D_PARAM_PLANE_CRCB422;
  } else if (stream_info->fmt == CAM_FORMAT_YUV_420_YV12) {
    stream_params->hw_params.input_info.c2d_plane_fmt =
      C2D_PARAM_PLANE_CRCB420;
    stream_params->hw_params.input_info.c2d_plane_fmt = C2D_PARAM_PLANE_CRCB420;
  } else if (stream_info->fmt == CAM_FORMAT_YUV_RAW_8BIT_YUYV) {
    stream_params->hw_params.input_info.c2d_plane_fmt =
      C2D_PARAM_PLANE_YCBYCR422;
  } else if (stream_info->fmt == CAM_FORMAT_YUV_RAW_8BIT_YVYU) {
    stream_params->hw_params.input_info.c2d_plane_fmt =
      C2D_PARAM_PLANE_YCRYCB422;
  } else if (stream_info->fmt == CAM_FORMAT_YUV_RAW_8BIT_UYVY) {
    stream_params->hw_params.input_info.c2d_plane_fmt =
      C2D_PARAM_PLANE_CBYCRY422;
  } else if (stream_info->fmt == CAM_FORMAT_YUV_RAW_8BIT_VYUY) {
    stream_params->hw_params.input_info.c2d_plane_fmt =
      C2D_PARAM_PLANE_CRYCBY422;
  } else {
    C2D_ERR("Format not supported\n");
    PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
    return -EINVAL;
  }
  /* update the dimension of the stream */
  stream_params->hw_params.input_info.width = stream_info->dim.width;
  stream_params->hw_params.input_info.height = stream_info->dim.height;
  stream_params->out_dim_initialized = TRUE;
  stream_params->hw_params.input_info.plane_info =
    stream_info->buf_planes.plane_info;

  C2D_INFO("stream type %d, width %d, height %d, num planes %d",
    stream_info->stream_type, stream_info->dim.width,
    stream_info->dim.height,
     stream_info->buf_planes.plane_info.num_planes);
  for (k = 0;  k < stream_info->buf_planes.plane_info.num_planes; k++) {
    C2D_INFO("plane %d, stride %d, scanline %d, offset_x %d,"
     "offset_y %d, len %d, offset:%d", k,
     stream_info->buf_planes.plane_info.mp[k].stride,
     stream_info->buf_planes.plane_info.mp[k].scanline,
     stream_info->buf_planes.plane_info.mp[k].offset_x,
     stream_info->buf_planes.plane_info.mp[k].offset_y,
     stream_info->buf_planes.plane_info.mp[k].len,
     stream_info->buf_planes.plane_info.mp[k].offset);
  }

  stream_params->hw_params.input_info.cam_fmt = stream_info->fmt;
  /* init crop info */
  stream_params->hw_params.crop_info.stream_crop.x = 0;
  stream_params->hw_params.crop_info.stream_crop.y = 0;
  stream_params->hw_params.crop_info.stream_crop.dx = stream_info->dim.width;
  stream_params->hw_params.crop_info.stream_crop.dy = stream_info->dim.height;
  stream_params->hw_params.crop_info.is_crop.x = 0;
  stream_params->hw_params.crop_info.is_crop.y = 0;
  stream_params->hw_params.crop_info.is_crop.dx = stream_info->dim.width;
  stream_params->hw_params.crop_info.is_crop.dy = stream_info->dim.height;

  if ((stream_params->stream_info->is_type == IS_TYPE_EIS_2_0 ||
    stream_params->interleaved) && !stream_params->single_module){
    c2d_module_stream_params_t *linked_stream_params = NULL;
    mct_stream_info_t           c2d_out_stream_info;
    mct_event_t                 c2d_dim_event;

    memset(&c2d_out_stream_info, 0, sizeof(c2d_out_stream_info));
    memset(&c2d_dim_event, 0, sizeof(c2d_dim_event));
    if (!stream_params->interleaved) {
      stream_params->hw_params.output_info.c2d_plane_fmt =
        stream_params->hw_params.input_info.c2d_plane_fmt;
    }

    linked_stream_params = stream_params->linked_stream_params;

    stream_params->hw_params.crop_info.stream_crop.x = 0;
    stream_params->hw_params.crop_info.stream_crop.y = 0;
    stream_params->hw_params.crop_info.stream_crop.dx =
      stream_info->dim.width;
    stream_params->hw_params.crop_info.stream_crop.dy =
      stream_info->dim.height;
    stream_params->hw_params.crop_info.is_crop.x = 0;
    stream_params->hw_params.crop_info.is_crop.y = 0;
    stream_params->hw_params.crop_info.is_crop.dx =
      stream_info->dim.width;
    stream_params->hw_params.crop_info.is_crop.dy =
      stream_info->dim.height;
    if (stream_params->stream_info->is_type == IS_TYPE_EIS_2_0) {
      stream_params->hw_params.crop_info.is_crop.dx =
        stream_params->hw_params.output_info.width;
      stream_params->hw_params.crop_info.is_crop.dy =
        stream_params->hw_params.output_info.height;
    }
    c2d_out_stream_info.dim.width =
      stream_params->hw_params.output_info.width;
    c2d_out_stream_info.dim.height =
      stream_params->hw_params.output_info.height;
    c2d_out_stream_info.buf_planes.plane_info.num_planes =
      stream_params->hw_params.output_info.plane_info.num_planes;
    C2D_HIGH("c2d's o/p dim:  stream type %d, width %d, height %d,"
      "num planes %d", stream_info->stream_type,
      c2d_out_stream_info.dim.width, c2d_out_stream_info.dim.height,
      c2d_out_stream_info.buf_planes.plane_info.num_planes);
    for (k = 0; k < stream_params->hw_params.output_info.plane_info.num_planes;
      k++) {
      c2d_out_stream_info.buf_planes.plane_info.mp[k].stride =
        stream_params->hw_params.output_info.plane_info.mp[k].stride;
      c2d_out_stream_info.buf_planes.plane_info.mp[k].scanline =
        stream_params->hw_params.output_info.plane_info.mp[k].scanline;
      c2d_out_stream_info.buf_planes.plane_info.mp[k].offset =
        stream_params->hw_params.output_info.plane_info.mp[k].offset;
      c2d_out_stream_info.buf_planes.plane_info.mp[k].offset_x =
        stream_params->hw_params.output_info.plane_info.mp[k].offset;
      c2d_out_stream_info.buf_planes.plane_info.mp[k].offset_y =
        stream_params->hw_params.output_info.plane_info.mp[k].offset;
      c2d_out_stream_info.buf_planes.plane_info.mp[k].len =
        stream_params->hw_params.output_info.plane_info.mp[k].len;
      c2d_out_stream_info.buf_planes.plane_info.frame_len +=
        stream_params->hw_params.output_info.plane_info.mp[k].len;
      C2D_HIGH("c2d's o/p dim: plane %d, stride %d, scanline %d,"
        "offset_x %d, offset_y %d, len %d, offset:%d", k,
        c2d_out_stream_info.buf_planes.plane_info.mp[k].stride,
        c2d_out_stream_info.buf_planes.plane_info.mp[k].scanline,
        c2d_out_stream_info.buf_planes.plane_info.mp[k].offset_x,
        c2d_out_stream_info.buf_planes.plane_info.mp[k].offset_y,
        c2d_out_stream_info.buf_planes.plane_info.mp[k].len,
        c2d_out_stream_info.buf_planes.plane_info.mp[k].offset);
    }
    c2d_out_stream_info.fmt = stream_info->fmt;
    c2d_out_stream_info.stream_type = stream_info->stream_type;
    if ( stream_params->interleaved) {
      c2d_out_stream_info.fmt = stream_params->hw_params.output_info.cam_fmt;
    }

    c2d_dim_event.u.module_event.type = MCT_EVENT_MODULE_ISP_OUTPUT_DIM;
    c2d_dim_event.u.module_event.module_event_data =
      (void *)&c2d_out_stream_info;
    c2d_dim_event.type = MCT_EVENT_MODULE_EVENT;
    c2d_dim_event.identity = event->identity;
    c2d_dim_event.direction = MCT_EVENT_DOWNSTREAM;

    C2D_HIGH("identity=0x%x, dim=%dx%d buff=%dx%d, fmt= %d\n",
      c2d_dim_event.identity, c2d_out_stream_info.dim.width,
      c2d_out_stream_info.dim.height,
      c2d_out_stream_info.buf_planes.plane_info.mp[0].stride,
      c2d_out_stream_info.buf_planes.plane_info.mp[0].scanline,
      c2d_out_stream_info.fmt);

    PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
    rc = c2d_module_send_event_downstream(module, &c2d_dim_event);
    if (rc < 0) {
      C2D_ERR("failed, module_event_type=%d, identity=0x%x",
        c2d_dim_event.u.module_event.type,
        c2d_dim_event.identity);
      return -EFAULT;
    }
    return 0;
  }
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
  rc = c2d_module_send_event_downstream(module, event);
  if(rc < 0) {
    C2D_ERR("failed, module_event_type=%d, identity=0x%x",
      event->u.module_event.type, event->identity);
    return -EFAULT;
  }
  return 0;
}

/* c2d_module_handle_stream_crop_event:
 *
 * Description:
 *
 **/
int32_t c2d_module_handle_stream_crop_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t rc;
  if(!module || !event) {
    C2D_ERR("failed, module=%p, event=%p\n", module, event);
    return -EINVAL;
  }
  c2d_module_ctrl_t* ctrl = (c2d_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    C2D_ERR("failed\n");
    return -EFAULT;
  }
  C2D_DBG("identity=0x%x", event->identity);
  mct_bus_msg_stream_crop_t *stream_crop =
    (mct_bus_msg_stream_crop_t *) event->u.module_event.module_event_data;
  if(!stream_crop) {
    C2D_ERR("failed\n");
    return -EFAULT;
  }
  /* if crop is (0, 0, 0, 0) ignore the event */
  if (stream_crop->x == 0 && stream_crop->y == 0 &&
      stream_crop->crop_out_x == 0 && stream_crop->crop_out_y == 0) {
    return 0;
  }
  /* get stream parameters based on the event identity */
  c2d_module_stream_params_t *stream_params = NULL;
  c2d_module_session_params_t *session_params = NULL;
  c2d_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    C2D_ERR("failed\n");
    return -EFAULT;
  }

  /* if crop is (0, 0, 0, 0) ignore the event */
  if (stream_crop->x == 0 && stream_crop->y == 0 &&
      stream_crop->crop_out_x == 0 && stream_crop->crop_out_y == 0) {
    //return 0;
      PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
      stream_params->hw_params.crop_info.stream_crop.x = stream_crop->x;
      stream_params->hw_params.crop_info.stream_crop.y = stream_crop->y;
      stream_params->hw_params.crop_info.stream_crop.dx =
        stream_params->hw_params.input_info.width;
      stream_params->hw_params.crop_info.stream_crop.dy =
        stream_params->hw_params.input_info.height;
      stream_crop->crop_out_x =
        stream_params->hw_params.input_info.width;
      stream_crop->crop_out_y =
        stream_params->hw_params.input_info.height;

      PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
      rc = c2d_module_send_event_downstream(module, event);
      if(rc < 0) {
        C2D_ERR("failed, module_event_type=%d, identity=0x%x",
          event->u.module_event.type, event->identity);
        return -EFAULT;
      }
      return 0;

  }

  if (stream_params->stream_info->is_type == IS_TYPE_EIS_2_0) {

    if (stream_params->hw_params.input_info.width &&
        stream_params->hw_params.output_info.width) {
      c2d_module_calculate_stream_crop_info(stream_params, stream_crop);
    }

    rc = c2d_module_send_event_downstream(module, event);
    if (rc < 0) {
       C2D_ERR("failed, module_event_type=%d, identity=0x%x",
        event->u.module_event.type, event->identity);
       return -EFAULT;
    }
    return 0;
  } else if (stream_params->interleaved && !stream_params->single_module) {
    rc = c2d_module_send_event_downstream(module, event);
    if (rc < 0) {
       C2D_ERR("failed, module_event_type=%d, identity=0x%x",
         event->u.module_event.type, event->identity);
       return -EFAULT;
    }
    return 0;
  }

  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  stream_params->hw_params.crop_info.stream_crop.x = stream_crop->x;
  stream_params->hw_params.crop_info.stream_crop.y = stream_crop->y;
  stream_params->hw_params.crop_info.stream_crop.dx = stream_crop->crop_out_x;
  stream_params->hw_params.crop_info.stream_crop.dy = stream_crop->crop_out_y;
  C2D_HIGH("stream_crop.x=%d, stream_crop.y=%d, stream_crop.dx=%d,"
           " stream_crop.dy=%d, identity=0x%x",
           stream_crop->x, stream_crop->y, stream_crop->crop_out_x,
           stream_crop->crop_out_y, event->identity);
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

  rc = c2d_module_send_event_downstream(module, event);
  if(rc < 0) {
    C2D_ERR("failed, module_event_type=%d, identity=0x%x",
      event->u.module_event.type, event->identity);
    return -EFAULT;
  }
  return 0;
}

/* c2d_module_handle_dis_update_event:
 *
 * Description:
 *
 **/
int32_t c2d_module_handle_dis_update_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t rc, j;

  if(!module || !event) {
    C2D_ERR("failed, module=%p, event=%p\n", module, event);
    return -EINVAL;
  }
  c2d_module_ctrl_t* ctrl = (c2d_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    C2D_ERR("failed\n");
    return -EFAULT;
  }
  is_update_t *is_update =
    (is_update_t *) event->u.module_event.module_event_data;
  if(!is_update) {
    C2D_ERR("failed\n");
    return -EFAULT;
  }
  /* get stream parameters based on the event identity */
  c2d_module_stream_params_t *stream_params = NULL;
  c2d_module_stream_params_t *s_params = NULL;
  c2d_module_session_params_t *session_params = NULL;
  c2d_module_frame_hold_t     *frame_hold = FALSE;
  c2d_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!session_params || !stream_params) {
    C2D_ERR("failed session %p or stream %p\n",  session_params, stream_params);
    return -EFAULT;
  }

  if (stream_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    C2D_DBG("DIS event should not be recieved on offline stream");
    return 0;
  } else {
    for(j = 0; j < C2D_MODULE_MAX_STREAMS; j++) {
      if (session_params->stream_params[j]) {
        switch (session_params->stream_params[j]->stream_type) {
        case CAM_STREAM_TYPE_PREVIEW:
        case CAM_STREAM_TYPE_VIDEO:
          s_params = session_params->stream_params[j];
          PTHREAD_MUTEX_LOCK(&(s_params->mutex));
          /* Check whether DIS is enabled, else return without storing */
          if (s_params->dis_enable == 0) {
            C2D_HIGH("dis enable %d\n", s_params->dis_enable);
            PTHREAD_MUTEX_UNLOCK(&(s_params->mutex));
            continue;
          }
          s_params->hw_params.crop_info.is_crop.use_3d = is_update->use_3d;
          memcpy(&s_params->hw_params.crop_info.is_crop.transform_matrix,
            &is_update->transform_matrix,
            sizeof(s_params->hw_params.crop_info.is_crop.transform_matrix));

          s_params->hw_params.crop_info.is_crop.transform_type =
            is_update->transform_type;

          s_params->hw_params.crop_info.is_crop.x =
            (is_update->x * s_params->hw_params.output_info.width) /
            is_update->width;
          s_params->hw_params.crop_info.is_crop.y =
            (is_update->y * s_params->hw_params.output_info.height) /
            is_update->height;
          s_params->hw_params.crop_info.is_crop.dx =
            (is_update->width *  s_params->hw_params.output_info.width) /
            is_update->width;
          s_params->hw_params.crop_info.is_crop.dy =
            (is_update->height * s_params->hw_params.output_info.height) /
            is_update->height;

          /* Update frame id in session_params */
          s_params->dis_hold.is_valid = TRUE;
          s_params->dis_hold.dis_frame_id = is_update->frame_id;
          C2D_HIGH("stream %d, stream_identity 0x%x, is_crop.x=%d, is_crop.y=%d,"
           "is_crop.dx=%d, is_crop.dy=%d, rxved identity=0x%x, is_frame_id %d",
            s_params->stream_type, s_params->identity,
            s_params->hw_params.crop_info.is_crop.x,
            s_params->hw_params.crop_info.is_crop.y,
            s_params->hw_params.crop_info.is_crop.dx,
            s_params->hw_params.crop_info.is_crop.dy,
            event->identity, is_update->frame_id);
          PTHREAD_MUTEX_UNLOCK(&(s_params->mutex));
          break;
        case CAM_STREAM_TYPE_OFFLINE_PROC:
          break;
        default:
          break;
        }
      }
    }
   /* Send buffer */
    for(j = 0; j < C2D_MODULE_MAX_STREAMS; j++) {
      if (session_params->stream_params[j]) {
        switch (session_params->stream_params[j]->stream_type) {
        case CAM_STREAM_TYPE_PREVIEW:
        case CAM_STREAM_TYPE_VIDEO:
          s_params = session_params->stream_params[j];
          PTHREAD_MUTEX_LOCK(&(s_params->mutex));
         /* Check whether DIS is enabled, else return without storing */
          if (s_params->dis_enable == 0) {
            C2D_ERR("dis enable %d\n", s_params->dis_enable);
            PTHREAD_MUTEX_UNLOCK(&(s_params->mutex));
            continue;
          }
          /* put this in a seperate function */
          c2d_module_frame_hold_t *p_holder;
          int32_t i, f_idx = s_params->frame_hold_f_idx;
          if (s_params->frame_hold_cnt > 0) {
            for (i = f_idx; i < (C2D_NUM_FRAME_HOLD + f_idx); i++) {
              p_holder = &s_params->frame_hold[i % C2D_NUM_FRAME_HOLD];
              if (!s_params->frame_hold_cnt) {
                /* we have reached the end of circular buffer */
                break;
              }
              if ((s_params->dis_hold.dis_frame_id <
                p_holder->isp_buf.buffer.sequence)) {
                /* frame has not arrived yet*/
                break;
              } else if (p_holder->isp_buf.buffer.sequence <
                s_params->dis_hold.dis_frame_id) {
                C2D_DBG("dis update not received for frame %d for processing for stream %d",
                  p_holder->isp_buf.buffer.sequence, s_params->stream_type);
              } else {
                /* dis update on valid frame send for processing */
              }
              C2D_DBG("send frame %d for processing for stream %d\n",
                p_holder->isp_buf.buffer.sequence, s_params->stream_type);
              /* send frame for processing */
              c2d_module_send_buf_divert_event(module, p_holder->identity,
                &p_holder->isp_buf);
              c2d_module_release_holder(s_params, p_holder);
            }
          }
          PTHREAD_MUTEX_UNLOCK(&(s_params->mutex));
          break;
        case CAM_STREAM_TYPE_OFFLINE_PROC:
          break;
        default:
          break;
        }
      }
    }
  }
  return 0;
}

/* c2d_module_handle_dis_config_event:
 *
 * Description:
 *
 **/
int32_t c2d_module_handle_dis_config_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t rc;
  uint32_t dis_config_identity;

  if(!module || !event) {
    C2D_ERR("failed, module=%p, event=%p\n", module, event);
    return -EINVAL;
  }
  c2d_module_ctrl_t* ctrl = (c2d_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    C2D_ERR("failed\n");
    return -EFAULT;
  }
  isp_dis_config_info_t *dis_config =
    (isp_dis_config_info_t *) event->u.module_event.module_event_data;
  if(!dis_config) {
    C2D_ERR("failed\n");
    return -EFAULT;
  }

  C2D_LOW(" MCT_EVENT_MODULE_ISP_DIS_CONFIG, sid = %u, strid = %x, "
    " col_num = %u, row_num = %u, w = %u, h = %u",
    dis_config->session_id, dis_config->stream_id,
     dis_config->col_num, dis_config->row_num,
    dis_config->width, dis_config->height);

  dis_config_identity = pack_identity(dis_config->session_id,
                          dis_config->stream_id);

  /* get stream parameters based on the event identity */
  c2d_module_stream_params_t *stream_params = NULL;
  c2d_module_session_params_t *session_params = NULL;
  c2d_module_stream_params_t  *linked_stream_params = NULL;
  c2d_module_get_params_for_identity(ctrl, dis_config_identity,
    &session_params, &stream_params);
  if(!stream_params) {
    C2D_ERR("failed\n");
    return -EFAULT;
  }

  linked_stream_params = stream_params->linked_stream_params;
  if (!linked_stream_params) {
    C2D_LOW("No linked stream.");
    C2D_LOW("identity=0x%x stream op %d X %d",
      dis_config_identity, stream_params->hw_params.output_info.width,
      stream_params->hw_params.output_info.height);
  } else {
    C2D_LOW("identity=0x%x stream op %d X %d linked stream %d X %d",
      dis_config_identity, stream_params->hw_params.output_info.width,
      stream_params->hw_params.output_info.height,
      linked_stream_params->hw_params.output_info.width,
      linked_stream_params->hw_params.output_info.height);
  }

  if((dis_config->width !=
    (uint32_t)stream_params->hw_params.output_info.width) ||
    (dis_config->height !=
    (uint32_t)stream_params->hw_params.output_info.height)) {
    C2D_DBG("Dimension Mismatch. Updating. \n");

    c2d_module_update_stream_dimensions(ctrl, stream_params, dis_config);
    if (linked_stream_params) {
      c2d_module_update_stream_dimensions(ctrl, linked_stream_params, dis_config);
    }
    /* Destroy and Re-create the native buffer queue */
    /* free native buffers */
    uint32_t link_identity =
      stream_params->streaming_identity;

    pp_native_buf_mgr_free_queue(&ctrl->native_buf_mgr,
      C2D_GET_SESSION_ID(link_identity),
      C2D_GET_STREAM_ID(link_identity));
    c2d_module_init_native_buffers(ctrl,
      &stream_params->hw_params.output_info.plane_info, link_identity,
      C2D_NUM_NATIVE_BUFFERS);

    //Send ISP output dim event since buffer size has changed
    if (stream_params->linked_stream_params)
      c2d_module_send_isp_output_dimension_event(module,
        stream_params->linked_stream_params);
  }
  //Send ISP output dim event since dimensions have changes
  c2d_module_send_isp_output_dimension_event(module, stream_params);

  rc = c2d_module_send_event_downstream(module, event);
  if(rc < 0) {
    C2D_ERR("failed, module_event_type=%d, identity=0x%x",
      event->u.module_event.type, event->identity);
    return -EFAULT;
  }
  return 0;
}


/* c2d_module_handle_stream_cfg_event:
 *
 * Description:
 *
 **/
int32_t c2d_module_handle_stream_cfg_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t rc;
  if(!module || !event) {
    C2D_ERR("failed, module=%p, event=%p\n", module, event);
    return -EINVAL;
  }
  c2d_module_ctrl_t* ctrl = (c2d_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    C2D_ERR("failed\n");
    return -EFAULT;
  }
  sensor_out_info_t *sensor_out_info =
    (sensor_out_info_t *)(event->u.module_event.module_event_data);
  if (!sensor_out_info) {
    C2D_ERR("failed\n");
    return -EFAULT;
  }
  /* get stream parameters based on the event identity */
  c2d_module_stream_params_t *stream_params = NULL;
  c2d_module_session_params_t *session_params = NULL;
  c2d_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!session_params) {
    C2D_ERR("failed\n");
    return -EFAULT;
  }
  /* apply this to all streams for that session */
  int i;
  for(i=0; i<C2D_MODULE_MAX_STREAMS; i++) {
    if(session_params->stream_params[i] && stream_params->single_module) {
      PTHREAD_MUTEX_LOCK(&(session_params->stream_params[i]->mutex));
      session_params->stream_params[i]->hfr_skip_info.frame_offset =
        sensor_out_info->num_frames_skip + 1;
      session_params->stream_params[i]->hfr_skip_info.input_fps =
        sensor_out_info->max_fps;
      c2d_module_update_hfr_skip(session_params->stream_params[i]);
      PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));
      C2D_HIGH("frame_offset=%d, input_fps=%.2f, identity=0x%x",
        session_params->stream_params[i]->hfr_skip_info.frame_offset,
        session_params->stream_params[i]->hfr_skip_info.input_fps,
        session_params->stream_params[i]->identity);
    }
  }
  rc = c2d_module_send_event_downstream(module, event);
  if(rc < 0) {
    C2D_ERR("failed, module_event_type=%d, identity=0x%x",
      event->u.module_event.type, event->identity);
    return -EFAULT;
  }
  return 0;
}

/* c2d_module_set_parm_hfr_mode:
 *
 **/
static int32_t c2d_module_set_parm_hfr_mode(c2d_module_ctrl_t *ctrl,
  uint32_t identity, cam_hfr_mode_t hfr_mode)
{
  if(!ctrl) {
    C2D_ERR("failed");
    return -EFAULT;
  }
  /* get parameters based on the event identity */
  c2d_module_stream_params_t *stream_params = NULL;
  c2d_module_session_params_t *session_params = NULL;
  c2d_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!session_params) {
    C2D_ERR("failed\n");
    return -EFAULT;
  }
  /* apply this to all streams where hfr skip is required */
  uint32_t i;
  for(i=0; i<C2D_MODULE_MAX_STREAMS; i++) {
    if(session_params->stream_params[i] && stream_params->single_module) {
      PTHREAD_MUTEX_LOCK(&(session_params->stream_params[i]->mutex));
      if(session_params->stream_params[i]->hfr_skip_info.skip_required) {
        switch(hfr_mode) {
        case CAM_HFR_MODE_OFF:
          session_params->stream_params[i]->hfr_skip_info.skip_count = 0;
          break;
        case CAM_HFR_MODE_60FPS:
          session_params->stream_params[i]->hfr_skip_info.skip_count = 1;
          break;
        case CAM_HFR_MODE_90FPS:
          session_params->stream_params[i]->hfr_skip_info.skip_count = 2;
          break;
        case CAM_HFR_MODE_120FPS:
          session_params->stream_params[i]->hfr_skip_info.skip_count = 3;
          break;
        case CAM_HFR_MODE_150FPS:
          session_params->stream_params[i]->hfr_skip_info.skip_count = 4;
          break;
        default:
          C2D_ERR("bad hfr_mode=%d", hfr_mode);
          PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));
          return -EINVAL;
        }
      }
      PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));
    }
  }
  return 0;
}

/* c2d_module_set_parm_fps_range:
 *
 **/
static int32_t c2d_module_set_parm_fps_range(c2d_module_ctrl_t *ctrl,
  uint32_t identity, cam_fps_range_t *fps_range)
{
  if ((!ctrl) || (!fps_range)) {
    C2D_ERR("failed");
    return -EFAULT;
  }
  /* get parameters based on the event identity */
  c2d_module_stream_params_t *stream_params = NULL;
  c2d_module_session_params_t *session_params = NULL;
  c2d_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!session_params) {
    C2D_ERR("failed\n");
    return -EFAULT;
  }
  /* apply this to all streams where hfr skip is required */
  int i;
  session_params->fps_range.max_fps = fps_range->max_fps;
  session_params->fps_range.min_fps = fps_range->min_fps;
  session_params->fps_range.video_max_fps = fps_range->video_max_fps;
  session_params->fps_range.video_min_fps = fps_range->video_min_fps;
  C2D_HIGH("max_fps %f min_fps %f video_max_fps %f video_min_fps %f",
    fps_range->max_fps, fps_range->min_fps,
    fps_range->video_max_fps, fps_range->video_min_fps);
  for (i=0; i<C2D_MODULE_MAX_STREAMS; i++) {
    if (session_params->stream_params[i] && stream_params->single_module) {
      PTHREAD_MUTEX_LOCK(&(session_params->stream_params[i]->mutex));
      if (session_params->stream_params[i]->hfr_skip_info.skip_required) {
        if(session_params->stream_params[i]->stream_type ==
          CAM_STREAM_TYPE_VIDEO) {
          session_params->stream_params[i]->hfr_skip_info.output_fps =
           fps_range->video_max_fps;
        } else {
          session_params->stream_params[i]->hfr_skip_info.output_fps =
           fps_range->max_fps;
        }
        c2d_module_update_hfr_skip(session_params->stream_params[i]);
      }
      PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));
    }
  }
  return 0;
}

/*
 * c2d_module_set_parm_dis:
 *    ctrl: c2d_module control structure
 *    identity: current strean identity
 *    dis_enable: flag used to enable or disabel DIS
 *
 *    This is a event handler of event CAM_INTF_PARM_DIS_ENABLE. This event is
 *    is used to enable/diable DIS
 *
 *    It returns 0 at success.
 **/
static int32_t c2d_module_set_parm_dis(c2d_module_ctrl_t *ctrl,
  uint32_t identity, int32_t dis_enable)
{
  if (!ctrl) {
    C2D_ERR("failed");
    return -EFAULT;
  }
  /* get parameters based on the event identity */
  c2d_module_stream_params_t *stream_params = NULL;
  c2d_module_stream_params_t *s_params = NULL;
  c2d_module_session_params_t *session_params = NULL;
  int32_t j;
  c2d_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if (!session_params || !stream_params) {
    C2D_ERR("failed session_params %p or stream_params %p NULL\n",
      session_params, stream_params);
    return -EFAULT;
  }

  session_params->dis_enable = dis_enable;
  for(j = 0; j < C2D_MODULE_MAX_STREAMS; j++) {
    if (session_params->stream_params[j]) {
      switch (session_params->stream_params[j]->stream_type) {
      case CAM_STREAM_TYPE_PREVIEW:
      case CAM_STREAM_TYPE_VIDEO:
          s_params = session_params->stream_params[j];
          PTHREAD_MUTEX_LOCK(&(s_params->mutex));
          /* Update dis_enable flag in session_params */
          if (s_params->stream_info->is_type == IS_TYPE_EIS_2_0) {
            s_params->dis_enable = dis_enable;
            C2D_HIGH("dis_enable %d\n", dis_enable);
            if (dis_enable == 0) {
            /* Invalidate DIS hold flag */
              s_params->dis_hold.is_valid = FALSE;
            }
          }
          PTHREAD_MUTEX_UNLOCK(&(s_params->mutex));
         break;
      case CAM_STREAM_TYPE_OFFLINE_PROC:
        break;
      default:
        break;
      }
    }
  }
  return 0;
}

int32_t c2d_module_handle_sof_set_parm_event(mct_module_t* module,
  mct_event_t* event)
{
  C2D_LOW("E");
  mct_event_super_control_parm_t *param = NULL;
  uint32_t index = 0;
  int32_t rc = 0;
  mct_event_t sub_event;
  c2d_module_ctrl_t *ctrl = NULL;
  c2d_module_stream_params_t *stream_params = NULL;
  c2d_module_session_params_t *session_params = NULL;

  MCT_PROF_LOG_BEG(PROF_C2D_SP);
  if (!module || !event) {
   C2D_ERR("failed, module=%p, event=%p", module, event);
   return -EINVAL;
 }

  param =
    (mct_event_super_control_parm_t *)event->u.ctrl_event.control_event_data;
  if (!param) {
    C2D_ERR("failed: param %p", param);
    return FALSE;
  }

  ctrl = (c2d_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    C2D_ERR("failed");
    return -EFAULT;
  }

  /* get parameters based on the session id */
  c2d_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if (!session_params) {
    C2D_ERR("failed\n");
    return -EFAULT;
  }


  C2D_DBG("SOF for frame id %d", event->u.ctrl_event.current_frame_id);
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
    rc = c2d_module_handle_set_parm_event(module, &sub_event);
  }
  MCT_PROF_LOG_END();

  rc = c2d_module_send_event_downstream(module, event);
  if(rc < 0) {
    C2D_ERR("failed, ctrl_event_type=%d, identity=0x%x",
      event->u.ctrl_event.type, event->identity);
    return -EFAULT;
  }

  C2D_LOW("X");
  return 0;

}

/* c2d_module_handle_set_parm_event:
 *
 * Description:
 *   Handle the set_parm event.
 **/
int32_t c2d_module_handle_set_parm_event(mct_module_t* module,
  mct_event_t* event)
{
  if(!module || !event) {
    C2D_ERR("failed, module=%p, event=%p", module, event);
    return -EINVAL;
  }
  mct_event_control_parm_t *ctrl_parm =
    (mct_event_control_parm_t *) event->u.ctrl_event.control_event_data;
  if(!ctrl_parm) {
    C2D_ERR("failed");
    return -EFAULT;
  }
  c2d_module_ctrl_t *ctrl = (c2d_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    C2D_ERR("failed");
    return -EFAULT;
  }
  int32_t rc;

  switch (ctrl_parm->type) {
  case CAM_INTF_PARM_HFR: {
    if(!(ctrl_parm->parm_data)) {
      C2D_ERR("failed");
      return -EFAULT;
    }
    cam_hfr_mode_t hfr_mode =
      *(cam_hfr_mode_t *)(ctrl_parm->parm_data);
    C2D_DBG("CAM_INTF_PARM_HFR, mode=%d, identity=0x%x",
      hfr_mode, event->identity);
    rc = c2d_module_set_parm_hfr_mode(ctrl, event->identity, hfr_mode);
    if (rc < 0) {
      C2D_ERR("failed");
      return rc;
    }
    break;
  }
  case CAM_INTF_PARM_FPS_RANGE: {
    if(!(ctrl_parm->parm_data)) {
      C2D_ERR("failed");
      return -EFAULT;
    }
    cam_fps_range_t *fps_range = (cam_fps_range_t *)(ctrl_parm->parm_data);
    C2D_HIGH("CAM_INTF_PARM_FPS_RANGE,, max_fps=%.2f, identity=0x%x",
      fps_range->max_fps, event->identity);
    rc = c2d_module_set_parm_fps_range(ctrl, event->identity, fps_range);
    if(rc < 0) {
      C2D_ERR("failed");
      return rc;
    }
    break;
  }
  case CAM_INTF_PARM_DIS_ENABLE: {
    if (!(ctrl_parm->parm_data)) {
      C2D_ERR("failed");
      return -EFAULT;
    }
    int32_t dis_enable =
      *(int32_t *)(ctrl_parm->parm_data);
    C2D_DBG("CAM_INTF_PARM_DIS_ENABLE, enable=%d, identity=0x%x",
      dis_enable, event->identity);
    rc = c2d_module_set_parm_dis(ctrl, event->identity, dis_enable);
    if (rc < 0) {
      C2D_ERR("failed");
      return rc;
    }
    break;
  }
  case CAM_INTF_PARM_HAL_VERSION: {
    if(!(ctrl_parm->parm_data)) {
      C2D_ERR("failed");
      return -EFAULT;
    }
    int32_t value = *(int32_t*)(ctrl_parm->parm_data);
    c2d_module_session_params_t* session_params = NULL;
    c2d_module_stream_params_t* stream_params = NULL;
    C2D_DBG("CAM_INTF_PARM_HAL_VERSION, value=%d, identity=0x%x",
      value, event->identity);
    /* get parameters based on the event identity */
    rc = c2d_module_get_params_for_identity(ctrl, event->identity,
      &session_params, &stream_params);
    if(!session_params) {
      C2D_ERR("failed\n");
      return -EFAULT;
    }
    session_params->hal_version = value;
    break;
  }
  case CAM_INTF_META_STREAM_ID: {
    c2d_module_session_params_t *session_params = NULL;
    c2d_module_stream_params_t  *stream_params = NULL;
    uint32_t j = 0;
    uint32_t q_idx = 0;
    uint32_t apply_frame_id = event->u.ctrl_event.current_frame_id;

    C2D_DBG("CAM_INTF_META_STREAM_ID\n");
    /* get parameters based on the event identity */
    rc = c2d_module_get_params_for_identity(ctrl, event->identity,
      &session_params, &stream_params);
    if (!session_params ||  !stream_params) {
      C2D_ERR("null session params %p or stream params %p, failed\n",
        session_params, stream_params);
      return -EFAULT;
    }

    if (stream_params->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) {
      apply_frame_id = event->u.ctrl_event.current_frame_id +
        session_params->max_apply_delay -
        session_params->c2d_delay;
    }
    q_idx = apply_frame_id % PER_FRAME_QUEUE_SIZE;
    for (j = 0; j < ((cam_stream_ID_t *)ctrl_parm->parm_data)->num_streams; j++) {
      C2D_LOW("META_STREAM_ID q_idx %d frame id %d streamID:%d",
        q_idx, apply_frame_id,
        ((cam_stream_ID_t *)ctrl_parm->parm_data)->stream_request[j].streamID);
    }
    PTHREAD_MUTEX_LOCK(&(session_params->c2d_per_frame_mutex));
    session_params->valid_stream_ids[q_idx] = *(cam_stream_ID_t *)(ctrl_parm->parm_data);
    PTHREAD_MUTEX_UNLOCK(&(session_params->c2d_per_frame_mutex));
    if (rc < 0) {
      C2D_ERR("failed %d", rc);
      return rc;
    }
    break;
  }
  case CAM_INTF_META_STREAM_INFO: {
    mct_event_t               c2d_event;
    isp_preferred_streams    preferred_mapping;
    c2d_module_session_params_t* session_params = NULL;
    c2d_module_stream_params_t* stream_params = NULL;
    cam_stream_size_info_t     *meta_stream_info =
        (cam_stream_size_info_t *)(ctrl_parm->parm_data);
    uint32_t i = 0;
    bool video_4k = 0;
    bool is_eis_2_0 = 0;

    C2D_HIGH("CAM_INTF_META_STREAM_INFO, identity=0x%x", event->identity);
    /* get parameters based on the event identity */
    rc = c2d_module_get_params_for_identity(ctrl, event->identity,
      &session_params, &stream_params);
    if(!session_params || !stream_params) {
      C2D_ERR("failed, session_params %p or stream_params %p NULL\n",
        session_params, stream_params);
      return -EFAULT;
    }

    for (i = 0; i < meta_stream_info->num_streams; i++) {
      C2D_INFO("META_STREAM_INFO type = %d w x h : %d x %d pp_mask = 0x%x",
        meta_stream_info->type[i],
        meta_stream_info->stream_sizes[i].width,
      meta_stream_info->stream_sizes[i].height,
      meta_stream_info->postprocess_mask[i]);
      if (meta_stream_info->type[i] == CAM_STREAM_TYPE_VIDEO) {
        video_4k = IS_4k_VIDEO(meta_stream_info->type[i],
          meta_stream_info->stream_sizes[i].width,
          meta_stream_info->stream_sizes[i].height,
          meta_stream_info->postprocess_mask[i]);
        if(meta_stream_info->is_type[i] == IS_TYPE_EIS_2_0) {
          is_eis_2_0 = true;
        }
      }
    }

    PTHREAD_MUTEX_LOCK(&(session_params->c2d_per_frame_mutex));
    memset(&session_params->valid_stream_ids[0], 0,
      sizeof(session_params->valid_stream_ids));
    PTHREAD_MUTEX_UNLOCK(&(session_params->c2d_per_frame_mutex));

    if ((stream_params->interleaved) || (is_eis_2_0 &&
      (!video_4k))) {
      preferred_mapping.stream_num = 1;
      preferred_mapping.streams[0].stream_mask =
        (1 << CAM_STREAM_TYPE_PREVIEW) | (1 << CAM_STREAM_TYPE_VIDEO);
      preferred_mapping.streams[0].max_streams_num = 2;
      if (stream_params->interleaved)
        preferred_mapping.streams[0].stream_mask |=
        (1 << CAM_STREAM_TYPE_ANALYSIS);

      c2d_event.type = MCT_EVENT_MODULE_EVENT;
      c2d_event.identity = event->identity;
      c2d_event.direction = MCT_EVENT_UPSTREAM;
      c2d_event.u.module_event.type =
        MCT_EVENT_MODULE_SET_PREFERRED_STREAMS_MAPPING;
      c2d_event.u.module_event.module_event_data = &preferred_mapping;
      c2d_module_send_event_upstream(module, &c2d_event);
      C2D_HIGH("Sending prefered streams to ISP, identity=0x%x",
        event->identity);
    }

    /* Need to reset after every stream configuration */
    stream_params->dis_hold.is_valid = FALSE;
    stream_params->dis_hold.dis_frame_id = 0;

    break;
  }
  default:
    break;
  }

  /* Since handle set param event is being reused from sof_set_param_event
     this function does not forward the event to downstream module.*/
#if 0
  rc = c2d_module_send_event_downstream(module, event);
  if(rc < 0) {
    C2D_ERR("failed, module_event_type=%d, identity=0x%x",
      event->u.module_event.type, event->identity);
    return -EFAULT;
  }
#endif
  return 0;
}

/* c2d_module_handle_streamon_event:
 *
 **/
int32_t c2d_module_handle_streamon_event(mct_module_t* module,
  mct_event_t* event)
{
  c2d_module_stream_buff_info_t   stream_buff_info;
  c2d_hardware_stream_buff_info_t hw_strm_buff_info;
  mct_stream_info_t              *streaminfo =
    (mct_stream_info_t *)event->u.ctrl_event.control_event_data;
  c2d_module_ctrl_t              *ctrl =
    (c2d_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  c2d_hardware_cmd_t              cmd;
  boolean                         rc = -EINVAL;
  mct_event_t                     new_event;
  stats_get_data_t                stats_get;
  mct_port_t                      *stream_port = NULL;

  /* get stream parameters */
  c2d_module_session_params_t* session_params = NULL;
  c2d_module_stream_params_t*  stream_params = NULL;
  c2d_module_stream_params_t*  linked_stream_params = NULL;
  c2d_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    C2D_ERR("failed\n");
    return -EFAULT;
  }

  stream_port = (mct_port_t *) stream_params->parent;
  if (stream_port == NULL) {
    C2D_ERR("invalid stream port, failed\n");
    return -EFAULT;
  }

  MCT_OBJECT_LOCK(stream_port);
  linked_stream_params = stream_params->linked_stream_params;

  memset(&stream_buff_info, 0, sizeof(c2d_module_stream_buff_info_t));
  memset(&hw_strm_buff_info, 0, sizeof(c2d_hardware_stream_buff_info_t));

  if ((stream_params->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) &&
    (stream_params->stream_type != CAM_STREAM_TYPE_PARM)) {
    session_params->real_time_stream_cnt++;
  }
  /* attach the identity */
  stream_buff_info.identity = event->identity;
  stream_params->stream_info->img_buffer_list = streaminfo->img_buffer_list;
  /* traverse through the mct stream buff list and create c2d module's
     own list of buffer info */
  if (mct_list_traverse(streaminfo->img_buffer_list,
    c2d_module_util_map_buffer_info, &stream_buff_info) == FALSE) {
    C2D_ERR("error creating stream buff list\n");
    goto C2D_MODULE_STREAMON_ERROR1;
  }

  /* create and translate to hardware buffer array */
  hw_strm_buff_info.buffer_info = (c2d_hardware_buffer_info_t *)CAM_MALLOC(
    sizeof(c2d_hardware_buffer_info_t) * stream_buff_info.num_buffs);
  if(NULL == hw_strm_buff_info.buffer_info) {
    C2D_ERR("error creating hw buff list\n");
    goto C2D_MODULE_STREAMON_ERROR1;
  }

  hw_strm_buff_info.identity = stream_buff_info.identity;
  if (mct_list_traverse(stream_buff_info.buff_list,
    c2d_module_util_create_hw_stream_buff, &hw_strm_buff_info) == FALSE) {
    C2D_ERR("error creating stream buff list\n");
    goto C2D_MODULE_STREAMON_ERROR2;
  }

  if(hw_strm_buff_info.num_buffs != stream_buff_info.num_buffs) {
    C2D_ERR("error creating stream buff list\n");
    goto C2D_MODULE_STREAMON_ERROR2;
  }

  cmd.type = C2D_HW_CMD_STREAMON;
  cmd.u.stream_buff_list = &hw_strm_buff_info;
  rc = c2d_hardware_process_command(ctrl->c2dhw, cmd);
  if(rc < 0) {
    C2D_ERR("failed\n");
    goto C2D_MODULE_STREAMON_ERROR2;
  }
  /* change state to stream ON */
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  stream_params->is_stream_on = TRUE;
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

  rc = c2d_module_send_event_downstream(module,event);
  if(rc < 0) {
    C2D_ERR("failed\n");
    goto C2D_MODULE_STREAMON_ERROR2;
  }


  rc = 0;

C2D_MODULE_STREAMON_ERROR2:
  free(hw_strm_buff_info.buffer_info);

C2D_MODULE_STREAMON_ERROR1:
  mct_list_traverse(stream_buff_info.buff_list,
    c2d_module_util_free_buffer_info, &stream_buff_info);
  mct_list_free_list(stream_buff_info.buff_list);

  MCT_OBJECT_UNLOCK(stream_port);
  return rc;
}

/* c2d_module_handle_streamoff_event:
 *
 **/
int32_t c2d_module_handle_streamoff_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t rc;
  int32_t handle_downstream_error;
  c2d_module_frame_hold_t *frame_hold = NULL;
  mct_port_t *stream_port = NULL;

  if(!module || !event) {
    C2D_ERR("failed, module=%p, event=%p\n", module, event);
    return -EINVAL;
  }
  mct_stream_info_t *streaminfo =
    (mct_stream_info_t *)event->u.ctrl_event.control_event_data;
  uint32_t identity = event->identity;
  C2D_HIGH("info: doing stream-off for identity 0x%x", identity);

  c2d_module_ctrl_t* ctrl = (c2d_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    C2D_ERR("failed\n");
    return -EINVAL;
  }
  c2d_module_session_params_t* session_params = NULL;
  c2d_module_stream_params_t*  stream_params = NULL;
  c2d_module_stream_params_t*  linked_stream_params = NULL;
  c2d_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!stream_params || !session_params) {
    C2D_ERR("failed stream_params %p, session_params %p NULL\n",
      stream_params, session_params);
    return -EFAULT;
  }

  stream_port = (mct_port_t *) stream_params->parent;
  if (stream_port == NULL) {
    C2D_ERR("invalid stream port, failed\n");
    return -EFAULT;
  }
  MCT_OBJECT_LOCK(stream_port);

  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  /* change the state of this stream to OFF, this will prevent
     any incoming buffers to be added to the processing queue  */
  stream_params->is_stream_on = FALSE;

  linked_stream_params = stream_params->linked_stream_params;

  c2d_module_frame_hold_t *p_holder;
  int32_t i, f_idx = stream_params->frame_hold_f_idx;
  if (stream_params->frame_hold_cnt > 0) {
    for (i = f_idx; i < (C2D_NUM_FRAME_HOLD + f_idx); i++) {
      p_holder = &stream_params->frame_hold[i % C2D_NUM_FRAME_HOLD];
      if (!stream_params->frame_hold_cnt) {
        /* we have reached the end of circular buffer */
        break;
      }
      if ((stream_params->identity == p_holder->identity) ||
        (linked_stream_params && (linked_stream_params->identity ==
        p_holder->identity))) {
        /* send frame for processing */
        C2D_DBG("send %d for processing\n",
          p_holder->isp_buf.buffer.sequence);
        c2d_module_send_buf_divert_event(module, p_holder->identity,
          &p_holder->isp_buf);
        c2d_module_release_holder(stream_params, p_holder);
      }
    }
    C2D_DBG("info: hold buffer cnt %d", stream_params->frame_hold_cnt);
  }
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

  /* invalidate any remaining entries in queue corresponding to
     this identity. This will also send/update corresponding ACKs */
  C2D_HIGH("info: invalidating queue.");
  rc = c2d_module_invalidate_queue(ctrl, identity);
    if(rc < 0) {
    C2D_ERR("failed\n");
    MCT_OBJECT_UNLOCK(stream_port);
    return -EFAULT;
  }

  /* send stream_off to downstream. This blocking call ensures
     downstream modules are streamed off and no acks pending from them */
  handle_downstream_error = c2d_module_send_event_downstream(module, event);
  if(handle_downstream_error < 0) {
    C2D_ERR("failed\n");
  }
  C2D_HIGH("info: downstream stream-off done.");

  /* create c2d-event */
  c2d_module_event_t *c2d_event = (c2d_module_event_t*)
    CAM_MALLOC(sizeof(c2d_module_event_t));
  if(c2d_event) {
    c2d_event->u.streamoff_data.identity = streaminfo->identity;
    c2d_event->u.streamoff_data.stream_info = streaminfo;
    c2d_event->type = C2D_MODULE_EVENT_STREAM_OFF;
    c2d_event->hw_process_flag = FALSE;
    c2d_event->invalid = FALSE;
    c2d_priority_t prio = (streaminfo->stream_type ==
      CAM_STREAM_TYPE_OFFLINE_PROC) ? C2D_PRIORITY_OFFLINE :
      C2D_PRIORITY_REALTIME;

    /* add to queue */
    c2d_module_reorder_q_event(module, c2d_event, prio);
    /* post to thread */
    c2d_thread_msg_t msg;
    memset(&msg, 0, sizeof(c2d_thread_msg_t));
    msg.type = C2D_THREAD_MSG_NEW_EVENT_IN_Q;
    c2d_module_post_msg_to_thread(module, msg);
    /* wait for streamoff_return */
    c2d_hardware_cmd_t cmd;
    cmd.type = C2D_HW_CMD_STREAM_WAIT;
    cmd.u.streamoff_identity = streaminfo->identity;
    rc = c2d_hardware_process_command(ctrl->c2dhw, cmd);
    if (rc < 0)
      C2D_ERR("hw streamoff failed\n");
  } else {
    C2D_ERR("CAM_MALLOC() failed, module stream off failed\n");
  }

  if(handle_downstream_error < 0) {
    C2D_ERR("error from down stream modules  = %d \n",
      handle_downstream_error);
    MCT_OBJECT_UNLOCK(stream_port);
    return -EFAULT;
  }

  if ((stream_params->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) &&
    (stream_params->stream_type != CAM_STREAM_TYPE_PARM)) {
    session_params->real_time_stream_cnt--;
  }

  if (!session_params->real_time_stream_cnt) {
    PTHREAD_MUTEX_LOCK(&(session_params->c2d_per_frame_mutex));
    memset(&session_params->valid_stream_ids[0], 0,
      sizeof(session_params->valid_stream_ids));
    PTHREAD_MUTEX_UNLOCK(&(session_params->c2d_per_frame_mutex));
  }

  C2D_HIGH("info: stream-off done for identity 0x%x", identity);

  MCT_OBJECT_UNLOCK(stream_port);
  return 0;
}

/** c2d_module_process_event
 *    @module: pproc module
 *    @event: mct event to be handled
 *
 *  Handle event set on this module. As per current
 *  architecture.
 *
 *  Return: TRUE if event is handled successfully
 *          FALSE otherwise **/
boolean c2d_module_process_event(mct_module_t *module,
  mct_event_t *event)
{
  boolean              rc = TRUE;
  int32_t              ret = 0;
  mct_event_control_t *ctrl_event = NULL;

  /* Validate input parameters */
  if (!module || !event) {
    C2D_ERR("failed: module %p event %p\n", module, event);
    rc = FALSE;
    goto ERROR;
  }

  if(!MCT_OBJECT_NAME(module)) {
    C2D_ERR("failed: module name is NULL \n");
    rc = FALSE;
    goto ERROR;
  }

  if (strcmp(MCT_OBJECT_NAME(module), "c2d") != 0) {
    C2D_ERR("failed: invalid module");
    rc = FALSE;
    goto ERROR;
  }

  /* Check whether event's type is MCT_EVENT_CONTROL_CMD */
  if (event->type != MCT_EVENT_CONTROL_CMD) {
    C2D_ERR("failed invalid event type %d\n", event->type);
    rc = FALSE;
    goto ERROR;
  }

  ctrl_event = &event->u.ctrl_event;
  C2D_LOW("event %d", ctrl_event->type);
  switch (ctrl_event->type) {
  case MCT_EVENT_CONTROL_SET_PARM: {
    mct_event_control_parm_t *ctrl_parm =
      (mct_event_control_parm_t *)event->u.ctrl_event.control_event_data;
    if(!ctrl_parm) {
      C2D_ERR("failed");
      return FALSE;
    }
    c2d_module_ctrl_t *ctrl = (c2d_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
    if(!ctrl) {
      C2D_ERR("failed");
      return FALSE;
    }

    if (ctrl_parm->type == CAM_INTF_PARM_DIS_ENABLE) {
      if (!(ctrl_parm->parm_data)) {
        C2D_ERR("failed");
        return FALSE;
      }
      int32_t dis_enable =
        *(int32_t *)(ctrl_parm->parm_data);
      C2D_DBG("CAM_INTF_PARM_DIS_ENABLE, enable=%d, identity=0x%x",
        dis_enable, event->identity);
      ret = c2d_module_set_parm_dis(ctrl, event->identity, dis_enable);
      if (ret < 0) {
        C2D_ERR("failed");
        return FALSE;
      }
    }
  }
    break;
  default:
    break;
  }

ERROR:
  return rc;
}
