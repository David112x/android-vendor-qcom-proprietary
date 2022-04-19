/*============================================================================
Copyright (c) 2013,2015-2017 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================*/
#ifndef __IFACE_BUF_MGR_H__

#define __IFACE_BUF_MGR_H__
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
#include "iface_def.h"
#if TARGET_ION_ABI_VERSION >=2
#include <ion/ion.h>
#endif

#define IFACE_MAX_NUM_BUF_QUEUE 28

typedef struct {
  uint32_t buf_handle;
  uint32_t session_id;
  uint32_t stream_id;
  int vfe_fd;
} isp_buf_register_t;

typedef struct {
  uint32_t buf_handle;
  uint32_t session_id;
  uint32_t stream_id;
  uint32_t use_native_buf;
  int current_num_buf;
  int total_num_buf;
  cam_frame_len_offset_t buf_info;
  enum msm_isp_buf_type buf_type;
  int cached;
  mct_list_t *img_buf_list;
  uint8_t adsp_heap;
  enum smmu_attach_mode security_mode;
} iface_buf_request_t;

typedef struct {
  uint32_t user_bufq_handle;
  uint32_t kernel_bufq_handle;
  uint32_t session_id;
  uint32_t stream_id;
  int current_num_buffer;
  int total_num_buffer; /*For def buf allocation, we need to know total count*/
  int open_cnt;
  uint32_t use_native_buf;
  enum msm_isp_buf_type buf_type;
  iface_frame_buffer_t image_bufs[IFACE_MAX_IMG_BUF];
  uint32_t used;
  int vfe_fds[2];
  int num_vfe_fds;
  pthread_mutex_t mutex;
  enum smmu_attach_mode security_mode;
} iface_bufq_t;

typedef struct {
  pthread_mutex_t mutex;
  pthread_mutex_t req_mutex;
  uint32_t use_cnt;
  int ion_fd;
  uint32_t bufq_handle_count;
  iface_bufq_t bufq[IFACE_MAX_NUM_BUF_QUEUE];
} iface_buf_mgr_t;

static int iface_do_munmap_ion (int ion_fd, struct ion_fd_data *ion_info_fd,
  void *addr, size_t size);
int iface_init_native_buffer(iface_frame_buffer_t *buf, int buf_idx,
  int ion_fd, cam_frame_len_offset_t *len_offset, int cached,
  uint8_t adsp_heap, enum smmu_attach_mode security_mode);
int iface_do_cache_inv(uint32_t bufq_handle,
  iface_buf_mgr_t* buf_mgr, int ion_fd, uint32_t buf_idx,
  uint32_t buffer_access);
int iface_do_cache_inv_ion(int ion_fd, iface_frame_buffer_t *image_buf,
  uint32_t buffer_access);
void iface_deinit_native_buffer(iface_frame_buffer_t *buf, int ion_fd);
static int iface_generate_new_bufq_handle(iface_buf_mgr_t *buf_mgr, int index);
static uint32_t iface_get_bufq_handle(iface_buf_mgr_t *buf_mgr);
static void iface_free_bufq_handle(iface_buf_mgr_t *buf_mgr, iface_bufq_t *bufq);
static boolean iface_init_hal_buffer(void *data, void *user_data);
static int iface_validate_buf_request(iface_buf_mgr_t *buf_mgr,
    iface_buf_request_t *buf_request, int bufq_handle);
static int iface_request_kernel_bufq(iface_buf_mgr_t *buf_mgr,
  iface_bufq_t *bufq, int vfe_fd);
static void iface_release_kernel_bufq(iface_buf_mgr_t *buf_mgr,
  iface_bufq_t *bufq, int vfe_fd);
static int iface_queue_buf_list_update(iface_buf_mgr_t *buf_mgr, iface_bufq_t *bufq,
  int vfe_fd);
static int iface_queue_buf_all(iface_buf_mgr_t *buf_mgr,
  iface_bufq_t *bufq, int vfe_fd);
static int iface_queue_buf_int(iface_buf_mgr_t *buf_mgr,
  uint32_t bufq_handle, int buf_idx, uint32_t dirty_buf, int vfe_fd,
  uint32_t buffer_access);
int iface_queue_buf(iface_buf_mgr_t *buf_mgr,
  uint32_t bufq_handle, int buf_idx, uint32_t dirty_buf, int vfe_fd,
  uint32_t buffer_access);
int iface_unmap_buf(iface_buf_mgr_t *buf_mgr, int vfe_fd, uint32_t buf_fd);
void *iface_get_buf_vaddr(iface_buf_mgr_t *buf_mgr,
  uint32_t bufq_handle, int buf_idx);
iface_frame_buffer_t *iface_get_buf_by_idx(iface_buf_mgr_t *buf_mgr,
  uint32_t bufq_handle, int buf_idx);
uint32_t iface_find_matched_bufq_handle(iface_buf_mgr_t *buf_mgr,
  uint32_t session_id, uint32_t stream_id);
uint32_t iface_find_matched_bufq_handle_by_kernel_handle(iface_buf_mgr_t *buf_mgr,
  uint32_t kernel_bufq_handle);
static int iface_save_vfe_fd(iface_bufq_t *bufq, int vfe_fd);
static int iface_remove_vfe_fd(iface_bufq_t *bufq, int vfe_fd);
int iface_register_buf_list_update(iface_buf_mgr_t *buf_mgr,
  uint32_t bufq_handle, iface_buf_request_t *buf_request, int vfe_fd);
int iface_register_buf(iface_buf_mgr_t *buf_mgr,
  uint32_t bufq_handle, int vfe_fd);
int iface_unregister_buf_list(iface_buf_mgr_t *buf_mgr,
  uint32_t bufq_handle, iface_buf_request_t *buf_request, int vfe_fd);
int iface_request_buf(iface_buf_mgr_t *buf_mgr, iface_buf_request_t *buf_request);
int iface_unregister_buf(iface_buf_mgr_t *buf_mgr,
  uint32_t bufq_handle, int vfe_fd);
void iface_release_buf(iface_buf_mgr_t *buf_mgr, uint32_t bufq_handle,
  mct_list_t *img_buffer_list);
int iface_open_buf_mgr(iface_buf_mgr_t *buf_mgr);
void iface_close_buf_mgr(iface_buf_mgr_t *buf_mgr);
int iface_init_buf_mgr(iface_buf_mgr_t *buf_mgr);
void iface_deinit_buf_mgr(iface_buf_mgr_t *buf_mgr);
void iface_close_ion(int ion_fd);
int iface_open_ion(void);
boolean iface_free_buf(void *data, void *user_data __unused);
int iface_alloc_native_buf_handles(iface_buf_mgr_t *buf_mgr,
      cam_frame_len_offset_t *buf_info, uint32_t num_bufs, uint8_t adsp_heap,
      iface_frame_buffer_t p_image_buff_list[IFACE_MAX_IMG_BUF], enum smmu_attach_mode security_mode);
int iface_update_native_buf_handles(iface_buf_mgr_t *buf_mgr,
      iface_buf_request_t *buf_request,iface_frame_buffer_t p_image_buff_list[IFACE_MAX_IMG_BUF]);
int iface_release_native_buf_handles(iface_buf_mgr_t *buf_mgr,
     uint32_t num_bufs, iface_frame_buffer_t image_buff_list[IFACE_MAX_IMG_BUF]);
int iface_get_current_num_buffer(iface_buf_mgr_t *buf_mgr, uint32_t bufq_handle);

#endif /* __IFACE_BUF_MGR_H__ */
