/*============================================================================

Copyright (c) 2013-2019 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/

#include <stdint.h>
#include <sys/time.h>
#include <linux/media.h>
#include <media/msmb_generic_buf_mgr.h>
#include <linux/msm_ion.h>
#if TARGET_ION_ABI_VERSION >= 2
/* For new ION include libion */
#include <ion/ion.h>
#include <linux/dma-buf.h>
#endif
#include "camera_dbg.h"
#include "pp_buf_mgr.h"

/* macros for unpacking identity */
#define BUF_MGR_GET_STREAM_ID(identity) ((identity) & 0xFFFF)
#define BUF_MGR_GET_SESSION_ID(identity) (((identity) & 0xFFFF0000) >> 16)

/** pp_buf_mgr_t: buffer manager structure
 *
 *  @fd: fd to communicate with buffer manager subdevice
 *  @mutex: mutex to ensure shared resources are locked for
 *        concurrency
 *  @ref_count: ref count for open and close
 *
 *  buffer manager struct that has resources specific to buffer
 *  manager client **/
typedef struct _pp_buf_mgr_t {
  int32_t         fd;
  pthread_mutex_t mutex;
  uint32_t        ref_count;
  int ion_fd;
} pp_buf_mgr_t;

/* Have only one instance of buffer manager*/
static pp_buf_mgr_t g_buf_mgr;

/** pp_buf_mgr_open_subdev: buffer manager subdevice open
 *
 *  @buf_mgr: buf mgr instance
 *
 *  find buffer manager subdev node and open it
 *
 *  Return: TRUE on SUCCESS
 *          FALSE on failure **/
static boolean pp_buf_mgr_open_subdev(pp_buf_mgr_t *buf_mgr)
{
  struct media_device_info mdev_info;
  int32_t num_media_devices = 0;
  char dev_name[32];
  char subdev_name[32];
  int32_t dev_fd = 0, ioctl_ret;
  boolean ret = FALSE;
  uint32_t i = 0;

  CDBG("%s:%d Enter\n", __func__, __LINE__);
  if (!buf_mgr) {
    CDBG_ERROR("%s:%d buf mgr NULL\n", __func__, __LINE__);
    return FALSE;
  }
  while (1) {
    int32_t num_entities = 0;
    snprintf(dev_name, sizeof(dev_name), "/dev/media%d", num_media_devices);
    dev_fd = open(dev_name, O_RDWR | O_NONBLOCK);
    if (dev_fd < 0) {
      CDBG("%s:%d Done enumerating media devices\n", __func__, __LINE__);
      break;
    }
    num_media_devices++;
    ioctl_ret = ioctl(dev_fd, MEDIA_IOC_DEVICE_INFO, &mdev_info);
    if (ioctl_ret < 0) {
      CDBG("%s:%d Done enumerating media devices\n", __func__, __LINE__);
      close(dev_fd);
      break;
    }

    if ((strncmp(mdev_info.model, "msm_config", sizeof(mdev_info.model)) != 0)) {
      close(dev_fd);
      continue;
    }
    while (1) {
      struct media_entity_desc entity;
      memset(&entity, 0, sizeof(entity));
      entity.id = num_entities | MEDIA_ENT_ID_FLAG_NEXT;
      CDBG("%s:%d entity id %d", __func__, __LINE__, entity.id);
      ioctl_ret = ioctl(dev_fd, MEDIA_IOC_ENUM_ENTITIES, &entity);
      if (ioctl_ret < 0) {
        CDBG("%s:%d Done enumerating media entities\n", __func__, __LINE__);
        break;
      }
      num_entities = entity.id;
      CDBG("%s:%d entity name %s type %d group id %d\n", __func__, __LINE__,
        entity.name, entity.type, entity.group_id);
      if (mct_util_match_subdev_type(entity, MSM_CAMERA_SUBDEV_BUF_MNGR,
        MEDIA_ENT_T_V4L2_SUBDEV)) {
        snprintf(subdev_name, sizeof(dev_name), "/dev/%s", entity.name);
        CDBG("%s: subdev_name:%s\n", __func__, subdev_name);
        buf_mgr->fd = open(subdev_name, O_RDWR);
        if (buf_mgr->fd < 0) {
          CDBG("%s:Open subdev failed\n", __func__);
          continue;
        }
        CDBG("%s:Open subdev Success\n", __func__);
        ret = TRUE;
        break;
      }
    }
    close(dev_fd);
  }
  return ret;
}

/** pp_buf_mgr_match_buf_index: buffer manager subdevice open
 *
 *  @data1: fd to communicate with buffer manager subdevice
 *  @data2: mutex to ensure shared resources are locked for
 *        concurrency
 *
 *  buffer manager struct that has resources specific to buffer
 *  manager client **/
static boolean pp_buf_mgr_match_buf_index(void *data1, void *data2)
{
  mct_stream_map_buf_t *list_buf = (mct_stream_map_buf_t *)data1;
  uint32_t *user_buf_index = (uint32_t *)data2;

  /* Validate input parameters */
  if (!list_buf || !user_buf_index) {
    CDBG_ERROR("%s:%d failed: invalid input params\n", __func__, __LINE__);
    goto ERROR;
  }

  /* Match list buffer index with user buffer index */
  if (list_buf->buf_index == *user_buf_index) {
    return TRUE;
  }

ERROR:
  return FALSE;
}

/** pp_buf_mgr_open: buffer manager open
 *
 *  On first call, create buffer manager instance, call function
 *  to open buffer manager subdevice and increment ref count.
 *
 *  Return: buffer manager instance on SUCCESS
 *          NULL on failure **/
void *pp_buf_mgr_open(void)
{
  boolean rc = TRUE;

  /* Open subdev if this is the first call */
  if (!g_buf_mgr.ref_count) {
#ifndef TARGET_ION_ABI_VERSION
    g_buf_mgr.ion_fd = open("/dev/ion", O_RDONLY | O_SYNC);
#else
    g_buf_mgr.ion_fd = ion_open();
#endif

    if (g_buf_mgr.ion_fd < 0) {
      CDBG_ERROR("failed: to open ion fd");
      goto ERROR;
    }

    rc = pp_buf_mgr_open_subdev(&g_buf_mgr);
    if (rc == FALSE || g_buf_mgr.fd < 0) {
      CDBG_ERROR("failed: to open subdev rc %d fd %d \n",
        rc, g_buf_mgr.fd);
      close(g_buf_mgr.ion_fd);
      goto ERROR;
    }
  }

  /* Increment ref count */
  g_buf_mgr.ref_count++;
  return &g_buf_mgr;
ERROR:
  return NULL;
}

/** pp_buf_mgr_close: buffer manager close
 *
 *  @buf_mgr: buf mgr instance
 *
 *  Decrement buffer manager ref count. If ref count is zero,
 *  close sub device and free memory
 *
 *  Return: TRUE on SUCCESS
 *          FALSE on failure **/
boolean pp_buf_mgr_close(void *v_buf_mgr)
{
  pp_buf_mgr_t *buf_mgr = (pp_buf_mgr_t *)v_buf_mgr;

  /* Validate input parameters */
  if (!buf_mgr || !buf_mgr->ref_count) {
    CDBG_ERROR("%s:%d invalid params\n", __func__, __LINE__);
    goto ERROR;
  }

  /* Decrement refcount and close fd if ref count is 0 */
  if (--buf_mgr->ref_count == 0) {
    close(buf_mgr->fd);
    buf_mgr->fd = 0;
  }

  if (buf_mgr->ion_fd >= 0) {
#ifndef TARGET_ION_ABI_VERSION
    close(buf_mgr->ion_fd);
#else
    ion_close(buf_mgr->ion_fd);
#endif
    buf_mgr->ion_fd = -1;
  }

  return TRUE;
ERROR:
  return FALSE;
}

/** pp_buf_mgr_get_buf: get buf API to acquire buffer
 *
 *  @v_buf_mgr: buf mgr instance
 *  @stream_info: stream info handle
 *
 *  Get buffer from kernel buffer manager and match its index
 *  from stream info's img buf list to extract other params like
 *  fd, vaddr
 *
 *  Return: SUCCESS - stream map struct representing buffer
 *          FAILURE - NULL **/
mct_stream_map_buf_t *pp_buf_mgr_get_buf(void *v_buf_mgr,
  mct_stream_info_t *stream_info)
{
  int32_t                   ret = 0;
  pp_buf_mgr_t             *buf_mgr = (pp_buf_mgr_t *)v_buf_mgr;
  struct msm_buf_mngr_info  buffer;
  mct_list_t               *img_buf_list = NULL;

  /* Validate input parameters */
  if (!buf_mgr || !stream_info) {
    CDBG_ERROR("%s:%d invalid params\n", __func__, __LINE__);
    goto ERROR;
  }

  /* Validate buffer manager parameters */
  if (buf_mgr->fd < 0 || !buf_mgr->ref_count) {
    CDBG_ERROR("%s:%d invalid buf mgr params\n", __func__, __LINE__);
    goto ERROR;
  }

  /* Validate stream info parameters */
  if (!stream_info->img_buffer_list) {
    CDBG_ERROR("%s:%d invalid steam info params\n", __func__, __LINE__);
    goto ERROR;
  }

  /* Get buffer from buf mgr node */
  buffer.session_id = BUF_MGR_GET_SESSION_ID(stream_info->identity);
  buffer.stream_id = BUF_MGR_GET_STREAM_ID(stream_info->identity);
  buffer.type = MSM_CAMERA_BUF_MNGR_BUF_PLANAR;
  ret = ioctl(buf_mgr->fd, VIDIOC_MSM_BUF_MNGR_GET_BUF, &buffer);
  if (ret < 0) {
    CDBG("%s:%d failed: to get buf from buf mgr", __func__, __LINE__);
    goto ERROR;
  }

  /* Match vb2 buffer index with stream info buf list */
  img_buf_list = mct_list_find_custom(stream_info->img_buffer_list,
    &buffer.index, pp_buf_mgr_match_buf_index);
  if (!img_buf_list || !img_buf_list->data) {
    CDBG_ERROR("%s:%d failed: to match kernel buf index with stream buf list\n",
      __func__, __LINE__);
    goto ERROR;
  }

  return img_buf_list->data;
ERROR:
  return NULL;
}

mct_stream_map_buf_t* pp_buf_mgr_get_buf_idx(void *v_buf_mgr,
  uint32_t identity,mct_list_t *list, uint32_t idx)
{
  int32_t                   ret = 0;
  pp_buf_mgr_t             *buf_mgr = (pp_buf_mgr_t *)v_buf_mgr;
  struct msm_buf_mngr_info  buffer;
  mct_list_t               *img_buf_list = NULL;
  struct msm_camera_private_ioctl_arg priv_arg;


  /* Validate input parameters */
  if (!buf_mgr || !list) {
    CDBG_ERROR("%s:%d invalid params %p %p\n", __func__, __LINE__,buf_mgr,list);
    goto ERROR;
  }

  /* Validate buffer manager parameters */
  if (buf_mgr->fd < 0 || !buf_mgr->ref_count) {
    CDBG_ERROR("%s:%d invalid buf mgr params\n", __func__, __LINE__);
    goto ERROR;
  }

  /* Get buffer from buf mgr node */
  buffer.session_id = BUF_MGR_GET_SESSION_ID(identity);
  buffer.stream_id = BUF_MGR_GET_STREAM_ID(identity);
  buffer.type = MSM_CAMERA_BUF_MNGR_BUF_PLANAR;
  buffer.index = idx;

  priv_arg.id = MSM_CAMERA_BUF_MNGR_IOCTL_ID_GET_BUF_BY_IDX;
  priv_arg.size = sizeof(struct msm_buf_mngr_info);
  priv_arg.result = 0;
  priv_arg.reserved = 0x0;
  priv_arg.ioctl_ptr = (unsigned long long)&buffer;

  CDBG("%s:%d bfr index %d", __func__, __LINE__,buffer.index);
  ret = ioctl(buf_mgr->fd,  VIDIOC_MSM_BUF_MNGR_IOCTL_CMD, &priv_arg);
  if (ret < 0) {
    CDBG_ERROR("%s:%d failed: to get buf from buf mgr", __func__, __LINE__);
    goto ERROR;
  }

  /* Match vb2 buffer index with stream info buf list */
  img_buf_list = mct_list_find_custom(list,
    &buffer.index, pp_buf_mgr_match_buf_index);
  if (!img_buf_list || !img_buf_list->data) {
    CDBG_ERROR("%s:%d failed: to match kernel buf index with stream buf list\n",
    __func__, __LINE__);
   goto ERROR;
  }

  return img_buf_list->data;

ERROR:
  return NULL;
}

/** pp_buf_mgr_put_buf: put buf API to release buffer to buf mgr
 *  without doing buf done on HAL
 *
 *  @v_buf_mgr: buf mgr instance
 *  @stream_info: stream info handle
 *
 *  Invoke put buf on buffer manager node
 *
 *  Return: SUCCESS - TRUE
 *          FAILURE - FALSE **/
boolean pp_buf_mgr_put_buf(void *v_buf_mgr, uint32_t identity,
  uint32_t buff_idx, uint32_t frameid, struct timeval timestamp)
{
  int                       ret = 0;
  pp_buf_mgr_t             *buf_mgr = (pp_buf_mgr_t *)v_buf_mgr;
  struct msm_buf_mngr_info  buff;

  /* Validate input parameters */
  if (!buf_mgr) {
    CDBG_ERROR("%s:%d invalid params\n", __func__, __LINE__);
    goto ERROR;
  }

  /* Validate buffer manager parameters */
  if (buf_mgr->fd < 0 || !buf_mgr->ref_count) {
    CDBG_ERROR("%s:%d invalid buf mgr params\n", __func__, __LINE__);
    goto ERROR;
  }

  /* Perform ioctl for put buf on buf mgr node */
  buff.index = buff_idx;
  buff.session_id = BUF_MGR_GET_SESSION_ID(identity);
  buff.stream_id = BUF_MGR_GET_STREAM_ID(identity);
  buff.frame_id = frameid;
  buff.timestamp = timestamp;
  ret = ioctl(buf_mgr->fd, VIDIOC_MSM_BUF_MNGR_PUT_BUF, &buff);
  if (ret < 0) {
    CDBG_ERROR("%s:%d failed: to put buf on kernel buf mgr node", __func__,
      __LINE__);
    goto ERROR;
  }
  return TRUE;

ERROR:
  return FALSE;
}

/** pp_buf_mgr_buf_done: buf done API to release buffer to buf
 *  mgr + buf done on HAL
 *
 *  @v_buf_mgr: buf mgr instance
 *  @stream_info: stream info handle
 *
 *  Invoke buf done on buffer manager node
 *
 *  Return: SUCCESS - TRUE
 *          FAILURE - FALSE **/
boolean pp_buf_mgr_buf_done(void *v_buf_mgr, uint32_t identity,
  uint32_t buff_idx, uint32_t frameid, struct timeval timestamp)
{
  int                       ret = 0;
  pp_buf_mgr_t             *buf_mgr = (pp_buf_mgr_t *)v_buf_mgr;
  struct msm_buf_mngr_info  buff;

  /* Validate input parameters */
  if (!buf_mgr) {
    CDBG_ERROR("%s:%d invalid params\n", __func__, __LINE__);
    goto ERROR;
  }

  /* Validate buffer manager parameters */
  if (buf_mgr->fd < 0 || !buf_mgr->ref_count) {
    CDBG_ERROR("%s:%d invalid buf mgr params\n", __func__, __LINE__);
    goto ERROR;
  }

  /* Perform ioctl for buf done on buf mgr node */
  buff.index = buff_idx;
  buff.session_id = BUF_MGR_GET_SESSION_ID(identity);
  buff.stream_id = BUF_MGR_GET_STREAM_ID(identity);
  buff.frame_id = frameid;
  buff.timestamp = timestamp;
  ret = ioctl(buf_mgr->fd, VIDIOC_MSM_BUF_MNGR_BUF_DONE, &buff);
  if (ret < 0) {
    CDBG_ERROR("%s:%d failed: to buf done on kernel buf mgr node", __func__,
      __LINE__);
    goto ERROR;
  }
  return TRUE;

ERROR:
  return FALSE;
}

/** pp_buf_mgr_get_buf_info: get buf info from buffer index
 *  and associated stream info
 *
 *  @v_buf_mgr: [INPUT] buf mgr instance
 *  @buff_idx: [INPUT] buf index
 *  @stream_info: [INPUT] stream info handle
 *  @buf_info: [OUTPUT] buf info
 *
 *  Get buf info from buffer index and associated stream
 *  info
 *
 *  Return: SUCCESS - TRUE
 *          FAILURE - FALSE **/
mct_stream_map_buf_t* pp_buf_mgr_get_buf_info(void *v_buf_mgr, uint32_t buff_idx,
  mct_stream_info_t *stream_info)
{
  boolean              rc = TRUE;
  pp_buf_mgr_t         *buf_mgr = (pp_buf_mgr_t *)v_buf_mgr;
  mct_list_t           *img_buf_list = NULL;
  mct_stream_map_buf_t *img_buf = NULL;

  /* Validate input parameters */
  if (!buf_mgr || !stream_info) {
    CDBG_ERROR("invalid params\n");
    goto ERROR;
  }

  /* Validate buffer manager parameters */
  if (buf_mgr->fd < 0 || !buf_mgr->ref_count) {
    CDBG_ERROR("invalid buf mgr params\n");
    goto ERROR;
  }

  /* Validate stream info parameters */
  if (!stream_info->img_buffer_list) {
    CDBG_ERROR("invalid steam info params\n");
    goto ERROR;
  }

  /* Match vb2 buffer index with stream info buf list */
  img_buf_list = mct_list_find_custom(stream_info->img_buffer_list,
    &buff_idx, pp_buf_mgr_match_buf_index);
  if (!img_buf_list || !img_buf_list->data) {
    CDBG_ERROR("failed: to match kernel buf index with stream buf list\n");
    goto ERROR;
  }

  img_buf = (mct_stream_map_buf_t *)img_buf_list->data;

  return img_buf;
ERROR:
  return NULL;
}

/** pp_buf_mgr_get_vaddr: get virtual address from buffer index
 *  and associated stream info
 *
 *  @v_buf_mgr: [INPUT] buf mgr instance
 *  @buff_idx: [INPUT] buf index
 *  @stream_info: [INPUT] stream info handle
 *  @vaddr: [OUTPUT] address of virtual address
 *
 *  Get virtual address from buffer index and associated stream
 *  info
 *
 *  Return: SUCCESS - TRUE
 *          FAILURE - FALSE **/
boolean pp_buf_mgr_get_vaddr(void *v_buf_mgr, uint32_t buff_idx,
  mct_stream_info_t *stream_info, void **vaddr)
{
  mct_stream_map_buf_t *img_buf =
    pp_buf_mgr_get_buf_info(v_buf_mgr, buff_idx, stream_info);

  if (img_buf) {
    /* Fill virtual address */
    *vaddr = img_buf->buf_planes[0].buf;
    return TRUE;
  } else {
    return FALSE;
  }
}

/** pp_buf_mgr_cacheops: Do cache ops as requested
 *
 *  @v_buf_mgr: [INPUT] buf mgr instance
 *  @vaddr: [INPUT] buffer address
 *  @fd: [INPUT] buffer fd
 *  @len: [INPUT] buffer length
 *  @cache_ops: [INPUT] cache operation that needs to be done
 *
 *  Do the cache operation as requested by client
 *
 *  Return: SUCCESS - TRUE
 *          FAILURE - FALSE **/
int32_t pp_buf_mgr_cacheops(void *v_buf_mgr, void *vaddr,
   int fd, int len, pp_buf_mgr__cache_ops_type cache_ops)
{
  int rc = 0;
#ifndef TARGET_ION_ABI_VERSION
  struct ion_flush_data cache_inv_data;
  struct ion_custom_data custom_data;
  struct ion_fd_data fd_data;
  struct ion_handle_data handle_data;

  uint32_t cmd = ION_IOC_CLEAN_INV_CACHES;
  pp_buf_mgr_t *buf_mgr = (pp_buf_mgr_t *)v_buf_mgr;

  CDBG("%s: vaddr %p, fd %d,  len %d, pp_buf_mgr %p ", __func__,
    vaddr, fd, len, buf_mgr);

  memset (&fd_data, 0x0, sizeof(struct ion_fd_data));
  memset (&cache_inv_data, 0x0, sizeof(struct ion_flush_data));
  memset (&custom_data, 0x0, sizeof(struct ion_custom_data));
  memset (&handle_data, 0x0, sizeof(struct ion_handle_data));

  if (fd < 0) {
    CDBG_ERROR("Invalid ION fd %d", fd);
    return -1;
  }
  if (cache_ops == PP_BUF_MGR_CACHE_NO_OP) {
    CDBG_HIGH("NO Cache Op");
    return 0;
  }

  if (NULL == vaddr) {
    CDBG_ERROR("Buffer is null");
    return -1;
  }

  fd_data.fd = fd;
  rc = ioctl (buf_mgr->ion_fd, ION_IOC_IMPORT, fd_data);
  if (rc < 0) {
    fd_data.handle = 0;
  }

  switch (cache_ops) {
  case PP_BUF_MGR_CACHE_INVALIDATE:
    cmd = ION_IOC_INV_CACHES;
    break;
  case PP_BUF_MGR_CACHE_CLEAN:
    cmd = ION_IOC_CLEAN_CACHES;
    break;
  default:
  case PP_BUF_MGR_CACHE_CLEAN_INVALIDATE:
    cmd = ION_IOC_CLEAN_INV_CACHES;
    break;
  }

  CDBG_HIGH("CAMCACHE : CacheOp : "
    "Buffer(vaddr=%p, fd=%d, size=%d, offset=%d), "
    "type=%s, ion_device_fd=%d",
    vaddr, fd_data.fd, len, 0,
    (cache_ops == PP_BUF_MGR_CACHE_INVALIDATE) ? "INV" :
    (cache_ops == PP_BUF_MGR_CACHE_CLEAN) ? "CLEAN" : "CLEAN_INV",
    buf_mgr->ion_fd);

  cache_inv_data.vaddr = vaddr;
  cache_inv_data.fd =  fd_data.fd;
  cache_inv_data.handle = fd_data.handle;
  cache_inv_data.length =  (unsigned int)len;
  cache_inv_data.offset =  0;

  custom_data.cmd =  (unsigned int)cmd;
  custom_data.arg = (unsigned long)&cache_inv_data;
  handle_data.handle = fd_data.handle;

  rc = ioctl (buf_mgr->ion_fd, ION_IOC_CUSTOM, &custom_data);
  if (rc < 0) {
      CDBG_ERROR("%s: ION_IOC_CUSTOM error %d", __func__, rc);
      goto end;
  }

  CDBG("%s: ### ION_IOC_CUSTOM return success %d", __func__, rc);

end:
  if (fd_data.handle != 0) {
    ioctl(buf_mgr->ion_fd, ION_IOC_FREE, &handle_data);
  }
#else
  CDBG("%s: vaddr %p, fd %d, len %d", __func__,
    vaddr, fd, len);
  (void)v_buf_mgr;

  struct dma_buf_sync buf_sync_start;
  struct dma_buf_sync buf_sync_end;

  /* ION_IOC_CLEAN_CACHES-->call the DMA_BUF_IOCTL_SYNC IOCTL with flags
     DMA_BUF_SYNC_START and DMA_BUF_SYNC_WRITE and then call
     DMA_BUF_IOCTL_SYNC IOCTL with flags DMA_BUF_SYNC_END and
     DMA_BUF_SYNC_WRITE

     ION_IOC_INV_CACHES-->call the DMA_BUF_IOCTL_SYNC IOCT with flags
     DMA_BUF_SYNC_START and DMA_BUF_SYNC_WRITE and then call the
     DMA_BUF_IOCTL_SYNC IOCT with flags DMA_BUF_SYNC_END
     and DMA_BUF_SYNC_READ
  */
    switch (cache_ops) {
    case PP_BUF_MGR_CACHE_INVALIDATE:
      buf_sync_start.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_WRITE;
      buf_sync_end.flags   = DMA_BUF_SYNC_END   | DMA_BUF_SYNC_READ;
      break;
    case PP_BUF_MGR_CACHE_CLEAN:
      buf_sync_start.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_WRITE;
      buf_sync_end.flags   = DMA_BUF_SYNC_END   | DMA_BUF_SYNC_WRITE;
      break;
    default:
    case PP_BUF_MGR_CACHE_CLEAN_INVALIDATE:
      buf_sync_start.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_RW;
      buf_sync_end.flags   = DMA_BUF_SYNC_END   | DMA_BUF_SYNC_RW;
      break;
   }

   rc = ioctl(fd, DMA_BUF_IOCTL_SYNC, &buf_sync_start.flags);
   if (rc) {
      CDBG_ERROR("Failed first DMA_BUF_IOCTL_SYNC start\n");
   }

   rc = ioctl(fd, DMA_BUF_IOCTL_SYNC, &buf_sync_end.flags);
   if (rc) {
      CDBG_ERROR("Failed first DMA_BUF_IOCTL_SYNC end\n");
   }
#endif

  return rc;
}


/*
 * PP manager for native buffers. It allocates native buffers per request.
 * It creates a queue for every stream, that uses native buffers. Once created
 * buffers can be "get" or "put".
 */

/** pp_do_mmap_ion:
 *
 *  @ion_fd - File descriptor for ION device.
 *  @alloc - Pointer to the structure that holds data necessary
 *            for buffer allocation,
 *  @ion_info_fd - Pointer to structure that is used when buffer is
 *            set for sharing.
 *  @mapFd - Pointer to the variable that keeps file descriptor of the
 *            new buffer.
 *     This function sends commands to ION device to allocate new buffer.
 *  width lenght specified in alloc structure. On success it commands
 *  ION device to set this buffers as shared and then maps it to the virtual
 *  memory.
 *
 *  Return: Virtual address of the allocated buffer on success.
 *          on failure it returns 0
 **/
static uint8_t *pp_do_mmap_ion(int ion_fd, struct ion_allocation_data *alloc,
  struct ion_fd_data *ion_info_fd, int *mapFd)
{
  void *ret; /* returned virtual address */
  int rc = 0;

  /* to make it page size aligned */
  alloc->len = (alloc->len + 4095) & (~4095);
#ifndef TARGET_ION_ABI_VERSION
  struct ion_handle_data handle_data;
  rc = ioctl(ion_fd, ION_IOC_ALLOC, alloc);
  if (rc < 0) {
    CDBG_ERROR("%s: ION allocation failed %d\n", __func__,rc);
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
  if (rc) {
    CDBG_ERROR("ION map failed %s\n", strerror(errno));
    goto ION_MAP_FAILED;
  }
  ion_info_fd->fd = *mapFd;
#endif

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

  return ret;

ION_MAP_FAILED:
#ifndef TARGET_ION_ABI_VERSION
  handle_data.handle = ion_info_fd->handle;
  ioctl(ion_fd, ION_IOC_FREE, &handle_data);
#else
  /*TODO: need to check correct usage*/
  if (*mapFd > 0)
    close(*mapFd);
#endif
ION_ALLOC_FAILED:
  return NULL;
}

/** pp_do_munmap_ion:
 *
 *  @ion_fd - File descriptor for ION device.
 *  @ion_info_fd - Pointer to structure that is used when buffer is
 *            set for sharing.
 *  @addr - Pointer to the virtual addres of the buffer which have to be
 *            freed.
 *  @size - The size of the bufer which jave to be freed.
 *
 *     This function unmaps given buffer form virtual memory and sends command
 *  to ION device to free it.
 *
 *  Return: returns 0 on success.
 **/
static int pp_do_munmap_ion (int ion_fd, struct ion_fd_data *ion_info_fd,
  void *addr, size_t size)
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

/** pp_native_buf_mgr_init:
 *
 *  @native_buf_mgr - pointer to the structure that holds all operational data
 *                    of native buffer manager.
 *  @client_id - unique id for the buffer manger.
 *
 *     This function initializes the variables in the native buffer manager
 *   structure. It also opens ION device.
 *
 *  Return: returns TRUE on success.
 **/
boolean pp_native_buf_mgr_init(pp_native_buf_mgr_t *native_buf_mgr,
  uint32_t client_id)
{
  uint32_t i;

  if (native_buf_mgr->use_cnt++)
    return TRUE;

  memset(native_buf_mgr, 0, sizeof(pp_native_buf_mgr_t));
  memset(&native_buf_mgr->bufq, 0, sizeof(pp_bufq_t) * PP_MAX_NUM_BUF_QUEUE);

  pthread_mutex_init(&native_buf_mgr->mutex, NULL);
  pthread_mutex_init(&native_buf_mgr->req_mutex, NULL);

  for (i = 0; i < PP_MAX_NUM_BUF_QUEUE; i++)
    pthread_mutex_init(&native_buf_mgr->bufq[i].mutex, NULL);

  pthread_mutex_lock(&native_buf_mgr->mutex);
#ifndef TARGET_ION_ABI_VERSION
  native_buf_mgr->ion_fd = open("/dev/ion", O_RDONLY | O_SYNC);
#else
  native_buf_mgr->ion_fd = ion_open();
#endif
  if (native_buf_mgr->ion_fd < 0) {
    CDBG_ERROR("%s:%d Fail to open ION device", __func__, __LINE__);
    pthread_mutex_unlock(&native_buf_mgr->mutex);
    return FALSE;
  }
  pthread_mutex_unlock(&native_buf_mgr->mutex);

  native_buf_mgr->client_id = client_id;

  return TRUE;

}

/** pp_native_buf_mgr_deinit:
 *
 *  @native_buf_mgr - pointer to the structure that holds all operational data
 *                    of native buffer manager.
 *
 *     This function frees all the variables in the native buffer manager
 *   structure. It also closes ION device.
 *
 *  Return: It is void function.
 **/
void pp_native_buf_mgr_deinit(pp_native_buf_mgr_t *native_buf_mgr)
{
  uint32_t i;

  if (--native_buf_mgr->use_cnt)
    return;

  pthread_mutex_lock(&native_buf_mgr->mutex);
  if (native_buf_mgr->ion_fd) {
#ifndef TARGET_ION_ABI_VERSION
    close(native_buf_mgr->ion_fd);
#else
    ion_close(native_buf_mgr->ion_fd);
#endif
    native_buf_mgr->ion_fd = 0;
  }
  pthread_mutex_unlock(&native_buf_mgr->mutex);

  for (i = 0; i < PP_MAX_NUM_BUF_QUEUE; i++)
    pthread_mutex_destroy(&native_buf_mgr->bufq[i].mutex);

  pthread_mutex_destroy(&native_buf_mgr->req_mutex);
  pthread_mutex_destroy(&native_buf_mgr->mutex);
}


/** pp_init_native_buffer:
 *
 *  @img_buf - Pointer to the structure that holds all parameters of the
 *             buffer that will be allocated.
 *  @ion_fd - File descriptor for ION device.
 *  @idx - Index of the new buffer.
 *  @len_offset - Structure that holds the required length and offset
 *             of the frame.
 *  @cached - Flag that indicates whether the buffer is cache-able or not.
 *
 *     This function initialize all necessary fields in alloc structure and
 *  calls function pp_do_mmap_ion() to allocate and map new buffer. On success
 *  it populates buffer structure with the buffer parameters and address.
 *
 *  Return: Returns 0 on success.
 **/
int32_t pp_init_native_buffer(pp_frame_buffer_t *img_buf, int ion_fd,
  uint32_t idx, cam_frame_len_offset_t *len_offset, uint32_t cached)
{
  int current_fd = -1;
  uint32_t i;
  unsigned long current_addr = 0;

  memset(img_buf, 0, sizeof(pp_frame_buffer_t));
  img_buf->buffer.m.planes = &img_buf->planes[0];

  /* now we only use contigous memory.
   * We could use per plane memory */
  img_buf->ion_alloc[0].len = len_offset->frame_len;
  if (cached) {
    img_buf->ion_alloc[0].flags = ION_FLAG_CACHED;
  } else {
    img_buf->ion_alloc[0].flags = 0;
  }

  img_buf->ion_alloc[0].heap_id_mask = ION_HEAP(ION_SYSTEM_HEAP_ID);
  img_buf->ion_alloc[0].align = 4096;
  current_addr = (unsigned long) pp_do_mmap_ion(ion_fd,
    &(img_buf->ion_alloc[0]), &(img_buf->fd_data[0]), &current_fd);
  if (current_addr == 0) {
    CDBG_ERROR("%s: ION allocation no mem\n", __func__);
    return -1;
  }
  img_buf->vaddr = (void *)current_addr;

  for (i = 0; i < len_offset->num_planes; i++) {
    img_buf->buffer.m.planes[i].m.userptr = current_fd;
    img_buf->buffer.m.planes[i].data_offset = len_offset->mp[i].offset;
    img_buf->buffer.m.planes[i].length = len_offset->mp[i].len;
    img_buf->addr[0] = current_addr;
  }

  img_buf->buffer.length = len_offset->num_planes;
  img_buf->buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  img_buf->buffer.index = idx;
  img_buf->buffer.memory = V4L2_MEMORY_USERPTR;
  img_buf->fd = current_fd;
  img_buf->cached = cached;

  img_buf->busy = 0;

  return 0;
}


/** pp_deinit_native_buf:
 *
 *  @img_buf - Pointer to the structure that holds all parameters of the
 *             buffer that will be allocated.
 *  @ion_fd - File descriptor for ION device.
 *
 *     This function calls pp_do_munmap_ion() to unmap and free the required
 *   buffer.
 *
 *  Return: Returns 0 on success.
 **/
int32_t pp_deinit_native_buf(pp_frame_buffer_t *img_buf, int ion_fd)
{
  uint32_t i;

  if (img_buf->fd <= 0)
    return -EINVAL;

  for (i = 0; i < img_buf->buffer.length; i++) {
    if (img_buf->ion_alloc[i].len != 0) {
      pp_do_munmap_ion(ion_fd, &(img_buf->fd_data[0]),
        (void *)img_buf->addr[i], img_buf->ion_alloc[i].len);
    }
  }
  memset(img_buf, 0, sizeof(pp_frame_buffer_t));

  return 0;
}


/** pp_native_buf_mgr_find_queue:
 *
 *  @native_buf_mgr - Native buffer manager structure holding all data.
 *  @session_id - Current session id.
 *  @stream_id - Current stream id.
 *
 *     This function seeks buffer queue which is defined for the current
 *   session and stream ids. If it doesn't find such. takes the first free
 *   queue and initialize it to hold the buffers for this session and stream id.
 *
 *  Return: Pointer to the buffer queue that was found on success. On failure
 *        it returns NULL.
 **/
pp_bufq_t *pp_native_buf_mgr_find_queue(pp_native_buf_mgr_t *native_buf_mgr,
  uint32_t session_id, uint32_t stream_id)
{
  uint32_t i;

  for (i = 0; i < PP_MAX_NUM_BUF_QUEUE; i++) {
    if (native_buf_mgr->bufq[i].used &&
        (native_buf_mgr->bufq[i].session_id == session_id) &&
        (native_buf_mgr->bufq[i].stream_id == stream_id)) {
      break;
    }
  }

  if (i < PP_MAX_NUM_BUF_QUEUE)
    return (&native_buf_mgr->bufq[i]);

  for (i = 0; i < PP_MAX_NUM_BUF_QUEUE; i++) {
    pthread_mutex_lock(&native_buf_mgr->bufq[i].mutex);
    if (!native_buf_mgr->bufq[i].used) {
      native_buf_mgr->bufq[i].used = 1;
      native_buf_mgr->bufq[i].session_id = session_id;
      native_buf_mgr->bufq[i].stream_id = stream_id;
      native_buf_mgr->bufq[i].current_buf_count = 0;
      native_buf_mgr->bufq[i].current_getbuf_index = 0;
      pthread_mutex_unlock(&native_buf_mgr->bufq[i].mutex);

      return (&native_buf_mgr->bufq[i]);
    }
    pthread_mutex_unlock(&native_buf_mgr->bufq[i].mutex);
  }

  return NULL;
}


/** pp_native_buf_mgr_free_queue:
 *
 *  @native_buf_mgr - Native buffer manager structure holding all data.
 *  @session_id - Current session id.
 *  @stream_id - Current stream id.
 *
 *     This function seeks buffer queue which is defined for the current
 *   session and stream ids. If it finds such, frees all buffers in it and
 *   releases the queue.
 *
 *  Return: It is a void function.
 **/
void pp_native_buf_mgr_free_queue(pp_native_buf_mgr_t *native_buf_mgr,
  uint32_t session_id, uint32_t stream_id)
{
  uint32_t i, j;

  for (i = 0; i < PP_MAX_NUM_BUF_QUEUE; i++) {
    pthread_mutex_lock(&native_buf_mgr->bufq[i].mutex);
    if (native_buf_mgr->bufq[i].used &&
        (native_buf_mgr->bufq[i].session_id == session_id) &&
        (native_buf_mgr->bufq[i].stream_id == stream_id)) {
      native_buf_mgr->bufq[i].used = 0;
      native_buf_mgr->bufq[i].session_id = 0;
      native_buf_mgr->bufq[i].stream_id = 0;
      if (native_buf_mgr->bufq[i].current_buf_count) {
        for (j = 0; j < native_buf_mgr->bufq[i].current_buf_count; j++) {
          pp_deinit_native_buf(&native_buf_mgr->bufq[i].image_buf[j],
            native_buf_mgr->ion_fd);
        }
        native_buf_mgr->bufq[i].current_buf_count = 0;
      }
      pthread_mutex_unlock(&native_buf_mgr->bufq[i].mutex);
      break;
    }
    pthread_mutex_unlock(&native_buf_mgr->bufq[i].mutex);
  }
}


/** pp_native_buf_mgr_allocate_buff:
 *
 *  @native_buf_mgr - Native buffer manager structure holding all data.
 *  @buff_count - number of the buffers to be allocated.
 *  @session_id - Current session id.
 *  @stream_id - Current stream id.
 *  @len_offset - Structure that holds the required length and offset
 *             of the frame.
 *  @cached - Flag that indicates whether the buffer is cache-able or not.
 *
 *     This function checks whether the required number of buffers can be
 *   allocated. If it is possible it searches for appropriate buffer queue
 *   using function  pp_native_buf_mgr_find_queue(). On success it calls
 *   function pp_init_native_buffer() to allocate required number of buffers.
 *   and save it in the queue.
 *
 *  Return: It returns 0 on success.
 **/
int32_t pp_native_buf_mgr_allocate_buff(pp_native_buf_mgr_t *native_buf_mgr,
  uint32_t buff_count, uint32_t session_id, uint32_t stream_id,
  cam_frame_len_offset_t *len_offset,uint32_t cached)
{
  pp_bufq_t *buff_q;
  uint32_t i;
  int32_t rc;
  uint32_t buff_idx;

  buff_q = pp_native_buf_mgr_find_queue(native_buf_mgr,session_id,stream_id);

  if(buff_q == NULL) {
    CDBG_ERROR("%s:%d: There is no free queue",__func__,__LINE__);
    return -ENOMEM;
  }

  pthread_mutex_lock(&buff_q->mutex);
  if ((buff_q->current_buf_count + buff_count) > PP_MAX_BUF_COUNT) {
    CDBG_ERROR("%s:%d: The queue is full",__func__,__LINE__);
    pthread_mutex_unlock(&buff_q->mutex);
    return -EINVAL;
  }
  for (i = 0; i < buff_count; i++) {
    buff_idx = ((native_buf_mgr->client_id << 24) + (stream_id << 16) +
      buff_q->current_buf_count);
    rc = pp_init_native_buffer(&buff_q->image_buf[buff_q->current_buf_count],
      native_buf_mgr->ion_fd, buff_idx, len_offset, cached);
    if ( rc < 0) {
      CDBG_ERROR("%s:%d: Error when native buffer was allocated",
        __func__, __LINE__);
      pthread_mutex_unlock(&buff_q->mutex);
      return -EFAULT;
    }
    buff_q->current_buf_count++;
  }
  pthread_mutex_unlock(&buff_q->mutex);

  return 0;
}

/** pp_native_buf_mgr_free_buff:
 *
 *  @native_buf_mgr - Native buffer manager structure holding all data.
 *  @session_id - Current session id.
 *  @stream_id - Current stream id.
 *  @buf_index - index of the buffer to be freed
 *
 *  This function search for buffer queue with the current identity and
 *  frees the buffer who has the required buffer index.
 *
 *  Return: It returns 0 on success.
 **/
int32_t pp_native_buf_mgr_free_buff(pp_native_buf_mgr_t *native_buf_mgr,
   uint32_t session_id, uint32_t stream_id, uint32_t buf_index)
{
  pp_bufq_t *buff_q;
  uint32_t i;
  int32_t rc;

  buff_q = pp_native_buf_mgr_find_queue(native_buf_mgr,session_id,stream_id);

  if(buff_q == NULL) {
    CDBG_ERROR("%s:%d: There is no free queue",__func__,__LINE__);
    return -ENOMEM;
  }

  for(i = 0; i < buff_q->current_buf_count; i++) {
    if (buff_q->image_buf[i].buffer.index == buf_index) {
      pp_deinit_native_buf(&buff_q->image_buf[i],
        native_buf_mgr->ion_fd);
    }
  }

  if (i > buff_q->current_buf_count) {
    CDBG_ERROR("%s:%d: Buffer index doesn't match",__func__,__LINE__);
    return -EFAULT;
  }

  return 0;
}

/** pp_native_manager_get_bufs:
 *
 *  @native_buf_mgr - Native buffer manager structure holding all data.
 *  @buff_count - number of the buffers to be received.
 *  @session_id - Current session id.
 *  @stream_id - Current stream id.
 *  @img_list - pointer to a list that will hold the received buffers.
 *
 *     This function searches for appropriate buffer queue using function
 *   pp_native_buf_mgr_find_queue(). On success it uses the queue to find
 *   unused buffers and loads their buffer holders in the list.
 *
 *  Return: It returns number of the loaded buffers.
 **/
int32_t pp_native_manager_get_bufs(pp_native_buf_mgr_t *native_buf_mgr,
  uint32_t buff_count, uint32_t session_id, uint32_t stream_id,
  mct_list_t **img_list)
{
  pp_bufq_t *buff_q;
  uint32_t i, added_count;
  uint32_t getbuf_idx;

  buff_q = pp_native_buf_mgr_find_queue(native_buf_mgr,session_id,stream_id);
  if (!buff_q) {
    CDBG_ERROR("%s:%d: buff_q not found.",__func__, __LINE__);
    return -1;
  }
  if (buff_q->current_buf_count == 0) {
    CDBG_ERROR("%s:%d: No buffers allocated for this identity",
      __func__, __LINE__);
    pp_native_buf_mgr_free_queue(native_buf_mgr,session_id,stream_id);
    return -EINVAL;
  }

  getbuf_idx = buff_q->current_getbuf_index;
  if (!((getbuf_idx + 1)% buff_q->current_buf_count)) {
    getbuf_idx = 0;
  }

  pthread_mutex_lock(&buff_q->mutex);
  for (i = getbuf_idx, added_count = 0; i < buff_q->current_buf_count; i++) {
    if (!buff_q->image_buf[i].busy) {
      *img_list = mct_list_append(*img_list, &buff_q->image_buf[i], NULL, NULL);
      buff_q->image_buf[i].busy = 1;
      buff_q->current_getbuf_index = i;
      if (++added_count == buff_count)
        break;
    }
  }
  pthread_mutex_unlock(&buff_q->mutex);

  return added_count;
}

pp_frame_buffer_t* pp_native_buf_manage_get_buf_info(
  pp_native_buf_mgr_t *native_buf_mgr, uint32_t session_id, uint32_t stream_id,
  uint32_t buf_idx)
{
  pp_bufq_t *buff_q;
  uint32_t i;

  buff_q = pp_native_buf_mgr_find_queue(native_buf_mgr,session_id,stream_id);

  if (!buff_q) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return NULL;
  }
  if (buff_q->current_buf_count == 0) {
    CDBG_ERROR("%s:%d: No buffers allocated for this identity",
      __func__, __LINE__);
    pp_native_buf_mgr_free_queue(native_buf_mgr,session_id,stream_id);
    return NULL;
  }
  for (i=0; i<buff_q->current_buf_count; i++) {
    if (buff_q->image_buf[i].buffer.index == buf_idx) {
      return &(buff_q->image_buf[i]);
    }
  }
  return NULL;
}

boolean pp_native_buf_mgr_queue_exists(pp_native_buf_mgr_t *native_buf_mgr,
                                        uint32_t session_id,uint32_t stream_id){
  pp_bufq_t *buff_q;
  uint32_t i;

  for (i = 0; i < PP_MAX_NUM_BUF_QUEUE; i++) {
    if (native_buf_mgr->bufq[i].used &&
      (native_buf_mgr->bufq[i].session_id == session_id) &&
      (native_buf_mgr->bufq[i].stream_id == stream_id)) {
       return true;
    }
  }
  return false;
}

/** pp_native_manager_put_buf:
 *
 *  @native_buf_mgr - Native buffer manager structure holding all data.
 *  @session_id - Current session id.
 *  @stream_id - Current stream id.
 *  @buff_idx - The index of the buffer to be released.
 *
 *     This function searches for appropriate buffer queue using function
 *   pp_native_buf_mgr_find_queue(). On success it release the buffer
 *   corresponding to the buff_idx parameter.
 *
 *  Return: It returns 0 on success.
 **/
int32_t pp_native_manager_put_buf(pp_native_buf_mgr_t *native_buf_mgr,
  uint32_t session_id, uint32_t stream_id, uint32_t buff_idx)
{
  pp_bufq_t *buff_q;
  uint32_t i;

  buff_q = pp_native_buf_mgr_find_queue(native_buf_mgr,session_id,stream_id);
  if (!buff_q) {
    CDBG_ERROR("%s:%d: buff_q not found", __func__, __LINE__);
    return -1;
  }

  if (buff_q->current_buf_count == 0) {
    CDBG_ERROR("%s:%d: No buffers allocated for this identity",
      __func__, __LINE__);
    pp_native_buf_mgr_free_queue(native_buf_mgr,session_id,stream_id);
    return -EINVAL;
  }

  pthread_mutex_lock(&buff_q->mutex);
  for (i = 0; i < buff_q->current_buf_count; i++) {
    if (buff_q->image_buf[i].buffer.index == buff_idx) {
      buff_q->image_buf[i].busy = 0;
      break;
    }
  }
  pthread_mutex_unlock(&buff_q->mutex);

  if (i == buff_q->current_buf_count) {
    CDBG_ERROR("%s:%d: Can not find buffer index", __func__, __LINE__);
    return -1;
    }


  return 0;
}

int32_t pp_buf_mgr_get_buf_handle(pp_native_buf_mgr_t  *pp_buf_mgr,
  struct ion_fd_data *fd_data)
{
  int rc = 0;
#ifndef TARGET_ION_ABI_VERSION
  pthread_mutex_lock(&pp_buf_mgr->mutex);
  rc = ioctl (pp_buf_mgr->ion_fd, ION_IOC_IMPORT, fd_data);
  if (rc < 0) {
    CDBG_ERROR("%s: ION_IOC_IMPORT failed %d", __func__, rc);
    goto end;
  }
  CDBG("%s: ION_IOC_IMPORT return success %d", __func__, rc);

end:
  pthread_mutex_unlock(&pp_buf_mgr->mutex);
#else
  (void)pp_buf_mgr;
  (void)fd_data;
#endif
  return rc;
}

/** pp_native_buf_mgr_cacheops: Do cache ops as requested
 *
 *  @v_buf_mgr: [INPUT] native buf mgr instance
 *  @vaddr: [INPUT] buffer address
 *  @fd: [INPUT] buffer fd
 *  @len: [INPUT] buffer length
 *  @cache_ops: [INPUT] cache operation that needs to be done
 *
 *  Do the cache operation as requested by client
 *
 *  Return: SUCCESS - TRUE
 *          FAILURE - FALSE **/
int32_t pp_native_buf_mgr_cacheops(pp_native_buf_mgr_t *pp_buf_mgr, void *vaddr,
   int fd, int len, pp_buf_mgr__cache_ops_type cache_ops)
{

int rc = 0;

#ifndef TARGET_ION_ABI_VERSION
  struct ion_flush_data cache_inv_data;
  struct ion_custom_data custom_data;
  struct ion_fd_data fd_data;
  struct ion_handle_data handle_data;
  uint32_t cmd = ION_IOC_CLEAN_INV_CACHES;

  CDBG("%s: vaddr %p, fd %d,  len %d, pp_buf_mgr %p ", __func__,
    vaddr, fd, len, pp_buf_mgr);

  memset (&fd_data, 0x0, sizeof(struct ion_fd_data));
  memset (&cache_inv_data, 0x0, sizeof(struct ion_flush_data));
  memset (&custom_data, 0x0, sizeof(struct ion_custom_data));
  memset (&handle_data, 0x0, sizeof(struct ion_handle_data));

  if (fd < 0) {
    CDBG_ERROR("Invalid ION fd %d", fd);
    return -1;
  }

  if (NULL == vaddr) {
    CDBG_ERROR("Buffer is null");
    return -1;
  }

  fd_data.fd = fd;
  rc = pp_buf_mgr_get_buf_handle(pp_buf_mgr, &fd_data);
  if (rc < 0) {
    fd_data.handle = 0;
  }

  switch (cache_ops) {
  case PP_BUF_MGR_CACHE_INVALIDATE:
    cmd = ION_IOC_INV_CACHES;
    break;
  case PP_BUF_MGR_CACHE_CLEAN:
    cmd = ION_IOC_CLEAN_CACHES;
    break;
  default:
  case PP_BUF_MGR_CACHE_CLEAN_INVALIDATE:
    cmd = ION_IOC_CLEAN_INV_CACHES;
    break;
  }

  CDBG_HIGH("CAMCACHE : CacheOp : "
    "Buffer(vaddr=%p, fd=%d, size=%d, offset=%d), "
    "type=%s, ion_device_fd=%d",
    vaddr, fd_data.fd, len, 0,
    (cache_ops == PP_BUF_MGR_CACHE_INVALIDATE) ? "INV" :
    (cache_ops == PP_BUF_MGR_CACHE_CLEAN) ? "CLEAN" : "CLEAN_INV",
    pp_buf_mgr->ion_fd);

  cache_inv_data.vaddr = vaddr;
  cache_inv_data.fd =  fd_data.fd;
  cache_inv_data.handle = fd_data.handle;
  cache_inv_data.length =  (unsigned int)len;
  cache_inv_data.offset =  0;

  custom_data.cmd =  (unsigned int)cmd;
  custom_data.arg = (unsigned long)&cache_inv_data;
  handle_data.handle = fd_data.handle;

  pthread_mutex_lock(&pp_buf_mgr->mutex);
  rc = ioctl (pp_buf_mgr->ion_fd, ION_IOC_CUSTOM, &custom_data);
  if (rc < 0) {
      CDBG_ERROR("%s: ION_IOC_CUSTOM error %d", __func__, rc);
      goto end;
  }

  CDBG("%s: ### ION_IOC_CUSTOM return success %d", __func__, rc);

end:
  if (fd_data.handle != 0) {
    ioctl(pp_buf_mgr->ion_fd, ION_IOC_FREE, &handle_data);
  }
  pthread_mutex_unlock(&pp_buf_mgr->mutex);
  return rc;
#else
  CDBG_HIGH("CAMCACHE : CacheOp : "
    "Buffer(vaddr=%p, size=%d, offset=%d), "
    "type=%s, ion_device_fd=%d",
    vaddr, len, 0,
    (cache_ops == PP_BUF_MGR_CACHE_INVALIDATE) ? "INV" :
    (cache_ops == PP_BUF_MGR_CACHE_CLEAN) ? "CLEAN" : "CLEAN_INV",
    pp_buf_mgr->ion_fd);

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
    switch (cache_ops) {
    case PP_BUF_MGR_CACHE_INVALIDATE:
      buf_sync_start.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_WRITE;
      buf_sync_end.flags   = DMA_BUF_SYNC_END   | DMA_BUF_SYNC_READ;
      break;
   case PP_BUF_MGR_CACHE_CLEAN:
      buf_sync_start.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_WRITE;
      buf_sync_end.flags   = DMA_BUF_SYNC_END   | DMA_BUF_SYNC_WRITE;
      break;
   default:
   case PP_BUF_MGR_CACHE_CLEAN_INVALIDATE:
      buf_sync_start.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_RW;
      buf_sync_end.flags   = DMA_BUF_SYNC_END   | DMA_BUF_SYNC_RW;
      break;
   }

   rc = ioctl(fd, DMA_BUF_IOCTL_SYNC, &buf_sync_start.flags);
   if (rc) {
      LOGE("Failed first DMA_BUF_IOCTL_SYNC start\n");
   }

   rc = ioctl(fd, DMA_BUF_IOCTL_SYNC, &buf_sync_end.flags);
   if (rc) {
      LOGE("Failed first DMA_BUF_IOCTL_SYNC end\n");
   }

  return 0;
#endif
}
