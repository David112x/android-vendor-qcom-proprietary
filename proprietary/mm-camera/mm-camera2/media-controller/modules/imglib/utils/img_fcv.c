/**********************************************************************
* Copyright (c) 2015-2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "img_dsp_dl_mgr.h"
#include "img_common.h"
#include "img_fcv.h"
#include "img_comp_priv.h"
#include <stdbool.h>
#include <dlfcn.h>

 /** img_fcv_obj_t
 *
 *   @hdl: fastcv lib handle
 *   @mutex: mutex to protect global variables
 *   @ref_cnt : reference count for fastcv load
 *   @dsp_dlopen_f : flag indicating dsp_dlopen successful
 *
 *   Struct for img_fcv obj
 **/
typedef struct {
  img_fcv_lib_handle_t hdl;
  pthread_mutex_t mutex;
  uint32_t ref_cnt;
  bool dsp_dlopen_f;
} img_fcv_obj_t;

/*global img_fcv_obj variable*/
static img_fcv_obj_t g_obj = {
  .mutex = PTHREAD_MUTEX_INITIALIZER
};

/**
 * Function: img_fcv_load_fn_ptrs
 *
 * Description: This function is used to map fastcv functions
 *
 *  Arguments:
 *    @p_fcv_lib_hdl - Pointer to fastcv lib handle.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR_
 *
 * Notes: none
 **/
static int img_fcv_load_fn_ptrs(img_fcv_lib_handle_t *p_fcv_lib_hdl)
{
  IMG_DLSYM_ERROR_RET(p_fcv_lib_hdl, ptr,
    pFcvScaleDownMNu8, "fcvScaleDownMNu8", 1);
  IMG_DLSYM_ERROR_RET(p_fcv_lib_hdl, ptr,
    pFcvScaleDownMNInterleaveu8, "fcvScaleDownMNInterleaveu8", 1);
  IMG_DLSYM_ERROR_RET(p_fcv_lib_hdl, ptr,
    pfcvRotateImageu8, "fcvRotateImageu8", 1);
  IMG_DLSYM_ERROR_RET(p_fcv_lib_hdl, ptr,
    pfcvRotateImageInterleavedu8, "fcvRotateImageInterleavedu8", 1);
  IMG_DLSYM_ERROR_RET(p_fcv_lib_hdl, ptr,
    pfcvSetOperationMode, "fcvSetOperationMode" , 1);
  IMG_DLSYM_ERROR_RET(p_fcv_lib_hdl, ptr,
    pfcvCleanUp, "fcvCleanUp" , 1);

  return IMG_SUCCESS;
}

/** img_fcv_unload_lib
 *
 *  Arguments:
 *    @p_handle - Pointer to fastcv lib handle.
 *    @p_userdata - Pointer to userdata.
 *
 *  Return values:
 *    IMG_SUCCESS/IMG_ERR_
 *
 *  Description:
 *    This function is called to unload fastcv lib
 *
 **/
int img_fcv_unload_lib(void *p_handle, void *p_userdata)
{
  pthread_mutex_lock(&g_obj.mutex);

  IMG_UNUSED(p_handle);
  IMG_UNUSED(p_userdata);

  if (g_obj.hdl.ptr == NULL) {
    IDBG_ERROR("%s:%d] Error: fcv unload called when handle NULL",
      __func__, __LINE__);
    pthread_mutex_unlock(&g_obj.mutex);
    return IMG_ERR_INVALID_OPERATION;
  }

  if (g_obj.ref_cnt <= 1) {
    if (dlclose(g_obj.hdl.ptr)) {
      IDBG_ERROR("%s:%d] error %s", __func__, __LINE__,
        dlerror());
    }

    g_obj.hdl.ptr = NULL;
    g_obj.hdl.pFcvScaleDownMNu8 = NULL;
    g_obj.hdl.pFcvScaleDownMNInterleaveu8 = NULL;
    g_obj.hdl.pfcvRotateImageu8 = NULL;
    g_obj.hdl.pfcvRotateImageInterleavedu8 = NULL;
    g_obj.hdl.pfcvSetOperationMode = NULL;
    g_obj.hdl.pfcvCleanUp = NULL;

  }

  if (g_obj.ref_cnt > 0) {
    g_obj.ref_cnt--;
  }
  pthread_mutex_unlock(&g_obj.mutex);

  return IMG_SUCCESS;
}

/** img_fcv_reload_lib
 *
 *  Arguments:
 *    @p_handle - Pointer to fastcv lib handle.
 *    @p_lib_name - fastcv library name.
 *    @p_userdata - Pointer to userdata.
 *
 *  Return values:
 *    IMG_SUCCESS/IMG_ERR_
 *
 *  Description:
 *    This function is called to load fastcv lib.
 *
 **/
int img_fcv_reload_lib(void *p_handle, const char *p_lib_name, void *p_userdata)
{
  int rc = IMG_SUCCESS;
  IDBG_HIGH("%s:%d] E", __func__, __LINE__);

  IMG_UNUSED(p_handle);
  IMG_UNUSED(p_userdata);

  pthread_mutex_lock(&g_obj.mutex);

  if (!g_obj.hdl.ptr) {
    g_obj.hdl.ptr = dlopen(p_lib_name, RTLD_NOW);
    if (!g_obj.hdl.ptr) {
      IDBG_ERROR("%s:%d] Error opening fastcv library %s\n",
        __func__, __LINE__, dlerror());
      pthread_mutex_unlock(&g_obj.mutex);
      return IMG_ERR_NOT_FOUND;
    }

    if (IMG_SUCCESS != img_fcv_load_fn_ptrs(&g_obj.hdl)) {
      IDBG_ERROR("%s:%d] Error fastcv loading failed\n",
        __func__, __LINE__);
      pthread_mutex_unlock(&g_obj.mutex);
      return IMG_ERR_NOT_FOUND;
    }

    if (!g_obj.dsp_dlopen_f) {
      rc = img_dsp_dlopen(p_lib_name,
        &g_obj.hdl,
        img_fcv_unload_lib,
        img_fcv_reload_lib);
      if (rc == IMG_SUCCESS) {
        g_obj.dsp_dlopen_f = TRUE;
      } else {
        IDBG_ERROR("%s:%d] Error opening img_dsp\n",
          __func__, __LINE__);
      }
    }

  }

  g_obj.ref_cnt++;
  pthread_mutex_unlock(&g_obj.mutex);

  return IMG_SUCCESS;
}

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
int img_fcv_get_lib_hdl(img_fcv_lib_handle_t **pp_handle)
{
  int rc = IMG_SUCCESS;

  if (pp_handle == NULL) {
    IDBG_ERROR("%s:%d] Error Invalid NULL pointer\n",
      __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  rc = img_fcv_reload_lib(NULL, IMG_FCV_LIB_NAME, NULL);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] Fastcv lib load failed, error %d\n",
      __func__, __LINE__, rc);
    return rc;
  }
  *pp_handle = &g_obj.hdl;

  return rc;
}

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
int img_fcv_put_lib_hdl(img_fcv_lib_handle_t *p_handle)
{
  int rc = IMG_SUCCESS;

  rc = img_fcv_unload_lib(NULL, NULL);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] Fastcv lib unload failed, error %d\n",
      __func__, __LINE__, rc);
    return rc;
  }

  if (g_obj.ref_cnt < 1) {
    if (g_obj.dsp_dlopen_f == TRUE) {
      img_dsp_dlclose(&g_obj.hdl);
      g_obj.dsp_dlopen_f = FALSE;
    }
  }

  return rc;
}
