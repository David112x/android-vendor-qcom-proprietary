/* is_port.c
 *
 * Copyright (c) 2013-2017, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "mct_port.h"
#include "mct_stream.h"
#include "modules.h"
#include "stats_module.h"
#include "is.h"
#include "is_thread.h"
#include "is_port.h"
#include "aec.h"
#include "camera_dbg.h"
#include "c2dExt.h"
#include "stats_util.h"
#define FLOOR2(X)  ((X) & 0xFFFE)


#define IS_VIDEO_STREAM_RUNNING (private->reserved_id[IS_VIDEO] & 0xFFFF)
#define SEC_TO_USEC     (1000000L)
#define Q8              (0x00000100)

/*Forward Declarations*/
static boolean is_port_link_to_peer(mct_port_t *port, mct_event_t *event);
static boolean is_port_unlink_from_peer(mct_port_t *port, mct_event_t *event);
static boolean is_port_local_unlink(mct_port_t *port, mct_event_t *event);
static boolean is_port_remote_unlink(mct_port_t *port);
static boolean is_port_handle_role_switch(mct_port_t *port,mct_event_t *event);
static boolean is_port_set_role_switch(is_port_private_t *private);
static boolean is_port_reset_dual_cam_info(is_port_private_t *port);
static boolean is_port_if_stats_can_be_configured(is_port_private_t *port);
static boolean is_port_update_aux_sync_and_update(is_port_private_t *port);
static boolean is_is_aux_sync_enabled(is_port_private_t *port);
static boolean is_is_aux_update_enabled(is_port_private_t *port);
static boolean is_port_update_LPM(mct_port_t *port,cam_dual_camera_perf_control_t* perf_ctrl);
static boolean is_port_is_algo_active(mct_port_t *port);
static boolean is_port_intra_event(mct_port_t *port, mct_event_t *event);

/** is_port_init_thread:
 *    @port: IS port
 *
 *  Returns TRUE IS thread was successfuly started.
 **/
static boolean is_port_init_thread(mct_port_t *port)
{
  boolean rc = FALSE;
  is_port_private_t *private;

  private = (is_port_private_t *)port->port_private;
  private->thread_data = is_thread_init();
  IS_LOW("private->thread_data: %p", private->thread_data);
  if (private->thread_data != NULL) {
    private->thread_data->is_port = port;
    rc = TRUE;
  } else {
    IS_ERR("private->thread_data is NULL");
  }
  return rc;
}


/** is_port_start_thread:
 *    @port: IS port
 *
 *  Returns TRUE IS thread was successfuly started.
 **/
static boolean is_port_start_thread(mct_port_t *port)
{
  boolean rc = FALSE;
  is_port_private_t *private;

  private = (is_port_private_t *)port->port_private;
  if (private->thread_data != NULL) {
     IS_LOW("Start IS thread");
    rc = is_thread_start(private->thread_data);
    if (rc == FALSE) {
      is_thread_deinit(private->thread_data);
      private->thread_data = NULL;
    }
  } else {
    rc = FALSE;
  }
  return rc;
}


/** is_port_handle_stream_config_event:
 *    @private: private port data
 *    @mod_evt: module event
 **/
static boolean is_port_handle_stream_config_event(is_port_private_t *private,
  mct_event_module_t *mod_event)
{
  boolean rc = TRUE;
  sensor_out_info_t *sensor_info =
    (sensor_out_info_t *)mod_event->module_event_data;

   IS_HIGH("w = %u, h = %u, ma = %u, p = %d",
    sensor_info->dim_output.width, sensor_info->dim_output.height,
    sensor_info->sensor_mount_angle, sensor_info->position);

  is_thread_msg_t *msg = (is_thread_msg_t *)
    malloc(sizeof(is_thread_msg_t));

  if (msg != NULL ) {
    STATS_MEMSET(msg, 0, sizeof(is_thread_msg_t));
    msg->type = MSG_IS_SET;
    msg->u.is_set_parm.type = IS_SET_PARAM_STREAM_CONFIG;
    msg->u.is_set_parm.u.is_sensor_info.sensor_mount_angle =
      sensor_info->sensor_mount_angle;
    msg->u.is_set_parm.u.is_sensor_info.camera_position =
      sensor_info->position;
    is_thread_en_q_msg(private->thread_data, msg);
  } else {
    IS_ERR("malloc failed!");
    rc = FALSE;
  }

  return rc;
}


/** is_port_handle_set_is_enable:
 *    @private: private port data
 *    @ctrl_evt: control event
 *
 *  Returns TRUE if event was successfuly queued to the IS thread for
 *  processing.
 **/
static boolean is_port_handle_set_is_enable(is_port_private_t *private,
  mct_event_control_t *ctrl_event)
{
  boolean rc = TRUE;
  stats_set_params_type *stats_parm = ctrl_event->control_event_data;

  IS_HIGH("IS enable = %d", stats_parm->u.is_param.u.is_enable);
  is_thread_msg_t *msg = (is_thread_msg_t *)
    malloc(sizeof(is_thread_msg_t));

  if (msg != NULL ) {
    STATS_MEMSET(msg, 0, sizeof(is_thread_msg_t));
    msg->type = MSG_IS_SET;
    msg->u.is_set_parm.type = IS_SET_PARAM_IS_ENABLE;
    msg->u.is_set_parm.u.is_enable = stats_parm->u.is_param.u.is_enable;
    is_thread_en_q_msg(private->thread_data, msg);
  } else {
    IS_ERR("malloc failed!");
    rc = FALSE;
  }

  return rc;
}


/** is_port_handle_set_is_flush:
 *    @private: private port data
 *    @ctrl_evt: control event
 *
 *  Returns TRUE if event was successfuly queued to the IS thread for
 *  processing.
 **/
static boolean is_port_handle_set_is_flush(is_port_private_t *private,
  mct_event_control_t *ctrl_event)
{
  boolean rc = TRUE;

  IS_HIGH("IS flush");
  is_thread_msg_t *msg = (is_thread_msg_t *) malloc(sizeof(is_thread_msg_t));

  if (msg != NULL ) {
    STATS_MEMSET(msg, 0, sizeof(is_thread_msg_t));
    msg->type = MSG_IS_PROCESS;
    msg->u.is_process_parm.type = IS_PROCESS_FLUSH_MODE;
    msg->u.is_process_parm.u.flush_mode.is_info = &private->is_info;
    stats_set_params_type *stats_parm = ctrl_event->control_event_data;
    if (stats_parm->u.is_param.u.is_flush == TRUE) {
      msg->u.is_process_parm.u.flush_mode.flush_mode = TRUE;
    }
    else {
      IS_HIGH("Resume recording");
      msg->u.is_process_parm.u.flush_mode.flush_mode = FALSE;
    }
    is_thread_en_q_msg(private->thread_data, msg);
  } else {
    IS_ERR("malloc failed!");
    rc = FALSE;
  }

  return rc;
}


/** is_port_handle_stream_event:
 *    @port: IS port
 *    @event: event
 *
 *  Returns TRUE if event was successfuly queued to the IS thread for
 *  processing.
 **/
static boolean is_port_handle_stream_event(mct_port_t *port, mct_event_t *event)
{
  boolean rc = TRUE;
  is_port_private_t *private = port->port_private;
  mct_event_control_t *control = &event->u.ctrl_event;

  if (control->type == MCT_EVENT_CONTROL_STREAMON) {
    IS_HIGH("MCT_EVENT_CONTROL_STREAMON, identity = 0x%x", event->identity);
    if (event->identity == private->reserved_id[IS_PREVIEW] || event->identity == private->reserved_id[IS_VIDEO]) {
      mct_event_t fps_event;
      unsigned long fps;

      /* Get FPS */
      fps_event.type = MCT_EVENT_MODULE_EVENT;
      fps_event.identity = event->identity;
      fps_event.direction = MCT_EVENT_UPSTREAM;
      fps_event.u.module_event.type = MCT_EVENT_MODULE_3A_GET_CUR_FPS;
      fps_event.u.module_event.module_event_data = &fps;
      mct_port_send_event_to_peer(port, &fps_event);
      fps /= Q8;
      private->is_info.fps = fps;
    }
  } else {
    IS_HIGH("MCT_EVENT_CONTROL_STREAMOFF, identity = 0x%x",
      event->identity);
  }

  is_thread_msg_t *msg = (is_thread_msg_t *)
    malloc(sizeof(is_thread_msg_t));

  if (msg != NULL ) {
    STATS_MEMSET(msg, 0, sizeof(is_thread_msg_t));
    msg->type = MSG_IS_PROCESS;
    msg->u.is_process_parm.type = IS_PROCESS_STREAM_EVENT;
    if (event->identity == private->reserved_id[IS_VIDEO]) {
      if (control->type == MCT_EVENT_CONTROL_STREAMON) {
        msg->u.is_process_parm.u.stream_event_data.stream_event =
          IS_VIDEO_STREAM_ON;
      } else {
        msg->u.is_process_parm.u.stream_event_data.stream_event =
          IS_VIDEO_STREAM_OFF;
      }
    } else if (event->identity == private->reserved_id[IS_PREVIEW]) {
      if (control->type == MCT_EVENT_CONTROL_STREAMON) {
        msg->u.is_process_parm.u.stream_event_data.stream_event =
          IS_PREVIEW_STREAM_ON;
      } else {
        msg->u.is_process_parm.u.stream_event_data.stream_event =
          IS_PREVIEW_STREAM_OFF;
      }
    } else {
      msg->u.is_process_parm.u.stream_event_data.stream_event =
        IS_OTHER_STREAM_ON_OFF;
    }
    msg->u.is_process_parm.u.stream_event_data.is_info = &private->is_info;
    is_thread_en_q_msg(private->thread_data, msg);
  } else {
    IS_ERR("malloc failed!");
    rc = FALSE;
  }

  return rc;
}


/** is_port_handle_stats_event:
 *    @private: private port data
 *    @event: event
 *
 *  Returns TRUE if event was successfully handled.
 **/
static boolean is_port_handle_stats_event(is_port_private_t *private, mct_event_t *event)
{
  boolean rc = TRUE;
  mct_event_stats_ext_t *stats_ext_event =
    (mct_event_stats_ext_t *)event->u.module_event.module_event_data;
  mct_event_stats_isp_t *stats_event = stats_ext_event->stats_data;

  IS_LOW("stream_on[%d, %d], stats_mask = %x",
    private->is_info.stream_on[IS_VIDEO],
    private->is_info.stream_on[IS_PREVIEW], stats_event->stats_mask);

  if ((private->is_info.stream_on[IS_PREVIEW] && private->is_info.run_is[IS_PREVIEW]) ||
    (private->is_info.stream_on[IS_VIDEO] && private->is_info.run_is[IS_VIDEO])) {
    if ((stats_event->stats_mask & (1 << MSM_ISP_STATS_RS)) &&
        (stats_event->stats_mask & (1 << MSM_ISP_STATS_CS))) {
      private->RSCS_stats_ready = TRUE;
      if (!private->is_info.is_inited[IS_PREVIEW] && !private->is_info.is_inited[IS_VIDEO]) {
        private->is_info.num_row_sum = ((q3a_rs_stats_t *)
          stats_event->stats_data[MSM_ISP_STATS_RS].stats_buf)->num_v_regions;
        private->is_info.num_col_sum = ((q3a_cs_stats_t *)
          stats_event->stats_data[MSM_ISP_STATS_CS].stats_buf)->num_col_sum;
      }
    }

    if (private->RSCS_stats_ready) {
      is_thread_msg_t *msg;
      private->is_info.timestamp = stats_event->timestamp;
      msg = (is_thread_msg_t *)malloc(sizeof(is_thread_msg_t));
      if (msg != NULL) {
        STATS_MEMSET(msg, 0, sizeof(is_thread_msg_t));
        msg->type = MSG_IS_PROCESS;
        msg->u.is_process_parm.type = IS_PROCESS_RS_CS_STATS;
        msg->u.is_process_parm.u.stats_data.frame_id = stats_event->frame_id;
        msg->u.is_process_parm.u.stats_data.identity = event->identity;
        msg->u.is_process_parm.u.stats_data.is_info = &private->is_info;
        if (private->is_info.use_stats) {
          msg->u.is_process_parm.u.stats_data.yuv_rs_cs_data.p_q3a_rs_stats = \
            stats_event->stats_data[MSM_ISP_STATS_RS].stats_buf;
          msg->u.is_process_parm.u.stats_data.yuv_rs_cs_data.p_q3a_cs_stats = \
            stats_event->stats_data[MSM_ISP_STATS_CS].stats_buf;
          msg->u.is_process_parm.u.stats_data.ack_data = stats_ext_event;
          circular_stats_data_use(stats_ext_event);
        }

        rc = is_thread_en_q_msg(private->thread_data, msg);
        if (!rc && private->is_info.use_stats) {
          circular_stats_data_done(stats_ext_event, 0, 0, 0);
        }
      } else {
        IS_ERR("malloc failed!");
        rc = FALSE;
      }
      private->RSCS_stats_ready = FALSE;
    }
  }

  return rc;
}


/** is_port_handle_gyro_stats_event:
 *    @private: private port data
 *    @event: event
 *
 *  Returns TRUE if event was successfully handled.
 **/
static boolean is_port_handle_gyro_stats_event(is_port_private_t *private, mct_event_t *event)
{
  boolean rc = TRUE;
  mct_event_gyro_stats_t *gyro_stats_event =
    (mct_event_gyro_stats_t *)event->u.module_event.module_event_data;

  is_thread_msg_t *msg = (is_thread_msg_t *)
    malloc(sizeof(is_thread_msg_t));

  IS_LOW("gyro frame id = %u", gyro_stats_event->is_gyro_data.frame_id);
  if (msg != NULL ) {
    STATS_MEMSET(msg, 0, sizeof(is_thread_msg_t));
    msg->type = MSG_IS_PROCESS;
    msg->u.is_process_parm.type = IS_PROCESS_GYRO_STATS;
    msg->u.is_process_parm.u.gyro_data.frame_id =
      gyro_stats_event->is_gyro_data.frame_id;
    msg->u.is_process_parm.u.gyro_data.is_info = &private->is_info;
    STATS_MEMCPY(&msg->u.is_process_parm.u.gyro_data.gyro_data, sizeof(mct_event_gyro_data_t),
      &gyro_stats_event->is_gyro_data, sizeof(mct_event_gyro_data_t));
    is_thread_en_q_msg(private->thread_data, msg);
  } else {
    IS_ERR("malloc failed!");
    rc = FALSE;
  }

  return rc;
}


/** is_port_handle_no_gyro_data:
 *    @private: private port data
 *    @sof_id: current frame id
 **/
static boolean is_port_handle_no_gyro_data(is_port_private_t *private, unsigned int sof_id)
{
  boolean rc = TRUE;

  IS_ERR("Gyro stoppage: SOF frame id = %u, gyro frame id = %u", sof_id, private->is_info.gyro_frame_id);
  is_thread_msg_t *msg = (is_thread_msg_t *)
    malloc(sizeof(is_thread_msg_t));

  if (msg != NULL ) {
    STATS_MEMSET(msg, 0, sizeof(is_thread_msg_t));
    msg->type = MSG_IS_PROCESS;
    msg->u.is_process_parm.type = IS_PROCESS_NO_GYRO_DATA;
    msg->u.is_process_parm.u.no_gyro_data.sof_id = sof_id;
    msg->u.is_process_parm.u.no_gyro_data.is_info = &private->is_info;
    is_thread_en_q_msg(private->thread_data, msg);
  } else {
    IS_ERR("malloc failed!");
    rc = FALSE;
  }

  return rc;
}


/** is_port_handle_dis_config_event:
 *    @private: private port data
 *    @mod_evt: module event
 **/
static boolean is_port_handle_dis_config_event(is_port_private_t *private,
  mct_event_module_t *mod_event)
{
  boolean rc = TRUE;
  isp_dis_config_info_t *dis_config;
  cam_stream_type_t stream_type;

  dis_config = (isp_dis_config_info_t *)mod_event->module_event_data;

  dis_config->fps = private->is_info.fps;

  if (dis_config->stream_id == (private->reserved_id[IS_VIDEO] & 0xFFFF)) {
    stream_type = CAM_STREAM_TYPE_VIDEO;
    IS_HIGH("Set stream_type to VIDEO");
  } else if (dis_config->stream_id == (private->reserved_id[IS_PREVIEW] & 0xFFFF)) {
    stream_type = CAM_STREAM_TYPE_PREVIEW;
    IS_HIGH("Set stream_type to PREVIEW");
  } else {
    IS_HIGH("Junking MCT_EVENT_MODULE_ISP_DIS_CONFIG event");
    return FALSE;
  }

  IS_HIGH("MCT_EVENT_MODULE_ISP_DIS_CONFIG, sid = %u, strid = %x, "
    "vid res id= %x, res id= %x, col_num = %u, row_num = %u, w = %u, h = %u",
    dis_config->session_id, dis_config->stream_id, private->reserved_id[IS_VIDEO],
    private->reserved_id[IS_PREVIEW], dis_config->col_num, dis_config->row_num,
    dis_config->width, dis_config->height);

  is_thread_msg_t *msg = (is_thread_msg_t *)
    malloc(sizeof(is_thread_msg_t));

  if (msg != NULL ) {
    STATS_MEMSET(msg, 0, sizeof(is_thread_msg_t));
    msg->type = MSG_IS_SET;
    msg->u.is_set_parm.type = IS_SET_PARAM_DIS_CONFIG;
    msg->u.is_set_parm.u.is_config_info.stream_type = stream_type;
    msg->u.is_set_parm.u.is_config_info.width = dis_config->width;
    msg->u.is_set_parm.u.is_config_info.height = dis_config->height;
    is_thread_en_q_msg(private->thread_data, msg);
  } else {
    IS_ERR("malloc failed!");
    rc = FALSE;
  }

  return rc;
}


/** is_port_handle_chromatix_event
 *    @port: IS port
 *    @mod_evt: event
 */
static boolean is_port_handle_chromatix_event(mct_port_t *port, mct_event_t *event)
{
  boolean rc = TRUE;
  void *chromatix_wrapper = event->u.module_event.module_event_data;
  is_port_private_t *private = (is_port_private_t *)port->port_private;

  is_thread_msg_t *msg = malloc(sizeof(is_thread_msg_t));
  if (msg != NULL ) {
    STATS_MEMSET(msg, 0, sizeof(is_thread_msg_t));
    msg->type = MSG_IS_SET;
    msg->u.is_set_parm.type = IS_SET_PARAM_CHROMATIX;
    msg->u.is_set_parm.u.is_init_param.chromatix = chromatix_wrapper;
    msg->u.is_set_parm.u.is_init_param.port = port;
    msg->u.is_set_parm.u.is_init_param.event_id = event->identity;

    is_thread_en_q_msg(private->thread_data, msg);
  } else {
    IS_ERR("malloc failed!");
    rc = FALSE;
  }
  return rc;
}

void is_port_config_margins(is_port_private_t *private, mct_event_t *event) {

  sensor_isp_stream_sizes_t *isp_size_req =
    (sensor_isp_stream_sizes_t *)event->u.module_event.module_event_data;
  int i;
  /*Find the video index*/
  for(i =0; i < MAX_NUM_STREAMS; i++) {
    switch(isp_size_req->type[i]) {
      case CAM_STREAM_TYPE_VIDEO : {
        if(isp_size_req->is_type[i] > IS_TYPE_NONE
          && isp_size_req->is_type[i] < IS_TYPE_MAX) {
          isp_size_req->margins[i].widthMargins = private->is_info.margin;
          isp_size_req->stream_sz_plus_margin[i].width =
            (int32_t)FLOOR2((int32_t)((1.0 + private->is_info.margin)
              *((float)isp_size_req->stream_sizes[i].width) + 0.5));
          isp_size_req->margins[i].heightMargins= private->is_info.margin;
          isp_size_req->stream_sz_plus_margin[i].height =
            (int32_t)FLOOR2((int32_t)((1.0 + private->is_info.margin)
              *((float)isp_size_req->stream_sizes[i].height) + 0.5));
          IS_LOW(" Video Width- %d, Height- %d", isp_size_req->stream_sz_plus_margin[i].width,
            isp_size_req->stream_sz_plus_margin[i].height);
        }
      }
      break;
      case CAM_STREAM_TYPE_PREVIEW : {
        if(isp_size_req->is_type[i] > IS_TYPE_NONE
          && isp_size_req->is_type[i] < IS_TYPE_MAX) {
          isp_size_req->margins[i].widthMargins = private->is_info.margin;
          isp_size_req->stream_sz_plus_margin[i].width =
            (int32_t)FLOOR2((int32_t)((1.0 + private->is_info.margin)
              *((float)isp_size_req->stream_sizes[i].width) + 0.5));
          isp_size_req->margins[i].heightMargins= private->is_info.margin;
          isp_size_req->stream_sz_plus_margin[i].height =
            (int32_t)FLOOR2((int32_t)((1.0 + private->is_info.margin)
              *((float)isp_size_req->stream_sizes[i].height) + 0.5));
          IS_LOW("Preview Width- %d, Height- %d", isp_size_req->stream_sz_plus_margin[i].width,
            isp_size_req->stream_sz_plus_margin[i].height);
        }
      }
      break;
      default : {
        IS_LOW("Margins not needed for Stream Type %d", isp_size_req->type[i]);
      }
      break;
    }
  }
}

/** is_port_send_is_event:
 *    @port: IS port
 *    @private: private port data
 *    @stream_type: 0 (preview), 1 (video)
 **/
static void is_port_send_is_event(mct_port_t *port, is_port_private_t *private,
  is_output_type *is_output, int stream_type)
{
  mct_event_t is_update_event;
  is_update_t is_update;
  int i;

  is_update.id = private->reserved_id[stream_type];
  is_update.x = is_output->x;
  is_update.y = is_output->y;
  is_update.width = private->is_info.width[stream_type];
  is_update.height = private->is_info.height[stream_type];
  is_update.frame_id = is_output->frame_id;
  if (private->is_info.is_type[stream_type] == IS_TYPE_EIS_3_0) {
    is_update.use_3d = 1;
    is_update.num_matrices = private->is_info.num_mesh_y + 1;
    STATS_MEMCPY(is_update.transform_matrix, sizeof(is_update.transform_matrix),
      is_output->transform_matrix, (private->is_info.num_mesh_y + 1) * sizeof(float) * NUM_MATRIX_ELEMENTS);
    is_update.transform_type = 0;
  } else if (private->is_info.is_type[stream_type] == IS_TYPE_EIS_2_0) {
    is_update.use_3d = 1;
    is_update.num_matrices = 1;
    STATS_MEMCPY(is_update.transform_matrix, sizeof(is_update.transform_matrix),
      is_output->transform_matrix, sizeof(float) * NUM_MATRIX_ELEMENTS);
    is_update.transform_type = private->is_info.transform_type;
  } else {
    is_update.use_3d = 0;
    is_update.transform_type = 0;
  }

  IS_LOW("eid = 0x%x, fid = %u, x = %d, y = %d, w = %d, h = %d",
    is_update.id, is_update.frame_id, is_update.x, is_update.y, is_update.width,
    is_update.height);
  if (is_update.use_3d) {
    for (i = 0; i < is_update.num_matrices; i++) {
      IS_LOW("tt = %u, tm = %f %f %f %f %f %f %f %f %f", is_update.transform_type,
        is_update.transform_matrix[i*NUM_MATRIX_ELEMENTS + 0],
        is_update.transform_matrix[i*NUM_MATRIX_ELEMENTS + 1],
        is_update.transform_matrix[i*NUM_MATRIX_ELEMENTS + 2],
        is_update.transform_matrix[i*NUM_MATRIX_ELEMENTS + 3],
        is_update.transform_matrix[i*NUM_MATRIX_ELEMENTS + 4],
        is_update.transform_matrix[i*NUM_MATRIX_ELEMENTS + 5],
        is_update.transform_matrix[i*NUM_MATRIX_ELEMENTS + 6],
        is_update.transform_matrix[i*NUM_MATRIX_ELEMENTS + 7],
        is_update.transform_matrix[i*NUM_MATRIX_ELEMENTS + 8]);
    }
  }

  is_update_event.type = MCT_EVENT_MODULE_EVENT;
  is_update_event.identity = is_update.id;
  is_update_event.direction = MCT_EVENT_UPSTREAM;
  is_update_event.u.module_event.type = MCT_EVENT_MODULE_STATS_DIS_UPDATE;
  is_update_event.u.module_event.module_event_data = &is_update;
  mct_port_send_event_to_peer(port, &is_update_event);
}


/** is_port_handle_output_dim_event:
 *    @private: private port data
 *    @mod_evt: module event
 **/
static boolean is_port_handle_output_dim_event(is_port_private_t *private,
  mct_event_module_t *mod_event)
{
  boolean rc = TRUE;
  mct_stream_info_t *stream_info = NULL;
  int stream_type;

  stream_info = (mct_stream_info_t *)mod_event->module_event_data;
  IS_HIGH("MCT_EVENT_MODULE_ISP_OUTPUT_DIM, steam_type = %d, w = %d, "
    "h = %d, IS mode = %d", stream_info->stream_type,
    stream_info->dim.width, stream_info->dim.height, stream_info->is_type);

  if (stream_info->stream_type == CAM_STREAM_TYPE_VIDEO) {
    private->is_info.vfe_width[IS_VIDEO] = stream_info->dim.width;
    private->is_info.vfe_height[IS_VIDEO] = stream_info->dim.height;
    if (stream_info->is_type != IS_TYPE_NONE &&
      stream_info->is_type != IS_TYPE_CROP) {
      private->is_info.run_is[IS_VIDEO] = TRUE;
    }
  } else if (stream_info->stream_type == CAM_STREAM_TYPE_PREVIEW) {
    private->is_info.vfe_width[IS_PREVIEW] = stream_info->dim.width;
    private->is_info.vfe_height[IS_PREVIEW] = stream_info->dim.height;
    if (stream_info->is_type != IS_TYPE_NONE &&
      stream_info->is_type != IS_TYPE_CROP) {
      private->is_info.run_is[IS_PREVIEW] = TRUE;
    }
  } else {
    rc = FALSE;
  }

  return rc;
}


/** is_port_send_bus_msg:
 *    @port: IS port
 *    @msg_type: message type
 *    @msg: message
 *    @size: size of message
 **/
static void is_port_send_bus_msg(mct_port_t *port, mct_bus_msg_type_t msg_type, void *msg, int msg_size)
{
  is_port_private_t *private = port->port_private;
  mct_event_t event;
  mct_bus_msg_t bus_msg;
  boolean rc = TRUE;

  STATS_MEMSET(&bus_msg, 0, sizeof(mct_bus_msg_t));
  bus_msg.sessionid = (private->reserved_id[IS_PREVIEW] >> 16);
  bus_msg.type = msg_type;
  bus_msg.msg = msg;
  bus_msg.size = msg_size;

  /* Pack into an mct_event object*/
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = private->reserved_id[IS_PREVIEW];
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_POST_TO_BUS;
  event.u.module_event.module_event_data = (void *)(&bus_msg);
  event.u.module_event.current_frame_id = private->is_info.gyro_frame_id;

  rc = mct_port_send_event_to_peer(port, &event);
  if (rc == FALSE) {
    IS_ERR("Error sending bus event");
  }
}


/** is_port_send_is_update:
 *    @port: IS port
 *    @private: private port data
 **/
static void is_port_send_is_update(mct_port_t *port, is_port_private_t *private)
{
  int i;

  for (i = 0; i < IS_MAX_STREAMS; i++) {
    /* Sanity check */
    /* is_enabled is reset to 0 when IS initialization fails.  By checking this
       flag for 0, IS won't send DIS_UPDATE event when it is not operational. */
    if (private->is_output[i].x < 0 || private->is_output[i].y < 0 ||
      private->is_info.is_enabled[i] == 0) {
      continue;
    }

    /* Send DIS_UPDATE if IS has output */
    if (private->is_output[i].has_output) {
      is_port_send_is_event(port, private, &private->is_output[i], i);
    }
  }
}


/** is_port_send_lpm_update:
 *    @port: IS port
 *    @request_frame: Indicates whether or not to requst more frames
 *
 *  This function sends bus message to enable/disable LPM.
 **/
static void is_port_send_lpm_update(mct_port_t *port, boolean request_frame)
{
  mct_bus_msg_ctrl_request_frame_t ctrl_request_frame;

  STATS_MEMSET(&ctrl_request_frame, 0, sizeof(mct_bus_msg_ctrl_request_frame_t));
  ctrl_request_frame.lpm_req_bit = 0;
  ctrl_request_frame.lpm_req_bit |= LPM_REQUEST_BIT_STATS_IS;
  ctrl_request_frame.request_flag = request_frame;
  if (request_frame) {
    ctrl_request_frame.req_mode = FRM_REQ_MODE_CONTINUOUS;
    IS_HIGH("Vote wake-up for session 0x%x",
      ((is_port_private_t *)port->port_private)->reserved_id[IS_PREVIEW] & 0xFFFF0000);
  } else {
    /* Allow secondary camera to suspend */
    ctrl_request_frame.req_mode = FRM_REQ_MODE_NONE;
    IS_HIGH("Vote suspend for session 0x%x",
      ((is_port_private_t *)port->port_private)->reserved_id[IS_PREVIEW] & 0xFFFF0000);
  }
  is_port_send_bus_msg(port, MCT_BUS_MSG_CONTROL_REQUEST_FRAME, &ctrl_request_frame,
    sizeof(mct_bus_msg_ctrl_request_frame_t));
}


static void is_port_stats_done_callback(void *port, void *stats)
{
  if (!port || !stats) {
    return;
  }

  is_port_private_t *private = ((mct_port_t *)port)->port_private;
  is_stats_data_t   *is_stats = (is_stats_data_t *)stats;

  if (!private || !is_stats) {
    return;
  }

  IS_LOW("DONE IS STATS ACK back");

  circular_stats_data_done(is_stats->ack_data, port,
    is_stats->identity, is_stats->frame_id);
}


/** is_port_callback:
 *    @port: IS port
 *    @output: Output from processing IS event
 **/
static void is_port_set_callback(mct_port_t *port, is_set_output_t *output)
{
  switch (output->type) {
  case IS_SET_OUTPUT_ROLE_SWITCH:
    is_port_send_lpm_update(port, output->request_frame);
    break;

  default: ;
  }
}


/** is_port_process_callback:
 *    @port: IS port
 *    @output: Output from processing IS event
 **/
static void is_port_process_callback(mct_port_t *port, is_process_output_t *output)
{
  is_port_private_t *private = port->port_private;

  IS_LOW("IS process ouput type = %d", output->type);
  switch (output->type) {
  case IS_PROCESS_OUTPUT_RS_CS_STATS:
    is_port_send_is_update(port, private);
    break;

  case IS_PROCESS_OUTPUT_GYRO_STATS:
    is_port_send_is_update(port, private);
    if (private->is_info.sof_countdown > 0) {
      private->is_info.sof_countdown--;
      if (private->is_info.sof_countdown == 0) {
        is_port_send_lpm_update(port, FALSE);
      }
    }
    break;

  case IS_PROCESS_OUTPUT_FLUSH_MODE: {
    int i;

    IS_HIGH("FLUSH_MODE: num output = %d", output->num_output);
    for (i = 0; i < output->num_output; i++) {
      is_port_send_is_event(port, port->port_private,
        &private->is_output_flush[i], IS_VIDEO);
    }
    if (output->num_output > 0) {
      private->is_output[IS_VIDEO].frame_id = private->is_output_flush[output->num_output - 1].frame_id;
    }
    output->num_output = 0;
  }
    break;

  case IS_PROCESS_OUTPUT_STREAM_EVENT: {
    int stream_type = IS_PREVIEW;

    switch (output->is_stream_event) {
    case IS_VIDEO_STREAM_ON:
      private->is_info.stream_on[IS_VIDEO] = TRUE;
      stream_type = IS_VIDEO;
      break;

    case IS_PREVIEW_STREAM_ON:
      private->is_info.stream_on[IS_PREVIEW] = TRUE;
      stream_type = IS_PREVIEW;
      break;

    case IS_VIDEO_STREAM_OFF: {
      int i;

      IS_HIGH("STREAM_OFF: num output = %d", output->num_output);
      for (i = 0; i < output->num_output; i++) {
        is_port_send_is_event(port, port->port_private,
          &private->is_output_flush[i], IS_VIDEO);
      }
      output->num_output = 0;
      private->is_info.stream_on[IS_VIDEO] = FALSE;
      private->is_output[IS_VIDEO].has_output = 0;
      stream_type = IS_VIDEO;
      is_port_send_lpm_update(port, FALSE);
    }
      break;

    case IS_PREVIEW_STREAM_OFF:
      private->is_info.stream_on[IS_PREVIEW] = FALSE;
      private->is_output[IS_PREVIEW].has_output = 0;
      stream_type = IS_PREVIEW;
      is_port_send_lpm_update(port, FALSE);
      break;

    default: ;
    }

    if (output->is_stream_event == IS_VIDEO_STREAM_ON ||
      output->is_stream_event == IS_PREVIEW_STREAM_ON) {
      int i;

      /* Default offsets to half margin for cropping at center during camcorder
         preview no recording. */
      if (private->is_info.is_type[stream_type] != IS_TYPE_EIS_2_0 &&
        private->is_info.is_type[stream_type] != IS_TYPE_EIS_3_0) {
        private->is_output[stream_type].x =
          (private->is_info.vfe_width[stream_type] -
           private->is_info.width[stream_type]) / 2;
        private->is_output[stream_type].y =
          (private->is_info.vfe_height[stream_type] -
           private->is_info.height[stream_type]) / 2;
      } else {
        private->is_output[stream_type].x = 0;
        private->is_output[stream_type].y = 0;
      }

      /* For now, front/rear camera has same virtual margin */
      for (i = 0; i < MAX_IS_MESH_Y + 1; i++) {
        private->is_output[stream_type].transform_matrix[i*NUM_MATRIX_ELEMENTS + 0] = 1.0 /
         (1 + 2 * private->is_info.is_chromatix_info.virtual_margin);
        private->is_output[stream_type].transform_matrix[i*NUM_MATRIX_ELEMENTS + 1] = 0.0;
        private->is_output[stream_type].transform_matrix[i*NUM_MATRIX_ELEMENTS + 2] = 0.0;
        private->is_output[stream_type].transform_matrix[i*NUM_MATRIX_ELEMENTS + 3] = 0.0;
        private->is_output[stream_type].transform_matrix[i*NUM_MATRIX_ELEMENTS + 4] = 1.0 /
          (1 + 2 * private->is_info.is_chromatix_info.virtual_margin);
        private->is_output[stream_type].transform_matrix[i*NUM_MATRIX_ELEMENTS + 5] = 0.0;
        private->is_output[stream_type].transform_matrix[i*NUM_MATRIX_ELEMENTS + 6] = 0.0;
        private->is_output[stream_type].transform_matrix[i*NUM_MATRIX_ELEMENTS + 7] = 0.0;
        private->is_output[stream_type].transform_matrix[i*NUM_MATRIX_ELEMENTS + 8] = 1.0;
      }

      if (private->is_info.is_type[stream_type] == IS_TYPE_CROP) {
        is_port_send_is_event(port, private, &private->is_output[stream_type], stream_type);
      }
    }
  }
    break;

  case IS_PROCESS_OUTPUT_NO_GYRO_DATA:
    IS_HIGH("IS_PROCESS_OUTPUT_NO_GYRO_DATA");
    is_port_send_is_update(port, private);
    break;

  default:
    break;
  }
}


/** is_port_event:
 *    @port: IS port
 *    @event: event
 *
 *  This function handles events for the IS port.
 *
 *  Returns TRUE on successful event processing.
 **/
static boolean is_port_event(mct_port_t *port, mct_event_t *event)
{
  boolean rc = TRUE;
  is_port_private_t *private;

  /* sanity check */
  if (!port || !event)
    return FALSE;

  private = (is_port_private_t *)(port->port_private);
  if (!private)
    return FALSE;

  /* sanity check: ensure event is meant for port with same identity*/
  if ((private->reserved_id[IS_PREVIEW] & 0xFFFF0000) !=
      (event->identity & 0xFFFF0000))
  {
    return FALSE;
  }

  switch (MCT_EVENT_DIRECTION(event)) {
  case MCT_EVENT_DOWNSTREAM: {
    switch (event->type) {
    case MCT_EVENT_CONTROL_CMD: {
      mct_event_control_t *control = &event->u.ctrl_event;

      // IS_LOW("Control event type %d", control->type);
      switch (control->type) {
      case MCT_EVENT_CONTROL_SOF: {
        mct_bus_msg_isp_sof_t *sof_event =
         (mct_bus_msg_isp_sof_t *)event->u.module_event.module_event_data;
        is_info_t *is_info = &private->is_info;

        if (private->thread_data &&
            (is_info->is_inited[IS_VIDEO] && is_info->is_type[IS_VIDEO] == IS_TYPE_EIS_3_0) &&
            (IS_VIDEO_STREAM_RUNNING)) {
          if (is_info->initial_sof_id == 0) {
            is_info->initial_sof_id = sof_event->frame_id;
            IS_HIGH("Initial SOF = %u", sof_event->frame_id);
          }
          if (sof_event->frame_id > is_info->gyro_frame_id + 5) {
            /* EIS 3.0 is running and gyro stoppage occurred */
            rc = is_port_handle_no_gyro_data(private, sof_event->frame_id);
          }
        }
      }
        break;

      case MCT_EVENT_CONTROL_STREAMON:
        if (private->thread_data) {
          rc = is_port_handle_stream_event(port, event);
        }
        break;

      case MCT_EVENT_CONTROL_STREAMOFF:
        is_port_unlink_from_peer(port, event);
        if (private->thread_data) {
          rc = is_port_handle_stream_event(port, event);
        }
        break;

      case MCT_EVENT_CONTROL_MASTER_INFO:
        if (private->thread_data) {
          is_port_handle_role_switch(port, event);
        }
        break;

      case MCT_EVENT_CONTROL_LINK_INTRA_SESSION:
        if (private->thread_data) {
          is_port_link_to_peer(port, event);
        }
        break;

      case MCT_EVENT_CONTROL_UNLINK_INTRA_SESSION: {
        is_port_unlink_from_peer(port, event);
      }
        break;

      case MCT_EVENT_CONTROL_SET_PARM: {
        stats_set_params_type *stats_parm = control->control_event_data;
        if (private->thread_data) {
          if (stats_parm->param_type == STATS_SET_IS_PARAM) {
            if (stats_parm->u.is_param.type == IS_SET_PARAM_IS_ENABLE) {
              rc = is_port_handle_set_is_enable(private, control);
            } else if (stats_parm->u.is_param.type == IS_SET_PARAM_IS_FLUSH) {
              rc = is_port_handle_set_is_flush(private, control);
            }
          } else if (stats_parm->param_type == STATS_SET_COMMON_PARAM &&
              stats_parm->u.common_param.type == COMMON_SET_PARAM_STREAM_ON_OFF) {
            stats_common_set_parameter_t *common_param =
              &(stats_parm->u.common_param);

            IS_LOW("COMMON_SET_PARAM_STREAM_ON_OFF %d", common_param->u.stream_on);
            private->thread_data->no_stats_mode = !common_param->u.stream_on;

            /* stream off, need to flush existing stats */
            /* send a sync msg here to flush the stats & other msg */
            if (!common_param->u.stream_on) {
              is_thread_msg_t is_msg;
              STATS_MEMSET(&is_msg, 0, sizeof(is_thread_msg_t));
              is_msg.type = MSG_IS_STATS_MODE;
              is_msg.sync_flag = TRUE;
              is_thread_en_q_msg(private->thread_data, &is_msg);
              IS_LOW("COMMON_SET_PARAM_STREAM_ON_OFF end");
            }
          }
        }
      }
        break;

      default:
        break;
      }
    } /* case MCT_EVENT_CONTROL_CMD */
      break;

    case MCT_EVENT_MODULE_EVENT: {
      mct_event_module_t *mod_event = &event->u.module_event;

      switch (mod_event->type) {
      case MCT_EVENT_MODULE_STATS_EXT_DATA:
        if (private->is_info.use_stats &&
          (private->is_info.is_enabled[IS_PREVIEW] || private->is_info.is_enabled[IS_VIDEO]) &&
          (IS_VIDEO_STREAM_RUNNING)) {
          rc = is_port_handle_stats_event(private, event);
        }
        break;

      case MCT_EVENT_MODULE_STATS_GYRO_STATS:
        MCT_OBJECT_LOCK(port);
        if (private->thread_data &&
          ((private->is_info.is_inited[IS_VIDEO] && private->is_info.is_type[IS_VIDEO] != IS_TYPE_DIS && private->is_info.is_type[IS_VIDEO] != IS_TYPE_VENDOR_EIS) ||
           (private->is_info.is_inited[IS_PREVIEW] && private->is_info.is_type[IS_PREVIEW] != IS_TYPE_DIS && private->is_info.is_type[IS_PREVIEW] != IS_TYPE_VENDOR_EIS)) &&
          (IS_VIDEO_STREAM_RUNNING)) {
          rc = is_port_handle_gyro_stats_event(private, event);
        }
        MCT_OBJECT_UNLOCK(port);
        break;

      case MCT_EVENT_MODULE_SET_STREAM_CONFIG:
        if (private->thread_data) {
          rc = is_port_handle_stream_config_event(private, mod_event);
        }
        break;

      case MCT_EVENT_MODULE_ISP_DIS_CONFIG: {
        if (private->thread_data) {
          rc = is_port_handle_dis_config_event(private, mod_event);
        }
      }
        break;

      case MCT_EVENT_MODULE_SENSOR_QUERY_OUTPUT_SIZE: {
        IS_LOW("Configure the margins needed for ISP o/p");
        is_port_config_margins(private, event);
      }
        break;

      case MCT_EVENT_MODULE_IS_CONFIG: {
        mct_event_is_config_t *is_config = mod_event->module_event_data;

        is_config->margin_value = private->is_info.margin;
        is_config->num_buffers = private->is_info.buffer_delay;
        is_config->num_mesh_x = private->is_info.num_mesh_x;
        is_config->num_mesh_y = private->is_info.num_mesh_y;
        IS_HIGH("MCT_EVENT_MODULE_IS_CONFIG, m = %f, nb = %u, "
          "nmx = %d, nmy = %u", is_config->margin_value, is_config->num_buffers,
          is_config->num_mesh_x, is_config->num_mesh_y);
      }
        break;

      case MCT_EVENT_MODULE_MODE_CHANGE: {
        private->stream_type = ((stats_mode_change_event_data *)
          (mod_event->module_event_data))->stream_type;
      }
        break;

      case MCT_EVENT_MODULE_START_STOP_STATS_THREADS: {
        uint8_t *start_flag = (uint8_t*)(mod_event->module_event_data);
        IS_LOW("MCT_EVENT_MODULE_START_STOP_STATS_THREADS start_flag: %d",
          *start_flag);
        if (*start_flag) {
          if (is_port_start_thread(port) == FALSE) {
            IS_ERR("is thread start failed");
            rc = FALSE;
          }
        } else {
          if (private->thread_data) {
            is_thread_stop(private->thread_data);
          }
        }
      }
        break;

      case MCT_EVENT_MODULE_ISP_OUTPUT_DIM: {
        if (private->thread_data) {
          rc = is_port_handle_output_dim_event(private, mod_event);
        }
      }
        break;

      case MCT_EVENT_MODULE_SET_CHROMATIX_WRP: {
        if (private->thread_data) {
          rc = is_port_handle_chromatix_event(port, event);
        }
      }
        break;

      case MCT_EVENT_MODULE_REQUEST_STATS_TYPE: {
        mct_event_request_stats_type *stats_info =
          (mct_event_request_stats_type *)mod_event->module_event_data;

        if (ISP_STREAMING_OFFLINE == stats_info->isp_streaming_type) {
          IS_HIGH("IS doesn't support offline processing yet. Returning.");
          break;
        } else if ( private->dual_cam_sensor_info == CAM_TYPE_AUX) {
          break;
        }
        IS_HIGH(" Enable stats mask only when IS is enabled, cur status: %d",
          private->is_info.is_enabled[IS_PREVIEW]);
        /*Opt: Enable IS stats only when IS is enabled*/
        if (private->is_info.is_enabled[IS_PREVIEW] && private->is_info.use_stats) {
          if (stats_info->supported_stats_mask & (1 << MSM_ISP_STATS_RS)) {
            stats_info->enable_stats_mask |= (1 << MSM_ISP_STATS_RS);
            stats_info->enable_stats_parse_mask |= (1 << MSM_ISP_STATS_RS);
            stats_info->stats_comp_grp_mask[STATS_GROUP_0] |= (1 << MSM_ISP_STATS_RS);
          }
          if (stats_info->supported_stats_mask & (1 << MSM_ISP_STATS_CS)) {
            stats_info->enable_stats_mask |= (1 << MSM_ISP_STATS_CS);
            stats_info->enable_stats_parse_mask |= (1 << MSM_ISP_STATS_CS);
            stats_info->stats_comp_grp_mask[STATS_GROUP_0] |= (1 << MSM_ISP_STATS_CS);
          }
        }
      }
        break;

      default:
        break;
      }
    } /* case MCT_EVENT_MODULE_EVENT */
      break;

    default:
      break;
    } /* switch (event->type) */

  } /* case MCT_EVENT_TYPE_DOWNSTREAM */
    break;

  case MCT_EVENT_UPSTREAM: {
    mct_port_t *peer = MCT_PORT_PEER(port);
    MCT_PORT_EVENT_FUNC(peer)(peer, event);
  }
    break;

  default:
    break;
  } /* switch (MCT_EVENT_DIRECTION(event)) */

  return rc;
}


/** is_port_ext_link:
 *    @identity: session id | stream id
 *    @port: IS port
 *    @peer: For IS sink port, peer is most likely stats port
 *
 *  Sets IS port's external peer port.
 *
 *  Returns TRUE on success.
 **/
static boolean is_port_ext_link(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  boolean rc = FALSE, thread_init = FALSE;
  is_port_private_t *private;
  mct_event_t event;

   IS_LOW("Enter");
  if (strcmp(MCT_OBJECT_NAME(port), "is_sink"))
    return FALSE;

  private = (is_port_private_t *)port->port_private;
  if (!private)
    return FALSE;

  MCT_OBJECT_LOCK(port);
  switch (private->state) {
  case IS_PORT_STATE_RESERVED:
     IS_LOW("IS_PORT_STATE_RESERVED");
    if ((private->reserved_id[IS_PREVIEW] & 0xFFFF0000) != (identity & 0xFFFF0000)) {
      break;
    }
  /* Fall through */
  case IS_PORT_STATE_UNLINKED:
     IS_LOW("IS_PORT_STATE_UNLINKED");
    if ((private->reserved_id[IS_PREVIEW] & 0xFFFF0000) != (identity & 0xFFFF0000)) {
      break;
    }

  case IS_PORT_STATE_CREATED:
    if ((private->reserved_id[IS_PREVIEW] & 0xFFFF0000) == (identity & 0xFFFF0000)) {
      thread_init = TRUE;
    }
    rc = TRUE;
    break;

  case IS_PORT_STATE_LINKED:
    IS_LOW("IS_PORT_STATE_LINKED");
    if ((private->reserved_id[IS_PREVIEW] & 0xFFFF0000) == (identity & 0xFFFF0000)) {
      rc = TRUE;
    }
    break;

  default:
    break;
  }

  if (rc == TRUE) {
    /* If IS module requires a thread and the port state above warrants one,
       create the thread here */
    if (thread_init == TRUE) {
      if (private->thread_data == NULL) {
        rc = FALSE;
        goto init_thread_fail;
      }
    }
    private->state = IS_PORT_STATE_LINKED;
    MCT_PORT_PEER(port) = peer;
    MCT_OBJECT_REFCOUNT(port) += 1;
  }

init_thread_fail:
  MCT_OBJECT_UNLOCK(port);
  mct_port_add_child(identity, port);
  IS_LOW("rc=%d", rc);
  return rc;
}


/** is_port_unlink:
 *  @identity: session id | stream id
 *  @port: IS port
 *  @peer: IS port's peer port (probably stats port)
 *
 *  This funtion unlinks the IS port from its peer.
 **/
static void is_port_unlink(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  is_port_private_t *private;

  if (!port || !peer || MCT_PORT_PEER(port) != peer)
    return;

  private = (is_port_private_t *)port->port_private;
  if (!private)
    return;

  IS_LOW("port state = %d, identity = 0x%x", private->state, identity);
  MCT_OBJECT_LOCK(port);
  if (private->state == IS_PORT_STATE_LINKED &&
      (private->reserved_id[IS_PREVIEW] & 0xFFFF0000) == (identity & 0xFFFF0000)) {
    MCT_OBJECT_REFCOUNT(port) -= 1;
    if (!MCT_OBJECT_REFCOUNT(port)) {
      private->state = IS_PORT_STATE_UNLINKED;
      IS_LOW("Stop IS thread, video reserved id = %x", private->reserved_id[IS_VIDEO]);
    }
  }
  MCT_OBJECT_UNLOCK(port);
  mct_port_remove_child(identity, port);

  return;
}


/** is_port_set_caps:
 *    @port: port object whose caps are to be set
 *    @caps: this port's capability.
 *
 *  Function overwrites a ports capability.
 *
 *  Returns TRUE if it is valid source port.
 **/
static boolean is_port_set_caps(mct_port_t *port,
  mct_port_caps_t *caps)
{
  IS_LOW("Enter");
  if (strcmp(MCT_PORT_NAME(port), "is_sink"))
    return FALSE;

  port->caps = *caps;
  return TRUE;
}


/** is_port_check_caps_reserve:
 *    @port: this interface module's port
 *    @peer_caps: the capability of peer port which wants to match
 *                interface port
 *    @stream_info: stream information
 *
 *  Returns TRUE on success.
 **/
static boolean is_port_check_caps_reserve(mct_port_t *port, void *caps,
  void *stream_info)
{
  boolean rc = FALSE;
  mct_port_caps_t *port_caps;
  is_port_private_t *private;
  mct_stream_info_t *strm_info = (mct_stream_info_t *)stream_info;

  IS_LOW("Enter");

  if (!port || !caps || !strm_info ||
      strcmp(MCT_OBJECT_NAME(port), "is_sink")) {
    IS_LOW("Exit unsucessful");
    return rc;
  }

  MCT_OBJECT_LOCK(port);

  port_caps = (mct_port_caps_t *)caps;
  if (port_caps->port_caps_type != MCT_PORT_CAPS_STATS) {
    rc = FALSE;
    goto reserve_done;
  }

  private = (is_port_private_t *)port->port_private;
  IS_LOW("port state = %d, identity = 0x%x, stream_type = %d",
    private->state, strm_info->identity, strm_info->stream_type);
  switch (private->state) {
  case IS_PORT_STATE_LINKED:
  if ((private->reserved_id[IS_PREVIEW] & 0xFFFF0000) ==
      (strm_info->identity & 0xFFFF0000)) {
    if (strm_info->stream_type == CAM_STREAM_TYPE_VIDEO) {
      private->reserved_id[IS_VIDEO] = strm_info->identity;
      private->is_info.is_type[IS_VIDEO] = strm_info->is_type;
      IS_HIGH("video id = 0x%x, is_type = %d, w = %d, h = %d",
        private->reserved_id[IS_VIDEO], strm_info->is_type, strm_info->dim.width,
        strm_info->dim.height);
      if ((strm_info->is_type == IS_TYPE_DIS) ||
          (strm_info->is_type == IS_TYPE_EIS_2_0 &&
           private->is_info.dis_bias_correction)) {
        private->is_info.use_stats |= IS_VIDEO_USE_STATS_MASK;
      }
    } else if (strm_info->stream_type == CAM_STREAM_TYPE_PREVIEW) {
      private->reserved_id[IS_PREVIEW] = strm_info->identity;
      private->is_info.is_type[IS_PREVIEW] = strm_info->is_type;
      IS_HIGH("preview id = 0x%x, is_type = %d, w = %d, h = %d",
        private->reserved_id[IS_PREVIEW], strm_info->is_type, strm_info->dim.width,
        strm_info->dim.height);
      if ((strm_info->is_type == IS_TYPE_DIS) ||
          (strm_info->is_type == IS_TYPE_EIS_2_0 &&
           private->is_info.dis_bias_correction)) {
        private->is_info.use_stats |= IS_PREVIEW_USE_STATS_MASK;
      }
    } else if (strm_info->stream_type == CAM_STREAM_TYPE_SNAPSHOT) {
      IS_LOW("snapshot id = 0x%x", strm_info->identity);
    }

    rc = TRUE;
  }
  break;

  case IS_PORT_STATE_CREATED:
  case IS_PORT_STATE_UNRESERVED:
    if (strm_info->stream_type == CAM_STREAM_TYPE_VIDEO) {
      private->reserved_id[IS_VIDEO] = strm_info->identity;
      IS_LOW("video id = 0x%x", private->reserved_id[IS_VIDEO]);
      IS_LOW("w = %d, h = %d", strm_info->dim.width, strm_info->dim.height);
    }
    private->reserved_id[IS_PREVIEW] = strm_info->identity;
    private->stream_type = strm_info->stream_type;
    private->state = IS_PORT_STATE_RESERVED;
    rc = TRUE;
    break;

  case IS_PORT_STATE_RESERVED:
    if ((private->reserved_id[IS_PREVIEW] & 0xFFFF0000) ==
        (strm_info->identity & 0xFFFF0000))
      rc = TRUE;
    break;

  default:
    break;
  }

reserve_done:
  MCT_OBJECT_UNLOCK(port);
  return rc;
}


/** is_port_check_caps_unreserve:
 *    @port: this port object to remove the session/stream
 *    @identity: session+stream identity
 *
 *  This function frees the identity from port's children list.
 *
 *  Returns FALSE if the identity does not exist.
 **/
static boolean is_port_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  boolean rc = FALSE;
  is_port_private_t *private;

  if (!port || strcmp(MCT_OBJECT_NAME(port), "is_sink"))
    return FALSE;

  IS_LOW("E, identity = 0x%x", identity);
  private = (is_port_private_t *)port->port_private;
  if (!private)
    return FALSE;

  IS_LOW("port state = %d, identity = 0x%x", private->state, identity);
  if (private->state == IS_PORT_STATE_UNRESERVED)
    return TRUE;

  MCT_OBJECT_LOCK(port);
  if (private->state == IS_PORT_STATE_LINKED &&
    private->reserved_id[IS_VIDEO] == identity) {
    private->reserved_id[IS_VIDEO] = (private->reserved_id[IS_VIDEO] & 0xFFFF0000);
    IS_HIGH("Reset video reserved id to 0x%x", private->reserved_id[IS_VIDEO]);
  }

  if ((private->state == IS_PORT_STATE_UNLINKED ||
       private->state == IS_PORT_STATE_RESERVED) &&
      ((private->reserved_id[IS_PREVIEW] & 0xFFFF0000) == (identity & 0xFFFF0000))) {

    if (!MCT_OBJECT_REFCOUNT(port)) {
      private->state = IS_PORT_STATE_UNRESERVED;
      private->reserved_id[IS_VIDEO] = (private->reserved_id[IS_VIDEO] & 0xFFFF0000);
      private->reserved_id[IS_PREVIEW] = (private->reserved_id[IS_PREVIEW] & 0xFFFF0000);
    }
    rc = TRUE;
  }

unreserve_done:
  MCT_OBJECT_UNLOCK(port);
  return rc;
}


/** is_port_init:
 *    @port: IS port
 *    @session_id: session id
 *
 *  This function initializes the IS port's internal variables.
 *
 *  Returns TRUE on success.
 **/
boolean is_port_init(mct_port_t *port, unsigned int session_id)
{
  mct_port_caps_t caps;
  is_port_private_t *private;

  if (port == NULL || strcmp(MCT_OBJECT_NAME(port), "is_sink"))
    return FALSE;

  private = (void *)malloc(sizeof(is_port_private_t));
  if (!private)
    return FALSE;

  STATS_MEMSET(private, 0, sizeof(is_port_private_t));
  private->set_parameters = is_set_parameters;
  private->process = is_process;
  private->set_callback = is_port_set_callback;
  private->process_callback = is_port_process_callback;
  private->is_stats_cb = is_port_stats_done_callback;
  private->is_process_output.is_output = private->is_output;
  private->is_process_output.is_output_flush = private->is_output_flush;
  private->reserved_id[IS_VIDEO] = session_id;
  private->reserved_id[IS_PREVIEW] = session_id;
  private->state = IS_PORT_STATE_CREATED;
  private->dual_cam_sensor_info = CAM_TYPE_STANDALONE;
  private->is_info.flush_mode = FALSE;

  private->is_info.transform_type = C2D_LENSCORRECT_PERSPECTIVE |
    C2D_LENSCORRECT_BILINEAR | C2D_LENSCORRECT_ORIGIN_IN_MIDDLE |
    C2D_LENSCORRECT_SOURCE_RECT;

  /* Explicitly disable DIS bias correction for EIS 2.0 for clarity */
  private->is_info.dis_bias_correction = 0;

  IS_PROP_GET_MARGIN(private->is_info.margin);
  IS_PROP_GET_BUFFER_DELAY(private->is_info.buffer_delay);
  IS_PROP_GET_NUM_MESH_X(private->is_info.num_mesh_x);
  IS_PROP_GET_NUM_MESH_Y(private->is_info.num_mesh_y);

  /* Cap the buffer delay to some maximum */
  private->is_info.buffer_delay = MIN(private->is_info.buffer_delay, IS_MAX_BUFFER_DELAY);

  port->port_private = private;
  port->direction = MCT_PORT_SINK;
  caps.port_caps_type = MCT_PORT_CAPS_STATS;
  caps.u.stats.flag   = MCT_PORT_CAP_STATS_CS_RS;

  is_port_init_thread(port);

  mct_port_set_event_func(port, is_port_event);
  mct_port_set_intra_event_func(port, is_port_intra_event);
  /* Accept default int_link function */
  mct_port_set_ext_link_func(port, is_port_ext_link);
  mct_port_set_unlink_func(port, is_port_unlink);
  mct_port_set_set_caps_func(port, is_port_set_caps);
  mct_port_set_check_caps_reserve_func(port, is_port_check_caps_reserve);
  mct_port_set_check_caps_unreserve_func(port, is_port_check_caps_unreserve);

  if (port->set_caps) {
    port->set_caps(port, &caps);
  }
  return TRUE;
}


/** is_port_deinit:
 *    @port: IS port
 *
 * This function frees the IS port's memory.
 **/
void is_port_deinit(mct_port_t *port)
{
  is_port_private_t *private;
  if (!port || strcmp(MCT_OBJECT_NAME(port), "is_sink"))
    return;

  private = port->port_private;

  if (private != NULL) {
    is_thread_deinit(private->thread_data);
    free(port->port_private);
  }
}


/** is_port_find_identity:
 *    @port: IS port
 *    @identity: session id | stream id
 *
 * This function checks for the port with a given session.
 *
 * Returns TRUE if the port is found.
 **/
boolean is_port_find_identity(mct_port_t *port, unsigned int identity)
{
  is_port_private_t *private;

  if (!port) {
    return FALSE;
  }

  if (strcmp(MCT_OBJECT_NAME(port), "is_sink")) {
    return FALSE;
  }

  private = port->port_private;
  if (private) {
    return ((private->reserved_id[IS_PREVIEW] & 0xFFFF0000) ==
            (identity & 0xFFFF0000) ? TRUE : FALSE);
  }

  return FALSE;
}

/** is_port_link_to_peer:
 *    @port: private IS port data
 *    @event: module event received
 *
 * Links to peer IS Port
 *
 * Return boolean
 **/
static boolean is_port_link_to_peer(mct_port_t *port, mct_event_t *event)
{
  boolean                                rc = FALSE;
  mct_port_t                            *this_port = port;
  mct_port_t                            *peer_port = NULL;
  cam_sync_related_sensors_event_info_t *link_param = NULL;
  uint32_t                               peer_identity = 0;
  is_port_private_t *private = (is_port_private_t *)(port->port_private);

  link_param = (cam_sync_related_sensors_event_info_t *)
    (event->u.ctrl_event.control_event_data);
  peer_identity = link_param->related_sensor_session_id;

  rc = stats_util_get_peer_port(event, peer_identity, this_port, &peer_port);

  if (rc == FALSE) {
    IS_ERR("FAIL to Get Peer Port");
    return rc;
  }
  private->dual_cam_info.cam_role = link_param->cam_role;
  private->dual_cam_info.mode = link_param->mode;
  private->dual_cam_info.sync_3a_mode = link_param->sync_3a_config.sync_mode_stats;
  private->dual_cam_info.is_LPM_on = FALSE;
  private->dual_cam_info.intra_peer_id = peer_identity;
  is_port_handle_role_switch(port, NULL);
  MCT_PORT_INTRALINKFUNC(this_port)(peer_identity, this_port, peer_port);
  IS_HIGH("ISDualCam-[mode -%d role-%d 3aSync-%d AuxSync-%d AuxUpdate-%d] Linking to session %x Success",
    private->dual_cam_info.mode,
    private->dual_cam_info.cam_role,
    private->dual_cam_info.sync_3a_mode,
    private->dual_cam_info.is_aux_sync_enabled,
    private->dual_cam_info.is_aux_update_enabled,
    peer_identity);
  return rc;
}

/** is_port_unlink_from_peer:
 *    @port: MCT port data
 *    @event: module event received
 *
 * UnLinks from peer IS Port (Master/Slave)
 *
 * Return boolean
 **/
static boolean is_port_unlink_from_peer(mct_port_t *port, mct_event_t *event)
{
  is_port_private_t  *private = (is_port_private_t *)(port->port_private);
  if (private->dual_cam_info.intra_peer_id) {
    IS_HIGH("ISDualCam-[mode -%d role-%d] unlink Started",
      private->dual_cam_info.mode, private->dual_cam_info.cam_role);
    is_port_local_unlink(port, event);
    is_port_remote_unlink(port);
    IS_HIGH("ISDualCam-[mode -%d role-%d] Unlink complete",
      private->dual_cam_info.mode, private->dual_cam_info.cam_role);
  }

  return TRUE;
}

/** is_port_local_unlink:
 *    @port: MCT port data
 *    @event: module event received
 *
 * UnLinks from peer IS Port locally (Master/Slave)
 *
 * Return boolean
 **/
static boolean is_port_local_unlink(mct_port_t *port, mct_event_t *event)
{
  boolean                                rc = FALSE;
  mct_port_t                            *this_port = port;
  mct_port_t                            *peer_port = NULL;
  cam_sync_related_sensors_event_info_t *link_param = NULL;
  is_port_private_t                     *private = (is_port_private_t *)(port->port_private);
  uint32_t                               peer_identity = private->dual_cam_info.intra_peer_id;

  if (peer_identity) {
    rc = stats_util_get_peer_port(event, peer_identity, this_port, &peer_port);
    if (rc == TRUE) {
      MCT_PORT_INTRAUNLINKFUNC(peer_port);
      is_port_reset_dual_cam_info(private);
    }
    IS_HIGH("ISDualCam-[mode -%d role-%d] Unlinking from peer session",
      private->dual_cam_info.mode, private->dual_cam_info.cam_role);
  } else {
    rc = TRUE;
  }
  return rc;
}

/** is_port_remote_unlink:
 *    @port: MCT port data
 *    @event: module event received
 *
 * Asks Peer to do unlink (Master/Slave)
 *
 * Return boolean
 **/
static boolean is_port_remote_unlink(mct_port_t *port)
{
  boolean                                rc = FALSE;
  mct_port_t                            *this_port = port;
  mct_port_t                            *peer_port = NULL;
  cam_sync_related_sensors_event_info_t *link_param = NULL;
  uint32_t                               peer_identity = 0;
  is_port_private_t  *private = (is_port_private_t *)(port->port_private);
  /* Forward the IS update info to the slave session */
  IS_HIGH("ISDualCam-[mode -%d role-%d] Remote Unlink issued",
    private->dual_cam_info.mode,
    private->dual_cam_info.cam_role);
  rc = stats_util_post_intramode_event(port,
                                       private->dual_cam_info.intra_peer_id,
                                       MCT_EVENT_MODULE_STATS_PEER_UNLINK,
                                       NULL);
  return rc;
}

/** is_port_handle_role_switch:
 *    @port: MCT port data
 *    @event: module event
 *
 * Handles Role switch from Master to Slave and Vice versa (Master/Slave)
 *
 * Return boolean
 **/
static boolean is_port_handle_role_switch(mct_port_t *port, mct_event_t *event)
{
  mct_port_t *this_port = port;
  is_port_private_t *private = (is_port_private_t *)(port->port_private);
  if (event) {
    cam_dual_camera_master_info_t *dual_master_info = (cam_dual_camera_master_info_t *)
      (event->u.ctrl_event.control_event_data);
    if (dual_master_info){
      private->dual_cam_info.mode = dual_master_info->mode;
    }
  }

  is_port_update_aux_sync_and_update(private);
  IS_HIGH("ISDualCam-[mode -%d role-%d] Role Switch",
    private->dual_cam_info.mode, private->dual_cam_info.cam_role);
  if (event) {
    is_port_set_role_switch(private);
  }
  return TRUE;
}

/** is_port_set_role_switch
 *    @private:   Private data of the port
 *
 * Return: TRUE if no error
 **/
static boolean is_port_set_role_switch(is_port_private_t *private)
{
  boolean rc = TRUE;

  is_thread_msg_t *msg = (is_thread_msg_t *)malloc(sizeof(is_thread_msg_t));

  if (msg != NULL) {
    STATS_MEMSET(msg, 0, sizeof(is_thread_msg_t));
    msg->type = MSG_IS_SET;
    msg->u.is_set_parm.type = IS_SET_PARAM_ROLE_SWITCH;
    msg->u.is_set_parm.u.is_role_switch_mode = private->dual_cam_info.mode;
    is_thread_en_q_msg(private->thread_data, msg);
  } else {
    IS_ERR("malloc failed!");
    return FALSE;
  }

  IS_HIGH("ISDualCam-[mode -%d role-%d] Role Switch Master=%d",
    private->dual_cam_info.mode,
    private->dual_cam_info.cam_role,
    msg->u.is_set_parm.u.is_role_switch_mode);
  return rc;
}

/** is_port_reset_dual_cam_info:
 *    @port: IS Private  port data
 *
 * Resets Dual Cam Info (Master/Slave)
 *
 * Return boolean
 **/
static boolean is_port_reset_dual_cam_info(is_port_private_t *port)
{
  IS_HIGH("ISDualCam-[mode -%d role-%d] reset",
    port->dual_cam_info.mode, port->dual_cam_info.cam_role);
  port->dual_cam_info.mode = CAM_MODE_PRIMARY;
  port->dual_cam_info.sync_3a_mode = CAM_3A_SYNC_NONE;
  port->dual_cam_info.is_LPM_on = FALSE;
  port->dual_cam_info.is_aux_sync_enabled = FALSE;
  port->dual_cam_info.is_aux_update_enabled = FALSE;
  port->dual_cam_info.intra_peer_id = 0;
  return TRUE;
}

/** is_port_if_stats_can_be_configured:
 *    @port: IS Private  port data
 *
 * Checks if Stats can be configured (Master/Slave)
 *
 * Return boolean
 **/
static boolean is_port_if_stats_can_be_configured(is_port_private_t *port)
{
  boolean config = TRUE;

  /*We dont want to configure stats if we are in Follow and Mono mode*/
  if ((port->dual_cam_info.sync_3a_mode == CAM_3A_SYNC_FOLLOW) &&
      (port->dual_cam_info.mode == CAM_MODE_SECONDARY) &&
      (port->dual_cam_info.cam_role == CAM_ROLE_MONO)) {
    IS_HIGH("ISDualCam-[mode -%d role-%d] Stats cannot be configured",
      port->dual_cam_info.mode,
      port->dual_cam_info.cam_role);
    config = FALSE;
  }
  return config;

}

/** is_port_update_aux_sync_and_update:
 *    @port: IS Private  port data
 *
 * Updates Aux sync and update conditions (Master/Slave)
 *
 * Return boolean
 **/
static boolean is_port_update_aux_sync_and_update(is_port_private_t *port)
{
  boolean is_aux_sync_enabled = FALSE;
  boolean is_aux_update_enabled = FALSE;
  boolean is_algo_active = TRUE;

  if(port->dual_cam_info.is_LPM_on) {
    port->dual_cam_info.is_aux_sync_enabled = FALSE;
    port->dual_cam_info.is_aux_update_enabled = FALSE;
    return TRUE;
  }

  if ((port->dual_cam_info.is_LPM_on == FALSE)
    && (port->dual_cam_info.sync_3a_mode == CAM_3A_SYNC_FOLLOW)
    && (port->dual_cam_info.mode == CAM_MODE_PRIMARY)
    && (port->dual_cam_info.intra_peer_id != 0)
    && (port->dual_cam_info.cam_role != CAM_ROLE_BAYER)) {
    is_aux_sync_enabled = TRUE;
  }

  if ((port->dual_cam_info.is_LPM_on == FALSE)
      &&(port->dual_cam_info.sync_3a_mode == CAM_3A_SYNC_FOLLOW)
      && (port->dual_cam_info.mode == CAM_MODE_SECONDARY)
      && (port->dual_cam_info.intra_peer_id != 0)
      && (port->dual_cam_info.cam_role != CAM_ROLE_MONO)) {
    is_aux_update_enabled = TRUE;
  }

  if((port->dual_cam_info.sync_3a_mode == CAM_3A_SYNC_FOLLOW)
      && (port->dual_cam_info.mode == CAM_MODE_SECONDARY)) {
    is_algo_active = FALSE;
  }

  if(is_algo_active != port->dual_cam_info.is_algo_active) {
    IS_HIGH("ISDualCam-[mode -%d role-%d] is_algo_active=%d",
      port->dual_cam_info.mode,
      port->dual_cam_info.cam_role,
      is_algo_active);
    port->dual_cam_info.is_algo_active = is_algo_active;
  }

  if(is_aux_sync_enabled != port->dual_cam_info.is_aux_sync_enabled) {
    IS_HIGH("ISDualCam-[mode -%d role-%d] Aux Sync Enabled=%d",
      port->dual_cam_info.mode,
      port->dual_cam_info.cam_role,
      is_aux_sync_enabled);
    port->dual_cam_info.is_aux_sync_enabled = is_aux_sync_enabled;
  }

  if(is_aux_update_enabled != port->dual_cam_info.is_aux_update_enabled) {
    IS_HIGH("ISDualCam-[mode -%d role-%d] Aux Update Enabled=%d",
      port->dual_cam_info.mode,
      port->dual_cam_info.cam_role,
      is_aux_update_enabled);
    port->dual_cam_info.is_aux_update_enabled = is_aux_update_enabled;
  }
  return TRUE;
}

/** is_is_aux_sync_enabled:
 *    @port: port data
 *
 * Returns if Syncing to Aux Camera is allowed
 *
 * Return boolean
 **/
static boolean is_is_aux_sync_enabled(is_port_private_t *port)
{
  return port->dual_cam_info.is_aux_sync_enabled;
}

/** is_is_aux_update_enabled:
 *    @port: port data
 *
 * Returns if Update from aux Camera is allowed
 *
 * Return boolean
 **/
static boolean is_is_aux_update_enabled(is_port_private_t *port)
{
  return port->dual_cam_info.is_aux_update_enabled;
}

/** is_port_update_LPM:
 *    @port: IS Private port data
 *    @perf_cntrl: performance control data
 *
 * Update LPM mode  (Master/Slave)
 *
 * Return boolean
 **/
static boolean is_port_update_LPM(mct_port_t *port, cam_dual_camera_perf_control_t* perf_ctrl)
{
  boolean rc = FALSE;
  is_port_private_t *private = (is_port_private_t *)(port->port_private);

  is_thread_msg_t *msg = malloc(sizeof(is_thread_msg_t));

  if (NULL != msg) {
    STATS_MEMSET(msg, 0, sizeof(is_thread_msg_t));
    msg->type = MSG_IS_SET;
    msg->u.is_set_parm.type = IS_SET_PARAM_LOW_POWER_MODE;
    msg->u.is_set_parm.u.is_low_power_mode.enable = perf_ctrl->enable;
    msg->u.is_set_parm.u.is_low_power_mode.priority = perf_ctrl->priority;
    msg->u.is_set_parm.u.is_low_power_mode.perf_mode = perf_ctrl->perf_mode;
    is_thread_en_q_msg(private->thread_data, msg);
  } else {
    IS_ERR("malloc failed!");
    return FALSE;
  }

  if(private->dual_cam_info.is_LPM_on != perf_ctrl->enable) {
    private->dual_cam_info.is_LPM_on = perf_ctrl->enable;
    IS_HIGH("ISDualCam-[mode -%d role-%d] LPM Updated=%d",
      private->dual_cam_info.mode,
      private->dual_cam_info.cam_role,
      perf_ctrl->enable);
    is_port_update_aux_sync_and_update(private);
  }
  return rc;
}

/** is_port_is_algo_active:
 *    @port: IS port
 *
 * Is Algorithm Active (Master/Slave)
 *
 * Return boolean
 **/
static boolean is_port_is_algo_active(mct_port_t *port)
{
  is_port_private_t *private = (is_port_private_t *)(port->port_private);
  return private->dual_cam_info.is_algo_active;
}

/** is_port_intra_event:
 *    @port: IS port
 *    @event: module event
 *
 * Handles the intra-module events sent between IS master and slave sessions
 *
 * Return TRUE if the event is processed successfully.
 **/
static boolean is_port_intra_event(mct_port_t *port, mct_event_t *event)
{
  is_port_private_t *private;

  /* sanity check */
  if (!port || !event) {
    return FALSE;
  }

  private = (is_port_private_t *)(port->port_private);
  if (!private) {
    return FALSE;
  }

  IS_LOW("ISDualCam-[mode -%d role-%d] Received IS intra-module peer event",
    private->dual_cam_info.mode, private->dual_cam_info.cam_role);

  /* sanity check: ensure event is meant for port with same identity*/
  int i = 0;
  for (i = 0; i < IS_MAX_STREAMS; i++) {
    if ((private->reserved_id[i] & 0xFFFF0000) == (event->identity & 0xFFFF0000)) {
      break;
    }
  }
  if (i >= IS_MAX_STREAMS) {
    return FALSE;
  }


  /* check if there's need for extended handling. */
  if (private->func_tbl.ext_handle_intra_event) {
    stats_ext_return_type ret;
    IS_LOW("Handle extended intra port event!");
    ret = private->func_tbl.ext_handle_intra_event(port, event);
    /* Check if this event has been completely handled. If not we'll process it further here. */
    if (STATS_EXT_HANDLING_COMPLETE == ret) {
      IS_LOW("Intra port event %d handled by extended functionality!",
        event->u.module_event.type);
      return TRUE;
    }
  }

  switch(event->u.module_event.type) {
    case MCT_EVENT_MODULE_STATS_PEER_UPDATE_EVENT:
      IS_LOW("ISDualCam-[mode -%d role-%d] Received MCT_EVENT_MODULE_STATS_PEER_UPDATE_EVENT",
        private->dual_cam_info.mode,
        private->dual_cam_info.cam_role);
      break;
    case MCT_EVENT_MODULE_STATS_PEER_STATUS_EVENT:
      IS_LOW("ISDualCam-[mode -%d role-%d] Received MCT_EVENT_MODULE_STATS_PEER_STATUS_EVENT",
        private->dual_cam_info.mode,
        private->dual_cam_info.cam_role);
      break;
    case MCT_EVENT_MODULE_STATS_PEER_UNLINK:
      IS_LOW("ISDualCam-[mode -%d role-%d] Received MCT_EVENT_MODULE_STATS_PEER_UNLINK",
        private->dual_cam_info.mode,
        private->dual_cam_info.cam_role);
      is_port_local_unlink(port,event);
      is_port_reset_dual_cam_info(private);
      break;
    default:
      IS_ERR("ISDualCam-[mode -%d role-%d] Error! Received unknown intra-module event type: %d",
        private->dual_cam_info.mode,
        private->dual_cam_info.cam_role,
        event->u.module_event.type);
      break;
  }

  return TRUE;
}

/*End of IS Dual Camera Port handling*/

