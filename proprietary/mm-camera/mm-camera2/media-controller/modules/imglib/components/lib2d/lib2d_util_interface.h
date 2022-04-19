/***************************************************************************
* Copyright (c) 2015-2017 Qualcomm Technologies, Inc.                           *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
****************************************************************************/

#ifndef __LIB2D_UTIL_INTERFACE_H__
#define __LIB2D_UTIL_INTERFACE_H__

#ifndef boolean
typedef uint8_t boolean;
#endif


/**
 * Function: c2d_util_open
 *
 * Description: function to open c2d library. Creates
 *     default source, destination surfaces.
 *
 * Input parameters:
 *   lib_private - handle to c2d_util_handle
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t c2d_util_open(void **lib_private);

/**
 * Function: c2d_util_close
 *
 * Description: function to close c2d library.
 *
 * Input parameters:
 *   lib_private - handle to c2d_util_handle
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int32_t c2d_util_close(void *lib_private);

/**
 * Function: fcv_rotate
 *
 * Description: Rotate the frame
 *
 * Input parameters:
 *   fcv_hdl - handle to fcv
 *   p_in_frame - pointer to input frame
 *   p_out_frame - pointer to output frame
 *   p_meta - pointer to meta data
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/

int32_t  fcv_rotate(void *fcv_hdl, img_frame_t *p_in_frame,
  img_frame_t *p_out_frame, img_meta_t *p_meta);

/**
 * Function: c2d_util_process_frame
 *
 * Description: Process the frame
 *
 * Input parameters:
 *   lib_private - handle to c2d_util_handle
 *   p_in_frame - pointer to input frame
 *   p_out_frame - pointer to output frame
 *   p_meta - pointer to meta data
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t c2d_util_process_frame(void *lib_private, img_frame_t *p_in_frame,
  img_frame_t *p_out_frame, img_meta_t *p_meta);

/**
 * Function: c2d_util_update_default_surface
 *
 * Description: Update the default surface. If the current
 *     requested format is not same as default surface's format
 *     destroy the default surface and create with current format.
 *
 * Input parameters:
 *   lib_private - handle to c2d_util_handle
 *   is_source - whether this is a source or destination surface
 *   format - format of the current surface
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int32_t c2d_util_update_default_surface(void *lib_private,
  boolean is_source, cam_format_t format);

#endif
