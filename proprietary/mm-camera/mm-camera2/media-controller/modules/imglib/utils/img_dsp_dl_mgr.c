/**********************************************************************
* Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "img_dsp_dl_mgr.h"
#include "img_list.h"
#include "img_common.h"
#include <pthread.h>
#include "hdr.h"
#include "img_fcv.h"


 /** faceproc_dsp_comp_t
 *   @p_dl_lib_list: list to store information about components
 *      registering callbacks
 *   @b_need_dsp_reload: Flag to indicate if dsp failure is
 *      reported
 *   @m_mutex : mutex to handle concurrecy around dsp
 *      reload flag
 *
 *   DSP dynamic linking mgr
 **/
typedef struct {
  img_list_t *p_dl_lib_list;
  int b_need_dsp_reload;
  pthread_mutex_t m_mutex;
} img_dsp_dl_mgr_t;

/** g_img_dsp_dl_mgr:
 *
 *  Global instance of DSP dynamic linking mgr
 **/
static img_dsp_dl_mgr_t g_dsp_dl_mgr;

/** DL_NAME_LENGTH_MAX:
 *
 *  Max length of dynamically loadd library
 **/
#define DL_NAME_LENGTH_MAX 256
/** STR_LLVD_LIB_NAME:
 *
 *  Partial name of llvd library to compare against.
 **/
#define STR_LLVD_LIB_NAME "llvd"
/** STR_FACEPROC_ADSP_LIB_NAME:
 *
 *  name of faceproc adsp library to compare against
 **/
#define STR_FACEPROC_ADSP_LIB_NAME "faceproc_adspstub"

/** STR_EA_LIB_NAME:
 *
 *  Partial name of edge smooth library to compare against.
 **/
#define STR_EA_LIB_NAME "edgesmooth"

/** STR_STILLMORE_LIB_NAME:
 *
 *  Partial name of stillmore library to compare against.
 **/
#define STR_STILLMORE_LIB_NAME "stillmore"

/**STR_SW_TNR_LIB_NAME:
 *
 *  Partial name of sw tnr library to compare against.
 **/
#define STR_SW_TNR_LIB_NAME "camera_sw_tnr"

/** img_dl_info_t:
 *
 *  Arguments:
 *    @dl_name: name of dsp library that opens adsprpc
 *    @p_libptr: pointer to userdata/pointer to struct storing
 *       pointer to library
 *    @fn_unload: pointer to function that unloads adsprpc
 *       dependent library
 *    @fn_reload:  pointer to function that reloads adsprpc
 *       dependent library
 *
 *  Description:
 *    Struct to store info about adsprpc dependent
 *    dynamically linked shared objects and functions to
 *    unload/reload.
 */
typedef struct {
  char dl_name[DL_NAME_LENGTH_MAX];
  void* p_libptr;
  IMG_DL_UNLOAD_FN_TYPE fn_unload;
  IMG_DL_RELOAD_FN_TYPE fn_reload;
} img_dl_info_t;

img_dl_info_t g_dl_info_t;

/** img_dsp_dlopen
 *
 *  Arguments:
 *    @dl_name: name of dsp library that opens adsprpc
 *    @p_libptr: pointer to userdata/pointer to struct storing
 *      pointer to library
 *    @fn_unload: pointer to function that unloads adsprpc
 *      dependent library
 *    @fn_reload:  pointer to function that reloads adsprpc
 *      dependent library
 *
 *   Return values:
 *     IMG_SUCCESS/IMG_ERR_
 *
 *  Description:
 *    this function is called after a adsprpc dependent so lib
 *    is loaded to register a callbacks to unload and reload
 *    when adsp SSR happens.
 *
 **/
int img_dsp_dlopen(const char *name, void* p_libptr,
  IMG_DL_UNLOAD_FN_TYPE fn_unload,
  IMG_DL_RELOAD_FN_TYPE fn_reload)
{
  img_dl_info_t *p_dl_info = NULL;
  IDBG_HIGH("%s:%d] Enter %s", __func__, __LINE__, name);

  if (!g_dsp_dl_mgr.p_dl_lib_list) {
    g_dsp_dl_mgr.p_dl_lib_list = img_list_create();

    if (!g_dsp_dl_mgr.p_dl_lib_list) {
      IDBG_ERROR("%s:%d] image list creat failed", __func__, __LINE__);
      return IMG_ERR_GENERAL;
    }

    if (pthread_mutex_init(&g_dsp_dl_mgr.m_mutex, NULL)) {
      IDBG_ERROR("%s:%d] Mutex initialization failed", __func__, __LINE__);
      return IMG_ERR_GENERAL;
    }
  }

  p_dl_info = (img_dl_info_t *) malloc(sizeof(img_dl_info_t));
  if (!p_dl_info) {
    IDBG_ERROR("%s:%d] Cannot allocate memory", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }

  if (strstr(name, STR_LLVD_LIB_NAME) ||
    strstr(name, STR_FACEPROC_ADSP_LIB_NAME) ||
    strstr(name, HDR_COMP_LIB_NAME) ||
    strstr(name, STR_EA_LIB_NAME) ||
    strstr(name, STR_SW_TNR_LIB_NAME) ||
    strstr(name, IMG_FCV_LIB_NAME) ||
    strstr(name, STR_STILLMORE_LIB_NAME)) {
    IDBG_HIGH("%s:%d] Found lib %s",
      __func__, __LINE__, name);
    strlcpy( p_dl_info->dl_name, name, DL_NAME_LENGTH_MAX);
    p_dl_info->p_libptr = p_libptr;
    p_dl_info->fn_unload = fn_unload;
    p_dl_info->fn_reload = fn_reload;
  } else {
    IDBG_MED("%s:%d] lib %s not found ",
      __func__, __LINE__, name);
  }
  //Add job to job list
  img_list_append(g_dsp_dl_mgr.p_dl_lib_list, p_dl_info);

  return IMG_SUCCESS;
}

/** img_dsp_dl_find_dl_info_by_libptr
 *
 *  Arguments:
 *    @p_nodedata: pointer to node data
 *    @p_userdata: pointer to userdata provided for searching in
 *      the list
 *
 *   Return values:
 *     IMG_SUCCESS/IMG_ERR_
 *
 *  Description:
 *    finds the node in the list by the libptr provided with
 *    callback registration.
 *
 **/
static int img_dsp_dl_find_dl_info_by_libptr(void *p_nodedata,
  void *p_userdata)
{
  img_dl_info_t *p_dl_info = NULL;

  if ((NULL == p_nodedata) || (NULL == p_userdata)) {
    IDBG_ERROR("%s:%d] Invalid input", __func__,  __LINE__);
    return FALSE;
  }

  p_dl_info = (img_dl_info_t *)p_nodedata;

  if (p_dl_info->p_libptr == p_userdata) {
    IDBG_HIGH("%s:%d] FOUND ", __func__, __LINE__);
    return TRUE;
  }
  IDBG_HIGH("%s:%d] NOT FOUND ", __func__, __LINE__);
  return FALSE;
}


/** img_dsp_dlclose
 *
 *  Arguments:
 *    @p_libptr: pointer to userdata/pointer to struct storing
 *
 *   Return values:
 *     IMG_SUCCESS/IMG_ERR_
 *
 *  Description:
 *    this function is called after a adsprpc dependent so lib
 *    is closed to unregister the unload/reload callbacks
 *
 **/
int img_dsp_dlclose(void* p_libptr)
{
  int rc = IMG_SUCCESS;

  IDBG_HIGH("%s:%d] E", __func__, __LINE__);
  if (!g_dsp_dl_mgr.p_dl_lib_list) {
    IDBG_ERROR("%s:%d] no list present", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  rc = img_list_remove(g_dsp_dl_mgr.p_dl_lib_list,
    img_dsp_dl_find_dl_info_by_libptr, p_libptr);

  if (IMG_SUCCESS == rc) {
    IDBG_HIGH("%s:%d] Removed successfully ", __func__, __LINE__);
  } else {
    IDBG_HIGH("%s:%d] NOT FOUND ", __func__, __LINE__);
  }
  return rc;
}

/** img_dsp_dl_process_list_each_node_unload
 *
 *  Arguments:
 *    @p_nodedata: pointer to node data
 *    @p_userdata: pointer to userdata provided for searching in
 *       the list. Not used here
 *
 *   Return values:
 *     IMG_SUCCESS/IMG_ERR_
 *
 *  Description:
 *    Function is called for each node in list, this calls the
 *    unload callback for current node.
 *
 **/
static int img_dsp_dl_process_list_each_node_unload(void *p_nodedata,
  void *p_userdata)
{
  img_dl_info_t *p_dl_info = NULL;

  if (NULL == p_nodedata) {
    IDBG_ERROR("%s:%d] Invalid input",  __func__,  __LINE__);
    return IMG_ERR_GENERAL;
  }
  p_dl_info = (img_dl_info_t *)p_nodedata;

  if (p_dl_info->p_libptr &&
    p_dl_info->fn_reload && p_dl_info->fn_unload) {
    IDBG_HIGH("%s:%d] processing node %s ", __func__, __LINE__,
      p_dl_info->dl_name);
    p_dl_info->fn_unload(p_dl_info->p_libptr, p_userdata);
    IDBG_HIGH("%s:%d] Unload done ", __func__, __LINE__);
  } else {
    IDBG_HIGH("not procesing node name %s ", (strlen(p_dl_info->dl_name)?
      p_dl_info->dl_name: ""));
  }
  return IMG_SUCCESS;
}

/** img_dsp_dl_process_list_each_node_reload
 *
 *  Arguments:
 *    @p_nodedata: pointer to node data
 *    @p_userdata: pointer to userdata provided for searching in
 *      the list
 *
 *   Return values:
 *     IMG_SUCCESS/IMG_ERR_
 *
 *  Description:
 *    Function is called for each node in list, this calls the
 *    lib reload callback for current node.
 *
 **/
static int img_dsp_dl_process_list_each_node_reload(void *p_nodedata,
  void *p_userdata)
{
  IMG_UNUSED(p_userdata);
  img_dl_info_t *p_dl_info = NULL;

  if (NULL == p_nodedata) {
    IDBG_ERROR("%s:%d] Invalid input",  __func__,  __LINE__);
    return IMG_ERR_GENERAL;
  }
  p_dl_info = (img_dl_info_t *)p_nodedata;

  if (p_dl_info->p_libptr &&
    p_dl_info->fn_reload && p_dl_info->fn_unload) {
    IDBG_HIGH("%s:%d] processing node %s ", __func__, __LINE__,
      p_dl_info->dl_name);
    p_dl_info->fn_reload(p_dl_info->p_libptr, p_dl_info->dl_name, p_userdata);
    IDBG_HIGH("%s:%d] Reload done ", __func__, __LINE__);
  } else {
    IDBG_HIGH("not procesing node name %s ", (strlen(p_dl_info->dl_name)?
      p_dl_info->dl_name: ""));
  }
  return IMG_SUCCESS;
}

/** img_dsp_dl_requestall_to_close_and_reopen
 *
 *  Arguments:
 *    None
 *
 *   Return values:
 *     IMG_SUCCESS/IMG_ERR_
 *
 *  Description:
 *    this function is called after a adsprpc dependent so lib
 *    is closed to unregister the unload/reload callbacks
 *
 **/
int img_dsp_dl_requestall_to_close_and_reopen()
{
  uint8_t ssr_flag;

  IDBG_MED("%s:%d] E", __func__, __LINE__);

  if (img_dsp_dl_mgr_get_reload_needed()) {
    IDBG_HIGH("%s:%d] SSR occured, reload libs", __func__, __LINE__);
    pthread_mutex_lock(&g_dsp_dl_mgr.m_mutex);

    ssr_flag = 1;
    img_list_traverse(g_dsp_dl_mgr.p_dl_lib_list,
      img_dsp_dl_process_list_each_node_unload, &ssr_flag);
    img_list_traverse(g_dsp_dl_mgr.p_dl_lib_list,
      img_dsp_dl_process_list_each_node_reload, &ssr_flag);

    g_dsp_dl_mgr.b_need_dsp_reload = FALSE;
    IDBG_ERROR("SSR occured, reload libs DONE");

    pthread_mutex_unlock(&g_dsp_dl_mgr.m_mutex);
  }
  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return IMG_SUCCESS;
}



/** img_dsp_dl_mgr_set_reload_needed
 *
 *  Arguments:
 *    @b_need_reload - if reload will be needed.
 *
 *  Return values:
 *     IMG_SUCCESS
 *
 *  Description:
 *    this function is called to check if reload of DSP libs is
 *    needed
 *
 **/
int img_dsp_dl_mgr_set_reload_needed(int b_need_reload)
{
  pthread_mutex_lock(&g_dsp_dl_mgr.m_mutex);
  if (g_dsp_dl_mgr.b_need_dsp_reload != b_need_reload) {
    IDBG_ERROR("Need reload for SSR old %d new %d",
      g_dsp_dl_mgr.b_need_dsp_reload, b_need_reload);
  }
  g_dsp_dl_mgr.b_need_dsp_reload = b_need_reload;
  pthread_mutex_unlock(&g_dsp_dl_mgr.m_mutex);
  return IMG_SUCCESS;
}

/** img_dsp_dl_mgr_get_reload_needed
 *
 *  Arguments:
 *    None
 *
 *   Return values:
 *     TRUE
 *     FALSE
 *
 *  Description:
 *    this function is called to check if reload of DSP libs is
 *    needed
 *
 **/
int img_dsp_dl_mgr_get_reload_needed()
{
  int b_ret;
  pthread_mutex_lock(&g_dsp_dl_mgr.m_mutex);
  IDBG_MED("%s:%d] Need reload for SSR %d", __func__, __LINE__,
    g_dsp_dl_mgr.b_need_dsp_reload);
  b_ret = g_dsp_dl_mgr.b_need_dsp_reload;
  pthread_mutex_unlock(&g_dsp_dl_mgr.m_mutex);
  return g_dsp_dl_mgr.b_need_dsp_reload;
}

