/**********************************************************************
* Copyright (c) 2015, 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __IMG_FCV_H__
#define __IMG_FCV_H__
#include "fastcv/fastcv.h"


#define IMG_FCV_LIB_NAME "libfastcvopt.so"

/** faceproc_fcv_lib_info_t
 *
 *  @ptr: pointer to dynamically loaded lib
 *  @pFcvScaleDownMNu8: Function pointer
 *  @pFcvScaleDownMNInterleaveu8: Function pointer
 *
 *   This structure represents fastcv lib info
 **/
typedef struct {
  void *ptr;
  void (*pFcvScaleDownMNu8)(
    const uint8_t* __restrict src,
    uint32_t srcWidth,
    uint32_t srcHeight,
    uint32_t srcStride,
    uint8_t* __restrict dst,
    uint32_t dstWidth,
    uint32_t dstHeight,
    uint32_t dstStride);
  void (*pFcvScaleDownMNInterleaveu8)(
    const uint8_t* __restrict src,
    uint32_t srcWidth,
    uint32_t srcHeight,
    uint32_t srcStride,
    uint8_t* __restrict dst,
    uint32_t dstWidth,
    uint32_t dstHeight,
    uint32_t dstStride);
  fcvStatus (*pfcvRotateImageu8)(
    const uint8_t* __restrict src,
    uint32_t srcWidth,
    uint32_t srcHeight,
    uint32_t srcStride,
    uint8_t* __restrict dst,
    uint32_t dstStride,
    uint32_t  degree);
  fcvStatus (*pfcvRotateImageInterleavedu8)(
    const uint8_t* __restrict src,
    uint32_t srcWidth,
    uint32_t srcHeight,
    uint32_t srcStride,
    uint8_t* __restrict dst,
    uint32_t dstStride,
    uint32_t  degree);
  int (*pfcvSetOperationMode)(
    fcvOperationMode mode);
  void (*pfcvCleanUp)(void);
} img_fcv_lib_handle_t;

/** img_fcv_get_lib_hdl
 *
 *  Arguments:
 *    @pp_handle - double pointer to fastcv lib handle.
 *
 *  Return values:
 *    IMG_SUCCESS/IMG_ERR_
 *
 *  Description:
 *    This function is called to get fastcv lib handle
 *
 **/
int img_fcv_get_lib_hdl(img_fcv_lib_handle_t **pp_handle);

/** img_fcv_put_lib_hdl
 *
 *  Arguments:
 *    @p_handle - pointer to fastcv lib handle.
 *
 *  Return values:
 *    IMG_SUCCESS/IMG_ERR_
 *
 *  Description:
 *    This function is called to put the fastcv lib handle
 *
 **/
int img_fcv_put_lib_hdl(img_fcv_lib_handle_t *p_handle);

#endif //__IMG_FCV_H__
