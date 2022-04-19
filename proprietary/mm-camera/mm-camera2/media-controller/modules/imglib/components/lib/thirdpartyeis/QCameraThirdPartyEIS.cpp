/**********************************************************************
*  Copyright (c) 2019 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "QCameraThirdPartyEIS.h"
#include <sys/types.h>
#include <stdbool.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <utils/Log.h>
#include <assert.h>
#include <sys/time.h>
#include <fcntl.h>
#include <cutils/properties.h>
#include <android/log.h>
#include <vector>

#ifdef _ANDROID_
#include <utils/Log.h>
#include <cutils/properties.h>
#endif

/**
 * Function: img_algo_init
 *
 * Description: algorirhm initialization
 *
 * Arguments:
 *   @pp_context: return context [Output parameter]
 *   @p_params: init params
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
extern "C" int img_algo_init(void **p_context, img_init_params_t *p_params)
{
  IDBG_LOW(": E");
  int ret = IMG_ERR_GENERAL;
  IMG_UNUSED(p_params);

  QCameraThirdPartyEIS *pCamThirdPartyEIS;
  pCamThirdPartyEIS = new QCameraThirdPartyEIS;
  if (pCamThirdPartyEIS)
    *p_context = pCamThirdPartyEIS;
  else
    ret = IMG_ERR_GENERAL;
  IDBG_LOW(": X");

  return ret;
}

extern "C" int img_algo_get_lib_config(void *p_context, void *p_data)
{
  IDBG_LOW(": E");
  return IMG_SUCCESS;
}

/**
 * Function: img_algo_deinit
 *
 * Description: algorithm de-initialization
 *
 * Arguments:
 *   @arg1:
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
extern "C" int img_algo_deinit(void *p_context)
{
  IDBG_LOW("img_algo_deinit called");
  assert(NULL != p_context);

  QCameraThirdPartyEIS* pCamThirdPartyEIS = (QCameraThirdPartyEIS*) p_context;
  delete pCamThirdPartyEIS;

  IDBG_LOW(": X");
  return IMG_SUCCESS;
}

/**
 * Function: img_algo_deinit
 *
 * Description: algorithm de-initialization
 *
 * Arguments:
 *   @arg1:
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
extern "C" int img_algo_stop(void *p_context)
{
  IDBG_LOW("img_algo_stop called");
  assert(NULL != p_context);

  QCameraThirdPartyEIS* pCamThirdPartyEIS = (QCameraThirdPartyEIS*) p_context;
  if (pCamThirdPartyEIS) {
    pCamThirdPartyEIS->Flush();
  }
  IDBG_LOW(": X");
  return IMG_SUCCESS;
}

/**
 * Function: img_algo_set_callback
 *
 * Description: Set callback pointer
 *
 * Arguments:
 *   @p_context: class pointer
 *   @algo_callback: user callback function
 *
 * Return values:
 *   IMG_SUCCESS or IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
extern "C" int img_algo_set_callback(void *p_context,
  int (*algo_callback)(void *user_data))
{
  assert(NULL != p_context);
  QCameraThirdPartyEIS *pCamThirdPartyEIS = (QCameraThirdPartyEIS *)p_context;
  int ret = IMG_SUCCESS;

  if (!pCamThirdPartyEIS || !algo_callback) {
    IDBG_ERROR(": failed: %p %p", pCamThirdPartyEIS, algo_callback);
    return IMG_ERR_INVALID_INPUT;
  }
  pCamThirdPartyEIS->setDeinitStart(false);
  pCamThirdPartyEIS->callback = algo_callback;

  return IMG_SUCCESS;
}



/**
 * Function: img_algo_process
 *
 * Description: algorithm main body
 *
 * Arguments:
 *   @arg1: p_context: class pointer
 *   @arg2: p_in_frame: input frame handler
 *   @arg3: in_frame_cnt: number of input frames
 *   @arg4: p_out_frame: output frame handler
 *   @arg5: out_frame_cnt: number of output frames
 *   @arg6: p_meta: frame meta data
 *   @arg7: meta_cnt: number of meta data array
 *
 * Return values:
 *   IMG_SUCCESS or IMG_ERR_GENERAL
 *
 * Notes: none
 **/
extern "C" int img_algo_process(void *p_context, img_frame_t *p_in_frame[],
  int in_frame_cnt,img_frame_t *p_out_frame[],
  int out_frame_cnt, img_meta_t *p_meta[], int meta_cnt)
{
  int ret = IMG_SUCCESS;

  if (p_context != NULL) {
    QCameraThirdPartyEIS* pCamThirdPartyEIS = (QCameraThirdPartyEIS*) p_context;
  }
  IDBG_LOW(": X");

  return ret;
}

/**
 * Function: img_algo_set_frame_ops
 *
 * Description: Function to set frame operations
 *
 * Arguments:
 *   @p_context: ptr to instance
 *   @p_ops: ptr to frame ops
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
extern "C" int img_algo_set_frame_ops(void *p_context,
  img_frame_ops_t *p_ops)
{
  IDBG_LOW(": E");
  int ret = IMG_SUCCESS;
  assert(NULL != p_context);
  assert(NULL != p_ops);
  QCameraThirdPartyEIS* instance = (QCameraThirdPartyEIS*) p_context;
  instance->setFrameOps(p_ops);
  IDBG_LOW(": X");
  return ret;
}

/**
 * Function: QCameraThirdPartyEIS
 *
 * Description: constructor of class
 *
 * Arguments:
 *   none
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
QCameraThirdPartyEIS::QCameraThirdPartyEIS()
{
  IDBG_LOW(": E");

  /* OEM can initialize their lib data */

  IDBG_LOW(": X");
}

/**
 * Function: ~QCameraThirdPartyEIS
 *
 * Description: destructor of class
 *
 * Arguments:
 *   @arg1:
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
QCameraThirdPartyEIS::~QCameraThirdPartyEIS()
{

}

/**
 * Function: QCameraThirdPartyEIS: Init
 *
 * Description: Init of class
 *
 * Arguments:
 *   @arg1:
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
bool QCameraThirdPartyEIS::Init()
{
  IDBG_LOW("E");

  return true;
}

/**
 * Function: QCameraThirdPartyEIS::Flush
 *
 * Description: DeInit function of class
 *
 * Arguments:
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void QCameraThirdPartyEIS::Flush()
{

}


/**
 * Function: QCameraThirdPartyEIS::deInit
 *
 * Description: DeInit function of class
 *
 * Arguments:
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
bool QCameraThirdPartyEIS::DeInit()
{
  bool ret = TRUE;
  return ret;
}
