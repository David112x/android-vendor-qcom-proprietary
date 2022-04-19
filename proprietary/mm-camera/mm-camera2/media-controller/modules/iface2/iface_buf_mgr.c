/*============================================================================
Copyright (c) 2013, 2015-2016, 2018 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================*/
#include "iface_buf_mgr.h"
#include "mct_stream.h"
#include "iface.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#ifdef TARGET_ION_ABI_VERSION
#include <linux/dma-buf.h>
#endif

#define IFACE_BUFQ_HANDLE_SHIFT 16
#define IFACE_BUFQ_INDEX(handle) ((handle) & 0xFFFF)
#define IFACE_GET_BUFQ(mgr, handle)  \
  ( (IFACE_BUFQ_INDEX(handle) < IFACE_MAX_NUM_BUF_QUEUE) ? \
  (&mgr->bufq[IFACE_BUFQ_INDEX(handle)]) : NULL )

#define IFACE_DEINIT_BUF(buf_ptr, ion_fd) { \
    if ((int)((buf_ptr)->buffer.length) > 0) { \
      iface_deinit_native_buffer((buf_ptr), (ion_fd)); \
    } \
  }

typedef struct {
  iface_buf_mgr_t *buf_mgr;
  iface_frame_buffer_t *iface_map_bufs;
  int cnt;
  int vfe_fd;
  uint32_t bufq_handle;
} find_map_buf_t;

/** iface_do_mmap_ion
 *
 * DESCRIPTION:
 *
 **/
static boolean iface_do_mmap_ion(int ion_fd, struct ion_allocation_data *alloc,
  struct ion_fd_data *ion_info_fd, int *mapFd, void **map_addr)
{
  void *ret; /* returned virtual address */
  int rc = 0;
  unsigned int flags = 0;
  int map_fd = -1;
  struct ion_handle_data handle_data;

  *map_addr = NULL;
  /* to make it page size aligned */
  alloc->len = (alloc->len + 4095) & (~4095);
#ifndef TARGET_ION_ABI_VERSION
  rc = ioctl(ion_fd, ION_IOC_ALLOC, alloc);
#else
   rc = ion_alloc_fd(ion_fd, alloc->len, alloc->align, alloc->heap_id_mask,
                      alloc->flags, &map_fd);
  *mapFd = map_fd;
#endif//TARGET_ION_ABI_VERSION endif
  if (rc < 0) {
    CDBG_ERROR("%s: ION allocation failed %s\n", __func__, strerror(errno));
    goto ION_ALLOC_FAILED;
  }
#ifndef TARGET_ION_ABI_VERSION
  ion_info_fd->handle = alloc->handle;
  rc = ioctl(ion_fd, ION_IOC_SHARE, ion_info_fd);
  if (rc < 0) {
    CDBG_ERROR("ION map failed %s\n", strerror(errno));
    goto ION_MAP_FAILED;
  }
  *mapFd = ion_info_fd->fd;

#else
  ion_info_fd->fd = map_fd;
#endif//TARGET_ION_ABI_VERSION endif
  if (alloc->heap_id_mask == ION_HEAP(ION_SECURE_DISPLAY_HEAP_ID))
    return TRUE;

  ret = mmap(NULL,
    alloc->len,
    PROT_READ  | PROT_WRITE,
    MAP_SHARED,
    *mapFd,
    0);

  if (ret == MAP_FAILED) {
    CDBG_ERROR("ION_MMAP_FAILED: %s (%d)\n", strerror(errno), errno);
    goto ION_MAP_FAILED;
  }

  *map_addr = ret;
  return TRUE;

ION_MAP_FAILED:
#ifndef TARGET_ION_ABI_VERSION
  handle_data.handle = ion_info_fd->handle;
  ioctl(ion_fd, ION_IOC_FREE, &handle_data);
#endif
ION_ALLOC_FAILED:
  return FALSE;
}

/** iface_do_munmap_ion
 *
 * DESCRIPTION:
 *
 **/
static int iface_do_munmap_ion (int ion_fd, struct ion_fd_data *ion_info_fd,
  void *addr, size_t size)
{
  int rc = 0;
  rc = munmap(addr, size);
#ifndef TARGET_ION_ABI_VERSION
  close(ion_info_fd->fd);

  struct ion_handle_data handle_data;
  handle_data.handle = ion_info_fd->handle;
  ioctl(ion_fd, ION_IOC_FREE, &handle_data);
#else
  close(ion_info_fd->fd);
  (void) ion_fd;
#endif
  return rc;
}

/** iface_init_native_buffer
 *
 *  @adsp_heap: Flag to indicate whether adsp heap needs to be
 *               used
 * DESCRIPTION:
 *
 **/
int iface_init_native_buffer(iface_frame_buffer_t *buf, int buf_idx,
  int ion_fd, cam_frame_len_offset_t *len_offset, int cached,
  uint8_t adsp_heap, enum smmu_attach_mode security_mode)
{
  int current_fd = -1;
  uint32_t i;
  unsigned long current_addr = 0;
  uint32_t accumu_addr = 0;

  memset(buf, 0, sizeof(iface_frame_buffer_t));
  buf->buffer.m.planes = &buf->planes[0];

  /* now we only use contigous memory.
   * We could use per plane memory */
  buf->ion_alloc[0].len = len_offset->frame_len;

  CDBG("%s:%d Allocate buffer with len 0x%x\n",
       __func__, __LINE__, buf->ion_alloc[0].len);

  /*page table 4K allign*/
  CDBG("%s:%d] Heap type adsp %d security mode %d",
        __func__, __LINE__, adsp_heap, security_mode);
  if (security_mode == SECURE_MODE) {
    buf->ion_alloc[0].heap_id_mask = ION_HEAP(ION_SECURE_DISPLAY_HEAP_ID);
    buf->ion_alloc[0].flags = (ION_FLAG_SECURE | ION_FLAG_CP_CAMERA);
    /* 2MB align */
    buf->ion_alloc[0].align = 0x200000;
#ifdef SECURE_SLAVE_MODE
  } else if (security_mode == SECURE_SLAVE_MODE) {
    buf->ion_alloc[0].heap_id_mask = ION_HEAP(ION_CP_MM_HEAP_ID);
    buf->ion_alloc[0].flags = (ION_FLAG_SECURE | ION_FLAG_CP_CAMERA);
    /* 2MB align */
    buf->ion_alloc[0].align = 0x200000;
#endif
  } else {
    buf->ion_alloc[0].heap_id_mask = adsp_heap ?
      ION_HEAP(ION_ADSP_HEAP_ID) : ION_HEAP(ION_SYSTEM_HEAP_ID);
    buf->ion_alloc[0].flags = ION_FLAG_CACHED;
    buf->ion_alloc[0].align = 4096;
  }
  if (iface_do_mmap_ion(ion_fd,
    &(buf->ion_alloc[0]), &(buf->fd_data[0]), &current_fd,
    &buf->vaddr) == FALSE) {
    CDBG_ERROR("%s: ION allocation no mem\n", __func__);
    return -1;
  }
  current_addr = (unsigned long)buf->vaddr;

  for (i = 0; i < len_offset->num_planes; i++) {
    buf->buffer.m.planes[i].m.userptr = current_fd;
    buf->buffer.m.planes[i].data_offset = len_offset->mp[i].offset;
    buf->buffer.m.planes[i].length = len_offset->mp[i].len;
    if (security_mode == SECURE_MODE)
      buf->addr[i] = 0;
    else
      buf->addr[i] = current_addr + accumu_addr;
    accumu_addr += buf->buffer.m.planes[i].length;
  }

  buf->buffer.length = len_offset->num_planes;
  buf->buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  buf->buffer.index = buf_idx;
  buf->buffer.memory = V4L2_MEMORY_USERPTR;
  buf->fd = current_fd;
  if (security_mode == SECURE_MODE
#ifdef SECURE_SLAVE_MODE
  || security_mode == SECURE_SLAVE_MODE
#endif
    )
    buf->cached = 0;
  else
    buf->cached = cached;
  return 0;
}

int iface_do_cache_inv(uint32_t bufq_handle,
  iface_buf_mgr_t* buf_mgr, int ion_fd, uint32_t buf_idx,
  uint32_t buffer_access)
{

  iface_bufq_t *bufq = IFACE_GET_BUFQ(buf_mgr, bufq_handle);

  return iface_do_cache_inv_ion(ion_fd,
    &bufq->image_bufs[buf_idx],
    buffer_access & CPU_HAS_WRITTEN);
}

/** iface_do_cache_inv_ion
 *
 * DESCRIPTION:
 *
 **/
int iface_do_cache_inv_ion(int ion_fd, iface_frame_buffer_t *image_buf,
  uint32_t buffer_access)
{
  int ret = 0;
 #ifndef TARGET_ION_ABI_VERSION
  struct ion_flush_data cache_inv_data;
  struct ion_custom_data custom_data;

  if (image_buf->vaddr == NULL) {
    CDBG_ERROR("Invalid vaddr for cache\n");
    return -1;
  }

  memset(&cache_inv_data, 0, sizeof(cache_inv_data));
  memset(&custom_data, 0, sizeof(custom_data));
  cache_inv_data.vaddr = image_buf->vaddr;
  cache_inv_data.fd = image_buf->fd;
  cache_inv_data.handle = image_buf->ion_alloc[0].handle;
  cache_inv_data.length = image_buf->ion_alloc[0].len;
  if (buffer_access == CPU_HAS_READ) {
    custom_data.cmd = ION_IOC_INV_CACHES;
  } else if (buffer_access == CPU_HAS_WRITTEN) {
    custom_data.cmd = ION_IOC_CLEAN_CACHES;
  } else if (buffer_access ==
    (CPU_HAS_WRITTEN | CPU_HAS_READ)) {
    custom_data.cmd = ION_IOC_CLEAN_INV_CACHES;
  } else {
    return ret;
  }

  custom_data.arg = (unsigned long)&cache_inv_data;

  ret = ioctl(ion_fd, ION_IOC_CUSTOM, &custom_data);
#else
(void) ion_fd;
  struct dma_buf_sync buf_sync_start;
  struct dma_buf_sync buf_sync_end;
 /*  ION_IOC_INV_CACHES-->call the DMA_BUF_IOCTL_SYNC IOCT with flags DMA_BUF_SYNC_START and
     DMA_BUF_SYNC_WRITE and then call the DMA_BUF_IOCTL_SYNC IOCT with flags DMA_BUF_SYNC_END
     and DMA_BUF_SYNC_READ
     ION_IOC_CLEAN_CACHES-->call the DMA_BUF_IOCTL_SYNC IOCTL with flags DMA_BUF_SYNC_START
     and DMA_BUF_SYNC_WRITE and then call the DMA_BUF_IOCTL_SYNC IOCTL with flags DMA_BUF_SYNC_END
     and DMA_BUF_SYNC_WRITE
     ION_IOC_CLEAN_INV_CACHES-->call the DMA_BUF_IOCTL_SYNC IOCTL with flags DMA_BUF_SYNC_START
     and DMA_BUF_SYNC_RW and then call the DMA_BUF_IOCTL_SYNC IOCTL with flags DMA_BUF_SYNC_END
     and DMA_BUF_SYNC_RW
  */

  if (buffer_access == CPU_HAS_READ) {
    buf_sync_start.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_WRITE;
    buf_sync_end.flags   = DMA_BUF_SYNC_END   | DMA_BUF_SYNC_READ;
  }
   else if (buffer_access == CPU_HAS_WRITTEN) {
    buf_sync_start.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_WRITE;
    buf_sync_end.flags   = DMA_BUF_SYNC_END   | DMA_BUF_SYNC_WRITE;
  } else if (buffer_access ==
    (CPU_HAS_WRITTEN | CPU_HAS_READ)) {
    buf_sync_start.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_RW;
    buf_sync_end.flags   = DMA_BUF_SYNC_END   | DMA_BUF_SYNC_RW;
  } else {
    return ret;
  }
    ret = ioctl(image_buf->fd, DMA_BUF_IOCTL_SYNC, &buf_sync_start.flags);
 if (ret < 0){
       CDBG_ERROR("Failed DMA_BUF_IOCTL_SYNC start: %s\n", strerror(errno));
}
    ret = ioctl(image_buf->fd, DMA_BUF_IOCTL_SYNC, &buf_sync_end.flags);
 if (ret < 0){
       CDBG_ERROR("Failed DMA_BUF_IOCTL_SYNC end: %s\n", strerror(errno));
}
#endif//TARGET_ION_ABI_VERSION
  return ret;
}

/** iface_deinit_native_buffer
 *
 * DESCRIPTION:
 *
 **/
void iface_deinit_native_buffer(iface_frame_buffer_t *buf, int ion_fd)
{
  int i;

  if (buf->fd <= 0)
    return;
  for (i = 0; i < (int)buf->buffer.length; i++) {
    if (buf->ion_alloc[i].len != 0) {
      iface_do_munmap_ion(ion_fd, &(buf->fd_data[0]),
        (void *)buf->addr[i], buf->ion_alloc[i].len);
    }
  }
  memset(buf, 0, sizeof(iface_frame_buffer_t));
}

/** iface_generate_new_bufq_handle
 *
 * DESCRIPTION:
 *
 **/
static int iface_generate_new_bufq_handle(iface_buf_mgr_t *buf_mgr, int index)
{
  if ((buf_mgr->bufq_handle_count << IFACE_BUFQ_HANDLE_SHIFT) == 0)
    buf_mgr->bufq_handle_count = 1;
  return buf_mgr->bufq_handle_count++ << IFACE_BUFQ_HANDLE_SHIFT | index;
}

/** iface_get_bufq_handle
 *
 * DESCRIPTION:
 *
 **/
static uint32_t iface_get_bufq_handle(iface_buf_mgr_t *buf_mgr)
{
  int i;

  for (i = 0; i < IFACE_MAX_NUM_BUF_QUEUE; i++) {
    if (!buf_mgr->bufq[i].used)
      break;
  }

  if (i == IFACE_MAX_NUM_BUF_QUEUE) {
    CDBG_ERROR("%s: No free buf queue\n", __func__);
    return 0;
  }

  buf_mgr->bufq[i].used = 1;
  buf_mgr->bufq[i].user_bufq_handle = iface_generate_new_bufq_handle(buf_mgr, i);
  return buf_mgr->bufq[i].user_bufq_handle;
}

/** iface_free_bufq_handle
 *
 * DESCRIPTION:
 *
 **/
static void iface_free_bufq_handle(
  iface_buf_mgr_t *buf_mgr __unused,
  iface_bufq_t    *bufq)
{
  memset(bufq, 0, sizeof(iface_bufq_t));
}

/** iface_init_hal_buffer
 *
 * DESCRIPTION:
 *
 **/
static boolean iface_init_hal_buffer(void *data, void *user_data)
{
  uint32_t i, idx;
  find_map_buf_t *map_buf = (find_map_buf_t *)user_data;
  mct_stream_map_buf_t *img_buf = (mct_stream_map_buf_t *)data;
  struct v4l2_buffer *v4l2_buf = NULL;

  for (i = 0; i < IFACE_MAX_IMG_BUF; i++) {
    if (!map_buf->iface_map_bufs[i].buffer.m.planes) {
      idx = i;
      break;
    }
  }
  if (i == IFACE_MAX_IMG_BUF) {
    CDBG_ERROR("%s: Cannot add more than %d buffers\n", __func__, i);
    return FALSE;
  }

  /*fill in v4l2 buffer info*/
  v4l2_buf = &map_buf->iface_map_bufs[idx].buffer;
  v4l2_buf->m.planes = map_buf->iface_map_bufs[idx].planes;
  for (i = 0; i < img_buf->num_planes; i++) {
    v4l2_buf->m.planes[i].m.userptr = img_buf->buf_planes[i].fd;
    v4l2_buf->m.planes[i].data_offset = img_buf->buf_planes[i].offset;
    v4l2_buf->m.planes[i].length = img_buf->buf_planes[i].size;
    map_buf->iface_map_bufs[idx].addr[i] =
      (unsigned long)img_buf->buf_planes[i].buf;
  }

  v4l2_buf->length = img_buf->num_planes;
  v4l2_buf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  v4l2_buf->index = img_buf->buf_index;
  v4l2_buf->memory = V4L2_MEMORY_USERPTR;

  /*update mapped buf*/
  map_buf->iface_map_bufs[idx].is_reg = FALSE;
  map_buf->cnt++;

  return TRUE;
}

/** iface_dequeue_buf_int
 *
 * DESCRIPTION:
 *
 **/
int iface_dequeue_buf_int(
  iface_buf_mgr_t *buf_mgr,
  uint32_t         bufq_handle,
  int              buf_idx,
  int              vfe_fd)
{
  int rc;
  struct msm_isp_qbuf_info qbuf_info;
  iface_bufq_t *bufq = IFACE_GET_BUFQ(buf_mgr, bufq_handle);

  if (!bufq) {
    CDBG_ERROR("%s: cannot find bufq with handle 0x%x\n",
      __func__, bufq_handle);
    return -1;
  }

  memset(&qbuf_info, 0, sizeof(qbuf_info));
  qbuf_info.handle = bufq->kernel_bufq_handle;
  qbuf_info.buf_idx = bufq->image_bufs[buf_idx].buffer.index;

  rc = ioctl(vfe_fd, VIDIOC_MSM_ISP_DEQUEUE_BUF, &qbuf_info);
  if (rc < 0) {
    CDBG_ERROR("%s: queue buf to kernel failed, rc = -1\n", __func__);
    return -1;
  }
  bufq->image_bufs[buf_idx].is_reg = FALSE;

  return 0;
}

/** iface_deinit_hal_buffer
 *
 * DESCRIPTION:
 *
 **/
static boolean iface_deinit_hal_buffer(void *data, void *user_data)
{
  find_map_buf_t *map_buf = (find_map_buf_t *)user_data;
  mct_stream_map_buf_t *img_buf = (mct_stream_map_buf_t *)data;
  struct v4l2_buffer *v4l2_buf = NULL;
  uint32_t i;
  int32_t rc;

  /* Find curr buffer in mapped buffers */
  for (i = 0; i < IFACE_MAX_IMG_BUF; i++) {
    v4l2_buf = &map_buf->iface_map_bufs[i].buffer;
    if (map_buf->iface_map_bufs[i].buffer.m.planes &&
        img_buf->buf_index == v4l2_buf->index) {
      /* Dequeue buffer from kernel queue */
      if (map_buf->iface_map_bufs[i].is_reg) {
        rc = iface_dequeue_buf_int(map_buf->buf_mgr, map_buf->bufq_handle, i,
          map_buf->vfe_fd);
        if (rc < 0) {
          CDBG_ERROR("%s: iface_dequeue_buf_int failed\n", __func__);
          return FALSE;
        }

        /* Remove from map buffer internal list */
        map_buf->iface_map_bufs[i].buffer.m.planes = 0;
        map_buf->cnt--;
      } else {
        CDBG_ERROR("%s: buffer %d is not register\n", __func__,
          img_buf->buf_index);
        return FALSE;
      }
      break;
    }
  }

  return TRUE;
}

/** iface_validate_buf_request
 *
 * DESCRIPTION:
 *
 **/
static int iface_validate_buf_request(iface_buf_mgr_t *buf_mgr,
    iface_buf_request_t *buf_request, int bufq_handle)
{
  int i;

  iface_bufq_t *bufq = IFACE_GET_BUFQ(buf_mgr, bufq_handle);
  if (!bufq) {
    CDBG_ERROR("%s: cannot find bufq with handle 0x%x\n",
      __func__, bufq_handle);
    return -1;
  }

  /*Initially num_buffer would be zero, but when HAL updates
  buf_list dynamically after streamon, if MCT sends only
  delta of buf list need to pupolate ISP image_buf array
  only for additional buffers.
  handle the case, in which native bufs are allocated by downstream comp
  and sent to iface for offline processing
 */
  if ((!buf_request->use_native_buf) ||
      (buf_request->use_native_buf && buf_request->img_buf_list)) {
    find_map_buf_t map_buf;
    map_buf.cnt = bufq->current_num_buffer;
    map_buf.iface_map_bufs = bufq->image_bufs;

    /*map the buf from buf_request to iface map buf*/
    mct_list_traverse(buf_request->img_buf_list,
      iface_init_hal_buffer, (void *)&map_buf);
    /*update buf request numbber after map buf*/
    buf_request->current_num_buf = map_buf.cnt;
  }

  if (buf_request->current_num_buf < 0 ||
      buf_request->current_num_buf > IFACE_MAX_IMG_BUF) {
    CDBG_ERROR("%s: Invalid number of buffer,current num_buf = %d\n",
      __func__, buf_request->current_num_buf);
    return -1;
  }
  buf_request->buf_handle = bufq_handle;

  /*update bufq info*/
  CDBG("%s:hw stream %x: current buf cnt: %d, updated %d\n", __func__,
    buf_request->stream_id, bufq->current_num_buffer,
    buf_request->current_num_buf);
  bufq->session_id = buf_request->session_id;
  bufq->stream_id = buf_request->stream_id;
  bufq->buf_type = buf_request->buf_type;
  bufq->use_native_buf = buf_request->use_native_buf;
  bufq->current_num_buffer = buf_request->current_num_buf;
  bufq->total_num_buffer = buf_request->total_num_buf;
  bufq->security_mode = buf_request->security_mode;

  /*If HAL has not passed num_bufs information, use the
  count of buffers passed in im_buf_list as total bufs*/
  if (bufq->total_num_buffer == 0)
    bufq->total_num_buffer = bufq->current_num_buffer;

  if (bufq->total_num_buffer < bufq->current_num_buffer) {
    CDBG_ERROR("%s: hw stream %x curr buf_cnt %d > total %d request from HAL\n",
      __func__, bufq->stream_id, bufq->current_num_buffer,
      bufq->total_num_buffer);
    return -1;
  }

  CDBG("%s: hw stream %x: buf count total %d curr %d\n",
    __func__, bufq->stream_id, bufq->total_num_buffer,
    bufq->current_num_buffer);

  /* If totalnumber of buffer are less than or equal to zero,
  with stream buffer configuration*/
  assert(bufq->total_num_buffer > 0);
  return 0;
}

/** iface_request_kernel_bufq
 *
 * DESCRIPTION:
 *
 **/
static int iface_request_kernel_bufq(
  iface_buf_mgr_t *buf_mgr __unused,
  iface_bufq_t    *bufq,
  int              vfe_fd)
{
  int rc = 0;
#ifdef VIDIOC_MSM_ISP_REQUEST_BUF_VER2
  struct msm_isp_buf_request_ver2 bufq_request_ver2;
  bufq_request_ver2.session_id = bufq->session_id;
  bufq_request_ver2.stream_id = bufq->stream_id;
  bufq_request_ver2.num_buf = bufq->total_num_buffer;
  bufq_request_ver2.buf_type = bufq->buf_type;
  bufq_request_ver2.security_mode = bufq->security_mode;
  rc = ioctl(vfe_fd, VIDIOC_MSM_ISP_REQUEST_BUF_VER2, &bufq_request_ver2);
  if (!rc)
    bufq->kernel_bufq_handle = bufq_request_ver2.handle;
#else
  struct msm_isp_buf_request bufq_request;
  bufq_request.session_id = bufq->session_id;
  bufq_request.stream_id = bufq->stream_id;
  bufq_request.num_buf = bufq->total_num_buffer;
  bufq_request.buf_type = bufq->buf_type;
  if (bufq->security_mode != SECURE_MODE) {
    rc = ioctl(vfe_fd, VIDIOC_MSM_ISP_REQUEST_BUF, &bufq_request);
  }
  else {
    /* this option is not supported w/o kernel change */
    CDBG_ERROR("%s: invalid secure mode set in bufq, rc = -1\n", __func__);
    rc = -1;
  }
  if (!rc)
    bufq->kernel_bufq_handle = bufq_request.handle;
#endif

  if (rc < 0) {
    CDBG_ERROR("%s: kernel request buf failed, rc = -1\n", __func__);
    return -1;
  }


  return 0;
}

/** iface_release_kernel_bufq
 *
 * DESCRIPTION:
 *
 **/
static void iface_release_kernel_bufq(
  iface_buf_mgr_t *buf_mgr __unused,
  iface_bufq_t    *bufq,
  int              vfe_fd)
{
  struct msm_isp_buf_request bufq_release;
  bufq_release.handle = bufq->kernel_bufq_handle;
  ioctl(vfe_fd, VIDIOC_MSM_ISP_RELEASE_BUF, &bufq_release);

  return;
}


/** iface_queue_buf_list_update
 *    @buf_mgr: buf manager
 *    @bufq: buffer queue
 *    @vfe_fd : hw id
 *
 *  HAL has updated buffer list. Now we have to update and
 *  register new buffers to kernel.
 *
 *  This function is called in MCT thread context
 *
 *  Returns 0 for success and negative error for failure
 **/
static int iface_queue_buf_list_update(iface_buf_mgr_t *buf_mgr, iface_bufq_t *bufq,
  int vfe_fd)
{
  int rc = 0, i;
  uint32_t dirty_buf = 0;

  CDBG("%s: E\n", __func__);
  /*if vfe_fd is zero update buffer list to any of available hw*/
  if (vfe_fd <= 0) {
    if (bufq->vfe_fds[0] > 0)
      vfe_fd = bufq->vfe_fds[0];
    else if (bufq->vfe_fds[1] > 0)
      vfe_fd= bufq->vfe_fds[1];
    else {
      CDBG_ERROR("%s: error, do not have VFE fd\n", __func__);
      rc = -1;
      goto end;
    }
  }

  /*Re-iterate through entire image_buf array and enque the newly added
    buffers to kernel*/
  for (i = 0; i < IFACE_MAX_IMG_BUF; i++) {
    CDBG("%s: buffer %x buf_idx %d is reg = %d\n", __func__,
      bufq->image_bufs[i].buffer, i, bufq->image_bufs[i].is_reg);
    if (!bufq->image_bufs[i].is_reg && bufq->image_bufs[i].buffer.m.planes) {
      rc = iface_queue_buf_int(buf_mgr, bufq->user_bufq_handle, i, dirty_buf,
        vfe_fd, 0);
      if (rc < 0) {
        CDBG_ERROR("%s: buffer enque to kernel failed rc = %d\n", __func__, rc);
        rc = -1;
        goto end;
      }
      bufq->image_bufs[i].is_reg = TRUE;
    }
  }
end:
  CDBG("%s: X, rc = %d\n", __func__, rc);
  return rc;
}

/** iface_queue_buf_all
 *    @buf_mgr: buf manager
 *    @bufq: buffer queue
 *    @vfe_fd : hw id
 *
 *  Queue all buffers to kernel
 *
 * Returns 0 for success and negative error for failure
 *
 **/
static int iface_queue_buf_all(iface_buf_mgr_t *buf_mgr,
  iface_bufq_t *bufq, int vfe_fd)
{
  int rc, i;
  uint32_t dirty_buf = 0;

  for (i = 0; i < bufq->current_num_buffer; i++) {
    rc = iface_queue_buf_int(buf_mgr, bufq->user_bufq_handle, i, dirty_buf,
      vfe_fd, 0);
    if (rc < 0) {
      CDBG_ERROR("%s: iface_queue_buf_int failed, rc = %d\n", __func__, rc);
      return rc;
    }
  }

  return 0;
}

/** isp_copy_planes_from_v4l2_buffer
 *    @qbuf_buf: target isp qbuf buffer
 *    @v4l2_buf: source v4l2 buffer
 *
 * Copies planes info from V4L2 buffer to isp queue buffer
 *
 * Return: None
 *
 **/
static void iface_copy_planes_from_v4l2_buffer(
  struct msm_isp_qbuf_buffer *qbuf_buf, const struct v4l2_buffer *v4l2_buf)
{
  unsigned int i;

  qbuf_buf->num_planes = v4l2_buf->length;
  for (i = 0; i < qbuf_buf->num_planes; i++) {
    qbuf_buf->planes[i].addr = v4l2_buf->m.planes[i].m.userptr;
    qbuf_buf->planes[i].offset = v4l2_buf->m.planes[i].data_offset;
    qbuf_buf->planes[i].length = v4l2_buf->m.planes[i].length;
  }
}

/** iface_queue_buf_int
 *
 * DESCRIPTION:
 *
 **/
static int iface_queue_buf_int(iface_buf_mgr_t *buf_mgr,
  uint32_t bufq_handle, int buf_idx, uint32_t dirty_buf, int vfe_fd,
  uint32_t buffer_access)
{
  int rc;
  struct msm_isp_qbuf_info qbuf_info;
  iface_bufq_t *bufq = IFACE_GET_BUFQ(buf_mgr, bufq_handle);

  if (!bufq) {
    CDBG_ERROR("%s: cannot find bufq with handle 0x%x\n",
      __func__, bufq_handle);
    return -1;
  }

#ifdef CACHE_PHASE2
  /* Invalidate cache only for all buffer */
  qbuf_info.buffer_access = 0;
#endif
  if (bufq->image_bufs[buf_idx].cached &&
    (buffer_access != 0)) {
  /* if the buffer is cached it needs to be invalidated first */
    iface_do_cache_inv_ion(buf_mgr->ion_fd, &bufq->image_bufs[buf_idx],
     CPU_HAS_READ);
  }

  qbuf_info.handle = bufq->kernel_bufq_handle;
  qbuf_info.buf_idx = bufq->image_bufs[buf_idx].buffer.index;
  iface_copy_planes_from_v4l2_buffer(&qbuf_info.buffer,
    &(bufq->image_bufs[buf_idx].buffer));
  qbuf_info.dirty_buf = dirty_buf;

  CDBG("%s: queue buf_info: handle = %x, buf idx = %d, dirty ? = %d"
    "\n", __func__, qbuf_info.handle,
    qbuf_info.buf_idx, qbuf_info.dirty_buf);
  rc = ioctl(vfe_fd, VIDIOC_MSM_ISP_ENQUEUE_BUF, &qbuf_info);
  if (rc < 0) {
    CDBG_ERROR("%s: queue buf to kernel failed, rc = -1\n", __func__);
    return -1;
  }

  bufq->image_bufs[buf_idx].is_reg = TRUE;

  return 0;
}

/** iface_unmap_buf
 *
 * DESCRIPTION:
 *
 **/
int iface_unmap_buf(
  iface_buf_mgr_t *buf_mgr __unused,
  int              vfe_fd,
  uint32_t         buf_fd)
{
  int rc = 0;
  struct msm_isp_unmap_buf_req unmap_req;

  /* Unmap offline mode buffer after read done */
  unmap_req.fd = buf_fd;
  rc = ioctl(vfe_fd, VIDIOC_MSM_ISP_UNMAP_BUF, &unmap_req);
  if (rc < 0) {
    CDBG_ERROR("%s: unmap buf in kernel failed, rc = -1\n", __func__);
    return -1;
  }
  return rc;
}

/** iface_queue_buf
 *
 * DESCRIPTION:
 *
 **/
int iface_queue_buf(iface_buf_mgr_t *buf_mgr,
  uint32_t bufq_handle, int buf_idx, uint32_t dirty_buf, int vfe_fd,
  uint32_t buffer_access)
{
  int rc;
  struct msm_isp_qbuf_info qbuf_info;
  iface_bufq_t *bufq = IFACE_GET_BUFQ(buf_mgr, bufq_handle);

  if (!bufq) {
    CDBG_ERROR("%s: error, bufq is NULL\n", __func__);
    return -1;
  }

  pthread_mutex_lock(&bufq->mutex);
  if (vfe_fd <= 0) {
    if (bufq->vfe_fds[0] > 0)
      vfe_fd = bufq->vfe_fds[0];
    else if (bufq->vfe_fds[1] > 0)
      vfe_fd= bufq->vfe_fds[1];
    else {
      CDBG_ERROR("%s: error, do not have VFE fd\n", __func__);
      rc = -1;
      goto end;
    }
  }

  rc = iface_queue_buf_int(buf_mgr, bufq_handle, buf_idx, dirty_buf, vfe_fd,
    buffer_access);

end:
  pthread_mutex_unlock(&bufq->mutex);

  return rc;
}

/** iface_get_buf_vaddr
 *
 * DESCRIPTION:
 *
 **/
void *iface_get_buf_vaddr(iface_buf_mgr_t *buf_mgr,
  uint32_t bufq_handle, int buf_idx)
{
  int rc = 0;
  iface_bufq_t *bufq = IFACE_GET_BUFQ(buf_mgr, bufq_handle);

  if (!bufq) {
    CDBG_ERROR("%s: cannot find bufq with handle 0x%x\n",
      __func__, bufq_handle);
    return NULL;
  }

  pthread_mutex_lock(&bufq->mutex);
  if (bufq->used && bufq->total_num_buffer > buf_idx) {
    pthread_mutex_unlock(&bufq->mutex);
    return bufq->image_bufs[buf_idx].vaddr;
  }
  pthread_mutex_unlock(&bufq->mutex);

  return NULL;
}

/** iface_get_buf_by_idx
 *
 * DESCRIPTION:
 *
 **/
iface_frame_buffer_t *iface_get_buf_by_idx(iface_buf_mgr_t *buf_mgr,
  uint32_t bufq_handle, int buf_idx)
{
  int rc = 0;
  iface_bufq_t *bufq = IFACE_GET_BUFQ(buf_mgr, bufq_handle);
  iface_frame_buffer_t *image_buf = NULL;

  if (!bufq) {
    CDBG_ERROR("%s: cannot find bufq with handle 0x%x\n",
      __func__, bufq_handle);
    return NULL;
  }

  pthread_mutex_lock(&bufq->mutex);
  if (bufq->used && bufq->total_num_buffer > buf_idx) {
    image_buf = &bufq->image_bufs[buf_idx];
  }
  pthread_mutex_unlock(&bufq->mutex);

  return image_buf;
}

/** iface_find_matched_bufq_handle_by_kernel_handle
 *
 * DESCRIPTION:
 *
 **/
uint32_t iface_find_matched_bufq_handle_by_kernel_handle(
  iface_buf_mgr_t *buf_mgr, uint32_t kernel_bufq_handle)
{
  int i;
  iface_bufq_t *tmp_bufq = NULL;
  uint32_t bufq_handle = 0;

  if (!buf_mgr) {
    CDBG_ERROR("%s: Null, buf_mgr %p, can not find buffer queue\n", __func__, buf_mgr);
    return -1;
  }

  pthread_mutex_lock(&buf_mgr->mutex);
  for (i = 0; i < IFACE_MAX_NUM_BUF_QUEUE; i++) {
    tmp_bufq = &buf_mgr->bufq[i];
    if(tmp_bufq->used &&
       tmp_bufq->kernel_bufq_handle == kernel_bufq_handle){
      bufq_handle = tmp_bufq->user_bufq_handle;
      break;
    }
  }
  pthread_mutex_unlock(&buf_mgr->mutex);

  return bufq_handle;
}

/** iface_find_matched_bufq_handle
 *
 * DESCRIPTION:
 *
 **/
uint32_t iface_find_matched_bufq_handle(iface_buf_mgr_t *buf_mgr,
  uint32_t session_id, uint32_t stream_id)
{
  int i;
  iface_bufq_t *tmp_bufq;
  uint32_t bufq_handle = 0;

  pthread_mutex_lock(&buf_mgr->mutex);
  for (i = 0; i < IFACE_MAX_NUM_BUF_QUEUE; i++) {
    tmp_bufq = &buf_mgr->bufq[i];
    if(tmp_bufq->used &&
       tmp_bufq->session_id == session_id &&
       tmp_bufq->stream_id == stream_id){
      bufq_handle = tmp_bufq->user_bufq_handle;
      break;
    }
  }
  pthread_mutex_unlock(&buf_mgr->mutex);

  return bufq_handle;
}

/** iface_save_vfe_fd
 *
 * DESCRIPTION:
 *
 **/
static int iface_save_vfe_fd(iface_bufq_t *bufq, int vfe_fd)
{
  if (bufq->vfe_fds[0] == 0)
    bufq->vfe_fds[0] = vfe_fd;
  else if (bufq->vfe_fds[1] == 0)
    bufq->vfe_fds[1] = vfe_fd;
  else
    return -1;

  bufq->num_vfe_fds++;

  return 0;
}

/** iface_remove_vfe_fd
 *
 * DESCRIPTION:
 *
 **/
static int iface_remove_vfe_fd(iface_bufq_t *bufq, int vfe_fd)
{
  if (vfe_fd <= 0) {
    CDBG_ERROR("%s: invalid vfe_fd = %d\n", __func__, vfe_fd);
    return -1;
  }

  if (bufq->vfe_fds[0] == vfe_fd)
    bufq->vfe_fds[0] = 0;
  else if (bufq->vfe_fds[1] == vfe_fd)
    bufq->vfe_fds[1] = 0;
  else
    return -1;

  bufq->num_vfe_fds--;

  return 0;
}

/** iface_register_buf_list_update
 *    @buf_mgr: buf manager
 *    @bufq_handle: buffer queue
 *    @buf_request: cotain info about buf_mgr and bufq
 *    @vfe_fd : hw id
 *
 *  HAL has updated buffer list. Now we have to update and
 *  register new buffers to kernel.
 *
 *  This function runs in MCT thread context
 *
 *  Returns 0 for success and negative error for failure
 **/
int iface_register_buf_list_update(iface_buf_mgr_t *buf_mgr,
  uint32_t bufq_handle, iface_buf_request_t *buf_request, int vfe_fd)
{
  int i = 0;
  iface_bufq_t *bufq;
  int rc = 0;

  bufq = IFACE_GET_BUFQ(buf_mgr, bufq_handle);
  if (!bufq) {
    CDBG_ERROR("%s: cannot find bufq with handle 0x%x\n",
      __func__, bufq_handle);
    return -1;
  }

  pthread_mutex_lock(&buf_mgr->mutex);

  /* total number of buffers cannot be update. */
  if (buf_request->total_num_buf != bufq->total_num_buffer) {
    CDBG("%s: Total number of buffers cannot be update!", __func__);
    buf_request->total_num_buf = bufq->total_num_buffer;
  }

  /*Validate buf request and populate info such as updated num_buffer*/
  rc = iface_validate_buf_request(buf_mgr, buf_request, bufq_handle);
  if (rc < 0) {
    CDBG_ERROR("%s: validate_buf failed\n", __func__);
    pthread_mutex_unlock(&buf_mgr->mutex);
    goto queue_buf_error;
  }

  /*Enqueue additional buffers to kernel*/
  rc = iface_queue_buf_list_update(buf_mgr, bufq, vfe_fd);
  if (rc < 0) {
    CDBG_ERROR("%s: cannot enqueue additional bufs\n", __func__);
    pthread_mutex_unlock(&buf_mgr->mutex);
    goto queue_buf_error;
  }
  pthread_mutex_unlock(&buf_mgr->mutex);

queue_buf_error:
  CDBG("%s: X,rc = %d\n", __func__, rc);
  return rc;
}

/** iface_unregister_buf_list
 *    @buf_mgr: buf manager
 *    @bufq_handle: buffer queue
 *    @buf_request: cotain info about buf_mgr and bufq
 *    @vfe_fd : hw id
 *
 *  HAL has updated buffer list. Now we have to unregister buffers from kernel.
 *
 *  This function runs in MCT thread context
 *
 *  Returns 0 for success and negative error for failure
 **/
int iface_unregister_buf_list(iface_buf_mgr_t *buf_mgr,
  uint32_t bufq_handle, iface_buf_request_t *buf_request, int vfe_fd)
{
  int i = 0;
  iface_bufq_t *bufq;
  int rc = 0;

  bufq = IFACE_GET_BUFQ(buf_mgr, bufq_handle);
  if (!bufq) {
    CDBG_ERROR("%s: cannot find bufq with handle 0x%x\n",
      __func__, bufq_handle);
    return -1;
  }

  if (vfe_fd <= 0) {
    if (bufq->vfe_fds[0] > 0)
      vfe_fd = bufq->vfe_fds[0];
    else if (bufq->vfe_fds[1] > 0)
      vfe_fd = bufq->vfe_fds[1];
    else {
      CDBG_ERROR("%s: error, do not have VFE fd\n", __func__);
      return -1;
    }
  }

  pthread_mutex_lock(&buf_mgr->mutex);
  pthread_mutex_lock(&bufq->mutex);

  /* total number of buffers cannot be update. */
  buf_request->total_num_buf = bufq->total_num_buffer;

  if (!buf_request->use_native_buf) {
    find_map_buf_t map_buf;
    map_buf.cnt = bufq->current_num_buffer;
    map_buf.iface_map_bufs = bufq->image_bufs;
    map_buf.vfe_fd = vfe_fd;
    map_buf.bufq_handle = bufq->user_bufq_handle;
    map_buf.buf_mgr = buf_mgr;

    /*map the buf from buf_request to iface map buf*/
    mct_list_traverse(buf_request->img_buf_list, iface_deinit_hal_buffer,
      (void *)&map_buf);
    /* update buf request number after unmap buf */
    buf_request->current_num_buf = map_buf.cnt;
    bufq->current_num_buffer = buf_request->current_num_buf;
  }

  pthread_mutex_unlock(&bufq->mutex);
  pthread_mutex_unlock(&buf_mgr->mutex);

  return rc;
}

/** iface_register_buf
 *    @buf_mgr: buf manager
 *    @bufq_handle: buffer queue
 *    @vfe_fd : hw id
 *
 *  Buffer list is received , here we register new buffers to
 *  kernel.
 *
 *  Returns 0 for success and negative error for failure
 **/
int iface_register_buf(iface_buf_mgr_t *buf_mgr,
  uint32_t bufq_handle, int vfe_fd)
{
  int i = 0;
  iface_bufq_t *bufq;

  int rc = 0;

  bufq = IFACE_GET_BUFQ(buf_mgr, bufq_handle);
  if (!bufq) {
    CDBG_ERROR("%s: cannot find bufq with handle 0x%x, rc = -1\n",
      __func__, bufq_handle);
    return -1;
  }

  pthread_mutex_lock(&bufq->mutex);
  if (iface_save_vfe_fd(bufq, vfe_fd) < 0) {
    CDBG_ERROR("%s: cannot save vfe_fd %d with handle 0x%x, rc = -1\n",
      __func__, vfe_fd, bufq_handle);
    pthread_mutex_unlock(&bufq->mutex);
    return -1;
  }

  if (bufq->num_vfe_fds > 1) {
    pthread_mutex_unlock(&bufq->mutex);
    return 0;
  }

  if (bufq->use_native_buf)
    bufq->stream_id = bufq->stream_id | ISP_NATIVE_BUF_BIT;

  /*Request kernel buffer queue*/
  rc = iface_request_kernel_bufq(buf_mgr, bufq, vfe_fd);
  if (rc < 0) {
    CDBG_ERROR("%s: iface_request_kernel_bufq failed, rc = -1\n", __func__);
    goto request_buf_error;
  }

  /*Enqueue buffer to kernel*/
  rc = iface_queue_buf_all(buf_mgr, bufq, vfe_fd);
  if (rc < 0) {
    CDBG_ERROR("%s: iface_queue_buf_all failed, rc = -1\n", __func__);
    goto queue_buf_error;
  }
  pthread_mutex_unlock(&bufq->mutex);

  return 0;

queue_buf_error:
  iface_release_kernel_bufq(buf_mgr, bufq, vfe_fd);

request_buf_error:
  pthread_mutex_unlock(&bufq->mutex);

  return rc;
}
/** iface_free_buf
 *
 * DESCRIPTION:
 *
 **/
boolean iface_free_buf(void *data, void *user_data __unused)
{
  if (!data) {
    return FALSE;
  }
  free(data);

  return TRUE;
}

/** iface_append_native_buff_to_list
 *
 * DESCRIPTION:
 *
 **/
static int iface_append_native_buff_to_list(mct_list_t **img_buf_list,
  iface_frame_buffer_t *native_buff, cam_frame_len_offset_t *buf_info)
{
  mct_stream_map_buf_t *img_buf;
  uint32_t i;

  img_buf = calloc(1, sizeof(*img_buf));
  if (!img_buf) {
    CDBG_ERROR("%s: malloc fail\n", __func__);
    return -1;
  }

  img_buf->buf_index = native_buff->buffer.index;
  img_buf->buf_size = buf_info->frame_len;
  img_buf->buf_type = CAM_MAPPING_BUF_TYPE_STREAM_BUF;
  img_buf->num_planes = native_buff->buffer.length;
  for (i = 0; i < img_buf->num_planes; i++) {
    img_buf->buf_planes[i].fd = native_buff->fd;
    img_buf->buf_planes[i].offset = native_buff->buffer.m.planes[i].data_offset;
    img_buf->buf_planes[i].size = native_buff->buffer.m.planes[i].length;
    img_buf->buf_planes[i].buf = native_buff->vaddr;
    img_buf->buf_planes[i].stride = buf_info->mp[i].stride;
    img_buf->buf_planes[i].scanline = buf_info->mp[i].scanline;
  }
  *img_buf_list = mct_list_append(*img_buf_list, img_buf, NULL, NULL);

  return 0;
}

/** iface_alloc_native_buf_handles
 *    @buf_mgr: buf manager
 *    @buf_info: buf allocation info
 *    @num_bufs : num of bugs to be allocated
 *    @adsp_heap : adsp heap flag
 *    @image_buff_list : img buf list to be allocated
 *
 *  allocate native buf handles in kernel
 *
 *  Returns 0 for success and negative error for failure
 **/
int iface_alloc_native_buf_handles(
  iface_buf_mgr_t        *buf_mgr,
  cam_frame_len_offset_t *buf_info,
  uint32_t                num_bufs,
  uint8_t                 adsp_heap,
  iface_frame_buffer_t    image_buff_list[IFACE_MAX_IMG_BUF],
  enum smmu_attach_mode security_mode)
{
  uint32_t j = 0;
  int32_t i = 0;
  int rc = 0;

  for (j = 0; j < num_bufs; j++) {
    rc = iface_init_native_buffer(&image_buff_list[j], j, buf_mgr->ion_fd,
      buf_info, 1, adsp_heap, security_mode);

    if (rc < 0) {
      i = j;
      goto native_buf_error;
    }
  }

  return rc;

  native_buf_error:
    CDBG_ERROR("%s: Error\n",__func__);
  while (i >= 0) {
    IFACE_DEINIT_BUF(&image_buff_list[i], buf_mgr->ion_fd);
    i--;
  }

  return rc;
}


/** iface_release_native_buf_handles
 *    @buf_mgr: buf manager
 *    @num_bufs : num of bugs to be allocated
 *    @image_buff_list : img buf list to be allocated
 *
 *  release native buf handles in kernel
 *
 *  Returns 0 for success and negative error for failure
 **/
int iface_release_native_buf_handles(iface_buf_mgr_t *buf_mgr,
  uint32_t num_bufs, iface_frame_buffer_t image_buff_list[IFACE_MAX_IMG_BUF])
{
  uint32_t i = 0;
  int rc = 0;

  for (i = 0; i < num_bufs; i++)
    IFACE_DEINIT_BUF(&image_buff_list[i], buf_mgr->ion_fd);

  return rc;
}

/** iface_update_native_buf_handles
 *    @buf_mgr: buf manager
 *    @buf_request :  buf request pointer
 *    @image_buff_list : img buf list to be used to update the buf req
 *
 *  release native buf handles in kernel
 *
 *  Returns 0 for success and negative error for failure
 **/
int iface_update_native_buf_handles(iface_buf_mgr_t *buf_mgr,
  iface_buf_request_t *buf_request,
  iface_frame_buffer_t image_buff_list[IFACE_MAX_IMG_BUF])
{
  int i = 0,j=0;
  iface_bufq_t *bufq;
  uint32_t bufq_handle;
  int rc = 0;
  CDBG("%s: Enter\n", __func__);

  bufq_handle = iface_find_matched_bufq_handle(buf_mgr,
    buf_request->session_id, buf_request->stream_id);
  if (bufq_handle > 0) {
    CDBG_ERROR("%s: bufq already existed!!!!!, bufq handle = %x, hw stream id = %x\n",
      __func__, bufq_handle, buf_request->stream_id);
    /* buf queue already requested. */
    buf_request->buf_handle = bufq_handle;
    return 0;
  }

  pthread_mutex_lock(&buf_mgr->mutex);
  /*Get new userspace buffer queue from buf mgr*/
  bufq_handle = iface_get_bufq_handle(buf_mgr);
  /* already set the used bit in bufq, no lock is needed */
  pthread_mutex_unlock(&buf_mgr->mutex);

  if (bufq_handle == 0) {
    CDBG_ERROR("%s: get bufq handle = 0 , rc=  -1\n", __func__);
    return -1;
  }

  /*Validate buf request and populate info*/
  rc = iface_validate_buf_request(buf_mgr,
    buf_request, bufq_handle);
  if (rc < 0)
    return rc;

  /*Allocate buffer for native buffer or
    populate buf info from HAL mapped buffer */
  bufq = IFACE_GET_BUFQ(buf_mgr, bufq_handle);
  if (!bufq) {
    CDBG_ERROR("%s: cannot find bufq with handle 0x%x\n",
      __func__, bufq_handle);
    return -1;
  }

  if (bufq->use_native_buf) {
    for (i = 0; i < buf_request->total_num_buf; i++) {
      bufq->image_bufs[i] = image_buff_list[i];
      bufq->image_bufs[i].buffer.m.planes = &bufq->image_bufs[i].planes[0];
      memset(&image_buff_list[i], 0, sizeof(image_buff_list[i]));
    }
  }

  CDBG("%s: Exit\n", __func__);
  return rc;
}

/** iface_request_buf
 *
 * DESCRIPTION:
 *
 **/
int iface_request_buf(iface_buf_mgr_t *buf_mgr, iface_buf_request_t *buf_request)
{
  int i = 0;
  iface_bufq_t *bufq;
  uint32_t bufq_handle;
  int rc = 0;


  CDBG("%s: request buf for hw stream id = %x\n",
       __func__, buf_request->stream_id);

  bufq_handle = iface_find_matched_bufq_handle(buf_mgr,
    buf_request->session_id, buf_request->stream_id);
  if (bufq_handle > 0) {
    CDBG_ERROR("%s: bufq already existed!!!!!, bufq handle = %x, hw stream id = %x\n",
      __func__, bufq_handle, buf_request->stream_id);
    /* buf queue already requested. */
    buf_request->buf_handle = bufq_handle;
    return 0;
  }

  pthread_mutex_lock(&buf_mgr->mutex);
  /*Get new userspace buffer queue from buf mgr*/
  bufq_handle = iface_get_bufq_handle(buf_mgr);

  /* already set the used bit in bufq, no lock is needed */
  pthread_mutex_unlock(&buf_mgr->mutex);
  if (bufq_handle == 0) {
    CDBG_ERROR("%s: get bufq handle = 0 , rc=  -1\n", __func__);
    return -1;
  }

  /*Validate buf request and populate info*/
  rc = iface_validate_buf_request(buf_mgr,
    buf_request, bufq_handle);
  if (rc < 0)
    return rc;

  /*Allocate buffer for native buffer or
    populate buf info from HAL mapped buffer */
  bufq = IFACE_GET_BUFQ(buf_mgr, bufq_handle);
  if (!bufq) {
    CDBG_ERROR("%s: cannot find bufq with handle 0x%x\n",
      __func__, bufq_handle);
    return -1;
  }

  if (bufq->use_native_buf && !buf_request->img_buf_list) {
    for (i = 0; i < buf_request->total_num_buf; i++) {
      rc = iface_init_native_buffer(&bufq->image_bufs[i],
        i, buf_mgr->ion_fd, &buf_request->buf_info, buf_request->cached,
        buf_request->adsp_heap, buf_request->security_mode);
      if (rc < 0)
        goto native_buf_error;
      if (!(buf_request->stream_id & ISP_STATS_STREAM_BIT)) {
         rc = iface_append_native_buff_to_list(&buf_request->img_buf_list,
           &bufq->image_bufs[i], &buf_request->buf_info);
         if (rc < 0) {
           CDBG_ERROR("%s: failed iface_append_native_buff_to_list\n", __func__);
           goto native_buf_error;
         }
      }
    }
  }

  return 0;

native_buf_error:
  if (bufq->use_native_buf) {
    for (i--; i >= 0; i--) {
      IFACE_DEINIT_BUF(&bufq->image_bufs[i], buf_mgr->ion_fd);
    }
    mct_list_free_all(buf_request->img_buf_list, iface_free_buf);
  }
  pthread_mutex_lock(&buf_mgr->mutex);
  iface_free_bufq_handle(buf_mgr, bufq);
  pthread_mutex_unlock(&buf_mgr->mutex);

  return rc;
}

/** iface_unregister_buf
 *
 * DESCRIPTION:
 *
 **/
int iface_unregister_buf(iface_buf_mgr_t *buf_mgr,
  uint32_t bufq_handle, int vfe_fd)
{
  int i;
  iface_bufq_t *bufq = IFACE_GET_BUFQ(buf_mgr, bufq_handle);

  if (!bufq) {
    CDBG_ERROR("%s: cannot find bufq with handle 0x%x\n",
      __func__, bufq_handle);
    return -1;
  }

  if (!bufq->used) {
    CDBG_ERROR("%s: buf is not used, return -1\n",
      __func__);
    return -1;
  }

  /* release kernel bufq will unprepare/free buf in kernel buf queue
     also, it will free the buffer queue handle */
  pthread_mutex_lock(&bufq->mutex);
  iface_remove_vfe_fd(bufq, vfe_fd);
  if (bufq->num_vfe_fds > 0) {
    pthread_mutex_unlock(&bufq->mutex);
    return 1;
  }
  iface_release_kernel_bufq(buf_mgr, bufq, vfe_fd);
  pthread_mutex_unlock(&bufq->mutex);

  return 0;
}

/** iface_release_buf
 *
 * DESCRIPTION:
 *
 **/
void iface_release_buf(iface_buf_mgr_t *buf_mgr, uint32_t bufq_handle,
  mct_list_t *img_buffer_list)
{
  int i;
  iface_bufq_t *bufq = IFACE_GET_BUFQ(buf_mgr, bufq_handle);

  if (!bufq) {
    CDBG_ERROR("%s: cannot find bufq with handle 0x%x\n",
      __func__, bufq_handle);
    return;
  }

  if (!bufq->used)
    return;

  if (bufq->use_native_buf) {
    for (i = 0; i < bufq->total_num_buffer; i++)
      IFACE_DEINIT_BUF(&bufq->image_bufs[i], buf_mgr->ion_fd);
    if (img_buffer_list)
      mct_list_free_all(img_buffer_list, iface_free_buf);
  }

  pthread_mutex_lock(&buf_mgr->mutex);
  iface_free_bufq_handle(buf_mgr, bufq);
  pthread_mutex_unlock(&buf_mgr->mutex);

}

/** iface_open_buf_mgr
 *
 * DESCRIPTION:
 *
 **/
int iface_open_buf_mgr(iface_buf_mgr_t *buf_mgr)
{
  pthread_mutex_lock(&buf_mgr->mutex);
  if (buf_mgr->use_cnt++) {
    pthread_mutex_unlock(&buf_mgr->mutex);
    return 0;
  }
#ifndef TARGET_ION_ABI_VERSION
  buf_mgr->ion_fd = open("/dev/ion", O_RDONLY | O_SYNC);
#else
  buf_mgr->ion_fd = ion_open();
#endif

  if (buf_mgr->ion_fd < 0) {
    CDBG_ERROR("%s: ion open failed\n", __func__);
    buf_mgr->use_cnt = 0;
    pthread_mutex_unlock(&buf_mgr->mutex);
    return -1;
  }
  pthread_mutex_unlock(&buf_mgr->mutex);

  return 0;
}

/** iface_close_buf_mgr
 *
 * DESCRIPTION:
 *
 **/
void iface_close_buf_mgr(iface_buf_mgr_t *buf_mgr)
{
  pthread_mutex_lock(&buf_mgr->mutex);
  if (--buf_mgr->use_cnt) {
    pthread_mutex_unlock(&buf_mgr->mutex);
    return;
  }

  if (buf_mgr->ion_fd) {
    close(buf_mgr->ion_fd);
    buf_mgr->ion_fd = 0;
  }
  pthread_mutex_unlock(&buf_mgr->mutex);

  return;
}

/** iface_init_buf_mgr
 *
 * DESCRIPTION:
 *
 **/
int iface_init_buf_mgr(iface_buf_mgr_t *buf_mgr)
{
  int i;

  if (buf_mgr->use_cnt++)
    return 0;

  memset(buf_mgr, 0, sizeof(iface_buf_mgr_t));
  memset(buf_mgr->bufq, 0, sizeof(iface_bufq_t) * IFACE_MAX_NUM_BUF_QUEUE);

  pthread_mutex_init(&buf_mgr->mutex, NULL);
  pthread_mutex_init(&buf_mgr->req_mutex, NULL);

  for (i = 0; i < IFACE_MAX_NUM_BUF_QUEUE; i++)
    pthread_mutex_init(&buf_mgr->bufq[i].mutex, NULL);

  return 0;
}

/** iface_deinit_buf_mgr
 *
 * DESCRIPTION:
 *
 **/
void iface_deinit_buf_mgr(iface_buf_mgr_t *buf_mgr)
{
  int i;

  if (--buf_mgr->use_cnt)
    return;

  for (i = 0; i < IFACE_MAX_NUM_BUF_QUEUE; i++)
    pthread_mutex_destroy(&buf_mgr->bufq[i].mutex);

  pthread_mutex_destroy(&buf_mgr->req_mutex);
  pthread_mutex_destroy(&buf_mgr->mutex);
}

/** iface_close_ion
 *
 * DESCRIPTION:
 *
 **/
void iface_close_ion(int ion_fd)
{

  if (ion_fd > 0)
#ifndef TARGET_ION_ABI_VERSION
    close(ion_fd);
#else
    ion_close(ion_fd);
#endif //TARGET_ION_ABI_VERSION
}
/** iface_open_ion
 *
 * DESCRIPTION:
 *
 **/
int iface_open_ion(void)
{
  int fd;

#ifndef TARGET_ION_ABI_VERSION
  fd = open("/dev/ion", O_RDONLY | O_SYNC);
#else
  fd = ion_open();
#endif
return fd;
}

int iface_get_current_num_buffer(iface_buf_mgr_t *buf_mgr, uint32_t bufq_handle)
{
  iface_bufq_t *bufq = IFACE_GET_BUFQ(buf_mgr, bufq_handle);
  if (!bufq) {
    CDBG_ERROR("%s: cannot find bufq with handle 0x%x\n",
      __func__, bufq_handle);
    return 0;
  }
  return bufq->current_num_buffer;
}
