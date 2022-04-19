/*============================================================================
Copyright (c) 2013-2017,2019-2020 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/videodev2.h>

#include "cam_intf.h"
#include "mct_controller.h"
#include "modules.h"
#include "camera_dbg.h"
#include "iface_ops.h"
#include "iface_axi.h"
#include "iface_axi_util.h"
#include "iface.h"
#include "sensor_lib.h"
#include "iface_util.h"

#include "mct_profiler.h"
#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

static int iface_axi_stream_set_skip_pattern(
  iface_axi_hw_t                   *axi,
  iface_param_frame_skip_pattern_t *skip_pattern,
  uint32_t                          in_params_size __unused);

/** iface_dump_stats_stream_cfg:
 *
 *    @stats_request_cmd: request to kernel
 *
 *    this funciton dump the stats stream info to kernel
 *    return void
 *
 **/
void iface_dump_stats_stream_cfg(
  struct msm_vfe_stats_stream_request_cmd stats_request_cmd)
{
  CDBG("%s: ===== AXI DUMP (STATS)=====\n", __func__);
  CDBG("%s: session_id = %d, stream id = %x\n", __func__,
    stats_request_cmd.session_id, stats_request_cmd.stream_id);
  CDBG("%s: stats_type = %d, comp_flag = %d\n",
    __func__, stats_request_cmd.stats_type, stats_request_cmd.composite_flag);
  CDBG("%s: framedrop_pattern = %d, irq_subsample_pattern = %x\n", __func__,
    stats_request_cmd.framedrop_pattern, stats_request_cmd.irq_subsample_pattern);
  CDBG("%s: buffer_offset = %d\n", __func__, stats_request_cmd.buffer_offset);
}

/** iface_axi_start_fetch:
 *
 *    @axi: axi hw
 *    @action_data: triger_fetch_stream_t
 *    @action_data_size: size
 *  This funciton passes buffer information for fetch engine to kernel
 *  After reading of input buffer is completed "ISP_EVENT_FE_READ_DONE"
 * subdev event is generated and handled to release/enqueue buffer.
 *    return 0 on success
 *
 **/
static int iface_axi_start_fetch(iface_axi_hw_t *axi,
  triger_fetch_stream_t *action_data, uint32_t action_data_size)
{
  int rc = 0;
#ifdef VIDIOC_MSM_ISP_MAP_BUF_START_MULTI_PASS_FE
  struct msm_vfe_fetch_eng_multi_pass_start fe_cfg;
  iface_axi_stream_t *stream = NULL;
#else
  struct msm_vfe_fetch_eng_start fe_cfg;
#endif
  if (sizeof(triger_fetch_stream_t) != action_data_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -100;
  }

  /*
        Fetch engine should be already configured before this.by
        'iface_util_fe_cfg'. This function passes only buffer information
        to kernel and starts fetch engine by reload & go cmd
     */
  fe_cfg.session_id = action_data->session_id;
  fe_cfg.stream_id = action_data->hw_stream_id;
  fe_cfg.buf_idx = action_data->buf_idx;
  fe_cfg.offline_mode = action_data->offline_mode;
  fe_cfg.fd = action_data->fd;
  fe_cfg.frame_id = action_data->frame_id;
#ifdef VIDIOC_MSM_ISP_MAP_BUF_START_MULTI_PASS_FE
  fe_cfg.offline_pass = action_data->offline_pass;
  fe_cfg.output_buf_idx = action_data->output_buf_idx;
  fe_cfg.input_buf_offset = action_data->input_buf_offset;

  stream = iface_axi_util_find_stream(axi, action_data->session_id,
    action_data->output_stream_id);
  if (stream != NULL) {
      fe_cfg.output_stream_id = stream->axi_stream_handle;
  }
  rc = ioctl(axi->fd, VIDIOC_MSM_ISP_MAP_BUF_START_MULTI_PASS_FE, &fe_cfg);
  if (rc < 0) {
    CDBG_ERROR("%s: iface_axi_start_fetch error = %d errno %d\n",
      __func__, rc, errno);
  }
#else
  rc = ioctl(axi->fd, VIDIOC_MSM_ISP_MAP_BUF_START_FE, &fe_cfg);
  if (rc < 0) {
    CDBG_ERROR("%s: iface_axi_start_fetch error = %d errno %d\n",
      __func__, rc, errno);
  }
#endif
  return rc;
}


/** iface_axi_request_stream:
 *
 *    @axi:
 *    @params:
 *
 **/
static int iface_axi_request_stream(iface_axi_hw_t *axi_hw, iface_axi_stream_t *axi_stream)
{
  int rc = 0;
  int i;
  struct msm_vfe_axi_stream_request_cmd *request_cfg;

  request_cfg = &axi_hw->work_struct.u.stream_request_cmd;

  memset(&axi_hw->work_struct, 0, sizeof(axi_hw->work_struct));

  request_cfg->session_id = axi_stream->hw_stream_info.session_id;
  request_cfg->stream_id = axi_stream->hw_stream_info.hw_stream_id;

  request_cfg->output_format =
    iface_axi_util_cam_fmt_to_v4l2_fmt(axi_stream->hw_stream_info.stream_info.fmt,
      axi_stream->hw_stream_info.need_uv_subsample);

  if (request_cfg->output_format == 0) {
    /* invalid format */
    CDBG_ERROR("%s: invalid cam_format %d, session_id = %d, hw_stream_id = %d, rc = -100\n",
      __func__, axi_stream->hw_stream_info.stream_info.fmt,
      axi_stream->hw_stream_info.session_id, axi_stream->hw_stream_info.hw_stream_id);
    return -100;
  }

  request_cfg->stream_src = axi_stream->hw_stream_info.axi_path; /*CAMIF/IDEAL/RDIs*/
  request_cfg->hfr_mode = 0; /* TODO: need fix */
  request_cfg->frame_base = axi_stream->hw_stream_info.frame_base;
  request_cfg->burst_count = axi_stream->hw_stream_info.stream_info.num_burst;
  request_cfg->init_frame_drop = axi_stream->hw_stream_info.sensor_skip_cnt;
  request_cfg->axi_stream_handle = 0; /*Return values*/
  request_cfg->buf_divert = axi_stream->hw_stream_info.need_divert;
  request_cfg->controllable_output =
    axi_stream->hw_stream_info.controllable_output;
  request_cfg->vt_enable = axi_stream->hw_stream_info.vt_enable;

  request_cfg->frame_skip_pattern = axi_stream->hw_stream_info.frame_skip_pattern;
  request_cfg->rdi_input_type = MSM_CAMERA_RDI_MIN;

  if (axi_stream->hw_stream_info.ext_stats_type == PD_STATS)
    request_cfg->rdi_input_type = MSM_CAMERA_RDI_PDAF;

  rc = iface_axi_util_fill_plane_info(axi_hw, request_cfg->plane_cfg, axi_stream);
  if (rc != 0)
    CDBG_ERROR("iface_axi_util_fill_plane_info failed !!!\n");

  iface_dump_axi_plane_config(axi_stream, request_cfg,
    axi_stream->hw_stream_info.stream_info.fmt);

  rc = ioctl(axi_hw->fd, VIDIOC_MSM_ISP_REQUEST_STREAM, request_cfg);
  if (rc < 0) {
    CDBG_ERROR("%s: ISP_REQUEST_STREAM error= %d, sess_id = %x, hw stream id = %x\n",
      __func__, rc, axi_stream->hw_stream_info.session_id,
      axi_stream->hw_stream_info.hw_stream_id);
    goto error;
  }

  /* save the handle */
  axi_stream->axi_stream_handle = request_cfg->axi_stream_handle;
  CDBG("%s: axi_stream: hw_stream_id = %d, axi_stream_handle = %x\n", __func__,
    axi_stream->hw_stream_info.hw_stream_id, axi_stream->axi_stream_handle);

  return 0;

error:
  /* TODO: need to remove already configured streams */
  return rc;
}

/** iface_axi_stream_config:
 *
 *    @axi:
 *    @in_params:
 *    @in_params_size:
 *
 **/


static int iface_axi_stream_config(
  iface_axi_hw_t               *axi_hw,
  iface_hw_stream_set_params_t *hw_streams,
  uint32_t                      in_params_size __unused)
{
  uint32_t i, j = 0;
  int rc = 0;
  iface_axi_stream_t *axi_stream = NULL;
  uint8_t stream_idx = 0;

  if (!hw_streams) {
    CDBG_ERROR("Hw streams %p",hw_streams);
    return -1;
  }
  CDBG("%s num Streams = %d",__func__,hw_streams->num_hw_streams );
  for (j = 0; j < hw_streams->num_hw_streams; j++) {
    for (i = 0; i < IFACE_AXI_STREAM_MAX; i++) {
      /* for first time config we reserve the stream */
      if (axi_hw->stream[i].state == IFACE_AXI_STREAM_STATE_INVALID) {
        axi_stream= &axi_hw->stream[i];
        break;
      }
    }

    if (axi_stream == NULL) {
      CDBG_ERROR("%s: no more empty stream slots\n", __func__);
      return -1;
    }
    axi_stream->hw_stream_info = hw_streams->hw_streams[j];
    axi_stream->state = IFACE_AXI_STREAM_STATE_CFG;

    /* have not request the kernel axi stream yet. request new kernel axi stream */
    if (!axi_stream->axi_stream_handle) {
      rc = iface_axi_request_stream(axi_hw, axi_stream);
      if (rc < 0) {
        CDBG_ERROR("%s: iface REQUEST AXI STREAM error = %d\n", __func__, rc);
        break;
      }
      stream_idx = axi_stream->axi_stream_handle & 0xFF;
      if (stream_idx < VFE_AXI_SRC_MAX) {
        axi_hw->axi_handle_to_stream_idx[stream_idx] = i;
      }
    }
  }

  return rc;
}


/** iface_stats_reg_buf:
 *
 *    @axi_hw: axi handle
 *    @axi_stats_stream: stats stream info
 *
 *    return integer
 **/
static uint32_t iface_stats_reg_buf(
  iface_axi_hw_t           *axi_hw,
  iface_axi_stats_stream_t *axi_stats_stream)
{
  int rc = 0;
  iface_buf_request_t buf_request;

  memset(&buf_request, 0, sizeof(buf_request));

  if (axi_hw->buf_mgr == NULL) {
    CDBG_ERROR("%s: no buf_mgr ptr, rc = -1\n", __func__);
    return -1;
  }

  if (axi_stats_stream->stats_stream_info.num_bufs > IFACE_STATS_MAX_BUFS) {
    CDBG_ERROR("%s: num_buf %d > max number %d, rc = -1\n", __func__,
      axi_stats_stream->stats_stream_info.num_bufs, IFACE_STATS_MAX_BUFS);
    return -1;
  }

  buf_request.session_id = axi_stats_stream->stats_stream_info.session_id;
  buf_request.stream_id = axi_stats_stream->stats_stream_info.stats_stream_id;
  buf_request.use_native_buf =
    axi_stats_stream->stats_stream_info.use_native_buf;

  /*For deferred buffer allocation, HAL buffers are updated after streamon
  For non HAL buffers current and total number of buffers is same*/
  buf_request.total_num_buf = axi_stats_stream->stats_stream_info.num_bufs;
  buf_request.current_num_buf = axi_stats_stream->stats_stream_info.num_bufs;
  buf_request.buf_info.num_planes = 1;
  buf_request.buf_info.mp[0].len = axi_stats_stream->stats_stream_info.buf_len;
  buf_request.buf_info.mp[0].offset = 0;
  buf_request.buf_info.frame_len = axi_stats_stream->stats_stream_info.buf_len;
  buf_request.img_buf_list = NULL;
  buf_request.cached = 1;
  buf_request.buf_type = axi_stats_stream->stats_stream_info.buf_type;
  buf_request.security_mode = NON_SECURE_MODE;

  pthread_mutex_lock(&axi_hw->buf_mgr->req_mutex);

  /*find if we already request, if dual vfe,
    we request only once, share the same bufq handle*/
  axi_stats_stream->bufq_handle = iface_find_matched_bufq_handle(
    axi_hw->buf_mgr, axi_stats_stream->stats_stream_info.session_id,
    axi_stats_stream->stats_stream_info.stats_stream_id);

  /*if can not find it then means not regustered yet*/
  if (axi_stats_stream->bufq_handle == 0) {
    rc = iface_request_buf(axi_hw->buf_mgr, &buf_request);
    if (rc < 0) {
      pthread_mutex_unlock(&axi_hw->buf_mgr->req_mutex);
      return -1;
    }
   axi_stats_stream->bufq_handle = buf_request.buf_handle;
  }
  pthread_mutex_unlock(&axi_hw->buf_mgr->req_mutex);
  mct_list_free_all(buf_request.img_buf_list, iface_free_buf);

  rc = iface_register_buf(axi_hw->buf_mgr,
    axi_stats_stream->bufq_handle, axi_hw->fd);
  if (rc < 0) {
    CDBG_ERROR("%s: isp_register_buf error = %d, rc = -1\n", __func__, rc);
    iface_release_buf(axi_hw->buf_mgr, axi_stats_stream->bufq_handle, NULL);
    axi_stats_stream->bufq_handle= 0;
    return -1;
  }

  return rc;
}

/** iface_axi_stats_stream_config:
 *
 *    @axi_hw: axi handle
 *    @stats stream: stats hw stream info
 *    @in_params_siz: size of stats stream inifo
 *
 *    return integer rc
 **/
static int iface_axi_stats_stream_config(
  iface_axi_hw_t                  *axi_hw,
  iface_stats_stream_set_params_t *stats_streams,
  uint32_t                         in_params_size __unused)
{
  uint32_t i = 0;
  int32_t rc = 0;
  uint32_t num =0;
  struct msm_vfe_stats_stream_request_cmd req_cmd;
  iface_axi_stats_stream_t *axi_stats_stream = NULL;
  iface_stats_stream_t *stats_stream = NULL;

  if (!axi_hw || !stats_streams) {
    CDBG_ERROR("%s: NULL pointer, axi_hw = %p, stats stream = %p\n", __func__, axi_hw, stats_stream);
    return 0;
  }
  CDBG("%s:No of stats Stream %d\n", __func__,stats_streams->num_stats_streams);
  for(num = 0; num < stats_streams->num_stats_streams; num++) {
    stats_stream = &stats_streams->stats_stream[num];

    if(stats_stream->stats_type >= MSM_ISP_STATS_MAX) {
      CDBG_ERROR("%s: invalid stats type = %d\n", __func__,
        stats_stream->stats_type);
      return 0;
    }

    axi_stats_stream = &axi_hw->stats_stream[stats_stream->stats_type];

    /*deep copy the stats stream info*/
    axi_stats_stream->stats_stream_info = *stats_stream;

    memset(&req_cmd, 0, sizeof(req_cmd));
    req_cmd.session_id = axi_stats_stream->stats_stream_info.session_id;
    req_cmd.stream_id = axi_stats_stream->stats_stream_info.stats_stream_id;
    req_cmd.stats_type = axi_stats_stream->stats_stream_info.stats_type;
    req_cmd.composite_flag = axi_stats_stream->stats_stream_info.comp_flag;
    req_cmd.framedrop_pattern = axi_stats_stream->stats_stream_info.pattern;
    req_cmd.init_frame_drop = axi_stats_stream->stats_stream_info.init_frame_drop;

    /* depends on config VFE0 or VFE1*/
    if (axi_stats_stream->stats_stream_info.buf_type == ISP_SHARE_BUF) {
      if (axi_stats_stream->stats_stream_info.isp_stripe_id == ISP_STRIPE_LEFT)
        req_cmd.buffer_offset = 0;
      else
        req_cmd.buffer_offset = axi_stats_stream->stats_stream_info.buf_len / 2;
    } else {
      req_cmd.buffer_offset = 0;
    }

    iface_dump_stats_stream_cfg(req_cmd);
    rc = ioctl(axi_hw->fd, VIDIOC_MSM_ISP_REQUEST_STATS_STREAM, &req_cmd);
    if (rc < 0) {
      CDBG_ERROR("%s: cannot request stream for stats 0x%x, rc = %d\n",
        __func__, axi_stats_stream->stats_stream_info.stats_type, rc);
      return rc;
    }

    axi_stats_stream->state = IFACE_AXI_STREAM_STATE_CFG;
    /*stream handle store in each axi instance*/
    axi_stats_stream->stream_handle = req_cmd.stream_handle;

    rc = iface_stats_reg_buf(axi_hw, axi_stats_stream);
    if (rc < 0) {
       CDBG("%s: iface request stats buffer failed, rc = %d\n", __func__, rc);
      struct msm_vfe_stats_stream_release_cmd rel_cmd;
      rel_cmd.stream_handle = axi_stats_stream->stream_handle;
      ioctl(axi_hw->fd, VIDIOC_MSM_ISP_RELEASE_STATS_STREAM, &rel_cmd);
    }
  }

  return rc;
}


/** iface_stats_unreg_buf:
 *
 *    @axi_hw: axi handel
 *    @axi_stats stream: stats stream info
 *
 *    return integer rc
 **/
static int iface_stats_unreg_buf(iface_axi_hw_t *axi_hw,
  iface_axi_stats_stream_t *axi_stats_stream)
{
  int rc = 0;

  if (axi_stats_stream->bufq_handle == 0) {
    CDBG_ERROR("%s: error, buf_handle == 0\n", __func__);
    return -1;
  }

  rc = iface_unregister_buf(axi_hw->buf_mgr,
    axi_stats_stream->bufq_handle, axi_hw->fd);
  if (rc < 0) {
    CDBG_ERROR("%s: isp_unregister_buf error, rc %d\n", __func__, rc);
    return rc;
  }

  /*iface_unregister_buf return 0 means last VFE close, 1 means still has one vfe left*/
  if (rc == 0) {
    CDBG("%s: last VFE close, relase stats buffer\n", __func__);
    iface_release_buf(axi_hw->buf_mgr, axi_stats_stream->bufq_handle, NULL);
  }

  axi_stats_stream->bufq_handle = 0;

  return rc;
}

/** iface_axi_stats_stream_unconfig:
 *
 *    @axi_hw: axi handle
 *    @stats stream: stats stream info
 *    @param_size: size of input param
 *
 *    return integer rc
 **/
static int iface_axi_stats_stream_unconfig(iface_axi_hw_t *axi_hw)
{
  int rc = 0;
  int i = 0;
  struct msm_vfe_stats_stream_release_cmd rel_cmd;
  iface_axi_stats_stream_t *axi_stats_stream = NULL;

  if (axi_hw == NULL) {
    CDBG_ERROR("%s: NULL pointer!\n", __func__);
    return -1;
  }

  /*stats always close all*/
  for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
    if (axi_hw->stats_stream[i].stream_handle != 0)
      axi_stats_stream = &axi_hw->stats_stream[i];
    else
      continue;

    memset(&rel_cmd,  0, sizeof(rel_cmd));

    rel_cmd.stream_handle = axi_stats_stream->stream_handle;
    CDBG("%s: axi_hw->fd = %d\n", __func__, axi_hw->fd);

    CDBG("%s: release stats stream id %x, handle %x\n", __func__,
      axi_stats_stream->stats_stream_info.stats_stream_id,
      axi_stats_stream->stream_handle);
    rc = ioctl(axi_hw->fd, VIDIOC_MSM_ISP_RELEASE_STATS_STREAM, &rel_cmd);
    if (rc < 0) {
      CDBG_ERROR("%s: cannot release stream for stats 0x%x\n",
        __func__, axi_stats_stream->stats_stream_info.stats_type);
    }

    rc = iface_stats_unreg_buf(axi_hw, axi_stats_stream);
    if (rc < 0) {
      CDBG_ERROR("%s: error iface_stats_unreg_buf! rc = %d\n", __func__, rc);
      return rc;
    }

    memset(axi_stats_stream, 0, sizeof(iface_axi_stats_stream_t));
  }

  return rc;
}

/** iface_dump_axi_plane_config:
 *
 *    @stream:
 *    @axi_plane_cfg:
 *    @cam_format:
 *
 **/
void iface_dump_axi_plane_config(iface_axi_stream_t *stream,
  struct msm_vfe_axi_stream_request_cmd *axi_request_cfg,
  cam_format_t cam_format)
{
  uint32_t i = 0;
  uint32_t plane_num = 0;
  uint32_t hw_stream_id = 0;
  struct msm_vfe_axi_plane_cfg *axi_plane_cfg = NULL;
  isp_stripe_id_t stripe_id = ISP_STRIPE_MAX;
  uint32_t is_split = 0;
  char *axi_intf = NULL;

  if (stream == NULL || axi_request_cfg == NULL) {
    IFACE_ERR("NULL pointer stream %p, request_cmd %p, return\n",
      stream, axi_request_cfg);
    return;
  }

  stripe_id = stream->hw_stream_info.isp_split_output_info.stripe_id;
  is_split = stream->hw_stream_info.isp_split_output_info.is_split;
  axi_plane_cfg = &axi_request_cfg->plane_cfg[0];

  switch (axi_request_cfg->stream_src) {
  case PIX_ENCODER:
    axi_intf = "ENCODER";
    break;

  case PIX_VIEWFINDER:
    axi_intf = "VIEWFINDER";
    break;

  case PIX_VIDEO:
    axi_intf = "VIDEO";
    break;

  case CAMIF_RAW:
    axi_intf = "CAMIF_RAW";
    break;

  case IDEAL_RAW:
    axi_intf = "IDEAL_RAW";
    break;

  case RDI_INTF_0:
    axi_intf = "RDI_0";
    break;

  case RDI_INTF_1:
    axi_intf = "RDI_1";
    break;

  case RDI_INTF_2:
    axi_intf = "RDI_2";
    break;

  default:
    axi_intf = "axi_src_invalid";
    break;
  }

  IFACE_HIGH("=== AXI DUMP: VFE%s session_id %d, hw_stream_id %x ===\n",
    (is_split == 0) ? "":
    ((stripe_id == ISP_STRIPE_RIGHT) ? "_Right" : "_Left"),
    stream->hw_stream_info.session_id,
    stream->hw_stream_info.hw_stream_id);
  IFACE_HIGH("%s Axi_src: %s hw_stream width %d, height %d\n",
    "    AXI DUMP", axi_intf,
    stream->hw_stream_info.stream_info.dim.width,
    stream->hw_stream_info.stream_info.dim.height);
  IFACE_HIGH("need buf divert = %d, burst count = %d\n",
    axi_request_cfg->buf_divert, axi_request_cfg->burst_count);
  IFACE_HIGH("hfr mode = %d, skip pattern = %d, init_frame_drop = %d\n",
    axi_request_cfg->hfr_mode, axi_request_cfg->frame_skip_pattern,
    axi_request_cfg->init_frame_drop);

  switch (cam_format) {
  case CAM_FORMAT_YUV_420_NV12:
  case CAM_FORMAT_YUV_420_NV12_UBWC:
  case CAM_FORMAT_YUV_420_NV12_VENUS:
  case CAM_FORMAT_YUV_420_NV21_VENUS:
  case CAM_FORMAT_YUV_420_NV21:
  case CAM_FORMAT_YUV_420_NV21_ADRENO:
  case CAM_FORMAT_YUV_422_NV16:
  case CAM_FORMAT_YUV_422_NV61:
  case CAM_FORMAT_YUV_444_NV24:
  case CAM_FORMAT_YUV_444_NV42: {
    /* two planes */
    plane_num = 2;
  }
    break;

  case CAM_FORMAT_YUV_420_YV12: {
    /* 3 planes */
    plane_num = 3;
  }
    break;

  default: {
    /*single plane*/
    plane_num = 1;
  }
    break;
  }

  for (i = 0; i < plane_num; i++) {
    IFACE_HIGH("plane[%d]: plane_fmt %d(Y-0/CB-1/Cr-2/CrCb-3/CbCr-4)\n",
      i, axi_plane_cfg[i].output_plane_format);
    IFACE_HIGH("plane[%d]: width = %d, height = %d\n", i,
      axi_plane_cfg[i].output_width, axi_plane_cfg[i].output_height);
    IFACE_HIGH("plane[%d]: stride = %d, scanlines = %d\n", i,
      axi_plane_cfg[i].output_stride,  axi_plane_cfg[i].output_scan_lines);
    IFACE_HIGH("plane[%d]: address_offset %x\n", i,
       axi_plane_cfg[i].plane_addr_offset);
   }

  return;
}

/** iface_dump_axi_update_info:
 *
 *    @stream:
 *    @axi_plane_cfg:
 *    @cam_format:
 *
 **/
void iface_dump_axi_update_info(iface_axi_stream_t *stream,
  struct msm_vfe_axi_plane_cfg *axi_plane_cfg,
  cam_format_t cam_format)
{
  uint32_t i;
  uint32_t plane_num =0;
  uint32_t hw_stream_id;

  if (stream == NULL || axi_plane_cfg == NULL) {
    CDBG_ERROR("%s: NULL pointer stream %p, axi_plane_cfg %p, return\n",
      __func__, stream, axi_plane_cfg);
    return;
  }

  CDBG("%s:=== CDS DUMP: session_id %d,  hw_stream_id %x ====\n",
    __func__, stream->hw_stream_info.session_id,
    stream->hw_stream_info.hw_stream_id);

  switch (cam_format) {
  case CAM_FORMAT_YUV_420_NV12:
  case CAM_FORMAT_YUV_420_NV12_UBWC:
  case CAM_FORMAT_YUV_420_NV12_VENUS:
  case CAM_FORMAT_YUV_420_NV21_VENUS:
  case CAM_FORMAT_YUV_420_NV21:
  case CAM_FORMAT_YUV_420_NV21_ADRENO:
  case CAM_FORMAT_YUV_422_NV16:
  case CAM_FORMAT_YUV_422_NV61:
  case CAM_FORMAT_YUV_444_NV24:
  case CAM_FORMAT_YUV_444_NV42: {
    /* two planes */
    plane_num = 2;
  }
    break;

  case CAM_FORMAT_YUV_420_YV12: {
    /* 3 planes */
    plane_num = 3;
  }
    break;

  default: {
    /*single plane*/
    plane_num = 1;
  }
    break;
  }

  for (i = 0; i < plane_num; i++) {
    CDBG("%s: plane[%d]: plane_fmt %d(Y-0/CB-1/Cr-2/CrCb-3/CbCr-4)\n",
      __func__, i, axi_plane_cfg[i].output_plane_format);
    CDBG("%s: plane[%d]: width = %d, height = %d\n", __func__, i,
      axi_plane_cfg[i].output_width, axi_plane_cfg[i].output_height);
    CDBG("%s: plane[%d]: stride = %d, scanlines = %d\n", __func__, i,
      axi_plane_cfg[i].output_stride,  axi_plane_cfg[i].output_scan_lines);
    CDBG("%s: plane[%d]: address_offset %x\n", __func__, i,
       axi_plane_cfg[i].plane_addr_offset);
   }

  return;
}

/** iface_axi_stream_unconfig:
 *
 *    @axi:
 *    @in_params:
 *    @in_params_size:
 *
 **/
static int iface_axi_stream_unconfig(iface_axi_hw_t *axi_hw,
  iface_hw_stream_t *hw_stream, uint32_t in_params_size)
{
  int i, rc = 0;
  iface_axi_stream_t *stream = NULL;
  start_stop_stream_t release_stream_param;
  uint32_t stream_id;
  uint8_t stream_idx;
  enum msm_vfe_input_src intf = VFE_SRC_MAX;
  struct msm_vfe_axi_stream_release_cmd *cmd;

  cmd = &axi_hw->work_struct.u.stream_release_cmd;

  if (in_params_size != sizeof(iface_hw_stream_t)) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -1;
  }

  stream = iface_axi_util_find_stream(axi_hw, hw_stream->session_id, hw_stream->hw_stream_id);
  if (stream) {
    /* if axi stream exists in kernel release it */
    if (stream->axi_stream_handle) {
      memset(&axi_hw->work_struct, 0, sizeof(axi_hw->work_struct));
      cmd->stream_handle = stream->axi_stream_handle;

      rc = ioctl(axi_hw->fd, VIDIOC_MSM_ISP_RELEASE_STREAM, cmd);
      if (rc < 0) {
        CDBG_ERROR("%s: ISP_RELEASE_STREAM error= %d\n", __func__, rc);
        return rc;
      }

      intf = SRC_TO_INTF(hw_stream->axi_path);
      axi_hw->intf_param[intf].session_id = 0;
      stream_idx = stream->axi_stream_handle & 0xFF;
      if (stream_idx < VFE_AXI_SRC_MAX) {
        axi_hw->axi_handle_to_stream_idx[stream_idx] = -1;
      }
      stream->axi_stream_handle = 0;
    }

    memset(stream, 0, sizeof(iface_axi_stream_t));
    stream->state = IFACE_AXI_STREAM_STATE_INVALID;
  }

  return rc;
}

/** iface_axi_stream_set_stats_skip_pattern:
 *
 *    @axi:
 *    @skip_pattern:
 *    @in_params_size:
 *
 **/

static int iface_axi_stream_set_skip_pattern_all_stream(
  iface_axi_hw_t                   *axi_hw,
  iface_param_frame_skip_pattern_t *skip_pattern,
  uint32_t                          in_params_size __unused)
{

  int rc = 0;
  int i = 0;
  struct msm_vfe_axi_stream_update_cmd axi_update_cmd;

  if (!skip_pattern->use_sw_skip) {
    memset(&axi_update_cmd, 0, sizeof(struct msm_vfe_axi_stream_update_cmd));

    /*stream handle will have two instance.
      need to request the handle and save in each axi instance*/
    for (i = 0; i < IFACE_MAX_STREAMS; i++) {
      if (axi_hw->stream[i].axi_stream_handle &&
        ((int)axi_hw->stream[i].state == (int)IFACE_HW_STREAM_STATE_ACTIVE) &&
        !axi_hw->stream[i].hw_stream_info.controllable_output) {
        axi_update_cmd.update_info[axi_update_cmd.num_streams].stream_handle =
          axi_hw->stream[i].axi_stream_handle;
        if (skip_pattern->bracketing_state == MCT_BRACKETING_STATE_DEINIT &&
          (axi_hw->stream[i].hw_stream_info.frame_skip_pattern > skip_pattern->pattern)) {
          axi_update_cmd.update_info[axi_update_cmd.num_streams].skip_pattern =
            axi_hw->stream[i].hw_stream_info.frame_skip_pattern;
        } else {
          axi_update_cmd.update_info[axi_update_cmd.num_streams].skip_pattern =
            skip_pattern->pattern;
        }

        CDBG("%s: stream_handle:%x state:%d bracketing:%d, pattern:%d\n", __func__,
          axi_update_cmd.update_info[axi_update_cmd.num_streams].stream_handle,
          axi_hw->stream[i].state,
          skip_pattern->bracketing_state,
          skip_pattern->pattern);
        axi_update_cmd.num_streams++;
      }
    }

    axi_update_cmd.update_type = UPDATE_STREAM_FRAMEDROP_PATTERN;

    if (axi_update_cmd.num_streams == 0) {
      CDBG_HIGH("%s: warning: no stream to update skip pattern!\n", __func__);
      return rc;
    }
  } else {
    rc = iface_axi_prepare_sw_frame_skip_info(axi_hw, skip_pattern, &axi_update_cmd);
    if (rc < 0) {
      CDBG("%s: no axi skip requested!\n",
                  __func__);
      return 0;
    }
  }

  rc = ioctl(axi_hw->fd, VIDIOC_MSM_ISP_UPDATE_STREAM, &axi_update_cmd);
  if (rc < 0)
    CDBG_ERROR("%s: VIDIOC_MSM_ISP_UPDATE_STREAM error= %d\n",
                __func__, rc);

  return rc;
}

/** iface_axi_stream_set_stats_skip_pattern:
 *
 *    @axi:
 *    @skip_pattern:
 *    @in_params_size:
 *
 **/
static int iface_axi_stream_set_stats_skip_pattern(
  iface_axi_hw_t                   *axi_hw,
  iface_param_frame_skip_pattern_t *skip_pattern,
  uint32_t                          in_params_size __unused)
{
  int rc = 0;
  uint32_t i = 0;
  iface_axi_stats_stream_t *stats_stream = NULL;
  struct msm_vfe_axi_stream_update_cmd axi_update_cmd;
  iface_hw_stream_t *hw_stream = NULL;
  iface_param_frame_skip_pattern_t frame_skip;
  int stream_idx_in_array;

  if (!skip_pattern->use_sw_skip) {
    memset(&axi_update_cmd, 0, sizeof(struct msm_vfe_axi_stream_update_cmd));
    for (i = RDI_INTF_0; i != VFE_AXI_SRC_MAX; i++) {
      stream_idx_in_array = axi_hw->axi_handle_to_stream_idx[i];
      hw_stream = &axi_hw->stream[stream_idx_in_array].hw_stream_info;
      if (hw_stream->axi_path < RDI_INTF_0 ||
          hw_stream->axi_path == VFE_AXI_SRC_MAX ||
          hw_stream->state != IFACE_HW_STREAM_STATE_RESERVED)
        continue;
      if (!hw_stream->divert_to_3a)
        continue;
      frame_skip.pattern = (enum msm_vfe_frame_skip_pattern)
                                        (skip_pattern->pattern);
      frame_skip.session_id = hw_stream->session_id;
      frame_skip.hw_stream_id = hw_stream->hw_stream_id;
      frame_skip.use_sw_skip = FALSE;
      rc = iface_axi_stream_set_skip_pattern(axi_hw, &frame_skip, rc);
      if (rc < 0)
        CDBG_ERROR("%s: VIDIOC_MSM_ISP_UPDATE_STREAM error= %d\n",
                __func__, rc);
    }

    /*stream handle will have two instance.
      need to request the handle and save in each axi instance*/
    for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
      if (axi_hw->stats_stream[i].stream_handle) {
        axi_update_cmd.update_info[axi_update_cmd.num_streams].stream_handle =
          axi_hw->stats_stream[i].stream_handle;
        axi_update_cmd.update_info[axi_update_cmd.num_streams].skip_pattern =
          skip_pattern->pattern;

        CDBG("%s: stats handle %x, type %d, pattern %d\n", __func__,
          axi_update_cmd.update_info[axi_update_cmd.num_streams].stream_handle,
          axi_hw->stats_stream[i].stats_stream_info.stats_type,
          skip_pattern->pattern);
        axi_update_cmd.num_streams++;
      }
    }

    axi_update_cmd.update_type = UPDATE_STREAM_STATS_FRAMEDROP_PATTERN;

    if (axi_update_cmd.num_streams == 0) {
      CDBG_ERROR("%s: no stream to update skip pattern!\n", __func__);
      return rc;
    }
  } else {
    rc = iface_stats_prepare_sw_frame_skip_info(axi_hw, skip_pattern,
                                                &axi_update_cmd);
    if (rc < 0) {
      CDBG("%s: no stats skip requestd!\n",
                  __func__);
      return 0;
    }
  }

  rc = ioctl(axi_hw->fd, VIDIOC_MSM_ISP_UPDATE_STATS_STREAM, &axi_update_cmd);
  if (rc < 0)
    CDBG_ERROR("%s: VIDIOC_MSM_ISP_UPDATE_STATS_STREAM error= %d\n",
                __func__, rc);

  return rc;
}

/** iface_axi_stream_set_skip_pattern:
 *
 *    @axi:
 *    @skip_pattern:
 *    @in_params_size:
 *
 **/
static int iface_axi_stream_set_skip_pattern(
  iface_axi_hw_t                   *axi,
  iface_param_frame_skip_pattern_t *skip_pattern,
  uint32_t                          in_params_size __unused)
{
  int rc = 0;
  iface_axi_stream_t *stream = NULL;
  struct msm_vfe_axi_stream_update_cmd update_cmd;

  CDBG("%s:E session_id = %d\n",
    __func__, skip_pattern->session_id);

  if (!skip_pattern->use_sw_skip) {
    stream = iface_axi_util_find_stream(axi, skip_pattern->session_id,
               skip_pattern->hw_stream_id);

    if (!stream) {
      CDBG_ERROR("%s: cannot find stream, session_id = %d, stream_id = %d\n",
        __func__, skip_pattern->session_id, skip_pattern->hw_stream_id);
      return 0;
    }

    if (stream->hw_stream_info.controllable_output) {
      /* In case of controllable output "frame skip" does not make sense because
       * the stream is always in a per frame request mode. Thus we are dropping
       * the frame skip configuration for this stream */
      return 0;
    }

    update_cmd.num_streams = 1;
    update_cmd.update_info[0].skip_pattern = skip_pattern->pattern;
    update_cmd.update_info[0].stream_handle = stream->axi_stream_handle;
    update_cmd.update_type = UPDATE_STREAM_FRAMEDROP_PATTERN;
  } else {
    rc = iface_axi_prepare_sw_frame_skip_info(axi, skip_pattern, &update_cmd);
    if (rc < 0) {
      CDBG("%s: no axi skip requested!\n",
                  __func__);
      return 0;
    }
  }

  rc = ioctl(axi->fd, VIDIOC_MSM_ISP_UPDATE_STREAM, &update_cmd);
  if (rc < 0)
    CDBG_ERROR("%s: MSM_ISP_UPDATE_STREAM error= %d\n", __func__, rc);

  return rc;
}

/** iface_axi_stream_request_frame:
 *
 *    @axi:
 *    @frame_request:
 *    @in_params_size:
 *
 **/
int iface_axi_stream_request_frame(
  iface_axi_hw_t              *axi,
  iface_param_frame_request_t *frame_request,
  uint32_t                     in_params_size __unused)
{
  int rc = 0;
  iface_axi_stream_t *stream = NULL;
  struct msm_vfe_axi_stream_update_cmd update_cmd;
  ATRACE_BEGIN_SNPRINTF(32, "iface_request_frame:%d", frame_request->frame_id);

  stream = iface_axi_util_find_stream(axi, frame_request->session_id,
              frame_request->hw_stream_id);

  if (!stream) {
    CDBG_ERROR("%s: cannot find stream, session_id = %d, stream_id = %d\n",
      __func__, frame_request->session_id, frame_request->hw_stream_id);
    return 0;
  }

  if (!stream->hw_stream_info.controllable_output) {
    CDBG_ERROR("%s: hw stream id %d is not controllable\n",
      __func__, stream->hw_stream_info.hw_stream_id);
    return -1;
  }

  update_cmd.num_streams = 1;

#ifdef UPDATE_STREAM_REQUEST_FRAMES_VER2
  {
    struct msm_vfe_axi_stream_cfg_update_info_req_frm *req_frm_ver2;
    update_cmd.update_type = UPDATE_STREAM_REQUEST_FRAMES_VER2;
    req_frm_ver2 = (struct msm_vfe_axi_stream_cfg_update_info_req_frm *)
                  &update_cmd.req_frm_ver2;
    req_frm_ver2->user_stream_id =
      frame_request->user_stream_id;
    req_frm_ver2->stream_handle = stream->axi_stream_handle;
    req_frm_ver2->frame_id = frame_request->frame_id;
    req_frm_ver2->buf_index = frame_request->buf_index;
  }
#else
  update_cmd.update_type = UPDATE_STREAM_REQUEST_FRAMES;
  update_cmd.update_info[0].user_stream_id = frame_request->user_stream_id;
  update_cmd.update_info[0].stream_handle = stream->axi_stream_handle;
  update_cmd.update_info[0].frame_id = frame_request->frame_id;
#endif

  rc = ioctl(axi->fd, VIDIOC_MSM_ISP_UPDATE_STREAM, &update_cmd);
  if (rc < 0)
    CDBG_ERROR("%s: MSM_ISP_UPDATE_STREAM error= %d\n", __func__, rc);

  ATRACE_END();
  return rc;
}

/** iface_axi_stream_add_bufq:
 *
 *    @axi:
 *    @frame_request:
 *    @in_params_size:
 *
 **/
static int iface_axi_stream_add_bufq(
  iface_axi_hw_t          *axi,
  iface_param_add_queue_t *in_param,
  uint32_t                 in_params_size __unused)
{
  int rc = 0;
  iface_axi_stream_t *stream = NULL;
  struct msm_vfe_axi_stream_update_cmd update_cmd;

  stream = iface_axi_util_find_stream(axi, in_param->session_id,
    in_param->hw_stream_id);

  if (!stream) {
    CDBG_ERROR("%s: cannot find stream, session_id = %d, stream_id = %d\n",
      __func__, in_param->session_id, in_param->hw_stream_id);
    return 0;
  }

  update_cmd.num_streams = 1;
  update_cmd.update_info[0].user_stream_id = in_param->user_stream_id;
  update_cmd.update_info[0].stream_handle = stream->axi_stream_handle;
  update_cmd.update_type = UPDATE_STREAM_ADD_BUFQ;
  rc = ioctl(axi->fd, VIDIOC_MSM_ISP_UPDATE_STREAM, &update_cmd);
  if (rc < 0)
    CDBG_ERROR("%s: MSM_ISP_UPDATE_STREAM error= %d\n", __func__, rc);

  return rc;
}

/** iface_axi_stream_remove_bufq:
 *
 *    @axi:
 *    @frame_request:
 *    @in_params_size:
 *
 **/
static int iface_axi_stream_remove_bufq(
  iface_axi_hw_t          *axi,
  iface_param_add_queue_t *in_param,
  uint32_t                 in_params_size __unused)
{
  int rc = 0;
  iface_axi_stream_t *stream = NULL;
  struct msm_vfe_axi_stream_update_cmd update_cmd;

  stream = iface_axi_util_find_stream(axi, in_param->session_id,
    in_param->hw_stream_id);

  if (!stream) {
    CDBG_ERROR("%s: cannot find stream, session_id = %d, stream_id = %d\n",
      __func__, in_param->session_id, in_param->hw_stream_id);
    return 0;
  }

  update_cmd.num_streams = 1;
  update_cmd.update_info[0].user_stream_id = in_param->user_stream_id;
  update_cmd.update_info[0].stream_handle = stream->axi_stream_handle;
  update_cmd.update_type = UPDATE_STREAM_REMOVE_BUFQ;
  rc = ioctl(axi->fd, VIDIOC_MSM_ISP_UPDATE_STREAM, &update_cmd);
  if (rc < 0)
    CDBG_ERROR("%s: MSM_ISP_UPDATE_STREAM error= %d\n", __func__, rc);

  return rc;
}

/** iface_axi_stream_set_stream_update:
 *
 *    @axi:
 *    @stream_cfg:
 *    @in_params_size:
 *
 **/
static int iface_axi_stream_set_stream_update(
  iface_axi_hw_t        *axi_hw,
  iface_stream_update_t *stream_update_parm,
  uint32_t               in_params_size __unused)
{
  int rc = 0;
  int i;
  iface_axi_stream_t *stream = NULL;
  struct msm_vfe_axi_stream_update_cmd *update_cmd;
  update_cmd = &axi_hw->update_cmd;
  update_cmd->update_type = UPDATE_STREAM_AXI_CONFIG;
  uint32_t num_streams = 0;

  /*PACK different stream cfg toaxi  update command*/
  for (i = 0; i < stream_update_parm->num_hw_streams; i++) {
    stream = iface_axi_util_find_stream(axi_hw,
      stream_update_parm->session_id, stream_update_parm->hw_stream_ids[i]);
    if (!stream) {
      CDBG_ERROR("%s: cannot find stream, hw stream id = %x, session id = %d\n",
        __func__, stream_update_parm->hw_stream_ids[i],
        stream_update_parm->session_id);
      continue;
    }

    stream->hw_stream_info.need_uv_subsample =
      stream_update_parm->uv_subsample_enb;
    update_cmd->update_info[num_streams].stream_handle = stream->axi_stream_handle;
    rc = iface_axi_util_fill_plane_info(axi_hw,
      update_cmd->update_info[num_streams].plane_cfg, stream);
    if (rc < 0) {
      CDBG_ERROR("%s: iface_axi_util_fill_plane_info error, rc = %d\n",
        __func__, rc);
      return rc;
    }
    iface_dump_axi_update_info(stream,
      update_cmd->update_info[num_streams].plane_cfg,
      stream->hw_stream_info.stream_info.fmt);

    update_cmd->update_info[num_streams].output_format =
      iface_axi_util_cam_fmt_to_v4l2_fmt(stream->hw_stream_info.stream_info.fmt,
        stream->hw_stream_info.need_uv_subsample);

    num_streams++;
  }

  update_cmd->num_streams = num_streams;

  if (num_streams)
    axi_hw->hw_update_pending = TRUE;

  CDBG("%s: X, rc = %d\n", __func__, rc);
  return rc;
}

/** iface_axi_stream_set_offline_stream_update:
 *
 *    @axi:
 *    @stream_cfg:
 *    @in_params_size:
 *
 **/
static int iface_axi_stream_set_offline_stream_update(
  iface_axi_hw_t        *axi_hw,
  iface_stream_update_t *stream_update_parm,
  uint32_t               in_params_size __unused)
{
  int rc = 0;
  int i;
  iface_axi_stream_t *stream = NULL;
  struct msm_vfe_axi_stream_update_cmd *update_cmd;
  update_cmd = &axi_hw->update_cmd;
#ifdef VIDIOC_MSM_ISP_MAP_BUF_START_MULTI_PASS_FE
  update_cmd->update_type = UPDATE_STREAM_OFFLINE_AXI_CONFIG;
#else
  update_cmd->update_type = UPDATE_STREAM_AXI_CONFIG;
#endif
  uint32_t num_streams = 0;

  /*PACK different stream cfg toaxi  update command*/
  for (i = 0; i < stream_update_parm->num_hw_streams; i++) {
    stream = iface_axi_util_find_stream(axi_hw,
      stream_update_parm->session_id, stream_update_parm->hw_stream_ids[i]);
    if (!stream) {
      CDBG_ERROR("%s: cannot find stream, hw stream id = %x,session id = %d\n",
        __func__, stream_update_parm->hw_stream_ids[i],
        stream_update_parm->session_id);
      continue;
    }

    stream->hw_stream_info.isp_split_output_info.stripe_id =
      stream_update_parm->stripe_id;
    update_cmd->update_info[num_streams].stream_handle =
      stream->axi_stream_handle;
    rc = iface_axi_util_fill_plane_info(axi_hw,
      update_cmd->update_info[num_streams].plane_cfg, stream);
    if (rc < 0) {
      CDBG_ERROR("%s: iface_axi_util_fill_plane_info error, rc = %d\n",
        __func__, rc);
      return rc;
    }
    iface_dump_axi_update_info(stream,
      update_cmd->update_info[num_streams].plane_cfg,
      stream->hw_stream_info.stream_info.fmt);
    update_cmd->update_info[num_streams].output_format =
      iface_axi_util_cam_fmt_to_v4l2_fmt(
        stream->hw_stream_info.stream_info.fmt,
        stream->hw_stream_info.need_uv_subsample);

    num_streams++;
  }

  update_cmd->num_streams = num_streams;

  if (num_streams)
    axi_hw->hw_update_pending = TRUE;

  CDBG("%s: X, rc = %d\n", __func__, rc);
  return rc;
}


/** iface_axi_deinit_native_buf:
 *
 *    @axi:
 *    @stream:
 *
 **/
static int iface_axi_deinit_native_buf(
  iface_axi_hw_t     *axi __unused,
  iface_axi_stream_t *stream __unused)
{
  int rc = 0;

  return rc;
}

/** iface_axi_reg_input_buf:
 *
 *    @axi:
 *    @params:
 *
 **/
static int iface_axi_reg_input_buf(iface_axi_hw_t *axi_hw, start_stop_stream_t *start_parm)
{
  int rc = 0;
  iface_param_add_queue_t add_bufq;
  uint32_t buf_handle;

    buf_handle = iface_find_matched_bufq_handle(axi_hw->buf_mgr,
      start_parm->session_id, start_parm->stream_ids[0]);
    if (buf_handle == 0) {
      CDBG_ERROR("%s: cannot find buf handle, sessid = %d, straemid = %x\n",
        __func__, start_parm->session_id, start_parm->stream_ids[0]);
      return -1;
    }

    CDBG("%s: register user bufq 0x%x for user stream %x\n",
      __func__, buf_handle, start_parm->stream_ids[0]);

    rc = iface_register_buf(axi_hw->buf_mgr, buf_handle, axi_hw->fd);
    if (rc < 0) {
      CDBG_ERROR("%s: isp_register_buf error, sessid = %d, straemid = %d\n",
        __func__, start_parm->session_id, start_parm->stream_ids[0]);
      return rc;
    }
  return 0;
}

/** iface_axi_unreg_buf:
 *
 *    @axi:
 *    @params:
 *
 **/
static int iface_axi_unreg_input_buf(iface_axi_hw_t *axi_hw, start_stop_stream_t *start_stop_parm)
{
  int rc = 0;
  uint32_t buf_handle;
  iface_param_add_queue_t remove_bufq;

    buf_handle = iface_find_matched_bufq_handle(axi_hw->buf_mgr,
      start_stop_parm->session_id, start_stop_parm->stream_ids[0]);
    if (buf_handle == 0) {
      CDBG_ERROR("%s: cannot find buf handle, sessid = %d, straemid = %x\n",
        __func__, start_stop_parm->session_id, start_stop_parm->stream_ids[0]);
      return -1;
    }
    rc = iface_unregister_buf(axi_hw->buf_mgr, buf_handle, axi_hw->fd);
    if (rc < 0) {
      CDBG_ERROR("%s: isp_unregister_buf error, sessid = %d, straemid = %x\n",
        __func__, start_stop_parm->session_id, start_stop_parm->stream_ids[0]);
      return -1;
    }

    CDBG("%s: Unregister buf session_id = %d, stream_id = %x, buf_handle = 0x%x\n",
      __func__, start_stop_parm->session_id, start_stop_parm->stream_ids[0], buf_handle);

  return rc;
}

/** iface_axi_reg_buf:
 *
 *    @axi:
 *    @params:
 *
 **/
static int iface_axi_reg_buf(iface_axi_hw_t *axi_hw, start_stop_stream_t *start_parm)
{
  int rc = 0;
  int i, k;
  iface_axi_stream_t *stream;
  iface_param_add_queue_t add_bufq;
  uint32_t buf_handle;
  uint32_t stream_id;

  for (i = 0; i < start_parm->num_streams; i++) {
    stream = iface_axi_util_find_stream(axi_hw, start_parm->session_id,
               start_parm->stream_ids[i]);
    if (stream == NULL) {
      CDBG_ERROR("%s: cannot find the stream, rc=  -1\n", __func__);
      return -1;
    }

    /*Peter: where do we associate buf mgr to axi,
      bufq handle will be set up when iface request buf during streamon*/
    if (stream->hw_stream_info.controllable_output &&
        stream->hw_stream_info.shared_hw_stream) {
      stream_id = start_parm->mct_streamon_id[i];
    } else {
      stream_id = stream->hw_stream_info.hw_stream_id;
    }

    buf_handle = iface_find_matched_bufq_handle(axi_hw->buf_mgr,
      stream->hw_stream_info.session_id, stream_id);
    if (buf_handle == 0) {
      CDBG_ERROR("%s: cannot find buf handle, sessid = %d, straemid = %x\n",
        __func__, stream->hw_stream_info.session_id, stream_id);
      return -1;
    }

    CDBG("%s: register user bufq 0x%x for user stream %x\n",
      __func__, buf_handle, stream_id);

    rc = iface_register_buf(axi_hw->buf_mgr, buf_handle, axi_hw->fd);
    if (rc < 0) {
      CDBG_ERROR("%s: isp_register_buf error, sessid = %d, straemid = %d\n",
        __func__, stream->hw_stream_info.session_id, stream_id);
      return rc;
    }

    memset(&add_bufq, 0, sizeof(add_bufq));
    add_bufq.session_id = start_parm->session_id;
    add_bufq.hw_stream_id = stream->hw_stream_info.hw_stream_id;
    add_bufq.user_stream_id = stream_id;

    CDBG("%s: Add bufq in kernel with hw stream 0x%x, use stream %d\n",
      __func__, add_bufq.hw_stream_id, add_bufq.user_stream_id);

    rc = iface_axi_stream_add_bufq(axi_hw, &add_bufq, sizeof(add_bufq));
    if (rc < 0) {
      CDBG_ERROR("%s: iface_axi_stream_add_bufq, rc = %d\n", __func__, rc);
      return rc;
    }
  }

  return 0;
}

/** iface_axi_unreg_buf:
 *
 *    @axi:
 *    @params:
 *
 **/
static int iface_axi_unreg_buf(iface_axi_hw_t *axi_hw, start_stop_stream_t *start_stop_parm)
{
  int rc = 0;
  int i, k;
  iface_axi_stream_t *stream;
  uint32_t buf_handle;
  uint32_t stream_id;
  iface_param_add_queue_t remove_bufq;

  for (i = 0; i < start_stop_parm->num_streams; i++) {
    stream = iface_axi_util_find_stream(axi_hw, start_stop_parm->session_id,
               start_stop_parm->stream_ids[i]);

    if (stream == NULL) {
      CDBG_ERROR("%s: cannot find the stream\n", __func__);
      continue;
    }

    if (stream->hw_stream_info.controllable_output &&
        stream->hw_stream_info.shared_hw_stream) {
      stream_id = start_stop_parm->mct_streamon_id[i];
    } else {
      stream_id = stream->hw_stream_info.hw_stream_id;
    }

    buf_handle = iface_find_matched_bufq_handle(axi_hw->buf_mgr,
      stream->hw_stream_info.session_id, stream_id);
    if (buf_handle == 0) {
      CDBG_ERROR("%s: cannot find buf handle, sessid = %d, straemid = %x\n",
        __func__, stream->hw_stream_info.session_id, stream_id);
      return -1;
    }

    memset(&remove_bufq, 0, sizeof(remove_bufq));
    remove_bufq.session_id = start_stop_parm->session_id;
    remove_bufq.hw_stream_id = stream->hw_stream_info.hw_stream_id;
    remove_bufq.user_stream_id = stream_id;

    CDBG("%s: Revmoe bufq in kernel with hw stream 0x%x, use stream %d\n",
      __func__, remove_bufq.hw_stream_id, remove_bufq.user_stream_id);

    rc = iface_axi_stream_remove_bufq(axi_hw, &remove_bufq, sizeof(remove_bufq));
    if (rc < 0) {
      CDBG_ERROR("%s: iface_axi_stream_remove_bufq, rc = %d\n", __func__, rc);
      return rc;
    }

    rc = iface_unregister_buf(axi_hw->buf_mgr, buf_handle, axi_hw->fd);
    if (rc < 0) {
      CDBG_ERROR("%s: isp_unregister_buf error, sessid = %d, straemid = %x\n",
        __func__, stream->hw_stream_info.session_id, stream_id);
      return -1;
    }

    CDBG("%s: Unregister buf session_id = %d, stream_id = %x, buf_handle = 0x%x\n",
      __func__, stream->hw_stream_info.session_id, stream_id, buf_handle);
  }

  return rc;
}

/** iface_axi_cfg_stats_stream_stop:
 *
 *    @axi_hw: axi handle
 *    @start_stop_parm: start/stop info
 *    @num_pix_streams
 *
 *    return integer rc
 *
 **/
static int iface_axi_cfg_stats_stream_stop(
  iface_axi_hw_t      *axi_hw,
  start_stop_stream_t *start_stop_parm __unused,
  uint32_t             num_pix_streams)
{
  int rc = 0, i;
  struct msm_vfe_stats_stream_cfg_cmd cmd;
  uint32_t handle;
  memset(&cmd, 0, sizeof(cmd));

  /*only stop the stats when last stream off*/
  if (num_pix_streams > axi_hw->num_pix_streams) {
    CDBG_ERROR("%s: Error! Invalid num of streams \n", __func__);
    axi_hw->num_pix_streams = 0; // Stop stats and make count 0;
  } else if (axi_hw->num_pix_streams - num_pix_streams > 0) {
    axi_hw->num_pix_streams -= num_pix_streams;
    return 0;
  } else {
    axi_hw->num_pix_streams -= num_pix_streams;
  }

  /*stream handle will have two instance.
    need to request the handle and save in each axi instance*/
  cmd.enable = FALSE;
  for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
    if (axi_hw->stats_stream[i].stream_handle &&
        axi_hw->stats_stream[i].state == IFACE_AXI_STREAM_STATE_ACTIVE) {
      cmd.stream_handle[cmd.num_streams++] =
        axi_hw->stats_stream[i].stream_handle;
      axi_hw->stats_stream[i].state = IFACE_AXI_STREAM_STATE_STOPPING;
    }
  }


  CDBG_HIGH("%s: stats_cmd: enable  = %d, num of stats stream = %d\n",
    __func__, cmd.enable, cmd.num_streams);

  if (cmd.num_streams == 0) {
    CDBG_HIGH("%s: no stats stream to START/STOP, stats cmd.num_streams = %d\n",
      __func__, cmd.num_streams);
    return rc;
  }

  rc = ioctl(axi_hw->fd, VIDIOC_MSM_ISP_CFG_STATS_STREAM, &cmd);
  if (rc < 0) {
    CDBG_ERROR("%s:  VIDIOC_MSM_ISP_CFG_STATS_STREAM error, rc = %d\n",
    __func__, rc);
  }

  for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
    if (axi_hw->stats_stream[i].stream_handle &&
        axi_hw->stats_stream[i].state == IFACE_AXI_STREAM_STATE_STOPPING) {
      axi_hw->stats_stream[i].state = IFACE_AXI_STREAM_STATE_CFG;
    }
  }


  return rc;
}

/** iface_axi_cfg_stats_stream_start:
 *
 *    @axi_hw: axi handle
 *    @start_stop_parm: start/stop info
 *    @num_pix_streams:
 *
 *    return integer rc
 *
 **/
static int iface_axi_cfg_stats_stream_start(
  iface_axi_hw_t      *axi_hw,
  start_stop_stream_t *start_stop_parm __unused,
  uint32_t             num_pix_streams)
{
  int rc = 0, i;
  struct msm_vfe_stats_stream_cfg_cmd cmd;
  uint32_t handle;
  memset(&cmd, 0, sizeof(cmd));

  /*only start the stats when first stream on*/
  if (axi_hw->num_pix_streams > 0) {
    axi_hw->num_pix_streams += num_pix_streams;
    return 0;
  }
  axi_hw->num_pix_streams += num_pix_streams;

  /*stream handle will have two instance.
    need to request the handle and save in each axi instance*/
  for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
    if (axi_hw->stats_stream[i].stream_handle &&
        axi_hw->stats_stream[i].state == IFACE_AXI_STREAM_STATE_CFG) {
      cmd.stream_handle[cmd.num_streams++] =
        axi_hw->stats_stream[i].stream_handle;
      axi_hw->stats_stream[i].state = IFACE_AXI_STREAM_STATE_STARTING;
    }
  }

  /* if there are no stats streams - no need to configure and start them */
  if(cmd.num_streams == 0) {
    CDBG_HIGH("%s: no stats stream to START/STOP, stats cmd.num_streams = %d\n",
      __func__, cmd.num_streams);
    return 0;
  }

  cmd.enable = TRUE;

  CDBG_HIGH("%s: stats_cmd: enable = %d, num of stats stream = %d\n",
    __func__, cmd.enable, cmd.num_streams);

  rc = ioctl(axi_hw->fd, VIDIOC_MSM_ISP_CFG_STATS_STREAM, &cmd);
  if (rc < 0) {
    CDBG_ERROR("%s:  VIDIOC_MSM_ISP_CFG_STATS_STREAM error, rc = %d\n",
    __func__, rc);
  }

  for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
    if (axi_hw->stats_stream[i].stream_handle &&
        axi_hw->stats_stream[i].state == IFACE_AXI_STREAM_STATE_STARTING) {
      axi_hw->stats_stream[i].state = IFACE_AXI_STREAM_STATE_ACTIVE;
    }
  }

  return rc;
}

/** iface_axi_set_master_slave_info:
 *
 *    @axi_hw: axi handle
 *
 *    Return 0 on Success
 **/
int iface_axi_set_master_slave_info(iface_axi_hw_t *axi_hw,
  struct msm_isp_set_dual_hw_ms_cmd *ms_cmd)
{
  int rc = 0;

  rc = ioctl(axi_hw->fd, VIDIOC_MSM_ISP_SET_DUAL_HW_MASTER_SLAVE, ms_cmd);
  if (rc < 0) {
    CDBG_ERROR("%s ioctl VIDIOC_MSM_ISP_SET_DUAL_HW_MASTER_SLAVE failed \n",
      __func__);
  }

  return rc;
}

/** iface_axi_set_master_slave_sync
 *
 *    @axi_hw: axi handle
 *
 *    Return 0 on Success
 **/
#ifdef VIDIOC_MSM_ISP_DUAL_HW_MASTER_SLAVE_SYNC
int iface_axi_set_master_slave_sync(iface_axi_hw_t *axi_hw,
  struct msm_isp_dual_hw_master_slave_sync *sync)
{
  int rc = 0;

  rc = ioctl(axi_hw->fd, VIDIOC_MSM_ISP_DUAL_HW_MASTER_SLAVE_SYNC, sync);
  if (rc < 0) {
    CDBG_ERROR("%s ioctl VIDIOC_MSM_ISP_DUAL_HW_MASTER_SLAVE_SYNC failed \n",
      __func__);
  }

  return rc;
}
#endif

/** iface_axi_lpm_session
 *
 *    @axi_hw: axi handle
 *
 *    Return 0 on Success
 **/
#ifdef VIDIOC_MSM_ISP_DUAL_HW_LPM_MODE
int iface_axi_lpm_session(iface_axi_hw_t *axi_hw,
   struct msm_vfe_dual_lpm_mode *ab_ib_vote)
{
  int rc = 0;

  rc = ioctl(axi_hw->fd, VIDIOC_MSM_ISP_DUAL_HW_LPM_MODE, ab_ib_vote);
  if (rc < 0) {
    CDBG_ERROR("%s ioctl VIDIOC_MSM_ISP_DUAL_HW_LPM_MODE failed \n",
      __func__);
  }

  return rc;
}
#endif

/** iface_axi_halt_hw:
 *
 *    @axi_hw: axi handle
 *
 *    Return 0 on Success
 **/
static int iface_axi_halt_hw(iface_axi_hw_t *axi_hw)
{
  int rc = 0;
  struct msm_vfe_axi_halt_cmd *halt_cmd;

  halt_cmd = &axi_hw->work_struct.u.halt_cmd;
  memset(halt_cmd, 0, sizeof(struct msm_vfe_axi_halt_cmd));
  halt_cmd->stop_camif = 1;
  halt_cmd->overflow_detected = 1;
  halt_cmd->blocking_halt = 1;

  rc = ioctl(axi_hw->fd, VIDIOC_MSM_ISP_AXI_HALT, halt_cmd);
  if (rc < 0) {
    CDBG_ERROR("%s ioctl VIDIOC_MSM_ISP_AXI_HALT failed \n", __func__);
  }

  return rc;
}

/** iface_axi_reset_hw:
 *
 *    @axi_hw: axi handle
 *    @action_data: action data
 *    @action_data_size: action size
 *
 *    Reset ISP HW
 *
 *    Return 0 on Success
 **/
static int iface_axi_reset_hw(iface_axi_hw_t *axi_hw, void *action_data,
  uint32_t action_data_size)
{
  int rc = 0;
  struct msm_vfe_axi_reset_cmd *reset_cmd;
  uint32_t *frame_id = NULL;

  if (!action_data) {
    CDBG_ERROR("%s Error! Invalid arguments \n", __func__);
    return -1;
  }
  if (action_data_size != sizeof(uint32_t)) {
    CDBG_ERROR("%s Error! Size mismatch \n", __func__);
    return -1;
  }

  frame_id = (uint32_t *)action_data;
  reset_cmd = &axi_hw->work_struct.u.reset_cmd;
  memset(reset_cmd, 0, sizeof(struct msm_vfe_axi_reset_cmd));
  reset_cmd->blocking = 1;
  reset_cmd->frame_id = (uint32_t)(*frame_id);
  rc = ioctl(axi_hw->fd, VIDIOC_MSM_ISP_AXI_RESET, reset_cmd);
  if (rc < 0) {
    CDBG_ERROR("%s ioctl VIDIOC_MSM_ISP_AXI_RESET failed \n", __func__);
  }

  return rc;
}

/** iface_axi_restart_hw:
 *
 *    @axi_hw:
 *
 *    Restart ISP AXI and CAMIF
 *
 *    Return 0 on Success
 **/
static int iface_axi_restart_hw(iface_axi_hw_t *axi_hw)
{
  int rc = 0;
  struct msm_vfe_axi_restart_cmd *restart_cmd;

  restart_cmd = &axi_hw->work_struct.u.restart_cmd;
  memset(restart_cmd, 0, sizeof(struct msm_vfe_axi_restart_cmd));
  restart_cmd->enable_camif = 1;

  rc = ioctl(axi_hw->fd, VIDIOC_MSM_ISP_AXI_RESTART, restart_cmd);
  if (rc < 0) {
    CDBG_ERROR("%s ioctl VIDIOC_MSM_ISP_AXI_RESTART failed \n", __func__);
  }

  return rc;
}

/** iface_axi_cfg_stream:
 *
 *    @axi:
 *    @params:
 *    @start:
 *
 **/
static int iface_axi_cfg_stream(iface_axi_hw_t *axi_hw, start_stop_stream_t *start_stop_parm,
  boolean start)
{
  int rc = 0;
  int i;
  iface_axi_stream_t *stream;
  struct msm_vfe_axi_stream_cfg_cmd *cmd;

  CDBG("%s: E, start_flag = %d, sessionid = %d", __func__, start,
    start_stop_parm->session_id);

  cmd = &axi_hw->work_struct.u.stream_start_stop_cmd;
  memset(&axi_hw->work_struct, 0, sizeof(axi_hw->work_struct));

  for (i = 0; i < start_stop_parm->num_streams; i++) {
    stream = iface_axi_util_find_stream(axi_hw, start_stop_parm->session_id,
      start_stop_parm->stream_ids[i]);
    if (stream == NULL) {
      CDBG_ERROR("%s: cannot find the stream\n", __func__);
      rc = -100;
      goto end;
    }

    if (((start == TRUE) && (stream->state != IFACE_AXI_STREAM_STATE_CFG)) ||
       ((start == FALSE) && (stream->state != IFACE_AXI_STREAM_STATE_ACTIVE))) {
      CDBG_ERROR("%s: invalid stream state!, stream id %x, state %d\n",
        __func__, stream->hw_stream_info.hw_stream_id, stream->state);
      continue;
    }
    cmd->stream_handle[i] = stream->axi_stream_handle;

    CDBG_HIGH("%s:%d: Cmd (%d) axi stream 0x%x on vfe %d\b",
              __func__, __LINE__, start, stream->axi_stream_handle,
              axi_hw->dev_idx);

    /*update stream state*/
    if (start == TRUE)
      stream->state = IFACE_AXI_STREAM_STATE_STARTING;
    else
      stream->state = IFACE_AXI_STREAM_STATE_STOPPING;
  }

  cmd->num_streams = start_stop_parm->num_streams;
  cmd->sync_frame_id_src = start_stop_parm->sync_frame_id_src;

  if (start == TRUE) {
    cmd->cmd = START_STREAM;
  } else {
    cmd->cmd = start_stop_parm->stop_cmd;
  }

  rc = ioctl(axi_hw->fd, VIDIOC_MSM_ISP_CFG_STREAM, cmd);
  if (rc < 0) {
    CDBG_ERROR("%s: ISP_CFG_STREAM error = %d, start_straem = %d\n",
      __func__, rc, start);
    return rc;
  }

end:
  return rc;
}

/** iface_axi_start_stream:
 *
 *    @axi:
 *    @action_data:
 *    @action_data_size:
 *
 **/
static int iface_axi_start_stream(iface_axi_hw_t *axi_hw,
  start_stop_stream_t *start_parm, uint32_t action_data_size)
{
  int rc = 0, rc_unreg = 0, i = 0;
  uint32_t num_pix_streams = 0;
  boolean start = TRUE;
  iface_axi_stream_t *stream = NULL;

  if (sizeof(start_stop_stream_t) != action_data_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -100;
  }

  for (i = 0; i < start_parm->num_streams; i++) {
    stream = iface_axi_util_find_stream(axi_hw, start_parm->session_id,
      start_parm->stream_ids[i]);
    if (stream == NULL) {
      CDBG_ERROR("%s: cannot find the stream, rc = %d\n", __func__, rc);
      goto unreg_buf;
    }
    if (SRC_TO_INTF(stream->hw_stream_info.axi_path) == VFE_PIX_0) {
      num_pix_streams++;
    }
  }

  if (num_pix_streams) {
    rc = iface_axi_cfg_stats_stream_start(axi_hw, start_parm, num_pix_streams);
    if (rc < 0) {
      CDBG_ERROR("%s: iface_axi_start_stats_stream error = %d\n",
        __func__, rc);
      goto error;
    }
  }

  rc = iface_axi_reg_buf(axi_hw, start_parm);
  if (rc < 0) {
    CDBG_ERROR("%s: iface_axi_reg_buf error = %d\n", __func__, rc);
    goto error;
  }

  if (start_parm->first_pix_start) {
    rc = iface_axi_subscribe_v4l2_event_multi(axi_hw, start);
    if (rc < 0) {
      CDBG_ERROR("%s: iface_axi_subscribe_v4l2_event_multi error = %d\n", __func__, rc);
      goto unreg_buf;
    }
  }

  rc = iface_axi_cfg_stream(axi_hw, start_parm, start);
  if (rc < 0) {
    CDBG_ERROR("%s: iface_axi_streamon error = %d\n", __func__, rc);
    goto unreg_buf;
  }

  /* update strream state*/
  for (i = 0; i < start_parm->num_streams; i++) {
    stream = iface_axi_util_find_stream(axi_hw, start_parm->session_id,
      start_parm->stream_ids[i]);
    if (stream == NULL) {
      CDBG_ERROR("%s: cannot find the stream, rc = %d\n", __func__, rc);
      goto unreg_buf;
    }
    stream->state = IFACE_AXI_STREAM_STATE_ACTIVE;
  }

  axi_hw->num_active_streams += start_parm->num_streams;

  return rc;

unreg_buf:
  rc_unreg = iface_axi_unreg_buf(axi_hw, start_parm);
  if (rc_unreg) {
    CDBG_ERROR("%s: rc_unreg = %d\n", __func__, rc_unreg);
  }
error:
  return rc;
}

/** iface_axi_stop_stream:
 *
 *    @axi:
 *    @action_data:
 *    @action_data_size:
 *
 **/
static int iface_axi_stop_stream(iface_axi_hw_t *axi_hw,
  start_stop_stream_t *stop_parm, uint32_t action_data_size)
{
  int i, rc = 0;
  uint32_t num_pix_streams = 0;
  boolean start = FALSE;
  iface_axi_stream_t *stream;

  if (sizeof(start_stop_stream_t) != action_data_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -100;
  }

  rc = iface_axi_cfg_stream(axi_hw, stop_parm, start);
  if (rc < 0) {
    CDBG_ERROR("%s: isp_axi_stop error = %d\n", __func__, rc);
  }

  rc = iface_axi_unreg_buf(axi_hw, stop_parm);
  if (rc < 0) {
    CDBG_ERROR("%s: isp_axi_wm_uncfg error = %d\n", __func__, rc);
  }

  /* update state*/
  for (i = 0; i < stop_parm->num_streams; i++) {
    stream = iface_axi_util_find_stream(axi_hw, stop_parm->session_id,
      stop_parm->stream_ids[i]);
    if (stream == NULL) {
      CDBG_ERROR("%s: cannot find the stream, rc = %d\n", __func__, rc);
      continue;
    }
    if (SRC_TO_INTF(stream->hw_stream_info.axi_path) == VFE_PIX_0) {
      num_pix_streams++;
    }
    stream->state = IFACE_AXI_STREAM_STATE_CFG;
  }

  rc = iface_axi_cfg_stats_stream_stop(axi_hw, stop_parm, num_pix_streams);
  if (rc < 0) {
    CDBG_ERROR("%s: iface_axi_start_stats_stream error = %d\n",
      __func__, rc);
  }

  axi_hw->num_active_streams -= stop_parm->num_streams;
  if (axi_hw->num_active_streams == 0) {
    rc = iface_axi_subscribe_v4l2_event_multi(axi_hw, start);
    if (rc < 0) {
      CDBG_ERROR("%s: iface_axi_subscribe_v4l2_event_multi error = %d\n", __func__, rc);
      return rc;
    }
  }

  return rc;
}

/** iface_axi_stats_notify_ack:
 *
 *    @axi_hw: axi handle
 *    @stats_ack: ack from ISP
 *    @data_size: size of ack struct
 *
 **/
static int iface_axi_stats_notify_ack(
  iface_axi_hw_t             *axi_hw,
  iface_raw_stats_buf_info_t *stats_ack,
  uint32_t                    data_size __unused)
{
  int rc = 0;
  int i = 0;
  CDBG("%s: E, stats mask = %x\n",
    __func__, stats_ack->stats_mask);

  /* Assume vfe stats buffer were ACCESSED by ISP all the time,
   * assume MSM_CAMERA_BUF_FLAG_CACHE_READ by default
   */
  for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
    if (stats_ack->stats_mask & (1 << i)) {
      rc = iface_queue_buf(axi_hw->buf_mgr,
        axi_hw->stats_stream[i].bufq_handle,
        stats_ack->stats_buf_idxs[i], 1 , axi_hw->fd,
          CPU_HAS_READ);
      if (rc < 0)
        CDBG_ERROR("%s: iface_queue_buf error, i = %d, rc= %d\n",
          __func__, i, rc);
    }
  }

end:
  return rc;

}

/** iface_axi_divert_ack:
 *
 *    @axi:
 *    @ack:
 *    @data_size:
 *
 **/
int iface_axi_divert_ack(
  iface_axi_hw_t             *axi_hw,
  iface_axi_buf_divert_ack_t *ack,
  uint32_t                    data_size __unused)
{
  int rc = 0;
  iface_axi_stream_t *stream;
  uint32_t buf_handle;

  stream = iface_axi_util_find_stream(axi_hw, ack->session_id, ack->stream_id);
  if (stream == NULL) {
    CDBG_ERROR("%s: not find stream, sesid = %d, streamid = %d, nop\n",
      __func__, ack->session_id, ack->stream_id);
    goto end;
  }

  buf_handle = iface_find_matched_bufq_handle(axi_hw->buf_mgr,
    stream->hw_stream_info.session_id, ack->stream_id);
  if (buf_handle == 0) {
    CDBG_ERROR("%s: cannot find buf handle, sessid = %d, straemid = %x, rc = -1\n",
      __func__, stream->hw_stream_info.session_id,
      stream->hw_stream_info.hw_stream_id);
    return -1;
  }

  rc = iface_queue_buf(axi_hw->buf_mgr,
    buf_handle, ack->buf_idx, ack->is_buf_dirty, axi_hw->fd,
    ack->buffer_access);
  if (rc < 0)
    CDBG_ERROR("%s: isp_queue_buf error = %d\n", __func__, rc);

end:
  return rc;

}

/** iface_axi_unmap_buf:
 *
 *    @axi:
 *    @buf_fd:
 *    @data_size:
 *
 **/
static int iface_axi_unmap_buf(iface_axi_hw_t *axi_hw, uint32_t *buf_fd)
{
  int rc = 0;
  if (!buf_fd) {
    CDBG_ERROR("%s: NULL ptr buf_fd=%p\n", __func__, buf_fd);
    return -1;
  }

  rc = iface_unmap_buf(axi_hw->buf_mgr, axi_hw->fd, *buf_fd);
  if (rc < 0)
    CDBG_ERROR("%s: iface_unmap failed fd=%d\n", __func__, *buf_fd);
  return rc;
}

/** iface_axi_do_hw_update:
 *
 *    @axi:
 *
 **/
static int iface_axi_do_hw_update(iface_axi_hw_t *axi_hw)
{
  int rc = 0;
  struct msm_vfe_axi_stream_update_cmd *update_cmd = &axi_hw->update_cmd;

  if (axi_hw->hw_update_pending) {
    rc = ioctl(axi_hw->fd, VIDIOC_MSM_ISP_UPDATE_STREAM, update_cmd);
    if (rc < 0)
      CDBG_ERROR("%s: MSM_ISP_UPDATE_STREAM error= %d\n", __func__, rc);

    axi_hw->hw_update_pending = FALSE;
  }

  return rc;
}

/** iface_axi_reg_shared_buf:
 *
 *    @axi:
 *
 **/
static int iface_axi_reg_shared_buf(iface_axi_hw_t *axi_hw,
  iface_reg_buf_t *in_param, uint32_t action_data_size)
{
  int rc = 0;
  int i, k;
  iface_axi_stream_t *stream;
  uint32_t buf_handle;

  if (sizeof(iface_reg_buf_t) != action_data_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -100;
  }

  stream = iface_axi_util_find_stream(axi_hw, in_param->session_id,
             in_param->hw_stream_id);
  if (stream == NULL) {
    CDBG_ERROR("%s: cannot find the stream, rc=  -1\n", __func__);
    return -1;
  }

  buf_handle = iface_find_matched_bufq_handle(axi_hw->buf_mgr,
    stream->hw_stream_info.session_id, in_param->user_stream_id);
  if (buf_handle == 0) {
    CDBG_ERROR("%s: cannot find buf handle, sessid = %d, straemid = %x\n",
      __func__, stream->hw_stream_info.session_id,
      stream->hw_stream_info.hw_stream_id);
    return -1;
  }

  rc = iface_register_buf(axi_hw->buf_mgr, buf_handle, axi_hw->fd);
  if (rc < 0) {
    CDBG_ERROR("%s: isp_register_buf error, sessid = %d, straemid = %d\n",
      __func__, stream->hw_stream_info.session_id,
      stream->hw_stream_info.hw_stream_id);
    return rc;
  }

  return rc;
}

/** iface_axi_reg_shared_buf:
 *
 *    @axi:
 *
 **/
static int iface_axi_unreg_shared_buf(iface_axi_hw_t *axi_hw,
  iface_reg_buf_t *in_param, uint32_t action_data_size)
{
  int rc = 0;
  int i, k;
  iface_axi_stream_t *stream;
  uint32_t buf_handle;
  uint32_t stream_id;

  if (sizeof(iface_reg_buf_t) != action_data_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -100;
  }

  stream = iface_axi_util_find_stream(axi_hw, in_param->session_id,
             in_param->hw_stream_id);
  if (stream == NULL) {
    CDBG_ERROR("%s: cannot find the stream, rc=  -1\n", __func__);
    return -1;
  }

  if (stream->hw_stream_info.controllable_output &&
      stream->hw_stream_info.shared_hw_stream) {
    stream_id = in_param->user_stream_id;
  } else {
    stream_id = stream->hw_stream_info.hw_stream_id;
  }

  buf_handle = iface_find_matched_bufq_handle(axi_hw->buf_mgr,
    stream->hw_stream_info.session_id, stream_id);
  if (buf_handle == 0) {
    CDBG_ERROR("%s: cannot find buf handle, sessid = %d, straemid = %x, rc = -1\n",
      __func__, stream->hw_stream_info.session_id, stream_id);
    return -1;
  }

  rc = iface_unregister_buf(axi_hw->buf_mgr, buf_handle, axi_hw->fd);
  if (rc < 0) {
    CDBG_ERROR("%s: isp_unregister_buf error, sessid = %d, straemid = %x, rc = -1\n",
      __func__, stream->hw_stream_info.session_id, stream_id);
    return rc;
  }

  return rc;
}

/** iface_axi_map_hw_stream_from_axi_src
 *
 *  Description:
 *  this funciton is called to get hw_stream from axi_src
 *
 *    @axi_hw:          axi hw resource
 *    @axi_src:         represents AXI PATH
 *    @hw_stream_info:  returned hw_stream_info struct
 *
 *  Returns 0 for success and negative error on failure
 **/
static int iface_axi_map_hw_stream_from_axi_src(
    iface_axi_hw_t               *axi_hw,
    enum msm_vfe_axi_stream_src   axi_src,
    iface_hw_stream_t            **hw_stream_info)
{
  int stream_idx_in_array;

  if (axi_src == VFE_AXI_SRC_MAX || !axi_hw) {
    IFACE_ERR(" Error! Invalid arguments axi_src %d axi_hw %p",
      axi_src, axi_hw);
    return -1;
  }

  stream_idx_in_array = axi_hw->axi_handle_to_stream_idx[axi_src];
  if (stream_idx_in_array == -1) {
    IFACE_ERR(" Error! Invalid arguments axi_src %d axi_hw %p hw_stream_info %p",
      axi_src, axi_hw, hw_stream_info);
    return -1;
  }

  *hw_stream_info = &axi_hw->stream[stream_idx_in_array].hw_stream_info;

  return 0;
}

/** iface_axi_fill_framedrop_struct_from_axi_src
 *
 *  Description:
 *  this funciton is called to create the framedrop report due
 *  to reg update error
 *
 *    @axi_hw:          axi hw resource
 *    @error_mask:      reported error mask, represents AXI PATH
 *    @error_frame_id:  frame id the error mask is related to
 *    @frame_drop:  returned frame drop report
 *
 *  Returns 0 for success and negative error on failure
 **/
static int iface_axi_fill_framedrop_struct_from_axi_src(
    iface_axi_hw_t       *axi_hw,
    uint16_t              error_mask,
    uint32_t              error_frame_id,
    iface_axi_framedrop  *frame_drop)
{
  int rc = 0;
  int stream_idx_in_array;
  int i;
  iface_hw_stream_t *hw_stream_info = NULL;

  if (!error_mask)
    return rc;

  memset(frame_drop, 0, sizeof(*frame_drop));

  frame_drop->frame_id = error_frame_id;

  for (i = 0; i < VFE_AXI_SRC_MAX; i++) {
    if (!(error_mask & (1 << i)))
      continue;

    error_mask &= ~(1 << i);
    rc = iface_axi_map_hw_stream_from_axi_src(axi_hw, i, &hw_stream_info);
    if (rc < 0 || !hw_stream_info) {
      CDBG_HIGH("warning: Failed to report drop on axi_src %d frame_id %d", i,
        error_frame_id);
      continue;
    }

    frame_drop->framedrop.stream_request[
      frame_drop->framedrop.num_streams++].streamID =
        hw_stream_info->hw_stream_id;
    IFACE_DBG("hw_stream_id 0x%x frame_id %d", hw_stream_info->hw_stream_id,
      error_frame_id);

    if (!error_mask)
      break;
  }

  return rc;
}

/** iface_axi_fill_framedrop_struct_from_bufq_handle
 *
 *  Description:
 *  this funciton is called to create the framedrop report due
 *  to get buf error or return empty buffer due to delayed
 * frame request
 *
 *    @axi_hw:          axi hw resource
 *    @error_mask:      reported error mask, represent by bufq_id
 *    @error_frame_id:  frame id the error mask is related to
 *    @frame_drop:  returned frame drop report
 *
 *  Returns 0 for success and negative error on failure
 **/
static int iface_axi_fill_framedrop_struct_from_bufq_handle(
    iface_axi_hw_t       *axi_hw,
    uint32_t              error_mask,
    uint32_t              error_frame_id,
    iface_axi_framedrop  *frame_drop)
{
  int      rc = 0;
  uint32_t i  = 0;
  uint32_t bufq_idx = 0;
  uint32_t j  = 0;
  iface_hw_stream_t *hw_stream_info;
  uint32_t bufq_handle;

  if (!error_mask)
    return rc;

  memset(frame_drop, 0, sizeof(*frame_drop));

  /* based on the design, the frame drop report should be two frames delayed */
  frame_drop->frame_id = error_frame_id;

  for (j = 0; j < IFACE_MAX_NUM_BUF_QUEUE; j++) {
    bufq_handle = axi_hw->buf_mgr->bufq[j].kernel_bufq_handle;
    bufq_idx = bufq_handle & 0xFF;
    if (error_mask & (1 << bufq_idx)) {
      error_mask &= ~(1 << i); // Clear mask

      frame_drop->framedrop.stream_request[
        frame_drop->framedrop.num_streams++].streamID =
          axi_hw->buf_mgr->bufq[j].stream_id;
    }
    if (!error_mask)
      break;
  }

  if (error_mask) {
    for (i = 0; i < frame_drop->framedrop.num_streams; i++) {
    ATRACE_BEGIN_SNPRINTF(32, "Uncleared Error_frame:%d", error_frame_id);
    IFACE_ERR("Error! Uncleared Error mask 0x%x frame_id %d streamID =%x\n", error_mask,
      error_frame_id,frame_drop->framedrop.stream_request[i].streamID);
    ATRACE_END();
    }
    rc = -1;
  }
  return rc;
}

/** iface_axi_handle_sof_event:
 *
 *  Description:
 *  this funciton is called when SOF event is received from
 *  kernel
 *
 *    @isp_hw: isp hw
 *    @isp_event_data: isp event data
 *
 *  Returns 0 for success and negative error on failure
 **/
static int iface_axi_handle_sof_event(
    iface_axi_hw_t *axi_hw,
    struct msm_isp_event_data *sof,
    enum msm_vfe_input_src input_src)
{
  int rc = 0;

  iface_axi_sof_event sof_event;
  mct_bus_msg_isp_sof_t isp_sof;
  uint32_t ksofid = 0;
  iface_t *iface = (void*)axi_hw->notify_ops->parent;
  iface_session_t *session;
  iface_axi_hw_t *axi_hw_vfe1 = NULL;

  memset(&sof_event, 0, sizeof(sof_event));

  session = iface_util_get_session_by_id(iface, axi_hw->intf_param[input_src].session_id);
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n",
      __func__, axi_hw->intf_param[input_src].session_id);
    return -1;
  }

  /* Update the buffer drop mask due to out of buffer,
     The droped frame id is the current_sof frame id - 1 */
  rc = iface_axi_fill_framedrop_struct_from_bufq_handle(
      axi_hw,
      sof->u.sof_info.stream_get_buf_fail_mask,
      sof->frame_id - 1,
      &sof_event.get_buf_fail_drop);
  if (rc != 0) {
    CDBG("report frame drop !!!\n");
  }
  sof_event.get_buf_fail_drop.controllable_output = FALSE;

  /* TODO: handle regs not updated error. This error show that last
     * settings are not apply for current frame */
  if (sof->u.sof_info.regs_not_updated) {
    IFACE_HIGH("%s: regs_not_updated 0x%x for frame_id %d", __func__,
      sof->u.sof_info.regs_not_updated, sof->frame_id);
  }

  /* Report frame drop due to reg update error. The frame id
     will be the currect frame id */
  rc = iface_axi_fill_framedrop_struct_from_bufq_handle(
      axi_hw,
#ifdef REG_UPDATE_FAIL_MASK_EXT
      sof->u.sof_info.reg_update_fail_mask_ext,
#else
      sof->u.sof_info.reg_update_fail_mask,
#endif
      sof->frame_id,
      &sof_event.reg_update_fail_drop);
  if (rc != 0) {
    CDBG("report frame drop !!!\n");
  }
  sof_event.reg_update_fail_drop.controllable_output = TRUE;

#ifdef AXI_UPDATING_MASK
  /* Report frame drop due to CDS frame skip. The frame id
     will be the currect frame id */
  rc = iface_axi_fill_framedrop_struct_from_axi_src(
      axi_hw,
      sof->u.sof_info.axi_updating_mask,
      sof->frame_id,
      &sof_event.axi_updating_drop);
  if (rc != 0) {
    CDBG("failed to report frame drop !!!\n");
  }
  sof_event.axi_updating_drop.controllable_output = FALSE;
#endif

  sof_event.sof_buf_msg.type = MCT_BUS_MSG_ISP_SOF;
  sof_event.sof_buf_msg.sessionid = axi_hw->intf_param[input_src].session_id;
  sof_event.sof_buf_msg.size = sizeof(mct_bus_msg_isp_sof_t);
  sof_event.sof_buf_msg.msg = (void*) &isp_sof;

  isp_sof.frame_id = sof->frame_id;
  isp_sof.frame_src = (uint32_t)input_src;
  isp_sof.timestamp = sof->timestamp;
  isp_sof.mono_timestamp = sof->mono_timestamp;

  if (axi_hw->kstate_ptr.isp_kstate_ptr) {
   /* Set the dual cam drop as zero after wake */
   if ((session->ms_type != MS_TYPE_NONE ) && (session->lpm_enable == 0)) {
	 if (session->cur_lpm_mode != session->prev_lpm_mode) {
	  ((struct isp_kstate *)axi_hw->kstate_ptr.isp_kstate_ptr)->dual_cam_drop = 0;
	    IFACE_ERR("session %x VFE%d, src %d with frame_id %d cur_lpm %d pre_lpm %d",
	    axi_hw->intf_param[input_src].session_id, axi_hw->dev_idx, input_src,
	    sof->frame_id,
	    session->cur_lpm_mode,
	     session->prev_lpm_mode);
	 }
   }
    session->dual_cam_drop =
  	((struct isp_kstate *)axi_hw->kstate_ptr.isp_kstate_ptr)->dual_cam_drop;
  }

  if (axi_hw->kstate_ptr.isp_kstate_ptr) {

   if (!session->session_resource.ispif_split_info.is_split) {
     isp_sof.kstate_ptr = &axi_hw->kstate_ptr;
     ksofid = ((struct isp_kstate *)axi_hw->kstate_ptr.isp_kstate_ptr)->kernel_sofid;
   } else {
      if (session->dual_isp_sync) {
        axi_hw_vfe1 =
          (iface_axi_hw_t *)iface->isp_axi_data.axi_data[VFE0].axi_hw_ops->ctrl;
      } else {
        axi_hw_vfe1 =
          (iface_axi_hw_t *)iface->isp_axi_data.axi_data[VFE1].axi_hw_ops->ctrl;
      }
      if (axi_hw_vfe1->kstate_ptr.isp_kstate_ptr) {
         isp_sof.kstate_ptr = &axi_hw_vfe1->kstate_ptr;
         ksofid = ((struct isp_kstate *)axi_hw_vfe1->kstate_ptr.isp_kstate_ptr)->kernel_sofid;
      }
   }
  } else {
    isp_sof.kstate_ptr = NULL;
    ksofid = 0;
  }
  isp_sof.regs_not_updated = sof->u.sof_info.regs_not_updated;

  IFACE_ERR("Event SOF session %x VFE%d, src %d with frame_id %d ts: %lld",
       axi_hw->intf_param[input_src].session_id, axi_hw->dev_idx, input_src,
       sof->frame_id,
       (int64_t)sof->mono_timestamp.tv_sec * 1000000000 +
       (int64_t)sof->mono_timestamp.tv_usec * 1000);

  ATRACE_BEGIN_SNPRINTF(32, "Iface_axi_SOF_frame:%d", sof->frame_id);

  isp_sof.nano_timestamp = session->nano_timestamp;
  rc = axi_hw->notify_ops->notify(
      (void*)axi_hw->notify_ops->parent, axi_hw->dev_idx,
      axi_hw->intf_param[input_src].session_id, IFACE_AXI_NOTIFY_CAMIF_SOF,
      &sof_event, sizeof(sof_event));
  if (rc < 0) {
    CDBG_ERROR("%s: SOF to media bus error = %d\n", __func__, rc);
  }
  ATRACE_END();
  return rc;
}

#ifdef VIDIOC_MSM_ISP_NANOSEC_TIMESTAMP

/** iface_axi_handle_sof_event_update_nano_timestamp:
 *
 *  Description:
 *  this funciton is called when SOF event is received from
 *  kernel
 *
 *    @isp_hw: isp hw
 *    @isp_event_data: isp event data
 *
 *  Returns 0 for success and negative error on failure
 **/
static int iface_axi_handle_sof_update_nano_timestamp(
    iface_axi_hw_t *axi_hw,
    struct msm_isp_event_data_nanosec *sof,
    enum msm_vfe_input_src input_src)
{
  int rc = 0;
  iface_t *iface = (void*)axi_hw->notify_ops->parent;
  iface_session_t *session;

  session = iface_util_get_session_by_id(iface, axi_hw->intf_param[input_src].session_id);
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n",
      __func__, axi_hw->intf_param[input_src].session_id);
    return -1;
  }
  CDBG_HIGH("sof->frame_id %d session->prev_sof_frame_id %d",
            sof->frame_id, session->prev_sof_frame_id);
  if (sof->frame_id == session->prev_sof_frame_id + 1)
    session->nano_timestamp = sof->nano_timestamp;
  else {
    CDBG_ERROR("%s failed",__func__);
    session->nano_timestamp = 0;
  }
  return rc;
}
#endif

/** iface_axi_notify_buf_divert_event:
 *
 *    @isp_hw: isp hw
 *    @isp_event_data: isp event data
 *
 *  Returns 0 for success and negative error on failure
 **/
static int iface_axi_notify_buf_divert_event(iface_axi_hw_t *axi_hw,
  struct msm_isp_event_data *isp_event_data)
{
  int rc = 0;
  iface_frame_divert_notify_t event;
  iface_axi_buf_divert_ack_t ack;
  memset(&event,  0,  sizeof(iface_frame_divert_notify_t));
  event.isp_event_data = isp_event_data;

  rc = axi_hw->notify_ops->notify(
    (void*)axi_hw->notify_ops->parent, axi_hw->dev_idx,
    isp_event_data->u.buf_done.session_id,
    IFACE_AXI_NOTIFY_BUF_DIVERT, &event,
    sizeof(iface_frame_divert_notify_t));

  return rc;
}

/** iface_axi_handle_bus_overflow_error:
 *
 *    @axi_hw: axi handler
 *    @frame_id: frame id
 *
 *  Returns 0 for success and negative error on failure
 **/
static int iface_axi_handle_bus_overflow_error(iface_axi_hw_t *axi_hw,
  uint32_t frame_id, enum msm_vfe_input_src input_src)
{
  int rc;
  iface_halt_recovery_info_t halt_recovery_info;

  CDBG_HIGH("%s: overflow_dbg notify bus overflow!\n", __func__);

  halt_recovery_info.session_id = axi_hw->intf_param[input_src].session_id;
  halt_recovery_info.frame_id = frame_id;
  rc = axi_hw->notify_ops->notify((void*)axi_hw->notify_ops->parent,
    axi_hw->dev_idx, axi_hw->intf_param[input_src].session_id,
    IFACE_AXI_NOTIFY_BUS_OVERFLOW,
    &halt_recovery_info, sizeof(iface_halt_recovery_info_t));
  if (rc < 0) {
    CDBG_ERROR("%s: overflow recovery error = %d\n", __func__, rc);
    return rc;
  }

  return rc;
}

/** iface_axi_handle_output_error:
 *
 *    @axi_hw: axi handler
 *    @stream_id: stream id
 *    @frame_id: frame id
 *
 *  Returns 0 for success and negative error on failure
 **/
static int iface_axi_handle_output_error(iface_axi_hw_t *axi_hw,
  uint32_t err_mask, uint32_t frame_id, uint32_t session_id)
{
  int rc;
  mct_bus_msg_t bus_msg;
  iface_axi_framedrop iface_framedrop;

  CDBG_ERROR("%s: OUTPUT_ERROR session_id %d ", __func__,
    session_id);

  memset(&iface_framedrop, 0 , sizeof(iface_axi_framedrop));

  /* frame request failed. The frame id refer in the failure is the
     requested frame ID */
  rc = iface_axi_fill_framedrop_struct_from_bufq_handle(
      axi_hw,
      err_mask,
      frame_id,
      &iface_framedrop);
  if (rc != 0) {
    CDBG("report frame drop !!!\n");
  }
  iface_framedrop.controllable_output = TRUE;

  rc = axi_hw->notify_ops->notify((void*)axi_hw->notify_ops->parent,
    axi_hw->dev_idx, session_id, IFACE_AXI_NOTIFY_OUTPUT_ERROR,
    &iface_framedrop, sizeof(iface_axi_framedrop));
  if (rc < 0) {
    CDBG_ERROR("%s: OUTPUT_ERROR to media bus error = %d\n", __func__, rc);
    return rc;
  }

  return rc;
}

/** iface_axi_util_proc_subdev_event
 *    @isp_hw:
 *    @thread_data:
 *
 **/
void iface_axi_proc_subdev_event(
  iface_axi_hw_t *axi_hw,
  iface_thread_t *thread_data __unused)
{
  int rc;
  int i = 0;
  struct v4l2_event v4l2_event;
  struct msm_isp_event_data *iface_isp_subdev_event_data = NULL;
  enum msm_vfe_input_src input_src = 0;
  char str[80];
  uint32_t axi_event = 0;

  memset(&v4l2_event, 0, sizeof(v4l2_event));
  rc = ioctl(axi_hw->fd, VIDIOC_DQEVENT, &v4l2_event);

  if (rc >= 0) {
    iface_isp_subdev_event_data = (struct msm_isp_event_data *)v4l2_event.u.data;
    ATRACE_BEGIN_SNPRINTF(64, "IFACE_PROC_EVENT_STREAM_UPDATE_DONE:%d", iface_isp_subdev_event_data->frame_id);

    if ((v4l2_event.type & 0xFFFFFF00) >= ISP_EVENT_BUF_DIVERT) {
      input_src = v4l2_event.type & 0xFF;
      v4l2_event.type &= ~input_src;
      input_src &= 0x3;
    }

    switch(v4l2_event.type) {
    case ISP_EVENT_STATS_NOTIFY:{
      int buf_idx = 0;
      void *raw_buf = NULL;
      struct msm_isp_event_data *buf_event = NULL;
      struct msm_isp_stats_event *stats_buf_event = NULL;
      iface_raw_stats_buf_info_t stats_notify_info;
      iface_frame_buffer_t *frame_buf = NULL;
      iface_session_t *session = NULL;
      uint32_t         buffs_id = 0;
      iface_hw_stream_t *hw_stream = NULL;

      memset(&stats_notify_info, 0, sizeof(iface_raw_stats_buf_info_t));

      buf_event = (struct msm_isp_event_data *)v4l2_event.u.data;
      stats_buf_event = &buf_event->u.stats;

      IFACE_DBG("stats notify! stats event mask = %x\n",
        stats_buf_event->stats_mask);

      /*fill in stats notify data*/
      stats_notify_info.stats_mask = stats_buf_event->stats_mask;
      stats_notify_info.session_id = axi_hw->intf_param[VFE_PIX_0].session_id;
      stats_notify_info.frame_id = buf_event->frame_id;
      stats_notify_info.timestamp = buf_event->timestamp;
      stats_notify_info.ack_flag = FALSE;
      /* mono_timestamp is used as sof_timestamp in stats event */
      stats_notify_info.sof_timestamp = buf_event->mono_timestamp;
      memcpy(&stats_notify_info.stats_buf_idxs[0],
        &stats_buf_event->stats_buf_idxs[0],
        MSM_ISP_STATS_MAX * sizeof(uint8_t));
      /*get stats buffer virtual addr and attatch to event*/
      for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
        if (stats_notify_info.stats_mask & (1 << i)) {
          buf_idx = stats_buf_event->stats_buf_idxs[i];
          raw_buf = iface_get_buf_vaddr(axi_hw->buf_mgr,
            axi_hw->stats_stream[i].bufq_handle, buf_idx);
          stats_notify_info.raw_stats_buffer[i] = raw_buf;
          stats_notify_info.raw_stats_buf_len[i] =
            axi_hw->stats_stream[i].stats_stream_info.buf_len;
        }
      }

      IFACE_DBG("stats_dbg stats notify! stats event mask = %x vfe_id %d \
        session_id: 0x%x\n",
        stats_buf_event->stats_mask, axi_hw->dev_idx,
        axi_hw->intf_param[VFE_PIX_0].session_id);
      if (stats_notify_info.stats_mask & (1 << MSM_ISP_STATS_BF) &&
                         stats_buf_event->pd_stats_idx != 0xF) {
        session = iface_util_get_session_by_id((iface_t*)axi_hw->notify_ops->parent,
                    axi_hw->intf_param[VFE_PIX_0].session_id);
           if (!session) {
              IFACE_ERR("cannot find session for session id %d",
                axi_hw->intf_param[VFE_PIX_0].session_id);
              break;
           }
        stats_notify_info.pd_data.buf_idx = stats_buf_event->pd_stats_idx;
        for (i = 0; i < IFACE_MAX_STREAMS; i++) {
          if (session->hw_streams[i].ext_stats_type == PD_STATS) {
            hw_stream = &session->hw_streams[i];
            break;
          }
        }
        if (hw_stream) {
          buffs_id = iface_util_get_buffers_id(hw_stream,
            hw_stream->hw_stream_id);
          if (buffs_id < VFE_BUF_QUEUE_MAX) {
            frame_buf = iface_get_buf_by_idx(
              axi_hw->buf_mgr,
              hw_stream->buffers_info[buffs_id].bufq_handle,
              stats_buf_event->pd_stats_idx);
            if (!frame_buf)
              break;
            stats_notify_info.pd_data.max_buffers =
                iface_get_current_num_buffer(axi_hw->buf_mgr,
                  hw_stream->buffers_info[buffs_id].bufq_handle);
              stats_notify_info.pd_data.pd_buffer_data = (void*)
              frame_buf->vaddr;
            IFACE_DBG("PD STATS buf idx %d max buffers %d frame id %d\n",
              stats_buf_event->pd_stats_idx, stats_notify_info.pd_data.max_buffers,
              session->sof_frame_id);
          }
        }
      }
      IFACE_DBG("stats_dbg stats notify! stats event mask = %x vfe_id %d\n",
        stats_buf_event->stats_mask, axi_hw->dev_idx);
      /* notify ptr is isp_core_hw_notify */
      rc = axi_hw->notify_ops->notify(
        (void*)axi_hw->notify_ops->parent, axi_hw->dev_idx,
        axi_hw->intf_param[VFE_PIX_0].session_id,
        IFACE_AXI_NOTIFY_STATS, (void *)&stats_notify_info,
        sizeof(iface_raw_stats_buf_info_t));

      if (rc < 0 || stats_notify_info.ack_flag == TRUE) {
        IFACE_HIGH("enforce queue back stats buf!"
          "rc = %d, piggy back flag %d\n",
          rc, stats_notify_info.ack_flag);

        for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
          if (stats_notify_info.stats_mask & (1 << i)) {
            IFACE_DBG("queue back stats buffer, i = %d, buf idx = %d\n",
              i, stats_notify_info.stats_buf_idxs[i]);
            rc = iface_queue_buf(axi_hw->buf_mgr,
              axi_hw->stats_stream[i].bufq_handle,
              stats_notify_info.stats_buf_idxs[i], 1 , axi_hw->fd,
              0);
            if (rc < 0)
              IFACE_ERR("iface_queue_buf error, i = %d, rc= %d\n",
                i, rc);
          }
        }
        return;
      }
    }
      break;

    case ISP_EVENT_ERROR: {
      struct msm_isp_event_data *error_data =
        (struct msm_isp_event_data *)v4l2_event.u.data;
      struct msm_isp_error_info *error_info = &error_data->u.error_info;

      switch (error_info->err_type) {
      case ISP_ERROR_FRAME_ID_MISMATCH:
        IFACE_ERR("Frame ID mismatch. Trigger recovery\n");
        /* use same recover as bus overflow */
      case ISP_ERROR_BUS_OVERFLOW:
        rc = iface_axi_handle_bus_overflow_error(axi_hw, error_data->frame_id,
          input_src);
        if (rc < 0) {
          IFACE_ERR("failed: iface_axi_handle_bus_overflow\n");
        }
        break;

      case ISP_ERROR_RETURN_EMPTY_BUFFER:
        IFACE_DBG("ISP_ERROR_RETURN_EMPTY_BUFFER. frame_id %d stream_id %x\n",
          error_data->frame_id, error_info->stream_id_mask);
        rc = iface_axi_handle_output_error(axi_hw, error_info->stream_id_mask,
          error_data->frame_id, axi_hw->intf_param[input_src].session_id);
        if (rc < 0) {
          IFACE_ERR("failed: iface_axi_handle_output_error\n");
        }
        break;

      default:
        IFACE_ERR("error type %d not handled\n",
          error_info->err_type);
        break;
      }
      MCT_PROF_LOG_END();
    }
      break;

    case ISP_EVENT_IOMMU_P_FAULT:
      strlcpy(str, "IOMMU page fault", 80);
      axi_event = IFACE_AXI_NOTIFY_PAGE_FAULT;
    case ISP_EVENT_REG_UPDATE_MISSING:
      if (!axi_event) {
        strlcpy(str, "Reg update missing", 80);
        axi_event = IFACE_AXI_NOTIFY_REG_UPDATE_MISSING;
      }
    case ISP_EVENT_PING_PONG_MISMATCH:
      if (!axi_event) {
        strlcpy(str, "Ping pong mismatch", 80);
        axi_event = IFACE_AXI_NOTIFY_PING_PONG_MISMATCH;
      }
    case ISP_EVENT_BUF_FATAL_ERROR:
      if (!axi_event) {
        strlcpy(str, "Fatal buffer error", 80);
        axi_event = IFACE_AXI_NOTIFY_BUF_FATAL_ERROR;
      }
    {
      struct msm_isp_event_data *error_event = (struct msm_isp_event_data *)v4l2_event.u.data;
      mct_bus_msg_t bus_msg;

      IFACE_ERR("%s occured at Frame Id %d\n", str, error_event->frame_id);
      memset(&bus_msg, 0, sizeof(bus_msg));

      bus_msg.sessionid = axi_hw->intf_param[input_src].session_id;
      bus_msg.type = MCT_BUS_MSG_SEND_HW_ERROR;

      rc = axi_hw->notify_ops->notify(
        (void*)axi_hw->notify_ops->parent, axi_hw->dev_idx,
        axi_hw->intf_param[input_src].session_id,
        axi_event, &bus_msg,
        sizeof(mct_bus_msg_t));
      if (rc < 0) {
        IFACE_ERR("overflow recovery error = %d\n",rc);
        return;
      }
    }
      break;

    case ISP_EVENT_STREAM_UPDATE_DONE: {
      int rc;
      struct msm_isp_event_data *event =
         (struct msm_isp_event_data *)v4l2_event.u.data;
      rc = axi_hw->notify_ops->notify(
        (void*)axi_hw->notify_ops->parent, axi_hw->dev_idx,
        axi_hw->intf_param[input_src].session_id,
        IFACE_AXI_NOTIFY_STREAM_UPDATE_DONE, &event->frame_id,
        sizeof(event->frame_id));
      if (rc < 0) {
        IFACE_ERR("stream update done error = %d\n", rc);
        return;
      }
    }
      break;

    case ISP_EVENT_REG_UPDATE: {
      int rc;
      mct_event_t mct_event;
      struct msm_isp_event_data *reg_update =
         (struct msm_isp_event_data *)v4l2_event.u.data;
      ATRACE_BEGIN_SNPRINTF(64, "IFACE_PROC_EVENT_REG_UPDATE:%d", reg_update->frame_id);
      if (axi_hw->num_active_streams == 0) {
        IFACE_ERR("no hw stream , skip sending REG_UPDATE!\n");
        break;
      }
      IFACE_DBG("REG_UPDATE received frame_id %u \n",
        reg_update->frame_id);
      memset(&mct_event, 0, sizeof(mct_event));
      mct_event.type = MCT_EVENT_MODULE_EVENT;
      mct_event.direction = MCT_EVENT_DOWNSTREAM;
      mct_event.u.module_event.type = MCT_EVENT_MODULE_REG_UPDATE_NOTIFY;
      mct_event.u.module_event.module_event_data =
        (void *)&reg_update->frame_id;
      mct_event.u.module_event.current_frame_id = reg_update->frame_id;

      /* notify ptr is isp_core_hw_notify */
      rc = axi_hw->notify_ops->notify(
        (void*)axi_hw->notify_ops->parent, axi_hw->dev_idx,
        axi_hw->intf_param[input_src].session_id,
        IFACE_AXI_NOTIFY_CAMIF_REG_UPDATE, &mct_event, sizeof(mct_event_t));
      if (rc < 0) {
        IFACE_ERR("REG_UPDATE to media bus error = %d\n", rc);
        return;
      }
      ATRACE_END();
    }
      break;

    case ISP_EVENT_SOF:{
      switch (input_src) {
      case VFE_PIX_0:
      case VFE_RAW_0:
      case VFE_RAW_1:
      case VFE_RAW_2: {
        struct msm_isp_event_data *sof =
          (struct msm_isp_event_data *)v4l2_event.u.data;
        ATRACE_BEGIN_SNPRINTF(32, "IFACE_PROC_EVENT_SOF:%d", sof->frame_id);

        if (axi_hw->num_active_streams == 0) {
          IFACE_ERR("no hw stream , skip sending SOF!\n");
          break;
        }
        rc = iface_axi_handle_sof_event(axi_hw, sof, input_src);
        if (rc != 0) {
          IFACE_ERR("Error iface_axi_handle_sof_event\n");
        }
        ATRACE_END();
      }
      break;
      default:
        IFACE_ERR("Error! Invalid frame_src on SOF event\n");
        break;
      }
    }
    break;
 #ifdef VIDIOC_MSM_ISP_NANOSEC_TIMESTAMP
    case ISP_EVENT_SOF_UPDATE_NANOSEC:{
      struct msm_isp_event_data_nanosec *sof =
        (struct msm_isp_event_data_nanosec *)(v4l2_event.u.data);

      if (axi_hw->num_active_streams == 0) {
        IFACE_ERR("no hw stream , skip sending SOF!\n");
        break;
      }
      IFACE_DBG("frame_id %d nano_timestamp %lld",
          sof->frame_id, sof->nano_timestamp);

      rc = iface_axi_handle_sof_update_nano_timestamp(axi_hw, sof, input_src);
      if (rc != 0) {
        IFACE_ERR("Error iface_axi_handle_sof_event\n");
      }
    }
    break;
 #endif
    case ISP_EVENT_BUF_DIVERT: {
      ATRACE_BEGIN_SNPRINTF(64, "IFACE_PROC_EVENT_BUF_DIVERT:%d", iface_isp_subdev_event_data->frame_id);
      IFACE_DBG("buf_dbg  session id %d buf_divert frame %d stream_id %x \n",
           iface_isp_subdev_event_data->u.buf_done.session_id,
           iface_isp_subdev_event_data->frame_id,
           iface_isp_subdev_event_data->u.buf_done.stream_id);
      rc = iface_axi_notify_buf_divert_event(axi_hw, iface_isp_subdev_event_data);
      if (rc != 0) {
        IFACE_ERR("Failed iface_axi_notify_buf_divert_event\n");
      }
      ATRACE_END();
    }
      break;

    case ISP_EVENT_FE_READ_DONE:{
      uint32_t i =0;
      int rc;
      iface_bufq_t *bufq = NULL;
      mct_bus_msg_t bus_msg;
      mct_bus_msg_isp_rd_done_t  rd_done_event;
      struct msm_isp_event_data *fe_read_done =
        (struct msm_isp_event_data *)v4l2_event.u.data;
      bus_msg.sessionid = axi_hw->intf_param[input_src].session_id;
      bus_msg.type = MCT_BUS_MSG_ISP_RD_DONE;
      bus_msg.metadata_collection_type = MCT_BUS_ONLINE_METADATA;
      bus_msg.msg = (void *)fe_read_done;
      bus_msg.size = sizeof(struct msm_isp_event_data);
      rd_done_event.timestamp = fe_read_done->timestamp;
      iface_axi_stream_t *stream = NULL;
      IFACE_HIGH("VFE%d  ISP_EVENT_FE_READ_DONE buf_idx %d handle %x",
        axi_hw->init_params.dev_idx,
        fe_read_done->u.buf_done.buf_idx, fe_read_done->u.buf_done.handle);

      for (i = 0; i < IFACE_MAX_NUM_BUF_QUEUE; i++) {
        bufq = &axi_hw->buf_mgr->bufq[i];
        if (fe_read_done->u.buf_done.handle == bufq->kernel_bufq_handle) {
        break;
       }
      }
      rc = axi_hw->notify_ops->notify(
        (void*)axi_hw->notify_ops->parent, axi_hw->dev_idx,
        axi_hw->intf_param[input_src].session_id,
        IFACE_AXI_NOTIFY_FE_READ_DONE, &bus_msg,
        sizeof(mct_bus_msg_t));
      if (rc < 0) {
        IFACE_ERR("fe read done error = %d\n", rc);
        return;
      }
      /* Unmap offline mode buffer after read done */
      if (fe_read_done->u.fetch_done.offline_mode) {
        rc = axi_hw->notify_ops->notify(
          (void*)axi_hw->notify_ops->parent, axi_hw->dev_idx,
          fe_read_done->u.fetch_done.session_id,
          IFACE_AXI_NOTIFY_UNMAP_BUF, &fe_read_done->u.fetch_done.fd,
          sizeof(uint32_t));
        if (rc < 0) {
          CDBG_ERROR("%s: unmap buf error = %d\n", __func__, rc);
          return;
        }
      }
    }
      break;

    default: {
    }
      break;
    }
    ATRACE_END();
  } else {
    IFACE_HIGH("VIDIOC_DQEVENT type failed\n");
  }

  return;
}

/** iface_axi_sem_thread_execute
 *
 *    @ctrl:
 *    @thread:
 *    @action:
 *
 **/
static int iface_axi_sem_thread_execute(
  void           *ctrl __unused,
  iface_thread_t *thread,
  void           *cmd,
  uint32_t        cmd_id)
{
  int rc = 0;

  pthread_mutex_lock(&thread->cmd_mutex);

  thread->cmd_id = cmd_id;
  switch (cmd_id) {
  case IFACE_THREAD_CMD_ACTION:
    thread->action_cmd = cmd;
    break;
  case IFACE_THREAD_CMD_SET_PARAMS:
    thread->set_param_cmd = cmd;
    break;
  case IFACE_THREAD_CMD_GET_PARAMS:
    thread->get_param_cmd = cmd;
    break;
  case IFACE_THREAD_CMD_NOTIFY_OPS_INIT:
    thread->init_cmd = cmd;
    break;
  default:
    CDBG_ERROR("%s: not support this thread cmd!, cmd id = %d\n", __func__, cmd_id);
    break;
  }

  /* wake up sem thread in sem_main_looop*/
  sem_post(&thread->thread_wait_sem);

  /* wait thill sem thread to wake up
     only stream start/stop are non-blocking call,
     all others are blocking call*/
  sem_wait(&thread->sig_sem);

  rc = thread->return_code;

  pthread_mutex_unlock(&thread->cmd_mutex);

  return rc;
}

/** iface_axi_pipe_thread_execute
 *
 *    @ctrl:
 *    @thread:
 *    @action:
 *
 **/
static int iface_axi_pipe_thread_execute(
  void           *ctrl __unused,
  iface_thread_t *thread,
  void           *cmd,
  uint32_t        cmd_id)
{
  int len, rc = 0;

  pthread_mutex_lock(&thread->cmd_mutex);

  thread->action_cmd = cmd;
  thread->cmd_id = cmd_id;

  len = write(thread->pipe_fds[1], &cmd_id, sizeof(cmd_id));
  if (len != sizeof(cmd_id)) {
    pthread_mutex_unlock(&thread->cmd_mutex);
    CDBG_ERROR("%s: write to pipe error\n", __func__);
    return -EPIPE;
  }

  sem_wait(&thread->sig_sem);
  rc = thread->return_code;

  pthread_mutex_unlock(&thread->cmd_mutex);

  return rc;
}

/** iface_axi_destroy_thread
 *
 *    @thread: information about the thread to be destroyed
 *
 *  Destroys isp hw thread
 **/
static int iface_axi_destroy_thread(iface_thread_t *thread)
{
  int len;
  uint32_t cmd = IFACE_THREAD_CMD_DESTROY;
  iface_axi_hw_t *axi_hw;

  axi_hw = (iface_axi_hw_t *)thread->hw_ptr;

  pthread_mutex_lock(&thread->cmd_mutex);

  if (axi_hw->kstate_ptr.isp_kstate_ptr)
    MCT_REFCOUNT_PUT(&axi_hw->kstate_ptr, refc);

  if (thread->return_code == -EPIPE) {
    CDBG_ERROR("%s: Pipe read error\n", __func__);
    pthread_mutex_unlock(&thread->cmd_mutex);
    return -EPIPE;
  }

  len = write(thread->pipe_fds[1], &cmd, sizeof(cmd));
  if (len != sizeof(cmd)) {
    /* we got error here */
    pthread_mutex_unlock(&thread->cmd_mutex);
    CDBG_ERROR("%s: Pipe write error\n", __func__);
    return -EPIPE;
  }

  /* we do not wait on sem but join to wait the thread to die. */
  if (pthread_join(thread->pid, NULL) != 0)
    CDBG("%s: pthread dead already\n", __func__);

  sem_destroy(&thread->sig_sem);

  pthread_mutex_destroy(&thread->busy_mutex);
  pthread_mutex_unlock(&thread->cmd_mutex);
  pthread_mutex_destroy(&thread->cmd_mutex);

  return 0;
}

/** iface_axi_destroy
 *    @ctrl:
 *
 **/
int iface_axi_destroy(void *ctrl)
{
  int rc = 0;
  iface_axi_hw_t *axi_hw = NULL;

  if (!ctrl) {
    CDBG_ERROR("%s: NULL pointer ctrl = %p, rc = 0\n", __func__, ctrl);
    return 0;
  }
  axi_hw = (iface_axi_hw_t *)ctrl;

  if (axi_hw->fd > 0) {
    close(axi_hw->fd);
    axi_hw->fd = 0;
  }

  free(axi_hw);

  return rc;
}

/** iface_axi_destroy_to_thread
 *
 *    @ctrl:
 *
 *  Destroys isp hw
 **/
static int iface_axi_destroy_to_thread(void *ctrl)
{
  int i, len, rc = 0;
  iface_axi_hw_t *axi_hw = ctrl;

  if (axi_hw == NULL) {
    CDBG_ERROR("%s: axi hw is already released!\n", __func__);
    return 0;
  }

  rc = iface_axi_destroy_thread(&axi_hw->thread_poll);
  if(rc < 0)
    goto EXIT;

  rc = iface_sem_thread_stop(&axi_hw->thread_stream);
  if(rc < 0)
    goto EXIT;

EXIT:
  iface_axi_destroy(ctrl);

  return rc;
}

/** iface_axi_set_params:
 *
 *    @ctrl:
 *    @params_id:
 *    @in_params_size:
 *
 **/
int iface_axi_set_params(void *ctrl, uint32_t params_id, void *in_params,
  uint32_t in_params_size)
{
  iface_axi_hw_t *axi = ctrl;
  int rc = 0;

  switch (params_id) {
  case IFACE_AXI_SET_STREAM_CFG: {
    rc = iface_axi_stream_config(axi,
      (iface_hw_stream_set_params_t *)in_params, in_params_size);
  }
    break;

  case IFACE_AXI_SET_STREAM_UNCFG: {
    rc = iface_axi_stream_unconfig(axi,
           (iface_hw_stream_t *)in_params, in_params_size);
  }
    break;
  case IFACE_AXI_SET_STATS_STREAM_CFG: {
    rc = iface_axi_stats_stream_config(axi,
      (iface_stats_stream_set_params_t *)in_params, in_params_size);
  }
    break;

  case IFACE_AXI_SET_STATS_STREAM_UNCFG: {
    rc = iface_axi_stats_stream_unconfig(axi);
  }
    break;

  case IFACE_AXI_SET_PARAM_FRAME_SKIP: {
    rc = iface_axi_stream_set_skip_pattern(axi,
           (iface_param_frame_skip_pattern_t *)in_params, in_params_size);
  }
    break;

  case IFACE_AXI_SET_PARAM_FRAME_SKIP_ALL_STREAM: {
    rc = iface_axi_stream_set_skip_pattern_all_stream(axi,
           (iface_param_frame_skip_pattern_t *)in_params, in_params_size);
  }
    break;

  case IFACE_AXI_SET_REQUEST_FRAME: {
    rc = iface_axi_stream_request_frame(axi,
           (iface_param_frame_request_t *)in_params, in_params_size);
  }
    break;

  case IFACE_AXI_SET_PARAM_STATS_SKIP: {
    rc = iface_axi_stream_set_stats_skip_pattern(axi,
           (iface_param_frame_skip_pattern_t *)in_params, in_params_size);
  }
    break;

  case IFACE_AXI_SET_STREAM_UPDATE: {
    rc = iface_axi_stream_set_stream_update(axi,
      (iface_stream_update_t *)in_params, in_params_size);
  }
    break;

  case IFACE_AXI_SET_OFFLINE_STREAM_UPDATE: {
    rc = iface_axi_stream_set_offline_stream_update(axi,
      (iface_stream_update_t *)in_params, in_params_size);
  }
    break;

  default: {
  }
    break;
  }

  return rc;
}

/** iface_axi_set_params_to_thread
 *
 *    @ctrl:
 *    @params_id:
 *    @in_params:
 *    @in_params_size:
 *
 *
 **/
static int iface_axi_set_params_to_thread(void *ctrl, uint32_t params_id, void *in_params,
  uint32_t in_params_size)
{
  int len, rc = 0;
  iface_axi_hw_t *axi_hw = ctrl;
  iface_pipe_set_params_t set_params_cmd;
  uint32_t cmd_id = IFACE_THREAD_CMD_SET_PARAMS;
  iface_thread_t *thread;

  set_params_cmd.in_params = in_params;
  set_params_cmd.in_params_size = in_params_size;
  set_params_cmd.params_id = params_id;

  /* use stream thread to do stream on*/
  thread = &axi_hw->thread_stream;
  rc = iface_axi_sem_thread_execute(ctrl, thread, &set_params_cmd, cmd_id);

  return rc;
}


/** iface_axi_action:
 *
 *    @axi:
 *
 **/
int iface_axi_action(void *ctrl, uint32_t action_code, void *action_data,
  uint32_t action_data_size, int previous_ret_code)
{
  iface_axi_hw_t *axi_hw = ctrl;
  int rc = 0;

  switch (action_code) {
  case IFACE_AXI_ACTION_CODE_REG_INPUTBUF: {
    rc = iface_axi_reg_input_buf(axi_hw, action_data);
    if (rc < 0) {
      CDBG_ERROR("%s: iface_axi_reg_buf error = %d\n", __func__, rc);
    }
  }
    break;

  case IFACE_AXI_ACTION_CODE_UNREG_INPUTBUF: {
    rc = iface_axi_unreg_input_buf(axi_hw, action_data);
    if (rc < 0) {
      CDBG_ERROR("%s: iface_axi_unreg_buf error = %d\n", __func__, rc);
    }
  }
    break;

  case IFACE_AXI_ACTION_CODE_STREAM_START: {
    rc = iface_axi_start_stream(axi_hw, action_data, action_data_size);
  }
    break;

  case IFACE_AXI_ACTION_CODE_STREAM_STOP: {
    rc = iface_axi_stop_stream(axi_hw, action_data, action_data_size);
  }
    break;

  case IFACE_AXI_ACTION_CODE_STREAM_START_ACK: {
    rc = previous_ret_code;
  }
    break;

  case IFACE_AXI_ACTION_CODE_STREAM_STOP_ACK: {
    rc = previous_ret_code;
  }
    break;

  case IFACE_AXI_ACTION_CODE_STREAM_DIVERT_ACK: {
    rc = iface_axi_divert_ack(axi_hw, action_data, action_data_size);
  }
    break;

  case IFACE_AXI_ACTION_CODE_STATS_NOTIFY_ACK: {
    rc = iface_axi_stats_notify_ack(axi_hw, action_data, action_data_size);
  }
    break;

  case IFACE_AXI_ACTION_CODE_HW_UPDATE: {
    rc = iface_axi_do_hw_update(axi_hw);
  }
    break;

  case IFACE_AXI_ACTION_CODE_HALT_HW: {
    rc = iface_axi_halt_hw(axi_hw);
  }
    break;

  case IFACE_AXI_ACTION_CODE_RESET_HW: {
    rc = iface_axi_reset_hw(axi_hw, action_data, action_data_size);
  }
    break;

  case IFACE_AXI_ACTION_CODE_RESTART_HW: {
    rc = iface_axi_restart_hw(axi_hw);
  }
    break;

  case IFACE_AXI_ACTION_CODE_REG_BUF: {
    rc = iface_axi_reg_shared_buf(axi_hw, action_data, action_data_size);
  }
    break;

  case IFACE_AXI_ACTION_CODE_UNREG_BUF: {
    rc = iface_axi_unreg_shared_buf(axi_hw, action_data, action_data_size);
  }
    break;

  case IFACE_AXI_ACTION_CODE_ADD_BUFQ: {
    rc = iface_axi_stream_add_bufq(axi_hw,
           (iface_param_add_queue_t *)action_data, action_data_size);
  }
    break;

  case IFACE_AXI_ACTION_CODE_REMOVE_BUFQ: {
    rc = iface_axi_stream_remove_bufq(axi_hw,
           (iface_param_add_queue_t *)action_data, action_data_size);
  }
    break;

  case IFACE_AXI_ACTION_CODE_FETCH_START: {
    rc = iface_axi_start_fetch(axi_hw, action_data, action_data_size);
  }
    break;

  case IFACE_AXI_ACTION_CODE_UNMAP_BUF: {
    rc = iface_axi_unmap_buf(axi_hw, action_data);
  }
    break;

  default: {
  }
    break;
  }
  return rc;
}

/** iface_axi_action_to_thread
 *
 *    @ctrl:
 *    @action_code:
 *    @action_data:
 *    @action_data_size:
 *
 **/
static int iface_axi_action_to_thread(void *ctrl, uint32_t action_code, void *action_data,
  uint32_t action_data_size)
{
  int len, rc = 0;
  iface_axi_hw_t *axi_hw = ctrl;
  iface_pipe_action_t action_cmd;
  iface_thread_t *thread;

  uint32_t cmd_id = IFACE_THREAD_CMD_ACTION;

  switch(action_code) {
  case IFACE_AXI_ACTION_CODE_REG_INPUTBUF :
  case  IFACE_AXI_ACTION_CODE_STREAM_START: {
    if (sizeof(start_stop_stream_t) != action_data_size) {
      CDBG_ERROR("%s: error, action_code = %d, data size mismatch\n",
        __func__, action_code);
      return -1;
    }

    /* action code = IFACE_AXI_ACTION_CODE_STREAM_START,
       action data = start_stop_stream_t*/
    action_cmd.action_code = action_code;
    action_cmd.data = action_data;
    action_cmd.data_size = action_data_size;

    /* use stream thread to do stream on*/
    thread = &axi_hw->thread_stream;
    rc = iface_axi_sem_thread_execute(ctrl, thread, (void *)&action_cmd, cmd_id);

  }
    break;
  case IFACE_AXI_ACTION_CODE_UNREG_INPUTBUF:
  case  IFACE_AXI_ACTION_CODE_STREAM_STOP: {
    /* stream start/stop passing start/stop parm to thread*/
    if (sizeof(start_stop_stream_t) != action_data_size) {
      CDBG_ERROR("%s: error, action_code = %d, data size mismatch\n",
        __func__, action_code);
      return -1;
    }
    /* action code = IFACE_AXI_ACTION_CODE_STREAM_STOP,
       action data = start_stop_stream_t*/
    action_cmd.action_code = action_code;
    action_cmd.data = action_data;
    action_cmd.data_size = action_data_size;

    /* use stream thread to do stream off*/
    thread = &axi_hw->thread_stream;
    rc = iface_axi_sem_thread_execute(ctrl, thread, (void *)&action_cmd, cmd_id);
  }
    break;

  case IFACE_AXI_ACTION_CODE_STREAM_START_ACK:
  case IFACE_AXI_ACTION_CODE_STREAM_STOP_ACK: {
    if (sizeof(start_stop_stream_t) != action_data_size) {
      CDBG_ERROR("%s: error, action_code = %d, data size mismatch\n",
        __func__, action_code);
      return -1;
    }
    /* action code = IFACE_AXI_ACTION_CODE_STREAM_START_ACK / STOP_ACK,
       action data = start_stop_stream_t*/
    action_cmd.action_code = action_code;
    action_cmd.data = action_data;
    action_cmd.data_size = action_data_size;

    /* use stream thread to do start/stop ACK*/
    thread = &axi_hw->thread_stream;
    rc = iface_axi_sem_thread_execute(ctrl, thread, (void *)&action_cmd, cmd_id);
  }
    break;

  case  IFACE_AXI_ACTION_CODE_HW_UPDATE: {
    action_cmd.action_code = action_code;
    action_cmd.data = action_data;
    action_cmd.data_size = action_data_size;

    thread = &axi_hw->thread_stream;
    rc = iface_axi_sem_thread_execute(ctrl, thread, (void *)&action_cmd, cmd_id);

  }
    break;

  case  IFACE_AXI_ACTION_CODE_HALT_HW:
  case  IFACE_AXI_ACTION_CODE_RESET_HW:
  case  IFACE_AXI_ACTION_CODE_RESTART_HW:
  default: {
    action_cmd.action_code = action_code;
    action_cmd.data = action_data;
    action_cmd.data_size = action_data_size;

    thread = &axi_hw->thread_stream;
    rc = iface_axi_sem_thread_execute(ctrl, thread, (void *)&action_cmd, cmd_id);
  }
    break;
  }

  return rc;
}


/** iface_axi_get_params
 *
 *    @ctrl: isp root
 *    @params_id:
 *    @in_params:
 *    @in_params_size:
 *    @out_params:
 *    @out_params_size:
 *
 **/
int iface_axi_get_params(
  void     *ctrl,
  uint32_t params_id __unused,
  void     *in_params __unused,
  uint32_t in_params_size __unused,
  void     *out_params __unused,
  uint32_t  out_params_size __unused)
{
  int rc = 0;
  iface_axi_hw_t *axi_hw = ctrl;

  return rc;
}

/** iface_axi_get_params_to_thread
 *
 *    @ctrl:
 *    @params_id:
 *    @in_params:
 *    @in_params_size:
 *    @out_params:
 *    @out_params_size:
 *
 *
 **/
static int iface_axi_get_params_to_thread(void *ctrl, uint32_t params_id, void *in_params,
  uint32_t in_params_size, void *out_params, uint32_t out_params_size)
{
  int len, rc = 0;
  iface_axi_hw_t *axi_hw = ctrl;
  iface_pipe_get_params_t get_params_cmd;
  uint32_t cmd_id = IFACE_THREAD_CMD_GET_PARAMS;
  iface_thread_t *thread;

  get_params_cmd.in_params = in_params;
  get_params_cmd.in_params_size = in_params_size;
  get_params_cmd.params_id = params_id;
  get_params_cmd.out_params = out_params;
  get_params_cmd.out_params_size = out_params_size;

  /* use stream thread to do stream on*/
  thread = &axi_hw->thread_stream;
  rc = iface_axi_sem_thread_execute(ctrl, thread, &get_params_cmd, cmd_id);

  return rc;
}


/** iface_axi_init
 *    @ctrl:
 *    @in_params:
 *    @notify_ops
 *
 **/
int iface_axi_init(void *ctrl, void *in_params,
      iface_notify_ops_t *notify_ops)
{
  int rc = 0;
  iface_axi_hw_t *axi_hw = ctrl;
  iface_intf_type_t intf;
  iface_axi_init_params_t *init_params = (iface_axi_init_params_t *)in_params;

  switch (axi_hw->hw_state) {
  case IFACE_AXI_HW_STATE_DEV_OPEN: {
    axi_hw->notify_ops = notify_ops;
    axi_hw->hw_state = IFACE_AXI_HW_STATE_IDLE;
    axi_hw->init_params = *init_params;
    axi_hw->dev_idx = init_params->dev_idx;
    axi_hw->buf_mgr = init_params->buf_mgr;
    axi_hw->isp_version = init_params->isp_version;
    axi_hw->parent = notify_ops->parent;

  }
  break;

  default:
    break;
  }

  /* Initialize the axi_hw->intf with the correct session id.
   * This is because 2 sessions can share VFE */
  for (intf = 0; intf < IFACE_INTF_MAX; intf++) {
    if (init_params->session_resource->used_resource_mask &
      (1 << (16 * init_params->dev_idx + intf))) {
      if (axi_hw->intf_param[intf].session_id != 0) {
        CDBG_HIGH("%s: session id not cleared. Possible error\n", __func__);
      }
      axi_hw->intf_param[intf].session_id = init_params->session_id;
    }
  }

  return rc;
}

/** iface_axi_init_to_thread
 *
 *    @ctrl: isp root
 *    @out_params_size:
 *    @notify_ops:
 *
 **/
static int iface_axi_init_to_thread(void *ctrl, void *in_params, iface_notify_ops_t *notify_ops)
{
  int len, rc = 0;
  iface_axi_hw_t *axi_hw = ctrl;
  iface_axi_notify_ops_init_t init_cmd;
  iface_thread_t *thread;

  /* set init cmd to thread*/
  uint32_t cmd_id = IFACE_THREAD_CMD_NOTIFY_OPS_INIT;

  init_cmd.init_params = in_params;
  init_cmd.notify_ops = notify_ops;

  /* use stream thread to do stream on*/
  thread = &axi_hw->thread_stream;
  rc = iface_axi_sem_thread_execute(ctrl, thread, &init_cmd, cmd_id);

  return rc;
}

/** iface_axi_open
 *
 *    @isp_hw:
 *    @dev_name:
 *
 **/
static int iface_axi_open(iface_axi_hw_t *axi_hw, char *dev_name)
{
  int rc = 0;

  switch (axi_hw->hw_state) {
  case IFACE_AXI_HW_STATE_INVALID: {
    axi_hw->fd = open(dev_name, O_RDWR | O_NONBLOCK);

    if (axi_hw->fd <= 0) {
      CDBG_ERROR("%s: cannot open '%s'\n", __func__, dev_name);
      axi_hw->fd = 0;
      return -1;
    }

    /* hw opened, set state to idle */
    axi_hw->hw_state = IFACE_AXI_HW_STATE_DEV_OPEN;
  }
    break;

  default: {
    /* cannot open twice, nop */
    rc = -EAGAIN;
  }
    break;
  }

  if (rc == 0) {
    /* fork the hw thread to poll on vfe subdev and pipe */
    rc = iface_thread_start(&axi_hw->thread_poll, axi_hw, axi_hw->fd);
  }

  if(rc == 0) {
    /* fork the hw thread for stream on/off */
    rc = iface_sem_thread_start(&axi_hw->thread_stream, axi_hw);
  }

  return rc;
}

/** iface_create_axi_hw
 *
 *    @dev_name:
 *
 **/
iface_ops_t *iface_axi_create_hw(char *dev_name)
{
  int sd_num;
  int rc = 0, dev_fd = 0;
  iface_axi_hw_t *axi_hw = NULL;

  axi_hw = malloc(sizeof(iface_axi_hw_t));
  if (axi_hw == NULL) {
    /* no mem */
    IFACE_ERR("%s: no mem", __func__);
    return NULL;
  }

  memset(axi_hw, 0, sizeof(iface_axi_hw_t));
  axi_hw->hw_state = IFACE_AXI_HW_STATE_INVALID;
  axi_hw->hw_ops.ctrl = axi_hw;

  /* ops function will pass thread cmd to thread, thread will execute the function*/
  axi_hw->hw_ops.init = iface_axi_init_to_thread;
  axi_hw->hw_ops.destroy = iface_axi_destroy_to_thread;
  axi_hw->hw_ops.set_params = iface_axi_set_params_to_thread;
  axi_hw->hw_ops.get_params = iface_axi_get_params_to_thread;
  axi_hw->hw_ops.action = iface_axi_action_to_thread;

  /* open ISP and fork thread for event polling and cmd processing */
  rc = iface_axi_open(axi_hw, dev_name);
  if (rc < 0) {
    goto error;
  }

  return &axi_hw->hw_ops;

error:
  iface_axi_destroy(axi_hw->hw_ops.ctrl);

  return NULL;
}
