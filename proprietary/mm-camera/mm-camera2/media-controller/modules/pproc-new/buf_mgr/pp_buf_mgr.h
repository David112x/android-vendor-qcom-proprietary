/* pp_buf_mgr.h
 *
 * Copyright (c) 2013,2017, 2019 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __PP_BUF_MGR_H__
#define __PP_BUF_MGR_H__

#include "mtype.h"
#include "mct_stream.h"
#include <linux/msm_ion.h>
#if TARGET_ION_ABI_VERSION >= 2
#include <ion/ion.h>
#endif

#define PP_MAX_NUM_BUF_QUEUE 6
#define PP_MAX_BUF_COUNT  32

/** pp_buf_mgr__cache_ops_type
*   @PP_BUF_MGR_CACHE_NO_OP: No operation
*   @PP_BUF_MGR_CACHE_INVALIDATE: invalidation
*   @PP_BUF_MGR_CACHE_CLEAN: cache clean
*   @PP_BUF_MGR_CACHE_CLEAN_INVALIDATE: cache clean invalidation
*
*    Different cache operations
*
**/
typedef enum {
  PP_BUF_MGR_CACHE_NO_OP,
  PP_BUF_MGR_CACHE_INVALIDATE,
  PP_BUF_MGR_CACHE_CLEAN,
  PP_BUF_MGR_CACHE_CLEAN_INVALIDATE,
} pp_buf_mgr__cache_ops_type;

typedef struct _pp_frame_buffer_t {
  void *vaddr;
  int fd;
  struct v4l2_buffer buffer;
  struct v4l2_plane planes[VIDEO_MAX_PLANES];
  unsigned long addr[VIDEO_MAX_PLANES];
  struct ion_allocation_data ion_alloc[VIDEO_MAX_PLANES];
  struct ion_fd_data fd_data[VIDEO_MAX_PLANES];
  int32_t cached;
  uint32_t busy;
} pp_frame_buffer_t;

typedef struct _pp_bufq_t {
  uint32_t session_id;
  uint32_t stream_id;
  uint32_t used;
  uint32_t current_buf_count;
  uint32_t current_getbuf_index;
  pp_frame_buffer_t image_buf[PP_MAX_BUF_COUNT];
  pthread_mutex_t mutex;
} pp_bufq_t;


typedef struct _pp_native_buf_mgr_t {
  pthread_mutex_t mutex;
  pthread_mutex_t req_mutex;
  uint32_t use_cnt;
  int ion_fd;
  uint32_t bufq_handle_count;
  pp_bufq_t bufq[PP_MAX_NUM_BUF_QUEUE];
  uint32_t client_id;
} pp_native_buf_mgr_t;




/** pp_buf_mgr_open: buffer manager open
 *
 *  On first call, create buffer manager instance, call function
 *  to open buffer manager subdevice and increment ref count.
 *
 *  Return: buffer manager instance on SUCCESS
 *          NULL on failure **/
void *pp_buf_mgr_open(void);

/** pp_buf_mgr_close: buffer manager close
 *
 *  @buf_mgr: buf mgr instance
 *
 *  Decrement buffer manager ref count. If ref count is zero,
 *  close sub device and free memory
 *
 *  Return: TRUE on SUCCESS
 *          FALSE on failure **/
boolean pp_buf_mgr_close(void *v_buf_mgr);

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
  mct_stream_info_t *stream_info);

mct_stream_map_buf_t* pp_buf_mgr_get_buf_idx(void *v_buf_mgr,
  uint32_t identity,mct_list_t *list, uint32_t idx);

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
  uint32_t buff_idx, uint32_t frameid, struct timeval timestamp);

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
  uint32_t buff_idx, uint32_t frameid, struct timeval timestamp);

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
  mct_stream_info_t *stream_info);

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
  mct_stream_info_t *stream_info, void **vaddr);

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
   int fd, int len, pp_buf_mgr__cache_ops_type cache_ops);

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
  uint32_t client_id);

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
void pp_native_buf_mgr_deinit(pp_native_buf_mgr_t *native_buf_mgr);

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
int32_t pp_init_native_buffer(pp_frame_buffer_t *img_buf, int ion_fd, uint32_t idx,
  cam_frame_len_offset_t *len_offset, uint32_t cached);

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
int32_t pp_deinit_native_buf(pp_frame_buffer_t *img_buf, int ion_fd);

boolean pp_native_buf_mgr_queue_exists(pp_native_buf_mgr_t *native_buf_mgr,
                                        uint32_t session_id, uint32_t stream_id);
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
  cam_frame_len_offset_t *len_offset,uint32_t cached);

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
   uint32_t session_id, uint32_t stream_id, uint32_t buf_index);

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
  mct_list_t **img_list);

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
  uint32_t session_id, uint32_t stream_id, uint32_t buff_idx);


void pp_native_buf_mgr_free_queue(pp_native_buf_mgr_t *native_buf_mgr,
  uint32_t session_id, uint32_t stream_id);

pp_frame_buffer_t* pp_native_buf_manage_get_buf_info(
  pp_native_buf_mgr_t *native_buf_mgr, uint32_t session_id, uint32_t stream_id,
  uint32_t buf_idx);

int32_t pp_native_buf_mgr_cacheops(pp_native_buf_mgr_t *pp_buf_mgr, void *vaddr,
  int fd, int len, pp_buf_mgr__cache_ops_type cache_ops);

int32_t pp_buf_mgr_get_buf_handle(pp_native_buf_mgr_t  *pp_buf_mgr,
  struct ion_fd_data *fd_data);
#endif
