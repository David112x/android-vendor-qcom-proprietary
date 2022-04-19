/***************************************************************************
* Copyright (c) 2015, 2017 Qualcomm Technologies, Inc.                           *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
****************************************************************************/

#ifndef __LIB2D_COMP_H__
#define __LIB2D_COMP_H__

#include "img_comp_priv.h"
#include "cam_intf.h"
#include "img_fcv.h"

/** lib2d_comp_t
 *   @b: base component
 *   @lib2dutil_handle: lib2d util handle
 *   @mode: component mode
 *   @src_format: source surface format
 *   @dst_format: destination surface format
 *   @userdata: user data
 *   @lib2d_cb: callback
 *   @p_fcv_hdl: fcv_handle
 *
 **/
typedef struct {
  /*base component*/
  img_component_t b;
  void *lib2dutil_handle;
  img_comp_mode_t mode;
  cam_format_t src_format;
  cam_format_t dst_format;
  void *userdata;
  int (*lib2d_cb) (void *userdata, img_frame_t *p_in_frame,
    img_frame_t *p_out_frame, img_meta_t *p_meta);
  img_fcv_lib_handle_t *p_fcv_hdl;
} lib2d_comp_t;

#endif
