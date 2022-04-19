/*============================================================================
Copyright (c) 2013-2017,2019 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================*/

#include <stdlib.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "camera_dbg.h"
#include "cam_intf.h"
#include "mct_controller.h"
#include "mct_port.h"
#include "modules.h"
#include "iface_def.h"
#include "isp_event.h"
#include "iface.h"
#include "iface_util.h"
#include "mct_profiler.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif


/** port_iface_free_mem_func:
 *    @data: MCTL port
 *    @user_data: not used
 *
 *  This function runs in MCTL thread context.
 *
 *  This is a visitor function: frees a port private data.
 *
 *  Return:  TRUE
 **/
static boolean port_iface_free_mem_func(
  void *data,
  void *user_data __unused)
{
  mct_port_t *port = (mct_port_t *)data;

  assert(port != NULL);
  assert(port->private != NULL);

  if (port->port_private){
    free (port->port_private);
    port->port_private = NULL;
  }

  return TRUE;
}

/** port_iface_destroy_ports:
 *    @iface: iface instance
 *
 *  This function runs in MCTL thread context.
 *
 *  This function destroys module ports and frees their resources
 *
 *  Return: None
 **/
void port_iface_destroy_ports(iface_t *iface)
{
  if (iface->module->sinkports) {
    mct_list_traverse(iface->module->sinkports,
              port_iface_free_mem_func, NULL);
    mct_list_free_list(iface->module->sinkports);
    iface->module->sinkports= NULL;
  }
  if (iface->module->srcports) {
    mct_list_traverse(iface->module->srcports,
              port_iface_free_mem_func, NULL);
    mct_list_free_list(iface->module->srcports);
    iface->module->srcports= NULL;
  }
}

/** port_iface_send_event_to_peer:
 *    @data1: MCTL port to which peer will send event
 *    @user_data: MCTL event that will be send
 *
 *  This function runs in MCTL thread context.
 *
 *  This is a visitor function: sends event to peer port
 *
 *  Return: TRUE  - event sent successfuly
 **/
static boolean port_iface_send_event_to_peer(void *data1, void *user_data)
{
  mct_port_t *mct_port = (mct_port_t *)data1;
  mct_event_t *event = (mct_event_t *)user_data;
  iface_port_t *iface_port = (iface_port_t * )mct_port->port_private;
  iface_stream_t *stream = NULL;
  iface_sink_port_t *sink_port = NULL;
  iface_src_port_t *src_port = NULL;
  int i;
  uint32_t identity;
  boolean rc = FALSE;

  if (iface_port->state == IFACE_PORT_STATE_CREATED) {
    /* not used port */
    return TRUE;
  }

  if (mct_port->direction == MCT_PORT_SINK) {
    sink_port = &iface_port->u.sink_port;


    for (i = 0; i < IFACE_MAX_STREAMS; i++) {
      stream = &sink_port->streams[i];
      if (stream->stream_id == 0)
        continue;

      identity = pack_identity(stream->session_id, stream->stream_id);
      if (identity != (uint32_t)event->identity)
        continue;

      rc = mct_port->peer->event_func(mct_port->peer, event);
      if (rc == FALSE) {
        CDBG("%s: direction= %d event = %d rc = FALSE\n", __func__,
          mct_port->direction, event->type);
        return rc;
      }
      break;
    }


  } else if (mct_port->direction == MCT_PORT_SRC) {
    src_port = &iface_port->u.src_port;

    for (i = 0; i < IFACE_MAX_STREAMS; i++) {
       /*note! in src port we only save pointer from sink port*/
      stream = &src_port->streams[i];
      if (stream->stream_id == 0)
        continue;

      identity = pack_identity(stream->session_id, stream->stream_id);
      if (identity != (uint32_t)event->identity)
        continue;

      rc = mct_port->peer->event_func(mct_port->peer, event);
      if (rc == FALSE) {
        CDBG("%s: direction= %d event = %d rc = FALSE\n", __func__,
          mct_port->direction, event->type);
        return rc;
      }
      break;
    }

  } else
    return TRUE;

  return TRUE;
}

/** port_iface_forward_event_to_peer:
 *
 *  This function runs in MCTL thread context.
 *
 *  This function forwards event to peer port in desired direction
 *
 *  Return: TRUE  - event sent successfuly
 *          FALSE - Invalid port type
 **/
boolean port_iface_forward_event_to_peer(iface_t *iface,
  mct_port_t *mct_port, mct_event_t *event)
{
  /* if receive from sink forward to src's peer */
  if (mct_port->direction == MCT_PORT_SINK)
    return mct_list_traverse(iface->module->srcports,
                           port_iface_send_event_to_peer,
                           (void *)event);

  else if (mct_port->direction == MCT_PORT_SRC)
    return mct_list_traverse(iface->module->sinkports,
                           port_iface_send_event_to_peer,
                           (void *)event);
  else
    return FALSE;
}

/** port_iface_proc_mct_ctrl_cmd:
 *    @port: MCTL port to which peer will send event
 *    @event: MCTL event that contain command
 *
 *  This function runs in MCTL thread context.
 *
 *  This function processes a command event from MCTL
 *
 *  Return:  0 - Command executed and forwarded successfully
 *          -1 - Error
 **/
static int port_iface_proc_mct_ctrl_cmd(mct_port_t *mct_iface_port, mct_event_t *event)
{
  int ret = 0;
  boolean rc = FALSE;
  mct_event_control_t *ctrl;
  iface_port_t *iface_port;
  iface_t *iface;
  iface_session_t *session = NULL;

  if (!mct_iface_port || !event) {
    CDBG_ERROR ("%s: error: port or event is NULL: port %p, event %p", __func__,
      mct_iface_port, event );
    return -1;
  }

  ctrl = &event->u.ctrl_event;
  /* should not happen */
  assert(mct_iface_port->port_private != NULL);
  iface_port = (iface_port_t *)mct_iface_port->port_private;
  iface = (iface_t *)iface_port->iface;

  session = iface_util_get_session_by_id(iface,
    UNPACK_SESSION_ID(event->identity));
  if (!session) {
    CDBG_ERROR("%s: cannot find session %d\n", __func__,
      UNPACK_SESSION_ID(event->identity));
    return -1;
  }

  CDBG("%s: E, type = %d\n", __func__, ctrl->type);
  switch (ctrl->type) {
  case MCT_EVENT_CONTROL_STREAMON:
    ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_IFACE_STREAMON_FWD);
    ret = iface_sink_port_stream_config(iface, iface_port,
      UNPACK_STREAM_ID(event->identity),
      session);
    ATRACE_CAMSCOPE_END(CAMSCOPE_IFACE_STREAMON_FWD);
    if (ret < 0) {
      CDBG_ERROR("%s: iface_sink_port_stream_config error\n", __func__);
    } else {
      ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_IFACE_STREAMON_THREAD);
      ret = iface_streamon_to_thread(iface, mct_iface_port, session,
        UNPACK_STREAM_ID(event->identity), event);
      ATRACE_CAMSCOPE_END(CAMSCOPE_IFACE_STREAMON_THREAD);

    }
    break;

  case MCT_EVENT_CONTROL_STREAMOFF:
    ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_IFACE_STREAMOFF_THREAD);
    ret = iface_streamoff_to_thread(iface, iface_port, session,
      UNPACK_STREAM_ID(event->identity), event);
    ATRACE_CAMSCOPE_END(CAMSCOPE_IFACE_STREAMOFF_THREAD);
    if (ret != 0) {
      CDBG_ERROR("%s:iface_streamoff_to_thread failed, rc = -1\n",
        __func__);
    }

    ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_IFACE_STREAMOFF_FWD);
    rc = port_iface_forward_event_to_peer(
      iface, mct_iface_port, event);
    ATRACE_CAMSCOPE_END(CAMSCOPE_IFACE_STREAMOFF_FWD);
    if (rc == 0) {
      CDBG_ERROR("%s:forward MCT_EVENT_CONTROL_STREAMOFF evt error, rc = -1\n",
        __func__);
    }
    rc = iface_streamoff_post_isp(iface, iface_port, session);
    if (rc != 0) {
      CDBG_ERROR("%s:iface_streamoff_post_isp, rc = -1\n",
        __func__);
    }
    ret = 0;
    break;

  case MCT_EVENT_CONTROL_SET_PARM: {
    rc = port_iface_forward_event_to_peer(iface, mct_iface_port, event);
    if (rc == 0) {
      CDBG_ERROR("%s:%d forward_event error\n", __func__, __LINE__);
    }

    ret = iface_set_hal_param(iface, iface_port,
      session,
      UNPACK_STREAM_ID(event->identity),
      (mct_event_control_parm_t *)event->u.ctrl_event.control_event_data,
      session->sof_frame_id);

    if (ret < 0)
      CDBG_ERROR("%s:%d set hal param error\n", __func__, __LINE__);
  }
    break;

  case MCT_EVENT_CONTROL_LINK_INTRA_SESSION: {
      ret = iface_handle_link_intra_session(iface, iface_port,
        session, event);
      if (ret < 0) {
        CDBG_ERROR("%s: iface_handle_link_intra_session error\n", __func__);
      }
      rc = port_iface_forward_event_to_peer(iface, mct_iface_port, event);
      if (rc == 0) {
        CDBG_ERROR("%s:%d forward_event error\n", __func__, __LINE__);
      }
      break;
    }

  case MCT_EVENT_CONTROL_PARM_STREAM_BUF:
    /*
     * When offline ISP buffer directly goes through CPP, CPP
     * needs to know offline feature masks before buf divert.
     * iface forwards offline reprocess set param to cpp after
     * it completes reading the input raw buffer. By that time
     * CPP may have received buf divert and does not know
     * which all post proc flag are applied, hence added change
     * to forward event to cpp first.
    */
    rc = port_iface_forward_event_to_peer(
      iface, mct_iface_port, event);
    if (rc == 0) {
      CDBG_ERROR("%s:%d forward_event error\n", __func__, __LINE__);
      ret = -1;
    } else {
      ret = iface_set_hal_stream_param(iface, iface_port,
        session,
        UNPACK_STREAM_ID(event->identity), event);
    }
     break;

   case MCT_EVENT_CONTROL_UPDATE_BUF_INFO: {
     /* This is the event for deferred buf allocation. HAL sends total num of
          bufs initially. Iface reserves those many total bufs for that stream in
          bufq. In this event, iface gets buffer list one by one from HAL,
          iface maps the buffer list and enques it to kernel. Currently this
          is supported for snapshot buffer in ZSL*/
     CDBG("%s: E, identity = 0x%x, UPDATE_BUF_INFO\n", __func__,
       event->identity);
     if (FALSE == port_iface_forward_event_to_peer(
       iface, mct_iface_port, event)){
       CDBG_ERROR("%s: UPDATE_BUF_INFO, forward_event_to_peer error\n",
         __func__);
       ret = -1;
     } else {
       ret = iface_util_update_buf_info(iface, iface_port,
         session,
         UNPACK_STREAM_ID(event->identity), event);
       if (ret != 0) {
         CDBG_ERROR("%s: error in isp_update_buf_info, identity = 0x%x\n",
           __func__, event->identity);
       }
     }
     CDBG("%s: X, identity = 0x%x, UPDATE_BUF_INFO, ret = %d\n",
     __func__, event->identity, ret);
   }
     break;

   case MCT_EVENT_CONTROL_REMOVE_BUF_INFO: {
     /* This is the event for deferred buf allocation. HAL sends total num of
        bufs initially. Iface reserves those many total bufs for that stream in
        bufq. In this event, iface gets buffer list one by one from HAL,
        iface unmap the buffer list and dequeues it from kernel. */
     CDBG("%s: E, identity = 0x%x, REMOVE_BUF_INFO\n", __func__,
       event->identity);
     if (FALSE == port_iface_forward_event_to_peer(
       iface, mct_iface_port, event)){
       CDBG_ERROR("%s: REMOVE_BUF_INFO, forward_event_to_peer error\n",
         __func__);
       ret = -1;
     } else {
       ret = iface_util_remove_buf_info(iface, iface_port,
         session,
         UNPACK_STREAM_ID(event->identity), event);
       if (ret != 0) {
         CDBG_ERROR("%s: error in isp_update_buf_info, identity = 0x%x\n",
           __func__, event->identity);
       }
     }
     CDBG("%s: X, identity = 0x%x, UPDATE_BUF_INFO, ret = %d\n",
     __func__, event->identity, ret);
   }
     break;

  case MCT_EVENT_CONTROL_SET_SUPER_PARM:
    MCT_PROF_LOG_BEG(PROF_IFACE_SP);
    ret = iface_set_super_param(iface, iface_port,
      session,
      UNPACK_STREAM_ID(event->identity), event);
    MCT_PROF_LOG_END();
    if (ret == 0) {
      rc = port_iface_forward_event_to_peer(iface, mct_iface_port, event);
      if (rc == 0) {
        CDBG_ERROR("%s:%d forward_event error\n", __func__, __LINE__);
        ret = -1;
      }
    }
    break;

  case MCT_EVENT_CONTROL_SOF:
    MCT_PROF_LOG_BEG(PROF_IFACE_SOF);
    ATRACE_BEGIN("iface SOF");
    ret = iface_post_control_sof_to_thread(iface, iface_port,
      session,
      UNPACK_STREAM_ID(event->identity), event);
    MCT_PROF_LOG_END();
    if (ret) {
      CDBG_ERROR("%s:%d ifcae control_sof processing error \n",
        __func__, __LINE__);
      ret = 0;
    }

    rc = port_iface_forward_event_to_peer(iface, mct_iface_port, event);
    if (rc == FALSE) {
       CDBG_ERROR("%s:%d forward_event error\n", __func__, __LINE__);
       ret = -1;
    }
    ATRACE_END();
    break;

  case MCT_EVENT_CONTROL_STOP_ZSL_SNAPSHOT:
    session->cds_skip_disable = TRUE;
  case MCT_EVENT_CONTROL_START_ZSL_SNAPSHOT: {
    rc = port_iface_forward_event_to_peer(iface, mct_iface_port, event);
    if (rc == 0) {
      CDBG_ERROR("%s:%d forward_event error\n", __func__, __LINE__);
      ret = -1;
    }
    session->skip_cds_timeout = 0;
    }
        break;

  case MCT_EVENT_CONTROL_MASTER_INFO:
    if (session->ms_type == MS_TYPE_NONE)
      iface_util_map_cam_sync_mode_to_iface(
          session,
          *((cam_sync_mode_t *)ctrl->control_event_data));
    rc = port_iface_forward_event_to_peer(iface, mct_iface_port, event);
    if (rc == 0) {
      CDBG_ERROR("%s:%d forward_event error\n", __func__, __LINE__);
      ret = -1;
    }
    break;

#ifdef VIDIOC_MSM_ISP_DUAL_HW_MASTER_SLAVE_SYNC
  case MCT_EVENT_CONTROL_HW_SLEEP:
    ret = iface_session_hw_sleep(iface, iface_port, session, event);
    rc = port_iface_forward_event_to_peer(iface, mct_iface_port, event);
    if (rc == 0) {
      CDBG_ERROR("%s:%d forward_event error\n", __func__, __LINE__);
      ret = -1;
    }
    break;

  case MCT_EVENT_CONTROL_HW_WAKEUP:
    ret = iface_session_hw_wake(iface, iface_port, session, event);
    break;
#endif
  default:
    rc = port_iface_forward_event_to_peer(iface, mct_iface_port, event);
    if (rc == 0) {
      CDBG_ERROR("%s:%d forward_event error\n", __func__, __LINE__);
      ret = -1;
    }
    break;
  }

  rc = (ret == 0) ? TRUE : FALSE;
  CDBG("%s: X, type = %d, rc = %d\n", __func__, ctrl->type, rc);

  return rc;
}

/** port_iface_proc_module_event:
 *    @port: MCTL port to which peer will send event
 *    @event: MCTL event that will be processed
 *
 *  This function runs in MCTL thread context.
 *
 *  This function processes a module event from MCTL
 *
 *  Return: TRUE  - Event processed and forwarded successfully
 *          FALSE - Error
 **/
static boolean port_iface_proc_module_event(mct_port_t *mct_iface_port,
  mct_event_t *event)
{
  int ret = 0;
  uint32_t i = 0;
  boolean rc = FALSE;
  mct_event_module_t *mod_event;
  iface_port_t *iface_port;
  iface_t *iface = NULL;
  iface_session_t *session = NULL;

  if (!mct_iface_port || !event) {
    CDBG_ERROR ("%s: error: port or event is NULL: port %p, event %p", __func__,
      mct_iface_port, event );
    return FALSE;
  }

  mod_event = &event->u.module_event;
  /* should not happen */
  assert(mct_iface_port->port_private != NULL);
  iface_port = (iface_port_t *)mct_iface_port->port_private;
  iface = (iface_t *)iface_port->iface;

  session = iface_util_get_session_by_id(iface,
        UNPACK_SESSION_ID(event->identity));

  if (!session) {
    CDBG_ERROR("%s: error: session is NULL", __func__);
    return FALSE;
  }

  switch (mod_event->type) {
  case MCT_EVENT_MODULE_SET_STREAM_CONFIG:
    CDBG("%s: receive set stream config event!!!!!\n", __func__);
    /* IFACE uses a separate event to config ISP. */
    ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_IFACE_CFG_ISP);
    rc = port_iface_forward_event_to_peer(iface, mct_iface_port, event);
    if (rc == TRUE) {
      rc = iface_sink_port_copy_stream_config(iface, iface_port,
        UNPACK_STREAM_ID(event->identity),
        session,
        (sensor_out_info_t *)mod_event->module_event_data);
      if (rc < 0) {
        CDBG_ERROR("%s: iface_sink_port_stream_config error\n", __func__);
        ret = -1;
      }
    }
    ATRACE_CAMSCOPE_END(CAMSCOPE_IFACE_CFG_ISP);
    break;

  case MCT_EVENT_MODULE_STATS_AEC_UPDATE: {
    stats_update_t *stats_update = NULL;
    rc = port_iface_forward_event_to_peer(iface, mct_iface_port, event);
    if (rc == TRUE) {
      stats_update =
        (stats_update_t *)mod_event->module_event_data;
    } else {
      ret = -1;
      break;
    }

    if (stats_update != NULL) {
      /* Check new capture mode and destination */
      if (stats_update->aec_update.aec_dst &&
          !(stats_update->aec_update.aec_dst & STATS_UPDATE_DST_ISPIF)) {
          break;
      }
      if (session->hvx.enabled){
        iface_hvx_aec_update(&session->hvx, stats_update);
      }

      if (stats_update->aec_update.low_light_capture_update_flag)
        session->cds_skip_disable = FALSE;

      if ((stats_update->aec_update.est_state == AEC_EST_DONE) &&
        (session->cds_skip_disable == FALSE)) {
        /* Skip CDS toggling during preflash-off to Main flash on period */
        session->skip_cds_timeout = SKIP_CDS_FRAMES_THRESHOLD;
      }
      if (session->cds_feature_enable && !session->fast_aec_mode) {
        ret = iface_util_decide_cds_update(iface, session,
          UNPACK_STREAM_ID(event->identity), stats_update);
        if (ret < 0) {
          CDBG_ERROR("%s: iface_util_handle_aec_update error, rc = %d\n",
            __func__, ret);
        }
      }
    }
  }
  break;

  case MCT_EVENT_MODULE_UPDATE_STATS_SKIP: {
    rc = port_iface_forward_event_to_peer(iface, mct_iface_port, event);
    if (rc == TRUE) {
      enum msm_vfe_frame_skip_pattern *stats_skip_pattern =
        (enum msm_vfe_frame_skip_pattern *)mod_event->module_event_data;
      ret = iface_util_set_stats_frame_skip(iface,
        session, (int32_t *)stats_skip_pattern);
      if (ret < 0) {
        CDBG_ERROR("%s: iface_util_handle_bracketing_update error, rc = %d\n",
          __func__, ret);
      }
    }
  }
    break;

  case MCT_EVENT_MODULE_SET_CHROMATIX_PTR: {
    rc = port_iface_forward_event_to_peer(iface, mct_iface_port, event);
    if (rc == FALSE) {
      CDBG_ERROR("%s: MCT_EVENT_MODULE_SET_CHROMATIX_PTR fail, rc = -1\n",
        __func__);
      ret = -1;
    } else {
      modulesChromatix_t *chromatix_param =
        (modulesChromatix_t *)mod_event->module_event_data;
      ret = iface_util_set_chromatix(iface,
        session, UNPACK_STREAM_ID(event->identity), chromatix_param);
    }
  }
    break;
  case MCT_EVENT_MODULE_BRACKETING_UPDATE: {
    rc = port_iface_forward_event_to_peer(iface, mct_iface_port, event);
    if (rc == TRUE) {
      mct_bracketing_update_t *bracketing_update =
        (mct_bracketing_update_t *)mod_event->module_event_data;
      ret = iface_util_handle_bracketing_update(iface, session,
              bracketing_update);
      if (ret < 0) {
        CDBG_ERROR("%s: iface_util_handle_bracketing_update error, rc = %d\n",
          __func__, ret);
      }
    }
  }
    break;

  case MCT_EVENT_MODULE_REQUEST_FRAME_SKIP: {
    rc = port_iface_forward_event_to_peer(iface, mct_iface_port, event);
    if (rc == TRUE) {
      enum msm_vfe_frame_skip_pattern *skip_pattern =
        (enum msm_vfe_frame_skip_pattern *)mod_event->module_event_data;
      ret = iface_util_set_bracketing_frame_skip(iface, session,
              (int32_t *)skip_pattern);
      if (ret < 0) {
        CDBG_ERROR("%s: iface_util_handle_bracketing_update error, rc = %d\n",
          __func__, ret);
      }
    }
  }
    break;

  case MCT_EVENT_MODULE_REQUEST_SW_FRAME_SKIP: {
    iface_sw_frameskip *sw_skip_info =
      (iface_sw_frameskip *)mod_event->module_event_data;

    sw_skip_info->session_id = session->session_id;
    ret = iface_sw_skip_config_to_thread(iface, sw_skip_info);
    if (ret >= 0) {
      /* we need to propagate only in case of success otherwise we can confuse
       * subsequent modules that we will skip frames. */
      rc = port_iface_forward_event_to_peer(iface, mct_iface_port, event);
      if (rc == FALSE) {
        CDBG_ERROR("%s: SW frame skip forward event error, rc = %d\n",
          __func__, ret);
        ret = -1;
      }
    } else {
      /* valid case - ISP tasks may be busy and may need to retry command */
      CDBG("%s: MCT_EVENT_MODULE_REQUEST_SW_FRAME_SKIP error, rc = %d\n",
        __func__, ret);
    }
  }
    break;

  case MCT_EVENT_MODULE_BUF_DIVERT_ACK: {
    isp_buf_divert_ack_t *buf_divert_ack = (isp_buf_divert_ack_t*)
      mod_event->module_event_data;
    CDBG("%s: receive buf divert ack, identity = 0x%x, bayerdata %d\n",
      __func__, event->identity, buf_divert_ack->bayerdata);
    ret = iface_util_divert_ack(iface,
      session, UNPACK_STREAM_ID(event->identity),
      buf_divert_ack->buf_idx, buf_divert_ack->is_buf_dirty,
      buf_divert_ack->bayerdata, buf_divert_ack->buffer_access);
  }
    break;

  case MCT_EVENT_MODULE_BAYERPROC_OUTPUT_DIM:
    ret = iface_util_save_input_dim(iface, event);
    if (ret) {
      CDBG_ERROR("%s: error in iface_util_save_input_dim", __func__);
    }
   break;
   case MCT_EVENT_MODULE_OUTPUT_BUFF_LIST:
    ret = iface_util_request_image_bufq_input(iface, event);
    if (ret) {
       CDBG_ERROR("%s: error in request input image buffer", __func__);
    }
    break;
   case MCT_EVENT_MODULE_BUF_DIVERT:
      ret = iface_util_handle_buffer_divert(iface, event);
      if (ret) {
        CDBG_ERROR("%s: error iface_util_handle_buffer_divert", __func__);
      }
    break;
   case MCT_EVENT_MODULE_RAW_STATS_DIVERT_ACK: {
    CDBG("%s: receive stats buf ack, identity = 0x%x\n",
      __func__, event->identity);
    iface_raw_stats_buf_info_t *stats_buf_ack = (iface_raw_stats_buf_info_t*)
      mod_event->module_event_data;
    ret = iface_util_stats_ack(iface,
      session, UNPACK_STREAM_ID(event->identity),
      stats_buf_ack);
  }
    break;
  case MCT_EVENT_MODULE_SENSOR_META_CONFIG: {
    sensor_meta_data_t *sensor_meta_cfg = (sensor_meta_data_t *)
      mod_event->module_event_data;
    for (i = 0; i < sensor_meta_cfg->num_meta; i++) {
    CDBG("%s: identity %x num_meta %d is valid %d\n", __func__,
      event->identity, sensor_meta_cfg->num_meta,
        sensor_meta_cfg->sensor_meta_info[i].is_valid);
      CDBG("%s: i %d dim- %d x %d fmt %d", __func__, i,
        sensor_meta_cfg->sensor_meta_info[i].dim.width,
        sensor_meta_cfg->sensor_meta_info[i].dim.height,
        sensor_meta_cfg->sensor_meta_info[i].fmt);
    }
    iface_util_config_meta(iface, iface_port,
      session,
      UNPACK_STREAM_ID(event->identity), sensor_meta_cfg);
    rc = port_iface_forward_event_to_peer(iface, mct_iface_port, event);
    if (rc == FALSE) {
      CDBG_ERROR("%s:%d rc %d\n", __func__, __LINE__, rc);
    }
  }
    break;
  case MCT_EVENT_MODULE_SENSOR_PDAF_CONFIG: {
    pdaf_sensor_native_info_t  *pdaf_information =
     (pdaf_sensor_native_info_t *)mod_event->module_event_data;
#ifdef PDAF_DBG
    CDBG_ERROR("<pdaf_dbg> H %d W %d offsetX = %d offsetY %d count %d",
      pdaf_information->block_pattern.block_dim.height,
      pdaf_information->block_pattern.block_dim.width,
      pdaf_information->block_pattern.pd_offset_horizontal,
      pdaf_information->block_pattern.pd_offset_vertical,
      pdaf_information->block_pattern.pix_count);
    for (i = 0 ; i < pdaf_information->block_pattern.pix_count; i++){
      CDBG_ERROR("PDAF_CONFIG i= %d  X = %d y %d", i,
        pdaf_information->block_pattern.pix_coords[i].x,
        pdaf_information->block_pattern.pix_coords[i].y);
    }
#endif
    iface_util_config_pdaf_meta(iface, iface_port,
      session,
      UNPACK_STREAM_ID(event->identity), pdaf_information);
    rc = port_iface_forward_event_to_peer(iface, mct_iface_port, event);
    if (rc == FALSE) {
      CDBG_ERROR("%s:%d rc %d\n", __func__, __LINE__, rc);
    }
  }
  break;

  case MCT_EVENT_MODULE_ISP_CDS_REQUEST: {
    isp_cds_request_t *isp_cds_request =
      (isp_cds_request_t *)mod_event->module_event_data;

    if (isp_cds_request->cds_request_done == TRUE) {
      ret = iface_util_cds_request_done(iface,
        session, isp_cds_request);
    }
  }
    break;

  case MCT_EVENT_MODULE_SET_FAST_AEC_CONVERGE_MODE: {
    mct_fast_aec_mode_t *fast_aec =
      (mct_fast_aec_mode_t*)
      mod_event->module_event_data;;
    CDBG_HIGH("%s: FASTAEC identity = 0x%x enable= %d\n",
      __func__, event->identity, fast_aec->enable);
     rc = port_iface_forward_event_to_peer(iface, mct_iface_port, event);
    if (rc == TRUE) {
      iface_fast_aec_mode(iface,  UNPACK_STREAM_ID(event->identity),
        session, fast_aec);
    }
  }
    break;

  case MCT_EVENT_MODULE_FRAME_SKIP_NOTIFY: {
    uint32_t skip_frame_id;
    rc = port_iface_forward_event_to_peer(iface, mct_iface_port, event);
    if (rc == TRUE) {
      skip_frame_id = *(uint32_t *)mod_event->module_event_data;
      ret = iface_util_handle_frame_skip_event(iface,
        session, UNPACK_STREAM_ID(event->identity),
        skip_frame_id);
    } else {
      ret = -1;
    }
  }
    break;

  case MCT_EVENT_MODULE_SET_PREFERRED_STREAMS_MAPPING:
    ret = iface_util_set_preferred_mapping(iface, event);
    if (ret == FALSE) {
      CDBG_ERROR("%s:%d failed: isp_util_set_preferred_mapping type %d",
        __func__, __LINE__, mod_event->type);
    }
    break;

  case MCT_EVENT_MODULE_SET_SENSOR_OUTPUT_INFO:
    iface_set_sensor_output(iface, event);
    rc = port_iface_forward_event_to_peer(iface, mct_iface_port, event);
    if (rc == FALSE) {
      CDBG_ERROR("%s:%d rc %d\n", __func__, __LINE__, rc);
    }
    break;

  case MCT_EVENT_MODULE_STATS_AWB_UPDATE:
    rc = port_iface_forward_event_to_peer(iface, mct_iface_port, event);
    if (rc == TRUE) {
      stats_update_t *stats_update =
        (stats_update_t *)mod_event->module_event_data;
      if (session->hvx.enabled){
        iface_hvx_awb_update(&session->hvx, stats_update);
      }
    }
    break;
  case MCT_EVENT_MODULE_SENSOR_QUERY_OUTPUT_SIZE:
    rc = iface_util_set_buf_allignment(iface, event);
    if (rc == TRUE)
      rc = port_iface_forward_event_to_peer(iface, mct_iface_port, event);
    if (rc == TRUE) {
        sensor_isp_stream_sizes_t *sensor_isp_info =
            (sensor_isp_stream_sizes_t *)mod_event->module_event_data;
        for (i = 0; i< MAX_NUM_STREAMS; i++) {
            if (sensor_isp_info->stream_sz_plus_margin[i].width &&
                sensor_isp_info->postprocess_mask[i]) {
                sensor_isp_info->stream_sz_plus_margin[i].width =
                    CEILING16(sensor_isp_info->stream_sz_plus_margin[i].width);
                sensor_isp_info->stream_sz_plus_margin[i].height =
                    CEILING4(sensor_isp_info->stream_sz_plus_margin[i].height);
            }
        }
    }
   break;
  default:
    rc = port_iface_forward_event_to_peer(
             iface, mct_iface_port, event);
    if (rc == FALSE) {
      CDBG("%s: port_iface_forward_event_to_peer error\n", __func__);
      ret = -1;
    }
    break;
  }

  rc = (ret == 0)? TRUE : FALSE;

  return rc;
}

/** port_iface_event_func:
 *    @port: MCTL port to which peer will send event
 *    @event: MCTL event that will be processed
 *
 *  This function runs in MCTL thread context.
 *
 *  This function processes a MCTL event
 *
 *  Return: TRUE  - Event processed and forwarded successfully
 *          FALSE - Error
 **/
static boolean port_iface_event_func(mct_port_t *mct_port, mct_event_t *event)
{
  boolean rc = FALSE;

  switch (event->type) {
  case MCT_EVENT_CONTROL_CMD:
    /* MCT ctrl event */
    rc = port_iface_proc_mct_ctrl_cmd(mct_port, event);
    break;

  case MCT_EVENT_MODULE_EVENT:
    /* Event among modules */
    rc = port_iface_proc_module_event(mct_port, event);
    break;

  default:
    rc = FALSE;
  }

  return rc;
}

/** port_iface_int_link_func:
 *    @identity: identity
 *    @port: MCTL port to be linked
 *
 *  This function runs in MCTL thread context.
 *
 *  This function implements port internal link method of port object - dummy
 *
 *  Return: NULL
 **/
static mct_list_t *port_iface_int_link_func(
  unsigned int identity __unused,
  mct_port_t  *port __unused)
{
  /* dummy function. */
  return NULL;
}

/** port_iface_ext_link_func:
 *    @identity: identity
 *    @port: MCTL port to be linked
 *    @peer: MCTL port to which port will link
 *
 *  This function runs in MCTL thread context.
 *
 *  This function implements port external link method of port object
 *
 *  Return: TRUE  - Ports linked succsessfuly
 *          FALSE - Error
 **/
static boolean port_iface_ext_link_func(unsigned int identity, mct_port_t* port,
  mct_port_t *peer)
{
  boolean rc = TRUE;
  int ret = 0;
  iface_port_t *linking_iface_port = (iface_port_t *)port->port_private;
  iface_t *iface = (iface_t *)linking_iface_port->iface;

  CDBG("%s: E, identity = 0x%x, port = %p\n", __func__, identity, port);

  pthread_mutex_lock(&iface->mutex);
  if (port->direction == MCT_PORT_SRC)
    ret = iface_ext_link_src_port(iface, linking_iface_port, peer,
            UNPACK_SESSION_ID(identity), UNPACK_STREAM_ID(identity));
  else
    ret = iface_ext_link_sink_port(iface, linking_iface_port, peer,
            UNPACK_SESSION_ID(identity), UNPACK_STREAM_ID(identity));

  pthread_mutex_unlock(&iface->mutex);

  rc = (ret == 0)? TRUE : FALSE;

  CDBG("%s: X, rc = %d, identity = 0x%x, port = %p\n", __func__, rc, identity,
    port);

  return rc;
}

/** port_iface_ext_unlink_func:
 *    @identity: identity
 *    @port: MCTL port to be unlinked
 *    @peer: MCTL port to which port is linked
 *
 *  This function runs in MCTL thread context.
 *
 *  This function implements port unlink method of port object
 *
 *  Return: None
 **/
static void port_iface_ext_unlink_func(unsigned int identity, mct_port_t *port,
  mct_port_t *peer)
{
  int ret = 0;
  iface_port_t *unlinking_iface_port = (iface_port_t *)port->port_private;
  uint32_t params_id;
  iface_t *iface = (iface_t *)unlinking_iface_port->iface;

  CDBG("%s: E, identity = 0x%x, port = %p, direction = %d\n",
       __func__, identity, port, port->direction);

  pthread_mutex_lock(&iface->mutex);
  if (port->direction == MCT_PORT_SINK)
    ret = iface_ext_unlink_sink_port(iface, unlinking_iface_port, peer,
            UNPACK_SESSION_ID(identity), UNPACK_STREAM_ID(identity));
  else
    ret = iface_ext_unlink_src_port(iface, unlinking_iface_port, peer,
            UNPACK_SESSION_ID(identity), UNPACK_STREAM_ID(identity));

  pthread_mutex_unlock(&iface->mutex);

  CDBG("%s: X, ret = %d, identity = 0x%x, port = %p, direction = %d\n",
       __func__, ret, identity, port, port->direction);

}


/** port_iface_set_caps_func:
 *    @port: MCTL port
 *    @caps: MCTL port capabilities
 *
 *  This function runs in MCTL thread context.
 *
 *  This function implements port set capabilities method of port object
 *
 *  Return: TRUE
 **/
static boolean port_iface_set_caps_func(
  mct_port_t      *port __unused,
  mct_port_caps_t *caps __unused)
{
  boolean rc = TRUE;
  return rc;
}

/** port_iface_check_caps_reserve_func:
 *    @port: MCTL port
 *    @peer_caps: MCTL port capabilities
 *    @pstream_info: stream info
 *
 *  This function runs in MCTL thread context.
 *
 *  This function checks and tries to reserve port according given capabilities
 *
 *  Return: TRUE  - ports reserved succesfuly
 *          FALSE - Error
 **/
static boolean port_iface_check_caps_reserve_func(mct_port_t *port,
  void *peer_caps, void *pstream_info)
{
  mct_stream_info_t *stream_info = pstream_info;
  int i = 0, ret = 0;
  boolean rc = FALSE;
  iface_port_t *reserving_iface_port = (iface_port_t *)port->port_private;
  iface_t *iface = (iface_t *)reserving_iface_port->iface;

  CDBG("%s: E, identity = 0x%x, port = %p, direction = %d\n",
    __func__, stream_info->identity, port, port->direction);
  pthread_mutex_lock(&iface->mutex);
  if (port->direction == MCT_PORT_SINK) {
    ret = iface_reserve_sink_port(iface, reserving_iface_port,
      (sensor_src_port_cap_t *)peer_caps, stream_info,
      UNPACK_SESSION_ID(stream_info->identity),
      UNPACK_STREAM_ID(stream_info->identity));
  } else {
    ret = iface_reserve_src_port(iface, reserving_iface_port, stream_info,
      UNPACK_SESSION_ID(stream_info->identity),
      UNPACK_STREAM_ID(stream_info->identity));
  }

  if(port->direction == MCT_PORT_SRC) {
    switch(reserving_iface_port->u.src_port.caps.sensor_cap.sensor_cid_ch[0].fmt) {
      case CAM_FORMAT_YUV_RAW_8BIT_YUYV: {
        port->caps.u.frame.format_flag = MCT_PORT_CAP_FORMAT_YCBCR |
            MCT_PORT_CAP_FORMAT_YCBYCR;
      }
        break;
      case CAM_FORMAT_YUV_RAW_8BIT_YVYU: {
        port->caps.u.frame.format_flag = MCT_PORT_CAP_FORMAT_YCBCR |
            MCT_PORT_CAP_FORMAT_YCRYCB;
      }
        break;
      case CAM_FORMAT_YUV_RAW_8BIT_UYVY: {
        port->caps.u.frame.format_flag = MCT_PORT_CAP_FORMAT_YCBCR |
            MCT_PORT_CAP_FORMAT_CBYCRY;
      }
        break;
      case CAM_FORMAT_YUV_RAW_8BIT_VYUY: {
        port->caps.u.frame.format_flag = MCT_PORT_CAP_FORMAT_YCBCR |
            MCT_PORT_CAP_FORMAT_CRYCBY;
      }
        break;
      default:
        if(reserving_iface_port->u.src_port.caps.use_pix)
          port->caps.u.frame.format_flag |= MCT_PORT_CAP_FORMAT_YCBCR;
        else
          port->caps.u.frame.format_flag |= MCT_PORT_CAP_FORMAT_BAYER;
        break;
    }
  }

  pthread_mutex_unlock(&iface->mutex);

  rc = (ret == 0)? TRUE : FALSE;

  return rc;
}

/** port_iface_check_caps_unreserve_func:
 *    @port: MCTL port
 *    @identity: stream identity
 *
 *  This function runs in MCTL thread context.
 *
 *  This function frees reserved port according given capabilities
 *
 *  Return: TRUE  - ports unreserved succesfuly
 *          FALSE - Error
 **/
static boolean port_iface_check_caps_unreserve_func(mct_port_t *port,
  unsigned int identity)
{
  boolean rc = TRUE;
  int i = 0, ret = 0;
  iface_port_t *unreserving_iface_port = (iface_port_t *)port->port_private;
  iface_t *iface = (iface_t *)unreserving_iface_port->iface;
  uint32_t params_id;

  CDBG("%s:  E, identity = 0x%x, port = %p, direction = %d\n",
       __func__, identity, port, port->direction);

  pthread_mutex_lock(&iface->mutex);
  if (port->direction == MCT_PORT_SINK)
    ret = iface_unreserve_sink_port(iface, unreserving_iface_port,
            UNPACK_SESSION_ID(identity), UNPACK_STREAM_ID(identity));
  else
    ret = iface_unreserve_src_port(iface, unreserving_iface_port,
            UNPACK_SESSION_ID(identity), UNPACK_STREAM_ID(identity));

  pthread_mutex_unlock(&iface->mutex);
  rc = (ret == 0)? TRUE : FALSE;
  CDBG("%s: X, rc = %d, identity = 0x%x, port = %p, direction = %d\n",
       __func__, rc, identity, port, port->direction);

  return rc;
}

/** iface_overwrite_port_funcs:
 *    @port: MCTL port
 *    @private_data: port private data
 *
 *  This function runs in MCTL thread context.
 *
 *  This function replaces port methods with custom ones
 *
 *  Return: None
 **/
static void iface_overwrite_port_funcs(mct_port_t *port, void *private_data)
{
  port->port_private = private_data;
  port->event_func = port_iface_event_func;
  port->int_link = port_iface_int_link_func;
  port->ext_link = port_iface_ext_link_func;
  port->un_link = port_iface_ext_unlink_func;
  port->set_caps = port_iface_set_caps_func;
  port->check_caps_reserve = port_iface_check_caps_reserve_func;
  port->check_caps_unreserve = port_iface_check_caps_unreserve_func;
}

/** port_iface_add_ports:
 *    @num_ports: number of ports
 *    @is_sink: true if ports are sink
 *
 *  This function runs in MCTL thread context.
 *
 *  This function adds number of sink or src ports to iface
 *  module
 *
 *  Return: 0 - Success
 *         -1 - Error while adding ports
 **/
static int port_iface_add_ports(iface_t *iface, int num_ports, int is_sink)
{
  int i;
  int rc = 0;
  iface_port_t *iface_port = NULL;
  char port_name[32];
  mct_port_t *mct_port = NULL;
  mct_module_t *iface_module = iface->module;

  CDBG("%s: E\n", __func__);
  for (i = 0; i < num_ports; i++) {
    iface_port = malloc(sizeof(iface_port_t));
    if (!iface_port) {
      CDBG_ERROR("%s: cannot malloc iface sink port\n", __func__);
      rc = -ENOMEM;
      goto end;
    }

    memset(iface_port, 0, sizeof(iface_port_t));
    if (is_sink)
      snprintf(port_name, sizeof(port_name), "iface_sink%d", i);
    else
      snprintf(port_name, sizeof(port_name), "iface_src%d", i);

    mct_port = mct_port_create(port_name);
    if (!mct_port) {
      CDBG_ERROR("%s: mct_port_create error\n", __func__);
      free (iface_port);
      rc = -ENOMEM;
      goto end;
    }

    iface_port->port = mct_port;

    if (is_sink)
      mct_port->direction = MCT_PORT_SINK;
    else
      mct_port->direction = MCT_PORT_SRC;

    mct_module_add_port(iface_module, mct_port);
    if (is_sink) {
      mct_port->caps.port_caps_type = MCT_PORT_CAPS_OPAQUE;
      mct_port->caps.u.data = (void *)&iface_port->u.sink_port.sensor_cap;
    } else {
      mct_port->caps.port_caps_type = MCT_PORT_CAPS_FRAME;
      mct_port->caps.u.data = (void *)&iface_port->u.src_port.caps;
    }

    iface_overwrite_port_funcs(iface_port->port, (void *)iface_port);
    iface_port->iface = (void *)iface;
    iface_port->state = IFACE_PORT_STATE_CREATED;
  }

end:
  return rc;
}

/** port_iface_create_ports:
 *    @iface: iface instance
 *
 *  This function runs in MCTL thread context.
 *
 *  This function create all ports to iface module
 *
 *  Return: 0 - Success
 *         -1 - Error while adding ports
 **/
int port_iface_create_ports(iface_t *iface)
{
  int rc = 0;
  int is_sink = 1;

  rc = port_iface_add_ports(iface, IFACE_SINK_PORTS_NUM, is_sink);
  if (rc == 0) {
    is_sink= 0;
    rc = port_iface_add_ports(iface, IFACE_SRC_PORTS_NUM, is_sink);
  }

end:
  if (rc < 0)
    port_iface_destroy_ports(iface);

  return rc;
}
