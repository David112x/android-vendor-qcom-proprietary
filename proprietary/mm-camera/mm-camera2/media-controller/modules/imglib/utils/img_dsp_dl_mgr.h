/**********************************************************************
* Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __IMG_DSP_DL_MGR_H__
#define __IMG_DSP_DL_MGR_H__

/** IMG_DL_UNLOAD_FN_TYPE:
 *
 *  Function type to for a lib unload callback
 **/
typedef  int (* IMG_DL_UNLOAD_FN_TYPE )(void *, void *);
/** IMG_DL_RELOAD_FN_TYPE:
 *
 *  Function type to for a lib reload callback
 **/
typedef  int (* IMG_DL_RELOAD_FN_TYPE )(void *, const char *, void *);

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
int img_dsp_dlopen (const char *name, void* p_libptr,
  IMG_DL_UNLOAD_FN_TYPE fn_unload,
  IMG_DL_RELOAD_FN_TYPE fn_reload);

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
int img_dsp_dlclose (void* p_libptr);

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
int img_dsp_dl_requestall_to_close_and_reopen();

/** img_dsp_dl_mgr_set_reload_needed
 *
 *  Arguments:
 *    @bSetNeedReload - if reload will be needed.
 *
 *   Return values:
 *     IMG_SUCCESS/IMG_ERR_
 *
 *  Description:
 *    this function is called to set if reload of DSP libs is
 *    needed
 *
 **/
int img_dsp_dl_mgr_set_reload_needed(int b_need_reload);

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
int img_dsp_dl_mgr_get_reload_needed();

#endif //__IMG_DSP_DL_MGR_H__
