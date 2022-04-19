/*============================================================================
Copyright (c) 2013-2017,2019 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/videodev2.h>

#include "camera_dbg.h"
#include "iface_ops.h"
#include "iface_axi.h"
#include "iface.h"

#include "cam_intf.h"
#include "mct_controller.h"
#include "modules.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

/** iface_axi_util_find_stream:
 *
 *    @axi:
 *    @session_id:
 *    @stream_id:
 *
 **/
iface_axi_stream_t *iface_axi_util_find_stream(iface_axi_hw_t *axi, uint32_t session_id,
  uint32_t stream_id)
{
  int i;

  for (i = 0; i < IFACE_AXI_STREAM_MAX; i++) {
    CDBG("%s: session_id = %d, hw_stream_id = %d, in_sess_id = %d"
         "in_hw_stream_id = %d", __func__,
         axi->stream[i].hw_stream_info.session_id,
         axi->stream[i].hw_stream_info.hw_stream_id, session_id, stream_id);

    if (axi->stream[i].state != IFACE_AXI_STREAM_STATE_INVALID &&
        axi->stream[i].hw_stream_info.session_id == session_id &&
        axi->stream[i].hw_stream_info.hw_stream_id == stream_id)
      return &axi->stream[i];
  }

  return NULL;
}

/** iface_axi_util_find_stream_handle:
 *
 *    @axi:
 *    @handle:
 *
 **/
iface_axi_stream_t *iface_axi_util_find_stream_handle(iface_axi_hw_t *axi_hw,
  uint32_t handle)
{
  int i;

  for (i = 0; i < IFACE_AXI_STREAM_MAX; i++) {
    if (axi_hw->stream[i].axi_stream_handle == handle &&
        axi_hw->stream[i].state != IFACE_AXI_STREAM_STATE_INVALID) {
      return &axi_hw->stream[i];
    }
  }

  return NULL;
}

/** iface_axi_util_cam_fmt_to_v4l2_fmt:
 *
 *    @fmt:
 *    @uv_subsample:
 *
 **/
uint32_t iface_axi_util_cam_fmt_to_v4l2_fmt(
  cam_format_t fmt, uint32_t uv_subsample)
{
  switch (fmt) {
  case CAM_FORMAT_YUV_420_NV12:
  case CAM_FORMAT_YUV_420_NV12_UBWC:
  case CAM_FORMAT_YUV_420_NV12_VENUS:
    return (uv_subsample) ? V4L2_PIX_FMT_NV14 : V4L2_PIX_FMT_NV12;

  case CAM_FORMAT_YUV_RAW_8BIT_YUYV:
    return V4L2_PIX_FMT_YUYV;

  case CAM_FORMAT_YUV_RAW_8BIT_YVYU:
    return V4L2_PIX_FMT_YVYU;

  case CAM_FORMAT_YUV_RAW_8BIT_VYUY:
    return V4L2_PIX_FMT_VYUY;

  case CAM_FORMAT_YUV_RAW_8BIT_UYVY:
    return V4L2_PIX_FMT_UYVY;

  case CAM_FORMAT_YUV_420_NV21:
  case CAM_FORMAT_YUV_420_NV21_VENUS:
    return (uv_subsample) ? V4L2_PIX_FMT_NV41 : V4L2_PIX_FMT_NV21;

  case CAM_FORMAT_YUV_420_NV21_ADRENO:
    return (uv_subsample) ? V4L2_PIX_FMT_NV41 : V4L2_PIX_FMT_NV21;

  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
    return V4L2_PIX_FMT_SBGGR8;

  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
    return V4L2_PIX_FMT_SGBRG8;

  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
    return V4L2_PIX_FMT_SGRBG8;

  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
    return V4L2_PIX_FMT_SRGGB8;

  case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_BGGR:
    return V4L2_PIX_FMT_QBGGR8;

  case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GBRG:
    return V4L2_PIX_FMT_QGBRG8;

  case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GRBG:
    return V4L2_PIX_FMT_QGRBG8;

  case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_RGGB:
    return V4L2_PIX_FMT_QRGGB8;

  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
    return V4L2_PIX_FMT_SBGGR10;

  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
    return V4L2_PIX_FMT_SGBRG10;

  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
    return V4L2_PIX_FMT_SGRBG10;

  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
    return V4L2_PIX_FMT_SRGGB10;

  case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_BGGR:
    return V4L2_PIX_FMT_QBGGR10;

  case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GBRG:
    return V4L2_PIX_FMT_QGBRG10;

  case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GRBG:
    return V4L2_PIX_FMT_QGRBG10;

  case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_RGGB:
    return V4L2_PIX_FMT_QRGGB10;

  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
    return V4L2_PIX_FMT_SBGGR12;

  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
    return V4L2_PIX_FMT_SGBRG12;

  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
    return V4L2_PIX_FMT_SGRBG12;

  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
    return V4L2_PIX_FMT_SRGGB12;

  case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_BGGR:
    return V4L2_PIX_FMT_QBGGR12;

  case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GBRG:
    return V4L2_PIX_FMT_QGBRG12;

  case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GRBG:
    return V4L2_PIX_FMT_QGRBG12;

  case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_RGGB:
    return V4L2_PIX_FMT_QRGGB12;

  case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_BGGR:
    return V4L2_PIX_FMT_SBGGR14;

  case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GBRG:
    return V4L2_PIX_FMT_SGBRG14;

  case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GRBG:
    return V4L2_PIX_FMT_SGRBG14;

  case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_RGGB:
    return V4L2_PIX_FMT_SRGGB14;

  case CAM_FORMAT_BAYER_QCOM_RAW_14BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_14BPP_BGGR:
    return V4L2_PIX_FMT_QBGGR14;

  case CAM_FORMAT_BAYER_QCOM_RAW_14BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_14BPP_GBRG:
    return V4L2_PIX_FMT_QGBRG14;

  case CAM_FORMAT_BAYER_QCOM_RAW_14BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_14BPP_GRBG:
    return V4L2_PIX_FMT_QGRBG14;

  case CAM_FORMAT_BAYER_QCOM_RAW_14BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_14BPP_RGGB:
    return V4L2_PIX_FMT_QRGGB14;

  case CAM_FORMAT_YUV_420_YV12:
    return V4L2_PIX_FMT_YVU420;

  case CAM_FORMAT_YUV_422_NV16:
    return (uv_subsample) ? V4L2_PIX_FMT_NV14 : V4L2_PIX_FMT_NV16;

  case CAM_FORMAT_YUV_422_NV61:
    return (uv_subsample) ? V4L2_PIX_FMT_NV41 : V4L2_PIX_FMT_NV61;

  case CAM_FORMAT_JPEG_RAW_8BIT:
    return V4L2_PIX_FMT_JPEG;

  case CAM_FORMAT_META_RAW_8BIT:
    return V4L2_PIX_FMT_META;
  case CAM_FORMAT_META_RAW_10BIT:
    return V4L2_PIX_FMT_META10;

  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GBRG:
  case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GBRG:
    return V4L2_PIX_FMT_P16GBRG10;

  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GRBG:
  case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GRBG:
    return V4L2_PIX_FMT_P16GRBG10;

  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_RGGB:
  case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_RGGB:
    return V4L2_PIX_FMT_P16RGGB10;

  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_BGGR:
  case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_BGGR:
    return V4L2_PIX_FMT_P16BGGR10;

  case CAM_FORMAT_Y_ONLY:
    return V4L2_PIX_FMT_GREY;

  case CAM_FORMAT_Y_ONLY_10_BPP:
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GREY:
    return V4L2_PIX_FMT_Y10;

  case CAM_FORMAT_Y_ONLY_12_BPP:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GREY:
    return V4L2_PIX_FMT_Y12;

  case CAM_FORMAT_YUV_444_NV24:
    return V4L2_PIX_FMT_NV24;

  case CAM_FORMAT_YUV_444_NV42:
    return V4L2_PIX_FMT_NV42;

  default:
    return 0;
  }
}

/** iface_axi_util_calculate_output_width:
 *
 *    @stream:
 *
 **/
static uint32_t iface_axi_util_calculate_output_width(iface_axi_stream_t *stream)
{
  if (stream->hw_stream_info.isp_split_output_info.is_split) {
    if (stream->hw_stream_info.isp_split_output_info.stripe_id == ISP_STRIPE_LEFT)
      return stream->hw_stream_info.isp_split_output_info.left_output_width;
    else
      return stream->hw_stream_info.isp_split_output_info.right_output_width;
  }

  return stream->hw_stream_info.stream_info.dim.width; /*Include padding*/
}

/** iface_axi_util_calculate_plane_addr_offset:
 *
 *    @stream:
 *
 **/
static uint32_t iface_axi_util_calculate_plane_addr_offset(
  iface_axi_stream_t *stream, cam_stream_buf_plane_info_t *buf_planes,
  uint32_t *calculated_plane_addr_offset)
{
  int i = 0;
  uint32_t dual_vfe_addr_offset = 0;
  uint32_t curr_buf_start_addr = 0;

  if (stream->hw_stream_info.isp_split_output_info.is_split) {
    if (stream->hw_stream_info.isp_split_output_info.stripe_id
        == ISP_STRIPE_LEFT) {
      dual_vfe_addr_offset = 0;
    } else {
      dual_vfe_addr_offset =
        stream->hw_stream_info.isp_split_output_info.left_output_width;
    }
  }

  for (i = 0; i < IFACE_MAX_NUM_PLANE; i++) {
    if ((i != 0) && stream->hw_stream_info.need_uv_subsample) {
      /* if dual vfe right stripe, offset need to adjust for CDS*/
      if (stream->hw_stream_info.isp_split_output_info.is_split &&
          stream->hw_stream_info.isp_split_output_info.stripe_id
          == ISP_STRIPE_RIGHT)
        dual_vfe_addr_offset /= 2;
    }

    if (buf_planes->plane_info.mp[i].len != 0) {
      CDBG_HIGH("%s: buf_dbg plane[%d](CbCr) dual vfe axi offset / 2!\n",
        __func__, i);
      calculated_plane_addr_offset[i] =
        buf_planes->plane_info.mp[i].offset + dual_vfe_addr_offset;
    } else {
      CDBG("%s: buf_dbg plane[%d]  length = %d, skip!\n",
        __func__, i, buf_planes->plane_info.mp[i].len);
    }
  }

  return 0;
}

/** iface_axi_util_get_dwords_per_line
 *
 *  @fmt: output format
 *
 */
int iface_axi_util_get_dwords_per_line(
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

/** iface_axi_util_fill_plane_info:
 *
 *    @axi:
 *    @planes:
 *    @stream:
 *
 **/
int iface_axi_util_fill_plane_info(
  iface_axi_hw_t               *axi_hw __unused,
  struct msm_vfe_axi_plane_cfg *planes,
  iface_axi_stream_t           *axi_stream)
{
  int i = 0;
  int rc = 0;
  uint32_t reserved_intf_mask = axi_stream->hw_stream_info.interface_mask;
  uint32_t calculated_output_width = 0;
  uint32_t calculated_plane_addr_offset[IFACE_MAX_NUM_PLANE];
  cam_stream_buf_plane_info_t *buf_planes = NULL;
  int dwords = 12;

  /*calculate output width considering dual vfe or single vfe*/
  calculated_output_width =
    iface_axi_util_calculate_output_width(axi_stream);

  /*calculate offset consider dual vfe and request address offset*/
  buf_planes = &axi_stream->hw_stream_info.stream_info.buf_planes;
  memset(calculated_plane_addr_offset, 0,
    IFACE_MAX_NUM_PLANE * sizeof(uint32_t));
  rc = iface_axi_util_calculate_plane_addr_offset(axi_stream, buf_planes,
    &calculated_plane_addr_offset[0]);
  if (rc < 0) {
    CDBG_ERROR("%s: calculate_plane_addr_offset failed, rc = %d\n",
      __func__, rc);
    return rc;
  }

  switch (axi_stream->hw_stream_info.stream_info.fmt) {
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
    for (i = 0; i < 2; i++) {
      planes[i].output_width = calculated_output_width; /*Include padding*/
      planes[i].output_height = axi_stream->hw_stream_info.stream_info.dim.height;
      planes[i].output_stride =  axi_stream->hw_stream_info.plane_info.strides[i];
      planes[i].output_scan_lines = axi_stream->hw_stream_info.plane_info.scanline[i];
      planes[i].plane_addr_offset = calculated_plane_addr_offset[i];

      if((reserved_intf_mask & (1 << (16 * VFE0 + IFACE_INTF_RDI0))) ||
       (reserved_intf_mask & (1 << (16 * VFE1 + IFACE_INTF_RDI0)))) {
        planes[i].csid_src = 0;
      } else if ((reserved_intf_mask & (1 << (16 * VFE0 + IFACE_INTF_RDI1))) ||
       (reserved_intf_mask & (1 << (16 * VFE1 + IFACE_INTF_RDI1)))) {
        planes[i].csid_src = 1;
      } else if ((reserved_intf_mask & (1 << (16 * VFE0 + IFACE_INTF_RDI2))) ||
       (reserved_intf_mask & (1 << (16 * VFE1 + IFACE_INTF_RDI2)))) {
        planes[i].csid_src = 2;
      } else {
        planes[i].csid_src = 0;
      }

      if (axi_stream->hw_stream_info.num_cids > 1) {
        planes[i].rdi_cid =
          axi_stream->hw_stream_info.cids[i]; /*CID 1-16*/
      } else {
        planes[i].rdi_cid =
          axi_stream->hw_stream_info.cids[0];/*CID 1-16*/
      }

      if (i == 0) {
        planes[i].output_plane_format = Y_PLANE; /*Y/Cb/Cr/CbCr*/
      } else {
        if ((axi_stream->hw_stream_info.stream_info.fmt ==
          CAM_FORMAT_YUV_420_NV12) ||
          (axi_stream->hw_stream_info.stream_info.fmt ==
          CAM_FORMAT_YUV_420_NV12_VENUS) ||
          (axi_stream->hw_stream_info.stream_info.fmt ==
          CAM_FORMAT_YUV_420_NV12_UBWC) ||
          (axi_stream->hw_stream_info.stream_info.fmt ==
          CAM_FORMAT_YUV_422_NV16) ||
          (axi_stream->hw_stream_info.stream_info.fmt ==
          CAM_FORMAT_YUV_444_NV24)) {
          planes[i].output_plane_format = CBCR_PLANE;
        } else {
          planes[i].output_plane_format = CRCB_PLANE;
        }

        if ((axi_stream->hw_stream_info.stream_info.fmt ==
          CAM_FORMAT_YUV_420_NV12) ||
          (axi_stream->hw_stream_info.stream_info.fmt ==
          CAM_FORMAT_YUV_420_NV12_VENUS) ||
          (axi_stream->hw_stream_info.stream_info.fmt ==
          CAM_FORMAT_YUV_420_NV12_UBWC) ||
          (axi_stream->hw_stream_info.stream_info.fmt ==
            CAM_FORMAT_YUV_420_NV12) ||
            (axi_stream->hw_stream_info.stream_info.fmt ==
            CAM_FORMAT_YUV_420_NV12_VENUS) ||
            (axi_stream->hw_stream_info.stream_info.fmt ==
            CAM_FORMAT_YUV_420_NV21) ||
          (axi_stream->hw_stream_info.stream_info.fmt ==
          CAM_FORMAT_YUV_420_NV21_VENUS)) {
          planes[i].output_height /= 2;
        } else if ((axi_stream->hw_stream_info.stream_info.fmt ==
                  CAM_FORMAT_YUV_444_NV24) ||
                  (axi_stream->hw_stream_info.stream_info.fmt ==
                  CAM_FORMAT_YUV_444_NV42)) {
          planes[i].output_width *= 2;
        }

        if (axi_stream->hw_stream_info.need_uv_subsample) {
          planes[i].output_width  /= 2;
          planes[i].output_height /= 2;
        }
      }
    }
  }
    break;

  case CAM_FORMAT_YUV_420_YV12: {
    /* three panes */
    for (i = 0; i < 3; i++) {
      planes[i].output_width = calculated_output_width; /*Include padding*/
      planes[i].output_height = axi_stream->hw_stream_info.stream_info.dim.height;
      planes[i].output_stride =
        axi_stream->hw_stream_info.plane_info.strides[i];
      planes[i].output_scan_lines =
        axi_stream->hw_stream_info.plane_info.scanline[i];
      planes[i].plane_addr_offset = calculated_plane_addr_offset[i];

      if((reserved_intf_mask & (1 << (16 * VFE0 + IFACE_INTF_RDI0))) ||
       (reserved_intf_mask & (1 << (16 * VFE1 + IFACE_INTF_RDI0)))) {
        planes[i].csid_src = 0;
      } else if ((reserved_intf_mask & (1 << (16 * VFE0 + IFACE_INTF_RDI1))) ||
       (reserved_intf_mask & (1 << (16 * VFE1 + IFACE_INTF_RDI1)))) {
        planes[i].csid_src = 1;
      } else if ((reserved_intf_mask & (1 << (16 * VFE0 + IFACE_INTF_RDI2))) ||
       (reserved_intf_mask & (1 << (16 * VFE1 + IFACE_INTF_RDI2)))) {
        planes[i].csid_src = 2;
      } else {
        planes[i].csid_src = 0;
      }

      if (axi_stream->hw_stream_info.num_cids > 2) {
        planes[i].rdi_cid =
           axi_stream->hw_stream_info.cids[i]; /*CID 1-16*/
      } else {
        planes[i].rdi_cid =
          axi_stream->hw_stream_info.cids[0];/*CID 1-16*/
      }

      if (i == 0) {
        planes[i].output_plane_format = Y_PLANE; /*Y/Cb/Cr/CbCr*/
      } else if (i == 1) {
        planes[i].output_plane_format = CB_PLANE;
      } else {
        planes[i].output_plane_format = CR_PLANE;
      }

      if (i != 0 && axi_stream->hw_stream_info.need_uv_subsample) {
        planes[i].output_width  /= 2;
        planes[i].output_height /= 2;
      }
    }
  }
    break;

  /*added for analysis stream */
  case CAM_FORMAT_Y_ONLY:
  case CAM_FORMAT_Y_ONLY_10_BPP:
  case CAM_FORMAT_Y_ONLY_12_BPP: {
    planes[0].output_width = calculated_output_width; /*Include padding*/
    planes[0].output_height = axi_stream->hw_stream_info.stream_info.dim.height;
    planes[0].output_stride =  axi_stream->hw_stream_info.plane_info.strides[i];
    planes[0].output_scan_lines = axi_stream->hw_stream_info.plane_info.scanline[i];
    planes[0].plane_addr_offset = calculated_plane_addr_offset[0];

    planes[0].csid_src = IFACE_INTF_PIX;

    planes[0].rdi_cid =
      axi_stream->hw_stream_info.cids[0];/*CID 1-16*/
    planes[0].output_plane_format = Y_PLANE;
  }
    break;
  default: {
    /* single plane */
    dwords = iface_axi_util_get_dwords_per_line(
      axi_stream->hw_stream_info.stream_info.fmt);
    planes[0].output_width = calculated_output_width; /*Include padding*/
    planes[0].output_height = axi_stream->hw_stream_info.stream_info.dim.height;
    planes[0].output_stride = axi_stream->hw_stream_info.plane_info.strides[0];
    planes[0].output_scan_lines = axi_stream->hw_stream_info.plane_info.scanline[0];
    planes[0].plane_addr_offset = (calculated_plane_addr_offset[0] * 16)/dwords;

    if((reserved_intf_mask & (1 << (16 * VFE0 + IFACE_INTF_RDI0))) ||
     (reserved_intf_mask & (1 << (16 * VFE1 + IFACE_INTF_RDI0)))) {
      planes[0].csid_src = 0;
    } else if ((reserved_intf_mask & (1 << (16 * VFE0 + IFACE_INTF_RDI1))) ||
     (reserved_intf_mask & (1 << (16 * VFE1 + IFACE_INTF_RDI1)))) {
      planes[0].csid_src = 1;
    } else if ((reserved_intf_mask & (1 << (16 * VFE0 + IFACE_INTF_RDI2))) ||
     (reserved_intf_mask & (1 << (16 * VFE1 + IFACE_INTF_RDI2)))) {
      planes[0].csid_src = 2;
    } else {
      planes[0].csid_src = IFACE_INTF_PIX;
    }

    planes[0].rdi_cid = axi_stream->hw_stream_info.cids[0]; /*CID 1-16*/
    planes[0].output_plane_format = Y_PLANE; /* define for opaque? */

    CDBG_HIGH("%s: single plane: isp_out_intf_mask %x, rdi_cid %d, fmt = %d, "
     "W = %d, H = %d, stride = %d, scanline = %d\n",
     __func__, axi_stream->hw_stream_info.interface_mask,
     planes[0].rdi_cid, axi_stream->hw_stream_info.stream_info.fmt,
     planes[0].output_width, planes[0].output_height,
     planes[0].output_stride, planes[0].output_scan_lines);
    break;
  }
  }

  return 0;
}

/** iface_axi_util_subscribe_v4l2_event:
 *
 *    @axi:
 *    @event_type:
 *    @subscribe:
 *
 **/
int iface_axi_util_subscribe_v4l2_event(iface_axi_hw_t *axi_hw,
  uint32_t event_type, boolean subscribe)
{
  int rc = 0;
  struct v4l2_event_subscription sub;

  CDBG("%s: event_type = 0x%x, is_subscribe = %d",  __func__, event_type,
    subscribe);

  memset(&sub, 0, sizeof(sub));
  sub.type = event_type;

  if(axi_hw->fd < 0)
  {
    CDBG_ERROR("%s: error, axi_hw->fd is incorrect", __func__);
  }

  if (subscribe) {
    rc = ioctl(axi_hw->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
  } else {
    rc = ioctl(axi_hw->fd, VIDIOC_UNSUBSCRIBE_EVENT, &sub);
  }

  if (rc < 0)
    CDBG_ERROR("%s: error, event_type = 0x%x, is_subscribe = %d", __func__,
      event_type, subscribe);

  return rc;
}

/** iface_axi_subscribe_v4l2_event_multi
 *    @isp_hw:
 *    @subscribe:
 *
 **/
int iface_axi_subscribe_v4l2_event_multi(iface_axi_hw_t *axi_hw,
  boolean subscribe)
{
  int rc = 0;
  CDBG("%s:E\n", __func__);

  uint32_t subs_event_type = ISP_EVENT_SUBS_MASK_COMP_STATS_NOTIFY |
                             ISP_EVENT_SUBS_MASK_ERROR |
                             ISP_EVENT_SUBS_MASK_IOMMU_P_FAULT |
                             ISP_EVENT_SUBS_MASK_FE_READ_DONE |
                             ISP_EVENT_SUBS_MASK_STATS_NOTIFY |
                             ISP_EVENT_SUBS_MASK_SOF |
                             ISP_EVENT_SUBS_MASK_REG_UPDATE |
                             ISP_EVENT_SUBS_MASK_STREAM_UPDATE_DONE |
                             ISP_EVENT_SUBS_MASK_BUF_DIVERT |
                             ISP_EVENT_SUBS_MASK_REG_UPDATE_MISSING |
                             ISP_EVENT_SUBS_MASK_PING_PONG_MISMATCH |
#ifdef VIDIOC_MSM_ISP_NANOSEC_TIMESTAMP
                             ISP_EVENT_SUBS_MASK_SOF_UPDATE_NANOSEC |
#endif
                             ISP_EVENT_SUBS_MASK_BUF_FATAL_ERROR ;

  rc = iface_axi_util_subscribe_v4l2_event(axi_hw,
    subs_event_type, subscribe);
  if (rc < 0) {
    CDBG_ERROR("%s:%d failed subscribing event mask= 0x%x\n",
      __func__, __LINE__, subs_event_type);
    if (subscribe) {
      rc = iface_axi_util_subscribe_v4l2_event(axi_hw,
           subs_event_type, FALSE);
      if (rc !=0) {
        CDBG("failed !! iface_axi_util_subscribe_v4l2_event\n");
      }
    }
    return -1;
  }

  return rc;
}

/** iface_axi_prepare_sw_frame_skip_info
 *    @isp_hw:
 *    @skip_pattern:
 *
 **/
int iface_axi_prepare_sw_frame_skip_info(iface_axi_hw_t *axi,
  iface_param_frame_skip_pattern_t *skip_pattern,
  struct msm_vfe_axi_stream_update_cmd *update_cmd)
{
  int i, rc = 0;
  struct msm_isp_sw_framskip *sw_frame_skip = NULL;
  enum msm_vfe_input_src intf;

  if (!skip_pattern->sw_frame_skip_info.skip_image_frames) {
    CDBG_HIGH("%s:E session_id = %d no image skip requested %d\n",
    __func__, skip_pattern->session_id,
    skip_pattern->sw_frame_skip_info.skip_image_frames);
    return -1;
  }

  memset(update_cmd, 0, sizeof(struct msm_vfe_axi_stream_update_cmd));
  update_cmd->update_type = UPDATE_STREAM_SW_FRAME_DROP;

  CDBG("%s:E session_id = %d %d\n",
    __func__, skip_pattern->session_id,
    skip_pattern->sw_frame_skip_info.skip_image_frames);


  for (i = 0; i < IFACE_AXI_STREAM_MAX; i++) {
    intf = SRC_TO_INTF(axi->stream[i].hw_stream_info.axi_path);
    if (axi->stream[i].state == IFACE_AXI_STREAM_STATE_INVALID ||
      intf >= VFE_SRC_MAX ||
      axi->intf_param[intf].session_id != skip_pattern->session_id) {
      continue;
    }
    sw_frame_skip = &update_cmd->update_info[update_cmd->num_streams].sw_skip_info;
  memset(sw_frame_skip, 0, sizeof(struct msm_isp_sw_framskip));
  switch (skip_pattern->sw_frame_skip_info.skip_mode) {
    case IFACE_SKIP_ALL:
      sw_frame_skip->skip_mode = SKIP_ALL;
    break;

    case IFACE_SKIP_RANGE:
      sw_frame_skip->skip_mode = SKIP_RANGE;
    break;

    case IFACE_SKIP_NONE:
    default:
      sw_frame_skip->skip_mode = NO_SKIP;
    break;
  }
  sw_frame_skip->min_frame_id = skip_pattern->sw_frame_skip_info.min_frame_id;
  sw_frame_skip->max_frame_id = skip_pattern->sw_frame_skip_info.max_frame_id;
    update_cmd->update_info[update_cmd->num_streams].stream_handle =
      axi->stream[i].axi_stream_handle;
        sw_frame_skip->stream_src_mask |=
          (1 << axi->stream[i].hw_stream_info.axi_path);
    update_cmd->num_streams++;
  }

  if (sw_frame_skip != NULL) {
      CDBG_HIGH("%s:E sw_frame_skip->stream_src_mask = %x\n",
      __func__, sw_frame_skip->stream_src_mask);
  }
  return rc;
}

/** iface_stats_prepare_sw_frame_skip_info
 *    @isp_hw:
 *    @skip_pattern:
 *
 **/
int iface_stats_prepare_sw_frame_skip_info(iface_axi_hw_t *axi,
  iface_param_frame_skip_pattern_t *skip_pattern,
  struct msm_vfe_axi_stream_update_cmd *update_cmd)
{
  int i, rc = 0;
  struct msm_isp_sw_framskip *sw_frame_skip = NULL;
  if (!skip_pattern->sw_frame_skip_info.skip_stats_frames) {
    CDBG_HIGH("%s:E session_id = %d no stats skip requested %d\n",
    __func__, skip_pattern->session_id,
    skip_pattern->sw_frame_skip_info.skip_stats_frames);
    return -1;
  }

  memset(update_cmd, 0, sizeof(struct msm_vfe_axi_stream_update_cmd));
  update_cmd->update_type = UPDATE_STREAM_SW_FRAME_DROP;

  for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
    if (!axi->stats_stream[i].stream_handle ||
      (axi->stats_stream[i].stats_stream_info.session_id !=
      skip_pattern->session_id)) {
      continue;
    }
    sw_frame_skip = &update_cmd->update_info[update_cmd->num_streams].sw_skip_info;
    memset(sw_frame_skip, 0, sizeof(struct msm_isp_sw_framskip));

    switch (skip_pattern->sw_frame_skip_info.skip_mode) {
      case IFACE_SKIP_ALL:
        sw_frame_skip->skip_mode = SKIP_ALL;
      break;

      case IFACE_SKIP_RANGE:
        sw_frame_skip->skip_mode = SKIP_RANGE;
      break;

      case IFACE_SKIP_NONE:
      default:
        sw_frame_skip->skip_mode = NO_SKIP;
      break;
    }
    sw_frame_skip->min_frame_id = skip_pattern->sw_frame_skip_info.min_frame_id;
    sw_frame_skip->max_frame_id = skip_pattern->sw_frame_skip_info.max_frame_id;

    update_cmd->update_info[update_cmd->num_streams].stream_handle =
      axi->stats_stream[i].stream_handle;
    sw_frame_skip->stats_type_mask |=
      (1 << axi->stats_stream[i].stats_stream_info.stats_type);
    update_cmd->num_streams++;
  }

  if (sw_frame_skip != NULL) {
      CDBG_HIGH("%s:X stats_type_mask %x stream_src_mask = %x num_streams %d\n",
        __func__, sw_frame_skip->stats_type_mask, sw_frame_skip->stream_src_mask,
        update_cmd->num_streams);
  }
  return rc;
}
