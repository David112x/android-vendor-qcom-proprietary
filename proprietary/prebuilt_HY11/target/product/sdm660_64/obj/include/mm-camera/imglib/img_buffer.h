/***************************************************************************
* Copyright (c) 2013-2015 Qualcomm Technologies, Inc.                      *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
****************************************************************************/

#ifndef __IMG_BUFFER_H__
#define __IMG_BUFFER_H__

#include "img_common.h"
#include "img_mem_ops.h"

/** img_buffer_t:
 *
 *  Arguments:
 *     @ion_info_fd: ION fd information
 *     @alloc: ION allocation data
 *     @fd: buffer fd
 *     @ion_fd: ION fd
 *     @addr: virtual address
 *     @cached: Flag indicating if the memory is cached or
 *            uncached
 *
 *  Description:
 *      Buffer structure
 *
 **/
typedef struct  {
  struct ion_fd_data ion_info_fd;
  struct ion_allocation_data alloc;
  int fd;
  int ion_fd;
  uint8_t *addr;
  int cached;
} img_buffer_t;

/** img_buffer_priv_t
 *
 *  Arguments:
 *     @ion_fd: Ion device fd
 *     @ref_count: Ref count of the number of clients opening
 *               the ion device
 *
 *  Description:
 *     Data structure to hold generic data for the img buffer.
 *
 */
typedef struct {
  int ion_fd;
  int ref_count;
} img_buffer_priv_t;

/** img_buffer_open
 *
 *  Arguments:
 *     None
 *
 *  Return:
 *     Ion device fd
 *
 *  Description:
 *      Open Ion device and returns the fd. Should be called
 *      only once for all modules in imglib
 *
 **/
int img_buffer_open();


/** img_buffer_close
 *
 *  Arguments:
 *     @ion_fd: Ion device Fd
 *
 *  Return:
 *     0 for success, -ve values for failure
 *
 *  Description:
 *      Close the Ion device
 *
 **/
int img_buffer_close();

/** img_buffer_get
 *
 *  Arguments:
 *     @type: buffer type
 *     @heapid: heapid needs to be provided if buffer type is
 *            IMG_BUFFER_ION_CUSTOM. -1 otherwise.
 *     @cached: Flag to indicate whether the buffer needs to be
 *            cached
 *     @length: buffer size to be allocated
 *     @p_handle: memory output
 *
 *  Return:
 *     0 for success, -ve values for failure
 *
 *  Description:
 *      allocates buffer
 *
 **/
int img_buffer_get(img_buf_type_t type, int heapid, int8_t cached, int length,
    img_mem_handle_t *p_handle);

/** img_buffer_release:
 *
 *  Arguments:
 *     @p_handle: image memory handle
 *
 *  Return:
 *     Upon successful completion, returns 0. Otherwise, it returns -1
 *
 *  Description:
 *      deallocates ION buffer
 *
 **/
int img_buffer_release(img_mem_handle_t *p_handle);

/** img_buffer_cacheops:
 *
 *  Arguments:
 *     @p_handle: image memory handle
 *     @ops: cache operation type
 *     @mem_alloc_type: Memory Alloc type - internal to imglib
 *                    or external
 *
 *  Return:
 *     Upon successful completion, returns 0. Otherwise, it returns -1
 *
 *  Description:
 *      Cache operations for ION buffer
 *
 **/
int img_buffer_cacheops(img_mem_handle_t *p_handle, img_cache_ops_t ops,
  img_mem_alloc_type_t mem_alloc_type);


/***********  APIs for Buffer Pool **************************************/

/** img_buf_params_t:
 *  @min_dim: minimum dimension among width, height. if input
 *          width is smaller, width of the scaled frame will be
 *          will be set to min_dim and height will scaled based
 *          on the input aspect ratio. and viceversa...
 *
 *  Buffer parameters
 */
typedef struct {
  uint32_t min_dim;
} img_buf_params_t;

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
void *img_buf_pool_create(img_buf_params_t *p_params);

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
void img_buf_pool_destroy(void *handle);

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
  char *debug_str);

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
  char *debug_str);

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
int32_t img_buf_pool_reset(void *handle, int8_t release_buf);

#endif //__IMG_BUFFER_H__

