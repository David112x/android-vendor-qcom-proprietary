/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#ifndef VPU_TEST_H
#define VPU_TEST_H

#include <sys/mman.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/ion.h>
#include <linux/videodev2.h>
#include <linux/msm_ion.h>
#include <media/msmb_camera.h>
#include <media/msmb_pproc.h>
#include "mct_pipeline.h"
#include "mct_module.h"
#include "mct_stream.h"
#include "modules.h"
#include "pproc_port.h"
#include "vpu_log.h"

#define VPU_TEST_NUM_BUF_MAX 8

typedef struct v4l2_frame_buf {
  struct v4l2_buffer buffer;
  unsigned long addr[VIDEO_MAX_PLANES];
  uint32_t size;
  struct ion_allocation_data ion_alloc[VIDEO_MAX_PLANES];
  struct ion_fd_data fd_data[VIDEO_MAX_PLANES];
} v4l2_frame_buf_t;

typedef struct _test_case_t {
  char input_path[256];
  int32_t width;
  int32_t height;
  v4l2_frame_buf_t buf[VPU_TEST_NUM_BUF_MAX];
  int32_t buf_count;
} test_case_t;


#define STD_LOG(fmt, args...) \
  printf(fmt"\n", ##args); \
  VPU_HIGH(fmt, ##args)

#define STD_ERR(fmt, args...) \
  printf("Error: "fmt"\n", ##args); \
  VPU_ERR(fmt, ##args)

#endif
