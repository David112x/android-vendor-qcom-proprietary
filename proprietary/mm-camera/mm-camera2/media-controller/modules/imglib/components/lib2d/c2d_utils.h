/***************************************************************************
* Copyright (c) 2015 Qualcomm Technologies, Inc.                           *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
****************************************************************************/

#ifndef __C2D_UTIL_H__
#define __C2D_UTIL_H__

#include "c2d2.h"
#include "c2dExt.h"

/** c2d_util_lib_t
 * @ptr: handle to the c2d library
 * @c2dCreateSurface: function pointer to c2dCreateSurface
 * @c2dUpdateSurface: function pointer to c2dUpdateSurface
 * @c2dDraw: function pointer to c2dDraw
 * @c2dLensCorrection: function pointer to c2dLensCorrection
 * @c2dFinish: function pointer to c2dFinish
 * @c2dDestroySurface: function pointer to c2dDestroySurface
 * @c2dMapAddr: function pointer to c2dMapAddr
 * @c2dUnMapAddr: function pointer to c2dUnMapAddr
**/
typedef struct {
  void *ptr;
  C2D_STATUS (*c2dCreateSurface)(uint32_t *surface_id, uint32_t surface_bits,
    C2D_SURFACE_TYPE surface_type, void *surface_definition);
  C2D_STATUS (*c2dUpdateSurface)(uint32_t surface_id, uint32_t surface_bits,
    C2D_SURFACE_TYPE surface_type, void *surface_definition);
  C2D_STATUS (*c2dDraw)(uint32_t target_id, uint32_t target_config,
    C2D_RECT *target_scissor, uint32_t target_mask_id,
    uint32_t target_color_key, C2D_OBJECT *objects_list, uint32_t num_objects);
  C2D_STATUS (*c2dLensCorrection)(uint32_t targetSurface,
    C2D_LENSCORRECT_OBJECT *sourceObject);
  C2D_STATUS (*c2dFinish)(uint32_t target_id);
  C2D_STATUS (*c2dDestroySurface)(uint32_t surface_id);
  C2D_STATUS (*c2dMapAddr)(int mem_fd, void *hostptr, uint32_t len,
    uint32_t offset, uint32_t flags, void **gpuaddr);
  C2D_STATUS (*c2dUnMapAddr)(void *gpuaddr);
  /* c2dGetDriverCapabilities is not present in A family */
  //C2D_STATUS (*c2dGetDriverCapabilities)(C2D_DRIVER_INFO * driver_info);
} c2d_util_lib_t;

/** c2d_surface_type
 * @C2D_SURFACE_RGB: RGB surface type
 * @C2D_SURFACE_YUV: YUV surface type
**/
typedef enum c2d_surface_type_t {
  C2D_SURFACE_RGB,
  C2D_SURFACE_YUV,
} c2d_surface_type;

/** c2d_yuv_surface
 * @surface_def: c2d yuv surface defintions
 * @format: c2d YUV surface format
**/
typedef struct _c2d_yuv_surface_t {
  C2D_YUV_SURFACE_DEF surface_def;
  C2D_YUV_FORMAT      format;
} c2d_yuv_surface;

/** c2d_rgb_surface
 * @surface_def: c2d rgb surface defintions
 * @format: c2d rgb surface format
**/
typedef struct _c2d_rgb_surface_t {
  C2D_RGB_SURFACE_DEF surface_def;
  C2D_RGB_FORMAT      format;
} c2d_rgb_surface;

/** c2d_surface
 * @id: c2d surface id
 * @surface_type: c2d surface type
 * @yuv_surface: yuv surface handle
 * @rgb_surface: rgb surface handle
 * @surface_bit: surface bits
 * @gpu_map_addr: gpu mapped address
**/
typedef struct _c2d_surface_t {
  uint32_t         id;
  c2d_surface_type surface_type;
  union {
    c2d_yuv_surface yuv_surface;
    c2d_rgb_surface rgb_surface;
  };
  C2D_SURFACE_BITS surface_bit;
  void *gpu_map_addr;
} c2d_surface;

/** c2d_util_handle
 * @c2d_lib: c2d util lib handle
 * @src_surface: c2d source surface handle
 * @dst_surface: c2d dst surface handle
**/
typedef struct _c2d_util_handle_t {
  c2d_util_lib_t     c2d_lib;
  c2d_surface        src_surface;
  c2d_surface        dst_surface;
} c2d_util_handle;

#endif
