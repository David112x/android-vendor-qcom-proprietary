/*============================================================================

   Copyright (c) 2012, 2014, 2019 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#ifndef JPEGDMA_LIB_H
#define JPEGDMA_LIB_H
#include <linux/msm_ion.h>
#if TARGET_ION_ABI_VERSION >= 2
#include <ion/ion.h>
#endif
#include "jpegdma_lib_common.h"

#define EINVALID 1

typedef void *jpegdma_hw_obj_t;

struct jpegdma_hw_buf {
  uint32_t type;
  int fd;
  void *vaddr;
  uint32_t y_off;
  uint32_t y_len;
  uint32_t framedone_len;
  uint32_t cbcr_off;
  uint32_t cbcr_len;
  uint32_t cr_len;
  uint32_t cr_offset;
  uint32_t num_of_mcu_rows;
  uint32_t offset;
  int ion_fd_main;
  struct ion_allocation_data alloc_ion;
  struct ion_fd_data fd_ion_map;
};

struct jpegdma_hw_evt {
  uint32_t type;
  uint32_t len;
  void *value;
};

int jpegdma_lib_init(jpegdma_hw_obj_t * jpegdma_hw_obj, void *p_userdata,
  int (*event_handler) (void * p_userdata,
  struct jpegdma_hw_evt *,
  int event),
  int (*output_handler) (void*, struct jpegdma_hw_buf *),
  int (*input_handler) (void*, struct jpegdma_hw_buf *));

int jpegdma_lib_release(jpegdma_hw_obj_t jpegdma_hw_obj);


int jpegdma_lib_hw_config(jpegdma_hw_obj_t jpegdma_hw_obj,
  jpegdma_cmd_input_cfg * p_input_cfg,
  jpegdma_cmd_jpeg_transfer_cfg * p_transfer_cfg,
  jpegdma_cmd_scale_cfg * p_scale_cfg);
int jpegdma_lib_input_buf_enq(jpegdma_hw_obj_t jpegdma_hw_obj,
  struct jpegdma_hw_buf *);
int jpegdma_lib_output_buf_enq(jpegdma_hw_obj_t jpegdma_hw_obj,
  struct jpegdma_hw_buf *);
int jpegdma_lib_transfer(jpegdma_hw_obj_t jpegdma_hw_obj);

int jpegdma_lib_get_event(jpegdma_hw_obj_t jpegdma_hw_obj,
    struct jpegdma_hw_evt *);
int jpegdma_lib_get_input(jpegdma_hw_obj_t jpegdma_hw_obj,
    struct jpegdma_hw_buf *);
int jpegdma_lib_get_output(jpegdma_hw_obj_t jpegdma_hw_obj,
    struct jpegdma_hw_buf *);

int jpegdma_lib_wait_done(jpegdma_hw_obj_t jpegdma_hw_obj);
int jpegdma_lib_stop(jpegdma_hw_obj_t jpegdma_hw_obj);

#endif /* JPEGDMA_LIB_H */
