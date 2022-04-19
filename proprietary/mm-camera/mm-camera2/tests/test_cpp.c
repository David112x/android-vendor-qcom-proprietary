/*============================================================================

  Copyright (c) 2013, 2016, 2019 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <sys/mman.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <linux/ion.h>
#include <linux/msm_ion.h>
#if TARGET_ION_ABI_VERSION >= 2
#include <ion/ion.h>
#endif

#include <media/msmb_camera.h>
#include <media/msmb_pproc.h>
#include "mct_pipeline.h"
#include "mct_module.h"
#include "mct_stream.h"
#include "modules.h"
#include "pproc_port.h"
#include "cam_intf.h"
#include "camera_dbg.h"
#include <dlfcn.h>
#include <media/msm_media_info.h>
#include <mct_controller.h>

struct v4l2_frame_buffer {
  struct v4l2_buffer buffer;
  unsigned long addr[VIDEO_MAX_PLANES];
  uint32_t size;
  struct ion_allocation_data ion_alloc[VIDEO_MAX_PLANES];
  struct ion_fd_data fd_data[VIDEO_MAX_PLANES];
};

#define OFFSET_PAD(offset, pad) ((offset + pad - 1) & ~(pad - 1))
pthread_cond_t  frame_done_cond;
pthread_mutex_t mutex;
boolean         frame_pending = FALSE;

typedef struct {
  uint32_t     input_width;
  uint32_t     input_height;
  uint32_t     output_width;
  uint32_t     output_height;
  uint32_t     process_window_first_pixel;
  uint32_t     process_window_first_line;
  uint32_t     process_window_width;
  uint32_t     process_window_height;
  uint16_t     rotation;
  uint16_t     flip;
  double       h_scale_ratio;
  double       v_scale_ratio;
  char         input_filename[256];
  char         output_filename[256];
  char         chromatix_file[256];
  double       denoise_enable;
  double       tnr_enable;
  double       sharpness_ratio;
  int32_t      save_file;
  uint32_t     frame_count;
  uint32_t     scale_enable;
  float        lux_idx;
  float        real_gain;
  int          offset_x;
  int          offset_y;
  int          plane_padding;
  int          width_padding;
  int          height_padding;
  int32_t      ubwc;
  int32_t      cds;
} cpp_testcase_input_t;

uint8_t *do_mmap_ion(int ion_fd, struct ion_allocation_data *alloc,
  struct ion_fd_data *ion_info_fd, int *mapFd)
{
  void                  *ret; /* returned virtual address */
  int                    rc = 0;
  /* to make it page size aligned */
  alloc->len = (alloc->len + 4095) & (~4095);
#ifndef TARGET_ION_ABI_VERSION
  struct ion_handle_data handle_data;

  rc = ioctl(ion_fd, ION_IOC_ALLOC, alloc);
  if (rc < 0) {
    CDBG_ERROR("ION allocation failed\n");
    goto ION_ALLOC_FAILED;
  }

  ion_info_fd->handle = alloc->handle;
  rc = ioctl(ion_fd, ION_IOC_SHARE, ion_info_fd);
  if (rc < 0) {
    CDBG_ERROR("ION map failed %s\n", strerror(errno));
    goto ION_MAP_FAILED;
  }
  *mapFd = ion_info_fd->fd;
#else
  rc = ion_alloc_fd(ion_fd, alloc->len, alloc->align, alloc->heap_id_mask,
    alloc->flags, mapFd);
  if (rc < 0) {
    CDBG_ERROR("ION allocation failed\n");
    goto ION_ALLOC_FAILED;
  }
  ion_info_fd->fd = *mapFd;
#endif
  ret = mmap(NULL, alloc->len, PROT_READ | PROT_WRITE, MAP_SHARED, *mapFd, 0);
  if (ret == MAP_FAILED) {
    CDBG_ERROR("ION_MMAP_FAILED: %s (%d)\n", strerror(errno), errno);
    goto ION_MAP_FAILED;
  }

  return ret;

ION_MAP_FAILED:
#ifndef TARGET_ION_ABI_VERSION
  handle_data.handle = ion_info_fd->handle;
  ioctl(ion_fd, ION_IOC_FREE, &handle_data);
#else
  close(ion_info_fd->fd);
#endif
ION_ALLOC_FAILED:
  return NULL;
}

int do_munmap_ion(int ion_fd, struct ion_fd_data *ion_info_fd, void *addr,
  size_t size)
{
  int rc = 0;

  rc = munmap(addr, size);
  close(ion_info_fd->fd);
#ifndef TARGET_ION_ABI_VERSION
  struct ion_handle_data handle_data;
  handle_data.handle = ion_info_fd->handle;
  ioctl(ion_fd, ION_IOC_FREE, &handle_data);
#else
  (void)ion_fd;
#endif
  return rc;
}

void dump_test_case_params(cpp_testcase_input_t *test_case)
{
  int i;
  CDBG_ERROR("input_width: %d\n", test_case->input_width);
  CDBG_ERROR("input_height: %d\n", test_case->input_height);
  CDBG_ERROR("process_window_first_pixel: %d\n", test_case->process_window_first_pixel);
  CDBG_ERROR("process_window_first_line: %d\n", test_case->process_window_first_line);
  CDBG_ERROR("process_window_width: %d\n", test_case->process_window_width);
  CDBG_ERROR("process_window_height: %d\n", test_case->process_window_height);
  CDBG_ERROR("rotation: %d\n", test_case->rotation);
  CDBG_ERROR("flip: %d\n", test_case->flip);
  CDBG_ERROR("h_scale_ratio: %f\n", test_case->h_scale_ratio);
  CDBG_ERROR("v_scale_ratio: %f\n", test_case->v_scale_ratio);
  CDBG_ERROR("input_filename: %s\n", test_case->input_filename);
  CDBG_ERROR("output_filename: %s\n", test_case->output_filename);
  CDBG_ERROR("denoise_enable: %f\n", test_case->denoise_enable);
  CDBG_ERROR("sharpness_ratio: %f\n", test_case->sharpness_ratio);
  CDBG_ERROR("save_file: %d\n", test_case->save_file);
  CDBG_ERROR("output_width: %d\n", test_case->output_width);
  CDBG_ERROR("output_height: %d\n", test_case->output_height);
  CDBG_ERROR("ubwc: %d\n", test_case->ubwc);
  CDBG_ERROR("tnr_enable: %f\n", test_case->tnr_enable);
  CDBG_ERROR("width_padding: %d\n", test_case->width_padding);
  CDBG_ERROR("height_padding: %d\n", test_case->height_padding);
  CDBG_ERROR("plane_padding: %d\n", test_case->plane_padding);
}

int parse_test_case_file(char **argv, cpp_testcase_input_t *test_case)
{
  char *filename = argv[1];
  char  type[256], value[256];
  FILE *fp;

  CDBG_HIGH("%s:%d] file name: %s\n", __func__, __LINE__, filename);

  fp = fopen(filename, "r");
  if (fp == NULL) {
    CDBG_ERROR("%s:%d] Cannot open test case file!\n", __func__, __LINE__);
    return -1;
  }
  while (!feof(fp)) {
    if (fscanf(fp, "%s %s", type, value) != 2)
      break;

    if (!strncmp(type, "input_width", 256)) {
      test_case->input_width = atoi(value);
    } else if (!strncmp(type, "input_height", 256)) {
      test_case->input_height = atoi(value);
    }else if (!strncmp(type, "output_width", 256)) {
      test_case->output_width = atoi(value);
    } else if (!strncmp(type, "output_height", 256)) {
      test_case->output_height = atoi(value);
    } else if (!strncmp(type, "process_window_first_pixel", 256)) {
      test_case->process_window_first_pixel = atoi(value);
    } else if (!strncmp(type, "process_window_first_line", 256)) {
      test_case->process_window_first_line = atoi(value);
    } else if (!strncmp(type, "process_window_width", 256)) {
      test_case->process_window_width = atoi(value);
    } else if (!strncmp(type, "process_window_height", 256)) {
      test_case->process_window_height = atoi(value);
    } else if (!strncmp(type, "rotation", 256)) {
      test_case->rotation = atoi(value);
    } else if (!strncmp(type, "flip", 256)) {
      test_case->flip = atoi(value);
    } else if (!strncmp(type, "h_scale_ratio", 256)) {
      test_case->h_scale_ratio = atof(value);
    } else if (!strncmp(type, "v_scale_ratio", 256)) {
      test_case->v_scale_ratio = atof(value);
    } else if (!strncmp(type, "input_filename", 256)) {
      strlcpy(test_case->input_filename, value, 256);
    } else if (!strncmp(type, "output_filename", 256)) {
      strlcpy(test_case->output_filename, value, 256);
    } else if (!strncmp(type, "chromatix_file", 256)) {
      strlcpy(test_case->chromatix_file, value, 256);
    } else if (!strncmp(type, "denoise_enable", 256)) {
      test_case->denoise_enable = atoi(value);
    } else if (!strncmp(type, "tnr_enable", 256)) {
      test_case->tnr_enable = atoi(value);
    } else if (!strncmp(type, "save_file", 256)) {
      test_case->save_file = atoi(value);
    } else if (!strncmp(type, "sharpness_ratio", 256)) {
      test_case->sharpness_ratio = atof(value);
    } else if (!strncmp(type, "frame_count", 256)) {
      test_case->frame_count = atoi(value);
    } else if (!strncmp(type, "scale_enable", 256)) {
      test_case->scale_enable = atoi(value);
    } else if (!strncmp(type, "lux_idx", 256)) {
      test_case->lux_idx = atof(value);
    } else if (!strncmp(type, "real_gain", 256)) {
      test_case->real_gain = atof(value);
    } else if (!strncmp(type, "offset_x", 256)) {
      test_case->offset_x = atoi(value);
    } else if (!strncmp(type, "offset_y", 256)) {
      test_case->offset_y = atoi(value);
    } else if (!strncmp(type, "plane_padding", 256)) {
      test_case->plane_padding = atoi(value);
    } else if (!strncmp(type, "width_padding", 256)) {
      test_case->width_padding = atoi(value);
    } else if (!strncmp(type, "height_padding", 256)) {
      test_case->height_padding = atoi(value);
    } else if (!strncmp(type, "ubwc", 256)) {
      test_case->ubwc = atoi(value);
    } else if (!strncmp(type, "cds", 256)) {
      test_case->cds = atoi(value);
    }
  }
  dump_test_case_params(test_case);
  return 0;
}

#define PPROC_TEST_INPUT_WIDTH 640
#define PPROC_TEST_INPUT_HEIGHT 480
#define PPROC_TEST_ALIGN_4K 4096

static boolean pproc_test_port_event(mct_port_t *port __unused, mct_event_t *event)
{
  switch(event->type) {
  case MCT_EVENT_MODULE_EVENT: {
    switch(event->u.module_event.type) {
    case MCT_EVENT_MODULE_BUF_DIVERT_ACK:
      pthread_mutex_lock(&mutex);
      frame_pending = FALSE;
      pthread_cond_signal(&frame_done_cond);
      pthread_mutex_unlock(&mutex);
      break;
    default:
      break;
    }
    break;
  }
  default:
    break;
  }
  return TRUE;
}

static boolean pproc_test_create_stream_info(unsigned int identity,
  mct_stream_info_t *stream_info, mct_list_t *img_buf_list,
  cpp_testcase_input_t *test_case, metadata_buffer_t *metadata)
{
  cam_pp_offline_src_config_t *offline_src_cfg;
  cam_pp_feature_config_t     *pp_feature_config;
  uint32_t i = 0;

  stream_info->identity = identity;
  if (test_case->ubwc)
    stream_info->fmt = CAM_FORMAT_YUV_420_NV12_UBWC;
  else
    stream_info->fmt = CAM_FORMAT_YUV_420_NV12;
  stream_info->stream_type = CAM_STREAM_TYPE_OFFLINE_PROC;
  stream_info->streaming_mode = CAM_STREAMING_MODE_BURST;
  stream_info->num_burst = 1;
  stream_info->img_buffer_list = img_buf_list;

  stream_info->buf_planes.plane_info.num_planes = 2;
  stream_info->reprocess_config.pp_type = CAM_OFFLINE_REPROCESS_TYPE;
  if (test_case->output_width && test_case->output_height) {
    int stride = 0, scanline = 0, meta_stride = 0, meta_scanline = 0;
    stream_info->dim.width = test_case->output_width;
    stream_info->dim.height = test_case->output_height;
    if (test_case->ubwc) {
      stride = VENUS_Y_STRIDE(COLOR_FMT_NV12_UBWC, test_case->output_width);
      scanline = VENUS_Y_SCANLINES(COLOR_FMT_NV12_UBWC, test_case->output_height);
      meta_stride = VENUS_Y_META_STRIDE(COLOR_FMT_NV12_UBWC, test_case->output_width);
      meta_scanline = VENUS_Y_META_SCANLINES(COLOR_FMT_NV12_UBWC, test_case->output_height);
      stream_info->buf_planes.plane_info.frame_len =
        VENUS_BUFFER_SIZE(COLOR_FMT_NV12_UBWC, test_case->output_width, test_case->output_height);
    } else {
      //luma stride
      stride =  OFFSET_PAD(test_case->output_width, test_case->width_padding);
      scanline = OFFSET_PAD(test_case->output_height, test_case->height_padding);
      meta_stride = 0;
      meta_scanline = 0;
    }
    stream_info->buf_planes.plane_info.mp[0].offset = 0;
    stream_info->buf_planes.plane_info.mp[0].stride = stride;
    stream_info->buf_planes.plane_info.mp[0].scanline = scanline;
    stream_info->buf_planes.plane_info.mp[0].offset_x = 0;
    stream_info->buf_planes.plane_info.mp[0].offset_y = 0;
    stream_info->buf_planes.plane_info.mp[0].width = test_case->output_width;
    stream_info->buf_planes.plane_info.mp[0].height = test_case->output_height;
    stream_info->buf_planes.plane_info.mp[0].meta_stride = meta_stride;
    stream_info->buf_planes.plane_info.mp[0].meta_scanline = meta_scanline;
    stream_info->buf_planes.plane_info.mp[0].meta_len =
      MSM_MEDIA_ALIGN(meta_stride * meta_scanline, 4096);
    if (test_case->ubwc) {
      stream_info->buf_planes.plane_info.mp[0].len =
        (uint32_t)(MSM_MEDIA_ALIGN((stride * scanline), 4096) +
        (stream_info->buf_planes.plane_info.mp[0].meta_len));
      stride = VENUS_UV_STRIDE(COLOR_FMT_NV12_UBWC, test_case->output_width);
      scanline = VENUS_UV_SCANLINES(COLOR_FMT_NV12_UBWC, test_case->output_height);
      meta_stride = VENUS_UV_META_STRIDE(COLOR_FMT_NV12_UBWC,
        test_case->output_width);
      meta_scanline = VENUS_UV_META_SCANLINES(COLOR_FMT_NV12_UBWC,
        test_case->output_height);
      stream_info->buf_planes.plane_info.mp[1].len =
        (uint32_t)(MSM_MEDIA_ALIGN((stride * scanline), 4096) +
        (stream_info->buf_planes.plane_info.mp[1].meta_len));
    } else {
      stream_info->buf_planes.plane_info.mp[0].len = stride * scanline;
      //chroma stride
      stride =  OFFSET_PAD(test_case->output_width, test_case->width_padding);
      scanline = OFFSET_PAD(test_case->output_height/2, test_case->height_padding);
      stream_info->buf_planes.plane_info.mp[1].len = stride * scanline;
      stream_info->buf_planes.plane_info.frame_len =
        OFFSET_PAD((stream_info->buf_planes.plane_info.mp[0].len +
        stream_info->buf_planes.plane_info.mp[1].len), 4096);
      meta_stride = 0;
      meta_scanline = 0;
    }

    stream_info->buf_planes.plane_info.mp[1].offset = 0;
    stream_info->buf_planes.plane_info.mp[1].stride = stride;
    stream_info->buf_planes.plane_info.mp[1].scanline = scanline;
    stream_info->buf_planes.plane_info.mp[1].offset_x = 0;
    stream_info->buf_planes.plane_info.mp[1].offset_y = 0;

    stream_info->buf_planes.plane_info.mp[1].width = test_case->output_width;
    stream_info->buf_planes.plane_info.mp[1].height = test_case->output_height / 2;
    stream_info->buf_planes.plane_info.mp[1].meta_stride = meta_stride;
    stream_info->buf_planes.plane_info.mp[1].meta_scanline = meta_scanline;
    stream_info->buf_planes.plane_info.mp[1].meta_len =
      MSM_MEDIA_ALIGN(meta_stride * meta_scanline, 4096);
    CDBG_ERROR("%s: luma: width %d, height %d,stride %d, scanline %d, meta_stride %d,"
      "meta_scanline %d meta_len %d , frame_len %d", __func__,
      stream_info->buf_planes.plane_info.mp[0].width,
      stream_info->buf_planes.plane_info.mp[0].height,
      stream_info->buf_planes.plane_info.mp[0].stride,
      stream_info->buf_planes.plane_info.mp[0].scanline,
      stream_info->buf_planes.plane_info.mp[0].meta_stride,
      stream_info->buf_planes.plane_info.mp[0].meta_scanline,
      stream_info->buf_planes.plane_info.mp[0].meta_len,
      stream_info->buf_planes.plane_info.mp[0].len);
    CDBG_ERROR("%s: chroma: width %d, height %d,stride %d, scanline %d, meta_stride %d,"
      "meta_scanline %d meta_len %d , frame_len %d", __func__,
      stream_info->buf_planes.plane_info.mp[1].width,
      stream_info->buf_planes.plane_info.mp[1].height,
      stream_info->buf_planes.plane_info.mp[1].stride,
      stream_info->buf_planes.plane_info.mp[1].scanline,
      stream_info->buf_planes.plane_info.mp[1].meta_stride,
      stream_info->buf_planes.plane_info.mp[1].meta_scanline,
      stream_info->buf_planes.plane_info.mp[1].meta_len,
      stream_info->buf_planes.plane_info.mp[1].len);
  }


  offline_src_cfg = &stream_info->reprocess_config.offline;
  offline_src_cfg->num_of_bufs = 1;
  offline_src_cfg->input_fmt = CAM_FORMAT_YUV_420_NV12;
  offline_src_cfg->input_buf_planes.plane_info.num_planes = 2;
  if (test_case->input_width && test_case->input_height) {
    offline_src_cfg->input_dim.width = test_case->input_width;
    offline_src_cfg->input_dim.height = test_case->input_height;
    CDBG_ERROR("### width %d, height %d ### TESTAPP",
      offline_src_cfg->input_dim.width, offline_src_cfg->input_dim.height);
    for (i = 0; i < offline_src_cfg->input_buf_planes.plane_info.num_planes;
      i++) {
      offline_src_cfg->input_buf_planes.plane_info.mp[i].stride =
      test_case->input_width + (2 * test_case->offset_x);
    offline_src_cfg->input_buf_planes.plane_info.mp[i].scanline =
      ((test_case->input_height/(i+1)) + (2 * test_case->offset_y));
    offline_src_cfg->input_buf_planes.plane_info.mp[i].offset =
      (offline_src_cfg->input_buf_planes.plane_info.mp[i].stride *
      test_case->offset_y) + test_case->offset_x; //remove padding issue
     //   offline_src_cfg->input_buf_planes.plane_info.mp[i].stride * 32;
    offline_src_cfg->input_buf_planes.plane_info.mp[i].offset_x = test_case->offset_x;
    offline_src_cfg->input_buf_planes.plane_info.mp[i].offset_y = test_case->offset_y;
    offline_src_cfg->input_buf_planes.plane_info.mp[i].len =
    offline_src_cfg->input_buf_planes.plane_info.mp[i].stride *
      offline_src_cfg->input_buf_planes.plane_info.mp[i].scanline;
   CDBG_ERROR("%s:stride %d,scanline %d,offset %d,offsetx %d,offsety %d,len %d",
     __func__, offline_src_cfg->input_buf_planes.plane_info.mp[i].stride,
     offline_src_cfg->input_buf_planes.plane_info.mp[i].scanline,
     offline_src_cfg->input_buf_planes.plane_info.mp[i].offset,
     offline_src_cfg->input_buf_planes.plane_info.mp[i].offset_x,
     offline_src_cfg->input_buf_planes.plane_info.mp[i].offset_y,
     offline_src_cfg->input_buf_planes.plane_info.mp[i].len);
    }
  } else {
    offline_src_cfg->input_dim.width = PPROC_TEST_INPUT_WIDTH;
    offline_src_cfg->input_dim.height = PPROC_TEST_INPUT_HEIGHT;
    for (i = 0 ; i < 2; i++) {
    offline_src_cfg->input_buf_planes.plane_info.mp[i].stride =
      PPROC_TEST_INPUT_WIDTH +  (2 * test_case->offset_x);
    offline_src_cfg->input_buf_planes.plane_info.mp[i].scanline =
      (PPROC_TEST_INPUT_HEIGHT/(i+1)) + (2 * test_case->offset_y);
    offline_src_cfg->input_buf_planes.plane_info.mp[i].offset =
      (offline_src_cfg->input_buf_planes.plane_info.mp[i].stride *
      (test_case->offset_y)) + test_case->offset_x;
    offline_src_cfg->input_buf_planes.plane_info.mp[i].offset_x = test_case->offset_x;
    offline_src_cfg->input_buf_planes.plane_info.mp[i].offset_y = test_case->offset_y;
    offline_src_cfg->input_buf_planes.plane_info.mp[i].len =
    offline_src_cfg->input_buf_planes.plane_info.mp[i].stride *
      offline_src_cfg->input_buf_planes.plane_info.mp[i].scanline;
   }
  }

  pp_feature_config = &stream_info->reprocess_config.pp_feature_config;
  if (test_case->denoise_enable) {
    pp_feature_config->feature_mask |= CAM_QCOM_FEATURE_DENOISE2D;
    pp_feature_config->denoise2d.denoise_enable = 1;
  }
  if (test_case->tnr_enable) {
    pp_feature_config->feature_mask |= CAM_QCOM_FEATURE_CPP_TNR;
    //pp_feature_config->tnr.denoise_enable = 1;
  }
  if (test_case->sharpness_ratio) {
    pp_feature_config->feature_mask |= CAM_QCOM_FEATURE_SHARPNESS;
    pp_feature_config->sharpness = (uint32_t) test_case->sharpness_ratio;
  }

  if (test_case->rotation) {
    cam_rotation_info_t rotation_info;
    pp_feature_config->feature_mask |= CAM_QCOM_FEATURE_ROTATION;
    if (test_case->rotation > 3)
      test_case->rotation = 3;
    pp_feature_config->rotation = 1 << test_case->rotation;
    if (test_case->rotation == 1 || test_case->rotation == 3) {
      uint32_t swap_dim;
      swap_dim = stream_info->dim.width;
      stream_info->dim.width = stream_info->dim.height;
      stream_info->dim.height = swap_dim;
      swap_dim = stream_info->buf_planes.plane_info.mp[0].stride;
      stream_info->buf_planes.plane_info.mp[0].stride =
        stream_info->buf_planes.plane_info.mp[0].scanline;
      stream_info->buf_planes.plane_info.mp[0].scanline = swap_dim;
    }
    rotation_info.rotation = pp_feature_config->rotation;
    add_metadata_entry(CAM_INTF_PARM_ROTATION, sizeof(cam_rotation_info_t),
      &rotation_info, metadata);
  }

  if (test_case->process_window_height && test_case->process_window_width) {
    pp_feature_config->feature_mask |= CAM_QCOM_FEATURE_CROP;
  }

  if (test_case->scale_enable) {
    pp_feature_config->feature_mask |= CAM_QCOM_FEATURE_SCALE;
  }

  if (test_case->flip) {
    pp_feature_config->feature_mask |= CAM_QCOM_FEATURE_FLIP;
    pp_feature_config->flip = test_case->flip;
    add_metadata_entry(CAM_INTF_PARM_FLIP, sizeof(int32_t), &test_case->flip,
      metadata);
  }

  stream_info->stream = mct_stream_new(identity & 0x0000FFFF);
  if(!stream_info->stream) {
    CDBG_ERROR("mct_stream_new failed\n");
    return FALSE;
  }
  stream_info->stream->streaminfo = *stream_info;
  return TRUE;
}

static boolean pproc_test_destroy_stream_info(mct_stream_info_t *stream_info)
{
  pthread_mutex_destroy(MCT_OBJECT_GET_LOCK(stream_info->stream));
  free(stream_info->stream);
  return TRUE;
}

int main(int argc, char * argv[])
{
  boolean                  rc = FALSE;
  uint32_t                 i;
  int32_t                  main_ret = -1;
  cpp_testcase_input_t     test_case;
  int                      ionfd = 0;
  struct v4l2_frame_buffer in_frame, out_frame;
  int                      in_frame_fd = 0, out_frame_fd = 0;
  int                      read_len = 0;
  int                      in_file_fd, out_file_fd;
  char                     out_fname[256];
  mct_module_t            *pproc;
  mct_port_t              *pproc_port = NULL, *test_port = NULL;
  mct_stream_info_t        stream_info;
  unsigned int             identity;
  mct_event_t              event;
  mct_stream_map_buf_t     img_buf_input, img_buf_output, meta_buf;
  mct_list_t              *img_buf_list = NULL, *list;
  cam_stream_parm_buffer_t parm_buf;
  mct_pipeline_t          *pipeline = NULL;
  void                    *chromatixCpp_header = NULL;
  void                    *chromatixCppPtr = NULL;
  mct_stream_session_metadata_info *priv_metadata;
  metadata_buffer_t       *metadata = NULL;
  mct_controller_t * mct = NULL;

  void *(*open_lib)(void);
  stats_get_data_t        stats_get;
  memset(&stats_get, 0, sizeof(stats_get_data_t));

  pthread_mutex_init(&mutex, NULL);
  if (argc > 1) {
    main_ret = parse_test_case_file(argv, &test_case);
    if (main_ret < 0)
      return main_ret;
  } else {
    CDBG_ERROR("%s:%d] Usage: cpp-test-app <test case file>\n", __func__,
      __LINE__);
    goto EXIT1;
  }

  in_file_fd = open(test_case.input_filename, O_RDWR | O_CREAT, 0777);
  if (in_file_fd < 0) {
    CDBG_ERROR("%s:%d] Cannot open input file\n", __func__, __LINE__);
    goto EXIT1;
  }

  chromatixCpp_header = dlopen((const char *)test_case.chromatix_file, RTLD_NOW);
  if(!chromatixCpp_header) {
    CDBG_ERROR("Error opening chromatix file %s \n",test_case.chromatix_file);
  } else {
    *(void **)&open_lib = dlsym(chromatixCpp_header, "load_chromatix");
    if (!open_lib) {
      CDBG_ERROR("Fail to find symbol %s",dlerror());
    } else {
      chromatixCppPtr = open_lib();
    }
  }

  pthread_cond_init(&frame_done_cond, NULL);

  /* open ion device */
  ionfd = open("/dev/ion", O_RDONLY | O_SYNC);
  if (ionfd < 0) {
    CDBG_ERROR("%s:%d] Ion device open failed\n", __func__, __LINE__);
    goto EXIT2;
  }

  if (!test_case.input_width || !test_case.input_height) {
    test_case.input_width = PPROC_TEST_INPUT_WIDTH;
    test_case.input_height = PPROC_TEST_INPUT_HEIGHT;
    test_case.offset_x = 0;
    test_case.offset_y = 0;
  }
  if (!test_case.output_width || !test_case.output_height) {
    test_case.output_width = PPROC_TEST_INPUT_WIDTH;
    test_case.output_height = PPROC_TEST_INPUT_HEIGHT;
  }
  int ipstride = 0, ipscanline = 0, ipbuf_len = 0;
    if (test_case.offset_x != 0)
      test_case.offset_x = OFFSET_PAD(test_case.offset_x,
        test_case.plane_padding);
    if (test_case.offset_y != 0)
      test_case.offset_y = OFFSET_PAD(test_case.offset_y,
        test_case.plane_padding);
    ipstride = test_case.input_width + 2 * test_case.offset_x;
    //Include both planes
    ipscanline = (test_case.input_height) + 2 * test_case.offset_y;
    ipbuf_len = ipstride * ((test_case.input_height * 1.5f) + 2 * test_case.offset_y);
    CDBG_ERROR("%s: width %d,height %d,offsetx %d,offsety %d,plane padding %d,"
      "stride %d scanline %d,buf len %d",
      __func__, test_case.input_width, test_case.input_height,
      test_case.offset_x, test_case.offset_y, test_case.plane_padding,
      ipstride, ipscanline, ipbuf_len);


  int opstride = 0, opscanline = 0, opbuf_len = 0;
  if (test_case.ubwc) {
    CDBG_ERROR("%s: UBWC ENABLED %d", __func__, test_case.ubwc);
    opbuf_len = VENUS_BUFFER_SIZE(COLOR_FMT_NV12_UBWC, test_case.output_width, test_case.output_height);
    CDBG_ERROR("%s: width %d,height %d,offsetx %d,offsety %d,plane padding %d,"
      "buf len %d", __func__, test_case.input_width, test_case.input_height,
      test_case.offset_x, test_case.offset_y, test_case.plane_padding,
      opbuf_len);
  } else {

    //Luma plane
    opstride =  OFFSET_PAD(test_case.output_width, test_case.width_padding);
    opscanline = OFFSET_PAD(test_case.output_height, test_case.height_padding);
    opbuf_len = opstride * opscanline;
    CDBG_ERROR("%s: luma o/p  width %d,height %d,offsetx %d,offsety %d,plane padding %d,"
      "stride %d scanline %d, buf len %d", __func__, test_case.output_width,
      test_case.output_height, test_case.offset_x, test_case.offset_y,
      test_case.plane_padding, opstride, opscanline, opbuf_len);

    //Chroma plane
    opstride =  OFFSET_PAD(test_case.output_width, test_case.width_padding);
    opscanline = OFFSET_PAD(test_case.output_height/2, test_case.height_padding);
    opbuf_len += opstride * opscanline;
    OFFSET_PAD(opbuf_len, CAM_PAD_TO_4K);
    CDBG_ERROR("%s: chroma o/p width %d,height %d,offsetx %d,offsety %d,plane padding %d,"
      "stride %d scanline %d, buf len %d", __func__, test_case.output_width,
      test_case.output_height, test_case.offset_x, test_case.offset_y,
      test_case.plane_padding, opstride, opscanline, opbuf_len);
  }

  /* Create input buffer */
  memset(&in_frame, 0, sizeof(struct v4l2_frame_buffer));
  if (test_case.input_width && test_case.input_height) {
    in_frame.ion_alloc[0].len = ipbuf_len;
    CDBG_HIGH("%s:%d] width %d, height %d, stride %d, scanline %d, len %d",
       __func__, __LINE__, test_case.input_width, test_case.input_height,
       ipstride, ipscanline, in_frame.ion_alloc[0].len);
  } else {
    CDBG_ERROR("INVALID width and height");
    goto EXIT3;
  }

#ifndef TARGET_ION_ABI_VERSION
  in_frame.ion_alloc[0].heap_id_mask = (0x1 << ION_IOMMU_HEAP_ID);
#else
  in_frame.ion_alloc[0].heap_id_mask = (0x1 << ION_SYSTEM_HEAP_ID);
#endif
  in_frame.ion_alloc[0].align = PPROC_TEST_ALIGN_4K;
  in_frame.addr[0] = (unsigned long)do_mmap_ion(ionfd,
    &(in_frame.ion_alloc[0]), &(in_frame.fd_data[0]), &in_frame_fd);
  if (!in_frame.addr[0]) {
    CDBG_ERROR("%s:%d] error mapping input ion fd\n", __func__, __LINE__);
    goto EXIT3;
  }

  int length_plane0 = ipstride * ipscanline;
  memset(&img_buf_input, 0, sizeof(mct_stream_map_buf_t));
  img_buf_input.buf_planes[0].buf = (void *)in_frame.addr[0];
  img_buf_input.buf_planes[1].buf = (void *)(in_frame.addr[0] + length_plane0);
  img_buf_input.buf_planes[0].fd = in_frame_fd;
  img_buf_input.num_planes = 2;
  img_buf_input.buf_index = 0;
  img_buf_input.buf_type = CAM_MAPPING_BUF_TYPE_OFFLINE_INPUT_BUF;
  img_buf_input.common_fd = TRUE;
  list = mct_list_append(img_buf_list, &img_buf_input, NULL, NULL);
  if (!list) {
    CDBG_ERROR("%s:%d] error appending input buffer\n", __func__, __LINE__);
    goto EXIT4;
  }
  img_buf_list = list;
  uint32_t k, len;
  int8_t *vaddr = (int8_t *)in_frame.addr[0];
  int8_t *plane_addr = vaddr;
  /* Read from input file */
  vaddr += test_case.offset_x + (test_case.offset_y * ipstride);
  for (k = 0; k < (test_case.input_height); k++) {
      len = read(in_file_fd, (void *)(vaddr), test_case.input_width);
      vaddr += ipstride;
      read_len += len;
  }
  vaddr += (ipstride * (2 * test_case.offset_y));
    (2 * test_case.offset_x);
  for (k = 0; k < ((test_case.input_height / 2)); k++) {
      len = read(in_file_fd, (void *) (vaddr), test_case.input_width);
      vaddr += ipstride;
      read_len += len;
  }
  if (read_len != (int)in_frame.ion_alloc[0].len) {
    CDBG_ERROR("%s:%d] Copy input image failed read_len%d, file_len:%d\n",
      __func__, __LINE__, read_len, in_frame.ion_alloc[0].len);
    //goto EXIT5;
  }

#if ENABLE_INPUT_DUMP
  FILE *fp;
  fp = fopen("/data/input_dump.yuv", "ab");
  if (fp)
    fwrite ((void *)in_frame.addr[0],1,in_frame.ion_alloc[0].len,fp);
  if (fp)
    fclose(fp);
#endif
  /* Create output buffer */
  memset(&out_frame, 0, sizeof(struct v4l2_frame_buffer));
  if (test_case.output_width && test_case.output_height) {
    out_frame.ion_alloc[0].len = opbuf_len;
  } else {
    CDBG_ERROR("INVALID width and height");
    goto EXIT5;
  }

#ifndef TARGET_ION_ABI_VERSION
  out_frame.ion_alloc[0].heap_id_mask = (0x1 << ION_IOMMU_HEAP_ID);
#else
  out_frame.ion_alloc[0].heap_id_mask = (0x1 << ION_SYSTEM_HEAP_ID);
#endif
  out_frame.ion_alloc[0].align = PPROC_TEST_ALIGN_4K;
  out_frame.addr[0] = (unsigned long)do_mmap_ion(ionfd,
    &(out_frame.ion_alloc[0]), &(out_frame.fd_data[0]), &out_frame_fd);
  memset((void *) out_frame.addr[0], 0x0, out_frame.ion_alloc[0].len);
  if (!out_frame.addr[0]) {
    CDBG_ERROR("%s:%d] error mapping output ion fd\n", __func__, __LINE__);
    goto EXIT5;
  }
  memset(&img_buf_output, 0, sizeof(mct_stream_map_buf_t));
  img_buf_output.buf_planes[0].buf = (void *)out_frame.addr[0];
  img_buf_output.buf_planes[0].fd = out_frame_fd;
  img_buf_output.num_planes = 2;
  img_buf_output.buf_index = 1;
  img_buf_output.buf_type = CAM_MAPPING_BUF_TYPE_STREAM_BUF;
  img_buf_output.common_fd = TRUE;
  list = mct_list_append(img_buf_list, &img_buf_output, NULL, NULL);
  if (!list) {
    CDBG_ERROR("%s:%d] error appending output buffer\n", __func__, __LINE__);
    goto EXIT6;
  }
  img_buf_list = list;


  metadata = malloc(sizeof(metadata_buffer_t));
  if(metadata == NULL) {
    CDBG_ERROR("Fail to allocate metadata buffer\n");
  } else {
    priv_metadata = (mct_stream_session_metadata_info *)
      POINTER_OF_META(CAM_INTF_META_PRIVATE_DATA, metadata);
    priv_metadata->sensor_data.cpp_chromatix_ptr = chromatixCppPtr;
    stats_get.aec_get.lux_idx = test_case.lux_idx;
    stats_get.aec_get.real_gain[0] = test_case.real_gain;
    memcpy(&priv_metadata->stats_aec_data.private_data, &stats_get,
      sizeof(stats_get_data_t));
    memset(&meta_buf, 0, sizeof(mct_stream_map_buf_t));
    meta_buf.buf_planes[0].buf = (void *)metadata;
    meta_buf.buf_planes[0].fd = 0;
    meta_buf.num_planes = 0;
    meta_buf.buf_index = 2;
    meta_buf.buf_type = CAM_MAPPING_BUF_TYPE_OFFLINE_META_BUF;
    meta_buf.common_fd = TRUE;
    list = mct_list_append(img_buf_list, &meta_buf, NULL, NULL);
    if (!list) {
      CDBG_ERROR("%s:%d] error appending meta buffer\n", __func__, __LINE__);
    }
    img_buf_list = list;
  }

  /* Start session on pproc */
  pproc = (mct_module_t *)pproc_module_init("pproc");
  if (!pproc) {
    CDBG_ERROR("%s:%d] error getting pproc module\n", __func__, __LINE__);
    goto EXIT7;
  }
  identity = pack_identity(0x0003, 0x0003);
  pproc->set_mod(pproc, MCT_MODULE_FLAG_SOURCE, identity);
  rc = pproc->start_session(pproc, 0x0003);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error starting session in pproc\n", __func__, __LINE__);
    goto EXIT8;
  }

  /* Create a test port and set function handles */
  test_port = mct_port_create("test_port");
  if (!test_port) {
    CDBG_ERROR("%s:%d] error creating test port", __func__, __LINE__);
    goto EXIT9;
  }
  mct_port_set_event_func(test_port, pproc_test_port_event);

  test_port->caps.port_caps_type = MCT_PORT_CAPS_FRAME;

  mct = (mct_controller_t *)malloc(sizeof(mct_controller_t));
  if (!mct)
     goto EXIT10;
  ALOGE("%s: ### Creating new mct_controller", __func__);
  /* Create pipeline */
  pipeline = mct_pipeline_new(0x0003, mct);
  if (!pipeline) {
    CDBG_ERROR("%s:%d] error creating pipeline", __func__, __LINE__);
    goto EXIT10;
  }

  /* Create stream info */

  memset(&stream_info, 0, sizeof(mct_stream_info_t));
  rc = pproc_test_create_stream_info(identity, &stream_info,
    img_buf_list, &test_case, metadata);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error creating stream info\n", __func__, __LINE__);
    goto EXIT11;
  }

  /* Add stream to pipeline */
  rc = mct_object_set_parent(MCT_OBJECT_CAST(stream_info.stream),
    MCT_OBJECT_CAST(pipeline));
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error adding stream to pipeline\n", __func__, __LINE__);
    goto EXIT12;
  }

  /* Caps reserve on pproc module */
  pproc_port = pproc_port_resrv_port_on_module(pproc, &stream_info,
    MCT_PORT_SINK, test_port);
  if (!pproc_port) {
    CDBG_ERROR("%s:%d] error reserving pproc port\n", __func__, __LINE__);
    goto EXIT13;
  }

  /* Ext link on pproc port */
  rc = pproc_port->ext_link(identity, pproc_port, test_port);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error linking pproc port\n", __func__, __LINE__);
    goto EXIT14;
  }

  rc = mct_port_add_child(identity, pproc_port);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error adding identity to port\n", __func__, __LINE__);
    goto EXIT15;
  }

  rc = mct_object_set_parent(MCT_OBJECT_CAST(pproc),
    MCT_OBJECT_CAST(stream_info.stream));
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error adding pproc to stream\n", __func__, __LINE__);
    goto EXIT16;
  }

  /* Stream on event */
  memset(&event, 0, sizeof(mct_event_t));
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_CONTROL_CMD;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_STREAMON;
  event.u.ctrl_event.control_event_data = (void *)&stream_info;
  rc = pproc_port->event_func(pproc_port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error in streaming on\n", __func__, __LINE__);
    goto EXIT17;
  }

  /* Set output buffer to stream */
  memset(&event, 0, sizeof(mct_event_t));
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_PPROC_SET_OUTPUT_BUFF;
  event.u.module_event.module_event_data = (void *)&img_buf_output;
  rc = pproc_port->event_func(pproc_port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error setting output buffer to stream\n", __func__,
      __LINE__);
    goto EXIT18;
  }

  for (i = 0; i < test_case.frame_count; i++) {

    /* Trigger stream param buffer event */
    pthread_mutex_lock(&mutex);
    frame_pending = TRUE;
    memset(&parm_buf, 0, sizeof(cam_stream_parm_buffer_t));
    event.identity = identity;
    event.direction = MCT_EVENT_DOWNSTREAM;
    event.type = MCT_EVENT_CONTROL_CMD;
    event.u.ctrl_event.type = MCT_EVENT_CONTROL_PARM_STREAM_BUF;
    event.u.ctrl_event.control_event_data = (void *)&parm_buf;
    /* Set reprocess offline parameters */
    parm_buf.type = CAM_STREAM_PARAM_TYPE_DO_REPROCESS;
    parm_buf.reprocess.frame_idx = 0; /* Frame id */
    parm_buf.reprocess.buf_index = 0;
    if (metadata) {
      parm_buf.reprocess.meta_present = 1;
      parm_buf.reprocess.meta_buf_index = 2;
    }
    if (test_case.process_window_height && test_case.process_window_width &&
      metadata) {
      cam_crop_data_t crop_data;

      crop_data.num_of_streams = 1;
      crop_data.crop_info[0].stream_id = identity & 0xFFFF;
      crop_data.crop_info[0].crop.left =
         test_case.process_window_first_pixel;
      crop_data.crop_info[0].crop.top =
        test_case.process_window_first_line;
      crop_data.crop_info[0].crop.width =
        test_case.process_window_width;
      crop_data.crop_info[0].crop.height =
        test_case.process_window_height;
      add_metadata_entry(CAM_INTF_META_CROP_DATA,
        sizeof(cam_crop_data_t), &crop_data, metadata);
    }
    rc = pproc->process_event(pproc, &event);
    if (rc == FALSE) {
      CDBG_ERROR("%s:%d] error sending stream param buff event\n", __func__,
        __LINE__);
      frame_pending = FALSE;
      pthread_mutex_unlock(&mutex);
      goto EXIT18;
    }

    /* Wait for condition signal */
    while (frame_pending == TRUE) {
      pthread_cond_wait(&frame_done_cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);
    if (test_case.save_file) {
      /* Copy to output file */
      snprintf(out_fname,strlen(test_case.output_filename) + 7, "%s_%d.yuv",
        test_case.output_filename, i);
      out_file_fd = open(out_fname, O_RDWR | O_CREAT, 0777);
      if (out_file_fd < 0) {
        CDBG_ERROR("%s:%d] Cannot open file\n", __func__, __LINE__);
        goto EXIT18;
      }
      write(out_file_fd, (const void *)out_frame.addr[0],
        out_frame.ion_alloc[0].len);

      close(out_file_fd);
    }
  }

  CDBG_ERROR("FILE WRITTEN ####");
EXIT18:
  /* Stream off event */
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_CONTROL_CMD;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_STREAMOFF;
  event.u.ctrl_event.control_event_data = (void *)&stream_info;
  pproc_port->event_func(pproc_port, &event);
EXIT17:
  mct_list_free_all_on_data(MCT_OBJECT_CHILDREN(stream_info.stream),
    mct_stream_remove_stream_from_module, stream_info.stream);
EXIT16:
  mct_port_remove_child(identity, pproc_port);
EXIT15:
  /* Unlink on pproc port */
  pproc_port->un_link(identity, pproc_port, test_port);
EXIT14:
  /* Caps unreserve on pproc port */
  pproc_port->check_caps_unreserve(pproc_port, identity);
EXIT13:
  MCT_PIPELINE_CHILDREN(pipeline) =
    mct_list_remove(MCT_PIPELINE_CHILDREN(pipeline), stream_info.stream);
  (MCT_PIPELINE_NUM_CHILDREN(pipeline))--;
EXIT12:
  pproc_test_destroy_stream_info(&stream_info);
EXIT11:
  mct_pipeline_destroy(pipeline);
EXIT10:
  if (mct) {
    free(mct);
    mct = NULL;
  }
  mct_port_destroy(test_port);
EXIT9:
  /* Stop session on pproc */
  pproc->stop_session(pproc, 0x0003);
EXIT8:
  pproc_module_deinit(pproc);
EXIT7:
  if (metadata) {
    img_buf_list = mct_list_remove(img_buf_list, &metadata);
    free(metadata);
  }
  img_buf_list = mct_list_remove(img_buf_list, &img_buf_output);
EXIT6:
  do_munmap_ion(ionfd, &(out_frame.fd_data[0]), (void *)out_frame.addr[0],
    out_frame.ion_alloc[0].len);
EXIT5:
  img_buf_list = mct_list_remove(img_buf_list, &img_buf_input);
EXIT4:
  do_munmap_ion(ionfd, &(in_frame.fd_data[0]), (void *)in_frame.addr[0],
    in_frame.ion_alloc[0].len);
EXIT3:
  close(ionfd);
EXIT2:
  if (chromatixCpp_header) {
    dlclose(chromatixCpp_header);
  }
  close(in_file_fd);
EXIT1:
  pthread_mutex_destroy(&mutex);
  return main_ret;
}
