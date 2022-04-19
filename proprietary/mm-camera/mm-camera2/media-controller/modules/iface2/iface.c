/*============================================================================
Copyright (c) 2013-2016, 2018-2020 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/media.h>
#include "camera_dbg.h"
#include "cam_types.h"
#include "cam_intf.h"
#include "mct_controller.h"
#include "modules.h"
#include "iface_def.h"
#include "iface.h"
#include "iface_util.h"
#include "iface_buf_mgr.h"
#include "ispif/iface_ispif.h"
#include "axi/iface_axi.h"
#include "mct_profiler.h"
#include "iface_hvx.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

/*#define IFACE_DEBUG */
#ifdef IFACE_DEBUG
#undef CDBG
#define CDBG ALOGE
#undef CDBG_ERROR
#define CDBG_ERROR ALOGE
#undef CDBG_HIGH
#define CDBG_HIGH ALOGE
#endif

static int iface_restore_hal_param(iface_t *iface, iface_session_t *session,
  mct_queue_t *parm_q, pthread_mutex_t *parm_q_lock, uint32_t frame_id);

/** iface_update_sw_frame_skip:
 *
 *@iface : iface instance
 *@sw_skip_info : stream info
 *
 * Updates requested SW frame ski configuration.
 *
 *  Return: 0: Success
 *          Negative: Error
 **/
int iface_update_sw_frame_skip(iface_t *iface,
  iface_sw_frameskip *sw_skip_info)
{
  int isp, rc = 0;
  iface_param_frame_skip_pattern_t frame_skip;
  /* reset Hw skip pattern */
  enum msm_vfe_frame_skip_pattern skip_pattern = NO_SKIP;
  uint32_t temp_min_id = 0;
  uint32_t temp_max_id = 0;


  if (!iface || !sw_skip_info) {
    CDBG_ERROR("%s: iface %p, sw_skip_info %p, rc = -1\n",
      __func__, iface, sw_skip_info);
    return -1;
  }

  iface_session_t *session =
    iface_util_get_session_by_id(iface, sw_skip_info->session_id);
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n",
      __func__, sw_skip_info->session_id);
    return -1;
  }
  temp_min_id = session->sw_skip_info.min_frame_id;
  temp_max_id = session->sw_skip_info.max_frame_id;

  if (sw_skip_info->skip_mode == IFACE_SKIP_RANGE &&
    (sw_skip_info->min_frame_id <= (temp_max_id + 1))) {
    /* If sensor updates skip and it includes current skip, preserve min id
            e.g. if current frame skip is 100 to 105 and at in between lets
            say at frame 104 sensor updates skip to be 106-108, sesnor still
            expect us to drop 105 which wont happen if we dont save min. */
    sw_skip_info->min_frame_id = temp_min_id;
    CDBG_HIGH("<skip_dbg> sw_skip_info->min_frame_id %d ",
      sw_skip_info->min_frame_id);
  }
  memset(&session->sw_skip_info, 0, sizeof(session->sw_skip_info));
  session->sw_skip_info = *sw_skip_info;

  memset(&frame_skip, 0, sizeof(frame_skip));
  frame_skip.use_sw_skip = TRUE;
  frame_skip.session_id = sw_skip_info->session_id;
  frame_skip.sw_frame_skip_info = *sw_skip_info;
  CDBG_HIGH("%s: <skip_dbg> skip image %d skip_stats %d mode %d min %d max %d\n",
    __func__, sw_skip_info->skip_image_frames,
    sw_skip_info->skip_stats_frames,
    sw_skip_info->skip_mode,
    sw_skip_info->min_frame_id,
    sw_skip_info->max_frame_id);

  for(isp = VFE0; isp < VFE_MAX; isp++) {
    if (session->session_resource.isp_id_mask & (1 << isp)) {
      if (iface->isp_axi_data.axi_data[isp].axi_hw_ops) {
        rc = iface->isp_axi_data.axi_data[isp].axi_hw_ops->set_params(
          iface->isp_axi_data.axi_data[isp].axi_hw_ops->ctrl,
          IFACE_AXI_SET_PARAM_FRAME_SKIP, (void *)&frame_skip, sizeof(frame_skip));
        if (rc < 0) {
          CDBG_ERROR("%s:isp IFACE_AXI_SET_PARAM_FRAME_SKIP failed, rc = %d\n",
            __func__, rc);
          return rc;
        }
        rc = iface->isp_axi_data.axi_data[isp].axi_hw_ops->set_params(
           iface->isp_axi_data.axi_data[isp].axi_hw_ops->ctrl,
           IFACE_AXI_SET_PARAM_STATS_SKIP, (void *)&frame_skip, sizeof(frame_skip));
        if (rc < 0) {
          CDBG_ERROR("%s:isp IFACE_AXI_SET_PARAM_STATS_SKIP failed, rc = %d\n",
            __func__, rc);
          return rc;
        }
      }
    }
  }

  return rc;
}

/** iface_fast_aec_mode:
 *
 *@iface : iface instance
 *@stream_id : stream info
 *@session_id : session info
 *@fast_aec : fast aec mode info
 *
 * Stores fast aec information in session params
 *
 *  Return: TRUE/FALSE
 **/
boolean iface_fast_aec_mode(
  iface_t             *iface __unused,
  uint32_t             stream_id,
  iface_session_t     *session,
  mct_fast_aec_mode_t *fast_aec)
{
    if (session == NULL) {
        CDBG_ERROR("failed: %s no session found \n", __func__);
        return FALSE;
    }

    CDBG_HIGH("%s: fast_aec= %d stream_id= %x session_id= %x\n",
    __func__, fast_aec->enable, stream_id, session->session_id);

    session->fast_aec_mode = fast_aec->enable;
    if (session->fast_aec_mode) {
      session->hal_bundle_parm.num_of_streams = 0;
      session->hal_bundle_parm.bundle_id = -1;
    }

  return TRUE;
}
/** iface_start_session:
 *    @iface: isp_interface pointer
 *    @session_id session id to be started
 *
 *  This function runs in MCTL thread context.
 *
 *  This function starts an iface session
 *
 *  Return:  0 - Success
 *          -1 - Session not found
 **/
int iface_start_session(iface_t *iface, uint32_t session_id)
{
  int i, j, rc = 0;
  iface_session_t *session = NULL;
  char value[PROPERTY_VALUE_MAX];

  CDBG("%s: E\n", __func__);

  for (i = 0; i < IFACE_MAX_SESSIONS; i++) {
    if (iface->sessions[i].iface == NULL) {
      /* save the 1st unused session ptr */
      session = &iface->sessions[i];
      memset(session, 0, sizeof(iface_session_t));
      session->ion_fd = iface_open_ion();
      if (session->ion_fd < 0) {
        CDBG_ERROR("%s: Ion device open failed\n", __func__);
        return -1;
      }

      rc = iface_open_buf_mgr(&iface->buf_mgr);
      if (rc < 0) {
        CDBG_ERROR("%s: open bug mgr ion fd failed, rc = %d\n", __func__, rc);
        return rc;
      }

      rc = iface_session_sem_thread_start(
        &iface->sessions[i].session_thread, &iface->sessions[i]);
      if (rc < 0) {
        CDBG_ERROR("%s: start session thread fail, rc = %d\n", __func__, rc);
        return rc;
      }
      session->iface = iface;
      session->session_id = session_id;
      session->hfr_param.hfr_mode = CAM_HFR_MODE_OFF;
      session->dynamic_stats_skip_feature_enb = FALSE;
      session->session_resource.main_frame_intf = VFE_SRC_MAX;
      session->session_resource.main_frame_vfe = VFE_MAX;
      session->use_testgen = false;
      session->security_mode = NON_SECURE_MODE;
      session->nano_timestamp = 0;
#ifdef _ANDROID_
      property_get("persist.vendor.camera.isp.testgen", value, "0");
      if (atoi(value))
        session->use_testgen = true;
#endif

      for (j = 0; j < IFACE_FRAME_CTRL_SIZE; j++) {
        session->parm_q[j] = (mct_queue_t *)calloc(1, sizeof(mct_queue_t));
        if (!session->parm_q[j]) {
          CDBG_ERROR("%s:%d calloc failed\n", __func__, __LINE__);
          for (j--; j > -1; j--) {
            free(session->parm_q[j]);
          }
          break;
        }
        mct_queue_init(session->parm_q[j]);
        pthread_mutex_init(&session->parm_q_lock[j], NULL);
      }

      /* Initialize HVX */
      rc = iface_hvx_start_session(&iface->hvx_singleton, &session->hvx);
      if (rc < 0) {
        CDBG_ERROR("%s:%d failed: iface_hvx_start_session rc %d\n", __func__,
          __LINE__, rc);
      }

      pthread_mutex_init(&session->mutex, NULL);
      pthread_mutex_init(&session->fe.mutex, NULL);
      session->fe.frame_q = (mct_queue_t *)calloc(1, sizeof(mct_queue_t));
      if (!session->fe.frame_q) {
        CDBG_ERROR("%s:%d calloc failed\n", __func__, __LINE__);
        rc = -ENOMEM;
        return rc;
      }
      mct_queue_init(session->fe.frame_q);
      session->frame_request.hw_stream_id = 0;
      CDBG("%s: X", __func__);
      return 0;
    }
  }

  CDBG_ERROR("%s: X, rc = -1\n", __func__);
  return -1;
}


/** iface_discover_subdev_nodes: discover ISPIF subdev
 *    @iface: iface pointer
 *
 *  This function runs in MCTL thread context.
 *
 *  This function discover ISPIF HW
 *
 *  Return:  0 - Success
 *          -1 - error during discovering subdevices
 **/
static int iface_discover_subdev_nodes(iface_t *iface)
{
  struct media_device_info mdev_info;
  int num_media_devices = 0;
  char dev_name[32];
  int rc = 0, dev_fd = 0;
  int num_entities;
  int ret = 0;

  CDBG("%s: E\n", __func__);
  iface_isp_subdevs_t *subdev_info;
  while (1) {
    snprintf(dev_name, sizeof(dev_name), "/dev/media%d", num_media_devices);
    dev_fd = open(dev_name, O_RDWR | O_NONBLOCK);
    if (dev_fd < 0) {
      CDBG("Done discovering media devices\n");
      break;
    }

    num_media_devices++;
    rc = ioctl(dev_fd, MEDIA_IOC_DEVICE_INFO, &mdev_info);
    if (rc < 0) {
      CDBG_ERROR("Error: ioctl media_dev failed: %s\n", strerror(errno));
      close(dev_fd);
      continue;
    }

    if (strncmp(mdev_info.model, "msm_config", sizeof(mdev_info.model)) != 0) {
      close(dev_fd);
      continue;
    }

    num_entities = 0;
    while (1) {
      struct media_entity_desc entity;
      memset(&entity, 0, sizeof(entity));
      entity.id = num_entities | MEDIA_ENT_ID_FLAG_NEXT;
      rc = ioctl(dev_fd, MEDIA_IOC_ENUM_ENTITIES, &entity);
      if (rc < 0) {
        CDBG("Done enumerating media entities\n");
        rc = 0;
        break;
      }
      num_entities = entity.id;

      CDBG_ERROR("%s:%d entity name %s type %x group id %d\n", __func__, __LINE__,
        entity.name, entity.type, entity.group_id);

      /* fill in ispif dev name*/
      if(mct_util_match_subdev_type(entity, MSM_CAMERA_SUBDEV_ISPIF,
        MEDIA_ENT_T_V4L2_SUBDEV)) {
        snprintf(iface->ispif_data.subdev_name, sizeof(iface->ispif_data.subdev_name), "/dev/%s",
          entity.name);
        iface->ispif_data.num_subdev++;
      }

      /* fill in isp dev name*/
      ret = mct_util_match_subdev_type(entity, MSM_CAMERA_SUBDEV_VFE,
        MEDIA_ENT_T_V4L2_SUBDEV);
      if ( ret &&
        iface->isp_axi_data.num_subdev < IFACE_ISP_SD_NODE_ID_MAX_NUM) {
        subdev_info = &iface->isp_axi_data.isp_subdev[iface->isp_axi_data.num_subdev];
        snprintf(subdev_info->subdev_name, sizeof(subdev_info->subdev_name),
          "/dev/%s", entity.name);
        iface->isp_axi_data.num_subdev++;
      }
    }

    close(dev_fd);
  }

  if (iface->ispif_data.num_subdev == 0 || iface->isp_axi_data.num_subdev == 0) {
   CDBG_ERROR("%s: no ispif/isp subdev found!!! error rc = -1\n", __func__);
   rc = -1;
  }

  return rc;
}

/** iface_reserve_sink_port:
 *
 *  This function runs in MCTL thread context.
 *
 *  This function tries to reserve a sink port in ispif module.
 *
 *  Return:  0 - Success
 *     -EAGAIN - Has another port matching the sensor cap.
 *      other negative value - error while reserving port
 **/
int iface_reserve_sink_port(iface_t *iface, iface_port_t *reserving_sink_port,
  sensor_src_port_cap_t *sensor_port_cap, mct_stream_info_t *stream_info,
  unsigned int session_id, unsigned int stream_id)
{
  int rc = 0;
  int is_new_sink = 0;
  iface_stream_t *stream = NULL;
  iface_port_t *iface_sink_port = NULL;
  CDBG("%s: E\n", __func__);

  iface_session_t *session = iface_util_get_session_by_id(iface,  session_id);
  if (!session){
      CDBG_ERROR("%s: cannot find session %d\n", __func__, session_id);
      return -1;
  }

  stream = iface_util_find_stream_in_sink_port_list(iface, session_id, stream_id);
  if (stream != NULL) {
    CDBG_ERROR("%s: stream %d already exist. rc = -1\n", __func__, stream_id);
    return -1;
  }

  /* has another port matching the sensor cap. EAGAIN
     just check the session id in the cap,
     if match the session id, then return true
     MCT will check if it ext linked and unreserve and reserve to next port again*/
  iface_sink_port = iface_util_find_sink_port_by_session_id(iface, &session_id);
  if (iface_sink_port != NULL && iface_sink_port != reserving_sink_port) {
    return -EAGAIN;
  }

  /*no match port yet, so do reserve,
    fill in snesor cap and streaminfo to iface_sink_port (reserving src port)*/
  if (iface_sink_port == NULL) {
    if (reserving_sink_port->state == IFACE_PORT_STATE_CREATED) {
      iface_sink_port = reserving_sink_port;
      iface_sink_port->iface = iface;
      iface_sink_port->state = IFACE_PORT_STATE_RESERVED;
      is_new_sink = 1;
    } else
     return -1;
  }

  if (is_new_sink) {
    /* remove it, move the num_cid, cid channels to sensor_out_info_t,
       dont use this caps*/
    iface_sink_port->session_id = session_id;
  }
  if (sensor_port_cap)
    iface_sink_port->u.sink_port.sensor_cap = *sensor_port_cap;

  /* add stream to port private (iface_sinkport)*/
  stream = iface_util_add_stream_to_sink_port(iface, iface_sink_port, session, stream_id, stream_info);
  if (!stream) {
    /* no stream slot available */
    if (is_new_sink) {
      iface_sink_port->state = IFACE_PORT_STATE_CREATED;
    }
    goto error;
  }
  if (stream_info->streaming_mode == CAM_STREAMING_MODE_BATCH) {
    if (session->batch_info.batch_mode &&
      session->batch_info.batch_size > 0) {
      /* One batch stream is already present, currently can not support
        * more than one batch streams */
        CDBG_ERROR("%s: Stream with batch %d already present can't start %x\n",
          __func__, session->batch_info.batch_size, stream_info->identity);
        goto error;
    }
    if (!session->batch_info.batch_size ||
      stream_info->user_buf_info.frame_buf_cnt <
    session->batch_info.batch_size) {
      /* If multiple steams are connected, use smallest batch size */
      session->batch_info.batch_size =
          stream_info->user_buf_info.frame_buf_cnt;
      CDBG_ERROR("%s:batch_dbg stream %x batch_size %d", __func__,
        stream_info->identity, session->batch_info.batch_size);
    }
    session->batch_info.batch_mode = TRUE;
  }

  /*currently CDS is per session based, will need to change to per stream based
    for future feature coming in.*/
  if(stream_info->reprocess_config.pp_feature_config.feature_mask &
    CAM_QCOM_FEATURE_CDS) {
    session->cds_feature_enable = TRUE;
  }
  /*update the security mode in session parameter to use across all
    iface sub modules*/
  if ((session->security_mode == SECURE_MODE) &&
      (stream_info->is_secure == NON_SECURE)) {
    CDBG_HIGH("%s: non secure stream not allowed in secure session\n",
      __func__);
    goto error;
  }
  if (stream_info->is_secure == NON_SECURE) {
    session->security_mode = NON_SECURE_MODE;
  }
  else if (stream_info->is_secure == SECURE) {
    if (stream_info->secure_mode == SECURE_SLAVE) {
#ifdef SECURE_SLAVE_MODE
        session->security_mode = SECURE_SLAVE_MODE;
    }
#else
        CDBG_ERROR("Invalid secure mode %d set!\n");
        goto error;
    }
#endif
    else if (stream_info->secure_mode == SECURE_MASTER) {
        session->security_mode = SECURE_MODE;
    }
  }
  CDBG_HIGH("%s: stream type %d is_secure %d secure_mode %d "
    "session->security mode %d\n", __func__, stream_info->stream_type,
    stream_info->is_secure, stream_info->secure_mode, session->security_mode);

  if (stream_info->stream_type == CAM_STREAM_TYPE_PARM ) {
    session->session_streams_identity = stream_id;
  }

  return 0;

error:
  rc = -1;

  if (stream) {
    iface_unreserve_sink_port(iface, iface_sink_port,
      stream->session_id, stream->stream_id);
  } else {
    iface_unreserve_sink_port(iface, iface_sink_port, session_id, stream_id);
  }

  return rc;
}

/** iface_ext_unlink_sink_port:
 *
 *  This function runs in MCTL thread context.
 *
 *  This function unlinks ispif sink port
 *
 *  Return:  0 - Success
 *          -1 - Cannot find stream
 **/
int iface_ext_unlink_sink_port(
  iface_t      *iface,
  iface_port_t *unlinking_sink_port __unused,
  mct_port_t   *peer_port __unused,
  uint32_t      session_id,
  uint32_t      stream_id)
{
  int rc = 0;
  iface_stream_t *stream = NULL;
  CDBG("%s: E\n", __func__);

  stream = iface_util_find_stream_in_sink_port_list(iface, session_id, stream_id);
  if (stream == NULL) {
    CDBG_ERROR("%s: stream not found in sink ports. error\n", __func__);
    return -1;
  }

  return rc;
}


/** iface_ext_unlink_src_port:
 *
 *  This function runs in MCTL thread context.
 *
 *  This function unlinks ispif source port
 *
 *  Return:  0 - Success
 *          -1 - cannot find stream
 **/
int iface_ext_unlink_src_port(
  iface_t      *iface,
  iface_port_t *unlinking_src_port __unused,
  mct_port_t   *peer_port __unused,
  uint32_t      session_id,
  uint32_t      stream_id)
{
  int i, rc = 0;

  iface_session_t *session = NULL;
  iface_stream_t *user_stream = NULL;
  iface_hw_stream_t *hw_stream = NULL;
  iface_port_t     *iface_sink_port = NULL;

  CDBG("%s: E\n", __func__);
  session = iface_util_get_session_by_id(iface,session_id);
  if (session == NULL) {
    CDBG_ERROR("%s: session not found, session_id = %d, rc = -1\n", __func__, session_id);
    return -1;
  }

  /* remove hw_stream's resource
     if all the mapped mct streasm got unlinked */
  hw_stream =
    iface_util_find_hw_stream_by_mct_id(iface, session, stream_id);
  if (hw_stream == NULL) {
    IFACE_DBG("mct stream already removed from hw stream\n");
    return 0;
  }

  for (i = 0; i < IFACE_MAX_STREAMS; i++) {
    if (hw_stream->mapped_mct_stream_id[i] == stream_id) {
     hw_stream->num_mct_stream_mapped--;
     hw_stream->mapped_mct_stream_id[i] = 0;
    }
  }

  /* the timing to release resource:
    1. all streamoff, we will release all the resource,
      we will get set stream config to reserve stream again when first streamon
    2. all the mapped mct stream unlinked, release resource on this hw stream
    */
  if (hw_stream->num_mct_stream_mapped == 0) {
    CDBG_HIGH("%s: release HW stream id %x!!! \n", __func__,
      hw_stream->hw_stream_id);
    iface_util_release_resource(iface, session, hw_stream);
  }

  /*when last stream off, the state should be reset already,
    however, here is doing the clean up resource when any stream off fail*/
  if (session->num_hw_stream == 0) {
    iface_sink_port = iface_util_find_sink_port_by_session_id(iface, &session_id);
    if (!iface_sink_port) {
      CDBG("%s: sink port already unlinked\n", __func__);
      return 0;
    }

    iface_sink_port->state = IFACE_PORT_STATE_RESERVED;
  }

  return rc;
}

/** iface_reserve_src_port:
 *
 *  This function runs in MCTL thread context.
 *
 *  This function reserves an ispif sink port
 *
 *  Return:  0 - Success
 *          -1 - Cannot find stream or port is in invalid state
 *     -EAGAIN - The given src port cannot be used to reserve
 **/
int iface_reserve_src_port(
  iface_t           *iface,
  iface_port_t      *reserving_src_port,
  mct_stream_info_t *stream_info __unused,
  unsigned int       session_id,
  unsigned int       stream_id)
{
  int i, rc = 0;
  int is_new_sink = 0;
  iface_stream_t *stream = NULL;
  iface_port_t *iface_src_port = NULL;
  iface_port_t *iface_sink_port = NULL;

  CDBG("%s: E\n", __func__);

  stream = iface_util_find_stream_in_sink_port_list(iface, session_id, stream_id);
  if (stream == NULL) {
    CDBG_ERROR("%s: stream not found in all sink ports. error\n", __func__);
    return -1;
  }

  /* found stream, means we already added it to sink port*/
  iface_sink_port = (iface_port_t *)stream->sink_port->port_private;

  /*find if we have any src port already reserved, if yes then return*/
  iface_src_port = iface_util_get_match_src_port(iface, iface_sink_port, stream);
  if (iface_src_port != NULL && iface_src_port != reserving_src_port) {
    /* the given src port cannot be used to reserve */
    CDBG_ERROR("%s: this src port can not use!iface_src_port %p, reserving_src_port %p\n",
      __func__, iface_src_port, reserving_src_port);
    return -EAGAIN;
  }

  /*fill src port data on iface_src_por (reserving src port)t*/
  if (iface_src_port == NULL) {
    if (reserving_src_port->state == IFACE_PORT_STATE_CREATED) {
      iface_src_port = reserving_src_port;
      iface_src_port->iface = iface;
      /* Q? if we need it, if not remove it.*/
      if (stream->use_pix == 1) {
         iface_src_port->u.src_port.caps.use_pix = stream->use_pix;
      }
      iface_src_port->u.src_port.caps.input_type = ISP_INPUT_ISPIF;
      iface_src_port->u.src_port.caps.sensor_cap =
        iface_sink_port->u.sink_port.sensor_cap;
      iface_src_port->state = IFACE_PORT_STATE_RESERVED;

      /* need!*/
      iface_src_port->session_id = stream->session_id;
    } else {
      CDBG_ERROR("%s: port not created! port state = %d\n", __func__, reserving_src_port->state);
      return -1;
    }
  }

  iface_util_add_stream_to_src_port(iface, iface_src_port, stream);

  stream->src_port = iface_src_port->port;
  stream->state = IFACE_STREAM_ASSOCIATED_WITH_SRC_PORT;

  return rc;
}

/** iface_unreserve_sink_port:
 *
 *  This function runs in MCTL thread context.
 *
 *  This function unreserves an ispif port.
 *
 *  Return:  0 - Success
 *          -1 - Stream not found
 **/
int iface_unreserve_sink_port(iface_t *iface, iface_port_t *unreserving_sink_port,
  uint32_t session_id, uint32_t stream_id)
{
  int i, rc = 0;
  iface_stream_t *stream = NULL;
  CDBG("%s: E\n", __func__);

  stream = iface_util_find_stream_in_sink_port_list(iface, session_id, stream_id);
  if (stream == NULL) {
    CDBG_ERROR("%s: stream not found in sink ports. error, rc = -1\n", __func__);
    return -1;
  }

  iface_session_t *session = iface_util_get_session_by_id(iface,  session_id);
  if (!session){
      CDBG_ERROR("%s: cannot find session %d\n", __func__, session_id);
      return -1;
  }

  IFACE_HIGH("stream_id %x mode %d \n", stream->stream_id,
    stream->stream_info.streaming_mode);
  if (stream->stream_info.streaming_mode == CAM_STREAMING_MODE_BATCH) {
    /* only one batch stream is supported, need to reset the batch info
       we need to revisit this batch thing */
    memset(&session->batch_info, 0, sizeof(session->batch_info));
  }
  rc = iface_util_del_stream_from_sink_port(iface, unreserving_sink_port, stream);
  if (rc < 0) {
    CDBG_ERROR("%s: iface_util_del_stream_from_sink_port err, rc = %d\n", __func__, rc);
    return rc;
  }

  session->security_mode = NON_SECURE_MODE;
  if (unreserving_sink_port->u.sink_port.num_streams == 0) {
    /* no more stream associated with it. release the port */
    unreserving_sink_port->state = IFACE_PORT_STATE_CREATED;
    unreserving_sink_port->session_id = 0; /* remove the session id */
    unreserving_sink_port->port->peer = NULL;
  }
  if (session->session_streams_identity == stream_id) {
    session->session_streams_identity = 0;
  }

  return rc;
}

/** iface_clear_hal_param_q
*
*  @parm_q: array of param queues
*  @parm_q_lock: array of mutexes for param queue
*
*  Free all data in all param queues
*
*  Returns 0 on success and negative error on failure
**/
static int iface_clear_hal_param_q(iface_t *iface, iface_session_t *session,
  mct_queue_t **parm_q, pthread_mutex_t *parm_q_lock)
{
  iface_frame_ctrl_data_t *frame_ctrl_data = NULL;
  uint32_t i;
  int rc = 0;

  if (!iface || !session || !parm_q || !parm_q_lock) {
    CDBG_ERROR("Failed: %p %p %p %p\n", iface, session, parm_q,
      parm_q_lock);
    return -1;
  }

  for (i = 0; i < IFACE_FRAME_CTRL_SIZE; i++) {
    if (parm_q[i] == NULL) {
      CDBG_ERROR("%s: error parm_q\n", __func__);
      rc = -1;
      break;
    }
    iface_restore_hal_param(iface, session, parm_q[i], &parm_q_lock[i],
      session->sof_frame_id);
  }

  return rc;
}

/** iface_free_hal_param_q_data:
 *
 *  @data: list data
 *
 *  @user_data : user data (NULL)
 *
 *  Free list data
 **/
static boolean iface_free_hal_param_q_data(
  void *data,
  void *user_data __unused)
{
  iface_frame_ctrl_data_t *frame_ctrl_data = (iface_frame_ctrl_data_t *)data;

  if (frame_ctrl_data->ctrl_param.parm_data) {
    free(frame_ctrl_data->ctrl_param.parm_data);
    frame_ctrl_data->ctrl_param.parm_data = NULL;
  }
  free(frame_ctrl_data);

  return TRUE;
}

/** iface_stop_session:
 *
 *  This function runs in MCTL thread context.
 *
 * This functions stops a session
 *
 *  Return:  0 - Success
 *          -1 - Session not found
 **/
int iface_stop_session(iface_t *iface, uint32_t session_id)
{
  int i, j;
  iface_session_t *session = NULL;
  int rc;

  for (i = 0; i < IFACE_MAX_SESSIONS; i++) {
    session = &iface->sessions[i];
    /*memset whole session*/
    if (session->iface && session->session_id == session_id) {

      /* De-Initialize HVX */
      rc = iface_hvx_stop_session(&iface->hvx_singleton, &session->hvx);
      if (rc < 0) {
        CDBG_ERROR("%s:%d failed: iface_hvx_stop_session rc %d\n", __func__,
          __LINE__, rc);
      }
      /* clean up the allocated native buffer */
      for (i=0; i<VFE_AXI_SRC_MAX; i++) {
        iface_release_native_buf_handles(&iface->buf_mgr, IFACE_MAX_IMG_BUF,
          session->image_bufs[i]);
      }

      iface_close_ion(session->ion_fd);
      /* save the 1st unused session ptr */
      iface_close_buf_mgr(&iface->buf_mgr);
      for (j = 0; j < VFE_MAX; j++) {
        if ((session->session_resource.isp_id_mask & (1 << j)) &&
            iface->isp_axi_data.axi_data[j].axi_hw_ops != NULL) {
              iface->isp_axi_data.axi_data[j].axi_hw_ops->destroy(
          iface->isp_axi_data.axi_data[j].axi_hw_ops->ctrl);
        }
	/*
         * Currently offline VFE uses different VFE thatn online.
         * TODO: Use Same VFE for online & offline processing.
         */
        if ((session->session_resource.offline_isp_id_mask& (1 << j)) &&
            iface->isp_axi_data.axi_data[j].axi_hw_ops != NULL) {
              iface->isp_axi_data.axi_data[j].axi_hw_ops->destroy(
          iface->isp_axi_data.axi_data[j].axi_hw_ops->ctrl);
        }
      }
      pthread_mutex_destroy(&session->mutex);
      iface_session_sem_thread_execute(session, &session->session_thread,
        IFACE_ASYNC_COMMAND_EXIT);

      iface_session_sem_thread_stop(&session->session_thread);

      mct_queue_free_all(session->fe.frame_q, NULL);
      pthread_mutex_destroy(&session->fe.mutex);
      memset(&session->fe, 0, sizeof(session->fe));

      for (j = 0; j < IFACE_FRAME_CTRL_SIZE; j++) {
        mct_queue_free_all(session->parm_q[j], iface_free_hal_param_q_data);
        pthread_mutex_destroy(&session->parm_q_lock[j]);
      }
      memset(session, 0, sizeof(iface_session_t));

      return 0;
    }
  }

  return -1;
}

/** iface_set_session_data:
 *
 *  This function runs in MCTL thread context.
 *
 * This store pipeline delays and zzHDR HW available
 *
 *  Return:  0 - Success
 *          -1 - Session not found
 **/
int iface_set_session_data(iface_t *iface, uint32_t session_id,
  mct_pipeline_session_data_t *session_data)
{
  iface_session_t   *session;

  session = iface_util_get_session_by_id(iface, session_id);
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n", __func__, session_id);
    return -1;
  }

  session->max_apply_delay = session_data->max_pipeline_frame_applying_delay;
  session->max_reporting_delay =
      session_data->max_pipeline_meta_reporting_delay;

  session->zzhdr_hw_available = session_data->zzhdr_hw_available;
  return 0;

}

/** iface_unreserve_src_port:
 *    @ispif: ispif pointer
 *    @ispif_src_port: source port to be unreserved
 *    @session_id: session id
 *    @stream_id: stream id
 *
 *  This function runs in MCTL thread context.
 *
 *  This function unreserves an ispif source port
 *
 *  Return:  0 - Success
 *          -1 - Stream not found
 **/
int iface_unreserve_src_port(iface_t *iface, iface_port_t *iface_src_port,
  unsigned int session_id, unsigned int stream_id)
{
  int rc = 0;

  rc = iface_util_del_stream_from_src_port(iface, iface_src_port, session_id,
    stream_id);
  if (iface_src_port->u.src_port.num_streams == 0) {
    CDBG("%s: remove src port peer and set state to created\n", __func__);
    /* no more stream associated with it. release the port */
    iface_src_port->state = IFACE_PORT_STATE_CREATED;
    iface_src_port->session_id = 0;
    iface_src_port->port->peer = NULL;
  }
  return rc;
}

/** iface_ext_link_sink_port:
 *
 *  This function runs in MCTL thread context.
 *
 *  This pointer links ispif sink port with corresponding source port of other
 *  module.
 *
 *  Return:  0 - Success
 *          -1 - Cannot find stream or
 *               stream not in given sink port or
 *               peer is not matching
 **/
int iface_ext_link_sink_port(iface_t *iface, iface_port_t *linking_sink_port,
  mct_port_t *peer_port, uint32_t session_id, uint32_t stream_id)
{
  int i, rc = 0;
  iface_stream_t *stream = NULL;

  CDBG("%s: E\n", __func__);
  stream = iface_util_find_stream_in_sink_port_list(iface, session_id, stream_id);
  if (stream == NULL) {
    CDBG_ERROR("%s: stream not found in sink ports. error\n", __func__);
    return -1;
  }

  if (linking_sink_port->port->peer == NULL)
    linking_sink_port->port->peer = peer_port;
  else {
    if (linking_sink_port->port->peer != peer_port) {
      CDBG_ERROR("%s: peer port not matching (existing = %p, new = %p\n",
        __func__, linking_sink_port->port->peer, peer_port);
      rc = -1;
    }
  }
  CDBG("%s: link stream sessionid = %d, streamid = %d to sink port %p\n",
    __func__, session_id, stream_id, linking_sink_port);

  return rc;
}

/** iface_ext_link_src_port:
 *    @ispif: pointer to ispif instance
 *    @ispif_port: pointer to ispif source port
 *    @peer_port: pointer to which sink port will link
 *    @session_id: session id
 *    @stream_id: stream id
 *
 *  This function runs in MCTL thread context.
 *
 *  This pointer links ispif source port with corresponding sink port of other
 *  module.
 *
 *  Return:  0 - Success
 *          -1 - Cannot find stream or stream not reserved at source port
 **/
int iface_ext_link_src_port(iface_t *iface, iface_port_t *linking_src_port,
  mct_port_t *peer_port, uint32_t session_id, uint32_t stream_id)
{
  int rc = 0;
  iface_stream_t *stream = NULL;

  CDBG("%s: E\n", __func__);

  stream = iface_util_find_stream_in_sink_port_list(iface, session_id, stream_id);
  if (stream == NULL) {
    CDBG_ERROR("%s: stream not found in sink ports. error\n", __func__);
    return -1;
  }

  if (stream->src_port != linking_src_port->port) {
    /* stream is not reserved at teh given src port */
    CDBG_ERROR("%s: stream (sessid = %d, streamid = %d, ) "
      "not reserved at src port %p\n", __func__, session_id, stream_id,
      linking_src_port);
    return -1;
  }

  if (linking_src_port->port->peer == NULL)
    linking_src_port->port->peer = peer_port;
  else {
    if (linking_src_port->port->peer != peer_port) {
      CDBG_ERROR("%s: peer port not matching (existing = %p, new = %p\n",
        __func__, linking_src_port->port->peer, peer_port);
      rc = -1;
    }
  }

  return rc;
}

/** iface_sink_port_copy_stream_config:
 *
 *  This function configures iface hw streams and save in
 *  session
 *
 *  Return: 0 for success and negative error on failure
 **/
int iface_sink_port_copy_stream_config(
  iface_t           *iface __unused,
  iface_port_t      *iface_sink_port,
  uint32_t           stream_id __unused,
  iface_session_t   *session,
  sensor_out_info_t *sensor_out_info)
{
  int i, rc = 0;
  iface_hw_stream_t *hw_stream = NULL;
  struct msm_vfe_smmu_attach_cmd cmd;

  if (session == NULL) {
    CDBG_ERROR("%s: can not find session\n", __func__);
    return -1;
  }

  /* copy sensor config params */
  session->sensor_out_info = *sensor_out_info;
  iface_sink_port->u.sink_port.sensor_out_info = *sensor_out_info;

  return 0;
}

/** iface_sink_port_stream_config:
 *
 *  This function configures iface hw streams and save in
 *  session
 *
 *  Return: 0 for success and negative error on failure
 **/
int iface_sink_port_stream_config(iface_t *iface, iface_port_t *iface_sink_port,
  uint32_t stream_id, iface_session_t *iface_session)
{
  int i, rc = 0;
  iface_stream_t *stream = NULL;
  iface_hw_stream_t *hw_stream = NULL;
  struct msm_vfe_smmu_attach_cmd cmd;
  uint32_t vfe_clk[VFE_MAX];

  if (iface_session == NULL) {
    CDBG_ERROR("%s: can not find session \n", __func__);
    return -1;
  }

  stream = iface_util_find_stream_in_sink_port_list(iface, iface_session->session_id, stream_id);
  if (stream == NULL) {
    IFACE_ERR("stream not found. stream id: %d \n", stream_id);
    return -1;
  }

  if (iface_sink_port->state != IFACE_PORT_STATE_RESERVED ||
      stream->stream_info.stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
     IFACE_DBG("port state = %d, skip set stream config\n", iface_sink_port->state);
     return 0;
  }

  CDBG("%s: Sink port start stream config\n", __func__);

  iface_util_dump_sensor_cfg(&iface_sink_port->u.sink_port);

  /* if rc <0 for reserve resource failed case*/
  ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_IFACE_HW_CFG);
  rc = iface_util_set_hw_stream_cfg(iface,
    iface_sink_port, iface_session->session_id, stream_id);
  if (rc < 0) {
    CDBG_ERROR("%s: set stream config failed, rc = %d\n", __func__, rc);
    ATRACE_CAMSCOPE_END(CAMSCOPE_IFACE_HW_CFG);
    goto end;
  }

  for (i = 0; (uint32_t)i < iface_session->num_hw_stream; i++) {
    hw_stream = &iface_session->hw_streams[i];
    if (hw_stream->hw_stream_id != 0) {
      iface_util_send_hw_stream_output_dim_downstream_int(
        iface, iface_session, hw_stream);
    }
  }
  ATRACE_CAMSCOPE_END(CAMSCOPE_IFACE_HW_CFG);

  ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_IFACE_CREATE_AXI_HW);
  /* ispif open before axi stream*/
  if (!iface_sink_port->u.sink_port.sensor_out_info.is_dummy) {
     rc = iface_ispif_proc_open(iface,
                                iface_session,
                                iface_sink_port);
     if (rc < 0) {
       IFACE_ERR("iface_ispif_proc_open error, rc = %d\n", rc);
       return rc;
     }
  }

  /*Create AXI HW:
  after reserve isp resource and output. will get fd after create axi hw*/
  if (iface_session->session_resource.isp_id_mask & (1 << VFE0)) {
    if (iface_session->create_axi_count[VFE0] == 0) {
      rc = iface_create_axi(iface, iface_session, VFE0);
      if (rc < 0) {
        CDBG_ERROR("%s: cannot create ISP HW %d, rc = %d\n",
                   __func__, VFE0, rc);
        ATRACE_CAMSCOPE_END(CAMSCOPE_IFACE_CREATE_AXI_HW);
        goto clean_ispif;
      }
      CDBG("%s: create axi vfe0, cnt = %d\n",
                 __func__, iface_session->create_axi_count[VFE0]);
    }
    iface_session->create_axi_count[VFE0] +=
      iface_session->session_resource.camif_cnt +
      iface_session->session_resource.rdi_cnt;
  }

  if (iface_session->session_resource.isp_id_mask & (1 << VFE1)) {
    if (iface_session->create_axi_count[VFE1] == 0) {
      rc = iface_create_axi(iface, iface_session, VFE1);
      if (rc < 0) {
        CDBG_ERROR("%s: cannot create ISP HW %d, rc = %d\n",
                   __func__, VFE1, rc);
        ATRACE_CAMSCOPE_END(CAMSCOPE_IFACE_CREATE_AXI_HW);
        goto clean_axi_vfe0;
      }
      CDBG("%s: create axi vfe1, cnt = %d\n",
                 __func__, iface_session->create_axi_count[VFE1]);
    }
    iface_session->create_axi_count[VFE1] +=
      iface_session->session_resource.camif_cnt +
      iface_session->session_resource.rdi_cnt;
  }
  ATRACE_CAMSCOPE_END(CAMSCOPE_IFACE_CREATE_AXI_HW);

  /* config IO for pix interface or RDI*/
  if (iface_session->session_resource.isp_id_mask & (1 << VFE0)) {
     rc = iface_util_io_cfg(iface, iface_session, iface_sink_port, VFE0,
       &vfe_clk[VFE0]);
     if (rc < 0) {
       CDBG_ERROR("%s; input cff failed, rc= %d\n", __func__, rc);
       goto clean_axi_vfe1;
     }
  }
  if (iface_session->session_resource.isp_id_mask & (1 << VFE1)) {
     rc = iface_util_io_cfg(iface, iface_session, iface_sink_port, VFE1,
       &vfe_clk[VFE1]);
     if (rc < 0) {
       CDBG_ERROR("%s; input cfg failed, rc= %d\n", __func__, rc);
       goto clean_io_cfg_vfe0;
     }
  }

  if (iface_session->hvx.enabled == TRUE) {
    /* Configure HVX */
    rc = iface_hvx_handle_set_stream_config(&iface->hvx_singleton,
      &iface_session->hvx, &iface_sink_port->u.sink_port.sensor_out_info,
      &iface_session->session_resource, vfe_clk);
    if (rc < 0) {
      CDBG_ERROR("%s:%d failed: iface_hvx_handle_set_stream_config rc %d\n",
        __func__, __LINE__, rc);
      goto clean_io_cfg_vfe1;
    }
  }

  ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_IFACE_CFG_AXI_HW);
  /* config AXI HW*/
  rc = iface_util_config_axi(iface, iface_session);
  if (rc < 0) {
     CDBG_ERROR("%s: config axi failed! rc = %d\n", __func__, rc);
     ATRACE_CAMSCOPE_END(CAMSCOPE_IFACE_CFG_AXI_HW);
     goto clean_hvx_stream_config;
  }
  ATRACE_CAMSCOPE_END(CAMSCOPE_IFACE_CFG_AXI_HW);

  iface_sink_port->state = IFACE_PORT_STATE_HW_CONFIGED;

  if (iface_session->ms_type != MS_TYPE_NONE) {
    rc = iface_util_set_master_slave_info(iface_session, iface);
    if (rc < 0) {
      goto clean_config_axi;
    }
  }

  return rc;

clean_config_axi:
clean_hvx_stream_config:
clean_io_cfg_vfe1:
clean_io_cfg_vfe0:
clean_axi_vfe1:
  if (iface_session->session_resource.isp_id_mask & (1 << VFE1) &&
      (iface_session->create_axi_count[VFE1] > 0)) {
    iface_session->create_axi_count[VFE1] -=
      iface_session->session_resource.camif_cnt +
      iface_session->session_resource.rdi_cnt;
    if (iface_session->create_axi_count[VFE1] == 0) {
      iface_destroy_hw(iface, VFE1);
    }
  }
clean_axi_vfe0:
  if (iface_session->session_resource.isp_id_mask & (1 << VFE0) &&
      (iface_session->create_axi_count[VFE0] > 0)) {
    iface_session->create_axi_count[VFE0] -=
      iface_session->session_resource.camif_cnt +
      iface_session->session_resource.rdi_cnt;
    if (iface_session->create_axi_count[VFE0] == 0) {
      iface_destroy_hw(iface, VFE0);
    }
  }

  for (i = 0; i < IFACE_MAX_STREAMS; i++) {
    if (iface_session->hw_streams[i].stream_info.cam_stream_type ==
        CAM_STREAM_TYPE_OFFLINE_PROC) {
      CDBG_HIGH("%s: offline stream resources %x will be released once done\n",
        __func__, __LINE__, iface_session->hw_streams[i].hw_stream_id);
      continue;
    }
    CDBG("%s: hw_stream %p type %d axi_path %d id %x\n", __func__,
      &iface_session->hw_streams[i], iface_session->hw_streams[i].stream_type,
      iface_session->hw_streams[i].axi_path, iface_session->hw_streams[i].hw_stream_id);
    if (iface_session->hw_streams[i].hw_stream_id) {
      iface_util_release_resource(iface, iface_session, &iface_session->hw_streams[i]);
    }

    if (iface_session->stats_streams[i].stats_stream_id) {
      memset(&iface_session->stats_streams[i], 0 , sizeof(iface_stats_stream_t));
    }
  }
  iface_session->num_stats_stream = 0;
clean_ispif:
  iface_ispif_proc_close(iface);
end:
  return rc;
}

#ifdef VIDIOC_MSM_ISP_DUAL_HW_MASTER_SLAVE_SYNC
int iface_session_hw_sleep(
  iface_t         *iface,
  iface_port_t    *iface_sink_port,
  iface_session_t *iface_session,
  mct_event_t     *event)
{
  int rc = 0;

  CDBG("%s:iface_session->session_id = 0x%x\n", __func__, iface_session->session_id);

  if (!iface || !iface_sink_port || !event) {
    CDBG_ERROR("%s: Invalid parameter\n", __func__);
    return -1;
  }
  cam_dual_camera_perf_control_t* perf_ctrl;
  perf_ctrl = (cam_dual_camera_perf_control_t*)
    event->u.ctrl_event.control_event_data;
  if (!perf_ctrl) {
    CDBG_ERROR("%s:%d no mct_event_data\n", __func__, __LINE__);
    return -1;
  }
  if (perf_ctrl->enable == TRUE) {
    iface_session->lpm_enable = 1;
    iface_session->prev_lpm_mode = iface_session->cur_lpm_mode;
    iface_session->cur_lpm_mode = 1;
    CDBG("%s:perf_ctrl->perf_mode = %d", __func__, perf_ctrl->perf_mode);
    switch(perf_ctrl->perf_mode) {
      case CAM_PERF_SENSOR_SUSPEND:
        rc = iface_util_set_master_slave_sync(iface, iface_session,
                        MSM_ISP_DUAL_CAM_ASYNC);
#ifdef VIDIOC_MSM_ISP_DUAL_HW_LPM_MODE
        if (rc == 0)
          rc = iface_util_lpm_session(iface, iface_session, TRUE);
#endif
        break;
      case CAM_PERF_STATS_FPS_CONTROL:
        rc = iface_util_set_master_slave_sync(iface, iface_session,
			MSM_ISP_DUAL_CAM_ASYNC);
        break;

      case CAM_PERF_ISPIF_FRAME_DROP:
        rc = iface_util_set_master_slave_sync(iface, iface_session,
          MSM_ISP_DUAL_CAM_ASYNC);
        if (rc == 0)
        {
          rc = iface_util_set_ispif_frame_drop(iface, iface_session,
            iface_sink_port, ISPIF_STOP_FRAME_BOUNDARY);
        }
#ifdef VIDIOC_MSM_ISP_DUAL_HW_LPM_MODE
        if (rc == 0) {
          rc = iface_util_lpm_session(iface, iface_session, TRUE);
        }
#endif
        break;
      case CAM_PERF_ISPIF_FRAME_SKIP:
        break;

      default:
        rc = -1;
    }
  }
  return rc;
}

int iface_session_hw_wake(
  iface_t         *iface,
  iface_port_t    *iface_sink_port,
  iface_session_t *iface_session,
  mct_event_t     *event)
{
  int rc = 0;
  int j;

  CDBG("%s:iface_session->session_id = 0x%x\n",
    __func__, iface_session->session_id);

  if (!iface || !iface_sink_port || !event) {
    CDBG_ERROR("%s: Invalid parameter\n", __func__);
    return -1;
  }

  cam_dual_camera_perf_control_t* perf_ctrl;
  perf_ctrl = (cam_dual_camera_perf_control_t*)
    event->u.ctrl_event.control_event_data;
  if (!perf_ctrl) {
    CDBG_ERROR("%s:%d no mct_event_data\n", __func__, __LINE__);
    return -1;
  }

  CDBG("%s:perf_ctrl->perf_mode = %d", __func__, perf_ctrl->perf_mode);
  iface_session->lpm_enable = 0;
  iface_session->prev_lpm_mode = iface_session->cur_lpm_mode;
  iface_session->cur_lpm_mode = 0;
  switch(perf_ctrl->perf_mode) {
    case CAM_PERF_SENSOR_SUSPEND:
     rc = iface_util_set_master_slave_sync(iface, iface_session,
           MSM_ISP_DUAL_CAM_SYNC);
#ifdef VIDIOC_MSM_ISP_DUAL_HW_LPM_MODE
     rc = iface_util_lpm_session(iface, iface_session, FALSE);
     if (rc < 0) {
       CDBG_ERROR("failed !!! iface_util_lpm_session mode %d\n",
         perf_ctrl->perf_mode);
       return rc;
     }
#endif
        break;
    case CAM_PERF_STATS_FPS_CONTROL:
      rc = iface_util_set_master_slave_sync(iface, iface_session,
            MSM_ISP_DUAL_CAM_SYNC);
      break;

    case CAM_PERF_ISPIF_FRAME_DROP:
      rc = iface_util_set_master_slave_sync(iface, iface_session,
            MSM_ISP_DUAL_CAM_SYNC);
    #ifdef VIDIOC_MSM_ISP_DUAL_HW_LPM_MODE
      rc = iface_util_lpm_session(iface, iface_session, FALSE);
      if (rc < 0) {
        CDBG_ERROR("failed !!! iface_util_lpm_session mode %d\n",
          perf_ctrl->perf_mode);
        return rc;
      }
     #endif
      rc = iface_util_set_ispif_frame_drop(iface, iface_session,
        iface_sink_port, ISPIF_RESTART_FRAME_BOUNDARY);
      if (rc != 0)
      {
        CDBG_ERROR("%s:%d start ispif at frame boundary failed, rc = %d\n",
          __func__, __LINE__, rc);
        return rc;
      }
        break;
    case CAM_PERF_ISPIF_FRAME_SKIP:
        break;

      default:
        CDBG_ERROR("Invalid LPM mode selected %d\n", perf_ctrl->perf_mode);
        return -1;
  }

  for (j = 0; j < IFACE_FRAME_CTRL_SIZE; j++) {
        pthread_mutex_lock(&iface_session->parm_q_lock[j]);
        mct_queue_flush(iface_session->parm_q[j], iface_free_hal_param_q_data);
        pthread_mutex_unlock(&iface_session->parm_q_lock[j]);
  }
  return rc;
}
#endif


/** iface_handle_link_intra_session:
 *
 *  This function links the intra port for related session.
 *  Also it saves the Master/Slave info to session.
 *
 *  Return: 0 for success and negative error on failure
 **/
int iface_handle_link_intra_session(
  iface_t         *iface,
  iface_port_t    *iface_sink_port,
  iface_session_t *iface_session,
  mct_event_t     *event)
{
  uint32_t identity;
  mct_port_t *peer;
  int index;
  cam_sync_related_sensors_event_info_t *related_session_info = NULL;

  CDBG_HIGH("%s:%d] MCT_EVENT_CONTROL_LINK_INTRA_SESSION",
    __func__, __LINE__);

  if (!event || !iface_sink_port || !iface || !iface_session) {
    CDBG_ERROR("%s: Error! Invalid Params event %p iface %p port %p iface_session %p\n",
      __func__, event, iface, iface_sink_port, iface_session);
    return -1;
  }

  related_session_info = (cam_sync_related_sensors_event_info_t*)
    (event->u.ctrl_event.control_event_data);
  identity = related_session_info->related_sensor_session_id;
  peer = mct_port_find_intra_port_across_session(MCT_PORT_SINK,
    iface_sink_port->port, identity);

  CDBG_HIGH("%s: port %p intrapeer %p  identity %d"
    "sessionid %d dual_cam_sensor_info->type %d",
    __func__, iface_sink_port->port, peer,
    identity, iface_session->session_id,
    related_session_info->type);

  if (peer && iface_sink_port->port != peer) {
    MCT_PORT_INTRALINKFUNC(iface_sink_port->port)(identity,
      iface_sink_port->port, peer);

    pthread_mutex_lock(&iface_session->mutex);
    iface_session->peer_session_id = identity;
    pthread_mutex_unlock(&iface_session->mutex);

    for (index = 0; index < MAX_PORT_CONNECTIONS; index++) {
      CDBG("%s:%d] peer ports for %p is filled %d iden %d %p",
        __func__, __LINE__, iface_sink_port->port,
        iface_sink_port->port->inter_session_peer[index].is_filled,
        iface_sink_port->port->inter_session_peer[index].identity,
        iface_sink_port->port->inter_session_peer[index].port);
    }
  }

  return 0;
}

/** iface_axi_sem_thread_execute
 *
 *    @ctrl:
 *    @thread:
 *    @action:
 *
 **/
static int iface_session_sem_thread_execute(
  void                   *ctrl __unused,
  iface_session_thread_t *thread,
  iface_async_cmd_id_t    async_cmd_id)
{
  int rc = 0;
  iface_session_cmd_stream_cfg_t on_off_parm;
  CDBG("%s: E\n", __func__);

  if (async_cmd_id == IFACE_ASYNC_COMMAND_UV_SUBSAMPLE) {
    /*non blocking: dont wait if locked*/
    if (0 != pthread_mutex_trylock(&thread->cmd_mutex)) {
      CDBG_HIGH("%s: cds_dbg thread busy! skip thread request\n", __func__);;
      return -1;
    }
  } else {
    /* blocking call: execute the cmd*/
    pthread_mutex_lock(&thread->cmd_mutex);
  }

  thread->async_cmd_id = async_cmd_id;
  on_off_parm = thread->stream_on_off_parm;

  /* wake up sem thread in sem_main_looop*/
  sem_post(&thread->thread_wait_sem);

  if (async_cmd_id == IFACE_ASYNC_COMMAND_STREAMON ) {
    rc = port_iface_forward_event_to_peer(
        on_off_parm.iface, on_off_parm.mct_port, on_off_parm.event);
    if (rc == 0) {
      CDBG_ERROR("%s: forward_event error\n", __func__);
    }
    sem_post(&thread->ispif_start_sem);
  }

  /* stream on  need to wait till both axi done stream on*/
  sem_wait(&thread->sig_sem);
  /* Wait here to decrement the semaphore value to 0 in case the iface stream on
   * thread did not wait on it */
  if (async_cmd_id == IFACE_ASYNC_COMMAND_STREAMON)
    sem_trywait(&thread->ispif_start_sem);

  rc = thread->return_code;
  pthread_mutex_unlock(&thread->cmd_mutex);

  CDBG("%s: X\n", __func__);
  return rc;
}


/** iface_sw_skip_config_to_thread
 *
 *    @iface: iface handle
 *    @skip_info: SW skip info
 *
 **/
int iface_sw_skip_config_to_thread(iface_t *iface,
  iface_sw_frameskip *skip_info)
{
  int rc = 0;
  skip_info->ack_msg = IFACE_SKIP_STATUS_FAIL;
  iface_session_t *session = iface_util_get_session_by_id(iface,
                                  skip_info->session_id);
  if (session == NULL) {
    CDBG_ERROR("%s: no session found , id = %d\n, rc = -1", __func__,
    skip_info->session_id);
    return -1;
  }

  CDBG_HIGH("%s: E, session id = %d skip_image %d\n",
    __func__, skip_info->session_id, skip_info->skip_image_frames);

  if (skip_info->skip_mode == IFACE_SKIP_RANGE &&
    session->sof_frame_id >= skip_info->min_frame_id) {
    skip_info->ack_msg = IFACE_SKIP_STATUS_RETRY;
    CDBG_ERROR("%s: <skip_dbg> Too late for isp to apply skip %d >= %d\n",
      __func__, session->sof_frame_id, skip_info->min_frame_id);

    return -1;
  }

  iface_update_sw_frame_skip(iface, skip_info);

  skip_info->ack_msg = IFACE_SKIP_STATUS_OK;
  CDBG_HIGH("%s: X, session id = %d\n",
    __func__, skip_info->session_id);
  return rc;
}

/** iface_handle_cds_request_to_thread
 *
 *    @ctrl:
 *    @thread:
 *    @action:
 *
 **/
int iface_handle_cds_request_to_thread(iface_t *iface,
  iface_session_t *session, uint32_t cds_enable)
{
  int rc = 0;
  int i = 0;
  iface_hw_stream_t *hw_stream = NULL;

  if (!iface) {
    CDBG_ERROR("%s: NULL pointer! iface = %p\n", __func__, iface);
    return -1;
  }

  if (session == NULL) {
    CDBG_ERROR("%s: no session found ,\n, rc = -1", __func__);
    return -1;
  }

  /* Chroma Down Sampling
    1.give up when busy
    2.after UV_SS,session_thread will be blocked by busy cnt = 2. update in sof notify*/
  pthread_mutex_lock(&session->session_thread.busy_mutex);

  /* is busy need to move out:
     streamon doesnt got blocked by busy flag, uv ss will be blocked*/
  if (session->session_thread.thread_busy != TRUE) {
    session->session_thread.thread_busy = TRUE;
  } else {
    IFACE_ERR("<cds_dbg>thread busy, inform ISP drop CDS\n");
    pthread_mutex_unlock(&session->session_thread.busy_mutex);
    return -1;
  }
  pthread_mutex_unlock(&session->session_thread.busy_mutex);

  IFACE_HIGH("<cds_dbg2> EXECUTE AXI updating!! curr enb = %d, request enb = %d\n",
    session->cds_curr_enb,cds_enable);

  rc = iface_session_sem_thread_execute(session,
    &session->session_thread, IFACE_ASYNC_COMMAND_UV_SUBSAMPLE);
  if (rc < 0) {
    IFACE_ERR("CDS to thread error, rc= %d\n", rc);
    return rc;
  }

  return rc;
}

/** iface_offline_config_to_thread
 *
 *    @iface: iface handle
 *    @session id: session id
 *    @user stream id: user stream id
 *
 **/
int iface_offline_config_to_thread(iface_t *iface, iface_port_t *iface_sink_port,
  uint32_t user_stream_id, uint32_t session_id,
  iface_offline_isp_info_t  *offline_info)
{
  int rc = 0;

  iface_session_t *session = iface_util_get_session_by_id(iface,session_id);
  if (session == NULL) {
    CDBG_ERROR("%s: no session found , id = %d\n, rc = -1", __func__, session_id);
    return -1;
  }

  CDBG_HIGH("%s: E, session id = %d, user stream id = %d\n",
    __func__, session_id, user_stream_id);
  /* offline config
    1.dont give up, but mct thread will be blocked by the cmd_mutex */
  pthread_mutex_lock(&session->session_thread.busy_mutex);
  if (session->session_thread.thread_busy != TRUE)
     session->session_thread.thread_busy = TRUE;
  pthread_mutex_unlock(&session->session_thread.busy_mutex);

  session->session_thread.offline_cfg_parm.iface = iface;
  session->session_thread.offline_cfg_parm.iface_sink_port = iface_sink_port;
  session->session_thread.offline_cfg_parm.session_id = session_id;
  session->session_thread.offline_cfg_parm.stream_id = user_stream_id;
  session->session_thread.offline_cfg_parm.offline_info = offline_info;

  rc = iface_session_sem_thread_execute(session, &session->session_thread,
    IFACE_ASYNC_COMMAND_OFFLINE_CONFIG);
  if (rc < 0) {
    CDBG_ERROR("%s: streamon to thread error, rc= %d\n", __func__, rc);
    return rc;
  }

  CDBG_HIGH("%s: X, session id = %d, user stream id = %d\n",
    __func__, session_id, user_stream_id);
  return rc;
}

/** iface_resource_alloc_to_thread
 *
 *    @iface: iface handle
*    @iface_port: iface port
*    @user stream id: user stream id
*    @session id: session id
*    @p_buf_alloc_info id: buffer allocation info
 **/
int iface_resource_alloc_to_thread(iface_t *iface,
          uint32_t user_stream_id, uint32_t session_id,
          iface_buf_alloc_t *p_buf_alloc_info)
{
  int rc = 0;

  iface_session_t *session = iface_util_get_session_by_id(iface,session_id);
  if (session == NULL) {
    CDBG_ERROR("%s: no session found , id = %d\n, rc = -1",
        __func__, session_id);
    return -1;
  }

  /*
    1.dont give up, but mct thread will be blocked by the cmd_mutex */
  pthread_mutex_lock(&session->session_thread.busy_mutex);
  if (session->session_thread.thread_busy != TRUE)
     session->session_thread.thread_busy = TRUE;
  pthread_mutex_unlock(&session->session_thread.busy_mutex);

  session->session_thread.resource_alloc.iface       = iface;
  session->session_thread.resource_alloc.session_id  = session_id;
  session->session_thread.resource_alloc.stream_id   = user_stream_id;
  memcpy(&(session->session_thread.resource_alloc.buf_alloc_info),
        p_buf_alloc_info, sizeof(iface_buf_alloc_t));

  rc = iface_session_sem_thread_execute(session, &session->session_thread,
              IFACE_ASYNC_COMMAND_RESOURCE_ALLOC);
  if (rc < 0)
    CDBG_ERROR("%s: streamon to thread error, rc= %d\n", __func__, rc);

  return rc;
}

/** iface_offline_start_to_thread
 *
 *    @ctrl:
 *    @thread:
 *    @action:
 *
 **/
int iface_halt_recovery_to_thread(iface_t *iface,
  iface_halt_recovery_info_t *halt_recovery_info, uint32_t isp_id)
{
  int               rc = 0;
  iface_session_t  *session = NULL;

  if (!iface || !halt_recovery_info) {
    CDBG_ERROR("%s: NULL pointer %p %p\n", __func__, iface, halt_recovery_info);
    return -1;
  }

  session = iface_util_get_session_by_id(iface, halt_recovery_info->session_id);
  if (session == NULL) {
    CDBG_ERROR("%s: no session found , id = %d\n, rc = -1",
      __func__,  halt_recovery_info->session_id);
    return -1;
  }

  CDBG_HIGH("%s: E, session id = %d, isp id = %d\n",
    __func__,  halt_recovery_info->session_id, isp_id);
  /* streamon
    1.dont give up, but mct thread will be blocked by the cmd_mutex
    2. session thread will still be blocked if just done uv subsample(cnt = 2)*/
  pthread_mutex_lock(&session->session_thread.busy_mutex);
  /* is busy need to move out: streamon doesnt got blocked by busy flag, uv ss will be blocked*/
  if (session->session_thread.thread_busy != TRUE)
     session->session_thread.thread_busy = TRUE;

  /*halt will disable SOF, so need to reset all the waiting like CDS*/
  if (session->session_thread.busy_cnt > 0 &&
     (session->session_thread.is_busy_wait == TRUE)) {
      session->session_thread.busy_cnt = 0;
      session->session_thread.is_busy_wait = FALSE;
      sem_post(&session->session_thread.busy_sem);
  }
  pthread_mutex_unlock(&session->session_thread.busy_mutex);

  session->session_thread.recovery_param.iface = iface;
  session->session_thread.recovery_param.session = session;;
  session->session_thread.recovery_param.session_id =
    halt_recovery_info->session_id;
  session->session_thread.recovery_param.frame_id =
    session->sof_frame_id;
  session->session_thread.recovery_param.isp_id = isp_id;

  rc = iface_session_sem_thread_execute(session, &session->session_thread,
    IFACE_ASYNC_COMMAND_RECOVERY);
  if (rc < 0) {
    CDBG_ERROR("%s: IFACE_ASYNC_COMMAND_RECOVERY to thread error, rc= %d\n", __func__, rc);
    return rc;
  }

  CDBG_HIGH("%s: X, session id = %d, isp_id = %d\n",
    __func__, halt_recovery_info->session_id, isp_id);
  return rc;

}
/** iface_streamon_to_thread
 *
 *    @ctrl:
 *    @thread:
 *    @action:
 *
 **/
int iface_streamon_to_thread(iface_t *iface, mct_port_t *mct_iface_port ,
  iface_session_t *session, uint32_t user_stream_id, mct_event_t *event)
{
  int rc = 0;
  iface_port_t *iface_sink_port;

  if (session == NULL ) {
    IFACE_ERR("%s: no session found \n, rc = -1", __func__);
    return -1;
  }
  iface_sink_port = (iface_port_t *)mct_iface_port->port_private;

  IFACE_HIGH("%s: E, session id = %d, user stream id = %d\n",
    __func__, session->session_id, user_stream_id);
  /* streamon
    1.dont give up, but mct thread will be blocked by the cmd_mutex
    2. session thread will still be blocked if just done uv subsample(cnt = 2)*/
  pthread_mutex_lock(&session->session_thread.busy_mutex);
  /* is busy need to move out: streamon doesnt got blocked by busy flag, uv ss will be blocked*/
  if (session->session_thread.thread_busy != TRUE)
     session->session_thread.thread_busy = TRUE;
  pthread_mutex_unlock(&session->session_thread.busy_mutex);
  session->session_thread.stream_on_off_parm.iface = iface;
  session->session_thread.stream_on_off_parm.event = event;
  session->session_thread.stream_on_off_parm.iface_sink_port = iface_sink_port;
  session->session_thread.stream_on_off_parm.session = session;
  session->session_thread.stream_on_off_parm.session_id = session->session_id;
  session->session_thread.stream_on_off_parm.stream_id = user_stream_id;
  session->session_thread.stream_on_off_parm.mct_port  = mct_iface_port;

  if (session->active_count == 0) {
    session->session_thread.stream_on_off_parm.is_first_streamon = TRUE;
  }

  rc = iface_session_sem_thread_execute(session,
    &session->session_thread, IFACE_ASYNC_COMMAND_STREAMON);
  if (rc < 0) {
    IFACE_ERR("streamon to thread error, rc= %d\n", rc);
    return rc;
  }

  IFACE_HIGH("%s: X, session id = %d, user stream id = %d\n",
    __func__, session->session_id, user_stream_id);
  return rc;
}

/** iface_streamoff_to_thread
 *
 *    @ctrl:
 *    @thread:
 *    @action:
 *
 **/
int iface_streamoff_to_thread(
  iface_t         *iface,
  iface_port_t    *iface_sink_port,
  iface_session_t *session,
  uint32_t         user_stream_id,
  mct_event_t     *event)
{
  int rc = 0;

  if (session == NULL) {
    IFACE_ERR("%s: no session found, rc = -1", __func__);
    return -1;
  }

  IFACE_HIGH("%s: E, session id = %d, user stream id = %d\n",
    __func__, session->session_id, user_stream_id);

  /* streamoff
    1.dont give up, but mct thread will be blocked by the cmd_mutex
    2. session thread will still be blocked if just done uv subsample(cnt = 2)*/
  pthread_mutex_lock(&session->session_thread.busy_mutex);
  /* is busy need to move out: streamon doesnt got blocked by busy flag, uv ss will be blocked*/
  if (session->session_thread.thread_busy != TRUE) {
     session->session_thread.thread_busy = TRUE;
  }
  pthread_mutex_unlock(&session->session_thread.busy_mutex);

  session->session_thread.stream_on_off_parm.iface = iface;
  session->session_thread.stream_on_off_parm.event = event;
  session->session_thread.stream_on_off_parm.iface_sink_port = iface_sink_port;
  session->session_thread.stream_on_off_parm.session = session;;
  session->session_thread.stream_on_off_parm.session_id = session->session_id;
  session->session_thread.stream_on_off_parm.stream_id = user_stream_id;

  rc = iface_session_sem_thread_execute(session, &session->session_thread, IFACE_ASYNC_COMMAND_STREAMOFF);
  if (rc < 0) {
    IFACE_ERR("streamon to thread error, rc= %d\n", rc);
    return rc;
  }

  IFACE_HIGH("%s: X, session id = %d, user stream id = %d\n",
    __func__, session->session_id, user_stream_id);

  return rc;
}

/** iface_decide_session_hw_stream:
 *  @iface: iface object
 *  @session_id:
 *  @ num_hw_streams
 *  @ hw_stream_ids
 *  @ isStreamon
 *
 *  Return:  num_hw_streams
 *          -1 - cannot find session/stream or
 *               start streaming is not successful
 **/
boolean iface_decide_session_hw_stream(
  iface_t         *iface,
  iface_session_t *session,
  uint32_t        *num_hw_streams,
  uint32_t        *hw_stream_ids,
  uint32_t         is_streamon)
{
  uint32_t         hw_session_stream_ids[IFACE_MAX_STREAMS];
  uint32_t         num_session_hw_streams;
  boolean          add_sess_hw_stream = FALSE;
  uint32_t         i =0;

  if (!iface || !hw_stream_ids || !session) {
    CDBG_ERROR("%s NULL pointer %p %p %p\n", __func__, iface, hw_stream_ids,
      session);
    return false;
  }

  memset(hw_session_stream_ids, 0, sizeof(uint32_t) * IFACE_MAX_STREAMS);
  num_session_hw_streams =
    iface_util_decide_hw_streams(iface, session, 1,
      &session->session_streams_identity, hw_session_stream_ids, is_streamon);
  if (num_session_hw_streams == 0) {
    CDBG("%s: no Session based hw streams to streamon, *num_hw_streams = %d\n",
    __func__, num_session_hw_streams);
  }

  if (is_streamon) {
    if ((!session->active_count) && *num_hw_streams)
      add_sess_hw_stream = TRUE;
  } else {
    if (session->active_count == (*num_hw_streams + num_session_hw_streams))
      add_sess_hw_stream = TRUE;
  }

  if (add_sess_hw_stream == TRUE) {
    for (i = *num_hw_streams;
      i < *num_hw_streams + num_session_hw_streams; i++) {
      if ( i > IFACE_MAX_STREAMS )
      {
        CDBG_ERROR("%s Exceeded the max hw stream ids %d\n", __func__, i);
        return false;
      }
        hw_stream_ids[i] = hw_session_stream_ids[i - *num_hw_streams];
    }
    *num_hw_streams += num_session_hw_streams;
  }

  return true;
}

/** iface_post_control_sof_to_thread:
 *  @iface: iface object
 *  @iface_sink_port
 *  @session_id:
 *  @ hw_stream_ids
 *  @ event: control_sof_event
 *
 *  Return:  num_hw_streams
 *          -1 - cannot find session/stream or
 *               iface control sof processing status
 **/
int iface_post_control_sof_to_thread(iface_t *iface,
  iface_port_t *iface_sink_port, iface_session_t *session, uint32_t user_stream_id,
  mct_event_t *event)
{
  int rc = 0;
  boolean thread_busy = FALSE;

  if (session == NULL) {
    CDBG_ERROR("%s: no session found \n, rc = -1", __func__);
    return -1;
  }

  CDBG("%s: E, session id = %d, user stream id = %d\n",
    __func__, session->session_id, user_stream_id);


  rc = iface_handle_control_sof(iface, iface_sink_port, session,
    user_stream_id, event);
  if (rc < 0) {
    CDBG_ERROR("%s:%d failed: iface_handle_control_sof", __func__, __LINE__);
  }

  /* Check the status of session thread */
  pthread_mutex_lock(&session->session_thread.busy_mutex);
  thread_busy = session->session_thread.thread_busy;
  pthread_mutex_unlock(&session->session_thread.busy_mutex);

  /* If session thread is busy, we should not be requesting ISP to do
     CDS related settings, as there could be an issue where ISP completes
     applying settings but IFACE is still busy */
  if (thread_busy == FALSE) {
    rc = iface_util_update_cds(iface,
           session->session_id, user_stream_id);
    if (rc) {
      CDBG_ERROR("%s:%d failed iface_util_update_cds\n", __func__, __LINE__);
    }
  }
  return rc;
}

static int32_t iface_update_pix_streamon_info(iface_session_t *session,
  int num_hw_streams, uint32_t *hw_stream_ids, boolean is_streamon)
{
  int i = 0;
  iface_hw_stream_t *hw_stream = NULL;

  if (!session || (num_hw_streams <= 0) || !hw_stream_ids) {
    CDBG_ERROR("%s:%d session %p num_hw_streams %d hw_stream_ids %p\n",
      __func__, __LINE__, session, num_hw_streams, hw_stream_ids);
    return -1;
  }

   for (i = 0; i < num_hw_streams; i++) {
     hw_stream = iface_util_find_hw_stream_in_session(
       session, hw_stream_ids[i]);
     if (!hw_stream)
       continue;

     /* find starting hw stream for specific VFE*/
     if ((hw_stream->interface_mask & (1 << (16 * VFE0 + IFACE_INTF_PIX))) ||
       (hw_stream->interface_mask & (1 << (16 * VFE1 + IFACE_INTF_PIX)))) {
       if (is_streamon == TRUE) {
         session->num_pix_streamon++;
         if (session->num_pix_streamon == 1) {
           session->first_pix_stream_on_off = TRUE;
         }
       } else {
         session->num_pix_streamon--;
         if (!session->num_pix_streamon) {
           session->first_pix_stream_on_off = TRUE;
         }
       }
     }
   }

  return 0;
}

  /** iface_util_cleanup_axi_stream_on_failure:
 *
 *  This function runs in MCTL thread context.
 *
 * Clean up of AXI stream on failure
 *
 **/
static void iface_util_cleanup_axi_stream_on_failure(
  iface_t         *iface,
  iface_session_t *session,
  uint32_t         num_hw_streams,
  int              num_user_streams,
  uint32_t        *hw_stream_ids,
  uint32_t         user_stream_id,
  uint32_t        *user_streamon_ids)
{
  iface_hw_stream_t *hw_stream;
  uint32_t i;


  iface_decide_session_hw_stream(iface, session, &num_hw_streams,
    hw_stream_ids, 0);

  iface_util_decide_hw_streams(iface, session, num_user_streams,
    user_streamon_ids, hw_stream_ids, 0);

  iface_util_get_user_streams_by_bundle(session,
    user_stream_id, user_streamon_ids, 0);

  for (i = 0; i < num_hw_streams; i++) {
    hw_stream = iface_util_find_hw_stream_in_session(session, hw_stream_ids[i]);
    if (!hw_stream) {
      CDBG_ERROR("%s:%d failed hw_stream %p i %d\n", __func__, __LINE__,
        hw_stream, i);
      continue;
    }
   iface_util_release_image_buf(iface, session, hw_stream);
  }



  return;

}

/** iface_util_cleanup_ispif_stream_on_failure:
 *
 *  This function runs in MCTL thread context.
 *
 * Clean up of ispif stream on failure
 *
 **/
static void iface_util_cleanup_ispif_stream_on_failure(
  iface_t         *iface,
  iface_session_t *session,
  uint32_t         num_hw_streams,
  int              num_user_streams,
  uint32_t        *hw_stream_ids,
  uint32_t         user_stream_id,
  uint32_t        *user_streamon_ids,
  iface_port_t    *iface_sink_port __unused)
{

  iface_util_axi_streamoff(iface, session, num_hw_streams, hw_stream_ids, STOP_STREAM);

  iface_util_cleanup_axi_stream_on_failure(iface,session, num_hw_streams,
    num_user_streams, hw_stream_ids, user_stream_id, user_streamon_ids);

  return;
}

/** iface_streamon:
 *    @iface:            iface object pointer
 *    @iface_sink_port:  iface sink port object pointer
 *    @session_id:       session id for the stream off
 *    @user_stream_id:   mct stream id
 *    @event:            streamon event
 *
 *  This function streamon the mct stream specified by the
 *  MCT/HAL. It starts the hw stream, creates the bufq in user
 *  space and kernel as required. It also starts the ispif
 *  streaming
 *
 *  Return:  0 - Success
 *          -1 - Invalid stream/session ID or
 *               unsuccessful stopping of stream
 **/
int iface_streamon(
    iface_t       *iface,
    iface_port_t  *iface_sink_port,
    uint32_t       session_id,
    uint32_t       user_stream_id,
    mct_event_t   *event)
{
  uint32_t                          i;
  int                               k;
  int                               rc              = 0;
  boolean                           retValue        = true;
  iface_stream_t                   *user_stream     = NULL;
  iface_hw_stream_t                *hw_stream       = NULL;
  iface_session_t                  *session         = NULL;
  mct_stream_info_t                *mct_stream_info;
  iface_util_process_bundle_info_t  bundle_info;
  int32_t analysis_enable = 1;
#ifdef _ANDROID_
  char value[PROPERTY_VALUE_MAX];
#endif

  ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_IFACE_STREAMON);
  IFACE_HIGH("E, session_id %d mct_stream_id = %d",
    session_id, user_stream_id);
  session = iface_util_get_session_by_id(iface, session_id);
  if (!session){
      CDBG_ERROR("%s: cannot find session %d\n", __func__, session_id);
      ATRACE_CAMSCOPE_END(CAMSCOPE_IFACE_STREAMON);
      return -1;
  }

  user_stream = iface_util_find_stream_in_sink_port(iface_sink_port,
                                                    session_id,
                                                    user_stream_id);
  if (user_stream == NULL) {
    CDBG_ERROR("%s: stream not exist in this sinkport. error -1\n", __func__);
    ATRACE_CAMSCOPE_END(CAMSCOPE_IFACE_STREAMON);
    return -1;
  }

  if (0 == session->active_count) {
    iface->frame_cnt = 0;
    iface->skip_meta = 0;
    session->drop_reconfig = 0;
    session->dual_cam_drop = 0;
  }

  /* MCT does not pass the HAL buffer list pointer when link the stream.
     Here we save the HAL buffer list pointer for buffer mapping purpose */
  mct_stream_info =
    (mct_stream_info_t *)event->u.ctrl_event.control_event_data;
  user_stream->stream_info.img_buffer_list = mct_stream_info->img_buffer_list;
  user_stream->stream_info.num_bufs = mct_stream_info->num_bufs;

  CDBG("%s, streamon for id %d with buffer list 0x%x, num buf %d\n",
       __func__, user_stream_id, mct_stream_info->img_buffer_list,
       mct_stream_info->num_bufs);

  /* get all streamon hw streams by HAL bundling mask
     1. get all bundled user stream
     2. get hw streams by mapped mct stream info*/
  bundle_info.num_user_streams =
    iface_util_get_user_streams_by_bundle(session,
                                          user_stream_id,
                                          bundle_info.user_stream_ids,
                                          1);
  if (bundle_info.num_user_streams <= 0) {
    CDBG("%s: no user streams to streamon, num_user_streams = %d\n",
    __func__, bundle_info.num_user_streams);
    ATRACE_CAMSCOPE_END(CAMSCOPE_IFACE_STREAMON);
    return 0;
  }

  /* get all streamon hw streams by HAL bundling mask
     1. get all bundled user stream
     2. get hw streams by mapped mct stream info*/
  bundle_info.num_hw_streams =
    iface_util_decide_hw_streams(iface,
                                 session,
                                 bundle_info.num_user_streams,
                                 bundle_info.user_stream_ids,
                                 bundle_info.hw_stream_ids,
                                 1);
  if (bundle_info.num_hw_streams == 0) {
    /* If HW stream is running we just add extra info for new user stream. */
    rc = iface_util_add_user_stream_to_hw_stream(iface,
                                                 iface_sink_port,
                                                 session_id,
                                                 user_stream_id);

    IFACE_HIGH("X no hw streams to streamon\n");
    ATRACE_CAMSCOPE_END(CAMSCOPE_IFACE_STREAMON);
    return rc;
  }

  retValue =
    iface_decide_session_hw_stream(iface,
                                   session,
                                   &bundle_info.num_hw_streams,
                                   bundle_info.hw_stream_ids,
                                   1);
  if (retValue == false) {
    IFACE_ERR("%s: num_hw_streams = %d\n", __func__,
               bundle_info.num_hw_streams);

    iface_util_decide_hw_streams(iface,
                                 session,
                                 bundle_info.num_user_streams,
                                 bundle_info.user_stream_ids,
                                 bundle_info.hw_stream_ids,
                                 0);

    iface_util_get_user_streams_by_bundle(session,
                                          user_stream_id,
                                          bundle_info.user_stream_ids,
                                          0);

    ATRACE_CAMSCOPE_END(CAMSCOPE_IFACE_STREAMON);
    return -1;
  }

  CDBG_HIGH("%s: num_user_streams %d, num_hw_streams %d\n", __func__,
            bundle_info.num_user_streams, bundle_info.num_hw_streams);

  /* safety check */
  if (bundle_info.num_hw_streams > MAX_STREAM_NUM_IN_BUNDLE) {
    CDBG_ERROR("bundle stream number exceeds limit of %d\n",
      MAX_STREAM_NUM_IN_BUNDLE);
    bundle_info.num_hw_streams = MAX_STREAM_NUM_IN_BUNDLE;
  }

  /* alway return success*/
  rc = iface_util_process_bundle_streamonoff_list(session,
                                                  &bundle_info,
                                                  TRUE);
  if (rc < 0) {
    IFACE_ERR("iface_util_process_bundle_streamonoff_list failed\n");
  }

  /*update hw buf info by user buf info just updated on every streamon*/
  rc = iface_util_update_hw_buf_info(iface,
                                     iface_sink_port,
                                     session,
                                     bundle_info.mct_ids_for_default_bufq,
                                     bundle_info.num_mct_ids_for_default_bufq);
  if (rc < 0) {
    IFACE_ERR("iface_util_update_hw_buf_info failed\n");
  }


  /*request buffer: hal buffer or native buffer*/
  rc = iface_util_request_image_buf(iface,
                                    session,
                                    bundle_info.num_hw_streams,
                                    bundle_info.hw_stream_ids);
  if (rc < 0) {
    IFACE_ERR("iface_util_request_image_buf error = %d \n", rc);

    iface_decide_session_hw_stream(
        iface,
        session,
        &bundle_info.num_hw_streams,
        bundle_info.hw_stream_ids,
        0);

    iface_util_decide_hw_streams(
        iface,
        session,
        bundle_info.num_user_streams,
        bundle_info.user_stream_ids,
        bundle_info.hw_stream_ids,
        0);

    iface_util_get_user_streams_by_bundle(
        session,
        user_stream_id,
        bundle_info.user_stream_ids,
        0);

    ATRACE_CAMSCOPE_END(CAMSCOPE_IFACE_STREAMON);
    return rc;
  }

  /* send buffer list to imglib */
  rc = iface_util_send_buff_list_downstream(iface, session, bundle_info.num_hw_streams,
    bundle_info.hw_stream_ids, user_stream_id);
  if (rc < 0) {
    CDBG_ERROR("%s: iface_util_send_buff_list_downstream rc %d\n", __func__, rc);
    return rc;
  }

  if (session->hvx.enabled == TRUE) {
    session->first_pix_stream_on_off = FALSE;
    /* Find if this stream on is the first pix stream on */
    rc = iface_update_pix_streamon_info(session,
                                        bundle_info.num_hw_streams,
                                        bundle_info.hw_stream_ids,
                                        TRUE);
    if (!rc) {
      if (session->first_pix_stream_on_off == TRUE) {
        session->first_pix_stream_on_off = FALSE;
        /* Stream ON HVX before starting CAMIF */
        rc = iface_hvx_streamon(&session->hvx);
        if (rc < 0) {
          IFACE_ERR("failed: iface_hvx_streamon rc %d\n", rc);
        }
      }
    } else {
      IFACE_ERR("failed: iface_update_pix_streamon_info rc %d\n",
        rc);
    }
  }

  pthread_mutex_lock(&iface->mutex);
  /* for testgen let isp complete before axi stream start */
  if (session->use_testgen || session->sensor_out_info.is_dummy) {
    sem_wait(&session->session_thread.ispif_start_sem);
  }
#ifdef _ANDROID_
  if(user_stream->stream_info.stream_type== CAM_STREAM_TYPE_ANALYSIS) {
    property_get("persist.vendor.camera.isp.analysis_en", value, "1");
    analysis_enable = atoi(value);
  }
#endif
  if ((user_stream->stream_info.stream_type != CAM_STREAM_TYPE_ANALYSIS) ||
    ((user_stream->stream_info.stream_type == CAM_STREAM_TYPE_ANALYSIS) &&
    analysis_enable)) {
    if (!session->hvx.enabled) {
         session->first_pix_stream_on_off = FALSE;
         /* Find if this stream on is the first pix stream on */
         rc = iface_update_pix_streamon_info(session,
                                           bundle_info.num_hw_streams,
                                           bundle_info.hw_stream_ids,
                                           TRUE);
         if (!rc) {
           if (session->first_pix_stream_on_off == TRUE) {
             session->first_pix_stream_on_off = FALSE;
           }
         } else {
           IFACE_ERR("failed: iface_update_pix_streamon_info rc %d\n",
             rc);
         }
      }
      rc = iface_util_axi_streamon(iface,
                                   session,
                                   bundle_info.num_hw_streams,
                                   bundle_info.hw_stream_ids);
      if (rc < 0) {
        IFACE_ERR("iface_axi_streamon error = %d \n", rc);

        iface_util_cleanup_axi_stream_on_failure(
            iface,
            session,
            bundle_info.num_hw_streams,
            bundle_info.num_user_streams,
            bundle_info.hw_stream_ids,
            user_stream_id,
            bundle_info.user_stream_ids);
        pthread_mutex_unlock(&iface->mutex);
        ATRACE_CAMSCOPE_END(CAMSCOPE_IFACE_STREAMON);
        return rc;
      }
  }

  /* ispif stream on after axi streamon*/
  if (!session->use_testgen && !session->sensor_out_info.is_dummy) {
    sem_wait(&session->session_thread.ispif_start_sem);
  }

  if (!iface_sink_port->u.sink_port.sensor_out_info.is_dummy) {
     rc = iface_ispif_proc_streamon(iface,
                                    session,
                                    iface_sink_port,
                                    bundle_info.num_hw_streams,
                                    bundle_info.hw_stream_ids);
     if (rc < 0) {
       IFACE_ERR("iface_ispif_proc_streamon error, rc = %d\n", rc);
       iface_util_cleanup_ispif_stream_on_failure(
             iface,
             session,
             bundle_info.num_hw_streams,
             bundle_info.num_user_streams,
             bundle_info.hw_stream_ids,
             user_stream_id,
             bundle_info.user_stream_ids,
             iface_sink_port);

       pthread_mutex_unlock(&iface->mutex);
       ATRACE_CAMSCOPE_END(CAMSCOPE_IFACE_STREAMON);
       return rc;
     }
   }
  pthread_mutex_unlock(&iface->mutex);
  /* update state: stream state and port state*/
  for (i = 0; i < bundle_info.num_hw_streams; i++) {
    hw_stream =
      iface_util_find_hw_stream_in_session(session,
                                           bundle_info.hw_stream_ids[i]);
    if (hw_stream == NULL)
       continue;

    hw_stream->streaming_mode = mct_stream_info->streaming_mode;
    hw_stream->num_burst = mct_stream_info->num_burst;

    hw_stream->state = IFACE_HW_STREAM_STATE_ACTIVE;
  }

  session->active_count += bundle_info.num_hw_streams;
  iface_sink_port->state = IFACE_PORT_STATE_ACTIVE;

  /* now we need to process the mct stream using shared bufq */
  rc = iface_util_process_mct_stream_for_shared_bufq(iface,
                                                     iface_sink_port,
                                                     session,
                                                     &bundle_info,
                                                     TRUE);
  IFACE_HIGH("X\n");
  ATRACE_CAMSCOPE_END(CAMSCOPE_IFACE_STREAMON);
  return rc;
}


/** iface_streamoff:
 *    @iface:           iface object pointer
 *    @iface_sink_port  iface sink port object pointer
 *    @session_id:      session id for the stream off
 *    @user_stream_id:  mct stream id
 *    @event:           streamoff event
 *
 *  This function streamoff the mct stream specifeid by the
 *  MCT/HAL. It stopped the hw stream, release the bufq as
 *  required. It also stops ispif streaming
 *
 *  Return:  0 - Success
 *          -1 - Invalid stream/session ID or
 *               unsuccessful stopping of stream
 **/
int iface_streamoff(
    iface_t         *iface,
    iface_port_t    *iface_sink_port,
    uint32_t         session_id,
    uint32_t         user_stream_id,
    mct_event_t     *event __unused)
{
  uint32_t                           i;
  int                                rc           = 0;
  boolean                            retValue     = true;
  iface_stream_t                    *user_stream  = NULL;
  iface_hw_stream_t                 *hw_stream    = NULL;
  iface_session_t                   *session      = NULL;
  iface_util_process_bundle_info_t   bundle_info;
  enum msm_vfe_axi_stream_cmd        stop_cmd     = 0;
  mct_stream_t *stream = NULL;
  mct_list_t *stream_list = NULL;
  mct_pipeline_t *pipeline = NULL;
  int32_t analysis_enable = 1;
#ifdef _ANDROID_
  char value[PROPERTY_VALUE_MAX];
#endif

  ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_IFACE_STREAMOFF);
  IFACE_HIGH("E, session_id %d mct_stream_id = %d\n",
    session_id, user_stream_id);

  session = iface_util_get_session_by_id(iface, session_id);
  if (!session){
    CDBG_ERROR("%s: cannot find session %d\n", __func__, session_id);
    ATRACE_CAMSCOPE_END(CAMSCOPE_IFACE_STREAMOFF);
    return -1;
  }

  user_stream = iface_util_find_stream_in_sink_port(iface_sink_port,
                                                    session_id,
                                                    user_stream_id);
  if (user_stream == NULL) {
    CDBG_ERROR("%s: stream not exist in this sinkport. error -1\n", __func__);
    ATRACE_CAMSCOPE_END(CAMSCOPE_IFACE_STREAMOFF);
    return -1;
  }

  if(user_stream->stream_info.stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    for (i = 0; i< IFACE_MAX_STREAMS; i++) {
      if (session->hw_streams[i].stream_info.cam_stream_type ==
        CAM_STREAM_TYPE_OFFLINE_PROC) {
        break;
      }
    }
    if (i < IFACE_MAX_STREAMS) {
      rc = iface_offline_stream_unconfig(iface,
                                         session->hw_streams[i].hw_stream_id,
                                         session_id);
    }
    ATRACE_CAMSCOPE_END(CAMSCOPE_IFACE_STREAMOFF);
    return rc;
  }

    /* Get iface module's parent - stream */
  stream_list = mct_list_find_custom(MCT_MODULE_PARENT(iface->module), &session_id,
    iface_find_stream_by_sessionid);
  if (!stream_list) {
    CDBG_ERROR("%s:%d failed stream_list %p\n",
      __func__, __LINE__, stream_list);
    return -1;
  }
  if (!stream_list->data) {
    CDBG_ERROR("%s:%d failed stream_list->data %p", __func__, __LINE__,
      stream_list->data);
    return -1;
  }
  stream = (mct_stream_t *)stream_list->data;
  /* Get stream's parent - pipeline */
  pipeline = MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data);
  if (!pipeline) {
    CDBG_ERROR("%s:%d failed pipeline %p\n", __func__, __LINE__, pipeline);
    return -1;
  }

  /* get all bundled user stream */
  bundle_info.num_user_streams =
    iface_util_get_user_streams_by_bundle(session,
                                          user_stream_id,
                                          bundle_info.user_stream_ids,
                                          0);
  if (bundle_info.num_user_streams == 0) {
    CDBG("%s: no user streams to streamoff for user stream %d",
         __func__, user_stream_id);
    goto end;
  }
  if (bundle_info.num_user_streams > MAX_STREAM_NUM_IN_BUNDLE) {
    CDBG_ERROR("%s:%d: failed: num_user_streams %d max %d\n",
               __func__, __LINE__, bundle_info.num_user_streams,
               MAX_STREAM_NUM_IN_BUNDLE);
    ATRACE_CAMSCOPE_END(CAMSCOPE_IFACE_STREAMOFF);
    return -1;
  }

  /*  get hw streams by mapped mct stream info*/
  bundle_info.num_hw_streams =
    iface_util_decide_hw_streams(iface,
                                 session,
                                 bundle_info.num_user_streams,
                                 bundle_info.user_stream_ids,
                                 bundle_info.hw_stream_ids,
                                 0);
  if (bundle_info.num_hw_streams == 0) {
    CDBG("%s: no hw stream to streamoff, streamoff user stream %d\n",
         __func__, user_stream_id);
    hw_stream = iface_util_find_hw_stream_by_mct_id(iface,
                                                    session,
                                                    user_stream_id);
    if (hw_stream == NULL) {
       CDBG_ERROR("%s: no hw stream found, ERROR!"
                  "mct stream id = %x, rc = -1\n",
                  __func__, user_stream_id);
       ATRACE_CAMSCOPE_END(CAMSCOPE_IFACE_STREAMOFF);
       return -1;
    }
    if (hw_stream->shared_hw_stream &&
        user_stream->stream_info.stream_type != CAM_STREAM_TYPE_SNAPSHOT) { //do not change stream state when snapshot stream is shared.
      hw_stream->state = IFACE_HW_STREAM_STATE_STOPPING;
    }
    if (hw_stream->shared_hw_stream) {
      CDBG("%s: stream off on shared hw stream \n", __func__);
      rc = iface_util_remove_bufq_from_stream(session,
                                              iface,
                                              hw_stream->hw_stream_id,
                                              user_stream_id);
      if (rc) {
        CDBG_ERROR("%s: failed: iface_util_add_bufq_to_stream\n", __func__);
      }

      rc = iface_util_axi_unreg_shared_bufq(session,
                                            iface,
                                            hw_stream->hw_stream_id,
                                            user_stream_id);
      if (rc) {
        CDBG_ERROR("%s: iface_util_axi_unreg_shared_bufq error = %d\n",
                   __func__, rc);
      }
    }
    goto end;
  }

  retValue =
    iface_decide_session_hw_stream(
        iface,
        session,
        &bundle_info.num_hw_streams,
        bundle_info.hw_stream_ids,
        0);
  if (retValue == false) {
    CDBG_ERROR("%s:  num_hw_streams = %d\n",
               __func__, bundle_info.num_hw_streams);
    ATRACE_CAMSCOPE_END(CAMSCOPE_IFACE_STREAMOFF);
    return -1;
  }

  if (session->hvx.enabled == TRUE) {
    session->first_pix_stream_on_off = FALSE;
    /* Find if this stream on is the first pix stream on */
    rc = iface_update_pix_streamon_info(session,
                                        bundle_info.num_hw_streams,
                                        bundle_info.hw_stream_ids,
                                        FALSE);
    if (!rc) {
      if (session->first_pix_stream_on_off == TRUE) {
        session->first_pix_stream_on_off = FALSE;
        /* Stream OFF HVX before stopping CAMIF */
        rc = iface_hvx_streamoff(&session->hvx);
        if (rc < 0) {
          CDBG_ERROR("%s:%d failed: iface_hvx_streamon rc %d\n", __func__,
            __LINE__, rc);
        }
      }
    } else {
      CDBG_ERROR("%s:%d failed: iface_update_pix_streamon_info rc %d\n",
        __func__, __LINE__, rc);
    }
  }

  /* safety check */
  if (bundle_info.num_hw_streams > MAX_STREAM_NUM_IN_BUNDLE) {
    CDBG_ERROR("bundle stream number exceeds limit of %d\n",
      MAX_STREAM_NUM_IN_BUNDLE);
    bundle_info.num_hw_streams = MAX_STREAM_NUM_IN_BUNDLE;
  }
  rc = iface_util_process_bundle_streamonoff_list(session,
                                                  &bundle_info,
                                                  FALSE);

  session->active_count -= bundle_info.num_hw_streams;

  pthread_mutex_lock(&iface->mutex);
  /* ispif streamoff*/
  if (!iface_sink_port->u.sink_port.sensor_out_info.is_dummy) {
    /*skip update stream count if hvx enabled*/
    if (!session->hvx.enabled) {
        session->first_pix_stream_on_off = FALSE;
        /* Find if this stream on is the first pix stream on */
        rc = iface_update_pix_streamon_info(session,
                                            bundle_info.num_hw_streams,
                                            bundle_info.hw_stream_ids,
                                            FALSE);
        if (!rc) {
          if (session->first_pix_stream_on_off == TRUE) {
            session->first_pix_stream_on_off = FALSE;
          }
        } else {
          CDBG_ERROR("%s:%d failed: iface_update_pix_streamon_info rc %d\n",
            __func__, __LINE__, rc);
        }
    }
    rc = iface_ispif_streamoff(iface,
                               session,
                               iface_sink_port,
                               bundle_info.num_hw_streams,
                               bundle_info.hw_stream_ids, &stop_cmd);
    if (rc <  0) {
      CDBG_ERROR("%s: error! ispif streamon failed! rc = %d\n", __func__, rc);
    }
  }
#ifdef _ANDROID_
    if(user_stream->stream_info.stream_type== CAM_STREAM_TYPE_ANALYSIS) {
      property_get("persist.vendor.camera.isp.analysis_en", value, "1");
      analysis_enable = atoi(value);
    }
#endif

  /*axi streamoff*/
  if ((user_stream->stream_info.stream_type != CAM_STREAM_TYPE_ANALYSIS) ||
    ((user_stream->stream_info.stream_type == CAM_STREAM_TYPE_ANALYSIS) &&
    analysis_enable))
    rc = iface_util_axi_streamoff(iface,
                                  session,
                                  bundle_info.num_hw_streams,
                                  bundle_info.hw_stream_ids, stop_cmd);
  if (rc < 0) {
    CDBG_ERROR("%s: iface_util_axi_streamoff error = %d \n", __func__, rc);
  }
  pthread_mutex_unlock(&iface->mutex);

  /* now we need to process the mct stream that using the shared bufq */
  rc = iface_util_process_mct_stream_for_shared_bufq(iface,
                                                     iface_sink_port,
                                                     session,
                                                     &bundle_info,
                                                     FALSE);

  /* update state: stream state state */
  for (i = 0; i < bundle_info.num_hw_streams; i++) {
    hw_stream =
      iface_util_find_hw_stream_in_session(session,
                                           bundle_info.hw_stream_ids[i]);
    if (hw_stream == NULL)
       continue;

    hw_stream->state = IFACE_HW_STREAM_STATE_HW_CFG_DONE;

    /*release only HAL buffer for next stream on buf remapping */
    if (hw_stream->use_native_buf == 0) {
      iface_util_release_image_buf(iface, session, hw_stream);
      CDBG("%s: hw_stream id %x mct_stream_id %x user_stream_id %x\n",
           __func__, hw_stream->hw_stream_id, hw_stream->mapped_mct_stream_id[0],
           user_stream_id);
    }
  }

  IFACE_HIGH("session_id = %d, active_streams = %d\n",
    session->session_id, session->active_count);

end:
  if ((session->num_bundled_streamon == 0) && (!pipeline->start_int_streamoff_on)) {
    IFACE_HIGH("all bundled stream streamoff, clear hal bundled mask\n");
    memset(&session->hal_bundle_parm, 0, sizeof(session->hal_bundle_parm));
  }

  ATRACE_CAMSCOPE_END(CAMSCOPE_IFACE_STREAMOFF);
  IFACE_HIGH("X\n") ;
  return rc;
}

int iface_streamoff_post_isp(
    iface_t         *iface,
    iface_port_t    *iface_sink_port,
    iface_session_t *session)
{
  int                                rc = 0;
  iface_hw_stream_t                 *hw_stream    = NULL;
  int                                i;

  /*only reset all and release hw when all the user streamoff comes*/
  if (session->active_count == 0 && session->num_bundled_streamon == 0) {
    /* update state: stream state state*/
    for (i = 0; i < IFACE_MAX_STREAMS; i++) {
      hw_stream = &session->hw_streams[i];
      if ((hw_stream->hw_stream_id != 0) &&
        (hw_stream->state != IFACE_HW_STREAM_STATE_INITIAL) &&
        (hw_stream->stream_info.cam_stream_type !=
        CAM_STREAM_TYPE_OFFLINE_PROC)) {
        if (hw_stream->use_native_buf == 1) {
	  IFACE_DBG("free native buf session: %d hw_stream: %d\n",
	  session->session_id, hw_stream->hw_stream_id);
          iface_util_release_image_buf(iface, session, hw_stream);
        }
      }
    }
    IFACE_DBG("session id = 0x%X,Delta between vfe irqs %ld sec %ld usec\n",
      session->session_id, session->delta_time_stamp.tv_sec,
      session->delta_time_stamp.tv_usec);
    session->delta_time_stamp.tv_sec = 0;
    session->delta_time_stamp.tv_usec = 0;

    /* reset dynamic_stats_skip_feature_enb flag */
    session->dynamic_stats_skip_feature_enb = FALSE;

    /* reset the sof frame id to zero.*/
    session->sof_frame_id = 0;
    session->prev_sof_frame_id = 0;
    session->drop_reconfig = 0;
    session->dual_cam_drop = 0;
    session->cds_status = IFACE_CDS_STATUS_IDLE;
    session->cds_saved_state = session->cds_curr_enb;
    session->cds_curr_enb = 0;
    memset(&session->cds_info, 0, sizeof(iface_cds_info_t));
    iface_ispif_proc_close(iface);
    /* one sink port one session for iface*/
    iface_sink_port->state = IFACE_PORT_STATE_RESERVED;

    session->num_bundle_meta = 0;
    memset(&session->bundle_meta_info[0], 0,
      IFACE_MAX_STREAMS * sizeof(iface_bundle_meta_info_t));

    session->num_pdaf_meta = 0;
    memset(session->bundle_pdaf_info, 0,
      IFACE_MAX_STREAMS * sizeof(iface_bundle_pdaf_info_t));

    /* all stream off, un config axi streams*/
    rc = iface_util_unconfig_axi(iface, session);
    if (rc < 0) {
      CDBG_ERROR("%s: iface_util_unconfig_axi error = %d \n", __func__, rc);
    }

    /* when  no active stream running for this session
       we can loop through it to release isp resource.*/
    for (i = 0; i < IFACE_MAX_STREAMS; i++) {
      if (session->hw_streams[i].stream_info.cam_stream_type ==
          CAM_STREAM_TYPE_OFFLINE_PROC) {
        CDBG_HIGH("%s: offline stream resources %x will be released once done\n",
          __func__, __LINE__, hw_stream->hw_stream_id);
        continue;
      }
      CDBG("%s: hw_stream %p type %d axi_path %d id %x\n", __func__,
        &session->hw_streams[i], session->hw_streams[i].stream_type,
        session->hw_streams[i].axi_path, session->hw_streams[i].hw_stream_id);
      if (session->hw_streams[i].hw_stream_id) {
        iface_util_release_resource(iface, session, &session->hw_streams[i]);
      }

      if (session->stats_streams[i].stats_stream_id) {
        memset(&session->stats_streams[i], 0 , sizeof(iface_stats_stream_t));
      }
    }
    session->num_stats_stream = 0;

    rc = iface_clear_hal_param_q(iface,
                                 session,
                                 session->parm_q,
                                 session->parm_q_lock);
    if (rc < 0) {
      IFACE_ERR("iface_clear_hal_param_q error = %d \n", rc);
    }

    /* clean up fetch engine data */
    session->fe.busy = FALSE;
    session->fe.num = 0;
    session->fe.next_free_entry = 0;
    session->frame_request.hw_stream_id = 0;
  }
  return rc;
}

/** iface_halt_recovery
 *    @iface: iface handle
 *    @session id: session id
 *    @isp_id: ISP on which overflow detected
 *
 *  1. HALT ISP
 *  2. STOP ISPIF INTF
 *  3. RESET ISP
 *  4. RESTART ISP
 *  5. RESET ISPIF INTF
 *  6. RESTART ISPIF INTF
 *
 * Return: 0 - success and negative value - failure
 **/
int iface_halt_recovery(
  iface_t *iface,
  uint32_t *session_id,
  uint32_t  isp_id __unused,
  uint32_t  halt_frame_id)
{
  int rc = 0;
  iface_session_t            *session = NULL;
  uint32_t                    hw_stream_ids[IFACE_MAX_STREAMS];
  uint32_t                    num_hw_streams = 0;
  iface_ispif_t              *ispif = &iface->ispif_data.ispif_hw;
  struct ispif_cfg_data      *cfg_cmd = &ispif->cfg_cmd;
  iface_axi_recovery_state_t  recovery_state = 0;
  iface_port_t               *iface_sink_port = NULL;
  uint32_t                    i = 0;
  uint32_t                    buf_done_frame_id = 0;
  int                         axi_idx;
  iface_hvx_t                 *hvx;

  if (!iface || !session_id) {
    CDBG_ERROR("%s: iface %p, rc = -1\n",
      __func__, iface );
    return -1;
  }

  iface_sink_port = iface_util_find_sink_port_by_session_id(iface, session_id);
  if (!iface_sink_port) {
    CDBG_ERROR("%s: can not find iface sink port for this session, rc = -1\n", __func__);
    return -1;
  }

  session =
    iface_util_get_session_by_id(iface, *session_id);
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n", __func__, *session_id);
    return IFACE_AXI_RECOVERY_STATE_RECOVERY_FAILED;
  }

  if (session->active_count == 0) {
    CDBG_ERROR("%s: all stream has stopped, no need to recover halt!\n", __func__);
    return 0;
  }

  hvx = &session->hvx;
    /* Call hvx_streamoff */
  if (session->hvx.enabled == TRUE) {
    rc = iface_hvx_streamoff(hvx);
    if (rc < 0) {
      CDBG_ERROR("failed: iface_hvx_streamoff rc %d\n", rc);
      return rc;
    }
  }

  /* stop ispif */
  IFACE_HIGH("STOP ISPIF ... \n");
  memset(&hw_stream_ids, 0, sizeof(uint32_t) * IFACE_MAX_STREAMS);
  num_hw_streams = iface_util_get_hw_streams_ids_in_session(iface, session, hw_stream_ids);

  rc = iface_ispif_get_cfg_params_from_hw_streams(iface, iface_sink_port,
    session, num_hw_streams, hw_stream_ids, FALSE);
  if (rc < 0 )
    IFACE_ERR("iface_ispif_get_cfg_params_from_hw_streams failed !!\n");

  if (cfg_cmd->params.num == 0) {
    IFACE_ERR("ISPIF configure cmd is seriously wrong\n");
    rc = -1;
    goto error;
  }
  rc = iface_ispif_call_ioctl_ext(ISPIF_STOP_FRAME_BOUNDARY, ispif);
  if (rc != 0) {
    IFACE_ERR("ISPIF_START_FRAME_BOUNDARY error = %d\n", rc);
    goto error;
  }

  /* start HALT AXI fro dual vfe */
  IFACE_HIGH("HALT AXI... \n");
  for (axi_idx = 0; axi_idx < VFE_MAX; axi_idx++) {
    if (session->session_resource.isp_id_mask & (1 << axi_idx)) {
        iface->isp_axi_data.axi_data[axi_idx].axi_hw_ops->action(
          iface->isp_axi_data.axi_data[axi_idx].axi_hw_ops->ctrl,
          IFACE_AXI_ACTION_CODE_HALT_HW, NULL, 0);
    }
  }

  /* RESET HW: reset buf done frame id
     on session SOF frame id didnt get proper output frame,
     reset to (frame - 1) which hassuccessfully done both SOF and BUF output
     once restart, will restart the frame id which posted SOF but no BUF done
     in this case, skip the first SOF notify which has been posted. */
  IFACE_HIGH("RESET VFE HW ... \n");
  buf_done_frame_id = halt_frame_id - 1;
  for (i = 0; i < VFE_MAX; i++) {
    if (session->session_resource.isp_id_mask & (1 << i)) {
      CDBG("%s:%d  VFE%d \n", __func__, __LINE__, i);
      iface->isp_axi_data.axi_data[i].axi_hw_ops->action(
        iface->isp_axi_data.axi_data[i].axi_hw_ops->ctrl,
        IFACE_AXI_ACTION_CODE_RESET_HW, &buf_done_frame_id, sizeof(buf_done_frame_id));
    }
  }

  /* Call hvx_streamon before CAMIF start  */
  if (session->hvx.enabled == TRUE) {
    rc = iface_hvx_streamon(hvx);
    if (rc < 0) {
      IFACE_ERR("failed: iface_hvx_streamon rc %d\n", rc);
      goto error;
    }
  }

  IFACE_HIGH("RESTART AXI ... \n");
  for (i = 0; i< VFE_MAX; i++) {
    if (session->session_resource.isp_id_mask & (1 << i)) {
        CDBG("%s:%d VFE%d \n", __func__, __LINE__, i);
        iface->isp_axi_data.axi_data[i].axi_hw_ops->action(
          iface->isp_axi_data.axi_data[i].axi_hw_ops->ctrl,
          IFACE_AXI_ACTION_CODE_RESTART_HW, NULL, 0);
    }
  }

  IFACE_HIGH("RESTART ISPIF ... \n");
  /* reconfig ispif and restart*/
  rc = iface_ispif_get_cfg_params_from_hw_streams(iface, iface_sink_port,
    session, num_hw_streams, hw_stream_ids, TRUE);
  if (rc < 0 )
    IFACE_ERR("iface_ispif_get_cfg_params_from_hw_streams failed !!\n");

  if (cfg_cmd->params.num == 0) {
    IFACE_ERR("ISPIF configure cmd is seriously wrong\n");
    rc = -1;
    goto error;
  }
  rc = iface_ispif_call_ioctl_ext(ISPIF_RESTART_FRAME_BOUNDARY, ispif);
  if (rc != 0) {
    IFACE_ERR("%s: ISPIF_START_FRAME_BOUNDARY error = %d\n", __func__, rc);
    goto error;
  }

  /* re config ispif and restart reset state to
     IFACE_AXI_RECOVERY_STATE_NO_OVERFLOW after skip 1st SOF */
  IFACE_HIGH("Overflow recovery done! skip 1st SOF and prepare missed buf\n");

error:
  return rc;
}
/** iface_set_hal_param:
*
*  @iface: handle to iface
*  @iface_sink_port: sink port handle where this event arrived
*  @session_id: current session id
*  @stream_id: current stream id
*  @param: control event handle
*
*  Set diff control paramsreceived from HAL, such as sharpness,
*  cpntrast, specialeffects, rolloff, saturation , mce, sce,
*  recording hint etc.
*
*  Returns 0 on success and negative error on failure
**/
int iface_set_hal_param(iface_t *iface, iface_port_t *iface_sink_port,
  iface_session_t *session, uint32_t stream_id, mct_event_control_parm_t *param,
  uint32_t frame_id)
{
  int rc = 0;

  if (!iface || !iface_sink_port || !param || !session) {
    CDBG_ERROR("%s:%d %p %p %p %p\n", __func__, __LINE__, iface, iface_sink_port,
      param, session);
    return -EINVAL;
  }

  CDBG("%s: E, iface %p, session_id %d stream_id %d param_type %d",
    __func__, (void *)iface,
    session->session_id, stream_id, param->type);
  switch (param->type) {

  case CAM_INTF_PARM_HFR: {
    rc = iface_util_set_hfr(iface, session, stream_id,
     (int32_t *)param->parm_data);
  }
    break;

  case CAM_INTF_PARM_BESTSHOT_MODE: {
    rc = iface_util_handle_bestshot(iface, session->session_id,
      (cam_scene_mode_type *)param->parm_data);
    break;
  }

  case CAM_INTF_PARM_FRAMESKIP: {
    /*when HAL request, skip for all streams with agreement with HAL team*/
    rc = iface_util_set_hal_frame_skip(iface, session,
      (int32_t *)param->parm_data);
  }
    break;

  case CAM_INTF_META_STREAM_ID: {
    rc = iface_util_request_frame_by_stream_ids(iface, session->session_id,
      (void *)param->parm_data, frame_id);
  }
    break;

  case CAM_INTF_PARM_HAL_VERSION: {
    rc = iface_util_set_hal_version(iface, session->session_id,
      (cam_hal_version_t *)param->parm_data);
  }
    break;

  case CAM_INTF_PARM_CDS_MODE: {
    rc = iface_util_set_cds_mode(iface,
      session->session_id, param->parm_data);
  }
    break;

  case CAM_INTF_PARM_VT:
  case CAM_INTF_META_USE_AV_TIMER: {
    rc = iface_util_set_vt(iface, session->session_id,
      (int32_t *)param->parm_data);
  }
    break;
  case CAM_INTF_META_STREAM_INFO:{
    rc = iface_util_handle_meta_stream_info(iface, session->session_id, stream_id,
       (cam_stream_size_info_t *)param->parm_data);
    break;
  }

  case CAM_INTF_PARM_UPDATE_DEBUG_LEVEL: {
    rc = iface_util_setloglevel_frm_hal((uint32_t *)param->parm_data);
  }
    break;

  case CAM_INTF_PARM_DIS_ENABLE: {
    rc = iface_util_set_dis_enable_flag(iface, session->session_id,
      (int32_t *)param->parm_data);
  }
    break;

  case CAM_INTF_PARM_SENSOR_HDR:{
      rc = iface_util_set_sensor_hdr(iface, session->session_id, stream_id,
      (cam_sensor_hdr_type_t *)param->parm_data);
  }
    break;

  case CAM_INTF_META_BINNING_CORRECTION_MODE: {
    session->binncorr_mode= *(cam_binning_correction_mode_t *)param->parm_data;
    CDBG("%s: binn_corr_mode %d", __func__, session->binncorr_mode);
  }
    break;

  case CAM_INTF_PARM_ADV_CAPTURE_MODE: {
    rc = iface_util_handle_adv_capturemode(iface, session->session_id,
      (uint32_t*)param->parm_data);
    break;
  }

  case CAM_INTF_PARM_VFE1_RESERVED_RDI: {
    iface->vfe1_reserved_rdi = *((int*)param->parm_data);
    rc = 0;
    break;
  }

  case CAM_INTF_META_CAPTURE_INTENT: {
    if (*(cam_intent_t *)param->parm_data == CAM_INTENT_STILL_CAPTURE) {
        session->cds_skip_disable = FALSE;
        session->skip_cds_timeout = SKIP_CDS_FRAMES_THRESHOLD;
    } else if ((*(cam_intent_t *)param->parm_data != CAM_INTENT_STILL_CAPTURE) &&
        (session->skip_cds_timeout == 0)) {
        session->cds_skip_disable = TRUE;
    }
    break;
  }
  default: {
    CDBG("%s: Type %d not supported", __func__, param->type);
  }
    break;
  }

  return rc;
}

/** iface_store_hal_param:
*
*  @iface: handle to iface
*  @session_id: current session id
*  @stream_id: current stream id
*  @param: control event handle
*  @parm_q: array of param queues
*  @parm_q_lock: array of mutexes for param queue
*
*  Copy hal param to local queue.
*
*  Returns 0 on success and negative error on failure
**/
static int iface_store_hal_param(iface_t *iface, iface_port_t *iface_sink_port,
  uint32_t session_id, uint32_t stream_id, mct_event_control_parm_t *param,
  mct_queue_t *parm_q, pthread_mutex_t *parm_q_lock)
{
  iface_frame_ctrl_data_t *frame_ctrl_data = NULL;
  uint32_t size = 0;
  int rc = 0;

  if (!iface || !param || !parm_q) {
    CDBG_ERROR("%s:%d %p %p %p\n", __func__, __LINE__, iface, param, parm_q);
    return -EINVAL;
  }

  switch (param->type) {
  case CAM_INTF_PARM_HFR:
    size = sizeof(int32_t);
    break;

  case CAM_INTF_PARM_FRAMESKIP:
    size = sizeof(int32_t);
    break;

  case CAM_INTF_META_STREAM_ID:
    size = sizeof(cam_stream_ID_t);
    break;

  case CAM_INTF_PARM_CDS_MODE:
     size = sizeof(int32_t);
     break;

 case CAM_INTF_PARM_SENSOR_HDR:
    size = sizeof(int32_t);
    break;

  case CAM_INTF_META_BINNING_CORRECTION_MODE:
    size = sizeof(int32_t);
    break;

  default:
    size = 0;
    break;
  }

  if (size) {
    frame_ctrl_data = malloc(sizeof(iface_frame_ctrl_data_t));
    if(!frame_ctrl_data) {
      CDBG_ERROR("%s: param malloc failed\n", __func__);
      return -1;
    }
    frame_ctrl_data->type = IFACE_PARAM_Q_HAL_PARAM;
    frame_ctrl_data->session_id = session_id;
    frame_ctrl_data->stream_id = stream_id;
    frame_ctrl_data->iface_sink_port = iface_sink_port;
    frame_ctrl_data->ctrl_param.type = param->type;
    frame_ctrl_data->ctrl_param.parm_data = malloc(size);
    if (!frame_ctrl_data->ctrl_param.parm_data) {
      free(frame_ctrl_data);
      CDBG_ERROR("%s: parm_data malloc failed\n", __func__);
      return -1;
    }
    memcpy(frame_ctrl_data->ctrl_param.parm_data, param->parm_data, size);

    pthread_mutex_lock(parm_q_lock);
    mct_queue_push_tail(parm_q, (void *)frame_ctrl_data);
    pthread_mutex_unlock(parm_q_lock);
  }

  return rc;
}

/** iface_restore_hal_param:
*
*  @iface: handle to iface
*  @parm_q: array of param queues
*  @parm_q_lock: array of mutexes for param queue
*
*  Apply stored in queue HAL params
*
*  Returns 0 on success and negative error on failure
**/
static int iface_restore_hal_param(iface_t *iface, iface_session_t *session,
  mct_queue_t *parm_q, pthread_mutex_t *parm_q_lock, uint32_t frame_id)
{
  iface_frame_ctrl_data_t *frame_ctrl_data = NULL, *temp = NULL;
  int rc = 0;
  iface_axi_hw_t  *axi_hw = NULL;
  uint32_t drop_reconfig;

  if (!iface || !session || !parm_q || !parm_q_lock) {
    CDBG_ERROR("%s:%d failed %p %p %p %p\n", __func__, __LINE__, iface, session,
      parm_q, parm_q_lock);
    return -EINVAL;
  }
  while (1) {
    pthread_mutex_lock(parm_q_lock);
    frame_ctrl_data = mct_queue_look_at_head(parm_q);
    if (frame_ctrl_data &&
       frame_ctrl_data->ctrl_param.type != CAM_INTF_META_STREAM_ID)
       frame_ctrl_data = mct_queue_pop_head(parm_q);
    pthread_mutex_unlock(parm_q_lock);

    if (!frame_ctrl_data)
      break;

    if (frame_ctrl_data->type == IFACE_PARAM_Q_HAL_PARAM) {
      rc |= iface_set_hal_param(iface, frame_ctrl_data->iface_sink_port,
        session, frame_ctrl_data->stream_id,
        &frame_ctrl_data->ctrl_param, frame_id);
        if (session->session_resource.isp_id_mask & (1 << VFE0)) {
           axi_hw =
              (iface_axi_hw_t *)iface->isp_axi_data.axi_data[VFE0].axi_hw_ops->ctrl;
        }
        if (session->session_resource.isp_id_mask & (1 << VFE1) && !session->dual_isp_sync) {
           axi_hw =
             (iface_axi_hw_t *)iface->isp_axi_data.axi_data[VFE1].axi_hw_ops->ctrl;
        }
        if ((axi_hw != NULL) && axi_hw->kstate_ptr.isp_kstate_ptr) {
           drop_reconfig = axi_hw->kstate_ptr.isp_kstate_ptr->drop_reconfig;
        }
        /* if the request_frame is sucess drop_reconfig will be zero
         * so go ahead and delete the entry in param_q.
         * In case request_frame failure, kernel driver would set
         * drop_reconfig flag to indicate request again on next sof.
         * To handle this we should avoid deleting the request entry in
         * the param_q and adjust the entry to next sof. Store the flag
         * in session structure, actual adjust is done in next control sof.
         */
        if (frame_ctrl_data->ctrl_param.type == CAM_INTF_META_STREAM_ID) {
           if ((drop_reconfig == 0) || (frame_id == 0)) {
              pthread_mutex_lock(parm_q_lock);
              temp = mct_queue_pop_head(parm_q);
              pthread_mutex_unlock(parm_q_lock);
              if (frame_ctrl_data->ctrl_param.parm_data) {
                  free(frame_ctrl_data->ctrl_param.parm_data);
                  frame_ctrl_data->ctrl_param.parm_data = NULL;
              }
              free(frame_ctrl_data);
              frame_ctrl_data = NULL;
           }
           else {
              IFACE_HIGH("drop_reconfig set from kernel, keep the settings\n");
              session->drop_reconfig = drop_reconfig;
              break;
           }
        }
    } else if (frame_ctrl_data->type == IFACE_PARAM_Q_REPORT_METADATA) {
      if (frame_ctrl_data->iface_metadata.type == MCT_BUS_MSG_FRAME_DROP) {
        rc |= iface_util_post_bus_msg(session, frame_ctrl_data->iface_metadata.type,
          &frame_ctrl_data->iface_metadata.frame_drop,
          sizeof(frame_ctrl_data->iface_metadata.frame_drop));
      } else if (frame_ctrl_data->iface_metadata.type == MCT_BUS_MSG_IFACE_METADATA) {
        rc |= iface_util_post_bus_msg(session, MCT_BUS_MSG_IFACE_METADATA,
          &frame_ctrl_data->iface_metadata,
          sizeof(frame_ctrl_data->iface_metadata));
      }
    }
    if ((frame_ctrl_data != NULL) && frame_ctrl_data->ctrl_param.type != CAM_INTF_META_STREAM_ID) {
       if (frame_ctrl_data->ctrl_param.parm_data) {
          free(frame_ctrl_data->ctrl_param.parm_data);
          frame_ctrl_data->ctrl_param.parm_data = NULL;
       }
       free(frame_ctrl_data);
       frame_ctrl_data = NULL;
    }
  }

  return rc;
}

/** iface_set_super_param:
*
*  @iface: handle to iface
*  @iface_sink_port: sink port handle where this event arrived
*  @session_id: current session id
*  @stream_id: current stream id
*  @event: super parm event handle
*
*  Set diff control paramsreceived from HAL, such as sharpness,
*  cpntrast, specialeffects, rolloff, saturation , mce, sce,
*  recording hint etc.
*
*  Returns 0 on success and negative error on failure
**/
int iface_set_super_param(iface_t *iface, iface_port_t *iface_sink_port,
  iface_session_t *session, uint32_t stream_id, mct_event_t *event)
{
  int                             rc = 0;
  uint32_t                        i = 0;
  uint32_t                        q_idx;
  mct_event_super_control_parm_t *param = NULL;


  if (!iface || !iface_sink_port || !event || !session) {
    CDBG_ERROR("%s:%d %p %p %p %p\n", __func__, __LINE__, iface, iface_sink_port,
      event, session);
    return -EINVAL;
  }

  param =
    (mct_event_super_control_parm_t *)event->u.ctrl_event.control_event_data;

  if ((!param) || ((param->num_of_parm_events) && !(param->parm_events))){
    if (param) {
      CDBG_ERROR("%s:%d data %p num_event %d parm_events %p \n", __func__,
        __LINE__, param, param->num_of_parm_events, param->parm_events);
    } else {
      CDBG_ERROR("%s:%d param is NULL\n", __func__, __LINE__);
    }
    return -EINVAL;
  }

  if (session->active_count == 0 && session->num_bundled_streamon == 0) {
    for (i = 0; i < param->num_of_parm_events; i++) {
      rc = iface_set_hal_param(iface, iface_sink_port,
        session,
        UNPACK_STREAM_ID(event->identity),
        &param->parm_events[i], session->sof_frame_id);
      if (rc < 0) {
        CDBG_ERROR("%s:%d failed set parm type %d", __func__, __LINE__,
          param->parm_events[i].type);
      }
    }
  } else {
    q_idx = ((event->u.ctrl_event.current_frame_id + session->max_apply_delay) %
      IFACE_FRAME_CTRL_SIZE);

    for (i = 0; i < param->num_of_parm_events; i++) {
      if (param->parm_events[i].type == CAM_INTF_PARM_HFR) {
        CDBG_LOW("%s:%d store q_idx %d\n", __func__, __LINE__, q_idx);
      }
      if (param->parm_events[i].type == CAM_INTF_META_STREAM_ID) {
        CDBG_LOW("%s:%d cur_frmid: %d max_apply_delay: %d store q_idx %d\n",
          __func__, __LINE__, event->u.ctrl_event.current_frame_id,
          session->max_apply_delay, q_idx);
      }
      rc = iface_store_hal_param(iface, iface_sink_port, session->session_id, stream_id,
        &param->parm_events[i], session->parm_q[q_idx],
        &session->parm_q_lock[q_idx]);
      if (rc < 0) {
        CDBG_ERROR("%s:%d failed iface_store_hal_param type %d\n", __func__,
          __LINE__, param->parm_events[i].type);
      }
    }
  }
  return 0;
}

/** iface_store_per_frame_metadata:
*
*  @session: session handle
*  @iface_metadata: data handle
*
*  Returns TRUE on success and FALSE on failure
**/
boolean iface_store_per_frame_metadata(iface_session_t *session,
  mct_bus_msg_iface_metadata_t *iface_metadata, uint32_t q_idx)
{
  int32_t                  rc = 0;
  iface_frame_ctrl_data_t *frame_ctrl_data = NULL;

  if (!session || !iface_metadata) {
    CDBG_ERROR("%s:%d failed: %p %p\n", session, iface_metadata);
    return FALSE;
  }

  frame_ctrl_data = calloc(1, sizeof(iface_frame_ctrl_data_t));
  if(!frame_ctrl_data) {
    CDBG_ERROR("%s: param malloc failed\n", __func__);
    return FALSE;
  }

  frame_ctrl_data->type = IFACE_PARAM_Q_REPORT_METADATA;
  frame_ctrl_data->session_id = session->session_id;
  frame_ctrl_data->iface_metadata = *iface_metadata;

  pthread_mutex_lock(&session->parm_q_lock[q_idx]);
  mct_queue_push_tail(session->parm_q[q_idx], (void *)frame_ctrl_data);
  pthread_mutex_unlock(&session->parm_q_lock[q_idx]);

  return TRUE;
}

/** iface_handle_control_sof:
*
*  @iface: handle to iface
*  @iface_sink_port: sink port handle where this event arrived
*  @session_id: current session id
*  @stream_id: current stream id
*  @event: super parm event handle
*
*  Set diff control paramsreceived from HAL, such as sharpness,
*  cpntrast, specialeffects, rolloff, saturation , mce, sce,
*  recording hint etc.
*
*  Returns 0 on success and negative error on failure
**/
int iface_handle_control_sof(
  iface_t         *iface,
  iface_port_t    *iface_sink_port __unused,
  iface_session_t *session,
  uint32_t         stream_id __unused,
  mct_event_t     *event)
{
  uint32_t                    q_idx;
  int                         rc = 0;
  uint32_t            skip_frame_id;
  uint32_t            frame_skip_num;
  mct_bus_msg_isp_sof_t       *sof_event = NULL;

  if (!session) {
    CDBG_ERROR("%s: cannot find session \n", __func__);
    return -1;
  }

  q_idx = ((event->u.ctrl_event.current_frame_id + IFACE_APPLY_DELAY) %
    IFACE_FRAME_CTRL_SIZE);

  if (session->hal_version == CAM_HAL_V3) {
    /*Assign here the offset sent by MCT_SOF*/
    sof_event =
      (mct_bus_msg_isp_sof_t *)event->u.ctrl_event.control_event_data;
    if (!sof_event) {
      CDBG_ERROR("failed: sof_event %p", sof_event);
      return FALSE;
    }
    skip_frame_id = event->u.ctrl_event.current_frame_id;
    frame_skip_num = sof_event->tardy;
    if (frame_skip_num > 0) {
      IFACE_HIGH("skip sent by MCT: %d skip_frame_id: %d \
        q_idx: %d max_apply_delay: %d \n", sof_event->tardy,
        skip_frame_id, q_idx, session->max_apply_delay);
      /* if tardy is set we shift the current request to next sof.
       * during drop_reconfig case we also need to shift previous
       * request to current frame.
       */
      rc = iface_util_adjust_paramq(iface, session, stream_id,
        skip_frame_id - session->drop_reconfig, frame_skip_num);
      if (rc < 0) {
        CDBG_ERROR("%s:%d failed: iface_util_handle_frame_skip_event\n",
          __func__, __LINE__);
        return -1;
      }
       session->drop_reconfig = 0;
	   session->dual_cam_drop = 0;
    }
  }

  rc = iface_restore_hal_param(iface, session, session->parm_q[q_idx],
    &session->parm_q_lock[q_idx], event->u.ctrl_event.current_frame_id);
  if (rc) {
    CDBG_ERROR("%s:%d failed iface_restore_hal_param\n", __func__, __LINE__);
  }

  if (session->hvx.enabled == TRUE) {
    iface_hvx_sof(&session->hvx, event->u.ctrl_event.current_frame_id);
  }

  return rc;
}


boolean iface_find_stream_by_sessionid(void *data1, void *data2)
{
  mct_stream_t *stream = (void *)data1;
  uint32_t *input_session_id = (uint32_t *)data2;

  if (!stream) {
    CDBG_ERROR("%s:%d failed stream %p", __func__, __LINE__,stream);
    return FALSE;
  }
  if (!input_session_id) {
    CDBG_ERROR("%s:%d failed input_session_id %p", __func__, __LINE__,
      input_session_id);
    return FALSE;
  }
  if (((stream->streaminfo.identity & 0xFFFF0000) >> 16) == *input_session_id) {
    return TRUE;
  }
  return FALSE;
}

boolean iface_find_stream_by_streamid(void *data1, void *data2)
{
  mct_stream_t *stream = (void *)data1;
  uint32_t *input_stream_id = (uint32_t *)data2;
  if (!stream) {
    CDBG_ERROR("%s:%d failed stream %p", __func__, __LINE__,stream);
    return FALSE;
  }
  if (!input_stream_id) {
    CDBG_ERROR("%s:%d failed input_stream_id %p", __func__, __LINE__,input_stream_id);
    return FALSE;
  }
  if ((stream->streaminfo.identity & 0xFFFF) == *input_stream_id) {
    return TRUE;
  }
  return FALSE;
}

boolean iface_find_online_input_buffer(void *data1, void *data2)
{
  mct_stream_map_buf_t *buf_holder = (mct_stream_map_buf_t *)data1;
  uint32_t *buf_index = (uint32_t *)data2;
  if (!buf_holder || !buf_index) {
    CDBG_ERROR("%s:%d failed buf_holder %p buf_index %p\n", __func__, __LINE__,
      buf_holder, buf_index);
    return FALSE;
  }
  if ((buf_holder->buf_index == *buf_index) &&
      (buf_holder->buf_type == CAM_MAPPING_BUF_TYPE_STREAM_BUF)) {
    return TRUE;
  }
  return FALSE;
}

static boolean iface_find_offline_input_buffer(void *data1, void *data2)
{
  mct_stream_map_buf_t *buf_holder = (mct_stream_map_buf_t *)data1;
  uint32_t *buf_index = (uint32_t *)data2;
  if (!buf_holder || !buf_index) {
    CDBG_ERROR("%s:%d failed buf_holder %p buf_index %p\n", __func__, __LINE__,
      buf_holder, buf_index);
    return FALSE;
  }
  if ((buf_holder->buf_index == *buf_index) &&
      (buf_holder->buf_type == CAM_MAPPING_BUF_TYPE_OFFLINE_INPUT_BUF)) {
    return TRUE;
  }
  return FALSE;
}
/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
void * iface_get_input_buffer(mct_module_t *module,
  mct_stream_info_t *stream_info, cam_stream_parm_buffer_t *parm_buf,
  uint32_t identity, boolean online)
{
  uint32_t session_id = identity >> 16;
  uint32_t stream_id = identity & 0xFFFF;
  mct_pipeline_t *pipeline = NULL;
  mct_stream_t *stream = NULL;
  mct_list_t *stream_list = NULL, *buf_list = NULL;

  /* Validate input params */
  if (!module || !stream_info) {
    CDBG_ERROR("%s:%d failed module %p stream_info %p\n", __func__, __LINE__,
      module, stream_info);
    return NULL;
  }
  /* Get iface module's parent - stream */
  stream_list = mct_list_find_custom(MCT_MODULE_PARENT(module), &session_id,
    iface_find_stream_by_sessionid);
  if (!stream_list) {
    CDBG_ERROR("%s:%d failed stream_list %p\n",
      __func__, __LINE__, stream_list);
    return NULL;
  }
  if (!stream_list->data) {
    CDBG_ERROR("%s:%d failed stream_list->data %p", __func__, __LINE__,
      stream_list->data);
    return FALSE;
  }
  stream = (mct_stream_t *)stream_list->data;
  /* Get stream's parent - pipeline */
  pipeline = MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data);
  if (!pipeline) {
    CDBG_ERROR("%s:%d failed pipeline %p\n", __func__, __LINE__, pipeline);
    return NULL;
  }

  if (online)
    stream_list = mct_list_find_custom(MCT_PIPELINE_CHILDREN(pipeline),
      &stream_info->reprocess_config.online.input_stream_id,
      iface_find_stream_by_streamid);
  else
      stream_list = mct_list_find_custom(MCT_PIPELINE_CHILDREN(pipeline),
        &stream_id, iface_find_stream_by_streamid);

  if (!stream_list) {
    CDBG_ERROR("%s:%d failed stream_list %p", __func__, __LINE__,
      stream_list);
    return FALSE;
  }
  if (!stream_list->data) {
    CDBG_ERROR("%s:%d failed stream_list->data %p", __func__, __LINE__,
      stream_list->data);
    return FALSE;
  }
  stream = (mct_stream_t *)stream_list->data;

  if (!stream->streaminfo.img_buffer_list) {
    CDBG_ERROR("%s:%d failed img_buffer_list = NULL", __func__, __LINE__);
    return FALSE;
  }

  if (online)
    buf_list = mct_list_find_custom(stream->streaminfo.img_buffer_list,
      &parm_buf->reprocess.buf_index, iface_find_online_input_buffer);
  else
    buf_list = mct_list_find_custom(stream->streaminfo.img_buffer_list,
      &parm_buf->reprocess.buf_index, iface_find_offline_input_buffer);

  if (!buf_list) {
    CDBG_ERROR("%s:%d failed buf_list %p", __func__, __LINE__, buf_list);
    return FALSE;
  }
  CDBG("%s stream_id %x  buf_list %p reprocess_buf idx %d\n",
    __func__, stream->streamid,
    buf_list,  parm_buf->reprocess.buf_index);
  if (!buf_list->data) {
    CDBG_ERROR("%s:%d failed buf_list->data %p", __func__, __LINE__,
      buf_list->data);
    return FALSE;
  }
  return buf_list->data;
}

/** iface_handle_reprocess_online
 *    @module: iface module
 *
 *  Handle iface_handle_reprocess_online
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean iface_handle_reprocess_online(iface_t *iface,
  iface_port_t *iface_port, mct_stream_info_t *stream_info,
  cam_stream_parm_buffer_t *parm_buf, uint32_t identity)
{
  boolean                    ret = TRUE;
  int32_t                    rc = 0;
  mct_stream_map_buf_t      *buf_holder = NULL;
  iface_hw_stream_t         *input_hw_stream = NULL;
  iface_hw_stream_t         *offline_hw_stream = NULL;
  iface_offline_isp_info_t  *offline_info = NULL;
  iface_session_t           *iface_session = NULL;
  iface_stream_t            *user_stream = NULL;

  /* Pick Input buffer from different stream */
  buf_holder = iface_get_input_buffer(iface->module, stream_info,
    parm_buf, identity, 1);
  if (!buf_holder) {
    CDBG_ERROR("%s:%d failed: buf_holder NULL\n", __func__, __LINE__);
    ret = FALSE;
    goto ERROR;
  }
  CDBG_HIGH("%s:%d input buf fd %d type %d size %d dim %d x %d\n",
    __func__, __LINE__,
    buf_holder->buf_planes[0].fd, buf_holder->buf_type,
    buf_holder->buf_size,
    buf_holder->buf_planes[0].stride, buf_holder->buf_planes[0].scanline);

  iface_session = iface_util_get_session_by_id(iface,
    UNPACK_SESSION_ID(stream_info->identity));
  if (iface_session == NULL) {
    CDBG_ERROR("%s: can not find session, id = %d\n",
      __func__, UNPACK_SESSION_ID(stream_info->identity));
    return -1;
  }

  offline_info = &iface_session->offline_info;

  /* Get hw stream info for input raw stream*/
  input_hw_stream = iface_util_find_hw_stream_by_mct_id(iface,
    iface_session,
    stream_info->reprocess_config.online.input_stream_id);
  if (input_hw_stream == NULL) {
    CDBG_ERROR("%s: can not in hw stream= %d\n",
      __func__, stream_info->reprocess_config.online.input_stream_id);
    return -1;
  }

  user_stream = iface_util_find_stream_in_sink_port_list(iface,
    UNPACK_SESSION_ID(stream_info->identity),
    stream_info->reprocess_config.online.input_stream_id);
  if (!user_stream) {
    CDBG_ERROR("%s: can not input stream id = %x\n",
      __func__, stream_info->reprocess_config.online.input_stream_id);
    return -1;
  }



  offline_info->input_dim = input_hw_stream->stream_info.dim;
  offline_info->input_fmt= input_hw_stream->stream_info.fmt;
  offline_info->input_stream_id =
    stream_info->reprocess_config.online.input_stream_id;
  offline_info->stream_param_buf= parm_buf;
  offline_info->input_stream_info = user_stream->stream_info;
  offline_info->offline_mode = FALSE;
  offline_info->fd = buf_holder->buf_planes[0].fd;
  offline_info->offline_frame_id = parm_buf->reprocess.frame_idx;
#ifdef VIDIOC_MSM_ISP_MAP_BUF_START_MULTI_PASS_FE
  offline_info->offline_pass = OFFLINE_FIRST_PASS;
#endif
  offline_info->output_buf_idx = 0;
  offline_info->input_buf_offset = 0;


  /* configures offline stream using input stream format*/

  rc = iface_offline_config_to_thread(iface, iface_port,
    UNPACK_STREAM_ID(stream_info->identity),
    UNPACK_SESSION_ID(stream_info->identity),
    offline_info);
  if (rc < 0) {
    CDBG_ERROR("%s: iface_offline_config_to_thread = %d\n", __func__, rc);
    return FALSE;
  }

ERROR:
   return ret;
}

/** iface_handle_reprocess_offline
 *    @module: iface module
 *
 *  Handle iface_module_handle_reprocess_offline
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean iface_handle_reprocess_offline(iface_t *iface,
  iface_port_t *iface_port, mct_stream_info_t *stream_info,
  cam_stream_parm_buffer_t *parm_buf, uint32_t identity)
{
  boolean                    ret = TRUE;
  int32_t                    rc = 0;
  mct_stream_map_buf_t      *buf_holder = NULL;
  iface_hw_stream_t         *offline_hw_stream = NULL;
  iface_offline_isp_info_t  *offline_info = NULL;
  iface_session_t           *iface_session = NULL;
  iface_stream_t            *user_stream = NULL;
  uint32_t                   stream_id = UNPACK_STREAM_ID(identity);

  /* Pick Input buffer from different stream */
  buf_holder = iface_get_input_buffer(iface->module, stream_info,
    parm_buf, identity, 0);
  if (!buf_holder) {
    CDBG_ERROR("%s:%d failed: buf_holder NULL\n", __func__, __LINE__);
    ret = FALSE;
    goto ERROR;
  }
  CDBG("%s:%d input buf fd %d type %d size %d dim %d x %d fmt %d\n",
    __func__, __LINE__,
    buf_holder->buf_planes[0].fd, buf_holder->buf_type, buf_holder->buf_size,
    stream_info->reprocess_config.offline.input_dim.width,
    stream_info->reprocess_config.offline.input_dim.height,
    stream_info->reprocess_config.offline.input_fmt);

  iface_session = iface_util_get_session_by_id(iface,
    UNPACK_SESSION_ID(stream_info->identity));
  if (iface_session == NULL) {
    CDBG_ERROR("%s: can not find session, id = %d\n",
      __func__, UNPACK_SESSION_ID(stream_info->identity));
    ret = FALSE;
    goto ERROR;
  }

  offline_info = &iface_session->offline_info;

  user_stream = iface_util_find_stream_in_sink_port_list(iface,
    UNPACK_SESSION_ID(stream_info->identity),
    stream_id);
  if (!user_stream) {
    CDBG_ERROR("%s: can not find input stream, id = %x\n",
      __func__, stream_id);
    ret = FALSE;
    goto ERROR;
  }



  offline_info->input_dim = stream_info->reprocess_config.offline.input_dim;
  offline_info->input_fmt = stream_info->reprocess_config.offline.input_fmt;
  offline_info->input_stream_id = stream_id;
  offline_info->stream_param_buf= parm_buf;
  offline_info->input_stream_info = user_stream->stream_info;
  offline_info->offline_mode = TRUE;
  offline_info->fd = buf_holder->buf_planes[0].fd;
  offline_info->offline_frame_id = parm_buf->reprocess.frame_idx;
#ifdef VIDIOC_MSM_ISP_MAP_BUF_START_MULTI_PASS_FE
  offline_info->offline_pass = OFFLINE_FIRST_PASS;
#endif
  offline_info->output_buf_idx = 0;
  offline_info->input_buf_offset = 0;


  /* configures offline stream using input stream format*/
  rc = iface_offline_config_to_thread(iface, iface_port,
    UNPACK_STREAM_ID(stream_info->identity),
    UNPACK_SESSION_ID(stream_info->identity),
    offline_info);
  if (rc < 0) {
    CDBG_ERROR("%s: iface_offline_stream_config = %d\n", __func__, rc);
    ret = FALSE;
    goto ERROR;
  }

ERROR:
   return ret;
}

/** iface_offline_stream_param_buf
 *    @module: iface module
 *    @event: mct event to be handled
 *    @data: stream info
 *
 *  Handle stream param buf event for offline stream. Use input
 *  buffer index sent as part of this call for performing post
 *  processing. Use crop information, chromatix pointers and AEC
 *  trigger from stream info and meta data respectively.
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
int iface_offline_stream_param_buf(iface_t *iface,
  iface_port_t *iface_port, iface_session_t *session,
  int32_t stream_id, mct_event_t *event)
{
  boolean                     ret = TRUE;
  int32_t                     rc = 0;
  uint32_t                    i = 0;
  mct_port_t                 *port = NULL;
  mct_stream_info_t          *stream_info = NULL;
  cam_stream_parm_buffer_t   *parm_buf = NULL;
  iface_stream_t             *user_stream = NULL;

  /* Validate input pameters */
  if (!event || !iface_port) {
    CDBG_ERROR("%s:%d failed: data %p %p\n", __func__, __LINE__,
      event, iface_port);
    rc = FALSE;
    goto ERROR;
  }

  parm_buf = (cam_stream_parm_buffer_t *)
    event->u.ctrl_event.control_event_data;


  user_stream = iface_util_find_stream_in_sink_port(iface_port,
    session->session_id, stream_id);
  if (user_stream == NULL) {
    CDBG_ERROR("%s: stream not exist in this sinkport. error -1\n", __func__);
    return -1;
  }

  stream_info = &user_stream->stream_info;
  CDBG("%s: param type %d stream_id %x type %d", __func__,
    parm_buf->type, stream_info->identity, stream_info->stream_type);

  if (CAM_STREAM_PARAM_TYPE_DO_REPROCESS == parm_buf->type &&
    stream_info->reprocess_config.pp_feature_config.feature_mask &
    CAM_QCOM_FEATURE_RAW_PROCESSING) {
    /* Find the type of reprocess, offline / online */
    if (stream_info->reprocess_config.pp_type == CAM_ONLINE_REPROCESS_TYPE) {
      ret = iface_handle_reprocess_online(iface, iface_port, stream_info,
        parm_buf, event->identity);
      if (ret == FALSE) {
        CDBG_ERROR("%s:%d failed: handle reprocess online\n", __func__,
          __LINE__);
        rc = -1;
        goto ERROR;
      }
    } else if (stream_info->reprocess_config.pp_type ==
      CAM_OFFLINE_REPROCESS_TYPE) {
      ret = iface_handle_reprocess_offline(iface, iface_port, stream_info,
        parm_buf, event->identity);
      if (ret== FALSE) {
        CDBG_ERROR("%s:%d failed: handle reprocess offline\n", __func__,
          __LINE__);
        rc = -1;
        goto ERROR;
      }
    }
  }

ERROR:
  CDBG("%s:%d rc %d X\n", __func__,
    __LINE__, rc);
  return rc;
}


/** iface_set_hal_stream_param:
 *    @ispif: ispif pointer
 *    @ispif_sink_port: ispif sink port
 *    @stream_idi: stream id
 *    @session_id: session id
 *    @event: MCTL control event
 *
 *  This function runs in MCTL thread context.
 *
 *  This function sets hal stream parameter to ispif module
 *
 *  Return:  0 - No error
 *          -1 - Cannot find session or stream
 **/
int iface_set_hal_stream_param(iface_t *iface, iface_port_t *iface_sink_port,
  iface_session_t *session, uint32_t stream_id, mct_event_t *event)
{
  int rc = 0;
  cam_stream_parm_buffer_t *param =
    event->u.ctrl_event.control_event_data;

  switch (param->type) {
  case CAM_STREAM_PARAM_TYPE_SET_BUNDLE_INFO:
    rc = iface_util_set_bundle(iface, iface_sink_port,
      session, stream_id, &param->bundleInfo);
    break;

  case CAM_STREAM_PARAM_TYPE_DO_REPROCESS:
    rc = iface_offline_stream_param_buf(iface, iface_sink_port,
      session, stream_id, event);
    break;
  case CAM_INTF_PARM_FRAMESKIP:
    rc = iface_util_set_frame_skip(iface, session->session_id,
      stream_id, &param->skipPattern);
    break;

  default:
    break;

  } /* switch (param->type) */

  return rc;
}

/** iface_set_sensor_output
 *
 *  @iface: iface handle
 *  @event: event handle
 *
 *  If HVX is supported, pass sensor output information to OEM
 *  stub and modify sensor output based on HVX output
 *
 *  Return 0 on success and negative error on failure
 **/
int32_t iface_set_sensor_output(iface_t *iface, mct_event_t *event)
{
  int32_t           rc = 0;
  sensor_set_dim_t *set_dim = NULL;
  iface_session_t  *iface_session = NULL;

  if (!iface || !event) {
    CDBG_ERROR("%s:%d failed: iface %p event %p\n", __func__, __LINE__, iface,
      event);
    return -1;
  }

  set_dim = (sensor_set_dim_t *)event->u.module_event.module_event_data;
  if (!set_dim) {
    CDBG_ERROR("%s:%d failed: set_dim %p\n", __func__, __LINE__, set_dim);
    return -1;
  }

  iface_session = iface_util_get_session_by_id(iface,
    UNPACK_SESSION_ID(event->identity));
  if (!iface_session) {
    CDBG_ERROR("%s:%d failed: iface_session %p\n", __func__, __LINE__,
      iface_session);
    return -1;
  }

  iface_session->hvx.set_dim = *set_dim;

  return rc;
}

/** iface_destroy_hw
 *
 * TODO
 *
 * Return: nothing
 **/
void iface_destroy_hw(iface_t *iface, int hw_idx)
{
  CDBG("%s: E, hw_idx %d", __func__, hw_idx);
  struct msm_vfe_smmu_attach_cmd cmd;
  int fd, rc = 0;

  if (!iface || (hw_idx >= VFE_MAX)) {
    IFACE_ERR("%s:%d failed iface %p hw_idx %d\n", __func__, __LINE__, iface,
      hw_idx);
    return;
  }

  pthread_mutex_lock(&iface->isp_axi_data.axi_data[hw_idx].mutex);

  /* caller will ensure only call this function once per session */
  if (iface->isp_axi_data.axi_data[hw_idx].ref_cnt > 0 &&
      --iface->isp_axi_data.axi_data[hw_idx].ref_cnt > 0) {
      pthread_mutex_unlock(&iface->isp_axi_data.axi_data[hw_idx].mutex);
      return;
  }

  assert(iface->isp_axi_data.axi_data[hw_idx].axi_hw_ops);
  assert(iface->isp_axi_data.axi_data[hw_idx].axi_hw_ops->ctrl);

  fd = iface->isp_axi_data.axi_data[hw_idx].fd;
  cmd.iommu_attach_mode = IOMMU_DETACH;
  rc = ioctl(fd, VIDIOC_MSM_ISP_SMMU_ATTACH, &cmd);
  if (rc < 0) {
    IFACE_ERR("%s: isp smmu detach error = %d\n", __func__, rc);
  }

  IFACE_HIGH("%s: Destroy axi hw on VFE :%d\n", __func__, hw_idx);

  iface->isp_axi_data.axi_data[hw_idx].axi_hw_ops->destroy(
      iface->isp_axi_data.axi_data[hw_idx].axi_hw_ops->ctrl);

  iface->isp_axi_data.axi_data[hw_idx].axi_hw_ops = NULL;

  pthread_mutex_unlock(&iface->isp_axi_data.axi_data[hw_idx].mutex);
}

/** iface_create_axi
 *
 *  This function assumes that it will not be called twice for same session
 *
 * Return: nothing
 **/
int iface_create_axi(iface_t *iface,
                     iface_session_t *session,
                     int hw_idx)
{
  int rc = 0;
  iface_axi_init_params_t init_params;
  iface_axi_hw_t *axi_hw = NULL;
  struct msm_vfe_smmu_attach_cmd cmd;

  if (!iface || !session || hw_idx >= VFE_MAX) {
    CDBG_ERROR("%s: Error! Invalid params iface %p session %p hw_idx %d\n",
      __func__, iface, session, hw_idx);
    return -1;
  }

  pthread_mutex_lock(&iface->isp_axi_data.axi_data[hw_idx].mutex);

  CDBG_HIGH("%s: E, hw_idx %d, num_pix_streams %d, num_rdi_stream %d\n",
            __func__, hw_idx, session->session_resource.camif_cnt,
            session->session_resource.rdi_cnt);

  /* caller will ensure this function called only once per session */
  if (iface->isp_axi_data.axi_data[hw_idx].ref_cnt == 0) {
    IFACE_HIGH("%s:%d: Create axi hw for session %d on VFE %d\n", __func__, __LINE__,
              session->session_id, hw_idx);

    /* create axi hw and open subdev node */
    iface->isp_axi_data.axi_data[hw_idx].axi_hw_ops =
      iface_axi_create_hw(iface->isp_axi_data.isp_subdev[hw_idx].subdev_name);

    if (iface->isp_axi_data.axi_data[hw_idx].axi_hw_ops == NULL) {
      IFACE_ERR("%s: cannot create hw, dev_name = '%s'\n",
                 __func__,  iface->isp_axi_data.isp_subdev[hw_idx].subdev_name);
      rc = -1;
      goto end;
    }

    axi_hw =
      (iface_axi_hw_t *)iface->isp_axi_data.axi_data[hw_idx].axi_hw_ops->ctrl;

    iface->isp_axi_data.axi_data[hw_idx].fd = axi_hw->fd;
    cmd.security_mode = session->security_mode;
    cmd.iommu_attach_mode = IOMMU_ATTACH;
    CDBG("%s: security mode : %d\n", __func__, cmd.security_mode);
    rc = ioctl(axi_hw->fd, VIDIOC_MSM_ISP_SMMU_ATTACH, &cmd);
    if (rc < 0) {
      IFACE_ERR("%s: isp smmu attach error = %d\n", __func__, rc);
      rc = -1;
      goto release_axi_hw;
    }
  }
  if (iface->isp_axi_data.axi_data[hw_idx].axi_hw_ops == NULL) {
    IFACE_ERR("Invalid axi hw osp with ref count %d\n",
                iface->isp_axi_data.axi_data[hw_idx].ref_cnt);
    rc = -1;
    goto release_axi_hw;
  }

  /* after create axi hw, init axi hw by hw ops just created
     1. set init param
     2. init notify ops*/
  init_params.cap = iface->isp_axi_data.isp_subdev[hw_idx].hw_cap;
  init_params.isp_version = iface->isp_axi_data.isp_subdev[hw_idx].isp_version;
  init_params.dev_idx = hw_idx;
  init_params.session_id = session->session_id;
  init_params.session_resource = &session->session_resource;
  init_params.buf_mgr = &iface->buf_mgr;

  iface->isp_axi_data.axi_data[hw_idx].notify_ops.handle = hw_idx;
  iface->isp_axi_data.axi_data[hw_idx].notify_ops.parent = iface;
  iface->isp_axi_data.axi_data[hw_idx].notify_ops.notify = iface_util_axi_notify;

  rc = iface->isp_axi_data.axi_data[hw_idx].axi_hw_ops->init(
    iface->isp_axi_data.axi_data[hw_idx].axi_hw_ops->ctrl, (void *)&init_params,
    &iface->isp_axi_data.axi_data[hw_idx].notify_ops);
  if (rc < 0) {
    IFACE_ERR("%s: error in axi hw init, rc = %d\n", __func__, rc);
    rc = -1;
    goto detach_smmu_in_kernel;
  }

  iface->isp_axi_data.axi_data[hw_idx].ref_cnt++;

  pthread_mutex_unlock(&iface->isp_axi_data.axi_data[hw_idx].mutex);
  return rc;

detach_smmu_in_kernel:
  cmd.iommu_attach_mode = IOMMU_DETACH;
  if(axi_hw)
    ioctl(axi_hw->fd, VIDIOC_MSM_ISP_SMMU_ATTACH, &cmd);
release_axi_hw:
  assert(iface->isp_axi_data.axi_data[hw_idx].axi_hw_ops);
  if (iface->isp_axi_data.axi_data[hw_idx].ref_cnt == 0) {
    iface->isp_axi_data.axi_data[hw_idx].axi_hw_ops->destroy(
        iface->isp_axi_data.axi_data[hw_idx].axi_hw_ops->ctrl);
    iface->isp_axi_data.axi_data[hw_idx].axi_hw_ops = NULL;
  }
end:
  pthread_mutex_unlock(&iface->isp_axi_data.axi_data[hw_idx].mutex);
  return rc;
}

/** iface_init:
 *    @ispif: ispif pointer
 *
 *  This function runs in MCTL thread context.
 *
 *  This function initializes ispif module.
 *
 *  Return:  0 - Success
 *          -1 - error during discovering subdevices
 **/
int iface_init(iface_t *iface)
{
  int rc = 0;
  int i = 0;
  uint32_t sd_num;
  CDBG("%s: E\n", __func__);

  pthread_mutex_init(&iface->mutex, NULL);

  for (i = 0; i < VFE_MAX; i++)
    pthread_mutex_init(&iface->isp_axi_data.axi_data[i].mutex, NULL);

  pthread_mutex_init(&iface->ispif_data.ispif_hw.mutex, NULL);

  /*save subdev name in iface*/
  rc = iface_discover_subdev_nodes(iface);

  /*init iface bug mgr*/
  iface_init_buf_mgr(&iface->buf_mgr);
  pthread_mutex_init(&iface->hvx_singleton.mutex, NULL);

  return rc;
}

/** iface_deinit:
 *    @iface: iface pointer
 *
 *  This function runs in MCTL thread context.
 *
 *  Return:  0 - Success
 *          -1 - error during discovering subdevices
 **/
void iface_deinit(iface_t *iface)
{
  int i = 0;

  pthread_mutex_destroy(&iface->mutex);
  pthread_mutex_destroy(&iface->hvx_singleton.mutex);

  for (i = 0; i < VFE_MAX; i++)
    pthread_mutex_destroy(&iface->isp_axi_data.axi_data[i].mutex);

  pthread_mutex_destroy(&iface->ispif_data.ispif_hw.mutex);

  /*init iface bug mgr*/
  iface_deinit_buf_mgr(&iface->buf_mgr);

  return;
}
