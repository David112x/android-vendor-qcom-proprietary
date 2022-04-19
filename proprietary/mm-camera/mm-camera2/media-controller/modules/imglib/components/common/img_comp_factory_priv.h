/**********************************************************************
*  Copyright (c) 2013-2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __IMG_COMP_FACTORY_PRIV_H__
#define __IMG_COMP_FACTORY_PRIV_H__

#include "img_comp_factory.h"

/** img_comp_reg_t
 *   @role: role of the component
 *   @name: name of the component
 *   @ops: function table for the operation
 *
 *   Registry for the imaging components
 *
 **/
typedef struct {
  img_comp_role_t role;
  char *name;
  img_core_ops_t ops;
} img_comp_reg_t;


/* Since all the components are compiled as part of the
 * same library, the function symbols needs to be exported
 */
/** wd_comp_create
 *   @handle: library handle
 *   @p_ops: pointer to the image component ops
 *
 *   Create wavelet denoise component
 **/
extern int wd_comp_create(void* handle, img_component_ops_t *p_ops);

/** wd_comp_load
 *   @name: library name
 *   @handle: library handle
 *
 *   Load wavelet denoise component
 **/
extern int wd_comp_load(const char* name, void** handle);

/** wd_comp_load
 *   @handle: library handle
 *
 *   UnLoad wavelet denoise component
 **/
extern int wd_comp_unload(void* handle);

/** hdr_comp_create
 *   @handle: library handle
 *   @p_ops: pointer to the image component ops
 *
 *   Create HDR component
 **/
extern int hdr_comp_create(void* handle, img_component_ops_t *p_ops);

/** hdr_comp_load
 *   @name: library name
 *   @handle: library handle
 *
 *   Load HDR component
 **/
extern int hdr_comp_load(const char* name, void** handle);

/** hdr_comp_unload
 *   @handle: library handle
 *
 *   UnLoad HDR component
 **/
extern int hdr_comp_unload(void* handle);

/** faceproc_comp_create
 *   @p_ops: pointer to the image component ops
 *   @handle: library handle
 *
 *   Create faceproc component
 **/
extern int faceproc_comp_create(void* handle, img_component_ops_t *p_ops);

/** faceproc_comp_load
 *   @name: library name
 *   @handle: library handle
 *
 *   Load faceproc component
 **/
extern int faceproc_comp_load(const char* name, void** handle);

/** faceproc_comp_unload
 *   @handle: library handle
 *
 *   UnLoad faceproc component
 **/
extern int faceproc_comp_unload(void* handle);

/** faceproc_dsp_comp_create
 *   @p_ops: pointer to the image component ops
 *   @handle: library handle
 *
 *   Create faceproc component
 **/
extern int faceproc_dsp_comp_create(void* handle, img_component_ops_t *p_ops);

/** faceproc_dsp_comp_load
 *   @name: library name
 *   @handle: library handle
 *
 *   Load faceproc component
 **/
extern int faceproc_dsp_comp_load(const char* name, void** handle);

/** faceproc_dsp_comp_unload
 *   @handle: library handle
 *
 *   UnLoad faceproc component
 **/
extern int faceproc_dsp_comp_unload(void* handle);

/** faceproc_hw_comp_create
 *   @p_ops: pointer to the image component ops
 *
 *   Create HW faceproc component
 **/
extern int faceproc_hw_comp_create(void* handle, img_component_ops_t *p_ops);

/** faceproc_hw_comp_load
 *   @name: library name
 *   @handle: library handle
 *
 *   Load HW faceproc component
 **/
extern int faceproc_hw_comp_load(const char* name, void** handle);

/** faceproc_hw_comp_unload
 *   @handle: library handle
 *
 *   UnLoad HW faceproc component
 **/
extern int faceproc_hw_comp_unload(void* handle);

/** cac_comp_create
 *   @handle: library handle
 *   @p_ops: pointer to the image component ops
 *
 *   Create cac component
 **/
extern int cac_comp_create(void* handle, img_component_ops_t *p_ops);

/** cac_comp_load
 *   @name: library name
 *   @handle: library handle
 *
 *   Load CAC component
 **/
extern int cac_comp_load(const char* name, void** handle);

/** cac_comp_unload
 *   @handle: library handle
 *
 *   UnLoad CAC component
 **/
extern int cac_comp_unload(void* handle);

/** frameproc_comp_create
 *   @handle: library handle
 *   @p_ops: pointer to the image component ops
 *
 *   Create faameproc component
 **/
extern int frameproc_comp_create(void* handle, img_component_ops_t *p_ops);

/** frameproc_comp_load
 *   @name: library name
 *   @handle: library handle
 *
 *   Load faameproc component
 **/
extern int frameproc_comp_load(const char* name, void** handle);

/** frameproc_comp_unload
 *   @handle: library handle
 *
 *   UnLoad faameproc component
 **/
extern int frameproc_comp_unload(void* handle);

/** cac2_comp_create
 *   @handle: library handle
 *   @p_ops: pointer to the image component ops
 *
 *   Create cac component
 **/
extern int cac2_comp_create(void* handle, img_component_ops_t *p_ops);

/** cac2_comp_load
 *   @name: library name
 *   @handle: library handle
 *
 *   Load CAC component
 **/
extern int cac2_comp_load(const char* name, void** handle);

/** cac2_comp_unload
 *
 *   UnLoad CAC component
 **/
extern int cac2_comp_unload(void* handle);

/** cac2_comp_alloc
 *   @handle: pointer to the handle
 *   @p_params: pointer to the alloc params
 *
 *   Allocate Memory for cac
 **/
extern int cac2_comp_alloc(void *, void *);

/** cac2_comp_dealloc
 *   @handle: pointer to the handle
 *
 *   deallocate Memory for cac
 **/
extern int cac2_comp_dealloc(void *);

/** cac3_comp_create
 *   @p_ops: pointer to the image component ops
 *
 *   Create cac3 component
 **/
extern int cac3_comp_create(void* handle, img_component_ops_t *p_ops);

/** cac3_comp_load
 *   @name: library name
 *   @handle: library handle
 *
 *   Load CAC component
 **/
extern int cac3_comp_load(const char* name, void** handle);

/** cac3_comp_unload
 *
 *   UnLoad CAC component
 **/
extern int cac3_comp_unload(void* handle);

/** cac3_comp_alloc
 *   @handle: pointer to the handle
 *   @p_params: pointer to the alloc params
 *
 *   Allocate Memory for cac
 **/
 extern int cac3_comp_alloc(void *, void *);

/** cac3_comp_alloc
 *   @handle: pointer to the handle
 *
 *   deallocate Memory for cac
 **/
extern int cac3_comp_dealloc(void *);

/** dual_frameproc_comp_create
 *   @handle: library handle
 *   @p_ops: pointer to the image component ops
 *
 *   Create dual_frameproc component
 **/
extern int dual_frameproc_comp_create(void* handle, img_component_ops_t *p_ops);

/** dual_frameproc_comp_load
 *   @name: library name
 *   @handle: library handle
 *
 *   Load dual_frameproc component
 **/
extern int dual_frameproc_comp_load(const char* name, void** handle);

/** dual_frameproc_comp_unload
 *   @handle: library handle
 *
 *   UnLoad dual_frameproc component
 **/
extern int dual_frameproc_comp_unload(void* handle);

/** dual_frameproc_comp_bind
 *   @handle_main: Handle of the main instance
 *   @handle_main: Handle of the aux instance
 *
 *   Bind 2 instances of dual_frameproc component
 **/
extern int dual_frameproc_comp_bind(void* handle_main, void* handle_aux);

/** dual_frameproc_comp_unbind
 *   @handle_main: Handle of the main instance
 *   @handle_main: Handle of the aux instance
 *
 *   Unbind 2 instances of dual_frameproc component
 **/
extern int dual_frameproc_comp_unbind(void* handle_main, void* handle_aux);

/**
 * Function: frameproc_comp_alloc
 *
 * Description: This function is used to Allocate buffers
 * library
 *
 * Input parameters:
 *   @handle: library handle
 *   @p_params: alloc parameters
 *
 * Return values:
 *     Standard Imglb Error values
 *
 * Notes: none
 **/
extern int frameproc_comp_alloc(void *handle, void *p_params);

/**
 * Function: frameproc_comp_dealloc
 *
 * Description: This function is used to deallocate buffers in
 * the library
 *
 * Input parameters:
 *   @handle: library handle
 *
 * Return values:
 *     Standard Imglb Error values
 *
 * Notes: none
 **/
extern int frameproc_comp_dealloc(void *handle);

/**
 * Function: frameproc_comp_preload_needed
 *
 * Description: This function is used to indicate whether
 *              preload is needed
 *
 * Input parameters:
 *   @handle: library handle
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
extern bool frameproc_comp_preload_needed(void *handle);

/** lib2d_comp_create
 *   @handle: library handle
 *   @p_ops: pointer to the image component ops
 *
 *   Create lib2d component
 **/
extern int lib2d_comp_create(void* handle, img_component_ops_t *p_ops);

/** lib2d_comp_load
 *   @name: library name
 *   @handle: library handle
 *
 *   Load lib2d component
 **/
extern int lib2d_comp_load(const char* name, void** handle);

/** lib2d_comp_unload
 *
 *   UnLoad lib2d component
 **/
extern int lib2d_comp_unload(void* handle);

/** jpegdma_comp_create
 *   @handle: library handle
 *   @p_ops: pointer to the image component ops
 *
 *   Create jpegdma component
 **/
extern int jpegdma_comp_create(void* handle, img_component_ops_t *p_ops);

/** jpegdma_comp_load
 *   @name: library name
 *   @handle: library handle
 *
 *   Load jpegdma component
 **/
extern int jpegdma_comp_load(const char* name, void** handle);

/** jpegdma_comp_unload
 *
 *   UnLoad jpegdma component
 **/
extern int jpegdma_comp_unload(void* handle);


/** multi_frameproc_comp_create
 *   @handle: library handle
 *   @p_ops: pointer to the image component ops
 *
 *   Create multi_frameproc component
 **/
extern int multi_frameproc_comp_create(void* handle, img_component_ops_t *p_ops);

/** multi_frameproc_core_load
 *   @name: library name
 *   @handle: library handle
 *
 *   Load multi_frameproc component
 **/
extern int multi_frameproc_core_load(const char* name, void** handle);

/** multi_frameproc_core_unload
 *   @handle: library handle
 *
 *   UnLoad multi_frameproc component
 **/
extern int multi_frameproc_core_unload(void* handle);

/** multi_frameproc_mgr_bind
 *   @handle_main: Handle of the first instance
 *   @handle_main: Handle of the second instance
 *
 *   Bind 2 instances of multi_frameproc component
 **/
extern int multi_frameproc_mgr_bind(void* handle_main, void* handle_aux);

/** multi_frameproc_mgr_unbind
 *   @handle_main: Handle of the first instance
 *   @handle_main: Handle of the second instance
 *
 *   Unbind 2 instances of multi_frameproc component
 **/
extern int multi_frameproc_mgr_unbind(void* handle_main, void* handle_aux);

#endif //__IMG_COMP_FACTORY_PRIV_H__
