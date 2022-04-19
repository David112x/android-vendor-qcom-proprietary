/*============================================================================

  Copyright (c) 2013-2018 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "c2d_module.h"
#include "c2d_port.h"
#include "c2d_log.h"
#include "c2d_module_events.h"
#include "pp_buf_mgr.h"
#include <cutils/properties.h>

#define C2D_PORT_NAME_LEN     32
#define C2D_NUM_SINK_PORTS    8
#define C2D_NUM_SOURCE_PORTS  8
#define C2D_CLIENT_ID         30
volatile int32_t gcam_c2d_loglevel = 0;
volatile int32_t gcam_pp_feat_mask = 0;
static const char c2d_sink_port_name[C2D_NUM_SINK_PORTS][C2D_PORT_NAME_LEN] = {
 "c2d_sink_0",
 "c2d_sink_1",
 "c2d_sink_2",
 "c2d_sink_3",
 "c2d_sink_4",
 "c2d_sink_5",
 "c2d_sink_6",
 "c2d_sink_7",
};

static const char c2d_src_port_name[C2D_NUM_SOURCE_PORTS][C2D_PORT_NAME_LEN] = {
 "c2d_src_0",
 "c2d_src_1",
 "c2d_src_2",
 "c2d_src_3",
 "c2d_src_4",
 "c2d_src_5",
 "c2d_src_6",
 "c2d_src_7",
};
static boolean c2d_module_set_session_data(mct_module_t *module,
  void *set_buf, uint32_t sessionid);
/** c2d_module_init:
 *  Args:
 *    @name: module name
 *  Return:
 *    - mct_module_t pointer corresponding to c2d on SUCCESS
 *    - NULL in case of FAILURE or if C2D hardware does not
 *      exist
 **/
mct_module_t *c2d_module_init(const char *name)
{
  mct_module_t *module;
  c2d_module_ctrl_t* ctrl;
  char prop[PROPERTY_VALUE_MAX];
  int val = 0;
  C2D_HIGH("name=%s", name);
  property_get("persist.vendor.camera.c2d.disable", prop, "0");
  val = atoi(prop);
  if (val) {
    C2D_HIGH("Disabling C2D module");
    return NULL;
  }
  module = mct_module_create(name);
  if(!module) {
    C2D_ERR("failed.");
    return NULL;
  }
  ctrl = c2d_module_create_c2d_ctrl();
  if(!ctrl) {
    C2D_ERR("failed");
    goto error_cleanup_module;
  }

  MCT_OBJECT_PRIVATE(module) = ctrl;
  ctrl->p_module = module;

  ctrl->c2d = pproc_library_init();
  if (!ctrl->c2d) {
    C2D_ERR("pproc_library_init() failed");
    return FALSE;
  }

  /* Open c2d instance */
  if((ctrl->c2d->func_tbl->open(&ctrl->c2d_ctrl) < 0)) {
    C2D_ERR("ctrl->c2d->func_tbl->open() failed");
    return FALSE;
  }

  module->set_mod = c2d_module_set_mod;
  module->query_mod = c2d_module_query_mod;
  module->start_session = c2d_module_start_session;
  module->stop_session = c2d_module_stop_session;
  module->process_event = c2d_module_process_event;
  module->set_session_data = c2d_module_set_session_data;

  mct_port_t* port;
  uint32_t i;
  /* Create default ports */
  for(i=0; i < C2D_NUM_SOURCE_PORTS; i++) {
    port = c2d_port_create(c2d_src_port_name[i], MCT_PORT_SRC);
    if(!port) {
      C2D_ERR("failed.");
      goto error_cleanup_module;
    }
    module->srcports = mct_list_append(module->srcports, port, NULL, NULL);
    MCT_PORT_PARENT(port) = mct_list_append(MCT_PORT_PARENT(port), module,
                              NULL, NULL);
  }
  for(i=0; i < C2D_NUM_SINK_PORTS; i++) {
    port = c2d_port_create(c2d_sink_port_name[i], MCT_PORT_SINK);
    if(!port) {
      C2D_ERR("failed.");
      goto error_cleanup_module;
    }
    module->sinkports = mct_list_append(module->sinkports, port, NULL, NULL);
    MCT_PORT_PARENT(port) = mct_list_append(MCT_PORT_PARENT(port), module,
                              NULL, NULL);
  }
  C2D_HIGH("info: C2D module_init successful");
  return module;

error_cleanup_module:
  mct_module_destroy(module);
  return NULL;
}

/** c2d_module_loglevel:
 *
 *  Args:
 *  Return:
 *    void
 **/
static void c2d_module_loglevel()
{
  char c2d_prop[PROPERTY_VALUE_MAX];
  memset(c2d_prop, 0, sizeof(c2d_prop));
  property_get("persist.vendor.camera.c2d.debug.mask", c2d_prop, "1");
  gcam_c2d_loglevel = atoi(c2d_prop);
  char c2d_feat_mask[PROPERTY_VALUE_MAX];
  memset(c2d_feat_mask, 0, sizeof(c2d_feat_mask));
  property_get("persist.vendor.cam.pp.feat.mask", c2d_feat_mask, "0");
  gcam_pp_feat_mask = atoi(c2d_feat_mask);
}

/** c2d_module_deinit:
 *
 *  Args:
 *    @module: pointer to c2d mct module
 *  Return:
 *    void
 **/
void c2d_module_deinit(mct_module_t *module)
{
  c2d_module_ctrl_t *ctrl =
    (c2d_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);

 /* Close c2d instance */
  if (ctrl->c2d->func_tbl->close(ctrl->c2d_ctrl) < 0)
    C2D_ERR("ctrl->c2d->func_tbl->close() failed");

  c2d_module_destroy_c2d_ctrl(ctrl);
  /* TODO: free other dynamically allocated resources in module */
  mct_module_destroy(module);
}

static c2d_module_ctrl_t* c2d_module_create_c2d_ctrl(void)
{
  c2d_module_ctrl_t *ctrl = NULL;
  mct_queue_t *q;
  int32_t rc;
  ctrl = (c2d_module_ctrl_t *) CAM_MALLOC(sizeof(c2d_module_ctrl_t));
  if(!ctrl) {
    C2D_ERR("CAM_MALLOC failed");
    return NULL;
  }
  memset(ctrl, 0x00, sizeof(c2d_module_ctrl_t));

  /* create real-time queue */
  ctrl->realtime_queue.q = (mct_queue_t*) CAM_MALLOC(sizeof(mct_queue_t));
  if(!ctrl->realtime_queue.q) {
    C2D_ERR("CAM_MALLOC failed");
    goto error_queue;
  }
  memset(ctrl->realtime_queue.q, 0x00, sizeof(mct_queue_t));
  mct_queue_init(ctrl->realtime_queue.q);
  pthread_mutex_init(&(ctrl->realtime_queue.mutex), NULL);

  /* create offline queue*/
  ctrl->offline_queue.q = (mct_queue_t*) CAM_MALLOC(sizeof(mct_queue_t));
  if(!ctrl->offline_queue.q) {
    C2D_ERR("CAM_MALLOC failed");
    goto error_queue;
  }
  memset(ctrl->offline_queue.q, 0x00, sizeof(mct_queue_t));
  mct_queue_init(ctrl->offline_queue.q);
  pthread_mutex_init(&(ctrl->offline_queue.mutex), NULL);

  /* create ack list */
  ctrl->ack_list.list = NULL;
  ctrl->ack_list.size = 0;
  pthread_mutex_init(&(ctrl->ack_list.mutex), NULL);

  /* Create PIPE for communication with c2d_thread */
  rc = pipe(ctrl->pfd);
  if(rc < 0) {
    C2D_ERR("pipe() failed");
    goto error_pipe;
  }
  pthread_cond_init(&(ctrl->th_start_cond), NULL);
  ctrl->session_count = 0;

  /* initialize cpp_mutex */
  pthread_mutex_init(&(ctrl->c2d_mutex), NULL);
  /* Create the C2D hardware instance */
  ctrl->c2dhw = c2d_hardware_create();
  if(ctrl->c2dhw == NULL) {
    C2D_ERR("failed, cannnot create c2d hardware instance\n");
    goto error_hw;
  }

  c2d_hardware_cmd_t cmd;
  cmd.type = C2D_HW_CMD_GET_CAPABILITIES;
  rc = c2d_hardware_process_command(ctrl->c2dhw, cmd);
  if(rc < 0) {
    C2D_ERR("C2D_HW_CMD_GET_CAPABILITIES: failed\n");
    goto destroy_hw;
  }

  return ctrl;

destroy_hw:
  c2d_hardware_destroy(ctrl->c2dhw);
error_hw:
  close(ctrl->pfd[READ_FD]);
  close(ctrl->pfd[WRITE_FD]);
error_pipe:
  free(ctrl->realtime_queue.q);
  free(ctrl->offline_queue.q);
error_queue:
  free(ctrl);
  return NULL;
}

static int32_t c2d_module_destroy_c2d_ctrl(c2d_module_ctrl_t *ctrl)
{
  if(!ctrl) {
    return 0;
  }
  /* TODO: remove all entries from queues */
  mct_queue_free(ctrl->realtime_queue.q);
  mct_queue_free(ctrl->offline_queue.q);
  pthread_mutex_destroy(&(ctrl->realtime_queue.mutex));
  pthread_mutex_destroy(&(ctrl->offline_queue.mutex));
  pthread_mutex_destroy(&(ctrl->ack_list.mutex));
  pthread_mutex_destroy(&(ctrl->c2d_mutex));
  pthread_cond_destroy(&(ctrl->th_start_cond));
  close(ctrl->pfd[READ_FD]);
  close(ctrl->pfd[WRITE_FD]);
  c2d_hardware_destroy(ctrl->c2dhw);
  free(ctrl);
  return 0;
}

void c2d_module_set_mod(mct_module_t *module, mct_module_type_t module_type,
  uint32_t identity)
{
  C2D_DBG("module_type=%d\n", module_type);
  if(!module) {
    C2D_ERR("failed");
    return;
  }
  if (mct_module_find_type(module, identity) != MCT_MODULE_FLAG_INVALID) {
    mct_module_remove_type(module, identity);
  }
  mct_module_add_type(module, module_type, identity);
}

boolean c2d_module_query_mod(mct_module_t *module, void *buf,
  uint32_t sessionid __unused)
{
  int32_t rc;
  uint32_t padding = 0;
  if(!module || !buf) {
    C2D_ERR("failed, module=%p, query_buf=%p", module, buf);
    return FALSE;
  }
  mct_pipeline_cap_t *query_buf = (mct_pipeline_cap_t *)buf;
  mct_pipeline_pp_cap_t *pp_cap = &(query_buf->pp_cap);

  c2d_module_ctrl_t *ctrl = (c2d_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);

  /* TODO: Need a linking function to fill pp cap based on HW caps? */
  pp_cap->width_padding =  mct_util_calculate_lcm(pp_cap->width_padding,
    ctrl->c2dhw->caps.buf_align_caps.width_padding);
  pp_cap->height_padding = mct_util_calculate_lcm(pp_cap->height_padding,
    ctrl->c2dhw->caps.buf_align_caps.height_padding);
  pp_cap->plane_padding = mct_util_calculate_lcm(pp_cap->plane_padding,
    ctrl->c2dhw->caps.buf_align_caps.plane_padding);
  query_buf->common_cap.buf_alignment =
    mct_util_calculate_lcm(query_buf->common_cap.buf_alignment,
    pp_cap->width_padding);

  pp_cap->min_num_pp_bufs += MODULE_C2D_MIN_NUM_PP_BUFS;
  pp_cap->feature_mask |= (CAM_QCOM_FEATURE_CROP | CAM_QCOM_FEATURE_FLIP|
    CAM_QCOM_FEATURE_SCALE);

  return TRUE;
}

boolean c2d_module_start_session(mct_module_t *module, uint32_t sessionid)
{
  int32_t rc;
  char value[PROPERTY_VALUE_MAX];
  int32_t enabled = 0;

  c2d_module_loglevel(); //dynamic logging level
  C2D_HIGH("info: starting session %d", sessionid);
  if(!module) {
    C2D_ERR("failed");
    return FALSE;
  }
  c2d_module_ctrl_t *ctrl = (c2d_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    C2D_ERR("failed");
    return FALSE;
  }
  if(ctrl->session_count >= C2D_MODULE_MAX_SESSIONS) {
    C2D_ERR("failed, too many sessions, count=%d", ctrl->session_count);
    return FALSE;
  }

  /* create a new session specific params structure */
  uint32_t i;
  for(i=0; i < C2D_MODULE_MAX_SESSIONS; i++) {
    if(ctrl->session_params[i] == NULL) {
      ctrl->session_params[i] =
        (c2d_module_session_params_t*)
           CAM_MALLOC(sizeof(c2d_module_session_params_t));
      memset(ctrl->session_params[i], 0x00,
        sizeof(c2d_module_session_params_t));
      ctrl->session_params[i]->session_id = sessionid;
      ctrl->session_params[i]->fps_range.max_fps = 30.0f;
      ctrl->session_params[i]->fps_range.min_fps = 30.0f;
      ctrl->session_params[i]->fps_range.video_max_fps = 30.0f;
      ctrl->session_params[i]->fps_range.video_min_fps = 30.0f;
      ctrl->session_params[i]->ion_fd = -1;
      pthread_mutex_init(&(ctrl->session_params[i]->c2d_per_frame_mutex), NULL);
      property_get("persist.vendor.camera.pproc.debug.en", value, "0");
      enabled = atoi(value);
      if (enabled)
         ctrl->session_params[i]->c2d_debug_enable = TRUE;
      break;
    }
  }

  /* start the thread only when first session starts */
  if(ctrl->session_count == 0) {
    /* spawn the c2d thread */
    rc = c2d_thread_create(module);
    if(rc < 0) {
      C2D_ERR("c2d_thread_create() failed");
      return FALSE;
    }
    C2D_HIGH("info: c2d_thread created.");
    /* Create buffer manager instance */
    ctrl->buf_mgr = pp_buf_mgr_open();
    if (!ctrl->buf_mgr) {
      C2D_ERR("pp_buf_mgr_open() failed");
      return FALSE;
    }

    /* init native buf mgr */
    if (!pp_native_buf_mgr_init(&ctrl->native_buf_mgr, C2D_CLIENT_ID)) {
      C2D_ERR("failed");
      return FALSE;
    }
  }
  ctrl->session_count++;
  C2D_HIGH("info: session %d started.", sessionid);
  return TRUE;
}

boolean c2d_module_stop_session(mct_module_t *module, uint32_t sessionid)
{
  int32_t rc;
  if(!module) {
    C2D_ERR("failed");
    return FALSE;
  }
  c2d_module_ctrl_t *ctrl = (c2d_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    C2D_ERR("failed");
    return FALSE;
  }
  C2D_HIGH("info: stopping session %d ...", sessionid);
  ctrl->session_count--;
  /* stop the thread only when last session terminates */
  if(ctrl->session_count == 0) {
    /* stop the C2D thread */
    C2D_DBG("info: stopping c2d_thread...");
    c2d_thread_msg_t msg;
    msg.type = C2D_THREAD_MSG_ABORT;
    rc = c2d_module_post_msg_to_thread(module, msg);
    if(rc < 0) {
      C2D_ERR("c2d_module_post_msg_to_thread() failed");
      return FALSE;
    }
    /* wait for thread completion */
    pthread_join(ctrl->c2d_thread, NULL);
    /* close the c2d hardware */
    C2D_DBG("closing c2d subdev...");
    /* Close buffer manager instance */
    pp_buf_mgr_close(ctrl->buf_mgr);
    pp_native_buf_mgr_deinit(&ctrl->native_buf_mgr);
  }
  /* remove the session specific params */
  uint32_t i;
  for(i=0; i < C2D_MODULE_MAX_SESSIONS; i++) {
    if(ctrl->session_params[i]) {
      if(ctrl->session_params[i]->session_id == sessionid) {
        if (ctrl->session_params[i]->ion_fd != -1) {
          close(ctrl->session_params[i]->ion_fd);
        }
        PTHREAD_MUTEX_LOCK(&(ctrl->session_params[i]->c2d_per_frame_mutex));
        memset(&ctrl->session_params[i]->valid_stream_ids[0], 0,
          sizeof(&ctrl->session_params[i]->valid_stream_ids));
        PTHREAD_MUTEX_UNLOCK(&(ctrl->session_params[i]->c2d_per_frame_mutex));
        pthread_mutex_destroy(&(ctrl->session_params[i]->c2d_per_frame_mutex));
        free(ctrl->session_params[i]);
        ctrl->session_params[i] = NULL;
        break;
      }
    }
  }
  C2D_HIGH("info: session %d stopped.", sessionid);
  return TRUE;
}

/* c2d_module_post_msg_to_thread:
 *
 * @module: c2d module pointer
 * @msg: message to be posted for thread
 * Description:
 *  Writes message to the pipe for which the c2d_thread is listening to.
 *
 **/
int32_t c2d_module_post_msg_to_thread(mct_module_t *module,
  c2d_thread_msg_t msg)
{
  int32_t rc;
  if(!module) {
    C2D_ERR("failed");
    return -EINVAL;
  }
  C2D_LOW("msg.type=%d", msg.type);
  c2d_module_ctrl_t *ctrl = (c2d_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  rc = write(ctrl->pfd[WRITE_FD], &msg, sizeof(c2d_thread_msg_t));
  if(rc < 0) {
    C2D_ERR("write() failed\n");
    return -EIO;
  }
  return 0;
}

/* c2d_module_reorder_q_event:
 *
 * @module: c2d module pointer
 * @event:  c2d_event to be queued
 * @prio:   priority of the event(realtime/offline)
 *
 * Description:
 *  Enqueues a c2d_event into realtime or offline queue based on the
 *  priority.
 *
 **/
int32_t c2d_module_reorder_q_event(mct_module_t* module,
  c2d_module_event_t* c2d_event, c2d_priority_t prio)
{
  if(!module || !c2d_event) {
    C2D_ERR("failed, module=%p, event=%p", module, c2d_event);
    return -EINVAL;
  }
  c2d_module_ctrl_t *ctrl = (c2d_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  c2d_module_event_t* loc_c2d_event = NULL;
  int length, i;

  C2D_LOW("prio=%d",prio);
  switch (prio) {
    case C2D_PRIORITY_REALTIME:
      PTHREAD_MUTEX_LOCK(&(ctrl->realtime_queue.mutex));
      length = ctrl->realtime_queue.q->length;
      C2D_DBG("real-time queue size = %d", ctrl->realtime_queue.q->length);
      mct_queue_push_tail(ctrl->realtime_queue.q, c2d_event);
      C2D_DBG("real-time queue size after streamoff = %d",
        ctrl->realtime_queue.q->length);
      for (i = 0; i < length; i++) {
        loc_c2d_event = (c2d_module_event_t *)
          mct_queue_pop_head(ctrl->realtime_queue.q);
        mct_queue_push_tail(ctrl->realtime_queue.q, loc_c2d_event);
      }
      C2D_DBG("real-time queue size = %d, after re-add",
        ctrl->realtime_queue.q->length);
      PTHREAD_MUTEX_UNLOCK(&(ctrl->realtime_queue.mutex));
      break;
    case C2D_PRIORITY_OFFLINE:
      PTHREAD_MUTEX_LOCK(&(ctrl->offline_queue.mutex));
      length = ctrl->offline_queue.q->length;
      C2D_LOW("offline queue size = %d", ctrl->offline_queue.q->length);
      mct_queue_push_tail(ctrl->offline_queue.q, c2d_event);
      C2D_LOW("offline queue size after streamoff = %d",
        ctrl->offline_queue.q->length);
      for (i = 0; i < length; i++) {
        loc_c2d_event = (c2d_module_event_t *)
          mct_queue_pop_head(ctrl->offline_queue.q);
        mct_queue_push_tail(ctrl->offline_queue.q, loc_c2d_event);
      }
      C2D_LOW("offline queue size = %d", ctrl->offline_queue.q->length);
      PTHREAD_MUTEX_UNLOCK(&(ctrl->offline_queue.mutex));
      break;
    default:
      C2D_ERR("failed, bad prio value=%d", prio);
      return -EINVAL;
  }
  return 0;
}

/* c2d_module_enq_event:
 *
 * @module: c2d module pointer
 * @event:  c2d_event to be queued
 * @prio:   priority of the event(realtime/offline)
 *
 * Description:
 *  Enqueues a c2d_event into realtime or offline queue based on the
 *  priority.
 *
 **/
int32_t c2d_module_enq_event(mct_module_t* module,
  c2d_module_event_t* c2d_event, c2d_priority_t prio)
{
  if(!module || !c2d_event) {
    C2D_ERR("failed, module=%p, event=%p", module, c2d_event);
    return -EINVAL;
  }
  c2d_module_ctrl_t *ctrl = (c2d_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);

  C2D_LOW("prio=%d, type %d",prio, c2d_event->type);
  switch (prio) {
  case C2D_PRIORITY_REALTIME:
    PTHREAD_MUTEX_LOCK(&(ctrl->realtime_queue.mutex));
    mct_queue_push_tail(ctrl->realtime_queue.q, c2d_event);
    C2D_LOW("real-time queue size = %d", ctrl->realtime_queue.q->length);
    PTHREAD_MUTEX_UNLOCK(&(ctrl->realtime_queue.mutex));
    break;
  case C2D_PRIORITY_OFFLINE:
    PTHREAD_MUTEX_LOCK(&(ctrl->offline_queue.mutex));
    mct_queue_push_tail(ctrl->offline_queue.q, c2d_event);
    C2D_LOW("offline queue size = %d", ctrl->offline_queue.q->length);
    PTHREAD_MUTEX_UNLOCK(&(ctrl->offline_queue.mutex));
    break;
  default:
    C2D_ERR("failed, bad prio value=%d", prio);
    return -EINVAL;
  }
  return 0;
}

int32_t c2d_module_send_event_downstream(mct_module_t* module,
   mct_event_t* event)
{
  boolean ret;
  if(!module || !event) {
    C2D_ERR("failed, module=%p, event=%p", module, event);
    return -EINVAL;
  }
  uint32_t identity = event->identity;

  /* TODO: Because module type is not identity based in mct_module_t we can
     either privately use it or depend on the port's peer existence to
     determine whether to forward event downstream or not. So temporarily
     disabling this condition check as because video stream comes after
     preview and could potentially modify module->type to sink so that
     downstream module will not receive events and hence ack is not reflected
     back. Non-existence of a port with match identity means downstream module
     is not available and is just an indication of this module is sink or
     peerless */
#if 0
  /* forward the event downstream only if we are not sink/peerless module */
  if(module->type == MCT_MODULE_FLAG_SINK ||
     module->type == MCT_MODULE_FLAG_PEERLESS) {
    /* C2D_DBG("info: module is sink/peerless, event not"
         "sent for identity=0x%x", identity);
    */
    return 0;
  }
#endif
  /* find corresponding source port */
  mct_port_t* port = c2d_module_find_port_with_identity(module, MCT_PORT_SRC,
                       identity);
  if(!port) {
    C2D_LOW("info: no source port found.with identity=0x%x", identity);
    return 0;
  }
  /* if port has a peer, post event to the downstream peer */
  if(MCT_PORT_PEER(port) == NULL) {
    C2D_ERR("failed, no downstream peer found.");
    return -EINVAL;
  }
  ret = mct_port_send_event_to_peer(port, event);
  if(ret == FALSE) {
    C2D_ERR("failed\n");
    return -EFAULT;
  }
  return 0;
}

/* c2d_module_send_event_upstream:
 *
 * Description:
 *  Sends event to the upstream peer based on the event identity.
 *
 **/
int32_t c2d_module_send_event_upstream(mct_module_t* module,
   mct_event_t* event)
{
  boolean ret;
  if(!module || !event) {
    C2D_ERR("failed, module=%p, event=%p", module, event);
    return -EINVAL;
  }
  uint32_t identity = event->identity;
  /* find corresponding sink port */
  mct_port_t* port = c2d_module_find_port_with_identity(module, MCT_PORT_SINK,
                       identity);
  if(!port) {
    C2D_ERR("failed, no sink port found.with identity=0x%x",
      identity);
    return -EINVAL;
  }
  /* if port has a peer, post event to the upstream peer */
  if(!MCT_PORT_PEER(port)) {
    C2D_ERR("failed, no upstream peer found.");
    return -EINVAL;
  }
  ret = mct_port_send_event_to_peer(port, event);
  if(ret == FALSE) {
    C2D_ERR("failed\n");
    return -EFAULT;
  }
  return 0;
}

/** c2d_module_invalidate_and_free_qentry
 *    @queue: queue to invalidate and free entries
 *    @identity: identity to invalidate
 *
 *  Invalidate the queue entries corresponding to
 *  given identity. The invalidated entries are acked
 *  and then freed from the list.
 *
 *  Return: void
 **/
static void c2d_module_invalidate_and_free_qentry(c2d_module_ctrl_t* ctrl,
  c2d_module_event_queue_t *queue, uint32_t identity)
{
  mct_list_t *key_list = NULL;
  void*  input[3];
  input[0] = ctrl;
  input[1] = &identity;
  input[2] = &key_list;
  /* First get all keys correspoding to the identity in key_list. Then traverse
     key_list and release the acks from ack_list. This is to avoid holding queue
     mutex when sending an event upstream to avoid potential deadlocks */
  PTHREAD_MUTEX_LOCK(&(queue->mutex));
  mct_queue_traverse(queue->q, c2d_module_invalidate_q_traverse_func,
    input);
  PTHREAD_MUTEX_UNLOCK(&(queue->mutex));
  /* traverse key list to release acks */
  mct_list_traverse(key_list, c2d_module_release_ack_traverse_func, ctrl);
  /* free the key list */
  mct_list_free_all(key_list, c2d_module_key_list_free_traverse_func);
  return;
}

/* c2d_module_invalidate_queue:
 *
 **/
int32_t c2d_module_invalidate_queue(c2d_module_ctrl_t* ctrl,
  uint32_t identity)
{
  if(!ctrl) {
    C2D_ERR("failed\n");
    return -EINVAL;
  }
  c2d_module_invalidate_and_free_qentry(ctrl, &ctrl->realtime_queue, identity);
  c2d_module_invalidate_and_free_qentry(ctrl, &ctrl->offline_queue, identity);
  return 0;
}

/* c2d_module_send_buf_divert_ack:
 *
 *  Sends a buf_divert_ack to upstream module.
 *
 **/
static int32_t c2d_module_send_buf_divert_ack(c2d_module_ctrl_t *ctrl,
  isp_buf_divert_ack_t isp_ack)
{
  mct_event_t event;
  int32_t rc;
  memset(&event, 0x00, sizeof(mct_event_t));
  event.type = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = isp_ack.identity;
  event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT_ACK;
  event.u.module_event.module_event_data = &isp_ack;
  C2D_DBG("sending isp ack with event identity=0x%x, buf identity 0x%x,"
          "is_buf_dirty=%d, buf_idx=%d", isp_ack.identity,
          isp_ack.buf_identity, isp_ack.is_buf_dirty, isp_ack.buf_idx);
  rc = c2d_module_send_event_upstream(ctrl->p_module, &event);
  if(rc < 0) {
    C2D_ERR("failed");
    return -EFAULT;
  }
  return 0;
}

/* c2d_module_do_ack:
 *
 *  Decrements the refcount of the ACK which is stored in the ack_list,
 *  correspoding to the key. If the refcount becomes 0, a buf_divert_ack
 *  is sent upstream. At this time the ack entry is removed from list.
 *
 **/
int32_t c2d_module_do_ack(c2d_module_ctrl_t *ctrl,
  c2d_module_ack_key_t key, uint32_t buffer_access)
{
  if(!ctrl) {
    C2D_ERR("failed");
    return -EINVAL;
  }
  /* find corresponding ack from the list. If the all references
     to that ack are done, send the ack and remove the entry from the list */
  c2d_module_ack_t *c2d_ack;
  C2D_LOW("buf_idx=%d, identity=0x%x", key.buf_idx, key.identity);
  PTHREAD_MUTEX_LOCK(&(ctrl->ack_list.mutex));
  c2d_ack = c2d_module_find_ack_from_list(ctrl, key);
  if(!c2d_ack) {
    C2D_ERR("failed, ack not found in list, for buf_idx=%d, "
      "identity=0x%x", key.buf_idx, key.identity);
    PTHREAD_MUTEX_UNLOCK(&(ctrl->ack_list.mutex));
    return -EFAULT;
  }
  c2d_ack->ref_count--;

  // While creating cpp_ack, we set isp_buf_divert_ack.buffer_access = 0,
  // We need to do | of all acks that we are getting from downstream.
  c2d_ack->isp_buf_divert_ack.buffer_access |= buffer_access;

  C2D_DBG("c2d_ack->ref_count=%d\n", c2d_ack->ref_count);
  struct timeval tv;
  if(c2d_ack->ref_count == 0) {
    ctrl->ack_list.list = mct_list_remove(ctrl->ack_list.list, c2d_ack);
    ctrl->ack_list.size--;
    /* unlock before sending event to prevent any deadlock */
    PTHREAD_MUTEX_UNLOCK(&(ctrl->ack_list.mutex));
    gettimeofday(&(c2d_ack->out_time), NULL);
    /* fill buf identity from key */
    c2d_ack->isp_buf_divert_ack.buf_identity = key.buf_identity;

    C2D_LOW("in_time=%ld.%ld us, out_time=%ld.%ld us, ",
      c2d_ack->in_time.tv_sec, c2d_ack->in_time.tv_usec,
      c2d_ack->out_time.tv_sec, c2d_ack->out_time.tv_usec);
    C2D_LOW("holding time = %6ld us, ",
      (c2d_ack->out_time.tv_sec - c2d_ack->in_time.tv_sec)*1000000L +
      (c2d_ack->out_time.tv_usec - c2d_ack->in_time.tv_usec));
    c2d_module_send_buf_divert_ack(ctrl, c2d_ack->isp_buf_divert_ack);
    gettimeofday(&tv, NULL);
    C2D_LOW("upstream event time = %6ld us, ",
      (tv.tv_sec - c2d_ack->out_time.tv_sec)*1000000L +
      (tv.tv_usec - c2d_ack->out_time.tv_usec));
    free(c2d_ack);
  } else {
    PTHREAD_MUTEX_UNLOCK(&(ctrl->ack_list.mutex));
  }
  return 0;
}

/* c2d_module_handle_ack_from_downstream:
 *
 *  Handles the buf_divert_ack event coming from downstream module.
 *  Corresponding ACK stored in ack_list is updated and/or released
 *  accordingly.
 *
 */
static int32_t c2d_module_handle_ack_from_downstream(mct_module_t* module,
  mct_event_t* event)
{
  int32_t                      rc = 0;
  c2d_module_stream_params_t  *stream_params = NULL;
  c2d_module_session_params_t *session_params = NULL;
  boolean                      bool_ret = TRUE;
  c2d_hardware_cmd_t           cmd;

  if(!module || !event) {
    C2D_ERR("failed, module=%p, event=%p\n", module, event);
    return -EINVAL;
  }
  c2d_module_ctrl_t* ctrl = (c2d_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    C2D_ERR("failed\n");
    return -EINVAL;
  }

  c2d_module_get_params_for_identity(ctrl, event->identity, &session_params,
    &stream_params);
  if (!session_params || !stream_params) {
    C2D_ERR("failed params %p %p\n",
      session_params, stream_params);
    return -EFAULT;
  }

  isp_buf_divert_ack_t* isp_buf_ack =
    (isp_buf_divert_ack_t*)(event->u.module_event.module_event_data);

  if ((stream_params->stream_info->is_type == IS_TYPE_EIS_2_0  ||
    stream_params->interleaved) && !stream_params->single_module){

    C2D_HIGH("CAMCACHE : BufAck PUT_BUF(native) : buffer_access=%s(0x%x)",
      (isp_buf_ack->buffer_access == 0) ? "NONE" :
      (isp_buf_ack->buffer_access == CPU_HAS_READ) ? "READ" :
      (isp_buf_ack->buffer_access == CPU_HAS_WRITTEN) ? "WRITTEN" :
      "READ_WRITTEN",
      isp_buf_ack->buffer_access);

    if (stream_params->native_buff) {

      if (isp_buf_ack->buffer_access != 0) {
        // Do invalidate before put_buf
        pp_frame_buffer_t* pp_buf =
          pp_native_buf_manage_get_buf_info(&ctrl->native_buf_mgr,
          session_params->session_id,
          C2D_GET_STREAM_ID(stream_params->streaming_identity),
          isp_buf_ack->buf_idx);

        if (pp_buf) {
          pp_native_buf_mgr_cacheops(&ctrl->native_buf_mgr,
            pp_buf->vaddr,
            pp_buf->fd,
            stream_params->hw_params.output_info.plane_info.frame_len,
            PP_BUF_MGR_CACHE_INVALIDATE);
        }
      }
      pp_native_manager_put_buf(&ctrl->native_buf_mgr,
        session_params->session_id,
        C2D_GET_STREAM_ID(stream_params->streaming_identity),
        isp_buf_ack->buf_idx);
    } else {
      if (isp_buf_ack->buffer_access != 0) {
        // Do invalidate before put_buf
        mct_stream_map_buf_t *p_img_buf =
          pp_buf_mgr_get_buf_info(ctrl->buf_mgr,
          isp_buf_ack->buf_idx, stream_params->stream_info);

        if (p_img_buf) {
          pp_buf_mgr_cacheops(ctrl->buf_mgr,
            p_img_buf->buf_planes[0].buf,
            p_img_buf->buf_planes[0].fd,
            p_img_buf->buf_planes[0].size,
            PP_BUF_MGR_CACHE_INVALIDATE);
        }
      }
      /* Put acked buffer */
      bool_ret = pp_buf_mgr_put_buf(ctrl->buf_mgr,isp_buf_ack->identity,
        isp_buf_ack->buf_idx, isp_buf_ack->frame_id, isp_buf_ack->timestamp);
      if (bool_ret == FALSE) {
        C2D_ERR("pp_buf_mgr_put_buf idx %d frame id %d\n",
          isp_buf_ack->buf_idx, isp_buf_ack->frame_id);
      }
    }
  } else {
    c2d_module_ack_key_t key;
    key.identity = isp_buf_ack->identity;
    key.buf_idx = isp_buf_ack->buf_idx;
    key.buf_identity = isp_buf_ack->buf_identity;
    key.channel_id =  isp_buf_ack->channel_id;
    C2D_DBG("doing ack for divert_done ack from downstream");
    c2d_module_do_ack(ctrl, key, isp_buf_ack->buffer_access);
  }

  cmd.type = C2D_HW_CMD_RELEASE_DIVERT_BUF;
  cmd.u.event_data.frame_id = isp_buf_ack->frame_id;
  cmd.u.event_data.identity =  event->identity;
  cmd.u.event_data.buf_idx = isp_buf_ack->buf_idx;
  cmd.u.event_data.cookie = NULL;
  rc = c2d_hardware_process_command(ctrl->c2dhw, cmd);

  if (!bool_ret)
    rc = -EINVAL;

  return rc;
}

/* c2d_module_put_new_ack_in_list:
 *
 * Description:
 *   Adds a new ACK in the ack_list with the given params.
 **/
int32_t c2d_module_put_new_ack_in_list(c2d_module_ctrl_t *ctrl,
  c2d_module_ack_key_t key, int32_t buf_dirty, int32_t ref_count,
  isp_buf_divert_t *isp_buf)
{
  if(!ctrl) {
    C2D_ERR("failed\n");
    return -EINVAL;
  }
  /* this memory will be freed by thread when ack is removed from list */
  c2d_module_ack_t *c2d_ack =
    (c2d_module_ack_t *) CAM_MALLOC (sizeof(c2d_module_ack_t));
  if(!c2d_ack) {
    C2D_ERR("CAM_MALLOC failed\n");
    return -ENOMEM;
  }
  memset(c2d_ack, 0x00, sizeof(c2d_module_ack_t));
  c2d_ack->isp_buf_divert_ack.identity = key.identity;
  c2d_ack->isp_buf_divert_ack.buf_idx = key.buf_idx;
  c2d_ack->isp_buf_divert_ack.is_buf_dirty = buf_dirty;
  c2d_ack->isp_buf_divert_ack.channel_id = key.channel_id;
  c2d_ack->isp_buf_divert_ack.frame_id = isp_buf->buffer.sequence;
  c2d_ack->isp_buf_divert_ack.timestamp = isp_buf->buffer.timestamp;
  // Start buffer access with 0. We handle buf_divert in c2d and forward
  // it downstream. Once the ack comes back, we need to OR the
  // buffer_access flags from all the acks
  c2d_ack->isp_buf_divert_ack.buffer_access = 0;
  c2d_ack->ref_count = ref_count;
  C2D_DBG("adding ack in list, identity=0x%x",
    c2d_ack->isp_buf_divert_ack.identity);
  C2D_DBG("buf_idx=%d, ref_count=%d",
    c2d_ack->isp_buf_divert_ack.buf_idx, c2d_ack->ref_count);
  PTHREAD_MUTEX_LOCK(&(ctrl->ack_list.mutex));
  gettimeofday(&(c2d_ack->in_time), NULL);
  ctrl->ack_list.list = mct_list_append(ctrl->ack_list.list,
                          c2d_ack, NULL, NULL);
  ctrl->ack_list.size++;
  PTHREAD_MUTEX_UNLOCK(&(ctrl->ack_list.mutex));
  return 0;
}

int32_t c2d_module_process_downstream_event(mct_module_t* module,
  mct_event_t* event)
{
  boolean ret;
  int32_t rc;
  if(!module || !event) {
    C2D_ERR("failed, module=%p, event=%p", module, event);
    return -EINVAL;
  }
  uint32_t identity = event->identity;
  c2d_module_ctrl_t *ctrl = (c2d_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  /* handle events based on type, if not handled, forward it downstream */
  switch(event->type) {
  case MCT_EVENT_MODULE_EVENT: {
    switch(event->u.module_event.type) {
    case MCT_EVENT_MODULE_BUF_DIVERT:
      C2D_LOW("MCT_EVENT_MODULE_BUF_DIVERT: identity=0x%x", identity);
      rc = c2d_module_handle_buf_divert_event(module, event);
      if(rc < 0) {
        C2D_ERR("failed\n");
        return rc;
      }
      break;
    case MCT_EVENT_MODULE_IFACE_REQUEST_PP_DIVERT: {
      pp_buf_divert_request_t *divert_request;
      divert_request =
        (pp_buf_divert_request_t *)(event->u.module_event.module_event_data);
      divert_request->need_divert = TRUE;
      divert_request->num_additional_buffers += 1;
      divert_request->buf_alignment = mct_util_calculate_lcm(
        divert_request->buf_alignment,
        ctrl->c2dhw->caps.buf_align_caps.width_padding);
      C2D_LOW("IFACE_REQUEST_PP_DIVERT: iden=0x%x, buf_alignment %d",
        identity, divert_request->buf_alignment);
      rc = c2d_module_send_event_downstream(module, event);
      if(rc < 0) {
        C2D_ERR("failed\n");
      }
      break;
    }
      break;
    case MCT_EVENT_MODULE_ISP_OUTPUT_DIM:
      C2D_LOW("MCT_EVENT_MODULE_ISP_OUTPUT_DIM: identity=0x%x", identity);
      rc = c2d_module_handle_isp_out_dim_event(module, event);
      if (rc < 0) {
        C2D_ERR("failed\n");
        //isp output dim should be passed to downstream no matter what
        rc = c2d_module_send_event_downstream(module, event);
        return rc;
      }
      break;
    case MCT_EVENT_MODULE_STREAM_CROP:
      C2D_LOW("MCT_EVENT_MODULE_STREAM_CROP: identity=0x%x", identity);
      rc = c2d_module_handle_stream_crop_event(module, event);
      if(rc < 0) {
        C2D_ERR("failed\n");
        return rc;
      }
      break;
    case MCT_EVENT_MODULE_STATS_DIS_UPDATE:
      C2D_LOW("MCT_EVENT_MODULE_STATS_DIS_UPDATE: identity=0x%x", identity);
      rc = c2d_module_handle_dis_update_event(module, event);
      if(rc < 0) {
        C2D_ERR("failed\n");
        return rc;
      }
      break;
    case MCT_EVENT_MODULE_ISP_DIS_CONFIG: {
      C2D_LOW("MCT_EVENT_MODULE_ISP_DIS_CONFIG: identity=0x%x", identity);
      rc = c2d_module_handle_dis_config_event(module, event);
      }
      break;
    case MCT_EVENT_MODULE_SET_STREAM_CONFIG:
      C2D_LOW("MCT_EVENT_MODULE_SET_STREAM_CONFIG: identity=0x%x", identity);
      rc = c2d_module_handle_stream_cfg_event(module, event);
      if(rc < 0) {
        C2D_ERR("failed\n");
        return rc;
      }
      break;
    default:
      rc = c2d_module_send_event_downstream(module, event);
      if(rc < 0) {
        C2D_ERR("failed, module_event_type=%d, identity=0x%x",
          event->u.module_event.type, identity);
        return -EFAULT;
      }
      break;
    }
    break;
  }
  case MCT_EVENT_CONTROL_CMD: {
    switch(event->u.ctrl_event.type) {
    case MCT_EVENT_CONTROL_STREAMON: {
      rc = c2d_module_handle_streamon_event(module, event);
      if(rc < 0) {
        C2D_ERR("streamon failed\n");
        return rc;
      }
      break;
    }
    case MCT_EVENT_CONTROL_STREAMOFF: {
      rc = c2d_module_handle_streamoff_event(module, event);
      if(rc < 0) {
        C2D_ERR("streamoff failed\n");
        return rc;
      }
      break;
    }
    case MCT_EVENT_CONTROL_SET_PARM: {
      rc = c2d_module_handle_set_parm_event(module, event);
      if(rc < 0) {
        C2D_ERR("failed\n");
        return rc;
      }

      /* Since handle set param event is being reused from sof_set_param_event
         this function does not forward the event to downstream module.*/
      rc = c2d_module_send_event_downstream(module, event);
      if(rc < 0) {
        C2D_ERR("failed, module_event_type=%d, identity=0x%x",
          event->u.module_event.type, event->identity);
        return -EFAULT;
      }
      break;
    }

    case MCT_EVENT_CONTROL_SET_SUPER_PARM: {
      rc = c2d_module_handle_sof_set_parm_event(module, event);
      if(rc < 0) {
        C2D_ERR("failed\n");
        return rc;
      }
      break;
    }

    default:
      rc = c2d_module_send_event_downstream(module, event);
      if(rc < 0) {
        C2D_ERR("failed, control_event_type=%d, identity=0x%x",
          event->u.ctrl_event.type, identity);
        return -EFAULT;
      }
      break;
    }
    break;
  }
  default:
    C2D_ERR("failed, bad event type=%d, identity=0x%x",
      event->type, identity);
    return -EFAULT;
  }
  return 0;
}

int32_t c2d_module_process_upstream_event(mct_module_t* module,
  mct_event_t *event)
{
  int32_t rc;
  if(!module || !event) {
    C2D_ERR("failed, module=%p, event=%p", module, event);
    return -EINVAL;
  }
  uint32_t identity = event->identity;
  C2D_LOW("identity=0x%x, event->type=%d", identity, event->type);
  /* todo : event handling */
  switch(event->type) {
  case MCT_EVENT_MODULE_EVENT: {
    switch(event->u.module_event.type) {
    case MCT_EVENT_MODULE_BUF_DIVERT_ACK: {
      C2D_LOW("MCT_EVENT_MODULE_BUF_DIVERT_ACK: identity=0x%x", identity);
      c2d_module_stream_params_t  *stream_params = NULL;
      c2d_module_session_params_t *session_params = NULL;
      c2d_module_ctrl_t* ctrl = (c2d_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
      if(!ctrl) {
        C2D_ERR("failed\n");
        return -EINVAL;
      }

      c2d_module_get_params_for_identity(ctrl, event->identity, &session_params,
         &stream_params);
      if (!session_params || !stream_params) {
        C2D_ERR("failed params %p %p\n", session_params, stream_params);
        return -EFAULT;
      }

      rc = c2d_module_handle_ack_from_downstream(module, event);
      if(rc < 0) {
        C2D_ERR("failed\n");
      }
      if ((stream_params->stream_info->is_type == IS_TYPE_EIS_2_0  ||
        stream_params->interleaved) && !stream_params->single_module) {
        return rc;
      }
      break;
    }
    default:
      break;
    }
    break;
  }
  default:
    /* all upstream events are module events */
    break;
  }
  rc = c2d_module_send_event_upstream(module, event);
  if(rc < 0) {
    C2D_ERR("failed\n");
    return rc;
  }
  return 0;
}

int32_t c2d_module_init_native_buffers(c2d_module_ctrl_t *ctrl,
  cam_frame_len_offset_t *plane_info, uint32_t identity,
  int32_t buf_count)
{
  cam_frame_len_offset_t buf_info;
  int32_t rc;
  C2D_DBG("E: identity=%x", identity);

  uint32_t i;
  memset(&buf_info, 0x00, sizeof(cam_frame_len_offset_t));
  buf_info.frame_len = 0;
  buf_info.num_planes = plane_info->num_planes;
  for (i=0; i<plane_info->num_planes; i++) {
    buf_info.mp[i].stride = plane_info->mp[i].stride;
    buf_info.mp[i].scanline = plane_info->mp[i].scanline;
    buf_info.mp[i].len = plane_info->mp[i].len;
    C2D_HIGH("plane %d: st=%d, sc=%d, len=%d", i,
      buf_info.mp[i].stride, buf_info.mp[i].scanline, buf_info.mp[i].len);
  }
  buf_info.frame_len = plane_info->frame_len;
  C2D_HIGH("frame_len = %d", buf_info.frame_len);
  rc = pp_native_buf_mgr_allocate_buff(&ctrl->native_buf_mgr, buf_count,
    C2D_GET_SESSION_ID(identity), C2D_GET_STREAM_ID(identity),
    &buf_info, 0);
  if (rc < 0) {
    C2D_ERR("failed");
    return -1;
  }
  return 0;
}

/* c2d_module_get_input_format
 *
 *  @ peer_port_caps - port capabilities of the peer module
 *  @ stream_info - stream info snet during link modules
 *
 *  Return the format of the input frame, based on the peer source port format
 *
 *  Return value is input frame format flag.
 */
cam_format_t c2d_module_get_input_format(mct_port_caps_t *peer_port_caps,
  mct_stream_info_t* stream_info) {

  int32_t fmt = 0;
  if (peer_port_caps->u.frame.format_flag & MCT_PORT_CAP_FORMAT_YCBYCR) {
    return CAM_FORMAT_YUV_RAW_8BIT_YUYV;
  } else if (peer_port_caps->u.frame.format_flag & MCT_PORT_CAP_FORMAT_YCRYCB) {
    return CAM_FORMAT_YUV_RAW_8BIT_YVYU;
  } else if (peer_port_caps->u.frame.format_flag & MCT_PORT_CAP_FORMAT_CBYCRY) {
    return CAM_FORMAT_YUV_RAW_8BIT_UYVY;
  } else if (peer_port_caps->u.frame.format_flag & MCT_PORT_CAP_FORMAT_CRYCBY) {
    return CAM_FORMAT_YUV_RAW_8BIT_VYUY;
  } else {
    c2d_module_util_update_format(stream_info, &fmt);
    return fmt;
  }
}

int32_t c2d_module_check_interleaved_stream(mct_module_t* module,
  uint32_t identity)
{
  c2d_module_ctrl_t *ctrl = (c2d_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  uint32_t i,j;

  for(i=0; i < C2D_MODULE_MAX_SESSIONS; i++) {
    for(j=0; j < C2D_MODULE_MAX_STREAMS; j++) {
      if (ctrl->session_params[i]) {
        if ((ctrl->session_params[i]->stream_params[j]) &&
          (ctrl->session_params[i]->stream_params[j]->identity == identity)) {
          return (ctrl->session_params[i]->stream_params[j]->interleaved);
        }
      }
    }
  }
  return 0;
}

/*
 * c2d_module_update_output_dimension
 * @ hw_params - hardware params of the stream for whose output
 *              dimension needs to be update
 * @ stream_params - plane info of the output stream needs to be picked
 *                   from this stream params.
 * @ width - width of the output buffer
 * @ height - height of the output buffer
 * @ custom_param - custom calculation for plane information
 *
 * Update the output info in the input hw params with width/height provided
 * Custom calculation is used for plane info picked from the stream params.
 */
int32_t c2d_module_update_output_dimension(
  c2d_module_ctrl_t *ctrl,
  c2d_hardware_params_t *hw_params,
  c2d_module_stream_params_t *stream_params,
  uint32_t width, uint32_t height, bool custom_param)
{

  uint32_t i = 0, ht;
  hw_params->output_info.width = width;
  hw_params->output_info.height = height;
  if (!custom_param) {
    hw_params->output_info.plane_info =
      (stream_params->stream_info->buf_planes.plane_info);
  } else {
    hw_params->output_info.plane_info.num_planes =
      stream_params->stream_info->buf_planes.plane_info.num_planes;
    /* reset frame length to zero */
    hw_params->output_info.plane_info.frame_len = 0;
    for (i = 0; i <
      stream_params->stream_info->buf_planes.plane_info.num_planes; i++) {
      ht = !i ? hw_params->output_info.height :
        (hw_params->output_info.height / 2);
      hw_params->output_info.plane_info.mp[i].stride =
        PAD_TO_SIZE(hw_params->output_info.width,
        ctrl->c2dhw->caps.buf_align_caps.width_padding);
      hw_params->output_info.plane_info.mp[i].scanline =
        PAD_TO_SIZE(ht,
        ctrl->c2dhw->caps.buf_align_caps.height_padding);
      hw_params->output_info.plane_info.mp[i].len =
        hw_params->output_info.plane_info.mp[i].stride *
        hw_params->output_info.plane_info.mp[i].scanline;
      hw_params->output_info.plane_info.frame_len +=
        hw_params->output_info.plane_info.mp[i].len;
    }
  }
  C2D_DBG("plane %d, width %d height %d, frame_len %d, custom_param %d",
    stream_params->stream_info->buf_planes.plane_info.num_planes,
    width, height, hw_params->output_info.plane_info.frame_len, custom_param);
  C2D_DBG("stride0 %d, scanline0 %d , plane_len0 %d"
    "stride1 %d, scanline1 %d , plane_len1 %d",
    hw_params->output_info.plane_info.mp[0].stride,
    hw_params->output_info.plane_info.mp[0].scanline,
    hw_params->output_info.plane_info.mp[0].len,
    hw_params->output_info.plane_info.mp[1].stride,
    hw_params->output_info.plane_info.mp[1].scanline,
    hw_params->output_info.plane_info.mp[1].len);
  return 0;
}

/*
 * c2d_module_set_output_stream_parameters
 *
 * @ ctrl - c2d control parameters
 * @ session_params - session parameters
 * @ stream_params - Stream parameters
 *
 * Update the output dimesion/ streaming identity and
 * allocate native buffers if required.
 */

int32_t c2d_module_set_output_stream_parameters(
  c2d_module_ctrl_t *ctrl,
  c2d_module_session_params_t *session_params __unused,
  c2d_module_stream_params_t *stream_params)
{
  bool custom_calc = false;
  bool alloc_required = false;

  /* Use default params for session stream and if c2d is a single module */
  if ((stream_params->single_module) ||
    (stream_params->stream_info->stream_type == CAM_STREAM_TYPE_PARM)) {

    /* Copy over the default parameters */
    c2d_module_update_output_dimension(ctrl, &stream_params->hw_params,
      stream_params, stream_params->stream_info->dim.width,
      stream_params->stream_info->dim.height, false);
    return 0;
  }

  /*
   * use custom calculation if output format of the pipeline is UBWC.
   * stream info parameters have meta data entries as well. This could
   * have impact on c2d
   */
  custom_calc = (stream_params->stream_info->fmt ==
    CAM_FORMAT_YUV_420_NV12_UBWC) ||
    ((stream_params->linked_stream_params) &&
    (stream_params->linked_stream_params->stream_info->fmt ==
    CAM_FORMAT_YUV_420_NV12_UBWC));

  /*
   * Update stream params with linked stream parameters  if linked stream
   * dimension is bigger
   */
  if ((stream_params->stream_info->is_type == IS_TYPE_EIS_2_0) ||
    (stream_params->interleaved)) {

    /* native buffer required for  EIS_2_0 or interleaved format */
    stream_params->native_buff = TRUE;
    stream_params->streaming_identity = stream_params->identity;

    if (!stream_params->linked_stream_params) {
      C2D_DBG("No linked stream update default parameters");
      alloc_required = true;
    } else if (IS_LINKED_DIMENSION_BIGGER_OR_EQUAL(stream_params)) {
      /* update output_info for current stream from linked stream params */
      c2d_module_update_output_dimension(
        ctrl, &stream_params->hw_params,
        stream_params->linked_stream_params,
        stream_params->linked_stream_params->stream_info->dim.width,
        stream_params->linked_stream_params->stream_info->dim.height,
        custom_calc);

      /* update the streaming identity with linked stream's streaming identity */
      stream_params->streaming_identity =
        stream_params->linked_stream_params->streaming_identity;

      alloc_required = false;
    } else {
      /* update output_info for linked stream with current stream params */
      c2d_module_update_output_dimension(
        ctrl, &stream_params->linked_stream_params->hw_params,
        stream_params,
        stream_params->stream_info->dim.width,
        stream_params->stream_info->dim.height,
        custom_calc);

        /* update linked stream's streaming identity with stream's identity */
        stream_params->linked_stream_params->streaming_identity =
          stream_params->identity;
        alloc_required = true;
    }
  }

  /* Update output info for current streamCopy over the default parameters */
  c2d_module_update_output_dimension(ctrl, &stream_params->hw_params,
    stream_params, stream_params->stream_info->dim.width,
    stream_params->stream_info->dim.height, custom_calc);

  /* Allocate native buffer if required */
  if (alloc_required) {
     /* free and allocate buffer */
     /* Destroy and Re-create the native buffer queue */
     /* free native buffers */
     pp_native_buf_mgr_free_queue(&ctrl->native_buf_mgr,
       C2D_GET_SESSION_ID(stream_params->streaming_identity),
       C2D_GET_STREAM_ID(stream_params->streaming_identity));

     /* Create the native buffer queue */
     c2d_module_init_native_buffers(ctrl,
       &stream_params->hw_params.output_info.plane_info,
       stream_params->streaming_identity,
       C2D_NUM_NATIVE_BUFFERS);
  }

  return 0;
}



/* c2d_module_notify_add_stream:
 *
 * creates and initializes the stream-specific paramater structures when a
 * stream is reserved in port
 **/
int32_t c2d_module_notify_add_stream(mct_module_t* module, mct_port_t* port,
  mct_stream_info_t* stream_info, void *peer_caps)
{
  int32_t rc, fmt;
  cam_pp_feature_config_t *pp_config;
  c2d_module_stream_params_t *stream_params;
  if(!module || !stream_info || !port || !peer_caps) {
    C2D_ERR("failed, module=%p, port=%p, stream_info=%p peers %p\n",
      module, port, stream_info, peer_caps);
    return -EINVAL;
  }
  mct_port_caps_t *peer_port_caps = (mct_port_caps_t *)peer_caps;
  c2d_module_ctrl_t *ctrl = (c2d_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    C2D_ERR("failed, module=%p\n", module);
    return -EINVAL;
  }
  uint32_t identity = stream_info->identity;
  /* create stream specific params structure */
  uint32_t session_id;
  uint32_t i,j;
  boolean success = FALSE;
  c2d_hardware_params_t *hw_params;
  session_id = C2D_GET_SESSION_ID(identity);
  C2D_DBG("identity=0x%x\n", identity);

  /* find if a stream is already added on this port. If yes, we need to link
     that stream with this. (only for continuous streams)*/
  c2d_module_session_params_t *linked_session_params = NULL;
  c2d_module_stream_params_t *linked_stream_params = NULL;
  uint32_t linked_identity;
  int32_t found = c2d_port_get_linked_identity(port, identity,
    &linked_identity);
  if (found > 0) {
    C2D_DBG("found linked identity=0x%x", linked_identity);
    c2d_module_get_params_for_identity(ctrl,linked_identity,
      &linked_session_params, &linked_stream_params);
    if (!linked_stream_params) {
      C2D_ERR("failed, module=%p\n", module);
      return -EINVAL;
    }
  }

  for(i=0; i < C2D_MODULE_MAX_SESSIONS; i++) {
    if(ctrl->session_params[i]) {
      if(ctrl->session_params[i]->session_id == session_id) {
        for(j=0; j < C2D_MODULE_MAX_STREAMS; j++) {
          if(ctrl->session_params[i]->stream_params[j] == NULL) {
            ctrl->session_params[i]->stream_params[j] =
              (c2d_module_stream_params_t *)
                 CAM_MALLOC (sizeof(c2d_module_stream_params_t));
            if (!ctrl->session_params[i]->stream_params[j]) {
              C2D_ERR("failed: to CAM_MALLOC\n");
              return -ENOMEM;
            }
            memset(ctrl->session_params[i]->stream_params[j], 0x00,
              sizeof(c2d_module_stream_params_t));
            ctrl->session_params[i]->stream_params[j]->identity = identity;
            ctrl->session_params[i]->stream_params[j]->stream_info =
              stream_info;
            /* set linked stream */
            ctrl->session_params[i]->stream_params[j]->
              linked_stream_params = NULL;
            if (linked_stream_params) {
              ctrl->session_params[i]->stream_params[j]->
                linked_stream_params = linked_stream_params;
              linked_stream_params->linked_stream_params =
                ctrl->session_params[i]->stream_params[j];
            }
            ctrl->session_params[i]->stream_params[j]->parent = port;
            if (MCT_MODULE_FLAG_SINK == mct_module_find_type(module,identity))
              ctrl->session_params[i]->stream_params[j]->single_module = TRUE;
            else
              ctrl->session_params[i]->stream_params[j]->single_module = FALSE;
            /* assign priority */
            if(stream_info->streaming_mode == CAM_STREAMING_MODE_CONTINUOUS) {
              ctrl->session_params[i]->stream_params[j]->priority =
                C2D_PRIORITY_REALTIME;
            } else {
              ctrl->session_params[i]->stream_params[j]->priority =
                C2D_PRIORITY_REALTIME;
            }

            if (ctrl->session_params[i]->stream_params[j]->single_module &&
              stream_info->fmt == CAM_FORMAT_YUV_420_NV12_UBWC) {
              C2D_ERR("UBWC Invalid format for C2d sink module");
              return -EINVAL;
            }
            /* initialize input/output fps values */
            if(stream_info->stream_type == CAM_STREAM_TYPE_VIDEO) {
              ctrl->session_params[i]->stream_params[j]->
                hfr_skip_info.input_fps =
                ctrl->session_params[i]->fps_range.video_max_fps;
              ctrl->session_params[i]->stream_params[j]->
                hfr_skip_info.output_fps =
                ctrl->session_params[i]->fps_range.video_max_fps;
            } else {
              ctrl->session_params[i]->stream_params[j]->
                hfr_skip_info.input_fps = ctrl->session_params[i]->fps_range.max_fps;
              ctrl->session_params[i]->stream_params[j]->
                hfr_skip_info.output_fps = ctrl->session_params[i]->fps_range.max_fps;
            }
            C2D_HIGH("input_fps=%.2f, output_fps %f identity=0x%x",
              ctrl->session_params[i]->stream_params[j]->hfr_skip_info.input_fps,
              ctrl->session_params[i]->stream_params[j]->hfr_skip_info.output_fps,
              ctrl->session_params[i]->stream_params[j]->identity);

            ctrl->session_params[i]->stream_params[j]->
              hfr_skip_info.skip_count = 0;
            /* hfr_skip_required in only in preview stream */
            ctrl->session_params[i]->stream_params[j]->
              hfr_skip_info.skip_required =
                ((stream_info->stream_type == CAM_STREAM_TYPE_PREVIEW)||
                (stream_info->stream_type == CAM_STREAM_TYPE_CALLBACK) ||
                (stream_info->stream_type == CAM_STREAM_TYPE_VIDEO)) ?
                  TRUE : FALSE;

            /* assign stream type */
            ctrl->session_params[i]->stream_params[j]->stream_type =
              stream_info->stream_type;
            /* set interleaved */
            if (peer_port_caps->u.frame.format_flag & MCT_PORT_CAP_INTERLEAVED) {
              ctrl->session_params[i]->stream_params[j]->interleaved = 1;
            } else {
              ctrl->session_params[i]->stream_params[j]->interleaved = 0;
            }

            hw_params = &ctrl->session_params[i]->stream_params[j]->hw_params;

            stream_params = ctrl->session_params[i]->stream_params[j];
            stream_params->native_buff = FALSE;
            /* frame_hold is already set to 0 */
            stream_params->dis_hold.is_valid = FALSE;
            if (stream_info->is_type == IS_TYPE_EIS_2_0)
              stream_params->dis_enable = ctrl->session_params[i]->dis_enable;

            C2D_INFO("width %d, height %d, stride %d, scanline %d, is_type %d",
              stream_info->dim.width, stream_info->dim.height,
              stream_info->buf_planes.plane_info.mp[0].stride,
              stream_info->buf_planes.plane_info.mp[0].scanline,
              stream_info->is_type);

            c2d_module_set_output_stream_parameters(ctrl, ctrl->session_params[i],
              stream_params);

            if (stream_info->stream_type == CAM_STREAM_TYPE_PARM) {
              goto session_add_stream_exit;
            }

            /* rotation/flip */
            if (stream_info->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
                pp_config = &stream_info->reprocess_config.pp_feature_config;
            } else {
                pp_config = &stream_info->pp_config;
            }
            hw_params->mirror = pp_config->flip;
             C2D_DBG("Rotation=%d", pp_config->rotation);
            if (pp_config->rotation == ROTATE_0) {
                hw_params->rotation = 0;
            } else if (pp_config->rotation == ROTATE_90) {
                hw_params->rotation = 1;
            } else if (pp_config->rotation == ROTATE_180) {
                 hw_params->rotation = 2;
            } else if (pp_config->rotation == ROTATE_270) {
                 hw_params->rotation = 3;
            }

            if ((stream_info->is_type == IS_TYPE_EIS_2_0 ||
              ctrl->session_params[i]->stream_params[j]->interleaved) &&
              !ctrl->session_params[i]->stream_params[j]->single_module) {
              if ((hw_params->rotation == 1) || (hw_params->rotation == 3)) {
                uint32_t temp_dim;
                temp_dim = hw_params->output_info.width;
                hw_params->output_info.width = hw_params->output_info.height;
                hw_params->output_info.height = temp_dim;
                temp_dim = hw_params->output_info.plane_info.mp[0].stride;
                hw_params->output_info.plane_info.mp[0].stride =
                  hw_params->output_info.plane_info.mp[0].scanline;
                hw_params->output_info.plane_info.mp[0].scanline = temp_dim;

              }
              hw_params->rotation = 0;
            }

            c2d_module_util_update_format(stream_info, &fmt);
            /* format info */
            if (fmt == CAM_FORMAT_YUV_420_NV12 ||
                fmt == CAM_FORMAT_YUV_420_NV12_VENUS ||
                fmt == CAM_FORMAT_YUV_420_NV12_UBWC) {
              hw_params->output_info.c2d_plane_fmt = C2D_PARAM_PLANE_CBCR;
            } else if (fmt == CAM_FORMAT_YUV_420_NV21 ||
                fmt == CAM_FORMAT_YUV_420_NV21_VENUS) {
              hw_params->output_info.c2d_plane_fmt = C2D_PARAM_PLANE_CRCB;
            } else if (fmt == CAM_FORMAT_YUV_422_NV16) {
              hw_params->output_info.c2d_plane_fmt = C2D_PARAM_PLANE_CBCR422;
            } else if (fmt == CAM_FORMAT_YUV_422_NV61) {
              hw_params->output_info.c2d_plane_fmt = C2D_PARAM_PLANE_CRCB422;
            } else if (fmt == CAM_FORMAT_YUV_420_YV12) {
              hw_params->output_info.c2d_plane_fmt = C2D_PARAM_PLANE_CRCB420;
            } else {
              C2D_ERR("failed. Format not supported\n");
              return -EINVAL;
            }

            hw_params->output_info.cam_fmt = fmt;
           if (!linked_stream_params) {
             ctrl->session_params[i]->stream_params[j]->c2d_input_lib_params.format =
                  fmt;
             ctrl->session_params[i]->stream_params[j]->c2d_input_lib_params.surface_bit = C2D_SOURCE;
             rc = ctrl->c2d->func_tbl->process(ctrl->c2d_ctrl,
             PPROC_IFACE_CREATE_SURFACE,
             &ctrl->session_params[i]->stream_params[j]->c2d_input_lib_params);
             if (rc < 0) {
               C2D_ERR("failed");
               return rc;
             }
           } else {
             if ((linked_stream_params->stream_info->dim.width *
                 linked_stream_params->stream_info->dim.height <
                 stream_info->dim.width* stream_info->dim.height)) {
                 rc = ctrl->c2d->func_tbl->process(ctrl->c2d_ctrl,
                 PPROC_IFACE_DESTROY_SURFACE,
                 &linked_stream_params->c2d_input_lib_params.id);
                 ctrl->session_params[i]->stream_params[j]->c2d_input_lib_params.format =
                   c2d_module_get_input_format(peer_port_caps,stream_info);
                 ctrl->session_params[i]->stream_params[j]->c2d_input_lib_params.surface_bit = C2D_SOURCE;
                 rc = ctrl->c2d->func_tbl->process(ctrl->c2d_ctrl,
                 PPROC_IFACE_CREATE_SURFACE,
                 &ctrl->session_params[i]->stream_params[j]->c2d_input_lib_params);
                 if (rc < 0) {
                   C2D_ERR("failed");
                   return rc;
                 }
                 memcpy(
                   &linked_stream_params->c2d_input_lib_params,
                   &ctrl->session_params[i]->stream_params[j]->c2d_input_lib_params,
                   sizeof (c2d_libparams));
            } else {
                memcpy(
                 &ctrl->session_params[i]->stream_params[j]->c2d_input_lib_params,
                 &linked_stream_params->c2d_input_lib_params,
                 sizeof (c2d_libparams));
             }
          }
          if (stream_info->is_type == IS_TYPE_EIS_2_0) {
            ctrl->session_params[i]->stream_params[j]->c2d_output_lib_params.format =
              ctrl->session_params[i]->stream_params[j]->c2d_input_lib_params.format;
          } else {
            c2d_module_util_update_format(stream_info, &fmt);
            ctrl->session_params[i]->stream_params[j]->
                c2d_output_lib_params.format = fmt;
            if ((ctrl->session_params[i]->stream_params[j]->stream_type ==
                  CAM_STREAM_TYPE_PREVIEW && linked_stream_params &&
                  ctrl->session_params[i]->stream_params[j]->interleaved) &&
                  !ctrl->session_params[i]->stream_params[j]->single_module) {
              //change output format to that of linked stream
              ctrl->session_params[i]->stream_params[j]
                ->c2d_output_lib_params.format =
                linked_stream_params->c2d_output_lib_params.format;
              hw_params->output_info.cam_fmt =
                linked_stream_params->hw_params.output_info.cam_fmt;
              hw_params->output_info.c2d_plane_fmt =
                linked_stream_params->hw_params.output_info.c2d_plane_fmt;
              C2D_HIGH("changing output format c2d_plane_fmt %d identity 0x%x\n",
                hw_params->output_info.c2d_plane_fmt,
                hw_params->identity);
            }
          }
          ctrl->session_params[i]->stream_params[j]->c2d_output_lib_params.surface_bit = C2D_TARGET;
          rc = ctrl->c2d->func_tbl->process(ctrl->c2d_ctrl,
            PPROC_IFACE_CREATE_SURFACE,
            &ctrl->session_params[i]->stream_params[j]->c2d_output_lib_params);
           if (rc < 0) {
             C2D_ERR("failed");
             return rc;
           }

session_add_stream_exit:
            /* initialize the mutex for stream_params */
            pthread_mutex_init(
              &(ctrl->session_params[i]->stream_params[j]->mutex), NULL);
            /* Initialize stream info */
            ctrl->session_params[i]->stream_params[j]->stream_info =
              stream_info;
            ctrl->session_params[i]->stream_count++;
            success = TRUE;
            c2d_module_dump_stream_params(
              ctrl->session_params[i]->stream_params[j], __func__, __LINE__);
            break;
          }
        }
      }
    }
    if(success == TRUE) {
      break;
    }
  }
  if(success == FALSE) {
    C2D_ERR("failed, identity=0x%x", identity);
    return -EFAULT;
  }
  C2D_HIGH("info: success, identity=0x%x", identity);
  return 0;
}

/* c2d_module_notify_remove_stream:
 *
 *  destroys stream-specific data structures when a stream is unreserved
 *  in port
 **/
int32_t c2d_module_notify_remove_stream(mct_module_t* module, uint32_t identity)
{
  c2d_module_stream_params_t *stream_params;
  if(!module) {
    C2D_ERR("failed\n");
    return -EINVAL;
  }
  c2d_module_ctrl_t *ctrl = (c2d_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    C2D_ERR("failed\n");
    return -EINVAL;
  }
  C2D_HIGH("identity=0x%x\n", identity);
  /* destroy stream specific params structure */
  uint32_t session_id;
  uint32_t i,j;
  boolean success = FALSE;
  session_id = C2D_GET_SESSION_ID(identity);
  for(i=0; i < C2D_MODULE_MAX_SESSIONS; i++) {
    if(ctrl->session_params[i]) {
      if(ctrl->session_params[i]->session_id == session_id) {
        for(j=0; j < C2D_MODULE_MAX_STREAMS; j++) {
          if(ctrl->session_params[i]->stream_params[j]) {
            if(ctrl->session_params[i]->stream_params[j]->identity ==
                identity) {
              stream_params = ctrl->session_params[i]->stream_params[j];
              if (stream_params->stream_type == CAM_STREAM_TYPE_PARM) {
                goto session_remove_stream_exit;
              }
              if (!stream_params->linked_stream_params) {
                 ctrl->c2d->func_tbl->process(ctrl->c2d_ctrl,
                 PPROC_IFACE_DESTROY_SURFACE,
                 &stream_params->c2d_input_lib_params.id);

                 if ((stream_params->stream_info->is_type == IS_TYPE_EIS_2_0 ||
                   stream_params->interleaved) &&
                   !stream_params->single_module &&
                   (stream_params->stream_info->stream_type !=
                   CAM_STREAM_TYPE_PARM)){
                   pp_native_buf_mgr_free_queue(&ctrl->native_buf_mgr,
                     C2D_GET_SESSION_ID(stream_params->streaming_identity),
                     C2D_GET_STREAM_ID(stream_params->streaming_identity));
                 }
              }

              ctrl->c2d->func_tbl->process(ctrl->c2d_ctrl,
                PPROC_IFACE_DESTROY_SURFACE,
                &ctrl->session_params[i]->stream_params[j]->c2d_output_lib_params.id);
              /* remove linked params */
              if (ctrl->session_params[i]->stream_params[j]->
                  linked_stream_params) {
                ctrl->session_params[i]->stream_params[j]->
                  linked_stream_params->linked_stream_params = NULL;
                ctrl->session_params[i]->stream_params[j]->
                  linked_stream_params = NULL;
              }
session_remove_stream_exit:
              pthread_mutex_destroy(
                &(ctrl->session_params[i]->stream_params[j]->mutex));
              free(ctrl->session_params[i]->stream_params[j]);
              ctrl->session_params[i]->stream_params[j] = NULL;
              ctrl->session_params[i]->stream_count--;
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
    C2D_ERR("failed, identity=0x%x", identity);
    return -EFAULT;
  }
  return 0;
}

/** c2d_module_set_session_data: set session data
 *
 *  @module: c2d module handle
 *  @set_buf: set buffer handle that has session data
 *  @sessionid: session id for which session data shall be
 *            applied
 *
 *  This function provides session data that has per frame
 *  contorl parameters
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean c2d_module_set_session_data(mct_module_t *module,
  void *set_buf, uint32_t sessionid)
{
  boolean                      ret = FALSE;
  mct_pipeline_session_data_t *frame_ctrl_data = NULL;
  c2d_module_ctrl_t           *ctrl = NULL;
  c2d_module_session_params_t *session_params = NULL;
  int i = 0;

  /* Validate input parameters */
  if (!module || !set_buf) {
    C2D_LOW("failed: invalid params %p %p\n", module, set_buf);
    return FALSE;
  }

  frame_ctrl_data = (mct_pipeline_session_data_t *)set_buf;

  ctrl = (c2d_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    C2D_ERR("invalid control, failed");
    return FALSE;
  }

  /* get parameters based on the session id */
  for (i = 0; i < C2D_MODULE_MAX_SESSIONS; i++) {
    if ((ctrl->session_params[i]) &&
      (ctrl->session_params[i]->session_id == sessionid)) {
      session_params = ctrl->session_params[i];
      break;
    }
  }
  if(!session_params) {
    C2D_ERR("invalid session params, failed\n");
    return FALSE;
  }

  C2D_HIGH("sessionid:%d max_apply_delay%d max_report_delay:%d\n",
    sessionid, frame_ctrl_data->max_pipeline_frame_applying_delay,
    frame_ctrl_data->max_pipeline_meta_reporting_delay);
  PTHREAD_MUTEX_LOCK(&session_params->c2d_per_frame_mutex);
  session_params->max_apply_delay =
    frame_ctrl_data->max_pipeline_frame_applying_delay;
  session_params->max_report_delay =
    frame_ctrl_data->max_pipeline_meta_reporting_delay;
  PTHREAD_MUTEX_UNLOCK(&session_params->c2d_per_frame_mutex);

  return TRUE;
}
