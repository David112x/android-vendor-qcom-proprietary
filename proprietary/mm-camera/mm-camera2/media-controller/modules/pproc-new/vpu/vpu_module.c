/*============================================================================

 Copyright (c) 2014-2018 Qualcomm Technologies, Inc. All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#include "pp_buf_mgr.h"
#include "vpu_module.h"
#include "vpu_port.h"
#include "vpu_log.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#define VPU_PORT_NAME_LEN     32
#define VPU_NUM_SINK_PORTS    6
#define VPU_NUM_SOURCE_PORTS  6
#define VPU_CLIENT_ID         10

static const char vpu_sink_port_name[VPU_NUM_SINK_PORTS][VPU_PORT_NAME_LEN] = {
  "vpu_sink_0",
  "vpu_sink_1",
  "vpu_sink_2",
  "vpu_sink_3",
  "vpu_sink_4",
  "vpu_sink_5",
};

static const char vpu_src_port_name[VPU_NUM_SOURCE_PORTS][VPU_PORT_NAME_LEN] =
{
  "vpu_src_0",
  "vpu_src_1",
  "vpu_src_2",
  "vpu_src_3",
  "vpu_src_4",
  "vpu_src_5",
};

/** vpu_module_init:
 *  Args:
 *    @name: module name
 *  Return:
 *    - mct_module_t pointer corresponding to vpu on SUCCESS
 *    - NULL in case of FAILURE or if VPU hardware does not
 *      exist
 **/
mct_module_t *vpu_module_init(const char *name)
{
  mct_module_t      *module;
  vpu_module_ctrl_t *ctrl;
  uint32_t           i;
  VPU_HIGH("name=%s", name);

  /* enable the vpu module based on a property */
  char value[PROPERTY_VALUE_MAX];
  property_get("persist.vendor.camera.enable_vpu", value, "0");
  i = atoi(value);
  if (!i)
    return NULL;

  module = mct_module_create(name);
  if (!module) {
    VPU_ERR("failed");
    return NULL;
  }
  ctrl = vpu_module_create_vpu_ctrl();
  if (!ctrl) {
    VPU_ERR("failed");
    goto error_cleanup_module;
  }
  MCT_OBJECT_PRIVATE(module) = ctrl;
  ctrl->p_module = module;
  module->set_mod = vpu_module_set_mod;
  module->query_mod = vpu_module_query_mod;
  module->start_session = vpu_module_start_session;
  module->stop_session = vpu_module_stop_session;

  mct_port_t* port;

  /* Create default ports */
  for (i = 0; i < VPU_NUM_SOURCE_PORTS; i++) {
    port = vpu_port_create(vpu_src_port_name[i], MCT_PORT_SRC);
    if (!port) {
      VPU_ERR("failed");
      return NULL;
    }
    module->srcports = mct_list_append(module->srcports, port, NULL, NULL);
    MCT_PORT_PARENT(port) = mct_list_append(MCT_PORT_PARENT(port), module, NULL,
        NULL);
  }
  for (i = 0; i < VPU_NUM_SINK_PORTS; i++) {
    port = vpu_port_create(vpu_sink_port_name[i], MCT_PORT_SINK);
    if (!port) {
      VPU_ERR("failed");
      return NULL;
    }
    module->sinkports = mct_list_append(module->sinkports, port, NULL, NULL);
    MCT_PORT_PARENT(port) = mct_list_append(MCT_PORT_PARENT(port), module, NULL,
        NULL);
  }
  VPU_DBG("SUCCESS");
  return module;

  error_cleanup_module: mct_module_destroy(module);
  return NULL;
}

/** vpu_module_deinit:
 *
 *  Args:
 *    @module: pointer to vpu mct module
 *  Return:
 *    void
 **/
void vpu_module_deinit(mct_module_t *module)
{
  vpu_module_ctrl_t *ctrl = (vpu_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  vpu_module_destroy_vpu_ctrl(ctrl);
  mct_module_destroy(module);
}

static vpu_module_ctrl_t* vpu_module_create_vpu_ctrl(void)
{
  vpu_module_ctrl_t *ctrl = NULL;

  ctrl = (vpu_module_ctrl_t *) CAM_MALLOC(sizeof(vpu_module_ctrl_t));
  if (!ctrl) {
    VPU_ERR("CAM_MALLOC failed");
    return NULL;
  }
  memset(ctrl, 0x00, sizeof(vpu_module_ctrl_t));

  ctrl->ack_list.list = NULL;
  ctrl->ack_list.size = 0;
  pthread_mutex_init(&(ctrl->ack_list.mutex), NULL);

  ctrl->session_count = 0;

  pthread_mutex_init(&(ctrl->mutex), NULL);

  /* get handle to the buf manager */
  ctrl->buf_mgr = pp_buf_mgr_open();
  if (!ctrl->buf_mgr) {
    VPU_ERR("failed");
    goto err0;
  }

  /* init native buf mgr */
  if (!pp_native_buf_mgr_init(&ctrl->native_buf_mgr, VPU_CLIENT_ID)) {
    VPU_ERR("failed");
    goto err0;
  }

  ctrl->client = vpu_client_create();
  if (!ctrl->client) {
    VPU_ERR("failed");
    goto err0;
  }

  return ctrl;
  err0: free(ctrl);
  return NULL;
}

static int32_t vpu_module_destroy_vpu_ctrl(vpu_module_ctrl_t *ctrl)
{
  if (!ctrl) {
    VPU_ERR("ctrl=NULL");
    return 0;
  }
  pthread_mutex_destroy(&(ctrl->ack_list.mutex));
  pthread_mutex_destroy(&(ctrl->mutex));

  pp_buf_mgr_close(ctrl->buf_mgr);
  pp_native_buf_mgr_deinit(&ctrl->native_buf_mgr);
  vpu_client_destroy(ctrl->client);

  free(ctrl);
  return 0;
}

void vpu_module_set_mod(mct_module_t *module, mct_module_type_t module_type,
    uint32_t identity)
{
  VPU_DBG("module_type=%d", module_type);
  if (!module) {
    VPU_ERR("failed");
    return;
  }
  if (mct_module_find_type(module, identity) != MCT_MODULE_FLAG_INVALID) {
    mct_module_remove_type(module, identity);
  }
  mct_module_add_type(module, module_type, identity);
}

boolean vpu_module_query_mod(mct_module_t *module, void *buf,
    uint32_t sessionid __unused)
{
  if (!module || !buf) {
    VPU_ERR("failed, module=%p, query_buf=%p", module, buf);
    return FALSE;
  }
  mct_pipeline_cap_t *query_buf = (mct_pipeline_cap_t *) buf;
  mct_pipeline_pp_cap_t *pp_cap = &(query_buf->pp_cap);
  /* TODO */
  return TRUE;
}

/* vpu_module_send_buf_divert_ack_event:
 *
 *  Sends a buf_divert_ack to upstream module.
 *
 **/
static int32_t vpu_module_send_buf_divert_ack_event(vpu_module_ctrl_t *ctrl,
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
  VPU_DBG(
      "sending isp ack with identity=0x%x, is_buf_dirty=%d, " "buf_idx=%d channel_id=%d",
      isp_ack.identity, isp_ack.is_buf_dirty, isp_ack.buf_idx,
      isp_ack.channel_id);

  rc = vpu_module_send_event_upstream(ctrl->p_module, &event);
  if (rc < 0) {
    VPU_ERR("failed");
    return -1;
  }
  return 0;
}

int32_t vpu_module_do_ack_upstream(vpu_module_ctrl_t *ctrl, int32_t buf_idx)
{
  PTHREAD_MUTEX_LOCK(&ctrl->ack_list.mutex);
  vpu_module_ack_t *vpu_ack = vpu_module_find_ack_from_list(ctrl, buf_idx);
  PTHREAD_MUTEX_UNLOCK(&ctrl->ack_list.mutex);
  if (!vpu_ack) {
    VPU_ERR("failed");
    return -1;
  }

  vpu_module_send_buf_divert_ack_event(ctrl, vpu_ack->isp_buf_divert_ack);

  PTHREAD_MUTEX_LOCK(&ctrl->ack_list.mutex);
  /* remove entry from the list */
  ctrl->ack_list.list = mct_list_remove(ctrl->ack_list.list, vpu_ack);
  ctrl->ack_list.size--;
  PTHREAD_MUTEX_UNLOCK(&ctrl->ack_list.mutex);
  free(vpu_ack);
  return 0;
}

int32_t vpu_module_do_buf_done(vpu_module_ctrl_t *ctrl,
  uint32_t identity, struct v4l2_buffer *buffer)
{
  boolean ret;
  struct timeval tv;
  gettimeofday(&tv, NULL);
  ret = pp_buf_mgr_buf_done(ctrl->buf_mgr, identity,
    buffer->index, buffer->sequence, tv);
  if (ret == FALSE) {
    VPU_ERR("pp_buf_mgr_buf_done() failed");
    return -1;
  }
  return 0;
}

int32_t vpu_module_send_divert_downstream(vpu_module_ctrl_t *ctrl,
    struct v4l2_buffer *buffer)
{
  int32_t rc;
  mct_event_t event;
  struct v4l2_plane planes[VIDEO_MAX_PLANES];
  isp_buf_divert_t buf_div;
  vpu_module_session_params_t *session_params;
  vpu_module_stream_params_t *stream_params;
  void* vaddr = NULL;

  rc = vpu_module_get_params_for_identity(ctrl, ctrl->client_identity,
         &session_params, &stream_params);
  if (rc < 0) {
    VPU_ERR("failed");
    return -1;
  }
  /* if vpu output is native buf, divert will need to provide vaddr */
  if (stream_params->is_native_buf) {
    pp_frame_buffer_t* pp_buf = pp_native_buf_manage_get_buf_info(
        &ctrl->native_buf_mgr, VPU_GET_SESSION_ID(stream_params->native_identity),
        VPU_GET_STREAM_ID(stream_params->native_identity),
        buffer->index);
    if (!pp_buf) {
      VPU_ERR("failed, idx=%d, native_identity=%x",
        buffer->index, stream_params->native_identity);
      return -1;
    }
    vaddr = pp_buf->vaddr;
  }
  memset(&buf_div, 0x00, sizeof(isp_buf_divert_t));
  memcpy(planes, buffer->m.planes,
      VIDEO_MAX_PLANES * sizeof(struct v4l2_plane));

  buf_div.buffer = *buffer;
  buf_div.buffer.m.planes = planes;
  buf_div.identity = ctrl->client_identity;
  buf_div.native_buf = stream_params->is_native_buf;
  buf_div.fd = planes[0].m.fd;
  buf_div.vaddr = &vaddr;

  VPU_DBG("buf_idx=%d, vaddr=%p, frame-id=%d",
           buffer->index, vaddr, buffer->sequence);

  memset(&event, 0, sizeof(mct_event_t));
  event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT;
  event.u.module_event.module_event_data = (void *) &buf_div;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = ctrl->client_identity;
  event.direction = MCT_EVENT_DOWNSTREAM;

  rc = vpu_module_send_event_downstream(ctrl->p_module, &event);
  if (rc < 0) {
    VPU_ERR("failed");
    return rc;
  }

  /* if a piggy back ack is received */
  if (buf_div.ack_flag == TRUE) {
    if (stream_params->is_native_buf) {
      rc = pp_native_manager_put_buf(&ctrl->native_buf_mgr,
        VPU_GET_SESSION_ID(stream_params->native_identity),
        VPU_GET_STREAM_ID(stream_params->native_identity),
        buffer->index);
      if (rc < 0) {
        VPU_ERR("failed");
        return -1;
      }
    } else {
      vpu_module_do_buf_done(ctrl, buf_div.identity, buffer);
    }
  }
  return 0;
}

void vpu_module_client_cb(vpu_client_event_t event, void *arg, void *userdata)
{
  vpu_module_ctrl_t *ctrl = (vpu_module_ctrl_t *) userdata;
  struct v4l2_buffer *buffer = (struct v4l2_buffer *) arg;

  switch (event) {
  case VPU_CLIENT_EVENT_INPUT_BUF_READY:
    VPU_DBG("VPU_CLIENT_EVENT_INPUT_BUF_READY, idx=%d", buffer->index);
    vpu_module_do_ack_upstream(ctrl, buffer->index);
    break;
  case VPU_CLIENT_EVENT_OUTPUT_BUF_READY:
    VPU_DBG("VPU_CLIENT_EVENT_OUTPUT_BUF_READY, idx=%d", buffer->index);
    vpu_module_send_divert_downstream(ctrl, buffer);
    break;
  default:
    VPU_ERR("failed, invalid event=%d", event);
  }
}

static boolean vpu_module_start_session(mct_module_t *module,
    uint32_t sessionid)
{
  int32_t rc;
  if (!module) {
    VPU_ERR("failed");
    return FALSE;
  }
  vpu_module_ctrl_t *ctrl = (vpu_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if (ctrl->session_count >= VPU_MODULE_MAX_SESSIONS) {
    VPU_ERR("max session limit reached");
    return FALSE;
  }

  rc = vpu_client_init_session(ctrl->client, vpu_module_client_cb, ctrl);
  if (rc < 0) {
    VPU_ERR("failed");
    return FALSE;
  }

  uint32_t i;

  for (i=0; i<VPU_MODULE_MAX_SESSIONS; i++) {
    if (ctrl->session_params[i] == NULL) {
      ctrl->session_params[i] = (vpu_module_session_params_t *)
        CAM_MALLOC(sizeof(vpu_module_session_params_t));
      if (!ctrl->session_params[i]) {
        VPU_ERR("CAM_MALLOC failed");
        return FALSE;
      }
      memset(ctrl->session_params[i], 0x00, sizeof(vpu_module_session_params_t));
      ctrl->session_params[i]->session_id = sessionid;
      break;
    }
  }
  ctrl->session_count++;
  VPU_HIGH("session %d started", sessionid);
  return TRUE;
}

static boolean vpu_module_stop_session(mct_module_t *module,
    uint32_t sessionid)
{
  int32_t rc;
  if (!module) {
    VPU_ERR("failed");
    return FALSE;
  }
  vpu_module_ctrl_t *ctrl = (vpu_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if (ctrl->session_count == 0) {
    VPU_ERR("no sessions active");
    return FALSE;
  }

  rc = vpu_client_deinit_session(ctrl->client);
  if (rc < 0) {
    VPU_ERR("failed");
    return FALSE;
  }

  uint32_t i;
  for (i=0; i<VPU_MODULE_MAX_SESSIONS; i++) {
    if (ctrl->session_params[i] != NULL) {
      if (ctrl->session_params[i]->session_id == sessionid) {
        free(ctrl->session_params[i]);
        ctrl->session_params[i] = NULL;
        break;
      }
    }
  }

  ctrl->session_count--;
  VPU_HIGH("session %d stopped", sessionid);
  return TRUE;
}

int32_t vpu_module_notify_add_stream(mct_module_t* module, mct_port_t* port,
    mct_stream_info_t* stream_info)
{
  int32_t rc;
  if(!module || !stream_info || !port) {
    VPU_ERR("failed, module=%p, port=%p, stream_info=%p",
      module, port, stream_info);
    return -1;
  }
  vpu_module_ctrl_t *ctrl = (vpu_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if (!ctrl) {
    VPU_ERR("failed, module=%p", module);
    return -1;
  }
  uint32_t identity = stream_info->identity;
  VPU_DBG("identity = %x", identity);

  /* find if a stream is already added on this port. If yes, we need to link
     that stream with this. (only for continuous streams)*/
  uint32_t linked_identity;
  vpu_module_session_params_t *session_params;
  vpu_module_stream_params_t *stream_params;
  vpu_module_session_params_t *linked_session_params=NULL;
  vpu_module_stream_params_t *linked_stream_params=NULL;

  int32_t found = vpu_port_get_linked_identity(port, identity,
    &linked_identity);
  if (found > 0) {
    VPU_DBG("found linked identity=0x%x", linked_identity);
    vpu_module_get_params_for_identity(ctrl, linked_identity,
      &linked_session_params, &linked_stream_params);
  }

  rc = vpu_module_get_params_for_session_id(ctrl, VPU_GET_SESSION_ID(identity),
    &session_params);
  if (rc < 0) {
    VPU_ERR("failed");
    return -1;
  }
  if (session_params->stream_count >= VPU_MODULE_MAX_STREAMS) {
    VPU_ERR("failed, cannot add another stream, count=%d", session_params->stream_count);
    return -1;
  }

  uint32_t i;
  for (i=0; i<VPU_MODULE_MAX_STREAMS; i++) {
    if (session_params->stream_params[i] == NULL) {
      session_params->stream_params[i] = (vpu_module_stream_params_t *)
        CAM_MALLOC(sizeof(vpu_module_stream_params_t));
      if (!session_params->stream_params[i]) {
        VPU_ERR("CAM_MALLOC failed");
        return -1;
      }
      stream_params = session_params->stream_params[i];
      memset(stream_params, 0x00, sizeof(vpu_module_stream_params_t));
      stream_params->identity = identity;
      memcpy(&stream_params->stream_info, stream_info, sizeof(mct_stream_info_t));
      if (linked_stream_params) {
        /* link streams */
        stream_params->linked_stream_params = linked_stream_params;
        linked_stream_params->linked_stream_params = stream_params;
      }
      VPU_DBG("identity %x added, type = %d", identity, stream_info->stream_type);
      session_params->stream_count ++;
      return 0;
    }
  }
  VPU_ERR("failed, could not add identity %x", identity);
  return -1;
}

int32_t vpu_module_notify_remove_stream(mct_module_t* module, uint32_t identity)
{
  int32_t rc;
  vpu_module_ctrl_t *ctrl = (vpu_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  vpu_module_session_params_t *session_params;
  rc = vpu_module_get_params_for_session_id(ctrl, VPU_GET_SESSION_ID(identity),
    &session_params);
  if (rc < 0) {
    VPU_ERR("failed");
    return -1;
  }
  if (session_params->stream_count == 0) {
    VPU_ERR("failed, no streams added for session %d", VPU_GET_SESSION_ID(identity));
    return -1;
  }
  uint32_t i;
  for (i = 0; i < VPU_MODULE_MAX_STREAMS; i++) {
    if (session_params->stream_params[i] != NULL) {
      if (session_params->stream_params[i]->identity == identity) {
        if (session_params->stream_params[i]->linked_stream_params) {
          /* unlink streams */
          session_params->stream_params[i]->
            linked_stream_params->linked_stream_params = NULL;
        }
        free(session_params->stream_params[i]);
        session_params->stream_params[i] = NULL;
        session_params->stream_count--;
        break;
      }
    }
  }
  VPU_DBG("identity %x removed", identity);
  return 0;
}

int32_t vpu_module_send_event_downstream(mct_module_t* module,
    mct_event_t* event)
{
  boolean ret;
  if (!module || !event) {
    VPU_ERR("failed, module=%p, event=%p", module, event);
    return -EINVAL;
  }
  uint32_t identity = event->identity;

  /* find corresponding source port */
  mct_port_t* port = vpu_module_find_port_with_identity(module, MCT_PORT_SRC,
      identity);
  if (!port) {
    VPU_DBG("no source port found.with identity=0x%x", identity);
    return 0;
  }
  /* if port has a peer, post event to the downstream peer */
  if (MCT_PORT_PEER(port) == NULL) {
    VPU_ERR("failed, no downstream peer found");
    return -EINVAL;
  }
  ret = mct_port_send_event_to_peer(port, event);
  if (ret == FALSE) {
    VPU_ERR("failed");
    return -EFAULT;
  }
  return 0;
}

int32_t vpu_module_send_event_upstream(mct_module_t* module, mct_event_t* event)
{
  boolean ret;
  if (!module || !event) {
    VPU_ERR("failed, module=%p, event=%p", module, event);
    return -EINVAL;
  }
  uint32_t identity = event->identity;
  /* find corresponding sink port */
  mct_port_t* port = vpu_module_find_port_with_identity(module, MCT_PORT_SINK,
      identity);
  if (!port) {
    VPU_ERR("failed, no sink port found.with identity=0x%x", identity);
    return -EINVAL;
  }
  /* if port has a peer, post event to the upstream peer */
  if (!MCT_PORT_PEER(port)) {
    VPU_ERR("failed, no upstream peer found");
    return -EINVAL;
  }
  ret = mct_port_send_event_to_peer(port, event);
  if (ret == FALSE) {
    VPU_ERR("failed");
    return -EFAULT;
  }
  return 0;
}

int32_t vpu_module_process_downstream_event(mct_module_t* module,
    mct_event_t *event)
{
  int32_t rc;
  if (!module || !event) {
    VPU_ERR("failed, module=%p, event=%p", module, event);
    return -EINVAL;
  }
  uint32_t identity = event->identity;

  /* handle events based on type, if not handled, forward it downstream */
  switch (event->type) {
  case MCT_EVENT_MODULE_EVENT:
    switch (event->u.module_event.type) {
    case MCT_EVENT_MODULE_BUF_DIVERT:
      VPU_LOW("MCT_EVENT_MODULE_BUF_DIVERT: identity=0x%x", identity);
      rc = vpu_module_handle_buf_divert_event(module, event);
      if (rc < 0) {
        VPU_ERR("failed");
        return rc;
      }
      break;
    case MCT_EVENT_MODULE_SET_CHROMATIX_PTR:
      VPU_LOW("MCT_EVENT_MODULE_SET_CHROMATIX_PTR: identity=0x%x", identity);
      rc = vpu_module_handle_chromatix_ptr_event(module, event);
      if (rc < 0) {
        VPU_ERR("failed");
        return rc;
      }
      break;
    case MCT_EVENT_MODULE_STATS_AEC_UPDATE:
      VPU_LOW("MCT_EVENT_MODULE_STATS_AEC_UPDATE: identity=0x%x", identity);
      rc = vpu_module_handle_aec_update_event(module, event);
      if (rc < 0) {
        VPU_ERR("failed");
        return rc;
      }
      break;
    case MCT_EVENT_MODULE_ISP_OUTPUT_DIM:
      VPU_LOW("MCT_EVENT_MODULE_ISP_OUTPUT_DIM: identity=0x%x", identity);
      rc = vpu_module_handle_isp_out_dim_event(module, event);
      if(rc < 0) {
        VPU_ERR("failed");
        return rc;
      }
      break;
    case MCT_EVENT_MODULE_IFACE_REQUEST_PP_DIVERT:
      VPU_LOW("MCT_EVENT_MODULE_IFACE_REQUEST_PP_DIVERT: identity=0x%x", identity);
      rc = vpu_module_handle_iface_div_req_event(module, event);
      if(rc < 0) {
        VPU_ERR("failed");
        return rc;
      }
      break;
    default:
      rc = vpu_module_send_event_downstream(module, event);
      if (rc < 0) {
        VPU_ERR("failed, module_event_type=%d, identity=0x%x",
          event->u.module_event.type, identity);
        return -1;
      }
      break;
    }
    break;
  case MCT_EVENT_CONTROL_CMD:
    switch (event->u.ctrl_event.type) {
    case MCT_EVENT_CONTROL_STREAMON:
      VPU_LOW("MCT_EVENT_CONTROL_STREAMON: identity=0x%x", identity);
      rc = vpu_module_handle_streamon_event(module, event);
      if (rc < 0) {
        VPU_ERR("streamon failed");
        return rc;
      }
      break;
    case MCT_EVENT_CONTROL_STREAMOFF:
      VPU_LOW("MCT_EVENT_CONTROL_STREAMOFF: identity=0x%x", identity);
      rc = vpu_module_handle_streamoff_event(module, event);
      if (rc < 0) {
        VPU_ERR("streamoff failed");
        return rc;
      }
      break;
    case MCT_EVENT_CONTROL_SET_PARM: {
      VPU_LOW("MCT_EVENT_CONTROL_SET_PARM: identity=0x%x", identity);
      rc = vpu_module_handle_set_parm_event(module, event);
      if (rc < 0) {
        VPU_ERR("set_parm failed");
        return rc;
      }
      break;
    }
    default:
      rc = vpu_module_send_event_downstream(module, event);
      if (rc < 0) {
        VPU_ERR("failed, control_event_type=%d, identity=0x%x",
          event->u.ctrl_event.type, identity);
        return -1;
      }
      break;
    }
    break;
  default:
    VPU_ERR("invalid event type %d", event->type);
    return -EINVAL;
  }
  return 0;
}
int32_t vpu_module_process_upstream_event(mct_module_t* module,
    mct_event_t *event)
{
  int32_t rc;
  if (!module || !event) {
    VPU_ERR("failed, module=%p, event=%p", module, event);
    return -EINVAL;
  }
  uint32_t identity = event->identity;
  switch (event->type) {
  case MCT_EVENT_MODULE_EVENT:
    switch (event->u.module_event.type) {
    case MCT_EVENT_MODULE_BUF_DIVERT_ACK:
      VPU_LOW("MCT_EVENT_MODULE_BUF_DIVERT_ACK: identity=0x%x", identity);
      rc = vpu_module_handle_buf_divert_ack_event(module, event);
      if (rc < 0) {
        VPU_ERR("failed");
        return rc;
      }
      break;
    default:
      rc = vpu_module_send_event_upstream(module, event);
      if (rc < 0) {
        VPU_ERR("failed");
        return rc;
      }
      break;
    }
    break;
  default:
    /* all upstream events are module events, other events are invalid */
    VPU_ERR("invalid event type %d", event->type);
    break;
  }
  return 0;
}
