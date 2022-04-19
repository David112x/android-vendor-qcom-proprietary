/*============================================================================
Copyright (c) 2013-2020 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/media.h>

#include "chromatix.h"
#include "camera_dbg.h"
#include "cam_types.h"
#include "cam_intf.h"
#include "mct_controller.h"
#include "modules.h"
#include "iface.h"
#include "iface_util.h"
#include "ispif/iface_ispif.h"
#include "axi/iface_axi_util.h"
#include "axi/iface_axi.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#define ALL_ONES_16(x) (x ? (((uint16_t)~0) << (16-x)) : 0)
#define PAD_TO_SIZE(size, padding)  (((size) + padding - 1) / padding * padding)
#ifndef CEILING32
#define CEILING32(X) (((X) + 0x001F) & 0xFFE0)
#endif
/* 10% margin for vfe line blanking */
#define IFACE_LINE_TIME_MARGIN 0.1

/* #define IFACE_UTIL_DEBUG */
#ifdef IFACE_UTIL_DEBUG
#undef CDBG
#define CDBG ALOGE
#undef CDBG_ERROR
#define CDBG_ERROR ALOGE
#undef CDBG_HIGH
#define CDBG_HIGH ALOGE
#endif

#define CDS_TRIGGER_A 300
#define CDS_TRIGGER_B 330

#define TESTGEN_HBI (0.05)
#define TESTGEN_VBI (0.10)

#define PDAF_DEBUG 0

#define BYTES_PER_DWORD 16
#define REQUIRED_MIN_HBI 64
#define REQUIRED_MIN_VBI 32

int iface_util_fe_cfg(iface_session_t *session, int fd, cam_dimension_t dim,
  uint32_t op_pixel_clk, cam_format_t fmt, isp_stripe_id_t stripe_id);

void iface_util_broadcast_fe_sof_msg_to_modules(iface_t *iface, uint32_t session_id,
  uint32_t user_stream_id, mct_bus_msg_isp_sof_t *sof_event);

static iface_output_pack_mode iface_util_choose_pack_mode(unsigned char cid,
  iface_output_pack_mode sensor_cap_pack_mode, sensor_out_info_t* sensor_out_info);

static void iface_util_pdaf_dbg_info(
   pdaf_block_pattern_t *native_pattern,
   pdaf_data_buffer_info_t * decimated_info)
{
  pdaf_block_pattern_t *camif_v0_pattern =
   &decimated_info->buffer_block_pattern_left;
  pdaf_block_pattern_t *camif_v1_pattern =
   &decimated_info->buffer_block_pattern_right;
  int32_t i;
  CDBG_ERROR("native block pattern");
  CDBG_ERROR("pix_count %d", native_pattern->pix_count);
  for (i = 0; i < (int)native_pattern->pix_count; i++)
  {
    CDBG_ERROR("position %d pix_coords %d, x %d, flag %d",
      i,
      native_pattern->pix_coords[i].x,
      native_pattern->pix_coords[i].y,
      native_pattern->pix_coords[i].flag);
  }
  CDBG_ERROR("pd_offset_horizontal %d, pd_offset_vertical %d",
      native_pattern->pd_offset_horizontal,
      native_pattern->pd_offset_vertical);
  CDBG_ERROR("block_dim.width %d, block_dim.height %d",
      native_pattern->block_dim.width, native_pattern->block_dim.height);

  CDBG_ERROR("camif v0 block pattern");
  CDBG_ERROR("pix_count %d", camif_v0_pattern->pix_count);
  for (i = 0; i < (int)camif_v0_pattern->pix_count; i++)
  {
    CDBG_ERROR("position %d pix_coords %d, x %d, flag %d",
      i,
      camif_v0_pattern->pix_coords[i].x,
      camif_v0_pattern->pix_coords[i].y,
      camif_v0_pattern->pix_coords[i].flag);
  }
  CDBG_ERROR("pd_offset_horizontal %d, pd_offset_vertical %d",
      camif_v0_pattern->pd_offset_horizontal,
      camif_v0_pattern->pd_offset_vertical);
  CDBG_ERROR("block_dim.width %d, block_dim.height %d",
      camif_v0_pattern->block_dim.width, camif_v0_pattern->block_dim.height);
  /* dual isp */
  if(decimated_info->image_overlap)
  {
    CDBG_ERROR("camif v1 block pattern");
    CDBG_ERROR("pix_count %d", camif_v1_pattern->pix_count);
    for (i = 0; i < (int)camif_v1_pattern->pix_count; i++)
    {
      CDBG_ERROR("position %d pix_coords %d, x %d, flag %d",
        i,
        camif_v1_pattern->pix_coords[i].x,
        camif_v1_pattern->pix_coords[i].y,
        camif_v1_pattern->pix_coords[i].flag);
    }
    CDBG_ERROR("pd_offset_horizontal %d, pd_offset_vertical %d",
        camif_v1_pattern->pd_offset_horizontal,
        camif_v1_pattern->pd_offset_vertical);
    CDBG_ERROR("block_dim.width %d, block_dim.height %d",
        camif_v1_pattern->block_dim.width, camif_v1_pattern->block_dim.height);
  }
}

/** iface_util_camif_extract_decimated_pattern:
 *    @p_native_info: original pd pattern in metal grid
 *    @updated_global_offset_x: offset after camif skipping and
 *                            binning
 *    @updated_global_offset_y: offset after camif skipping and
 *                            binning
 *    @p_buffer_block_pattern: pd pattern after camif skipping
 *                           and binning
 *  function runs in MCTL thread context.
 *
 *  This function takes pdaf native pattern and generate the pd
 *  pattern in raw buffer
 *
 **/
static void iface_util_camif_extract_decimated_pattern(
    pdaf_sensor_native_info_t * p_native_info,
    int32_t updated_global_offset_x,
    int32_t updated_global_offset_y,
    pdaf_block_pattern_t * p_buffer_block_pattern)
{
  iface_camif_pix_ext_t new_pdaf_pix_coords[MAX_CAMIF_PDAF_PIXELS];
  iface_camif_pix_ext_t sub_pdaf_pix_coords[MAX_CAMIF_PDAF_PIXELS];
  int32_t               i;
  int32_t               pdaf_pix_count;
  uint32_t              line_skip, pixel_skip;
  int32_t               sub_box_width, sub_box_height;
  int32_t               offset_sum_x, offset_sum_y;
  int32_t               offset_b_x, offset_b_y, offset_ext_x, offset_ext_y;
  int32_t               const_x, const_y;

   pdaf_pix_count = p_native_info->block_pattern.pix_count;
   /* CAMIF Pixel and Line Skip operate on 16 pixels and 16 lines
      at a time, respectively. Hence a 16x16 block is considered
      while sub sampling is enabled */
   const_x = p_native_info->block_pattern.block_dim.width / 16;
   const_y = p_native_info->block_pattern.block_dim.height / 16;

  line_skip = 0;
  pixel_skip = 0;

  for (i = 0; i < pdaf_pix_count; i++)
  {
    new_pdaf_pix_coords[i].x =
      (p_native_info->block_pattern.pix_coords[i].x +
        updated_global_offset_x) % p_native_info->block_pattern.block_dim.width;
    new_pdaf_pix_coords[i].y =
      (p_native_info->block_pattern.pix_coords[i].y +
        updated_global_offset_y) % p_native_info->block_pattern.block_dim.height;

    /* index of each 16x16 block */
    new_pdaf_pix_coords[i].b_x = new_pdaf_pix_coords[i].x / 16;
    new_pdaf_pix_coords[i].b_y = new_pdaf_pix_coords[i].y / 16;

    /* mod by 16 to fit camif subsampling pattern */
    new_pdaf_pix_coords[i].x = (new_pdaf_pix_coords[i].x) % 16;
    new_pdaf_pix_coords[i].y = (new_pdaf_pix_coords[i].y) % 16;

   /*find line and column index for camif subsample
   line skip MSB - first line
   set the corresponding bit to one */
    line_skip = line_skip |
        (((uint32_t)(1)) << (15 - new_pdaf_pix_coords[i].y));

  /* pixel skip MSB - first pixel */
    pixel_skip = pixel_skip |
        (((uint32_t)(1)) << (15 - new_pdaf_pix_coords[i].x));
  }

  /* subsampled block size */
  sub_box_width = iface_util_find_number_skip(pixel_skip);
  sub_box_height = iface_util_find_number_skip(line_skip);

  /*
   * pixel coord in the subsampled image, 16x16 to sub_box_width * sub_box_height
   * and recover to corresponding 64x64 range
   */
  for (i = 0; i < pdaf_pix_count; i++)
  {
    sub_pdaf_pix_coords[i].x =
        iface_util_find_number_skip(pixel_skip >> (15 - new_pdaf_pix_coords[i].x)) - 1;
    sub_pdaf_pix_coords[i].x +=
        new_pdaf_pix_coords[i].b_x * sub_box_width;
    sub_pdaf_pix_coords[i].y =
        iface_util_find_number_skip(line_skip >> (15 - new_pdaf_pix_coords[i].y)) - 1;
    sub_pdaf_pix_coords[i].y +=
        new_pdaf_pix_coords[i].b_y * sub_box_height;
  }

  offset_b_x = ((int32_t)(updated_global_offset_x / 16)) * sub_box_width;
  offset_b_y = ((int32_t)(updated_global_offset_y / 16)) * sub_box_height;

  offset_ext_x = 0;
  if ((updated_global_offset_x % 16) > 0)
  {
    offset_ext_x = iface_util_find_number_skip(pixel_skip >> (16 - (updated_global_offset_x % 16)));
  }

  offset_ext_y = 0;
  if ((updated_global_offset_y % 16) > 0)
  {
    offset_ext_y = iface_util_find_number_skip(line_skip >> (16 - (updated_global_offset_y % 16)));
  }

  offset_sum_x                                 = offset_b_x + offset_ext_x;
  offset_sum_y                                 = offset_b_y + offset_ext_y;
  p_buffer_block_pattern->pd_offset_horizontal = offset_sum_x;
  p_buffer_block_pattern->pd_offset_vertical   = offset_sum_y;
  p_buffer_block_pattern->block_dim.width = sub_box_width * const_x;
  p_buffer_block_pattern->block_dim.height = sub_box_height * const_y;
  p_buffer_block_pattern->pix_count = pdaf_pix_count;

  /* remove offset info from coordinates */
  for (i = 0; i < pdaf_pix_count; i++)
  {
    p_buffer_block_pattern->pix_coords[i].x =
        (sub_pdaf_pix_coords[i].x - offset_sum_x
        + 1024 * sub_box_width * const_x)
        % (sub_box_width * const_x);
    p_buffer_block_pattern->pix_coords[i].y =
        (sub_pdaf_pix_coords[i].y - offset_sum_y
         + 1024 * sub_box_height * const_y) % (sub_box_height * const_y);
    p_buffer_block_pattern->pix_coords[i].flag =
        p_native_info->block_pattern.pix_coords[i].flag;
  }
}

/** iface_util_pdaf_get_camif_decimated_pattern:
 *    @p_native_info: original pd pattern in metal grid
 *    @decimated_pattern: pd pattern information
 *     after CAMIF subsampling of the original pd
 *     image
 *  Function runs in MCTL thread context.
 *
 *  This function takes pdaf native pattern, adjusts based on
 *  orientation and calls the function to generate the pd
 *  pattern in left/right raw buffer
 **/
static void iface_util_pdaf_get_camif_decimated_pattern(
    pdaf_sensor_native_info_t * p_native_info,
    pdaf_data_buffer_info_t * decimated_pattern)
{
  uint32_t image_overlap, right_image_offset;
  uint32_t global_offset_horizontal, global_offset_vertical;
  int32_t  divide_idx, adj_left_vfe_width, adj_right_vfe_offset_x;
  pdaf_block_pattern_t * p_buffer_block_pattern_left;
  pdaf_block_pattern_t * p_buffer_block_pattern_right;

  image_overlap = decimated_pattern->image_overlap;
  right_image_offset = decimated_pattern->right_image_offset;
  p_buffer_block_pattern_left = &decimated_pattern->buffer_block_pattern_left;
  p_buffer_block_pattern_right = &decimated_pattern->buffer_block_pattern_right;

  global_offset_horizontal
   = p_native_info->block_pattern.pd_offset_horizontal;
  global_offset_vertical
   = p_native_info->block_pattern.pd_offset_vertical;

  /* single VFE*/
  if (image_overlap == 0)
  {
    iface_util_camif_extract_decimated_pattern(p_native_info,
        global_offset_horizontal,
        global_offset_vertical,
        p_buffer_block_pattern_left);
  } else { /* dual VFE */
    divide_idx = (image_overlap + right_image_offset -
        global_offset_horizontal) /
        p_native_info->block_pattern.block_dim.width;
    adj_left_vfe_width = divide_idx *
        p_native_info->block_pattern.block_dim.width +
        global_offset_horizontal;
    adj_right_vfe_offset_x = adj_left_vfe_width - right_image_offset;

    iface_util_camif_extract_decimated_pattern(p_native_info,
        global_offset_horizontal,
        global_offset_vertical,
        p_buffer_block_pattern_left);

    iface_util_camif_extract_decimated_pattern(p_native_info,
        adj_right_vfe_offset_x,
        global_offset_vertical,
        p_buffer_block_pattern_right);
  }
}

/** iface_util_find_primary_cid:
 *    @sensor_cfg: Sensor configuration
 *    @sensor_cap: Sensor source port capabilities
 *
 *  This function runs in MCTL thread context.
 *
 *  This function finds primary CID for sensor source port by format
 *
 *  Return: Array index of the primary CID
 **/
uint32_t iface_util_find_primary_cid(sensor_out_info_t *sensor_cfg,
  sensor_src_port_cap_t *sensor_cap)
{
  int i;

  if (sensor_cap->num_cid_ch > SENSOR_CID_CH_MAX) {
    CDBG_ERROR("%s:%d Invalid number of sensor CIDs: %d", __func__,
      __LINE__, sensor_cap->num_cid_ch);

    return(0);
  }

  for(i = 0; i < sensor_cap->num_cid_ch; i++)
    if (sensor_cfg->fmt == sensor_cap->sensor_cid_ch[i].fmt)
      break;

  if((i == sensor_cap->num_cid_ch) ||
     (i >= SENSOR_CID_CH_MAX)) {
    IFACE_HIGH("%s:%d error cannot find primary sensor format cids %d",
      __func__, __LINE__, sensor_cap->num_cid_ch);

    return(0);
  }

  return(i);
}

/** iface_util_sensor_is_yuv:
 *    @sink_port: Sink port connected to sensor
 *
 *  This function runs in MCTL thread context.
 *
 *  This function returns true if connected sensor ouput format is YUV
 *  interleaved.
 *
 *  Return: None
 **/
static boolean iface_util_sensor_is_yuv(iface_sink_port_t *sink_port)
{
  uint32_t           primary_cid_idx;
  cam_format_t       format;
  bool               is_depth = FALSE;
  cam_stream_type_t  stream_type = CAM_STREAM_TYPE_DEPTH;
  int                i = 0;

  for (i=0; i < sink_port->num_streams; i++) {
    if (stream_type ==
      sink_port->streams[i].stream_info.stream_type)
      is_depth = TRUE;
  }
  primary_cid_idx =
    iface_util_find_primary_cid(&sink_port->sensor_out_info,
      &sink_port->sensor_cap);
  format = sink_port->sensor_cap.sensor_cid_ch[primary_cid_idx].fmt;
  /* Here the assumption is sensor has only Depth format and no PIX *
   * format support for given depth sensor. If in future a given    *
   * sensor supports both PIX and Depth then is_depth logic is made *
   * per Virtual Channel                                            */
  return ((is_depth == TRUE) ||
          ((format >= CAM_FORMAT_YUV_RAW_8BIT_YUYV) &&
          (format <= CAM_FORMAT_YUV_RAW_8BIT_VYUY)));
}

/** iface_util_dump_sensor_cfg:
 *    @sink_port: Sink port connected to sensor
 *
 *  This function runs in MCTL thread context.
 *
 *  This function dumps sensor configuration by port.
 *
 *  Return: None
 **/
void iface_util_dump_sensor_cfg(iface_sink_port_t *sink_port)
{
  sensor_out_info_t *sensor_cfg = &sink_port->sensor_out_info;
  uint32_t           primary_cid_idx, k = 0;
  primary_cid_idx =
    iface_util_find_primary_cid(sensor_cfg, &sink_port->sensor_cap);
  if (primary_cid_idx >= SENSOR_CID_CH_MAX) {
    CDBG_ERROR("%s:%d primary_cid_idx %d SENSOR_CID_CH_MAX %d", __func__,
      __LINE__, primary_cid_idx, SENSOR_CID_CH_MAX);
    return;
  }
  CDBG("%s: num_cid_ch= %d num_meta_ch= %d \n", __func__,
    sink_port->sensor_cap.num_cid_ch,
    sink_port->sensor_cap.num_meta_ch);

  for (k = 0; k < sink_port->sensor_cap.num_cid_ch
    && k < SENSOR_CID_CH_MAX; k++) {
    CDBG("%s: non meta cid[%d] = %d csid %d\n", __func__, k,
      sink_port->sensor_cap.sensor_cid_ch[k].cid,
      sink_port->sensor_cap.sensor_cid_ch[k].csid);
  }

  for (k = 0; k < sink_port->sensor_cap.num_meta_ch && k < MAX_META; k++) {
    CDBG("%s: meta cid[%d] = %d csid %d\n", __func__, k,
      sink_port->sensor_cap.meta_ch[k].cid,
      sink_port->sensor_cap.meta_ch[k].csid);
  }

  IFACE_HIGH("%s: sensor dim: width = %d, heght = %d, fmt = %d, is_bayer = %d, init_skip = %d\n",
    __func__, sensor_cfg->dim_output.width, sensor_cfg->dim_output.height,
    sink_port->sensor_cap.sensor_cid_ch[primary_cid_idx].fmt,
    sink_port->sensor_cap.sensor_cid_ch[primary_cid_idx].is_bayer_sensor,
    sink_port->sensor_out_info.num_frames_skip);

  CDBG("%s: camif_crop: first_pix = %d, last_pix = %d, "
    "first_line = %d, last_line = %d, max_fps = %d\n", __func__,
    sensor_cfg->request_crop.first_pixel, sensor_cfg->request_crop.last_pixel,
    sensor_cfg->request_crop.first_line, sensor_cfg->request_crop.last_line,
    (int)sensor_cfg->max_fps);

  CDBG("%s: meta info: num_meta = %d, format = %d, W %d, H %d\n", __func__,
    sensor_cfg->meta_cfg.num_meta,
    sensor_cfg->meta_cfg.sensor_meta_info[0].fmt,
    sensor_cfg->meta_cfg.sensor_meta_info[0].dim.width,
    sensor_cfg->meta_cfg.sensor_meta_info[0].dim.height);
}

/** iface_util_get_empty_hw_stream:
 *    @iface_session: iface session
 *    @hw_stream: output hw stream
 *
 *  This function finds empty slot for hw streams and return when found
 *
 *  Return: NULL: no empty hw stream found
 **/
iface_hw_stream_t *iface_util_get_empty_hw_stream(iface_session_t *iface_session)
{
  int32_t            i = 0;
  iface_hw_stream_t *hw_stream = NULL;

  for (i = 0; i < IFACE_MAX_STREAMS; i++) {
     /* find empty slot in session hw streams*/
     if (&iface_session->hw_streams[i] != NULL &&
       iface_session->hw_streams[i].stream_info.dim.width == 0) {
       hw_stream = &iface_session->hw_streams[i];
       return hw_stream;
     }
  }
  CDBG_ERROR("%s:fatal error: No empty slot available for hw stream\n",
    __func__);
  return NULL;
}

/** iface_util_get_hw_stream_by_mask:
 *    @iface_session: iface session
 *    @hw_stream: output hw stream
 *
 *  This function finds used hw stream by specific stream type.
 *
 *  Return: NULL: no empty hw stream found
 **/
static iface_hw_stream_t *iface_util_get_hw_stream_by_mask(
  iface_session_t *iface_session,
  iface_port_t    *iface_sink_port,
  const uint32_t   mask)
{
  uint32_t            i, j;
  iface_hw_stream_t *hw_stream = NULL;
  iface_stream_t    *iface_stream = NULL;

  for (i = 0; i < IFACE_MAX_STREAMS; i++) {
    /* find empty slot in session hw streams*/
    if (&iface_session->hw_streams[i] != NULL &&
      iface_session->hw_streams[i].stream_info.dim.width != 0) {
      hw_stream = &iface_session->hw_streams[i];
      for (j = 0; j < hw_stream->num_mct_stream_mapped; j++) {
        iface_stream = iface_util_find_stream_in_sink_port(iface_sink_port,
          iface_session->session_id, hw_stream->mapped_mct_stream_id[j]);
        if(iface_stream)
          if((1 << iface_stream->stream_info.stream_type) & mask) {
            return hw_stream;
          }
      }
    }
  }
  return NULL;
}

/** iface_util_has_isp_pix_interface
 *
 * DESCRIPTION:
 *
 **/
boolean iface_util_has_isp_pix_interface(iface_t *iface)
{
  int i;
  uint32_t total_used_intf = 0;
  uint32_t pix0_intf_mask = 1 << (16 * VFE0 + IFACE_INTF_PIX);
  uint32_t pix1_intf_mask = 1 << (16 * VFE1 + IFACE_INTF_PIX);
  uint8_t has_pix_intef = FALSE;

  for (i = 0; i < IFACE_MAX_SESSIONS; i++) {
     if (iface->sessions[i].iface != NULL) {
       total_used_intf |= iface->sessions[i].session_resource.used_resource_mask;
     }
  }

  if ((total_used_intf & pix0_intf_mask) && (total_used_intf & pix1_intf_mask))
    has_pix_intef = FALSE;
  else
    has_pix_intef = TRUE;

  return has_pix_intef;
}

/** iface_util_calc_num_plane:
 *
 *  This function checks num_plane for each stream
 *
 **/
uint32_t iface_util_calc_num_plane(cam_format_t stream_fmt)
{
  int plane_num = 0;

  switch (stream_fmt) {
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

  return plane_num;
}
/** iface_util_has_pix_resource:
 *
 *  This function runs in MCTL thread context.
 *
 *  This function checks if PIX interface is needed and available
 *
 *  Return: TRUE  - There is PIX interface available or it is not needed for
 *                  this stream
 *          FALSE - There is no available PIX interface
 **/
boolean iface_util_has_pix_resource(iface_t *iface, iface_sink_port_t *sink_port,
  mct_stream_info_t *stream_info)
{
  sensor_src_port_cap_t *sensor_port_cap = &sink_port->sensor_cap;
  uint32_t primary_cid_idx = iface_util_find_primary_cid(&sink_port->sensor_out_info,
    sensor_port_cap);
  if (primary_cid_idx >= SENSOR_CID_CH_MAX) {
    CDBG_ERROR("%s:%d primary_cid_idx %d SENSOR_CID_CH_MAX %d", __func__,
      __LINE__, primary_cid_idx, SENSOR_CID_CH_MAX);
    return FALSE;
  }

  if (sensor_port_cap->sensor_cid_ch[primary_cid_idx].is_bayer_sensor &&
      sensor_port_cap->sensor_cid_ch[primary_cid_idx].fmt != stream_info->fmt) {
    switch (stream_info->fmt) {
    case CAM_FORMAT_YUV_420_NV12:
    case CAM_FORMAT_YUV_420_NV12_UBWC:
    case CAM_FORMAT_YUV_420_NV12_VENUS:
    case CAM_FORMAT_YUV_420_NV21_VENUS:
    case CAM_FORMAT_YUV_420_NV21:
    case CAM_FORMAT_YUV_420_NV21_ADRENO:
    case CAM_FORMAT_YUV_420_YV12:
    case CAM_FORMAT_YUV_422_NV16:
    case CAM_FORMAT_YUV_422_NV61:
    case CAM_FORMAT_YUV_444_NV24:
    case CAM_FORMAT_YUV_444_NV42:
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GBRG:
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GRBG:
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_RGGB:
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_BGGR:
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GBRG:
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GRBG:
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_RGGB:
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_BGGR:
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GBRG:
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GRBG:
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_RGGB:
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_BGGR:
    case CAM_FORMAT_BAYER_QCOM_RAW_14BPP_GBRG:
    case CAM_FORMAT_BAYER_QCOM_RAW_14BPP_GRBG:
    case CAM_FORMAT_BAYER_QCOM_RAW_14BPP_RGGB:
    case CAM_FORMAT_BAYER_QCOM_RAW_14BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_14BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_14BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_14BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_14BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_14BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_14BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_14BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_14BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_BGGR:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GBRG:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GRBG:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_RGGB:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_BGGR:
    case CAM_FORMAT_Y_ONLY:
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GREY:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GREY:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GREY:
    case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GREY:
      return iface_util_has_isp_pix_interface(iface);
    default:
      return TRUE;
    }
  } else
    return TRUE;
}

/** iface_util_find_stream_in_sink_port:
 *
 *  This function runs in MCTL thread context.
 *
 *  This function finds a stream with certain ID in session with given id
 *
 *  Return: NULL - Stream not found
 *          Otherwise - pointer to stream found
 **/
iface_stream_t *iface_util_find_stream_in_sink_port(
  iface_port_t *iface_port,
  uint32_t      session_id,
  uint32_t      stream_id)
{
  int i, k;
  iface_sink_port_t *iface_sink_port = &iface_port->u.sink_port;
  iface_stream_t *iface_stream;

  for (i = 0; i < IFACE_MAX_STREAMS; i++) {
     iface_stream = &iface_sink_port->streams[i];
     if (iface_stream->stream_id == stream_id && iface_stream->session_id == session_id) {
       CDBG("%s: found stream in sink port, session id = %x, streamid = %x\n", __func__, session_id, stream_id);
       return iface_stream;
      }
   }

  CDBG("%s: X; no mct stream, return NULL, stream id %d, sess id %d\n",
       __func__, stream_id, session_id);
  return NULL;
}

/** ispif_util_find_stream_in_session:
 *    @session: Session to find stream in
 *    @stream_id: Stream id
 *
 *  This function runs in MCTL thread context.
 *
 *  This function finds a stream with certain ID in session
 *
 *  Return: NULL - Stream not found
 *          Otherwise - pointer to stream found
 **/
iface_hw_stream_t *iface_util_find_hw_stream_in_session(iface_session_t *session,
  uint32_t hw_stream_id)
{
  int i;

  for (i = 0; i < IFACE_MAX_STREAMS; i++) {
    CDBG("%s: stream: sessionid = %d, hw_stream_id = %x\n",
      __func__, session->session_id, session->hw_streams[i].hw_stream_id);
    if (session->hw_streams[i].session_id != 0 &&
          session->hw_streams[i].hw_stream_id == hw_stream_id) {
      return &session->hw_streams[i];
    }
  }

  return NULL;
}

/** iface_util_count_diverted_hw_stream_in_session:
 *    @session: Session to find stream in
 *
 *  This function runs in MCTL thread context.
 *
 *  This function counts number of streams that divert buffers
 *
 *  Return: Nnumber of streams
 **/
static int iface_util_count_diverted_hw_stream_in_session(
  iface_session_t *session)
{
  int i;
  int cnt = 0;

  for (i = 0; i < IFACE_MAX_STREAMS; i++) {
    CDBG("%s: stream: sessionid = %d, hw_stream_id = %x\n",
      __func__, session->session_id, session->hw_streams[i].hw_stream_id);
    if (session->hw_streams[i].session_id != 0 &&
        session->hw_streams[i].need_divert) {
      cnt++;
    }
  }

  return cnt;
}

/** iface_util_find_stream_in_src_port:
 *    @ispif: ispif isntance
 *    @ispif_src_port: ispif source port
 *    @session_id: Session ID
 *    @stream_id: Stream ID
 *
 *  This function runs in MCTL thread context.
 *
 *  This function finds a stream linked to ispif source port by ID
 *
 *  Return: NULL - Stream not found
 *          Otherwise - pointer to stream found
 **/
iface_stream_t *iface_util_find_stream_in_src_port(
  iface_t      *iface __unused,
  iface_port_t *iface_src_port,
  uint32_t      session_id,
  uint32_t      stream_id)
{
  int i;

  for (i = 0; i < IFACE_MAX_STREAMS; i++) {
    if ((iface_src_port->u.src_port.streams[i].link_cnt > 0) &&
        iface_src_port->u.src_port.streams[i].session_id == session_id &&
        iface_src_port->u.src_port.streams[i].stream_id == stream_id) {

      return &iface_src_port->u.src_port.streams[i];
    }
  }

  return NULL;
}

/** iface_util_update_stream_info_dims_for_rotation:
 *    @stream_info: stream info
 *
 *  This function runs in MCTL thread context.
 *
 *  This function swaps width and height in case of 90/270 degree rotation.
 *
 *  Return: 0 - Success
 *         -1 - Port is connected to its max number of streams
 **/
void iface_util_update_stream_info_dims_for_rotation(
    mct_stream_info_t *stream_info)
{
    switch (stream_info->pp_config.rotation) {
      case ROTATE_90:
      case ROTATE_270: {
        int32_t swap;

        swap = stream_info->dim.width;
        stream_info->dim.width = stream_info->dim.height;
        stream_info->dim.height = swap;
      }
        break;
      case ROTATE_0:
      case ROTATE_180:
      default:
        break;
    }
}

/** iface_util_add_stream_to_sink_port:
 *    @ispif: ispif isntance
 *    @ispif_sink_port: ispif sinc port
 *    @stream: stream
 *
 *  This function runs in MCTL thread context.
 *
 *  This function adds a stream to sink potr
 *
 *  Return: 0 - Success
 *         -1 - Port is connected to its max number of streams
 **/
iface_stream_t *iface_util_add_stream_to_sink_port(iface_t *iface,
  iface_port_t *iface_port, iface_session_t *session,
  uint32_t stream_id, mct_stream_info_t *stream_info)
{
  int i;

  for (i = 0; i < IFACE_MAX_STREAMS; i++) {

    if (iface_port->u.sink_port.streams[i].sink_port == NULL) {
      memset(&iface_port->u.sink_port.streams[i], 0,
        sizeof(iface_port->u.sink_port.streams[i]));
      iface_port->u.sink_port.streams[i].sink_port = iface_port->port;
      iface_port->u.sink_port.streams[i].state = IFACE_STREAM_ASSOCIATED_WITH_SINK_PORT;
      iface_port->u.sink_port.streams[i].stream_id = stream_id;
      iface_port->u.sink_port.streams[i].stream_info = *stream_info;
      iface_util_update_stream_info_dims_for_rotation(
          &iface_port->u.sink_port.streams[i].stream_info);
      iface_port->u.sink_port.num_streams++;

      /* save session info, add num stream to session*/
      iface_port->u.sink_port.streams[i].session_id = session->session_id;
      iface_port->u.sink_port.streams[i].session = session;

      /* here we choose if pix is needed for the stream
         move it to set stream config
                check if sensor format is the same HAL format,
                then we use RDI*/
      iface_util_choose_isp_interface(iface, iface_port,
        &iface_port->u.sink_port.streams[i]);
      iface_port->u.sink_port.streams[i].link_cnt++;

      CDBG("%s: ADD sessid = %d streamid = %d, use_pix = %d, link_cnt = %d\n",
        __func__,  iface_port->u.sink_port.streams[i].session_id,
         iface_port->u.sink_port.streams[i].stream_id,
         iface_port->u.sink_port.streams[i].use_pix,
         iface_port->u.sink_port.streams[i].link_cnt);

      return &iface_port->u.sink_port.streams[i];
    }
  }

  return NULL;
}

/** iface_util_del_stream_from_sink_port:
 *
 *  This function runs in MCTL thread context.
 *
 *  This function deletes a stream from sink port
 *
 *  Return: 0 - Success
 *         -1 - Stream not found
 **/
int iface_util_del_stream_from_sink_port(
  iface_t        *iface __unused,
  iface_port_t   *iface_sink_port,
  iface_stream_t *stream)
{
  int i;
  CDBG("%s: E\n", __func__);

  for (i = 0; i < IFACE_MAX_STREAMS; i++) {
    if (&iface_sink_port->u.sink_port.streams[i] == stream) {
      CDBG("%s: delete stream id %d from sink port, link_cnt = %d--\n",
        __func__, stream->stream_id, stream->link_cnt);
      stream->link_cnt--;
      memset(&iface_sink_port->u.sink_port.streams[i], 0, sizeof(iface_stream_t));

      iface_sink_port->u.sink_port.num_streams--;
      return 0;
    }
  }

  return -1;
}

/** iface_util_add_stream_to_src_port:
 *
 *  This function runs in MCTL thread context.
 *
 *  This function ads a stream to source port
 *
 *  Return: 0 - Success
 *         -1 - Port is connected to its max number of streams
 **/
int iface_util_add_stream_to_src_port(
  iface_t        *iface __unused,
  iface_port_t   *iface_src_port,
  iface_stream_t *stream)
{
  int i;

  for (i = 0; i < IFACE_MAX_STREAMS; i++) {
    if (iface_src_port->u.src_port.streams[i].link_cnt == 0) {
      memcpy(&iface_src_port->u.src_port.streams[i], stream,
        sizeof(iface_src_port->u.src_port.streams[i]));
      iface_src_port->u.src_port.num_streams++;
      return 0;
    }
  }

  return -1;
}
/** iface_util_del_stream_from_src_port:
 *    @ispif: ispif instance
 *    @ispif_src_port: ispif source port
 *    @stream: stream
 *
 *  This function runs in MCTL thread context.
 *
 *  This function deletes a stream from source port
 *
 *  Return: 0 - Success
 *         -1 - Stream not found
 **/
int iface_util_del_stream_from_src_port(
  iface_t      *iface __unused,
  iface_port_t *iface_src_port,
  unsigned int  session_id,
  unsigned int  stream_id)
{
  int i;

  for (i = 0; i < IFACE_MAX_STREAMS; i++) {
    if ((iface_src_port->u.src_port.streams[i].link_cnt > 0) &&
      (iface_src_port->u.src_port.streams[i].session_id == session_id) &&
      (iface_src_port->u.src_port.streams[i].stream_id == stream_id)) {
      memset(&iface_src_port->u.src_port.streams[i], 0,
        sizeof(iface_src_port->u.src_port.streams[i]));
      iface_src_port->u.src_port.num_streams--;

      return 0;
    }
  }

  return -1;
}

/** iface_util_get_preferred_mask_by_type:
 *    @preferred_mapping: preffered mapping received from PPROC
 *    @stream_type: stream type which we are looking in prefered mapping for
 *
 *  This function returns a preffered mapping mask containing given stream
 *
 *  Return: mask for given stream preffered mapping
 **/
static uint32_t iface_util_get_preferred_mask_by_type(
  const isp_preferred_streams *preferred_mapping,
  const cam_stream_type_t      stream_type)
{
  uint32_t i;
  for (i = 0; i < preferred_mapping->stream_num
    && i < ISP_NUM_PIX_STREAM_MAX; i++) {
    if ((1 << stream_type) & preferred_mapping->streams[i].stream_mask) {
      return(preferred_mapping->streams[i].stream_mask);
    }
  }
  return 0;
}

/** iface_util_check_port_for_preferred_stream:
 *    @preferred_mapping: preffered mapping received from PPROC
 *    @src_port_data: iface source port
 *    @stream: stream
 *
 *  This function checks if a port is already allocated for stream which is
 *       grouped with currently given stream in requested preferences
 *
 *  Return:  TRUE - port can be reused for this stream
 *          FALSE - port cannot be reused
 **/
static boolean iface_util_check_port_for_preferred_stream(
    const isp_preferred_streams *preferred_mapping,
    const iface_src_port_t *src_port_data,
    const iface_stream_t *stream)
{
  int       i;
  uint32_t  mask;
  mask = iface_util_get_preferred_mask_by_type(preferred_mapping,
      stream->stream_info.stream_type);
  for (i = 0; i < src_port_data->num_streams && i < IFACE_MAX_STREAMS; i++) {
    if ((1 << src_port_data->streams[i].stream_info.stream_type) & mask) {
      return TRUE;
    }
  }
  return FALSE;
}

/** iface_util_find_matched_src_port_by_caps:
 *    @data1: MCTL port
 *    @data2: stream and sink port struct to match against
 *
 *  This function runs in MCTL thread context.
 *
 *  This function is a visitor: finds a matching source port for given stream
 *                               in a list
 *
 *  Return: TRUE  - Success
 *          FALSE - Port does not match
 **/
static boolean iface_util_find_matched_src_port_by_caps(void *data1, void *data2)
{
  mct_port_t *mct_port = (mct_port_t *)data1;
  iface_get_match_src_port_t *params = (iface_get_match_src_port_t *)data2;

  iface_port_t *iface_sink_port = params->iface_sink_port;
  iface_sink_port_t *sink_port_data = &iface_sink_port->u.sink_port;

  iface_port_t *iface_src_port = (iface_port_t * )mct_port->port_private;
  iface_src_port_t *src_port_data = &iface_src_port->u.src_port;
  if (iface_src_port->state != IFACE_PORT_STATE_CREATED &&
      (memcmp(&sink_port_data->sensor_cap, &src_port_data->caps.sensor_cap,
        sizeof(sink_port_data->sensor_cap)) == 0) &&
        (iface_sink_port->session_id == iface_src_port->session_id)) {
      if(iface_util_sensor_is_yuv(sink_port_data) &&
         !iface_util_check_port_for_preferred_stream(params->preferred_mapping,
              src_port_data, params->stream)) {
        /* In case of YUV sensor we will put every stream on a separate port
           unless PPROC specifies different preference */
        return FALSE;
      }
      /* this src port is for that stream */
      return TRUE;
  }
  return FALSE;
}

/** iface_util_get_match_src_port:
 *    @ispif: ispif pointer
 *    @ispif_sink_port: ispif pointer
 *    @stream: ispif pointer
 *
 *  This function runs in MCTL thread context.
 *
 *  This function finds matching source port for a stream
 *
 *  Return: NULL - Port not found
 *          Otherwise - Pointer to matching port
 **/
iface_port_t *iface_util_get_match_src_port(iface_t *iface,
  iface_port_t *iface_sink_port, iface_stream_t *stream)
{
  int i, rc = 0;

  iface_session_t *iface_session;
  iface_port_t *iface_src_port = NULL;
  iface_get_match_src_port_t params;

  mct_list_t *src_port_list = NULL;
  mct_port_t *mct_port = NULL;

  memset(&params,  0,  sizeof(params));
  params.iface_sink_port = iface_sink_port;
  params.stream = stream;
  iface_session = iface_util_get_session_by_id(iface, stream->session_id);
  if(!iface_session) {
    CDBG_ERROR("%s: failed: session %x is NULL\n", __func__,
        stream->session_id);
    return NULL;
  }
  params.preferred_mapping = &iface_session->preferred_mapping;

  src_port_list = mct_list_find_custom (iface->module->srcports,
    (void *)&params, iface_util_find_matched_src_port_by_caps);

  if (src_port_list) {
    mct_port = (mct_port_t *)src_port_list->data;
    iface_src_port = (iface_port_t *)mct_port->port_private;
  }

  return iface_src_port;
}

/** iface_util_compare_sink_port_caps:
 *    @data1: MCTL port
 *    @data2: sensor source port capabilities
 *
 *  This function runs in MCTL thread context.
 *
 *  This function is a visitor: finds a matching sink port for given sensor
 *                              port capabilities
 *
 *  Return: TRUE  - Success
 *          FALSE - Port does not match
 **/
static boolean iface_util_compare_sink_port_caps(void *data1, void *data2)
{
  sensor_src_port_cap_t *sensor_cap = (sensor_src_port_cap_t *)data2;
  mct_port_t *mct_port = (mct_port_t *)data1;
  iface_port_t *iface_sink_port = (iface_port_t * )mct_port->port_private;
  iface_sink_port_t *sink_port = &iface_sink_port->u.sink_port;

  if (iface_sink_port->state != IFACE_PORT_STATE_CREATED &&
      memcmp(sensor_cap, &sink_port->sensor_cap,
        sizeof(sensor_src_port_cap_t)) == 0) {
      /* has the match */
    CDBG("%s: find an old sink port\n", __func__);
      return TRUE;
  } else
    return FALSE;
}

/** iface_util_compare_sink_port_session_id:
 *    @data1: MCTL port
 *    @data2: sensor source port capabilities
 *
 *  This function runs in MCTL thread context.
 *
 *  This function is a visitor: finds a matching sink port for given sensor
 *                              port capabilities
 *
 *  Return: TRUE  - Success
 *          FALSE - Port does not match
 **/
static boolean iface_util_compare_sink_port_session_id(void *data1, void *data2)
{
  uint32_t *session_id = (uint32_t *)data2;
  mct_port_t *mct_port = (mct_port_t *)data1;
  iface_port_t *iface_sink_port = (iface_port_t * )mct_port->port_private;

  if (iface_sink_port->state != IFACE_PORT_STATE_CREATED &&
      iface_sink_port->session_id == *session_id) {
      /* has the match */
    CDBG("%s: find an old sink port\n", __func__);
      return TRUE;
  } else
    return FALSE;
}

/** iface_util_compare_sink_port_identity:
 *    @data1: MCTL port
 *    @data2: sensor source port capabilities
 *
 *  This function runs in MCTL thread context.
 *
 *  This function is a visitor: finds a matching sink port for given sensor
 *                              port capabilities
 *
 *  Return: TRUE  - Success
 *          FALSE - Port does not match
 **/
static boolean iface_util_compare_identity_sink_port(void *data1, void *data2)
{
  mct_port_t *mct_port = (mct_port_t *)data1;
  uint32_t identity = *((uint32_t *)data2);
  iface_port_t *iface_port = (iface_port_t * )mct_port->port_private;

  uint32_t session_id =  UNPACK_SESSION_ID(identity);
  uint32_t stream_id = UNPACK_STREAM_ID(identity);

  if (iface_port->state != IFACE_PORT_STATE_CREATED) {
     if (NULL != iface_util_find_stream_in_sink_port(iface_port, session_id, stream_id)) {
       /* has the match */
       CDBG("%s: find matching streasm in sinkport\n", __func__);
       return TRUE;
     }
  }

  CDBG("%s:X,  no stream found, stream id = %x \n", __func__, stream_id);
  return FALSE;
}

/** iface_util_find_sink_port_by_cap:
 *    @ispif: ispif instance
 *    @sensor_cap: ispif pointer
 *
 *  This function runs in MCTL thread context.
 *
 *  This function finds a sink port matching to a sensor source port with given
 *  capabilities
 *
 *  Return: NULL - Port not fouund
 *          Otherwise - Pointer to ispif sink port
 **/
iface_port_t *iface_util_find_sink_port_by_cap(iface_t *iface,
  sensor_src_port_cap_t *sensor_cap)
{
  mct_list_t *sink_port_list = NULL;
  mct_port_t *mct_port = NULL;
  iface_port_t *iface_sink_port = NULL;

  /*find sink port match the sensor cap*/
  sink_port_list = mct_list_find_custom(iface->module->sinkports,
    (void *)sensor_cap, iface_util_compare_sink_port_caps);

  if (sink_port_list != NULL) {
    mct_port = (mct_port_t *)sink_port_list->data;
    iface_sink_port = (iface_port_t *)mct_port->port_private;
    CDBG("%s: old sink port used\n", __func__);
  }

  return iface_sink_port;
}

/** iface_util_find_sink_port_by_session_id:
 *    @iface: iface instance
 *    @session id: session identity
 *
 *  This function runs in MCTL thread context.
 *
 *  This function finds a sink port matching to a sensor source port with given
 *  capabilities
 *
 *  Return: NULL - Port not fouund
 *          Otherwise - Pointer to ispif sink port
 **/
iface_port_t *iface_util_find_sink_port_by_session_id(iface_t *iface,
  uint32_t *session_id)
{
  mct_list_t *sink_port_list = NULL;
  mct_port_t *mct_port = NULL;
  iface_port_t *iface_sink_port = NULL;

  /*find sink port match the sensor cap*/
  sink_port_list = mct_list_find_custom(iface->module->sinkports,
    (uint32_t *)session_id, iface_util_compare_sink_port_session_id);

  if (sink_port_list != NULL) {
    mct_port = (mct_port_t *)sink_port_list->data;
    iface_sink_port = (iface_port_t *)mct_port->port_private;
    CDBG("%s: old sink port used\n", __func__);
  }

  return iface_sink_port;
}

/** iface_util_find_stream_in_portlist:
 *    @ispif: ispif instance
 *    @sensor_cap: ispif pointer
 *
 *  This function runs in MCTL thread context.
 *
 *  This function finds a sink port matching to a sensor source port with given
 *  capabilities
 *
 *  Return: NULL - Port not fouund
 *          Otherwise - Pointer to ispif sink port
 **/
iface_stream_t *iface_util_find_stream_in_sink_port_list(
  iface_t  *iface,
  uint32_t  session_id,
  uint32_t  stream_id)
{
  mct_list_t *sink_port_list = NULL;
  mct_port_t *mct_port = NULL;
  iface_port_t *iface_port = NULL;
  uint32_t identity = pack_identity(session_id, stream_id);
  iface_stream_t *iface_stream = NULL;

  /*find iface sink port match the identity*/
  sink_port_list = mct_list_find_custom(iface->module->sinkports,
    (void *)&identity, iface_util_compare_identity_sink_port);

  if (sink_port_list != NULL) {
    mct_port = (mct_port_t *)sink_port_list->data;
    iface_port = (iface_port_t *)mct_port->port_private;
    CDBG("%s: old sink port used, iface_port = %p\n", __func__, iface_port);
  }

  if (iface_port != NULL) {
     iface_stream =
       iface_util_find_stream_in_sink_port(iface_port, session_id, stream_id);
  }

  return iface_stream;
}

/** iface_util_get_session_by_id:
 *    @ispif: ispif pointer
 *    @session_id: Session ID
 *
 *  This function runs in MCTL thread context.
 *
 *  This function finds session with given ID
 *
 *  Return: NULL - Session not found
 *          Otherwise - pointer to session
 **/
iface_session_t *iface_util_get_session_by_id(iface_t *iface,
  uint32_t session_id)
{
  int i;

  for (i = 0; i < IFACE_MAX_SESSIONS; i++) {
    if (iface->sessions[i].session_id == session_id &&
        iface->sessions[i].iface) {
      CDBG("%s: session id = %d found!!!!!\n", __func__, session_id);
      return &iface->sessions[i];
    }
  }

  CDBG("%s:X, no session found!!!!\n", __func__);
  return NULL;
}

/** iface_util_get_stats_stream_id:
 *
 *  @stats_type: which kind of stats
 *  @is_offline: offline/online stats
 *
 *  Return: void
 *
 **/
static uint32_t iface_util_get_stats_stream_id(
  enum msm_isp_stats_type stats_type, boolean is_offline)
{
  uint32_t stats_stream_id;
  if (is_offline) {
    stats_stream_id = stats_type | ISP_NATIVE_BUF_BIT | ISP_STATS_STREAM_BIT |
      ISP_OFFLINE_STATS_BIT;
  } else {
    stats_stream_id = stats_type | ISP_NATIVE_BUF_BIT | ISP_STATS_STREAM_BIT;
  }
  return stats_stream_id;
}

static boolean iface_util_is_mipi_stream(
  cam_format_t fmt)
{
  switch(fmt) {

    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
    case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GBRG:
    case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GRBG:
    case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_RGGB:
    case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_BGGR:
    case CAM_FORMAT_JPEG_RAW_8BIT:
    case CAM_FORMAT_META_RAW_8BIT:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GBRG:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GRBG:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_RGGB:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_BGGR:
      return TRUE;
      break;

    default:
      return FALSE;
      break;
  }
}

/** iface_util_choose_isp_interface:
 *    @ispif: ispif pointer
 *    @ispif_sink_port: ispif sink port
 *    @session: ispif session
 *    @stream: ispif stream
 *
 *  This function runs in MCTL thread context.
 *
 *  This function determines ISP output interface for given stream
 *
 *  Return: None
 **/
void iface_util_choose_isp_interface(
  iface_t        *ispif __unused,
  iface_port_t   *iface_port,
  iface_stream_t *stream)
{
  sensor_src_port_cap_t *sensor_cap = &iface_port->u.sink_port.sensor_cap;
  uint32_t primary_cid_idx = 0;
  uint32_t is_mipi_stream = 0;


  CDBG("%s: fmt %d\n", __func__, stream->stream_info.fmt);
  is_mipi_stream = iface_util_is_mipi_stream(stream->stream_info.fmt);

  if ((stream->stream_info.stream_type == CAM_STREAM_TYPE_RAW &&
      is_mipi_stream) ||
    iface_util_sensor_is_yuv(&iface_port->u.sink_port))
  {
    /* For MIPI RAW format or YUV sensor or Depth senosr, use RDI
     * interface to dump the data directly*/
    stream->use_pix = 0;
    CDBG("%s: use RDI\n", __func__);
  } else if ((stream->stream_info.fmt >= CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GBRG &&
              stream->stream_info.fmt <= CAM_FORMAT_BAYER_QCOM_RAW_12BPP_BGGR) ||
              (stream->stream_info.fmt >= CAM_FORMAT_BAYER_QCOM_RAW_14BPP_GBRG &&
              stream->stream_info.fmt <= CAM_FORMAT_BAYER_QCOM_RAW_14BPP_BGGR)){
    stream->use_pix = 0;
    stream->axi_path = CAMIF_RAW;
    CDBG("%s: CAMIF_RAW\n", __func__);
  } else if ((stream->stream_info.fmt >=
    CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GBRG &&
             stream->stream_info.fmt <=
    CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_BGGR) ||
    (stream->stream_info.fmt >=
    CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_14BPP_GBRG &&
              stream->stream_info.fmt <=
    CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_BGGR)) {
    stream->use_pix = 0;
    stream->axi_path = IDEAL_RAW;
    CDBG("%s: IDEAL_RAW\n", __func__);
  } else if (stream->stream_info.stream_type == CAM_STREAM_TYPE_PARM) {
    stream->use_pix = 0;
    CDBG("%s:%d DO NOT use PIX\n", __func__, __LINE__);
  } else if (stream->stream_info.fmt !=
    sensor_cap->sensor_cid_ch[primary_cid_idx].fmt) {
    /* sensor output format does not match stream fmt. We need pix or PP.
     * By default ispif assign it to use vfe pix output.
     * Another rule is to push all non bayer camera to use RDI.
     * For specific OEM customozation is needed.
     * For Google stock solution we do not implement the case that different.
     * color plane using different CIDs. That can be achieved by using      .
     * sensor_cid_ch[1]. This is not implemented in this phase. */
    stream->use_pix = 1;
    CDBG("%s: use PIX\n", __func__);
  }
}


static int iface_util_request_isp_output_resource(iface_t *iface,
  iface_resource_request_t *output_resource_info,
  uint32_t session_id, uint32_t stream_id, boolean offline)
{
  int             rc = 0;
  iface_stream_t *stream = NULL;
  mct_port_t     *isp_sink_port = NULL;
  mct_event_t     event;

  if (!iface || !output_resource_info) {
    CDBG_ERROR("%s:%d iface %p output_resource_info %p\n", __func__, __LINE__,
      iface, output_resource_info);
    return -1;
  }

  stream =
    iface_util_find_stream_in_sink_port_list(iface, session_id, stream_id);
  if (!stream) {
    CDBG_ERROR("%s:%d stream %p\n", __func__, __LINE__, stream);
    return -1;
  }

  if (!stream->src_port) {
    CDBG_ERROR("%s:%d stream->src_port %p\n", __func__, __LINE__,
      stream->src_port);
    return -1;
  }

  isp_sink_port = stream->src_port->peer;
  if (!isp_sink_port) {
    CDBG_ERROR("%s:%d isp_sink_port %p\n", __func__, __LINE__, isp_sink_port);
    return 0;
  }

  memset(&event, 0, sizeof(mct_event_t));

  event.type = MCT_EVENT_MODULE_EVENT;
  if (offline == FALSE) {
    event.u.module_event.type = MCT_EVENT_MODULE_IFACE_REQUEST_OUTPUT_RESOURCE;
  } else {
    event.u.module_event.type =
      MCT_EVENT_MODULE_IFACE_REQUEST_OFFLINE_OUTPUT_RESOURCE;
  }
  event.u.module_event.module_event_data = (void *)output_resource_info;
  event.identity = pack_identity(session_id, stream_id);
  event.direction = MCT_EVENT_DOWNSTREAM;

  if (FALSE == isp_sink_port->event_func(isp_sink_port, &event)) {
    rc = -1;
    CDBG_ERROR("%s: error in isp sink port event\n", __func__);
  }

  return rc;

}

/** iface_util_request_stream_mapping_info:
 *
 *    @iface: iface_handle
 *    @buf_alloc_info: native buf alloc info
 *    @session_id: session identity
 *    @stream_id: stream identity
 *
 *    requests the current stream mapping info
 *    return int
 **/
int iface_util_request_stream_mapping_info(iface_t *iface,
  iface_buf_alloc_t *buf_alloc_info,
  uint32_t session_id, uint32_t stream_id)
{
  int             rc = 0;
  iface_stream_t *stream = NULL;
  mct_port_t     *isp_sink_port = NULL;
  mct_event_t     event;

  if (!iface || !buf_alloc_info) {
    CDBG_ERROR("%s:%d iface %p buf_alloc_info %p\n", __func__, __LINE__,
      iface, buf_alloc_info);
    return -1;
  }

  stream =
    iface_util_find_stream_in_sink_port_list(iface, session_id, stream_id);
  if (!stream) {
    CDBG_ERROR("%s:%d stream %p\n", __func__, __LINE__, stream);
    return -1;
  }

  memset(&event, 0, sizeof(mct_event_t));
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_IFACE_REQUEST_STREAM_MAPPING_INFO;
  event.u.module_event.module_event_data = (void *)buf_alloc_info;
  event.identity = pack_identity(session_id, stream_id);
  event.direction = MCT_EVENT_DOWNSTREAM;

  /* Broadcast event to upstream event */
  rc = mct_port_send_event_to_peer(stream->src_port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d post module msg error\n", __func__, __LINE__);
    return -1;
  }

  return 0;
}

/** iface_util_request_pp_meta_stream_info:
 *
 *    @iface: iface_handle
 *    @pp_stream_info: additional buf need and adsp heap details
 *    @session_id: session identity
 *    @stream_id: stream identity
 *
 *    requests the current stream mapping info
 *    return int
 **/
int iface_util_request_pp_meta_stream_info(iface_t *iface,
  pp_meta_stream_info_request_t *pp_stream_info,
  uint32_t session_id, uint32_t stream_id)
{
  int             rc = 0;
  iface_stream_t *stream = NULL;
  mct_port_t     *isp_sink_port = NULL;
  mct_event_t     event;

  if (!iface || !pp_stream_info) {
    IFACE_ERR("iface %p pp_stream_info %p\n", iface, pp_stream_info);
    return -1;
  }

  stream =
    iface_util_find_stream_in_sink_port_list(iface, session_id, stream_id);
  if (!stream) {
    CDBG_ERROR("%s:%d stream %p\n", __func__, __LINE__, stream);
    return -1;
  }

  memset(&event, 0, sizeof(mct_event_t));
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_IFACE_REQUEST_META_STREAM_INFO;
  event.u.module_event.module_event_data = (void *)pp_stream_info;
  event.identity = pack_identity(session_id, stream_id);
  event.direction = MCT_EVENT_DOWNSTREAM;

  /* Broadcast event to upstream event */
  rc = mct_port_send_event_to_peer(stream->src_port, &event);
  if (rc == FALSE) {
    IFACE_ERR("post module msg error\n");
    return -1;
  }

  return 0;
}


/** iface_util_send_request_pp_divert:
 *
 *    @iface: iface_handle
 *    @session_id: session identity
 *    @stream_id: stream identity
 *
 *    Clear and initialize planes info
 *    return int
 **/
static int iface_util_send_request_pp_divert(iface_t *iface,
  uint32_t session_id, uint32_t stream_id,
  pp_buf_divert_request_t *pp_divert_request)
{
  int            rc = 0;
  iface_stream_t *stream = NULL;
  mct_port_t     *isp_sink_port = NULL;
  mct_event_t     event;


  if (!iface) {
    CDBG_ERROR("%s:%d iface %p\n", __func__, __LINE__,
      iface);
    return -1;
  }

  stream =
    iface_util_find_stream_in_sink_port_list(iface, session_id, stream_id);
  if (!stream) {
    CDBG_ERROR("%s:%d stream %p\n", __func__, __LINE__, stream);
    return -1;
  }

  if (!stream->src_port) {
    CDBG_ERROR("%s:%d stream->src_port %p\n", __func__, __LINE__,
      stream->src_port);
    return -1;
  }

  isp_sink_port = stream->src_port->peer;
  if (!isp_sink_port) {
    CDBG_ERROR("%s:%d isp_sink_port %p\n", __func__, __LINE__, isp_sink_port);
    return -1;
  }

  memset(&event, 0, sizeof(mct_event_t));
  pp_divert_request->num_additional_buffers = 0;

  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_IFACE_REQUEST_PP_DIVERT;
  event.u.module_event.module_event_data = (void *)pp_divert_request;
  event.identity = pack_identity(session_id, stream_id);
  event.direction = MCT_EVENT_DOWNSTREAM;

  if (FALSE == isp_sink_port->event_func(isp_sink_port, &event)) {
    CDBG_ERROR("%s: divert request failed\n", __func__);
  }

  return rc;

}

/** iface_util_calc_cds_trigger:
 *
 *    @iface: iface_handle
 *    @session: session
 *    @stats_udpate: 3A decision
 *    @cds_enable: CDS enable flag
 *
 *    calculate CDS trigger to decide if do cds update
 *    return int
 **/
iface_cds_status_t iface_util_calc_cds_trigger(iface_t *iface,
  iface_session_t *session, stats_update_t *stats_update,
  uint32_t *cds_enable)
{
  iface_cds_status_t    cds_status = IFACE_CDS_STATUS_IDLE;
  chromatix_parms_type *chromatix_ptr = NULL;
  chromatix_CDS_type   *chromatix_cds_type = NULL;
  float                 current_trigger = 0.0;
  float                 lowlight_start = 0.0;
  float                 lowlight_end = 0.0;

  if (!session || !iface || !stats_update || !cds_enable) {
    CDBG_ERROR("%s: NULL pointer, no cds update!"
      "session %p, iface %p, stats_update %p, cds_enable %p\n",
      __func__, session, iface, stats_update, cds_enable);
    return FALSE;
  }

  chromatix_ptr =
    (chromatix_parms_type *)session->chromatix_param.chromatixPtr;

  if (!chromatix_ptr) {
    CDBG_ERROR("%s: NULL pointer, no cds update! chromatix_ptr %p\n",
      __func__, chromatix_ptr);
    return FALSE;
  }

  chromatix_cds_type = &chromatix_ptr->
    chromatix_post_processing.chromatix_chroma_sub_sampling;

  /*0 - lux based, 1- gain based*/
  if (chromatix_cds_type->control_CDS == 0) {
    current_trigger = stats_update->aec_update.lux_idx;
    lowlight_start = chromatix_cds_type->cds_trigger.lux_index_start;
    lowlight_end = chromatix_cds_type->cds_trigger.lux_index_end;
  } else if (chromatix_cds_type->control_CDS == 1) {
    current_trigger = stats_update->aec_update.real_gain;
    lowlight_start = chromatix_cds_type->cds_trigger.gain_start;
    lowlight_end = chromatix_cds_type->cds_trigger.gain_end;
  } else {
    CDBG_ERROR("%s: CDS control type not supported\n", __func__);
    return FALSE;
  }

  /*Read UVsubsample trigger points from Chromatix header in the future
  Lux values: Min-----A-----B-----Max
  1. UV subsampling is ON  when lux idx > B
  2. UV subsampling is OFF when lux idx < A
  3. no change between A & B
  */
  switch (session->cds_mode) {
  case CAM_CDS_MODE_AUTO:
    if (current_trigger > lowlight_end &&
       (session->cds_curr_enb == 0)) {
      *cds_enable = 1;
      cds_status = IFACE_CDS_STATUS_UPDATE;
    } else if (current_trigger < lowlight_start &&
      (session->cds_curr_enb == 1)) {
      *cds_enable = 0;
      cds_status = IFACE_CDS_STATUS_UPDATE;
    } else {
      cds_status = IFACE_CDS_STATUS_IDLE;
    }
    break;
  case CAM_CDS_MODE_ON:
    if (session->cds_curr_enb == 0) {
      CDBG("%s:<cds_debug> HAL CDS mode on\n", __func__);
      *cds_enable = 1;
      cds_status = IFACE_CDS_STATUS_UPDATE;
    }

    break;
  case CAM_CDS_MODE_OFF:
    if (session->cds_curr_enb == 1) {
       CDBG("%s:<cds_debug> HAL CDS mode off\n", __func__);
      *cds_enable = 0;
      cds_status = IFACE_CDS_STATUS_UPDATE;
    }
    break;
  default:
    CDBG_ERROR("%s:<cds_debug> cds mode %d not supported\n", __func__, session->cds_mode);
    cds_status = IFACE_CDS_STATUS_IDLE;
    break;
  }

  if (cds_status == IFACE_CDS_STATUS_UPDATE) {
    CDBG_HIGH("%s:<cds_debug> take CDS! request enb = %d, curr enb = %d"
      " curr_triiger %f, lowlight_start %f, lowlight_end %f\n",
       __func__, *cds_enable, session->cds_curr_enb,
       current_trigger, lowlight_start, lowlight_end);
  }

  return cds_status;
}

/** iface_util_set_cds_mode
 *    @iface: iface handle
 *    @session_id: session id
 *    @cds_mode: camera cds mode
 *
 * Return: integer
 **/
int iface_util_set_cds_mode(iface_t *iface, uint32_t session_id,
  cam_cds_mode_type_t *cds_mode)
{
  iface_session_t *session = NULL;
  int              rc = 0;

  if (!iface || !cds_mode) {
    CDBG_ERROR("%s: null pointer iface %p, cds_mode %p rc = -1\n",
      __func__, iface, cds_mode);
    return -1;
  }

  session = iface_util_get_session_by_id(iface, session_id);
  if (session == NULL) {
    CDBG_ERROR("%s: can not find session, rc = -1\n", __func__);
    return -1;
  }

  if (*cds_mode >= CAM_CDS_MODE_MAX) {
    CDBG_ERROR("%s: invalid CDS mode %d\n", __func__, *cds_mode);
    return -1;
  }

  CDBG_HIGH("%s: set HAL CDS mode = %d\n", __func__, *cds_mode);
  session->cds_mode = *cds_mode;

  return rc;
}

/** iface_util_handle_bracketing_update
 *    @iface: iface handle
 *    @session_id: session id
 *    @cds_mode: camera cds mode
 *
 * Return: integer
 **/
int iface_util_handle_bracketing_update(iface_t *iface, iface_session_t *session,
  mct_bracketing_update_t *bracketing_update)
{
  int              rc = 0;

  if (!iface || !bracketing_update || !session) {
    CDBG_ERROR("%s: null pointer iface %p, bracketing_update %p session %p rc = -1\n",
      __func__, iface, bracketing_update, session);
    return -1;
  }

  session->bracketing_state = bracketing_update->state;

  rc = iface_util_set_frame_skip_all_stream(iface, session);

  return rc;
}

/** iface_util_decide_cds_update
 *
 *    @ctrl:
 *    @thread:
 *    @action:
 *
 **/
int iface_util_decide_cds_update(iface_t *iface, iface_session_t *session,
  uint32_t user_stream_id, stats_update_t *stats_update)
{
  int32_t i = 0;
  iface_hw_stream_t  *hw_stream = NULL;
  uint32_t           delay_numframes = 0;
  mct_event_t         event;
  boolean             ret = FALSE;
  iface_stream_t     *user_stream = NULL;


  if (!iface || !stats_update || !session) {
    CDBG_ERROR("%s: null pointer iface %p, stats_update %p, session %p\n",
      __func__, iface, stats_update, session);
    return -1;
  }

  /*configurable cds feature, also cds will be disabled when EIS2.0 enable*/
  if (session->cds_feature_enable == FALSE) {
    CDBG("%s: cds feature disabled, ignore CDS update\n", __func__);
    return 0;
  }

  if (session->skip_cds_timeout > 0 && (session->cds_skip_disable == FALSE)) {
    /* During preflashoff/ main flash off, there is aec jump expected which
    * Ideally HAL should control it using set_param but
    * this is protection in backend. */
    session->skip_cds_timeout--;
    CDBG_HIGH("%s: Skip CDS update %d\n", __func__, session->skip_cds_timeout);
    return 0;
  }

  user_stream = iface_util_find_stream_in_sink_port_list(iface, session->session_id,
    user_stream_id);
  if (!user_stream) {
    CDBG_ERROR("%s: cannot find user_stream, session_id = %d, stream_id =  %x",
      __func__, session->session_id, user_stream_id);
    return -1;
  }

  /* Chroma Down Sampling
    1.give up when thread busy, start pending(liveshot), axi updating
    2.axi updating: after UV_SS,
      session_thread will be blocked by busy cnt = 2. update in sof notify*/
  pthread_mutex_lock(&session->session_thread.busy_mutex);

  /*thread busy, cds updating(isp or iface), drop CDS*/
  if (session->session_thread.thread_busy == TRUE ||
      session->cds_status != IFACE_CDS_STATUS_IDLE) {
    CDBG("%s: cds_status %d, thread busy %d! drop CDS!\n", __func__,
       session->cds_status, session->session_thread.thread_busy);
    pthread_mutex_unlock(&session->session_thread.busy_mutex);
    return 0;
  }

  /*no stream active, drop CDS*/
  if (session->active_count == 0) {
    CDBG("%s: active acoutn = 0, ignore CDS upadte\n", __func__);
    pthread_mutex_unlock(&session->session_thread.busy_mutex);
    return 0;
  }

  /*LIVESHOT active, Drop CDS*/
  for (i = 0; i < IFACE_MAX_STREAMS; i++) {
    hw_stream = &session->hw_streams[i];
    if (hw_stream && hw_stream->hw_stream_id != 0 &&
        (hw_stream->state == IFACE_HW_STREAM_STATE_ACTIVE) &&
        (hw_stream->stream_info.num_burst > 0)) {
      CDBG_HIGH("%s: burst stream id %d ACTIVE! ignore AEC update\n",
        __func__, hw_stream->hw_stream_id);
      pthread_mutex_unlock(&session->session_thread.busy_mutex);
      return 0;
    }
  }

  /*fill in session based cds info*/
  session->cds_info.v_scale = 2;
  session->cds_info.w_scale = 2;

  /*after all the check, decide if doing CDS*/
  session->cds_status = iface_util_calc_cds_trigger(iface,
    session, stats_update, &session->cds_info.need_cds_subsample);
  if (session->cds_status == IFACE_CDS_STATUS_IDLE) {
    pthread_mutex_unlock(&session->session_thread.busy_mutex);
    return 0;
  }

  pthread_mutex_unlock(&session->session_thread.busy_mutex);

  delay_numframes = 1;
  memset(&event, 0, sizeof(mct_event_t));
  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = pack_identity(session->session_id, user_stream_id);
  event.direction =  MCT_EVENT_UPSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_DELAY_FRAME_SETTING;
  event.u.module_event.current_frame_id = session->sof_frame_id;
  event.u.module_event.module_event_data = &delay_numframes;

  /* Broadcast event to upstream event */
  ret = mct_port_send_event_to_peer(user_stream->sink_port, &event);
  if (ret == FALSE) {
    CDBG_ERROR("%s:%d post module msg error\n", __func__, __LINE__);
    return -1;
  }

  event.direction = MCT_EVENT_DOWNSTREAM;
  /* Broadcast event to downstream event */
  ret = mct_port_send_event_to_peer(user_stream->sink_port, &event);
  if (ret == FALSE) {
    CDBG_ERROR("%s:%d post module msg error\n", __func__, __LINE__);
    return -1;
  }

  return 0;
}

/** iface_util_cds_request_done:
 *
 *    @iface: iface_handle
 *    @session_id: session identity
 *
 *    Clear and initialize planes info
 *    return int
 **/
int iface_util_cds_request_done(iface_t *iface, iface_session_t *session,
  isp_cds_request_t *isp_cds_request)
{
  int rc = 0;
  int32_t i = 0;
  iface_hw_stream_t *hw_stream = NULL;

  if (session == NULL) {
    CDBG_ERROR("%s: can not find stream, rc = -1\n", __func__);
    return -1;
  }

  IFACE_HIGH("<cds_dbg> isp cds updating, enable = %d\n",
     isp_cds_request->cds_request_info.need_cds_subsample);

  /*kernel will reject active stream update, so we will reject here*/
  pthread_mutex_lock(&session->session_thread.busy_mutex);
  /*thread busy, liveshot start pending, cds updating(isp or iface)*/
  if (session->session_thread.thread_busy == TRUE) {
    IFACE_DBG("<cds_dbg> session_thread busy. Notify ISP\n");
    pthread_mutex_unlock(&session->session_thread.busy_mutex);
    return -1;
  }

  /*LIVESHOT active*/
  for (i = 0; i < IFACE_MAX_STREAMS; i++) {
    hw_stream = &session->hw_streams[i];
    if (hw_stream && hw_stream->hw_stream_id != 0 &&
        (hw_stream->state == IFACE_HW_STREAM_STATE_ACTIVE) &&
        (hw_stream->stream_info.num_burst > 0)) {
      IFACE_HIGH("<cds_dbg> burst stream id %d ACTIVE! drop CDS!\n",
        hw_stream->hw_stream_id);
      pthread_mutex_unlock(&session->session_thread.busy_mutex);
      return -1;
    }
  }
  pthread_mutex_unlock(&session->session_thread.busy_mutex);

  if (session->cds_status == IFACE_CDS_STATUS_REQUST_SEND &&
     (session->cds_curr_enb != session->cds_info.need_cds_subsample)) {
     rc = iface_handle_cds_request_to_thread(iface, session,
       session->cds_info.need_cds_subsample);
  } else {
    if (session->cds_curr_enb == session->cds_info.need_cds_subsample) {
      IFACE_HIGH("<cds_dbg> CDS request current, unblock ISP hw update "
        "request enb = %d, curr enb = %d\n",
        session->cds_info.need_cds_subsample,
        session->cds_curr_enb);
      return 0;
    }
    CDBG_ERROR("%s: CDS updating error!, "
      "cds status = %d, request enb = %d, curr enb = %d\n",
      __func__, session->cds_status, session->cds_info.need_cds_subsample,
      session->cds_curr_enb);
    return -1;
  }

  return rc;
}

/** iface_util_request_isp_cds_update:
 *
 *    @iface: iface_handle
 *    @session_id: session identity
 *
 *    Clear and initialize planes info
 *    return int
 **/
int iface_util_request_isp_cds_update(iface_t *iface,
  iface_session_t *session)
{
  int                   rc = 0;
  int                   i = 0;
  iface_hw_stream_t     *hw_stream = NULL;
  iface_stream_t        *user_stream = NULL;
  mct_port_t            *isp_sink_port = NULL;
  mct_event_t            event;
  isp_cds_request_t      isp_cds_request;

  if (!iface || !session) {
    CDBG_ERROR("%s:%d iface %p, session %p\n", __func__, __LINE__,
      iface, session);
    return -1;
  }

  for (i = 0; i < IFACE_MAX_STREAMS; i++) {
    if (session->hw_streams[i].state == IFACE_HW_STREAM_STATE_ACTIVE) {
      hw_stream = &session->hw_streams[i];
      break;
    }
  }
  if (!hw_stream) {
    CDBG_ERROR("%s:%d can not find active  hw stream %p\n",
      __func__, __LINE__, hw_stream);
    return 0;
  }

  user_stream = iface_util_find_stream_in_sink_port_list(iface,
    session->session_id, hw_stream->mct_streamon_id);
  if (!user_stream || !user_stream->src_port) {
    CDBG_ERROR("%s:%d NULL pointer  rc = -1\n",
      __func__, __LINE__);
    return -1;
  }

  isp_sink_port = user_stream->src_port->peer;
  if (!isp_sink_port) {
    CDBG_ERROR("%s:%d isp_sink_port %p rc = -1\n", __func__, __LINE__, isp_sink_port);
    return -1;
  }

  memset(&event, 0, sizeof(mct_event_t));
  memset(&isp_cds_request, 0 , sizeof(isp_cds_request_t));
  isp_cds_request.cds_request_info = session->cds_info;
  isp_cds_request.cds_request = TRUE;

  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_CDS_REQUEST;
  event.u.module_event.module_event_data = (void *)&isp_cds_request;
  event.identity = pack_identity(session->session_id, user_stream->stream_id);
  event.direction = MCT_EVENT_DOWNSTREAM;

  if (FALSE == isp_sink_port->event_func(isp_sink_port, &event)) {
    rc = -1;
    CDBG_ERROR("%s: error in isp sink port event\n", __func__);
  }

  return rc;

}
/** iface_util_fill_hw_stream_plane_info:
 *
 *    @planes: plane info
 *    @stream: hw stream
 *
 *    Clear and initialize planes info
 **/
static void iface_util_fill_hw_stream_plane_info(iface_hw_stream_t *hw_stream,
  cam_frame_len_offset_t *mct_plane_info)
{
  uint32_t i;
  iface_stream_t *iface_stream = NULL;

  memset(&hw_stream->plane_info, 0, sizeof(iface_plane_info_t));
  hw_stream->plane_info.num_planes =
    mct_plane_info->num_planes;

  for (i = 0; i < mct_plane_info->num_planes; i++) {
    if (i >= MAX_STREAM_PLANES) {
      CDBG_ERROR("%s:ERROR!  not support plane number %d\n",
        __func__, mct_plane_info->num_planes);
      break;
    }

    hw_stream->plane_info.strides[i] = mct_plane_info->mp[i].stride;
    hw_stream->plane_info.scanline[i] = mct_plane_info->mp[i].scanline;
    hw_stream->plane_info.addr_offset[i] = mct_plane_info->mp[i].offset;
    hw_stream->plane_info.addr_offset_h[i] = mct_plane_info->mp[i].offset_x;
    hw_stream->plane_info.addr_offset_v[i] = mct_plane_info->mp[i].offset_y;
    CDBG("%s: fmt= %d, plane idx= %d, wd= %d, stride= %d, scanline= %d\n",
      __func__, hw_stream->stream_info.fmt, i,
      hw_stream->stream_info.dim.width, mct_plane_info->mp[i].stride,
      mct_plane_info->mp[i].scanline);
  }
}

/** iface_util_fill_sensor_csid_infoto_hw:
 *
 *    @iface: iface_handle
 *    @session_id: session identity
 *
 *    Clear and initialize planes info
 *    return int
 **/
static int32_t iface_util_fill_sensor_csid_info_to_hw_by_format(iface_hw_stream_t *hw_stream,
  sensor_src_port_cap_t *sensor_caps, cam_format_t format, sensor_out_info_t *sensor_out_info)
{
  int32_t rc = -1;
  uint32_t k = 0;
  iface_output_pack_mode pack_mode = PACK_MODE_BYTE;

  if (!hw_stream || !sensor_caps) {
    CDBG_ERROR("%s: null pointer %p %p", __func__,
      hw_stream, sensor_caps);
    return rc;
  }

  hw_stream->num_cids = 0;
  if (format == CAM_FORMAT_META_RAW_10BIT) {
    for (k = 0; k < sensor_caps->num_meta_ch && k < MAX_META; k++) {
      if (sensor_caps->meta_ch[k].fmt == format) {
        hw_stream->csid = sensor_caps->meta_ch[k].csid;
        hw_stream->cids[0] = sensor_caps->meta_ch[k].cid;
        hw_stream->num_cids = 1;
        break;
      }
    }
  } else {
    hw_stream->is_stereo = sensor_caps->is_stereo_config;
    for (k = 0; k < sensor_caps->num_cid_ch && k < SENSOR_CID_CH_MAX; k++) {
      if (sensor_caps->sensor_cid_ch[k].fmt == format) {
        hw_stream->cids[hw_stream->num_cids] =
          sensor_caps->sensor_cid_ch[k].cid;
        hw_stream->csid =
          sensor_caps->sensor_cid_ch[k].csid;
        if (!hw_stream->use_pix) {
          pack_mode = iface_util_choose_pack_mode(sensor_caps->sensor_cid_ch[k].cid,
            sensor_caps->sensor_cid_ch[k].pack_mode, sensor_out_info);
          hw_stream->pack_mode[sensor_caps->sensor_cid_ch[k].cid] = pack_mode;
        } else
          hw_stream->pack_mode[sensor_caps->sensor_cid_ch[k].cid] =
            PACK_MODE_BYTE;
        if (sensor_caps->is_stereo_config) {
          hw_stream->right_cids[hw_stream->num_cids]  =
            sensor_caps->sensor_cid_ch[k].stereo_right_cid;
          hw_stream->right_csid =
            sensor_caps->sensor_cid_ch[k].stereo_right_csid;
          if (!hw_stream->use_pix) {
            pack_mode = iface_util_choose_pack_mode(sensor_caps->sensor_cid_ch[k].cid,
              sensor_caps->sensor_cid_ch[k].pack_mode, sensor_out_info);
            hw_stream->pack_mode[sensor_caps->sensor_cid_ch[k].cid] = pack_mode;
          } else
            hw_stream->pack_mode[hw_stream->right_cids[hw_stream->num_cids]] =
              PACK_MODE_BYTE;
        } else {
          hw_stream->right_cids[hw_stream->num_cids] = CID_MAX;
          hw_stream->right_csid = CSID_MAX;
        }
        hw_stream->num_cids++;

        rc = 0;
      }
    }
  }

  return rc;
}

/** iface_util_set_chromatix:
 *
 *    @iface: iface_handle
 *    @session_id: session identity
 *
 *    Clear and initialize planes info
 *    return int
 **/
int iface_util_set_chromatix(iface_t *iface, iface_session_t *session, uint32_t stream_id,
  modulesChromatix_t *chromatix_param)
{
  int rc = 0;
  int32_t i = 0;

  if (!iface || !chromatix_param || !session) {
    CDBG_ERROR("%s: NULL pointer iface %p, chromatix_param %p session %p\n",
      __func__, iface, chromatix_param, session);
    return 0;
  }

  session->chromatix_param = *chromatix_param;


  session->hvx.new_sof_params.chromatix_ptr =
    session->chromatix_param.chromatixPtr;
  session->hvx.new_sof_params.chromatix_comm_ptr =
    session->chromatix_param.chromatixComPtr;

  session->hvx.stub_lib_name = session->chromatix_param.external;

  /* if zzhdr HW block available and HVX is also asking to do
     zzHDR in HVX , ignore HVX zzHDR */

  if (session->zzhdr_hw_available && session->hvx.stub_lib_name &&
    !strncmp(session->hvx.stub_lib_name,"hvx_zzHDR", 9)){
    CDBG_ERROR("%s zzhdr_hw_available %d hdr_mode %d stub_lib_name %s ", __func__,
      session->zzhdr_hw_available, session->hdr_mode, session->hvx.stub_lib_name);
    return rc;
  }

  rc = iface_hvx_open(&iface->hvx_singleton, &session->hvx,
      &session->hvx.set_dim);
  if (rc < 0 || !session->hvx.enabled) {
    CDBG_ERROR("%s:%d failed: iface_hvx_open rc %d\n", __func__,
      __LINE__, rc);
    return 0;
  }

  if (session->hvx.enabled){
    rc = iface_hvx_send_module_event_isp(iface,session,stream_id);
    if ( rc < 0) {
      CDBG_ERROR("%s:%d failed: iface_hvx_send_module_event_isp rc %d\n", __func__,
      __LINE__, rc);
      return 0;
    }
  }


  return rc;
}

/** iface_util_update_plane_info_for_native_buf:
 *
 *    @hw_stream: hw stream handle
 *    @buf_alloc_info: native buffer allocation info
 *
 *    return int
 **/
static int iface_util_update_plane_info_for_native_buf(
  iface_hw_stream_t *hw_stream,
  iface_buf_alloc_t *buf_alloc_info)
{
  int      rc = -1;
  uint32_t i  = 0;
  cam_stream_buf_plane_info_t *buf_planes_out = &hw_stream->stream_info.buf_planes;

  if (hw_stream->stream_info.cam_stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    return rc;
  }
  memset(buf_planes_out, 0, sizeof(cam_stream_buf_plane_info_t));

  for (i = 0; i < buf_alloc_info->num_pix_stream
    && i < ISP_NUM_PIX_STREAM_MAX; i++) {
    if (buf_alloc_info->stream_info[i].use_native_buffer) {
      if ((hw_stream->axi_path == buf_alloc_info->stream_info[i].axi_stream_src) &&
        (buf_alloc_info->stream_info[i].buf_planes.plane_info.frame_len > 0) ) {
        memcpy(buf_planes_out, &buf_alloc_info->stream_info[i].buf_planes,
          sizeof(cam_stream_buf_plane_info_t));
        rc = 0;
        break;
      }
    }
  }

  return rc;
}

/** iface_util_modify_plane_info_for_native_buf
 *
 * DESCRIPTION:
 *
 **/
static int iface_util_modify_plane_info_for_native_buf(
  iface_hw_stream_t *hw_stream)
{
  cam_stream_buf_plane_info_t *buf_planes = &hw_stream->stream_info.buf_planes;
  cam_dimension_t *dim = &hw_stream->stream_info.dim;
  int32_t stride = 0, scanline = 0;
  int32_t stride_plane = 0, scanline_plane = 0;
  int rc = 0;
  uint32_t min_buf_padding = 0;

  if (hw_stream->native_buf_min_stride < (uint32_t)dim->width) {
    stride = dim->width;
  } else {
    stride = hw_stream->native_buf_min_stride;
  }

  if (hw_stream->native_buf_min_scanline < (uint32_t)dim->height) {
    scanline = dim->height;
  } else {
    scanline = hw_stream->native_buf_min_scanline;
  }

  memset(buf_planes, 0, sizeof(cam_stream_buf_plane_info_t));

  CDBG("%s: input fmt %d wxh = %d x %d",
    __func__, hw_stream->stream_info.fmt,
    dim->width, dim->height);
  switch (hw_stream->stream_info.fmt) {
  case CAM_FORMAT_YUV_420_NV12_VENUS:
  case CAM_FORMAT_YUV_420_NV21_VENUS:
  case CAM_FORMAT_YUV_420_NV12:
  case CAM_FORMAT_YUV_420_NV12_UBWC:
  case CAM_FORMAT_YUV_420_NV21: {
    min_buf_padding = AXI_BUF_ALIGHN_BYTE;
    if ((hw_stream->native_buf_alignment > min_buf_padding) &&
        (hw_stream->native_buf_alignment % min_buf_padding) == 0) {
      min_buf_padding = hw_stream->native_buf_alignment;
    }

    /* 2 planes: Y + CbCr */
    buf_planes->plane_info.num_planes = 2;
    /*Plane1: Y plane, offset need to be put evenly on left and right*/
    stride_plane =
      PAD_TO_SIZE(stride +
      (2 * hw_stream->axi_addr_offset_h), min_buf_padding);
    scanline_plane =
      PAD_TO_SIZE(scanline +
      (2 * hw_stream->axi_addr_offset_v), CAM_PAD_TO_2);

    buf_planes->plane_info.mp[0].offset =
      OFFSET_TO_ADDRESS(hw_stream->axi_addr_offset_h,
      hw_stream->axi_addr_offset_v, stride_plane);
    buf_planes->plane_info.mp[0].offset_x = hw_stream->axi_addr_offset_h;
    buf_planes->plane_info.mp[0].offset_y = hw_stream->axi_addr_offset_v;
    buf_planes->plane_info.mp[0].len = stride_plane * scanline_plane;
    buf_planes->plane_info.mp[0].stride = stride_plane;
    buf_planes->plane_info.mp[0].scanline = scanline_plane;

    /*Plane1: CbCr interleave plane, Y stride same as CbCr*/
    stride_plane =
      buf_planes->plane_info.mp[0].stride;
    scanline_plane =
      PAD_TO_SIZE((scanline / 2) + (2 * hw_stream->axi_addr_offset_v),
      CAM_PAD_TO_2);

    buf_planes->plane_info.mp[1].offset =
      OFFSET_TO_ADDRESS(hw_stream->axi_addr_offset_h,
      hw_stream->axi_addr_offset_v, stride_plane);
    buf_planes->plane_info.mp[1].offset_x = hw_stream->axi_addr_offset_h;
    buf_planes->plane_info.mp[1].offset_y = hw_stream->axi_addr_offset_v;
    buf_planes->plane_info.mp[1].len = stride_plane * scanline_plane;
    buf_planes->plane_info.mp[1].stride = stride_plane;
    buf_planes->plane_info.mp[1].scanline = scanline_plane;

    buf_planes->plane_info.frame_len =
      PAD_TO_SIZE(buf_planes->plane_info.mp[0].len +
      buf_planes->plane_info.mp[1].len,
      CAM_PAD_TO_4K);
  }
    break;

  case CAM_FORMAT_Y_ONLY:
  case CAM_FORMAT_Y_ONLY_10_BPP:
  case CAM_FORMAT_Y_ONLY_12_BPP: {
      min_buf_padding = CAM_PAD_TO_32;
      if ((hw_stream->native_buf_alignment > min_buf_padding) &&
          (hw_stream->native_buf_alignment % min_buf_padding) == 0 &&
          (hw_stream->native_buf_alignment <= 128)) {
        min_buf_padding = hw_stream->native_buf_alignment;
      }

      /* 1 plane Y only  */
      buf_planes->plane_info.num_planes = 1;
      stride = PAD_TO_SIZE(stride, min_buf_padding);
      scanline = PAD_TO_SIZE(scanline, CAM_PAD_TO_2);
      buf_planes->plane_info.mp[0].offset = 0;
      buf_planes->plane_info.mp[0].len = stride * scanline;
      buf_planes->plane_info.mp[0].stride = stride;
      buf_planes->plane_info.mp[0].scanline = scanline;
      buf_planes->plane_info.frame_len =
        PAD_TO_SIZE(buf_planes->plane_info.mp[0].len, CAM_PAD_TO_4K);
  }
  break;

  case CAM_FORMAT_YUV_444_NV24:
  case CAM_FORMAT_YUV_444_NV42:{
    min_buf_padding = CAM_PAD_TO_32;
    if ((hw_stream->native_buf_alignment > min_buf_padding) &&
        (hw_stream->native_buf_alignment % min_buf_padding) == 0 &&
        (hw_stream->native_buf_alignment <= 128)) {
      min_buf_padding = hw_stream->native_buf_alignment;
    }

    /* 2 planes: Y + CbCr */
    buf_planes->plane_info.num_planes = 2;
    stride = PAD_TO_SIZE(stride, min_buf_padding);
    scanline = PAD_TO_SIZE(scanline, CAM_PAD_TO_2);
    buf_planes->plane_info.mp[0].offset = 0;
    buf_planes->plane_info.mp[0].len = stride * scanline;
    buf_planes->plane_info.mp[0].stride = stride;
    buf_planes->plane_info.mp[0].scanline = scanline;

    stride *= 2;
    buf_planes->plane_info.mp[1].offset =
      buf_planes->plane_info.mp[0].len;
    buf_planes->plane_info.mp[1].len = stride * scanline;
    buf_planes->plane_info.mp[1].stride = stride;
    buf_planes->plane_info.mp[1].scanline = scanline;

    buf_planes->plane_info.frame_len =
      PAD_TO_SIZE(buf_planes->plane_info.mp[0].len +
      buf_planes->plane_info.mp[1].len, CAM_PAD_TO_4K);
  }
    break;

  case CAM_FORMAT_YUV_420_NV21_ADRENO: {
    /* 2 planes: Y + CbCr */
    min_buf_padding = AXI_BUF_ALIGHN_BYTE;
    if ((hw_stream->native_buf_alignment > min_buf_padding) &&
        (hw_stream->native_buf_alignment % min_buf_padding) == 0) {
      min_buf_padding = hw_stream->native_buf_alignment;
    }

    buf_planes->plane_info.num_planes = 2;

    /*Y plane*/
    stride_plane =
      PAD_TO_SIZE(stride + (2 * hw_stream->axi_addr_offset_h),
      min_buf_padding);
    scanline_plane =
      PAD_TO_SIZE(scanline + (2 * hw_stream->axi_addr_offset_v),
      min_buf_padding);

    buf_planes->plane_info.mp[0].offset =
      OFFSET_TO_ADDRESS(hw_stream->axi_addr_offset_h,
      hw_stream->axi_addr_offset_v, stride_plane);
    buf_planes->plane_info.mp[0].offset_x = hw_stream->axi_addr_offset_h;
    buf_planes->plane_info.mp[0].offset_y = hw_stream->axi_addr_offset_v;
    buf_planes->plane_info.mp[0].len =
      PAD_TO_SIZE(stride_plane * scanline_plane, CAM_PAD_TO_4K);
    buf_planes->plane_info.mp[0].stride = stride_plane;
    buf_planes->plane_info.mp[0].scanline = scanline_plane;

    /*CbCr plane*/
    stride_plane = PAD_TO_SIZE(stride / 2, min_buf_padding) * 2;
    scanline_plane = PAD_TO_SIZE(scanline / 2, min_buf_padding);
    buf_planes->plane_info.mp[1].offset =
      OFFSET_TO_ADDRESS(hw_stream->axi_addr_offset_h,
      hw_stream->axi_addr_offset_v, stride_plane);
    buf_planes->plane_info.mp[1].offset_x = hw_stream->axi_addr_offset_h;
    buf_planes->plane_info.mp[1].offset_y = hw_stream->axi_addr_offset_v;
    buf_planes->plane_info.mp[1].len =
      PAD_TO_SIZE(stride_plane * scanline_plane, CAM_PAD_TO_4K);
    buf_planes->plane_info.mp[1].stride = stride_plane;
    buf_planes->plane_info.mp[1].scanline = scanline_plane;

    buf_planes->plane_info.frame_len =
      PAD_TO_SIZE(buf_planes->plane_info.mp[0].len +
      buf_planes->plane_info.mp[1].len, CAM_PAD_TO_4K);
  }
    break;

  case CAM_FORMAT_YUV_420_YV12: {
    /* 3 planes: Y + Cr + Cb */
    min_buf_padding = CAM_PAD_TO_32;
    if ((hw_stream->native_buf_alignment > min_buf_padding) &&
        (hw_stream->native_buf_alignment % min_buf_padding) == 0) {
      min_buf_padding = hw_stream->native_buf_alignment;
    }
    buf_planes->plane_info.num_planes = 3;

    /*Y plane*/
    stride_plane =
      PAD_TO_SIZE(stride + (2 * hw_stream->axi_addr_offset_h),
      min_buf_padding);
    scanline_plane =
      PAD_TO_SIZE(scanline + (2 * hw_stream->axi_addr_offset_v),
      CAM_PAD_TO_2);

    buf_planes->plane_info.mp[0].offset =
      OFFSET_TO_ADDRESS(hw_stream->axi_addr_offset_h,
      hw_stream->axi_addr_offset_v, stride_plane);
    buf_planes->plane_info.mp[0].offset_x = hw_stream->axi_addr_offset_h;
    buf_planes->plane_info.mp[0].offset_y = hw_stream->axi_addr_offset_v;
    buf_planes->plane_info.mp[0].len = stride_plane * scanline_plane;
    buf_planes->plane_info.mp[0].stride = stride_plane;
    buf_planes->plane_info.mp[0].scanline = scanline_plane;

    /*Cb plane*/
    stride_plane =
      PAD_TO_SIZE(stride + (2 * hw_stream->axi_addr_offset_h),
      min_buf_padding);
    scanline_plane =
      (scanline / 2) + (2 * hw_stream->axi_addr_offset_v);

    buf_planes->plane_info.mp[1].offset =
      OFFSET_TO_ADDRESS(hw_stream->axi_addr_offset_h,
      hw_stream->axi_addr_offset_v, stride_plane);
    buf_planes->plane_info.mp[1].offset_x = hw_stream->axi_addr_offset_h;
    buf_planes->plane_info.mp[1].offset_y = hw_stream->axi_addr_offset_v;
    buf_planes->plane_info.mp[1].len = stride_plane * scanline_plane;
    buf_planes->plane_info.mp[1].stride = stride_plane;
    buf_planes->plane_info.mp[1].scanline = scanline_plane;

    /*Cr plane = Cb plane*/
    buf_planes->plane_info.mp[2].offset =
      OFFSET_TO_ADDRESS(hw_stream->axi_addr_offset_h,
      hw_stream->axi_addr_offset_v, stride_plane);
    buf_planes->plane_info.mp[2].offset_x = hw_stream->axi_addr_offset_h;
    buf_planes->plane_info.mp[2].offset_y = hw_stream->axi_addr_offset_v;
    buf_planes->plane_info.mp[2].len = stride_plane * scanline_plane;
    buf_planes->plane_info.mp[2].stride = stride_plane;
    buf_planes->plane_info.mp[2].scanline = scanline_plane;

    buf_planes->plane_info.frame_len =
      PAD_TO_SIZE(buf_planes->plane_info.mp[0].len +
      buf_planes->plane_info.mp[1].len +
      buf_planes->plane_info.mp[2].len, CAM_PAD_TO_4K);
  }
    break;

  case CAM_FORMAT_YUV_422_NV16:
  case CAM_FORMAT_YUV_422_NV61: {
    /* 2 planes: Y + CbCr */
    min_buf_padding = CAM_PAD_TO_32;
    if ((hw_stream->native_buf_alignment > min_buf_padding) &&
        (hw_stream->native_buf_alignment % min_buf_padding) == 0) {
      min_buf_padding = hw_stream->native_buf_alignment;
    }
    buf_planes->plane_info.num_planes = 2;

    /*Y plane*/
    stride_plane =
      PAD_TO_SIZE(stride + (2 * hw_stream->axi_addr_offset_h),
      min_buf_padding);
    scanline_plane =
      scanline + (2 * hw_stream->axi_addr_offset_v);

    buf_planes->plane_info.mp[0].offset =
      OFFSET_TO_ADDRESS(hw_stream->axi_addr_offset_h,
      hw_stream->axi_addr_offset_v, stride_plane);
    buf_planes->plane_info.mp[0].offset_x = hw_stream->axi_addr_offset_h;
    buf_planes->plane_info.mp[0].offset_y = hw_stream->axi_addr_offset_v;
    buf_planes->plane_info.mp[0].len = stride_plane * scanline_plane;
    buf_planes->plane_info.mp[0].stride = stride_plane;
    buf_planes->plane_info.mp[0].scanline = scanline_plane;

    /*CbCr plane = Y plane at 422*/
    buf_planes->plane_info.mp[1].offset =
      OFFSET_TO_ADDRESS(hw_stream->axi_addr_offset_h,
      hw_stream->axi_addr_offset_v, stride_plane);
    buf_planes->plane_info.mp[1].offset_x = hw_stream->axi_addr_offset_h;
    buf_planes->plane_info.mp[1].offset_y = hw_stream->axi_addr_offset_v;
    buf_planes->plane_info.mp[1].len = stride_plane * scanline_plane;
    buf_planes->plane_info.mp[1].stride = stride_plane;
    buf_planes->plane_info.mp[1].scanline = scanline_plane;

    buf_planes->plane_info.frame_len =
      PAD_TO_SIZE(buf_planes->plane_info.mp[0].len +
      buf_planes->plane_info.mp[1].len, CAM_PAD_TO_4K);
  }
    break;

    case CAM_FORMAT_YUV_RAW_8BIT_YUYV:
    case CAM_FORMAT_YUV_RAW_8BIT_YVYU:
    case CAM_FORMAT_YUV_RAW_8BIT_UYVY:
    case CAM_FORMAT_YUV_RAW_8BIT_VYUY:
    case CAM_FORMAT_JPEG_RAW_8BIT:
    case CAM_FORMAT_META_RAW_8BIT:
    case CAM_FORMAT_META_RAW_10BIT: {
      /* 1 plane */
      /* Every 32 pixels occupy 32 bytes */
      stride = PAD_TO_SIZE(stride, 32);
      buf_planes->plane_info.num_planes = 1;
      buf_planes->plane_info.mp[0].offset = 0;
      buf_planes->plane_info.mp[0].len =
        PAD_TO_SIZE(stride * scanline * 2, CAM_PAD_TO_4);
      buf_planes->plane_info.frame_len =
        PAD_TO_SIZE(buf_planes->plane_info.mp[0].len, CAM_PAD_TO_4K);
      buf_planes->plane_info.mp[0].offset_x =0;
      buf_planes->plane_info.mp[0].offset_y = 0;
      buf_planes->plane_info.mp[0].stride = stride;
      buf_planes->plane_info.mp[0].scanline = scanline;
    }
      break;

    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GBRG:
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GRBG:
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_RGGB:
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_BGGR:
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GREY:
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GREY:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GREY:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GREY:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_BGGR: {
      /* 1 plane */
      /* Every 32 pixels occupy 32 bytes */
      stride = PAD_TO_SIZE(stride, 32);
      buf_planes->plane_info.num_planes = 1;
      buf_planes->plane_info.mp[0].offset = 0;
      buf_planes->plane_info.mp[0].len =
        PAD_TO_SIZE(stride * scanline, CAM_PAD_TO_4);
      buf_planes->plane_info.frame_len =
        PAD_TO_SIZE(buf_planes->plane_info.mp[0].len, CAM_PAD_TO_4K);
      buf_planes->plane_info.mp[0].offset_x =0;
      buf_planes->plane_info.mp[0].offset_y = 0;
      buf_planes->plane_info.mp[0].stride = stride;
      buf_planes->plane_info.mp[0].scanline = scanline;
    }
      break;

    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GBRG:
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GRBG:
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_RGGB:
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_BGGR:
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GREY:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GREY: {
      /* Every 24 pixels occupy 32 bytes */
      stride = PAD_TO_SIZE(stride, 24);
      buf_planes->plane_info.num_planes = 1;
      buf_planes->plane_info.mp[0].offset = 0;
      buf_planes->plane_info.mp[0].len =
        PAD_TO_SIZE(stride * scanline * 8 / 6, CAM_PAD_TO_4);
      buf_planes->plane_info.frame_len =
        PAD_TO_SIZE(buf_planes->plane_info.mp[0].len, CAM_PAD_TO_4K);
      buf_planes->plane_info.mp[0].offset_x =0;
      buf_planes->plane_info.mp[0].offset_y = 0;
      buf_planes->plane_info.mp[0].stride = stride;
      buf_planes->plane_info.mp[0].scanline = scanline;
    }
      break;

    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GBRG:
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GRBG:
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_RGGB:
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_BGGR:
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GREY:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GREY: {
      /* Every 20 pixels occupy 32 bytes */
      stride = PAD_TO_SIZE(stride, 20);
      buf_planes->plane_info.num_planes = 1;
      buf_planes->plane_info.mp[0].offset = 0;
      buf_planes->plane_info.mp[0].len =
        PAD_TO_SIZE(stride * scanline * 8 / 5, CAM_PAD_TO_4);
      buf_planes->plane_info.frame_len =
        PAD_TO_SIZE(buf_planes->plane_info.mp[0].len, CAM_PAD_TO_4K);
      buf_planes->plane_info.mp[0].offset_x =0;
      buf_planes->plane_info.mp[0].offset_y = 0;
      buf_planes->plane_info.mp[0].stride = stride;
      buf_planes->plane_info.mp[0].scanline = scanline;
    }
      break;

    case CAM_FORMAT_BAYER_QCOM_RAW_14BPP_GBRG:
    case CAM_FORMAT_BAYER_QCOM_RAW_14BPP_GRBG:
    case CAM_FORMAT_BAYER_QCOM_RAW_14BPP_RGGB:
    case CAM_FORMAT_BAYER_QCOM_RAW_14BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_14BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_14BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_14BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_14BPP_BGGR:{
      CDBG_ERROR("14BPP RAW QCOM packaging");
      /* Every 16 pixels occupy 32 bytes */
      stride = PAD_TO_SIZE(stride, 16);
      buf_planes->plane_info.num_planes = 1;
      buf_planes->plane_info.mp[0].offset = 0;
      buf_planes->plane_info.mp[0].len =
        PAD_TO_SIZE(stride * scanline * 8 / 4, CAM_PAD_TO_4);
      buf_planes->plane_info.frame_len =
        PAD_TO_SIZE(buf_planes->plane_info.mp[0].len, CAM_PAD_TO_4K);
      buf_planes->plane_info.mp[0].offset_x =0;
      buf_planes->plane_info.mp[0].offset_y = 0;
      buf_planes->plane_info.mp[0].stride = stride;
      buf_planes->plane_info.mp[0].scanline = scanline;
    }
    break;

    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GREY:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GREY: {
      /* Every 128 pixels occupy 160 (= 32 x 5) bytes */
      stride = PAD_TO_SIZE(stride, 128);
      buf_planes->plane_info.num_planes = 1;
      buf_planes->plane_info.mp[0].offset = 0;
      buf_planes->plane_info.mp[0].len =
        PAD_TO_SIZE(stride * scanline * 5 / 4, CAM_PAD_TO_4);
      buf_planes->plane_info.frame_len =
        PAD_TO_SIZE(buf_planes->plane_info.mp[0].len, CAM_PAD_TO_4K);
      buf_planes->plane_info.mp[0].offset_x =0;
      buf_planes->plane_info.mp[0].offset_y = 0;
      buf_planes->plane_info.mp[0].stride = stride;
      buf_planes->plane_info.mp[0].scanline = scanline;
    }
      break;
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GREY:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GREY: {
      /* Every 64 pixels occupy 96 (= 32 x 3) bytes */
      stride = PAD_TO_SIZE(stride, 64);
      buf_planes->plane_info.num_planes = 1;
      buf_planes->plane_info.mp[0].offset = 0;
      buf_planes->plane_info.mp[0].len =
        PAD_TO_SIZE(stride * scanline * 3 / 2, CAM_PAD_TO_4);
      buf_planes->plane_info.frame_len =
        PAD_TO_SIZE(buf_planes->plane_info.mp[0].len, CAM_PAD_TO_4K);
      buf_planes->plane_info.mp[0].offset_x =0;
      buf_planes->plane_info.mp[0].offset_y = 0;
      buf_planes->plane_info.mp[0].stride = stride;
      buf_planes->plane_info.mp[0].scanline = scanline;
    }
      break;

    case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GBRG:
    case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GRBG:
    case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_RGGB:
    case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_14BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_14BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_14BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_14BPP_BGGR:{
      CDBG_ERROR("14BPP RAW MIPI packaging");
      /* Every 128 pixels occupy 224 (= 32 x 7) bytes */
      stride = PAD_TO_SIZE(stride, 128);
      buf_planes->plane_info.num_planes = 1;
      buf_planes->plane_info.mp[0].offset = 0;
      buf_planes->plane_info.mp[0].len =
        PAD_TO_SIZE(stride * scanline * 7 / 4, CAM_PAD_TO_4);
      buf_planes->plane_info.frame_len =
        PAD_TO_SIZE(buf_planes->plane_info.mp[0].len, CAM_PAD_TO_4K);
      buf_planes->plane_info.mp[0].offset_x =0;
      buf_planes->plane_info.mp[0].offset_y = 0;
      buf_planes->plane_info.mp[0].stride = stride;
      buf_planes->plane_info.mp[0].scanline = scanline;
    }
      break;

    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_BGGR:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GBRG:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GRBG:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_RGGB:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_BGGR: {
      /* Every 16 pixels occupy 32 bytes */
      stride = PAD_TO_SIZE(stride, 16);
      buf_planes->plane_info.num_planes = 1;
      buf_planes->plane_info.mp[0].offset = 0;
      buf_planes->plane_info.mp[0].len =
        PAD_TO_SIZE(stride * scanline * 2, CAM_PAD_TO_4);
      buf_planes->plane_info.frame_len =
        PAD_TO_SIZE(buf_planes->plane_info.mp[0].len, CAM_PAD_TO_4K);
      buf_planes->plane_info.mp[0].offset_x =0;
      buf_planes->plane_info.mp[0].offset_y = 0;
      buf_planes->plane_info.mp[0].stride = stride;
      buf_planes->plane_info.mp[0].scanline = scanline;
    }
      break;

  default:
    CDBG_WARN("%s: Invalid cam_format %d",
      __func__, hw_stream->stream_info.fmt);
    rc = -1;
    break;
  }

  return rc;
}

/** iface_util_calculate_frame_length_for_native_buf
 *
 * DESCRIPTION:
 *
 **/
int iface_util_calculate_frame_length_for_native_buf(cam_dimension_t *dim,
    cam_format_t fmt, cam_stream_buf_plane_info_t *buf_planes,
    uint32_t buf_alignment, uint32_t min_stride, uint32_t min_scanline)
{
  int rc = 0;
  //temporary hw stream, just to get plane info
  iface_hw_stream_t tmp_hw_stream;
  memset(&tmp_hw_stream, 0, sizeof(iface_hw_stream_t));

  //Populate the input values required for getting plane info
  memcpy(&tmp_hw_stream.stream_info.dim, dim, sizeof(cam_dimension_t));
  tmp_hw_stream.stream_info.fmt         = fmt;
  tmp_hw_stream.native_buf_min_stride   = min_stride;
  tmp_hw_stream.native_buf_min_scanline = min_scanline;
  tmp_hw_stream.native_buf_alignment    = buf_alignment;

  rc = iface_util_modify_plane_info_for_native_buf(&tmp_hw_stream);
  if (rc==0) {
    memcpy(buf_planes, &tmp_hw_stream.stream_info.buf_planes,
             sizeof(cam_stream_buf_plane_info_t));
  }
  else {
    CDBG_WARN("%s: returned fail=%d",__func__,rc);
  }

  return rc;
}

/** iface_util_prepare_pix_plane_info:
 *
 *    @planes: plane info
 *    @stream: hw stream
 *
 *    Clear and initialize planes info
 **/
static int iface_util_prepare_pix_plane_info(iface_session_t *session,
  iface_hw_stream_t *reserving_pix_stream, iface_port_t *iface_sink_port,
  isp_pix_out_info_t *isp_pix_output)
{
  int rc = 0;
  iface_stream_t *iface_stream = NULL;
  CDBG("%s: E\n", __func__);

  if (reserving_pix_stream->use_native_buf == 1) {
    rc = iface_util_update_plane_info_for_native_buf(reserving_pix_stream,
        &session->nativbuf_alloc_info);
     /*man made plane info*/
    if (rc != 0) {
      rc = iface_util_modify_plane_info_for_native_buf(reserving_pix_stream);
      if (rc < 0) {
        CDBG_ERROR("%s: modify_plane_info_for_native_buf error, rc = %d\n",
          __func__, rc);
        return rc;
      }
    }
    iface_util_fill_hw_stream_plane_info(reserving_pix_stream,
      &reserving_pix_stream->stream_info.buf_planes.plane_info);
  } else {
    /*mapped stream = 1, plane info comes from mct stream info*/
    iface_stream = iface_util_find_stream_in_sink_port(iface_sink_port,
      session->session_id, isp_pix_output->mapped_mct_stream_id[0]);
    if (iface_stream == NULL) {
      CDBG_ERROR("%s: stream not exist in this sinkport. error -1\n", __func__);
     return -1;
    }
    /*copy the plane info from MCt stream to HW stream*/
    reserving_pix_stream->stream_info.buf_planes.plane_info =
      iface_stream->stream_info.buf_planes.plane_info;
    iface_util_fill_hw_stream_plane_info(reserving_pix_stream,
      &iface_stream->stream_info.buf_planes.plane_info);
  }

  return 0;
}

/** iface_util_get_buffers_id:
 *    @hw_stream: output hw stream
 *    @stream_id: user stream id
 *
 *    Return id of buffer info per user streams for shared hw stream
 *
 *    Return buffer id or VFE_BUF_QUEUE_MAX if error occur.
 **/
int32_t iface_util_get_buffers_id(iface_hw_stream_t *hw_stream,
  uint32_t stream_id)
{
  uint32_t buff_id;

  if (!hw_stream->shared_hw_stream) {
    /* Use only one buffer queue if hw stream is not shared */
    return VFE_BUF_QUEUE_DEFAULT;
  }

  /* Search for buffer id by user stream id */
  for (buff_id = 0; buff_id < VFE_BUF_QUEUE_MAX; buff_id++) {
    if (stream_id == hw_stream->buffers_info[buff_id].user_stream_id) {
      return buff_id;
    }
  }

  /* if all entries are occupied return -1. */
  CDBG_ERROR("%s: can not find buffers id, stream_id %d rc = -1\n", __func__,
    stream_id);

  return VFE_BUF_QUEUE_MAX;
}

/** iface_util_update_hw_buf_info:
 *
 *    @planes: plane info
 *    @stream: hw stream
 *
 *    Clear and initialize planes info
 **/
int iface_util_update_hw_buf_info(iface_t *iface, iface_port_t *iface_sink_port,
  iface_session_t *session, uint32_t *user_stream_ids, uint32_t num_user_streams)
{
  int rc = 0;
  uint32_t i = 0;
  iface_hw_stream_t *hw_stream = NULL;
  iface_stream_t *user_stream = NULL;
  int32_t buffs_id;

  for (i = 0; i < num_user_streams; i++) {
    user_stream = iface_util_find_stream_in_sink_port(iface_sink_port,
      session->session_id, user_stream_ids[i]);
    if (user_stream == NULL) {
       CDBG_ERROR("%s: cannot find mct stream id = %d, rc = -1\n", __func__, user_stream_ids[i]);
       return -1;;
    }

    CDBG("%s:update user stream %d\n", __func__, user_stream->stream_id);

    hw_stream = iface_util_find_hw_stream_by_mct_id(iface, session,
      user_stream->stream_id);
    if (hw_stream == NULL) {
       CDBG_ERROR("%s: can not find hw stream for mct stream id %d, rc = -1\n",
         __func__, user_stream->stream_id);
       return -1;
    }

    if (hw_stream->shared_hw_stream) {
      /* Get first free buffer info holder */
      for (buffs_id = 0; buffs_id < VFE_BUF_QUEUE_MAX; buffs_id++) {
        if (!hw_stream->buffers_info[buffs_id].user_stream_id) {
          break;
        }
      }

      if (buffs_id >= VFE_BUF_QUEUE_MAX) {
        CDBG_ERROR("%s: can not find buffers id for user stream %d,"
                   "hw stream %d, rc = -1\n", __func__,
                   user_stream->stream_id, hw_stream->hw_stream_id);
        return -1;
      }
    } else {
      if (hw_stream->buffers_info[0].user_stream_id) {
        CDBG("%s: already mapped we just return here\n", __func__);
        continue;
      }
      buffs_id = 0;
    }

    CDBG("%s: update hw buf info with idx %d, buf list 0x%x, num bufs %d,"
         "hw stream id 0x%x, hw streamon id %d, user stream id %d",
         __func__, buffs_id, user_stream->stream_info.img_buffer_list,
         user_stream->stream_info.num_bufs,
         hw_stream->hw_stream_id,
         hw_stream->mct_streamon_id,
         user_stream->stream_id);

    hw_stream->buffers_info[buffs_id].img_buffer_list =
      user_stream->stream_info.img_buffer_list;
    hw_stream->buffers_info[buffs_id].num_bufs =
      user_stream->stream_info.num_bufs;
    hw_stream->buffers_info[buffs_id].user_stream_id = user_stream->stream_id;
  }

  return rc;
}

/** iface_util_decide_num_burst:
 *
 *    @planes: plane info
 *    @stream: hw stream
 *
 *    Clear and initialize planes info
 **/
static int iface_util_decide_num_burst(iface_session_t *session,
  iface_hw_stream_t *reserving_pix_stream, iface_port_t *iface_sink_port,
  isp_pix_out_info_t *isp_pix_output)
{
  iface_stream_t *iface_stream = NULL;
  uint32_t i = 0, num_burst = 0, mode_mask = 0;
  int rc = 0;
  CDBG("%s: E\n", __func__);
  /*fill in num burst in stream info*/
  for (i = 0; i < isp_pix_output->num_mct_stream_mapped
    && i < ISP_NUM_PIX_STREAM_MAX; i++) {
    iface_stream = iface_util_find_stream_in_sink_port(iface_sink_port,
      session->session_id, isp_pix_output->mapped_mct_stream_id[i]);
    if (iface_stream == NULL) {
      CDBG_ERROR("%s: can't find stream, sess id %d, stream id %d, rc = -1",
        __func__, session->session_id,
        isp_pix_output->mapped_mct_stream_id[i]);
      return -1;
    }

    mode_mask |= (1 << iface_stream->stream_info.streaming_mode);
    /* If Pproc has requested different streaming mode, update hw_stream*/
    if (reserving_pix_stream->stream_info.streaming_mode !=
      iface_stream->stream_info.streaming_mode) {
      mode_mask |= (1 << reserving_pix_stream->stream_info.streaming_mode);
    }
    if (reserving_pix_stream->stream_info.num_burst < iface_stream->stream_info.num_burst) {
      num_burst = iface_stream->stream_info.num_burst;
    }
  }

  if (mode_mask & (1 << CAM_STREAMING_MODE_CONTINUOUS)) {
    reserving_pix_stream->stream_info.num_burst = 0;
  } else {
    reserving_pix_stream->stream_info.num_burst = num_burst;
  }
  return rc;
}

/** iface_util_request_pp_divert_info:
 *
 *    @planes: plane info
 *    @stream: hw stream
 *
 *    Clear and initialize planes info
 **/
static int32_t iface_util_request_pp_divert_info(iface_t *iface,
  iface_session_t *session, isp_pix_out_info_t *isp_pix_output,
  iface_hw_stream_t *reserving_pix_stream)
{
  uint32_t i = 0;
  pp_buf_divert_request_t pp_divert_request;

  memset(&pp_divert_request, 0, sizeof(pp_buf_divert_request_t));

  /*ISP AXI buffer limit is 16 byte alignment*/
  reserving_pix_stream->native_buf_alignment = AXI_BUF_ALIGHN_BYTE;
  reserving_pix_stream->native_buf_use_adspheap = FALSE;

  /*send event downstream to decide need divert and alignment request*/
  for (i = 0; i < isp_pix_output->num_mct_stream_mapped
    && i < ISP_NUM_PIX_STREAM_MAX; i++) {
    pp_divert_request.force_streaming_mode = CAM_STREAMING_MODE_MAX;
    iface_util_send_request_pp_divert(iface, session->session_id,
      isp_pix_output->mapped_mct_stream_id[i], &pp_divert_request);
    if (pp_divert_request.need_divert == TRUE) {
      reserving_pix_stream->need_divert = 1;
      reserving_pix_stream->num_additional_native_buffers = MAX(
        reserving_pix_stream->num_additional_native_buffers,
        pp_divert_request.num_additional_buffers);
      if (((pp_divert_request.buf_alignment % AXI_BUF_ALIGHN_BYTE) == 0)) {
        reserving_pix_stream->native_buf_alignment =
          MAX(reserving_pix_stream->native_buf_alignment,
          pp_divert_request.buf_alignment);
      } else {
        CDBG_ERROR("%s: pp request buf alignment %d not %d bit align! skip!",
          __func__, pp_divert_request.buf_alignment, AXI_BUF_ALIGHN_BYTE);
        continue;
      }

      reserving_pix_stream->native_buf_min_stride =
        MAX(reserving_pix_stream->native_buf_min_stride,
        pp_divert_request.min_stride);
      reserving_pix_stream->native_buf_min_scanline =
        MAX(reserving_pix_stream->native_buf_min_scanline,
        pp_divert_request.min_scanline);
      /* use ADSP heap if any of the mapped stream needs it */
      reserving_pix_stream->native_buf_use_adspheap |=
        pp_divert_request.need_adsp_heap;

      /* will go over boundary if do this, need to adjust */
      reserving_pix_stream->axi_addr_offset_h =
        MAX(reserving_pix_stream->axi_addr_offset_h,
        pp_divert_request.min_addr_offset_h);
      reserving_pix_stream->axi_addr_offset_v =
        MAX(reserving_pix_stream->axi_addr_offset_v,
        pp_divert_request.min_addr_offset_v);

      /* If Pproc has requested different streaming mode, update hw_stream*/
      if (pp_divert_request.force_streaming_mode != CAM_STREAMING_MODE_MAX &&
          reserving_pix_stream->stream_info.streaming_mode !=
          pp_divert_request.force_streaming_mode) {
        reserving_pix_stream->stream_info.streaming_mode =
          pp_divert_request.force_streaming_mode;
      }
    }
  }

  /* h offset need to be 32 byte alignmentt*/
  reserving_pix_stream->axi_addr_offset_h =
    PAD_TO_SIZE(reserving_pix_stream->axi_addr_offset_h,
    AXI_ADDR_ALIGHN_BYTE);
  CDBG("%s: ===== divert info ======== \n", __func__);
  CDBG("%s: hw stream id %x \n", __func__, reserving_pix_stream->hw_stream_id);
  CDBG("%s: num additional buffer %d\n", __func__,
    reserving_pix_stream->num_additional_native_buffers);
  CDBG("%s: address offset H: %d, V %d\n", __func__,
    reserving_pix_stream->axi_addr_offset_h, reserving_pix_stream->axi_addr_offset_v);
  CDBG("%s: pp foce streaming mode =  %d\n",
    __func__, pp_divert_request.force_streaming_mode);
  return 0;
}

static int iface_util_decide_zsl_mode(iface_session_t *session,
  iface_hw_stream_t *reserving_pix_stream, iface_port_t *iface_sink_port,
  isp_pix_out_info_t *isp_pix_output)
{
  iface_stream_t *iface_stream = NULL;
  uint32_t i = 0;
  int rc = 0;
  enum msm_vfe_frame_skip_pattern skip_pattern;

  rc = iface_util_decide_frame_skip_pattern(session,iface_sink_port,&skip_pattern);

  /*fill in num burst in stream info*/
  for (i = 0; i < isp_pix_output->num_mct_stream_mapped; i++) {
    iface_stream = iface_util_find_stream_in_sink_port(iface_sink_port,
      session->session_id, isp_pix_output->mapped_mct_stream_id[i]);
    if (iface_stream == NULL) {
      CDBG_ERROR("%s: can't find stream, sess id %d, stream id %d, rc = -1",
        __func__, session->session_id,
        isp_pix_output->mapped_mct_stream_id[i]);
      return -1;
    }

    if (iface_stream->stream_info.stream_type == CAM_STREAM_TYPE_SNAPSHOT
       && iface_stream->stream_info.streaming_mode == CAM_STREAMING_MODE_CONTINUOUS) {
       rc = 1;
       if ((session->bracketing_state == MCT_BRACKETING_STATE_INIT) ||
         (session->peer_session_id))
         reserving_pix_stream->frame_skip_pattern = NO_SKIP;
       else
         reserving_pix_stream->frame_skip_pattern =  skip_pattern;
       break;
    }

  }
  return rc;
}

/** iface_util_decide_frame_skip_pattern:
*
*   @ iface session info
*   @ skip pattern variable
*
*   return success or failure
**/

int iface_util_decide_frame_skip_pattern(iface_session_t *session,
iface_port_t *iface_sink_port,
 enum msm_vfe_frame_skip_pattern *skip_pattern)
{
  int rc = 0;
  uint32_t max_sensor_fps = 0;
  uint32_t max_fps = 0;
  int no_of_frames_skipped = 0;
  int i = 0, nlength = 0;

  *skip_pattern = NO_SKIP;

  max_sensor_fps = iface_sink_port->u.sink_port.sensor_out_info.vt_pixel_clk /
    (iface_sink_port->u.sink_port.sensor_out_info.ll_pck *
     iface_sink_port->u.sink_port.sensor_out_info.fl_lines);

  nlength = sizeof(iface_maxfps_hw_version)/sizeof(iface_util_maxfps_info_t);
  for (i = 0; i < nlength ; i++)
  {
    if (session->hw_version == iface_maxfps_hw_version[i].hw_version)
    {
      max_fps = iface_maxfps_hw_version[i].max_fps;
      if (max_sensor_fps > max_fps)
      {
        no_of_frames_skipped = (((float)max_sensor_fps/(float)max_fps)+0.5)-1;
      }
      break;
    }
  }

  switch (no_of_frames_skipped)
  {
    case EVERY_2FRAME:
      *skip_pattern = EVERY_2FRAME;
       break;
    case EVERY_3FRAME:
      *skip_pattern = EVERY_3FRAME;
       break;
    case EVERY_4FRAME:
      *skip_pattern = EVERY_4FRAME;
       break;
    case EVERY_5FRAME:
      *skip_pattern = EVERY_5FRAME;
       break;
    case EVERY_6FRAME:
      *skip_pattern = EVERY_6FRAME;
       break;
    case EVERY_7FRAME:
      *skip_pattern = EVERY_7FRAME;
       break;
    case EVERY_8FRAME:
      *skip_pattern = EVERY_8FRAME;
       break;
    case EVERY_16FRAME:
      *skip_pattern = EVERY_16FRAME;
       break;
    case EVERY_32FRAME:
      *skip_pattern = EVERY_32FRAME;
       break;
    case NO_SKIP:
    default:
      *skip_pattern = NO_SKIP;
       break;
 }

 return rc;
}

/** iface_util_fill_hw_stream_info:
 *
 *    @planes: plane info
 *    @stream: hw stream
 *
 *    Clear and initialize planes info
 **/
static int32_t iface_util_fill_hw_stream_info_pix(
  iface_hw_stream_t  *reserving_pix_stream,
  iface_session_t    *session,
  iface_port_t       *iface_sink_port,
  isp_pix_out_info_t *isp_pix_output,
  uint32_t            isp_id_mask,
  uint8_t             initial_frame_skip,
  boolean             is_burst)
{
  uint32_t               i = 0;
  int32_t                rc = 0;
  uint32_t               k = 0;
  int32_t                j = 0;
  iface_stream_t        *mct_stream = NULL;
  iface_sink_port_t     *sink_port = NULL;
  boolean                need_divert = FALSE;
  iface_t               *iface = NULL;
  boolean                batch_mode = FALSE;
  mct_event_t            event;
  isp_cds_request_t      isp_cds_request;
  mct_port_t             *isp_sink_port = NULL;
  mct_bus_msg_iface_metadata_t iface_metadata;
  iface_hw_stream_t      *hw_stream = NULL;

  if (!reserving_pix_stream || !session || !iface_sink_port ||
      !isp_pix_output) {
    CDBG_ERROR("%s:%d %p %p %p %p\n", __func__, __LINE__, reserving_pix_stream,
      session, iface_sink_port, isp_pix_output);
    return -1;
  }

  sink_port = &iface_sink_port->u.sink_port;
  iface = (iface_t *)iface_sink_port->iface;

  mct_stream = iface_util_find_stream_in_sink_port(iface_sink_port,
    session->session_id, isp_pix_output->mapped_mct_stream_id[0]);
  if (mct_stream == NULL) {
    CDBG_ERROR("%s: can not find user stream id %d, sess %d\n, rc = -1",
      __func__, isp_pix_output->mapped_mct_stream_id[0], session->session_id);
    return -1;
  }

  if (isp_id_mask & (1 << VFE0))
    reserving_pix_stream->interface_mask |=
    (1 << (16 * VFE0 + IFACE_INTF_PIX));
  if (isp_id_mask & (1 << VFE1))
    reserving_pix_stream->interface_mask |=
    (1 << (16 * VFE1 + IFACE_INTF_PIX));

  /* fill in pix output streaminfo,
     mct mapped info, mapped stream id and mappen number of streams*/
  reserving_pix_stream->session_id = session->session_id;
  reserving_pix_stream->use_pix = 1;
  reserving_pix_stream->axi_path = isp_pix_output->axi_path;

  /*fill in stream info*/
  reserving_pix_stream->stream_info.dim = isp_pix_output->dim;
  reserving_pix_stream->stream_info.fmt = isp_pix_output->fmt;
  reserving_pix_stream->stream_info.streaming_mode =
    isp_pix_output->streaming_mode;
  reserving_pix_stream->native_buf_min_stride =
    CEILING32(isp_pix_output->dim.width);
  reserving_pix_stream->native_buf_min_scanline =
    CEILING32(isp_pix_output->dim.height);
  /* In fast aec force batch streams to non batch mode */
  if (session->fast_aec_mode &&
    reserving_pix_stream->stream_info.streaming_mode == CAM_STREAMING_MODE_BATCH)
    reserving_pix_stream->stream_info.streaming_mode =
      CAM_STREAMING_MODE_CONTINUOUS;
  batch_mode = (reserving_pix_stream->stream_info.streaming_mode ==
    CAM_STREAMING_MODE_BATCH);

  /* assign vt_enable */
  reserving_pix_stream->vt_enable = session->vt_enable;

  /*decide need pp divert,
    request for all the user stream mapped to this hw stream*/
  reserving_pix_stream->num_mct_stream_mapped =
    isp_pix_output->num_mct_stream_mapped;

  reserving_pix_stream->shared_hw_stream = isp_pix_output->shared_output;
  reserving_pix_stream->controllable_output =
    isp_pix_output->controlable_output;

  for (i = 0; i < isp_pix_output->num_mct_stream_mapped
    && i < IFACE_MAX_STREAMS; i++) {
    reserving_pix_stream->mapped_mct_stream_id[i] =
       isp_pix_output->mapped_mct_stream_id[i];
  }

  /* communicate with PP for PP limitation:
     1. min stride/scanline
     2. buffer alignment
     3. pp additional buffer request
     4. adsp buffer request
     5. minimum start address offset for WNR extra buffer processing */
  rc = iface_util_request_pp_divert_info(iface, session,
    isp_pix_output, reserving_pix_stream);
  if (rc < 0) {
    CDBG_ERROR("%s: iface_util_request_pp_divert_info fail,rc = %d\n",
      __func__, rc);
    return rc;
  }

  /* Convert yv12 to nv12 format. CPP will convert NV12 to YV12 before
   * sending it to HAL/app */
  if (reserving_pix_stream->stream_info.fmt == CAM_FORMAT_YUV_420_YV12) {
    reserving_pix_stream->stream_info.fmt = CAM_FORMAT_YUV_420_NV12;
    isp_pix_output->use_native_buffer = TRUE;
  }

  /*decide native buffer and also enforce divert whenver:
    1. isp output not match hal buffer size
    2. map more than 1 stream*/
  if ((isp_pix_output->num_mct_stream_mapped > 1 &&
      isp_pix_output->shared_output == 0) ||
      reserving_pix_stream->need_divert  ||
      iface_util_sensor_is_yuv(sink_port)) {
     reserving_pix_stream->use_native_buf = 1;
     reserving_pix_stream->hw_stream_id =
      isp_pix_output->mapped_mct_stream_id[0] | ISP_NATIVE_BUF_BIT;
     /*enforce divert when use native buf*/
     reserving_pix_stream->need_divert = 1;
     CDBG_HIGH("%s:#%d batch_dbg stream_id %d type %d streaming_mode %d\n",
       __func__, __LINE__, isp_pix_output->mapped_mct_stream_id[0],
       mct_stream->stream_info.stream_type,
       mct_stream->stream_info.streaming_mode);
     if (batch_mode) {
       int32_t batch_size = session->batch_info.batch_size;
       int32_t num_planes =
         reserving_pix_stream->stream_info.buf_planes.plane_info.num_planes;
       reserving_pix_stream->stream_info.batch_mode = TRUE;
       reserving_pix_stream->stream_info.batch_size = batch_size;
       session->batch_info.camif_period = batch_size;

       for (j = 0; j < num_planes && j < VIDEO_MAX_PLANES; j++) {
         reserving_pix_stream->stream_info.buf_planes.plane_info.mp[j].scanline
           *= batch_size;
       }
       for (i = 0; i < reserving_pix_stream->plane_info.num_planes
        && i < MAX_STREAM_PLANES; i++) {
         reserving_pix_stream->plane_info.scanline[i] *= batch_size;
       }
       reserving_pix_stream->stream_info.dim.height*= batch_size;
       mct_stream->stream_info.dim.height *= batch_size;
     }
  } else {
    reserving_pix_stream->use_native_buf = 0;
    reserving_pix_stream->hw_stream_id =
      isp_pix_output->mapped_mct_stream_id[0];
  }

  if (isp_pix_output->use_native_buffer == 1 &&
    reserving_pix_stream->need_divert == 0) {
    CDBG_ERROR("%s:failed, isp use native buf while pp not connected, rc = -1\n",
      __func__);
    return -1;
  }
#ifdef _ANDROID_
  int enabled = 0;
  char value[PROPERTY_VALUE_MAX];
  property_get("persist.vendor.camera.isp.ppbypass", value, "0");
  enabled = atoi(value);
  if (enabled == 1) {
    session->cds_feature_enable = FALSE;
    /*num_buf is the buffer number from HAL.
      when bypass PP, use hal buffer if HAL do provide buffer*/
    if (mct_stream->stream_info.num_bufs != 0) {
      CDBG_HIGH("%s:buf_dbg  hw stream %x, need divert %d, use native %d stream type %d\n",
        __func__, isp_pix_output->mapped_mct_stream_id[0],
        reserving_pix_stream->need_divert, reserving_pix_stream->need_divert,
        mct_stream->stream_info.stream_type);
      reserving_pix_stream->use_native_buf = 0;
      reserving_pix_stream->hw_stream_id =
        isp_pix_output->mapped_mct_stream_id[0];
      reserving_pix_stream->need_divert = 0;
    } else {
     reserving_pix_stream->use_native_buf = 1;
     reserving_pix_stream->hw_stream_id =
      isp_pix_output->mapped_mct_stream_id[0] | ISP_NATIVE_BUF_BIT;
     reserving_pix_stream->need_divert = 1;
    }
  }
#endif

  if ((!reserving_pix_stream->use_native_buf) &&
      ((mct_stream->stream_info.dim.width < isp_pix_output->dim.width) ||
       (mct_stream->stream_info.dim.height < isp_pix_output->dim.height))) {
    CDBG_ERROR("%s: we can not fit in output buffer!\n", __func__);
    return -1;
  }

  rc = iface_util_prepare_pix_plane_info(session, reserving_pix_stream,
    iface_sink_port, isp_pix_output);
  if (rc < 0) {
    CDBG_ERROR("%s: iface_util_prepare_plane_info error, rc = %d\n",
      __func__, rc);
    return rc;
  }

  /*if using HAL buffer, need to use offset from HAL plane info
    currently only support universal offset for all plane.
    since the requirement is from CPP. */
  if (!reserving_pix_stream->use_native_buf) {
    reserving_pix_stream->axi_addr_offset_h =
      mct_stream->stream_info.buf_planes.plane_info.mp[0].offset_x;
    reserving_pix_stream->axi_addr_offset_v =
      mct_stream->stream_info.buf_planes.plane_info.mp[0].offset_y;

    /* h offset need to be 32 byte alignmentt*/
    reserving_pix_stream->axi_addr_offset_h =
      PAD_TO_SIZE(reserving_pix_stream->axi_addr_offset_h,
      AXI_ADDR_ALIGHN_BYTE);
  }

  rc = iface_util_decide_num_burst(session, reserving_pix_stream,
    iface_sink_port, isp_pix_output);
  if (rc < 0) {
    CDBG_ERROR("%s: iface_util_decide_num_burst error, rc = %d\n",
      __func__, rc);
    return rc;
  }
 iface_util_decide_zsl_mode(session, reserving_pix_stream,
    iface_sink_port, isp_pix_output);
  /* get split info from ISP, need for axi configuration*/
  reserving_pix_stream->isp_split_output_info =
    isp_pix_output->isp_split_output_info;

  /*init stream config dont need uv subsample*/
  reserving_pix_stream->need_uv_subsample = 0;
  if ((session->cds_feature_enable) && ((session->cds_saved_state &&
    is_burst) || (session->cds_mode == CAM_CDS_MODE_ON))) {
    isp_sink_port = mct_stream->src_port->peer;
    if (isp_sink_port != NULL) {
      memset(&event, 0, sizeof(mct_event_t));
      memset(&isp_cds_request, 0 , sizeof(isp_cds_request_t));
      session->cds_info.need_cds_subsample = 1;
      session->cds_info.w_scale = 2;
      session->cds_info.v_scale = 2;
      isp_cds_request.cds_request_info = session->cds_info;
      isp_cds_request.cds_request = FALSE;

      event.type = MCT_EVENT_MODULE_EVENT;
      event.u.module_event.type = MCT_EVENT_MODULE_ISP_CDS_REQUEST;
      event.u.module_event.module_event_data = (void *)&isp_cds_request;
      event.identity = pack_identity(session->session_id, mct_stream->stream_id);
      event.direction = MCT_EVENT_DOWNSTREAM;
      if (isp_sink_port->event_func(isp_sink_port, &event)) {
        reserving_pix_stream->need_uv_subsample = 1;
        session->cds_curr_enb = 1;
        for (i = 0; i < IFACE_MAX_STREAMS; i++) {
          if (session->hw_streams[i].stream_info.dim.width != 0) {
            hw_stream = &session->hw_streams[i];
            hw_stream->need_uv_subsample = 1;
          }
        }
      }
      memset(&iface_metadata, 0, sizeof(iface_metadata));
      iface_metadata.is_uv_subsampled = reserving_pix_stream->need_uv_subsample;
      iface_metadata.type = MCT_BUS_MSG_IFACE_METADATA;
      iface_util_post_bus_msg(session, MCT_BUS_MSG_IFACE_METADATA,
        &iface_metadata, sizeof(iface_metadata));

    }

  }

  /* frame stream, metadata dont go through pix
   non meta data stream. */
  rc = iface_util_fill_sensor_csid_info_to_hw_by_format(reserving_pix_stream,
    &sink_port->sensor_cap, sink_port->sensor_out_info.fmt,
    &sink_port->sensor_out_info);
  if (rc < 0) {
    CDBG_ERROR("%s: iface_util_fill_sensor_csid_info_to_hw_by_format error, rc = %d\n",
      __func__, rc);
    return rc;
  }

  reserving_pix_stream->sensor_skip_cnt = session->bayer_processing ? 0 : initial_frame_skip;

  /* may have STAT and IMAGE, seperate*/
  reserving_pix_stream->stream_type = IFACE_STREAM_TYPE_IMAGE;

  /*we can use  session->hfr_param to skip to achieve preview 30fps,
    but to achieve video no skip with preview skip,
    have an agreement to use CPP SW skip, if we divert to cpp*/
  if (session->bracketing_state == MCT_BRACKETING_STATE_ON) {
    /* frame bracketing mode */
    CDBG_HIGH("%s: bracketing mode on, skip pattern %d!\n",
      __func__, session->bracketing_frame_skip_pattern);
    reserving_pix_stream->frame_skip_pattern =
      session->bracketing_frame_skip_pattern;
  } else {
    if (session->hal_skip_pattern != NO_SKIP) {
      CDBG_HIGH("%s: HAL skip pattern = %d, enforce!\n",
        __func__, session->hal_skip_pattern);
      reserving_pix_stream->frame_skip_pattern = session->hal_skip_pattern;
    } else {
      /* if link to PP, ISP will never do skip, its handled in PP*/
      if (reserving_pix_stream->need_divert)
        reserving_pix_stream->frame_skip_pattern = NO_SKIP;
      else if (session->hfr_param.hfr_mode)
        reserving_pix_stream->frame_skip_pattern =
          iface_util_get_hfr_skip_pattern(session);
    }
  }

  /* If any hw stream is in batch mode put rest streams in subsampling mode */
  if (session->fast_aec_mode) {
    reserving_pix_stream->frame_skip_pattern =
      iface_util_get_hfr_skip_pattern(session);
  } else if (session->batch_info.batch_mode && reserving_pix_stream->
    stream_info.streaming_mode != CAM_STREAMING_MODE_BATCH) {
    reserving_pix_stream->frame_skip_pattern =
        session->batch_info.batch_size - 1;
  }

  /*change the state for hw stream*/
  reserving_pix_stream->state = IFACE_HW_STREAM_STATE_RESERVED;

  return rc;
}

/** iface_util_dump_isp_resource_request
 *
 *    @iface: iface instance
 *    @isp_resource: mask to be filled according to the resource alloted
 *
 *  Reserve VFE 0 or 1 for RDI and set the mask bits accordingly
 *
  * Returns 0 on success.
 **/
static void iface_util_dump_isp_resource_request(iface_t *iface,
  iface_resource_request_t *isp_resource)
{
  uint32_t i = 0;

  if (!iface || !isp_resource) {
     return;
   }

  IFACE_HIGH(" =====DUMP RESOURCE: PIX STREAM=== \n");
  IFACE_HIGH("num pix stream = %d\n", isp_resource->num_pix_stream);
  IFACE_HIGH("num_isp = %d, isp_mask = %x, request_op_pix_clk = %d\n",
    isp_resource->num_isps, isp_resource->isp_id_mask,
    isp_resource->request_pix_op_clk);
  IFACE_HIGH("ispif stripe info: split %d, overlap %d, stripe offset %d\n",
    isp_resource->ispif_split_output_info.is_split,
    isp_resource->ispif_split_output_info.overlap,
    isp_resource->ispif_split_output_info.right_stripe_offset);
  for (i = 0; i < isp_resource->num_pix_stream
    && i < ISP_NUM_PIX_STREAM_MAX; i++) {
    IFACE_HIGH("pix stream %d\n", i);
    IFACE_HIGH("num mct stream mapped = %d(Mapped stream ID[0] = %d)\n",
      isp_resource->isp_pix_output[i].num_mct_stream_mapped,
      isp_resource->isp_pix_output[i].mapped_mct_stream_id[0]);
    IFACE_HIGH("axi path = %d(ENC-0/VIEW-1/CAMIF-2/IDEAL-3), cam_fmt %d\n",
     isp_resource->isp_pix_output[i].axi_path,
     isp_resource->isp_pix_output[i].fmt);
    IFACE_DBG("Width = %d, Height = %d\n",
      isp_resource->isp_pix_output[i].dim.width,
      isp_resource->isp_pix_output[i].dim.height);
    IFACE_DBG("split_info: is_split %d, right stripe offset = %d\n",
      isp_resource->isp_pix_output[i].isp_split_output_info.is_split,
      isp_resource->isp_pix_output[i].isp_split_output_info.right_stripe_offset
      );
    IFACE_DBG("split_info: left output = %d, right output = %d\n",
      isp_resource->isp_pix_output[i].isp_split_output_info.left_output_width,
      isp_resource->isp_pix_output[i].isp_split_output_info.right_output_width
      );
    IFACE_DBG("use native buffer: %d frame_skip %x\n",
      isp_resource->isp_pix_output[i].use_native_buffer,
      isp_resource->isp_pix_output[i].isp_frame_skip);

    IFACE_DBG("ISP CDS Capability %d\n",
        isp_resource->cds_capable[i]);
  }

  IFACE_DBG("=====DUMP RESOURCE:  STATS STREAM===\n");
  IFACE_HIGH("num STATS stream = %d\n",
    isp_resource->num_stats_stream);
  for (i = 0; i < isp_resource->num_stats_stream
    && i < MSM_ISP_STATS_MAX; i++) {
    IFACE_DBG("stats type %d(7-BG/8-Bf/9-BE), is composite %d \
      parse enable %d\n",
      isp_resource->isp_stats_info[i].stats_type,
      isp_resource->isp_stats_info[i].comp_flag,
      isp_resource->isp_stats_info[i].parse_flag);
    IFACE_DBG("buf type %d(0-priv/1-shared), num_buf = %d, buf len %d\n",
      isp_resource->isp_stats_info[i].buf_type,
      isp_resource->isp_stats_info[i].num_bufs,
      isp_resource->isp_stats_info[i].buf_len);
  }

}

/** iface_util_get_vfe_clk
 *
 *    @overlap: Dual VFE Overlap
 *    @sensor_out_info: Sensor output information
 *
  * Returns vfe_clk on success.
 **/

static uint64_t iface_util_get_vfe_clk(
  uint32_t num_sensor_meta,
  ispif_out_info_t *split_info,
  sensor_out_info_t *sensor_out_info,
  uint32_t isp_id)
{
  uint64_t act_pixels, vfe_clk, pixels_per_line;
  uint64_t ll_max, theoretical_line_length, min_line_length;

  if (split_info == NULL || sensor_out_info == NULL) {
    CDBG_ERROR(" Not Enough Information to Calculate Clock ");
    return 0;
  }
  if (split_info->is_split) {
    if (isp_id == VFE0){
      act_pixels = split_info->right_stripe_offset +
        split_info->overlap;
    } else if(isp_id == VFE1) {
      act_pixels = sensor_out_info->dim_output.width -
        split_info->right_stripe_offset;
    } else {
      CDBG_ERROR("Invalid ISP id");
      return 0;
    }
  } else {
    act_pixels = sensor_out_info->dim_output.width;
  }
  pixels_per_line = (CEILING32(act_pixels + REQUIRED_MIN_HBI));

 /*  We need to ensure that this clock rate is good enough.
       So we should also check against the theoretical width
       based on the op_pixel clock the vendor is recommending*/

  if (sensor_out_info->min_hbi) {
    theoretical_line_length = sensor_out_info->dim_output.width + sensor_out_info->min_hbi;

  } else {
    theoretical_line_length = sensor_out_info->op_pixel_clk /
            (sensor_out_info->orig_max_fps * sensor_out_info->fl_lines);
  }

  CDBG("%s: theoretical_line_length %d, actual pixels %llu, \
       required pixels_per_line %d\n",__func__,
       theoretical_line_length, act_pixels, pixels_per_line);

  /*  If the vendor is using some other (lower) ll_pck, we'll
         fall back to at least the active width and ensure that
         we're not overclocking by some large value*/
  ll_max = MAX(act_pixels, sensor_out_info->ll_pck);

  /*  Compare theoretical_line_length with ll_max. If the vendor
      did not provide the right length info, use the smaller of the two.
      In the worst case, if the vendor provided an very large value that is
      much larger than the active width (either due to an error or if there
      really is a large amount of blanking),
      we know we'll have sufficient time with just using op_pixel_clk/(active_width)
      - the width being inversely proportional to the clock,
      it will crank up the clock to a high enough value.*/

  min_line_length = MIN(ll_max, theoretical_line_length);
  if (min_line_length) {
    vfe_clk = pixels_per_line * sensor_out_info->op_pixel_clk /
               min_line_length;
  } else {
    vfe_clk = sensor_out_info->op_pixel_clk;
  }

  /* Make sure the vfe clock meets the minimum VBI requirements */
  if ((sensor_out_info->min_vbi != 0) &&
      (sensor_out_info->min_vbi < REQUIRED_MIN_VBI)){
    vfe_clk = vfe_clk * REQUIRED_MIN_VBI / sensor_out_info->min_vbi;
    CDBG_HIGH("%s: not enough VBI cycle %d\n",__func__,
      sensor_out_info->min_vbi);
  }

  { /* override to force ISP clock rate */
    char prop[255];
    int override = 0;
    property_get("persist.vendor.camera.dualcam.override", prop, "0");
    override = atoi(prop);
    if (override) {
      vfe_clk = MAX(vfe_clk, sensor_out_info->op_pixel_clk);
    }
  } /* override to force ISP clock rate */

  /* when PD data is enable disable clock optimization. Also in secure camera usecase
     PDAF is disabled in s/w. However, in the h/w the sensor is still providing the
     PDAF data to ISP. Hence disable optimization even in secure camera use case */
  if ((num_sensor_meta > 0 || sensor_out_info->is_secure) && (!split_info->is_split) &&
      (sensor_out_info->min_hbi == 0)) {
    vfe_clk = MAX(vfe_clk, sensor_out_info->op_pixel_clk);
    CDBG_HIGH("%s: force vfe_clk %d same as sensor op_clk\n",__func__,
      vfe_clk);
  }
  return vfe_clk;
}



/** iface_util_reserve_pix_resource
 *
 *    @iface: iface instance
 *    @session_id: current session id
 *    @isp_resource: mask to be filled according to the resource alloted
 *
 *  Reserve VFE 0 or 1 for RDI and set the mask bits accordingly
 *
  * Returns 0 on success.
 **/
static int iface_util_reserve_pix_resource(
  iface_t                  *iface,
  uint32_t                  session_id,
  iface_resource_request_t *isp_resource)
{
  int rc = 0;
  uint32_t i = 0;
  iface_session_t *iface_session = NULL;
  int request_wm = 0;

  CDBG("%s: E\n", __func__);
  iface_session = iface_util_get_session_by_id(iface, session_id);
  if (iface_session == NULL) {
     CDBG_ERROR("%s: can not find session, id = %d, rc = -1\n", __func__, session_id);
     return -1;
  }

  pthread_mutex_lock(&iface->mutex);

  /* early return when requested wm exceeds the HW limitation*/
  for (i = 0; i < isp_resource->num_pix_stream
    && i < ISP_NUM_PIX_STREAM_MAX; i++) {
    request_wm += iface_util_calc_num_plane(
      isp_resource->isp_pix_output[i].fmt);
  }
  if ((isp_resource->isp_id_mask & (1 << VFE0)) &&
    (iface->used_wm[VFE0] + request_wm > NUM_AXI_WM)) {
      pthread_mutex_unlock(&iface->mutex);
      return -1;
  }
  if ((isp_resource->isp_id_mask & (1 << VFE1)) &&
    (iface->used_wm[VFE1] + request_wm > NUM_AXI_WM)) {
      pthread_mutex_unlock(&iface->mutex);
      return -1;
  }

  /* save resource mask in session resource*/
  iface_session->session_resource.isp_id_mask = isp_resource->isp_id_mask;
  iface_session->session_resource.num_isps = isp_resource->num_isps;
  iface_session->session_resource.camif_cnt += iface_session->bayer_processing ? 0 : isp_resource->num_pix_stream;
  iface_session->session_resource.ispif_split_info = isp_resource->ispif_split_output_info;


  if (isp_resource->isp_id_mask & (1 << VFE0))
    iface_session->session_resource.used_resource_mask |= (1 << (16 * VFE0 + IFACE_INTF_PIX));
  if (isp_resource->isp_id_mask & (1 << VFE1))
    iface_session->session_resource.used_resource_mask |= (1 << (16 * VFE1 + IFACE_INTF_PIX));

  if (isp_resource->isp_id_mask & (1 << VFE0)) {
    iface_session->session_resource.used_wm[VFE0] += request_wm;
    iface->used_wm[VFE0] += request_wm;
  }
  if (isp_resource->isp_id_mask & (1 << VFE1)) {
    iface_session->session_resource.used_wm[VFE1] += request_wm;
    iface->used_wm[VFE1] += request_wm;
  }

  pthread_mutex_unlock(&iface->mutex);

  return rc;
}

/** iface_utill_fill_stats_stream_info
 *
 *  @session: session data pointer
 *  @isp_resource_request: isp stats info request
 *
 *  fill in stats stream info from requesting ISP
 *
 *  return integer rc
 **/
int iface_utill_fill_stats_stream_info(iface_session_t *session,
  iface_resource_request_t *isp_resource_request, iface_port_t *iface_sink_port,
  boolean is_offline)
{
  uint32_t                         i = 0;
  int                              rc = 0;
  iface_stats_stream_t            *stats_stream = NULL;
  iface_sink_port_t               *sink_port = NULL;
  enum msm_vfe_frame_skip_pattern  stats_skip_pattern = NO_SKIP;

  if (!session || !isp_resource_request || !iface_sink_port) {
    CDBG_ERROR("%s: NULL pointer! %p %p %p, rc = -1\n", __func__,
      session, isp_resource_request, iface_sink_port);
    return -1;
  }

  sink_port = &iface_sink_port->u.sink_port;
  /*we can use  session->hfr_param to skip to achieve preview 30fps,
    but to achieve video no skip with preview skip,
    have an agreement to use CPP SW skip, if we divert to cpp*/

  /*no skip on stats when fast aec mode*/
  if(session->fast_aec_mode) {
    stats_skip_pattern = NO_SKIP;
  } else {
    if (session->dynamic_stats_skip_feature_enb == FALSE) {
      switch (session->hfr_param.hfr_mode) {
      case CAM_HFR_MODE_60FPS:
        stats_skip_pattern = EVERY_2FRAME;
        break;

      case CAM_HFR_MODE_90FPS:
        stats_skip_pattern = EVERY_3FRAME;
        break;

      case CAM_HFR_MODE_120FPS:
        stats_skip_pattern = EVERY_4FRAME;
        break;

      case CAM_HFR_MODE_150FPS:
        stats_skip_pattern = EVERY_5FRAME;
        break;

      case CAM_HFR_MODE_180FPS:
        stats_skip_pattern = EVERY_6FRAME;
         break;

      case CAM_HFR_MODE_210FPS:
        stats_skip_pattern = EVERY_7FRAME;
         break;

      case CAM_HFR_MODE_240FPS:
        stats_skip_pattern = EVERY_8FRAME;
         break;

      case CAM_HFR_MODE_480FPS:
        stats_skip_pattern = EVERY_16FRAME;
         break;

      default:
        stats_skip_pattern = NO_SKIP;
        break;
      }
    } else {
      stats_skip_pattern = session->stats_skip_pattern;
    }
    if (session->batch_info.batch_mode)
      stats_skip_pattern = session->batch_info.batch_size - 1;
  }

  session->stats_skip_pattern = stats_skip_pattern;
  for (i = 0; i < isp_resource_request->num_stats_stream &&
    i < MSM_ISP_STATS_MAX; i++) {
    if (is_offline) {
      stats_stream = &session->offline_stats_streams[i];
    } else {
      stats_stream = &session->stats_streams[i];
    }
    stats_stream->session_id =
      isp_resource_request->isp_stats_info[i].session_id;
    stats_stream->stats_type =
      isp_resource_request->isp_stats_info[i].stats_type;
    stats_stream->comp_flag =
      isp_resource_request->isp_stats_info[i].comp_flag;
    stats_stream->buf_len =
      isp_resource_request->isp_stats_info[i].buf_len;
    stats_stream->num_bufs =
      isp_resource_request->isp_stats_info[i].num_bufs;
    stats_stream->pattern = stats_skip_pattern;

    /*SHARED BUF: dual vfe will share one mega buffer between vfe0 & 1*/
    stats_stream->buf_type = isp_resource_request->isp_stats_info[i].buf_type;

    stats_stream->stats_stream_id =
      iface_util_get_stats_stream_id(stats_stream->stats_type, is_offline);
    /*align the init stats drop with init frame drop*/
    if (is_offline || session->bayer_processing) {
      /*for offline stats set frame drop as 0 as we need stats from 1st frame*/
      stats_stream->init_frame_drop = 0;
    } else {
      stats_stream->init_frame_drop =
        sink_port->sensor_out_info.num_frames_skip;
    }

    /* stats buffer allocated by isp native buffer*/
    stats_stream->use_native_buf = 1;

    session->num_stats_stream++;
  }

  return rc;

}

/** iface_util_set_hw_stream_config_pix_offline
 *
 * DESCRIPTION:
 *
 **/
int iface_util_set_hw_stream_config_pix_offline(iface_t *iface,
  iface_port_t *iface_sink_port, uint32_t session_id, uint32_t stream_id)
{
  int rc = 0;
  uint32_t i, j;
  iface_resource_request_t isp_resource_request;
  iface_session_t *iface_session;
  iface_hw_stream_t *reserving_hw_stream = NULL;
  isp_pix_out_info_t *isp_pix_output;
  CDBG("%s: E\n", __func__);

  iface_session = iface_util_get_session_by_id(iface, session_id);
  if (iface_session == NULL) {
     CDBG_ERROR("%s: can not find session , session id = %d, rc = -1\n",
       __func__, session_id);
     return -1;
  }

  /* send event to isp to query pix output stream*/
  memset(&isp_resource_request, 0, sizeof(iface_resource_request_t));
  rc = iface_util_request_isp_output_resource(iface,
    &isp_resource_request, session_id, stream_id, TRUE);
  if (rc < 0) {
     CDBG_ERROR("%s: iface_util_request_isp_output_resource failed, rc = %d\n",
       __func__, rc);
     return rc;
  }

  iface_util_dump_isp_resource_request(iface, &isp_resource_request);
  if (isp_resource_request.num_pix_stream == 0) {
     CDBG_ERROR("%s: no pix output resource from ISP!\n", __func__);
     return -1;
  }

  /*reserve offline resource in session*/
  iface_session->session_resource.offline_isp_id_mask =
    isp_resource_request.isp_id_mask;

  if (iface_session->session_resource.offline_isp_id_mask & (1 << VFE0)) {
    iface_session->session_resource.used_resource_mask |=
      (1 << (16 * VFE0 + IFACE_INTF_PIX));
  }

  if (iface_session->session_resource.offline_isp_id_mask & (1 << VFE1)) {
    iface_session->session_resource.used_resource_mask |=
      (1 << (16 * VFE1 + IFACE_INTF_PIX));
  }

  /*after reserve resource, set pix clk rate*/
  for (i = 0; i < VFE_MAX; i++) {
    if (isp_resource_request.isp_id_mask & (1 << i)) {
        iface->isp_axi_data.axi_data[i].interface_clk_request[IFACE_INTF_PIX]
           = isp_resource_request.request_pix_op_clk;
    }
  }

  /* after reserve pix resource, fill in hw stream*/
  for (i = 0; i < isp_resource_request.num_pix_stream
    && i < ISP_NUM_PIX_STREAM_MAX; i++) {
    isp_pix_output = &isp_resource_request.isp_pix_output[i];
    /*fill pix hw stream info into session*/
    for (j = 0; j < IFACE_MAX_STREAMS; j++) {
      if (iface_session->hw_streams[j].stream_info.dim.width == 0) {
        reserving_hw_stream = &iface_session->hw_streams[j];
        break;
      }
    }

    if (!reserving_hw_stream) {
      CDBG_ERROR("%s:%d failed: reserving_hw_stream %p\n", __func__, __LINE__,
        reserving_hw_stream);
      continue;
    }

    memset(reserving_hw_stream, 0 ,sizeof(iface_hw_stream_t));

    reserving_hw_stream->stream_info.cam_stream_type =
          CAM_STREAM_TYPE_OFFLINE_PROC;
    rc = iface_util_fill_hw_stream_info_pix(reserving_hw_stream, iface_session,
      iface_sink_port, isp_pix_output, isp_resource_request.isp_id_mask, 0, FALSE);
    if (rc < 0) {
      CDBG_ERROR("%s: iface_util_fill_hw_stream_info_pix ERROR, rc = -1\n",
        __func__);
      return -1;
    }

    if (isp_resource_request.max_nominal_clk) {
      iface_session->offline_info.op_pixel_clk =
        isp_resource_request.max_nominal_clk;
    } else {
      CDBG_ERROR("%s: ERROR in getting isp clock rc = -1\n",
        __func__);
      return -1;
    }
    iface_session->num_hw_stream++;
  }

  rc = iface_utill_fill_stats_stream_info(iface_session, &isp_resource_request,
    iface_sink_port, TRUE);
  if (rc < 0) {
    CDBG_ERROR("%s: iface_utill_fill_stats_stream_info error! rc = %d\n",
      __func__, rc);
    return rc;
  }

  return rc;
}

/** iface_util_set_hw_stream_config_pix
 *
 * DESCRIPTION:
 *
 **/
int iface_util_set_hw_stream_config_pix(iface_t *iface,
  iface_port_t *iface_sink_port, uint32_t session_id, uint32_t stream_id)
{
  int rc = 0;
  uint32_t i, j, vfe_clk;
  iface_resource_request_t isp_resource_request;
  iface_session_t *iface_session;
  iface_hw_stream_t *reserving_hw_stream = NULL;
  isp_pix_out_info_t *isp_pix_output;
  sensor_out_info_t *sensor_out_info;
  ispif_out_info_t *split_info;
  uint32_t max_fps = 0;
  int clk_opt_enable = 0;
  char value[PROPERTY_VALUE_MAX];
  boolean is_burst = TRUE;

  CDBG("%s: E\n", __func__);

  iface_session = iface_util_get_session_by_id(iface, session_id);
  if (iface_session == NULL) {
     CDBG_ERROR("%s: can not find session , session id = %d, rc = -1\n",
       __func__, session_id);
     return -1;
  }

  /* send event to isp to query pix output stream*/
  memset(&isp_resource_request, 0, sizeof(iface_resource_request_t));
  rc = iface_util_request_isp_output_resource(iface,
    &isp_resource_request, session_id, stream_id, FALSE);
  if (rc < 0) {
     CDBG_ERROR("%s: iface_util_request_isp_output_resource failed, rc = %d\n",
       __func__, rc);
  }

  /* if isp module is not linked there is not pix streams. YUV sensor case. */
  if (!isp_resource_request.num_pix_stream) {
    /* if no pix stream but need split, config split info for camif stream*/
    iface_resource_t *session_resource = NULL;
    session_resource = &iface_session->session_resource;
    session_resource->isp_id_mask = isp_resource_request.isp_id_mask;
    session_resource->num_isps = isp_resource_request.num_isps;
    if (isp_resource_request.num_isps > 1) {
      sensor_out_info_t *sensor_out_info = NULL;
      sensor_out_info = &(iface_sink_port->u.sink_port.sensor_out_info);
      session_resource->ispif_split_info.is_split = TRUE;
      session_resource->ispif_split_info.overlap = 0;
      session_resource->ispif_split_info.right_stripe_offset =
        (sensor_out_info->request_crop.last_pixel -
        sensor_out_info->request_crop.first_pixel + 1) / 2;
    }
    return 0;
  }
  /* store the device type i.e hw version */
  iface_session->hw_version = isp_resource_request.hw_version;
  /* Store info about pipeline support camif raw crop or camif raw op fmt etc*/
  iface_session->camif_cap = isp_resource_request.camif_cap;
  /* Add ISP initial frame skip. Since we config pix streams first RDI streams
   * will use same initial frame skip */
  iface_session->initial_frame_skip +=
    isp_resource_request.isp_pix_output[0].isp_frame_skip;

  if (iface_session->hw_version == ISP_MSM8937 ||
      iface_session->hw_version == ISP_MSM8953 ||
      iface_session->hw_version == ISP_MSM8917) {
        iface_session->cds_feature_enable = TRUE;
  }
  /* after got resource from ISP, reserve pix resource
    reserve resource to keep track used interface and how we turn on pix clk*/
  rc = iface_util_reserve_pix_resource(iface, session_id,
    &isp_resource_request);
  if (rc < 0) {
     CDBG_ERROR("%s: iface_util_reserve_pix_resource failed, rc = %d\n",
       __func__, rc);
     return rc;
  }

  /* Set this intf as main frame intf if there is at least 1 camif stream in session */
  if (iface_session->ms_type != MS_TYPE_NONE &&
     (iface_session->session_resource.camif_cnt > 0)) {
    /* These 2 enums are assumed to be similar */
    iface_session->session_resource.main_frame_intf = VFE_PIX_0;
    /* Check for VFE0 first in case of split, we want timestamp of left */
    if (isp_resource_request.isp_id_mask & (1 << VFE0)) {
      iface_session->session_resource.main_frame_vfe = VFE0;
    } else {
      iface_session->session_resource.main_frame_vfe = VFE1;
    }
    IFACE_HIGH("%s: Assigned vfe %d intf %d for related cam type %d\n",
      __func__, iface_session->session_resource.main_frame_vfe,
      iface_session->session_resource.main_frame_intf,
      iface_session->ms_type);
  }

  iface_util_dump_isp_resource_request(iface, & isp_resource_request);
  property_get("persist.vendor.camera.isp.clock.optmz", value, "1");
  clk_opt_enable = atoi(value);
  if (clk_opt_enable) {
    sensor_out_info = &iface_sink_port->u.sink_port.sensor_out_info;
    split_info = &iface_session->session_resource.ispif_split_info;
    /*after reserve resource, set pix clk rate*/
    for (i = 0; i < VFE_MAX; i++) {
      if (isp_resource_request.isp_id_mask & (1 << i)) {
        vfe_clk = iface_util_get_vfe_clk(iface_session->num_bundle_meta,
          split_info, sensor_out_info, i);
        if (vfe_clk > 0) {
          iface->isp_axi_data.axi_data[i].interface_clk_request[IFACE_INTF_PIX] =
            vfe_clk;
        } else {
            CDBG_ERROR("ERROR While Calculating the Clock");
            return -1;
        }
      }
    }
  } else {
     max_fps = iface_sink_port->u.sink_port.sensor_out_info.vt_pixel_clk /
      (iface_sink_port->u.sink_port.sensor_out_info.ll_pck *
      iface_sink_port->u.sink_port.sensor_out_info.fl_lines);
     /*after reserve resource, set pix clk rate*/
     for (i = 0; i < VFE_MAX; i++) {
       if (isp_resource_request.isp_id_mask & (1 << i)) {
         if (iface_session->session_resource.num_isps > 1){
           iface->isp_axi_data.axi_data[i].interface_clk_request[IFACE_INTF_PIX]
           = iface_sink_port->u.sink_port.sensor_out_info.dim_output.width *
             max_fps *
              iface_sink_port->u.sink_port.sensor_out_info.fl_lines;
         } else {
             iface->isp_axi_data.axi_data[i].interface_clk_request[IFACE_INTF_PIX]
             = isp_resource_request.request_pix_op_clk;
         }
       }
     }
  }

  for (i = 0; i < isp_resource_request.num_pix_stream
    && i < ISP_NUM_PIX_STREAM_MAX; i++) {
    if ((isp_resource_request.isp_pix_output[i].streaming_mode ==
      CAM_STREAMING_MODE_CONTINUOUS) ||(isp_resource_request.
      isp_pix_output[i].streaming_mode == CAM_STREAMING_MODE_BATCH)) {
      is_burst = FALSE;
    }
    iface_session->cds_feature_enable &=
      isp_resource_request.cds_capable[i];
  }
  /* after reserve pix resource, fill in hw stream*/
  for (i = 0; i < isp_resource_request.num_pix_stream
    && i < ISP_NUM_PIX_STREAM_MAX; i++) {
    isp_pix_output = &isp_resource_request.isp_pix_output[i];
    /*fill pix hw stream info into session*/
    for (j = 0; j < IFACE_MAX_STREAMS; j++) {
      if (iface_session->hw_streams[j].stream_info.dim.width == 0) {
        reserving_hw_stream = &iface_session->hw_streams[j];
        break;
      }
    }

    if (!reserving_hw_stream) {
      CDBG_ERROR("%s:%d failed: reserving_hw_stream %p\n", __func__, __LINE__,
        reserving_hw_stream);
      continue;
    }

    memset(reserving_hw_stream, 0 ,sizeof(iface_hw_stream_t));
    rc = iface_util_fill_hw_stream_info_pix(reserving_hw_stream, iface_session,
      iface_sink_port, isp_pix_output, isp_resource_request.isp_id_mask,
      iface_session->initial_frame_skip, is_burst);
    if (rc < 0) {
      CDBG_ERROR("%s: iface_util_fill_hw_stream_info_pix ERROR, rc = -1\n",
        __func__);
      return -1;
    }
    iface_session->num_hw_stream++;
  }
  rc = iface_utill_fill_stats_stream_info(iface_session, &isp_resource_request,
    iface_sink_port, FALSE);
  if (rc < 0) {
    CDBG_ERROR("%s: iface_utill_fill_stats_stream_info error! rc = %d\n",
      __func__, rc);
    return rc;
  }

  return rc;
}

/** iface_get_rdi_resource
 *
 * DESCRIPTION:
 *
 **/
static int iface_get_rdi_resource(int isp_id, iface_t *iface, iface_session_t *iface_session,
  iface_intf_type_t *reserved_rdi_intf)
{
  int i;
  uint32_t isp_id_mask = 1 << isp_id;
  int rc = -1;
  int vfe1_reserved_rdi = 3;
  int num_used_rdi = 0;

  if (isp_id == VFE1) {
    vfe1_reserved_rdi = iface->vfe1_reserved_rdi;
    if (vfe1_reserved_rdi < 0 || vfe1_reserved_rdi > 3)
      vfe1_reserved_rdi = 3;
  }

  CDBG("%s: isp_id= %d used_resource_mask= %x\n", __func__,
    isp_id, iface_session->session_resource.used_resource_mask);

  pthread_mutex_lock(&iface->mutex);
  num_used_rdi = (iface->rdi_used_resource_mask >> (16 * isp_id)) & 0xFFFF;
  num_used_rdi = ((num_used_rdi >> IFACE_INTF_RDI0) & 0x1) +
    ((num_used_rdi >> IFACE_INTF_RDI1) & 0x1) +
    ((num_used_rdi >> IFACE_INTF_RDI2) & 0x1);

  if (num_used_rdi >= vfe1_reserved_rdi ||
    iface->used_wm[isp_id] >= NUM_AXI_WM) {
    pthread_mutex_unlock(&iface->mutex);
    return rc;
  }

  for (i = IFACE_INTF_RDI0; i <= IFACE_INTF_RDI2; i++) {
    if (iface->rdi_used_resource_mask & (1 << (16 * isp_id + i)))
      continue;
    *reserved_rdi_intf = i;
    iface_session->session_resource.isp_id_mask |= isp_id_mask;
    iface->rdi_used_resource_mask |=
      1 << (16 * isp_id + i);

    iface_session->session_resource.used_wm[isp_id] += 1;
    iface->used_wm[isp_id] += 1;
    iface_session->session_resource.used_resource_mask |=
      1 << (16 * isp_id + i);
    if (iface_session->session_resource.num_isps == 0)
      iface_session->session_resource.num_isps = 1;
    CDBG("%s: isp_id_mask %x in %x used mask %x\n", __func__,
      iface_session->session_resource.isp_id_mask, isp_id_mask,
      iface_session->session_resource.used_resource_mask);
    rc = 0;
    break;
  }
  pthread_mutex_unlock(&iface->mutex);
  return rc;
}

/** iface_util_fill_hw_stream_info_pix_raw:
 *    @camif_hw_stream: hw stream struct to be populated
 *    @iface_sink_port: sink port of iface
 *    @iface_stream: iface user stream
 *    @isp_id: vfe hw id to use
 *    @session: iface session handle
 *    @axi_path: axi path
 *
 *    Clear and initialize planes info for camif/idal raw
 **/
int32_t iface_util_fill_hw_stream_info_pix_raw(
  iface_hw_stream_t *camif_hw_stream, iface_sink_port_t *iface_sink_port,
  iface_stream_t *iface_stream, uint32_t isp_id, iface_session_t *session,
  enum msm_vfe_axi_stream_src axi_path)
{
  uint32_t k;
  int32_t  rc = 0;
  ispif_out_info_t split_info;

  split_info = session->session_resource.ispif_split_info;

  CDBG("%s: fmt %d type %d width %d ht %d streaming mode %d",
    __func__, iface_stream->stream_info.fmt,
    iface_stream->stream_info.stream_type,
    iface_stream->stream_info.dim.width, iface_stream->stream_info.dim.height,
    iface_stream->stream_info.streaming_mode);
  memset(camif_hw_stream, 0 ,sizeof(iface_hw_stream_t));
  camif_hw_stream->session_id = iface_stream->session_id;
  camif_hw_stream->use_pix = 0;
  camif_hw_stream->hw_stream_id = iface_stream->stream_id;
  camif_hw_stream->need_divert = 0;
  camif_hw_stream->frame_base = 0;
  camif_hw_stream->mapped_mct_stream_id[0] = iface_stream->stream_id;
  camif_hw_stream->num_mct_stream_mapped = 1;
  camif_hw_stream->need_uv_subsample = 0;

  if (split_info.is_split == TRUE) {
    camif_hw_stream->isp_split_output_info.is_split = 1;
    camif_hw_stream->interface_mask |= 1 << (16 * VFE0 + IFACE_INTF_PIX);
    camif_hw_stream->interface_mask |= 1 << (16 * VFE1 + IFACE_INTF_PIX);
  } else {
    camif_hw_stream->isp_split_output_info.is_split = 0;
    camif_hw_stream->interface_mask |= 1 << (16 * isp_id + IFACE_INTF_PIX);
  }

  camif_hw_stream->stream_info.buf_planes =
    iface_stream->stream_info.buf_planes;
  camif_hw_stream->stream_info.dim = iface_stream->stream_info.dim;
  camif_hw_stream->stream_info.fmt = iface_stream->stream_info.fmt;
  camif_hw_stream->buffers_info[VFE_BUF_QUEUE_DEFAULT].img_buffer_list =
    iface_stream->stream_info.img_buffer_list;
  camif_hw_stream->buffers_info[VFE_BUF_QUEUE_DEFAULT].num_bufs =
    iface_stream->stream_info.num_bufs;
  camif_hw_stream->stream_info.num_burst = iface_stream->stream_info.num_burst;
  camif_hw_stream->stream_info.streaming_mode =
    iface_stream->stream_info.streaming_mode;

  /* initial value no skip, HAL can set hfr parm after stream
    created by iface_util_set_hfr*/
  camif_hw_stream->frame_skip_pattern = NO_SKIP;
  camif_hw_stream->left_pixel_hor_skip = 0;
  camif_hw_stream->left_pixel_ver_skip = 0;
  camif_hw_stream->right_pixel_hor_skip = 0;
  camif_hw_stream->right_pixel_ver_skip = 0;
  camif_hw_stream->camif_output_format =
        (enum msm_vfe_camif_output_format) CAMIF_QCOM_FORMAT;

  camif_hw_stream->axi_path = axi_path;

  if (session->hal_version == CAM_HAL_V3) {
    camif_hw_stream->controllable_output = 1;
  } else {
    camif_hw_stream->controllable_output = 0;
  }
  iface_util_fill_hw_stream_plane_info(camif_hw_stream,
    &camif_hw_stream->stream_info.buf_planes.plane_info);

  /* frame stream, non meta data stream.
     add meta stream is only add another rdi stream and config different cid*/
  rc = iface_util_fill_sensor_csid_info_to_hw_by_format(camif_hw_stream,
    &iface_sink_port->sensor_cap, iface_sink_port->sensor_out_info.fmt,
    &iface_sink_port->sensor_out_info);
  if (rc < 0) {
    CDBG_ERROR("%s: iface_util_fill_sensor_csid_info_to_hw_by_format error, rc = %d\n",
      __func__, rc);
    return rc;
  }

  camif_hw_stream->stream_type = IFACE_STREAM_TYPE_IMAGE;
  camif_hw_stream->sensor_skip_cnt = session->initial_frame_skip;

  /*change the state for hw stream*/
  camif_hw_stream->state = IFACE_HW_STREAM_STATE_RESERVED;

  return rc;
}

/** iface_util_fill_hw_stream_info_rdi:
 *
 *    @planes: plane info
 *    @stream: hw stream
 *
 *    Clear and initialize planes info
 **/
iface_hw_stream_t *iface_util_fill_hw_stream_info_rdi(
  iface_session_t *iface_session, iface_sink_port_t *iface_sink_port,
  iface_stream_t *iface_stream, iface_intf_type_t reserved_rdi_intf,
  uint32_t isp_id)
{
  uint32_t           k;
  int32_t            rc = 0;
  iface_hw_stream_t *rdi_hw_stream = NULL;
  cam_dimension_t    dim;
  cam_format_t       format;

  CDBG("%s: reserved_rdi_intf  %d\n", __func__, reserved_rdi_intf );
  rdi_hw_stream = iface_util_get_empty_hw_stream(iface_session);
  if (!rdi_hw_stream || !iface_sink_port || !iface_stream) {
    CDBG_ERROR("%s:%d %p %p %p\n", __func__, __LINE__, rdi_hw_stream,
      iface_sink_port, iface_stream);
    return NULL;
  }

  if (reserved_rdi_intf == IFACE_INTF_RDI0) {
    rdi_hw_stream->interface_mask |=
      1 << (16 * isp_id + IFACE_INTF_RDI0);
    rdi_hw_stream->axi_path = RDI_INTF_0;
  } else if (reserved_rdi_intf == IFACE_INTF_RDI1) {
    rdi_hw_stream->interface_mask |=
      1 << (16 * isp_id + IFACE_INTF_RDI1);
    rdi_hw_stream->axi_path = RDI_INTF_1;
  } else if (reserved_rdi_intf == IFACE_INTF_RDI2) {
    rdi_hw_stream->interface_mask |=
      1 << (16 * isp_id + IFACE_INTF_RDI2);
    rdi_hw_stream->axi_path = RDI_INTF_2;
  } else {
    CDBG_ERROR("%s: invalid rdi_intf %d, return -1!\n",
      __func__, reserved_rdi_intf);
    return NULL;
  }

  rdi_hw_stream->session_id = iface_stream->session_id;
  rdi_hw_stream->use_pix = 0;
  /*when we saw it on HAL stream, use HAL buf
    if we create our own RDI, then use Native buf*/
  rdi_hw_stream->hw_stream_id = iface_stream->stream_id;
  rdi_hw_stream->need_divert = 0;
  /*RDI is frame base*/
  rdi_hw_stream->frame_base = 1;
  rdi_hw_stream->mapped_mct_stream_id[rdi_hw_stream->num_mct_stream_mapped] =
    iface_stream->stream_id;
  rdi_hw_stream->num_mct_stream_mapped++;
  /*RDI not support split*/
  rdi_hw_stream->isp_split_output_info.is_split = 0;
  rdi_hw_stream->need_uv_subsample = 0;

  if(iface_util_sensor_is_yuv(iface_sink_port)) {

    rdi_hw_stream->use_native_buf = 1;
    rdi_hw_stream->need_divert = 1;
    rdi_hw_stream->hw_stream_id |= ISP_NATIVE_BUF_BIT;
    if (iface_sink_port->sensor_out_info.is_depth_sensor == TRUE)
      rdi_hw_stream->stream_info.dim.width =
        iface_sink_port->sensor_out_info.dim_output.width;
    else
      rdi_hw_stream->stream_info.dim.width =
        iface_sink_port->sensor_out_info.dim_output.width/2;
    rdi_hw_stream->stream_info.dim.height =
      iface_sink_port->sensor_out_info.dim_output.height;
    /* Here iface sink port expects sensor to report acual MIPI format  *
     * like CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG not Depth format which *
     * is like CAM_FORMAT_DEPTH16 or CAM_FORMAT_DEPTH_POINT_CLOUD       */
    rdi_hw_stream->stream_info.fmt =
      iface_sink_port->sensor_out_info.fmt;
    iface_util_modify_plane_info_for_native_buf(rdi_hw_stream);
  } else if(iface_session->bayer_processing){
     rdi_hw_stream->use_native_buf = 1;
     rdi_hw_stream->need_divert = 1;
#ifdef ISP_SVHDR_IN_BIT
     rdi_hw_stream->hw_stream_id |= (ISP_NATIVE_BUF_BIT | ISP_SVHDR_IN_BIT);
#endif
     CDBG("sensor_info.custom_format.enable = %d, wxh %dx%d subframe_cnt %d",
        iface_sink_port->sensor_out_info.custom_format.enable,
        iface_sink_port->sensor_out_info.custom_format.width,
        iface_sink_port->sensor_out_info.custom_format.height,
        iface_sink_port->sensor_out_info.custom_format.subframes_cnt);

     if (iface_sink_port->sensor_out_info.custom_format.enable) {
       rdi_hw_stream->stream_info.dim.width =
          iface_sink_port->sensor_out_info.custom_format.width;
       rdi_hw_stream->stream_info.dim.height =
          iface_sink_port->sensor_out_info.custom_format.height *
          iface_sink_port->sensor_out_info.custom_format.subframes_cnt;
    } else {
       rdi_hw_stream->stream_info.dim.width =
          iface_sink_port->sensor_out_info.dim_output.width;
       rdi_hw_stream->stream_info.dim.height =
          iface_sink_port->sensor_out_info.dim_output.height;
    }
     rdi_hw_stream->stream_info.fmt =
        iface_sink_port->sensor_out_info.fmt;
     iface_util_modify_plane_info_for_native_buf(rdi_hw_stream);
  }
  else {
    rdi_hw_stream->stream_info.dim =
      iface_stream->stream_info.dim;
    rdi_hw_stream->stream_info.fmt =
      iface_stream->stream_info.fmt;
    rdi_hw_stream->stream_info.buf_planes =
      iface_stream->stream_info.buf_planes;
  }

  rdi_hw_stream->buffers_info[VFE_BUF_QUEUE_DEFAULT].img_buffer_list =
    iface_stream->stream_info.img_buffer_list;
  rdi_hw_stream->buffers_info[VFE_BUF_QUEUE_DEFAULT].num_bufs =
    iface_stream->stream_info.num_bufs;
  rdi_hw_stream->stream_info.num_burst =
    iface_stream->stream_info.num_burst;
  rdi_hw_stream->stream_info.streaming_mode =
    iface_stream->stream_info.streaming_mode;

  // initial value no skip, HAL can set hfr parm after stream created by iface_util_set_hfr
  rdi_hw_stream->frame_skip_pattern = NO_SKIP;
  if (iface_session->hal_version == CAM_HAL_V3 &&
        !iface_session->bayer_processing) {
    rdi_hw_stream->controllable_output = 1;
  } else {
    rdi_hw_stream->controllable_output = 0;
  }
  if (iface_sink_port->sensor_out_info.is_depth_sensor == TRUE)
    rdi_hw_stream->controllable_output = 0;

  iface_util_fill_hw_stream_plane_info(rdi_hw_stream,
    &rdi_hw_stream->stream_info.buf_planes.plane_info);

  memset(rdi_hw_stream->pack_mode, 0, sizeof(rdi_hw_stream->pack_mode));
  /* frame stream, non meta data stream.
     add meta stream is only add another rdi stream and config different cid*/
  rc = iface_util_fill_sensor_csid_info_to_hw_by_format(rdi_hw_stream,
    &iface_sink_port->sensor_cap, iface_sink_port->sensor_out_info.fmt,
    &iface_sink_port->sensor_out_info);
  if (rc < 0) {
    CDBG_ERROR("%s: iface_util_fill_sensor_csid_info_to_hw_by_format error, rc = %d\n",
      __func__, rc);
    return NULL;
  }
  format = iface_stream->stream_info.fmt;
  if ((format == CAM_FORMAT_META_RAW_10BIT)||
     (format >=CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GBRG &&
      format <= CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_BGGR)) {
    iface_util_fill_sensor_csid_info_to_hw_by_format(rdi_hw_stream,
      &iface_sink_port->sensor_cap, iface_stream->stream_info.fmt,
      &iface_sink_port->sensor_out_info);
  } else {
    iface_util_fill_sensor_csid_info_to_hw_by_format(rdi_hw_stream,
      &iface_sink_port->sensor_cap, iface_sink_port->sensor_out_info.fmt,
      &iface_sink_port->sensor_out_info);
  }

  rdi_hw_stream->stream_type = IFACE_STREAM_TYPE_IMAGE;
  rdi_hw_stream->sensor_skip_cnt = iface_session->initial_frame_skip;

  /*change the state for hw stream*/
  rdi_hw_stream->state = IFACE_HW_STREAM_STATE_RESERVED;

  iface_session->session_resource.rdi_cnt++;
  iface_session->num_hw_stream++;

  return rdi_hw_stream;
}

/** iface_util_reserve_camif_resource
 *
 *    @iface: iface instance
 *    @session: current session instance
 *    @isp_id: need to fill up which isp id is reserved for operation.
 *
 *  Reserve VFE 0 or 1 for CAMIF raw and set the mask bit accordingly
 *
  * Returns 0 on success.
 **/
int iface_util_reserve_camif_resource(
  iface_t         *iface,
  iface_session_t *session,
  uint32_t        *isp_id)
{
  int rc = 0;
  int i = 0;

  *isp_id = VFE_MAX;
  /*reserve VFE resource, save info in session and user streams*/
  /* Check if any VFE is already used by the session thenchoose the same */

  pthread_mutex_lock(&iface->mutex);

  for (i = VFE_MAX - 1; i >= 0; i--) {
    if ((1 << i) & session->session_resource.isp_id_mask) {
      if (iface->used_wm[i] >= NUM_AXI_WM) {
        pthread_mutex_unlock(&iface->mutex);
        return -1;
      }
    }
  }

  for (i = VFE_MAX - 1; i >= 0; i--) {
    if ((1 << i) & session->session_resource.isp_id_mask) {
      *isp_id = i;
      session->session_resource.used_wm[*isp_id] += 1;
      iface->used_wm[*isp_id] += 1;
    }
  }

  if (*isp_id == VFE_MAX) {
    *isp_id = VFE0;
    if (iface->used_wm[*isp_id] >= NUM_AXI_WM) {
      pthread_mutex_unlock(&iface->mutex);
      return -1;
    }
    session->session_resource.used_resource_mask |=
      (1 << (16 * *isp_id + IFACE_INTF_PIX));
    session->session_resource.isp_id_mask |= 1 << *isp_id;
    session->session_resource.used_wm[*isp_id] += 1;
      iface->used_wm[*isp_id] += 1;
  }
  pthread_mutex_unlock(&iface->mutex);

  CDBG_HIGH("%s: Camif Choose VFE%d resource ", __func__, *isp_id);
  return rc;
}

/** iface_util_reserve_rdi_resource
 *
 *    @iface: iface instance
 *    @session: current session instance
 *    @reserved_rdi_intf: mask to be filled according to the resource alloted
 *    @isp_id: need to fill up which isp id is reserved for operation.
 *
 *  Reserve VFE 0 or 1 for RDI and set the mask bits accordingly
 *
 * Returns 0 on success.
 **/
static int iface_util_reserve_rdi_resource(
  iface_t           *iface,
  iface_session_t   *session,
  iface_intf_type_t *reserved_rdi_intf,
  uint32_t          *isp_id)
{
  int rc = 0;
  int i;
  int vfe_tried[VFE_MAX];
  int vfe1_reserved_rdi = 3;

  vfe1_reserved_rdi = iface->vfe1_reserved_rdi;
  if (vfe1_reserved_rdi > 0 && vfe1_reserved_rdi <= 3) {
    rc = iface_get_rdi_resource(VFE1, iface, session, reserved_rdi_intf);
    if (rc)
      vfe_tried[VFE1] = 1;
    else {
      *isp_id = VFE1;
      goto done;
    }
  }

  memset(&vfe_tried, 0, sizeof(vfe_tried));
  /* first try to alloc from already used vfe for this session */
  for (i = 0; i < VFE_MAX; i++) {
    if (!(session->session_resource.isp_id_mask & (1 << i)))
      continue;
    rc = iface_get_rdi_resource(i, iface, session, reserved_rdi_intf);
    if (rc) {
      vfe_tried[i] = 1;
      continue;
    }
    *isp_id = i;
    goto done;
  }

  /* try with the vfe that is used on some other session */
  for (i = 0; i < VFE_MAX; i++) {
    if (vfe_tried[i])
      continue;
    if (!iface->isp_axi_data.axi_data[i].ref_cnt)
      continue;
    rc = iface_get_rdi_resource(i, iface, session, reserved_rdi_intf);
    if (rc) {
      vfe_tried[i] = 1;
      continue;
    }
    *isp_id = i;
    goto done;
  }

  /* try remaining vfe's */
  for (i = 0; i < VFE_MAX; i++) {
    if (vfe_tried[i])
      continue;
    rc = iface_get_rdi_resource(i, iface, session, reserved_rdi_intf);
    if (rc)
      continue;
    *isp_id = i;
    goto done;
  }
  /* no free resource */
  rc = -1;

done:
  if (rc < 0) {
    CDBG_ERROR("No free rdi resource found for session %d, used rdi mask %x\n",
              session->session_id, iface->rdi_used_resource_mask);
    return rc;
  }

  CDBG("%s:RDI resource reserved! VFE%d RDI_INTF %d\n", __func__,
    *isp_id, *reserved_rdi_intf);
  return rc;
}

cam_format_t iface_util_meta_camif_format(cam_format_t mipi_format,
  camif_fmt_t camif_out_format)
{
  switch (camif_out_format) {
    case CAMIF_QCOM_FORMAT: {
      switch(mipi_format) {
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
        return CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GBRG;
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
        return CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GRBG;
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
        return CAM_FORMAT_BAYER_QCOM_RAW_8BPP_RGGB;
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
        return CAM_FORMAT_BAYER_QCOM_RAW_8BPP_BGGR;
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
        return  CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GBRG;
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
        return CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GRBG;
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
        return CAM_FORMAT_BAYER_QCOM_RAW_10BPP_RGGB;
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
        return CAM_FORMAT_BAYER_QCOM_RAW_10BPP_BGGR;
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
        return CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GBRG;
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
        return CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GRBG;
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
        return CAM_FORMAT_BAYER_QCOM_RAW_12BPP_RGGB;
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
        return CAM_FORMAT_BAYER_QCOM_RAW_12BPP_BGGR;
      case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GBRG:
        return CAM_FORMAT_BAYER_QCOM_RAW_14BPP_GBRG;
      case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GRBG:
        return CAM_FORMAT_BAYER_QCOM_RAW_14BPP_GRBG;
      case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_RGGB:
        return CAM_FORMAT_BAYER_QCOM_RAW_14BPP_RGGB;
      case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_BGGR:
        return CAM_FORMAT_BAYER_QCOM_RAW_14BPP_BGGR;
      default:
        return 0;
      }
    }

    case CAMIF_MIPI_FORMAT: {
      return mipi_format;
    }

    case CAMIF_PLAIN16_FORMAT: {
      switch(mipi_format) {
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
        return CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GBRG;
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
        return CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GRBG;
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
        return CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_RGGB;
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
        return CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_BGGR;
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
        return  CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GBRG;
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
        return CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GRBG;
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
        return CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_RGGB;
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
        return CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_BGGR;
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
        return CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GBRG;
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
        return CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GRBG;
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
        return CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_RGGB;
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
        return CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_BGGR;
      case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GBRG:
        return CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_GBRG;
      case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GRBG:
        return CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_GRBG;
      case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_RGGB:
        return CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_RGGB;
      case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_BGGR:
        return CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_BGGR;
      default:
        return 0;
      }
    }

    case CAMIF_PLAIN8_FORMAT: {
      switch(mipi_format) {
        case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
          return CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GBRG;
        case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
          return CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GRBG;
        case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
          return CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_RGGB;
        case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
          return CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_BGGR;
        default:
          return 0;
      }
    }

    default:
      return 0;
  }
  return 0;
}

enum msm_vfe_camif_output_format iface_util_convert_camif_format(
  camif_fmt_t camif_format)
{
  enum msm_vfe_camif_output_format output_format;
  switch(camif_format) {
    case CAMIF_MIPI_FORMAT:
      output_format = CAMIF_MIPI_RAW;
      break;
    case CAMIF_QCOM_FORMAT:
      output_format = CAMIF_QCOM_RAW;
      break;
    case CAMIF_PLAIN16_FORMAT:
      output_format = CAMIF_PLAIN_16;
      break;
    case CAMIF_PLAIN8_FORMAT:
      output_format = CAMIF_PLAIN_8;
      break;
    default:
      output_format = CAMIF_QCOM_RAW;
      break;
  }
  return output_format;
}

int32_t iface_util_fill_camif_meta_info(iface_session_t *iface_session,
  iface_hw_stream_t *camif_hw_stream, iface_stream_t *user_stream,
  iface_bundle_pdaf_info_t *bundle_pdaf_info)
{
  int32_t  rc = 0;
  uint32_t k = 0, i =0;

  if (!camif_hw_stream || !user_stream || !bundle_pdaf_info){
    CDBG_ERROR("camif_hw_stream %p, user_stream %p bundle_pdaf_info %p",
      camif_hw_stream, user_stream, bundle_pdaf_info);
    return -1;
  }

  camif_hw_stream->need_divert = 1;
  camif_hw_stream->hw_stream_id = user_stream->stream_id | ISP_NATIVE_BUF_BIT |
    ISP_META_CHANNEL_BIT;
  camif_hw_stream->divert_to_3a = TRUE;
  camif_hw_stream->use_native_buf = 1;
  camif_hw_stream->controllable_output = 0;
  camif_hw_stream->stream_info.dim.width = bundle_pdaf_info->block_dim.width;
  camif_hw_stream->stream_info.dim.height = bundle_pdaf_info->block_dim.height;
  CDBG("pdaf_dbg format %d  W %d H %d \n",camif_hw_stream->stream_info.fmt,
    camif_hw_stream->stream_info.dim.width,
    camif_hw_stream->stream_info.dim.height);
  camif_hw_stream->buffers_info[VFE_BUF_QUEUE_DEFAULT].num_bufs =
    IFACE_MAX_NATIVE_BUF_NUM;

  camif_hw_stream->ext_stats_type = 1;
  camif_hw_stream->stream_type = IFACE_STREAM_TYPE_META_DATA;
  camif_hw_stream->stream_info.fmt =
    iface_util_meta_camif_format(iface_session->sensor_out_info.fmt,
      bundle_pdaf_info->camif_output_format);
  camif_hw_stream->camif_output_format = iface_util_convert_camif_format(
     bundle_pdaf_info->camif_output_format);
  iface_util_modify_plane_info_for_native_buf(camif_hw_stream);
  iface_util_fill_hw_stream_plane_info(camif_hw_stream,
    &camif_hw_stream->stream_info.buf_planes.plane_info);
  CDBG("%s: wd x ht= %d x %d fmt= %d hw_stream_id %d\n", __func__,
    camif_hw_stream->stream_info.dim.width,
    camif_hw_stream->stream_info.dim.height,
    camif_hw_stream->stream_info.fmt,
    camif_hw_stream->hw_stream_id);
  return rc;
}

/** iface_util_get_dwords_per_line
 *
 *  @fmt: output format
 *
 */
int iface_util_get_dwords_per_line(
  cam_format_t format)
{
  switch (format) {
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_BGGR:
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GBRG:
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GRBG:
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_BGGR:
      return 16;
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_BGGR:
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_RGGB:
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GRBG:
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GBRG:
      return 12;
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_BGGR:
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GBRG:
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GRBG:
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_RGGB:
      return 10;
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_BGGR:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GBRG:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GRBG:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_RGGB:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_BGGR:
      return 8;
    default :
      return 12;
    }
}


/**
 * iface_util_config_meta_raw_size
 *
 * @param sensor_out_info
 * @param bundle_pdaf_info
 *
 * @return boolean
 */
boolean iface_util_config_meta_raw_size(
  iface_session_t          *iface_session,
  iface_hw_stream_t        *camif_hw_stream,
  iface_bundle_pdaf_info_t *bundle_pdaf_info)
{
  uint16_t skip_horiz_pattern = 0x0;
  uint16_t skip_vert_pattern =  0x0;
  uint32_t no_of_hor_skip;
  uint32_t no_of_vert_skip;
  uint32_t Width;
  uint32_t height;
  uint32_t shift_x;
  uint32_t shift_y;
  int residual_width = 0;
  int residual_height = 0;
  uint32_t i = 0;
  int j =0, dwords = 12;
  pdaf_sensor_native_info_t *pdaf_config;
  sensor_out_info_t *sensor_out_info;
  pdaf_data_buffer_info_t *camif_pdaf_info;
  ispif_out_info_t *split_info;
  uint32_t left_shift_x;
  uint32_t right_shift_x;
  uint32_t left_residual_width = 0;
  uint32_t right_residual_width = 0;
  uint16_t left_skip_horiz_pattern = 0x0;
  uint16_t right_skip_horiz_pattern = 0x0;

  if (!iface_session || !bundle_pdaf_info) {
    CDBG_ERROR("iface_session %p bundle_pdaf_info %p",
      iface_session,bundle_pdaf_info);
    return FALSE;
  }

  pdaf_config = &bundle_pdaf_info->pdaf_config;
  sensor_out_info = &iface_session->sensor_out_info;
  camif_pdaf_info = &iface_session->camif_info;
  split_info = &iface_session->session_resource.ispif_split_info;

  /* Sensor will send the coordinates of actual position Since
     we need to add  the skip pattern for every 16 pixels/line
     taking mod of the actual variable will do
     lets take sensor send coordinates as 201,210 as the masked
     pixel in their mask
     201 %16 = 9 , will mask the 9th position 210 %16 mask the 2nd
     position
  */
  for (i=0 ; i < pdaf_config->block_pattern.pix_count
    && i < MAX_CAMIF_PDAF_PIXELS; i++) {
    CDBG("pdaf_dbg pdaf_config->block_pattern.pix_coords[i].x %d mod16 %d",
      pdaf_config->block_pattern.pix_coords[i].x,
      pdaf_config->block_pattern.pix_coords[i].x%16);
    CDBG("pdaf_dbg pdaf_config->block_pattern.pix_coords[i].y %d mod16 %d",
      pdaf_config->block_pattern.pix_coords[i].y,
      pdaf_config->block_pattern.pix_coords[i].y%16);
    skip_horiz_pattern |= (1<<(pdaf_config->block_pattern.pix_coords[i].x % 16));
    skip_vert_pattern  |= (1<<(pdaf_config->block_pattern.pix_coords[i].y % 16));
  }

  CDBG("pdaf_dbg %s  hor pattern %x ver pattern %x ", __func__,
    skip_horiz_pattern, skip_vert_pattern);
  no_of_hor_skip = iface_util_find_number_skip(skip_horiz_pattern);
  CDBG("%s pdaf no_of_hor_skip %d", __func__, no_of_hor_skip);
  no_of_vert_skip = iface_util_find_number_skip(skip_vert_pattern);
  CDBG("%s pdaf_dbg no_of_vert_skip %d", __func__, no_of_vert_skip);
  CDBG("<pdaf_dbg> crop information  first line %d first pixel %d"
    "last line %d last pixel %d ", sensor_out_info->request_crop.first_line,
    sensor_out_info->request_crop.first_pixel,
    sensor_out_info->request_crop.last_line,
    sensor_out_info->request_crop.last_pixel);

  if (split_info->is_split == 0) {
    /* adjusted the position based on the crop
       If there is crop, we need to shift the pixels to
       extract pdaf pixels
    */
    shift_x = sensor_out_info->request_crop.first_pixel % 16;
    shift_y = sensor_out_info->request_crop.first_line % 16;
    /* first reverse the pattern then do the operations */
    skip_horiz_pattern = iface_util_reverse_skip(skip_horiz_pattern);
    skip_horiz_pattern = iface_util_circularShiftLeft(skip_horiz_pattern,shift_x);
    skip_vert_pattern = iface_util_reverse_skip(skip_vert_pattern);
    skip_vert_pattern = iface_util_circularShiftLeft(skip_vert_pattern,shift_y);
    CDBG("pdaf_dbg %s after circular rotate hor pattern %x ver pattern %x ", __func__,
      skip_horiz_pattern, skip_vert_pattern);
    CDBG("%s pdaf_dbg W %d H %d", __func__,sensor_out_info->dim_output.width,
      sensor_out_info->dim_output.height );

    bundle_pdaf_info->block_dim.width = (sensor_out_info->request_crop.last_pixel -
      sensor_out_info->request_crop.first_pixel + 1)/16 * no_of_hor_skip;
    bundle_pdaf_info->block_dim.height = (sensor_out_info->request_crop.last_line -
      sensor_out_info->request_crop.first_line + 1)/16 * no_of_vert_skip;
    CDBG("%s %d pdaf_dbg cal width W %d H %d", __func__, __LINE__,
      bundle_pdaf_info->block_dim.width, bundle_pdaf_info->block_dim.height);

    residual_width = (sensor_out_info->request_crop.last_pixel -
      sensor_out_info->request_crop.first_pixel + 1) % 16;
    residual_height = (sensor_out_info->request_crop.last_line -
      sensor_out_info->request_crop.first_line + 1) % 16;
    CDBG("%s pdaf_dbg residual  W %d H %d", __func__,residual_width,
      residual_height);

    /* taking and of the residual width + skip pattern to find how
       many width and heights are missed, if the height/width is
       not a multiple of 16. */
    bundle_pdaf_info->block_dim.width += iface_util_find_number_skip(
      skip_horiz_pattern & ALL_ONES_16(residual_width));
    bundle_pdaf_info->block_dim.height += iface_util_find_number_skip(
      skip_vert_pattern & ALL_ONES_16(residual_height));

    CDBG("%s %d pdaf_dbg cal width W %d H %d", __func__ ,__LINE__,
      bundle_pdaf_info->block_dim.width, bundle_pdaf_info->block_dim.height);

    camif_pdaf_info->camif_buffer_height = bundle_pdaf_info->block_dim.height;

    /* Since, Hardware is MSB aligned, we are reversing the skip
       patterns */
    bundle_pdaf_info->left_pix_skip_pattern = skip_horiz_pattern;
    bundle_pdaf_info->left_line_skip_pattern = skip_vert_pattern;
    camif_hw_stream->left_pixel_hor_skip = skip_horiz_pattern;
    camif_hw_stream->left_pixel_ver_skip = skip_vert_pattern;
  } else {
    CDBG("PDAF Dual vfe");

    dwords = iface_util_get_dwords_per_line(
      iface_util_meta_camif_format(iface_session->sensor_out_info.fmt,
      bundle_pdaf_info->camif_output_format));
    /*Left VFE*/
    left_shift_x = sensor_out_info->request_crop.first_pixel % 16;
    camif_hw_stream->isp_split_output_info.left_output_width =
      (split_info->right_stripe_offset + split_info->overlap -
      sensor_out_info->request_crop.first_pixel)/16 * no_of_hor_skip;
    left_residual_width = (split_info->right_stripe_offset +
      split_info->overlap - sensor_out_info->request_crop.first_pixel) % 16;

    left_skip_horiz_pattern = iface_util_reverse_skip(skip_horiz_pattern);
    left_skip_horiz_pattern =
      iface_util_circularShiftLeft(left_skip_horiz_pattern, left_shift_x);
    camif_hw_stream->isp_split_output_info.left_output_width +=
      iface_util_find_number_skip(
      left_skip_horiz_pattern & ALL_ONES_16(left_residual_width));

    /*Right VFE*/
    right_shift_x = (split_info->right_stripe_offset +
      sensor_out_info->request_crop.first_pixel) % 16;
    camif_hw_stream->isp_split_output_info.right_output_width =
      (sensor_out_info->request_crop.last_pixel -
      split_info->right_stripe_offset + 1)/16 * no_of_hor_skip;
    CDBG("pdaf split_info->right_stripe_offset %d right_shift_x %d",
      split_info->right_stripe_offset, right_shift_x);
    right_residual_width = (sensor_out_info->request_crop.last_pixel -
      split_info->right_stripe_offset + 1) % 16;

    right_skip_horiz_pattern = iface_util_reverse_skip(skip_horiz_pattern);
    right_skip_horiz_pattern =
      iface_util_circularShiftLeft(right_skip_horiz_pattern, right_shift_x);

    camif_hw_stream->isp_split_output_info.right_output_width +=
      iface_util_find_number_skip(right_skip_horiz_pattern &
      ALL_ONES_16(right_residual_width));

    camif_hw_stream->isp_split_output_info.right_stripe_offset =
      ((camif_hw_stream->isp_split_output_info.left_output_width +
      (dwords -1))/dwords) * dwords;
    camif_hw_stream->isp_split_output_info.left_output_width =
      camif_hw_stream->isp_split_output_info.right_stripe_offset;
    camif_hw_stream->isp_split_output_info.right_output_width =
      ((camif_hw_stream->isp_split_output_info.right_output_width +
      (dwords - 1))/dwords) * dwords;
    bundle_pdaf_info->block_dim.width =
      camif_hw_stream->isp_split_output_info.left_output_width
      + camif_hw_stream->isp_split_output_info.right_output_width;

    camif_hw_stream->left_pixel_hor_skip = left_skip_horiz_pattern;
    camif_hw_stream->right_pixel_hor_skip = right_skip_horiz_pattern;

    shift_y = sensor_out_info->request_crop.first_line % 16;
    bundle_pdaf_info->block_dim.height = (sensor_out_info->request_crop.last_line -
      sensor_out_info->request_crop.first_line + 1)/16 * no_of_vert_skip;
    residual_height = sensor_out_info->dim_output.height % 16;
    skip_vert_pattern = iface_util_reverse_skip(skip_vert_pattern);
    skip_vert_pattern = iface_util_circularShiftLeft(skip_vert_pattern, shift_y);

    bundle_pdaf_info->block_dim.height += iface_util_find_number_skip(
      skip_vert_pattern & ALL_ONES_16(residual_height));

    camif_hw_stream->left_pixel_ver_skip = skip_vert_pattern;
    camif_hw_stream->right_pixel_ver_skip = skip_vert_pattern;

    camif_pdaf_info->camif_buffer_height = bundle_pdaf_info->block_dim.height;
    camif_pdaf_info->image_overlap = split_info->overlap;
    camif_pdaf_info->right_image_offset = split_info->right_stripe_offset;
  }

  return TRUE;
}

/** iface_utill_cfg_hw_stream_pix_raw
 *
 *    @session: iface session handle
 *    @iface: iface object
 *    @iface_sink_port: sink port
 *    @user_stream: user stream info
 *    @iface_session: session object
 *    @axi_path: axi path
 *
 *  This function prepares hw stream for camif/pix raw.
 *
 **/
int iface_utill_cfg_hw_stream_pix_raw(iface_t *iface,
  iface_sink_port_t *iface_sink_port, iface_stream_t *user_stream,
  iface_session_t *iface_session, boolean is_meta,
  iface_bundle_pdaf_info_t *bundle_pdaf_info,
  enum msm_vfe_axi_stream_src axi_path)
{
  int                        rc = 0;
  iface_hw_stream_t          *camif_hw_stream = NULL;
  uint32_t                   isp_id, i = 0, total_stride;
  pdaf_sensor_native_info_t  *pdaf_config;
  int                        dwords = 12;

  if (!iface || !iface_sink_port || !user_stream || !iface_session) {
   CDBG_ERROR("%s: failed: %p %p %p %p", __func__,
     iface, iface_sink_port, user_stream, iface_session);
   return -1;
  }
  CDBG("%s: E", __func__);


  CDBG("%s <pdaf_dbg> W %d H %d", __func__,
    iface_session->sensor_out_info.dim_output.width ,
    iface_session->sensor_out_info.dim_output.height );

  camif_hw_stream = iface_util_find_hw_stream_by_mct_id(iface,
    iface_session, user_stream->stream_id);

  if (camif_hw_stream != NULL) {
    CDBG_ERROR("%s: hw stream id already reserved by user stream id %x,return!"
          " path %d type %d wxh (%d x %d)" ,
      __func__, user_stream->stream_id, camif_hw_stream->axi_path,
      user_stream->stream_info.stream_type, user_stream->stream_info.dim.width,
      user_stream->stream_info.dim.height);
    return 0;
  }

  rc = iface_util_reserve_camif_resource(iface, iface_session, &isp_id);
  if (rc < 0) {
    CDBG_ERROR("%s: reserve vfe resource failed, rc = %d\n", __func__, rc);
    return rc;
  }

  /* after reserve session resource,
     fill output hw_stream info for camif HW stream*/
  camif_hw_stream = iface_util_get_empty_hw_stream(iface_session);

  if (!camif_hw_stream) {
    CDBG_ERROR("%s:%d: unable to find empty slot for camif hw stream\n",
      __func__, __LINE__);
    return -EINVAL;
  }

  memset(&iface_session->camif_info, 0, sizeof(iface_session->camif_info));
  rc = iface_util_fill_hw_stream_info_pix_raw(camif_hw_stream,
    iface_sink_port, user_stream, isp_id, iface_session, axi_path);
  if (rc < 0) {
    CDBG_ERROR("%s: iface_util_fill_hw_stream_info_pix_raw error, rc = %d\n",
      __func__, rc);
  }
  if (is_meta == TRUE) {
    pdaf_config = &bundle_pdaf_info->pdaf_config;
    if (pdaf_config == NULL) {
      CDBG_ERROR("%s: meta ch info = NULL, is meta = TRUE, rc = -1\n",
        __func__);
      return -1;
    }
    iface_util_config_meta_raw_size(iface_session, camif_hw_stream,
      bundle_pdaf_info);
    rc = iface_util_fill_camif_meta_info(iface_session, camif_hw_stream,
      user_stream, bundle_pdaf_info);
    if (rc < 0) {
      CDBG_ERROR("%s: iface_util_fill_meta_info error rc %d\n",
               __func__, rc);
      return rc;
    }

    dwords = iface_util_get_dwords_per_line(
      iface_util_meta_camif_format(iface_session->sensor_out_info.fmt,
      bundle_pdaf_info->camif_output_format));
    if (!iface_session->session_resource.ispif_split_info.is_split) {
      iface_session->camif_info.camif_left_buffer_width =
        bundle_pdaf_info->block_dim.width;
     iface_session->camif_info.camif_left_buffer_stride =
       (camif_hw_stream->plane_info.strides[0]/dwords * BYTES_PER_DWORD);
    } else {
      iface_session->camif_info.camif_left_buffer_width =
        camif_hw_stream->isp_split_output_info.left_output_width;
      iface_session->camif_info.camif_left_buffer_stride =
        (camif_hw_stream->isp_split_output_info.left_output_width +
        (dwords -1))/dwords * BYTES_PER_DWORD;
      iface_session->camif_info.camif_right_buffer_width =
        camif_hw_stream->isp_split_output_info.right_output_width;
      total_stride =
        camif_hw_stream->stream_info.buf_planes.plane_info.mp[0].stride;
      iface_session->camif_info.camif_right_buffer_stride = ((total_stride -
        iface_session->camif_info.camif_left_buffer_width) +
        (dwords -1))/dwords * BYTES_PER_DWORD;
      CDBG("%s: <pdaf_dbg> "
       "iface_session->camif_left_buffer_width %d,\
        iface_session->camif_info.camif_left_buffer_stride %d,\
        iface_session->camif_info.camif_right_buffer_width %d,\
        iface_session->camif_info.camif_right_buffer_stride %d \
        total_stride %d", __func__,
        iface_session->camif_info.camif_left_buffer_width,
        iface_session->camif_info.camif_left_buffer_stride,
        iface_session->camif_info.camif_right_buffer_width,
        iface_session->camif_info.camif_right_buffer_stride,
        total_stride);
    }
    switch(camif_hw_stream->camif_output_format) {
      case CAMIF_MIPI_RAW:
        iface_session->camif_info.buffer_data_type =
          PDAF_DATA_TYPE_RAW10_PACKED;
        break;
      case CAMIF_PLAIN_16:
        iface_session->camif_info.buffer_data_type =
          PDAF_DATA_TYPE_RAW16;
          break;
      case CAMIF_QCOM_RAW:
        iface_session->camif_info.buffer_data_type =
          PDAF_DATA_TYPE_RAW10_LSB;
        break;
      case CAMIF_PLAIN_8:
        iface_session->camif_info.buffer_data_type =
          PDAF_DATA_TYPE_RAW8;
        break;
      default:
        iface_session->camif_info.buffer_data_type =
          PDAF_DATA_TYPE_RAW10_LSB;
        break;
    }
    for ( i = 0; i< bundle_pdaf_info->pdaf_config.block_pattern.pix_count; i++)
    {
      bundle_pdaf_info->pdaf_config.block_pattern.pix_coords[i].x -=
       bundle_pdaf_info->pdaf_config.block_pattern.pd_offset_horizontal;
      bundle_pdaf_info->pdaf_config.block_pattern.pix_coords[i].y -=
       bundle_pdaf_info->pdaf_config.block_pattern.pd_offset_vertical;
    }
    iface_util_pdaf_get_camif_decimated_pattern(
      &bundle_pdaf_info->pdaf_config,
      &iface_session->camif_info);

#if PDAF_DEBUG
    iface_util_pdaf_dbg_info(
      &bundle_pdaf_info->pdaf_config.block_pattern,
      &iface_session->camif_info);
#endif
  }
  iface_session->num_hw_stream ++;

  return rc;
}



/** iface_util_fill_meta_info
 *
 *    @iface: iface object
 *    @iface_sink_port: sink port
 *    @rdi_hw_stream: reserving hw stream
 *    @user_stream: user stream info
 *    @meta_ch_info: session meta stream info
 *
 *  This function prepares hw stream for camif raw.
 *
 **/
int32_t iface_util_fill_meta_info(
  iface_sink_port_t *iface_sink_port,
  iface_hw_stream_t *rdi_hw_stream,
  iface_stream_t    *user_stream __unused,
  sensor_meta_t     *meta_ch_info)
{
  int32_t  rc = 0;
  uint32_t k = 0, i =0;
#ifdef _ANDROID_
  char  value[PROPERTY_VALUE_MAX];
#endif

  rdi_hw_stream->need_divert = 1;
  rdi_hw_stream->num_cids = 1; //RDI can support only 1 meta cid at a time
  rdi_hw_stream->hw_stream_id = meta_ch_info->stats_type |
    ISP_NATIVE_BUF_BIT | ISP_META_CHANNEL_BIT;
  rdi_hw_stream->divert_to_3a = TRUE;
  rdi_hw_stream->use_native_buf = 1;
  rdi_hw_stream->controllable_output = 0;
  rdi_hw_stream->stream_info.dim.width = meta_ch_info->dim.width;
  rdi_hw_stream->stream_info.dim.height = meta_ch_info->dim.height;
  rdi_hw_stream->stream_info.fmt = meta_ch_info->fmt;
  rdi_hw_stream->buffers_info[VFE_BUF_QUEUE_DEFAULT].num_bufs =
    IFACE_MAX_NATIVE_BUF_NUM;
#ifdef _ANDROID_
      property_get("persist.vendor.camera.isp.buf_cnt", value, "IFACE_MAX_NATIVE_BUF_NUM");
      if(atoi(value)){
          rdi_hw_stream->buffers_info[VFE_BUF_QUEUE_DEFAULT].num_bufs =
            atoi(value);
      }
#endif
  rdi_hw_stream->ext_stats_type = meta_ch_info->stats_type;

  for (k = 0, i = 0; k < iface_sink_port->sensor_cap.num_meta_ch
    && k < SENSOR_CID_CH_MAX && i < SENSOR_CID_CH_MAX; k++) {
    if (iface_sink_port->sensor_cap.meta_ch[k].dt ==
      meta_ch_info->dt) {
      rdi_hw_stream->cids[i] =
        iface_sink_port->sensor_cap.meta_ch[k].cid;
      rdi_hw_stream->csid =
        iface_sink_port->sensor_cap.meta_ch[k].csid;
      if (!rdi_hw_stream->use_pix) {
        rdi_hw_stream->pack_mode[rdi_hw_stream->cids[i]] =
          iface_sink_port->sensor_cap.meta_ch[k].pack_mode;
        CDBG("%s: cid %d pack mode %d \n", __func__,
          iface_sink_port->sensor_cap.meta_ch[k].cid,
          rdi_hw_stream->pack_mode[rdi_hw_stream->cids[i]]);
      } else
        rdi_hw_stream->pack_mode[rdi_hw_stream->cids[i]]=
          PACK_MODE_BYTE;
      i++;
    }
  }
  if (i == 0) {
    CDBG_ERROR("%s: No matching meta channel found \n", __func__);
    return -1;
  }
  rdi_hw_stream->stream_type = IFACE_STREAM_TYPE_META_DATA;

  iface_util_modify_plane_info_for_native_buf(rdi_hw_stream);
  iface_util_fill_hw_stream_plane_info(rdi_hw_stream,
    &rdi_hw_stream->stream_info.buf_planes.plane_info);
  CDBG("%s: wd x ht= %d x %d fmt= %d hw_stream_id %d\n", __func__,
    rdi_hw_stream->stream_info.dim.width,
    rdi_hw_stream->stream_info.dim.height,
    rdi_hw_stream->stream_info.fmt,
    rdi_hw_stream->hw_stream_id);
  return rc;
}

/** iface_util_set_hw_stream_cfg_rdi
 *
 * DESCRIPTION:
 *
 **/
int32_t iface_util_set_hw_stream_cfg_rdi(iface_t *iface,
  iface_port_t *iface_sink_port, iface_stream_t *user_stream,
  iface_session_t *iface_session, boolean is_meta, sensor_meta_t *meta_ch_info)
{
  int32_t                rc = 0;
  uint32_t               isp_id = 0;
  uint32_t               mask = 0;
  iface_hw_stream_t     *rdi_hw_stream = NULL;
  iface_intf_type_t      reserved_rdi_intf = IFACE_INTF_MAX;
  isp_preferred_streams  *preferred_mapping;

  if (!iface || !iface_sink_port || !user_stream || !iface_session) {
    CDBG_ERROR("%s: failed: %p %p %p %p", __func__,
      iface, iface_sink_port, user_stream, iface_session);
    return -1;
  }
  CDBG("%s: user_stream->axi_path  %d stram id %x ", __func__,
    user_stream->axi_path, user_stream->stream_id);

  preferred_mapping = &iface_session->preferred_mapping;
  mask = iface_util_get_preferred_mask_by_type(preferred_mapping,
    user_stream->stream_info.stream_type);
  rdi_hw_stream = iface_util_get_hw_stream_by_mask(iface_session,
    iface_sink_port, mask);
  if (rdi_hw_stream) {
    CDBG_HIGH("%s:io_dbg hw_stream existed!\n", __func__);
    rdi_hw_stream->mapped_mct_stream_id[rdi_hw_stream->num_mct_stream_mapped] =
      user_stream->stream_id;
    rdi_hw_stream->num_mct_stream_mapped++;
  } else {
    /*reserve RDI resource, save info in session and rdi streams,
      get which isp and which rdi we are reserving, pass to hw stream config*/
    rc = iface_util_reserve_rdi_resource(iface, iface_session,
      &reserved_rdi_intf, &isp_id);
    if (rc < 0) {
      CDBG_ERROR("%s: reserve RDI failed, rc = %d\n", __func__, rc);
      return rc;
    }

    /*after reserve resource, set clk rate, formula from system team*/
    iface->isp_axi_data.axi_data[isp_id].interface_clk_request[reserved_rdi_intf]
      = user_stream->stream_info.dim.width * (1 + IFACE_LINE_TIME_MARGIN) *
        iface_sink_port->u.sink_port.sensor_out_info.vt_pixel_clk /
        iface_sink_port->u.sink_port.sensor_out_info.ll_pck / 16;

    rdi_hw_stream = iface_util_fill_hw_stream_info_rdi(iface_session,
      &iface_sink_port->u.sink_port, user_stream, reserved_rdi_intf, isp_id);
    if (!rdi_hw_stream) {
      CDBG_ERROR("%s: fill_hw_stream_info_rdi error, is_meta = %d\n",
                 __func__, is_meta);
      return -1;
    }
    if (is_meta == TRUE) {
      if (meta_ch_info == NULL) {
        CDBG_ERROR("%s: meta ch info = NULL, is meta = TRUE, rc = -1\n",
          __func__);
        return -1;
      }

      rc = iface_util_fill_meta_info(&iface_sink_port->u.sink_port, rdi_hw_stream,
        user_stream, meta_ch_info);
      if (rc < 0) {
        CDBG_ERROR("%s: iface_util_fill_meta_info error rc %d\n",
                 __func__, rc);
        return rc;
      }

      /* If any hw stream is in batch mode put metadata streams in subsampling mode */
      rdi_hw_stream->frame_skip_pattern =
            iface_util_get_hfr_skip_pattern(iface_session);
      if (iface_session->batch_info.batch_mode &&
        rdi_hw_stream->stream_info.streaming_mode != CAM_STREAMING_MODE_BATCH) {
        if (iface_session->batch_info.batch_mode)
          rdi_hw_stream->frame_skip_pattern =
            iface_session->batch_info.batch_size - 1;
      }
    }
  }

  if (!is_meta) {
    /* Set this RDI intf as main frame intf if YUV sensor or only RAW stream in session */
    /* Does not support RDI on dual vfe at the moment */
    if (iface_session->ms_type != MS_TYPE_NONE &&
      (user_stream->stream_info.stream_type == CAM_STREAM_TYPE_PREVIEW ||
      user_stream->stream_info.stream_type == CAM_STREAM_TYPE_DEPTH    ||
      user_stream->stream_info.stream_type == CAM_STREAM_TYPE_VIDEO) &&
      (iface_util_sensor_is_yuv(&iface_sink_port->u.sink_port) ||
      (iface_session->session_resource.camif_cnt == 0))) {
      if (rdi_hw_stream->interface_mask & 0xFFFF) {
        isp_id = 0;
      } else if (rdi_hw_stream->interface_mask & 0xFFFF0000) {
        isp_id = 1;
      } else {
        CDBG_ERROR("%s: Error! Invalid interface mask 0x%x\n", __func__,
          rdi_hw_stream->interface_mask);
        return -1;
      }
      /* These 2 enums are assumed to be similar */
      iface_session->session_resource.main_frame_intf =
        SRC_TO_INTF(rdi_hw_stream->axi_path);
      iface_session->session_resource.main_frame_vfe = isp_id;
      CDBG_ERROR("%s: Assigned vfe %d intf %d for related cam type %d\n",
        __func__, isp_id, reserved_rdi_intf,
        iface_session->ms_type);
    }
  }

  CDBG("%s: num_hw_streams %d", __func__, iface_session->num_hw_stream);
  return rc;
}

/** iface_util_find_hw_stream_by_mct_id
 *    @stream_id:
 *    @hfr_mode:
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
iface_hw_stream_t *iface_util_find_hw_stream_by_mct_id(
  iface_t         *iface __unused,
  iface_session_t *session,
  uint32_t         mct_stream_id)
{
  int i = 0;
  int j = 0;
  iface_hw_stream_t *hw_stream;

  if (!session) {
    CDBG_ERROR("%s: can not find session \n", __func__);
    return NULL;
  }

  for (i = 0; i < IFACE_MAX_STREAMS; i++) {
    hw_stream = &session->hw_streams[i];
    if (hw_stream == NULL) {
      CDBG("%s: hw stream  = NULL\n", __func__);
      continue;
    }
    for (j = 0; j < IFACE_MAX_STREAMS; j++) {
       if (mct_stream_id == hw_stream->mapped_mct_stream_id[j])
          return hw_stream;
    }
  }

  return NULL;
}

/** iface_util_map_cam_sync_type_to_iface
 *    @sync_type: cam_sync_type value from HAL
 *
 * Return: enum msm_vfe_dual_hw_ms_type
 **/
void iface_util_map_cam_sync_type_to_iface(
  iface_session_t *session,
  cam_sync_type_t sync_type)
{
  enum msm_vfe_dual_hw_ms_type ms_type;

  switch (sync_type) {
  case CAM_TYPE_MAIN:
    session->ms_type = MS_TYPE_MASTER;
    break;
  case CAM_TYPE_AUX:
    session->ms_type = MS_TYPE_SLAVE;
    break;
  default:
    session->ms_type = MS_TYPE_NONE;
    return;
  }
}

void iface_util_map_cam_sync_mode_to_iface(
   iface_session_t *session,
   cam_sync_mode_t sync_mode)
{
  enum msm_vfe_dual_hw_ms_type ms_type;

  switch (sync_mode) {
  case CAM_MODE_PRIMARY:
    session->ms_type = MS_TYPE_MASTER;
    break;
  case CAM_MODE_SECONDARY:
    session->ms_type = MS_TYPE_SLAVE;
    break;
  default:
    session->ms_type = MS_TYPE_NONE;
    return;
  }

  { /* override to disable ISP frame-matching */
    char prop[255];
    int override = 0;
    property_get("persist.vendor.camera.dualcam.idsyncoff", prop, "0");
    override = atoi(prop);
    if (override) {
      session->ms_type = MS_TYPE_NONE;
    }
  } /* override to disable ISP frame-matching */
}

/** iface_util_process_hw_error
 *    @iface: iface handle
 *    @mct_bus_data
 *
 * Post hw error message to MCT bus
 *
 * Return: 0 - success and negative value - failure
 **/
static int iface_util_process_hw_error(iface_t *iface,
  mct_bus_msg_t *mct_bus_data)
{
  int               rc = 0;
  iface_session_t  *session = NULL;

  if (!iface || !mct_bus_data) {
    CDBG_ERROR("%s: NULL  pointer %p %p, rc = -1\n",
     __func__, iface);
    return -1;
  }

  session =
    iface_util_get_session_by_id(iface, mct_bus_data->sessionid);
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n", __func__, mct_bus_data->sessionid);
    return -1;
  }

  pthread_mutex_lock(&session->mutex);
  session->overflow_recovery_state =
    IFACE_AXI_RECOVERY_STATE_FORCE_HALT;
  pthread_mutex_unlock(&session->mutex);

  rc = mct_module_post_bus_msg(iface->module, mct_bus_data);
  return rc;
}

/** iface_util_send_divert_ack
 * @port: port on which ack to be sent
 *  @buff_divert_ack: payload
 * DESCRIPTION: send ack for fe input bufq
 *
 **/
static int iface_util_send_divert_ack(mct_port_t *port,
    isp_buf_divert_ack_t *buff_divert_ack)
{

  mct_event_t mct_event;
  boolean     ret;

  memset(&mct_event, 0, sizeof(mct_event));
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.identity = buff_divert_ack->identity;
  mct_event.direction = MCT_EVENT_DOWNSTREAM;
  mct_event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT_ACK;
  mct_event.u.module_event.module_event_data = (void *)buff_divert_ack;

  ret = mct_port_send_event_to_peer(port, &mct_event);
  if (FALSE == ret) {
    CDBG_ERROR("%s: failed to send MCT_EVENT_MODULE_BUF_DIVERT_ACK", __func__);
    return -1;
  }

  return 0;
}

/** int iface_util_in_queue_put:
 *
 *  @session : session data
 *  @fe_in_buf: fe input buffer description
 *
 *  Queue input frame
 *
 *  Return 0 on success and -1 on failure
 **/
int iface_util_in_queue_put(iface_session_t *session,
  iface_util_fe_input_buf_t *fe_in_buf)
{
  iface_util_fe_input_buf_t *temp;

  if (session->fe.num >= IFACE_UTIL_INPUT_Q_SIZE) {
    CDBG_HIGH("%s:frame queue is full %d\n", __func__, session->fe.num);
    return -1;
  }

  temp = &session->fe.q_data[session->fe.next_free_entry];
  session->fe.next_free_entry =
    (session->fe.next_free_entry + 1) % IFACE_UTIL_INPUT_Q_SIZE;

  memcpy(temp, fe_in_buf, sizeof(*temp));

  mct_queue_push_tail(session->fe.frame_q, (void *)temp);
  session->fe.num++;

  return 0;
}

/** iface_process_internal_fetch_engine:
 *
 *    @iface: iface
 *    @session_id : session id
 *    @hw_stream_id: hw stream id
 *    @buf_idx: buf index
 *  This function configures iface hw streams and save in
 *  session
 *
 *  Return: 0 for success and negative error on failure
 **/
int iface_process_internal_fetch_engine(
  iface_t                  *iface,
  iface_session_t          *session,
  uint32_t                  buf_stream_id,
  uint32_t                  isp_id,
  uint32_t                 buf_idx,
  uint32_t                 frame_id)
{
  int                    rc           = 0;
  uint32_t               fd           = 0;
  uint8_t                offline_mode = 0;
  iface_hw_stream_t     *hw_stream    = NULL;
  triger_fetch_stream_t  fetch_params;

  if (!session) {
    CDBG_ERROR("%s: cannot find session is NULL \n",
        __func__);
    return -1;
  }
  memset(&fetch_params, 0, sizeof(triger_fetch_stream_t));
  fetch_params.session_id = session->session_id;
  fetch_params.hw_stream_id= buf_stream_id;
  fetch_params.buf_idx= buf_idx;
  fetch_params.offline_mode = FALSE;
  fetch_params.frame_id = frame_id;

  CDBG("%s reprocess offline mode %d, frame_id %d\n",
            __func__, fetch_params.offline_mode, frame_id);

  rc = iface->isp_axi_data.axi_data[isp_id].axi_hw_ops->action(
    iface->isp_axi_data.axi_data[isp_id].axi_hw_ops->ctrl,
    IFACE_AXI_ACTION_CODE_FETCH_START, (void *)&fetch_params,
    sizeof(triger_fetch_stream_t));
  return rc;
}

/** iface_util_process_raw_frame
 *    @iface: ISP pointer
 *    @session: session pointer
 *
 * Return: 0 - success, negative vale - error.
 **/

int iface_util_process_raw_frame(iface_t *iface,
   iface_session_t *session,
   iface_util_fe_input_buf_t *fe_in_buf,
   int buffer_access)
{
   uint32_t user_bufq_handle = 0;
   int isp_id, rc = 0;

   if (!iface || !fe_in_buf || !session) {
     CDBG_ERROR("%s: NULL pointer %p %p %p\n", __func__,
       iface, fe_in_buf, session);
     return -1;
   }
   if (!session->bayer_processing)
       return 0;


  iface_do_cache_inv(fe_in_buf->user_bufq_handle,
      &iface->buf_mgr,
      iface->buf_mgr.ion_fd,
      fe_in_buf->buf_idx,
      buffer_access & CPU_HAS_WRITTEN);

   for (isp_id = 0; isp_id < VFE_MAX; isp_id++) {
     if (fe_in_buf->interface_mask & (1 << (16 * isp_id +
       IFACE_INTF_PIX))) {
         iface_process_internal_fetch_engine(iface,
          session, fe_in_buf->hw_stream_id, isp_id,
          fe_in_buf->buf_idx, fe_in_buf->frame_id);
     }
   }

   session->fe.busy = TRUE;
   return rc;
}

/** int iface_util_in_queue_get:
 *
 *  @session : session data
 *  @fe_in_buf: fe input buffer description
 *
 *  Dequeue input frame
 *
 *  Return 0 on success and -1 on failure
 **/
int iface_util_in_queue_get(iface_session_t *session,
  iface_util_fe_input_buf_t **fe_in_buf)
{
  if (!session->fe.num) {
    CDBG_HIGH("%s:frame queue is empty\n", __func__);
    return -1;
  }

  *fe_in_buf = mct_queue_pop_head(session->fe.frame_q);
  if (!(*fe_in_buf)) {
    CDBG_HIGH("%s:error null ptr\n", __func__);
    return -1;
  }
  session->fe.num--;

  return 0;
}

/** iface_util_schedule_fe
 *    @iface: mct module handle
 *    @session: session data
 *
 *  Handle pending requests
 *
 * Return TRUE on success and FALSE if fails
 **/
static int iface_util_schedule_fe(iface_t *iface, iface_session_t *session)
{
  iface_util_fe_input_buf_t *fe_in_buf = NULL;
  int rc = 0;
  pthread_mutex_lock(&session->fe.mutex);
  session->fe.busy = FALSE;

  if (!session->fe.num) {
    /* no pending requests */
    pthread_mutex_unlock(&session->fe.mutex);
    return 0;
  }

  rc = iface_util_in_queue_get(session, &fe_in_buf);
  if (rc < 0) {
    CDBG_ERROR("%s: failed: iface_util_in_queue_get\n", __func__);
    pthread_mutex_unlock(&session->fe.mutex);
    return -1;
  }

  rc = iface_util_process_raw_frame(iface, session, fe_in_buf, 0);
  if (rc < 0) {
    CDBG_ERROR("%s:%d: fail to start fe", __func__, __LINE__);
    pthread_mutex_unlock(&session->fe.mutex);
    return -1;
  }
  pthread_mutex_unlock(&session->fe.mutex);

  return 0;
}

/** iface_util_handle_buffer_divert
 *    @module: mct module handle
 *    @event: module event
 *
 *  Handle buffer divert
 *
 * Return TRUE on success and FALSE if fails
 **/
int iface_util_handle_buffer_divert(iface_t *iface, mct_event_t *event)
{
  uint32_t  i = 0;
  boolean rc = FALSE;
  struct msm_isp_event_data  buf_divert;
  iface_util_fe_input_buf_t fe_in_buf;
  iface_hw_stream_t  *hw_stream = NULL;
  mct_event_module_t *mod_event = &event->u.module_event;
  iface_session_t *session = iface_util_get_session_by_id(iface,
        UNPACK_SESSION_ID(event->identity));
  isp_buf_divert_t *buf_divert_event = (isp_buf_divert_t*)
                    mod_event->module_event_data;
  if (!session) {
    CDBG_ERROR("can not find session %d", UNPACK_SESSION_ID(event->identity));
    return -1;
  }
  for (i = 0; i < session->num_hw_stream; i++) {
    hw_stream = &session->hw_streams[i];
    if (hw_stream->axi_path < CAMIF_RAW) /*use pix hw stream for offline processing*/
        break;
  }
  if(hw_stream) {

  CDBG("hw_stream_id = %x, axi_path = %d, buf_idx = %d frame_id %d",
      hw_stream->hw_stream_id, hw_stream->axi_path,
      buf_divert_event->buffer.index, buf_divert_event->buffer.sequence);
  fe_in_buf.buf_idx = buf_divert_event->buffer.index;
  fe_in_buf.frame_id = buf_divert_event->buffer.sequence;
  fe_in_buf.hw_stream_id =  session->reproc_buff_info.user_stream_id;
  fe_in_buf.user_bufq_handle = session->reproc_buff_info.bufq_handle;

  fe_in_buf.interface_mask = hw_stream->interface_mask;

  /* Check if FE is ready for next frame */
  pthread_mutex_lock(&session->fe.mutex);
  if (session->fe.busy) {
     /* Fetch engine is not ready for start push buff in queue */
     rc = iface_util_in_queue_put(session, &fe_in_buf);
     if (rc < 0) {
        CDBG_ERROR("fail to put input frame in fe queue, sending piggyback");
        buf_divert_event->ack_flag = 1;
        pthread_mutex_unlock(&session->fe.mutex);
        return -1;
     }
     CDBG_ERROR("fe.busy, frame_id %d queued", fe_in_buf.frame_id);
  } else {
      mct_bus_msg_isp_sof_t sof_event;
      sof_event.frame_id = buf_divert_event->buffer.sequence;
      /*send request frame if any*/
      if (session->frame_request.hw_stream_id) {
         rc = iface_util_request_frame(session, iface, &session->frame_request);
         if (rc) {
            CDBG_ERROR("%s: failed: iface_util_request_frame\n", __func__);
         }
         session->frame_request.hw_stream_id = 0;
      }
      /*send fe sof */
      iface_util_broadcast_fe_sof_msg_to_modules(iface,
        session->session_id, UNPACK_STREAM_ID(event->identity), &sof_event);

      rc = iface_util_process_raw_frame(iface, session, &fe_in_buf,
          buf_divert_event->buffer_access);
      buf_divert_event->buffer_access = 0;

     if (rc < 0) {
       CDBG_ERROR("fail to start fe");
       pthread_mutex_unlock(&session->fe.mutex);
       return -1;
      }
   }
  pthread_mutex_unlock(&session->fe.mutex);
  }
  return rc;
}

/** iface_util_process_fe_read_done
 *    @iface: iface handle
 *    @mct_bus_data
 *
 *
 * Return: 0 - success and negative value - failure
 **/
static int iface_util_process_fe_read_done(iface_t *iface,
  mct_bus_msg_t *mct_bus_data)
{
  int   rc = 0, i = 0;
  iface_session_t   *session = NULL;
  iface_hw_stream_t *hw_stream = NULL;
  boolean post_fe_done = TRUE;
  iface_bufq_t     *bufq = NULL;
  iface_stream_t   *user_stream = NULL;
  uint32_t          stream_id = 0;
  isp_buf_divert_ack_t        buff_divert_ack;
  struct msm_isp_event_data  *rd_done_event;
  mct_bus_msg_isp_rd_done_t   mct_isp_rd_done_event;

  if (!iface || !mct_bus_data) {
    CDBG_ERROR("%s: NULL  pointer %p \n",
      __func__, iface);
    return -1;
  }

  session = iface_util_get_session_by_id(iface, mct_bus_data->sessionid);
  if (!session) {
    CDBG_ERROR("%s: NULL session pointer %p session_id %d\n",
      __func__, session, mct_bus_data->sessionid);
    return -1;
  }

  if (session->session_resource.offline_split_info.is_split) {
     post_fe_done = FALSE;
     for (i = 0; i < IFACE_MAX_STREAMS; i++) {
       hw_stream = &session->hw_streams[i];
       if (hw_stream->stream_info.cam_stream_type ==
         CAM_STREAM_TYPE_OFFLINE_PROC ) {
         session->fe_done_cnt++;
         if (session->fe_done_cnt == IFACE_MAX_OFFLINE_PASS) {
           post_fe_done = TRUE;
           session->fe_done_cnt = 0;
         } else {
           return -1;
         }
       }
     }
  }
  rd_done_event = (struct msm_isp_event_data *)mct_bus_data->msg;
  if (session->bayer_processing) {
     iface_util_schedule_fe(iface, session);
      for (i = 0; i < IFACE_MAX_NUM_BUF_QUEUE; i++) {
        bufq = &iface->buf_mgr.bufq[i];
        if (rd_done_event->u.buf_done.handle == bufq->kernel_bufq_handle) {
        break;
       }
      }

     /* return buffer to source mudule */
#ifdef ISP_SVHDR_OUT_BIT
     stream_id = rd_done_event->u.buf_done.stream_id &
         ~(ISP_SVHDR_OUT_BIT | ISP_NATIVE_BUF_BIT);
#endif
     user_stream = iface_util_find_stream_in_sink_port_list(iface,
       session->session_id, stream_id);
     if (!user_stream) {
       CDBG_ERROR("%s: can not find user stream %x!\n", __func__, stream_id);
       return -1;
     }
     post_fe_done = FALSE;
     memset(&buff_divert_ack, 0, sizeof(buff_divert_ack));
     buff_divert_ack.buf_idx = rd_done_event->u.buf_done.buf_idx;
     buff_divert_ack.identity = pack_identity(mct_bus_data->sessionid,
       user_stream->stream_id);
     buff_divert_ack.is_buf_dirty = 0;
     buff_divert_ack.frame_id = rd_done_event->frame_id;
     buff_divert_ack.bayerdata = 1;

     CDBG("sending divert_ack on prot %s, identity %x, frame_id %d",
        MCT_PORT_NAME(user_stream->src_port), buff_divert_ack.identity,
        buff_divert_ack.frame_id);
     rc = iface_util_send_divert_ack(user_stream->src_port, &buff_divert_ack);
     if (rc < 0) {
       CDBG_ERROR("%s: failed: iface_util_send_divert_ack", __func__);
       return rc;
     }
   }
  if (post_fe_done) {
     // send the event to mct about fe done with struct mct_bus_msg_isp_rd_done_t
      mct_isp_rd_done_event.frame_id = session->offline_info.offline_frame_id;
      mct_isp_rd_done_event.timestamp = rd_done_event->timestamp;
      mct_bus_data->msg = (void *)&mct_isp_rd_done_event;
      mct_bus_data->size = sizeof(mct_isp_rd_done_event);
      rc = mct_module_post_bus_msg(iface->module, mct_bus_data);
  }
  return rc;
}

/** iface_util_process_overflow_recovery
 *    @iface: iface handle
 *    @session id: session id
 *    @isp_id: ISP on which overflow detected
 *
 * Return: 0 - success and negative value - failure
 **/
static int iface_util_process_overflow_recovery(iface_t *iface,
  iface_halt_recovery_info_t *halt_recovey_info, int isp_id)
{
  int                         rc = 0;
  iface_session_t            *session = NULL;
  iface_port_t               *iface_sink_port = NULL;

  if (!iface || !halt_recovey_info) {
    CDBG_ERROR("%s: NULL  pointer %p %p, rc = -1\n",
      __func__, iface, halt_recovey_info);
    return IFACE_AXI_RECOVERY_STATE_RECOVERY_FAILED;
  }

  CDBG_HIGH("%s:%d overflow detected on VFE%d \n", __func__, __LINE__, isp_id);

  session =
    iface_util_get_session_by_id(iface, halt_recovey_info->session_id);
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n", __func__, halt_recovey_info->session_id);
    return IFACE_AXI_RECOVERY_STATE_RECOVERY_FAILED;
  }

  iface_sink_port = iface_util_find_sink_port_by_session_id(iface,
    &halt_recovey_info->session_id);
  if (!iface_sink_port) {
    CDBG_ERROR("%s: can not find iface sink port for this session, rc = -1\n",
      __func__);
    return -1;
  }
  if (session->active_count == 0) {
    CDBG_ERROR("%s: on polling thread, active count = 0, drop recovery!!!\n",
      __func__);
    return 0;
  }

  pthread_mutex_lock(&session->mutex);
    if ((session->overflow_recovery_state ==
        IFACE_AXI_RECOVERY_STATE_FORCE_HALT) ||
        (session->overflow_recovery_state ==
        IFACE_AXI_RECOVERY_STATE_OVERFLOW_DETECTED)) {
      CDBG_ERROR("%s: can not take new recovery request! overflow state %d\n",
        __func__, session->overflow_recovery_state);
      pthread_mutex_unlock(&session->mutex);
      return 0;
  }
  session->overflow_recovery_state =
    IFACE_AXI_RECOVERY_STATE_OVERFLOW_DETECTED;
  pthread_mutex_unlock(&session->mutex);

  rc = iface_halt_recovery_to_thread(iface,
    halt_recovey_info, (uint32_t)isp_id);
  if (rc < 0) {
    CDBG_ERROR("%s: iface_halt_recoveryto thread failed!, rc = %d\n",
      __func__, rc);
    return rc;
  }

  return rc;
}

/** iface_util_hw_notify_buf_drop_in_meta
 *
 * DESCRIPTION:
 * This function reports the frame drop in the meta buffer
 *
 **/
static int iface_util_hw_notify_buf_drop_in_meta(
  iface_t             *iface,
  uint32_t             session_id,
  iface_axi_framedrop *axi_framedrop,
  int                  axi_idx __unused)
{
  int rc = 0;
  uint32_t i = 0;
  uint32_t j = 0;
  iface_session_t *session = iface_util_get_session_by_id(iface, session_id);
  mct_bus_msg_iface_metadata_t iface_metadata;
  uint32_t q_idx = 0;
  iface_hw_stream_t *hw_stream;

  if (!session) {
    CDBG_ERROR("%s: can not find session, session_id = %d\n", __func__, session_id);
    return -1;
  }
  memset(&iface_metadata, 0, sizeof(iface_metadata));

  iface_metadata.type = MCT_BUS_MSG_FRAME_DROP;
  /* the reporting delay in the queue is 2 */
  q_idx = ((axi_framedrop->frame_id + IFACE_APPLY_DELAY + IFACE_APPLY_DELAY) % IFACE_FRAME_CTRL_SIZE);

  for (i = 0; i < axi_framedrop->framedrop.num_streams
    && i < MAX_NUM_STREAMS; i++) {
    if (axi_framedrop->controllable_output) {
        iface_metadata.frame_drop.stream_request[iface_metadata.
          frame_drop.num_streams++].streamID =
            axi_framedrop->framedrop.stream_request[i].streamID;
        CDBG_HIGH("%s: Send buf drop in meta for controllable stream (0x%x, fid: 0x%x)",
            __func__, axi_framedrop->framedrop.stream_request[i].streamID,
            axi_framedrop->frame_id);
    } else {
      hw_stream =
        iface_util_find_hw_stream_in_session(
            session, axi_framedrop->framedrop.stream_request[i].streamID);
      if (hw_stream != NULL && !hw_stream->controllable_output) {
        for (j = 0; j < hw_stream->num_mct_stream_mapped &&
          j < IFACE_MAX_STREAMS; j++) {
          iface_metadata.frame_drop.stream_request[iface_metadata.frame_drop.num_streams++].streamID =
            hw_stream->mapped_mct_stream_id[j];
          CDBG_HIGH("%s: Send buf drop in meta (0x%x, fid: 0x%x)",
            __func__, hw_stream->mapped_mct_stream_id[j],
            axi_framedrop->frame_id);
        }
      } else {
        CDBG_ERROR("%s: Error! Could not find hw_stream for session %d hw_stream_id 0x%x hw_stream %p\n",
          __func__, session_id, axi_framedrop->framedrop.stream_request[i].streamID, hw_stream);
        continue;
      }
    }
  }

  if (TRUE != iface_store_per_frame_metadata(session, &iface_metadata, q_idx)) {
    CDBG_ERROR("%s:%d failed: iface_store_per_frame_metadata\n", __func__,
       __LINE__);
    rc = -1;
  }

  return rc;
}

/** iface_util_set_vt
 *    @session_id
 *    @vt_enbale:
 *  Sets the vt_enable flag
 *
 * Return: 0- success and negative value - failure
 **/
int iface_util_set_vt(iface_t *iface, uint32_t session_id, int32_t *vt_enable)
{
  iface_session_t *session = iface_util_get_session_by_id(iface, session_id);

  if (!session) {
    CDBG_ERROR("%s: can not find session, session_id = %d\n", __func__, session_id);
    return -1;
  }
  session->vt_enable = *vt_enable;
  return 0;
}

/** iface_util_set_dis_enable_flag
 *    @session_id
 *    @dis_enable_flag:
 *  Sets the dis flag
 *
 * Return: 0- success and negative value - failure
 **/
int iface_util_set_dis_enable_flag(iface_t *iface, uint32_t session_id,
                         int32_t *dis_enable_flag)

{
  iface_session_t *session = iface_util_get_session_by_id(iface, session_id);

  if (!session) {
    CDBG_ERROR("%s: can not find session, session_id = %d\n", __func__, session_id);
    return -1;
  }
  session->dis_enabled = *dis_enable_flag;
  CDBG_INFO("%s:%d DIS enable flag = %d\n", __func__, __LINE__,
        session->dis_enabled);
  return 0;
}

/** iface_util_set_sensor_hdr
 *    @session_id
 *    @dhdr_mode:
 *  sets hdr_mode
 *
 * Return: 0- success and negative value - failure
 **/
int iface_util_set_sensor_hdr(iface_t *iface, uint32_t session_id, uint32_t stream_id,
                         cam_sensor_hdr_type_t *hdr_mode)

{
  iface_session_t *session = iface_util_get_session_by_id(iface, session_id);
  iface_hw_stream_t     *hw_stream = NULL;
  iface_stream_t        *user_stream = NULL;
  mct_port_t            *isp_sink_port = NULL;
  mct_event_t            event;
  isp_hdr_mode_t         isp_hdr_mode;
  uint32_t               i=0;

  if (!session) {
    CDBG_ERROR("%s: can not find session, session_id = %d\n", __func__, session_id);
    return -1;
  }

  memset(&event, 0, sizeof(mct_event_t));
  memset(&isp_hdr_mode , 0 , sizeof(isp_hdr_mode_t));

  session->hdr_mode = *hdr_mode;
  isp_hdr_mode.hdr_mode = *hdr_mode;

  if (!session->zzhdr_hw_available && *hdr_mode == CAM_SENSOR_HDR_ZIGZAG){
    isp_hdr_mode.hdr_mode  = CAM_SENSOR_HDR_IN_SENSOR;
  }

  CDBG("%s:%d hdr_mode = %d\n", __func__, __LINE__,
        session->hdr_mode);

  user_stream = iface_util_find_stream_in_sink_port_list(iface,
    session->session_id, stream_id);
  if (!user_stream || !user_stream->src_port) {
    CDBG_ERROR("%s:%d NULL pointer  rc = -1\n",
      __func__, __LINE__);
    return -1;
  }

  isp_sink_port = user_stream->src_port->peer;
  if (!isp_sink_port) {
    CDBG_ERROR("%s:%d isp_sink_port %p rc = -1\n", __func__, __LINE__, isp_sink_port);
    return -1;
  }

  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_HDR;
  event.u.module_event.module_event_data = (void *)&isp_hdr_mode;
  event.identity = pack_identity(session->session_id, user_stream->stream_id);
  event.direction = MCT_EVENT_DOWNSTREAM;


  if (FALSE == isp_sink_port->event_func(isp_sink_port, &event)) {
    CDBG_ERROR("%s: error in isp sink port event\n", __func__);
    return -1;
  }

  return 0;
}

/** iface_util_handle_meta_stream_info
 *    @iface
 *    @session_id
 *    @stream_id
 *    @stream_desc:
 *  Handles the meta stream info for native buf allocation purpose mostly
 *
 * Return: 0- success and negative value - failure
 **/
int iface_util_handle_meta_stream_info(iface_t *iface,
                uint32_t session_id, uint32_t stream_id,
                cam_stream_size_info_t *streams_desc)
{
  iface_session_t *session = iface_util_get_session_by_id(iface, session_id);
  uint32_t i=0, j=0;
  int rc = 0;
  boolean b_native_alloc = FALSE;
  iface_buf_alloc_t buf_alloc_info;
  pp_meta_stream_info_request_t pp_stream_info;
  enum msm_vfe_axi_stream_src tmp_axi_src = VFE_AXI_SRC_MAX;
  cam_format_t    tmp_cam_fmt[VFE_AXI_SRC_MAX];
  memset(&(session->nativbuf_alloc_info), 0, sizeof(iface_buf_alloc_t));
  memset(&(tmp_cam_fmt), CAM_FORMAT_MAX, sizeof(tmp_cam_fmt));

  if (!session || !streams_desc) {
    CDBG_ERROR("%s: can not find session, session_id = %d streams_desc = %p\n",
      __func__, session_id, streams_desc);
    return -1;
  }

  if (streams_desc->num_streams >= MAX_NUM_STREAMS) {
    CDBG_ERROR("%s: Invalid stream desc argument, num streams %d\n",
      __func__, streams_desc->num_streams);
    return -1;
  }

  if (session->ms_type == MS_TYPE_NONE)
    iface_util_map_cam_sync_type_to_iface(
      session, streams_desc->sync_type);
  IFACE_HIGH("%s: ms_type %d num streams %d\n", __func__, session->ms_type,
            streams_desc->num_streams);

  session->hal_max_buffer = streams_desc->buffer_info.max_buffers;

  CDBG("%s:num_streams %d \n", __func__, streams_desc->num_streams);
  if (session->hvx.enabled == TRUE &&
    session->hvx.state >= IFACE_STATE_OPENED &&
    session->hvx.state <= IFACE_STATE_MAX) {
      /* Deinit HVX */
      iface_hvx_close(&iface->hvx_singleton, &session->hvx);
   }

    /* clean up the allocated native buffer */
  for (i=0; i<VFE_AXI_SRC_MAX; i++) {
    iface_release_native_buf_handles(&iface->buf_mgr, IFACE_MAX_IMG_BUF,
      session->image_bufs[i]);
  }

  if (!streams_desc->num_streams) {
    CDBG_HIGH("%s:%d num_streams is 0 no need to allocate buffer\n",
      __func__, __LINE__);
    return 0;
  }

  if (session->dis_enabled) {
    CDBG_HIGH("%s:%d DIS enabled, no native buf opt\n", __func__, __LINE__);
    return 0;
  }

  if (streams_desc->batch_size > 1) {
    CDBG_ERROR("%s:%d batch mode enabled with size=%d, no native buf opt\n",
      __func__, __LINE__, streams_desc->batch_size);
    return 0;
  }

  // Here, we calculate the native buff size and allocate native buffers
  memset(&buf_alloc_info, 0, sizeof(iface_buf_alloc_t));

  rc = iface_util_request_stream_mapping_info(iface,&buf_alloc_info,
    session_id, stream_id);
  if (rc != 0) {
    CDBG_ERROR("%s:%d stream mapping req failed, no native buf opt\n",
                __func__, __LINE__);
    return 0;
  }

  if (buf_alloc_info.num_pix_stream > ISP_NUM_PIX_STREAM_MAX) {
    CDBG_ERROR("Num of PIxel stream returned by ISP exceeds HW limitation!\n");
    buf_alloc_info.num_pix_stream = ISP_NUM_PIX_STREAM_MAX;
  }

  // Here, we query the ADSP heap info and additional buf requirement from PP & fmt
  memset(&pp_stream_info, 0, sizeof(pp_meta_stream_info_request_t));
  pp_stream_info.num_streams = buf_alloc_info.num_pix_stream;

  //Update the cam format ...
  for (i = 0; i < buf_alloc_info.num_pix_stream
    && i < MAX_NUM_STREAMS; i++) {
    pp_stream_info.buf_info[i].stream_type =
      buf_alloc_info.stream_info[i].stream_type;

    for (j = 0; j < streams_desc->num_streams
      && j < MAX_NUM_STREAMS; j++) {
      if (buf_alloc_info.stream_info[i].stream_type == streams_desc->type[j]) {
         if (tmp_cam_fmt[buf_alloc_info.stream_info[i].axi_stream_src]
          == CAM_FORMAT_MAX) {
         tmp_cam_fmt[buf_alloc_info.stream_info[i].axi_stream_src] =
          buf_alloc_info.stream_info[i].fmt;
        } else {
           if (iface_util_calc_num_plane(tmp_cam_fmt[buf_alloc_info.
            stream_info[i].axi_stream_src]) < iface_util_calc_num_plane(buf_alloc_info.
            stream_info[i].fmt)) {
             tmp_cam_fmt[buf_alloc_info.stream_info[i].axi_stream_src] =
              buf_alloc_info.stream_info[i].fmt;
          }
        }
        /* Convert yv12 to nv12 format. CPP will convert NV12 to YV12 before
         * sending it to HAL/app */
        buf_alloc_info.stream_info[i].fmt =
          (streams_desc->format[j] == CAM_FORMAT_YUV_420_YV12) ?
          CAM_FORMAT_YUV_420_NV12 : tmp_cam_fmt[buf_alloc_info.stream_info[i].axi_stream_src];
        CDBG("stream type %d, stream fmt %d hw fmt %d\n",
             streams_desc->type[j], streams_desc->format[j],
             buf_alloc_info.stream_info[i].fmt);
        break;
      }
    }
  }

  rc = iface_util_request_pp_meta_stream_info(iface, &pp_stream_info,
    session_id, stream_id);
  if (rc != 0) {
    CDBG_ERROR("%s:%d stream mapping req failed, no native buf opt\n",
                __func__, __LINE__);
    return 0;
  }

  session->hfr_param.hfr_mode = streams_desc->hfr_mode;

  for (i = 0; i < buf_alloc_info.num_pix_stream
    && i < ISP_NUM_PIX_STREAM_MAX; i++) {
    buf_alloc_info.stream_info[i].need_adsp_heap =
      pp_stream_info.buf_info[i].need_adsp_heap;
    buf_alloc_info.stream_info[i].num_additional_buffers =
      pp_stream_info.buf_info[i].num_additional_buffers;

    tmp_axi_src = buf_alloc_info.stream_info[i].axi_stream_src;
    if (buf_alloc_info.stream_info[i].use_native_buffer &&
      tmp_axi_src < VFE_AXI_SRC_MAX) {
      rc |= iface_util_calculate_frame_length_for_native_buf(
        &buf_alloc_info.hw_stream_super_dim[tmp_axi_src],
        buf_alloc_info.stream_info[i].fmt,
        &buf_alloc_info.stream_info[i].buf_planes,
        streams_desc->buf_alignment,
        streams_desc->min_stride,
        streams_desc->min_scanline);
        b_native_alloc = TRUE;
    }
  }

  if (rc == 0 && b_native_alloc == TRUE)
    iface_resource_alloc_to_thread(
      iface, stream_id, session_id, &buf_alloc_info);
  else
    CDBG_HIGH("%s:%d: Skip pre allocate buffers\n", __func__, __LINE__);

  return 0;
}
/** iface_util_set_hfr
 *    @stream_id:
 *    @hfr_mode:
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int iface_util_set_hfr(iface_t *iface, iface_session_t *session, uint32_t stream_id,
  int32_t *hfr_mode)
{
  int i = 0;
  enum msm_vfe_frame_skip_pattern skip_pattern;
  iface_hw_stream_t *hw_stream;

  if (!session) {
    CDBG_ERROR("%s: can not find session\n", __func__);
    return -1;
  }

  session->hfr_param.hfr_mode = *hfr_mode;
  skip_pattern =  iface_util_get_hfr_skip_pattern(session);

  hw_stream = iface_util_find_hw_stream_by_mct_id(iface, session, stream_id);
  if (hw_stream == NULL) {
     CDBG("%s: can't find hw stream, only save hfr mode\n", __func__);
     return 0;
  }
  hw_stream->frame_skip_pattern = skip_pattern;

  return 0;
}

/** iface_util_handle_bestshot
 *    @session_id:
 *    @bestshot_mode:
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int iface_util_handle_bestshot(iface_t *iface, uint32_t session_id,
  cam_scene_mode_type *bestshot_mode)
{
  int rc = 0;
  iface_session_t *session = iface_util_get_session_by_id(iface, session_id);

  if (!session) {
    CDBG_ERROR("%s: can not find session, session_id = %d\n", __func__, session_id);
    return -1;
  }

  if (*bestshot_mode == CAM_SCENE_MODE_HDR) {
    CDBG("%s: HDR Enabled", __func__);
    session->bracketing_state = MCT_BRACKETING_STATE_INIT;
    rc = iface_util_set_frame_skip_all_stream(iface, session);
  } else {
    CDBG("%s: HDR Disabled", __func__);
    session->bracketing_state = MCT_BRACKETING_STATE_DEINIT;
  }

  return rc;
}


int iface_util_handle_adv_capturemode(iface_t *iface, uint32_t session_id,
  uint32_t *enable)
{
  int rc = 0;
  iface_session_t *session = iface_util_get_session_by_id(iface, session_id);

  if (!session) {
    CDBG_ERROR("%s: can not find session, session_id = %d\n", __func__, session_id);
    return -1;
  }

  if (*enable) {
    CDBG("%s: adv capture mode enable", __func__);
    session->bracketing_state = MCT_BRACKETING_STATE_INIT;
    rc = iface_util_set_frame_skip_all_stream(iface, session);
  } else {
    CDBG("%s: adv capture mode disable", __func__);
    session->bracketing_state = MCT_BRACKETING_STATE_DEINIT;
  }

  return rc;
}

/** iface_util_set_hal_frame_skip
 *    @iface: iface handle
 *    @session id: session id
 *    @skip_pattern: skip pattern
 *
 * Return: 0 - success and negative value - failure
 **/
int iface_util_set_hal_frame_skip(iface_t *iface,
  iface_session_t *session, int32_t *skip_pattern)
{
  int rc = 0;

if (!iface || !skip_pattern || !session) {
    CDBG_ERROR("%s: iface %p, skip_pattern %p, session %p rc = -1\n",
      __func__, iface, skip_pattern, session);
    return -1;
  }

  CDBG_HIGH("%s: session id = %d, skip pattern = %d\n",
    __func__, session->session_id, *skip_pattern);

  /*session need to keep track HAL request skip pattern*/
  session->hal_skip_pattern = *skip_pattern;

  rc = iface_util_set_frame_skip_all_stream(iface, session);

  return rc;
}

/** iface_util_set_bracketing_frame_skip
 *    @iface: iface handle
 *    @session id: session id
 *    @skip_pattern: skip pattern
 *
 * Return: 0 - success and negative value - failure
 **/
int iface_util_set_bracketing_frame_skip(iface_t *iface,
  iface_session_t *session, int32_t *skip_pattern)
{
  int rc = 0;

if (!iface || !skip_pattern || !session) {
    CDBG_ERROR("%s: iface %p, skip_pattern %p, session %p rc = -1\n",
      __func__, iface, skip_pattern, session);
    return -1;
  }

  CDBG_HIGH("%s: session id = %d, skip pattern = %d\n",
    __func__, session->session_id, *skip_pattern);

  /*session need to keep track HAL request skip pattern*/
  session->bracketing_frame_skip_pattern = *skip_pattern;

  rc = iface_util_set_frame_skip_all_stream(iface, session);

  return rc;
}

/** iface_util_set_frame_skip_all_stream
 *    @iface: iface handle
 *    @session id: session id
 *    @stream_id: stream id
 *    @skip_pattern: skip pattern
 *
 * Return: 0 - success and negative value - failure
 **/
int iface_util_set_frame_skip_all_stream(iface_t *iface,
  iface_session_t *session)
{
  int rc = 0;
  uint32_t i = 0;
  uint32_t hw_stream_id = 0;
  uint32_t hw_id = 0;
  int32_t skip_pattern;
  iface_param_frame_skip_pattern_t frame_skip;

  if (!iface || !session) {
    CDBG_ERROR("%s: iface %p, session %p, rc = -1\n",
      __func__, iface, session);
    return -1;
  }

  /* Don't skip frames by default for any stream in HDR mode. When snapshot
     is taken program the skip as specified by sensor */
  switch (session->bracketing_state) {
  case MCT_BRACKETING_STATE_INIT:
  case MCT_BRACKETING_STATE_OFF:
    skip_pattern = (int32_t) NO_SKIP;
    break;
  case MCT_BRACKETING_STATE_ON:
    /* Set Sensor Bracketing frame skip */
    skip_pattern = (int32_t) session->bracketing_frame_skip_pattern;
    break;
  case MCT_BRACKETING_STATE_DEINIT:
  default:
    /* Restore HAL frame skip */
    skip_pattern = (int32_t) session->hal_skip_pattern;
    break;
  }

  CDBG_HIGH("%s: bracketing_state=%d skip pattern = %d\n", __func__,
    session->bracketing_state, skip_pattern);

  memset(&frame_skip, 0, sizeof(iface_param_frame_skip_pattern_t));
  frame_skip.pattern = (enum msm_vfe_frame_skip_pattern)(skip_pattern);
  frame_skip.session_id = session->session_id;
  frame_skip.hw_stream_id = 0;
  frame_skip.use_sw_skip = FALSE;
  frame_skip.bracketing_state = session->bracketing_state;


  for (hw_id = 0; hw_id < VFE_MAX; hw_id++) {
    if (session->session_resource.isp_id_mask & (1 << hw_id)) {
      if (iface->isp_axi_data.axi_data[hw_id].axi_hw_ops) {
        rc = iface->isp_axi_data.axi_data[hw_id].axi_hw_ops->set_params(
          iface->isp_axi_data.axi_data[hw_id].axi_hw_ops->ctrl,
          IFACE_AXI_SET_PARAM_FRAME_SKIP_ALL_STREAM, (void *)&frame_skip, sizeof(frame_skip));
        if (rc !=0) {
          CDBG_ERROR("Failed!! FRAME_SKIP_ALL_STREAM\n");
        }
      }
    }
  }

  return 0;
}

/** iface_util_request_frame
 *  @session: session data
 *  @iface: iface handle
 *  @frame_request: description of requested frame
 *
 *  Request frame from controlled output
 *
 * Return: 0 - success and negative value - failure
 **/
int iface_util_request_frame(iface_session_t *session,
  iface_t *iface, iface_param_frame_request_t *frame_request)
{
  int rc = 0;
  iface_axi_hw_t  *axi_hw = NULL;
  uint32_t         kernel_sofid = 0;

  if (session->session_resource.isp_id_mask & (1 << VFE0)) {
    axi_hw =
      (iface_axi_hw_t *)iface->isp_axi_data.axi_data[VFE0].axi_hw_ops->ctrl;
  }
  if (session->session_resource.isp_id_mask & (1 << VFE1)) {
    axi_hw =
      (iface_axi_hw_t *)iface->isp_axi_data.axi_data[VFE1].axi_hw_ops->ctrl;
  }

  if ((axi_hw != NULL) && axi_hw->kstate_ptr.isp_kstate_ptr) {
    kernel_sofid = *(unsigned int *)(axi_hw->kstate_ptr.isp_kstate_ptr);
  }

  /*If request frame id is behind or equal to kernel frameid
   *Its an error */
  if (frame_request->frame_id <= kernel_sofid) {
    CDBG_ERROR("%s:%d: request frame for session %d, mct stream %d,\
      hw stream 0x%x, buf_idx: %d frame id %d ker_Frmid: %d\n",
      __func__, __LINE__, frame_request->session_id,
      frame_request->user_stream_id, frame_request->hw_stream_id,
      frame_request->frame_id, kernel_sofid);
  }

  if (session->session_resource.isp_id_mask & (1 << VFE0)) {
    rc = iface_axi_stream_request_frame(
            iface->isp_axi_data.axi_data[VFE0].axi_hw_ops->ctrl,
           (iface_param_frame_request_t *)frame_request,
            sizeof(*frame_request));
    if (rc < 0) {
      CDBG_ERROR("%s: VFE0 iface request frame failed! rc = %d\n",
        __func__, rc);
      return rc;
    }
  }

  if (session->session_resource.isp_id_mask & (1 << VFE1)) {
    rc = iface_axi_stream_request_frame(
            iface->isp_axi_data.axi_data[VFE1].axi_hw_ops->ctrl,
           (iface_param_frame_request_t *)frame_request,
            sizeof(*frame_request));
    if (rc < 0) {
      CDBG_ERROR("%s: VFE1 iface request frame failed! rc = %d\n", __func__,
        rc);
      return rc;
    }
  }

  return rc;
}

/** iface_util_add_bufq_to_stream
 *  @session: session data
 *  @iface: iface handle
 *  @frame_request: description of requested frame
 *
 *  Add shared buffer queue for controlled output
 *
 * Return: 0 - success and negative value - failure
 **/
int iface_util_add_bufq_to_stream(iface_session_t *session, iface_t *iface,
  uint32_t hw_stream_id, uint32_t user_stream_id)
{
  iface_param_add_queue_t add_bufq;
  int rc = 0;

  memset(&add_bufq, 0, sizeof(add_bufq));
  add_bufq.session_id = session->session_id;
  add_bufq.hw_stream_id = hw_stream_id;
  add_bufq.user_stream_id = user_stream_id;

  if (session->session_resource.isp_id_mask & (1 << VFE0)) {
    rc = iface->isp_axi_data.axi_data[VFE0].axi_hw_ops->action(
      iface->isp_axi_data.axi_data[VFE0].axi_hw_ops->ctrl,
      IFACE_AXI_ACTION_CODE_ADD_BUFQ, (void *)&add_bufq, sizeof(add_bufq));
    if (rc < 0) {
      CDBG_ERROR("%s: VFE0 IFACE_AXI_ACTION_CODE_ADD_BUFQ failed! rc = %d\n",
        __func__, rc);
      return rc;
    }
  }

  if (session->session_resource.isp_id_mask & (1 << VFE1)) {
    rc = iface->isp_axi_data.axi_data[VFE1].axi_hw_ops->action(
      iface->isp_axi_data.axi_data[VFE1].axi_hw_ops->ctrl,
      IFACE_AXI_ACTION_CODE_ADD_BUFQ, (void *)&add_bufq, sizeof(add_bufq));
    if (rc < 0) {
      CDBG_ERROR("%s: VFE1 IFACE_AXI_ACTION_CODE_ADD_BUFQ failed! rc = %d\n",
        __func__, rc);
      return rc;
    }
  }

  return rc;
}

/** iface_util_remove_bufq_from_stream
 *  @session: session data
 *  @iface: iface handle
 *  @frame_request: description of requested frame
 *
 *  Remove shared buffer queue for controlled output
 *
 * Return: 0 - success and negative value - failure
 **/
int iface_util_remove_bufq_from_stream(iface_session_t *session, iface_t *iface,
  uint32_t hw_stream_id, uint32_t user_stream_id)
{
  iface_param_add_queue_t add_bufq;
  int rc = 0;

  memset(&add_bufq, 0, sizeof(add_bufq));
  add_bufq.session_id = session->session_id;
  add_bufq.hw_stream_id = hw_stream_id;
  add_bufq.user_stream_id = user_stream_id;

  if (session->session_resource.isp_id_mask & (1 << VFE0)) {
    rc = iface->isp_axi_data.axi_data[VFE0].axi_hw_ops->action(
      iface->isp_axi_data.axi_data[VFE0].axi_hw_ops->ctrl,
      IFACE_AXI_ACTION_CODE_REMOVE_BUFQ, (void *)&add_bufq, sizeof(add_bufq));
    if (rc < 0) {
      CDBG_ERROR("%s: VFE0 IFACE_AXI_ACTION_CODE_REMOVE_BUFQ failed! rc = %d\n",
        __func__, rc);
      return rc;
    }
  }

  if (session->session_resource.isp_id_mask & (1 << VFE1)) {
    rc = iface->isp_axi_data.axi_data[VFE1].axi_hw_ops->action(
      iface->isp_axi_data.axi_data[VFE1].axi_hw_ops->ctrl,
      IFACE_AXI_ACTION_CODE_REMOVE_BUFQ, (void *)&add_bufq, sizeof(add_bufq));
    if (rc < 0) {
      CDBG_ERROR("%s: VFE1 IFACE_AXI_ACTION_CODE_REMOVE_BUFQ failed! rc = %d\n",
        __func__, rc);
      return rc;
    }
  }

  return rc;
}

/** iface_util_check_valid_frame
 * @stream_ids: requested streams
 * @mapped_mct_stream_id: streams mapped to HW stream
 *
 * Check if there is frame request for this HW stream and return user stream id
 *
 * Return: 0 - success and negative value - failure
 **/
static uint32_t iface_util_check_valid_frame(iface_hw_stream_t *hw_stream,
  cam_stream_ID_t *stream_ids, uint32_t *user_stream_id, uint32_t *buf_index)
{
  uint32_t i, j, stream_id = 0;
  uint32_t *mapped_mct_stream_id;
  uint8_t streams_num;

  *user_stream_id = 0;

  /* Check of this hw stream is controllable */
  if (!hw_stream->controllable_output) {
    return 0;
  }

  mapped_mct_stream_id = hw_stream->mapped_mct_stream_id;
  streams_num = 0;

  /* Check if we have request for this HW stream */
  for (i = 0; i < stream_ids->num_streams && i < MAX_NUM_STREAMS; i++) {
    for (j = 0; j < IFACE_MAX_STREAMS; j++) {
      if (stream_ids->stream_request[i].streamID == mapped_mct_stream_id[j]) {
        (streams_num)++;
        stream_id = stream_ids->stream_request[i].streamID;
        *buf_index = stream_ids->stream_request[i].buf_index;
      }
    }
  }

  /* we cannot supply more than one streams at same time on
   * controllable output */
  if (streams_num > 1) {
    CDBG_ERROR("%s: failed: streams_num %d\n", __func__, streams_num);
    return -1;
  }

  *user_stream_id = stream_id;

  return 0;
}

/** iface_util_request_frame_by_stream_ids
 * @iface: iface handle
 * @session id: session id
 * @parm_data: requested stream IDs
 *
 * Request output frame for controlled outputs
 *
 * Return: 0 - success and negative value - failure
 **/
int iface_util_request_frame_by_stream_ids(iface_t *iface,
  uint32_t session_id, void *parm_data, uint32_t frame_id)
{
  iface_param_frame_request_t  frame_request;
  iface_hw_stream_t           *hw_stream;
  iface_session_t             *session;
  cam_stream_ID_t             *stream_ids;
  uint32_t                     i, user_stream_id;
  uint32_t                     buf_index;
  int                          rc = 0;

  if (!iface || !parm_data) {
    CDBG_ERROR("%s: iface %p, parm_data %p\n", __func__, iface, parm_data);
    return -1;
  }

  stream_ids = (cam_stream_ID_t *)parm_data;
  session = iface_util_get_session_by_id(iface, session_id);
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n", __func__, session_id);
    return -1;
  }

#ifdef PRINT_REQUEST_STREAMIDS
  ALOGE("%s:%d ============ start =============", __func__, __LINE__);
  for (i = 0; i < stream_ids->num_streams  && i < MAX_NUM_STREAMS; i++) {
    ALOGE("%s:%d i %d stream id %d\n", __func__, __LINE__, i,
      stream_ids->stream_request[i].streamID);
  }
  ALOGE("%s:%d ============ end =============", __func__, __LINE__);
#endif

  for (i = 0; i < IFACE_MAX_STREAMS; i++) {
    hw_stream = &session->hw_streams[i];
    if (hw_stream->state != IFACE_HW_STREAM_STATE_ACTIVE)
      continue;

    rc = iface_util_check_valid_frame(hw_stream, stream_ids, &user_stream_id,
                                      &buf_index);
    if (rc) {
      CDBG_ERROR("%s: failed: iface_util_check_valid_frame\n", __func__);
      return rc;
    }
    if (!user_stream_id) {
      continue;
    }

    frame_request.session_id = session_id;
    frame_request.hw_stream_id = hw_stream->hw_stream_id;
    frame_request.user_stream_id = user_stream_id;
    frame_request.frame_id = frame_id + IFACE_APPLY_DELAY;
    frame_request.buf_index = buf_index;
    if (session->bayer_processing) {
        /*copy the frame_request, to be used at fetch engine SOF*/
        session->frame_request = frame_request;
    }
    else {
       rc = iface_util_request_frame(session, iface, &frame_request);
       if (rc) {
         CDBG_ERROR("%s: failed: iface_util_request_frame\n", __func__);
         return rc;
       }
    }
  }

  return rc;
}

/** iface_util_set_stats_frame_skip
 *    @iface: iface handle
 *    @session id: session id
 *    @stream_id: stream id
 *    @skip_pattern: skip pattern
 *
 * Return: 0 - success and negative value - failure
 **/
int iface_util_set_stats_frame_skip(iface_t *iface,
  iface_session_t *session, int32_t *skip_pattern)
{
  int rc = 0;
  uint32_t hw_id = 0;
  uint32_t hw_stream_id = 0;
  iface_param_frame_skip_pattern_t stats_skip;
  enum msm_vfe_frame_skip_pattern stats_skip_pattern = NO_SKIP;

  if (!iface || !skip_pattern || !session) {
    CDBG_ERROR("%s: iface %p skip_pattern %p session %p rc = -1\n",
      __func__, iface, skip_pattern, session);
    return -1;
  }

  IFACE_HIGH("%s: dynamica stats skip feature is enabled by 3A!\n", __func__);
  session->dynamic_stats_skip_feature_enb = TRUE;

  stats_skip_pattern =
    (enum msm_vfe_frame_skip_pattern)(*skip_pattern);
  if (stats_skip_pattern >= MAX_SKIP) {
    CDBG_ERROR("%s: request stats skip %d over supported skip\n",
      __func__, stats_skip_pattern);
    return -1;
  }

  session->stats_skip_pattern = stats_skip_pattern;
  CDBG_HIGH("%s:sess id %d, skip pattern %d(0-NO Skip/1-EVERY2/2-EVERY3...)\n",
    __func__, session->session_id, session->stats_skip_pattern);

  memset(&stats_skip, 0, sizeof(iface_param_frame_skip_pattern_t));
  stats_skip.pattern = stats_skip_pattern;
  stats_skip.session_id = session->session_id;
  /*skip all stats instead of specific stats, can be extended*/
  stats_skip.hw_stream_id = 0;

  for (hw_id = 0; hw_id < VFE_MAX; hw_id++) {
    if (session->session_resource.isp_id_mask & (1 << hw_id)) {
      if (iface->isp_axi_data.axi_data[hw_id].axi_hw_ops) {
        rc = iface->isp_axi_data.axi_data[hw_id].axi_hw_ops->set_params(
          iface->isp_axi_data.axi_data[hw_id].axi_hw_ops->ctrl,
          IFACE_AXI_SET_PARAM_STATS_SKIP, (void *)&stats_skip, sizeof(stats_skip));
        if (rc != 0) {
          CDBG_ERROR("Failed!! IFACE_AXI_SET_PARAM_STATS_SKIP\n");
        }
      }
    }
  }

  return 0;
}

/** iface_util_set_hal_version
 * @iface: iface handle
 * @session_id: session id
 * @hal_version: HAL version
 *
 *  Save the HAL version
 *
 * Return: 0 - success and negative value - failure
 **/
int iface_util_set_hal_version(iface_t *iface, uint32_t session_id,
  cam_hal_version_t *hal_version)
{
  iface_session_t *session;

  if (!iface || !hal_version) {
    CDBG_ERROR("%s: failed: %p %p", __func__, iface, hal_version);
    return -1;
  }

  session = iface_util_get_session_by_id(iface, session_id);
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n", __func__, session_id);
    return -1;
  }

  session->hal_version = *hal_version;

  return 0;
}

/** iface_util_set_frame_skip
 *    @stream_id:
 *    @skip_pattern:
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int iface_util_set_frame_skip(iface_t *iface, uint32_t session_id,
  uint32_t hw_stream_id, enum msm_vfe_frame_skip_pattern *skip_pattern)
{
  int rc = 0;
  iface_param_frame_skip_pattern_t frame_skip;

  if (!iface || !skip_pattern) {
    CDBG_ERROR("%s: iface %p, skip_pattern %p, rc = -1\n",
      __func__, iface, skip_pattern);
    return -1;
  }

  CDBG_HIGH("%s: hw stream id = %x, skip pattern = %d\n",
    __func__, hw_stream_id, *skip_pattern);

  iface_session_t *session =
    iface_util_get_session_by_id(iface,session_id);
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n", __func__, session_id);
    return -1;
  }

  frame_skip.pattern = *skip_pattern;
  frame_skip.session_id = session_id;
  frame_skip.hw_stream_id = hw_stream_id;
  frame_skip.use_sw_skip = FALSE;

  if (session->session_resource.isp_id_mask & (1 << VFE0)) {
    if (iface->isp_axi_data.axi_data[VFE0].axi_hw_ops) {
      rc = iface->isp_axi_data.axi_data[VFE0].axi_hw_ops->set_params(
        iface->isp_axi_data.axi_data[VFE0].axi_hw_ops->ctrl,
        IFACE_AXI_SET_PARAM_FRAME_SKIP, (void *)&frame_skip, sizeof(frame_skip));
    }
  }

  if (session->session_resource.isp_id_mask & (1 << VFE1)) {
    if (iface->isp_axi_data.axi_data[VFE1].axi_hw_ops) {
      rc = iface->isp_axi_data.axi_data[VFE1].axi_hw_ops->set_params(
        iface->isp_axi_data.axi_data[VFE1].axi_hw_ops->ctrl,
        IFACE_AXI_SET_PARAM_FRAME_SKIP, (void *)&frame_skip, sizeof(frame_skip));
    }
  }

  return rc;
}

/** iface_util_get_hfr_skip_pattern
 *
 * DESCRIPTION:
 *
 **/
enum msm_vfe_frame_skip_pattern iface_util_get_hfr_skip_pattern(
  iface_session_t *session)
{
  if (session->fast_aec_mode)
    return SKIP_ALL;

  switch (session->hfr_param.hfr_mode) {
  case CAM_HFR_MODE_60FPS:
    return EVERY_2FRAME;

  case CAM_HFR_MODE_90FPS:
    return EVERY_3FRAME;

  case CAM_HFR_MODE_120FPS:
    return EVERY_4FRAME;

  case CAM_HFR_MODE_150FPS:
    return EVERY_5FRAME;

  case CAM_HFR_MODE_180FPS:
    return EVERY_6FRAME;

  case CAM_HFR_MODE_210FPS:
    return EVERY_7FRAME;

  case CAM_HFR_MODE_240FPS:
    return EVERY_8FRAME;

  case CAM_HFR_MODE_480FPS:
    return EVERY_16FRAME;

  default:
    break;
  }

  CDBG("%s: X, no skip\n", __func__);
  return NO_SKIP;
}

/** iface_util_get_frame_skip_period
 *
 * DESCRIPTION:
 *
 **/
uint32_t iface_util_get_frame_skip_period(
  enum msm_vfe_frame_skip_pattern hfr_skip_pattern)
{
  uint32_t skip_period = 0;

  switch (hfr_skip_pattern) {
  case NO_SKIP:
  case EVERY_2FRAME:
  case EVERY_3FRAME:
  case EVERY_4FRAME:
  case EVERY_5FRAME:
  case EVERY_6FRAME:
  case EVERY_7FRAME:
  case EVERY_8FRAME:
    skip_period = hfr_skip_pattern + 1;
    break;
  case EVERY_16FRAME:
    skip_period  = 16;
    break;
  case EVERY_32FRAME:
    skip_period = 32;
    break;
  default:
     break;
  }

   return skip_period;
}

/** iface_offline_start_fetch_engine:
 *
 *    @iface: iface
 *    @session_id : session id
 *    @hw_stream_id: hw stream id
 *    @buf_idx: buf index
 *  This function configures iface hw streams and save in
 *  session
 *
 *  Return: 0 for success and negative error on failure
 **/
int iface_offline_start_fetch_engine(
  iface_t                  *iface,
  iface_session_t          *session,
  uint32_t                  hw_stream_id,
  uint32_t                  buf_stream_id,
  iface_offline_isp_info_t *offline_info)
{
  int                    rc           = 0;
  uint32_t               isp_id       = 0;
  uint32_t               buf_idx      = 0;
  uint32_t               fd           = 0;
  uint8_t                offline_mode = 0;
  iface_hw_stream_t     *hw_stream    = NULL;
  triger_fetch_stream_t  fetch_params;

  if (!session || !offline_info) {
    CDBG_ERROR("%s: cannot find session OR offline_info %p is NULL \n",
        __func__, offline_info);
    return -1;
  }
  hw_stream = iface_util_find_hw_stream_in_session(session,
    hw_stream_id);
  if (!hw_stream) {
    CDBG_ERROR("%s: can not find HW stream id %d\n",
      __func__, hw_stream_id);
    return -1;
  }

  if (hw_stream->state != IFACE_HW_STREAM_STATE_ACTIVE) {
    CDBG_ERROR("%s: Offline HW stream invalid state %d\n",
      __func__, hw_stream->state);
    return -1;
  }
  memset(&fetch_params, 0, sizeof(triger_fetch_stream_t));
  fetch_params.session_id = hw_stream->session_id;
  fetch_params.hw_stream_id= buf_stream_id;
  fetch_params.buf_idx= offline_info->stream_param_buf->reprocess.buf_index;
  fetch_params.offline_mode = offline_info->offline_mode;
  fetch_params.fd = offline_info->fd;
  fetch_params.frame_id = offline_info->offline_frame_id;
#ifdef VIDIOC_MSM_ISP_MAP_BUF_START_MULTI_PASS_FE
  fetch_params.offline_pass = offline_info->offline_pass;
#endif
  fetch_params.output_buf_idx = offline_info->output_buf_idx;
  fetch_params.input_buf_offset = offline_info->input_buf_offset;
  fetch_params.output_stream_id = offline_info->output_stream_id;

  CDBG("%s reprocess offline mode %d\n", __func__, fetch_params.offline_mode);
  for (isp_id = 0; isp_id < VFE_MAX; isp_id++) {
    if (hw_stream->interface_mask & (1 << (16 * isp_id +
      IFACE_INTF_PIX))) {
      rc = iface->isp_axi_data.axi_data[isp_id].axi_hw_ops->action(
        iface->isp_axi_data.axi_data[isp_id].axi_hw_ops->ctrl,
        IFACE_AXI_ACTION_CODE_FETCH_START, (void *)&fetch_params,
        sizeof(triger_fetch_stream_t));
      if (rc < 0) {
        CDBG_ERROR("%s: Fetch engine busy = %d errno %d QBUF %d\n",
          __func__, rc, errno, buf_idx);
        hw_stream = iface_util_find_hw_stream_in_session(session,
          buf_stream_id);
        if (!hw_stream) {
          CDBG_ERROR("%s: can not find HW stream id %d\n",
            __func__, buf_stream_id);
          return -1;
        }
         iface_queue_buf(&iface->buf_mgr,
           hw_stream->buffers_info[VFE_BUF_QUEUE_DEFAULT].bufq_handle,
           buf_idx, TRUE, 0, 0);
      }
    }
  }
  return rc;
}

/** iface_offline_stream_unconfig:
 *
 *    @stream_id: offline stream id
 *    @dim : offline stream dim
 *    @fmt: input buffer format
 *  This function configures iface hw streams and save in
 *  session
 *
 *  Return: 0 for success and negative error on failure
 **/
int iface_offline_stream_unconfig(iface_t *iface,
  uint32_t hw_stream_id, uint32_t session_id)
{
  int32_t                         i, rc = 0;
  iface_stream_t                 *stream = NULL;
  iface_session_t                *iface_session = NULL;
  struct msm_vfe_smmu_attach_cmd  cmd;
  iface_hw_stream_t              *offline_hw_stream = NULL;
  start_stop_stream_t             start_param;
  uint32_t                        isp_id, num_hw_streams = 0;
  uint32_t                        hw_stream_ids[ISP_NUM_PIX_STREAM_MAX];

  iface_session = iface_util_get_session_by_id(iface, session_id);
  if (iface_session == NULL) {
    CDBG_ERROR("%s: can not find session, id = %d\n", __func__, session_id);
    return -1;
  }

   /* Reset offline isp frame id */
  iface_session->offline_info.offline_frame_id = 0;

  offline_hw_stream = iface_util_find_hw_stream_in_session(iface_session,
    hw_stream_id);
  if (!offline_hw_stream){
    CDBG_ERROR("%s: Offline stream not found %d \n", __func__, hw_stream_id);
    return -1;
  };
  CDBG_ERROR("%s:#%d offline_hw_stream %x\n", __func__, __LINE__,
    offline_hw_stream->hw_stream_id);

  /*  AXI streamoff,
     */
  for (isp_id = 0; isp_id < VFE_MAX; isp_id++) {
    memset(&start_param, 0, sizeof(start_stop_stream_t));
    for (i = 0; i< IFACE_MAX_STREAMS; i++) {
      /* Check for offline stream only */
      if (iface_session->hw_streams[i].stream_info.cam_stream_type !=
        CAM_STREAM_TYPE_OFFLINE_PROC) {
        continue;
      }
      offline_hw_stream = &iface_session->hw_streams[i];
      if (offline_hw_stream->interface_mask & (1 << (16 * isp_id +
        IFACE_INTF_PIX))) {
        offline_hw_stream->state = IFACE_HW_STREAM_STATE_STOPPING;
        start_param.session_id = iface_session->session_id;
        start_param.stream_ids[start_param.num_streams++] =
          offline_hw_stream->hw_stream_id;
      }
    }
    if (start_param.num_streams > 0) {
      CDBG("%s:#%d stop _stream %x\n", __func__, __LINE__,
        offline_hw_stream->hw_stream_id);
      rc = iface->isp_axi_data.axi_data[isp_id].axi_hw_ops->action(
        iface->isp_axi_data.axi_data[isp_id].axi_hw_ops->ctrl,
        IFACE_AXI_ACTION_CODE_STREAM_STOP,
        (void *)&start_param, sizeof(start_stop_stream_t));
      if (rc < 0) {
        CDBG_ERROR("%s: VFE%d AXI_STREAM_STOP failed! rc = %d\n",
          __func__, isp_id, rc);
        return rc;
      }
      rc = iface->isp_axi_data.axi_data[isp_id].axi_hw_ops->action(
        iface->isp_axi_data.axi_data[isp_id].axi_hw_ops->ctrl,
        IFACE_AXI_ACTION_CODE_STREAM_STOP_ACK,
        (void *)&start_param, sizeof(start_stop_stream_t));
      if (rc < 0) {
        CDBG_ERROR("%s: VFE%d AXI_STREAM_STOP failed! rc = %d\n",
          __func__, isp_id, rc);
        return rc;
      }
    }
  }

  CDBG("%s:#%d release_image_buf \n", __func__, __LINE__);
  /*release buffer: hal buffer or native buffer*/
  rc = iface_util_release_image_buf(iface, iface_session, offline_hw_stream);
  if (rc < 0) {
    CDBG_ERROR("%s: iface_util_request_image_buf error = %d \n", __func__, rc);
    return rc;
  }
  for (i = 0; i< IFACE_MAX_STREAMS; i++) {
    if (iface_session->hw_streams[i].stream_info.cam_stream_type !=
      CAM_STREAM_TYPE_OFFLINE_PROC) {
      continue;
    }
     offline_hw_stream = &iface_session->hw_streams[i];
     offline_hw_stream->state = IFACE_HW_STREAM_STATE_RESERVED;
     hw_stream_ids[num_hw_streams++] = offline_hw_stream->hw_stream_id;

    /* Configure ISP offline pipeline */
    iface_util_start_isp_pipeline_cfg(iface, &iface_session->offline_info,
      ISP_FETCH_PIPELINE_STOP, iface_session,
      offline_hw_stream->hw_stream_id);

    for (isp_id = 0; isp_id < VFE_MAX; isp_id++) {
      if (offline_hw_stream->interface_mask & (1 << (16 * isp_id +
        IFACE_INTF_PIX))) {
        CDBG("%s:#%d stream_uncfg & stats_uncfg \n", __func__, __LINE__);
        if (iface->isp_axi_data.axi_data[isp_id].axi_hw_ops != NULL) {
          rc = iface->isp_axi_data.axi_data[isp_id].axi_hw_ops->set_params(
            iface->isp_axi_data.axi_data[isp_id].axi_hw_ops->ctrl,
            IFACE_AXI_SET_STREAM_UNCFG, offline_hw_stream,
            sizeof(iface_hw_stream_t));
          if (rc != 0) {
            CDBG_ERROR("failed IFACE_AXI_SET_STREAM_UNCFG !!!\n");
          }
          rc = iface->isp_axi_data.axi_data[isp_id].axi_hw_ops->set_params(
            iface->isp_axi_data.axi_data[isp_id].axi_hw_ops->ctrl,
            IFACE_AXI_SET_STATS_STREAM_UNCFG, NULL, 0);
          if (rc < 0) {
            CDBG_ERROR("%s: VFE%d AXI STATS_STREAM_UNCFG failed! rc = %d\n",
              __func__, isp_id, rc);
            return rc;
          }
        }
        if (iface_session->num_hw_stream)
          iface_session->num_hw_stream--;
        memset(offline_hw_stream, 0, sizeof(iface_hw_stream_t));
        CDBG("%s:#%d Destroy hw %d\n", __func__, __LINE__, isp_id);
        /* destroy axi/hw stream*/
        if (iface_session->create_axi_count[isp_id] > 0 &&
            --iface_session->create_axi_count[isp_id] == 0) {
          iface_destroy_hw(iface, isp_id);
        }
        iface_session->session_resource.offline_isp_id_mask &=
           ~(1 << isp_id);
        if (iface_session->offline_stats_streams[i].stats_stream_id) {
          memset(&iface_session->offline_stats_streams[i], 0 ,
            sizeof(iface_stats_stream_t));
        }
        /* Reset session_resource mask if both offline & online VFE are done */
        if (iface_session->session_resource.camif_cnt == 0 &&
            iface_session->session_resource.rdi_cnt == 0 &&
            iface_session->session_resource.offline_isp_id_mask == 0) {
          memset(&iface_session->session_resource, 0, sizeof(iface_resource_t));
        }
      }
    }
  }

  CDBG("%s:#%d rc = %d X\n", __func__, __LINE__, rc);
  return rc;
}

/** iface_util_send_offline_input_cfg
 *
 *  @session_id : session_id
 *  @stream_id  : stream_id
 *  @iface      : pointer to iface
 *  @offline_info : pointer to offline info
 *
 **/
int iface_util_send_offline_input_cfg(iface_t *iface,
  uint32_t session_id, uint32_t stream_id,
  iface_offline_isp_info_t *offline_info)
{
  mct_event_t        event;
  sensor_set_dim_t   offline_input_cfg;
  iface_stream_t     *user_stream = NULL;

  if (!iface || !offline_info) {
    IFACE_ERR("<OFFLINE>Error: iface %p offline_info %p",
      iface, offline_info);
    return -1;
  }

  user_stream = iface_util_find_stream_in_sink_port_list(iface,
    session_id, stream_id);
  if (!user_stream) {
    IFACE_ERR("<OFFLINE>Error: stream %p\n", user_stream);
    return -1;
  }

  memset(&event, 0, sizeof(mct_event_t));
  memset(&offline_input_cfg, 0, sizeof(sensor_set_dim_t));

  offline_input_cfg.dim_output.height =
    offline_info->input_dim.height;
  offline_input_cfg.dim_output.width =
    offline_info->input_dim.width;
  offline_input_cfg.output_format =
    offline_info->input_fmt;

  event.u.module_event.type = MCT_EVENT_MODULE_ISP_OFFLINE_INPUT_CONFIG;
  event.u.module_event.module_event_data =
      (void *)&offline_input_cfg;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = pack_identity(session_id, stream_id);
  event.direction = MCT_EVENT_DOWNSTREAM;
  mct_port_send_event_to_peer(user_stream->src_port, &event);
  return 0;
}

/** iface_offline_stream_config:
 *
 *    @stream_id: offline stream id
 *    @dim : offline stream dim
 *    @fmt: input buffer format
 *  This function configures iface hw streams and save in
 *  session
 *
 *  Return: 0 for success and negative error on failure
 **/
int iface_offline_stream_config(
  iface_t                  *iface,
  iface_port_t             *iface_sink_port,
  uint32_t                  stream_id,
  iface_session_t          *iface_session,
  iface_offline_isp_info_t *offline_info)
{
  int32_t                         i, rc = 0;
  iface_stream_t                 *stream = NULL;
  struct msm_vfe_smmu_attach_cmd  cmd;
  uint32_t                        offline_user_stream_id = 0;
  iface_hw_stream_t              *offline_hw_stream = NULL;
  iface_hw_stream_t              *input_hw_stream = NULL;
  start_stop_stream_t             start_param;
  uint32_t                        isp_id, num_hw_streams = 0;
  uint32_t                        hw_stream_ids[ISP_NUM_PIX_STREAM_MAX];
  iface_stream_t                 *user_stream = NULL;
  boolean                         offline_stream_ready = FALSE;
  iface_hw_stream_set_params_t    vfe_hw_params[VFE_MAX];
  uint32_t                        vfe_num_hw_streams[VFE_MAX];
  iface_stats_stream_set_params_t vfe_stats_stream[VFE_MAX];
  iface_stats_stream_t           *stats_stream = NULL;

  if (iface_session == NULL) {
    CDBG_ERROR("%s: can not find session\n", __func__);
    return -1;
  }

  if (!iface_sink_port || !offline_info) {
    CDBG_ERROR("%s: can not find session, id = %d %p\n",
      __func__, iface_session->session_id, offline_info);
    return -1;
  }

  memset(vfe_num_hw_streams, 0, sizeof(vfe_num_hw_streams));
  memset(vfe_hw_params, 0, sizeof(vfe_hw_params));

  for (i = 0; i < IFACE_MAX_STREAMS; i++) {
    CDBG_HIGH("hw stream %d  id %x type %d", i,
      iface_session->hw_streams[i].hw_stream_id,
      iface_session->hw_streams[i].stream_info.cam_stream_type);
    if (iface_session->hw_streams[i].stream_info.cam_stream_type ==
      CAM_STREAM_TYPE_OFFLINE_PROC) {
      CDBG_HIGH("Offline ISP stream found %d", i);
      offline_hw_stream = &iface_session->hw_streams[i];
      offline_stream_ready = TRUE;
      break;
    }
  }
  if (i >= IFACE_MAX_STREAMS) {
    CDBG("Create offline hw stream %d", i);
    /*Send offline input config Information to ISP*/
    rc = iface_util_send_offline_input_cfg(iface,
      iface_session->session_id, stream_id, offline_info);
    if (rc !=0 ) {
      CDBG_ERROR("iface_util_send_offline_input_cfg failed !!\n");
    }
    rc = iface_util_set_hw_stream_config_pix_offline(iface, iface_sink_port,
      iface_session->session_id, stream_id);
    if (rc < 0) {
      CDBG_ERROR("%s: failed: set_hw_stream_config_pix_offline %x rc=-1\n",
        __func__, stream_id);
      return -1;
    }
  } else {
    CDBG_HIGH("%s: Offline stream Already active\n", __func__);
    offline_stream_ready = TRUE;
  }
  if (offline_stream_ready == FALSE) {
    for (i = 0; i< IFACE_MAX_STREAMS; i++) {
      if (iface_session->hw_streams[i].stream_info.cam_stream_type !=
        CAM_STREAM_TYPE_OFFLINE_PROC) {
        continue;
      }

      offline_hw_stream = &iface_session->hw_streams[i];
      offline_hw_stream->state = IFACE_HW_STREAM_STATE_HW_CFG_DONE;
      hw_stream_ids[num_hw_streams++] = offline_hw_stream->hw_stream_id;
      offline_user_stream_id = offline_hw_stream->mapped_mct_stream_id[0];
      offline_hw_stream->mct_streamon_id =
        offline_hw_stream->mapped_mct_stream_id[0];


      /* update hal buffer list from user stream*/
      user_stream = iface_util_find_stream_in_sink_port(iface_sink_port,
        iface_session->session_id, offline_user_stream_id);
      if (user_stream == NULL) {
         CDBG_ERROR("%s: cannot find mct stream id = %d, rc = -1\n",
           __func__, offline_user_stream_id);
         return -1;;
      }
      if (!offline_hw_stream->use_native_buf) {
        CDBG_HIGH("%s: updated hal buf list num_bufs = %d\n",
          __func__, user_stream->stream_info.num_bufs);
        offline_hw_stream->buffers_info[VFE_BUF_QUEUE_DEFAULT].img_buffer_list =
          user_stream->stream_info.img_buffer_list;
        offline_hw_stream->buffers_info[VFE_BUF_QUEUE_DEFAULT].num_bufs =
          user_stream->stream_info.num_bufs;
      } else {
        iface_util_send_hw_stream_output_dim_downstream_int(iface,
          iface_session, offline_hw_stream);
      }

      /* Configure ISP offline pipeline */
      iface_util_start_isp_pipeline_cfg(iface, offline_info,
        ISP_FETCH_PIPELINE_START, iface_session,
        offline_hw_stream->hw_stream_id);

      for (isp_id = 0; isp_id < VFE_MAX; isp_id++) {
        if (offline_hw_stream->interface_mask & (1 << (16 * isp_id +
          IFACE_INTF_PIX))) {
          if (iface_session->create_axi_count[isp_id] == 0) {
            /* Create axi/hw stream. Assume one offline stream only */
            rc = iface_create_axi(iface, iface_session, isp_id);
            if (rc < 0) {
              CDBG_ERROR("%s: cannot create ISP HW %d, rc = %d\n",
                         __func__, isp_id, rc);
              return rc;
            }
          }
          iface_session->create_axi_count[isp_id]++;

          input_hw_stream = iface_util_find_hw_stream_by_mct_id(iface,
            iface_session, offline_info->input_stream_id);
          if (!input_hw_stream) {
            CDBG_ERROR("%s: can not in hw stream= %d\n",
              __func__, offline_info->input_stream_id);
            return -1;
          }

          /* Configure read engine /fetch engine*/
          rc = iface_util_fe_cfg(iface_session,
            iface->isp_axi_data.axi_data[isp_id].fd,
            offline_info->input_dim, offline_info->op_pixel_clk,
            offline_info->input_fmt, ISP_STRIPE_LEFT);

          if (rc != 0) {
            CDBG_ERROR("iface_util_fe_cfg failed !!!\n");
          }

          /* Configure write masters*/
          if (offline_hw_stream->isp_split_output_info.is_split) {
              offline_hw_stream->isp_split_output_info.stripe_id =
                ISP_STRIPE_LEFT;
              offline_hw_stream->remaining_pass = 2;
          } else {
              offline_hw_stream->remaining_pass = 1;
          }
          vfe_hw_params[isp_id].hw_streams[vfe_num_hw_streams[isp_id]++] =
            *offline_hw_stream;
          vfe_hw_params[isp_id].num_hw_streams = vfe_num_hw_streams[isp_id];
        }
      }
    }
    for (isp_id = 0; isp_id < VFE_MAX; isp_id++) {
      if (vfe_num_hw_streams[isp_id]) {
        rc = iface->isp_axi_data.axi_data[isp_id].axi_hw_ops->set_params(
          iface->isp_axi_data.axi_data[isp_id].axi_hw_ops->ctrl,
          IFACE_AXI_SET_STREAM_CFG, (void *)&vfe_hw_params[isp_id],
          sizeof(iface_hw_stream_t));
          if (rc < 0) {
            CDBG_ERROR("%s: offline VFE%d AXI_SET_STREAM_CFG failed! rc = %d\n",
              __func__, isp_id, rc);
            return rc;
          }
      }
    }

    for (i= 0; i < MSM_ISP_STATS_MAX; i++) {
      stats_stream = &iface_session->offline_stats_streams[i];
      if (stats_stream == NULL || stats_stream->stats_stream_id == 0) {
        CDBG("%s: stats_stream index %d not config yet, continue.\n",
          __func__, i);
        continue;
      } else {
        CDBG("%s: stats_stream found: stats_stream id = %x, stats type = %d,"
             " buf_len = %d, comp flag = %d\n",__func__,
          stats_stream->stats_stream_id, stats_stream->stats_type,
          stats_stream->buf_len, stats_stream->comp_flag);
      }
      for (isp_id = 0; isp_id < VFE_MAX ; isp_id++) {
        if (iface_session->session_resource.offline_isp_id_mask & (1 << isp_id)) {
          vfe_stats_stream[isp_id].stats_stream
            [vfe_stats_stream[isp_id].num_stats_streams++] = *stats_stream;
        }
      }
    }
    for (isp_id = 0; isp_id < VFE_MAX ; isp_id++) {
      if (iface_session->session_resource.offline_isp_id_mask & (1 << isp_id)) {
        CDBG("off_dbg req stats stream vfe id %d", isp_id);
        if ( vfe_stats_stream[isp_id].num_stats_streams > 0) {
          rc = iface->isp_axi_data.axi_data[isp_id].axi_hw_ops->set_params(
          iface->isp_axi_data.axi_data[isp_id].axi_hw_ops->ctrl,
            IFACE_AXI_SET_STATS_STREAM_CFG, (void *)&vfe_stats_stream[isp_id],
            sizeof(iface_stats_stream_set_params_t));
          if (rc < 0) {
            CDBG_ERROR("%s: VFE0 IFACE_AXI_SET_STATS_STREAM_CFG failed! rc = %d\n",
              __func__, rc);
            return rc;
          }
        }
      }
    }
    /*request buffer: hal buffer or native buffer*/
    rc = iface_util_request_image_buf(iface, iface_session, num_hw_streams,
      hw_stream_ids);
    if (rc < 0) {
      CDBG_ERROR("%s: iface_util_request_image_buf error = %d \n", __func__, rc);
      return rc;
    }

    /*  AXI streamon,
           at this point VFE pipeline is all set, just need read engine
           to start and pass pixel line to ISP.
       */
    for (isp_id = 0; isp_id < VFE_MAX; isp_id++) {
      memset(&start_param, 0, sizeof(start_stop_stream_t));
      for (i = 0; i < IFACE_MAX_STREAMS; i++) {
        /* Check for offline stream only */
        if (iface_session->hw_streams[i].stream_info.cam_stream_type !=
          CAM_STREAM_TYPE_OFFLINE_PROC) {
          continue;
        }
        offline_hw_stream = &iface_session->hw_streams[i];
        if (offline_hw_stream->interface_mask & (1 << (16 * isp_id +
          IFACE_INTF_PIX))) {
          offline_hw_stream->state = IFACE_HW_STREAM_STATE_ACTIVE;
          start_param.session_id = iface_session->session_id;
          start_param.stream_ids[start_param.num_streams++] =
            offline_hw_stream->hw_stream_id;
        }
      }
      if (start_param.num_streams > 0) {
        // We need to set this flag to TRUE for offline stream configuration
        // so that  when we start the stream, we subscribe for the v412 event.
        start_param.first_pix_start = TRUE;
        rc = iface->isp_axi_data.axi_data[isp_id].axi_hw_ops->action(
          iface->isp_axi_data.axi_data[isp_id].axi_hw_ops->ctrl,
          IFACE_AXI_ACTION_CODE_STREAM_START,
          (void *)&start_param, sizeof(start_stop_stream_t));
        if (rc < 0) {
          CDBG_ERROR("%s: VFE%d AXI_STREAM_STOP failed! rc = %d\n",
            __func__, isp_id, rc);
          return rc;
        }
      }
    }
    input_hw_stream = iface_util_find_hw_stream_by_mct_id(iface,
      iface_session, offline_info->input_stream_id);
    if (!input_hw_stream || !offline_hw_stream) {
      CDBG_ERROR("%s: can not in hw stream= %d offline_hw_stream %p\n",
        __func__, offline_info->input_stream_id, offline_hw_stream);
      return -1;
    }
    /* Start fetch engine with input buffer information*/
    iface_offline_start_fetch_engine(iface,
      iface_session, offline_hw_stream->hw_stream_id,
      input_hw_stream->hw_stream_id, offline_info);
  } else {
  /* isp is already configured just do pipeline config and fetch engine*/
    if (!offline_hw_stream) {
      CDBG_ERROR("%s: NULL ptr offline_hw_stream %p\n",
        __func__, offline_hw_stream);
      return -1;
    }
    /* Configure ISP offline pipeline */
    iface_util_start_isp_pipeline_cfg(iface, offline_info,
      ISP_FETCH_PIPELINE_REPROCESS, iface_session,
      offline_hw_stream->hw_stream_id);

    input_hw_stream = iface_util_find_hw_stream_by_mct_id(iface,
      iface_session, offline_info->input_stream_id);
    if (!input_hw_stream || !offline_hw_stream) {
      CDBG_ERROR("%s: can not in hw stream= %d offline_hw_stream %p\n",
        __func__, offline_info->input_stream_id, offline_hw_stream);
      return -1;
    }
    /* Start fetch engine with input buffer information*/
    iface_offline_start_fetch_engine(iface,
      iface_session, offline_hw_stream->hw_stream_id,
      input_hw_stream->hw_stream_id, offline_info);
  }

  return rc;
}

/** iface_util_dump_frame
 *
 * DESCRIPTION:
 *
 **/
void iface_util_dump_frame(int ion_fd, hw_stream_info_t *stream_info,
  cam_stream_type_t stream_type, iface_frame_buffer_t *image_buf,
  uint32_t frame_idx, uint32_t session_id)
{
  CDBG("%s: E", __func__);
  int32_t enabled = 0;
#ifdef _ANDROID_
  char value[PROPERTY_VALUE_MAX];
#endif
  char buf[64];
  int frm_num = 0;
  static int sDumpFrmCnt = 0;
  /* Usage: To enable dumps
    Preview: adb shell setprop persist.vendor.camera.isp.dump 2
    Analysis: adb shell setprop persist.vendor.camera.isp.dump 2048
    Snapshot: adb shell setprop persist.vendor.camera.isp.dump 8
    Video: adb shell setprop persist.vendor.camera.isp.dump 16
    To dump 10 frames again, just reset prop value to 0 and then set again */
#ifdef _ANDROID_
  property_get("persist.vendor.camera.isp.dump", value, "0");
  enabled = atoi(value);
#endif
  CDBG("%s:frame_idx %d enabled %d streamtype %d width %d height %d",
    __FUNCTION__, frame_idx, enabled,stream_type,
    stream_info->dim.width, stream_info->dim.height);

  if(!enabled){
    sDumpFrmCnt = 0;
    return;
  }
#ifdef _ANDROID_
  property_get("persist.vendor.camera.isp.dump_cnt", value, "10");
  frm_num = atoi(value);
#endif

  if((1<<(int)stream_type) & enabled) {
    CDBG_HIGH("%s: dump enabled for stream %d", __FUNCTION__,
      stream_type);

    if(sDumpFrmCnt >= 0 && sDumpFrmCnt <= frm_num) {
      int w, h;
      int file_fd;
      w = stream_info->dim.width;
      h = stream_info->dim.height;

      switch (stream_type) {
      case CAM_STREAM_TYPE_PREVIEW:
        snprintf(buf, sizeof(buf), QCAMERA_DUMP_FRM_LOCATION"isp_dump_%d_preview_%d_%d_%d.yuv",
          session_id, w, h, frame_idx);
        file_fd = open(buf, O_RDWR | O_CREAT, 0777);
        break;

      case CAM_STREAM_TYPE_VIDEO:
        snprintf(buf, sizeof(buf), QCAMERA_DUMP_FRM_LOCATION"isp_dump_%d_video_%d_%d.yuv",
          frame_idx, w, h);
        file_fd = open(buf, O_RDWR | O_CREAT, 0777);
        break;

      case CAM_STREAM_TYPE_POSTVIEW:
        snprintf(buf, sizeof(buf), QCAMERA_DUMP_FRM_LOCATION"isp_dump_%d_postview_%d_%d.yuv",
          frame_idx, w, h);
        file_fd = open(buf, O_RDWR | O_CREAT, 0777);
        break;

      case CAM_STREAM_TYPE_SNAPSHOT:
        snprintf(buf, sizeof(buf), QCAMERA_DUMP_FRM_LOCATION"isp_dump_%d_snapshot_%d_%d.yuv",
          frame_idx, w, h);
        file_fd = open(buf, O_RDWR | O_CREAT, 0777);
        break;

      case CAM_STREAM_TYPE_CALLBACK:
        snprintf(buf, sizeof(buf), QCAMERA_DUMP_FRM_LOCATION"isp_dump_%d_callback_%d_%d.yuv",
          frame_idx, w, h);
        file_fd = open(buf, O_RDWR | O_CREAT, 0777);
        break;

      case CAM_STREAM_TYPE_RAW:
        snprintf(buf, sizeof(buf), QCAMERA_DUMP_FRM_LOCATION"isp_dump_%d_raw_%d_%d.raw",
          frame_idx, w, h);
        file_fd = open(buf, O_RDWR | O_CREAT, 0777);
        break;

      case CAM_STREAM_TYPE_OFFLINE_PROC:
        snprintf(buf, sizeof(buf), QCAMERA_DUMP_FRM_LOCATION"isp_dump_%d_offline_%d_%d.yuv",
          frame_idx, w, h);
        file_fd = open(buf, O_RDWR | O_CREAT, 0777);
        break;

      case CAM_STREAM_TYPE_PARM:
        CDBG_HIGH("<pdaf_dbg> W = %d h = %d ", w, h);
        snprintf(buf, sizeof(buf), QCAMERA_DUMP_FRM_LOCATION"isp_dump_%d_subsampled_raw_%d_%d.raw",
          frame_idx, w, h);
        file_fd = open(buf, O_RDWR | O_CREAT, 0777);
        break;
      case CAM_STREAM_TYPE_ANALYSIS:
        snprintf(buf, sizeof(buf), QCAMERA_DUMP_FRM_LOCATION"isp_dump_%d_analysis_%d_%d_%d.yuv",
          session_id, w, h, frame_idx);
        file_fd = open(buf, O_RDWR | O_CREAT, 0777);
        break;

      case CAM_STREAM_TYPE_DEPTH:
        snprintf(buf, sizeof(buf), QCAMERA_DUMP_FRM_LOCATION"isp_dump_%d_depth_%d_%d_%d.raw",
          session_id, w, h, frame_idx);
        file_fd = open(buf, O_RDWR | O_CREAT, 0777);
        break;

      case CAM_STREAM_TYPE_METADATA:
      case CAM_STREAM_TYPE_DEFAULT:
      default:
        file_fd = -1;
        break;
      }

      if (file_fd < 0) {
        CDBG_ERROR("%s: cannot open file\n", __func__);
      } else {
        CDBG("%s: num_planes %d", __FUNCTION__, image_buf->buffer.length);
        unsigned int i = 0;
        uint8_t *vaddr = NULL;

        for (i=0;i<image_buf->buffer.length && i < VIDEO_MAX_PLANES; i++) {
          /*vaddr now use per plane v address*/
          vaddr = (uint8_t *)image_buf->addr[i];

          if (vaddr == NULL)
            continue;
          CDBG("%s:file_fd %d vaddr[%d] %x, size %d \n", __FUNCTION__, file_fd, i,
          (unsigned int)(vaddr + image_buf->buffer.m.planes[i].data_offset),
          image_buf->buffer.m.planes[i].length );

          write(file_fd,
            (const void *)(vaddr + image_buf->buffer.m.planes[i].data_offset),
            image_buf->buffer.m.planes[i].length);
        }

        close(file_fd);
        /* buffer is invalidated from cache*/
        iface_do_cache_inv_ion(ion_fd, image_buf,
          CPU_HAS_READ);
      }
    }
    sDumpFrmCnt++;
  }

end:
  return;
}

/** iface_util_update_offline_axi_stream
 *
 *    @ctrl:
 *    @thread:
 *    @action:
 *
 **/
int iface_util_update_offline_axi_stream(
  iface_t *iface, iface_stream_update_t *stream_update_parm, uint32_t isp_id)
{
  int rc = 0;

   rc = iface->isp_axi_data.axi_data[isp_id].axi_hw_ops->set_params(
      iface->isp_axi_data.axi_data[isp_id].axi_hw_ops->ctrl,
      IFACE_AXI_SET_OFFLINE_STREAM_UPDATE, (void *)stream_update_parm,
      sizeof(iface_hw_stream_t));
    if (rc < 0) {
      CDBG_ERROR("%s: VFE0 ISP_AXI_SET_STREAM_CFG failed! VFE0 rc = %d\n",
        __func__, rc);
      return rc;
    }

    rc = iface->isp_axi_data.axi_data[isp_id].axi_hw_ops->action(
       iface->isp_axi_data.axi_data[isp_id].axi_hw_ops->ctrl,
       IFACE_AXI_ACTION_CODE_HW_UPDATE, NULL, 0);
     if (rc < 0) {
       CDBG_ERROR("%s: VFE0 IFACE_AXI_ACTION_CODE_HW_UPDATE failed! rc = %d\n",
         __func__, rc);
       return rc;
     }

  return rc;
}

/** iface_util_get_dwords_per_line
 *
 *  @fmt: output format
 *
 */
int iface_util_get_bits_per_pixels(
  cam_format_t format)
{
  switch (format) {
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
      return 8;
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
      return 10;
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
      return 12;
    case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_BGGR:
    case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GBRG:
    case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GRBG:
    case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_RGGB:
      return 14;
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_BGGR:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GBRG:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GRBG:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_RGGB:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_BGGR:
      return 16;
    default :
      return 16;
    }
}

int iface_util_offline_process_multi_pass(iface_t *iface,
  iface_session_t *session, struct msm_isp_event_data *buf_divert,
  iface_hw_stream_t *hw_stream)
{
   iface_hw_stream_t          *input_hw_stream;
   iface_offline_isp_info_t   *offline_info;
   iface_stream_update_t      stream_update_parm;
   int                        isp_id, rc = 0;

  if (!iface || !buf_divert || !session || !hw_stream) {
    CDBG_ERROR("%s: NULL pointer %p %p %p %p\n", __func__,
      iface, buf_divert, session, hw_stream);
    return -1;
  }

  offline_info = &session->offline_info;
  /* Configure ISP pipeline for second pass*/
  iface_util_start_isp_pipeline_cfg(iface, &session->offline_info,
     ISP_FETCH_PIPELINE_REPROCESS, session,
     hw_stream->hw_stream_id);

  /* Update AXI stream, for second pass */
  for (isp_id = 0; isp_id < VFE_MAX; isp_id++) {
     if (hw_stream->interface_mask & (1 << (16 * isp_id +
         IFACE_INTF_PIX))) {
       memset(&stream_update_parm, 0, sizeof(iface_stream_update_t));
       stream_update_parm.hw_stream_ids[stream_update_parm.num_hw_streams++] =
         hw_stream->hw_stream_id;
       stream_update_parm.session_id = session->session_id;
       stream_update_parm.stripe_id = ISP_STRIPE_RIGHT;

       rc = iface_util_update_offline_axi_stream(iface, &stream_update_parm,
         isp_id);
       if (rc < 0) {
         CDBG_ERROR("%s: error iface_util_update_axi_stream! rc = %d\n",
           __func__, rc);
         return rc;
       }
       /* Configure read engine /fetch engine*/
       rc = iface_util_fe_cfg(session,
         iface->isp_axi_data.axi_data[isp_id].fd,
         offline_info->input_dim, offline_info->op_pixel_clk,
         offline_info->input_fmt, ISP_STRIPE_RIGHT);
       if (rc < 0) {
         CDBG_ERROR("%s: error iface_util_fe_cfg! rc = %d\n",
           __func__, rc);
       }
      }
  }

  input_hw_stream = iface_util_find_hw_stream_by_mct_id(iface,
    session, session->offline_info.input_stream_id);
  if (!input_hw_stream) {
    CDBG_ERROR("%s: can not in hw stream= %d\n",
      __func__, session->offline_info.input_stream_id);
    return -1;
  }

   /* Start fetch engine with input buffer information*/
#ifdef VIDIOC_MSM_ISP_MAP_BUF_START_MULTI_PASS_FE
   offline_info->offline_pass = OFFLINE_SECOND_PASS;
#endif
   offline_info->input_buf_offset =
     (session->session_resource.offline_split_info.right_stripe_offset *
     iface_util_get_bits_per_pixels(session->offline_info.input_fmt)) / 8;
   offline_info->output_buf_idx = buf_divert->u.buf_done.buf_idx;
   offline_info->output_stream_id = hw_stream->hw_stream_id;
   rc = iface_offline_start_fetch_engine(iface,
     session, hw_stream->hw_stream_id,
     input_hw_stream->hw_stream_id, offline_info);
   return rc;
}

/** iface_util_buf_divert_notify
 *
 * DESCRIPTION:
 *
 **/
int iface_util_buf_divert_notify(iface_t *iface,
  iface_frame_divert_notify_t *divert_event)
{
  int rc = 0;
  uint32_t i = 0;
  uint32_t j = 0;
  struct msm_isp_event_data     *buf_divert = NULL;
  isp_buf_divert_t              pp_divert;
  iface_hw_stream_t             *hw_stream = NULL;
  iface_stream_t                *user_stream;
  mct_event_t                   mct_event;
  iface_port_t                  *iface_port;
  iface_frame_buffer_t          *image_buf;
  boolean                       is_buf_dirty = FALSE;
  iface_session_t               *session = NULL;
  int32_t                       buffs_id;
  mct_bus_msg_t                 bus_msg;
  cam_buf_divert_info_t divert_msg;
  cam_dual_camera_perf_control_t perf_ctrl;

  memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
  memset(&divert_msg, 0, sizeof(cam_buf_divert_info_t));

  if (!iface || !divert_event) {
    CDBG_ERROR("%s: NULL pointer %p %p\n", __func__,
      iface, divert_event);
    return -1;
  }

  buf_divert = divert_event->isp_event_data;
  if (!buf_divert) {
    CDBG_ERROR("%s: NULL pointer %p\n", __func__, buf_divert);
    return -1;
  }

  session = iface_util_get_session_by_id(iface,
    buf_divert->u.buf_done.session_id);
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n",
      __func__, buf_divert->u.buf_done.session_id);
    return -1;
  }
  /* In dual camera SAT use case when LPM switch happens,need to
     notify cpp once a buffer returns from wake up session.*/
  if (session->cur_lpm_mode == 0 && session->prev_lpm_mode == 1) {
   for (i = 0; i < IFACE_MAX_STREAMS; i++) {
    if (session->hw_streams[i].state == IFACE_HW_STREAM_STATE_ACTIVE) {
      hw_stream = &session->hw_streams[i];
  if (!hw_stream) {
    CDBG_ERROR("%s:%d can not find active  hw stream %p\n",
      __func__, __LINE__, hw_stream);
    return 0;
  }
  for (j = 0; j < hw_stream->num_mct_stream_mapped
    && j < IFACE_MAX_STREAMS; j++) {
    user_stream = iface_util_find_stream_in_sink_port_list(iface,
    session->session_id, hw_stream->mapped_mct_stream_id[j]);
  if (!user_stream || !user_stream->src_port) {
    CDBG_ERROR("%s:%d NULL pointer  rc = -1\n",
      __func__, __LINE__);
    return -1;
  }
  memset(&mct_event, 0, sizeof(mct_event));
  mct_event.type = MCT_EVENT_CONTROL_CMD;
  mct_event.direction = MCT_EVENT_DOWNSTREAM;
  mct_event.identity = pack_identity(hw_stream->session_id, user_stream->stream_id);
  mct_event.u.ctrl_event.type = MCT_EVENT_CONTROL_HW_WAKEUP;
  perf_ctrl.enable = 0;
  mct_event.u.ctrl_event.control_event_data = (void *)&perf_ctrl;
  rc = mct_port_send_event_to_peer(user_stream->src_port, &mct_event);
   if (rc < 0) {
      CDBG_ERROR("%s:can't post start sof event to cpp on wake\n", __func__);
    }
   }
  }
 }
}
  /* Update the previous lpm mode to current */
  session->prev_lpm_mode = session->cur_lpm_mode;

  hw_stream = iface_util_find_hw_stream_in_session(session,
    buf_divert->u.buf_done.stream_id);
  if (!hw_stream) {
     uint32_t user_bufq_handle = 0;
     CDBG_ERROR("%s: cannot find hw_stream, sess_id = %d, hw_stream_id = %x,"
       " directly queue back buf!\n",
       __func__, buf_divert->u.buf_done.session_id,
       buf_divert->u.buf_done.stream_id);
     is_buf_dirty = TRUE;
     user_bufq_handle = iface_find_matched_bufq_handle_by_kernel_handle(
       &iface->buf_mgr, buf_divert->u.buf_done.handle);
     rc = iface_queue_buf(&iface->buf_mgr, user_bufq_handle,
       buf_divert->u.buf_done.buf_idx, is_buf_dirty, 0, 0);
     if (rc < 0) {
       CDBG_ERROR("%s: queue back buffer failed!"
        "hw stream id %x, bufq handle %x, buf_idx %d rc = %d\n",
        __func__, buf_divert->u.buf_done.stream_id, user_bufq_handle,
        buf_divert->u.buf_done.buf_idx, rc);
    }
    return rc;
  }

  if (hw_stream->stream_info.cam_stream_type == CAM_STREAM_TYPE_OFFLINE_PROC &&
     (--hw_stream->remaining_pass >= 1)) {
    rc = iface_util_offline_process_multi_pass(iface, session,
      buf_divert, hw_stream);
    return rc;
  }

  int num_hw_stream = iface_util_count_diverted_hw_stream_in_session(session);
  if ((CAM_STREAMING_MODE_BURST == hw_stream->streaming_mode) &&
      (hw_stream->num_burst * num_hw_stream <= ++iface->frame_cnt)) {
    iface->skip_meta = 1;
  } else {
    iface->skip_meta = 0;
  }

  buffs_id = iface_util_get_buffers_id(hw_stream,
    buf_divert->u.buf_done.stream_id);
  if (buffs_id < 0 || buffs_id >= VFE_BUF_QUEUE_MAX) {
    CDBG_ERROR("%s: can not find buffers id, rc = -1\n", __func__);
    return -1;
  }

  pp_divert.buffer_access = 0;
  /* send out the event needs the user stream linked prot*/
  user_stream = iface_util_find_stream_in_sink_port_list(iface,
    hw_stream->session_id, hw_stream->mct_streamon_id);
  if (!user_stream) {
     CDBG_ERROR("%s: cannot find user_stream, sess_id = %d, "
       "user_stream_id = %x, directly queue back buf!\n",
       __func__, hw_stream->session_id, hw_stream->mct_streamon_id);
     is_buf_dirty = TRUE;
     goto ERROR;
  }
  bus_msg.sessionid = session->session_id;
  bus_msg.type = MCT_BUS_MSG_BUF_DIVERT_INFO;
  bus_msg.size = sizeof(cam_buf_divert_info_t);
  bus_msg.msg = &divert_msg;
  divert_msg.frame_id = buf_divert->frame_id;
  for (i = 0; i < hw_stream->num_mct_stream_mapped
    && i < MAX_NUM_STREAMS; i++) {
    divert_msg.stream_id[i] = hw_stream->mapped_mct_stream_id[i];
    divert_msg.num_streams++;
  }
  if (TRUE != mct_module_post_bus_msg(iface->module, &bus_msg)) {
    CDBG_ERROR("%s: buf divert to bus error, mct stream %d, frame %d\n",
      __func__, divert_msg.stream_id, divert_msg.frame_id);
  }
  /*fill in buffer divert event struct*/
  memset(&mct_event,  0,  sizeof(mct_event));
  mct_event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT;
  mct_event.u.module_event.module_event_data = (void *)&pp_divert;
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.identity = pack_identity(hw_stream->session_id,
    hw_stream->mct_streamon_id);
  mct_event.direction = MCT_EVENT_DOWNSTREAM;

  memset(&pp_divert, 0, sizeof(pp_divert));
  pp_divert.identity = mct_event.identity;
  pp_divert.native_buf = hw_stream->use_native_buf;

  image_buf = iface_get_buf_by_idx(&iface->buf_mgr,
     hw_stream->buffers_info[buffs_id].bufq_handle,
     buf_divert->u.buf_done.buf_idx);
  if (!image_buf) {
    CDBG_ERROR("%s: iface_get_buf_by_idx error, directly queue back buf\n",
      __func__);
    is_buf_dirty = TRUE;
    goto ERROR;
  }

  /* image_buf->vaddr : the actual Virdual address of the buffer.
     image_buf->addr: is a pointer to Virtual address of the buffer.
                      since other modules already did the de reference,
                      temp keep as it is.  */
  if(hw_stream->divert_to_3a) {
    pp_divert.vaddr = image_buf->vaddr;
    mct_event.u.module_event.type = MCT_EVENT_MODULE_ISP_DIVERT_TO_3A;
    pp_divert.stats_type = hw_stream->ext_stats_type;
    /* Assume sensor stats will always be READ by CPU */
    pp_divert.buffer_access |= CPU_HAS_READ;
  } else {
     /* todo: change to per plane address.*/
    pp_divert.vaddr = image_buf->addr;
    pp_divert.buffer_access |= 0;
  }

  /*add data offset into divert event*/
  for (i = 0; i < IFACE_MAX_NUM_PLANE; i++) {
    pp_divert.plane_vaddr[i] = image_buf->addr[i];
    /*image_buf->buufer is v4l2 format*/
    CDBG("%s: plane[%d] offset = %d\n", __func__, i,
      image_buf->buffer.m.planes[i].data_offset);
  }
  pp_divert.max_buffers = iface_get_current_num_buffer(&iface->buf_mgr,
                          hw_stream->buffers_info[buffs_id].bufq_handle);
  pp_divert.sof_timestamp = buf_divert->mono_timestamp;
  pp_divert.frame_id = buf_divert->frame_id;
  pp_divert.fd = image_buf->fd;
  pp_divert.buffer = image_buf->buffer;
  pp_divert.buffer.sequence = buf_divert->frame_id;
  pp_divert.buffer.timestamp = buf_divert->timestamp;
  pp_divert.channel_id = hw_stream->mct_streamon_id;
  pp_divert.is_uv_subsampled =
    (buf_divert->u.buf_done.output_format == V4L2_PIX_FMT_NV14 ||
     buf_divert->u.buf_done.output_format == V4L2_PIX_FMT_NV41);
  if (hw_stream->stream_info.batch_mode) {
    pp_divert.batch_info.batch_mode = hw_stream->stream_info.batch_mode;
    pp_divert.batch_info.batch_size = hw_stream->stream_info.batch_size;
    pp_divert.batch_info.intra_plane_offset[0] =
      (hw_stream->stream_info.buf_planes.plane_info.mp[0].scanline *
      hw_stream->stream_info.buf_planes.plane_info.mp[0].stride)/
      hw_stream->stream_info.batch_size;
    pp_divert.batch_info.intra_plane_offset[1] =
      (hw_stream->stream_info.buf_planes.plane_info.mp[1].scanline *
      hw_stream->stream_info.buf_planes.plane_info.mp[1].stride)/
      hw_stream->stream_info.batch_size;
    if (pp_divert.is_uv_subsampled)
      pp_divert.batch_info.intra_plane_offset[1]/=2;
  }
#ifdef ISP_SVHDR_IN_BIT
  if(hw_stream->hw_stream_id & ISP_SVHDR_IN_BIT) {
     pp_divert.bayerdata = 1;
  }
  else {
     pp_divert.bayerdata = 0;
  }
#endif
  if (session->stats_skip_pattern == NO_SKIP) {
    pp_divert.stats_valid = TRUE;
  } else {
    if (iface_util_get_frame_skip_period(session->stats_skip_pattern) !=0 &&
      ((buf_divert->frame_id - 1) %
      (iface_util_get_frame_skip_period(session->stats_skip_pattern))) == 0)
      pp_divert.stats_valid = TRUE;
  }
  iface_util_dump_frame(iface->buf_mgr.ion_fd, &hw_stream->stream_info,
    user_stream->stream_info.stream_type, image_buf,
    buf_divert->frame_id, session->session_id);

  for (i = 0; i < hw_stream->num_mct_stream_mapped; i++) {
    if (session->prev_sent_streamids_cnt < MAX_STREAMS_NUM) {
      session->prev_sent_streamids[session->prev_sent_streamids_cnt++] =
          hw_stream->mapped_mct_stream_id[i];
    } else {
      CDBG_ERROR("%s: error: session->prev_sent_streamids_cnt %d is more than %d\n",
        __func__, session->prev_sent_streamids_cnt, MAX_STREAMS_NUM);
      is_buf_dirty = TRUE;
      goto ERROR;
    }
  }

  pthread_mutex_lock(&session->mutex);
  if (!hw_stream->divert_to_3a) {
    /*sanity check if frame id and time stamp reverse*/
    if (session->overflow_recovery_state !=
          IFACE_AXI_RECOVERY_STATE_NO_OVERFLOW &&
        buf_divert->frame_id == hw_stream->last_frame_id){
      IFACE_HIGH("Drop duplicated frames in recovery: hw stream 0x%x, frame id %d\n",
        hw_stream->hw_stream_id, buf_divert->frame_id);
      is_buf_dirty = TRUE;
      pthread_mutex_unlock(&session->mutex);
      goto ERROR;
    } else if ((buf_divert->frame_id <= hw_stream->last_frame_id &&
        hw_stream->stream_info.cam_stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) ||
       ((buf_divert->timestamp.tv_sec <= hw_stream->last_timestamp.tv_sec) &&
       (buf_divert->timestamp.tv_usec <= hw_stream->last_timestamp.tv_usec))) {
      pthread_mutex_unlock(&session->mutex);

      CDBG_INFO("%s: HW error! frame id reverse!! sessid %d, hw streamid %X\n",
        __func__, user_stream->session_id,
        hw_stream->hw_stream_id);
      CDBG_INFO("%s: curr: frame %d, sec %ld usec %ld\n",
        __func__, buf_divert->frame_id, buf_divert->timestamp.tv_sec,
        buf_divert->timestamp.tv_usec);
      CDBG_INFO("%s: prev: frame %d, sec %ld usec %ld\n",
        __func__, hw_stream->last_frame_id, hw_stream->last_timestamp.tv_sec,
        hw_stream->last_timestamp.tv_usec);

      /*when set dirty buf, kernel will put buf*/
      is_buf_dirty = TRUE;
      goto ERROR;
    } else {
      /*if pass sanity then do divert*/
      hw_stream->last_frame_id = buf_divert->frame_id;
      hw_stream->last_timestamp = buf_divert->timestamp;
    }
  }
  pthread_mutex_unlock(&session->mutex);

  CDBG(" buf_divert to pp frame_id %d, streamid %d, buf_idx %d buf_handle %x, bufq_handle %x bayerdata %d\n",
    buf_divert->frame_id,
    user_stream->stream_id, buf_divert->u.buf_done.buf_idx, buf_divert->u.buf_done.handle,
    hw_stream->buffers_info[buffs_id].bufq_handle, pp_divert.bayerdata);

  /*iface to isp only have one port*/
  if (user_stream->src_port) {
    pthread_mutex_lock(&session->mutex);
    int buff_id = iface_util_get_buffers_id(hw_stream, user_stream->stream_id);
    bool ret = FALSE;
    if (buff_id < 0 || buff_id >= VFE_BUF_QUEUE_MAX) {
      IFACE_ERR("can not find buffers id, rc = -1");
      pthread_mutex_unlock(&session->mutex);
      return -1;
    }
    if (hw_stream->buffers_info[buff_id].is_mapped != FALSE)
      ret = mct_port_send_event_to_peer(user_stream->src_port, &mct_event);
    pthread_mutex_unlock(&session->mutex);

    if (FALSE == ret) {
      CDBG_ERROR("%s: buf_divert to pp, sessid %d, streamid %d, rc = -1\n",
        __func__, user_stream->session_id,
        user_stream->stream_id);
     is_buf_dirty = TRUE;
     goto ERROR;
    } else {
      /* in success case we need to check
       * if we have piggy back ack in the return */
      if (pp_divert.ack_flag == TRUE) {
        /* process the piggy back ack */
        divert_event->ack_flag = pp_divert.ack_flag;
        divert_event->is_buf_dirty = pp_divert.is_buf_dirty;
        is_buf_dirty = pp_divert.is_buf_dirty;
        goto ERROR;
      }
    }
  } else {
    CDBG_ERROR("%s: no src frame port linked, sessid = %d, streamid = %d, rc = -1\n",
      __func__, user_stream->session_id,
        user_stream->stream_id);
     is_buf_dirty = TRUE;
     goto ERROR;
  }

  return 0;

ERROR:
  if (hw_stream == NULL) {
    CDBG_ERROR("%s: NULL pointer, hw_Stream %p, rc = -1\n",
      __func__, hw_stream);
    return -1;
  }

  rc = iface_queue_buf(&iface->buf_mgr,
    hw_stream->buffers_info[buffs_id].bufq_handle,
    buf_divert->u.buf_done.buf_idx, is_buf_dirty, 0,
    pp_divert.buffer_access);
  if (rc < 0) {
    CDBG_ERROR("%s: queue back buffer failed!"
      "hw stream id %x, bufq handle %x, buf_idx %d rc = %d\n",
      __func__, buf_divert->u.buf_done.stream_id,
      hw_stream->buffers_info[buffs_id].bufq_handle,
      buf_divert->u.buf_done.buf_idx, rc);
  }
  return rc;
}
/** iface_util_send_offline_cfg_request
 *    @iface: ISP pointer
 *    @session_id: session id
 *    @stream_id: straem id
 *    @fetch_engine_cfg_request: fetch cfg event pointer
 *
 *
 * Return: 0 - success, negative vale - error.
 **/
int iface_util_send_offline_cfg_request(
  iface_t                  *iface,
  iface_fetch_engine_cfg_t *fetch_engine_cfg_request,
  iface_session_t          *session,
  uint32_t                  hw_stream_id)
{
  int                rc = 0;
  iface_port_t      *iface_port = NULL;
  iface_stream_t    *user_stream = NULL;
  iface_hw_stream_t *hw_stream = NULL;
  mct_event_t        mct_event;
  int                i = 0;

  if (!iface || !fetch_engine_cfg_request) {
    CDBG_ERROR("%s: NULL pointer, %p %p\n",
      __func__, iface, fetch_engine_cfg_request);
    return rc;
  }

  if (!session) {
    CDBG_ERROR("%s: can not find session id\n",
      __func__);
    return -1;
  }

  CDBG("%s: E, session_d %d, stream_id %d",
    __func__, session->session_id, hw_stream_id);

  hw_stream = iface_util_find_hw_stream_in_session(session,
    hw_stream_id);
  if (!hw_stream) {
    CDBG_ERROR("%s: can not find HW stream id %d\n",
      __func__, hw_stream_id);
    return -1;
  }

  user_stream = iface_util_find_stream_in_sink_port_list(iface,
    session->session_id, hw_stream->mapped_mct_stream_id[0]);
  if (!user_stream) {
    CDBG_ERROR("%s: can not find user stream %x!\n",
      __func__, hw_stream->mapped_mct_stream_id[0]);
    return rc;
  }

  memset(&mct_event, 0, sizeof(mct_event));

  mct_event.u.module_event.type = MCT_EVENT_MODULE_ISP_OFFLINE_PIPELINE_CONFIG;
  mct_event.u.module_event.module_event_data =
    (void *)fetch_engine_cfg_request;
  mct_event.u.module_event.current_frame_id =
      session->offline_info.offline_frame_id;
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.identity = pack_identity(session->session_id, user_stream->stream_id);
  mct_event.direction = MCT_EVENT_DOWNSTREAM;
  mct_port_send_event_to_peer(user_stream->src_port, &mct_event);

  CDBG("%s: X", __func__);
  return rc;
}

/** iface_util_start_fetch_pipeline_cfg
 *    @iface: iface pointer
 *    @session_id: session id
 *    @stream_id: straem id
 *
 *
 * Return: 0 - success, negative vale - error.
 **/
int iface_util_start_isp_pipeline_cfg(
  iface_t                  *iface,
  iface_offline_isp_info_t *offline_info,
  iface_fetch_engine_cmd_t  fetch_cfg_cmd,
  iface_session_t          *session,
  uint32_t                  hw_stream_id)
{
  int                       i = 0;
  int                       rc = 0;
  iface_port_t             *iface_port = NULL;
  iface_hw_stream_t        *hw_stream = NULL;
  iface_fetch_engine_cfg_t  fetch_engine_cfg_request;
  uint32_t                  offline_vfe_id_mask = 0;
  cam_stream_parm_buffer_t temp_local_stream_param_buf;

  if (!iface || !offline_info) {
    CDBG_ERROR("%s: NULL pointer, %p %p\n",
      __func__, iface, offline_info);
    return -1;
  }

  if (!session) {
    CDBG_ERROR("%s: can not find session, rc = -1\n", __func__);
    return -1;
  }

  CDBG_HIGH("%s: E, session_d %d, stream_id %d",
    __func__, session->session_id, hw_stream_id);

  hw_stream = iface_util_find_hw_stream_in_session(session,
    hw_stream_id);
  if (!hw_stream) {
    CDBG_ERROR("%s: can not find HW stream id %d\n",
      __func__, hw_stream_id);
    return -1;
  }

  for (i = 0; i < VFE_MAX; i++) {
    if (((hw_stream->interface_mask & (0xFFFF << (i * 16))) != 0)) {
      offline_vfe_id_mask |= (1 << i);
    }
  }

  memset(&fetch_engine_cfg_request, 0, sizeof(iface_fetch_engine_cfg_t));
  fetch_engine_cfg_request.fetch_cfg_cmd = fetch_cfg_cmd;
  fetch_engine_cfg_request.isp_id_mask = offline_vfe_id_mask;
  fetch_engine_cfg_request.stream_param_buf = offline_info->stream_param_buf;
  fetch_engine_cfg_request.input_stream_info = offline_info->input_stream_info;
  fetch_engine_cfg_request.input_dim = offline_info->input_dim;
  fetch_engine_cfg_request.input_fmt = offline_info->input_fmt;
  /* For Second pass Send the split output info */
  if (fetch_cfg_cmd == ISP_FETCH_PIPELINE_REPROCESS) {
      fetch_engine_cfg_request.isp_split_output_info =
        hw_stream->isp_split_output_info;
  }
  iface_util_send_offline_cfg_request(iface,
    &fetch_engine_cfg_request, session, hw_stream_id);

  CDBG("%s: X", __func__);
  if (fetch_cfg_cmd == ISP_FETCH_PIPELINE_START) {
      hw_stream->isp_split_output_info =
        fetch_engine_cfg_request.isp_split_output_info;
      session->session_resource.offline_split_info =
        fetch_engine_cfg_request.ispif_split_output_info;
  }

  return rc;
}

/** iface_util_broadcast_sof_msg_to_modules
 *    @isp: ISP pointer
 *    @session_id: session id
 *    @stream_id: straem id
 *    @sof_event: sof event pointer
 *
 * Process any v4l2_event received from kernel ISP driver.
 *
 * Return: 0 - success, negative vale - error.
 **/
void iface_util_broadcast_sof_msg_to_modules(iface_t *iface, uint32_t session_id,
  uint32_t user_stream_id, mct_bus_msg_isp_sof_t *sof_event)
{
  iface_port_t *iface_port;
  iface_stream_t *user_stream;
  mct_event_t mct_event;
  iface_session_t *session;

  CDBG("%s: E, session_d %d, stream_id %d",
    __func__, session_id, user_stream_id);

  session = iface_util_get_session_by_id(iface, session_id);
  if (!session)
    return;

  user_stream = iface_util_find_stream_in_sink_port_list(iface, session_id, user_stream_id);
  if (!user_stream)
    return;

  memset(&mct_event, 0, sizeof(mct_event));
  mct_event.u.module_event.type = MCT_EVENT_MODULE_SOF_NOTIFY;
  mct_event.u.module_event.module_event_data = (void *)sof_event;
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.identity = pack_identity(session_id, user_stream_id);
  mct_event.direction = MCT_EVENT_UPSTREAM;
  /* broadcast sof upstream */
  mct_port_send_event_to_peer(user_stream->sink_port, &mct_event);

 /* broadcast sof downstream */
  mct_event.direction = MCT_EVENT_DOWNSTREAM;
  mct_port_send_event_to_peer(user_stream->src_port, &mct_event);

  CDBG("%s: X", __func__);
}


/** iface_util_broadcast_fe_sof_msg_to_modules
 *    @isp: ISP pointer
 *    @session_id: session id
 *    @stream_id: straem id
 *    @sof_event: sof event pointer
 *
 * Process any v4l2_event received from kernel ISP driver.
 *
 * Return: 0 - success, negative vale - error.
 **/
void iface_util_broadcast_fe_sof_msg_to_modules(iface_t *iface, uint32_t session_id,
  uint32_t user_stream_id, mct_bus_msg_isp_sof_t *sof_event)
{
  iface_port_t *iface_port;
  iface_stream_t *user_stream;
  mct_event_t mct_event;
  iface_session_t *session;

  CDBG("%s: E, session_d %d, stream_id %d",
    __func__, session_id, user_stream_id);

  session = iface_util_get_session_by_id(iface, session_id);
  if (!session)
    return;

  user_stream = iface_util_find_stream_in_sink_port_list(iface, session_id, user_stream_id);
  if (!user_stream)
    return;

  memset(&mct_event, 0, sizeof(mct_event));
  mct_event.u.module_event.type = MCT_EVENT_MODULE_FE_SOF;
  mct_event.u.module_event.module_event_data = (void *)sof_event;
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.identity = pack_identity(session_id, user_stream_id);
  mct_event.direction = MCT_EVENT_DOWNSTREAM;
  CDBG("send MCT_EVENT_MODULE_FE_SOF session_d %d, stream_id %d",
     session_id, user_stream_id);

  /* broadcast sof upstream */
  mct_port_send_event_to_peer(user_stream->sink_port, &mct_event);

  CDBG("%s: X", __func__);
}

static void iface_update_thread_parm_in_sof(
  iface_session_thread_t *session_thread)
{
  pthread_mutex_lock(&session_thread->sof_mutex);
  /* wake up session thread if waiting for sof*/
  if (session_thread->wait_for_sof) {
     session_thread->wait_for_sof = FALSE;
     sem_post(&session_thread->wait_sof_sem);
  }
  pthread_mutex_unlock(&session_thread->sof_mutex);

}

/** iface_util_check_dual_vfe_notify
 *
 * DESCRIPTION:
 *
 **/
static int iface_util_check_dual_vfe_notify(
  iface_session_t *session,
  uint32_t         notify_event,
  uint32_t         axi_idx,
  uint32_t         frame_id __unused,
  boolean         *update_needed)
{
  if (!session || (axi_idx >= VFE_MAX) || !update_needed) {
    CDBG_ERROR("%s: Error! session param %p axi_idx %d update_needed %p\n",
      __func__, session, axi_idx, update_needed);
    return -EINVAL;
  }

  /* Fill default */
  *update_needed = FALSE;

  pthread_mutex_lock(&session->mutex);
  session->dual_vfe_notify[notify_event][axi_idx] = TRUE;

  if (((session->dual_vfe_notify[notify_event][VFE0] == TRUE) &&
      (session->dual_vfe_notify[notify_event][VFE1] == TRUE)) ||
      ((session->dual_vfe_notify[notify_event][VFE0] == TRUE) &&
      session->dual_isp_sync)) {
    *update_needed = TRUE;
    session->dual_vfe_notify[notify_event][VFE0] =
      session->dual_vfe_notify[notify_event][VFE1] = FALSE;
  }
  pthread_mutex_unlock(&session->mutex);

  return 0;
}

/** iface_util_notify_update_stream_done
 *
 * DESCRIPTION:
 *
 **/
static int iface_util_notify_update_stream_done(iface_t *iface,
  uint32_t session_id, uint32_t axi_idx, void *notify_data,
  uint32_t notify_data_size)
{
  iface_session_t *session = NULL;
  iface_session_thread_t *session_thread = NULL;
  uint32_t *frame_id = notify_data;
  boolean update_needed = FALSE;
  mct_bus_msg_iface_metadata_t iface_metadata;
  int32_t rc = 0;
  uint32_t q_idx = 0;

  if ((!frame_id) || (notify_data_size != sizeof(uint32_t))) {
    CDBG_ERROR("%s: Error! Invalid params data %p data_size %u expected %u\n",
      __func__, notify_data, notify_data_size, sizeof(uint32_t));
    return -1;
  }

  session = iface_util_get_session_by_id(iface, session_id);
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n",
      __func__, session_id);
    return -1;
  }

  if (session->session_resource.num_isps > 1) {
    rc = iface_util_check_dual_vfe_notify(session,
      IFACE_AXI_NOTIFY_STREAM_UPDATE_DONE, axi_idx, *frame_id, &update_needed);
    if ((rc < 0) || (update_needed == FALSE)) {
      /* skip notify in dual_vfe case since both vfe event are not received */
      return 0;
    }
  }

  session_thread = &session->session_thread;

  pthread_mutex_lock(&session_thread->busy_mutex);
  session->cds_curr_enb = session_thread->uv_subsample_parm.enable;
  pthread_mutex_unlock(&session_thread->busy_mutex);

  memset(&iface_metadata, 0, sizeof(iface_metadata));
  if (session->cds_curr_enb > 0) {
    iface_metadata.is_uv_subsampled = 1;
  } else {
    iface_metadata.is_uv_subsampled = 0;
  }
  iface_metadata.type = MCT_BUS_MSG_IFACE_METADATA;
  /* If this function is called in frame X, cds switch happens in frame X
   * This has to be posted in frame X+1 (after max_reporting_delay),
   * at frame X+1, we access X+2 queue (after IFACE_APPLY_DELAY)
  */
  q_idx = ((*frame_id + session->max_reporting_delay +
    IFACE_APPLY_DELAY) % IFACE_FRAME_CTRL_SIZE);
  if (TRUE != iface_store_per_frame_metadata(session, &iface_metadata, q_idx)) {
    CDBG_ERROR("%s:%d failed: iface_store_per_frame_metadata\n", __func__,
       __LINE__);
  }

  pthread_mutex_lock(&session_thread->busy_mutex);
  if (session_thread->is_busy_wait == TRUE) {
    session_thread->is_busy_wait = FALSE;
    session_thread->applied_frame_id = *frame_id;
    sem_post(&session_thread->busy_sem);
  }
  pthread_mutex_unlock(&session_thread->busy_mutex);

  return 0;
}

/** iface_util_hw_notify_buf_drop
 *    @iface: top level iface pointer
 *    @drop_event:  frame drop event to down stream
 *    @axi_idx:     axi hw index
 *    @session_id:  sesion id
 *
 *   This function will post a module event to the downstream
 *   module for notify the frame drop
 *
 * Return: 0 - success, negative vale - error.
 **/
static int iface_util_hw_notify_buf_drop(
  iface_t             *iface,
  iface_axi_framedrop *axi_frame_drop,
  uint32_t             axi_idx __unused,
  uint32_t             session_id)
{
  uint32_t j = 0;
  int stream_cnt = 0, rc = 0;
  int k;
  uint32_t i;
  uint32_t mask;
  iface_session_t *session;
  iface_stream_t *user_stream = NULL;
  iface_port_t *iface_src_port;
  iface_hw_stream_t *hw_stream;
  mct_event_t drop_event;

  if (!axi_frame_drop) {
    return rc;
  }

  session = iface_util_get_session_by_id(iface, session_id);
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n",
      __func__, session_id);
    return -1;
  }

  /* no need for private data for drop event */
  memset(&drop_event, 0, sizeof(drop_event));
  drop_event.type = MCT_EVENT_MODULE_EVENT;
  drop_event.direction = MCT_EVENT_DOWNSTREAM;
  drop_event.u.module_event.type = MCT_EVENT_MODULE_FRAME_DROP_NOTIFY;
  /* Need to notify the actual frame number for which buffer was dropped
     This is current_sof - 1 */
  drop_event.u.module_event.current_frame_id = axi_frame_drop->frame_id;

  CDBG_HIGH("%s:%d: need to notify cpp to drop frame %d\n", __func__,
             __LINE__, drop_event.u.module_event.current_frame_id);

  for (i=0; i<axi_frame_drop->framedrop.num_streams
    && i < MAX_NUM_STREAMS; i++) {
    hw_stream = iface_util_find_hw_stream_in_session(
        session, axi_frame_drop->framedrop.stream_request[i].streamID);

    if (hw_stream == NULL) {
      continue;
    }

    if (!(hw_stream->state == IFACE_HW_STREAM_STATE_ACTIVE &&
        hw_stream->need_divert == 1))
      continue;

    CDBG("%s:  hw stream %x state %d, need divert %d",
      __func__, hw_stream->hw_stream_id, hw_stream->state,
      hw_stream->need_divert);

    for (j = 0; j < hw_stream->num_mct_stream_mapped
      && j < IFACE_MAX_STREAMS; j++) {
      user_stream = iface_util_find_stream_in_sink_port_list(
          iface, session_id, hw_stream->mapped_mct_stream_id[j]);

      if (user_stream == NULL) {
        CDBG_ERROR("%s: Error! Could not find user_stream for session %d\n",
                   __func__, session_id);
        continue;
      }

      if (!user_stream->src_port) {
        CDBG_ERROR("%s: no src port on this user stream id %d\n",
                   __func__, user_stream->stream_id);
        continue;
      }

      drop_event.identity = pack_identity(session_id, user_stream->stream_id);

      CDBG_HIGH("%s: Send buf drop to peer (0x%x, fid: 0x%x)",
        __func__, drop_event.identity,
        drop_event.u.module_event.current_frame_id);

      /* this module event should never be blocked since we are doing SOF next.
         Also, this event has no payload */
      mct_port_send_event_to_peer(user_stream->src_port, &drop_event);
    }
  }

  return rc;
}


/** iface_util_hw_notify_reg_update
 *
 * DESCRIPTION:
 *
 **/
static int iface_util_hw_notify_reg_update(iface_t *iface,
  mct_event_t *mct_event, uint32_t axi_idx, uint32_t session_id)
{
  int j, stream_cnt = 0, rc = 0;
  int k;
  uint32_t i;
  uint32_t mask;
  uint32_t *frame_id =
    (uint32_t *)mct_event->u.module_event.module_event_data;
  iface_session_t *session;
  iface_hw_stream_t *hw_stream;
  iface_stream_t *user_stream = NULL;
  boolean update_needed = FALSE;
  mct_port_t *available_src_port = NULL;

  session = iface_util_get_session_by_id(iface, session_id);
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n",
      __func__, session_id);
    return -1;
  }

  if (session->session_resource.num_isps > 1) {
    rc = iface_util_check_dual_vfe_notify(session,
      IFACE_AXI_NOTIFY_CAMIF_REG_UPDATE, axi_idx, *frame_id, &update_needed);
    if ((rc < 0) || (update_needed == FALSE)) {
      /* skip notify in dual_vfe case since both vfe event are not received */
      return 0;
    }
  }

  if (session->session_resource.offline_isp_id_mask & (1 << axi_idx)) {
      /* skip notify in offline ISP since its not needed */
      CDBG("%s: Offline ISP reg_update (mask %d, id %d)\n",
        __func__, session->session_resource.offline_isp_id_mask, axi_idx);
      return 0;
  }

  /* TODO:
   * This nested FOR loop is used to find some userstream in session to send event down.
   * This is used by reg_update, sof. So make utility and try to optimize as well.
   */
  for (i = 0; i < IFACE_MAX_STREAMS; i++) {
    hw_stream = &session->hw_streams[i];
    if (hw_stream == NULL || hw_stream->stream_info.dim.width == 0) {
      continue;
    }
    CDBG("%s: %d Found hw stream id = %x, num_mapped_mct_stream = %d\n",
      __func__, i, hw_stream->hw_stream_id, hw_stream->num_mct_stream_mapped);

    for (k = 0; k < IFACE_MAX_STREAMS; k++) {
       if (hw_stream->mapped_mct_stream_id[k] == 0)
          continue;

       user_stream = iface_util_find_stream_in_sink_port_list(iface,
         hw_stream->session_id, hw_stream->mapped_mct_stream_id[k]);
       if (user_stream == NULL) {
          CDBG_ERROR("%s: no userstream, stream id = %d, session id = %d\n",
            __func__, hw_stream->session_id,
            hw_stream->mapped_mct_stream_id[k]);
          continue;
       }
       if (!user_stream->src_port) {
         CDBG_ERROR("%s: no src port on this user stream id %d\n",
           __func__, user_stream->stream_id);
         continue;
       } else {
         available_src_port = user_stream->src_port;
       }

       break;
    }
  }
  if (user_stream) {
    mct_event->identity = pack_identity(session_id, user_stream->stream_id);
    /* broadcast reg_update downstream */
    mct_event->direction = MCT_EVENT_DOWNSTREAM;
    mct_port_send_event_to_peer(available_src_port, mct_event);
  } else {
    CDBG_ERROR("%s: Error! Could not find user_stream for session %d\n",
      __func__, session_id);
  }

  return rc;
}

/** iface_util_hw_notify_sof
 *
 * DESCRIPTION: This function handles the axi sof event. it
 * posts SOF bus message and any frame drop due to out of buffer
 * or error(missing reg update)
 *
 **/
static int iface_util_hw_notify_sof(
  iface_t             *iface,
  uint32_t             session_id,
  iface_axi_sof_event *axi_sof_event,
  void                *ctrl __unused,
  int                  axi_idx)
{
  int j, stream_cnt = 0, rc = 0;
  int k;
  uint32_t i;
  uint32_t mask;
  uint32_t diff_sof;
  mct_bus_msg_t *bus_msg = &axi_sof_event->sof_buf_msg;
  mct_bus_msg_isp_sof_t *sof_event = bus_msg->msg;

  iface_session_t *session;
  iface_hw_stream_t *hw_stream;
  iface_stream_t *user_stream;
  boolean post_sof = FALSE;
  enum msm_vfe_input_src frame_src = sof_event->frame_src;

  if (axi_idx >= VFE_MAX) {
    CDBG_ERROR("%s:%d failed: invalid axi_idx %d\n", __func__, __LINE__);
    return -EINVAL;
  }

  session = iface_util_get_session_by_id(iface, session_id);
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n",
               __func__, bus_msg->sessionid);
    return -1;
  }

  if ((frame_src != VFE_PIX_0) &&
    (session->session_resource.camif_cnt > 0)) {
    /* Use RDI SOF only if there is no PIX stream */
    return 0;
  }

  pthread_mutex_lock(&session->mutex);

  /* skip posted SOF after recovery*/
  if (session->overflow_recovery_state ==
      IFACE_AXI_RECOVERY_STATE_OVERFLOW_DETECTED) {
    if (sof_event->frame_id <= session->sof_frame_id) {
      IFACE_HIGH("Skip posted SOF %d during recovery\n", sof_event->frame_id);
      pthread_mutex_unlock(&session->mutex);
      return 0;
    }
    else {
      IFACE_HIGH("Recovery done at SOF %d\n", sof_event->frame_id);
      session->overflow_recovery_state = IFACE_AXI_RECOVERY_STATE_RECOVERY_DONE;
    }
  }

  /* Fill frame drop structure for dual camera  */
  diff_sof = sof_event->frame_id - (session->sof_frame_id + 1);
  if ((session->ms_type != MS_TYPE_NONE ) && (session->lpm_enable == 0)) {
    if ((session->cur_lpm_mode == session->prev_lpm_mode) &&
        (session->sof_frame_id + 1) < sof_event->frame_id) {
      for (i = 0; i < IFACE_MAX_STREAMS; i++) {
        hw_stream = &session->hw_streams[i];
        if (hw_stream == NULL ||
             hw_stream->stream_info.dim.width == 0) {
           continue;
        }
        axi_sof_event->dual_cam_sync_drop.framedrop.stream_request[
        axi_sof_event->dual_cam_sync_drop.framedrop.num_streams++].streamID =
            hw_stream->hw_stream_id;
      }
    }
  }

#ifndef VIDIOC_MSM_ISP_DUAL_SYNC_CFG_VER2
  if (!session->session_resource.ispif_split_info.is_split) {
    /* single VFE */
    if (session->sof_frame_id < sof_event->frame_id) {
      session->sof_frame_id = sof_event->frame_id;
      post_sof = TRUE;
    }
  } else {
    /* dual VFE */
    if (sof_event->frame_src == VFE_PIX_0) {
      if (sof_event->frame_id <= session->sof_frame_id) {
        post_sof = TRUE;
      }
    }
    if (sof_event->frame_id > session->sof_frame_id) {
      session->sof_frame_id = sof_event->frame_id;
    }
  }
  #else
  if (!session->session_resource.ispif_split_info.is_split ||
      (session->dual_isp_sync &&
       session->session_resource.ispif_split_info.is_split)) {
    /* single VFE or DUAL ISP SYNC */
    if (session->sof_frame_id < sof_event->frame_id) {
      session->sof_frame_id = sof_event->frame_id;
      post_sof = TRUE;
    }
  } else {
    /* dual VFE */
    if (sof_event->frame_src == VFE_PIX_0) {
      if (sof_event->frame_id <= session->sof_frame_id) {
        post_sof = TRUE;
      }
    }
    if (sof_event->frame_id > session->sof_frame_id) {
      session->sof_frame_id = sof_event->frame_id;
    }
  }
  #endif
  if (post_sof == TRUE && (session->overflow_recovery_state !=
      IFACE_AXI_RECOVERY_STATE_NO_OVERFLOW)) {
    IFACE_HIGH("Recovery completed, resume SOF post at %d",
                sof_event->frame_id);
    session->overflow_recovery_state = IFACE_AXI_RECOVERY_STATE_NO_OVERFLOW;
  }

  pthread_mutex_unlock(&session->mutex);

  /* process sof event */
  for (i = 0; i < IFACE_MAX_STREAMS; i++) {
    hw_stream = &session->hw_streams[i];
    if (hw_stream == NULL || hw_stream->stream_info.dim.width == 0) {
      continue;
    }
    CDBG("%s: %d Found hw stream id = %x, num_mapped_mct_stream = %d\n",
      __func__, i, hw_stream->hw_stream_id, hw_stream->num_mct_stream_mapped);

    for (k = 0; k < IFACE_MAX_STREAMS; k++) {
       if (hw_stream->mapped_mct_stream_id[k] == 0)
          continue;

       user_stream = iface_util_find_stream_in_sink_port_list(iface,
         hw_stream->session_id, hw_stream->mapped_mct_stream_id[k]);
       if (user_stream == NULL) {
          CDBG_ERROR("%s: no userstream, stream id = %d, session id = %d\n",
            __func__, hw_stream->session_id,
            hw_stream->mapped_mct_stream_id[k]);
          continue;
       }
       if (!user_stream->src_port) {
         CDBG_ERROR("%s: no src port on this user stream id %d\n",
           __func__, user_stream->stream_id);
         continue;
       }

       if (stream_cnt < MAX_STREAMS_NUM)
         sof_event->streamids[stream_cnt++] = user_stream->stream_id;
    }
  }

  sof_event->num_streams = stream_cnt;
  sof_event->skip_meta = iface->skip_meta;
  if ((sof_event->num_streams > 0) && (post_sof == TRUE)) {
    if ((session->ms_type == MS_TYPE_NONE ) &&
        (sof_event->frame_id - session->prev_sof_frame_id) != 1) {
      sof_event->frame_id--;
      for (i = 0; i < MAX_STREAMS_NUM; i++) {
        sof_event->prev_sent_streamids[i] = session->prev_sent_streamids[i];
        session->prev_sent_streamids[i] = 0;
      }
      session->prev_sent_streamids_cnt = 0;

      /* broadcast sof msg to all the port */
      /* based on latest information we need this for fast aec mode
      * and for legacy compatable */
      iface_util_broadcast_sof_msg_to_modules(iface,
       bus_msg->sessionid, sof_event->streamids[0], sof_event);
      if (session->fast_aec_mode != TRUE) {
        /* send out SOF after meta bus msg*/
        if (TRUE != mct_module_post_bus_msg(iface->module, bus_msg))
           CDBG_ERROR("%s: SOF to bus error\n", __func__);
      }
      sof_event->frame_id++;
    }
    session->prev_sof_frame_id = sof_event->frame_id;
    for (i = 0; i < MAX_STREAMS_NUM; i++) {
      sof_event->prev_sent_streamids[i] = session->prev_sent_streamids[i];
      session->prev_sent_streamids[i] = 0;
    }
    session->prev_sent_streamids_cnt = 0;

    /* process drop frame mask*/
    if (axi_sof_event->get_buf_fail_drop.framedrop.num_streams > 0 &&
        session->hal_version == CAM_HAL_V3) {
      axi_sof_event->get_buf_fail_drop.frame_id -= diff_sof;
      rc = iface_util_hw_notify_buf_drop(
          iface, &axi_sof_event->get_buf_fail_drop, axi_idx, session_id);
      if (rc != 0) {
        CDBG_ERROR("iface_util_hw_notify_buf_drop failed !!\n");
      }
      rc = iface_util_hw_notify_buf_drop_in_meta(
          iface, session_id, &axi_sof_event->get_buf_fail_drop, axi_idx);
    }

    CDBG("%s: session %x frame_id %d frame_src = %d\n", __func__, bus_msg->sessionid,
      sof_event->frame_id, sof_event->frame_src);
    /* broadcast sof msg to all the port */
    /* based on latest information we need this for fast aec mode
     * and for legacy compatable */
    iface_util_broadcast_sof_msg_to_modules(iface,
      bus_msg->sessionid, sof_event->streamids[0], sof_event);
    if (session->fast_aec_mode != TRUE) {
      /* send out SOF after meta bus msg*/
      if (TRUE != mct_module_post_bus_msg(iface->module, bus_msg))
        CDBG_ERROR("%s: SOF to bus error\n", __func__);
    }
    /* TBD process reg not updated mask */


    /* process error mask */
    if (axi_sof_event->reg_update_fail_drop.framedrop.num_streams > 0 &&
      session->hal_version == CAM_HAL_V3) {
      axi_sof_event->reg_update_fail_drop.frame_id -= diff_sof;
      rc = iface_util_hw_notify_buf_drop_in_meta(
          iface, session_id, &axi_sof_event->reg_update_fail_drop, axi_idx);
    }

    /* process axi updating mask */
    if (axi_sof_event->axi_updating_drop.framedrop.num_streams > 0 &&
      session->hal_version == CAM_HAL_V3) {
      axi_sof_event->axi_updating_drop.frame_id -= diff_sof;
      rc = iface_util_hw_notify_buf_drop(
           iface, &axi_sof_event->axi_updating_drop, axi_idx, session_id);
    }

    /* process dual cam mask */
    if (axi_sof_event->dual_cam_sync_drop.framedrop.num_streams > 0 &&
      session->hal_version == CAM_HAL_V3) {
      for (i = diff_sof; i > 0; --i) {
        axi_sof_event->dual_cam_sync_drop.frame_id = sof_event->frame_id - i;
        CDBG_ERROR("%s: dual cam drop %d",__func__,
            axi_sof_event->dual_cam_sync_drop.frame_id);
        rc = iface_util_hw_notify_buf_drop(
             iface, &axi_sof_event->dual_cam_sync_drop, axi_idx, session_id);
      }
    }

    /*update session parm in sof*/
    iface_update_thread_parm_in_sof(&session->session_thread);
  }

  return rc;
}

/** iface_util_find_isp_id
 *
 *  @vfe_id: vfe id
 *
 *  convert vfe id to isp id
 *
 *  return integer rc
 **/
static inline isp_hw_id_t iface_util_find_isp_id(uint32_t vfe_id)
{
  isp_hw_id_t isp_id = ISP_HW_MAX;
  switch (vfe_id) {
  case 0:
    isp_id = ISP_HW_0;
    break;
  case 1:
    isp_id = ISP_HW_1;
    break;
  default:
    CDBG_ERROR("failed: vfe id is out of range\n");
  }
  return isp_id;
}
/** iface_util_hw_notify_raw_stats
 *
 *  @iface: iface handle
 *  @raw_stats_info: stats notify data
 *
 *  notify ISP for raw stats buffer
 *
 *  return integer rc
 **/
int iface_util_hw_notify_raw_stats(iface_t *iface,
  iface_raw_stats_buf_info_t *raw_stats_info, uint32_t vfe_id)
{
  int rc = 0;
  int i = 0;
  int k = 0;
  void *raw_buf = NULL;
  mct_event_t mct_event;
  iface_session_t *session = NULL;
  iface_stream_t *user_stream = NULL;
  iface_hw_stream_t *hw_stream  = NULL;
  CDBG("%s: E\n", __func__);
  session =
    iface_util_get_session_by_id(iface, raw_stats_info->session_id);
  if (session == NULL) {
    CDBG_ERROR("%s: can not find session id = %d, rc = -1\n",
      __func__, raw_stats_info->session_id);
    return -1;
  }
  if (session->session_resource.offline_isp_id_mask & (1 << vfe_id)) {
    /*if the raw stats is from offline isp then use the offline stream*/
    for (i = 0; i < IFACE_MAX_STREAMS; i++) {
      hw_stream = &session->hw_streams[i];
      if (hw_stream->stream_info.cam_stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) {
        continue;
      }
      user_stream = iface_util_find_stream_in_sink_port_list(iface,
        hw_stream->session_id, hw_stream->mapped_mct_stream_id[k]);
      if (!user_stream || !user_stream->src_port) {
        CDBG_ERROR("%s failed: offline port not linked user stream %p\n",
                   __func__, user_stream);
        return -1;
      }
      /*if found user stream to send event, then break*/
      if (user_stream != NULL && user_stream->src_port)
        break;
    }
  } else {
    /*find one user stream to send out mct event to modules
      from existing hw stream to find one user stream*/
    for (i = 0; i < IFACE_MAX_STREAMS; i++) {
      hw_stream = &session->hw_streams[i];
      if (hw_stream->stream_info.dim.width == 0)
        continue;

      /*found HW stream, then find user stream from this hw stream*/
      for (k = 0; k < IFACE_MAX_STREAMS; k++) {
        if (hw_stream->mapped_mct_stream_id[k] == 0)
           continue;

        user_stream = iface_util_find_stream_in_sink_port_list(iface,
          hw_stream->session_id, hw_stream->mapped_mct_stream_id[k]);
        if (user_stream != NULL && user_stream->src_port) {
          break;
        }
      }
      /*if found user stream to send event, then break*/
      if (user_stream != NULL && user_stream->src_port)
        break;
    }
  }

  /*if can not find any user stream in session*/
  if (!user_stream || !user_stream->src_port) {
    CDBG_ERROR("%s: no user stream with src port in session %d, return 0\n",
      __func__, session->session_id);
    return 0;
  }
  raw_stats_info->hw_id = iface_util_find_isp_id(vfe_id);
  memset(&mct_event,  0,  sizeof(mct_event));
  mct_event.u.module_event.type = MCT_EVENT_MODULE_RAW_STATS_DIVERT;
  mct_event.u.module_event.module_event_data = (void *)raw_stats_info;
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.identity = pack_identity(session->session_id,
    user_stream->stream_id);
  mct_event.direction = MCT_EVENT_DOWNSTREAM;

  if (FALSE == mct_port_send_event_to_peer(
    user_stream->src_port, &mct_event)) {
    CDBG_ERROR("%s: buf_divert to pp, sessid %d, streamid %d, rc = -1\n",
      __func__, user_stream->session_id, user_stream->stream_id);
    /* in case of error, caller takes care of queing back the buffers */
    return -1;
  }
  CDBG("raw_stats_info frame_id %d, buf_idx %d pd_buffer_data %p pd_stats_read_data_length %d",
    raw_stats_info->frame_id, raw_stats_info->pd_data.buf_idx,
    raw_stats_info->pd_data.pd_buffer_data,
    raw_stats_info->pd_data.pd_stats_read_data_length);
  return rc;
}


/** iface_util_process_fetch_done
 *
 *  @iface: iface handle
 *  @fd: fetch done buf fd
 *
 *  unmap buffer after fetch done
 *
 *  return integer rc
 **/
int iface_util_process_fetch_done(iface_t *iface,
  uint32_t *fd, int vfe_id)
{
  uint32_t buf_fd;
  int rc = 0;
  if (!fd) {
    CDBG_ERROR("%s:NULL pointer fd %p\n", __func__, fd);
    return -1;
  }
  buf_fd = *fd;
  rc = iface->isp_axi_data.axi_data[vfe_id].axi_hw_ops->action(
    iface->isp_axi_data.axi_data[vfe_id].axi_hw_ops->ctrl,
    IFACE_AXI_ACTION_CODE_UNMAP_BUF, (void *)&buf_fd, sizeof(uint32_t));
  if (rc < 0) {
    CDBG_ERROR("%s: VFE%d IFACE_AXI_UNMAP_BUF failed! rc = %d\n"
      , __func__, vfe_id, rc);
  }
  return rc;
}

/** iface_util_axi_notify
 *    @parent: ISP pointer
 *    @handle: ISP driver index
 *    @type: notify type
 *    @notify_data: notify payload pointer
 *    @notify_data_size: notify payload data size
 *
 * Process ISP driver's notify event.
 *
 * Return: 0 - success, negative vale - error.
 **/
int iface_util_axi_notify (void *parent,  uint32_t handle, uint32_t session_id,
  uint32_t type, void *notify_data, uint32_t notify_data_size)
{
  int rc = 0;
  iface_t *iface = parent;
  int axi_idx = (int)handle;
  iface_axi_t *axi = &iface->isp_axi_data.axi_data[axi_idx];


  CDBG("%s: E, type %d", __func__, type);
  switch (type) {
  case IFACE_AXI_NOTIFY_CAMIF_SOF: {
    if (notify_data_size == sizeof(iface_axi_sof_event))
      rc = iface_util_hw_notify_sof(
          iface, session_id,
          (iface_axi_sof_event *)notify_data, axi->axi_hw_ops->ctrl, axi_idx);
    else
      CDBG_ERROR("%s fatal error regarding the sof event from axi hw",
                 __func__);
  }
  break;
  case IFACE_AXI_NOTIFY_STREAM_UPDATE_DONE: {
    rc = iface_util_notify_update_stream_done(iface, session_id, axi_idx,
      notify_data, notify_data_size);
  }
    break;

  case IFACE_AXI_NOTIFY_CAMIF_REG_UPDATE: {
    rc = iface_util_hw_notify_reg_update(iface, notify_data, axi_idx,
      session_id);
  }
    break;

  case IFACE_AXI_NOTIFY_BUF_DIVERT: {
    rc = iface_util_buf_divert_notify(iface,
      (iface_frame_divert_notify_t *)notify_data);
  }
    break;

  case IFACE_AXI_NOTIFY_STATS: {
    rc = iface_util_hw_notify_raw_stats(iface,
      (iface_raw_stats_buf_info_t *)notify_data, axi_idx);
  }
    break;

  case IFACE_AXI_NOTIFY_BUS_OVERFLOW: {
    rc = iface_util_process_overflow_recovery(iface,
     (iface_halt_recovery_info_t *)notify_data, axi_idx);
  }
    break;

  case IFACE_AXI_NOTIFY_PING_PONG_MISMATCH:
  case IFACE_AXI_NOTIFY_REG_UPDATE_MISSING:
  case IFACE_AXI_NOTIFY_PAGE_FAULT:
  case IFACE_AXI_NOTIFY_BUF_FATAL_ERROR:
  {
    rc = iface_util_process_hw_error(iface, (mct_bus_msg_t*)notify_data);
  }
    break;

  case IFACE_AXI_NOTIFY_FE_READ_DONE: {
    rc = iface_util_process_fe_read_done(iface, (mct_bus_msg_t*)notify_data);
  }
    break;

  case IFACE_AXI_NOTIFY_OUTPUT_ERROR: {
    rc = iface_util_hw_notify_buf_drop_in_meta(iface,session_id,
     (iface_axi_framedrop *)notify_data, axi_idx);
  }
    break;

  case IFACE_AXI_NOTIFY_UNMAP_BUF: {
    rc = iface_util_process_fetch_done(iface,
      (uint32_t *)notify_data, axi_idx);
  }
    break;

  default: {
    CDBG_HIGH("%s: type %d not supported", __func__, type);
  }
    break;
  }
  CDBG("%s: X, rc = %d", __func__, rc);
  return rc;
}

/** iface_util_fmt_to_pix_pattern
 *    @fmt: HAL camera format
 *
 * translate HAL format to pix pattern
 *
 * Return: ISP pix pattern
 **/
enum ISP_START_PIXEL_PATTERN iface_util_fmt_to_pix_pattern(cam_format_t fmt)
{
  CDBG_HIGH("%s: format %d", __func__, fmt);

  switch (fmt) {
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
  case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_RGGB:
  case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_RGGB:
  /* Grey formats need to be set as any one of the below types
   * because enum maps to CORE_CFG register, PIX_PATTERN fields.
   * So we cannot add new enum value for Grey. Arbitarily pick
   * RGRG */
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GREY:
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GREY:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GREY:
    return ISP_BAYER_RGRGRG;

  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
  case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_GRBG:
  case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GRBG:
    return ISP_BAYER_GRGRGR;

  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
  case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_BGGR:
  case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_BGGR:
    return ISP_BAYER_BGBGBG;

  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
  case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_GBRG:
  case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GBRG:
    return ISP_BAYER_GBGBGB;

  case CAM_FORMAT_YUV_RAW_8BIT_YUYV:
  case CAM_FORMAT_YUV_422_NV16:
    return ISP_YUV_YCbYCr;

  case CAM_FORMAT_YUV_RAW_8BIT_YVYU:
  case CAM_FORMAT_YUV_422_NV61:
    return ISP_YUV_YCrYCb;

  case CAM_FORMAT_YUV_RAW_8BIT_UYVY:
    return ISP_YUV_CbYCrY;

  case CAM_FORMAT_YUV_RAW_8BIT_VYUY:
    return ISP_YUV_CrYCbY;

  default:
    CDBG_ERROR("%s: invalid fmt=%d", __func__, fmt);
    return ISP_PIX_PATTERN_MAX;
  }
  return ISP_PIX_PATTERN_MAX;
}

/** iface_util_dump_fe_cfg:
 *
 *    @input_cfg: kernel cfg cmd
 *
 *    Logs fe config
 **/
  static void iface_util_dump_fe_cfg(struct msm_vfe_input_cfg *input_cfg)
  {
    struct msm_vfe_pix_cfg *pix_cfg = NULL;

    if (input_cfg == NULL)
      return;

    pix_cfg = &input_cfg->d.pix_cfg;

    CDBG_HIGH("%s: =====Fetch Engine DUMP cfg for PIX interface====\n",
      __func__);
    CDBG_HIGH("%s: fe input format= %d(SRGGB10=%d), op_clk = %d\n",
      __func__,
      pix_cfg->fetch_engine_cfg.input_format, V4L2_PIX_FMT_SRGGB10,
      input_cfg->input_pix_clk);
    CDBG_HIGH("%s: fe wd x ht = %d x %d\n",
      __func__, pix_cfg->fetch_engine_cfg.fetch_width,
      pix_cfg->fetch_engine_cfg.fetch_height);
    CDBG_HIGH("%s: fe buf x offset %d y offset = %d\n",
      __func__, pix_cfg->fetch_engine_cfg.x_offset,
      pix_cfg->fetch_engine_cfg.y_offset);
    CDBG_HIGH("%s: fe buf wd x ht %d x %d\n", __func__,
      pix_cfg->fetch_engine_cfg.buf_width,
      pix_cfg->fetch_engine_cfg.buf_height);
  }

/** iface_util_dump_camif_cfg:
 *
 *    @pix_cfg: kernel cfg cmd
 *
 *    Logs pix config
 **/
static void iface_util_dump_camif_cfg(struct msm_vfe_input_cfg *input_cfg)
{
  struct msm_vfe_pix_cfg *pix_cfg = NULL;

  if (input_cfg == NULL)
    return;

  pix_cfg = &input_cfg->d.pix_cfg;
  IFACE_HIGH("=====Camif DUMP cfg for PIX interface====\n");
  IFACE_HIGH("camif input type = %d(MIPI=3), op_pix_clk = %d\n",
    pix_cfg->camif_cfg.camif_input, input_cfg->input_pix_clk);
  IFACE_HIGH("camif pix_pattern(RGRG-0/GRGR-1/BGBG-2/GBGB-3) = %d\n",
    pix_cfg->pixel_pattern);
  IFACE_HIGH("camif first_pix = %d, last_pix = %d\n",
    pix_cfg->camif_cfg.first_pixel, pix_cfg->camif_cfg.last_pixel);
  IFACE_HIGH("camif first_line = %d, last_line = %d\n",
    pix_cfg->camif_cfg.first_line, pix_cfg->camif_cfg.last_line);
  IFACE_HIGH("camif pixels_per_line = %d, lines_per_frame = %d\n",
    pix_cfg->camif_cfg.pixels_per_line, pix_cfg->camif_cfg.lines_per_frame);
  IFACE_HIGH("camif irq subsample pattern = %x, period = %d sof_step %d\n",
    pix_cfg->camif_cfg.subsample_cfg.irq_subsample_pattern,
    pix_cfg->camif_cfg.subsample_cfg.irq_subsample_period,
    pix_cfg->camif_cfg.subsample_cfg.sof_counter_step);
}

/** iface_util_input_cfg:
 *
 *    @fd: vfe fd
 *    @dim: fetch engine input dimensions
 *    @op_pixel_clk: vfe pixel clk
 *    @fmt: data packing fmt
 *
 *  This function prepares fetch engine input dimensions and clk.
 *  Now, when buffer is available, call "iface_axi_start_fetch" passing
 *  the buffer information.
 *  Returns 0 for success and negative error on failure
 **/
int iface_util_fe_cfg(iface_session_t *session, int fd, cam_dimension_t dim,
  uint32_t op_pixel_clk, cam_format_t fmt, isp_stripe_id_t stripe_id)
{
  int rc = 0;
  struct msm_vfe_input_cfg input_cfg;
  memset(&input_cfg, 0, sizeof(input_cfg));
  CDBG("%s: fetch engine config %d\n", __func__, fd);

  input_cfg.d.pix_cfg.input_mux = EXTERNAL_READ;
  input_cfg.d.pix_cfg.pixel_pattern =
    iface_util_fmt_to_pix_pattern(fmt);
  if (!session->session_resource.offline_split_info.is_split) {
      input_cfg.d.pix_cfg.fetch_engine_cfg.buf_width    =
        dim.width;
      input_cfg.d.pix_cfg.fetch_engine_cfg.buf_height   =
        dim.height;
      input_cfg.d.pix_cfg.fetch_engine_cfg.fetch_width  =
        dim.width;
      input_cfg.d.pix_cfg.fetch_engine_cfg.fetch_height =
        dim.height;
      input_cfg.d.pix_cfg.fetch_engine_cfg.x_offset     = 0;
      input_cfg.d.pix_cfg.fetch_engine_cfg.y_offset     = 0;
      input_cfg.d.pix_cfg.fetch_engine_cfg.buf_stride   =
        dim.width;
  } else if (stripe_id == ISP_STRIPE_LEFT) {
    input_cfg.d.pix_cfg.fetch_engine_cfg.buf_width =
      session->session_resource.offline_split_info.right_stripe_offset +
      session->session_resource.offline_split_info.overlap;
    input_cfg.d.pix_cfg.fetch_engine_cfg.buf_height   =
      dim.height;
    input_cfg.d.pix_cfg.fetch_engine_cfg.fetch_width  =
      dim.width;
    input_cfg.d.pix_cfg.fetch_engine_cfg.fetch_height =
      dim.height;
    input_cfg.d.pix_cfg.fetch_engine_cfg.x_offset     = 0;
    input_cfg.d.pix_cfg.fetch_engine_cfg.y_offset     = 0;
    input_cfg.d.pix_cfg.fetch_engine_cfg.buf_stride   =
      dim.width;
  } else {
    input_cfg.d.pix_cfg.fetch_engine_cfg.buf_width    =
      dim.width -
      session->session_resource.offline_split_info.
      right_stripe_offset;
    input_cfg.d.pix_cfg.fetch_engine_cfg.buf_height   =
      dim.height;
    input_cfg.d.pix_cfg.fetch_engine_cfg.fetch_width  =
      dim.width;
    input_cfg.d.pix_cfg.fetch_engine_cfg.fetch_height =
      dim.height;
    input_cfg.d.pix_cfg.fetch_engine_cfg.x_offset     = 0;
    input_cfg.d.pix_cfg.fetch_engine_cfg.y_offset     = 0;
    input_cfg.d.pix_cfg.fetch_engine_cfg.buf_stride   =
      dim.width;
  }

  input_cfg.input_pix_clk = op_pixel_clk;
  input_cfg.input_src = VFE_PIX_0;
  input_cfg.d.pix_cfg.fetch_engine_cfg.input_format =
  input_cfg.d.pix_cfg.input_format = iface_axi_util_cam_fmt_to_v4l2_fmt(
    fmt, 0);

  input_cfg.d.pix_cfg.is_split = FALSE;
  iface_util_dump_fe_cfg(&input_cfg);
  rc = ioctl(fd, VIDIOC_MSM_ISP_INPUT_CFG, &input_cfg);
  if (rc < 0) {
    CDBG_ERROR("%s: input cfg error = %d\n", __func__, rc);
  }
  return rc;
}

/** iface_util_camif_cfg:
 *
 *    @iface: iface handle
 *    @session: session data
 *    @iface_sink_port: sink port private
 *    @isp_id: isp id
 *
 *  Returns 0 for success and negative error on failure
 **/
static int iface_util_camif_cfg(
  iface_t                *iface __unused,
  iface_session_t        *session,
  iface_sink_port_t      *sink_port,
  struct msm_vfe_pix_cfg *pix_cfg,
  uint8_t                 primary_cid_idx,
  uint32_t                isp_id)
{
  int i = 0, offset;
  char                    value[PROPERTY_VALUE_MAX];
  uint32_t                epoch2_offset, last_line;

  pix_cfg->pixel_pattern = iface_util_fmt_to_pix_pattern(
    sink_port->sensor_cap.sensor_cid_ch[primary_cid_idx].fmt);

  pix_cfg->input_format = iface_axi_util_cam_fmt_to_v4l2_fmt(
    sink_port->sensor_cap.sensor_cid_ch[primary_cid_idx].fmt, 0);

  /*camif config*/
  pix_cfg->camif_cfg.camif_input = CAMIF_MIPI_INPUT;
  /*Default we fix epoch2 at 50th Line from last line */
  property_get("persist.vendor.camera.isp.epoch2.offset", value, "50");
  offset = atoi(value);
  epoch2_offset = offset < 0 ? 0 - offset : offset;
  if (session->batch_info.camif_period > 0) {
    pix_cfg->camif_cfg.subsample_cfg.irq_subsample_pattern =
      1 << (session->batch_info.camif_period - 1);
    pix_cfg->camif_cfg.subsample_cfg.irq_subsample_period =
      session->batch_info.camif_period;
    /* 3A relies on consecutive SOF ids hence keeping step 1 */
    pix_cfg->camif_cfg.subsample_cfg.sof_counter_step = 1;
  } else {
    pix_cfg->camif_cfg.subsample_cfg.irq_subsample_pattern = 0;
    pix_cfg->camif_cfg.subsample_cfg.irq_subsample_period = 0;
    pix_cfg->camif_cfg.subsample_cfg.sof_counter_step = 1;
  }
  if (session->session_resource.num_isps == 1) {
    CDBG_HIGH("%s: <dual_dbg> single vfe Camif\n", __func__);
    pix_cfg->camif_cfg.is_split = 0;
    pix_cfg->camif_cfg.first_line =
      sink_port->sensor_out_info.request_crop.first_line;
    pix_cfg->camif_cfg.last_line =
      sink_port->sensor_out_info.request_crop.last_line;
    pix_cfg->camif_cfg.first_pixel =
      sink_port->sensor_out_info.request_crop.first_pixel;
    pix_cfg->camif_cfg.last_pixel =
      sink_port->sensor_out_info.request_crop.last_pixel;
    pix_cfg->camif_cfg.lines_per_frame =
      sink_port->sensor_out_info.dim_output.height;
    pix_cfg->camif_cfg.pixels_per_line =
       sink_port->sensor_out_info.dim_output.width;
      for (i = 0; i < IFACE_MAX_STREAMS; i++) {
        if (session->hw_streams[i].axi_path == CAMIF_RAW){
          pix_cfg->camif_cfg.subsample_cfg.pixel_skip =
            session->hw_streams[i].left_pixel_hor_skip;
          pix_cfg->camif_cfg.subsample_cfg.line_skip =
            session->hw_streams[i].left_pixel_ver_skip;
          /* Userspace is responsible to know pipeline supports camif op fmt feature */
          CDBG_HIGH("%s: supported %d fmt %d\n", __func__,
            session->camif_cap.is_camif_raw_op_fmt_supported,
            session->hw_streams[i].camif_output_format);
          if (session->camif_cap.is_camif_raw_op_fmt_supported) {
            pix_cfg->camif_cfg.subsample_cfg.output_format =
              session->hw_streams[i].camif_output_format;
          } else {
            pix_cfg->camif_cfg.subsample_cfg.output_format = CAMIF_QCOM_RAW;
          }
        }
      }
  } else {
    CDBG_HIGH("%s: <dual_dbg> dual vfe Camif\n", __func__);
    pix_cfg->camif_cfg.is_split = 1;
    if (isp_id == VFE0) {
      pix_cfg->camif_cfg.first_line =
        sink_port->sensor_out_info.request_crop.first_line;
      pix_cfg->camif_cfg.last_line =
        sink_port->sensor_out_info.request_crop.last_line;
       /* left VFE need to start from the sensor request first pix*/
      pix_cfg->camif_cfg.first_pixel =
        sink_port->sensor_out_info.request_crop.first_pixel;
      /* right stripe offset already consider camif crop first pix*/
      pix_cfg->camif_cfg.last_pixel =
        session->session_resource.ispif_split_info.right_stripe_offset +
        session->session_resource.ispif_split_info.overlap - 1;
      pix_cfg->camif_cfg.lines_per_frame =
        sink_port->sensor_out_info.dim_output.height;
      pix_cfg->camif_cfg.pixels_per_line =
        session->session_resource.ispif_split_info.right_stripe_offset +
        session->session_resource.ispif_split_info.overlap;
      for (i = 0; i < IFACE_MAX_STREAMS; i++) {
        if (session->hw_streams[i].axi_path == CAMIF_RAW) {
          pix_cfg->camif_cfg.subsample_cfg.pixel_skip =
            session->hw_streams[i].left_pixel_hor_skip;
          pix_cfg->camif_cfg.subsample_cfg.line_skip =
            session->hw_streams[i].left_pixel_ver_skip;
          /* Userspace is responsible to know pipeline supports camif op fmt feature */
          if (session->camif_cap.is_camif_raw_op_fmt_supported) {
            pix_cfg->camif_cfg.subsample_cfg.output_format =
              session->hw_streams[i].camif_output_format;
          } else {
            pix_cfg->camif_cfg.subsample_cfg.output_format = CAMIF_QCOM_RAW;
          }
          CDBG_HIGH("%s %d pdaf: hor_skip = %x ver_skip %x isp_id %d",
            __func__,__LINE__, pix_cfg->camif_cfg.subsample_cfg.pixel_skip,
            pix_cfg->camif_cfg.subsample_cfg.line_skip);
       }
      }
    } else {
      pix_cfg->camif_cfg.first_line =
        sink_port->sensor_out_info.request_crop.first_line;
      pix_cfg->camif_cfg.last_line =
        sink_port->sensor_out_info.request_crop.last_line;
      /* right VFE doesnt need to consider camif crop first pix anymore*/
      pix_cfg->camif_cfg.first_pixel = 0;
      /* last camif pix: from right offset to sensor request last pix*/
      pix_cfg->camif_cfg.last_pixel =
        sink_port->sensor_out_info.request_crop.last_pixel -
        session->session_resource.ispif_split_info.right_stripe_offset;
      pix_cfg->camif_cfg.lines_per_frame =
        sink_port->sensor_out_info.dim_output.height;
      pix_cfg->camif_cfg.pixels_per_line =
        sink_port->sensor_out_info.dim_output.width -
        session->session_resource.ispif_split_info.right_stripe_offset;
      for (i = 0; i < IFACE_MAX_STREAMS; i++) {
        if (session->hw_streams[i].axi_path == CAMIF_RAW){
          pix_cfg->camif_cfg.subsample_cfg.pixel_skip =
            session->hw_streams[i].right_pixel_hor_skip;
          pix_cfg->camif_cfg.subsample_cfg.line_skip =
           session->hw_streams[i].right_pixel_ver_skip;
          /* Userspace is responsible to know pipeline supports camif op fmt feature */
          if (session->camif_cap.is_camif_raw_op_fmt_supported) {
            pix_cfg->camif_cfg.subsample_cfg.output_format =
              session->hw_streams[i].camif_output_format;
          } else {
            pix_cfg->camif_cfg.subsample_cfg.output_format = CAMIF_QCOM_RAW;
          }
          CDBG_HIGH("%s %d pdaf: hor_skip = %x ver_skip %x ",__func__,
            __LINE__, pix_cfg->camif_cfg.subsample_cfg.pixel_skip,
            pix_cfg->camif_cfg.subsample_cfg.line_skip);
        }
      }

    }
  }
  last_line = pix_cfg->camif_cfg.last_line;
  if (epoch2_offset >= last_line) {
    pix_cfg->camif_cfg.epoch_line1 = last_line;
  } else {
    pix_cfg->camif_cfg.epoch_line1 = last_line - epoch2_offset;
  }
  CDBG_HIGH("%s:%d: epoch_line1: %d \n",
    __func__, __LINE__, pix_cfg->camif_cfg.epoch_line1);

  return 0;
}


/** iface_util_testgen_cfg:
 *
 *    @iface: iface handle
 *    @session: session data
 *    @iface_sink_port: sink port private
 *    @isp_id: isp id
 *
 *  Returns 0 for success and negative error on failure
 **/
static int iface_util_testgen_cfg(
  iface_t                    *iface,
  iface_session_t            *session,
  iface_sink_port_t          *sink_port,
  struct msm_vfe_testgen_cfg *testgen_cfg,
  uint8_t                     primary_cid_idx,
  uint32_t                    isp_id __unused)
{
  int pixel_per_line = 0;
  if (!iface || !session || !sink_port || !testgen_cfg) {
    CDBG_ERROR("%s: NULL pointer!  %p %p %p %p rc = -1\n",
      __func__, iface, session, sink_port, testgen_cfg);
    return -1;
  }

  testgen_cfg->pixel_bayer_pattern = iface_util_fmt_to_pix_pattern(
    sink_port->sensor_cap.sensor_cid_ch[primary_cid_idx].fmt);

  if (session->session_resource.num_isps == 1) {
    CDBG_HIGH("%s: <dual_dbg> single vfe Camif\n", __func__);
    testgen_cfg->pixels_per_line =
      sink_port->sensor_out_info.dim_output.width;
    testgen_cfg->lines_per_frame =
      sink_port->sensor_out_info.dim_output.height;
    } else {
      CDBG_HIGH("%s: <dual_dbg> dual vfe Camif\n", __func__);
      /*Setting to max pixel_per_line for both VFEs */
      if ((sink_port->sensor_out_info.dim_output.width -
        session->session_resource.ispif_split_info.right_stripe_offset) >
        (session->session_resource.ispif_split_info.right_stripe_offset +
        session->session_resource.ispif_split_info.overlap)) {

        pixel_per_line = sink_port->sensor_out_info.dim_output.width -
        session->session_resource.ispif_split_info.right_stripe_offset;
      } else {
        pixel_per_line = session->session_resource.ispif_split_info.right_stripe_offset +
          session->session_resource.ispif_split_info.overlap;
      }
    }
    if (isp_id == VFE0) {
      testgen_cfg->lines_per_frame =
        sink_port->sensor_out_info.dim_output.height;
      testgen_cfg->pixels_per_line = pixel_per_line;
    } else {
      testgen_cfg->lines_per_frame =
        sink_port->sensor_out_info.dim_output.height;
      testgen_cfg->pixels_per_line = pixel_per_line;
    }
  testgen_cfg->v_blank = testgen_cfg->pixels_per_line * TESTGEN_HBI;
  testgen_cfg->h_blank = testgen_cfg->lines_per_frame * TESTGEN_VBI;

  testgen_cfg->rotate_period = 5;
  testgen_cfg->color_bar_pattern = COLOR_BAR_8_COLOR;

  /*0: continuous*/
  testgen_cfg->burst_num_frame = 0;

  return 0;
}

/** iface_util_input_cfg:
 *
 *    @iface: iface handle
 *    @session: session data
 *    @iface_sink_port: sink port private
 *    @isp_id: isp id
 *
 *  Returns 0 for success and negative error on failure
 **/
int iface_util_io_cfg(iface_t *iface, iface_session_t *session,
  iface_port_t *iface_sink_port, uint32_t isp_id, uint32_t *vfe_clk)
{
  int                     rc = 0;
  uint32_t                i = 0;
  struct                  msm_vfe_input_cfg input_cfg;
  iface_sink_port_t       *sink_port = NULL;
  uint32_t                intf_mask = 0;
  uint8_t                 primary_cid_idx = 0;
  struct msm_vfe_pix_cfg *pix_cfg = NULL;
  iface_hw_stream_t      *hw_stream = NULL;
  int32_t                 testgen_enable = 0;
  char                    value[PROPERTY_VALUE_MAX];
  int                     clk_opt_enable = 0;

  if (!session || !iface || !iface_sink_port || !vfe_clk) {
    CDBG_ERROR("%s: null pointer: %p %p %p %p, rc = -1",
      __func__, session, iface, iface_sink_port, vfe_clk);
    return -1;
  }

  memset(&input_cfg, 0, sizeof(input_cfg));

  sink_port = &iface_sink_port->u.sink_port;
  primary_cid_idx = iface_ispif_util_find_primary_cid(
     &sink_port->sensor_out_info, &sink_port->sensor_cap);
  if (primary_cid_idx >= SENSOR_CID_CH_MAX) {
    CDBG_ERROR("%s: invalid CID idx %d\n", __func__, primary_cid_idx);
    return -1;
  }

  /*find out the interface mask in this isp*/
  intf_mask =
    session->session_resource.used_resource_mask >> (16 * isp_id);

  /*config pixel interface: Camif/testgen*/
  if (((1 << IFACE_INTF_PIX) & intf_mask) && !session->bayer_processing) {
    pix_cfg = &input_cfg.d.pix_cfg;
    memset(pix_cfg, 0, sizeof(struct msm_vfe_pix_cfg));
    /*testgen also need to config camif*/
    input_cfg.input_src = VFE_PIX_0;
    pix_cfg->input_mux = CAMIF;

  if (session->hvx.enabled == TRUE) {
    if (session->hvx.hvx_info.is_pix_intf_needed == HVX_TRUE) {
      pix_cfg->hvx_cmd = HVX_ROUND_TRIP;
    } else {
      pix_cfg->hvx_cmd = HVX_ONE_WAY;
    }
  }

  rc = iface_util_camif_cfg(iface, session, sink_port,
      pix_cfg, primary_cid_idx, isp_id);
    if (rc < 0) {
      CDBG_ERROR("%s: iface_util_camif_cfg failed, rc = %d\n", __func__, rc);
      return rc;
    }

    if (session->use_testgen || session->sensor_out_info.is_dummy) {
      pix_cfg->input_mux = TESTGEN;
      rc = iface_util_testgen_cfg(iface, session, sink_port,
        &pix_cfg->testgen_cfg, primary_cid_idx, isp_id);
      if (rc < 0) {
        CDBG_ERROR("%s: iface_util_testgen_cfg failed, rc = %d\n", __func__, rc);
        return rc;
      }
    }
    property_get("persist.vendor.camera.isp.clock.optmz", value, "1");
    clk_opt_enable = atoi(value);
    if (clk_opt_enable) {
      input_cfg.input_pix_clk =
        iface->isp_axi_data.axi_data[isp_id].
        interface_clk_request[IFACE_INTF_PIX];
    } else {
       /*use sensor request clk before get proper clk calculation*/
        if (session->session_resource.num_isps == 1) {
          input_cfg.input_pix_clk =
          sink_port->sensor_out_info.op_pixel_clk;
        } else {
          if (session->use_testgen) {
            input_cfg.input_pix_clk =
              sink_port->sensor_out_info.op_pixel_clk *
              ((float)input_cfg.d.pix_cfg.testgen_cfg.pixels_per_line /
              (float)sink_port->sensor_out_info.dim_output.width);
          } else {
            input_cfg.input_pix_clk =
              sink_port->sensor_out_info.op_pixel_clk *
              ((float)input_cfg.d.pix_cfg.camif_cfg.pixels_per_line /
              (float)sink_port->sensor_out_info.dim_output.width);
          }
        }
    }

    CDBG("%s %d <pdaf_dbg>: hor_skip = %x ver_skip %x ",__func__, __LINE__,
      input_cfg.d.pix_cfg.camif_cfg.subsample_cfg.pixel_skip,
      input_cfg.d.pix_cfg.camif_cfg.subsample_cfg.line_skip);

    if (session->session_resource.num_isps == 1)
      pix_cfg->is_split = FALSE;
    else if (session->session_resource.num_isps == 2)
      pix_cfg->is_split = TRUE;

#ifdef VIDIOC_MSM_ISP_DUAL_SYNC_CFG_VER2
  {
    struct msm_vfe_dual_vfe_sync_mode mode;
    property_get("persist.vendor.camera.dual.isp.sync", value, "1");
    session->dual_isp_sync = (pix_cfg->is_split & atoi(value));
    mode.enable = session->dual_isp_sync;
    IFACE_HIGH("dual_isp_sync enable = %d\n", mode.enable);
    if (mode.enable) {
      rc = ioctl(iface->isp_axi_data.axi_data[isp_id].fd,
        VIDIOC_MSM_ISP_DUAL_SYNC_CFG_VER2, &mode);
      if (rc < 0) {
        CDBG_ERROR("VIDIOC_MSM_ISP_DUAL_SYNC_CFG_VER2 err, rc = %d\n", rc);
        return rc;
      }
    }
  }
#endif

#ifdef VIDIOC_MSM_ISP_NANOSEC_TIMESTAMP
  {
    struct msm_vfe_nano_sec_timestamp mode;
    property_get("persist.vendor.camera.nanosec.timestamp", value, "0");
    session->nanosec_ts_enable = atoi(value);
    mode.enable = session->nanosec_ts_enable;
    CDBG_HIGH("isp nanosec time enable = %d\n", session->nanosec_ts_enable);
    rc = ioctl(iface->isp_axi_data.axi_data[isp_id].fd,
      VIDIOC_MSM_ISP_NANOSEC_TIMESTAMP, &mode);
    if (rc < 0) {
      CDBG_ERROR("VIDIOC_MSM_ISP_NANOSEC_TIMESTAMP err, rc = %d\n", rc);
      return rc;
    }
  }
#endif

    iface_util_dump_camif_cfg(&input_cfg);
    rc = ioctl(iface->isp_axi_data.axi_data[isp_id].fd,
      VIDIOC_MSM_ISP_INPUT_CFG, &input_cfg);
    if (rc < 0) {
      CDBG_ERROR("%s: input cfg error, rc = %d\n", __func__, rc);
      return rc;
    }
    *vfe_clk = input_cfg.input_pix_clk;
  }
  /*for RDI need to link csid to WM/XBAR*/
  for (i = 0; i < IFACE_MAX_STREAMS; i++) {
    hw_stream = &session->hw_streams[i];
    intf_mask = hw_stream->interface_mask >> (16 * isp_id);

    if (intf_mask & (1 << IFACE_INTF_PIX))
      continue;
    else if (intf_mask & (1 << IFACE_INTF_RDI0))
      input_cfg.input_src = VFE_RAW_0;
    else if (intf_mask & (1 << IFACE_INTF_RDI1))
      input_cfg.input_src = VFE_RAW_1;
    else if (intf_mask & (1 << IFACE_INTF_RDI2))
      input_cfg.input_src = VFE_RAW_2;
    else {
      continue;
    }

    if(input_cfg.input_src >= VFE_SRC_MAX) {
      CDBG_ERROR("%s:%d input_src out of range :%d, rc = -1\n",
          __func__, __LINE__, input_cfg.input_src);
      return -1;
    }

    input_cfg.d.rdi_cfg.cid = hw_stream->cids[0];
    input_cfg.d.rdi_cfg.frame_based = 1;

    if (input_cfg.input_pix_clk == 0) {
      input_cfg.input_pix_clk = sink_port->sensor_out_info.op_pixel_clk;
      CDBG("%s, set proper pix clk for RDI only stream(s) to: %d\n",
          __func__, input_cfg.input_pix_clk);
    }
    rc = ioctl(iface->isp_axi_data.axi_data[isp_id].fd,
      VIDIOC_MSM_ISP_INPUT_CFG, &input_cfg);
    if (rc < 0) {
      CDBG_ERROR("%s: input RDI cfg error, rc = %d\n", __func__, rc);
      return rc;
    }
  }

  CDBG("Programmed VFE Clock %d",input_cfg.input_pix_clk);
  if( session->bayer_processing ) {
     iface_util_fe_input_buf_t fe_in_buf;
     cam_dimension_t dim;
     cam_format_t fmt;
     dim.width =  session->offline_info.input_dim.width;
     dim.height = session->offline_info.input_dim.height;
     fmt = session->offline_info.input_fmt;
     session->fe.busy = FALSE; /*reset fe state*/

     IFACE_HIGH("Configuring fetch engine isp_id %d, WxH %d, %d, fmt %d, clk=%d",
          isp_id, dim.width, dim.height, fmt,
          iface->isp_axi_data.axi_data[isp_id].interface_clk_request[IFACE_INTF_PIX]);
     iface_util_fe_cfg(session, iface->isp_axi_data.axi_data[isp_id].fd, dim,
          iface->isp_axi_data.axi_data[isp_id].interface_clk_request[IFACE_INTF_PIX],
          fmt, ISP_STRIPE_MAX);
  }

  return rc;
}

/** iface_util_config_axi
 *    @iface: iface pointer
 *    @session: session data
 *
 *   config AXI HW
 *
 * Return: integer rc
 **/
int iface_util_config_axi(
  iface_t         *iface,
  iface_session_t *session)
{
  int rc = 0;
  int i = 0, isp_id = 0;
  iface_hw_stream_t *hw_stream;
  iface_stats_stream_t *stats_stream = NULL;
  iface_hw_stream_set_params_t vfe_hw_streams[VFE_MAX];
  iface_stats_stream_set_params_t vfe_stats_stream[VFE_MAX];
  CDBG("%s: E\n", __func__);

  if (!iface || !session) {
    CDBG_ERROR("%s: NULL pointer, iface = %p, session = %p, rc = -1\n",
      __func__, iface, session);
    return -1;
  }
  memset(vfe_hw_streams, 0, sizeof(vfe_hw_streams));
  memset(vfe_stats_stream, 0, sizeof(vfe_stats_stream));
  /* todo: find pix hw stream and rdi hw stream*/
  for (i= 0; i < IFACE_MAX_STREAMS; i++) {
    hw_stream = &session->hw_streams[i];
    if (hw_stream == NULL || hw_stream->stream_info.dim.width == 0) {
      CDBG("%s: hw stream index %d not configyet, continue.\n",
        __func__, i);
      continue;
    } else {
      CDBG("%s: stream found: str_id = %x, W = %d, H= %d, is split = %d,"
           " use pix %d, intf mask %x\n",
        __func__, hw_stream->hw_stream_id, hw_stream->stream_info.dim.width,
        hw_stream->stream_info.dim.height,
        hw_stream->isp_split_output_info.is_split, hw_stream->use_pix,
        hw_stream->interface_mask);
    }
    if (hw_stream->state != IFACE_HW_STREAM_STATE_RESERVED) {
      continue;
    }

    /* prepare stream config for ioctl "REQUEST STREAM"
       if its vfe0 then tell axi its left stripe, vfe0 for right stripe*/
    for (isp_id = 0; isp_id < VFE_MAX ; isp_id++) {
      if ((hw_stream->interface_mask & (0xffff << (16* isp_id))) != 0) {
        if (hw_stream->isp_split_output_info.is_split == 1) {
          if (isp_id == VFE0)
            hw_stream->isp_split_output_info.stripe_id = ISP_STRIPE_LEFT;
          else if (isp_id == VFE1)
            hw_stream->isp_split_output_info.stripe_id = ISP_STRIPE_RIGHT;
        }
        vfe_hw_streams[isp_id].hw_streams[vfe_hw_streams[isp_id].num_hw_streams++] = *hw_stream;
      }
    }
  }

  CDBG("%s %d No of Hw streams vfe0 = %d vfe1 = %d ",__func__,__LINE__,
    vfe_hw_streams[0].num_hw_streams, vfe_hw_streams[1].num_hw_streams);

  for (isp_id = 0; isp_id < VFE_MAX ; isp_id++) {
    if ( vfe_hw_streams[isp_id].num_hw_streams > 0) {
      rc = iface->isp_axi_data.axi_data[isp_id].axi_hw_ops->set_params(
        iface->isp_axi_data.axi_data[isp_id].axi_hw_ops->ctrl,
        IFACE_AXI_SET_STREAM_CFG, (void *)&vfe_hw_streams[isp_id],
        sizeof(iface_hw_stream_set_params_t));
      if (rc < 0) {
        CDBG_ERROR("%s: VFE0 ISP_AXI_SET_STREAM_CFG failed! rc = %d\n",
          __func__, rc);
        return rc;
      }
    }
  }


  /* after axi,
    change the channel state to be ISP_CHANNEL_STATE_HW_CFG*/
  for (i= 0; i < IFACE_MAX_STREAMS; i++) {
    hw_stream = &session->hw_streams[i];
    if (hw_stream == NULL || hw_stream->stream_info.dim.width == 0
          ||hw_stream->state != IFACE_HW_STREAM_STATE_RESERVED) {
      CDBG("%s: hw stream index %d not configyet, continue.\n",
        __func__, i);
      continue;
    } else {
      CDBG("%s: stream found: str_id = %x, W = %d, H= %d, is split = %d,"
        " use pix %d, intf mask %x\n",
         __func__, hw_stream->hw_stream_id, hw_stream->stream_info.dim.width,
         hw_stream->stream_info.dim.height,
         hw_stream->isp_split_output_info.is_split, hw_stream->use_pix,
         hw_stream->interface_mask);
    }
    hw_stream->state = IFACE_HW_STREAM_STATE_HW_CFG_DONE;
  }

  for (i= 0; i < MSM_ISP_STATS_MAX; i++) {
    stats_stream = &session->stats_streams[i];
    if (stats_stream == NULL || stats_stream->stats_stream_id == 0) {
      CDBG("%s: stats_stream index %d not config yet, continue.\n",
        __func__, i);
      continue;
     } else {
       CDBG("%s: stats_stream found: stats_stream id = %x, stats type = %d,"
            " buf_len = %d, comp flag = %d\n",__func__,
         stats_stream->stats_stream_id, stats_stream->stats_type,
         stats_stream->buf_len, stats_stream->comp_flag);
     }

    /* prepare stats stream config
       if its vfe0 then tell axi its left stripe, vfe0 for right stripe*/
    for (isp_id = 0; isp_id < VFE_MAX ; isp_id++) {
      if (session->session_resource.isp_id_mask & (1 << isp_id)) {
        if (isp_id == VFE1 && stats_stream->buf_type == ISP_SHARE_BUF )
          stats_stream->isp_stripe_id = ISP_STRIPE_RIGHT;
        else
          stats_stream->isp_stripe_id = ISP_STRIPE_LEFT;
        vfe_stats_stream[isp_id].stats_stream
          [vfe_stats_stream[isp_id].num_stats_streams++] = *stats_stream;
        }
      }
  }
  CDBG("%s %d No of Stats streams vfe0 = %d vfe1 = %d ",__func__, __LINE__,
      vfe_stats_stream[0].num_stats_streams, vfe_stats_stream[1].num_stats_streams);

  for (isp_id = 0; isp_id < VFE_MAX ; isp_id++) {
    if ( vfe_stats_stream[isp_id].num_stats_streams > 0) {
      rc = iface->isp_axi_data.axi_data[isp_id].axi_hw_ops->set_params(
      iface->isp_axi_data.axi_data[isp_id].axi_hw_ops->ctrl,
        IFACE_AXI_SET_STATS_STREAM_CFG, (void *)&vfe_stats_stream[isp_id],
        sizeof(iface_stats_stream_set_params_t));
      if (rc < 0) {
        CDBG_ERROR("%s: VFE0 IFACE_AXI_SET_STATS_STREAM_CFG failed! rc = %d\n",
          __func__, rc);
        return rc;
      }
    }
  }
  return rc;
}

/** iface_util_find_start_stop_parm_per_vfe
 *    @iface iface handle
 *    @hw_idx: vfe hw index
 *    @num_streams: notify type
 *
 * TODO
 *
 * Return: nothing
 **/
int iface_util_find_start_stop_parm_per_vfe(iface_t *iface,
  iface_session_t *session, uint32_t num_hw_stream,
  uint32_t  *hw_stream_ids, start_stop_stream_t *start_parm,
  uint32_t vfe_id)
{
   int rc = 0;
   uint32_t i = 0;
   iface_hw_stream_t *hw_stream = NULL;

   if (!iface || !session) {
     CDBG_ERROR("%s: NULL pointer %p %p %p %p, rc = -1\n",
       __func__, iface, session, hw_stream_ids, start_parm);
     return -1;
   }

   start_parm->session_id = session->session_id;
   start_parm->sync_frame_id_src = 0;

   CDBG("%s: Start param for vfe %d\n", __func__, vfe_id);

   start_parm->first_pix_start = (session->active_count == 0) ? TRUE : FALSE;
   for (i = 0; i < num_hw_stream; i++) {
     hw_stream = iface_util_find_hw_stream_in_session(
       session, hw_stream_ids[i]);
     if (hw_stream == NULL)
       continue;

     /* find starting hw stream for specific VFE*/
     if (((hw_stream->interface_mask & (0xFFFF << (vfe_id * 16))) != 0)) {
       start_parm->stream_ids[start_parm->num_streams] =
         hw_stream->hw_stream_id;
       start_parm->mct_streamon_id[start_parm->num_streams] =
         hw_stream->mct_streamon_id;
       CDBG("%s: Start param for hw stream 0x%x, mct stream %d\n", __func__,
            start_parm->stream_ids[start_parm->num_streams],
            start_parm->mct_streamon_id[start_parm->num_streams]);
       start_parm->num_streams++;
     }
     /* Set flag incase PIX and RDI streams are part of same session. This
      * will ensure RDI stream will have same frame id as of PIX stream
      */
     if ((session->num_pix_streamon > 0) &&
       (SRC_TO_INTF(hw_stream->axi_path) >= VFE_RAW_0) &&
       (SRC_TO_INTF(hw_stream->axi_path) < VFE_SRC_MAX) &&
       (hw_stream->stream_type == IFACE_STREAM_TYPE_IMAGE))
       start_parm->sync_frame_id_src = 1;
     CDBG_HIGH("%s: num_pix: %d axi_path: %d sync_frmid: %d strmtype: %d \n", __func__,
       session->num_pix_streamon, hw_stream->axi_path, start_parm->sync_frame_id_src,
       hw_stream->stream_type);
   }

  return rc;
}

/** iface_util_axi_streamon
 *    @isp: ISP pointer
 *    @hw_idx: vfe hw index
 *    @num_streams: notify type
 *
 * TODO
 *
 * Return: nothing
 **/
int iface_util_axi_streamon(
    iface_t           *iface,
    iface_session_t   *iface_session,
    uint32_t           num_hw_streams,
    uint32_t          *hw_stream_ids)
{
   int rc = 0;
   int i = 0;
   uint32_t vfe_id = 0;
   iface_hw_stream_t *hw_stream;
   start_stop_stream_t start_param;

  for (vfe_id = 0; vfe_id < VFE_MAX; vfe_id++) {
    /*find each HW start parm and hw stream id per VFE,
      RDI stream can be non symetric*/
    memset(&start_param, 0, sizeof(start_stop_stream_t));;
    rc = iface_util_find_start_stop_parm_per_vfe(iface, iface_session,
      num_hw_streams, hw_stream_ids, &start_param, vfe_id);
    if (rc < 0) {
       CDBG_ERROR("%s: VFE%d iface_util_find_start_stop_parm_per_vfe faile rc = -1\n",
         __func__, vfe_id);
       return rc;
    }

    if (start_param.num_streams > 0) {
       /* prepare stream config for ioctl "REQUEST STREAM" , set parm: stream_start
          if its vfe0 then tell axi its left stripe, vfe0 for right stripe*/
       rc = iface->isp_axi_data.axi_data[vfe_id].axi_hw_ops->action(
         iface->isp_axi_data.axi_data[vfe_id].axi_hw_ops->ctrl,
         IFACE_AXI_ACTION_CODE_STREAM_START, (void *)&start_param,
         sizeof(start_stop_stream_t));
       if (rc < 0) {
         CDBG_ERROR("%s: VFE%d AXI_STREAM_START failed! rc = %d\n",
           __func__, vfe_id, rc);
         return rc;
       }
    }
  }

  /*after start all the stream then wait for start done on both vfe*/
  for (vfe_id = 0; vfe_id < VFE_MAX; vfe_id++) {
    memset(&start_param, 0, sizeof(start_stop_stream_t));
    rc = iface_util_find_start_stop_parm_per_vfe(iface, iface_session,
      num_hw_streams, hw_stream_ids, &start_param, vfe_id);
    if (rc < 0) {
       CDBG_ERROR("%s: VFE%d iface_util_find_start_stop_parm_per_vfe faile rc = -1\n",
         __func__, vfe_id);
       return rc;
    }

    if (start_param.num_streams > 0) {
       rc = iface->isp_axi_data.axi_data[vfe_id].axi_hw_ops->action(
         iface->isp_axi_data.axi_data[vfe_id].axi_hw_ops->ctrl,
         IFACE_AXI_ACTION_CODE_STREAM_START_ACK, (void *)&start_param,
         sizeof(start_stop_stream_t));
       if (rc < 0) {
         CDBG_ERROR("%s: VFE%d AXI_STREAM_START_ACK failed! rc = %d\n",
           __func__, vfe_id, rc);
         return rc;
       }
    }
  }

/* register input bufq with buf mgr*/
  if (iface_session->bayer_processing) {
     start_param.session_id = iface_session->session_id;
     start_param.sync_frame_id_src = 0;
     start_param.first_pix_start = (iface_session->active_count == 0) ? TRUE : FALSE;
     start_param.num_streams = 1;
     CDBG("register input bufq stream id = %x", iface_session->reproc_buff_info.user_stream_id);
     hw_stream = iface_util_find_hw_stream_in_session(
                   iface_session, hw_stream_ids[0]);
      if (hw_stream == NULL) {
        CDBG_ERROR("hw stream for stream id = %x not found", hw_stream_ids[0]);
        return rc;
       }

      start_param.stream_ids[0] = iface_session->reproc_buff_info.user_stream_id;
      start_param.mct_streamon_id[0] = iface_session->reproc_buff_info.user_stream_id;
      for (vfe_id = 0; vfe_id < VFE_MAX; vfe_id++) {
           if ((hw_stream->interface_mask & (1 << (16 * vfe_id +
             IFACE_INTF_PIX))) ||
             (hw_stream->interface_mask & (1 << (16 * vfe_id +
             IFACE_INTF_RDI0)))) break;
       }
      if (vfe_id > 1)
         return rc;
      CDBG("vfe_id %d hw_stream %x matching PIX stream interface_mask %x",
        vfe_id, hw_stream->hw_stream_id, hw_stream->interface_mask);
      rc = iface->isp_axi_data.axi_data[vfe_id].axi_hw_ops->action(
        iface->isp_axi_data.axi_data[vfe_id].axi_hw_ops->ctrl,
        IFACE_AXI_ACTION_CODE_REG_INPUTBUF, (void *)&start_param,
        sizeof(start_stop_stream_t));
      if (rc < 0) {
        CDBG_ERROR("%s: VFE%d IFACE_AXI_ACTION_CODE_REG_INPUTBUF failed! rc = %d\n",
         __func__, vfe_id, rc);
       return rc;
      }
  }
  return rc;
}

/** iface_util_unconfig_axi
 *    @iface: iface handle
 *    @session: session data
 *
 * this funciton un conofig AXI
 *
 * Return: nothing
 **/
int iface_util_unconfig_axi(iface_t *iface, iface_session_t *session)
{
   int rc = 0;
   int i = 0;
   iface_hw_stream_t *hw_stream;
   uint32_t vfe_id = 0;

  if (!session || !iface) {
    CDBG_ERROR("%s: NULL pointer! %p %p rc = -1\n", __func__, session, iface);
    return -1;
  }
   /*if dual vfe stats stream always split
     since its sitting inside pix interface*/
  if (session->num_stats_stream != 0) {
    for (vfe_id = 0; vfe_id < VFE_MAX; vfe_id++) {
      if ((session->session_resource.isp_id_mask & (1 << vfe_id)) &&
        (iface->isp_axi_data.axi_data[vfe_id].axi_hw_ops != NULL)) {
        rc = iface->isp_axi_data.axi_data[vfe_id].axi_hw_ops->set_params(
          iface->isp_axi_data.axi_data[vfe_id].axi_hw_ops->ctrl,
          IFACE_AXI_SET_STATS_STREAM_UNCFG, NULL, 0);
        if (rc < 0) {
          CDBG_ERROR("%s: VFE%d AXI STATS_STREAM_UNCFG failed! rc = %d\n",
            __func__, vfe_id, rc);
          return rc;
        }
      }
    }
  }

   /* todo: find pix hw stream and rdi hw stream*/
  for (i= 0; i < IFACE_MAX_STREAMS; i++) {
    hw_stream = &session->hw_streams[i];
    if (hw_stream == NULL ||
        hw_stream->state != IFACE_HW_STREAM_STATE_HW_CFG_DONE ||
        hw_stream->stream_info.cam_stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
      continue;
    }

    hw_stream->state = IFACE_HW_STREAM_STATE_RESERVED;

    /* unconfig VFE0 frame stream and stats stream */
    if (((hw_stream->interface_mask & (0xffff << (16 * VFE0))) != 0) &&
      (iface->isp_axi_data.axi_data[VFE0].axi_hw_ops != NULL)) {
      if (hw_stream->isp_split_output_info.is_split == 1)
          hw_stream->isp_split_output_info.stripe_id = ISP_STRIPE_LEFT;

      rc = iface->isp_axi_data.axi_data[VFE0].axi_hw_ops->set_params(
        iface->isp_axi_data.axi_data[VFE0].axi_hw_ops->ctrl,
        IFACE_AXI_SET_STREAM_UNCFG, (void *)hw_stream,
        sizeof(iface_hw_stream_t));
      if (rc < 0) {
        CDBG_ERROR("%s: VFE0 AXI_SET_STREAM_CFG failed! rc = %d\n",
          __func__, rc);
        return rc;
      }
    }

    if ((hw_stream->interface_mask & (0xffff << (16 * VFE1))) != 0 &&
      (iface->isp_axi_data.axi_data[VFE1].axi_hw_ops != NULL)) {
      if (hw_stream->isp_split_output_info.is_split == 1)
        hw_stream->isp_split_output_info.stripe_id = ISP_STRIPE_RIGHT;
      rc = iface->isp_axi_data.axi_data[VFE1].axi_hw_ops->set_params(
        iface->isp_axi_data.axi_data[VFE1].axi_hw_ops->ctrl,
        IFACE_AXI_SET_STREAM_UNCFG, (void *)hw_stream, sizeof(iface_hw_stream_t));
      if (rc < 0) {
        CDBG_ERROR("%s: VFE1 ISP_AXI_SET_STREAM_CFG failed! VFE1 rc = %d\n",
        __func__, rc);
        return rc;
      }
    }
  }

  return rc;
}

/** iface_util_axi_streamon
 *    @isp: ISP pointer
 *    @hw_idx: vfe hw index
 *    @num_streams: notify type
 *
 * TODO
 *
 * Return: nothing
 **/
int iface_util_axi_streamoff(
    iface_t         *iface,
    iface_session_t *session,
    uint32_t         num_hw_streams,
    uint32_t        *hw_stream_ids,
    enum msm_vfe_axi_stream_cmd stop_cmd)
{
   int rc = 0;
   int i = 0;
   iface_hw_stream_t *hw_stream;
   start_stop_stream_t stop_param;
   uint32_t vfe_id = 0;

   for (vfe_id = 0; vfe_id < VFE_MAX; vfe_id++) {
      /* prepare stream config for ioctl "REQUEST STREAM" , set parm: stream_start
         if its vfe0 then tell axi its left stripe, vfe0 for right stripe*/
          /*find each HW start parm and hw stream id per VFE,
         RDI stream can be non symetric*/
     memset(&stop_param, 0, sizeof(start_stop_stream_t));
     rc = iface_util_find_start_stop_parm_per_vfe(
         iface, session, num_hw_streams, hw_stream_ids, &stop_param, vfe_id);
     if (rc < 0) {
       CDBG_ERROR("%s: VFE%d iface_util_find_start_stop_parm_per_vfe faile rc = -1\n",
         __func__, vfe_id);
       return rc;
     }

     stop_param.stop_cmd = stop_cmd;
     if (stop_param.num_streams > 0) {
        rc = iface->isp_axi_data.axi_data[vfe_id].axi_hw_ops->action(
          iface->isp_axi_data.axi_data[vfe_id].axi_hw_ops->ctrl,
          IFACE_AXI_ACTION_CODE_STREAM_STOP, (void *)&stop_param, sizeof(start_stop_stream_t));
        if (rc < 0) {
          CDBG_ERROR("%s: VFE%d AXI_STREAM_STOP failed! rc = %d\n",
            __func__, vfe_id, rc);
          return rc;
        }
     }
   }

  for (vfe_id = 0; vfe_id < VFE_MAX; vfe_id++) {
    /*stop ack to wait until config axi 0&1 done*/
    memset(&stop_param, 0, sizeof(start_stop_stream_t));
    rc = iface_util_find_start_stop_parm_per_vfe(iface, session,
      num_hw_streams, hw_stream_ids, &stop_param, vfe_id);
    if (rc < 0) {
      CDBG_ERROR("%s: VFE%d iface_util_find_start_stop_parm_per_vfe faile rc = -1\n",
        __func__, vfe_id);
      return rc;
    }

    if (stop_param.num_streams) {
       rc = iface->isp_axi_data.axi_data[vfe_id].axi_hw_ops->action(
         iface->isp_axi_data.axi_data[vfe_id].axi_hw_ops->ctrl,
         IFACE_AXI_ACTION_CODE_STREAM_STOP_ACK, (void *)&stop_param, sizeof(start_stop_stream_t));
       if (rc < 0) {
         CDBG_ERROR("%s: VFE%d AXI_STREAM_STOP_ACK failed! rc = %d\n",
           __func__, vfe_id, rc);
         return rc;
       }
    }
  }
#ifdef ISP_SVHDR_OUT_BIT
  /* unregister input bufq with buf mgr*/
  if (session->bayer_processing &&
    (session->reproc_buff_info.user_stream_id & ISP_SVHDR_OUT_BIT)) {
     stop_param.session_id = session->session_id;
     stop_param.sync_frame_id_src = 0;
     stop_param.first_pix_start = (session->active_count == 0) ? TRUE : FALSE;
     stop_param.num_streams = 1;
     CDBG("unregister input bufq stream id = %x", session->reproc_buff_info.user_stream_id);
     hw_stream = iface_util_find_hw_stream_in_session(
                    session, hw_stream_ids[0]);
     if (hw_stream == NULL) {
        CDBG_ERROR("hw stream for stream id = %x not found", hw_stream_ids[0]);
        return -1;
        }
      stop_param.stream_ids[0] = session->reproc_buff_info.user_stream_id;
      stop_param.mct_streamon_id[0] = session->reproc_buff_info.user_stream_id;
     for (vfe_id = 0; vfe_id < VFE_MAX; vfe_id++) {
       if ((hw_stream->interface_mask & (1 << (16 * vfe_id +
        IFACE_INTF_PIX))) ||
        (hw_stream->interface_mask & (1 << (16 * vfe_id +
        IFACE_INTF_RDI0)))) break;
     }
     if (vfe_id > 1)
        return rc;
     rc = iface->isp_axi_data.axi_data[vfe_id].axi_hw_ops->action(
        iface->isp_axi_data.axi_data[vfe_id].axi_hw_ops->ctrl,
        IFACE_AXI_ACTION_CODE_UNREG_INPUTBUF, (void *)&stop_param,
        sizeof(start_stop_stream_t));
      if (rc < 0) {
        CDBG_ERROR("%s: VFE%d IFACE_AXI_ACTION_CODE_UNREG_INPUTBUF failed! rc = %d\n",
        __func__, vfe_id, rc);
       return rc;
    }
    }
#endif
  return rc;
}

/** iface_util_check_raw_stream_fmt:
 *    @iface_sink_port: iface sink port
 *    @sensor_meta_info: sensor meta info
 *
 *  This function checks whether HAL has created
 *  a stream with same format.
 *
 *  Return:1 - Success
 *             0 - Fail
 **/

static boolean iface_util_check_raw_stream_fmt(iface_sink_port_t *sink_port,
  sensor_meta_t *sensor_meta_info)
{
  int32_t           i = 0;
  mct_stream_info_t *stream_info = NULL;

  if (!sink_port || !sensor_meta_info) {
    CDBG_ERROR("%s:%d sink_port %p sensor_meta_info %p", sink_port,
      sensor_meta_info);
    return FALSE;
  }

  for (i = 0; i < sink_port->num_streams; i++) {
    stream_info = &sink_port->streams[i].stream_info;
    if (stream_info->fmt == sensor_meta_info->fmt) {
      return TRUE;
    }
  }

  return FALSE;
}

/** iface_util_set_hw_stream_cfg:
 *    @iface: iface instance
 *    @iface_sink_port: iface sink port
 *
 *  This function runs in MCTL thread context.
 *
 *  This function reserves an ISP resource
 *
 *  Return: 0 - Success
 *         -1 - Couldn't reserve resource
 **/
int iface_util_set_hw_stream_cfg(iface_t *iface,
  iface_port_t *iface_sink_port, uint32_t session_id, uint32_t stream_id)
{
  uint32_t i = 0;
  uint32_t j = 0;
  iface_stream_t *user_stream;
  iface_sink_port_t *sink_port = &iface_sink_port->u.sink_port;
  iface_session_t *session = NULL;
  uint32_t         user_stream_id = 0;
  int rc = 0;
  iface_bundle_meta_info_t *bundle_meta_info = NULL;
  iface_bundle_pdaf_info_t *bundle_pdaf_info = NULL;
  mct_event_t event;
  CDBG("E stream_id %x\n", stream_id);

  session = iface_util_get_session_by_id(iface, session_id);
  if (session == NULL) {
     CDBG_ERROR("%s: can not find session, id = %d, rc = -1\n", __func__, session_id);
     return -1;
  }

  if ((session->hdr_mode == CAM_SENSOR_HDR_STAGGERED &&
         sink_port->sensor_out_info.custom_format.enable) ||
       (session->binncorr_mode && session->sensor_out_info.binning_factor > 1)) {
     session->bayer_processing = 1;
  } else {
     session->bayer_processing = 0;
  }
  CDBG("iface bayer_processing = %d",session->bayer_processing);

  /* use common initial frame skip for all streams */
  session->initial_frame_skip =
    iface_sink_port->u.sink_port.sensor_out_info.num_frames_skip;

  /* reserve PIX: request ISP to feedback the resource output info
     will reserve: 1. session resource
                   2. stream interface mask
                   3. hw stream output info*/
  rc = iface_util_set_hw_stream_config_pix(iface, iface_sink_port, session_id, stream_id);
  if (rc < 0) {
     CDBG_ERROR("%s: reserve camif error! session = %x\n", __func__, session_id);
     return rc;
  }

  for (i = 0; i < session->num_pdaf_meta && i < IFACE_MAX_STREAMS; i++) {
    bundle_pdaf_info = &session->bundle_pdaf_info[i];
    if (bundle_pdaf_info->user_stream_id == 0) {
      continue;
    }

    user_stream = iface_util_find_stream_in_sink_port(iface_sink_port,
      session_id, bundle_pdaf_info->user_stream_id);
    if (!user_stream) {
      CDBG_ERROR("%s: no user stream found\n", __func__);
      return -1;
    }
    rc = iface_utill_cfg_hw_stream_pix_raw(iface,
      sink_port, user_stream, session, TRUE,
      bundle_pdaf_info, CAMIF_RAW);
    if (rc < 0) {
      CDBG_ERROR("%s: Error config meta RDI stream\n", __func__);
      return rc;
    }
    memset(&event, 0, sizeof(mct_event_t));
    event.type = MCT_EVENT_MODULE_EVENT;
    event.u.module_event.type = MCT_EVENTS_MODULE_PDAF_ISP_INFO;
    event.u.module_event.module_event_data = (void *)&session->camif_info;
    event.identity = pack_identity(session->session_id,
      user_stream->stream_id);
    event.direction = MCT_EVENT_UPSTREAM;
    if (FALSE == mct_port_send_event_to_peer(
      user_stream->sink_port, &event)) {
      CDBG_ERROR("%s: PDAF_ISP_INFO error sessid %d, streamid %d, rc = -1\n",
        __func__, user_stream->session_id, user_stream->stream_id);
      return -1;
    }
  }

  if(session->bayer_processing) {
     for (i = 0; i < IFACE_MAX_STREAMS; i++) {
        user_stream = &sink_port->streams[i];
        if (user_stream->stream_info.stream_type == CAM_STREAM_TYPE_VIDEO ||
            user_stream->stream_info.stream_type == CAM_STREAM_TYPE_PREVIEW) {
          CDBG("configure RDI for stream_id %x", stream_id);
          rc = iface_util_set_hw_stream_cfg_rdi(iface,
          iface_sink_port, user_stream, session, FALSE, NULL);
          if (rc < 0) {
            CDBG_ERROR("%s: Error reserving RDI resource\n", __func__);
            return rc;
          }
          break;
       }
    }
  }
  /* reserve the rest hw stream,
     go through each stream to reserve*/
  for (i = 0; i < IFACE_MAX_STREAMS; i++) {
    if (sink_port->streams[i].sink_port == NULL)
      continue;

    user_stream = &sink_port->streams[i];

   /*if EIS 2.0  enabled, then we dont enable CDS*/
   if (user_stream->stream_info.is_type == IS_TYPE_EIS_2_0 ||
       user_stream->stream_info.is_type == IS_TYPE_EIS_3_0) {
     session->cds_feature_enable = FALSE;
   }

    /* in iface only reserve RDI stream, ISP will reserve pix streams,
       if already reserved then continue to next stream in sink port
       move the use_pix logic to here */
    CDBG_HIGH("%s: user_stream->use_pix %d stream id %x", __func__, user_stream->use_pix,
      user_stream->stream_id);
    if (user_stream->use_pix ||
      user_stream->stream_info.stream_type == CAM_STREAM_TYPE_OFFLINE_PROC ||
      user_stream->stream_info.stream_type == CAM_STREAM_TYPE_PARM) {
       continue;
    }

    /* reserve resource and reserve output hw stream info
     * will reserve: 1. session resource
     *               2. stream interface mask
     *               3. hw stream output info
     */
    CDBG("%s: user_stream->axi_path  %d stram id %x ", __func__,
      user_stream->axi_path, user_stream->stream_id);

    if ((user_stream->axi_path == CAMIF_RAW) ||
      (user_stream->axi_path == IDEAL_RAW)) {
      rc = iface_utill_cfg_hw_stream_pix_raw(iface,
        sink_port, user_stream, session, FALSE, NULL, user_stream->axi_path);
      if (rc < 0) {
        CDBG_ERROR("%s: Error reserving RDI resource\n", __func__);
        return rc;
      }
    } else if(user_stream->use_pix == 0){
        /* RDI Raw */
        rc = iface_util_set_hw_stream_cfg_rdi(iface,
          iface_sink_port, user_stream, session, FALSE, NULL);
        if (rc < 0) {
          CDBG_ERROR("%s: Error reserving RDI resource\n", __func__);
          return rc;
        }
    }
  }

  /* Sensor Meta channel (VHDR)
   * If there are multiple meta channel per user stream then iterate
   * through them and create hw streams accordingly.
   */
  for (i = 0; i < session->num_bundle_meta && i < IFACE_MAX_STREAMS; i++) {
    bundle_meta_info = &session->bundle_meta_info[i];
    if (bundle_meta_info->user_stream_id == 0) {
      continue;
    } else {
      CDBG_HIGH("%s: sensor meta RDI with user stream id %d\n",
        __func__, bundle_meta_info->user_stream_id);
    }
    for (j = 0; j < bundle_meta_info->sensor_meta.num_meta
      && j < MAX_META; j++) {
      user_stream = iface_util_find_stream_in_sink_port(iface_sink_port,
        session_id, bundle_meta_info->user_stream_id);
      /* Check if HAL created stream with same format, if so,
       * Do not create native buf stream for this sensor_meta_config
       * since backend doesn't need this metadata
       */
      if (iface_util_check_raw_stream_fmt(sink_port,
        &bundle_meta_info->sensor_meta.sensor_meta_info[j]) == FALSE) {
        rc = iface_util_set_hw_stream_cfg_rdi(iface,
          iface_sink_port, user_stream, session, TRUE,
          &bundle_meta_info->sensor_meta.sensor_meta_info[j]);
        if (rc < 0) {
          CDBG_ERROR("%s: Error config meta RDI stream\n", __func__);
          return rc;
        }
      }
    }
  }

  return rc;
}

/** iface_util_reset_resource_mask
 *
 * DESCRIPTION:
 *
 **/
static int iface_util_reset_resource_mask(iface_t *iface, iface_session_t *iface_session,
  uint32_t interface_mask, uint32_t isp_id_mask)
{
  CDBG("%s: interface_mask= %x isp_id_mask= %x\n", __func__,
    interface_mask, isp_id_mask);

  iface_session->session_resource.used_resource_mask &= ~interface_mask;
  iface->rdi_used_resource_mask &= ~interface_mask;

  return 0;
}

/** iface_util_release_resource:
 *    @iface: iface pointer
 *    @stream: stream
 *
 *  This function runs in MCTL thread context.
 *
 *  This function releases ISP resources associated with a stream
 *
 *  Return: None
 **/
void iface_util_release_resource(iface_t *iface,
  iface_session_t *session, iface_hw_stream_t *hw_stream)
{
  int rc = 0;
  CDBG("%s: E  hw stream %x interface_mask %x use_pix %d\n",
    __func__,hw_stream->hw_stream_id,
    hw_stream->interface_mask, hw_stream->use_pix);

  if (hw_stream->interface_mask == 0)
    return;

  if (hw_stream->use_pix) {
    if (session->session_resource.camif_cnt) {
      session->session_resource.camif_cnt--;
    }
  } else {
    /* RDI case */
     if (session->session_resource.rdi_cnt) {
       session->session_resource.rdi_cnt--;
     }
  }


  IFACE_HIGH("camif_cnt= %d, rdi_cnt= %d, used mask %x\n",
            session->session_resource.camif_cnt,
            session->session_resource.rdi_cnt,
            session->session_resource.used_resource_mask);

  if (session->session_resource.isp_id_mask  & (1 << VFE0)) {
    if (session->create_axi_count[VFE0] > 0 &&
        --session->create_axi_count[VFE0] == 0) {
      iface_destroy_hw(iface, VFE0);
      CDBG_HIGH("%s: Destroy axi vfe0, cnt = %d\n",
                __func__, session->create_axi_count[VFE0]);
    } else {
      CDBG("%s: skip destroy axi vfe0, cnt = %d\n",
           __func__, session->create_axi_count[VFE0]);
    }
  }

  if (session->session_resource.isp_id_mask  & (1 << VFE1)) {
    if (session->create_axi_count[VFE1] > 0 &&
        --session->create_axi_count[VFE1] == 0) {
      iface_destroy_hw(iface, VFE1);
      CDBG_HIGH("%s: Destroy axi vfe1, cnt = %d\n",
                __func__, session->create_axi_count[VFE1]);
    } else {
      CDBG("%s: skip destroy axi vfe1, cnt = %d\n",
           __func__, session->create_axi_count[VFE1]);
    }
  }

  pthread_mutex_lock(&iface->mutex);
  iface_util_reset_resource_mask(iface, session, hw_stream->interface_mask,
    session->session_resource.isp_id_mask);

  /* Reset session_resource mask if both offline & online VFE are done */
  if (session->session_resource.camif_cnt == 0 &&
      session->session_resource.rdi_cnt == 0 &&
      session->session_resource.offline_isp_id_mask == 0) {
    uint32_t i = 0;
    for (i = 0; i < VFE_MAX; i ++)
      iface->used_wm[i] -= session->session_resource.used_wm[i];
    memset(&session->session_resource, 0, sizeof(iface_resource_t));
  }
  pthread_mutex_unlock(&iface->mutex);

  if (session->num_hw_stream) {
    session->num_hw_stream--;
  }

  memset(hw_stream, 0, sizeof(*hw_stream));
}

/** iface_util_find_isp_intf_type:
 *    @stream: stream
 *
 *  This function runs in MCTL thread context.
 *
 *  This function determines ISP interface type associates with a stream
 *  by output mask
 *
 *  Return: msm_iface_intftype enumeration of interface found
 *          INTF_MAX - invalid mask
 **/
enum msm_ispif_intftype iface_util_find_isp_intf_type(
  iface_hw_stream_t *hw_stream, uint32_t isp_id)
{
  if (hw_stream->interface_mask & (1 << (16 * isp_id + IFACE_INTF_PIX)))
    return PIX0;
  else if (hw_stream->interface_mask & (1 << (16 * isp_id + IFACE_INTF_RDI0)))
    return RDI0;
  else if (hw_stream->interface_mask & (1 << (16 * isp_id + IFACE_INTF_RDI1)))
    return RDI1;
  else if (hw_stream->interface_mask & (1 << (16 * isp_id + IFACE_INTF_RDI2)))
    return RDI2;
  else
    return INTF_MAX;
}

/** iface_util_find_isp_intf_type:
 *    @session: session_handle
 *    @hw_stream: hw stream handle
 *
 *  This function decide if its dual vfe for the stream
 *
 *  Return: TRUE: dual vfe per session
 *          FALSE: single vfe per session
 **/
boolean iface_util_is_dual_vfe_used_per_stream(iface_session_t *session,
  iface_hw_stream_t *hw_stream)
{
  uint32_t stream_mask_vfe0 = 0xffff << (16 * VFE0);
  uint32_t stream_mask_vfe1 = 0xffff << (16 * VFE1);
  uint32_t dual_vfe_session_mask = (1 << VFE0) | (1 << VFE1);

  if (!session || !hw_stream) {
    CDBG_ERROR("%s: null pointer session %p hw_stream %p\n",
      __func__, session, hw_stream);
    return FALSE;
  }
  if ((session->session_resource.isp_id_mask == dual_vfe_session_mask) &&
      ((hw_stream->interface_mask & stream_mask_vfe0) != 0) &&
      ((hw_stream->interface_mask & stream_mask_vfe1) != 0)) {
    CDBG("%s: dual_vfe enabled\n", __func__);
    return TRUE;
  }

  CDBG("%s: single vfe\n", __func__);
  return FALSE;
}

/** iface_util_request_image_buf_hal
 *
 * DESCRIPTION:
 *
 **/
static int iface_util_request_image_buf_hal(iface_t *iface,
  iface_session_t *session, iface_hw_stream_t *hw_stream)
{
  iface_buf_request_t buf_request;
  uint32_t i;
  int rc = 0, buff_id = 0;
  memset(&buf_request, 0, sizeof(buf_request));
  buf_request.buf_handle = 0;
  buf_request.session_id = hw_stream->session_id;

   /* if shared HW stream, use user stream id */
  if (hw_stream->shared_hw_stream) {
    buf_request.stream_id = hw_stream->mct_streamon_id;
  } else {
    buf_request.stream_id = hw_stream->hw_stream_id;
  }

  CDBG("%s: request image buf hal for hw stream 0x%x, stream id %d\n",
       __func__, hw_stream->mct_streamon_id, buf_request.stream_id);

  /* dual VFEs use shared buf type
     single ISP case use private buf type*/
  if (iface_util_is_dual_vfe_used_per_stream(session, hw_stream) == TRUE) {
    buf_request.buf_type = ISP_SHARE_BUF;
  } else {
    buf_request.buf_type = ISP_PRIVATE_BUF;
  }

  /* if shared HW stream is present,
     we already update mct streamon id in decide hw stream */
  buff_id = iface_util_get_buffers_id(hw_stream, hw_stream->mct_streamon_id);
  if (buff_id < 0 || buff_id >= VFE_BUF_QUEUE_MAX) {
    CDBG_ERROR("%s: can not find buffers id, rc = -1\n", __func__);
    return -1;
  }

  if (hw_stream->buffers_info[buff_id].is_mapped == TRUE) {
   /* for shared hw streams only need to request buf once
    * when the first shared stream streamon */
   CDBG_HIGH("%s:%d hw_stream_id %x buffers are already mapped skip now\n",
     __func__, __LINE__, hw_stream->hw_stream_id);
    return 0;
  }

  CDBG("%s:request image buf hal for hw stream 0x%x with buf index %d\n",
       __func__, hw_stream->mct_streamon_id, buff_id);

  buf_request.img_buf_list = hw_stream->buffers_info[buff_id].img_buffer_list;
  /*Kernel buf struct is allocated once, even for deferred buf alloc case.
  Hence we need to update count of total buffers to kernel first time*/
  buf_request.total_num_buf = hw_stream->buffers_info[buff_id].num_bufs;
  buf_request.security_mode = session->security_mode;

  CDBG("%s:request buf with image buf list 0x%x, num bufs %d\n",
       __func__,buf_request.img_buf_list, buf_request.total_num_buf);

  rc = iface_request_buf(&iface->buf_mgr, &buf_request);
  if (rc < 0) {
    CDBG_ERROR("%s:%d isp_request_buf stream_id: %d buff_id: %d",
      __func__, __LINE__, buf_request.stream_id, buff_id);
    return -1;
  }

  hw_stream->buffers_info[buff_id].bufq_handle = buf_request.buf_handle;
  hw_stream->buffers_info[buff_id].total_num_bufs = buf_request.total_num_buf;
  hw_stream->buffers_info[buff_id].current_num_bufs =
    buf_request.current_num_buf;
  hw_stream->buffers_info[buff_id].is_mapped = TRUE;

  return rc;
}

/** iface_util_request_image_bufs:
 *    @iface: iface handle
 *    @buf_info: native buf alloc info
 *    @axi_stream_src: axi stream src
 *    @need_adsp_heap: adsp heap flag
 *    @num_additional_buffers: num of additional bufs required
 *
 *  This function allocates native buf handles early for later stream on
 *
 *  Returns 0 for success and negative error on failure
 **/
int iface_util_request_image_bufs(iface_t *iface,
                iface_session_t *session,
                cam_frame_len_offset_t *buf_info,
                enum msm_vfe_axi_stream_src axi_stream_src,
                boolean  need_adsp_heap, uint32_t num_additional_buffers)
{
  int ret = 0;
  uint32_t max_native_buf_num = IFACE_MAX_NATIVE_BUF_NUM;
  int i=0;
  iface_frame_buffer_t tmp_image_bufs[IFACE_MAX_IMG_BUF];

  CDBG("%s: Enter\n", __func__);

  /*For deferred buffer allocation, HAL buffers are updated after streamon
      For non HAL buffers current and total number of buffers is same*/
  if (session->hfr_param.hfr_mode > CAM_HFR_MODE_OFF &&
    session->hfr_param.hfr_mode < CAM_HFR_MODE_MAX ) {
    max_native_buf_num = IFACE_MAX_NATIVE_HFR_BUF_NUM ;
  } else {
    max_native_buf_num = IFACE_MAX_NATIVE_BUF_NUM;
  }
  /* If we have inflight request from HAL
     and that (inflight request + Min BUF) Requirement from ISP
     is greater over write with greater value. */
  if ((session->hal_max_buffer + IFACE_MIN_NATIVE_BUF_NUM) >
      IFACE_MAX_NATIVE_BUF_NUM) {
    max_native_buf_num = session->hal_max_buffer +
      IFACE_MIN_NATIVE_BUF_NUM;
  }
  max_native_buf_num += num_additional_buffers;

  ret = iface_alloc_native_buf_handles(&iface->buf_mgr, buf_info,
           max_native_buf_num, need_adsp_heap,
           session->image_bufs[axi_stream_src], session->security_mode);

  return ret;
}

/** iface_util_request_image_buf_native
 *
 * DESCRIPTION:
 *
 **/
int iface_util_request_image_buf_native(iface_t *iface,
                iface_session_t *session, iface_hw_stream_t *hw_stream)
{
  int rc = 0, buff_id = 0;
  int ret = 0;
  iface_buf_request_t buf_request;
#ifdef _ANDROID_
  char  value[PROPERTY_VALUE_MAX];
#endif
  uint32_t max_native_buf_num = IFACE_MAX_NATIVE_BUF_NUM;

  /*if shared hw stream, in decide hw stream we already update the mct streamon id*/
  buff_id = iface_util_get_buffers_id(hw_stream, hw_stream->mct_streamon_id);
  if (buff_id < 0 || buff_id >= VFE_BUF_QUEUE_MAX) {
    CDBG_ERROR("%s: can not find buffers id, rc = -1\n", __func__);
    return -1;
  }

  if (hw_stream->buffers_info[buff_id].is_mapped == TRUE) {
   /* for shared hw streams only need to request buf once
    * when the first shared stream streamon */
   CDBG_HIGH("%s:%d hw_stream_id %x buffers are already mapped skip now\n",
     __func__, __LINE__, hw_stream->hw_stream_id);
   return 0;
  }

  memset(&buf_request, 0, sizeof(buf_request));
  buf_request.buf_handle = 0;
  buf_request.session_id = hw_stream->session_id;
  buf_request.stream_id = hw_stream->hw_stream_id;
  buf_request.use_native_buf = hw_stream->use_native_buf;
  buf_request.adsp_heap = hw_stream->native_buf_use_adspheap;
  buf_request.security_mode = session->security_mode;

  /*For deferred buffer allocation, HAL buffers are updated after streamon
  For non HAL buffers current and total number of buffers is same*/
  if (session->hfr_param.hfr_mode > CAM_HFR_MODE_OFF &&
    session->hfr_param.hfr_mode < CAM_HFR_MODE_MAX ) {
    max_native_buf_num = IFACE_MAX_NATIVE_HFR_BUF_NUM ;
  } else {
    max_native_buf_num = IFACE_MAX_NATIVE_BUF_NUM;
  }
  /* If we have inflight request from HAL
     and that (inflight request + Min BUF) Requirement from ISP
     is greater over write with greater value. */
  if (((session->hal_max_buffer + IFACE_MIN_NATIVE_BUF_NUM) >
      IFACE_MAX_NATIVE_BUF_NUM) && !hw_stream->divert_to_3a) {
    max_native_buf_num = session->hal_max_buffer +
      IFACE_MIN_NATIVE_BUF_NUM;
  }
  max_native_buf_num += hw_stream->num_additional_native_buffers;

  CDBG("%s: Buf num=%d hw_stream_id=0x%x axi_path=%d buff_id=%d hal_max_buffer %d\n", __func__,
       max_native_buf_num,hw_stream->hw_stream_id,hw_stream->axi_path,buff_id,session->hal_max_buffer);

  buf_request.total_num_buf = max_native_buf_num;
  buf_request.current_num_buf = max_native_buf_num;
  hw_stream->buffers_info[buff_id].total_num_bufs =
    max_native_buf_num;
  hw_stream->buffers_info[buff_id].current_num_bufs =
    max_native_buf_num;

  /* dual VFEs use shared buf type
     single ISP case use private buf type*/
  if (iface_util_is_dual_vfe_used_per_stream(session, hw_stream) == TRUE)
    buf_request.buf_type = ISP_SHARE_BUF;
  else
    buf_request.buf_type = ISP_PRIVATE_BUF;

  buf_request.buf_info = hw_stream->stream_info.buf_planes.plane_info;
  buf_request.cached = 1;
  if (session->image_bufs[hw_stream->axi_path][0].buffer.m.planes != NULL &&
    hw_stream->stream_info.cam_stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) {
    CDBG("%s: Enhanced native buf reuse for axi_src=%d\n",
        __func__, hw_stream->axi_path);
    rc = iface_update_native_buf_handles(&iface->buf_mgr, &buf_request,
        session->image_bufs[hw_stream->axi_path]);
  }
  else {
    CDBG("%s: Traditional native buf alloc for axi_src=%d\n",
        __func__, hw_stream->axi_path);
    rc = iface_request_buf(&iface->buf_mgr, &buf_request);
  }

  if (rc < 0) {
    CDBG_ERROR("%s: isp_request_buf error= %d\n", __func__, rc);
  } else {
    hw_stream->buffers_info[buff_id].bufq_handle =
      buf_request.buf_handle;
    hw_stream->buffers_info[buff_id].is_mapped = TRUE;
    hw_stream->buffers_info[buff_id].img_buffer_list = buf_request.img_buf_list;
    hw_stream->buffers_info[buff_id].num_bufs = buf_request.total_num_buf;
  }

  return rc;
}

/** iface_util_count_image_buff
 *    @d1: list entry data
 *    @d2: user data
 *
 * Count buffer in list
 *
 *  Returns TRUE for success and FALSE for failure
 **/
static boolean iface_util_count_image_buff(void *d1 __unused, void *d2)
{
  int32_t *cnt = (int32_t *)d2;

  (*cnt)++;

  return TRUE;
}

/** iface_util_request_image_bufq_input
 *
 * DESCRIPTION:
 *
 **/
int iface_util_request_image_bufq_input(iface_t *iface, mct_event_t *event)
{
  int rc = 0;
  iface_buf_request_t buf_request;
  iface_session_t *session = NULL;
  mct_list_t *img_buffer_list = NULL;
  iface_hw_stream_t *hw_stream = NULL;
  uint32_t isp_id = 0;

  if (!iface || !event) {
    CDBG_ERROR("%s: failed: %p %p\n", __func__, iface, event);
    return -1;
  }

  img_buffer_list = (mct_list_t *)event->u.module_event.module_event_data;
  if (!img_buffer_list) {
    CDBG_ERROR("%s: failed: stream info is NULL\n", __func__);
    return -1;
  }

  session = iface_util_get_session_by_id(iface,
    UNPACK_SESSION_ID(event->identity));
  if (!session) {
    CDBG_ERROR("%s: cannot find session %d\n", __func__,
      UNPACK_SESSION_ID(event->identity));
    return -1;
  }
  if (!session->bayer_processing) {
    return 0;
  }

  memset(&buf_request, 0, sizeof(buf_request));
  buf_request.buf_handle = 0;
  buf_request.session_id = session->session_id;
#ifdef ISP_SVHDR_OUT_BIT
  buf_request.stream_id =
    UNPACK_STREAM_ID(event->identity) | ISP_SVHDR_OUT_BIT | ISP_NATIVE_BUF_BIT;
#endif
  buf_request.use_native_buf = TRUE;
  buf_request.buf_type = ISP_PRIVATE_BUF;
  mct_list_traverse(img_buffer_list, iface_util_count_image_buff,
    &buf_request.current_num_buf);
  buf_request.total_num_buf = buf_request.current_num_buf;
  buf_request.img_buf_list = img_buffer_list;

  rc = iface_request_buf(&iface->buf_mgr, &buf_request);
  if (rc < 0) {
    CDBG_ERROR("%s: isp_request_buf error= %d\n", __func__, rc);
    return rc;
  }

    session->reproc_buff_info.total_num_bufs = buf_request.total_num_buf;
    session->reproc_buff_info.current_num_bufs = buf_request.total_num_buf;
    session->reproc_buff_info.bufq_handle = buf_request.buf_handle;
    session->reproc_buff_info.user_stream_id = buf_request.stream_id;
    session->reproc_buff_info.is_mapped = TRUE;

    CDBG_ERROR("%s:%d: ide %x isp_id %d stream_id %x buf_handle %x total_num_buf %d",
      __func__, __LINE__, event->identity, isp_id, buf_request.stream_id,
      buf_request.buf_handle, buf_request.total_num_buf);

  return rc;
}
/** iface_util_save_input_dim
 *
 * DESCRIPTION:
 *
 **/
int iface_util_save_input_dim(iface_t *iface, mct_event_t *event)
{
  iface_session_t   *session = NULL;
  mct_stream_info_t *stream_info = NULL;
  iface_hw_stream_t *hw_stream = NULL;
  uint32_t           isp_id;

  if (!iface || !event) {
    CDBG_ERROR("%s: failed: %p %p\n", __func__, iface, event);
    return -1;
  }

  stream_info = (mct_stream_info_t *)event->u.module_event.module_event_data;
  if (!stream_info) {
    CDBG_ERROR("%s: failed: stream info is NULL\n", __func__);
    return -1;
  }

  session = iface_util_get_session_by_id(iface,
    UNPACK_SESSION_ID(event->identity));
  if (!session) {
    CDBG_ERROR("%s: cannot find session %d\n", __func__,
      UNPACK_SESSION_ID(event->identity));
    return -1;
  }
  if (!session->bayer_processing) {
    return 0;
  }

  session->offline_info.input_fmt = stream_info->fmt;
  session->offline_info.input_dim.width = stream_info->buf_planes.plane_info.mp[0].width;
  session->offline_info.input_dim.height = stream_info->buf_planes.plane_info.mp[0].height;

  CDBG_ERROR("%s:%d: ide %x  fmt %d num_planes %d, dim %dx%d ", __func__,
      __LINE__, event->identity,  stream_info->fmt,
      stream_info->buf_planes.plane_info.num_planes,
      stream_info->buf_planes.plane_info.mp[0].width,
      stream_info->buf_planes.plane_info.mp[0].height);
  return 0;
}

/** iface_util_request_image_buf
 *    @stream_ids:
 *    @num_streams:
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int iface_util_request_image_buf(iface_t *iface, iface_session_t *session,
  int num_hw_streams, uint32_t *hw_stream_ids)
{
  int i, rc = 0;
  iface_hw_stream_t *hw_stream;
  mct_list_t *img_buf_list;

  for (i = 0; i < num_hw_streams; i++) {
    hw_stream = iface_util_find_hw_stream_in_session(session, hw_stream_ids[i]);
    if (!hw_stream) {
      CDBG_ERROR("%s:%d failed hw_stream %p i %d\n", __func__, __LINE__,
        hw_stream, i);
      continue;
    }

    /*request image buffer based on hal buf or native buf*/
    if (!hw_stream->use_native_buf) {
      rc = iface_util_request_image_buf_hal(iface, session, hw_stream);
    } else {
      rc = iface_util_request_image_buf_native(iface, session, hw_stream);
    }

    if (rc < 0) {
      CDBG_ERROR("%s: error in request image buffer, rc = %d, "
        "sessionid = %d, hw_stream_id = %x\n",
        __func__, rc, session->session_id, hw_stream->hw_stream_id);
      return rc;
    }
  }

  return 0;
}

/** iface_util_release_image_buf
 *    @stream_ids:
 *    @num_streams:
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int iface_util_release_image_buf(
  iface_t             *iface,
  iface_session_t     *session,
  iface_hw_stream_t   *hw_stream)
{
  uint32_t user_stream_id;
  int rc = 0, i = 0, buff_id = 0, j = 0;

  pthread_mutex_lock(&session->mutex);
  if (hw_stream->streamon_count == 0) {
    if (hw_stream->shared_hw_stream) {
      for (i = 0; i < IFACE_MAX_STREAMS; i++) {
        user_stream_id = hw_stream->mapped_mct_stream_id[i];
        if (!user_stream_id)
          continue;

        buff_id = iface_util_get_buffers_id(hw_stream, user_stream_id);
        if (buff_id < 0 || buff_id >= VFE_BUF_QUEUE_MAX) {
          CDBG_ERROR("%s: can not find buf_id for stream 0x%x\n",
            __func__, user_stream_id);
          continue;
        }

        if (hw_stream->buffers_info[buff_id].bufq_handle > 0) {
          iface_release_buf(&iface->buf_mgr,
            hw_stream->buffers_info[buff_id].bufq_handle,
            hw_stream->buffers_info[buff_id].img_buffer_list);
          hw_stream->buffers_info[buff_id].bufq_handle = 0;
          hw_stream->buffers_info[buff_id].user_stream_id = 0;
          hw_stream->buffers_info[buff_id].is_mapped = FALSE;
        }
      }
    } else {
      if (hw_stream->buffers_info[VFE_BUF_QUEUE_DEFAULT].bufq_handle > 0) {
        iface_release_buf(&iface->buf_mgr,
          hw_stream->buffers_info[VFE_BUF_QUEUE_DEFAULT].bufq_handle,
          hw_stream->buffers_info[VFE_BUF_QUEUE_DEFAULT].img_buffer_list);
        hw_stream->buffers_info[VFE_BUF_QUEUE_DEFAULT].bufq_handle = 0;
        hw_stream->buffers_info[buff_id].user_stream_id = 0;
        hw_stream->buffers_info[buff_id].is_mapped = FALSE;
      }
    }
  /* if an internal stream is released it means that
     * input buffer queue is not needed anymore */

   if (session->reproc_buff_info.bufq_handle > 0) {
    CDBG("releasing bufq_handle %x, stream_id %x",
        session->reproc_buff_info.bufq_handle, session->reproc_buff_info.user_stream_id);
     iface_release_buf(&iface->buf_mgr,
       session->reproc_buff_info.bufq_handle,
       session->reproc_buff_info.img_buffer_list);
     session->reproc_buff_info.bufq_handle = 0;
     session->reproc_buff_info.user_stream_id = 0;
     session->reproc_buff_info.is_mapped = FALSE;
   }
  }

  pthread_mutex_unlock(&session->mutex);
  return rc;
}

/** iface_util_update_axi_stream
 *
 *    @ctrl:
 *    @thread:
 *    @action:
 *
 **/
int iface_util_update_axi_stream(iface_session_t *session,
  iface_t *iface, iface_stream_update_t *stream_update_parm)
{
  int rc = 0;

  if (session->session_resource.isp_id_mask & (1 << VFE0)) {
    rc = iface->isp_axi_data.axi_data[VFE0].axi_hw_ops->set_params(
      iface->isp_axi_data.axi_data[VFE0].axi_hw_ops->ctrl,
      IFACE_AXI_SET_STREAM_UPDATE, (void *)stream_update_parm, sizeof(iface_hw_stream_t));
    if (rc < 0) {
      CDBG_ERROR("%s: VFE0 ISP_AXI_SET_STREAM_CFG failed! VFE0 rc = %d\n",
        __func__, rc);
      return rc;
    }
  }

  if (session->session_resource.isp_id_mask & (1 << VFE1)) {
    rc = iface->isp_axi_data.axi_data[VFE1].axi_hw_ops->set_params(
      iface->isp_axi_data.axi_data[VFE1].axi_hw_ops->ctrl,
      IFACE_AXI_SET_STREAM_UPDATE, (void *)stream_update_parm, sizeof(iface_hw_stream_t));
    if (rc < 0) {
      CDBG_ERROR("%s: VFE1 ISP_AXI_SET_STREAM_CFG failed! VFE1 rc = %d\n", __func__,
        rc);
      return rc;
    }
  }
  return rc;
}

/** iface_util_do_axi_hw_update
 *
 *    @session:
 *    @iface:
 *
 **/
int iface_util_do_axi_hw_update(iface_session_t *session, iface_t *iface)
{
  int rc = 0;
   if (session->session_resource.isp_id_mask & (1 << VFE0)) {
     rc = iface->isp_axi_data.axi_data[VFE0].axi_hw_ops->action(
       iface->isp_axi_data.axi_data[VFE0].axi_hw_ops->ctrl,
       IFACE_AXI_ACTION_CODE_HW_UPDATE, NULL, 0);
     if (rc < 0) {
       CDBG_ERROR("%s: VFE0 IFACE_AXI_ACTION_CODE_HW_UPDATE failed! rc = %d\n",
         __func__, rc);
       return rc;
     }
   }

   if (session->session_resource.isp_id_mask & (1 << VFE1)) {
     rc = iface->isp_axi_data.axi_data[VFE1].axi_hw_ops->action(
       iface->isp_axi_data.axi_data[VFE1].axi_hw_ops->ctrl,
       IFACE_AXI_ACTION_CODE_HW_UPDATE, NULL, 0);
     if (rc < 0) {
       CDBG_ERROR("%s: VFE1 IFACE_AXI_ACTION_CODE_HW_UPDATE failed! rc = %d\n",
         __func__, rc);
       return rc;
     }
   }

   return rc;
}

/** iface_util_set_master_slave_info
 *
 *    @iface_session:
 *    @iface:
 *
 **/
int iface_util_set_master_slave_info(iface_session_t *session,
  iface_t *iface)
{
  enum msm_ispif_vfe_intf vfe_intf;
  struct msm_isp_set_dual_hw_ms_cmd ms_cmd;
  iface_axi_hw_t *axi_hw = NULL;
  int rc = 0;
  enum msm_vfe_input_src intf;
  uint32_t max_sensor_fps = 0;

  memset(&ms_cmd, 0, sizeof(ms_cmd));
  vfe_intf = session->session_resource.main_frame_vfe;
  if (vfe_intf >= VFE_MAX) {
    CDBG_ERROR("%s: Error! invalid vfe %d\n", __func__, vfe_intf);
    return -1;
  }
  axi_hw = (iface_axi_hw_t *)
    iface->isp_axi_data.axi_data[vfe_intf].axi_hw_ops->ctrl;

  ms_cmd.primary_intf = session->session_resource.main_frame_intf;
  ms_cmd.dual_hw_ms_type = session->ms_type;

  if (session->ms_type == MS_TYPE_MASTER) {
    max_sensor_fps = session->sensor_out_info.vt_pixel_clk /
       (session->sensor_out_info.ll_pck *
        session->sensor_out_info.fl_lines);
    /* Assuming both sensors work at same fps */
    ms_cmd.sof_delta_threshold = ceil((1000/(float)max_sensor_fps)/2);
    ms_cmd.num_src = 0;
  } else {
    for (intf = 0; intf < VFE_SRC_MAX; intf++) {
      if ((session->session_resource.used_resource_mask &
        (1 << (16 * vfe_intf + intf))) && intf != ms_cmd.primary_intf) {
        ms_cmd.input_src[ms_cmd.num_src] = (enum msm_vfe_input_src) intf;
        ms_cmd.num_src++;
      }
    }
  }

  rc = iface_axi_set_master_slave_info(axi_hw, &ms_cmd);
  if (rc < 0) {
    CDBG_ERROR("%s: failed! rc = %d\n",
      __func__, rc);
    return rc;
  }

#ifdef VIDIOC_MSM_ISP_DUAL_HW_MASTER_SLAVE_SYNC
  if (session->peer_session_id) {
    struct msm_isp_dual_hw_master_slave_sync sync;
    sync.sync_mode = MSM_ISP_DUAL_CAM_SYNC;
    rc = iface_axi_set_master_slave_sync(axi_hw, &sync);
    if (rc < 0) {
      CDBG_ERROR("%s: failed! iface_axi_set_master_slave_sync rc = %d\n",
        __func__, rc);
    }
  }
#endif

  /* take care of intf on other VFE than main */
  if (session->session_resource.num_isps > 1) {
    vfe_intf = (vfe_intf == VFE0) ? VFE1 : VFE0;
    axi_hw = (iface_axi_hw_t *)
      iface->isp_axi_data.axi_data[vfe_intf].axi_hw_ops->ctrl;
    memset(&ms_cmd, 0, sizeof(ms_cmd));
    ms_cmd.dual_hw_ms_type = session->ms_type;
    /* Since this is not Main VFE, there is no primary intf.
     * Hence VFE_SRC_MAX */
    ms_cmd.primary_intf = VFE_SRC_MAX;

    for (intf = 0; intf < VFE_SRC_MAX; intf++) {
      if ((session->session_resource.used_resource_mask &
        (1 << (16 * vfe_intf + intf)))) {
        ms_cmd.input_src[ms_cmd.num_src] = intf;
        ms_cmd.num_src++;
      }
    }

    rc = iface_axi_set_master_slave_info(axi_hw, &ms_cmd);
    if (rc < 0) {
      CDBG_ERROR("%s:%d] failed! setting MS info on other vfe rc = %d\n",
        __func__, __LINE__, rc);
      return rc;
    }
  }

  return rc;
}

#ifdef VIDIOC_MSM_ISP_DUAL_HW_MASTER_SLAVE_SYNC
int iface_util_set_master_slave_sync(iface_t *iface,
  iface_session_t *session,
  uint32_t sync)
{
  struct msm_isp_dual_hw_master_slave_sync ms_sync;
  iface_axi_hw_t *axi_hw = NULL;
  enum msm_ispif_vfe_intf vfe_intf;
  int rc = 0;

  if (session->session_resource.num_isps == 0)
    return 0;

  vfe_intf = session->session_resource.main_frame_vfe;
  if (vfe_intf >= VFE_MAX) {
    CDBG_ERROR("%s: Error! invalid vfe %d\n", __func__, vfe_intf);
    return -1;
  }
  axi_hw = (iface_axi_hw_t *)
    iface->isp_axi_data.axi_data[vfe_intf].axi_hw_ops->ctrl;

  ms_sync.sync_mode = sync;
  rc = iface_axi_set_master_slave_sync(axi_hw, &ms_sync);
  if (rc < 0) {
    CDBG_ERROR("%s: failed! iface_axi_set_master_slave_link rc = %d\n",
      __func__, rc);
  }
  return rc;
}
#else
int iface_util_set_master_slave_sync(iface_t *iface __attribute__((unused)),
  iface_session_t *session __attribute__((unused)),
  uint32_t sync __attribute__((unused)))
{
  return 0;
}
#endif

/** iface_util_lpm_session
 *
 *    @iface: iface handle
 *
 *    Return 0 on Success
 **/
#ifdef VIDIOC_MSM_ISP_DUAL_HW_LPM_MODE
int iface_util_lpm_session(iface_t *iface,
  iface_session_t *session,
  bool lpm_mode)
{
  iface_axi_hw_t *axi_hw = NULL;
  int vfe_intf = 0;
  int rc = 0, i = 0;
  struct msm_vfe_dual_lpm_mode ab_ib_vote;
  memset(&ab_ib_vote, 0, sizeof(ab_ib_vote));
  if (session->session_resource.num_isps == 0)
    return 0;

   for (vfe_intf = 0; vfe_intf < VFE_MAX; vfe_intf++) {
     if (session->session_resource.isp_id_mask & (1 << vfe_intf)) {
       axi_hw = (iface_axi_hw_t *)
         iface->isp_axi_data.axi_data[vfe_intf].axi_hw_ops->ctrl;
       ab_ib_vote.lpm_mode = lpm_mode;
       for (i = 0; i < VFE_AXI_SRC_MAX; i++) {
        /* stream_src is nothing but axi_src i.e PIX_ENCODER, PIX_VIWEFINDER ..etc */
         if (axi_hw->stream[i].state == IFACE_AXI_STREAM_STATE_ACTIVE) {
           ab_ib_vote.stream_src[i]
             = axi_hw->stream[i].hw_stream_info.axi_path;
           ab_ib_vote.num_src++;
         }
       }
       rc = iface_axi_lpm_session(axi_hw, &ab_ib_vote);
       if (rc < 0) {
         CDBG_ERROR("%s: failed! iface_axi_lpm_session rc = %d\n",
           __func__, rc);
       }
     }
   }
  return rc;
}
#endif

int iface_util_set_ispif_frame_drop(iface_t *iface,
  iface_session_t *session, iface_port_t *iface_sink_port, enum ispif_cfg_type_t cfg_type )
{
  uint32_t  				 hw_stream_ids[IFACE_MAX_STREAMS];
  uint32_t                   num_hw_streams = 0;
  iface_ispif_t              *ispif = &iface->ispif_data.ispif_hw;
  struct ispif_cfg_data      *cfg_cmd = &ispif->cfg_cmd;
  int                        rc = 0;

  if (!iface || !iface_sink_port) {
    CDBG_ERROR("%s: Invalid parameter\n", __func__);
    return -1;
  }

  pthread_mutex_lock(&ispif->mutex);
  memset(&hw_stream_ids, 0, sizeof(uint32_t) * IFACE_MAX_STREAMS);
  num_hw_streams = iface_util_get_hw_streams_ids_in_session(iface, session, hw_stream_ids);

  IFACE_HIGH("%s: num_hw_streams = %d\n",__func__, num_hw_streams);
  if (cfg_type == ISPIF_STOP_FRAME_BOUNDARY)
  {
    rc = iface_ispif_get_cfg_params_for_framedrop(iface, iface_sink_port,
      session, num_hw_streams, hw_stream_ids);
  }
  else
  {
    rc = iface_ispif_get_cfg_params_for_framedrop(iface, iface_sink_port,
      session, num_hw_streams, hw_stream_ids);
  }

  if (rc == 0)
  {
    if (cfg_cmd->params.num == 0) {
      IFACE_ERR("ISPIF configure cmd is seriously wrong\n");
      rc = -1;
    }
  }

  if (rc == 0)
  {
    cfg_cmd->cfg_type = cfg_type;
    rc = ioctl(ispif->fd, VIDIOC_MSM_ISPIF_CFG, cfg_cmd);
    if (rc != 0) {
      IFACE_ERR("STOP/START ISPIF at frame boundary succeeded, cfg_type = %d, rc = %d\n", cfg_type, rc);
    }
    else {
      IFACE_HIGH("STOP/START ISPIF at frame boundary succeeded, cfg_type = %d\n", cfg_type);
    }
  }
  pthread_mutex_unlock(&ispif->mutex);

  return rc;
}

/** iface_util_axi_reg_shared_bufq
 *
 *    @iface_session:
 *    @iface:
 *    @hw_stream_id:
 *    @user_stream_id:
 *
 **/
int iface_util_axi_reg_shared_bufq(iface_session_t *session, iface_t *iface,
  uint32_t hw_stream_id, uint32_t user_stream_id)
{
  iface_reg_buf_t reg_buf;
  int rc = 0;

  memset(&reg_buf, 0, sizeof(reg_buf));
  reg_buf.session_id = session->session_id;
  reg_buf.hw_stream_id = hw_stream_id;
  reg_buf.user_stream_id = user_stream_id;

  if (session->session_resource.isp_id_mask & (1 << VFE0)) {
    rc = iface->isp_axi_data.axi_data[VFE0].axi_hw_ops->action(
      iface->isp_axi_data.axi_data[VFE0].axi_hw_ops->ctrl,
      IFACE_AXI_ACTION_CODE_REG_BUF, (void *)&reg_buf, sizeof(iface_reg_buf_t));
    if (rc < 0) {
       if (session->session_resource.isp_id_mask & (1 << VFE1)) {
          CDBG_ERROR("%s: VFE0 reg buffer failure, check if VFE1");
       } else {
       CDBG_ERROR("%s: VFE0 IFACE_AXI_ACTION_CODE_REG_BUF failed! rc = %d\n",
          __func__, rc);
       return rc;
       }
    }
  }

  if (session->session_resource.isp_id_mask & (1 << VFE1)) {
    rc = iface->isp_axi_data.axi_data[VFE1].axi_hw_ops->action(
      iface->isp_axi_data.axi_data[VFE1].axi_hw_ops->ctrl,
      IFACE_AXI_ACTION_CODE_REG_BUF, (void *)&reg_buf, sizeof(iface_reg_buf_t));
    if (rc < 0) {
      CDBG_ERROR("%s: VFE1 IFACE_AXI_ACTION_CODE_REG_BUF failed! rc = %d\n",
        __func__, rc);
      return rc;
    }
  }

  return rc;
}

/** iface_util_axi_unreg_shared_bufq
 *
 *    @ctrl:
 *    @thread:
 *    @action:
 *
 **/
int iface_util_axi_unreg_shared_bufq(iface_session_t *session, iface_t *iface,
  uint32_t hw_stream_id, uint32_t user_stream_id)
{
  iface_reg_buf_t reg_buf;
  int rc = 0;

  memset(&reg_buf, 0, sizeof(reg_buf));
  reg_buf.session_id = session->session_id;
  reg_buf.hw_stream_id = hw_stream_id;
  reg_buf.user_stream_id = user_stream_id;

  if (session->session_resource.isp_id_mask & (1 << VFE0)) {
    rc = iface->isp_axi_data.axi_data[VFE0].axi_hw_ops->action(
      iface->isp_axi_data.axi_data[VFE0].axi_hw_ops->ctrl,
      IFACE_AXI_ACTION_CODE_UNREG_BUF, (void *)&reg_buf, sizeof(iface_reg_buf_t));
    if (rc < 0) {
      CDBG_ERROR("%s: VFE0 IFACE_AXI_ACTION_CODE_REG_BUF failed! rc = %d\n",
        __func__, rc);
      return rc;
    }
  }

  if (session->session_resource.isp_id_mask & (1 << VFE1)) {
    rc = iface->isp_axi_data.axi_data[VFE1].axi_hw_ops->action(
      iface->isp_axi_data.axi_data[VFE1].axi_hw_ops->ctrl,
      IFACE_AXI_ACTION_CODE_UNREG_BUF, (void *)&reg_buf, sizeof(iface_reg_buf_t));
    if (rc < 0) {
      CDBG_ERROR("%s: VFE1 IFACE_AXI_ACTION_CODE_REG_BUF failed! rc = %d\n",
        __func__, rc);
      return rc;
    }
  }

  return rc;
}
/** iface_util_uv_subsample
 *
 *    @ctrl:
 *    @thread:
 *    @action:
 *
 **/
int iface_util_uv_subsample(iface_session_t *session,
  uint32_t uv_subsample_enb)
{
  int rc = 0;
  int i = 0;
  iface_stream_update_t stream_update_parm;
  iface_hw_stream_t *hw_stream = NULL;
  iface_t *iface = (iface_t *)session->iface;
    uint32_t update_hw_stream_ids[IFACE_MAX_STREAMS];

  CDBG("%s: E, updating AXI\n", __func__);

  memset(&stream_update_parm, 0, sizeof(iface_stream_update_t));
  /*set uv subsample enable*/
  stream_update_parm.uv_subsample_enb = uv_subsample_enb;

  /*find out the streams need to be update*/
  stream_update_parm.session_id = session->session_id;
  for (i = 0; i < IFACE_MAX_STREAMS; i++) {
    hw_stream = &session->hw_streams[i];
    if ((hw_stream != NULL) && (hw_stream->hw_stream_id != 0) &&
      (hw_stream->state != IFACE_HW_STREAM_STATE_INITIAL)) {
      /* uv subsample only update pix streams*/
       if (hw_stream->use_pix == 1)
        stream_update_parm.hw_stream_ids[stream_update_parm.num_hw_streams++]
          = hw_stream->hw_stream_id;
    }
  }

  rc = iface_util_update_axi_stream(session, iface, &stream_update_parm);
  if (rc < 0) {
   CDBG_ERROR("%s: error iface_util_update_axi_stream! rc = %d\n", __func__, rc);
    return rc;
  }

  /* action to do hw update: flexible to move the hw update to any timing:
     thus we seperate hw update from set parm*/
  rc = iface_util_do_axi_hw_update(session, iface);
  if (rc < 0) {
   CDBG_ERROR("%s: error iface_util_do_axi_hw_update! rc = %d\n", __func__, rc);
    return rc;
  }

  CDBG("%s: X, updating AXI done!\n", __func__);
  return rc;
}

/** iface_util_update_streamon_id
 *    @session: isp session
 *    @stream_id: Stream ID
 *    @stream_ids: Stream ID
 *
 *  Gets IDs of streams from same HAL bunfle as given stream.
 **/
boolean iface_util_update_streamon_id(iface_hw_stream_t *hw_stream,
 uint32_t user_streamon_id, uint32_t is_streamon)
{
   boolean is_update = FALSE;

   /* track which user stream trigger the hw stream on
      streamon check if its cfg_done, or skip streamon
      streamoff check if its active, or skip streamoff*/
   if (is_streamon) {
     if (hw_stream->state == IFACE_HW_STREAM_STATE_HW_CFG_DONE) {
       hw_stream->mct_streamon_id = user_streamon_id;
       is_update = TRUE;
     } else if (hw_stream->shared_hw_stream &&
                hw_stream->state == IFACE_HW_STREAM_STATE_ACTIVE) {
       hw_stream->mct_streamon_id = user_streamon_id;
       is_update = FALSE;
     } else {
       CDBG_HIGH("%s:skip, hw stream state = %d, hw stream id = %x, mct streamon id = %x\n",
          __func__, hw_stream->state, hw_stream->hw_stream_id, hw_stream->mct_streamon_id);
       is_update = FALSE;
     }
   } else {
     if (hw_stream->streamon_count == 0) {
       is_update = TRUE;
       /* stop stream is including un config axi,
          so we will need to prepare the hw stream id
          even stream may not be active*/
       if ((hw_stream->state != IFACE_HW_STREAM_STATE_STOPPING) &&
           (hw_stream->state != IFACE_HW_STREAM_STATE_ACTIVE)) {
         CDBG_ERROR("%s: invalid hw stream %x state %d, enforce streamoff!\n",
                    __func__, hw_stream->hw_stream_id, hw_stream->state);
       }
     } else {
       CDBG_HIGH("%s: left stream on count %d, skip hw streamoff\n",
         __func__, hw_stream->streamon_count);
       is_update = FALSE;
     }
   }

  return is_update;
}

/** iface_util_check_hw_stream_list
 *    @num_hw_Stream: number_hw_stream in list
 *    @hw_stream_ids: hw Stream ID list
 *    @hw_stream_id: checking hw Stream ID
 *
 *  Gets IDs of streams from same HAL bunfle as given stream.
 **/
boolean iface_util_check_hw_stream_list(uint32_t num_hw_streams,
  uint32_t *hw_stream_ids, uint32_t check_hw_stream_id)
{
  uint32_t i = 0;
  boolean is_hw_stream_in_list = FALSE;

  /* check if the hw stream is already in stream ids*/
  for (i = 0; i < num_hw_streams; i++) {
    if (hw_stream_ids[i] == check_hw_stream_id) {
      is_hw_stream_in_list = TRUE;
      break;
    }
  }

  return is_hw_stream_in_list;
}

/** iface_util_get_hw_streams_ids_in_session
 *    @iface: iface pointer
 *    @session: iface session
 *    @hw_stream_ids: output array of hw_stream_ids to be filled
 *
 *  Gets IDs of streams in session
 **/
uint32_t iface_util_get_hw_streams_ids_in_session(
  iface_t         *iface __unused,
  iface_session_t *session,
  uint32_t        *hw_stream_ids)
{
  int k;
  uint32_t num_hw_streams = 0;
  iface_hw_stream_t *hw_stream = NULL;

  for (k = 0; k < IFACE_MAX_STREAMS; k++) {
    hw_stream = &session->hw_streams[k];
    if (hw_stream->state == IFACE_HW_STREAM_STATE_ACTIVE ||
      hw_stream->state == IFACE_HW_STREAM_STATE_STOPPING) {
      hw_stream_ids[num_hw_streams++] = hw_stream->hw_stream_id;
    }
  }

  return num_hw_streams;
}

/** iface_util_decide_hw_streams
 *    @session: isp session
 *    @stream_id: Stream ID
 *    @stream_ids: Stream ID
 *
 *  Gets IDs of streams from same HAL bunfle as given stream.
 **/
uint32_t iface_util_decide_hw_streams(
  iface_t         *iface __unused,
  iface_session_t *session,
  uint32_t         num_user_streams,
  uint32_t        *user_stream_ids,
  uint32_t        *hw_stream_ids,
  uint32_t         is_streamon)
{
  int32_t            rc = 0;
  uint32_t           num_hw_streams = 0;
  uint32_t           i = 0,
                     j = 0,
                     k =0;
  iface_stream_t    *stream;
  iface_hw_stream_t *hw_stream = NULL;
  boolean            is_hw_stream_in_list = FALSE;
  boolean            is_update_streamon_id = FALSE;

  /*
     * 1.if not bundled, hw stream id is only one, find in mapped mct stream id
     * 2. if bundled and break means got all bundled stream,
     *   get hw stream id but mct id
     */
  for (i = 0; i < num_user_streams; i++) {
    /*loop through all hw streams to find all associated with this user stream*/
    for (k = 0; k < IFACE_MAX_STREAMS; k++) {
      hw_stream = NULL;
      /*check if this user stream in the mapped streams*/
      for (j = 0; j < session->hw_streams[k].num_mct_stream_mapped; j++) {
        if (session->hw_streams[k].mapped_mct_stream_id[j] ==
            user_stream_ids[i]) {
          hw_stream = &session->hw_streams[k];
          break;
        }
      }

      /*mapped stream not match with this stream*/
      if (hw_stream == NULL)
        continue;

      /*stream off will decrease streamon cnt
        associated with this hw steream */
      if (is_streamon)
        hw_stream->streamon_count++;
      else
        hw_stream->streamon_count--;

      /*
           * update mct stream on id in hw stream,
           * 1. set id when stream on, reset id when streamoff.
           * 2. if the stream on id not get update,
           *  means no need to hw streamon/off
           */
      is_update_streamon_id = iface_util_update_streamon_id(hw_stream,
        user_stream_ids[i], is_streamon);

      /*
           * if not update streamon id, means already hw streamon by other stream
           * do not add this stream into hw streamoff list
           */
      if (is_update_streamon_id == FALSE)
        continue;

      /*
           * check if the hw stream is already in stream ids
           * if not, then add to hw stream list
           */
      if (iface_util_check_hw_stream_list(num_hw_streams,
          &hw_stream_ids[0], hw_stream->hw_stream_id) == FALSE) {
        hw_stream_ids[num_hw_streams++] = hw_stream->hw_stream_id;
        CDBG("%s: num_hw_streams %d id %x\n", __func__,
          num_hw_streams, hw_stream->hw_stream_id);
      }
    }
  }

  CDBG_HIGH("%s: num_user_streams %d num_hw_streams %d\n", __func__,
    num_user_streams, num_hw_streams);
  return num_hw_streams;
}

/** iface_util_get_hw_streams_by_bundle
 *    @session: isp session
 *    @stream_id: Stream ID
 *    @stream_ids: Stream ID
 *
 *  Gets IDs of streams from same HAL bunfle as given stream.
 **/
int iface_util_get_user_streams_by_bundle(iface_session_t *session,
  uint32_t stream_id, uint32_t *user_stream_ids, uint32_t is_streamon)
{
  int num_user_streams = 0, k = 0;
  int rc = 0;
  int j = 0;
  uint32_t i;
  iface_stream_t *stream;
  uint32_t is_bundled = 0;
  uint32_t is_hw_stream_reserved = 0;
  iface_hw_stream_t *hw_stream;
  uint32_t streamon_all_bundled = 0;
  uint32_t streamoff_all_bundled = 0;

  /* find out bundle mct streamon ids*/
  for (i = 0; i < session->hal_bundle_parm.num_of_streams
    && i < MAX_STREAM_NUM_IN_BUNDLE; i++) {
     if (stream_id == session->hal_bundle_parm.stream_ids[i]) {
        is_bundled = 1;
        /* for Streamon: if got all bundled streams, then break and continue processing
           for streamoff: the first stream off all the bundled streamoff*/
        if (is_streamon) {
          session->num_bundled_streamon++;
          /* streamon case*/
          if (session->num_bundled_streamon ==
            session->hal_bundle_parm.num_of_streams) {
          IFACE_HIGH("streamon, got all bundled streamon!\n");
          streamon_all_bundled = 1;
          break;
          } else {
            IFACE_HIGH("got No.%d bundled streamon, mct stream id %d"
              "total bundled stream num = %d\n", session->num_bundled_streamon,
              stream_id, session->hal_bundle_parm.num_of_streams);
            return 0;
          }
        } else {
          /* stream off case */
          if (session->num_bundled_streamon == session->hal_bundle_parm.num_of_streams) {
             streamoff_all_bundled = 1;
             session->num_bundled_streamon--;
             IFACE_HIGH("streamoff, first bundled streamoff all bindled streams!\n")
             break;
          } else {
            session->num_bundled_streamon--;
            IFACE_HIGH("skip streamoff! left %d bundled streamoff, mct stream id %d"
              "total bundled stream num = %d\n",
              session->num_bundled_streamon, stream_id, session->hal_bundle_parm.num_of_streams);
            return 0;
          }
        }
     }
  }

  /* 1.if not bundled then hw stream id is only one, find in mapped mct stream id
     2. if bundled and break means got all bundled stream, get hw stream id but mct id*/
  if (is_bundled == 0) {
    num_user_streams = 1;
    user_stream_ids[0] = stream_id;
  } else{
     if (streamon_all_bundled || streamoff_all_bundled) {
       num_user_streams = session->hal_bundle_parm.num_of_streams;
       if (num_user_streams > MAX_STREAM_NUM_IN_BUNDLE) {
         IFACE_ERR("failed: num_user_streams %d max %d\n",
           num_user_streams, MAX_STREAM_NUM_IN_BUNDLE);
         return 0;
       }
       for (k = 0; k < num_user_streams && k < MAX_STREAM_NUM_IN_BUNDLE; k++)
         user_stream_ids[k] = session->hal_bundle_parm.stream_ids[k];
     }
  }

  return num_user_streams;
}

/** iface_util_set_bundle:
 *    @iface: iface instance
 *    @iface_sink_port: iface sink port
 *    @session_id: session id
 *    @stream_id: stream id
 *    @bundle_param: HAL bundle params
 *
 *  This function runs in MCTL thread context.
 *
 *  This function sets iface bundling mask according to bundle configuration
 *  by output mask
 *
 *  Return: 0 - Success
 *         -1 - CAnnot find session or stream
 **/
int iface_util_set_bundle(
  iface_t             *iface __unused,
  iface_port_t        *iface_sink_port __unused,
  iface_session_t     *session,
  uint32_t             stream_id __unused,
  cam_bundle_config_t *bundle_param)
{
  int rc = 0;
  iface_stream_t *stream;

  if (!session) {
    CDBG_ERROR("%s: cannot find session\n", __func__);
    return -1;
  }

  session->hal_bundle_parm = *bundle_param;
  return rc;
}

/** iface_util_stats_ack
 *    @isp: ISP pointer
 *    @hw_idx: vfe hw index
 *    @num_streams: notify type
 *
 * TODO
 *
 * Return: nothing
 **/
int iface_util_stats_ack(iface_t *iface, iface_session_t *session,
  uint32_t user_stream_id, iface_raw_stats_buf_info_t *stats_buf_ack)
{
  int rc = 0;
  iface_hw_stream_t *hw_stream = NULL;

  if (session == NULL) {
    CDBG_ERROR("%s: can not find session, rc = -1\n", __func__);
    return -1;
  }

  hw_stream =
    iface_util_find_hw_stream_by_mct_id(iface, session, user_stream_id);
  if (hw_stream == NULL) {
    CDBG_ERROR("%s: can not find hw stream by mct stream id %d, sess id %d, rc = -1\n",
      __func__, user_stream_id, session->session_id);
    return -1;
  }

  if (session->session_resource.ispif_split_info.is_split) {
    /*if split then we are suing shared buf, only enqueue once*/
    rc = iface->isp_axi_data.axi_data[VFE0].axi_hw_ops->action(
      iface->isp_axi_data.axi_data[VFE0].axi_hw_ops->ctrl,
      IFACE_AXI_ACTION_CODE_STATS_NOTIFY_ACK, (void *)stats_buf_ack,
      sizeof(iface_raw_stats_buf_info_t));
    if (rc < 0) {
      CDBG_ERROR("%s: VFE0 STATS_NOTIFY_ACK failed! rc = %d\n",
        __func__, rc);
      return rc;
    }
  } else {
     /*received divert buf ack from cpp, enque buf back to kernel for reuse*/
     if ((hw_stream->interface_mask & (0xffff << (16* VFE0))) != 0) {
       rc = iface->isp_axi_data.axi_data[VFE0].axi_hw_ops->action(
         iface->isp_axi_data.axi_data[VFE0].axi_hw_ops->ctrl,
         IFACE_AXI_ACTION_CODE_STATS_NOTIFY_ACK, (void *)stats_buf_ack,
         sizeof(iface_raw_stats_buf_info_t));
       if (rc < 0) {
         CDBG_ERROR("%s: VFE0 STATS_NOTIFY_ACK failed! rc = %d\n",
           __func__, rc);
         return rc;
       }
     }

     if ((hw_stream->interface_mask & (0xffff << (16* VFE1))) != 0) {
       rc = iface->isp_axi_data.axi_data[VFE1].axi_hw_ops->action(
         iface->isp_axi_data.axi_data[VFE1].axi_hw_ops->ctrl,
         IFACE_AXI_ACTION_CODE_STATS_NOTIFY_ACK, (void *)stats_buf_ack,
         sizeof(iface_raw_stats_buf_info_t));
       if (rc < 0) {
         CDBG_ERROR("%s: VFE1 STATS_NOTIFY_ACK failed! rc = %d\n", __func__,
           rc);
         return rc;
       }
     }
  }

  return rc;
}

/** iface_util_adjust_paramq
 *    @iface: iface private data
 *    @session_id: session id
 *    @user_stream_id: stream id
 *    @skip_frame_id: frame id for skip
 *    @frame_skip_num: num of frames to skip
 *
 *  Handle frame skip event. IFACE module will not skip frame for streams
 *  which needs divert. IFACE will postpone only applying of supper param.
 *  As result IFACE will output frames for streams which does not require
 *  divert one frame later. This is equal to one frame skip. This
 *  functionality will work only for HAL3.
 *
 *  Return: 0 - Success
 *         -1 - Cannot find session
 **/
int iface_util_adjust_paramq(
  iface_t         *iface __unused,
  iface_session_t *session,
  uint32_t         user_stream_id __unused,
  uint32_t         skip_frame_id,
  uint32_t         frame_skip_num)
{
  mct_queue_t            *parm_q_tmp;
  int32_t                 cnt, src_idx, dst_idx, free_q_idx;
  int32_t                 latest_q_idx, current_q_idx, previous_q_idx;
  int                     rc = 0;
  uint32_t                l_skip_frame_id = skip_frame_id;
  iface_axi_hw_t  *axi_hw = NULL;

  if (!session) {
    CDBG_ERROR("%s: cannot find session \n", __func__);
    return -1;
  }

  if (frame_skip_num == 0) {
    CDBG_LOW("Adjustment not needed\n");
    return 0;
  }

  /* In case of dual cam drop, we need to shift the dropped frame requests
  to the current frame */
  skip_frame_id = skip_frame_id - session->dual_cam_drop;
  /*This is the max index where a pending request migt reside*/
  latest_q_idx = (skip_frame_id +
    session->max_apply_delay) % IFACE_FRAME_CTRL_SIZE;

  current_q_idx = (skip_frame_id +
    IFACE_APPLY_DELAY) % IFACE_FRAME_CTRL_SIZE;
  previous_q_idx = ((skip_frame_id - 1) +
    IFACE_APPLY_DELAY) % IFACE_FRAME_CTRL_SIZE;

  IFACE_DBG("1. skip_frame_id: %d latest_q_idx: %d \
    current_q_idx: %d previous_q_idx: %d\n",
    skip_frame_id, latest_q_idx, current_q_idx,
    previous_q_idx);

  cnt = 0;
  free_q_idx = latest_q_idx;
  /* After below loop exit, updated free_q_idx
   * willbe first free entry*/
  while ((session->parm_q[free_q_idx]->length) &&
    (cnt < (IFACE_FRAME_CTRL_SIZE - 2))) {
    free_q_idx++;
    free_q_idx %= IFACE_FRAME_CTRL_SIZE;
    cnt++;
  }

  /* After below loop exit, updated latest_q_idx
   * willbe first filled entry to move*/
  cnt = 0;
  while ((session->parm_q[latest_q_idx]->length == 0) &&
    (latest_q_idx != current_q_idx) &&
    (cnt < (IFACE_FRAME_CTRL_SIZE - 2))) {
    latest_q_idx--;
    if (latest_q_idx < 0)
      latest_q_idx = IFACE_FRAME_CTRL_SIZE - 1;
    cnt++;
  }

  IFACE_DBG("2. free_q_idx: %d newlatest_q_idx: %d \
    req_delay:%d \n", free_q_idx, latest_q_idx,
    frame_skip_num);

  free_q_idx =
    (free_q_idx + frame_skip_num - 1) % IFACE_FRAME_CTRL_SIZE;
  /* Check for frame skip overlap */
  if (free_q_idx == current_q_idx) {
    CDBG_ERROR("Cannot handle huge delay: %d in request\n",
      frame_skip_num);
    return 0;
  }

  /*free_q_idx is delayed free idx  Entry*/
  dst_idx = free_q_idx;
  /*latest_q_idx is first available Entry*/
  src_idx = latest_q_idx;

  IFACE_DBG("3. New free_q_idx: dst_idx: %d  src_idx: %d \
    prev_q_idx: %d \n", dst_idx, src_idx, previous_q_idx);

  if (dst_idx == src_idx) {
     CDBG_ERROR("3. New free_q_idx: dst_idx: %d src_idx: %d \
      prev_q_idx: %d are equal !\n", dst_idx, src_idx, previous_q_idx);
     return 0;
  }

  while(src_idx != previous_q_idx) {

    pthread_mutex_lock(&session->parm_q_lock[src_idx]);
    pthread_mutex_lock(&session->parm_q_lock[dst_idx]);

    parm_q_tmp = session->parm_q[src_idx];
    session->parm_q[src_idx] = session->parm_q[dst_idx];
    session->parm_q[dst_idx] = parm_q_tmp;

    pthread_mutex_unlock(&session->parm_q_lock[dst_idx]);
    pthread_mutex_unlock(&session->parm_q_lock[src_idx]);
    IFACE_HIGH("4. Swap q_idx[%d] <--> q_idx[%d]\n",
      dst_idx, src_idx);
    dst_idx = dst_idx - 1;
    if (dst_idx < 0)
      dst_idx = IFACE_FRAME_CTRL_SIZE - 1;

    src_idx = src_idx - 1;
    if (src_idx < 0)
      src_idx = IFACE_FRAME_CTRL_SIZE - 1;

  }

  return 0;
}

/** iface_util_handle_frame_skip_event
 *    @iface: iface private data
 *    @session_id: session id
 *    @user_stream_id: stream id
 *    @skip_frame_id: frame id for skip
 *
 *  Handle frame skip event. IFACE module will not skip frame for streams
 *  which needs divert. IFACE will postpone only applying of supper param.
 *  As result IFACE will output frames for streams which does not require
 *  divert one frame later. This is equal to one frame skip. This
 *  functionality will work only for HAL3.
 *
 *  Return: 0 - Success
 *         -1 - Cannot find session
 **/
int iface_util_handle_frame_skip_event(
  iface_t         *iface __unused,
  iface_session_t *session,
  uint32_t         user_stream_id __unused,
  uint32_t         skip_frame_id)
{
  mct_queue_t            *parm_q_tmp;
  int32_t                 q_idx_skip, q_idx_tmp, cnt, src_idx, dst_idx;
  int                     rc = 0;

  if (!session) {
    CDBG_ERROR("%s: cannot find session \n", __func__);
    return -1;
  }

  if (skip_frame_id < session->max_apply_delay) {
    CDBG_ERROR("%s: cannot skip frame %d\n", __func__, skip_frame_id);
    return -1;
  }

  skip_frame_id -= IFACE_APPLY_DELAY;
  q_idx_skip = skip_frame_id % IFACE_FRAME_CTRL_SIZE;
  q_idx_tmp = (q_idx_skip + 1) % IFACE_FRAME_CTRL_SIZE;

  cnt = 0;
  /* find first free entry */
  while ((session->parm_q[q_idx_tmp]->length) &&
    (cnt < (IFACE_FRAME_CTRL_SIZE - 2))) {
    q_idx_tmp++;
    q_idx_tmp %= IFACE_FRAME_CTRL_SIZE;
    cnt++;
  }

  if (q_idx_tmp == ((q_idx_skip + 1) % IFACE_FRAME_CTRL_SIZE)) {
    /* Nothing in queue */
    return 0;
  }

  dst_idx = q_idx_tmp;
  while (dst_idx != q_idx_skip) {
    src_idx = dst_idx - 1;
    if (src_idx < 0)
      src_idx = IFACE_FRAME_CTRL_SIZE - 1;

    pthread_mutex_lock(&session->parm_q_lock[src_idx]);
    pthread_mutex_lock(&session->parm_q_lock[dst_idx]);

    parm_q_tmp = session->parm_q[src_idx];
    session->parm_q[src_idx] = session->parm_q[dst_idx];
    session->parm_q[dst_idx] = parm_q_tmp;

    pthread_mutex_unlock(&session->parm_q_lock[dst_idx]);
    pthread_mutex_unlock(&session->parm_q_lock[src_idx]);

    dst_idx = src_idx;
  }

  return 0;
}

/** iface_util_update_cds
 *
 *    @ctrl:
 *    @thread:
 *    @action:
 *
 **/
int iface_util_update_cds(iface_t *iface, uint32_t session_id,
  uint32_t user_stream_id)
{
  iface_session_t    *session = NULL;
  mct_port_t         *src_port = NULL;
  iface_stream_t     *user_stream = NULL;
  mct_event_t         event;
  mct_bus_msg_t       bus_msg;
  uint32_t            skip_frame_id;
  boolean             ret;
  int                 rc = 0;
  mct_bus_msg_delay_dequeue_t   bus_delay_msg;

  if (!iface) {
    CDBG_ERROR("%s: null pointer iface\n", __func__);
    return -1;
  }

  session = iface_util_get_session_by_id(iface, session_id);
  if (!session) {
    CDBG_ERROR("%s: can not find session, id = %d\n", __func__, session_id);
    return -1;
  }

  if (session->cds_status != IFACE_CDS_STATUS_UPDATE) {
    CDBG("%s: No CDS update required", __func__);
    return 0;
  }

  /* Change of CDS state require reconfiguration of WMs. This cause one
   * frame skip. We need to send bus message to notify MCT. MCT need know
   * about frame skip in order to postpone next super param. */
  bus_delay_msg.curr_frame_id = session->sof_frame_id;
  bus_delay_msg.delay_numframes = 1;

  if (session->hal_version == CAM_HAL_V3) {
    memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
    bus_msg.sessionid = session_id;
    bus_msg.type = MCT_BUS_MSG_FRAME_SKIP;
    bus_msg.size = sizeof(mct_bus_msg_delay_dequeue_t);
    bus_msg.msg = &bus_delay_msg;
    if (TRUE != mct_module_post_bus_msg(iface->module, &bus_msg)) {
      CDBG_ERROR("%s:%d post to bus error\n", __func__, __LINE__);
    }

    user_stream = iface_util_find_stream_in_sink_port_list(iface, session_id,
      user_stream_id);
    if (!user_stream) {
      CDBG_ERROR("%s: cannot find user_stream, session_id = %d, stream_id = %x",
        __func__, session_id, user_stream_id);
      return -1;
    }

    skip_frame_id = session->sof_frame_id + session->max_apply_delay;

    memset(&event, 0, sizeof(mct_event_t));
    event.type = MCT_EVENT_MODULE_EVENT;
    event.identity = pack_identity(session_id, user_stream_id);
    event.direction = MCT_EVENT_DOWNSTREAM;
    event.u.module_event.type = MCT_EVENT_MODULE_FRAME_SKIP_NOTIFY;
    event.u.module_event.current_frame_id = session->sof_frame_id;
    event.u.module_event.module_event_data = &skip_frame_id;

    /* Post event to downstream event */
    ret = mct_port_send_event_to_peer(user_stream->src_port, &event);
    if (ret == FALSE) {
      CDBG_ERROR("%s:%d post module msg error\n", __func__, __LINE__);
      return -1;
    }

    rc = iface_util_handle_frame_skip_event(iface, session, user_stream_id,
      skip_frame_id);
    if (rc < 0) {
      CDBG_ERROR("%s:%d failed: iface_util_handle_frame_skip_event\n",
        __func__, __LINE__);
      return -1;
    }
  }

  rc = iface_util_request_isp_cds_update(iface, session);
  if (rc < 0) {
    CDBG_ERROR("%s: iface_util_request_isp_cds_update CDS on failed! rc = %d",
      __func__, rc);
    return rc;
  }
  session->cds_status = IFACE_CDS_STATUS_REQUST_SEND;

  return rc;
}

/** iface_util_divert_ack
 *    @isp: ISP pointer
 *    @hw_idx: vfe hw index
 *    @num_streams: notify type
 *
 * TODO
 *
 * Return: nothing
 **/
int iface_util_divert_ack(iface_t *iface, iface_session_t *session,
  uint32_t user_stream_id, uint32_t buf_idx, uint32_t is_dirty,
  boolean bayerdata, uint32_t buffer_access)
{
  int rc = 0;
  iface_hw_stream_t *hw_stream = NULL;
  iface_axi_buf_divert_ack_t axi_divert_ack;
  uint32_t i;
  if (session == NULL) {
    CDBG_ERROR("%s: can not find session, rc = -1\n", __func__);
    return -1;
  }

  if(bayerdata) {
    for (i = 0; i < session->num_hw_stream; i++) {
       hw_stream = &session->hw_streams[i];
       CDBG("checking if %x is rdi_stream", hw_stream->hw_stream_id);
#ifdef ISP_SVHDR_IN_BIT
       if(hw_stream->hw_stream_id & ISP_SVHDR_IN_BIT) break;
#endif
    }
  } else {
    CDBG("user_stream_id %x session->session_id %x", user_stream_id, session->session_id);
    hw_stream =
      iface_util_find_hw_stream_by_mct_id(iface, session, user_stream_id);
  }
  if (hw_stream == NULL) {
    CDBG_ERROR("%s: can not find hw stream by mct stream id %d, sess id %d, rc = -1\n",
      __func__, user_stream_id, session->session_id);
    return -1;
  }

   memset(&axi_divert_ack, 0, sizeof(iface_axi_buf_divert_ack_t));
   /* prepare axi divert ack, communicate to AXI with HW stream id*/
   axi_divert_ack.session_id = session->session_id;
   axi_divert_ack.stream_id = hw_stream->hw_stream_id;
   axi_divert_ack.buf_idx = buf_idx;
   axi_divert_ack.is_buf_dirty = is_dirty;

   axi_divert_ack.buffer_access |= buffer_access;

  /* TODO:
   * Use direct buf_mgr api instead of axi since buf_mgr is higher layer than axi
   */
   if (hw_stream->isp_split_output_info.is_split == TRUE &&
      hw_stream->stream_info.cam_stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) {
     /*if hw stream split, only enqueue once since its shared buf*/
     rc = iface_axi_divert_ack(
      iface->isp_axi_data.axi_data[VFE0].axi_hw_ops->ctrl,
      &axi_divert_ack, sizeof(axi_divert_ack));
     if (rc < 0) {
       CDBG_ERROR("%s: VFE0 IFACE_AXI_ACTION_CODE_STREAM_DIVERT_ACK failed! rc = %d\n",
         __func__, rc);
       return rc;
     }
  } else {
     /*received divert buf ack from cpp, enque buf back to kernel for reuse*/
     if ((hw_stream->interface_mask & (0xffff << (16* VFE0))) != 0 &&
        iface->isp_axi_data.axi_data[VFE0].axi_hw_ops != NULL) {
       rc = iface_axi_divert_ack(
         iface->isp_axi_data.axi_data[VFE0].axi_hw_ops->ctrl,
         &axi_divert_ack, sizeof(axi_divert_ack));
       if (rc < 0) {
         CDBG_ERROR("%s: VFE0 IFACE_AXI_ACTION_CODE_STREAM_DIVERT_ACK failed! rc = %d\n",
           __func__, rc);
         return rc;
       }
     }

     if ((hw_stream->interface_mask & (0xffff << (16* VFE1))) != 0 &&
        iface->isp_axi_data.axi_data[VFE1].axi_hw_ops != NULL) {
       rc = iface_axi_divert_ack(
         iface->isp_axi_data.axi_data[VFE1].axi_hw_ops->ctrl,
         &axi_divert_ack, sizeof(axi_divert_ack));
       if (rc < 0) {
         CDBG_ERROR("%s: VFE1 IFACE_AXI_ACTION_CODE_STREAM_DIVERT_ACK failed! rc = %d\n", __func__,
           rc);
         return rc;
       }
     }
  }

  return rc;
}

/** iface_util_dump_hw_stream_output
 *
 * DESCRIPTION: debug message for isp channel info
 *
 **/
void iface_util_dump_hw_stream_output(iface_hw_stream_t *hw_stream)
{
  int i;

  if (hw_stream == NULL) {
     CDBG_ERROR("%s: hw stream NULL!\n", __func__);
     return;
  }

  CDBG("%s: session_id = %d, hw_stream_id = %x\n",
    __func__, hw_stream->session_id, hw_stream->hw_stream_id);
  CDBG("%s: Width = %d, Height = %d\n", __func__,
    hw_stream->stream_info.dim.width, hw_stream->stream_info.dim.height);
  CDBG("%s: plane format %d, num_plane %d\n", __func__,
    hw_stream->stream_info.fmt, hw_stream->plane_info.num_planes);

  for (i = 0; i < hw_stream->plane_info.num_planes
    && i < MAX_STREAM_PLANES; i++) {
    CDBG("%s: stride = %d, scanline = %d\n",
      __func__, hw_stream->plane_info.strides[i],
      hw_stream->plane_info.scanline[i]);
  }
}

/** iface_util_send_hw_stream_output_dim_downstream_int
 *    @session:
 *    @stream:
 *
 * TODO
 *
 * Return: nothing
 **/
void iface_util_send_hw_stream_output_dim_downstream_int(iface_t *iface,
  iface_session_t *session, iface_hw_stream_t *hw_stream)
{
  mct_event_t mct_event;
  mct_stream_info_t stream_info;
  iface_stream_t *user_stream = NULL;
  uint32_t i = 0 , j = 0;

  iface_util_dump_hw_stream_output(hw_stream);

  for (i = 0; i < hw_stream->num_mct_stream_mapped
    && i < IFACE_MAX_STREAMS; i++) {
     CDBG("%s: mapped mct stream id %d\n", __func__,
       hw_stream->mapped_mct_stream_id[i]);
     user_stream = iface_util_find_stream_in_sink_port_list(iface,
       session->session_id, hw_stream->mapped_mct_stream_id[i]);
     if (user_stream == NULL) {
        CDBG_ERROR("%s: cannot find user stream id %d\n", __func__,
          hw_stream->mapped_mct_stream_id[i]);
        continue;
     }

     /* Sensor meta data stream dimensions should be skipped */
     if(hw_stream->stream_type == IFACE_STREAM_TYPE_META_DATA) {
       CDBG("Skipping sensor meta data stream");
       continue;
     }

     /* fill mct stream info by copy userstream info first*/
     stream_info = user_stream->stream_info;

     /*fill in mct stream info modified by hw stream*/
     for (j = 0 ; j < hw_stream->plane_info.num_planes
      && j < VIDEO_MAX_PLANES; j++) {
       stream_info.buf_planes.plane_info.mp[j].stride = hw_stream->plane_info.strides[j];
       stream_info.buf_planes.plane_info.mp[j].scanline = hw_stream->plane_info.scanline[j];
       if (hw_stream->stream_info.batch_mode)
         stream_info.buf_planes.plane_info.mp[j].scanline /= hw_stream->stream_info.batch_size;

       stream_info.buf_planes.plane_info.mp[j].len =
         hw_stream->plane_info.strides[j] * hw_stream->plane_info.scanline[j];
       stream_info.buf_planes.plane_info.num_planes = hw_stream->plane_info.num_planes;
       stream_info.buf_planes.plane_info.mp[j].offset = hw_stream->plane_info.addr_offset[j];
       stream_info.buf_planes.plane_info.mp[j].offset_x = hw_stream->plane_info.addr_offset_h[j];
       stream_info.buf_planes.plane_info.mp[j].offset_y = hw_stream->plane_info.addr_offset_v[j];
     }

    /* need to send frame len information to CPP module */
     stream_info.buf_planes.plane_info.frame_len =
       hw_stream->stream_info.buf_planes.plane_info.frame_len;

     stream_info.dim = hw_stream->stream_info.dim;
     if (hw_stream->stream_info.batch_mode)
       stream_info.dim.height /= hw_stream->stream_info.batch_size;
     stream_info.fmt = hw_stream->stream_info.fmt;
     /* if the fmt is UBWC then change it to NV12 as the conversion happens
       in cpp. ISP outputs NV12*/
     if (stream_info.fmt == CAM_FORMAT_YUV_420_NV12_UBWC) {
       stream_info.fmt = CAM_FORMAT_YUV_420_NV12;
     }
     stream_info.num_burst = hw_stream->stream_info.num_burst;
     stream_info.img_buffer_list =
       hw_stream->buffers_info[VFE_BUF_QUEUE_DEFAULT].img_buffer_list;
     stream_info.num_bufs =
       hw_stream->buffers_info[VFE_BUF_QUEUE_DEFAULT].num_bufs;
     stream_info.streaming_mode = hw_stream->stream_info.streaming_mode;
     stream_info.user_buf_info.frame_buf_cnt =
       hw_stream->stream_info.batch_size;
#ifdef ISP_SVHDR_IN_BIT
     stream_info.bayerdata = hw_stream->hw_stream_id & ISP_SVHDR_IN_BIT ? 1 : 0;
#endif
     memset(&mct_event, 0, sizeof(mct_event));
     mct_event.u.module_event.type = MCT_EVENT_MODULE_ISP_OUTPUT_DIM;
     mct_event.u.module_event.module_event_data = (void *)&stream_info;
     mct_event.type = MCT_EVENT_MODULE_EVENT;
     mct_event.identity = pack_identity(session->session_id, user_stream->stream_id);
     mct_event.direction = MCT_EVENT_DOWNSTREAM;
     if (user_stream->src_port) {
       mct_port_send_event_to_peer(user_stream->src_port, &mct_event);
     }
  }
}

/** iface_util_send_hw_stream_output_dim_downstream
 *    @num_streams:
 *    @stream_ids:
 *
 * TODO
 *
 * Return: nothing
 **/
void iface_util_send_hw_stream_output_dim_downstream(iface_t *iface,
  iface_session_t *session, int32_t num_hw_streams, uint32_t *hw_stream_ids)
{
  iface_hw_stream_t *hw_stream = NULL;
  mct_stream_info_t stream_info;
  int32_t i = 0;
  for (i = 0; i < num_hw_streams; i++) {
    CDBG("%s:i = %d, hw stream_id = %x\n", __func__, i, hw_stream_ids[i]);
    hw_stream = iface_util_find_hw_stream_in_session(session, hw_stream_ids[i]);
    if (hw_stream == NULL) {
      CDBG_ERROR("%s: cannot find stream (session_id = %d, stream_id = %d)\n",
        __func__, session->session_id, hw_stream_ids[i]);
    } else
      iface_util_send_hw_stream_output_dim_downstream_int(iface, session, hw_stream);
  }
}

/** iface_util_append_image_buff
 *    @d1: list entry data
 *    @d2: user data
 *
 * Append image buffer to list
 *
 *  Returns TRUE for success and FALSE for failure
 **/
static boolean iface_util_append_image_buff(void *d1, void *d2)
{
  mct_stream_map_buf_t *img_buf = (mct_stream_map_buf_t *)d1;
  mct_list_t **img_buf_list = (mct_list_t **)d2;

  *img_buf_list = mct_list_append(*img_buf_list, img_buf, NULL, NULL);

  return TRUE;
}

/** iface_util_send_buff_list_downstream
 *    @iface: iface module data
 *    @session: iface session data
 *    @num_hw_streams: num of hw streams that will be streamon
 *    @hw_stream_ids: hw stream that will be streamon for this user stream
 *    @streamon_id: user stream id
 *
 * Send buffer list to bayerproc
 *
 *  Returns 0 for success and negative error for failure
 **/
int iface_util_send_buff_list_downstream(iface_t *iface,
  iface_session_t *session, int num_hw_streams, uint32_t *hw_stream_ids,
  uint32_t streamon_id)
{
  mct_event_t mct_event;
  iface_hw_stream_t *hw_stream = NULL;
  iface_stream_t *user_stream = NULL;
  mct_list_t *img_buf_list = NULL;
  uint32_t buff_id;
  int32_t i, j, rc = 0;
  iface_bufq_t *bufq;
  uint32_t bufq_handle;

  CDBG("send_buff_list for user stream id %x\n", streamon_id);

  for (i = 0; i < num_hw_streams; i++) {
    hw_stream = iface_util_find_hw_stream_in_session(session, hw_stream_ids[i]);
    if (!hw_stream) {
      CDBG_ERROR("%s:%d failed hw_stream %p i %d\n", __func__, __LINE__,
        hw_stream, i);
      continue;
    }
#ifdef ISP_SVHDR_IN_BIT
    if (!hw_stream->use_native_buf || !(hw_stream->hw_stream_id & ISP_SVHDR_IN_BIT)) {
      continue;
    }
#endif
    CDBG("filling img_buf_list for hw_stream_id %x, hw_img_buflist %x\n", hw_stream->hw_stream_id,
    hw_stream->buffers_info[VFE_BUF_QUEUE_DEFAULT].img_buffer_list);

    mct_list_traverse(hw_stream->buffers_info[VFE_BUF_QUEUE_DEFAULT].img_buffer_list,
      iface_util_append_image_buff, &img_buf_list);
    user_stream = iface_util_find_stream_in_sink_port_list(iface,
        session->session_id, hw_stream->mapped_mct_stream_id[0]);
    if (!user_stream) {
        CDBG_ERROR("%s: cannot find user stream id %d\n",
            __func__, hw_stream->mapped_mct_stream_id[0]);
    return -1;
    }
  }

  if (img_buf_list) {

    memset(&mct_event, 0, sizeof(mct_event));
    mct_event.u.module_event.type = MCT_EVENT_MODULE_OUTPUT_BUFF_LIST;
    mct_event.u.module_event.module_event_data = (void *)img_buf_list;
    mct_event.type = MCT_EVENT_MODULE_EVENT;
    mct_event.identity = pack_identity(session->session_id, user_stream->stream_id);
    mct_event.direction = MCT_EVENT_DOWNSTREAM;
    CDBG_ERROR("sending MCT_EVENT_MODULE_OUTPUT_BUFF_LIST on stream id %x, img buflist %x\n",
        user_stream->stream_id, img_buf_list);

    mct_port_send_event_to_peer(user_stream->src_port, &mct_event);

    /* free only list, data is owned by buffer manager */
    mct_list_free_list(img_buf_list);
  }

  return 0;
}

/** iface_util_reg_buf_list_update
 *    @iface: iface instance
 *    @session: session instance
 *    @stream_id: stream id for which buffers will be updated
 *    @map_buffer_list: Contains information of buffer to be
 *                    appended
 *
 *  Using the MCT buffer received, this function prepares
 *  information to be passed to buffer manager, such as buf mgr,
 *  bufq handle.
 *
 *  Returns 0 for success and negative error for failure
 **/
int iface_util_reg_buf_list_update(iface_t *iface, iface_session_t *session,
  int stream_id, mct_stream_map_buf_t * map_buffer_list, iface_port_t *iface_port)
{
  int rc = 0;
  iface_buf_request_t buf_request;
  uint32_t bufq_handle;
  int32_t buffs_id;
  mct_stream_info_t *stream_info = NULL;
  iface_stream_t *user_stream = NULL;

  CDBG("E. mct stream id:%x, buf type:%d", stream_id, map_buffer_list->buf_type);

  user_stream = iface_util_find_stream_in_sink_port(iface_port,
    session->session_id, stream_id);
  if (user_stream == NULL) {
    CDBG_ERROR("%s: stream not exist in this sinkport. error -1\n", __func__);
    return -1;
  }

  stream_info = &user_stream->stream_info;
  if (stream_info->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC &&
      (stream_info->reprocess_config.pp_feature_config.feature_mask &
      CAM_QCOM_FEATURE_RAW_PROCESSING)) {
    CDBG("offline ISP stream, ignore buf update.");
    return 0;
  }

  iface_hw_stream_t *hw_stream =
    iface_util_find_hw_stream_by_mct_id(iface, session, stream_id);
  if (hw_stream == NULL) {
    if (map_buffer_list && (map_buffer_list->buf_type ==
      CAM_MAPPING_BUF_TYPE_OFFLINE_INPUT_BUF ||
      map_buffer_list->buf_type == CAM_MAPPING_BUF_TYPE_OFFLINE_META_BUF)) {
      return 0;
    } else {
       CDBG_ERROR("%s: no hw stream found, ERROR! mct stream id = %x, rc = -1\n",
       __func__, stream_id);
       return -1;
     }
  }

  /* no update for native buffers */
  if (hw_stream->use_native_buf)
    return 0;

  buffs_id = iface_util_get_buffers_id(hw_stream, stream_id);
  if (buffs_id < 0 || buffs_id >= VFE_BUF_QUEUE_MAX) {
    CDBG_ERROR("%s: can not find buffers id, rc = -1\n", __func__);
    return -1;
  }

  bufq_handle = hw_stream->buffers_info[buffs_id].bufq_handle;
  if (bufq_handle == 0) {
    CDBG_ERROR("%s:failed, bufq not existed,"
      " bufq handle = %x, hw stream id = %x, buffs_id=0x%x rc = -1\n",
      __func__, bufq_handle, hw_stream->hw_stream_id,buffs_id);
    return -1;
  }

  CDBG("%s: E, sessid = %d, stream_id = %d bufq_handle = %x"
       "map_buffer_list %x\n", __func__, session->session_id, stream_id,
       bufq_handle, (unsigned int)map_buffer_list);

  assert(map_buffer_list != NULL);
  memset(&buf_request, 0, sizeof(buf_request));
  buf_request.buf_handle = bufq_handle;
  buf_request.session_id = session->session_id;
  buf_request.stream_id = stream_id;
  buf_request.use_native_buf = hw_stream->use_native_buf;

  /*Dual VFEs use shared buf type single ISP case use private buf type*/
  if (iface_util_is_dual_vfe_used_per_stream(session, hw_stream) == TRUE)
    buf_request.buf_type = ISP_SHARE_BUF;
  else
    buf_request.buf_type = ISP_PRIVATE_BUF;

  /* MCT sends one buffer at a time, we update internal buf array for which
     * we have to pass a buf list. Create a list using buf received from MCT*/
  buf_request.img_buf_list = mct_list_append(buf_request.img_buf_list,
    map_buffer_list,NULL, NULL);

  rc = iface_register_buf_list_update(&iface->buf_mgr, bufq_handle,
    &buf_request, 0);
  if (rc < 0) {
    CDBG_ERROR("%s: unable to update new buf list!\n", __func__);
    return rc;
  }

  hw_stream->buffers_info[buffs_id].current_num_bufs =
    buf_request.current_num_buf;

  buf_request.img_buf_list = mct_list_remove(buf_request.img_buf_list,
    map_buffer_list);

  CDBG("%s: X, rc = %d\n", __func__, rc);
  return rc;
}

/** iface_update_buf_info
 *    @iface: iface instance
 *    @session_id: session id
 *    @stream_id: stream id for which buffers will be updated
 *    @event: Contains information of buffer to be appended
 *
 *  This method is used to dynamically update hal buffers for a
 *  stream already running. We receive new buffer info from MCT,
 *  we update internal ISP buffer array and then enqueue to
 *  kernel without stopping the stream.
 *
 *  Returns 0 for success and negative error for failure
 **/
int iface_util_update_buf_info(
  iface_t         *iface,
  iface_port_t    *iface_sink_port,
  iface_session_t *session,
  uint32_t         user_stream_id,
  mct_event_t     *event)
{
  int rc = 0;

  if (!session){
      CDBG_ERROR("%s: cannot find session \n", __func__);
      rc = -1;
      goto end;
  }

  CDBG("%s: E, session_id = %d, stream_id = %d\n", __func__,
    session->session_id, user_stream_id);

  mct_stream_map_buf_t *buf_holder =
    (mct_stream_map_buf_t *)event->u.ctrl_event.control_event_data;
  if (!buf_holder){
      CDBG_ERROR("%s: failed %p\n", __func__, buf_holder);
      rc = -1;
      goto end;
  }

  rc = iface_util_reg_buf_list_update(iface, session, user_stream_id,
    buf_holder, iface_sink_port);
  if (rc < 0) {
    CDBG_ERROR("%s: isp_ch_util_reg_buf_list_update failed\n", __func__);
    goto end;
  }

end:
  CDBG("%s: X rc = %d\n", __func__, rc);
  return rc;
}

/** iface_util_remove_buf_info
 *    @iface: iface instance
 *    @session_id: session id
 *    @stream_id: stream id for which buffers will be updated
 *    @event: Contains information of buffer to be appended
 *
 *  This method is used to dynamically update hal buffers for a
 *  stream already running. We receive buffer info from MCT,
 *  we update internal ISP buffer array and then dequeue from
 *  kernel without stopping the stream.
 *
 *  Returns 0 for success and negative error for failure
 **/
int iface_util_remove_buf_info(
  iface_t         *iface,
  iface_port_t    *iface_sink_port __unused,
  iface_session_t *session,
  uint32_t         stream_id,
  mct_event_t     *event)
{
  iface_buf_request_t buf_request;
  uint32_t bufq_handle;
  int32_t buffs_id;
  int rc = 0;

  if (!session){
    CDBG_ERROR("%s: cannot find session \n", __func__);
    return -1;
  }

  CDBG("%s: E, session_id = %d, stream_id = %d\n", __func__,
    session->session_id, stream_id);

  mct_stream_map_buf_t *buf_holder =
    (mct_stream_map_buf_t *)event->u.ctrl_event.control_event_data;
  if (!buf_holder){
    CDBG_ERROR("%s: failed %p\n", __func__, buf_holder);
    return -1;
  }

  iface_hw_stream_t *hw_stream =
    iface_util_find_hw_stream_by_mct_id(iface, session, stream_id);
  if (hw_stream == NULL || hw_stream->state != IFACE_HW_STREAM_STATE_ACTIVE) {
    /* We do not need to remove buffers after streamoff. All of them are
     * removed from ISP kernel buffer manager during streamoff */
    return 0;
  }

  /* no update for native buffers */
  if (hw_stream->use_native_buf)
    return 0;

  buffs_id = iface_util_get_buffers_id(hw_stream, stream_id);
  if (buffs_id < 0 || buffs_id >= VFE_BUF_QUEUE_MAX) {
    CDBG_ERROR("%s: can not find buffers id, rc = -1\n", __func__);
    return -1;
  }

  bufq_handle = hw_stream->buffers_info[buffs_id].bufq_handle;
  if (bufq_handle == 0) {
    CDBG_ERROR("%s:failed, bufq not existed,"
      " bufq handle = %x, hw stream id = %x, rc = -1\n",
      __func__, bufq_handle, hw_stream->hw_stream_id);
    return -1;
  }

  assert(buf_holder != NULL);
  memset(&buf_request, 0, sizeof(buf_request));
  buf_request.buf_handle = bufq_handle;
  buf_request.session_id = session->session_id;
  buf_request.stream_id = stream_id;
  buf_request.use_native_buf = hw_stream->use_native_buf;

  /* Dual VFEs use shared buf type single ISP case use private buf type */
  if (iface_util_is_dual_vfe_used_per_stream(session, hw_stream) == TRUE)
    buf_request.buf_type = ISP_SHARE_BUF;
  else
    buf_request.buf_type = ISP_PRIVATE_BUF;

  /* MCT sends one buffer at a time, we update internal buf array for which
   * we have to pass a buf list. Create a list using buf received from MCT */
  buf_request.img_buf_list = mct_list_append(buf_request.img_buf_list,
    buf_holder, NULL, NULL);

  rc = iface_unregister_buf_list(&iface->buf_mgr, bufq_handle, &buf_request, 0);
  if (rc < 0) {
    CDBG_ERROR("%s: unable to update buf list!\n", __func__);
    return rc;
  }

  hw_stream->buffers_info[buffs_id].current_num_bufs =
    buf_request.current_num_buf;

  buf_request.img_buf_list = mct_list_remove(buf_request.img_buf_list,
    buf_holder);

end:
  CDBG("%s: X rc = %d\n", __func__, rc);
  return rc;
}

/** iface_util_add_buf_info
 *    @iface: iface instance
 *    @session_id: session id
 *    @user_stream_id: stream id for which buffers will be updated
 *
 *  Add buffers for new user stream which is mapped to running HW stream.
 *  This is need only in case of shared HW stream. If stream is not shared
 *  all buffers are add during stream on of first user stream mapped to
 *  this HW stream. This function should be used only if HW stream is running!
 *
 *  Returns 0 for success and negative error for failure
 **/
int iface_util_add_buf_info(iface_t *iface, iface_session_t *session,
  int user_stream_id)
{
  iface_hw_stream_t *hw_stream;
  uint32_t bufq_handle;
  int32_t buffs_id;
  int rc = 0;

  hw_stream = iface_util_find_hw_stream_by_mct_id(iface, session,
    user_stream_id);
  if (hw_stream == NULL) {
     CDBG_ERROR("%s: no hw stream found, ERROR! mct stream id = %x, rc = -1\n",
       __func__, user_stream_id);
     return -1;
  }

  /* if HW is not shared all buffers are already added. */
  if (!hw_stream->shared_hw_stream)
    return 0;

  buffs_id = iface_util_get_buffers_id(hw_stream, user_stream_id);
  if (buffs_id < 0 || buffs_id >= VFE_BUF_QUEUE_MAX) {
    CDBG_ERROR("%s: can not find buffers id, rc = -1\n", __func__);
    return -1;
  }

  bufq_handle = hw_stream->buffers_info[buffs_id].bufq_handle;
  if (bufq_handle == 0) {
    CDBG_ERROR("%s:failed, bufq not existed,"
      " bufq handle = %x, hw stream id = %x, rc = -1\n",
      __func__, bufq_handle, hw_stream->hw_stream_id);
    return -1;
  }

  rc = iface_util_axi_reg_shared_bufq(session, iface, hw_stream->hw_stream_id,
    user_stream_id);
  if (rc) {
    CDBG_ERROR("%s: iface_util_axi_reg_shared_bufq error = %d\n", __func__, rc);
    return rc;
  }

  rc = iface_util_add_bufq_to_stream(session, iface, hw_stream->hw_stream_id,
    user_stream_id);
  if (rc) {
    CDBG_ERROR("%s: failed: iface_util_add_bufq_to_stream\n", __func__);
    return rc;
  }

  return rc;
}

/** iface_util_config_meta:
 *    @ispif: ispif isntance
 *    @ispif_sink_port: ispif sinc port
 *    @stream: stream
 *
 *  This function runs in MCTL thread context.
 *
 *  This function stores meta channel information from sensor
 *
 *  Return: TRUE - Success
 *         FALSE-error
 **/
boolean iface_util_config_meta(
  iface_t            *iface __unused,
  iface_port_t       *iface_sink_port __unused,
  iface_session_t    *session,
  uint32_t            stream_id,
  sensor_meta_data_t *sensor_meta_cfg)
{
  uint32_t         i = 0;
  iface_bundle_meta_info_t *request_bundle_meta = NULL;

  if (!session) {
    CDBG_ERROR("%s: No sessionfound to associate meta information %d\n",
      __func__, stream_id);
    return FALSE;
  }

  CDBG("%s: session_id %x stream_id %x  num_meta %d\n",
    __func__, session->session_id, stream_id, sensor_meta_cfg->num_meta);

  /*find a non used bundle meta slot in session*/
  for (i = 0; i < IFACE_MAX_STREAMS; i++) {
    request_bundle_meta = &session->bundle_meta_info[i];
    if (request_bundle_meta->user_stream_id == 0) {
      CDBG_HIGH("%s: get no.%d user meta info, user_stream id = %d\n",
        __func__, i, stream_id);
      break;
    } else {
      continue;
    }
  }
  /*loop through all slot, no more empty meta slot*/
  if (i == IFACE_MAX_STREAMS) {
    CDBG_ERROR("%s: no more room for metadata stream, return false\n",
      __func__);
    return FALSE;
  }

  request_bundle_meta->user_stream_id = stream_id;
  request_bundle_meta->sensor_meta = *sensor_meta_cfg;
  session->num_bundle_meta++;

  return TRUE;
}

/** iface_util_config_pdaf_meta:
 *    @ispif: ispif isntance
 *    @ispif_sink_port: ispif sinc port
 *    @stream: stream
 *
 *  This function runs in MCTL thread context.
 *
 *  This function stores pdaf type 3 information from sensor
 *
 *  Return: TRUE - Success
 *         FALSE-error
 **/
boolean iface_util_config_pdaf_meta(
  iface_t         *iface __unused,
  iface_port_t    *iface_sink_port __unused,
  iface_session_t *session,
  uint32_t        stream_id,
  pdaf_sensor_native_info_t *pdaf_config)
{
  uint32_t         i = 0;
  iface_bundle_pdaf_info_t *request_pdaf_config = NULL;

  if (!pdaf_config) {
    CDBG_ERROR("%s pdaf_config %p \n", __func__, pdaf_config);
    return FALSE;
  }

  if (!session) {
    CDBG_ERROR("%s: No sessionfound to associate meta information %d\n",
      __func__, stream_id);
    return FALSE;
  }

  CDBG("%s: <pdaf_dbg> pdaf session_id %x stream_id %x \n",
    __func__, session->session_id, stream_id);

  /*find a non used bundle meta slot in session*/
  for (i = 0; i < IFACE_MAX_STREAMS; i++) {
    request_pdaf_config = &session->bundle_pdaf_info[i];
    if (request_pdaf_config->user_stream_id == 0) {
      CDBG_HIGH("%s: get no.%d user meta info, user_stream id = %d\n",
        __func__, i, stream_id);
      break;
    }
  }
  /*loop through all slot, no more empty meta slot*/
  if (i == IFACE_MAX_STREAMS) {
    CDBG_ERROR("%s: no more room for metadata stream, return false\n",
      __func__);
    return FALSE;
  }

  request_pdaf_config->user_stream_id = stream_id;
  request_pdaf_config->pdaf_config = *pdaf_config;
  request_pdaf_config->block_dim.width         = 0;
  request_pdaf_config->block_dim.height        = 0;
  request_pdaf_config->left_pix_skip_pattern   = 0;
  request_pdaf_config->left_line_skip_pattern  = 0;
  request_pdaf_config->right_pix_skip_pattern  = 0;
  request_pdaf_config->right_line_skip_pattern = 0;
  switch (pdaf_config->buffer_data_type) {
    case PDAF_DATA_TYPE_RAW16:
      request_pdaf_config->camif_output_format = CAMIF_PLAIN16_FORMAT;
      break;
    case PDAF_DATA_TYPE_RAW10_LSB:
      request_pdaf_config->camif_output_format = CAMIF_QCOM_FORMAT;
      break;
    case PDAF_DATA_TYPE_RAW10_PACKED:
      request_pdaf_config->camif_output_format = CAMIF_MIPI_FORMAT;
      break;
    case PDAF_DATA_TYPE_RAW8:
      request_pdaf_config->camif_output_format = CAMIF_PLAIN8_FORMAT;
      break;
    default:
      CDBG_ERROR("<PDAF> Requested output format not support.Falling back to \
        QCOM_RAW");
      request_pdaf_config->camif_output_format = CAMIF_QCOM_FORMAT;
      break;
  }
  session->num_pdaf_meta++;
  return TRUE;
}

/**
 * iface_util_find_number_skip
 *
 * @param skip_pattern
 *
 * @return uint32_t
 */
uint32_t iface_util_find_number_skip(uint16_t skip_pattern)
{
  uint32_t count = 0;
  while (skip_pattern > 0) {
    count = count + 1;
    skip_pattern = skip_pattern & (skip_pattern - 1);
  }
  return count;
}

/**
 * iface_util_circularShiftLeft
 *
 * @param value
 * @param shift
 *
 * @return uint16_t
 */
uint16_t iface_util_circularShiftLeft(uint16_t value, int shift)
{
    return ((value << shift) | (value >> (16 - shift)));
}


/**
 * iface_util_reverse_skip
 *
 * @param skip_pattern
 *
 * @return int
 */
uint16_t iface_util_reverse_skip(uint16_t skip_pattern)
{
  uint16_t reverse_pattern = 0;
  int i = 0;
  for (i = 0; i < 15; i++) {
    reverse_pattern |= (skip_pattern & 1);
    skip_pattern = skip_pattern >> 1;
    reverse_pattern = reverse_pattern << 1;
  }
  reverse_pattern |=(skip_pattern & 1);
  return reverse_pattern;
}


/** iface_util_add_user_stream_to_hw_stream:
 *
 *    @iface: iface handle
 *    @iface_sink_port: sink port handle where this event arrived
 *    @session id: session id
 *    @user_stream_id: user stream id
 *
 *    Add addition info for new user stream to running HW stream.
 **/
int iface_util_add_user_stream_to_hw_stream(iface_t *iface,
  iface_port_t *iface_sink_port, uint32_t session_id,
  uint32_t user_stream_id)
{
  iface_session_t *session = NULL;
  iface_hw_stream_t *hw_stream = NULL;
  int rc = 0;

  session = iface_util_get_session_by_id(iface, session_id);
  if (!session) {
    CDBG_ERROR("%s: cannot find session %d\n", __func__, session_id);
    return -1;
  }

  hw_stream = iface_util_find_hw_stream_by_mct_id(iface, session,
    user_stream_id);
  if (hw_stream == NULL) {
    /* Raw stream does not need HW stream. */
    return 0;
  }

  if (!hw_stream->shared_hw_stream) {
    CDBG_HIGH("%s: hw stream already streaming. hw stream id %x, is_share_hw_stream?%d\n",
      __func__, hw_stream->hw_stream_id, hw_stream->shared_hw_stream);
    return 0;
  }

  /* update hw buf info by user buf info just updated on every streamon */
  rc = iface_util_update_hw_buf_info(iface, iface_sink_port, session,
    &user_stream_id, 1);
  if (rc < 0) {
    CDBG_ERROR("%s: iface_util_update_hw_buf_info failed, rc= %d\n", __func__,
      rc);
    return rc;
  }

  /* request buffer: hal buffer or native buffer */
  rc = iface_util_request_image_buf(iface, session, 1,
    &hw_stream->hw_stream_id);
  if (rc < 0) {
    CDBG_ERROR("%s: iface_util_request_image_buf failed, rc = %d\n", __func__,
      rc);
    return rc;
  }

  /* append buffer queue to existing HW stream */
  rc = iface_util_add_buf_info(iface, session, user_stream_id);
  if (rc < 0) {
    CDBG_ERROR("%s: iface_util_add_buf_info failed, rc = %d\n", __func__, rc);
    return rc;
  }

  return 0;
}

/** iface_util_setloglevel:
 *
 *
 *  This function stores sets the logging level in iface
 *
 *  Return: TRUE - Success
 *         FALSE-error
 **/

void iface_util_setloglevel(void)
{
  char prop[PROPERTY_VALUE_MAX];
  iface_loglevel_t globalloglevel ;

  property_get("persist.vendor.camera.iface.logs", prop, "1");
  g_ifaceloglevel = atoi(prop);
  memset(prop, 0, sizeof(prop));
  property_get("persist.vendor.camera.global.debug", prop, "0");
  globalloglevel = atoi(prop);
  if (globalloglevel > g_ifaceloglevel) {
    g_ifaceloglevel = globalloglevel;
  }
  CDBG_HIGH("%s:%d IFACE_Loglevel %d", __func__, __LINE__, g_ifaceloglevel);
  return;
}

boolean iface_util_setloglevel_frm_hal(uint32_t *halloglevel)
{
  if (!halloglevel) {
    CDBG_ERROR("%s:%d Null Pointer halloglevel %p\n", halloglevel);
    return FALSE;
  }
  iface_util_setloglevel();

  return TRUE;
}

/** iface_util_set_preferred_mapping
 *    @module: mct module handle
 *    @event: module event
 *
 *  Handle preferred stream mapping
 *
 * Return TRUE on success and FALSE if fails
 **/
boolean iface_util_set_preferred_mapping(iface_t *iface, mct_event_t *event)
{
  iface_session_t          *session = NULL;
  isp_preferred_streams    *preferred_mapping;
  uint32_t                  i, j, curr_num;
  boolean                   ret;

  session = iface_util_get_session_by_id(iface,
    UNPACK_SESSION_ID(event->identity));
  if (!session) {
    CDBG_ERROR("%s: cannot find session %d\n", __func__,
      UNPACK_SESSION_ID(event->identity));
    return -1;
  }

  preferred_mapping = (isp_preferred_streams *)
    event->u.module_event.module_event_data;

  /* Update preferred streams */
  curr_num = session->preferred_mapping.stream_num;
  for (i = 0; i < preferred_mapping->stream_num
    && i < ISP_NUM_PIX_STREAM_MAX; i++) {
    for(j = 0; j < (i + curr_num)
      && j < ISP_NUM_PIX_STREAM_MAX; j++) {
      if(session->preferred_mapping.streams[j].stream_mask &
          preferred_mapping->streams[i].stream_mask) {
        CDBG_ERROR("%s, error: Requested same stream type on different ports\n",
            __func__);
        return FALSE;
      }
    }
    session->preferred_mapping.streams[i + curr_num] =
      preferred_mapping->streams[i];
  }
  session->preferred_mapping.stream_num += preferred_mapping->stream_num;

  return TRUE;
}

/** iface_util_post_bus_msg:
 *
 *  @session: session handle
 *  @type: bus msg type
 *  @data: payload handle
 *  @data_size: payload size
 *
 *  Post bus msg
 *
 *  Return 0 on success and -1 on failure
 **/
int32_t iface_util_post_bus_msg(iface_session_t *session,
  mct_bus_msg_type_t type, void *data, uint32_t data_size)
{
  int32_t        rc = 0;
  boolean        ret = TRUE;
  mct_bus_msg_t  bus_msg;
  iface_t       *iface = NULL;

  if (!session || !data) {
    CDBG_ERROR("%s:%d failed: %p %p\n", session, data);
    return -EINVAL;
  }

  iface = (iface_t *)session->iface;
  if (!iface) {
    CDBG_ERROR("%s:%d failed: iface %p", __func__, __LINE__, iface);
    return -EINVAL;
  }

  /* Post metadata */
  memset(&bus_msg, 0, sizeof(bus_msg));

  bus_msg.sessionid = session->session_id;
  bus_msg.type = type;
  bus_msg.msg = data;
  bus_msg.size = data_size;
  ret = mct_module_post_bus_msg(iface->module, &bus_msg);
  if (ret == FALSE) {
    CDBG_ERROR("%s:%d mct_module_post_bus_msg type %d\n", __func__, __LINE__,
      type);
    rc = -1;
  }

  return rc;
}


/** iface_util_process_bundle_streamonoff_list:
 *
 *  @session:       session handle
 *  @bundle_info:   bundle information for processing
 *  @is_streamon:   boolean flag for streamon or off
 *
 *  This funciton returns the mct streamon/off list in bundle
 *  info.
 *
 *  Always return 0 (success)
 **/
int iface_util_process_bundle_streamonoff_list(
    iface_session_t                     *session,       /* iface session */
    iface_util_process_bundle_info_t    *bundle_info,   /* bundle information for process */
    boolean                              is_streamon)   /* is streamon or not */
{
  uint32_t           i;
  uint32_t           j;
  uint32_t           k;
  iface_hw_stream_t *hw_stream;

  /* this is callled only in streamon/off, no need to check null here */

  bundle_info->num_mct_ids_for_default_bufq = 0;
  bundle_info->num_mct_ids_for_shared_bufq = 0;

  if (bundle_info->num_user_streams == 0) {
    return 0;
  }

  if (bundle_info->num_user_streams == 1) {
    bundle_info->num_mct_ids_for_default_bufq = 1;
    bundle_info->mct_ids_for_default_bufq[0] =
      bundle_info->user_stream_ids[0];
    return 0;
  }

  /* need some rework for the bundle streams on */
  for (i = 0; i < bundle_info->num_hw_streams; i++) {
    hw_stream =
      iface_util_find_hw_stream_in_session(session,
                                           bundle_info->hw_stream_ids[i]);
    if(hw_stream == NULL)
      continue;

    CDBG("%s, hw stream id %d, streamon count = %d", __func__,
         hw_stream->hw_stream_id, hw_stream->streamon_count);

    if (hw_stream->shared_hw_stream) {
      CDBG("%s,Rework the shared hw stream 0x%x\n", __func__,
           hw_stream->hw_stream_id);

      boolean found = FALSE;
      for (k = 0; k < hw_stream->num_mct_stream_mapped; k++) {
        for (j = 0; j < bundle_info->num_user_streams; j++) {
          if (hw_stream->mapped_mct_stream_id[k] ==
              bundle_info->user_stream_ids[j]) {
            break;
          }
        }

        if (j < bundle_info->num_user_streams) {
          /*  hw stream id is created before stream on, so the mct streamon_id
              should be the one matches the hw stream id for defalut bufq */
          if ((hw_stream->hw_stream_id & 0xFF) !=
              hw_stream->mapped_mct_stream_id[k]) {

            bundle_info->mct_ids_for_shared_bufq[
              bundle_info->num_mct_ids_for_shared_bufq++] =
              hw_stream->mapped_mct_stream_id[k];

            if (is_streamon) {
              /* for streamon, we need to do this since the stream is removed
                 from the default bufq list. The count will be increase when
                 we process the shared bufq list */
              hw_stream->streamon_count--;
            }
          } else {
            bundle_info->mct_ids_for_default_bufq[
              bundle_info->num_mct_ids_for_default_bufq++] =
                hw_stream->mapped_mct_stream_id[k];
            hw_stream->mct_streamon_id = hw_stream->mapped_mct_stream_id[k];
            found = TRUE;
          }
        }
      }

      if (found == FALSE) {
        CDBG_ERROR("%s: Error: Can not found default stream for the"
                   "default bufq\n", __func__);
      }
    } else {
      /* none shared hw stream */
      for (k = 0; k < hw_stream->num_mct_stream_mapped; k++) {
        for (j = 0; j < bundle_info->num_user_streams; j++) {
          if (hw_stream->mapped_mct_stream_id[k] ==
              bundle_info->user_stream_ids[j]) {
            break;
          }
        }

        if (j < bundle_info->num_user_streams) {

          bundle_info->mct_ids_for_default_bufq[
            bundle_info->num_mct_ids_for_default_bufq++] =
              hw_stream->mapped_mct_stream_id[k];
        }
      }
    }

    CDBG("%s, hw stream id %d, streamon count = %d", __func__,
               hw_stream->hw_stream_id, hw_stream->streamon_count);
  }
  return 0;
}

/** iface_util_process_mct_stream_for_shared_bufq:
 *
 *  @iface:         iface object pointer
 *  @session:       session handle
 *  @bundle_info:   bundle information for processing
 *  @is_streamon:   boolean flag for streamon or off
 *
 *  This funciton process the mct stream for streamon/off
 *
 *  Always return 0 (success)
 **/
int iface_util_process_mct_stream_for_shared_bufq(
    iface_t                             *iface,
    iface_port_t                        *iface_sink_port,
    iface_session_t                     *session,
    iface_util_process_bundle_info_t    *bundle_info,
    boolean                              is_streamon)
{
  int                      rc;
  uint32_t                 i;
  iface_hw_stream_t       *hw_stream    = NULL;


  for (i = 0; i< bundle_info->num_mct_ids_for_shared_bufq; i++) {
    if (is_streamon == TRUE) {
      CDBG("%s: Processing streamon with mct stream %d"
           "that are using shared bufq\n",
           __func__, bundle_info->mct_ids_for_shared_bufq[i]);

      bundle_info->num_hw_streams =
        iface_util_decide_hw_streams(iface,
                                     session,
                                     1,
                                     &bundle_info->mct_ids_for_shared_bufq[i],
                                     bundle_info->hw_stream_ids,
                                     1);
      /* num hw stream must be 0 */
      if (bundle_info->num_hw_streams == 0) {
        CDBG("%s: Processing mct stream %d on shared bufq\n",
              __func__, bundle_info->mct_ids_for_shared_bufq[i]);
        rc =
          iface_util_add_user_stream_to_hw_stream(
              iface,
              iface_sink_port,
              session->session_id,
              bundle_info->mct_ids_for_shared_bufq[i]);
      if (rc != 0) {
        CDBG_ERROR("iface_util_add_user_stream_to_hw_stream failed !!!\n");
      }
      } else {
        CDBG_ERROR("%s: Serious problem in bundle stream on!\n", __func__);
      }
    } else {

      CDBG("%s: Processing streamoff with mct stream %d"
           "that are using shared bufq\n",
           __func__, bundle_info->mct_ids_for_shared_bufq[i]);

      hw_stream =
        iface_util_find_hw_stream_by_mct_id(
            iface,
            session,
            bundle_info->mct_ids_for_shared_bufq[i]);
      if (hw_stream == NULL) {
         CDBG_ERROR("%s: no hw stream found, ERROR!"
                    "mct stream id = %x, rc = -1\n",
                    __func__, bundle_info->mct_ids_for_shared_bufq[i]);
         continue;
      }

      rc =
        iface_util_remove_bufq_from_stream(
            session,
            iface,
            hw_stream->hw_stream_id,
            bundle_info->mct_ids_for_shared_bufq[i]);
      if (rc) {
        CDBG_ERROR("%s: failed: iface_util_add_bufq_to_stream\n", __func__);
      }

      rc =
        iface_util_axi_unreg_shared_bufq(
            session,
            iface,
            hw_stream->hw_stream_id,
            bundle_info->mct_ids_for_shared_bufq[i]);
      if (rc) {
        CDBG_ERROR("%s: iface_util_axi_unreg_shared_bufq error = %d\n",
                   __func__, rc);
      }
    }
  }
  return 0;
}


/** iface_util_set_preferred_mapping
 *    @module: mct module handle
 *    @event: module event
 *
 *  Handle preferred stream mapping
 *
 * Return TRUE on success and FALSE if fails
 **/
boolean iface_util_set_buf_allignment(iface_t *iface, mct_event_t *event)
{
  iface_session_t          *session = NULL;
  sensor_isp_stream_sizes_t *stream_size;
  uint32_t                  min_buf_padding = 0;


  if (!iface || !event){
    IFACE_ERR("failed iface %p event %p", iface, event);
    return FALSE;
  }

  stream_size = (sensor_isp_stream_sizes_t *)
    event->u.module_event.module_event_data;

  if (!stream_size) {
    IFACE_ERR("failed stream_size %p ", stream_size);
    return FALSE;
  }

  /* All YUV width op are padded to 32 bit boundary*/
  min_buf_padding = CAM_PAD_TO_32;
  if (stream_size->width_alignment <  min_buf_padding) {
    stream_size->width_alignment  = min_buf_padding;
  }
  min_buf_padding = CAM_PAD_TO_2;

  if (stream_size->height_alignment <  min_buf_padding) {
    stream_size->height_alignment  = min_buf_padding;
  }


  return TRUE;
}

/** iface_util_choose_pack_mode:
 *
 *  @cid:                                   CID sent from the sensor
 *  @sensor_cap_pack_mode:  Original pack mode present in sensor caps
 *  @sensor_out_info:              Sensor set stream config data
 *
 *
 *  This function overrides the original pack mode from the sensor caps with
 *  the ones in the override list from the sensor_out_info sructure for the
 *  appropriate CIDs that match. This pack mode override list is filled in by
 *  the sensor when the original pack mode from the sensor caps need to
 *  be overriden based on input from HAL.
 *
 *  Returns the pack mode based on the cid
 **/
iface_output_pack_mode iface_util_choose_pack_mode(unsigned char cid,
  iface_output_pack_mode sensor_cap_pack_mode, sensor_out_info_t* sensor_out_info)
{
  uint32_t i = 0;
  iface_output_pack_mode pack_mode = PACK_MODE_BYTE;
  boolean cid_found = FALSE;

  for (i = 0; i < sensor_out_info->pack_mode_override.num_entries; i++) {
    if (cid == sensor_out_info->pack_mode_override.pack_mode_info[i].cid) {
      cid_found = TRUE;
      pack_mode = sensor_out_info->pack_mode_override.pack_mode_info[i].pack_mode;
      break;
    }
  }

  if (!cid_found)
    pack_mode = sensor_cap_pack_mode;

  return pack_mode;
}
