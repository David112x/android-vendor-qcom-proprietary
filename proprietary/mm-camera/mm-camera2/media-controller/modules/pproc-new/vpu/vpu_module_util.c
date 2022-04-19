/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#include "vpu_module.h"
#include "vpu_port.h"
#include "vpu_log.h"


boolean ack_find_func(void* data, void* userdata)
{
  if(!data || !userdata) {
    VPU_ERR("failed, data=%p, userdata=%p",
              data, userdata);
    return FALSE;
  }
  vpu_module_ack_t* vpu_ack = (vpu_module_ack_t*) data;
  uint32_t idx = *(uint32_t *)userdata;
  if(vpu_ack->isp_buf_divert_ack.buf_idx == idx) {
    return TRUE;
  }
  return FALSE;
}

vpu_module_ack_t* vpu_module_find_ack_from_list(vpu_module_ctrl_t *ctrl,
  int32_t buf_idx)
{
  mct_list_t *templist;
  templist = mct_list_find_custom(ctrl->ack_list.list, &buf_idx, ack_find_func);
  if(templist) {
    return (vpu_module_ack_t*)(templist->data);
  }
  return NULL;
}

static boolean find_port_with_identity_find_func(void *data, void *user_data)
{
  if(!data || !user_data) {
    VPU_ERR("failed, data=%p, user_data=%p", data, user_data);
    return FALSE;
  }
  mct_port_t *port = (mct_port_t*) data;
  uint32_t identity = *(uint32_t*) user_data;

  vpu_port_data_t *port_data = (vpu_port_data_t *) MCT_OBJECT_PRIVATE(port);
  uint32_t i;
  for(i=0; i<VPU_MAX_STREAMS_PER_PORT; i++) {
    if(port_data->stream_data[i].port_state != VPU_PORT_STATE_UNRESERVED &&
        port_data->stream_data[i].identity == identity) {
      return TRUE;
    }
  }
  return FALSE;
}

mct_port_t* vpu_module_find_port_with_identity(mct_module_t *module,
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
    VPU_ERR("failed, invalid port_direction=%d", dir);
    return NULL;
  }
  return port;
}

int32_t vpu_module_get_params_for_identity(vpu_module_ctrl_t* ctrl,
  uint32_t identity, vpu_module_session_params_t** session_params,
  vpu_module_stream_params_t** stream_params)
{
  if(!ctrl || !session_params || !stream_params) {
    VPU_ERR("failed, ctrl=%p, session_params=%p, stream_params=%p",
      ctrl, session_params, stream_params);
    return -1;
  }
  uint32_t session_id;
  uint32_t i,j;
  boolean success = FALSE;
  session_id = VPU_GET_SESSION_ID(identity);
  for(i=0; i < VPU_MODULE_MAX_SESSIONS; i++) {
    if(ctrl->session_params[i]) {
      if(ctrl->session_params[i]->session_id == session_id) {
        for(j=0; j < VPU_MODULE_MAX_STREAMS; j++) {
          if(ctrl->session_params[i]->stream_params[j]) {
            if(ctrl->session_params[i]->stream_params[j]->identity ==
                identity) {
              *stream_params = ctrl->session_params[i]->stream_params[j];
              *session_params = ctrl->session_params[i];
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
    VPU_ERR("failed, identity=0x%x", identity);
    return -1;
  }
  return 0;
}

int32_t vpu_module_get_params_for_session_id(vpu_module_ctrl_t* ctrl,
  uint32_t session_id, vpu_module_session_params_t** session_params)
{
  uint32_t i = 0;
  if (!ctrl || !session_params) {
    VPU_ERR("failed, ctrl=%p, session_params=%p",
      ctrl, session_params);
    return -1;
  }
  *session_params = NULL;
  for (i = 0; i < VPU_MODULE_MAX_SESSIONS; i++) {
    if (ctrl->session_params[i]) {
      if (ctrl->session_params[i]->session_id == session_id) {
        *session_params = ctrl->session_params[i];
        break;
      }
    }
  }
  if(!(*session_params)) {
    VPU_ERR("failed, session_id=0x%x", session_id);
    return -1;
  }
  return 0;
}
