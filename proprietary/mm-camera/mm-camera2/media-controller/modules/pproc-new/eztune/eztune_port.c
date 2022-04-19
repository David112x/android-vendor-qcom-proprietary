/*============================================================================

 Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdint.h>
#include <stdbool.h>
#include "pp_utils.h"
#include "base_module.h"
#include "base_port.h"
#include "eztune_module.h"
#include "eztune_port.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "mm-camera-eztune"

// declare these here so they are not public outside of this library
extern void eztune_notify_streamon(mct_module_t *module, mct_event_t *event);
extern void eztune_notify_streamoff(mct_module_t *module, mct_event_t *event);
extern void eztune_notify_set_chromatix_ptr(mct_module_t *module, mct_event_t *event);
extern void eztune_notify_metadata_frame(mct_module_t *module,
                                         metadata_buffer_t *metadata);
extern void eztune_notify_preview_frame(mct_module_t *module,
                                        mct_stream_t *stream,
                                        isp_buf_divert_t *p_buf_divert);
extern void eztune_notify_snapshot_frame(mct_module_t *module,
                                         void *params);

static int32_t eztune_port_handle_buf_divert(mct_port_t *port,
                                             mct_event_t *event)
{
  mct_event_module_t *p_mod_event = &event->u.module_event;
  isp_buf_divert_t *p_buf_divert = p_mod_event->module_event_data;
  mct_module_t *module = MCT_MODULE_CAST(MCT_OBJECT_PARENT(port)->data);
  mct_stream_t *stream = eztune_get_stream_by_id(event->identity);

  if(stream && stream->streaminfo.stream_type == CAM_STREAM_TYPE_PREVIEW) {
    eztune_notify_preview_frame(module, stream, p_buf_divert);
  }
  p_buf_divert->ack_flag = 1;
  p_buf_divert->is_buf_dirty = 0;

  return 0;
}

boolean eztune_port_src_event_func(mct_port_t *port, mct_event_t *event)
{
  int32_t rc=0;

  if(event->direction != MCT_EVENT_UPSTREAM) {
    PP_ERR("invalid");
    return mct_port_send_event_to_peer(port, event);
  }

  if (event->type == MCT_EVENT_MODULE_EVENT &&
      event->u.module_event.type == MCT_EVENT_MODULE_BUF_DIVERT_ACK) {
      rc = base_port_handle_buf_divert_ack(port, event);
  } else {
    base_port_fwd_event_to_intlinks(port, event);
  }

  return (rc >= 0);
}

boolean eztune_port_sink_event_func(mct_port_t *port, mct_event_t *event)
{
  int32_t rc=0;
  boolean fwd_event = true;
  mct_module_t *module = MCT_MODULE_CAST(MCT_OBJECT_PARENT(port)->data);

  if(event->direction != MCT_EVENT_DOWNSTREAM) {
    return mct_port_send_event_to_peer(port, event);
  }

  switch(event->type) {

    case MCT_EVENT_MODULE_EVENT: {
      mct_event_module_t *mod_event = &event->u.module_event;

      switch(mod_event->type) {
        case MCT_EVENT_MODULE_BUF_DIVERT: {
          rc = eztune_port_handle_buf_divert(port, event);
          fwd_event = true;
          break;
        }
        case MCT_EVENT_MODULE_QUERY_DIVERT_TYPE: {
          uint32_t *divert_mask = (uint32_t *)mod_event->module_event_data;
          *divert_mask |= PPROC_DIVERT_PROCESSED;
          break;
        }
        case MCT_EVENT_MODULE_SET_CHROMATIX_PTR: {
          eztune_notify_set_chromatix_ptr(module, event);
          break;
        }
        default:
          break;
      } //switch(event->u.module_event.type)
      break;
    } //case MCT_EVENT_MODULE_EVENT:

    case MCT_EVENT_CONTROL_CMD: {
      mct_event_control_t *ctrl_event = &event->u.ctrl_event;

      switch(ctrl_event->type) {
        case MCT_EVENT_CONTROL_STREAMON: {
          eztune_notify_streamon(module, event);
          break;
        }
        case MCT_EVENT_CONTROL_STREAMOFF: {
          eztune_notify_streamoff(module, event);
          break;
        }
        case MCT_EVENT_CONTROL_SET_PARM: {
          mct_event_control_parm_t *ctrl_parm =
              (mct_event_control_parm_t*)ctrl_event->control_event_data;
          if(ctrl_parm != NULL && ctrl_parm->type == CAM_INTF_PARM_INT_EVT) {
            eztune_notify_snapshot_frame(module, ctrl_parm->parm_data);
            fwd_event = false;
          }
          break;
        } // case MCT_EVENT_CONTROL_SET_PARM:
        case MCT_EVENT_CONTROL_METADATA_UPDATE: {
          metadata_buffer_t *metadata =
              (metadata_buffer_t*)ctrl_event->control_event_data;
          if(metadata != NULL && module != NULL) {
            eztune_notify_metadata_frame(module, metadata);
            fwd_event = false;
          }
          break;
        } //case MCT_EVENT_CONTROL_METADATA_UPDATE
        default:
          break;
      } //switch(event->u.ctrl_event.type)
      break;
    } //case MCT_EVENT_CONTROL_CMD:

    default:
      break;
  } //switch(event->type)

  if(fwd_event) {
    rc = base_port_fwd_event_to_intlinks(port, event);
  }
  return (rc >= 0);
}

