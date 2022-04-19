/***************************************************************************
* Copyright (c) 2013-2015 Qualcomm Technologies, Inc.                      *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
****************************************************************************/

#ifndef __IMG_MEM_OPS_H__
#define __IMG_MEM_OPS_H__

#include <stdio.h>
#include <linux/msm_ion.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

/** img_buf_type_t
 *  IMG_BUFFER_HEAP: Heap buffer
 *  IMG_BUFFER_ION_IOMMU: ION IOMMU buffer
 *  IMG_BUFFER_ION_ADSP: ION ADSP buffer
 *  IMG_BUFFER_ION_CUSTOM: Custom heap like a pre allocated
 *  heap. Heap id must be provided with this option.
 *
 *  Imaging buffer type
 **/
typedef enum {
  IMG_BUFFER_HEAP,
  IMG_BUFFER_ION_IOMMU,
  IMG_BUFFER_ION_ADSP,
  IMG_BUFFER_ION_CUSTOM,
} img_buf_type_t;

/** img_cache_ops_t
 *  IMG_CACHE_INV: Invalidate cache
 *  IMG_CACHE_CLEAN: Clean cache
 *  IMG_CACHE_CLEAN_INV:
 *
 *  Imaging cache operation type
 **/
typedef enum {
  IMG_CACHE_NO_OP,
  IMG_CACHE_INV,
  IMG_CACHE_CLEAN,
  IMG_CACHE_CLEAN_INV,
} img_cache_ops_t;

/** img_mem_alloc_type_t
 *  IMG_INTERNAL: Allocated internally in the imglib
 *  IMG_EXTERNAL: Allocated external to the imglib
 *
 *  Indicates if the memory was allocated internally in the
 *  imglib module or externally
 **/
typedef enum {
  IMG_INTERNAL,
  IMG_EXTERNAL,
} img_mem_alloc_type_t;

/** img_mem_handle_t
 *  @fd: buffer fd
 *  @vaddr: virtual address of the buffer
 *  @handle: buffer handle
 *  @length: buffer length
 *
 *  Image memory handle
 **/
typedef struct {
  int fd;
  void *vaddr;
  void *handle;
  size_t length;
} img_mem_handle_t;

/** img_mem_ops_t
 *    @open: function pointer to open/initialize the mem
 *         framework
 *    @get_buffer: function pointer for creating buffer
 *    @release_buffer:  function pointer for releasing the
 *                   buffer
 *    @cache_ops: cache ops for buffer
 *    @close: function pointer to close/deinit the mem framework
 *
 *    Memory ops table
 **/
typedef struct {
  /** img_buffer_open:
   *
   *  Arguments:
   *    None
   *
   *  Return:
   *     Ion device fd. -1 on failure
   *
   *  Description:
   *      Open Ion device and returns the fd. Only one instance of
   *      the ion device is open for all imglib ion memory
   *      operatons
   *
   **/
  int (*open)();

  /** get_buffer:
   *
   *  Arguments:
   *     @type: buffer type
   *     @heapid: heapid needs to be provided if buffer type is
   *            IMG_BUFFER_ION_CUSTOM. -1 otherwise.
   *     @cached: Flag to indicate whether the buffer needs to be
   *            cached
   *     @p_handle: memory output handle
   *     @length: buffer size
   *
   *  Return:
   *     0 for success, -ve values for failure
   *
   *  Description:
   *      Allocates buffer
   *
   **/
  int (*get_buffer)(img_buf_type_t type, int heapid, int8_t cached, int length,
    img_mem_handle_t *p_handle);

  /** release_buffer:
   *
   *  Arguments:
   *     @p_handle: image memory handle
   *
   *  Return:
   *     Upon successful completion, returns 0. Otherwise, it returns -1
   *
   *  Description:
   *      Deallocates buffer
   *
   **/
  int (*release_buffer)(img_mem_handle_t *p_handle);

  /** cache_ops:
   *
   *  Arguments:
   *     @p_handle: image memory handle
   *     @ops: cache operation type
   *     @alloc_type: memory allocation type
   *
   *  Return:
   *     Upon successful completion, returns 0. Otherwise, it returns -1
   *
   *  Description:
   *      deallocates ION buffer
   *
   **/
  int (*cache_ops)(img_mem_handle_t *p_handle, img_cache_ops_t ops,
    img_mem_alloc_type_t alloc_type);

  /** img_buffer_close:
   *
   *  Arguments:
   *    None
   *
   *  Return:
   *     0 for success, -ve values for failure
   *
   *  Description:
   *      Close the Ion device
   *
   **/
  int (*close)();

} img_mem_ops_t;

#endif //__IMG_MEM_OPS_H__

