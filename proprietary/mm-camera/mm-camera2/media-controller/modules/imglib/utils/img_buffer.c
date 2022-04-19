/**********************************************************************
*  Copyright (c) 2013-2016, 2018-2019 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include <sys/ioctl.h>
#include "img_buffer.h"
#include <linux/msm_ion.h>
#include "img_common.h"

#ifdef __CAM_FCV__
#include <fastcv/fastcv.h>
#endif

// #define IMG_BUF_POOL_DEBUG

#ifdef IMG_BUF_POOL_DEBUG
#undef IDBG_LOW
#undef IDBG_MED
#undef IDBG_HIGH
#define IDBG_LOW IDBG_INFO
#define IDBG_MED IDBG_INFO
#define IDBG_HIGH IDBG_INFO
#endif //IMG_BUF_POOL_DEBUG

#ifdef TARGET_ION_ABI_VERSION
#include <linux/dma-buf.h>
#endif

static img_buffer_priv_t *g_img_buff_info;
pthread_mutex_t g_img_buff_lock = PTHREAD_MUTEX_INITIALIZER;


/** img_buf_node_t:
 *
 *  @id: Unique id of the node
 *  @scaled_frame: Scaled frame in the buffer pool
 *  @ref_cnt: reference count
 *  @is_init: indicate if the memory handle is valid
 *  @handle: memory handle
 *
 *  This data structure represents buffer node
 */
typedef struct {
  int32_t id;
  img_frame_t scaled_frame;
  uint8_t ref_cnt;
  int8_t is_init;
  img_mem_handle_t handle;
} img_buf_node_t;

/**
 * maximum size of the buffer node
 */
#define MAX_BUF_NODE 4

/** img_buf_pool_t:
 *  @node: list of buffer nodes
 *  @params: buffer parameters
 *  @length: length of the buffer
 *  @frame_info: frame structure to hold the frame info
 *  @frame_info_valid: Flag to indicate whether the frame info
 *                   is valid
 *  @lock: mutex
 *
 */
typedef struct {
  img_buf_node_t node[MAX_BUF_NODE];
  img_buf_params_t params;
  uint32_t length;
  img_frame_t frame_info;
  int8_t frame_info_valid;
  pthread_mutex_t lock;
} img_buf_pool_t;

/** img_buffer_open
 *
 *  Arguments:
 *     None
 *
 *  Return:
 *     Ion device fd. -1 on failure
 *
 *  Description:
 *      Open Ion device and returns the fd. Should be called
 *      only once for all modules in imglib
 *
 **/
int img_buffer_open()
{
  QIMG_LOCK(&g_img_buff_lock);

  if (!g_img_buff_info) {
    g_img_buff_info = (img_buffer_priv_t *) calloc(1, sizeof(*g_img_buff_info));
    if (!g_img_buff_info) {
      IDBG_ERROR("%s %d: Cannot allocate memory", __func__, __LINE__);
      QIMG_UNLOCK(&g_img_buff_lock);
      return -1;
    }
    g_img_buff_info->ref_count = 0;
    g_img_buff_info->ion_fd = -1;
    //Open ION device
#ifndef TARGET_ION_ABI_VERSION
    g_img_buff_info->ion_fd = open("/dev/ion", O_RDONLY);
#else
    g_img_buff_info->ion_fd = ion_open();
#endif
    if (g_img_buff_info->ion_fd < 0) {
      IDBG_ERROR("%s:%d]Ion open failed", __func__, __LINE__);
      free(g_img_buff_info);
      g_img_buff_info = NULL;
      QIMG_UNLOCK(&g_img_buff_lock);
      return -1;
    }
  }
  g_img_buff_info->ref_count++;
  QIMG_UNLOCK(&g_img_buff_lock);
  return g_img_buff_info->ion_fd;
}


/** img_buffer_close
 *
 *  Arguments:
 *
 *  Return:
 *     0 for success, -ve values for failure
 *
 *  Description:
 *      Close the Ion device
 *
 **/
int img_buffer_close()
{
  int lrc = 0;
  QIMG_LOCK(&g_img_buff_lock);

  if (NULL == g_img_buff_info) {
    IDBG_ERROR("%s %d] Ion device already closed", __func__, __LINE__ );
    QIMG_UNLOCK(&g_img_buff_lock);
    return 0;
  }

  g_img_buff_info->ref_count--;

  //Release the Ion device only when ref count is 0
  if (g_img_buff_info->ref_count == 0) {
    if (g_img_buff_info->ion_fd >= 0) {
#ifndef TARGET_ION_ABI_VERSION
      lrc = close(g_img_buff_info->ion_fd);
#else
      lrc = ion_close(g_img_buff_info->ion_fd);
#endif
      g_img_buff_info->ion_fd = -1;
    } else {
    IDBG_ERROR("%s %d] Invalid Ion fd, Cannot close device",
      __func__, __LINE__ );
    }
    free(g_img_buff_info);
    g_img_buff_info = NULL;
  }
  QIMG_UNLOCK(&g_img_buff_lock);
  return lrc;
}

/** img_buffer_get
 *
 *  Arguments:
 *     @type: buffer type
 *     @heapid: heapid needs to be provided if buffer type is
 *            IMG_BUFFER_ION_CUSTOM. -1 otherwise.
 *     @cached: Flag to indicate whether the buffer needs to be
 *            cached
 *     @length: buffer size
 *     @p_handle: memory output
 *
 *
 *  Return:
 *     0 for success, -ve values for failure
 *
 *  Description:
 *      allocates buffer
 *
 **/
int img_buffer_get(img_buf_type_t type, int heapid, int8_t cached, int length,
    img_mem_handle_t *p_handle)
{
  img_buffer_t *p_buffer;
  int lrc = 0;

  if ((NULL == p_handle) || (length <= 0)) {
    IDBG_ERROR("%s:%d] invalid input", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  p_buffer = malloc(sizeof(img_buffer_t));
  if (NULL == p_buffer) {
    IDBG_ERROR("%s:%d] No memory", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }

  /* heap buffer */
  if (IMG_BUFFER_HEAP == type) {
    IDBG_MED("%s:%d] try buff_type IMG_BUFFER_HEAP(%d) size %zu ",
      __func__, __LINE__, (int) type, length);
    p_buffer->fd = -1;
    p_buffer->addr = malloc((size_t)length);
    if (!p_buffer->addr) {
      IDBG_ERROR("%s %d]: Memory allocation failed", __func__, __LINE__);
      goto ion_open_failed;
    }
    p_buffer->alloc.len = (size_t)length;
    goto alloc_success;
  }

  /* ION buffer */
  if (NULL == g_img_buff_info) {
    IDBG_ERROR("%s:%d:] Opening ION device since fd is invalid",
      __func__, __LINE__);
    p_buffer->ion_fd = img_buffer_open();
    if (p_buffer->ion_fd < 0) {
      IDBG_ERROR("%s:%d:] Ion open failed",
      __func__, __LINE__);
      goto ion_open_failed;
    }
  } else {
    p_buffer->ion_fd = g_img_buff_info->ion_fd;
  }

  p_buffer->alloc.len = (size_t)length;
  p_buffer->alloc.align = 4096;
  p_buffer->alloc.flags = (cached) ? ION_FLAG_CACHED : 0;

  if (type == IMG_BUFFER_ION_CUSTOM) {
    if (heapid < 0) {
      IDBG_ERROR("%s %d: Invalid heap id %d", __func__, __LINE__, heapid);
      goto ion_open_failed;
    }
    p_buffer->alloc.heap_id_mask = (uint32_t)ION_HEAP(heapid);
  } else {
#ifndef TARGET_ION_ABI_VERSION
    p_buffer->alloc.heap_id_mask = (type == IMG_BUFFER_ION_ADSP) ?
      (0x1 << ION_ADSP_HEAP_ID) :
      (0x1 << ION_IOMMU_HEAP_ID);
#else
    p_buffer->alloc.heap_id_mask = (type == IMG_BUFFER_ION_ADSP) ?
      (0x1 << ION_ADSP_HEAP_ID) :
      (0x1 << ION_SYSTEM_HEAP_ID);
#endif
  }
  IDBG_MED("%s:%d]ION Alloc buff_type %d size %zu HeapIdMask 0x%0x",
    __func__, __LINE__, (int)type, length, p_buffer->alloc.heap_id_mask);

  /* Make it page size aligned */
  p_buffer->alloc.len = (p_buffer->alloc.len + 4095) & (~4095U);
#ifndef TARGET_ION_ABI_VERSION
  lrc = ioctl(p_buffer->ion_fd, ION_IOC_ALLOC, &p_buffer->alloc);
  if (lrc < 0) {
    IDBG_ERROR("%s:%d] ION allocation failed len %zu", __func__,
      __LINE__, p_buffer->alloc.len);
    goto ion_open_failed;
  }

  p_buffer->ion_info_fd.handle = p_buffer->alloc.handle;
  lrc = ioctl(p_buffer->ion_fd, ION_IOC_SHARE,
    &p_buffer->ion_info_fd);
  if (lrc < 0) {
    IDBG_ERROR("%s:%d] ION map failed %s", __func__, __LINE__,
      strerror(errno));
    goto ion_map_failed;
  }
#else
  int mapFd = -1;
  lrc = ion_alloc_fd(p_buffer->ion_fd, p_buffer->alloc.len,
     p_buffer->alloc.align, p_buffer->alloc.heap_id_mask,
     p_buffer->alloc.flags, &mapFd);
  if (lrc < 0) {
    IDBG_ERROR("%s:%d] ION allocation failed len %zu", __func__,
      __LINE__, p_buffer->alloc.len);
    goto ion_open_failed;
  }
  p_buffer->ion_info_fd.fd = mapFd;
#endif
  p_buffer->addr = mmap(NULL, p_buffer->alloc.len, PROT_READ  | PROT_WRITE,
    MAP_SHARED, p_buffer->ion_info_fd.fd, 0);
  p_buffer->fd = p_buffer->ion_info_fd.fd;

  if (p_buffer->addr == MAP_FAILED) {
    IDBG_ERROR("%s:%d] ION_MMAP_FAILED: %s (%d)", __func__, __LINE__,
      strerror(errno), errno);
    goto ion_map_failed;
  }

  p_buffer->cached = cached;

alloc_success:
  p_handle->fd = p_buffer->fd;
  p_handle->vaddr = p_buffer->addr;
  p_handle->length = (uint32_t)p_buffer->alloc.len;
  p_handle->handle = (void *)p_buffer;
  IDBG_MED("%s:%d] Success fd %d addr %p", __func__, __LINE__,
    p_handle->fd, p_handle->vaddr);
  return 0;

ion_map_failed:
#ifndef TARGET_ION_ABI_VERSION
  ioctl(p_buffer->ion_fd, ION_IOC_FREE, &p_buffer->ion_info_fd.handle);
#else
  close( p_buffer->ion_info_fd.fd);
#endif
ion_open_failed:
    free(p_buffer);

  return IMG_ERR_NO_MEMORY;

}

/** img_buffer_release:
 *
 *  Arguments:
 *     @p_handle: memory handle
 *
 *  Return:
 *     Standard imglib return values
 *
 *  Description:
 *      Deallocates buffer
 *
 **/
int img_buffer_release(img_mem_handle_t *p_handle)
{
  int lrc = 0;
  img_buffer_t *p_buffer;
  size_t lsize;

  if ((NULL == p_handle) ||
    (NULL == p_handle->handle)) {
    IDBG_ERROR("%s:%d] invalid input", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  p_buffer = (img_buffer_t *)p_handle->handle;

  /* heap buffer */
  if (p_handle->fd < 0) {
    if (p_buffer->addr) {
      free(p_buffer->addr);
      p_buffer->addr = NULL;
    }
    goto dealloc_success;
  }

  /*Ion buffer*/
  if (NULL == g_img_buff_info) {
    IDBG_ERROR("%s:%d:] Opening ION device since fd is invalid",
      __func__, __LINE__);
    p_buffer->ion_fd = img_buffer_open();
    if (p_buffer->ion_fd < 0) {
      IDBG_ERROR("%s:%d:] Ion open failed",
      __func__, __LINE__);
      return IMG_ERR_GENERAL;
    }
  } else {
    p_buffer->ion_fd = g_img_buff_info->ion_fd;
  }

  lsize = (p_buffer->alloc.len + 4095) & (~4095U);

  lrc = munmap(p_buffer->addr, lsize);
  if (lrc < 0) {
    IDBG_ERROR("%s:%d] unmap failed %s (%d)", __func__, __LINE__,
      strerror(errno), errno);
  }

  lrc = close(p_buffer->ion_info_fd.fd);
  if (lrc < 0) {
    IDBG_ERROR("%s:%d] ion fd close failed  %s (%d)", __func__, __LINE__,
      strerror(errno), errno);
  }

#ifndef TARGET_ION_ABI_VERSION
 lrc = ioctl(p_buffer->ion_fd, ION_IOC_FREE,
    &p_buffer->ion_info_fd.handle);
  if (lrc < 0) {
    IDBG_ERROR("%s:%d] ion free failed %s (%d)", __func__, __LINE__,
      strerror(errno), errno);
  }
#endif

 dealloc_success:
  if (p_buffer) {
    free(p_buffer);
    p_handle->handle = NULL;
    p_handle->vaddr = NULL;
    p_handle->fd = -1;
    p_handle->length = 0;
  }
  IDBG_MED("%s:%d] Success fd %d addr %p", __func__, __LINE__,
    p_handle->fd, p_handle->vaddr);

  return lrc;
}

/** img_buffer_cacheops:
 *
 *  Arguments:
 *     @p_handle: image memory handle
 *     @ops: cache operation type
 *     @mem_alloc_type: Memory Alloc type - internal to imglib
 *                    or external
 *
 *  Return:
 *     Standard imglib return values
 *
 *  Description:
 *      Cache operations for ION buffer
 *
 **/
int img_buffer_cacheops(img_mem_handle_t *p_handle, img_cache_ops_t ops,
  img_mem_alloc_type_t mem_alloc_type)
{

  int lrc = IMG_SUCCESS;
  img_buffer_t *p_buffer;

  if ((NULL == p_handle) ||
    (NULL == p_handle->handle)) {
    IDBG_ERROR("%s:%d] invalid input", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  if (ops == IMG_CACHE_NO_OP) {
    IDBG_MED("No cache operation");
    return IMG_SUCCESS;
  }

  p_buffer = (img_buffer_t *)p_handle->handle;

  /* heap buffer */
  if ((p_handle->fd < 0) || !p_buffer->cached) {
    IDBG_MED("%s:%d] no need of cache ops", __func__, __LINE__);
    return IMG_SUCCESS;
  }

  if (NULL == g_img_buff_info) {
    IDBG_ERROR("%s:%d:] Opening ION device since fd is invalid",
      __func__, __LINE__);
    p_buffer->ion_fd = img_buffer_open();
    if (p_buffer->ion_fd < 0) {
      IDBG_ERROR("%s:%d:] Ion open failed",
      __func__, __LINE__);
      return IMG_ERR_GENERAL;
    }
  } else {
    p_buffer->ion_fd = g_img_buff_info->ion_fd;
  }
 #ifndef TARGET_ION_ABI_VERSION
  struct ion_custom_data custom_data;
  struct ion_flush_data flush_data;
  struct ion_fd_data fd_data;
  struct ion_handle_data handle_data;
  uint32_t cmd;

  memset(&flush_data, 0, sizeof(struct ion_flush_data));
  memset(&custom_data, 0, sizeof(struct ion_custom_data));
  memset(&fd_data,  0 , sizeof(struct ion_fd_data));
  memset(&handle_data,  0 , sizeof(struct ion_handle_data));

  if (mem_alloc_type == IMG_EXTERNAL) {
    fd_data.fd = p_buffer->fd;
    lrc = ioctl (p_buffer->ion_fd, ION_IOC_IMPORT, &fd_data);
    if (lrc) {
      IDBG_ERROR("%s:%d: ION_IOC_IMPORT failed", __func__, __LINE__);
      return IMG_ERR_GENERAL;
    }
    flush_data.handle = fd_data.handle;
    handle_data.handle = fd_data.handle;
  } else {
    flush_data.handle =  p_buffer->ion_info_fd.handle;
    handle_data.handle = p_buffer->ion_info_fd.handle;
  }

  switch (ops) {
  case IMG_CACHE_INV:
    cmd = ION_IOC_INV_CACHES;
    break;
  case IMG_CACHE_CLEAN:
    cmd = ION_IOC_CLEAN_CACHES;
    break;
  default:
  case IMG_CACHE_CLEAN_INV:
    cmd = ION_IOC_CLEAN_INV_CACHES;
    break;
  }

  IDBG_HIGH("CAMCACHE : CacheOp : (vaddr=%p, fd=%d, size=%d, offset=%d), "
    "type=%s, ion_device_fd=%d",
    p_buffer->addr, p_buffer->fd, p_buffer->alloc.len, 0,
    (ops == IMG_CACHE_INV) ? "INV" :
    (ops == IMG_CACHE_CLEAN) ? "CLEAN" : "CLEAN_INV",
    p_buffer->ion_fd);

  flush_data.vaddr = p_buffer->addr;
  flush_data.fd = p_buffer->fd;
  flush_data.length = (uint32_t)p_buffer->alloc.len;

  custom_data.cmd = cmd;
  custom_data.arg = (unsigned long)&flush_data;

  lrc = ioctl(p_buffer->ion_fd, ION_IOC_CUSTOM, &custom_data);
  if (lrc < 0) {
    IDBG_ERROR("%s:%d] failed with errno %s\n", __func__, __LINE__,
      strerror(errno));
    lrc = IMG_ERR_GENERAL;
    goto ion_cache_inv_failed;
  } else
    IDBG_MED("%s:%d] success", __func__, __LINE__);

ion_cache_inv_failed:
  if (mem_alloc_type == IMG_EXTERNAL)
    ioctl(p_buffer->ion_fd, ION_IOC_FREE, &handle_data);

  return lrc;
#else
  struct dma_buf_sync buf_sync_start;
  struct dma_buf_sync buf_sync_end;

  /* ION_IOC_CLEAN_CACHES-->call the DMA_BUF_IOCTL_SYNC IOCTL with flags
     DMA_BUF_SYNC_START and DMA_BUF_SYNC_WRITE and then call the
     DMA_BUF_IOCTL_SYNC IOCTL with flags DMA_BUF_SYNC_END
     and DMA_BUF_SYNC_WRITE

     ION_IOC_INV_CACHES-->call the DMA_BUF_IOCTL_SYNC IOCT with flags
     DMA_BUF_SYNC_START and DMA_BUF_SYNC_WRITE and then call the
     DMA_BUF_IOCTL_SYNC IOCT with flags DMA_BUF_SYNC_END
     and DMA_BUF_SYNC_READ
  */
  switch (ops) {
  case IMG_CACHE_INV:
    buf_sync_start.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_WRITE;
    buf_sync_end.flags   = DMA_BUF_SYNC_END   | DMA_BUF_SYNC_READ;
    break;
  case IMG_CACHE_CLEAN:
    buf_sync_start.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_WRITE;
    buf_sync_end.flags   = DMA_BUF_SYNC_END   | DMA_BUF_SYNC_WRITE;
    break;
  default:
  case IMG_CACHE_CLEAN_INV:
    buf_sync_start.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_RW;
    buf_sync_end.flags   = DMA_BUF_SYNC_END   | DMA_BUF_SYNC_RW;
    break;
  }

  lrc = ioctl(p_buffer->fd, DMA_BUF_IOCTL_SYNC, &buf_sync_start.flags);
  if (lrc) {
    IDBG_ERROR("Failed first DMA_BUF_IOCTL_SYNC start\n");
  }

  lrc = ioctl(p_buffer->fd, DMA_BUF_IOCTL_SYNC, &buf_sync_end.flags);
  if (lrc) {
    IDBG_ERROR("Failed first DMA_BUF_IOCTL_SYNC end\n");
  }

  return lrc;
#endif
}

/** img_buf_pool_print
 *
 *  Arguments:
 *     @handle: buffer pool handle
 *
 *  Return:
 *     none
 *
 *  Description:
 *      API to print the buffers
 *
 **/
void img_buf_pool_print(void *handle)
{
  img_buf_node_t *p_node;
  int i = 0;
  img_buf_pool_t *p_pool = (img_buf_pool_t *)handle;

  if (!p_pool) {
    IDBG_ERROR("%s:%d] Error buf pool not init", __func__, __LINE__);
    return;
  }

  QIMG_LOCK(&p_pool->lock);
  for (i = 0; i < MAX_BUF_NODE; i++) {
    p_node = &p_pool->node[i];
    IDBG_INFO("%s:%d] Buffer [%d] ref_cnt %d",
      __func__, __LINE__,
      p_node->scaled_frame.frame_id, p_node->ref_cnt);
  }
  QIMG_UNLOCK(&p_pool->lock);
}

/** img_buf_pool_alloc
 *
 *  Arguments:
 *     @p_pool: buffer pool handle
 *     @idx: index of the node in which buffer needs to be
 *         allocated
 *
 *  Return:
 *     Imaging errors
 *
 *  Description:
 *      helper function to allocate the frame
 *
 **/
static int32_t img_buf_pool_alloc(img_buf_pool_t *p_pool, int idx)
{
  int32_t rc = IMG_SUCCESS;
  img_buf_node_t *p_node;

  p_node = &p_pool->node[idx];
  if (!p_node->is_init) {
    rc = img_buffer_get(IMG_BUFFER_ION_IOMMU, -1, TRUE, p_pool->length,
      &p_node->handle);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("%s:%d] Error getting buffers", __func__, __LINE__);
      goto error;
    }
    p_node->is_init = TRUE;
  }

error:
  return rc;
}

/** img_buf_pool_reset
 *
 *  Arguments:
 *     @handle: buffer pool handle
 *     @release_buf: Flag to indicate whether to release the
 *                 buffer or not
 *
 *  Return:
 *     Imaging errors
 *
 *  Description:
 *      API to reset and destroy all the buffers
 *
 **/
int32_t img_buf_pool_reset(void *handle, int8_t release_buf)
{
  int32_t rc = IMG_SUCCESS;
  img_buf_node_t *p_node;
  int i = 0;
  img_buf_pool_t *p_pool = (img_buf_pool_t *)handle;

  if (!p_pool) {
    IDBG_ERROR("%s:%d] Error buf pool not init", __func__, __LINE__);
    return IMG_ERR_INVALID_OPERATION;
  }

  QIMG_LOCK(&p_pool->lock);
  for (i = 0; i < MAX_BUF_NODE; i++) {
    p_node = &p_pool->node[i];
    if (release_buf && p_node->is_init) {
      img_buffer_release(&p_node->handle);
      p_node->is_init = FALSE;
    }
    if (p_node->ref_cnt) {
      IDBG_ERROR("%s:%d] Buffer %d not released %d",
        __func__, __LINE__,
        p_node->scaled_frame.frame_id, p_node->ref_cnt);
    }
    p_node->ref_cnt = 0;
  }
  p_pool->frame_info_valid = FALSE;

  QIMG_UNLOCK(&p_pool->lock);
  return rc;
}

/** img_buf_pool_set_frame_info
 *
 *  Arguments:
 *     @p_pool: buffer pool
 *     @p_frame: frame information of frame to be set
 *
 *  Return:
 *     Imaging errors
 *
 *  Description:
 *      API to set the frame information
 *
 **/
int32_t img_buf_pool_set_frame_info(img_buf_pool_t *p_pool,
  img_frame_t *p_frame)
{
  int32_t rc = IMG_SUCCESS;
  img_frame_t *p_s_frame;
  uint32_t out_stride;
  uint32_t i = 0;
  uint32_t out_width;
  uint32_t out_height;
  float ratio, asp_ratio;
  int32_t shift_factor;

  uint32_t width = QIMG_WIDTH(p_frame, 0);
  uint32_t height = QIMG_HEIGHT(p_frame, 0);

  if ((p_pool->params.min_dim > width) ||
    (p_pool->params.min_dim > height)) {
    IDBG_ERROR("%s:%d] Error cannot upscale", __func__, __LINE__);
    rc = IMG_ERR_INVALID_OPERATION;
    goto end;
  }

  asp_ratio = (float)width/(float)height;
  if (asp_ratio > 2.0) {
    IDBG_ERROR("%s:%d] invalid dim %dx%d ratio %f", __func__, __LINE__,
      width, height, asp_ratio);
    rc = IMG_ERR_INVALID_INPUT;
    goto end;
  }

  if (width >= height) {
    ratio = (float)p_pool->params.min_dim/(float)height;
    out_height = p_pool->params.min_dim;
    out_width = width * ratio;
  } else {
    ratio = (float)p_pool->params.min_dim/(float)width;
    out_width = p_pool->params.min_dim;
    out_height = height * ratio;
  }

  p_s_frame = &p_pool->frame_info;
  memset(p_s_frame, 0x0, sizeof(img_frame_t));
  p_s_frame->frame_cnt = 1;
  p_s_frame->info.width = out_width;
  p_s_frame->info.height = out_height;
  out_stride = QIMG_CEILINGN(out_width, 16);
  p_s_frame->frame[0].plane_cnt = p_frame->frame[0].plane_cnt;

  for (i = 0; i < p_frame->frame[0].plane_cnt; i++) {
    p_s_frame->frame[0].plane[i].offset = 0;
    shift_factor = (i == 0) ? 0 : 1;
    QIMG_WIDTH(p_s_frame, i) = p_s_frame->info.width;
    QIMG_HEIGHT(p_s_frame, i) = p_s_frame->info.height >> shift_factor;
    QIMG_STRIDE(p_s_frame, i) = out_stride;
    QIMG_SCANLINE(p_s_frame, i) = p_s_frame->info.height >> shift_factor;
    QIMG_LEN(p_s_frame, i) = out_stride * out_height >> shift_factor;
    QIMG_PL_TYPE(p_s_frame, i) = QIMG_PL_TYPE(p_frame, i);
  }

end:
  return rc;
}

/** img_buf_pool_release_frame
 *
 *  Arguments:
 *     @handle: buffer pool handle
 *     @id: unique id of the buffer. optional, pass -1 if the
 *        buffer need not be associated with the particular id.
 *     @frame_idx: frame_id of the frame to be released
 *     @debug_str: debug string of the client, optional
 *
 *  Return:
 *     Imaging errors
 *
 *  Description:
 *      API to get release the scaled frame.
 *
 **/
int32_t img_buf_pool_release_frame(void *handle, int32_t id, uint32_t frame_id,
  char *debug_str)
{
  int32_t rc = IMG_SUCCESS;
  img_buf_node_t *p_node = NULL;
  int i = 0;
  char *cl_str = (debug_str) ? debug_str : "";
  img_buf_pool_t *p_pool = (img_buf_pool_t *)handle;

  if (!p_pool) {
    IDBG_ERROR("%s:%d] Error buf pool not init %s",
      __func__, __LINE__, cl_str);
    return IMG_ERR_INVALID_OPERATION;
  }

  QIMG_LOCK(&p_pool->lock);
  /* check if the frame is already available */
  for (i = 0; i < MAX_BUF_NODE; i++) {
    p_node = &p_pool->node[i];
    if (p_node->ref_cnt &&
      ((id < 0) || (p_node->id == id)) &&
      (p_node->scaled_frame.frame_id == frame_id)) {
      /* found the node.*/
      p_node->ref_cnt--;
      break;
    }
  }

  if (IMG_SUCCEEDED(rc) && p_node) {
    if (p_node->ref_cnt <= 0) {
      IDBG_MED("%s:%d] Removed frame_id %d %s", __func__, __LINE__,
        p_node->scaled_frame.frame_id, cl_str);
    } else {
      IDBG_LOW("%s:%d] requested by client %s frame_id %d", __func__, __LINE__,
        cl_str, p_node->scaled_frame.frame_id);
    }
  }
  QIMG_UNLOCK(&p_pool->lock);
  return rc;
}

/** img_buf_pool_get_frame
 *
 *  Arguments:
 *     @handle: buffer pool handle
 *     @id: unique id of the buffer. optional, pass -1 if the
 *        buffer need not be associated with the particular id.
 *     @p_input: input frame to be scaled
 *     @p_output: output frame (scaled)
 *     @debug_str: debug string of the client, optional
 *
 *  Return:
 *     Imaging errors
 *
 *  Description:
 *      API to get scaled frame based on frame_id. if the buffer
 *      is already available, scaling operation wont be done
 *
 **/
int32_t img_buf_pool_get_frame(void *handle, int32_t id, img_frame_t *p_input,
  img_frame_t **p_output,
  char *debug_str)
{
  int32_t rc = IMG_SUCCESS;
  img_buf_node_t *p_node = NULL;
  img_frame_t *p_frame;
  int i = 0;
  int32_t alloc_idx = -1;
  int32_t set_idx = -1;
  img_buf_pool_t *p_pool = (img_buf_pool_t *)handle;
  uint8_t *p_src, *p_temp = NULL;
  char *cl_str = (debug_str) ? debug_str : "";

  if (!p_pool) {
    IDBG_ERROR("%s:%d] Error buf pool not init %s",
      __func__, __LINE__, cl_str);
    return IMG_ERR_INVALID_OPERATION;
  }

  QIMG_LOCK(&p_pool->lock);

  if (!p_pool->frame_info_valid) {
    rc = img_buf_pool_set_frame_info(p_pool, p_input);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("%s:%d] Error cannot set frame info %s",
        __func__, __LINE__, cl_str);
      goto end;
    }
    p_pool->frame_info_valid = TRUE;
  }
  /* check if the frame is already available */
  for (i = 0; i < MAX_BUF_NODE; i++) {
    p_node = &p_pool->node[i];
    if (p_node->is_init &&
      p_node->ref_cnt &&
      ((id < 0) || (p_node->id == id)) &&
      (p_node->scaled_frame.frame_id == p_input->frame_id)) {
      /* send the buffer */
      p_frame = &p_node->scaled_frame;
      p_node->id = id;
      IDBG_LOW("%s:%d] found frame_idx %d %s", __func__, __LINE__,
        p_input->frame_id, cl_str);
      rc = IMG_SUCCESS;
      goto end;
    } else if (p_node->is_init && !p_node->ref_cnt && set_idx < 0) {
      /* mark the node in case frame needs to be set in pre-allocated frame */
      set_idx = i;
    } else if (!p_node->is_init && (alloc_idx < 0)) {
      /* mark the node in case frame needs to be allocated */
      alloc_idx = i;
    }
  }

  /* get a frame which is already allocated */
  if (set_idx >= 0) {
    /* scale and store the frame */
    p_node = &p_pool->node[set_idx];
    IDBG_LOW("%s:%d] set frame_idx %d set_idx %d %s",
      __func__, __LINE__,
      p_input->frame_id, set_idx, cl_str);
  } else if (alloc_idx >= 0) { /* allocate and get frame */
    /* scale and store the frame */
    p_node = &p_pool->node[alloc_idx];
    rc = img_buf_pool_alloc(p_pool, alloc_idx);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("%s:%d] Error alloc failed %s",
        __func__, __LINE__, cl_str);
      goto end;
    }
    IDBG_LOW("%s:%d] allocate frame_idx %d alloc_idx %d %s",
      __func__, __LINE__,
      p_input->frame_id, alloc_idx, cl_str);
  } else {
    IDBG_ERROR("%s:%d] Error no frame available %s",
      __func__, __LINE__, cl_str);
    rc = IMG_ERR_NO_MEMORY;
    goto end;
  }
  p_frame = &p_node->scaled_frame;

  *p_frame = p_pool->frame_info;
  QIMG_ADDR(p_frame, 0) = p_node->handle.vaddr;
  QIMG_FD(p_frame, 0)  = p_node->handle.fd;

  /* deinterleave the frames if needed */
  if (QIMG_SINGLE_PLN_INTLVD(p_input)) {
    img_frame_t temp_frame;
    IDBG_INFO("%s:%d] Buffer is interleaved", __func__, __LINE__);
    p_temp = calloc(1, QIMG_LEN(p_input, 0));
    if (NULL == p_temp) {
      /* alloc failed */
      rc = IMG_ERR_NO_MEMORY;
      goto end;
    }

    memset(&temp_frame, 0x00, sizeof(img_frame_t));
    QIMG_ADDR(&temp_frame, 0) = p_temp;
    QIMG_LEN(&temp_frame, 0) = QIMG_WIDTH(p_input, 0) *
      QIMG_HEIGHT(p_input, 0);
    QIMG_ADDR(&temp_frame, 1) = p_temp + QIMG_LEN(&temp_frame, 0);
    rc = img_plane_deinterleave(QIMG_ADDR(p_input, 0),
      QIMG_PL_TYPE(p_input, 0), &temp_frame);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("%s:%d] Error interleave failed %d %s",
        __func__, __LINE__, rc, cl_str);
      goto end;
    }
    p_src = p_temp;
  } else {
    p_src = QIMG_ADDR(p_input, 0);
  }

#ifdef __CAM_FCV__
  fcvScaleDownMNu8(p_src,
    QIMG_WIDTH(p_input, 0),
    QIMG_HEIGHT(p_input, 0),
    QIMG_STRIDE(p_input, 0),
    QIMG_ADDR(p_frame, 0),
    QIMG_WIDTH(p_frame, 0),
    QIMG_HEIGHT(p_frame, 0),
    QIMG_STRIDE(p_frame, 0));
#else
  IDBG_ERROR("%s:%d] FastCV not present", __func__, __LINE__);
  goto end;
#endif

  p_frame->frame_id = p_input->frame_id;
  p_frame->idx = p_input->idx;

end:

  if (IMG_SUCCEEDED(rc) && p_node && p_frame) {
    p_node->ref_cnt++;
    *p_output = p_frame;
  }

  if (p_temp) {
    free(p_temp);
  }
  QIMG_UNLOCK(&p_pool->lock);
  return rc;
}

/** img_buf_pool_create
 *
 *  Arguments:
 *     @p_params: buffer ctl init parameters
 *
 *  Return:
 *     buffer pool handle
 *
 *  Description:
 *      Creates buffer pool.
 *
 **/
void *img_buf_pool_create(img_buf_params_t *p_params)
{
  img_buf_pool_t *p_pool = calloc(1, sizeof(img_buf_pool_t));

  if (!p_pool) {
    IDBG_ERROR("%s:%d] Error buf pool not init", __func__, __LINE__);
    return NULL;
  }

  pthread_mutex_init(&p_pool->lock, NULL);
  p_pool->params = *p_params;
  /* only upto 2.0 aspect ratio is supported*/
  p_pool->length = QIMG_CEILINGN(p_params->min_dim << 1, 16) *
    p_params->min_dim;
  img_buffer_open();
  return p_pool;
}

/** img_buf_pool_destroy
 *
 *  Arguments:
 *     @handle: buffer pool handle
 *
 *  Return:
 *     none
 *
 *  Description:
 *      Destroys the buffer pool
 *
 **/
void img_buf_pool_destroy(void *handle)
{
  img_buf_pool_t *p_pool = (img_buf_pool_t *)handle;
  if (p_pool) {
    img_buf_pool_reset(handle, TRUE);
    img_buffer_close();
    free(handle);
  }
}
